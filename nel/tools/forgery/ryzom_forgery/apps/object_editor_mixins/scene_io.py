"""ObjectEditorApp mixin: the scene's link to the shape being edited, and its
persistence -- which scene entry is the shape being edited (the one the whole
editor works on, self.shape_file), keeping the edits of the entries left
behind in memory, "modified" tracking per shape with the Save/Discard prompt
shown on quit, and saving/loading scenes (`<workspace>/scenes/*.scene`, JSON,
see ryzom_forgery/scene.py) -- including `default.scene`, always written when
Patina quits and offered for reload at the next launch.

The scene also remembers the viewer's own settings (camera, open panels,
lights, wind, viewport toggles, bind creature) in its `ui` dict, so loading
one brings the whole session back, like a .blend file.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui
from panda3d.core import Point3

from pynel.ryzom_bnp import BnpError, BnpReader
from pynel.ryzom_shape import ShapeWriteError, dumps, save_shape

from ryzom_forgery import scene as scene_model
from ryzom_forgery.explorer import ExplorerItem
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.settings import PanelState
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_colored_button, _icon_button, _CONFIRM_NO_COLOR, _CONFIRM_YES_COLOR,
)

_UNSAVED_POPUP_ID = "Unsaved shapes"
_UNSAVED_POPUP_MIN_WIDTH = 320.0
_SCENE_LOAD_POPUP_ID = "##scene-load-popup"
_SCENE_SAVE_POPUP_ID = "##scene-save-popup"
_OVERWRITE_COLOR = (0.95, 0.55, 0.15, 1.0)

_PANEL_ATTRS = {
	"wind": "_wind_panel",
	"bone_preview": "_bone_preview_panel",
	"bind": "_bind_panel",
	"light": "_light_panel",
	"info": "_info_panel",
}


def _bnp_has(bnp_path, name):
	try:
		BnpReader(bnp_path).find(name)
	except (BnpError, OSError):
		return False
	return True


class SceneIOMixin:
	# ------------------------------------------------------------------
	# Which entry is the shape being edited, and what "modified" means
	# ------------------------------------------------------------------

	def _capture_shape_baseline(self):
		"""Remembers how the shape being edited serializes right now (after
		its pivot was baked into it, exactly as a Save would) -- what
		_edited_shape_modified() compares against. Called after a load and
		after a successful Save. None when this shape type can't be written
		at all (nothing to compare, nothing to save)."""
		self._shape_baseline_bytes = None
		if self.shape_file is None:
			return
		try:
			self._bake_transform_into_shape()
			self._shape_baseline_bytes = dumps(self.shape_file)
		except ShapeWriteError:
			pass

	def _edited_shape_modified(self):
		"""True if the shape being edited differs from what it was when it
		was loaded/last saved -- including a pending position/rotation/scale
		edit, which lives on the pivot until a save bakes it into the shape.
		A brand-new imported mesh (no file behind it) is always modified."""
		if self.shape_file is None or self._shape_baseline_bytes is None:
			return False
		if not self.model_root.get_transform().is_identity():
			return True  # a pivot-locked edit, baked into the vertices only at save time
		try:
			self._bake_transform_into_shape()
			return dumps(self.shape_file) != self._shape_baseline_bytes
		except ShapeWriteError:
			return False

	def _current_shape_item(self):
		"""The ExplorerItem the shape being edited was loaded from, or None
		for a shape with no file behind it (an imported mesh)."""
		if not self._shape_source_name:
			return None
		if self._shape_source_bnp_path is not None:
			return ExplorerItem(path=self._shape_source_bnp_path, name=self._shape_source_name,
			                    bnp_path=self._shape_source_bnp_path)
		if self._shape_source_path is not None:
			return ExplorerItem(path=self._shape_source_path, name=self._shape_source_name)
		return None

	def _register_edited_scene_entry(self, item):
		"""Called once a shape was loaded in the editor (an Explorer click, a
		reload...): it becomes the scene's edited entry -- replacing what the
		previous edited entry pointed to (loading from the Explorer swaps the
		shape being edited, it does not pile shapes up), or creating the
		entry when nothing was being edited yet. Another entry already
		pointing to the same file is dropped (it is this one now)."""
		source = self._scene_source_for_item(item)
		for other in list(self._scene.entries):
			if other.source == source and other.id != self._scene.edited_id:
				self._scene.remove(other.id)
				self._scene_shapes.pop(other.id, None)
				self._scene_dirty.pop(other.id, None)
		entry = self._scene.entry(self._scene.edited_id)
		if entry is None:
			entry = scene_model.SceneEntry(id=self._new_scene_entry_id("shape"), source=source, kept=False)
			self._scene.add(entry)
			self._scene.edited_id = entry.id
		else:
			entry.source = source
		self._scene_shapes[entry.id] = self.shape_file
		self._scene_dirty.pop(entry.id, None)
		# The bind of the shape just displayed (slot auto-detected while it
		# loaded, before this entry existed) belongs to its entry too.
		entry.bind_slot = self._bind_slot_override
		entry.bind_attach_point = self._bind_attach_point
		self._refresh_scene_shapes()

	def _prepare_replace_edited_shape(self, item):
		"""Called before an Explorer load: when the shape being edited was
		kept in the scene (added with [+], or selected from the bar) and the
		file being loaded is another one, it stays in the scene -- its
		unsaved edits kept in memory -- instead of being replaced; the
		loaded file then gets an entry of its own. An entry that was not
		kept is replaced in place (see _register_edited_scene_entry())."""
		entry = self._scene.entry(self._scene.edited_id)
		if entry is None or not entry.kept:
			return
		if entry.source == self._scene_source_for_item(item):
			return  # reloading the very same file: nothing to keep
		self._stash_edited_shape()
		self._scene.edited_id = ""

	def _edited_shape_replacement_loses_edits(self, item):
		"""True if loading `item` from the Explorer would throw away unsaved
		edits: the shape being edited is modified and either was not kept in
		the scene, or is the very file being reloaded."""
		if self.shape_file is None or not self._edited_shape_modified():
			return False
		entry = self._scene.entry(self._scene.edited_id)
		return entry is None or not entry.kept or entry.source == self._scene_source_for_item(item)

	def _keep_edited_scene_shape(self):
		"""[+] > "Add the current shape": the shape being edited stays in the
		scene when another one is loaded from the Explorer."""
		entry = self._scene.entry(self._scene.edited_id)
		if entry is not None:
			entry.kept = True

	def _detach_edited_scene_entry(self):
		"""The shape on screen no longer is what the edited entry points to
		(a new mesh was imported over it): the entry leaves the scene."""
		entry = self._scene.entry(self._scene.edited_id)
		if entry is not None:
			self._scene.remove(entry.id)
			self._scene_shapes.pop(entry.id, None)
			self._scene_dirty.pop(entry.id, None)
		self._scene.edited_id = ""
		self._shape_baseline_bytes = b""  # no file behind it: modified until saved
		self._refresh_scene_shapes()

	def _stash_edited_shape(self):
		"""Before another entry becomes the edited one: a shape with unsaved
		changes is kept as-is in memory (its pivot and locked edits baked in,
		as a save would) and stays in the scene, drawn from that in-memory
		copy; an unmodified one is simply forgotten and re-read from disk
		next time."""
		entry = self._scene.entry(self._scene.edited_id)
		if entry is None or self.shape_file is None:
			return
		if not self._edited_shape_modified():
			self._scene_shapes.pop(entry.id, None)
			return
		self._bake_transform_into_shape()
		self._bake_locked_edit_into_vertices()
		self._scene_shapes[entry.id] = self.shape_file
		self._scene_dirty[entry.id] = {
			"shape_file": self.shape_file, "baseline": self._shape_baseline_bytes,
			"path": self._shape_source_path, "bnp": self._shape_source_bnp_path,
			"name": self._shape_source_name, "search_dirs": list(self._texture_search_dirs),
		}

	def _select_edited_scene_shape(self, entry):
		"""Makes `entry` (an imported entry of the scene) the shape being
		edited, from the top-left bar."""
		if entry.id == self._scene.edited_id or not entry.editable:
			return
		item = self._scene_item_for_entry(entry)
		stash = self._scene_dirty.pop(entry.id, None)
		if item is None and stash is None:
			print(f"[object_editor] scene shape {entry.id!r} no longer exists, removed from the scene")
			self._remove_scene_shape(entry)
			return
		# Leaving the temp shape for a shape of the scene: it disappears if it
		# was not modified, otherwise it is kept in the scene (its edits stay
		# in memory) rather than lost.
		leaving = self._scene.entry(self._scene.edited_id)
		drop_leaving = False
		if leaving is not None and not leaving.kept:
			if self._edited_shape_modified():
				leaving.kept = True
			else:
				drop_leaving = True
		self._stash_edited_shape()
		stored_slot, stored_attach_point = entry.bind_slot, entry.bind_attach_point
		self._scene.edited_id = entry.id
		if stash is not None:
			self._reset_shape_state()
			self._shape_source_path = stash["path"]
			self._shape_source_bnp_path = stash["bnp"]
			self._shape_source_name = stash["name"]
			self._texture_search_dirs = list(stash["search_dirs"])
			self._display_shape(stash["shape_file"])
			self._shape_baseline_bytes = stash["baseline"]
		else:
			self._scene_shapes.pop(entry.id, None)
			self._load_shape(item, register=False)
		if self.shape_file is None:
			self._scene.edited_id = ""
			self._remove_scene_shape(entry)
			return
		self._scene_shapes[entry.id] = self.shape_file
		if drop_leaving:
			self._scene.remove(leaving.id)
			self._scene_shapes.pop(leaving.id, None)
		if stored_slot:
			self._bind_slot_override = stored_slot
		self._bind_attach_point = stored_attach_point
		self._apply_loaded_shape_to_creature()

	# ------------------------------------------------------------------
	# Unsaved shapes prompt (quit, loading another scene)
	# ------------------------------------------------------------------

	def _collect_unsaved_shapes(self):
		items = []
		if self._edited_shape_modified():
			items.append({"name": self._shape_source_name or "(new shape)", "entry_id": self._scene.edited_id})
		for entry_id, stash in self._scene_dirty.items():
			items.append({"name": stash["name"] or entry_id, "entry_id": entry_id})
		return items

	def _ask_unsaved_shapes_then(self, then):
		"""Runs `then` right away when nothing is modified, otherwise after
		the Save/Discard prompt was answered for every modified shape."""
		items = self._collect_unsaved_shapes()
		if not items:
			then()
			return
		self._scene_unsaved_items = items
		self._scene_unsaved_then = then
		self._scene_unsaved_prompt_open = True

	def request_exit(self):
		"""Quit button / window close: asks about the modified shapes first
		(ForgeryApp.request_exit() just exits)."""
		self._ask_unsaved_shapes_then(self.userExit)

	def _save_unsaved_item(self, item):
		"""Saves one modified shape into the active workspace (where Save
		writes) -- True if it was written."""
		entry_id = item["entry_id"]
		if entry_id == self._scene.edited_id:
			path = self._workspace_shape_save_path()
			if path is None:
				self._save_status = f"Cannot save {item['name']}: no active workspace"
				return False
			self._write_shape(path)
			return not self._edited_shape_modified()
		stash = self._scene_dirty.get(entry_id)
		if stash is None:
			return True
		path = self._workspace_shape_save_path_for(stash["name"])
		if path is None:
			self._save_status = f"Cannot save {item['name']}: no active workspace"
			return False
		try:
			path.parent.mkdir(parents=True, exist_ok=True)
			save_shape(path, stash["shape_file"])
		except (OSError, ShapeWriteError) as exc:
			self._save_status = f"Save failed: {exc}"
			print(f"[object_editor] {self._save_status}")
			return False
		print(f"[object_editor] Saved to {path}")
		self._scene_dirty.pop(entry_id, None)
		return True

	def _discard_unsaved_item(self, item):
		entry_id = item["entry_id"]
		if self._scene_dirty.pop(entry_id, None) is not None:
			self._scene_shapes.pop(entry_id, None)

	def _draw_scene_unsaved_popup(self):
		"""Drawn every frame from draw_panel() while the quit / scene-load
		guard is waiting for an answer: one row per modified shape, Save or
		Discard each; once none is left the guarded action goes on."""
		if not self._scene_unsaved_prompt_open:
			return
		if not imgui.is_popup_open(_UNSAVED_POPUP_ID):
			imgui.open_popup(_UNSAVED_POPUP_ID)
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_UNSAVED_POPUP_ID, None, flags)
		if not opened:
			return
		run_then = None
		imgui.dummy((_UNSAVED_POPUP_MIN_WIDTH, 0))
		imgui.text_wrapped("These shapes have unsaved changes:")
		for index, item in enumerate(list(self._scene_unsaved_items)):
			imgui.push_id(f"unsaved-{index}")
			imgui.text(item["name"])
			imgui.same_line()
			if _colored_button("Save", _CONFIRM_YES_COLOR):
				if self._save_unsaved_item(item):
					self._scene_unsaved_items.remove(item)
			imgui.same_line()
			if _colored_button("Discard", _CONFIRM_NO_COLOR):
				self._discard_unsaved_item(item)
				self._scene_unsaved_items.remove(item)
			imgui.pop_id()
		imgui.separator()
		if _colored_button("Cancel", _CONFIRM_NO_COLOR):
			self._scene_unsaved_items = []
			self._scene_unsaved_then = None
			self._scene_unsaved_prompt_open = False
			imgui.close_current_popup()
		elif not self._scene_unsaved_items:
			run_then, self._scene_unsaved_then = self._scene_unsaved_then, None
			self._scene_unsaved_prompt_open = False
			imgui.close_current_popup()
		if self._save_status:
			imgui.text_wrapped(self._save_status)
		imgui.end_popup()
		if run_then is not None:
			run_then()

	# ------------------------------------------------------------------
	# Scene files
	# ------------------------------------------------------------------

	def _collect_scene_ui(self):
		"""Everything about the viewer itself the scene brings back."""
		camera = self.orbit_camera
		ambient_intensity = (self._saved_ambient_light_intensity if self._shading_mode == "constant"
		                     and self._saved_ambient_light_intensity is not None else self._ambient_light_intensity)
		return {
			"camera": {
				"target": [camera.target.x, camera.target.y, camera.target.z],
				"distance": camera.distance, "heading": camera.heading, "pitch": camera.pitch,
			},
			"panels": {
				name: {"open": state.open, "x": state.x, "y": state.y}
				for name, state in self._collect_panel_states().items()
			},
			"lights": {
				"ambient_color": list(self._ambient_light_color), "ambient_intensity": ambient_intensity,
				"sun_color": list(self._sun_light_color), "sun_intensity": self._sun_light_intensity,
				"sun_heading": self._sun_heading_deg, "sun_pitch": self._sun_pitch_deg,
			},
			"wind": {
				"power": self._wind_power, "direction": self._wind_direction_deg, "animate": self._wind_animate,
			},
			"viewport": {
				"grid": self._grid_visible, "world_axes": self._world_axes_visible,
				"pivot_axes": self._pivot_axes_visible, "transparent": self._object_transparent,
				"wireframe": self._wireframe_mode, "shading": self._shading_mode,
				"unshaded_color": list(self._unshaded_color),
				"background_colors": [list(color) for color in self._viewport_bg_colors],
				"background_active": self._viewport_bg_active,
			},
			"bind": {
				"creature": self._bind_creature_name, "mode": self._bind_mode,
				"show": self._show_assembled_creature,
			},
		}

	def _apply_panel_states(self, states):
		for name, attr_prefix in _PANEL_ATTRS.items():
			state = states.get(name)
			if state is not None:
				setattr(self, f"{attr_prefix}_open", state.open)
				setattr(self, f"{attr_prefix}_pos", (state.x, state.y))

	def _apply_scene_ui(self, ui):
		"""Restores the viewer settings saved in a scene (see
		_collect_scene_ui()) -- every key optional, a malformed value is
		skipped rather than failing the whole load."""
		try:
			panels = ui.get("panels")
			if panels:
				self._apply_panel_states(
					{name: PanelState(open=bool(p["open"]), x=float(p["x"]), y=float(p["y"])) for name, p in panels.items()})
			lights = ui.get("lights")
			if lights:
				self._ambient_light_color = tuple(lights["ambient_color"])
				self._ambient_light_intensity = float(lights["ambient_intensity"])
				self._sun_light_color = tuple(lights["sun_color"])
				self._sun_light_intensity = float(lights["sun_intensity"])
				self._sun_heading_deg = float(lights["sun_heading"])
				self._sun_pitch_deg = float(lights["sun_pitch"])
				self._apply_light_settings()
			wind = ui.get("wind")
			if wind:
				self._wind_power = float(wind["power"])
				self._wind_direction_deg = float(wind["direction"])
				self._wind_animate = bool(wind["animate"])
			viewport = ui.get("viewport")
			if viewport:
				self._apply_scene_viewport_ui(viewport)
			bind = ui.get("bind")
			if bind:
				self._bind_mode = str(bind.get("mode", ""))
				creature = str(bind.get("creature", ""))
				if creature and creature in self._ensure_bind_creatures():
					self._select_bind_creature(creature)
					self._show_assembled_creature = bool(bind.get("show", True))
					self._rebuild_assembled_creature()
			# Last: loading the shape and rebuilding the creature both reframe the camera.
			camera = ui.get("camera")
			if camera:
				self.orbit_camera._anim = None
				self.orbit_camera.target = Point3(*camera["target"])
				self.orbit_camera.distance = float(camera["distance"])
				self.orbit_camera.heading = float(camera["heading"])
				self.orbit_camera.pitch = float(camera["pitch"])
				self.orbit_camera._update_camera_pos()  # the attributes alone don't move the real camera
		except (KeyError, TypeError, ValueError) as exc:
			print(f"[object_editor] could not fully restore the scene's viewer settings: {exc}")

	def _apply_scene_viewport_ui(self, viewport):
		self._grid_visible = bool(viewport["grid"])
		(self._grid_np.show if self._grid_visible else self._grid_np.hide)()
		self._world_axes_visible = bool(viewport["world_axes"])
		(self._world_axes_np.show if self._world_axes_visible else self._world_axes_np.hide)()
		self._pivot_axes_visible = bool(viewport["pivot_axes"])
		(self._pivot_axes_np.show if self._pivot_axes_visible else self._pivot_axes_np.hide)()
		self._object_transparent = bool(viewport["transparent"])
		self._apply_object_transparency()
		self._set_object_wireframe_mode(str(viewport["wireframe"]))
		self._unshaded_color = tuple(viewport["unshaded_color"])
		self._set_shading_mode(str(viewport["shading"]))
		self._viewport_bg_colors = [tuple(color) for color in viewport["background_colors"]]
		if viewport["background_active"] is not None:
			self._set_viewport_background_color(int(viewport["background_active"]))

	def _scene_snapshot(self):
		"""self._scene with its bind and viewer settings brought up to date."""
		entry = self._scene.entry(self._scene.edited_id)
		if entry is not None:
			entry.bind_slot = self._bind_slot_override
			entry.bind_attach_point = self._bind_attach_point
		self._scene.creature = self._bind_creature_name
		self._scene.ui = self._collect_scene_ui()
		return self._scene

	def _save_scene_file(self, file_name):
		"""Writes the scene as `<workspace>/scenes/<file_name>` -- True on
		success. No active workspace: nowhere to write."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return False
		try:
			path = scene_model.save_scene(self._scene_snapshot(), workspace_dir, file_name)
		except OSError as exc:
			self._save_status = f"Could not save the scene: {exc}"
			print(f"[object_editor] {self._save_status}")
			return False
		self._save_status = f"Scene saved to {path}"
		return True

	def _save_current_scene(self):
		"""At quit: the scene goes back into the file it was loaded from /
		last saved to (default.scene when it never had one) -- never into
		another one."""
		if self._pending_reload_scene is not None:
			return  # the startup question was never answered: nothing of this session to keep
		try:
			self._save_scene_file(self._scene_name)
		except Exception as exc:  # a nice-to-have at quit time, never worth blocking it
			print(f"[object_editor] could not save {self._scene_name}: {exc}")

	def _read_scene_file(self, path):
		return scene_model.load_scene(Path(path), self.workspace_setup_dialog.active_workspace_dir, _bnp_has)

	def _read_last_scene(self, file_name):
		"""The scene Patina was working on when it last quit (`file_name`,
		default.scene when unknown) as (scene, file name), or (None, "")."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return None, ""
		for name in (file_name, scene_model.DEFAULT_SCENE_NAME):
			if not name:
				continue
			path = scene_model.scenes_dir(workspace_dir) / name
			if path.is_file():
				return self._read_scene_file(path), name
		return None, ""

	def _on_load_scene_command(self, items):
		if items:
			self._request_load_scene(items[0].path)

	def _request_load_scene(self, path):
		scene = self._read_scene_file(path)
		if scene is None:
			self._save_status = f"Could not read the scene {Path(path).name}"
			print(f"[object_editor] {self._save_status}")
			return
		self._ask_unsaved_shapes_then(lambda: self._apply_loaded_scene(scene, Path(path).name))

	def _apply_loaded_scene(self, scene, file_name=scene_model.DEFAULT_SCENE_NAME):
		"""Replaces the whole scene by `scene` (shapes, binds, viewer
		settings): its edited shape is loaded in the editor, the other
		entries are drawn from their files. `file_name` is what the Save
		popup proposes afterwards."""
		self._scene_name = file_name
		edited_id, scene.edited_id = scene.edited_id, ""
		self._scene = scene
		self._scene_shapes = {}
		self._scene_dirty = {}
		self._bind_attach_point = ""
		edited = scene.entry(edited_id)
		item = self._scene_item_for_entry(edited) if edited is not None else None
		if item is not None:
			scene.edited_id = edited_id
			stored_slot, stored_attach_point = edited.bind_slot, edited.bind_attach_point
			self._load_shape(item, register=False)
			if self.shape_file is None:
				scene.remove(edited_id)
			else:
				self._scene_shapes[edited_id] = self.shape_file
				if stored_slot:
					self._bind_slot_override = stored_slot
				self._bind_attach_point = stored_attach_point
		elif edited is not None:
			scene.remove(edited_id)
		else:
			current = self._current_shape_item()
			if current is not None and self.shape_file is not None:
				self._register_edited_scene_entry(current)
		for entry in scene.entries:
			if entry.id != scene.edited_id:
				entry.visible = entry.visible and self._get_scene_shape(entry) is not None
		self._apply_loaded_shape_to_creature()
		self._apply_scene_ui(scene.ui)

	def _scene_file_names(self):
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return []
		folder = scene_model.scenes_dir(workspace_dir)
		if not folder.is_dir():
			return []
		return sorted(p.name for p in folder.glob("*" + scene_model.SCENE_EXTENSION))

	def _draw_scene_file_buttons(self, large_font):
		"""Load / Save-as buttons of the top-left scene bar, stacked under [+]."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Load a scene of this workspace", False,
		                square=True, large_font=large_font, disabled=workspace_dir is None):
			imgui.open_popup(_SCENE_LOAD_POPUP_ID)
		if imgui.begin_popup(_SCENE_LOAD_POPUP_ID):
			names = self._scene_file_names()
			if not names:
				imgui.text_disabled("(no scene saved in this workspace yet)")
			for name in names:
				clicked, _selected = imgui.selectable(name, False)
				if clicked and workspace_dir is not None:
					self._request_load_scene(scene_model.scenes_dir(workspace_dir) / name)
			imgui.end_popup()
		if _icon_button(fa_icons.ICON_FA_FLOPPY_DISK, "Save the scene as...", False,
		                square=True, large_font=large_font, disabled=workspace_dir is None):
			self._scene_name_input = Path(self._scene_name).stem
			imgui.open_popup(_SCENE_SAVE_POPUP_ID)
		if imgui.begin_popup(_SCENE_SAVE_POPUP_ID):
			imgui.set_next_item_width(220)
			_, self._scene_name_input = imgui.input_text("##scene-name", self._scene_name_input)
			# The extension is always .scene: only the name is typed.
			stem = self._scene_name_input.strip()
			file_name = stem + scene_model.SCENE_EXTENSION
			valid = bool(stem) and Path(stem).name == stem and stem not in (".", "..")
			# Saving over an existing scene needs a second click, the button
			# says so (and the confirmation is dropped as soon as the name changes).
			exists = valid and workspace_dir is not None \
				and (scene_model.scenes_dir(workspace_dir) / file_name).is_file()
			if not exists or self._scene_overwrite_confirm != file_name:
				self._scene_overwrite_confirm = ""
			if exists:
				imgui.text_colored(_OVERWRITE_COLOR, f"{file_name} already exists")
			imgui.begin_disabled(not valid)
			label = "Save"
			if exists:
				label = "Click again to overwrite" if self._scene_overwrite_confirm == file_name else "Overwrite..."
			if imgui.button(label):
				if exists and self._scene_overwrite_confirm != file_name:
					self._scene_overwrite_confirm = file_name
				elif self._save_scene_file(file_name):
					self._scene_name = file_name
					self._scene_overwrite_confirm = ""
					imgui.close_current_popup()
			imgui.end_disabled()
			imgui.end_popup()
