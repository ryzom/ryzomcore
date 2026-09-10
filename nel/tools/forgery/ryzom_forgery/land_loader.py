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

"""Discovers `.land` files under a `ryzom-data` checkout, indexed by
continent identifier -- used by Edition mode to filter the continent combo
down to continents that actually have a `.land`
(project-todos/forgery/landscape_editor__land_preview.md step 2).

The real `.land` files live under `<ryzom-data>/leveldesign/landscape/`
(never `graphics/landscape/ligo/`, confirmed obsolete -- see
docs/apps/landscape_editor.md), and a `.land`'s filename stem is the same
reliable continent identifier used everywhere else in Ryzom tooling
(`PacsRBank`, e.g. "fyros"/"matis"/"nexus" -- confirmed 2026-09-10 against
the real files: every `.land` stem matches a real `pacs_rbank`, never a
folder name like "legranddesert").
"""

from pathlib import Path
from typing import Dict, Union

_LANDSCAPE_SUBPATH = "leveldesign/landscape"


def find_land_files(ryzom_data_path: Union[str, Path]) -> Dict[str, Path]:
	"""{continent identifier (the `.land`'s own filename stem) -> its path},
	recursive scan of `<ryzom_data_path>/leveldesign/landscape/`. Empty dict
	if that directory doesn't exist (unconfigured/invalid ryzom-data, or a
	checkout missing this subtree)."""
	root = Path(ryzom_data_path) / _LANDSCAPE_SUBPATH
	if not root.is_dir():
		return {}
	return {path.stem: path for path in root.rglob("*.land")}
