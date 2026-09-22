"""Keeps an active workspace's dds/ subfolder automatically in sync with its
tex/ subfolder: every TGA/PNG source in tex/ gets a matching .dds in dds/,
regenerated whenever the source is created/modified, and removed whenever
the source is deleted -- see the "patina-tex-dds-autoexport" chantier in
project-todos/ryzom-core/patina-tex-dds-autoexport.md.

Doesn't own its own filesystem watch -- `handle_settled()` is meant to be
registered onto a shared `workspace_watch.WorkspaceWatcher` for the "tex"
subfolder (see apps/object_editor.py). See workspace_watch.py's module
docstring for why this was consolidated out of a dedicated Observer per
feature.

Specular textures are out of scope for now: Patina doesn't manage texture
roles/slots yet, so tex/ never contains a recognizable specular file in
practice. Every supported file in tex/ is converted uniformly, including
Multi Bitmap season/quality variants (this is a plain per-file mirror, not
aware of which shape/material actually uses a given texture).

DDS export always builds mipmaps (these are always 3D model textures, see
dds_export.py's own docstring on why an asset regenerated without mipmaps
looks visibly worse than the original).
"""

import threading
from pathlib import Path

from ryzom_forgery.dds_export import load_rgba, build_dds, pick_default_algo

# Same scope as tga2dds.cpp/dds_export.py -- not explorer.py's broader
# _TEXTURE_EXTENSIONS (which also lists .dds/.jpg/.jpeg/.bmp, formats this
# pipeline was never meant to read as a *source*).
_SUPPORTED_EXTENSIONS = {".tga", ".png"}


def _dds_path_for(tex_root, dds_root, tex_path):
	return dds_root / tex_path.relative_to(tex_root).with_suffix(".dds")


class TexDdsSyncWatcher:
	"""Mirrors the active workspace's tex/ subfolder into dds/, converting
	every TGA/PNG to a .dds with mipmaps. A no-op end to end without an
	active workspace.

	Call set_workspace_dir() whenever the active workspace changes (pass
	None to stop tracking) -- this also kicks off a background reconcile()
	pass, since a workspace closed then reopened (or a tex/ file edited by
	hand outside Patina) can have drifted out of sync while nothing was
	watching. Doesn't watch the filesystem itself -- register
	handle_settled onto a shared WorkspaceWatcher for the "tex" subfolder
	instead."""

	def __init__(self, on_status=None):
		self._workspace_dir = None
		self._on_status = on_status or (lambda message, is_error=False: None)

	def set_workspace_dir(self, workspace_dir):
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		if self._workspace_dir is None:
			return
		(self._workspace_dir / "tex").mkdir(parents=True, exist_ok=True)
		(self._workspace_dir / "dds").mkdir(parents=True, exist_ok=True)
		threading.Thread(target=self.reconcile, daemon=True).start()

	def reconcile(self):
		"""Manual/startup catch-up: regenerates every tex/ file whose dds/
		counterpart is missing or older, and removes every dds/ file whose
		tex/ source is gone -- for whatever changed while nothing was
		watching. Runs on a background thread (like
		WorkspaceSyncWatcher.sync_now()) since a workspace can hold a lot of
		files."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		tex_dir = workspace_dir / "tex"
		dds_dir = workspace_dir / "dds"
		dds_dir.mkdir(parents=True, exist_ok=True)
		if not tex_dir.is_dir():
			return

		expected_dds_paths = set()
		for tex_path in tex_dir.rglob("*"):
			if not tex_path.is_file() or tex_path.suffix.lower() not in _SUPPORTED_EXTENSIONS:
				continue
			dds_path = _dds_path_for(tex_dir, dds_dir, tex_path)
			expected_dds_paths.add(dds_path)
			if not dds_path.is_file() or tex_path.stat().st_mtime > dds_path.stat().st_mtime:
				self._convert(tex_path, dds_path)

		for dds_path in dds_dir.rglob("*.dds"):
			if dds_path.is_file() and dds_path not in expected_dds_paths:
				self._remove(dds_path)

	def handle_settled(self, tex_path):
		"""Registered onto a shared WorkspaceWatcher for the "tex"
		subfolder -- runs off that watcher's background thread."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		tex_dir = workspace_dir / "tex"
		dds_dir = workspace_dir / "dds"
		try:
			tex_path.relative_to(tex_dir)
		except ValueError:
			return  # shouldn't happen -- watched paths are always under tex_dir
		if tex_path.suffix.lower() not in _SUPPORTED_EXTENSIONS:
			return  # not a source format this pipeline reads -- silently ignored, same as explorer's own texture filters

		dds_path = _dds_path_for(tex_dir, dds_dir, tex_path)
		if tex_path.is_file():
			self._convert(tex_path, dds_path)
		else:
			self._remove(dds_path)

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
			self._on_status(f"{tex_path.name} -> dds/", False)

	def _remove(self, dds_path):
		try:
			dds_path.unlink()
		except FileNotFoundError:
			pass
		except OSError as exc:
			print(f"[tex_dds_sync] failed to remove {dds_path}: {exc}")
		else:
			print(f"[tex_dds_sync] removed {dds_path}")
