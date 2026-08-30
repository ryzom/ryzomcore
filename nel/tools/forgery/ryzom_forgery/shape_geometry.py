"""Shared geometry/material extraction for CMesh/CMeshMRM/CMeshMultiLod
`.shape` values -- used both by the live 3D editor (`apps/object_editor.py`)
and by the .shape exporters (`shape_export.py`), so the two don't each
maintain their own copy of "how to walk a parsed shape's render passes".
"""

import dataclasses
import math
from pathlib import Path

import numpy
from panda3d.core import PNMImage, StringStream, Texture as PandaTexture

from pynel.ryzom_shape import (
	Matrix, Mesh, MeshGeom, MeshMRM, MeshMRMGeom, MeshMRMSkinned, MeshMRMSkinnedGeom, MeshMultiLod,
	Quaternion, VertexBuffer,
)
from pynel.ryzom_skin import bone_skin_matrices_for_mesh, skin_vertex

from .search_paths import FoundEntry, TEXTURE_FALLBACK_EXTENSIONS

# Subfolders (relative to each of load_panda_texture()'s search_dirs, "" =
# the dir itself) checked for a texture that isn't found by name (`finder`)
# elsewhere -- an imported .fbx/.dae/.obj routinely references textures
# sitting right next to the source file, or in one of these conventional
# sibling folders, rather than inside the configured search paths.
_LOCAL_TEXTURE_SUBDIRS = ("", "tex", "textures", "data")

IDENTITY_QUAT = Quaternion(x=0.0, y=0.0, z=0.0, w=1.0)


def shape_default_rot_quat(shape_value):
	"""shape_value.base.default_rot_quat, or identity if shape_value has no base."""
	base = getattr(shape_value, "base", None)
	if base is None:
		return IDENTITY_QUAT
	return base.default_rot_quat


def rotate_vector_by_quat(v, quat: Quaternion):
	"""Standard quaternion-rotates-a-vector formula."""
	qx, qy, qz, qw = quat.x, quat.y, quat.z, quat.w
	vx, vy, vz = v
	uvx = qy * vz - qz * vy
	uvy = qz * vx - qx * vz
	uvz = qx * vy - qy * vx
	uuvx = qy * uvz - qz * uvy
	uuvy = qz * uvx - qx * uvz
	uuvz = qx * uvy - qy * uvx
	return (vx + 2.0 * (qw * uvx + uuvx), vy + 2.0 * (qw * uvy + uuvy), vz + 2.0 * (qw * uvz + uuvz))


def rotate_mesh_geom(geom: MeshGeom, quat: Quaternion) -> None:
	"""Rotates every Position/Normal in geom.vertex_buffer in place by quat. No-op if quat is identity."""
	if quat == IDENTITY_QUAT:
		return
	channels = geom.vertex_buffer.channels
	positions = channels.get("Position")
	if positions:
		channels["Position"] = [rotate_vector_by_quat(p, quat) for p in positions]
	normals = channels.get("Normal")
	if normals:
		channels["Normal"] = [rotate_vector_by_quat(n, quat) for n in normals]


def _passes_from_mesh_geom(geom: MeshGeom):
	for matrix_block in geom.matrix_blocks:
		for rdr_pass in matrix_block.rdr_passes:
			yield geom.vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def _resolve_lod_geomorphs(vertex_buffer, lod):
	"""Returns a copy of vertex_buffer with its geomorph placeholder wedges
	resolved to a static value for this lod.

	A CMeshMRM's progressive mesh reserves a block of empty wedges (position/
	normal/uv all zero, a default-constructed CWedge()) shared for geomorph
	blending between adjacent LODs -- wedge index i for i < len(lod.geomorphs)
	is one of these, meant to be filled at render time by
	CMeshMRMGeom::applyGeomorph, blending toward geomorphs[i]'s "end" wedge as
	detail increases (see mrm_builder.cpp's wedge-decal step). For a static,
	non-transitioning render of this lod (this tool doesn't animate LOD
	blending), the resolved value is always exactly that "end" wedge --
	without this, those vertices render collapsed at the origin.
	"""
	if not lod.geomorphs:
		return vertex_buffer

	channels = {}
	for name, values in vertex_buffer.channels.items():
		resolved = list(values)
		for wedge_index, (_start, end) in enumerate(lod.geomorphs):
			if end < len(values):
				resolved[wedge_index] = values[end]
		channels[name] = resolved

	return dataclasses.replace(vertex_buffer, channels=channels)


