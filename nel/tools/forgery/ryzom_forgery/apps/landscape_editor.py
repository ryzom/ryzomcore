"""Ryzom Forgery landscape editor, branded "Atyscape" (Nuno, 2026-09-07 --
same naming convention as object_editor.py/"Patina"): 3D landscape
composition/build tool, replacing the current 2D bitmap-tile Ligo editor.

See project-todos/forgery/landscape_editor.md for the planned progressive
rendering steps. Visualisation/Edition mode-specific logic lives in the two
mixins this class inherits from (project-todos/forgery/
landscape_editor__land_preview.md step 5): `landscape_editor_edit_mode.py`
(EditModeMixin) and `landscape_editor_view_mode.py` (ViewModeMixin) --
shared rendering (zone_geometry.py/zone_cache.py/zone_geom_cache.py,
_apply_render_mode()/_resolve_zone_for_mode()) stays here, in the base
class.
"""

import math
import threading
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, imgui_ctx
from panda3d.core import Point3, Quat, TransparencyAttrib

from pynel import repository_paths
from pynel.ryzom_bnp import BnpError
from pynel.ryzom_ig import parse_ig, IgParseError
from pynel.ryzom_zone import parse_zone, ZoneParseError

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.apps.landscape_editor_edit_mode import EditModeMixin
from ryzom_forgery.apps.landscape_editor_modes import (
	_detect_app_mode, _MODE_BADGE_COLOR, _MODE_BADGE_LABEL, _MODE_EDITION, _MODE_REAL_EXTENSIONS, _MODE_VISUALISATION,
	_OTHER_MODE, _RENDER_MODES, _resolve_zone_for_mode,
)
from ryzom_forgery.apps.landscape_editor_view_mode import ViewModeMixin
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_begin_tab_item_with_icon, _icon_button, _OBJECT_TRANSPARENCY_ALPHA, _pop_tab_color, _push_tab_color,
	_VIEWPORT_TOGGLE_MARGIN_PX,
)
from ryzom_forgery.camera import AXIS_VIEWS, OrbitCamera
from ryzom_forgery.land_geometry import expected_zone_name
from ryzom_forgery import continent_selector
from ryzom_forgery import continent_ecosystem
from ryzom_forgery.error_log import report_error
from ryzom_forgery import live_data
from ryzom_forgery.mouse_picking import mouse_ground_position
from ryzom_forgery.live_data_setup_dialog import LiveDataSetupDialog
from ryzom_forgery import pipeline_data_installer
from ryzom_forgery.pipeline_data_install_dialog import PipelineDataInstallDialog
from ryzom_forgery.region_hierarchy import assign_zones_to_regions, get_regions_for_continent, get_regions_for_continent_edition
from ryzom_forgery.region_loader import (
	find_zones_in_region, get_continent_z_range, get_ig_index, get_zone_extensions_index, has_pipeline_export,
	load_zone_cache_data, read_ig_ref_bytes, RegionLoadError, zone_ref_extension,
)
from ryzom_forgery import continent_pipeline_reference as cpr
from ryzom_forgery import ig_geometry
from ryzom_forgery.ryzom_paths_section import RyzomPathsSection
from ryzom_forgery.search_paths_dialog import SearchPathsDialog
from ryzom_forgery import settings as app_settings
from ryzom_forgery import zone_geom_cache
from ryzom_forgery.zone_geom_cache import ZoneGeomManifest, ZoneManifestEntry
from ryzom_forgery.zone_geometry import (
	build_zone_geom_from_cache, build_zone_grid_geom, build_zone_placeholders_geom, build_zone_selection_border_geom,
	get_stage_colors, hex_to_rgb, rgb_to_hex, set_stage_colors, ZONE_CELL_SIZE, zone_build_stage, zone_to_cache_data,
)

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

# Zone selection (project-todos/forgery/landscape_editor.md, Nuno
# 2026-09-11) -- max normalized mouse-coordinate movement (range [-1, 1]
# across the window) between mouse1-down and mouse1-up still counted as a
# "click" (selects the zone under the cursor) rather than a drag (left
# handled by OrbitCamera's own orbit instead) -- generous enough to absorb
# real hand jitter on a click without ever mistaking a real orbit drag for one.
_ZONE_CLICK_MAX_DRAG = 0.015

# Wireframe cycling button (project-todos/forgery/wireframe_cycle_states.md)
# -- same icon in all 3 states (only the tooltip/active highlight change).
_WIREFRAME_MODE_LABELS = {
	"off": "Wireframe off",
	"overlay": "Wireframe (overlay on shaded render)",
	"pure": "Wireframe (pure, no texture)",
}

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


