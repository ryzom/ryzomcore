#!/usr/bin/env python3
"""Add missing emoji glyphs to ryzom.ttf from Noto Emoji.

Why a script and not a hand-edited binary
-----------------------------------------
Todo.txt asked "is this the current and correct ryzom.ttf?" and nobody could
answer. A committed script makes the font re-derivable, so the question has an
answer next time instead of being archaeology.

What it does
------------
Copies glyphs for the requested codepoints out of a donor font and maps them in
the target's cmap. Noto Emoji is the right donor and this is verifiable, not a
guess: the emoji already in ryzom.ttf carry Noto Emoji's own glyph names
(`u1F3DF`, `u1F680`, `houseWithGarden`) and several outlines compare
byte-identical. Both fonts are 2048 units/em, so nothing is scaled.

Glyph naming
------------
The donor's names for these glyphs collide with DejaVu names already in the
target and bound to different artwork -- Noto calls U+1F3E0 `house`, but
ryzom.ttf's `house` is DejaVu's U+2302 house symbol; likewise `uni2690`/`uni2691`
(symbol flags) and `dagger` (U+2020). So glyphs are added under `u<CPHEX>`,
which is the convention the font's other emoji already use, and the script
refuses to run if such a name is somehow taken.

Usage
-----
    ./extend_font.py --target ryzom.ttf --donor NotoEmoji-static.ttf \
                     --codepoints 1F3E0,1F3F3,1F3F4,1F5E1 --out ryzom-extended.ttf
    ./extend_font.py --target ryzom.ttf --report-only     # just list what is missing
"""

import argparse
import sys

from fontTools.ttLib import TTFont


def parse_cps(text):
    out = []
    for part in text.split(","):
        part = part.strip()
        if part:
            out.append(int(part, 16))
    return out


def add_cmap_entry(font, cp, glyph_name):
    """Map cp -> glyph_name in every cmap subtable that can hold it."""
    placed = 0
    for sub in font["cmap"].tables:
        # Codepoints above the BMP only fit UCS-4 subtables (format 12 / and
        # format 4 cannot express them at all).
        if cp > 0xFFFF and sub.format != 12:
            continue
        sub.cmap[cp] = glyph_name
        placed += 1
    return placed


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", required=True)
    ap.add_argument("--donor")
    ap.add_argument("--codepoints", default="1F3E0,1F3F3,1F3F4,1F5E1")
    ap.add_argument("--out")
    ap.add_argument("--report-only", action="store_true")
    args = ap.parse_args()

    tgt = TTFont(args.target)
    tcm = tgt.getBestCmap()
    cps = parse_cps(args.codepoints)

    missing = [c for c in cps if c not in tcm]
    present = [c for c in cps if c in tcm]
    for c in present:
        print(f"  U+{c:05X} already present as {tcm[c]!r} -- skipping")
    if not missing:
        print("nothing to do: every requested codepoint is already mapped")
        return
    print(f"missing from {args.target}: " + ", ".join(f"U+{c:05X}" for c in missing))

    if args.report_only:
        return
    if not args.donor or not args.out:
        sys.exit("ERROR: --donor and --out are required unless --report-only")

    donor = TTFont(args.donor)
    if donor["head"].unitsPerEm != tgt["head"].unitsPerEm:
        sys.exit(f"ERROR: units/em differ (donor {donor['head'].unitsPerEm} vs "
                 f"target {tgt['head'].unitsPerEm}); scaling is not implemented")
    dcm = donor.getBestCmap()
    dglyf, tglyf = donor["glyf"], tgt["glyf"]

    plan = []
    for cp in missing:
        dname = dcm.get(cp)
        if dname is None:
            sys.exit(f"ERROR: donor has no glyph for U+{cp:05X}")
        g = dglyf[dname]
        if g.isComposite():
            sys.exit(f"ERROR: U+{cp:05X} ({dname}) is composite; copying "
                     f"components is not implemented")
        newname = f"u{cp:04X}"
        if newname in tgt.getGlyphOrder():
            sys.exit(f"ERROR: target already has a glyph named {newname!r}")
        plan.append((cp, dname, newname, g))

    order = list(tgt.getGlyphOrder())
    for cp, dname, newname, g in plan:
        order.append(newname)
    tgt.setGlyphOrder(order)
    # glyf and hmtx both index by the font's glyph order, so it has to be
    # refreshed on the tables as well as the font object.
    tglyf.glyphOrder = order

    for cp, dname, newname, g in plan:
        g.expand(dglyf)               # make sure coordinates are loaded
        tglyf.glyphs[newname] = g
        tgt["hmtx"].metrics[newname] = donor["hmtx"].metrics[dname]
        n = add_cmap_entry(tgt, cp, newname)
        print(f"  + U+{cp:05X}  donor {dname!r} -> {newname!r}  "
              f"width={donor['hmtx'].metrics[dname][0]}  contours={g.numberOfContours}  "
              f"cmap subtables updated={n}")
        if n == 0:
            sys.exit(f"ERROR: no cmap subtable could hold U+{cp:05X}")

    tgt.save(args.out)
    print(f"\nwrote {args.out}")

    # --- verify by reloading, not by trusting the in-memory object ---
    check = TTFont(args.out)
    ccm = check.getBestCmap()
    gs = check.getGlyphSet()
    from fontTools.pens.recordingPen import RecordingPen
    bad = []
    for cp, dname, newname, _g in plan:
        if ccm.get(cp) != newname:
            bad.append(f"U+{cp:05X} maps to {ccm.get(cp)!r}, expected {newname!r}")
            continue
        p = RecordingPen()
        gs[newname].draw(p)
        if not p.value:
            bad.append(f"U+{cp:05X} ({newname}) draws nothing")
    print(f"verify: {len(plan) - len(bad)}/{len(plan)} new codepoint(s) OK")
    for b in bad:
        print("  !!", b)
    if bad:
        sys.exit(1)
    print(f"verify: numGlyphs {tgt['maxp'].numGlyphs} -> {check['maxp'].numGlyphs}, "
          f"mapped codepoints {len(ccm)}")


if __name__ == "__main__":
    main()
