#!/usr/bin/env python3
"""Corpus driver for the .max -> .cmb collision-mesh pipeline (build_gamedata processes/rbank's
direct cmb_export.ms path, and the ligo process's own exportCollisionsFromZone call).

Two GATED tiers, both driven by the single pipeline_max_export_cmb tool — the ligo tier is a
real Max export step (`nel_ligo_export.ms` calls `NelExportCollision` per brick exactly like it
calls `NelExportInstanceGroup` for `.ig`), landing brick-level `.cmb` files in
`LigoEcosystemCmbExportDirectory` that `land_exporter` then uses as `RefCMBDir` to build the
placed-instance `.cmb` set `rbank`'s "Build rbank indoor" step consumes for `.rbank`/`.gr`/`.lr`.
So the ligo tier is exactly as load-bearing as the standalone one, and the gate treats it that
way — same soft-gating convention as `ligo_ig_corpus.py` (a `LIGO_DIFF_BUDGET` that fails only
on regression past the observed count; tighten as diffs close).

  direct  RBankCmbSourceDirectories (== ShapeSourceDirectories) under continents/indoors/
          directories.py: the 16 construction .max files, one .cmb per distinct ig name inside,
          compared against ~/pipeline_export/continents/indoors/rbank_cmb_export/<name>.cmb.
          --gate-t3: T1/T2 must be clean; T3 diffs beyond DIRECT_DIFF_BUDGET fail.

  ligo    the same zonematerial-*/zonespecial-*/zonetransition-* brick corpus as
          ligo_ig_corpus.py/zone_corpus.py (landscape/ligo/<eco>/max), run through
          pipeline_max_export_cmb --ligo, compared against
          ~/pipeline_export/ecosystems/<eco>/ligo_es/cmb/<igname>.cmb. --gate-t3: T3 diffs
          beyond LIGO_DIFF_BUDGET fail; regressions past that budget block the gate the same
          way ligo_ig_corpus.py does. Tighten the budget as diffs close.
"""

import argparse, math, os, struct, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77

ECOSYSTEMS = ["desert", "jungle", "lacustre", "primes_racines"]
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

DIRECT_SOURCES = [
    ("fyros", "fy_cn_appart_joueur", "fy_appart_joueur.max"),
    ("fyros", "fy_cn_hall_conseil", "fy_hall_conseil.max"),
    ("fyros", "fy_cn_hall_reunion_vitrine", "fy_hall_reunion.max"),
    ("fyros", "fy_cn_salle_npc", "fy_cn_salle_npc.max"),
    ("matis", "appart_joueur", "ma_appart_joueur.max"),
    ("matis", "hall_du_conseil", "ma_hall_conseil.max"),
    ("matis", "hall_vitrine_hall_reunion", "ma_hall_vitrine_hall_reunion.max"),
    ("matis", "salle_npc", "ma_salle_npc.max"),
    ("tryker", "hall_conseil", "tr_hall_conseil.max"),
    ("tryker", "hall_vitrine_reunion", "tr_hall_vitrine_reunion.max"),
    ("tryker", "piece_npc", "tr_piece_npc.max"),
    ("tryker", "tr_appart", "tr_appart.max"),
    ("zorai", "appart_joueur", "zo_bt_appart.max"),
    ("zorai", "hall_conseil", "zo_bt_hall_conseil.max"),
    ("zorai", "hall_reunion_vitrine", "zo_bt_hall_reunion_vitrine.max"),
    ("zorai", "salle_npc", "zo_bt_piece_npc.max"),
]

# Known-open T3 diffs — regressions past these budgets fail --gate-t3; dropping below is welcome
# (tighten the budget when it does — track budget-vs-actual in the same doc section that landed
# each fix).
#
# Direct tier (16 files, 16 igs):
#   Zo_bt_Hall_Conseil — 80/80 verts, 0 face diffs, ~0.05 unit vertex offset (Y-preserved X/Z
#                        diverge; root cause unresolved, precision ruled out via VS2008/x87).
#   (FY_hall_reunion + Zo_bt_hall_Reunion_vitrine CLOSED 2026-07-10 — created-vert records with
#    srcTag != -1 are CLONE+OFFSET, resolved against the source vertex's pre-move position; both
#    files are byte-identical now. Design doc §10z-quinze.)
DIRECT_DIFF_BUDGET = 1

