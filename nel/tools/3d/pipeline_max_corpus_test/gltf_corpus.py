#!/usr/bin/env python3
"""Differential gate for the max2gltf route (wiki drafts/max2gltf_plan.md).

For every shape-corpus .max (the same enumeration shape_corpus.py drives):

  direct:  pipeline_max_export_shape  .max -> .shape            (the frozen reference route)
  via:     pipeline_max_export_gltf   .max -> .gltf + .bin
           mesh_export                .gltf -> .shape           (nel-extras exact-tier import)

Every shape produced by BOTH routes must be BYTE-IDENTICAL (same extraction, same NL3D build
calls, same host — the glTF hop is supposed to be lossless; a float-noise verdict already means
an encoding leak). Shapes only the direct route produces are bucketed by the via-route's skip
classes (multilod, morpher-mrm, ... — staged coverage, tracked not failed); shapes only the via
route produces are always failures.

Per-file outputs are deleted right after comparison (disk stays flat over the 2850-file
corpus); pass --keep-diff to retain the artifacts of mismatching files for triage.
"""

import argparse, os, shutil, subprocess, sys, collections

import shape_corpus  # same-directory corpus enumeration + helpers
import ig_corpus     # ig-process source enumeration (the standalone 116-file ig corpus)
import anim_corpus   # anim-source enumeration (fauna/characters/sky AnimSourceDirectories)
import zone_corpus   # ligo zone source enumeration (per-ecosystem zonematerial/transition/special)

