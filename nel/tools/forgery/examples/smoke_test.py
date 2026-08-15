"""Manual smoke test for ryzom_forgery: window + orbit camera + standard
sysinfo/explorer/panel layout.

Not an actual tool, just a way to visually check the base lib works before
any real tool app (e.g. the object editor) is built on top of it. The
explorer root is this examples/ directory itself, just so there's something
to browse without depending on a real Ryzom data tree.
"""

from pathlib import Path

from panda3d.core import CardMaker, Point3

from imgui_bundle import imgui

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.camera import OrbitCamera


class SmokeTestApp(ForgeryApp):
	def __init__(self):
		ForgeryApp.__init__(self, explorer_root=Path(__file__).resolve().parent,
		                     title="Ryzom Forgery - smoke test")

		card_maker = CardMaker("test-card")
		card_maker.setFrame(-2, 2, -2, 2)
		self.card = self.render.attachNewNode(card_maker.generate())
		self.card.setColor(0.9, 0.3, 0.2, 1)

		self.orbit_camera = OrbitCamera(self, target=Point3(0, 0, 0), distance=10.0)

		self.commands.register_global("Print path", lambda items: print([str(i.path) for i in items]))

	def draw_panel(self):
		imgui.text("Middle-drag: orbit")
		imgui.text("Shift + middle-drag: pan")
		imgui.text("Wheel: zoom")

	def on_selection_changed(self, items):
		print("Selection changed:", [item.name for item in items])


if __name__ == "__main__":
	SmokeTestApp().run()
