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

"""Reader/writer for Ryzom `.continent` files -- a thin typed wrapper over
the generic Georges FORM tree (`ryzom_georges_form`).

Only a handful of top-level ATOM fields are exposed individually (the ones
callers actually need so far); everything else -- lighting, places, sky
sheets, weather functions, ... -- stays reachable through `ContinentFile.raw`
(a `GeorgesStruct`, see `ryzom_georges_form.GeorgesStruct`).

The most important field is `pacs_rbank`: neither a `.continent` file's own
filename nor the directory it lives in under `leveldesign/world/` is a
reliable runtime continent identifier (both can differ from it -- e.g.
`lecarrefour.continent`, folder `lecarrefour/`, is actually the continent
**nexus**; `lesfalaises(matis)/lesfalaises.continent` is actually **matis**).
The one reliable field is the `PacsRBank` ATOM (e.g. `Value="nexus.rbank"`),
whose value with the `.rbank` suffix stripped is the same identifier used
everywhere else in Ryzom tooling (`pipeline/export/continents/<name>/`,
`live_data_path`'s per-continent sheets, etc.) -- see `pacs_rbank` below.

Usage:
	from pynel import ryzom_continent

	cf = ryzom_continent.load_continent("lecarrefour.continent")
	print(cf.pacs_rbank)   # "nexus"
	print(cf.ecosystem)    # "jungle"

	cf.raw.fields["Name"] = "Nexus Minor (renamed)"
	ryzom_continent.save_continent("lecarrefour.continent", cf)
"""

from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Union

from . import ryzom_georges_form as gform


def _strip_suffix(value: Optional[str], suffix: str) -> Optional[str]:
	if value is None:
		return None
	return value[: -len(suffix)] if value.endswith(suffix) else value


@dataclass
class ContinentFile:
	raw: gform.GeorgesForm
	name: Optional[str]
	pacs_rbank: Optional[str]
	ecosystem: Optional[str]


def parse_continent(data: bytes) -> ContinentFile:
	form = gform.parse_georges_form(data)
	return ContinentFile(
		raw=form,
		name=form.root.atom("Name"),
		pacs_rbank=_strip_suffix(form.root.atom("PacsRBank"), ".rbank"),
		ecosystem=_strip_suffix(form.root.atom("Ecosystem"), ".ecosystem"),
	)


def load_continent(path: Union[str, Path]) -> ContinentFile:
	return parse_continent(Path(path).read_bytes())


def dumps_continent(cf: ContinentFile) -> bytes:
	return gform.dumps_georges_form(cf.raw)


def save_continent(path: Union[str, Path], cf: ContinentFile) -> None:
	Path(path).write_bytes(dumps_continent(cf))
