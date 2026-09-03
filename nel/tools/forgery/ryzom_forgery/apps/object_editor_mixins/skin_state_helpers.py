"""Pure numpy/Panda3D live-skinning math shared by ObjectEditorApp and its
object_editor_mixins/ mixins -- the main shape's own live wind/skin preview
(_rebuild_geometry(), viewport_transform.py) and the Bind preview's assembled
creature (_update_assembled_creature_skin(), creature_bind.py) both build and
blend these same per-vertex tables every frame.

Like ui_helpers.py/geometry_helpers.py, has ZERO dependency on
object_editor.py -- see ui_helpers.py's module docstring for why.
"""

import numpy

from panda3d.core import InternalName

from pynel.ryzom_skin import _matrix_field_to_dense
from ryzom_forgery.shape_geometry import _numpy_skin_batch, finest_skinned_lod


class _WindState:
	"""Per-loaded-shape data for the live wind preview (see
	ObjectEditorApp._update_wind()) -- built once in _rebuild_geometry() from
	the shape's WindTreeParams + PrimaryColor vertex channel, then read every
	frame by the animation task. `vdata` (the shared GeomVertexData every
	render pass's Geom points at, see _build_vertex_data()) is filled in by
	the caller once it's built."""

	def __init__(self, params, base_positions, factors, idx2, idx3):
		self.params = params
		self.base_positions = base_positions  # (N,3) float32
		self.factors = factors  # (N,3) float32, level-0/1/2 blend weight per vertex
		self.idx2 = idx2  # (N,) int, which of the 4 level-1 phase branches each vertex follows
		self.idx3 = idx3  # (N,) int, same for level-2
		self.current_time = [0.0, 0.0, 0.0]  # per level, wrapped to [0, 1), see _update_wind()
		self.vdata = None
		# vdata's array-0 layout (floats per row / the "vertex" column's start
		# offset within a row) -- fixed for the shape's lifetime once vdata is
		# built, so computed once and cached, unlike the numpy view itself
		# (see _update_wind(), that must be re-acquired every frame).
		self.vertex_stride = None
		self.vertex_pos_offset = None


def _build_wind_state(wind_params, vertex_buffer):
	"""Precomputes the per-vertex data _update_wind() needs every frame, from
	the shape's own PrimaryColor channel -- encodes, per vertex, its level-0/
	1/2 wind blend weight (R) and which of the 4 level-1/level-2 phase
	branches it swings with (G/B), exactly like the engine's own
	wind_tree_vp.glsl decodes it (`vprimaryColor.xxx*3 + (0,-1,-2)` clamped,
	and `vprimaryColor.yz*3.99` truncated to an index) -- see
	nel/src/3d/meshvp_wind_tree.cpp for the reference implementation this
	whole preview is ported from. None if the shape has no PrimaryColor data
	to decode (WindTreeParams present but no per-vertex assignment yet)."""
	positions = vertex_buffer.channels.get("Position")
	colors = vertex_buffer.channels.get("PrimaryColor")
	if not positions or not colors:
		return None

	base_positions = numpy.array(positions, dtype=numpy.float32)
	rgb = numpy.array([(c.r, c.g, c.b) for c in colors], dtype=numpy.float32) / 255.0
	r, g, b = rgb[:, 0], rgb[:, 1], rgb[:, 2]
	factors = numpy.clip(numpy.stack([r * 3.0, r * 3.0 - 1.0, r * 3.0 - 2.0], axis=1), 0.0, 1.0).astype(numpy.float32)
	idx2 = numpy.minimum((g * 3.99).astype(numpy.int32), 3)
	idx3 = numpy.minimum((b * 3.99).astype(numpy.int32), 3)
	return _WindState(wind_params, base_positions, factors, idx2, idx3)


_IDENTITY4 = numpy.eye(4, dtype=numpy.float32)


def _bone_inv_bind_matrices(bone_names, skeleton):
	"""(B,4,4) float32 numpy array of each of `bone_names`'s own static
	inverse-bind-pose matrix (CBone.inv_bind_pos -- see
	pynel.ryzom_skin._matrix_field_to_dense()) -- precomputed ONCE per shape
	at build time (_build_skin_state()/_build_mrm_skin_state()), not
	reconverted from its sparse on-disk Matrix form every single frame for
	every shape the way pynel.ryzom_skin.bone_skin_matrices_for_mesh() does
	(fine for its original one-shape use case, the dominant avoidable
	per-frame cost once Bind preview started re-skinning ~8 shapes at once,
	bug found 2026-08-31, Nuno: "10 fps de perdu"). Identity for any bone
	missing from `skeleton.bone_map`."""
	matrices = numpy.empty((len(bone_names), 4, 4), dtype=numpy.float32)
	for i, name in enumerate(bone_names):
		bone_index = skeleton.bone_map.get(name)
		matrices[i] = _matrix_field_to_dense(skeleton.bones[bone_index].inv_bind_pos) if bone_index is not None else _IDENTITY4
	return matrices


