"""The standard per-user CACHE directory for Ryzom Forgery tools -- distinct
from config_dir() (settings a user explicitly chose, worth keeping forever)
because this one only holds data that's cheap to regenerate (parsed .skel/
.anim scan results) and that an OS/user is allowed to purge at any time
without anything breaking, just re-scanned a bit slower next time.
"""

import os
import sys
from pathlib import Path


def cache_dir() -> Path:
	"""No external lib, just the well-known env vars/paths for each OS."""
	if sys.platform == "win32":
		base = os.environ.get("LOCALAPPDATA", str(Path.home() / "AppData" / "Local"))
	elif sys.platform == "darwin":
		base = str(Path.home() / "Library" / "Caches")
	else:
		base = os.environ.get("XDG_CACHE_HOME", str(Path.home() / ".cache"))
	return Path(base) / "ryzom_forgery"
