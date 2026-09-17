"""Runtime discovery of Ryzom Forgery's GUI apps, used by ryztart to build its
Forgery app list without needing to know about this package's internal
module layout.
"""

import importlib
import pkgutil

from ryzom_forgery import apps as _apps_package

# Each ryzom_forgery.apps module that is a launchable GUI app exposes an
# APP_INFO dict ({"id", "name", "description"}) and a main() entry point.
# Modules without APP_INFO (e.g. shape_exporter/shape_importer, which are
# command-line tools, not GUI apps) are skipped.


def list_apps():
	found = []
	for module_info in pkgutil.iter_modules(_apps_package.__path__):
		module = importlib.import_module(f"ryzom_forgery.apps.{module_info.name}")
		app_info = getattr(module, "APP_INFO", None)
		if app_info is not None:
			found.append(dict(app_info))
	return found


def launch_app(app_id):
	for module_info in pkgutil.iter_modules(_apps_package.__path__):
		module = importlib.import_module(f"ryzom_forgery.apps.{module_info.name}")
		app_info = getattr(module, "APP_INFO", None)
		if app_info is not None and app_info["id"] == app_id:
			module.main()
			return
	raise ValueError(f"Unknown Ryzom Forgery app id: {app_id!r}")
