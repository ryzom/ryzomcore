import json
import math
import os
import re
import subprocess
import sys
import time
from pathlib import Path

from direct.showbase.ShowBase import ShowBase
from panda3d.core import AmbientLight, DirectionalLight, Texture, WindowProperties

import p3dimgui
import imgui_bundle
from imgui_bundle import imgui, imgui_ctx

from . import settings as app_settings
from .commands import CommandRegistry
from .explorer import DEFAULT_FILTER, Explorer
from .splash import Splash
from .sysinfo import SysInfoBar
from .workspace_setup_dialog import WorkspaceSetupDialog

# Font Awesome 6 Solid -- imgui_bundle bundles this .otf too (not just the
# older FA4 .ttf a stale comment here used to claim was "the only one"),
# merged into the default font so any tool app can use icons_fontawesome_6's
# ICON_FA_* glyphs in a normal imgui.text/button/etc call instead of a text
# label -- the default ImGui font alone has no icon glyphs at all. Switched
# from FA4 2026-09-03 (Nuno): FA4 kept missing icons Forgery actually wanted
# (wind, bone, sync, home, save, ...) -- FA6 Solid covers every ICON_FA_*
# name used across this codebase, see the icons_fontawesome_6 module for the
# full up-to-date list instead of guessing from FA4-era names.
_ICON_FONT_PATH = (
	Path(imgui_bundle.__file__).resolve().parent / "assets" / "fonts" / "Font_Awesome_6_Free-Solid-900.otf")

# UI text font choices offered in Settings (see Settings.ui_font_name/
# ui_font_size in settings.py, and _draw_ui_font_settings() in
# object_editor.py) -- every regular (non-icon) .ttf imgui_bundle itself
# ships, so no extra font files need bundling in the wheel. Key is what's
# stored in settings.toml and shown in the picker; value is the path
# relative to imgui_bundle's own assets/fonts/ directory.
_FONTS_DIR = Path(imgui_bundle.__file__).resolve().parent / "assets" / "fonts"
_AVAILABLE_FONTS = {
	"Roboto Bold": "Roboto/Roboto-Bold.ttf",
	"Roboto Regular": "Roboto/Roboto-Regular.ttf",
	"Roboto Bold Italic": "Roboto/Roboto-BoldItalic.ttf",
	"Roboto Italic": "Roboto/Roboto-RegularItalic.ttf",
	"Droid Sans": "DroidSans.ttf",
	"Inconsolata (monospace)": "Inconsolata-Medium.ttf",
}
_DEFAULT_FONT_NAME = "Roboto Bold"


def _dpi_scale() -> float:
	"""RYZOM_FORGERY_DPI_SCALE, set by ryztart's launcher (see
	ryzom_forgery_launcher/launcher.py's _dpiScale(), pywebview's own
	screen.scale -- Qt/GTK/Cocoa under the hood, abstracts X11/Wayland/
	Windows/macOS DPI detection none of which Panda3D's own windowing has
	any support for). Multiplies every UI font size so text stays a
	consistent physical size instead of tracking Panda3D's raw (DPI-unaware
	on Linux) window pixel size. 1.0 (no scaling) when launched directly
	(e.g. dev.sh, no ryztart involved) or on a malformed/missing value."""
	try:
		return float(os.environ.get("RYZOM_FORGERY_DPI_SCALE", "1.0"))
	except ValueError:
		return 1.0