def finest_skinned_lod(geom: MeshMRMSkinnedGeom):
	"""The finest lod (geom.lods[-1], see _passes_from_mrm_geom()'s own note
	on this convention) plus its geomorph-resolved PackedVertex list -- wedge
	index i for i < len(lod.geomorphs) is a placeholder, resolved here to its
	"end" wedge for a static (non-blending) render of this lod, same idea as
	_resolve_lod_geomorphs() for a plain VertexBuffer's channels. None if the
	geom has no lods at all. Exposed (not just used internally by
	_passes_from_mrm_skinned_geom()) since a live per-frame re-skin (e.g.
	object_editor's animated preview) needs this same resolved vertex list to
	build its own static per-vertex tables once, ahead of time."""
	if not geom.lods:
		return None
	lod = geom.lods[-1]
	packed_vertices = geom.packed_vertices
	if not lod.geomorphs:
		return lod, packed_vertices
	resolved = list(packed_vertices)
	for wedge_index, (_start, end) in enumerate(lod.geomorphs):
		if end < len(packed_vertices):
			resolved[wedge_index] = packed_vertices[end]
	return lod, resolved


def _passes_from_mrm_skinned_geom_rigid(geom: MeshMRMSkinnedGeom):
	"""Same shape as _passes_from_mrm_skinned_geom(), but with no skeleton at
	all: yields the mesh's raw bind-pose local vertices (decompacted, not
	skinned by any bone matrix) -- matching the real Ryzom client's own
	fallback for a CMeshMRMSkinned with no skeleton attached
	(CMeshMRMSkinnedGeom::render()'s "no skeleton" path in
	nel/src/3d/mesh_mrm_skinned.cpp, itself sourced from _VBufferFinal/
	getVertexBuffer() -- ugly/rigid, but lets the shape be inspected before a
	compatible .skel is picked)."""
	resolved_lod = finest_skinned_lod(geom)
	if resolved_lod is None:
		return
	lod, resolved = resolved_lod
	vertex_buffer = VertexBuffer(
		name="rigid",
		num_verts=len(resolved),
		vertex_color_format=0,
		channels={
			"Position": [v.decompact_pos(geom.decompact_scale) for v in resolved],
			"Normal": [v.decompact_normal() for v in resolved],
			"TexCoord0": [v.decompact_uv() for v in resolved],
		},
	)
	for rdr_pass in lod.rdr_passes:
		yield vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def _passes_from_mrm_skinned_geom(geom: MeshMRMSkinnedGeom, skeleton, bone_world_matrices):
	"""Skins geom.packed_vertices against `skeleton` (a
	pynel.ryzom_shape.SkeletonShape) at whatever pose `bone_world_matrices`
	(a {bone name: 4x4 matrix} dict, e.g. from
	pynel.ryzom_animation.evaluate_bone_world_matrix(), one entry per bone in
	geom.bones_name) represents, and yields (vertex_buffer, material_id,
	indices) passes -- same shape as the other _passes_from_*_geom() helpers,
	so callers don't need to special-case CMeshMRMSkinned."""
	resolved_lod = finest_skinned_lod(geom)
	if resolved_lod is None:
		return
	lod, resolved = resolved_lod
	bone_skin_matrices = bone_skin_matrices_for_mesh(geom, skeleton, bone_world_matrices)
	skinned = [skin_vertex(v, geom.decompact_scale, bone_skin_matrices) for v in resolved]
	vertex_buffer = VertexBuffer(
		name="skinned",
		num_verts=len(skinned),
		vertex_color_format=0,
		channels={
			"Position": [(v.pos.x, v.pos.y, v.pos.z) for v in skinned],
			"Normal": [(v.normal.x, v.normal.y, v.normal.z) for v in skinned],
			"TexCoord0": [v.uv for v in skinned],
		},
	)
	for rdr_pass in lod.rdr_passes:
		yield vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def _passes_from_mrm_geom(geom: MeshMRMGeom):
	# lods[-1], not lods[0]: CMeshMRM's progressive mesh is streamed
	# coarsest-first (see CMeshMRMGeom::loadFirstLod/loadNextLod in
	# nel/src/3d/mesh_mrm.cpp, and chooseLod's alphaMRM=0 -> numLod=0
	# mapping to the *lowest* poly count) -- lods[0] is the least detailed,
	# lods[-1] the most, matching MeshMRMGeom.num_triangles's own convention.
	if geom.lods:
		lod = geom.lods[-1]
		vertex_buffer = _resolve_lod_geomorphs(geom.vertex_buffer, lod)
		for rdr_pass in lod.rdr_passes:
			yield vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def iter_render_passes(shape_value, skeleton=None, bone_world_matrices=None):
	"""Yields (vertex_buffer, material_id, indices) for the renderable
	geometry of a CMesh/CMeshMRM/CMeshMultiLod(slot 0)/CMeshMRMSkinned shape
	value. `skeleton`/`bone_world_matrices` are only used for CMeshMRMSkinned
	(see _passes_from_mrm_skinned_geom()) -- without them (no skeleton loaded
	yet by the caller), a skinned shape still renders, via
	_passes_from_mrm_skinned_geom_rigid()'s raw bind-pose fallback."""
	if isinstance(shape_value, Mesh):
		yield from _passes_from_mesh_geom(shape_value.geom)
	elif isinstance(shape_value, MeshMRM):
		yield from _passes_from_mrm_geom(shape_value.geom)
	elif isinstance(shape_value, MeshMultiLod) and shape_value.slots:
		slot_geom = shape_value.slots[0].mesh_geom
		if isinstance(slot_geom, MeshGeom):
			yield from _passes_from_mesh_geom(slot_geom)
		elif isinstance(slot_geom, MeshMRMGeom):
			yield from _passes_from_mrm_geom(slot_geom)
	elif isinstance(shape_value, MeshMRMSkinned):
		if skeleton is not None and bone_world_matrices is not None:
			yield from _passes_from_mrm_skinned_geom(shape_value.geom, skeleton, bone_world_matrices)
		else:
			yield from _passes_from_mrm_skinned_geom_rigid(shape_value.geom)


