"""Export flow UI for .shape -> .obj/.dae/.stl/.fbx/.gltf/.glb: asks for an
output folder (native dialog) every time, then, for formats that carry
materials, how to handle textures -- no "remember this choice" shortcut
(removed in the patina-export-rework chantier: it was a single setting
shared across every workspace, so it stuck regardless of which workspace
was active, which read as the export folder being "stuck" to one place).
"""

import zipfile
from pathlib import Path
from typing import List

from imgui_bundle import imgui, portable_file_dialogs as pfd

from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.settings import TEXTURE_MODE_COPY_PNG, TEXTURE_MODE_REFERENCE_ONLY
from ryzom_forgery.shape_export import export_assembled_creature, export_mesh_with_skin, export_shape
from ryzom_forgery.workspace_sync import pack_workspace_bnp

_CONFIRM_POPUP_ID = "Export"
_SKEL_ANIM_POPUP_ID = "Export mesh/skeleton/animation"

# Extensions export_mesh_with_skin() (shape_export.py's _ASSIMP_FORMAT_IDS) can
# actually embed a skeleton/animation into -- .obj/.stl have no such concept.
_SKIN_CAPABLE_EXTENSIONS = {"dae", "fbx", "gltf", "glb"}

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
		self._skel_anim_open = False
		self._include_skel = True
		self._include_anim = True
		self._combined = True
		self._status = ""

	def export(self, shape_value, name, export_format, texture_finder, source_folder=None, skeleton=None, animation=None):
		"""Starts exporting `shape_value` (the shape currently open in the
		editor, possibly edited -- see object_editor.py's Export button in
		the bottom bar) via `export_format`. `name` supplies the output
		file's stem. Always asks for an output folder, then (for formats
		that carry materials) how to handle textures, then -- if `skeleton`
		is given (the Skinning preview's currently loaded squelette, see
		mesh_skel_anim_io.md sous-chantier 6) and `export_format` can embed
		one -- a mesh/skeleton/animation selection popup (see
		_draw_skel_anim_popup())."""
		self._pending = {
			"kind": "shape",
			"shape_value": shape_value, "name": name, "format": export_format, "texture_finder": texture_finder,
			"skeleton": skeleton, "animation": animation,
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

	def export_assembled_creature(
			self, shape_values, skeleton, animation, name, export_format, texture_finder,
			weapon_shape_value=None, weapon_bone_name=None, source_folder=None):
		""""Export PNJ" (mesh_skel_anim_io.md item 7) -- see
		creature_bind.py's own "Export this assembled creature" button.
		Always combines everything into one file (mesh+skeleton+animation,
		masked parts already excluded since `shape_values` only ever holds
		one piece per slot -- see shape_export.export_assembled_creature()'s
		own docstring), no mesh/skel/anim popup (nothing to choose between).
		Named `__skip__<name>.<ext>` (see import_watcher.py's own
		`__skip__` filename filter) so it's never picked back up as new
		content to import. Asks for an output folder, then (for formats
		that carry materials) how to handle textures, same as export()."""
		self._pending = {
			"kind": "assembled_creature",
			"shape_values": shape_values, "skeleton": skeleton, "animation": animation,
			"name": name, "format": export_format, "texture_finder": texture_finder,
			"weapon_shape_value": weapon_shape_value, "weapon_bone_name": weapon_bone_name,
		}
		self._folder_dialog = pfd.select_folder("Choose export folder", str(source_folder or ""))

	def draw(self):
		"""Call once per ImGui frame."""
		self._poll_folder_dialog()
		self._draw_confirmation_popup()
		self._draw_skel_anim_popup()

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
		if (self._pending["kind"] == "shape" and self._pending.get("skeleton") is not None
				and self._pending["format"].extension in _SKIN_CAPABLE_EXTENSIONS):
			self._include_skel = True
			self._include_anim = self._pending.get("animation") is not None
			self._combined = True
			self._skel_anim_open = True
			imgui.open_popup(_SKEL_ANIM_POPUP_ID)
			return
		self._run_export()

	def _draw_skel_anim_popup(self):
		"""Mesh/skeleton/animation selection popup (mesh_skel_anim_io.md
		sous-chantier 6) -- only reachable from _start_export() when a
		skeleton is loaded and the chosen format can embed one. "Separate"
		always keeps the mesh in every file (an assimp Scene needs at least
		one, see shape_export.py's own "No renderable geometry" guard) --
		only skeleton/animation presence toggles between the produced files;
		an Anim file always carries its skeleton too (an animation targets
		bone nodes, meaningless without them)."""
		if not self._skel_anim_open:
			return
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_SKEL_ANIM_POPUP_ID, None, flags)
		if not opened:
			return

		pending = self._pending
		has_anim = pending.get("animation") is not None

		imgui.begin_disabled(True)
		imgui.checkbox("Mesh", True)
		imgui.end_disabled()
		_, self._include_skel = imgui.checkbox("Skeleton", self._include_skel)
		imgui.begin_disabled(not has_anim)
		_, self._include_anim = imgui.checkbox("Animation", self._include_anim and has_anim)
		imgui.end_disabled()

		imgui.separator()
		if imgui.radio_button("Combined (one file)", self._combined):
			self._combined = True
		if imgui.radio_button("Separate files", not self._combined):
			self._combined = False

		imgui.separator()
		frame_padding_x2 = imgui.get_style().frame_padding.x * 2
		export_label = "Export"
		_center_next_widget(imgui.calc_text_size(export_label).x + frame_padding_x2)
		if _colored_button(export_label, _COPY_TEXTURES_COLOR):
			self._skel_anim_open = False
			imgui.close_current_popup()
			self._run_export()
		cancel_label = "Cancel"
		_center_next_widget(imgui.calc_text_size(cancel_label).x + frame_padding_x2)
		if imgui.button(cancel_label):
			self._skel_anim_open = False
			self._pending = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _run_skel_anim_export(self, pending, skeleton, animation) -> List[Path]:
		"""Combined (one file with whatever's checked) or separate (mesh
		alone always written; a second file adds the skeleton if checked; a
		third adds the animation too if checked, since an animation targets
		bone nodes and is meaningless without them -- see
		_draw_skel_anim_popup()'s own docstring) mesh/skeleton/animation
		export, both via export_mesh_with_skin()."""
		stem = Path(pending["name"]).stem
		extension = pending["format"].extension
		folder = Path(pending["folder"])
		args = (pending["shape_value"], pending["texture_mode"], pending["texture_finder"])

		if self._combined:
			output_path = folder / f"{stem}.{extension}"
			return export_mesh_with_skin(args[0], skeleton, output_path, args[1], args[2], animation=animation)

		written = list(export_mesh_with_skin(args[0], None, folder / f"{stem}.{extension}", args[1], args[2]))
		if self._include_skel:
			written += export_mesh_with_skin(args[0], skeleton, folder / f"{stem}_skel.{extension}", args[1], args[2])
		if self._include_anim and animation is not None:
			written += export_mesh_with_skin(
				args[0], skeleton, folder / f"{stem}_anim.{extension}", args[1], args[2], animation=animation)
		return written

	def _run_export(self):
		pending, self._pending = self._pending, None
		try:
			if pending["kind"] == "assembled_creature":
				output_path = Path(pending["folder"]) / f"__skip__{Path(pending['name']).stem}.{pending['format'].extension}"
				written = export_assembled_creature(
					pending["shape_values"], pending["skeleton"], pending["animation"], output_path,
					pending["texture_mode"], pending["texture_finder"],
					weapon_shape_value=pending.get("weapon_shape_value"), weapon_bone_name=pending.get("weapon_bone_name"))
			else:
				skeleton = pending.get("skeleton") if self._include_skel or self._include_anim else None
				animation = pending.get("animation") if self._include_anim else None
				if skeleton is not None and pending["format"].extension in _SKIN_CAPABLE_EXTENSIONS:
					written = self._run_skel_anim_export(pending, skeleton, animation)
				else:
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

	def quick_export(self, shape_value, name, export_format, texture_finder, output_dir, skeleton=None, animation=None):
		"""Workspace-only fast path (see apps/object_editor.py's bottom-bar
		[Export] button, to the left of [Export as...]): writes straight to
		`output_dir` (always `<workspace>/exports/`) with no folder prompt and
		no texture-handling popup -- always TEXTURE_MODE_REFERENCE_ONLY, since
		a workspace's textures already live locally (`tex/`/`dds/`) or as an
		absolute path outside the workspace, so there's nothing to copy.
		"No prompts" also means no mesh/skeleton/animation popup either -- if
		`skeleton` is given and the format can embed one, everything
		available (mesh + skeleton + animation if any) is combined into a
		single file, matching this button's own "fast, no choices" nature."""
		try:
			if skeleton is not None and export_format.extension in _SKIN_CAPABLE_EXTENSIONS:
				output_path = Path(output_dir) / f"{Path(name).stem}.{export_format.extension}"
				written = export_mesh_with_skin(
					shape_value, skeleton, output_path, TEXTURE_MODE_REFERENCE_ONLY, texture_finder, animation=animation)
			else:
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
