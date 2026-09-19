"""Enumerates every real `.ig` file belonging to a continent, regardless of
which zone (if any) it's attached to, and whether that zone is currently
loaded or even a real, valid zone (project-todos/forgery/
landscape_editor__ig_full_load.md step 2) -- replaces the previous
per-visible-zone `.ig` lookup (region_loader.py's old get_ig_index()) plus
the `.continent` Villages-array zone attribution it needed on top of that
(both deleted the same chantier, step 1): a "village"/water/sky `.ig` whose
own name never matches any zone name (e.g. `tr_villagea.ig`) or whose
attributed zone isn't even a real, loadable zone (e.g. `tr_water.ig`'s
`Zone="218_di"`, confirmed 2026-09-13 invalid/off-grid) is picked up here
regardless, simply by being a real file physically present under one of the
continent's own `.ig`-holding locations.
"""

import re
from pathlib import Path
from typing import Dict, List, NamedTuple, Optional, Set, Tuple

from pynel import ryzom_continent
from pynel.ryzom_bnp import BnpError, BnpReader
from pynel.ryzom_georges_form import GeorgesFormParseError

from . import continent_pipeline_reference as cpr
from . import continent_selector
from . import region_loader

_bnp_reader_cache: Dict[str, BnpReader] = {}

# Same zone-name grid convention as region_loader.py's own _ZONE_NAME_RE
# (digits + 2 letters, e.g. "215_ed"/"208_EC") -- duplicated rather than
# imported to keep this module decoupled from region_loader.py's own
# (unrelated) zone-index caching concerns; a one-line regex, not worth a
# cross-import for.
_ZONE_NAME_RE = re.compile(r"^\d+_[A-Za-z]{2}$")

# Continent-level `.continent` ATOMs naming the sky/canopy `.ig` (project-
# todos/forgery/landscape_editor__ig_full_load.md's 2026-09-13 revision,
# Nuno: "Ce qu'il ne faut surtout pas charger c'est les ig de sky. ça ne
# sert à rien") -- confirmed 2026-09-13 against tryker.continent: `SkyIg`
# and the 4 `*CanopyIG` atoms all resolve to "canope_tryker.ig". Binary
# packed-sheets field names differ from the raw XML atom names (positional,
# not name-matched) -- `SkyIg` -> `ContinentParameters.background_ig_name`,
# the 4 `*CanopyIG` atoms -> `ContinentParameters.canopy_ig_file_name`.
_SKY_IG_XML_ATOMS = ("SkyIg", "SpringCanopyIG", "SummerCanopyIG", "AutumnCanopyIG", "WinterCanopyIG")


class FullIgRef(NamedTuple):
	"""Where one real `.ig` file lives, enough to read it on demand
	(read_ig_ref_bytes()) -- `name` is its own bare file name, NOT
	necessarily a zone name (e.g. "tr_villagea" as well as "208_ec")."""
	name: str
	source_path: Path  # a loose .ig file/directory entry, or a .bnp/.bnpe path
	bnp_entry: Optional[str]  # entry name inside source_path if packed, else None (loose file)


def _get_bnp_reader(path: Path) -> BnpReader:
	key = str(path)
	reader = _bnp_reader_cache.get(key)
	if reader is None:
		reader = BnpReader(path)
		_bnp_reader_cache[key] = reader
	return reader


def _ig_refs_from_bnp(bnp_path: Path) -> List[FullIgRef]:
	try:
		entries = _get_bnp_reader(bnp_path).list()
	except BnpError:
		return []
	refs = []
	for entry in entries:
		entry_path = Path(entry.name)
		if entry_path.suffix.lower() == ".ig":
			refs.append(FullIgRef(name=entry_path.stem, source_path=bnp_path, bnp_entry=entry.name))
	return refs


def _ig_refs_from_dir(directory: Path) -> List[FullIgRef]:
	if not directory.is_dir():
		return []
	return [
		FullIgRef(name=p.stem, source_path=p, bnp_entry=None)
		for p in sorted(directory.iterdir()) if p.is_file() and p.suffix.lower() == ".ig"
	]


