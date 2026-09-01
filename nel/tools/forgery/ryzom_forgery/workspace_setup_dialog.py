"""First-launch (and Settings-tab) UI for `settings.workspaces_root` -- the
single folder, shared by every Forgery app, that holds one subfolder per
*project*, itself holding one subfolder per editable *workspace* (see
workspaces.py and the forgery-workspace-projects chantier). Mirrors
export_dialog.py's non-blocking `portable_file_dialogs.select_folder`
polling pattern.
"""

from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd

from ryzom_forgery import settings as app_settings
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.workspaces import (
	active_workspace_path, create_project, create_workspace, ensure_structure, external_workspace_path,
	is_root_configured, list_external_workspaces, list_projects, list_workspaces, migrate_legacy_workspaces,
	add_external_workspace, open_in_system_file_manager, project_path, workspace_path,
)

_POPUP_ID = "Welcome to Ryzom Forgery"
_NEW_PROJECT_POPUP_ID = "New project"
_NEW_WORKSPACE_POPUP_ID = "New workspace"
_ELLIPSIS = "..."
_NEW_LABEL = "<new>"
_NEW_WORKSPACE_LABEL = "<New>"
_IMPORT_FOLDER_LABEL = "<Import Folder>"

_DEFAULT_PROJECT_NAME = "Ryzom"
_DEFAULT_WORKSPACE_NAME = "WIP"


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
		self._folder_dialog = None  # active portable_file_dialogs.select_folder, for the Workspaces root
		self._import_folder_dialog = None  # active portable_file_dialogs.select_folder, for <Import Folder>
		self._prompt_offered = False

		# Mandatory setup popup's own pending fields -- not written to
		# settings until "Finish"/"Migrate" is clicked (see
		# _draw_prompt_popup()). Pre-filled with sensible defaults rather
		# than empty strings requiring input just to reach "ready".
		self._setup_project_name = _DEFAULT_PROJECT_NAME
		self._setup_workspace_name = _DEFAULT_WORKSPACE_NAME
		self._setup_dpi_scale = self._settings.dpi_scale
		self._setup_error = ""

		self._new_project_pending = False  # same deferred-open pattern as _new_workspace_pending below
		self._new_project_name = ""
		self._new_project_error = ""

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
		# "Finish"/"Migrate" is clicked, after its settings are saved. A
		# relaunch is what this is normally wired to: needed for the DPI
		# value to fully apply (see app.py's set_live_ui_scale_preview()),
		# and gives a clean restart of workspace-dependent state (Explorer
		# virtual categories, search-path indexing) after a migration
		# physically moved folders out from under whatever was watching them.
		self.on_setup_finished = None

	def is_configured(self) -> bool:
		return is_root_configured(self._settings.workspaces_root)

	def _needs_migration(self) -> bool:
		"""True for a config that predates projects: workspaces_root and
		active_workspace already set (a first-time setup was completed
		before), but active_project is still None -- the marker (see
		Settings.active_project's own docstring in settings.py). Checked
		before _needs_setup() below, which would otherwise also be true for
		this same case and show the full (wrong) form instead of the
		shorter migration-only one."""
		return self.is_configured() and bool(self._settings.active_workspace) and not self._settings.active_project

	def _needs_setup(self) -> bool:
		"""True until a Workspaces folder, an active project, AND an active
		workspace are all set -- having no active project/workspace is no
		longer a valid steady state (see the mandatory setup popup below),
		so this is stricter than is_configured() (which only gates the
		combos on there being anything to list)."""
		return not (self.is_configured() and self._settings.active_project and self._settings.active_workspace)

	@property
	def active_project_name(self):
		return self._settings.active_project

	@property
	def active_workspace_name(self):
		return self._settings.active_workspace

	def project_names(self):
		if not self.is_configured():
			return []
		return list_projects(self._settings.workspaces_root)

	def workspace_names(self):
		"""Every workspace (internal and external) of the *active* project
		-- [] if no project is active yet."""
		if not self.is_configured() or not self._settings.active_project:
			return []
		project_dir = project_path(self._settings.workspaces_root, self._settings.active_project)
		internal = list_workspaces(self._settings.workspaces_root, self._settings.active_project)
		external = [name for name, _ in list_external_workspaces(project_dir)]
		return sorted(set(internal) | set(external), key=str.lower)

	@property
	def active_workspace_dir(self):
		return active_workspace_path(self._settings)

	def is_active_workspace_external(self) -> bool:
		"""True if the currently active workspace is registered in its
		project's external_workspaces.toml rather than being a real
		subfolder Forgery itself created -- callers use this to decide
		whether it's safe to backfill/restructure it (never, for an
		external one -- see workspaces.py's own module docstring)."""
		if not self._settings.active_project or not self._settings.active_workspace:
			return False
		project_dir = project_path(self._settings.workspaces_root, self._settings.active_project)
		return external_workspace_path(project_dir, self._settings.active_workspace) is not None

	def set_active_project(self, name):
		"""Switching projects clears the active workspace -- a workspace
		name from the previous project has no meaning in the new one (see
		draw()'s own self-heal, which picks a fresh one next frame if the
		new project has any)."""
		self._settings.active_project = name
		self._settings.active_workspace = None
		self._save()
		if self.on_active_workspace_changed is not None:
			self.on_active_workspace_changed(None)

	def set_active_workspace(self, name):
		self._settings.active_workspace = name
		self._save()
		if name is not None:
			project_dir = project_path(self._settings.workspaces_root, self._settings.active_project)
			# Never restructure/backfill an external workspace -- only an
			# internal one (created by Forgery itself, so relying on its
			# own standard SUBDIRS is safe) gets ensure_structure()'s
			# idempotent backfill of any SUBDIRS added since it was first
			# created.
			if external_workspace_path(project_dir, name) is None:
				ensure_structure(workspace_path(self._settings.workspaces_root, self._settings.active_project, name))
		if self.on_active_workspace_changed is not None:
			self.on_active_workspace_changed(self.active_workspace_dir)

	def _save(self):
		"""Same pattern as export_dialog.py's _save(): reload fresh and only
		overwrite our own fields, so other components' concurrent settings
		changes aren't clobbered by a stale in-memory copy."""
		fresh = app_settings.load()
		fresh.workspaces_root = self._settings.workspaces_root
		fresh.active_project = self._settings.active_project
		fresh.active_workspace = self._settings.active_workspace
		app_settings.save(fresh)
		self._settings = fresh

	def draw(self):
		"""Call once per frame from the host app (see app.py's draw_ui())."""
		self._poll_folder_dialog()
		self._poll_import_folder_dialog()

		# Self-heal, project level: a Workspaces folder with projects in it
		# (hand-edited settings.toml, or a project removed/renamed on disk)
		# but none active -- silently activate the first one instead of
		# forcing the mandatory setup popup below on someone who already
		# has projects set up. A genuinely pre-project (legacy) config
		# never reaches this: list_projects() is empty until migration
		# actually runs (see _needs_migration()), so this never fires for
		# that case.
		if self.is_configured() and not self._settings.active_project:
			projects = self.project_names()
			if projects:
				self.set_active_project(projects[0])

		# Self-heal, workspace level: same idea, scoped to the now-active
		# project.
		if self._settings.active_project and not self._settings.active_workspace:
			names = self.workspace_names()
			if names:
				self.set_active_workspace(names[0])

		if not self._prompt_offered and (self._needs_migration() or self._needs_setup()):
			self._prompt_offered = True
			imgui.open_popup(_POPUP_ID)

		# OpenPopup()/BeginPopupModal() both resolve their popup's actual ID
		# using the current window/ID-stack context at the time they're
		# called -- draw_active_workspace_row() (called from inside the
		# Explorer window, see object_editor.py's Explorer.extra_header)
		# only *requests* these two (self._new_project_pending/
		# self._new_workspace_pending) rather than calling open_popup()
		# itself, so the real calls happen here instead, at the same
		# top-level context _draw_new_project_popup()/
		# _draw_new_workspace_popup() call begin_popup_modal() from below.
		# Calling OpenPopup() from inside Explorer's window context while
		# BeginPopupModal() is called from here (no parent window)
		# previously computed two different IDs for the same string, so
		# the popup could never actually open.
		if self._new_project_pending:
			self._new_project_pending = False
			imgui.open_popup(_NEW_PROJECT_POPUP_ID)
		if self._new_workspace_pending:
			self._new_workspace_pending = False
			imgui.open_popup(_NEW_WORKSPACE_POPUP_ID)

		self._draw_prompt_popup()
		self._draw_new_project_popup()
		self._draw_new_workspace_popup()

	def draw_active_workspace_row(self, width=160):
		"""Two stacked rows -- "Project: [combo]" then "Workspace: [combo]
		[open in system file manager]" -- called from the host app's panel
		(see object_editor.py's draw_panel()), above the Save/Export/Quit
		bar. Both combos are disabled with an explanatory tooltip until a
		Workspaces root is configured (Workspace's also until a project is
		active), since there's nothing to list otherwise."""
		self._draw_project_row(width)
		self._draw_workspace_row(width)

	def _draw_project_row(self, width):
		names = self.project_names()
		current = self._settings.active_project if self._settings.active_project in names else None
		preview = current or "(none)"

		imgui.text("Project:")
		imgui.same_line()
		imgui.begin_disabled(not self.is_configured())
		imgui.set_next_item_width(width)
		if imgui.begin_combo("##project-selector", preview):
			for name in names:
				clicked, _ = imgui.selectable(name, name == current)
				if clicked:
					self.set_active_project(name)
			if names:
				imgui.separator()
			clicked, _ = imgui.selectable(_NEW_LABEL, False)
			if clicked:
				self._new_project_name = ""
				self._new_project_error = ""
				self._new_project_pending = True
			imgui.end_combo()
		imgui.end_disabled()
		if not self.is_configured() and imgui.is_item_hovered():
			imgui.set_tooltip("Set up a Workspaces folder first (Settings tab)")

	def _draw_workspace_row(self, width):
		names = self.workspace_names()
		current = self._settings.active_workspace if self._settings.active_workspace in names else None
		# Self-heal: the active workspace named in settings no longer
		# exists (internal or external) but others do -- having no active
		# workspace isn't a valid steady state anymore, so fall back to the
		# first one rather than showing "(none)".
		if current is None and names:
			self.set_active_workspace(names[0])
			current = names[0]
		preview = current or "(none)"
		has_project = bool(self._settings.active_project)

		imgui.text("Workspace:")
		imgui.same_line()
		imgui.begin_disabled(not has_project)
		imgui.set_next_item_width(width)
		if imgui.begin_combo("##workspace-selector", preview):
			for name in names:
				clicked, _ = imgui.selectable(name, name == current)
				if clicked:
					self.set_active_workspace(name)
			if names:
				imgui.separator()
			clicked, _ = imgui.selectable(_NEW_WORKSPACE_LABEL, False)
			if clicked:
				self._new_workspace_name = ""
				self._new_workspace_error = ""
				self._new_workspace_pending = True
			clicked, _ = imgui.selectable(_IMPORT_FOLDER_LABEL, False)
			if clicked:
				self._import_folder_dialog = pfd.select_folder("Import workspace folder", "")
			imgui.end_combo()
		imgui.end_disabled()
		if not has_project and imgui.is_item_hovered():
			imgui.set_tooltip("Pick or create a project first")

		imgui.same_line()
		active_path = self.active_workspace_dir
		imgui.begin_disabled(active_path is None)
		if _icon_button(fa_icons.ICON_FA_EYE, "Open workspace folder in system file manager"):
			open_in_system_file_manager(active_path)
		imgui.end_disabled()

	def _draw_new_project_popup(self):
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_NEW_PROJECT_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text("Project name:")
		imgui.set_next_item_width(220)
		_, self._new_project_name = imgui.input_text("##new-project-name", self._new_project_name)
		if self._new_project_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._new_project_error)

		if imgui.button("Create"):
			name = self._new_project_name.strip()
			if not name:
				self._new_project_error = "Name can't be empty."
			elif name in self.project_names():
				self._new_project_error = "A project with this name already exists."
			else:
				create_project(self._settings.workspaces_root, name)
				self.set_active_project(name)
				imgui.close_current_popup()
		imgui.same_line()
		if imgui.button("Cancel"):
			imgui.close_current_popup()
		imgui.end_popup()

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
				create_workspace(self._settings.workspaces_root, self._settings.active_project, name)
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
		folder, a project, and an active workspace are required for the app
		to be usable at all (shapes are only ever saved into a workspace,
		never elsewhere), so this stays open until "Finish"/"Migrate"
		actually completes it.

		Two different forms share this one popup, chosen once per opening
		(recomputed every frame it's open is fine too -- neither condition
		can flip while it's up, since nothing else can touch
		workspaces_root/active_project/active_workspace while this is the
		only interactive thing on screen):
		- _needs_migration(): a pre-project config -- just the project name
		  (folder and workspace already exist, only the project layer is
		  missing).
		- otherwise (_needs_setup()): a genuinely first-time setup -- DPI,
		  folder, project name, and first workspace name together."""
		# Only touch SetNextWindowPos while this popup is actually open --
		# this method runs every frame for the app's entire lifetime (not
		# gated behind an instance flag like the other popups below), so
		# calling center_next_popup() unconditionally left a pending
		# "next window position" unconsumed on every frame this popup
		# *isn't* open, which was interfering with the very next
		# Begin-family call that frame. always=True: can open on
		# essentially the very first ImGui frame, before Panda3D's
		# requested window geometry has actually settled -- see
		# center_next_popup()'s own docstring.
		if imgui.is_popup_open(_POPUP_ID):
			center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_POPUP_ID, None, flags)
		if not opened:
			return

		if self._needs_migration():
			self._draw_migration_form()
		else:
			self._draw_full_setup_form()
		imgui.end_popup()

	def _draw_migration_form(self):
		imgui.text_wrapped(
			"Ryzom Forgery workspaces now live inside a project. Pick a name for "
			"the project that will hold your existing workspace(s) -- everything "
			f"currently under \"{self._settings.workspaces_root}\" will be moved "
			"into it.")
		imgui.spacing()

		imgui.separator()
		imgui.text("Project name:")
		imgui.set_next_item_width(220)
		_, self._setup_project_name = imgui.input_text("##setup-project-name-migrate", self._setup_project_name)
		if self._setup_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._setup_error)
		imgui.separator()

		name = self._setup_project_name.strip()
		ready = bool(name) and name not in self.project_names()
		imgui.begin_disabled(not ready)
		if imgui.button("Migrate"):
			self._finish_migration(name)
		imgui.end_disabled()
		if not ready and imgui.is_item_hovered():
			imgui.set_tooltip("Name your project (must not already exist).")

	def _draw_full_setup_form(self):
		imgui.text_wrapped(
			"A couple of things to set up before you start.")
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
		imgui.text("Project name:")
		imgui.set_next_item_width(220)
		_, self._setup_project_name = imgui.input_text("##setup-project-name", self._setup_project_name)

		imgui.text("First workspace name:")
		imgui.set_next_item_width(220)
		_, self._setup_workspace_name = imgui.input_text("##setup-workspace-name", self._setup_workspace_name)
		if self._setup_error:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), self._setup_error)
		imgui.separator()

		ready = (is_root_configured(self._settings.workspaces_root)
		         and bool(self._setup_project_name.strip()) and bool(self._setup_workspace_name.strip()))
		imgui.begin_disabled(not ready)
		if imgui.button("Finish"):
			self._finish_setup()
		imgui.end_disabled()
		if not ready and imgui.is_item_hovered():
			imgui.set_tooltip("Pick a Workspaces folder, and name your first project and workspace.")

	def _finish_migration(self, project_name):
		migrate_legacy_workspaces(self._settings.workspaces_root, project_name)
		fresh = app_settings.load()
		fresh.active_project = project_name
		# active_workspace itself is untouched -- migrate_legacy_workspaces()
		# preserves every folder's own name, only its physical location
		# changes, so the name already in settings is still correct.
		app_settings.save(fresh)
		self._settings = fresh
		self._after_setup_finished()

	def _finish_setup(self):
		project_name = self._setup_project_name.strip()
		workspace_name = self._setup_workspace_name.strip()
		if project_name in self.project_names() and workspace_name in list_workspaces(
				self._settings.workspaces_root, project_name):
			self._setup_error = "A project/workspace with these names already exists."
			return
		create_project(self._settings.workspaces_root, project_name)
		create_workspace(self._settings.workspaces_root, project_name, workspace_name)

		fresh = app_settings.load()
		fresh.workspaces_root = self._settings.workspaces_root
		fresh.active_project = project_name
		fresh.active_workspace = workspace_name
		fresh.dpi_scale = self._setup_dpi_scale
		app_settings.save(fresh)
		self._settings = fresh
		self._after_setup_finished()

	def _after_setup_finished(self):
		"""Shared tail of _finish_setup()/_finish_migration() -- closed
		regardless of on_setup_finished being wired -- the normal path
		(relaunch(), an os.execv that replaces the process) never actually
		reaches the next frame for this to matter, but a host app that
		didn't wire the callback would otherwise be left with this popup
		stuck open forever despite being fully configured now."""
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

	def _poll_import_folder_dialog(self):
		if self._import_folder_dialog is None or not self._import_folder_dialog.ready(0):
			return
		result = self._import_folder_dialog.result()
		self._import_folder_dialog = None
		if not result or not self._settings.active_project:
			return
		self._register_external_workspace(Path(result))

	def _register_external_workspace(self, folder: Path):
		"""Registers `folder` (picked via <Import Folder>) as an external
		workspace of the active project, named after the folder's own
		basename -- deduplicated with a numeric suffix if that name is
		already taken (internal or external), rather than blocking on a
		separate naming prompt for what's meant to be a quick action."""
		project_dir = project_path(self._settings.workspaces_root, self._settings.active_project)
		existing = set(self.workspace_names())
		base_name = folder.name
		name = base_name
		suffix = 2
		while name in existing:
			name = f"{base_name} ({suffix})"
			suffix += 1
		add_external_workspace(project_dir, name, str(folder))
		self.set_active_workspace(name)

	def draw_settings_content(self):
		"""Embedded in the host app's Settings tab, same spot as
		export_dialog.py's draw_settings_content()."""
		label = "Workspaces: "
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