def shape_geom(shape_value):
	"""Returns the MeshGeom/MeshMRMGeom/MeshMRMSkinnedGeom for a
	CMesh/CMeshMRM/CMeshMultiLod(slot 0)/CMeshMRMSkinned shape value -- same
	dispatch as iter_render_passes(), for code that needs geom-level data
	(e.g. vertex_program/WindTreeParams) rather than per-pass vertex buffers.
	None for any other shape type."""
	if isinstance(shape_value, (Mesh, MeshMRM, MeshMRMSkinned)):
		return shape_value.geom
	if isinstance(shape_value, MeshMultiLod) and shape_value.slots:
		return shape_value.slots[0].mesh_geom
	return None


def shape_bbox(shape_value):
	if isinstance(shape_value, (Mesh, MeshMRM, MeshMRMSkinned)):
		return shape_value.bbox
	if isinstance(shape_value, MeshMultiLod) and shape_value.slots:
		slot_geom = shape_value.slots[0].mesh_geom
		return slot_geom.bbox if slot_geom is not None else None
	return None


def rgba_to_color(rgba):
	return (rgba.r / 255.0, rgba.g / 255.0, rgba.b / 255.0, rgba.a / 255.0)


def solid_color_texture(color):
	"""A 1x1 Panda3D Texture filled with `color` (r, g, b[, a] floats 0-1) --
	lets a plain material color be shown with the exact same UI widget as a
	real texture (e.g. an ImGui image_button), border/hover styling and all,
	instead of a hand-approximated look-alike."""
	image = PNMImage(1, 1, 4)  # explicit 4 channels: set_xel_a() needs an alpha channel to exist
	r, g, b = color[0], color[1], color[2]
	a = color[3] if len(color) > 3 else 1.0
	image.set_xel_a(0, 0, r, g, b, a)
	texture = PandaTexture()
	texture.load(image)
	return texture


