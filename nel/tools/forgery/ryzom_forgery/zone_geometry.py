"""Pure Panda3D geometry-building helpers for `.zone`/`.zonew`/`.zonel`
landscape data (`pynel.ryzom_zone.Zone`) -- used by the Landscape Editor
(project-todos/forgery/landscape_editor.md).

Like apps/object_editor_mixins/geometry_helpers.py, has ZERO dependency on
any app module.
"""

import numpy as np
from panda3d.core import Geom, GeomEnums, GeomNode, GeomTriangles, GeomVertexData, GeomVertexFormat

from pynel.ryzom_zone import unpack_vertex

from .zone_cache import PatchPositions, ZoneCacheData

# Above this many vertices in a single zone, a uint16 triangle index would
# overflow -- switch to uint32 (see build_zone_geom_from_cache()). Real zones
# are nowhere near this (max 16x16 segments/patch, dozens of patches), but a
# silent index overflow would scramble the mesh rather than fail loudly, so
# it's checked rather than assumed.
_MAX_UINT16_VERTICES = 65535

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


def _elevation_colors_uint8(z, min_z, max_z):
	"""Elevation gradient (green, low -> high), vectorized: `z` is a numpy
	array (one zone's worth of vertex Z's), returns an (N, 4) uint8 array
	ready to write
	straight into a v3c4 GeomVertexData's "color" column. Truncates (not
	rounds) `component * 255.0`, matching GeomVertexWriter.add_data4()'s own
	float->uint8 packing exactly (verified byte-for-bit, see
	build_zone_geom_from_cache())."""
	span = max_z - min_z
	if span <= 0.0:
		t = np.full_like(z, 0.5)
	else:
		t = np.clip((z - min_z) / span, 0.0, 1.0)
	low = np.array(_LOW_COLOR)
	high = np.array(_HIGH_COLOR)
	colors = low + (high - low) * t[:, None]
	return (colors * 255.0).astype(np.uint8)


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


def compute_zone_patch_positions(zone) -> tuple:
	"""The expensive part of rendering a zone -- Bezier-evaluated positions
	only (`_eval_bezier_patch()`), no color/triangle indices (those stay
	derived at GeomNode-build time, see build_zone_geom_from_cache()'s own
	docstring). Each patch tessellated on a fixed (order_s+1) x (order_t+1)
	grid (a patch with no stored tile grid -- order_s/order_t == 0, patch
	version < 2 -- falls back to a single 1x1 cell, still Bezier-evaluated
	at its 4 corners+midpoints exactly as any other grid size would be, just
	coarser), positions flattened row-major (i in range(n_s+1): j in
	range(n_t+1)) to match zone_cache.PatchPositions's own layout.

	Shared by build_zone_tessellated_geom() (wraps this straight into
	build_zone_geom_from_cache()) and the disk-cache writer
	(project-todos/forgery/landscape_editor__zone_disk_cache.md step 4) --
	only one place ever evaluates CBezierPatch::eval(), so the live render
	path and what gets cached to disk can never drift apart."""
	patches = []
	for patch in zone.patchs:
		n_s = max(patch.order_s, _MIN_GRID_SEGMENTS)
		n_t = max(patch.order_t, _MIN_GRID_SEGMENTS)
		grid = _bezier_control_grid(patch, zone.patch_bias, zone.patch_scale)
		positions = []
		for i in range(n_s + 1):
			s = i / n_s
			for j in range(n_t + 1):
				t = j / n_t
				x, y, z = _eval_bezier_patch(grid, s, t)
				positions.append(x)
				positions.append(y)
				positions.append(z)
		patches.append(PatchPositions(n_s=n_s, n_t=n_t, positions=tuple(positions)))
	return tuple(patches)


def zone_to_cache_data(zone) -> ZoneCacheData:
	"""Converts a parsed pynel Zone into the same ZoneCacheData shape used by
	the disk cache (zone_cache.py) -- positions via
	compute_zone_patch_positions(), bb straight from zone.zone_bb. Shared by
	build_zone_tessellated_geom() (render path) and the disk-cache writer
	(region_loader.load_zone_cache_data(), project-todos/forgery/
	landscape_editor__zone_disk_cache.md step 4), so both ways of getting a
	ZoneCacheData from a real zone agree by construction."""
	bb = zone.zone_bb
	return ZoneCacheData(
		patches=compute_zone_patch_positions(zone),
		bb_center=(bb.center.x, bb.center.y, bb.center.z),
		bb_half_size=(bb.half_size.x, bb.half_size.y, bb.half_size.z),
	)


