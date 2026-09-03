"""First-launch (and Settings-tab) UI for `settings.live_data_path` -- see
live_data.py's own module docstring for what this path is and how
auto-detection works. Mirrors workspace_setup_dialog.py's mandatory-popup
pattern, but simpler (a single form, no migration case): silently
auto-filled when `ryzom.ini` resolves a valid path, blocking popup only when
it doesn't (no `ryzom.ini` found, no [Atys] section, or a stale path).
"""

from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, portable_file_dialogs as pfd

from ryzom_forgery import live_data
from ryzom_forgery import settings as app_settings
from ryzom_forgery.icon_colors import pastel_color_for
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.workspace_setup_dialog import _truncate_path_to_width

_POPUP_ID = "Ryzom Live game data"


def _icon_button(icon, tooltip):
	imgui.push_style_color(imgui.Col_.text.value, pastel_color_for(icon))
	clicked = imgui.button(icon)
	imgui.pop_style_color()
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


class LiveDataSetupDialog:
	def __init__(self):
		self._settings = app_settings.load()
		self._folder_dialog = None  # active portable_file_dialogs.select_folder, or None
		self._prompt_offered = False
		self._pending_path = self._settings.live_data_path or ""
		# Set by the host app (see object_editor.py) -- called when the
		# Settings tab's refresh button is clicked, to force-rebuild
		# creature_full_index.py's cache regardless of sha1 staleness.
		self.on_refresh_requested = None

		# Silent auto-fill: only attempted once, only when nothing is set yet
		# -- a user who already confirmed/changed this manually is never
		# second-guessed by a fresh detection on a later launch (e.g. they
		# deliberately picked Yubo/Gingo/Rendor data, or a non-default
		# install ryzom.ini doesn't know about).
		if not self._settings.live_data_path:
			detected = live_data.detect_atys_live_data_path()
			if detected is not None:
				self._settings.live_data_path = str(detected)
				self._pending_path = str(detected)
				self._save()

	def is_configured(self) -> bool:
		return live_data.is_valid_live_data_path(self._settings.live_data_path)

	def _needs_setup(self) -> bool:
		return not self.is_configured()

	@property
	def live_data_dir(self):
		return Path(self._settings.live_data_path) if self._settings.live_data_path else None

	def _save(self):
		"""Same reload-fresh-then-overwrite-only-our-field pattern as
		WorkspaceSetupDialog._save() -- avoids clobbering other components'
		concurrent settings changes with a stale in-memory copy."""
		fresh = app_settings.load()
		fresh.live_data_path = self._settings.live_data_path
		app_settings.save(fresh)
		self._settings = fresh

	def draw(self):
		"""Call once per frame from the host app (see object_editor.py's
		draw_ui()), same spot as workspace_setup_dialog.draw()."""
		self._poll_folder_dialog()

		if not self._prompt_offered and self._needs_setup():
			self._prompt_offered = True
			imgui.open_popup(_POPUP_ID)

		self._draw_prompt_popup()

	def _draw_prompt_popup(self):
		# Same always=True rationale as WorkspaceSetupDialog._draw_prompt_popup():
		# this can open on essentially the very first ImGui frame.
		if imgui.is_popup_open(_POPUP_ID):
			center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text_wrapped(
			"Ryzom Forgery couldn't automatically find your Ryzom Live install "
			"(ryzom.ini not found, or it doesn't point at a valid data folder). "
			"Point it at the \"data\" folder of a Ryzom Live client install -- "
			"needed for real skeleton/animation lookups.")
		imgui.spacing()
		self._draw_path_row()
		imgui.separator()

		ready = live_data.is_valid_live_data_path(self._pending_path)
		imgui.begin_disabled(not ready)
		if imgui.button("Confirm"):
			self._settings.live_data_path = self._pending_path
			self._save()
			imgui.close_current_popup()
		imgui.end_disabled()
		if not ready and imgui.is_item_hovered():
			imgui.set_tooltip("Pick a folder containing creature.packed_sheets (a Ryzom Live \"data\" folder).")
		imgui.end_popup()

	def _draw_path_row(self):
		imgui.text("Data folder:")
		imgui.same_line()
		path_text = self._pending_path or "(not set)"
		imgui.text_disabled(_truncate_path_to_width(path_text, 260))
		if self._pending_path and imgui.is_item_hovered():
			imgui.set_tooltip(self._pending_path)
		imgui.same_line()
		if imgui.button("Choose folder..."):
			self._folder_dialog = pfd.select_folder("Choose Ryzom Live's \"data\" folder", self._pending_path or "")
		if self._pending_path and not live_data.is_valid_live_data_path(self._pending_path):
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), "This folder doesn't look like a Ryzom data folder.")

	def _poll_folder_dialog(self):
		if self._folder_dialog is None or not self._folder_dialog.ready(0):
			return
		result = self._folder_dialog.result()
		self._folder_dialog = None
		if result:
			self._pending_path = result

	def draw_settings_content(self):
		"""Embedded in the host app's Settings tab, same spot as
		workspace_setup_dialog.py's draw_settings_content() -- freely
		editable any time, not just at first launch."""
		self._poll_folder_dialog()
		if self._pending_path != (self._settings.live_data_path or ""):
			# A change made from Settings (not the mandatory popup) applies
			# immediately once it resolves to a valid folder -- no separate
			# "Confirm" step needed here, unlike the blocking popup, since
			# there's no gate being unblocked.
			if live_data.is_valid_live_data_path(self._pending_path):
				self._settings.live_data_path = self._pending_path
				self._save()

		label = "Ryzom Live data: "
		path_text = self._settings.live_data_path or "(not set)"

		style = imgui.get_style()
		# Two icon buttons on this row now (folder picker + refresh) --
		# available must reserve space for both, not just one, or the
		# second overflows the window's right edge (found by Nuno,
		# 2026-09-03, right after adding the refresh button here).
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width * 2 - style.item_spacing.x * 2)

		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if self._settings.live_data_path and imgui.is_item_hovered():
			imgui.set_tooltip(self._settings.live_data_path)
		imgui.same_line()
		# "##live-data-folder": without a unique suffix this collides with
		# workspace_setup_dialog.py's own identical-glyph "Choose folder..."
		# button -- both rows are drawn back to back in the same Settings >
		# Paths collapsing header (same ImGui ID scope), and imgui.button()
		# hashes its ID from the label text alone when there's no "##" in it,
		# so two buttons with the exact same icon glyph and no window/PushID
		# boundary between them resolve to the SAME widget ID (found by
		# Nuno, 2026-09-03 -- same class of mistake to watch for on every
		# icon-only button added next to an existing one in the same scope).
		if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN + "##live-data-folder", "Choose folder..."):
			self._folder_dialog = pfd.select_folder("Choose Ryzom Live's \"data\" folder", self._settings.live_data_path or "")
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_ARROWS_ROTATE + "##live-data-refresh", "Force-rebuild the creature/animation index"):
			if self.on_refresh_requested is not None:
				self.on_refresh_requested()
