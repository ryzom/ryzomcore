"""Imports a plain interchange format into a `pynel.ryzom_shape.Mesh`, so it
can be saved as a real `.shape` via `save_shape()`.

Only `CMesh` can be built this way (see `ryzom_shape.py`'s `dumps()`
docstring): unlike `CMeshMRM`, its geometry is written field-by-field rather
than copied byte-for-byte from an already-parsed file, so it's the only
shape type pynel can construct from scratch. A `CMesh` has no LOD levels --
turning an imported mesh into a real progressive-LOD `CMeshMRM` needs
`CMRMBuilder` (`nel/include/nel/3d/mrm_builder.h`), a C++-only class with no
Python binding; out of scope here (see logs/forgery-object-editor.md for the investigation).

`.obj`/`.mtl` are hand-parsed here for the same reason `shape_export.py`
hand-writes them: simple, dependency-free text formats. `.dae` and `.fbx` go
through `assimp-py` instead (see `_import_via_assimp()`) -- both are complex
enough formats that hand-parsing isn't worth it once a real importer library
is already a dependency.
"""

import bisect
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from pynel.ryzom_shape import (
	AABBox, Bone, Material, Matrix, MatrixBlock, Mesh, MeshBase, MeshGeom, Quaternion, RdrPass, Rgba, SkeletonLod,
	SkeletonShape, Texture, TexEnv, Vector3, VertexBuffer,
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

# _build_material()'s fallback values, used only when the source .obj/.dae/
# .fbx/.gltf doesn't specify a given property itself (an untextured/plain
# `usemtl` face, or a source file with no material data at all). Until
# 2026-09-06 these were used UNCONDITIONALLY, deliberately ignoring the
# source file's own diffuse/ambient/specular/emissive/opacity/shininess --
# matching the real Ryzom content pipeline instead (the 3ds Max NeL exporter,
# nel/tools/3d/plugin_max/nel_mesh_lib/export_material.cpp, reads every one
# of these off the NeL-material-plugin instance the artist assigns in 3ds Max,
# unrelated to the imported source file's own data -- confirmed by comparing
# a real Cinema4D->.fbx->3dsMax->.shape export against the same .fbx imported
# directly through here). That call was reversed 2026-09-06 (Nuno): it broke
# Forgery's own export->reimport round-trip validation (mesh_skel_anim_io.md)
# by always discarding the very material colors the export step had just
# written, and there's no way to tell "genuine 3ds Max import" from "Forgery's
# own round-trip" apart to keep the old behavior for only one of them.
_NEL_DEFAULT_GRAY = Rgba(150, 150, 150, 255)  # 3ds Max's own "Standard material" default diffuse/ambient swatch
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
	emissive: Tuple[float, float, float] = (0.0, 0.0, 0.0)
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
		elif keyword == "Ke":
			current.emissive = (float(parts[1]), float(parts[2]), float(parts[3]))
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


def _texture_base_name(texture_name: str, base_dir: Optional[Path] = None) -> str:
	"""Resolves a texture reference against `base_dir` (the imported mesh
	file's own folder) if it isn't already absolute -- an imported
	.fbx/.dae/.obj routinely carries either kind, and on a Windows-authored
	file an absolute one uses backslashes, which pathlib.Path().name leaves
	untouched on POSIX (it only splits on `/`), so both separators are
	normalized first either way.

	If that resolves to a real file *right now*, the full resolved path is
	kept as-is (see shape_geometry.resolve_texture_ref()'s absolute-path
	handling) -- lets the shape render immediately without the user having
	to separately hunt down/re-link the texture. Explicitly copying it into
	the active workspace later (see object_editor.py's texture-copy button)
	is what turns this into a normal bare name, portable across machines.
	Falls back to just the bare file name (today's behavior) when there's
	nothing on disk to preserve."""
	normalized = texture_name.replace("\\", "/")
	candidate = Path(normalized)
	if not candidate.is_absolute() and base_dir is not None:
		candidate = base_dir / candidate
	if candidate.is_file():
		return str(candidate.resolve())
	return Path(normalized).name


def _rgba_from_floats(rgb: Optional[Tuple[float, float, float]], alpha: float = 1.0) -> Optional[Rgba]:
	"""Inverse of shape_geometry.rgba_to_color(): a 0-1 float (r, g, b) triple
	(+ separate 0-1 alpha) back into a NeL Rgba (0-255 per channel), or None
	if `rgb` itself is None (caller falls back to its own default then)."""
	if rgb is None:
		return None
	r, g, b = rgb
	return Rgba(round(r * 255), round(g * 255), round(b * 255), round(alpha * 255))


def _build_material(texture_name: Optional[str] = None, double_sided: bool = False,
                     base_dir: Optional[Path] = None,
                     diffuse: Optional[Tuple[float, float, float]] = None,
                     ambient: Optional[Tuple[float, float, float]] = None,
                     specular: Optional[Tuple[float, float, float]] = None,
                     emissive: Optional[Tuple[float, float, float]] = None,
                     shininess: Optional[float] = None,
                     opacity: Optional[float] = None) -> Material:
	"""Builds a NeL material from an imported .obj/.dae/.fbx/.gltf source's
	own material data -- `diffuse`/`ambient`/`specular`/`emissive` (0-1 float
	RGB triples, matching shape_export.py's own COLOR_DIFFUSE/COLOR_AMBIENT/
	COLOR_SPECULAR/COLOR_EMISSIVE), `shininess` and `opacity` (0-1), each
	falling back to _NEL_DEFAULT_GRAY et al above when the source file didn't
	specify that property itself (an untextured plain `usemtl`, or no
	material data at all). `double_sided` is a real geometric necessity
	(thin panels/foliage with no back faces), always honored regardless."""
	textures: List[Optional[Texture]] = []
	tex_envs: List[Optional[TexEnv]] = []
	if texture_name:
		resolved = _texture_base_name(texture_name, base_dir)
		# A resolved absolute path must keep its real on-disk casing
		# (significant on Linux/macOS); only a bare name is case-insensitive
		# in Ryzom's own texture lookup.
		file_name = resolved if Path(resolved).is_absolute() else resolved.lower()
		textures = [Texture(class_name="CTextureFile", file_name=file_name, allow_degradation=True)]
		tex_envs = [_MODULATE_TEX_ENV]

	flags = _DEFAULT_MATERIAL_FLAGS | (_MAT_FLAG_DOUBLE_SIDED if double_sided else 0)
	diffuse_rgba = _rgba_from_floats(diffuse, opacity if opacity is not None else 1.0) or _NEL_DEFAULT_GRAY
	return Material(
		shader_type=_SHADER_NORMAL,
		flags=flags,
		src_blend=0,
		dst_blend=0,
		z_function=_ZFUNC_LESSEQUAL,
		z_bias=0.0,
		color=Rgba(255, 255, 255, 255),
		emissive=_rgba_from_floats(emissive) or Rgba(0, 0, 0, 255),
		ambient=_rgba_from_floats(ambient) or _NEL_DEFAULT_GRAY,
		diffuse=diffuse_rgba,
		specular=_rgba_from_floats(specular) or _NEL_DEFAULT_SPECULAR,
		shininess=shininess if shininess is not None else _NEL_DEFAULT_SHININESS,
		alpha_test_threshold=0.5,
		tex_coord_gen_mode=0,
		textures=textures,
		tex_envs=tex_envs,
	)


def _assemble_mesh(
		positions: List[Tuple[float, float, float]], normals: List[Tuple[float, float, float]],
		texcoords: List[Tuple[float, float]], materials: List[Material], rdr_passes: Optional[List[RdrPass]] = None,
		matrix_blocks: Optional[List[MatrixBlock]] = None, bones_name: Optional[List[str]] = None,
		skin_weights: Optional[Tuple[
			List[Tuple[float, float, float, float]], List[Tuple[int, int, int, int]]]] = None,
		flip_v: bool = True) -> Mesh:
	"""Shared final assembly step for every importer, from already-built
	vertex channels: an unskinned, single-matrix-block CMesh by default
	(`rdr_passes`), or -- when `matrix_blocks`/`bones_name`/`skin_weights` are
	given instead (see _build_skinned_matrix_blocks()) -- a skinned CMesh
	using those pre-built, possibly multiple, matrix blocks. `flip_v`: see
	the TexCoord0 block below -- False for glTF/glb specifically."""
	channels = {"Position": positions}
	types = [0] * 16
	types[0] = 7  # Position: float3
	if normals:
		channels["Normal"] = normals
		types[1] = 7  # Normal: float3
	if skin_weights is not None:
		weight_channel, palette_channel = skin_weights
		channels["Weight"] = weight_channel
		types[12] = 10  # Weight: float4 (CVertexBuffer::Weight/Float4, mesh.cpp)
		channels["PaletteSkin"] = palette_channel
		types[13] = 12  # PaletteSkin: uint8x4 (CVertexBuffer::PaletteSkin/UChar4, mesh.cpp)
	if texcoords:
		# .obj/.dae/.fbx (hand-parsed or via assimp-py) all use the format's
		# own native V-origin convention (0 at the bottom, matching OpenGL --
		# verified against assimp's own FBX/OBJ reader source, neither flips
		# it), the opposite of what real NeL .shape files store (V=0 at the
		# top -- see object_editor.py's _build_vertex_data()). Converting
		# here, once, at import time, rather than working around it at
		# display time, is what actually makes the *saved* .shape file
		# correct on its own -- a viewer-side-only flip would round-trip
		# wrong (looks right in Forgery only for as long as it remembers
		# this mesh came from an import; wrong once reloaded as a plain
		# .shape, and wrong in the real engine too, which has no such
		# per-file memory).
		#
		# glTF is the one exception (`flip_v=False`, see _import_via_assimp()):
		# its spec mandates V=0 at the TOP (matching NeL's own convention
		# already, unlike every other format here) -- assimp-py's own glTF
		# reader returns it as-is, unflipped, verified 2026-09-05 by
		# comparing a .dae/.fbx/.glb export of the same shape reimported
		# back: dae/fbx agreed on V, glb's V was their exact `1-v` mirror
		# until this exception was added (Nuno: "les textures sont de
		# nouveau en miroir horizontal" reimporting a Forgery-exported .glb).
		channels["TexCoord0"] = [(u, 1.0 - v if flip_v else v) for u, v in texcoords]
		types[2] = 4  # TexCoord0: float2

	vertex_buffer = VertexBuffer(
		name="", num_verts=len(positions), vertex_color_format=0, channels=channels, types=types)
	if matrix_blocks is None:
		matrix_blocks = [MatrixBlock(matrix_id=[0] * 16, num_matrix=0, rdr_passes=rdr_passes)]

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
		bones_name=bones_name or [], mesh_morpher=None, vertex_buffer=vertex_buffer,
		matrix_blocks=matrix_blocks, bbox=bbox, skinned=skin_weights is not None,
	)
	return Mesh(base=base, geom=geom)


