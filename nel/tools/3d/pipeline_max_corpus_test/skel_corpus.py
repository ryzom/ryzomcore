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
from concurrent.futures import ThreadPoolExecutor

# Autotools/CTest skip convention: exit 77 means "test could not run here", distinct from a real
# failure (exit 1). Used when the private asset checkouts (ryzomcore_graphics, core4_data,
# ryzomcore_leveldesign) or the built binaries aren't present — e.g. any CI/dev machine that
# doesn't have Kaetemi's local asset checkouts. The CMake `add_test()` registration for this
# script sets the SKIP_RETURN_CODE test property to 77 to match.
SKIP_CODE = 77

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

def read_skel_bones(path):
    """Parse a .skel fully into per-bone dicts. Every float is returned both as a Python float
    and as its raw uint32 bit pattern (key + '_b'), so callers can do bit-exact and ULP-level
    comparisons, not just epsilon comparisons."""
    with open(path, "rb") as f: data = f.read()
    pos = [0]
    def rd(n): v = data[pos[0]:pos[0]+n]; pos[0] += n; return v
    def u8(): return rd(1)[0]
    def u32(): return struct.unpack("<I", rd(4))[0]
    def i32(): return struct.unpack("<i", rd(4))[0]
    def u64(): return struct.unpack("<Q", rd(8))[0]
    def f32b():
        raw = rd(4)
        return struct.unpack("<f", raw)[0], struct.unpack("<I", raw)[0]
    def str_(): n = u32(); return rd(n).decode("utf-8", "replace")
    def vecb(n):
        fs, bs = [], []
        for _ in range(n):
            f, b = f32b(); fs.append(f); bs.append(b)
        return tuple(fs), tuple(bs)
    rd(4); u64(); str_(); u8(); bc = i32()
    out = []
    for i in range(bc):
        bone = {}
        boneVer = u8(); bone["name"] = str_()
        # InvBindPos: CMatrix = ver u8, StateBit u32, Scale33 f32, then rot 3x3 / trans / proj
        # by state bits (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY -> 9, MAT_TRANS -> 3, MAT_PROJ -> 4).
        u8(); state = u32()
        bone["ibp_state"] = state
        bone["ibp_scale33"], bone["ibp_scale33_b"] = f32b()
        rot = ((), ()); trans = ((), ()); proj = ((), ())
        if state & (2|4|8): rot = vecb(9)
        if state & 1: trans = vecb(3)
        if state & 16: proj = vecb(4)
        bone["ibp_rot"], bone["ibp_rot_b"] = rot
        bone["ibp_trans"], bone["ibp_trans_b"] = trans
        bone["ibp_proj"], bone["ibp_proj_b"] = proj
        bone["father"] = i32()
        bone["unherit"] = u8()
        bone["lod"], bone["lod_b"] = f32b()
        u8(); bone["dpos"], bone["dpos_b"] = vecb(3)
        u8(); bone["euler"], bone["euler_b"] = vecb(3)
        u8(); bone["drot"], bone["drot_b"] = vecb(4)
        u8(); bone["dscale"], bone["dscale_b"] = vecb(3)
        u8(); bone["pivot"], bone["pivot_b"] = vecb(3)
        if boneVer >= 2:
            bone["skinscale"], bone["skinscale_b"] = vecb(3)
        else:
            bone["skinscale"], bone["skinscale_b"] = (1.0, 1.0, 1.0), None
        out.append(bone)
    return out

def read_skel_dpos(path):
    """Back-compat wrapper: [(bone_name, dpos_tuple, drot_tuple), ...]."""
    return [(b["name"], b["dpos"], b["drot"]) for b in read_skel_bones(path)]

def ulp_dist(a_bits, b_bits):
    """ULP distance between two float32 bit patterns (monotonic IEEE754 key; +0 == -0)."""
    def key(b):
        return b if b < 0x80000000 else 0x80000000 - b
    return abs(key(a_bits) - key(b_bits))

