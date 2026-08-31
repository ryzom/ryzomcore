#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""CPU linear-blend skinning for CMeshMRMSkinned (nel/src/3d/mesh_mrm_skinned.cpp)
and classic CMeshMRM's own, separate skin data (nel/src/3d/mesh_mrm_skin.cpp,
see skin_mesh_mrm_geom() below) -- two different on-disk formats for the same
underlying idea (a CMeshMRM can itself be skinned, independently of whether
it's the newer CMeshMRMSkinned class).

Resolves each vertex's final position/normal from up to 4 weighted bone
influences, given already-evaluated bone world matrices -- typically from
`pynel.ryzom_animation.evaluate_bone_world_matrix()`, one call per bone that
influences the mesh, done once per frame by the caller. Not coupled to
`ryzom_animation` itself (callers just pass in a name -> matrix mapping), and
has no Panda3D/rendering dependency -- callers convert the returned per-vertex
`SkinnedVertex` list into whatever vertex-buffer form they need.

Per-vertex skin formula, replicated from `CMeshMRMSkinnedGeom::computeBonesId()`
(mesh_mrm_skinned.cpp:1163-1240) and `CBone::compute()` (bone.cpp:228-241):
a `PackedVertex.matrices[i]` is a direct index into the mesh's own
`bones_name` list (confirmed against the engine's own remap step, which
range-checks it against exactly that list's length before ever touching the
skeleton), and each vertex's final position is the weighted sum (weights read
straight as `weight/255`, no renormalization -- matches well-formed exported
content, where they already sum to 1) of `BoneSkinMatrix_i(localPos)` for each
non-zero-weight influence, where
`BoneSkinMatrix = LocalSkeletonMatrix(bone) * InvBindPos(bone)`.

