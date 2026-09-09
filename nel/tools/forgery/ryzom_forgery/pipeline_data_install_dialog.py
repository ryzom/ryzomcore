"""Popup proposing to download/install missing pipeline data (see
pipeline_data_installer.py) -- "1 or 2 elements missing (continent +
ecosystem), download and install?", then a shared progress bar while
`pipeline_data_installer.download_and_install()` runs each item in turn in
a background thread. Same dict-polling pattern as `landscape_editor.py`'s
`_load_continent`/`_apply_render_mode` (see their own docstrings for why:
plain dict field writes from a background thread are safe under the GIL).

Usage:
	from ryzom_forgery.pipeline_data_install_dialog import PipelineDataInstallDialog

	dialog = PipelineDataInstallDialog()
	...
	dialog.open([("pipeline_continents", "nexus"), ("pipeline_ecosystems", "jungle")])
	...
	# once per frame:
	dialog.draw()
"""

import threading
from typing import List, Optional, Tuple

from imgui_bundle import imgui

from ryzom_forgery import pipeline_data_installer as pdi
from ryzom_forgery.popup_utils import center_next_popup

_POPUP_ID = "Pipeline data missing"

_CATEGORY_LABELS = {
	"landscape": "landscape",
	"pipeline_ecosystems": "écosystème",
	"pipeline_continents": "continent",
}


def _item_label(category: str, name: str) -> str:
	return f"{_CATEGORY_LABELS.get(category, category)} {name}"


class PipelineDataInstallDialog:
	def __init__(self):
		self._items: List[Tuple[str, str]] = []
		# Refused (category, name) sets, remembered in memory only for the
		# rest of this session -- never proposed again for the exact same
		# set until Atyscape is relaunched (project-todos/forgery/
		# landscape_editor__zone_render_modes__pipeline_data_installer.md
		# step 4).
		self._declined_sets = set()
		self._progress: Optional[dict] = None
		self._error: Optional[str] = None

	def open(self, items: List[Tuple[str, str]]) -> None:
		"""Opens the popup for `items` (1 or 2 `(category, name)` pairs), or
		does nothing if `items` is empty, a download is already running, or
		this exact set was already declined this session."""
		if not items or self._progress is not None:
			return
		key = frozenset(items)
		if key in self._declined_sets:
			return
		self._items = items
		imgui.open_popup(_POPUP_ID)

	def draw(self) -> None:
		"""Call once per frame from the host app's draw_panel()."""
		if imgui.is_popup_open(_POPUP_ID):
			center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_POPUP_ID, None, flags)
		if not opened:
			return

		if self._progress is not None:
			self._draw_progress()
		elif self._error is not None:
			self._draw_error()
		else:
			self._draw_prompt()

		imgui.end_popup()

	def _draw_prompt(self) -> None:
		labels = ", ".join(_item_label(c, n) for c, n in self._items)
		# Explicit wrap width: this popup has no other wide content (unlike
		# e.g. live_data_setup_dialog.py's path row) to anchor
		# always_auto_resize's width, so plain text_wrapped() (wrap width
		# implicitly following the window's own, not-yet-settled width)
		# collapses to a tiny/squashed column -- found 2026-09-09, Nuno.
		imgui.push_text_wrap_pos(imgui.get_font_size() * 28)
		imgui.text_wrapped(f"Éléments manquants : {labels}. Télécharger et installer ?")
		imgui.pop_text_wrap_pos()
		imgui.spacing()
		if imgui.button("Oui"):
			self._start_download()
		imgui.same_line()
		if imgui.button("Non"):
			self._declined_sets.add(frozenset(self._items))
			imgui.close_current_popup()

	def _start_download(self) -> None:
		progress = {
			"done": False, "error": None,
			"current_index": 0, "total": len(self._items), "current_label": "",
			"phase": None, "downloaded_bytes": 0, "total_bytes": None,
		}
		self._progress = progress
		thread = threading.Thread(target=self._run_download, args=(self._items, progress), daemon=True)
		thread.start()

	def _run_download(self, items: List[Tuple[str, str]], progress: dict) -> None:
		try:
			for i, (category, name) in enumerate(items):
				progress["current_index"] = i
				progress["current_label"] = _item_label(category, name)
				pdi.download_and_install(category, name, progress)
		except pdi.PipelineDataInstallError as exc:
			progress["error"] = str(exc)
		finally:
			progress["done"] = True

	def _draw_error(self) -> None:
		imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._error)
		if imgui.button("Fermer"):
			self._error = None
			imgui.close_current_popup()

	def _draw_progress(self) -> None:
		progress = self._progress
		if progress["done"]:
			self._progress = None
			if progress["error"]:
				self._error = progress["error"]
			else:
				imgui.close_current_popup()
			return

		total_items = progress["total"]
		overlay = f"{progress['current_index'] + 1}/{total_items} : {progress['current_label']}"
		imgui.text(overlay)

		downloaded = progress["downloaded_bytes"]
		total_bytes = progress["total_bytes"]
		if progress["phase"] == "extracting":
			imgui.progress_bar(1.0, overlay="Extraction...")
		elif total_bytes:
			imgui.progress_bar(downloaded / total_bytes, overlay=f"{downloaded // 1024} / {total_bytes // 1024} Ko")
		else:
			imgui.progress_bar(0.0, overlay=f"{downloaded // 1024} Ko téléchargés...")
