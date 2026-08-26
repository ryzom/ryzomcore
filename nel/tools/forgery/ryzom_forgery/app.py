import json
import re
import time
from pathlib import Path

from direct.showbase.ShowBase import ShowBase
from panda3d.core import AmbientLight, DirectionalLight, Texture, WindowProperties

import p3dimgui
import imgui_bundle
from imgui_bundle import imgui, imgui_ctx

from .commands import CommandRegistry
from .explorer import DEFAULT_FILTER, Explorer
from .splash import Splash
from .sysinfo import SysInfoBar
from .workspace_setup_dialog import WorkspaceSetupDialog

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

# Panda3D's own default window size (~800x600 depending on config.prc) is
# cramped for these tools -- used as the starting size the very first time an
# app runs, before any geometry has been saved for it. From then on, the
# window's last position/size (per app, keyed by its title) is remembered
# across launches -- see _load_window_geometry()/_save_window_geometry().
_DEFAULT_WINDOW_SIZE = (1600, 900)
_WINDOW_GEOMETRY_DIR = Path.home() / ".ryzom_forgery"
_ICON_PATH = Path(__file__).resolve().parent / "forgery.png"
_SPLASH_PATH = Path(__file__).resolve().parent / "splashscreen.png"


def _slugify(title):
	return re.sub(r"[^a-z0-9]+", "_", title.lower()).strip("_")


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
		self._window_geometry_path = _WINDOW_GEOMETRY_DIR / f"{_slugify(title)}.json"
		geometry = self._load_window_geometry()

		# winfo_screenwidth()/screenheight() report the whole virtual desktop
		# on multi-monitor X11 setups, so centering on those would straddle
		# both screens -- center on the target window's own monitor instead,
		# using its last known position/size (falling back to the virtual
		# desktop center only on this app's very first launch).
		center_on = (geometry["x"], geometry["y"], geometry["width"], geometry["height"]) if geometry else None
		splash = Splash(_SPLASH_PATH, center_on=center_on) if _SPLASH_PATH.exists() else None
		if splash is not None:
			time.sleep(1)

		ShowBase.__init__(self)

		props = WindowProperties()
		props.setTitle(title)
		if _ICON_PATH.exists():
			props.setIconFilename(str(_ICON_PATH))
		if geometry is not None:
			props.setOrigin(geometry["x"], geometry["y"])
			props.setSize(geometry["width"], geometry["height"])
		else:
			props.setSize(*_DEFAULT_WINDOW_SIZE)
		self.win.requestProperties(props)

		# Default trackball camera controls would conflict with a tool's own
		# camera controller.
		self.disableMouse()

		# Panda3D's default lens near plane (1.0 world unit) clips out small
		# shapes entirely once OrbitCamera.frame() puts the camera closer
		# than that -- e.g. a hairstyle's own bbox radius can floor out at
		# its 0.1 minimum (see object_editor.py's _display_shape()), giving
		# a framing distance well under 1.0, so the whole mesh sat behind the
		# near plane and simply never rendered. Ryzom assets range from tiny
		# props to whole ecosystems, so both near and far need real headroom
		# rather than relying on Panda3D's one-size-fits-all default.
		self.camLens.set_near_far(0.02, 20000.0)

		p3dimgui.init()
		self._load_icon_font()
		self._panel_watermark_tex_ref = None
		if _SPLASH_PATH.exists():
			watermark_texture = Texture()
			watermark_texture.read(str(_SPLASH_PATH))
			self._panel_watermark_tex_ref = self.imgui.loadTexture(watermark_texture)

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
		self.workspace_setup_dialog = WorkspaceSetupDialog()
		self.explorer = Explorer(self, Path(explorer_root), self.commands,
		                          default_filter=explorer_default_filter)
		self.explorer.on_selection_changed = self._on_explorer_selection_changed

		self.accept("imgui-new-frame", self.draw_ui)

		if splash is not None:
			splash.close()

	def windowEvent(self, win):
		super().windowEvent(win)
		if win == self.win:
			self._save_window_geometry()

	def _load_window_geometry(self):
		try:
			data = json.loads(self._window_geometry_path.read_text())
		except (OSError, ValueError):
			return None
		if not all(isinstance(data.get(key), int) for key in ("x", "y", "width", "height")):
			return None
		return data

	def _save_window_geometry(self):
		props = self.win.getProperties()
		data = {"x": props.getXOrigin(), "y": props.getYOrigin(),
		        "width": props.getXSize(), "height": props.getYSize()}
		try:
			self._window_geometry_path.parent.mkdir(parents=True, exist_ok=True)
			self._window_geometry_path.write_text(json.dumps(data))
		except OSError:
			pass

	def _load_icon_font(self):
		# self.large_icon_font (1.5x _ICON_FONT_SIZE, standalone -- not merged
		# into the default font like the one above) is for spots that want a
		# bigger icon-only button than the normal UI text size, e.g.
		# object_editor.py's viewport toggle bars: push_font()/pop_font()
		# around those buttons, since there's no per-window font-scale API in
		# this imgui_bundle version to reach for instead.
		if not _ICON_FONT_PATH.exists():
			self.large_icon_font = None
			self.large_icon_font_size = _ICON_FONT_SIZE * 1.5
			return
		font_config = imgui.ImFontConfig()
		font_config.merge_mode = True
		imgui.get_io().fonts.add_font_from_file_ttf(str(_ICON_FONT_PATH), _ICON_FONT_SIZE, font_config)
		self.large_icon_font_size = _ICON_FONT_SIZE * 1.5
		self.large_icon_font = imgui.get_io().fonts.add_font_from_file_ttf(str(_ICON_FONT_PATH), self.large_icon_font_size)

	def draw_panel(self):
		"""Override in subclasses to draw the app-specific right panel content each frame."""
		pass

	def panel_title(self):
		"""Override in subclasses for a more useful right-panel window title
		than the generic "Panel" (e.g. the name of whatever's currently
		loaded there)."""
		return "Panel"

	_PANEL_WATERMARK_SIZE = 256
	_PANEL_WATERMARK_COLOR = (1.0, 1.0, 1.0, 0.30)

	_PANEL_WATERMARK_MARGIN = 12

	def _draw_panel_watermark(self):
		"""Faint branding image horizontally centered at the bottom of the
		panel, drawn before draw_panel()'s own widgets so it sits behind them
		in the same window's draw list -- low enough alpha to stay out of the
		way of whatever's drawn on top."""
		window_pos = imgui.get_window_pos()
		window_size = imgui.get_window_size()
		size = self._PANEL_WATERMARK_SIZE
		margin = self._PANEL_WATERMARK_MARGIN
		p_max = (window_pos.x + window_size.x / 2 + size / 2, window_pos.y + window_size.y - margin)
		p_min = (p_max[0] - size, p_max[1] - size)
		imgui.get_window_draw_list().add_image(
			self._panel_watermark_tex_ref, p_min, p_max, (0, 0), (1, 1),
			imgui.get_color_u32(self._PANEL_WATERMARK_COLOR))

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
		self.workspace_setup_dialog.draw()

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
		# "###panel" keeps the window's ImGui identity (position/size
		# persistence) stable even though the displayed title itself
		# (panel_title()) changes with whatever's currently loaded.
		with imgui_ctx.begin(f"{self.panel_title()}###panel", flags=_PINNED_FLAGS):
			self.panel_width = imgui.get_window_size().x
			if self._panel_watermark_tex_ref is not None:
				self._draw_panel_watermark()
			self.draw_panel()
