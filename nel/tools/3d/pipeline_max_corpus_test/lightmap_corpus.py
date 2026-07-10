#!/usr/bin/env python3
"""Corpus driver for the standalone lightmapper (pipeline_max_design.md §11-lm).

Per shape-source .max (the exact ShapeSourceDirectories listing the shape process drives):
run pipeline_max_export_shape with --lm-scene into a per-file scratch dir; when the file
has lightmapped receivers (an .lmscene appears), run shape_lightmapper on it and compare:

  - lightmapped .shape vs the reference shape_not_optimized / shape_with_coarse_mesh
    (pipeline_max_export_shape --compare): identical / floateq / diff
  - generated lightmap TGAs vs the reference shape_lightmap_not_optimized, pixel-wise:
    tga-identical / tga-near (mean abs channel diff <= --tga-mean-tol and max channel
    diff <= --tga-max-tol) / tga-diff; plus missing/extra accounting per receiver
  - the per-lightmap light-list logs vs the reference .txt: layer count + ordered
    (light name, group, animation) entries per layer

Per-file scratch directories (no shared-outdir races — the veget_corpus lesson).
Defaults are the layout on Kaetemi's machine; override via CLI when running elsewhere.
"""

import argparse, os, re, shutil, struct, subprocess, sys, collections
from concurrent.futures import ThreadPoolExecutor

SKIP_CODE = 77

DEF_GRAPHICS = os.path.expanduser("~/ryzomcore_graphics")
DEF_WORKSPACE = os.path.expanduser("~/ryzomcore_leveldesign/workspace")
DEF_REF = os.path.expanduser("~/pipeline_export")
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
    rel = rel.replace("\\", "/")
    for cand in (rel.lower(), rel):
        p = os.path.join(root, cand)
        if os.path.isdir(p):
            return p
    return None


def enumerate_corpus(graphics_dir, workspace_dir):
    out = []
    seen = set()
    for root in ("common", "ecosystems", "continents", "shard"):
        base = os.path.join(workspace_dir, root)
        if not os.path.isdir(base):
            continue
        for name in sorted(os.listdir(base)):
            d = os.path.join(base, name, "directories.py")
            p2 = os.path.join(base, name, "process.py")
            if not os.path.isfile(d) or not os.path.isfile(p2):
                continue
            pns = load_dirs(p2)
            if not pns or "shape" not in (pns.get("ProcessToComplete") or []):
                continue
            ns = load_dirs(d)
            if ns is None:
                continue
            for dd in ns.get("ShapeSourceDirectories") or []:
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
                    out.append((root + "/" + name, p))
    return out


def is_ole(path):
    with open(path, "rb") as f:
        return f.read(8) == OLE_MAGIC


def project_ref_dirs(ref_dir, proj):
    grp, leaf = proj.split("/")
    return (os.path.join(ref_dir, grp, leaf, "shape_not_optimized"),
            os.path.join(ref_dir, grp, leaf, "shape_with_coarse_mesh"),
            os.path.join(ref_dir, grp, leaf, "shape_lightmap_not_optimized"))


