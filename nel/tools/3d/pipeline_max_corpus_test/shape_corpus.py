#!/usr/bin/env python3
"""Corpus driver for the shape .max -> .shape pipeline.

Enumerates every shape-source .max from the ryzomcore_leveldesign workspace configs
(ShapeSourceDirectories over all projects running the "shape" process — the exact listing
build_gamedata processes/shape/1_export.py drives), and runs test tiers:

  T1  structural roundtrip:  the pipeline_max_corpus_test binary, no --parse.
  T2  parse/build roundtrip: the pipeline_max_corpus_test binary with --parse.
  T3  .shape export:         pipeline_max_export_shape per file, then per exported shape a
      byte-compare against the raw 1_export reference under --ref
      (~/pipeline_export/<group>/<project>/shape_not_optimized + shape_with_coarse_mesh).
      SKIP lines from the exporter are bucketed per feature class; reference shapes not
      produced by us are reported per skip class (coverage accounting).

Defaults are the layout on Kaetemi's machine; override via CLI when running elsewhere.
"""

import argparse, os, re, subprocess, sys, collections

SKIP_CODE = 77

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_WORKSPACE = os.path.expanduser("~/ryzomcore_leveldesign/workspace")
DEF_REF = os.path.expanduser("~/pipeline_export")
DEF_BIN = os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin")

OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"


def load_dirs(path):
    ns = {}
    try:
        exec(compile(open(path).read(), path, "exec"), ns)
    except Exception:
        return None
    return ns


def resolve_ci(root, rel):
    rel = rel.replace("\\", "/")
    for cand in (rel.lower(), rel):
        p = os.path.join(root, cand)
        if os.path.isdir(p):
            return p
    return None


def enumerate_corpus(graphics_dir, workspace_dir):
    """[(project, max_path)] in workspace order, unique by path (first project wins)."""
    out = []
    seen = set()
    for root in ("common", "ecosystems", "continents", "shard"):
        base = os.path.join(workspace_dir, root)
        if not os.path.isdir(base):
            continue
        for name in sorted(os.listdir(base)):
            d = os.path.join(base, name, "directories.py")
            p2 = os.path.join(base, name, "process.py")
            if not os.path.isfile(d) or not os.path.isfile(p2):
                continue
            pns = load_dirs(p2)
            if not pns or "shape" not in (pns.get("ProcessToComplete") or []):
                continue
            ns = load_dirs(d)
            if ns is None:
                continue
            for dd in ns.get("ShapeSourceDirectories") or []:
                full = resolve_ci(graphics_dir, dd)
                if not full:
                    continue
                for f in sorted(os.listdir(full)):
                    if not f.lower().endswith(".max"):
                        continue
                    p = os.path.join(full, f)
                    if p in seen:
                        continue
                    seen.add(p)
                    out.append((root + "/" + name, p))
    return out


def is_ole(path):
    with open(path, "rb") as f:
        return f.read(8) == OLE_MAGIC


