# Changelog

## 2026-09-11 — ✨ Split Atyscape into Edit/View mode mixins, 3-state wireframe cycle, Forgery 4.7.0

`landscape_editor__land_preview.md` (steps 5-7, closing `landscape_editor.md`
step 9) and `wireframe_cycle_states.md` closed.

**Edit/View mode mixin split**: `landscape_editor.py`'s Edition-only logic
(continent list from `ryzom.world`, the `.land`+brick fallback resolution,
the cursor status's brick name, the "Generate missing .zonew" button --
meaningless in Visualisation since `ryzom_data_path` is always `None`
there) moved to a new `EditModeMixin` (`landscape_editor_edit_mode.py`).
Visualisation's own (much thinner) logic moved to `ViewModeMixin`
(`landscape_editor_view_mode.py`). Shared render-mode constants/helpers
(`_RENDER_MODES`, `_resolve_zone_for_mode()`, `_detect_app_mode()`...)
extracted to `landscape_editor_modes.py` so both mixins can import them
without ever importing back from `landscape_editor.py` (circular import) --
same reasoning as Patina's own `object_editor_mixins/ui_helpers.py`. The
shared rendering pipeline itself (`_apply_render_mode()`/`_run_load_refs()`/
`_set_loaded_zones()`) stays in the base class, never duplicated. Validated
by Nuno: no regression in either mode on a real continent.

**Wireframe cycling button, 3 states (Patina + Atyscape)**: the wireframe
toggle (On/Off, drawing on top of the shaded/textured render) became a
3-state cycle on the same button/icon -- "Off" (no override), "Overlay"
(the previous behavior, `set_render_mode_filled_wireframe()`), and "Pure"
(new: `set_render_mode_wireframe()` + `set_texture_off()`, true wireframe
with no texture or fill at all). Left-click cycles, right-click opens a
popup to jump straight to any state.

**Also fixed along the way**: `.gitignore`'s `nel_tools*`/`ryzom_tools*`
patterns were unanchored, silently excluding
`ryzom_forgery/ryzom_tools_setup_dialog.py` from every commit since it was
written -- anchored to the repo root and the file committed (see its own
entry below, 4.6.1).

## 2026-09-11 — Reverted: Shading/Constant Shading/Unshaded cycling button on Atyscape's terrain

`landscape_editor__shading_modes.md` abandoned, never committed to code
history. Ported from Patina's own shading mode button (same 3 states,
applied to `self._zone_root` instead of `model_root`) but reverted after
Nuno tested it in Atyscape: only plain "Shading" (the normal, already
always-on rendering) turned out useful for terrain -- "Constant Shading"
(sun light off, ambient only) and "Unshaded" (flat color, no texture) added
no value there, unlike on a single inspected object in Patina. State,
methods (`_apply_shading_mode`/`_transition_shading_mode`/
`_cycle_shading_mode`/`_set_shading_mode`) and the viewport button were all
removed again before ever reaching a real commit.

## 2026-09-11 — 🐛 Fix .gitignore swallowing ryzom_tools_setup_dialog.py, Forgery 4.6.1

`.gitignore`'s `nel_tools*`/`ryzom_tools*` patterns (meant for old repo-root
tool directories, none of which exist anymore) were unanchored, so they
matched at any depth -- silently excluding
`ryzom_forgery/ryzom_tools_setup_dialog.py` from every commit since it was
first written. The file existed and worked in every worktree that happened
to have it locally, but never made it into git or any built package, hence
`No module named 'ryzom_forgery.ryzom_tools_setup_dialog'` on a real
install (`ryzom_paths_section.py` imports it unconditionally). Fixed by
anchoring both patterns to the repo root (`/nel_tools*`/`/ryzom_tools*`) and
committing the rescued file.

## 2026-09-11 — ✨ Génération .zonew, cache continent .bam, 2D/3D, wireframe/shading Patina, Forgery 4.6.0

`landscape_editor__zone_render_modes.md` (validation finale, étape 8 du
`landscape_editor.md` clôturée), `geomnode_continent_cache.md`,
`landscape_editor__2d_3d_toggle.md`, `landscape_editor__transparency_wireframe.md`,
`object_editor__wireframe_overlay.md` and `object_editor__shading_modes.md`
closed.

