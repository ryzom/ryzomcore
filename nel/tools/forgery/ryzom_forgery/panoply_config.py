"""Lookup for Panoply's real color-shift parameters -- reads panoply.cfg (the
production skin/user/hair/eyes color tables, consolidated into one bundled
file so Patina never needs a user to pick/craft a panoply_*.cfg -- see
/repos/project-todos/ryzom-core/panoply-runtime-tint.md "Unified panoply.cfg
+ Patina integration" for the full writeup, and that bundled file's own
header for the exact schema). No I/O beyond reading that one file (or, if
set_workspace_dir() points at an active workspace containing its own
panoply.cfg, that one instead -- see _resolve_cfg_path()), so this module --
and whatever calls it -- doesn't depend on the user's search paths covering
ryzom-data's leveldesign/ tree.

Feeds panoply_colorize.py's convert_bitmap() (live-preview path, via
object_editor.py) and panoply_bake.py's axes_for_source() (real-bake path,
via apps/panoply_maker.py and Patina's own "bake" action) alike --
hue/lightness/saturation/luminosity/contrast map directly onto that
function's matching arguments.
"""

from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

from pynel.config_file import Document

_BUNDLED_CFG_PATH = Path(__file__).parent / "panoply.cfg"
_WORKSPACE_CFG_NAME = "panoply.cfg"

# 2-letter race prefix, as seen at the start of a base texture's file name
# (e.g. "tr_hom_armor00_epaule_c1.tga") -> the race key prefix used by
# panoply.cfg's "<race>_hair_*"/"<race>_eyes_*" variables.
RACE_PREFIX_TO_TABLE = {"fy": "fyros", "ma": "matis", "tr": "tryker", "zo": "zorai", "ge": "generique"}

# Axes common to every race (unprefixed keys, "skin_*"/"user_*") vs. ones
# that need a race prefix resolved first ("<race>_hair_*"/"<race>_eyes_*") --
# matches panoply.AXES.
_COMMON_AXES = ("skin", "user")
_RACE_AXES = ("hair", "eyes")

_workspace_dir: Optional[Path] = None
_cache_path: Optional[Path] = None
_cache_mtime: Optional[float] = None
_cache_doc: Optional[Document] = None


@dataclass(frozen=True)
class ColorParams:
	"""One axis/color_id's target parameters -- feeds directly into
	panoply_colorize.convert_bitmap()'s hue/lightness/saturation/
	luminosity/contrast arguments."""

	id: str
	hue: float
	lightness: float
	saturation: float
	luminosity: float
	contrast: float


def set_workspace_dir(path) -> None:
	"""Called by object_editor.py's _on_active_workspace_changed() (same
	pattern as search_paths_dialog.set_workspace_dir()) whenever Patina's
	active workspace changes -- a `panoply.cfg` directly at the root of that
	workspace, if present, overrides the bundled default entirely (see
	_resolve_cfg_path()). Pass None (e.g. no workspace configured yet) to
	fall back to the bundled default only. Doesn't reload anything by
	itself -- the next lookup re-resolves and reloads lazily if the
	effective path or its mtime actually changed."""
	global _workspace_dir
	_workspace_dir = Path(path) if path is not None else None


def _resolve_cfg_path() -> Path:
	if _workspace_dir is not None:
		candidate = _workspace_dir / _WORKSPACE_CFG_NAME
		if candidate.is_file():
			return candidate
	return _BUNDLED_CFG_PATH


def workspace_cfg_path(workspace_dir) -> Path:
	"""Where a workspace's own panoply.cfg override would live, if it
	existed -- used by object_editor.py's "copy panoply.cfg to workspace"
	button to know its destination."""
	return Path(workspace_dir) / _WORKSPACE_CFG_NAME


def bundled_cfg_path() -> Path:
	"""The bundled default panoply.cfg's own path -- the source the "copy to
	workspace" button copies from."""
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


def _prefix_for(axis: str, race: Optional[str]) -> Optional[str]:
	if axis in _COMMON_AXES:
		return axis
	if axis not in _RACE_AXES or race is None:
		return None
	return f"{race}_{axis}"


def _entries_for(axis: str, race: Optional[str]) -> Optional[List[ColorParams]]:
	prefix = _prefix_for(axis, race)
	if prefix is None:
		return None
	doc = _load_doc()
	try:
		hues = doc.get(f"{prefix}_hues")
	except KeyError:
		return None
	lightness = doc.get(f"{prefix}_lightness")
	saturations = doc.get(f"{prefix}_saturations")
	luminosities = doc.get(f"{prefix}_luminosities")
	contrasts = doc.get(f"{prefix}_constrasts")
	color_ids = doc.get(f"{prefix}_color_id")
	return [
		ColorParams(
			id=str(color_ids[i]), hue=float(hues[i]), lightness=float(lightness[i]),
			saturation=float(saturations[i]), luminosity=float(luminosities[i]), contrast=float(contrasts[i]),
		)
		for i in range(len(color_ids))
	]


def get_color_params(axis: str, color_id: str, race: Optional[str] = None) -> Optional[ColorParams]:
	"""race must be one of RACE_PREFIX_TO_TABLE's values (e.g. "fyros"),
	required for "hair"/"eyes", ignored for "skin"/"user". Returns None if
	this axis doesn't apply to that race at all (zorai has no "eyes",
	"generique" has neither "hair" nor "eyes") or color_id isn't found."""
	entries = _entries_for(axis, race)
	if entries is None:
		return None
	for entry in entries:
		if entry.id == color_id:
			return entry
	return None


def available_color_ids(axis: str, race: Optional[str] = None) -> List[str]:
	"""Every color_id defined for this axis, optionally scoped to one
	race's hair/eyes table -- e.g. to build a picker UI. Empty if this axis
	doesn't apply to that race."""
	entries = _entries_for(axis, race)
	if entries is None:
		return []
	return [entry.id for entry in entries]
