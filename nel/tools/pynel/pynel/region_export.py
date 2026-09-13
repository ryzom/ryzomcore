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

"""Exports the continent/region/place hierarchy from `region_<continent>.primitive`
files (project-todos/pynel/region_places_export.md) to the compact nested-array
JSON format Nuno already uses in his own `world.json`.

These `.primitive` files (LIGO primitive tree, `pynel.ryzom_primitive`) live
in the private `ryzom-private-data` repository (leveldesign authoring
source), under `primitives/**/region_*.primitive`. A file's `CPrimNode` root
holds one or more sibling `PrimZone class="continent"` nodes (e.g.
`region_fyros.primitive` has `continent_fyros`, `continent_fyros_newbie` AND
`continent_fyros_islands` side by side -- confirmed 2026-09-13), each
containing `PrimZone class="region"` children, each of which directly
contains `PrimZone class="place"` leaves.

Verified field mapping (2026-09-13):
- `class`: "continent"/"region"/"place" -- the level in the hierarchy.
- `name`: becomes the JSON key.
- `displayed`: "true"/"false", optional (absent = visible) -- this is what
  becomes the leading `visible` boolean, NOT the unrelated `hidden` property
  (a LIGO editor cosmetic flag, always empty in every example seen).
- `place_type` (only on `class="place"` nodes): "Capital"/"stable"/"Outpost"/
  "Village"/"Street"/absent (default "place") -- becomes the 3rd array
  element for a leaf, instead of a dict of children for continent/region.
- `PrimAlias` children ("class"="alias") carry no useful data here and are
  skipped -- only `PrimZone` children matter.
"""

import argparse
import glob
import json
import re
from pathlib import Path
from typing import Dict, List, Tuple, Union

from . import repository_paths
from .ryzom_primitive import get_property, load_primitive, PrimZone

JsonNode = list  # [visible: bool, points: List[List[float]], children_or_type: Union[dict, str]]


def primzone_to_json(prim: PrimZone) -> JsonNode:
	"""Converts one `PrimZone` (and recursively its `PrimZone` children) to
	the `[visible, points, children_or_type]` structure described in this
	module's own docstring. The leaf/branch distinction is the node's own
	`class` property ("place" = leaf), NOT whether it happens to have
	`PrimZone` children -- confirmed 2026-09-13 against a real childless
	continent (`continent_indoors`, no `region` children at all): its
	`world.json` entry is `[visible, points, []]` (an EMPTY LIST, not a
	`place_type` string), so a childless continent/region must still
	produce a (possibly empty) children container, never be mistaken for a
	leaf place just because it has nothing under it yet."""
	visible = get_property(prim, "displayed") != "false"
	# int(), NOT round(): confirmed 2026-09-13 against Nuno's own world.json
	# (e.g. raw x=18993.566406 -> world.json has 18993, not round()'s 18994)
	# that coordinates are truncated toward zero, matching a plain C++
	# float-to-int cast, not rounded to the nearest integer.
	points = [[int(pt.x), int(pt.y)] for pt in prim.points]

	if get_property(prim, "class") == "place":
		place_type = get_property(prim, "place_type") or "place"
		return [visible, points, place_type]

	child_zones = [child for child in prim.children if isinstance(child, PrimZone)]
	if not child_zones:
		return [visible, points, []]

	children = {}
	for child in child_zones:
		name = get_property(child, "name")
		children[name] = primzone_to_json(child)
	return [visible, points, children]


def build_world_regions(ryzom_private_data_path: Union[str, Path]) -> Dict[str, JsonNode]:
	"""Scans every `primitives/**/region_*.primitive` file under
	`ryzom_private_data_path` and returns the merged top-level dict --
	`{continent_root_name: primzone_to_json(continent_root)}` for EVERY
	sibling `PrimZone class="continent"` root of EVERY file (not just the
	first child of each file's `CPrimNode` root -- see this module's own
	docstring: a single file routinely holds several, e.g.
	`region_r2.primitive`'s 5 R2 sub-continents)."""
	pattern = str(Path(ryzom_private_data_path) / "primitives" / "**" / "region_*.primitive")
	regions: Dict[str, JsonNode] = {}
	for path in sorted(glob.glob(pattern, recursive=True)):
		primitive_file = load_primitive(path)
		for root in primitive_file.root.children:
			if not isinstance(root, PrimZone):
				continue
			name = get_property(root, "name")
			regions[name] = primzone_to_json(root)
	return regions


def _lua_points(points) -> str:
	return "{ " + ", ".join(f"{{ {x}, {y} }}" for x, y in points) + " }"


