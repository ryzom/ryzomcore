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

"""Disk cache of a whole continent's already-built `NodePath` (project-todos/
forgery/geomnode_continent_cache.md) -- one step further than `zone_cache.py`
(which only caches tessellated positions, still requiring a fresh
`GeomNode` build -- couleur/indices/`attach_new_node`, ~8.9ms/zone measured
on nexus, 1.3s+ for a whole continent -- on every load). Here the actual
built Panda3D scene graph is serialized via the native `.bam` format
(`NodePath.write_bam_file()`/`Loader.load_model()`), so a cache hit skips
`GeomNode` construction entirely.

A zone only ever belongs to one continent, so one bundle per `(continent,
render mode)` is reused wholesale as long as nothing in it is stale --
`ContinentManifest` records, per zone, exactly which extension was resolved
and its source file's mtime/size (region_loader.py's own staleness
convention), plus the global `min_z`/`max_z` the elevation gradient was
baked against (project-todos/forgery/landscape_editor.md step 7: the
gradient spans the whole loaded set, not each zone's own range, so ANY
zone's range shifting invalidates every other zone's baked color too).
Comparing the stored manifest to a freshly computed one is how the caller
(`landscape_editor.py`'s `_set_loaded_zones()`) decides whether the bundle is
still valid outright, without ever loading `.bam` payloads just to check.
"""

import pickle
from pathlib import Path
from typing import Dict, NamedTuple, Optional, Tuple

from .config_dir import config_dir

_CACHE_DIR_NAME = "continent_geom_cache"
# Bumped whenever ContinentManifest's own shape changes -- any manifest
# written under an older version is treated as absent (forces a full rebuild
# + re-save, same as a first-ever load).
_MANIFEST_FORMAT_VERSION = 1


class ZoneManifestEntry(NamedTuple):
	"""What one zone contributed to a saved bundle -- the resolved extension
	(".zone"/".zonew"/".zonel", never "which mode" -- a mode is already part
	of the bundle's own file name) and its source file's freshness stamp
	(region_loader.py's own convention: a loose file's own mtime/size, or the
	whole `.bnp`/`.bnpe`'s if packed)."""
	extension: str
	source_mtime: float
	source_size: int


class ContinentManifest(NamedTuple):
	"""Everything needed to tell whether a saved `.bam` bundle is still an
	exact match for what `_set_loaded_zones()` would build fresh right now --
	compared field-by-field, never partially trusted."""
	format_version: int
	zones: Dict[str, ZoneManifestEntry]
	min_z: float
	max_z: float


def _bundle_key(continent: str, mode: str) -> str:
	# Both continent names and render mode strings are already filesystem-safe
	# (alnum/underscore/dash -- continent_selector.py/region_loader.py never
	# produce anything else), no sanitizing needed.
	return f"{continent}__{mode}"


def _bam_path(continent: str, mode: str) -> Path:
	return config_dir() / _CACHE_DIR_NAME / f"{_bundle_key(continent, mode)}.bam"


def _manifest_path(continent: str, mode: str) -> Path:
	return config_dir() / _CACHE_DIR_NAME / f"{_bundle_key(continent, mode)}.manifest"


def write_continent_bundle(continent: str, mode: str, node_path, manifest: ContinentManifest) -> None:
	"""Overwrites the saved bundle for `(continent, mode)` with `node_path`'s
	current subtree + `manifest` -- the only bundle ever kept for this key,
	never accumulated (project-todos/forgery/geomnode_continent_cache.md step
	1). Both files are written to a temp path then renamed into place, same
	reasoning as zone_cache.py's write_zone_cache(): a crash mid-write must
	never leave a half-written file for read_continent_bundle() to choke on
	next run."""
	cache_dir = config_dir() / _CACHE_DIR_NAME
	cache_dir.mkdir(parents=True, exist_ok=True)

	bam_path = _bam_path(continent, mode)
	bam_tmp_path = bam_path.with_suffix(bam_path.suffix + ".tmp")
	node_path.write_bam_file(str(bam_tmp_path))
	bam_tmp_path.replace(bam_path)

	manifest_path = _manifest_path(continent, mode)
	manifest_tmp_path = manifest_path.with_suffix(manifest_path.suffix + ".tmp")
	with open(manifest_tmp_path, "wb") as f:
		pickle.dump(manifest, f, protocol=pickle.HIGHEST_PROTOCOL)
	manifest_tmp_path.replace(manifest_path)


def read_manifest(continent: str, mode: str) -> Optional[ContinentManifest]:
	"""Returns the saved manifest for `(continent, mode)`, or None if absent/
	corrupt/an old format version -- cheap (no `.bam` touched), so the caller
	can decide a full rebuild is needed without ever loading geometry it's
	about to discard."""
	path = _manifest_path(continent, mode)
	try:
		with open(path, "rb") as f:
			manifest = pickle.load(f)
	except (OSError, pickle.UnpicklingError, EOFError, AttributeError, ValueError):
		return None
	if not isinstance(manifest, ContinentManifest) or manifest.format_version != _MANIFEST_FORMAT_VERSION:
		return None
	return manifest


def read_continent_bundle(continent: str, mode: str, loader) -> Optional[Tuple[object, ContinentManifest]]:
	"""Loads the saved `(continent, mode)` bundle's `.bam` via `loader`
	(a Panda3D `Loader`, e.g. `ShowBase.loader`) and returns it alongside its
	manifest -- None if either is missing/corrupt, or if the `.bam` fails to
	load (never raises: a broken cache must degrade to a full rebuild, not
	crash the app). `noCache=True` bypasses Panda3D's own model pool, so a
	bundle overwritten earlier in the same run is never served stale from
	memory."""
	manifest = read_manifest(continent, mode)
	if manifest is None:
		return None
	bam_path = _bam_path(continent, mode)
	if not bam_path.is_file():
		return None
	try:
		node_path = loader.load_model(str(bam_path), noCache=True, okMissing=True)
	except Exception:
		return None
	if node_path is None:
		return None
	return node_path, manifest
