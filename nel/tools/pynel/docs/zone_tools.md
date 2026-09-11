# `zone_welder`/`zone_lighter` reference (native tools, invoked as subprocesses)

Source of truth: `nel/tools/3d/zone_welder/zone_welder.cpp`,
`nel/tools/3d/zone_lighter/zone_lighter.cpp`,
`nel/tools/3d/zone_lib/zone_utility.cpp`/`.h`, and
`nel/include/nel/misc/config_file.h` (the `.cfg` format both tools read).
Investigated 2026-09-06 for `project-todos/pynel/zone_read_write.md` step 4
(orchestration: pynel/Forgery drive these binaries as subprocesses to
produce a real `.zonew`/`.zonel`, not a rename). Binaries are built via
`ryzom-docker/tools` (see `nel/tools/3d/CMakeLists.txt`, no `WIN32` guard on
either target -- confirmed buildable and runnable on Linux, step 3 of the
same chantier).

These tools were **not written for pynel** -- they're the real production
build tools, unchanged. This doc describes their existing behavior so pynel
can drive them correctly, not a new design.

## 1. `zone_welder`

```
zone_welder <input.zone> <output.zone> [<weld_threshold>]
zone_welder /?                                              -- help only
```

- `argc < 3` or `argv[1] == "/?"`: prints usage, exits 0 (not an error code).
- `weld_threshold` optional, default **1.1** (`weldRadius`, `zone_welder.cpp:53`).
- `inputDir`/`inputExt` = directory+extension of `argv[1]`; `outputDir`/
  `outputExt` = directory+extension of `argv[2]` (`zone_welder.cpp:994-997`).
  The zone's bare name (no dir/ext) is `getName(argv[1])`.
- Always writes a log to **`./log.txt`** (the process's current working
  directory, not next to the input/output files) -- free-text weld detail,
  not meant for parsing (`zone_welder.cpp:988`).

### Behavior (`weldZones()`, `zone_welder.cpp:183-964`)

1. Loads `inputDir/<center><inputExt>`.
2. If the zone has no id yet (`ZoneId==0`), assigns one via `createZoneId()`
   (`zone_utility.cpp:269`: `((y-1)<<8) + x`, `x`/`y` decoded from the
   **filename**, see naming convention below -- note the `-1` on `y`).
3. `CleanZone()` -- internal cleanup (unresolved 1-1/1-2/1-4 binds, internal
   weld, forces tangents). Implementation not located in `zone_welder.cpp`
   itself (declared forward at line 178) -- likely another file in the same
   tool directory; not required to know its internals to drive the CLI.
4. `CZoneSmoother::smoothTangents()` on edges internal to the center zone
   only (30 degrees, lines 238-245).
5. Computes the **8 adjacent zone names** via `getAdjacentZonesName()` (see
   naming convention below) -- a name past the grid edge (x<0 or y<0)
   becomes `"empty"` and is skipped.
6. **Loads the neighbors from `outputDir/<name><outputExt>`** -- NOT
   `inputDir`. The neighbors must already exist in the **output** directory.
   This matches the real pipeline (`build_gamedata`, see below): every zone
   in the source directory is processed once, writing into the same output
   directory each time -- a zone processed early can only weld against
   neighbors already written, so full-grid welding may need more than one
   pass depending on processing order (the real pipeline does a single
   alphabetical pass over `ls zone_exported/*.zone`, with no geographic
   ordering guarantee).
7. For each neighbor found: welds nearby vertices (distance < `weld_threshold`)
   via a quadtree, averages tangents, averages border-vertex positions,
   makes shared edges coplanar (`CZoneTgtSmoother::makeVerticesCoplanar`).
8. **Bind conflict** (a patch already bound to a different neighbor):
   accumulated as a warning message, not an exception -- if any conflict
   message exists at the end, **nothing is written** (neither the center
   nor any neighbor), only `nlwarning` output (lines 951-962).
9. On success: **rewrites every modified adjacent zone AND the center zone**
   into `outputDir` (lines 914-950) -- so one invocation can write up to 9
   files (center + up to 8 neighbors), not just the literal `argv[2]` path
   (which only names the center's output; neighbors are written as
   `outputDir/<their name><outputExt>`).