def _effective_ui_scale(settings) -> float:
	"""The actual multiplier applied to every UI size (fonts, icons, and --
	via ForgeryApp.__init__'s style.scale_all_sizes() call -- every style
	metric: paddings, spacing, button/scrollbar sizes, corner rounding...):
	_dpi_scale() (auto-detected, from ryztart) times Settings.dpi_scale (the
	user's own manual fine-tune, set from the first-launch setup popup or
	Settings > UI later, see workspace_setup_dialog.py/object_editor.py's
	_draw_ui_font_settings()). Every call site that used to read
	_dpi_scale() directly for sizing now reads this instead."""
	return _dpi_scale() * settings.dpi_scale

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
		# Kept open past the end of __init__ -- closed on the first real
		# draw_ui() call below instead, so it covers the actual gap until
		# something is on screen, not just Python-side construction (there's
		# a real gap between __init__ returning and ShowBase's run() loop
		# firing its first frame).
		self._splash = Splash(_SPLASH_PATH, center_on=center_on) if _SPLASH_PATH.exists() else None

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
		style = imgui.get_style()
		# Rounded corners app-wide (buttons, inputs, checkboxes...) instead of
		# ImGui's sharp-cornered default -- purely cosmetic, applies to every
		# Forgery tool app since it's set once here in the shared base class.
		style.frame_rounding = 4.0
		# Dear ImGui's stock dark theme leaves every popup/window background
		# noticeably translucent (WindowBg/PopupBg alpha ~0.94 by default) --
		# never touched before now, so every popup in every Forgery app has
		# always looked slightly hazy. Fully opaque backgrounds read as
		# crisp/solid instead.
		style.set_color_(imgui.Col_.window_bg.value, (0.06, 0.06, 0.06, 1.0))
		style.set_color_(imgui.Col_.popup_bg.value, (0.08, 0.08, 0.08, 1.0))
		# Dear ImGui's stock dark theme's default Button color is itself a
		# blue (~#4296fa) -- fought for contrast against icon_colors.py's own
		# blue/sky/indigo pastel tints (an icon glyph in that hue family all
		# but disappeared into its own button's idle background). Neutral
		# gray instead, distinct from window_bg/popup_bg above, so every
		# icon color (and the existing blue _ACTIVE_COLOR toggle-highlight,
		# see ui_helpers.py) reads clearly against it (Nuno, 2026-09-03).
		style.set_color_(imgui.Col_.button.value, (0.2, 0.2, 0.2, 1.0))
		style.set_color_(imgui.Col_.button_hovered.value, (0.28, 0.28, 0.28, 1.0))
		style.set_color_(imgui.Col_.button_active.value, (0.35, 0.35, 0.35, 1.0))
		# ModalWindowDimBg is meant to only dim whatever's *behind* a modal
		# popup -- in this rendering pipeline it was instead blending over
		# the modal's own content too (confirmed: a pushed pure-black text
		# color was rendering as 0x0A instead of 0x00, exactly matching
		# alpha=0.4 blended with the dim color -- 0.4*0.1 + 0.6*0 = 0.04).
		# Disabled entirely rather than just toned down, so every popup's own
		# colors/text render exactly as set -- the trade-off is no dimming of
		# whatever's behind a modal popup anymore.
		style.set_color_(imgui.Col_.modal_window_dim_bg.value, (0.0, 0.0, 0.0, 0.0))
		# Dear ImGui's dark theme default for regular text isn't pure white
		# either -- force it, for the same "everything looks a bit washed
		# out" reason as the background colors above.
		style.set_color_(imgui.Col_.text.value, (1.0, 1.0, 1.0, 1.0))
		# Whole-UI DPI scale (paddings, spacing, button/scrollbar sizes, the
		# frame_rounding/colors just set above included) -- ScaleAllSizes()
		# only ever multiplies whatever's already there, so this has to run
		# after every other one-off style tweak above it, not before.
		# Applied exactly once here, at startup, from ImGui's own freshly
		# built (p3dimgui.init(), right above) style -- deliberately NOT
		# also live-rescaled while the DPI control in the setup popup/
		# Settings tab is being dragged (see set_live_ui_scale_preview()):
		# repeatedly calling ScaleAllSizes() relative to an already-scaled
		# style floors several fields towards zero over many small steps,
		# which crashed for real in earlier testing. self._ui_scale is kept
		# around so _load_ui_font()/_load_icon_font() (below) and the live
		# font-only preview can read it.
		self._ui_scale = _effective_ui_scale(app_settings.load())
		style.scale_all_sizes(self._ui_scale)
		self._load_ui_font()
		self._load_icon_font()
		self._panel_watermark_tex_ref = None
		if _SPLASH_PATH.exists():
			watermark_texture = Texture()
			watermark_texture.read(str(_SPLASH_PATH))
			self._panel_watermark_tex_ref = self.imgui.loadTexture(watermark_texture)

		# Applying a Material (as tool apps do for shape rendering) has no
		# visible effect without at least one light in the scene -- without
		# this, lit geometry renders black regardless of its texture.
		# Stored as instance attributes (not just local vars) so a subclass
		# (e.g. object_editor.py's Lighting panel) can read/adjust them
		# later -- the creation/defaults themselves are unchanged.
		self.ambient_light = AmbientLight("ambient")
		self.ambient_light.set_color((0.4, 0.4, 0.4, 1))
		self.ambient_light_np = self.render.attach_new_node(self.ambient_light)
		self.render.set_light(self.ambient_light_np)

		self.sun_light = DirectionalLight("sun")
		self.sun_light.set_color((0.8, 0.8, 0.8, 1))
		self.sun_light_np = self.render.attach_new_node(self.sun_light)
		self.sun_light_np.set_hpr(45, -45, 0)
		self.render.set_light(self.sun_light_np)

		self.explorer_width = explorer_width
		self.panel_width = panel_width
		self.sysinfo_height = sysinfo_height

		self.commands = CommandRegistry()
		self.sysinfo = SysInfoBar()
		self.workspace_setup_dialog = WorkspaceSetupDialog()
		self.workspace_setup_dialog.on_dpi_preview_changed = self.set_live_ui_scale_preview
		self.workspace_setup_dialog.on_setup_finished = self.relaunch
		self.explorer = Explorer(self, Path(explorer_root), self.commands,
		                          default_filter=explorer_default_filter)
		self.explorer.on_selection_changed = self._on_explorer_selection_changed

		# (section, field_key, expires_at) or None -- see
		# request_settings_attention() below.
		self._settings_attention = None

		self.accept("imgui-new-frame", self.draw_ui)

		# ShowBase.windowEvent() (see the override below) already calls
		# self.userExit() itself the moment the OS's own close button/Alt+F4/
		# etc sets the window's getOpen() to False -- userExit() -> Panda3D's
		# own finalizeExit() -> self.exitFunc() (if set) -> sys.exit(). That
		# SystemExit unwinds straight out of super().windowEvent(win) inside
		# our own override below, well before it would ever reach any code
		# placed after that call there -- exitFunc is the one hook Panda3D
		# actually guarantees to run first, no matter which of the two paths
		# (this, or the in-app Quit button, which also just calls
		# self.userExit() -- see object_editor.py) triggered the exit.
		self.exitFunc = self._on_exit

	def windowEvent(self, win):
		super().windowEvent(win)
		if win == self.win:
			self._save_window_geometry()

	def _on_exit(self):
		"""Hook for subclasses: called once, right before the process
		actually exits (self.exitFunc, set in __init__ above), regardless
		of whether that was requested via the in-app Quit button or the
		OS's own window-close control. No-op here."""
		pass

	def relaunch(self):
		"""Relaunches this app with the exact same command line
		(sys.executable + sys.argv, so this works the same whether launched
		via `python -m ryzom_forgery.apps.object_editor`, a direct script
		path, or however ryztart itself invokes it -- sys.argv reflects the
		actual invocation regardless). Uses os.execv rather than
		spawning a subprocess and exiting this one separately -- execv
		replaces this process in place (same PID), so there is never a
		moment with two copies alive, and no way for a slow/failed shutdown
		of the old one to pile up extra processes. Used e.g. by the UI
		font/size Settings, which only take effect on a fresh font atlas
		build at startup -- see _load_ui_font().

		Named relaunch() rather than restart() deliberately -- ShowBase
		(our own base class) already defines its own restart(), called from
		inside ShowBase.__init__ itself to start the IGLOOP task. A same-named
		override here would shadow that and fire on every single app launch,
		mid-__init__, via os.execv -- an infinite same-PID re-exec loop
		disguised as "the app keeps restarting itself for no reason"."""
		os.execv(sys.executable, [sys.executable] + sys.argv)
		os.execv(sys.executable, [sys.executable] + sys.argv)

	def set_live_ui_scale_preview(self, candidate_scale):
		"""Live *text-only* DPI preview -- called every frame by
		WorkspaceSetupDialog.on_dpi_preview_changed (setup popup) or
		object_editor.py's own DPI control (Settings tab) while the user is
		dragging a candidate value, before it's actually saved/applied.

		Deliberately does NOT touch style.scale_all_sizes() here (paddings,
		spacing, button/scrollbar sizes stay exactly as they were set at
		startup until a relaunch()) -- only updates self._ui_scale, which
		draw_ui() reads fresh every frame for its push_font(font, size) call
		(ImGui 1.92+'s dynamic font sizing, re-rasterizes on demand, no
		atlas rebuild needed). Repeatedly rescaling the whole style live
		while dragging a slider crashed for real in earlier testing (
		ScaleAllSizes() floors several fields to whole pixels, and doing
		that many times in a row relative to an already-scaled style erodes
		them towards zero) -- text-only keeps the live preview useful
		without touching that fragile path at all."""
		self._ui_scale = candidate_scale

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

	def _load_ui_font(self):
		"""Loads the user's chosen UI font (Settings.ui_font_name/
		ui_font_size, see _AVAILABLE_FONTS above) as ImGui's default font,
		right before _load_icon_font() merges Font Awesome's glyphs into
		whichever font was most recently added to the atlas -- so the icons
		end up merged into this one rather than into ImGui's own built-in
		default. Falls back to _DEFAULT_FONT_NAME if the stored choice is
		stale (a font file imgui_bundle no longer ships, e.g. after an
		imgui_bundle upgrade)."""
		# self._ui_font_size_base: the unscaled point size draw_ui() combines
		# with the *current* self._ui_scale every frame (imgui's dynamic
		# font sizing, push_font(font, size) -- ImGui 1.92+, no atlas
		# rebuild needed) for a live preview while the DPI control is being
		# adjusted -- set unconditionally, before the early-return below,
		# since draw_ui() needs a valid value regardless of whether a
		# custom font file actually loaded.
		settings = app_settings.load()
		self._ui_font_size_base = settings.ui_font_size
		relative_path = _AVAILABLE_FONTS.get(settings.ui_font_name) or _AVAILABLE_FONTS[_DEFAULT_FONT_NAME]
		font_path = _FONTS_DIR / relative_path
		if not font_path.exists():
			return
		font_size = settings.ui_font_size * self._ui_scale
		font = imgui.get_io().fonts.add_font_from_file_ttf(str(font_path), font_size)
		imgui.get_io().font_default = font

	def _load_icon_font(self):
		# self.large_icon_font (1.5x the merged icon size below, standalone --
		# not merged into the default font like the one above) is for spots
		# that want a bigger icon-only button than the normal UI text size,
		# e.g. object_editor.py's viewport toggle bars: push_font()/pop_font()
		# around those buttons, since there's no per-window font-scale API in
		# this imgui_bundle version to reach for instead.
		# icon_font_size tracks the UI font's own size (self._ui_font_size_base,
		# set by _load_ui_font() right before this is called) rather than a
		# fixed point size: a merged icon glyph rasterized noticeably larger
		# than the surrounding text overflows the frame's line height (based
		# on the base font's own metrics), cropping into -- or entirely past
		# -- the frame's top padding. Worst at small ui_font_size values
		# (9-12) combined with a high ui_scale, where a fixed-size icon used
		# to be proportionally largest relative to the text (reported/
		# repro'd by Nuno, 2026-09-04).
		icon_font_size = self._ui_font_size_base * self._ui_scale
		if not _ICON_FONT_PATH.exists():
			self.large_icon_font = None
			self.large_icon_font_size = icon_font_size * 1.5
			return
		font_config = imgui.ImFontConfig()
		font_config.merge_mode = True
		imgui.get_io().fonts.add_font_from_file_ttf(str(_ICON_FONT_PATH), icon_font_size, font_config)
		self.large_icon_font_size = icon_font_size * 1.5
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

	def request_settings_attention(self, section, field_key, duration=3.0):
		"""Ask the Settings tab to select itself, expand `section` (a
		Settings collapsing_header's own label, e.g. "Tools"/"Paths"), and
		flash the field identified by `field_key` for `duration` seconds --
		the shared alternative to a disabled button+tooltip or a blocking
		popup when an action needs a Settings value that isn't configured
		yet (e.g. an external editor path, a repository path). Any Forgery
		app's UI code can call this from a button's click handler; a
		subclass wires _consume_settings_tab_flags()/
		_consume_settings_section_open()/_begin_attention_flash() into its
		own Settings tab drawing code to actually act on it (see
		object_editor.py's Settings tab for the reference wiring)."""
		self._settings_attention = (section, field_key, time.time() + duration)

	def _consume_settings_tab_flags(self):
		"""Call where the Settings tab item itself begins (whatever draws
		its imgui.begin_tab_item/begin_tab_item_simple) -- returns the
		TabItemFlags to pass so it's forced selected this frame if an
		attention request is still pending, 0 otherwise. Doesn't clear the
		request -- _consume_settings_section_open()/_begin_attention_flash()
		below still need it this same frame, once the tab's own content is
		what's actually being drawn."""
		if self._settings_attention is not None and self._settings_attention[2] > time.time():
			return imgui.TabItemFlags_.set_selected.value
		return 0

	def _consume_settings_section_open(self, section):
		"""Call right before drawing a Settings collapsing_header -- forces
		it open this frame if the pending attention request targets it."""
		if self._settings_attention is not None and self._settings_attention[0] == section:
			imgui.set_next_item_open(True)

	def _begin_attention_flash(self, field_key):
		"""Call right before drawing a specific field's own widget(s) --
		pushes a pulsing border color/thickness around it while an
		attention request targets `field_key`. Returns whether it pushed
		anything, for _end_attention_flash() to match (imgui's
		push/pop style calls must always balance). Clears the request once
		it expires, so the flash eventually stops on its own without
		needing a separate timeout mechanism."""
		if self._settings_attention is None:
			return False
		section, key, expires_at = self._settings_attention
		if key != field_key:
			return False
		if expires_at <= time.time():
			self._settings_attention = None
			return False
		pulse = (math.sin(time.time() * 6.0) + 1.0) / 2.0
		imgui.push_style_color(imgui.Col_.border.value, (1.0, 0.4 + pulse * 0.4, 0.0, 1.0))
		imgui.push_style_var(imgui.StyleVar_.frame_border_size.value, 2.0)
		return True

	def _end_attention_flash(self, active):
		if active:
			imgui.pop_style_var()
			imgui.pop_style_color()

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
		if self._splash is not None:
			self._splash.close()
			self._splash = None

		# Dynamic font sizing (imgui_bundle's push_font(font, size) --
		# ImGui 1.92+'s on-demand glyph rasterization, no atlas rebuild
		# needed): re-requests the already-loaded default font at the
		# *current* self._ui_scale every single frame, so a live DPI
		# preview (see set_live_ui_scale_preview()) shows genuinely crisp
		# text -- no FontGlobalScale-style softening, and no atlas rebuild
		# needed for this part even after the value is actually saved
		# (only a font *family* change still needs relaunch(), since a
		# different .ttf has to be loaded into the atlas first).
		imgui.push_font(None, self._ui_font_size_base * self._ui_scale)

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

		imgui.pop_font()
