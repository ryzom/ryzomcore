#!/usr/bin/env python3
"""Validate pipeline_max_export_ig's parametric primitive topology (--dump-prim) against the
Max 9 ground-truth dump in ~/prim_mesh_dataset/manifest.txt (gen_prim_mesh_dataset.ms).

Compares vertex positions (relative tolerance — the manifest prints ~6 significant digits) and
exact 1-based face indices for every non-modifier case in the manifest. The two Mirror-modifier
cases are skipped here (modifier evaluation is exercised by ig_corpus.py, not --dump-prim).

Self-skips (exit 77) when the dataset or the binary is missing.
"""

import os, subprocess, sys

SKIP_CODE = 77
DEF_BIN = os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin")
DEF_DATASET = os.path.expanduser("~/prim_mesh_dataset/manifest.txt")

# manifest case name -> (kind, args) for --dump-prim; params in pblock order
CASES = {
    "box_l2_w2.2_h3_s111": ("box", ["2.0", "2.2", "3.0", "1", "1", "1"]),
    "box_l0.4_w2_h2.8_s111": ("box", ["0.4", "2.0", "2.8", "1", "1", "1"]),
    "box_l2_w3_h1_s232": ("box", ["2.0", "3.0", "1.0", "3", "2", "2"]),
    "box_negheight": ("box", ["1.0", "1.0", "-1.0", "1", "1", "1"]),
    "box_neglength": ("box", ["-0.4", "2.0", "2.8", "1", "1", "1"]),
    "box_negwidth": ("box", ["0.4", "-2.0", "2.8", "1", "1", "1"]),
    "cyl_r7.8_h7.34_s16": ("cylinder", ["7.8", "7.34", "1", "1", "16"]),
    "cyl_r2_h3_hs2_s8": ("cylinder", ["2.0", "3.0", "2", "1", "8"]),
    "sphere_r1.5_s16": ("sphere", ["1.5", "16"]),
    "plane_l3.5_w2.3_s11": ("plane", ["3.5", "2.3", "1", "1"]),
    "plane_l2_w4_s23": ("plane", ["2.0", "4.0", "2", "3"]),
    "plane_l4_w4_s44": ("plane", ["4.0", "4.0", "4", "4"]),
}


def parse_manifest(path):
    meshes = {}
    name = None
    for line in open(path):
        parts = line.strip().split("\t")
        if parts[0] == "MESH":
            name = parts[1]
            meshes[name] = {"verts": [], "faces": []}
        elif parts[0] == "V" and name:
            meshes[name]["verts"].append(tuple(float(x) for x in parts[2:5]))
        elif parts[0] == "F" and name:
            meshes[name]["faces"].append(tuple(int(x) for x in parts[2:5]))
    return meshes


def main():
    binpath = os.path.join(sys.argv[sys.argv.index("--bin") + 1] if "--bin" in sys.argv else DEF_BIN,
                           "pipeline_max_export_ig")
    dataset = DEF_DATASET
    if not os.path.isfile(dataset):
        print("SKIP: ~/prim_mesh_dataset not present")
        return SKIP_CODE
    if not os.path.isfile(binpath):
        print("SKIP: pipeline_max_export_ig not built")
        return SKIP_CODE

    gt = parse_manifest(dataset)
    fails = 0
    for case, (kind, args) in CASES.items():
        if case not in gt:
            print("MISSING GT %s" % case)
            fails += 1
            continue
        r = subprocess.run([binpath, "--dump-prim", kind] + args, capture_output=True, text=True)
        ours = parse_manifest_text(r.stdout)
        g = gt[case]
        ok = True
        if len(ours["verts"]) != len(g["verts"]) or len(ours["faces"]) != len(g["faces"]):
            print("FAIL %s: counts verts %d/%d faces %d/%d" % (case, len(ours["verts"]), len(g["verts"]),
                                                               len(ours["faces"]), len(g["faces"])))
            ok = False
        else:
            for i, (a, b) in enumerate(zip(ours["verts"], g["verts"])):
                if any(abs(x - y) > 1e-4 * max(1.0, abs(y)) for x, y in zip(a, b)):
                    print("FAIL %s: vert %d %s vs %s" % (case, i + 1, a, b))
                    ok = False
                    break
            for i, (a, b) in enumerate(zip(ours["faces"], g["faces"])):
                if a != b:
                    print("FAIL %s: face %d %s vs %s" % (case, i + 1, a, b))
                    ok = False
                    break
        if ok:
            print("OK %s (%d verts, %d faces)" % (case, len(g["verts"]), len(g["faces"])))
        else:
            fails += 1
    print("%d/%d cases match" % (len(CASES) - fails, len(CASES)))
    return 1 if fails else 0


def parse_manifest_text(text):
    out = {"verts": [], "faces": []}
    for line in text.splitlines():
        parts = line.strip().split("\t")
        if parts[0] == "V":
            out["verts"].append(tuple(float(x) for x in parts[2:5]))
        elif parts[0] == "F":
            out["faces"].append(tuple(int(x) for x in parts[2:5]))
    return out


if __name__ == "__main__":
    sys.exit(main())
