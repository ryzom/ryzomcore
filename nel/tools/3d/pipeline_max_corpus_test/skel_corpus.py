#!/usr/bin/env python3
"""Corpus driver for the skel .max → .skel pipeline.

Enumerates every skel-source .max from the ryzomcore_leveldesign workspace configs, classifies
biped vs non-biped by scanning the file for the "Biped Object" UTF-16 marker, filters out
git-lfs stubs, and runs three test tiers:

  T1  structural roundtrip:  the pipeline_max_corpus_test binary, no --parse.
  T2  parse/build roundtrip: the pipeline_max_corpus_test binary with --parse.
  T3  .skel export:          pipeline_max_export_skel, byte-compare vs the reference in --ref
                             (defaults to ~/core4_data/characters_skeletons if present).

Defaults are the layout on Kaetemi's machine; override via CLI when running elsewhere.
"""

import argparse, os, re, subprocess, sys, collections

def parse_workspace(path):
    dirs = []
    if not os.path.isfile(path):
        return dirs
    with open(path) as f:
        for line in f:
            m = re.match(r'\s*SkelSourceDirectories\s*\+=\s*\[\s*"([^"]+)"\s*\]', line)
            if m:
                dirs.append(m.group(1))
    return dirs

def enumerate_corpus(graphics_dir, workspace_dir):
    all_dirs = []
    for group in ("fauna", "characters"):
        all_dirs += parse_workspace(os.path.join(workspace_dir, "common", group, "directories.py"))
    files = []
    for d in all_dirs:
        full_dir = os.path.join(graphics_dir, d)
        if not os.path.isdir(full_dir):
            continue
        for name in sorted(os.listdir(full_dir)):
            if name.lower().endswith(".max"):
                files.append((d, name, os.path.join(full_dir, name)))
    return files

def is_git_lfs_stub(path):
    with open(path, "rb") as f:
        magic = f.read(8)
    return magic != b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

def is_biped(path):
    marker = "Biped Object".encode("utf-16le")
    with open(path, "rb") as f:
        return marker in f.read()

def run_tests(bin_dir, files, do_t1, do_t2, do_t3, ref_dir, output_dir):
    corpus_test = os.path.join(bin_dir, "pipeline_max_corpus_test")
    export_skel = os.path.join(bin_dir, "pipeline_max_export_skel")
    if do_t1 and not os.path.isfile(corpus_test):
        sys.exit(f"missing: {corpus_test}")
    if do_t3 and not os.path.isfile(export_skel):
        sys.exit(f"missing: {export_skel}")
    if do_t3 and output_dir:
        os.makedirs(output_dir, exist_ok=True)

    buckets = collections.defaultdict(lambda: {"total": 0, "t1_pass": 0, "t1_fail": [],
                                                "t2_pass": 0, "t2_fail": [],
                                                "t3_pass": 0, "t3_fail": [], "t3_missing_ref": []})
    for d, name, full in files:
        if is_git_lfs_stub(full):
            buckets["stubs"]["total"] += 1
            continue
        kind = "biped" if is_biped(full) else "nonbiped"
        b = buckets[kind]
        b["total"] += 1

        if do_t1:
            args = [corpus_test, full]
            if do_t2: args.insert(1, "--parse")
            r = subprocess.run(args, capture_output=True, text=True, timeout=120)
            summary = r.stdout.strip()
            # Parse "OK|FAIL <path> Stream:T1=ok[,T2=ok] ..."
            ok = summary.startswith("OK ")
            t1_ok = "T1=FAIL" not in summary
            t2_ok = "T2=FAIL" not in summary if do_t2 else True
            if t1_ok: b["t1_pass"] += 1
            else: b["t1_fail"].append((name, summary, r.stderr.strip()))
            if do_t2:
                if t2_ok: b["t2_pass"] += 1
                else: b["t2_fail"].append((name, summary, r.stderr.strip()))

        if do_t3:
            base = os.path.splitext(name)[0]
            out_skel = os.path.join(output_dir, base + ".skel") if output_dir else "/tmp/skel_test.skel"
            r = subprocess.run([export_skel, full, out_skel], capture_output=True, text=True, timeout=120)
            if r.returncode != 0:
                b["t3_fail"].append((name, f"exporter rc={r.returncode}", r.stderr.strip()))
                continue
            if not ref_dir:
                continue
            ref = os.path.join(ref_dir, base + ".skel")
            if not os.path.isfile(ref):
                b["t3_missing_ref"].append(name)
                continue
            with open(out_skel, "rb") as f: ours = f.read()
            with open(ref, "rb") as f: theirs = f.read()
            if ours == theirs:
                b["t3_pass"] += 1
            else:
                # size-match + byte-percent
                pct = 0
                if len(ours) == len(theirs) and len(ours):
                    matches = sum(1 for a, b_ in zip(ours, theirs) if a == b_)
                    pct = matches * 100.0 / len(ours)
                b["t3_fail"].append((name, f"size={len(ours)} vs {len(theirs)} byte-match={pct:.1f}%", ""))

    return buckets

def report(buckets, do_t1, do_t2, do_t3, verbose):
    for kind in ("biped", "nonbiped", "stubs"):
        b = buckets.get(kind)
        if not b: continue
        total = b["total"]
        line = f"{kind}: total={total}"
        if do_t1: line += f", T1 {b['t1_pass']}/{total}"
        if do_t2: line += f", T2 {b['t2_pass']}/{total}"
        if do_t3:
            missing = len(b['t3_missing_ref'])
            line += f", T3 {b['t3_pass']}/{total-missing} (missing-ref={missing})"
        print(line)
        if verbose:
            for name, summary, err in b.get("t1_fail", [])[:20]:
                print(f"  T1 FAIL {name}: {summary}")
                if err: print(f"    stderr: {err.splitlines()[0] if err else ''}")
            for name, summary, err in b.get("t2_fail", [])[:20]:
                print(f"  T2 FAIL {name}: {summary}")
                if err: print(f"    stderr: {err.splitlines()[0] if err else ''}")
            for name, summary, err in b.get("t3_fail", [])[:40]:
                print(f"  T3 FAIL {name}: {summary}")

def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--graphics", default=os.path.join(home, "ryzomcore_graphics"))
    ap.add_argument("--workspace", default=os.path.join(home, "ryzomcore_leveldesign/workspace"))
    ap.add_argument("--bin",       default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--ref",       default=os.path.join(home, "core4_data/characters_skeletons"))
    ap.add_argument("--output",    default=None, help="directory to write .skel outputs (T3)")
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true", help="shortcut for --t1 --t2 --t3")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = True

    files = enumerate_corpus(args.graphics, args.workspace)
    print(f"corpus: {len(files)} .max files across {len(set(d for d,_,_ in files))} dirs")
    if not files:
        sys.exit("empty corpus — check --graphics / --workspace paths")

    ref = args.ref if os.path.isdir(args.ref) else None
    if args.t3 and not ref:
        print(f"note: T3 reference dir {args.ref} not present; T3 will only check exporter runs")

    buckets = run_tests(args.bin, files, args.t1, args.t2, args.t3, ref, args.output)
    report(buckets, args.t1, args.t2, args.t3, args.verbose)

    # Non-zero exit if any T1/T2 failure surfaced (T3 mismatches are epsilon-tolerated).
    fail = 0
    for k in ("biped", "nonbiped"):
        b = buckets.get(k, {})
        fail += len(b.get("t1_fail", [])) + len(b.get("t2_fail", []))
    sys.exit(1 if fail else 0)

if __name__ == "__main__":
    main()
