#!/usr/bin/env python3
"""Corpus driver for the veget .max -> .veget pipeline (build_gamedata processes/veget's
NelExportVegetable path).

Enumerates every .max under the workspace VegetSourceDirectories (landscape/microveget/<eco>
across the ecosystems that run the veget process — jungle/lacustre/primes_racines/desert) and
runs:

  T1  structural roundtrip:  pipeline_max_corpus_test, no --parse.
  T2  parse/build roundtrip: pipeline_max_corpus_test --parse.
  T3  .veget export:         pipeline_max_export_veget, byte-compare against
        ~/pipeline_export/ecosystems/<eco>/veget/<node>.veget.

Landing (2026-07-09, §10z-onze): first pass of the tool, size-match verified on the samples;
byte-identity present on the primes_racines sample, float-noise diffs on jungle. Whole-corpus
buckets are the interesting output.
"""

import argparse, os, re, subprocess, sys, collections
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

# workspace ecosystem name -> VegetSourceDirectories subfolder (they always match).
ECOSYSTEMS = ["jungle", "lacustre", "primes_racines", "desert"]


def is_ole(path):
    with open(path, "rb") as f:
        return f.read(8) == OLE_MAGIC


def enumerate_corpus(graphics_dir):
    files = []
    for eco in ECOSYSTEMS:
        d = os.path.join(graphics_dir, "landscape", "microveget", eco)
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
    ap.add_argument("--out", default="/tmp/veget_corpus_out.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true", help="fail on T3 regressions")
    ap.add_argument("--min-identical", type=int, default=0)
    ap.add_argument("--max-diff", type=int, default=1000000)
    ap.add_argument("--only", default=None, help="substring filter on the .max path")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_veget")
    if not os.path.isdir(args.graphics):
        print("SKIP: asset checkouts not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or (args.t3 and not os.path.isfile(export_bin)):
        print("SKIP: binaries not built")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics)
    if args.only:
        corpus = [c for c in corpus if args.only in c[1]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

    t1_pass = t1_fail = t2_pass = t2_fail = stubs = 0
    export_fail = []

    os.makedirs(args.out, exist_ok=True)

    def run_file(item):
        eco, path = item
        res = {"eco": eco, "path": path}
        if not is_ole(path):
            res["stub"] = True
            return res
        if args.t1:
            r = subprocess.run([corpus_bin, path], capture_output=True)
            res["t1"] = r.returncode == 0
        if args.t2:
            r = subprocess.run([corpus_bin, "--parse", path], capture_output=True)
            res["t2"] = r.returncode == 0
        if args.t3:
            # Per-FILE outdir: the old shared per-eco outdir with before/after dir-listing
            # snapshots raced under -j (another thread's outputs landed between the snapshots
            # and were attributed to several files at once — parallel sweeps reported 6x the
            # serial exported count, nondeterministically). One export call per .max means a
            # per-file dir needs no snapshot at all.
            outdir = os.path.join(args.out, eco, os.path.splitext(os.path.basename(path))[0])
            os.makedirs(outdir, exist_ok=True)
            r = subprocess.run([export_bin, "--db", args.graphics, path, outdir],
                               capture_output=True, text=True)
            res["t3rc"] = r.returncode
            res["t3out"] = r.stdout
            res["t3err"] = r.stderr
            new = []
            for f in os.listdir(outdir):
                if f.endswith(".veget"):
                    new.append(os.path.join(outdir, f))
            res["t3new"] = new
        return res

    results = []
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for res in ex.map(run_file, corpus):
            results.append(res)

    identical = size_eq = size_diff = us_only = 0
    ref_missing = 0
    diff_samples = []
    for res in results:
        if res.get("stub"):
            stubs += 1
            continue
        if args.t1:
            if res.get("t1"): t1_pass += 1
            else: t1_fail += 1; print("T1 FAIL %s" % res["path"])
        if args.t2:
            if res.get("t2"): t2_pass += 1
            else: t2_fail += 1; print("T2 FAIL %s" % res["path"])
        if args.t3:
            if res.get("t3rc") != 0:
                export_fail.append(res["path"])
                print("T3 EXPORT FAIL %s" % res["path"])
                sys.stdout.write((res.get("t3err") or "")[-500:])
            for outp in res.get("t3new") or []:
                bn = os.path.basename(outp)
                # Ref location: ~/pipeline_export/ecosystems/<eco>/veget/<bn>
                ref = os.path.join(args.ref, "ecosystems", res["eco"], "veget", bn)
                if not os.path.isfile(ref):
                    us_only += 1
                    continue
                a = open(outp, "rb").read()
                b = open(ref, "rb").read()
                if a == b:
                    identical += 1
                elif len(a) == len(b):
                    size_eq += 1
                    if len(diff_samples) < 15:
                        # count byte diffs
                        d = sum(1 for i in range(len(a)) if a[i] != b[i])
                        diff_samples.append("SIZE_EQ %s (%d/%d bytes)" % (bn, d, len(a)))
                else:
                    size_diff += 1
                    if len(diff_samples) < 15:
                        diff_samples.append("SIZE_DIFF %s (%d vs %d)" % (bn, len(a), len(b)))

    # Count missing references (files we should have exported but didn't)
    if args.t3:
        exported_names = set()
        for res in results:
            for p in res.get("t3new") or []:
                exported_names.add(os.path.basename(p))
        for eco in set(r["eco"] for r in results if not r.get("stub")):
            refdir = os.path.join(args.ref, "ecosystems", eco, "veget")
            if not os.path.isdir(refdir):
                continue
            for f in os.listdir(refdir):
                if f.endswith(".veget") and f not in exported_names:
                    ref_missing += 1

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail; %d stubs" % (t1_pass, t1_fail, t2_pass, t2_fail, stubs))
    if args.t3:
        total = identical + size_eq + size_diff + us_only
        print("T3: %d exported: %d byte-identical, %d size-eq (byte-diff), %d size-diff, %d without reference; %d references not produced"
              % (total, identical, size_eq, size_diff, us_only, ref_missing))
        print("    export failures: %d" % len(export_fail))
        for s in diff_samples:
            print("    %s" % s)

    fails = t1_fail + t2_fail + len(export_fail)
    if args.gate_t3:
        if identical + size_eq < args.min_identical:
            fails += 1
        fails += max(0, size_diff - args.max_diff)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
