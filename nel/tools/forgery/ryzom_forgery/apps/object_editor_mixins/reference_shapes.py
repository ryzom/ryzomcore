"""ObjectEditorApp mixin: the scene's extra shapes shown alongside whatever's
loaded -- the 3 toggleable scale-reference shapes (1x1x1 cube,
shortest/tallest playable character, for an at-a-glance sense of scale)
plus any imported .shape added with the [+] menu -- their toggle bar,
placement (side-by-side / world origin / object pivot), transparency,
solo, removal, and the geometry build behind them. State lives in
self._scene (ryzom_forgery/scene.py), one SceneEntry per shape. Split out of object_editor.py, see the "Split
object_editor.py into theme files" chantier in
project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.{ui_helpers,geometry_helpers}, NOT from
object_editor.py itself -- see ui_helpers.py's module docstring for why.
"""

import contextlib
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, imgui_ctx, portable_file_dialogs as pfd
from panda3d.core import GeomNode, TransparencyAttrib

from ryzom_forgery import scene as scene_model
from ryzom_forgery.explorer import ExplorerItem
from pynel.ryzom_bnp import BnpError, BnpReader
from pynel.ryzom_shape import ShapeParseError, parse_shape

from ryzom_forgery.shape_geometry import iter_render_passes, shape_bbox
from ryzom_forgery.apps.object_editor_mixins.geometry_helpers import _build_geom, _build_vertex_data
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_icon_button, _OBJECT_TRANSPARENCY_ALPHA, _VIEWPORT_TOGGLE_MARGIN_PX,
)

# Toggleable scale-reference shapes shown alongside whatever's loaded, for an
# at-a-glance sense of scale -- a 1x1x1 cube and the shortest/tallest playable
# character shapes, kept in this repo (not the Ryzom data tree) since they're
# tool fixtures, not game assets.
_REFERENCE_EXAMPLES_DIR = Path(__file__).resolve().parent.parent.parent / "examples"
_REFERENCE_SHAPES = [
	("Cube (1x1x1)", "ge_mission_1_caisse.shape"),
	("Smallest character", "npc_dummy_short.shape"),
	("Tallest character", "npc_dummy_tall.shape"),
]
_EDITED_COLOR = (0.95, 0.55, 0.15, 1.0)  # button background of the scene shape currently being edited
_REFERENCE_GAP = 1.5  # meters between reference objects (and the main shape), beyond their own bbox width

# Icon for each toggle in the top-left viewport button bar (see
# _draw_reference_shapes_toggles()) -- these read as "important, chunky
# toggles" next to the smaller bottom-left _draw_viewport_toggles() bar, so
# they're drawn with app.large_icon_font (2x _ICON_FONT_SIZE, see app.py's
# _load_icon_font()) instead of the normal merged-into-text-font icon glyphs.
_IMPORTED_ICON = fa_icons.ICON_FA_CUBES
_ADD_POPUP_ID = "##scene-add-popup"
_REFERENCE_ICONS = {"Cube (1x1x1)": fa_icons.ICON_FA_CUBE, "Smallest character": fa_icons.ICON_FA_CHILD,
                     "Tallest character": fa_icons.ICON_FA_PERSON}


def reference_entry_id(label):
	return f"ref:{label}"


