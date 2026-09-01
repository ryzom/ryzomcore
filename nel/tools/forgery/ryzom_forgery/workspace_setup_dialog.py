"""First-launch (and Settings-tab) UI for `settings.workspaces_root` --
the single folder, shared by every Forgery app, that holds one subfolder
per workspace (see workspaces.py). Mirrors export_dialog.py's non-blocking
`portable_file_dialogs.select_folder` polling pattern.
"""

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd

from ryzom_forgery import settings as app_settings
from ryzom_forgery.workspaces import (
	active_workspace_path, create_workspace, ensure_structure, is_root_configured, list_workspaces,
	open_in_system_file_manager, workspace_path,
)

_POPUP_ID = "Set up Workspaces folder"
_NEW_WORKSPACE_POPUP_ID = "New workspace"
_ELLIPSIS = "..."
_NONE_LABEL = "(none)"
_NEW_LABEL = "<new>"


def _icon_button(icon, tooltip):
	clicked = imgui.button(icon)
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


def _truncate_path_to_width(path, max_width):
	"""Same binary-search-for-longest-fitting-tail approach as
	search_paths_dialog.py's own copy -- ImGui doesn't wrap a same_line() row
	on its own, so a long path has to be shrunk to whatever pixel width is
	actually left, truncated from the front (the tail is usually the
	meaningful part), full path always available as a tooltip on hover."""
	if imgui.calc_text_size(path).x <= max_width:
		return path
	if imgui.calc_text_size(_ELLIPSIS).x > max_width:
		return _ELLIPSIS

	lo, hi, best = 0, len(path), _ELLIPSIS
	while lo <= hi:
		mid = (lo + hi) // 2
		candidate = _ELLIPSIS + path[-mid:] if mid > 0 else _ELLIPSIS
		if imgui.calc_text_size(candidate).x <= max_width:
			best = candidate
			lo = mid + 1
		else:
			hi = mid - 1
	return best


