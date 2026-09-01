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
		# Computed once -- importlib.metadata.version() reads install
		# metadata off disk, and the running version can't change mid-session.
		self._version_label = _version_label()

	def set_status(self, text: str, color=None):
		self.status = text
		self.status_color = color

	def draw(self):
		framerate = imgui.get_io().framerate
		frame_ms = (1000.0 / framerate) if framerate > 0 else 0.0
		imgui.text(f"{framerate:.1f} FPS ({frame_ms:.2f} ms)")

		if self.status:
			imgui.same_line(spacing=20)
			if self.status_color is not None:
				imgui.text_colored(self.status_color, self.status)
			else:
				imgui.text(self.status)

		label_width = imgui.calc_text_size(self._version_label).x
		imgui.same_line(imgui.get_window_width() - label_width - imgui.get_style().item_spacing.x)
		imgui.text_disabled(self._version_label)
