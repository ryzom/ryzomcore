"""Export flow UI for .shape -> .obj/.dae/.stl: asks for an output folder
(native dialog) and, for formats that carry materials, how to handle
textures -- each with a "remember this choice" checkbox that persists into
`export_config`. A separate always-available settings window lets those
remembered choices be reviewed/changed without triggering an export.
"""

from imgui_bundle import imgui, portable_file_dialogs as pfd

from ryzom_forgery import export_config
from ryzom_forgery.export_config import TEXTURE_MODE_COPY_PNG, TEXTURE_MODE_REFERENCE_ONLY
from ryzom_forgery.shape_export import export_shape

_CONFIRM_POPUP_ID = "Export"


class ExportDialog:
	def __init__(self):
		self._config = export_config.load()

		self._pending = None  # dict, set by export(), cleared once resolved/cancelled
		self._folder_dialog = None  # active portable_file_dialogs.select_folder, for an in-flight export

		self._confirm_open = False
		self._confirm_remember_folder = False
		self._confirm_texture_mode = self._config.texture_mode
		self._confirm_remember_texture = False

		self._settings_folder_dialog = None  # separate instance, independent of an in-flight export

		self._status = ""

	def export(self, shape_value, name, export_format, asset_index, source_folder=None):
		"""Starts exporting `shape_value` (the shape currently open in the
		editor, possibly edited -- see object_editor.py's Export button in
		the bottom bar) via `export_format`. `name` supplies the output
		file's stem. Asks for an output folder (and, if applicable, texture
		handling) unless those are already remembered."""
		self._pending = {
			"shape_value": shape_value, "name": name, "format": export_format, "asset_index": asset_index,
			"folder_just_picked": False,
		}
		if self._config.remember_output_folder and self._config.output_folder:
			self._pending["folder"] = self._config.output_folder
			self._open_confirmation_if_needed()
		else:
			self._folder_dialog = pfd.select_folder("Choose export folder", str(source_folder or ""))

	def draw(self):
		"""Call once per ImGui frame -- everything here except
		draw_settings_content() (embedded by the caller in its own Settings
		tab, see that method's docstring)."""
		self._poll_folder_dialog()
		self._poll_settings_folder_dialog()
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
		self._pending["folder_just_picked"] = True
		self._open_confirmation_if_needed()

	def _open_confirmation_if_needed(self):
		needs_folder_remember_prompt = self._pending["folder_just_picked"] and not self._config.remember_output_folder
		needs_texture_prompt = self._pending["format"].supports_materials and not self._config.remember_texture_mode

		if not needs_folder_remember_prompt and not needs_texture_prompt:
			self._pending["texture_mode"] = self._config.texture_mode
			self._run_export()
			return

		self._confirm_remember_folder = False
		self._confirm_texture_mode = self._config.texture_mode
		self._confirm_remember_texture = False
		self._confirm_open = True
		imgui.open_popup(_CONFIRM_POPUP_ID)

	def _draw_confirmation_popup(self):
		if not self._confirm_open:
			return

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_CONFIRM_POPUP_ID, None, flags)
		if not opened:
			return

		pending = self._pending
		imgui.text(f"Export folder:\n{pending['folder']}")
		if pending["folder_just_picked"] and not self._config.remember_output_folder:
			_, self._confirm_remember_folder = imgui.checkbox(
				"Always use this folder (skip asking)", self._confirm_remember_folder)

		if pending["format"].supports_materials and not self._config.remember_texture_mode:
			imgui.separator()
			imgui.text("Textures:")
			if imgui.radio_button("Copy as .png next to the export", self._confirm_texture_mode == TEXTURE_MODE_COPY_PNG):
				self._confirm_texture_mode = TEXTURE_MODE_COPY_PNG
			if imgui.radio_button(
					"Reference original filename only", self._confirm_texture_mode == TEXTURE_MODE_REFERENCE_ONLY):
				self._confirm_texture_mode = TEXTURE_MODE_REFERENCE_ONLY
			_, self._confirm_remember_texture = imgui.checkbox(
				"Always use this choice (skip asking)", self._confirm_remember_texture)

		imgui.separator()
		if imgui.button("Export"):
			if self._confirm_remember_folder:
				self._config.output_folder = pending["folder"]
				self._config.remember_output_folder = True
			if pending["format"].supports_materials:
				self._config.texture_mode = self._confirm_texture_mode
				if self._confirm_remember_texture:
					self._config.remember_texture_mode = True
			export_config.save(self._config)

			pending["texture_mode"] = self._confirm_texture_mode if pending["format"].supports_materials \
				else TEXTURE_MODE_REFERENCE_ONLY
			self._confirm_open = False
			imgui.close_current_popup()
			self._run_export()
		imgui.same_line()
		if imgui.button("Cancel"):
			self._confirm_open = False
			self._pending = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _run_export(self):
		pending, self._pending = self._pending, None
		try:
			written = export_shape(
				pending["shape_value"], pending["name"], pending["format"], pending["folder"],
				pending["texture_mode"], pending["asset_index"])
			self._status = f"Exported {len(written)} file(s) to {pending['folder']}"
			print(f"[export] {self._status}: {[str(path) for path in written]}")
		except Exception as exc:
			self._status = f"Export failed: {exc}"
			print(f"[export] {self._status}")

	def _start_settings_folder_dialog(self):
		self._settings_folder_dialog = pfd.select_folder("Choose export folder", self._config.output_folder or "")

	def _poll_settings_folder_dialog(self):
		if self._settings_folder_dialog is None or not self._settings_folder_dialog.ready(0):
			return
		result = self._settings_folder_dialog.result()
		self._settings_folder_dialog = None
		if result:
			self._config.output_folder = result

	def draw_settings_content(self):
		"""Export settings, meant to be embedded directly in the host app's
		own "Settings" tab (see object_editor.py's draw_panel()) rather than
		a separate floating window -- these are app-wide preferences, not
		tied to any one shape, so they belong somewhere always reachable
		rather than behind a right-click command on a file (which read as
		out of place: every other right-click command there acts on the
		selected file)."""
		imgui.text(f"Output folder: {self._config.output_folder or '(not set)'}")
		if imgui.button("Choose folder..."):
			self._start_settings_folder_dialog()
		_, self._config.remember_output_folder = imgui.checkbox(
			"Always use this folder (skip asking)", self._config.remember_output_folder)

		imgui.separator()
		imgui.text("Default texture handling:")
		if imgui.radio_button("Copy as .png next to the export", self._config.texture_mode == TEXTURE_MODE_COPY_PNG):
			self._config.texture_mode = TEXTURE_MODE_COPY_PNG
		if imgui.radio_button(
				"Reference original filename only", self._config.texture_mode == TEXTURE_MODE_REFERENCE_ONLY):
			self._config.texture_mode = TEXTURE_MODE_REFERENCE_ONLY
		_, self._config.remember_texture_mode = imgui.checkbox(
			"Always use this choice (skip asking)", self._config.remember_texture_mode)

		if imgui.button("Save"):
			export_config.save(self._config)

		if self._status:
			imgui.separator()
			imgui.text_wrapped(self._status)
