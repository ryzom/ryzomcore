"""Ryzom Forgery object editor: browse, inspect and edit .shape files.

3D display is supported for CMesh, CMeshMRM (finest LOD) and CMeshMultiLod
(slot 0, whose geometry is itself a CMesh/CMeshMRM). Other shape types
(skeleton, water, flare, particles, ...) show their properties only, no 3D
render yet.
"""

from pathlib import Path

from panda3d.core import (
	Geom, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat, GeomVertexWriter,
	Material as PandaMaterial, Point3,
)

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.camera import OrbitCamera
from ryzom_forgery.asset_index import AssetIndex
from ryzom_forgery.export_dialog import ExportDialog
from ryzom_forgery.material_docs import load_material_docs
from ryzom_forgery.navcube import NavigationCube
from ryzom_forgery.properties import draw_properties
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.shape_geometry import iter_render_passes, load_panda_texture, rgba_to_color, shape_bbox

from pynel.ryzom_shape import ShapeParseError, ShapeWriteError, Texture, parse_shape, save_shape

DEFAULT_DATA_ROOT = Path("~/.local/share/Ryzom/ryzom_live/data").expanduser()

# Shape types pynel's save_shape() can actually write back out -- matches
# ryzom_shape.py's _SHAPE_CLASS_NAMES, the Save/Save As UI only shows for these.
_WRITABLE_SHAPE_TYPES = {"Mesh", "MeshMRM", "MeshMRMSkinned", "MeshMultiLod"}

_OVERWRITE_POPUP_ID = "Overwrite shape?"

_STATUS_HINT_COLOR = (1.0, 0.6, 0.15, 1.0)  # orange, for material_options.md hints shown in the status bar
_COLOR_POPUP_ID = "material-color-picker"

# Multi Bitmap slot index -> (quality label, ecosystem label, season label),
# the three known Georges/engine conventions documented in
# docs/material_options.md (item_map.typ's map_variant, _creature_texture.typ,
# and EGSPD::CSeason in ryzom/common/src/game_share/season.h) -- which one
# actually applies depends on the shape, so all three are always shown together.
_MULTI_BITMAP_SLOT_LABELS = [
	("Low Quality", "Forest", "Spring"),  # "Forest": labelled "none" in _creature_texture.typ, but means Forest in practice
	("Medium Quality", "Lacustre", "Summer"),
	("High Quality", "Desert", "Autumn"),
	("Super Quality", "Jungle", "Winter"),
	("XL Quality", "Primr", None),
	("Suprem Quality", "goo", None),
	("Divine Quality", None, None),
	("Obiwan Quality", None, None),
]


def _icon_button(icon, tooltip):
	"""An icon-only button (Font Awesome glyph, see ryzom_forgery.app's
	_load_icon_font) with a hover tooltip, since an icon alone isn't always
	self-explanatory."""
	clicked = imgui.button(icon)
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


def _multi_bitmap_slot_label(index):
	if 0 <= index < len(_MULTI_BITMAP_SLOT_LABELS):
		quality, ecosystem, season = _MULTI_BITMAP_SLOT_LABELS[index]
		labels = " / ".join(label for label in (quality, ecosystem, season) if label)
		return f"{index} - {labels}"
	return str(index)


