# `pipeline/` directories actually used by the Forgery-side land pipeline

Nuno, 2026-09-11: "je veux aussi savoir quels sont les dossiers utiliser
dans pipeline/ pour chaque outil, a mettre dans un doc/ a part. L'idée
c'est de savoir s'il y a des dossier inutilisé pour cleaner les archives
.zip. Donc tout ce qui est utile on le note" -- this lists every
`pipeline/`-rooted directory the 6 tools we actually orchestrate
(`land_export`, `zone_welder`, `zone_elevation`, `zone_dependencies`,
`zone_lighter`, `zone_ig_lighter`, see `docs/zone_tools.md`) read or write,
confirmed against each tool's real config fields (`LandExportConfig`/
`PropertiesConfig`, `pynel.ryzom_land_tools`) and
`continent_pipeline_reference.csv`'s own columns.

**Scope note**: this covers only OUR 6-tool subset. A real `pipeline/
export/<continent>/` directory has many more subfolders than listed here
(e.g. `ai_wmap`, `rbank_*`, `shape_lightmap*`, `ig_elev_*`, `ig_static_*`,
`ig_temp_*`, `shape_clodtex_build`, `map_tag`, `zone_lwsl_temp` --
confirmed present under `pipeline/export/continents/nexus/` 2026-09-11) --
those belong to OTHER steps of the full legacy build_gamedata pipeline
(far-LOD banks, AI navigation, lightmap variants, etc.), never touched by
this project's 6 tools. Anything not in the tables below is a candidate
for trimming from a `pipeline/` `.zip` this project downloads, but
confirm first whether some other part of Forgery (or a future chantier)
needs it before deleting anything real.

Per the rules in `docs/zone_tools.md` §9: `pipeline/export/` holds NeL-
tool-generated output -- `graphics/` and `leveldesign/workspace/` are
never valid sources for any of this (see `continent_pipeline_reference.csv`'s
own methodology notes). Raw/authored source data that isn't tool-generated
output (heightmaps, tile banks, `.land`/`.continent` files) lives under
`leveldesign/<subfolder>/<eco or continent>/` instead -- `pipeline/landscape/`
briefly held the tile bank file and, even more briefly, the heightmaps (see
the "Outside `pipeline/` entirely" table below) but Nuno moved both for real
2026-09-12, so `pipeline/landscape/` is unused by this project's tools as of
that date.

## Per-ecosystem (`pipeline/export/ecosystems/<eco>/`)

| Directory | Role (CSV column) | Used by | Read/Write |
|---|---|---|---|
| `shape_optimized` | `search_pathes` (`ShapeLookupDirectories`) | `zone_dependencies`, `zone_lighter`, `zone_ig_lighter` | read |
| `shape_with_coarse_mesh` | `search_pathes` | same 3 | read |
| `map_export` | `search_pathes` (`MapLookupDirectories`) | same 3 | read |
| `map_uncompressed` | `search_pathes` | same 3 | read |
| `pacs_prim` | `search_pathes` (`PacsPrimLookupDirectories`) | same 3 | read |
| `displace` | `search_pathes` -- added 2026-09-11 | same 3, once the `tile_bank.cpp` backslash-lookup fix lands (`docs/zone_tools.md` §2) | read |
| `smallbank/<eco>.smallbank` | `bank_name` | `zone_lighter`, `zone_ig_lighter` | read |
| `ligo_es/zones` | `ref_zone_dir` | `land_export` | read |
| `ligo_es/igs` | `ref_ig_dir` + `additionnal_ig_in_dir` | `land_export` | read |
| `ligo_es/zoneligos` | `ligo_bank_dir` | `land_export` | read |
| `ligo_es/cmb` | `ref_cmb_dir` | `land_export` | read |
| `farbank/<eco>.farbank` | `bankfar_name` | **none of our 6 tools** -- belongs to `build_far_bank`, a different NeL tool not yet in scope | -- |

## Per-continent (`pipeline/export/continents/<name>/`)

| Directory | Role (CSV column) | Used by | Read/Write |
|---|---|---|---|
| `ig_land` | `search_pathes` (`IgLookupDirectories`) | `zone_dependencies` (only if `compute_dependencies_with_igs=1`), `zone_lighter`, `zone_ig_lighter` (the zone's own `.ig` lookup) | read |
| `ig_other` | `search_pathes` | same 3 | read |
| `shape_optimized` | `search_pathes` | same 3 | read |
| `shape_with_coarse_mesh` | `search_pathes` | same 3 | read |
| `map_export` | `search_pathes` | same 3 | read |
| `map_uncompressed` | `search_pathes` | same 3 | read |
| `ligo_zones` | `out_zone_dir` | `land_export` | write |
| `ligo_ig_land` | `out_ig_dir` | `land_export` | write |
| `ligo_ig_other` | `additionnal_ig_out_dir` | `land_export` | write |
| `rbank_cmb_export` | `out_cmb_dir` | `land_export` | write |
| `rbank_output/<name>.gr` / `.rbank` | `grbank` / `rbank` | **none of our 6 tools** -- these enable `CSurfaceLighting` in `zone_lighter`'s `.cfg` grammar (`nel/include/nel/3d/zone_lighter.h`), but neither field is in the confirmed-required/optional list for any of the 3 lighting tools (`docs/zone_tools.md` §2/§4/§5) -- likely consumed by a separate rbank-building step, not this project's scope | -- |

## Not folder-based (single files, still under `pipeline/`)

| Path | Role | Used by |
|---|---|---|
| `pipeline/export/continents/<name>/rbank_output/...` | see above | out of scope |

## Outside `pipeline/` entirely (still valid, per `docs/zone_tools.md` §9)

| Path | Role | Used by |
|---|---|---|
| `leveldesign/world/continents/<name>.continent` | `continent_file` | `land_export`; optionally `zone_dependencies` (`computeIGBBoxFromContinent`) |
| `leveldesign/landscape/<eco>/<name>.land` | `zone_region_file` | `land_export`, `zone_elevation` (`--land`) |
| `leveldesign/landscape/<eco>/big_<continent>.tga` | `heightmap_file1` | `land_export` (main heightmap) |
| `leveldesign/landscape/<eco>/noise_<continent>.tga` | `heightmap_file2` | `land_export` (detail/noise heightmap) |
| `leveldesign/landscape/<eco>/colormap_<continent>.tga` | `colormap_file` | `land_export` (optional per-vertex tile color tint, `CExport::addColorMap()`, `export.cpp:1170` -- missed in the original field survey, found 2026-09-12; `land_export` runs fine without it) |
| `leveldesign/ecosystems/<eco>/<eco>.bank` (moved here from `pipeline/landscape/` by Nuno 2026-09-12 -- note the one folder-name exception: `primes_racines`'s `.bank` lives under a folder literally named `primes_roots`, every other ecosystem's folder matches its own identifier) | `tile_bank_file` | `land_export`, `zone_lighter` (tile-noise displacement lookup, `docs/zone_tools.md` §2) |
| `leveldesign/DFN` | `level_design_dfn_directory` / `dfn_dir` | `land_export`; optionally `zone_dependencies` |
| `leveldesign/world` | `level_design_world_directory` / `continents_dir` | `land_export`; optionally `zone_dependencies` |
