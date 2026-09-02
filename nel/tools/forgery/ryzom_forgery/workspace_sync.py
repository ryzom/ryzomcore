"""Mirrors every .shape/.anim/.skel/.dds found anywhere in an active
workspace (excluded paths aside) into a flat external sync folder -- see
the "Sync workspace to an external folder" chantier in
`project-todos/ryzom-core/forgery-object-editor.md`. Copy-only: a file
removed from the workspace is left as-is in the sync folder, never deleted
there.

Doesn't own its own filesystem watch -- `handle_settled()` is meant to be
registered onto a shared `workspace_watch.WorkspaceWatcher` via
`register_extension()` for SYNCED_EXTENSIONS (see apps/object_editor.py).
See workspace_watch.py's module docstring for why this was consolidated
out of a dedicated Observer per feature.

Reworked 2026-09-02 (see the workspace-watcher chantier in
project-todos/ryzom-core/forgery-object-editor.md): triggers on a matching
file anywhere in the workspace now, not just the 4 old fixed subfolders
(anims/shapes/skels/dds), and the mirror is flat
(<sync_folder>/forgery/<workspace>/<filename>, no subfolders) instead of
preserving each file's relative path -- matches pack_workspace_bnp()'s own
flat packing (a same-name collision would surface there regardless, so the
sync mirror shouldn't pretend to avoid it by keeping subfolders it won't
have once packed anyway). Flattening means two different sources sharing
an exact filename (case-insensitive) would now silently collide in the
mirror -- guarded against via a DuplicateNameGuard (compared by full
filename here, not stem: nothing is converted, the output name is the
input name as-is), routing a collision to on_name_conflict() instead of
copying either file.
"""

import shutil
import tempfile
import threading
from pathlib import Path

from pynel.ryzom_bnp import pack_directory

from ryzom_forgery import settings as app_settings
from ryzom_forgery.duplicate_name_guard import DuplicateNameGuard
from ryzom_forgery.virtual_categories import iter_included_files

# Deliberately not masks/exports/imports/ -- masks are Panoply recolor
# inputs (not shipped assets themselves), exports/ is the export dialog's
# own output staging area, and imports/ is the auto-export watcher's own
# staging area (its own output already lands in shapes/, which is synced).
# tex/ isn't synced either as of the patina-tex-dds-autoexport chantier --
# build/dds/ (the generated .dds mirror, see tex_dds_sync.py) is what the
# game client actually needs, so that's what gets synced instead.
SYNCED_EXTENSIONS = {".shape", ".anim", ".skel", ".dds"}


