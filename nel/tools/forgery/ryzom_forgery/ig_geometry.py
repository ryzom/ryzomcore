"""Pure-ish Panda3D geometry-building helpers for `.ig` (instance group)
placement data (project-todos/forgery/landscape_editor__ig_full_load.md) --
visualisation only, no editing (select/move/add/delete is step 13). Mirrors
zone_geometry.py's "no app dependency" rule: the caller (landscape_editor.py)
does all Panda3D scene-graph attachment and threading, this module only
turns already-loaded pynel data into geometry -- ONE deliberate exception,
`build_textured_instance_template()` below, which reuses
`apps.object_editor_mixins`' already-proven material/texture application
(`MaterialsMixin._apply_material_common`, `_material_alpha_from_texture`,
the blend/alpha-test constants) and vertex-buffer building
(`geometry_helpers._build_vertex_data`/`_build_geom`) rather than
duplicating that logic a second time.

Water detection (project-todos/forgery/landscape_editor.md step 11, Nuno
2026-09-08: no heuristic shortcut, a real per-instance `.ig`+`.shape` check
only) is `ShapeFile.type_name in WATER_TYPE_NAMES` -- the Python class name
pynel.ryzom_shape gives a parsed shape value (e.g. "WaterShape"), not the
on-disk "CWaterShape" tag.
"""

import dataclasses
from typing import Callable, List, Optional

from panda3d.core import (
	AlphaTestAttrib, ColorBlendAttrib, Geom, GeomLines, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat,
	GeomVertexWriter, Material as PandaMaterial, NodePath,
)

from pynel.ryzom_ig import InstanceGroup
from pynel.ryzom_shape import ShapeFile, ShapeParseError, WaterShape, parse_shape

from . import shape_geometry
from .apps.object_editor_mixins.geometry_helpers import _build_geom, _build_vertex_data
from .apps.object_editor_mixins.materials import (
	MaterialsMixin, _IDRV_MAT_ALPHA_TEST, _IDRV_MAT_BLEND, _material_alpha_from_texture, _TBLEND_TO_PANDA_OPERAND,
)

# ShapeFile.type_name values with no renderable mesh (project-todos/forgery/
# landscape_editor.md step 11) -- WaterShape has a flat polygon footprint,
# WaveMakerShape just a point source, neither goes through iter_render_passes().
WATER_TYPE_NAMES = ("WaterShape", "WaveMakerShape")

# Cyan, semi-bright, semi-transparent (Nuno 2026-09-13: "tu peux faire l'eau
# semi-transparent?") -- distinct from the terrain's elevation-gradient
# greens/browns/reds (zone_geometry.py) and from the real textured colors of
# ordinary mesh instances, so a water instance reads as "water" at a glance.
# The alpha channel here only takes effect once the caller also enables
# blending on the instance NodePath (landscape_editor.py's own
# _attach_textured_ig_instance(), TransparencyAttrib.M_alpha) -- Panda3D
# ignores per-vertex alpha by default.
_WATER_COLOR = (0.2, 0.7, 0.9, 0.5)
# World-space half-size of the little cross marking a WaveMakerShape (a pure
# point source, no footprint of its own to draw).
_WAVE_MAKER_MARKER_SIZE = 1.0


@dataclasses.dataclass(slots=True)
class ResolvedInstance:
	"""One `.ig` Instance, classified and ready to place -- `kind` is "mesh"
	(render `shape_value` via build_textured_instance_template()), "water_polygon"
	(render `water_polygon` via build_water_polygon_geom()), or "water_point"
	(a WaveMakerShape, render build_water_point_geom()). `shape_value`/
	`water_polygon` are only set for their own matching `kind`."""
	shape_name: str
	pos: tuple
	rot: tuple  # (w, x, y, z), Panda3D Quat order
	scale: tuple
	kind: str
	shape_value: object = None
	water_polygon: Optional[List[tuple]] = None


def _shape_file_name(shape_name: str) -> Optional[str]:
	"""The real `.shape` file name to resolve `shape_name` against, or None
	if it clearly isn't one -- confirmed 2026-09-13 against real data
	(live_data's own `*_ig.bnp`) that ~25% of all real instances give a bare
	name with NO extension at all (e.g. "Water_village_nb_05"), which the
	engine resolves as "<name>.shape" (a real "water_village_nb_05.shape"
	does exist in lacustre_shapes.bnp) -- NOT every extension-less name is
	missing one, so this must special-case "no dot at all", not merely "not
	ending in .shape". A minority reference a genuinely different format
	instead (".ps" particle systems, ".pacs_prim" PACS collision
	primitives) -- neither a static mesh nor water, out of scope for step 11
	(particle effects/PACS overlay are their own separate concerns, step 15
	for PACS), so these are rejected outright rather than fed to
	parse_shape() only to fail."""
	if "." not in shape_name:
		return f"{shape_name}.shape"
	if shape_name.lower().endswith(".shape"):
		return shape_name
	return None


