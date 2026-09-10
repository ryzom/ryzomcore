"""Continent selector + real region bounds, for the Landscape Editor
(project-todos/forgery/landscape_editor__continent-selector.md).

Loads world.packed_sheets (the continent list, CWorldSheet.cont_locs) and
continent.packed_sheets (per-continent zone_min/zone_max) from the Ryzom
Live client's real game data (see live_data.py), resolving a continent's
real world-space bounding box the same way CContinent::getCorners() does
(ryzom-core continent.cpp:845-867): ContinentParameters.zone_min/zone_max
are zone NAMES, not raw coordinates (see
pynel.ryzom_packed_sheets.zone_name_to_world_pos()'s own docstring) -- each
decoded to its origin-corner position, swapped per axis if reversed (a
zone's letter/digit order isn't guaranteed min-to-max), and +160 applied to
the max corner only (a zone name gives its origin corner, not its extent).
"""

from pathlib import Path
from typing import List, Tuple, Union

from pynel import ryzom_packed_sheets as ps
from pynel import ryzom_world
from pynel.ryzom_bnp import BnpError, BnpReader
from pynel.ryzom_georges_form import GeorgesFormParseError

from ryzom_forgery import creature_ref

# Confirmed on a real Ryzom Live install (2026-09-07): sheet_id.bin is NOT a
# loose file in the data folder (unlike creature.packed_sheets/
# world.packed_sheets/continent.packed_sheets) -- it only exists packed
# inside leveldesign.bnp.
_SHEET_ID_BNP = "leveldesign.bnp"


class ContinentSelectorError(Exception):
	pass


def _load_sheet_id_bytes(live_data_path: Path) -> bytes:
	flat_path = live_data_path / "sheet_id.bin"
	if flat_path.is_file():
		return flat_path.read_bytes()
	bnp_path = live_data_path / _SHEET_ID_BNP
	if not bnp_path.is_file():
		raise ContinentSelectorError(f"sheet_id.bin not found loose, and {_SHEET_ID_BNP} doesn't exist either")
	try:
		return BnpReader(bnp_path).read_file("sheet_id.bin")
	except BnpError as exc:
		raise ContinentSelectorError(f"sheet_id.bin not found in {_SHEET_ID_BNP}: {exc}")


def load_continent_locations(live_data_path: Union[str, Path]) -> List[ps.ContLoc]:
	"""ContLocs from world.packed_sheets's single real entry ("ryzom.world"),
	sorted by selection_name for a stable dropdown order."""
	world_sheets = ps.load_world_packed_sheets(Path(live_data_path) / "world.packed_sheets")
	if not world_sheets.entries:
		raise ContinentSelectorError("world.packed_sheets has no entries")
	world = next(iter(world_sheets.entries.values()))
	return sorted(world.cont_locs, key=lambda c: c.selection_name)


def load_continent_locations_from_world_file(world_file_path: Union[str, Path]) -> List[ps.ContLoc]:
	"""ContLocs read directly from a `ryzom.world` file under a `ryzom-data`
	checkout (Edition mode, project-todos/forgery/
	landscape_editor__land_preview.md step 2) -- no live_data_path/
	world.packed_sheets involved. Uses `WorldContinentEntry.struct_name` as
	the continent identifier (same semantics as ContLoc.continent_name, e.g.
	"matis") -- deliberately NOT `WorldContinentEntry.continent_name`, which
	`ryzom_world.py` documents as unreliable (e.g. the "matis" entry's own
	`continent_name` field reads "lesfalaises")."""
	try:
		world_file = ryzom_world.load_world(world_file_path)
	except (OSError, GeorgesFormParseError) as exc:
		raise ContinentSelectorError(f"Failed to load {world_file_path}: {exc}")
	locs = [
		ps.ContLoc(
			selection_name=entry.selection_name or entry.struct_name,
			continent_name=entry.struct_name,
			min_x=float(entry.minx or 0),
			min_y=float(entry.miny or 0),
			max_x=float(entry.maxx or 0),
			max_y=float(entry.maxy or 0),
		)
		for entry in world_file.continents
		if entry.struct_name is not None
	]
	return sorted(locs, key=lambda c: c.selection_name)


def resolve_continent_bounds(live_data_path: Union[str, Path], continent_name: str) -> Tuple[float, float, float, float]:
	"""Real world-space bounding box (min_x, min_y, max_x, max_y) for
	`continent_name` (ContLoc.continent_name, e.g. "matis" -- confirmed on
	real data, 2026-09-07: this field is the bare sheet stem, no extension;
	sheet_id.bin itself keys it with ".continent" appended, e.g.
	"matis.continent") -- see module docstring for the zone-name-to-bbox
	formula."""
	live_data_path = Path(live_data_path)
	sheet_id_names = ps.parse_sheet_id_bin(_load_sheet_id_bytes(live_data_path))
	name_to_id = creature_ref.build_name_to_id(sheet_id_names)
	full_name = f"{continent_name}.continent"
	sheet_id = name_to_id.get(full_name.lower())
	if sheet_id is None:
		raise ContinentSelectorError(f"{full_name!r} not found in sheet_id.bin")

	continent_sheets = ps.load_continent_packed_sheets(live_data_path / "continent.packed_sheets")
	sheet = continent_sheets.entries.get(sheet_id)
	if sheet is None:
		raise ContinentSelectorError(f"{full_name!r} (sheet_id {sheet_id}) not found in continent.packed_sheets")

	zone_min = ps.zone_name_to_world_pos(sheet.continent.zone_min)
	zone_max = ps.zone_name_to_world_pos(sheet.continent.zone_max)
	min_x, max_x = sorted((zone_min.x, zone_max.x))
	min_y, max_y = sorted((zone_min.y, zone_max.y))
	return min_x, min_y, max_x + 160.0, max_y + 160.0