class LandscapeEditorApp(EditModeMixin, ViewModeMixin, ForgeryApp):
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

		# Per-build-stage zone color overrides (project-todos/forgery/
		# landscape_editor__land_composition.md step 4's Colors settings
		# section, Nuno 2026-09-12) -- applied once at launch; zone_geometry's
		# own _STAGE_COLORS already hold sensible built-in defaults, a saved
		# override here only replaces the ones the user actually changed.
		for stage_str, hex_pair in settings.landscape_zone_stage_colors.items():
			try:
				set_stage_colors(int(stage_str), hex_to_rgb(hex_pair[0]), hex_to_rgb(hex_pair[1]))
			except (ValueError, IndexError):
				pass

		# OrbitCamera._update() (camera.py) reads both of these every frame --
		# there's no ObjectManipulator/target object here (unlike
		# object_editor.py, which sets these to drive its own navcube-driven
		# drag-mode overrides), so both stay at their "no override" default.
		self.forced_drag_mode = None
		self.target_mode = None

		# Landscape zones are 160 world units wide -- a much larger scale than
		# object_editor's single-shape inspection, so this starts zoomed out
		# much further than that app's own OrbitCamera default.
		# 2D (top-down, no rotation) is the default view at launch
		# (project-todos/forgery/landscape_editor__2d_3d_toggle.md, Nuno
		# 2026-09-11) -- built directly with the "+z" heading/pitch rather
		# than the controller's own (0, 0) default + an animated
		# snap_to_axis() call, since there's nothing to animate FROM yet.
		top_down_heading, top_down_pitch = AXIS_VIEWS["+z"]
		self.orbit_camera = OrbitCamera(self, distance=200.0, heading=top_down_heading, pitch=top_down_pitch)
		self.orbit_camera.lock_rotation = True
		self._top_down_locked = True
		# Terrain display toggles (project-todos/forgery/
		# landscape_editor__transparency_wireframe.md) -- independent of
		# self.render_mode and of each other, applied straight to
		# self._zone_root so they cover whatever is currently loaded there.
		self._zone_transparent = False
		# "off"/"overlay"/"pure" (project-todos/forgery/wireframe_cycle_states.md,
		# Nuno 2026-09-11 -- was a plain bool) -- "overlay" draws wireframe on
		# top of the normal shaded/textured render, "pure" is wireframe only
		# (no texture, no fill).
		self._wireframe_mode = "off"
		# The 3D orientation to restore when leaving 2D for the first time
		# since launch (no real 3D view has existed yet to remember) -- an
		# angled overview rather than another top-down look.
		self._saved_3d_heading_pitch = (0.0, 45.0)
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
		self._pending_continent_reload = False

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
		# The zone-grid overlay's own bounds while a whole continent is
		# loaded (project-todos/forgery/landscape_editor__region_management.md
		# step 3 fix, Nuno 2026-09-13: the grid must always span the WHOLE
		# continent, never shrink to just the region(s) currently checked) --
		# set to self.continent_bounds once a continent's zone index scan
		# completes, cleared back to None by a single-zone selection
		# (on_selection_changed()) or a continent deselect, both of which
		# want _set_loaded_zones()'s old behaviour (grid == loaded zones'
		# own bounds) instead.
		self._continent_loaded_grid_bounds = None

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

		# `.ig` instance display (project-todos/forgery/landscape_editor.md
		# step 11, visualisation only -- editing is step 12). Resolves each
		# instance's `.shape` by name via search_paths_dialog (same generic,
		# .bnp-aware index Patina already uses for textures/.skel/.anim, see
		# search_paths_dialog.py's own module docstring), so a normal, unlit
		# Settings surface isn't needed here -- it's the same search paths
		# already configured for the whole suite (see explorer_root above).
		self.search_paths_dialog = SearchPathsDialog()
		self._ig_root = self.render.attach_new_node("ig-root")
		self._ig_nodes = {}  # zone name -> NodePath (parent of that zone's instance NodePaths)
		self._ig_shape_templates = {}  # shape_name -> GeomNode or None (unresolvable/no mesh)
		self._ig_load_progress = None
		self._ig_loaded_zone_names = frozenset()
		self._ig_visible = True
		self._ig_error = None

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
		self._land_missing_cells = set()
		# "Generate missing .zonew" button state (project-todos/forgery/
		# landscape_editor__zone_render_modes.md step 9) -- None while idle,
		# else the dict a background thread (_generate_missing_zonew()) is
		# writing into, polled the same way as _continent_load_progress.
		self._weld_generate_progress = None

		# Region-based lazy loading (project-todos/forgery/
		# landscape_editor__region_management.md) -- Visualisation mode only
		# (source: world.lua embedded in gamedev.bnp, live_data_path
		# exclusive). self._all_continent_refs/_all_continent_extensions hold
		# EVERY real zone found for the loaded continent (populated once per
		# continent load, never touched by a region checkbox toggle);
		# self._loaded_refs/_loaded_extensions (above) are narrowed down to
		# only the zones whose region is currently checked -- toggling a
		# region rebuilds that subset and calls _apply_render_mode() on it,
		# same background-reload pipeline as everything else. self._region_
		# names/_region_zone_map/_region_checked are all empty for a
		# continent with no region_hierarchy.get_regions_for_continent()
		# entries -- see _load_continent()'s own handling of that fallback,
		# which restores the pre-region_management behaviour (every zone
		# loaded immediately, no purple squares/checkboxes at all).
		self._all_continent_refs = {}
		self._all_continent_extensions = {}
		self._region_names = []
		self._region_zone_map = {}  # zone name -> region name
		self._region_checked = {}  # region name -> bool
		# Édition mode only (step 5 fix): {(pos_x, pos_y): region name} for
		# EVERY used `.land` cell, real export or not -- gates the `.land`+
		# brick fallback (_apply_render_mode()) to checked regions too, not
		# just the real, already-exported zones self._region_zone_map covers.
		self._land_cell_region_map = {}
		# Whole-continent elevation-color reference (project-todos/forgery/
		# landscape_editor__region_management__zone_bam_cache.md step 1) --
		# (min_z, max_z) across every real zone of the loaded continent,
		# regardless of which region is checked; None until a continent
		# with at least one successfully-parsed real zone has loaded.
		self._continent_z_range = None
		self._region_placeholder_np = self.render.attach_new_node("zone-region-placeholder")
		# (pos_x, pos_y) -> ZoneUnit.zone_name (the brick the .land itself
		# assigns to that cell), for the cursor status line -- always the
		# .land's own brick reference, regardless of which pipeline stage
		# (.zone/.zonew/.zonel) actually renders there (Nuno 2026-09-10:
		# "je veux le nom du fichier DANS le .land", every case, not just
		# fallback cells). Cached per continent, see
		# _ensure_land_cell_names_loaded().
		self._land_cell_names = {}
		self._land_cell_names_continent = None

		# Grid-cell selection for the composition editor (project-todos/
		# forgery/landscape_editor__land_composition.md step 6) -- (pos_x,
		# pos_y) in the `.land`'s own coordinate space, captured alongside the
		# existing zone-name selection (_select_zone_at_cursor()) but kept
		# separate: it's derived directly from the cursor's world position
		# (floor(x/CELL), floor(y/CELL)), never through a real zone name (see
		# _build_land_driven_refs()'s own docstring, landscape_editor_edit_
		# mode.py, for why world_pos_to_zone_name() is NOT a safe way to get
		# a `.land` grid position from an arbitrary interior point). `None`
		# when the cursor is outside the valid grid or not in Edition mode.
		# `_land_region`/`_land_region_path` cache the loaded `.land` for the
		# editor UI itself (EditModeMixin), invalidated whenever the
		# continent selection changes.
		self._land_selected_cell = None
		self._land_region = None
		self._land_region_path = None
		self._land_available_bricks = None
		self._land_available_bricks_dir = None
		self._land_edit_error = None
		# "Build" button state (project-todos/forgery/
		# landscape_editor__land_composition.md step 7) -- None while idle,
		# else the dict a background thread (_run_land_build(),
		# landscape_editor_edit_mode.py) is writing into, polled the same way
		# as self._weld_generate_progress.
		self._land_build_progress = None

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

		# Zone selection (project-todos/forgery/landscape_editor.md, Nuno
		# 2026-09-11): a plain left-click (no drag -- see
		# _on_zone_click_up()'s own threshold, so it never fires alongside
		# OrbitCamera's own left-drag orbit) on a loaded zone highlights its
		# border and makes it the rotation pivot. Bound via accept() (event-
		# based down/up), independent of OrbitCamera._update()'s own
		# per-frame polling of the same button -- both coexist fine, a real
		# drag just never leaves _zone_click_down_pos close enough to its
		# start for _on_zone_click_up() to treat it as a click.
		self._selected_zone_name = None
		self._zone_click_down_pos = None
		self._zone_selection_np = self.render.attach_new_node("zone-selection-border-placeholder")
		self.accept("mouse1", self._on_zone_click_down)
		self.accept("mouse1-up", self._on_zone_click_up)

	def _on_zone_click_down(self):
		mw = self.mouseWatcherNode
		if self.imgui.isMouseCaptured() or not mw.hasMouse():
			self._zone_click_down_pos = None
			return
		mouse = mw.getMouse()
		self._zone_click_down_pos = (mouse.getX(), mouse.getY())

	def _on_zone_click_up(self):
		down_pos = self._zone_click_down_pos
		self._zone_click_down_pos = None
		mw = self.mouseWatcherNode
		if down_pos is None or self.imgui.isMouseCaptured() or not mw.hasMouse():
			return
		mouse = mw.getMouse()
		dx, dy = mouse.getX() - down_pos[0], mouse.getY() - down_pos[1]
		if (dx * dx + dy * dy) ** 0.5 > _ZONE_CLICK_MAX_DRAG:
			return  # a real drag (orbit/pan/zoom), not a click
		self._select_zone_at_cursor()

	def _select_zone_at_cursor(self):
		"""Selects the zone under the mouse cursor, or clears the selection
		if the cursor is over no zone (off the edge of the valid grid, or no
		zone actually loaded there) -- same ground-plane math as
		_update_cursor_status(). In Edition mode, also captures the `.land`
		grid cell under the cursor (project-todos/forgery/
		landscape_editor__land_composition.md step 6) -- a direct floor() of
		the world position, independent of whether a loaded zone covers it."""
		ground_pos = mouse_ground_position(self)
		if ground_pos is None:
			self._land_selected_cell = None
			self._clear_zone_selection()
			return
		x, y = ground_pos
		self._land_selected_cell = (math.floor(x / ZONE_CELL_SIZE), math.floor(y / ZONE_CELL_SIZE)) if self._app_mode == _MODE_EDITION else None
		name = self._find_loaded_zone_at(x, y)
		if name is None:
			self._clear_zone_selection()
			return
		self._select_zone(name)

	def _find_loaded_zone_at(self, x, y):
		"""The name of whichever currently loaded zone/piece (self.zones --
		real single-cell zones and multi-cell `.land` fallback pieces alike)
		actually covers world position `(x, y)`, by its own real bounding
		box -- not derived from a fixed 160-unit name-based grid cell at all
		(unlike the old world_pos_to_zone_name()-based lookup this replaces),
		so a multi-cell fallback piece's WHOLE real footprint is found and
		selected as one match (Nuno 2026-09-12: "si la zone est sur
		plusieurs zones... ca selectionne TOUT"), never just the 160x160
		slice its clicked corner would nominally belong to."""
		for name, cache_data in self.zones.items():
			half_x, half_y = cache_data.bb_half_size[0], cache_data.bb_half_size[1]
			center_x, center_y = cache_data.bb_center[0], cache_data.bb_center[1]
			if center_x - half_x <= x <= center_x + half_x and center_y - half_y <= y <= center_y + half_y:
				return name
		return None

	def _select_zone(self, zone_name):
		"""Highlights `zone_name`'s border (orange, double the grid's own
		default thickness) and, in 3D only, makes its center -- at world
		Z=0, matching the terrain's own sea-level anchor -- the OrbitCamera's
		rotation pivot -- distance is left untouched (Nuno 2026-09-11: no
		automatic zoom on selection). In 2D (self._top_down_locked), the
		camera never moves/retargets at all (Nuno 2026-09-12: "en vue 2D la
		camera ne bouge pas, pas de pivot a gerer" -- 2D has no orbit
		rotation to pivot in the first place, lock_rotation is already on).
		The border always matches `zone_name`'s own REAL bounding box
		(self.zones), correctly covering a multi-cell `.land` fallback
		piece's whole footprint, not a fixed single 160x160 cell."""
		cache_data = self.zones.get(zone_name)
		if cache_data is None:
			return
		half_x, half_y = cache_data.bb_half_size[0], cache_data.bb_half_size[1]
		center_x, center_y = cache_data.bb_center[0], cache_data.bb_center[1]
		min_x, max_x = center_x - half_x, center_x + half_x
		min_y, max_y = center_y - half_y, center_y + half_y

		self._selected_zone_name = zone_name
		self._zone_selection_np.remove_node()
		self._zone_selection_np = self.render.attach_new_node(
			build_zone_selection_border_geom(min_x, min_y, max_x, max_y)
		)
		# Same "always on top, flat at sea level" treatment as self._grid_np
		# (Nuno 2026-09-08's own reasoning applies identically here) --
		# bin order 101 (one above the grid's 100) so the orange selection
		# border draws over the plain grid line it overlaps, not the other
		# way around (Nuno 2026-09-11: "la bordure se voit mal").
		self._zone_selection_np.set_light_off()
		self._zone_selection_np.set_depth_test(False)
		self._zone_selection_np.set_depth_write(False)
		self._zone_selection_np.set_bin("fixed", 101)

		if not self._top_down_locked:
			self.orbit_camera.retarget(Point3(center_x, center_y, 0.0))

	def _clear_zone_selection(self):
		if self._selected_zone_name is None:
			return
		self._selected_zone_name = None
		self._zone_selection_np.remove_node()
		self._zone_selection_np = self.render.attach_new_node("zone-selection-border-placeholder")

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
			report_error(self._zone_error)
			self._set_loaded_zones({})
			return
		self._loaded_refs = {}
		# Only the clicked file's own extension is actually known here (no
		# disk scan for its siblings) -- enough for zone_build_stage() to
		# still color it correctly (e.g. a directly-picked .zonel reads as
		# stage 3, not the stage-0 default an empty ext_map would give).
		self._loaded_extensions = {name: {item.suffix.lower(): None}}
		self._land_missing_cells = set()
		# A single-zone selection isn't continent-based -- no region
		# checkboxes/purple squares apply to it (project-todos/forgery/
		# landscape_editor__region_management.md).
		self._all_continent_refs = {}
		self._all_continent_extensions = {}
		self._region_names = []
		self._region_zone_map = {}
		self._region_checked = {}
		self._land_cell_region_map = {}
		self._continent_z_range = None
		self._continent_loaded_grid_bounds = None
		self._rebuild_region_placeholders()
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
		# Release (visualisation) must read live_data_path exclusively (per
		# project-todos/forgery/landscape_editor__land_preview.md's own
		# design) -- forcing ryzom_data_path to None here, rather than
		# passing whatever repository_paths.get() returns unconditionally,
		# is what actually enforces that: find_zones_in_region()/
		# get_zone_extensions_index()/has_pipeline_export() below would
		# otherwise silently prefer ryzom-data's pipeline export over
		# live_data_path whenever one exists for the selected continent,
		# regardless of mode (found 2026-09-10, Nuno: nexus showed 134
		# zones instead of the real live count once ryzom-data was
		# configured for Dev-mode testing).
		ryzom_data_path = repository_paths.get("ryzom-data") if self._app_mode == _MODE_EDITION else None
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
				report_error(self._zone_error)
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
			uses_pipeline_export = has_pipeline_export(ryzom_data_path, pipeline_continent_name)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_run_load_continent) "
			      f"mode={self._app_mode} continent={pipeline_continent_name!r} live_data_path={live_data_path!r} "
			      f"ryzom_data_path={ryzom_data_path!r} uses_pipeline_export={uses_pipeline_export} bounds={bounds}")
			land_cell_region_map = {}
			if self._app_mode == _MODE_EDITION:
				# Land-driven enumeration (project-todos/forgery/
				# landscape_editor__land_composition.md step 3) -- a
				# freshly-authored composition with zero real exports yet is
				# NOT an error here (unlike Visualisation below): the `.land`+
				# brick fallback (_apply_render_mode()) covers that case
				# entirely on its own. `all_used_cells` (EVERY used `.land`
				# cell, real export or not) is assigned to a region the same
				# way real zones are (project-todos/forgery/landscape_editor__
				# region_management.md step 5 fix, Nuno 2026-09-13: the
				# fallback was found still loading every brick regardless of
				# region, defeating the whole point) -- _apply_render_mode()
				# uses this to gate the fallback itself to checked regions
				# only, same as real zones.
				default_refs, extensions, all_used_cells = self._build_land_driven_refs(
					live_data_path, ryzom_data_path, pipeline_continent_name,
				)
				regions = get_regions_for_continent_edition(ryzom_data_path, pipeline_continent_name)
				cell_positions = {cell: (cell[0] * ZONE_CELL_SIZE, cell[1] * ZONE_CELL_SIZE) for cell in all_used_cells}
				land_cell_region_map = assign_zones_to_regions(cell_positions, regions)
			else:
				refs = find_zones_in_region(
					live_data_path, min_x, min_y, max_x, max_y, pipeline_continent_name, ryzom_data_path,
				)
				if not refs:
					progress["error"] = "No real zone found for this continent."
					report_error(progress["error"])
					return
				default_refs = {ref.name: ref for ref in refs}
				full_extensions_index = get_zone_extensions_index(live_data_path, pipeline_continent_name, ryzom_data_path)
				extensions = {ref.name: full_extensions_index.get(ref.name, {}) for ref in refs}
				regions = get_regions_for_continent(live_data_path, pipeline_continent_name)
			# Region hierarchy (project-todos/forgery/
			# landscape_editor__region_management.md) -- one source per
			# mode (world.lua/gamedev.bnp for Visualisation, world.json for
			# Édition, resolved above), same downstream handling either way.
			# A continent with no region entries in its own source
			# (regions == []) falls back to the pre-region_management
			# behaviour below: draw_panel()'s consumer keeps ALL of
			# default_refs in self._loaded_refs in that case.
			zone_positions = {name: (ref.x, ref.y) for name, ref in default_refs.items()}
			progress["regions"] = [r.name for r in regions]
			progress["zone_region_map"] = assign_zones_to_regions(zone_positions, regions)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_run_load_continent) "
			      f"zones found for {pipeline_continent_name!r}: {len(default_refs)}")
			progress["default_refs"] = default_refs
			progress["extensions"] = extensions
			progress["land_cell_region_map"] = land_cell_region_map
			# Whole-continent elevation-color reference (project-todos/
			# forgery/landscape_editor__region_management__zone_bam_cache.md
			# step 1) -- computed once here from every real zone of the
			# continent (default_refs), regardless of which region ends up
			# checked, so the gradient (and a future per-zone `.bam` cache
			# keyed partly on it) never depends on the checked subset.
			progress["continent_z_range"] = get_continent_z_range(default_refs)
		except OSError as exc:
			progress["error"] = str(exc)
			report_error(progress["error"])
		finally:
			progress["done"] = True

	def _rebuild_region_placeholders(self):
		"""Rebuilds the purple-square overlay (project-todos/forgery/
		landscape_editor__region_management.md step 3) for every zone/cell
		whose region checkbox, if any, is unchecked (or every zone/cell,
		before any is ever loaded). Called once after a continent finishes
		loading and again on every _toggle_region() call, same trigger
		points as _apply_render_mode().

		Visualisation: one purple square per zone in self._all_continent_refs
		NOT currently in self._loaded_refs.

		Édition (step 5 fix, Nuno 2026-09-13): one purple square per USED
		`.land` cell (self._land_cell_region_map, real export or not) whose
		region isn't checked -- a checked region's own missing cells are
		NOT squared here, since _apply_render_mode()'s `allowed_land_cells`
		gating already lets the `.land`+brick fallback render a real brick
		for those instead (avoids double geometry at the same cell)."""
		if self._app_mode == _MODE_EDITION:
			pending = [
				(cell[0] * ZONE_CELL_SIZE, cell[1] * ZONE_CELL_SIZE)
				for cell, region in self._land_cell_region_map.items()
				if not self._region_checked.get(region, False)
			]
		else:
			pending = [
				(ref.x, ref.y) for name, ref in self._all_continent_refs.items() if name not in self._loaded_refs
			]
		self._region_placeholder_np.remove_node()
		self._region_placeholder_np = self.render.attach_new_node(build_zone_placeholders_geom(pending))

	def _toggle_region(self, region_name):
		"""Flips `region_name`'s checkbox (project-todos/forgery/
		landscape_editor__region_management.md step 4): recomputes
		self._loaded_refs/_loaded_extensions as the union of every checked
		region's own zones (self._region_zone_map), then reloads that whole
		subset via the existing _apply_render_mode() background pipeline
		(same full-reload behaviour as a render-mode switch, just over a
		different zone subset) and refreshes the purple-square overlay for
		whatever is left unchecked."""
		self._region_checked[region_name] = not self._region_checked.get(region_name, False)
		active_names = {
			name for name, region in self._region_zone_map.items() if self._region_checked.get(region, False)
		}
		self._loaded_refs = {name: self._all_continent_refs[name] for name in active_names}
		self._loaded_extensions = {name: self._all_continent_extensions.get(name, {}) for name in active_names}
		self._apply_render_mode()
		self._rebuild_region_placeholders()

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
		# Resolved before the early-return below (project-todos/forgery/
		# landscape_editor__land_composition.md step 4): a freshly-authored
		# composition can have ZERO real exports yet (self._loaded_refs
		# empty) and still have a full `.land` fallback to render -- the old
		# `not self._loaded_refs` guard alone would have skipped straight to
		# "nothing to show" and never reached _resolve_land_fallback_paths()
		# at all in that case.
		land_fallback = None
		if self._app_mode == _MODE_EDITION and self.selected_continent_name:
			land_fallback = self._resolve_land_fallback_paths()
		# Gates the `.land`+brick fallback to checked regions too
		# (project-todos/forgery/landscape_editor__region_management.md step
		# 5 fix, Nuno 2026-09-13: it was found still loading every brick
		# regardless of region, defeating the whole point of this chantier)
		# -- `None` (no region_hierarchy entries for this continent) means
		# unrestricted, the pre-region_management behaviour.
		allowed_land_cells = None
		if land_fallback is not None and self._region_names:
			allowed_land_cells = {
				cell for cell, region in self._land_cell_region_map.items() if self._region_checked.get(region, False)
			}

		if not self._loaded_refs and land_fallback is None:
			self._gray_zones = set()
			self._missing_for_mode = 0
			self._land_missing_cells = set()
			# Actually tears down whatever real zone geometry was displayed
			# before (project-todos/forgery/landscape_editor__region_
			# management.md step 4 bug fix, Nuno 2026-09-13: unchecking the
			# last active region left the old mesh on screen forever, with
			# the purple placeholder squares drawn right on top of it --
			# every other caller that empties self._loaded_refs pairs it
			# with its own explicit _set_loaded_zones({}) call, this
			# early-return path was the one spot that didn't). grid_bounds
			# keeps the reference grid spanning the whole continent even
			# with zero regions checked (same fix's other half).
			self._set_loaded_zones({}, grid_bounds=self._continent_loaded_grid_bounds)
			return
		if self._mode_reload_progress is not None and not self._mode_reload_progress["done"]:
			return

		self._recompute_missing_for_mode()

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

		# `land_fallback` itself was already resolved above (before the
		# early-return) -- the .land cell's own (pos_x, pos_y) plus which
		# ones are ALREADY covered by `refs` (known only once _run_load_refs()
		# has actually loaded them, bb_center) still need the background
		# thread below, same as the real refs. All 3 render modes get this
		# fallback now (project-todos/forgery/landscape_editor__land_
		# composition.md step 4): a .land cell with no matching real file for
		# the current mode's stage is rendered as the raw brick instead of
		# staying invisible, whichever mode is active.

		self._zone_error = None
		progress = {
			"done": False, "error": None, "zones": {}, "manifest_zones": {},
			"gray": gray, "total": len(refs), "processed": 0,
		}
		self._mode_reload_progress = progress
		thread = threading.Thread(
			target=self._run_load_refs, args=(refs, progress, land_fallback, allowed_land_cells), daemon=True,
		)
		thread.start()

	def _run_load_refs(self, refs, progress, land_fallback=None, allowed_land_cells=None):
		"""Background-thread body for _apply_render_mode() -- pure file I/O/
		pickle (region_loader.py/zone_cache.py load_zone_cache_data(), same
		cache-first behavior as the old _run_load_continent()), never touches
		Panda3D. Writes only to `progress`, see _run_load_continent()'s own
		docstring for why. `land_fallback`, when given, is
		`(land_path, brick_zones_dir)` -- see _apply_render_mode()'s own
		docstring for why it's resolved there but consumed here."""
		try:
			zones = {}
			failed = []
			# Per-zone staleness stamp (project-todos/forgery/
			# geomnode_continent_cache.md step 2) -- lets _set_loaded_zones()
			# decide whether a saved continent .bam bundle still matches
			# exactly what would be built fresh, without ever loading the
			# bundle's actual geometry just to check.
			manifest_zones = {}
			for name, ref in refs.items():
				try:
					stat = ref.source_path.stat()
					zones[name] = load_zone_cache_data(ref)
					manifest_zones[name] = ZoneManifestEntry(zone_ref_extension(ref), stat.st_mtime, stat.st_size)
				except (RegionLoadError, OSError) as exc:
					failed.append(str(exc))
				progress["processed"] += 1
			if land_fallback is not None:
				land_path, brick_zones_dir = land_fallback
				self._load_land_fallback_pieces(
					land_path, brick_zones_dir, zones, manifest_zones, progress, failed,
					allowed_cells=allowed_land_cells,
				)
			progress["zones"] = zones
			progress["manifest_zones"] = manifest_zones
			if failed:
				progress["error"] = f"{len(failed)}/{len(refs)} zone(s) failed to load: {'; '.join(failed[:3])}"
				report_error(progress["error"])
		except OSError as exc:
			progress["error"] = str(exc)
			report_error(progress["error"])
		finally:
			progress["done"] = True

	def _ig_bytes_for_zone(self, name, live_data_path, ryzom_data_path, app_mode, continent):
		"""Raw bytes of zone `name`'s own `.ig` (project-todos/forgery/
		landscape_editor.md step 11), or None if there is none -- confirmed
		2026-09-13 against real data that a real `.ig` always shares its
		zone's exact name (one file per zone, same convention as
		.zone/.zonew/.zonel): Visualisation resolves it via
		region_loader.get_ig_index() (live_data_path's own `.bnp`s, e.g.
		bagne_ig.bnp), Edition via continent_pipeline_reference's own
		out_ig_dir (zone_ig_lighter's real output, same directory
		_invalidate_built_zone_files() already deletes stale `.ig` from --
		landscape_editor_edit_mode.py). Pure I/O, safe on a background
		thread."""
		if app_mode == _MODE_EDITION:
			if not ryzom_data_path or not continent:
				return None
			try:
				out_ig_dir = Path(cpr.build_land_export_config(ryzom_data_path, continent).out_ig_dir)
			except cpr.ContinentPipelineReferenceError:
				return None
			try:
				return (out_ig_dir / f"{name}.ig").read_bytes()
			except OSError:
				return None
		ig_ref = get_ig_index(live_data_path).get(name)
		if ig_ref is None:
			return None
		try:
			return read_ig_ref_bytes(ig_ref)
		except RegionLoadError:
			return None

	def _read_ig_shape_bytes(self, shape_name):
		"""Resolves+reads one `.ig` instance's referenced `.shape` by bare
		name (e.g. "pr_s3_amoeba_c.shape") via self.search_paths_dialog's own
		generic, `.bnp`-aware name index (search_paths_dialog.py's own module
		docstring already lists `.shape` among what it resolves) -- the exact
		same index Patina uses for textures/.skel/.anim, just looked up here
		instead of drawn as a Settings tab (see search_paths_dialog's own
		field comment in __init__). None if not found/unreadable. Called from
		_run_load_ig()'s background thread -- find_texture()'s own index read
		is a plain dict `.get()`, safe under the GIL even while
		_advance_external_scan() (draw(), main thread) concurrently rebuilds
		it (dict reassignment is atomic, see search_paths_dialog.py's own
		_merge_and_publish())."""
		found = self.search_paths_dialog.find_texture(shape_name)
		if found is None:
			return None
		try:
			return found.read_bytes()
		except (OSError, BnpError):
			return None

	def _load_ig_for_zones(self, zone_names):
		"""Kicks off a background reload of `.ig` instance data for exactly
		`zone_names` (project-todos/forgery/landscape_editor.md step 11) --
		manual only (project-todos/forgery/landscape_editor__region_
		management.md step 6, Nuno 2026-09-13): only the viewport toolbar's
		tree icon, turning ON (_toggle_ig_visibility()), calls this, never a
		zone-set change on its own (self._loaded_refs, NOT self.zones/
		self._mode_reload_progress's own "land:x:y" fallback pseudo-names:
		those have no real zone file, so no `.ig` either) -- the per-zone
		`.ig`/`.shape` parsing cost is pure Python and GIL-bound even on this
		background thread, so auto-triggering it on every single region
		checkbox toggle stuttered the whole app. Independent of
		self.render_mode (POLY/WELD/LIGHT): `.ig` instance placement doesn't
		change across a zone's own build stages, only its lit-or-not
		instances would (out of scope here -- lighting is step 13), so a
		render-mode switch alone never triggers this either. A reload
		already running is left to finish; a fresh call while one is in
		flight is a no-op (this function's own guard, right below)."""
		if self._ig_load_progress is not None and not self._ig_load_progress["done"]:
			return
		if not zone_names:
			self._ig_loaded_zone_names = frozenset()
			self._set_loaded_ig({})
			return
		live_data_path = app_settings.load().live_data_path
		ryzom_data_path = repository_paths.get("ryzom-data")
		progress = {"done": False, "error": None, "instances_by_zone": {}, "zone_names": frozenset(zone_names)}
		self._ig_load_progress = progress
		thread = threading.Thread(
			target=self._run_load_ig,
			args=(frozenset(zone_names), live_data_path, ryzom_data_path, self._app_mode, self._selected_continent_pipeline_name, progress),
			daemon=True,
		)
		thread.start()

	def _run_load_ig(self, zone_names, live_data_path, ryzom_data_path, app_mode, continent, progress):
		"""Background-thread body for _load_ig_for_zones() -- pure file I/O +
		pynel parsing (ryzom_ig.parse_ig()/ig_geometry.resolve_ig_instances(),
		which itself calls pynel.ryzom_shape.parse_shape() per unique
		referenced `.shape`), never touches Panda3D -- _set_loaded_ig()
		builds the actual GeomNodes on the main thread, same split as
		_run_load_refs()/_set_loaded_zones()."""
		try:
			instances_by_zone = {}
			for name in sorted(zone_names):
				data = self._ig_bytes_for_zone(name, live_data_path, ryzom_data_path, app_mode, continent)
				if data is None:
					continue
				try:
					ig = parse_ig(data)
				except IgParseError as exc:
					print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_run_load_ig) "
					      f"{name}.ig parse failed: {exc}")
					continue
				instances_by_zone[name] = ig_geometry.resolve_ig_instances(ig, self._read_ig_shape_bytes)
			total = sum(len(v) for v in instances_by_zone.values())
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_run_load_ig) "
			      f"zones_with_ig={len(instances_by_zone)}/{len(zone_names)} total_instances_resolved={total}")
			progress["instances_by_zone"] = instances_by_zone
		except OSError as exc:
			progress["error"] = str(exc)
			report_error(progress["error"])
		finally:
			progress["done"] = True

	def _ig_geom_node_for(self, resolved):
		"""The (cached) GeomNode to instance for `resolved` -- built once per
		unique (kind, shape_name) and reused (a plain GeomNode can be
		attached under any number of NodePaths in Panda3D, no explicit
		instance_to() needed) across every placement of that shape, since a
		real continent routinely places the same prop hundreds of times
		(project-todos/forgery/landscape_editor.md step 11). None if the
		shape has no renderable mesh (e.g. a FlareShape/ParticleSystemShape
		instance)."""
		if resolved.kind == "water_point":
			key = ("water_point", None)
		else:
			key = (resolved.kind, resolved.shape_name)
		if key not in self._ig_shape_templates:
			if resolved.kind == "mesh":
				node = ig_geometry.build_instance_mesh_geom(resolved.shape_value)
			elif resolved.kind == "water_polygon":
				node = ig_geometry.build_water_polygon_geom(resolved.water_polygon)
			else:
				node = ig_geometry.build_water_point_geom()
			self._ig_shape_templates[key] = node
		return self._ig_shape_templates[key]

	def _set_loaded_ig(self, instances_by_zone):
		"""Tears down whatever `.ig` geometry was attached before and builds
		fresh geometry for `instances_by_zone` (zone name -> list of
		ig_geometry.ResolvedInstance) -- mirrors _set_loaded_zones()'s own
		tear-down/rebuild shape, one child NodePath per zone under
		self._ig_root for easy per-zone bookkeeping, one grandchild NodePath
		per instance carrying that instance's own pos/rot/scale."""
		for node in self._ig_nodes.values():
			node.remove_node()
		self._ig_nodes = {}
		self._ig_shape_templates = {}
		total_instances = 0
		for name, resolved_list in instances_by_zone.items():
			zone_np = self._ig_root.attach_new_node(f"ig-zone-{name}")
			for resolved in resolved_list:
				geom_node = self._ig_geom_node_for(resolved)
				if geom_node is None:
					continue
				instance_np = zone_np.attach_new_node(geom_node)
				instance_np.set_pos(*resolved.pos)
				instance_np.set_quat(Quat(*resolved.rot))
				instance_np.set_scale(*resolved.scale)
				# A water plane is a single flat polygon -- backface-culled
				# by default, it's only visible from one side, so panning
				# the camera under the water level (or a surface whose
				# winding happens to face away from the initial view) made
				# it disappear entirely (Nuno 2026-09-13: "il ne sont pas
				# toujours visibles"). Two-sided, like the terrain mesh
				# itself (_set_loaded_zones()), for the same reason.
				if resolved.kind in ("water_polygon", "water_point"):
					instance_np.set_two_sided(True)
				total_instances += 1
			self._ig_nodes[name] = zone_np
		print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_set_loaded_ig) "
		      f"zones={len(instances_by_zone)} instances_placed={total_instances}")
		if not self._ig_visible:
			self._ig_root.hide()
		else:
			self._ig_root.show()

	def _toggle_ig_visibility(self):
		"""Viewport toolbar's tree icon (project-todos/forgery/
		landscape_editor__region_management.md step 6, Nuno 2026-09-13: "le
		bouton de la vue 3D doit déclencher le chargement" -- this existing
		icon, not a separate panel button) -- turning it ON (re)loads `.ig`
		for the CURRENT zone set (self._loaded_refs), replacing whatever was
		shown before; turning it OFF just hides the already-built geometry,
		no reload needed to turn back on unless the loaded zones changed
		since (_load_ig_for_zones() itself no-ops while a load is already
		in flight)."""
		self._ig_visible = not self._ig_visible
		if self._ig_visible:
			self._ig_root.show()
			self._load_ig_for_zones(frozenset(self._loaded_refs.keys()))
		else:
			self._ig_root.hide()

	def _elevation_z_range(self, zones):
		"""(min_z, max_z) for the elevation-color gradient -- self.
		_continent_z_range (project-todos/forgery/landscape_editor__region_
		management__zone_bam_cache.md step 2: the WHOLE continent's real
		zones, stable no matter which region is checked) when it's known,
		else `zones`' own combined range (single-zone selection, or a
		continent whose z-range computation found no parseable zone at
		all -- the pre-region_management behaviour)."""
		if self._continent_z_range is not None:
			return self._continent_z_range
		min_z = min(cd.bb_center[2] - cd.bb_half_size[2] for cd in zones.values())
		max_z = max(cd.bb_center[2] + cd.bb_half_size[2] for cd in zones.values())
		return min_z, max_z

	def _recompute_missing_for_mode(self):
		"""Recomputes self._missing_for_mode from self._loaded_extensions for
		self.render_mode -- shared by _apply_render_mode() (full mode switch)
		and the live per-zone updates applied while "Generate missing
		.zonew" (project-todos/forgery/landscape_editor__zone_render_modes.md
		step 9) is running, so the counter stays correct without a full
		reload."""
		real_exts = _MODE_REAL_EXTENSIONS.get(self.render_mode)
		if real_exts is None:
			self._missing_for_mode = 0
			return
		self._missing_for_mode = sum(
			1 for name, ext_map in self._loaded_extensions.items()
			if not any(ext in ext_map for ext in real_exts)
		)
		if self.render_mode == "WELD":
			self._missing_for_mode += len(self._land_missing_cells)

	def _rebuild_zone_node(self, name, cache_data):
		"""Replaces the single geometry node for zone `name` with one built
		from `cache_data`, keeping the stage-3 gradient consistent with every
		other currently loaded fully-built zone (step 9's live update -- see
		_set_loaded_zones()'s own docstring for why that gradient spans the
		whole loaded set, not each zone's own range). Does not touch the
		grid overlay or camera framing -- the loaded set's bounds don't
		change when a zone advances a build stage."""
		old_node = self._zone_nodes.get(name)
		if old_node is not None:
			old_node.remove_node()
		if not self.zones:
			return
		min_z, max_z = self._elevation_z_range(self.zones)
		stage = zone_build_stage(self._loaded_extensions.get(name, {}))
		node = build_zone_geom_from_cache(cache_data, min_z=min_z, max_z=max_z, stage=stage)
		node_path = self._zone_root.attach_new_node(node)
		node_path.set_two_sided(True)
		self._zone_nodes[name] = node_path

	def _rebuild_all_zone_nodes(self):
		"""Rebuilds every currently loaded zone's GeomNode in place (project-
		todos/forgery/landscape_editor__land_composition.md step 4's live
		color-tuning UI, Nuno 2026-09-12) -- a stage color picker edit only
		affects zones built AFTER the change (zone_geometry.set_stage_colors()'s
		own docstring), so seeing the new color on what's already on screen
		needs an explicit rebuild pass, same GeomNode-per-zone cost as any
		other full reload."""
		for name, cache_data in self.zones.items():
			self._rebuild_zone_node(name, cache_data)


	def _set_loaded_zones(self, zones, gray=None, manifest_zones=None, mode=None, grid_bounds=None, frame_camera=True):
		"""Tears down whatever geometry was attached before and builds fresh
		geometry for `zones` (name -> zone_cache.ZoneCacheData) -- colored by
		elevation over the combined Z range of every zone in `zones`, not
		each zone's own (see build_zone_geom_from_cache()'s own docstring)
		-- then reframes the camera on their combined bounding box. Shared by
		on_selection_changed() (a single zone) and _load_continent()
		(a whole continent), both via _apply_render_mode()/_run_load_refs().

		`grid_bounds` (min_x, min_y, max_x, max_y), when given, overrides
		the reference-grid overlay's own extent -- used by the continent
		load path (project-todos/forgery/landscape_editor__region_
		management.md step 3 fix, Nuno 2026-09-13: "il faut que la grille
		fasse TOUT le continent, pas juste la région") to always span the
		whole continent, regardless of how many zones are actually loaded
		for real right now (self._loaded_refs, a region-checkbox-filtered
		subset). `None` (every other caller) keeps the old behaviour: the
		grid spans exactly `zones`' own combined bounds.

		`gray` (zone names, project-todos/forgery/
		landscape_editor__zone_render_modes.md step 6) no longer changes the
		coloring itself (superseded by the per-zone build-stage gradient,
		project-todos/forgery/landscape_editor__land_composition.md step 4,
		zone_geometry.zone_build_stage()) -- kept only for _missing_for_mode's
		own "N zone(s) not welded/lit" count, a genuinely mode-dependent
		metric distinct from a zone's real, mode-independent build stage.

		`manifest_zones`/`mode` (project-todos/forgery/landscape_editor__
		region_management__zone_bam_cache.md), when both given (continent
		loads only -- on_selection_changed()'s single-zone path never passes
		them), enable the PER-ZONE `.bam` cache (`zone_geom_cache.py`): each
		zone's saved bundle is reused as-is if its own manifest matches
		exactly what would be built fresh for it (its own staleness stamp,
		same global elevation range -- see _elevation_z_range()), skipping
		that zone's GeomNode construction entirely; a zone with no cache hit
		gets a full rebuild as before and the fresh result is saved for next
		time. Independent per zone, unlike the old whole-continent bundle
		(`continent_geom_cache.py`, retired) -- one zone's cache miss never
		forces a rebuild of any other zone.

		`frame_camera` (project-todos/forgery/landscape_editor__region_
		management.md step 8, Nuno 2026-09-13: "le centrage automatique en
		vue 2D c'est vraiment horrible"), when False, skips
		_frame_on_loaded_zones() -- used by the continent-load/reload path
		(_select_continent() already frames the camera on the WHOLE
		continent once, up front; re-framing on just whichever subset a
		region toggle happens to load made the camera jump/zoom on every
		single checkbox click). Single-zone selection (on_selection_
		changed()) keeps the default True -- framing on that one zone is
		exactly what picking it in the Explorer is for."""
		self._gray_zones = gray or set()
		self.zones = zones

		if not zones:
			for node in self._zone_nodes.values():
				node.remove_node()
			self._zone_nodes = {}
			self._grid_np.remove_node()
			if grid_bounds is not None:
				grid_min_x, grid_min_y, grid_max_x, grid_max_y = grid_bounds
				self._grid_np = self.render.attach_new_node(
					build_zone_grid_geom(grid_min_x, grid_min_y, grid_max_x, grid_max_y),
				)
				self._grid_np.set_light_off()
				self._grid_np.set_depth_test(False)
				self._grid_np.set_depth_write(False)
				self._grid_np.set_bin("fixed", 100)
				if not self._grid_visible:
					self._grid_np.hide()
			else:
				self._grid_np = self.render.attach_new_node("zone-grid-placeholder")
			return

		# Elevation color spans the whole set of loaded zones, not each
		# zone's own tiny Z range -- every zone re-normalizing to the same
		# green gradient on its own relief made a whole continent look like
		# a patchwork of disconnected tiles at zone boundaries (found by
		# Nuno 2026-09-08 testing a real continent). As of project-todos/
		# forgery/landscape_editor__region_management__zone_bam_cache.md
		# step 2, "the whole set" means the WHOLE CONTINENT
		# (self._continent_z_range), not just whichever region subset is
		# currently checked -- see _elevation_z_range()'s own docstring.
		min_x = min(cache_data.bb_center[0] - cache_data.bb_half_size[0] for cache_data in zones.values())
		min_y = min(cache_data.bb_center[1] - cache_data.bb_half_size[1] for cache_data in zones.values())
		max_x = max(cache_data.bb_center[0] + cache_data.bb_half_size[0] for cache_data in zones.values())
		max_y = max(cache_data.bb_center[1] + cache_data.bb_half_size[1] for cache_data in zones.values())
		min_z, max_z = self._elevation_z_range(zones)

		grid_min_x, grid_min_y, grid_max_x, grid_max_y = grid_bounds if grid_bounds is not None else (min_x, min_y, max_x, max_y)
		self._grid_np.remove_node()
		self._grid_np = self.render.attach_new_node(build_zone_grid_geom(grid_min_x, grid_min_y, grid_max_x, grid_max_y))
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

		cache_enabled = manifest_zones is not None and mode is not None

		for node in self._zone_nodes.values():
			node.remove_node()
		self._zone_nodes = {}
		for name, cache_data in zones.items():
			stage = zone_build_stage(self._loaded_extensions.get(name, {}))
			node_path = None
			wanted_manifest = None
			if cache_enabled and name in manifest_zones:
				wanted_manifest = ZoneGeomManifest(
					format_version=1, entry=manifest_zones[name], min_z=min_z, max_z=max_z,
				)
				cached = zone_geom_cache.read_zone_bundle(name, mode, self.loader)
				if cached is not None:
					cached_node_path, cached_manifest = cached
					if cached_manifest == wanted_manifest:
						cached_node_path.set_name(name)
						cached_node_path.reparent_to(self._zone_root)
						node_path = cached_node_path
					else:
						cached_node_path.remove_node()
			if node_path is None:
				node = build_zone_geom_from_cache(cache_data, min_z=min_z, max_z=max_z, stage=stage)
				node_path = self._zone_root.attach_new_node(node)
				# Winding isn't guaranteed to match Panda3D's expected front-face
				# direction (zone_geometry.py's grid triangulation follows NeL's
				# own Bezier control-point convention) -- two-sided so every
				# patch is visible regardless, rather than risking half the
				# terrain silently backface-culled.
				node_path.set_two_sided(True)
				# Named after the zone (project-todos/forgery/
				# landscape_editor__region_management__zone_bam_cache.md step
				# 4) -- build_zone_geom_from_cache()'s own GeomNode is always
				# named "zone-tessellated", so without this every saved
				# per-zone `.bam` would come back under the same name on
				# reload, making it impossible to tell zones apart.
				node_path.set_name(name)
				if wanted_manifest is not None:
					zone_geom_cache.write_zone_bundle(name, mode, node_path, wanted_manifest)
			self._zone_nodes[name] = node_path

		if frame_camera:
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
		if self._app_mode == _MODE_EDITION:
			self._load_edition_continent_locations()
		else:
			self._load_visualisation_continent_locations()


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
		# Grid editor state (project-todos/forgery/
		# landscape_editor__land_composition.md step 6) is per-continent --
		# stale otherwise (a cached ZoneRegion/brick list from the previous
		# continent, or a selected cell that no longer means anything).
		self._land_selected_cell = None
		self._land_region = None
		self._land_region_path = None
		self._land_available_bricks = None
		self._land_available_bricks_dir = None
		self._land_edit_error = None
		self._land_build_progress = None
		if self._app_mode == _MODE_EDITION:
			# Edition mode never reads live_data_path/sheet_id.bin (see
			# _load_edition_continent_locations()) -- continent_name here is
			# ryzom.world's own struct_name (e.g. "nexus"), which sheet_id.bin
			# doesn't know at all (it keys by the raw .continent filename
			# stem, e.g. "lecarrefour.continent" for that same continent --
			# the exact trap pynel's ryzom_world.py already documents).
			# Bounds come from the same ryzom.world entry instead (minx/miny/
			# maxx/maxy, already read into ContLoc by
			# load_continent_locations_from_world_file()) -- no extra file.
			cont_loc = next((loc for loc in self._cont_locs or () if loc.continent_name == continent_name), None)
			if cont_loc is None:
				self._bounds_error = f"Failed to resolve bounds: {continent_name!r} not found in the loaded continent list."
				report_error(self._bounds_error)
				return
			self.continent_bounds = (cont_loc.min_x, cont_loc.min_y, cont_loc.max_x, cont_loc.max_y)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_select_continent) "
			      f"edition bounds for {continent_name!r} from ryzom.world: {self.continent_bounds}")
		else:
			live_data_path = app_settings.load().live_data_path
			try:
				self.continent_bounds = continent_selector.resolve_continent_bounds(live_data_path, continent_name)
			except (OSError, continent_selector.ContinentSelectorError) as exc:
				self._bounds_error = f"Failed to resolve bounds: {exc}"
				report_error(self._bounds_error)
				return
		# Frames the camera on the WHOLE continent immediately, same
		# frame_bounds() as the "Center camera here" button below --
		# previously this only ever happened once real zone geometry
		# finished loading (_frame_on_loaded_zones(), called from
		# _set_loaded_zones()), which a continent with regions (project-
		# todos/forgery/landscape_editor__region_management.md) no longer
		# does until a checkbox is checked, leaving the camera pointed at
		# nothing/the previous continent while the purple placeholder
		# squares sat off-screen (Nuno 2026-09-13: "aucune zone violette
		# n'est créer[e]" -- they were, just unseen).
		min_x, min_y, max_x, max_y = self.continent_bounds
		self.orbit_camera.frame_bounds(min_x, min_y, max_x, max_y, margin=ZONE_CELL_SIZE)
		self._load_continent()

	def _toggle_grid(self):
		self._grid_visible = not self._grid_visible
		(self._grid_np.show if self._grid_visible else self._grid_np.hide)()

	def _toggle_top_down(self):
		"""2D/3D viewport toggle (project-todos/forgery/
		landscape_editor__2d_3d_toggle.md) -- purely a camera behavior switch,
		independent of self.render_mode ([POLY]/[WELD]/[LIGHT] keep resolving
		geometry exactly the same either way). Leaving 3D for 2D remembers the
		orientation being left, so coming back to 3D restores it instead of
		leaving the view stuck on the top-down look."""
		self._top_down_locked = not self._top_down_locked
		if self._top_down_locked:
			self._saved_3d_heading_pitch = (self.orbit_camera.heading, self.orbit_camera.pitch)
			self.orbit_camera.lock_rotation = True
			self.orbit_camera.snap_to_axis("+z")
		else:
			self.orbit_camera.lock_rotation = False
			self.orbit_camera.animate_to_orientation(*self._saved_3d_heading_pitch)

	def _toggle_zone_transparency(self):
		"""50% terrain transparency (project-todos/forgery/
		landscape_editor__transparency_wireframe.md) -- same mechanism as
		Patina's own object transparency toggle (object_editor_mixins/
		viewport_transform.py's _apply_object_transparency()), applied to
		self._zone_root instead of a single object's model_root, so it
		covers every zone currently loaded regardless of self.render_mode.
		Independent of _cycle_zone_wireframe() -- both can be on at once."""
		self._zone_transparent = not self._zone_transparent
		if self._zone_transparent:
			self._zone_root.set_transparency(TransparencyAttrib.M_alpha)
			self._zone_root.set_color_scale(1, 1, 1, _OBJECT_TRANSPARENCY_ALPHA)
		else:
			self._zone_root.clear_transparency()
			self._zone_root.clear_color_scale()

	def _apply_zone_wireframe(self):
		"""Applies self._wireframe_mode to self._zone_root (project-todos/
		forgery/wireframe_cycle_states.md) -- independent of
		_toggle_zone_transparency()/shading mode, all combinable:
		- "off": no override.
		- "overlay": set_render_mode_filled_wireframe() -- wireframe drawn on
		  top of the normal shaded/textured render, never replacing it
		  (project-todos/forgery/object_editor__wireframe_overlay.md).
		- "pure": set_render_mode_wireframe() + set_texture_off() -- wireframe
		  only, no texture, no fill."""
		if self._wireframe_mode == "off":
			self._zone_root.clear_render_mode()
			self._zone_root.clear_texture()
		elif self._wireframe_mode == "overlay":
			self._zone_root.clear_texture()
			self._zone_root.set_render_mode_filled_wireframe((0, 0, 0, 1), 1)
		else:  # "pure"
			self._zone_root.set_render_mode_wireframe(1)
			self._zone_root.set_texture_off(1)

	def _set_zone_wireframe_mode(self, mode):
		self._wireframe_mode = mode
		self._apply_zone_wireframe()

	def _cycle_zone_wireframe(self):
		"""Left-click behavior for the wireframe button -- advances to the
		next state (project-todos/forgery/wireframe_cycle_states.md, general
		cycling-button convention: right-click instead jumps straight to a
		chosen state, see _draw_viewport_toggles()'s own popup)."""
		next_mode = {"off": "overlay", "overlay": "pure", "pure": "off"}
		self._set_zone_wireframe_mode(next_mode[self._wireframe_mode])

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
			imgui.same_line()
			top_down_tooltip = "Switch to 3D view" if self._top_down_locked else "Switch to 2D view (top-down, no rotation)"
			if _icon_button(fa_icons.ICON_FA_CUBE, top_down_tooltip, not self._top_down_locked, square=True,
			                large_font=large_font):
				self._toggle_top_down()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_CIRCLE_HALF_STROKE, "50% zone transparency",
			                self._zone_transparent, square=True, large_font=large_font):
				self._toggle_zone_transparency()
			imgui.same_line()
			wireframe_tooltip = f"{_WIREFRAME_MODE_LABELS[self._wireframe_mode]} (right-click to choose)"
			if _icon_button(fa_icons.ICON_FA_DRAW_POLYGON, wireframe_tooltip,
			                self._wireframe_mode != "off", square=True, large_font=large_font):
				self._cycle_zone_wireframe()
			if imgui.begin_popup_context_item("##wireframe-mode-popup"):
				for mode in ("off", "overlay", "pure"):
					clicked, _ = imgui.selectable(_WIREFRAME_MODE_LABELS[mode], self._wireframe_mode == mode)
					if clicked:
						self._set_zone_wireframe_mode(mode)
				imgui.end_popup()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_TREE, "Show .ig instances (props/water)", self._ig_visible,
			                square=True, large_font=large_font):
				self._toggle_ig_visibility()
			self._viewport_toggle_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def panel_title(self):
		return "Landscape Editor"

	def _resolve_app_mode(self):
		"""Release/Dev is normally a user choice, persisted in
		Settings.landscape_editor_mode (project-todos/forgery/
		landscape_editor__land_preview.md step 4) -- _detect_app_mode()'s
		auto-detection from ryzom-data only ever applies the very first time
		(no saved choice yet). A saved "Dev" choice is force-reverted to
		"Release" whenever ryzom-data is no longer configured/valid (Dev has
		no meaning without it) -- the saved preference itself is left
		untouched on disk, so it takes over again as soon as ryzom-data is
		reconfigured."""
		ryzom_data_valid = repository_paths.is_valid("ryzom-data")
		saved_mode = app_settings.load().landscape_editor_mode
		if saved_mode is None:
			return _detect_app_mode()
		if saved_mode == _MODE_EDITION and not ryzom_data_valid:
			return _MODE_VISUALISATION
		return saved_mode

	def _set_app_mode(self, mode):
		settings = app_settings.load()
		settings.landscape_editor_mode = mode
		app_settings.save(settings)

	def _update_cursor_status(self):
		"""Zone name + world position under the mouse cursor, in the shared
		SysInfoBar next to the FPS counter (project-todos/forgery/
		landscape_editor__cursor_zone_status.md step 3) -- cleared whenever
		the cursor isn't over the 3D viewport (captured by an ImGui window
		instead, same check camera.py's OrbitCamera uses) or is outside the
		valid zone grid (e.g. looking at the sky, or off the edge of the
		world)."""
		if self.imgui.isMouseCaptured():
			self.sysinfo.set_cursor_info("")
			return
		ground_pos = mouse_ground_position(self)
		if ground_pos is None:
			self.sysinfo.set_cursor_info("")
			return
		x, y = ground_pos
		# expected_zone_name() (land_geometry.py), NOT a direct
		# world_pos_to_zone_name(x, y) call on the raw interior cursor
		# position -- found 2026-09-12 (Nuno: "impossible de generer la
		# zone 45_BZ" -- turned out to be a display bug, not a build one)
		# that world_pos_to_zone_name() is off by one row from the real
		# pipeline zone name for any point strictly inside a cell (only its
		# EXACT corner lands correctly, see expected_zone_name()'s own
		# docstring) -- confirmed again here against nexus's real exported
		# 44_BZ.zone/45_BZ.zone (their own bb_center floors to (51,-44)/
		# (51,-45) respectively): a cursor centered in land cell (51,-45)
		# showed "44_BZ" from the old call, one row off from the name the
		# `.land`-driven Build system (and the real exported file itself)
		# actually uses for that position.
		cell = (math.floor(x / ZONE_CELL_SIZE), math.floor(y / ZONE_CELL_SIZE))
		zone_name = expected_zone_name(*cell)
		if zone_name is None:
			self.sysinfo.set_cursor_info("")
			return
		# _ensure_land_cell_names_loaded() (EditModeMixin, landscape_editor_
		# edit_mode.py) has no meaning outside Edition mode -- ryzom-data
		# (its only source) is never read in Visualisation.
		if self._app_mode == _MODE_EDITION:
			self._ensure_land_cell_names_loaded()
		else:
			self._land_cell_names = {}
			self._land_cell_names_continent = None
		brick_name = self._land_cell_names.get(cell)
		brick_suffix = f" -- {brick_name}" if brick_name is not None else ""
		self.sysinfo.set_cursor_info(f"{zone_name} ({x:.0f}, {y:.0f}){brick_suffix}")

	def draw_panel(self):
		# `.ig` instance display (project-todos/forgery/landscape_editor.md
		# step 11) resolves each instance's `.shape` against this index --
		# kicked off once, then advanced a time-bounded slice per frame from
		# here (search_paths_dialog.py's own module docstring: "driven from
		# draw(), not a background thread"), same as object_editor.py's own
		# Skinning preview. No visible UI of its own here (draw() only
		# advances the scan/polls the add-dir file dialog, never renders a
		# window) -- landscape_editor doesn't need a Settings surface for it,
		# it reuses the same suite-wide search paths already configured for
		# the Explorer root (see __init__).
		self.search_paths_dialog.ensure_scanned()
		self.search_paths_dialog.draw()

		self._update_cursor_status()
		mode = self._resolve_app_mode()
		if mode != self._app_mode:
			# Continent list source depends on the mode (step 3 below) --
			# force a reload rather than keep showing a stale list from the
			# mode we just left. The actually-loaded zones (self._loaded_refs/
			# self.zones) come from that same stale source too (found
			# 2026-09-10, Nuno: switching mode kept showing the old mode's
			# zones) -- _pending_continent_reload re-selects the current
			# continent from _draw_landscape_tab(), once _cont_locs has been
			# refreshed for the new mode (needed for Edition's bounds
			# lookup), which re-triggers _load_continent() end to end.
			self._app_mode = mode
			self._cont_locs = None
			self._cont_locs_error = None
			self._pending_continent_reload = True
		imgui.text("Mode:")
		imgui.same_line()
		imgui.text_colored(_MODE_BADGE_COLOR[mode], _MODE_BADGE_LABEL[mode])
		imgui.same_line()
		other_mode = _OTHER_MODE[mode]
		ryzom_data_valid = repository_paths.is_valid("ryzom-data")
		imgui.begin_disabled(not ryzom_data_valid)
		if imgui.button(f"Switch to {_MODE_BADGE_LABEL[other_mode]}"):
			self._set_app_mode(other_mode)
		imgui.end_disabled()
		if not ryzom_data_valid and imgui.is_item_hovered():
			imgui.set_tooltip("Configure ryzom-data in Settings > Ryzom Paths to enable Dev mode.")
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
				self._draw_zone_colors_settings()
				imgui.end_tab_item()
			_pop_tab_color()
			imgui.end_tab_bar()

	def _draw_zone_colors_settings(self):
		"""Atyscape's own "Colors" settings section (project-todos/forgery/
		landscape_editor__land_composition.md step 4, Nuno 2026-09-12) --
		each build-stage color (zone_geometry.zone_build_stage()) gets a
		low/high hex color pair here, persisted (Settings.
		landscape_zone_stage_colors) so a choice survives across relaunches --
		unlike the app's own Settings tab for Paths (RyzomPathsSection,
		shared with Patina), this stays local to landscape_editor.py since
		it's purely an Atyscape display concern (feedback-forgery-apps-own-
		settings: each app edits its own settings itself)."""
		imgui.separator()
		with imgui_ctx.begin_group():
			imgui.text_colored(_TAB_COLOR_SETTINGS, fa_icons.ICON_FA_PALETTE)
			imgui.same_line()
			imgui.text("Colors")
		stage_labels = {
			3: "✅ .zonel", 2: "✅ .zonew", 1: "✅ .zone", 0: "❌ none built yet",
		}
		settings = app_settings.load()
		color_flags = imgui.ColorEditFlags_.no_inputs.value
		for stage in (3, 2, 1, 0):
			imgui.text(stage_labels[stage])
			imgui.same_line()
			low, high = get_stage_colors(stage)
			changed_low, new_low = imgui.color_edit3(f"##settings-stage{stage}-low", low[:3], flags=color_flags)
			imgui.same_line()
			changed_high, new_high = imgui.color_edit3(f"##settings-stage{stage}-high", high[:3], flags=color_flags)
			if changed_low or changed_high:
				final_low = new_low if changed_low else low[:3]
				final_high = new_high if changed_high else high[:3]
				set_stage_colors(stage, final_low, final_high)
				self._rebuild_all_zone_nodes()
				settings.landscape_zone_stage_colors[str(stage)] = [rgb_to_hex(final_low), rgb_to_hex(final_high)]
				app_settings.save(settings)

	def _draw_landscape_tab(self):
		self._ensure_continent_locations_loaded()

		if self._pending_continent_reload:
			# Deferred from draw_panel()'s mode-change handling to here,
			# AFTER _ensure_continent_locations_loaded() above -- Edition
			# mode's bounds resolution (_select_continent()) looks up the
			# selection in self._cont_locs, which must already reflect the
			# new mode's own continent list by the time this runs.
			#
			# ContLoc.continent_name is NOT a stable cross-mode identifier
			# (found 2026-09-10, Nuno): in Release it's the packed_sheets/
			# sheet_id.bin identifier (e.g. "lecarrefour" for nexus), in
			# Edition it's ryzom.world's struct_name (the reliable
			# PacsRBank-style "nexus") -- re-selecting by the OLD mode's
			# continent_name would look up the wrong/nonexistent continent
			# in the new mode. selection_name (the displayed label, e.g.
			# "nexus" either way) is the one field both sources agree on --
			# already kept in self._selected_continent_pipeline_name.
			self._pending_continent_reload = False
			previous_selection_name = self._selected_continent_pipeline_name
			cont_loc = next((loc for loc in self._cont_locs or () if loc.selection_name == previous_selection_name), None)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_draw_landscape_tab) "
			      f"mode switch reload: selection_name={previous_selection_name!r} found={cont_loc}")
			if cont_loc is not None:
				self._select_continent(cont_loc.continent_name, cont_loc.selection_name)
			elif previous_selection_name is not None:
				# Not available under the new mode (e.g. no .land for it in
				# Edition) -- clear the stale selection/geometry rather than
				# keep showing the old mode's zones under a now-invalid name.
				self.selected_continent_name = None
				self._selected_continent_pipeline_name = None
				self.continent_bounds = None
				self._loaded_refs = {}
				self._loaded_extensions = {}
				self._all_continent_refs = {}
				self._all_continent_extensions = {}
				self._region_names = []
				self._region_zone_map = {}
				self._region_checked = {}
				self._land_cell_region_map = {}
				self._continent_z_range = None
				self._continent_loaded_grid_bounds = None
				self._rebuild_region_placeholders()
				self._set_loaded_zones({})

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
				self.orbit_camera.frame_bounds(min_x, min_y, max_x, max_y, margin=ZONE_CELL_SIZE)

		if self._region_names:
			imgui.separator()
			imgui.text("Regions (check to load, uncheck to unload)")
			for region_name in self._region_names:
				checked = self._region_checked.get(region_name, False)
				changed, new_value = imgui.checkbox(region_name, checked)
				if changed and new_value != checked:
					self._toggle_region(region_name)

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
					self._all_continent_refs = progress["default_refs"]
					self._all_continent_extensions = progress["extensions"]
					self._land_cell_region_map = progress.get("land_cell_region_map", {})
					self._continent_z_range = progress.get("continent_z_range")
					regions = progress.get("regions", [])
					self._continent_loaded_grid_bounds = self.continent_bounds
					if regions:
						# Lazy-by-region loading (project-todos/forgery/
						# landscape_editor__region_management.md step 3) --
						# nothing checked yet, so nothing loaded for real:
						# every zone starts as a purple placeholder square,
						# see _rebuild_region_placeholders() below. Exactly
						# one region on the continent is auto-checked (Nuno
						# 2026-09-13: "si le continent ne possède qu'une seule
						# région, il faut l'activer par défaut. Mais si le
						# continent a plus d'une région, on n'active rien
						# par défaut") -- more than one region, or none,
						# leaves every checkbox unchecked.
						self._region_names = regions
						self._region_zone_map = progress.get("zone_region_map", {})
						self._region_checked = {name: False for name in regions}
						if len(regions) == 1:
							self._region_checked[regions[0]] = True
						active_names = {
							name for name, region in self._region_zone_map.items()
							if self._region_checked.get(region, False)
						}
						self._loaded_refs = {name: self._all_continent_refs[name] for name in active_names}
						self._loaded_extensions = {
							name: self._all_continent_extensions.get(name, {}) for name in active_names
						}
					else:
						# No region_hierarchy entries for this continent --
						# pre-region_management behaviour, unchanged: every
						# zone loads immediately, no checkbox panel at all.
						self._region_names = []
						self._region_zone_map = {}
						self._region_checked = {}
						self._loaded_refs = self._all_continent_refs
						self._loaded_extensions = self._all_continent_extensions
					self._apply_render_mode()
					self._rebuild_region_placeholders()
			else:
				imgui.text("Scanning zone index...")

		if self._mode_reload_progress is not None:
			progress = self._mode_reload_progress
			if progress["done"]:
				self._mode_reload_progress = None
				print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:draw_panel) mode reload done, error={progress['error']!r} zones={len(progress['zones'])} gray={sorted(progress['gray'])}")
				if progress["error"]:
					self._zone_error = progress["error"]
				# The old whole-continent `.bam` cache used to be disabled
				# outright in Édition (found 2026-09-12, Nuno: an unexpected
				# stale cache hit after deleting a built zone -- see
				# project-todos/forgery/landscape_editor__land_composition.md
				# step 7's own decisions) -- the per-zone cache
				# (project-todos/forgery/landscape_editor__region_
				# management__zone_bam_cache.md step 5) replaces it in both
				# modes: each zone/land-piece invalidates on its OWN
				# mtime/size/rot/flip, so a deleted/changed file can never
				# make a DIFFERENT zone's cache entry look stale (or the
				# reverse), unlike the old single whole-continent bundle.
				self._set_loaded_zones(
					progress["zones"], gray=progress["gray"], manifest_zones=progress["manifest_zones"],
					mode=self.render_mode, grid_bounds=self._continent_loaded_grid_bounds, frame_camera=False,
				)
				# Cells with no real zone file at all show up as synthesized
				# "land:x:y" pseudo-names (see _run_load_refs()'s land
				# fallback) -- tracked separately from self._loaded_extensions
				# (which only ever knows about real, positioned zones) so
				# [WELD]'s "Generate missing .zonew" can report them as
				# blocked instead of silently ignoring them (step 9,
				# Nuno 2026-09-11).
				self._land_missing_cells = {name for name in progress["gray"] if name.startswith("land:")}
				self._recompute_missing_for_mode()
			else:
				total = progress["total"]
				processed = progress["processed"]
				fraction = processed / total if total else 0.0
				overlay = f"{processed}/{total} zones" if total else "Loading..."
				imgui.progress_bar(fraction, overlay=overlay)

		if self._ig_load_progress is not None and self._ig_load_progress["done"]:
			progress = self._ig_load_progress
			self._ig_load_progress = None
			self._ig_loaded_zone_names = progress["zone_names"]
			if progress["error"]:
				self._ig_error = progress["error"]
			else:
				self._ig_error = None
				self._set_loaded_ig(progress["instances_by_zone"])

		if self._weld_generate_progress is not None:
			progress = self._weld_generate_progress
			# Live, one zone at a time (Nuno 2026-09-11: waiting for the
			# whole batch before showing anything was worse than necessary)
			# -- each ready zone goes from gray fallback to real welded
			# geometry the moment its own .zonew is written, without
			# touching any zone still pending.
			ready = progress["ready"]
			while ready:
				name, cache_data, new_ref = ready.pop(0)
				self._loaded_extensions.setdefault(name, {})[".zonew"] = new_ref
				if self.render_mode == "WELD":
					self.zones[name] = cache_data
					self._gray_zones.discard(name)
					self._rebuild_zone_node(name, cache_data)
			self._recompute_missing_for_mode()
			if progress["done"]:
				self._weld_generate_progress = None
				if progress["error"]:
					self._zone_error = progress["error"]

		if self._zone_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._zone_error)

		# `.ig` instance loading (project-todos/forgery/
		# landscape_editor__region_management.md step 6) -- manual only,
		# triggered by the viewport toolbar's tree icon (_toggle_ig_
		# visibility(), not a panel button), never automatically by a
		# zone-set change (region checkbox, continent/zone selection): the
		# CPU cost (pure-Python .ig/.shape parsing, GIL-bound even off the
		# main thread) made it stutter the whole app on every single region
		# toggle (Nuno 2026-09-13).
		if self._ig_load_progress is not None and not self._ig_load_progress["done"]:
			imgui.text("Loading .ig instances...")
		if self._ig_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._ig_error)

		imgui.separator()
		self._draw_render_mode_bar()

		if self.zones:
			total_patches = sum(len(cache_data.patches) for cache_data in self.zones.values())
			imgui.text(f"{len(self.zones)} zone(s) loaded -- {total_patches} patches total")

		self._draw_land_composition_editor()
		self._draw_land_build_button()

	def _select_render_mode(self, mode):
		self.render_mode = mode
		self._apply_render_mode()

	def _draw_render_mode_bar(self):
		"""[LAND/POLY][WELD][LIGHT] button row (project-todos/forgery/
		landscape_editor__zone_render_modes.md step 6, renamed
		project-todos/forgery/landscape_editor__land_composition.md step 4) --
		the active mode is highlighted; WELD/LIGHT additionally show how many
		of the currently loaded zones are missing their own extension
		(rendered gray, see _resolve_zone_for_mode()). The internal "POLY"
		identifier (self.render_mode/_RENDER_MODES) is unchanged -- only Dev
		(Edition)'s displayed label becomes "LAND" (Release/Visualisation has
		no `.land` access at all, so it keeps showing "POLY" -- Nuno
		2026-09-12: LAND replaces POLY only where the `.land` actually drives
		the render).

		Release/Visualisation never shows this button row at all (Nuno
		2026-09-12): a real shipped `*_zones.bnp` only ever contains `.zonel`
		(confirmed empirically against `nexus_zones.bnp`, 155/155 entries
		`.zonel`, zero `.zone`/`.zonew`), so POLY/WELD/LIGHT would all
		resolve to the exact same file there -- three buttons producing an
		identical result, pure UI noise. `self.render_mode` itself is left
		untouched (still whatever it was, POLY by default) since it already
		resolves correctly either way; only the picker disappears, replaced
		by a plain "LIGHT" label (the semantically accurate name for
		finished, shipped data)."""
		if self._app_mode != _MODE_EDITION:
			imgui.text("Render mode: LIGHT")
			return
		for i, mode in enumerate(_RENDER_MODES):
			if i > 0:
				imgui.same_line()
			active = self.render_mode == mode
			label = "LAND" if (mode == "POLY" and self._app_mode == _MODE_EDITION) else mode
			if active:
				imgui.push_style_color(imgui.Col_.button.value, (0.35, 0.55, 0.35, 1.0))
			clicked = imgui.button(label)
			if active:
				imgui.pop_style_color()
			if clicked and not active:
				self._select_render_mode(mode)

		real_exts = _MODE_REAL_EXTENSIONS.get(self.render_mode)
		if real_exts is not None and self._loaded_refs:
			imgui.text(f"{self._missing_for_mode} zone(s) not {self.render_mode.lower()}ed ({'/'.join(real_exts)} missing)")
			# Generation is only wired for [WELD] (project-todos/forgery/
			# landscape_editor__zone_render_modes.md step 9's own scope --
			# [LIGHT] never gets a generation button, see the chantier's
			# "Hors scope explicite").
			if self.render_mode == "WELD" and self._missing_for_mode > 0:
				generating = self._weld_generate_progress is not None and not self._weld_generate_progress["done"]
				if generating:
					progress = self._weld_generate_progress
					total = progress["total"]
					processed = progress["processed"]
					fraction = processed / total if total else 0.0
					imgui.progress_bar(fraction, overlay=f"{processed}/{total} zone(s) welded")
				elif imgui.button(f"Generate {self._missing_for_mode} missing .zonew"):
					self._generate_missing_zonew()


def main(argv=None):
	LandscapeEditorApp().run()


if __name__ == "__main__":
	main()
