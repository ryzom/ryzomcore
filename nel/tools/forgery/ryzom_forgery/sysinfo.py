import importlib.metadata

from imgui_bundle import imgui


def _version_label():
	try:
		version = importlib.metadata.version("ryzom_forgery")
	except importlib.metadata.PackageNotFoundError:
		version = "dev"
	return f"Ryzom Forgery v{version} -- Ulukyn, Claude@anthropic"


class SysInfoBar:
	"""Standard bottom status bar for Ryzom Forgery tool apps.

	Shows engine perf stats (FPS, frame time) plus a status line any part
	of the app (the explorer, the tool panel, ...) can update via
	set_status(), e.g. the path of the hovered/selected file or the result
	of the last action.
	"""

	def __init__(self):
		self.status = ""
		self.status_color = None  # (r, g, b, a) or None for the default text color
		# Separate from `status` (project-todos/forgery/
		# landscape_editor__cursor_zone_status.md step 3) -- `status` is
		# already used by ForgeryApp's own Explorer selection display
		# (app.py, "N selected"/hovered path), so reusing it for a per-frame
		# cursor readout would fight with that instead of coexisting with
		# it. Drawn right after FPS, `status` (if any) drawn after this.
		self.cursor_info = ""
		# Computed once -- importlib.metadata.version() reads install
		# metadata off disk, and the running version can't change mid-session.
		self._version_label = _version_label()

	def set_status(self, text: str, color=None):
		self.status = text
		self.status_color = color

	def set_cursor_info(self, text: str):
		self.cursor_info = text

	def draw(self):
		framerate = imgui.get_io().framerate
		frame_ms = (1000.0 / framerate) if framerate > 0 else 0.0
		imgui.text(f"{framerate:.1f} FPS ({frame_ms:.2f} ms)")

		if self.cursor_info:
			imgui.same_line(spacing=20)
			imgui.text(self.cursor_info)

		if self.status:
			imgui.same_line(spacing=20)
			if self.status_color is not None:
				imgui.text_colored(self.status_color, self.status)
			else:
				imgui.text(self.status)

		label_width = imgui.calc_text_size(self._version_label).x
		imgui.same_line(imgui.get_window_width() - label_width - imgui.get_style().item_spacing.x)
		imgui.text_disabled(self._version_label)
