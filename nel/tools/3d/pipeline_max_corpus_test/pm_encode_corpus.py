#!/usr/bin/env python3
"""Corpus driver for the PatchMesh chunk-stream encoder (Tier B, design doc §14-paint).

One tier over every OLE .max under the graphics + snowballs workspaces:

  PM-MODIFY-SAVE  pipeline_max_corpus_test --pm-modify-save-test: decode every PatchMesh in
                  the file (the base stream of every RklPatch object and the OUTPUT copy
                  under every NeL Edit Patch / NeL Patch Painter modifier's 0x1140), write
                  each straight back through encodePatchMesh in place, rebuild the Scene
                  stream, write the whole .max back, and require EVERY stream
                  byte-identical — decode -> encode is the identity, proven BEFORE any
                  topological op exists. Max 3 streams (reconstructed edge tables) are
                  counted and skipped: their edge data is derived at decode.

Gate floors pin corpus coverage (2026-07-29 landing sweep over graphics + snowballs:
6943 base + 1539 modifier copies from the Tier B survey are the ligo minimum; the floors
sit slightly under the observed totals so a moved workspace fails loud, a grown one not at
all). Self-skips (exit 77) when the asset workspaces aren't present.
"""

import argparse, concurrent.futures, os, subprocess, sys, re

SKIP_CODE = 77
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_SNOWBALLS = os.path.expanduser("~/snowballs_source")
DEF_BIN = os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin")

SAVE_RE = re.compile(r"^(OK|FAIL) pm-modify-save: (?:no-pm|max3-only, (\d+) skipped|(\d+) base, (\d+) mod, (\d+) max3-skip, (\d+) fail)")


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
    ap.add_argument("--min-base", type=int, default=6900)
    ap.add_argument("--min-mod", type=int, default=1500)
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

    def run(p):
        try:
            r = subprocess.run([binp, "--pm-modify-save-test", p], capture_output=True, text=True, timeout=900)
        except subprocess.TimeoutExpired:
            return (p, None, "", "TIMEOUT")
        return (p, r.returncode, r.stdout, r.stderr)

    tot = {"base": 0, "mod": 0, "max3": 0, "fail": 0}
    hard_fails = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for p, rc, out, errout in ex.map(run, files):
            m = None
            for line in out.splitlines():
                mm = SAVE_RE.match(line)
                if mm:
                    m = mm
            if not m or rc != 0 or m.group(1) == "FAIL":
                hard_fails.append((p, "rc=%s %s %s" % (rc, (m.group(0) if m else "no summary"), errout.strip()[:200])))
                continue
            if m.group(2) is not None:
                tot["max3"] += int(m.group(2))
            elif m.group(3) is not None:
                tot["base"] += int(m.group(3))
                tot["mod"] += int(m.group(4))
                tot["max3"] += int(m.group(5))
                tot["fail"] += int(m.group(6))
    print("PM-MODIFY-SAVE: %d files, %d base, %d mod, %d max3-skip, %d fail"
          % (len(files), tot["base"], tot["mod"], tot["max3"], tot["fail"]), flush=True)

    for p, why in hard_fails[:40]:
        print("  FAIL %s : %s" % (p, why))

    ok = not hard_fails and not tot["fail"]
    if args.gate:
        if tot["base"] < args.min_base:
            print("GATE: base %d < floor %d" % (tot["base"], args.min_base)); ok = False
        if tot["mod"] < args.min_mod:
            print("GATE: mod %d < floor %d" % (tot["mod"], args.min_mod)); ok = False
    print("%s pm-encode-corpus" % ("OK" if ok else "FAIL"))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
