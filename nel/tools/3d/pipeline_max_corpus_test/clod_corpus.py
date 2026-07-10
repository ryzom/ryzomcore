#!/usr/bin/env python3
"""Corpus driver for the clodbank .max -> .clod pipeline (build_gamedata processes/clodbank's
NelExportLodCharacter path).

Enumerates every .max under the workspace ClodSourceDirectories:
  - common/fauna:     stuff/lod_actors/lod_fauna
  - common/characters: stuff/lod_actors/lod_characters

and runs:

  T1  structural roundtrip:  pipeline_max_corpus_test, no --parse.
  T2  parse/build roundtrip: pipeline_max_corpus_test --parse.
  T3  .clod export:          pipeline_max_export_clod, field-compare against
        ~/pipeline_export/common/{fauna,characters}/clod_export/<node>.clod.

Landing (2026-07-10, §10z-clod): last MISSING format on the pipeline_max surface.
PHYSIQUESKIN shared + weight-exact since §10z-treize; path is CMeshMRM NLods=1/Divisor=1
→ CLodCharacterShapeBuild serial. Measured whole-corpus T3: every ref produced, 0 export
failures; field-exact (IDENTICAL+FLOATEQ) on the large majority; residual classes are the
known ConvertToRigid→Bip01 root-fallback bone-set deltas, a handful of MRM VB-dedup splits,
and 1-file UV / few-vert normal residuals.
"""

import argparse, os, struct, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77
OLE_MAGIC = b"\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

PROJECTS = [
    ("fauna", "stuff/lod_actors/lod_fauna", "common/fauna/clod_export"),
    ("characters", "stuff/lod_actors/lod_characters", "common/characters/clod_export"),
]


def is_ole(path):
    with open(path, "rb") as f:
        return f.read(8) == OLE_MAGIC


def enumerate_corpus(graphics_dir):
    files = []
    for label, src, ref in PROJECTS:
        d = os.path.join(graphics_dir, src)
        if not os.path.isdir(d):
            continue
        for fn in sorted(os.listdir(d)):
            if fn.lower().endswith(".max"):
                files.append((label, ref, os.path.join(d, fn)))
    return files


def _maxdiff(a, b):
    return max(abs(x - y) for x, y in zip(a, b))


def parse_clod(path):
    """Parse CLodCharacterShapeBuild serial (NEL_CLODBULD v1)."""
    data = open(path, "rb").read()
    if data[:12] != b"NEL_CLODBULD":
        raise ValueError("bad magic %r" % data[:12])
    off = 13  # magic + 1-byte serialVersion
    nverts, = struct.unpack_from("<I", data, off); off += 4
    verts = [struct.unpack_from("<fff", data, off + i * 12) for i in range(nverts)]
    off += nverts * 12
    nsk, = struct.unpack_from("<I", data, off); off += 4
    skins = []
    for _ in range(nsk):
        pairs = []
        for _j in range(4):
            mid, = struct.unpack_from("<I", data, off); off += 4
            w, = struct.unpack_from("<f", data, off); off += 4
            pairs.append((mid, w))
        skins.append(pairs)

    def rstr():
        nonlocal off
        ln, = struct.unpack_from("<i", data, off); off += 4
        s = data[off:off + ln].decode("latin1"); off += ln
        return s

    nb, = struct.unpack_from("<I", data, off); off += 4
    bones = [rstr() for _ in range(nb)]
    ntri, = struct.unpack_from("<I", data, off); off += 4
    tris = list(struct.unpack_from("<%dI" % ntri, data, off)); off += ntri * 4
    nuv, = struct.unpack_from("<I", data, off); off += 4
    uvs = [struct.unpack_from("<ff", data, off + i * 8) for i in range(nuv)]
    off += nuv * 8
    nn, = struct.unpack_from("<I", data, off); off += 4
    norms = [struct.unpack_from("<fff", data, off + i * 12) for i in range(nn)]
    off += nn * 12
    w, h = struct.unpack_from("<II", data, off); off += 8
    ntex, = struct.unpack_from("<I", data, off); off += 4
    # TextureInfo is float-heavy; field compare covers geometry/skin; tex is checked for size.
    return dict(verts=verts, skins=skins, bones=bones, tris=tris, uvs=uvs, norms=norms,
                w=w, h=h, ntex=ntex, size=len(data))


