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

"""Pure Panda3D geometry-building helpers for `.primitive` flora placement
points (project-todos/forgery/landscape_editor__primitives_view.md) -- first
version, visualisation only (a simple point marker per `CPrimPoint`, not the
real `.shape`/`.plant` -- Nuno 2026-09-15: "on va commencer juste par
afficher des points"). Mirrors zone_geometry.py/ig_geometry.py's "no app
dependency" rule: the caller (landscape_editor.py) does all Panda3D
scene-graph attachment, this module only turns already-loaded pynel data
into geometry.
"""

import numpy as np
from panda3d.core import Geom, GeomEnums, GeomNode, GeomPoints, GeomVertexData, GeomVertexFormat

from pynel.ryzom_primitive import get_property, load_primitive, PrimPoint

# A raw GeomPoints primitive renders as 1px dots regardless of distance --
# too small to spot at the scale a whole continent's flora is viewed at, so
# every marker NodePath built here gets this thickness (viewport pixels,
# same convention as Panda3D's own set_render_mode_thickness()).
MARKER_THICKNESS = 6.0

_FLORA_MARKER_COLOR = (1.0, 0.3, 0.85, 1.0)  # bright pink -- distinct from every other overlay color already in use
_DIFF_MISSING_COLOR = (0.2, 1.0, 0.3, 1.0)  # green -- in .primitive, no matching .ig instance (landscape_editor__flora_ig_import.md)
_DIFF_EXTRA_COLOR = _FLORA_MARKER_COLOR  # pink -- in .ig, no matching .primitive point (same as the plain/no-diff color)


def marker_color_for_point(point) -> tuple:
	"""(r, g, b, a) for `point`'s own `diff` property (Nuno 2026-09-15):
	`"missing"` -> green, `"extra"` -> pink, absent (no diff info, e.g. every
	continent without a measured `.ig` discrepancy) -> the plain default
	pink, same as before this distinction existed."""
	diff = get_property(point, "diff")
	if diff == "missing":
		return _DIFF_MISSING_COLOR
	if diff == "extra":
		return _DIFF_EXTRA_COLOR
	return _FLORA_MARKER_COLOR


def flora_points_from_primitive(path) -> list:
	"""Every `CPrimPoint` with `class="prim"` in the `.primitive` file at
	`path` -- same filter as `pynel.flora_primitives_split.scan_flora_points()`,
	just for a single already-known file instead of a recursive directory
	scan."""
	points = []

	def walk(prim):
		if isinstance(prim, PrimPoint) and get_property(prim, "class") == "prim":
			points.append(prim)
		for child in prim.children:
			walk(child)

	primitive_file = load_primitive(path)
	walk(primitive_file.root)
	return points


def build_flora_markers_geom(points) -> GeomNode:
	"""One point-primitive marker per `PrimPoint` in `points`, batched into a
	single GeomNode the same way zone_geometry.build_zone_placeholders_geom()
	batches its own quads -- a continent's flora routinely has tens of
	thousands of points at once (zorai: 55270, tryker: 48263, both
	verified 2026-09-15 against real data)."""
	points = list(points)
	n = len(points)
	vdata = GeomVertexData("flora-markers", GeomVertexFormat.get_v3c4(), Geom.UH_static)
	vdata.set_num_rows(n)
	vertex_array = vdata.modify_array(0)

	vertex_buf = np.empty(n, dtype=[("vertex", "<f4", 3), ("color", "u1", 4)])
	for i, point in enumerate(points):
		vertex_buf["vertex"][i] = (point.point.x, point.point.y, point.point.z)
		vertex_buf["color"][i] = tuple(round(c * 255) for c in marker_color_for_point(point))
	vertex_array.modify_handle().set_data(vertex_buf.tobytes())

	primitive = GeomPoints(Geom.UH_static)
	primitive.set_index_type(GeomEnums.NT_uint32)
	index_array = primitive.modify_vertices()
	index_array.set_num_rows(n)
	index_array.modify_handle().set_data(np.arange(n, dtype="<u4").tobytes())
	primitive.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(primitive)
	node = GeomNode("flora-markers")
	node.add_geom(geom)
	return node
