"""Ryzom Forgery landscape editor: 3D landscape composition/build tool,
replacing the current 2D bitmap-tile Ligo editor.

Scaffolding only for now (project-todos/forgery/landscape_editor.md step 1)
-- app entry point, window, free camera. No terrain rendering yet, see the
chantier for the planned progressive rendering steps.
"""

from pathlib import Path

from imgui_bundle import imgui

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.camera import OrbitCamera
from ryzom_forgery import continent_selector
from ryzom_forgery import live_data
from ryzom_forgery import settings as app_settings

APP_INFO = {
	"id": "landscape_editor",
	"name": "Landscape Editor",
	"subtitle": "Landscape Editor",
	"description": "3D landscape composition/build tool (.land/.zone).",
}


class LandscapeEditorApp(ForgeryApp):
	def __init__(self):
		configured_dirs = app_settings.load().search_paths
		explorer_root = Path(configured_dirs[0].path) if configured_dirs else Path.home()
		ForgeryApp.__init__(self, explorer_root=explorer_root, title="Ryzom Forgery - Landscape Editor",
		                     explorer_default_filter="*.land")

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


def main(argv=None):
	LandscapeEditorApp().run()


if __name__ == "__main__":
	main()
