"""Generic debounced filesystem watcher for a workspace: a single watchdog
Observer (one background thread) recursively watching the whole workspace
root, dispatching each settled per-file change to whichever callbacks are
registered for the top-level subfolder the file lives under.

Replaces what used to be one independent Observer per feature
(import_watcher.py, workspace_sync.py), each duplicating the same per-path
debounce logic. Consolidated per user request (2026-08-27): separate
Observers gave no real concurrency/frame-smoothness benefit here --
watchdog's OS-level wait is idle-cost-free regardless of Observer count,
and Panda3D's render loop was already decoupled from these watchers via
cross-thread status queuing on the app side -- it was organic duplication,
not a deliberate perf design. A single shared implementation is also
easier to instrument/harden (logging, crash isolation per callback) than
several near-identical ones.
"""

import threading
from pathlib import Path

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

# Same value every per-feature watcher in this codebase used independently
# before this consolidation -- an inactivity debounce (the timer restarts
# on every event), so it's robust regardless of how long a given file
# takes to finish writing.
_WATCH_DEBOUNCE_SECONDS = 0.5

_RELEVANT_EVENT_TYPES = {"created", "modified", "moved", "deleted"}


class _DebouncedHandler(FileSystemEventHandler):
	"""Each changed file's own burst of events is debounced independently
	(per-path timer), so editing one file doesn't reset another file's
	pending timer. A "moved" event schedules settling for *both* its src
	and dest path (a rename affects two logical files). watchdog calls
	on_any_event() from its own OS-native watcher thread, never the
	main/render thread."""

	def __init__(self, on_settled):
		self._on_settled = on_settled
		self._timers = {}
		self._lock = threading.Lock()

	def on_any_event(self, event):
		if event.is_directory or event.event_type not in _RELEVANT_EVENT_TYPES:
			return
		paths = [Path(event.src_path)]
		if event.event_type == "moved":
			paths.append(Path(event.dest_path))
		for path in paths:
			self._schedule(path)

	def _schedule(self, path):
		with self._lock:
			timer = self._timers.pop(path, None)
			if timer is not None:
				timer.cancel()
			timer = threading.Timer(_WATCH_DEBOUNCE_SECONDS, self._fire, args=(path,))
			timer.daemon = True
			self._timers[path] = timer
			timer.start()

	def _fire(self, path):
		with self._lock:
			self._timers.pop(path, None)
		self._on_settled(path)

	def cancel(self):
		with self._lock:
			for timer in self._timers.values():
				timer.cancel()
			self._timers.clear()


class WorkspaceWatcher:
	"""Watches an active workspace's root recursively with a single
	Observer/debounce handler, dispatching each settled file path to every
	callback registered for the top-level subfolder it falls under (e.g.
	"tex", "imports").

	register(subdir, callback) any time, before or after
	set_workspace_dir() -- callback(path: Path) is invoked from a
	background thread once a settle period has passed since the file's
	last change; `path` may no longer exist (deleted). A callback that
	raises is caught and logged -- one broken conversion must never take
	the whole watcher down or block other callbacks.

	Call set_workspace_dir() whenever the active workspace changes (pass
	None to stop watching)."""

	def __init__(self):
		self._observer = None
		self._handler = None
		self._workspace_dir = None
		self._callbacks = {}  # {subdir_name: [callback, ...]}

	def register(self, subdir, callback):
		self._callbacks.setdefault(subdir, []).append(callback)

	def set_workspace_dir(self, workspace_dir):
		"""Stops any previous watch and, if `workspace_dir` is set, starts a
		fresh one recursively on the workspace root."""
		if self._observer is not None:
			self._observer.stop()
			self._observer = None
		if self._handler is not None:
			self._handler.cancel()
			self._handler = None
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		if self._workspace_dir is None:
			return

		self._workspace_dir.mkdir(parents=True, exist_ok=True)
		handler = _DebouncedHandler(self._dispatch)
		observer = Observer()
		try:
			observer.schedule(handler, str(self._workspace_dir), recursive=True)
		except OSError as exc:
			print(f"[workspace_watch] could not watch {self._workspace_dir!r}: {exc}")
			return
		observer.daemon = True
		observer.start()
		self._observer = observer
		self._handler = handler

	def _dispatch(self, path):
		"""Runs off the watchdog Observer's own background thread."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		try:
			relative = path.relative_to(workspace_dir)
		except ValueError:
			return  # shouldn't happen -- watched paths are always under workspace_dir
		if not relative.parts:
			return  # the workspace root itself, not a file within a subfolder
		subdir = relative.parts[0]
		for callback in self._callbacks.get(subdir, ()):
			try:
				callback(path)
			except Exception as exc:  # noqa: BLE001 -- one broken callback must not affect the others or kill the watcher
				print(f"[workspace_watch] callback for {subdir}/ failed on {path}: {exc}")
