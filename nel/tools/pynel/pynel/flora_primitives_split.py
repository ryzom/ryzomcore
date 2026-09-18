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

"""Splits every World Editor flora placement point (`CPrimPoint` with
`class="prim"`) found across ALL `.primitive` files in `ryzom-private-data`
by the REAL continent it geographically belongs to (project-todos/pynel/
flora_primitives_split_by_continent.md), and writes one `flora_<continent>.
primitive` per continent to `ryzom-data/leveldesign/landscape/`.

Origin (2026-09-15, conversation with Nuno): grouping by source FILE NAME
(e.g. assuming `flora_nexus_minor.primitive` is exhaustively nexus's flora)
is unreliable -- `tryker.primitive` (no "flora"/"flore" in its name) turned
out to contain 48273 real `class="prim"` points, `matis`'s own flora is
split across `temp_flora_verdantheight*.primitive`, etc. Geography (the
point's actual world position, resolved against each continent's real
`.land` zone grid) is the only reliable ground truth.

Usage:
	from pynel import flora_primitives_split as fps
	result = fps.run(ryzom_data_path, ryzom_private_data_path, output_dir)
	print(result.report_text())
"""

import argparse
import csv
import glob
import uuid
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

from . import repository_paths
from .ryzom_land import load_land
from .ryzom_packed_sheets import PackedSheetsParseError, world_pos_to_zone_name
from .ryzom_primitive import get_property, load_primitive, set_property, Primitive, PrimitiveFile, PrimPoint, save_primitive

# Property name Atyscape (project-todos/forgery/landscape_editor.md step 19,
# .ig editing) will read/write to keep a `.ig` instance and its source
# `.primitive` point matched across edits (decision, Nuno 2026-09-15) --
# assigned once here for every point already split by continent, never
# recomputed afterwards (a real UUID4, not a sequential index: stays valid
# even if points are later added/removed/reordered in the file).
_ID_PROPERTY = "id"

_CSV_RELATIVE_PATH = Path("leveldesign") / "world" / "continent_pipeline_reference.csv"
_OUTPUT_RELATIVE_DIR = Path("leveldesign") / "landscape"


class FloraSplitError(Exception):
	pass


def _read_csv_table(csv_path: Union[str, Path]) -> Dict[str, Dict[str, str]]:
	"""Reads `continent_pipeline_reference.csv`'s transposed layout (field
	name in column 0, one column per continent) -- deliberately a standalone
	parser, NOT a reuse of Forgery's `continent_pipeline_reference.py`, so
	this stays a self-contained pynel script (same principle as
	`region_export.py`)."""
	with open(csv_path, newline="", encoding="utf-8") as f:
		rows = list(csv.reader(f))
	if not rows or rows[0][0] != "continent":
		raise FloraSplitError(f"{csv_path}: missing 'continent' header row")
	continents = rows[0][1:]
	table: Dict[str, Dict[str, str]] = {name: {} for name in continents}
	for row in rows[1:]:
		if not row:
			continue
		field_name = row[0]
		for i, continent in enumerate(continents, start=1):
			table[continent][field_name] = row[i] if i < len(row) else ""
	return table


_ZONE_CELL_SIZE = 160.0