def build_mesh(obj_mesh: ObjMesh, mtl_materials: Dict[str, MtlMaterial], base_dir: Optional[Path] = None) -> Mesh:
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
			return _build_material(base_dir=base_dir)
		return _build_material(
			texture_name=mtl.diffuse_texture, base_dir=base_dir,
			diffuse=mtl.diffuse, ambient=mtl.ambient, specular=mtl.specular, emissive=mtl.emissive,
			shininess=mtl.shininess, opacity=mtl.opacity,
		)

	materials = [material_for(name) for name in material_order]
	rdr_passes = [RdrPass(material_id=material_ids[name], indices=pass_indices[material_ids[name]])
	              for name in material_order]
	positions = [_yup_to_zup(p) for p in positions]
	if normals:
		normals = [_yup_to_zup(n) for n in normals]
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

	return build_mesh(obj_mesh, mtl_materials, base_dir=path.parent)


# ---------------------------------------------------------------------------
# .dae / .fbx (via assimp-py)
# ---------------------------------------------------------------------------
#
# Both formats go through the same Assimp-based path: Assimp auto-detects the
# format from content/extension, and exposes the same Scene/Mesh/Material API
# regardless of source, so there's nothing format-specific left to write once
# node transforms are handled generically (see _iter_mesh_instances()).
#
# Both Assimp's FBX and Collada loaders always normalize whatever up axis the
# source file declares into Assimp's own canonical Y-up (Collada:
# ColladaLoader.cpp's ConvertScene(), rotates unless already Y_UP -- FBX:
# FBXConverter.cpp's correctRootTransform(), reads the file's UpAxis/
# FrontAxis/CoordAxis metadata and bakes the equivalent conversion into the
# root node's transform), so _iter_mesh_instances() below is seeded with
# _YUP_TO_ZUP_MATRIX instead of a plain identity, converting that canonical
# Y-up into Ryzom's own Z-up on the way out (confirmed necessary: a shape
# authored in Cinema4D, exported to .fbx, then imported to 3ds Max and saved
# as `.shape` from there came out with a different bbox orientation than the
# same .fbx imported directly through here, since 3ds Max's own FBX importer
# targets its native Z-up while Assimp always targets Y-up).
#
# .dae used to be hand-parsed via pycollada instead (kept no longer, see git
# history) -- swapped once assimp-py was already a dependency for .fbx. The
# apparent round-trip match against Forgery's own .dae exports found while
# validating that swap (see logs/forgery-object-editor.md) was this same Y-up normalization
# being a no-op: those exports used to declare a `Y_UP` <asset><up_axis> tag
# (pycollada's own default) while actually holding Z-up data, so
# re-importing them without _YUP_TO_ZUP_MATRIX happened to match by pure
# coincidence -- it would have silently mis-rotated any *correctly*
# Z_UP-tagged .dae. Fixed in shape_export.py's _export_dae() (now declares
# the true Z_UP), so this module's own unconditional _YUP_TO_ZUP_MATRIX
# handles Forgery's own re-exported .dae correctly too, same as any other
# properly-tagged one.

# Converts Assimp's canonical Y-up (X=right, Y=up, Z=forward) into Ryzom's
# Z-up (X=right, Z=up, Y=forward): newX=x, newY=-z, newZ=y -- the exact
# inverse of ColladaLoader.cpp's own Z_UP-to-Y_UP conversion matrix (see the
# comment above), a proper rotation (determinant +1, no mirroring, so vertex
# winding/normals need no correction beyond this). Row-major, matching
# Assimp's own aiMatrix4x4 convention (assimp-py's Node.transformation
# mirrors it verbatim) -- this is used as _iter_mesh_instances()'s starting
# "parent" transform, so it composes through the whole node walk for free.
_YUP_TO_ZUP_MATRIX = ((1.0, 0.0, 0.0, 0.0), (0.0, 0.0, -1.0, 0.0), (0.0, 1.0, 0.0, 0.0), (0.0, 0.0, 0.0, 1.0))


def _yup_to_zup(v):
	"""Same conversion as _YUP_TO_ZUP_MATRIX above, applied directly to a
	plain (x, y, z) vector -- for import_obj()/build_mesh(), which has no
	node hierarchy to seed a matrix walk with (unlike _iter_mesh_instances(),
	used for .dae/.fbx/.gltf): .obj has no up-axis metadata at all, same
	de facto Y-up convention as every format here (see shape_export.py's own
	_zup_to_yup(), this function's exact inverse) -- never converted on
	import before (found 2026-09-05, Nuno: "obj -> shape : pas bon, Y-Up",
	once _export_obj() started correctly writing real Y-up .obj files)."""
	x, y, z = v
	return (x, -z, y)


def _mat_mul_mat(a, b):
	return tuple(tuple(sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)) for i in range(4))


