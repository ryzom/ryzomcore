#!/usr/bin/env python3
"""Behavior test for the glTF-skins interop view (wiki drafts/max2gltf_plan.md): skinned meshes
in max2gltf output carry standard JOINTS_0/WEIGHTS_0 attributes plus a glTF skin object
(joints in BonesNames order, inverse bind matrices), and the biped rig nodes carry their
figure-mode rest pose instead of identity (biped TM controllers are not PRS).

Fixture: one skinned character .max exported by pipeline_max_export_gltf. Asserted:
  1. every skinned mesh (nel_skin_weights present) has JOINTS_0/WEIGHTS_0 and a node skin
  2. JOINTS_0/WEIGHTS_0 match nel_skin_joints/nel_skin_weights via nel_vertex_ids exactly
     (weighted slots; zero-weight slots must be joint 0 / weight 0 — the NeL side leaves
     them uninitialized)
  3. rest-pose skinning identity: for every skinned vertex, sum(w * world(joint) @ IBM) applied
     to POSITION stays within tolerance of POSITION — proves node TRS (figure-mode decode),
     IBMs, and the world-space vertices are mutually consistent the way a glTF viewer
     composes them
  4. the biped rig is not degenerate: a known biped bone has a non-identity local TRS
"""

import json, os, shutil, struct, subprocess, sys

import shape_corpus

SKIP_CODE = 77
FIXTURE = "stuff/fyros/agents/actors/male/fy_hom_armor00.max"  # 5 skinned meshes, biped rig
TOLERANCE = 1e-4  # meters; measured worst on the fixture is ~3.3e-7


def mul(A, B):
    C = [0.0] * 16
    for c in range(4):
        for r in range(4):
            C[c * 4 + r] = sum(A[k * 4 + r] * B[c * 4 + k] for k in range(4))
    return C


def trs(n):
    t = n.get("translation", [0, 0, 0])
    q = n.get("rotation", [0, 0, 0, 1])
    s = n.get("scale", [1, 1, 1])
    x, y, z, w = q
    R = [1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w), 0,
         2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w), 0,
         2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y), 0,
         0, 0, 0, 1]
    for c in range(3):
        for r in range(3):
            R[c * 4 + r] *= s[c]
    R[12], R[13], R[14] = t
    return R


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

    out = "/tmp/gltf_skins_test.%d" % os.getpid()
    os.makedirs(out, exist_ok=True)
    fails = []

    def check(name, ok, detail=""):
        print("%s %s%s" % ("ok  " if ok else "FAIL", name, (": " + detail) if detail else ""))
        if not ok:
            fails.append(name)

    try:
        r = subprocess.run([gltf_bin, "--db", graphics, max_path, out],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
        gltf_path = os.path.join(out, "fy_hom_armor00.gltf")
        if r.returncode != 0 or not os.path.isfile(gltf_path):
            print(r.stdout)
            print("FAIL fixture export")
            return 1
        d = json.load(open(gltf_path))
        bin = open(os.path.join(out, "fy_hom_armor00.bin"), "rb").read()

        def acc(idx, fmtc, ncomp):
            a = d["accessors"][idx]
            bv = d["bufferViews"][a["bufferView"]]
            return a["count"], struct.unpack_from("<%d%s" % (a["count"] * ncomp, fmtc),
                                                  bin, bv["byteOffset"])

        # world matrices per glTF composition
        world = [None] * len(d["nodes"])

        def compute(i, pw):
            world[i] = mul(pw, trs(d["nodes"][i]))
            for c in d["nodes"][i].get("children", []):
                compute(c, world[i])
        ident = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
        for rt in d["scenes"][0]["nodes"]:
            compute(rt, ident)

        # 1. coverage: every nel-skinned mesh has the interop view
        skinned_meshes = [i for i, m in enumerate(d.get("meshes", []))
                          if "nel_skin_weights" in m.get("extras", {})]
        nodes_by_mesh = {n["mesh"]: n for n in d["nodes"] if "mesh" in n}
        covered = [mi for mi in skinned_meshes
                   if "skin" in nodes_by_mesh.get(mi, {})
                   and all("JOINTS_0" in p.get("attributes", {}) and "WEIGHTS_0" in p.get("attributes", {})
                           for p in d["meshes"][mi]["primitives"])]
        check("skin-coverage", len(skinned_meshes) > 0 and covered == skinned_meshes,
              "%d of %d skinned meshes covered" % (len(covered), len(skinned_meshes)))

        # 2 + 3. exact extras match + rest-pose skinning identity
        worst = 0.0
        checked = 0
        mismatch = 0
        zero_slot_bad = 0
        for mi in covered:
            m = d["meshes"][mi]
            sk = d["skins"][nodes_by_mesh[mi]["skin"]]
            nibm, ibm = acc(sk["inverseBindMatrices"], "f", 16)
            JM = [mul(world[j], list(ibm[k * 16:(k + 1) * 16])) for k, j in enumerate(sk["joints"])]
            mex = m["extras"]
            _, nelw = acc(mex["nel_skin_weights"], "f", 4)
            _, nelj = acc(mex["nel_skin_joints"], "I", 1)
            for p in m["primitives"]:
                a = p["attributes"]
                npos, pos = acc(a["POSITION"], "f", 3)
                _, jo = acc(a["JOINTS_0"], "H", 4)
                _, we = acc(a["WEIGHTS_0"], "f", 4)
                _, vid = acc(p["extras"]["nel_vertex_ids"], "I", 1)
                for v in range(npos):
                    ov = vid[v]
                    for k in range(4):
                        w = we[v * 4 + k]
                        if w != 0.0:
                            if w != nelw[ov * 4 + k] or jo[v * 4 + k] != nelj[ov * 4 + k]:
                                mismatch += 1
                        elif jo[v * 4 + k] != 0:
                            zero_slot_bad += 1
                    x, y, z = pos[v * 3:v * 3 + 3]
                    sx = sy = sz = 0.0
                    for k in range(4):
                        w = we[v * 4 + k]
                        if w == 0.0:
                            continue
                        M = JM[jo[v * 4 + k]]
                        sx += w * (M[0] * x + M[4] * y + M[8] * z + M[12])
                        sy += w * (M[1] * x + M[5] * y + M[9] * z + M[13])
                        sz += w * (M[2] * x + M[6] * y + M[10] * z + M[14])
                    e = max(abs(sx - x), abs(sy - y), abs(sz - z))
                    if e > worst:
                        worst = e
                    checked += 1
        check("extras-match", checked > 0 and mismatch == 0 and zero_slot_bad == 0,
              "%d verts, %d mismatches, %d bad zero slots" % (checked, mismatch, zero_slot_bad))
        check("rest-pose-identity", worst < TOLERANCE, "worst deviation %.3g m" % worst)

        # 4. rig not degenerate
        pelvis = next((n for n in d["nodes"] if n["name"] == "Bip01 Pelvis"), None)
        check("rig-rest-pose", pelvis is not None
              and pelvis.get("rotation", [0, 0, 0, 1]) != [0, 0, 0, 1])
    finally:
        shutil.rmtree(out, ignore_errors=True)

    if fails:
        print("GLTF SKINS: %d FAILURES: %s" % (len(fails), ", ".join(fails)))
        return 1
    print("GLTF SKINS: all behaviors green")
    return 0


if __name__ == "__main__":
    sys.exit(main())
