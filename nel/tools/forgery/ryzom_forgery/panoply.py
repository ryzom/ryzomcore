"""Ryzom's "panoply" system: pre-baked texture recolors, generated at build
time (nel/tools/3d/panoply_maker) from a base texture plus one grayscale mask
per axis (mask weight in the mask's red channel) -- each present mask adds one
"_<color_id>" suffix to the output file name, so a texture with N masks gets
every combination of their color IDs. The 4 axes seen in real production data
(ryzom-data/leveldesign/workspace/common/characters_maps_hr/panoply_*.cfg):
"skin" (race skin tone -- FY/MA/TR/ZO, shared across every race's build),
"user" (the item's craft color -- U1..U8, likewise shared), and "hair"/"eyes"
(H1..H6/E1..E8, defined per-race since hair/eye hues differ by race, but only
relevant to head/face textures -- an armor piece never has these masks).

A given base texture only ever carries whichever axes its own mask files
existed for at build time (e.g. armor: race+user; a hairstyle: hair alone) --
"<base>.tga" with no variant on some axis just means that axis's mask never
existed for it, not missing data. The full list of every variant actually
built is "panoply_files.txt" (found inside `characters_maps_hr.bnp`, via the
generic .bnp-aware search paths, see search_paths.py/search_paths_dialog.py),
one file name per line. This module is just the pure parsing/naming logic --
no I/O, no UI.
"""

import re
from pathlib import Path
from typing import Dict, List, Optional

# Axis name -> (regex matching one filename token for this axis, letter used
# to build a variant file name back from a value). "skin" tokens are the
# literal 2-letter race codes themselves (FY/MA/TR/ZO), not a letter+digit
# pair like the other 3 axes -- see panoply_common.cfg's skin_color_id.
RACES = ("fy", "ma", "tr", "zo")
AXES = ("skin", "user", "hair", "eyes")

_TOKEN_PATTERNS = {
	"skin": re.compile(r"^(fy|ma|tr|zo)$", re.IGNORECASE),
	"user": re.compile(r"^u(\d+)$", re.IGNORECASE),
	"hair": re.compile(r"^h(\d+)$", re.IGNORECASE),
	"eyes": re.compile(r"^e(\d+)$", re.IGNORECASE),
}
_AXIS_LETTER = {"user": "U", "hair": "H", "eyes": "E"}


def _classify_token(token: str):
	"""(axis, value) for one "_"-separated file-name token, or None if it
	doesn't look like any known axis's color ID -- `value` is the race code
	lowercased for "skin", an int for the other 3 axes."""
	for axis, pattern in _TOKEN_PATTERNS.items():
		match = pattern.match(token)
		if match:
			value = match.group(1).lower() if axis == "skin" else int(match.group(1))
			return axis, value
	return None


def parse_panoply_files(text: str) -> Dict[str, Dict[str, List]]:
	"""{base texture stem (lowercase, no extension): {axis: [values, sorted]}}
	from panoply_files.txt's raw content (one file name per line, e.g.
	"tr_hof_armor00_handupside_c1_FY_U1.tga"). Trailing "_<color_id>" tokens
	are stripped one at a time (in whatever order they appear -- real output
	order is skin/user/hair/eyes, but nothing here depends on that) until one
	doesn't match a known axis; what's left is the base name. A line with no
	recognizable suffix at all (the un-panoplied base texture itself,
	routinely also listed) contributes nothing."""
	variants: Dict[str, Dict[str, List]] = {}
	for line in text.splitlines():
		name = line.strip()
		if not name.lower().endswith(".tga"):
			continue
		tokens = name[:-len(".tga")].split("_")

		found: Dict[str, object] = {}
		while tokens:
			classified = _classify_token(tokens[-1])
			if classified is None or classified[0] in found:
				break
			axis, value = classified
			found[axis] = value
			tokens.pop()
		if not found or not tokens:
			continue

		base = "_".join(tokens).lower()
		entry = variants.setdefault(base, {axis: set() for axis in AXES})
		for axis, value in found.items():
			entry[axis].add(value)

	for entry in variants.values():
		for axis in AXES:
			entry[axis] = sorted(entry[axis])
	return variants


def variant_file_name(base_texture_name: str, **dims: Dict[str, object]) -> str:
	"""The real file name for `base_texture_name` (e.g.
	"tr_hof_armor00_handupside_c1.tga") carrying the given axis values (e.g.
	`skin="tr", user=4` -> "..._TR_U4.tga") -- only the axes passed are
	appended, in AXES order, so a texture missing some axis's mask can still
	be resolved correctly by only passing the axes it actually has (see
	object_editor.py's _resolve_panoply_texture_name()). The shape's own
	material data is never touched -- this is only used to resolve which
	file to actually load for rendering/preview."""
	path = Path(base_texture_name)
	suffix = path.suffix or ".tga"
	parts = [path.stem]
	for axis in AXES:
		value = dims.get(axis)
		if value is None:
			continue
		parts.append(value.upper() if axis == "skin" else f"{_AXIS_LETTER[axis]}{value}")
	return "_".join(parts) + suffix
