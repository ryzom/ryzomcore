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
  light  — relight every reference .zonel with the modern zone_lighter using the
           derived-from-2001 cfg (zone_lighter/cfg/snowballs_derived.cfg) and
           tolerance-compare lumels against the reference lighting

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

def parse_zone_patches(path):
    """Minimal CZone reader: PatchBias/PatchScale + packed control points per patch.

    Handles zone versions 3..5 and patch versions 2..7 (the 2001 .zonel refs are zone v3
    / patch v4 with 5-byte era TileColors; our exports are v4+ / patch v7). CBorderVertex
    is 7 bytes (version byte + three u16). Control points are u16 words unpacked as
    bias + word * scale.
    """
    import struct
    d = open(path, "rb").read()
    o = 1  # zone version byte
    if d[o:o + 4] != b"ZONE":
        raise ValueError("no ZONE magic")
    o += 4 + 2   # magic, ZoneId
    o += 1 + 24  # CAABBoxExt version, bbox center+halfsize
    bias = struct.unpack_from("<3f", d, o); o += 12
    scale = struct.unpack_from("<f", d, o)[0]; o += 4
    o += 4       # NumVertices
    nb = struct.unpack_from("<I", d, o)[0]; o += 4
    o += nb * 7
    npatch = struct.unpack_from("<I", d, o)[0]; o += 4
    patches = []
    for _ in range(npatch):
        pver = d[o]; o += 1
        if not 2 <= pver <= 7:
            raise ValueError("patch version %d" % pver)
        verts = struct.unpack_from("<12H", d, o); o += 24
        tang = struct.unpack_from("<24H", d, o); o += 48
        inter = struct.unpack_from("<12H", d, o); o += 24
        ntiles = struct.unpack_from("<I", d, o)[0]; o += 4
        o += ntiles * 8
        ncol = struct.unpack_from("<I", d, o)[0]; o += 4
        o += ncol * (2 if pver >= 7 else 5)
        o += 2  # OrderS, OrderT
        nlum = struct.unpack_from("<I", d, o)[0]; o += 4
        o += nlum
        if pver >= 3:
            o += 2  # NoiseRotation, _CornerSmoothFlag
        if pver >= 4:
            o += 1  # Flags
        if pver >= 5:
            ntli = struct.unpack_from("<I", d, o)[0]; o += 4
            o += ntli * 3
        patches.append((verts, tang, inter))
    return {"bias": bias, "scale": scale, "patches": patches}

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
    ap.add_argument("--skip-lighting", action="store_true")
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
        # Geometry-level T3: the reference .zonel are POST-PIPELINE (zone_welder moved border
        # verts by up to ~1m, then zone_lighter lit them), so raw exports never match them.
        # Replicate the weld pass (neighbors resolved from the output dir, classic recipe:
        # copy all raw zones there first), then compare packed control points field-level.
        # Classification instead of a blanket epsilon:
        #   exact    < 2 cm   — export + weld reproduce the reference control points
        #   near     < 20 cm  — sub-tile residue (weld order / handle refresh)
        #   residual < 5 m    — real deltas (source .max edited after the 2003 data build)
        #   scale    ~ 39.37x — source authored in inch system units (different artist;
        #                       the era plugin had no unit conversion, so these sources
        #                       cannot be the ones the reference build used)
        #   wild               — anything else
        zone_welder = tool("zone_welder")
        refz = os.path.join(args.ref, "zones")
        wdir = os.path.join(args.out, "welded")
        geom = {"exact": 0, "near": 0, "residual": 0, "scale": 0, "wild": 0, "skip": 0}
        if os.path.isdir(refz) and os.path.isfile(zone_welder):
            os.makedirs(wdir, exist_ok=True)
            zones = [f for f in sorted(os.listdir(zdir)) if f.endswith(".zone")]
            for f in zones:  # raw copies so the welder finds every neighbor
                with open(os.path.join(zdir, f), "rb") as s, \
                     open(os.path.join(wdir, f), "wb") as t:
                    t.write(s.read())
            for f in zones:
                run([zone_welder, os.path.join(zdir, f), os.path.join(wdir, f)], timeout=120)
            worst = []
            for f in zones:
                stem = f[:-5]
                ref_zl = os.path.join(refz, stem + ".zonel")
                if not os.path.isfile(ref_zl):
                    geom["skip"] += 1
                    continue
                try:
                    A = parse_zone_patches(os.path.join(wdir, f))
                    B = parse_zone_patches(ref_zl)
                except Exception as e:
                    geom["wild"] += 1
                    worst.append((stem, "parse: %s" % e))
                    continue
                if len(A["patches"]) != len(B["patches"]):
                    geom["wild"] += 1
                    worst.append((stem, "npatch %d vs %d" % (len(A["patches"]), len(B["patches"]))))
                    continue
                max_v = 0.0
                for pa, pb in zip(A["patches"], B["patches"]):
                    for i in range(12):
                        d = abs((A["bias"][i % 3] + pa[0][i] * A["scale"])
                                - (B["bias"][i % 3] + pb[0][i] * B["scale"]))
                        if d > max_v:
                            max_v = d
                if max_v < 0.02:
                    geom["exact"] += 1
                elif max_v < 0.2:
                    geom["near"] += 1
                elif max_v < 5.0:
                    geom["residual"] += 1
                    worst.append((stem, "verts max %.2fm" % max_v))
                else:
                    br = max(abs(v) for v in A["bias"]) / max(1e-6, max(abs(v) for v in B["bias"]))
                    if 35.0 < br < 45.0:
                        geom["scale"] += 1
                        worst.append((stem, "inch-authored source (bias ratio %.2f)" % br))
                    else:
                        geom["wild"] += 1
                        worst.append((stem, "verts max %.1fm" % max_v))
            for stem, why in worst[:10]:
                print("  ZONE T3 %-6s %s" % (stem, why))
            if len(worst) > 10:
                print("  ZONE T3 ... %d more non-exact zones" % (len(worst) - 10))
        else:
            print("  ZONE T3 skipped (no refs or zone_welder)")
        print("ZONE T3 (welded verts vs .zonel): %d exact, %d near, %d residual, "
              "%d inch-scale, %d wild, %d skip"
              % (geom["exact"], geom["near"], geom["residual"],
                 geom["scale"], geom["wild"], geom["skip"]))
        summary["zone_ok"] = ok
        summary["zone_fail"] = fail
        summary["zone_geom"] = dict(geom)
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
                # --com-node-prefix: the 2003 refs carry "Bip01.pos"/"Bip01.rotquat" with
                # bare bone tracks; Ryzom per-node refs are bare "pos"/"rotquat".
                rc, out = run([anim_bin, "--bip", os.path.join(bipdir, bip),
                               "--com-node-prefix", gmax, outa], timeout=300)
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

    # --- lighting (.zonel: relight refs with the modern lighter, derived era cfg) ---
    # The 2001 production lighting parameters were reverse-engineered byte-level
    # (zone_lighter_2001/cfg/snowballs_recovered.cfg); this tier checks that the
    # MODERN zone_lighter with the derived cfg still reproduces the reference
    # lighting within tolerance. Residual bands are cross-codec skew (modern >>8
    # dequant vs era /7,/5), zbuffer-vs-polygonal shadow edges, x87-vs-SSE float
    # rounding and post-lighting asset drift — all value-level, so the gate is a
    # tolerance band, not byte equality.
    if not args.skip_lighting:
        lighter = tool("zone_lighter")
        refz = os.path.join(args.ref, "zones")
        bank = os.path.join(args.ref, "tiles", "bank.bank")
        shapes = os.path.join(args.ref, "shapes")
        cfg_tpl = os.path.normpath(os.path.join(
            os.path.dirname(os.path.abspath(__file__)), "..", "zone_lighter", "cfg",
            "snowballs_derived.cfg"))
        light_ok = True
        if not (os.path.isdir(refz) and os.path.isfile(bank)
                and os.path.isfile(lighter) and os.path.isfile(cfg_tpl)):
            print("LIGHT skipped (needs reference zones+bank, zone_lighter, %s)"
                  % os.path.basename(cfg_tpl))
        else:
            sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
            from snowballs_zonel_gate import era_unpack, parse_zone_lumdata

            ldir = os.path.join(args.out, "lighting")
            mirror = os.path.join(ldir, "zones_lc")
            os.makedirs(mirror, exist_ok=True)
            # lowercase mirror: the era data is mixed-case, the lighter loads
            # dependencies and igs by lowercase literal path
            for f in os.listdir(refz):
                if not (f.endswith(".zonel") or f.endswith(".ig")):
                    continue
                dst = os.path.join(mirror, f.lower())
                if not os.path.exists(dst):
                    os.symlink(os.path.join(refz, f), dst)
            cfg = os.path.join(ldir, "derived.cfg")
            with open(cfg_tpl) as f:
                tpl = f.read()
            with open(cfg, "w") as f:
                f.write(tpl.replace("@BANK@", bank).replace("@ZONES@", mirror)
                        .replace("@SHAPES@", shapes))

            zones = sorted(f[:-6].lower() for f in os.listdir(refz) if f.endswith(".zonel"))

            def ring(zone):
                m = re.match(r"^(\d+)_([a-z])([a-z])$", zone)
                if not m:
                    return []
                r, c1, c2 = int(m.group(1)), m.group(2), m.group(3)
                out = []
                for dr in (-1, 0, 1):
                    for dc in (-1, 0, 1):
                        if dr == 0 and dc == 0:
                            continue
                        nc2 = chr(ord(c2) + dc)
                        nz = "%d_%s%s" % (r + dr, c1, nc2)
                        if ("a" <= nc2 <= "z" and r + dr >= 1
                                and os.path.isfile(os.path.join(mirror, nz + ".zonel"))):
                            out.append(nz)
                return out

            def unpack_zone(path):
                out = []
                for os_, ot_, comp in parse_zone_lumdata(path):
                    vals = bytearray(os_ * 4 * ot_ * 4)
                    ns = os_ * 4
                    for tt in range(ot_):
                        for ts in range(os_):
                            ru = era_unpack(comp[(ts + tt * os_) * 8:(ts + tt * os_) * 8 + 8])
                            for v in range(4):
                                for u in range(4):
                                    vals[ts * 4 + u + (tt * 4 + v) * ns] = ru[v * 4 + u]
                    out.append(bytes(vals))
                return out

            def light_one(zone):
                dep = os.path.join(ldir, "dep_%s.cfg" % zone)
                with open(dep, "w") as f:
                    f.write("dependencies = { %s };\n"
                            % ", ".join('"%s"' % n for n in ring(zone)))
                outz = os.path.join(ldir, zone + ".zonel")
                rc, out = run([lighter, os.path.join(mirror, zone + ".zonel"), outz, cfg, dep],
                              timeout=1800)
                if rc != 0 or not os.path.isfile(outz):
                    return zone, None
                try:
                    A = unpack_zone(outz)
                    B = unpack_zone(os.path.join(refz, zone.upper() + ".zonel"))
                except Exception:
                    return zone, None
                t = e = w3 = 0
                for a, b in zip(A, B):
                    if len(a) != len(b):
                        continue
                    for x, y in zip(a, b):
                        t += 1
                        d = x - y
                        if d == 0:
                            e += 1
                        if -3 <= d <= 3:
                            w3 += 1
                return zone, (e, w3, t)

            te = t3 = tt = 0
            lfail = []
            worst = []
            # zone_lighter is multi-threaded itself; a couple in flight is plenty
            with ThreadPoolExecutor(max_workers=max(1, min(4, args.jobs))) as ex:
                for zone, res in ex.map(light_one, zones):
                    if res is None:
                        lfail.append(zone)
                        continue
                    e, w3, t = res
                    te += e; t3 += w3; tt += t
                    pz = 100.0 * w3 / t
                    if pz < 90.0:
                        worst.append((pz, zone, 100.0 * e / t))
            for pz, zone, pe in sorted(worst):
                print("  LIGHT %-6s +-3 %.2f%% exact %.2f%%" % (zone, pz, pe))
            if tt:
                print("LIGHT (.zonel relight vs 2001 refs): %d zones, exact %.2f%%, "
                      "+-3 %.2f%% (gate: >=96%%), %d failed"
                      % (len(zones) - len(lfail), 100.0 * te / tt, 100.0 * t3 / tt,
                         len(lfail)))
                summary["light_exact"] = round(100.0 * te / tt, 2)
                summary["light_w3"] = round(100.0 * t3 / tt, 2)
                light_ok = not lfail and (100.0 * t3 / tt) >= 96.0
            else:
                print("LIGHT: no zones compared")
                light_ok = False
            summary["light_fail"] = len(lfail)
            if args.gate and not light_ok:
                return 1

    print("SUMMARY", summary)
    return 0

if __name__ == "__main__":
    sys.exit(main())