def _bone_skin_matrices_numpy(bone_names, inv_bind_matrices, bone_world_matrices):
	"""Per-frame counterpart of _bone_inv_bind_matrices(): gathers the
	CURRENT world matrix of each of `bone_names` (the only part that
	actually changes frame to frame) and combines it with the precomputed
	static inv-bind matrices in a single batched numpy matmul, instead of
	pynel.ryzom_skin.bone_skin_matrices_for_mesh()'s own per-bone pure-Python
	loop. Identity for any bone missing from `bone_world_matrices`."""
	world = numpy.array([bone_world_matrices.get(name, _IDENTITY4) for name in bone_names], dtype=numpy.float32)
	return numpy.matmul(world, inv_bind_matrices)


class _SkinState:
	"""Per-loaded-shape data for live CMeshMRMSkinned re-skinning (see
	ObjectEditorApp._update_skin_preview()) -- built once in _rebuild_geometry()
	from the finest lod's resolved PackedVertex list, then read every frame by
	the animation task. `vdata` is filled in by the caller once it's built,
	same as _WindState."""

	def __init__(self, geom, skeleton, bone_names, local_positions, local_normals, bone_indices, weights):
		self.geom = geom
		self.skeleton = skeleton
		self.bone_names = bone_names  # geom.bones_name, same order/indices as bone_indices
		self.local_positions = local_positions  # (N,4) float32, homogeneous (w=1)
		self.local_normals = local_normals  # (N,4) float32, homogeneous (w=0, no translation)
		self.bone_indices = bone_indices  # (N,4) int32, indices into bone_names
		self.weights = weights  # (N,4) float32, already /255
		self.inv_bind_matrices = _bone_inv_bind_matrices(bone_names, skeleton)  # (B,4,4), see _bone_inv_bind_matrices()
		self.vdata = None
		self.vertex_stride = None
		self.vertex_pos_offset = None
		self.vertex_normal_offset = None


def _build_skin_state(geom, skeleton):
	"""Precomputes the static per-vertex tables _update_skin_preview() blends
	every frame, from the finest lod's geomorph-resolved PackedVertex list
	(see finest_skinned_lod()) -- None if the geom has no lods at all."""
	resolved_lod = finest_skinned_lod(geom)
	if resolved_lod is None:
		return None
	_lod, resolved = resolved_lod

	n = len(resolved)
	local_positions = numpy.empty((n, 4), dtype=numpy.float32)
	local_normals = numpy.empty((n, 4), dtype=numpy.float32)
	bone_indices = numpy.empty((n, 4), dtype=numpy.int32)
	weights = numpy.empty((n, 4), dtype=numpy.float32)
	local_positions[:, 3] = 1.0
	local_normals[:, 3] = 0.0
	for i, v in enumerate(resolved):
		local_positions[i, :3] = v.decompact_pos(geom.decompact_scale)
		local_normals[i, :3] = v.decompact_normal()
		bone_indices[i] = v.matrices
		weights[i] = [w / 255.0 for w in v.weights]

	# Bug found and fixed 2026-08-30 (see shape_geometry.py's
	# _numpy_skin_batch() for the full writeup, same root cause): a
	# zero-weight slot's own matrix_id isn't guaranteed valid content (only
	# slot 0 is, per CMesh::CSkinWeight's own doc comment) -- confirmed
	# real on ma_hof_armor01_pantabottes.shape, a matrix_id of 77 into an
	# 8-bone array, only in an unused slot. _update_skin_preview() gathers
	# all 4 slots unconditionally every frame before weighting, so this
	# crashed on load. Clamp to slot 0's own index (always valid) wherever
	# weight is 0 -- its contribution is zeroed out by the weight anyway,
	# regardless of which bone the clamp picks.
	bone_indices = numpy.where(weights > 0, bone_indices, bone_indices[:, :1])

	return _SkinState(geom, skeleton, list(geom.bones_name), local_positions, local_normals, bone_indices, weights)