**WELD zone generation button (Atyscape)**: "Generate N missing .zonew" on
`[WELD]` runs the native `zone_welder` on every zone missing one, one at a
time in a background thread, writing straight to
`<ryzom-data>/pipeline/export/continents/<continent>/zone_weld/`. Live,
per-zone display -- each zone flips from the gray/pink fallback gradient to
the real elevation gradient the moment its own `.zonew` lands, without
waiting for the whole batch. `.land` cells with no real `.zone` at all now
also show up in `[WELD]` (same fallback gradient) and count toward "N
missing", but are never attempted -- each gets an explicit per-cell error
("finish land_composition/land_export first") instead, without blocking the
zones that actually can be welded.

**Land-fallback brick performance**: `build_land_piece_cache_data()` was
redoing a full parse + Bezier tessellation for every `.land` fallback brick
on every continent load. Split into two disk caches (`zone_cache.py`, same
mechanism as the existing per-zone cache): the brick's raw untransformed
geometry (shared across every placement/rotation of the same brick) and the
final transformed piece per placement. `land_geometry.transform_zone_cache_data()`
(new, extracted from the old `build_land_piece_cache_data()`, now a thin
wrapper around it) applies position/rotation/flip to an already-tessellated
`ZoneCacheData` without ever re-evaluating Bezier. Measured on nexus (17
fallback cells): 2.135s -> 0.134s (~16x).

**Continent-wide `.bam` geometry cache**: even with every zone's tessellated
positions cached, building the actual Panda3D `GeomNode`s (color, indices,
`attach_new_node`) still cost 1.349s for 151 zones (~8.9ms/zone), entirely
on the main thread. New module `ryzom_forgery/continent_geom_cache.py`
serializes the whole built `NodePath` via Panda3D's native `.bam` format
(`write_bam_file()`/`load_model(..., noCache=True)`) instead -- since a zone
only ever belongs to one continent, a `.bam` + manifest bundle per
`(continent, render mode)` can be reloaded wholesale as long as nothing in
it is stale (per-zone extension + source mtime/size, plus the global
elevation range the color gradient was baked against). A cache hit skips
`GeomNode` construction entirely; a miss falls back to today's full rebuild
(never worse) and saves a fresh bundle for next time. Confirmed by Nuno on a
repeated continent reload.

**2D/3D viewport toggle (Atyscape)**: `[2D]` removed from the render-mode
bar (`[POLY]`/`[WELD]`/`[LIGHT]` only now, `[2D]` and `[POLY]` resolved
identical geometry anyway) and replaced by a dedicated camera-only toggle
next to the grid icon. `OrbitCamera.lock_rotation` (`camera.py`, new flag,
shared with Patina but inert until an app touches it) disables left-drag
orbit while pan/zoom keep working. 2D is now the default view at launch;
leaving it for 3D restores whichever 3D orientation was last left (or a
default angled view the first time), animated via the new
`OrbitCamera.animate_to_orientation()` (which `snap_to_axis()` now also
delegates to) rather than jumping instantly.

**Terrain transparency/wireframe (Atyscape)**: two independent, combinable
toggle buttons next to the 2D/3D one, same mechanism as Patina's own
(`self._zone_root` instead of `model_root`).

**Wireframe overlay, not replacement (Patina + Atyscape)**: both apps'
wireframe toggle switched from `set_render_mode_wireframe()` to
`set_render_mode_filled_wireframe((0, 0, 0, 1), 1)` -- the wireframe now
draws on top of the normal shaded/textured render instead of replacing it.

**Shading mode cycling button (Patina)**: new button next to Patina's
wireframe toggle, cycling `model_root` through Shading (normal, untouched)
/ Constant Shading (directional sun light off, ambient-only, still
textured) / Unshaded (all lighting and textures off, flat user-chosen
color instead). Left-click cycles; right-click opens a popup to jump
straight to any state -- a general convention for every future
Forgery cycling button, not just this one. Constant Shading also boosts the
scene's ambient light intensity to its max while active, restoring the
previous value on leaving it (never in Shading or Unshaded themselves).

All confirmed working by Nuno.

## 2026-09-11 — ✨ Add wireframe toggle to Patina, Forgery 4.5.0

`object_editor__wireframe.md` closed.

