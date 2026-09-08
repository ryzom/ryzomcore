"""Settings-tab UI for `pynel.repository_paths` -- one folder picker + Clone
button per `REPOSITORIES` entry (ryzom-core/ryzom-data/ryzom-private-data/
ryzom-docker). A standalone module, not a method on
object_editor_mixins.settings_dialogs.SettingsDialogsMixin, precisely so any
Forgery app can instantiate and draw it -- same reasoning as
live_data_setup_dialog.py's own module docstring. `draw(app)` takes the host
app itself (not just self) since it calls `app._begin_attention_flash`/
`app._end_attention_flash` (defined on ForgeryApp, see app.py's
request_settings_attention() docstring -- shared by every Forgery app, not
Patina-specific).
"""

import threading
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, portable_file_dialogs as pfd

from pynel import repository_paths

from ryzom_forgery.apps.object_editor_mixins.ui_helpers import _icon_button
from ryzom_forgery.workspace_setup_dialog import _truncate_path_to_width


class RepositoryPathsDialog:
	def __init__(self):
		self._repository_paths_dialog = None  # active portable_file_dialogs.select_folder, or None
		self._repository_paths_dialog_repo = None  # which REPOSITORIES entry _repository_paths_dialog is for
		self._clone_target_dialog = None  # active portable_file_dialogs.select_folder for clone target, or None
		self._clone_dialog_repo = None  # which repo the clone dialog is for
		self._clone_status_message = None  # status message from clone_repo() for display

	def _poll_repository_paths_dialog(self):
		if self._repository_paths_dialog is None or not self._repository_paths_dialog.ready(0):
			return
		result = self._repository_paths_dialog.result()
		repo_name = self._repository_paths_dialog_repo
		self._repository_paths_dialog = None
		self._repository_paths_dialog_repo = None
		if not result:
			return
		repository_paths.set_path(repo_name, result)

	def _poll_clone_target_dialog(self):
		"""Poll the clone target folder selection dialog and trigger the clone."""
		if self._clone_target_dialog is None or not self._clone_target_dialog.ready(0):
			return
		result = self._clone_target_dialog.result()
		repo_name = self._clone_dialog_repo
		self._clone_target_dialog = None
		self._clone_dialog_repo = None
		if not result:
			return

		# Construct the target path (parent folder + repo name)
		target_path = str(Path(result[0]) / repo_name)

		# Clone using pynel's mutualized function
		def clone_and_set_status():
			self._clone_status_message = repository_paths.clone_repo(repo_name, target_path)

		thread = threading.Thread(target=clone_and_set_status, daemon=True)
		thread.start()

	def _draw_clone_status(self):
		"""Draw clone status messages."""
		if self._clone_status_message:
			msg = self._clone_status_message
			if msg == "success":
				imgui.text_colored(f"{fa_icons.ICON_FA_CHECK_CIRCLE} Successfully cloned repository", (0, 1, 0, 1))
			elif msg.startswith("error:"):
				imgui.text_colored(f"{fa_icons.ICON_FA_EXCLAMATION_CIRCLE} {msg}", (1, 0, 0, 1))
			# Clear after displaying
			self._clone_status_message = None

	def draw(self, app):
		"""Settings tab, "Ryzom Paths" section -- one folder picker per
		pynel.repository_paths.REPOSITORIES entry, so any tool built on
		pynel can resolve "where is ryzom-data on this machine" without
		asking the user again. Stored outside Forgery's own settings.toml
		(see repository_paths.py's own docstring on why)."""
		self._poll_repository_paths_dialog()
		self._poll_clone_target_dialog()
		self._draw_clone_status()

		configured = repository_paths.load()
		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		clone_button_width = imgui.calc_text_size(fa_icons.ICON_FA_DOWNLOAD).x + style.frame_padding.x * 2

		for repo_name in repository_paths.REPOSITORIES:
			label = f"{repo_name}: "
			path_text = configured.get(repo_name) or "(not set)"
			available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
			             - button_width - clone_button_width - style.item_spacing.x * 2)

			flashing = app._begin_attention_flash(repo_name)
			imgui.text(label)
			imgui.same_line()
			imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
			if repo_name in configured and imgui.is_item_hovered():
				imgui.set_tooltip(configured[repo_name])
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##repo-{repo_name}", f"Choose the {repo_name} checkout..."):
				self._repository_paths_dialog_repo = repo_name
				self._repository_paths_dialog = pfd.select_folder(f"Choose {repo_name}", configured.get(repo_name, ""))
			imgui.same_line()
			clone_tooltip = f"Clone {repo_name} repository"
			path_exists = configured.get(repo_name) and Path(configured[repo_name]).exists()
			imgui.begin_disabled(path_exists)
			if _icon_button(f"{fa_icons.ICON_FA_DOWNLOAD}##clone-{repo_name}", clone_tooltip):
				# Open folder picker for target directory
				self._clone_dialog_repo = repo_name
				self._clone_target_dialog = pfd.select_folder(f"Choose parent folder for {repo_name}")
			imgui.end_disabled()
			if path_exists and imgui.is_item_hovered():
				imgui.set_tooltip(f"{repo_name} is already configured at {configured[repo_name]}")
			app._end_attention_flash(flashing)