def list_continent_ig_refs_visualisation(live_data_path, continent: str) -> List[FullIgRef]:
	"""Every `.ig` packed in `<continent>_ig.bnp` (or `.bnpe`) at the root of
	`live_data_path` -- confirmed 2026-09-13 present for most real
	continents on a real Ryzom Live install (bagne, corrupted_moor, fyros,
	fyros_island, fyros_newbie, indoors, matis, matis_island, newbieland,
	nexus, route_gouffre, sources, terre, tryker, tryker_island,
	tryker_newbie, undernexus, zorai, zorai_island); absent for a handful
	(kitiniere, the r2_* variants) -- an empty result there, not an error."""
	if not live_data_path:
		return []
	live_data_path = Path(live_data_path)
	for suffix in (".bnp", ".bnpe"):
		bnp_path = live_data_path / f"{continent}_ig{suffix}"
		if bnp_path.is_file():
			return _ig_refs_from_bnp(bnp_path)
	return []


def _ig_overrides(ryzom_data_path, continent: str) -> Dict[str, Path]:
	"""Bare name -> path for every working `.ig` copy under `assets_src/
	landscape/ig/<continent>/` (land) and `assets_src/landscape/ig_other/`
	(other) -- project-todos/forgery/landscape_editor__ig_editing.md,
	project-todos/forgery/pipeline_export_dirs_rename.md. Takes priority
	over the last real Build's own final output wherever a name matches, so
	Atyscape shows a hand edit immediately, before the next full Build
	re-bakes it in -- and surfaces a brand new name (added directly via the
	`.continent` sheet, Nuno 2026-09-15) the native pipeline doesn't produce
	at all. Path construction duplicated from `ig_editing.py` rather than
	imported: that module imports THIS one (`_find_live_ref()`), importing
	it back here would be circular."""
	overrides: Dict[str, Path] = {}
	for override_dir in (
		Path(ryzom_data_path) / "assets_src" / "landscape" / "ig" / continent,
		Path(ryzom_data_path) / "assets_src" / "landscape" / "ig_other",
	):
		if not override_dir.is_dir():
			continue
		overrides.update({p.stem: p for p in override_dir.iterdir() if p.is_file() and p.suffix.lower() == ".ig"})
	return overrides


def list_continent_ig_refs_edition(ryzom_data_path, continent: str) -> List[FullIgRef]:
	"""Every `.ig` file directly under the last real Build's `final_ig_dir`
	(`land_build.py`'s own final, game-ready output -- land AND other
	together, project-todos/forgery/pipeline_export_dirs_rename.md, this
	conversation's own starting point: files sitting only in the OLD
	`additionnal_ig_out_dir`/`ligo_ig_other` were never read here at all).
	Any name with a working copy under `_ig_overrides()` is served from
	there instead (existing name) or added on top (brand new name, not yet
	produced by any Build)."""
	if not ryzom_data_path:
		return []
	final_ig_dir = region_loader.continent_final_export_dir(ryzom_data_path, continent) / f"{continent}_ig"
	refs = _ig_refs_from_dir(final_ig_dir)
	overrides = _ig_overrides(ryzom_data_path, continent)
	if not overrides:
		return refs
	refs = [
		FullIgRef(name=ref.name, source_path=overrides[ref.name], bnp_entry=None) if ref.name in overrides else ref
		for ref in refs
	]
	known_names = {ref.name for ref in refs}
	refs += [
		FullIgRef(name=name, source_path=path, bnp_entry=None)
		for name, path in overrides.items() if name not in known_names
	]
	return refs


def list_continent_ig_refs(live_data_path, ryzom_data_path, is_edition: bool, continent: Optional[str]) -> List[FullIgRef]:
	"""Every real `.ig` ref for `continent`, from the source matching
	`is_edition` (see list_continent_ig_refs_visualisation()/
	list_continent_ig_refs_edition()'s own docstrings) -- `[]` if `continent`
	is unset."""
	if not continent:
		return []
	if is_edition:
		return list_continent_ig_refs_edition(ryzom_data_path, continent)
	return list_continent_ig_refs_visualisation(live_data_path, continent)


