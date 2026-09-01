"""First-launch (and Settings-tab) UI for `settings.workspaces_root` --
the single folder, shared by every Forgery app, that holds one subfolder
per workspace (see workspaces.py). Mirrors export_dialog.py's non-blocking
`portable_file_dialogs.select_folder` polling pattern.
"""

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd

from ryzom_forgery import settings as app_settings
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.workspaces import (
	active_workspace_path, create_workspace, ensure_structure, is_root_configured, list_workspaces,
	open_in_system_file_manager, workspace_path,
)

_POPUP_ID = "Welcome to Ryzom Forgery"
_NEW_WORKSPACE_POPUP_ID = "New workspace"
_ELLIPSIS = "..."
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
		self._prompt_offered = False

		# Mandatory setup popup's own pending fields -- not written to
		# settings until "Finish" is clicked (see _draw_prompt_popup()).
		self._setup_workspace_name = ""
		self._setup_dpi_scale = self._settings.dpi_scale
		self._setup_error = ""

		self._new_workspace_pending = False  # set inside the combo, popup opened right after end_combo()
		self._new_workspace_name = ""
		self._new_workspace_error = ""

		# Set by the host app (see object_editor.py) -- called with the new
		# active workspace's Path (or None) every time it changes, so search
		# resolution (SearchPathsDialog.set_workspace_dir()) stays in sync.
		self.on_active_workspace_changed = None
		# Set by the host app (see app.py's ForgeryApp.__init__) -- called
		# every frame the setup popup's DPI control is up, with the
		# candidate value being tried, so its text preview updates live
		# (see app.py's set_live_ui_scale_preview()).
		self.on_dpi_preview_changed = None
		# Set by the host app -- called once the mandatory setup popup's
		# "Finish" is clicked, after its settings are saved. A relaunch is
		# needed for the DPI value to fully apply (paddings/spacing/button
		# sizes are only set once at startup, not live -- see
		# app.py's set_live_ui_scale_preview() for why).
		self.on_setup_finished = None

	def is_configured(self) -> bool:
		return is_root_configured(self._settings.workspaces_root)

	def _needs_setup(self) -> bool:
		"""True until both a Workspaces folder and an active workspace are
		set -- having no active workspace is no longer a valid steady state
		(see the mandatory setup popup below), so this is stricter than
		is_configured() (which only gates the "Current workspace" combo on
		there being anything to list)."""
		return not (self.is_configured() and self._settings.active_workspace)

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

		# Self-heal: a Workspaces folder that already has workspaces in it
		# (hand-edited settings.toml, or upgrading from before this field
		# existed) but no active one picked yet -- silently activate the
		# first one instead of forcing the mandatory setup popup below on
		# someone who's already set this up.
		if self.is_configured() and not self._settings.active_workspace:
			names = self.workspace_names()
			if names:
				self.set_active_workspace(names[0])

		if not self._prompt_offered and self._needs_setup():
			self._prompt_offered = True
			imgui.open_popup(_POPUP_ID)

		# OpenPopup()/BeginPopupModal() both resolve their popup's actual ID
		# using the current window/ID-stack context at the time they're
		# called -- draw_active_workspace_row() (called from inside the
		# Explorer window, see object_editor.py's Explorer.extra_header)
		# only *requests* this one (self._new_workspace_pending) rather than
		# calling open_popup() itself, so the real call happens here instead,
		# at the same top-level context _draw_new_workspace_popup() calls
		# begin_popup_modal() from right below. Calling OpenPopup() from
		# inside Explorer's window context while BeginPopupModal() is called
		# from here (no parent window) previously computed two different
		# IDs for the same string, so the popup could never actually open.
		if self._new_workspace_pending:
			self._new_workspace_pending = False
			imgui.open_popup(_NEW_WORKSPACE_POPUP_ID)

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
		# Self-heal: the active workspace named in settings no longer
		# exists on disk (renamed/deleted outside the app) but others do --
		# having no active workspace isn't a valid steady state anymore, so
		# fall back to the first one rather than showing "(none)".
		if current is None and names:
			self.set_active_workspace(names[0])
			current = names[0]
		preview = current or "(none)"

		imgui.text("Current workspace:")
		imgui.same_line()

		imgui.begin_disabled(not self.is_configured())
		imgui.set_next_item_width(width)
		if imgui.begin_combo("##workspace-selector", preview):
			if names:
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

		# self._new_workspace_pending only *requests* the popup here --
		# the actual open_popup() call happens in draw() instead, at the
		# same top-level context begin_popup_modal() is called from (see
		# draw()'s own comment for why: this method runs nested inside the
		# Explorer window, which would give OpenPopup() a different
		# resolved ID than a call from draw()'s top-level context).

	def _draw_new_workspace_popup(self):
		center_next_popup()
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
		"""Mandatory first-launch setup -- no "Later"/skip: a Workspaces
		folder and an active workspace are required for the app to be
		usable at all (shapes are only ever saved into a workspace, never
		elsewhere), so this stays open until "Finish" actually completes
		it."""
		# Only touch SetNextWindowPos while this popup is actually open --
		# this method runs every frame for the app's entire lifetime (not
		# gated behind an instance flag like the other popups below), so
		# calling center_next_popup() unconditionally left a pending
		# "next window position" unconsumed on every frame this popup
		# *isn't* open, which was interfering with the very next
		# Begin-family call that frame (_draw_new_workspace_popup()'s own
		# begin_popup_modal() -- symptom: <new> in the combo stopped
		# opening its popup at all). always=True: can open on essentially
		# the very first ImGui frame, before Panda3D's requested window
		# geometry has actually settled -- see center_next_popup()'s own
		# docstring.
		if imgui.is_popup_open(_POPUP_ID):
			center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text_wrapped(
			"A Workspaces folder is required before you can do anything -- it holds "
			"your editable workspaces (one subfolder each), shared by every Forgery "
			"app. Shapes are only ever saved into a workspace, never elsewhere.")
		imgui.spacing()

		imgui.separator()
		imgui.text("Interface scale:")
		imgui.same_line()
		imgui.set_next_item_width(140)
		_, self._setup_dpi_scale = imgui.drag_float(
			"##setup-dpi-scale", self._setup_dpi_scale, v_speed=0.01, v_min=0.5, v_max=3.0, format="%.2fx")
		self._setup_dpi_scale = max(0.5, min(3.0, self._setup_dpi_scale))
		if self.on_dpi_preview_changed is not None:
			self.on_dpi_preview_changed(self._setup_dpi_scale)
		imgui.text_disabled("Text preview updates live; the rest of the interface after Finish.")

		imgui.separator()
		imgui.text("Workspaces folder:")
		imgui.same_line()
		path_text = self._settings.workspaces_root or "(not set)"
		imgui.text_disabled(_truncate_path_to_width(path_text, 220))
		if self._settings.workspaces_root and imgui.is_item_hovered():
			imgui.set_tooltip(self._settings.workspaces_root)
		imgui.same_line()
		if imgui.button("Choose folder..."):
			self._folder_dialog = pfd.select_folder("Choose Workspaces folder", self._settings.workspaces_root or "")

		imgui.separator()
		imgui.text("First workspace name:")
		imgui.set_next_item_width(220)
		_, self._setup_workspace_name = imgui.input_text("##setup-workspace-name", self._setup_workspace_name)
		if self._setup_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._setup_error)
		imgui.separator()

		ready = is_root_configured(self._settings.workspaces_root) and bool(self._setup_workspace_name.strip())
		imgui.begin_disabled(not ready)
		if imgui.button("Finish"):
			self._finish_setup()
		imgui.end_disabled()
		if not ready and imgui.is_item_hovered():
			imgui.set_tooltip("Pick a Workspaces folder and name your first workspace first.")
		imgui.end_popup()

	def _finish_setup(self):
		name = self._setup_workspace_name.strip()
		if name in self.workspace_names():
			self._setup_error = "A workspace with this name already exists."
			return
		create_workspace(self._settings.workspaces_root, name)
		self.set_active_workspace(name)  # also saves workspaces_root, via _save()

		fresh = app_settings.load()
		fresh.dpi_scale = self._setup_dpi_scale
		app_settings.save(fresh)
		self._settings = fresh

		# Closed regardless of on_setup_finished being wired -- the normal
		# path (relaunch(), an os.execv that replaces the process) never
		# actually reaches the next frame for this to matter, but a host
		# app that didn't wire the callback would otherwise be left with
		# this popup stuck open forever despite being fully configured now.
		imgui.close_current_popup()
		if self.on_setup_finished is not None:
			self.on_setup_finished()

	def _poll_folder_dialog(self):
		if self._folder_dialog is None or not self._folder_dialog.ready(0):
			return
		result = self._folder_dialog.result()
		self._folder_dialog = None
		if not result:
			return
		self._settings.workspaces_root = result
		self._save()

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
