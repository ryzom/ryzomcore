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

"""Reader/writer for Ryzom/NeL .primitive (LIGO primitive tree) files.

Format reverse-engineered from nel/include/nel/ligo/primitive.h and
nel/src/ligo/primitive.cpp (IPrimitive::read/write, CPrimitives::read/write,
CPrimPoint/CPrimPath/CPrimZone/CPrimAlias::read/write). Unlike .ig/.shape,
this is a plain XML tree, not a binary format.

Structure (see CPrimitives::read/write):

	<PRIMITIVES VERSION="1">
	  <ROOT_PRIMITIVE TYPE="CPrimNode">
	    <ALIAS LAST_GENERATED="22"/>        <!-- only if VERSION > 0 -->
	    <PROPERTY TYPE="string|string_array|color">
	      <NAME>...</NAME>
	      <STRING>...</STRING>              <!-- string: one; string_array: repeated -->
	      <COLOR R="" G="" B="" A=""/>       <!-- color only -->
	    </PROPERTY>
	    <CHILD TYPE="CPrimNode|CPrimPoint|CPrimPath|CPrimZone|CPrimAlias">
	      <PT X="" Y="" Z="" SELECTED="true"/>   <!-- CPrimPoint (one), CPrimPath/CPrimZone (many) -->
	      <ANGLE VALUE=""/>                       <!-- CPrimPoint only, omitted if 0 -->
	      <ALIAS VALUE="7"/>                       <!-- CPrimAlias only -->
	      <!-- + nested PROPERTY*/CHILD*, recursively -->
	    </CHILD>
	  </ROOT_PRIMITIVE>
	</PRIMITIVES>

The "class"/"name" strings seen on most primitives (e.g. class="region") are
just ordinary string properties, not a dedicated XML construct — the set of
valid classes and their property schemas lives in a separate LIGO config
file (e.g. world_editor_classes.xml, referenced by Studio's "LIGO config
file" setting), not in the .primitive file itself. This module doesn't
interpret that schema; it round-trips whatever properties are present.

NLMISC::CPrimitives::_Properties is a std::map<std::string, IProperty*>, so
the C++ engine always writes properties out in ascending name order — this
module does the same on write, for the same reason .ig/.shape always emit
the latest sub-format version: matching the engine's own output as closely
as possible.

Usage:
	from pynel import ryzom_primitive as prim

	pf = prim.load_primitive("dummy.primitive")
	for child in pf.root.children:
		print(child.type, prim.get_property(child, "name"))

	zone = prim.PrimZone(points=[prim.Vector(0, 0, 0), prim.Vector(1, 0, 0), prim.Vector(1, 1, 0)])
	prim.set_property(zone, "class", "region")
	prim.set_property(zone, "name", "my_new_zone")
	pf.root.children.append(zone)
	prim.save_primitive("dummy_edited.primitive", pf)
"""

import argparse
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Union

PRIMITIVE_VERSION = 1


class PrimitiveParseError(Exception):
	pass


@dataclass
class Vector:
	x: float = 0.0
	y: float = 0.0
	z: float = 0.0
	selected: bool = False


@dataclass
class Rgba:
	r: int = 0
	g: int = 0
	b: int = 0
	a: int = 255


@dataclass
class PropertyString:
	value: str = ""


@dataclass
class PropertyStringArray:
	value: List[str] = field(default_factory=list)


@dataclass
class PropertyColor:
	value: Rgba = field(default_factory=Rgba)


PropertyValue = Union[PropertyString, PropertyStringArray, PropertyColor]


@dataclass
class Primitive:
	"""Base for every primitive node; also IS the concrete CPrimNode (no
	extra fields of its own, just properties/children like every other
	primitive type)."""

	properties: Dict[str, PropertyValue] = field(default_factory=dict)
	children: List["Primitive"] = field(default_factory=list)
	# Raw text of an XML comment preceding the properties, if any (LIGO's
	# "unparsed properties for editor view" — preserved verbatim, not
	# interpreted).
	unparsed_properties: str = ""

	@property
	def type(self) -> str:
		"""The NeL class name for this primitive, e.g. "CPrimZone"."""
		return _TYPE_NAMES[type(self)]


@dataclass
class PrimPoint(Primitive):
	point: Vector = field(default_factory=Vector)
	angle: float = 0.0  # radians, CCW around OZ


