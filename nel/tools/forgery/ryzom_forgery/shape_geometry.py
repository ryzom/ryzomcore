"""Shared geometry/material extraction for CMesh/CMeshMRM/CMeshMultiLod
`.shape` values -- used both by the live 3D editor (`apps/object_editor.py`)
and by the .shape exporters (`shape_export.py`), so the two don't each
maintain their own copy of "how to walk a parsed shape's render passes".
"""

import collections
import dataclasses
import math
from pathlib import Path

import numpy
from panda3d.core import PNMImage, StringStream, Texture as PandaTexture

from pynel.ryzom_shape import (
	Matrix, Mesh, MeshGeom, MeshMRM, MeshMRMGeom, MeshMRMSkinned, MeshMRMSkinnedGeom, MeshMultiLod,
	Quaternion, ShadowSkin, ShadowVertex, Vector3, VertexBuffer,
)
from pynel.ryzom_skin import bone_skin_matrices_for_mesh

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


def _numpy_skin_batch(local_positions, local_normals, matrix_ids, weights, bone_skin_matrices):
	"""Vectorized (numpy) equivalent of pynel.ryzom_skin's per-vertex
	skin_vertex()/skin_mesh_mrm_geom() loops -- same linear-blend formula
	(BoneSkinMatrix_i(localPos) weighted-summed over up to 4 influences),
	just computed for the whole mesh in one batch instead of once per
	vertex in plain Python. `local_positions`/`local_normals`: (N,3)
	float32. `matrix_ids`: (N,4) int, indices into `bone_skin_matrices`
	(same convention as PackedVertex.matrices / CMeshMRMGeom.skin_weights's
	matrix_id). `weights`: (N,4) float32, already normalized to sum to ~1
	(caller divides PackedVertex's 0-255 packed weights by 255 first --
	CMeshMRMGeom.skin_weights's own floats need no such scaling, see
	skin_mesh_mrm_geom()'s own docstring). `bone_skin_matrices`: (B,4,4)
	float32. Introduced 2026-08-30: building 7+ skinned body-part shapes
	for Patina's creature assembler in pure Python took ~100-160ms each
	(~700ms total per rebuild) -- this cuts that down to the cost of a
	handful of numpy ops per shape, same technique object_editor.py's own
	_update_skin_preview() already uses for its live per-frame re-skin.
	Returns (positions, normals), each (N,3) float32."""
	pos_h = numpy.concatenate([local_positions, numpy.ones((len(local_positions), 1), dtype=numpy.float32)], axis=1)
	normal_h = numpy.concatenate([local_normals, numpy.zeros((len(local_normals), 1), dtype=numpy.float32)], axis=1)

	# The plain-Python original (skin_vertex()) skips any slot>0 with
	# weight==0 entirely ("if weight == 0 and slot > 0: continue") --
	# CMesh::CSkinWeight's own doc comment ("if you don't use all matrix
	# for this vertex, use at least the 0th matrix") confirms unused slots'
	# matrix_id is content the exporter never bothered making valid, only
	# slot 0 is guaranteed real. Gathering all 4 slots unconditionally (as
	# fancy-indexing needs to, before weighting) can hit one of those,
	# crashing on a garbage/out-of-range id whose contribution would've
	# been zero anyway (confirmed real, 2026-08-30:
	# fy_hof_civil01_armpad.shape had a matrix_id of 17 into a 13-bone
	# array, only in an unused, zero-weight slot) -- clamp those to a safe
	# index first, since a weight of 0 zeroes out whatever garbage would
	# otherwise be gathered there regardless of which bone it nominally is.
	safe_matrix_ids = numpy.where(weights > 0, matrix_ids, 0)
	gathered = bone_skin_matrices[safe_matrix_ids]  # (N,4,4,4): (vertex, slot, row, col)
	transformed_pos = numpy.einsum("nsij,nj->nsi", gathered, pos_h)
	weighted_pos = (transformed_pos[:, :, :3] * weights[:, :, None]).sum(axis=1)
	transformed_normal = numpy.einsum("nsij,nj->nsi", gathered, normal_h)
	weighted_normal = (transformed_normal[:, :, :3] * weights[:, :, None]).sum(axis=1)

	lengths = numpy.linalg.norm(weighted_normal, axis=1, keepdims=True)
	lengths[lengths == 0] = 1.0
	weighted_normal = weighted_normal / lengths

	# No real influence at all (shouldn't happen for well-formed content,
	# see skin_vertex()'s own note) -- fall back to the raw local vertex
	# rather than an all-zero point.
	no_influence = weights.sum(axis=1) == 0
	if no_influence.any():
		weighted_pos[no_influence] = local_positions[no_influence]
		weighted_normal[no_influence] = local_normals[no_influence]

	return weighted_pos, weighted_normal