class WorkspaceSyncWatcher:
	"""Mirrors every .shape/.anim/.skel/.dds found anywhere in the active
	workspace (see SYNCED_EXTENSIONS) into a flat configured external sync
	folder. A no-op end to end without both a workspace and a sync folder
	configured.

	Call set_workspace_dir() whenever the active workspace changes (pass
	None to stop tracking), and set_sync_folder() whenever the configured
	sync folder itself changes (see object_editor.py's Settings > Tools UI)
	-- independent of each other, so either can be set/cleared without
	touching the other. Doesn't watch the filesystem itself -- register
	handle_settled onto a shared WorkspaceWatcher via register_extension()
	for SYNCED_EXTENSIONS instead."""

	def __init__(self, on_name_conflict=None):
		"""`on_name_conflict(path_a, path_b)`, if given, is called whenever
		two different sources anywhere in the workspace share an exact
		filename (case-insensitive) -- neither is copied until the host app
		resolves it (see DuplicateNameGuard)."""
		self._workspace_dir = None
		self._sync_folder = None
		self._fully_synced = True  # see refresh_fully_synced()/is_fully_synced()
		self._syncing = False  # see sync_now()/is_syncing()
		self._guard = DuplicateNameGuard(
			key_of=lambda path: path.name.lower(),
			on_conflict=on_name_conflict or (lambda path_a, path_b: None))

	def set_workspace_dir(self, workspace_dir):
		"""Tracks the active workspace and rebuilds the duplicate-name index
		on a background thread -- only to catch a conflict that already
		existed before this session started watching (see
		import_watcher.py's own set_workspace_dir() for why this doesn't
		also re-copy anything by itself; unlike that one though, sync_now()
		below reuses this same index for its own manual catch-up). Pass
		None when no workspace is active."""
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		self._guard.reset()
		if self._workspace_dir is not None:
			threading.Thread(target=self._rebuild_index, daemon=True).start()

	def set_sync_folder(self, sync_folder):
		"""None disables mirroring (handle_settled() becomes a no-op)
		without touching the shared watcher's registration itself."""
		self._sync_folder = Path(sync_folder) if sync_folder else None
		self.refresh_fully_synced()

	def _synced_sources(self):
		exclusion_rules = app_settings.load().exclusion_rules
		return (
			path for path in iter_included_files(self._workspace_dir, exclusion_rules)
			if path.suffix.lower() in SYNCED_EXTENSIONS)

	def _rebuild_index(self):
		if self._workspace_dir is not None:
			self._guard.scan(self._synced_sources())

	def refresh_fully_synced(self):
		"""Recomputes whether every currently known-safe workspace file
		already has a mirrored copy on the sync-folder side -- existence
		only, not content/mtime (the live watch already keeps up with edits
		going forward; this is only about files that predate it -- e.g.
		right after configuring a sync folder, or one deleted from the
		destination by hand). Uses the duplicate-name index's own current
		snapshot rather than a fresh directory walk -- deliberately only
		called here and from sync_now() -- never every frame -- see
		is_fully_synced()."""
		if self._workspace_dir is None or self._sync_folder is None:
			self._fully_synced = True  # nothing configured to report on -- the UI hides the "Sync now" button either way
			return
		dest_root = self._sync_folder / "forgery" / self._workspace_dir.name
		self._fully_synced = all((dest_root / path.name).is_file() for path in self._guard.known_paths())

	def is_fully_synced(self):
		return self._fully_synced

	def is_syncing(self):
		return self._syncing

	def sync_now(self):
		"""Manual catch-up: rebuilds the duplicate-name index fresh (so a
		conflict introduced while the app wasn't watching -- or resolved
		since -- is caught right away instead of waiting for the next fs
		event) then mirrors every currently known-safe file, regardless of
		whether the live watch already caught it -- for whatever predates
		the watch itself (see refresh_fully_synced()). Runs on a background
		thread (reusing handle_settled() for the actual per-file copy, same
		as the watch's own debounced callback) since a workspace can hold a
		lot of files -- same reasoning as SearchPathsDialog's own background
		scan."""
		if self._workspace_dir is None or self._sync_folder is None or self._syncing:
			return
		self._syncing = True
		threading.Thread(target=self._sync_now_worker, daemon=True).start()

	def _sync_now_worker(self):
		safe_sources = self._guard.scan(self._synced_sources())
		for path in safe_sources:
			self.handle_settled(path)
		self.refresh_fully_synced()
		self._syncing = False

	def handle_settled(self, source_path):
		"""Registered onto a shared WorkspaceWatcher via register_extension()
		for SYNCED_EXTENSIONS -- runs off that watcher's background thread.
		A file that no longer exists (deleted, or the source side of a
		rename) is dropped from the duplicate-name index -- if that resolves
		a conflict, the surviving path is copied right away (it was held
		back the whole time the conflict stood)."""
		workspace_dir = self._workspace_dir
		sync_folder = self._sync_folder
		if workspace_dir is None:
			return

		if not source_path.is_file():
			survivor = self._guard.remove(source_path)
			if survivor is not None and sync_folder is not None:
				self._copy(survivor, sync_folder / "forgery" / workspace_dir.name / survivor.name)
			return

		if not self._guard.update(source_path):
			return  # conflicting with another source sharing the same filename -- on_name_conflict already fired
		if sync_folder is None:
			return
		self._copy(source_path, sync_folder / "forgery" / workspace_dir.name / source_path.name)

	def _copy(self, source_path, dest_path):
		# "forgery/<workspace name>/" prefix (workspace_dir.name -- workspace
		# folders are always named after the workspace itself, see
		# workspaces.py's workspace_path()) rather than dumping straight into
		# sync_folder -- keeps this tool's output identifiable and separate
		# from anything else already living there, and lets several
		# workspaces share the same sync folder without their shapes/anims/
		# skels/dds colliding with each other.
		try:
			dest_path.parent.mkdir(parents=True, exist_ok=True)
			shutil.copy2(source_path, dest_path)
		except OSError as exc:
			print(f"[workspace_sync] failed to mirror {source_path.name}: {exc}")
		else:
			print(f"[workspace_sync] mirrored {source_path.name} -> {dest_path}")


def pack_workspace_bnp(workspace_dir, bnp_path):
	"""Packs every file matching SYNCED_EXTENSIONS found anywhere in
	`workspace_dir` (excluded paths aside -- the same scope an external sync
	folder receives) into a single flat `.bnp` -- the "Full workspace"
	option in Patina's export format menu (see the patina-export-rework
	chantier). Stages a flat copy first (a `.bnp` has no sub-folder concept,
	and two different sources sharing an exact filename would collide --
	unlikely now that the workspace watcher itself guards against this
	living in the workspace in the first place, but `pack_directory()`
	still raises `pynel.ryzom_bnp.BnpError` as a last-resort guard if it
	ever happens anyway, e.g. a conflict introduced while Patina wasn't
	running to catch it)."""
	from ryzom_forgery import settings as app_settings

	workspace_dir = Path(workspace_dir)
	exclusion_rules = app_settings.load().exclusion_rules
	with tempfile.TemporaryDirectory() as staging:
		staging_path = Path(staging)
		for path in iter_included_files(workspace_dir, exclusion_rules):
			if path.suffix.lower() in SYNCED_EXTENSIONS:
				shutil.copy2(path, staging_path / path.name)
		pack_directory(staging_path, bnp_path)
