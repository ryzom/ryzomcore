"""LandscapeEditorApp mixin: everything specific to Edition mode (project-todos/
forgery/landscape_editor__land_preview.md step 5) -- continent list read
straight from `ryzom-data`'s own `ryzom.world`, the `.land`+brick fallback
for zones/cells never yet exported by the pipeline, and the cursor status's
`.land`-assigned brick name.

Imports from `landscape_editor_modes.py`, NOT from `landscape_editor.py`
itself -- see that module's own docstring for why (circular import).
"""

import math
import threading
import time
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui

from pynel import repository_paths
from pynel.ryzom_land import load_land, save_land, LandParseError, STRING_UNUSED, ZoneUnit
from pynel.ryzom_packed_sheets import zone_name_to_world_pos
from pynel.ryzom_zone import load_zone, ZoneParseError

from ryzom_forgery import continent_ecosystem
from ryzom_forgery import continent_selector
from ryzom_forgery import continent_pipeline_reference as cpr
from ryzom_forgery.ui.colors import pastel_color_for
from ryzom_forgery.zone_geom_cache import ZoneManifestEntry
from ryzom_forgery.error_log import report_error
from ryzom_forgery import land_build
from ryzom_forgery import land_loader
from ryzom_forgery import settings as app_settings
from ryzom_forgery import workspaces
from ryzom_forgery.land_geometry import (
	brick_size_in_cells_from_half_size, expected_zone_name, find_missing_land_cells, land_cell_for_zone_name,
	land_cell_index, piece_origin, transform_zone_cache_data, used_land_cells,
)
from ryzom_forgery.apps.landscape_editor_modes import _MODE_EDITION
from ryzom_forgery.ui.widgets import _colored_button, _icon_button
from ryzom_forgery.ui.popups import center_next_popup
from ryzom_forgery.region_loader import (
	best_ref_for_extensions, continent_zone_dirs, get_zone_extensions_index, load_zone_cache_data,
	RegionLoadError, ZoneRef,
)
from ryzom_forgery.zone_cache import read_zone_cache, write_zone_cache
from ryzom_forgery.zone_geometry import ZONE_CELL_SIZE, zone_to_cache_data

# "Comme Patina" (Nuno 2026-09-14) -- an identifier called out in a section
# otherwise plain white, same idea as landscape_editor.py's own
# _STATS_HEADER_COLOR (orange for an identifier).
_NAME_COLOR = (1.0, 0.75, 0.3, 1.0)

_BUILD_PROGRESS_POPUP_ID = "Build progress"

# Minimum time between two real land_build.compute_step_statuses() calls
# (project-todos/forgery/landscape_editor__pacs_export.md step 8, Nuno
# 2026-09-19: "ça me fais ramer atyscape de fou, je tombe a 14fps") --
# _draw_pipeline_steps_section() runs every frame, but compute_step_statuses()
# itself does several hundred/thousand stat()/glob() calls on a large
# continent (up to ~10 stat() per zone just for _zone_needs_rbank_zone_
# rebuild()'s 3x3 neighbor scan, times 236 zones on newbieland) -- cheap
# enough once a second, ruinous every frame. Self._pipeline_step_statuses_cache
# holds (continent, monotonic timestamp, statuses) from the last real
# computation; a stale-by-up-to-this-much read is fine, the same tolerance
# _zone_needs_rbank_zone_rebuild()/_rbank_global_needs_rebuild() already
# accept for their own background-rebuild race (a frame or a second late
# either way, never wrong forever).
_PIPELINE_STATUS_REFRESH_INTERVAL = 1.0

# "Pipeline steps" tick colors (project-todos/forgery/landscape_editor__
# pacs_export.md step 7h, Nuno 2026-09-18) -- green/pink for a real
# up-to-date/stale check, gray/neutral for the 2 steps with no output file
# of their own to check (land_build._NO_CHECK_STEP_INDICES).
_TICK_OK_COLOR = (0.4, 1.0, 0.4, 1.0)
_TICK_STALE_COLOR = (1.0, 0.4, 0.7, 1.0)
_TICK_NEUTRAL_COLOR = (0.6, 0.6, 0.6, 1.0)
# A step with a `warning` (project-todos/forgery/landscape_editor__
# pacs_export.md step 7k, Nuno 2026-09-18: "s'il manque quoi que ce soit,
# c'est warning obligatoire") draws this instead of the plain tick -- e.g.
# flora merge with no live_data_path configured, where the .ig produced
# will silently be missing content rather than merely "not yet built".
_TICK_WARNING_COLOR = (1.0, 0.75, 0.0, 1.0)

# "Build" button (project-todos/forgery/landscape_editor__pacs_export.md
# step 7m, Nuno 2026-09-18: "en couleur") -- same green convention as
# object_editor_mixins/texture_widgets.py's own primary-action button.
_BUILD_BUTTON_COLOR = (0.2, 0.65, 0.2, 1.0)


def _indeterminate_progress_fraction() -> float:
	"""Animated back-and-forth fraction for a stage with no real
	sub-progress (a single native-tool call with no per-item feedback) --
	shared by the Build progress popup and each "Pipeline steps" row's own
	inline progress bar (project-todos/forgery/landscape_editor__
	pacs_export.md step 7n), so both animate identically rather than
	duplicating the same formula twice."""
	return (math.sin(time.monotonic() * 3.0) + 1.0) / 2.0


