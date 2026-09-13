# region_export — continent/region/place export

Status: **implemented** (`pynel.region_export`, CLI + library).

## Why this exists

Ryzom's named lore regions per continent (e.g. Tryker mainland: `lagoonsofloria`,
`windsofmuse`, `bountybeaches`, `thefount`, `enchantedisle`, `libertylake`,
`dewdrops`, `restingwater`) and the places inside them (cities, outposts,
stables, streets...) live nowhere a generic tool/player has access to:
- `live_data_path`'s `continent.packed_sheets`/`world.packed_sheets` only
  carry a continent's bounding box and a handful of UI map "places"
  (`place_avendale`, newbie-island `region_tryker_islandN`...), never the
  real mainland lore regions.
- `ryzom.world` (Georges FORM, `pynel.ryzom_world`) is the same story: a
  `maps list` of UI zoom/click rectangles that happen to share the word
  "region" in some names, but neither the real polygons nor the real names.

The only real source is `region_<continent>.primitive` (LIGO primitive
tree, `pynel.ryzom_primitive`), which lives in the private leveldesign
authoring repository `ryzom-private-data` (`primitives/**/region_*.primitive`,
already a registered path in `pynel.repository_paths`). This module reads
those files and can also produce/consume the same data as a `.lua`/`.json`
table -- the format the real Ryzom Live client itself ships as
`gamedev/world.lua` (used by the client's own world-map UI).

## `.primitive` structure (verified 2026-09-13)

A `region_<continent>.primitive` file's `CPrimNode` root holds one or more
sibling `PrimZone class="continent"` nodes side by side (not always just
one -- e.g. `region_fyros.primitive` has `continent_fyros`,
`continent_fyros_newbie` AND `continent_fyros_islands`; `region_r2.primitive`
has 5 separate R2 sub-continents). Each continent contains `PrimZone
class="region"` children, each of which directly contains `PrimZone
class="place"` leaves (no separate per-place file).

Relevant properties (`pynel.ryzom_primitive.get_property()`):
- `class` -- `"continent"`/`"region"`/`"place"`, the level in the hierarchy
  and the ONLY reliable leaf/branch signal (a childless continent/region
  still has `class != "place"` and must still produce an empty children
  container, never be mistaken for a leaf).
- `name` -- becomes the output key.
- `displayed` -- `"true"`/`"false"`, optional (absent = visible). This is
  what becomes the leading `visible` boolean -- NOT the unrelated `hidden`
  property (a LIGO editor cosmetic flag, always empty in every real example
  seen).
- `place_type` (only on `class="place"` nodes) -- `"Capital"`/`"stable"`/
  `"Outpost"`/`"Village"`/`"Street"`/absent (default `"place"`).
- `PrimAlias` children (`class="alias"`) carry no useful data and are
  skipped -- only `PrimZone` children matter.

## Output structure

`{name: [visible, points, children_or_type]}`, recursively:
- `visible`: bool.
- `points`: `[[x, y], ...]`, coordinates **truncated toward zero** (`int()`,
  a plain C++ float-to-int cast) -- NOT rounded to the nearest integer
  (confirmed against a real point: raw `18993.566406` becomes `18993`, not
  `round()`'s `18994`).
- `children_or_type`: a `dict` of further `{name: [...]}` entries for a
  continent/region (an **empty list `[]`**, never `{}`, if it happens to
  have none), or a `place_type` string for a leaf place.

This is exactly the structure the real client's `world.lua` encodes
(`game.World = { continent_x = { true, {points}, { region_y = {...}, ... } } }`)
and the JSON form Nuno used before this module existed. Both are supported
as output/input formats:

- JSON: plain `json.dumps`/`json.loads` of the structure above -- no custom
  code needed.
- Lua: `to_lua(regions) -> str` / `parse_world_lua(text) -> regions`, a
  dedicated tokenizer + recursive-descent parser (not a general Lua engine)
  for exactly the grammar `to_lua()` itself emits. A name that isn't a
  valid bare Lua identifier (spaces, accents, punctuation -- real examples:
  `"Karavan Embassy"`, `"endroit_résidu_oeuf"`, `"region_fyros_island1PvP 3"`)
  is written/read as `["name"] = {...}` instead of `name = {...}`, matching
  the real file. The real file's trailing `-- VERSION --` /
  `FILE_WORLD_VERSION = <n>` footer is a real client build's own version
  number; a regeneration from `ryzom-private-data` has no way to know or
  reproduce it, so `to_lua()` never emits one and `parse_world_lua()` never
  reads one (everything after the table's closing `}` is ignored).

`parse_world_lua(to_lua(regions)) == regions` for any `regions`
`build_world_regions()` can produce -- round-trip fidelity is exact.

## API

- `primzone_to_json(prim: PrimZone) -> JsonNode` -- converts one already-parsed
  `PrimZone` (and its `PrimZone` children, recursively) to the output structure.
- `build_world_regions(ryzom_private_data_path) -> Dict[str, JsonNode]` --
  scans every `primitives/**/region_*.primitive` under that path and merges
  every continent root of every file into one dict.
- `to_lua(regions) -> str` / `parse_world_lua(text) -> Dict[str, JsonNode]`.

## CLI

```
python -m pynel.region_export [--format {json,lua}] [--output-dir <folder>]
```

One format per run (`--format` defaults to `json`), written as `world.<format>`
inside `--output-dir` (defaults to `<ryzom-data>`, via
`repository_paths.get("ryzom-data")`) -- to get both files, run it twice
(see `validate_region_export.py`, which does exactly that against a real
`ryzom-data` checkout as its own smoke test).

- Source: `pynel.repository_paths.get("ryzom-private-data")` (never a
  hardcoded path) -- must be configured first (`repository_paths.set_path`,
  or the "Ryzom Paths" settings section any pynel-based tool -- Forgery's
  Patina/Atyscape included -- already exposes for all 4
  `repository_paths.REPOSITORIES`).

## Usage

```python
from pynel import region_export, repository_paths

regions = region_export.build_world_regions(repository_paths.get("ryzom-private-data"))
lua_text = region_export.to_lua(regions)
regions_again = region_export.parse_world_lua(lua_text)
assert regions_again == regions
```
