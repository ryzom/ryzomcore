"""Settings-tab "Ryzom Paths" section, grouping every generic suite-wide
path setting (not specific to a single Forgery app) under one header:
`live_data_path`, the `pynel.repository_paths` checkouts, and
`ryzom_tools_path`. A standalone module, drawable by any Forgery app's own
Settings UI (`draw(app)`), so a user never has to open a *different* app to
configure a setting their own app needs -- see [[feedback_forgery_apps_own_settings]]
(Patina and Atyscape are independent apps with disjoint audiences; each
edits the settings it consumes itself, but these particular settings are
generic infrastructure useful to any of them, same precedent as
`live_data_path` already being duplicated wherever needed).

Takes an existing `LiveDataSetupDialog` instance rather than creating its
own: the host app may need that instance for more than this section (e.g.
object_editor.py's own mandatory first-launch popup + `live_data_dir`
property, drawn/read independently of whether this section's header is
even open) -- see live_data_setup_dialog.py's own module docstring.
"""

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui

from ryzom_forgery.repository_paths_dialog import RepositoryPathsDialog
from ryzom_forgery.ryzom_tools_setup_dialog import RyzomToolsSetupDialog

_SECTION_NAME = "Ryzom Paths"


class RyzomPathsSection:
	def __init__(self, live_data_dialog):
		self.live_data_dialog = live_data_dialog
		self.repository_paths_dialog = RepositoryPathsDialog()
		self.ryzom_tools_dialog = RyzomToolsSetupDialog()

	def draw(self, app):
		"""Call from the host app's own Settings tab drawing code."""
		app._consume_settings_section_open(_SECTION_NAME)
		if imgui.collapsing_header(f"{fa_icons.ICON_FA_GLOBE} {_SECTION_NAME}"):
			self.live_data_dialog.draw_settings_content()
			imgui.separator()
			self.repository_paths_dialog.draw(app)
			imgui.separator()
			self.ryzom_tools_dialog.draw_settings_content()
