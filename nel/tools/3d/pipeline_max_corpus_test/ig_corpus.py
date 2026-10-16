#!/usr/bin/env python3
"""Corpus driver for the ig .max -> .ig pipeline.

Enumerates every ig-source .max from the ryzomcore_leveldesign workspace configs
(IgOtherSourceDirectories + IgLandSourceDirectories over all projects — the exact listing
build_gamedata processes/ig/1_export.py drives), and runs test tiers:

  T1  structural roundtrip:  the pipeline_max_corpus_test binary, no --parse.
  T2  parse/build roundtrip: the pipeline_max_corpus_test binary with --parse.
  T3  .ig export:            pipeline_max_export_ig per file, then per exported ig:
        - direct tier: byte-compare against the raw intermediate export under --igref
          (~/pipeline_export/<group>/<project>/ig_static_other/<igname>.ig — the 1_export
          stage outputs of the original pipeline, run on the Max side). Byte-identity is
          the target; a field compare with --mask-uninit (SunContribution/Light[] are
          uninitialized memory in the reference exporter) classifies near-misses.
        - processed tier: structural compare (pipeline_max_export_ig --compare
          --mask-lighting) against the FINAL client data references under --ref
          (~/core4_data/*_ig + outgame + sky) — these are elevated + lighted, so lighting
          fields are masked (lights compared as subset) and Pos.z differences are masked
          (heightmap elevation class).

Defaults are the layout on Kaetemi's machine; override via CLI when running elsewhere.
"""

import argparse, os, re, subprocess, sys, collections
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_WORKSPACE = os.path.expanduser("~/ryzomcore_leveldesign/workspace")
DEF_REF = os.path.expanduser("~/core4_data")
DEF_IGREF = os.path.expanduser("~/pipeline_export")
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
    """Resolve a workspace-authored (Windows, case-insensitive) subdir under the checkout: the
    on-disk tree is lowercase (except possibly some filenames), so lowercase the directory path
    and fall back to the verbatim spelling (e.g. stuff/Generique/Decors/Constructions ->
    stuff/generique/decors/constructions)."""
    rel = rel.replace("\\", "/")
    for cand in (rel.lower(), rel):
        p = os.path.join(root, cand)
        if os.path.isdir(p):
            return p
    return None


def enumerate_corpus(graphics_dir, workspace_dir):
    """[(project, kind, source_subdir, max_path)] in workspace order, unique by path."""
    out = []
    seen = set()
    for root in ("common", "ecosystems", "continents", "shard"):
        base = os.path.join(workspace_dir, root)
        if not os.path.isdir(base):
            continue
        for name in sorted(os.listdir(base)):
            d = os.path.join(base, name, "directories.py")
            if not os.path.isfile(d):
                continue
            ns = load_dirs(d)
            if ns is None:
                continue
            for kind, dirs in (("other", ns.get("IgOtherSourceDirectories") or []),
                               ("land", ns.get("IgLandSourceDirectories") or [])):
                for dd in dirs:
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
                        out.append((root + "/" + name, kind, dd, p))
    return out


def is_ole(path):
    with open(path, "rb") as f:
        return f.read(8) == OLE_MAGIC