Patina already had a 50% object transparency toggle
(`_object_transparent`/`_toggle_object_transparency`/
`_apply_object_transparency`, `viewport_transform.py`, applied to
`model_root`, re-applied after every `_rebuild_geometry()` since
`model_root` is destroyed/recreated on every shape load/replace). Added the
same mechanism for a wireframe toggle (`_object_wireframe`/
`_toggle_object_wireframe`/`_apply_object_wireframe`, `set_render_mode_
wireframe()`/`clear_render_mode()`) -- independent of transparency, both
combinable, both survive a shape reload/replace. New button next to the
transparency one in the viewport toggle bar (`ICON_FA_DRAW_POLYGON`).

Validated by Nuno.

## 2026-09-11 — ✨ Add .land fallback + multi-cell piece support, Release/Dev toggle, stderr errors, cursor status, Forgery 4.4.0

`landscape_editor__land_preview.md` steps 3-4 and
`landscape_editor__cursor_zone_status.md` closed.

**`.land`+brick fallback for `[POLY]`/`[2D]` in Edition mode**: when a `.land`
cell has no exported `.zone` yet (a brick just placed locally, never run
through `land_export`), `land_geometry.py` computes its geometry directly
from the `.land` + the referenced brick file, rendered with the existing
violet->pink fallback gradient rather than counted as simply missing.
Positioning went through two wrong iterations before landing on the correct
one, each found by comparing computed positions against every real exported
zone of `ryzom-data/pipeline/export/continents/bagne/` (kept locally for
exactly this kind of check): rotating/mirroring about the brick's local
(0,0) corner (a literal reading of `CExport::transformZone()`'s `CMatrix`
construction) was off by up to a full cell; centering the rotation on a
fixed 160x160 square fixed single-cell bricks but still mispositioned
multi-cell "large pieces" (some real bricks are 320x160 or 320x320, not
160x160). The final fix derives each piece's real size directly from its
own loaded bounding box (no ZoneBank file needed) and ports
`CExport::treatPattern()`'s deltaX/deltaY resolution to recover a multi-cell
piece's shared grid origin from any one of its referencing cells --
`landscape_editor.py` now also deduplicates so a piece spanning several
`.land` cells (each storing its own "position in the piece",
`ZoneUnit.pos_x`/`pos_y`, a different field than the grid position despite
the name) renders exactly once instead of once per cell it overlaps
(previously visible as duplicated, overlapping geometry, e.g. nexus's
`49_CL`/`49_CM`). Verified against `bagne.land`'s two real multi-cell
pieces: computed and real bounding-box centers now agree within ~2 units.
The fallback's own violet->pink gradient is now computed per zone (its own
Z range) instead of over the whole loaded continent's range -- these zones
never join seamlessly with neighbors anyway, and a global range made most
of them read as a near-uniform color. Its low end was also darkened.

**Release/Dev manual toggle**: the Visualisation/Edition mode (Forgery
4.3.1) is no longer purely automatic -- a button next to the mode badge
("Release"/"Dev", the UI labels; `_MODE_VISUALISATION`/`_MODE_EDITION` stay
the internal identifiers) lets the user switch manually, persisted in
`Settings.landscape_editor_mode`, disabled while `ryzom-data` isn't
configured (Dev has no meaning without it). Two bugs found while testing
this: switching mode didn't reload the currently loaded zones (they kept
showing whatever was loaded under the previous mode) -- fixed by
re-triggering `_select_continent()`/`_load_continent()` on a mode change,
matching the continent by `selection_name` rather than `continent_name`
(the latter is packed_sheets's raw identifier in Release, e.g.
`"lecarrefour"`, but `ryzom.world`'s own reliable `struct_name` in Edition,
e.g. `"nexus"`, for the very same continent). And Release mode was still
silently reading `ryzom-data`'s pipeline export whenever one existed for the
selected continent (the older, still-present per-continent
`has_pipeline_export()` mechanism from `zone_render_modes.md` never checked
which mode was active) -- fixed by forcing `ryzom_data_path` to `None` in
`_load_continent()` outside Edition mode.

**Every Forgery UI error also on stderr**: `error_log.report_error()`
(`print(..., file=sys.stderr)`) is now called alongside every existing
`self._x_error = ...`/progress-dict-error assignment in `landscape_editor.py`
-- these were previously only ever visible in whichever tab/panel happened
to be showing the error string, easy to miss (`error_stderr_logging.md`,
Atyscape done first; the rest of Forgery's apps/dialogs are separate,
not-yet-done steps of that same chantier).

