"""Single unified TOML settings file for every Ryzom Forgery tool app
preference (export flow, Explorer favorites, generic search paths,
workspaces) -- one file at `config_dir()/"settings.toml"` instead of one
JSON file per concern, since a single, easy to read/edit-by-hand file is
what actually matters here over any of these being independently loadable.

`workspaces_root` is the one actual "data root" concept: a single folder,
shared across every Forgery app, containing one subfolder per editable
workspace (see workspaces.py). It is unrelated to `search_paths`, which
stays a read-only, priority-ordered list of extra folders to resolve assets
from (shapes' textures, .skel/.anim compatibility, panoply) -- see
search_paths_dialog.py.
"""

from dataclasses import asdict, dataclass, field
from typing import List, Optional

import tomlkit

from .config_dir import config_dir

TEXTURE_MODE_COPY_PNG = "copy_png"
TEXTURE_MODE_REFERENCE_ONLY = "reference_only"

_SETTINGS_FILE_NAME = "settings.toml"


@dataclass
class ExportSettings:
	# None = "same folder as the source .shape", the default until the user
	# picks (and chooses to remember) an explicit folder.
	output_folder: Optional[str] = None
	remember_output_folder: bool = False
	texture_mode: str = TEXTURE_MODE_COPY_PNG
	remember_texture_mode: bool = False


@dataclass
class SearchPathDir:
	path: str
	recursive: bool = False


@dataclass
class Settings:
	explorer_favorites: List[str] = field(default_factory=list)
	export: ExportSettings = field(default_factory=ExportSettings)
	search_paths: List[SearchPathDir] = field(default_factory=list)
	# Root folder containing one subfolder per workspace (see workspaces.py)
	# -- shared across every Forgery app, unlike export.output_folder.
	workspaces_root: Optional[str] = None
	# Name of the active workspace (subfolder of workspaces_root), or None
	# if no workspace is currently active.
	active_workspace: Optional[str] = None
	# Restore-on-launch session state (see object_editor.py's
	# _save_session_state()/_restore_session_state()) -- where the Explorer
	# was browsing and which shape was loaded, so relaunching the app looks
	# like it was never closed. last_shape_bnp/last_bnp are set only when
	# the shape/folder being browsed lives inside a .bnp archive (a .bnp
	# has no real sub-folder concept of its own, so browsing "inside" one
	# is really still browsing last_folder, just displaying that archive's
	# contents -- see explorer.py's _current_bnp).
	last_folder: Optional[str] = None
	last_bnp: Optional[str] = None
	last_shape_path: Optional[str] = None
	last_shape_bnp: Optional[str] = None
	last_shape_name: Optional[str] = None
	# Path to an external image editor executable (e.g. GIMP/Krita/Photoshop),
	# used by the Textures tab's "Edit" button (see object_editor.py) once a
	# texture already lives in the active workspace. None until the user
	# picks one in Settings -- the button stays disabled until then.
	image_editor_path: Optional[str] = None


def load() -> Settings:
	path = config_dir() / _SETTINGS_FILE_NAME
	try:
		data = tomlkit.parse(path.read_text())
	except (OSError, tomlkit.exceptions.TOMLKitError):
		return Settings()

	settings = Settings()
	settings.explorer_favorites = list(data.get("explorer_favorites", []))
	settings.workspaces_root = data.get("workspaces_root") or None
	settings.active_workspace = data.get("active_workspace") or None
	settings.last_folder = data.get("last_folder") or None
	settings.last_bnp = data.get("last_bnp") or None
	settings.last_shape_path = data.get("last_shape_path") or None
	settings.last_shape_bnp = data.get("last_shape_bnp") or None
	settings.last_shape_name = data.get("last_shape_name") or None
	settings.image_editor_path = data.get("image_editor_path") or None

	export_data = data.get("export", {})
	for field_name in settings.export.__dataclass_fields__:
		if field_name in export_data:
			setattr(settings.export, field_name, export_data[field_name])

	settings.search_paths = [
		SearchPathDir(path=entry["path"], recursive=bool(entry.get("recursive", False)))
		for entry in data.get("search_paths", []) if isinstance(entry, dict) and "path" in entry
	]
	return settings


def save(settings: Settings) -> None:
	doc = tomlkit.document()
	doc.add(tomlkit.comment("Ryzom Forgery settings -- safe to edit by hand."))

	doc["explorer_favorites"] = settings.explorer_favorites

	if settings.workspaces_root is not None:
		doc["workspaces_root"] = settings.workspaces_root
	if settings.active_workspace is not None:
		doc["active_workspace"] = settings.active_workspace
	if settings.last_folder is not None:
		doc["last_folder"] = settings.last_folder
	if settings.last_bnp is not None:
		doc["last_bnp"] = settings.last_bnp
	if settings.last_shape_path is not None:
		doc["last_shape_path"] = settings.last_shape_path
	if settings.last_shape_bnp is not None:
		doc["last_shape_bnp"] = settings.last_shape_bnp
	if settings.last_shape_name is not None:
		doc["last_shape_name"] = settings.last_shape_name
	if settings.image_editor_path is not None:
		doc["image_editor_path"] = settings.image_editor_path

	export_table = tomlkit.table()
	for key, value in asdict(settings.export).items():
		if value is not None:
			export_table[key] = value
	doc["export"] = export_table

	doc["search_paths"] = [asdict(entry) for entry in settings.search_paths]

	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _SETTINGS_FILE_NAME).write_text(tomlkit.dumps(doc))
