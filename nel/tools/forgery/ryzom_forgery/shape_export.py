"""Exports a parsed .shape (CMesh/CMeshMRM/CMeshMultiLod) to plain
interchange formats.

.stl is hand-written directly from the parsed vertex buffers/indices --
simple enough a text format that a small dependency-free writer beats
pulling in a mesh library, and it has no material/texture support at all
(it's geometry only), so there's nothing `assimp_py` would add over the
hand-written path.

.obj, .dae (COLLADA), .fbx and .gltf/.glb all go through `assimp_py`'s own
writer (`export_file()`, see project-todos/assimp_py/bones_animations.md,
chantier closed 2026-09-04) instead of `pycollada`/`pygltflib`/a hand-written
.obj+.mtl writer -- a single Scene/Mesh/Bone/Node model shared with the read
side, letting a skinned shape's skeleton/bone weights be embedded directly
(see project-todos/forgery/mesh_skin_export.md), which pycollada/pygltflib
had no way to express, and avoiding axis/UV convention bugs having to be
fixed twice (see mesh_skin_export.md's "migrer _export_obj" chantier).
`pycollada`/`pygltflib` are no longer a dependency of this module.
"""

import collections
import copy
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Dict, List, Optional

from ryzom_forgery.settings import TEXTURE_MODE_COPY_PNG
from ryzom_forgery.shape_geometry import (
	IDENTITY_QUAT, UNIT_VECTOR, ZERO_VECTOR, bake_default_transform_into_geom, iter_render_passes,
	load_panda_texture, rgba_to_color, shape_default_transform, shape_geom, texture_to_pnm_image,
)


@dataclass
class ExportFormat:
	extension: str
	label: str
	supports_materials: bool
	# (shape_value, materials, output_path, texture_mode, texture_finder) -> list[Path] of files written
	export: Callable


def _resolve_material_texture(texture_finder, material, output_dir: Path, texture_mode: str, texture_cache: dict):
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

	panda_texture = load_panda_texture(source_name, finder=texture_finder)
	if panda_texture is None:
		texture_cache[png_name] = False
		return None

	image = texture_to_pnm_image(panda_texture)
	image.write(str(output_dir / png_name))
	texture_cache[png_name] = True
	return png_name


def _triangle_normal(a, b, c):
	ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
	vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
	nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
	length = (nx * nx + ny * ny + nz * nz) ** 0.5
	if length == 0.0:
		return (0.0, 0.0, 0.0)
	return (nx / length, ny / length, nz / length)


def _export_stl(shape_value, materials, output_path: Path, texture_mode: str, texture_finder) -> List[Path]:
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


def _zup_to_yup(v):
	"""Ryzom's own Z-up (X=right, Y=forward, Z=up) into the Y-up convention
	.fbx/.gltf mandate and .dae defaults to (x=right, y=up, z=forward):
	(X, Y, Z) -> (X, Z, -Y). Only applied to a RIGID (no skeleton) export --
	see _assimp_scene_from_shape()'s own docstring for why a skinned export
	stays in raw Ryzom space for now."""
	x, y, z = v
	return (x, z, -y)


_IDENTITY_MATRIX4 = ((1.0, 0.0, 0.0, 0.0), (0.0, 1.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, 0.0, 0.0, 1.0))


