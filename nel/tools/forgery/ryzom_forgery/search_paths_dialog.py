"""Settings UI ("Paths" section) + scan logic for Forgery's generic search
paths: user-configured, priority-ordered folders (recursive or not,
.bnp-aware, see search_paths.py) that Forgery resolves `.shape`/.skel/.anim/
texture files from. Finds .skel files compatible with the loaded shape's own
skinning bones (CSkeletonModel::remapSkinBones-equivalent matching), .anim
files compatible with a given skeleton, resolves material texture references
(see shape_geometry.py's load_panda_texture()), and picks up Ryzom's
"panoply_files.txt" (see panoply.py) if one is found among them.

The active workspace's own folder (see workspaces.py) is injected ahead of
these user-configured folders -- always highest priority, never persisted
into `settings.search_paths` itself -- via set_workspace_dir(), kept in sync
by the host app whenever the active workspace changes (see
object_editor.py's wiring of WorkspaceSetupDialog.on_active_workspace_changed).
"""

import json
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, portable_file_dialogs as pfd
from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from pynel import repository_paths
from pynel.ryzom_animation import AnimationParseError, parse_animation
from pynel.ryzom_shape import ShapeParseError, SkeletonShape, parse_shape

from ryzom_forgery import panoply, search_paths, virtual_categories
from ryzom_forgery import settings as app_settings
from ryzom_forgery.icon_colors import pastel_color_for
from ryzom_forgery.settings import SearchPathDir

_PANOPLY_FILE_NAME = "panoply_files.txt"
# A real data tree scan (see _scan_dirs_incremental()) builds on the order of
# 10^5 plain Python objects/dict entries -- real CPU-bound work in the
# interpreter, not I/O. Running that on a background thread meant it
# competed with the render thread for the GIL badly enough to drag frame
# rate well under 30fps (only ever noticed once ensure_scanned()'s cache-hit
# path made the app interactive *during* a scan, instead of blocking behind
# a popup for its whole duration -- a shorter sys.setswitchinterval() helped
# but wasn't enough on its own). Scanning a small time-bounded slice per
# frame on the main thread instead removes the second thread entirely, so
# there's nothing left to contend with -- the scan itself just takes longer
# in wall-clock time (only ~this many ms of every ~16ms frame budget, shared
# with rendering instead of running flat-out alone), which is the trade-off
# asked for: never block the UI, however long the scan takes.
_SCAN_FRAME_TIME_BUDGET = 0.002
# Bulk operations (extracting a .bnp, a git checkout, a texture batch export
# from another tool...) fire dozens of individual FS events in a burst --
# without debouncing, each one would kick off its own full rescan.
_WATCH_DEBOUNCE_SECONDS = 0.5
# watchdog also reports pure-read access ('opened'/'closed_no_write', e.g.
# from this app's own _load_shape() just reading a .shape file that happens
# to live under the watched workspace) -- without filtering these out, every
# shape load anywhere inside the workspace triggered a full rescan. Only
# actual content changes should count (same filtering as
# workspace_watch.py's own _DebouncedHandler).
_RELEVANT_EVENT_TYPES = {"created", "deleted", "modified", "moved"}
# Same on/off icon-color-toggle pattern as explorer.py's favorite star
# (_FAVORITE_STAR_COLOR/_NON_FAVORITE_STAR_COLOR) -- white/orange
# instead of a checkbox, to save row width.
_RECURSIVE_ON_COLOR = (1.0, 0.6, 0.0, 1.0)
_RECURSIVE_OFF_COLOR = (1.0, 1.0, 1.0, 1.0)


def _make_workspace_exclude(workspace_root: Path, exclusion_rules):
	"""Builds the `exclude` predicate _reload_workspace_only() passes to
	_scan_dirs_incremental()/search_paths.iter_all_entries() -- an excluded
	workspace file (folder or file-pattern rule, see
	virtual_categories.is_path_excluded()) is skipped entirely from the
	texture/.skel/.anim index, same "never taken into account in search"
	rule as the virtual Explorer categories use for the search side of
	things (unlike their own display bucketing, where a file-pattern match
	still shows, just in "others" -- irrelevant here, this predicate only
	ever gates indexing)."""
	def exclude(found) -> bool:
		fs_path = found.bnp_path if found.fs_path is None else found.fs_path
		try:
			relative_dir = fs_path.relative_to(workspace_root).parent
		except ValueError:
			return False
		return virtual_categories.is_path_excluded(relative_dir, found.name, exclusion_rules)
	return exclude


def _animation_bone_names(anim):
	"""The set of bone names an Animation actually has tracks for --
	pynel's Animation has no such field directly, only `id_by_name`, a dict
	of track keys shaped like "{bone_name}.pos"/".rotquat"/".scale" (see
	pynel.ryzom_animation's own docstring)."""
	names = set()
	for key in anim.id_by_name:
		bone_name, separator, _ = key.rpartition(".")
		if separator:
			names.add(bone_name)
	return names