def _passes_from_mrm_skinned_geom(geom: MeshMRMSkinnedGeom, skeleton, bone_world_matrices):
	"""Skins geom.packed_vertices against `skeleton` (a
	pynel.ryzom_shape.SkeletonShape) at whatever pose `bone_world_matrices`
	(a {bone name: 4x4 matrix} dict, e.g. from
	pynel.ryzom_animation.evaluate_bone_world_matrix(), one entry per bone in
	geom.bones_name) represents, and yields (vertex_buffer, material_id,
	indices) passes -- same shape as the other _passes_from_*_geom() helpers,
	so callers don't need to special-case CMeshMRMSkinned. Vectorized (numpy)
	via _numpy_skin_batch() -- see its own docstring for why."""
	resolved_lod = finest_skinned_lod(geom)
	if resolved_lod is None:
		return
	lod, resolved = resolved_lod
	bone_skin_matrices = numpy.array(bone_skin_matrices_for_mesh(geom, skeleton, bone_world_matrices), dtype=numpy.float32)

	local_positions = numpy.array([v.decompact_pos(geom.decompact_scale) for v in resolved], dtype=numpy.float32)
	local_normals = numpy.array([v.decompact_normal() for v in resolved], dtype=numpy.float32)
	matrix_ids = numpy.array([v.matrices for v in resolved], dtype=numpy.int64)
	weights = numpy.array([v.weights for v in resolved], dtype=numpy.float32) / 255.0
	uvs = [v.decompact_uv() for v in resolved]

	positions, normals = _numpy_skin_batch(local_positions, local_normals, matrix_ids, weights, bone_skin_matrices)
	vertex_buffer = VertexBuffer(
		name="skinned",
		num_verts=len(resolved),
		vertex_color_format=0,
		channels={
			"Position": [tuple(p) for p in positions],
			"Normal": [tuple(n) for n in normals],
			"TexCoord0": uvs,
		},
	)
	for rdr_pass in lod.rdr_passes:
		yield vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def _passes_from_mrm_geom(geom: MeshMRMGeom, skeleton=None, bone_world_matrices=None):
	# lods[-1], not lods[0]: CMeshMRM's progressive mesh is streamed
	# coarsest-first (see CMeshMRMGeom::loadFirstLod/loadNextLod in
	# nel/src/3d/mesh_mrm.cpp, and chooseLod's alphaMRM=0 -> numLod=0
	# mapping to the *lowest* poly count) -- lods[0] is the least detailed,
	# lods[-1] the most, matching MeshMRMGeom.num_triangles's own convention.
	if not geom.lods:
		return
	lod = geom.lods[-1]

	# A plain CMeshMRM (not CMeshMRMSkinned) can itself carry skin data --
	# confirmed real, 2026-08-30, e.g. Ryzom's *_visage.shape face pieces
	# (geom.skinned=True, geom.bones_name/geom.skin_weights populated).
	# Previously always rendered rigid regardless (this branch had no
	# skeleton-aware path at all) -- was the actual cause of a "face sits at
	# the wrong height, differently per race" bug in Patina's creature
	# assembler, not a positioning/transform issue as first assumed. See
	# pynel.ryzom_skin.skin_mesh_mrm_geom() for the different on-disk skin
	# data format vs. CMeshMRMSkinned's.
	if geom.skinned and skeleton is not None and bone_world_matrices is not None:
		# Vectorized (numpy) equivalent of pynel.ryzom_skin.skin_mesh_mrm_geom()
		# -- see _numpy_skin_batch()'s own docstring for why. skin_weights'
		# floats are already normalized (unlike PackedVertex's 0-255 packed
		# ones, see skin_mesh_mrm_geom()'s own docstring), no /255 needed here.
		bone_skin_matrices = numpy.array(bone_skin_matrices_for_mesh(geom, skeleton, bone_world_matrices), dtype=numpy.float32)
		local_positions = numpy.array(geom.vertex_buffer.channels.get("Position", []), dtype=numpy.float32)
		# pynel.ryzom_skin.skin_mesh_mrm_geom()'s own per-vertex reference
		# falls back to a zero vector for any vertex the Normal channel
		# doesn't cover (missing entirely, or shorter than Position) --
		# matched here rather than feeding the raw channel straight to
		# _numpy_skin_batch(), which assumes local_normals is already
		# (N,3): an absent/short channel there crashes outright (an empty
		# list becomes a 1-D (0,) array, invalid for the batch's own
		# axis=1 concatenate) instead of degrading gracefully.
		normal_channel = geom.vertex_buffer.channels.get("Normal", [])
		local_normals = numpy.zeros_like(local_positions)
		if normal_channel:
			normal_array = numpy.array(normal_channel, dtype=numpy.float32)
			n = min(len(normal_array), len(local_normals))
			local_normals[:n] = normal_array[:n]
		matrix_ids = numpy.array([w[0] for w in geom.skin_weights], dtype=numpy.int64)
		weights = numpy.array([w[1] for w in geom.skin_weights], dtype=numpy.float32)

		positions, normals = _numpy_skin_batch(local_positions, local_normals, matrix_ids, weights, bone_skin_matrices)
		source_buffer = dataclasses.replace(geom.vertex_buffer, channels={
			**geom.vertex_buffer.channels,
			"Position": [tuple(p) for p in positions],
			"Normal": [tuple(n) for n in normals],
		})
	else:
		source_buffer = geom.vertex_buffer

	vertex_buffer = _resolve_lod_geomorphs(source_buffer, lod)
	for rdr_pass in lod.rdr_passes:
		yield vertex_buffer, rdr_pass.material_id, rdr_pass.indices