def _skin_state_for_shape(shape_value, skeleton):
	"""Dispatches to whichever of skin_state_helpers.py's three
	_build_*_skin_state() builders matches shape_value's own mesh type,
	returning the same static per-vertex tables the live Skinning preview
	precomputes (bone_names / local_positions / local_normals / a per-vertex
	(N,4) bone-index array / a per-vertex (N,4) weight array /
	inv_bind_matrices) -- reused here as-is rather than duplicated, since a
	one-shot export needs exactly the same tables a live re-skin does, just
	evaluated once instead of every frame. None if shape_value has no geom,
	isn't actually skinned, or skin_state_helpers itself returns None (e.g.
	no lods).

	Row alignment with the Position/Normal/TexCoord0 channels
	_assimp_mesh_from_geometry() reads via iter_render_passes() (see
	_assimp_scene_from_shape()) is guaranteed for all three mesh types:
	geomorph placeholder wedges (MeshMRM/MeshMRMSkinned) only ever change a
	row's VALUE, never the row count or index space, so the skin tables
	built here (bone-index/weight only, never geomorph-resolved) stay valid
	against iter_render_passes()'s own geomorph-resolved geometry."""
	from pynel.ryzom_shape import Mesh, MeshMRM, MeshMRMSkinned
	from ryzom_forgery.apps.object_editor_mixins.skin_state_helpers import (
		_build_mesh_skin_state, _build_mrm_skin_state, _build_skin_state,
	)

	geom = shape_geom(shape_value)
	if geom is None:
		return None
	if isinstance(shape_value, MeshMRMSkinned):
		return _build_skin_state(geom, skeleton)
	if isinstance(shape_value, MeshMRM):
		return _build_mrm_skin_state(geom, skeleton)
	if isinstance(shape_value, Mesh):
		return _build_mesh_skin_state(geom, skeleton)
	return None


def _skin_bones_for_export(bone_names, bone_indices, weights, inv_bind_matrices):
	"""Transposes the per-vertex skin tables above (top-4 (bone index,
	weight) pairs per vertex, the pynel/Ryzom convention) into the per-bone
	(vertex_ids, weights) lists `assimp_py.Bone` expects (the opposite
	convention) -- one `assimp_py.Bone` per bone actually referenced by at
	least one vertex with a nonzero weight; a bone in `bone_names` never
	used by any vertex is simply omitted from the returned list (it can
	still exist as a Node in the skeleton hierarchy, see
	_assimp_skeleton_root_node()). `Bone.offset_matrix` is
	`inv_bind_matrices[bone_index]` unchanged -- that's exactly
	CBone.inv_bind_pos, already the mesh-space -> bone-space matrix
	assimp_py.Bone.offset_matrix documents."""
	import assimp_py

	per_bone: dict = {}
	num_vertices = len(bone_indices)
	for vertex_id in range(num_vertices):
		for slot in range(4):
			weight = float(weights[vertex_id][slot])
			if weight <= 0.0:
				continue
			bone_index = int(bone_indices[vertex_id][slot])
			ids, ws = per_bone.setdefault(bone_index, ([], []))
			ids.append(vertex_id)
			ws.append(weight)

	bones = []
	for bone_index in sorted(per_bone):
		vertex_ids, bone_weights = per_bone[bone_index]
		offset_matrix = tuple(tuple(float(x) for x in row) for row in inv_bind_matrices[bone_index])
		bones.append(assimp_py.Bone(bone_names[bone_index], offset_matrix, vertex_ids, bone_weights))
	return bones


def _assimp_mesh_from_geometry(name, vertex_buffer, indices, material_index, bones=None, convert_to_yup=True):
	"""Builds one `assimp_py.Mesh` from a single iter_render_passes() pass
	-- one assimp Mesh per pass (a pass never mixes materials, matching
	aiMesh's own one-material-per-mesh convention). `bones`, if given, is
	attached unchanged (see _skin_bones_for_export()) -- its `vertex_ids`
	index into the SAME Position/Normal arrays built here, since both are
	read from the exact same row-aligned source (see
	_skin_state_for_shape()'s own docstring)."""
	import assimp_py

	positions = vertex_buffer.channels.get("Position") or []
	normals = vertex_buffer.channels.get("Normal")
	texcoords = vertex_buffer.channels.get("TexCoord0")

	if convert_to_yup:
		positions = [_zup_to_yup(p) for p in positions]
		if normals:
			normals = [_zup_to_yup(n) for n in normals]

	flat_vertices = [c for p in positions for c in p]
	flat_normals = [c for n in normals for c in n] if normals else None
	texcoords_arg = None
	num_uv_components = None
	if texcoords:
		# Inverse of shape_import.py's own import-time flip (see its
		# build_mesh() docstring note): a real .shape's TexCoord0 is stored
		# with NeL's own V=0-at-top convention, but assimp-py's exporters
		# (.dae/.fbx/.gltf alike) expect the OpenGL-style V=0-at-bottom
		# convention their own importers read verbatim -- without flipping
		# back here, every exported UV came out vertically mirrored (found
		# 2026-09-05, Nuno: "uv pas bon, il y a un miroir", exporting to
		# .gltf and checking in Blender).
		texcoords_arg = [[c for u, v in texcoords for c in (u, 1.0 - v)]]
		num_uv_components = [2]

	return assimp_py.Mesh(
		name, flat_vertices, list(indices),
		normals=flat_normals, tangents=None, bitangents=None,
		colors=None, texcoords=texcoords_arg, num_uv_components=num_uv_components,
		bones=bones, material_index=material_index,
	)


