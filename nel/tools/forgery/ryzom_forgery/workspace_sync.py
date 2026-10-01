"""Mirrors files from an active workspace's anims/, shapes/, skels/, and
dds/ subfolders into an external sync folder -- see the "Sync workspace to
an external folder" chantier in
`project-todos/ryzom-core/forgery-object-editor.md`. Copy-only: a file
removed from the workspace is left as-is in the sync folder, never deleted
there.

Doesn't own its own filesystem watch -- `handle_settled()` is meant to be
registered onto a shared `workspace_watch.WorkspaceWatcher` for each of
SYNCED_SUBDIRS (see apps/object_editor.py). See workspace_watch.py's
module docstring for why this was consolidated out of a dedicated Observer
per feature.
"""

import shutil
import tempfile
import threading
from pathlib import Path

from pynel.ryzom_bnp import pack_directory

# Deliberately not masks/exports/imports/ -- masks are Panoply recolor
# inputs (not shipped assets themselves), exports/ is the export dialog's
# own output staging area, and imports/ is the auto-export watcher's own
# staging area (its own output already lands in shapes/, which is synced).
# tex/ isn't synced either as of the patina-tex-dds-autoexport chantier --
# dds/ (the generated .dds mirror of tex/, see tex_dds_sync.py) is what the
# game client actually needs, so that's what gets synced instead.
SYNCED_SUBDIRS = ("anims", "shapes", "skels", "dds")


class WorkspaceSyncWatcher:
	"""Mirrors the active workspace's anims/shapes/skels/dds subfolders (see
	SYNCED_SUBDIRS) into a configured external sync folder, preserving each
	file's path relative to the workspace root. A no-op end to end without
	both a workspace and a sync folder configured.

	Call set_workspace_dir() whenever the active workspace changes (pass
	None to stop tracking), and set_sync_folder() whenever the configured
	sync folder itself changes (see object_editor.py's Settings > Tools UI)
	-- independent of each other, so either can be set/cleared without
	touching the other. Doesn't watch the filesystem itself -- register
	handle_settled onto a shared WorkspaceWatcher for each of
	SYNCED_SUBDIRS instead."""

	def __init__(self):
		self._workspace_dir = None
		self._sync_folder = None
		self._fully_synced = True  # see refresh_fully_synced()/is_fully_synced()
		self._syncing = False  # see sync_now()/is_syncing()

	def set_workspace_dir(self, workspace_dir):
		"""Tracks the active workspace and ensures its synced subfolders
		exist. Pass None when no workspace is active."""
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		if self._workspace_dir is not None:
			for subdir in SYNCED_SUBDIRS:
				(self._workspace_dir / subdir).mkdir(parents=True, exist_ok=True)

	def set_sync_folder(self, sync_folder):
		"""None disables mirroring (handle_settled() becomes a no-op)
		without touching the shared watcher's registration itself."""
		self._sync_folder = Path(sync_folder) if sync_folder else None
		self.refresh_fully_synced()

	def _iter_workspace_files(self):
		for subdir in SYNCED_SUBDIRS:
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
		"""Manual catch-up: mirrors every current anims/shapes/skels/dds
		file right away, regardless of whether the live watch already
		caught it -- for whatever predates the watch itself (see
		refresh_fully_synced()). Runs on a background thread (reusing
		handle_settled() for the actual per-file copy, same as the watch's
		own debounced callback) since a workspace can hold a lot of files --
		same reasoning as SearchPathsDialog's own background scan."""
		if self._workspace_dir is None or self._sync_folder is None or self._syncing:
			return
		self._syncing = True
		threading.Thread(target=self._sync_now_worker, daemon=True).start()

	def _sync_now_worker(self):
		for path in self._iter_workspace_files():
			self.handle_settled(path)
		self.refresh_fully_synced()
		self._syncing = False

	def handle_settled(self, source_path):
		"""Registered onto a shared WorkspaceWatcher for each of
		SYNCED_SUBDIRS -- runs off that watcher's background thread."""
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
		# skels/dds colliding with each other.
		dest_path = sync_folder / "forgery" / workspace_dir.name / relative_path
		try:
			dest_path.parent.mkdir(parents=True, exist_ok=True)
			shutil.copy2(source_path, dest_path)
		except OSError as exc:
			print(f"[workspace_sync] failed to mirror {relative_path}: {exc}")
		else:
			print(f"[workspace_sync] mirrored {relative_path} -> {dest_path}")


def pack_workspace_bnp(workspace_dir, bnp_path):
	"""Packs every file under this workspace's SYNCED_SUBDIRS (the same
	scope an external sync folder receives) into a single flat `.bnp` --
	the "Full workspace" option in Patina's export format menu (see the
	patina-export-rework chantier). Stages a flat copy first (a `.bnp` has
	no sub-folder concept, and `SYNCED_SUBDIRS` may hold same-named files
	in different sub-folders which would collide -- unlikely in practice
	given each sub-folder holds a distinct asset type, but `pack_directory()`
	raises `pynel.ryzom_bnp.BnpError` if it ever happens)."""
	workspace_dir = Path(workspace_dir)
	with tempfile.TemporaryDirectory() as staging:
		staging_path = Path(staging)
		for subdir in SYNCED_SUBDIRS:
			base = workspace_dir / subdir
			if not base.is_dir():
				continue
			for path in base.rglob("*"):
				if path.is_file():
					shutil.copy2(path, staging_path / path.name)
		pack_directory(staging_path, bnp_path)
