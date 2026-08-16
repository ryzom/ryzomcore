from pathlib import Path

from direct.showbase.ShowBase import ShowBase
from panda3d.core import AmbientLight, DirectionalLight, WindowProperties

import p3dimgui
import imgui_bundle
from imgui_bundle import imgui, imgui_ctx

from .commands import CommandRegistry
from .explorer import DEFAULT_FILTER, Explorer
from .sysinfo import SysInfoBar

# Font Awesome 4 (the only icon font imgui_bundle ships a .ttf for), merged
# into the default font so any tool app can use icons_fontawesome_4's
# ICON_FA_* glyphs in a normal imgui.text/button/etc call instead of a text
# label -- the default ImGui font alone has no icon glyphs at all.
_ICON_FONT_PATH = Path(imgui_bundle.__file__).resolve().parent / "assets" / "fonts" / "fontawesome-webfont.ttf"
_ICON_FONT_SIZE = 14.0

# Sysinfo is a fixed thin strip; explorer/panel are pinned in place (no_move)
# but resizable in width via SetNextWindowSizeConstraints (height is locked
# to the constraint range instead, so only the width can actually change).
_PINNED_FLAGS = imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
_SYSINFO_FLAGS = _PINNED_FLAGS | imgui.WindowFlags_.no_resize.value | imgui.WindowFlags_.no_title_bar.value
_SIDE_PANEL_MIN_WIDTH = 150
_SIDE_PANEL_MAX_WIDTH = 900


class ForgeryApp(ShowBase):
	"""Base class for Ryzom Forgery tool apps.

	Opens a Panda3D window with a Dear ImGui overlay, and lays out the
	standard Ryzom Forgery UI every tool app shares: a sysinfo bar at the
	bottom, a file/`.bnp` explorer on the left, and an app-defined panel on
	the right. The center is left free for the 3D viewport. Subclasses
	implement their tool-specific logic and override draw_panel() to draw
	their own UI in the right panel each frame.
	"""

	def __init__(self, explorer_root, title="Ryzom Forgery",
	             explorer_width=300, panel_width=320, sysinfo_height=28,
	             explorer_default_filter=DEFAULT_FILTER):
		ShowBase.__init__(self)

		props = WindowProperties()
		props.setTitle(title)
		self.win.requestProperties(props)

		# Default trackball camera controls would conflict with a tool's own
		# camera controller.
		self.disableMouse()

		p3dimgui.init()
		self._load_icon_font()

		# Applying a Material (as tool apps do for shape rendering) has no
		# visible effect without at least one light in the scene -- without
		# this, lit geometry renders black regardless of its texture.
		ambient = AmbientLight("ambient")
		ambient.set_color((0.4, 0.4, 0.4, 1))
		self.render.set_light(self.render.attach_new_node(ambient))

		sun = DirectionalLight("sun")
		sun.set_color((0.8, 0.8, 0.8, 1))
		sun_np = self.render.attach_new_node(sun)
		sun_np.set_hpr(45, -45, 0)
		self.render.set_light(sun_np)

		self.explorer_width = explorer_width
		self.panel_width = panel_width
		self.sysinfo_height = sysinfo_height

		self.commands = CommandRegistry()
		self.sysinfo = SysInfoBar()
		self.explorer = Explorer(self, Path(explorer_root), self.commands,
		                          default_filter=explorer_default_filter)
		self.explorer.on_selection_changed = self._on_explorer_selection_changed

		self.accept("imgui-new-frame", self.draw_ui)

	def _load_icon_font(self):
		if not _ICON_FONT_PATH.exists():
			return
		font_config = imgui.ImFontConfig()
		font_config.merge_mode = True
		imgui.get_io().fonts.add_font_from_file_ttf(str(_ICON_FONT_PATH), _ICON_FONT_SIZE, font_config)

	def draw_panel(self):
		"""Override in subclasses to draw the app-specific right panel content each frame."""
		pass

	def on_selection_changed(self, items):
		"""Override in subclasses to react to the explorer's selection changing."""
		pass

	def _on_explorer_selection_changed(self, items):
		if not items:
			self.sysinfo.set_status("")
		elif len(items) == 1:
			item = items[0]
			label = f"{item.bnp_path.name}!{item.name}" if item.bnp_path is not None else str(item.path)
			self.sysinfo.set_status(label)
		else:
			self.sysinfo.set_status(f"{len(items)} selected")

		self.on_selection_changed(items)

	def draw_ui(self):
		display_size = imgui.get_io().display_size
		width, height = display_size.x, display_size.y
		body_height = height - self.sysinfo_height

		imgui.set_next_window_pos((0, body_height))
		imgui.set_next_window_size((width, self.sysinfo_height))
		with imgui_ctx.begin("##sysinfo", flags=_SYSINFO_FLAGS):
			self.sysinfo.draw()

		once = imgui.Cond_.first_use_ever.value

		imgui.set_next_window_pos((0, 0))
		imgui.set_next_window_size((self.explorer_width, body_height), cond=once)
		imgui.set_next_window_size_constraints(
			(_SIDE_PANEL_MIN_WIDTH, body_height), (_SIDE_PANEL_MAX_WIDTH, body_height))
		with imgui_ctx.begin("Explorer", flags=_PINNED_FLAGS):
			self.explorer_width = imgui.get_window_size().x
			self.explorer.draw()

		imgui.set_next_window_pos((width - self.panel_width, 0))
		imgui.set_next_window_size((self.panel_width, body_height), cond=once)
		imgui.set_next_window_size_constraints(
			(_SIDE_PANEL_MIN_WIDTH, body_height), (_SIDE_PANEL_MAX_WIDTH, body_height))
		with imgui_ctx.begin("Panel", flags=_PINNED_FLAGS):
			self.panel_width = imgui.get_window_size().x
			self.draw_panel()