def _invert_matrix(m):
	"""General 4x4 inverse via Gauss-Jordan elimination with partial pivoting
	-- unlike _mat_mul_dir()'s own rotation/uniform-scale-only shortcut, a
	bone's assimp offset_matrix can carry arbitrary scale (confirmed real,
	see extract_skeleton()'s own docstring on the Spider_Armature case), so
	nothing less than a real inverse is correct here. Raises ShapeImportError
	for a singular matrix (shouldn't happen for a real bone offset matrix,
	which is always invertible by construction)."""
	a = [list(row) for row in m]
	inv = [[1.0 if i == j else 0.0 for j in range(4)] for i in range(4)]
	for col in range(4):
		pivot_row = max(range(col, 4), key=lambda r: abs(a[r][col]))
		if abs(a[pivot_row][col]) < 1e-12:
			raise ShapeImportError("cannot invert a singular bone offset matrix")
		a[col], a[pivot_row] = a[pivot_row], a[col]
		inv[col], inv[pivot_row] = inv[pivot_row], inv[col]
		pivot = a[col][col]
		a[col] = [v / pivot for v in a[col]]
		inv[col] = [v / pivot for v in inv[col]]
		for row in range(4):
			if row != col:
				factor = a[row][col]
				a[row] = [av - factor * cv for av, cv in zip(a[row], a[col])]
				inv[row] = [iv - factor * cv for iv, cv in zip(inv[row], inv[col])]
	return tuple(tuple(row) for row in inv)


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
	if not mesh.normals:
		return None
	data = list(mesh.normals)
	return [tuple(data[i:i + 3]) for i in range(0, len(data), 3)] if data else None


def _mesh_texcoords(mesh):
	if not mesh.texcoords or not len(mesh.texcoords[0]):
		return None
	data = list(mesh.texcoords[0])
	stride = mesh.num_uv_components[0]
	return [tuple(data[i:i + 2]) for i in range(0, len(data), stride)]


def _mesh_bones(mesh) -> Optional[List[List[Tuple[str, float]]]]:
	"""Regroups assimp's per-bone vertex weight lists (`mesh.bones`, one
	entry per bone with its own `vertex_ids`/`weights` memoryviews -- see
	assimp_py's Bone type) into a per-vertex list of (bone_name, weight)
	pairs, indexed like `mesh.vertices`/_mesh_positions() -- the layout
	`CSkinWeight` needs (per vertex), the opposite of assimp's own
	per-bone one (mirroring Assimp's own aiBone/aiVertexWeight).

	No extra remapping needed for JoinIdenticalVertices: Assimp's own
	JoinVerticesProcess (already applied, see _import_via_assimp()'s
	flags) remaps every bone's vertex_ids to the final deduplicated
	vertex indices itself (see JoinVerticesProcess.cpp's "adjust bone
	vertex weights" step) -- mesh.bones is already aligned with
	mesh.vertices as returned here, same as mesh.indices already is.

	Returns None if the mesh has no bones at all (same convention as
	_mesh_normals()/_mesh_texcoords() for an absent channel)."""
	if not mesh.bones:
		return None
	num_vertices = len(mesh.vertices) // 3
	per_vertex: List[List[Tuple[str, float]]] = [[] for _ in range(num_vertices)]
	for bone in mesh.bones:
		if not bone.vertex_ids or not bone.weights:
			# A bone listed in the skeleton/deformer with zero actual vertex
			# influence -- assimp_py.Bone.vertex_ids/weights come back None
			# for one instead of empty memoryviews (found 2026-09-05
			# reimporting a shape_export.py-produced .fbx, see
			# mesh_skel_anim_io.md). Nothing to add for it.
			continue
		for vertex_id, weight in zip(bone.vertex_ids, bone.weights):
			per_vertex[vertex_id].append((bone.name, weight))
	return per_vertex


# CMesh::CSkinWeight only ever keeps this many (MatrixId, Weight) slots per
# vertex (NL3D_MESH_SKINNING_MAX_MATRIX, mesh.h) -- extra assimp bone
# influences beyond the 4 heaviest are dropped, matching every real NeL
# content exporter (3ds Max plugin included).
_MAX_SKIN_MATRICES = 4

# IDriver::MaxModelMatrix (driver.h): a CMatrixBlock's own `matrix_id` is a
# fixed uint32[16], and CVertexBuffer::PaletteSkin values are indices *into
# that array* (block-local), not global bone ids -- see
# _build_skinned_matrix_blocks().
_MAX_MATRICES_PER_BLOCK = 16


def _normalize_skin_weights(
		per_vertex_bones: List[List[Tuple[str, float]]],
) -> Tuple[List[str], List[Tuple[float, float, float, float]], List[Tuple[int, int, int, int]]]:
	"""Reduces each vertex's (bone_name, weight) list (from _mesh_bones(),
	already merged across every mesh instance) to the top `_MAX_SKIN_MATRICES`
	weights, normalized to sum to 1, and turns bone names into indices into a
	shared `bones_name` list (first-seen order) -- CSkinWeight's own MatrixId
	indexes that array (mesh.h: "Each matrix id used in SkinWeights must have
	a corresponding string in the bone name array"). An unused slot (fewer
	than 4 influences) is padded with the vertex's own heaviest bone id at
	weight 0, mirroring CMeshGeom::buildSkin's own step 0 normalization
	(mesh.cpp) -- needed so a padding slot never pulls in an extra, otherwise
	unused bone once _build_skinned_matrix_blocks() groups faces by bone use.
	A vertex with no bone influence at all (an unweighted vertex in the
	source file, or a whole unskinned mesh instanced alongside skinned ones)
	is rigidly bound to bone 0 at full weight instead, since CSkinWeight's
	own weights-must-sum-to-1 contract allows no true all-zero entry."""
	bones_name: List[str] = []
	bone_index: Dict[str, int] = {}
	for vertex_bones in per_vertex_bones:
		for name, _ in vertex_bones:
			if name not in bone_index:
				bone_index[name] = len(bones_name)
				bones_name.append(name)

	weights: List[Tuple[float, float, float, float]] = []
	matrix_ids: List[Tuple[int, int, int, int]] = []
	for vertex_bones in per_vertex_bones:
		top = sorted(vertex_bones, key=lambda name_weight: name_weight[1], reverse=True)[:_MAX_SKIN_MATRICES]
		total = sum(weight for _, weight in top)
		if total <= 0.0:
			top = [(bones_name[0], 1.0)]
			total = 1.0
		pad = _MAX_SKIN_MATRICES - len(top)
		matrix_ids.append(tuple(bone_index[name] for name, _ in top) + (bone_index[top[0][0]],) * pad)
		weights.append(tuple(weight / total for _, weight in top) + (0.0,) * pad)
	return bones_name, weights, matrix_ids