def project_refs(ref_dir, proj):
    """shape name (lower) -> path, for both the normal and coarse reference dirs."""
    grp, leaf = proj.split("/")
    refs = {}
    coarse = {}
    d = os.path.join(ref_dir, grp, leaf, "shape_not_optimized")
    if os.path.isdir(d):
        for f in os.listdir(d):
            if f.endswith(".shape"):
                refs[f.lower()] = os.path.join(d, f)
    d = os.path.join(ref_dir, grp, leaf, "shape_with_coarse_mesh")
    if os.path.isdir(d):
        for f in os.listdir(d):
            if f.endswith(".shape"):
                coarse[f.lower()] = os.path.join(d, f)
    return refs, coarse


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=DEF_GRAPHICS)
    ap.add_argument("--workspace", default=DEF_WORKSPACE)
    ap.add_argument("--ref", default=DEF_REF)
    ap.add_argument("--bin", default=DEF_BIN)
    ap.add_argument("--out", default="/tmp/shape_corpus_out.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true", help="fail on T3 regressions")
    ap.add_argument("--min-identical", type=int, default=0,
                    help="minimum byte-identical-or-floateq shape count for --gate-t3")
    ap.add_argument("--max-diff", type=int, default=0,
                    help="budgeted structural-diff count for --gate-t3 (milestone budget)")
    ap.add_argument("--only", default=None, help="substring filter on the .max path")
    ap.add_argument("--project", default=None, help="substring filter on the project")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_shape")
    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print("SKIP: asset checkouts not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or (args.t3 and not os.path.isfile(export_bin)):
        print("SKIP: binaries not built")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics, args.workspace)
    if args.only:
        corpus = [c for c in corpus if args.only in c[1]]
    if args.project:
        corpus = [c for c in corpus if args.project in c[0]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

    t1_pass = t1_fail = t2_pass = t2_fail = 0
    stubs = 0
    mapext_nodes = set()
    mapext_report = []
    lightmap_nodes = set()
    export_fail = []
    produced = {}
    skip_classes = collections.Counter()
    warn_classes = collections.Counter()

    os.makedirs(args.out, exist_ok=True)

    from concurrent.futures import ThreadPoolExecutor

    # Reference material-animation (.anim) index: the shape process writes a per-node <node>.anim
    # (texture-matrix tracks); the final client references live under ~/core4_data/*_shapes.
    import glob as _glob
    g_animRefIdx = {}
    if args.t3:
        for r in _glob.glob(os.path.expanduser("~/core4_data") + "/**/*.anim", recursive=True):
            g_animRefIdx.setdefault(os.path.basename(r), r)

    def run_file(item):
        proj, path = item
        res = {"proj": proj, "path": path}
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
            flat = proj.replace("/", "_")
            outdir = os.path.join(args.out, flat)
            coarsedir = os.path.join(args.out, flat + "_coarse")
            animdir = os.path.join(args.out, flat + "_anim")
            os.makedirs(outdir, exist_ok=True)
            os.makedirs(coarsedir, exist_ok=True)
            os.makedirs(animdir, exist_ok=True)
            before = set(os.listdir(outdir)) | set("c/" + f for f in os.listdir(coarsedir))
            animbefore = set(os.listdir(animdir))
            r = subprocess.run([export_bin, "--db", args.graphics, "--coarse-out", coarsedir,
                                "--anim-out", animdir, path, outdir], capture_output=True, text=True)
            res["t3rc"] = r.returncode
            res["t3out"] = r.stdout
            res["t3err"] = r.stderr
            new = []
            for f in os.listdir(outdir):
                if f.endswith(".shape") and f not in before:
                    new.append((f, os.path.join(outdir, f), False))
            for f in os.listdir(coarsedir):
                if f.endswith(".shape") and ("c/" + f) not in before:
                    new.append((f, os.path.join(coarsedir, f), True))
            res["t3new"] = new
            # Material animation (.anim) — the shape process's per-node NelExportAnimation step;
            # compare byte-exact against the final client references indexed from ~/core4_data.
            animres = []
            for f in os.listdir(animdir):
                if not f.endswith(".anim") or f in animbefore:
                    continue
                ref = g_animRefIdx.get(f)
                if not ref:
                    animres.append(("noref", f))
                elif open(os.path.join(animdir, f), "rb").read() == open(ref, "rb").read():
                    animres.append(("ident", f))
                else:
                    animres.append(("diff", f))
            res["animres"] = animres
        return res

    results = []
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for res in ex.map(run_file, corpus):
            results.append(res)

    # aggregate
    anim_ident = anim_diff = anim_noref = 0
    anim_diffs = []
    proj_produced = collections.defaultdict(dict)  # proj -> name -> (path, coarse)
    for res in results:
        if res.get("stub"):
            stubs += 1
            continue
        for tag, bn in res.get("animres", []):
            if tag == "ident":
                anim_ident += 1
            elif tag == "noref":
                anim_noref += 1
            else:
                anim_diff += 1
                anim_diffs.append(bn)
        if args.t1:
            if res.get("t1"):
                t1_pass += 1
            else:
                t1_fail += 1
                print("T1 FAIL %s" % res["path"])
        if args.t2:
            if res.get("t2"):
                t2_pass += 1
            else:
                t2_fail += 1
                print("T2 FAIL %s" % res["path"])
        if args.t3:
            if res.get("t3rc") != 0:
                export_fail.append(res["path"])
                print("T3 EXPORT FAIL %s" % res["path"])
                sys.stdout.write((res.get("t3err") or "")[-500:])
            for line in (res.get("t3out") or "").splitlines():
                if line.startswith("SKIPCLASS "):
                    parts = line.split()
                    skip_classes[parts[1]] += int(parts[2])
                elif line.startswith("MAPEXT "):
                    mapext_nodes.add((res["proj"], line.split(None, 1)[1] + ".shape"))
                    mapext_report.append("%s\t%s\t%s" % (res["proj"], res["path"], line.split(None, 1)[1]))
                elif line.startswith("LIGHTMAP "):
                    lightmap_nodes.add((res["proj"], line.split(None, 1)[1] + ".shape"))
            for line in (res.get("t3err") or "").splitlines():
                m = re.search(r"unhandled modifier \(0x([0-9a-f]+), 0x([0-9a-f]+)\)", line)
                if m:
                    warn_classes["mod:" + m.group(1)] += 1
                elif "object class" in line and "not implemented" in line:
                    m = re.search(r"class \(0x([0-9a-f]+), 0x([0-9a-f]+)\)", line)
                    if m:
                        warn_classes["obj:" + m.group(1)] += 1
                elif "shape-class base object" in line:
                    m = re.search(r"object \(0x([0-9a-f]+), 0x([0-9a-f]+)\)", line)
                    if m:
                        warn_classes["shape:" + m.group(1)] += 1
            for (name, path, coarse) in res.get("t3new") or []:
                proj_produced[res["proj"]][name.lower()] = (path, coarse)

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail; %d stubs" % (t1_pass, t1_fail, t2_pass, t2_fail, stubs))

    identical = floateq = diff = us_only = mapext = lightmap_ok = lightmap_diff = 0
    ref_missing = 0
    diff_samples = []
    if args.t3:
        projs = sorted(set(p for p, _ in corpus))
        for proj in projs:
            refs, coarse_refs = project_refs(args.ref, proj)
            mine = proj_produced.get(proj, {})
            for name, (path, coarse) in sorted(mine.items()):
                ref = (coarse_refs if coarse else refs).get(name) or refs.get(name) or coarse_refs.get(name)
                if not ref:
                    us_only += 1
                    continue
                a = open(path, "rb").read()
                b = open(ref, "rb").read()
                if a == b:
                    identical += 1
                elif (proj, name) in mapext_nodes:
                    # custom UVW plugin (Map Extender): reference UVs are garbage by design-doc
                    # pre-triage; bucketed out of the diff gate, listed in the asset report
                    mapext += 1
                elif (proj, name) in lightmap_nodes:
                    # lightmapped shape: the reference carries export-time lightmaps, the
                    # headless export is unmapped by design (pending the standalone lightmapper).
                    # Run --compare-lightmap-mask to promote from bucketed → structurally-verified
                    # when the base fields the lightmapper doesn't touch all match.
                    r = subprocess.run([export_bin, "--compare-lightmap-mask", path, ref], capture_output=True, text=True)
                    if r.returncode == 0:
                        lightmap_ok += 1
                    else:
                        lightmap_diff += 1
                        if len(diff_samples) < 25:
                            summary = ""
                            for line in r.stdout.splitlines():
                                ls = line.strip()
                                if ls.startswith(("VB", "material", "DefaultPos", "DefaultRotQuat", "DefaultScale", "rdrpass", "class:", "matrix")) and ("differ" in ls or "vs" in ls):
                                    summary += "|" + ls[:60]
                            diff_samples.append("LM %s:%s%s" % (proj, name, summary[:220]))
                else:
                    r = subprocess.run([export_bin, "--compare", path, ref], capture_output=True, text=True)
                    if r.returncode == 0:
                        floateq += 1
                    else:
                        diff += 1
                        if len(diff_samples) < 25:
                            summary = ""
                            for line in r.stdout.splitlines():
                                ls = line.strip()
                                if ls.startswith(("VB", "material", "DefaultPos", "DefaultRotQuat", "DefaultScale", "rdrpass", "class:", "matrix")) and ("differ" in ls or "vs" in ls):
                                    summary += "|" + ls[:60]
                            diff_samples.append("%s:%s%s" % (proj, name, summary[:220]))
            have = set(refs) | set(coarse_refs)
            missing = have - set(mine)
            ref_missing += len(missing)
        print("T3: %d exported: %d byte-identical, %d float-noise-eq, %d differ, %d mapext-bucketed, "
              "%d lightmap-verified + %d lightmap-diff, %d without reference; %d reference shapes not produced"
              % (identical + floateq + diff + mapext + lightmap_ok + lightmap_diff + us_only,
                 identical, floateq, diff, mapext, lightmap_ok, lightmap_diff, us_only, ref_missing))
        if mapext_report:
            report_path = os.path.join(args.out, "mapext_assets.txt")
            with open(report_path, "w") as fh:
                fh.write("# nodes carrying the custom UVW-mapping plugin modifier (mapext198m3.dlm);\n")
                fh.write("# reference .shape UVs are garbage for these (design doc pre-triage)\n")
                fh.write("\n".join(sorted(mapext_report)) + "\n")
            print("    mapext asset report: %s (%d nodes)" % (report_path, len(mapext_report)))
        if skip_classes:
            print("    skip classes: %s" % ", ".join("%s=%d" % kv for kv in skip_classes.most_common()))
        if warn_classes:
            print("    warn classes: %s" % ", ".join("%s=%d" % kv for kv in warn_classes.most_common(20)))
        for s in diff_samples:
            print("    DIFF %s" % s)
        print("    export failures: %d" % len(export_fail))
        print("    material anim (.anim): %d byte-identical, %d differ, %d without reference"
              % (anim_ident, anim_diff, anim_noref))
        for bn in anim_diffs[:20]:
            print("    ANIM DIFF %s" % bn)

    fails = t1_fail + t2_fail + len(export_fail)
    if args.gate_t3:
        fails += anim_diff  # material animations must be byte-exact against the references
        if identical + floateq + lightmap_ok < args.min_identical:
            fails += 1
        # structural diffs above the milestone budget are regressions (both the plain and the
        # lightmap-mask class count against the budget)
        fails += max(0, diff + lightmap_diff - args.max_diff)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
