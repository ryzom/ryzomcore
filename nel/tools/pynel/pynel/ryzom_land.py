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

"""Read Ryzom/NeL .land landscape composition files (CZoneRegion::serial,
nel/src/ligo/zone_region.cpp:154).

Unlike .zone/.zonew/.zonel (raw binary, see ryzom_zone.py), .land is plain
XML on disk -- CZoneRegion::serial() is generic and adapts to the stream
type passed, and the real tool (world_editor) always uses CIXml/COXml for
it. Parsed here with xml.etree.ElementTree, same approach already used for
mode2animset.string_array in ryzom_packed_sheets.py.

A .land is a flat row-major grid of "bricks", each an ordinary .zone file
already fully supported by pynel.ryzom_zone (load_zone/build_zone) -- no
extra geometry format to trace here. An empty grid cell is NOT an empty
string: NeL's own default (SZoneUnit::SZoneUnit(), zone_region.cpp:38) is
the literal sentinel STRING_UNUSED = "< UNUSED >" (zone_region.h:32), and
that is what's actually written to disk -- confirmed against real data
(ryzom-data/leveldesign/landscape/primes_racines/bagne.land).

Two on-disk region versions (the LAND/VERSION XML field, independent of
each ELM's own inner <VERSION> which is SZoneUnit2's own sub-version, always
0 today):
  - v1 (current): each cell is a SZoneUnit2 (adds DateLow/DateHigh, purely
    informative last-modified timestamps).
  - v0 (legacy, read-only): each cell is a plain SZoneUnit (no dates). Read
    here and upgraded to ZoneUnit with dates=0, same convention as the rest
    of pynel (silent upgrade on read, always write back as the current
    version -- see ryzom_land.build_land in a later step).

LAND/CHECK is a fixed magic checksum (f.serialCheck(NELID("DNAL")) in
CZoneRegion::serial), not real data -- verified on read, not exposed.

Read-only for now: write support (round-trip serialization) and
land_export/zone_elevation/zone_dependencies/zone_ig_lighter orchestration
are later steps of project-todos/pynel/land_pipeline.md.

Usage:
	from pynel import ryzom_land as rl
	land = rl.load_land("bagne.land")
	print(land.min_x, land.max_x, len(land.zones))
	unit = land.zones[(x - land.min_x) + (y - land.min_y) * (1 + land.max_x - land.min_x)]
	if unit.zone_name != rl.STRING_UNUSED:
		print(unit.zone_name)
"""

import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, List, Union

# SZoneUnit's default ZoneName/SharingMatNames[i] (zone_region.h:32) -- the sentinel for
# an unused grid cell / an edge with no sharing-material metadata. NOT an empty string.
STRING_UNUSED = "< UNUSED >"

# NELID("DNAL") (stream.h) -- CZoneRegion::serial's f.serialCheck() magic, decimal since
# it's serialized through xmlSerial(uint32) rather than raw bytes like .zone's MAGIC.
CHECK_MAGIC = 1145979212


class LandParseError(Exception):
	pass


@dataclass
class ZoneUnit:
	"""CZoneRegion::SZoneUnit2 (SZoneUnit + dates), one grid cell of a .land."""
	zone_name: str
	pos_x: int
	pos_y: int
	rot: int
	flip: int
	sharing_mat_names: List[str] = field(default_factory=lambda: [STRING_UNUSED] * 4)
	sharing_cut_edges: List[int] = field(default_factory=lambda: [0, 0, 0, 0])
	date_low: int = 0
	date_high: int = 0


@dataclass
class ZoneRegion:
	"""CZoneRegion (zone_region.h/.cpp), the top-level .land payload: a flat
	row-major grid of ZoneUnit cells over [min_x..max_x] x [min_y..max_y].
	index = (x - min_x) + (y - min_y) * (1 + max_x - min_x)."""
	min_x: int
	min_y: int
	max_x: int
	max_y: int
	zones: List[ZoneUnit] = field(default_factory=list)


def _find_text(elm: ET.Element, tag: str) -> str:
	child = elm.find(tag)
	if child is None or child.text is None:
		raise LandParseError(f"missing <{tag}> in <{elm.tag}>")
	return child.text


def _parse_string_field(elm: ET.Element, tag: str) -> str:
	"""A NeL string field serialized as <TAG><S>value</S></TAG>. An empty
	string is written as the self-closed <S/> (s.text is then None)."""
	child = elm.find(tag)
	if child is None:
		raise LandParseError(f"missing <{tag}> in <{elm.tag}>")
	s = child.find("S")
	if s is None:
		raise LandParseError(f"missing <S> inside <{tag}> in <{elm.tag}>")
	return s.text or ""


def _parse_zone_unit(elm: ET.Element, has_dates: bool) -> ZoneUnit:
	zone_name = _parse_string_field(elm, "NAME")
	pos_x = int(_find_text(elm, "X"))
	pos_y = int(_find_text(elm, "Y"))
	rot = int(_find_text(elm, "ROT"))
	flip = int(_find_text(elm, "FLIP"))

	mat_name_elms = elm.findall("MAT_NAMES")
	cut_edge_elms = elm.findall("CUR_EDGES")
	if len(mat_name_elms) != 4 or len(cut_edge_elms) != 4:
		raise LandParseError(
			f"expected 4 MAT_NAMES/CUR_EDGES pairs, got {len(mat_name_elms)}/{len(cut_edge_elms)}"
		)
	sharing_mat_names = []
	for mat_elm in mat_name_elms:
		s = mat_elm.find("S")
		if s is None:
			raise LandParseError("missing <S> inside <MAT_NAMES>")
		sharing_mat_names.append(s.text or "")
	sharing_cut_edges = [int(c.text) for c in cut_edge_elms]

	date_low = 0
	date_high = 0
	if has_dates:
		date_low = int(_find_text(elm, "LOW"))
		date_high = int(_find_text(elm, "HIGH"))

	return ZoneUnit(
		zone_name=zone_name,
		pos_x=pos_x,
		pos_y=pos_y,
		rot=rot,
		flip=flip,
		sharing_mat_names=sharing_mat_names,
		sharing_cut_edges=sharing_cut_edges,
		date_low=date_low,
		date_high=date_high,
	)