def iter_render_passes(shape_value, skeleton=None, bone_world_matrices=None):
	"""Yields (vertex_buffer, material_id, indices) for the renderable
	geometry of a CMesh/CMeshMRM/CMeshMultiLod(slot 0)/CMeshMRMSkinned shape
	value. `skeleton`/`bone_world_matrices` drive skinning for CMeshMRMSkinned
	(_passes_from_mrm_skinned_geom()) and, when a CMeshMRM's own geom.skinned
	is True, for that classic format's own skin data too (_passes_from_mrm_geom(),
	see pynel.ryzom_skin.skin_mesh_mrm_geom()) -- without them (no skeleton
	loaded yet by the caller), either kind still renders, at its raw bind-pose
	local vertices."""
	if isinstance(shape_value, Mesh):
		yield from _passes_from_mesh_geom(shape_value.geom)
	elif isinstance(shape_value, MeshMRM):
		yield from _passes_from_mrm_geom(shape_value.geom, skeleton, bone_world_matrices)
	elif isinstance(shape_value, MeshMultiLod) and shape_value.slots:
		slot_geom = shape_value.slots[0].mesh_geom
		if isinstance(slot_geom, MeshGeom):
			yield from _passes_from_mesh_geom(slot_geom)
		elif isinstance(slot_geom, MeshMRMGeom):
			yield from _passes_from_mrm_geom(slot_geom, skeleton, bone_world_matrices)
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


def boundary_edges(indices):
	"""Edges belonging to exactly one triangle in `indices` -- the open border
	of the mesh those triangles form."""
	edge_count = {}
	for i in range(0, len(indices), 3):
		a, b, c = indices[i], indices[i + 1], indices[i + 2]
		for u, v in ((a, b), (b, c), (c, a)):
			key = (u, v) if u < v else (v, u)
			edge_count[key] = edge_count.get(key, 0) + 1
	return [edge for edge, count in edge_count.items() if count == 1]


