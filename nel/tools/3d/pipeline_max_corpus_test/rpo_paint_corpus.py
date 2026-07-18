#!/usr/bin/env python3
"""Corpus driver for the zone painter's RPatchMesh write path (design doc §14-paint).

Two tiers over every OLE .max under the graphics + snowballs workspaces:

  SELFTEST     pipeline_max_corpus_test --rpo-selftest: decode->encode must be the byte
               identity on every RPatchMesh blob — the base RPO 0x08FD of every RklPatch
               object and the RFINALPATCH 0x4001 of every NeL Edit Patch / NeL Patch
               Painter per-node snapshot — and 0x4001 must appear only under those two
               modifier classes.
  MODIFY-SAVE  pipeline_max_corpus_test --rpo-modify-save-test on every blob-carrying file:
               push every blob through the write path in place (setRPatch / re-encoded
               0x4001 leaf), rebuild the Scene stream, write the whole .max back, and
               require EVERY stream byte-identical — the painter's save path must be a
               no-op for a null edit.

Gate floors pin corpus coverage (2026-07-18 landing sweep: 8743 files, 7190 rpo + 1660
snapshots, all v9). Self-skips (exit 77) when the asset workspaces aren't present.
"""

import argparse, concurrent.futures, os, re, subprocess, sys

SKIP_CODE = 77
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_SNOWBALLS = os.path.expanduser("~/snowballs_source")
DEF_BIN = os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin")

SELF_RE = re.compile(r"^(OK|FAIL) rpo-selftest: (\d+) rpo, (\d+) snapshots, (\d+) decode-fail, (\d+) mismatch, (\d+) other-class")
SAVE_RE = re.compile(r"^(OK|FAIL) rpo-modify-save: (?:no-rpo|(\d+) rpo, (\d+) snapshots, (\d+) fail)")


def enumerate_corpus(roots):
    files = []
    for root in roots:
        if not os.path.isdir(root):
            continue
        for dirpath, dirnames, filenames in os.walk(root):
            for fn in filenames:
                if not fn.lower().endswith(".max"):
                    continue
                p = os.path.join(dirpath, fn)
                try:
                    with open(p, "rb") as f:
                        if f.read(8) != OLE_MAGIC:
                            continue
                except OSError:
                    continue
                files.append(p)
    files.sort()
    return files


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=DEF_GRAPHICS)
    ap.add_argument("--snowballs", default=DEF_SNOWBALLS)
    ap.add_argument("--bin", default=DEF_BIN)
    ap.add_argument("--gate", action="store_true")
    ap.add_argument("--min-rpo", type=int, default=7190)
    ap.add_argument("--min-snapshots", type=int, default=1660)
    ap.add_argument("--filter", default=None)
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()

    if not os.path.isdir(args.graphics):
        print("SKIP: no graphics workspace at %s" % args.graphics)
        return SKIP_CODE
    binp = os.path.join(args.bin, "pipeline_max_corpus_test")

    files = enumerate_corpus([args.graphics, args.snowballs])
    if args.filter:
        files = [f for f in files if args.filter in f]
    print("%d ole .max files" % len(files), flush=True)
    if not files:
        return SKIP_CODE

    def run(mode, p):
        try:
            r = subprocess.run([binp, mode, p], capture_output=True, text=True, timeout=900)
        except subprocess.TimeoutExpired:
            return (p, None, "", "TIMEOUT")
        return (p, r.returncode, r.stdout, r.stderr)

    # --- SELFTEST tier ---------------------------------------------------------------------
    tot = {"rpo": 0, "snap": 0, "dfail": 0, "mismatch": 0, "oclass": 0}
    carriers = []
    hard_fails = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for p, rc, out, errout in ex.map(lambda f: run("--rpo-selftest", f), files):
            m = None
            for line in out.splitlines():
                mm = SELF_RE.match(line)
                if mm:
                    m = mm
            if not m or rc not in (0, 1):
                hard_fails.append((p, "selftest rc=%s %s" % (rc, errout.strip()[:200])))
                continue
            rpo, snap = int(m.group(2)), int(m.group(3))
            tot["rpo"] += rpo
            tot["snap"] += snap
            tot["dfail"] += int(m.group(4))
            tot["mismatch"] += int(m.group(5))
            tot["oclass"] += int(m.group(6))
            if rpo or snap:
                carriers.append(p)
            if m.group(1) == "FAIL":
                hard_fails.append((p, m.group(0)))
    print("SELFTEST: %d files, %d rpo, %d snapshots, %d decode-fail, %d mismatch, %d other-class, %d carriers"
          % (len(files), tot["rpo"], tot["snap"], tot["dfail"], tot["mismatch"], tot["oclass"], len(carriers)), flush=True)

    # --- MODIFY-SAVE tier (blob-carrying files only) ---------------------------------------
    sv = {"rpo": 0, "snap": 0, "fail": 0}
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for p, rc, out, errout in ex.map(lambda f: run("--rpo-modify-save-test", f), carriers):
            m = None
            for line in out.splitlines():
                mm = SAVE_RE.match(line)
                if mm:
                    m = mm
            if not m or rc != 0 or m.group(1) == "FAIL":
                hard_fails.append((p, "modify-save rc=%s %s %s" % (rc, (m.group(0) if m else "no summary"), errout.strip()[:200])))
                continue
            if m.group(2) is not None:
                sv["rpo"] += int(m.group(2))
                sv["snap"] += int(m.group(3))
                sv["fail"] += int(m.group(4))
    print("MODIFY-SAVE: %d carriers, %d rpo, %d snapshots, %d fail"
          % (len(carriers), sv["rpo"], sv["snap"], sv["fail"]), flush=True)

    for p, why in hard_fails[:40]:
        print("  FAIL %s : %s" % (p, why))

    ok = not hard_fails and not tot["dfail"] and not tot["mismatch"] and not tot["oclass"] and not sv["fail"]
    if args.gate:
        if tot["rpo"] < args.min_rpo:
            print("GATE: rpo %d < floor %d" % (tot["rpo"], args.min_rpo)); ok = False
        if tot["snap"] < args.min_snapshots:
            print("GATE: snapshots %d < floor %d" % (tot["snap"], args.min_snapshots)); ok = False
        if sv["rpo"] != tot["rpo"] or sv["snap"] != tot["snap"]:
            print("GATE: modify-save coverage %d/%d != selftest %d/%d"
                  % (sv["rpo"], sv["snap"], tot["rpo"], tot["snap"])); ok = False
    print("RPO-PAINT %s" % ("OK" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