def build_zone_continent_index(ryzom_data_path: Union[str, Path]) -> Dict[str, str]:
	"""{zone_name: continent_name}, built by loading every continent's real
	`.land` (via `ryzom_land.load_land()`, `zone_region_file` CSV column)
	and registering each of its USED grid cells (skipping `STRING_UNUSED`).

	The dict is keyed by the REAL, GLOBALLY UNIQUE zone name (e.g. "52_CB"),
	derived from each cell's own (x, y) GRID position via
	`world_pos_to_zone_name()` -- NOT by `ZoneUnit.zone_name` itself, which
	is the name of the reusable BRICK (.zone template) placed at that cell,
	not a position (confirmed 2026-09-15: the same brick, e.g. "jungle-2",
	is placed at many different grid cells across many different
	continents, so keying by it produces thousands of false collisions).
	A cell's (x, y) is NOT stored per-cell either -- `ZoneRegion.zones` is a
	flat row-major array, so it's reconstructed from `min_x`/`min_y` and
	the cell's index, same convention as this module's own usage example.
	Verified 2026-09-15 against real data: nexus's `.land` cell (x=53,
	y=-52) resolves to zone name "52_CB", and `52_CB.zone` really exists in
	`pipeline/export/continents/nexus/zone/`.

	Raises FloraSplitError (listing every conflicting zone_name/continent
	pair at once, not just the first) if the same real zone name is claimed
	by more than one continent -- should never legitimately happen, a real
	world zone belongs to exactly one continent."""
	from .ryzom_land import STRING_UNUSED

	ryzom_data_path = Path(ryzom_data_path)
	table = _read_csv_table(ryzom_data_path / _CSV_RELATIVE_PATH)

	index: Dict[str, str] = {}
	collisions: List[Tuple[str, str, str]] = []
	for continent, row in table.items():
		land_path = ryzom_data_path / row["zone_region_file"]
		region = load_land(land_path)
		width = region.max_x - region.min_x + 1
		for i, unit in enumerate(region.zones):
			if unit.zone_name == STRING_UNUSED:
				continue
			x = region.min_x + i % width
			y = region.min_y + i // width
			zone_name = world_pos_to_zone_name(x * _ZONE_CELL_SIZE, y * _ZONE_CELL_SIZE)
			if zone_name is None:
				continue
			existing = index.get(zone_name)
			if existing is not None and existing != continent:
				collisions.append((zone_name, existing, continent))
			else:
				index[zone_name] = continent

	if collisions:
		lines = [f"  {zone_name!r}: {c1!r} vs {c2!r}" for zone_name, c1, c2 in collisions]
		raise FloraSplitError("zone_name claimed by more than one continent:\n" + "\n".join(lines))
	return index


@dataclass
class FloraPoint:
	node: PrimPoint
	source_file: str


def scan_flora_points(ryzom_private_data_path: Union[str, Path]) -> List[FloraPoint]:
	"""Every `CPrimPoint` with `class="prim"` found recursively in ALL
	`primitives/**/*.primitive` files under `ryzom_private_data_path` --
	NOT filtered by file name (see this module's own docstring for why)."""
	pattern = str(Path(ryzom_private_data_path) / "primitives" / "**" / "*.primitive")
	points: List[FloraPoint] = []

	def walk(prim: Primitive, source_file: str) -> None:
		if isinstance(prim, PrimPoint) and get_property(prim, "class") == "prim":
			points.append(FloraPoint(node=prim, source_file=source_file))
		for child in prim.children:
			walk(child, source_file)

	for path in sorted(glob.glob(pattern, recursive=True)):
		primitive_file = load_primitive(path)
		walk(primitive_file.root, path)
	return points


@dataclass
class SplitResult:
	by_continent: Dict[str, List[FloraPoint]] = field(default_factory=dict)
	unresolved: List[Tuple[FloraPoint, str]] = field(default_factory=list)  # (point, reason)

	def report_text(self) -> str:
		lines = ["=== Flora points by continent ==="]
		for continent in sorted(self.by_continent):
			lines.append(f"  {continent}: {len(self.by_continent[continent])} point(s)")
		lines.append(f"=== {len(self.unresolved)} unresolved point(s) ===")
		for point, reason in self.unresolved:
			x, y = point.node.point.x, point.node.point.y
			lines.append(f"  {point.source_file} @ ({x:.1f}, {y:.1f}): {reason}")
		return "\n".join(lines)


def resolve_continents(points: List[FloraPoint], zone_index: Dict[str, str]) -> SplitResult:
	"""Resolves each point's real continent via `world_pos_to_zone_name()` +
	`zone_index`. Never drops a point silently: one that resolves to no zone
	(outside the valid grid) or to a zone absent from `zone_index` (no
	continent's `.land` claims it) goes to `result.unresolved` instead,
	with the reason, for Nuno to review (project-todos/pynel/
	flora_primitives_split_by_continent.md's own scope decision)."""
	result = SplitResult()
	for point in points:
		x, y = point.node.point.x, point.node.point.y
		try:
			zone_name = world_pos_to_zone_name(x, y)
		except PackedSheetsParseError as exc:
			result.unresolved.append((point, str(exc)))
			continue
		if zone_name is None:
			result.unresolved.append((point, "position outside the valid zone grid"))
			continue
		continent = zone_index.get(zone_name)
		if continent is None:
			result.unresolved.append((point, f"zone {zone_name!r} not claimed by any continent's .land"))
			continue
		result.by_continent.setdefault(continent, []).append(point)
	return result


