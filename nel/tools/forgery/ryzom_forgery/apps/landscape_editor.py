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

	def panel_title(self):
		return "Landscape Editor"

	def draw_panel(self):
		imgui.text("Landscape Editor -- work in progress.")


def main(argv=None):
	LandscapeEditorApp().run()


if __name__ == "__main__":
	main()