def _build_geom(vertex_buffer, indices):
	positions = vertex_buffer.channels.get("Position")
	normals = vertex_buffer.channels.get("Normal")
	texcoords = vertex_buffer.channels.get("TexCoord0")

	if normals and texcoords:
		vformat = GeomVertexFormat.get_v3n3t2()
	elif normals:
		vformat = GeomVertexFormat.get_v3n3()
	elif texcoords:
		vformat = GeomVertexFormat.get_v3t2()
	else:
		vformat = GeomVertexFormat.get_v3()

	vdata = GeomVertexData("shape", vformat, Geom.UH_static)
	vdata.set_num_rows(vertex_buffer.num_verts)

	vertex_writer = GeomVertexWriter(vdata, "vertex")
	for p in positions:
		vertex_writer.add_data3(p[0], p[1], p[2])

	if normals:
		normal_writer = GeomVertexWriter(vdata, "normal")
		for n in normals:
			normal_writer.add_data3(n[0], n[1], n[2])

	if texcoords:
		uv_writer = GeomVertexWriter(vdata, "texcoord")
		for uv in texcoords:
			uv_writer.add_data2(uv[0], uv[1])

	triangles = GeomTriangles(Geom.UH_static)
	for i in range(0, len(indices), 3):
		triangles.add_vertices(indices[i], indices[i + 1], indices[i + 2])
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	return geom


