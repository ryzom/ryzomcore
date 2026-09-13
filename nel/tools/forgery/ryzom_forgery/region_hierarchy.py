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

"""Continent -> region hierarchy for the Landscape Editor's lazy-by-region
zone loading (project-todos/forgery/landscape_editor__region_management.md).

Two independent sources, one per app mode (never one falling back to the
other): Visualisation reads `world.lua`, embedded in `gamedev.bnp` under
`live_data_path`; Édition reads `ryzom-data/leveldesign/world/world.json`
directly -- the same `{name: [visible, points, children_or_type]}` structure
(`pynel.region_export.JsonNode`) either way, just JSON instead of Lua
(`pynel.region_export.to_lua()`'s own docstring: byte-identical hierarchy,
different serialization). Both are read-only here -- world.json is never
written by Atyscape at this stage (Nuno, 2026-09-13: that's a separate,
not-yet-planned chantier, once primitive editing is tackled). Each source is
parsed once per process and cached, like `region_loader.py`'s own zone
index.
"""

import json
from pathlib import Path
from typing import Dict, List, NamedTuple, Union

from pynel.ryzom_bnp import BnpError, BnpReader
from pynel.region_export import JsonNode, parse_world_lua, WorldLuaParseError

from .zone_geometry import ZONE_CELL_SIZE


class Region(NamedTuple):
	name: str
	points: List[List[int]]


# str(live_data_path) -> {continent world.lua key: JsonNode} -- the full
# parsed `game.World` table, cached once per live_data_path.
_world_cache: Dict[str, Dict[str, JsonNode]] = {}

# str(ryzom_data_path) -> same structure, parsed from world.json instead.
_world_json_cache: Dict[str, Dict[str, JsonNode]] = {}


def _load_world(live_data_path: Union[str, Path]) -> Dict[str, JsonNode]:
	key = str(live_data_path)
	cached = _world_cache.get(key)
	if cached is not None:
		return cached
	try:
		text = BnpReader(Path(live_data_path) / "gamedev.bnp").read_file("world.lua").decode("latin-1")
		regions = parse_world_lua(text)
	except (BnpError, WorldLuaParseError, OSError):
		regions = {}
	_world_cache[key] = regions
	return regions


def _load_world_json(ryzom_data_path: Union[str, Path]) -> Dict[str, JsonNode]:
	key = str(ryzom_data_path)
	cached = _world_json_cache.get(key)
	if cached is not None:
		return cached
	try:
		text = (Path(ryzom_data_path) / "leveldesign" / "world" / "world.json").read_text()
		regions = json.loads(text)
	except (OSError, json.JSONDecodeError):
		regions = {}
	_world_json_cache[key] = regions
	return regions


def _regions_from_world(world: Dict[str, JsonNode], pipeline_continent_name: str) -> List[Region]:
	"""The immediate region children of `pipeline_continent_name` within an
	already-loaded `world` table (either source, see this module's own
	docstring), or `[]` if the continent has no entry, or its entry has no
	region-level children at all (childless continents like
	"continent_indoors" render as an empty `[]`/`{}` third element, see
	pynel.region_export.primzone_to_json()'s docstring) -- both cases mean
	the caller should fall back to the pre-region_management behaviour
	(load every zone immediately, no purple-square/checkbox UI)."""
	# "newbieland" is the one continent whose key has no "continent_"
	# prefix -- confirmed 2026-09-13 against the real gamedev.bnp's key
	# list (world.json shares the exact same keys, same source hierarchy).
	node = world.get(f"continent_{pipeline_continent_name}")
	if node is None:
		node = world.get(pipeline_continent_name)
	if node is None:
		return []
	children = node[2]
	if not isinstance(children, dict) or not children:
		return []
	# "pvp_zone_*" entries (e.g. "pvp_zone_ichor", "pvp_zone_nexus") sit at
	# the exact same hierarchy level as real regions but aren't one --
	# they're a PvP-flagged overlay, typically covering the same ground as
	# a real region rather than a distinct place of its own (found 2026-09-13,
	# Nuno: "dans bagne, pvp_zone_ichor n'est pas une région" -- confirmed
	# against every continent's own world.lua: every genuine region is
	# "region_*", with no exception, across all 24 continents checked).
	return [
		Region(name=name, points=child[1]) for name, child in children.items()
		if not name.startswith("pvp_zone_")
	]


def get_regions_for_continent(live_data_path: Union[str, Path], pipeline_continent_name: str) -> List[Region]:
	"""Visualisation mode -- see this module's own docstring."""
	return _regions_from_world(_load_world(live_data_path), pipeline_continent_name)


def get_regions_for_continent_edition(ryzom_data_path: Union[str, Path], pipeline_continent_name: str) -> List[Region]:
	"""Édition mode -- see this module's own docstring."""
	return _regions_from_world(_load_world_json(ryzom_data_path), pipeline_continent_name)


def _point_in_polygon(x: float, y: float, points: List[List[int]]) -> bool:
	"""Standard ray-casting test (odd-even rule) -- `points` is a `world.lua`
	region polygon, already in world units, not necessarily closed (last
	point != first point, handled by wrapping via modulo below)."""
	inside = False
	n = len(points)
	for i in range(n):
		x1, y1 = points[i]
		x2, y2 = points[(i + 1) % n]
		if (y1 > y) != (y2 > y):
			x_at_y = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
			if x < x_at_y:
				inside = not inside
	return inside


def assign_zones_to_regions(zone_positions: Dict[str, "tuple"], regions: List[Region]) -> Dict[str, str]:
	"""`{zone name: region name}` for every zone in `zone_positions`
	(`{zone name: (origin_x, origin_y)}`, `region_loader.ZoneRef.x`/`.y`)
	whose 160x160 footprint center falls inside one of `regions`'s polygons
	-- a zone outside every region's polygon is simply absent from the
	result (project-todos/forgery/landscape_editor__region_management.md's
	own rule: it stays a permanent purple square, never assignable to any
	checkbox). A zone matching more than one region's polygon (overlapping
	regions) keeps the first match in `regions`' own order."""
	half = ZONE_CELL_SIZE / 2
	assignment: Dict[str, str] = {}
	for name, (origin_x, origin_y) in zone_positions.items():
		center_x = origin_x + half
		center_y = origin_y + half
		for region in regions:
			if _point_in_polygon(center_x, center_y, region.points):
				assignment[name] = region.name
				break
	return assignment