def _icon_button(icon, tooltip, disabled=False, color=None):
	"""Same minimal icon-button-with-tooltip pattern as explorer.py's own
	_icon_button() -- each module keeps its own tiny copy rather than
	sharing one, matching how this codebase already does it (see also
	object_editor.py's own, more featureful version). Tinted a deterministic
	pastel color (see icon_colors.py) unless `color` is given -- needed by
	the Recursive toggle's own on/off color, which a pastel tint would
	otherwise silently override."""
	imgui.push_style_color(imgui.Col_.text.value, color if color is not None else pastel_color_for(icon))
	imgui.begin_disabled(disabled)
	clicked = imgui.button(icon)
	imgui.end_disabled()
	imgui.pop_style_color()
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked

# Long paths would otherwise push the Recursive/Remove buttons off the
# Settings panel -- ImGui doesn't wrap/reflow a same_line() row on its own,
# it just keeps extending past the window's edge, so the path text has to be
# shrunk to fit whatever pixel width is actually left over, not a fixed
# character count (a 40-char budget still overflows a narrow panel, and
# wastes space in a wide one). Truncated from the front (the tail of a path
# is usually the meaningful part, e.g. ".../data/textures"), full path
# always available as a tooltip on hover.
_ELLIPSIS = "..."


def _truncate_path_to_width(path, max_width):
	if imgui.calc_text_size(path).x <= max_width:
		return path
	if imgui.calc_text_size(_ELLIPSIS).x > max_width:
		return _ELLIPSIS

	# Binary search for the longest tail of `path` that still fits alongside
	# the ellipsis -- calc_text_size() isn't linear in character count
	# (proportional font), so this can't be computed directly.
	lo, hi, best = 0, len(path), _ELLIPSIS
	while lo <= hi:
		mid = (lo + hi) // 2
		candidate = _ELLIPSIS + path[-mid:] if mid > 0 else _ELLIPSIS
		if imgui.calc_text_size(candidate).x <= max_width:
			best = candidate
			lo = mid + 1
		else:
			hi = mid - 1
	return best


class _DebouncedReloadHandler(FileSystemEventHandler):
	"""Coalesces a burst of *content-changing* filesystem events (see
	_RELEVANT_EVENT_TYPES -- pure-read access is ignored) into a single
	`on_settled` call, fired _WATCH_DEBOUNCE_SECONDS after the last event --
	watchdog calls on_any_event() from its own OS-native watcher thread,
	never the main/render thread, same as any other background worker
	here."""

	def __init__(self, on_settled):
		self._on_settled = on_settled
		self._timer = None
		self._lock = threading.Lock()

	def on_any_event(self, event):
		if event.event_type not in _RELEVANT_EVENT_TYPES:
			return
		with self._lock:
			if self._timer is not None:
				self._timer.cancel()
			self._timer = threading.Timer(_WATCH_DEBOUNCE_SECONDS, self._on_settled)
			self._timer.daemon = True
			self._timer.start()

	def cancel(self):
		with self._lock:
			if self._timer is not None:
				self._timer.cancel()
				self._timer = None


@dataclass
class _ScanResult:
	"""One `_scan_dirs_incremental()` call's worth of freshly built entries -- kept
	separate per source (external search paths vs. the active workspace, see
	SearchPathsDialog._external_result/_workspace_result) so a workspace-only
	rescan (see _reload_workspace_only()) can refresh just its own half and
	re-merge, without re-walking (or discarding the results of) the external
	paths, which the workspace watcher could never have any reason to
	invalidate in the first place."""
	skeleton_entries: dict = field(default_factory=dict)
	skeleton_bones: dict = field(default_factory=dict)
	animation_entries: dict = field(default_factory=dict)
	animation_bones: dict = field(default_factory=dict)
	texture_entries: dict = field(default_factory=dict)
	panoply_variants: dict = field(default_factory=dict)
	total: int = 0


_EXTERNAL_INDEX_CACHE_FILE_NAME = "external_scan_index_cache.json"


def _serialize_found_entry(found: search_paths.FoundEntry) -> dict:
	return {
		"name": found.name,
		"fs_path": str(found.fs_path) if found.fs_path is not None else None,
		"bnp_path": str(found.bnp_path) if found.bnp_path is not None else None,
	}


def _deserialize_found_entry(data: dict) -> search_paths.FoundEntry:
	return search_paths.FoundEntry(
		name=data["name"],
		fs_path=Path(data["fs_path"]) if data["fs_path"] is not None else None,
		bnp_path=Path(data["bnp_path"]) if data["bnp_path"] is not None else None,
	)


def _serialize_scan_result(result: _ScanResult) -> dict:
	return {
		"skeleton_entries": {name: _serialize_found_entry(e) for name, e in result.skeleton_entries.items()},
		"skeleton_bones": {name: sorted(bones) for name, bones in result.skeleton_bones.items()},
		"animation_entries": {name: _serialize_found_entry(e) for name, e in result.animation_entries.items()},
		"animation_bones": {name: sorted(bones) for name, bones in result.animation_bones.items()},
		"texture_entries": {name: _serialize_found_entry(e) for name, e in result.texture_entries.items()},
		"panoply_variants": result.panoply_variants,
		"total": result.total,
	}


