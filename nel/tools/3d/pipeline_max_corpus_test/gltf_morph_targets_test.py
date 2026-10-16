#!/usr/bin/env python3
"""Behavior test for standard glTF morph targets (max2gltf viewing tier): meshes with MRM
morph targets (nel_bs_*) also emit per-primitive POSITION/NORMAL delta targets, mesh.weights
defaults (NeL percents / 100) and Blender-convention extras.targetNames. The nel_bs_* corner
streams stay authoritative for the exact tier. Asserted on the visage fixture:
  1. every primitive has one target per nel_bs geom; targetNames == nel_bs_names;
     weights == nel_bs_factors / 100
  2. POSITION deltas equal nel_bs_<i>_vertices - nel_vertices gathered via nel_vertex_ids,
     float-exact (both sides derive from the same CMeshBuild data)
"""

import json, os, shutil, struct, subprocess, sys

import shape_corpus

SKIP_CODE = 77
FIXTURE = "stuff/fyros/agents/actors/visages/fy_hof_visage.max"  # 28 morph targets


def main():
    graphics = shape_corpus.DEF_GRAPHICS
    bindir = shape_corpus.DEF_BIN
    args = sys.argv[1:]
    while args:
        if args[0] == "--graphics":
            graphics = args[1]; args = args[2:]
        elif args[0] == "--bin":
            bindir = args[1]; args = args[2:]
        else:
            print("unknown arg %s" % args[0]); return 2
    gltf_bin = os.path.join(bindir, "pipeline_max_export_gltf")
    max_path = os.path.join(graphics, FIXTURE)
    if not os.path.isfile(max_path):
        print("SKIP: corpus fixture not present")
        return SKIP_CODE
    if not os.path.isfile(gltf_bin):
        print("SKIP: pipeline_max_export_gltf not built")
        return SKIP_CODE

    out = "/tmp/gltf_morph_targets_test.%d" % os.getpid()
    os.makedirs(out, exist_ok=True)
    fails = []

    def check(name, ok, detail=""):
        print("%s %s%s" % ("ok  " if ok else "FAIL", name, (": " + detail) if detail else ""))
        if not ok:
            fails.append(name)

    try:
        r = subprocess.run([gltf_bin, "--db", graphics, max_path, out],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
        gltf_path = os.path.join(out, "fy_hof_visage.gltf")
        if r.returncode != 0 or not os.path.isfile(gltf_path):
            print(r.stdout)
            print("FAIL fixture export")
            return 1
        d = json.load(open(gltf_path))
        bin = open(os.path.join(out, "fy_hof_visage.bin"), "rb").read()

        def acc(idx, fmtc, ncomp):
            a = d["accessors"][idx]
            bv = d["bufferViews"][a["bufferView"]]
            return a["count"], struct.unpack_from("<%d%s" % (a["count"] * ncomp, fmtc),
                                                  bin, bv["byteOffset"])

        morph_meshes = [(i, m) for i, m in enumerate(d.get("meshes", []))
                        if "nel_bs_geoms" in m.get("extras", {})]
        check("fixture-has-morphs", len(morph_meshes) >= 1)

        for mi, m in morph_meshes:
            mex = m["extras"]
            ngeoms = mex["nel_bs_geoms"]
            node = next(n for n in d["nodes"] if n.get("mesh") == mi)
            names = mex.get("targetNames", m.get("extras", {}).get("targetNames"))
            check("target-counts", all(len(p.get("targets", [])) == ngeoms for p in m["primitives"])
                  and len(m.get("weights", [])) == ngeoms and len(names or []) == ngeoms,
                  "%d geoms" % ngeoms)
            nbn = node["extras"]["nel_bs_names"]
            nbf = node["extras"]["nel_bs_factors"]
            check("names-weights-match", names == nbn
                  and all(abs(m["weights"][k] - nbf[k] / 100.0) < 1e-6 for k in range(ngeoms)))

            _, base = acc(mex["nel_vertices"], "f", 3)
            ok_delta = True
            for k in range(ngeoms):
                _, bsv = acc(mex["nel_bs_%d_vertices" % k], "f", 3)
                for p in m["primitives"]:
                    _, vid = acc(p["extras"]["nel_vertex_ids"], "I", 1)
                    _, dpos = acc(p["targets"][k]["POSITION"], "f", 3)
                    for v in range(len(vid)):
                        ov = vid[v]
                        for c in range(3):
                            if dpos[v * 3 + c] != bsv[ov * 3 + c] - base[ov * 3 + c]:
                                ok_delta = False
            check("position-deltas-exact", ok_delta)
    finally:
        shutil.rmtree(out, ignore_errors=True)

    if fails:
        print("GLTF MORPH TARGETS: %d FAILURES: %s" % (len(fails), ", ".join(fails)))
        return 1
    print("GLTF MORPH TARGETS: all behaviors green")
    return 0


if __name__ == "__main__":
    sys.exit(main())