**Zone name + position under the cursor**: the shared bottom status bar
(`SysInfoBar`, next to the FPS counter, new `cursor_info` field distinct
from the existing Explorer-selection `status` field) now shows the 160x160
zone under the mouse cursor and its world position, e.g.
`27_AG (2541, -4848)` -- `mouse_picking.mouse_ground_position()` (new,
zero-dependency module) unprojects the cursor via `camLens.extrude()` and
intersects the Z=0 plane (no real terrain raycast, no altitude), converted
to a zone name via pynel's new `world_pos_to_zone_name()`. In Edition mode
the line also shows the brick name the `.land` itself assigns to that cell
(`ZoneUnit.zone_name`, read once per continent selection and cached) --
always that name, regardless of which pipeline stage actually renders
there; two earlier attempts (the real loaded file's own name, then that
name with its extension) were rejected as either redundant with the
already-shown zone name or simply not the requested information.

## 2026-09-10 — ✨ Filter continent combo by .land in Edition mode, Forgery 4.3.2

`landscape_editor__land_preview.md` step 2 done. In Edition mode, the
continent combo now reads `ryzom.world` directly from `ryzom-data`
(`continent_selector.load_continent_locations_from_world_file()`, new,
never `live_data_path`/`world.packed_sheets`) and only lists continents that
actually have a `.land` under `<ryzom-data>/leveldesign/landscape/`
(`land_loader.find_land_files()`, new, indexed by filename stem -- confirmed
a real `.land`'s stem always matches its `PacsRBank`, never a folder name).
Uses `WorldContinentEntry.struct_name` as the continent identifier, not
`.continent_name` (documented by pynel as unreliable). Visualisation mode
unchanged. `_cont_locs` cache invalidated on every mode change.

## 2026-09-10 — ✨ Add Visualisation/Edition mode detection + badge, Forgery 4.3.1

`landscape_editor__land_preview.md` step 1 done -- first step of a chantier
redesigning the previous per-continent/per-button live_data-vs-ryzom-data
bascule (`landscape_editor__zone_render_modes.md` step 6) into a single
global app mode, later made user-toggleable (see 4.4.0 above). `_detect_app_mode()`
switches between "visualisation" and "edition" based on whether
`pynel.repository_paths.is_valid("ryzom-data")`, recomputed every
`draw_panel()` frame; a colored badge shows the active mode in the panel.

## 2026-09-09 — ✨ Add pipeline data download/install, Forgery 4.3.0

`landscape_editor__zone_render_modes__pipeline_data_installer.md` closed.

**`pipeline_data_installer.py`**: downloads and installs the `.zip`
archives Nuno publishes for the `build_gamedata` pipeline data (too large
to version in `ryzom-data`) into `<ryzom-data>/pipeline/`. Three
categories, each with a base URL and target subdirectory: `landscape`
(`download.ryzom.com/tools/landscape/<ecosystem>.zip` ->
`pipeline/landscape/<ecosystem>/`), `pipeline_ecosystems`
(`.../tools/pipeline/<ecosystem>.zip` -> `pipeline/export/ecosystems/
<ecosystem>/`), `pipeline_continents` (same base, `<continent>.zip` ->
`pipeline/export/continents/<continent>/`). `is_installed()`/
`download_and_install()` -- streamed download via stdlib `urllib.request`,
extraction via stdlib `zipfile` (reads DEFLATE and LZMA transparently, so a
`.zip` written with `7z a -tzip -mm=LZMA` for a much better compression
ratio than plain `zip` needs no code change to read). No new dependency.

**`continent_ecosystem.py`**: `get_ecosystem_for_continent()` maps a
continent's real runtime name to its ecosystem, primarily from
`<ryzom-data>/leveldesign/workspace/continents/<name>/directories.py`'s
`EcosystemName` (regex-extracted) -- more reliable than each
`.continent`'s own `Ecosystem` field, which can be entirely absent even
when the ecosystem is real (found for every `r2_*` Ring continent) and can
spell the name differently (`primes_roots` vs the `primes_racines`
convention the download categories and `directories.py` actually use).
Falls back to the `.continent` file itself only for continents absent from
`workspace/` (just `testroom`).

**`pipeline_data_install_dialog.py`** (`PipelineDataInstallDialog`):
proposes downloading whatever's missing (continent + its ecosystem's
export + its ecosystem's raw landscape zones -- `landscape` turned out to
be a hard requirement, not optional, since it's the only starting material
to compose a continent from its `.land` before any per-continent `.zone`
has been generated) as one popup, then runs the downloads sequentially in
a background thread with a shared progress bar. A declined set is
remembered in memory for the session (never persisted).

