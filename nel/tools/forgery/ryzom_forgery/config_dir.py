"""The standard per-user config/cache directories for Ryzom Forgery tools --
shared by every module that persists something there, so they all land next
to each other (config_dir() for user preferences as JSON -- export settings,
explorer favorites... -- cache_dir() for disposable, regenerable data --
zone_cache.py/continent_geom_cache.py).
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


def cache_dir() -> Path:
	"""Same per-OS-convention reasoning as config_dir(), but the real CACHE
	location on each platform (Nuno 2026-09-12: "c'est tres moche... un
	dossier de cache dans le dossier de config" -- zone_cache.py/
	continent_geom_cache.py used to live under config_dir() itself). Purely
	disposable/regenerable data belongs here, never user preferences --
	deleting this whole directory must always be safe."""
	if sys.platform == "win32":
		base = os.environ.get("LOCALAPPDATA", str(Path.home() / "AppData" / "Local"))
	elif sys.platform == "darwin":
		base = str(Path.home() / "Library" / "Caches")
	else:
		base = os.environ.get("XDG_CACHE_HOME", str(Path.home() / ".cache"))
	return Path(base) / "ryzom_forgery"
