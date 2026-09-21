"""Disposable driver for ui/box_panel.py's TreeListWidget --
project-todos/forgery/box_ui_migration__explorer_panel.md. Only builds a
tree and runs it; TreeListWidget/LeftPanel themselves are the REAL code
from the project, not defined here (see .test_box_panel.py's own docstring
on why a driver script never redefines them).

A 200px LeftPanel, padding=20 (Nuno 2026-09-20: "un padding de 20 au panel
qu'on vois si c'est bon"), red window_bg so the panel's real edges are
unambiguous, filled with a TreeListWidget over fake nested folder/file data
(no real Explorer/disk involved) -- validates that every row's right edge
respects the panel's padding, and that expanding a folder grows the
reserved height correctly across frames, before wiring this into the real
Wexplorer/PathBar.

Run: ~/.cache/venvs/forgery.ryzom.com/bin/python .test_tree_list_widget.py
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent / "nel" / "tools" / "forgery"))

from direct.showbase.ShowBase import ShowBase
from panda3d.core import WindowProperties

import p3dimgui
from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui

from ryzom_forgery.ui.box_panel import LeftPanel, TreeListWidget

# folder name -> dict (subfolders, recursively) | None (a leaf file)
_FAKE_TREE = {
	"shapes": {
		"anlor_hof_helmet01.shape": None,
		"anlor_stick.shape": None,
		"anlor_weapon01.shape": None,
	},
	"textures": {
		"tex1.dds": None,
		"sub": {
			"tex2.dds": None,
			"tex3.dds": None,
		},
	},
	"empty_folder": {},
	"root_file.shape": None,
}


def _draw_fake_tree(entries):
	for name, children in entries.items():
		if children is None:
			imgui.tree_node_ex(
				name, imgui.TreeNodeFlags_.leaf.value | imgui.TreeNodeFlags_.no_tree_push_on_open.value,
				f"{fa_icons.ICON_FA_FILE} {name}")
		else:
			opened = imgui.tree_node_ex(
				name, imgui.TreeNodeFlags_.open_on_arrow.value,
				f"{fa_icons.ICON_FA_FOLDER} {name} ({len(children)})")
			if opened:
				_draw_fake_tree(children)
				imgui.tree_pop()


class TestTreeListWidgetApp(ShowBase):
	def __init__(self):
		ShowBase.__init__(self)
		props = WindowProperties()
		props.setTitle("TreeListWidget test -- fake data, padding=20, red bg")
		props.setSize(400, 500)
		self.win.requestProperties(props)
		self.disableMouse()

		p3dimgui.init()
		# Disabled: without this, ImGui persists this window's size to
		# imgui.ini keyed by its title, and re-running this disposable
		# script reuses that stale size instead of the size=200 below
		# (Panel.layout()'s cond=first_use_ever only applies size on a
		# window ImGui has never seen before) -- confusing when iterating.
		imgui.get_io().set_ini_filename("")

		self.panel = LeftPanel(size=200, title="Fake Explorer", padding=20.0)
		# auto_height=False (default): v=STRETCH, fills the whole panel
		# height and scrolls for overflow (Nuno 2026-09-20: "etire le
		# TreeView verticalement aussi").
		self.panel.add(TreeListWidget(
			lambda w: _draw_fake_tree(_FAKE_TREE), "##fake-tree",
			dock="top"))

		self.accept("imgui-new-frame", self.draw_ui)

	def draw_ui(self):
		display_size = imgui.get_io().display_size
		available = (0.0, 0.0, display_size.x, display_size.y)
		imgui.push_style_color(imgui.Col_.window_bg.value, (1.0, 0.0, 0.0, 1.0))
		# TreeListWidget's own child window shares Col_.child_bg -- green so
		# its real confined bounds are visible against the red panel
		# (Nuno 2026-09-20: "remplis le TreeListWidget de vert").
		imgui.push_style_color(imgui.Col_.child_bg.value, (0.0, 0.6, 0.0, 1.0))
		self.panel.layout(available)
		imgui.pop_style_color()
		imgui.pop_style_color()


TestTreeListWidgetApp().run()
