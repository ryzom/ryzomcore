"""Disposable driver for ui/box_panel.py's Widget-level default margin --
project-todos/forgery/box_ui_migration__explorer_panel.md (Nuno 2026-09-20:
"je pense [qu'il faut] un margin par defaut... autour des widgets qui sont
visible : boutons, listes, texte, entry, etc..."). Only builds a tree and
runs it; Box/Widget/LeftPanel/every concrete widget class are the REAL code
from the project, not redefined here (see .test_box_panel.py's own
docstring on why a driver script never redefines them).

Several stacked TextInputWidget ("Entry") rows only (Nuno 2026-09-20,
after confirming the icon/label/combo/list rows all looked correct at an
exaggerated margin=20: "met que des Entry" -- isolates whether several
Entries stacked directly on top of each other show a clean, consistent
gap between them at the real default margin=4). Each row filled with its
OWN flat background color (ColoredBox, same trick as .test_box_panel.py)
so the widget's own margin-inset content is unambiguous against its full
row slot.

Run: ~/.cache/venvs/forgery.ryzom.com/bin/python .test_widget_margin.py
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent / "nel" / "tools" / "forgery"))

from direct.showbase.ShowBase import ShowBase
from panda3d.core import WindowProperties

import p3dimgui
from imgui_bundle import imgui

from ryzom_forgery.ui.box import Box, STRETCH
from ryzom_forgery.ui.box_panel import LeftPanel, TextInputWidget


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


_ROW_COLORS = [
	(0.35, 0.35, 0.75, 1.0),
	(0.35, 0.65, 0.35, 1.0),
	(0.75, 0.55, 0.25, 1.0),
	(0.65, 0.35, 0.65, 1.0),
	(0.35, 0.65, 0.65, 1.0),
]
_SLOT_COLORS = [
	(0.20, 0.20, 0.45, 1.0),
	(0.20, 0.40, 0.20, 1.0),
	(0.45, 0.25, 0.15, 1.0),
	(0.35, 0.15, 0.35, 1.0),
	(0.15, 0.35, 0.35, 1.0),
]


class TestWidgetMarginApp(ShowBase):
	def __init__(self):
		ShowBase.__init__(self)
		props = WindowProperties()
		props.setTitle("Widget margin test -- stacked Entries only")
		props.setSize(500, 500)
		self.win.requestProperties(props)
		self.disableMouse()

		p3dimgui.init()
		imgui.get_io().set_ini_filename("")

		self.panel = LeftPanel(size=300, title="Margin test", padding=4.0)

		self.accept("imgui-new-frame", self.draw_ui)

	def draw_ui(self):
		# Rebuilt every frame using the REAL imgui.get_frame_height() (not
		# available before the first frame starts) -- same principle as the
		# real Explorer's own build_content_box().
		#
		# Row height = frame_height + 2*margin, NOT frame_height alone
		# (Nuno 2026-09-20: "y a plus de margin la"): imgui.get_frame_height()
		# is already the natural, no-slack height ImGui itself uses for a
		# single-line input (its own FramePadding included) -- reserving
		# exactly that for the ROW leaves NO room to also carve a margin out
		# of it. Once margin is subtracted, internal_rect is shorter than
		# input_text's own natural height, but input_text (unlike a button)
		# never shrinks to fit a smaller given height -- it just renders at
		# its natural size regardless, overflowing the tiny internal_rect
		# and visually erasing the margin entirely. The row must reserve the
		# natural content height PLUS the margin on top of it, not expect
		# the margin to be carved out of an already-tight natural-size row.
		margin = 4.0
		frame_height = imgui.get_frame_height() + 2 * margin
		self.panel.children = []
		# Every row: an OUTER ColoredBox (the row's full slot) plus an INNER
		# ColoredBox exactly at the Entry's own allotted slot (before ITS
		# OWN margin insets further) -- TextInputWidget draws no background
		# of its own outside the actual input frame, so without this second
		# color its real bounds (vs. the row's) are invisible (Nuno
		# 2026-09-20: "il faut que tu mettre un background sinon on peut
		# pas bien voir"). Five Entries stacked directly on top of each
		# other, no separator between them -- isolates whether the gap
		# BETWEEN rows (not just within one) is clean and consistent.
		for i in range(5):
			row = self.panel.add(ColoredBox(_ROW_COLORS[i], dock="top", v=frame_height))
			slot = row.add(ColoredBox(_SLOT_COLORS[i], dock="left", h=STRETCH))
			slot.add(TextInputWidget(hint=f"entry {i}", dock="left", h=STRETCH))

		display_size = imgui.get_io().display_size
		available = (0.0, 0.0, display_size.x, display_size.y)
		self.panel.layout(available)


TestWidgetMarginApp().run()