def quat_dist(a, b):
    """Quaternion distance accounting for double-cover (q == -q)."""
    d1 = sum((a[i]-b[i])**2 for i in range(4)) ** 0.5
    d2 = sum((a[i]+b[i])**2 for i in range(4)) ** 0.5
    return min(d1, d2)

ULP_BUCKETS = (0, 1, 2, 4, 16, 256)  # histogram edges; ">256" is the overflow bucket

def bone_accuracy(ours, ref_path):
    """Compare our .skel bones to the reference at float level, across EVERY stored field.
    Epsilon buckets (dpos/drot exact/close) keep their historical meaning; the *_bit counters
    are strict bit-identity per bone; ibp_* quantifies InvBindPos per element in ULPs."""
    zero = dict(total=0, dp_exact=0, dp_close=0, dp_err=0.0, dr_exact=0, dr_close=0, dr_err=0.0,
                dp_bit=0, dr_bit=0, scale_bit=0, lod_bit=0, father_ok=0, unherit_ok=0,
                aux_bit=0,  # euler + pivot + skinScale + ibp scale33 all bit-exact
                ibp_state_ok=0, ibp_bit=0, ibp_elems=0,
                ibp_ulp_hist=[0]*(len(ULP_BUCKETS)+1), ibp_ulp_max=0)
    try:
        a = read_skel_bones(ours)
        b = read_skel_bones(ref_path)
    except Exception:
        return dict(zero, ibp_ulp_hist=list(zero["ibp_ulp_hist"]))
    if len(a) != len(b): return dict(zero, ibp_ulp_hist=list(zero["ibp_ulp_hist"]))
    r = dict(zero, ibp_ulp_hist=list(zero["ibp_ulp_hist"]))
    for ba, bb in zip(a, b):
        if ba["name"] != bb["name"]: continue
        r["total"] += 1
        pa, pb = ba["dpos"], bb["dpos"]
        err = ((pa[0]-pb[0])**2 + (pa[1]-pb[1])**2 + (pa[2]-pb[2])**2) ** 0.5
        r["dp_err"] += err
        if err < 1e-5: r["dp_exact"] += 1
        elif err < 0.02: r["dp_close"] += 1
        qerr = quat_dist(ba["drot"], bb["drot"])
        r["dr_err"] += qerr
        if qerr < 1e-3: r["dr_exact"] += 1
        elif qerr < 0.02: r["dr_close"] += 1
        # strict bit-identity per field (drot also accepts the negated quat: same rotation,
        # and the reference exporter emits either sign)
        if ba["dpos_b"] == bb["dpos_b"]: r["dp_bit"] += 1
        negb = tuple(x ^ 0x80000000 for x in bb["drot_b"])
        if ba["drot_b"] == bb["drot_b"] or ba["drot_b"] == negb: r["dr_bit"] += 1
        if ba["dscale_b"] == bb["dscale_b"]: r["scale_bit"] += 1
        if ba["lod_b"] == bb["lod_b"]: r["lod_bit"] += 1
        if ba["father"] == bb["father"]: r["father_ok"] += 1
        if ba["unherit"] == bb["unherit"]: r["unherit_ok"] += 1
        if (ba["euler_b"] == bb["euler_b"] and ba["pivot_b"] == bb["pivot_b"]
                and ba["skinscale_b"] == bb["skinscale_b"]
                and ba["ibp_scale33_b"] == bb["ibp_scale33_b"]):
            r["aux_bit"] += 1
        # InvBindPos: state bits must agree for element comparison to be meaningful
        if ba["ibp_state"] == bb["ibp_state"]:
            r["ibp_state_ok"] += 1
            ea = ba["ibp_rot_b"] + ba["ibp_trans_b"] + ba["ibp_proj_b"]
            eb = bb["ibp_rot_b"] + bb["ibp_trans_b"] + bb["ibp_proj_b"]
            bone_bit = True
            for x, y in zip(ea, eb):
                r["ibp_elems"] += 1
                u = ulp_dist(x, y)
                if u: bone_bit = False
                if u > r["ibp_ulp_max"]: r["ibp_ulp_max"] = u
                for k, edge in enumerate(ULP_BUCKETS):
                    if u <= edge:
                        r["ibp_ulp_hist"][k] += 1
                        break
                else:
                    r["ibp_ulp_hist"][len(ULP_BUCKETS)] += 1
            if bone_bit: r["ibp_bit"] += 1
    return r

