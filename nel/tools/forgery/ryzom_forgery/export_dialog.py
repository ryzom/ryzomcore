"""Export flow UI for .shape -> .obj/.dae/.stl/.gltf/.glb: asks for an
output folder (native dialog) every time, then, for formats that carry
materials, how to handle textures -- no "remember this choice" shortcut
(removed in the patina-export-rework chantier: it was a single setting
shared across every workspace, so it stuck regardless of which workspace
was active, which read as the export folder being "stuck" to one place).
"""

import zipfile
from pathlib import Path

from imgui_bundle import imgui, portable_file_dialogs as pfd

from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.settings import TEXTURE_MODE_COPY_PNG, TEXTURE_MODE_REFERENCE_ONLY
from ryzom_forgery.shape_export import export_shape
from ryzom_forgery.workspace_sync import pack_workspace_bnp

_CONFIRM_POPUP_ID = "Export"

_COPY_TEXTURES_COLOR = (0.6, 0.9, 0.6, 1.0)  # light green
_ZIP_TEXTURES_COLOR = (0.6, 0.9, 0.9, 1.0)  # light cyan
_NO_TEXTURES_COLOR = (0.95, 0.65, 0.8, 1.0)  # pink
_PLAIN_EXPORT_COLOR = (0.75, 0.75, 0.75, 1.0)  # gray -- formats with no materials at all (STL)


def _colored_button(label, color):
	"""Same tinted-button-with-black-text pattern as apps/object_editor.py's
	own _colored_button() -- each module keeps its own tiny copy, matching
	how this codebase already does it (see e.g. search_paths_dialog.py's
	_icon_button)."""
	r, g, b, a = color
	imgui.push_style_color(imgui.Col_.button.value, color)
	imgui.push_style_color(imgui.Col_.button_hovered.value, (min(r + 0.1, 1.0), min(g + 0.1, 1.0), min(b + 0.1, 1.0), a))
	imgui.push_style_color(imgui.Col_.button_active.value, (max(r - 0.1, 0.0), max(g - 0.1, 0.0), max(b - 0.1, 0.0), a))
	imgui.push_style_color(imgui.Col_.text.value, (0.0, 0.0, 0.0, 1.0))
	clicked = imgui.button(label)
	imgui.pop_style_color(4)
	return clicked


def _center_next_widget(width):
	"""Same centering helper as apps/object_editor.py's own
	_center_next_widget()."""
	avail = imgui.get_content_region_avail().x
	if avail > width:
		imgui.set_cursor_pos_x(imgui.get_cursor_pos_x() + (avail - width) / 2)