def _find_local_texture_ref(name, search_dirs):
	"""Fallback for load_panda_texture() when `finder` doesn't have `name`
	either -- checks each of `search_dirs` (typically just the folder an
	imported mesh file was loaded from) and their tex/textures/data
	subfolders, same name-matching rules as search_paths.find_texture()
	(case-insensitive, also tries swapping the extension for another common
	texture one)."""
	candidates = [name.lower()]
	stem = Path(name).stem.lower()
	candidates += [stem + extension for extension in TEXTURE_FALLBACK_EXTENSIONS]

	for base_dir in search_dirs:
		for subdir in _LOCAL_TEXTURE_SUBDIRS:
			folder = (base_dir / subdir) if subdir else base_dir
			if not folder.is_dir():
				continue
			try:
				entries = {path.name.lower(): path for path in folder.iterdir() if path.is_file()}
			except OSError:
				continue
			for candidate in candidates:
				match = entries.get(candidate)
				if match is not None:
					return FoundEntry(name=match.name, fs_path=match, bnp_path=None)
	return None


def resolve_texture_ref(name, search_dirs=None, finder=None):
	"""Resolves a texture reference by name, the same two-step order
	load_panda_texture() uses internally: `search_dirs`
	(_find_local_texture_ref(), typically just the loaded shape's own
	folder) first, then `finder(name)` (e.g. SearchPathsDialog.find_texture).
	None if neither finds it. Exposed separately (not just inlined in
	load_panda_texture()) for callers that need to resolve a specific
	texture reference -- to inspect it (e.g. panoply_live.is_baked_stale())
	or decode it themselves (e.g. panoply_texture.ref_to_rgba_array()) --
	without going through load_panda_texture()'s own name-keyed cache.

	`name` may also be a full absolute path (see shape_import.py's
	_texture_base_name() -- an imported .obj/.dae/.fbx keeps one as-is
	instead of collapsing to a bare name, as long as it resolves to a real
	file at import time): resolved directly, bypassing search_dirs/finder
	entirely, since those only ever match by bare name. A stale absolute
	reference (the file's since moved/gone) falls back to a by-name search
	on just its basename instead of failing outright."""
	ref_path = Path(name)
	if ref_path.is_absolute():
		if ref_path.is_file():
			return FoundEntry(name=ref_path.name, fs_path=ref_path, bnp_path=None)
		name = ref_path.name
	ref = _find_local_texture_ref(name, search_dirs) if search_dirs else None
	if ref is None and finder is not None:
		ref = finder(name)
	return ref


_NEL_WRAP_TO_PANDA = {0: PandaTexture.WM_repeat, 1: PandaTexture.WM_clamp}
_NEL_MAG_FILTER_TO_PANDA = {0: PandaTexture.FT_nearest, 1: PandaTexture.FT_linear}
_NEL_MIN_FILTER_TO_PANDA = {
	0: PandaTexture.FT_nearest,
	1: PandaTexture.FT_nearest_mipmap_nearest,
	2: PandaTexture.FT_nearest_mipmap_linear,
	3: PandaTexture.FT_linear,
	4: PandaTexture.FT_linear_mipmap_nearest,
	5: PandaTexture.FT_linear_mipmap_linear,
}


