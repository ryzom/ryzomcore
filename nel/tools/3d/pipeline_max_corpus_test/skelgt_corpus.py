#!/usr/bin/env python3
"""Validation driver for the LEGACY ground-truth dump (~/skel_gt) — Max 9's own evaluation of
the ORIGINAL corpus skeleton files (skel_gt_dump_max9.ms, read-only: loads each .max and logs
every scene node's world transform plus the per-system biped.getTransform view; nothing saved).

This is the per-bone decode oracle for the legacy (figure-version 3) records that the reference
.skel files cannot provide (78/169 are era-stale) and that the regen corpus cannot reach (the
regenerated rigs are fresh-format re-encodings with Max-re-derived proportions). It covers the
questions still open on the legacy side: corpus-era L-half finger/toe base matrices, the
non-COM-parented prs:name markers, the moved-footsteps grounds, and the two-biped kitin rigs.

For each FILE in the dump manifest: locate the original .max via the workspace SkelSourceDirs,
run pipeline_max_export_skel --manifest, and compare per-node world transforms by name against
the dump's NODE lines (which include PRS markers, footsteps and nubs — not only biped bones).

Self-skips (exit 77) when ~/skel_gt or the binaries aren't present.
"""

import argparse, collections, math, os, subprocess, sys

from skel_corpus import enumerate_corpus, is_git_lfs_stub, SKIP_CODE


def pos_tol(p):
    m = max(abs(p[0]), abs(p[1]), abs(p[2]))
    return max(2e-4, 2e-5 * m)

ROT_EXACT = 1e-4
ROT_CLOSE = 0.02
POS_CLOSE = 0.02


def parse_gt_manifest(path):
    """dump manifest -> {base: {node_name: (parent, ctrl, pos3, quat4)}} (first entry wins)"""
    files = {}
    cur = None
    for line in open(path, errors="replace"):
        line = line.rstrip("\n")
        if line.startswith("FILE\t"):
            cur = line.split("\t")[1]
            if cur.endswith(".max"):
                cur = cur[:-4]
            files[cur] = {}
        elif line.startswith("  NODE\t") and cur is not None:
            p = line.strip().split("\t")
            name = p[1]
            vals = {p[k]: p[k + 1] for k in range(2, len(p) - 1, 2)}
            if name in files[cur]:
                continue
            try:
                pos = tuple(float(x) for x in vals["pos"].split(","))
                rot = tuple(float(x) for x in vals["rot"].split(","))
            except (KeyError, ValueError):
                continue
            files[cur][name] = (vals.get("parent", "-"), vals.get("ctrl", "?"), pos, rot)
        elif line.startswith("ERROR\t") and cur is not None:
            files[cur] = None
    return files


def parse_dump(path):
    out = {}
    for line in open(path, errors="replace"):
        if not line.startswith("  BONE\t"):
            continue
        p = line.strip().split("\t")
        name = p[1]
        vals = {p[k]: p[k + 1] for k in range(2, len(p) - 1, 2)}
        if name in out:
            continue
        out[name] = (tuple(float(x) for x in vals["pos"].split(",")),
                     tuple(float(x) for x in vals["rot"].split(",")))
    return out


def quat_dist(a, b):
    d1 = math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(4)))
    d2 = math.sqrt(sum((a[i] + b[i]) ** 2 for i in range(4)))
    return min(d1, d2)