Wired into `landscape_editor.py`'s continent combo (`_load_continent`).
Found and fixed two real bugs during Nuno's testing: (1) the popup never
appeared at all -- `_load_continent()` runs from inside the continent
combo's own `begin_combo()`/`end_combo()` block, and calling
`imgui.open_popup()` for a different popup while still inside another
popup's Begin/End silently loses the open request when the combo closes;
fixed by deferring the open to the next `draw_panel()`, outside any combo,
via a pending-list field. (2) the popup rendered squashed to a tiny width
-- it has no other wide content to anchor `always_auto_resize`'s width
calculation, so plain `text_wrapped()` collapsed to a minimal column;
fixed with an explicit `push_text_wrap_pos()`.

## 2026-09-09 — ✨ Add [2D][POLY][WELD][LIGHT] render modes + pipeline-export zone source, Forgery 4.2.0

`landscape_editor__zone_render_modes.md` step 6 closed.

**Render mode bar** (`landscape_editor.py`, `_draw_render_mode_bar()`):
`self.render_mode` (`"2D"/"POLY"/"WELD"/"LIGHT"`, default `"POLY"`) drives,
for every zone of the loaded continent, which real extension to actually
display (`_resolve_zone_for_mode()`). `[WELD]` accepts either `.zonew` OR
`.zonel` as "really welded" -- a shipped `.zonel` necessarily went through
welding already, even when the intermediate `.zonew` was never kept (a real
`live_data` install only ships the final pipeline stage). `[LIGHT]` only
accepts `.zonel`. Either mode falls back to an earlier stage when its own
extension is missing, rendering that zone with a distinct fallback gradient
(`zone_geometry.build_zone_geom_from_cache(..., fallback=True)`) instead of
the real elevation-colored one -- purple at the lowest point loaded, pink at
the highest. A first grayscale attempt (near-black -> light gray) turned
out indistinguishable from the viewport's own gray background: a fully
fallback-colored zone at low elevation looked exactly like a hole in the
terrain rather than a grayed-out zone. `[2D]` only triggers the existing
top-view camera snap. The Explorer single-file-selection path
(`on_selection_changed()`) is deliberately NOT routed through this logic --
it's never used in practice, the real workflow always loads a whole
continent via the combo.

**Zone source per continent -- `live_data_path` vs a real pipeline export**
(`region_loader.py`): `find_zones_in_region()`/`get_zone_index()`/
`get_zone_extensions_index()` now take an optional `continent_name` +
`ryzom_data_path`. If `<ryzom_data_path>/pipeline/export/continents/<continent_name>/`
exists (a real `build_gamedata` pipeline export, scanning only its `zone`/
`zone_weld`/`zone_lighted` subdirectories), it entirely replaces
`live_data_path` as that continent's zone source -- never merged with it,
and `live_data_path` doesn't even need to be configured for such a
continent. This source is intentionally never cached (unlike
`live_data_path`), since real multi-stage test data can change between
calls (e.g. deleting a `.zonew` mid-session to exercise a WELD fallback).
`ryzom_data_path` is resolved via the already-existing
`pynel.repository_paths.get("ryzom-data")` (Settings > Ryzom Paths) -- a
first version introduced a second, redundant `Settings.ryzom_data_path`
field, caught and removed.

**Gotcha found while wiring this up**: a continent's displayed combo label
(`ContLoc.selection_name`, e.g. `"nexus"`) can differ from its internal sheet
stem (`ContLoc.continent_name`, e.g. `"lecarrefour"`, used to resolve
bounds) -- a real pipeline export directory is named after
`selection_name`, not `continent_name`. `landscape_editor.py` now tracks
both (`self.selected_continent_name` for bounds, `self._selected_continent_pipeline_name`
for the pipeline export lookup) -- conflating them silently made the
pipeline-export source never match and fall through to `live_data_path`
without any error.

Also confirmed (comparing 27 real `.land` files byte-for-byte):
`ryzom-data/leveldesign/landscape/` is the current/maintained source
(2025-dated, all XML); `ryzom-data/graphics/landscape/ligo/` holds stale
data (2020-dated, at least one file in an obsolete binary format, one file
missing entirely) -- never read the latter for active leveldesign.

