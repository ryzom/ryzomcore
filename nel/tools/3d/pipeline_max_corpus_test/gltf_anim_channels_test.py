#!/usr/bin/env python3
"""Behavior test for the sampled glTF animation channels (max2gltf viewing tier): TRS tracks
from the nel_anim blob LINEAR-sampled at 30 fps onto the matching rig nodes. The blob stays
authoritative; this view is lossy by design. Asserted on one real clip fixture:
  1. an animations[] entry exists with rotation/translation channels on many distinct nodes
  2. every channel target is a valid node, every sampler input is strictly increasing with
     min/max present, input count == output count
  3. rotation outputs are unit quaternions with hemisphere-aligned consecutive samples
     (no long-way slerp flips)
"""

import json, os, shutil, struct, subprocess, sys

import shape_corpus

SKIP_CODE = 77
FIXTURE = "stuff/fyros/agents/actors/male/animation/anims/fy_hom_a2m_idle1_1.max"


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

    out = "/tmp/gltf_anim_channels_test.%d" % os.getpid()
    os.makedirs(out, exist_ok=True)
    fails = []

    def check(name, ok, detail=""):
        print("%s %s%s" % ("ok  " if ok else "FAIL", name, (": " + detail) if detail else ""))
        if not ok:
            fails.append(name)

    try:
        r = subprocess.run([gltf_bin, "--db", graphics, max_path, out],
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
        gltf_path = os.path.join(out, "fy_hom_a2m_idle1_1.gltf")
        if r.returncode != 0 or not os.path.isfile(gltf_path):
            print(r.stdout)
            print("FAIL fixture export")
            return 1
        d = json.load(open(gltf_path))
        bin = open(os.path.join(out, "fy_hom_a2m_idle1_1.bin"), "rb").read()

        def acc(idx, fmtc, ncomp):
            a = d["accessors"][idx]
            bv = d["bufferViews"][a["bufferView"]]
            return a["count"], struct.unpack_from("<%d%s" % (a["count"] * ncomp, fmtc),
                                                  bin, bv["byteOffset"])

        anims = d.get("animations", [])
        check("anim-present", len(anims) == 1)
        if not anims:
            return 1
        a = anims[0]
        ch = a["channels"]
        paths = {}
        for c in ch:
            paths[c["target"]["path"]] = paths.get(c["target"]["path"], 0) + 1
        targets = set(c["target"]["node"] for c in ch)
        check("channel-shape", paths.get("rotation", 0) > 50 and paths.get("translation", 0) >= 1
              and len(targets) > 50, "%s on %d nodes" % (paths, len(targets)))

        bad_target = bad_time = bad_pair = bad_quat = bad_flip = 0
        ncomp_of = {"rotation": 4, "translation": 3, "scale": 3}
        for c in ch:
            if not (0 <= c["target"]["node"] < len(d["nodes"])):
                bad_target += 1
                continue
            s = a["samplers"][c["sampler"]]
            nt, times = acc(s["input"], "f", 1)
            ai = d["accessors"][s["input"]]
            if "min" not in ai or "max" not in ai or any(times[i] >= times[i + 1] for i in range(nt - 1)):
                bad_time += 1
            nc = ncomp_of[c["target"]["path"]]
            nv, vals = acc(s["output"], "f", nc)
            if nv != nt:
                bad_pair += 1
            if c["target"]["path"] == "rotation":
                prev = None
                for v in range(nv):
                    q = vals[v * 4:v * 4 + 4]
                    n2 = sum(x * x for x in q)
                    if abs(n2 - 1.0) > 1e-3:
                        bad_quat += 1
                    if prev and sum(q[i] * prev[i] for i in range(4)) < 0:
                        bad_flip += 1
                    prev = q
        check("channels-wellformed", bad_target == 0 and bad_time == 0 and bad_pair == 0,
              "target %d, time %d, pair %d" % (bad_target, bad_time, bad_pair))
        check("quats-unit-aligned", bad_quat == 0 and bad_flip == 0,
              "nonunit %d, flips %d" % (bad_quat, bad_flip))
    finally:
        shutil.rmtree(out, ignore_errors=True)

    if fails:
        print("GLTF ANIM CHANNELS: %d FAILURES: %s" % (len(fails), ", ".join(fails)))
        return 1
    print("GLTF ANIM CHANNELS: all behaviors green")
    return 0


if __name__ == "__main__":
    sys.exit(main())
