"""Watches a workspace's `imports/` folder for new/changed source meshes
(any format `shape_import.IMPORTERS` knows -- .obj/.dae/.fbx/.gltf/.glb) and
keeps `<workspace>/shapes/<name>.shape` in sync
automatically -- see the "Auto-export imports/ -> shapes/" chantier in
`project-todos/ryzom-core/forgery-object-editor.md`.

Doesn't own its own filesystem watch -- `handle_settled()` is meant to be
registered onto a shared `workspace_watch.WorkspaceWatcher` for the
"imports" subfolder (see apps/object_editor.py). See workspace_watch.py's
module docstring for why this was consolidated out of a dedicated Observer
per feature.
"""

import re
from datetime import datetime
from pathlib import Path

from ryzom_forgery.shape_geometry import IDENTITY_QUAT
from ryzom_forgery.shape_import import IMPORTERS, ShapeImportError, find_importer

from pynel.ryzom_shape import ShapeFile, ShapeParseError, ShapeWriteError, Texture, parse_shape, save_shape

# Only letters, digits, '_' and '-' survive; everything else (spaces, accents,
# punctuation...) becomes '_'. Case is kept as-is.
_INVALID_NAME_CHARS = re.compile(r"[^A-Za-z0-9_-]")


class UnsupportedShapeTypeError(Exception):
	"""The existing target shape isn't a plain `Mesh` -- same restriction as
	Patina's own manual Import -> Replace (`_on_import_replace()`): only a
	`CMesh`'s geometry can be swapped this way (see shape_import.py's module
	docstring)."""


class MaterialCountMismatch(Exception):
	"""The imported mesh and the existing target shape don't have the same
	number of materials -- left to the caller to route to
	ImportWatcher._backup_and_reexport()."""


def sanitize_shape_name(stem: str) -> str:
	"""A source mesh file's stem, turned into a safe `.shape` base name."""
	return _INVALID_NAME_CHARS.sub("_", stem)


def target_shape_path(workspace_dir: Path, source_path: Path) -> Path:
	"""Where a source mesh sitting in `<workspace_dir>/imports/` auto-exports
	to: `<workspace_dir>/shapes/<sanitized name>.shape`."""
	return workspace_dir / "shapes" / f"{sanitize_shape_name(source_path.stem)}.shape"


def export_new_shape(source_path: Path, target_path: Path) -> None:
	"""Full headless import of `source_path` into `target_path`: used when
	`target_path` doesn't exist yet, so there's no existing materials/edits to
	preserve -- same path as `apps/shape_importer.py`'s CLI. Raises
	`ShapeImportError` for an unsupported extension or a malformed source
	file, `OSError`/`ShapeWriteError` for a write failure -- left to the
	caller to catch and report (see the chantier's Step 5)."""
	importer = find_importer(source_path)
	if importer is None:
		raise ShapeImportError(f"unsupported mesh format: {source_path.suffix!r}")
	mesh = importer(source_path)
	target_path.parent.mkdir(parents=True, exist_ok=True)
	save_shape(target_path, ShapeFile(type_name="Mesh", value=mesh))


def _update_diffuse_texture(existing_material, imported_material) -> None:
	"""Overwrites `existing_material`'s stage-0 (diffuse) texture reference
	with `imported_material`'s own, but only if the file name actually
	differs -- every other material field (blend, alpha-test, 2-sided,
	tex_envs, any further Multi Bitmap stage...) is left as-is, so edits made
	in Patina survive. Matches shape_import.py's own "only the diffuse
	texture reference carries over from the source file" convention: an
	imported mesh's materials never carry more than that one stage."""
	imported_textures = imported_material.textures
	imported_tex = imported_textures[0] if imported_textures else None
	imported_name = imported_tex.file_name if imported_tex is not None else None
	if not imported_name:
		return

	if not existing_material.textures:
		existing_material.textures.append(None)
	existing_tex = existing_material.textures[0]
	existing_name = existing_tex.file_name if existing_tex is not None else None
	if existing_name == imported_name:
		return

	if existing_tex is not None:
		existing_tex.file_name = imported_name
	else:
		existing_material.textures[0] = Texture(class_name="CTextureFile", file_name=imported_name)


