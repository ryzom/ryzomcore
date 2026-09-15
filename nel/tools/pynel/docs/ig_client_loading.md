# `.ig` client loading and generation

How the real Ryzom client loads `.ig` (instance group) files at runtime, and where each kind is generated from. Read this before touching anything `.ig`-related in Forgery's `landscape_editor` app -- researched 2026-09-15 by reading `ryzom-core` directly (client source + `build_gamedata` pipeline scripts), not guessed.

## Two kinds of `.ig`, two loading mechanisms

### "Land" `.ig` -- pure filename convention, no config

One `.ig` per terrain zone (e.g. `39_CF.ig`), holding the zone's procedurally-placed flora/decoration. Loaded automatically whenever that zone streams in:

- `ryzom/client/src/continent.cpp`: `Landscape->getAllZoneLoaded()` / `refreshAllZonesAround()` gives the terrain engine's list of currently-streamed zone names.
- `nel/src/3d/landscapeig_manager.cpp` (`CLandscapeIGManager::loadArrayZoneIG()`): for each zone name, loads `<ZONE_NAME>.ig` (uppercased). No lookup table, no indirection -- the `.ig` filename IS the zone name.

Consequence: a "land" `.ig`'s filename is derived purely from its zone -- safe to reason about by zone name alone, but never rename it independently of the zone.

### "Other" `.ig` (villages, sky/canopy, city instances) -- name declared in a Georges sheet

Unlike "land", the filename is **data, declared in a sheet**, never deduced from position or convention:

- **Villages**: `ryzom/client/src/village.cpp` (`CVillage::enumIGs()`) reads the `.village` Georges sheet's `IGs[]` array (`client_sheets/village_sheet.cpp`, each entry a `{IgName, ParentName}` pair) -- that's where names like `tr_villagea.ig` come from. Loaded/unloaded by distance from the player (`streamable_ig.cpp`, `CStreamableIG`, radii `forceLoad`/`load`/`unload`), independent of terrain zone streaming.
- **Sky/canopy**: declared directly in the continent's own `.continent` Georges sheet (`client_sheets/continent_sheet.cpp`), fields `SkyIg` and `<Season>CanopyIG` (one name per season).

Consequence: an "other" `.ig`'s filename is an external identity (referenced by a sheet elsewhere) -- it can never be renamed freely, only its *content* edited.

## Generation (build_gamedata pipeline, `ryzom-core/nel/tools/build_gamedata/processes/`)

- **Land**: Ligo (procedural ecosystem placement, `ligo` process) optionally merged with World Editor `.primitive` flora placements via `prim_export`/`ig_add`, then `ig_elevation`, then lit per-zone by `zone_ig_lighter`.
- **Other**: hand-built **3ds Max scenes** (`.max` files under `IgOtherSourceDirectories`, e.g. `stuff/<continent>/decors/constructions`), exported to static `.ig` via a MAXScript (`processes/ig/1_export.py` calling `ig_export.ms`) -- requires 3ds Max itself (`MaxAvailable`, Windows/Autodesk), impossible to run from this Linux pipeline. Then elevated+lit globally (not per-zone, no `.depend`) via `ig_elevation`/`ig_lighter` (`processes/ig_light/`).

**Decision (Nuno, 2026-09-15)**: Forgery never reconstructs either kind from these raw sources (World Editor `.primitive` found stale on nexus; 3ds Max simply unavailable on Linux). `live_data` is the one reliable, already-generated-and-shipped reference for both kinds -- Forgery copies it as a base and edits directly on top (`project-todos/forgery/landscape_editor__ig_editing.md`), relying only on the already-proven-idempotent lighting tools (`zone_ig_lighter`/`ig_lighter`) to relight after edits, never on regenerating from source.
