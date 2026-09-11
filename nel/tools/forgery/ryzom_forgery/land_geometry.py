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

"""Positions `.land` brick cells (`pynel.ryzom_land.ZoneRegion`) that have no
real exported `.zone` yet -- Edition mode fallback for `[POLY]`/`[2D]`
(project-todos/forgery/landscape_editor__land_preview.md step 3): a
leveldesigner can place a brick in a local `.land` before ever running
`land_export`, and this is the only way to preview it.

A brick isn't always a single 160x160 cell -- some are "large pieces"
spanning several cells (e.g. `solprimer-mz_coulea.zone`, 482x162 units,
confirmed 2026-09-10, Nuno). Each cell such a piece occupies references the
SAME brick `zone_name` in the `.land`, but stores a DIFFERENT
`ZoneUnit.pos_x`/`pos_y` -- confusingly named like a grid position but
actually "position in a large piece" (`SZoneUnit::PosX`/`PosY`,
`zone_region.h`): which sub-cell of the piece this particular grid cell
represents, not a world/grid coordinate at all. Naively placing the brick's
full geometry once per referencing cell (as if each were an independent
160x160 tile) duplicates and overlaps the piece across every cell it spans.

`_piece_deltas()` ports `CExport::treatPattern()`'s `deltaX`/`deltaX`
resolution (`ryzom-core/ryzom/tools/leveldesign/world_editor/
land_export_lib/export.cpp:479-498`) to recover the piece's own grid origin
from one referencing cell's grid position + its `pos_x`/`pos_y` within the
piece -- every cell belonging to the same piece resolves to the identical
origin, which is what `find_missing_land_cells()`'s caller uses to render
the piece exactly once. `treatPattern()` gets the piece's size in cells from
a "ZoneBank" tile database this project doesn't parse (out of scope, see
`land_pipeline.md`) -- sidestepped here by deriving it directly from the
already-loaded brick's own bounding box instead (Nuno 2026-09-10: "quand tu
charges un .zone tu dois bien avoir les dimensions").

`_transform_point()` then positions the piece by mirroring/rotating it
**about the center of its own real footprint** (`width x height` from the
brick's bounding box, not a fixed `ZONE_CELL_SIZE`) -- a first attempt
rotated/mirrored about the local (0,0) corner (reading
`CExport::transformZone()`'s `CMatrix` construction literally) and was off
by up to a full cell; centering fixed single-cell bricks but a further
attempt keeping the rotation centered on a fixed 160x160 square (rather than
the piece's true, possibly larger, footprint) still mispositioned rotated
multi-cell pieces. Both corrected 2026-09-10 by comparing computed positions
against every real exported zone of `ryzom-data/pipeline/export/continents/
bagne/` for the matching `.land`, including its two real multi-cell/rotated
pieces (`solprimer-mz_monticulea`/`solprimer-mz_coulea`) -- computed and
real bounding-box centers now agree within ~2 units (float/mesh-shape noise)
on every case tested. The center-based formulas (`x`/`y` in
`[0, width]`/`[0, height]`, mirror first if `flip`, then rotate):

	rot=0: (x, y)
	rot=1: (height - y, x)
	rot=2: (width - x, height - y)
	rot=3: (y, width - x)

Applied directly to each already-Bezier-evaluated vertex position rather
than to the patch's raw control points: a Bezier surface is an affine
combination of its control points (Bernstein basis sums to 1), so an affine
transform commutes with evaluation --
`transform(eval(P0..P15)) == eval(transform(P0)..transform(P15))` -- letting
`zone_geometry.compute_zone_patch_positions()` be reused as-is.
"""

from typing import Dict, Tuple

from pynel.ryzom_land import STRING_UNUSED, ZoneRegion, ZoneUnit

from .zone_cache import PatchPositions, ZoneCacheData
from .zone_geometry import ZONE_CELL_SIZE, compute_zone_patch_positions


def _piece_deltas(pos_x: int, pos_y: int, rot: int, flip: int, size_x: int, size_y: int) -> Tuple[int, int]:
	"""How far the CURRENT cell is offset from the whole piece's own grid
	origin, given this cell's (pos_x, pos_y) "position in a large piece" and
	the piece's rot/flip/size in cells -- port of `CExport::treatPattern()`'s
	deltaX/deltaY switch (export.cpp:479-498). (0, 0) for an ordinary
	single-cell brick (size_x == size_y == 1, pos_x == pos_y == 0 always in
	that case), i.e. a no-op."""
	if not flip:
		if rot == 0:
			return -pos_x, -pos_y
		if rot == 1:
			return -(size_y - 1 - pos_y), -pos_x
		if rot == 2:
			return -(size_x - 1 - pos_x), -(size_y - 1 - pos_y)
		return -pos_y, -(size_x - 1 - pos_x)  # rot == 3
	else:
		if rot == 0:
			return -(size_x - 1 - pos_x), -pos_y
		if rot == 1:
			return -(size_y - 1 - pos_y), -(size_x - 1 - pos_x)
		if rot == 2:
			return -pos_x, -(size_y - 1 - pos_y)
		return -pos_y, -pos_x  # rot == 3


