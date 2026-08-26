"""Workspace model: `settings.workspaces_root` is a single folder, shared
across every Forgery app, containing one subfolder per editable workspace.
Each workspace subfolder holds `SUBDIRS` -- the app never writes outside of
the active workspace (see the "Workspaces" chantier in
`.todo/forgery-object-editor.md`).
"""

import subprocess
import sys
from pathlib import Path
from typing import List, Optional

# Every workspace gets these subfolders up front so tools can rely on them
# existing rather than mkdir-on-demand scattered across call sites.
SUBDIRS = ("tex", "shapes", "anims", "skels", "exports", "imports")


def is_root_configured(root: Optional[str]) -> bool:
	"""True if `root` (settings.workspaces_root) points at an existing directory."""
	return bool(root) and Path(root).is_dir()


def list_workspaces(root: str) -> List[str]:
	"""Names of the workspace subfolders directly under `root`, sorted case-insensitively."""
	try:
		names = [entry.name for entry in Path(root).iterdir() if entry.is_dir()]
	except OSError:
		return []
	return sorted(names, key=str.lower)


def workspace_path(root: str, name: str) -> Path:
	return Path(root) / name


def create_workspace(root: str, name: str) -> Path:
	"""Creates the workspace subfolder (with its standard SUBDIRS) under `root`. Idempotent."""
	path = workspace_path(root, name)
	ensure_structure(path)
	return path


def ensure_structure(path: Path) -> None:
	path.mkdir(parents=True, exist_ok=True)
	for subdir in SUBDIRS:
		(path / subdir).mkdir(exist_ok=True)


def is_inside(path: Path, workspace: Path) -> bool:
	"""True if `path` lives anywhere under `workspace` (resolved, so symlinks/`..` don't fool it)."""
	try:
		path.resolve().relative_to(workspace.resolve())
		return True
	except ValueError:
		return False


def active_workspace_path(settings) -> Optional[Path]:
	"""Resolves `settings.active_workspace` against `settings.workspaces_root`, or None if
	either isn't set, or the workspace folder no longer exists on disk (e.g. renamed/deleted
	outside the app since it was last picked -- callers should fall back to "no active
	workspace" rather than error out)."""
	if not is_root_configured(settings.workspaces_root) or not settings.active_workspace:
		return None
	path = workspace_path(settings.workspaces_root, settings.active_workspace)
	return path if path.is_dir() else None


def open_in_system_file_manager(path: Path) -> None:
	"""Opens `path` in the OS's own file manager -- the well-known per-platform
	mechanism for each (no xdotool/X11-only hacks, see project_forgery_cross_platform)."""
	if sys.platform == "win32":
		import os
		os.startfile(str(path))  # noqa: S606 -- Windows-only API, not a shell injection risk (no shell involved)
	elif sys.platform == "darwin":
		subprocess.Popen(["open", str(path)])
	else:
		subprocess.Popen(["xdg-open", str(path)])