def _assimp_material_dict(materials, material_id, output_dir: Path, texture_mode: str, texture_finder, texture_cache: dict) -> dict:
	"""Builds the material dict `assimp_py.export_file()` expects (see
	assimp_py's dict_to_ai_material()/material_key_map -- NAME/COLOR_*/
	OPACITY/SHININESS/TEXTURES[DIFFUSE], everything _export_dae()/
	_export_gltf_or_glb() used to set by hand via pycollada/pygltflib)."""
	import assimp_py

	material = materials[material_id] if materials and material_id < len(materials) else None
	if material is None:
		return {"NAME": f"material_{material_id}", "COLOR_DIFFUSE": [0.8, 0.8, 0.8], "OPACITY": 1.0}

	diffuse = rgba_to_color(material.diffuse)
	mat_dict = {
		"NAME": f"material_{material_id}",
		"COLOR_DIFFUSE": list(diffuse[:3]),
		"OPACITY": diffuse[3],
		"COLOR_AMBIENT": list(rgba_to_color(material.ambient)[:3]),
		"COLOR_SPECULAR": list(rgba_to_color(material.specular)[:3]),
		"COLOR_EMISSIVE": list(rgba_to_color(material.emissive)[:3]),
		"SHININESS": material.shininess,
	}
	texture_name = _resolve_material_texture(texture_finder, material, output_dir, texture_mode, texture_cache)
	if texture_name:
		mat_dict["TEXTURES"] = {int(assimp_py.TextureType_DIFFUSE): [texture_name]}
	return mat_dict


def _assimp_skeleton_root_node(skeleton):
	"""Builds an assimp_py.Node hierarchy mirroring `skeleton`'s bone tree
	(father_id), one Node per bone, wrapped under a single "Armature" root
	Node -- assimp locates a skinned mesh's bones purely by matching
	Bone.name against a Node.name anywhere in the scene graph (confirmed in
	assimp_py's build_ai_bone(): bone->mNode/mArmature are left NULL, never
	set by this binding), so this hierarchy only needs to exist, not be
	referenced back from the Bone objects themselves.

	Each bone's own Node.transformation is its LOCAL (parent-relative) bind
	pose matrix, reusing pynel.ryzom_animation._bone_local_matrix() -- the
	same per-bone local-matrix computation evaluate_all_bone_world_matrices()
	composes recursively (pivot/rotation/scale, plus CBone::compute()'s
	root-bone InvBindPos fallback) -- not recomputed here.

	Known limitation: CBone::compute()'s UnheritScale compensation (see
	ryzom_animation._unherit_scale_comp()) inserts an extra corrective
	matrix between a father and an unherit_scale=True child when composing
	WORLD matrices -- a single local Node.transformation per bone can't
	represent that. Every bone a pynel-generated .skel carries
	(shape_import.py's extract_skeleton(), see project-todos/forgery/
	skel_export.md) always has unherit_scale=False, so this doesn't affect
	that path -- only a real 3dsMax-authored .skel (Biped rigs) with
	unherit_scale=True bones would export with a subtly wrong bind pose
	here."""
	import assimp_py
	from pynel.ryzom_animation import _bone_local_matrix

	children_by_father = collections.defaultdict(list)
	for index, bone in enumerate(skeleton.bones):
		father = bone.father_id if bone.father_id is not None and bone.father_id >= 0 else -1
		children_by_father[father].append(index)

	def build_node(index):
		bone = skeleton.bones[index]
		local_matrix, _scale = _bone_local_matrix(bone, None, 0.0)
		children = [build_node(child_index) for child_index in children_by_father.get(index, [])]
		return assimp_py.Node(bone.name, local_matrix, None, children, [])

	root_children = [build_node(index) for index in children_by_father.get(-1, [])]
	return assimp_py.Node("Armature", _IDENTITY_MATRIX4, None, root_children, [])