def load_panda_texture(name, cache=None, search_dirs=None, repeat=False, finder=None,
                        wrap_s=None, wrap_t=None, min_filter=None, mag_filter=None,
                        load_grayscale_as_alpha=None):
	"""Resolves and decodes a material texture reference (by base file name,
	as stored in the shape's Texture.file_name) into a Panda3D Texture --
	see resolve_texture_ref() for how `name` is actually found. `finder`
	just needs to return something with `.name` and `.read_bytes()`, same
	duck-typed shape as a search_paths.FoundEntry. Returns None if it can't
	be found/decoded. `cache`, if given, is a dict reused across calls to
	avoid re-decoding the same texture for multiple materials/passes --
	`repeat`/`wrap_s`/`wrap_t`/`min_filter`/`mag_filter` only have an effect
	the first time a given `name` is actually loaded (a cache hit skips
	straight past them), matching how these are really a property of how
	the shape's own material uses the texture, not of the texture file
	itself.

	`wrap_s`/`wrap_t`/`min_filter`/`mag_filter` are the real, per-material
	values from the shape's own Texture (None if the caller has none, e.g.
	a freshly imported mesh with no real value of its own -- see
	shape_import.py) -- when given, they take priority over `repeat`'s
	heuristic guess entirely for that axis/filter."""
	if cache is not None and name in cache:
		return cache[name]

	texture = None
	ref = resolve_texture_ref(name, search_dirs, finder)
	if ref is None:
		print(f"[shape_geometry] texture not found: {name!r}")
	else:
		try:
			data = ref.read_bytes()
		except OSError as exc:
			print(f"[shape_geometry] failed to read {ref.name!r}: {exc}")
			data = None

		if data is not None and ref.name.lower().endswith(".dds"):
			# DDS is a compressed/GPU-native format; PNMImage can't read it,
			# Texture has a dedicated loader for it.
			texture = PandaTexture()
			try:
				decoded = texture.read_dds(StringStream(data), ref.name, False)
			except AssertionError as exc:
				# Some DDS files in the wild have a malformed header (e.g. a
				# linearSize field that doesn't match the actual pitch) that
				# trips an assert inside Panda3D's C++ reader instead of
				# read_dds() returning False gracefully.
				print(f"[shape_geometry] malformed DDS header in {ref.name!r}: {exc}")
				decoded = False
			if not decoded:
				print(f"[shape_geometry] failed to decode DDS texture {ref.name!r}")
				texture = None
		elif data is not None:
			image = PNMImage()
			if image.read(StringStream(data), ref.name):
				# NeL .shape files' V-origin flip (relative to Panda3D) is
				# applied once, unconditionally, for every texture format
				# alike, via the UV coordinates written in
				# object_editor.py's _build_vertex_data() -- not here, since
				# the DDS path has no equivalent way to flip a compressed
				# image's pixel rows after decoding. That flip is safe to
				# apply to every shape's data uniformly (real .shape files
				# and freshly-imported .obj/.dae/.fbx ones alike) because
				# shape_import.py's importers already convert the source
				# format's own V convention into this same NeL one at import
				# time, in _assemble_mesh() -- see its comment there.
				texture = PandaTexture()
				texture.load(image)
				if load_grayscale_as_alpha and image.get_num_channels() == 1:
					# Matches CBitmap::_LoadGrayscaleAsAlpha's alphaToRGBA
					# (bitmap.cpp:806, already ported once for dds_export.py's
					# load_rgba() -- same transform, reused here on the
					# already-decoded Panda ram image instead of re-reading
					# the file): a pure single-channel image, normally shown
					# as a grey color (Panda's own default), is instead
					# treated as an alpha-only mask -- RGB forced to white,
					# alpha = the grey value.
					width, height = texture.get_x_size(), texture.get_y_size()
					rgba = numpy.frombuffer(
						texture.get_ram_image_as("RGBA"), dtype=numpy.uint8).reshape(height, width, 4).copy()
					grey = rgba[..., 0].copy()
					rgba[..., 0] = 255
					rgba[..., 1] = 255
					rgba[..., 2] = 255
					rgba[..., 3] = grey
					texture.set_ram_image_as(rgba.tobytes(), "RGBA")
			else:
				print(f"[shape_geometry] failed to decode texture {ref.name!r}")

	if texture is not None:
		if wrap_s is not None:
			texture.set_wrap_u(_NEL_WRAP_TO_PANDA.get(wrap_s, PandaTexture.WM_repeat))
		elif repeat:
			# Panda3D's own default wrap mode is already WM_clamp, correct
			# for the common case of a single, non-tiling texture per
			# material -- only overridden when the caller (object_editor.py's
			# _uvs_need_repeat()) actually found UVs relying on tiling (e.g. a
			# whole render pass at V in [1, 2) on ooc_summer_raceline.shape).
			# Switching every texture to repeat unconditionally visibly
			# shifted imported (.fbx/.dae/.obj) meshes' textures instead:
			# their UVs routinely overshoot [0, 1] by float noise with no
			# tiling intent, and repeat wraps that into a full, visible seam.
			# This heuristic only applies here (wrap_s is None) -- a real
			# shape's own wrap_s/wrap_t above always wins.
			texture.set_wrap_u(PandaTexture.WM_repeat)
		if wrap_t is not None:
			texture.set_wrap_v(_NEL_WRAP_TO_PANDA.get(wrap_t, PandaTexture.WM_repeat))
		elif repeat:
			texture.set_wrap_v(PandaTexture.WM_repeat)
		if mag_filter is not None:
			texture.set_magfilter(_NEL_MAG_FILTER_TO_PANDA.get(mag_filter, PandaTexture.FT_linear))
		if min_filter is not None:
			texture.set_minfilter(_NEL_MIN_FILTER_TO_PANDA.get(min_filter, PandaTexture.FT_linear_mipmap_linear))

	if cache is not None:
		cache[name] = texture
	return texture


