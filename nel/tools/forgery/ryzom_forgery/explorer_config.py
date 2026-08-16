"""Persisted user preferences for the Explorer: favorite folder paths,
stored as JSON in the same per-user config directory as export settings.
"""

import json
from dataclasses import asdict, dataclass, field
from typing import List

from .config_dir import config_dir

_CONFIG_FILE_NAME = "explorer_settings.json"


@dataclass
class ExplorerConfig:
	favorites: List[str] = field(default_factory=list)


def load() -> ExplorerConfig:
	path = config_dir() / _CONFIG_FILE_NAME
	try:
		data = json.loads(path.read_text())
	except (OSError, ValueError):
		return ExplorerConfig()

	config = ExplorerConfig()
	for field_name in config.__dataclass_fields__:
		if field_name in data:
			setattr(config, field_name, data[field_name])
	return config


def save(config: ExplorerConfig) -> None:
	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _CONFIG_FILE_NAME).write_text(json.dumps(asdict(config), indent="\t"))
