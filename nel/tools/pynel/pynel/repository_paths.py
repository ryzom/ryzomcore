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

"""Per-user config for the local checkout paths of Ryzom's own git
repositories -- shared across every tool built on pynel (Forgery/Patina,
ryztart, ...), so a user only ever points these out once, in one place, not
once per tool. Stored as one small JSON file at
config_dir()/"repository_paths.json", deliberately independent of any single
tool's own settings (Forgery has its own settings.toml for Forgery-only
preferences) -- this lives in pynel, not Forgery, since any tool built on
top of pynel may need to resolve "where is ryzom-data on this machine" the
same way. No external dependency (plain json, matching pynel's own
zero-dependency policy), unlike ryzom_forgery.settings's tomlkit-based file.

Usage:
	from pynel import repository_paths
	data_root = repository_paths.get("ryzom-data")
	if data_root is None or not data_root.is_dir():
		... # ask the user to configure it first
"""

import json
import os
import sys
from pathlib import Path
from typing import Dict, Optional

# The 4 git repositories a Ryzom Core contributor typically has checked out
# side by side. Every caller of this module uses these exact keys.
REPOSITORIES = ("ryzom-core", "ryzom-data", "ryzom-private-data", "ryzom-docker")

_FILE_NAME = "repository_paths.json"


def config_dir() -> Path:
	"""No external lib, just the well-known env vars/paths for each OS --
	same convention as ryzom_forgery.config_dir()/cache_dir(), reimplemented
	here (not imported) since pynel has zero dependencies and must not
	depend on Forgery -- the dependency only ever goes the other way."""
	if sys.platform == "win32":
		base = os.environ.get("APPDATA", str(Path.home() / "AppData" / "Roaming"))
	elif sys.platform == "darwin":
		base = str(Path.home() / "Library" / "Application Support")
	else:
		base = os.environ.get("XDG_CONFIG_HOME", str(Path.home() / ".config"))
	return Path(base) / "ryzom_tools"


def load() -> Dict[str, str]:
	"""{repo_name: path} for whichever of REPOSITORIES the user has already
	configured -- an unconfigured repo just isn't a key in the dict, never
	an empty string. Corrupt/missing file is just an empty starting point,
	not an error."""
	path = config_dir() / _FILE_NAME
	try:
		data = json.loads(path.read_text())
	except (OSError, ValueError):
		return {}
	if not isinstance(data, dict):
		return {}
	return {name: value for name, value in data.items() if name in REPOSITORIES and value}


def save(paths: Dict[str, str]) -> None:
	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	clean = {name: value for name, value in paths.items() if name in REPOSITORIES and value}
	(directory / _FILE_NAME).write_text(json.dumps(clean, indent="\t", sort_keys=True))


def set_path(repo_name: str, path: str) -> None:
	"""Updates a single repo's path, leaving the others untouched -- the
	usual call from a settings UI's folder picker."""
	if repo_name not in REPOSITORIES:
		raise ValueError(f"unknown repository {repo_name!r}, expected one of {REPOSITORIES}")
	current = load()
	current[repo_name] = path
	save(current)


def get(repo_name: str) -> Optional[Path]:
	"""The configured path for one repo, or None if not set. Doesn't check
	the path still exists on disk -- see is_valid()."""
	value = load().get(repo_name)
	return Path(value) if value else None


def is_valid(repo_name: str) -> bool:
	"""True if `repo_name` is configured AND the configured path currently
	exists as a directory -- what a caller should actually gate on before
	trying to read/write files under it (a stale/moved checkout is common
	enough to not just trust a configured string blindly)."""
	path = get(repo_name)
	return path is not None and path.is_dir()