def _deserialize_scan_result(data: dict) -> _ScanResult:
	return _ScanResult(
		skeleton_entries={name: _deserialize_found_entry(e) for name, e in data["skeleton_entries"].items()},
		skeleton_bones={name: frozenset(bones) for name, bones in data["skeleton_bones"].items()},
		animation_entries={name: _deserialize_found_entry(e) for name, e in data["animation_entries"].items()},
		animation_bones={name: frozenset(bones) for name, bones in data["animation_bones"].items()},
		texture_entries={name: _deserialize_found_entry(e) for name, e in data["texture_entries"].items()},
		panoply_variants=data["panoply_variants"],
		total=data["total"],
	)


def _load_external_index_cache() -> "_ScanResult | None":
	"""The last full reload()'s external-search-paths _ScanResult, persisted
	to disk -- lets ensure_scanned() show real (if possibly slightly stale)
	data immediately at startup instead of a blank index until the real
	(directory-walk-bound, potentially slow on a large data tree) scan
	finishes. None if there's no cache yet (e.g. first run ever) or it
	failed to parse -- the caller falls back to waiting on the real scan,
	same as before this cache existed."""
	path = search_paths.cache_dir() / _EXTERNAL_INDEX_CACHE_FILE_NAME
	try:
		return _deserialize_scan_result(json.loads(path.read_text()))
	except (OSError, ValueError, KeyError):
		return None


def _save_external_index_cache(result: _ScanResult) -> None:
	directory = search_paths.cache_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _EXTERNAL_INDEX_CACHE_FILE_NAME).write_text(json.dumps(_serialize_scan_result(result)))


