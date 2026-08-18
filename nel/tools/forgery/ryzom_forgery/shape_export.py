"""Exports a parsed .shape (CMesh/CMeshMRM/CMeshMultiLod) to plain
interchange formats.

.obj+.mtl and .stl are hand-written directly from the parsed vertex
buffers/indices -- both are simple enough text formats that a small
dependency-free writer beats pulling in a mesh library. .stl has no
material/texture support at all (it's geometry only), unlike .obj and .dae.

.dae (COLLADA) is XML-heavy enough that hand-writing it isn't worth it --
`pycollada` builds the document instead, referencing texture images by
filename only (no PIL/pixel decoding needed on its side either). .gltf/.glb
similarly go through `pygltflib` rather than a hand-written binary packer,
for the same reason: getting buffer/accessor byte layout exactly right by
hand is easy to get subtly wrong.
"""

from dataclasses import dataclass
from pathlib import Path
from typing import Callable, List, Optional

from ryzom_forgery.export_config import TEXTURE_MODE_COPY_PNG
from ryzom_forgery.shape_geometry import iter_render_passes, load_panda_texture, rgba_to_color, texture_to_pnm_image


@dataclass
class ExportFormat:
	extension: str
	label: str
	supports_materials: bool
	# (shape_value, materials, output_path, texture_mode, asset_index) -> list[Path] of files written
	export: Callable


def _resolve_material_texture(asset_index, material, output_dir: Path, texture_mode: str, texture_cache: dict):
	"""Returns the texture file name to reference in an exported material
	(or None if it has none), writing a decoded .png copy next to the
	export first if `texture_mode` asks for it. `texture_cache` is reused
	across materials in one export so the same texture isn't decoded/written
	twice."""
	if not material.textures or not material.textures[0] or not material.textures[0].file_name:
		return None
	source_name = material.textures[0].file_name

	if texture_mode != TEXTURE_MODE_COPY_PNG:
		return source_name

	png_name = Path(source_name).stem + ".png"
	if png_name in texture_cache:
		return png_name if texture_cache[png_name] else None

	panda_texture = load_panda_texture(asset_index, source_name)
	if panda_texture is None:
		texture_cache[png_name] = False
		return None

	image = texture_to_pnm_image(panda_texture)
	image.write(str(output_dir / png_name))
	texture_cache[png_name] = True
	return png_name


def _export_obj(shape_value, materials, output_path: Path, texture_mode: str, asset_index) -> List[Path]:
	output_dir = output_path.parent
	mtl_path = output_path.with_suffix(".mtl")

	obj_lines = ["# Exported by Ryzom Forgery\n", f"mtllib {mtl_path.name}\n"]
	# OBJ's v/vn/vt indices are global to the whole file, and passes that
	# share the same underlying vertex buffer (very common -- a Mesh's
	# matrix block or a MeshMRM's finest LOD hands the exact same buffer to
	# every one of its render passes, only the index/material differ) must
	# only get their v/vn/vt lines written once, not duplicated per pass.
	v_count = vn_count = vt_count = 0
	buffer_offsets = {}
	used_material_ids = []

	for vertex_buffer, material_id, indices in iter_render_passes(shape_value):
		if not indices:
			continue
		positions = vertex_buffer.channels.get("Position")
		if not positions:
			continue
		normals = vertex_buffer.channels.get("Normal")
		texcoords = vertex_buffer.channels.get("TexCoord0")

		key = id(vertex_buffer)
		if key not in buffer_offsets:
			v_base = v_count
			for p in positions:
				obj_lines.append(f"v {p[0]} {p[1]} {p[2]}\n")
			v_count += len(positions)

			vn_base: Optional[int] = None
			if normals:
				vn_base = vn_count
				for n in normals:
					obj_lines.append(f"vn {n[0]} {n[1]} {n[2]}\n")
				vn_count += len(normals)

			vt_base: Optional[int] = None
			if texcoords:
				vt_base = vt_count
				for uv in texcoords:
					obj_lines.append(f"vt {uv[0]} {uv[1]}\n")
				vt_count += len(texcoords)

			buffer_offsets[key] = (v_base, vn_base, vt_base)

		v_base, vn_base, vt_base = buffer_offsets[key]

		if material_id not in used_material_ids:
			used_material_ids.append(material_id)
		obj_lines.append(f"usemtl material_{material_id}\n")

		for i in range(0, len(indices), 3):
			face_tokens = []
			for k in range(3):
				local_idx = indices[i + k]
				v_idx = local_idx + v_base + 1
				if vt_base is not None and vn_base is not None:
					token = f"{v_idx}/{local_idx + vt_base + 1}/{local_idx + vn_base + 1}"
				elif vt_base is not None:
					token = f"{v_idx}/{local_idx + vt_base + 1}"
				elif vn_base is not None:
					token = f"{v_idx}//{local_idx + vn_base + 1}"
				else:
					token = f"{v_idx}"
				face_tokens.append(token)
			obj_lines.append(f"f {' '.join(face_tokens)}\n")

	if v_count == 0:
		raise ValueError("No renderable geometry to export")

	written = [output_path, mtl_path]
	texture_cache: dict = {}
	mtl_lines = []
	for material_id in used_material_ids:
		material = materials[material_id] if materials and material_id < len(materials) else None
		mtl_lines.append(f"newmtl material_{material_id}\n")
		if material is not None:
			diffuse = rgba_to_color(material.diffuse)
			mtl_lines.append(f"Kd {diffuse[0]} {diffuse[1]} {diffuse[2]}\n")
			mtl_lines.append(f"d {diffuse[3]}\n")
			texture_name = _resolve_material_texture(asset_index, material, output_dir, texture_mode, texture_cache)
			if texture_name:
				mtl_lines.append(f"map_Kd {texture_name}\n")
				written.append(output_dir / texture_name)
		mtl_lines.append("\n")

	output_path.write_text("".join(obj_lines))
	mtl_path.write_text("".join(mtl_lines))
	return written


