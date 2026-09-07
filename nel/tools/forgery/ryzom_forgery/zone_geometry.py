"""Pure Panda3D geometry-building helpers for `.zone`/`.zonew`/`.zonel`
landscape data (`pynel.ryzom_zone.Zone`) -- used by the Landscape Editor
(project-todos/forgery/landscape_editor.md).

Like apps/object_editor_mixins/geometry_helpers.py, has ZERO dependency on
any app module.
"""

from panda3d.core import Geom, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat, GeomVertexWriter

from pynel.ryzom_zone import unpack_vertex

# Elevation gradient (low -> high): dark green -> pale yellow-green, a single
# hue so relief reads clearly at a glance (real tile texturing/lighting is
# out of scope until later steps -- project-todos/forgery/landscape_editor.md
# step 5 is the dedicated "Rendu par élévation" step this pre-empts for a
# nicer-looking step-3 low-poly view, per Nuno, 2026-09-07).
_LOW_COLOR = (0.05, 0.2, 0.05, 1.0)
_HIGH_COLOR = (0.85, 0.95, 0.35, 1.0)


def _elevation_color(z, min_z, max_z):
	span = max_z - min_z
	t = 0.5 if span <= 0.0 else max(0.0, min(1.0, (z - min_z) / span))
	return tuple(lo + (hi - lo) * t for lo, hi in zip(_LOW_COLOR, _HIGH_COLOR))


def build_zone_low_poly_geom(zone) -> GeomNode:
	"""Low-poly zone mesh: one quad per patch, using only its 4 stored
	corners (patch.vertices) -- no Bezier tessellation (project-todos/
	forgery/landscape_editor.md step 3; step 4 adds real Bezier-evaluated
	tessellation later). Colored per vertex by elevation (green gradient,
	see _elevation_color()) rather than by CTileColor -- a nicer-looking
	stand-in for the dedicated "Rendu par élévation" step (step 5) until real
	texturing exists; real per-tile coloring can come back later if wanted.

	Also serves as the "2D projected" view (same step): there's no separate
	2D-only renderer, the top-down view is just this exact mesh seen from
	directly above (see LandscapeEditorApp's "Top view" button, snapping
	OrbitCamera to the "+z" axis view)."""
	bb = zone.zone_bb
	min_z = bb.center.z - bb.half_size.z
	max_z = bb.center.z + bb.half_size.z

	vdata = GeomVertexData("zone-low-poly", GeomVertexFormat.get_v3c4(), Geom.UH_static)
	vdata.set_num_rows(len(zone.patchs) * 4)
	vertex_writer = GeomVertexWriter(vdata, "vertex")
	color_writer = GeomVertexWriter(vdata, "color")

	triangles = GeomTriangles(Geom.UH_static)
	for patch_index, patch in enumerate(zone.patchs):
		base = patch_index * 4
		# Loop order matches CBezierPatch::eval()'s actual corner mapping
		# (Vertices[0]=(s=0,t=0), [3]=(s=1,t=0), [2]=(s=1,t=1), [1]=(s=0,t=1)
		# -- see ryzom-core bezier_patch.cpp:98-133), NOT the raw on-disk
		# index order, so the quad winds around its real boundary instead of
		# crossing itself.
		loop = (patch.vertices[0], patch.vertices[3], patch.vertices[2], patch.vertices[1])
		for packed_vertex in loop:
			world = unpack_vertex(packed_vertex, zone.patch_bias, zone.patch_scale)
			vertex_writer.add_data3(world.x, world.y, world.z)
			color_writer.add_data4(*_elevation_color(world.z, min_z, max_z))
		triangles.add_vertices(base + 0, base + 1, base + 2)
		triangles.add_vertices(base + 0, base + 2, base + 3)
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("zone-low-poly")
	node.add_geom(geom)
	return node