_ANIM_SAMPLE_RATE = 30.0  # matches project-todos/pynel/anim_write.md's own CAnimationOptimizer-derived constant


def _assimp_animation_from_pynel(anim, skeleton, sample_rate: float = _ANIM_SAMPLE_RATE):
	"""Builds one `assimp_py.Animation` from a pynel `Animation` (already
	loaded/parsed, e.g. Skinning preview's own self._bone_preview_animation)
	-- one `assimp_py.NodeAnimTrack` per bone that has at least one of its
	own `.pos`/`.rotquat`/`.scale` tracks in `anim.id_by_name` (a bone with
	none gets no track at all, same "missing channel" convention already
	used by pynel.ryzom_animation.build_animation() on the write side, see
	project-todos/pynel/anim_write.md). Each channel is resampled at a fixed
	`sample_rate` (Hz) via `evaluate_track()` -- assimp_py's own read side
	already hands us pre-sampled/pre-interpolated tracks the same way
	regardless of the source .anim's own on-disk format (Sampled vs
	KeyFramerLinear vs Bezier/TCB), so re-sampling here for WRITING is the
	symmetric, already-proven approach rather than trying to preserve each
	track's original (and possibly irregular) key times.

	`Animation.duration`/`ticks_per_second`: ticks_per_second is set to
	`sample_rate` itself, so tick i is exactly sample i and duration is the
	sample count minus one -- duration/ticks_per_second is then exactly the
	clip's real length in seconds, matching aiAnimation's own convention."""
	import assimp_py
	from pynel.ryzom_animation import animation_duration, evaluate_track

	duration_seconds = animation_duration(anim)
	num_samples = max(2, round(duration_seconds * sample_rate) + 1)
	sample_times = [i / sample_rate for i in range(num_samples)]
	tick_times = [float(i) for i in range(num_samples)]

	channels = []
	for bone in skeleton.bones:
		pos_idx = anim.id_by_name.get(f"{bone.name}.pos")
		rot_idx = anim.id_by_name.get(f"{bone.name}.rotquat")
		scale_idx = anim.id_by_name.get(f"{bone.name}.scale")
		has_pos = pos_idx is not None and anim.tracks[pos_idx] is not None
		has_rot = rot_idx is not None and anim.tracks[rot_idx] is not None
		has_scale = scale_idx is not None and anim.tracks[scale_idx] is not None
		if not (has_pos or has_rot or has_scale):
			continue

		position_keys = None
		if has_pos:
			track = anim.tracks[pos_idx]
			position_keys = [c for t in sample_times for v in (evaluate_track(track, t),) for c in (v.x, v.y, v.z)]
		rotation_keys = None
		if has_rot:
			track = anim.tracks[rot_idx]
			rotation_keys = [c for t in sample_times for v in (evaluate_track(track, t),) for c in (v.w, v.x, v.y, v.z)]
		scaling_keys = None
		if has_scale:
			track = anim.tracks[scale_idx]
			scaling_keys = [c for t in sample_times for v in (evaluate_track(track, t),) for c in (v.x, v.y, v.z)]

		channels.append(assimp_py.NodeAnimTrack(
			bone.name,
			position_times=tick_times if has_pos else None, position_keys=position_keys,
			rotation_times=tick_times if has_rot else None, rotation_keys=rotation_keys,
			scaling_times=tick_times if has_scale else None, scaling_keys=scaling_keys,
		))

	return assimp_py.Animation(anim.name or "anim", float(num_samples - 1), sample_rate, channels)


