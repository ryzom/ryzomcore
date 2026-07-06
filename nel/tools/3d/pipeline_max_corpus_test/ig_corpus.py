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
    ap.add_argument("--only", default=None, help="substring filter on the .max path")
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

    for proj, kind, subdir, path in corpus:
        name = os.path.basename(path)
        if not is_ole(path):
            stubs += 1
            continue
        if args.t1:
            r = subprocess.run([corpus_bin, path], capture_output=True)
            if r.returncode == 0:
                t1_pass += 1
            else:
                t1_fail += 1
                print("T1 FAIL %s" % path)
        if args.t2:
            r = subprocess.run([corpus_bin, path, "--parse"], capture_output=True)
            if r.returncode == 0:
                t2_pass += 1
            else:
                t2_fail += 1
                print("T2 FAIL %s" % path)
        if args.t3:
            flat = subdir.replace("/", "_").replace(" ", "_")
            outdir = os.path.join(args.out, flat)
            os.makedirs(outdir, exist_ok=True)
            before = set(os.listdir(outdir))
            r = subprocess.run([export_bin, "--db", args.graphics, path, outdir], capture_output=True, text=True)
            if r.returncode == 3:
                nothing += 1
                continue
            if r.returncode != 0:
                export_fail.append(path)
                print("T3 EXPORT FAIL %s" % path)
                sys.stdout.write(r.stderr[-500:] if r.stderr else "")
                continue
            exported += 1
            # per exported ig (only those THIS file produced): reference comparison
            leaf = proj.split("/")[-1]
            for igfile in sorted(set(os.listdir(outdir)) - before):
                if not igfile.endswith(".ig"):
                    continue
                ours = os.path.join(outdir, igfile)
                # direct tier: raw intermediate reference from the original pipeline's
                # ig_static_other export directory of the owning project (group folder name may
                # differ, e.g. "continents - Copy"); falls back to a global search by ig name.
                import glob as _glob
                cands = _glob.glob(os.path.join(args.igref, "*", leaf, "ig_static_other", igfile))
                if not cands:
                    cands = _glob.glob(os.path.join(args.igref, "*", "*", "ig_static_other", igfile))
                if cands:
                    dpath = cands[0]
                    if open(ours, "rb").read() == open(dpath, "rb").read():
                        direct_match += 1
                    else:
                        r3 = subprocess.run([export_bin, "--compare", ours, dpath, "--mask-uninit"],
                                            capture_output=True, text=True)
                        if r3.returncode == 0:
                            direct_near += 1
                            diff_details.append(("direct-uninit-only", path, igfile))
                        else:
                            direct_diff += 1
                            fields = set()
                            for line in r3.stdout.splitlines():
                                for m in re.finditer(r" ([A-Za-z]+)\(", line):
                                    fields.add(m.group(1))
                                if line.startswith("DIFF num"):
                                    fields.add(line.split()[1])
                                for m in re.finditer(r" (LightMissing|Ambient|Diffuse|Specular|Attenuation|AnimatedLight|LightGroup|SpotDirection|SpotAngle|StaticLightEnabled|AvoidStaticLightPreCompute|LocalAmbientId|DontCastShadowForInterior|DontCastShadowForExterior|DontAddToScene|Visible)\b", line):
                                    fields.add(m.group(1))
                            for f in fields:
                                direct_field_counter[f] += 1
                            diff_details.append(("direct", path, igfile, sorted(fields)))
                else:
                    direct_noref.append((proj, igfile))
                # processed tier: structural vs final client refs
                rlist = refs.get(igfile.lower())
                if not rlist:
                    noref.append((proj, igfile))
                    continue
                r2 = subprocess.run([export_bin, "--compare", ours, rlist[0], "--mask-lighting", "--mask-z"],
                                    capture_output=True, text=True)
                if r2.returncode == 0:
                    struct_match += 1
                else:
                    struct_diff += 1
                    fields = set()
                    for line in r2.stdout.splitlines():
                        for m in re.finditer(r" ([A-Za-z]+)\(", line):
                            fields.add(m.group(1))
                        if line.startswith("DIFF num"):
                            fields.add(line.split()[1])
                        m2 = re.search(r" (Clusters)\(", line)
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
        fails += direct_diff
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