SKIP_CODE = 77


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=shape_corpus.DEF_GRAPHICS)
    ap.add_argument("--workspace", default=shape_corpus.DEF_WORKSPACE)
    ap.add_argument("--bin", default=shape_corpus.DEF_BIN)
    ap.add_argument("--out", default="/tmp/gltf_corpus_out.%d" % os.getpid())
    ap.add_argument("--only", default=None, help="substring filter on the .max path")
    ap.add_argument("--project", default=None, help="substring filter on the project")
    ap.add_argument("--keep-diff", action="store_true",
                    help="keep the per-file artifacts of mismatching files")
    ap.add_argument("--gate", action="store_true", help="fail on any diff / via-only shape")
    ap.add_argument("--min-identical", type=int, default=0,
                    help="minimum byte-identical co-produced shape count for --gate")
    ap.add_argument("--min-anims", type=int, default=0,
                    help="minimum byte-identical co-produced anim count for --gate")
    ap.add_argument("--min-zones", type=int, default=0,
                    help="minimum byte-identical co-produced zone file count for --gate")
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    args = ap.parse_args()

    shape_bin = os.path.join(args.bin, "pipeline_max_export_shape")
    gltf_bin = os.path.join(args.bin, "pipeline_max_export_gltf")
    import_bin = os.path.join(args.bin, "mesh_export")
    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print("SKIP: asset checkouts not present")
        return SKIP_CODE
    for b in (shape_bin, gltf_bin, import_bin):
        if not os.path.isfile(b):
            print("SKIP: %s not built" % os.path.basename(b))
            return SKIP_CODE

    corpus = shape_corpus.enumerate_corpus(args.graphics, args.workspace)
    # The ig-process sources (mostly disjoint from the shape corpus): igs ride the same
    # differential, direct pipeline_max_export_ig .ig vs the glTF's nel_igs blob re-emitted by
    # mesh_export. Files present in both corpora run once with both comparisons.
    ig_paths = set()
    for (proj, kind, d, path) in ig_corpus.enumerate_corpus(args.graphics, args.workspace):
        ig_paths.add(path)
    shape_paths = set(p for (_, p) in corpus)
    corpus = corpus + [("ig/" + proj, path)
                       for (proj, kind, d, path) in ig_corpus.enumerate_corpus(args.graphics, args.workspace)
                       if path not in shape_paths]
    # The anim-process sources (mostly disjoint from the shape corpus): the .anim rides the
    # same differential, direct pipeline_max_export_anim vs the glTF's nel_anim blob re-emitted
    # by mesh_export.
    anim_paths = set()
    seen_paths = set(p for (_, p) in corpus)
    anim_extra = []
    for (group, d, name, path) in anim_corpus.enumerate_corpus(args.graphics, args.workspace):
        anim_paths.add(path)
        if path not in seen_paths:
            seen_paths.add(path)
            anim_extra.append(("anim/" + group, path))
    corpus = corpus + anim_extra
    # The ligo zone sources: .zone/.ligozone ride the differential as the nel_zones blob list,
    # direct pipeline_max_export_zone --ligo vs the blob re-emission. Per-ecosystem smallbank.
    zone_banks = {}
    zone_extra = []
    for (eco, path) in zone_corpus.enumerate_corpus(args.graphics):
        zone_banks[path] = os.path.join(zone_corpus.DEF_REF,
                                        "ecosystems", eco, "smallbank", eco + ".smallbank")
        if path not in seen_paths:
            seen_paths.add(path)
            zone_extra.append(("zone/" + eco, path))
    corpus = corpus + zone_extra
    if args.only:
        corpus = [c for c in corpus if args.only in c[1]]
    if args.project:
        corpus = [c for c in corpus if args.project in c[0]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

    ig_bin = os.path.join(args.bin, "pipeline_max_export_ig")
    anim_bin = os.path.join(args.bin, "pipeline_max_export_anim")
    zone_bin = os.path.join(args.bin, "pipeline_max_export_zone")
    ps_paths = [d for d in (os.path.expanduser("~/pipeline_export/common/sfx/ps"),) if os.path.isdir(d)]

    os.makedirs(args.out, exist_ok=True)

    def shapes_in(d):
        out = {}
        if os.path.isdir(d):
            for f in os.listdir(d):
                if f.endswith(".shape"):
                    out[f] = os.path.join(d, f)
        return out

    def skipclasses(text):
        out = collections.Counter()
        for line in (text or "").splitlines():
            if line.startswith("SKIPCLASS "):
                parts = line.split()
                out[parts[1]] += int(parts[2])
        return out

    def run_file(item):
        idx, (proj, path) = item
        res = {"proj": proj, "path": path}
        if not shape_corpus.is_ole(path):
            res["stub"] = True
            return res
        stem = os.path.splitext(os.path.basename(path))[0].lower()
        base = os.path.join(args.out, "%05d_%s" % (idx, stem))
        d_dir = os.path.join(base, "direct")
        dc_dir = os.path.join(base, "direct_c")
        di_dir = os.path.join(base, "direct_ig")
        da_dir = os.path.join(base, "direct_anim")
        dz_dir = os.path.join(base, "direct_zone")
        g_dir = os.path.join(base, "gltf")
        v_dir = os.path.join(base, "via")
        vc_dir = os.path.join(base, "via_c")
        vi_dir = os.path.join(base, "via_ig")
        va_dir = os.path.join(base, "via_anim")
        vz_dir = os.path.join(base, "via_zone")
        for d in (d_dir, dc_dir, di_dir, da_dir, dz_dir, g_dir, v_dir, vc_dir, vi_dir, va_dir, vz_dir):
            os.makedirs(d, exist_ok=True)

        r = subprocess.run([shape_bin, "--db", args.graphics, "--coarse-out", dc_dir, path, d_dir],
                           capture_output=True, text=True)
        res["direct_rc"] = r.returncode
        res["direct_skips"] = skipclasses(r.stdout)

        ps_args = []
        for pp in ps_paths:
            ps_args += ["--ps-path", pp]
        zone_args = []
        if path in zone_banks:
            zone_args = ["--zone-bank", zone_banks[path]]
        r = subprocess.run([gltf_bin, "--db", args.graphics] + ps_args + zone_args + [path, g_dir],
                           capture_output=True, text=True)
        res["gltf_rc"] = r.returncode
        res["gltf_skips"] = skipclasses(r.stdout)
        gltf_path = os.path.join(g_dir, stem + ".gltf")

        via_skips = collections.Counter()
        if os.path.isfile(gltf_path):
            r = subprocess.run([import_bin, "-d", v_dir, "--coarse-dst", vc_dir,
                                "--ig-dst", vi_dir, "--anim-dst", va_dir,
                                "--zone-dst", vz_dir, gltf_path],
                               capture_output=True, text=True)
            res["import_rc"] = r.returncode
            via_skips = skipclasses(r.stdout)
        else:
            res["import_rc"] = -1
        res["via_skips"] = via_skips

        direct = shapes_in(d_dir)
        direct_c = shapes_in(dc_dir)
        via = shapes_in(v_dir)
        via_c = shapes_in(vc_dir)

        # IG differential: the direct .ig set vs the glTF nel_igs blob re-emission. Run the
        # direct exporter for the ig-corpus files and for any file whose glTF carried igs.
        def igs_in(d):
            out = {}
            if os.path.isdir(d):
                for f in os.listdir(d):
                    if f.endswith(".ig"):
                        out[f] = os.path.join(d, f)
            return out

        via_igs = igs_in(vi_dir)
        if path in ig_paths or via_igs:
            r = subprocess.run([ig_bin, "--db", args.graphics] + ps_args + [path, di_dir],
                               capture_output=True, text=True)
            # exit 3 = nothing to export (not an error)
            res["ig_rc"] = 0 if r.returncode in (0, 3) else r.returncode
        direct_igs = igs_in(di_dir)

        # Anim differential: direct .anim vs the glTF nel_anim blob re-emission. Run the direct
        # exporter for the anim-corpus files and for any file whose glTF carried an anim.
        def anims_in(d):
            out = {}
            if os.path.isdir(d):
                for f in os.listdir(d):
                    if f.endswith(".anim"):
                        out[f] = os.path.join(d, f)
            return out

        via_anims = anims_in(va_dir)
        if path in anim_paths or via_anims:
            r = subprocess.run([anim_bin, path, os.path.join(da_dir, stem + ".anim")],
                               capture_output=True, text=True, timeout=300)
            # exit 3 = nothing to export (not an error)
            res["anim_rc"] = 0 if r.returncode in (0, 3) else r.returncode
        direct_anims = anims_in(da_dir)

        # Zone differential: direct --ligo output tree vs the glTF nel_zones blob re-emission,
        # relative paths (zones/*.zone + zoneligos/*.ligozone) compared byte-wise. A file the
        # direct tool refuses (authoring errors, e.g. duplicate NelPatchMesh) is symmetric —
        # the via route emits no nel_zones for it either.
        def zones_in(d):
            out = {}
            for sub in ("zones", "zoneligos"):
                sd = os.path.join(d, sub)
                if os.path.isdir(sd):
                    for f in os.listdir(sd):
                        out[sub + "/" + f] = os.path.join(sd, f)
            return out

        via_zones = zones_in(vz_dir)
        zone_refused = False
        if path in zone_banks:
            r = subprocess.run([zone_bin, "--ligo", dz_dir, "--bank", zone_banks[path], path],
                               capture_output=True, text=True, timeout=300)
            if r.returncode != 0 and not via_zones:
                zone_refused = True  # symmetric refusal (direct errors, via emitted nothing)
            elif r.returncode != 0:
                res["zone_rc"] = r.returncode
        direct_zones = {} if zone_refused else zones_in(dz_dir)

        ident = 0
        floateq = []
        diff = []
        direct_only = []
        via_only = []
        mismatch = False
        for (dmap, vmap) in ((direct, via), (direct_c, via_c)):
            for name, dpath in sorted(dmap.items()):
                vpath = vmap.get(name)
                if not vpath:
                    direct_only.append(name)
                    continue
                a = open(dpath, "rb").read()
                b = open(vpath, "rb").read()
                if a == b:
                    ident += 1
                    continue
                mismatch = True
                r = subprocess.run([shape_bin, "--compare", dpath, vpath],
                                   capture_output=True, text=True)
                summary = ""
                for line in r.stdout.splitlines():
                    ls = line.strip()
                    if ("differ" in ls or "vs" in ls) and len(summary) < 200:
                        summary += "|" + ls[:60]
                (floateq if r.returncode == 0 else diff).append(name + summary)
            for name in sorted(vmap):
                if name not in dmap:
                    via_only.append(name)
                    mismatch = True
        ig_ident = 0
        for name, dpath in sorted(direct_igs.items()):
            vpath = via_igs.get(name)
            if not vpath:
                # unlike shapes there is no staged-coverage class for igs — the writer embeds
                # every ig the direct route builds, so a missing one is a defect
                diff.append("ig-missing:" + name)
                mismatch = True
                continue
            if open(dpath, "rb").read() == open(vpath, "rb").read():
                ig_ident += 1
            else:
                mismatch = True
                diff.append("ig:" + name)
        for name in sorted(via_igs):
            if name not in direct_igs:
                via_only.append("ig:" + name)
                mismatch = True
        anim_ident = 0
        for name, dpath in sorted(direct_anims.items()):
            vpath = via_anims.get(name)
            if not vpath:
                # like igs, the writer embeds the anim the direct route builds — a missing one
                # is a defect, not staged coverage
                diff.append("anim-missing:" + name)
                mismatch = True
                continue
            if open(dpath, "rb").read() == open(vpath, "rb").read():
                anim_ident += 1
            else:
                mismatch = True
                diff.append("anim:" + name)
        for name in sorted(via_anims):
            if name not in direct_anims:
                via_only.append("anim:" + name)
                mismatch = True
        zone_ident = 0
        for name, dpath in sorted(direct_zones.items()):
            vpath = via_zones.get(name)
            if not vpath:
                diff.append("zone-missing:" + name)
                mismatch = True
                continue
            if open(dpath, "rb").read() == open(vpath, "rb").read():
                zone_ident += 1
            else:
                mismatch = True
                diff.append("zone:" + name)
        for name in sorted(via_zones):
            if name not in direct_zones:
                via_only.append("zone:" + name)
                mismatch = True

        res["ident"] = ident
        res["ig_ident"] = ig_ident
        res["anim_ident"] = anim_ident
        res["zone_ident"] = zone_ident
        res["floateq"] = floateq
        res["diff"] = diff
        res["direct_only"] = direct_only
        res["via_only"] = via_only

        if mismatch and args.keep_diff:
            res["kept"] = base
        else:
            shutil.rmtree(base, ignore_errors=True)
        return res

    from concurrent.futures import ThreadPoolExecutor
    results = []
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for res in ex.map(run_file, enumerate(corpus)):
            results.append(res)

    stubs = 0
    ident = 0
    ig_ident = 0
    anim_ident = 0
    zone_ident = 0
    floateq = []
    diffs = []
    via_only = []
    direct_only = 0
    tool_fail = []
    skip_direct = collections.Counter()
    skip_via = collections.Counter()
    for res in results:
        if res.get("stub"):
            stubs += 1
            continue
        if res.get("direct_rc") != 0 or res.get("gltf_rc") != 0 or res.get("import_rc") != 0 \
           or res.get("ig_rc", 0) != 0 or res.get("anim_rc", 0) != 0 or res.get("zone_rc", 0) != 0:
            tool_fail.append("%s (rc d=%s g=%s i=%s ig=%s a=%s z=%s)"
                             % (res["path"], res.get("direct_rc"), res.get("gltf_rc"),
                                res.get("import_rc"), res.get("ig_rc", 0), res.get("anim_rc", 0),
                                res.get("zone_rc", 0)))
        ident += res.get("ident", 0)
        ig_ident += res.get("ig_ident", 0)
        anim_ident += res.get("anim_ident", 0)
        zone_ident += res.get("zone_ident", 0)
        for n in res.get("floateq", []):
            floateq.append("%s:%s" % (res["proj"], n))
        for n in res.get("diff", []):
            diffs.append("%s:%s" % (res["proj"], n))
        for n in res.get("via_only", []):
            via_only.append("%s:%s:%s" % (res["proj"], os.path.basename(res["path"]), n))
        direct_only += len(res.get("direct_only", []))
        skip_direct.update(res.get("direct_skips", {}))
        skip_via.update(res.get("gltf_skips", {}))
        skip_via.update(res.get("via_skips", {}))
        if res.get("kept"):
            print("KEPT %s" % res["kept"])

    print()
    print("GLTF DIFFERENTIAL: %d files (%d stubs); co-produced: %d byte-identical shapes + "
          "%d byte-identical igs + %d byte-identical anims + %d byte-identical zone files, "
          "%d float-eq, %d diff; %d direct-only (staged coverage), %d via-only"
          % (len(results), stubs, ident, ig_ident, anim_ident, zone_ident, len(floateq),
             len(diffs), direct_only, len(via_only)))
    if skip_direct:
        print("    direct skip classes: %s" % ", ".join("%s=%d" % kv for kv in skip_direct.most_common()))
    if skip_via:
        print("    via skip classes: %s" % ", ".join("%s=%d" % kv for kv in skip_via.most_common()))
    for s in floateq[:20]:
        print("    FLOATEQ %s" % s)
    for s in diffs[:20]:
        print("    DIFF %s" % s)
    for s in via_only[:20]:
        print("    VIA-ONLY %s" % s)
    for s in tool_fail[:10]:
        print("    TOOL FAIL %s" % s)

    fails = len(tool_fail)
    if args.gate:
        # The glTF hop must be lossless: every co-produced shape byte-identical. Float-noise
        # against our own direct output is an encoding leak, not tolerance material.
        fails += len(diffs) + len(floateq) + len(via_only)
        if ident < args.min_identical:
            fails += 1
        if anim_ident < args.min_anims:
            fails += 1
        if zone_ident < args.min_zones:
            fails += 1
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
