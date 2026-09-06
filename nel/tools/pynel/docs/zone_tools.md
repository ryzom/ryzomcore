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

## 3. `CConfigFile` syntax (`nel/include/nel/misc/config_file.h:32-100`)

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

## 4. Real pipeline scripts (confirmed, not guessed)

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

## 5. pynel orchestration (`pynel.ryzom_zone_tools`)

Thin subprocess wrappers only -- no automatic binary discovery (the caller
passes an explicit path to each binary; pynel stays decoupled from where
Nuno keeps the `ryzom-docker/tools` build output) and no automatic
parameter-file generation (scene/continent-specific, the caller supplies a
real `.cfg`, following the `zone_lighter.cfg`/`properties_*.cfg` examples
above). Since `zone_lighter`'s exit code is always 0 even on failure (see
above), callers must validate the produced file (e.g. `load_zone()`
succeeding) rather than trust the return code alone.

See `ryzom_zone_tools.py` docstrings for the exact function signatures.