def parse_land(data: bytes) -> ZoneRegion:
	root = ET.fromstring(data)
	if root.tag != "LAND":
		raise LandParseError(f"expected root <LAND>, got <{root.tag}>")

	version = int(_find_text(root, "VERSION"))
	if version not in (0, 1):
		raise LandParseError(f"unsupported .land version {version}")

	check = int(_find_text(root, "CHECK"))
	if check != CHECK_MAGIC:
		raise LandParseError(f"bad .land CHECK magic: {check} (expected {CHECK_MAGIC})")

	min_x = int(_find_text(root, "MIN_X"))
	min_y = int(_find_text(root, "MIN_Y"))
	max_x = int(_find_text(root, "MAX_X"))
	max_y = int(_find_text(root, "MAX_Y"))

	vector = root.find("VECTOR")
	if vector is None:
		raise LandParseError("missing <VECTOR>")
	elms = vector.findall("ELM")

	expected_count = (max_x - min_x + 1) * (max_y - min_y + 1)
	if len(elms) != expected_count:
		raise LandParseError(
			f"grid size mismatch: {len(elms)} <ELM> for a "
			f"[{min_x}..{max_x}]x[{min_y}..{max_y}] grid ({expected_count} expected)"
		)

	zones = [_parse_zone_unit(elm, has_dates=(version == 1)) for elm in elms]

	return ZoneRegion(min_x=min_x, min_y=min_y, max_x=max_x, max_y=max_y, zones=zones)


def load_land(path: Union[str, Path, BinaryIO]) -> ZoneRegion:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_land(data)


# Region version this writer always emits -- a .land read from a legacy v0 file
# is always safe to write back, silently upgraded to v1 (adds per-cell dates),
# same convention as ryzom_zone.py's build_zone.
_LAND_VERSION = 1


def _append_string_field(parent: ET.Element, tag: str, value: str) -> None:
	field = ET.SubElement(parent, tag)
	ET.SubElement(field, "S").text = value


def _build_zone_unit_elm(vector: ET.Element, unit: ZoneUnit) -> None:
	elm = ET.SubElement(vector, "ELM")
	ET.SubElement(elm, "VERSION").text = "0"  # SZoneUnit2's own sub-version, always 0

	_append_string_field(elm, "NAME", unit.zone_name)
	ET.SubElement(elm, "X").text = str(unit.pos_x)
	ET.SubElement(elm, "Y").text = str(unit.pos_y)
	ET.SubElement(elm, "ROT").text = str(unit.rot)
	ET.SubElement(elm, "FLIP").text = str(unit.flip)

	if len(unit.sharing_mat_names) != 4 or len(unit.sharing_cut_edges) != 4:
		raise LandParseError("ZoneUnit sharing_mat_names/sharing_cut_edges must have exactly 4 entries")
	for mat_name, cut_edge in zip(unit.sharing_mat_names, unit.sharing_cut_edges):
		_append_string_field(elm, "MAT_NAMES", mat_name)
		ET.SubElement(elm, "CUR_EDGES").text = str(cut_edge)

	ET.SubElement(elm, "LOW").text = str(unit.date_low)
	ET.SubElement(elm, "HIGH").text = str(unit.date_high)


def build_land(region: ZoneRegion) -> bytes:
	"""Serializes a ZoneRegion back to .land XML bytes, always at the current
	region version (v1, per-cell dates included) -- matching the engine's own
	re-save-upgrades behaviour. No geometric transformation is performed; this
	is a pure round-trip writer."""
	expected_count = (region.max_x - region.min_x + 1) * (region.max_y - region.min_y + 1)
	if len(region.zones) != expected_count:
		raise LandParseError(
			f"grid size mismatch: {len(region.zones)} zones for a "
			f"[{region.min_x}..{region.max_x}]x[{region.min_y}..{region.max_y}] grid ({expected_count} expected)"
		)

	root = ET.Element("LAND")
	ET.SubElement(root, "VERSION").text = str(_LAND_VERSION)
	ET.SubElement(root, "CHECK").text = str(CHECK_MAGIC)
	ET.SubElement(root, "MIN_X").text = str(region.min_x)
	ET.SubElement(root, "MIN_Y").text = str(region.min_y)
	ET.SubElement(root, "MAX_X").text = str(region.max_x)
	ET.SubElement(root, "MAX_Y").text = str(region.max_y)

	vector = ET.SubElement(root, "VECTOR")
	vector.set("size", str(len(region.zones)))
	for unit in region.zones:
		_build_zone_unit_elm(vector, unit)

	ET.indent(root, space="  ")
	body = ET.tostring(root, encoding="unicode")
	return ('<?xml version="1.0"?>\n' + body + "\n").encode("utf-8")


def save_land(path: Union[str, Path, BinaryIO], region: ZoneRegion) -> None:
	data = build_land(region)
	if hasattr(path, "write"):
		path.write(data)
	else:
		Path(path).write_bytes(data)
