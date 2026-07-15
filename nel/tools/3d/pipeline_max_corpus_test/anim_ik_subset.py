#!/usr/bin/env python3
"""Fast subset A/B for biped-IK heuristic changes (the §10s pick-up list).

A full anim_corpus.py T3 sweep over the 3173 biped direct-reference files takes 7-10 minutes;
iterating an IK heuristic against that is prohibitively slow, and per-file probes are unreliable
(pipeline_max_design.md §10r: "per-file probes systematically lied about corpus-wide effect").
This driver sweeps a small (~250-file) subset in ~30 s: the known worst offenders + flip-floppers
+ a seeded random sample of the biped direct-ref corpus. It exports each file twice (baseline and
with an env-var override set), compares the per-file worst key delta against the direct reference,
and reports the net improved/regressed/same tally plus the top regressions — the trustworthy signal
for whether a heuristic change is corpus-net-positive BEFORE committing to a full sweep.

Usage:
  anim_ik_subset.py [--env NAME=VAL] [--env2 NAME=VAL] ... [-n NRANDOM] [--seed S] [--bin DIR]
  --env NAME=VAL   environment override for the "A" run (repeatable). Default is none (self-check
                   against the shipping default — arm-pin ON since §10s-quat). To A/B arms off:
                   --env PMB_BIPED_IK_ARMS=0 (A = legs-only, B = default arms-on). Pass
                   '--env BASE' for an explicit no-override self-check.
  -n NRANDOM       random-sample size on top of the fixed worst-offender/flip-flopper set (default 220).
  --seed S         random seed (default 42).
  --bin DIR        binary dir (default ~/ryzomcore/build/nel-pipeline/bin).

Only biped direct-ref files are swept (the IK class); non-biped and optimized-fauna files are
byte-identical or informational and unaffected by IK heuristics. Compares against
~/pipeline_export/common/characters/anim_export (the pre-optimizer direct reference anim_corpus uses).
"""
import argparse, os, random, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from anim_corpus import compare_direct_float, enumerate_corpus, is_git_lfs_stub, is_biped

# Known worst offenders (the strike/coup/run class) + historical flip-floppers, always included so
# a change's effect on the files it is meant to help is visible even at -n 0. Extend as new
# flip-floppers surface. The §10s-bis arm-pin's characterized win/regress classes are both here so a
# gate change's effect on each is visible: the dynamic-strike/mount wins AND the held-pose
# (idle_attente/engarde) regressions the next gate must exclude.
MUST_INCLUDE = [
    # dynamic-strike / mount-attack WINS (the class the arm-pin helps):
    "fy_hof_co_l2m_coup_fort_03", "fy_hom_co_l2m_coup_fort_03", "fy_hom_co_a2m_coup1",
    "fy_hom_monture_aquatique_recul_attack", "fy_hom_monture_aquatique_impact",
    "fy_hom_monture_aquatique_demitour_gauche", "fy_hom_monture_aquatique_tourne_gauche_attack",
    "fy_hom_emote_indifferent", "fy_hom_swim_hisser", "fy_hof_monture_aquatique_nage_attack",
    "fy_hom_co_a2m_coup_1stperson", "fy_hof_co_h2m_couplourd", "fy_hof_co_a1m_coup1",
    # held-pose REGRESSIONS (idle_attente/engarde — the class the next gate must exclude):
    "fy_hof_co_a2m_idle_attente1", "fy_hof_co_a2m_idle_attente2", "fy_hom_co_a2m_engarde_attente1",
    "fy_hom_co_a2m_engarde_attente2", "fy_hof_co_l2m_idle_attente1", "fy_hof_co_l2m_idle_attente2",
    "fy_hom_co_grs_lancegrenade", "ca_hom_trooper_impact",
    # other historical flip-floppers / worst offenders:
    "fy_hom_emote_beta_testeur", "fy_hof_emote_beta_testeur", "fy_hom_co_a2m_couplourd",
    "fy_hof_co_l2m_toupie", "fy_hom_co_fus_magie_cur_end_link", "zo_hof_co_mn_course_frappe",
    "fy_hof_co_l2m_coup2_bas", "fy_hom_co_a1md_marche_arriere_coup", "fy_hof_co_mn_coupdepied_milieu",
    "fy_hom_co_l2m_marche_coup", "fy_hof_co_mn_course_frappe", "fy_hof_co_a2m_coup_medium",
    "fy_hom_co_l2m_toupie_haut", "zo_hof_co_a2m_course", "ca_hom_trooper_straf_gauche",
    "fy_hom_co_fus_demitour_go", "fy_hof_co_fus_tourne_gauche", "fy_hom_l2m_tournedroite",
]