def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--gt", default=os.path.join(home, "skel_gt"))
    ap.add_argument("--graphics", default=os.path.join(home, "ryzomcore_graphics"))
    ap.add_argument("--workspace", default=os.path.join(home, "ryzomcore_leveldesign/workspace"))
    ap.add_argument("--bin", default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--only", default=None)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    manifest_path = os.path.join(args.gt, "manifest.txt")
    if not os.path.isfile(manifest_path):
        print(f"SKIP: legacy GT dump not present at {args.gt} (run skel_gt_dump_max9.ms in Max 9 first)")
        sys.exit(SKIP_CODE)
    export_skel = os.path.join(args.bin, "pipeline_max_export_skel")
    if not os.path.isfile(export_skel):
        print(f"SKIP: missing binary {export_skel}")
        sys.exit(SKIP_CODE)
    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print("SKIP: asset checkouts not present")
        sys.exit(SKIP_CODE)

    sources = {}
    for d, name, full in enumerate_corpus(args.graphics, args.workspace):
        if is_git_lfs_stub(full):
            continue
        sources[os.path.splitext(name)[0]] = full

    gt = parse_gt_manifest(manifest_path)
    bases = sorted(b for b, v in gt.items() if v)
    if args.only:
        bases = [b for b in bases if args.only in b]
    errors = sorted(b for b, v in gt.items() if v is None)
    print(f"legacy GT: {len(bases)} dumped files ({len(errors)} load errors in Max: {', '.join(errors[:5])}{'...' if len(errors) > 5 else ''})")

    tmp = os.path.join(os.environ.get("TMPDIR", "/tmp"), f"skelgt.{os.getpid()}")
    agg = dict(files=0, files_ok=0, bones=0, pos_exact=0, pos_close=0, rot_exact=0, rot_close=0,
               pos_err=0.0, rot_err=0.0, missing=0)
    worst = []
    for base in bases:
        full = sources.get(base)
        if not full:
            print(f"  no corpus source for {base}")
            continue
        r = subprocess.run([export_skel, "--manifest", tmp + ".gt", full, tmp + ".skel"],
                           capture_output=True, text=True, timeout=120)
        if r.returncode != 0:
            print(f"  EXPORTER rc={r.returncode} {base}")
            continue
        ours = parse_dump(tmp + ".gt")
        agg["files"] += 1
        file_ok = True
        for name, (parent, ctrl, mpos, mrot) in gt[base].items():
            if name not in ours:
                # GT covers ALL scene nodes; we only walk the Bip01 subtree. Count only biped
                # nodes we're expected to export — the _big/_small/_selle variant files carry a
                # full duplicate rig suffixed _ref_scale (the artists' re-proportioning
                # template, never exported; 5205 nodes corpus-wide), which doesn't count.
                if (ctrl.startswith("BipSlave") or ctrl.startswith("BipDriven")
                        or ctrl.startswith("Vertical_Horizontal")) \
                        and "_ref_scale" not in name and name.startswith("Bip0"):
                    agg["missing"] += 1
                    file_ok = False
                continue
            opos, orot = ours[name]
            perr = math.sqrt(sum((opos[i] - mpos[i]) ** 2 for i in range(3)))
            rerr = quat_dist(orot, mrot)
            agg["bones"] += 1
            agg["pos_err"] += perr
            agg["rot_err"] += rerr
            pe = perr <= pos_tol(mpos)
            re_ = rerr <= ROT_EXACT
            if pe: agg["pos_exact"] += 1
            elif perr <= POS_CLOSE: agg["pos_close"] += 1
            if re_: agg["rot_exact"] += 1
            elif rerr <= ROT_CLOSE: agg["rot_close"] += 1
            if not (pe and re_):
                file_ok = False
                worst.append((max(perr, rerr), base, name, ctrl, perr, rerr))
        if file_ok:
            agg["files_ok"] += 1

    for suffix in (".gt", ".skel"):
        try: os.unlink(tmp + suffix)
        except OSError: pass

    b = agg["bones"]
    if b:
        print(f"GT: files fully-exact {agg['files_ok']}/{agg['files']}; bones {b}"
              f" pos {agg['pos_exact']}+{agg['pos_close']}/{b} exact+close (err {agg['pos_err']/b:.5f})"
              f" rot {agg['rot_exact']}+{agg['rot_close']}/{b} exact+close (err {agg['rot_err']/b:.5f})"
              f" missing {agg['missing']}")
        if args.verbose:
            worst.sort(reverse=True)
            for w, base, name, ctrl, perr, rerr in worst[:40]:
                print(f"  WORST {base:28s} {name:26s} {ctrl[:18]:18s} perr {perr:.5f} rerr {rerr:.5f}")
    sys.exit(0)


if __name__ == "__main__":
    main()