def update_existing_shape(source_path: Path, target_path: Path) -> None:
	"""Updates `target_path`'s geometry (and, per-material, its diffuse
	texture reference where changed) from `source_path`, leaving every other
	material edit made in Patina untouched -- used when `target_path` already
	exists. Raises `ShapeParseError` for a malformed existing shape,
	`ShapeImportError` for an unsupported/malformed source mesh,
	`UnsupportedShapeTypeError` if the existing shape isn't a plain `Mesh`,
	`MaterialCountMismatch` if the two don't have the same number of
	materials (see the chantier's Step 4) -- all left to the caller to catch
	and report."""
	shape_file = parse_shape(target_path.read_bytes())
	if shape_file.type_name != "Mesh":
		raise UnsupportedShapeTypeError(
			f"{target_path.name} is a {shape_file.type_name!r}, only a plain Mesh can be auto-updated")

	importer = find_importer(source_path)
	if importer is None:
		raise ShapeImportError(f"unsupported mesh format: {source_path.suffix!r}")
	mesh = importer(source_path)

	existing_materials = shape_file.value.materials
	if len(mesh.materials) != len(existing_materials):
		raise MaterialCountMismatch(
			f"imported mesh has {len(mesh.materials)} material(s), {target_path.name} has "
			f"{len(existing_materials)}")

	shape_file.value.geom = mesh.geom
	# The imported mesh's own default_rot_quat is identity (a fresh import
	# always is), but the *export* that produced source_path already baked
	# the target's own default_rot_quat into its vertices (see
	# shape_export.py's export_shape()) -- so the new geometry here already
	# represents the correct final orientation. Resetting the target's
	# default_rot_quat to identity avoids applying that rotation a second
	# time on top of already-rotated geometry.
	base = getattr(shape_file.value, "base", None)
	if base is not None:
		base.default_rot_quat = IDENTITY_QUAT
	for existing_material, imported_material in zip(existing_materials, mesh.materials):
		_update_diffuse_texture(existing_material, imported_material)

	save_shape(target_path, shape_file)


