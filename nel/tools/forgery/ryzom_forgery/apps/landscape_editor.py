"""Ryzom Forgery landscape editor, branded "Atyscape" (Nuno, 2026-09-07 --
same naming convention as object_editor.py/"Patina"): 3D landscape
composition/build tool, replacing the current 2D bitmap-tile Ligo editor.

See project-todos/forgery/landscape_editor.md for the planned progressive
rendering steps. Visualisation/Edition mode-specific logic lives in the two
mixins this class inherits from (project-todos/forgery/
landscape_editor__land_preview.md step 5): `landscape_editor_edit_mode.py`
(EditModeMixin) and `landscape_editor_view_mode.py` (ViewModeMixin) --
shared rendering (zone_geometry.py/zone_cache.py/zone_geom_cache.py,
_apply_render_mode()) stays here, in the base class.
"""

import math
import threading
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, imgui_ctx
from panda3d.core import NodePath, Point3, Quat, TransparencyAttrib, Vec3

from pynel import repository_paths
from pynel.ryzom_bnp import BnpError
from pynel.ryzom_ig import parse_ig, IgParseError
from pynel.ryzom_zone import parse_zone, ZoneParseError

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.apps.landscape_editor_edit_mode import EditModeMixin
from ryzom_forgery.apps.landscape_editor_modes import (
	_detect_app_mode, _MODE_BADGE_COLOR, _MODE_BADGE_LABEL, _MODE_EDITION, _MODE_VISUALISATION, _OTHER_MODE,
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
	find_zones_in_region, get_continent_z_range, get_zone_extensions_index, has_pipeline_export, load_zone_cache_data,
	RegionLoadError, zone_ref_extension,
)
from ryzom_forgery import continent_pipeline_reference as cpr
from ryzom_forgery import ig_full_geom_cache
from ryzom_forgery.icon_colors import pastel_color_for
from ryzom_forgery import ig_full_load
from ryzom_forgery import ig_geometry
from ryzom_forgery.ryzom_paths_section import RyzomPathsSection
from ryzom_forgery.search_paths_dialog import SearchPathsDialog
from ryzom_forgery import settings as app_settings
from ryzom_forgery import zone_geom_cache
from ryzom_forgery.zone_cache import ZoneCacheData
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

# World-space Z nudge applied to every `.ig` water instance on top of its own
# real position (project-todos/forgery/landscape_editor__ig_full_load.md,
# Nuno 2026-09-14: water/terrain z-fighting, set_depth_offset() confirmed to
# have no visible effect) -- see _attach_textured_ig_instance()'s own comment
# for why a real position change, not a depth-buffer trick, is the fix here.
_WATER_Z_LIFT = 0.15

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

# "Stats" section (project-todos/forgery/landscape_editor__render_modes_
# removal.md, Nuno 2026-09-14: "illisible... il faut de la couleur") --
# header/total lines in a warm amber, table values in a cool cyan, so both
# stand out against the plain white body text around them.
_STATS_HEADER_COLOR = (1.0, 0.8, 0.4, 1.0)
_STATS_VALUE_COLOR = (0.55, 0.85, 1.0, 1.0)

