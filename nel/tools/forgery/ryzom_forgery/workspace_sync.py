"""Mirrors files from an active workspace's anims/, shapes/, skels/, and
tex/ subfolders into an external sync folder -- see the "Sync workspace to
an external folder" chantier in
`project-todos/ryzom-core/forgery-object-editor.md`. Copy-only: a file
removed from the workspace is left as-is in the sync folder, never deleted
there.
"""

import shutil
import threading
from pathlib import Path

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

# Same value as import_watcher.py/search_paths_dialog.py's own watches -- an
# inactivity debounce (the timer restarts on every event), so it's robust
# regardless of how long a given file takes to finish writing.
_WATCH_DEBOUNCE_SECONDS = 0.5

_RELEVANT_EVENT_TYPES = {"created", "modified", "moved"}

# Deliberately not masks/exports/imports/ -- masks are Panoply recolor
# inputs (not shipped assets themselves), exports/ is the export dialog's
# own output staging area, and imports/ is the auto-export watcher's own
# staging area (its own output already lands in shapes/, which is synced).
_SYNCED_SUBDIRS = ("anims", "shapes", "skels", "tex")


class _DebouncedSyncHandler(FileSystemEventHandler):
	"""Same coalescing idea as import_watcher.py's own
	_DebouncedImportHandler: each changed file's own burst of write events is
	debounced independently (per-path timer), so editing one file doesn't
	reset another file's pending timer. watchdog calls on_any_event() from
	its own OS-native watcher thread, never the main/render thread."""

	def __init__(self, on_settled):
		self._on_settled = on_settled
		self._timers = {}
		self._lock = threading.Lock()

	def on_any_event(self, event):
		if event.is_directory or event.event_type not in _RELEVANT_EVENT_TYPES:
			return
		# FileMovedEvent (a file renamed/dropped into place) carries the final
		# path as dest_path instead of src_path.
		path = Path(getattr(event, "dest_path", None) or event.src_path)
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


class WorkspaceSyncWatcher:
	"""Watches the active workspace's anims/shapes/skels/tex subfolders (see
	_SYNCED_SUBDIRS) and mirrors any created/modified/moved file into a
	configured external sync folder, preserving its path relative to the
	workspace root. A no-op end to end without both a workspace and a sync
	folder configured.

	Call set_workspace_dir() whenever the active workspace changes (pass
	None to stop watching), and set_sync_folder() whenever the configured
	sync folder itself changes (see object_editor.py's Settings > Tools UI)
	-- independent of each other, so either can be set/cleared without
	touching the other."""

	def __init__(self):
		self._observer = None
		self._workspace_dir = None
		self._sync_folder = None
		self._fully_synced = True  # see refresh_fully_synced()/is_fully_synced()
		self._syncing = False  # see sync_now()/is_syncing()

	def set_workspace_dir(self, workspace_dir):
		"""Stops any previous watch and, if `workspace_dir` is set, starts a
		fresh one on its anims/shapes/skels/tex subfolders -- same
		native-OS-API-via-watchdog approach, and same
		silently-give-up-on-failure-per-folder reasoning, as
		import_watcher.py's own workspace watch."""
		if self._observer is not None:
			self._observer.stop()
			self._observer = None
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		if self._workspace_dir is None:
			return

		handler = _DebouncedSyncHandler(self._handle_settled)
		observer = Observer()
		scheduled = False
		for subdir in _SYNCED_SUBDIRS:
			watched_dir = self._workspace_dir / subdir
			watched_dir.mkdir(parents=True, exist_ok=True)
			try:
				observer.schedule(handler, str(watched_dir), recursive=True)
				scheduled = True
			except OSError as exc:
				print(f"[workspace_sync] could not watch {watched_dir!r}: {exc}")
		if not scheduled:
			return
		observer.daemon = True
		observer.start()
		self._observer = observer

	def set_sync_folder(self, sync_folder):
		"""None disables mirroring (still watching, but _handle_settled()
		becomes a no-op) without tearing down/rebuilding the watch itself."""
		self._sync_folder = Path(sync_folder) if sync_folder else None
		self.refresh_fully_synced()

	def _iter_workspace_files(self):
		for subdir in _SYNCED_SUBDIRS:
			base = self._workspace_dir / subdir
			if not base.is_dir():
				continue
			for path in base.rglob("*"):
				if path.is_file():
					yield path

	def refresh_fully_synced(self):
		"""Recomputes whether every current workspace file already has a
		mirrored copy on the sync-folder side -- existence only, not
		content/mtime (the live watch already keeps up with edits going
		forward; this is only about files that predate it -- e.g. right
		after configuring a sync folder, or one deleted from the
		destination by hand). A real directory walk, so deliberately only
		called here and from sync_now() -- never every frame -- see
		is_fully_synced()."""
		if self._workspace_dir is None or self._sync_folder is None:
			self._fully_synced = True  # nothing configured to report on -- the UI hides the "Sync now" button either way
			return
		dest_root = self._sync_folder / "forgery" / self._workspace_dir.name
		self._fully_synced = all(
			(dest_root / path.relative_to(self._workspace_dir)).is_file()
			for path in self._iter_workspace_files())

	def is_fully_synced(self):
		return self._fully_synced

	def is_syncing(self):
		return self._syncing

	def sync_now(self):
		"""Manual catch-up: mirrors every current anims/shapes/skels/tex
		file right away, regardless of whether the live watch already
		caught it -- for whatever predates the watch itself (see
		refresh_fully_synced()). Runs on a background thread (reusing
		_handle_settled() for the actual per-file copy, same as the watch's
		own debounced callback) since a workspace can hold a lot of files --
		same reasoning as SearchPathsDialog's own background scan."""
		if self._workspace_dir is None or self._sync_folder is None or self._syncing:
			return
		self._syncing = True
		threading.Thread(target=self._sync_now_worker, daemon=True).start()

	def _sync_now_worker(self):
		for path in self._iter_workspace_files():
			self._handle_settled(path)
		self.refresh_fully_synced()
		self._syncing = False

	def _handle_settled(self, source_path):
		"""Runs off the watchdog Observer's own background thread."""
		workspace_dir = self._workspace_dir
		sync_folder = self._sync_folder
		if workspace_dir is None or sync_folder is None:
			return
		try:
			relative_path = source_path.relative_to(workspace_dir)
		except ValueError:
			return  # shouldn't happen -- watched paths are always under workspace_dir
		if not source_path.is_file():
			return  # gone (deleted/renamed away) by the time the debounce settled -- copy-only, nothing to mirror

		# "forgery/<workspace name>/" prefix (workspace_dir.name -- workspace
		# folders are always named after the workspace itself, see
		# workspaces.py's workspace_path()) rather than dumping straight into
		# sync_folder -- keeps this tool's output identifiable and separate
		# from anything else already living there, and lets several
		# workspaces share the same sync folder without their shapes/anims/
		# skels/tex colliding with each other.
		dest_path = sync_folder / "forgery" / workspace_dir.name / relative_path
		try:
			dest_path.parent.mkdir(parents=True, exist_ok=True)
			shutil.copy2(source_path, dest_path)
		except OSError as exc:
			print(f"[workspace_sync] failed to mirror {relative_path}: {exc}")
		else:
			print(f"[workspace_sync] mirrored {relative_path} -> {dest_path}")
