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
hand-writes them: simple, dependency-free text formats. `.dae` and `.fbx` go
through `assimp-py` instead (see `_import_via_assimp()`) -- both are complex
enough formats that hand-parsing isn't worth it once a real importer library
is already a dependency.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from pynel.ryzom_shape import (
	AABBox, Material, MatrixBlock, Mesh, MeshBase, MeshGeom, Quaternion, RdrPass, Rgba, Texture, TexEnv, Vector3,
	VertexBuffer,
)

# CMaterial's own documented default-construction values (nel/include/nel/3d/material.h:273):
# "normal shader, SrcBlend is srcalpha, dstblend is invsrcalpha, ZFunction is lessequal, ZBias is 0".
_SHADER_NORMAL = 0
_ZFUNC_LESSEQUAL = 5
_MAT_FLAG_ZWRITE = 0x00000004
_MAT_FLAG_LIGHTING = 0x00000010
_MAT_FLAG_DOUBLE_SIDED = 0x00000100
_DEFAULT_MATERIAL_FLAGS = _MAT_FLAG_ZWRITE | _MAT_FLAG_LIGHTING

_DEFAULT_MATERIAL_NAME = "__default__"  # faces with no `usemtl` in effect

# _build_material()'s fixed defaults, deliberately NOT read from the source
# .obj/.dae/.fbx's own diffuse/ambient/specular/opacity/shininess -- these
# match the real Ryzom content pipeline instead: the 3ds Max NeL exporter
# (nel/tools/3d/plugin_max/nel_mesh_lib/export_material.cpp) reads every one
# of these off the NeL-material-plugin instance the artist assigns in 3ds Max
# itself, which is unrelated to whatever the imported source file's own
# material data says -- an artist who leaves that plugin's fields untouched
# (typical for a simple textured prop) gets exactly these values, confirmed
# by comparing an artist's real Cinema4D->.fbx->3dsMax->.shape export against
# the same .fbx imported directly through here (see docs/log.md). Only the
# diffuse texture reference carries over from the source file.
_NEL_DEFAULT_GRAY = Rgba(150, 150, 150, 255)  # 3ds Max's own "Standard material" default diffuse/ambient swatch
_NEL_DEFAULT_SPECULAR = Rgba(0, 0, 0, 0)
_NEL_DEFAULT_SHININESS = 8.0  # 2^(glossiness*10)*4 at 3ds Max's default 10% Glossiness
_NEL_DEFAULT_DIST_MAX = 1000.0

# CMaterial::setShader()'s own documented default stage-0 texture blending
# once a texture is assigned (material.h:427-429): modulate the texture by
# the material's Diffuse color/alpha (RGBArg1/AlphaArg1 = Diffuse rather than
# the engine's own baseline default of Previous) -- what 3ds Max's NeL
# exporter writes for a texture whose "Texture Shader" rollout was left at
# its own default setting (confirmed the same way as the constants above).
_MODULATE_TEX_ENV = TexEnv(
	op_rgb=1, src_arg0_rgb=0, op_arg0_rgb=0, src_arg1_rgb=2, op_arg1_rgb=0,
	op_alpha=1, src_arg0_alpha=0, op_arg0_alpha=2, src_arg1_alpha=2, op_arg1_alpha=2,
	constant_color=Rgba(255, 255, 255, 255),
	src_arg2_rgb=1, op_arg2_rgb=0, src_arg2_alpha=1, op_arg2_alpha=2,
)


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


def _texture_base_name(texture_name: str) -> str:
	"""Reduces a texture reference to a plain base file name, as Ryzom
	Texture.file_name always is -- an imported .fbx/.dae routinely carries an
	absolute path instead (the exporting tool's own disk location, useless on
	another machine), and on a Windows-authored file that path uses
	backslashes, which pathlib.Path().name leaves untouched on POSIX (it only
	splits on `/`), so both separators are normalized here before taking the
	last component."""
	return Path(texture_name.replace("\\", "/")).name