def piece_origin(grid_x: int, grid_y: int, unit: ZoneUnit, size_x: int, size_y: int) -> Tuple[int, int]:
	"""The whole piece's own grid origin (its rot=0/flip=0 top-left cell),
	given ONE cell that references it -- every cell belonging to the same
	piece resolves to the identical origin (verified 2026-09-10 against real
	`.land`/exported-zone data), which is what the caller dedupes on to
	render a multi-cell piece exactly once."""
	delta_x, delta_y = _piece_deltas(unit.pos_x, unit.pos_y, unit.rot, unit.flip, size_x, size_y)
	return grid_x + delta_x, grid_y + delta_y


def brick_size_in_cells(brick_zone) -> Tuple[int, int]:
	"""(size_x, size_y) in whole cells, derived from the brick's own real
	bounding box (never a ZoneBank lookup, see module docstring) -- at least
	1x1 even if the mesh's own bounds fall slightly short of a full cell."""
	bb = brick_zone.zone_bb
	size_x = max(1, round((bb.half_size.x * 2) / ZONE_CELL_SIZE))
	size_y = max(1, round((bb.half_size.y * 2) / ZONE_CELL_SIZE))
	return size_x, size_y


def _transform_point(
	x: float, y: float, z: float, origin_x: int, origin_y: int, rot: int, flip: int, width: float, height: float,
) -> Tuple[float, float, float]:
	if flip:
		x = width - x
	rot = rot % 4
	if rot == 1:
		x, y = height - y, x
	elif rot == 2:
		x, y = width - x, height - y
	elif rot == 3:
		x, y = y, width - x
	return x + origin_x * ZONE_CELL_SIZE, y + origin_y * ZONE_CELL_SIZE, z


def build_land_piece_cache_data(brick_zone, origin_x: int, origin_y: int, rot: int, flip: int) -> ZoneCacheData:
	"""ZoneCacheData for one whole `.land` piece (a single cell is just the
	size_x == size_y == 1 case), positioned at `origin_x`/`origin_y` (its own
	grid origin -- see `piece_origin()`) exactly like `CExport::
	transformZone()`/`treatPattern()` would -- see module docstring.
	`brick_zone` itself is never mutated."""
	bb = brick_zone.zone_bb
	width = bb.half_size.x * 2
	height = bb.half_size.y * 2
	patches = tuple(
		PatchPositions(
			n_s=patch.n_s,
			n_t=patch.n_t,
			positions=tuple(
				coord
				for i in range(0, len(patch.positions), 3)
				for coord in _transform_point(*patch.positions[i:i + 3], origin_x, origin_y, rot, flip, width, height)
			),
		)
		for patch in compute_zone_patch_positions(brick_zone)
	)
	cx, cy, cz = _transform_point(bb.center.x, bb.center.y, bb.center.z, origin_x, origin_y, rot, flip, width, height)
	hx, hy, hz = bb.half_size.x, bb.half_size.y, bb.half_size.z
	if rot % 2 == 1:
		hx, hy = hy, hx
	return ZoneCacheData(patches=patches, bb_center=(cx, cy, cz), bb_half_size=(hx, hy, hz))


def find_missing_land_cells(land: ZoneRegion, existing_cells: set) -> Dict[Tuple[int, int], ZoneUnit]:
	"""{(pos_x, pos_y) -> ZoneUnit} for every used cell of `land`
	(`zone_name != STRING_UNUSED`) whose grid position isn't in
	`existing_cells` (the (pos_x, pos_y) of every zone already loaded from a
	real pipeline export, see its caller) -- these are the cells with no
	real exported `.zone` yet, the only genuine fallback case (Nuno
	2026-09-10: "de base donc, TOUT existe" once data is downloaded -- a
	brick just placed locally in a `.land` is the one case that never has an
	export). Multiple entries here can belong to the same multi-cell piece
	(same `ZoneUnit.zone_name`, different `pos_x`/`pos_y` "position in the
	piece") -- deduplicating those into a single render is the caller's job
	(`piece_origin()`/`build_land_piece_cache_data()` above), not this
	function's, since it doesn't load any brick file itself."""
	width = land.max_x - land.min_x + 1
	missing = {}
	for index, unit in enumerate(land.zones):
		if unit.zone_name == STRING_UNUSED:
			continue
		pos_x = land.min_x + (index % width)
		pos_y = land.min_y + (index // width)
		if (pos_x, pos_y) not in existing_cells:
			missing[(pos_x, pos_y)] = unit
	return missing
