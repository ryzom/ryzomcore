"""Disk cache of tessellated zone positions (project-todos/forgery/
landscape_editor__zone_disk_cache.md step 1) -- caches ONLY the tessellated
Bezier positions (the expensive part: bnp read + parse_zone + the per-vertex
`_eval_bezier_patch()` loop in zone_geometry.py), never color/normals/UV/
triangle indices, which are all cheap to derive from these positions alone
and must stay that way -- so future rendering changes (lighting step 9,
texturing step 11 of landscape_editor.md) never invalidate this cache, see
the sub-chantier's scope decisions.
"""

import pickle
from pathlib import Path
from typing import NamedTuple, Optional, Tuple

from .config_dir import config_dir

_CACHE_DIR_NAME = "zone_cache"
# Bumped whenever ZoneCacheData's own shape changes (not for rendering
# changes downstream -- those never touch this cache, see module docstring)
# -- any envelope written under an older version is treated as absent.
_CACHE_FORMAT_VERSION = 1


class PatchPositions(NamedTuple):
	"""One patch's tessellated grid, flattened in build_zone_tessellated_geom()'s
	own row-major order (i in range(n_s+1): j in range(n_t+1)) -- a flat
	tuple of floats pickles as a single buffer, not one Python float object
	per component."""
	n_s: int
	n_t: int
	positions: Tuple[float, ...]  # x0,y0,z0, x1,y1,z1, ... -- len == 3*(n_s+1)*(n_t+1)


class ZoneCacheData(NamedTuple):
	"""Everything needed to rebuild a zone's GeomNode (zone_geometry.py)
	without ever touching the source `.zone*`/`.bnp` again -- color
	(elevation), triangle indices, and any future normals/UV are all
	derived from this at load time, never stored here."""
	patches: Tuple[PatchPositions, ...]
	bb_center: Tuple[float, float, float]
	bb_half_size: Tuple[float, float, float]


class _CacheEnvelope(NamedTuple):
	"""On-disk pickle payload -- format_version + the source file's
	freshness stamp wrap the actual ZoneCacheData."""
	format_version: int
	source_mtime: float
	source_size: int
	data: ZoneCacheData


def _cache_path(zone_name: str, extension: str) -> Path:
	# extension includes its leading "." (e.g. ".zonew") -- stripped here so
	# the cache file name stays a single clean "<name>__<ext>.zonecache"
	# rather than doubling up dots.
	return config_dir() / _CACHE_DIR_NAME / f"{zone_name}__{extension.lstrip('.')}.zonecache"


def write_zone_cache(zone_name: str, extension: str, source_path: Path, data: ZoneCacheData) -> None:
	"""Writes/overwrites the disk cache for `zone_name`'s `extension` (e.g.
	".zonew"), stamped with `source_path`'s current mtime/size -- its
	`.zone`/`.zonew`/`.zonel` file if loose, or the whole `.bnp`/`.bnpe` if
	the zone is packed (a single entry inside an archive can't be dated on
	its own, see the sub-chantier's scope decisions). `extension` is part of
	the cache key (region_loader.zone_ref_extension()) so switching a zone's
	render mode between raw/welded/lit never serves a stale geometry from
	another mode's cache entry."""
	stat = source_path.stat()
	envelope = _CacheEnvelope(
		format_version=_CACHE_FORMAT_VERSION,
		source_mtime=stat.st_mtime,
		source_size=stat.st_size,
		data=data,
	)
	path = _cache_path(zone_name, extension)
	path.parent.mkdir(parents=True, exist_ok=True)
	# Written to a temp file then renamed into place -- an interrupted write
	# (crash, kill) must never leave a half-written file that
	# read_zone_cache() would then try (and fail) to unpickle next run.
	tmp_path = path.with_suffix(path.suffix + ".tmp")
	with open(tmp_path, "wb") as f:
		pickle.dump(envelope, f, protocol=pickle.HIGHEST_PROTOCOL)
	tmp_path.replace(path)


def read_zone_cache(zone_name: str, extension: str, source_path: Path) -> Optional[ZoneCacheData]:
	"""Returns the cached ZoneCacheData for `zone_name`'s `extension` if a
	cache file exists AND `source_path`'s current mtime/size still match what
	was stamped at write_zone_cache() time -- None otherwise (missing, stale
	format/source, or unreadable), meaning the caller must rebuild it from
	the real source and call write_zone_cache() again."""
	path = _cache_path(zone_name, extension)
	try:
		with open(path, "rb") as f:
			envelope = pickle.load(f)
	except (OSError, pickle.UnpicklingError, EOFError, AttributeError, ValueError):
		return None
	if not isinstance(envelope, _CacheEnvelope) or envelope.format_version != _CACHE_FORMAT_VERSION:
		return None
	try:
		stat = source_path.stat()
	except OSError:
		return None
	if envelope.source_mtime != stat.st_mtime or envelope.source_size != stat.st_size:
		return None
	return envelope.data
