"""Pure Panda3D geometry-building helpers shared by ObjectEditorApp and its
object_editor_mixins/ mixins (main shape rendering, creature-bind assembly,
reference shapes...).

Like ui_helpers.py, has ZERO dependency on object_editor.py -- see that
module's docstring for why a mixin must never import back from
object_editor.py.
"""

from math import cos, pi, sin

from panda3d.core import Geom, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat, GeomVertexWriter, LineSegs

from pynel.ryzom_shape import MeshMRM, MeshMRMSkinned

# Viewport helper toggles (floor grid, world/pivot axes) drawn bottom-left of
# the 3D viewport -- see ObjectEditorApp._draw_viewport_toggles(). Grid squares
# are 1x1m, matching how shapes are actually scaled in-game. Both the grid's
# footprint and the axes' length are re-derived from the loaded shape's own
# bounding box each time geometry is (re)built (see _rebuild_viewport_helpers()),
# so they scale to cover whatever object is on screen instead of a fixed size
# that's too small for a building and absurdly oversized for a trinket. The
# constants below are only the fallback used before any shape is loaded.
_GRID_SQUARES = 10
_GRID_STEP = 1.0
_GRID_COLOR = (0.5, 0.5, 0.5, 1.0)
_GRID_MARGIN_SQUARES = 2  # extra squares of margin around the bbox footprint
_MIN_GRID_SQUARES = 4

# World axes use the conventional red/green/blue X/Y/Z scheme (matches
# navcube.py's own gizmo); pivot axes deliberately use a different palette so
# both can be shown together without one being mistaken for the other.
_WORLD_AXIS_COLORS = {"x": (0.95, 0.25, 0.25, 1.0), "y": (0.3, 0.85, 0.3, 1.0), "z": (0.3, 0.55, 1.0, 1.0)}
_PIVOT_AXIS_COLORS = {"x": (0.95, 0.2, 0.85, 1.0), "y": (0.95, 0.85, 0.15, 1.0), "z": (0.2, 0.9, 0.9, 1.0)}
_AXIS_VECTORS = {"x": (1.0, 0.0, 0.0), "y": (0.0, 1.0, 0.0), "z": (0.0, 0.0, 1.0)}
_AXIS_LENGTH = 3.0  # fallback, before any shape is loaded
_AXIS_MARGIN_FACTOR = 1.2  # how far axes extend past the bbox radius


def _build_grid_geom(center_x, center_y, z, squares):
	"""LineSegs floor grid on the world XY plane at height `z`, centered on
	(center_x, center_y): `squares` x `squares` squares, _GRID_STEP meters each."""
	half = squares * _GRID_STEP / 2.0
	lines = LineSegs("floor-grid")
	lines.set_color(*_GRID_COLOR)
	for i in range(squares + 1):
		coord = -half + i * _GRID_STEP
		lines.move_to(center_x + coord, center_y - half, z)
		lines.draw_to(center_x + coord, center_y + half, z)
		lines.move_to(center_x - half, center_y + coord, z)
		lines.draw_to(center_x + half, center_y + coord, z)
	return lines.create()


def _build_axes_geom(colors, length):
	"""LineSegs X/Y/Z axes through the origin, each spanning +/-length along
	its axis, colored per `colors` (one of _WORLD_AXIS_COLORS/_PIVOT_AXIS_COLORS)."""
	lines = LineSegs("axes")
	for axis, vector in _AXIS_VECTORS.items():
		lines.set_color(*colors[axis])
		lines.move_to(-vector[0] * length, -vector[1] * length, -vector[2] * length)
		lines.draw_to(vector[0] * length, vector[1] * length, vector[2] * length)
	return lines.create()


# Warm yellow/gold, evokes "sun" -- distinct from every other gizmo palette
# above so it's never mistaken for one of them.
_SUN_GIZMO_COLOR = (1.0, 0.85, 0.2, 1.0)
_SUN_GLOBE_FRACTION = 0.06  # solid globe radius, as a fraction of length -- small, just a marker
# `length` here is already axis_length (bbox radius * _AXIS_MARGIN_FACTOR,
# 1.2 -- see _rebuild_viewport_helpers()), so this factor is chosen to land
# the globe at roughly 2x the shape's own bbox radius (2.0 / 1.2), not 2x
# axis_length itself.
_SUN_GLOBE_DISTANCE_FACTOR = 2.0 / _AXIS_MARGIN_FACTOR
_SUN_GLOBE_RINGS = 6
_SUN_GLOBE_SEGMENTS = 8


