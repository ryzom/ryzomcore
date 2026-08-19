"""Lookup for Panoply's real color-shift parameters -- reads the production
constants snapshotted in panoply_colors.toml (see that file's header for
where they came from and how to regenerate it if ryzom-data's palette ever
changes). No I/O beyond reading this one bundled file, so this module -- and
whatever calls it -- doesn't depend on the user's search paths covering
ryzom-data's leveldesign/ tree. See .todo/forgery-object-editor.md
"génération live des textures Panoply" and panoply_colorize.py, which these
parameters feed directly (hue/lightness/saturation/luminosity/contrast ->
convert_bitmap()'s matching arguments).
"""

from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

import tomlkit

_TOML_PATH = Path(__file__).parent / "panoply_colors.toml"

# 2-letter race prefix, as seen at the start of a base texture's file name
# (e.g. "tr_hom_armor00_epaule_c1.tga") -> the race table name below, which
# matches the source panoply_<race>.cfg's own file name.
RACE_PREFIX_TO_TABLE = {"fy": "fyros", "ma": "matis", "tr": "tryker", "zo": "zorai", "ge": "generique"}

# Axes common to every race/texture (panoply_common.cfg) vs. ones that need
# a race table resolved first (panoply_<race>.cfg) -- matches panoply.AXES.
_COMMON_AXES = ("skin", "user")
_RACE_AXES = ("hair", "eyes")


@dataclass(frozen=True)
class ColorParams:
	"""One axis/color_id's target parameters -- feeds directly into
	panoply_colorize.convert_bitmap()'s hue/lightness/saturation/
	luminosity/contrast arguments."""

	id: str
	hue: float
	lightness: float
	saturation: float
	luminosity: float
	contrast: float


def _load_doc():
	with open(_TOML_PATH, "r", encoding="utf-8") as f:
		return tomlkit.parse(f.read())


_DOC = _load_doc()


def _table_for(axis: str, race: Optional[str]):
	if axis in _COMMON_AXES:
		return _DOC.get(axis)
	if axis not in _RACE_AXES or race is None:
		return None
	race_table = _DOC.get(race)
	if race_table is None:
		return None
	return race_table.get(axis)


def get_color_params(axis: str, color_id: str, race: Optional[str] = None) -> Optional[ColorParams]:
	"""race must be one of RACE_PREFIX_TO_TABLE's values (e.g. "fyros"),
	required for "hair"/"eyes", ignored for "skin"/"user". Returns None if
	this axis doesn't apply to that race at all (zorai has no "eyes",
	"generique" has neither "hair" nor "eyes") or color_id isn't found."""
	table = _table_for(axis, race)
	if table is None:
		return None
	for entry in table.get("color", []):
		if entry["id"] == color_id:
			return ColorParams(
				id=entry["id"],
				hue=float(entry["hue"]),
				lightness=float(entry["lightness"]),
				saturation=float(entry["saturation"]),
				luminosity=float(entry["luminosity"]),
				contrast=float(entry["contrast"]),
			)
	return None


def available_color_ids(axis: str, race: Optional[str] = None) -> List[str]:
	"""Every color_id defined for this axis, optionally scoped to one
	race's hair/eyes table -- e.g. to build a picker UI. Empty if this axis
	doesn't apply to that race."""
	table = _table_for(axis, race)
	if table is None:
		return []
	return [entry["id"] for entry in table.get("color", [])]
