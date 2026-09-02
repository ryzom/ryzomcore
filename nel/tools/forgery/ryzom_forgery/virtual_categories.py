"""Classifies a workspace's files into fixed virtual categories (see the
forgery-workspace-projects chantier) purely by name -- extension for most,
an `_<axis>` suffix for Panoply masks -- regardless of which real subfolder
they actually live in on disk. Files aren't moved; this is a display/
lookup grouping only, consumed by explorer.py's virtual browsing mode and
by save/export target resolution (find_existing_file()).

dds is deliberately not a category of its own: it's derived/final output
(build/dds/, itself covered by the default "build" folder exclusion), not
a source to browse alongside the others -- any stray .dds file that isn't
under an excluded folder just lands in CATEGORY_OTHERS like anything else
unrecognized.
"""

import fnmatch
import os
from pathlib import Path
from typing import Dict, Iterator, List, Optional

from .settings import EXCLUSION_KIND_FILE, EXCLUSION_KIND_FOLDER

CATEGORY_SHAPES = "shapes"
CATEGORY_TEXTURES = "textures"
CATEGORY_3D_FILES = "3d files"
CATEGORY_MASKS = "masks"
CATEGORY_ANIMS = "anims"
CATEGORY_SKELS = "skels"
CATEGORY_OTHERS = "others"

CATEGORIES = (
	CATEGORY_SHAPES, CATEGORY_TEXTURES, CATEGORY_3D_FILES,
	CATEGORY_MASKS, CATEGORY_ANIMS, CATEGORY_SKELS, CATEGORY_OTHERS,
)

# dds excluded on purpose -- see module docstring.
_TEXTURE_EXTENSIONS = {".tga", ".png", ".jpg", ".jpeg", ".bmp"}
_3D_FILE_EXTENSIONS = {".obj", ".dae", ".fbx", ".gltf", ".glb"}
# Same 4 axes as panoply.py's AXES / panoply_bake.py's _CANDIDATE_AXES --
# object_editor.py's mask export already names files "<stem>_<axis>.tga"
# (e.g. _draw_mask_export's `f"{stem}_{axis}.tga"`), this just recognizes
# that same convention back.
_MASK_AXES = ("skin", "user", "hair", "eyes")


def _is_mask_name(stem: str) -> bool:
	lower = stem.lower()
	return any(lower.endswith(f"_{axis}") for axis in _MASK_AXES)


def categorize(name: str) -> str:
	"""The virtual category a file named `name` (just the filename, no
	directory) belongs in, purely from its own name. Doesn't know about
	exclusion rules -- scan_workspace() below is what forces a
	file-pattern-excluded file into CATEGORY_OTHERS regardless of this
	result (see ExclusionRule's own docstring in settings.py)."""
	path = Path(name)
	suffix = path.suffix.lower()
	if suffix == ".shape":
		return CATEGORY_SHAPES
	if suffix == ".anim":
		return CATEGORY_ANIMS
	if suffix == ".skel":
		return CATEGORY_SKELS
	if suffix in _3D_FILE_EXTENSIONS:
		return CATEGORY_3D_FILES
	if suffix in _TEXTURE_EXTENSIONS:
		return CATEGORY_MASKS if _is_mask_name(path.stem) else CATEGORY_TEXTURES
	return CATEGORY_OTHERS


def _is_folder_excluded(relative_dir: Path, exclusion_rules) -> bool:
	"""True if `relative_dir` (a workspace-relative path, e.g.
	Path("build") or Path("build/dds")) lives under any folder-kind
	exclusion rule -- a prefix match on path parts, so a rule for
	"build" also covers everything nested under it."""
	parts = relative_dir.parts
	for rule in exclusion_rules:
		if rule.kind != EXCLUSION_KIND_FOLDER:
			continue
		rule_parts = Path(rule.pattern).parts
		if parts[:len(rule_parts)] == rule_parts:
			return True
	return False


def _is_file_excluded(filename: str, exclusion_rules) -> bool:
	return any(
		rule.kind == EXCLUSION_KIND_FILE and fnmatch.fnmatch(filename.lower(), rule.pattern.lower())
		for rule in exclusion_rules
	)


