#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Disk cache of ONE zone's already-built `NodePath` (project-todos/forgery/
landscape_editor__region_management__zone_bam_cache.md) -- replaces
`continent_geom_cache.py`'s whole-continent-bundle approach, which stopped
paying off once zones started loading by region (project-todos/forgery/
landscape_editor__region_management.md): that cache held ONE bundle per
`(continent, render mode)`, containing every currently-loaded zone fused
together, so checking a different region combination invalidated and
overwrote the ENTIRE bundle every single time -- a per-zone cache stays
valid for a zone no matter which other zones/regions are loaded alongside it.

Like `continent_geom_cache.py`, the actual built Panda3D scene graph is
serialized via the native `.bam` format (`NodePath.write_bam_file()`/
`Loader.load_model()`), so a cache hit skips `GeomNode` construction
entirely (couleur/indices/`attach_new_node`, ~8.9ms/zone measured on nexus).

`ZoneGeomManifest` records exactly which extension was resolved and the
source file's mtime/size (region_loader.py's own staleness convention),
plus the `(min_z, max_z)` the elevation gradient was baked against --
`region_loader.get_continent_z_range()`'s WHOLE-CONTINENT range, not the
currently-loaded subset's own (see that function's own docstring for why:
a per-checked-subset range would invalidate every zone's cache entry on
every single region toggle, defeating the whole point here). Comparing the
stored manifest to a freshly computed one is how the caller
(`landscape_editor.py`'s `_set_loaded_zones()`) decides whether a zone's
bundle is still valid, without ever loading its `.bam` payload just to check.
"""

import pickle
from pathlib import Path
from typing import NamedTuple, Optional, Tuple

from .config_dir import cache_dir

_CACHE_DIR_NAME = "zone_geom_cache"
# Bumped whenever ZoneGeomManifest's own shape changes -- any manifest
# written under an older version is treated as absent (forces a full rebuild
# + re-save, same as a first-ever load). Bumped to 2 (project-todos/forgery/
# landscape_editor__render_modes_removal.md perf fix, Nuno 2026-09-14) when
# bb_center/bb_half_size were added to ZoneGeomManifest.
_MANIFEST_FORMAT_VERSION = 2


class ZoneManifestEntry(NamedTuple):
	"""What one zone/land-piece contributed to a build -- the resolved
	extension (".zone"/".zonew"/".zonel", or ".land" for a `.land`+brick
	fallback piece -- never "which mode", that's a separate manifest field)
	and its source file's freshness stamp (region_loader.py's own
	convention: a loose file's own mtime/size, or the whole `.bnp`/`.bnpe`'s
	if packed; a `.land` fallback piece instead stamps its own BRICK file).
	`rot`/`flip` (`pynel.ryzom_land.ZoneUnit.rot`/`.flip`, always `0`/`0` for
	a real zone -- rotation is a `.land` placement concept, not something a
	real exported zone ever has) catch a brick re-rotated/re-flipped IN
	PLACE (same brick file, same mtime/size, different placement) -- a case
	the extension+mtime+size alone can't see (found while designing the
	per-zone cache below: the OLD whole-continent cache caught this instead
	via a separate, blunter "the whole `.land` file's own mtime changed"
	stamp, invalidating every cell at once whenever ANY cell's placement
	changed -- per-cell rot/flip is the precise per-cell equivalent)."""
	extension: str
	source_mtime: float
	source_size: int
	rot: int = 0
	flip: int = 0


class ZoneGeomManifest(NamedTuple):
	"""Everything needed to tell whether a saved `.bam` for one zone is still
	an exact match for what `_set_loaded_zones()` would build fresh right
	now -- compared field-by-field, never partially trusted.

	`bb_center`/`bb_half_size` (project-todos/forgery/landscape_editor__
	render_modes_removal.md perf fix, Nuno 2026-09-14) are NOT part of the
	staleness comparison itself (deterministic given the same `entry` --
	same source file content always tessellates to the same bounds) --
	they're carried here purely so a caller can read a zone's bounds
	straight off this cheap manifest (no `.bam` touched) when it already
	knows the geometry cache will hit, skipping the far more expensive
	`zone_cache.py` position-cache load (~3ms/zone measured, mostly pickle
	unpacking thousands of individual Python floats) entirely for that
	zone -- see `_run_load_refs()`'s own early-skip, `landscape_editor.py`."""
	format_version: int
	entry: ZoneManifestEntry
	min_z: float
	max_z: float
	bb_center: Tuple[float, float, float] = (0.0, 0.0, 0.0)
	bb_half_size: Tuple[float, float, float] = (0.0, 0.0, 0.0)


