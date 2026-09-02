"""Keeps an active workspace's build/dds/ subfolder automatically in sync
with every TGA/PNG found anywhere in the workspace (excluded paths aside):
each source gets a flat build/dds/<stem>.dds, regenerated whenever the
source is created/modified, and removed whenever the source is deleted --
see the "patina-tex-dds-autoexport" chantier in
project-todos/ryzom-core/patina-tex-dds-autoexport.md.

Doesn't own its own filesystem watch -- handle_settled() is meant to be
registered onto a shared workspace_watch.WorkspaceWatcher via
register_extension() for TEX_EXTENSIONS (see apps/object_editor.py).
See workspace_watch.py's module docstring for why this was consolidated
out of a dedicated Observer per feature.

Reworked 2026-09-02 (see the workspace-watcher chantier in
project-todos/ryzom-core/forgery-object-editor.md): triggers on a matching
file anywhere in the workspace now, not just tex/, and the dds/ mirror is
flat (build/dds/<stem>.dds) instead of mirroring the source's own subpath
-- matches the .bnp packing format, and per Nuno a same-stem collision
would surface at .bnp-pack time regardless. Also fixes a found
pre-existing bug: this module used to read/write a top-level
<workspace>/dds/, but workspaces.py actually creates build/dds/
(_BUILD_SUBDIRS, moved there in the 2026-09-01 workspace rework) -- the two
were never actually the same folder. Flattening means two different
sources sharing a stem (case-insensitive) would now silently collide on
the same build/dds/<stem>.dds -- guarded against via a DuplicateNameGuard
(see that module), routing a collision to on_name_conflict() instead of
converting either file.

Specular textures are out of scope for now: Patina doesn't manage texture
roles/slots yet, so a source file is never recognizable as specular in
practice. Every supported file is converted uniformly, including Multi
Bitmap season/quality variants (this is a plain per-file mirror, not aware
of which shape/material actually uses a given texture).

DDS export always builds mipmaps (these are always 3D model textures, see
dds_export.py's own docstring on why an asset regenerated without mipmaps
looks visibly worse than the original).
"""

import threading
from pathlib import Path

from ryzom_forgery import settings as app_settings
from ryzom_forgery.dds_export import load_rgba, build_dds, pick_default_algo
from ryzom_forgery.duplicate_name_guard import DuplicateNameGuard
from ryzom_forgery.virtual_categories import iter_included_files

# Same scope as tga2dds.cpp/dds_export.py -- not explorer.py's broader
# _TEXTURE_EXTENSIONS (which also lists .dds/.jpg/.jpeg/.bmp, formats this
# pipeline was never meant to read as a *source*).
TEX_EXTENSIONS = {".tga", ".png"}


class TexDdsSyncWatcher:
	"""Mirrors every TGA/PNG found anywhere in the active workspace into a
	flat build/dds/, converting each to a .dds with mipmaps. A no-op end to
	end without an active workspace.

	Call set_workspace_dir() whenever the active workspace changes (pass
	None to stop tracking) -- this also kicks off a background reconcile()
	pass, since a workspace closed then reopened (or a source file edited
	by hand outside Patina) can have drifted out of sync while nothing was
	watching. Doesn't watch the filesystem itself -- register
	handle_settled onto a shared WorkspaceWatcher via register_extension()
	for TEX_EXTENSIONS instead."""

	def __init__(self, on_status=None, on_name_conflict=None):
		"""`on_name_conflict(path_a, path_b)`, if given, is called (from
		whatever thread detected it -- reconcile()'s background thread or
		handle_settled()'s watcher thread) whenever two different source
		files would produce the same build/dds/<stem>.dds -- neither is
		converted until the host app resolves it (see DuplicateNameGuard)."""
		self._workspace_dir = None
		self._on_status = on_status or (lambda message, is_error=False: None)
		self._guard = DuplicateNameGuard(
			key_of=lambda path: path.stem.lower(),
			on_conflict=on_name_conflict or (lambda path_a, path_b: None))

	def _dds_dir(self):
		return self._workspace_dir / "build" / "dds"

	def set_workspace_dir(self, workspace_dir):
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		self._guard.reset()
		if self._workspace_dir is None:
			return
		self._dds_dir().mkdir(parents=True, exist_ok=True)
		threading.Thread(target=self.reconcile, daemon=True).start()

	def _tex_sources(self):
		exclusion_rules = app_settings.load().exclusion_rules
		return (
			path for path in iter_included_files(self._workspace_dir, exclusion_rules)
			if path.suffix.lower() in TEX_EXTENSIONS)

	def reconcile(self):
		"""Manual/startup catch-up: scans the whole workspace for tex
		sources, flags any stem collision (via DuplicateNameGuard, before
		converting anything for a colliding pair), regenerates every
		conflict-free source whose build/dds/ counterpart is missing or
		older, and removes every build/dds/ file whose source is gone or
		conflicting. Runs on a background thread (like
		WorkspaceSyncWatcher.sync_now()) since a workspace can hold a lot of
		files."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		dds_dir = self._dds_dir()
		dds_dir.mkdir(parents=True, exist_ok=True)

		safe_sources = self._guard.scan(self._tex_sources())
		expected_dds_paths = set()
		for tex_path in safe_sources:
			dds_path = dds_dir / f"{tex_path.stem}.dds"
			expected_dds_paths.add(dds_path)
			if not dds_path.is_file() or tex_path.stat().st_mtime > dds_path.stat().st_mtime:
				self._convert(tex_path, dds_path)

		for dds_path in dds_dir.glob("*.dds"):
			if dds_path.is_file() and dds_path not in expected_dds_paths:
				self._remove(dds_path)

	def handle_settled(self, tex_path):
		"""Registered onto a shared WorkspaceWatcher via
		register_extension() for TEX_EXTENSIONS -- runs off that
		watcher's background thread."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		dds_path = self._dds_dir() / f"{tex_path.stem}.dds"

		if not tex_path.is_file():
			survivor = self._guard.remove(tex_path)
			self._remove(dds_path)
			if survivor is not None:
				self._convert(survivor, self._dds_dir() / f"{survivor.stem}.dds")
			return

		if self._guard.update(tex_path):
			self._convert(tex_path, dds_path)

	def _convert(self, tex_path, dds_path):
		try:
			rgba = load_rgba(str(tex_path))
			algo = pick_default_algo(rgba)
			data = build_dds(rgba, algo, build_mipmaps=True)
			dds_path.parent.mkdir(parents=True, exist_ok=True)
			dds_path.write_bytes(data)
		except Exception as exc:  # noqa: BLE001 -- a single bad texture must not kill the watcher/reconcile pass
			message = f"failed to convert {tex_path.name} to DDS: {exc}"
			print(f"[tex_dds_sync] {message}")
			self._on_status(message, True)
		else:
			print(f"[tex_dds_sync] {tex_path.name} -> {dds_path}")
			self._on_status(f"{tex_path.name} -> build/dds/", False)

	def _remove(self, dds_path):
		try:
			dds_path.unlink()
		except FileNotFoundError:
			pass
		except OSError as exc:
			print(f"[tex_dds_sync] failed to remove {dds_path}: {exc}")
		else:
			print(f"[tex_dds_sync] removed {dds_path}")
