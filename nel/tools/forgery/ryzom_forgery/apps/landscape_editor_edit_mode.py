"""LandscapeEditorApp mixin: everything specific to Edition mode (project-todos/
forgery/landscape_editor__land_preview.md step 5) -- continent list read
straight from `ryzom-data`'s own `ryzom.world`, the `.land`+brick fallback
for zones/cells never yet exported by the pipeline, the cursor status's
`.land`-assigned brick name, and the "Generate missing .zonew" button
(`[WELD]`, meaningless in Visualisation mode since `ryzom_data_path` is
always `None` there -- see `_load_continent()`, `landscape_editor.py`).

Imports from `landscape_editor_modes.py`, NOT from `landscape_editor.py`
itself -- see that module's own docstring for why (circular import).
"""

import math
import threading
from pathlib import Path

from pynel import repository_paths
from pynel.ryzom_land import load_land, LandParseError, STRING_UNUSED
from pynel.ryzom_zone import load_zone, ZoneParseError

from ryzom_forgery import continent_ecosystem
from ryzom_forgery import continent_selector
from ryzom_forgery.continent_geom_cache import ZoneManifestEntry
from ryzom_forgery.error_log import report_error
from ryzom_forgery import land_loader
from ryzom_forgery.land_geometry import (
	brick_size_in_cells_from_half_size, find_missing_land_cells, piece_origin, transform_zone_cache_data,
)
from ryzom_forgery.apps.landscape_editor_modes import _MODE_REAL_EXTENSIONS
from ryzom_forgery import settings as app_settings
from ryzom_forgery import zone_tools
from ryzom_forgery.region_loader import ZoneRef
from ryzom_forgery.zone_cache import read_zone_cache, write_zone_cache
from ryzom_forgery.zone_geometry import ZONE_CELL_SIZE, zone_to_cache_data


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
		landscape_editor__land_preview.md step 3, extended to `[WELD]` by
		landscape_editor__zone_render_modes.md step 9), cheap enough to
		resolve eagerly on the main thread. Only ever called from
		`_apply_render_mode()` (landscape_editor.py) once already gated on
		Edition mode + a compatible render mode + a selected continent -- see
		its own docstring for why the actual missing-cell computation happens
		later, in the background thread."""
		ryzom_data_path = repository_paths.get("ryzom-data")
		land_path = land_loader.find_land_files(ryzom_data_path).get(self.selected_continent_name)
		ecosystem_name = continent_ecosystem.get_ecosystem_for_continent(
			self._selected_continent_pipeline_name or self.selected_continent_name
		)
		if land_path is None or not ecosystem_name:
			return None
		return land_path, Path(ryzom_data_path) / "pipeline" / "landscape" / ecosystem_name / "zones"

	def _load_land_fallback_pieces(self, land_path, brick_zones_dir, zones, manifest_zones, progress, failed):
		"""Background-thread body for the `.land`+brick fallback (called from
		`_run_load_refs()`, landscape_editor.py, only when `land_fallback` is
		not None -- i.e. always from Edition mode). Mutates `zones`/
		`manifest_zones` (dicts) and `failed` (list) in place, and adds to
		`progress["gray"]` -- same reasoning as `_run_load_continent()`'s own
		docstring for why background-thread code only ever writes to shared
		mutable structures, never touches Panda3D."""
		try:
			land = load_land(land_path)
			# The `.land` file's own layout (which cell uses which
			# brick/rotation/flip) can change without any individual
			# brick's own file changing -- a single whole-file stamp
			# here invalidates every "land:x:y" manifest entry at once
			# whenever that happens, rather than trying to detect a
			# reassigned rotation/brick per cell (project-todos/
			# forgery/geomnode_continent_cache.md step 2).
			land_stat = land_path.stat()
			manifest_zones["__land_file__"] = ZoneManifestEntry(".land_file", land_stat.st_mtime, land_stat.st_size)
			used_cells = sum(1 for unit in land.zones if unit.zone_name != STRING_UNUSED)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor_edit_mode.py:_load_land_fallback_pieces) "
			      f"land loaded: {land_path} used_cells={used_cells} exported_zones={len(zones)}")
			# floor(), not round(): a cell's bb_center sits at exactly
			# (pos+0.5)*ZONE_CELL_SIZE, and round()'s round-half-to-even
			# would map that .5 inconsistently depending on pos's
			# parity -- floor(pos+0.5) always recovers pos exactly.
			existing_cells = {
				(math.floor(cache_data.bb_center[0] / ZONE_CELL_SIZE), math.floor(cache_data.bb_center[1] / ZONE_CELL_SIZE))
				for cache_data in zones.values()
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
			for (pos_x, pos_y), unit in find_missing_land_cells(land, existing_cells).items():
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
				progress["gray"].add(cell_name)
				# ".land" is never a real zone extension (.zone/.zonew/
				# .zonel) -- unambiguous marker that this manifest entry
				# is a placed brick, not an exported zone, and its
				# staleness stamp is the brick file's, not a ZoneRef's.
				try:
					brick_stat = brick_path.stat()
					manifest_zones[cell_name] = ZoneManifestEntry(".land", brick_stat.st_mtime, brick_stat.st_size)
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

	def _generate_missing_zonew(self):
		"""Starts a background weld pass (project-todos/forgery/
		landscape_editor__zone_render_modes.md step 9) over every currently
		loaded zone missing a `.zonew`/`.zonel` -- one native `zone_welder`
		call per zone (zone_tools.run_zone_welder()), each result persisted
		straight to `<ryzom-data>/pipeline/export/continents/<continent>/
		zone_weld/<name>.zonew` (zone_tools.missing_zonew_dest()). Each
		welded zone is queued on `progress["ready"]` as it completes so
		draw_panel() can display it immediately (live, one zone at a time)
		instead of waiting for the whole batch. A second call while one is
		already running is ignored, same as _load_continent()."""
		if self._weld_generate_progress is not None and not self._weld_generate_progress["done"]:
			return
		real_exts = _MODE_REAL_EXTENSIONS.get(self.render_mode)
		if real_exts is None:
			return
		missing_refs = [
			self._loaded_refs[name] for name, ext_map in self._loaded_extensions.items()
			if name in self._loaded_refs and not any(ext in ext_map for ext in real_exts)
		]
		# Cells with no real .zone at all (project-todos/forgery/
		# landscape_editor__zone_render_modes.md step 9, Nuno 2026-09-11) --
		# nothing zone_welder can work on, always reported as blocked rather
		# than attempted (land_export/land_composition territory, see
		# _run_generate_missing_zonew()'s own docstring).
		land_missing_names = sorted(self._land_missing_cells)
		if not missing_refs and not land_missing_names:
			return
		self._zone_error = None
		live_data_path = app_settings.load().live_data_path
		ryzom_data_path = repository_paths.get("ryzom-data")
		pipeline_continent_name = self._selected_continent_pipeline_name
		if missing_refs and not pipeline_continent_name:
			self._zone_error = "No pipeline continent selected -- cannot place generated .zonew files."
			report_error(self._zone_error)
			return
		progress = {
			"done": False, "error": None,
			"total": len(missing_refs) + len(land_missing_names), "processed": 0, "ready": [],
		}
		self._weld_generate_progress = progress
		thread = threading.Thread(
			target=self._run_generate_missing_zonew,
			args=(missing_refs, land_missing_names, live_data_path, ryzom_data_path, pipeline_continent_name, progress),
			daemon=True,
		)
		thread.start()

	def _run_generate_missing_zonew(
		self, refs, land_missing_names, live_data_path, ryzom_data_path, pipeline_continent_name, progress,
	):
		"""Background-thread body for _generate_missing_zonew() -- calls the
		native `zone_welder` once per zone in `refs` and persists each result
		(see zone_tools.run_zone_welder()'s own `persist_to` docstring),
		appending `(name, ZoneCacheData, new_ref)` to `progress["ready"]` for
		each success so draw_panel() can pick it up on its next frame --
		`new_ref` is a loose-file ZoneRef pointing at the just-written
		`.zonew`, for draw_panel() to merge into self._loaded_extensions.

		`land_missing_names` (this chantier's step 9, Nuno 2026-09-11) are
		"land:x:y" cells with no real .zone anywhere -- producing one is
		`land_export`, tracked separately as
		project-todos/forgery/landscape_editor__land_composition.md step 4,
		itself blocked on project-todos/pynel/land_pipeline.md step 1. Never
		attempted here: each is reported as a clear, individual failure
		instead, without holding up the zones that CAN actually be welded.

		Writes only to `progress` (list.append()/dict field writes, safe
		under the GIL, same reasoning as _run_load_continent())."""
		failed = []
		for ref in refs:
			try:
				dest = zone_tools.missing_zonew_dest(ryzom_data_path, pipeline_continent_name, ref.name)
				zone = zone_tools.run_zone_welder(ref, live_data_path, persist_to=dest)
				new_ref = ZoneRef(name=ref.name, x=ref.x, y=ref.y, source_path=dest, bnp_entry=None)
				progress["ready"].append((ref.name, zone_to_cache_data(zone), new_ref))
			except zone_tools.ZoneToolError as exc:
				failed.append(str(exc))
			progress["processed"] += 1
		for cell_name in land_missing_names:
			_, pos_x, pos_y = cell_name.split(":")
			failed.append(
				f"cell ({pos_x}, {pos_y}): no .zone exported yet -- finish land_composition (land_export) first"
			)
			progress["processed"] += 1
		if failed:
			progress["error"] = f"{len(failed)}/{progress['total']} zone(s) failed to weld: {'; '.join(failed[:3])}"
			report_error(progress["error"])
		progress["done"] = True
