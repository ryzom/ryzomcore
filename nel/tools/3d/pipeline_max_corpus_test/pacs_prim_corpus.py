#!/usr/bin/env python3
"""Corpus driver for the pacs_prim .max -> .pacs_prim pipeline (build_gamedata
processes/pacs_prim's NelExportPACSPrimitives path).

Enumerates every .max under PacsPrimSourceDirectories (ecosystems/<eco>/directories.py:
DatabaseRootPath + "/decors/vegetations", i.e. stuff/<race>/decors/vegetations for the
desert/jungle/lacustre/primes_racines ecosystems) and runs:

  T1  structural roundtrip:  pipeline_max_corpus_test, no --parse.
  T2  parse/build roundtrip: pipeline_max_corpus_test --parse.
  T3  .pacs_prim export:     pipeline_max_export_pacs_prim, byte-compare against
        ~/pipeline_export/ecosystems/<eco>/pacs_prim/<name>.pacs_prim. A source with zero PACS
        primitives (exit code 3, no output) is expected to have no reference either.

Landing state (2026-07-08): 509 corpus files, 493 with PACS primitives, all 493 byte-identical
against the reference (0 diffs) — see pipeline_max_design.md.
"""

import argparse, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77

ECO_DIRS = {
    "fyros": "desert",
    "jungle": "jungle",
    "tryker": "lacustre",
    "primes_racines": "primes_racines",
}


def enumerate_corpus(graphics_dir):
    files = []
    for race, eco in ECO_DIRS.items():
        d = os.path.join(graphics_dir, "stuff", race, "decors", "vegetations")
        if not os.path.isdir(d):
            continue
        for fn in sorted(os.listdir(d)):
            if fn.lower().endswith(".max"):
                files.append((eco, os.path.join(d, fn)))
    return files


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--graphics", default=os.path.expanduser("~/ryzomcore_graphics"))
    ap.add_argument("--ref", default=os.path.expanduser("~/pipeline_export"))
    ap.add_argument("--bin", default=os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_pacs_prim")
    if not os.path.isdir(args.graphics):
        print("SKIP: asset checkout not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or (args.t3 and not os.path.isfile(export_bin)):
        print("SKIP: binaries not built")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics)
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE
    print("corpus: %d pacs_prim-source .max files" % len(corpus))

    have_ref = os.path.isdir(args.ref)

    def process_file(indexed):
        idx, (eco, path) = indexed
        res = {"eco": eco, "path": path, "t1": None, "t2": None, "t3": None}
        if args.t1:
            res["t1"] = subprocess.run([corpus_bin, path], capture_output=True).returncode == 0
        if args.t2:
            res["t2"] = subprocess.run([corpus_bin, "--parse", path], capture_output=True).returncode == 0
        if args.t3:
            base = os.path.splitext(os.path.basename(path))[0]
            out = "/tmp/pacs_prim_corpus.%d.%s.pacs_prim" % (os.getpid(), base)
            r = subprocess.run([export_bin, path, out], capture_output=True, text=True)
            if r.returncode == 3:
                res["t3"] = ("nothing", None)
            elif r.returncode != 0:
                res["t3"] = ("exportfail", r.stderr[-300:] if r.stderr else "")
            else:
                ref = os.path.join(args.ref, "ecosystems", eco, "pacs_prim", base + ".pacs_prim")
                if not os.path.isfile(ref):
                    res["t3"] = ("noref", None)
                elif open(out, "rb").read() == open(ref, "rb").read():
                    res["t3"] = ("match", None)
                else:
                    res["t3"] = ("diff", None)
                try:
                    os.unlink(out)
                except OSError:
                    pass
        return res

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        results = list(ex.map(process_file, enumerate(corpus)))

    t1_pass = t1_fail = t2_pass = t2_fail = 0
    nothing = export_fail = match = noref = diff = 0
    diffs = []
    for res in results:
        if res["t1"] is not None:
            if res["t1"]:
                t1_pass += 1
            else:
                t1_fail += 1
                print("T1 FAIL %s" % res["path"])
        if res["t2"] is not None:
            if res["t2"]:
                t2_pass += 1
            else:
                t2_fail += 1
                print("T2 FAIL %s" % res["path"])
        t3 = res["t3"]
        if t3 is None:
            continue
        kind, detail = t3
        if kind == "nothing":
            nothing += 1
        elif kind == "exportfail":
            export_fail += 1
            print("T3 EXPORT FAIL %s: %s" % (res["path"], detail))
        elif kind == "match":
            match += 1
        elif kind == "noref":
            noref += 1
        else:
            diff += 1
            diffs.append(res["path"])
            print("T3 DIFF %s" % res["path"])

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail" % (t1_pass, t1_fail, t2_pass, t2_fail))
    if args.t3:
        print("T3: %d nothing-to-export, %d export failures" % (nothing, export_fail))
        if have_ref:
            print("    %d byte-identical, %d differ, %d without ref" % (match, diff, noref))
        else:
            print("    (no --ref directory: byte-compare skipped)")

    fails = t1_fail + t2_fail + export_fail
    if args.gate_t3 and have_ref:
        fails += diff
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