def is_path_excluded(relative_dir: Path, filename: str, exclusion_rules) -> bool:
	"""True if `filename` (inside `relative_dir`, a workspace-relative
	directory) should be excluded from *search*/indexing -- unlike
	scan_workspace()'s display bucketing (where a file-kind exclusion
	still shows the file, just forced into CATEGORY_OTHERS), search
	indexing treats both kinds the same: never returned at all, per
	ExclusionRule's own docstring in settings.py ("never taken into
	account in search" for both kinds -- a folder-kind rule additionally
	hides it from display too, a file-kind rule doesn't). Used by
	search_paths_dialog.py to keep an excluded workspace file out of the
	texture/.skel/.anim index entirely."""
	return _is_folder_excluded(relative_dir, exclusion_rules) or _is_file_excluded(filename, exclusion_rules)


def _iter_included_files(workspace_root: Path, exclusion_rules) -> Iterator[Path]:
	"""Every file under `workspace_root`, at any real nesting depth,
	skipping whole subtrees that a folder-kind exclusion rule covers --
	pruned from os.walk's own dirnames list in place, so excluded folders
	are never even descended into, not just filtered out after the fact.
	File-kind exclusions are NOT applied here (a file-pattern-excluded
	file is still a real file that exists -- see scan_workspace()/
	find_existing_file() for how each of those two callers treats that
	differently)."""
	for dirpath, dirnames, filenames in os.walk(workspace_root):
		current_dir = Path(dirpath)
		dirnames[:] = [
			name for name in dirnames
			if not _is_folder_excluded((current_dir / name).relative_to(workspace_root), exclusion_rules)
		]
		for filename in filenames:
			yield current_dir / filename


def iter_included_files(workspace_root: Path, exclusion_rules) -> Iterator[Path]:
	"""Every file under `workspace_root` that isn't excluded at all, folder
	or file-kind alike -- unlike _iter_included_files() above (only prunes
	folder-kind, kept file-kind-excluded files visible for
	scan_workspace()/find_existing_file()'s own display/save-target
	purposes), this is for the workspace watcher's own extension-triggered
	scans (see workspace_watch.py, import_watcher.py, tex_dds_sync.py,
	workspace_sync.py): an excluded file must never be auto-imported/
	converted/synced, full stop."""
	for file_path in _iter_included_files(workspace_root, exclusion_rules):
		if not _is_file_excluded(file_path.name, exclusion_rules):
			yield file_path


def scan_workspace(workspace_root: Path, exclusion_rules) -> Dict[str, List[Path]]:
	"""Recursively scans `workspace_root`, bucketed by virtual category
	(see categorize()) -- regardless of which real subfolder a file
	actually lives in. A file matching a file-kind exclusion rule is
	still included, but forced into CATEGORY_OTHERS regardless of its
	natural category (see ExclusionRule's own docstring in settings.py);
	a folder-kind exclusion rule instead prunes that whole subtree, so
	nothing under it appears in any bucket at all."""
	buckets = {category: [] for category in CATEGORIES}
	for file_path in _iter_included_files(workspace_root, exclusion_rules):
		if _is_file_excluded(file_path.name, exclusion_rules):
			buckets[CATEGORY_OTHERS].append(file_path)
		else:
			buckets[categorize(file_path.name)].append(file_path)
	return buckets


def find_existing_file(workspace_root: Path, filename: str, exclusion_rules) -> Optional[Path]:
	"""Finds a file named exactly `filename` anywhere in `workspace_root`
	(any real nesting, folder exclusions respected -- never look for a
	save/export target inside an excluded folder), for save/export
	target resolution (see the forgery-workspace-projects chantier): a
	shape should overwrite wherever it already lives, not always some
	fixed canonical subfolder. None if no match exists anywhere (a
	genuinely new asset -- the caller falls back to the workspace's own
	canonical subfolder for that category in that case). If more than
	one real match exists (an actual duplicate name in two different
	real subfolders), the most recently modified one wins -- picked as
	the one more likely still in active use."""
	matches = [path for path in _iter_included_files(workspace_root, exclusion_rules) if path.name == filename]
	if not matches:
		return None
	return max(matches, key=lambda path: path.stat().st_mtime)
