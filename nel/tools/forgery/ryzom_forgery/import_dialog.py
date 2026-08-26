"""Import flow UI for .obj/.dae/.fbx -> .shape: asks for a source mesh file
(native dialog), then how to bring it in -- as a brand-new shape, or
replacing the currently open one's geometry while keeping its materials
(and any edits made to them: blend/alpha-test/2-sided/Multi Bitmap...).
"""

from pathlib import Path

from imgui_bundle import imgui, portable_file_dialogs as pfd

from ryzom_forgery.shape_import import ShapeImportError, find_importer

_MODE_POPUP_ID = "Import mesh"


class ImportDialog:
	"""`on_new_shape(mesh, source_path)` and `on_replace(mesh, source_path)`
	are called with the parsed `pynel.ryzom_shape.Mesh` once the user picks a
	mode in the popup -- `source_path` is the imported .obj/.dae/.fbx's own
	path, handed back so the new-shape flow can default its name/save
	location to it, and both flows can fall back to looking for the mesh's
	textures next to it (see object_editor.py's _texture_search_dirs)."""

	def __init__(self, on_new_shape, on_replace):
		self._on_new_shape = on_new_shape
		self._on_replace = on_replace

		self._file_dialog = None
		self._pending_mesh = None  # parsed Mesh, waiting on the mode popup
		self._pending_path = None  # the source file it was parsed from
		self._has_current_shape = False
		self._status = ""

	def open(self, has_current_shape):
		"""Opens a native file picker for a source .obj/.dae/.fbx. `has_current_shape`
		gates whether "Replace in current shape" is offered in the mode popup."""
		self._has_current_shape = has_current_shape
		self._file_dialog = pfd.open_file("Import mesh", "", ["Mesh files", "*.obj *.dae *.fbx"])

	def draw(self):
		"""Call once per ImGui frame."""
		self._poll_file_dialog()
		self._draw_mode_popup()

	def _poll_file_dialog(self):
		if self._file_dialog is None or not self._file_dialog.ready(0):
			return
		dialog, self._file_dialog = self._file_dialog, None
		result = dialog.result()
		if not result:
			return

		path = Path(result[0])
		importer = find_importer(path)
		if importer is None:
			self._status = f"Unsupported mesh format: {path.suffix!r}"
			return

		try:
			self._pending_mesh = importer(path)
		except (OSError, ShapeImportError) as exc:
			self._status = f"Import failed: {exc}"
			return

		self._pending_path = path
		self._status = ""
		imgui.open_popup(_MODE_POPUP_ID)

	def _draw_mode_popup(self):
		if self._pending_mesh is None:
			return

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_MODE_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text(f"{len(self._pending_mesh.materials)} material(s) in the imported mesh.")
		imgui.separator()

		if imgui.button("Import as new shape"):
			mesh, self._pending_mesh = self._pending_mesh, None
			path, self._pending_path = self._pending_path, None
			imgui.close_current_popup()
			self._on_new_shape(mesh, path)

		imgui.begin_disabled(not self._has_current_shape)
		if imgui.button("Replace in current shape"):
			mesh, self._pending_mesh = self._pending_mesh, None
			path, self._pending_path = self._pending_path, None
			imgui.close_current_popup()
			self._on_replace(mesh, path)
		imgui.end_disabled()

		imgui.begin_disabled(True)
		imgui.button("Add to current shape (coming soon)")
		imgui.end_disabled()

		imgui.separator()
		if imgui.button("Cancel"):
			self._pending_mesh = None
			self._pending_path = None
			imgui.close_current_popup()

		imgui.end_popup()