def _reskin_state(state, bone_world_matrices):
	"""Blends `state`'s precomputed per-vertex tables (see _build_skin_state())
	against `bone_world_matrices` and writes the result straight into
	`state.vdata` -- the vectorized (numpy) math ObjectEditorApp._update_skin_preview()
	used to do inline for the loaded shape's own live re-skin, factored out here
	so ObjectEditorApp._update_assembled_creature_skin() can reuse it for every
	body-part shape of an assembled Bind-preview creature too (2026-08-31, Nuno:
	"Possible d'avoir un bouton pour lancer l'animation?"). No-op if `state.vdata`
	isn't built yet."""
	if state.vdata is None:
		return
	matrices = _bone_skin_matrices_numpy(state.bone_names, state.inv_bind_matrices, bone_world_matrices)  # (B,4,4)

	# state.bone_indices: (N,4) int, one of the B bones per influence slot --
	# gathers each vertex's own 4 candidate matrices in one shot.
	gathered = matrices[state.bone_indices]  # (N,4,4,4): (vertex, slot, row, col)
	transformed_pos = numpy.einsum("nsij,nj->nsi", gathered, state.local_positions)
	weighted_pos = (transformed_pos[:, :, :3] * state.weights[:, :, None]).sum(axis=1)

	transformed_normal = numpy.einsum("nsij,nj->nsi", gathered, state.local_normals)
	weighted_normal = (transformed_normal[:, :, :3] * state.weights[:, :, None]).sum(axis=1)
	lengths = numpy.linalg.norm(weighted_normal, axis=1, keepdims=True)
	lengths[lengths == 0] = 1.0
	weighted_normal /= lengths

	# Same buffer-protocol/modification-stamp technique as _update_wind() --
	# see its own docstring for why a per-row GeomVertexRewriter loop isn't
	# used instead.
	array_data = state.vdata.modify_array(0)
	if state.vertex_pos_offset is None:
		array_format = array_data.get_array_format()
		state.vertex_stride = array_format.get_stride() // 4
		state.vertex_pos_offset = array_format.get_column(InternalName.get_vertex()).get_start() // 4
		state.vertex_normal_offset = array_format.get_column(InternalName.get_normal()).get_start() // 4

	view = numpy.frombuffer(array_data, dtype=numpy.float32).reshape(-1, state.vertex_stride)
	view[:, state.vertex_pos_offset:state.vertex_pos_offset + 3] = weighted_pos
	view[:, state.vertex_normal_offset:state.vertex_normal_offset + 3] = weighted_normal


class _ShadowSkinPreviewState:
	"""Per-loaded-shape data for the live CShadowSkin ground-shadow preview
	(see ObjectEditorApp._update_shadow_skin_preview()) -- built once in
	_rebuild_geometry() from geom.shadow_skin, then read every frame by the
	animation task, which poses it (if a skeleton is loaded) and flattens it
	onto the ground plane along the current sun direction (see
	_reskin_shadow_skin_state()). One bone per vertex when posed
	(CShadowVertex's own rigid skinning, see pynel.ryzom_shape.ShadowSkin's
	docstring), no weight blending needed -- a strict simplification of
	_SkinState's 4-slot weighted case. `bone_names`/`bone_indices`/
	`inv_bind_matrices` are None when built without a skeleton (see
	_build_shadow_skin_preview_state()) -- the preview then just shows the
	shape's own raw bind-pose positions, ground-projected but unposed."""

	def __init__(self, local_positions, bone_names=None, bone_indices=None, inv_bind_matrices=None):
		self.local_positions = local_positions  # (N,4) float32, homogeneous (w=1)
		self.bone_names = bone_names  # None if no skeleton loaded -- static preview
		self.bone_indices = bone_indices  # (N,) int32, indices into bone_names, or None
		self.inv_bind_matrices = inv_bind_matrices  # or None
		self.vdata = None
		self.vertex_stride = None
		self.vertex_pos_offset = None