def _build_skinned_matrix_blocks(
		pass_indices: Dict[int, List[int]], vertex_matrix_ids: List[Tuple[int, int, int, int]],
		vertex_weights: List[Tuple[float, float, float, float]],
) -> Tuple[List[MatrixBlock], List[Tuple[float, float, float, float]], List[Tuple[int, int, int, int]], List[int]]:
	"""Greedily packs every triangle from `pass_indices` (material_id ->
	flat corner-index list, indices into the shared vertex channels) into
	`MatrixBlock`s of at most `_MAX_MATRICES_PER_BLOCK` distinct bones each --
	mirrors CMeshGeom::buildSkin (mesh.cpp), minus its step 4 bone-reordering
	pass (a pure render-time matrix-change-minimization optimization, not
	needed for a correct file, so skipped here for simplicity). A single
	triangle can use at most 3*_MAX_SKIN_MATRICES == 12 distinct bones,
	always <= _MAX_MATRICES_PER_BLOCK, so it always fits in a fresh block.

	A vertex referenced by triangles landing in more than one matrix block is
	duplicated once per extra block, since its PaletteSkin value is
	block-local (an index into that block's own `matrix_id` array, not a
	global bone id -- see mesh.h's CVertexBuffer::PaletteSkin doc) and a
	vertex can only carry one PaletteSkin value.

	Returns (matrix_blocks, weight_channel, palette_channel,
	extra_vertex_sources): the first three are already the final,
	block-local per-vertex VertexBuffer channel values, ordered
	[one entry per original vertex][one entry per duplicate, in
	`extra_vertex_sources` order -- extra_vertex_sources[i] names the
	original vertex index duplicate `i` was copied from, for the caller to
	also duplicate Position/Normal/TexCoord0 the same way]."""
	triangles: List[Tuple[int, int, int, int]] = []  # (material_id, v0, v1, v2)
	for material_id, indices in pass_indices.items():
		for i in range(0, len(indices), 3):
			triangles.append((material_id, indices[i], indices[i + 1], indices[i + 2]))

	num_original = len(vertex_matrix_ids)
	weight_channel: List[Tuple[float, float, float, float]] = list(vertex_weights)
	palette_channel: List[Tuple[int, int, int, int]] = [(0, 0, 0, 0)] * num_original
	extra_vertex_sources: List[int] = []
	used_in_any_block: set = set()
	vertex_dup_for_block: Dict[Tuple[int, int], int] = {}

	def vertex_for_block(v: int, block_index: int, local_ids: Dict[int, int]) -> int:
		key = (v, block_index)
		real_v = vertex_dup_for_block.get(key)
		if real_v is not None:
			return real_v
		if v not in used_in_any_block:
			real_v = v
			used_in_any_block.add(v)
		else:
			real_v = num_original + len(extra_vertex_sources)
			extra_vertex_sources.append(v)
			weight_channel.append(vertex_weights[v])
			palette_channel.append((0, 0, 0, 0))
		vertex_dup_for_block[key] = real_v
		palette_channel[real_v] = tuple(local_ids[bone_id] for bone_id in vertex_matrix_ids[v])
		return real_v

	blocks: List[MatrixBlock] = []
	current_bones: List[int] = []
	current_bone_pos: Dict[int, int] = {}
	current_block_passes: Dict[int, List[int]] = {}

	def flush_block():
		nonlocal current_bones, current_bone_pos, current_block_passes
		if current_bones:
			rdr_passes = [RdrPass(material_id=mid, indices=idx) for mid, idx in current_block_passes.items()]
			blocks.append(MatrixBlock(matrix_id=current_bones[:], num_matrix=len(current_bones), rdr_passes=rdr_passes))
		current_bones, current_bone_pos, current_block_passes = [], {}, {}

	for material_id, v0, v1, v2 in triangles:
		face_bones = sorted({bone_id for v in (v0, v1, v2) for bone_id in vertex_matrix_ids[v]})
		new_bones = [bone_id for bone_id in face_bones if bone_id not in current_bone_pos]
		if len(current_bones) + len(new_bones) > _MAX_MATRICES_PER_BLOCK:
			flush_block()
			new_bones = face_bones
		for bone_id in new_bones:
			current_bone_pos[bone_id] = len(current_bones)
			current_bones.append(bone_id)
		block_index = len(blocks)  # the block currently being filled
		corners = [vertex_for_block(v, block_index, current_bone_pos) for v in (v0, v1, v2)]
		current_block_passes.setdefault(material_id, []).extend(corners)

	flush_block()
	return blocks, weight_channel, palette_channel, extra_vertex_sources


def _build_material_from_assimp_material(material: dict, base_dir: Optional[Path] = None) -> Material:
	"""`material` is one of assimp_py.Scene.materials' plain dicts (property
	name -> value, see assimp-py's own docs). COLOR_DIFFUSE/COLOR_AMBIENT/
	COLOR_SPECULAR/COLOR_EMISSIVE/SHININESS/OPACITY, when present, are passed
	through to _build_material() (see its own docstring) -- absent for a
	.gltf material (COLOR_AMBIENT/COLOR_SPECULAR have no PBR equivalent, see
	_assimp_material_dict()'s own comment) falls back to _build_material()'s
	defaults for just that property, same as an untextured `usemtl` in .obj.
	TWOSIDED (Assimp's own name for AI_MATKEY_TWOSIDED) is only present in
	the dict at all when the source file set it explicitly -- absent means
	"use the format's own default", which for every format Forgery imports
	is single-sided."""
	import assimp_py

	textures = material.get("TEXTURES", {})
	diffuse_paths = textures.get(assimp_py.TextureType_DIFFUSE)
	diffuse = material.get("COLOR_DIFFUSE")
	return _build_material(
		texture_name=diffuse_paths[0] if diffuse_paths else None,
		double_sided=bool(material.get("TWOSIDED", False)),
		base_dir=base_dir,
		diffuse=tuple(diffuse) if diffuse is not None else None,
		ambient=tuple(material["COLOR_AMBIENT"]) if "COLOR_AMBIENT" in material else None,
		specular=tuple(material["COLOR_SPECULAR"]) if "COLOR_SPECULAR" in material else None,
		emissive=tuple(material["COLOR_EMISSIVE"]) if "COLOR_EMISSIVE" in material else None,
		shininess=material.get("SHININESS"),
		opacity=material.get("OPACITY"),
	)


def _assimp_import_file(assimp_py, path: Path, flags: int):
	"""assimp_py.import_file(), with every native exception it can raise
	(RuntimeError for an assimp-side load failure, ValueError for a
	structural issue like a face assimp's own Process_Triangulate couldn't
	fully triangulate, FileNotFoundError, ...) turned into ShapeImportError --
	the one exception type every caller (_process() in import_watcher.py,
	both directly and via its reconcile()/handle_settled() paths) already
	knows how to report as a normal, non-crashing auto-export failure.
	Found 2026-09-05 on real content: an unhandled RuntimeError/ValueError
	from this call used to either crash a background thread outright
	(ImportWatcher.reconcile()) or surface with a poor, hard-to-read message
	(the bare native exception's own str())."""
	try:
		return assimp_py.import_file(str(path), flags)
	except Exception as exc:  # noqa: BLE001 -- see docstring: any native exception here means "assimp couldn't import this file", same as ShapeImportError
		raise ShapeImportError(f"assimp failed to import {path}: {exc}") from exc


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
	scene = _assimp_import_file(assimp_py, path, flags)

	if scene.num_meshes == 0:
		raise ShapeImportError(f"no meshes found in {path}")

	# A mesh named with "__skip__" (case-insensitive) is a technical/helper
	# mesh the artist never wants merged into the shape (collision proxy,
	# reference geometry...) -- excluded from every channel-presence check
	# and from the main fusion loop below, same convention as the armature
	# and animation-clip exclusion (see extract_skeleton()).
	included_mesh_indices = {
		i for i in range(scene.num_meshes) if "__skip__" not in scene.meshes[i].name.lower()}

	# assimp-py returns None (not an empty list/memoryview) for a channel a
	# mesh has none of at all -- e.g. .texcoords itself for a UV-less mesh --
	# so every len() here needs an `x and` guard first.
	has_normals = any(scene.meshes[i].normals and len(scene.meshes[i].normals) for i in included_mesh_indices)
	has_uvs = any(
		scene.meshes[i].texcoords and scene.meshes[i].texcoords[0] and len(scene.meshes[i].texcoords[0])
		for i in included_mesh_indices)
	has_bones = any(scene.meshes[i].bones and len(scene.meshes[i].bones) for i in included_mesh_indices)

	positions: List[Tuple[float, float, float]] = []
	normals: List[Tuple[float, float, float]] = []
	texcoords: List[Tuple[float, float]] = []
	bone_weights: List[List[Tuple[str, float]]] = []
	material_order: List[int] = []  # first-seen order of assimp material indices, becomes RdrPass material_id order
	pass_indices: Dict[int, List[int]] = {}

	for mesh_index, transform in _iter_mesh_instances(scene.root_node, _YUP_TO_ZUP_MATRIX):
		if mesh_index not in included_mesh_indices:
			continue
		mesh = scene.meshes[mesh_index]
		mesh_positions = _mesh_positions(mesh)
		mesh_normals = _mesh_normals(mesh)
		mesh_texcoords = _mesh_texcoords(mesh)
		mesh_bones = _mesh_bones(mesh) if has_bones else None
		base_index = len(positions)

		for i, position in enumerate(mesh_positions):
			positions.append(_mat_mul_point(transform, position))
			if has_normals:
				normal = mesh_normals[i] if mesh_normals else (0.0, 0.0, 1.0)
				normals.append(_normalize(_mat_mul_dir(transform, normal)))
			if has_uvs:
				texcoords.append(mesh_texcoords[i] if mesh_texcoords else (0.0, 0.0))
			if has_bones:
				# A mesh instanced alongside skinned ones but with no bones of
				# its own (mesh_bones is None) contributes unweighted
				# vertices, not a hard error -- see
				# _normalize_skin_weights()'s own no-influence fallback.
				bone_weights.append(mesh_bones[i] if mesh_bones else [])

		if mesh.material_index not in pass_indices:
			material_order.append(mesh.material_index)
			pass_indices[mesh.material_index] = []
		pass_indices[mesh.material_index].extend(base_index + index for index in mesh.indices)

	if not positions:
		raise ShapeImportError(f"no vertices found in {path}")

	materials = [_build_material_from_assimp_material(scene.materials[mid], path.parent) for mid in material_order]
	flip_v = path.suffix.lower() not in (".gltf", ".glb")

	if has_bones:
		bones_name, weights, matrix_ids = _normalize_skin_weights(bone_weights)
		final_pass_indices = {i: pass_indices[mid] for i, mid in enumerate(material_order)}
		matrix_blocks, weight_channel, palette_channel, extra_vertex_sources = _build_skinned_matrix_blocks(
			final_pass_indices, matrix_ids, weights)
		for v in extra_vertex_sources:
			positions.append(positions[v])
			if has_normals:
				normals.append(normals[v])
			if has_uvs:
				texcoords.append(texcoords[v])
		return _assemble_mesh(
			positions, normals, texcoords, materials, matrix_blocks=matrix_blocks,
			bones_name=bones_name, skin_weights=(weight_channel, palette_channel), flip_v=flip_v)

	rdr_passes = [RdrPass(material_id=i, indices=pass_indices[mid]) for i, mid in enumerate(material_order)]
	return _assemble_mesh(positions, normals, texcoords, materials, rdr_passes, flip_v=flip_v)