# A bare Lua identifier can't start with a digit or contain spaces/accented
# characters/punctuation -- confirmed 2026-09-13 against real names in
# `world.lua` that don't qualify (e.g. "Karavan Embassy", "endroit_résidu_oeuf",
# "region_fyros_island1PvP 3"): Lua (and the real file) falls back to the
# bracketed-string key form `["name"] = ...` for those, never a bare
# `name = ...`.
_LUA_IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


def _lua_key(name: str) -> str:
	if _LUA_IDENT_RE.match(name):
		return name
	return f'["{name}"]'


def _lua_node(name: str, node: JsonNode, indent: str, is_last: bool) -> str:
	"""Renders one `name = { visible, points, children_or_type }` (or
	`["name"] = { ... }`, see _lua_key()) entry -- recursively, for a branch
	(dict children) -- matching the real `world.lua`'s exact layout
	(verified 2026-09-13): a leaf ("place", `children_or_type` is a string)
	always fits on one line; a branch opens its children table `{` at the
	end of its own line, one child per line indented two further spaces,
	and closes with `} }` (children table + this node's own table) back at
	its own indent -- an empty branch (no children at all, e.g. a continent
	with no `region`) renders inline as `{ }`, never as a multi-line empty
	table."""
	key = _lua_key(name)
	visible, points, third = node
	visible_str = "true" if visible else "false"
	comma = "" if is_last else ","
	if isinstance(third, str):
		return f'{indent}{key} = {{ {visible_str}, {_lua_points(points)}, "{third}" }}{comma}'
	if not third:
		return f'{indent}{key} = {{ {visible_str}, {_lua_points(points)}, {{ }} }}{comma}'

	lines = [f'{indent}{key} = {{ {visible_str}, {_lua_points(points)}, {{']
	items = list(third.items())
	for i, (child_name, child_node) in enumerate(items):
		lines.append(_lua_node(child_name, child_node, indent + "  ", i == len(items) - 1))
	lines.append(f'{indent}}} }}{comma}')
	return "\n".join(lines)


def to_lua(regions: Dict[str, JsonNode]) -> str:
	"""Renders `regions` (build_world_regions()'s own output) as a `.lua`
	file in the exact layout of the real `world.lua` (verified 2026-09-13
	against `ryzom-data/world.lua`, byte-identical to the real client's own
	`live_data/gamedev/world.lua`) -- EXCEPT the trailing `-- VERSION --
	/ FILE_WORLD_VERSION = <n>` footer, deliberately omitted: that number is
	the real client build's own version, which a regeneration from
	`ryzom-private-data` has no way to know or reproduce (Nuno's decision,
	2026-09-13)."""
	lines = ["if (game==nil) then", "\tgame= {};", "end", "", "game.World = {"]
	items = list(regions.items())
	for i, (name, node) in enumerate(items):
		lines.append(_lua_node(name, node, "  ", i == len(items) - 1))
	lines.append("}")
	return "\n".join(lines)


class WorldLuaParseError(Exception):
	pass


# Tokenizer for EXACTLY the grammar to_lua() itself emits (identifiers,
# integers, quoted strings, {}/,/= punctuation) -- not a general Lua lexer
# (no floats, no escapes beyond \" inside a string, no other Lua syntax).
_TOKEN_RE = re.compile(r"""
	(?P<NUMBER>-?\d+)
	| (?P<STRING>"(?:[^"\\]|\\.)*")
	| (?P<IDENT>[A-Za-z_][A-Za-z0-9_]*)
	| (?P<LBRACE>\{)
	| (?P<RBRACE>\})
	| (?P<LBRACKET>\[)
	| (?P<RBRACKET>\])
	| (?P<COMMA>,)
	| (?P<EQUALS>=)
	| (?P<SKIP>\s+)
""", re.VERBOSE)


def _tokenize(text: str) -> List[Tuple[str, str]]:
	text = re.sub(r"--[^\n]*", "", text)  # strip `-- ...` line comments
	tokens = []
	pos = 0
	while pos < len(text):
		match = _TOKEN_RE.match(text, pos)
		if match is None:
			raise WorldLuaParseError(f"unexpected character at offset {pos}: {text[pos:pos + 20]!r}")
		pos = match.end()
		kind = match.lastgroup
		if kind != "SKIP":
			tokens.append((kind, match.group()))
	return tokens