class SearchPathsDialog:
	def __init__(self):
		self._dirs = app_settings.load().search_paths
		self._add_dir_dialog = None

		# The active workspace's own folder, always searched first -- see
		# set_workspace_dir(); kept separate from self._dirs (recursive,
		# never persisted, never shown/edited alongside the user's own
		# entries in draw_settings_content() beyond a read-only mention).
		self._workspace_dir = None
		# Watches _workspace_dir on disk so external changes (another tool
		# writing a shape, a git checkout, a manual copy...) get picked up
		# without the user having to hit Reload -- (re)started in
		# set_workspace_dir(), never for the user-configured self._dirs
		# (those are covered by the manual Reload button, same as before).
		self._workspace_observer = None

		# Built by reload()/_reload_workspace_only(), advanced a time-bounded
		# slice per frame on the main thread (see _advance_external_scan()/
		# _advance_workspace_scan(), called from draw()) -- kept as two
		# separate _ScanResult halves (see its own docstring) so a
		# workspace-only rescan never has to re-walk (or throw away) the
		# external one, merged into the public dicts below via
		# _merge_and_publish(), each swapped in as a whole fresh dict, never
		# mutated in place, so a frame reading them mid-scan just sees the
		# previous complete result, never a torn one.
		self._external_result = _ScanResult()
		self._workspace_result = _ScanResult()
		# In-flight _scan_dirs_incremental() generator + its own
		# (bnp_table_cache, cache) pair (held across the whole scan, saved
		# once it's exhausted) -- None when no scan of that half is running.
		self._external_scan_gen = None
		self._external_scan_caches = None
		self._external_scan_start_time = 0.0
		self._workspace_scan_gen = None
		self._workspace_scan_caches = None
		self._skeleton_entries = {}  # {name: search_paths.FoundEntry}
		self._skeleton_bones = {}  # {name: frozenset(bone names)} -- for compatible_for(), no full parse needed
		self._animation_entries = {}  # {name: search_paths.FoundEntry}
		self._animation_bones = {}  # {name: frozenset(bone names)} -- for compatible_animations_for()
		self._texture_entries = {}  # {name.lower(): search_paths.FoundEntry}
		self._panoply_variants = {}  # {base texture stem: {race: [user color, ...]}}, see panoply.py
		self._ryzom_data_panoply_variants = {}  # see _load_ryzom_data_panoply_variants()
		self._ryzom_data_panoply_mtime = None
		self._scan_status = ""
		# Two independent flags, not one shared -- at startup, set_workspace_dir()'s
		# own workspace-only scan and ensure_scanned()'s full reload() must be
		# able to advance concurrently (interleaved frame-by-frame, see
		# _advance_external_scan()/_advance_workspace_scan()) without either
		# blocking the other via a shared guard (reload() would otherwise
		# silently no-op if it happened to fire while the workspace-only scan
		# was still in flight). scanning (the public property) is true if
		# either is busy.
		self._scanning_external = False
		self._scanning_workspace = False
		self._scanned_once = False

		# Optional callback(), fired alongside reload() whenever the workspace
		# watch above settles -- the host app's hook for anything else that
		# should also react to the workspace changing on disk (e.g.
		# object_editor.py's Wexplorer, via Explorer.refresh()), without this
		# generic dialog needing to know what that is.
		self.on_workspace_changed = None

	@property
	def scanning(self):
		return self._scanning_external or self._scanning_workspace

	@property
	def has_scanned_data(self):
		"""True once the texture index has *any* entries -- distinguishes a
		cache-hit startup (real data published immediately, see
		ensure_scanned()) from a genuinely cold one (nothing to resolve
		against yet even though a scan is `scanning`). Used by
		object_editor.py's restore-scan popup to skip waiting on the
		background refresh scan when cached data already covers it."""
		return bool(self._texture_entries)

	def draw(self):
		"""Call once per ImGui frame, alongside the other always-polled
		dialogs (see object_editor.py's draw_panel())."""
		self._poll_add_dir_dialog()
		self._advance_external_scan()
		self._advance_workspace_scan()

	def _poll_add_dir_dialog(self):
		if self._add_dir_dialog is None or not self._add_dir_dialog.ready(0):
			return
		result = self._add_dir_dialog.result()
		self._add_dir_dialog = None
		if result and not any(entry.path == result for entry in self._dirs):
			self._dirs.append(SearchPathDir(path=result))
			self._save()

	def _save(self):
		"""Re-loads the shared settings file fresh and overwrites only the
		`search_paths` section with our own current state -- see
		export_dialog.py's own _save() for why (other components persist
		their own section independently)."""
		fresh = app_settings.load()
		fresh.search_paths = self._dirs
		app_settings.save(fresh)

	def set_workspace_dir(self, path):
		"""Injects `path` (the active workspace's own folder, or None) ahead
		of the user-configured search_paths -- called by the host app
		whenever the active workspace changes (see object_editor.py). Only
		ever touches the workspace half (_reload_workspace_only(), or an
		immediate clear+republish if `path` is None -- nothing to scan):
		switching workspaces has no bearing on the external search paths, so
		there's no reason to also rescan (or even glance at) those -- this
		used to call the same full reload() as the manual Reload button,
		needlessly re-walking the external paths on every switch.
		Also (re)starts a filesystem watcher on the new folder -- see
		_start_workspace_watch()."""
		new_dir = SearchPathDir(path=str(path), recursive=True) if path is not None else None
		if new_dir == self._workspace_dir:
			return
		self._workspace_dir = new_dir
		self._start_workspace_watch(path)
		if self._workspace_dir is None:
			self._workspace_result = _ScanResult()
			self._merge_and_publish()
		else:
			self._reload_workspace_only()

	def _start_workspace_watch(self, path):
		"""Stops any previous watch and, if `path` is set, starts a fresh
		one -- native OS APIs via watchdog (inotify/FSEvents/
		ReadDirectoryChangesW, with a polling fallback where none of those
		are available), the most robust cross-platform option. Silently
		gives up on a watch failure (e.g. an exotic filesystem inotify
		can't watch) -- external changes just won't auto-refresh then, no
		worse than before this existed, and not worth crashing over."""
		if self._workspace_observer is not None:
			self._workspace_observer.stop()
			self._workspace_observer = None
		if path is None:
			return
		handler = _DebouncedReloadHandler(self._on_workspace_settled)
		observer = Observer()
		try:
			observer.schedule(handler, str(path), recursive=True)
			observer.daemon = True
			observer.start()
		except OSError as e:
			print(f"[search_paths_dialog] could not watch workspace folder {path!r}: {e}")
			return
		self._workspace_observer = observer

	def _on_workspace_settled(self):
		"""_DebouncedReloadHandler's callback for the workspace watch -- only
		the workspace itself could have changed (that's the one and only
		folder this watch covers), so a full reload() re-walking every
		external search path too would be pure waste (see
		_reload_workspace_only()); then the host app's own extra hook, if
		any (see on_workspace_changed)."""
		self._reload_workspace_only()
		if self.on_workspace_changed is not None:
			self.on_workspace_changed()

	def ensure_scanned(self):
		"""Kicks off a first scan the first time it's needed (see
		object_editor.py's _display_shape()) -- so a freshly-opened Skinning
		preview already has a usable Skeleton/Animation list without the
		user having to press Reload themselves first. If a previous full
		reload() persisted an external-search-paths index (see
		_load_external_index_cache()), that's loaded and published
		immediately -- real data (if possibly slightly stale) rather than a
		blank index, with no directory walk, so this never blocks on a
		real data tree's own scan time. Either way, a real reload() also
		always runs right after, in the background: a cache hit is purely
		about not blocking the UI at startup, not a substitute for the real
		scan -- whatever actually changed since the cache was saved
		self-corrects a moment later, with no manual Reload needed."""
		if self._scanned_once:
			return
		self._scanned_once = True
		cached = _load_external_index_cache()
		if cached is not None:
			self._external_result = cached
			self._merge_and_publish()
			self._scan_status = f"Loaded cached index ({cached.total} asset(s)) -- refreshing in background..."
		self.reload()

	def reload(self):
		"""(Re)scans just the external search paths -- see
		_scan_dirs_incremental()/_advance_external_scan() (a small
		time-bounded slice per frame, driven from draw(), not a background
		thread -- see the module-level note on why). Persists the result
		when done (see _load_external_index_cache()/ensure_scanned()). A
		no-op while a scan is already running rather than restarting from
		scratch; the icon button is also disabled meanwhile (see
		draw_settings_content()/object_editor.py's Skinning preview). For a
		workspace-only change, see the far cheaper _reload_workspace_only()
		instead (used by the workspace watcher and set_workspace_dir() --
		the external paths never need rescanning just because the active
		workspace changed or a file changed inside it)."""
		if self._scanning_external:
			return
		self._scanning_external = True
		self._scan_status = "Scanning..."
		bnp_table_cache = search_paths.load_bnp_table_cache()
		cache = search_paths.load_scan_cache()
		self._external_scan_caches = (bnp_table_cache, cache)
		self._external_scan_gen = self._scan_dirs_incremental(self._dirs, bnp_table_cache, cache)
		self._external_scan_start_time = time.monotonic()

	def _advance_external_scan(self):
		"""Called once per frame (see draw()): pumps the in-flight external
		scan generator (if any) for up to _SCAN_FRAME_TIME_BUDGET, then
		yields back to the frame -- see reload(). Once the generator raises
		StopIteration (its return value is the finished _ScanResult, same
		as the old worker functions' local variable), persists+merges
		exactly like the old background-thread version did at the end."""
		if self._external_scan_gen is None:
			return
		slice_start = time.monotonic()
		deadline = slice_start + _SCAN_FRAME_TIME_BUDGET
		try:
			while time.monotonic() < deadline:
				next(self._external_scan_gen)
		except StopIteration as stop:
			self._external_result = stop.value
			bnp_table_cache, cache = self._external_scan_caches
			# Persisting the caches (mainly _save_external_index_cache(), which
			# serializes on the order of 10^5 entries to JSON) is itself
			# CPU-bound work on the order of a second -- doing it inline here
			# would re-introduce the exact kind of main-thread freeze the
			# incremental scan was built to avoid. Unlike the scan itself
			# (many seconds of *sustained* background CPU work, which caused
			# bad GIL contention with rendering), this is a one-shot ~1s burst
			# that runs once per finished scan, not continuously -- so a
			# background thread here only costs a brief FPS dip, not a
			# multi-second freeze. Nothing else touches these three cache
			# files while this thread runs, and self._external_result is
			# already reassigned above (read-only from here on for this scan).
			threading.Thread(
				target=self._save_external_scan_caches,
				args=(bnp_table_cache, cache, self._external_result),
				daemon=True,
			).start()
			self._merge_and_publish()

			elapsed = time.monotonic() - self._external_scan_start_time
			panoply_note = f", panoply_files.txt found ({len(self._panoply_variants)} textures)" \
				if self._panoply_variants else ""
			self._scan_status = (
				f"{self._external_result.total} asset(s) scanned in {elapsed:.2f}s -- "
				f"{len(self._skeleton_entries)} skeleton(s), {len(self._animation_entries)} animation(s){panoply_note}")
			self._scanning_external = False
			self._external_scan_gen = None
			self._external_scan_caches = None

	@staticmethod
	def _save_external_scan_caches(bnp_table_cache, cache, result):
		"""Runs off the main thread -- see _advance_external_scan(). Pure I/O
		+ JSON serialization on data nothing else mutates concurrently."""
		search_paths.save_scan_cache(cache)
		search_paths.save_bnp_table_cache(bnp_table_cache)
		_save_external_index_cache(result)

	def _reload_workspace_only(self):
		"""(Re)scans *only* the active workspace folder -- see
		_scan_dirs_incremental()/_advance_workspace_scan(). Triggered by the
		workspace watcher (_on_workspace_settled()) and by set_workspace_dir()
		itself: neither has any bearing on the external search paths, so
		this never touches _external_result. Unlike reload(), a call while
		one is already in flight *replaces* it with a fresh one (dropping
		the stale generator) rather than being ignored -- a rapid workspace
		switch or watcher re-trigger shouldn't get silently dropped, and
		there's no risk of piling up threads to guard against anymore."""
		if self._workspace_dir is None:
			return
		self._scanning_workspace = True
		bnp_table_cache = search_paths.load_bnp_table_cache()
		cache = search_paths.load_scan_cache()
		self._workspace_scan_caches = (bnp_table_cache, cache)
		exclude = _make_workspace_exclude(Path(self._workspace_dir.path), app_settings.load().exclusion_rules)
		self._workspace_scan_gen = self._scan_dirs_incremental(
			[self._workspace_dir], bnp_table_cache, cache, exclude=exclude)

	def _advance_workspace_scan(self):
		"""Same idea as _advance_external_scan(), for the workspace half."""
		if self._workspace_scan_gen is None:
			return
		deadline = time.monotonic() + _SCAN_FRAME_TIME_BUDGET
		try:
			while time.monotonic() < deadline:
				next(self._workspace_scan_gen)
		except StopIteration as stop:
			self._workspace_result = stop.value
			bnp_table_cache, cache = self._workspace_scan_caches
			search_paths.save_scan_cache(cache)
			search_paths.save_bnp_table_cache(bnp_table_cache)
			self._merge_and_publish()

			self._scan_status = f"Workspace: {self._workspace_result.total} asset(s) rescanned"
			self._scanning_workspace = False
			self._workspace_scan_gen = None
			self._workspace_scan_caches = None

	def _scan_dirs_incremental(self, dirs, bnp_table_cache, cache, exclude=None):
		"""Pure scan of exactly `dirs` -- no side effects on `self` (the
		caller decides what to do with the result and when to persist
		`bnp_table_cache`/`cache`, both mutated in place here: the .bnp
		table listing itself, search_paths.load_bnp_table_cache()/
		save_bnp_table_cache() -- opening+reading every .bnp's own table is
		the dominant cost of a scan across a real data tree, far more than
		iterating directories or indexing the resulting entries, so this is
		the one that actually matters for scan speed -- and the parsed
		.skel/.anim bone-name cache, search_paths.load_scan_cache()/
		save_scan_cache(), so a file whose (mtime, size) hasn't changed
		since it was last parsed is skipped entirely).

		A generator, not a plain function: `yield`s (no value, just to cede
		control) after every single entry, so a caller (_advance_external_scan()/
		_advance_workspace_scan()) can drive it a time-bounded slice per
		frame on the main thread instead of a background thread competing
		with the render thread for the GIL (see the module-level note).
		Yielding this often costs very little (a generator resume is
		~100ns-1us, negligible against a real scan's own per-entry cost) and
		gives the finest-grained, most responsive time-slicing. The final
		_ScanResult comes back as the exhausted generator's own
		`StopIteration.value` (a `return` inside a generator).

		`exclude`, forwarded to search_paths.iter_all_entries() as-is --
		only ever set for the workspace half (see _reload_workspace_only()'s
		_make_workspace_exclude()), never for the external search paths,
		which have no exclusion-rules concept of their own."""
		skeleton_entries, skeleton_bones = {}, {}
		animation_entries, animation_bones = {}, {}
		texture_entries = {}
		panoply_variants = {}
		total = 0

		entries = search_paths.iter_all_entries(dirs, bnp_table_cache, exclude=exclude)
		while True:
			# Unconditional, right at the top of the loop body -- every
			# iteration yields exactly once this way regardless of which of
			# the several `continue`s below it takes.
			yield
			try:
				found = next(entries)
			except StopIteration:
				break

			total += 1
			lower_name = found.name.lower()
			texture_entries.setdefault(lower_name, found)
			if lower_name == _PANOPLY_FILE_NAME and not panoply_variants:
				try:
					panoply_variants = panoply.parse_panoply_files(found.read_bytes().decode("latin-1"))
				except OSError:
					pass
				continue
			is_skel = lower_name.endswith(".skel")
			is_anim = lower_name.endswith(".anim")
			if not (is_skel or is_anim):
				continue

			try:
				mtime, size = found.cache_stat()
			except OSError:
				continue
			key = found.cache_key()
			cached = cache.get(key)
			if cached is not None and cached.get("mtime") == mtime and cached.get("size") == size:
				cached_bones = cached.get("bones")
				if cached_bones is None:
					# A cached *failure* (see the `else` branch below) --
					# known permanently unparseable as of this
					# (mtime, size), skip without re-attempting the
					# parse. Distinct from a cache miss, where `cached`
					# itself is None.
					continue
				bones = frozenset(cached_bones)
			else:
				bones = self._parse_bones(found, is_skel)
				# Cached either way, success or failure: a real .anim/
				# .skel that this parser can't handle stays permanently
				# unparseable (its mtime/size aren't going to change on
				# their own) -- caching only successes meant a file
				# pynel can't parse got a full, expensive, doomed parse
				# attempt on *every single scan, forever* (this is what
				# was actually behind the persistent per-frame stalls:
				# real Ryzom .anim files using track classes pynel
				# doesn't support yet, e.g. CTrackKeyFramerTCBQuat/
				# BezierVector/BezierFloat/ConstBool -- a pynel gap, not
				# a scanning bug; worth a separate look at some point).
				cache[key] = {
					"mtime": mtime, "size": size, "type": "skel" if is_skel else "anim",
					"bones": sorted(bones) if bones is not None else None,
				}
				if bones is None:
					continue

			if is_skel:
				skeleton_entries[found.name] = found
				skeleton_bones[found.name] = bones
			else:
				animation_entries[found.name] = found
				animation_bones[found.name] = bones

		return _ScanResult(
			skeleton_entries=skeleton_entries, skeleton_bones=skeleton_bones,
			animation_entries=animation_entries, animation_bones=animation_bones,
			texture_entries=texture_entries, panoply_variants=panoply_variants, total=total)

	def _load_ryzom_data_panoply_variants(self):
		"""panoply_files.txt straight from the configured ryzom-data
		checkout (`<ryzom-data>/final_bnps/characters_maps_hr/panoply_files.txt`,
		via `pynel.repository_paths` -- see docs/repository_paths.md), if
		configured and the file exists -- {} otherwise. This is the
		*working-copy* file, edited by hand as new items are added (see
		panoply_bake.py's own use of the same file as a bake source) --
		takes priority over whatever panoply_files.txt _scan_dirs_incremental()
		happens to find inside a shipped characters_maps_hr.bnp along the
		user's generic search paths (see _merge_and_publish()), since that
		shipped copy only reflects whatever last went through the real
		hls_bank_maker build, which lags behind ryzom-data's own working
		copy for anything not built yet (2026-08-29: this is exactly why
		ryw_mark1_hof_caster01_pantabottes's masks weren't showing up in
		Patina even though the .dds files already exist under
		ryzom-data/final_bnps/characters_maps_hr/mark_1/ -- the shipped
		.bnp's panoply_files.txt was never updated for them).

		Cached by mtime -- called every _merge_and_publish(), which itself
		only runs once per finished scan, but a plain read+parse of the real
		ryzom-data panoply_files.txt (comparable size to the shipped copy,
		681KB per the chantier notes) isn't free enough to redo pointlessly
		on every merge if the file hasn't actually changed."""
		if not repository_paths.is_valid("ryzom-data"):
			return {}
		path = repository_paths.get("ryzom-data") / "final_bnps" / "characters_maps_hr" / "panoply_files.txt"
		try:
			mtime = path.stat().st_mtime
		except OSError:
			return {}
		if self._ryzom_data_panoply_mtime == mtime:
			return self._ryzom_data_panoply_variants
		try:
			variants = panoply.parse_panoply_files(path.read_text(encoding="latin-1"))
		except OSError:
			return {}
		self._ryzom_data_panoply_variants = variants
		self._ryzom_data_panoply_mtime = mtime
		return variants

	def _merge_and_publish(self):
		"""Combines _external_result and _workspace_result into the public
		dicts every other method reads -- the workspace wins on a name
		collision (matches the old single-pass scan's own priority order:
		the workspace was always searched first). Each dict is swapped in as a
		whole fresh object, never mutated in place, so a frame reading it
		mid-merge just sees the previous complete result, never a torn one.

		_panoply_variants specifically (2026-08-29): ryzom-data's own working
		copy (_load_ryzom_data_panoply_variants()) wins over both halves of
		the scan if configured and non-empty -- see that method's docstring
		for why ryzom-data needs to take priority over whatever a shipped
		characters_maps_hr.bnp happens to carry."""
		external, workspace = self._external_result, self._workspace_result
		self._skeleton_entries = {**external.skeleton_entries, **workspace.skeleton_entries}
		self._skeleton_bones = {**external.skeleton_bones, **workspace.skeleton_bones}
		self._animation_entries = {**external.animation_entries, **workspace.animation_entries}
		self._animation_bones = {**external.animation_bones, **workspace.animation_bones}
		self._texture_entries = {**external.texture_entries, **workspace.texture_entries}
		self._panoply_variants = (
			self._load_ryzom_data_panoply_variants() or workspace.panoply_variants or external.panoply_variants)

	@staticmethod
	def _parse_bones(found, is_skel):
		"""Full parse, only for a cache miss -- the bone names a .skel
		defines (SkeletonShape.bone_map) or a .anim has tracks for
		(_animation_bone_names()). None on any parse failure (the file is
		just skipped, same as before caching existed)."""
		try:
			data = found.read_bytes()
		except OSError:
			return None
		if is_skel:
			try:
				shape_file = parse_shape(data)
			except ShapeParseError:
				return None
			if not isinstance(shape_file.value, SkeletonShape):
				return None
			return frozenset(shape_file.value.bone_map)
		try:
			anim = parse_animation(data)
		except AnimationParseError:
			return None
		return frozenset(_animation_bone_names(anim))

	def scanned_skeleton_names(self):
		return sorted(self._skeleton_bones)

	def compatible_for(self, bones_name):
		"""Names (sorted) of every scanned .skel whose bones are a
		superset of `bones_name` -- same compatibility test the engine
		itself applies at bind time (CSkeletonModel::remapSkinBones)."""
		wanted = set(bones_name)
		return sorted(name for name, bones in self._skeleton_bones.items() if wanted <= bones)

	def skeleton_for(self, name):
		"""Full parse of this one specific .skel, on demand -- only called
		once, when the user actually picks `name` (see
		object_editor.py's _apply_bone_preview_skeleton()), so there's no
		need to keep every scanned skeleton's full parsed data (inverse
		bind matrices etc.) in memory just for the combo/compatibility
		list, which only ever needed the bone names (see _skeleton_bones)."""
		entry = self._skeleton_entries.get(name)
		if entry is None:
			return None
		try:
			shape_file = parse_shape(entry.read_bytes())
		except (ShapeParseError, OSError):
			return None
		return shape_file.value if isinstance(shape_file.value, SkeletonShape) else None

	def scanned_animation_names(self):
		return sorted(self._animation_bones)

	def compatible_animations_for(self, skeleton):
		"""Names (sorted) of every scanned .anim whose own tracked bone
		names are all present in `skeleton` -- same idea as compatible_for()
		but the other way around (the animation's bones must be a subset of
		the skeleton's, not the other way -- an animation legitimately
		doesn't need to touch every bone)."""
		if skeleton is None:
			return []
		skeleton_bones = skeleton.bone_map.keys()
		return sorted(name for name, bones in self._animation_bones.items() if bones <= skeleton_bones)

	def animation_for(self, name):
		"""Full parse of this one specific .anim, on demand -- see
		skeleton_for()'s own note on why only the bone names are kept for
		every scanned entry, not the full parsed Animation."""
		entry = self._animation_entries.get(name)
		if entry is None:
			return None
		try:
			return parse_animation(entry.read_bytes())
		except (AnimationParseError, OSError):
			return None

	def find_texture(self, name):
		"""Resolves `name` against this dialog's own scanned (.bnp-aware)
		texture index -- see search_paths.find_texture() for the matching
		rules (case-insensitive, extension fallback)."""
		return search_paths.find_texture(self._texture_entries, name)

	def panoply_variants_for(self, base_texture_name):
		"""{axis: [value, ...]} (panoply.AXES) of panoply variants
		panoply_files.txt lists for `base_texture_name` (e.g.
		"tr_hof_armor00_handupside_c1.tga") -- an axis with an empty list
		means this texture has no mask for it (e.g. armor never has
		hair/eyes). {} (every axis empty) if no panoply_files.txt was found
		by the last scan, or this texture has no variants in it at all."""
		stem = Path(base_texture_name).stem.lower()
		return self._panoply_variants.get(stem, {})

	def draw_settings_content(self):
		"""Embedded in object_editor.py's Settings tab, under its own
		"Paths" section -- these are app-wide search folders, not tied to
		any one shape. Order matters: the first folder that has a given
		file wins (iter_all_entries()/find_texture() both just take the
		first match, in list order) -- the up/down buttons let a folder be
		promoted/demoted in priority instead of only add/remove."""
		imgui.text("Folders searched (top = highest priority):")
		if self._workspace_dir is not None:
			suffix = " (active workspace, always first)"
			available = imgui.get_content_region_avail().x - imgui.calc_text_size(suffix).x
			imgui.text_disabled(_truncate_path_to_width(self._workspace_dir.path, max(available, 20)) + suffix)
			if imgui.is_item_hovered():
				imgui.set_tooltip(self._workspace_dir.path)
		style = imgui.get_style()
		recursive_width = imgui.calc_text_size(fa_icons.ICON_FA_SITEMAP).x + style.frame_padding.x * 2
		reorder_width = imgui.calc_text_size(fa_icons.ICON_FA_ARROW_UP).x + style.frame_padding.x * 2
		remove_width = imgui.calc_text_size(fa_icons.ICON_FA_TRASH).x + style.frame_padding.x * 2
		remove_index = None
		move_up_index = None
		move_down_index = None
		for index, entry in enumerate(self._dirs):
			imgui.push_id(str(index))
			path_width = (imgui.get_content_region_avail().x - recursive_width - remove_width
			              - 2 * reorder_width - style.item_spacing.x * 4)
			imgui.text(_truncate_path_to_width(entry.path, max(path_width, 20)))
			if imgui.is_item_hovered():
				imgui.set_tooltip(entry.path)
			imgui.same_line()
			tooltip = "Recursive: includes subfolders (click to toggle)" if entry.recursive \
				else "Not recursive: this folder only (click to toggle)"
			recursive_color = _RECURSIVE_ON_COLOR if entry.recursive else _RECURSIVE_OFF_COLOR
			if _icon_button(fa_icons.ICON_FA_SITEMAP, tooltip, color=recursive_color):
				entry.recursive = not entry.recursive
				self._save()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_ARROW_UP, "Move up (higher priority)", disabled=index == 0):
				move_up_index = index
			imgui.same_line()
			if _icon_button(
					fa_icons.ICON_FA_ARROW_DOWN, "Move down (lower priority)", disabled=index == len(self._dirs) - 1):
				move_down_index = index
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_TRASH, "Remove this folder"):
				remove_index = index
			imgui.pop_id()
		if move_up_index is not None:
			self._dirs[move_up_index - 1], self._dirs[move_up_index] = \
				self._dirs[move_up_index], self._dirs[move_up_index - 1]
			self._save()
		if move_down_index is not None:
			self._dirs[move_down_index + 1], self._dirs[move_down_index] = \
				self._dirs[move_down_index], self._dirs[move_down_index + 1]
			self._save()
		if remove_index is not None:
			del self._dirs[remove_index]
			self._save()

		if _icon_button(f"{fa_icons.ICON_FA_PLUS}##add-search-folder", "Add folder..."):
			self._add_dir_dialog = pfd.select_folder("Choose a search folder")
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_ARROWS_ROTATE, "Reload", disabled=self.scanning):
			self.reload()

		if self._scan_status:
			imgui.text_disabled(self._scan_status)
