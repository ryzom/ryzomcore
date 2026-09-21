import importlib.metadata
from typing import Optional

from imgui_bundle import imgui

from .ui.box import Box, STRETCH
from .ui.box_panel import LabelWidget, ProgressBarWidget

_TASK_BAR_WIDTH = 220


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
		# {task_id: (label, fraction)} for every background task currently
		# reporting itself -- insertion order IS queue order (a plain dict
		# already preserves it), so the oldest still-active task is always
		# whichever one was registered first. One shared queue for every
		# background task across the whole app (project-todos/forgery/
		# unified_background_task_status_bar.md) instead of each task
		# inventing its own popup/panel text/nothing -- see report_task()/
		# clear_task().
		self._tasks = {}
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

	def report_task(self, task_id: str, label: str, fraction: Optional[float]):
		"""Call every frame a background task is active -- from whichever
		main-thread draw()/poll method already reads that task's own
		progress state (never from the background thread itself, same rule
		every such thread in this codebase already follows for imgui/
		sysinfo calls). `fraction` in [0, 1], or None if genuinely unknown
		(shown as a label with no bar, no fake indeterminate animation)."""
		if task_id not in self._tasks:
			print(f"(IA_AGENT_DEBUG) (background_task_bar) (sysinfo.py:report_task) registered task_id={task_id!r} label={label!r}")
		self._tasks[task_id] = (label, fraction)

	def clear_task(self, task_id: str):
		"""Call once a background task is done (success or error) -- no-op
		if `task_id` isn't currently tracked."""
		if task_id in self._tasks:
			print(f"(IA_AGENT_DEBUG) (background_task_bar) (sysinfo.py:clear_task) cleared task_id={task_id!r}")
		self._tasks.pop(task_id, None)

	def build_content_box(self):
		"""Rebuilt fresh every frame (box_ui_migration__sysinfo.md) -- this
		bar's content changes frame to frame (FPS, optional cursor/status,
		the oldest active task), so a Box tree built once wouldn't reflect
		it; Box/Widget objects are cheap plain-Python objects meant to be
		thrown away and rebuilt, same as ImGui's own immediate-mode model.
		Returns a Box (never draws anything itself -- that happens when the
		caller lays this tree out) with every visible segment as a real
		LabelWidget/ProgressBarWidget child, `align="center"` centering each
		one vertically in the row (replaces the old manual `center_v()`
		calls -- see `docs/ui.md`'s Historique/pièges on why a real Box tree
		is required here instead of a single opaque draw() callback)."""
		row = Box(h=STRETCH, v=STRETCH)
		line_height = imgui.get_text_line_height()
		item_spacing = imgui.get_style().item_spacing.x

		framerate = imgui.get_io().framerate
		frame_ms = (1000.0 / framerate) if framerate > 0 else 0.0
		fps_text = f"{framerate:.1f} FPS ({frame_ms:.2f} ms)"
		# margin=0 on every LabelWidget/ProgressBarWidget here: each is
		# already sized to an EXACT pixel width via imgui.calc_text_size()
		# (no allowance for margin) -- Widget's own default margin=2 would
		# shrink their content area past that exact size (ui/box_panel.py,
		# box_ui_migration__explorer_panel.md, Nuno 2026-09-20).
		row.add(LabelWidget(fps_text, dock="left", align="center", margin=0.0,
		                     h=imgui.calc_text_size(fps_text).x, v=line_height))

		if self.cursor_info:
			row.add(Box(dock="left", h=20))
			row.add(LabelWidget(self.cursor_info, dock="left", align="center", margin=0.0,
			                     h=imgui.calc_text_size(self.cursor_info).x, v=line_height))

		if self.status:
			row.add(Box(dock="left", h=20))
			row.add(LabelWidget(self.status, dock="left", align="center", color=self.status_color, margin=0.0,
			                     h=imgui.calc_text_size(self.status).x, v=line_height))

		if self._tasks:
			label, fraction = next(iter(self._tasks.values()))
			label_width = imgui.calc_text_size(label).x
			segment_width = label_width if fraction is None else label_width + item_spacing + _TASK_BAR_WIDTH
			segment = Box(dock="center", h=segment_width, v=STRETCH)
			segment.add(LabelWidget(label, dock="left", align="center", margin=0.0, h=label_width, v=line_height))
			if fraction is not None:
				segment.add(Box(dock="left", h=item_spacing))
				segment.add(ProgressBarWidget(fraction, dock="left", align="center", margin=0.0,
				                               h=_TASK_BAR_WIDTH, v=line_height))
			row.add(segment)

		# A `Percent`/fixed size on the version label's own width is measured
		# the same way the old code did (imgui.calc_text_size) -- the box
		# system has no built-in "size to my own text" mechanism, ImGui does.
		label_width = imgui.calc_text_size(self._version_label).x
		row.add(LabelWidget(self._version_label, dock="right", align="center", disabled=True, margin=0.0,
		                     h=label_width, v=line_height))
		return row
