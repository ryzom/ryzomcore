"""Ryzom Forgery landscape editor, branded "Atyscape" (Nuno, 2026-09-07 --
same naming convention as object_editor.py/"Patina"): 3D landscape
composition/build tool, replacing the current 2D bitmap-tile Ligo editor.

See project-todos/forgery/landscape_editor.md for the planned progressive
rendering steps -- step 9 (detecting which of .zone/.zonew/.zonel already
exist for a given zone, generating the missing ones on demand) is the
current one.
"""

import threading
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, imgui_ctx

from pynel import repository_paths
from pynel.ryzom_bnp import BnpError
from pynel.ryzom_zone import parse_zone, ZoneParseError

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_begin_tab_item_with_icon, _icon_button, _pop_tab_color, _push_tab_color, _VIEWPORT_TOGGLE_MARGIN_PX,
)
from ryzom_forgery.camera import OrbitCamera
from ryzom_forgery import continent_selector
from ryzom_forgery import continent_ecosystem
from ryzom_forgery import live_data
from ryzom_forgery.live_data_setup_dialog import LiveDataSetupDialog
from ryzom_forgery import pipeline_data_installer
from ryzom_forgery.pipeline_data_install_dialog import PipelineDataInstallDialog
from ryzom_forgery.region_loader import (
	find_zones_in_region, get_zone_extensions_index, has_pipeline_export, load_zone_cache_data, RegionLoadError,
)
from ryzom_forgery.ryzom_paths_section import RyzomPathsSection
from ryzom_forgery import settings as app_settings
from ryzom_forgery.zone_geometry import build_zone_geom_from_cache, build_zone_grid_geom, zone_to_cache_data

# Explorer's own filter combo (see explorer.py's extension_filter/
# extension_presets). Default filter is "*" (unfiltered), NOT "*.land": a
# .bnp archive is only shown at all if either its own filename matches the
# active filter or at least one of its contents does (see explorer.py's
# _draw_dir_contents()) -- the real *_zones.bnp archives (where .zone/
# .zonew/.zonel actually live in a real Ryzom Live install) never contain a
# .land, so a "*.land" default filter hid every one of them entirely (found
# 2026-09-07, Nuno couldn't navigate into any .bnp at all).
_EXPLORER_FILTER_PRESETS = ["*", "*.land", "*.zone", "*.zonew", "*.zonel"]
_ZONE_EXTENSIONS = (".zone", ".zonew", ".zonel")

# Render mode bar (project-todos/forgery/landscape_editor__zone_render_modes.md
# step 6) -- WELD/LIGHT each accept one or more "real" extensions (a zone
# doesn't need its own standalone .zonew to count as welded for [WELD]: a
# shipped .zonel necessarily went through welding already, even when the
# intermediate .zonew was never kept -- real shipped live_data installs only
# ship the final pipeline stage, confirmed 2026-09-09, Nuno), falling back
# (see _resolve_zone_for_mode()) to a genuinely earlier stage, rendered as a
# purple->pink gradient (zone_geometry.build_zone_geom_from_cache(fallback=True) --
# a first grayscale attempt was indistinguishable from the viewport's own
# gray background, Nuno 2026-09-09) rather than the real elevation-colored
# one. POLY/2D have no entry in
# _MODE_REAL_EXTENSIONS -- they always show each zone's own default (highest-
# priority, region_loader.py's own .zonel > .zonew > .zone) ZoneRef, exactly
# the pre-chantier behavior.
_RENDER_MODES = ("2D", "POLY", "WELD", "LIGHT")
_MODE_REAL_EXTENSIONS = {"WELD": (".zonew", ".zonel"), "LIGHT": (".zonel",)}
_MODE_FALLBACK_EXTENSIONS = {"WELD": (".zone",), "LIGHT": (".zonew", ".zone")}


def _resolve_zone_for_mode(default_ref, ext_map, mode):
	"""Which ZoneRef to actually load for `mode`, and whether it's a real
	match (True) or a fallback (False, caller should render it gray) -- see
	_MODE_REAL_EXTENSIONS/_MODE_FALLBACK_EXTENSIONS module comment. POLY/2D
	(no real-extensions entry) always return `default_ref` unchanged."""
	real_exts = _MODE_REAL_EXTENSIONS.get(mode)
	if real_exts is None:
		return default_ref, True
	for ext in real_exts:
		if ext in ext_map:
			return ext_map[ext], True
	for fallback_ext in _MODE_FALLBACK_EXTENSIONS[mode]:
		if fallback_ext in ext_map:
			return ext_map[fallback_ext], False
	return None, False