### Zone naming/grid convention (`zone_lib/zone_utility.cpp`)

`getZoneCoordByName()` (line 97): name = `"<row>_<letters>"` -- `row` =
digits before the first `_` (parsed directly as uint16), `letters` = a
base-26 decode (`x = x*26 + (letter - 'a')` per character, case-insensitive)
-- **confirmed consistent** with pynel's `zone_name_to_world_pos()`
(`getPosFromZoneName()`, `zone_util.cpp:33`): same letter-decoding formula,
only difference is `getPosFromZoneName` multiplies by 160 (world units) and
requires exactly 2 letters (error otherwise), while `getZoneCoordByName`
accepts any letter-string length -- for a standard 2-letter name, same
result in grid units (just without the x160).

`getZoneNameByCoord()` (line 168): inverse -- `"<y>_<letters(x)>"`,
`getLettersFromNum()` always emits exactly 2 uppercase letters (`num/26`,
`num%26`; `num > 26*26` only warns, doesn't throw).

`getAdjacentZonesName()` (line 190): the 8 neighbors in order
`[NW, N, NE, W, E, SW, S, SE]`. Low-edge names (x<0 or y<0) become `"empty"`;
**no high-edge check** exists for E/SE/S/SW (only N/NW/NE/W check `<0`) --
a name past the top of the grid is not specially handled, it just produces
a filename that won't exist (silently skipped by the `if (f.open(...))`
check).

## 2. `zone_lighter`

```
zone_lighter <in.zone> <out.zonel> <parameter_file.cfg> <dependency_file.cfg> [-waterpatch bkupdir]
```