def run_tests(bin_dir, files, do_t1, do_t2, do_t3, ref_biped, ref_nonbiped, output_dir, jobs=1):
    corpus_test = os.path.join(bin_dir, "pipeline_max_corpus_test")
    export_skel = os.path.join(bin_dir, "pipeline_max_export_skel")
    if (do_t1 or do_t2) and not os.path.isfile(corpus_test):
        print(f"SKIP: missing binary {corpus_test} (build it first)")
        sys.exit(SKIP_CODE)
    if do_t3 and not os.path.isfile(export_skel):
        print(f"SKIP: missing binary {export_skel} (build it first)")
        sys.exit(SKIP_CODE)
    if do_t3 and output_dir:
        os.makedirs(output_dir, exist_ok=True)

    buckets = collections.defaultdict(lambda: {"total": 0, "t1_pass": 0, "t1_fail": [],
                                                "t2_pass": 0, "t2_fail": [],
                                                "t3_pass": 0, "t3_fail": [], "t3_missing_ref": []})
    # Parallel pre-pass: run each file's subprocesses (T1/T2 corpus_test, T3 export) concurrently
    # and cache the raw results; the aggregation loop below stays serial and byte-identical, just
    # reading from the cache instead of shelling out. Each file exports to its own path so parallel
    # runs never collide.
    def _prep(indexed):
        idx, (d, name, full) = indexed
        if is_git_lfs_stub(full):
            return {"stub": True}
        pr = {"stub": False, "kind": "biped" if is_biped(full) else "nonbiped"}
        if do_t1 or do_t2:
            a = [corpus_test, full]
            if do_t2: a.insert(1, "--parse")
            r = subprocess.run(a, capture_output=True, text=True, timeout=120)
            pr["t12_out"] = r.stdout.strip(); pr["t12_err"] = r.stderr.strip()
        if do_t3:
            base = os.path.splitext(name)[0]
            out_skel = os.path.join(output_dir, base + ".skel") if output_dir else ("/tmp/skel_test_%d.skel" % idx)
            r = subprocess.run([export_skel, full, out_skel], capture_output=True, text=True, timeout=120)
            pr["t3_rc"] = r.returncode; pr["t3_err"] = r.stderr.strip(); pr["out_skel"] = out_skel
        return pr

    with ThreadPoolExecutor(max_workers=max(1, jobs)) as ex:
        prepped = list(ex.map(_prep, enumerate(files)))

    for idx, (d, name, full) in enumerate(files):
        pr = prepped[idx]
        if pr["stub"]:
            buckets["stubs"]["total"] += 1
            continue
        kind = pr["kind"]
        b = buckets[kind]
        b["total"] += 1

        if do_t1 or do_t2:
            summary = pr["t12_out"]
            # Parse "OK|FAIL <path> Stream:T1=ok[,T2=ok] ..."
            ok = summary.startswith("OK ")
            t1_ok = "T1=FAIL" not in summary
            t2_ok = "T2=FAIL" not in summary if do_t2 else True
            if t1_ok: b["t1_pass"] += 1
            else: b["t1_fail"].append((name, summary, pr["t12_err"]))
            if do_t2:
                if t2_ok: b["t2_pass"] += 1
                else: b["t2_fail"].append((name, summary, pr["t12_err"]))

        if do_t3:
            base = os.path.splitext(name)[0]
            out_skel = pr["out_skel"]
            if pr["t3_rc"] != 0:
                b["t3_fail"].append((name, f"exporter rc={pr['t3_rc']}", pr["t3_err"]))
                continue
            # A reference may live in either output set regardless of biped-ness: the biped kami
            # and degenerate-homin rigs are fauna, the humanoids are characters. Check both.
            ref = None
            for ref_dir_here in (ref_biped, ref_nonbiped):
                if not ref_dir_here:
                    continue
                cand = os.path.join(ref_dir_here, base + ".skel")
                if os.path.isfile(cand):
                    ref = cand
                    break
            if not ref:
                b["t3_missing_ref"].append(name)
                continue
            with open(out_skel, "rb") as f: ours = f.read()
            with open(ref, "rb") as f: theirs = f.read()
            # Aggregate per-bone dpos + drot accuracy (works even when total-size differs from ref).
            acc = bone_accuracy(out_skel, ref)
            b.setdefault("t3_bones_total", 0); b["t3_bones_total"] += acc["total"]
            b.setdefault("t3_bones_exact", 0); b["t3_bones_exact"] += acc["dp_exact"]
            b.setdefault("t3_bones_close", 0); b["t3_bones_close"] += acc["dp_close"]
            b.setdefault("t3_bones_err", 0.0); b["t3_bones_err"] += acc["dp_err"]
            b.setdefault("t3_drot_exact", 0); b["t3_drot_exact"] += acc["dr_exact"]
            b.setdefault("t3_drot_close", 0); b["t3_drot_close"] += acc["dr_close"]
            b.setdefault("t3_drot_err", 0.0); b["t3_drot_err"] += acc["dr_err"]
            for k in ("dp_bit", "dr_bit", "scale_bit", "lod_bit", "father_ok", "unherit_ok",
                      "aux_bit", "ibp_state_ok", "ibp_bit", "ibp_elems"):
                b.setdefault("t3_" + k, 0); b["t3_" + k] += acc[k]
            b.setdefault("t3_ibp_ulp_hist", [0]*(len(ULP_BUCKETS)+1))
            for k, v in enumerate(acc["ibp_ulp_hist"]): b["t3_ibp_ulp_hist"][k] += v
            b.setdefault("t3_ibp_ulp_max", 0)
            b["t3_ibp_ulp_max"] = max(b["t3_ibp_ulp_max"], acc["ibp_ulp_max"])
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
            dre = b.get("t3_drot_exact", 0); drc = b.get("t3_drot_close", 0)
            drerr_avg = (b.get("t3_drot_err", 0.0) / bt) if bt else 0.0
            line += (f", T3 {b['t3_pass']}/{total-missing} exact"
                     f" (size-match {sm}, avg byte-match {avg:.1f}%, missing-ref={missing};"
                     f" dpos {be}+{bc_}/{bt} exact+close, err {err_avg:.4f};"
                     f" drot {dre}+{drc}/{bt} exact+close, err {drerr_avg:.4f})")
        print(line)
        if do_t3 and b.get("t3_bones_total"):
            bt = b["t3_bones_total"]
            elems = b.get("t3_ibp_elems", 0)
            hist = b.get("t3_ibp_ulp_hist", [0]*(len(ULP_BUCKETS)+1))
            histstr = " ".join(
                (f"<={edge}:{hist[k]*100.0/elems:.1f}%" if elems else f"<={edge}:0")
                for k, edge in enumerate(ULP_BUCKETS)) +                 (f" >{ULP_BUCKETS[-1]}:{hist[len(ULP_BUCKETS)]*100.0/elems:.1f}%" if elems else "")
            print(f"  float-level: bit-exact dpos {b.get('t3_dp_bit',0)}/{bt}"
                  f" drot {b.get('t3_dr_bit',0)}/{bt}"
                  f" scale {b.get('t3_scale_bit',0)}/{bt} lod {b.get('t3_lod_bit',0)}/{bt}"
                  f" aux {b.get('t3_aux_bit',0)}/{bt};"
                  f" father {b.get('t3_father_ok',0)}/{bt} unherit {b.get('t3_unherit_ok',0)}/{bt}")
            print(f"  invbindpos: state-bits {b.get('t3_ibp_state_ok',0)}/{bt},"
                  f" bit-exact bones {b.get('t3_ibp_bit',0)}/{bt},"
                  f" element ULP [{histstr}] max={b.get('t3_ibp_ulp_max',0)}")
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
    ap.add_argument("--gate-t3", action="store_true",
                    help="fail (exit 1) when T3 export accuracy regresses below the recorded floor: "
                         "biped size-match 100%%, drot exact >= 97%%, dpos exact+close >= 72%%; "
                         "non-biped dpos and drot 100%% exact")
    ap.add_argument("-v", "--verbose", action="store_true")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = True

    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print(f"SKIP: asset checkouts not present ({args.graphics}, {args.workspace}) — "
              f"this corpus test only runs where Kaetemi's private ryzomcore_graphics / "
              f"ryzomcore_leveldesign checkouts are available")
        sys.exit(SKIP_CODE)

    files = enumerate_corpus(args.graphics, args.workspace)
    print(f"corpus: {len(files)} .max files across {len(set(d for d,_,_ in files))} dirs")
    if not files:
        print("SKIP: enumerated 0 .max files — check --graphics / --workspace paths")
        sys.exit(SKIP_CODE)

    ref_biped = args.ref_biped if os.path.isdir(args.ref_biped) else None
    ref_nonbiped = args.ref_nonbiped if os.path.isdir(args.ref_nonbiped) else None
    if args.t3:
        if not ref_biped: print(f"note: T3 biped-ref dir {args.ref_biped} not present")
        if not ref_nonbiped: print(f"note: T3 nonbiped-ref dir {args.ref_nonbiped} not present")

    buckets = run_tests(args.bin, files, args.t1, args.t2, args.t3, ref_biped, ref_nonbiped, args.output, args.jobs)
    report(buckets, args.t1, args.t2, args.t3, args.verbose)

    # Non-zero exit if any T1/T2 failure surfaced (T3 mismatches are epsilon-tolerated unless
    # --gate-t3 asks for the accuracy regression gate).
    fail = 0
    for k in ("biped", "nonbiped"):
        b = buckets.get(k, {})
        fail += len(b.get("t1_fail", [])) + len(b.get("t2_fail", []))
    if args.gate_t3 and args.t3:
        def frac(b, num_key, den_key):
            den = b.get(den_key, 0)
            return (b.get(num_key, 0) / den) if den else 1.0
        bb = buckets.get("biped", {})
        nb = buckets.get("nonbiped", {})
        gates = []
        n_cmp = bb.get("total", 0) - len(bb.get("t3_missing_ref", []))
        if n_cmp:
            sm = bb.get("t3_size_match", 0) + bb.get("t3_pass", 0)
            gates.append(("biped size-match", sm >= n_cmp))
            gates.append(("biped drot exact >= 97%", frac(bb, "t3_drot_exact", "t3_bones_total") >= 0.97))
            dp_ec = bb.get("t3_bones_exact", 0) + bb.get("t3_bones_close", 0)
            bt = bb.get("t3_bones_total", 0)
            gates.append(("biped dpos exact+close >= 72%", (dp_ec / bt if bt else 1.0) >= 0.72))
        nt = nb.get("t3_bones_total", 0)
        if nt:
            gates.append(("nonbiped dpos 100% exact", nb.get("t3_bones_exact", 0) >= nt))
            gates.append(("nonbiped drot 100% exact", nb.get("t3_drot_exact", 0) >= nt))
        # Field-level structural floors (currently 100% on both buckets): father links,
        # UnheritScale, LodDisableDistance, the always-constant aux fields (euler/pivot/
        # skinScale/scale33), and the InvBindPos CMatrix state bits.
        for label, bucket in (("biped", bb), ("nonbiped", nb)):
            t = bucket.get("t3_bones_total", 0)
            if not t: continue
            for key, nm in (("t3_father_ok", "father"), ("t3_unherit_ok", "unheritScale"),
                            ("t3_lod_bit", "lodDistance"), ("t3_aux_bit", "aux fields"),
                            ("t3_ibp_state_ok", "invbindpos state bits")):
                gates.append((f"{label} {nm} 100%", bucket.get(key, 0) >= t))
        for name, ok in gates:
            if not ok:
                print(f"T3 GATE FAIL: {name}")
                fail += 1
    sys.exit(1 if fail else 0)

if __name__ == "__main__":
    main()
