"""Watches a workspace's `imports/` folder for new/changed `.obj`/`.dae`/`.fbx`
source meshes and keeps `<workspace>/shapes/<name>.shape` in sync
automatically -- see the "Auto-export imports/ -> shapes/" chantier in
`project-todos/ryzom-core/forgery-object-editor.md`.
"""

import re
import threading
from pathlib import Path

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer

from ryzom_forgery.shape_import import ShapeImportError, find_importer

from pynel.ryzom_shape import ShapeFile, ShapeParseError, ShapeWriteError, Texture, parse_shape, save_shape

# Same value as search_paths_dialog.py's own workspace watch -- an
# inactivity debounce (the timer restarts on every event), so it's robust
# regardless of how long a given source file takes to finish writing.
_WATCH_DEBOUNCE_SECONDS = 0.5

_RELEVANT_EVENT_TYPES = {"created", "modified", "moved"}

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
	number of materials -- needs the interactive matching popup (see the
	chantier's Step 4), left to the caller to route there."""


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
	for existing_material, imported_material in zip(existing_materials, mesh.materials):
		_update_diffuse_texture(existing_material, imported_material)

	save_shape(target_path, shape_file)


class _DebouncedImportHandler(FileSystemEventHandler):
	"""Same coalescing idea as search_paths_dialog.py's own
	_DebouncedReloadHandler, but keyed per source file rather than firing one
	shared callback for the whole watched folder: each mesh's own burst of
	write events is debounced independently, so editing one file doesn't
	reset another's pending timer. watchdog calls on_any_event() from its own
	OS-native watcher thread, never the main/render thread -- same as any
	other background worker here."""

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
		if find_importer(path) is None:
			return
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


class ImportWatcher:
	"""Watches the active workspace's `imports/` folder and keeps `shapes/`
	in sync -- see this module's docstring. Call `set_workspace_dir()`
	whenever the active workspace changes (see object_editor.py); pass None
	to stop watching."""

	def __init__(self, is_shape_open=None, on_open_shape_conflict=None):
		"""Both hooks run off the main/render thread (see _handle_settled()),
		and are optional -- without them (e.g. test.sh's headless use), an
		existing target is always updated in place directly.

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
		object_editor.py's _on_open_shape_conflict())."""
		self._is_shape_open = is_shape_open
		self._on_open_shape_conflict = on_open_shape_conflict
		self._observer = None
		self._workspace_dir = None

	def set_workspace_dir(self, workspace_dir):
		"""Stops any previous watch and, if `workspace_dir` is set, starts a
		fresh one on its `imports/` subfolder -- same native-OS-API-via-
		watchdog approach, and same silently-give-up-on-failure reasoning, as
		search_paths_dialog.py's own workspace watch."""
		if self._observer is not None:
			self._observer.stop()
			self._observer = None
		self._workspace_dir = Path(workspace_dir) if workspace_dir is not None else None
		if self._workspace_dir is None:
			return

		imports_dir = self._workspace_dir / "imports"
		imports_dir.mkdir(parents=True, exist_ok=True)
		handler = _DebouncedImportHandler(self._handle_settled)
		observer = Observer()
		try:
			observer.schedule(handler, str(imports_dir), recursive=True)
			observer.daemon = True
			observer.start()
		except OSError as exc:
			print(f"[import_watcher] could not watch {imports_dir!r}: {exc}")
			return
		self._observer = observer

	def _handle_settled(self, source_path):
		"""Runs off the main/render thread (see _DebouncedImportHandler).
		Step 4 of the chantier still needs to add the material-count-mismatch
		path (the interactive matching popup) -- until then, a mismatch is
		left untouched."""
		workspace_dir = self._workspace_dir
		if workspace_dir is None:
			return
		target_path = target_shape_path(workspace_dir, source_path)

		if not target_path.exists():
			try:
				export_new_shape(source_path, target_path)
			except (OSError, ShapeImportError, ShapeWriteError) as exc:
				print(f"[import_watcher] auto-export of {source_path.name} failed: {exc}")
			else:
				print(f"[import_watcher] auto-exported {source_path.name} -> {target_path}")
			return

		if self._is_shape_open is not None and self._is_shape_open(target_path):
			if self._on_open_shape_conflict is not None:
				self._on_open_shape_conflict(source_path, target_path)
			return

		self._update_existing_target(source_path, target_path)

	def _update_existing_target(self, source_path, target_path):
		"""The actual `update_existing_shape()` call + outcome logging,
		factored out so both the automatic path (_handle_settled(), when the
		target isn't open in the viewport) and the host app's own conflict
		resolution (object_editor.py's _on_open_shape_conflict(), once the
		user picked how to proceed) can trigger it."""
		try:
			update_existing_shape(source_path, target_path)
		except MaterialCountMismatch as exc:
			print(f"[import_watcher] {target_path.name}: {exc} -- "
			      f"needs manual matching in Patina (Step 4 not implemented yet)")
		except UnsupportedShapeTypeError as exc:
			print(f"[import_watcher] {target_path.name}: {exc}")
		except (OSError, ShapeImportError, ShapeParseError, ShapeWriteError) as exc:
			print(f"[import_watcher] auto-update of {target_path.name} failed: {exc}")
		else:
			print(f"[import_watcher] auto-updated {target_path.name} from {source_path.name}")
