#!/usr/bin/env python3
"""Behavior test for the materials sidecar loader (wiki drafts/max2gltf_plan.md, decision
2026-07-14): a materials-only .gltf carrying nel_* material extras, bound by material NAME on
the assimp import route (mesh_export --materials).

Fixture: one corpus .max is exported by pipeline_max_export_gltf; from its .gltf we derive
  - a materials sidecar (asset + the materials array verbatim: name + PBR view + nel extras)
  - a "stripped" model (every nel_* extras key removed — simulates an artist-provided glTF;
    without asset.extras.nel_source it takes the assimp route naturally)

Asserted behaviors:
  1. every scene material with a sidecar entry binds ("bound from materials sidecar"), and the
     resulting .shape differs from a no-sidecar export (the exact materials actually landed)
  2. no sidecar -> plain assimp conversion still succeeds
  3. sidecar missing one name -> per-material warning + fallback, export still succeeds
  4. tampered nel_flags -> hard failure (codec flag verification)
  5. stacked sidecars (library then per-asset) -> later overrides earlier, all files load
  6. nel-extras input + --materials -> sidecar ignored with a warning, exact import proceeds
"""

import argparse, json, os, shutil, subprocess, sys

import shape_corpus  # same-directory corpus defaults

SKIP_CODE = 77
FIXTURE = "stuff/fyros/decors/constructions/fy_acc_ascenseur.max"  # 6 used + 4 unused-slot materials


def run(cmd, **kw):
    return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          text=True, timeout=300, **kw)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=shape_corpus.DEF_GRAPHICS)
    ap.add_argument("--bin", default=shape_corpus.DEF_BIN)
    args = ap.parse_args()
    graphics = args.graphics
    bindir = args.bin
    gltf_bin = os.path.join(bindir, "pipeline_max_export_gltf")
    import_bin = os.path.join(bindir, "mesh_export")
    max_path = os.path.join(graphics, FIXTURE)
    if not os.path.isfile(max_path):
        print("SKIP: corpus fixture not present")
        return SKIP_CODE
    for b in (gltf_bin, import_bin):
        if not os.path.isfile(b):
            print("SKIP: %s not built" % os.path.basename(b))
            return SKIP_CODE

    out = "/tmp/materials_sidecar_test.%d" % os.getpid()
    os.makedirs(out, exist_ok=True)
    fails = []

    def check(name, ok, detail=""):
        print("%s %s%s" % ("ok  " if ok else "FAIL", name, (": " + detail) if detail else ""))
        if not ok:
            fails.append(name)

    try:
        # Fixture generation
        r = run([gltf_bin, "--db", graphics, max_path, out])
        gltf_path = os.path.join(out, "fy_acc_ascenseur.gltf")
        if r.returncode != 0 or not os.path.isfile(gltf_path):
            print(r.stdout)
            print("FAIL fixture export")
            return 1
        doc = json.load(open(gltf_path))

        sidecar = {"asset": {"version": "2.0"}, "materials": doc["materials"]}
        json.dump(sidecar, open(os.path.join(out, "sidecar.gltf"), "w"))

        def strip(o):
            if isinstance(o, dict):
                ex = o.get("extras")
                if isinstance(ex, dict):
                    for k in [k for k in ex if k.startswith("nel_")]:
                        del ex[k]
                    if not ex:
                        del o["extras"]
                for v in o.values():
                    strip(v)
            elif isinstance(o, list):
                for v in o:
                    strip(v)
        stripped = json.load(open(gltf_path))
        strip(stripped)
        stripped_path = os.path.join(out, "stripped.gltf")
        json.dump(stripped, open(stripped_path, "w"))

        used = sorted({p["material"] for m in stripped.get("meshes", [])
                       for p in m.get("primitives", []) if "material" in p})
        used_names = [doc["materials"][i]["name"] for i in used]

        # 1. full sidecar: every used material binds, shapes produced
        r = run([import_bin, "-d", os.path.join(out, "o_sidecar"), "--materials",
                 os.path.join(out, "sidecar.gltf"), stripped_path])
        bound = r.stdout.count("bound from materials sidecar")
        check("bind-all", r.returncode == 0 and bound == len(used_names),
              "bound %d of %d" % (bound, len(used_names)))
        shape = os.path.join(out, "o_sidecar", "FY_acc_ascenseur.shape")
        check("shape-out", os.path.isfile(shape) and os.path.getsize(shape) > 0)

        # 2. no sidecar: fallback conversion, and the sidecar run differs from it
        r = run([import_bin, "-d", os.path.join(out, "o_plain"), stripped_path])
        plain = os.path.join(out, "o_plain", "FY_acc_ascenseur.shape")
        check("fallback-export", r.returncode == 0 and os.path.isfile(plain))
        check("sidecar-landed", os.path.isfile(plain) and
              open(shape, "rb").read() != open(plain, "rb").read())

        # 3. partial sidecar: dropped name warns + falls back, export succeeds
        partial = {"asset": {"version": "2.0"},
                   "materials": [m for m in doc["materials"] if m["name"] != used_names[0]]}
        json.dump(partial, open(os.path.join(out, "partial.gltf"), "w"))
        r = run([import_bin, "-d", os.path.join(out, "o_partial"), "--materials",
                 os.path.join(out, "partial.gltf"), stripped_path])
        check("partial-fallback", r.returncode == 0
              and r.stdout.count("not found in materials sidecar") == 1
              and r.stdout.count("bound from materials sidecar") == len(used_names) - 1)

        # 4. tampered nel_flags: hard failure
        tampered = json.loads(json.dumps(sidecar))
        tampered["materials"][0]["extras"]["nel_flags"] ^= 0x10
        json.dump(tampered, open(os.path.join(out, "tampered.gltf"), "w"))
        r = run([import_bin, "-d", os.path.join(out, "o_tamper"), "--materials",
                 os.path.join(out, "tampered.gltf"), stripped_path])
        check("tamper-fails", r.returncode != 0 and "failed to reconstruct" in r.stdout)

        # 5. stacked sidecars: later overrides earlier; a bad FIRST file still fails (both load)
        r = run([import_bin, "-d", os.path.join(out, "o_stack"),
                 "--materials", os.path.join(out, "partial.gltf"),
                 "--materials", os.path.join(out, "sidecar.gltf"), stripped_path])
        overrides = r.stdout.count("overrides earlier sidecar")
        check("stack-overrides", r.returncode == 0
              and overrides == len(partial["materials"])
              and r.stdout.count("bound from materials sidecar") == len(used_names))
        r = run([import_bin, "-d", os.path.join(out, "o_stack2"),
                 "--materials", os.path.join(out, "tampered.gltf"),
                 "--materials", os.path.join(out, "sidecar.gltf"), stripped_path])
        check("stack-loads-all", r.returncode != 0)

        # 6. nel-extras input: sidecar ignored with warning, exact import proceeds
        r = run([import_bin, "-d", os.path.join(out, "o_exact"), "--materials",
                 os.path.join(out, "sidecar.gltf"), gltf_path])
        check("exact-ignores", r.returncode == 0
              and "Materials sidecar ignored" in r.stdout
              and os.path.isfile(os.path.join(out, "o_exact", "fy_acc_ascenseur.shape")))
    finally:
        shutil.rmtree(out, ignore_errors=True)

    if fails:
        print("MATERIALS SIDECAR: %d FAILURES: %s" % (len(fails), ", ".join(fails)))
        return 1
    print("MATERIALS SIDECAR: all behaviors green")
    return 0


if __name__ == "__main__":
    sys.exit(main())
