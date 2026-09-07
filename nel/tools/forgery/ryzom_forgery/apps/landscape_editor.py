"""Ryzom Forgery landscape editor, branded "Atyscape" (Nuno, 2026-09-07 --
same naming convention as object_editor.py/"Patina"): 3D landscape
composition/build tool, replacing the current 2D bitmap-tile Ligo editor.

See project-todos/forgery/landscape_editor.md for the planned progressive
rendering steps -- step 3 (low-poly zone rendering) is the first one that
actually shows any terrain.
"""

from pathlib import Path

from imgui_bundle import imgui

from pynel.ryzom_bnp import BnpError
from pynel.ryzom_zone import parse_zone, ZoneParseError

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.camera import OrbitCamera
from ryzom_forgery import continent_selector
from ryzom_forgery import live_data
from ryzom_forgery import settings as app_settings
from ryzom_forgery.zone_geometry import build_zone_low_poly_geom

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

APP_INFO = {
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
		self.continent_bounds = None
		self._bounds_error = None

		# Loaded-zone state (project-todos/forgery/landscape_editor.md step 3
		# -- low-poly rendering, no Bezier tessellation yet, see step 4).
		# zone_node is the currently attached GeomNode NodePath (torn down and
		# rebuilt on every new zone load, None before the first one), zone is
		# the parsed pynel.ryzom_zone.Zone it was built from.
		self.zone = None
		self.zone_node = None
		self._zone_error = None
		self._zone_root = self.render.attach_new_node("zone-root")

	def on_selection_changed(self, items):
		if len(items) != 1 or items[0].suffix.lower() not in _ZONE_EXTENSIONS:
			return
		item = items[0]
		self._zone_error = None
		try:
			self.zone = parse_zone(item.read_bytes())
		except (OSError, BnpError, ZoneParseError) as exc:
			self.zone = None
			self._zone_error = f"Failed to load {item.name}: {exc}"
			self._clear_zone_geometry()
			return

		self._clear_zone_geometry()
		node = build_zone_low_poly_geom(self.zone)
		self.zone_node = self._zone_root.attach_new_node(node)
		# Winding of the low-poly quads (zone_geometry.py) follows
		# CBezierPatch's own corner convention, not necessarily Panda3D's
		# expected front-face direction -- two-sided so every patch is
		# visible regardless, rather than risking half the terrain silently
		# backface-culled.
		self.zone_node.set_two_sided(True)

		bb = self.zone.zone_bb
		frame_distance = max(bb.half_size.x, bb.half_size.y, bb.half_size.z, 10.0) * 2.5
		self.orbit_camera.frame((bb.center.x, bb.center.y, bb.center.z), frame_distance)

	def _clear_zone_geometry(self):
		if self.zone_node is not None:
			self.zone_node.remove_node()
			self.zone_node = None

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

	def _select_continent(self, continent_name):
		"""Resolves and caches continent_bounds for `continent_name` -- called
		only when the combo selection actually changes, not every frame (both
		files this reads are full-parse, no random access, see
		nel/tools/pynel/docs/packed_sheets.md)."""
		self.selected_continent_name = continent_name
		self.continent_bounds = None
		self._bounds_error = None
		live_data_path = app_settings.load().live_data_path
		try:
			self.continent_bounds = continent_selector.resolve_continent_bounds(live_data_path, continent_name)
		except (OSError, continent_selector.ContinentSelectorError) as exc:
			self._bounds_error = f"Failed to resolve bounds: {exc}"

	def panel_title(self):
		return "Landscape Editor"

	def draw_panel(self):
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
					self._select_continent(cont_loc.continent_name)
				if selected:
					imgui.set_item_default_focus()
			imgui.end_combo()

		if self._bounds_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._bounds_error)
		elif self.continent_bounds is not None:
			min_x, min_y, max_x, max_y = self.continent_bounds
			imgui.text(f"Bounds: X [{min_x:.0f} .. {max_x:.0f}]  Y [{min_y:.0f} .. {max_y:.0f}]")

		imgui.separator()
		imgui.text("Zone (select a .zone/.zonew/.zonel in the Explorer)")
		if self._zone_error is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._zone_error)
		elif self.zone is not None:
			imgui.text(f"Zone {self.zone.zone_id} -- {len(self.zone.patchs)} patches")
			if imgui.button("Top view (2D)"):
				self.orbit_camera.snap_to_axis("+z")


def main(argv=None):
	LandscapeEditorApp().run()


if __name__ == "__main__":
	main()
