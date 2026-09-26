#!/usr/bin/env python3
"""Generate the layout of the client's emoji picker.

Produces `emoji_picker.txt`: the groups the picker shows as tabs, the emoji in
each of them, and the description shown under the hovered one.

Sources of truth
----------------
order, groups and descriptions
    Unicode's own `emoji-test.txt` for UTS #51. It is the file every emoji
    keyboard is built from: it lists every emoji in the official order, cut
    into the nine groups ("Smileys & Emotion", "People & Body", ... "Flags")
    and carries the CLDR short name of each one ("rolling on the floor
    laughing"). Pinned to a version, like the Zulip table is.

    Do NOT use the old `emojiPicker.php` list from the web chat for this. It
    predates the group scheme, stops somewhere around Emoji 4.0 and has no
    flags at all.

name
    `emoji.txt`, i.e. what the chat itself understands. An emoji the client
    cannot name is one the player could not type either, so the picker has no
    business showing it: matching the two files is what keeps the picker and
    the chat from disagreeing.

    Which of its names to show comes from Zulip's `emoji_names.py`, the same
    file the table is built from: the picker inserts the canonical name, so
    what a player picks here is spelled the way the web chat spells it. The
    aliases are all equally typeable, but they read badly -- ":oops:" for the
    upside-down face, ":dissolve:" for the melting one. Without the Zulip file
    the first name alphabetically is used, which is usually but not always the
    canonical one.

Matching
--------
`emoji-test.txt` lists fully-qualified sequences, so they carry U+FE0F; the
Zulip table stores everything unqualified (see gen_emoji_table.py). Dropping
FE0F from the test file's sequences puts the two in the same shape, and then
they compare as plain strings.

Skin-tone and other variants fall out by themselves: nothing in the table has
a name for them, so they never match and never reach the picker.

Usage
-----
    curl -sSO https://unicode.org/Public/emoji/16.0/emoji-test.txt
    ./gen_emoji_picker.py --emoji-test emoji-test.txt \\
                          --table ../../client/data/gamedev/interfaces_v3/emoji.txt \\
                          --zulip-dir . \\
                          --out ../../client/data/gamedev/interfaces_v3/emoji_picker.txt \\
                          [--report picker_coverage.txt]

`--zulip-dir` is the same directory gen_emoji_table.py uses; both want
`emoji_names.py` from the Zulip tag the server runs.
"""

import argparse
import os
import re
import sys
from collections import OrderedDict

# "1F600 ; fully-qualified # 😀 E1.0 grinning face"
TEST_LINE_RE = re.compile(
    r"^(?P<codes>[0-9A-Fa-f ]+);\s*(?P<status>[a-z-]+)\s*#\s*(?P<glyph>\S+)\s+"
    r"E(?P<age>[0-9.]+)\s+(?P<desc>.+?)\s*$"
)

# The tenth group. It holds skin tones and hair styles -- modifiers, not emoji
# anyone picks on their own -- and every keyboard leaves it out.
SKIP_GROUPS = {"Component"}

# Group label -> i18n key. The client prefers the key when the translation is
# loaded and falls back to the English label from this file, so a missing
# translation costs a nicer word, not a working picker.
GROUP_KEYS = {
    "Smileys & Emotion": "uiEmojiGroupSmileys",
    "People & Body": "uiEmojiGroupPeople",
    "Animals & Nature": "uiEmojiGroupNature",
    "Food & Drink": "uiEmojiGroupFood",
    "Travel & Places": "uiEmojiGroupTravel",
    "Activities": "uiEmojiGroupActivities",
    "Objects": "uiEmojiGroupObjects",
    "Symbols": "uiEmojiGroupSymbols",
    "Flags": "uiEmojiGroupFlags",
}


def load_canonical_names(zulip_dir):
    """exec emoji_names.py -> {codepoints: canonical_name}. {} if not given."""
    if not zulip_dir:
        return {}
    path = os.path.join(zulip_dir, "emoji_names.py")
    if not os.path.isfile(path):
        sys.exit(f"ERROR: {path} not found. Download it from the Zulip tag first.")
    ns = {}
    with open(path, encoding="utf-8") as fh:
        exec(compile(fh.read(), path, "exec"), ns)  # noqa: S102 - trusted, pinned input
    maps = ns.get("EMOJI_NAME_MAPS")
    if not maps:
        sys.exit(f"ERROR: EMOJI_NAME_MAPS missing or empty in {path}")
    return {code: info["canonical_name"] for code, info in maps.items()}


def unqualify(codes):
    """['1F636', 'FE0F'] -> '1f636', the shape the Zulip table stores."""
    return "-".join(c.lower() for c in codes if c.upper() != "FE0F")


