#!/usr/bin/env python3
"""Validation driver for the Max 9 regenerated biped corpus (encode-direction cross-validation).

The regen corpus (~/biped_regen) is produced by running gen_biped_regen.py's MAXScript in
3ds Max 9: every single-biped rig from the skel corpus is recreated with biped.createNew from
the headless decode, saved as a fresh-format (.max figure-version 0x0115 == 0) file, and the
per-bone figure-mode world transforms Max actually stored are logged to manifest.txt
(biped.getTransform ground truth). Only the maxscript <-> outputs relation is authoritative:
Max re-derives constrained values (clavicle 2-DOF, thigh/pelvis widths, ...) so the regen rigs
intentionally do NOT byte-match the original corpus files.

Tiers here:
  T1/T2  roundtrip coherency of the fresh Max 9 files through pipeline_max (corpus_test --parse).
  GT     decode validation: pipeline_max_export_skel --manifest dumps our reconstructed
         figure-mode world transforms from the regen .max; compared bone-by-bone against the
         manifest.txt ground truth (pos euclidean, rot double-cover quat distance).

Defaults are the layout on Kaetemi's machine; self-skips (exit 77) when ~/biped_regen or the
binaries aren't present.
"""

import argparse, collections, os, subprocess, sys

SKIP_CODE = 77

# Manifest floats are MAXScript "as string" (6 significant digits), so GT resolution is ~1e-6
# relative. Positions are world meters (up to ~29 on the kite rigs) -> relative-aware threshold.
def pos_tol(p):
    m = max(abs(p[0]), abs(p[1]), abs(p[2]))
    return max(2e-4, 2e-5 * m)

ROT_EXACT = 1e-4   # quat components rounded to 6 sig digits -> ~5e-7/component noise floor
ROT_CLOSE = 0.02
POS_CLOSE = 0.02


def parse_manifest(path):
    """manifest.txt -> {base_name: [(bone_name, id_float, link_float, pos3, quat4), ...]}"""
    files = {}
    cur = None
    with open(path, errors="replace") as f:
        for line in f:
            line = line.rstrip("\n")
            if line.startswith("STATE\t"):
                cur = line.split("\t", 1)[1]
                files[cur] = []
            elif line.startswith("  BONE\t") and cur is not None:
                parts = line.strip().split("\t")
                # BONE <name> id <i> link <l> pos <x,y,z> rot <x,y,z,w>
                name = parts[1]
                vals = {}
                for k in range(2, len(parts) - 1, 2):
                    vals[parts[k]] = parts[k + 1]
                pos = tuple(float(x) for x in vals["pos"].split(","))
                rot = tuple(float(x) for x in vals["rot"].split(","))
                files[cur].append((name, float(vals["id"]), float(vals["link"]), pos, rot))
            elif line.startswith("ERROR\t"):
                base = line.split("\t")[1]
                files.setdefault(base, None)  # regeneration failed inside Max
    return files


def parse_dump(path):
    """--manifest dump -> {bone_name: (id, link, pos3, quat4, is_biped, is_com)} (first wins)"""
    out = {}
    with open(path, errors="replace") as f:
        for line in f:
            if not line.startswith("  BONE\t"):
                continue
            parts = line.strip().split("\t")
            name = parts[1]
            vals = {}
            for k in range(2, len(parts) - 1, 2):
                vals[parts[k]] = parts[k + 1]
            if name in out:
                continue
            out[name] = (int(vals["id"]), int(vals["link"]),
                         tuple(float(x) for x in vals["pos"].split(",")),
                         tuple(float(x) for x in vals["rot"].split(",")),
                         vals.get("biped") == "1", vals.get("com") == "1")
    return out


def quat_dist(a, b):
    d1 = sum((a[i] - b[i]) ** 2 for i in range(4)) ** 0.5
    d2 = sum((a[i] + b[i]) ** 2 for i in range(4)) ** 0.5
    return min(d1, d2)


