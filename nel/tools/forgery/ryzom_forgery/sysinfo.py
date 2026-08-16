from imgui_bundle import imgui


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
