"""Ryzom Forgery object viewer: browse and inspect .shape files.

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

from imgui_bundle import imgui

from ryzom_forgery.app import ForgeryApp
from ryzom_forgery.camera import OrbitCamera
from ryzom_forgery.asset_index import AssetIndex
from ryzom_forgery.export_dialog import ExportDialog
from ryzom_forgery.navcube import NavigationCube
from ryzom_forgery.properties import draw_properties
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.shape_geometry import iter_render_passes, load_panda_texture, rgba_to_color, shape_bbox

from pynel.ryzom_shape import ShapeParseError, parse_shape

DEFAULT_DATA_ROOT = Path("~/.local/share/Ryzom/ryzom_live/data").expanduser()


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


class ObjectViewerApp(ForgeryApp):
	def __init__(self, data_root=DEFAULT_DATA_ROOT):
		ForgeryApp.__init__(self, explorer_root=data_root, title="Ryzom Forgery - Object Viewer",
		                     explorer_default_filter="*.shape")

		self.data_root = Path(data_root)
		self.asset_index = AssetIndex(self.data_root)
		self.sysinfo.set_status("Indexing assets...")
		self.asset_index.build()
		self.sysinfo.set_status(f"{len(self.asset_index)} assets indexed")

		self._texture_cache = {}

		self.model_root = self.render.attach_new_node("shape-root")
		self.shape_file = None
		self.shape_error = None

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
		print(f"[object_viewer] selection changed: {[item.name for item in items]}")
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
			print(f"[object_viewer] pass {pass_count}: material={material_id} "
			      f"verts={vertex_buffer.num_verts} tris={len(indices) // 3} "
			      f"channels={list(vertex_buffer.channels.keys())}")
			geom = _build_geom(vertex_buffer, indices)
			geom_node = GeomNode(f"pass-{pass_count}")
			geom_node.add_geom(geom)
			node_path = self.model_root.attach_new_node(geom_node)
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
			print(f"[object_viewer] material {material_id}: no texture")
		elif not texture.file_name:
			print(f"[object_viewer] material {material_id}: texture slot present "
			      f"(class={texture.class_name}) but no file_name")
		else:
			print(f"[object_viewer] material {material_id}: texture reference {texture.file_name!r}")
			panda_texture = load_panda_texture(self.asset_index, texture.file_name, cache=self._texture_cache)
			if panda_texture is not None:
				node_path.set_texture(panda_texture)

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
		draw_properties(self.shape_file.value)


if __name__ == "__main__":
	ObjectViewerApp().run()
