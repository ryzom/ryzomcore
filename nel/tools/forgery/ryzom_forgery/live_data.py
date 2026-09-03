"""Locates the Ryzom Live client's real game-data folder (creature.packed_sheets,
item.packed_sheets, sheet_id.bin, .bnp archives, etc), needed by creature_ref.py
to build the real skel/anim lookup caches -- see forgery-object-editor.md's
"Real skel/anim lookup for the Skinning preview" chantier.

Auto-detection mirrors ryztart's own RyzomConfigIni (app/modules/ryzom_config_ini/
config.py, `getLocationUser()`/`getServersFromIni()`): the user-wide `ryzom.ini`
lives at `appdirs.user_data_dir("Ryzom", "", roaming=<True|False>)`, whichever of
the two candidate OS-specific locations actually exists (roaming tried first,
matching ryztart's own order) -- never a hardcoded path, Forgery is cross-platform
(confirmed with Nuno, 2026-09-03). Each shard has its own `[<ShardName>] location
= <install root>` entry (no trailing "data"); "Atys" is the live shard (see
ryztart's own `sAtys`/`dAtys`), the only one this module cares about.
"""

import configparser
from pathlib import Path
from typing import Optional

import appdirs

_RYZOM_INI_NAME = "ryzom.ini"
_ATYS_SECTION = "Atys"
_LOCATION_KEY = "location"
_DATA_SUBDIR = "data"
# Present in every real Ryzom Live "data" folder, cheap to stat -- used to
# sanity-check a candidate path (auto-detected or user-picked) actually
# contains what creature_ref.py's caches need, rather than just existing.
_SENTINEL_FILE = "creature.packed_sheets"


def _candidate_ryzom_user_dirs():
	# Roaming first, matching RyzomConfigIni.getLocationUser()'s own order.
	yield Path(appdirs.user_data_dir("Ryzom", "", roaming=True))
	yield Path(appdirs.user_data_dir("Ryzom", "", roaming=False))


def find_ryzom_ini() -> Optional[Path]:
	"""First existing `ryzom.ini` across the OS-specific candidate user-data
	directories, or None if ryztart/the official launcher was never run on
	this machine."""
	for directory in _candidate_ryzom_user_dirs():
		candidate = directory / _RYZOM_INI_NAME
		if candidate.is_file():
			return candidate
	return None


def is_valid_live_data_path(path) -> bool:
	"""True if `path` looks like a real Ryzom Live "data" folder -- present
	and containing at least the one sentinel file every install has."""
	if not path:
		return False
	return (Path(path) / _SENTINEL_FILE).is_file()


def detect_atys_live_data_path() -> Optional[Path]:
	"""The live (Atys) shard's "data" folder, read from `ryzom.ini`'s own
	`[Atys]` section, or None if `ryzom.ini` is missing, has no `[Atys]`
	section/location, or the resulting path doesn't actually hold real game
	data (e.g. a stale/moved install)."""
	ini_path = find_ryzom_ini()
	if ini_path is None:
		return None

	config = configparser.ConfigParser()
	try:
		config.read(ini_path, encoding="utf-8")
	except configparser.Error:
		return None

	if not config.has_option(_ATYS_SECTION, _LOCATION_KEY):
		return None
	location = config.get(_ATYS_SECTION, _LOCATION_KEY).strip()
	if not location:
		return None

	data_path = Path(location) / _DATA_SUBDIR
	return data_path if is_valid_live_data_path(data_path) else None
