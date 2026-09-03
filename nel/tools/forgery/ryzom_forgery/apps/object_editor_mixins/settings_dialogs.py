"""ObjectEditorApp mixin: Settings tab drawers and their file-dialog polling
-- external image/text editor pickers, workspace sync folder, exclusion
rules, repository paths, UI font/DPI, plus the panel's bottom bar
(Save/Export/Quit). Split out of object_editor.py, see the "Split
object_editor.py into theme files" chantier in
project-todos/ryzom-core/forgery-object-editor.md for the split's rationale
and ordering.

Imports _icon_button/_colored_button/button-color constants from
object_editor_mixins.ui_helpers, NOT from object_editor.py itself -- see
ui_helpers.py's module docstring for why a mixin must never import back from
object_editor.py (circular-import trap specific to how this app is
launched).
"""

from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, portable_file_dialogs as pfd

from pynel import repository_paths

from ryzom_forgery.app import _AVAILABLE_FONTS, _dpi_scale
from ryzom_forgery import settings as app_settings
from ryzom_forgery.shape_export import EXPORT_FORMATS
from ryzom_forgery.workspace_setup_dialog import _truncate_path_to_width
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_colored_button, _icon_button, _QUICK_EXPORT_BUTTON_COLOR, _QUIT_BUTTON_COLOR, _SAVE_BUTTON_COLOR,
	_SYNC_NOW_COLOR, _EXPORT_AS_BUTTON_COLOR, _WRITABLE_SHAPE_TYPES,
)


