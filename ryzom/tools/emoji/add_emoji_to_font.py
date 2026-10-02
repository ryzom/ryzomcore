#!/usr/bin/env python3
"""Give ANY TrueType font the emoji coverage the Ryzom chat needs.

The point of this tool
----------------------
Ryzom's emoji "unicode" mode renders emoji as glyphs out of the shipped font. If
somebody later swaps ryzom.ttf for a different typeface, that mode goes blank
unless the new font also carries ~1400 emoji glyphs -- which essentially no
normal text font does. Rather than being stuck with one font forever, run it
through this: it copies the emoji glyphs out of a donor emoji font into whatever
font you want to use, and the emoji support comes along.

It is deliberately not specific to ryzom.ttf. Give it any target TTF.

How it works
------------
Glyphs are copied by *drawing* the donor glyph into a fresh glyph rather than
transplanting the raw glyf record. That is slightly less compact -- composite
glyphs get flattened into contours -- but it is far more robust: component
closure, recursive component renaming and point-record quirks all stop being
this tool's problem, and an optional scale/offset transform comes for free.

The decomposition has to be forced. TTGlyphPen implements addComponent, so
drawing a composite donor glyph straight into it *keeps* the components, and
they still refer to donor glyph names that do not exist in the target -- the
font then fails to compile with a KeyError on a name like `u1F46A_u1F469`.
DecomposingRecordingPen resolves components to contours first.

Things that are easy to get wrong and are handled here:

* **units/em.** A 2048-upem donor dropped into a 1000-upem font would produce
  emoji twice the intended size. The donor is scaled to the target's upem first.
* **Astral codepoints.** Everything above U+FFFF needs a format-12 cmap
  subtable; a format-4 subtable cannot express it. Many text fonts ship only
  format 4, so a format-12 subtable is synthesised when missing.
* **Glyph-name collisions.** Emoji donors reuse ordinary names -- Noto Emoji
  calls U+1F3E0 `house`, and plenty of text fonts already have a `house` bound
  to U+2302. Copied glyphs get fresh names and an existing name is never
  clobbered.
* **Invisible codepoints.** ZWJ, variation selectors and the flag tag
  characters are part of emoji sequences but must draw nothing. Mapping them to
  a visible glyph puts junk between the parts of a sequence; leaving them
  unmapped can render tofu. They are mapped to a zero-width empty glyph.

Usage
-----
    # what would change?
    ./add_emoji_to_font.py --target MyFont.ttf --codepoints-from build/emoji.txt \
        --report-only

    # do it
    ./add_emoji_to_font.py --target MyFont.ttf --donor build/NotoEmoji-donor.ttf \
        --codepoints-from build/emoji.txt --out MyFont-emoji.ttf --sheet proof.png

    # a specific few
    ./add_emoji_to_font.py --target MyFont.ttf --donor d.ttf \
        --codepoints 1F600,1F3E0 --out out.ttf
"""

import argparse
import copy
import os
import sys

from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.pens.transformPen import TransformPen
from fontTools.pens.recordingPen import DecomposingRecordingPen, RecordingPen
from fontTools.ttLib import TTFont
from fontTools.ttLib.tables._c_m_a_p import CmapSubtable

# Codepoints that are part of emoji sequences but must render as nothing.
def is_invisible(cp):
    return (cp == 0x200D                      # zero width joiner
            or cp == 0x200B                   # zero width space
            or 0xFE00 <= cp <= 0xFE0F         # variation selectors
            or 0xE0020 <= cp <= 0xE007F)      # tag characters (subdivision flags)


def codepoints_from_table(path):
    """Union of every codepoint appearing in any sequence in emoji.txt."""
    cps = set()
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or not line.strip():
                continue
            parts = line.rstrip("\n").split("\t")
            if len(parts) != 3:
                continue
            for p in parts[1].split():
                cps.add(int(p, 16))
    return sorted(cps)


def parse_cps(text):
    return sorted({int(p.strip(), 16) for p in text.split(",") if p.strip()})


def free_name(font_names, cp):
    """A glyph name for cp that the target is not already using."""
    for cand in (f"u{cp:04X}", f"emoji_u{cp:04X}", f"emoji_{cp:04X}"):
        if cand not in font_names:
            return cand
    i = 0
    while True:
        cand = f"emoji_u{cp:04X}_{i}"
        if cand not in font_names:
            return cand
        i += 1