# ---------------------------------------------------------------------------
# Skeleton extraction (.dae / .fbx / .gltf, via assimp-py)
# ---------------------------------------------------------------------------
#
# A skinned mesh import (above) never generates a .skel of its own -- it must
# be paired with one that already exists (see this module's own docstring and
# skinned_mesh_import.md). extract_skeleton() below is the one thing that
# *does* build a brand new CSkeletonShape from a source file's own bone
# hierarchy, for import_watcher.py to write alongside the .shape it already
# auto-exports, when there's a bind pose worth starting from (see
# project-todos/forgery/skel_export.md for the design/decisions behind this).

# NLMISC::CMatrix's own StateBit flags (matrix.cpp: MAT_TRANS=1, MAT_ROT=2,
# MAT_SCALEUNI=4, MAT_SCALEANY=8, MAT_PROJ=16) -- MAT_TRANS|MAT_ROT|MAT_SCALEANY
# is exactly what CMatrix::setRot(m33[9]) itself sets (matrix.cpp) for a
# general (possibly non-uniform-scaled) 3x3 with no special-cased uniform
# scale, the safest lossless choice for an arbitrary bind-pose matrix -- and
# matches real production `.skel` files' own bone inv_bind_pos encoding
# (confirmed against ryzom-data/assets_src/mounts/tr_mo_capryni_mount.skel).
_MAT_STATE_TRANS_ROT_SCALEANY = 1 | 2 | 8


def _pynel_matrix_from_4x4(m) -> Matrix:
	"""Converts a plain row-major 4x4 (assimp's own convention, see this
	module's docstring) into pynel's own sparse `Matrix` encoding, losslessly
	(any affine 4x4 -- rotation, non-uniform scale, shear -- fits in the
	general-rotation state, see _MAT_STATE_TRANS_ROT_SCALEANY above)."""
	rot = (m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2])
	trans = (m[0][3], m[1][3], m[2][3])
	return Matrix(state_bit=_MAT_STATE_TRANS_ROT_SCALEANY, scale=1.0, rot=rot, trans=trans, proj=None)


def _matrix_to_quat(r) -> Quaternion:
	"""Standard trace-based rotation-matrix-to-quaternion conversion (Shepperd's
	method) -- `r` is a plain 3x3 (list of 3 rows of 3 floats), already
	normalized (no scale baked in, see _decompose_matrix())."""
	trace = r[0][0] + r[1][1] + r[2][2]
	if trace > 0:
		s = 0.5 / (trace + 1.0) ** 0.5
		return Quaternion((r[2][1] - r[1][2]) * s, (r[0][2] - r[2][0]) * s, (r[1][0] - r[0][1]) * s, 0.25 / s)
	if r[0][0] > r[1][1] and r[0][0] > r[2][2]:
		s = 2.0 * (1.0 + r[0][0] - r[1][1] - r[2][2]) ** 0.5
		return Quaternion(0.25 * s, (r[0][1] + r[1][0]) / s, (r[0][2] + r[2][0]) / s, (r[2][1] - r[1][2]) / s)
	if r[1][1] > r[2][2]:
		s = 2.0 * (1.0 + r[1][1] - r[0][0] - r[2][2]) ** 0.5
		return Quaternion((r[0][1] + r[1][0]) / s, 0.25 * s, (r[1][2] + r[2][1]) / s, (r[0][2] - r[2][0]) / s)
	s = 2.0 * (1.0 + r[2][2] - r[0][0] - r[1][1]) ** 0.5
	return Quaternion((r[0][2] + r[2][0]) / s, (r[1][2] + r[2][1]) / s, 0.25 * s, (r[1][0] - r[0][1]) / s)


def _decompose_matrix(m) -> Tuple[Vector3, Quaternion, Vector3]:
	"""Decomposes a row-major 4x4 (assimp's own convention) into a
	translation/rotation/scale triple, assuming no shear -- true for every
	node this is used on (extract_skeleton()'s own bone rest-pose transforms),
	an ordinary T*R*S authored in a 3D content tool."""
	translation = Vector3(m[0][3], m[1][3], m[2][3])
	columns = [(m[0][c], m[1][c], m[2][c]) for c in range(3)]
	scales = [(sum(v * v for v in col)) ** 0.5 for col in columns]
	rot = [[(columns[c][row] / scales[c] if scales[c] > 1e-12 else 0.0) for c in range(3)] for row in range(3)]
	return translation, _matrix_to_quat(rot), Vector3(*scales)


def _find_armature_root(node, bone_names, parent=None):
	"""Depth-first, pre-order search for the armature root -- the PARENT of
	the first real bone node encountered (bone identity comes from
	`bone_names`, itself derived from `mesh.bones`, so this is accurate
	regardless of what any node in the hierarchy happens to be named).

	Replaces an earlier version that instead required the wrapper node's own
	name to *contain the source file's name* -- worked for an authored file
	whose armature happens to be named after the character (e.g.
	"Spider_Armature" in `spider.dae`), but broke silently reimporting
	Forgery's own exports: `shape_export.py`'s own exporter always names
	that wrapper node literally "Armature", which never contains the file's
	name (found 2026-09-05, Nuno: "l'armature peut avoir n'importe quel nom,
	nan?" -- correct, there is no naming convention to rely on at all, from
	any tool). No transform tracking here: the node hierarchy is only ever
	used to find bone names/parentage, never for transform math -- see
	extract_skeleton()'s own docstring on why."""
	if node.name in bone_names:
		return parent
	for child in node.children:
		found = _find_armature_root(child, bone_names, node)
		if found is not None:
			return found
	return None


