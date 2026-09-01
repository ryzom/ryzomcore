"""Workspace model: `settings.workspaces_root` is a single folder, shared
across every Forgery app, containing one subfolder per *project*, itself
containing one subfolder per editable *workspace* (see the
forgery-workspace-projects chantier). Each workspace subfolder holds
`SUBDIRS` -- the app never writes outside of the active workspace.

A workspace can also be *external*: a folder living anywhere else on disk,
merely referenced by a project's own external_workspaces.toml manifest
(list_external_workspaces()/add_external_workspace() below) rather than
being a real subfolder of the project. Nothing is ever copied/moved into
place for those -- see active_workspace_path()'s own docstring for how the
two cases are told apart at resolution time.
"""

import shutil
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Tuple

import tomlkit

# Every workspace gets these subfolders up front so tools can rely on them
# existing rather than mkdir-on-demand scattered across call sites. `dds`
# is deliberately not a top-level entry -- panoply-baked .dds output lives
# in build/dds/ instead (see _BUILD_SUBDIRS below), since it's derived/
# final output, not a source to browse alongside the others.
SUBDIRS = ("tex", "masks", "shapes", "anims", "skels", "imports", "exports", "build")
_BUILD_SUBDIRS = ("dds",)  # nested inside build/, not a top-level SUBDIRS entry

_EXTERNAL_WORKSPACES_FILE_NAME = "external_workspaces.toml"


def is_root_configured(root: Optional[str]) -> bool:
	"""True if `root` (settings.workspaces_root) points at an existing directory."""
	return bool(root) and Path(root).is_dir()


def list_projects(root: str) -> List[str]:
	"""Names of the project subfolders directly under `root`, sorted case-insensitively."""
	try:
		names = [entry.name for entry in Path(root).iterdir() if entry.is_dir()]
	except OSError:
		return []
	return sorted(names, key=str.lower)


def project_path(root: str, project: str) -> Path:
	return Path(root) / project


def create_project(root: str, project: str) -> Path:
	"""Creates the project subfolder under `root`. Idempotent. Unlike
	create_workspace() below, a project itself has no SUBDIRS of its own --
	those belong to each workspace inside it."""
	path = project_path(root, project)
	path.mkdir(parents=True, exist_ok=True)
	return path


def list_workspaces(root: str, project: str) -> List[str]:
	"""Names of the *internal* workspace subfolders directly under this
	project, sorted case-insensitively -- external workspaces (see
	list_external_workspaces()) are a separate list, not included here;
	callers that want "everything choosable" combine both themselves."""
	try:
		names = [entry.name for entry in project_path(root, project).iterdir() if entry.is_dir()]
	except OSError:
		return []
	return sorted(names, key=str.lower)


def workspace_path(root: str, project: str, name: str) -> Path:
	return project_path(root, project) / name


def create_workspace(root: str, project: str, name: str) -> Path:
	"""Creates the workspace subfolder (with its standard SUBDIRS) under
	`root`/`project`. Idempotent."""
	path = workspace_path(root, project, name)
	ensure_structure(path)
	return path


def ensure_structure(path: Path) -> None:
	path.mkdir(parents=True, exist_ok=True)
	for subdir in SUBDIRS:
		(path / subdir).mkdir(exist_ok=True)
	for build_subdir in _BUILD_SUBDIRS:
		(path / "build" / build_subdir).mkdir(exist_ok=True)


def is_inside(path: Path, workspace: Path) -> bool:
	"""True if `path` lives anywhere under `workspace` (resolved, so symlinks/`..` don't fool it)."""
	try:
		path.resolve().relative_to(workspace.resolve())
		return True
	except ValueError:
		return False


def _external_workspaces_file(project_dir: Path) -> Path:
	return project_dir / _EXTERNAL_WORKSPACES_FILE_NAME


def list_external_workspaces(project_dir: Path) -> List[Tuple[str, str]]:
	"""[(name, path), ...] registered in this project's own
	external_workspaces.toml, sorted case-insensitively by name -- [] if
	the manifest doesn't exist yet (a project with no external workspaces
	registered) or is invalid. Lives inside the project folder itself
	(not the global settings.toml) so it travels with the project if
	shared/moved."""
	try:
		data = tomlkit.parse(_external_workspaces_file(project_dir).read_text())
	except (OSError, tomlkit.exceptions.TOMLKitError):
		return []
	entries = data.get("external_workspaces", [])
	pairs = [
		(str(entry["name"]), str(entry["path"]))
		for entry in entries if isinstance(entry, dict) and "name" in entry and "path" in entry
	]
	return sorted(pairs, key=lambda pair: pair[0].lower())


