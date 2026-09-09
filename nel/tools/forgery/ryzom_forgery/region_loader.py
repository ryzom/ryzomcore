"""Region-based zone discovery for the Landscape Editor
(project-todos/forgery/landscape_editor.md step 6): finds every real
`.zone`/`.zonew`/`.zonel` whose 160x160 footprint overlaps a given
world-space region, wherever it's actually stored (loose file, or packed in
a top-level `.bnp`) under `live_data_path`.

A zone's world position comes from its own file name (the same
"<row>_<letters>" grid convention `zone_welder`/`zone_lighter` use, see
nel/tools/pynel/docs/zone_tools.md's "Zone naming/grid convention"), decoded
via `pynel.ryzom_packed_sheets.zone_name_to_world_pos()` -- this is a single
world-wide grid (confirmed via project-todos/forgery/
landscape_editor__continent-selector.md's own bounding-box work: a
continent occupies a rectangular sub-region of it, not its own separate
coordinate space), so a zone name found in any `.bnp` is safe to place in
this shared grid regardless of which continent/archive it came from.

Every real zone lives in some `.bnp`, but NOT always a `*_zones.bnp` --
confirmed 2026-09-07 against a real install: the r2_* (Ring/scenario)
continents keep theirs directly in e.g. `r2_desert.bnp`, alongside unrelated
content, so filtering by archive name would silently miss them. There's no
shortcut around opening every `.bnp`'s file table (cheap, header-only, no
zone content read -- see BnpReader) at least once; build_zone_index() below
pays that cost a single time per `live_data_path` and caches the result
in-process, since it never changes while the app is running -- callers doing
more than one region load (e.g. panning the camera and reloading) should
never re-pay the full scan.
"""

import re
from pathlib import Path
from typing import Dict, List, NamedTuple, Optional

from pynel.ryzom_bnp import BnpError, BnpReader
from pynel.ryzom_packed_sheets import PackedSheetsParseError, zone_name_to_world_pos
from pynel.ryzom_zone import parse_zone, Zone, ZoneParseError

from .zone_cache import read_zone_cache, write_zone_cache, ZoneCacheData
from .zone_geometry import zone_to_cache_data, ZONE_CELL_SIZE

_ZONE_NAME_RE = re.compile(r"^\d+_[A-Za-z]{2}$")
# Preference order when the same zone name exists under more than one
# extension (e.g. both a loose .zone and a packed .zonel) -- most-finished
# pipeline stage first, matching zone_welder/zone_lighter's own
# .zone -> .zonew -> .zonel progression (docs/zone_tools.md).
_ZONE_EXTENSIONS_BY_PRIORITY = (".zonel", ".zonew", ".zone")

# Under a `ryzom_data_path` checkout (callers resolve this via
# `pynel.repository_paths.get("ryzom-data")` -- the same generic per-user/
# per-machine repo path already used everywhere else in Forgery, never a
# second/duplicate setting of its own, project-todos/forgery/
# landscape_editor__zone_render_modes.md step 6, Nuno 2026-09-09), a
# continent may have a real build_gamedata pipeline export with per-stage
# zone data, unlike live_data_path which only ever ships the final stage.
# When it does, it REPLACES live_data_path as the zone source for that
# continent entirely (never merged with it) -- lets real multi-stage data be
# used/edited for testing (e.g. deleting a .zonew to exercise a WELD
# fallback) without ever touching the real live_data install.
_PIPELINE_EXPORT_CONTINENTS_SUBPATH = Path("pipeline") / "export" / "continents"
# Only these three actually hold .zone/.zonew/.zonel among the many
# subdirectories build_gamedata produces per continent (docs/
# atyscape_workspace.md's own full list) -- the others (rbank_*, ig_*, map_*,
# shape_*...) are unrelated to zone geometry.
_PIPELINE_EXPORT_ZONE_SUBDIRS = ("zone", "zone_weld", "zone_lighted")