def build_zone_tessellated_geom(zone) -> GeomNode:
	"""Detailed zone mesh: each patch tessellated on a fixed
	(order_s+1) x (order_t+1) grid, evaluated via the real Bezier surface
	(compute_zone_patch_positions()) -- unlike the earlier low-poly pass (4
	raw corners only, project-todos/forgery/landscape_editor.md step 3), this
	follows the true curved surface, fixing the "sunken quad" gaps a flat
	4-corner approximation showed on steep relief.

	Colored per vertex by elevation (green gradient, see
	_elevation_colors_uint8()) -- see build_zone_tessellated_geom's callers for the
	"2D projected" top-down view, which reuses this exact mesh. Just wraps
	zone_to_cache_data() and delegates the actual GeomNode construction to
	build_zone_geom_from_cache(), so a freshly-computed zone and one rebuilt
	from the disk cache always render identically."""
	return build_zone_geom_from_cache(zone_to_cache_data(zone))


def build_zone_geom_from_cache(cache_data: ZoneCacheData) -> GeomNode:
	"""Same output as build_zone_tessellated_geom() (color by elevation,
	same triangle winding) but rebuilt straight from already-tessellated
	positions (zone_cache.py, project-todos/forgery/
	landscape_editor__zone_disk_cache.md step 2) -- never touches the source
	`.zone*`/`.bnp` again, no Bezier evaluation. Color/indices stay derived
	here rather than cached (see zone_cache.py's own docstring) so future
	rendering changes (normals for lighting, UV for texturing) never
	invalidate the disk cache.

	Writes vertex and index data in bulk (numpy, one `set_data()` call per
	array) instead of one GeomVertexWriter/GeomPrimitive call per vertex/
	triangle -- found 2026-09-08 (Nuno) that those per-call Python/C++
	binding crossings, not the color/index math itself, were the actual
	remaining bottleneck once the disk cache removed bnp read/parsing/Bezier
	eval (project-todos/forgery/landscape_editor__zone_disk_cache.md step
	7)."""
	min_z = cache_data.bb_center[2] - cache_data.bb_half_size[2]
	max_z = cache_data.bb_center[2] + cache_data.bb_half_size[2]

	total_vertices = sum((patch.n_s + 1) * (patch.n_t + 1) for patch in cache_data.patches)

	vdata = GeomVertexData("zone-tessellated", GeomVertexFormat.get_v3c4(), Geom.UH_static)
	vdata.set_num_rows(total_vertices)
	vertex_array = vdata.modify_array(0)
	array_format = vertex_array.get_array_format()
	# The raw byte layout assumed below (vertex float32x3 @0, color uint8x4
	# @12, 16-byte stride) is v3c4's actual layout on every Panda3D version
	# checked -- asserted here so a future mismatch fails loudly instead of
	# silently scrambling every zone's geometry.
	vertex_column = array_format.get_column("vertex")
	color_column = array_format.get_column("color")
	assert array_format.get_stride() == 16
	assert vertex_column.get_start() == 0 and vertex_column.get_num_components() == 3
	assert color_column.get_start() == 12 and color_column.get_num_components() == 4

	vertex_buf = np.empty(total_vertices, dtype=[("vertex", "<f4", 3), ("color", "u1", 4)])
	patch_index_arrays = []
	base = 0
	for patch in cache_data.patches:
		stride = patch.n_t + 1
		n = (patch.n_s + 1) * stride
		positions = np.array(patch.positions, dtype="<f4").reshape(n, 3)
		vertex_buf["vertex"][base:base + n] = positions
		vertex_buf["color"][base:base + n] = _elevation_colors_uint8(positions[:, 2], min_z, max_z)

		i_idx, j_idx = np.meshgrid(np.arange(patch.n_s), np.arange(patch.n_t), indexing="ij")
		i0 = base + i_idx * stride + j_idx
		i1 = i0 + 1
		i2 = i0 + stride
		i3 = i2 + 1
		# Two triangles per grid cell, kept adjacent in (i, j) order -- same
		# winding and ordering as the original per-vertex loop, just built
		# without a Python-level call per triangle.
		cell_triangles = np.stack([np.stack([i0, i2, i1], axis=-1), np.stack([i1, i2, i3], axis=-1)], axis=-2)
		patch_index_arrays.append(cell_triangles.reshape(-1, 3))

		base += n

	all_indices = np.concatenate(patch_index_arrays, axis=0) if patch_index_arrays else np.empty((0, 3), dtype=int)
	index_dtype = "<u2" if total_vertices <= _MAX_UINT16_VERTICES else "<u4"
	index_type = GeomEnums.NT_uint16 if total_vertices <= _MAX_UINT16_VERTICES else GeomEnums.NT_uint32
	flat_indices = all_indices.astype(index_dtype).reshape(-1)

	vertex_array.modify_handle().set_data(vertex_buf.tobytes())

	triangles = GeomTriangles(Geom.UH_static)
	triangles.set_index_type(index_type)
	index_array = triangles.modify_vertices()
	index_array.set_num_rows(len(flat_indices))
	index_array.modify_handle().set_data(flat_indices.tobytes())
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	node = GeomNode("zone-tessellated")
	node.add_geom(geom)
	return node