class _WorldLuaParser:
	"""Recursive-descent parser over `_tokenize()`'s output -- mirrors
	`to_lua()`'s own grammar exactly (see that function's docstring), so
	this is its inverse: `parse_world_lua(to_lua(regions)) == regions` for
	any `regions` `build_world_regions()` can produce."""

	def __init__(self, tokens: List[Tuple[str, str]]):
		self.tokens = tokens
		self.pos = 0

	def _peek(self) -> Tuple[str, str]:
		return self.tokens[self.pos] if self.pos < len(self.tokens) else (None, None)

	def _advance(self) -> Tuple[str, str]:
		token = self._peek()
		self.pos += 1
		return token

	def _expect(self, kind: str) -> str:
		actual_kind, value = self._advance()
		if actual_kind != kind:
			raise WorldLuaParseError(f"expected {kind}, got {actual_kind} {value!r} at token {self.pos}")
		return value

	def parse_table(self) -> Dict[str, JsonNode]:
		"""`{ name = node, name = node, ... }` -- a continent/region's own
		children table, or the top-level `game.World` table itself."""
		self._expect("LBRACE")
		result: Dict[str, JsonNode] = {}
		if self._peek()[0] == "RBRACE":
			self._advance()
			return result
		while True:
			# A key is either a bare identifier or, when the real name isn't
			# a valid one (spaces/accents/punctuation -- see _lua_key()),
			# `["name"]` (real examples: "Karavan Embassy",
			# "endroit_résidu_oeuf", "region_fyros_island1PvP 3").
			if self._peek()[0] == "LBRACKET":
				self._advance()
				name = self._expect("STRING")[1:-1]
				self._expect("RBRACKET")
			else:
				name = self._expect("IDENT")
			self._expect("EQUALS")
			result[name] = self.parse_node()
			if self._peek()[0] == "COMMA":
				self._advance()
				continue
			break
		self._expect("RBRACE")
		return result

	def parse_node(self) -> JsonNode:
		"""`{ true/false, { points }, third }`."""
		self._expect("LBRACE")
		visible = self._parse_bool()
		self._expect("COMMA")
		points = self._parse_points()
		self._expect("COMMA")
		third = self._parse_third()
		self._expect("RBRACE")
		return [visible, points, third]

	def _parse_bool(self) -> bool:
		kind, value = self._advance()
		if kind != "IDENT" or value not in ("true", "false"):
			raise WorldLuaParseError(f"expected true/false, got {value!r} at token {self.pos}")
		return value == "true"

	def _parse_points(self) -> List[List[int]]:
		self._expect("LBRACE")
		points = []
		if self._peek()[0] == "RBRACE":
			self._advance()
			return points
		while True:
			self._expect("LBRACE")
			x = int(self._expect("NUMBER"))
			self._expect("COMMA")
			y = int(self._expect("NUMBER"))
			self._expect("RBRACE")
			points.append([x, y])
			if self._peek()[0] == "COMMA":
				self._advance()
				continue
			break
		self._expect("RBRACE")
		return points

	def _parse_third(self) -> Union[str, dict, list]:
		kind, value = self._peek()
		if kind == "STRING":
			self._advance()
			return value[1:-1]
		# A branch's children table -- to_lua()'s own convention: an empty
		# one is `[]`, never `{}`, matching build_world_regions()'s own
		# leaf/branch distinction (see primzone_to_json()'s docstring).
		children = self.parse_table()
		return children if children else []


def parse_world_lua(text: str) -> Dict[str, JsonNode]:
	"""Reads a `world.lua` (or anything `to_lua()` produced) back into the
	same `{name: [visible, points, children_or_type]}` structure
	`build_world_regions()` builds -- the inverse of `to_lua()`. Everything
	before `game.World =` (the `if (game==nil) then ... end` guard) and
	after the table's own closing `}` (the real file's trailing
	`-- VERSION --` / `FILE_WORLD_VERSION = <n>` footer) is ignored."""
	marker = "game.World"
	marker_pos = text.find(marker)
	if marker_pos == -1:
		raise WorldLuaParseError("no 'game.World' table assignment found")
	equals_pos = text.index("=", marker_pos + len(marker))
	tokens = _tokenize(text[equals_pos + 1:])
	return _WorldLuaParser(tokens).parse_table()


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(
		description="Export the continent/region/place hierarchy from ryzom-private-data's region_*.primitive files to world.json or world.lua",
	)
	parser.add_argument("--format", choices=("json", "lua"), default="json", help="output format (default: json)")
	parser.add_argument("--output-dir", type=Path, default=None, help="destination folder for world.<format> (default: <ryzom-data>)")
	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	private_data_path = repository_paths.get("ryzom-private-data")
	if private_data_path is None:
		raise SystemExit("ryzom-private-data repository path not configured (pynel.repository_paths)")

	output_dir = args.output_dir
	if output_dir is None:
		output_dir = repository_paths.get("ryzom-data")
		if output_dir is None:
			raise SystemExit("--output-dir not given and ryzom-data repository path not configured (pynel.repository_paths)")

	regions = build_world_regions(private_data_path)
	output_path = output_dir / f"world.{args.format}"
	if args.format == "lua":
		output_path.write_text(to_lua(regions))
	else:
		output_path.write_text(json.dumps(regions, separators=(",", ":")))
	print(f"wrote {len(regions)} continent(s) to {output_path}")


if __name__ == "__main__":
	_main()