# str(live_data_path) -> full index (every real zone found, unfiltered) --
# process-wide, never invalidated (the real game data doesn't change under a
# running app).
_index_cache: Dict[str, List["ZoneRef"]] = {}

# str(live_data_path) -> {zone name -> {extension -> ZoneRef}} -- every
# extension found for every zone, not just the best one (see _index_cache
# above); built in the same scan pass as _index_cache, for callers that need
# to know which pipeline stages actually exist on disk for a zone (e.g.
# landscape_editor.py's [WELD]/[LIGHT] render mode status per zone).
_extensions_index_cache: Dict[str, Dict[str, Dict[str, "ZoneRef"]]] = {}

# ZoneRef -> already-parsed Zone -- reloading the same region (e.g. after
# just adjusting the radius) re-read and re-parsed every zone from scratch
# every single time, dominating "Load region"'s cost even with the index
# above cached (found 2026-09-07, Nuno: still slow the 2nd time on the same
# zones). ZoneRef is a plain-value NamedTuple, hashable as-is.
_zone_cache: Dict["ZoneRef", Zone] = {}

# str(bnp_path) -> BnpReader -- a region's zones are often clustered in the
# same one or two .bnp archives; without this, every single zone in the
# region re-opened and re-read that archive's whole header table from
# scratch (BnpReader.__init__), on top of the index build already having
# read it once.
_bnp_reader_cache: Dict[str, BnpReader] = {}


def _get_bnp_reader(bnp_path: Path) -> BnpReader:
	key = str(bnp_path)
	reader = _bnp_reader_cache.get(key)
	if reader is None:
		reader = BnpReader(bnp_path)
		_bnp_reader_cache[key] = reader
	return reader


class RegionLoadError(Exception):
	pass


class ZoneRef(NamedTuple):
	"""Where one real zone actually lives, plus its decoded world position --
	enough to load it on demand (load_zone_ref()) without holding its data."""
	name: str  # bare zone name, e.g. "55_CC"
	x: float
	y: float
	source_path: Path  # a loose .zone*/.bnp path
	bnp_entry: Optional[str]  # entry name inside source_path if packed, else None (loose file)


def _consider(
	best: Dict[str, tuple], all_by_name: Dict[str, Dict[str, "ZoneRef"]],
	name: str, ext: str, source_path: Path, bnp_entry,
) -> None:
	if not _ZONE_NAME_RE.match(name):
		return
	try:
		pos = zone_name_to_world_pos(name)
	except PackedSheetsParseError:
		return
	ref = ZoneRef(name=name, x=pos.x, y=pos.y, source_path=source_path, bnp_entry=bnp_entry)
	all_by_name.setdefault(name, {})[ext] = ref
	priority = _ZONE_EXTENSIONS_BY_PRIORITY.index(ext)
	existing = best.get(name)
	if existing is None or priority < existing[0]:
		best[name] = (priority, ref)


def _build_zone_index(live_data_path: Path) -> tuple:
	best: Dict[str, tuple] = {}
	all_by_name: Dict[str, Dict[str, ZoneRef]] = {}
	for entry in live_data_path.iterdir():
		if not entry.is_file():
			continue
		suffix = entry.suffix.lower()
		if suffix in _ZONE_EXTENSIONS_BY_PRIORITY:
			_consider(best, all_by_name, entry.stem, suffix, entry, None)
		elif suffix in (".bnp", ".bnpe"):
			try:
				bnp_entries = _get_bnp_reader(entry).list()
			except BnpError:
				continue
			for bnp_entry in bnp_entries:
				entry_path = Path(bnp_entry.name)
				entry_suffix = entry_path.suffix.lower()
				if entry_suffix in _ZONE_EXTENSIONS_BY_PRIORITY:
					_consider(best, all_by_name, entry_path.stem, entry_suffix, entry, bnp_entry.name)
	return [ref for _, ref in best.values()], all_by_name