# Ligo tier (1201 bricks, ~221 igs measured against reference): documented open classes are
# XRef sources resolving to classes cmb doesn't yet build a mesh for (unregistered scripted
# plugins, Shape/SplineShape superclass 0x40), and the shared cluster-containment / selection-
# order classes ligo_ig_corpus.py documents on the same brick set. Tighten as diffs close;
# regressions past this fail --gate-t3 the same way ligo_ig_corpus.py's DIFF_BUDGET does.
LIGO_DIFF_BUDGET = 95

# float32 position tolerance for the direct tier's "close" classification (x87-vs-SSE /
# operation-order noise — same POS_EPS-style tier already established for ig/zone/shape).
def pos_eps(mag):
    return max(1e-5, 3e-7 * mag)


def parse_cmb(path):
    d = open(path, "rb").read()
    o = 0
    (nv,) = struct.unpack_from("<I", d, o); o += 4
    verts = []
    for _ in range(nv):
        v = struct.unpack_from("<fff", d, o); o += 12
        verts.append(v)
    (nf,) = struct.unpack_from("<I", d, o); o += 4
    faces = []
    for _ in range(nf):
        v0, v1, v2 = struct.unpack_from("<III", d, o); o += 12
        vis = d[o:o + 3]; o += 3
        surf, mat = struct.unpack_from("<ii", d, o); o += 8
        faces.append((v0, v1, v2, vis, surf, mat))
    return verts, faces


def cmp_close(a_path, b_path):
    """True (exact-or-tolerance) / False (real diff) / None (structural mismatch, can't compare)."""
    a = open(a_path, "rb").read()
    b = open(b_path, "rb").read()
    if a == b:
        return True
    try:
        av, af = parse_cmb(a_path)
        bv, bf = parse_cmb(b_path)
    except (struct.error, AssertionError):
        return None
    if len(av) != len(bv) or af != bf:
        return None
    for x, y in zip(av, bv):
        d = math.sqrt(sum((p - q) ** 2 for p, q in zip(x, y)))
        mag = math.sqrt(sum(p * p for p in x)) or 1.0
        if d > pos_eps(mag):
            return None
    return True