def _build_sun_globe_geom(length):
	"""Small solid unlit sphere (position+color triangle mesh, coarse --
	6x8 -- since it's a tiny marker, not a real render target) at local
	(0, -distance, 0), radius and distance both derived from `length` (the
	loaded shape's own bbox-relative axis length, see
	_rebuild_viewport_helpers()). Deliberately just the globe, no ray fan/
	direction indicator, and no adjustable distance -- both tried, both
	dropped as more distracting than useful / not actually meaningful (a
	DirectionalLight has no position at all, so a "distance" control had
	nothing real to represent -- 2026-09-02, Nuno)."""
	radius = length * _SUN_GLOBE_FRACTION
	center_y = -length * _SUN_GLOBE_DISTANCE_FACTOR

	vdata = GeomVertexData("sun-globe", GeomVertexFormat.get_v3c4(), Geom.UH_static)
	vdata.set_num_rows((_SUN_GLOBE_RINGS + 1) * (_SUN_GLOBE_SEGMENTS + 1))
	vertex_writer = GeomVertexWriter(vdata, "vertex")
	color_writer = GeomVertexWriter(vdata, "color")
	for ring in range(_SUN_GLOBE_RINGS + 1):
		theta = pi * ring / _SUN_GLOBE_RINGS
		for seg in range(_SUN_GLOBE_SEGMENTS + 1):
			phi = 2.0 * pi * seg / _SUN_GLOBE_SEGMENTS
			x = radius * sin(theta) * cos(phi)
			y = radius * sin(theta) * sin(phi)
			z = radius * cos(theta)
			vertex_writer.add_data3(x, center_y + y, z)
			color_writer.add_data4(*_SUN_GIZMO_COLOR)

	triangles = GeomTriangles(Geom.UH_static)
	stride = _SUN_GLOBE_SEGMENTS + 1
	for ring in range(_SUN_GLOBE_RINGS):
		for seg in range(_SUN_GLOBE_SEGMENTS):
			i0 = ring * stride + seg
			i1 = i0 + 1
			i2 = i0 + stride
			i3 = i2 + 1
			triangles.add_vertices(i0, i2, i1)
			triangles.add_vertices(i1, i2, i3)
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("sun-globe")
	node.add_geom(geom)
	return node


# How far outside [0, 1] a UV has to land before it's treated as genuine tiling
# intent (see _uvs_need_repeat()) rather than float imprecision from an .fbx/
# .dae/.obj import (assimp/Blender routinely overshoot by a thousandth or so) --
# real tiling content overshoots by at least a whole unit (e.g. V up to 2.0 on
# ooc_summer_raceline.shape), nowhere close to this margin.
_UV_REPEAT_MARGIN = 0.05


def _uvs_need_repeat(texcoords):
	"""Whether `texcoords` (a shape's whole TexCoord0 channel) actually relies
	on the texture wrapping/repeating (see load_panda_texture()'s `repeat`
	param) -- Panda3D's own default wrap mode is clamp, correct for the
	common case of a single, non-tiling texture per material, but wrong for
	shapes that deliberately go outside [0, 1] for tiling. Real, native NeL
	shapes do this often (repeating a wall/floor texture); imported meshes'
	UVs staying just barely outside [0, 1] (e.g. 1.0008) is float noise, not
	tiling intent, and switching those to repeat too visibly shifts the
	texture instead (a fraction of a pixel wrapping becomes a full, visible
	seam once the *whole* texture wraps around the boundary)."""
	if not texcoords:
		return False
	return any(
		u < -_UV_REPEAT_MARGIN or u > 1.0 + _UV_REPEAT_MARGIN or v < -_UV_REPEAT_MARGIN or v > 1.0 + _UV_REPEAT_MARGIN
		for u, v in texcoords)


def _is_shape_skinned(shape_value):
	"""True if `shape_value` has real skin data to drive from a skeleton --
	either a CMeshMRMSkinned (always skinned) or a plain CMeshMRM that
	happens to carry its own skin data too (geom.skinned, a separate,
	older on-disk format -- confirmed real, 2026-08-30, e.g. Ryzom's
	*_visage.shape face pieces are plain CMeshMRM, not CMeshMRMSkinned, yet
	are skinned -- see shape_geometry.py's _passes_from_mrm_geom() and
	pynel.ryzom_skin.skin_mesh_mrm_geom()). Use this everywhere instead of
	`isinstance(shape_value, MeshMRMSkinned)` alone -- that check alone
	silently treats a skinned CMeshMRM as rigid."""
	if isinstance(shape_value, MeshMRMSkinned):
		return True
	if isinstance(shape_value, MeshMRM):
		return bool(shape_value.geom.skinned)
	return False


# Flat opaque black -- reads as an actual (approximate) cast shadow, not a
# debug gizmo, so no bright/distinct color needed like the other overlays.
# Deliberately opaque, not semi-transparent (tried first, dropped 2026-09-03,
# Nuno: overlapping triangles double up under alpha blending, showing as
# visible streaking where the mesh's own triangles overlap) -- fully opaque
# means overlap is invisible, every triangle just paints the same color.
_SHADOW_SKIN_COLOR = (0.0, 0.0, 0.0, 1.0)
# World Z the shadow is projected onto -- matches the floor grid's own fixed
# Z=0 (see _rebuild_viewport_helpers()'s own "actual ground reference" note).
_SHADOW_SKIN_GROUND_Z = 0.0
# Nudged slightly above the ground plane to avoid z-fighting with the floor
# grid's own lines (both would otherwise sit at the exact same Z).
_SHADOW_SKIN_GROUND_OFFSET = 0.01