def load_table(path):
    """emoji.txt -> ({codepoints: first name alphabetically}, {name: codepoints})."""
    by_code = {}
    by_name = {}
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) < 3:
                continue
            name, codes, stem = parts[0], parts[1], parts[2]
            if not stem:
                continue
            key = "-".join(codes.split())
            # The file is sorted by name, so the first one seen is the first
            # alphabetically -- the same one the client's reverse index keeps.
            by_code.setdefault(key, name)
            by_name[name] = key
    if not by_code:
        sys.exit(f"ERROR: no usable rows in {path}")
    return by_code, by_name


def load_emoji_test(path):
    """emoji-test.txt -> OrderedDict {group: [(codepoints, description)]}."""
    groups = OrderedDict()
    group = None
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.rstrip("\n")
            if line.startswith("# group:"):
                group = line.split(":", 1)[1].strip()
                if group not in SKIP_GROUPS:
                    groups.setdefault(group, [])
                continue
            if not line or line.startswith("#"):
                continue
            m = TEST_LINE_RE.match(line)
            if not m or m.group("status") != "fully-qualified":
                continue
            if group is None or group in SKIP_GROUPS:
                continue
            groups[group].append((unqualify(m.group("codes").split()),
                                  m.group("desc")))
    if not groups:
        sys.exit(f"ERROR: no groups found in {path} -- is it emoji-test.txt?")
    return groups


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--emoji-test", required=True,
                    help="Unicode's emoji-test.txt for the pinned version")
    ap.add_argument("--table", required=True, help="the client's emoji.txt")
    ap.add_argument("--zulip-dir",
                    help="dir holding Zulip's emoji_names.py, for the canonical "
                         "names; without it the first name alphabetically is used")
    ap.add_argument("--out", required=True)
    ap.add_argument("--report")
    args = ap.parse_args()

    by_code, by_name = load_table(args.table)
    canonical = load_canonical_names(args.zulip_dir)
    groups = load_emoji_test(args.emoji_test)

    rows = OrderedDict()
    used = set()
    missing = []
    uncanonical = []
    for group, entries in groups.items():
        kept = []
        for code, desc in entries:
            name = by_code.get(code)
            if name is None:
                missing.append((group, code, desc))
                continue
            # Prefer Zulip's canonical name, but only if the table really has
            # it under that spelling -- the client looks names up, it does not
            # guess.
            canon = canonical.get(code)
            if canon and by_name.get(canon) == code:
                name = canon
            elif canon:
                uncanonical.append((canon, name))
            if name in used:
                # A name can only sit in one tab, and the official order puts
                # the better home first.
                continue
            used.add(name)
            kept.append((name, desc))
        if kept:
            rows[group] = kept

    with open(args.out, "w", encoding="utf-8") as fh:
        fh.write("# Ryzom chat emoji picker layout -- GENERATED, do not edit by hand.\n")
        fh.write("# Regenerate with tools/emoji/gen_emoji_picker.py\n")
        fh.write("# Groups, order and descriptions come from Unicode's emoji-test.txt;\n")
        fh.write("# the names are the ones emoji.txt gives the chat.\n")
        fh.write("# g\\ti18n-key\\tEnglish label   starts a group\n")
        fh.write("# e\\tname\\tdescription      one emoji, in the group above\n")
        for group, kept in rows.items():
            key = GROUP_KEYS.get(group, "")
            fh.write(f"g\t{key}\t{group}\n")
            for name, desc in kept:
                fh.write(f"e\t{name}\t{desc}\n")

    total = sum(len(v) for v in rows.values())
    print(f"{total} emoji in {len(rows)} groups -> {args.out}", file=sys.stderr)
    print(f"{len(missing)} listed by Unicode but not in {args.table} "
          f"(no name or no tile)", file=sys.stderr)
    print(f"{len(by_code) - len(used)} in the table but not reachable from the "
          f"picker", file=sys.stderr)
    if not canonical:
        print("no --zulip-dir given: names are the first alphabetically, not "
              "necessarily the canonical ones", file=sys.stderr)
    elif uncanonical:
        print(f"{len(uncanonical)} canonical name(s) missing from the table, "
              f"fell back to an alias", file=sys.stderr)

    if args.report:
        with open(args.report, "w", encoding="utf-8") as fh:
            fh.write("Emoji picker coverage\n=====================\n\n")
            for group, kept in rows.items():
                fh.write(f"{group}: {len(kept)}\n")
            fh.write(f"\ntotal shown: {total}\n")
            fh.write(f"\nIn emoji-test.txt but not in the client table ({len(missing)}):\n")
            for group, code, desc in missing:
                fh.write(f"  {code}\t{desc}\t[{group}]\n")
            unreachable = sorted(set(by_code.values()) - used)
            fh.write(f"\nIn the client table but in no group ({len(unreachable)}):\n")
            for name in unreachable:
                fh.write(f"  {name}\n")


if __name__ == "__main__":
    main()
