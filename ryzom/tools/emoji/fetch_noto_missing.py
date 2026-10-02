#!/usr/bin/env python3
"""Fetch the emoji images noto-emoji does not ship as PNGs: the flags.

Why this is needed
------------------
noto-emoji's PNG output contains only the 26 bare regional-indicator letters --
the composed country flags are deliberately dropped from the PNG set (see
drop_flags.py upstream) because the colour font builds them from GSUB ligatures
instead. That is why all 262 flags are absent from a plain noto PNG download.

The artwork does exist in the repo, as `third_party/region-flags/waved-svg/`,
already named in the same codepoint convention the PNGs use
(`emoji_u1f1e9_1f1ea.svg`) and including the three tag-sequence subdivision
flags (England, Scotland, Wales). Those SVGs are what the colour font is built
from, so rasterising them keeps the flags stylistically identical to the rest of
the set rather than mixing in a second vendor's artwork.

Each SVG has a square `viewBox="0 0 1000 1000"` with the flag as a horizontal
band inside it, so rendering straight to 32x32 needs no aspect-ratio fixup.

Usage
-----
    ./fetch_noto_missing.py --table emoji.txt --have 32 --out 32_flags [--size 32]
"""

import argparse
import concurrent.futures
import os
import shutil
import subprocess
import sys
import urllib.request

RAW = ("https://raw.githubusercontent.com/googlefonts/noto-emoji/main/"
       "third_party/region-flags/waved-svg/{name}.svg")


def read_needed(table_path, have_dir):
    """Images the table asks for that we do not already have."""
    have = set(os.listdir(have_dir)) if os.path.isdir(have_dir) else set()
    needed = {}
    with open(table_path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            parts = line.rstrip("\n").split("\t")
            if len(parts) != 3:
                continue
            name, codes, stem = parts
            img = stem + ".png"
            if img not in have:
                needed.setdefault(img, []).append(name)
    return needed


def _get(url):
    req = urllib.request.Request(url, headers={"User-Agent": "ryzom-emoji-build"})
    with urllib.request.urlopen(req, timeout=30) as r:
        return r.read()


def _looks_like_svg(data):
    return data.lstrip().startswith(b"<svg") or b"<svg" in data[:512]


def fetch_one(img, size, out_dir):
    """Download one waved SVG and rasterise it. Returns (img, error_or_None).

    Eight of the waved SVGs are git SYMLINKS, for territories that fly another
    country's flag (Bouvet Island -> Norway, Diego Garcia -> UK, and so on).
    Over raw.githubusercontent a symlink serves its target *path* as plain text
    rather than the file, so those have to be followed by hand.
    """
    stem = img[:-4] if img.endswith(".png") else img
    svg_path = os.path.join(out_dir, stem + ".svg")
    png_path = os.path.join(out_dir, img)
    try:
        data = _get(RAW.format(name=stem))
        if not _looks_like_svg(data):
            target = data.strip().decode("utf-8", "replace")
            # A symlink body is a bare, single-line filename next to it.
            if ("\n" in target or not target.endswith(".svg")
                    or "/" in target or len(target) > 120):
                return img, f"not an SVG and not a symlink target: {target[:60]!r}"
            data = _get(RAW.format(name=target[:-4]))
            if not _looks_like_svg(data):
                return img, f"symlink target {target!r} is not an SVG either"
    except Exception as e:  # noqa: BLE001 - report, don't abort the batch
        return img, f"download failed: {e}"
    with open(svg_path, "wb") as fh:
        fh.write(data)
    try:
        subprocess.run(
            ["rsvg-convert", "-w", str(size), "-h", str(size),
             "-o", png_path, svg_path],
            check=True, capture_output=True,
        )
    except subprocess.CalledProcessError as e:
        return img, f"rsvg-convert failed: {e.stderr.decode(errors='replace')[:160]}"
    finally:
        if os.path.exists(svg_path):
            os.remove(svg_path)
    return img, None


def verify(png_path, size):
    """Confirm we produced a real, non-blank tile of the expected size."""
    try:
        from PIL import Image
    except ImportError:
        return None  # nothing to say without PIL
    try:
        im = Image.open(png_path)
    except Exception as e:  # noqa: BLE001
        return f"unreadable: {e}"
    if im.size != (size, size):
        return f"wrong size {im.size}"
    if im.mode != "RGBA":
        return f"wrong mode {im.mode}"
    if im.getbbox() is None:
        return "fully transparent"
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--table", required=True, help="emoji.txt from gen_emoji_table.py")
    ap.add_argument("--have", required=True, help="dir of images already downloaded")
    ap.add_argument("--out", required=True, help="dir to write fetched PNGs into")
    ap.add_argument("--size", type=int, default=32)
    ap.add_argument("--jobs", type=int, default=8)
    args = ap.parse_args()

    if not shutil.which("rsvg-convert"):
        sys.exit("ERROR: rsvg-convert not found (package: librsvg / librsvg2-tools)")

    needed = read_needed(args.table, args.have)
    # Anything already fetched into --out counts as had.
    os.makedirs(args.out, exist_ok=True)
    already = set(os.listdir(args.out))
    todo = sorted(i for i in needed if i not in already)
    print(f"table wants {len(needed)} image(s) not in {args.have}")
    print(f"  already in {args.out}: {len(needed) - len(todo)}")
    print(f"  to fetch            : {len(todo)}")
    if not todo:
        return

    failures = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as ex:
        futs = {ex.submit(fetch_one, i, args.size, args.out): i for i in todo}
        done = 0
        for fut in concurrent.futures.as_completed(futs):
            img, err = fut.result()
            done += 1
            if err:
                failures[img] = err
            if done % 50 == 0 or done == len(todo):
                print(f"  ...{done}/{len(todo)}")

    # verify what landed
    bad = {}
    for img in todo:
        if img in failures:
            continue
        p = os.path.join(args.out, img)
        problem = verify(p, args.size)
        if problem:
            bad[img] = problem

    ok = len(todo) - len(failures) - len(bad)
    print(f"\nfetched OK : {ok}")
    if failures:
        print(f"download/convert failures : {len(failures)}")
        for k, v in list(failures.items())[:10]:
            print(f"  {k}: {v}")
    if bad:
        print(f"produced but invalid : {len(bad)}")
        for k, v in list(bad.items())[:10]:
            print(f"  {k}: {v}")
    if failures or bad:
        sys.exit(1)


if __name__ == "__main__":
    main()