def find_refs(ref_dir):
    """ig name (lower, with .ig) -> [paths] over all reference dirs."""
    refs = collections.defaultdict(list)
    dirs = []
    for d in sorted(os.listdir(ref_dir)):
        if d.endswith("_ig") or d in ("outgame", "sky"):
            dirs.append(os.path.join(ref_dir, d))
    for d in dirs:
        for f in os.listdir(d):
            if f.endswith(".ig"):
                refs[f.lower()].append(os.path.join(d, f))
    return refs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=DEF_GRAPHICS)
    ap.add_argument("--workspace", default=DEF_WORKSPACE)
    ap.add_argument("--ref", default=DEF_REF)
    ap.add_argument("--igref", default=DEF_IGREF)
    ap.add_argument("--bin", default=DEF_BIN)
    ap.add_argument("--out", default="/tmp/ig_corpus_out.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true", help="fail on structural T3 regressions")
    ap.add_argument("--max-direct-diff", type=int, default=0,
                    help="allowed direct-ref field-compare failures under --gate-t3 (regression guard, 0 = strict — retired 2026-07-08 when the last open TR_hall_reu_vitrine_decors diff closed via §10w's Edit Mesh 0x0130 fix)")
    ap.add_argument("--only", default=None, help="substring filter on the .max path")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_ig")
    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print("SKIP: asset checkouts not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or not os.path.isfile(export_bin):
        print("SKIP: binaries not built")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics, args.workspace)
    if args.only:
        corpus = [c for c in corpus if args.only in c[3]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

    refs = find_refs(args.ref) if os.path.isdir(args.ref) else {}

    # .ps shape dirs for the clusterize FX-bbox path (export-era shapes first)
    ps_paths = [d for d in (os.path.join(args.igref, "common", "sfx", "ps"),
                            os.path.join(args.ref, "sfx")) if os.path.isdir(d)]

    t1_pass = t1_fail = t2_pass = t2_fail = 0
    stubs = 0
    exported = 0
    export_fail = []
    nothing = 0
    direct_match = direct_diff = direct_near = 0
    direct_noref = []
    direct_field_counter = collections.Counter()
    struct_match = struct_diff = 0
    noref = []
    diff_details = []
    field_counter = collections.Counter()

    os.makedirs(args.out, exist_ok=True)

    import glob as _glob

    # Per-file worker (parallel): runs T1/T2/T3 and the per-ig reference compares, returns a
    # result record. Each file exports into its OWN output dir so parallel runs never race on the
    # shared listing. Aggregation stays serial (below), in submission order, so output is
    # identical to a serial run.
    def process_file(indexed):
        idx, (proj, kind, subdir, path) = indexed
        res = {"proj": proj, "subdir": subdir, "path": path, "stub": False, "t1": None, "t2": None, "t3": None, "igs": []}
        if not is_ole(path):
            res["stub"] = True
            return res
        if args.t1:
            res["t1"] = subprocess.run([corpus_bin, path], capture_output=True).returncode == 0
        if args.t2:
            res["t2"] = subprocess.run([corpus_bin, "--parse", path], capture_output=True).returncode == 0
        if args.t3:
            outdir = os.path.join(args.out, "f%d" % idx)
            os.makedirs(outdir, exist_ok=True)
            cmd = [export_bin, "--db", args.graphics]
            for pp in ps_paths:
                cmd += ["--ps-path", pp]
            r = subprocess.run(cmd + [path, outdir], capture_output=True, text=True)
            if r.returncode == 3:
                res["t3"] = "nothing"
                return res
            if r.returncode != 0:
                res["t3"] = ("exportfail", r.stderr[-500:] if r.stderr else "")
                return res
            res["t3"] = "exported"
            leaf = proj.split("/")[-1]
            for igfile in sorted(f for f in os.listdir(outdir) if f.endswith(".ig")):
                ours = os.path.join(outdir, igfile)
                ig = {"name": igfile, "direct": None, "struct": None}
                cands = _glob.glob(os.path.join(args.igref, "*", leaf, "ig_static_other", igfile))
                if not cands:
                    cands = _glob.glob(os.path.join(args.igref, "*", "*", "ig_static_other", igfile))
                if cands:
                    dpath = cands[0]
                    if open(ours, "rb").read() == open(dpath, "rb").read():
                        ig["direct"] = ("match", "")
                    else:
                        r3 = subprocess.run([export_bin, "--compare", ours, dpath, "--mask-uninit"],
                                            capture_output=True, text=True)
                        ig["direct"] = ("near" if r3.returncode == 0 else "diff", r3.stdout)
                else:
                    ig["direct"] = ("noref", "")
                rlist = refs.get(igfile.lower())
                if not rlist:
                    ig["struct"] = ("noref", "")
                else:
                    r2 = subprocess.run([export_bin, "--compare", ours, rlist[0], "--mask-lighting", "--mask-z"],
                                        capture_output=True, text=True)
                    ig["struct"] = ("match" if r2.returncode == 0 else "diff", r2.stdout)
                res["igs"].append(ig)
        return res

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        results = list(ex.map(process_file, enumerate(corpus)))

    # Serial aggregation in submission order (deterministic output). The original serial driver
    # shared one output dir per subdir, so a same-named ig produced by more than one .max in the
    # same subdir was compared only for the first producer (subsequent ones were already on disk
    # and dropped by the new-files diff). Replicate that first-producer-per-(subdir, ig) dedup so
    # the parallel run's counts match the serial run exactly.
    seen_ig = set()
    for res in results:
        proj, path = res["proj"], res["path"]
        if res["stub"]:
            stubs += 1
            continue
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
                igfile = ig["name"]
                key = (res["subdir"], igfile)
                if key in seen_ig:
                    continue
                seen_ig.add(key)
                dk, dout = ig["direct"]
                if dk == "match":
                    direct_match += 1
                elif dk == "near":
                    direct_near += 1
                    diff_details.append(("direct-uninit-only", path, igfile))
                elif dk == "diff":
                    direct_diff += 1
                    fields = set()
                    for line in dout.splitlines():
                        for m in re.finditer(r" ([A-Za-z]+)\(", line):
                            fields.add(m.group(1))
                        if line.startswith("DIFF num"):
                            fields.add(line.split()[1])
                        for m in re.finditer(r" (LightMissing|Ambient|Diffuse|Specular|Attenuation|AnimatedLight|LightGroup|SpotDirection|SpotAngle|StaticLightEnabled|AvoidStaticLightPreCompute|LocalAmbientId|DontCastShadowForInterior|DontCastShadowForExterior|DontAddToScene|Visible)\b", line):
                            fields.add(m.group(1))
                    for f in fields:
                        direct_field_counter[f] += 1
                    diff_details.append(("direct", path, igfile, sorted(fields)))
                else:  # noref
                    direct_noref.append((proj, igfile))
                sk, sout = ig["struct"]
                if sk == "noref":
                    noref.append((proj, igfile))
                elif sk == "match":
                    struct_match += 1
                else:
                    struct_diff += 1
                    fields = set()
                    for line in sout.splitlines():
                        for m in re.finditer(r" ([A-Za-z]+)\(", line):
                            fields.add(m.group(1))
                        if line.startswith("DIFF num"):
                            fields.add(line.split()[1])
                    for f in fields:
                        field_counter[f] += 1
                    diff_details.append(("struct", path, igfile, sorted(fields)))

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail; %d stubs" % (t1_pass, t1_fail, t2_pass, t2_fail, stubs))
    if args.t3:
        print("T3: %d files exported, %d export failures, %d files with nothing to export" % (exported, len(export_fail), nothing))
        print("    direct-ref (raw intermediates): %d byte-identical, %d uninit-bytes-only, %d differ, %d without ref"
              % (direct_match, direct_near, direct_diff, len(direct_noref)))
        if direct_field_counter:
            print("    direct diff fields: %s" % ", ".join("%s=%d" % kv for kv in direct_field_counter.most_common()))
        print("    processed-ref (final client data, lighting+z masked): %d match, %d differ" % (struct_match, struct_diff))
        if noref:
            print("    no reference found: %s" % ", ".join("%s:%s" % (p, f) for p, f in noref[:10]))
        if field_counter:
            print("    struct diff fields: %s" % ", ".join("%s=%d" % kv for kv in field_counter.most_common()))
        for d in diff_details[:40]:
            print("    DIFF %s" % (d,))

    fails = t1_fail + t2_fail + len(export_fail)
    if args.gate_t3:
        if direct_diff > args.max_direct_diff:
            fails += direct_diff - args.max_direct_diff
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