def add_external_workspace(project_dir: Path, name: str, path: str) -> None:
	"""Registers an existing folder as an external workspace of this
	project -- appends to the manifest, never copies/moves `path` itself
	(see this module's own docstring)."""
	entries = list_external_workspaces(project_dir)
	entries.append((name, path))

	doc = tomlkit.document()
	doc.add(tomlkit.comment(
		"Ryzom Forgery external workspaces -- folders outside this project, referenced by path only."))
	doc["external_workspaces"] = [{"name": entry_name, "path": entry_path} for entry_name, entry_path in entries]

	project_dir.mkdir(parents=True, exist_ok=True)
	_external_workspaces_file(project_dir).write_text(tomlkit.dumps(doc))


def external_workspace_path(project_dir: Path, name: str) -> Optional[Path]:
	for entry_name, entry_path in list_external_workspaces(project_dir):
		if entry_name == name:
			return Path(entry_path)
	return None


def migrate_legacy_workspaces(root: str, project_name: str) -> Path:
	"""Moves every folder currently sitting directly under `root` (the
	pre-project layout -- one subfolder per workspace) into a newly
	created `project_name` project folder, preserving each one's own
	name -- not just the active one, all of them (see the
	forgery-workspace-projects chantier's design notes on why: an
	upgrading user's other, inactive workspaces would otherwise be
	silently orphaned outside the new project/workspace UI entirely).
	Called once, when Settings.active_project is still None despite
	workspaces_root/active_workspace already being set (see
	workspace_setup_dialog.py's migration flow).

	Skips (rather than overwrites) a legacy folder if a same-named folder
	already exists at the destination -- a real conflict there means
	something unexpected already happened; silently merging/overwriting
	real user data would be worse than leaving that one folder in place
	for the user to sort out by hand."""
	root_path = Path(root)
	project_dir = create_project(root, project_name)
	for entry in list(root_path.iterdir()):
		if not entry.is_dir() or entry == project_dir:
			continue
		destination = project_dir / entry.name
		if destination.exists():
			print(f"[workspaces] migration: skipping {entry} -- {destination} already exists")
			continue
		shutil.move(str(entry), str(destination))
	return project_dir


def active_workspace_path(settings) -> Optional[Path]:
	"""Resolves `settings.active_project`/`active_workspace` against
	`settings.workspaces_root`, or None if any of the three isn't set, or
	the workspace folder no longer exists on disk (e.g. renamed/deleted
	outside the app since it was last picked -- callers should fall back
	to "no active workspace" rather than error out).

	Checks the active project's external-workspace manifest first: an
	active_workspace name registered there resolves to that external
	path instead of the usual root/project/name internal location -- the
	two are mutually exclusive by construction (create_workspace() and
	add_external_workspace() are never called for the same name within
	one project), so whichever matches first is unambiguous."""
	if not is_root_configured(settings.workspaces_root) or not settings.active_project or not settings.active_workspace:
		return None
	project_dir = project_path(settings.workspaces_root, settings.active_project)
	external_path = external_workspace_path(project_dir, settings.active_workspace)
	if external_path is not None:
		return external_path if external_path.is_dir() else None
	path = workspace_path(settings.workspaces_root, settings.active_project, settings.active_workspace)
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


def reveal_in_system_file_manager(path: Path) -> None:
	"""Opens `path`'s *containing folder* in the OS's own file manager, with
	`path` itself selected/highlighted where the platform actually supports
	that -- Windows (`explorer /select,`) and macOS (`open -R`) both have an
	official mechanism for this; Linux has no cross-desktop-environment
	equivalent (a specific file manager like Nautilus/Dolphin does, but
	there's no universal command), so this falls back to
	open_in_system_file_manager() on the parent folder there instead (opens
	the folder, just doesn't select the file within it)."""
	if sys.platform == "win32":
		subprocess.Popen(["explorer", "/select,{}".format(path)])
	elif sys.platform == "darwin":
		subprocess.Popen(["open", "-R", str(path)])
	else:
		open_in_system_file_manager(path.parent)
