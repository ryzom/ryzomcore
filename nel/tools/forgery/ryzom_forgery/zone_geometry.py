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
# nicer-looking view, per Nuno, 2026-09-07).
_LOW_COLOR = (0.05, 0.2, 0.05, 1.0)
_HIGH_COLOR = (0.85, 0.95, 0.35, 1.0)

# "grille alignée sur OrderS x OrderT (max 16x16 tuiles/patch)" (project-todos/
# forgery/landscape_editor.md's scope decision) -- OrderS/OrderT themselves
# are already at most 16 on real data (tile bank's own patch subdivision
# limit), no extra clamp needed. Patches with no tile-grid info at all
# (order_s/order_t == 0, patch version < 2) fall back to this 1x1 grid --
# same as a plain 4-corner quad.
_MIN_GRID_SEGMENTS = 1


def _elevation_color(z, min_z, max_z):
	span = max_z - min_z
	t = 0.5 if span <= 0.0 else max(0.0, min(1.0, (z - min_z) / span))
	return tuple(lo + (hi - lo) * t for lo, hi in zip(_LOW_COLOR, _HIGH_COLOR))


def _cubic_bernstein(t):
	"""The 4 cubic Bernstein basis values at `t` -- B0=(1-t)^3, B1=3t(1-t)^2,
	B2=3t^2(1-t), B3=t^3. Matches CBezierPatch::eval()'s own s0/s1/s2/s3 (and
	t0/t1/t2/t3) terms exactly (ryzom-core bezier_patch.cpp:102-115)."""
	t1 = 1.0 - t
	t12 = t1 * t1
	t2 = t * t
	return (t12 * t1, 3.0 * t * t12, 3.0 * t2 * t1, t2 * t)


def _bezier_control_grid(patch, bias, scale):
	"""Builds the canonical 4x4 tensor-product Bezier control-point grid
	(world-space Vector3, grid[s_index][t_index]) from a Patch's NeL-indexed
	Vertices/Tangents/Interiors. Derived by inspecting which (s,t) corner
	each term of CBezierPatch::eval() (bezier_patch.cpp:98-133) contributes
	to -- e.g. the s0*t0 term uses Vertices[0], so grid[0][0] = Vertices[0];
	the s1*t0 term uses Tangents[7], so grid[1][0] = Tangents[7]; etc. Once
	arranged this way, a plain tensor-product Bezier evaluation (see
	_eval_bezier_patch()) reproduces eval() exactly, without hand-listing all
	16 flattened terms."""
	v = [unpack_vertex(p, bias, scale) for p in patch.vertices]
	tg = [unpack_vertex(p, bias, scale) for p in patch.tangents]
	it = [unpack_vertex(p, bias, scale) for p in patch.interiors]
	return (
		(v[0], tg[0], tg[1], v[1]),
		(tg[7], it[0], it[1], tg[2]),
		(tg[6], it[3], it[2], tg[3]),
		(v[3], tg[5], tg[4], v[2]),
	)


def _eval_bezier_patch(grid, s, t):
	bs = _cubic_bernstein(s)
	bt = _cubic_bernstein(t)
	x = y = z = 0.0
	for i in range(4):
		wi = bs[i]
		if wi == 0.0:
			continue
		row = grid[i]
		for j in range(4):
			w = wi * bt[j]
			if w == 0.0:
				continue
			cp = row[j]
			x += cp.x * w
			y += cp.y * w
			z += cp.z * w
	return x, y, z


def build_zone_tessellated_geom(zone) -> GeomNode:
	"""Detailed zone mesh: each patch tessellated on a fixed
	(order_s+1) x (order_t+1) grid, evaluated via the real Bezier surface
	(_eval_bezier_patch()) -- unlike the earlier low-poly pass (4 raw
	corners only, project-todos/forgery/landscape_editor.md step 3), this
	follows the true curved surface, fixing the "sunken quad" gaps a flat
	4-corner approximation showed on steep relief. A patch with no stored
	tile grid (order_s/order_t == 0, patch version < 2) falls back to a
	single 1x1 cell -- still Bezier-evaluated at its 4 corners+midpoints
	exactly as any other grid size would be, just coarser.

	Colored per vertex by elevation (green gradient, see
	_elevation_color()) -- see build_zone_tessellated_geom's callers for the
	"2D projected" top-down view, which reuses this exact mesh."""
	bb = zone.zone_bb
	min_z = bb.center.z - bb.half_size.z
	max_z = bb.center.z + bb.half_size.z

	# Pass 1: how many vertices this zone needs in total, so
	# GeomVertexData.set_num_rows() can reserve it upfront instead of
	# growing repeatedly (real zones run dozens of patches, each up to
	# 17x17 vertices at the 16x16-segment cap).
	patch_grid_sizes = []
	total_vertices = 0
	for patch in zone.patchs:
		n_s = max(patch.order_s, _MIN_GRID_SEGMENTS)
		n_t = max(patch.order_t, _MIN_GRID_SEGMENTS)
		patch_grid_sizes.append((n_s, n_t))
		total_vertices += (n_s + 1) * (n_t + 1)

	vdata = GeomVertexData("zone-tessellated", GeomVertexFormat.get_v3c4(), Geom.UH_static)
	vdata.set_num_rows(total_vertices)
	vertex_writer = GeomVertexWriter(vdata, "vertex")
	color_writer = GeomVertexWriter(vdata, "color")

	triangles = GeomTriangles(Geom.UH_static)
	base = 0
	for patch, (n_s, n_t) in zip(zone.patchs, patch_grid_sizes):
		grid = _bezier_control_grid(patch, zone.patch_bias, zone.patch_scale)
		stride = n_t + 1
		for i in range(n_s + 1):
			s = i / n_s
			for j in range(n_t + 1):
				t = j / n_t
				x, y, z = _eval_bezier_patch(grid, s, t)
				vertex_writer.add_data3(x, y, z)
				color_writer.add_data4(*_elevation_color(z, min_z, max_z))

		for i in range(n_s):
			for j in range(n_t):
				i0 = base + i * stride + j
				i1 = i0 + 1
				i2 = i0 + stride
				i3 = i2 + 1
				triangles.add_vertices(i0, i2, i1)
				triangles.add_vertices(i1, i2, i3)
		base += (n_s + 1) * stride
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("zone-tessellated")
	node.add_geom(geom)
	return node