class EditModeMixin:
	def _load_edition_continent_locations(self):
		"""Edition mode (project-todos/forgery/landscape_editor__land_preview.md
		step 2): continent list read directly from ryzom-data's own
		ryzom.world (never live_data_path/world.packed_sheets), filtered down
		to continents that actually have a `.land` under
		<ryzom-data>/leveldesign/landscape/ -- a continent listed in
		ryzom.world but missing its `.land` doesn't appear at all in Edition
		mode."""
		ryzom_data_path = repository_paths.get("ryzom-data")
		world_file_path = Path(ryzom_data_path) / "leveldesign" / "world" / "ryzom.world"
		try:
			all_locs = continent_selector.load_continent_locations_from_world_file(world_file_path)
		except (OSError, continent_selector.ContinentSelectorError) as exc:
			self._cont_locs_error = f"Failed to load continent list from ryzom-data: {exc}"
			report_error(self._cont_locs_error)
			return
		land_files = land_loader.find_land_files(ryzom_data_path)
		self._cont_locs = [loc for loc in all_locs if loc.continent_name in land_files]

	def _resolve_land_fallback_paths(self):
		"""`(land_path, brick_zones_dir)` for the currently selected continent,
		or `None` if either can't be resolved -- the static part of the
		`.land`+brick fallback (project-todos/forgery/
		landscape_editor__land_preview.md step 3), cheap enough to resolve
		eagerly on the main thread. Only ever called from
		`_apply_render_mode()` (landscape_editor.py) once already gated on
		Edition mode + a selected continent -- see its own docstring for why
		the actual missing-cell computation happens later, in the background
		thread."""
		ryzom_data_path = repository_paths.get("ryzom-data")
		land_path = land_loader.find_land_files(ryzom_data_path).get(self.selected_continent_name)
		ecosystem_name = continent_ecosystem.get_ecosystem_for_continent(
			self._selected_continent_pipeline_name or self.selected_continent_name
		)
		if land_path is None or not ecosystem_name:
			return None
		return land_path, Path(ryzom_data_path) / "pipeline" / "landscape" / ecosystem_name / "zones"

	def _build_land_driven_refs(self, live_data_path, ryzom_data_path, pipeline_continent_name):
		"""Edition mode's zone enumeration (project-todos/forgery/
		landscape_editor__land_composition.md step 3) -- replaces the disk-scan
		`region_loader.find_zones_in_region()` used in Visualisation with an
		enumeration driven entirely by the selected continent's own `.land`:
		only cells the `.land` *currently* assigns a brick to are ever
		considered, so a real `.zone`/`.zonew`/`.zonel` left on disk at a grid
		position the `.land` no longer references (composition edited since
		that export) is silently ignored in every render mode, rather than
		shown as if still current (Nuno 2026-09-12: "Oui, filtrer dès l'étape
		2"). Returns `(default_refs, extensions, all_used_cells)` -- the first
		two in the exact shape `_run_load_continent()` (landscape_editor.py)
		already builds from `find_zones_in_region()`, so it's a drop-in
		replacement for that half of Edition mode's continent load -- cells
		with no matching real file here simply aren't included, left
		entirely to the `.land`+brick fallback (`_load_land_fallback_pieces()`)
		to render. `all_used_cells` (`{(pos_x, pos_y): ZoneUnit}`, EVERY used
		cell regardless of whether it got a `default_refs` entry) is for the
		region assignment (project-todos/forgery/landscape_editor__region_
		management.md step 5): a cell with no real export yet still has a
		world position and can still belong to a region.

		A brick spans one or more grid cells (see module docstring), but
		`land_export` always produces one real zone file per grid cell
		regardless (`cutZone()` slices the composition into 160x160 tiles) --
		so, unlike the fallback's own piece dedup, every used cell here is
		resolved independently, one expected zone name each.

		`world_pos_to_zone_name()` must be called with the cell's EXACT
		corner (`pos * ZONE_CELL_SIZE`, no interior offset) -- confirmed
		2026-09-12 against `ryzom-data`'s real `bagne` continent
		(53/53 used `.land` cells matching their real exported `.zone` file
		name) that `world_pos_to_zone_name()`'s row formula
		(`floor(-y/ZONE_CELL_SIZE)`) is off by one row from
		`zone_name_to_world_pos()`'s own inverse for any Y strictly inside a
		cell (e.g. `world_pos_to_zone_name(880, -9840)` -- the real bb_center
		of `62_AF.zone` -- returns `"61_AF"`, not `"62_AF"`): the two
		functions are ports of two independent native functions
		(`getPosFromZoneName()`/`getZoneNameFromXY()`) and aren't exact
		inverses of each other off the cell's own corner. Only the exact
		corner itself lands on the correct row in both directions."""
		land_path = land_loader.find_land_files(ryzom_data_path).get(self.selected_continent_name)
		if land_path is None:
			return {}, {}, {}
		try:
			land = load_land(land_path)
		except (OSError, LandParseError):
			return {}, {}, {}
		full_extensions_index = get_zone_extensions_index(live_data_path, pipeline_continent_name, ryzom_data_path)
		all_used_cells = used_land_cells(land)
		default_refs = {}
		extensions = {}
		for pos_x, pos_y in all_used_cells:
			name = expected_zone_name(pos_x, pos_y)
			if name is None:
				continue
			ext_map = full_extensions_index.get(name)
			if not ext_map:
				continue
			default_ref = best_ref_for_extensions(ext_map)
			if default_ref is None:
				continue
			default_refs[name] = default_ref
			extensions[name] = ext_map
		return default_refs, extensions, all_used_cells

	def _load_land_cached(self, land_path):
		"""Reuses the last parsed `.land` for `land_path` if the file hasn't
		changed since (mtime check), instead of re-parsing it from scratch
		(project-todos/forgery/landscape_editor__render_modes_removal.md,
		Nuno 2026-09-14: "j'ai toujours la barre de progression... meme
		apres 3 lancement" -- traced via (IA_AGENT_DEBUG) logs to THIS
		exact call, `load_land()`, re-parsing tryker's own 1055-cell `.land`
		file from scratch on every single region checkbox toggle, since
		_toggle_region() reloads the WHOLE checked-region union through
		_apply_render_mode() -> _run_load_refs() -> here on every single
		toggle -- the per-zone `.bam`/position caches were never the
		bottleneck, confirmed 0 misses in Nuno's own logs).

		Only ever called from _load_land_fallback_pieces(), itself only
		ever running on the single background thread _apply_render_mode()
		allows in flight at once (its own `self._mode_reload_progress`
		single-flight guard) -- safe to cache on `self` without a lock,
		nothing else ever touches `self._land_fallback_cache`."""
		try:
			mtime = land_path.stat().st_mtime
		except OSError:
			mtime = None
		cached = self._land_fallback_cache
		if cached is not None and cached[0] == land_path and cached[1] == mtime:
			return cached[2]
		land = load_land(land_path)
		self._land_fallback_cache = (land_path, mtime, land)
		return land

	def _load_land_fallback_pieces(
		self, land_path, brick_zones_dir, zones, manifest_zones, progress, failed, allowed_cells=None,
	):
		"""Background-thread body for the `.land`+brick fallback (called from
		`_run_load_refs()`, landscape_editor.py, only when `land_fallback` is
		not None -- i.e. always from Edition mode). Mutates `zones`/
		`manifest_zones` (dicts) and `failed` (list) in place -- same
		reasoning as `_run_load_continent()`'s own docstring for why
		background-thread code only ever writes to shared mutable
		structures, never touches Panda3D.

		`allowed_cells` (project-todos/forgery/landscape_editor__region_
		management.md step 5 fix), when given, restricts which missing
		cells actually get a brick loaded to this set of `(pos_x, pos_y)` --
		everything else stays out of `zones` entirely (a purple square via
		_rebuild_region_placeholders(), not a loaded brick). `None` means
		unrestricted, the pre-region_management behaviour (every missing
		cell gets its brick, regardless of region)."""
		try:
			land = self._load_land_cached(land_path)
			used_cells = sum(1 for unit in land.zones if unit.zone_name != STRING_UNUSED)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor_edit_mode.py:_load_land_fallback_pieces) "
			      f"land loaded: {land_path} used_cells={used_cells} exported_zones={len(zones)}")
			# By NAME, not by the loaded zone's own geometric bb_center
			# (found 2026-09-12, Nuno: "on se retrouve avec 2 zones de 2
			# couleurs au meme endroit") -- a real zone's bounding box
			# doesn't always center exactly inside its own nominal cell,
			# which let a cell wrongly get BOTH a real zone (resolved by
			# name in _build_land_driven_refs()) AND a `.land` brick
			# fallback (resolved by bb_center here) at the same position.
			# land_cell_for_zone_name() is the exact inverse of
			# expected_zone_name()/_build_land_driven_refs()'s own
			# resolution, so the two can never disagree anymore.
			existing_cells = {
				cell for name in zones for cell in (land_cell_for_zone_name(name),) if cell is not None
			}
			# A brick can be a multi-cell "large piece" (e.g. 320x160)
			# referenced by several cells at once (each storing its
			# own sub-position within the piece, ZoneUnit.pos_x/
			# pos_y -- see land_geometry.py's own docstring) --
			# loaded_bricks avoids re-parsing the same file per cell,
			# rendered_pieces dedupes so a multi-cell piece is
			# rendered exactly once (found 2026-09-10, Nuno: without
			# this, the same piece was drawn once per cell it spans,
			# overlapping itself).
			loaded_bricks = {}
			rendered_pieces = set()
			missing_cells = find_missing_land_cells(land, existing_cells)
			for (pos_x, pos_y), unit in missing_cells.items():
				if allowed_cells is not None and (pos_x, pos_y) not in allowed_cells:
					continue
				brick_path = brick_zones_dir / f"{unit.zone_name}.zone"
				if unit.zone_name not in loaded_bricks:
					# Cached raw (untransformed) geometry (project-todos/
					# forgery/landscape_editor__zone_render_modes.md,
					# Nuno 2026-09-11 perf follow-up) -- keyed on the
					# brick file alone (never the placement), shared by
					# every cell/rotation that references this brick.
					# On a hit, `load_zone(brick_path)` never runs at
					# all: bb_half_size (for brick_size_in_cells_from_
					# half_size(), needed just below to even compute
					# origin_x/origin_y) comes straight from the cached
					# ZoneCacheData.
					raw_cache_name = f"land_brick_{unit.zone_name}"
					raw_data = read_zone_cache(raw_cache_name, ".zone", brick_path)
					if raw_data is None:
						try:
							brick_zone = load_zone(brick_path)
							raw_data = zone_to_cache_data(brick_zone)
							write_zone_cache(raw_cache_name, ".zone", brick_path, raw_data)
						except (OSError, ZoneParseError) as exc:
							raw_data = None
							failed.append(f"{unit.zone_name}.zone: {exc}")
					loaded_bricks[unit.zone_name] = raw_data
				raw_data = loaded_bricks[unit.zone_name]
				if raw_data is None:
					continue
				size_x, size_y = brick_size_in_cells_from_half_size(*raw_data.bb_half_size[:2])
				origin_x, origin_y = piece_origin(pos_x, pos_y, unit, size_x, size_y)
				piece_key = (unit.zone_name, origin_x, origin_y, unit.rot, unit.flip)
				if piece_key in rendered_pieces:
					continue
				rendered_pieces.add(piece_key)
				cell_name = f"land:{origin_x}:{origin_y}"
				# Cache key includes the piece's placement (origin/rot/
				# flip), not just the brick name -- the SAME brick can
				# appear rotated/flipped at several origins, each
				# needing its own transformed geometry.
				piece_cache_name = f"land_piece_{unit.zone_name}_{origin_x}_{origin_y}_{unit.rot}_{unit.flip}"
				piece_data = read_zone_cache(piece_cache_name, ".zone", brick_path)
				if piece_data is None:
					piece_data = transform_zone_cache_data(raw_data, origin_x, origin_y, unit.rot, unit.flip)
					try:
						write_zone_cache(piece_cache_name, ".zone", brick_path, piece_data)
					except OSError:
						pass
				zones[cell_name] = piece_data
				# ".land" is never a real zone extension (.zone/.zonew/
				# .zonel) -- unambiguous marker that this manifest entry
				# is a placed brick, not an exported zone, and its
				# staleness stamp is the brick file's, not a ZoneRef's.
				try:
					brick_stat = brick_path.stat()
					# rot/flip (project-todos/forgery/landscape_editor__
					# region_management__zone_bam_cache.md step 3): the same
					# brick file re-rotated/re-flipped in place at this cell
					# has an identical extension/mtime/size, so without
					# these two fields a per-zone `.bam` cache would wrongly
					# keep serving the old orientation.
					manifest_zones[cell_name] = ZoneManifestEntry(
						".land", brick_stat.st_mtime, brick_stat.st_size, rot=unit.rot, flip=unit.flip,
					)
				except OSError:
					pass
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor_edit_mode.py:_load_land_fallback_pieces) "
			      f"land fallback: existing_cells={len(existing_cells)} added={sorted(n for n in zones if n.startswith('land:'))}")
		except (OSError, LandParseError) as exc:
			failed.append(f"{land_path}: {exc}")

	def _ensure_land_cell_names_loaded(self):
		"""(pos_x, pos_y) -> ZoneUnit.zone_name (the brick the .land itself
		assigns to that cell), for _update_cursor_status() -- always the
		.land's own reference, whatever pipeline stage actually ends up
		rendered there (Nuno 2026-09-10). Cached per continent (re-read only
		when the selection changes), cleared when no continent is selected
		(the caller, landscape_editor.py's _update_cursor_status(), only
		calls this in Edition mode to begin with)."""
		if self.selected_continent_name is None:
			self._land_cell_names = {}
			self._land_cell_names_continent = None
			return
		if self._land_cell_names_continent == self.selected_continent_name:
			return
		self._land_cell_names_continent = self.selected_continent_name
		self._land_cell_names = {}
		ryzom_data_path = repository_paths.get("ryzom-data")
		land_path = land_loader.find_land_files(ryzom_data_path).get(self.selected_continent_name)
		if land_path is None:
			return
		try:
			land = load_land(land_path)
		except (OSError, LandParseError):
			return
		width = land.max_x - land.min_x + 1
		self._land_cell_names = {
			(land.min_x + (i % width), land.min_y + (i // width)): unit.zone_name
			for i, unit in enumerate(land.zones) if unit.zone_name != STRING_UNUSED
		}

	def _ensure_land_region_loaded(self):
		"""Loads (once per continent selection, project-todos/forgery/
		landscape_editor__land_composition.md step 6) and caches the `.land`
		ZoneRegion the grid editor mutates directly -- `self._land_region`/
		`self._land_region_path` are cleared whenever the continent selection
		changes (`_select_continent()`, landscape_editor.py), so a mismatch
		here always means "not loaded yet for the current continent", never a
		stale one from a previous selection."""
		if self.selected_continent_name is None:
			self._land_region = None
			self._land_region_path = None
			return
		land_fallback = self._resolve_land_fallback_paths()
		if land_fallback is None:
			self._land_region = None
			self._land_region_path = None
			return
		land_path, _ = land_fallback
		if self._land_region is not None and self._land_region_path == land_path:
			return
		try:
			self._land_region = load_land(land_path)
			self._land_region_path = land_path
		except (OSError, LandParseError) as exc:
			self._land_region = None
			self._land_region_path = None
			self._land_edit_error = f"Failed to load {land_path}: {exc}"
			report_error(self._land_edit_error)

	def _available_land_bricks(self, brick_zones_dir):
		"""Sorted brick `.zone` stems under `brick_zones_dir`, for the grid
		editor's brick picker (step 6) -- cached per directory (re-scanned
		only when the continent/ecosystem changes), since this same directory
		listing would otherwise be re-read from disk every single frame the
		editor panel is open."""
		if self._land_available_bricks_dir != brick_zones_dir:
			try:
				self._land_available_bricks = sorted(p.stem for p in brick_zones_dir.glob("*.zone"))
			except OSError as exc:
				self._land_available_bricks = []
				self._land_edit_error = f"Failed to list bricks in {brick_zones_dir}: {exc}"
				report_error(self._land_edit_error)
			self._land_available_bricks_dir = brick_zones_dir
		return self._land_available_bricks

	def _save_land_region(self):
		"""Writes `self._land_region` back to `self._land_region_path` (always
		v1, `pynel.ryzom_land.save_land()`'s own convention) -- a `.land` is
		small XML, cheap enough to rewrite in full on every single edit
		rather than batching changes (step 6). Does NOT reload the continent
		itself anymore (Nuno 2026-09-12: "ca ne devrait que modifier UNE
		seule zone") -- callers refresh just the edited cell's own display
		via _apply_lightweight_cell_update() instead of a full
		_load_continent()."""
		try:
			save_land(self._land_region_path, self._land_region)
		except OSError as exc:
			self._land_edit_error = f"Failed to save {self._land_region_path}: {exc}"
			report_error(self._land_edit_error)
			return
		self._land_edit_error = None
		# Forces _ensure_land_cell_names_loaded() to re-read the .land it
		# just cached for the cursor-status brick name (unrelated cache, see
		# that method's own docstring) -- otherwise the status line would
		# keep showing the pre-edit brick name at this cell.
		self._land_cell_names_continent = None

	def _apply_lightweight_cell_update(self, pos_x, pos_y):
		"""Refreshes ONLY cell `(pos_x, pos_y)`'s own displayed geometry after
		an edit (step 6, Nuno 2026-09-12: "ca ne devrait que modifier UNE
		seule zone (celles autour sont deja .zone only)") -- replaces the
		previous full `_load_continent()` reload, which redundantly rebuilt
		every other zone's `GeomNode` even though editing one cell never
		changes how any OTHER zone is DISPLAYED (only a real Build's
		welding/lighting can do that -- see `_invalidate_built_zone_files()`'s
		own docstring for why that still touches the 8 neighbors' BUILT
		FILES on disk, just never their live display here).

		The edited cell always becomes a `.land` brick fallback immediately
		(its own real files were just deleted by `_invalidate_built_zone_
		files()`), rendered as an ordinary single-cell piece -- this editor
		only ever writes `pos_x=pos_y=0` `ZoneUnit`s (see `_apply_land_cell_
		edit()`'s own docstring), never a multi-cell "large piece", so no
		piece-origin/dedup logic is needed here unlike `_load_land_fallback_
		pieces()`."""
		real_name = expected_zone_name(pos_x, pos_y)
		fallback_name = f"land:{pos_x}:{pos_y}"
		# Drop whatever was displayed at this position before, under EITHER
		# naming scheme (a real name if it was still fully built until this
		# very edit, or an older fallback piece if it already wasn't).
		for stale_name in (real_name, fallback_name):
			if stale_name is None:
				continue
			old_node = self._zone_nodes.pop(stale_name, None)
			if old_node is not None:
				old_node.remove_node()
			self.zones.pop(stale_name, None)
			self._loaded_refs.pop(stale_name, None)
			self._loaded_extensions.pop(stale_name, None)

		land_fallback = self._resolve_land_fallback_paths()
		if land_fallback is None or self._land_region is None:
			return
		_, brick_zones_dir = land_fallback
		index = land_cell_index(self._land_region, pos_x, pos_y)
		if index is None:
			return
		unit = self._land_region.zones[index]
		if unit.zone_name == STRING_UNUSED:
			return  # cleared -- nothing left to display at this cell

		brick_path = brick_zones_dir / f"{unit.zone_name}.zone"
		raw_cache_name = f"land_brick_{unit.zone_name}"
		raw_data = read_zone_cache(raw_cache_name, ".zone", brick_path)
		if raw_data is None:
			try:
				brick_zone = load_zone(brick_path)
			except (OSError, ZoneParseError) as exc:
				self._land_edit_error = f"Failed to load brick {unit.zone_name}: {exc}"
				report_error(self._land_edit_error)
				return
			raw_data = zone_to_cache_data(brick_zone)
			try:
				write_zone_cache(raw_cache_name, ".zone", brick_path, raw_data)
			except OSError:
				pass
		piece_data = transform_zone_cache_data(raw_data, pos_x, pos_y, unit.rot, unit.flip)
		self.zones[fallback_name] = piece_data
		self._rebuild_zone_node(fallback_name, piece_data)

	def _invalidate_built_zone_files(self, pos_x, pos_y):
		"""Deletes any already-built files for cell `(pos_x, pos_y)` AND its 8
		neighbors that are no longer valid after this edit (project-todos/
		forgery/landscape_editor__land_composition.md step 7's incremental
		Build) -- two different reasons, two different sets of files:

		- The 8 NEIGHBORS lose only `.zonew`/`.zonel`/`.depend`/`.ig`, never
		  `.zone`: a changed cell's welded border/lighting can affect an
		  unmoved neighbor too (Nuno 2026-09-12: "je vois un soucis... le
		  zone_lighter se fais sur toutes les zones... meme celles qui ont
		  deja un .zonel" -- his chosen fix, "suivre aussi les zones
		  voisines affectees"), but a neighbor's own raw elevation
		  (`.zone`) only ever samples the continent-wide heightmap at ITS
		  OWN world position, never this cell's composition -- it never
		  actually goes stale from this edit (Nuno 2026-09-12: "les .zone
		  voisines... je ne vois pas l'interet").
		- The EDITED cell itself additionally loses its OWN `.zone` too:
		  unlike a neighbor, ITS composition (brick/rot/flip) just
		  genuinely changed, so its existing `.zone` -- elevation output
		  for the OLD brick -- is now flat-out wrong geometry, not merely
		  "pending a rebuild" (found 2026-09-12, Nuno walking through a
		  concrete before/after scenario: without this, the edited cell
		  would incorrectly read as build-stage 1/orange, using stale
		  geometry, instead of stage 0/red).

		Called from _apply_land_cell_edit()/_clear_land_cell(), before
		_save_land_region()/_apply_lightweight_cell_update() persist the
		edit and refresh the display. Silently does nothing for a cell/
		neighbor outside the valid world grid, or if ryzom-data isn't
		configured."""
		ryzom_data_path = repository_paths.get("ryzom-data")
		continent = self._selected_continent_pipeline_name
		if not ryzom_data_path or not continent:
			return
		try:
			out_ig_dir = Path(cpr.build_land_export_config(ryzom_data_path, continent).out_ig_dir)
		except cpr.ContinentPipelineReferenceError as exc:
			report_error(f"Failed to invalidate built zone files: {exc}")
			return
		zone_dir, zone_weld_dir, zone_depends_dir, zone_lighted_dir = continent_zone_dirs(ryzom_data_path, continent)
		for dx in (-1, 0, 1):
			for dy in (-1, 0, 1):
				name = expected_zone_name(pos_x + dx, pos_y + dy)
				if name is None:
					continue
				paths = [
					zone_weld_dir / f"{name}.zonew",
					zone_lighted_dir / f"{name}.zonel", zone_depends_dir / f"{name.lower()}.depend",
					out_ig_dir / f"{name}.ig",
				]
				if dx == 0 and dy == 0:
					paths.append(zone_dir / f"{name}.zone")
				for path in paths:
					try:
						path.unlink()
					except FileNotFoundError:
						pass
					except OSError as exc:
						report_error(f"Failed to remove stale {path}: {exc}")

	def _apply_land_cell_edit(self, pos_x, pos_y, brick_name, rot, flip):
		"""Writes `brick_name`/`rot`/`flip` to `.land` cell `(pos_x, pos_y)`
		(step 6) -- always as an ordinary single-cell brick (`pos_x`/`pos_y`
		"position in a large piece" left at 0, `sharing_mat_names`/
		`sharing_cut_edges` reset to their unused defaults, dates left at 0):
		placing a multi-cell "large piece" isn't exposed by this editor
		(module docstring's own hors-scope note) -- only choosing/placing a
		single ordinary brick by name."""
		index = land_cell_index(self._land_region, pos_x, pos_y)
		if index is None:
			self._land_edit_error = f"Cell ({pos_x}, {pos_y}) is outside the .land grid."
			report_error(self._land_edit_error)
			return
		self._land_region.zones[index] = ZoneUnit(zone_name=brick_name, pos_x=0, pos_y=0, rot=rot, flip=1 if flip else 0)
		self._invalidate_built_zone_files(pos_x, pos_y)
		self._save_land_region()
		self._apply_lightweight_cell_update(pos_x, pos_y)

	def _clear_land_cell(self, pos_x, pos_y):
		"""Removes whatever brick `(pos_x, pos_y)` currently has (step 6) --
		sets it back to `STRING_UNUSED`, the `.land` format's own convention
		for an empty cell."""
		index = land_cell_index(self._land_region, pos_x, pos_y)
		if index is None:
			return
		self._land_region.zones[index] = ZoneUnit(zone_name=STRING_UNUSED, pos_x=0, pos_y=0, rot=0, flip=0)
		self._invalidate_built_zone_files(pos_x, pos_y)
		self._save_land_region()
		self._apply_lightweight_cell_update(pos_x, pos_y)

	def _draw_land_composition_editor(self):
		"""Grid editing panel (step 6) -- only meaningful in Edition mode,
		with a continent selected and a grid cell picked (project-todos/
		forgery/landscape_editor__render_modes_removal.md: shown unconditionally
		in Edition once those two are true, no render-mode gate anymore since
		the render mode picker itself was removed). Shown at the bottom of
		the Landscape tab (`_draw_landscape_tab()`, landscape_editor.py).
		Colored icon header + orange current-brick name (Nuno 2026-09-14:
		"comme Patina") -- the cell coordinates used to be in the title too,
		dropped as redundant noise (Nuno: "vire le -- cell, sert à rien"),
		the cursor status bar already shows them."""
		if self._app_mode != _MODE_EDITION or not self.selected_continent_name:
			return
		if self._land_selected_cell is None:
			return
		land_fallback = self._resolve_land_fallback_paths()
		if land_fallback is None:
			return
		_, brick_zones_dir = land_fallback
		self._ensure_land_region_loaded()
		if self._land_region is None:
			return
		pos_x, pos_y = self._land_selected_cell
		index = land_cell_index(self._land_region, pos_x, pos_y)

		imgui.separator()
		imgui.push_style_color(imgui.Col_.text.value, pastel_color_for(fa_icons.ICON_FA_LAYER_GROUP))
		imgui.text(f"{fa_icons.ICON_FA_LAYER_GROUP} Composition editor")
		imgui.pop_style_color()
		if index is None:
			imgui.text_colored((1.0, 0.7, 0.3, 1.0), "Outside the .land grid -- growing the grid isn't supported.")
			return

		unit = self._land_region.zones[index]
		current_brick = "" if unit.zone_name == STRING_UNUSED else unit.zone_name
		bricks = self._available_land_bricks(brick_zones_dir)
		imgui.text("Current brick:")
		imgui.same_line()
		imgui.text_colored(_NAME_COLOR, current_brick or "(empty)")

		# Build-status checklist (Nuno 2026-09-12) -- real per-extension
		# presence for this cell's own real zone name, straight from
		# self._loaded_extensions (already the FULL {ext: ZoneRef} map for
		# this name, see _build_land_driven_refs()'s own docstring), same
		# real-disk-state philosophy as zone_geometry.zone_build_stage()'s
		# coloring. A present stage's own file can be deleted (trash icon,
		# Nuno 2026-09-14: "quand on decoche => supprime le fichier .zone" --
		# a plain checkbox can't offer a one-way, check-only-to-uncheck
		# interaction in ImGui, so a delete button is the direct equivalent)
		# -- only for a loose file (never packed inside a `.bnp`, not a
		# real case in Édition but guarded anyway), and never for a missing
		# stage (nothing to delete, and a checkbox can't single-handedly
		# GENERATE a pipeline stage -- that's what [Build] is for).
		real_name = expected_zone_name(pos_x, pos_y)
		ext_map = self._loaded_extensions.get(real_name, {}) if real_name is not None else {}
		for ext in (".zone", ".zonew", ".zonel"):
			ref = ext_map.get(ext)
			present = ref is not None
			mark = "✅" if present else "❌"
			imgui.text(f"{mark} {ext}")
			if present and ref.bnp_entry is None:
				imgui.same_line()
				imgui.push_id(ext)
				if _icon_button(fa_icons.ICON_FA_TRASH, f"Delete {ext}", square=True):
					self._delete_zone_stage_file(pos_x, pos_y, ext)
				imgui.pop_id()

		if imgui.begin_combo("##land-brick-picker", current_brick or "(choose a brick)"):
			for brick_name in bricks:
				selected = brick_name == current_brick
				clicked, _ = imgui.selectable(brick_name, selected)
				if clicked:
					self._apply_land_cell_edit(pos_x, pos_y, brick_name, unit.rot, unit.flip)
				if selected:
					imgui.set_item_default_focus()
			imgui.end_combo()

		if current_brick:
			imgui.text(f"Rotation: {unit.rot * 90} deg")
			imgui.same_line()
			if imgui.button("Rotate 90 deg"):
				self._apply_land_cell_edit(pos_x, pos_y, unit.zone_name, (unit.rot + 1) % 4, unit.flip)
			flip_clicked, flip_value = imgui.checkbox("Flip", bool(unit.flip))
			if flip_clicked:
				self._apply_land_cell_edit(pos_x, pos_y, unit.zone_name, unit.rot, flip_value)
			if imgui.button("Clear cell"):
				self._clear_land_cell(pos_x, pos_y)

		if self._land_edit_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._land_edit_error)

	def _delete_zone_stage_file(self, pos_x, pos_y, ext):
		"""Deletes cell `(pos_x, pos_y)`'s own `ext` pipeline file on disk
		(project-todos/forgery/landscape_editor__render_modes_removal.md,
		the composition editor's per-stage trash icon) and updates the live
		display in place -- these are pure BUILD OUTPUTS (`.zone`/`.zonew`/
		`.zonel`), always regenerable from the `.land` composition via
		[Build], never the composition itself, so deleting one is safe,
		forced-rebuild territory, not data loss. Re-resolves to whatever
		stage remains (`best_ref_for_extensions()`) rather than blindly
		falling back to the raw `.land` brick -- deleting only the newest
		stage (e.g. `.zonel`) still shows the next one down (`.zonew`) if
		it's still on disk, not a full regression to stage 0."""
		real_name = expected_zone_name(pos_x, pos_y)
		if real_name is None:
			return
		ext_map = dict(self._loaded_extensions.get(real_name, {}))
		ref = ext_map.get(ext)
		if ref is None or ref.bnp_entry is not None:
			return
		try:
			ref.source_path.unlink()
		except OSError as exc:
			self._land_edit_error = f"Failed to delete {ref.source_path}: {exc}"
			report_error(self._land_edit_error)
			return
		self._land_edit_error = None
		del ext_map[ext]
		self._loaded_extensions[real_name] = ext_map
		new_ref = best_ref_for_extensions(ext_map)
		if new_ref is None:
			# Nothing real left at all -- drop it from the loaded set and
			# fall back to the raw `.land` brick, same as any other
			# never-built cell (_apply_lightweight_cell_update()'s own
			# normal case).
			self._loaded_refs.pop(real_name, None)
			old_node = self._zone_nodes.pop(real_name, None)
			if old_node is not None:
				old_node.remove_node()
			self.zones.pop(real_name, None)
			self._apply_lightweight_cell_update(pos_x, pos_y)
			return
		self._loaded_refs[real_name] = new_ref
		try:
			cache_data = load_zone_cache_data(new_ref)
		except (RegionLoadError, OSError) as exc:
			self._land_edit_error = f"Failed to reload {real_name}: {exc}"
			report_error(self._land_edit_error)
			return
		self.zones[real_name] = cache_data
		self._rebuild_zone_node(real_name, cache_data)

	def _start_land_build(self):
		"""Starts the full native pipeline (project-todos/forgery/
		landscape_editor__land_composition.md step 7) for the currently
		selected continent, on a background thread (land_build.run_full_build()
		-- real subprocess calls, potentially dozens of zones, never on the
		main/render thread). A second call while one is already running is
		ignored, same convention as _load_continent()/_generate_missing_
		zonew()."""
		if self._land_build_progress is not None and not self._land_build_progress["done"]:
			return
		if not self.selected_continent_name or not self._selected_continent_pipeline_name:
			return
		ryzom_data_path = repository_paths.get("ryzom-data")
		live_data_path = app_settings.load().live_data_path
		continent = self._selected_continent_pipeline_name
		progress = {
			"done": False, "error": None,
			"step_index": 0, "step_count": 1, "step_name": "Starting...", "fraction": None,
			"zonel_files": [], "ready": [], "reloaded": False,
		}
		self._land_build_progress = progress
		# Deferred to _draw_land_build_progress_popup() itself (project-todos/
		# forgery/landscape_editor__pacs_export.md step 7n bugfix, Nuno
		# 2026-09-18: "aucune popup de visible") -- imgui.open_popup(str_id)
		# hashes str_id together with whatever ID stack is active AT THE
		# CALL SITE (same mechanism as GetID() for any widget), so it must
		# run at the exact same ID-stack depth as the begin_popup_modal()
		# call that checks for it, never from inside a push_id()'d button
		# click (the isolated [rebuild] button's own bug, fixed the same
		# way here for consistency even though this particular call site
		# was never nested).
		self._pending_popup_open = True
		thread = threading.Thread(
			target=self._run_land_build, args=(ryzom_data_path, live_data_path, continent, progress), daemon=True,
		)
		thread.start()

	def _run_land_build(self, ryzom_data_path, live_data_path, continent, progress):
		"""Background-thread body for _start_land_build() -- writes only to
		`progress` (dict field writes/list.append(), safe under the GIL, same
		reasoning as _run_load_continent()'s own docstring). `progress["ready"]`
		is drained by _draw_land_build_button() for the live 3D viewport
		update."""
		def on_progress(step_index, step_count, step_name, fraction):
			progress["step_index"] = step_index
			progress["step_count"] = step_count
			progress["step_name"] = step_name
			progress["fraction"] = fraction

		try:
			zonel_files = land_build.run_full_build(
				ryzom_data_path, continent, live_data_path=live_data_path,
				on_progress=on_progress,
				on_zone_ready=lambda name, cache_data: progress["ready"].append((name, cache_data)),
			)
			progress["zonel_files"] = zonel_files
		except land_build.LandBuildError as exc:
			progress["error"] = str(exc)
			report_error(f"Land build failed: {exc}")
		finally:
			progress["done"] = True

	def _process_land_build_progress(self, progress):
		"""Live-viewport/reload side effects of a running or just-finished
		Build (project-todos/forgery/landscape_editor__pacs_export.md step
		7m) -- pure state update, no drawing, extracted out of
		_draw_land_build_button() so it keeps running every frame regardless
		of where that button is drawn on screen."""
		if progress is None:
			return
		# Live viewport update (Nuno 2026-09-12): each zone lit by
		# land_build.run_full_build() during this Build run appears in the
		# 3D view immediately, one zone at a time -- a brand new zone (never
		# loaded before, e.g. a newly-added cell) is added to
		# self.zones/self._loaded_refs/self._loaded_extensions here too, not
		# just refreshed, since _rebuild_zone_node() alone only replaces an
		# EXISTING entry.
		ready = progress["ready"]
		while ready:
			name, cache_data = ready.pop(0)
			is_new = name not in self.zones
			# _loaded_extensions must gain its new ".zonel" entry BEFORE
			# _rebuild_zone_node() runs -- that call computes the zone's
			# build-stage color from _loaded_extensions right away
			# (zone_geometry.zone_build_stage()), so doing this after would
			# color the zone one stage behind on this very update (found
			# 2026-09-12 reviewing this code).
			if is_new:
				pos = zone_name_to_world_pos(name)
				new_ref = ZoneRef(name=name, x=pos.x, y=pos.y, source_path=Path(), bnp_entry=None)
				self._loaded_refs[name] = new_ref
				self._loaded_extensions.setdefault(name, {})[".zonel"] = new_ref
			else:
				self._loaded_extensions.setdefault(name, {})[".zonel"] = self._loaded_refs.get(name)
			self.zones[name] = cache_data
			self._rebuild_zone_node(name, cache_data)
		if progress["done"] and not progress["reloaded"]:
			# One-time full reload once the Build finishes (Nuno 2026-09-12:
			# "tu ajoutes des zones sans retirer les existantes") -- the live
			# per-zone drain above only ever touches
			# self._zone_nodes[<real name>], never the OLD `.land`-fallback
			# node a just-lit cell used to be shown as (keyed
			# "land:<origin_x>:<origin_y>", a different dict key -- see
			# _load_land_fallback_pieces() -- and, for a multi-cell piece,
			# not even the same origin as this cell's own position).
			# _load_continent() does a full disk rescan +
			# _set_loaded_zones() rebuild, which tears down every old node
			# unconditionally and only recreates what the `.land`+disk state
			# actually calls for now -- the only reliable way to drop a
			# stale fallback node here, cheap enough to always do once per
			# Build (not per zone).
			progress["reloaded"] = True
			self._load_continent()

	def _draw_land_build_button(self):
		""""Build" button -- runs the whole `land_export` -> `zone_welder` ->
		`zone_elevation` -> `zone_welder` -> `zone_dependencies` ->
		`zone_lighter` -> `zone_ig_lighter` -> PACS (`build_ig_boxes`/
		`build_rbank`/`build_indoor_rbank`) -> `ai_wmap_export` chain for the
		current composition, producing final installable `.zonel`/`.ig`/
		`.rbank`/`.gr`/`.lr`/`ai_wmap` files. Only meaningful in Edition mode
		with a continent selected -- Visualisation never has a `.land` to
		build from. Progress is shown in a blocking modal popup
		(_draw_land_build_progress_popup()) AND inline per step in
		"Pipeline steps" (_draw_pipeline_steps_section()). Drawn at the very
		bottom, colored (project-todos/forgery/landscape_editor__
		pacs_export.md step 7m, Nuno 2026-09-18: "encore mieux d'avoir ces
		étapes visibles et validées une à une avec le build complet... le
		bouton build tout en bas, en couleur") -- "Pipeline steps" is drawn
		first and is no longer foldable, so it's always the thing right
		above this button."""
		if self._app_mode != _MODE_EDITION or not self.selected_continent_name:
			return
		imgui.separator()
		progress = self._land_build_progress
		self._process_land_build_progress(progress)
		self._draw_pipeline_steps_section()
		building = progress is not None and not progress["done"]
		imgui.begin_disabled(building)
		if _colored_button("Build", _BUILD_BUTTON_COLOR):
			self._start_land_build()
		imgui.end_disabled()
		if progress is not None and progress["done"]:
			if progress["error"] is not None:
				imgui.same_line()
				imgui.text_colored((1.0, 0.4, 0.4, 1.0), progress["error"])
			elif progress["zonel_files"]:
				imgui.same_line()
				imgui.text_colored((0.4, 1.0, 0.4, 1.0), f"{len(progress['zonel_files'])} zone(s) built.")
		self._draw_land_build_progress_popup()

	def _start_pipeline_step_rebuild(self, step_index: int, step_name: str):
		"""[rebuild] button click (project-todos/forgery/landscape_editor__
		pacs_export.md step 7i, Nuno 2026-09-18) -- clears step_index's own
		cache and reruns it (or its whole shared-indicator group) RIGHT NOW,
		on a background thread since it makes real native subprocess calls,
		never waiting for a subsequent full Build. Ignored while a full
		Build or another isolated rebuild is already running (same
		mutual-exclusion convention as _start_land_build()). Progress shown
		in the SAME modal popup as a full Build (step 7n, corrected per Nuno
		2026-09-18: "je veux les 2 builds en modal, rien du tout dans le
		tableau") -- never anything in the "Pipeline steps" table itself."""
		building = self._land_build_progress is not None and not self._land_build_progress["done"]
		rebuilding = self._pipeline_step_progress is not None and not self._pipeline_step_progress["done"]
		if building or rebuilding:
			return
		if not self.selected_continent_name or not self._selected_continent_pipeline_name:
			return
		ryzom_data_path = repository_paths.get("ryzom-data")
		live_data_path = app_settings.load().live_data_path
		continent = self._selected_continent_pipeline_name
		progress = {
			"step_index": step_index, "step_name": step_name, "done": False, "fraction": None, "error": None,
			"reloaded": False,
		}
		self._pipeline_step_progress = progress
		# Deferred to _draw_land_build_progress_popup() -- see
		# _start_land_build()'s own comment on why open_popup() can't run
		# from here directly (this exact call site is the one that
		# surfaced the bug: it fires from inside the table's push_id()'d
		# button click).
		self._pending_popup_open = True
		thread = threading.Thread(
			target=self._run_pipeline_step_rebuild,
			args=(ryzom_data_path, live_data_path, continent, step_index, step_name, progress),
			daemon=True,
		)
		thread.start()

	def _run_pipeline_step_rebuild(self, ryzom_data_path, live_data_path, continent, step_index, step_name, progress):
		"""Background-thread body for _start_pipeline_step_rebuild()."""
		def on_progress(reported_index, step_count, reported_name, fraction):
			progress["fraction"] = fraction

		try:
			land_build.run_single_step(
				ryzom_data_path, continent, step_index, live_data_path=live_data_path, on_progress=on_progress,
			)
			self._pipeline_force_error = None
		except land_build.LandBuildError as exc:
			progress["error"] = f"Failed to rebuild {step_name}: {exc}"
			self._pipeline_force_error = progress["error"]
			report_error(progress["error"])
		finally:
			progress["done"] = True

	def _draw_pipeline_steps_section(self):
		"""Always-visible "Pipeline steps" section, drawn ABOVE the [Build]
		button (project-todos/forgery/landscape_editor__pacs_export.md steps
		7f-7o, Nuno 2026-09-18: "encore mieux d'avoir ces étapes visibles et
		validées une à une avec le build complet" -- no longer foldable,
		unlike its original design): one line per run_full_build() stage,
		its short 2-word label (tooltip on hover gives the native tool
		actually invoked, or "None (Python)" for the 2 stages with none),
		and a colored tick (green = up to date, pink = stale, orange =
		warning, e.g. flora merge with no live_data_path configured) --
		never a progress bar in this table (step 7n, corrected per Nuno
		2026-09-18: "je veux les 2 builds en modal, rien du tout dans le
		tableau"): both a full Build and an isolated [rebuild] show their
		progress in the SAME modal popup (_draw_land_build_progress_popup()),
		nothing inline here. A [rebuild] button clears that
		step's cache and reruns it
		immediately (`land_build.run_single_step()`) -- one group of steps
		sharing a single indicator (weld pass 1/elevation/weld pass 2) shares
		ONE table row (step 7l) and ONE button, vertically centered against
		their multiple lines (step 7o, Nuno
		2026-09-18: "si tu peux centrer les boutons"); `land_build.
		compute_step_statuses()` decides which step has its own button, this
		just draws whatever it's told. A separate open-folder button (project-
		todos/forgery/pipeline_export_dirs_rename.md step 15b, Nuno
		2026-09-19) opens each step's own real destination directory
		(`status.destination_dir`) in the OS file manager -- always one per
		MEMBER line, even inside a group, unlike the shared [rebuild] button.
		The tick also carries a "built/total" count (step 15c, Nuno
		2026-09-19: "5/35 veut dire qu'il y a en sortie 5 fichier sur 35
		requis"), and turns into a lock icon -- [rebuild] frozen too -- when
		`can_build` is `False` (this step's own real input isn't there yet).
		Édition mode only, same gating as the
		[Build] button itself."""
		if self._app_mode != _MODE_EDITION or not self.selected_continent_name:
			return
		imgui.text(f"{fa_icons.ICON_FA_CIRCLE_INFO} Pipeline steps")
		ryzom_data_path = repository_paths.get("ryzom-data")
		continent = self._selected_continent_pipeline_name
		if not ryzom_data_path or not continent:
			return
		# self._continent_all_zone_names (every used .land cell's expected
		# zone name, real export or not) rather than self._loaded_extensions
		# (which only ever tracks a cell once it has a real exported file)
		# -- project-todos/forgery/landscape_editor__pacs_export.md step 7h
		# fix, Nuno 2026-09-18: a freshly-authored continent with zero
		# exports needs a real "0/53", not a vacuous "0/0" read as up to
		# date. Falls back to self._loaded_extensions if the continent
		# hasn't finished its own background load yet (all_zone_names still
		# None at that point).
		zone_names = self._continent_all_zone_names
		if zone_names is None:
			zone_names = self._loaded_extensions.keys()
		live_data_path = app_settings.load().live_data_path
		cache = self._pipeline_step_statuses_cache
		now = time.monotonic()
		if cache is not None and cache[0] == continent and now - cache[1] < _PIPELINE_STATUS_REFRESH_INTERVAL:
			statuses = cache[2]
		else:
			statuses = land_build.compute_step_statuses(ryzom_data_path, continent, zone_names, live_data_path)
			self._pipeline_step_statuses_cache = (continent, now, statuses)
		rebuild_progress = self._pipeline_step_progress
		rebuild_running = rebuild_progress is not None and not rebuild_progress["done"]
		build_running = self._land_build_progress is not None and not self._land_build_progress["done"]
		if rebuild_progress is not None and rebuild_progress["done"] and not rebuild_progress["reloaded"]:
			# Whatever step (or group) was just rebuilt in isolation, once
			# EVERY step now reads fully up to date (project-todos/forgery/
			# landscape_editor__pacs_export.md step 7p, Nuno 2026-09-18:
			# "quelque soit le build d'étapes, si toutes les étapes sont
			# validées [il faut] relancer le chargement des zones comme
			# avec le build complet") -- same one-time reload
			# _process_land_build_progress() already does at the end of a
			# full Build, so newly-produced content (a relit zone, a
			# rebuilt .rbank, a fresh ai_wmap...) actually shows up in the
			# 3D view instead of silently sitting on disk unseen. Evaluated
			# exactly once per rebuild (`reloaded` flips to True here
			# whether or not the continent turns out fully validated) --
			# most isolated rebuilds leave OTHER steps still stale, which
			# is expected and shouldn't reload on every single frame
			# forever until the continent happens to become fully built by
			# some later, unrelated rebuild.
			rebuild_progress["reloaded"] = True
			if all(status.up_to_date and status.warning is None for status in statuses):
				self._load_continent()
		table_flags = (
			imgui.TableFlags_.borders.value | imgui.TableFlags_.row_bg.value
			| imgui.TableFlags_.sizing_stretch_prop.value
		)
		if not imgui.begin_table("##pipeline-steps", 4, table_flags):
			return
		imgui.table_setup_column("Step")
		imgui.table_setup_column("Status")
		imgui.table_setup_column("", flags=imgui.TableColumnFlags_.width_fixed.value)
		imgui.table_setup_column("", flags=imgui.TableColumnFlags_.width_fixed.value)
		imgui.table_headers_row()
		# One table ROW per [rebuild] button (project-todos/forgery/
		# landscape_editor__pacs_export.md step 7l, Nuno 2026-09-18: "garde
		# la cellule mais fais 3 lignes") -- a grouped step's `group_size`
		# (>1 only on the group's first/representative entry, the one with
		# `has_own_button`) pulls in that many consecutive statuses and
		# draws one line per member in the Step/Status cells, instead of a
		# separate table row each; the [rebuild] button column still shows
		# only the group's single button. A standalone step (group_size ==
		# 1) draws exactly like before, just going through the same path.
		i = 0
		while i < len(statuses):
			status = statuses[i]
			if not status.has_own_button:
				i += 1
				continue
			group_members = statuses[i:i + status.group_size]
			i += status.group_size
			imgui.table_next_row()
			imgui.table_next_column()
			for member in group_members:
				imgui.text(f"{member.index + 1}. {member.short_name}")
				if imgui.is_item_hovered():
					imgui.set_tooltip(f"Tool: {member.tool}")
			imgui.table_next_column()
			for member in group_members:
				# "built/total" text (project-todos/forgery/
				# pipeline_export_dirs_rename.md step 15c, Nuno 2026-09-19:
				# "5/35 veut dire qu'il y a en sortie 5 fichier sur 35
				# requis") next to the tick -- same line, same color.
				count_suffix = f" {member.built_count}/{member.total_count}" if member.total_count is not None else ""
				if not member.can_build:
					imgui.text_colored(_TICK_NEUTRAL_COLOR, f"{fa_icons.ICON_FA_LOCK}{count_suffix}")
					if imgui.is_item_hovered():
						imgui.set_tooltip(member.blocked_reason or "Missing input")
				elif member.warning is not None:
					imgui.text_colored(_TICK_WARNING_COLOR, f"{fa_icons.ICON_FA_EXCLAMATION_CIRCLE}{count_suffix}")
					if imgui.is_item_hovered():
						imgui.set_tooltip(member.warning)
				elif member.up_to_date is None:
					imgui.text_colored(_TICK_NEUTRAL_COLOR, f"{fa_icons.ICON_FA_CIRCLE}{count_suffix}")
				elif member.up_to_date:
					imgui.text_colored(_TICK_OK_COLOR, f"{fa_icons.ICON_FA_CIRCLE_CHECK}{count_suffix}")
				else:
					imgui.text_colored(_TICK_STALE_COLOR, f"{fa_icons.ICON_FA_CIRCLE_XMARK}{count_suffix}")
			imgui.table_next_column()
			# Open-folder button (project-todos/forgery/pipeline_export_dirs_
			# rename.md step 15b, Nuno 2026-09-19) -- one per MEMBER line,
			# unlike the shared [rebuild] button below: each grouped step
			# still has its own real destination directory. Disabled (not
			# hidden) when that directory doesn't exist yet, e.g. the step
			# was never run.
			for member in group_members:
				imgui.push_id(f"folder-{member.index}")
				exists = member.destination_dir is not None and member.destination_dir.is_dir()
				imgui.begin_disabled(not exists)
				if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, f"Open folder: {member.destination_dir}", square=True):
					workspaces.open_in_system_file_manager(member.destination_dir)
				imgui.end_disabled()
				imgui.pop_id()
			imgui.table_next_column()
			if status.group_size > 1:
				# Vertically centers the group's single button against its
				# `group_size` stacked lines (Nuno 2026-09-18: "centrer les
				# boutons") -- a plain vertical offset before drawing it,
				# same trick as everywhere else in Forgery that centers a
				# fixed-size widget against a taller block of text.
				offset = (status.group_size * imgui.get_text_line_height_with_spacing() - imgui.get_frame_height()) / 2.0
				if offset > 0:
					imgui.dummy((0, offset))
			imgui.push_id(str(status.index))
			# Frozen (project-todos/forgery/pipeline_export_dirs_rename.md
			# step 15c, Nuno 2026-09-19) when ANY member of this row can't
			# build yet (its own real input is missing) -- same condition
			# `run_single_step()` itself would raise `LandBuildError` on, so
			# the button never lets the user hit that error after the fact.
			group_can_build = all(m.can_build for m in group_members)
			imgui.begin_disabled(rebuild_running or build_running or not group_can_build)
			if _icon_button(fa_icons.ICON_FA_ROTATE, f"Rebuild now: {status.name}", square=True):
				self._start_pipeline_step_rebuild(status.index, status.name)
			# Tooltip checked INSIDE the disabled block (Dear ImGui's own
			# documented pattern for explaining why a disabled control is
			# disabled) -- IsItemHovered() still reports the disabled button
			# here, not after end_disabled().
			if not group_can_build and imgui.is_item_hovered():
				blocked_reasons = " / ".join(sorted({m.blocked_reason for m in group_members if m.blocked_reason}))
				imgui.set_tooltip(blocked_reasons or "Missing input")
			imgui.end_disabled()
			imgui.pop_id()
		imgui.end_table()
		if self._pipeline_force_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._pipeline_force_error)

	def _draw_land_build_progress_popup(self):
		"""Modal popup for the structured progress of EITHER a full Build OR
		an isolated [rebuild] (project-todos/forgery/landscape_editor__
		pacs_export.md steps 7b-7d, 7n corrected per Nuno 2026-09-18: "je
		veux les 2 builds en modal, rien du tout dans le tableau") -- the two
		are mutually exclusive by construction (each _start_*() refuses to
		start while the other is running), so at most one ever needs
		showing here; blocks the rest of Atyscape while either is running
		(Nuno, 2026-09-17 -- no interaction possible with anything else
		until it's done). Opened by _start_land_build()/
		_start_pipeline_step_rebuild(); closes itself automatically on
		success, but stays open with a "Fermer" button on error so the
		message remains readable instead of vanishing the instant the
		thread finishes. The actual imgui.open_popup() call is deferred
		here (project-todos/forgery/landscape_editor__pacs_export.md step
		7n bugfix, Nuno 2026-09-18: "aucune popup de visible") -- this is
		the only place guaranteed to run at the same top-level ID-stack
		depth as begin_popup_modal() below it, whichever button (Build,
		nested deep inside "Pipeline steps"'s table, or the top-level
		[rebuild]) actually requested it via self._pending_popup_open."""
		if self._pending_popup_open:
			imgui.open_popup(_BUILD_PROGRESS_POPUP_ID)
			self._pending_popup_open = False
		if imgui.is_popup_open(_BUILD_PROGRESS_POPUP_ID):
			center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_BUILD_PROGRESS_POPUP_ID, None, flags)
		if not opened:
			return
		build_progress = self._land_build_progress
		rebuild_progress = self._pipeline_step_progress
		# Priority: whichever is ACTIVELY RUNNING wins first, regardless of
		# the other's past state -- only once NEITHER is running do we fall
		# back to showing a lingering error. Checking "done and error is
		# None" as the single condition (as a lone Build-only popup could)
		# is NOT enough with two sources: a past Build's own stale error
		# (done=True, error set, popup already dismissed once) would
		# otherwise outrank a brand new isolated rebuild actively running
		# right now, showing the wrong, outdated message (found by
		# simulating every state combination before shipping this, not
		# guessed).
		if build_progress is not None and not build_progress["done"]:
			source, label = build_progress, f"Step {build_progress['step_index']}/{build_progress['step_count']}: {build_progress['step_name']}"
		elif rebuild_progress is not None and not rebuild_progress["done"]:
			source, label = rebuild_progress, f"Rebuilding: {rebuild_progress['step_name']}"
		elif build_progress is not None and build_progress["error"] is not None:
			source, label = build_progress, f"Step {build_progress['step_index']}/{build_progress['step_count']}: {build_progress['step_name']}"
		elif rebuild_progress is not None and rebuild_progress["error"] is not None:
			source, label = rebuild_progress, f"Rebuilding: {rebuild_progress['step_name']}"
		else:
			imgui.close_current_popup()
			imgui.end_popup()
			return
		step_label = label
		fraction = source["fraction"]
		error = source["error"]
		imgui.text(step_label)
		if fraction is None:
			# No real sub-progress available for this stage (a single
			# native-tool call with no per-item feedback, e.g. land_export
			# or zone_dependencies) -- animated back-and-forth bar instead
			# of a frozen one, so it's visually clear the Build hasn't
			# hung (Nuno, 2026-09-17).
			imgui.progress_bar(_indeterminate_progress_fraction(), overlay="...")
		else:
			imgui.progress_bar(fraction, overlay=f"{fraction * 100:.0f}%")
		if error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), error)
			if imgui.button("Fermer"):
				imgui.close_current_popup()
		imgui.end_popup()