# Global Visualisation/Edition mode (project-todos/forgery/
# landscape_editor__land_preview.md step 1) -- a single automatic switch for
# the whole app, not a per-continent/per-button choice like
# _resolve_zone_for_mode() above. "edition" whenever ryzom-data is configured
# and points at an existing directory (pynel.repository_paths.is_valid()),
# "visualisation" otherwise -- deliberately ignores whether that ryzom-data
# checkout actually has any .land in it (later steps filter the continent
# combo for that; an unconfigured/missing ryzom-data is the only thing that
# forces visualisation mode).
_MODE_VISUALISATION = "visualisation"
_MODE_EDITION = "edition"
_MODE_BADGE_COLOR = {_MODE_VISUALISATION: (0.4, 0.7, 1.0, 1.0), _MODE_EDITION: (0.4, 1.0, 0.4, 1.0)}
_MODE_BADGE_LABEL = {_MODE_VISUALISATION: "Visualisation", _MODE_EDITION: "Edition"}


def _detect_app_mode():
	return _MODE_EDITION if repository_paths.is_valid("ryzom-data") else _MODE_VISUALISATION


# draw_panel()'s tab bar (_push_tab_color()) -- same idea as
# object_editor.py's own _TAB_COLOR_* constants, one per tab so each reads
# as visually distinct at a glance.
_TAB_COLOR_LANDSCAPE = (0.729, 0.867, 0.635, 1.0)  # light olive green -- terrain
_TAB_COLOR_SETTINGS = (0.8, 0.75, 0.15, 1.0)  # yellow, same as object_editor.py's own Settings tab

# Renamed from APP_INFO to hide Atyscape from ryztart's app list while it's
# still being built (see project-todos/forgery/landscape_editor.md) --
# list_apps()/launch_app() (ryzom_forgery/__init__.py) only pick up a module
# whose APP_INFO is named exactly that. dev.sh (direct launch by file path)
# is unaffected. Rename back to APP_INFO once ready to ship.
_APP_INFO_HIDDEN = {
	"id": "landscape_editor",
	"name": "Atyscape",
	"subtitle": "Landscape Editor",
	"description": "3D landscape composition/build tool (.land/.zone).",
}