def read_tga(path):
    d = open(path, "rb").read()
    idlen, imgtype = d[0], d[2]
    w, h = struct.unpack_from("<HH", d, 12)
    bpp = d[16]
    if imgtype != 2:
        return None
    off = 18 + idlen
    return (w, h, bpp, d[off:off + w * h * (bpp // 8)])


def cmp_tga(ours, ref, mean_tol, max_tol):
    a = read_tga(ours)
    b = read_tga(ref)
    if a is None or b is None or a[0] != b[0] or a[1] != b[1] or a[2] != b[2]:
        return ("tga-diff", "layout %s vs %s" % (a[:3] if a else None, b[:3] if b else None))
    pa, pb = a[3], b[3]
    n = min(len(pa), len(pb))
    if pa == pb:
        return ("tga-identical", "")
    total = n
    sumd = 0
    maxd = 0
    for i in range(n):
        dd = pa[i] - pb[i]
        if dd < 0:
            dd = -dd
        sumd += dd
        if dd > maxd:
            maxd = dd
    mean = sumd / max(1, total)
    if mean <= mean_tol and maxd <= max_tol:
        return ("tga-near", "mean %.3f max %d" % (mean, maxd))
    # Packing-invariant tier: the same lightmap content packed at different atlas positions
    # (the predicted §11-lm class — plane placement ties break differently under 1-ULP input
    # noise). Compare the sorted channel-value distributions (quantile distance).
    qa, qb = sorted(pa[:n]), sorted(pb[:n])
    q = sum(abs(qa[i] - qb[i]) for i in range(n)) / max(1, n)
    if q <= mean_tol:
        return ("tga-repacked", "pixel mean %.3f, quantile %.4f" % (mean, q))
    return ("tga-diff", "mean %.3f max %d quantile %.3f" % (mean, maxd, q))



def _only_lightmap_uv_diff(details):
    """DIFF details where every reported difference is either float-noise-scale or confined
    to VB value 3 (the lightmap UV set) / the VB size-with-equal-vert-count class."""
    import re as _re
    saw_uv1 = False
    for d in details:
        if d.startswith("class:") or d.startswith("VB:") or d.startswith("matrix blocks"):
            continue
        m = _re.match(r"VB value (\d+):", d)
        if m:
            if m.group(1) == "3":
                saw_uv1 = True
                continue
            # other VB values must be float noise (maxAbs <= 2e-6 tier reported by compare)
            m2 = _re.search(r"maxAbs ([0-9.e+-]+)", d)
            if m2 and float(m2.group(1)) <= 1e-4:
                continue
            return False
        # any other difference line disqualifies
        return False
    return saw_uv1


LOG_ENTRY = re.compile(r'^\t(.+) \(group (\d+), animation "(.*)"\)')


def parse_lm_log(path):
    """[(layer file basename, [(name, group, anim)])] in file order."""
    layers = []
    cur = None
    for line in open(path, encoding="utf-8", errors="replace"):
        m = LOG_ENTRY.match(line.rstrip("\n"))
        if m:
            if cur is not None:
                cur[1].append((m.group(1), int(m.group(2)), m.group(3)))
            continue
        s = line.strip()
        if s.endswith(".tga :"):
            cur = (os.path.basename(s[:-2].strip()).replace("\\", "/").split("/")[-1], [])
            layers.append(cur)
    return layers


def run_file(args, proj, path, refdirs):
    """Returns a stats Counter + report lines for one .max file."""
    st = collections.Counter()
    lines = []
    stem = os.path.splitext(os.path.basename(path))[0]
    scratch = os.path.join(args.scratch, proj.replace("/", "_"), stem)
    shutil.rmtree(scratch, ignore_errors=True)
    scenes = os.path.join(scratch, "scenes")
    shapes = os.path.join(scratch, "shapes")
    out = os.path.join(scratch, "out")
    lmdir = os.path.join(scratch, "lightmaps")
    for d in (scenes, shapes, out, lmdir):
        os.makedirs(d, exist_ok=True)

    exporter = os.path.join(args.bin, "pipeline_max_export_shape")
    lmtool = os.path.join(args.bin, "shape_lightmapper")

    r = subprocess.run([exporter, "--lm-scene", scenes, path, shapes, "--coarse-out", shapes],
                       capture_output=True, text=True, timeout=1800)
    scene = os.path.join(scenes, stem.lower() + ".lmscene")
    if not os.path.isfile(scene):
        st["no-receivers"] += 1
        if not args.keep:
            shutil.rmtree(scratch, ignore_errors=True)
        return st, lines
    st["scenes"] += 1

    # texture search paths for occluder transparency sampling
    texargs = []
    comp = os.path.relpath(path, args.graphics).split(os.sep)
    for cand in (os.path.join(args.graphics, comp[0], comp[1], "decors", "_textures") if len(comp) > 2 else None,
                 os.path.join(args.graphics, "stuff", "generique", "decors", "_textures"),
                 os.path.dirname(path)):
        if cand and os.path.isdir(cand):
            texargs += ["--texture-path", cand]

    r = subprocess.run([lmtool, "--lightmap-log", "--lightmaps", lmdir] + texargs + [scene, out],
                       capture_output=True, text=True, timeout=3600)
    if r.returncode != 0:
        st["lightmapper-fail"] += 1
        lines.append("LM FAIL %s (%s): %s" % (stem, proj, (r.stderr or "").strip().splitlines()[-1:]))
        if not args.keep:
            shutil.rmtree(scratch, ignore_errors=True)
        return st, lines

    refdir, coarsedir, lmrefdir = refdirs

    # Shapes
    for f in sorted(os.listdir(out)):
        if not f.endswith(".shape"):
            continue
        st["receivers"] += 1
        ref = os.path.join(refdir, f)
        if not os.path.isfile(ref):
            ref = os.path.join(coarsedir, f)
        if not os.path.isfile(ref):
            st["shape-no-ref"] += 1
            continue
        ours = os.path.join(out, f)
        if open(ours, "rb").read() == open(ref, "rb").read():
            st["shape-identical"] += 1
            continue
        rc = subprocess.run([exporter, "--compare", ours, ref], capture_output=True, text=True, timeout=300)
        verdict = "DIFF"
        details = []
        for l in (rc.stdout or "").splitlines():
            if l.startswith("VERDICT"):
                verdict = l.split()[-1]
            elif l.startswith("  "):
                details.append(l.strip())
        if verdict == "FLOATEQ":
            st["shape-floateq"] += 1
        elif verdict == "DIFF" and _only_lightmap_uv_diff(details):
            # UV1 (the lightmap atlas channel) is the only non-noise difference — the
            # repacked-atlas class, shape-side.
            st["shape-repacked"] += 1
        else:
            st["shape-diff"] += 1
            lines.append("SHAPE DIFF %s (%s): %s" % (f, proj, "; ".join(details[:4])))

    # TGAs (ours vs reference) + reference TGAs we did not produce
    ours_tgas = set(f for f in os.listdir(lmdir) if f.endswith(".tga"))
    for f in sorted(ours_tgas):
        ref = os.path.join(lmrefdir, f)
        if not os.path.isfile(ref):
            st["tga-no-ref"] += 1
            lines.append("TGA NO-REF %s (%s)" % (f, proj))
            continue
        verdict, detail = cmp_tga(os.path.join(lmdir, f), ref, args.tga_mean_tol, args.tga_max_tol)
        st[verdict] += 1
        if verdict == "tga-diff":
            lines.append("TGA DIFF %s (%s): %s" % (f, proj, detail))
    # expected reference TGAs for this scene's receivers: <stem[0]>_<node>_<n>.tga
    if os.path.isdir(lmrefdir):
        prefix = stem[0].lower() + "_"
        our_nodes = set(os.path.splitext(f)[0] for f in os.listdir(out) if f.endswith(".shape"))
        for f in os.listdir(lmrefdir):
            if not f.endswith(".tga") or not f.startswith(prefix):
                continue
            m = re.match(r"^%s(.+)_(\d+)\.tga$" % re.escape(prefix), f)
            if not m or m.group(1) not in our_nodes:
                continue
            if f not in ours_tgas:
                st["tga-missing"] += 1
                lines.append("TGA MISSING %s (%s)" % (f, proj))

    # Logs
    for f in sorted(os.listdir(lmdir)):
        if not f.endswith(".txt"):
            continue
        ref = os.path.join(lmrefdir, f)
        if not os.path.isfile(ref):
            st["log-no-ref"] += 1
            continue
        a = parse_lm_log(os.path.join(lmdir, f))
        b = parse_lm_log(ref)
        if a == b:
            st["log-match"] += 1
        else:
            st["log-diff"] += 1
            lines.append("LOG DIFF %s (%s): %d vs %d layers" % (f, proj, len(a), len(b)))

    if not args.keep:
        shutil.rmtree(scratch, ignore_errors=True)
    return st, lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=DEF_GRAPHICS)
    ap.add_argument("--workspace", default=DEF_WORKSPACE)
    ap.add_argument("--ref", default=DEF_REF)
    ap.add_argument("--bin", default=DEF_BIN)
    ap.add_argument("--scratch", default="/tmp/lightmap_corpus")
    ap.add_argument("--project", default=None, help="restrict to one project (e.g. continents/matis)")
    ap.add_argument("--only", default=None, help="restrict to .max basenames containing this substring")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    ap.add_argument("--keep", action="store_true", help="keep per-file scratch outputs")
    ap.add_argument("--tga-mean-tol", type=float, default=1.0)
    ap.add_argument("--tga-max-tol", type=int, default=32)
    ap.add_argument("--gate", action="store_true", help="exit nonzero past the gate budgets")
    ap.add_argument("--max-shape-diff", type=int, default=0)
    ap.add_argument("--max-tga-diff", type=int, default=0)
    ap.add_argument("--max-log-diff", type=int, default=0)
    ap.add_argument("--max-lm-fail", type=int, default=0)
    args = ap.parse_args()

    if not os.path.isdir(args.graphics) or not os.path.isdir(args.workspace):
        print("SKIP: private asset checkouts not present")
        sys.exit(SKIP_CODE)

    corpus = enumerate_corpus(args.graphics, args.workspace)
    if args.project:
        corpus = [(p, f) for (p, f) in corpus if p == args.project]
    if args.only:
        corpus = [(p, f) for (p, f) in corpus if args.only in os.path.basename(f)]
    corpus = [(p, f) for (p, f) in corpus if is_ole(f)]
    if args.limit:
        corpus = corpus[:args.limit]
    print("corpus: %d files" % len(corpus))

    total = collections.Counter()
    report = []

    def work(item):
        proj, path = item
        try:
            return run_file(args, proj, path, project_ref_dirs(args.ref, proj))
        except Exception as e:
            st = collections.Counter()
            st["harness-error"] += 1
            return st, ["HARNESS ERROR %s: %s" % (path, e)]

    with ThreadPoolExecutor(max_workers=args.jobs) as ex:
        for st, lines in ex.map(work, corpus):
            total.update(st)
            report.extend(lines)

    for l in report:
        print(l)
    print("== lightmap corpus ==")
    for k in sorted(total):
        print("%-18s %d" % (k, total[k]))

    if args.gate:
        bad = (total["shape-diff"] > args.max_shape_diff
               or total["tga-diff"] > args.max_tga_diff
               or total["log-diff"] > args.max_log_diff
               or total["lightmapper-fail"] > args.max_lm_fail
               or total["harness-error"] > 0)
        sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