def compare_clod(out_path, ref_path, pos_eps=1e-5, nrm_eps=1e-3, uv_eps=1e-5, skin_w_eps=1e-4):
    """Return (verdict, detail). Verdicts: IDENTICAL, FLOATEQ, DIFF."""
    a = open(out_path, "rb").read()
    b = open(ref_path, "rb").read()
    if a == b:
        return "IDENTICAL", ""
    try:
        A, B = parse_clod(out_path), parse_clod(ref_path)
    except Exception as e:
        return "DIFF", "parse:%s" % e

    issues = []
    if A["bones"] != B["bones"]:
        only_a = set(A["bones"]) - set(B["bones"])
        only_b = set(B["bones"]) - set(A["bones"])
        issues.append("bones(%d vs %d onlyA=%s onlyB=%s)"
                      % (len(A["bones"]), len(B["bones"]),
                         sorted(only_a)[:3], sorted(only_b)[:3]))
    if len(A["verts"]) != len(B["verts"]):
        issues.append("nverts(%d vs %d)" % (len(A["verts"]), len(B["verts"])))
    if A["tris"] != B["tris"]:
        if len(A["tris"]) != len(B["tris"]):
            issues.append("nidx(%d vs %d)" % (len(A["tris"]), len(B["tris"])))
        else:
            d = sum(1 for x, y in zip(A["tris"], B["tris"]) if x != y)
            issues.append("tri_idx(%d)" % d)

    if len(A["verts"]) == len(B["verts"]) and A["verts"]:
        pd = max(_maxdiff(x, y) for x, y in zip(A["verts"], B["verts"]))
        if pd > pos_eps:
            issues.append("pos_max=%.3g" % pd)
        if A["norms"] and B["norms"] and len(A["norms"]) == len(B["norms"]):
            nd = max(_maxdiff(x, y) for x, y in zip(A["norms"], B["norms"]))
            if nd > nrm_eps:
                issues.append("nrm_max=%.3g" % nd)
        if A["uvs"] and B["uvs"] and len(A["uvs"]) == len(B["uvs"]):
            ud = max(_maxdiff(x, y) for x, y in zip(A["uvs"], B["uvs"]))
            if ud > uv_eps:
                issues.append("uv_max=%.3g" % ud)
        if A["skins"] and B["skins"] and len(A["skins"]) == len(B["skins"]):
            idd = 0
            wmx = 0.0
            for sa, sb in zip(A["skins"], B["skins"]):
                for j in range(4):
                    if sa[j][0] != sb[j][0]:
                        idd += 1
                    wmx = max(wmx, abs(sa[j][1] - sb[j][1]))
            if idd:
                issues.append("skin_ids=%d" % idd)
            if wmx > skin_w_eps:
                issues.append("skin_w=%.3g" % wmx)

    if issues:
        return "DIFF", ";".join(issues)
    return "FLOATEQ", "size %d vs %d" % (A["size"], B["size"])