def _pipeline_export_zone_dirs(ryzom_data_path, continent_name: str) -> Optional[List[Path]]:
	"""The real zone/zone_weld/zone_lighted subdirectories of a build_gamedata
	pipeline export for `continent_name`, if `ryzom_data_path` is set and that
	continent has one under it -- None otherwise (caller falls back to
	live_data_path)."""
	if not ryzom_data_path:
		return None
	continent_dir = Path(ryzom_data_path) / _PIPELINE_EXPORT_CONTINENTS_SUBPATH / continent_name
	if not continent_dir.is_dir():
		return None
	dirs = [continent_dir / name for name in _PIPELINE_EXPORT_ZONE_SUBDIRS]
	return [d for d in dirs if d.is_dir()]


def has_pipeline_export(ryzom_data_path, continent_name: str) -> bool:
	"""True if `continent_name` has a real build_gamedata pipeline export
	under `ryzom_data_path` with at least one zone/zone_weld/zone_lighted
	subdirectory -- callers use this to skip requiring live_data_path to be
	configured/valid at all for such a continent (its zones never come from
	there, see get_zone_index()'s own docstring)."""
	return _pipeline_export_zone_dirs(ryzom_data_path, continent_name) is not None


def _build_zone_index_from_dirs(zone_dirs: List[Path]) -> tuple:
	"""Same merge logic as _build_zone_index(), but over a fixed list of
	plain loose-file directories (a pipeline export's zone/zone_weld/
	zone_lighted) instead of one live_data_path root plus its `.bnp`s."""
	best: Dict[str, tuple] = {}
	all_by_name: Dict[str, Dict[str, ZoneRef]] = {}
	for zone_dir in zone_dirs:
		for entry in zone_dir.iterdir():
			if not entry.is_file():
				continue
			suffix = entry.suffix.lower()
			if suffix in _ZONE_EXTENSIONS_BY_PRIORITY:
				_consider(best, all_by_name, entry.stem, suffix, entry, None)
	return [ref for _, ref in best.values()], all_by_name


def get_zone_index(live_data_path, continent_name: Optional[str] = None, ryzom_data_path=None) -> List[ZoneRef]:
	"""Every real zone found for `continent_name` under `ryzom_data_path`'s
	pipeline export (or under `live_data_path` unfiltered if no continent is
	given, or if that continent has no real pipeline export -- see
	_pipeline_export_zone_dirs()), scanning every loose file and every
	`.bnp`'s header table once per process (see module docstring) -- safe to
	call from a background thread (pure file I/O, no Panda3D/imgui calls). A
	pipeline-export-backed continent is intentionally NEVER cached (unlike
	live_data_path, real test data there can change between calls -- e.g.
	Nuno deleting a .zonew to exercise a WELD fallback), so it's always
	rescanned fresh."""
	if continent_name is not None:
		zone_dirs = _pipeline_export_zone_dirs(ryzom_data_path, continent_name)
		if zone_dirs is not None:
			index, _ = _build_zone_index_from_dirs(zone_dirs)
			return index

	key = str(live_data_path)
	index = _index_cache.get(key)
	if index is None:
		index, all_by_name = _build_zone_index(Path(live_data_path))
		_index_cache[key] = index
		_extensions_index_cache[key] = all_by_name
	return index


def get_zone_extensions_index(
	live_data_path, continent_name: Optional[str] = None, ryzom_data_path=None,
) -> Dict[str, Dict[str, ZoneRef]]:
	"""Every extension actually found on disk for every real zone for
	`continent_name` (or under `live_data_path` -- see get_zone_index()'s own
	docstring for the fallback/caching rules), zone name -> extension ->
	ZoneRef, unlike get_zone_index() which keeps only the most-advanced one
	per zone -- for callers that need to know which pipeline stages exist
	(e.g. landscape_editor.py's per-zone [WELD]/[LIGHT] render mode status)."""
	if continent_name is not None:
		zone_dirs = _pipeline_export_zone_dirs(ryzom_data_path, continent_name)
		if zone_dirs is not None:
			_, all_by_name = _build_zone_index_from_dirs(zone_dirs)
			return all_by_name

	get_zone_index(live_data_path)  # ensures both caches are populated
	return _extensions_index_cache[str(live_data_path)]