class WorkspaceSetupDialog:
	def __init__(self):
		self._settings = app_settings.load()
		self._folder_dialog = None  # active portable_file_dialogs.select_folder
		# Cancelling the startup prompt only skips it for this session -- the
		# folder can still be set later from the Settings tab (draw_settings_content()).
		self._prompt_dismissed = False
		self._prompt_offered = False

		self._new_workspace_pending = False  # set inside the combo, popup opened right after end_combo()
		self._new_workspace_name = ""
		self._new_workspace_error = ""

		# Set by the host app (see object_editor.py) -- called with the new
		# active workspace's Path (or None) every time it changes, so search
		# resolution (SearchPathsDialog.set_workspace_dir()) stays in sync.
		self.on_active_workspace_changed = None

	def is_configured(self) -> bool:
		return is_root_configured(self._settings.workspaces_root)

	@property
	def active_workspace_name(self):
		return self._settings.active_workspace

	def workspace_names(self):
		if not self.is_configured():
			return []
		return list_workspaces(self._settings.workspaces_root)

	@property
	def active_workspace_dir(self):
		return active_workspace_path(self._settings)

	def set_active_workspace(self, name):
		self._settings.active_workspace = name
		self._save()
		if name is not None:
			# Backfills any SUBDIRS added since this workspace was first
			# created (e.g. a folder introduced in a later Forgery version)
			# -- idempotent, so this is safe on every activation, not just
			# creation.
			ensure_structure(workspace_path(self._settings.workspaces_root, name))
		if self.on_active_workspace_changed is not None:
			self.on_active_workspace_changed(self.active_workspace_dir)

	def _save(self):
		"""Same pattern as export_dialog.py's _save(): reload fresh and only
		overwrite our own fields, so other components' concurrent settings
		changes aren't clobbered by a stale in-memory copy."""
		fresh = app_settings.load()
		fresh.workspaces_root = self._settings.workspaces_root
		fresh.active_workspace = self._settings.active_workspace
		app_settings.save(fresh)
		self._settings = fresh

	def draw(self):
		"""Call once per frame from the host app (see app.py's draw_ui())."""
		self._poll_folder_dialog()

		if not self._prompt_offered and not self._prompt_dismissed and not self.is_configured():
			self._prompt_offered = True
			imgui.open_popup(_POPUP_ID)

		self._draw_prompt_popup()
		self._draw_new_workspace_popup()

	def draw_active_workspace_row(self, width=160):
		"""Own dedicated row -- "Current workspace: [combo] [open in system
		file manager]" -- called from the host app's panel (see
		object_editor.py's draw_panel()), above the Save/Export/Quit bar.
		The combo is disabled with an explanatory tooltip until a Workspaces
		root is configured, since there's nothing to list otherwise."""
		names = self.workspace_names()
		current = self._settings.active_workspace if self._settings.active_workspace in names else None
		preview = current or _NONE_LABEL

		imgui.text("Current workspace:")
		imgui.same_line()

		imgui.begin_disabled(not self.is_configured())
		imgui.set_next_item_width(width)
		if imgui.begin_combo("##workspace-selector", preview):
			clicked, _ = imgui.selectable(_NONE_LABEL, current is None)
			if clicked:
				self.set_active_workspace(None)
			if names:
				imgui.separator()
				for name in names:
					clicked, _ = imgui.selectable(name, name == current)
					if clicked:
						self.set_active_workspace(name)
			imgui.separator()
			clicked, _ = imgui.selectable(_NEW_LABEL, False)
			if clicked:
				self._new_workspace_name = ""
				self._new_workspace_error = ""
				self._new_workspace_pending = True
			imgui.end_combo()
		imgui.end_disabled()
		if not self.is_configured() and imgui.is_item_hovered():
			imgui.set_tooltip("Set up a Workspaces folder first (Settings tab)")

		imgui.same_line()
		active_path = self.active_workspace_dir
		imgui.begin_disabled(active_path is None)
		if _icon_button(fa_icons.ICON_FA_EYE, "Open workspace folder in system file manager"):
			open_in_system_file_manager(active_path)
		imgui.end_disabled()

		# Deferred: begin_popup_modal() can't nest inside begin_combo()'s own
		# popup, so the actual open_popup() call happens right after end_combo().
		if self._new_workspace_pending:
			self._new_workspace_pending = False
			imgui.open_popup(_NEW_WORKSPACE_POPUP_ID)

	def _draw_new_workspace_popup(self):
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_NEW_WORKSPACE_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text("Workspace name:")
		imgui.set_next_item_width(220)
		_, self._new_workspace_name = imgui.input_text("##new-workspace-name", self._new_workspace_name)
		if self._new_workspace_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._new_workspace_error)

		if imgui.button("Create"):
			name = self._new_workspace_name.strip()
			if not name:
				self._new_workspace_error = "Name can't be empty."
			elif name in self.workspace_names():
				self._new_workspace_error = "A workspace with this name already exists."
			else:
				create_workspace(self._settings.workspaces_root, name)
				# Convenience default: a fresh workspace starts out synced to
				# wherever the user last pointed any workspace's sync folder
				# at (see settings.py's Settings.last_workspace_sync_folder),
				# rather than unconfigured every time -- still just a
				# starting point, editable right away from Settings > Tools.
				# Same reload-fresh-then-overwrite-only-our-fields pattern as
				# _save(), but for a different field, so kept separate from it.
				if self._settings.last_workspace_sync_folder is not None:
					fresh = app_settings.load()
					fresh.workspace_sync_folders[name] = self._settings.last_workspace_sync_folder
					app_settings.save(fresh)
					self._settings = fresh
				self.set_active_workspace(name)
				imgui.close_current_popup()
		imgui.same_line()
		if imgui.button("Cancel"):
			imgui.close_current_popup()
		imgui.end_popup()

	def _draw_prompt_popup(self):
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text_wrapped(
			"No Workspaces folder is set up yet. It holds your editable workspaces "
			"(one subfolder each), shared by every Forgery app -- shapes are only ever "
			"saved into a workspace, never elsewhere.")
		imgui.spacing()
		if imgui.button("Choose folder..."):
			self._folder_dialog = pfd.select_folder("Choose Workspaces folder", "")
		imgui.same_line()
		if imgui.button("Later"):
			self._prompt_dismissed = True
			imgui.close_current_popup()
		imgui.end_popup()

	def _poll_folder_dialog(self):
		if self._folder_dialog is None or not self._folder_dialog.ready(0):
			return
		result = self._folder_dialog.result()
		self._folder_dialog = None
		if not result:
			return
		self._settings.workspaces_root = result
		self._save()
		imgui.close_current_popup()

	def draw_settings_content(self):
		"""Embedded in the host app's Settings tab, same spot as
		export_dialog.py's draw_settings_content()."""
		label = "Workspaces folder: "
		path_text = self._settings.workspaces_root or "(not set)"

		style = imgui.get_style()
		button_width = imgui.calc_text_size(fa_icons.ICON_FA_FOLDER_OPEN).x + style.frame_padding.x * 2
		available = (imgui.get_content_region_avail().x - imgui.calc_text_size(label).x
		             - button_width - style.item_spacing.x)

		imgui.text(label)
		imgui.same_line()
		imgui.text(_truncate_path_to_width(path_text, max(available, 20)))
		if self._settings.workspaces_root and imgui.is_item_hovered():
			imgui.set_tooltip(self._settings.workspaces_root)
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Choose folder..."):
			self._folder_dialog = pfd.select_folder("Choose Workspaces folder", self._settings.workspaces_root or "")