def boundary_loops(indices):
	"""Groups boundary_edges(indices) into connected components (closed loops
	or open chains). Returns a list of vertex-index sets, one per component."""
	edges = boundary_edges(indices)
	adjacency = collections.defaultdict(set)
	for a, b in edges:
		adjacency[a].add(b)
		adjacency[b].add(a)
	seen = set()
	loops = []
	for start in adjacency:
		if start in seen:
			continue
		stack = [start]
		component = set()
		while stack:
			vertex = stack.pop()
			if vertex in component:
				continue
			component.add(vertex)
			stack.extend(adjacency[vertex] - component)
		seen |= component
		loops.append(component)
	return loops


_FACE_WELD_PRECISION = 4  # decimals -- matches the ~1e-4 exact-coincidence threshold used throughout


def build_face_vertex_index(face_positions):
	"""Maps each of `face_positions` (rounded to _FACE_WELD_PRECISION
	decimals) to its vertex index. Build this ONCE per face shape and reuse
	it across every hairstyle of that race/gender -- main_seam_loop() needs
	it to tell which boundary loop is actually welded onto the face, and
	rescanning all ~2500 face vertices for every hairstyle would be wasted
	repeat work."""
	return {tuple(round(c, _FACE_WELD_PRECISION) for c in p): i for i, p in enumerate(face_positions)}


def _is_welded(position, face_index):
	return tuple(round(c, _FACE_WELD_PRECISION) for c in position) in face_index


def main_seam_loop(positions, indices, face_index):
	"""Returns the boundary_loops(indices) component with the most vertices
	welded onto the face mesh described by `face_index` (see
	build_face_vertex_index()) -- i.e. the loop actually soldered to the
	scalp, as opposed to picking "the largest loop" (an earlier heuristic
	that's wrong in general: a hairstyle built from several independent
	hair-strand cards has multiple boundary loops, and the largest one isn't
	necessarily the one touching the face -- seen on
	fy_hof_cheveux_basic02.shape, where two 30-vertex strand loops outrank
	the real 26-vertex face seam). None if the mesh has no boundary, or none
	of its loops has any vertex welded to the face."""
	loops = boundary_loops(indices)
	if not loops:
		return None
	weld_counts = [sum(1 for i in loop if _is_welded(positions[i], face_index)) for loop in loops]
	best_index = max(range(len(loops)), key=lambda i: weld_counts[i])
	return loops[best_index] if weld_counts[best_index] > 0 else None


def seam_ring_by_angle(positions, indices, face_index):
	"""Returns main_seam_loop(positions, indices, face_index) as an ordered
	list of (angle_deg, position) pairs, sorted by angle around the loop's
	own vertical-axis centroid (x,y) -- the ordering used as the target
	curve to interpolate an arbitrary angle's position against (see the
	projection step that consumes this). Empty list if no loop is welded to
	the face."""
	loop = main_seam_loop(positions, indices, face_index)
	if not loop:
		return []
	points = [positions[i] for i in loop]
	cx = sum(p[0] for p in points) / len(points)
	cy = sum(p[1] for p in points) / len(points)
	ring = sorted(
		((math.degrees(math.atan2(p[1] - cy, p[0] - cx)), p) for p in points),
		key=lambda item: item[0],
	)
	return ring


def seam_loop_by_angle_indexed(positions, indices, face_index):
	"""Like seam_ring_by_angle(), but keyed by original vertex index instead
	of position -- used to know exactly which vertex of the source mesh's
	own vertex buffer to move when conforming its boundary onto another
	shape's seam ring (see conform_hairstyle_boundary())."""
	loop = main_seam_loop(positions, indices, face_index)
	if not loop:
		return []
	points = [positions[i] for i in loop]
	cx = sum(p[0] for p in points) / len(points)
	cy = sum(p[1] for p in points) / len(points)
	return sorted(
		((math.degrees(math.atan2(positions[i][1] - cy, positions[i][0] - cx)), i) for i in loop),
		key=lambda item: item[0],
	)


def interpolate_seam_ring(ring, angle_deg):
	"""Returns the position on `ring` (as returned by seam_ring_by_angle(),
	sorted ascending by angle) at `angle_deg`, linearly interpolating
	between the two ring points bracketing it -- wrapping around the +-180
	degree seam, since the ring covers a full loop around the head."""
	angles = [a for a, _ in ring]
	q = angle_deg
	while q < angles[0]:
		q += 360.0
	while q >= angles[0] + 360.0:
		q -= 360.0
	extended = list(ring) + [(angles[0] + 360.0, ring[0][1])]
	for (a0, p0), (a1, p1) in zip(extended, extended[1:]):
		if a0 <= q <= a1:
			t = 0.0 if a1 == a0 else (q - a0) / (a1 - a0)
			return tuple(p0[k] + t * (p1[k] - p0[k]) for k in range(3))
	return ring[-1][1]  # unreachable in practice, kept as a safe fallback