@dataclass
class PrimPath(Primitive):
	points: List[Vector] = field(default_factory=list)


@dataclass
class PrimZone(Primitive):
	points: List[Vector] = field(default_factory=list)


@dataclass
class PrimAlias(Primitive):
	alias: int = 0


_TYPE_NAMES = {
	Primitive: "CPrimNode",
	PrimPoint: "CPrimPoint",
	PrimPath: "CPrimPath",
	PrimZone: "CPrimZone",
	PrimAlias: "CPrimAlias",
}

# CPrimitives::read()/IPrimitive::read() also accepts these legacy
# lowercase shorthands for CHILD TYPE and normalizes them.
_TYPE_ALIASES = {
	"node": "CPrimNode",
	"point": "CPrimPoint",
	"path": "CPrimPath",
	"zone": "CPrimZone",
	"alias": "CPrimAlias",
	"CPrimNode": "CPrimNode",
	"CPrimPoint": "CPrimPoint",
	"CPrimPath": "CPrimPath",
	"CPrimZone": "CPrimZone",
	"CPrimAlias": "CPrimAlias",
}

_CLASS_BY_TYPE_NAME = {
	"CPrimNode": Primitive,
	"CPrimPoint": PrimPoint,
	"CPrimPath": PrimPath,
	"CPrimZone": PrimZone,
	"CPrimAlias": PrimAlias,
}


@dataclass
class PrimitiveFile:
	root: Primitive
	version: int = PRIMITIVE_VERSION
	last_generated_alias: int = 0


def get_property(prim: Primitive, name: str) -> Optional[str]:
	"""Convenience accessor for the common case of a string property
	(e.g. "class", "name"). Returns None if absent or not a string."""
	p = prim.properties.get(name)
	return p.value if isinstance(p, PropertyString) else None


def set_property(prim: Primitive, name: str, value: str) -> None:
	"""Convenience setter for a string property."""
	prim.properties[name] = PropertyString(value)


def _fmt_float(v: float) -> str:
	# NLMISC::toString(float) is toString("%f", val) -- printf's default
	# 6 fractional digits.
	return f"{v:.6f}"


_INT_RE = re.compile(r"\s*[-+]?\d+")
_FLOAT_RE = re.compile(r"\s*[-+]?(\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?")


def _atoi(s: str) -> int:
	"""Like C's atoi(): parses a leading integer, returns 0 if none (never
	raises). A few real .primitive files in the wild have empty
	VALUE="" attributes, which NeL's own atoi()-based reader silently
	turns into 0 -- match that instead of rejecting the file."""
	m = _INT_RE.match(s)
	return int(m.group()) if m else 0


def _atof(s: str) -> float:
	"""Like C's strtod(): parses a leading float and ignores any trailing
	garbage, returns 0.0 if nothing valid at the start (never raises). At
	least one real .primitive file has a stray space before the decimal
	point (e.g. "-3578 .000000"); strtod-style parsing reads "-3578" and
	stops there, same as the engine's own NLMISC::fromString would."""
	m = _FLOAT_RE.match(s)
	return float(m.group()) if m else 0.0


def _parse_vector(el: ET.Element) -> Vector:
	if "X" not in el.attrib or "Y" not in el.attrib or "Z" not in el.attrib:
		raise PrimitiveParseError(f"<{el.tag}> missing X/Y/Z")
	x = _atof(el.attrib["X"])
	y = _atof(el.attrib["Y"])
	z = _atof(el.attrib["Z"])
	selected = el.attrib.get("SELECTED") == "true"
	return Vector(x, y, z, selected)


def _parse_properties(el: ET.Element) -> Dict[str, PropertyValue]:
	properties: Dict[str, PropertyValue] = {}
	for prop_el in el.findall("PROPERTY"):
		name_el = prop_el.find("NAME")
		if name_el is None or name_el.text is None:
			raise PrimitiveParseError("<PROPERTY> missing <NAME>")
		name = name_el.text
		ptype = prop_el.attrib.get("TYPE")

		if ptype == "string":
			string_el = prop_el.find("STRING")
			properties[name] = PropertyString(string_el.text or "" if string_el is not None else "")
		elif ptype == "string_array":
			properties[name] = PropertyStringArray([s.text or "" for s in prop_el.findall("STRING")])
		elif ptype == "color":
			color_el = prop_el.find("COLOR")
			if color_el is None:
				raise PrimitiveParseError(f"<PROPERTY TYPE=\"color\"> {name!r} missing <COLOR>")
			properties[name] = PropertyColor(
				Rgba(
					_atoi(color_el.attrib.get("R", "0")),
					_atoi(color_el.attrib.get("G", "0")),
					_atoi(color_el.attrib.get("B", "0")),
					_atoi(color_el.attrib.get("A", "255")),
				)
			)
		else:
			raise PrimitiveParseError(f"unknown property type {ptype!r} for {name!r}")

	return properties