The matrix math here (`_mat_*`) duplicates `ryzom_animation.py`'s private
helpers rather than importing them -- same `NLMISC::CMatrix` row-major
convention, kept self-contained per this package's usual per-format module
convention (each module owns its own private helpers rather than importing
another module's).
"""

from dataclasses import dataclass
from math import sqrt
from typing import Dict, List, Optional

from pynel.ryzom_shape import Matrix, MeshMRMGeom, MeshMRMSkinnedGeom, PackedVertex, SkeletonShape, Vector3


def _mat_identity() -> tuple:
	return (
		(1.0, 0.0, 0.0, 0.0),
		(0.0, 1.0, 0.0, 0.0),
		(0.0, 0.0, 1.0, 0.0),
		(0.0, 0.0, 0.0, 1.0),
	)


def _mat_mul(a: tuple, b: tuple) -> tuple:
	return tuple(
		tuple(sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4))
		for i in range(4)
	)


def _mat_point(m: tuple, v: Vector3) -> Vector3:
	return Vector3(
		m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
		m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
		m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3],
	)


def _mat_vector(m: tuple, v: Vector3) -> Vector3:
	"""Like _mat_point(), but ignoring translation -- for normals."""
	return Vector3(
		m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
		m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
		m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z,
	)


def _matrix_field_to_dense(m: Optional[Matrix]) -> tuple:
	"""Converts a parsed ryzom_shape.Matrix (CMatrix's sparse on-disk
	encoding, see ryzom_shape.py's own Matrix docstring) to the dense 4x4
	form used here -- `scale`/`proj` aren't needed: `mulPoint()`/`mulVector()`
	(matrix.cpp) only ever use `rot`/`trans` directly, `Scale33` is metadata
	for CMatrix's own accessors, already baked into `rot`."""
	if m is None:
		return _mat_identity()
	rot = m.rot or (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
	trans = m.trans or (0.0, 0.0, 0.0)
	return (
		(rot[0], rot[1], rot[2], trans[0]),
		(rot[3], rot[4], rot[5], trans[1]),
		(rot[6], rot[7], rot[8], trans[2]),
		(0.0, 0.0, 0.0, 1.0),
	)


@dataclass
class SkinnedVertex:
	pos: Vector3
	normal: Vector3
	uv: tuple


def bone_skin_matrices_for_mesh(geom: MeshMRMSkinnedGeom, skeleton: SkeletonShape,
                                 bone_world_matrices: Dict[str, tuple]) -> List[tuple]:
	"""One BoneSkinMatrix per `geom.bones_name` entry (same indices as
	`PackedVertex.matrices`) -- `LocalSkeletonMatrix * InvBindPos`
	(bone.cpp:228-241). `bone_world_matrices` supplies the first term, already
	evaluated by the caller (e.g. via ryzom_animation.evaluate_bone_world_matrix(),
	one call per bone actually used here); identity for any bone missing from
	it or absent from `skeleton.bone_map`."""
	matrices = []
	for name in geom.bones_name:
		world = bone_world_matrices.get(name, _mat_identity())
		bone_index = skeleton.bone_map.get(name)
		inv_bind = _matrix_field_to_dense(
			skeleton.bones[bone_index].inv_bind_pos) if bone_index is not None else _mat_identity()
		matrices.append(_mat_mul(world, inv_bind))
	return matrices


def skin_vertex(vertex: PackedVertex, decompact_scale: float, bone_skin_matrices: List[tuple]) -> SkinnedVertex:
	local_pos = Vector3(*vertex.decompact_pos(decompact_scale))
	local_normal = Vector3(*vertex.decompact_normal())

	pos = Vector3(0.0, 0.0, 0.0)
	normal = Vector3(0.0, 0.0, 0.0)
	any_weight = False
	for slot in range(4):
		weight = vertex.weights[slot]
		if weight == 0 and slot > 0:
			continue
		any_weight = any_weight or weight > 0
		m = bone_skin_matrices[vertex.matrices[slot]]
		w = weight / 255.0
		p = _mat_point(m, local_pos)
		n = _mat_vector(m, local_normal)
		pos = Vector3(pos.x + p.x * w, pos.y + p.y * w, pos.z + p.z * w)
		normal = Vector3(normal.x + n.x * w, normal.y + n.y * w, normal.z + n.z * w)

	if not any_weight:
		# No real influence at all (shouldn't happen for well-formed content) --
		# fall back to the raw local position/normal rather than an all-zero point.
		return SkinnedVertex(pos=local_pos, normal=local_normal, uv=vertex.decompact_uv())

	length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z)
	if length > 0:
		normal = Vector3(normal.x / length, normal.y / length, normal.z / length)
	return SkinnedVertex(pos=pos, normal=normal, uv=vertex.decompact_uv())


def skin_mesh(geom: MeshMRMSkinnedGeom, skeleton: SkeletonShape,
              bone_world_matrices: Dict[str, tuple]) -> List[SkinnedVertex]:
	"""Skins every vertex in `geom.packed_vertices` (all LODs index into this
	same shared vertex pool -- only `rdr_passes`/`geomorphs` differ per LOD),
	returning one SkinnedVertex per input vertex, same order."""
	bone_skin_matrices = bone_skin_matrices_for_mesh(geom, skeleton, bone_world_matrices)
	return [skin_vertex(v, geom.decompact_scale, bone_skin_matrices) for v in geom.packed_vertices]


def skin_mesh_mrm_geom(geom: MeshMRMGeom, skeleton: SkeletonShape,
                        bone_world_matrices: Dict[str, tuple]) -> List[SkinnedVertex]:
	"""Skins a classic CMeshMRM's own (non-CMeshMRMSkinned) skin data --
	confirmed real, 2026-08-30 (e.g. Ryzom's *_visage.shape face pieces:
	`CMeshMRMGeom.skinned=True`, `bones_name` populated, `skin_weights` set,
	despite being a plain CMeshMRM, not CMeshMRMSkinned -- was silently
	rendered rigid before this, since Forgery's is_skinned check only
	tested for the CMeshMRMSkinned class).

	Different on-disk storage than CMeshMRMSkinned's packed_vertices pool:
	`geom.vertex_buffer`'s Position/Normal channels hold plain (already-
	decompacted) per-vertex values, and `geom.skin_weights[i]` is a parallel
	`(matrix_id: 4-tuple of int, weight: 4-tuple of float)` for that same
	vertex index `i` -- weights here are already-normalized floats used
	directly (`CMeshMRMGeom::applySkin()`, mesh_mrm_skin.cpp:118-136:
	`boneMat3x4[...].mulSetPoint(vertex, Weights[k], dst)`, no /255 scaling
	unlike PackedVertex's byte-compressed weights in skin_vertex() above).
	`matrix_id` indexes the same `geom.bones_name` list either way, so
	bone_skin_matrices_for_mesh() (which only reads `geom.bones_name`) is
	reused as-is. Returns one SkinnedVertex per vertex_buffer vertex, same
	order/index space as the input channels (safe to feed straight into
	shape_geometry.py's _resolve_lod_geomorphs(), same as the input buffer
	it replaces)."""
	bone_skin_matrices = bone_skin_matrices_for_mesh(geom, skeleton, bone_world_matrices)
	positions = geom.vertex_buffer.channels.get("Position", [])
	normals = geom.vertex_buffer.channels.get("Normal", [])
	uvs = geom.vertex_buffer.channels.get("TexCoord0", [])

	result = []
	for i, (matrix_ids, weights) in enumerate(geom.skin_weights):
		local_pos = Vector3(*positions[i])
		local_normal = Vector3(*normals[i]) if i < len(normals) else Vector3(0.0, 0.0, 0.0)

		pos = Vector3(0.0, 0.0, 0.0)
		normal = Vector3(0.0, 0.0, 0.0)
		any_weight = False
		for slot in range(4):
			weight = weights[slot]
			if weight == 0 and slot > 0:
				continue
			any_weight = any_weight or weight > 0
			m = bone_skin_matrices[matrix_ids[slot]]
			p = _mat_point(m, local_pos)
			n = _mat_vector(m, local_normal)
			pos = Vector3(pos.x + p.x * weight, pos.y + p.y * weight, pos.z + p.z * weight)
			normal = Vector3(normal.x + n.x * weight, normal.y + n.y * weight, normal.z + n.z * weight)

		if not any_weight:
			pos, normal = local_pos, local_normal
		else:
			length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z)
			if length > 0:
				normal = Vector3(normal.x / length, normal.y / length, normal.z / length)

		uv = uvs[i] if i < len(uvs) else (0.0, 0.0)
		result.append(SkinnedVertex(pos=pos, normal=normal, uv=uv))

	return result