- `argc < 5`: prints usage only, no distinct error exit code.
- `-waterpatch bkupdir` (`argc==7 && argv[5]=="-waterpatch"`): a special
  mode used by the real pipeline's `patch_tile_water.sh` -- **requires
  `argv[2]` to already exist** (it's reloaded, not created), backs it up
  into `bkupdir/<output filename>`, then recomputes only water-related tile
  flags (`computeTileFlagsOnly()`) without a full relight. Out of scope for
  this chantier (full lighting), documented to avoid confusion later.
- Output: writes **exactly one file**, `argv[2]` -- unlike the welder, it
  never rewrites dependency zones.
- **Exit code is always 0**, even on internal failure (file-not-found,
  parse exceptions -- all caught and logged as `nlwarning`/`fprintf(stderr,
  ...)`, not propagated as a process exit code). A caller must check the
  output file was actually produced/valid, not just the exit code.

### `parameter_file` (`CConfigFile`, see format below)

`CZoneLighter::CLightDesc` fields (`nel/include/nel/3d/zone_lighter.h:67-156`),
all read from the parameter file, confirmed exhaustive (no field missing):

```
bank_name                    string   -- tile bank filename (e.g. "primeracines.smallbank")
search_pathes                string[] -- extra CPath::addSearchPath() entries
additionnal_ig                string[] -- extra .ig files loaded unconditionally
load_ig                      int (0/1)
shadow                       int (0/1) -- gates loading dependency_file at all
sun_direction                real[3]
sun_center                   real[3]
sun_distance                 real
sun_fov                      real (radians)
sun_radius                   real
zbuffer_landscape_size       int
zbuffer_object_size          int
soft_shadow_samples_sqrt     int
soft_shadow_jitter           real
sun_contribution             int (0/1)
sky_contribution             int (0/1)
sky_intensity                real
global_illumination_cell_size  real
water_shadow_bias            real
water_ambient                real
water_diffuse                real
modulate_water_color         int (0/1)
sky_contribution_for_water   int (0/1)
global_illumination_length   real
quad_grid_size                int
quad_grid_cell_size          real
cpu_num                       int (0 = auto-detect)
vegetable_height              real
```

Optional extra vars (used only if present, wrapped in
`try/catch(EUnknownVar)` -- silently ignored otherwise,
`zone_lighter.cpp:193-276`): `continent_name`, `level_design_directory`,
`level_design_world_directory`, `level_design_dfn_directory` -- load a
`.continent` Georges sheet's villages to auto-add their `.ig` to the
relevant dependency zones. **Not needed for a minimal test** -- omit them.

The zone's own `.ig` (`<input zone name>.ig`) is looked up automatically
via `CPath::lookup` (must be reachable via `search_pathes` or the working
directory) -- absent is only a warning, not fatal. Same for `bank_name`.

**Tile-noise (displacement bump-map) lookup is broken on Linux, found
2026-09-11**: loading the tile bank (`bank_name`'s `.smallbank`, or more
precisely the underlying `.bank` referenced by `tile_bank_file`) can trigger
`CTileBank::getTileNoiseMap()` (`nel/src/3d/tile_bank.cpp:475`), which
builds its file path as `getAbsPath() + tileNoise._FileName` -- **both of
these strings are read verbatim from inside the `.bank` binary itself**
(`f.serial(_AbsPath)`/`f.serial(_FileName)`, not derived from
`properties.cfg` or any tool config), baked in from whatever Windows
machine originally authored the bank (e.g. `_AbsPath =
"R:/graphics/landscape/_texture_tiles/jungle/"`, `_FileName =
"displace\j_crevasse1.png"`). `CPath::lookup()` strips everything before the
last `/` to get a bare filename to search for, but since the embedded
relative name uses `\` (not `/`), that backslash stays glued to
`"displace\j_crevasse1.png"` as one opaque token, which never matches a
real registered file on Linux even when it exists on disk. Not yet fixed
(would need a code change in `tile_bank.cpp` to fall back to a `/`-and-`\`
-aware basename lookup via `search_pathes` when the literal legacy path
fails) -- and per Nuno's own rule (2026-09-11), any such fallback must
route through `pipeline/`, never `graphics/` (`docs/pipeline_directories.md`),
so `search_pathes` now includes `pipeline/export/ecosystems/<eco>/displace`
(§9) ready for whenever this fix lands.

### `dependency_file` (only loaded if `shadow=1`)

One var: `dependencies = { "zoneA", "zoneB", ... };` -- names resolved as
`<dir of argv[1]>/<name><ext of argv[1]>` (uses the **input** zone's
directory/extension, unlike the welder which uses the output side). Each
listed zone is loaded purely to contribute cross-zone shadows/lighting; its
`.ig` is also loaded (if `load_ig=1`). An empty `dependencies = {};` is
valid -- it just means no cross-zone shadow casting, still testable.

In the real pipeline, this file is generated by a **third tool**,
`zone_dependencies` (`nel/tools/3d/zone_dependencies/`): `zone_dependencies
<properties.cfg> <firstZone.zone> <lastZone.zone> <output.cfg>` -- computes
real shadow dependencies via bounding-box analysis across a zone range.
Out of this chantier's explicit scope (only welder/lighter); an empty
`dependencies = {};` is a legitimate substitute when no cross-zone shadow
analysis is needed.

## 3. `zone_elevation`

Investigated 2026-09-11 for `project-todos/pynel/land_pipeline.md` step 4.
Source: `nel/tools/3d/zone_elevation/zone_elevation.cpp`. Applies a heightmap
to a zone freshly produced by `land_export` (a `.zonenhw` -- "no height,
welded" -- becomes a `.zonew` with real Z positions).

```
zone_elevation <input.zonenhw> <output.zonew> --land <file.land> [options]
zone_elevation <input.zonenhw> <output.zonew> --zonemin <name> --zonemax <name> [options]
```

Unlike every other tool in this doc, `zone_elevation` uses `NLMISC::CCmdArgs`
(a real, modern argument parser: `--flag value` or `--flag=value`, both
accepted) and a **real exit code convention**: `EXIT_SUCCESS`/`EXIT_FAILURE`
(`main.cpp:498,607,612`) -- not the always-0 convention of the other tools
in this doc.

- **Required**: the two positional args (input/output), and either `--land`
  (a real `.land` file, loaded to derive the zone grid's min/max bounds) or
  both `--zonemin`/`--zonemax` (zone **names**, not files -- decoded via
  `getXYFromZoneName`, no file needed for this alternative).
- **Optional, never fatal**: `--heightmap`/`--heightmap2` -- every use of
  the loaded bitmap is guarded by `if (s_HeightMap != NULL)`
  (`zone_elevation.cpp:126,174`); a missing flag, a missing file, or a
  load exception all just leave it `NULL` and processing continues with no
  elevation applied from that source (logged via `nldebug`, not
  `nlwarning` -- see the log-routing note in §7 below for why that still
  reaches `log.log`).
- **Defaults** (real values in `zone_elevation.cpp`, used whenever the
  matching flag is omitted): `--zfactor`/`--zfactor2` = `1.0`, `--cellsize`
  = `160.0`, `--extendcoords` = off.

**Real bug found and fixed in `ryzom-core`, 2026-09-11** (`fixes` branch):
the tool crashed with a SIGSEGV on every invocation, even with zero CLI args
-- confirmed via `gdb` backtrace, the crash happened inside
`std::string::assign()` called from `NLLIGO::CZoneRegion::CZoneRegion()`,
itself called during static initialization (before `main()`), because
`zone_elevation.cpp` declared its `CZoneRegion s_Land;` as a plain **global**
(so it gets constructed at static-init time) instead of a local/heap-allocated
instance the way `land_export` does. Fixed by making it
`CZoneRegion *s_Land = NULL;`, allocated with `new` inside `loadLand()` --
same pattern this file already used for `s_HeightMap`/`s_HeightMap2`.

## 4. `zone_dependencies`

Investigated 2026-09-11 for `project-todos/pynel/land_pipeline.md` step 4.
Source: `nel/tools/3d/zone_dependencies/zone_dependencies.cpp`. Computes
cross-zone shadow/lighting dependencies by bounding-box analysis, writing
the `.depend` file `zone_lighter` (§2) and `zone_ig_lighter` (§5) both
consume.

```
zone_dependencies <properties.cfg> <firstZone.zone> <lastZone.zone> <output.depend>
```

- **Required**: `properties.cfg` itself, with (no `try/catch`, so genuinely
  required) `sun_direction`, `search_pathes`, `compute_dependencies_with_igs`.
- **`firstZone.zone`/`lastZone.zone` are never actually opened as files** --
  only their **names** are decoded (`getZoneCoordByName`) to derive a
  rectangular `[minX..maxX] x [minY..maxY]` coordinate range, and their
  shared directory/extension (`getDir`/`getExt` on `argv[2]`) to reconstruct
  every real zone's path inside that range. Neither file needs to exist on
  disk for the tool to run.
- Every zone coordinate in that rectangle is tried (`<dir>/<computed name>
  <ext>`) -- a coordinate with no corresponding file is silently skipped
  (a normal, expected case: not every grid cell has a zone).
- **`compute_dependencies_with_igs=1`** (true for every real continent
  checked, 2026-09-11) additionally tries, per loaded zone:
  - `<zone>.ig` via `CPath::lookup` -- **the `nlwarning` for this case is
    commented out in the source** (`zone_dependencies.cpp`, in
    `computeZoneIGBBox`): a missing `.ig` produces **zero trace anywhere**,
    not even in `log.log`. Confirmed by reading the code, not inferred --
    see §7 for what this means for pynel's own validation.
  - Every `.shape` referenced by that `.ig`'s instances, via `CPath::lookup`
    -- optional, a real `nlwarning` on failure (`"Unable to find shape
    '%s'"`).
  - The continent's own `.continent` form (`computeIGBBoxFromContinent()`),
    built from `level_design_world_directory`/`continent_name` -- wrapped in
    `try/catch(EUnknownVar)`, fully optional. Its path is constructed as a
    flat `<level_design_world_directory>/<continent_name>.continent`, but
    resolved via `CFormLoader::loadForm()` -> `CPath::lookup()` first (which
    matches by filename anywhere under whatever was registered with
    `addSearchPath(level_design_world_directory, recurse=true, ...)` just
    before) -- so this still finds the real file even though `ryzom-data`'s
    actual current layout nests it one level deeper
    (`leveldesign/world/continents/<name>.continent`, see §9), **as long as
    `continent_name` matches the real file's own basename**.

**Two real bugs found running this tool over a whole continent (nexus, 151
zones), 2026-09-11**:

- **The coordinate decoder is NOT the same as `zone_elevation`'s own.**
  `zone_dependencies` decodes `firstZone`/`lastZone` names via the SHARED
  `getZoneCoordByName()` (`nel/tools/3d/zone_lib/zone_utility.cpp`), which
  does **not** negate the row into Y (`y = row` directly) -- unlike
  `zone_elevation.cpp`'s own LOCAL `getXYFromZoneName()`, which does `y =
  -row`. These are two genuinely different functions despite decoding the
  same `row_XY` name format; a caller must never assume one tool's sign
  convention applies to the other.
- **The min/max swap is broken.** `zone_dependencies.cpp`'s own range-fixup
  (`if (lastX<firstX) { tmp=firstX; firstX=lastX; lastX=firstX; }`) has a
  copy-paste bug: the second assignment overwrites `lastX` with the
  already-mutated `firstX`, so instead of swapping, BOTH end up equal to the
  smaller value -- collapsing that axis's range to a single row/column
  instead of covering the intended span. Confirmed by constructing
  `firstZone`/`lastZone` names that require a swap and observing the
  resulting `.depend` count collapse from ~151 to 6. **Workaround (not a
  fix)**: always pass `firstZone`/`lastZone` names already in ascending
  row/col order (using `getZoneCoordByName()`'s own non-negated convention
  above), so the swap path is never triggered at all.

## 5. `zone_ig_lighter`

Investigated 2026-09-11 for `project-todos/pynel/land_pipeline.md` step 4.
Source: `nel/tools/3d/zone_ig_lighter/zone_ig_lighter.cpp`. Lights the
instances (`.ig`) placed in an already-lit zone (`.zonel`), using the same
`properties.cfg`/`.depend` convention as `zone_lighter` (§2) plus its own
`ig_oversampling`.

```
zone_ig_lighter <input.zonel> <output.ig> <properties.cfg> <dependency.depend>
```

- **Required, and genuinely blocking unlike every other file in this doc**:
  `<input.zonel>`, `properties.cfg`, `<dependency.depend>` (loaded with no
  `try/catch` around `dependency.load()` -- must exist and parse, an empty
  `dependencies = {};` is fine per §2's own note) -- **and the zone's own
  `<name>.ig`**, auto-looked-up (`<name>.ig`, `CPath::lookup`, same
  `search_pathes`-registered dirs as everything else). If that specific
  lookup fails: `zoneIgLoaded = false` -> **immediate `return 0`, the whole
  process exits having produced nothing**, with only a single
  `fprintf(stderr, "Warning: can't load instance group %s\n", ...)` (not
  even `nlwarning`, so it never reaches `log.log` either -- stderr only).
- `properties.cfg` fields read (no `try/catch`, so all required, matching
  the base list in §2 plus): `sun_direction`, `quad_grid_size`,
  `quad_grid_cell_size`, `shadow`, `ig_oversampling`, `search_pathes`,
  `bank_name`, `load_ig`.
- **Optional, warning-only**: `bank_name`'s actual tile bank file (continues
  without tile info if missing); every zone/`.ig` listed in the
  `dependencies = {...}` of the `.depend` file (each tried independently,
  `nlwarning` + `continue` on failure); every `.shape` referenced by any
  loaded `.ig` (center, dependency, or `additionnal_ig`), `nlwarning` on
  failure, same pattern as §4.
- **Exception**: if `additionnal_ig` (properties.cfg) is non-empty and ANY
  listed file fails to open, `continu=false` is set and the actual lighting
  is skipped -- but every real continent checked 2026-09-11 has this list
  empty, so this path is not currently exercised in practice.
- Always returns `0`, whether it produced real output or not -- same
  non-standard convention as `zone_lighter` (§2): the caller must check the
  output file itself, never the exit code.

## 6. `CConfigFile` syntax (`nel/include/nel/misc/config_file.h:32-100`)

```
// single-line comment
/* multi-line
   comment */

var1 = 123;                 // int
var2 = "some string";       // string
var3 = 123.456;             // real (double)
var4 = 123 + 2;             // arithmetic expressions supported (+,-,*,/);
                             // result type follows the first operand's type
var5 = { 10.0, 51.1 };      // array of real
var6 = { "a", "b" };        // array of string
```

- Redefining a var later in the file overwrites the earlier value.
- Documented quirk (in the header itself): a file ending in a comment with
  no trailing newline throws on load -- always end `.cfg` files with a
  blank line.
- `getVar()` raises `NLMISC::EUnknownVar` for a missing variable (matches
  every `try/catch(EUnknownVar)` seen in `zone_lighter.cpp`).

## 7. Real pipeline scripts (confirmed, not guessed)

`nel/tools/3d/build_gamedata/processes/zone/sh/build.sh` (weld step):
```bash
zone_welder zone_exported/<name>.zone zone_welded/<name>.zonew
```
One alphabetical pass over `ls zone_exported/*.zone` -- no geographic
ordering guarantee (see the welder's neighbor-availability caveat above).

`nel/tools/3d/build_gamedata/processes/zone_light/sh/build.sh` (light step):
```bash
zone_lighter ../zone/zone_welded/<name>.zonew zone_lighted/<name>.zonel \
    zone_lighter_properties.cfg ../zone/zone_depend/<name>.depend
```
`zone_lighter_properties.cfg` is a copy of `cfg/properties.cfg` (real
examples in the repo: `nel/tools/3d/build_gamedata/cfg/properties_final.cfg`/
`properties_draft.cfg`, plus `continent_name`/`level_design_*` appended
dynamically by the script). `.depend` files are generated by
`zone_dependencies` (see above), one per zone.

## 8. pynel orchestration (`pynel.ryzom_zone_tools`)

Thin subprocess wrappers only -- no automatic binary discovery (the caller
passes an explicit path to each binary; pynel stays decoupled from where
Nuno keeps the `ryzom-docker/tools` build output) and no automatic
parameter-file generation (scene/continent-specific, the caller supplies a
real `.cfg`, following the `zone_lighter.cfg`/`properties_*.cfg` examples
above). Since `zone_lighter`'s exit code is always 0 even on failure (see
above), callers must validate the produced file (e.g. `load_zone()`
succeeding) rather than trust the return code alone.

See `ryzom_zone_tools.py` docstrings for the exact function signatures.

## 9. `continent_pipeline_reference.csv` -- per-continent path/properties reference

Built 2026-09-11 for `project-todos/pynel/land_pipeline.md` step 4, at
Nuno's explicit request for **one single** reference table (not split by
source file) of every path and `properties.cfg` field the three tools in
§3-5 need, for **all 25 continents**. Lives at
`ryzom-data/leveldesign/world/continent_pipeline_reference.csv` (25 rows,
one per continent found under
`leveldesign/workspace/continents/*/directories.py`).

### Source of truth: `directories.py`, not the old generated `.cfg` files

The first version of this CSV was built from each continent's real
`generated/properties.cfg`/`generated/land_exporter.cfg`. Both turned out to
be **stale**, still pointing at two conventions abandoned by
`ryzom-data/world_continents_flatten.md`'s reorganization and never
regenerated since:
- the pre-flatten `leveldesign/world/<old_internal_name>/<old_internal_name>.continent`
  layout (e.g. nexus's old internal name was `lecarrefour`;
  `leveldesign/world/lecarrefour/` no longer exists at all);
- the obsolete `.land` location `graphics/landscape/ligo/<eco>/<name>.land`,
  superseded by `leveldesign/landscape/<eco>/<name>.land`.

Nuno's explicit correction after this was caught: the CSV must be a
reference of what **currently, actually exists**, "comme si les anciens
dossiers n'avaient jamais existe" -- never a faithful copy of a stale
config just because that's what the old file says. So the CSV is rebuilt
entirely from `leveldesign/workspace/continents/<continent>/directories.py`
-- present for all 25 continents, plain declarative Python (string
concatenation only), already the trusted source `ryzom_forgery.continent_ecosystem`
uses in Forgery. `directories.py` defines (relative to `pipeline/export/`,
except `TileRootSourceDirectory` which is relative to `graphics/`):
`EcosystemName`, `ContinentName`, `PropertiesExportBuildSearchPaths` (the
exact `search_pathes` list), `SmallbankExportDirectory`,
`FarbankBuildDirectory`, `RbankOutputBuildDirectory`,
`LigoEcosystemZoneExportDirectory`/`LigoEcosystemIgExportDirectory`/
`LigoEcosystemZoneLigoExportDirectory`,
`LigoZoneBuildDirectory`/`LigoIgLandBuildDirectory`/`LigoIgOtherBuildDirectory`,
`TileRootSourceDirectory`.

The builder script (`build_reference_csv.py`, kept as reference in this
doc's git history / re-derivable from `directories.py` at any time) `exec()`s
each continent's `directories.py` in an isolated namespace -- safe since the
file has no imports or side effects, just string concatenation -- and
derives every column from it. Confirmed correct against disk: every
`continent_file`/`zone_region_file` entry moved from mostly-broken (stale
paths) to 100% `OK` after the switch to `directories.py`.

### Columns still sourced from the real generated `properties.cfg`

A handful of `properties.cfg` fields are genuine per-continent artistic/
tuning values with **no equivalent in `directories.py`** (which only knows
about paths, not scene-lighting parameters) -- these are still read from
each continent's real `generated/properties.cfg`, the only current source
for them: `sun_direction`, `sun_center`, `sky_intensity`, `quad_grid_size`,
`quad_grid_cell_size`, `additionnal_ig`. Verified 2026-09-11 across 6 real
continents (nexus, bagne, fyros, matis, tryker, terre) that every other
`properties.cfg` field is identical across all continents (see §3-5's field
lists) -- those are stored once in the CSV-builder's `GLOBAL_PROPERTIES`
dict, not duplicated 25 times as a column.

### `*_status` columns

Every path-like column has a matching `<field>_status` column
(`OK`/`EMPTY`/`MISSING`/`ERROR`), computed by actually checking the real
`ryzom-data` checkout on disk at build time -- e.g. `bank_name_status` is
genuinely `MISSING` for every desert/lacustre-ecosystem continent (real,
confirmed-not-downloaded assets, not a path-construction bug -- do not
"fix" these by changing the path formula).

### `workspace/` and `graphics/` are now permanently off-limits (Nuno, 2026-09-11)

The `directories.py`-based build described above was a **one-time**
extraction, not a standing dependency. Nuno drew a hard line right after:
"on ne touche a aucun fichier de workspace/ on modife notre .csv. Legacy
pipeline = workspace. Forgery pipeline = csv. Workspace ne dois surtout pas
etre utilisé non plus. On utilise que pipeline/ et world/" -- `leveldesign/
workspace/` (where `directories.py` lives) belongs entirely to the
**legacy** build_gamedata pipeline; Forgery/pynel must never read OR modify
anything under it again. `graphics/` is off-limits too, separately -- no
Forgery-facing tool may ever read from it, it is not a validated data
source. The only two valid data roots for the Forgery-side pipeline from
now on are `pipeline/` and `world/` (i.e. `leveldesign/world/`).

Concretely: `continent_pipeline_reference.csv` is now THE one Forgery-side
source of truth, on its own -- never rebuilt from `directories.py` again. Any
addition or correction (a missing search path, a corrected field) is made
by **editing the CSV directly**, using its own already-present columns
(e.g. `ecosystem`) to derive new values, never by going back to
`workspace/`.

**First real case of this, 2026-09-11**: `zone_lighter`'s tile-noise
(displacement bump-map) texture lookup needs
`pipeline/export/ecosystems/<eco>/displace` in `search_pathes` -- a path the
legacy `directories.py`/`PropertiesExportBuildSearchPaths` never included
(that legacy convention pointed at a `graphics/`-rooted location instead,
now permanently out of scope). Nuno added the real `displace/` folders
under `pipeline/export/ecosystems/<eco>/` himself (all 4 ecosystems:
jungle, desert, lacustre, primes_racines); the CSV's `search_pathes` /
`search_pathes_status` columns were updated directly (all 25 rows, `OK`)
to include it, with no `directories.py` involved.

### Regenerating

There's no committed CLI for building this CSV from scratch (it was a
one-off script, since superseded by the direct-edit rule above) -- going
forward, the CSV is only ever hand-edited in place, never regenerated from
`directories.py`.