# "Comme Patina" (Nuno 2026-09-14) -- an identifier (zone/brick name) in
# orange, a coordinate/measured value in green, same convention as
# landscape_editor_edit_mode.py's own _NAME_COLOR/_VALUE_COLOR (kept as a
# separate copy there to avoid a cross-mixin import, same reasoning as
# this module's own docstring on landscape_editor_modes.py).
_NAME_COLOR = (1.0, 0.75, 0.3, 1.0)
_VALUE_COLOR = (0.6, 0.9, 0.5, 1.0)

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
		# "Low Poly" terrain toggle (project-todos/forgery/landscape_editor__
		# low_poly_mode.md, Nuno 2026-09-14, for weak GPUs) -- halves each
		# patch's tessellation grid; persisted like landscape_editor_mode.
		self._low_poly = settings.landscape_low_poly

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
		# landscape_editor__transparency_wireframe.md) -- independent of each
		# other, applied straight to self._zone_root so they cover whatever
		# is currently loaded there.
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

		# `.ig` instance display (project-todos/forgery/
		# landscape_editor__ig_full_load.md) -- loads and shows/hides
		# fully-textured `.bam` bundles, split in two (2026-09-13 revision,
		# after a first "everything in one bam" version proved unusable --
		# "y en a des tonnes, impossible de tout afficher en même temps"):
		# zone-owned `.ig`s (e.g. "215_ed.ig") follow the existing per-region
		# checkboxes (one `.bam` per region, loaded automatically when its
		# region is checked), everything else (villages/water/..., never
		# sky/canopy) is one continent-wide `.bam`, built automatically
		# whenever a continent loads (_load_ig_rest()). Never the per-visible-zone
		# incremental resolution the original mechanism used (deleted this
		# same chantier, step 1). Resolves each instance's `.shape`/textures
		# by name via search_paths_dialog (same generic,
		# .bnp-aware index Patina already uses for textures/.skel/.anim, see
		# search_paths_dialog.py's own module docstring), so a normal, unlit
		# Settings surface isn't needed here -- it's the same search paths
		# already configured for the whole suite (see explorer_root above).
		self.search_paths_dialog = SearchPathsDialog()
		# Two independent visibility roots (Nuno 2026-09-13: "il faudrait
		# séparer l'affichage des 2 [...] 2 .bam différents, 2 boutons
		# différents") -- zone-owned .ig (per-region bundles) and the
		# continent-wide "rest" bundle (villages/water) are two genuinely
		# different `.bam` sets, shown/hidden independently: the viewport
		# toolbar's tree icon controls _ig_region_root only, a second icon
		# right next to it controls _ig_rest_root only.
		self._ig_root = self.render.attach_new_node("ig-root")
		self._ig_region_root = self._ig_root.attach_new_node("ig-region-root")
		self._ig_rest_root = self._ig_root.attach_new_node("ig-rest-root")
		self._ig_region_nodes = {}  # region name -> NodePath (that region's own zone-.ig bundle)
		self._ig_region_progress = {}  # region name -> progress dict, in-flight builds
		self._ig_rest_np = None  # NodePath of the continent-wide "rest" bundle (villages/water/...), or None
		self._ig_rest_progress = None
		# Checkbox-tree state (project-todos/forgery/
		# landscape_editor__ig_inspector_tree.md) -- region name -> {ig_name:
		# entry}/{ig_name: entry} for IG Zones/IG Others respectively, each
		# entry {"checked", "shapes": {shape_name: bool}, "shape_nodes":
		# {shape_name: NodePath}}. Rebuilt from scratch (never persisted)
		# every time a bundle is (re)shown -- see _build_ig_tree_entries().
		self._ig_tree_state_zones = {}
		self._ig_tree_state_others = {}
		# Hidden by default (Nuno 2026-09-13: "il faut désactiver l'affichage
		# des ig par défaut") -- now that both bundles auto-load on continent
		# selection (see the "rest" one's own _load_ig_rest() call, and a
		# single region's own auto-check, both in draw_panel()'s continent-
		# scan completion handler), showing everything immediately by
		# default would clutter the view before the user asks for it.
		self._ig_region_visible = False
		self._ig_rest_visible = False
		self._ig_region_root.hide()
		self._ig_rest_root.hide()
		self._ig_error = None

		# Loaded zone set state. _loaded_refs (name -> default ZoneRef, the
		# region_loader.py .zonel > .zonew > .zone priority) and
		# _loaded_extensions (name -> {ext: ZoneRef}) describe the currently
		# loaded zone set (single Explorer selection or a whole continent) --
		# _apply_render_mode() reloads the actual geometry from these
		# whenever the set changes, without re-scanning the disk index.
		self._loaded_refs = {}
		self._loaded_extensions = {}
		self._mode_reload_progress = None
		# "Reload cache" buttons (project-todos/forgery/landscape_editor__
		# render_modes_removal.md, left panel) -- self._region_cache_reload_
		# progress is the single-region background reload in flight (None
		# while idle, polled in _draw_continent_and_regions());
		# self._continent_cache_reload_queue holds the region names still
		# left to process for a whole-continent reload ("region par
		# région"), drained one at a time via that same single-region
		# mechanism -- empty means no continent-wide reload is running.
		self._region_cache_reload_progress = None
		self._continent_cache_reload_queue = []
		# Incremental region check/uncheck (project-todos/forgery/
		# landscape_editor__render_modes_removal.md perf fix, Nuno
		# 2026-09-14) -- self._region_incremental_progress is the single
		# region-check background load in flight (None while idle, polled
		# in _draw_continent_and_regions()); self._region_incremental_queue
		# holds region names checked again while one was already loading,
		# drained one at a time once the current one finishes. Unchecking
		# never needs either (pure local teardown, see
		# _unload_region_incremental()).
		self._region_incremental_progress = None
		self._region_incremental_queue = []

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
		self._region_bounds = {}  # region name -> (min_x, min_y, max_x, max_y), for the "center camera" button
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
		# Total `.land` cell count for the loaded continent (Édition only,
		# project-todos/forgery/landscape_editor__render_modes_removal.md
		# "Continent status" section) -- None outside Édition/before a
		# continent has loaded.
		self._continent_total_used_cells = None
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
		# Last parsed `.land` for the fallback-piece background loader
		# (project-todos/forgery/landscape_editor__render_modes_removal.md,
		# Nuno 2026-09-14 perf fix) -- (land_path, mtime, Land) or None, see
		# EditModeMixin._load_land_cached()'s own docstring.
		self._land_fallback_cache = None

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
		# as self._mode_reload_progress.
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
		# Same self-measurement trick as self._viewport_toggle_size, for the
		# north compass (project-todos/forgery/landscape_editor__render_
		# modes_removal.md, bottom-right of the 3D viewport).
		self._compass_size = (10.0, 10.0)

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
		self._selected_zone_bounds = None  # (min_x, min_y, max_x, max_y), project-todos/forgery/landscape_editor__selected_zone_info.md
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
		self._selected_zone_bounds = (min_x, min_y, max_x, max_y)
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
		self._selected_zone_bounds = None
		self._zone_selection_np.remove_node()
		self._zone_selection_np = self.render.attach_new_node("zone-selection-border-placeholder")

	def on_selection_changed(self, items):
		"""A single .zone*/.bnp-contained zone picked in the Explorer -- loads
		exactly the clicked file. Not a real usage pattern (Nuno always loads
		a whole continent via the combo, never browses the Explorer for
		this, 2026-09-09) -- kept simple/direct rather than routed through
		_apply_render_mode()'s resolution, which only matters for continent
		loads."""
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
		# A single-zone selection isn't continent-based -- no region
		# checkboxes/purple squares apply to it (project-todos/forgery/
		# landscape_editor__region_management.md).
		self._all_continent_refs = {}
		self._all_continent_extensions = {}
		self._region_names = []
		self._region_bounds = {}
		self._region_zone_map = {}
		self._region_checked = {}
		self._land_cell_region_map = {}
		self._continent_z_range = None
		self._continent_total_used_cells = None
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
		polling of `progress` hands the result to _apply_render_mode(), which
		does the expensive per-zone load itself, in its own background pass.
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
			total_used_cells = None
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
				# Total `.land` cell count (project-todos/forgery/
				# landscape_editor__render_modes_removal.md "Statut du
				# continent" section) -- unlike land_cell_region_map (only
				# cells that fall inside a real region polygon), this counts
				# EVERY used cell regardless of region, correct even for a
				# continent with no region_hierarchy entries at all.
				total_used_cells = len(all_used_cells)
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
			# Region polygon bounds (already in world units, same space as
			# zone bb_center -- see assign_zones_to_regions()'s own
			# docstring), for the left panel's "center camera on this
			# region" button -- computed once here since `regions` (with
			# its own `.points`) is only ever available on this background
			# thread, never kept around after progress["regions"] discards
			# everything but the bare name.
			progress["region_bounds"] = {
				r.name: (
					min(p[0] for p in r.points), min(p[1] for p in r.points),
					max(p[0] for p in r.points), max(p[1] for p in r.points),
				)
				for r in regions if r.points
			}
			progress["zone_region_map"] = assign_zones_to_regions(zone_positions, regions)
			print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_run_load_continent) "
			      f"zones found for {pipeline_continent_name!r}: {len(default_refs)}")
			progress["default_refs"] = default_refs
			progress["extensions"] = extensions
			progress["land_cell_region_map"] = land_cell_region_map
			progress["total_used_cells"] = total_used_cells
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
		# 30% transparent (zone_geometry._ZONE_PLACEHOLDER_COLOR's own alpha)
		# AND lowest possible render priority (Nuno 2026-09-14: "il faut
		# qu'ils soit en priorité la plus basse possible, tout peux se
		# superposer") -- "background" bin (Panda3D's own lowest-priority
		# built-in bin, drawn before everything else) + no depth write, so
		# real geometry loaded later (a region getting checked, an .ig
		# bundle) never gets hidden behind a placeholder still sitting at
		# the same spot.
		self._region_placeholder_np.set_transparency(TransparencyAttrib.M_alpha)
		self._region_placeholder_np.set_bin("background", 0)
		self._region_placeholder_np.set_depth_write(False)

	def _toggle_region(self, region_name):
		"""Flips `region_name`'s checkbox (project-todos/forgery/
		landscape_editor__region_management.md step 4) -- INCREMENTAL since
		project-todos/forgery/landscape_editor__render_modes_removal.md's
		perf fix (Nuno 2026-09-14: checking a 2nd/3rd/... region used to
		reprocess every previously-checked region's zones all over again
		through _apply_render_mode()'s full-union reload, an O(n^2) cost
		across a sequence of checks -- measured 347 zones taking ~1.2s total
		via (IA_AGENT_DEBUG) timing, most of it re-doing work for zones
		already on screen). Checking now only loads THIS region's own zones
		in the background (_load_region_incremental()) and merges them in;
		unchecking only tears down THIS region's own zones
		(_unload_region_incremental()) -- neither ever touches any other
		region's already-loaded geometry. Also loads/unloads that region's
		own zone-`.ig` bundle (project-todos/forgery/
		landscape_editor__ig_full_load.md step 5) -- independent of the
		zone geometry above, its own background build/cache."""
		checking = not self._region_checked.get(region_name, False)
		self._region_checked[region_name] = checking
		self._rebuild_region_placeholders()
		if checking:
			self._load_region_ig(region_name)
			self._load_region_incremental(region_name)
		else:
			self._hide_ig_region(region_name)
			self._unload_region_incremental(region_name)

	def _load_region_incremental(self, region_name):
		"""Background-loads exactly `region_name`'s own zones -- its real
		refs (self._region_zone_map) AND its own `.land` fallback pieces
		(allowed_cells restricted to this region alone, same mechanism
		_apply_render_mode() used for the whole checked set) -- merging the
		result into whatever's already displayed once done, in
		_draw_continent_and_regions()'s own polling. A load already running
		queues this region (self._region_incremental_queue) instead of
		starting a second one; drained one at a time from that same
		polling once the current load finishes."""
		if self._region_incremental_progress is not None and not self._region_incremental_progress["done"]:
			if region_name not in self._region_incremental_queue:
				self._region_incremental_queue.append(region_name)
			return
		new_names = {name for name, r in self._region_zone_map.items() if r == region_name}
		refs = {name: self._all_continent_refs[name] for name in new_names}
		self._loaded_refs.update(refs)
		self._loaded_extensions.update({name: self._all_continent_extensions.get(name, {}) for name in new_names})
		land_fallback = None
		allowed_cells = None
		if self._app_mode == _MODE_EDITION and self.selected_continent_name:
			land_fallback = self._resolve_land_fallback_paths()
			if land_fallback is not None:
				allowed_cells = {cell for cell, r in self._land_cell_region_map.items() if r == region_name}
		progress = {
			"done": False, "error": None, "zones": {}, "manifest_zones": {},
			"total": len(refs), "processed": 0, "region": region_name,
		}
		self._region_incremental_progress = progress
		thread = threading.Thread(
			target=self._run_load_refs, args=(refs, progress, land_fallback, allowed_cells), daemon=True,
		)
		thread.start()

	def _unload_region_incremental(self, region_name):
		"""Tears down exactly `region_name`'s own currently-displayed zones
		-- its real zones (self._region_zone_map) plus any `.land` fallback
		pieces whose own origin cell belongs to it (self._land_cell_region_
		map) -- without touching any other region's zones. Purely local
		state/NodePath teardown, no background thread needed."""
		real_names = {name for name, r in self._region_zone_map.items() if r == region_name}
		land_names = set()
		for name in self.zones:
			if not name.startswith("land:"):
				continue
			_, ox, oy = name.split(":")
			if self._land_cell_region_map.get((int(ox), int(oy))) == region_name:
				land_names.add(name)
		for name in real_names | land_names:
			self._loaded_refs.pop(name, None)
			self._loaded_extensions.pop(name, None)
			node = self._zone_nodes.pop(name, None)
			if node is not None:
				node.remove_node()
			self.zones.pop(name, None)

	def _zone_refs_for_region(self, region_name):
		"""Every real zone `ZoneRef` belonging to `region_name`, whether that
		region is currently checked or not (project-todos/forgery/
		landscape_editor__render_modes_removal.md "reload cache" buttons) --
		unlike `self._loaded_refs` (only the checked subset), reads straight
		from `self._all_continent_refs`/`self._region_zone_map`, populated
		once for the WHOLE continent at scan time."""
		return {
			name: ref for name, ref in self._all_continent_refs.items()
			if self._region_zone_map.get(name) == region_name
		}

	def _reload_region_cache(self, region_name):
		"""Forces a fresh rebuild + re-save of `region_name`'s own per-zone
		caches -- BOTH of them (project-todos/forgery/landscape_editor__
		render_modes_removal.md, Nuno 2026-09-14: "invalider les 2 caches"):
		the position cache (`zone_cache.py`, bypassed via `force=True` on
		`load_zone_cache_data()`) AND the built-geometry `.bam` cache
		(`zone_geom_cache.py`, bypassed by never even attempting the
		manifest check that would otherwise skip the position reload --
		see `_run_load_refs()`'s own early-skip -- and by never trying
		`_rebuild_zone_node()`'s own `.bam` read either, see its
		consumer's own comment) -- without touching any other region's own
		caches or display. If `region_name` is currently checked, its live
		geometry is also patched in place (`_rebuild_zone_node()`, the same
		incremental-update primitive the Build progress drain already
		uses) as soon as the background load is done; if unchecked, only
		the on-disk caches are refreshed -- the region stays a purple
		placeholder, nothing changes on screen. A second call while one
		region reload is already running is ignored (same convention as
		every other background op in this file)."""
		if self._region_cache_reload_progress is not None and not self._region_cache_reload_progress["done"]:
			return
		refs = self._zone_refs_for_region(region_name)
		if not refs:
			return
		progress = {
			"done": False, "error": None, "zones": {}, "manifest_zones": {},
			"total": len(refs), "processed": 0, "region": region_name, "force": True,
		}
		self._region_cache_reload_progress = progress
		thread = threading.Thread(target=self._run_load_refs, args=(refs, progress), daemon=True)
		thread.start()

	def _reload_continent_cache(self):
		"""Forces a fresh rebuild + re-save of EVERY region's own per-zone
		`.bam` cache for the currently selected continent, one region at a
		time (project-todos/forgery/landscape_editor__render_modes_removal.md
		"reload cache" button, Nuno 2026-09-14: "il recharge TOUT le
		continent, région par région") -- queues every name in
		self._region_names and drains it through _reload_region_cache()
		itself, one region per completed background load (see
		_draw_continent_and_regions()'s own polling, which advances the
		queue). A region that's currently unchecked still gets its cache
		regenerated on disk (see _reload_region_cache()'s own docstring),
		it just never appears on screen. A second call while one is already
		queued/running is ignored."""
		if self._continent_cache_reload_queue:
			return
		if self._region_cache_reload_progress is not None and not self._region_cache_reload_progress["done"]:
			return
		if not self._region_names:
			return
		self._continent_cache_reload_queue = list(self._region_names)
		self._advance_continent_cache_reload()

	def _advance_continent_cache_reload(self):
		"""Pops the next region off self._continent_cache_reload_queue and
		starts its own reload -- called once up front by
		_reload_continent_cache() and again by _draw_continent_and_regions()
		every time a queued region's own reload finishes, until the queue
		is empty."""
		if not self._continent_cache_reload_queue:
			return
		region_name = self._continent_cache_reload_queue.pop(0)
		self._reload_region_cache(region_name)

	def _apply_render_mode(self, force=False):
		"""Re-resolves the currently loaded zone set (self._loaded_refs/
		self._loaded_extensions, populated by on_selection_changed()/
		_load_continent()) and reloads it in a background thread -- always
		each zone's own default ref (region_loader.py's own
		.zonel > .zonew > .zone priority, already baked into
		self._loaded_refs), a `.land`+brick fallback filling in anything
		still missing entirely (project-todos/forgery/
		landscape_editor__render_modes_removal.md: replaces the old per-mode
		POLY/WELD/LIGHT resolution -- a single always-on mode now, same
		behaviour POLY already had). A second call while one reload is
		already running is ignored (the caller will get another chance once
		it's done, since loaded_refs/loaded_extensions are read fresh here,
		not snapshotted). `force` is forwarded to _set_loaded_zones() once
		the background load is done (see `progress["force"]` below) -- the
		left panel's "reload cache" buttons (same chantier) use it to bypass
		the per-zone `.bam` cache and force a fresh rebuild+re-save."""
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

		refs = dict(self._loaded_refs)

		# `land_fallback` itself was already resolved above (before the
		# early-return) -- the .land cell's own (pos_x, pos_y) plus which
		# ones are ALREADY covered by `refs` (known only once _run_load_refs()
		# has actually loaded them, bb_center) still need the background
		# thread below, same as the real refs -- a .land cell with no
		# matching real file is rendered as the raw brick instead of staying
		# invisible.

		self._zone_error = None
		progress = {
			"done": False, "error": None, "zones": {}, "manifest_zones": {},
			"total": len(refs), "processed": 0, "force": force,
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
			force = progress.get("force", False)
			continent_z_range = self._continent_z_range
			for name, ref in refs.items():
				try:
					stat = ref.source_path.stat()
					entry = ZoneManifestEntry(zone_ref_extension(ref), stat.st_mtime, stat.st_size)
					manifest_zones[name] = entry
					cache_data = None
					if not force and continent_z_range is not None:
						min_z, max_z = continent_z_range
						# Cheap manifest-only read (no `.bam` payload
						# touched) -- if it already matches what
						# _set_loaded_zones() will want a moment later on
						# the main thread, the `.bam` geometry cache is
						# confirmed to hit, so the far more expensive
						# zone_cache.py position-cache load below (~3ms/
						# zone measured, mostly pickle unpacking thousands
						# of individual Python floats) is skipped entirely
						# -- only bb_center/bb_half_size (carried on the
						# manifest itself, see ZoneGeomManifest's own
						# docstring) are needed from here on for a
						# confirmed hit; `patches` stays empty, never
						# touched unless _set_loaded_zones()'s own
						# (identical) check somehow disagrees moments
						# later -- an unrealistic race, nothing plausible
						# changes a zone's `.bam`/the whole continent's
						# elevation range mid-operation.
						cached_manifest = zone_geom_cache.read_manifest(name, self._zone_cache_mode())
						if (cached_manifest is not None and cached_manifest.entry == entry
						        and cached_manifest.min_z == min_z and cached_manifest.max_z == max_z):
							cache_data = ZoneCacheData(
								patches=(), bb_center=cached_manifest.bb_center,
								bb_half_size=cached_manifest.bb_half_size,
							)
					if cache_data is None:
						cache_data = load_zone_cache_data(ref, force=force, low_poly=self._low_poly)
					zones[name] = cache_data
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

	def _read_ig_shape_bytes(self, shape_name, priority_paths=None):
		"""Resolves+reads one `.ig` instance's referenced `.shape` by bare
		name (e.g. "pr_s3_amoeba_c.shape") via self.search_paths_dialog's own
		generic, `.bnp`-aware name index (search_paths_dialog.py's own module
		docstring already lists `.shape` among what it resolves) -- the exact
		same index Patina uses for textures/.skel/.anim, just looked up here
		instead of drawn as a Settings tab (see search_paths_dialog's own
		field comment in __init__). None if not found/unreadable. Called from
		_build_ig_nodepath()'s background thread -- find_file()'s own
		index read is a plain dict `.get()`, safe under the GIL even while
		_advance_external_scan() (draw(), main thread) concurrently rebuilds
		it (dict reassignment is atomic, see search_paths_dialog.py's own
		_merge_and_publish()).

		`priority_paths`, forwarded to find_file() as-is, is Atyscape's own
		pipeline/export/continents/<continent> directory (Édition mode,
		_shape_priority_paths()) -- found 2026-09-14, Nuno: a real `.ig`
		instance's own `.shape` name collided with an unrelated, generic
		shape of the same bare name reachable through the user's own shared
		Search Paths, which silently won since it happened to be listed
		first -- forcing the real per-continent pipeline directory first
		removes that dependency on manual Search Paths ordering entirely."""
		found = self.search_paths_dialog.find_file(shape_name, priority_paths)
		if found is None:
			return None
		try:
			return found.read_bytes()
		except (OSError, BnpError):
			return None

	def _shape_priority_paths(self, ryzom_data_path, app_mode, continent):
		"""The directories Atyscape's own `.ig` `.shape` lookups
		(_read_ig_shape_bytes()) must always resolve from first, if any --
		`pipeline/export/continents/<continent>/` AND
		`pipeline/export/ecosystems/<ecosystem>/` (Édition mode only: the
		real per-continent/per-ecosystem build output; Visualisation has no
		such tree, live_data_path's own `.bnp`s are already continent-scoped
		by archive name, no collision risk). Found 2026-09-14, Nuno: a real
		`.ig` instance's own `.shape` (e.g. a `tr_villagea.ig` building,
		`tr_agora_village_a.shape`) lives under the ECOSYSTEM export tree
		(`pipeline/export/ecosystems/lacustre/shape_optimized/`, shared props
		reused across continents of the same ecosystem), not the continent
		one -- a continent-only priority path missed those entirely and fell
		through to the shared, unordered index. `continent`'s own ecosystem
		comes from `continent_pipeline_reference.csv`'s `ecosystem` row
		(`cpr.load_reference_table()`) -- silently dropped (continent-only
		priority) if the CSV can't be read. `[]` (no override -- find_file()
		falls straight through to the normal shared index) if `ryzom_data_path`
		or `continent` is unset."""
		if app_mode != _MODE_EDITION or not ryzom_data_path or not continent:
			return []
		export_root = Path(ryzom_data_path) / "pipeline" / "export"
		paths = [str(export_root / "continents" / continent)]
		try:
			ecosystem = cpr.load_reference_table(ryzom_data_path)[continent]["ecosystem"]
		except (cpr.ContinentPipelineReferenceError, KeyError):
			ecosystem = None
		if ecosystem:
			paths.append(str(export_root / "ecosystems" / ecosystem))
		return paths

	def _build_ig_nodepath(self, refs, progress, root_name, priority_paths=None):
		"""Shared background-thread body for building a detached, textured
		NodePath from `refs` (ig_full_load.FullIgRef list) -- parses each
		`.ig` (ryzom_ig.parse_ig()/ig_geometry.resolve_ig_instances(), which
		itself calls pynel.ryzom_shape.parse_shape() per unique referenced
		`.shape`), and builds the actual textured Panda3D geometry
		(ig_geometry.build_textured_instance_template(), which decodes real
		texture bytes via shape_geometry.load_panda_texture()) -- unlike the
		old per-zone mechanism this replaces (deleted this same chantier,
		step 1), which deliberately kept Panda3D object creation on the main
		thread. Safe here because every node built (`root` and everything
		under it) stays a fully DETACHED subtree -- never part of the live,
		currently-rendered scene graph -- until the caller's own poll
		reparents the finished `root` under `self._ig_root` on the main
		thread in one cheap call; nothing here ever touches `self._ig_root`
		or any other node Panda3D might be traversing to render the current
		frame. Updates `progress`'s "total"/"loaded"/"instances" fields as it
		goes. Returns `(root, total_instances)` -- used for both a region's
		own zone-`.ig` bundle (_run_load_region_ig()) and the continent-wide
		"rest" bundle (_run_load_ig_rest())."""
		progress["total"] = len(refs)
		root = NodePath(root_name)
		shape_templates = {}
		texture_cache = {}
		total_instances = 0
		failed = []
		read_shape_bytes = lambda name: self._read_ig_shape_bytes(name, priority_paths)
		for i, ref in enumerate(refs):
			progress["message"] = f"{ref.name}.ig"
			try:
				data = ig_full_load.read_ig_ref_bytes(ref)
				ig = parse_ig(data)
			except (OSError, BnpError, IgParseError) as exc:
				failed.append(f"{ref.name}.ig: {exc}")
				progress["loaded"] = i + 1
				continue
			resolved_list = ig_geometry.resolve_ig_instances(ig, read_shape_bytes)
			if resolved_list:
				ig_np = root.attach_new_node(f"ig-{ref.name}")
				# One child NodePath per unique shape name under this .ig
				# (project-todos/forgery/landscape_editor__ig_inspector_tree.md
				# step 2) -- lets the future checkbox tree show/hide a single
				# shape's instances (`.hide()`/`.show()` on its own shape_np)
				# without touching the rest of the .ig. Scoped per-ref (reset
				# for every `.ig`, unlike `shape_templates`/`texture_cache`
				# above, which stay process-wide across every ref in this
				# build): the SAME shape name can appear under several
				# different `.ig` files, each needing its own group node.
				shape_group_nodes = {}
				for resolved in resolved_list:
					if self._attach_textured_ig_instance(
						ig_np, resolved, shape_templates, texture_cache, shape_group_nodes,
					) is not None:
						total_instances += 1
			progress["loaded"] = i + 1
			progress["instances"] = total_instances
		if failed:
			progress["error"] = f"{len(failed)}/{len(refs)} .ig failed to load: {'; '.join(failed[:3])}"
			report_error(progress["error"])
		return root, total_instances

	def _load_ig_rest(self, force=False):
		"""Kicks off a background build of every real `.ig` belonging to the
		current continent that is NEITHER zone-owned NOR sky/canopy
		(villages, water, ...): ig_full_load.split_refs()'s own `rest_refs`
		(project-todos/forgery/landscape_editor__ig_full_load.md step 6).
		Zone-owned `.ig`s load per-region instead (_load_region_ig(),
		triggered by _toggle_region()). Called automatically whenever a
		continent loads (project-todos/forgery/landscape_editor__render_
		modes_removal.md removed the panel's own manual trigger button, this
		auto-call was already in place before that); a build already
		running is left to finish, a fresh call while one is in flight is a
		no-op (this function's own guard). `force` (same chantier, viewport
		toolbar right-click "Regenerate cache") skips the cache read below,
		forcing a fresh rebuild -- write_ig_bundle() still overwrites the
		on-disk `.bam` with the fresh result either way."""
		continent = self._selected_continent_pipeline_name
		if not continent:
			return
		app_mode = self._app_mode
		# Found 2026-09-13, Nuno: "je dois charger les .ig en plus (9) mais
		# ils ne sont jamais mis dans le .bam" -- write_ig_bundle() WAS
		# already called at the end of every build, but nothing ever read
		# it back: every click rebuilt from scratch instead of reusing a
		# `.bam` from an earlier session/click, unlike _load_region_ig(),
		# which already checked its own cache first.
		cached = None if force else ig_full_geom_cache.read_ig_bundle(continent, app_mode, None, self.loader)
		if cached is not None:
			self._show_ig_rest(cached)
			return
		if self._ig_rest_progress is not None and not self._ig_rest_progress["done"]:
			return
		live_data_path = app_settings.load().live_data_path
		ryzom_data_path = repository_paths.get("ryzom-data")
		progress = {
			"done": False, "error": None, "total": 0, "loaded": 0, "instances": 0,
			"continent": continent, "mode": app_mode, "node_path": None,
		}
		self._ig_rest_progress = progress
		thread = threading.Thread(
			target=self._run_load_ig_rest,
			args=(live_data_path, ryzom_data_path, self._app_mode, continent, progress),
			daemon=True,
		)
		thread.start()

	def _run_load_ig_rest(self, live_data_path, ryzom_data_path, app_mode, continent, progress):
		"""Background-thread body for _load_ig_rest()."""
		try:
			is_edition = app_mode == _MODE_EDITION
			refs = ig_full_load.list_continent_ig_refs(live_data_path, ryzom_data_path, is_edition, continent)
			sky_names = ig_full_load.sky_ig_names(live_data_path, ryzom_data_path, is_edition, continent)
			_zone_refs, rest_refs = ig_full_load.split_refs(refs, sky_names)
			priority_paths = self._shape_priority_paths(ryzom_data_path, app_mode, continent)
			root, total_instances = self._build_ig_nodepath(rest_refs, progress, "ig-rest-root", priority_paths)
			ig_full_geom_cache.write_ig_bundle(continent, app_mode, None, root)
			progress["node_path"] = root
		except OSError as exc:
			progress["error"] = str(exc)
			report_error(progress["error"])
		finally:
			progress["done"] = True

	def _regenerate_region_ig_caches(self):
		"""Viewport toolbar tree icon's right-click "Regenerate cache"
		(project-todos/forgery/landscape_editor__render_modes_removal.md) --
		force-reloads every currently CHECKED region's own zone-`.ig` bundle
		(_load_region_ig(force=True)); unchecked regions have nothing
		displayed to refresh, so they're skipped."""
		for region_name, checked in self._region_checked.items():
			if checked:
				self._load_region_ig(region_name, force=True)

	def _load_region_ig(self, region_name, force=False):
		"""Loads `region_name`'s own zone-owned `.ig` bundle -- called from
		_toggle_region() right after that region gets CHECKED (project-todos/
		forgery/landscape_editor__ig_full_load.md step 5). Tries
		ig_full_geom_cache.read_ig_bundle() first (a `.bam` from an earlier
		session/region-check); builds fresh in the background only if none
		exists yet. A build already running for this region is left to
		finish, a fresh call while one is in flight is a no-op. `force`
		(project-todos/forgery/landscape_editor__render_modes_removal.md,
		viewport toolbar right-click "Regenerate cache") skips the cache
		read below, same as _load_ig_rest()'s own `force`."""
		continent = self._selected_continent_pipeline_name
		if not continent:
			return
		app_mode = self._app_mode
		live_data_path = app_settings.load().live_data_path
		ryzom_data_path = repository_paths.get("ryzom-data")
		cached = None if force else ig_full_geom_cache.read_ig_bundle(continent, app_mode, region_name, self.loader)
		if cached is not None:
			self._show_ig_region(region_name, cached)
			return
		existing = self._ig_region_progress.get(region_name)
		if existing is not None and not existing["done"]:
			return
		progress = {
			"done": False, "error": None, "total": 0, "loaded": 0, "instances": 0,
			"continent": continent, "mode": app_mode, "region": region_name, "node_path": None,
		}
		self._ig_region_progress[region_name] = progress
		thread = threading.Thread(
			target=self._run_load_region_ig,
			args=(live_data_path, ryzom_data_path, app_mode, continent, region_name, progress),
			daemon=True,
		)
		thread.start()

	def _run_load_region_ig(self, live_data_path, ryzom_data_path, app_mode, continent, region_name, progress):
		"""Background-thread body for _load_region_ig() -- filters
		ig_full_load's zone-owned refs down to `region_name`'s own zones via
		self._region_zone_map (already computed for zone geometry loading,
		project-todos/forgery/landscape_editor__region_management.md's own
		zone -> region assignment)."""
		try:
			is_edition = app_mode == _MODE_EDITION
			refs = ig_full_load.list_continent_ig_refs(live_data_path, ryzom_data_path, is_edition, continent)
			sky_names = ig_full_load.sky_ig_names(live_data_path, ryzom_data_path, is_edition, continent)
			zone_refs, _rest_refs = ig_full_load.split_refs(refs, sky_names)
			region_refs = [ref for ref in zone_refs if self._region_zone_map.get(ref.name) == region_name]
			priority_paths = self._shape_priority_paths(ryzom_data_path, app_mode, continent)
			root, total_instances = self._build_ig_nodepath(
				region_refs, progress, f"ig-region-{region_name}-root", priority_paths,
			)
			ig_full_geom_cache.write_ig_bundle(continent, app_mode, region_name, root)
			progress["node_path"] = root
		except OSError as exc:
			progress["error"] = str(exc)
			report_error(progress["error"])
		finally:
			progress["done"] = True

	def _attach_textured_ig_instance(self, parent_np, resolved, shape_templates, texture_cache, shape_group_nodes):
		"""Attaches one real `.ig` instance under `parent_np`, via an
		intermediate per-`.shape`-name group NodePath (`shape_group_nodes`,
		project-todos/forgery/landscape_editor__ig_inspector_tree.md step 2
		-- one child of `parent_np` per unique `resolved.shape_name` under
		this `.ig`, so a future checkbox tree can show/hide every instance
		of one shape with a single `.hide()`/`.show()` on its own group
		node), and reusing a per-(kind, shape_name) template instanced via
		NodePath.instance_to() -- the same cheap-instancing idiom
		object_editor_mixins.materials' own specular overlay already relies
		on (_update_specular_overlay()), so the same shape placed thousands
		of times (a real continent's own vegetation/buildings) shares one
		copy of its Geom/Texture data. Mesh instances get real materials/
		textures (ig_geometry.build_textured_instance_template()); water
		instances keep their existing flat-cyan markers
		(build_water_polygon_geom()/build_water_point_geom(), project-todos/
		forgery/landscape_editor__ig_full_load.md's own scope decision -- no
		real water texture yet). Returns the new instance NodePath, or None
		if the shape has no renderable mesh at all (e.g. a FlareShape/
		ParticleSystemShape instance) -- no group node is created for a
		shape that never successfully attaches anything."""
		two_sided = False
		if resolved.kind == "mesh":
			key = ("mesh", resolved.shape_name)
			if key not in shape_templates:
				shape_templates[key] = ig_geometry.build_textured_instance_template(
					resolved.shape_value, self.search_paths_dialog.find_file, texture_cache,
				)
			template = shape_templates[key]
			if template is None:
				return None
			shape_np = shape_group_nodes.setdefault(resolved.shape_name, parent_np.attach_new_node(resolved.shape_name))
			instance_np = shape_np.attach_new_node("ig-instance")
			template.instance_to(instance_np)
		elif resolved.kind == "water_point":
			# A WaveMakerShape marker is always the same little cross,
			# genuinely shareable across every instance (unlike
			# water_polygon just below).
			key = ("water_point", None)
			if key not in shape_templates:
				shape_templates[key] = ig_geometry.build_water_point_geom()
			geom_node = shape_templates[key]
			shape_np = shape_group_nodes.setdefault(resolved.shape_name, parent_np.attach_new_node(resolved.shape_name))
			instance_np = shape_np.attach_new_node(geom_node)
			two_sided = True
		else:
			# water_polygon -- NEVER cached/shared: each instance has its
			# own real footprint (a lake/pond's own shape), unlike a mesh
			# .shape which always produces identical geometry. Found
			# 2026-09-13, Nuno: "tr_water je ne le vois pas" -- caching this
			# by (kind, None) meant the FIRST water_polygon instance built
			# in a whole run (any .ig, not just tr_water.ig) silently
			# decided every OTHER water_polygon's geometry too, including
			# returning None forever for all of them the moment one single
			# instance happened to have a degenerate (<3-point) footprint.
			geom_node = ig_geometry.build_water_polygon_geom(resolved.water_polygon)
			if geom_node is None:
				return None
			shape_np = shape_group_nodes.setdefault(resolved.shape_name, parent_np.attach_new_node(resolved.shape_name))
			instance_np = shape_np.attach_new_node(geom_node)
			two_sided = True

		instance_np.set_pos(*resolved.pos)
		instance_np.set_quat(Quat(*resolved.rot))
		instance_np.set_scale(*resolved.scale)
		if two_sided:
			# A water plane is a single flat polygon -- backface-culled by
			# default, it's only visible from one side, so panning the
			# camera under the water level (or a surface whose winding
			# happens to face away from the initial view) made it disappear
			# entirely (Nuno 2026-09-13: "il ne sont pas toujours
			# visibles"). Two-sided, like the terrain mesh itself
			# (_set_loaded_zones()), for the same reason.
			instance_np.set_two_sided(True)
			# Semi-transparent (Nuno 2026-09-13) -- Panda3D ignores
			# ig_geometry._WATER_COLOR's own alpha channel unless blending is
			# explicitly enabled on the NodePath.
			instance_np.set_transparency(TransparencyAttrib.M_alpha)
			# Real Tryker data has several water polygons overlapping/very
			# close together (adjoining lakes/lagoons) -- with depth WRITE
			# left on (Panda3D's own default even once transparency is
			# enabled), each one still fought the others for the depth
			# buffer, a very visible flicker (Nuno 2026-09-13: "le z-fighting
			# est vraiment tres tres moche"). Depth write off (test stays on,
			# so water still hides correctly behind terrain/buildings) is
			# the standard fix for overlapping transparent surfaces -- same
			# idiom object_editor_mixins.materials' own specular overlay
			# already relies on (_update_specular_overlay()).
			instance_np.set_depth_write(False)
			# Water/terrain z-fighting is a SEPARATE issue from the
			# water/water one above (Nuno 2026-09-14: "entre l'eau et le
			# terrain c'est encore moche") -- a lake's own polygon sits
			# right at (or extremely close to) the real terrain surface
			# below it, so depth-buffer precision alone can't reliably tell
			# them apart. `set_depth_offset()` (Panda3D's usual polygon-offset
			# tool for exactly this coplanar/decal case) was tried first and
			# confirmed to make NO visible difference even freshly rebuilt
			# (Nuno 2026-09-14) -- its glPolygonOffset-based bias is too
			# small relative to the real depth-buffer precision this far from
			# the camera to matter. A real, small world-space Z nudge (never
			# just a depth-buffer trick) is the reliable fix instead: lift
			# every water instance a little above wherever its own `.ig` data
			# places it, same idea as most games' own flat water planes
			# (deliberately floating a hair above the seabed/shoreline
			# terrain, not perfectly coincident with it).
			instance_np.set_z(instance_np.get_z() + _WATER_Z_LIFT)
		return instance_np

	def _build_ig_tree_entries(self, container_np):
		"""One entry per `ig-<name>` child of `container_np`, each holding
		every one of ITS OWN `.shape`-name group children (project-todos/
		forgery/landscape_editor__ig_inspector_tree.md step 2/3) -- fresh
		every time (never persisted): `{ig_name: {"checked": True, "shapes":
		{shape_name: True, ...}, "shape_nodes": {shape_name: NodePath, ...}}}`,
		everything starts checked/visible."""
		entries = {}
		for ig_np in container_np.get_children():
			name = ig_np.get_name()
			ig_name = name[3:] if name.startswith("ig-") else name
			shapes = {}
			shape_nodes = {}
			for shape_np in ig_np.get_children():
				shape_name = shape_np.get_name()
				shapes[shape_name] = True
				shape_nodes[shape_name] = shape_np
			entries[ig_name] = {"checked": True, "shapes": shapes, "shape_nodes": shape_nodes}
		return entries

	def _set_ig_checked(self, entry, checked):
		"""Cascades a `.ig` folder checkbox down to every one of its own
		`.shape` checkboxes/NodePaths (project-todos/forgery/
		landscape_editor__ig_inspector_tree.md step 3, Nuno: "Si je
		desactive l'affichage d'un .ig alors ça decoche tous les .shapes")."""
		entry["checked"] = checked
		for shape_name, shape_np in entry["shape_nodes"].items():
			entry["shapes"][shape_name] = checked
			if checked:
				shape_np.show()
			else:
				shape_np.hide()

	def _set_shape_checked(self, entry, shape_name, checked):
		"""Toggles a single `.shape` checkbox/NodePath -- checking ONE shape
		re-checks its `.ig` folder without touching any other shape's own
		state (Nuno: "Si je recoche un .shape alors ça réactive le .ig");
		unchecking the last remaining checked shape reads the `.ig` folder
		itself back to unchecked, for visual consistency."""
		entry["shapes"][shape_name] = checked
		shape_np = entry["shape_nodes"][shape_name]
		if checked:
			shape_np.show()
			entry["checked"] = True
		else:
			shape_np.hide()
			if not any(entry["shapes"].values()):
				entry["checked"] = False

	def _show_ig_region(self, region_name, node_path):
		"""Reparents `node_path` (region_name's own finished/cached bundle)
		under self._ig_region_root, replacing whatever was there before for
		this region only -- other regions' own bundles, and the "rest"
		bundle, are untouched. Rebuilds this region's own checkbox-tree
		state from the fresh bundle (everything starts checked)."""
		existing = self._ig_region_nodes.pop(region_name, None)
		if existing is not None:
			existing.remove_node()
		node_path.reparent_to(self._ig_region_root)
		self._ig_region_nodes[region_name] = node_path
		self._ig_tree_state_zones[region_name] = self._build_ig_tree_entries(node_path)

	def _hide_ig_region(self, region_name):
		"""Detaches `region_name`'s own `.ig` bundle, if shown -- called from
		_toggle_region() right after that region gets UNCHECKED (mirrors
		zone geometry's own per-region teardown)."""
		node = self._ig_region_nodes.pop(region_name, None)
		if node is not None:
			node.remove_node()
		self._ig_region_progress.pop(region_name, None)
		self._ig_tree_state_zones.pop(region_name, None)

	def _show_ig_rest(self, node_path):
		"""Reparents `node_path` (the continent-wide "rest" bundle) under
		self._ig_rest_root, replacing whatever "rest" bundle was there
		before -- region bundles are untouched. Rebuilds the "rest"
		checkbox-tree state from the fresh bundle (everything starts
		checked)."""
		if self._ig_rest_np is not None:
			self._ig_rest_np.remove_node()
		node_path.reparent_to(self._ig_rest_root)
		self._ig_rest_np = node_path
		self._ig_tree_state_others = self._build_ig_tree_entries(node_path)

	def _hide_all_ig(self):
		"""Detaches every `.ig` bundle currently shown (every region's own,
		plus the "rest" one), and forgets every in-flight build -- called
		whenever the selected continent or mode changes (_select_continent()),
		so a previous continent's buildings/props never linger in the view."""
		for node in self._ig_region_nodes.values():
			node.remove_node()
		self._ig_region_nodes = {}
		self._ig_region_progress = {}
		self._ig_tree_state_zones = {}
		if self._ig_rest_np is not None:
			self._ig_rest_np.remove_node()
			self._ig_rest_np = None
		self._ig_rest_progress = None
		self._ig_tree_state_others = {}

	def _toggle_ig_visibility(self):
		"""Viewport toolbar's tree icon -- toggles visibility of
		self._ig_region_root ONLY (project-todos/forgery/
		landscape_editor__ig_full_load.md's 2026-09-13 revision, Nuno: "il
		faudrait juste séparer l'affichage des 2 [...] 2 .bam différents, 2
		boutons différents" -- every checked region's own zone-`.ig` bundle,
		never the continent-wide "rest" one (see _toggle_ig_rest_visibility()
		for that, a separate icon). Never itself triggers any build/load: a
		region's own `.ig` loads automatically when that region gets checked
		(_toggle_region() -> _load_region_ig())."""
		self._ig_region_visible = not self._ig_region_visible
		if self._ig_region_visible:
			self._ig_region_root.show()
		else:
			self._ig_region_root.hide()

	def _toggle_ig_rest_visibility(self):
		"""Second viewport toolbar icon, right next to the tree one --
		toggles visibility of self._ig_rest_root ONLY (the continent-wide
		"rest" bundle: villages/water/..., never zone-owned buildings/props,
		see _toggle_ig_visibility()'s own docstring for why these are two
		independent toggles). Never itself triggers a build: that's
		_load_ig_rest(), called automatically whenever a continent loads."""
		self._ig_rest_visible = not self._ig_rest_visible
		if self._ig_rest_visible:
			self._ig_rest_root.show()
		else:
			self._ig_rest_root.hide()

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

	def _rebuild_zone_node(self, name, cache_data, manifest_entry=None):
		"""Replaces the single geometry node for zone `name` with one built
		from `cache_data`, keeping the stage-3 gradient consistent with every
		other currently loaded fully-built zone (step 9's live update -- see
		_set_loaded_zones()'s own docstring for why that gradient spans the
		whole loaded set, not each zone's own range). Does not touch the
		grid overlay or camera framing -- the loaded set's bounds don't
		change when a zone advances a build stage.

		`manifest_entry` (project-todos/forgery/landscape_editor__render_
		modes_removal.md perf fix, Nuno 2026-09-14), when given, is checked
		against the per-zone `.bam` cache first (same comparison
		_set_loaded_zones() does) -- this is what actually makes
		_run_load_refs()'s "skip the expensive position load, `cache_data.
		patches` stays empty" optimization safe to call THIS with: unlike
		before, this method now fetches the real cached `.bam` NodePath
		instead of trying (and silently failing, "rien ne s'affiche",
		Nuno 2026-09-14) to tessellate geometry from empty patches. If
		`manifest_entry` is omitted, or given but the `.bam` cache doesn't
		actually hit (stale), and `cache_data.patches` is empty, the real
		position data is loaded on demand as a last resort, so this never
		silently produces a blank zone."""
		old_node = self._zone_nodes.get(name)
		if old_node is not None:
			old_node.remove_node()
		if not self.zones:
			return
		min_z, max_z = self._elevation_z_range(self.zones)
		stage = zone_build_stage(self._loaded_extensions.get(name, {}))
		node_path = None
		if manifest_entry is not None:
			wanted_manifest = ZoneGeomManifest(
				format_version=2, entry=manifest_entry, min_z=min_z, max_z=max_z,
				bb_center=cache_data.bb_center, bb_half_size=cache_data.bb_half_size,
			)
			cached = zone_geom_cache.read_zone_bundle(name, self._zone_cache_mode(), self.loader)
			if cached is not None:
				cached_node_path, cached_manifest = cached
				if cached_manifest == wanted_manifest:
					cached_node_path.set_name(name)
					cached_node_path.reparent_to(self._zone_root)
					node_path = cached_node_path
				else:
					cached_node_path.remove_node()
		if node_path is None:
			if not cache_data.patches:
				ref = self._loaded_refs.get(name)
				if ref is None:
					return
				try:
					cache_data = load_zone_cache_data(ref, low_poly=self._low_poly)
				except (RegionLoadError, OSError):
					return
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


	def _set_loaded_zones(self, zones, manifest_zones=None, mode=None, grid_bounds=None, frame_camera=True, force=False):
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
					format_version=2, entry=manifest_zones[name], min_z=min_z, max_z=max_z,
					bb_center=cache_data.bb_center, bb_half_size=cache_data.bb_half_size,
				)
				# `force` (project-todos/forgery/landscape_editor__render_
				# modes_removal.md "reload cache" buttons) skips the read
				# entirely -- treated as a cache miss below, so the fresh
				# rebuild's write_zone_bundle() call further down still
				# overwrites the on-disk `.bam`/manifest with today's data,
				# even when the old one was already up to date.
				cached = None if force else zone_geom_cache.read_zone_bundle(name, mode, self.loader)
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
		# A previous continent/mode's `.ig` bundle (project-todos/forgery/
		# landscape_editor__ig_full_load.md step 6) must never linger once
		# the selection changes -- also covers a mode switch, which
		# re-triggers this same method for the current continent (see
		# draw_panel()'s own _pending_continent_reload comment).
		self._hide_all_ig()
		self._ig_error = None
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
		# frame_bounds() as the left panel's own "center camera" icon --
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
		independent of the zone geometry itself. Leaving 3D for 2D remembers
		the orientation being left, so coming back to 3D restores it instead
		of leaving the view stuck on the top-down look."""
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
		covers every zone currently loaded. Independent of
		_cycle_zone_wireframe() -- both can be on at once."""
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
			if _icon_button(fa_icons.ICON_FA_TREE,
			                "Show zone .ig instances (buildings/props, per region) (right-click to regenerate cache)",
			                self._ig_region_visible, square=True, large_font=large_font):
				self._toggle_ig_visibility()
			if imgui.begin_popup_context_item("##ig-zone-cache-popup"):
				clicked, _ = imgui.selectable("Regenerate cache", False)
				if clicked:
					self._regenerate_region_ig_caches()
				imgui.end_popup()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_HOUSE_CHIMNEY,
			                "Show remaining .ig instances (villages/water) (right-click to regenerate cache)",
			                self._ig_rest_visible, square=True, large_font=large_font):
				self._toggle_ig_rest_visibility()
			if imgui.begin_popup_context_item("##ig-rest-cache-popup"):
				clicked, _ = imgui.selectable("Regenerate cache", False)
				if clicked:
					self._load_ig_rest(force=True)
				imgui.end_popup()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_GAUGE_SIMPLE, "Low poly terrain (for weak GPUs)",
			                self._low_poly, square=True, large_font=large_font):
				self._toggle_low_poly()
			self._viewport_toggle_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _compass_north_angle(self):
		"""Clockwise angle (radians, 0 = straight up on screen) world north
		(+Y, see camera.py's own AXIS_VIEWS docstring: heading=0 looks from
		-Y towards +Y) currently points at -- read straight from the real
		camera transform (self.camera.get_mat(self.render), Panda3D's own
		CS_zup_right row layout: row 0 = local +X/right, row 2 = local
		+Z/up, both already in world space for an unscaled node), not
		re-derived from self.orbit_camera.heading/pitch by hand -- correct
		regardless of pitch/roll, unlike a heading-only approximation."""
		mat = self.camera.get_mat(self.render)
		screen_right = mat.get_row3(0)
		screen_up = mat.get_row3(2)
		north = Vec3(0, 1, 0)
		return math.atan2(north.dot(screen_right), north.dot(screen_up))

	def _draw_north_compass(self):
		"""Small floating compass bottom-right of the 3D viewport (project-
		todos/forgery/landscape_editor__render_modes_removal.md) -- same
		floating-window mechanism as _draw_viewport_toggles() (self-measured
		size, positioned off last frame's own captured size), just anchored
		to the right panel's own width instead of the left Explorer's. A
		hand-drawn arrow (ImGui has no built-in glyph/text rotation) always
		points at real world north, computed fresh every frame
		(_compass_north_angle()) -- Nuno 2026-09-14: a static, non-rotating
		icon here before wasn't actually indicating a direction at all. A
		click resets the camera's heading to due north (_reset_camera_
		heading()), leaving pitch/distance/target and the 2D/3D mode
		untouched."""
		display_size = imgui.get_io().display_size
		win_w, win_h = display_size.x, display_size.y
		if win_h <= 0:
			return

		radius = 22.0
		size = radius * 2.0
		x = win_w - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - size
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - size
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.no_background.value | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("##north-compass", flags=flags):
			imgui.invisible_button("##compass-reset", (size, size))
			clicked = imgui.is_item_clicked()
			if imgui.is_item_hovered():
				imgui.set_tooltip("Reset camera heading to north")
			pos = imgui.get_item_rect_min()
			center = (pos.x + radius, pos.y + radius)
			draw_list = imgui.get_window_draw_list()
			draw_list.add_circle_filled(center, radius, imgui.get_color_u32((0.12, 0.12, 0.15, 0.85)), num_segments=32)
			draw_list.add_circle(center, radius, imgui.get_color_u32((0.6, 0.6, 0.65, 1.0)), num_segments=32, thickness=1.5)
			angle = self._compass_north_angle()
			tip = (center[0] + radius * 0.8 * math.sin(angle), center[1] - radius * 0.8 * math.cos(angle))
			tail = (center[0] - radius * 0.4 * math.sin(angle), center[1] + radius * 0.4 * math.cos(angle))
			arrow_color = imgui.get_color_u32((0.9, 0.25, 0.25, 1.0))
			draw_list.add_line(tail, tip, arrow_color, thickness=2.5)
			head_left = (tip[0] + 7.0 * math.sin(angle + 2.5), tip[1] - 7.0 * math.cos(angle + 2.5))
			head_right = (tip[0] + 7.0 * math.sin(angle - 2.5), tip[1] - 7.0 * math.cos(angle - 2.5))
			draw_list.add_triangle_filled(tip, head_left, head_right, arrow_color)
			self._compass_size = (imgui.get_window_size().x, imgui.get_window_size().y)
			if clicked:
				self._reset_camera_heading()

	def _reset_camera_heading(self):
		"""Resets the camera's heading to due north (0.0), keeping pitch/
		distance/target untouched -- setting self.orbit_camera.heading
		alone would NOT move the real camera at all (camera.py's own
		OrbitCamera._update() only ever recomputes the actual position on a
		mouse drag or a running animation, see _update_camera_pos()'s own
		callers): animate_to_orientation() is the one already-public entry
		point that both updates self.heading correctly AND drives the
		camera to match, every frame, via its own animation (same mechanism
		as _toggle_top_down()'s "restore the saved 3D view" call, Nuno
		2026-09-14: "le reset ne fonctionne pas" -- found tracing camera.py,
		a plain assignment silently did nothing visible)."""
		self.orbit_camera.animate_to_orientation(0.0, self.orbit_camera.pitch)

	def _zone_cache_mode(self):
		"""The `mode` key used everywhere a zone's per-zone `.bam` geometry
		cache (`zone_geom_cache.py`) is read or written -- project-todos/
		forgery/landscape_editor__render_modes_removal.md already made this
		a stable constant ("zone") once the old per-render-mode variants
		(POLY/WELD/LIGHT) were removed; project-todos/forgery/
		landscape_editor__low_poly_mode.md revives exactly one variant axis
		on it -- quality -- so a low-poly zone's `.bam` never collides with
		its normal-quality one, same cache module, no new cache."""
		return "zone_low" if self._low_poly else "zone"

	def _toggle_low_poly(self):
		"""Flips the "Low Poly" terrain toggle (project-todos/forgery/
		landscape_editor__low_poly_mode.md, Nuno 2026-09-14, for weak GPUs),
		persists it, then forces a full reload of everything currently
		loaded (_apply_render_mode(), unlike a single region check/uncheck:
		a quality change legitimately affects every zone on screen at once,
		not just one region -- the same reasoning _apply_render_mode()'s
		own full-union reload existed for before the per-region incremental
		path replaced it for region toggles specifically)."""
		self._low_poly = not self._low_poly
		settings = app_settings.load()
		settings.landscape_low_poly = self._low_poly
		app_settings.save(settings)
		self._apply_render_mode()

	def panel_title(self):
		return "Landscape Editor"

	def _draw_continent_and_regions(self):
		"""Continent combo + region checkboxes (project-todos/forgery/
		landscape_editor__render_modes_removal.md, moved here from the right
		panel) -- also owns every background-progress polling that feeds
		this state (continent scan, zone-geometry reload, region/rest `.ig`
		builds, region cache reloads) so it keeps working while the
		Settings tab is active on the right (the right panel's own
		`_draw_landscape_tab()` is only ever drawn for its "Landscape" tab,
		this method isn't)."""
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
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_CROSSHAIRS, "Center camera on continent", square=True,
		                disabled=self.continent_bounds is None):
			min_x, min_y, max_x, max_y = self.continent_bounds
			self.orbit_camera.frame_bounds(min_x, min_y, max_x, max_y, margin=ZONE_CELL_SIZE)
		imgui.same_line()
		continent_reloading = bool(self._continent_cache_reload_queue) or (
			self._region_cache_reload_progress is not None and not self._region_cache_reload_progress["done"]
		)
		if _icon_button(fa_icons.ICON_FA_ROTATE, "Reload cache for the whole continent, region by region",
		                square=True, disabled=self.selected_continent_name is None or continent_reloading):
			self._reload_continent_cache()

		if self._bounds_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._bounds_error)

		if self._region_names:
			imgui.separator()
			for region_name in self._region_names:
				imgui.push_id(region_name)
				checked = self._region_checked.get(region_name, False)
				label = region_name[len("region_"):] if region_name.startswith("region_") else region_name
				changed, new_value = imgui.checkbox(label, checked)
				if changed and new_value != checked:
					self._toggle_region(region_name)
				imgui.same_line()
				if _icon_button(fa_icons.ICON_FA_CROSSHAIRS, f"Center camera on {label}", square=True,
				                disabled=region_name not in self._region_bounds):
					min_x, min_y, max_x, max_y = self._region_bounds[region_name]
					self.orbit_camera.frame_bounds(min_x, min_y, max_x, max_y, margin=ZONE_CELL_SIZE)
				imgui.same_line()
				region_reloading = (
					self._region_cache_reload_progress is not None
					and not self._region_cache_reload_progress["done"]
					and self._region_cache_reload_progress["region"] == region_name
				)
				if _icon_button(fa_icons.ICON_FA_ROTATE, f"Reload cache for {label}", square=True,
				                disabled=not checked or region_reloading):
					self._reload_region_cache(region_name)
				imgui.pop_id()

		if self._continent_load_progress is not None:
			progress = self._continent_load_progress
			if progress["done"]:
				self._continent_load_progress = None
				print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_draw_continent_and_regions) "
				      f"continent scan done, error={progress['error']!r} zones={len(progress['default_refs'])}")
				if progress["error"]:
					self._zone_error = progress["error"]
				else:
					self._all_continent_refs = progress["default_refs"]
					self._all_continent_extensions = progress["extensions"]
					self._land_cell_region_map = progress.get("land_cell_region_map", {})
					self._continent_z_range = progress.get("continent_z_range")
					self._continent_total_used_cells = progress.get("total_used_cells")
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
						self._region_bounds = progress.get("region_bounds", {})
						self._region_zone_map = progress.get("zone_region_map", {})
						self._region_checked = {name: False for name in regions}
						if len(regions) == 1:
							self._region_checked[regions[0]] = True
							# _toggle_region() itself isn't called for this
							# auto-check (it also flips a checkbox that's
							# already False here), so its own _load_region_ig()
							# call must be repeated here.
							self._load_region_ig(regions[0])
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
						self._region_bounds = {}
						self._region_zone_map = {}
						self._region_checked = {}
						self._loaded_refs = self._all_continent_refs
						self._loaded_extensions = self._all_continent_extensions
					self._apply_render_mode()
					self._rebuild_region_placeholders()
					# The "rest" bundle (villages/water/...) isn't tied to
					# any region, so it always loads here regardless of the
					# continent's own region_hierarchy shape -- from cache
					# if the continent was already built before (near
					# instant), built fresh otherwise (Nuno 2026-09-13: "il
					# faut charger ces .ig au chargement du continent").
					self._load_ig_rest()
			else:
				imgui.text("Scanning zone index...")

		if self._mode_reload_progress is not None:
			progress = self._mode_reload_progress
			if progress["done"]:
				self._mode_reload_progress = None
				print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor.py:_draw_continent_and_regions) "
				      f"mode reload done, error={progress['error']!r} zones={len(progress['zones'])}")
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
					progress["zones"], manifest_zones=progress["manifest_zones"],
					mode=self._zone_cache_mode(), grid_bounds=self._continent_loaded_grid_bounds, frame_camera=False,
					force=progress["force"],
				)
			else:
				total = progress["total"]
				processed = progress["processed"]
				fraction = processed / total if total else 0.0
				overlay = f"{processed}/{total} zones" if total else "Loading..."
				imgui.progress_bar(fraction, overlay=overlay)

		if self._region_cache_reload_progress is not None:
			progress = self._region_cache_reload_progress
			if progress["done"]:
				self._region_cache_reload_progress = None
				if progress["error"]:
					self._zone_error = progress["error"]
				else:
					# NOT `... if self.zones else (0.0, 1.0)` (found 2026-09-14,
					# Nuno: "0 skipped via .bam manifest" even on an exact
					# repeat load) -- _elevation_z_range() ALREADY prefers
					# self._continent_z_range over `zones` on its own; that
					# extra guard bypassed it whenever self.zones was still
					# empty (e.g. the very first region loaded for a fresh
					# continent selection), silently writing every manifest
					# with the placeholder (0.0, 1.0) instead of the real
					# whole-continent range -- permanently mismatching every
					# later _run_load_refs() comparison against the real
					# self._continent_z_range, so the skip-path never fired.
					min_z, max_z = self._elevation_z_range(self.zones)
					region_checked = self._region_checked.get(progress["region"], False)
					for name, cache_data in progress["zones"].items():
						manifest_entry = progress["manifest_zones"].get(name)
						if region_checked:
							# Live in-place patch (same primitive as the Build
							# progress drain) -- this region's own display
							# stays untouched apart from these zones. NOT
							# passing manifest_entry here (unlike the
							# incremental check/uncheck consumer below) --
							# this whole reload IS the "force, always
							# rebuild" path (project-todos/forgery/
							# landscape_editor__render_modes_removal.md,
							# Nuno 2026-09-14: "invalider les 2 caches"), so
							# _rebuild_zone_node() must never take its own
							# `.bam`-cache shortcut here either.
							self.zones[name] = cache_data
							self._rebuild_zone_node(name, cache_data)
						if manifest_entry is not None:
							node_path = self._zone_nodes.get(name)
							if node_path is not None:
								manifest = ZoneGeomManifest(
									format_version=2, entry=manifest_entry, min_z=min_z, max_z=max_z,
									bb_center=cache_data.bb_center, bb_half_size=cache_data.bb_half_size,
								)
								zone_geom_cache.write_zone_bundle(name, self._zone_cache_mode(), node_path, manifest)
				# Continent-wide reload (project-todos/forgery/
				# landscape_editor__render_modes_removal.md) -- advance to
				# the next queued region, if any.
				self._advance_continent_cache_reload()
			else:
				total = progress["total"]
				processed = progress["processed"]
				fraction = processed / total if total else 0.0
				overlay = f"{progress['region']}: {processed}/{total} zone(s)"
				imgui.progress_bar(fraction, overlay=overlay)

		if self._region_incremental_progress is not None:
			progress = self._region_incremental_progress
			if progress["done"]:
				self._region_incremental_progress = None
				if progress["error"]:
					self._zone_error = progress["error"]
				else:
					# Only actually shown if the region is STILL checked --
					# a rapid check-then-uncheck of the SAME region while
					# this background load was in flight already tore its
					# zones down synchronously (_unload_region_incremental()),
					# this stale result must not bring them back.
					if self._region_checked.get(progress["region"], False):
						# NOT `... if self.zones else (0.0, 1.0)` (found 2026-09-14,
						# Nuno: "0 skipped via .bam manifest" even on an exact
						# repeat load) -- _elevation_z_range() ALREADY prefers
						# self._continent_z_range over `zones` on its own; that
						# extra guard bypassed it whenever self.zones was still
						# empty (e.g. the very first region loaded for a fresh
						# continent selection), silently writing every manifest
						# with the placeholder (0.0, 1.0) instead of the real
						# whole-continent range -- permanently mismatching every
						# later _run_load_refs() comparison against the real
						# self._continent_z_range, so the skip-path never fired.
						min_z, max_z = self._elevation_z_range(self.zones)
						for name, cache_data in progress["zones"].items():
							manifest_entry = progress["manifest_zones"].get(name)
							self.zones[name] = cache_data
							self._rebuild_zone_node(name, cache_data, manifest_entry=manifest_entry)
							if manifest_entry is not None:
								node_path = self._zone_nodes.get(name)
								if node_path is not None:
									manifest = ZoneGeomManifest(
										format_version=2, entry=manifest_entry, min_z=min_z, max_z=max_z,
										bb_center=cache_data.bb_center, bb_half_size=cache_data.bb_half_size,
									)
									zone_geom_cache.write_zone_bundle(name, self._zone_cache_mode(), node_path, manifest)
				# Drain the next region queued while this one was loading
				# (Nuno checked several in a row before this one finished).
				if self._region_incremental_queue:
					next_region = self._region_incremental_queue.pop(0)
					if self._region_checked.get(next_region, False):
						self._load_region_incremental(next_region)
			else:
				total = progress["total"]
				processed = progress["processed"]
				fraction = processed / total if total else 0.0
				overlay = f"{progress['region']}: {processed}/{total} zone(s)"
				imgui.progress_bar(fraction, overlay=overlay)

		if self._ig_rest_progress is not None and self._ig_rest_progress["done"]:
			progress = self._ig_rest_progress
			self._ig_rest_progress = None
			# Discarded (geometry never attached) if the continent/mode was
			# switched away from while this build was still running --
			# _hide_all_ig() already cleared self._ig_root for the new
			# selection, this stale bundle must not repopulate it.
			still_current = (progress["continent"], progress["mode"]) == (
				self._selected_continent_pipeline_name, self._app_mode,
			)
			if progress["error"]:
				self._ig_error = progress["error"]
			elif progress["node_path"] is not None and still_current:
				self._ig_error = None
				self._show_ig_rest(progress["node_path"])

		for region_name in list(self._ig_region_progress.keys()):
			progress = self._ig_region_progress[region_name]
			if not progress["done"]:
				continue
			del self._ig_region_progress[region_name]
			# Same staleness guard as the "rest" bundle above, plus: the
			# region must still be CHECKED (unchecking it while its build
			# was running already called _hide_ig_region(), which popped
			# this same progress entry -- so this loop only ever sees a
			# region that's still both current and checked).
			still_current = (progress["continent"], progress["mode"]) == (
				self._selected_continent_pipeline_name, self._app_mode,
			)
			if progress["error"]:
				self._ig_error = progress["error"]
			elif progress["node_path"] is not None and still_current:
				self._ig_error = None
				self._show_ig_region(region_name, progress["node_path"])

	def draw_left_panel_content(self):
		"""Overrides ForgeryApp.draw_left_panel_content() (project-todos/
		forgery/landscape_editor__ig_inspector_tree.md, Nuno 2026-09-14:
		"vire l'explorer de gauche qui ne sert strictement à rien") -- the
		real-file Explorer is replaced entirely here by continent/region
		selection (project-todos/forgery/landscape_editor__render_modes_
		removal.md, moved from the right panel the same day) followed by a
		checkbox tree of what's actually loaded: IG Zones
		(self._ig_tree_state_zones, grouped by region) and IG Others
		(self._ig_tree_state_others, flat, the "rest" bundle). Losing the
		Explorer also loses its one other use in this app (selecting a
		single `.zone`/`.zonew`/`.zonel` file directly) -- Nuno confirmed not
		using that."""
		self._ensure_continent_locations_loaded()
		self._draw_continent_and_regions()
		imgui.separator()
		if imgui.tree_node_ex("##ig-zones-root", imgui.TreeNodeFlags_.default_open.value, "IG Zones"):
			for region_name in sorted(self._ig_tree_state_zones.keys()):
				imgui.push_id(region_name)
				if imgui.tree_node_ex("##region-root", imgui.TreeNodeFlags_.open_on_arrow.value, region_name):
					self._draw_ig_tree_entries(self._ig_tree_state_zones[region_name])
					imgui.tree_pop()
				imgui.pop_id()
			imgui.tree_pop()
		if imgui.tree_node_ex("##ig-others-root", imgui.TreeNodeFlags_.default_open.value, "IG Others"):
			self._draw_ig_tree_entries(self._ig_tree_state_others)
			imgui.tree_pop()

	def _draw_ig_tree_entries(self, entries):
		"""Draws one checkbox + expandable folder per `.ig` in `entries`
		(_build_ig_tree_entries()), each expanding to one checkbox per
		`.shape` it uses -- shared by both IG Zones (per-region) and IG
		Others (draw_left_panel_content())."""
		for ig_name in sorted(entries.keys()):
			entry = entries[ig_name]
			imgui.push_id(ig_name)
			changed, new_value = imgui.checkbox("##ig-checked", entry["checked"])
			if changed:
				self._set_ig_checked(entry, new_value)
			imgui.same_line()
			if imgui.tree_node_ex("##ig-root", imgui.TreeNodeFlags_.open_on_arrow.value, f"{ig_name}.ig"):
				for shape_name in sorted(entry["shapes"].keys()):
					shape_changed, shape_value = imgui.checkbox(shape_name, entry["shapes"][shape_name])
					if shape_changed:
						self._set_shape_checked(entry, shape_name, shape_value)
				imgui.tree_pop()
			imgui.pop_id()

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
		self._draw_north_compass()
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
				self._region_bounds = {}
				self._region_zone_map = {}
				self._region_checked = {}
				self._land_cell_region_map = {}
				self._continent_z_range = None
				self._continent_total_used_cells = None
				self._continent_loaded_grid_bounds = None
				self._rebuild_region_placeholders()
				self._set_loaded_zones({})

		if self._zone_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._zone_error)
		if self._ig_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._ig_error)

		self._draw_selected_zone_section()
		self._draw_continent_status_section()
		self._draw_continent_stats_section()

	def _draw_selected_zone_section(self):
		"""Repliable "Selected Zone" section (project-todos/forgery/
		landscape_editor__render_modes_removal.md) -- regroups the name+
		bounds text and the `.land` composition editor (`_draw_land_
		composition_editor()`, EditModeMixin) that used to sit as two
		separate always-visible blocks; disabled/grayed out entirely when
		nothing is selected. Colored icon header, zone name in orange,
		bounds in green (Nuno 2026-09-14: "comme Patina" -- same
		pastel_color_for(icon) tinting Patina/Explorer already use for
		icon-buttons app-wide, icon_colors.py)."""
		has_selection = self._selected_zone_name is not None and self._selected_zone_bounds is not None
		imgui.begin_disabled(not has_selection)
		imgui.push_style_color(imgui.Col_.text.value, pastel_color_for(fa_icons.ICON_FA_LOCATION_CROSSHAIRS))
		expanded = imgui.collapsing_header(
			f"{fa_icons.ICON_FA_LOCATION_CROSSHAIRS} Selected Zone", flags=imgui.TreeNodeFlags_.default_open.value,
		)
		imgui.pop_style_color()
		imgui.end_disabled()
		if not has_selection or not expanded:
			return
		min_x, min_y, max_x, max_y = self._selected_zone_bounds
		imgui.text("Selected zone:")
		imgui.same_line()
		imgui.text_colored(_NAME_COLOR, self._selected_zone_name)
		imgui.text("  Min:")
		imgui.same_line()
		imgui.text_colored(_VALUE_COLOR, f"({min_x:.1f}, {min_y:.1f})")
		imgui.text("  Max:")
		imgui.same_line()
		imgui.text_colored(_VALUE_COLOR, f"({max_x:.1f}, {max_y:.1f})")
		self._draw_land_composition_editor()

	def _draw_continent_status_section(self):
		"""Always-visible "Continent Status" section (Édition only,
		project-todos/forgery/landscape_editor__render_modes_removal.md) --
		how many of the continent's `.land` cells have reached each real
		pipeline stage on disk, straight from self._loaded_extensions (same
		real-disk-state source as zone_geometry.zone_build_stage()'s own
		coloring, not derived from a per-mode resolution). The [Build]
		button (`_draw_land_build_button()`, EditModeMixin) now only shows
		up here, and only while something's actually missing or a build is
		already running -- replaces the old unconditional bottom-of-panel
		button. Colored icon header, counts in green (Nuno 2026-09-14:
		"comme Patina")."""
		if self._app_mode != _MODE_EDITION or not self.selected_continent_name:
			return
		total = self._continent_total_used_cells if self._continent_total_used_cells is not None else len(self.zones)
		raw = sum(1 for ext_map in self._loaded_extensions.values() if ".zone" in ext_map)
		welded = sum(1 for ext_map in self._loaded_extensions.values() if ".zonew" in ext_map)
		lighted = sum(1 for ext_map in self._loaded_extensions.values() if ".zonel" in ext_map)
		imgui.separator()
		imgui.push_style_color(imgui.Col_.text.value, pastel_color_for(fa_icons.ICON_FA_CIRCLE_INFO))
		imgui.text(f"{fa_icons.ICON_FA_CIRCLE_INFO} Continent status")
		imgui.pop_style_color()
		for label, value in (
			("Zones:", total), ("Raw (.zone):", raw), ("Welded (.zonew):", welded), ("Lighted (.zonel):", lighted),
		):
			imgui.text(label)
			imgui.same_line()
			imgui.text_colored(_VALUE_COLOR, str(value))
		building = self._land_build_progress is not None and not self._land_build_progress["done"]
		if lighted < total or building:
			self._draw_land_build_button()

	def _draw_continent_stats_section(self):
		"""Repliable "Stats" section (project-todos/forgery/landscape_editor__
		render_modes_removal.md) -- real counts, computed on demand from
		already-loaded state (self._region_zone_map/self.zones for zones+
		patches, self._ig_tree_state_zones/_others' own `shape_nodes` --
		each shape group's NodePath children ARE its individual instances,
		regardless of whether their own checkbox is currently checked/
		visible -- for instance counts). Folded by default. A real ImGui
		table (Nuno 2026-09-14: "illisible... il faut de la couleur, des
		sauts de lignes, un mode tableau") -- striped row backgrounds,
		right-aligned numeric columns, borders; an unchecked/never-loaded
		region legitimately shows 0 everywhere (its zones/`.ig` were never
		actually loaded, nothing to count yet)."""
		if not self.zones:
			return
		if not imgui.collapsing_header("Stats"):
			return
		imgui.text_colored(_STATS_HEADER_COLOR, f"Regions: {len(self._region_names)}")
		table_flags = (
			imgui.TableFlags_.borders.value | imgui.TableFlags_.row_bg.value
			| imgui.TableFlags_.sizing_stretch_prop.value | imgui.TableFlags_.resizable.value
		)
		total_region_instances = 0
		if imgui.begin_table("##stats-regions", 4, table_flags):
			imgui.table_setup_column("Region")
			imgui.table_setup_column("Zones", flags=imgui.TableColumnFlags_.width_fixed.value)
			imgui.table_setup_column("Patches", flags=imgui.TableColumnFlags_.width_fixed.value)
			imgui.table_setup_column("IG instances", flags=imgui.TableColumnFlags_.width_fixed.value)
			imgui.table_headers_row()
			for region_name in self._region_names:
				zone_names = [name for name, r in self._region_zone_map.items() if r == region_name]
				patches = sum(len(self.zones[name].patches) for name in zone_names if name in self.zones)
				instances = sum(
					len(shape_np.get_children())
					for entry in self._ig_tree_state_zones.get(region_name, {}).values()
					for shape_np in entry["shape_nodes"].values()
				)
				total_region_instances += instances
				label = region_name[len("region_"):] if region_name.startswith("region_") else region_name
				imgui.table_next_row()
				imgui.table_next_column()
				imgui.text(label)
				imgui.table_next_column()
				imgui.text_colored(_STATS_VALUE_COLOR, str(len(zone_names)))
				imgui.table_next_column()
				imgui.text_colored(_STATS_VALUE_COLOR, str(patches))
				imgui.table_next_column()
				imgui.text_colored(_STATS_VALUE_COLOR, str(instances))
			imgui.end_table()

		others_igs = len(self._ig_tree_state_others)
		others_instances = sum(
			len(shape_np.get_children())
			for entry in self._ig_tree_state_others.values() for shape_np in entry["shape_nodes"].values()
		)
		imgui.text_colored(_STATS_VALUE_COLOR, f"IG Others: {others_igs} file(s), {others_instances} instance(s)")
		total_patches = sum(len(cache_data.patches) for cache_data in self.zones.values())
		imgui.text_colored(
			_STATS_HEADER_COLOR,
			f"Total: {len(self.zones)} zone(s), {total_patches} patch(es), "
			f"{total_region_instances + others_instances} ig instance(s)",
		)


def main(argv=None):
	LandscapeEditorApp().run()


if __name__ == "__main__":
	main()