class SettingsDialogsMixin:
	def _poll_skeleton_file_dialog(self):
		if self._skeleton_file_dialog is None or not self._skeleton_file_dialog.ready(0):
			return
		result = self._skeleton_file_dialog.result()
		self._skeleton_file_dialog = None
		if result:
			path = Path(result[0])
			self._load_skeleton_bytes(path.read_bytes(), path.name)

	def _poll_animation_file_dialog(self):
		if self._animation_file_dialog is None or not self._animation_file_dialog.ready(0):
			return
		result = self._animation_file_dialog.result()
		self._animation_file_dialog = None
		if result:
			path = Path(result[0])
			self._apply_bone_preview_animation_bytes(path.read_bytes(), path.name)

	def _poll_image_editor_dialog(self):
		if self._image_editor_dialog is None or not self._image_editor_dialog.ready(0):
			return
		result = self._image_editor_dialog.result()
		self._image_editor_dialog = None
		if result:
			fresh = app_settings.load()
			fresh.image_editor_path = result[0]
			app_settings.save(fresh)

	def _poll_text_editor_dialog(self):
		if self._text_editor_dialog is None or not self._text_editor_dialog.ready(0):
			return
		result = self._text_editor_dialog.result()
		self._text_editor_dialog = None
		if result:
			fresh = app_settings.load()
			fresh.text_editor_path = result[0]
			app_settings.save(fresh)

	def _draw_image_editor_settings(self):
		"""Settings tab -- lets the user pick an external image editor
		executable, used by the Textures tab's "Edit" button (see
		_draw_texture_edit_button()) once a texture already lives in the
		active workspace."""
		settings = app_settings.load()
		label = "Image editor: "
		path_text = settings.image_editor_path or "(not set)"

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		flashing = self._begin_attention_flash("image_editor_path")
		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if settings.image_editor_path and imgui.is_item_hovered():
			imgui.set_tooltip(settings.image_editor_path)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##image-editor", "Choose an image editor executable..."):
			self._image_editor_dialog = pfd.open_file("Choose image editor executable")
		self._end_attention_flash(flashing)

	def _draw_text_editor_settings(self):
		"""Settings tab -- lets the user pick an external text editor
		executable, used by the Panoply section's "Edit" button (see
		_draw_global_panoply_section()) once a workspace panoply.cfg already
		exists. Same pattern as _draw_image_editor_settings() above."""
		settings = app_settings.load()
		label = "Text editor: "
		path_text = settings.text_editor_path or "(not set)"

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		flashing = self._begin_attention_flash("text_editor_path")
		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if settings.text_editor_path and imgui.is_item_hovered():
			imgui.set_tooltip(settings.text_editor_path)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##text-editor", "Choose a text editor executable..."):
			self._text_editor_dialog = pfd.open_file("Choose text editor executable")
		self._end_attention_flash(flashing)

	def _poll_workspace_sync_folder_dialog(self):
		if self._workspace_sync_folder_dialog is None or not self._workspace_sync_folder_dialog.ready(0):
			return
		result = self._workspace_sync_folder_dialog.result()
		self._workspace_sync_folder_dialog = None
		workspace_name = self.workspace_setup_dialog.active_workspace_name
		if not result or workspace_name is None:
			return
		fresh = app_settings.load()
		fresh.workspace_sync_folders[workspace_name] = result
		fresh.last_workspace_sync_folder = result
		app_settings.save(fresh)
		self.workspace_sync.set_sync_folder(result)

	def _draw_workspace_sync_settings(self):
		"""Settings tab -- lets the user pick an external folder the active
		workspace's .shape/.anim/.skel/.dds files get auto-mirrored into,
		flat (see workspace_sync.py). Per-workspace: switching the active
		workspace shows/edits that workspace's own folder, not a single
		global one (see _on_active_workspace_changed())."""
		workspace_name = self.workspace_setup_dialog.active_workspace_name
		label = "Sync folder: "
		path_text = "(no active workspace)" if workspace_name is None else (
			app_settings.load().workspace_sync_folders.get(workspace_name) or "(not set)")

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		imgui.text(label)
		imgui.same_line()
		imgui.begin_disabled(workspace_name is None)
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if workspace_name is not None and imgui.is_item_hovered():
			imgui.set_tooltip(path_text)
		imgui.same_line()
		if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##sync-folder", "Choose a folder to mirror this workspace's .shape/.anim/.skel/.dds files into..."):
			current = app_settings.load().workspace_sync_folders.get(workspace_name) if workspace_name else None
			self._workspace_sync_folder_dialog = pfd.select_folder("Choose sync folder", current or "")
		imgui.end_disabled()

		# Only relevant once a sync folder is actually configured -- catches
		# up anything that predates the live watch (see
		# WorkspaceSyncWatcher.refresh_fully_synced()).
		sync_folder_set = workspace_name is not None and app_settings.load().workspace_sync_folders.get(workspace_name)
		if sync_folder_set and not self.workspace_sync.is_fully_synced():
			imgui.begin_disabled(self.workspace_sync.is_syncing())
			if _colored_button("Sync now", _SYNC_NOW_COLOR):
				self.workspace_sync.sync_now()
			imgui.end_disabled()
			if self.workspace_sync.is_syncing():
				imgui.same_line()
				imgui.text_disabled("Syncing...")

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

	def _draw_exclusion_rules_settings(self):
		"""Settings tab, "Paths" section -- edits Settings.exclusion_rules
		(see settings.py's ExclusionRule, and workspaces.py's virtual
		category scan / search-path indexing, both of which consume it):
		a folder-kind rule hides everything under it everywhere (Wexplorer
		display and search/indexing alike); a file-kind rule only hides
		from search -- matching files still show, bucketed into "others"
		(see ExclusionRule's own docstring). Same immediate-save-on-change,
		no separate Save button, convention as every other Settings field
		in this tab. Ships with two folder defaults (exports/, build/, see
		settings.py's _default_exclusion_rules()) but nothing here treats
		those specially -- removable/editable like any other entry."""
		imgui.text("Excluded:")
		if imgui.is_item_hovered():
			imgui.set_tooltip(
				"Excluded from every workspace -- folders are hidden everywhere, "
				"file patterns only from search (still shown, under \"others\")")
		settings = app_settings.load()
		remove_index = None
		for index, rule in enumerate(settings.exclusion_rules):
			imgui.push_id(f"exclusion-rule-{index}")
			is_folder = rule.kind == app_settings.EXCLUSION_KIND_FOLDER
			icon = fa_icons.ICON_FA_FOLDER if is_folder else fa_icons.ICON_FA_FILE
			tooltip = "Folder (click to switch to file pattern)" if is_folder \
				else "File pattern (click to switch to folder)"
			if _icon_button(icon, tooltip, active=is_folder):
				rule.kind = app_settings.EXCLUSION_KIND_FILE if is_folder else app_settings.EXCLUSION_KIND_FOLDER
				app_settings.save(settings)
			imgui.same_line()
			imgui.set_next_item_width(220)
			changed, new_pattern = imgui.input_text("##exclusion-rule-pattern", rule.pattern)
			if changed:
				rule.pattern = new_pattern
				app_settings.save(settings)
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_TRASH, "Remove this exclusion rule"):
				remove_index = index
			imgui.pop_id()
		if remove_index is not None:
			del settings.exclusion_rules[remove_index]
			app_settings.save(settings)

		if _icon_button(f"{fa_icons.ICON_FA_PLUS}##add-exclusion-rule", "Add exclusion rule"):
			settings.exclusion_rules.append(app_settings.ExclusionRule(pattern="", kind=app_settings.EXCLUSION_KIND_FOLDER))
			app_settings.save(settings)

	def _draw_repository_paths_settings(self):
		"""Settings tab -- one folder picker per pynel.repository_paths.REPOSITORIES
		entry, so any tool built on pynel (this app included -- see
		_bake_panoply_real()'s ryzom-data dependency) can resolve "where is
		ryzom-data on this machine" without asking the user again. Stored
		outside Forgery's own settings.toml (see repository_paths.py's own
		docstring on why)."""
		configured = repository_paths.load()
		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2

		for repo_name in repository_paths.REPOSITORIES:
			label = f"{repo_name}: "
			path_text = configured.get(repo_name) or "(not set)"
			available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
			             - button_width - style.item_spacing.x)

			flashing = self._begin_attention_flash(repo_name)
			imgui.text(label)
			imgui.same_line()
			imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
			if repo_name in configured and imgui.is_item_hovered():
				imgui.set_tooltip(configured[repo_name])
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##repo-{repo_name}", f"Choose the {repo_name} checkout..."):
				self._repository_paths_dialog_repo = repo_name
				self._repository_paths_dialog = pfd.select_folder(f"Choose {repo_name}", configured.get(repo_name, ""))
			self._end_attention_flash(flashing)

	def _draw_ui_font_settings(self):
		"""Settings tab -- lets the user pick the UI text font/size (see
		ForgeryApp._load_ui_font(), Settings.ui_font_name/ui_font_size).
		Takes effect on the next launch only -- no live font-atlas rebuild."""
		settings = app_settings.load()
		font_names = list(_AVAILABLE_FONTS)
		current_index = font_names.index(settings.ui_font_name) if settings.ui_font_name in font_names else 0

		imgui.text("Font: ")
		imgui.same_line()
		imgui.set_next_item_width(200)
		changed, new_index = imgui.combo("##ui-font-name", current_index, font_names)
		if changed:
			settings.ui_font_name = font_names[new_index]
			app_settings.save(settings)

		imgui.text("Size: ")
		imgui.same_line()
		imgui.set_next_item_width(100)
		changed, new_size = imgui.drag_float("##ui-font-size", settings.ui_font_size, v_speed=0.5, v_min=8.0, v_max=32.0)
		if changed:
			settings.ui_font_size = new_size
			app_settings.save(settings)

		imgui.text("DPI scale: ")
		imgui.same_line()
		imgui.set_next_item_width(100)
		changed, new_dpi_scale = imgui.drag_float(
			"##ui-dpi-scale", settings.dpi_scale, v_speed=0.01, v_min=0.5, v_max=3.0, format="%.2fx")
		if changed:
			new_dpi_scale = max(0.5, min(3.0, new_dpi_scale))
			settings.dpi_scale = new_dpi_scale
			app_settings.save(settings)
			# Text preview only (see app.py's set_live_ui_scale_preview()) --
			# paddings/spacing/button sizes still need a manual restart, see
			# the text below (no in-app restart button: doesn't work when
			# launched through ryztart).
			self.set_live_ui_scale_preview(_dpi_scale() * new_dpi_scale)

		imgui.text_disabled("Restart Patina manually for a font/size/DPI change to fully take effect.")

	def _draw_bottom_bar(self):
		"""Pinned at the very bottom of the panel: Save (only for a loaded,
		writable shape -- always targets the active workspace, see the
		Workspaces chantier in `.todo/forgery-object-editor.md`), Export
		(format picker, then the existing ExportDialog flow -- see
		ExportDialog.export() -- applied to the shape's current in-memory
		state, edits included, not a re-read from disk/bnp), and Quit flush
		against the right edge. Always drawn (even with nothing loaded) so
		Quit stays reachable. The active-workspace row used to live here too
		-- moved to the top of the Explorer window instead (see
		Explorer.extra_header in __init__), always reachable there regardless
		of what's loaded in this panel."""
		imgui.separator()

		writable = self.shape_file is not None and self.shape_file.type_name in _WRITABLE_SHAPE_TYPES
		if writable:
			save_path = self._workspace_shape_save_path()
			imgui.begin_disabled(save_path is None)
			if _colored_button(f"{fa_icons.ICON_FA_FLOPPY_DISK} Save", _SAVE_BUTTON_COLOR):
				self._on_save_clicked()
			imgui.end_disabled()
			if imgui.is_item_hovered() and save_path is None:
				imgui.set_tooltip("Save unavailable -- set up a Workspaces folder and pick an active workspace first")
			imgui.same_line()

			workspace_dir = self.workspace_setup_dialog.active_workspace_dir
			if workspace_dir is not None:
				if _colored_button("Export", _QUICK_EXPORT_BUTTON_COLOR):
					imgui.open_popup("##quick-export-format-popup")
				if imgui.is_item_hovered():
					imgui.set_tooltip(f"Export straight to {workspace_dir / 'exports'}, no prompts")
				if imgui.begin_popup("##quick-export-format-popup"):
					for export_format in EXPORT_FORMATS:
						clicked, _ = imgui.selectable(f"{export_format.label} (.{export_format.extension})", False)
						if clicked:
							exports_dir = workspace_dir / "exports"
							exports_dir.mkdir(parents=True, exist_ok=True)
							self.export_dialog.quick_export(
								self.shape_file.value, self._shape_source_name or "shape", export_format,
								self.search_paths_dialog.find_texture, exports_dir)
					imgui.separator()
					bnp_clicked, _ = imgui.selectable(f"Full workspace ({workspace_dir.name}.bnp)", False)
					if bnp_clicked:
						exports_dir = workspace_dir / "exports"
						exports_dir.mkdir(parents=True, exist_ok=True)
						self.export_dialog.quick_export_workspace_bnp(workspace_dir, exports_dir)
					imgui.end_popup()
				imgui.same_line()

			if _colored_button("Export as...", _EXPORT_AS_BUTTON_COLOR):
				imgui.open_popup("##export-format-popup")
			if imgui.begin_popup("##export-format-popup"):
				for export_format in EXPORT_FORMATS:
					clicked, _ = imgui.selectable(f"{export_format.label} (.{export_format.extension})", False)
					if clicked:
						source_folder = (
							self._shape_source_path.parent if self._shape_source_path is not None else None)
						self.export_dialog.export(
							self.shape_file.value, self._shape_source_name or "shape", export_format,
							self.search_paths_dialog.find_texture, source_folder=source_folder)
				if workspace_dir is not None:
					imgui.separator()
					bnp_clicked, _ = imgui.selectable(f"Full workspace ({workspace_dir.name}.bnp)", False)
					if bnp_clicked:
						source_folder = (
							self._shape_source_path.parent if self._shape_source_path is not None else None)
						self.export_dialog.export_workspace_bnp(workspace_dir, source_folder=source_folder)
				imgui.end_popup()
			imgui.same_line()

		quit_label = "Quit"
		quit_width = imgui.calc_text_size(quit_label).x + imgui.get_style().frame_padding.x * 2
		avail = imgui.get_content_region_avail().x
		if avail > quit_width:
			imgui.set_cursor_pos_x(imgui.get_cursor_pos_x() + avail - quit_width)
		if _colored_button(quit_label, _QUIT_BUTTON_COLOR):
			self.userExit()

		if writable:
			self._draw_save_confirmation_popup()
			if self._save_status:
				imgui.text_wrapped(self._save_status)
