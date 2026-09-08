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
from .zone_geometry import zone_to_cache_data

ZONE_CELL_SIZE = 160.0
_ZONE_NAME_RE = re.compile(r"^\d+_[A-Za-z]{2}$")
# Preference order when the same zone name exists under more than one
# extension (e.g. both a loose .zone and a packed .zonel) -- most-finished
# pipeline stage first, matching zone_welder/zone_lighter's own
# .zone -> .zonew -> .zonel progression (docs/zone_tools.md).
_ZONE_EXTENSIONS_BY_PRIORITY = (".zonel", ".zonew", ".zone")

# str(live_data_path) -> full index (every real zone found, unfiltered) --
# process-wide, never invalidated (the real game data doesn't change under a
# running app).
_index_cache: Dict[str, List["ZoneRef"]] = {}

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


def _consider(best: Dict[str, tuple], name: str, ext: str, source_path: Path, bnp_entry) -> None:
	if not _ZONE_NAME_RE.match(name):
		return
	try:
		pos = zone_name_to_world_pos(name)
	except PackedSheetsParseError:
		return
	priority = _ZONE_EXTENSIONS_BY_PRIORITY.index(ext)
	existing = best.get(name)
	if existing is None or priority < existing[0]:
		best[name] = (priority, ZoneRef(name=name, x=pos.x, y=pos.y, source_path=source_path, bnp_entry=bnp_entry))


def _build_zone_index(live_data_path: Path) -> List[ZoneRef]:
	best: Dict[str, tuple] = {}
	for entry in live_data_path.iterdir():
		if not entry.is_file():
			continue
		suffix = entry.suffix.lower()
		if suffix in _ZONE_EXTENSIONS_BY_PRIORITY:
			_consider(best, entry.stem, suffix, entry, None)
		elif suffix in (".bnp", ".bnpe"):
			try:
				bnp_entries = _get_bnp_reader(entry).list()
			except BnpError:
				continue
			for bnp_entry in bnp_entries:
				entry_path = Path(bnp_entry.name)
				entry_suffix = entry_path.suffix.lower()
				if entry_suffix in _ZONE_EXTENSIONS_BY_PRIORITY:
					_consider(best, entry_path.stem, entry_suffix, entry, bnp_entry.name)
	return [ref for _, ref in best.values()]


def get_zone_index(live_data_path) -> List[ZoneRef]:
	"""Every real zone found under `live_data_path`, scanning every loose
	file and every `.bnp`'s header table once per process (see module
	docstring) -- safe to call from a background thread (pure file I/O, no
	Panda3D/imgui calls)."""
	key = str(live_data_path)
	index = _index_cache.get(key)
	if index is None:
		index = _build_zone_index(Path(live_data_path))
		_index_cache[key] = index
	return index


def find_zones_in_region(
	live_data_path, min_x: float, min_y: float, max_x: float, max_y: float,
) -> List[ZoneRef]:
	"""Every real zone from get_zone_index(live_data_path) whose 160x160
	footprint overlaps [min_x, max_x) x [min_y, max_y)."""
	index = get_zone_index(live_data_path)
	return [
		ref for ref in index
		if not (ref.x + ZONE_CELL_SIZE <= min_x or ref.x >= max_x or ref.y + ZONE_CELL_SIZE <= min_y or ref.y >= max_y)
	]


def load_zone_ref(ref: ZoneRef) -> Zone:
	"""Loads and parses the zone `ref` points to -- cached by `ref` itself
	(see _zone_cache) so reloading the same region (e.g. after just
	adjusting the radius) doesn't re-read and re-parse every zone from
	scratch every time."""
	cached = _zone_cache.get(ref)
	if cached is not None:
		return cached

	if ref.bnp_entry is not None:
		try:
			data = _get_bnp_reader(ref.source_path).read_file(ref.bnp_entry)
		except BnpError as exc:
			raise RegionLoadError(f"{ref.name}: {exc}")
	else:
		try:
			data = ref.source_path.read_bytes()
		except OSError as exc:
			raise RegionLoadError(f"{ref.name}: {exc}")
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
	cache_data = read_zone_cache(ref.name, ref.source_path)
	if cache_data is not None:
		return cache_data

	zone = load_zone_ref(ref)
	cache_data = zone_to_cache_data(zone)
	try:
		write_zone_cache(ref.name, ref.source_path, cache_data)
	except OSError:
		# Best-effort: the cache is a pure perf optimization, a write
		# failure (disk full, permissions) must not block rendering the
		# zone with what was just computed.
		pass
	return cache_data