def enumerate_ligo(graphics_dir):
    files = []
    for eco in ECOSYSTEMS:
        d = os.path.join(graphics_dir, "landscape", "ligo", eco, "max")
        if not os.path.isdir(d):
            continue
        for fn in sorted(os.listdir(d)):
            if not fn.endswith(".max"):
                continue
            toks = fn[:-4].split("-")
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
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--graphics", default=os.path.expanduser("~/ryzomcore_graphics"))
    ap.add_argument("--ref", default=os.path.expanduser("~/pipeline_export"))
    ap.add_argument("--bin", default=os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--out", default="/tmp/cmb_corpus_out.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--ligo", action="store_true", help="also run the ligo-brick tier (slow, gated same as direct)")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = args.ligo = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_cmb")
    if not os.path.isdir(args.graphics):
        print("SKIP: asset checkout not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or (args.t3 and not os.path.isfile(export_bin)):
        print("SKIP: binaries not built")
        return SKIP_CODE

    direct_files = []
    for race, subdir, fn in DIRECT_SOURCES:
        p = os.path.join(args.graphics, "stuff", race, "decors", "constructions", subdir, fn)
        if os.path.isfile(p):
            direct_files.append(p)
    if not direct_files:
        print("SKIP: no direct corpus files found")
        return SKIP_CODE
    print("corpus: %d direct cmb-source .max files" % len(direct_files))

    have_ref = os.path.isdir(args.ref)
    os.makedirs(args.out, exist_ok=True)

    t1_pass = t1_fail = t2_pass = t2_fail = 0
    direct_match = direct_close = direct_diff = direct_noref = 0
    diff_names = []

    def process_direct(indexed):
        idx, path = indexed
        res = {"path": path, "t1": None, "t2": None, "igs": []}
        if args.t1:
            res["t1"] = subprocess.run([corpus_bin, path], capture_output=True).returncode == 0
        if args.t2:
            res["t2"] = subprocess.run([corpus_bin, "--parse", path], capture_output=True).returncode == 0
        if args.t3:
            outdir = os.path.join(args.out, "direct%d" % idx)
            os.makedirs(outdir, exist_ok=True)
            r = subprocess.run([export_bin, path, outdir], capture_output=True, text=True)
            if r.returncode not in (0, 3):
                res["exportfail"] = r.stderr[-300:] if r.stderr else ""
                return res
            refdir = os.path.join(args.ref, "continents", "indoors", "rbank_cmb_export")
            for name in sorted(f for f in os.listdir(outdir) if f.endswith(".cmb")):
                ours = os.path.join(outdir, name)
                ref = os.path.join(refdir, name)
                if not os.path.isfile(ref):
                    res["igs"].append((name, "noref"))
                else:
                    c = cmp_close(ours, ref)
                    res["igs"].append((name, "match" if c is True and open(ours, "rb").read() == open(ref, "rb").read()
                                        else ("close" if c is True else "diff")))
        return res

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        results = list(ex.map(process_direct, enumerate(direct_files)))

    for res in results:
        if res.get("t1") is not None:
            if res["t1"]:
                t1_pass += 1
            else:
                t1_fail += 1
                print("T1 FAIL %s" % res["path"])
        if res.get("t2") is not None:
            if res["t2"]:
                t2_pass += 1
            else:
                t2_fail += 1
                print("T2 FAIL %s" % res["path"])
        if "exportfail" in res:
            print("T3 EXPORT FAIL %s: %s" % (res["path"], res["exportfail"]))
            continue
        for name, status in res.get("igs", []):
            if status == "match":
                direct_match += 1
            elif status == "close":
                direct_close += 1
            elif status == "noref":
                direct_noref += 1
            else:
                direct_diff += 1
                diff_names.append(name)
                print("T3 DIFF %s" % name)

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail" % (t1_pass, t1_fail, t2_pass, t2_fail))
    if args.t3:
        print("T3 direct: %d byte-identical, %d within float tolerance, %d differ, %d without ref"
              % (direct_match, direct_close, direct_diff, direct_noref))

    ligo_match = ligo_close = ligo_diff = ligo_noref = ligo_exportfail = 0
    if args.ligo:
        ligo_corpus = enumerate_ligo(args.graphics)
        print("\nligo corpus: %d brick .max files (gated same as direct)" % len(ligo_corpus))

        def process_ligo(indexed):
            idx, (eco, path) = indexed
            outdir = os.path.join(args.out, "ligo%d" % idx)
            os.makedirs(outdir, exist_ok=True)
            r = subprocess.run([export_bin, "--ligo", path, outdir], capture_output=True, text=True)
            if r.returncode not in (0, 3):
                return {"exportfail": (path, r.stderr[-200:] if r.stderr else "")}
            refdir = os.path.join(args.ref, "ecosystems", eco, "ligo_es", "cmb")
            igs = []
            for name in sorted(f for f in os.listdir(outdir) if f.endswith(".cmb")):
                ours = os.path.join(outdir, name)
                ref = os.path.join(refdir, name)
                if not os.path.isfile(ref):
                    igs.append((name, "noref"))
                else:
                    c = cmp_close(ours, ref)
                    igs.append((name, "match" if c is True and open(ours, "rb").read() == open(ref, "rb").read()
                                 else ("close" if c is True else "diff")))
            return {"igs": igs}

        with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
            lresults = list(ex.map(process_ligo, enumerate(ligo_corpus)))
        seen = set()
        for res in lresults:
            if "exportfail" in res:
                ligo_exportfail += 1
                continue
            for name, status in res.get("igs", []):
                if name in seen:
                    continue
                seen.add(name)
                if status == "match":
                    ligo_match += 1
                elif status == "close":
                    ligo_close += 1
                elif status == "noref":
                    ligo_noref += 1
                else:
                    ligo_diff += 1
        print("ligo T3: %d byte-identical, %d within float tolerance, %d differ, %d without ref, %d export failures"
              % (ligo_match, ligo_close, ligo_diff, ligo_noref, ligo_exportfail))

    fails = t1_fail + t2_fail
    if args.gate_t3 and have_ref:
        fails += max(0, direct_diff - DIRECT_DIFF_BUDGET)
        print("\ndirect diff budget: %d used / %d allowed" % (direct_diff, DIRECT_DIFF_BUDGET))
        if args.ligo:
            ligo_over_budget = max(0, (ligo_diff + ligo_exportfail) - LIGO_DIFF_BUDGET)
            fails += ligo_over_budget
            print("ligo diff budget: %d used / %d allowed (%d diff + %d export-fail)"
                  % (ligo_diff + ligo_exportfail, LIGO_DIFF_BUDGET, ligo_diff, ligo_exportfail))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
