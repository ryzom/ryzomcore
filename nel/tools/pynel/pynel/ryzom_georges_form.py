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

"""Reader/writer for Ryzom/NeL "Georges FORM" files (e.g. `.continent`, `ryzom.world`).

This is a different XML format from the LIGO `.primitive` tree handled by
`ryzom_primitive.py` (PRIMITIVES/ROOT_PRIMITIVE/CHILD/PROPERTY) -- despite
the similar Ryzom/NeL lineage, the two formats are structurally unrelated.
A Georges FORM file looks like:

	<?xml version="1.0"?>
	<FORM Revision="$Revision: 1.27 $" State="modified">
	  <PARENT Filename="lesfalaises.continent"/>   <!-- optional -->
	  <STRUCT>
	    <ATOM Name="Name" Value="Nexus Minor"/>
	    <STRUCT Name="LightLandscapeDay">
	      <STRUCT Name="Ambiant">
	        <ATOM Name="A" Value="100"/>
	      </STRUCT>
	    </STRUCT>
	    <ARRAY Name="ZCs">
	      <STRUCT>                                  <!-- STRUCT items in an
	        <ATOM Name="Zone" Value="41_cf"/>            ARRAY have no Name -->
	      </STRUCT>
	    </ARRAY>
	    <ARRAY Name="WeatherSetups">
	      <ATOM Value="fo_fair1.weather_setup"/>    <!-- ATOM items in an
	      <ATOM Value="fo_fair2.weather_setup"/>        ARRAY have no Name -->
	    </ARRAY>
	  </STRUCT>
	</FORM>

A STRUCT/ARRAY/ATOM nested directly inside a STRUCT always carries a `Name`
attribute (its key in that struct's fields); a STRUCT/ARRAY/ATOM nested
inside an ARRAY never does (its position in the list is its identity),
except when the ARRAY holds named sub-structs (e.g. `ryzom.world`'s
"continents list", each entry `<STRUCT Name="matis">`) -- both shapes are
preserved via `GeorgesStruct.struct_name`, set from a STRUCT's own `Name`
attribute whenever present, regardless of whether it sits under a STRUCT or
an ARRAY.

This module does not resolve `<PARENT Filename="..."/>` inheritance (the
child form's fields are not merged with the parent's) -- callers needing a
field that may be inherited must load and check the parent form themselves,
following `GeorgesForm.parent_filename`.

Writing preserves field order as originally read (Python dicts already keep
insertion order), rather than re-deriving the canonical order from the
governing `.dfn` file (e.g. `leveldesign/DFN/world/continent.dfn`) the way
the real engine's `CFormElmStruct::build()` does -- this module has no `.dfn`
parser. This round-trips existing fields correctly (including edits to their
values) but a brand new field appended to a struct lands at the end rather
than at its DFN-declared position; not an issue for the current use case
(editing values of fields already present in the source file). Real game
data files also use `<FORM Revision="$Revision: 1.27 $" State="modified">`
(a CVS keyword-expanded revision string) rather than the `Version="X.Y"`
attribute the generic NeL engine sample (`nel/samples/georges/`) writes --
this module reproduces whatever `Revision`/`State` attributes were present
on read, matching observed real `.continent`/`ryzom.world` files rather than
the engine sample.

Usage:
	from pynel import ryzom_georges_form as gform

	form = gform.load_georges_form("lecarrefour.continent")
	print(form.root.atom("PacsRBank"))       # "nexus.rbank"
	print(form.root.struct("LightLandscapeDay").struct("Ambiant").atom("A"))  # "100"
	for item in form.root.array("ZCs"):
		print(item.atom("Zone"))             # "41_cf", "45_bz", ...

	form.root.fields["Name"] = "Nexus Minor (renamed)"
	gform.save_georges_form("lecarrefour.continent", form)
"""

import argparse
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Union


class GeorgesFormParseError(Exception):
	pass


GeorgesNode = Union[str, "GeorgesStruct", List["GeorgesNode"]]


