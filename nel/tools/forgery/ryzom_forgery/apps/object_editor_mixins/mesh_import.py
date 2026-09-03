"""ObjectEditorApp mixin: importing a mesh (.obj/.dae/.fbx/.gltf/.glb) as a
brand-new shape, or replacing the current Mesh's geometry with an imported
one (including the material-count-mismatch match-up popup). Split out of
object_editor.py, see the "Split object_editor.py into theme files" chantier
in project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui
from panda3d.core import Quat

from pynel.ryzom_shape import ShapeFile

from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.shape_geometry import IDENTITY_QUAT
from ryzom_forgery.shape_import import texture_search_dirs_for
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_colored_button, _CONFIRM_NO_COLOR, _CONFIRM_YES_COLOR, _icon_button,
)

_REPLACE_MATCH_POPUP_ID = "Match materials"


class MeshImportMixin:
	def _draw_import_toolbar_button(self):
		if _icon_button(fa_icons.ICON_FA_UPLOAD, "Import mesh (.obj/.dae/.fbx/.gltf/.glb)..."):
			self.import_dialog.open(self.shape_file is not None)

	def _on_import_new_shape(self, mesh, source_path):
		# Only CMesh can be built from scratch this way (see
		# ryzom_forgery/shape_import.py's module docstring) -- matches
		# shape_importer.py's CLI equivalent.
		self._reset_shape_state()
		if source_path is not None:
			self._shape_source_name = source_path.stem + ".shape"
			# The imported file's own folder (see load_panda_texture()'s
			# search_dirs) -- an .fbx/.dae/.obj's textures routinely sit right
			# next to it (or in a tex/textures/data subfolder, or an .fbx's
			# own <name>.fbm sibling) rather than anywhere in the Ryzom asset
			# root this app otherwise searches.
			self._texture_search_dirs = texture_search_dirs_for(source_path)
		self._display_shape(ShapeFile(type_name="Mesh", value=mesh))

	def _on_import_replace(self, mesh, source_path):
		if self.shape_file is None:
			return
		if self.shape_file.type_name != "Mesh":
			self._save_status = (
				f"Can't replace geometry: the current shape is a {self.shape_file.type_name!r}, "
				f"only a plain Mesh's geometry can be swapped this way.")
			return

		if source_path is not None:
			# Any material the mismatched-count remap path below adds from
			# the imported mesh (see _replace_geometry()) carries that mesh's
			# own texture names -- same reasoning as _on_import_new_shape().
			self._texture_search_dirs.extend(texture_search_dirs_for(source_path))

		current_materials = self.shape_file.value.materials
		if len(mesh.materials) == len(current_materials):
			# Same convention the .obj/.dae exporter itself used to number
			# materials in the first place -- indices already line up.
			self._replace_geometry(mesh, index_map=None)
		else:
			# open_popup() isn't called here: this runs while still nested
			# inside ImportDialog's own "Import mesh" popup (not yet closed
			# for the rest of this frame), and opening one popup from inside
			# another that hasn't ended yet silently failed to register.
			# _draw_replace_match_popup() opens it itself, next frame,
			# from draw_panel()'s top level instead.
			self._replace_pending_mesh = mesh
			self._replace_mapping = [None] * len(mesh.materials)

	def _replace_geometry(self, mesh, index_map):
		"""Swaps the current Mesh's geometry (vertex buffer, matrix
		blocks/render passes, bbox...) for `mesh.geom`, leaving
		self.shape_file.value's materials (and whatever editing was done to
		them) untouched. `index_map`, if given, remaps each render pass's
		material_id from `mesh`'s own material indices to the current
		shape's (used for the mismatched-material-count case, after
		_draw_replace_match_popup() resolved a mapping)."""
		if index_map is not None:
			for matrix_block in mesh.geom.matrix_blocks:
				for rdr_pass in matrix_block.rdr_passes:
					rdr_pass.material_id = index_map[rdr_pass.material_id]

		self.shape_file.value.geom = mesh.geom
		# See import_watcher.py's update_existing_shape() for why: an
		# imported mesh's source file already has the shape's own (previous)
		# default_rot_quat baked into its vertices by shape_export.py's
		# export_shape(), so the new geometry already represents the final
		# orientation -- reset to identity to avoid rotating it again.
		base = getattr(self.shape_file.value, "base", None)
		if base is not None:
			old_rot = base.default_rot_quat
			if old_rot != IDENTITY_QUAT:
				# _object_pivot was seeded with this same old_rot on the
				# original _display_shape() (plus, potentially, further
				# Ctrl+drag deltas right-multiplied onto it since -- see
				# ObjectManipulator._rotate()). Left-multiplying by its
				# conjugate strips exactly that seeded rotation back out,
				# leaving only the user's own manual deltas -- otherwise the
				# pivot would still apply old_rot on top of geometry that
				# now already has it baked in, throwing the model out of
				# sync with the camera framing computed from its new bbox.
				old_quat = Quat(old_rot.w, old_rot.x, old_rot.y, old_rot.z)
				self._object_pivot.set_quat(old_quat.conjugate() * self._object_pivot.get_quat())
			base.default_rot_quat = IDENTITY_QUAT
		self._rebuild_geometry()
		self._save_status = "Geometry replaced."

	def _describe_current_material(self, index):
		material = self.shape_file.value.materials[index]
		texture = material.textures[0] if material.textures else None
		name = texture.file_name if texture is not None and texture.file_name else None
		return f"{index}: {name}" if name else f"{index}: (no texture)"

	def _draw_replace_match_popup(self):
		"""Mismatched material counts between the imported mesh and the
		current shape: lets the user match each imported material to an
		existing one (keeping its edits) or add it as a new material,
		before _replace_geometry() actually swaps the geometry in."""
		if self._replace_pending_mesh is None:
			self._replace_match_popup_opened = False
			return

		if not self._replace_match_popup_opened:
			imgui.open_popup(_REPLACE_MATCH_POPUP_ID)
			self._replace_match_popup_opened = True

		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_REPLACE_MATCH_POPUP_ID, None, flags)
		if not opened:
			return

		mesh = self._replace_pending_mesh
		current_materials = self.shape_file.value.materials
		imgui.text(f"The imported mesh has {len(mesh.materials)} material(s), "
		           f"the current shape has {len(current_materials)} -- match each one:")
		imgui.separator()

		for i, material in enumerate(mesh.materials):
			texture = material.textures[0] if material.textures else None
			label = texture.file_name if texture is not None and texture.file_name else f"material {i}"
			imgui.text(label)
			imgui.same_line()
			imgui.push_id(f"replace-match-{i}")

			target = self._replace_mapping[i]
			preview = "Add as new material" if target is None else self._describe_current_material(target)
			imgui.set_next_item_width(220)
			if imgui.begin_combo("##target", preview):
				clicked, _ = imgui.selectable("Add as new material", target is None)
				if clicked:
					self._replace_mapping[i] = None
				for j in range(len(current_materials)):
					clicked, _ = imgui.selectable(self._describe_current_material(j), target == j)
					if clicked:
						self._replace_mapping[i] = j
				imgui.end_combo()

			imgui.pop_id()

		imgui.separator()
		if _colored_button("Replace", _CONFIRM_YES_COLOR):
			self._confirm_replace_matching()
			imgui.close_current_popup()
		imgui.same_line()
		if _colored_button("Cancel", _CONFIRM_NO_COLOR):
			self._replace_pending_mesh = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _confirm_replace_matching(self):
		mesh, self._replace_pending_mesh = self._replace_pending_mesh, None
		current_materials = self.shape_file.value.materials

		index_map = {}
		for i, target in enumerate(self._replace_mapping):
			if target is None:
				current_materials.append(mesh.materials[i])
				index_map[i] = len(current_materials) - 1
			else:
				index_map[i] = target

		self._replace_geometry(mesh, index_map)
