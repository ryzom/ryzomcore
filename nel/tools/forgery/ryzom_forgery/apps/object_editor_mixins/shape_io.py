"""ObjectEditorApp mixin: shape load/save, session state (restore-on-launch,
reopen-last-shape prompt), the ImportWatcher hooks and their conflict/status
popups, the unsaved-changes guard, transform baking, and shape writing.
Split out of object_editor.py, see the "Split object_editor.py into theme
files" chantier in project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

from datetime import datetime
from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui
from panda3d.core import Quat

from pynel.ryzom_shape import Quaternion, Vector3, ShapeParseError, ShapeWriteError, parse_shape, save_shape

from ryzom_forgery import settings as app_settings
from ryzom_forgery import virtual_categories
from ryzom_forgery.explorer import ExplorerItem
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.shape_geometry import IDENTITY_QUAT
from ryzom_forgery.workspaces import reveal_in_system_file_manager
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_colored_button, _icon_button, _CONFIRM_NO_COLOR, _CONFIRM_YES_COLOR, _MULTI_BITMAP_SLOT_LABELS,
)

_OVERWRITE_POPUP_ID = "Overwrite shape?"
_IMPORT_CONFLICT_POPUP_ID = "Shape open, about to auto-update"
_RESTORE_SCAN_POPUP_ID = "Scanning assets"
_REOPEN_SHAPE_POPUP_ID = "Reopen last shape?"
_LOAD_SHAPE_UNSAVED_POPUP_ID = "Unsaved changes"
_LOAD_SHAPE_UNSAVED_POPUP_MIN_WIDTH = 300.0

# _draw_import_conflict_popup()'s 4 choice buttons, color-coded by how
# "safe" each one is with the in-memory edits (green: nothing lost, orange:
# both kept but in 2 files, pink: current edits discarded, yellow: no-op).
_IMPORT_CONFLICT_SAVE_COLOR = (0.85, 0.55, 0.15, 1.0)  # orange
_IMPORT_CONFLICT_DISCARD_COLOR = (0.85, 0.35, 0.55, 1.0)  # pink
_IMPORT_CONFLICT_BACKUP_COLOR = (0.35, 0.75, 0.35, 1.0)  # green
_IMPORT_CONFLICT_CANCEL_COLOR = (0.8, 0.75, 0.15, 1.0)  # yellow

# ImportWatcher outcome messages on the sysinfo status bar (see
# _flush_pending_import_status()) -- same red as self.shape_error elsewhere
# in this file, same green as _IMPORT_CONFLICT_BACKUP_COLOR above.
_IMPORT_STATUS_ERROR_COLOR = (1.0, 0.4, 0.4, 1.0)  # red
_IMPORT_STATUS_SUCCESS_COLOR = (0.35, 0.75, 0.35, 1.0)  # green

# _draw_name_conflict_popup(): the workspace watcher's shared duplicate-
# name guard (see duplicate_name_guard.py) fires on_name_conflict() for 3
# independent groups (import_watcher/tex_dds_sync/workspace_sync), each
# flattening its matched files to a name-only output -- this one popup
# handles all 3, the message just varies by group.
_NAME_CONFLICT_POPUP_ID = "Duplicate file name"
_NAME_CONFLICT_MESSAGES = {
	"imports": "These two source meshes would both export to the same shapes/<name>.shape file:",
	"textures": "These two textures would both convert to the same build/dds/<name>.dds file:",
	"sync": "These two files would both sync to the same external file name:",
}


def _center_next_widget(width):
	"""Shifts the cursor so a `width`-wide widget drawn right after this call
	lands horizontally centered in the current window's content region --
	e.g. _draw_import_conflict_popup()'s buttons, each a different width
	(auto-sized to its own label) but all centered under each other. A no-op
	if `width` is already at least as wide as the available space."""
	avail = imgui.get_content_region_avail().x
	if avail > width:
		imgui.set_cursor_pos_x(imgui.get_cursor_pos_x() + (avail - width) / 2)


class ShapeIOMixin:
	def _is_shape_open_at(self, target_path):
		"""ImportWatcher's `is_shape_open` hook -- runs off its own background
		thread, but only ever reads a couple of plain attributes (no
		imgui/Panda3D calls), same as other cross-thread reads in this app."""
		return self.shape_file is not None and self._workspace_shape_save_path() == target_path

	def _on_open_shape_conflict(self, source_path, target_path):
		"""ImportWatcher's `on_open_shape_conflict` hook -- runs off its own
		background thread, so only queues state; _draw_import_conflict_popup()
		(called once per frame from draw_panel()) does the actual imgui/disk
		work on the main thread."""
		self._pending_import_conflict = (source_path, target_path)

	def _on_import_status(self, message, is_error):
		"""ImportWatcher's `on_status` hook -- runs off its own background
		thread, so only queues state; _flush_pending_import_status() (called
		once per frame from draw_panel()) does the actual imgui work on the
		main thread, same reasoning as _on_open_shape_conflict() above."""
		self._pending_import_status = (message, is_error)

	def _flush_pending_import_status(self):
		"""Surfaces the last ImportWatcher outcome (queued by
		_on_import_status()) on the sysinfo status bar -- red for a failure,
		green for a successful auto-export/update/backup-and-reexport, same
		colors as _IMPORT_CONFLICT_DISCARD_COLOR/_IMPORT_CONFLICT_BACKUP_COLOR
		use elsewhere in this file for the same "bad"/"safe" meaning."""
		if self._pending_import_status is None:
			return
		message, is_error = self._pending_import_status
		self._pending_import_status = None
		color = _IMPORT_STATUS_ERROR_COLOR if is_error else _IMPORT_STATUS_SUCCESS_COLOR
		self.sysinfo.set_status(message, color=color)

	def _on_name_conflict(self, group, path_a, path_b):
		"""on_name_conflict hook shared by import_watcher/tex_dds_sync/
		workspace_sync (see duplicate_name_guard.py) -- runs off whichever
		background thread detected it, so only queues state;
		_draw_name_conflict_popup() (called once per frame from
		draw_panel()) does the actual imgui/disk work on the main thread.
		Appended, not overwritten -- a single startup scan can surface
		several independent conflicts at once, each gets its own popup in
		turn."""
		self._pending_name_conflicts.append((group, path_a, path_b))

	def _apply_name_conflict_keep(self, keep_path, delete_path):
		"""Deletes `delete_path` -- the resulting real filesystem delete
		event settles through the normal watch, which hands the surviving
		`keep_path` back to whichever watcher was blocking on it (see
		DuplicateNameGuard.remove()), so no direct call into
		import_watcher/tex_dds_sync/workspace_sync is needed here."""
		try:
			delete_path.unlink()
		except OSError as exc:
			self.sysinfo.set_status(f"Could not delete {delete_path.name}: {exc}", color=_IMPORT_STATUS_ERROR_COLOR)

	def _apply_name_conflict_rename(self, path_a, path_b, name_a, name_b):
		"""Renames whichever of the two files actually got a different name
		typed into the popup (a field left unchanged is a no-op) -- the
		resulting real filesystem rename settles through the normal watch
		like any other rename, re-checking both the old (now-gone) and new
		name normally."""
		for path, new_name in ((path_a, name_a), (path_b, name_b)):
			new_name = new_name.strip()
			if not new_name or new_name == path.name:
				continue
			try:
				path.rename(path.with_name(new_name))
			except OSError as exc:
				self.sysinfo.set_status(f"Could not rename {path.name}: {exc}", color=_IMPORT_STATUS_ERROR_COLOR)

	def _draw_name_conflict_popup(self):
		"""Two different source files sharing the same output name (see
		_on_name_conflict()) -- neither is being processed
		(imported/converted/synced) by whichever watcher detected it until
		this is resolved here: keep one (deletes the other), or rename one
		or both (both kept). Handles the oldest still-pending conflict
		first; a conflict resolved from outside this popup (e.g. one file
		deleted by hand) while it's queued or open is simply dropped, no
		popup shown for it."""
		if not self._pending_name_conflicts:
			self._name_conflict_popup_opened = False
			return
		# None of these is nested under this popup -- any of them can be
		# open at the very same frames this one wants to open (the reopen/
		# restore-scan pair at startup; the import-conflict popup from a
		# live fs event racing with this one's own background-thread
		# trigger), and Dear ImGui doesn't cleanly support two
		# independently-opened top-level modals fighting over the popup
		# stack on the same frames (confirmed live, 2026-09-02: caused the
		# reopen-shape popup to flicker and become unclickable). Simplest
		# robust fix: this popup just waits its turn rather than trying to
		# coexist -- _pending_name_conflicts stays queued, nothing is lost.
		if (self._reopen_shape_prompt_open or self._restore_scan_popup_open
		        or self._pending_import_conflict is not None):
			return
		group, path_a, path_b = self._pending_name_conflicts[0]

		if not self._name_conflict_popup_opened:
			if not path_a.exists() or not path_b.exists():
				self._pending_name_conflicts.pop(0)
				return
			self._name_conflict_field_a = path_a.name
			self._name_conflict_field_b = path_b.name
			imgui.open_popup(_NAME_CONFLICT_POPUP_ID)
			self._name_conflict_popup_opened = True

		# always=True (not the default appearing-only): a conflict can be
		# detected by a background thread's startup scan (set_workspace_dir(),
		# called from __init__) before the very first ImGui frame, when
		# Panda3D's own window geometry can still be stale -- same reasoning
		# as _draw_reopen_shape_popup()'s own center_next_popup() call, see
		# center_next_popup()'s own docstring.
		center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_NAME_CONFLICT_POPUP_ID, None, flags)
		if not opened:
			return

		if not path_a.exists() or not path_b.exists():
			self._pending_name_conflicts.pop(0)
			self._name_conflict_popup_opened = False
			imgui.close_current_popup()
			imgui.end_popup()
			return

		imgui.text_wrapped(_NAME_CONFLICT_MESSAGES.get(group, "These two files share the same name:"))
		imgui.separator()

		for label, path, field_attr in (
			("1", path_a, "_name_conflict_field_a"), ("2", path_b, "_name_conflict_field_b")):
			imgui.text(f"File {label}:")
			imgui.same_line()
			if _icon_button(
				f"{fa_icons.ICON_FA_EYE}##name-conflict-reveal-{label}",
				f"Reveal {path} in the system file manager"):
				reveal_in_system_file_manager(path)
			imgui.set_next_item_width(300)
			_, new_value = imgui.input_text(f"##name-conflict-{label}", getattr(self, field_attr))
			setattr(self, field_attr, new_value)
			if imgui.is_item_hovered():
				imgui.set_tooltip(str(path))

		imgui.separator()
		can_rename = (
			self._name_conflict_field_a.strip() != "" and self._name_conflict_field_b.strip() != ""
			and self._name_conflict_field_a.strip().lower() != self._name_conflict_field_b.strip().lower())

		if imgui.button("Keep only file 1"):
			self._pending_name_conflicts.pop(0)
			self._name_conflict_popup_opened = False
			imgui.close_current_popup()
			self._apply_name_conflict_keep(path_a, path_b)
		imgui.same_line()
		if imgui.button("Keep only file 2"):
			self._pending_name_conflicts.pop(0)
			self._name_conflict_popup_opened = False
			imgui.close_current_popup()
			self._apply_name_conflict_keep(path_b, path_a)
		imgui.same_line()
		imgui.begin_disabled(not can_rename)
		if _colored_button("Rename", _CONFIRM_YES_COLOR):
			self._pending_name_conflicts.pop(0)
			self._name_conflict_popup_opened = False
			imgui.close_current_popup()
			self._apply_name_conflict_rename(
				path_a, path_b, self._name_conflict_field_a, self._name_conflict_field_b)
		imgui.end_disabled()

		imgui.end_popup()

	def _reload_shape_value_from_disk(self, target_path):
		"""Pulls target_path's just-rewritten geometry/materials back into the
		viewport without resetting editor-only state (material override
		colors, expanded panels, camera...) or re-seeding the pivot rotation
		-- same minimal-disruption approach as _replace_geometry(), since
		conceptually this *is* a replace (this exact shape, new geometry from
		an import), just triggered by the auto-export watcher instead of the
		manual Import -> Replace flow. Does strip the shape's own previous
		default_rot_quat back out of the live pivot, though -- see the
		comment below, same reasoning as _replace_geometry()."""
		old_base = getattr(self.shape_file.value, "base", None)
		old_rot = old_base.default_rot_quat if old_base is not None else IDENTITY_QUAT
		shape_file = parse_shape(target_path.read_bytes())
		self.shape_file.value = shape_file.value
		if old_rot != IDENTITY_QUAT:
			# Same reasoning as _replace_geometry(): the just-reloaded
			# geometry already has old_rot baked into its vertices (see
			# import_watcher.py's update_existing_shape()), so strip it back
			# out of the live pivot to avoid applying it twice.
			old_quat = Quat(old_rot.w, old_rot.x, old_rot.y, old_rot.z)
			self._object_pivot.set_quat(old_quat.conjugate() * self._object_pivot.get_quat())
		self._rebuild_geometry()

	def _has_unsaved_changes_at(self, target_path):
		"""True if the in-memory shape currently open differs from what's on
		disk at `target_path` -- lets _draw_import_conflict_popup() and
		_request_load_shape() only actually ask something when there's a
		real conflict, instead of on every single auto-update/load attempt.
		Rather than tracking every individual edit throughout the whole
		editor (a much bigger, more error-prone undertaking -- easy to miss
		a spot that mutates self.shape_file without going through it), bakes
		the current viewport transform (_bake_transform_into_shape(), same
		as a real save does -- otherwise a pending position/rotation/scale
		edit would compare equal and be missed) and serializes the result to
		a `target_path.name + '~'` backup file next to `target_path`, then
		compares that against a fresh read of `target_path` -- exact and
		always in sync with reality. The backup is deliberately left on disk
		(refreshed on every check) as a crash-recovery copy, not just a
		throwaway comparison buffer."""
		self._bake_transform_into_shape()
		backup_path = target_path.with_name(target_path.name + "~")
		try:
			save_shape(backup_path, self.shape_file)
		except (OSError, ShapeWriteError):
			return True
		try:
			backup_bytes = backup_path.read_bytes()
			on_disk = target_path.read_bytes()
		except OSError:
			return True
		return backup_bytes != on_disk

	def _apply_import_conflict_update(self, source_path, target_path):
		"""Common tail of every _draw_import_conflict_popup() choice: routes
		through ImportWatcher._update_existing_target() -- same update /
		material-count-mismatch-backup-and-reexport / error handling and
		sysinfo-bar reporting as the automatic (shape-not-open) path, so
		there's a single outcome-reporting story regardless of whether the
		target happened to be open in the viewport. Refreshes the viewport
		from disk only if target_path actually ended up rewritten."""
		if self.import_watcher._update_existing_target(source_path, target_path):
			self._reload_shape_value_from_disk(target_path)

	def _draw_import_conflict_popup(self):
		"""The shape currently open in the viewport is also the auto-export
		watcher's target for a just-changed import/ source file -- if the
		in-memory shape has actually diverged from what's on disk (see
		_has_unsaved_changes_at()), overwriting the file could silently
		discard that in-progress work, so this asks how to proceed instead of
		auto-updating (see the chantier discussion). When there's nothing
		unsaved, this behaves exactly like the "not currently open" case --
		no popup, straight to the update."""
		if self._pending_import_conflict is None:
			self._import_conflict_popup_opened = False
			return
		# Same reasoning as _draw_name_conflict_popup()'s own wait-your-turn
		# guard -- Dear ImGui doesn't cleanly support two independently-
		# opened top-level modals fighting over the popup stack on the same
		# frames (confirmed live, 2026-09-02: caused visible flicker).
		if (self._reopen_shape_prompt_open or self._restore_scan_popup_open
		        or self._pending_name_conflicts or self._name_conflict_popup_opened):
			return

		if not self._import_conflict_popup_opened:
			source_path, target_path = self._pending_import_conflict
			if not self._has_unsaved_changes_at(target_path):
				self._pending_import_conflict = None
				self._apply_import_conflict_update(source_path, target_path)
				return
			imgui.open_popup(_IMPORT_CONFLICT_POPUP_ID)
			self._import_conflict_popup_opened = True

		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_IMPORT_CONFLICT_POPUP_ID, None, flags)
		if not opened:
			return

		source_path, target_path = self._pending_import_conflict
		imgui.text(f"{target_path.name} is open in the viewport and about to be updated\n"
		           f"from the changed import {source_path.name}.")
		imgui.text_wrapped("This shape has unsaved changes in Patina -- choose how to proceed.")
		imgui.separator()

		save_label = f"Save this shape then import {source_path.name}"
		discard_label = f"Import {source_path.name} without saving"
		backup_label = "Save a copy of this shape"
		cancel_label = "Cancel (leave the on-disk file untouched)"
		frame_padding_x2 = imgui.get_style().frame_padding.x * 2

		_center_next_widget(imgui.calc_text_size(save_label).x + frame_padding_x2)
		if _colored_button(save_label, _IMPORT_CONFLICT_SAVE_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()
			self._write_shape(target_path)
			self._apply_import_conflict_update(source_path, target_path)
		_center_next_widget(imgui.calc_text_size(discard_label).x + frame_padding_x2)
		if _colored_button(discard_label, _IMPORT_CONFLICT_DISCARD_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()
			self._apply_import_conflict_update(source_path, target_path)
		_center_next_widget(imgui.calc_text_size(backup_label).x + frame_padding_x2)
		if _colored_button(backup_label, _IMPORT_CONFLICT_BACKUP_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()
			timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
			backup_path = target_path.with_name(f"{target_path.stem}_backup_{timestamp}{target_path.suffix}")
			self._write_shape(backup_path)
			self._apply_import_conflict_update(source_path, target_path)
		imgui.separator()
		_center_next_widget(imgui.calc_text_size(cancel_label).x + frame_padding_x2)
		if _colored_button(cancel_label, _IMPORT_CONFLICT_CANCEL_COLOR):
			self._pending_import_conflict = None
			imgui.close_current_popup()

		imgui.end_popup()

	def _on_load_command(self, items):
		if items:
			self._request_load_shape(items[0])

	def _request_load_shape(self, item):
		"""Routes every shape-load entry point through the unsaved-changes
		guard: loads `item` immediately unless the currently open shape is
		at risk of losing edits, in which case the load is queued and
		_draw_load_shape_unsaved_popup() asks first instead of silently
		discarding in-progress work. A shape loaded from a plain file
		(_shape_source_path set) is checked against that file directly; one
		loaded from inside a .bnp has no such file, so it's checked against
		wherever Save would actually write it instead
		(_workspace_shape_save_path()) -- if that resolves to nothing (no
		active workspace, or genuinely no copy anywhere in it yet), there's
		nothing to compare against and the edit is unconditionally at risk,
		same as _has_unsaved_changes_at() already treats a missing file."""
		unsaved = False
		if self.shape_file is not None:
			target_path = self._shape_source_path
			if target_path is None:
				target_path = self._workspace_shape_save_path()
			unsaved = target_path is None or self._has_unsaved_changes_at(target_path)
		if unsaved:
			self._pending_load_shape_item = item
			self._load_shape_unsaved_prompt_open = True
			return
		self._load_shape(item)

	def _draw_load_shape_unsaved_popup(self):
		"""Drawn every frame from draw_panel() while a shape load is queued
		behind the unsaved-changes guard (see _request_load_shape()) -- Yes
		discards the current shape's unsaved edits and loads the queued
		item, No cancels the load and keeps the current shape as-is."""
		if not self._load_shape_unsaved_prompt_open:
			return
		if not imgui.is_popup_open(_LOAD_SHAPE_UNSAVED_POPUP_ID):
			imgui.open_popup(_LOAD_SHAPE_UNSAVED_POPUP_ID)
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_LOAD_SHAPE_UNSAVED_POPUP_ID, None, flags)
		if opened:
			# An always_auto_resize modal sizes itself off the widest item
			# actually drawn; text_wrapped() alone wraps against whatever
			# (possibly tiny) width the window already has, which -- with
			# nothing else wide in this popup (unlike e.g.
			# _draw_import_conflict_popup()'s long button labels) --
			# collapses into a narrow one-word-per-line column. This
			# invisible dummy reserves a sane minimum width up front so the
			# text below has something real to wrap against.
			imgui.dummy((_LOAD_SHAPE_UNSAVED_POPUP_MIN_WIDTH, 0))
			imgui.text_wrapped(
				f"{self._shape_source_name} has unsaved changes. Loading "
				f"{self._pending_load_shape_item.name} will discard them.")
			if _colored_button("OK", _CONFIRM_YES_COLOR):
				item, self._pending_load_shape_item = self._pending_load_shape_item, None
				self._load_shape_unsaved_prompt_open = False
				imgui.close_current_popup()
				self._load_shape(item)
			imgui.same_line()
			if _colored_button("Cancel", _CONFIRM_NO_COLOR):
				self._pending_load_shape_item = None
				self._load_shape_unsaved_prompt_open = False
				imgui.close_current_popup()
			imgui.end_popup()

	def _load_shape(self, item):
		self._reset_shape_state()
		self._shape_source_path = item.path if item.bnp_path is None else None
		self._shape_source_bnp_path = item.bnp_path
		self._shape_source_name = item.name
		if self._shape_source_path is not None:
			self._texture_search_dirs = [self._shape_source_path.parent]

		try:
			shape_file = parse_shape(item.read_bytes())
		except ShapeParseError as exc:
			self.shape_file = None
			self.shape_error = str(exc)
			return

		self._display_shape(shape_file)

	def _save_session_state(self):
		"""Persists where the Explorer is browsing and which shape is
		loaded (see settings.py's last_folder/last_bnp/last_shape_* fields)
		so the next launch can pick up right where this one left off --
		called right before quitting (see _draw_bottom_bar()'s Quit
		button). Best-effort: this is a nice-to-have, not worth failing
		Quit over."""
		try:
			fresh = app_settings.load()
			fresh.last_folder = str(self.explorer.root)
			fresh.last_bnp = str(self.explorer._current_bnp) if self.explorer._current_bnp is not None else None
			shape_path = self._shape_source_bnp_path if self._shape_source_bnp_path is not None else self._shape_source_path
			fresh.last_shape_path = str(shape_path) if shape_path is not None else None
			fresh.last_shape_bnp = str(self._shape_source_bnp_path) if self._shape_source_bnp_path is not None else None
			fresh.last_shape_name = self._shape_source_name
			app_settings.save(fresh)
		except OSError as exc:
			print(f"[object_editor] could not save session state: {exc}")

	def _restore_session_state(self):
		"""Reopens whatever folder the Explorer was browsing at the end of
		the previous session (see _save_session_state()) -- called once
		from __init__(), after self.explorer/search_paths_dialog are set
		up. Best-effort: a folder that's since moved or vanished is
		silently skipped, same as active_workspace_path()'s own handling
		of a vanished workspace, not worth an error dialog over.

		The previously-loaded shape itself is no longer auto-reopened here
		-- only queued as a pending Yes/No prompt (see
		_draw_reopen_shape_popup()/_load_pending_reopen_shape()), asked
		once the first frame is up."""
		settings = app_settings.load()
		if settings.last_folder and Path(settings.last_folder).is_dir():
			self.explorer._navigate_to(Path(settings.last_folder))
			if settings.last_bnp and Path(settings.last_bnp).is_file():
				self.explorer._enter_bnp(Path(settings.last_bnp))

		if not (settings.last_shape_name and settings.last_shape_path):
			return
		if settings.last_shape_bnp:
			bnp_path = Path(settings.last_shape_bnp)
			if bnp_path.is_file():
				self._pending_reopen_shape_item = ExplorerItem(
					path=bnp_path, name=settings.last_shape_name, bnp_path=bnp_path)
		else:
			shape_path = Path(settings.last_shape_path)
			if shape_path.is_file():
				self._pending_reopen_shape_item = ExplorerItem(path=shape_path, name=settings.last_shape_name)

		if self._pending_reopen_shape_item is not None:
			self._reopen_shape_prompt_open = True

	def _load_pending_reopen_shape(self):
		"""Actually loads self._pending_reopen_shape_item (see
		_restore_session_state()) -- called once the user answers "Yes" to
		_draw_reopen_shape_popup()."""
		self._load_shape(self._pending_reopen_shape_item)

		# The shape displays right away, but its textures are resolved via
		# self.search_paths_dialog.find_texture() -- backed by the index
		# ensure_scanned() (called just before _restore_session_state() in
		# __init__) built. has_scanned_data (not just scanning) is what
		# actually matters here: a cache-hit startup already published a
		# real (if possibly slightly stale) index synchronously, so the
		# still-running background refresh scan has nothing left for this
		# popup to wait on. Only a genuinely cold start (no cache yet,
		# index still empty) needs to show the shape untextured with a
		# popup saying so, and refresh every material once that first scan
		# completes (see _draw_restore_scan_popup())."""
		if self.shape_file is not None and self.search_paths_dialog.scanning \
				and not self.search_paths_dialog.has_scanned_data:
			self._restore_scan_popup_open = True

	def _draw_reopen_shape_popup(self):
		"""Drawn every frame from draw_panel() while a previous session's
		shape is queued for a reopen decision (see
		_restore_session_state()) -- asks Yes/No instead of the old
		auto-reopen, naming the shape. No leaves nothing loaded; the
		pending item is discarded either way once answered (last_shape_*
		itself is untouched in settings.toml, so it's offered again next
		launch same as this one)."""
		if not self._reopen_shape_prompt_open:
			return
		if not imgui.is_popup_open(_REOPEN_SHAPE_POPUP_ID):
			imgui.open_popup(_REOPEN_SHAPE_POPUP_ID)
		# always=True: this can open on the very first ImGui frame (queued
		# from __init__ via _restore_session_state()), before Panda3D's
		# requested window geometry has actually settled -- see
		# center_next_popup()'s own docstring.
		center_next_popup(always=True)
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_REOPEN_SHAPE_POPUP_ID, None, flags)
		if opened:
			imgui.text(f"Reopen \"{self._pending_reopen_shape_item.name}\" from last session?")
			if _colored_button("Yes", _CONFIRM_YES_COLOR):
				self._load_pending_reopen_shape()
				self._reopen_shape_prompt_open = False
				self._pending_reopen_shape_item = None
				imgui.close_current_popup()
			imgui.same_line()
			if _colored_button("No", _CONFIRM_NO_COLOR):
				self._reopen_shape_prompt_open = False
				self._pending_reopen_shape_item = None
				imgui.close_current_popup()
			imgui.end_popup()

	def _draw_restore_scan_popup(self):
		"""Drawn every frame from draw_panel() while a shape was just
		restored (see _restore_session_state()) before the search-path scan
		it needs for textures had finished -- closes itself and refreshes
		every material the moment the scan completes."""
		if not self._restore_scan_popup_open:
			return
		if not imgui.is_popup_open(_RESTORE_SCAN_POPUP_ID):
			imgui.open_popup(_RESTORE_SCAN_POPUP_ID)
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_RESTORE_SCAN_POPUP_ID, None, flags)
		if opened:
			imgui.text("Scanning assets for textures...")
			if not self.search_paths_dialog.scanning:
				# load_panda_texture() unconditionally caches its result, a
				# miss (None) included -- the very first resolution attempt
				# (right after the shape was restored, well before the scan
				# populated the index) would otherwise poison every texture
				# name as "not found" forever, so _reapply_all_materials()
				# alone would just keep re-serving those same stale misses.
				self._texture_cache.clear()
				self._reapply_all_materials()
				imgui.close_current_popup()
				self._restore_scan_popup_open = False
			imgui.end_popup()

	def _reset_shape_state(self):
		"""Common reset before showing any shape in the viewer -- a `.shape`
		opened from the Explorer, or a mesh imported as a brand-new shape."""
		self.model_root.remove_node()
		self.model_root = self._object_pivot.attach_new_node("shape-root")
		self.shape_error = None
		self._material_node_paths = {}
		self._multi_bitmap_expanded = set()
		self._material_expanded = set()
		self._material_section_expanded = set()
		self._texture_browse_dialogs = {}
		self._material_override_colors = {}
		self._tex_transform_ui_state = {}
		self._panoply_selection = {}
		self._panoply_selection_defaulted = False
		self._shape_source_path = None
		self._shape_source_bnp_path = None  # set alongside _shape_source_path when the shape lives inside a .bnp -- see _save_session_state()
		self._shape_source_name = None
		self._texture_search_dirs = []
		self._texture_needs_repeat = False
		self._save_status = ""
		# Both keyed by texture *name* only, not by which shape/search_dirs
		# resolved it -- two different shapes can use the same texture name
		# for genuinely different files (different _texture_search_dirs
		# fallbacks) or the same file with different wrap-mode needs (see
		# _uvs_need_repeat()), which only gets applied the first time a name
		# is loaded (load_panda_texture()'s cache). Carrying either cache
		# over to a newly loaded shape risked reusing another shape's
		# resolved texture/wrap mode outright.
		self._texture_cache = {}
		self._cube_texture_cache = {}
		self._texture_freshness_mtimes = {}
		self._preview_texture_refs = {}
		# Same reasoning as _texture_cache just above -- also keyed by
		# resolved texture name only.
		self._panoply_texture_sources = {}
		self._panoply_texture_signatures = {}
		# Re-detected per shape in _display_shape() (via
		# _auto_detect_bind_slot()) -- a different shape's own slot never
		# applies to a newly loaded one, manual or auto.
		self._bind_slot_override = ""

	def _display_shape(self, shape_file):
		"""Renders an already-parsed/-built ShapeFile. Assumes
		_reset_shape_state() was already called (separate so a
		replace-geometry flow can skip resetting the editing state that's
		meant to survive it, e.g. material overrides)."""
		self.shape_file = shape_file

		# CMeshBase::DefaultPos/DefaultRotQuat/DefaultScale are what the
		# engine actually places/rotates/scales the object by at instance
		# creation (nel/src/3d/mesh_base.cpp:instanciateMeshBase(), verified
		# 2026-08-30 against the real client's own attach-point path too:
		# entity_cl.cpp's SInstanceCL::createLoading() creates the instance
		# -- Pos/RotQuat/Scale already applied at that point -- *then*
		# stickObject() parents it to the bone, with nothing in between
		# that resets them) -- not just an editor default. Seeding
		# _object_pivot with all three up front, on a fresh load, means the
		# shape (and the navcube gizmo's inner cube, which mirrors
		# _object_pivot) already shows its real in-game placement instead
		# of looking wrong until you happen to notice these fields in the
		# All Properties tab -- previously only rotation was seeded (Nuno,
		# 2026-08-31: needed for authoring an attach-point weapon's own
		# grip-point offset, e.g. after rescaling a shared mesh reused
		# across several items). A geometry-only replace (_replace_geometry(),
		# which calls _rebuild_geometry() directly and never this method)
		# deliberately does NOT re-seed -- it must preserve whatever the
		# user already set up via Ctrl+drag/the Transform panel.
		base = getattr(shape_file.value, "base", None)
		if base is not None:
			rot = base.default_rot_quat
			self._object_pivot.set_quat(Quat(rot.w, rot.x, rot.y, rot.z))
			pos = base.default_pos
			self._object_pivot.set_pos(pos.x, pos.y, pos.z)
			scale = base.default_scale
			self._object_pivot.set_scale(scale.x, scale.y, scale.z)

		# Baseline for the gizmo's Ctrl+Reset (see reset_object_rotation()) --
		# whatever the object's rotation is right after loading, until a save
		# makes the current Ctrl+drag rotation the new baseline instead.
		self._object_pivot_base_quat = Quat(self._object_pivot.get_quat())

		self._rebuild_geometry()
		self._auto_select_multi_bitmap_slot()
		self._auto_detect_bind_slot()

	def _auto_select_multi_bitmap_slot(self):
		"""If the shape's own stored Multi Bitmap selection (a CTextureMultiFile's
		_CurrSelectedTexture, faithfully preserved as-is by pynel -- see
		ryzom_shape.py) points at a slot that's empty for every material
		(common for creatures/props only exported with a subset of the
		quality/ecosystem/season variants filled in -- e.g. fo_carnitree.shape,
		whose slot 0 is empty across all 5 materials), the shape renders
		blank/white by default. Auto-switches to the first slot that actually
		has a texture in at least one material instead -- the real client
		picks its slot dynamically (graphics quality/ecosystem/season) rather
		than trusting whatever was last saved as "current", so there's no
		single "correct" slot to fall back to anyway; this just finds
		*something* to show rather than nothing."""
		entries = self._multi_bitmap_entries()
		if not entries:
			return
		representative = entries[0][1]
		current_index = representative.selected_index
		if current_index is not None and any(
				current_index < len(texture.file_names) and texture.file_names[current_index]
				for _material_id, texture in entries):
			return  # already resolves to something for at least one material

		slot_count = max(len(_MULTI_BITMAP_SLOT_LABELS), max(len(t.file_names) for _, t in entries))
		for index in range(slot_count):
			if any(index < len(texture.file_names) and texture.file_names[index] for _material_id, texture in entries):
				self._select_multi_bitmap_slot(entries, index)
				return

	def _workspace_shape_save_path(self):
		"""Where Save writes to -- overwrites wherever a file named
		self._shape_source_name already exists anywhere in the active
		workspace (any real nesting, exclusion rules respected -- see
		virtual_categories.find_existing_file()), regardless of where the
		shape was originally loaded from (even from inside a .bnp, which
		used to disable Save entirely) or which real subfolder it actually
		lives in; falls back to <active workspace>/shapes/<name> only for a
		genuinely new asset with no existing match anywhere. None if
		there's no active workspace configured yet, or no shape name is
		known."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None or not self._shape_source_name:
			return None
		exclusion_rules = app_settings.load().exclusion_rules
		existing = virtual_categories.find_existing_file(workspace_dir, self._shape_source_name, exclusion_rules)
		return existing if existing is not None else workspace_dir / "shapes" / self._shape_source_name

	def _bake_transform_into_shape(self):
		"""Whatever position/rotation/scale the object is currently at
		becomes the shape's own base.default_pos/default_rot_quat/
		default_scale: this is meant to be an authoring tool for those
		values, not just a live-viewer aid, so they must survive a
		save/reload round trip rather than silently reverting. Shared by
		_write_shape() (a real save) and _has_unsaved_changes_at() (which
		needs the same baking before comparing, or a pending transform-only
		edit reads as unsaved=False -- see the chantier discussion)."""
		base = getattr(self.shape_file.value, "base", None)
		if base is None:
			return
		# Read model_root's WORLD rotation (not just _object_pivot's own) --
		# _transform_node("rotation") lets the Rotation panel edit either
		# node depending on that row's pivot lock, and model_root's world
		# quat always reflects the total either way (it's pivot's rotation
		# composed with model_root's own local one, identity when unlocked).
		total_quat = self.model_root.get_quat(self.render)
		# Panda3D's LQuaternion stores (real, i, j, k) internally, and its
		# inherited get_x/y/z/w() accessors read that raw slot order rather
		# than remapping to (i, j, k, real) -- confirmed empirically (an
		# identity quat's get_x() came back 1.0, the real part, not 0.0) and
		# consistent with how _display_shape() already builds a Panda Quat
		# via Quat(rot.w, rot.x, rot.y, rot.z) (real first, positionally)
		# elsewhere in this file.
		base.default_rot_quat = Quaternion(
			x=total_quat.get_y(), y=total_quat.get_z(), z=total_quat.get_w(), w=total_quat.get_x())
		# Same reasoning as rotation just above: position/scale edits can
		# land on either _object_pivot or model_root depending on that row's
		# pivot lock (_transform_node()), so the WORLD pos/scale relative to
		# render is what actually reflects the total edit either way.
		# Verified 2026-08-30 these fields are genuinely used by the real
		# client for attach-point placement (entity_cl.cpp: the instance's
		# Default* already applied at creation survive stickObject()
		# unchanged) -- not editor-only, unlike DefaultPivot, which stays
		# untouched here (rotation/scale CENTER, not the object's own
		# placement -- see mesh_base.cpp:353's separate setPivot() call).
		world_pos = self.model_root.get_pos(self.render)
		base.default_pos = Vector3(x=world_pos.x, y=world_pos.y, z=world_pos.z)
		world_scale = self.model_root.get_scale(self.render)
		base.default_scale = Vector3(x=world_scale.x, y=world_scale.y, z=world_scale.z)

	def _write_shape(self, path):
		try:
			self._bake_transform_into_shape()
			path.parent.mkdir(parents=True, exist_ok=True)
			save_shape(path, self.shape_file)
			self._save_bind_slot_override()
			self._save_status = f"Saved to {path}"
			print(f"[object_editor] {self._save_status}")
			# The pivot's rotation/position/scale are now baked into
			# base.default_* above, so they're also the natural new reset
			# baseline -- otherwise Ctrl+Reset would keep snapping back to
			# the pre-save values forever.
			self._object_pivot_base_quat = Quat(self._object_pivot.get_quat())
		except (OSError, ShapeWriteError) as exc:
			self._save_status = f"Save failed: {exc}"
			print(f"[object_editor] {self._save_status}")

	def _on_save_clicked(self):
		save_path = self._workspace_shape_save_path()
		if save_path is None:
			return
		if self._save_overwrite_confirmed:
			self._write_shape(save_path)
		else:
			self._pending_save_path = save_path
			self._confirm_overwrite_open = True
			imgui.open_popup(_OVERWRITE_POPUP_ID)

	def _draw_save_confirmation_popup(self):
		if not self._confirm_overwrite_open:
			return

		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_OVERWRITE_POPUP_ID, None, flags)
		if not opened:
			return

		imgui.text(f"Overwrite this file?\n{self._pending_save_path}")
		imgui.text_wrapped("You won't be asked again this session.")
		imgui.separator()
		if _colored_button("Overwrite", _CONFIRM_YES_COLOR):
			self._save_overwrite_confirmed = True
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
			self._write_shape(self._pending_save_path)
		imgui.same_line()
		if _colored_button("Cancel", _CONFIRM_NO_COLOR):
			self._confirm_overwrite_open = False
			imgui.close_current_popup()
		imgui.end_popup()