def _assimp_scene_from_shape(
		shape_value, materials, output_path: Path, texture_mode: str, texture_finder, skeleton=None, animation=None):
	"""Builds the full `assimp_py.Scene` for `shape_value`, geometry only
	(rigid) or geometry+skin+skeleton if `skeleton` is given (a
	pynel.ryzom_shape.SkeletonShape whose bone names match
	shape_value's own bones_name -- see _skin_state_for_shape()). One
	assimp Mesh per iter_render_passes() pass, referenced by a single
	"Meshes" Node; the skeleton, if any, is a sibling "Armature" Node (see
	_assimp_skeleton_root_node()) -- both under one RootNode, so
	RootNode itself carries no mesh_indices of its own (some exporters,
	e.g. Collada's, special-case a root node that directly owns meshes).

	Axis convention: a RIGID export (skeleton=None) converts every
	Position/Normal from Ryzom's own Z-up into the Y-up convention .fbx/
	.gltf mandate (FBXExporter.cpp hardcodes UpAxis=1/Y regardless of scene
	content; glTF has no up-axis metadata at all) -- same _zup_to_yup()
	already proven correct by the previous pygltflib-based .gltf exporter.
	A SKINNED export (skeleton given) is intentionally left in raw,
	unconverted Z-up for now: converting a skinned mesh's vertices to Y-up
	would also require converting every bone's offset_matrix and the whole
	skeleton hierarchy consistently, or the skin binding breaks outright
	(not just a wrong-looking orientation, but a wrong-shaped deformation)
	-- left as a follow-up once Nuno's own Blender check (mesh_skin_export.md
	Étape 7) confirms whether/how badly this needs fixing, rather than
	shipping unverified axis math for the skinned case blind."""
	import assimp_py

	rigid = skeleton is None
	texture_cache: dict = {}
	created_materials: dict = {}
	material_dicts: List[dict] = []
	meshes = []
	skin_bones = None
	if not rigid:
		state = _skin_state_for_shape(shape_value, skeleton)
		if state is not None:
			bone_indices = getattr(state, "matrix_ids", None)
			if bone_indices is None:
				bone_indices = state.bone_indices
			skin_bones = _skin_bones_for_export(state.bone_names, bone_indices, state.weights, state.inv_bind_matrices)

	for pass_index, (vertex_buffer, material_id, indices) in enumerate(iter_render_passes(shape_value)):
		if not indices:
			continue
		if not vertex_buffer.channels.get("Position"):
			continue

		if material_id not in created_materials:
			created_materials[material_id] = len(material_dicts)
			material_dicts.append(_assimp_material_dict(materials, material_id, output_path.parent, texture_mode, texture_finder, texture_cache))

		mesh = _assimp_mesh_from_geometry(
			f"pass_{pass_index}", vertex_buffer, indices, created_materials[material_id],
			bones=skin_bones, convert_to_yup=rigid)
		meshes.append(mesh)

	if not meshes:
		raise ValueError("No renderable geometry to export")

	mesh_node = assimp_py.Node("Meshes", _IDENTITY_MATRIX4, None, [], list(range(len(meshes))))
	root_children = [mesh_node]
	if not rigid:
		root_children.append(_assimp_skeleton_root_node(skeleton))
	root_node = assimp_py.Node("RootNode", _IDENTITY_MATRIX4, None, root_children, [])

	animations = None
	if animation is not None and not rigid:
		animations = [_assimp_animation_from_pynel(animation, skeleton)]

	# FBX-only: without this, FBXExporter.cpp::WriteGlobalSettings() falls back to
	# UnitScaleFactor=1.0 (FBX's own convention for "these are centimeters"), even though our
	# data is Ryzom-native meters -- Process_GlobalScale then divides by 100 on reimport,
	# shrinking everything (found 2026-09-05, Nuno: "il apparait tres petit et tres loin de la
	# camera"). 100.0 declares "1 unit = 100cm = 1 meter", matching our actual data as-is: no
	# conversion needed, just an honest declaration. .dae/.gltf have no such per-file unit
	# ambiguity (fixed axis/unit conventions), so this is scoped to .fbx alone.
	metadata = {"UnitScaleFactor": 100.0} if output_path.suffix.lower() == ".fbx" else None

	return assimp_py.Scene(root_node, meshes, material_dicts, animations, metadata=metadata)