def build_flora_primitive_file(continent: str, points: List[FloraPoint]) -> PrimitiveFile:
	"""One `flora_<continent>.primitive`, structured like a real existing
	file (`primeroots/flora_nexus_minor.primitive`, inspected 2026-09-15):
	a `CPrimNode` root, one `CPrimNode` child (`class="flora"`,
	`name="flora_<continent>"`), whose own children are the `CPrimPoint`
	nodes themselves (unlike the real file, no `CPrimZone` boundary
	siblings -- those aren't tracked by this script, only the points)."""
	container = Primitive(
		properties={"class": _string_property("flora"), "name": _string_property(f"flora_{continent}")},
		children=[p.node for p in points],
	)
	root = Primitive(children=[container])
	return PrimitiveFile(root=root)


def _string_property(value: str):
	from .ryzom_primitive import PropertyString

	return PropertyString(value=value)


def assign_stable_ids(points: List[FloraPoint]) -> None:
	"""Sets the `id` property (see `_ID_PROPERTY`) on every point that
	doesn't already have one, to a fresh random UUID4 hex string -- mutates
	`point.node` in place. Never overwrites an existing `id` (idempotent:
	running this again on an already-ID'd file changes nothing)."""
	seen = {get_property(p.node, _ID_PROPERTY) for p in points} - {None}
	for point in points:
		if get_property(point.node, _ID_PROPERTY) is not None:
			continue
		new_id = uuid.uuid4().hex
		while new_id in seen:
			new_id = uuid.uuid4().hex
		seen.add(new_id)
		set_property(point.node, _ID_PROPERTY, new_id)


def write_flora_files(output_dir: Union[str, Path], result: SplitResult) -> List[Path]:
	"""Writes one `flora_<continent>.primitive` per resolved continent, plus
	`flora_unused.primitive` (same file, next to the others) holding every
	unresolved point (`result.unresolved`) -- so a point that couldn't be
	geographically assigned is still kept, never silently dropped from the
	output on disk (decision, Nuno 2026-09-15). Every point written also
	gets a stable `id` property (`assign_stable_ids()`) if it doesn't
	already have one."""
	output_dir = Path(output_dir)
	output_dir.mkdir(parents=True, exist_ok=True)
	assign_stable_ids([p for points in result.by_continent.values() for p in points])
	assign_stable_ids([p for p, _reason in result.unresolved])
	written = []
	for continent, points in sorted(result.by_continent.items()):
		path = output_dir / f"flora_{continent}.primitive"
		save_primitive(path, build_flora_primitive_file(continent, points))
		written.append(path)
	if result.unresolved:
		unused_points = [point for point, _reason in result.unresolved]
		path = output_dir / "flora_unused.primitive"
		save_primitive(path, build_flora_primitive_file("unused", unused_points))
		written.append(path)
	return written


@dataclass
class RunResult:
	split: SplitResult
	written_files: List[Path]

	def report_text(self) -> str:
		lines = [self.split.report_text(), "", f"=== {len(self.written_files)} file(s) written ==="]
		for path in self.written_files:
			lines.append(f"  {path}")
		return "\n".join(lines)


def run(
	ryzom_data_path: Union[str, Path],
	ryzom_private_data_path: Union[str, Path],
	output_dir: Optional[Union[str, Path]] = None,
) -> RunResult:
	if output_dir is None:
		output_dir = Path(ryzom_data_path) / _OUTPUT_RELATIVE_DIR
	zone_index = build_zone_continent_index(ryzom_data_path)
	points = scan_flora_points(ryzom_private_data_path)
	split = resolve_continents(points, zone_index)
	written = write_flora_files(output_dir, split)
	return RunResult(split=split, written_files=written)


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(
		description="Split World Editor flora primitive points by their real continent and write flora_<continent>.primitive files",
	)
	parser.add_argument("--output-dir", type=Path, default=None, help="destination folder (default: <ryzom-data>/leveldesign/landscape)")
	parser.add_argument("--report-file", type=Path, default=None, help="also write the report to this file")
	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	ryzom_data_path = repository_paths.get("ryzom-data")
	if ryzom_data_path is None:
		raise SystemExit("ryzom-data repository path not configured (pynel.repository_paths)")
	ryzom_private_data_path = repository_paths.get("ryzom-private-data")
	if ryzom_private_data_path is None:
		raise SystemExit("ryzom-private-data repository path not configured (pynel.repository_paths)")

	result = run(ryzom_data_path, ryzom_private_data_path, args.output_dir)
	report = result.report_text()
	print(report)
	if args.report_file is not None:
		args.report_file.write_text(report, encoding="utf-8")
		print(f"\nreport written to {args.report_file}")


if __name__ == "__main__":
	_main()