def conform_hairstyle_boundary(positions, indices, source_face_index, target_ring):
	"""Returns a new list of positions (same length/order as `positions`)
	for a hairstyle mesh whose main seam loop (see main_seam_loop(), found
	here using `source_face_index` -- the *source* race/gender's own face,
	see build_face_vertex_index()) should instead fit another race/gender's
	head -- as described by `target_ring` (a seam_ring_by_angle() result
	computed on that *target* race's own baseline hairstyle, e.g.
	tr_hof_cheveux_shave01, against the target race's own face).

	Two operations, per the user's own manual workflow:
	  1. Rigid pre-alignment: the whole mesh is translated by the difference
	     between `target_ring`'s centroid and the source seam loop's own
	     centroid (translation only, no rotation) -- brings the two seams as
	     close as possible before any per-vertex change, avoiding an
	     over-stretched/sunken result.
	  2. Boundary snap: each (now pre-aligned) boundary vertex is placed
	     exactly on `target_ring`, interpolated at that vertex's own angle --
	     guaranteeing no seam gap against the target head.

	Non-boundary vertices only receive the rigid translation from step 1 --
	restyling the interior for the new head shape is intentionally left to
	manual work in a 3D tool, every hairstyle differs too much for a general
	algorithm to do that part."""
	loop_angles = seam_loop_by_angle_indexed(positions, indices, source_face_index)
	if not loop_angles or not target_ring:
		return list(positions)

	source_points = [positions[i] for _, i in loop_angles]
	source_centroid = tuple(sum(p[k] for p in source_points) / len(source_points) for k in range(3))
	target_centroid = tuple(sum(p[k] for _, p in target_ring) / len(target_ring) for k in range(3))
	translation = tuple(target_centroid[k] - source_centroid[k] for k in range(3))

	new_positions = [tuple(p[k] + translation[k] for k in range(3)) for p in positions]
	for angle, vertex_index in loop_angles:
		new_positions[vertex_index] = interpolate_seam_ring(target_ring, angle)

	return new_positions


# Content-type-agnostic default LOD for rebuild_shadow_skin() when there's no
# existing CShadowSkin to infer a target from (see
# infer_shadow_skin_lod_index()) -- nothing in a .shape records whether it's
# a creature or a character piece, and real data (verified 2026-09-02/03,
# see forgery-object-editor.md's chantier writeup) shows the two use
# genuinely different (ratio, maxFace) conventions: character pieces always
# land on LOD index 3 regardless of face count (423/423 in
# characters_shapes.bnp), while creatures cluster their chosen LOD's actual
# triangle count around a hard ~1000-triangle cap (r=-0.88 vs. face count,
# r=-0.18 vs. physical bbox size, across fauna_shapes.bnp) whenever their
# finest LOD exceeds it.
_SHADOW_SKIN_DEFAULT_TARGET_TRIANGLES = 1000
_SHADOW_SKIN_DEFAULT_FALLBACK_LOD_INDEX = 3


def default_shadow_skin_lod_index(geom: MeshMRMSkinnedGeom) -> int:
	"""Pick the coarsest LOD with >=_SHADOW_SKIN_DEFAULT_TARGET_TRIANGLES
	triangles if one exists; otherwise fall back to
	_SHADOW_SKIN_DEFAULT_FALLBACK_LOD_INDEX. Reproduces both real-data
	regimes described above without ever needing to classify the shape's
	content type -- it's derived purely from the shape's own LOD triangle
	counts."""
	candidates = [i for i, lod in enumerate(geom.lods) if lod.num_triangles >= _SHADOW_SKIN_DEFAULT_TARGET_TRIANGLES]
	if candidates:
		return min(candidates, key=lambda i: geom.lods[i].num_triangles)
	return min(_SHADOW_SKIN_DEFAULT_FALLBACK_LOD_INDEX, len(geom.lods) - 1)


