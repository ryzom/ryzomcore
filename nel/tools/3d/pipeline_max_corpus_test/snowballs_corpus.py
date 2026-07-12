#!/usr/bin/env python3
"""Snowballs rebuild corpus driver.

Sources:  ~/snowballs_source
Refs:     ~/snowballs_reference/data

Gates:
  T1/T2  — parse→build roundtrip on every .max (Max 3 Scene 0x2004)
  zone   — export all max/zones/*.max → .zone (refs are .zonel post-lighter)
  shape  — export objects/sky/characters; name-map case-insensitive / strip _N
  skel   — gnu-assigned.max → gnu.skel
  anim   — figure.max + bip/*.bip via --bip (gnu ear flip expected residual)
  swt    — informational (snowballs sources often lack SWT appdata flags)

Self-skips (exit 77) when source/ref trees are absent.
"""
from __future__ import print_function

import argparse
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

HOME = os.path.expanduser("~")
SRC = os.path.join(HOME, "snowballs_source")
REF = os.path.join(HOME, "snowballs_reference", "data")

def find_bin(name, explicit=None):
    if explicit and os.path.isfile(explicit):
        return explicit
    for root in (
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "..", "build", "nel-pipeline", "bin"),
        os.path.join(HOME, "ryzomcore", "build", "nel-pipeline", "bin"),
        os.path.join(HOME, "build_vs2008_wine_pipeline", "winebin"),
    ):
        p = os.path.normpath(os.path.join(root, name))
        if os.path.isfile(p):
            return p
    return name

def run(cmd, timeout=120):
    try:
        r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           timeout=timeout, universal_newlines=True)
        return r.returncode, r.stdout or ""
    except subprocess.TimeoutExpired:
        return 99, "TIMEOUT"
    except OSError as e:
        return 98, str(e)

def list_max(root):
    out = []
    for dp, _, files in os.walk(root):
        for f in files:
            if f.lower().endswith(".max"):
                out.append(os.path.join(dp, f))
    return sorted(out)