def ensure_ucs4_cmap(font):
    """Make sure there is a cmap subtable that can hold codepoints > U+FFFF.

    Returns the list of subtables that accept astral codepoints.
    """
    wide = [st for st in font["cmap"].tables if st.format == 12]
    if wide:
        return wide
    # Seed a format 12 from the best available unicode subtable so existing
    # mappings are preserved, then register it for both the Unicode (0/4) and
    # Windows UCS-4 (3/10) platforms, which is what shapers look for.
    base = font.getBestCmap()
    made = []
    for platformID, platEncID in ((3, 10), (0, 4)):
        st = CmapSubtable.newSubtable(12)
        st.platformID, st.platEncID, st.format = platformID, platEncID, 12
        st.reserved, st.length, st.language = 0, 0, 0
        st.nGroups = 0
        st.cmap = dict(base)
        font["cmap"].tables.append(st)
        made.append(st)
    return made


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", required=True, help="TTF to add emoji to")
    ap.add_argument("--donor", help="monochrome emoji font to copy glyphs from")
    ap.add_argument("--codepoints-from", help="emoji.txt to derive codepoints from")
    ap.add_argument("--codepoints", help="explicit comma-separated hex list")
    ap.add_argument("--out", help="output TTF")
    ap.add_argument("--report-only", action="store_true")
    ap.add_argument("--replace-existing", action="store_true",
                    help="also re-map codepoints the target already covers")
    ap.add_argument("--emoji-scale", type=float, default=1.0,
                    help="scale copied glyphs (1.0 = donor size at target upem)")
    ap.add_argument("--y-offset", type=float, default=0.0,
                    help="shift copied glyphs vertically, in target units")
    ap.add_argument("--sheet", help="write a PNG contact sheet to eyeball the result")
    args = ap.parse_args()

    if not args.codepoints_from and not args.codepoints:
        sys.exit("ERROR: give --codepoints-from or --codepoints")
    want = (codepoints_from_table(args.codepoints_from) if args.codepoints_from
            else parse_cps(args.codepoints))

    tgt = TTFont(args.target)
    tcm = tgt.getBestCmap()
    upem = tgt["head"].unitsPerEm
    # getBestCmap() hands back the live subtable dict, which the mapping step
    # below mutates -- so the "before" figures have to be snapshotted as plain
    # numbers now, or the final report compares the result against itself.
    before_mapped = len(tcm)
    before_glyphs = tgt["maxp"].numGlyphs

    have = [c for c in want if c in tcm]
    need = want if args.replace_existing else [c for c in want if c not in tcm]
    vis_need = [c for c in need if not is_invisible(c)]
    inv_need = [c for c in need if is_invisible(c)]

    print(f"target      : {args.target}")
    print(f"  family    : {tgt['name'].getDebugName(1)!r}  upem={upem}  "
          f"glyphs={before_glyphs}  mapped={before_mapped}")
    print(f"requested   : {len(want)} codepoint(s)")
    print(f"  already in target : {len(have)}")
    print(f"  to add (visible)  : {len(vis_need)}")
    print(f"  to add (invisible): {len(inv_need)}")

    if args.report_only:
        return
    if not args.donor or not args.out:
        sys.exit("ERROR: --donor and --out are required unless --report-only")
    if not need:
        print("nothing to do")
        return

    donor = TTFont(args.donor)
    if "fvar" in donor:
        sys.exit("ERROR: donor is a variable font; instantiate it first "
                 "(fontTools.varLib.instancer)")
    dupem = donor["head"].unitsPerEm
    if dupem != upem:
        from fontTools.ttLib.scaleUpem import scale_upem
        print(f"donor upem {dupem} != target {upem} -- scaling donor")
        scale_upem(donor, upem)
    dcm = donor.getBestCmap()
    dgs = donor.getGlyphSet()

    names = set(tgt.getGlyphOrder())
    tglyf, thmtx = tgt["glyf"], tgt["hmtx"]
    order = list(tgt.getGlyphOrder())

    scale, dy = args.emoji_scale, args.y_offset
    transform = (scale != 1.0 or dy != 0.0)

    added, unavailable, blank = [], [], []

    # one shared zero-width empty glyph for every invisible codepoint
    blank_name = None
    if inv_need:
        blank_name = free_name(names, 0x200B) if "emoji_blank" in names else "emoji_blank"
        pen = TTGlyphPen(None)
        tglyf.glyphs[blank_name] = pen.glyph()
        thmtx.metrics[blank_name] = (0, 0)
        order.append(blank_name)
        names.add(blank_name)

    for cp in vis_need:
        dname = dcm.get(cp)
        if dname is None:
            unavailable.append(cp)
            continue
        # Decompose first: see the note in the module docstring.
        rp = DecomposingRecordingPen(dgs)
        try:
            dgs[dname].draw(rp)
        except Exception as e:  # noqa: BLE001 - a bad donor glyph must not abort the run
            unavailable.append(cp)
            print(f"  skip U+{cp:04X} ({dname}): cannot draw: {e}")
            continue
        pen = TTGlyphPen(None)
        rp.replay(TransformPen(pen, (scale, 0, 0, scale, 0, dy)) if transform else pen)
        glyph = pen.glyph()
        if not rp.value:
            blank.append(cp)          # donor glyph is itself empty; still map it
        newname = free_name(names, cp)
        tglyf.glyphs[newname] = glyph
        width = donor["hmtx"].metrics[dname][0]
        thmtx.metrics[newname] = (int(round(width * scale)), 0)
        order.append(newname)
        names.add(newname)
        added.append((cp, dname, newname))

    tgt.setGlyphOrder(order)
    tglyf.glyphOrder = order

    wide = ensure_ucs4_cmap(tgt)
    bmp = [st for st in tgt["cmap"].tables if st.format in (4, 6, 12)]
    for cp, _dn, newname in added:
        for st in (wide if cp > 0xFFFF else bmp):
            st.cmap[cp] = newname
    for cp in inv_need:
        for st in (wide if cp > 0xFFFF else bmp):
            st.cmap[cp] = blank_name

    tgt.save(args.out)
    print()
    print(f"added        : {len(added)} visible glyph(s)")
    if inv_need:
        print(f"invisible    : {len(inv_need)} codepoint(s) -> zero-width {blank_name!r}")
    if blank:
        print(f"note         : {len(blank)} donor glyph(s) drew nothing "
              f"(mapped anyway): {[f'U+{c:04X}' for c in blank[:6]]}")
    if unavailable:
        print(f"UNAVAILABLE  : {len(unavailable)} codepoint(s) the donor lacks: "
              f"{[f'U+{c:04X}' for c in unavailable[:12]]}")
    print(f"wrote        : {args.out}")

    # ---- verify against the file on disk, not the in-memory object ----
    chk = TTFont(args.out)
    ccm, cgs = chk.getBestCmap(), chk.getGlyphSet()
    bad, empty = [], 0
    for cp, _dn, newname in added:
        if ccm.get(cp) != newname:
            bad.append(f"U+{cp:04X} -> {ccm.get(cp)!r}, expected {newname!r}")
            continue
        rp = RecordingPen()
        cgs[newname].draw(rp)
        if not rp.value:
            empty += 1
    for cp in inv_need:
        if ccm.get(cp) != blank_name:
            bad.append(f"U+{cp:04X} (invisible) -> {ccm.get(cp)!r}")
    print()
    print(f"verify: mapped codepoints {before_mapped} -> {len(ccm)}, "
          f"glyphs {before_glyphs} -> {chk['maxp'].numGlyphs}")
    print(f"verify: {len(added) - len(bad)}/{len(added)} new codepoints map correctly, "
          f"{empty} draw nothing")
    for b in bad[:10]:
        print("  !!", b)

    if args.sheet:
        write_sheet(args.out, [cp for cp, _d, _n in added], args.sheet)

    if bad:
        sys.exit(1)


