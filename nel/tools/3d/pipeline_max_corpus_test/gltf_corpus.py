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
    if args.only:
        corpus = [c for c in corpus if args.only in c[1]]
    if args.project:
        corpus = [c for c in corpus if args.project in c[0]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

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
        g_dir = os.path.join(base, "gltf")
        v_dir = os.path.join(base, "via")
        vc_dir = os.path.join(base, "via_c")
        for d in (d_dir, dc_dir, g_dir, v_dir, vc_dir):
            os.makedirs(d, exist_ok=True)

        r = subprocess.run([shape_bin, "--db", args.graphics, "--coarse-out", dc_dir, path, d_dir],
                           capture_output=True, text=True)
        res["direct_rc"] = r.returncode
        res["direct_skips"] = skipclasses(r.stdout)

        r = subprocess.run([gltf_bin, "--db", args.graphics, path, g_dir],
                           capture_output=True, text=True)
        res["gltf_rc"] = r.returncode
        res["gltf_skips"] = skipclasses(r.stdout)
        gltf_path = os.path.join(g_dir, stem + ".gltf")

        via_skips = collections.Counter()
        if os.path.isfile(gltf_path):
            r = subprocess.run([import_bin, "-d", v_dir, "--coarse-dst", vc_dir, gltf_path],
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
        res["ident"] = ident
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
        if res.get("direct_rc") != 0 or res.get("gltf_rc") != 0 or res.get("import_rc") != 0:
            tool_fail.append("%s (rc d=%s g=%s i=%s)" % (res["path"], res.get("direct_rc"),
                                                         res.get("gltf_rc"), res.get("import_rc")))
        ident += res.get("ident", 0)
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
    print("GLTF DIFFERENTIAL: %d files (%d stubs); co-produced: %d byte-identical, %d float-eq, "
          "%d diff; %d direct-only (staged coverage), %d via-only"
          % (len(results), stubs, ident, len(floateq), len(diffs), direct_only, len(via_only)))
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
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