class ExportDialog:
	def __init__(self):
		self._pending = None  # dict, set by export(), cleared once resolved/cancelled
		self._folder_dialog = None  # active portable_file_dialogs.select_folder, for an in-flight export
		self._confirm_open = False
		self._status = ""

	def export(self, shape_value, name, export_format, texture_finder, source_folder=None):
		"""Starts exporting `shape_value` (the shape currently open in the
		editor, possibly edited -- see object_editor.py's Export button in
		the bottom bar) via `export_format`. `name` supplies the output
		file's stem. Always asks for an output folder, then (for formats
		that carry materials) how to handle textures."""
		self._pending = {
			"kind": "shape",
			"shape_value": shape_value, "name": name, "format": export_format, "texture_finder": texture_finder,
		}
		self._folder_dialog = pfd.select_folder("Choose export folder", str(source_folder or ""))

	def export_workspace_bnp(self, workspace_dir, source_folder=None):
		""""Full workspace" format choice in the format-picking menu (see
		apps/object_editor.py) -- asks for an output folder like a normal
		shape export, but skips the texture-handling popup entirely (a
		workspace .bnp has no per-texture choice to make, see
		pack_workspace_bnp())."""
		self._pending = {"kind": "bnp", "workspace_dir": workspace_dir}
		self._folder_dialog = pfd.select_folder("Choose export folder", str(source_folder or ""))

	def quick_export_workspace_bnp(self, workspace_dir, output_dir):
		"""Workspace-only fast path (see quick_export()'s docstring) for the
		"Full workspace" format choice -- no folder prompt, writes straight
		to `output_dir` (always `<workspace>/exports/`)."""
		self._write_workspace_bnp(workspace_dir, output_dir)

	def draw(self):
		"""Call once per ImGui frame."""
		self._poll_folder_dialog()
		self._draw_confirmation_popup()

	def _poll_folder_dialog(self):
		if self._folder_dialog is None or not self._folder_dialog.ready(0):
			return
		result = self._folder_dialog.result()
		self._folder_dialog = None
		if not result:
			self._pending = None
			return
		self._pending["folder"] = result
		if self._pending["kind"] == "bnp":
			pending, self._pending = self._pending, None
			self._write_workspace_bnp(pending["workspace_dir"], pending["folder"])
			return
		self._confirm_open = True
		imgui.open_popup(_CONFIRM_POPUP_ID)

	def _draw_confirmation_popup(self):
		if not self._confirm_open:
			return

		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_CONFIRM_POPUP_ID, None, flags)
		if not opened:
			return

		pending = self._pending
		output_name = f"{Path(pending['name']).stem}.{pending['format'].extension}"
		frame_padding_x2 = imgui.get_style().frame_padding.x * 2

		imgui.text(output_name)
		imgui.text(pending["folder"])
		imgui.separator()

		if pending["format"].supports_materials:
			copy_label = "Copy textures with export"
			zip_label = "Make a Zip archive with textures"
			no_textures_label = "Export without the textures"

			_center_next_widget(imgui.calc_text_size(copy_label).x + frame_padding_x2)
			if _colored_button(copy_label, _COPY_TEXTURES_COLOR):
				self._start_export(TEXTURE_MODE_COPY_PNG, make_zip=False)
			_center_next_widget(imgui.calc_text_size(zip_label).x + frame_padding_x2)
			if _colored_button(zip_label, _ZIP_TEXTURES_COLOR):
				self._start_export(TEXTURE_MODE_COPY_PNG, make_zip=True)
			_center_next_widget(imgui.calc_text_size(no_textures_label).x + frame_padding_x2)
			if _colored_button(no_textures_label, _NO_TEXTURES_COLOR):
				self._start_export(TEXTURE_MODE_REFERENCE_ONLY, make_zip=False)
		else:
			# No materials at all (STL is geometry-only) -- nothing to ask about textures.
			export_label = "Export"
			_center_next_widget(imgui.calc_text_size(export_label).x + frame_padding_x2)
			if _colored_button(export_label, _PLAIN_EXPORT_COLOR):
				self._start_export(TEXTURE_MODE_REFERENCE_ONLY, make_zip=False)

		imgui.separator()
		cancel_label = "Cancel"
		_center_next_widget(imgui.calc_text_size(cancel_label).x + frame_padding_x2)
		if imgui.button(cancel_label):
			self._confirm_open = False
			self._pending = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _start_export(self, texture_mode, make_zip):
		self._pending["texture_mode"] = texture_mode
		self._pending["zip"] = make_zip
		self._confirm_open = False
		imgui.close_current_popup()
		self._run_export()

	def _run_export(self):
		pending, self._pending = self._pending, None
		try:
			written = export_shape(
				pending["shape_value"], pending["name"], pending["format"], pending["folder"],
				pending["texture_mode"], pending["texture_finder"])
			if pending["zip"]:
				written = self._zip_written(pending["name"], pending["format"], pending["folder"], written)
			self._status = f"Exported {len(written)} file(s) to {pending['folder']}"
			print(f"[export] {self._status}: {[str(path) for path in written]}")
		except Exception as exc:
			self._status = f"Export failed: {exc}"
			print(f"[export] {self._status}")

	def quick_export(self, shape_value, name, export_format, texture_finder, output_dir):
		"""Workspace-only fast path (see apps/object_editor.py's bottom-bar
		[Export] button, to the left of [Export as...]): writes straight to
		`output_dir` (always `<workspace>/exports/`) with no folder prompt and
		no texture-handling popup -- always TEXTURE_MODE_REFERENCE_ONLY, since
		a workspace's textures already live locally (`tex/`/`dds/`) or as an
		absolute path outside the workspace, so there's nothing to copy."""
		try:
			written = export_shape(shape_value, name, export_format, output_dir, TEXTURE_MODE_REFERENCE_ONLY, texture_finder)
			self._status = f"Exported {len(written)} file(s) to {output_dir}"
			print(f"[export] {self._status}: {[str(path) for path in written]}")
		except Exception as exc:
			self._status = f"Export failed: {exc}"
			print(f"[export] {self._status}")

	def _write_workspace_bnp(self, workspace_dir, output_dir):
		bnp_path = Path(output_dir) / f"{Path(workspace_dir).name}.bnp"
		try:
			pack_workspace_bnp(workspace_dir, bnp_path)
			self._status = f"Exported workspace to {bnp_path}"
			print(f"[export] {self._status}")
		except Exception as exc:
			self._status = f"Export failed: {exc}"
			print(f"[export] {self._status}")

	def _zip_written(self, name, export_format, folder, written):
		"""Replaces the loose exported files with a single .zip containing
		them all -- reuses the normal TEXTURE_MODE_COPY_PNG export path
		(so textures are physically written first) then archives + removes
		the loose copies, rather than a separate from-scratch zip-writing
		export path."""
		zip_path = Path(folder) / f"{Path(name).stem}.zip"
		with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as archive:
			for path in written:
				archive.write(path, arcname=Path(path).name)
		for path in written:
			Path(path).unlink(missing_ok=True)
		return [zip_path]
