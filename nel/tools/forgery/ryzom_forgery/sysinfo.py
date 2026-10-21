import importlib.metadata

from imgui_bundle import imgui


def _version_label():
	try:
		version = importlib.metadata.version("ryzom_forgery")
	except importlib.metadata.PackageNotFoundError:
		version = "dev"
	return f"Ryzom Forgery v{version} -- Ulukyn, Claude@anthropic"


_TASK_BAR_WIDTH = 220


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
		# {task_id: (label, fraction or None)}, insertion order = queue order
		# (project-todos/forgery/unified_background_task_status_bar.md) --
		# only the oldest still-active task is drawn, centered.
		self._tasks = {}
		# Computed once -- importlib.metadata.version() reads install
		# metadata off disk, and the running version can't change mid-session.
		self._version_label = _version_label()

	def set_status(self, text: str, color=None):
		self.status = text
		self.status_color = color

	def set_cursor_info(self, text: str):
		self.cursor_info = text

	def report_task(self, task_id: str, label: str, fraction=None):
		"""Called every frame a background task is active (main thread only,
		never from the worker thread). `fraction` is 0..1, or None when
		unknown: the label is then drawn alone, without a bar."""
		self._tasks[task_id] = (label, fraction)

	def clear_task(self, task_id: str):
		self._tasks.pop(task_id, None)

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

		if self._tasks:
			label, fraction = next(iter(self._tasks.values()))
			segment_width = imgui.calc_text_size(label).x
			if fraction is not None:
				segment_width += imgui.get_style().item_spacing.x + _TASK_BAR_WIDTH
			centered_x = (imgui.get_window_width() - segment_width) / 2
			# Never overlap what is already drawn on the left.
			imgui.same_line(max(centered_x, imgui.get_cursor_pos_x() + 20))
			imgui.text(label)
			if fraction is not None:
				imgui.same_line()
				imgui.progress_bar(max(0.0, min(1.0, fraction)), (_TASK_BAR_WIDTH, 0))

		label_width = imgui.calc_text_size(self._version_label).x
		imgui.same_line(imgui.get_window_width() - label_width - imgui.get_style().item_spacing.x)
		imgui.text_disabled(self._version_label)