def shape_name_key(name):
    """Normalize shape basenames for ref matching: lower, strip trailing _digits."""
    b = os.path.splitext(os.path.basename(name))[0].lower()
    b = re.sub(r"_\d+$", "", b)
    return b

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--bin-dir", default=None, help="directory with pipeline_max_* tools")
    ap.add_argument("--src", default=SRC)
    ap.add_argument("--ref", default=REF)
    ap.add_argument("--out", default="/tmp/snowballs_corpus_out")
    ap.add_argument("--jobs", "-j", type=int, default=max(1, (os.cpu_count() or 2) - 1))
    ap.add_argument("--skip-t12", action="store_true")
    ap.add_argument("--skip-zone", action="store_true")
    ap.add_argument("--skip-shape", action="store_true")
    ap.add_argument("--skip-skel", action="store_true")
    ap.add_argument("--skip-anim", action="store_true")
    ap.add_argument("--gate", action="store_true", help="exit non-zero on T1/T2 fail or zone export fail")
    args = ap.parse_args()

    if not os.path.isdir(args.src) or not os.path.isdir(args.ref):
        print("SKIP: snowballs_source or snowballs_reference missing", file=sys.stderr)
        return 77

    def tool(n):
        if args.bin_dir:
            p = os.path.join(args.bin_dir, n)
            if os.path.isfile(p):
                return p
        return find_bin(n)

    corpus = tool("pipeline_max_corpus_test")
    zone_bin = tool("pipeline_max_export_zone")
    shape_bin = tool("pipeline_max_export_shape")
    skel_bin = tool("pipeline_max_export_skel")
    anim_bin = tool("pipeline_max_export_anim")

    os.makedirs(args.out, exist_ok=True)
    summary = {}

    # --- T1/T2 ---
    if not args.skip_t12:
        maxes = list_max(args.src)
        ok = fail = 0
        fails = []
        def one_t12(path):
            rc, out = run([corpus, "--parse", path])
            return path, rc, out
        with ThreadPoolExecutor(max_workers=args.jobs) as ex:
            futs = [ex.submit(one_t12, p) for p in maxes]
            for fut in as_completed(futs):
                path, rc, out = fut.result()
                if rc == 0 and "OK" in out:
                    ok += 1
                else:
                    fail += 1
                    fails.append(path)
        print("T1/T2: %d ok, %d fail / %d" % (ok, fail, len(maxes)))
        for p in fails[:20]:
            print("  FAIL", p)
        summary["t12_ok"] = ok
        summary["t12_fail"] = fail
        if args.gate and fail:
            return 1

    # --- zones ---
    if not args.skip_zone:
        zdir = os.path.join(args.out, "zones")
        os.makedirs(zdir, exist_ok=True)
        zsrc = os.path.join(args.src, "max", "zones")
        ok = fail = 0
        for f in sorted(os.listdir(zsrc)) if os.path.isdir(zsrc) else []:
            if not f.lower().endswith(".max"):
                continue
            stem = os.path.splitext(f)[0]
            outz = os.path.join(zdir, stem + ".zone")
            rc, out = run([zone_bin, "--zone", outz, os.path.join(zsrc, f)])
            if rc == 0 and os.path.isfile(outz) and os.path.getsize(outz) > 32:
                ok += 1
            else:
                fail += 1
                print("  ZONE FAIL", f, out[-200:] if out else "")
        print("ZONE export: %d ok, %d fail" % (ok, fail))
        # refs are .zonel (post lighter) — report size pairs only
        refz = os.path.join(args.ref, "zones")
        nref = 0
        if os.path.isdir(refz):
            for f in os.listdir(refz):
                if f.endswith(".zonel"):
                    nref += 1
        print("ZONE refs (.zonel): %d (T3 needs zone_lighter — sizes not comparable raw)" % nref)
        summary["zone_ok"] = ok
        summary["zone_fail"] = fail
        if args.gate and fail:
            return 1

    # --- shapes ---
    if not args.skip_shape:
        sdir = os.path.join(args.out, "shapes")
        os.makedirs(sdir, exist_ok=True)
        sources = []
        for sub in ("objects", "sky", "characters"):
            d = os.path.join(args.src, "max", sub)
            if os.path.isdir(d):
                sources.extend(list_max(d))
        produced = {}
        for p in sources:
            rc, out = run([shape_bin, p, sdir], timeout=180)
            for line in out.splitlines():
                line = line.strip()
                if line.startswith("OK "):
                    sp = line[3:].strip()
                    if os.path.isfile(sp):
                        produced[shape_name_key(sp)] = sp
            # Also pick up anything written to sdir (stdout parsing can miss Wine wrappers).
            if os.path.isdir(sdir):
                for f in os.listdir(sdir):
                    if f.lower().endswith(".shape"):
                        produced[shape_name_key(f)] = os.path.join(sdir, f)
        ref_shapes = {}
        rdir = os.path.join(args.ref, "shapes")
        if os.path.isdir(rdir):
            for f in os.listdir(rdir):
                if f.lower().endswith(".shape"):
                    ref_shapes[shape_name_key(f)] = os.path.join(rdir, f)
        matched = size_near = missing = 0
        for k, rp in sorted(ref_shapes.items()):
            if k not in produced:
                # ps files live under shapes/ in the ref pack — skip non-mesh
                if k.endswith((".ps",)):
                    continue
                # nel_logo may be absent from sources
                missing += 1
                print("  SHAPE no-export for ref", os.path.basename(rp))
                continue
            matched += 1
            a, b = os.path.getsize(produced[k]), os.path.getsize(rp)
            ratio = abs(a - b) / float(max(b, 1))
            if ratio < 0.15:
                size_near += 1
            print("  SHAPE %-28s ours=%6d ref=%6d ratio=%.3f" % (k, a, b, ratio))
        print("SHAPE: produced %d, ref-matched %d (size-near %d), ref-missing-export %d"
              % (len(produced), matched, size_near, missing))
        summary["shape_produced"] = len(produced)
        summary["shape_matched"] = matched

    # --- skel ---
    if not args.skip_skel:
        gmax = os.path.join(args.src, "max", "characters", "gnu", "gnu-assigned.max")
        outsk = os.path.join(args.out, "gnu.skel")
        refsk = os.path.join(args.ref, "anims", "gnu.skel")
        if os.path.isfile(gmax):
            rc, out = run([skel_bin, gmax, outsk], timeout=180)
            if os.path.isfile(outsk) and os.path.isfile(refsk):
                print("SKEL gnu: ours=%d ref=%d rc=%d"
                      % (os.path.getsize(outsk), os.path.getsize(refsk), rc))
            else:
                print("SKEL gnu: rc=%d out=%s" % (rc, os.path.isfile(outsk)))
        summary["skel"] = os.path.isfile(outsk) if os.path.isfile(gmax) else False

    # --- anim via --bip ---
    if not args.skip_anim:
        adir = os.path.join(args.out, "anims")
        os.makedirs(adir, exist_ok=True)
        gmax = os.path.join(args.src, "max", "characters", "gnu", "gnu-assigned.max")
        bipdir = os.path.join(args.src, "max", "characters", "gnu", "bip")
        refa = os.path.join(args.ref, "anims")
        # Map bip stem → ref anim name (avance → marche historically)
        bip_map = {
            "avance": "marche",
            "idle": "idle",
            "impact": "impact",
            "lancelaboule": "lancelaboule",
            "log_off": "log_off",
            "log_on": "log_on",
            "patterfeet": "patterfeet",
            "prepaboule": "prepaboule",
            "prepaboulecycle": "prepaboulecycle",
        }
        anim_ok = anim_fail = 0
        if os.path.isdir(bipdir) and os.path.isfile(gmax):
            for bip in sorted(os.listdir(bipdir)):
                if not bip.endswith(".bip"):
                    continue
                stem = os.path.splitext(bip)[0]
                ref_name = bip_map.get(stem, stem) + ".anim"
                outa = os.path.join(adir, ref_name)
                rc, out = run([anim_bin, "--bip", os.path.join(bipdir, bip), gmax, outa],
                              timeout=300)
                if rc == 0 and os.path.isfile(outa) and os.path.getsize(outa) > 64:
                    anim_ok += 1
                    rs = os.path.join(refa, ref_name)
                    if os.path.isfile(rs):
                        print("  ANIM %-20s ours=%6d ref=%6d"
                              % (ref_name, os.path.getsize(outa), os.path.getsize(rs)))
                    else:
                        print("  ANIM %-20s ours=%6d (no ref)" % (ref_name, os.path.getsize(outa)))
                else:
                    anim_fail += 1
                    print("  ANIM FAIL", bip, "rc", rc, (out or "")[-300:])
        print("ANIM (--bip): %d ok, %d fail (gnu ear flip residual expected)" % (anim_ok, anim_fail))
        summary["anim_ok"] = anim_ok
        summary["anim_fail"] = anim_fail

    print("SUMMARY", summary)
    return 0

if __name__ == "__main__":
    sys.exit(main())
