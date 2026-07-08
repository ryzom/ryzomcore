#!/usr/bin/env python3
"""Corpus driver for the ligo-brick .max -> .ig pipeline (build_gamedata processes/ligo's
exportInstanceGroupFromZone, as opposed to ig_corpus.py which drives the standalone processes/ig
ig_export.ms path).

Enumerates every ligo-source .max from the ryzomcore_graphics landscape/ligo/<eco>/max
directories (desert/jungle/lacustre/primes_racines — the same LigoMaxSourceDirectory listing
zone_corpus.py already drives), filtered to the zonematerial-*/zonespecial-*/zonetransition-*
protocol files, and runs test tiers:

  T1  structural roundtrip:  the pipeline_max_corpus_test binary, no --parse.
  T2  parse/build roundtrip: the pipeline_max_corpus_test binary with --parse.
  T3  .ig export:            pipeline_max_export_ig --ligo per file, then per exported ig,
        byte-compare against the raw intermediate export under --ref
        (~/pipeline_export/ecosystems/<eco>/ligo_es/igs/<name>.ig — the same 1_export-stage
        output ig_corpus.py's direct tier compares the standalone process against). A field
        compare with --mask-uninit (SunContribution/Light[]/light order — the same reference
        exporter uninitialized-memory classes ig_corpus.py already tolerates) classifies
        near-misses.

zonematerial/zonespecial export every distinct (lowercased) ig name in the file — same content
as the standalone process would produce for the same nodes, just lowercased and routed to the
ligo_es/igs output. zonetransition exports one ig per grid slot (0..8): only nodes whose ig name
matches that exact slot's zoneBaseName, repositioned via buildTransitionMatrixObj (see the tool's
main.cpp for the derivation — validated against all 9 grid slots across the 4-ecosystem corpus
during development, zero unexplained diffs).

A handful of "village"/"ville_zorai" bundle files carry dozens of distinct ig names in one .max
and take up to ~70s each (each name re-walks the scene through the existing, unmodified
buildInstanceGroup — legitimate additional work, not a regression); the per-file subprocess call
below has no built-in timeout so a full sweep still completes, just unevenly.

Known open diff classes (--gate-t3 budgets these; not silently passed — see DIFF_BUDGET):
  - Selection-order divergence on a subset of files (desert-nb01..05, jungle foret-18..21
    village_a/b/c/d and a couple of ilot_butte igs): our geometry/light/helper pass walks nodes
    in Scene-stream storage order; the reference walks Max's live per-category node array, which
    is usually the same order but can diverge on these specific files (same instance COUNT and
    NAME SET, different sequence — confirmed by diffing --info listings) for a reason not yet
    pinned. Same open-issue class as the shape exporter's "candide" duplicate-node-name note in
    pipeline_max_design.md section 10i.
  - A handful of accelerator cluster-membership mismatches (e.g. fy_mairie's decorative
    "_puit_carre" well meshes: reference says 1 cluster, ours says 0) — the vertex-in-volume
    clusterize link test producing a different containment verdict for a small number of
    borderline meshes; not yet root-caused.

Defaults are the layout on Kaetemi's machine; override via CLI when running elsewhere.
"""

import argparse, collections, os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_REF = os.path.expanduser("~/pipeline_export")
DEF_BIN = os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin")

ECOSYSTEMS = ["desert", "jungle", "lacustre", "primes_racines"]
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

# Known-open diff budget (see the module docstring). Regression guard: any count beyond this
# fails --gate-t3. Retire by fixing diffs and re-tightening — don't raise. Tightened 2026-07-08
# (§10w): 89 -> 28 after the Edit Mesh 0x0130 created-verts decode + tree-ordered per-category
# walk + ligo XRef-first pass. Remaining 28 = ~5 PS-instance clusterize AABBox corners landing
# within CLUSTERPRECISION=5mm of a cluster plane (x64/SSE vs x87, real content risk LOW — extra
# links harden culling, don't drop content) + ~23 village-bundle files (fy_module_village_nb_*,
# zo_imm_village_nb_*, tr_village_nb_*, jungle foret village_*, ilot_butte cases) where every
# node parents to the scene root so buildTreeOrder degenerates to storage order — cause not
# pinned without a Max-side $geometry probe on one of them.
DIFF_BUDGET = 28


