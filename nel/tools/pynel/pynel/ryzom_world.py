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

"""Reader/writer for Ryzom's `ryzom.world` file -- a thin typed wrapper over
the generic Georges FORM tree (`ryzom_georges_form`).

`ryzom.world` lists every continent as a named `<STRUCT Name="..">` inside
`<ARRAY Name="continents list">`. Note that its `continent_name` field is
**not** always the reliable runtime continent identifier -- e.g. the entry
named `matis` (both the STRUCT's own `Name` and its `selection_name`) has
`continent_name="lesfalaises"`, which does not match anything else in
Ryzom's tooling (confirmed 2026-09-09 by cross-checking against
`ryzom_continent.ContinentFile.pacs_rbank`, the actually reliable source).
This module only reads/writes the file as-is; fixing such a divergent
`continent_name` is the caller's job (see `ryzom-data/world_continents_flatten.md`).

Usage:
	from pynel import ryzom_world

	wf = ryzom_world.load_world("ryzom.world")
	for entry in wf.continents:
		print(entry.struct_name, entry.continent_name)

	matis = next(e for e in wf.continents if e.struct_name == "matis")
	matis.raw.fields["continent_name"] = "matis"
	ryzom_world.save_world("ryzom.world", wf)
"""

from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Union

from . import ryzom_georges_form as gform


def _atoi(value: Optional[str]) -> Optional[int]:
	if value is None:
		return None
	try:
		return int(value)
	except ValueError:
		return None


@dataclass
class WorldContinentEntry:
	raw: gform.GeorgesStruct
	struct_name: Optional[str]
	selection_name: Optional[str]
	continent_name: Optional[str]
	minx: Optional[int]
	miny: Optional[int]
	maxx: Optional[int]
	maxy: Optional[int]
	prim_file: Optional[str]


@dataclass
class WorldFile:
	raw: gform.GeorgesForm
	continents: List[WorldContinentEntry]


def _entry_from_struct(struct: gform.GeorgesStruct) -> WorldContinentEntry:
	return WorldContinentEntry(
		raw=struct,
		struct_name=struct.struct_name,
		selection_name=struct.atom("selection_name"),
		continent_name=struct.atom("continent_name"),
		minx=_atoi(struct.atom("minx")),
		miny=_atoi(struct.atom("miny")),
		maxx=_atoi(struct.atom("maxx")),
		maxy=_atoi(struct.atom("maxy")),
		prim_file=struct.atom("prim_file"),
	)


def parse_world(data: bytes) -> WorldFile:
	form = gform.parse_georges_form(data)
	continents = [item for item in form.root.array("continents list") if isinstance(item, gform.GeorgesStruct)]
	return WorldFile(raw=form, continents=[_entry_from_struct(s) for s in continents])


def load_world(path: Union[str, Path]) -> WorldFile:
	return parse_world(Path(path).read_bytes())


def dumps_world(wf: WorldFile) -> bytes:
	return gform.dumps_georges_form(wf.raw)


def save_world(path: Union[str, Path], wf: WorldFile) -> None:
	Path(path).write_bytes(dumps_world(wf))
