"""Pure-ish Panda3D geometry-building helpers for `.ig` (instance group)
placement data (project-todos/forgery/landscape_editor.md step 11) --
visualisation only, no editing (select/move/add/delete is step 12). Mirrors
zone_geometry.py's "no app dependency" rule: the caller (landscape_editor.py)
does all Panda3D scene-graph attachment and threading, this module only
turns already-loaded pynel data into geometry.

Water detection (project-todos/forgery/landscape_editor.md step 11, Nuno
2026-09-08: no heuristic shortcut, a real per-instance `.ig`+`.shape` check
only) is `ShapeFile.type_name in WATER_TYPE_NAMES` -- the Python class name
pynel.ryzom_shape gives a parsed shape value (e.g. "WaterShape"), not the
on-disk "CWaterShape" tag.
"""

import dataclasses
from typing import Callable, List, Optional

from panda3d.core import Geom, GeomLines, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat, GeomVertexWriter

from pynel.ryzom_ig import InstanceGroup
from pynel.ryzom_shape import ShapeFile, ShapeParseError, WaterShape, parse_shape

from . import shape_geometry

# ShapeFile.type_name values with no renderable mesh (project-todos/forgery/
# landscape_editor.md step 11) -- WaterShape has a flat polygon footprint,
# WaveMakerShape just a point source, neither goes through iter_render_passes().
WATER_TYPE_NAMES = ("WaterShape", "WaveMakerShape")

# Cyan, semi-bright -- distinct from the terrain's elevation-gradient greens/
# browns/reds (zone_geometry.py) and from the untextured gray of ordinary
# instances below, so a water instance reads as "water" at a glance.
_WATER_COLOR = (0.2, 0.7, 0.9, 1.0)
# Flat neutral gray -- no material/texture resolution for step 11 (deferred
# the same way zone texturing was, project-todos/forgery/landscape_editor.md's
# own scope decisions), just enough shading (via normals) to read shapes.
_INSTANCE_COLOR = (0.75, 0.75, 0.75, 1.0)
# World-space half-size of the little cross marking a WaveMakerShape (a pure
# point source, no footprint of its own to draw).
_WAVE_MAKER_MARKER_SIZE = 1.0


@dataclasses.dataclass(slots=True)
class ResolvedInstance:
	"""One `.ig` Instance, classified and ready to place -- `kind` is "mesh"
	(render `shape_value` via build_instance_mesh_geom()), "water_polygon"
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
						shape_file = None
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


def build_instance_mesh_geom(shape_value) -> Optional[GeomNode]:
	"""One flat-colored GeomNode (_INSTANCE_COLOR) merging every render pass
	of `shape_value` -- no materials/UVs/normals, texturing is out of scope
	for step 11 (same as terrain's own, deferred to step 14). None if
	`shape_value`'s type has no renderable mesh (iter_render_passes() yields
	nothing) -- e.g. a FlareShape/ParticleSystemShape instance, neither
	water nor a mesh."""
	passes = list(shape_geometry.iter_render_passes(shape_value))
	if not passes:
		return None

	total_verts = sum(vertex_buffer.num_verts for vertex_buffer, _material_id, _indices in passes)
	vdata, vertex_writer, color_writer = _make_vdata_writer(total_verts)

	triangles = GeomTriangles(Geom.UH_static)
	base_index = 0
	for vertex_buffer, _material_id, indices in passes:
		positions = vertex_buffer.channels.get("Position") or []
		for p in positions:
			vertex_writer.add_data3(p[0], p[1], p[2])
			color_writer.add_data4(*_INSTANCE_COLOR)
		for i in range(0, len(indices), 3):
			triangles.add_vertices(base_index + indices[i], base_index + indices[i + 1], base_index + indices[i + 2])
		base_index += vertex_buffer.num_verts
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("ig-instance-mesh")
	node.add_geom(geom)
	return node


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