def _collect_bone_hierarchy(node, bone_names, parent_bone_id: int, names, father_ids, bone_index) -> None:
	"""Recursively collects `names`/`father_ids` (parallel lists, depth-first
	order, so a bone's `father_id` always refers to an earlier index) from
	`node`'s subtree -- topology only (which node is a bone, and its nearest
	bone ancestor), no transform math at all: a bone's actual rest-pose
	transform is derived from its own assimp offset_matrix instead (see
	extract_skeleton()), not from this node's own local transform, so there's
	nothing to accumulate here. A node that is NOT itself a real bone (see
	skel_export.md's "nœuds non-os ignorés") is still traversed, but doesn't
	become a Bone -- its own children still get the nearest real bone
	ancestor as their father."""
	if node.name in bone_names and node.name not in bone_index:
		bone_index[node.name] = len(names)
		names.append(node.name)
		father_ids.append(parent_bone_id)
		next_parent_bone_id = bone_index[node.name]
	else:
		next_parent_bone_id = parent_bone_id
	for child in node.children:
		_collect_bone_hierarchy(child, bone_names, next_parent_bone_id, names, father_ids, bone_index)


def extract_skeleton(path: Path) -> Optional[Tuple[str, SkeletonShape]]:
	"""Builds a brand new `SkeletonShape` from `path`'s own bone hierarchy (a
	fresh starter skeleton, not a replacement for one already paired to this
	kind of asset -- see this module's docstring), or `None` when there's
	nothing to build one from: `.obj` (no bones concept at all), or no bones
	in any mesh (see `_find_armature_root()` for how the armature wrapper
	node is found -- purely structural, no naming convention assumed).
	Reparses `path` independently of _import_via_assimp() (accepted, simple,
	if wasteful for now -- see skel_export.md's transparency note).

	Each bone's rest-pose transform (default_pos/rot_quat/scale) is derived
	from its own `mesh.bones[i].offset_matrix` (mesh-space-to-bone-space at
	bind time) combined with the owning mesh INSTANCE's own node-to-root
	transform (the same `transform` _iter_mesh_instances() computes for that
	mesh's vertices) -- NOT by walking the raw scene-graph node hierarchy
	*starting from the armature's own node* as two earlier, both broken,
	versions of this function did. Found broken 2026-09-05 on a real file
	(`tests/spider.dae`), confirmed with `(IA_AGENT_DEBUG)` prints against
	real assimp-py output before fixing (Nuno: "le squelette est enorme...
	l'araigné n'est pas du tout alignée", then, after the first fix, "le skel
	n'est plus dans la bonne direction (rotation de 90°)" + "l'echelle n'est
	toujours pas bonne"):

	- The armature's own wrapper node (`Spider_Armature`) carries its own
	  arbitrary object-level transform (there: translate (0, 0.24, 41), scale
	  20x) that the MESH's own nodes (siblings of the armature, not its
	  children) never go through at all -- walking down from the armature
	  node baked that unrelated transform into every bone's rest pose
	  (1st fix attempt: switched to `offset_matrix`-based math instead, which
	  fixed the *shape* of the skeleton but not its overall scale/rotation).
	- `offset_matrix` alone is expressed in the mesh's own *raw, unconverted*
	  local space (the file's native units -- centimeters for this file,
	  confirmed via its `<unit meter="0.01">`, confirmed too via
	  `(IA_AGENT_DEBUG)`: raw `mesh.vertices` come back in the 1-3 range, cm-
	  scale, while the final *exported* `.shape` positions are ~0.01-0.05,
	  meter-scale) -- assimp-py's Process_GlobalScale (cm -> m) and the
	  Y-up -> Z-up axis normalization are NOT baked into `offset_matrix`
	  itself; both only get applied when a mesh INSTANCE's own node-to-root
	  transform chain is composed (exactly what `_iter_mesh_instances()`,
	  seeded with `_YUP_TO_ZUP_MATRIX` at `scene.root_node`, already does
	  correctly for mesh vertices -- confirmed via `(IA_AGENT_DEBUG)`:
	  `Process_GlobalScale` leaves its factor sitting on `scene.root_node`'s
	  own scale rather than rewriting `mesh.vertices` directly, so it's only
	  ever applied correctly by composing the *whole* node chain from the
	  scene root, same story as the up-axis conversion). That "2nd fix
	  attempt" bug (rotation off by 90°, scale still wrong) is what this
	  version actually fixes.

	The correct, general formula (derived from Assimp's own documented
	meaning of `aiBone::mOffsetMatrix`, "mesh space to bone space in bind
	pose", not guessed): a bone's absolute bind-pose transform in Ryzom's own
	(Z-up, meters) space is `mesh_instance_transform @
	inverse(bone.offset_matrix)`, where `mesh_instance_transform` is the
	*owning* mesh instance's own `_iter_mesh_instances()` transform (the
	exact same one used to place that mesh's vertices) -- this already
	includes both the axis conversion and the global scale, so no separate
	correction is needed. A bone's LOCAL (parent-relative) transform is then
	`inverse(parent_bone_world) @ bone_world`, or `bone_world` directly for a
	root bone (no parent). `inv_bind_pos` (meant to transform the *exported*,
	already-converted mesh vertices into bone space) is corrected the same
	way: `bone.offset_matrix @ inverse(mesh_instance_transform)`."""
	path = Path(path)
	if path.suffix.lower() == ".obj":
		return None
	import assimp_py

	flags = (assimp_py.Process_Triangulate | assimp_py.Process_JoinIdenticalVertices
	         | assimp_py.Process_GenNormals | assimp_py.Process_GlobalScale)
	scene = _assimp_import_file(assimp_py, path, flags)

	bone_names = {bone.name for mesh in scene.meshes if mesh.bones for bone in mesh.bones}
	if not bone_names:
		return None

	armature_node = _find_armature_root(scene.root_node, bone_names)
	if armature_node is None:
		return None

	# The transform that places each mesh INSTANCE's own vertices into
	# Ryzom's Z-up, meters space -- see this function's own docstring for why
	# a bone's offset_matrix needs the same one composed in. Only the first
	# instance of a given mesh index is kept (a skinned mesh instanced more
	# than once has no single well-defined bind pose anyway; same "first-seen
	# wins" simplification used elsewhere in this module).
	mesh_transforms: Dict[int, Tuple[Tuple[float, ...], ...]] = {}
	for mesh_index, transform in _iter_mesh_instances(scene.root_node, _YUP_TO_ZUP_MATRIX):
		mesh_transforms.setdefault(mesh_index, transform)

	bone_offsets: Dict[str, Tuple[Tuple[float, ...], ...]] = {}
	bone_mesh_transform: Dict[str, Tuple[Tuple[float, ...], ...]] = {}
	for mesh_index, mesh in enumerate(scene.meshes):
		if mesh.bones:
			mesh_transform = mesh_transforms.get(mesh_index, _YUP_TO_ZUP_MATRIX)
			for bone in mesh.bones:
				if bone.name not in bone_offsets:
					bone_offsets[bone.name] = bone.offset_matrix
					bone_mesh_transform[bone.name] = mesh_transform

	names: List[str] = []
	father_ids: List[int] = []
	bone_index: Dict[str, int] = {}
	for child in armature_node.children:
		_collect_bone_hierarchy(child, bone_names, -1, names, father_ids, bone_index)
	if not names:
		return None

	bones: List[Bone] = []
	for i, name in enumerate(names):
		offset = bone_offsets[name]
		mesh_transform = bone_mesh_transform[name]
		inv_bind_pos = _pynel_matrix_from_4x4(_mat_mul_mat(offset, _invert_matrix(mesh_transform)))
		bone_world = _mat_mul_mat(mesh_transform, _invert_matrix(offset))
		father_id = father_ids[i]
		if father_id < 0:
			local_transform = bone_world
		else:
			parent_name = names[father_id]
			parent_world = _mat_mul_mat(bone_mesh_transform[parent_name], _invert_matrix(bone_offsets[parent_name]))
			local_transform = _mat_mul_mat(_invert_matrix(parent_world), bone_world)
		pos, rot_quat, scale = _decompose_matrix(local_transform)
		bones.append(Bone(
			name=name, inv_bind_pos=inv_bind_pos, father_id=father_id, unherit_scale=False,
			lod_disable_distance=0.0, default_pos=pos, default_rot_euler=Vector3(0.0, 0.0, 0.0),
			default_rot_quat=rot_quat, default_scale=scale, default_pivot=Vector3(0.0, 0.0, 0.0),
			skin_scale=Vector3(1.0, 1.0, 1.0),
		))

	lods = [SkeletonLod(distance=0.0, active_bones=[0xFF] * len(bones))]
	return armature_node.name, SkeletonShape(bones=bones, bone_map=dict(bone_index), lods=lods)