def write_sheet(font_path, cps, out_png, cols=24, cell=40, sample=240):
    """Render a grid of the added emoji so a human can sanity-check the result."""
    try:
        from PIL import Image, ImageDraw, ImageFont
    except ImportError:
        print("sheet: Pillow not installed, skipping")
        return
    picks = cps[:: max(1, len(cps) // sample)][:sample]
    rows = (len(picks) + cols - 1) // cols
    img = Image.new("RGB", (cols * cell, rows * cell), "white")
    d = ImageDraw.Draw(img)
    f = ImageFont.truetype(font_path, int(cell * 0.72))
    blankish = 0
    for i, cp in enumerate(picks):
        x, y = (i % cols) * cell, (i // cols) * cell
        probe = Image.new("L", (cell, cell), 0)
        ImageDraw.Draw(probe).text((cell * 0.12, cell * 0.08), chr(cp), font=f, fill=255)
        if probe.getbbox() is None:
            blankish += 1
            d.rectangle([x, y, x + cell - 1, y + cell - 1], outline="red")
        img.paste(probe.convert("RGB").point(lambda v: 255 - v), (x, y))
    img.save(out_png)
    print(f"sheet: {out_png} ({len(picks)} of {len(cps)} sampled, "
          f"{blankish} rendered blank)")


if __name__ == "__main__":
    main()
