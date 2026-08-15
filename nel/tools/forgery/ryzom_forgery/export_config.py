"""Persisted user preferences for .shape export (output folder, texture
handling), stored as JSON in the OS's standard per-user config directory --
not the project directory, so preferences survive across checkouts/installs.
"""

import json
import os
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Optional

TEXTURE_MODE_COPY_PNG = "copy_png"
TEXTURE_MODE_REFERENCE_ONLY = "reference_only"

_CONFIG_FILE_NAME = "export_settings.json"


def _config_dir() -> Path:
	"""The standard per-user config directory for the current OS -- no
	external lib, just the well-known env vars/paths for each platform."""
	if sys.platform == "win32":
		base = os.environ.get("APPDATA", str(Path.home() / "AppData" / "Roaming"))
	elif sys.platform == "darwin":
		base = str(Path.home() / "Library" / "Application Support")
	else:
		base = os.environ.get("XDG_CONFIG_HOME", str(Path.home() / ".config"))
	return Path(base) / "ryzom_forgery"


@dataclass
class ExportConfig:
	# None = "same folder as the source .shape", the default until the user
	# picks (and chooses to remember) an explicit folder.
	output_folder: Optional[str] = None
	remember_output_folder: bool = False
	texture_mode: str = TEXTURE_MODE_COPY_PNG
	remember_texture_mode: bool = False


def load() -> ExportConfig:
	path = _config_dir() / _CONFIG_FILE_NAME
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
	config_dir = _config_dir()
	config_dir.mkdir(parents=True, exist_ok=True)
	(config_dir / _CONFIG_FILE_NAME).write_text(json.dumps(asdict(config), indent="\t"))