@dataclass
class GeorgesStruct:
	"""One <STRUCT> element. `struct_name` is its own `Name` attribute, when
	present (always present for a STRUCT nested in another STRUCT; present
	only for named entries of an ARRAY, e.g. ryzom.world's "continents
	list"; absent for the form's root STRUCT and for unnamed ARRAY items).
	`fields` maps each direct child ATOM/STRUCT/ARRAY's own `Name` attribute
	to its value (str for ATOM, GeorgesStruct for STRUCT, list for ARRAY)."""

	struct_name: Optional[str] = None
	fields: Dict[str, GeorgesNode] = field(default_factory=dict)

	def atom(self, name: str) -> Optional[str]:
		"""The string value of a direct ATOM child, or None if absent or not
		an ATOM."""
		value = self.fields.get(name)
		return value if isinstance(value, str) else None

	def struct(self, name: str) -> Optional["GeorgesStruct"]:
		"""A direct STRUCT child, or None if absent or not a STRUCT."""
		value = self.fields.get(name)
		return value if isinstance(value, GeorgesStruct) else None

	def array(self, name: str) -> List[GeorgesNode]:
		"""A direct ARRAY child's items, or an empty list if absent or not
		an ARRAY."""
		value = self.fields.get(name)
		return value if isinstance(value, list) else []


@dataclass
class GeorgesForm:
	revision: str = ""
	state: str = ""
	parent_filename: Optional[str] = None
	root: GeorgesStruct = field(default_factory=GeorgesStruct)
	# Real game-data files always carry 4 empty <STRUCT/> ("held elements",
	# CForm::HeldElementCount in the engine) plus <COMMENTS>/<LOG> right
	# before </FORM> -- their content (beyond being empty/a plain string)
	# isn't modeled, just preserved verbatim on round-trip.
	comments: str = ""
	log: str = ""


def _parse_struct(el: ET.Element) -> GeorgesStruct:
	struct_name = el.attrib.get("Name")
	fields: Dict[str, GeorgesNode] = {}
	for child in el:
		if child.tag == "ATOM":
			name = child.attrib.get("Name")
			if name is None:
				raise GeorgesFormParseError("<ATOM> directly inside <STRUCT> is missing Name")
			fields[name] = child.attrib.get("Value", "")
		elif child.tag == "STRUCT":
			name = child.attrib.get("Name")
			if name is None:
				raise GeorgesFormParseError("<STRUCT> directly inside <STRUCT> is missing Name")
			fields[name] = _parse_struct(child)
		elif child.tag == "ARRAY":
			name = child.attrib.get("Name")
			if name is None:
				raise GeorgesFormParseError("<ARRAY> directly inside <STRUCT> is missing Name")
			fields[name] = _parse_array(child)
		else:
			raise GeorgesFormParseError(f"unexpected <{child.tag}> inside <STRUCT>")
	return GeorgesStruct(struct_name=struct_name, fields=fields)


def _parse_array(el: ET.Element) -> List[GeorgesNode]:
	items: List[GeorgesNode] = []
	for child in el:
		if child.tag == "ATOM":
			items.append(child.attrib.get("Value", ""))
		elif child.tag == "STRUCT":
			items.append(_parse_struct(child))
		elif child.tag == "ARRAY":
			items.append(_parse_array(child))
		else:
			raise GeorgesFormParseError(f"unexpected <{child.tag}> inside <ARRAY>")
	return items


def parse_georges_form(data: bytes) -> GeorgesForm:
	try:
		root_el = ET.fromstring(data)
	except ET.ParseError as exc:
		raise GeorgesFormParseError(f"invalid XML: {exc}") from exc

	if root_el.tag != "FORM":
		raise GeorgesFormParseError(f"not a Georges FORM file (root is <{root_el.tag}>, expected <FORM>)")

	parent_el = root_el.find("PARENT")
	parent_filename = parent_el.attrib.get("Filename") if parent_el is not None else None

	struct_el = root_el.find("STRUCT")
	if struct_el is None:
		raise GeorgesFormParseError("missing top-level <STRUCT>")

	comments_el = root_el.find("COMMENTS")
	log_el = root_el.find("LOG")

	return GeorgesForm(
		revision=root_el.attrib.get("Revision", ""),
		state=root_el.attrib.get("State", ""),
		parent_filename=parent_filename,
		root=_parse_struct(struct_el),
		comments=(comments_el.text or "") if comments_el is not None else "",
		log=(log_el.text or "") if log_el is not None else "",
	)


def load_georges_form(path: Union[str, Path]) -> GeorgesForm:
	return parse_georges_form(Path(path).read_bytes())


def _esc_attr(s: str) -> str:
	return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")