def read_ig_ref_bytes(ref: FullIgRef) -> bytes:
	"""Raw bytes for whichever real file `ref` points to (loose file or
	packed `.bnp`/`.bnpe` entry)."""
	if ref.bnp_entry is not None:
		return _get_bnp_reader(ref.source_path).read_file(ref.bnp_entry)
	return ref.source_path.read_bytes()


def _strip_ig_extension(name: str) -> str:
	return name[:-3] if name.lower().endswith(".ig") else name


def is_zone_owned(ref_name: str) -> bool:
	"""True if `ref_name` (a `.ig`'s own bare file name) follows the zone
	grid naming convention (e.g. "215_ed", "208_EC") -- region_loader.py's
	own zone-index scan already only ever surfaces real zones matching this
	exact pattern, so any `.ig` sharing that shape is a zone-owned one."""
	return bool(_ZONE_NAME_RE.match(ref_name))


def sky_ig_names_edition(ryzom_data_path, continent: str) -> Set[str]:
	"""Bare names (no `.ig` suffix) of `continent`'s sky/canopy `.ig`s, read
	directly from the raw `.continent` file's own ATOMs (see
	_SKY_IG_XML_ATOMS's own comment) -- empty set (never raises) if the
	continent file/pipeline row is unavailable or unparseable."""
	if not ryzom_data_path:
		return set()
	try:
		continent_file = cpr.build_land_export_config(ryzom_data_path, continent).continent_file
	except cpr.ContinentPipelineReferenceError:
		return set()
	try:
		cf = ryzom_continent.load_continent(continent_file)
	except (OSError, GeorgesFormParseError):
		return set()
	names = (cf.raw.root.atom(atom) for atom in _SKY_IG_XML_ATOMS)
	return {_strip_ig_extension(name) for name in names if name}


def sky_ig_names_visualisation(live_data_path, continent: str) -> Set[str]:
	"""Same as sky_ig_names_edition(), but from `continent.packed_sheets`'s
	already-typed `ContinentSheet.continent` (`background_ig_name`/
	`canopy_ig_file_name`, the binary fields `SkyIg`/`*CanopyIG` resolve
	into -- see _SKY_IG_XML_ATOMS's own comment)."""
	if not live_data_path:
		return set()
	try:
		sheet = continent_selector.load_continent_sheet(live_data_path, continent)
	except continent_selector.ContinentSelectorError:
		return set()
	names = [sheet.continent.background_ig_name] + list(sheet.continent.canopy_ig_file_name)
	return {_strip_ig_extension(name) for name in names if name}


def sky_ig_names(live_data_path, ryzom_data_path, is_edition: bool, continent: Optional[str]) -> Set[str]:
	"""sky_ig_names_edition()/sky_ig_names_visualisation(), whichever matches
	`is_edition` -- empty set if `continent` is unset."""
	if not continent:
		return set()
	if is_edition:
		return sky_ig_names_edition(ryzom_data_path, continent)
	return sky_ig_names_visualisation(live_data_path, continent)


def split_refs(refs: List[FullIgRef], sky_names: Set[str]) -> Tuple[List[FullIgRef], List[FullIgRef]]:
	"""Splits `refs` into `(zone_refs, rest_refs)` -- `zone_refs` are the
	ones is_zone_owned() recognizes, `rest_refs` everything else EXCEPT
	`sky_names` (dropped outright, never returned in either list) --
	project-todos/forgery/landscape_editor__ig_full_load.md's 2026-09-13
	revision: zone `.ig`s load per-region (landscape_editor.py's own
	_toggle_region()), the rest loads as one continent-wide bundle, sky
	never loads at all."""
	zone_refs = []
	rest_refs = []
	for ref in refs:
		if ref.name in sky_names:
			continue
		(zone_refs if is_zone_owned(ref.name) else rest_refs).append(ref)
	return zone_refs, rest_refs
