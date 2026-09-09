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

"""Continent -> ecosystem lookup, from `ryzom-data`'s leveldesign sources.

Primary source: `<ryzom-data>/leveldesign/workspace/continents/<name>/
directories.py`'s `EcosystemName` (a plain string literal, confirmed present
for all 24 active continents there, 2026-09-09, including the `r2_*` Ring
ones -- e.g. `r2_desert` -> `"desert"`) -- regex-extracted rather than
executing the `.py` file, same approach as the migration script in
`ryzom-data/world_continents_flatten.md`. This source also uses the exact
ecosystem names `pipeline_data_installer.py`'s categories expect
(`primes_racines`), unlike a `.continent` file's own `Ecosystem` ATOM, which
can spell it differently (`primes_roots.ecosystem` in some source files) --
another reason `workspace/` wins over `.continent` when both exist.

Fallback for continents absent from `workspace/continents/` (only
`testroom`, as of 2026-09-09): the `.continent` file's own `Ecosystem` ATOM,
via `pynel.ryzom_continent.load_continent` on
`<ryzom-data>/leveldesign/world/continents/<name>.continent` (the flattened
structure from `ryzom-data/world_continents_flatten.md`). `None` for a
continent with neither source (not encountered as of 2026-09-09, but not
an error -- this lookup is only ever advisory, see `pipeline_data_installer.py`'s
callers, which never block on it).

Scanned once per process and cached in memory (never on disk) -- see
`get_ecosystem_for_continent`.

Usage:
	from ryzom_forgery import continent_ecosystem

	continent_ecosystem.get_ecosystem_for_continent("nexus")      # "jungle"
	continent_ecosystem.get_ecosystem_for_continent("r2_desert")  # "desert"
"""

import re
from pathlib import Path
from typing import Dict, Optional

from pynel import repository_paths
from pynel import ryzom_continent as rc

_WORKSPACE_CONTINENTS_SUBPATH = Path("leveldesign") / "workspace" / "continents"
_CONTINENTS_SUBPATH = Path("leveldesign") / "world" / "continents"

_ECOSYSTEM_NAME_RE = re.compile(r'^EcosystemName\s*=\s*"([^"]+)"', re.M)

# str(ryzom_data_path) -> {continent_name: ecosystem_name or None} -- built
# once per process per ryzom_data_path (see is_installed/download_and_install
# in pipeline_data_installer.py for why this isn't persisted to disk: the
# underlying sources don't change during a running session).
_cache: Dict[str, Dict[str, Optional[str]]] = {}


def _scan(ryzom_data_path: Path) -> Dict[str, Optional[str]]:
	result: Dict[str, Optional[str]] = {}

	workspace_dir = ryzom_data_path / _WORKSPACE_CONTINENTS_SUBPATH
	if workspace_dir.is_dir():
		for continent_dir in workspace_dir.iterdir():
			directories_py = continent_dir / "directories.py"
			if not directories_py.is_file():
				continue
			m = _ECOSYSTEM_NAME_RE.search(directories_py.read_text())
			if m:
				result[continent_dir.name] = m.group(1)

	continents_dir = ryzom_data_path / _CONTINENTS_SUBPATH
	if continents_dir.is_dir():
		for continent_file in continents_dir.glob("*.continent"):
			cf = rc.load_continent(continent_file)
			if cf.pacs_rbank and cf.pacs_rbank not in result:
				result[cf.pacs_rbank] = cf.ecosystem

	return result


def get_ecosystem_for_continent(continent_name: str) -> Optional[str]:
	"""The ecosystem name for `continent_name` (e.g. "nexus" -> "jungle"), or
	`None` if unknown by either source (see module docstring). Never raises
	for a missing `ryzom-data` path/folder -- just returns `None`, same as
	an unknown continent."""
	ryzom_data_path = repository_paths.get("ryzom-data")
	if ryzom_data_path is None:
		return None
	ryzom_data_path = Path(ryzom_data_path)
	key = str(ryzom_data_path)
	if key not in _cache:
		_cache[key] = _scan(ryzom_data_path)
	return _cache[key].get(continent_name)