def load_panda_cube_texture(sub_textures, cache=None, search_dirs=None, repeat=False, finder=None):
	"""Assembles a Panda3D cube-map Texture from a CTextureCube's 6 faces
	(pynel Texture.sub_textures). NeL's own face order -- texture_cube.h's
	TFace enum: positive_x, negative_x, positive_y, negative_y, positive_z,
	negative_z -- is the same order Panda3D expects for a cube map's ram
	image z-slices, so sub_textures[i] maps straight to z=i with no
	reordering. A missing/undecodable face falls back to the first
	successfully decoded one (a cube map with a hole is worse than one with
	a repeated face). `cache` is the same per-2D-texture decode cache
	load_panda_texture() takes -- this only adds the cost of reassembling
	the 6 (already-cached) faces into one cube texture, not re-decoding
	them."""
	if not sub_textures or len(sub_textures) != 6:
		return None

	faces = []
	for face_tex in sub_textures:
		if face_tex is None or not face_tex.file_name:
			faces.append(None)
			continue
		faces.append(load_panda_texture(
			face_tex.file_name, cache=cache, search_dirs=search_dirs, repeat=repeat, finder=finder,
			wrap_s=face_tex.wrap_s, wrap_t=face_tex.wrap_t,
			min_filter=face_tex.min_filter, mag_filter=face_tex.mag_filter,
			load_grayscale_as_alpha=face_tex.load_grayscale_as_alpha))

	decoded = [t for t in faces if t is not None]
	if not decoded:
		return None
	fallback = decoded[0]
	faces = [t if t is not None else fallback for t in faces]

	size = fallback.get_x_size()
	if any(t.get_x_size() != size or t.get_y_size() != size for t in faces):
		print("[shape_geometry] cube map faces have mismatched sizes, skipping")
		return None

	cube_texture = PandaTexture()
	cube_texture.setup_cube_map(size, PandaTexture.T_unsigned_byte, PandaTexture.F_rgba)
	cube_texture.set_ram_image(b"".join(t.get_ram_image_as("RGBA") for t in faces))
	return cube_texture


def decompose_uv_matrix(matrix):
	"""Decomposes a texture-stage UV Matrix (Material.tex_user_mat[stage] --
	NeL's generic CMatrix, loaded verbatim as the OpenGL texture matrix, see
	driver_opengl_material.cpp's glLoadMatrixf(mat.getUserTexMat(k).get()))
	into a friendly (offset_u, offset_v, scale_u, scale_v, rotation_degrees)
	tuple. Assumes the matrix only ever encodes offset+scale+rotation (no
	shear/projection) -- true for every known real use of this field, a
	3dsMax material's UV Offset/Tiling/Angle rollout. Returns the identity
	transform if `matrix` is None (the field isn't set)."""
	if matrix is None:
		return 0.0, 0.0, 1.0, 1.0, 0.0
	scale = matrix.scale
	a, c = (matrix.rot[0], matrix.rot[3]) if matrix.rot else (1.0, 0.0)
	b, d = (matrix.rot[1], matrix.rot[4]) if matrix.rot else (0.0, 1.0)
	a, b, c, d = a * scale, b * scale, c * scale, d * scale
	scale_u = math.hypot(a, c)
	scale_v = math.hypot(b, d)
	rotation = math.degrees(math.atan2(c, a))
	offset_u, offset_v = (matrix.trans[0], matrix.trans[1]) if matrix.trans else (0.0, 0.0)
	return offset_u, offset_v, scale_u, scale_v, rotation