def zone_ref_extension(ref: ZoneRef) -> str:
	"""The real extension of the zone `ref` points to (e.g. ".zonew") -- for a
	packed zone, `ref.source_path` is the containing `.bnp`/`.bnpe`, so the
	actual extension must come from the entry name (`ref.bnp_entry`)
	instead."""
	if ref.bnp_entry is not None:
		return Path(ref.bnp_entry).suffix.lower()
	return ref.source_path.suffix.lower()


def find_zones_in_region(
	live_data_path, min_x: float, min_y: float, max_x: float, max_y: float,
	continent_name: Optional[str] = None, ryzom_data_path=None,
) -> List[ZoneRef]:
	"""Every real zone from get_zone_index(live_data_path, continent_name,
	ryzom_data_path) whose 160x160 footprint overlaps
	[min_x, max_x) x [min_y, max_y)."""
	index = get_zone_index(live_data_path, continent_name, ryzom_data_path)
	return [
		ref for ref in index
		if not (ref.x + ZONE_CELL_SIZE <= min_x or ref.x >= max_x or ref.y + ZONE_CELL_SIZE <= min_y or ref.y >= max_y)
	]


def read_zone_ref_bytes(ref: ZoneRef) -> bytes:
	"""Raw bytes for whichever real file `ref` points to (loose file or
	packed `.bnp` entry) -- unparsed, for callers that need the bytes
	themselves rather than a parsed Zone (e.g. zone_tools.py writing them
	back out under a different extension for zone_welder)."""
	if ref.bnp_entry is not None:
		try:
			return _get_bnp_reader(ref.source_path).read_file(ref.bnp_entry)
		except BnpError as exc:
			raise RegionLoadError(f"{ref.name}: {exc}")
	try:
		return ref.source_path.read_bytes()
	except OSError as exc:
		raise RegionLoadError(f"{ref.name}: {exc}")


def load_zone_ref(ref: ZoneRef) -> Zone:
	"""Loads and parses the zone `ref` points to -- cached by `ref` itself
	(see _zone_cache) so reloading the same region (e.g. after just
	adjusting the radius) doesn't re-read and re-parse every zone from
	scratch every time."""
	cached = _zone_cache.get(ref)
	if cached is not None:
		return cached

	data = read_zone_ref_bytes(ref)
	try:
		zone = parse_zone(data)
	except ZoneParseError as exc:
		raise RegionLoadError(f"{ref.name}: {exc}")
	_zone_cache[ref] = zone
	return zone


def load_zone_cache_data(ref: ZoneRef) -> ZoneCacheData:
	"""Cache-first replacement for load_zone_ref() + compute_zone_patch_positions()
	(project-todos/forgery/landscape_editor__zone_disk_cache.md step 4) --
	pure data, safe to call from a background thread like load_zone_ref()
	itself (never touches Panda3D; building the actual GeomNode from the
	result is the caller's job, build_zone_geom_from_cache() in
	zone_geometry.py, step 2). Reads the disk cache if present and still
	fresh against ref.source_path (zone_cache.py), else parses+tessellates
	the real zone once and writes the cache for next time."""
	extension = zone_ref_extension(ref)
	cache_data = read_zone_cache(ref.name, extension, ref.source_path)
	if cache_data is not None:
		return cache_data

	zone = load_zone_ref(ref)
	cache_data = zone_to_cache_data(zone)
	try:
		write_zone_cache(ref.name, extension, ref.source_path, cache_data)
	except OSError:
		# Best-effort: the cache is a pure perf optimization, a write
		# failure (disk full, permissions) must not block rendering the
		# zone with what was just computed.
		pass
	return cache_data