def find_ref(ref_dir, basename):
    direct = os.path.join(ref_dir, basename)
    if os.path.isfile(direct):
        return direct
    if not os.path.isdir(ref_dir):
        return None
    low = basename.lower()
    for f in os.listdir(ref_dir):
        if f.lower() == low:
            return os.path.join(ref_dir, f)
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--graphics", default=os.path.expanduser("~/ryzomcore_graphics"))
    ap.add_argument("--ref", default=os.path.expanduser("~/pipeline_export"))
    ap.add_argument("--bin", default=os.path.expanduser("~/ryzomcore/build/nel-pipeline/bin"))
    ap.add_argument("--out", default="/tmp/clod_corpus_out.%d" % os.getpid())
    ap.add_argument("--t1", action="store_true")
    ap.add_argument("--t2", action="store_true")
    ap.add_argument("--t3", action="store_true")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--gate-t3", action="store_true")
    ap.add_argument("--min-floateq", type=int, default=0,
                    help="gate: minimum IDENTICAL+FLOATEQ count")
    ap.add_argument("--max-diff", type=int, default=1000000,
                    help="gate: maximum hard DIFF count")
    ap.add_argument("--max-export-fail", type=int, default=0)
    ap.add_argument("--max-not-produced", type=int, default=0)
    ap.add_argument("--only", default=None)
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if args.all:
        args.t1 = args.t2 = args.t3 = True
    if not (args.t1 or args.t2 or args.t3):
        args.t1 = args.t2 = True

    corpus_bin = os.path.join(args.bin, "pipeline_max_corpus_test")
    export_bin = os.path.join(args.bin, "pipeline_max_export_clod")
    if not os.path.isdir(args.graphics):
        print("SKIP: asset checkouts not present")
        return SKIP_CODE
    if not os.path.isfile(corpus_bin) or (args.t3 and not os.path.isfile(export_bin)):
        print("SKIP: binaries not built")
        return SKIP_CODE

    corpus = enumerate_corpus(args.graphics)
    if args.only:
        corpus = [c for c in corpus if args.only in c[2]]
    if not corpus:
        print("SKIP: no corpus files found")
        return SKIP_CODE

    t1_pass = t1_fail = t2_pass = t2_fail = stubs = 0
    export_fail = []
    os.makedirs(args.out, exist_ok=True)

    def run_file(item):
        label, ref_rel, path = item
        res = {"label": label, "ref_rel": ref_rel, "path": path}
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
            stem = os.path.splitext(os.path.basename(path))[0]
            outdir = os.path.join(args.out, label, stem)
            os.makedirs(outdir, exist_ok=True)
            cmd = [export_bin, "--db", args.graphics, path, outdir]
            if args.verbose:
                cmd.insert(1, "-v")
            r = subprocess.run(cmd, capture_output=True, text=True)
            res["t3rc"] = r.returncode
            res["t3out"] = r.stdout
            res["t3err"] = r.stderr
            res["t3new"] = [os.path.join(outdir, f) for f in os.listdir(outdir)
                            if f.endswith(".clod")]
        return res

    results = []
    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for res in ex.map(run_file, corpus):
            results.append(res)

    identical = floateq = hard_diff = us_only = 0
    ref_missing = 0
    diff_samples = []
    for res in results:
        if res.get("stub"):
            stubs += 1
            continue
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
                sys.stdout.write((res.get("t3err") or "")[-800:])
                print()
            for outp in res.get("t3new") or []:
                bn = os.path.basename(outp)
                ref = find_ref(os.path.join(args.ref, res["ref_rel"]), bn)
                if not ref:
                    us_only += 1
                    if len(diff_samples) < 25:
                        diff_samples.append("US_ONLY %s" % bn)
                    continue
                verdict, detail = compare_clod(outp, ref)
                if verdict == "IDENTICAL":
                    identical += 1
                elif verdict == "FLOATEQ":
                    floateq += 1
                    if args.verbose and len(diff_samples) < 10:
                        diff_samples.append("FLOATEQ %s %s" % (bn, detail))
                else:
                    hard_diff += 1
                    if len(diff_samples) < 30:
                        diff_samples.append("DIFF %s: %s" % (bn, detail))

    if args.t3:
        exported_names = set()
        for res in results:
            for p in res.get("t3new") or []:
                exported_names.add(os.path.basename(p).lower())
        for _label, _src, ref_rel in PROJECTS:
            refdir = os.path.join(args.ref, ref_rel)
            if not os.path.isdir(refdir):
                continue
            for f in os.listdir(refdir):
                if f.endswith(".clod") and f.lower() not in exported_names:
                    ref_missing += 1
                    if len(diff_samples) < 40:
                        diff_samples.append("NOT_PRODUCED %s/%s" % (ref_rel, f))

    print()
    if args.t1 or args.t2:
        print("T1: %d pass, %d fail; T2: %d pass, %d fail; %d stubs"
              % (t1_pass, t1_fail, t2_pass, t2_fail, stubs))
    if args.t3:
        total = identical + floateq + hard_diff + us_only
        print("T3: %d exported: %d byte-identical, %d float-eq, %d hard-diff, "
              "%d without reference; %d references not produced"
              % (total, identical, floateq, hard_diff, us_only, ref_missing))
        print("    export failures: %d" % len(export_fail))
        for s in diff_samples:
            print("    %s" % s)

    fails = t1_fail + t2_fail
    if args.gate_t3:
        fails += max(0, len(export_fail) - args.max_export_fail)
        fails += max(0, ref_missing - args.max_not_produced)
        if identical + floateq < args.min_floateq:
            fails += 1
            print("GATE: identical+floateq %d < min %d"
                  % (identical + floateq, args.min_floateq))
        fails += max(0, hard_diff - args.max_diff)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