def _triangle_normal(a, b, c):
	ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
	vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
	nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
	length = (nx * nx + ny * ny + nz * nz) ** 0.5
	if length == 0.0:
		return (0.0, 0.0, 0.0)
	return (nx / length, ny / length, nz / length)


def _export_stl(shape_value, materials, output_path: Path, texture_mode: str, asset_index) -> List[Path]:
	# Geometry only: STL has no concept of materials/UVs, so every pass is
	# just merged into one flat triangle soup, with a fresh per-triangle
	# facet normal computed from the winding (not reused from the source
	# vertex normals, which is the STL convention).
	lines = [f"solid {output_path.stem}\n"]
	triangle_count = 0

	for vertex_buffer, _material_id, indices in iter_render_passes(shape_value):
		if not indices:
			continue
		positions = vertex_buffer.channels.get("Position")
		if not positions:
			continue

		for i in range(0, len(indices), 3):
			a, b, c = positions[indices[i]], positions[indices[i + 1]], positions[indices[i + 2]]
			normal = _triangle_normal(a, b, c)
			lines.append(f"  facet normal {normal[0]} {normal[1]} {normal[2]}\n")
			lines.append("    outer loop\n")
			for vertex in (a, b, c):
				lines.append(f"      vertex {vertex[0]} {vertex[1]} {vertex[2]}\n")
			lines.append("    endloop\n")
			lines.append("  endfacet\n")
			triangle_count += 1

	if triangle_count == 0:
		raise ValueError("No renderable geometry to export")

	lines.append(f"endsolid {output_path.stem}\n")
	output_path.write_text("".join(lines))
	return [output_path]


