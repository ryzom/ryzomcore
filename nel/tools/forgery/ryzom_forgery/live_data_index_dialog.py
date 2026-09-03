"""Background rebuild + blocking progress popup for creature_full_index.py's
own on-disk cache -- same threading/progress-dict/modal pattern as
apps/object_editor_mixins/panoply_ui.py's `_start_panoply_bake`/
`_run_panoply_bake`/`_draw_bake_progress_popup`, applied to a sha1-staleness
check instead of a "just clicked Bake" trigger (see creature_full_index.py's
own module docstring for why sha1 here specifically).
"""

import threading

from imgui_bundle import imgui

from ryzom_forgery import creature_full_index
from ryzom_forgery.popup_utils import center_next_popup

_POPUP_ID = "Building creature/animation index"

# Exact `progress["stage"]` strings creature_full_index.build_index() sets,
# in emission order -- used only to turn "which stage are we in" into an
# overall progress fraction for the bar below. Keep in sync with
# build_index()'s own _stage(...) calls if that function's stage list ever
# changes.
_STAGE_ORDER = (
	"Reading creature.packed_sheets...", "Reading item.packed_sheets...", "Reading sitem.packed_sheets...",
	"Reading sheet_id.bin...", "Reading animset_list.packed_sheets...", "Reading mode2animset.string_array...",
	"Parsing creature.packed_sheets...", "Parsing item.packed_sheets...", "Parsing sitem.packed_sheets...",
	"Parsing sheet_id.bin...", "Building creature index...", "Inverting shape -> creature index...",
	"Resolving real animation lists...",
)


def _center_next_widget(width):
	"""Same centering helper as apps/object_editor_mixins/shape_io.py's own
	_center_next_widget() (not shared/exported, each popup module keeps its
	own tiny copy) -- shifts the cursor so a `width`-wide widget drawn right
	after this call lands horizontally centered in the popup."""
	avail = imgui.get_content_region_avail().x
	if avail > width:
		imgui.set_cursor_pos_x(imgui.get_cursor_pos_x() + (avail - width) / 2)


def _stage_fraction(progress) -> float:
	stage = progress.get("stage", "")
	try:
		i = _STAGE_ORDER.index(stage)
	except ValueError:
		return 0.0
	n = len(_STAGE_ORDER)
	step = 1.0 / n
	fraction = i * step
	if stage == "Building creature index..." and progress.get("index_total"):
		fraction += step * (progress["index_i"] / progress["index_total"])
	elif stage == "Inverting shape -> creature index..." and progress.get("invert_total"):
		fraction += step * (progress["invert_i"] / progress["invert_total"])
	return min(fraction, 1.0)


class LiveDataIndexDialog:
	def __init__(self):
		self._progress = None  # dict while a build is running/just finished, None otherwise
		self._checked_dir = None  # live_data_dir already is_stale()-checked, see ensure_built()

	def ensure_built(self, live_data_dir):
		"""Call once per frame (see object_editor.py's draw_ui()) once
		live_data_dialog.is_configured() -- starts a background rebuild the
		first time it sees `live_data_dir` if the on-disk index is missing
		or stale. is_stale() itself only hashes the source files (fast,
		no full packed_sheets parse), and is only actually run once per
		`live_data_dir` thanks to self._checked_dir, so calling this every
		frame is cheap."""
		if live_data_dir is None or self._progress is not None or self._checked_dir == live_data_dir:
			return
		self._checked_dir = live_data_dir
		if creature_full_index.is_stale(live_data_dir):
			self._start_build(live_data_dir)

	def force_rebuild(self, live_data_dir):
		"""Settings tab's refresh button (see live_data_setup_dialog.py's
		on_refresh_requested) -- rebuilds regardless of is_stale(), for
		whenever the user wants to be sure without waiting on a sha1
		mismatch (e.g. right after a manual game-data reinstall they know
		about). No-op while a build is already running."""
		if live_data_dir is None or self._progress is not None:
			return
		self._start_build(live_data_dir)

	def _start_build(self, live_data_dir):
		self._progress = {
			"stage": "Starting...", "index_i": 0, "index_total": 0, "invert_i": 0, "invert_total": 0,
			"done": False, "error": None,
		}
		thread = threading.Thread(target=self._run_build, args=(live_data_dir, self._progress), daemon=True)
		thread.start()

	def _run_build(self, live_data_dir, progress):
		"""Background-thread body -- never touches imgui, only `progress`
		(plain dict field writes, same safe-under-the-GIL reasoning as
		panoply_ui.py's _bake_progress) and creature_full_index's own pure
		file I/O + pynel parsing."""
		try:
			creature_full_index.build_and_save(live_data_dir, progress)
			creature_full_index.invalidate_loaded_index()
		except Exception as exc:
			progress["error"] = str(exc)
		finally:
			progress["done"] = True

	def draw(self):
		"""Call once per frame regardless of whether a build is running --
		no-ops until _start_build() sets self._progress."""
		progress = self._progress
		if progress is None:
			return
		if not imgui.is_popup_open(_POPUP_ID):
			imgui.open_popup(_POPUP_ID)
		# always=True: unlike panoply_ui.py's bake popup (always user-triggered
		# mid-session), this one can open on essentially the very first ImGui
		# frame at launch -- same "Panda3D window geometry not settled yet"
		# case workspace_setup_dialog.py's own mandatory popup documents.
		center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_POPUP_ID, None, flags)
		if not opened:
			return

		if progress["error"] is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), f"Build failed: {progress['error']}")
			_center_next_widget(imgui.calc_text_size("OK").x + imgui.get_style().frame_padding.x * 2)
			if imgui.button("OK"):
				self._progress = None
				# Retried on the very next frame's ensure_built() call --
				# self._checked_dir still points at this same dir, so force
				# a re-check by clearing it too, rather than being stuck
				# thinking it's already "checked" (and therefore skipped)
				# forever after a failed attempt.
				self._checked_dir = None
				imgui.close_current_popup()
			imgui.end_popup()
			return

		imgui.text_wrapped(
			"Ryzom's game data changed (or this is the first launch) -- "
			"building the real skeleton/animation index. This only happens "
			"when the source files actually change.")
		imgui.text(progress["stage"])
		imgui.progress_bar(1.0 if progress["done"] else _stage_fraction(progress), (320, 0))
		if progress["done"]:
			_center_next_widget(imgui.calc_text_size("OK").x + imgui.get_style().frame_padding.x * 2)
			if imgui.button("OK"):
				self._progress = None
		imgui.end_popup()
