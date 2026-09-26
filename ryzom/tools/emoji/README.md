# Chat emoji tooling

Generates and verifies the data behind emoji in the game chat. Everything here
is re-runnable: the emoji table, the atlas and the font coverage are all derived
rather than hand-maintained, so "is this file current?" has an answer.

| script | what it does |
|---|---|
| `gen_emoji_table.py` | Zulip's `emoji_names.py` -> `emoji.txt` + a coverage report |
| `gen_emoji_picker.py` | Unicode's `emoji-test.txt` + `emoji.txt` -> `emoji_picker.txt`, the picker's groups and order |
| `fetch_noto_missing.py` | downloads and rasterises the flag images noto ships only as SVG |
| `build_atlas.py` | stages the referenced tiles and packs them with `build_interface` |
| `extend_font.py` | adds a few missing glyphs to ryzom.ttf from a donor font |
| `add_emoji_to_font.py` | gives **any** TTF the full emoji set |

## Regenerating the table

`emoji.txt` maps every name Zulip can send to its codepoint sequence and image
name. It is built from upstream Zulip's own `tools/setup/emoji/emoji_names.py`
at the tag matching the deployed server, and reimplements Zulip's
`generate_name_to_codepoint_map` rather than inventing a mapping.

```bash
for f in emoji_names.py emoji_setup_utils.py; do
  curl -sSO "https://raw.githubusercontent.com/zulip/zulip/12.0/tools/setup/emoji/$f"
done
./gen_emoji_table.py --zulip-dir . --images <noto-png-dir> \
    --out emoji.txt --report coverage.txt
```

## Regenerating the picker

`emoji_picker.txt` is the layout of the in-game emoji picker: which tabs it has,
which emoji sit in each and in what order, and the description under the hovered
one. All of that is Unicode's, from the same `emoji-test.txt` every emoji
keyboard is built from; the names are Zulip's canonical ones, so picking an
emoji in game spells it the way the web chat spells it.

```bash
curl -sSO https://unicode.org/Public/emoji/16.0/emoji-test.txt
./gen_emoji_picker.py --emoji-test emoji-test.txt --zulip-dir . \
    --table ../../client/data/gamedev/interfaces_v3/emoji.txt \
    --out ../../client/data/gamedev/interfaces_v3/emoji_picker.txt \
    --report picker_coverage.txt
```

Run it again after the table changes: an emoji the table does not name is left
out of the picker, since the player could not type it either.

Local corrections belong in `emoji_overrides.txt`, which the client loads after
`emoji.txt` and which wins. That is the escape hatch for anything our Zulip does
differently from upstream; do not hand-edit the generated file.

## Changing the chat font

Emoji in the font-based display mode are glyphs, so swapping the font would
empty that mode -- no ordinary text font carries ~1400 emoji. Run the new font
through `add_emoji_to_font.py` and the support comes along:

```bash
./add_emoji_to_font.py --target MyFont.ttf --codepoints-from emoji.txt --report-only
./add_emoji_to_font.py --target MyFont.ttf --donor <ryzom.ttf> \
    --codepoints-from emoji.txt --out MyFont-emoji.ttf --sheet proof.png
```

Using the shipped `ryzom.ttf` as the donor gives full coverage in one pass; the
Noto Emoji build on google/fonts is missing a handful of recent codepoints.

## Tests

`tests/` holds the tests for `CViewText::getFormatTagLength` and
`getFormatTagPrefixAt`, which are what keep chat text colour correct when a
message is split around an emoji or a URL. They link the real `libnelgui`, so
they exercise shipped code rather than a copy.

They are not wired into `nel_unit_test`: that needs cpptest, which is not
installed here, and `WITH_NEL_TESTS` is off. Run them on demand with
`tests/run_format_tag_tests.sh` after building the `nelgui` target.