ROLE_NAMES = {1: "larm", 2: "rarm", 3: "lfingers", 4: "rfingers", 5: "lleg", 6: "rleg",
              7: "ltoes", 8: "rtoes", 9: "spine", 10: "tail", 11: "head", 12: "pelvis",
              13: "com", 14: "com", 15: "com", 16: "footsteps", 17: "neck",
              18: "pony1", 19: "pony2"}


def role_of(mid, mlink):
    n = ROLE_NAMES.get(int(mid), "id%d" % int(mid))
    return "%s.%d" % (n, int(mlink) - 1)


def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--regen", default=os.path.join(home, "biped_regen"))
    ap.add_argument("--bin", default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--t12", action="store_true", help="run T1/T2 roundtrip")
    ap.add_argument("--gt", action="store_true", help="run decode-vs-manifest ground truth compare")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-gt", action="store_true",
                    help="fail when GT accuracy drops below the recorded floor "
                         "(pos exact >= 72%%, rot exact >= 58%%, pos and rot exact+close >= 99.5%%, "
                         "no missing bones; the exact tier is 2e-4 m / 1e-4 quat, the close tier "
                         "0.02 — the remaining non-exact residues are the documented sub-mm "
                         "eps-twist class, see pipeline_max_design.md)")
    ap.add_argument("--only", default=None, help="restrict to files whose name contains this substring")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if args.all:
        args.t12 = args.gt = True
    if not (args.t12 or args.gt):
        args.t12 = args.gt = True

    manifest_path = os.path.join(args.regen, "manifest.txt")
    if not os.path.isdir(args.regen) or not os.path.isfile(manifest_path):
        print(f"SKIP: regen corpus not present at {args.regen} (run gen_biped_regen.py's script in Max 9 first)")
        sys.exit(SKIP_CODE)
    corpus_test = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_skel = os.path.join(args.bin, "pipeline_max_export_skel")
    for need in ((corpus_test,) if args.t12 else ()) + ((export_skel,) if args.gt else ()):
        if not os.path.isfile(need):
            print(f"SKIP: missing binary {need} (build it first)")
            sys.exit(SKIP_CODE)

    manifest = parse_manifest(manifest_path)
    bases = sorted(b for b, v in manifest.items() if v is not None)
    if args.only:
        bases = [b for b in bases if args.only in b]
    print(f"regen corpus: {len(bases)} rigs with manifest ground truth")

    t12_pass = 0
    t12_fail = []
    gt = dict(files=0, files_ok=0, bones=0, pos_exact=0, pos_close=0, rot_exact=0, rot_close=0,
              pos_err=0.0, rot_err=0.0, missing=0)
    role_stats = collections.defaultdict(lambda: [0, 0, 0, 0.0, 0.0])  # n, pos_exact, rot_exact, pos_err, rot_err
    worst = []

    tmpdir = os.environ.get("TMPDIR", "/tmp")
    dump_path = os.path.join(tmpdir, f"regen_corpus.{os.getpid()}.gt")
    skel_path = os.path.join(tmpdir, f"regen_corpus.{os.getpid()}.skel")

    for base in bases:
        full = os.path.join(args.regen, base + ".max")
        if not os.path.isfile(full):
            print(f"  missing regen file {base}.max (manifest entry without output?)")
            continue

        if args.t12:
            r = subprocess.run([corpus_test, "--parse", full], capture_output=True, text=True, timeout=120)
            if r.returncode == 0 and "FAIL" not in r.stdout:
                t12_pass += 1
            else:
                t12_fail.append((base, r.stdout.strip(), r.stderr.strip()))

        if args.gt:
            r = subprocess.run([export_skel, "--manifest", dump_path, full, skel_path],
                               capture_output=True, text=True, timeout=120)
            if r.returncode != 0:
                t12_fail.append((base, f"exporter rc={r.returncode}", r.stderr.strip()))
                continue
            ours = parse_dump(dump_path)
            gt["files"] += 1
            file_ok = True
            seen = set()
            for name, mid, mlink, mpos, mrot in manifest[base]:
                if name in seen:
                    continue  # COM appears three times (vertical/horizontal/turn ids)
                seen.add(name)
                if name not in ours:
                    gt["missing"] += 1
                    file_ok = False
                    continue
                _, _, opos, orot, _, _ = ours[name]
                perr = sum((opos[i] - mpos[i]) ** 2 for i in range(3)) ** 0.5
                rerr = quat_dist(orot, mrot)
                gt["bones"] += 1
                gt["pos_err"] += perr
                gt["rot_err"] += rerr
                pe = perr <= pos_tol(mpos)
                re_ = rerr <= ROT_EXACT
                if pe: gt["pos_exact"] += 1
                elif perr <= POS_CLOSE: gt["pos_close"] += 1
                if re_: gt["rot_exact"] += 1
                elif rerr <= ROT_CLOSE: gt["rot_close"] += 1
                if not (pe and re_):
                    file_ok = False
                    worst.append((max(perr, rerr), base, name, role_of(mid, mlink), perr, rerr))
                rs = role_stats[role_of(mid, mlink)]
                rs[0] += 1
                rs[1] += 1 if pe else 0
                rs[2] += 1 if re_ else 0
                rs[3] += perr
                rs[4] += rerr
            if file_ok:
                gt["files_ok"] += 1

    for p in (dump_path, skel_path):
        try: os.unlink(p)
        except OSError: pass

    if args.t12:
        total = len(bases)
        print(f"T1/T2: {t12_pass}/{total} pass")
        for base, summary, err in t12_fail[:20]:
            print(f"  FAIL {base}: {summary}")
            if err: print(f"    {err.splitlines()[0]}")

    fail = len(t12_fail)
    if args.gt and gt["bones"]:
        b = gt["bones"]
        print(f"GT: files fully-exact {gt['files_ok']}/{gt['files']}; bones {b}"
              f" pos {gt['pos_exact']}+{gt['pos_close']}/{b} exact+close (err {gt['pos_err']/b:.5f})"
              f" rot {gt['rot_exact']}+{gt['rot_close']}/{b} exact+close (err {gt['rot_err']/b:.5f})"
              f" missing {gt['missing']}")
        if args.verbose:
            print("  per-role (n, pos-exact, rot-exact, avg pos err, avg rot err):")
            for role in sorted(role_stats, key=lambda r: -(role_stats[r][0] - role_stats[r][2])):
                n, pe, re_, perr, rerr = role_stats[role]
                if pe == n and re_ == n:
                    continue
                print(f"    {role:14s} n={n:4d} pos {pe:4d} rot {re_:4d} perr {perr/n:.5f} rerr {rerr/n:.5f}")
            worst.sort(reverse=True)
            for w, base, name, role, perr, rerr in worst[:30]:
                print(f"  WORST {base:28s} {name:24s} {role:12s} perr {perr:.5f} rerr {rerr:.5f}")
        if args.gate_gt:
            if gt["pos_exact"] < 0.72 * b:
                print("GT GATE FAIL: pos exact < 72%")
                fail += 1
            if gt["rot_exact"] < 0.58 * b:
                print("GT GATE FAIL: rot exact < 58%")
                fail += 1
            if gt["pos_exact"] + gt["pos_close"] < 0.995 * b:
                print("GT GATE FAIL: pos exact+close < 99.5%")
                fail += 1
            if gt["rot_exact"] + gt["rot_close"] < 0.995 * b:
                print("GT GATE FAIL: rot exact+close < 99.5%")
                fail += 1
            if gt["missing"]:
                print("GT GATE FAIL: missing bones")
                fail += 1

    sys.exit(1 if fail else 0)


if __name__ == "__main__":
    main()