Validated by Nuno on a real continent (`nexus`), including deleting a real
`.zonew`/`.zonel` mid-session to confirm the fallback/gradient triggers
correctly.

## 2026-09-08 — ✨ Shared "Ryzom Paths" Settings section + Atyscape tab bar, Forgery 4.1.0

`project-todos/forgery/landscape_editor__zone_render_modes__ryzom_paths_ui.md`
-- fixed a design flaw found while adding `settings.ryzom_tools_path`:
generic suite-wide settings (`live_data_path`, the `pynel.repository_paths`
checkouts) were only editable from Patina's own Settings tab, even though
Patina and Atyscape are independent apps with disjoint audiences (graphic
artists vs level designers). A level designer using Atyscape should never
have to open Patina to configure a setting only Atyscape needs -- Nuno's
litmus test: publishing Atyscape with Patina disabled would leave those
settings permanently unconfigurable.

- `_push_tab_color`/`_pop_tab_color`/`_begin_tab_item_with_icon` moved from
  `object_editor.py` to `object_editor_mixins/ui_helpers.py` (pure
  functions, no `self` dependency) so any Forgery app's tab bar can reuse
  them, not just Patina's.
- New `repository_paths_dialog.py`: `RepositoryPathsDialog`, extracted from
  `settings_dialogs.py`'s `_draw_repository_paths_settings`/
  `_poll_repository_paths_dialog`/`_poll_clone_target_dialog`/
  `_draw_clone_status` (previously Patina-only mixin methods) into a
  standalone class with a `draw(app)` method -- takes the host app for
  `app._begin_attention_flash()`/`app._end_attention_flash()` (already
  defined on `ForgeryApp`, shared by every app, not Patina-specific).
- New `ryzom_paths_section.py`: `RyzomPathsSection`, composing
  `LiveDataSetupDialog` + `RepositoryPathsDialog` + `RyzomToolsSetupDialog`
  under one "Ryzom Paths" header. Takes an existing `LiveDataSetupDialog`
  instance rather than owning one, since Patina needs that instance for
  more than this section (its own mandatory first-launch popup +
  `live_data_dir`).
- `object_editor.py`: Settings tab reorganized -- "Paths" keeps only
  workspace root/exclusion rules/search paths; new "Ryzom Paths" section
  calls `self.ryzom_paths_section.draw(self)`; "Tools" keeps only image/text
  editor pickers + workspace sync. `panoply_ui.py`'s
  `request_settings_attention("Paths", "ryzom-data")` updated to
  `"Ryzom Paths"` to match.
- `landscape_editor.py` (Atyscape): `draw_panel()` now uses a tab bar
  (`Landscape`/`Settings`) instead of one flat panel, so future features get
  their own tab too. `_draw_viewport_toggles()` (zone grid) stays outside
  the tab bar -- its own floating window, never hidden by switching tabs.
  "Settings" draws the same `RyzomPathsSection` as Patina, with its own
  `LiveDataSetupDialog` instance (no mandatory popup needed here, just the
  Settings-tab folder picker).

## 2026-09-08 — 🐛 Fix crash from uncommitted repository clone-button code, Forgery 4.0.2

`11724de87` ("Add viewport background color swatches, Forgery 4.0.0") pushed
`object_editor.py`'s `draw_panel()` calling `self._poll_clone_target_dialog()`
(and the matching `_clone_target_dialog`/`_repository_paths_dialog_repo`
field init in `__init__`), but the method itself -- part of a "Clone repo"
button added to Settings > Paths next to each `pynel.repository_paths` entry
(`_poll_clone_target_dialog`/`_draw_clone_status` in `settings_dialogs.py`,
dated 2026-09-07 in `docs/apps/object_editor.md`) -- had only ever existed
uncommitted in a local working tree, never actually committed by whichever
session wrote it. The published wheel therefore crashed Patina on launch
(`AttributeError: 'ObjectEditorApp' object has no attribute
'_poll_clone_target_dialog'`) for anyone installing via ryztart. Fixed by
committing the already-written, self-consistent method definitions
(`_poll_clone_target_dialog`, `_draw_clone_status`, plus the Clone icon
button in `_draw_repository_paths_settings` -- disabled once the repo folder
already exists) that the call site expected all along.

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
