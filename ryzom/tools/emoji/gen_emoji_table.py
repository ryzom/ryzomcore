#!/usr/bin/env python3
"""Generate the Ryzom client's emoji lookup table.

Produces `emoji.txt`:  name <TAB> codepoints <TAB> image
and a coverage report saying which Zulip emoji names the client will actually
be able to show.

Sources of truth
----------------
name -> codepoints
    Zulip's own `tools/setup/emoji/emoji_names.py` (EMOJI_NAME_MAPS), pinned to
    a release tag. We reimplement `generate_name_to_codepoint_map` from
    `emoji_setup_utils.py` exactly: canonical_name and every alias map to the
    same codepoint string. Nothing is invented here -- if a name is not in
    Zulip's table, Zulip will not send it.

    Note Zulip stores codepoints UNQUALIFIED (its `get_emoji_code` calls
    `unqualify_emoji`, dropping U+FE0F). The noto-emoji PNG filenames drop FE0F
    too, so the two conventions line up and no fixup is needed.

codepoints -> image name
    Pure derivation, google/noto-emoji convention:
        "1f604"      -> emoji_u1f604
        "0023-20e3"  -> emoji_u0023_20e3
    i.e. "emoji_u" + codepoints.replace("-", "_")

    The table stores the STEM, with no extension, deliberately. The build tools
    work with .png files, but CViewRenderer::loadTextures silently rewrites
    ".png" to ".tga" when it reads the atlas UV list, so the name the client
    must hand to setTexture is "emoji_u1f604.tga". Storing either extension in
    the table would mean one of the two sides is always wrong; storing the stem
    lets each append what it needs.

Usage
-----
    ./gen_emoji_table.py --zulip-dir <dir with emoji_names.py> \
                         --images <dir of noto png files> \
                         --out emoji.txt [--report report.txt]
"""

import argparse
import os
import re
import sys

# Zulip's codepoint strings are lowercase hex groups joined by "-".
CODEPOINT_RE = re.compile(r"^[0-9a-f]{4,6}(-[0-9a-f]{4,6})*$")
# Names we are willing to emit.
#
# Do NOT restrict this to ASCII. Zulip ships 14 names containing non-ASCII
# letters, and for 8 of them that IS the canonical name -- the one its emoji
# picker inserts. ":pinata:" is an alias, ":pi\u00f1ata:" is what actually
# arrives on the wire. An [a-z0-9_+-] class would silently drop those.
#
# So the rule is structural rather than alphabetical: a name is any run of
# bytes that cannot be confused with the surrounding text. That means no
# colon (the delimiter), no whitespace or control bytes, and a length cap so a
# stray colon in a sentence cannot start an unbounded scan. The client's
# scanner must use this same rule and then simply hash-lookup the bytes; it
# needs no Unicode tables at all.
MAX_NAME_BYTES = 64
_BAD_NAME_CHARS = re.compile(r"[\s:\x00-\x1f\x7f]")


def name_ok(name):
    b = name.encode("utf-8")
    return 0 < len(b) <= MAX_NAME_BYTES and not _BAD_NAME_CHARS.search(name)


def load_zulip_name_maps(zulip_dir):
    """exec emoji_names.py and hand back EMOJI_NAME_MAPS."""
    path = os.path.join(zulip_dir, "emoji_names.py")
    if not os.path.isfile(path):
        sys.exit(f"ERROR: {path} not found. Download it from the Zulip tag first.")
    ns = {}
    with open(path, encoding="utf-8") as fh:
        exec(compile(fh.read(), path, "exec"), ns)  # noqa: S102 - trusted, pinned input
    maps = ns.get("EMOJI_NAME_MAPS")
    if not maps:
        sys.exit(f"ERROR: EMOJI_NAME_MAPS missing or empty in {path}")
    return maps


def name_to_codepoint(emoji_name_maps):
    """Reimplementation of Zulip's generate_name_to_codepoint_map."""
    out = {}
    collisions = []
    for code, info in emoji_name_maps.items():
        for name in [info["canonical_name"], *info["aliases"]]:
            if name in out and out[name] != code:
                collisions.append((name, out[name], code))
            out[name] = code
    return out, collisions


def image_stem(code):
    return "emoji_u" + code.replace("-", "_")


