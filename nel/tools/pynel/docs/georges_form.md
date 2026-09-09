# Georges FORM (`.continent`, `ryzom.world`, ...)

Reader/writer: `ryzom_georges_form.py` (generic tree) + `ryzom_continent.py`
(`.continent`) + `ryzom_world.py` (`ryzom.world`). See `docs/georges_sheets.md`
for the related, still-unimplemented `ryzom_georges.py` (sheet `PARENT`-chain
provenance) — that future module should build on top of
`ryzom_georges_form.GeorgesForm`/`GeorgesStruct` for the raw tree rather
than re-deriving it.

## Format

Plain XML, one `<FORM>` per file:

```xml
<?xml version="1.0"?>
<FORM Revision="$Revision: 1.27 $" State="modified">
  <PARENT Filename="lesfalaises.continent"/>   <!-- optional -->
  <STRUCT>
    <ATOM Name="Name" Value="Nexus Minor"/>
    <STRUCT Name="LightLandscapeDay">
      <STRUCT Name="Ambiant">
        <ATOM Name="A" Value="100"/>
      </STRUCT>
    </STRUCT>
    <ARRAY Name="ZCs">
      <STRUCT>                                  <!-- STRUCT items in an
        <ATOM Name="Zone" Value="41_cf"/>            ARRAY have no Name -->
      </STRUCT>
    </ARRAY>
    <ARRAY Name="WeatherSetups">
      <ATOM Value="fo_fair1.weather_setup"/>    <!-- ATOM items in an
      <ATOM Value="fo_fair2.weather_setup"/>        ARRAY have no Name -->
    </ARRAY>
  </STRUCT>
  <STRUCT/><STRUCT/><STRUCT/><STRUCT/>   <!-- "held elements", always empty in every file seen -->
  <COMMENTS>Converted from old format</COMMENTS>
  <LOG></LOG>
</FORM>
```

- `ATOM` — leaf key/value pair.
- `STRUCT` — a group of child ATOM/STRUCT/ARRAY. Carries its own `Name`
  attribute when nested inside a STRUCT, or when it's a *named* entry of an
  ARRAY (e.g. `ryzom.world`'s "continents list", each `<STRUCT Name="matis">`).
  No `Name` for the top-level struct or for unnamed ARRAY entries.
- `ARRAY` — an ordered list of ATOM/STRUCT/ARRAY items.
- `PARENT Filename="..."` — referenced but **not resolved/merged** by this
  module (no inheritance walk) — see `docs/georges_sheets.md` for where
  that belongs.

This is a different, unrelated XML format from `.primitive` (LIGO tree,
`ryzom_primitive.py`) despite the similar Ryzom/NeL lineage — don't confuse
the two.

## `PacsRBank` trap (continent identity)

Neither a `.continent` file's own filename nor the directory it lives in
under `leveldesign/world/` is a reliable runtime continent identifier:

| Folder | `.continent` filename | Real identifier (`PacsRBank`) |
|---|---|---|
| `lecarrefour/` | `lecarrefour.continent` | **nexus** |
| `legranddesert(fyros)/` | `fyros.continent` | fyros |
| `leseauxpures(tryker)/` | `tryker.continent` | tryker |
| `lesfalaises(matis)/` | `lesfalaises.continent` | **matis** |
| `lepaysmalade(zorai)/` | `lepaysmalade.continent` | **zorai** |

The one reliable field is the `PacsRBank` ATOM (e.g. `Value="nexus.rbank"`),
whose value with the `.rbank` suffix stripped is the identifier used
everywhere else in Ryzom tooling (`pipeline/export/continents/<name>/`,
`live_data_path`'s per-continent sheets, etc.) -- see
`ryzom_continent.ContinentFile.pacs_rbank`. Cross-checked 2026-09-09 against
`ryzom-data/leveldesign/workspace/continents/<name>/directories.py`'s
`ContinentName`/`EcosystemName` (an independent, non-XML source used by the
legacy `build_gamedata` pipeline) -- values match, confirming the mapping.

`ryzom.world`'s own `continent_name` field is *also* not always reliable —
e.g. the entry named `matis` (`STRUCT Name="matis"`, `selection_name="matis"`)
has `continent_name="lesfalaises"`, which matches neither `PacsRBank` nor
anything else. Use `ryzom_continent.ContinentFile.pacs_rbank`, never
`ryzom_world.WorldContinentEntry.continent_name`, to identify a continent.

## Writing

`dumps_georges_form`/`save_georges_form` (and the `dumps_continent`/
`dumps_world` wrappers) round-trip an existing file's field order (Python
dict insertion order) rather than deriving the canonical order from the
governing `.dfn` (e.g. `leveldesign/DFN/world/continent.dfn`) the way the
real engine's `CFormElmStruct::build()` does — no `.dfn` parser exists in
pynel. This is correct for editing a field that's already present; a
brand-new field lands at the end of its STRUCT instead of its DFN-declared
position. Real game-data files use `Revision="$Revision: x.y $"` (a CVS
keyword string), not the `Version="X.Y"` attribute the generic NeL engine
sample (`nel/samples/georges/`) writes — this module matches the real files.

## Not implemented

- `PARENT` inheritance resolution/merging (see `docs/georges_sheets.md`).
- `.dfn`-derived field ordering for newly-added fields.
- ATOM values containing a newline (written as a text child by the engine
  instead of a `Value` attribute) — not encountered in `.continent`/
  `ryzom.world`, not handled.