class ObjectEditorApp(ForgeryApp):
	def __init__(self, data_root=DEFAULT_DATA_ROOT):
		ForgeryApp.__init__(self, explorer_root=data_root, title="Ryzom Forgery - Object Editor",
		                     explorer_default_filter="*.shape")

		self.data_root = Path(data_root)
		self.material_docs = load_material_docs()
		self.asset_index = AssetIndex(self.data_root)
		self.sysinfo.set_status("Indexing assets...")
		self.asset_index.build()
		self.sysinfo.set_status(f"{len(self.asset_index)} assets indexed")

		self._texture_cache = {}

		self.model_root = self.render.attach_new_node("shape-root")
		self.shape_file = None
		self.shape_error = None
		self._material_node_paths = {}  # material_id -> list[NodePath], to re-apply a material live after an edit
		self._multi_bitmap_expanded = set()  # slot indices currently expanded in the Multi Bitmap editor
		self._multi_bitmap_hint_shown = False  # whether the status bar currently shows one of our doc hints
		self._texture_browse_dialogs = {}  # key -> (in-flight portable_file_dialogs.open_file, on_result callback)
		self._material_override_colors = {}  # material_id -> (r,g,b,a), a manual flat-color override for that material

		self._shape_source_path = None  # Path on disk, or None if loaded from inside a .bnp (Save disabled then)
		self._save_overwrite_confirmed = False  # session-scoped: asked once, no more Save confirmations after that
		self._confirm_overwrite_open = False
		self._save_dialog = None  # in-flight portable_file_dialogs.save_file, for Save As
		self._save_status = ""

		self.orbit_camera = OrbitCamera(self, distance=10.0)
		self.nav_cube = NavigationCube(self, self.orbit_camera)
		self.export_dialog = ExportDialog()
		self.commands.register_for_extension(".shape", "Load in viewer", self._on_load_command)
		for export_format in EXPORT_FORMATS:
			self.commands.register_for_extension(
				".shape", f"Export to .{export_format.extension}",
				lambda items, fmt=export_format: self._on_export_command(items, fmt))
		self.commands.register_global("Export settings...", lambda items: self.export_dialog.open())

	def on_selection_changed(self, items):
		print(f"[object_editor] selection changed: {[item.name for item in items]}")
		if len(items) == 1 and items[0].suffix.lower() == ".shape":
			self._load_shape(items[0])

	def _on_load_command(self, items):
		if items:
			self._load_shape(items[0])

	def _on_export_command(self, items, export_format):
		if items:
			self.export_dialog.export(items[0], export_format, self.asset_index)

	def _load_shape(self, item):
		self.model_root.remove_node()
		self.model_root = self.render.attach_new_node("shape-root")
		self.shape_error = None
		self._material_node_paths = {}
		self._multi_bitmap_expanded = set()
		self._texture_browse_dialogs = {}
		self._material_override_colors = {}
		self._shape_source_path = item.path if item.bnp_path is None else None
		self._save_status = ""

		try:
			self.shape_file = parse_shape(item.read_bytes())
		except ShapeParseError as exc:
			self.shape_file = None
			self.shape_error = str(exc)
			return

		pass_count = 0
		for vertex_buffer, material_id, indices in iter_render_passes(self.shape_file.value):
			if not indices:
				continue
			print(f"[object_editor] pass {pass_count}: material={material_id} "
			      f"verts={vertex_buffer.num_verts} tris={len(indices) // 3} "
			      f"channels={list(vertex_buffer.channels.keys())}")
			geom = _build_geom(vertex_buffer, indices)
			geom_node = GeomNode(f"pass-{pass_count}")
			geom_node.add_geom(geom)
			node_path = self.model_root.attach_new_node(geom_node)
			self._material_node_paths.setdefault(material_id, []).append(node_path)
			self._apply_material(node_path, material_id)
			pass_count += 1

		if pass_count == 0:
			self.shape_error = f"No renderable 3D geometry for shape type {self.shape_file.type_name!r}"

		bbox = shape_bbox(self.shape_file.value)
		if bbox is not None:
			center = Point3(bbox.center.x, bbox.center.y, bbox.center.z)
			radius = max(bbox.half_size.x, bbox.half_size.y, bbox.half_size.z, 0.1)
			self.orbit_camera.frame(center, radius * 3.0)

	def _apply_material(self, node_path, material_id):
		# Force double-sided rendering (like 3ds Max's "2-Sided" material
		# flag): the game engine may rely on backface culling plus paired
		# geometry to fake thickness, which leaves single-sided faces you
		# can see through in a standalone viewer with no matching backface.
		node_path.set_two_sided(True)
		node_path.clear_texture()
		node_path.clear_color()
		node_path.clear_material()
		node_path.clear_light()

		override_color = self._material_override_colors.get(material_id)
		if override_color is not None:
			# A manual color override (see the material color picker button)
			# replaces the texture entirely, so the material reads unambiguously
			# as "flagged this color" rather than a tinted texture. Lighting and
			# the Material (diffuse/ambient/...) must also be disabled here: a
			# lit NodePath computes its shaded color from its Material, not from
			# set_color(), so leaving either on would still show the old texture's
			# material tint instead of the flat override color.
			node_path.set_texture_off(1)
			node_path.set_material_off(1)
			node_path.set_light_off(1)
			node_path.set_color(override_color, 1)
			return

		materials = getattr(self.shape_file.value, "materials", None)
		if not materials or material_id >= len(materials):
			return
		material = materials[material_id]

		panda_material = PandaMaterial()
		panda_material.set_diffuse(rgba_to_color(material.diffuse))
		panda_material.set_ambient(rgba_to_color(material.ambient))
		panda_material.set_emission(rgba_to_color(material.emissive))
		panda_material.set_specular(rgba_to_color(material.specular))
		panda_material.set_shininess(material.shininess)
		panda_material.set_twoside(True)
		node_path.set_material(panda_material)

		texture = material.textures[0] if material.textures else None
		if texture is None:
			print(f"[object_editor] material {material_id}: no texture")
		elif not texture.file_name:
			print(f"[object_editor] material {material_id}: texture slot present "
			      f"(class={texture.class_name}) but no file_name")
		else:
			print(f"[object_editor] material {material_id}: texture reference {texture.file_name!r}")
			panda_texture = load_panda_texture(self.asset_index, texture.file_name, cache=self._texture_cache)
			if panda_texture is not None:
				node_path.set_texture(panda_texture)

	def _reapply_material(self, material_id):
		"""Re-runs _apply_material on every NodePath using this material, e.g.
		after an in-place edit of the parsed Material (texture set change...)."""
		for node_path in self._material_node_paths.get(material_id, []):
			self._apply_material(node_path, material_id)

	def _set_material_override_color(self, material_id, color):
		"""Sets (or, if color is None, clears) a material's manual flat-color
		override -- the "no color" option in its color picker button."""
		if color is None:
			self._material_override_colors.pop(material_id, None)
		else:
			self._material_override_colors[material_id] = tuple(color)
		self._reapply_material(material_id)

	def _draw_material_color_button(self, material_id):
		"""A small color-swatch button: click to open a picker (plus a "No
		color" option) that sets/clears that material's flat-color override,
		visualizing on the 3D model exactly which faces use that material."""
		override_color = self._material_override_colors.get(material_id)
		swatch_color = override_color if override_color is not None else (0.5, 0.5, 0.5, 0.4)
		if imgui.color_button(f"##color-{material_id}", swatch_color, 0, (20, 0)):
			imgui.open_popup(f"{_COLOR_POPUP_ID}-{material_id}")

		if imgui.begin_popup(f"{_COLOR_POPUP_ID}-{material_id}"):
			if imgui.button("No color"):
				self._set_material_override_color(material_id, None)
				imgui.close_current_popup()
			imgui.separator()
			current = override_color if override_color is not None else (1.0, 1.0, 1.0, 1.0)
			changed, new_color = imgui.color_picker4(f"##picker-{material_id}", current)
			if changed:
				self._set_material_override_color(material_id, new_color)
			imgui.end_popup()

	def _start_texture_browse(self, key, on_result):
		"""Opens a native file picker for a texture; on_result(file_name) is
		called with just the chosen file's base name once picked, matching
		how texture references are stored (name only, no path)."""
		dialog = pfd.open_file("Choose texture", str(self.data_root), ["Textures", "*.tga *.dds *.png"])
		self._texture_browse_dialogs[key] = (dialog, on_result)

	def _poll_texture_browse_dialogs(self):
		for key in list(self._texture_browse_dialogs.keys()):
			dialog, on_result = self._texture_browse_dialogs[key]
			if not dialog.ready(0):
				continue
			del self._texture_browse_dialogs[key]
			result = dialog.result()
			if result:
				# Texture names aren't case-sensitive; lower-cased consistently
				# rather than mixing whatever case a file happens to be on disk.
				on_result(Path(result[0]).name.lower())

	def _multi_bitmap_entries(self):
		"""(material_id, texture) for every material whose slot-0 texture is a
		CTextureMultiFile (see docs/material_options.md, "Plusieurs images
		alternatives par emplacement (Multi Bitmap)"). Only slot 0 is covered
		since it's the only slot _apply_material actually renders today."""
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			return []
		entries = []
		for material_id, material in enumerate(materials):
			texture = material.textures[0] if material.textures else None
			if texture is not None and texture.class_name == "CTextureMultiFile" and texture.file_names:
				entries.append((material_id, texture))
		return entries

	@staticmethod
	def _ensure_multi_bitmap_slot(texture, index):
		"""Pads texture.file_names up to `index` with "" if it's shorter --
		matches how the 3dsMax exporter itself leaves gaps for skipped slots
		(see e.g. xmas_tr_mektoub_selle.shape's slot 3, an empty string)."""
		while len(texture.file_names) <= index:
			texture.file_names.append("")

	def _select_multi_bitmap_slot(self, entries, index):
		"""The "Select" button: switches every Multi Bitmap material of the
		shape to the same slot index at once -- picking "Medium Quality" is a
		whole-object appearance choice, not a per-material one."""
		for material_id, texture in entries:
			self._ensure_multi_bitmap_slot(texture, index)
			texture.selected_index = index
			texture.file_name = texture.file_names[index]
			self._reapply_material(material_id)

	def _draw_multi_bitmap_editor(self):
		entries = self._multi_bitmap_entries()
		if not entries:
			return
		entries_by_id = dict(entries)

		imgui.separator()
		imgui.text("Multi Bitmap")
		doc = self.material_docs.get("multi-bitmap")
		hovered_hint = doc.summary if (doc and imgui.is_item_hovered()) else None

		representative = entries_by_id.get(0, entries[0][1])
		slot_count = max(len(_MULTI_BITMAP_SLOT_LABELS), max(len(t.file_names) for _, t in entries))

		for index in range(slot_count):
			imgui.push_id(f"mb-slot-{index}")

			is_active = representative.selected_index == index
			if is_active:
				imgui.push_style_color(imgui.Col_.button.value, (0.2, 0.65, 0.2, 1.0))
				imgui.push_style_color(imgui.Col_.button_hovered.value, (0.25, 0.7, 0.25, 1.0))
				imgui.push_style_color(imgui.Col_.button_active.value, (0.15, 0.55, 0.15, 1.0))
			if _icon_button(fa_icons.ICON_FA_CHECK, "Select this set for the whole shape"):
				self._select_multi_bitmap_slot(entries, index)
			if is_active:
				imgui.pop_style_color(3)
			imgui.same_line()

			expanded = index in self._multi_bitmap_expanded
			expand_icon = fa_icons.ICON_FA_CHEVRON_DOWN if expanded else fa_icons.ICON_FA_CHEVRON_RIGHT
			if _icon_button(expand_icon, "Collapse" if expanded else "Expand: edit per-material"):
				if expanded:
					self._multi_bitmap_expanded.discard(index)
				else:
					self._multi_bitmap_expanded.add(index)
			imgui.same_line()

			imgui.text(_multi_bitmap_slot_label(index))
			if doc and imgui.is_item_hovered():
				hovered_hint = doc.summary

			preview = representative.file_names[index] if index < len(representative.file_names) else ""
			imgui.text_disabled(preview or "(empty)")

			if expanded:
				imgui.indent()
				for material_id, texture in entries:
					self._ensure_multi_bitmap_slot(texture, index)
					imgui.push_id(f"mb-mat-{material_id}")

					self._draw_material_color_button(material_id)
					imgui.same_line()

					imgui.set_next_item_width(220)
					current_value = texture.file_names[index]
					changed, new_value = imgui.input_text(f"##text-{index}", current_value)
					new_value = new_value.lower()  # texture names aren't case-sensitive
					if changed and new_value != current_value:
						texture.file_names[index] = new_value
						if texture.selected_index == index:
							texture.file_name = new_value
							self._reapply_material(material_id)
					imgui.same_line()
					if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
						def _on_result(file_name, material_id=material_id, texture=texture, index=index):
							self._ensure_multi_bitmap_slot(texture, index)
							texture.file_names[index] = file_name
							if texture.selected_index == index:
								texture.file_name = file_name
								self._reapply_material(material_id)
						self._start_texture_browse(("multi-bitmap", material_id, index), _on_result)

					imgui.pop_id()
				imgui.unindent()

			imgui.pop_id()
			imgui.separator()

		if hovered_hint:
			self.sysinfo.set_status(hovered_hint, color=_STATUS_HINT_COLOR)
			self._multi_bitmap_hint_shown = True
		elif self._multi_bitmap_hint_shown:
			self.sysinfo.set_status("")
			self._multi_bitmap_hint_shown = False

	def _write_shape(self, path):
		try:
			save_shape(path, self.shape_file)
			self._save_status = f"Saved to {path}"
			print(f"[object_editor] {self._save_status}")
		except (OSError, ShapeWriteError) as exc:
			self._save_status = f"Save failed: {exc}"
			print(f"[object_editor] {self._save_status}")

	def _on_save_clicked(self):
		if self._save_overwrite_confirmed:
			self._write_shape(self._shape_source_path)
		else:
			self._confirm_overwrite_open = True
			imgui.open_popup(_OVERWRITE_POPUP_ID)

	def _draw_save_confirmation_popup(self):
		if not self._confirm_overwrite_open:
			return

		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_OVERWRITE_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text(f"Overwrite this file?\n{self._shape_source_path}")
		imgui.text_wrapped("You won't be asked again this session.")
		imgui.separator()
		if imgui.button("Overwrite"):
			self._save_overwrite_confirmed = True
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
			self._write_shape(self._shape_source_path)
		imgui.same_line()
		if imgui.button("Cancel"):
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
		imgui.end_popup()

	def _poll_save_dialog(self):
		if self._save_dialog is None or not self._save_dialog.ready(0):
			return
		result = self._save_dialog.result()
		self._save_dialog = None
		if result:
			self._write_shape(Path(result))

	def _draw_save_buttons(self):
		if self.shape_file.type_name not in _WRITABLE_SHAPE_TYPES:
			return

		imgui.separator()
		if self._shape_source_path is not None:
			if imgui.button("Save"):
				self._on_save_clicked()
			imgui.same_line()
		else:
			imgui.text_disabled("Save unavailable (loaded from inside a .bnp archive)")

		if imgui.button("Save As..."):
			default_path = str(self._shape_source_path) if self._shape_source_path is not None else ""
			self._save_dialog = pfd.save_file("Save shape as", default_path, ["Ryzom shape", "*.shape"])

		self._draw_save_confirmation_popup()
		self._poll_save_dialog()
		if self._save_status:
			imgui.text_wrapped(self._save_status)

	def _draw_materials_tab(self):
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			imgui.text("No materials.")
			return

		multi_bitmap_ids = {material_id for material_id, _ in self._multi_bitmap_entries()}

		imgui.text("Simple textures")
		imgui.separator()
		simple_ids = [material_id for material_id in range(len(materials)) if material_id not in multi_bitmap_ids]
		if not simple_ids:
			imgui.text_disabled("(none)")
		for material_id in simple_ids:
			self._draw_simple_material_row(material_id, materials[material_id])

		self._draw_multi_bitmap_editor()
		self._poll_texture_browse_dialogs()

	@staticmethod
	def _set_simple_material_texture(material, file_name):
		"""Sets slot 0's texture file name, creating a plain CTextureFile
		there if that slot was empty -- preserves any other texture slots
		the material might have rather than replacing the whole list."""
		if material.textures and material.textures[0] is not None:
			material.textures[0].file_name = file_name
		elif material.textures:
			material.textures[0] = Texture(class_name="CTextureFile", file_name=file_name)
		else:
			material.textures = [Texture(class_name="CTextureFile", file_name=file_name)]

	def _draw_simple_material_row(self, material_id, material):
		imgui.push_id(f"mat-simple-{material_id}")

		self._draw_material_color_button(material_id)
		imgui.same_line()

		texture = material.textures[0] if material.textures else None
		current_value = texture.file_name if (texture and texture.file_name) else ""
		imgui.set_next_item_width(220)
		changed, new_value = imgui.input_text("##text", current_value)
		new_value = new_value.lower()  # texture names aren't case-sensitive
		if changed and new_value != current_value:
			self._set_simple_material_texture(material, new_value)
			self._reapply_material(material_id)
		imgui.same_line()

		if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
			def _on_result(file_name, material_id=material_id, material=material):
				self._set_simple_material_texture(material, file_name)
				self._reapply_material(material_id)
			self._start_texture_browse(("simple", material_id), _on_result)

		imgui.pop_id()

	def draw_panel(self):
		self.nav_cube.draw_controls()
		self.export_dialog.draw()

		if self.shape_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self.shape_error)

		if self.shape_file is None:
			imgui.text("Select a .shape file in the explorer.")
			return

		imgui.text(f"Type: {self.shape_file.type_name}")
		imgui.separator()

		if imgui.begin_tab_bar("##panel-tabs"):
			if imgui.begin_tab_item_simple("Materials"):
				self._draw_materials_tab()
				imgui.end_tab_item()
			if imgui.begin_tab_item_simple("All Properties"):
				draw_properties(self.shape_file.value)
				imgui.end_tab_item()
			imgui.end_tab_bar()

		self._draw_save_buttons()


if __name__ == "__main__":
	ObjectEditorApp().run()