def _build_material(texture_name: Optional[str] = None, double_sided: bool = False) -> Material:
	"""Builds a "blank NeL material" (see _NEL_DEFAULT_GRAY et al above) with
	just a diffuse texture reference -- the only thing that reliably carries
	over from an imported .obj/.dae/.fbx's own material data (see the module
	comment there for why the rest deliberately doesn't). `double_sided` is
	the one exception: unlike color/shininess/etc (an artistic choice that
	gets manually redone in 3ds Max anyway), it's a real geometric necessity
	(thin panels/foliage with no back faces) that the source file's own
	material setting is worth honoring."""
	textures: List[Optional[Texture]] = []
	tex_envs: List[Optional[TexEnv]] = []
	if texture_name:
		textures = [Texture(class_name="CTextureFile", file_name=_texture_base_name(texture_name).lower(), allow_degradation=True)]
		tex_envs = [_MODULATE_TEX_ENV]

	flags = _DEFAULT_MATERIAL_FLAGS | (_MAT_FLAG_DOUBLE_SIDED if double_sided else 0)
	return Material(
		shader_type=_SHADER_NORMAL,
		flags=flags,
		src_blend=0,
		dst_blend=0,
		z_function=_ZFUNC_LESSEQUAL,
		z_bias=0.0,
		color=Rgba(255, 255, 255, 255),
		emissive=Rgba(0, 0, 0, 255),
		ambient=_NEL_DEFAULT_GRAY,
		diffuse=_NEL_DEFAULT_GRAY,
		specular=_NEL_DEFAULT_SPECULAR,
		shininess=_NEL_DEFAULT_SHININESS,
		alpha_test_threshold=0.5,
		tex_coord_gen_mode=0,
		textures=textures,
		tex_envs=tex_envs,
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
		dist_max=_NEL_DEFAULT_DIST_MAX,
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
		return _build_material(texture_name=mtl.diffuse_texture if mtl is not None else None)

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
# .dae / .fbx (via assimp-py)
# ---------------------------------------------------------------------------
#
# Both formats go through the same Assimp-based path: Assimp auto-detects the
# format from content/extension, and exposes the same Scene/Mesh/Material API
# regardless of source, so there's nothing format-specific left to write once
# node transforms are handled generically (see _iter_mesh_instances()).
#
# .dae used to be hand-parsed via pycollada instead (kept no longer, see git
# history) -- swapped once assimp-py was already a dependency for .fbx,
# confirmed to produce identical vertex positions on Forgery's own .dae
# exports (which write a `Y_UP` <asset><up_axis> tag via pycollada's own
# default while leaving vertex data in Ryzom's actual Z-up coordinates
# untouched -- Assimp does not auto-rotate based on that tag, verified
# against `test.dae`, so re-importing Forgery's own exports round-trips fine).

# 4x4 identity, row-major (Assimp's own aiMatrix4x4 convention: each inner
# tuple is one row, translation is the 4th element of the first 3 rows --
# assimp-py's Node.transformation is documented to mirror this verbatim).
_IDENTITY_MATRIX = ((1.0, 0.0, 0.0, 0.0), (0.0, 1.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, 0.0, 0.0, 1.0))


def _mat_mul_mat(a, b):
	return tuple(tuple(sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)) for i in range(4))


def _mat_mul_point(m, p):
	x, y, z = p
	return (
		m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3],
		m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3],
		m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3],
	)


def _mat_mul_dir(m, v):
	"""Like _mat_mul_point() but ignoring translation (for normals) -- uses
	the matrix's own upper-left 3x3 rather than an inverse-transpose, which
	is only exactly correct for rotation/uniform-scale transforms. Good
	enough here: non-uniform-scaled FBX nodes are rare, and obj/dae import
	don't handle that case either (they have no node transforms at all)."""
	x, y, z = v
	return (
		m[0][0] * x + m[0][1] * y + m[0][2] * z,
		m[1][0] * x + m[1][1] * y + m[1][2] * z,
		m[2][0] * x + m[2][1] * y + m[2][2] * z,
	)


def _normalize(v):
	"""Assimp's generated normals (Process_GenNormals) come back at whatever
	magnitude the cross product happened to produce in the file's original
	(pre Process_GlobalScale) units -- e.g. 100x too long for a
	centimeters-authored FBX -- rather than unit length, so every normal
	needs renormalizing after the node transform is applied, regardless of
	source."""
	x, y, z = v
	length = (x * x + y * y + z * z) ** 0.5
	return (x / length, y / length, z / length) if length > 1e-12 else (0.0, 0.0, 1.0)


def _iter_mesh_instances(node, parent_transform):
	"""Walks the assimp scene graph, yielding (mesh_index, world_transform)
	for every mesh referenced by every node -- a mesh can be instanced by
	more than one node, and each node's own transform (and its ancestors')
	must be baked into that instance's vertices, since a CMesh has no scene
	graph of its own to carry it (see _assemble_mesh()'s single matrix block)."""
	transform = _mat_mul_mat(parent_transform, node.transformation)
	for mesh_index in node.mesh_indices:
		yield mesh_index, transform
	for child in node.children:
		yield from _iter_mesh_instances(child, transform)


def _mesh_positions(mesh):
	data = list(mesh.vertices)
	return [tuple(data[i:i + 3]) for i in range(0, len(data), 3)]


def _mesh_normals(mesh):
	data = list(mesh.normals)
	return [tuple(data[i:i + 3]) for i in range(0, len(data), 3)] if data else None