def build_subset(files, n_random, seed, ref_dir):
    cand = set()
    by_name = {}
    for (g, d, name, full) in files:
        if is_git_lfs_stub(full) or not is_biped(full):
            continue
        if not os.path.isfile(os.path.join(ref_dir, name[:-4] + ".anim")):
            continue
        cand.add(name)
        by_name[name] = (g, d, name, full)
    chosen = set(n + ".max" for n in MUST_INCLUDE if (n + ".max") in cand)
    random.seed(seed)
    rest = [n for n in cand if n not in chosen]
    random.shuffle(rest)
    for n in rest[:n_random]:
        chosen.add(n)
    return [by_name[n] for n in chosen if n in by_name]


def main():
    home = os.path.expanduser("~")
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--env", action="append", default=[], help="NAME=VAL override for the A run (default none = self-check; 'BASE' = none)")
    ap.add_argument("-n", type=int, default=220, help="random-sample size (default 220)")
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--bin", default=os.path.join(home, "ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--graphics", default=os.path.join(home, "ryzomcore_graphics"))
    ap.add_argument("--workspace", default=os.path.join(home, "ryzomcore_leveldesign/workspace"))
    ap.add_argument("--ref", default=os.path.join(home, "pipeline_export/common/characters/anim_export"))
    ap.add_argument("-j", "--jobs", type=int, default=12)
    args = ap.parse_args()

    export_bin = os.path.join(args.bin, "pipeline_max_export_anim")
    if not os.path.isfile(export_bin):
        print(f"SKIP: missing binary {export_bin}")
        return 77
    if not os.path.isdir(args.ref):
        print(f"SKIP: missing reference dir {args.ref}")
        return 77

    env_overrides = {}
    if args.env:
        for e in args.env:
            if e.upper() == "BASE":
                continue
            if "=" not in e:
                print(f"bad --env {e} (need NAME=VAL)")
                return 1
            k, v = e.split("=", 1)
            env_overrides[k] = v
    # No default override: shipping default is arm-pin ON (§10s-quat). Use
    # --env PMB_BIPED_IK_ARMS=0 to A/B legs-only as the "A" run against default B.

    files = enumerate_corpus(args.graphics, args.workspace)
    subset = build_subset(files, args.n, args.seed, args.ref)
    print(f"subset: {len(subset)} biped direct-ref files (A overrides: {env_overrides or 'none'})", file=sys.stderr)

    def run(item):
        g, d, name, full = item
        base = name[:-4]
        ref = os.path.join(args.ref, base + ".anim")
        ob, oa = f"/tmp/iksub_b_{base}.anim", f"/tmp/iksub_a_{base}.anim"
        subprocess.run([export_bin, full, ob], capture_output=True)
        env = dict(os.environ); env.update(env_overrides)
        subprocess.run([export_bin, full, oa], capture_output=True, env=env)
        try:
            wb = compare_direct_float(ob, ref)[1]
            wa = compare_direct_float(oa, ref)[1]
        except Exception:
            return (name, None, None)
        for p in (ob, oa):
            if os.path.exists(p):
                os.remove(p)
        return (name, wb, wa)

    res = []
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for r in ex.map(run, subset):
            res.append(r)

    impr = regr = same = 0; sb = sa = 0.0; n = 0; regr_list = []; impr_list = []
    for (name, wb, wa) in res:
        if wb is None:
            continue
        n += 1; sb += wb; sa += wa; dl = wa - wb
        if dl < -0.01: impr += 1; impr_list.append((name, wb, wa, dl))
        elif dl > 0.01: regr += 1; regr_list.append((name, wb, wa, dl))
        else: same += 1
    print("\n=== SUBSET A/B ===")
    print(f"override: {env_overrides or '(none — self-check, expect all same)'}")
    print(f"improved={impr}  regressed={regr}  same={same}  (n={n})")
    print(f"mean worst: base={sb/n:.4f}  A={sa/n:.4f}  delta={ (sa-sb)/n:+.4f}")
    print("\nTop regressions:")
    for (name, wb, wa, dl) in sorted(regr_list, key=lambda x: -x[3])[:15]:
        print(f"  +{dl:.4f}  {name}  ({wb:.4f}->{wa:.4f})")
    print("Top improvements:")
    for (name, wb, wa, dl) in sorted(impr_list, key=lambda x: x[3])[:8]:
        print(f"  {dl:+.4f}  {name}  ({wb:.4f}->{wa:.4f})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
