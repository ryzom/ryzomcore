#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Robust ARK log watcher:
- Follows a logfile like `tail -F`
- Survives truncate, rotation, delete/recreate
- On "[ARK] AIS UP": re-open the log and start from the beginning of the (new) file
- Executes commands or opens URLs found after "[ARK]"
- Uses subprocess without shell, with proper quoting
- Optional: leverages inotify if pyinotify is available; otherwise falls back to polling
"""

import os
import sys
import time
import json
import errno
import signal
import socket
import urllib.request
import urllib.parse
import subprocess

# ----------- Config (adjust if needed) -----------
REMOTE_HOST = "app@app.ryzom.com"
REMOTE_CMD  = "/home/app/bin/run_cmd_gingo"
URL_OPEN_TIMEOUT = 10       # seconds
NET_RETRIES = 3
NET_RETRY_BACKOFF = 1.5     # exponential backoff multiplier
POLL_INTERVAL = 0.05        # seconds when idle
REOPEN_WAIT    = 0.2        # seconds between reopen attempts
# -------------------------------------------------

try:
    import pyinotify  # optional
    HAVE_INOTIFY = True
except Exception:
    HAVE_INOTIFY = False

# ----------- Helpers -----------

def debug(msg):
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{ts}] {msg}", flush=True)

def wait_for_file(path):
    """Block until file exists (useful after rotation/restart)."""
    while True:
        if os.path.exists(path) and os.path.isfile(path):
            try:
                # Also ensure it’s openable
                with open(path, "rb"):
                    return
            except Exception:
                pass
        time.sleep(REOPEN_WAIT)

def open_log(path, *, from_start=False):
    """
    Open the log file robustly.
    If from_start=True -> seek to beginning, else to end.
    Returns (fp, stat_result)
    """
    wait_for_file(path)
    fp = open(path, "r", encoding="utf-8", errors="replace", buffering=1)
    st = os.fstat(fp.fileno())
    if from_start:
        fp.seek(0, os.SEEK_SET)
    else:
        fp.seek(0, os.SEEK_END)
    debug(f"Opened log: ino={st.st_ino} size={st.st_size} from_start={from_start}")
    return fp, st

def same_file(st_a, st_b):
    return (st_a.st_ino == st_b.st_ino) and (st_a.st_dev == st_b.st_dev)

def fetch_url(url):
    """
    Open URL with retries; print response (truncated if huge).
    """
    last_err = None
    for attempt in range(1, NET_RETRIES + 1):
        try:
            with urllib.request.urlopen(url, timeout=URL_OPEN_TIMEOUT) as resp:
                data = resp.read()
                # Avoid dumping megabytes
                out = data[:4096]
                debug(f"URL OK ({len(data)} bytes, showing {len(out)}): {url}")
                try:
                    print(out.decode("utf-8", errors="replace"))
                except Exception:
                    print(out)
                return True
        except Exception as e:
            last_err = e
            debug(f"URL attempt {attempt}/{NET_RETRIES} failed: {e!r}")
            time.sleep((NET_RETRY_BACKOFF ** (attempt - 1)))
    debug(f"URL failed after {NET_RETRIES} attempts: {last_err!r}")
    return False

def run_remote_ark_command(command_text):
    """
    Safely run: ssh app@app.ryzom.com /home/app/bin/run_cmd_gingo -<encoded>
    """
    # Normalize and encode; preserve slashes
    encoded = urllib.parse.quote_plus(command_text, safe="/", encoding="utf-8", errors="replace")
    ssh_cmd = ["ssh", REMOTE_HOST, REMOTE_CMD, f"-{encoded}"]
    last_err = None
    for attempt in range(1, NET_RETRIES + 1):
        try:
            proc = subprocess.run(ssh_cmd, capture_output=True, text=True, check=False)
            debug(f"SSH returncode={proc.returncode}")
            if proc.stdout:
                print(proc.stdout.strip())
            if proc.stderr:
                print(proc.stderr.strip(), file=sys.stderr)
            if proc.returncode == 0:
                return True
            last_err = f"Non-zero exit: {proc.returncode}"
        except Exception as e:
            last_err = e
        debug(f"SSH attempt {attempt}/{NET_RETRIES} issue: {last_err}")
        time.sleep((NET_RETRY_BACKOFF ** (attempt - 1)))
    debug(f"SSH failed after {NET_RETRIES} attempts.")
    return False

def parse_ark_command(line):
    """
    Extract command after [ARK] token.
    Accepts both '[ARK] ' and '[ARK]' without trailing space.
    Returns None if no ARK token.
    """
    if "[ARK]" not in line:
        return None
    # Normalize NBSP -> space
    line = line.replace("\xa0", " ")
    # Split on first occurrence of [ARK]
    pre, sep, post = line.partition("[ARK]")
    if not sep:  # token not found
        return None
    # Trim an optional single leading space after [ARK]
    if post.startswith(" "):
        post = post[1:]
    cmd = post.strip()
    return cmd if cmd else None

class ReopenSignal(Exception):
    """Internal control flow to request log reopen (e.g., on AIS UP)."""

# ----------- Tailer core -----------

def follow_log(path):
    """
    Generator that yields new lines from log `path`, surviving rotations and truncations.
    - Reopen on inode change
    - Reopen on shrink
    - Reopen when file is deleted/recreated
    """
    fp, st = open_log(path, from_start=False)

    def need_reopen():
        nonlocal st
        try:
            cur = os.stat(path)
        except FileNotFoundError:
            return True, None
        if not same_file(st, cur):
            return True, cur
        try:
            cur_size = os.path.getsize(path)
        except FileNotFoundError:
            return True, None
        # If file shrank (truncate), we should reopen and go to start
        try:
            pos = fp.tell()
        except Exception:
            pos = None
        if cur_size < (pos if pos is not None else 0):
            return True, cur
        return False, cur

    # Optional inotify to wake faster
    wm = None
    notifier = None
    mask = 0
    if HAVE_INOTIFY:
        try:
            wm = pyinotify.WatchManager()
            mask = (pyinotify.IN_MODIFY | pyinotify.IN_MOVE_SELF |
                    pyinotify.IN_ATTRIB | pyinotify.IN_DELETE_SELF)
            class Handler(pyinotify.ProcessEvent):
                def process_default(self, event):
                    # We don't do anything here; notifier is just to wake quickly
                    pass
            notifier = pyinotify.Notifier(wm, Handler())
            wm.add_watch(path, mask, quiet=True)
            debug("inotify active")
        except Exception as e:
            debug(f"inotify unavailable: {e!r}")
            wm = None
            notifier = None

    try:
        while True:
            line = fp.readline()
            if line:
                yield line
                continue

            # No new line: check if we must reopen
            reopen, cur = need_reopen()
            if reopen:
                try:
                    fp.close()
                except Exception:
                    pass
                # If file vanished, wait until it returns; then from_start=True
                if cur is None:
                    debug("Log missing -> waiting to reopen (from start)")
                    fp, st = open_log(path, from_start=True)
                else:
                    debug("Log rotated/truncated -> reopening (from start)")
                    fp, st = open_log(path, from_start=True)
                # Refresh inotify watch if enabled
                if HAVE_INOTIFY and wm is not None:
                    try:
                        wm.rm_watch(list(wm.watches.values()))
                    except Exception:
                        pass
                    try:
                        wm.add_watch(path, mask, quiet=True)
                    except Exception as e:
                        debug(f"Could not re-add inotify watch: {e!r}")
                continue

            # Idle wait
            if notifier:
                # Non-blocking event loop step to wake quickly
                notifier.process_events()
                time.sleep(POLL_INTERVAL)
                if notifier.check_events(timeout=int(POLL_INTERVAL * 1000)):
                    notifier.read_events()
            else:
                time.sleep(POLL_INTERVAL)
    finally:
        try:
            fp.close()
        except Exception:
            pass
        if notifier:
            try:
                notifier.stop()
            except Exception:
                pass

# ----------- Main processing -----------

def handle_command(cmd):
    """
    Execute the action specified by ARK command string.
    Special case: "AIS UP" -> signal reopen/reset to caller.
    """
    if cmd == "AIS UP":
        debug("Detected AIS UP -> request log reopen from start.")
        raise ReopenSignal()

    if cmd.startswith("https://app.ryzom.com/") and not cmd.startswith("https://app.ryzom.com/app_arcc/"):
        debug(f"OPEN URL: {cmd}")
        fetch_url(cmd)
        return

    # Default: run remote ark command via ssh
    debug(f"RUN ARK COMMAND: {cmd}")
    run_remote_ark_command(cmd)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {os.path.basename(sys.argv[0])} /path/to/logfile", file=sys.stderr)
        sys.exit(2)

    log_path = sys.argv[1]
    debug(f"Watching ARK logs at: {log_path}")

    # Graceful shutdown
    stop = {"flag": False}
    def _sigterm(_s, _f):
        stop["flag"] = True
    signal.signal(signal.SIGINT, _sigterm)
    signal.signal(signal.SIGTERM, _sigterm)

    while not stop["flag"]:
        try:
            # Follow current log until we need to reopen (ReopenSignal) or external stop
            for line in follow_log(log_path):
                if stop["flag"]:
                    break
                cmd = parse_ark_command(line)
                if not cmd:
                    continue
                try:
                    handle_command(cmd)
                except ReopenSignal:
                    # On AIS UP: close current follow and immediately reopen from start of (new) file
                    debug("Reopening log due to AIS UP...")
                    # Break outer ‘follow_log’ loop to trigger a fresh follow
                    break
        except Exception as e:
            # Unexpected error: log and wait a bit before retry
            debug(f"Unhandled error: {e!r} -> retrying in 1s")
            time.sleep(1)
            continue
        # Small pause before re-follow to avoid busy loops on instant reopen
        time.sleep(0.1)

    debug("Stopped.")

if __name__ == "__main__":
    main()
