"""Single unified TOML settings file for every Ryzom Forgery tool app
preference (export flow, Explorer favorites, generic search paths) -- one
file at `config_dir()/"settings.toml"` instead of one JSON file per concern,
since a single, easy to read/edit-by-hand file is what actually matters here
over any of these being independently loadable. There's no separate "data
root": a search path with recursive on covers that same case, so resolving
everything (shapes' textures, .skel/.anim compatibility, panoply) through
this one priority-ordered list is the only mechanism -- see
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


def load() -> Settings:
	path = config_dir() / _SETTINGS_FILE_NAME
	try:
		data = tomlkit.parse(path.read_text())
	except (OSError, tomlkit.exceptions.TOMLKitError):
		return Settings()

	settings = Settings()
	settings.explorer_favorites = list(data.get("explorer_favorites", []))

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

	export_table = tomlkit.table()
	for key, value in asdict(settings.export).items():
		if value is not None:
			export_table[key] = value
	doc["export"] = export_table

	doc["search_paths"] = [asdict(entry) for entry in settings.search_paths]

	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _SETTINGS_FILE_NAME).write_text(tomlkit.dumps(doc))