def _build_shadow_skin_preview_state(geom, skeleton):
	"""Precomputes the static per-vertex tables _reskin_shadow_skin_state()
	uses every frame, from geom.shadow_skin -- None if it's empty (nothing to
	preview). Unlike _build_skin_state(), a skeleton isn't required: without
	one, the returned state just carries raw bind-pose positions (posing is
	skipped every frame, see _reskin_shadow_skin_state()) -- same "static
	fallback" idea as iter_render_passes()' own unposed render of an
	unskinned CMeshMRMSkinned instance."""
	shadow_skin = geom.shadow_skin
	if not shadow_skin.vertices:
		return None

	n = len(shadow_skin.vertices)
	local_positions = numpy.empty((n, 4), dtype=numpy.float32)
	local_positions[:, 3] = 1.0
	for i, v in enumerate(shadow_skin.vertices):
		local_positions[i, :3] = (v.position.x, v.position.y, v.position.z)

	if skeleton is None:
		return _ShadowSkinPreviewState(local_positions)

	bone_names = list(geom.bones_name)
	bone_indices = numpy.empty(n, dtype=numpy.int32)
	for i, v in enumerate(shadow_skin.vertices):
		bone_indices[i] = v.matrix_id
	inv_bind_matrices = _bone_inv_bind_matrices(bone_names, skeleton)
	return _ShadowSkinPreviewState(local_positions, bone_names, bone_indices, inv_bind_matrices)


def _reskin_shadow_skin_state(state, bone_world_matrices, sun_direction, ground_z):
	"""Poses `state`'s precomputed per-vertex tables against
	`bone_world_matrices` (skipped entirely if state.bone_names is None --
	see _build_shadow_skin_preview_state()'s "static" case), then flattens
	every vertex onto the Z=`ground_z` plane by following `sun_direction`
	(a plain (x,y,z) tuple, the direction the light travels -- see
	ObjectEditorApp._update_shadow_skin_preview()'s own note on where this
	comes from) until it reaches that height, and writes the result into
	`state.vdata` -- same buffer-rewrite technique as _reskin_state(). This
	is a flat "blob shadow" approximation, not a real shadow-map render (no
	soft edges, no terrain-shape wrapping, no blending with other body-part
	shadows -- see the 2026-09-03 discussion for why). No-op if
	`state.vdata` isn't built yet."""
	if state.vdata is None:
		return
	if state.bone_names is not None:
		matrices = _bone_skin_matrices_numpy(state.bone_names, state.inv_bind_matrices, bone_world_matrices)  # (B,4,4)
		gathered = matrices[state.bone_indices]  # (N,4,4)
		posed = numpy.einsum("nij,nj->ni", gathered, state.local_positions)[:, :3]
	else:
		posed = state.local_positions[:, :3]

	dx, dy, dz = sun_direction
	# Guard against a near-horizontal sun (dz~0): the ray toward the ground
	# would need to travel an enormous (or infinite) distance sideways --
	# clamp so the shadow just stretches very far instead of overflowing to
	# +-inf/NaN.
	if -1e-4 < dz < 1e-4:
		dz = -1e-4 if dz <= 0 else 1e-4
	t = (ground_z - posed[:, 2]) / dz
	ground_pos = posed.copy()
	ground_pos[:, 0] += dx * t
	ground_pos[:, 1] += dy * t
	ground_pos[:, 2] = ground_z

	# Same buffer-protocol/modification-stamp technique as _update_wind() --
	# see its own docstring for why a per-row GeomVertexRewriter loop isn't
	# used instead.
	array_data = state.vdata.modify_array(0)
	if state.vertex_pos_offset is None:
		array_format = array_data.get_array_format()
		state.vertex_stride = array_format.get_stride() // 4
		state.vertex_pos_offset = array_format.get_column(InternalName.get_vertex()).get_start() // 4

	view = numpy.frombuffer(array_data, dtype=numpy.float32).reshape(-1, state.vertex_stride)
	view[:, state.vertex_pos_offset:state.vertex_pos_offset + 3] = ground_pos


