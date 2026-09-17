"""ObjectEditorApp mixin: the Geometry tab -- shows whether the loaded shape
has skinning, and (a plain CMesh only) lets the user enable/disable its own
skin data, clear it entirely, or import a new rig from a .dae/.fbx file.

Toggling the "Skinned" checkbox mutates `geom.skinned` directly, which
_draw_transform_panel() (viewport_transform.py) already reads to gray out the
global position/rotation/scale editor for a skinned shape (its own transform
has zero effect in-game once skinned) -- except a pivot-locked edit, which
bakes into the mesh's own vertices instead and stays meaningful regardless.

Only a plain CMesh supports editing (checkbox, Clear Skinning, Import rig):
pynel only round-trips CMeshMRM/CMeshMultiLod geometry as opaque raw bytes
(materials are the only editable part, see ryzom_shape.py's own module
docstring) -- mutating `geom.skinned` or its vertex data on either would be
silently dropped on save. CMeshMRMSkinned has no such flag to begin with: the
class itself is always skinned, with per-LOD skin data (MeshMRMSkinnedLod) --
"disabling" it would mean converting to a different Python type entirely, not
toggling a field or clearing some channels.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

import dataclasses
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, portable_file_dialogs as pfd

from pynel.ryzom_shape import Mesh, MeshMRM, MeshMRMSkinned, MatrixBlock

from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.shape_geometry import shape_geom
from ryzom_forgery.shape_import import (
	ShapeImportError, _build_skinned_matrix_blocks, _normalize_skin_weights, import_rig_bone_weights,
)
from ryzom_forgery.apps.object_editor_mixins.geometry_helpers import _is_shape_skinned
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import _colored_button, _icon_button, _CONFIRM_NO_COLOR, _CONFIRM_YES_COLOR

_CLEAR_SKINNING_POPUP_ID = "Clear Skinning"


def _mesh_has_skin_data(geom) -> bool:
	"""True if `geom` (a Mesh's MeshGeom) already has real per-vertex skin
	data on it -- bone names plus both VertexBuffer channels a skinned CMesh
	needs (see shape_geometry.py's _passes_from_mesh_geom()). Used to gate
	the "Skinned" checkbox/[Clear Skinning]: never let the checkbox be
	checked with skinned=True but no real weight data behind it, which the
	engine would happily load (it just trusts the serialized flag, see
	mesh.cpp's CMeshGeom::serial()) but which would render as complete
	garbage or fail assertions downstream."""
	channels = geom.vertex_buffer.channels
	return bool(geom.bones_name) and bool(channels.get("Weight")) and bool(channels.get("PaletteSkin"))


def _skinning_summary(shape_value):
	"""(bone count, vertices with a real weight, total vertices) for any
	skinned shape type pynel can read -- None if `shape_value` has no
	skinning-relevant geom at all. Each shape type stores skin weights
	differently: a Mesh/MeshMRM's VertexBuffer-Weight channel/skin_weights
	list is a per-vertex float4 (all-zero means unweighted); a
	MeshMRMSkinned's packed_vertices quantize weights to uint8 (0 means the
	same)."""
	geom = shape_geom(shape_value)
	if geom is None:
		return None
	bones_name = getattr(geom, "bones_name", None)
	if bones_name is None:
		return None
	if isinstance(shape_value, Mesh):
		weights = geom.vertex_buffer.channels.get("Weight", [])
		total = geom.vertex_buffer.num_verts
		skinned = sum(1 for w in weights if sum(w) > 0)
	elif isinstance(shape_value, MeshMRM):
		total = geom.num_vertices
		skinned = sum(1 for _matrix_ids, w in geom.skin_weights if sum(w) > 0)
	elif isinstance(shape_value, MeshMRMSkinned):
		total = len(geom.packed_vertices)
		skinned = sum(1 for pv in geom.packed_vertices if any(pv.weights))
	else:
		return None
	return len(bones_name), skinned, total


class GeometryUIMixin:
	def _draw_geometry_tab(self):
		"""Geometry tab content: skinning status header (every shape type),
		then, for a plain CMesh only, the checkbox/[Clear Skinning]/
		[Import rig] controls (see this module's own docstring for why only
		CMesh)."""
		if self.shape_file is None:
			return
		shape_value = self.shape_file.value
		is_skinned = _is_shape_skinned(shape_value)
		imgui.text(f"Skinning: {'Yes' if is_skinned else 'No'}")
		if is_skinned:
			summary = _skinning_summary(shape_value)
			if summary is not None:
				num_bones, num_skinned, num_total = summary
				imgui.text(f"{num_bones} bone(s) used, {num_skinned}/{num_total} vertices weighted")
			imgui.text(f"Mesh type: {self.shape_file.type_name}")

		if not isinstance(shape_value, Mesh):
			imgui.text_wrapped(
				f"Editing skinning is only supported for a plain CMesh -- pynel can only read/render "
				f"{self.shape_file.type_name}'s geometry, not write changes to it."
			)
			return

		geom = shape_value.geom
		has_skin_data = _mesh_has_skin_data(geom)
		imgui.begin_disabled(not has_skin_data)
		changed, new_value = imgui.checkbox("Skinned", geom.skinned)
		imgui.end_disabled()
		if changed:
			geom.skinned = new_value
			self._rebuild_geometry_preserving_camera()
		if not has_skin_data:
			imgui.same_line()
			imgui.text_disabled("(no skin data yet -- use [Import rig] below)")

		imgui.separator()
		imgui.begin_disabled(not has_skin_data)
		if imgui.button("Clear Skinning"):
			imgui.open_popup(_CLEAR_SKINNING_POPUP_ID)
		imgui.end_disabled()
		self._draw_clear_skinning_popup()

		if _icon_button(fa_icons.ICON_FA_UPLOAD, "Import rig (.dae/.fbx)..."):
			self._import_rig_dialog = pfd.open_file("Import rig", "", ["Rig files", "*.dae *.fbx"])
		if self._geometry_import_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._geometry_import_error)

	def _draw_clear_skinning_popup(self):
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_CLEAR_SKINNING_POPUP_ID, None, flags)
		if not opened:
			return
		imgui.text("This will permanently erase this mesh's skinning data")
		imgui.text("(bone names and per-vertex weights). Vertex positions stay unchanged.")
		imgui.separator()
		if _colored_button("Clear", _CONFIRM_YES_COLOR):
			self._clear_skinning()
			imgui.close_current_popup()
		imgui.same_line()
		if _colored_button("Cancel", _CONFIRM_NO_COLOR):
			imgui.close_current_popup()
		imgui.end_popup()

	def _clear_skinning(self):
		geom = self.shape_file.value.geom
		geom.bones_name = []
		new_channels = {name: values for name, values in geom.vertex_buffer.channels.items()
		                if name not in ("Weight", "PaletteSkin")}
		geom.vertex_buffer = dataclasses.replace(geom.vertex_buffer, channels=new_channels)
		all_rdr_passes = [rdr_pass for matrix_block in geom.matrix_blocks for rdr_pass in matrix_block.rdr_passes]
		geom.matrix_blocks = [MatrixBlock(matrix_id=[0] * 16, num_matrix=0, rdr_passes=all_rdr_passes)]
		geom.skinned = False
		self._rebuild_geometry_preserving_camera()

	def _poll_import_rig_dialog(self):
		if self._import_rig_dialog is None or not self._import_rig_dialog.ready(0):
			return
		dialog, self._import_rig_dialog = self._import_rig_dialog, None
		result = dialog.result()
		if not result:
			return
		self._import_rig(Path(result[0]))

	def _import_rig(self, path):
		self._geometry_import_error = ""
		shape_value = self.shape_file.value
		if not isinstance(shape_value, Mesh):
			return
		geom = shape_value.geom
		num_verts = geom.vertex_buffer.num_verts
		try:
			bone_weights = import_rig_bone_weights(path)
		except (OSError, ShapeImportError) as exc:
			self._geometry_import_error = f"Import rig failed: {exc}"
			return
		if len(bone_weights) != num_verts:
			self._geometry_import_error = (
				f"Import rig failed: {path.name} has {len(bone_weights)} vertices, "
				f"the loaded shape has {num_verts} -- they must match exactly.")
			return

		bones_name, weights, matrix_ids = _normalize_skin_weights(bone_weights)
		pass_indices = {}
		for matrix_block in geom.matrix_blocks:
			for rdr_pass in matrix_block.rdr_passes:
				pass_indices.setdefault(rdr_pass.material_id, []).extend(rdr_pass.indices)
		matrix_blocks, weight_channel, palette_channel, extra_vertex_sources = _build_skinned_matrix_blocks(
			pass_indices, matrix_ids, weights)

		new_channels = dict(geom.vertex_buffer.channels)
		for channel_name in ("Position", "Normal", "TexCoord0"):
			values = new_channels.get(channel_name)
			if values is None:
				continue
			values = list(values)
			values.extend(values[v] for v in extra_vertex_sources)
			new_channels[channel_name] = values
		new_channels["Weight"] = weight_channel
		new_channels["PaletteSkin"] = palette_channel
		new_types = list(geom.vertex_buffer.types)
		new_types[12] = 10  # Weight: float4 (CVertexBuffer::Weight/Float4, mesh.cpp)
		new_types[13] = 12  # PaletteSkin: uint8x4 (CVertexBuffer::PaletteSkin/UChar4, mesh.cpp)
		geom.vertex_buffer = dataclasses.replace(
			geom.vertex_buffer, channels=new_channels, types=new_types,
			num_verts=num_verts + len(extra_vertex_sources))
		geom.bones_name = bones_name
		geom.matrix_blocks = matrix_blocks
		geom.skinned = True
		self._rebuild_geometry_preserving_camera()