def _export_dae(shape_value, materials, output_path: Path, texture_mode: str, asset_index) -> List[Path]:
	import numpy
	from collada import Collada
	from collada import geometry as col_geometry
	from collada import material as col_material
	from collada import scene as col_scene
	from collada import source as col_source

	doc = Collada()
	written = [output_path]
	texture_cache: dict = {}
	created_materials = {}
	geometry_nodes = []
	pass_index = 0

	for vertex_buffer, material_id, indices in iter_render_passes(shape_value):
		if not indices:
			continue
		positions = vertex_buffer.channels.get("Position")
		if not positions:
			continue
		normals = vertex_buffer.channels.get("Normal")
		texcoords = vertex_buffer.channels.get("TexCoord0")

		if material_id not in created_materials:
			material = materials[material_id] if materials and material_id < len(materials) else None
			diffuse = rgba_to_color(material.diffuse) if material is not None else (0.8, 0.8, 0.8, 1.0)

			texture_name = None
			if material is not None:
				texture_name = _resolve_material_texture(
					asset_index, material, output_path.parent, texture_mode, texture_cache)

			if texture_name:
				image = col_material.CImage(f"image_{material_id}", texture_name)
				doc.images.append(image)
				surface = col_material.Surface(f"surface_{material_id}", image)
				sampler = col_material.Sampler2D(f"sampler_{material_id}", surface)
				effect = col_material.Effect(
					f"effect_{material_id}", [surface, sampler], "phong",
					diffuse=col_material.Map(sampler, "UVSET0"))
				written.append(output_path.parent / texture_name)
			else:
				effect = col_material.Effect(f"effect_{material_id}", [], "phong", diffuse=diffuse)
			doc.effects.append(effect)

			col_mat = col_material.Material(f"material_{material_id}", f"material_{material_id}", effect)
			doc.materials.append(col_mat)
			created_materials[material_id] = col_mat

		col_mat = created_materials[material_id]

		sources = [col_source.FloatSource(
			f"verts-{pass_index}", numpy.array(positions, dtype=float).flatten(), ("X", "Y", "Z"))]
		input_list = col_source.InputList()
		input_list.addInput(0, "VERTEX", f"#verts-{pass_index}")

		if normals:
			sources.append(col_source.FloatSource(
				f"normals-{pass_index}", numpy.array(normals, dtype=float).flatten(), ("X", "Y", "Z")))
			input_list.addInput(1, "NORMAL", f"#normals-{pass_index}")

		if texcoords:
			sources.append(col_source.FloatSource(
				f"uv-{pass_index}", numpy.array(texcoords, dtype=float).flatten(), ("S", "T")))
			input_list.addInput(len(sources) - 1, "TEXCOORD", f"#uv-{pass_index}", set="0")

		geom = col_geometry.Geometry(doc, f"geometry-{pass_index}", f"pass_{pass_index}", sources)
		num_inputs = len(sources)
		flat_indices = numpy.repeat(numpy.array(indices), num_inputs)
		triset = geom.createTriangleSet(flat_indices, input_list, f"material_{material_id}")
		geom.primitives.append(triset)
		doc.geometries.append(geom)

		mat_inputs = [("UVSET0", "TEXCOORD", "0")] if texcoords else []
		mat_node = col_scene.MaterialNode(f"material_{material_id}", col_mat, inputs=mat_inputs)
		geometry_nodes.append(col_scene.GeometryNode(geom, [mat_node]))
		pass_index += 1

	if not geometry_nodes:
		raise ValueError("No renderable geometry to export")

	node = col_scene.Node("shape_root", children=geometry_nodes)
	my_scene = col_scene.Scene("scene", [node])
	doc.scenes.append(my_scene)
	doc.scene = my_scene
	doc.write(str(output_path))
	return written