class _MrmSkinState:
	"""Per-loaded-shape data for live re-skin of a plain skinned CMeshMRM
	body-part shape (e.g. *_visage.shape face pieces, geom.skinned=True but
	NOT the packed-vertex CMeshMRMSkinned format _SkinState/_build_skin_state()
	handle) -- same idea, different on-disk skin-weight layout
	(geom.skin_weights, one (matrix_ids, weights) pair per vertex, see
	shape_geometry._passes_from_mrm_geom()), so it needs its own per-vertex
	tables, PLUS its own geomorph-wedge resolution step
	(shape_geometry._resolve_lod_geomorphs()'s own numpy equivalent --
	precomputed once here as (dst, src) index arrays, applied every frame
	instead of once at build time, see _reskin_mrm_state()). Bug found
	2026-08-31, Nuno: "vu que tout le corps + cheveux bougent, ben le visage
	doit bouger aussi" -- face pieces were rendering skinned but frozen at
	whatever pose they were built with, the same limitation
	ObjectEditorApp._rebuild_geometry() already accepts for the *main*
	shape's own live re-skin ("extending the live path to that format too is
	future work") -- worth doing here since Bind preview assembles several
	body-part shapes at once and a frozen face next to a moving body/hair is
	visually jarring in a way a single standalone shape's own limitation
	never was."""

	def __init__(self, geom, skeleton, bone_names, local_positions, local_normals, matrix_ids, weights, geomorph_dst, geomorph_src):
		self.geom = geom
		self.skeleton = skeleton
		self.bone_names = bone_names
		self.local_positions = local_positions  # (N,3) float32
		self.local_normals = local_normals  # (N,3) float32
		self.matrix_ids = matrix_ids  # (N,4) int64
		self.weights = weights  # (N,4) float32, already normalized
		self.geomorph_dst = geomorph_dst  # (K,) int64 -- geomorph placeholder wedge indices
		self.geomorph_src = geomorph_src  # (K,) int64 -- the "end" wedge each one resolves to
		self.inv_bind_matrices = _bone_inv_bind_matrices(bone_names, skeleton)  # (B,4,4), see _bone_inv_bind_matrices()
		self.vdata = None
		self.vertex_stride = None
		self.vertex_pos_offset = None
		self.vertex_normal_offset = None


def _build_mrm_skin_state(geom, skeleton):
	"""Precomputes the static per-vertex tables _reskin_mrm_state() blends
	every frame, from geom's own raw (unresolved) vertex buffer -- None if
	geom isn't actually skinned, has no lods, or has no vertices at all."""
	if not geom.skinned or not geom.lods:
		return None
	lod = geom.lods[-1]
	local_positions = numpy.array(geom.vertex_buffer.channels.get("Position", []), dtype=numpy.float32)
	if len(local_positions) == 0:
		return None
	normal_channel = geom.vertex_buffer.channels.get("Normal", [])
	local_normals = numpy.zeros_like(local_positions)
	if normal_channel:
		normal_array = numpy.array(normal_channel, dtype=numpy.float32)
		n = min(len(normal_array), len(local_normals))
		local_normals[:n] = normal_array[:n]
	matrix_ids = numpy.array([w[0] for w in geom.skin_weights], dtype=numpy.int64)
	weights = numpy.array([w[1] for w in geom.skin_weights], dtype=numpy.float32)

	n = len(local_positions)
	geomorph_dst = [wedge_index for wedge_index, (_start, end) in enumerate(lod.geomorphs) if end < n]
	geomorph_src = [end for _start, end in lod.geomorphs if end < n]

	return _MrmSkinState(
		geom, skeleton, list(geom.bones_name), local_positions, local_normals, matrix_ids, weights,
		numpy.array(geomorph_dst, dtype=numpy.int64), numpy.array(geomorph_src, dtype=numpy.int64))


def _reskin_mrm_state(state, bone_world_matrices):
	"""Live per-frame equivalent of shape_geometry._passes_from_mrm_geom()'s
	skin + geomorph-resolve steps, for a plain skinned CMeshMRM body-part
	shape -- see _MrmSkinState's own docstring. No-op if state.vdata isn't
	built yet."""
	if state.vdata is None:
		return
	bone_skin_matrices = _bone_skin_matrices_numpy(state.bone_names, state.inv_bind_matrices, bone_world_matrices)
	positions, normals = _numpy_skin_batch(state.local_positions, state.local_normals, state.matrix_ids, state.weights, bone_skin_matrices)
	if len(state.geomorph_dst):
		positions[state.geomorph_dst] = positions[state.geomorph_src]
		normals[state.geomorph_dst] = normals[state.geomorph_src]

	array_data = state.vdata.modify_array(0)
	if state.vertex_pos_offset is None:
		array_format = array_data.get_array_format()
		state.vertex_stride = array_format.get_stride() // 4
		state.vertex_pos_offset = array_format.get_column(InternalName.get_vertex()).get_start() // 4
		state.vertex_normal_offset = array_format.get_column(InternalName.get_normal()).get_start() // 4

	view = numpy.frombuffer(array_data, dtype=numpy.float32).reshape(-1, state.vertex_stride)
	view[:, state.vertex_pos_offset:state.vertex_pos_offset + 3] = positions
	view[:, state.vertex_normal_offset:state.vertex_normal_offset + 3] = normals