def _mesh_texcoords(mesh):
	if not mesh.texcoords or not len(mesh.texcoords[0]):
		return None
	data = list(mesh.texcoords[0])
	stride = mesh.num_uv_components[0]
	return [tuple(data[i:i + 2]) for i in range(0, len(data), stride)]


def _build_material_from_assimp_material(material: dict) -> Material:
	"""`material` is one of assimp_py.Scene.materials' plain dicts (property
	name -> value, see assimp-py's own docs) -- only its diffuse texture and
	double-sided flag (if any) are used, see _build_material()'s own
	docstring for why the rest of the source material's own
	COLOR_*/SHININESS/OPACITY is deliberately ignored. TWOSIDED (Assimp's
	own name for AI_MATKEY_TWOSIDED) is only present in the dict at all when
	the source file set it explicitly -- absent means "use the format's own
	default", which for every format Forgery imports is single-sided."""
	import assimp_py

	textures = material.get("TEXTURES", {})
	diffuse_paths = textures.get(assimp_py.TextureType_DIFFUSE)
	return _build_material(
		texture_name=diffuse_paths[0] if diffuse_paths else None,
		double_sided=bool(material.get("TWOSIDED", False)),
	)


def _import_via_assimp(path: Path) -> Mesh:
	"""Parses `path` (.dae or .fbx) via assimp-py, returning a ready-to-save
	Mesh. Unlike the hand-parsed .obj path, both formats carry their own node
	hierarchy with per-node transforms -- baked into each mesh instance's
	vertices here (see _iter_mesh_instances()) since a CMesh has none of its
	own. Each assimp mesh is already an indexed vertex buffer (no manual
	per-corner dedup needed either)."""
	import assimp_py

	path = Path(path)
	# Process_GlobalScale normalizes the scene to 1 unit = 1 meter using the
	# file's own embedded unit metadata -- without it, a file authored in
	# centimeters (the common default, e.g. Blender's FBX exporter) comes back
	# 100x too large, since Assimp otherwise leaves that conversion factor
	# sitting on the root node's scale instead of applying it.
	flags = (assimp_py.Process_Triangulate | assimp_py.Process_JoinIdenticalVertices
	         | assimp_py.Process_GenNormals | assimp_py.Process_GlobalScale)
	scene = assimp_py.import_file(str(path), flags)

	if scene.num_meshes == 0:
		raise ShapeImportError(f"no meshes found in {path}")

	has_normals = any(len(scene.meshes[i].normals) for i in range(scene.num_meshes))
	has_uvs = any(len(scene.meshes[i].texcoords) and len(scene.meshes[i].texcoords[0]) for i in range(scene.num_meshes))

	positions: List[Tuple[float, float, float]] = []
	normals: List[Tuple[float, float, float]] = []
	texcoords: List[Tuple[float, float]] = []
	material_order: List[int] = []  # first-seen order of assimp material indices, becomes RdrPass material_id order
	pass_indices: Dict[int, List[int]] = {}

	for mesh_index, transform in _iter_mesh_instances(scene.root_node, _IDENTITY_MATRIX):
		mesh = scene.meshes[mesh_index]
		mesh_positions = _mesh_positions(mesh)
		mesh_normals = _mesh_normals(mesh)
		mesh_texcoords = _mesh_texcoords(mesh)
		base_index = len(positions)

		for i, position in enumerate(mesh_positions):
			positions.append(_mat_mul_point(transform, position))
			if has_normals:
				normal = mesh_normals[i] if mesh_normals else (0.0, 0.0, 1.0)
				normals.append(_normalize(_mat_mul_dir(transform, normal)))
			if has_uvs:
				texcoords.append(mesh_texcoords[i] if mesh_texcoords else (0.0, 0.0))

		if mesh.material_index not in pass_indices:
			material_order.append(mesh.material_index)
			pass_indices[mesh.material_index] = []
		pass_indices[mesh.material_index].extend(base_index + index for index in mesh.indices)

	if not positions:
		raise ShapeImportError(f"no vertices found in {path}")

	materials = [_build_material_from_assimp_material(scene.materials[mid]) for mid in material_order]
	rdr_passes = [RdrPass(material_id=i, indices=pass_indices[mid]) for i, mid in enumerate(material_order)]
	return _assemble_mesh(positions, normals, texcoords, materials, rdr_passes)


def import_dae(path: Path) -> Mesh:
	"""Parses `path` (a COLLADA .dae) via assimp-py, returning a ready-to-save
	Mesh -- see _import_via_assimp()."""
	return _import_via_assimp(path)


def import_fbx(path: Path) -> Mesh:
	"""Parses `path` (an FBX) via assimp-py, returning a ready-to-save Mesh --
	see _import_via_assimp()."""
	return _import_via_assimp(path)