class LandscapeEditorApp(ForgeryApp):
	def __init__(self):
		settings = app_settings.load()
		# The real *_zones.bnp archives (.zone/.zonew/.zonel) and world/
		# continent.packed_sheets all live under live_data_path -- start
		# there when it's configured (same setting continent_selector.py
		# already reads), rather than an unrelated configured search path or
		# the user's home folder, which would need manual navigation to reach
		# the one place this app's data actually lives.
		if live_data.is_valid_live_data_path(settings.live_data_path):
			explorer_root = Path(settings.live_data_path)
		elif settings.search_paths:
			explorer_root = Path(settings.search_paths[0].path)
		else:
			explorer_root = Path.home()
		ForgeryApp.__init__(self, explorer_root=explorer_root, title="Ryzom Forgery - Atyscape")
		self.explorer.extension_presets = _EXPLORER_FILTER_PRESETS

		# OrbitCamera._update() (camera.py) reads both of these every frame --
		# there's no ObjectManipulator/target object here (unlike
		# object_editor.py, which sets these to drive its own navcube-driven
		# drag-mode overrides), so both stay at their "no override" default.
		self.forced_drag_mode = None
		self.target_mode = None

		# Landscape zones are 160 world units wide -- a much larger scale than
		# object_editor's single-shape inspection, so this starts zoomed out
		# much further than that app's own OrbitCamera default.
		self.orbit_camera = OrbitCamera(self, distance=200.0)
		# OrbitCamera's own default max_distance (2000.0) doesn't reach a
		# whole continent (dozens of 160-unit zones across) -- both manual
		# zoom-out and the auto-frame after a continent load (frame() clamps
		# to max_distance) need much more room. 3x wasn't enough either
		# (Nuno, 2026-09-08 -- "recule encore le zoom"), raised to 6x
		# (12000.0), still comfortably under the 20000.0 far clip below.
		self.orbit_camera.max_distance *= 6.0

		# ForgeryApp.__init__ (app.py) sets a near/far of (0.02, 20000.0) --
		# a 1,000,000:1 ratio meant for Patina's own close-up inspection of
		# tiny shape details. Atyscape never gets that close (zones are 160
		# units wide, orbit starts at 200), so that ratio only cost depth-
		# buffer precision here: flat Z=0 overlay geometry (e.g. the zone
		# grid) z-fought against terrain crossing Z=0, differently depending
		# on camera distance -- found by Nuno 2026-09-08 (rendering changed
		# one zoom level apart), confirmed fixed by raising near to 1.0
		# (keeping the same far), which drops the ratio to 20,000:1.
		self.camLens.set_near_far(1.0, 20000.0)

		# Settings tab state (project-todos/forgery/
		# landscape_editor__zone_render_modes__ryzom_paths_ui.md) --
		# RyzomPathsSection is a standalone module (not object_editor_mixins.
		# settings_dialogs' Patina-only mixin) precisely so it can be drawn
		# here too: Patina and Atyscape are independent apps with disjoint
		# audiences, each edits the settings it consumes itself, but
		# live_data_path/repository paths/ryzom_tools_path are generic
		# suite-wide settings, exposed in both. Own LiveDataSetupDialog
		# instance (not object_editor.py's) -- Atyscape never draws its
		# mandatory first-launch popup (only object_editor.py's draw_ui()
		# does that), just the Settings-tab folder picker.
		self.ryzom_paths_section = RyzomPathsSection(LiveDataSetupDialog())

		# Global Visualisation/Edition mode (project-todos/forgery/
		# landscape_editor__land_preview.md step 1) -- recomputed every
		# draw_panel() call (repository_paths.is_valid() is a cheap JSON
		# read, no change-notification mechanism exists on that module),
		# cached here only to log on an actual transition rather than
		# every frame.
		self._app_mode = None

		# Continent selector state (project-todos/forgery/
		# landscape_editor__continent-selector.md steps 3-4). cont_locs/
		# cont_locs_error are loaded lazily on the first draw_panel() call,
		# not here, since ShowBase.__init__ above hasn't opened a window yet
		# and this is pure file I/O better done once the UI can actually show
		# an error if it fails. selected_continent_name/continent_bounds are
		# None until a continent is actually picked; continent_bounds is
		# recomputed only when the selection changes (not every frame).
		self._cont_locs = None
		self._cont_locs_error = None
		self.selected_continent_name = None
		# ContLoc.selection_name for the current selection -- see
		# _select_continent()'s own docstring for why this differs from
		# selected_continent_name and what it's used for.
		self._selected_continent_pipeline_name = None
		self.continent_bounds = None
		self._bounds_error = None

		# Loaded-zone state (project-todos/forgery/landscape_editor.md steps
		# 3-6 -- Bezier-tessellated rendering, see zone_geometry.py). zones/
		# zone_nodes are keyed by bare zone name (e.g. "55_CC") so a whole-
		# continent load (step 6) and a single Explorer selection (step 3)
		# share the same rendering path -- both just populate these dicts
		# differently, see _set_loaded_zones(). self.zones values are always
		# zone_cache.ZoneCacheData (never a raw pynel Zone), converted via
		# zone_to_cache_data() as soon as a zone is parsed, so
		# _set_loaded_zones()/_frame_on_loaded_zones() only ever handle one
		# shape of data regardless of where it came from.
		self.zones = {}
		self._zone_nodes = {}
		self._zone_error = None
		self._zone_root = self.render.attach_new_node("zone-root")

		# Render mode state (project-todos/forgery/
		# landscape_editor__zone_render_modes.md step 6). _loaded_refs (name ->
		# default/POLY ZoneRef) and _loaded_extensions (name -> {ext: ZoneRef})
		# describe the currently loaded zone set (single Explorer selection or
		# a whole continent) independently of which extension is actually
		# displayed -- _apply_render_mode() re-resolves and reloads from these
		# every time self.render_mode changes, without re-scanning the disk
		# index. _gray_zones/_missing_for_mode are recomputed by the same
		# call, for _draw_render_mode_bar()'s own display.
		self.render_mode = "POLY"
		self._loaded_refs = {}
		self._loaded_extensions = {}
		self._gray_zones = set()
		self._missing_for_mode = 0
		self._mode_reload_progress = None

		# Zone-boundary grid overlay (project-todos/forgery/
		# landscape_editor.md step 7) -- rebuilt in _set_loaded_zones()
		# alongside the terrain itself, shown/hidden per self._grid_visible
		# (same show()/hide() pattern as object_editor.py's own floor grid
		# toggle). Visible by default. Toggled from the viewport icon bar
		# (_draw_viewport_toggles()), not a panel checkbox.
		self._grid_np = self.render.attach_new_node("zone-grid-placeholder")
		self._grid_visible = True
		# Same size-tracking trick as object_editor.py's own
		# _viewport_toggle_size -- the bar's true size isn't known until
		# after the first imgui_ctx.begin() below, so _draw_viewport_toggles()
		# positions itself off of last frame's captured size, seeded here
		# with a small placeholder for the very first frame.
		self._viewport_toggle_size = (10.0, 10.0)

		# Whole-continent loading state (project-todos/forgery/
		# landscape_editor__zone_disk_cache.md steps 5-6) -- None while idle,
		# else the dict a background thread (started by _load_continent())
		# is writing into, see _run_load_continent()/draw_panel()'s own
		# polling of it.
		self._continent_load_progress = None

		# Proposes downloading missing pipeline data (project-todos/forgery/
		# landscape_editor__zone_render_modes__pipeline_data_installer.md)
		# for the continent about to load, see _load_continent(). Actually
		# opened from draw_panel(), not _load_continent() itself -- see the
		# comment at its only assignment for why.
		self._pipeline_data_install_dialog = PipelineDataInstallDialog()
		self._pipeline_data_install_pending = None

	def on_selection_changed(self, items):
		"""A single .zone*/.bnp-contained zone picked in the Explorer -- loads
		exactly the clicked file, unaffected by self.render_mode. Not a real
		usage pattern (Nuno always loads a whole continent via the combo,
		never browses the Explorer for this, 2026-09-09) -- kept simple/
		direct rather than routed through _apply_render_mode()'s mode
		resolution, which only matters for continent loads."""
		if len(items) != 1 or items[0].suffix.lower() not in _ZONE_EXTENSIONS:
			return
		item = items[0]
		name = item.stem
		self._zone_error = None
		try:
			zone = parse_zone(item.read_bytes())
		except (OSError, BnpError, ZoneParseError) as exc:
			self._zone_error = f"Failed to load {item.name}: {exc}"
			self._set_loaded_zones({})
			return
		self._loaded_refs = {}
		self._loaded_extensions = {}
		self._set_loaded_zones({name: zone_to_cache_data(zone)})

	def _load_continent(self):
		"""Starts a background load of every real zone in the selected
		continent's bounds (project-todos/forgery/
		landscape_editor__zone_disk_cache.md steps 5-6) -- cache-first
		(region_loader.load_zone_cache_data()), replaces whatever was loaded
		before (single zone or a previous continent) once done, same as
		on_selection_changed(). Runs off the main thread (region_loader.py's
		zone index build scans every .bnp's header table under
		live_data_path the first time it's needed, real disk I/O that
		stalled the whole UI when run synchronously -- found 2026-09-07,
		Nuno). A second call while one is already running is ignored.
		Triggered automatically once a continent is selected
		(_select_continent()) and by the explicit "Build cache for this
		continent" button -- both do the exact same thing, per Nuno
		2026-09-08."""
		if self._continent_load_progress is not None and not self._continent_load_progress["done"]:
			return
		if self.continent_bounds is None:
			return
		self._zone_error = None
		live_data_path = app_settings.load().live_data_path
		ryzom_data_path = repository_paths.get("ryzom-data")
		pipeline_continent_name = self._selected_continent_pipeline_name

		# Propose downloading whatever pipeline data is missing for this
		# continent (never blocking -- the load below proceeds regardless,
		# project-todos/forgery/landscape_editor__zone_render_modes__
		# pipeline_data_installer.md step 5). Always the continent, its
		# ecosystem's export AND its ecosystem's raw landscape zones
		# together when missing: `landscape/<eco>/zones/` (raw .zone bricks
		# + .zoneligo) is the only starting material to compose a continent
		# from its .land when no continent-specific .zone has been
		# generated yet (land_export, not wired here -- land_composition
		# chantier) -- one is as useless as the others without it in
		# practice (Nuno 2026-09-09).
		if pipeline_continent_name:
			missing = []
			if not pipeline_data_installer.is_installed("pipeline_continents", pipeline_continent_name):
				missing.append(("pipeline_continents", pipeline_continent_name))
			ecosystem_name = continent_ecosystem.get_ecosystem_for_continent(pipeline_continent_name)
			if ecosystem_name:
				if not pipeline_data_installer.is_installed("pipeline_ecosystems", ecosystem_name):
					missing.append(("pipeline_ecosystems", ecosystem_name))
				if not pipeline_data_installer.is_installed("landscape", ecosystem_name):
					missing.append(("landscape", ecosystem_name))
			if missing:
				# Deferred to the next draw_panel() (outside any
				# begin_combo()/end_combo() block) rather than opened here --
				# _load_continent() runs from inside the continent combo's
				# own popup (see _select_continent()'s call site), and
				# calling imgui.open_popup() for a different popup while
				# still inside another popup's Begin/End is a classic ImGui
				# trap: the new popup's open request is silently lost when
				# the combo closes (found 2026-09-09, Nuno: no popup ever
				# appeared despite `missing` being correctly non-empty).
				self._pipeline_data_install_pending = missing

		# A continent with a real pipeline export (region_loader.py's
		# has_pipeline_export()) never reads live_data_path at all, so it's
		# never required to be configured for that continent (project-todos/
		# forgery/landscape_editor__zone_render_modes.md step 6).
		if not has_pipeline_export(ryzom_data_path, pipeline_continent_name):
			if not live_data.is_valid_live_data_path(live_data_path):
				self._zone_error = "Ryzom Live data path not configured -- set it in Patina's Settings tab (Paths)."
				return

		progress = {"done": False, "error": None, "default_refs": {}, "extensions": {}}
		self._continent_load_progress = progress
		thread = threading.Thread(
			target=self._run_load_continent,
			args=(live_data_path, ryzom_data_path, pipeline_continent_name, self.continent_bounds, progress),
			daemon=True,
		)
		thread.start()

	def _run_load_continent(self, live_data_path, ryzom_data_path, pipeline_continent_name, bounds, progress):
		"""Background-thread body for _load_continent() -- just the disk-index
		lookups (region_loader.py's own cached zone index/extensions index,
		real disk I/O the first time it's needed for `live_data_path`, or a
		fresh uncached scan of `ryzom_data_path`'s pipeline export if
		`continent_name` has one -- see region_loader.get_zone_index()'s own
		docstring), never the actual per-zone tessellation -- draw_panel()'s
		polling of `progress` hands the result to _apply_render_mode()
		(project-todos/forgery/landscape_editor__zone_render_modes.md step 6),
		which does the expensive per-zone load itself, in its own background
		pass, for whichever extension self.render_mode currently wants.
		Writes only to `progress` (plain dict field writes, safe under the
		GIL, same reasoning as object_editor.py's creature-cache-rebuild
		background thread)."""
		try:
			min_x, min_y, max_x, max_y = bounds
			refs = find_zones_in_region(
				live_data_path, min_x, min_y, max_x, max_y, pipeline_continent_name, ryzom_data_path,
			)
			if not refs:
				progress["error"] = "No real zone found for this continent."
				return
			progress["default_refs"] = {ref.name: ref for ref in refs}
			full_extensions_index = get_zone_extensions_index(live_data_path, pipeline_continent_name, ryzom_data_path)
			progress["extensions"] = {ref.name: full_extensions_index.get(ref.name, {}) for ref in refs}
		except OSError as exc:
			progress["error"] = str(exc)
		finally:
			progress["done"] = True

	def _apply_render_mode(self):
		"""Re-resolves the currently loaded zone set (self._loaded_refs/
		self._loaded_extensions, populated by on_selection_changed()/
		_load_continent()) for self.render_mode and reloads it in a
		background thread (project-todos/forgery/
		landscape_editor__zone_render_modes.md step 6) -- POLY/2D always
		resolve to each zone's own default ref (_resolve_zone_for_mode()),
		WELD/LIGHT their own extension or a gray fallback. A second call
		while one reload is already running is ignored (the caller will get
		another chance once it's done, since render_mode/loaded_refs/
		loaded_extensions are read fresh here, not snapshotted)."""
		if not self._loaded_refs:
			self._gray_zones = set()
			self._missing_for_mode = 0
			return
		if self._mode_reload_progress is not None and not self._mode_reload_progress["done"]:
			return

		real_exts = _MODE_REAL_EXTENSIONS.get(self.render_mode)
		if real_exts is not None:
			self._missing_for_mode = sum(
				1 for name, ext_map in self._loaded_extensions.items()
				if not any(ext in ext_map for ext in real_exts)
			)
		else:
			self._missing_for_mode = 0

		refs = {}
		gray = set()
		for name, default_ref in self._loaded_refs.items():
			ext_map = self._loaded_extensions.get(name, {})
			ref, up_to_date = _resolve_zone_for_mode(default_ref, ext_map, self.render_mode)
			if ref is None:
				continue
			refs[name] = ref
			if not up_to_date:
				gray.add(name)

		print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_apply_render_mode) mode={self.render_mode} loaded={len(self._loaded_refs)} resolved={len(refs)} gray={sorted(gray)} missing_for_mode={self._missing_for_mode}")

		self._zone_error = None
		progress = {"done": False, "error": None, "zones": {}, "gray": gray, "total": len(refs), "processed": 0}
		self._mode_reload_progress = progress
		thread = threading.Thread(target=self._run_load_refs, args=(refs, progress), daemon=True)
		thread.start()

	def _run_load_refs(self, refs, progress):
		"""Background-thread body for _apply_render_mode() -- pure file I/O/
		pickle (region_loader.py/zone_cache.py load_zone_cache_data(), same
		cache-first behavior as the old _run_load_continent()), never touches
		Panda3D. Writes only to `progress`, see _run_load_continent()'s own
		docstring for why."""
		try:
			zones = {}
			failed = []
			for name, ref in refs.items():
				try:
					zones[name] = load_zone_cache_data(ref)
				except RegionLoadError as exc:
					failed.append(str(exc))
				progress["processed"] += 1
			progress["zones"] = zones
			if failed:
				progress["error"] = f"{len(failed)}/{len(refs)} zone(s) failed to load: {'; '.join(failed[:3])}"
		except OSError as exc:
			progress["error"] = str(exc)
		finally:
			progress["done"] = True

	def _set_loaded_zones(self, zones, gray=None):
		"""Tears down whatever geometry was attached before and builds fresh
		geometry for `zones` (name -> zone_cache.ZoneCacheData) -- colored by
		elevation over the combined Z range of every zone in `zones`, not
		each zone's own (see build_zone_geom_from_cache()'s own docstring)
		-- then reframes the camera on their combined bounding box. Shared by
		on_selection_changed() (a single zone) and _load_continent()
		(a whole continent), both via _apply_render_mode()/_run_load_refs().

		`gray` (zone names, project-todos/forgery/
		landscape_editor__zone_render_modes.md step 6) are rendered flat gray
		instead of the elevation gradient -- WELD/LIGHT zones shown via a
		fallback extension, not actually welded/lit."""
		self._gray_zones = gray or set()
		for node in self._zone_nodes.values():
			node.remove_node()
		self._zone_nodes = {}
		self.zones = zones

		# Elevation color spans the whole set of loaded zones, not each
		# zone's own tiny Z range -- every zone re-normalizing to the same
		# green gradient on its own relief made a whole continent look like
		# a patchwork of disconnected tiles at zone boundaries (found by
		# Nuno 2026-09-08 testing a real continent).
		self._grid_np.remove_node()
		if zones:
			min_x = min(cache_data.bb_center[0] - cache_data.bb_half_size[0] for cache_data in zones.values())
			min_y = min(cache_data.bb_center[1] - cache_data.bb_half_size[1] for cache_data in zones.values())
			min_z = min(cache_data.bb_center[2] - cache_data.bb_half_size[2] for cache_data in zones.values())
			max_x = max(cache_data.bb_center[0] + cache_data.bb_half_size[0] for cache_data in zones.values())
			max_y = max(cache_data.bb_center[1] + cache_data.bb_half_size[1] for cache_data in zones.values())
			max_z = max(cache_data.bb_center[2] + cache_data.bb_half_size[2] for cache_data in zones.values())
			self._grid_np = self.render.attach_new_node(build_zone_grid_geom(min_x, min_y, max_x, max_y))
			self._grid_np.set_light_off()
			# Always drawn on top, regardless of terrain depth -- flat at
			# Z=0 (sea level), it would otherwise be buried under any
			# terrain above sea level or invisible behind terrain below it.
			# It's a navigational reference overlay, not real geometry, so
			# skipping the depth test/write (and forcing it into a bin
			# rendered after everything else) is the right call here, per
			# Nuno 2026-09-08.
			self._grid_np.set_depth_test(False)
			self._grid_np.set_depth_write(False)
			self._grid_np.set_bin("fixed", 100)
			if not self._grid_visible:
				self._grid_np.hide()
		else:
			self._grid_np = self.render.attach_new_node("zone-grid-placeholder")

		for name, cache_data in zones.items():
			node = build_zone_geom_from_cache(cache_data, min_z=min_z, max_z=max_z, fallback=name in self._gray_zones)
			node_path = self._zone_root.attach_new_node(node)
			# Winding isn't guaranteed to match Panda3D's expected front-face
			# direction (zone_geometry.py's grid triangulation follows NeL's
			# own Bezier control-point convention) -- two-sided so every
			# patch is visible regardless, rather than risking half the
			# terrain silently backface-culled.
			node_path.set_two_sided(True)
			self._zone_nodes[name] = node_path

		if zones:
			self._frame_on_loaded_zones()

	def _frame_on_loaded_zones(self):
		bbs = [(cache_data.bb_center, cache_data.bb_half_size) for cache_data in self.zones.values()]
		min_x = min(center[0] - half[0] for center, half in bbs)
		min_y = min(center[1] - half[1] for center, half in bbs)
		min_z = min(center[2] - half[2] for center, half in bbs)
		max_x = max(center[0] + half[0] for center, half in bbs)
		max_y = max(center[1] + half[1] for center, half in bbs)
		max_z = max(center[2] + half[2] for center, half in bbs)
		center = ((min_x + max_x) / 2.0, (min_y + max_y) / 2.0, (min_z + max_z) / 2.0)
		frame_distance = max(max_x - min_x, max_y - min_y, max_z - min_z, 10.0) * 1.5
		self.orbit_camera.frame(center, frame_distance)

	def _ensure_continent_locations_loaded(self):
		if self._cont_locs is not None or self._cont_locs_error is not None:
			return
		live_data_path = app_settings.load().live_data_path
		if not live_data.is_valid_live_data_path(live_data_path):
			self._cont_locs_error = (
				"Ryzom Live data path not configured -- set it in Patina's "
				"Settings tab (Paths), it's a shared setting."
			)
			return
		try:
			self._cont_locs = continent_selector.load_continent_locations(live_data_path)
		except (OSError, continent_selector.ContinentSelectorError) as exc:
			self._cont_locs_error = f"Failed to load continent list: {exc}"

	def _select_continent(self, continent_name, selection_name):
		"""Resolves and caches continent_bounds for `continent_name` -- called
		only when the combo selection actually changes, not every frame (both
		files this reads are full-parse, no random access, see
		nel/tools/pynel/docs/packed_sheets.md). Immediately starts loading
		the whole continent (project-todos/forgery/
		landscape_editor__zone_disk_cache.md step 6, per Nuno 2026-09-08:
		never stream by region around the camera, load a continent whole as
		soon as it's selected).

		`continent_name` (ContLoc's own internal sheet stem, e.g.
		"lecarrefour") and `selection_name` (the combo's displayed label,
		e.g. "nexus") can genuinely differ -- confirmed 2026-09-09, Nuno:
		`continent_name` only names the leveldesign/world/<continent_name>
		project files, while a real build_gamedata pipeline export directory
		under ryzom-data/pipeline/export/continents/ is named after
		`selection_name` instead. self.selected_continent_name (bounds
		resolution) keeps the former; self._selected_continent_pipeline_name
		(region_loader.py's pipeline-export lookup, step 6) uses the
		latter."""
		self.selected_continent_name = continent_name
		self._selected_continent_pipeline_name = selection_name
		self.continent_bounds = None
		self._bounds_error = None
		live_data_path = app_settings.load().live_data_path
		try:
			self.continent_bounds = continent_selector.resolve_continent_bounds(live_data_path, continent_name)
		except (OSError, continent_selector.ContinentSelectorError) as exc:
			self._bounds_error = f"Failed to resolve bounds: {exc}"
			return
		self._load_continent()

	def _toggle_grid(self):
		self._grid_visible = not self._grid_visible
		(self._grid_np.show if self._grid_visible else self._grid_np.hide)()

	def _draw_viewport_toggles(self):
		"""Small floating icon-button bar bottom-left of the 3D viewport
		(zone grid) -- EXACT same positioning/sizing as object_editor.py's
		own _draw_viewport_toggles() (viewport_transform.py): same
		explorer_width/sysinfo_height/_VIEWPORT_TOGGLE_MARGIN_PX-based
		position, same _viewport_toggle_size self-measurement trick (true
		size only known after the window is drawn, so this frame positions
		off last frame's captured size), same large_icon_font. Found
		misaligned when it computed y off imgui.get_frame_height() instead
		of the real captured window height -- Nuno 2026-09-08."""
		display_size = imgui.get_io().display_size
		win_h = display_size.y
		if win_h <= 0:
			return

		width, height = self._viewport_toggle_size
		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - height
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##viewport-toggles", flags=flags):
			if _icon_button(fa_icons.ICON_FA_TABLE, "Show zone grid", self._grid_visible, square=True,
			                large_font=large_font):
				self._toggle_grid()
			self._viewport_toggle_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def panel_title(self):
		return "Landscape Editor"

	def draw_panel(self):
		mode = _detect_app_mode()
		self._app_mode = mode
		imgui.text("Mode:")
		imgui.same_line()
		imgui.text_colored(_MODE_BADGE_COLOR[mode], _MODE_BADGE_LABEL[mode])
		imgui.separator()

		# _draw_viewport_toggles() opens its own separate floating imgui
		# window, independent of the tab bar below -- always drawn
		# regardless of which tab is active, never hidden by switching to
		# Settings (project-todos/forgery/
		# landscape_editor__zone_render_modes__ryzom_paths_ui.md step 5).
		self._draw_viewport_toggles()
		if self._pipeline_data_install_pending is not None:
			self._pipeline_data_install_dialog.open(self._pipeline_data_install_pending)
			self._pipeline_data_install_pending = None
		self._pipeline_data_install_dialog.draw()

		if imgui.begin_tab_bar("##panel-tabs"):
			_push_tab_color(_TAB_COLOR_LANDSCAPE)
			if _begin_tab_item_with_icon(fa_icons.ICON_FA_MOUNTAIN, "Landscape"):
				self._draw_landscape_tab()
				imgui.end_tab_item()
			_pop_tab_color()
			_push_tab_color(_TAB_COLOR_SETTINGS)
			if _begin_tab_item_with_icon(fa_icons.ICON_FA_GEAR, "Settings"):
				self.ryzom_paths_section.draw(self)
				imgui.end_tab_item()
			_pop_tab_color()
			imgui.end_tab_bar()

	def _draw_landscape_tab(self):
		self._ensure_continent_locations_loaded()

		if self._cont_locs_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._cont_locs_error)
			return

		imgui.text("Continent")
		current_label = "(none selected)"
		if self.selected_continent_name is not None:
			for cont_loc in self._cont_locs:
				if cont_loc.continent_name == self.selected_continent_name:
					current_label = cont_loc.selection_name
					break
		if imgui.begin_combo("##continent_selector", current_label):
			for cont_loc in self._cont_locs:
				selected = cont_loc.continent_name == self.selected_continent_name
				clicked, _ = imgui.selectable(cont_loc.selection_name, selected)
				if clicked:
					self._select_continent(cont_loc.continent_name, cont_loc.selection_name)
				if selected:
					imgui.set_item_default_focus()
			imgui.end_combo()

		if self._bounds_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._bounds_error)
		elif self.continent_bounds is not None:
			min_x, min_y, max_x, max_y = self.continent_bounds
			imgui.text(f"Bounds: X [{min_x:.0f} .. {max_x:.0f}]  Y [{min_y:.0f} .. {max_y:.0f}]")
			if imgui.button("Center camera here"):
				center = ((min_x + max_x) / 2.0, (min_y + max_y) / 2.0, 0.0)
				self.orbit_camera.frame(center, max(max_x - min_x, max_y - min_y) * 0.75)

		imgui.separator()
		imgui.text("Zone (select a .zone/.zonew/.zonel in the Explorer,")
		imgui.text("or pick a continent above to load it whole)")
		if imgui.button("Build cache for this continent"):
			self._load_continent()

		if self._continent_load_progress is not None:
			progress = self._continent_load_progress
			if progress["done"]:
				self._continent_load_progress = None
				print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:draw_panel) continent scan done, error={progress['error']!r} zones={len(progress['default_refs'])}")
				if progress["error"]:
					self._zone_error = progress["error"]
				else:
					self._loaded_refs = progress["default_refs"]
					self._loaded_extensions = progress["extensions"]
					self._apply_render_mode()
			else:
				imgui.text("Scanning zone index...")

		if self._mode_reload_progress is not None:
			progress = self._mode_reload_progress
			if progress["done"]:
				self._mode_reload_progress = None
				print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:draw_panel) mode reload done, error={progress['error']!r} zones={len(progress['zones'])} gray={sorted(progress['gray'])}")
				if progress["error"]:
					self._zone_error = progress["error"]
				self._set_loaded_zones(progress["zones"], gray=progress["gray"])
			else:
				total = progress["total"]
				processed = progress["processed"]
				fraction = processed / total if total else 0.0
				overlay = f"{processed}/{total} zones" if total else "Loading..."
				imgui.progress_bar(fraction, overlay=overlay)

		if self._zone_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._zone_error)

		imgui.separator()
		self._draw_render_mode_bar()

		if self.zones:
			total_patches = sum(len(cache_data.patches) for cache_data in self.zones.values())
			imgui.text(f"{len(self.zones)} zone(s) loaded -- {total_patches} patches total")

	def _select_render_mode(self, mode):
		self.render_mode = mode
		if mode == "2D":
			self.orbit_camera.snap_to_axis("+z")
		self._apply_render_mode()

	def _draw_render_mode_bar(self):
		"""[2D][POLY][WELD][LIGHT] button row (project-todos/forgery/
		landscape_editor__zone_render_modes.md step 6) -- the active mode is
		highlighted; WELD/LIGHT additionally show how many of the currently
		loaded zones are missing their own extension (rendered gray, see
		_resolve_zone_for_mode())."""
		for i, mode in enumerate(_RENDER_MODES):
			if i > 0:
				imgui.same_line()
			active = self.render_mode == mode
			if active:
				imgui.push_style_color(imgui.Col_.button.value, (0.35, 0.55, 0.35, 1.0))
			clicked = imgui.button(mode)
			if active:
				imgui.pop_style_color()
			if clicked and not active:
				self._select_render_mode(mode)

		real_exts = _MODE_REAL_EXTENSIONS.get(self.render_mode)
		if real_exts is not None and self._loaded_refs:
			imgui.text(f"{self._missing_for_mode} zone(s) not {self.render_mode.lower()}ed ({'/'.join(real_exts)} missing)")


def main(argv=None):
	LandscapeEditorApp().run()


if __name__ == "__main__":
	main()