def compose_uv_matrix(offset_u, offset_v, scale_u, scale_v, rotation_degrees):
	"""Inverse of decompose_uv_matrix() -- builds a Matrix ready to store in
	Material.tex_user_mat[stage] from the friendly values."""
	angle = math.radians(rotation_degrees)
	cos_a, sin_a = math.cos(angle), math.sin(angle)
	a, b = scale_u * cos_a, -scale_v * sin_a
	c, d = scale_u * sin_a, scale_v * cos_a
	rot = (a, b, 0.0, c, d, 0.0, 0.0, 0.0, 1.0)
	trans = (offset_u, offset_v, 0.0)
	# state_bit: bit 0 = has_trans, any of bits 1/2/3 = has_rot (see
	# ryzom_shape.py's Matrix.matrix()) -- both always present here.
	return Matrix(state_bit=0b0011, scale=1.0, rot=rot, trans=trans, proj=None)


def uv_matrix_to_panda_mat4(matrix):
	"""Builds the Panda3D Mat4 for NodePath.set_tex_transform() from a
	texture-stage UV Matrix (Material.tex_user_mat[stage]), for Patina's own
	3D preview. NeL loads this matrix verbatim as the OpenGL texture matrix
	(driver_opengl_material.cpp's glLoadMatrixf(mat.getUserTexMat(k).get())),
	but Panda3D's own V axis runs opposite to that (same underlying cause as
	this module's own V-origin flip for baked vertex UVs, see
	load_panda_texture()'s docstring) -- confirmed by testing (2026-08-28)
	against the real client. Two DIFFERENT, both-needed corrections, not
	one: negating the matrix's own V-scale (b, d below) fixes the tiling
	pattern's alignment, but that alone also mirrors the image content
	(unavoidable -- that's what negating a scale axis does) -- pre-
	multiplying by a V-flip (V' = 1-V, applied to the input before
	anything else) puts the content back right-side up again afterwards.
	Both steps were confirmed needed together by testing, not derived from
	first principles -- don't try to "simplify" this to just one of them.
	This is Patina-preview-only -- the matrix actually saved to the .shape
	(compose_uv_matrix()) is untouched, since the real client already
	reads that one correctly as-is. Rotation direction is also reversed,
	confirmed separately (2026-08-28) -- all three corrections here (V-
	scale negate, V-flip pre-multiply, rotation reversal) were found and
	confirmed independently; don't assume any one of them implies another.

	Deliberately does NOT go through decompose_uv_matrix() first -- that
	always reports a positive scale magnitude (see its own docstring),
	which would silently discard the negated V-scale before this ever saw
	it. Reads a/b/c/d/tx/ty directly instead."""
	from panda3d.core import Mat4

	if matrix is None:
		return Mat4.ident_mat()
	a, b, _, c, d, _, *_ = matrix.rot if matrix.rot else (1.0, 0.0, 0.0, 0.0, 1.0, 0.0)
	scale = matrix.scale
	a, b, c, d = a * scale, b * scale, c * scale, d * scale
	b, c = -b, -c  # reverses the rotation direction -- confirmed needed by testing (2026-08-28)
	b, d = -b, -d  # negates the matrix's own V-scale -- fixes tiling alignment, but mirrors content
	tx, ty = (matrix.trans[0], matrix.trans[1]) if matrix.trans else (0.0, 0.0)
	ty = -ty  # confirmed separately (Offset V), unrelated to the two V-axis corrections above
	mat = Mat4(
		a, b, 0, 0,
		c, d, 0, 0,
		0, 0, 1, 0,
		tx, ty, 0, 1,
	)
	# Pre-multiply (flip is on the LEFT: v * flip * mat) so it acts on the
	# raw input V before anything else -- puts the content the above V-
	# scale negation mirrored back right-side up, without undoing that
	# negation's effect on the tiling alignment.
	flip_v = Mat4(
		1, 0, 0, 0,
		0, -1, 0, 0,
		0, 0, 1, 0,
		0, 1, 0, 1,
	)
	return flip_v * mat


def texture_to_pnm_image(panda_texture):
	"""Reads a Panda3D Texture's pixel data back out into a PNMImage (e.g.
	to save it as a plain .png), regardless of what format it was originally
	decoded from (tga/png/dds all end up as a normal Texture)."""
	image = PNMImage()
	panda_texture.store(image)
	return image