# ---------------------------------------------------------------------------
# .anim extraction (see project-todos/pynel/anim_write.md and
# project-todos/forgery/mesh_skel_anim_io.md's item 4/5)
# ---------------------------------------------------------------------------

_ANIM_SAMPLE_RATE = 30.0  # matches anim_write.md's own CAnimationOptimizer-derived constant


def _sample_flat_channel(times, values, num_components: int, t: float):
	"""Linearly interpolates a flat, `num_components`-wide channel (assimp's
	own NodeAnimTrack position_keys/scaling_keys layout) at tick time `t`.
	Clamps to the first/last key outside the track's own range. None if the
	channel is empty."""
	n = len(times)
	if n == 0:
		return None
	if n == 1 or t <= times[0]:
		return values[0:num_components]
	if t >= times[-1]:
		return values[(n - 1) * num_components:n * num_components]
	idx = bisect.bisect_right(times, t) - 1
	idx = max(0, min(idx, n - 2))
	t0, t1 = times[idx], times[idx + 1]
	frac = 0.0 if t1 <= t0 else (t - t0) / (t1 - t0)
	a = values[idx * num_components:(idx + 1) * num_components]
	b = values[(idx + 1) * num_components:(idx + 2) * num_components]
	return [a[i] + (b[i] - a[i]) * frac for i in range(num_components)]


def _sample_quat_channel(times, values, t: float):
	"""Same idea as _sample_flat_channel() for a 4-wide (w,x,y,z, assimp's
	own NodeAnimTrack.rotation_keys convention) quaternion channel --
	component-wise lerp + renormalize ("nlerp") rather than a full slerp:
	a documented simplification, acceptable for the animation-authoring
	rates real content uses (adjacent keys close in angle) -- true slerp
	would also need the same hemisphere-continuity care pynel.
	ryzom_animation._sample_track() applies on the write side (see
	project-todos/pynel/anim_write.md) if this ever needs upgrading."""
	sample = _sample_flat_channel(times, values, 4, t)
	if sample is None:
		return None
	w, x, y, z = sample
	length = (w * w + x * x + y * y + z * z) ** 0.5
	if length <= 0:
		return (1.0, 0.0, 0.0, 0.0)
	return (w / length, x / length, y / length, z / length)


def _compose_trs(pos: Vector3, quat: Quaternion, scale: Vector3):
	"""Inverse of _decompose_matrix(): a row-major 4x4 (translation in the
	last column, each rotation column pre-scaled) -- same convention
	_decompose_matrix() itself expects, so a round-trip is exact modulo
	floating point. Reuses pynel.ryzom_animation._mat_rotate() (already
	proven, see evaluate_all_bone_world_matrices()) for the quaternion ->
	rotation-matrix step rather than re-deriving those formulas."""
	from pynel.ryzom_animation import _mat_rotate
	rot = _mat_rotate(quat)
	return (
		(rot[0][0] * scale.x, rot[0][1] * scale.y, rot[0][2] * scale.z, pos.x),
		(rot[1][0] * scale.x, rot[1][1] * scale.y, rot[1][2] * scale.z, pos.y),
		(rot[2][0] * scale.x, rot[2][1] * scale.y, rot[2][2] * scale.z, pos.z),
		(0.0, 0.0, 0.0, 1.0),
	)


def _node_local_matrix_at(node, track, time_ticks: float):
	"""The local (parent-relative) transform of `node` at `time_ticks` --
	`node.transformation` (its static/bind transform) unless `track` (an
	assimp_py.NodeAnimTrack, or None) has a channel for a given component,
	in which case that channel is sampled instead. A channel `track` doesn't
	carry at all (e.g. a track with only rotation keys) falls back to that
	same component of the node's own static transform, decomposed --
	matches how a partially-animated node behaves in the source tool (only
	the animated channels move)."""
	if track is None:
		return node.transformation
	pos = _sample_flat_channel(list(track.position_times), list(track.position_keys), 3, time_ticks) if track.position_times else None
	rot = _sample_quat_channel(list(track.rotation_times), list(track.rotation_keys), time_ticks) if track.rotation_times else None
	scale = _sample_flat_channel(list(track.scaling_times), list(track.scaling_keys), 3, time_ticks) if track.scaling_times else None
	if pos is None or rot is None or scale is None:
		base_pos, base_rot, base_scale = _decompose_matrix(node.transformation)
		if pos is None:
			pos = (base_pos.x, base_pos.y, base_pos.z)
		if rot is None:
			rot = (base_rot.w, base_rot.x, base_rot.y, base_rot.z)
		if scale is None:
			scale = (base_scale.x, base_scale.y, base_scale.z)
	quat = Quaternion(rot[1], rot[2], rot[3], rot[0])  # (w,x,y,z) -> Quaternion(x,y,z,w)
	return _compose_trs(Vector3(*pos), quat, Vector3(*scale))


_IDENTITY_MATRIX4 = ((1.0, 0.0, 0.0, 0.0), (0.0, 1.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, 0.0, 0.0, 1.0))


def _walk_bone_local_transforms(node, accumulated, bone_names, tracks_by_name, time_ticks: float, result: Dict[str, Tuple]) -> None:
	"""Accumulates NODE-LOCAL transforms (see _node_local_matrix_at(), static
	or animated depending on `tracks_by_name`) starting from an armature
	root's own child, resetting the accumulator to identity every time a
	real bone is reached -- so `result[bone_name]` ends up being that
	bone's own local transform relative to its NEAREST BONE ANCESTOR (or
	the armature root itself, for a top-level bone), composing through any
	non-bone helper node's own transform along the way exactly like
	_collect_bone_hierarchy() already does for topology (same
	bone_names/traversal shape, deliberately).

	Unlike _iter_mesh_instances()/the old world-space walk this replaces,
	this one is NOT seeded with anything at the armature boundary (identity)
	-- it never needs to be: composing `inverse(parent_bone_local) @
	child_bone_local` (see extract_animation()'s own use) cancels out
	whatever the armature's own arbitrary transform was, since both sides
	share that same ancestor, for every bone EXCEPT the top-level one(s)
	(no NeL parent to cancel against) -- see extract_animation()'s own
	docstring for how those are handled instead."""
	local = _node_local_matrix_at(node, tracks_by_name.get(node.name), time_ticks)
	accumulated = _mat_mul_mat(accumulated, local)
	if node.name in bone_names:
		result[node.name] = accumulated
		next_accumulated = _IDENTITY_MATRIX4
	else:
		next_accumulated = accumulated
	for child in node.children:
		_walk_bone_local_transforms(child, next_accumulated, bone_names, tracks_by_name, time_ticks, result)