_ASSIMP_FORMAT_IDS = {".obj": "obj", ".dae": "collada", ".fbx": "fbx", ".gltf": "gltf2", ".glb": "glb2"}


def _export_via_assimp(
		shape_value, materials, output_path: Path, texture_mode: str, texture_finder, skeleton=None, animation=None,
) -> List[Path]:
	import assimp_py

	format_id = _ASSIMP_FORMAT_IDS[output_path.suffix.lower()]
	scene = _assimp_scene_from_shape(shape_value, materials, output_path, texture_mode, texture_finder, skeleton, animation)
	assimp_py.export_file(scene, str(output_path), format_id)

	written = [output_path]
	if format_id == "obj":
		# assimp's ObjExporter always writes a sibling .mtl next to the .obj
		# (unlike collada/fbx/gltf2, which embed materials in the one file)
		# -- not reflected anywhere in `scene`, so it has to be added here by
		# hand or callers relying on the full written-files list (the zip
		# export path, export_dialog.py::_zip_written()) would silently drop it.
		written.append(output_path.with_suffix(".mtl"))
	texture_cache_names = set()
	for material_dict in scene.materials:
		textures = material_dict.get("TEXTURES") or {}
		for names in textures.values():
			texture_cache_names.update(names)
	written.extend(output_path.parent / name for name in texture_cache_names)
	return written


def _export_obj(shape_value, materials, output_path: Path, texture_mode: str, texture_finder) -> List[Path]:
	return _export_via_assimp(shape_value, materials, output_path, texture_mode, texture_finder)


def _export_dae(shape_value, materials, output_path: Path, texture_mode: str, texture_finder) -> List[Path]:
	return _export_via_assimp(shape_value, materials, output_path, texture_mode, texture_finder)


def _export_fbx(shape_value, materials, output_path: Path, texture_mode: str, texture_finder) -> List[Path]:
	return _export_via_assimp(shape_value, materials, output_path, texture_mode, texture_finder)


def _export_gltf_or_glb(shape_value, materials, output_path: Path, texture_mode: str, texture_finder) -> List[Path]:
	return _export_via_assimp(shape_value, materials, output_path, texture_mode, texture_finder)


EXPORT_FORMATS = [
	ExportFormat("obj", "Wavefront OBJ", supports_materials=True, export=_export_obj),
	ExportFormat("dae", "COLLADA", supports_materials=True, export=_export_dae),
	ExportFormat("stl", "STL", supports_materials=False, export=_export_stl),
	ExportFormat("fbx", "FBX", supports_materials=True, export=_export_fbx),
	ExportFormat("gltf", "glTF (JSON)", supports_materials=True, export=_export_gltf_or_glb),
	ExportFormat("glb", "glTF (binary)", supports_materials=True, export=_export_gltf_or_glb),
]


