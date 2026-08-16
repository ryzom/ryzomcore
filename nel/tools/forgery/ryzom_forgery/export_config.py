"""Persisted user preferences for .shape export (output folder, texture
handling), stored as JSON in the OS's standard per-user config directory --
not the project directory, so preferences survive across checkouts/installs.
"""

import json
from dataclasses import asdict, dataclass
from typing import Optional

from .config_dir import config_dir

TEXTURE_MODE_COPY_PNG = "copy_png"
TEXTURE_MODE_REFERENCE_ONLY = "reference_only"

_CONFIG_FILE_NAME = "export_settings.json"


@dataclass
class ExportConfig:
	# None = "same folder as the source .shape", the default until the user
	# picks (and chooses to remember) an explicit folder.
	output_folder: Optional[str] = None
	remember_output_folder: bool = False
	texture_mode: str = TEXTURE_MODE_COPY_PNG
	remember_texture_mode: bool = False


def load() -> ExportConfig:
	path = config_dir() / _CONFIG_FILE_NAME
	try:
		data = json.loads(path.read_text())
	except (OSError, ValueError):
		return ExportConfig()

	config = ExportConfig()
	for field_name in config.__dataclass_fields__:
		if field_name in data:
			setattr(config, field_name, data[field_name])
	return config


def save(config: ExportConfig) -> None:
	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _CONFIG_FILE_NAME).write_text(json.dumps(asdict(config), indent="\t"))