def extract_animation(path: Path, skeleton: SkeletonShape) -> List[Tuple[str, "Animation"]]:
	"""Builds a pynel `Animation` (`pynel.ryzom_animation.Animation`, via
	`build_animation()`) per animation clip found in `path`'s own assimp
	scene, matching `skeleton`'s own bones by name (typically the one
	`extract_skeleton()` just built from the same file) -- one (clip_name,
	Animation) pair per `scene.animations` entry, ALL of them (see
	mesh_skel_anim_io.md's own decision: unlike `.skel`'s single "the right
	armature" pick, there's no by-name "the right clip" convention for
	animations -- a .dae/.fbx/.gltf with several clips has no reason to name
	any of them after the source file). A clip whose own name contains
	`__skip__` (case-insensitive) is excluded, same convention as
	sub-meshes/the armature (see skel_export.md). `[]` if `path` has no
	animation at all, or is `.obj` (no animation concept in that format).

	Each bone's LOCAL (parent-relative) transform is sampled at a fixed 30Hz
	rate (see anim_write.md's own reasoning) via _walk_bone_local_transforms()
	(node-local composition, reset at each bone boundary) rather than a
	world-space walk from scene.root_node -- an EARLIER version of this
	function did exactly that (mirroring _iter_mesh_instances()), which
	worked for the SKELETON (father_id>=0 bones cancel the shared ancestor
	via inverse(parent_world)@world, see below) but baked the armature
	node's own arbitrary object-level transform (the same one
	extract_skeleton() had to avoid for its OWN bind-pose math, see its
	docstring) into every TOP-LEVEL bone's animated samples with nothing to
	cancel it against -- found 2026-09-05, Nuno: "quand j'applique une
	animation l'araignée devient super grande", the exact same class of bug
	extract_skeleton() itself needed 3 iterations to fix.

	For a non-root bone (father_id>=0), `_walk_bone_local_transforms()`'s
	own reset-at-bone-boundary result IS already the correct final local
	sample -- no further correction needed (composing
	`inverse(parent_bone_world) @ bone_world` in the old world-space walk
	provably cancels any shared ancestor transform above the parent bone
	regardless of what it is, which is exactly what this accumulator
	computes directly). For a TOP-LEVEL bone (no NeL parent to cancel
	against), the accumulator's raw result is instead relative to the
	armature node -- corrected here by anchoring it to `skeleton`'s own
	already-trusted bind-pose world matrix (`evaluate_all_bone_world_matrices()`,
	the same value extract_skeleton()'s own bind pose derivation produces):
	`world(t) = bind_world[bone] @ inverse(bind_local_raw[bone]) @
	local_raw(t)`, where `bind_local_raw` is the same accumulator evaluated
	with no animation applied (`tracks_by_name={}`) -- the animated DELTA
	relative to this bone's own bind pose, composed on top of the already-
	correct absolute bind placement, instead of trusting the armature's raw
	transform at all."""
	path = Path(path)
	if path.suffix.lower() == ".obj":
		return []
	import assimp_py
	from pynel.ryzom_animation import (
		Quaternion as AnimQuaternion, Vector3 as AnimVector3, build_animation, evaluate_all_bone_world_matrices,
	)

	flags = (assimp_py.Process_Triangulate | assimp_py.Process_JoinIdenticalVertices
	         | assimp_py.Process_GenNormals | assimp_py.Process_GlobalScale)
	scene = _assimp_import_file(assimp_py, path, flags)
	if not scene.animations:
		return []

	bone_names = set(skeleton.bone_map)
	armature_node = _find_armature_root(scene.root_node, bone_names)
	if armature_node is None:
		return []
	bind_world = evaluate_all_bone_world_matrices(skeleton)
	bind_local_raw: Dict[str, Tuple] = {}
	for child in armature_node.children:
		_walk_bone_local_transforms(child, _IDENTITY_MATRIX4, bone_names, {}, 0.0, bind_local_raw)

	results = []
	for clip in scene.animations:
		if "__skip__" in (clip.name or "").lower():
			continue
		ticks_per_second = clip.ticks_per_second or 25.0  # Assimp's own documented default
		tracks_by_name = {c.node_name: c for c in (clip.channels or [])}
		duration_seconds = clip.duration / ticks_per_second

		num_samples = max(2, round(duration_seconds * _ANIM_SAMPLE_RATE) + 1)
		sample_times_sec = [i / _ANIM_SAMPLE_RATE for i in range(num_samples)]

		local_raw_by_sample = []
		for t_sec in sample_times_sec:
			t_ticks = t_sec * ticks_per_second
			local_raw: Dict[str, Tuple] = {}
			for child in armature_node.children:
				_walk_bone_local_transforms(child, _IDENTITY_MATRIX4, bone_names, tracks_by_name, t_ticks, local_raw)
			local_raw_by_sample.append(local_raw)

		node_tracks = {}
		for bone in skeleton.bones:
			if any(bone.name not in lr for lr in local_raw_by_sample):
				continue  # bone not present in this scene graph (shouldn't happen for a matching skeleton)
			if bone.father_id < 0:
				bind_correction = _mat_mul_mat(bind_world[bone.name], _invert_matrix(bind_local_raw[bone.name]))
				local_samples = [_mat_mul_mat(bind_correction, lr[bone.name]) for lr in local_raw_by_sample]
			else:
				local_samples = [lr[bone.name] for lr in local_raw_by_sample]
			positions, rotations, scales = [], [], []
			for m in local_samples:
				pos, rot, scale = _decompose_matrix(m)
				# _decompose_matrix() returns pynel.ryzom_shape's own Vector3/
				# Quaternion (right for extract_skeleton()'s Bone fields) --
				# pynel.ryzom_animation.build_animation() needs its OWN,
				# separate Vector3/Quaternion dataclasses instead (same
				# shape, different identity -- _lerp_value()'s isinstance()
				# check silently fails on the wrong one and falls through to
				# its plain-float branch, TypeError on Vector3-Vector3,
				# found 2026-09-05 testing against tests/spider.dae).
				positions.append(AnimVector3(pos.x, pos.y, pos.z))
				rotations.append(AnimQuaternion(rot.x, rot.y, rot.z, rot.w))
				scales.append(AnimVector3(scale.x, scale.y, scale.z))
			node_tracks[bone.name] = {
				"pos": (sample_times_sec, positions),
				"rotquat": (sample_times_sec, rotations),
				"scale": (sample_times_sec, scales),
			}

		if not node_tracks:
			continue
		clip_name = clip.name or path.stem
		results.append((clip_name, build_animation(clip_name, node_tracks, track_format="sampled")))

	return results


def import_dae(path: Path) -> Mesh:
	"""Parses `path` (a COLLADA .dae) via assimp-py, returning a ready-to-save
	Mesh -- see _import_via_assimp()."""
	return _import_via_assimp(path)


def import_fbx(path: Path) -> Mesh:
	"""Parses `path` (an FBX) via assimp-py, returning a ready-to-save Mesh --
	see _import_via_assimp()."""
	return _import_via_assimp(path)


def import_gltf(path: Path) -> Mesh:
	"""Parses `path` (a glTF, JSON or binary .glb) via assimp-py, returning a
	ready-to-save Mesh -- see _import_via_assimp(). Symmetric with
	shape_export.py's own .gltf/.glb export (`_export_gltf_or_glb()`)."""
	return _import_via_assimp(path)


# Single source of truth for "which import_*() handles this extension" --
# shared by every caller (import_dialog.py, apps/shape_importer.py,
# import_watcher.py) instead of each redefining its own copy.
IMPORTERS = {"obj": import_obj, "dae": import_dae, "fbx": import_fbx, "gltf": import_gltf, "glb": import_gltf}


def find_importer(path: Path):
	"""The IMPORTERS entry for `path`'s extension, or None if unsupported."""
	return IMPORTERS.get(Path(path).suffix.lstrip(".").lower())


def texture_search_dirs_for(path: Path) -> List[Path]:
	"""Folders worth falling back to (see shape_geometry.load_panda_texture's
	search_dirs) when resolving a texture referenced by `path`'s own imported
	material data -- just `path`'s own folder, plus its `<name>.fbm` sibling
	for an `.fbx`: the conventional folder 3ds Max/FBX SDK-based exporters
	(and Autodesk's own FBX Converter) drop embedded/referenced texture files
	into next to the .fbx itself, named after it."""
	path = Path(path)
	dirs = [path.parent]
	if path.suffix.lower() == ".fbx":
		dirs.append(path.parent / f"{path.stem}.fbm")
	return dirs
