# pynel documentation index

## Landscape & collision formats

- `zone_format.md` — `.zone`/`.zonew`/`.zonel` binary terrain format, shared across the build pipeline's raw/welded/lit stages.
- `land_format.md` — `.land` landscape composition format: a grid of `.zone` bricks placed with position/rotation/flip.
- `pacs_format.md` — PACS collision/navigation formats: `.rbank`/`.lr`/`.gr` walkable-surface retrievers and `.pacs_prim` movement obstacle primitives.

## Character & data formats

- `shape_format.md` — `.shape` 3D mesh format: meshes, skeletons, and special-purpose shapes (water, flares, particle systems).
- `config_file.md` — `.cfg` generic NeL config format (`CConfigFile`), used engine-wide.
- `georges_form.md` — Georges FORM XML tree format, used by `.continent` and `ryzom.world`.
- `georges_sheets.md` — Georges sheet inheritance (`PARENT` chains) for gameplay sheets (`.creature`, `.item`, `.sbrick`, ...).
- `packed_sheets.md` — `.packed_sheets` binary format for creature/item/sitem/animset/world/continent sheet data.
- `hls_texture_bank.md` — `.hlsbank`/`.hlsinfo` HLS-colorisable character texture bank format.

## Landscape pipeline tools

- `zone_tools.md` — `zone_welder`/`zone_lighter` native tool subprocess reference: CLI arguments, config format, invocation.
- `pipeline_directories.md` — `pipeline/` directories actually read or written by the land pipeline's orchestrated tools.

## World, region & instance data

- `region_export.md` — Reads/writes the real lore region and place polygons (`region_<continent>.primitive`) as `.lua`/`.json`.
- `ig_client_loading.md` — How the client loads and generates `.ig` instance-group files, for both "land" and "other" kinds.

## Infrastructure

- `repository_paths.md` — Shared local-checkout locations for the sibling Ryzom repositories (`ryzom-core`, `ryzom-data`, `ryzom-private-data`, `ryzom-docker`).

## Roadmap

- `roadmap.md` — Landscape/collision tooling goal and current state of that effort.
