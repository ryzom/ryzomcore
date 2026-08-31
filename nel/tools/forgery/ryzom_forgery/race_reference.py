"""Race/gender reference data (own face mesh + a reference full-skull-coverage
hairstyle) for hairstyle_conform.py's cross-race boundary snap -- see
shape_geometry.py's build_face_vertex_index()/main_seam_loop()/
seam_ring_by_angle() for what these feed into.

Same "bundled default + optional workspace override" pattern as
panoply_config.py for the config file itself (race_reference.cfg, a plain
NeL CConfigFile: `<race_key>_face = "name.shape";` /
`<race_key>_reference_hairstyle = "name.shape";` per race/gender key, e.g.
"fy_hof"). The two named .shape files aren't bundled (real game data, not
shipped with Forgery) -- resolved by name through the caller's own
search-path index (ryzom_forgery.search_paths, same .bnp-aware lookup every
other Forgery tool uses for textures/skeletons -- see
shape_exporter.py/hairstyle_conform.py for how a CLI builds one from
--search-path).

Everything derived from those two files (face_index, seam_ring) is cached
in memory per race_key, keyed off the config file's own (path, mtime) AND
each resolved FoundEntry's own cache_stat() (mtime, size -- the .bnp archive
's own stat for an entry inside one, see FoundEntry.cache_stat()) -- so
processing many hairstyles for the same race across one process only pays
the parsing/index-building cost once, and picks up a re-imported/edited
face or reference hairstyle automatically without a stale cache.
"""

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Optional, Tuple

from pynel.config_file import Document
from pynel.ryzom_shape import parse_shape

from .search_paths import find_texture
from .shape_geometry import build_face_vertex_index, iter_render_passes, seam_ring_by_angle

_BUNDLED_CFG_PATH = Path(__file__).parent / "race_reference.cfg"
_WORKSPACE_CFG_NAME = "race_reference.cfg"

_workspace_dir: Optional[Path] = None
_cache_path: Optional[Path] = None
_cache_mtime: Optional[float] = None
_cache_doc: Optional[Document] = None

_reference_cache: Dict[str, Tuple[tuple, "RaceReference"]] = {}


@dataclass
class RaceReference:
	"""Distilled reference data for one race/gender key -- face_index (see
	build_face_vertex_index()) to identify a hairstyle's real seam loop when
	this race/gender is the *source* of a conform, and seam_ring (see
	seam_ring_by_angle()) to interpolate a new boundary against when it's
	the *target*."""
	face_index: dict
	seam_ring: list


def set_workspace_dir(path) -> None:
	"""Same pattern as panoply_config.set_workspace_dir(): a
	race_reference.cfg directly at the root of the active workspace, if
	present, overrides the bundled default entirely. Doesn't invalidate
	anything by itself -- the next get_reference() call re-resolves and
	reloads lazily if the effective path or its mtime actually changed."""
	global _workspace_dir
	_workspace_dir = Path(path) if path is not None else None


def _resolve_cfg_path() -> Path:
	if _workspace_dir is not None:
		candidate = _workspace_dir / _WORKSPACE_CFG_NAME
		if candidate.is_file():
			return candidate
	return _BUNDLED_CFG_PATH


def workspace_cfg_path(workspace_dir) -> Path:
	"""Where a workspace's own race_reference.cfg override would live, if it
	existed -- destination for a "copy to workspace" button, same as
	panoply_config.workspace_cfg_path()."""
	return Path(workspace_dir) / _WORKSPACE_CFG_NAME


def bundled_cfg_path() -> Path:
	return _BUNDLED_CFG_PATH


def _load_doc() -> Document:
	global _cache_path, _cache_mtime, _cache_doc
	path = _resolve_cfg_path()
	try:
		mtime = path.stat().st_mtime
	except OSError:
		mtime = None
	if _cache_doc is None or _cache_path != path or _cache_mtime != mtime:
		_cache_doc = Document.load(path)
		_cache_path = path
		_cache_mtime = mtime
	return _cache_doc


def _load_positions_indices(data: bytes):
	positions = []
	indices = []
	base = 0
	shape_file = parse_shape(data)
	for vertex_buffer, _material_id, idx in iter_render_passes(shape_file.value):
		positions.extend(vertex_buffer.positions)
		indices.extend(i + base for i in idx)
		base += len(vertex_buffer.positions)
	return positions, indices


def get_reference(race_key: str, entries_by_lower_name: dict) -> RaceReference:
	"""Returns the cached RaceReference for `race_key` (e.g. "fy_hof"),
	rebuilding it only if the config file or either resolved .shape file
	changed since the last call -- see module docstring. `entries_by_lower_name`
	is a search_paths.build_texture_index() result (or
	SearchPathsDialog's own live-scanned equivalent), used to resolve the
	config's face/reference_hairstyle file names to actual bytes."""
	doc = _load_doc()
	face_name = doc.get_str(f"{race_key}_face")
	hairstyle_name = doc.get_str(f"{race_key}_reference_hairstyle")

	face_entry = find_texture(entries_by_lower_name, face_name)
	if face_entry is None:
		raise FileNotFoundError(f"{race_key}_face: {face_name!r} not found in the configured search paths")
	hairstyle_entry = find_texture(entries_by_lower_name, hairstyle_name)
	if hairstyle_entry is None:
		raise FileNotFoundError(
			f"{race_key}_reference_hairstyle: {hairstyle_name!r} not found in the configured search paths")

	cache_key = (doc.path, _cache_mtime, face_entry.cache_key(), face_entry.cache_stat(),
		hairstyle_entry.cache_key(), hairstyle_entry.cache_stat())
	cached = _reference_cache.get(race_key)
	if cached is not None and cached[0] == cache_key:
		return cached[1]

	face_positions, _face_indices = _load_positions_indices(face_entry.read_bytes())
	face_index = build_face_vertex_index(face_positions)

	hair_positions, hair_indices = _load_positions_indices(hairstyle_entry.read_bytes())
	seam_ring = seam_ring_by_angle(hair_positions, hair_indices, face_index)

	reference = RaceReference(face_index=face_index, seam_ring=seam_ring)
	_reference_cache[race_key] = (cache_key, reference)
	return reference