def resolve_ig_instances(ig: InstanceGroup, read_shape_bytes: Callable[[str], Optional[bytes]]) -> List[ResolvedInstance]:
	"""Classifies every `ig.instances` entry, resolving+parsing its `.shape`
	via `read_shape_bytes(shape_file_name)` (caller's job: this is where an
	app's SearchPathsDialog/`.bnp` lookup lives, kept out of this module same
	as zone_geometry.py stays free of region_loader.py's own file
	resolution). An instance whose shape can't be found/parsed/isn't a real
	`.shape` at all (see _shape_file_name()) is silently skipped (best-effort
	visualisation, not a hard requirement -- missing/broken shape references
	exist on real data). Pure Python/pynel, safe to call from a background
	thread (no Panda3D object built here)."""
	resolved = []
	shape_cache = {}
	for instance in ig.instances:
		pos = (instance.pos.x, instance.pos.y, instance.pos.z)
		rot = (instance.rot.w, instance.rot.x, instance.rot.y, instance.rot.z)
		scale = (instance.scale.x, instance.scale.y, instance.scale.z)

		if instance.shape_name not in shape_cache:
			shape_file = None
			shape_file_name = _shape_file_name(instance.shape_name)
			if shape_file_name is not None:
				data = read_shape_bytes(shape_file_name)
				if data is not None:
					try:
						shape_file = parse_shape(data)
					except ShapeParseError:
						pass
			shape_cache[instance.shape_name] = shape_file
		shape_file = shape_cache[instance.shape_name]
		if shape_file is None:
			continue

		if shape_file.type_name in WATER_TYPE_NAMES:
			if isinstance(shape_file.value, WaterShape):
				polygon = [(p.x, p.y) for p in shape_file.value.polygon]
				resolved.append(ResolvedInstance(instance.shape_name, pos, rot, scale, "water_polygon", water_polygon=polygon))
			else:
				resolved.append(ResolvedInstance(instance.shape_name, pos, rot, scale, "water_point"))
			continue

		resolved.append(ResolvedInstance(instance.shape_name, pos, rot, scale, "mesh", shape_value=shape_file.value))
	return resolved


def _make_vdata_writer(num_verts: int):
	"""v3c4 (position + per-vertex color, no normals/lighting) -- same
	convention as zone_geometry.py's own elevation-colored terrain: a flat,
	always-visible color regardless of scene lighting, rather than depending
	on a lit material/normals this step doesn't otherwise need (no
	texturing, deferred to step 14)."""
	vformat = GeomVertexFormat.get_v3c4()
	vdata = GeomVertexData("ig-instance", vformat, Geom.UH_static)
	vdata.set_num_rows(num_verts)
	return vdata, GeomVertexWriter(vdata, "vertex"), GeomVertexWriter(vdata, "color")


def _apply_ig_material(node_path, material, find_file: Callable, texture_cache: dict) -> None:
	"""Applies `material`'s real diffuse texture (+alpha test/blend) to
	`node_path` -- the same technique object_editor_mixins.materials'
	MaterialsMixin uses for Patina's own shape rendering
	(_apply_material_common() for two-sided/depth-write + blend/alpha-test
	flags, load_panda_texture() for the actual decode+cache), simplified for
	`.ig` instances: no panoply override (a `.ig` prop is never a player
	equipment slot), no specular overlay (project-todos/forgery/
	landscape_editor__ig_full_load.md's own scope decision -- diffuse only).
	No-op (besides the two-sided/depth-write flags) if `material` is None.

	Ambient forced to max, diffuse/specular/emissive left at the Material
	default rather than computed from `material`'s own real values (Nuno
	2026-09-13: real per-material diffuse/specular values, combined with
	whatever lights actually exist in the viewport, made a lot of real
	instances render solid black -- Atyscape has no proper scene lighting
	setup for lit materials, unlike Patina's own dedicated preview lights).
	The texture itself still carries all the real visual detail; this only
	controls how bright it reads under whatever lighting the viewport
	happens to have."""
	MaterialsMixin._apply_material_common(node_path, material)
	if material is None:
		return

	panda_material = PandaMaterial()
	panda_material.set_ambient((1.0, 1.0, 1.0, 1.0))
	panda_material.set_twoside(True)
	node_path.set_material(panda_material)

	if material.flags & _IDRV_MAT_BLEND:
		src_op = _TBLEND_TO_PANDA_OPERAND[material.src_blend]
		dst_op = _TBLEND_TO_PANDA_OPERAND[material.dst_blend]
		node_path.set_attrib(ColorBlendAttrib.make(ColorBlendAttrib.M_add, src_op, dst_op))
	if material.flags & _IDRV_MAT_ALPHA_TEST and _material_alpha_from_texture(material):
		node_path.set_attrib(AlphaTestAttrib.make(AlphaTestAttrib.M_greater, material.alpha_test_threshold))

	texture = material.textures[0] if material.textures else None
	if texture is not None and texture.file_name:
		panda_texture = shape_geometry.load_panda_texture(
			texture.file_name, cache=texture_cache, finder=find_file, repeat=True,
			wrap_s=texture.wrap_s, wrap_t=texture.wrap_t,
			min_filter=texture.min_filter, mag_filter=texture.mag_filter,
			load_grayscale_as_alpha=texture.load_grayscale_as_alpha,
		)
		if panda_texture is not None:
			node_path.set_texture(panda_texture)