def _parse_primitive(el: ET.Element) -> Primitive:
	type_name = _TYPE_ALIASES.get(el.attrib.get("TYPE", "CPrimNode"))
	if type_name is None:
		raise PrimitiveParseError(f"unknown primitive TYPE {el.attrib.get('TYPE')!r}")
	cls = _CLASS_BY_TYPE_NAME[type_name]

	comment = next((c.text or "" for c in el if c.tag is ET.Comment), "")

	if cls is PrimPoint:
		pt_el = el.find("PT")
		if pt_el is None:
			raise PrimitiveParseError("CPrimPoint missing <PT>")
		point = _parse_vector(pt_el)
		angle_el = el.find("ANGLE")
		angle = _atof(angle_el.attrib["VALUE"]) if angle_el is not None and "VALUE" in angle_el.attrib else 0.0
		prim: Primitive = PrimPoint(point=point, angle=angle)
	elif cls in (PrimPath, PrimZone):
		points = [_parse_vector(pt_el) for pt_el in el.findall("PT")]
		prim = cls(points=points)
	elif cls is PrimAlias:
		alias_el = el.find("ALIAS")
		if alias_el is None or "VALUE" not in alias_el.attrib:
			raise PrimitiveParseError("CPrimAlias missing <ALIAS VALUE=\"...\">")
		prim = PrimAlias(alias=_atoi(alias_el.attrib["VALUE"]))
	else:
		prim = Primitive()

	prim.unparsed_properties = comment
	prim.properties = _parse_properties(el)
	for child_el in el.findall("CHILD"):
		prim.children.append(_parse_primitive(child_el))

	return prim


def parse_primitive(data: bytes) -> PrimitiveFile:
	try:
		root_el = ET.fromstring(data)
	except ET.ParseError as exc:
		raise PrimitiveParseError(f"invalid XML: {exc}") from exc

	if root_el.tag != "PRIMITIVES":
		raise PrimitiveParseError(f"not a NeL primitive file (root is <{root_el.tag}>, expected <PRIMITIVES>)")

	version = _atoi(root_el.attrib.get("VERSION", "0"))
	if version > PRIMITIVE_VERSION:
		raise PrimitiveParseError(f"unsupported primitive file version {version} (max supported: {PRIMITIVE_VERSION})")

	root_prim_el = root_el.find("ROOT_PRIMITIVE")
	if root_prim_el is None:
		raise PrimitiveParseError("missing <ROOT_PRIMITIVE>")

	last_generated_alias = 0
	if version > 0:
		alias_el = root_prim_el.find("ALIAS")
		if alias_el is not None:
			last_generated_alias = _atoi(alias_el.attrib.get("LAST_GENERATED", "0"))

	root = _parse_primitive(root_prim_el)
	return PrimitiveFile(root=root, version=version, last_generated_alias=last_generated_alias)


def load_primitive(path: Union[str, Path]) -> PrimitiveFile:
	return parse_primitive(Path(path).read_bytes())


def _esc_attr(s: str) -> str:
	return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")


def _esc_text(s: str) -> str:
	return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def _write_vector(out: List[str], indent: str, tag: str, v: Vector) -> None:
	attrs = f'X="{_fmt_float(v.x)}" Y="{_fmt_float(v.y)}" Z="{_fmt_float(v.z)}"'
	if v.selected:
		attrs += ' SELECTED="true"'
	out.append(f"{indent}<{tag} {attrs}/>\n")