def export_shape(
		shape_value, name: str, export_format: ExportFormat, output_dir, texture_mode: str, texture_finder,
) -> List[Path]:
	"""Exports an already-parsed shape value -- e.g. the live, possibly-edited
	state of the shape currently open in the editor -- via `export_format`
	into `output_dir`. `name` supplies the output file's stem (typically the
	source .shape's own file name). Returns the list of files written.
	Raises `ValueError` (unsupported shape type / no renderable geometry) on
	failure.

	Bakes the shape's own `default_pos`/`default_pivot`/`default_rot_quat`/
	`default_scale` (the transform the engine actually applies at instance
	creation, see shape_geometry.py) into the exported vertices on a *copy*
	of the geometry -- the live shape open in the editor is never mutated --
	so the exported file shows the shape's real in-game placement, not its
	raw pre-transform storage pose. A no-op when the transform is identity.
	Found 2026-09-05 (Nuno: a shape with a non-null default_pos comes back
	at the origin after an export/reimport round trip) -- only rotation used
	to be baked, position/scale were silently dropped."""
	pos, pivot, quat, scale = shape_default_transform(shape_value)
	is_identity = pos == ZERO_VECTOR and pivot == ZERO_VECTOR and quat == IDENTITY_QUAT and scale == UNIT_VECTOR
	if not is_identity:
		shape_value = copy.deepcopy(shape_value)
		bake_default_transform_into_geom(shape_value.geom, pos, pivot, quat, scale)
	materials = getattr(shape_value, "materials", None)
	stem = Path(name).stem
	output_path = Path(output_dir) / f"{stem}.{export_format.extension}"
	return export_format.export(shape_value, materials, output_path, texture_mode, texture_finder)


def export_mesh_with_skin(shape_value, skeleton, output_path, texture_mode: str, texture_finder, animation=None) -> List[Path]:
	"""Public entry point for project-todos/forgery/mesh_skin_export.md --
	exports `shape_value` (a skinned Mesh/MeshMRM/MeshMRMSkinned) together
	with `skeleton` (a pynel.ryzom_shape.SkeletonShape whose bone names
	match shape_value's own bones_name) to `output_path`, format chosen from
	its extension among .dae/.fbx/.gltf/.glb. `animation`, if given (a
	pynel.ryzom_animation.Animation whose tracks are named after `skeleton`'s
	own bones), is embedded too (see _assimp_animation_from_pynel()) --
	ignored if `skeleton` is None (no skeleton to animate). Wired to Patina's
	export popup by mesh_skel_anim_io.md sous-chantier 6. Raises ValueError
	for an unsupported extension or no renderable geometry."""
	output_path = Path(output_path)
	suffix = output_path.suffix.lower()
	if suffix not in _ASSIMP_FORMAT_IDS:
		raise ValueError(f"Unsupported export format for a skinned mesh: {suffix!r} (expected one of {sorted(_ASSIMP_FORMAT_IDS)})")
	materials = getattr(shape_value, "materials", None)
	return _export_via_assimp(
		shape_value, materials, output_path, texture_mode, texture_finder, skeleton=skeleton, animation=animation)


def _attach_child_to_bone_node(node, bone_name: str, child) -> bool:
	"""Finds `bone_name` anywhere in `node`'s own subtree (an
	_assimp_skeleton_root_node() tree) and appends `child` to that Node's
	own `children` list in place -- `assimp_py.Node.children` is a plain
	Python list (see assimp_py's list_from_pyseq_required()), mutable after
	construction. Returns True once attached, so a caller walking multiple
	subtrees can stop early. Used to make a rigid weapon/shield mesh (see
	export_assembled_creature()) a child of its attach-point bone, so it
	inherits that bone's animated world transform for free instead of being
	baked to a single static pose."""
	if node.name == bone_name:
		node.children.append(child)
		return True
	return any(_attach_child_to_bone_node(c, bone_name, child) for c in node.children)