def _build_gltf_document(shape_value, materials, output_path: Path, texture_mode: str, asset_index):
	"""Shared by .gltf and .glb -- pygltflib's `GLTF2.save()` picks the JSON
	vs. binary container based on the output path's extension, so the
	document itself is built identically for both."""
	import struct

	import pygltflib

	doc = pygltflib.GLTF2()
	blob = bytearray()
	texture_cache: dict = {}
	created_materials = {}
	created_images = {}
	primitives = []
	written = [output_path]

	def add_accessor(packed_bytes, component_type, accessor_type, count, target, bounds=None):
		offset = len(blob)
		blob.extend(packed_bytes)
		while len(blob) % 4 != 0:  # keep every bufferView 4-byte aligned
			blob.append(0)
		doc.bufferViews.append(pygltflib.BufferView(
			buffer=0, byteOffset=offset, byteLength=len(packed_bytes), target=target))
		accessor = pygltflib.Accessor(
			bufferView=len(doc.bufferViews) - 1, componentType=component_type, count=count, type=accessor_type)
		if bounds is not None:
			accessor.min, accessor.max = bounds
		doc.accessors.append(accessor)
		return len(doc.accessors) - 1

	for vertex_buffer, material_id, indices in iter_render_passes(shape_value):
		if not indices:
			continue
		positions = vertex_buffer.channels.get("Position")
		if not positions:
			continue
		normals = vertex_buffer.channels.get("Normal")
		texcoords = vertex_buffer.channels.get("TexCoord0")

		xs, ys, zs = [p[0] for p in positions], [p[1] for p in positions], [p[2] for p in positions]
		pos_bytes = struct.pack(f"<{len(positions) * 3}f", *(c for p in positions for c in p))
		attributes = pygltflib.Attributes(POSITION=add_accessor(
			pos_bytes, pygltflib.FLOAT, pygltflib.VEC3, len(positions), pygltflib.ARRAY_BUFFER,
			bounds=([min(xs), min(ys), min(zs)], [max(xs), max(ys), max(zs)])))

		if normals:
			normal_bytes = struct.pack(f"<{len(normals) * 3}f", *(c for n in normals for c in n))
			attributes.NORMAL = add_accessor(
				normal_bytes, pygltflib.FLOAT, pygltflib.VEC3, len(normals), pygltflib.ARRAY_BUFFER)

		if texcoords:
			uv_bytes = struct.pack(f"<{len(texcoords) * 2}f", *(c for uv in texcoords for c in uv))
			attributes.TEXCOORD_0 = add_accessor(
				uv_bytes, pygltflib.FLOAT, pygltflib.VEC2, len(texcoords), pygltflib.ARRAY_BUFFER)

		index_bytes = struct.pack(f"<{len(indices)}I", *indices)
		index_accessor = add_accessor(
			index_bytes, pygltflib.UNSIGNED_INT, pygltflib.SCALAR, len(indices), pygltflib.ELEMENT_ARRAY_BUFFER)

		if material_id not in created_materials:
			material = materials[material_id] if materials and material_id < len(materials) else None
			diffuse = rgba_to_color(material.diffuse) if material is not None else (0.8, 0.8, 0.8, 1.0)
			pbr = pygltflib.PbrMetallicRoughness(
				baseColorFactor=list(diffuse), metallicFactor=0.0, roughnessFactor=1.0)

			texture_name = None
			if material is not None:
				texture_name = _resolve_material_texture(
					asset_index, material, output_path.parent, texture_mode, texture_cache)
			if texture_name:
				if texture_name not in created_images:
					doc.images.append(pygltflib.Image(uri=texture_name))
					created_images[texture_name] = len(doc.images) - 1
					written.append(output_path.parent / texture_name)
				doc.textures.append(pygltflib.Texture(source=created_images[texture_name]))
				pbr.baseColorTexture = pygltflib.TextureInfo(index=len(doc.textures) - 1)

			doc.materials.append(pygltflib.Material(pbrMetallicRoughness=pbr, name=f"material_{material_id}"))
			created_materials[material_id] = len(doc.materials) - 1

		primitives.append(pygltflib.Primitive(
			attributes=attributes, indices=index_accessor, material=created_materials[material_id]))

	if not primitives:
		raise ValueError("No renderable geometry to export")

	doc.meshes.append(pygltflib.Mesh(primitives=primitives))
	doc.nodes.append(pygltflib.Node(mesh=0))
	doc.scenes.append(pygltflib.Scene(nodes=[0]))
	doc.scene = 0
	doc.buffers.append(pygltflib.Buffer(byteLength=len(blob)))
	doc.set_binary_blob(bytes(blob))

	return doc, written


def _export_gltf_or_glb(shape_value, materials, output_path: Path, texture_mode: str, asset_index) -> List[Path]:
	doc, written = _build_gltf_document(shape_value, materials, output_path, texture_mode, asset_index)
	doc.save(str(output_path))
	if output_path.suffix.lower() == ".gltf":
		# .glb embeds the binary blob in the file itself; .gltf (JSON) instead
		# writes it out as a same-named companion .bin file -- confirmed live.
		written.append(output_path.with_suffix(".bin"))
	return written


EXPORT_FORMATS = [
	ExportFormat("obj", "Wavefront OBJ", supports_materials=True, export=_export_obj),
	ExportFormat("dae", "COLLADA", supports_materials=True, export=_export_dae),
	ExportFormat("stl", "STL", supports_materials=False, export=_export_stl),
	ExportFormat("gltf", "glTF (JSON)", supports_materials=True, export=_export_gltf_or_glb),
	ExportFormat("glb", "glTF (binary)", supports_materials=True, export=_export_gltf_or_glb),
]


def export_shape(
		shape_value, name: str, export_format: ExportFormat, output_dir, texture_mode: str, asset_index,
) -> List[Path]:
	"""Exports an already-parsed shape value -- e.g. the live, possibly-edited
	state of the shape currently open in the editor -- via `export_format`
	into `output_dir`. `name` supplies the output file's stem (typically the
	source .shape's own file name). Returns the list of files written.
	Raises `ValueError` (unsupported shape type / no renderable geometry) on
	failure."""
	materials = getattr(shape_value, "materials", None)
	stem = Path(name).stem
	output_path = Path(output_dir) / f"{stem}.{export_format.extension}"
	return export_format.export(shape_value, materials, output_path, texture_mode, asset_index)
