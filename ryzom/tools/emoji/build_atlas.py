#!/usr/bin/env python3
"""Stage the emoji tiles the table actually needs, then pack them into a
Ryzom interface atlas with build_interface.

Only the images referenced by emoji.txt are staged. That matters: the raw noto
download carries ~1900 skin-tone variants that no Zulip shortcode can reach, and
including them would push the atlas from 2048x1024 to 2048x2048 for nothing.

--no-border is always passed. build_interface duplicates tile borders by default
to stop bilinear filtering bleeding between neighbours, but loadTextures sets
UTexture::Nearest for interface atlases, so there is nothing to bleed and the
padding only costs atlas area (measured: it doubles it).

Usage
-----
    ./build_atlas.py --table emoji.txt --src 32 --src 32_flags \
                     --out-dir build --name texture_emojis \
                     [--build-interface /path/to/build_interface]
"""

import argparse
import os
import shutil
import struct
import subprocess
import sys


def read_table(path):
    stems = []
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            parts = line.rstrip("\n").split("\t")
            if len(parts) == 3:
                stems.append(parts[2])
    return stems


def tga_size(path):
    with open(path, "rb") as fh:
        h = fh.read(18)
    return struct.unpack_from("<H", h, 12)[0], struct.unpack_from("<H", h, 14)[0]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--table", required=True)
    ap.add_argument("--src", action="append", required=True,
                    help="directory of source PNGs; repeatable, earlier wins")
    ap.add_argument("--out-dir", required=True)
    ap.add_argument("--name", default="texture_emojis")
    ap.add_argument("--build-interface",
                    default="/home/bonny/dev/ryzom_docker/output/tools/build_interface")
    args = ap.parse_args()

    if not os.access(args.build_interface, os.X_OK):
        sys.exit(f"ERROR: build_interface not executable at {args.build_interface}")

    # index the sources, first --src wins on duplicates
    index = {}
    for d in args.src:
        if not os.path.isdir(d):
            sys.exit(f"ERROR: source dir {d} does not exist")
        for f in os.listdir(d):
            if f.endswith(".png"):
                index.setdefault(f, os.path.join(d, f))

    wanted = sorted(set(read_table(args.table)))
    stage = os.path.join(args.out_dir, "tiles")
    if os.path.isdir(stage):
        shutil.rmtree(stage)
    os.makedirs(stage)

    missing = []
    for stem in wanted:
        fn = stem + ".png"
        src = index.get(fn)
        if src is None:
            missing.append(fn)
            continue
        os.link(src, os.path.join(stage, fn)) if os.stat(src).st_dev == os.stat(stage).st_dev \
            else shutil.copy2(src, os.path.join(stage, fn))
    if missing:
        sys.exit(f"ERROR: {len(missing)} image(s) referenced by the table are absent, "
                 f"e.g. {missing[:5]}. Run fetch_noto_missing.py first.")

    staged = len(os.listdir(stage))
    print(f"staged {staged} tile(s) from {len(args.src)} source dir(s)")

    out_tga = os.path.join(args.out_dir, args.name + ".tga")
    out_txt = os.path.join(args.out_dir, args.name + ".txt")
    print(f"running build_interface --no-border -> {out_tga}")
    r = subprocess.run([args.build_interface, "--no-border", out_tga, stage],
                       capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f"build_interface failed ({r.returncode}):\n{r.stdout}\n{r.stderr}")

    if not os.path.exists(out_tga) or not os.path.exists(out_txt):
        sys.exit("ERROR: build_interface produced no atlas")

    w, h = tga_size(out_tga)
    with open(out_txt, encoding="utf-8") as fh:
        uv = [l for l in fh.read().splitlines() if l.strip()]

    # every staged tile must have landed in the UV list
    listed = {l.split()[0].lower() for l in uv}
    lost = [s + ".png" for s in wanted if (s + ".png").lower() not in listed]

    print()
    print(f"atlas      : {w}x{h}  ({os.path.getsize(out_tga)/1048576:.2f} MB RGBA .tga)")
    print(f"uv entries : {len(uv)}   staged: {staged}")
    print(f"grid slots at 32px : {(w // 32) * (h // 32)}  "
          f"({100.0 * staged / max(1, (w // 32) * (h // 32)):.1f}% used)")
    if lost:
        sys.exit(f"ERROR: {len(lost)} staged tile(s) are missing from the UV list, "
                 f"e.g. {lost[:5]}")
    print("all staged tiles are present in the UV list")
    print()
    print("NOTE: the .tga is RGBA. Whether it uploads as RGBA or DXTC5 is decided")
    print("      by which client_cfg.cpp list it goes in, not by this file.")


if __name__ == "__main__":
    main()