def _write_properties(out: List[str], indent: str, prim: Primitive) -> None:
	if prim.unparsed_properties:
		out.append(f"{indent}<!--{prim.unparsed_properties}-->\n")

	# std::map<std::string, IProperty*> iterates in ascending key order.
	for name in sorted(prim.properties):
		value = prim.properties[name]
		if isinstance(value, PropertyString):
			out.append(f'{indent}<PROPERTY TYPE="string">\n')
			out.append(f"{indent}  <NAME>{_esc_text(name)}</NAME>\n")
			out.append(f"{indent}  <STRING>{_esc_text(value.value)}</STRING>\n")
			out.append(f"{indent}</PROPERTY>\n")
		elif isinstance(value, PropertyStringArray):
			out.append(f'{indent}<PROPERTY TYPE="string_array">\n')
			out.append(f"{indent}  <NAME>{_esc_text(name)}</NAME>\n")
			for s in value.value:
				out.append(f"{indent}  <STRING>{_esc_text(s)}</STRING>\n")
			out.append(f"{indent}</PROPERTY>\n")
		elif isinstance(value, PropertyColor):
			c = value.value
			out.append(f'{indent}<PROPERTY TYPE="color">\n')
			out.append(f"{indent}  <NAME>{_esc_text(name)}</NAME>\n")
			out.append(f'{indent}  <COLOR R="{c.r}" G="{c.g}" B="{c.b}" A="{c.a}"/>\n')
			out.append(f"{indent}</PROPERTY>\n")
		else:
			raise PrimitiveParseError(f"unknown property value type for {name!r}: {type(value)!r}")


def _write_primitive(out: List[str], indent: str, prim: Primitive, tag: str) -> None:
	type_name = _TYPE_NAMES[type(prim)]
	out.append(f'{indent}<{tag} TYPE="{type_name}">\n')
	inner = indent + "  "

	if isinstance(prim, PrimPoint):
		_write_vector(out, inner, "PT", prim.point)
		if prim.angle != 0.0:
			out.append(f'{inner}<ANGLE VALUE="{_fmt_float(prim.angle)}"/>\n')
	elif isinstance(prim, (PrimPath, PrimZone)):
		for p in prim.points:
			_write_vector(out, inner, "PT", p)
	elif isinstance(prim, PrimAlias):
		out.append(f'{inner}<ALIAS VALUE="{prim.alias}"/>\n')

	_write_properties(out, inner, prim)

	for child in prim.children:
		_write_primitive(out, inner, child, "CHILD")

	out.append(f"{indent}</{tag}>\n")


def dumps(pf: PrimitiveFile) -> bytes:
	out: List[str] = ['<?xml version="1.0"?>\n']
	out.append(f'<PRIMITIVES VERSION="{pf.version}">\n')

	root_type = _TYPE_NAMES[type(pf.root)]
	out.append(f'  <ROOT_PRIMITIVE TYPE="{root_type}">\n')
	if pf.version > 0:
		out.append(f'    <ALIAS LAST_GENERATED="{pf.last_generated_alias}"/>\n')
	_write_properties(out, "    ", pf.root)
	for child in pf.root.children:
		_write_primitive(out, "    ", child, "CHILD")
	out.append("  </ROOT_PRIMITIVE>\n")

	out.append("</PRIMITIVES>\n")
	return "".join(out).encode("utf-8")


def save_primitive(path: Union[str, Path], pf: PrimitiveFile) -> None:
	Path(path).write_bytes(dumps(pf))


def _dump(prim: Primitive, depth: int = 0) -> None:
	label = get_property(prim, "name") or get_property(prim, "class") or ""
	extra = ""
	if isinstance(prim, PrimPoint):
		extra = f" point=({prim.point.x:.2f}, {prim.point.y:.2f}, {prim.point.z:.2f})"
	elif isinstance(prim, (PrimPath, PrimZone)):
		extra = f" points={len(prim.points)}"
	elif isinstance(prim, PrimAlias):
		extra = f" alias={prim.alias}"
	print("  " * depth + f"{type(prim).__name__} {label!r}{extra}")
	for child in prim.children:
		_dump(child, depth + 1)


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Inspect/edit Ryzom/NeL .primitive files")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print the primitive tree")
	p_dump.add_argument("path", type=Path)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	if args.command == "dump":
		pf = load_primitive(args.path)
		print(f"version={pf.version} last_generated_alias={pf.last_generated_alias}")
		_dump(pf.root)


if __name__ == "__main__":
	_main()
