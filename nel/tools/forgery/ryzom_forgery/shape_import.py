"""Imports a plain interchange format into a `pynel.ryzom_shape.Mesh`, so it
can be saved as a real `.shape` via `save_shape()`.

Only `CMesh` can be built this way (see `ryzom_shape.py`'s `dumps()`
docstring): unlike `CMeshMRM`, its geometry is written field-by-field rather
than copied byte-for-byte from an already-parsed file, so it's the only
shape type pynel can construct from scratch. A `CMesh` has no LOD levels --
turning an imported mesh into a real progressive-LOD `CMeshMRM` needs
`CMRMBuilder` (`nel/include/nel/3d/mrm_builder.h`), a C++-only class with no
Python binding; out of scope here (see docs/log.md for the investigation).

`.obj`/`.mtl` are hand-parsed here for the same reason `shape_export.py`
hand-writes them: simple, dependency-free text formats.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from pynel.ryzom_shape import (
	AABBox, Material, MatrixBlock, Mesh, MeshBase, MeshGeom, Quaternion, RdrPass, Rgba, Texture, Vector3,
	VertexBuffer,
)

# CMaterial's own documented default-construction values (nel/include/nel/3d/material.h:273):
# "normal shader, SrcBlend is srcalpha, dstblend is invsrcalpha, ZFunction is lessequal, ZBias is 0".
_SHADER_NORMAL = 0
_BLEND_SRCALPHA = 2
_BLEND_INVSRCALPHA = 3
_ZFUNC_LESSEQUAL = 5
_MAT_FLAG_ZWRITE = 0x00000004
_MAT_FLAG_LIGHTING = 0x00000010
_MAT_FLAG_DOUBLE_SIDED = 0x00000100
_DEFAULT_MATERIAL_FLAGS = _MAT_FLAG_ZWRITE | _MAT_FLAG_LIGHTING | _MAT_FLAG_DOUBLE_SIDED

_DEFAULT_MATERIAL_NAME = "__default__"  # faces with no `usemtl` in effect


class ShapeImportError(Exception):
	pass


# ---------------------------------------------------------------------------
# .obj
# ---------------------------------------------------------------------------


@dataclass
class ObjFace:
	material: Optional[str]
	# (position_index, texcoord_index, normal_index), 0-based, already resolved
	# (negative/relative .obj indices converted); texcoord/normal are None if absent.
	corners: List[Tuple[int, Optional[int], Optional[int]]]


@dataclass
class ObjMesh:
	positions: List[Tuple[float, float, float]] = field(default_factory=list)
	normals: List[Tuple[float, float, float]] = field(default_factory=list)
	texcoords: List[Tuple[float, float]] = field(default_factory=list)
	faces: List[ObjFace] = field(default_factory=list)
	mtllib: List[str] = field(default_factory=list)


def _resolve_obj_index(raw: str, count: int) -> int:
	i = int(raw)
	return i - 1 if i > 0 else count + i  # negative = relative to the current count


def _parse_obj_face_corner(token: str, num_positions: int, num_texcoords: int, num_normals: int):
	pieces = token.split("/")
	pos_index = _resolve_obj_index(pieces[0], num_positions)
	uv_index = _resolve_obj_index(pieces[1], num_texcoords) if len(pieces) > 1 and pieces[1] else None
	normal_index = _resolve_obj_index(pieces[2], num_normals) if len(pieces) > 2 and pieces[2] else None
	return (pos_index, uv_index, normal_index)


def parse_obj(path: Path) -> ObjMesh:
	mesh = ObjMesh()
	current_material: Optional[str] = None

	for raw_line in Path(path).read_text().splitlines():
		line = raw_line.strip()
		if not line or line.startswith("#"):
			continue
		parts = line.split()
		keyword = parts[0]

		if keyword == "v":
			mesh.positions.append((float(parts[1]), float(parts[2]), float(parts[3])))
		elif keyword == "vn":
			mesh.normals.append((float(parts[1]), float(parts[2]), float(parts[3])))
		elif keyword == "vt":
			mesh.texcoords.append((float(parts[1]), float(parts[2]) if len(parts) > 2 else 0.0))
		elif keyword == "usemtl":
			current_material = parts[1]
		elif keyword == "mtllib":
			mesh.mtllib.extend(parts[1:])
		elif keyword == "f":
			corners = [
				_parse_obj_face_corner(tok, len(mesh.positions), len(mesh.texcoords), len(mesh.normals))
				for tok in parts[1:]
			]
			# Fan-triangulate n-gons (n>=3), matching common .obj exporter output.
			for i in range(1, len(corners) - 1):
				mesh.faces.append(ObjFace(material=current_material, corners=[corners[0], corners[i], corners[i + 1]]))

	if not mesh.positions:
		raise ShapeImportError(f"no vertices found in {path}")
	if not mesh.faces:
		raise ShapeImportError(f"no faces found in {path}")
	return mesh


# ---------------------------------------------------------------------------
# .mtl
# ---------------------------------------------------------------------------


@dataclass
class MtlMaterial:
	name: str
	diffuse: Tuple[float, float, float] = (0.8, 0.8, 0.8)
	ambient: Tuple[float, float, float] = (0.2, 0.2, 0.2)
	specular: Tuple[float, float, float] = (0.0, 0.0, 0.0)
	shininess: float = 0.0
	opacity: float = 1.0
	diffuse_texture: Optional[str] = None


def parse_mtl(path: Path) -> Dict[str, MtlMaterial]:
	materials: Dict[str, MtlMaterial] = {}
	current: Optional[MtlMaterial] = None

	for raw_line in Path(path).read_text().splitlines():
		line = raw_line.strip()
		if not line or line.startswith("#"):
			continue
		parts = line.split()
		keyword = parts[0]

		if keyword == "newmtl":
			current = MtlMaterial(name=parts[1])
			materials[current.name] = current
		elif current is None:
			continue
		elif keyword == "Kd":
			current.diffuse = (float(parts[1]), float(parts[2]), float(parts[3]))
		elif keyword == "Ka":
			current.ambient = (float(parts[1]), float(parts[2]), float(parts[3]))
		elif keyword == "Ks":
			current.specular = (float(parts[1]), float(parts[2]), float(parts[3]))
		elif keyword == "Ns":
			current.shininess = float(parts[1])
		elif keyword == "d":
			current.opacity = float(parts[1])
		elif keyword == "Tr":
			current.opacity = 1.0 - float(parts[1])
		elif keyword == "map_Kd":
			current.diffuse_texture = parts[-1]  # ignore options (-o/-s/...), keep the trailing file name

	return materials


# ---------------------------------------------------------------------------
# ObjMesh -> pynel Mesh
# ---------------------------------------------------------------------------


def _clamp01(value: float) -> float:
	return max(0.0, min(1.0, value))


def _to_rgba(color: Tuple[float, float, float], alpha: float = 1.0) -> Rgba:
	r, g, b = color
	return Rgba(round(_clamp01(r) * 255), round(_clamp01(g) * 255), round(_clamp01(b) * 255), round(_clamp01(alpha) * 255))


def _build_material(
		diffuse: Tuple[float, float, float] = (0.8, 0.8, 0.8),
		ambient: Tuple[float, float, float] = (0.2, 0.2, 0.2),
		specular: Tuple[float, float, float] = (0.0, 0.0, 0.0),
		shininess: float = 0.0,
		opacity: float = 1.0,
		texture_name: Optional[str] = None) -> Material:
	textures: List[Optional[Texture]] = []
	if texture_name:
		textures = [Texture(class_name="CTextureFile", file_name=Path(texture_name).name.lower(), allow_degradation=True)]

	return Material(
		shader_type=_SHADER_NORMAL,
		flags=_DEFAULT_MATERIAL_FLAGS,
		src_blend=_BLEND_SRCALPHA,
		dst_blend=_BLEND_INVSRCALPHA,
		z_function=_ZFUNC_LESSEQUAL,
		z_bias=0.0,
		color=Rgba(255, 255, 255, 255),
		emissive=Rgba(0, 0, 0, 255),
		ambient=_to_rgba(ambient),
		diffuse=_to_rgba(diffuse, opacity),
		specular=_to_rgba(specular),
		shininess=shininess,
		alpha_test_threshold=0.5,
		tex_coord_gen_mode=0,
		textures=textures,
	)


def _assemble_mesh(
		positions: List[Tuple[float, float, float]], normals: List[Tuple[float, float, float]],
		texcoords: List[Tuple[float, float]], materials: List[Material], rdr_passes: List[RdrPass]) -> Mesh:
	"""Shared final assembly step for every importer: a single-matrix-block,
	unskinned CMesh from already-deduplicated vertex channels."""
	channels = {"Position": positions}
	types = [0] * 16
	types[0] = 7  # Position: float3
	if normals:
		channels["Normal"] = normals
		types[1] = 7  # Normal: float3
	if texcoords:
		channels["TexCoord0"] = texcoords
		types[2] = 4  # TexCoord0: float2

	vertex_buffer = VertexBuffer(
		name="", num_verts=len(positions), vertex_color_format=0, channels=channels, types=types)
	matrix_block = MatrixBlock(matrix_id=[0] * 16, num_matrix=0, rdr_passes=rdr_passes)

	xs, ys, zs = [p[0] for p in positions], [p[1] for p in positions], [p[2] for p in positions]
	min_v, max_v = (min(xs), min(ys), min(zs)), (max(xs), max(ys), max(zs))
	bbox = AABBox(
		center=Vector3(*((lo + hi) / 2 for lo, hi in zip(min_v, max_v))),
		half_size=Vector3(*((hi - lo) / 2 for lo, hi in zip(min_v, max_v))),
	)

	base = MeshBase(
		materials=materials,
		default_pos=Vector3(0.0, 0.0, 0.0),
		default_pivot=Vector3(0.0, 0.0, 0.0),
		default_rot_euler=Vector3(0.0, 0.0, 0.0),
		default_rot_quat=Quaternion(0.0, 0.0, 0.0, 1.0),
		default_scale=Vector3(1.0, 1.0, 1.0),
	)
	geom = MeshGeom(
		bones_name=[], mesh_morpher=None, vertex_buffer=vertex_buffer,
		matrix_blocks=[matrix_block], bbox=bbox, skinned=False,
	)
	return Mesh(base=base, geom=geom)


def build_mesh(obj_mesh: ObjMesh, mtl_materials: Dict[str, MtlMaterial]) -> Mesh:
	has_normals = bool(obj_mesh.normals)
	has_uvs = bool(obj_mesh.texcoords)

	positions: List[Tuple[float, float, float]] = []
	normals: List[Tuple[float, float, float]] = []
	texcoords: List[Tuple[float, float]] = []
	combined_index: Dict[Tuple[int, Optional[int], Optional[int]], int] = {}

	def combined_vertex_id(corner: Tuple[int, Optional[int], Optional[int]]) -> int:
		if corner in combined_index:
			return combined_index[corner]
		pos_index, uv_index, normal_index = corner
		index = len(positions)
		positions.append(obj_mesh.positions[pos_index])
		if has_normals:
			normals.append(obj_mesh.normals[normal_index] if normal_index is not None else (0.0, 0.0, 1.0))
		if has_uvs:
			texcoords.append(obj_mesh.texcoords[uv_index] if uv_index is not None else (0.0, 0.0))
		combined_index[corner] = index
		return index

	material_order: List[str] = []  # first-seen order, becomes material_id order
	material_ids: Dict[str, int] = {}
	pass_indices: Dict[int, List[int]] = {}

	def material_id_for(name: str) -> int:
		if name not in material_ids:
			material_ids[name] = len(material_order)
			material_order.append(name)
			pass_indices[material_ids[name]] = []
		return material_ids[name]

	for obj_face in obj_mesh.faces:
		material_id = material_id_for(obj_face.material or _DEFAULT_MATERIAL_NAME)
		for corner in obj_face.corners:
			pass_indices[material_id].append(combined_vertex_id(corner))

	def material_for(name: str) -> Material:
		mtl = mtl_materials.get(name)
		if mtl is None:
			return _build_material()
		return _build_material(
			diffuse=mtl.diffuse, ambient=mtl.ambient, specular=mtl.specular,
			shininess=mtl.shininess, opacity=mtl.opacity, texture_name=mtl.diffuse_texture)

	materials = [material_for(name) for name in material_order]
	rdr_passes = [RdrPass(material_id=material_ids[name], indices=pass_indices[material_ids[name]])
	              for name in material_order]
	return _assemble_mesh(positions, normals, texcoords, materials, rdr_passes)


def import_obj(path: Path) -> Mesh:
	"""Parses `path` (an .obj) and its referenced .mtl (if any, resolved
	relative to the .obj's own folder), returning a ready-to-save Mesh."""
	path = Path(path)
	obj_mesh = parse_obj(path)

	mtl_materials: Dict[str, MtlMaterial] = {}
	for mtllib_name in obj_mesh.mtllib:
		mtl_path = path.parent / mtllib_name
		if mtl_path.is_file():
			mtl_materials.update(parse_mtl(mtl_path))

	return build_mesh(obj_mesh, mtl_materials)


# ---------------------------------------------------------------------------
# .dae
# ---------------------------------------------------------------------------


def _dae_effect_rgb(value, default: Tuple[float, float, float]) -> Tuple[float, float, float]:
	"""An Effect property (diffuse/ambient/specular/...) is either a plain
	RGBA tuple or a `collada.material.Map` (textured) -- textured properties
	fall back to `default` here, same as .obj materials with no `Kd` line."""
	from collada.material import Map
	if value is None or isinstance(value, Map):
		return default
	return (value[0], value[1], value[2])


def _dae_effect_texture_name(value) -> Optional[str]:
	from collada.material import Map
	if not isinstance(value, Map):
		return None
	try:
		return Path(value.sampler.surface.image.path).name
	except AttributeError:
		return None


def _build_material_from_dae_material(dae_material) -> Material:
	"""`dae_material` is a bound `collada.material.Material` (`Triangle.material`
	on a `BoundTriangleSet`) -- its `.effect` carries the actual shading data."""
	effect = dae_material.effect if dae_material is not None else None
	if effect is None:
		return _build_material()

	opacity = 1.0
	transparency = getattr(effect, "transparency", None)
	if isinstance(transparency, (int, float)):
		opacity = float(transparency)

	return _build_material(
		diffuse=_dae_effect_rgb(effect.diffuse, (0.8, 0.8, 0.8)),
		ambient=_dae_effect_rgb(effect.ambient, (0.2, 0.2, 0.2)),
		specular=_dae_effect_rgb(effect.specular, (0.0, 0.0, 0.0)),
		shininess=float(effect.shininess or 0.0),
		opacity=opacity,
		texture_name=_dae_effect_texture_name(effect.diffuse),
	)


def import_dae(path: Path) -> Mesh:
	"""Parses `path` (a COLLADA .dae) via `pycollada`, returning a
	ready-to-save Mesh. Only triangulated geometry is read (COLLADA's
	<polylist>/<lines>/... primitive types are skipped)."""
	from collada import Collada

	doc = Collada(str(path))

	positions: List[Tuple[float, float, float]] = []
	normals: List[Tuple[float, float, float]] = []
	texcoords: List[Tuple[float, float]] = []
	combined_index: Dict[Tuple[Tuple[float, ...], Optional[Tuple[float, ...]], Optional[Tuple[float, ...]]], int] = {}

	material_order: List[str] = []  # first-seen order, keyed by the bound Effect's id (or _DEFAULT_MATERIAL_NAME)
	material_ids: Dict[str, int] = {}
	material_by_name: Dict[str, object] = {}  # name -> bound collada.material.Material (or None)
	pass_indices: Dict[int, List[int]] = {}

	def material_id_for(dae_material) -> int:
		name = dae_material.id if dae_material is not None else _DEFAULT_MATERIAL_NAME
		if name not in material_ids:
			material_ids[name] = len(material_order)
			material_order.append(name)
			material_by_name[name] = dae_material
			pass_indices[material_ids[name]] = []
		return material_ids[name]

	def combined_vertex_id(position, normal, uv) -> int:
		key = (tuple(position), tuple(normal) if normal is not None else None, tuple(uv) if uv is not None else None)
		if key in combined_index:
			return combined_index[key]
		index = len(positions)
		positions.append(key[0])
		if normal is not None:
			normals.append(key[1])
		if uv is not None:
			texcoords.append((key[2][0], key[2][1]))
		combined_index[key] = index
		return index

	for bound_geom in doc.scene.objects("geometry"):
		for primitive in bound_geom.primitives():
			if not hasattr(primitive, "triangles"):
				continue  # skip non-triangle primitives (lines, unsupported polylists, ...)
			for tri in primitive.triangles():
				material_id = material_id_for(tri.material)
				for i in range(3):
					normal = tri.normals[i] if tri.normals is not None else None
					uv = tri.texcoords[0][i] if tri.texcoords else None
					pass_indices[material_id].append(combined_vertex_id(tri.vertices[i], normal, uv))

	if not positions:
		raise ShapeImportError(f"no triangles found in {path}")

	materials = [_build_material_from_dae_material(material_by_name[name]) for name in material_order]
	rdr_passes = [RdrPass(material_id=material_ids[name], indices=pass_indices[material_ids[name]])
	              for name in material_order]
	return _assemble_mesh(positions, normals, texcoords, materials, rdr_passes)