def build_textured_instance_template(
	shape_value, find_file: Callable, texture_cache: dict,
) -> Optional[NodePath]:
	"""A detached template NodePath for `shape_value` (project-todos/forgery/
	landscape_editor__ig_full_load.md step 3) -- one child NodePath per real
	render pass, each with its own real diffuse(+alpha) texture/material
	applied via `_apply_ig_material()`. Meant to be `instance_to()`'d once
	per real `.ig` placement (Panda3D's own cheap scene-graph instancing --
	the underlying Geom/Texture data is shared across every instance, only
	each instance's own transform differs), NOT rebuilt per instance, same
	spirit as the old build_instance_mesh_geom() this replaces but now with
	real materials instead of a flat gray color. None if `shape_value` has
	no renderable render pass at all (e.g. a FlareShape/ParticleSystemShape
	instance)."""
	materials = getattr(shape_value, "materials", None)
	template = NodePath("ig-instance-template")
	vdata = None
	built_any = False
	for pass_index, (vertex_buffer, material_id, indices) in enumerate(shape_geometry.iter_render_passes(shape_value)):
		if not indices:
			continue
		if vdata is None:
			vdata = _build_vertex_data(vertex_buffer)
		geom = _build_geom(vdata, indices)
		geom_node = GeomNode(f"pass-{pass_index}")
		geom_node.add_geom(geom)
		pass_np = template.attach_new_node(geom_node)
		material = materials[material_id] if materials and material_id < len(materials) else None
		_apply_ig_material(pass_np, material, find_file, texture_cache)
		built_any = True
	return template if built_any else None


def build_water_polygon_geom(polygon: List[tuple]) -> Optional[GeomNode]:
	"""Flat cyan filled polygon (triangle fan around the footprint's own
	centroid) at local Z=0, from a WaterShape's own `polygon` (already in the
	shape's local space -- the caller's NodePath transform, the instance's
	own pos/rot/scale, places it in the world). None for a degenerate
	footprint (fewer than 3 points)."""
	if len(polygon) < 3:
		return None
	center_x = sum(p[0] for p in polygon) / len(polygon)
	center_y = sum(p[1] for p in polygon) / len(polygon)

	vdata, vertex_writer, color_writer = _make_vdata_writer(len(polygon) + 1)
	vertex_writer.add_data3(center_x, center_y, 0.0)
	color_writer.add_data4(*_WATER_COLOR)
	for x, y in polygon:
		vertex_writer.add_data3(x, y, 0.0)
		color_writer.add_data4(*_WATER_COLOR)

	triangles = GeomTriangles(Geom.UH_static)
	for i in range(1, len(polygon) + 1):
		next_i = i + 1 if i < len(polygon) else 1
		triangles.add_vertices(0, i, next_i)
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("ig-water-polygon")
	node.add_geom(geom)
	return node


def build_water_point_geom() -> GeomNode:
	"""Small cyan cross marking a WaveMakerShape instance (a pure point
	source, no footprint of its own to draw)."""
	size = _WAVE_MAKER_MARKER_SIZE
	vdata, vertex_writer, color_writer = _make_vdata_writer(4)
	for x, y in ((-size, 0.0), (size, 0.0), (0.0, -size), (0.0, size)):
		vertex_writer.add_data3(x, y, 0.0)
		color_writer.add_data4(*_WATER_COLOR)
	lines = GeomLines(Geom.UH_static)
	lines.add_vertices(0, 1)
	lines.add_vertices(2, 3)
	lines.close_primitive()
	geom = Geom(vdata)
	geom.add_primitive(lines)
	node = GeomNode("ig-wave-maker-point")
	node.add_geom(geom)
	return node
