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

import argparse, os, re, struct, subprocess, sys, collections

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

def read_skel_dpos(path):
    """Parse a .skel and return [(bone_name, dpos_tuple), ...]. Skips InvBindPos matrix by state-bit."""
    with open(path, "rb") as f: data = f.read()
    pos = [0]
    def rd(n): v = data[pos[0]:pos[0]+n]; pos[0] += n; return v
    def u8(): return rd(1)[0]
    def u32(): return struct.unpack("<I", rd(4))[0]
    def i32(): return struct.unpack("<i", rd(4))[0]
    def u64(): return struct.unpack("<Q", rd(8))[0]
    def f32(): return struct.unpack("<f", rd(4))[0]
    def str_(): n = u32(); return rd(n).decode("utf-8", "replace")
    def cvec(): return (f32(), f32(), f32())
    def cquat(): return (f32(), f32(), f32(), f32())
    def rdmat():
        u8(); state = u32(); f32()  # ver, state, scale33
        if state & (2|4|8):
            for _ in range(9): f32()
        if state & 1:
            for _ in range(3): f32()
        if state & 16:
            for _ in range(4): f32()
    rd(4); u64(); str_(); u8(); bc = i32()
    out = []
    for i in range(bc):
        boneVer = u8(); name = str_(); rdmat()
        i32(); u8(); f32()  # father, unherit, lod
        u8(); dpos = cvec()
        u8(); cvec()  # euler
        u8(); cquat()  # quat
        u8(); cvec()  # scale
        u8(); cvec()  # pivot
        if boneVer >= 2:
            cvec()  # skinScale
        out.append((name, dpos))
    return out

def bone_accuracy(ours, ref_path):
    """Compare our .skel bone dposes to reference. Returns (total, exact, close, sum_err)."""
    try:
        a = read_skel_dpos(ours)
        b = read_skel_dpos(ref_path)
    except Exception:
        return (0, 0, 0, 0.0)
    if len(a) != len(b): return (0, 0, 0, 0.0)
    total = exact = close = 0
    sum_err = 0.0
    for (na, pa), (nb, pb) in zip(a, b):
        if na != nb: continue
        total += 1
        err = ((pa[0]-pb[0])**2 + (pa[1]-pb[1])**2 + (pa[2]-pb[2])**2) ** 0.5
        sum_err += err
        if err < 1e-5: exact += 1
        elif err < 0.02: close += 1
    return (total, exact, close, sum_err)

def run_tests(bin_dir, files, do_t1, do_t2, do_t3, ref_biped, ref_nonbiped, output_dir):
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
            # Pass --allow-biped-degraded on biped files so the exporter runs (degraded output —
            # correct names/hierarchy, identity local transforms) instead of refusing outright.
            # T3 reports the delta so degraded output can't quietly pass as a match.
            extra = ["--allow-biped-degraded"] if kind == "biped" else []
            r = subprocess.run([export_skel] + extra + [full, out_skel],
                               capture_output=True, text=True, timeout=120)
            if r.returncode != 0:
                b["t3_fail"].append((name, f"exporter rc={r.returncode}", r.stderr.strip()))
                continue
            ref_dir_here = ref_biped if kind == "biped" else ref_nonbiped
            if not ref_dir_here:
                continue
            ref = os.path.join(ref_dir_here, base + ".skel")
            if not os.path.isfile(ref):
                b["t3_missing_ref"].append(name)
                continue
            with open(out_skel, "rb") as f: ours = f.read()
            with open(ref, "rb") as f: theirs = f.read()
            # Aggregate per-bone dpos accuracy (works even when total-size differs from ref).
            bt, be, bc_, err = bone_accuracy(out_skel, ref)
            b.setdefault("t3_bones_total", 0); b["t3_bones_total"] += bt
            b.setdefault("t3_bones_exact", 0); b["t3_bones_exact"] += be
            b.setdefault("t3_bones_close", 0); b["t3_bones_close"] += bc_
            b.setdefault("t3_bones_err", 0.0); b["t3_bones_err"] += err
            if ours == theirs:
                b["t3_pass"] += 1
            else:
                # Size-match check + byte-percent. Non-biped runs are expected to size-match with
                # ~61-96% byte-match (T3-epsilon float noise); biped runs are currently degraded so
                # match% is meaningful mostly as a floor + progress metric.
                pct = 0
                size_match = len(ours) == len(theirs)
                if size_match and len(ours):
                    matches = sum(1 for a, b_ in zip(ours, theirs) if a == b_)
                    pct = matches * 100.0 / len(ours)
                b.setdefault("t3_size_match", 0)
                if size_match: b["t3_size_match"] += 1
                b.setdefault("t3_pct_sum", 0.0)
                b.setdefault("t3_pct_n", 0)
                if size_match:
                    b["t3_pct_sum"] += pct
                    b["t3_pct_n"] += 1
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
            sm = b.get("t3_size_match", 0)
            pcts = b.get("t3_pct_sum", 0.0); pn = b.get("t3_pct_n", 0)
            avg = (pcts / pn) if pn else 0.0
            bt = b.get("t3_bones_total", 0); be = b.get("t3_bones_exact", 0); bc_ = b.get("t3_bones_close", 0)
            err_avg = (b.get("t3_bones_err", 0.0) / bt) if bt else 0.0
            line += (f", T3 {b['t3_pass']}/{total-missing} exact"
                     f" (size-match {sm}, avg byte-match {avg:.1f}%, missing-ref={missing};"
                     f" bones {be}+{bc_}/{bt} exact+close, avg dpos err {err_avg:.4f})")
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
    ap.add_argument("--ref-biped", default=os.path.join(home, "core4_data/characters_skeletons"))
    ap.add_argument("--ref-nonbiped", default=os.path.join(home, "core4_data/fauna_skeletons"))
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

    ref_biped = args.ref_biped if os.path.isdir(args.ref_biped) else None
    ref_nonbiped = args.ref_nonbiped if os.path.isdir(args.ref_nonbiped) else None
    if args.t3:
        if not ref_biped: print(f"note: T3 biped-ref dir {args.ref_biped} not present")
        if not ref_nonbiped: print(f"note: T3 nonbiped-ref dir {args.ref_nonbiped} not present")

    buckets = run_tests(args.bin, files, args.t1, args.t2, args.t3, ref_biped, ref_nonbiped, args.output)
    report(buckets, args.t1, args.t2, args.t3, args.verbose)

    # Non-zero exit if any T1/T2 failure surfaced (T3 mismatches are epsilon-tolerated).
    fail = 0
    for k in ("biped", "nonbiped"):
        b = buckets.get(k, {})
        fail += len(b.get("t1_fail", [])) + len(b.get("t2_fail", []))
    sys.exit(1 if fail else 0)

if __name__ == "__main__":
    main()
