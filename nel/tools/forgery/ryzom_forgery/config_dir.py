"""The standard per-user config directory for Ryzom Forgery tools -- shared
by every module that persists user preferences as JSON there (export
settings, explorer favorites...), so they all land next to each other.
"""

import os
import sys
from pathlib import Path


def config_dir() -> Path:
	"""No external lib, just the well-known env vars/paths for each OS --
	not the project directory, so preferences survive across checkouts/installs."""
	if sys.platform == "win32":
		base = os.environ.get("APPDATA", str(Path.home() / "AppData" / "Roaming"))
	elif sys.platform == "darwin":
		base = str(Path.home() / "Library" / "Application Support")
	else:
		base = os.environ.get("XDG_CONFIG_HOME", str(Path.home() / ".config"))
	return Path(base) / "ryzom_forgery"