def _bundle_key(zone_name: str, mode: str) -> str:
	# Both zone names (region_loader.py's own `_ZONE_NAME_RE`, digits +
	# letters + underscore) and render mode strings are already
	# filesystem-safe, no sanitizing needed.
	return f"{zone_name}__{mode}"


def _bam_path(zone_name: str, mode: str) -> Path:
	return cache_dir() / _CACHE_DIR_NAME / f"{_bundle_key(zone_name, mode)}.bam"


def _manifest_path(zone_name: str, mode: str) -> Path:
	return cache_dir() / _CACHE_DIR_NAME / f"{_bundle_key(zone_name, mode)}.manifest"


def write_zone_bundle(zone_name: str, mode: str, node_path, manifest: ZoneGeomManifest) -> None:
	"""Overwrites the saved bundle for `(zone_name, mode)` with `node_path`'s
	current subtree + `manifest` -- the only bundle ever kept for this key,
	never accumulated. Both files are written to a temp path then renamed
	into place, same reasoning as `zone_cache.py`'s `write_zone_cache()`:
	a crash mid-write must never leave a half-written file for
	`read_zone_bundle()` to choke on next run."""
	cache_root = cache_dir() / _CACHE_DIR_NAME
	cache_root.mkdir(parents=True, exist_ok=True)

	bam_path = _bam_path(zone_name, mode)
	bam_tmp_path = bam_path.with_suffix(bam_path.suffix + ".tmp")
	node_path.write_bam_file(str(bam_tmp_path))
	bam_tmp_path.replace(bam_path)

	manifest_path = _manifest_path(zone_name, mode)
	manifest_tmp_path = manifest_path.with_suffix(manifest_path.suffix + ".tmp")
	with open(manifest_tmp_path, "wb") as f:
		pickle.dump(manifest, f, protocol=pickle.HIGHEST_PROTOCOL)
	manifest_tmp_path.replace(manifest_path)


def read_manifest(zone_name: str, mode: str) -> Optional[ZoneGeomManifest]:
	"""Returns the saved manifest for `(zone_name, mode)`, or None if absent/
	corrupt/an old format version -- cheap (no `.bam` touched), so the caller
	can decide a rebuild is needed without ever loading geometry it's about
	to discard."""
	path = _manifest_path(zone_name, mode)
	try:
		with open(path, "rb") as f:
			manifest = pickle.load(f)
	except (OSError, pickle.UnpicklingError, EOFError, AttributeError, ValueError):
		return None
	if not isinstance(manifest, ZoneGeomManifest) or manifest.format_version != _MANIFEST_FORMAT_VERSION:
		return None
	return manifest


def read_zone_bundle(zone_name: str, mode: str, loader) -> Optional[Tuple[object, ZoneGeomManifest]]:
	"""Loads the saved `(zone_name, mode)` bundle's `.bam` via `loader`
	(a Panda3D `Loader`, e.g. `ShowBase.loader`) and returns it alongside its
	manifest -- None if either is missing/corrupt, or if the `.bam` fails to
	load (never raises: a broken cache must degrade to a full rebuild, not
	crash the app). `noCache=True` bypasses Panda3D's own model pool, so a
	bundle overwritten earlier in the same run is never served stale from
	memory."""
	manifest = read_manifest(zone_name, mode)
	if manifest is None:
		return None
	bam_path = _bam_path(zone_name, mode)
	if not bam_path.is_file():
		return None
	try:
		node_path = loader.load_model(str(bam_path), noCache=True, okMissing=True)
	except Exception:
		return None
	if node_path is None:
		return None
	return node_path, manifest
