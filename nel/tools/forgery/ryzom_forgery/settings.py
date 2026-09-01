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
from typing import Dict, List, Optional

import tomlkit

from .config_dir import config_dir

TEXTURE_MODE_COPY_PNG = "copy_png"
TEXTURE_MODE_REFERENCE_ONLY = "reference_only"
TEXTURE_MODE_ZIP = "zip"

_SETTINGS_FILE_NAME = "settings.toml"


@dataclass
class SearchPathDir:
	path: str
	recursive: bool = False


@dataclass
class Settings:
	explorer_favorites: List[str] = field(default_factory=list)
	search_paths: List[SearchPathDir] = field(default_factory=list)
	# Root folder containing one subfolder per workspace (see workspaces.py)
	# -- shared across every Forgery app.
	workspaces_root: Optional[str] = None
	# Name of the active workspace (subfolder of workspaces_root), or None
	# if no workspace is currently active.
	active_workspace: Optional[str] = None
	# Per-workspace external mirror folder (see workspace_sync.py) -- keyed
	# by workspace name, same as active_workspace above. A workspace absent
	# from this dict has no sync folder configured (auto-mirroring is a
	# no-op for it). last_workspace_sync_folder is the most recently set
	# value across any workspace, used only to pre-fill a newly-created
	# workspace's own entry as a convenience default (see
	# workspace_setup_dialog.py's "Create" flow) -- never read back for an
	# existing workspace, which always uses its own dict entry instead.
	workspace_sync_folders: Dict[str, str] = field(default_factory=dict)
	last_workspace_sync_folder: Optional[str] = None
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
	# Same idea as image_editor_path, but for plain text files (e.g. VS Code,
	# Notepad++, gedit) -- used by the Panoply section's "Edit" button once a
	# workspace panoply.cfg already exists (see object_editor.py's
	# _draw_global_panoply_section()).
	text_editor_path: Optional[str] = None
	# UI text font -- key into app.py's _AVAILABLE_FONTS, and its size in
	# points. Applied once at startup (ForgeryApp.__init__ builds the font
	# atlas before the first frame) -- a change here only takes effect after
	# restarting the app, no live atlas rebuild.
	ui_font_name: str = "Roboto Bold"
	ui_font_size: float = 14.0
	# Manual DPI multiplier, set from the first-launch setup popup (see
	# workspace_setup_dialog.py) or Settings > UI later -- combined with the
	# auto-detected _dpi_scale() (app.py, RYZOM_FORGERY_DPI_SCALE env var
	# from ryztart) via app.py's _effective_ui_scale(), rather than
	# replacing it: ryztart's own detection stays the base, this is the
	# user's manual fine-tune on top. Same "next launch only" caveat as
	# ui_font_size/ui_font_name above. Defaults to 2.0 rather than 1.0 (no
	# scaling) -- picked as a starting point that reads reasonably on both
	# FullHD and 4K displays, since ryztart's own auto-detection isn't
	# always available/correct (e.g. launched directly via dev.sh).
	dpi_scale: float = 2.0


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
	settings.workspace_sync_folders = {
		str(name): str(path) for name, path in data.get("workspace_sync_folders", {}).items()
	}
	settings.last_workspace_sync_folder = data.get("last_workspace_sync_folder") or None
	settings.last_folder = data.get("last_folder") or None
	settings.last_bnp = data.get("last_bnp") or None
	settings.last_shape_path = data.get("last_shape_path") or None
	settings.last_shape_bnp = data.get("last_shape_bnp") or None
	settings.last_shape_name = data.get("last_shape_name") or None
	settings.image_editor_path = data.get("image_editor_path") or None
	settings.text_editor_path = data.get("text_editor_path") or None
	settings.ui_font_name = data.get("ui_font_name") or settings.ui_font_name
	settings.ui_font_size = data.get("ui_font_size") or settings.ui_font_size
	settings.dpi_scale = data.get("dpi_scale") or settings.dpi_scale

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
	doc["workspace_sync_folders"] = dict(settings.workspace_sync_folders)
	if settings.last_workspace_sync_folder is not None:
		doc["last_workspace_sync_folder"] = settings.last_workspace_sync_folder
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
	if settings.text_editor_path is not None:
		doc["text_editor_path"] = settings.text_editor_path
	doc["ui_font_name"] = settings.ui_font_name
	doc["ui_font_size"] = settings.ui_font_size
	doc["dpi_scale"] = settings.dpi_scale

	doc["search_paths"] = [asdict(entry) for entry in settings.search_paths]

	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _SETTINGS_FILE_NAME).write_text(tomlkit.dumps(doc))