def _build_shadow_skin_vdata(shadow_skin):
	"""GeomVertexData (position-only, UH_dynamic) for the CShadowSkin ground-
	shadow preview -- one row per ShadowVertex, seeded at its raw (un-
	projected) bind-pose position; overwritten every frame with its actual
	ground-projected position by _reskin_shadow_skin_state() before the
	first real frame is seen (same buffer-rewrite technique as
	_update_skin_preview()/_update_wind())."""
	vdata = GeomVertexData("shadow-skin", GeomVertexFormat.get_v3(), Geom.UH_dynamic)
	vdata.set_num_rows(len(shadow_skin.vertices))
	vertex_writer = GeomVertexWriter(vdata, "vertex")
	for v in shadow_skin.vertices:
		vertex_writer.add_data3(v.position.x, v.position.y, v.position.z)
	return vdata


def _build_shadow_skin_ground_node(vdata, shadow_skin):
	"""Filled mesh (GeomTriangles, same triangle list as the CShadowSkin
	itself) over `vdata`'s vertices -- an approximate "blob shadow" flattened
	onto the ground plane by _reskin_shadow_skin_state() every frame, NOT a
	real shadow-map render (no soft edges, no terrain-shape wrapping, no
	blending with other body-part shadows -- see the 2026-09-03 discussion
	for what this consciously leaves out). Color/light-off/transparency
	applied by the caller via NodePath methods, same as the wireframe
	overlays elsewhere in this module."""
	triangles = GeomTriangles(Geom.UH_static)
	tris = shadow_skin.triangles
	for i in range(0, len(tris), 3):
		triangles.add_vertices(tris[i], tris[i + 1], tris[i + 2])
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("shadow-skin-ground")
	node.add_geom(geom)
	return node


def _build_vertex_data(vertex_buffer, dynamic=False):
	"""Builds the GeomVertexData for `vertex_buffer` -- shared by every render
	pass's Geom (all of a mesh's passes index into the exact same vertex
	array, see iter_render_passes()), so it's built once per _rebuild_geometry()
	call rather than once per pass. `dynamic`, when the loaded shape has wind
	animation data (see _update_wind()), marks the vertex column UH_dynamic so
	Panda3D doesn't assume the position data is upload-once-and-forget."""
	positions = vertex_buffer.channels.get("Position")
	normals = vertex_buffer.channels.get("Normal")
	texcoords = vertex_buffer.channels.get("TexCoord0")

	if normals and texcoords:
		vformat = GeomVertexFormat.get_v3n3t2()
	elif normals:
		vformat = GeomVertexFormat.get_v3n3()
	elif texcoords:
		vformat = GeomVertexFormat.get_v3t2()
	else:
		vformat = GeomVertexFormat.get_v3()

	vdata = GeomVertexData("shape", vformat, Geom.UH_dynamic if dynamic else Geom.UH_static)
	vdata.set_num_rows(vertex_buffer.num_verts)

	vertex_writer = GeomVertexWriter(vdata, "vertex")
	for p in positions:
		vertex_writer.add_data3(p[0], p[1], p[2])

	if normals:
		normal_writer = GeomVertexWriter(vdata, "normal")
		for n in normals:
			normal_writer.add_data3(n[0], n[1], n[2])

	if texcoords:
		uv_writer = GeomVertexWriter(vdata, "texcoord")
		for uv in texcoords:
			# NeL .shape files always store texture V with the opposite
			# origin from Panda3D; flip here (once, for every shape,
			# regardless of source -- shape_import.py's importers convert
			# .obj/.dae/.fbx's own native V convention into this same NeL one
			# at import time, precisely so this flip can stay simple and
			# unconditional and every *saved* .shape file is correct on its
			# own, not just correct-looking in Forgery for as long as it
			# remembers where a shape came from).
			#
			# Deliberately NOT wrapped to [0, 1) here, even though some
			# shapes' UVs go beyond that range (e.g. a whole render pass at V
			# in [1, 2) on ooc_summer_raceline.shape) -- reducing each vertex
			# mod 1 independently breaks any triangle whose UVs straddle an
			# integer boundary (routine for tiling textures, e.g. a
			# cylindrical trunk wrapping around): one corner jumps to the far
			# side of the texture while the others don't, stretching the
			# whole image across that triangle. The out-of-range values are
			# left as-is; the texture's own wrap mode (see
			# load_panda_texture()) is what needs to repeat correctly instead.
			uv_writer.add_data2(uv[0], 1.0 - uv[1])

	return vdata


def _build_geom(vdata, indices):
	"""Builds one render pass's Geom (just its triangle indices) against an
	already-built, shared vdata (see _build_vertex_data())."""
	triangles = GeomTriangles(Geom.UH_static)
	for i in range(0, len(indices), 3):
		triangles.add_vertices(indices[i], indices[i + 1], indices[i + 2])
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	return geom