class ImportWatcher:
	"""Keeps the active workspace's `shapes/` in sync with its `imports/`
	folder -- see this module's docstring. Call `set_workspace_dir()`
	whenever the active workspace changes (see object_editor.py); pass None
	to stop tracking. Doesn't watch the filesystem itself -- register
	`handle_settled` onto a shared `workspace_watch.WorkspaceWatcher` for
	the "imports" subfolder instead."""

	def __init__(self, is_shape_open=None, on_open_shape_conflict=None, on_status=None):
		"""All three hooks run off whatever thread calls handle_settled()
		(the shared WorkspaceWatcher's background thread in practice), and
		are optional -- without them (e.g. test.sh's headless use), an
		existing target is always updated in place directly and outcomes are
		only printed.

		`is_shape_open(target_path)`, if given, is consulted before an
		existing target is touched: True means the host app currently has
		that exact shape open in its viewport, with in-memory edits that
		could be lost by silently overwriting it (Patina has no dirty-edit
		tracking to know whether that's actually the case -- see the chantier
		discussion) -- routes to `on_open_shape_conflict` instead of
		auto-updating.

		`on_open_shape_conflict(source_path, target_path)`, if given, is
		called instead of the automatic update whenever `is_shape_open`
		returned True -- the host app's hook to ask the user how to proceed
		(save first / discard / save-as-backup-first -- see
		object_editor.py's _on_open_shape_conflict()).

		`on_status(message, is_error)`, if given, is called for every outcome
		(new shape written, updated, backed up and re-exported, or a failure)
		alongside the unconditional `print()` -- the host app's hook to
		surface the same message in its own UI (see object_editor.py's
		_on_import_status())."""
		self._is_shape_open = is_shape_open
		self._on_open_shape_conflict = on_open_shape_conflict
		self._on_status = on_status
		self._workspace_dir = None

	def _report(self, message, is_error=False):
		print(f"[import_watcher] {message}")
		if self._on_status is not None:
			self._on_status(message, is_error)

	def set_workspace_dir(self, workspace_dir):
		"""Tracks the active workspace and ensures its `imports/` subfolder
		exists. Pass None when no workspace is active."""
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		if self._workspace_dir is not None:
			(self._workspace_dir / "imports").mkdir(parents=True, exist_ok=True)

	def handle_settled(self, source_path):
		"""Registered onto a shared WorkspaceWatcher for the "imports"
		subfolder -- runs off that watcher's background thread. Silently
		ignores files that no longer exist (deleted before/while the
		debounce settled -- imports/ is a one-way staging area, nothing to
		clean up on delete, unlike tex/'s dds/ mirror). Reports (not
		silently ignores) a file whose format find_importer() doesn't know
		how to import (see IMPORTERS in shape_import.py) -- dropping
		one in imports/ used to do nothing with no feedback at all."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		if not source_path.is_file():
			return
		if find_importer(source_path) is None:
			self._report(
				f"{source_path.name}: unsupported import format {source_path.suffix!r} "
				f"(supported: {', '.join(sorted(IMPORTERS))})", is_error=True)
			return
		target_path = target_shape_path(workspace_dir, source_path)

		if not target_path.exists():
			try:
				export_new_shape(source_path, target_path)
			except (OSError, ShapeImportError, ShapeWriteError) as exc:
				self._report(f"auto-export of {source_path.name} failed: {exc}", is_error=True)
			else:
				self._report(f"auto-exported {source_path.name} -> {target_path.name}")
			return

		if self._is_shape_open is not None and self._is_shape_open(target_path):
			if self._on_open_shape_conflict is not None:
				self._on_open_shape_conflict(source_path, target_path)
			return

		self._update_existing_target(source_path, target_path)

	def _update_existing_target(self, source_path, target_path):
		"""The actual `update_existing_shape()` call + outcome logging,
		factored out so both the automatic path (handle_settled(), when the
		target isn't open in the viewport) and the host app's own conflict
		resolution (object_editor.py's _on_open_shape_conflict(), once the
		user picked how to proceed) can trigger it. Returns True if
		target_path ended up rewritten (a straight update or a
		backup-and-reexport), False on any failure -- so a caller that also
		needs to refresh an in-memory copy (object_editor.py's
		_apply_import_conflict_update()) knows whether target_path is
		actually safe to re-read."""
		try:
			update_existing_shape(source_path, target_path)
		except MaterialCountMismatch as exc:
			self._report(f"{target_path.name}: {exc} -- backing up and re-exporting", is_error=True)
			return self._backup_and_reexport(source_path, target_path)
		except UnsupportedShapeTypeError as exc:
			self._report(f"{target_path.name}: {exc}", is_error=True)
			return False
		except (OSError, ShapeImportError, ShapeParseError, ShapeWriteError) as exc:
			self._report(f"auto-update of {target_path.name} failed: {exc}", is_error=True)
			return False
		else:
			self._report(f"auto-updated {target_path.name} from {source_path.name}")
			return True

	def _backup_and_reexport(self, source_path, target_path):
		"""Material-count mismatch fallback: rather than an interactive
		matching popup, the existing target is preserved as a timestamped
		backup (same `<stem>_backup_<YYYYMMDD_HHMMSS><suffix>` naming as the
		manual Save-as-backup flow in object_editor.py) and the newly imported
		mesh is exported fresh under the real target name -- fully automatic,
		no viewport takeover, no popup. Returns True/False, see
		_update_existing_target()."""
		timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
		backup_path = target_path.with_name(f"{target_path.stem}_backup_{timestamp}{target_path.suffix}")
		try:
			target_path.rename(backup_path)
			export_new_shape(source_path, target_path)
		except (OSError, ShapeImportError, ShapeWriteError) as exc:
			self._report(f"backup-and-reexport of {target_path.name} failed: {exc}", is_error=True)
			return False
		else:
			self._report(f"backed up {target_path.name} -> {backup_path.name}, "
			             f"re-exported from {source_path.name}")
			return True