class ReferenceShapesMixin:
	def _get_scene_shape(self, entry):
		"""Lazily parses (once) and caches the ShapeFile of a scene entry --
		None if it can't be read/parsed. For the entry being edited the cache
		holds the live self.shape_file instead (see scene_io.py)."""
		if entry.id in self._scene_shapes:
			return self._scene_shapes[entry.id]
		source = entry.source
		kind = source.get("type")
		path = None
		try:
			if kind == scene_model.SOURCE_REFERENCE:
				path = _REFERENCE_EXAMPLES_DIR / dict(_REFERENCE_SHAPES)[source["label"]]
				data = path.read_bytes()
			elif kind == scene_model.SOURCE_BNP:
				path = Path(source["bnp"])
				data = BnpReader(path).read_file(source["name"])
			else:
				path = scene_model.resolve_source(source, self.workspace_setup_dialog.active_workspace_dir)
				data = path.read_bytes()
			shape_file = parse_shape(data)
		except (OSError, ShapeParseError, BnpError, KeyError, AttributeError) as exc:
			print(f"[object_editor] failed to load scene shape {entry.id!r} ({path}): {exc}")
			shape_file = None
		self._scene_shapes[entry.id] = shape_file
		return shape_file

	@contextlib.contextmanager
	def _scene_entry_textures(self, entry):
		"""Points _texture_search_dirs at the folder the entry's own file
		lives in while its geometry is built -- they otherwise stay those of
		the edited shape, so textures sitting next to another scene shape
		were reported missing."""
		source = entry.source
		dirs = []
		if source.get("type") in (scene_model.SOURCE_WORKSPACE, scene_model.SOURCE_FILE):
			path = scene_model.resolve_source(source, self.workspace_setup_dialog.active_workspace_dir)
			if path is not None:
				dirs = [path.parent]
		saved = self._texture_search_dirs
		self._texture_search_dirs = dirs
		try:
			yield
		finally:
			self._texture_search_dirs = saved

	def _scene_entry_label(self, entry):
		source = entry.source
		if source.get("type") == scene_model.SOURCE_REFERENCE:
			return source["label"]
		return Path(source.get("name") or source.get("path", entry.id)).name

	def _new_scene_entry_id(self, prefix):
		index = len(self._scene.entries)
		while self._scene.entry(f"{prefix}:{index}") is not None:
			index += 1
		return f"{prefix}:{index}"

	def _scene_source_for_item(self, item):
		if item.bnp_path is not None:
			return scene_model.source_for_bnp(item.bnp_path, item.name)
		return scene_model.source_for_path(item.path, self.workspace_setup_dialog.active_workspace_dir)

	def _scene_item_for_entry(self, entry):
		"""The ExplorerItem an imported entry was loaded from, or None for a
		reference / a source that no longer exists."""
		source = entry.source
		if source.get("type") == scene_model.SOURCE_BNP:
			bnp_path = Path(source["bnp"])
			return ExplorerItem(path=bnp_path, name=source["name"], bnp_path=bnp_path)
		path = scene_model.resolve_source(source, self.workspace_setup_dialog.active_workspace_dir)
		if path is None or source.get("type") == scene_model.SOURCE_REFERENCE:
			return None
		return ExplorerItem(path=path, name=path.name)

	def _scene_entry_for_item(self, item):
		source = self._scene_source_for_item(item)
		return next((e for e in self._scene.entries if e.source == source), None)

	def _add_scene_reference(self, label):
		entry_id = reference_entry_id(label)
		if self._scene.entry(entry_id) is not None:
			return
		self._scene.add(scene_model.SceneEntry(
			id=entry_id, source={"type": scene_model.SOURCE_REFERENCE, "label": label}))
		self._refresh_scene_shapes()

	def _add_scene_shape(self, item):
		"""Adds an imported .shape (an ExplorerItem) to the scene, shown
		side by side with the others -- ignored if that same file is already
		in the scene."""
		if self._scene_entry_for_item(item) is not None:
			return
		entry = scene_model.SceneEntry(id=self._new_scene_entry_id("shape"), source=self._scene_source_for_item(item))
		if self._get_scene_shape(entry) is None:
			self._scene_shapes.pop(entry.id, None)
			return
		self._scene.add(entry)
		self._refresh_scene_shapes()

	def _open_scene_shape_dialog(self):
		self._scene_shape_dialog = pfd.open_file("Add a shape to the scene", "", ["Shape files", "*.shape"])

	def _poll_scene_shape_dialog(self):
		if self._scene_shape_dialog is None or not self._scene_shape_dialog.ready(0):
			return
		dialog, self._scene_shape_dialog = self._scene_shape_dialog, None
		for result in dialog.result():
			path = Path(result)
			self._add_scene_shape(ExplorerItem(path=path, name=path.name))

	def _remove_scene_shape(self, entry):
		self._scene.remove(entry.id)
		self._scene_shapes.pop(entry.id, None)
		self._scene_dirty.pop(entry.id, None)
		self._refresh_scene_shapes()

	def _toggle_scene_visible(self, entry):
		if not entry.visible and self._get_scene_shape(entry) is None:
			return
		entry.visible = not entry.visible
		self._refresh_scene_shapes()

	def _toggle_scene_solo(self, entry):
		self._scene.set_solo(entry.id)
		self._refresh_scene_shapes()

	def _set_scene_placement(self, entry, placement):
		"""Clicking the already-active placement button turns it back off
		(back to the default side-by-side layout) instead of being stuck on."""
		entry.placement = "auto" if entry.placement == placement else placement
		self._refresh_scene_shapes()

	def _toggle_scene_transparency(self, entry):
		entry.transparent = not entry.transparent
		self._refresh_scene_shapes()

	def _refresh_scene_shapes(self):
		"""Re-applies everything the scene decides after an entry changed
		(visibility, solo, placement, transparency, bind...) without
		reframing the camera: the entries bound on the creature, then the
		standalone side-by-side ones."""
		self._apply_scene_shapes_to_creature()
		self._rebuild_reference_shapes()

	def _main_shape_shown(self):
		entry = self._scene.entry(self._scene.edited_id)
		return entry is None or self._scene.is_shown(entry)

	def _apply_main_shape_visibility(self):
		"""Shows/hides the shape being edited (visibility toggle, solo on
		another entry) -- its bound copy on the creature when there is one,
		its standalone model_root otherwise."""
		shown = self._main_shape_shown()
		node = self._assembled_creature_loaded_shape_node
		if node is not None and not node.is_empty():
			(node.show if shown else node.hide)()
		else:
			(self.model_root.show if shown else self.model_root.hide)()

	def _build_reference_geometry(self, shape_value, parent_node_path):
		materials = getattr(shape_value, "materials", None)
		vdata = None
		for vertex_buffer, material_id, indices in iter_render_passes(shape_value):
			if not indices:
				continue
			if vdata is None:
				# Built once, not per-pass -- see _build_vertex_data()'s docstring.
				vdata = _build_vertex_data(vertex_buffer)
			geom = _build_geom(vdata, indices)
			geom_node = GeomNode(f"ref-pass-{material_id}")
			geom_node.add_geom(geom)
			node_path = parent_node_path.attach_new_node(geom_node)
			material = materials[material_id] if materials and material_id < len(materials) else None
			self._apply_material_common(node_path, material)
			self._apply_material_texture(node_path, material, None)

	def _rebuild_reference_shapes(self):
		"""Lines up the currently-shown standalone scene shapes side by
		side, just past the main shape's own bbox (bottoms/centers aligned
		on X, so comparing sizes doesn't need any camera repositioning) --
		unless a shape has its own placement mode set (see
		_set_scene_placement()), in which case it's placed at the world
		origin or the main object's pivot instead, and skipped when
		advancing the side-by-side cursor. The entry being edited is the
		main shape itself (model_root), and an entry bound on the displayed
		creature is built there (_apply_scene_shapes_to_creature()): neither
		is drawn here. With a solo shape set, only that one is shown."""
		self._reference_root.remove_node()
		self._reference_root = self.render.attach_new_node("reference-root")

		main_bbox = shape_bbox(self.shape_file.value) if self.shape_file is not None else None
		cursor_x = (main_bbox.center.x + main_bbox.half_size.x + _REFERENCE_GAP) if main_bbox is not None else 0.0

		for entry in self._scene.entries:
			if entry.id == self._scene.edited_id or not self._scene.is_shown(entry):
				continue
			if self._scene_entry_bound_on_creature(entry):
				continue
			shape_file = self._get_scene_shape(entry)
			if shape_file is None:
				continue
			bbox = shape_bbox(shape_file.value)
			node_path = self._reference_root.attach_new_node(entry.id)
			placement = entry.placement
			if placement == "origin":
				node_path.set_pos(self.render, 0, 0, 0)
			elif placement == "pivot":
				node_path.set_pos(self.render, self._object_pivot.get_pos(self.render))
			else:
				if bbox is not None:
					node_path.set_x(cursor_x + bbox.half_size.x - bbox.center.x)
			with self._scene_entry_textures(entry):
				self._build_reference_geometry(shape_file.value, node_path)
			if entry.transparent:
				node_path.set_transparency(TransparencyAttrib.M_alpha)
				node_path.set_color_scale(1, 1, 1, _OBJECT_TRANSPARENCY_ALPHA)
			if placement == "auto" and bbox is not None:
				cursor_x += bbox.half_size.x * 2 + _REFERENCE_GAP

		self._apply_main_shape_visibility()

	def _draw_temp_shape_column(self, temp_entry, large_font):
		"""The first column of the bar: the "temp shape", the shape loaded
		from the Explorer and not (yet) added to the scene. Selected-as-edited
		indicator (orange), show/hide, show only -- all greyed out while
		there is no temp shape."""
		missing = temp_entry is None
		label = "" if missing else self._scene_entry_label(temp_entry)
		imgui.push_id("scene-temp-shape")
		with imgui_ctx.begin_group():
			tooltip = ("No temporary shape: load one from the Explorer" if missing
			           else f"{label} (temporary shape being edited: [+] > Add the current shape keeps it in the scene)")
			_icon_button(_IMPORTED_ICON, tooltip, not missing, square=True, large_font=large_font,
			             active_color=_EDITED_COLOR, disabled=missing)
			if _icon_button(fa_icons.ICON_FA_EYE, "Show/hide", not missing and temp_entry.visible,
			                square=True, large_font=large_font, disabled=missing):
				self._toggle_scene_visible(temp_entry)
			if _icon_button(fa_icons.ICON_FA_CROSSHAIRS, "Show only this shape",
			                not missing and self._scene.solo_id == temp_entry.id,
			                square=True, large_font=large_font, disabled=missing):
				self._toggle_scene_solo(temp_entry)
		imgui.pop_id()

	def _draw_scene_entry_column(self, entry, large_font):
		"""One scene shape's column of the bar. A reference's top button
		toggles its visibility. An imported shape's top button selects it as
		the shape being edited (orange background while it is), with its
		visibility eye right below. Then, for every visible shape except the
		edited one: placement (origin / pivot) and 50% transparency (the
		edited one: the viewport's own transparency); for an imported shape:
		"show only this one" (solo); finally "remove from the scene" (for the
		edited one: it goes back to being the temp shape)."""
		label = self._scene_entry_label(entry)
		is_edited = entry.id == self._scene.edited_id
		imgui.push_id(f"scene-shape-{entry.id}")
		with imgui_ctx.begin_group():
			if entry.editable:
				if _icon_button(_IMPORTED_ICON, f"{label} (click to edit this shape)", is_edited,
				                square=True, large_font=large_font, active_color=_EDITED_COLOR):
					self._select_edited_scene_shape(entry)
				if _icon_button(fa_icons.ICON_FA_EYE, "Show/hide", entry.visible,
				                square=True, large_font=large_font):
					self._toggle_scene_visible(entry)
			else:
				if _icon_button(_REFERENCE_ICONS[label], label, entry.visible, square=True, large_font=large_font):
					self._toggle_scene_visible(entry)
			if entry.visible and not is_edited:
				# Placement only matters for a standalone shape: a bound one is
				# positioned by its slot / attach point.
				if not self._scene_entry_bound_on_creature(entry):
					if _icon_button(fa_icons.ICON_FA_CIRCLE_DOT, "Place at 0,0,0",
					                entry.placement == "origin", square=True, large_font=large_font):
						self._set_scene_placement(entry, "origin")
					if _icon_button(fa_icons.ICON_FA_ANCHOR, "Place on the object's pivot",
					                entry.placement == "pivot", square=True, large_font=large_font):
						self._set_scene_placement(entry, "pivot")
				if _icon_button(fa_icons.ICON_FA_CIRCLE_HALF_STROKE, "50% transparent",
				                entry.transparent, square=True, large_font=large_font):
					self._toggle_scene_transparency(entry)
			elif is_edited:
				if _icon_button(fa_icons.ICON_FA_CIRCLE_HALF_STROKE, "50% transparent",
				                self._object_transparent, square=True, large_font=large_font):
					self._toggle_object_transparency()
			if entry.editable:
				if _icon_button(fa_icons.ICON_FA_CROSSHAIRS, "Show only this shape",
				                self._scene.solo_id == entry.id, square=True, large_font=large_font):
					self._toggle_scene_solo(entry)
			if is_edited:
				if _icon_button(fa_icons.ICON_FA_TRASH, "Remove from the scene (it stays loaded as the temp shape)",
				                False, square=True, large_font=large_font):
					entry.kept = False
			elif _icon_button(fa_icons.ICON_FA_TRASH, "Remove from the scene", False,
			                  square=True, large_font=large_font):
				self._remove_scene_shape(entry)
		imgui.pop_id()

	def _draw_reference_shapes_toggles(self):
		"""Top-left viewport bar of the scene's shapes. Square icon buttons
		at 2x _draw_viewport_toggles()'s icon size (app.large_icon_font), one
		column per shape, each column its own imgui group so everything stays
		aligned regardless of how many buttons it has.

		Left to right: the temp shape's column (the shape loaded from the
		Explorer and not added to the scene yet, greyed out when there is none
		-- _draw_temp_shape_column()), then one column per shape of the scene
		(the 3 scale references -- Cube / shortest / tallest character -- and
		the imported shapes, _draw_scene_entry_column()), then [+] (a menu
		listing the 3 references, "Add the current shape" while there is a
		temp shape, and "<add a .shape file>"), then the scene's own load /
		save-as buttons (scene_io.py)."""
		display_size = imgui.get_io().display_size
		if display_size.y <= 0:
			return
		self._poll_scene_shape_dialog()

		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##reference-shapes-toggles", flags=flags):
			edited_entry = self._scene.entry(self._scene.edited_id)
			temp_entry = edited_entry if edited_entry is not None and not edited_entry.kept else None
			self._draw_temp_shape_column(temp_entry, large_font)
			for entry in [e for e in self._scene.entries if e is not temp_entry]:
				imgui.same_line()
				self._draw_scene_entry_column(entry, large_font)

			imgui.same_line()
			imgui.begin_group()
			if _icon_button(fa_icons.ICON_FA_PLUS, "Add a shape to the scene", square=True, large_font=large_font):
				imgui.open_popup(_ADD_POPUP_ID)
			if imgui.begin_popup(_ADD_POPUP_ID):
				for label, _ in _REFERENCE_SHAPES:
					already = self._scene.entry(reference_entry_id(label)) is not None
					imgui.begin_disabled(already)
					clicked, _selected = imgui.selectable(label, already)
					imgui.end_disabled()
					if clicked:
						self._add_scene_reference(label)
				if temp_entry is not None:
					imgui.separator()
					clicked, _selected = imgui.selectable(
						f"Add the current shape ({self._scene_entry_label(temp_entry)})", False)
					if clicked:
						self._keep_edited_scene_shape()
				imgui.separator()
				clicked, _selected = imgui.selectable("<add a .shape file>", False)
				if clicked:
					self._open_scene_shape_dialog()
				imgui.end_popup()
			self._draw_scene_file_buttons(large_font)
			imgui.end_group()