def enumerate_corpus(graphics_dir):
    files = []
    for eco in ECOSYSTEMS:
        d = os.path.join(graphics_dir, "landscape", "ligo", eco, "max")
        if not os.path.isdir(d):
            continue
        for fn in sorted(os.listdir(d)):
            if not fn.endswith(".max"):
                continue
            base = fn[:-4]
            toks = base.split("-")
            if not ((len(toks) == 3 and toks[0] == "zonematerial")
                    or (len(toks) == 4 and toks[0] == "zonetransition")
                    or (len(toks) == 2 and toks[0] == "zonespecial")):
                continue
            path = os.path.join(d, fn)
            try:
                with open(path, "rb") as f:
                    if f.read(8) != OLE_MAGIC:
                        continue
            except OSError:
                continue
            files.append((eco, path))
    return files


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=DEF_GRAPHICS)
    ap.add_argument("--ref", default=DEF_REF)
    ap.add_argument("--bin", default=DEF_BIN)
    ap.add_argument("--out", default="/tmp/ligo_ig_corpus_out.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true", help="fail on T3 regressions")
    ap.add_argument("--only", default=None, help="substring filter on the .max path")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_ig")
    if not os.path.isdir(args.graphics):
        print("SKIP: asset checkout not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or not os.path.isfile(export_bin):
        print("SKIP: binaries not built")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics)
    if args.only:
        corpus = [c for c in corpus if args.only in c[1]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

    have_ref = os.path.isdir(args.ref)

    # .ps shape dirs for the clusterize FX-bbox path (export-era shapes first — same convention
    # as ig_corpus.py's direct tier).
    ps_paths = [d for d in (os.path.join(args.ref, "common", "sfx", "ps"),) if os.path.isdir(d)]

    t1_pass = t1_fail = t2_pass = t2_fail = 0
    exported = export_fail_n = nothing = 0
    export_fail = []
    direct_match = direct_near = direct_diff = 0
    noref = []
    diff_details = []
    field_counter = collections.Counter()

    os.makedirs(args.out, exist_ok=True)

    def process_file(indexed):
        idx, (eco, path) = indexed
        res = {"eco": eco, "path": path, "t1": None, "t2": None, "t3": None, "igs": []}
        if args.t1:
            res["t1"] = subprocess.run([corpus_bin, path], capture_output=True).returncode == 0
        if args.t2:
            res["t2"] = subprocess.run([corpus_bin, "--parse", path], capture_output=True).returncode == 0
        if args.t3:
            outdir = os.path.join(args.out, "f%d" % idx)
            os.makedirs(outdir, exist_ok=True)
            cmd = [export_bin, "--ligo", outdir, "--db", args.graphics]
            for pp in ps_paths:
                cmd += ["--ps-path", pp]
            r = subprocess.run(cmd + [path], capture_output=True, text=True)
            if r.returncode != 0:
                res["t3"] = ("exportfail", r.stderr[-500:] if r.stderr else "")
                return res
            names = sorted(f for f in os.listdir(outdir) if f.endswith(".ig"))
            if not names:
                res["t3"] = "nothing"
                return res
            res["t3"] = "exported"
            refdir = os.path.join(args.ref, "ecosystems", eco, "ligo_es", "igs")
            for name in names:
                ours = os.path.join(outdir, name)
                ig = {"name": name}
                refpath = os.path.join(refdir, name)
                if not os.path.isfile(refpath):
                    ig["status"] = ("noref", "")
                elif open(ours, "rb").read() == open(refpath, "rb").read():
                    ig["status"] = ("match", "")
                else:
                    r2 = subprocess.run([export_bin, "--compare", ours, refpath, "--mask-uninit"],
                                        capture_output=True, text=True)
                    ig["status"] = ("near" if r2.returncode == 0 else "diff", r2.stdout)
                res["igs"].append(ig)
        return res

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        results = list(ex.map(process_file, enumerate(corpus)))

    # Serial aggregation in submission order (deterministic output, matches ig_corpus.py /
    # anim_corpus.py's discipline for the same reason: parallel completion order must not affect
    # the printed report).
    seen_ig = set()
    for res in results:
        path = res["path"]
        if res["t1"] is not None:
            if res["t1"]:
                t1_pass += 1
            else:
                t1_fail += 1
                print("T1 FAIL %s" % path)
        if res["t2"] is not None:
            if res["t2"]:
                t2_pass += 1
            else:
                t2_fail += 1
                print("T2 FAIL %s" % path)
        t3 = res["t3"]
        if t3 == "nothing":
            nothing += 1
        elif isinstance(t3, tuple) and t3[0] == "exportfail":
            export_fail.append(path)
            print("T3 EXPORT FAIL %s" % path)
            sys.stdout.write(t3[1])
        elif t3 == "exported":
            exported += 1
            for ig in res["igs"]:
                key = (res["eco"], ig["name"])
                if key in seen_ig:
                    continue
                seen_ig.add(key)
                sk, sout = ig["status"]
                if sk == "match":
                    direct_match += 1
                elif sk == "near":
                    direct_near += 1
                elif sk == "noref":
                    noref.append("%s:%s" % (res["eco"], ig["name"]))
                else:
                    direct_diff += 1
                    fields = set()
                    for line in sout.splitlines():
                        for tok in line.split():
                            if "(" in tok:
                                fields.add(tok.split("(")[0])
                    for f in fields:
                        field_counter[f] += 1
                    diff_details.append((path, ig["name"], sorted(fields)))

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail" % (t1_pass, t1_fail, t2_pass, t2_fail))
    if args.t3:
        print("T3: %d files exported, %d export failures, %d files with nothing to export"
              % (exported, len(export_fail), nothing))
        if have_ref:
            print("    direct-ref: %d byte-identical, %d uninit-bytes-only, %d differ, %d without ref"
                  % (direct_match, direct_near, direct_diff, len(noref)))
            if field_counter:
                print("    diff fields: %s" % ", ".join("%s=%d" % kv for kv in field_counter.most_common()))
            if noref:
                print("    no reference found (first 10): %s" % ", ".join(noref[:10]))
        else:
            print("    (no --ref directory: byte-compare skipped)")
        for d in diff_details[:40]:
            print("    DIFF %s" % (d,))

    fails = t1_fail + t2_fail + len(export_fail)
    if args.gate_t3 and have_ref:
        if direct_diff > DIFF_BUDGET:
            fails += direct_diff - DIFF_BUDGET
        print("    diff budget: %d used / %d allowed" % (direct_diff, DIFF_BUDGET))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