def codepoints_hex(code):
    """'0023-20e3' -> '0023 20e3' (space separated, for the client)."""
    return " ".join(code.split("-"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--zulip-dir", required=True)
    ap.add_argument("--images", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--report")
    args = ap.parse_args()

    maps = load_zulip_name_maps(args.zulip_dir)
    n2c, collisions = name_to_codepoint(maps)
    have = {f for f in os.listdir(args.images) if f.endswith(".png")}

    rows, missing_img, bad_name, bad_code = [], {}, [], []
    for name in sorted(n2c):
        code = n2c[name]
        if not name_ok(name):
            bad_name.append(name)
            continue
        if not CODEPOINT_RE.match(code):
            bad_code.append((name, code))
            continue
        stem = image_stem(code)
        img = stem + ".png"
        if img not in have:
            missing_img.setdefault(code, []).append(name)
        rows.append((name, codepoints_hex(code), stem))

    with open(args.out, "w", encoding="utf-8") as fh:
        fh.write("# Ryzom chat emoji table -- GENERATED, do not edit by hand.\n")
        fh.write("# Regenerate with tools/emoji/gen_emoji_table.py\n")
        fh.write("# Put local corrections in emoji_overrides.txt instead; it is\n")
        fh.write("# loaded afterwards and wins.\n")
        fh.write("# name\\tcodepoints(hex)\\timage-stem (client appends .tga)\n")
        for name, code, img in rows:
            fh.write(f"{name}\t{code}\t{img}\n")

    # ---- report ----
    codes = set(n2c.values())
    missing_codes = set(missing_img)
    lines = []
    A = lines.append
    A("Zulip -> Ryzom emoji coverage")
    A("=" * 60)
    A(f"distinct codepoint sequences in Zulip table : {len(codes)}")
    A(f"names (canonical + aliases)                 : {len(n2c)}")
    A(f"rows written to {os.path.basename(args.out):<28}: {len(rows)}")
    A("")
    A(f"codepoints WITH an image  : {len(codes) - len(missing_codes)}")
    A(f"codepoints WITHOUT image  : {len(missing_codes)}")
    A(f"names affected by a missing image : {sum(len(v) for v in missing_img.values())}")
    A("")
    non_ascii = [n for n in n2c if not n.isascii()]
    A(f"names containing non-ASCII bytes : {len(non_ascii)} "
      f"(kept; the client scanner is byte-based)")
    if non_ascii:
        A(f"  e.g. {sorted(non_ascii)[:6]}")
    longest = max(n2c, key=lambda n: len(n.encode()))
    A(f"longest name : {len(longest.encode())} bytes ({longest!r}) "
      f"-- scanner cap is {MAX_NAME_BYTES}")
    if bad_name:
        A(f"!! names rejected ({len(bad_name)}): {bad_name[:10]}")
    if bad_code:
        A(f"!! codepoints rejected ({len(bad_code)}): {bad_code[:10]}")
    if collisions:
        A(f"note: {len(collisions)} name(s) map to more than one codepoint "
          f"(last wins, same as Zulip): {[c[0] for c in collisions[:10]]}")
    A("")
    # group the gaps so they are actionable rather than a wall of names
    flags = {c: v for c, v in missing_img.items()
             if all(0x1F1E6 <= int(p, 16) <= 0x1F1FF for p in c.split("-"))}
    other = {c: v for c, v in missing_img.items() if c not in flags}
    A(f"missing: regional-indicator flag pairs : {len(flags)}")
    A(f"missing: everything else               : {len(other)}")
    if other:
        A("")
        A("non-flag gaps (codepoint -> names):")
        for c in sorted(other):
            A(f"  {c:<24} {', '.join(other[c])}")
    # images present but unused -> atlas bloat
    used = {image_stem(c) + ".png" for c in codes}
    unused = sorted(have - used)
    A("")
    A(f"images present but NOT referenced by any Zulip name : {len(unused)}")
    skin = [u for u in unused if re.search(r"1f3f[b-f]", u)]
    A(f"  of which skin-tone variants : {len(skin)}")
    A(f"  other unused                : {len(unused) - len(skin)}")
    for u in [u for u in unused if u not in skin][:20]:
        A(f"    {u}")

    report = "\n".join(lines)
    print(report)
    if args.report:
        with open(args.report, "w", encoding="utf-8") as fh:
            fh.write(report + "\n")


if __name__ == "__main__":
    main()
