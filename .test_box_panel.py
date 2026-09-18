"""Disposable driver for ui/box.py + ui/box_panel.py -- project-todos/forgery/
box_panel_layout_system.md. Only builds a tree and runs it; Panel/Widget/
Box themselves are the REAL code from the project, not defined here (see
box_panel.py's own docstring on why they used to live in this file and
don't anymore).

Current tree: a 100px, resizable Panel, 2 Box children, a centered label in
the top one, a centered text input in the bottom one.

Run: ~/.cache/venvs/forgery.ryzom.com/bin/python .test_box_panel.py
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent / "nel" / "tools" / "forgery"))

from direct.showbase.ShowBase import ShowBase
from panda3d.core import WindowProperties

import p3dimgui
from imgui_bundle import imgui

from ryzom_forgery.ui.box import Box, STRETCH
from ryzom_forgery.ui.box_panel import Panel, ButtonWidget, LabelWidget, TextInputWidget


class ColoredBox(Box):
	"""A plain Box that also fills its own external_rect with a flat color
	-- purely a test-visualization aid, not part of the real ui/ package.
	Drawn BEFORE laying out children, so real widget content nested inside
	ends up on top in the same ImDrawList (draw order matters here)."""

	def __init__(self, color, **kwargs):
		super().__init__(**kwargs)
		self.color = color

	def layout(self, allocated):
		x, y, w, h = allocated
		imgui.get_window_draw_list().add_rect_filled((x, y), (x + w, y + h), imgui.get_color_u32(self.color))
		super().layout(allocated)


class TestBoxPanelApp(ShowBase):
	def __init__(self):
		ShowBase.__init__(self)
		props = WindowProperties()
		props.setTitle("ui/box.py test harness -- label/input")
		props.setSize(400, 400)
		self.win.requestProperties(props)
		self.disableMouse()

		p3dimgui.init()

		self.panel = Panel(dock="left", h=100, resizable=True, min_size=60, max_size=400, title="Label/Input test")
		top = self.panel.add(ColoredBox((0.35, 0.75, 0.35, 1.0), dock="top", h=STRETCH, v=STRETCH))
		top.add(LabelWidget("Hello", dock="center", h=60, v=20))
		bottom = self.panel.add(ColoredBox((0.30, 0.50, 0.85, 1.0), dock="top", h=STRETCH, v=STRETCH))
		bottom.add(LabelWidget("Label:", dock="left", h=50, v=STRETCH))
		bottom.add(ButtonWidget("OK", dock="right", h=50, v=STRETCH))
		bottom.add(TextInputWidget("type here", dock="left", h=STRETCH, v=STRETCH))

		self.screen_root = Box(dock="left", h=STRETCH, v=STRETCH)
		self.screen_root.add(self.panel)

		self.accept("imgui-new-frame", self.draw_ui)

	def draw_ui(self):
		display_size = imgui.get_io().display_size
		self.screen_root.layout((0.0, 0.0, display_size.x, display_size.y))


TestBoxPanelApp().run()
