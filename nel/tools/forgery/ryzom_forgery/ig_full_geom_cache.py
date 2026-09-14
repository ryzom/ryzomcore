"""Disk cache of fully-built, textured `.ig` `NodePath` bundles (project-todos/
forgery/landscape_editor__ig_full_load.md) -- one `.bam` per `(continent,
mode, region)`, where `region=None` means the continent-wide "rest" bundle
(every real `.ig` that isn't zone-owned nor sky/canopy, project-todos/forgery/
landscape_editor__ig_full_load.md's 2026-09-13 revision) and a real region
name means that region's own zone-owned `.ig`s only. Rebuilt only when
explicitly asked to (the panel's "reste" button, or re-checking an
already-loaded region) -- no mtime/size staleness check like
zone_geom_cache.py's own per-zone cache: a `.bam` here is trusted until
explicitly rebuilt.
"""

from pathlib import Path
from typing import Optional

from .config_dir import cache_dir

_CACHE_DIR_NAME = "ig_full_geom_cache"
# Not a real region name (region_hierarchy.py's own names are always
# "region_*"), safe as the "rest" bundle's own key component.
_REST_KEY = "__rest__"


def _bundle_key(continent: str, mode: str, region: Optional[str]) -> str:
	# Continent identifiers, render mode strings, and region names are all
	# already filesystem-safe, no sanitizing needed (same reasoning as
	# zone_geom_cache.py's own _bundle_key()).
	return f"{continent}__{mode}__{region or _REST_KEY}"


def _bam_path(continent: str, mode: str, region: Optional[str]) -> Path:
	return cache_dir() / _CACHE_DIR_NAME / f"{_bundle_key(continent, mode, region)}.bam"


def write_ig_bundle(continent: str, mode: str, region: Optional[str], node_path) -> None:
	"""Overwrites the saved `.ig` bundle for `(continent, mode, region)` with
	`node_path`'s current subtree -- the only bundle ever kept for this key,
	never accumulated. Written to a temp path then renamed into place, same
	reasoning as zone_cache.py's write_zone_cache(): a crash mid-write must
	never leave a half-written `.bam` for read_ig_bundle() to choke on."""
	cache_root = cache_dir() / _CACHE_DIR_NAME
	cache_root.mkdir(parents=True, exist_ok=True)

	bam_path = _bam_path(continent, mode, region)
	bam_tmp_path = bam_path.with_suffix(bam_path.suffix + ".tmp")
	node_path.write_bam_file(str(bam_tmp_path))
	bam_tmp_path.replace(bam_path)


def has_ig_bundle(continent: str, mode: str, region: Optional[str]) -> bool:
	"""True if a `.bam` was already built for `(continent, mode, region)` --
	cheap existence check, so a caller can skip a rebuild and go straight to
	read_ig_bundle()."""
	return _bam_path(continent, mode, region).is_file()


def read_ig_bundle(continent: str, mode: str, region: Optional[str], loader) -> Optional[object]:
	"""Loads the saved `(continent, mode, region)` bundle's `.bam` via
	`loader` (a Panda3D `Loader`, e.g. `ShowBase.loader`) -- None if missing
	or if the `.bam` fails to load (never raises: a broken cache must
	degrade to "nothing loaded yet", not crash the app). `noCache=True`
	bypasses Panda3D's own model pool, so a bundle overwritten earlier in
	the same run is never served stale from memory."""
	bam_path = _bam_path(continent, mode, region)
	if not bam_path.is_file():
		return None
	try:
		return loader.load_model(str(bam_path), noCache=True, okMissing=True)
	except Exception:
		return None