def _write_atom(out: List[str], indent: str, name: Optional[str], value: str) -> None:
	name_attr = f' Name="{_esc_attr(name)}"' if name is not None else ""
	if value == "":
		out.append(f"{indent}<ATOM{name_attr}/>\n")
	else:
		out.append(f'{indent}<ATOM{name_attr} Value="{_esc_attr(value)}"/>\n')


def _write_struct(out: List[str], indent: str, node: GeorgesStruct) -> None:
	name_attr = f' Name="{_esc_attr(node.struct_name)}"' if node.struct_name is not None else ""
	if not node.fields:
		out.append(f"{indent}<STRUCT{name_attr}/>\n")
		return
	out.append(f"{indent}<STRUCT{name_attr}>\n")
	inner = indent + "  "
	for name, value in node.fields.items():
		_write_field(out, inner, name, value)
	out.append(f"{indent}</STRUCT>\n")


def _write_array_item(out: List[str], indent: str, item: GeorgesNode) -> None:
	if isinstance(item, str):
		_write_atom(out, indent, None, item)
	elif isinstance(item, GeorgesStruct):
		_write_struct(out, indent, item)
	elif isinstance(item, list):
		raise GeorgesFormParseError("nested ARRAY-of-ARRAY is not a real Georges FORM shape and cannot be written")
	else:
		raise GeorgesFormParseError(f"unknown ARRAY item type: {type(item)!r}")


def _write_array(out: List[str], indent: str, name: str, items: List[GeorgesNode]) -> None:
	name_attr = f' Name="{_esc_attr(name)}"'
	if not items:
		out.append(f"{indent}<ARRAY{name_attr}/>\n")
		return
	out.append(f"{indent}<ARRAY{name_attr}>\n")
	inner = indent + "  "
	for item in items:
		_write_array_item(out, inner, item)
	out.append(f"{indent}</ARRAY>\n")


def _write_field(out: List[str], indent: str, name: str, value: GeorgesNode) -> None:
	if isinstance(value, str):
		_write_atom(out, indent, name, value)
	elif isinstance(value, GeorgesStruct):
		_write_struct(out, indent, value)
	elif isinstance(value, list):
		_write_array(out, indent, name, value)
	else:
		raise GeorgesFormParseError(f"unknown field value type for {name!r}: {type(value)!r}")


def dumps_georges_form(form: GeorgesForm) -> bytes:
	out: List[str] = ['<?xml version="1.0"?>\n']
	attrs = f'Revision="{_esc_attr(form.revision)}" State="{_esc_attr(form.state)}"'
	out.append(f"<FORM {attrs}>\n")
	if form.parent_filename is not None:
		out.append(f'  <PARENT Filename="{_esc_attr(form.parent_filename)}"/>\n')
	_write_struct(out, "  ", form.root)
	# 4 "held elements", always empty here (see GeorgesForm.comments/log docstring)
	out.append("  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n")
	out.append(f"  <COMMENTS>{_esc_attr(form.comments)}</COMMENTS>\n")
	out.append(f"  <LOG>{_esc_attr(form.log)}</LOG>\n")
	out.append("</FORM>\n")
	return "".join(out).encode("utf-8")


def save_georges_form(path: Union[str, Path], form: GeorgesForm) -> None:
	Path(path).write_bytes(dumps_georges_form(form))


def _dump(node: GeorgesNode, key: str = "", depth: int = 0) -> None:
	indent = "  " * depth
	if isinstance(node, GeorgesStruct):
		label = f'STRUCT {key!r}' if key else "STRUCT"
		if node.struct_name is not None:
			label += f' (Name={node.struct_name!r})'
		print(f"{indent}{label}")
		for name, value in node.fields.items():
			_dump(value, name, depth + 1)
	elif isinstance(node, list):
		print(f"{indent}ARRAY {key!r} ({len(node)} items)")
		for item in node:
			_dump(item, "", depth + 1)
	else:
		print(f"{indent}{key!r} = {node!r}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Inspect Ryzom/NeL Georges FORM files (.continent, ryzom.world, ...)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print the parsed FORM tree")
	p_dump.add_argument("path", type=Path)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	if args.command == "dump":
		form = load_georges_form(args.path)
		print(f"revision={form.revision!r} state={form.state!r} parent_filename={form.parent_filename!r}")
		_dump(form.root)


if __name__ == "__main__":
	_main()
