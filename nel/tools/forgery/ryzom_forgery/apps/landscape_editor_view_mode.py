"""LandscapeEditorApp mixin: everything specific to Visualisation mode
(project-todos/forgery/landscape_editor__land_preview.md step 5) -- as thin
as it is because Visualisation has no logic of its own beyond reading the
continent list from `live_data_path` (no `.land` fallback, no WELD
generation: `ryzom_data_path` is always `None` outside Edition mode, see
`_load_continent()`, `landscape_editor.py`).
"""

from pathlib import Path

from ryzom_forgery import continent_selector
from ryzom_forgery.error_log import report_error
from ryzom_forgery import live_data
from ryzom_forgery import settings as app_settings


class ViewModeMixin:
	def _load_visualisation_continent_locations(self):
		live_data_path = app_settings.load().live_data_path
		if not live_data.is_valid_live_data_path(live_data_path):
			self._cont_locs_error = (
				"Ryzom Live data path not configured -- set it in Patina's "
				"Settings tab (Paths), it's a shared setting."
			)
			report_error(self._cont_locs_error)
			return
		world_packed_sheets_path = Path(live_data_path) / "world.packed_sheets"
		print(f"(IA_AGENT_DEBUG) (landscape_editor) (landscape_editor_view_mode.py:_load_visualisation_continent_locations) "
		      f"reading {world_packed_sheets_path}")
		try:
			self._cont_locs = continent_selector.load_continent_locations(live_data_path)
		except (OSError, continent_selector.ContinentSelectorError) as exc:
			self._cont_locs_error = f"Failed to load continent list: {exc}"
			report_error(self._cont_locs_error)