def export_assembled_creature(
		shape_values: Dict[str, object], skeleton, animation, output_path, texture_mode: str, texture_finder,
		weapon_shape_value=None, weapon_bone_name: Optional[str] = None,
) -> List[Path]:
	"""Public entry point for project-todos/forgery/mesh_skel_anim_io.md's
	item 7 ("export PNJ") -- merges every already-resolved body-part piece
	in `shape_values` (`{slot_name: shape_value}`, e.g.
	creature_bind.py's own `self._assembled_creature_shape_values`, already
	masking-correct: a slot holds exactly one piece, the equipment override
	if any, nothing else to exclude -- see mesh_skel_anim_io.md's own note
	on this, confirmed by Nuno) into ONE combined skinned export, all
	sharing `skeleton`. `animation`, if given, is embedded the same way
	export_mesh_with_skin() does. `weapon_shape_value`/`weapon_bone_name`
	(both or neither), if given, add a RIGID (non-skinned) mesh as a child
	of that bone's own Node in the skeleton hierarchy (see
	_attach_child_to_bone_node()) -- weapons/shields are purely additive,
	never masking any body part (see mesh_skel_anim_io.md's own decision).

	Kept in raw, unconverted Z-up like export_mesh_with_skin() (see its own
	docstring on why a skinned export isn't Y-up-converted yet)."""
	import assimp_py

	output_path = Path(output_path)
	suffix = output_path.suffix.lower()
	if suffix not in _ASSIMP_FORMAT_IDS:
		raise ValueError(f"Unsupported export format for an assembled creature: {suffix!r} (expected one of {sorted(_ASSIMP_FORMAT_IDS)})")

	texture_cache: dict = {}
	created_materials: dict = {}
	material_dicts: List[dict] = []
	meshes = []

	def add_pass(name_prefix, shape_value, materials, material_key_prefix, skin_bones):
		for pass_index, (vertex_buffer, material_id, indices) in enumerate(iter_render_passes(shape_value)):
			if not indices or not vertex_buffer.channels.get("Position"):
				continue
			key = (material_key_prefix, material_id)
			if key not in created_materials:
				created_materials[key] = len(material_dicts)
				material_dicts.append(_assimp_material_dict(
					materials, material_id, output_path.parent, texture_mode, texture_finder, texture_cache))
			mesh = _assimp_mesh_from_geometry(
				f"{name_prefix}_{pass_index}", vertex_buffer, indices, created_materials[key],
				bones=skin_bones, convert_to_yup=False)
			meshes.append(mesh)

	for slot_name, shape_value in shape_values.items():
		state = _skin_state_for_shape(shape_value, skeleton)
		skin_bones = None
		if state is not None:
			bone_indices = getattr(state, "matrix_ids", None)
			if bone_indices is None:
				bone_indices = state.bone_indices
			skin_bones = _skin_bones_for_export(state.bone_names, bone_indices, state.weights, state.inv_bind_matrices)
		add_pass(slot_name, shape_value, getattr(shape_value, "materials", None), slot_name, skin_bones)

	weapon_mesh_start = len(meshes)
	if weapon_shape_value is not None and weapon_bone_name is not None:
		add_pass("weapon", weapon_shape_value, getattr(weapon_shape_value, "materials", None), "__weapon__", None)

	if not meshes:
		raise ValueError("No renderable geometry to export")

	mesh_node = assimp_py.Node("Meshes", _IDENTITY_MATRIX4, None, [], list(range(weapon_mesh_start)))
	armature_node = _assimp_skeleton_root_node(skeleton)
	root_children = [mesh_node, armature_node]

	if len(meshes) > weapon_mesh_start:
		weapon_indices = list(range(weapon_mesh_start, len(meshes)))
		weapon_node = assimp_py.Node("Weapon", _IDENTITY_MATRIX4, None, [], weapon_indices)
		if not _attach_child_to_bone_node(armature_node, weapon_bone_name, weapon_node):
			# Attach-point bone not found in this skeleton -- fall back to a
			# top-level (unanimated) node rather than dropping the weapon.
			root_children.append(weapon_node)

	root_node = assimp_py.Node("RootNode", _IDENTITY_MATRIX4, None, root_children, [])

	animations = None
	if animation is not None:
		animations = [_assimp_animation_from_pynel(animation, skeleton)]

	# See _assimp_scene_from_shape()'s own comment on why this is .fbx-only.
	metadata = {"UnitScaleFactor": 100.0} if suffix == ".fbx" else None
	scene = assimp_py.Scene(root_node, meshes, material_dicts, animations, metadata=metadata)
	format_id = _ASSIMP_FORMAT_IDS[suffix]
	assimp_py.export_file(scene, str(output_path), format_id)

	written = [output_path]
	texture_names = set()
	for material_dict in material_dicts:
		for names in (material_dict.get("TEXTURES") or {}).values():
			texture_names.update(names)
	written.extend(output_path.parent / name for name in texture_names)
	return written
