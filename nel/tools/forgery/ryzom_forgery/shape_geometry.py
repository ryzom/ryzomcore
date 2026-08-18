"""Shared geometry/material extraction for CMesh/CMeshMRM/CMeshMultiLod
`.shape` values -- used both by the live 3D editor (`apps/object_editor.py`)
and by the .shape exporters (`shape_export.py`), so the two don't each
maintain their own copy of "how to walk a parsed shape's render passes".
"""

import dataclasses
from pathlib import Path

from panda3d.core import PNMImage, StringStream, Texture as PandaTexture

from pynel.ryzom_shape import (
	Mesh, MeshGeom, MeshMRM, MeshMRMGeom, MeshMRMSkinned, MeshMRMSkinnedGeom, MeshMultiLod,
	VertexBuffer,
)
from pynel.ryzom_skin import bone_skin_matrices_for_mesh, skin_vertex

from .asset_index import AssetRef, TEXTURE_FALLBACK_EXTENSIONS

# Subfolders (relative to each of load_panda_texture()'s search_dirs, "" =
# the dir itself) checked for a texture that isn't anywhere in the indexed
# Ryzom asset root -- an imported .fbx/.dae/.obj routinely references
# textures sitting right next to the source file, or in one of these
# conventional sibling folders, rather than inside the game's own data tree.
_LOCAL_TEXTURE_SUBDIRS = ("", "tex", "textures", "data")


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
	(see _passes_from_mrm_skinned_geom()) -- without them (skeleton not yet
	loaded by the caller), a skinned shape simply yields nothing, same as a
	shape type with no renderable geometry at all."""
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
	"""Fallback for load_panda_texture() when `name` isn't in the AssetIndex
	at all -- checks each of `search_dirs` (typically just the folder an
	imported mesh file was loaded from) and their tex/textures/data
	subfolders, same name-matching rules as AssetIndex.find_texture()
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
					return AssetRef(name=match.name, path=match)
	return None


def load_panda_texture(asset_index, name, cache=None, search_dirs=None, repeat=False):
	"""Resolves and decodes a material texture reference (by base file name,
	as stored in the shape's Texture.file_name) into a Panda3D Texture, via
	the given AssetIndex, falling back to _find_local_texture_ref(search_dirs)
	if given and the AssetIndex doesn't have it. Returns None if it can't be
	found/decoded. `cache`, if given, is a dict reused across calls to avoid
	re-decoding the same texture for multiple materials/passes -- `repeat`
	only has an effect the first time a given `name` is actually loaded (a
	cache hit skips straight past it), matching how the wrap mode is really a
	property of how the shape's own UVs use the texture, not of the texture
	file itself."""
	if cache is not None and name in cache:
		return cache[name]

	texture = None
	ref = asset_index.find_texture(name)
	if ref is None and search_dirs:
		ref = _find_local_texture_ref(name, search_dirs)
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
			else:
				print(f"[shape_geometry] failed to decode texture {ref.name!r}")

	if texture is not None and repeat:
		# Panda3D's own default wrap mode is already WM_clamp, correct for
		# the common case of a single, non-tiling texture per material --
		# only overridden when the caller (object_editor.py's
		# _uvs_need_repeat()) actually found UVs relying on tiling (e.g. a
		# whole render pass at V in [1, 2) on ooc_summer_raceline.shape).
		# Switching every texture to repeat unconditionally visibly shifted
		# imported (.fbx/.dae/.obj) meshes' textures instead: their UVs
		# routinely overshoot [0, 1] by float noise with no tiling intent,
		# and repeat wraps that into a full, visible seam.
		texture.set_wrap_u(PandaTexture.WM_repeat)
		texture.set_wrap_v(PandaTexture.WM_repeat)

	if cache is not None:
		cache[name] = texture
	return texture


def texture_to_pnm_image(panda_texture):
	"""Reads a Panda3D Texture's pixel data back out into a PNMImage (e.g.
	to save it as a plain .png), regardless of what format it was originally
	decoded from (tga/png/dds all end up as a normal Texture)."""
	image = PNMImage()
	panda_texture.store(image)
	return image
