# Changelog

## 2026-09-08 — 🙈 Hide Atyscape from ryztart, Forgery 4.0.1

`landscape_editor.py`'s `APP_INFO` renamed to `_APP_INFO_HIDDEN` so
`ryzom_forgery.list_apps()`/`launch_app()` (`ryzom_forgery/__init__.py`,
used by `ryztart`'s Forgery launcher module) no longer discover it, while
it's still being built (`project-todos/forgery/landscape_editor.md`).
`dev.sh` (direct launch by file path) is unaffected. Rename back to
`APP_INFO` once ready to ship.

## 2026-09-08 — 🐛 Fix viewport toggle alignment and zoom z-fighting, Forgery 3.8.4

`project-todos/forgery/landscape_editor.md` -- an approximate water-plane
overlay (flat blue quad per zone at Z=0, meant as a placeholder for real
`.ig`/`CWaterShape` water detection) was tried and abandoned same-day: it
read as "water" over any terrain merely dipping below sea level, not just
real lakes, covering most of a real continent in blue. Removed entirely
(`zone_geometry.py`'s `build_water_plane_geom()`/`_WATER_PLANE_COLOR`,
`landscape_editor.py`'s water state/toggle) -- the corresponding chantier
step was dropped and the following steps renumbered back down.

Two real fixes came out of that detour and were kept:

- The "Show zone grid" toggle moved from a panel checkbox to a floating
  icon-button bar bottom-left of the 3D viewport (`_draw_viewport_toggles()`
  in `landscape_editor.py`), reusing `object_editor.py`'s own `_icon_button()`
  (`ui_helpers.py`, already designed with zero dependency on
  `object_editor.py` itself for exactly this kind of cross-app reuse). First
  attempt used `imgui.get_frame_height()` for vertical positioning and came
  out misaligned; matched byte-for-byte against `object_editor.py`'s own
  `_draw_viewport_toggles()` instead -- same `_viewport_toggle_size`
  self-measurement trick (true bar size only known after the window draws,
  so each frame positions off the previous frame's captured size) and same
  `large_icon_font`.
- `LandscapeEditorApp.__init__` now overrides `ForgeryApp`'s shared
  `camLens.set_near_far(0.02, 20000.0)` (a 1,000,000:1 ratio meant for
  Patina's own close-up shape inspection) to `(1.0, 20000.0)`: flat Z=0
  overlay geometry (the zone grid, and the water plane while it existed)
  was z-fighting against terrain crossing Z=0, rendering differently
  depending on camera distance -- the near/far ratio was destroying depth
  buffer precision at range. Confirmed fixed by Nuno.

Also bumped `OrbitCamera.max_distance`'s multiplier from 3x to 6x (12000.0,
still under the 20000.0 far clip) -- 3x still weren't enough to zoom out
comfortably on a whole continent.

## 2026-09-07 — ✨ Add landscape_editor app scaffolding, Forgery 3.5.0

New Forgery tool app, `ryzom_forgery/apps/landscape_editor.py`
(`project-todos/forgery/landscape_editor.md` step 1): scaffolding only,
no terrain rendering yet -- window, explorer filtered on `*.land`, free
`OrbitCamera` reused as-is from `camera.py`. Auto-discovered by ryztart via
the existing `APP_INFO`/`list_apps()` mechanism, no separate menu file to
edit.

`OrbitCamera` (and the `ObjectManipulator` it coexists with) reads
`app.forced_drag_mode`/`app.target_mode` every frame -- both undocumented
requirements on any `ForgeryApp` subclass wanting to reuse it (only
discovered by testing: `object_editor.py` happens to set both for its own
navcube-driven drag-mode overrides, but nothing enforces or documents that a
camera-only consumer must too). Set to `None` (no override) in
`LandscapeEditorApp.__init__`.

Also reordered `project-todos/forgery/landscape_editor.md`'s own steps 1-2:
the plan originally had "finish continent-selector sub-chantier" before
"create the app", but continent-selector's own step 3 needs the app's panel
to already exist to add its dropdown to -- a circular dependency, fixed by
swapping the two steps.

## 2026-09-07 — ✨ Add continent selector + real region bounds, Forgery 3.6.0

`project-todos/forgery/landscape_editor__continent-selector.md` closed.
Added `ryzom_forgery/continent_selector.py`
(`load_continent_locations()`/`resolve_continent_bounds()`) and wired a
continent-picker dropdown + resolved bounds display into
`landscape_editor.py`'s panel, reading `world.packed_sheets`/
`continent.packed_sheets`/`sheet_id.bin` from the same `live_data_path`
global setting Patina (`object_editor.py`) already exposes.

`resolve_continent_bounds()` reproduces `CContinent::getCorners()`:
`ContinentParameters.zone_min`/`zone_max` are zone names (not raw
coordinates), decoded via `pynel.ryzom_packed_sheets.zone_name_to_world_pos()`,
each axis reordered min/max (a zone name's letter/digit order isn't
guaranteed ascending), and +160 added to the max corner (a zone name gives
its origin corner, not its extent).

Two real-data surprises found only by testing against a real install (both
corrected in the sub-chantier's own notes, the initial plan had guessed
wrong on both):
- `ContLoc.continent_name` is the bare sheet stem with **no extension**
  (e.g. `"bagne"`), not `"bagne.continent"` as the original investigation
  assumed -- the `.continent` suffix has to be appended before looking the
  name up in `sheet_id.bin`.
- `sheet_id.bin` is **not** a loose file in the Ryzom Live data folder,
  unlike `creature.packed_sheets`/`world.packed_sheets`/
  `continent.packed_sheets` -- it only exists packed inside
  `leveldesign.bnp`. `continent_selector.py` tries the loose file first,
  then falls back to reading it out of `leveldesign.bnp` via
  `pynel.ryzom_bnp.BnpReader`.

Validated against a real Ryzom Live install: all 28 real continents resolve
bounds without error.

## 2026-09-08 — ✨ Add disk cache of tessellated zones, whole-continent loading, Forgery 3.8.2

`project-todos/forgery/landscape_editor__zone_disk_cache.md` closed
(`landscape_editor.md` step 6). Replaces region-around-the-camera streaming
(never implemented beyond a first draft) with loading an entire selected
continent at once, backed by a per-zone disk cache of tessellated positions.

**Why**: measured (194 zones, 400-unit radius around the camera) that
listing (`.bnp` header scan, cached once) and raw bnp reads are negligible,
but `parse_zone()` (11.221s, ~57.8 ms/zone) and tessellation (14.506s,
~74.8 ms/zone, a pure-Python loop in `zone_geometry._eval_bezier_patch()`)
dominate and are linear in zone count regardless of streaming strategy --
so the fix is to pay that cost once per zone and cache it to disk, not to
change how much of the world is requested at once.

**Cache design** (`ryzom_forgery/zone_cache.py`): stores only the
tessellated positions per patch (`n_s`/`n_t` + a flat float tuple) and the
zone's bounding box -- never color, triangle indices, or future normals/UV,
which stay derived at load time from positions alone (cheap: color is a
lookup on Z, indices are arithmetic on `n_s`/`n_t`). This means no future
rendering change (lighting, texturing) can ever invalidate the cache. One
pickle file per zone under `config_dir()/"zone_cache"`, written atomically
(temp file + rename), stamped with the source file's mtime/size (the whole
`.bnp` if the zone is packed, since an archive entry can't be dated on its
own) for staleness detection.

`zone_geometry.compute_zone_patch_positions()` isolates the expensive Bezier
evaluation, shared by the disk-cache writer (`region_loader.
load_zone_cache_data()`, cache-first: reads the disk cache if fresh, else
parses+tessellates+writes) and the direct-from-zone path
(`zone_to_cache_data()`, used by `build_zone_tessellated_geom()` and by
`on_selection_changed()`'s single-zone Explorer selection) -- one place
computes Bezier positions, so cached and freshly-computed zones can never
drift apart. `landscape_editor.py`'s `self.zones` is now always
`ZoneCacheData` regardless of where it came from.

Continent selection (`_select_continent()`) now triggers loading the whole
continent automatically; a "Build cache for this continent" button does the
same thing manually. A progress bar (zones processed/total) covers what can
be several hundred zones, computed on a background thread (pure data, no
Panda3D calls -- `load_zone_cache_data()` never touches it).

**Second bottleneck found after the cache worked** (Nuno, real testing):
with the cache warm, 53 zones loaded in 0.195s, but rebuilding their
`GeomNode` (`_set_loaded_zones`) still took 3.365s (~63.5 ms/zone) -- barely
better than the pre-cache 74.8 ms/zone. Removing bnp read/parsing/Bezier
eval only saved ~11 ms/zone: the actual bottleneck was the sheer number of
individual `GeomVertexWriter.add_data3()`/`add_data4()`/
`GeomTriangles.add_vertices()` Python/C++ binding calls, not the color/index
math itself. Fixed by having `build_zone_geom_from_cache()` build a `numpy`
structured buffer (`[('vertex','<f4',3),('color','u1',4)]`, matching
`GeomVertexFormat.get_v3c4()`'s real layout, asserted at runtime) and write
it in one `set_data()` call, with elevation color computed vectorized and
truncated (not rounded) to uint8 -- verified bit-for-bit identical to the
old per-vertex writer output. Triangle indices built the same way (flat
`numpy` array, `uint16`/`uint32` chosen by vertex count) instead of one
`add_vertices()` call per triangle. ~5.4x faster on synthetic data matching
real zone sizes; `build_zone_tessellated_geom()` (the direct-from-zone path)
delegates to the same function, so it benefits too with no separate code.

Validated by Nuno on a real Ryzom Live install: a 282-zone continent loads
in ~2.2s total once its cache is warm ("c'est bcp plus rapide"), down from
what would have been well over 20s at the pre-cache per-zone cost.

## 2026-09-08 — ✨ Fix camera zoom range, anchor elevation gradient at sea level, add zone grid overlay, Forgery 3.8.3

`landscape_editor.md` step 7 closed. Three fixes found by Nuno testing a
whole real continent for the first time (previous step's work).

**Camera zoom**: `OrbitCamera.max_distance` (2000.0, the shared default used
by every Forgery app) didn't reach far enough to see a whole continent.
Multiplied by 3 in `LandscapeEditorApp.__init__` only -- doesn't touch
`camera.py`'s shared default, so Patina/object_editor is unaffected. Also
fixes auto-framing after a continent load, which was silently clamped to
the same `max_distance` by `OrbitCamera.frame()`.

**Elevation gradient anchored at sea level**: the first version (single
min-max-normalized brown->green gradient over the whole loaded set's Z
range) washed out badly on a real continent -- a handful of very deep zones
(underwater/caves) dragged the range's minimum way down, so all
normal-depth terrain ended up reading as nearly the same shade near the
"high" end. Nuno's fix: anchor the gradient at world Z=0 (where the water
plane sits) instead of at the loaded set's own min/max. `zone_geometry.
_elevation_colors_uint8()` now blends two independent segments: dark red at
the lowest loaded point -> brown at Z=0 -> green at the highest loaded
point (`_DEEP_COLOR`/`_SEA_LEVEL_COLOR`/`_PEAK_COLOR`). Depth of the deepest
zone no longer affects how normal-depth terrain reads.

**Zone grid overlay**: `zone_geometry.build_zone_grid_geom()` draws a
`LineSegs` wireframe on the 160-unit (`ZONE_CELL_SIZE`) zone-cell
boundaries covering whatever's loaded, snapped outward to the nearest real
cell boundary so every zone gets its full outline. `ZONE_CELL_SIZE` moved
from `region_loader.py` to `zone_geometry.py` (the lower-level, dependency-free
module) to avoid a circular import -- `region_loader.py` now imports it from
there. Rebuilt in `_set_loaded_zones()` alongside the terrain itself, a
"Show zone grid" checkbox in the panel (on by default). Drawn with
`set_depth_test(False)`/`set_depth_write(False)` and forced into the
`"fixed"` render bin so it always shows on top of the terrain regardless of
relief -- it's a flat reference overlay at Z=0, not real geometry meant to
be occluded.

Validated by Nuno on a real continent.
