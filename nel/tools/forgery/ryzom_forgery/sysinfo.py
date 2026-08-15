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

	def set_status(self, text: str):
		self.status = text

	def draw(self):
		framerate = imgui.get_io().framerate
		frame_ms = (1000.0 / framerate) if framerate > 0 else 0.0
		imgui.text(f"{framerate:.1f} FPS ({frame_ms:.2f} ms)")

		if self.status:
			imgui.same_line(spacing=20)
			imgui.text(self.status)