def infer_shadow_skin_lod_index(geom: MeshMRMSkinnedGeom) -> int:
	"""Which of geom.lods an existing (possibly stale) CShadowSkin was built
	from, recovered by matching its Triangles count against each LOD's own
	total render-pass index count -- build_shadow_skin never re-triangulates,
	it reuses a LOD's own triangle list as-is, just reindexed onto a
	deduplicated vertex set (see rebuild_shadow_skin()), so the index count
	is preserved exactly. Falls back to default_shadow_skin_lod_index() if
	CShadowSkin is empty, or its Triangles count doesn't match exactly one
	LOD (ambiguous or untracked origin)."""
	shadow_skin = geom.shadow_skin
	if shadow_skin.vertices:
		target = len(shadow_skin.triangles)
		matches = [i for i, lod in enumerate(geom.lods) if sum(len(rp.indices) for rp in lod.rdr_passes) == target]
		if len(matches) == 1:
			return matches[0]
	return default_shadow_skin_lod_index(geom)


def _dominant_bone(packed_vertex):
	"""The bone (matrix id) with the highest of the vertex's up to 4 skin
	weights -- CShadowVertex uses one-bone rigid skinning, see ShadowSkin's
	own docstring. Mirrors addShadowMesh()'s own max-weight scan
	(nel/tools/3d/build_shadow_skin/main.cpp)."""
	return max(zip(packed_vertex.matrices, packed_vertex.weights), key=lambda mw: mw[1])[0]


def rebuild_shadow_skin(geom: MeshMRMSkinnedGeom, lod_index: int) -> ShadowSkin:
	"""Rebuild geom's CShadowSkin from one of its own existing LODs (index
	`lod_index` -- see infer_shadow_skin_lod_index()/
	default_shadow_skin_lod_index() to pick one), mirroring
	nel/tools/3d/build_shadow_skin/main.cpp's addShadowMesh() in pure
	Python: geometric decimation itself is never redone here (or by the
	official tool) -- a LOD's own already-baked vertex/triangle set is
	simply deduplicated by (position, dominant bone) and its triangle
	indices reindexed onto that deduplicated set. The official tool's final
	stripify pass (vertex-cache render order) is skipped -- pure GPU perf,
	irrelevant to correctness.

	Geomorph placeholder indices (raw index i < len(lod.geomorphs), see
	_resolve_lod_geomorphs()'s own docstring -- this applies to
	MeshMRMSkinnedLod.geomorphs same as MrmLod.geomorphs) are resolved to
	their "end" target's real position/bone before dedup, exactly like
	addShadowMesh()'s own vertexUsed[] resolution -- found and fixed
	2026-09-03 via a real fixture mismatch (fy_hof_cheveux_medium01.shape's
	LOD 3 rebuilt to 72 vertices instead of the original's 84: several of
	its early, placeholder-only indices were being read with their own
	empty CWedge()-equivalent data -- all bone 0/position 0 -- instead of
	being resolved first, silently over-merging them together)."""
	lod = geom.lods[lod_index]
	positions = [pv.decompact_pos(geom.decompact_scale) for pv in geom.packed_vertices]
	dominant_bones = [_dominant_bone(pv) for pv in geom.packed_vertices]
	geomorph_ends = [end for _start, end in lod.geomorphs]

	def resolve(raw_index):
		return geomorph_ends[raw_index] if raw_index < len(geomorph_ends) else raw_index

	raw_indices = [idx for rp in lod.rdr_passes for idx in rp.indices]
	used_indices = sorted({resolve(idx) for idx in raw_indices})

	shadow_vertices = []
	shadow_index_by_key = {}
	remap = {}
	for i in used_indices:
		key = (positions[i], dominant_bones[i])
		shadow_index = shadow_index_by_key.get(key)
		if shadow_index is None:
			shadow_index = len(shadow_vertices)
			shadow_vertices.append(ShadowVertex(position=Vector3(*positions[i]), matrix_id=dominant_bones[i]))
			shadow_index_by_key[key] = shadow_index
		remap[i] = shadow_index

	triangles = [remap[resolve(idx)] for idx in raw_indices]
	return ShadowSkin(vertices=shadow_vertices, triangles=triangles)


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
