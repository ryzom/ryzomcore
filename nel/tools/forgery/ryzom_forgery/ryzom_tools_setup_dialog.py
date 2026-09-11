"""Settings-tab UI for `settings.ryzom_tools_path` -- a folder containing the
native Ryzom pipeline tool executables (zone_welder today, more to come, see
settings.py's own field docstring). A standalone module, not a method on
object_editor_mixins.settings_dialogs.SettingsDialogsMixin, precisely so any
Forgery app can instantiate and draw it -- same reasoning as
live_data_setup_dialog.py's own module docstring. No mandatory first-launch
popup (unlike LiveDataSetupDialog): this path has no auto-detection and isn't
needed just to open an app, only to use a specific feature, same as
image_editor_path/text_editor_path.
"""

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, portable_file_dialogs as pfd

from ryzom_forgery import settings as app_settings
from ryzom_forgery.icon_colors import pastel_color_for
from ryzom_forgery.workspace_setup_dialog import _truncate_path_to_width


def _icon_button(icon, tooltip):
	imgui.push_style_color(imgui.Col_.text.value, pastel_color_for(icon))
	clicked = imgui.button(icon)
	imgui.pop_style_color()
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


class RyzomToolsSetupDialog:
	def __init__(self):
		self._folder_dialog = None  # active portable_file_dialogs.select_folder, or None

	def _poll_folder_dialog(self):
		if self._folder_dialog is None or not self._folder_dialog.ready(0):
			return
		result = self._folder_dialog.result()
		self._folder_dialog = None
		if result:
			fresh = app_settings.load()
			fresh.ryzom_tools_path = result
			app_settings.save(fresh)

	def draw_settings_content(self):
		"""Embedded in the host app's own Settings UI -- called from both
		Patina's (object_editor.py) and Atyscape's (landscape_editor.py) own
		Settings, since this is a generic suite-wide path, not specific to
		either app (see settings.py's ryzom_tools_path field docstring)."""
		self._poll_folder_dialog()
		settings = app_settings.load()
		label = "Ryzom tools folder: "
		path_text = settings.ryzom_tools_path or "(not set)"

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if settings.ryzom_tools_path and imgui.is_item_hovered():
			imgui.set_tooltip(settings.ryzom_tools_path)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##ryzom-tools", "Choose the Ryzom tools folder (zone_welder, ...)..."):
			self._folder_dialog = pfd.select_folder("Choose Ryzom tools folder", settings.ryzom_tools_path or "")
