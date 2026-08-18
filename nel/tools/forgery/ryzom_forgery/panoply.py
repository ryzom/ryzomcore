"""Ryzom's "panoply" system: armor/equipment texture variants per race and
per "user color" slot. A material's own texture name (e.g.
"tr_hof_armor00_handupside_c1.tga") is never itself one of the variant files
on disk -- the real per-race/per-user-color files
("<base>_<race>_u<N>.tga") are listed in "panoply_files.txt" (found inside
`characters_maps_hr.bnp`, via the generic .bnp-aware search paths, see
search_paths.py/search_paths_dialog.py), one file name per line. This
module is just the pure parsing/naming logic -- no I/O, no UI.
"""

import re
from pathlib import Path
from typing import Dict, List

# The 4 playable Ryzom races' 2-letter codes, as used throughout the game's
# own asset file names (nel/src/3d/mesh_lightmapper.cpp's CPeople enum,
# ryzom/tools/make_anim_by_race/main.cpp's raceAnimNames -- fy/ma/tr/zo, in
# that order).
RACES = ("fy", "ma", "tr", "zo")

_VARIANT_RE = re.compile(r"^(?P<base>.+)_(?P<race>fy|ma|tr|zo)_u(?P<user_color>\d+)\.tga$", re.IGNORECASE)


def parse_panoply_files(text: str) -> Dict[str, Dict[str, List[int]]]:
	"""{base texture stem (lowercase, no extension): {race: [user color
	numbers, sorted]}} from panoply_files.txt's raw content (one file name
	per line, e.g. "tr_hof_armor00_handupside_c1_fy_u1.tga")."""
	variants: Dict[str, Dict[str, List[int]]] = {}
	for line in text.splitlines():
		name = line.strip()
		if not name:
			continue
		match = _VARIANT_RE.match(name)
		if not match:
			continue
		base = match.group("base").lower()
		race = match.group("race").lower()
		user_color = int(match.group("user_color"))
		variants.setdefault(base, {}).setdefault(race, []).append(user_color)
	for race_map in variants.values():
		for race in race_map:
			race_map[race] = sorted(set(race_map[race]))
	return variants


def variant_file_name(base_texture_name: str, race: str, user_color: int) -> str:
	"""The real file name for `base_texture_name` (e.g.
	"tr_hof_armor00_handupside_c1.tga") at (race, user_color) -- e.g.
	"tr_hof_armor00_handupside_c1_tr_u4.tga". The shape's own material data
	is never touched -- this is only used to resolve which file to actually
	load for rendering/preview."""
	path = Path(base_texture_name)
	suffix = path.suffix or ".tga"
	return f"{path.stem}_{race}_u{user_color}{suffix}"
