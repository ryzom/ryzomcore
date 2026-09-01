"""ObjectEditorApp mixin: the 3 toggleable scale-reference shapes (1x1x1
cube, shortest/tallest playable character) shown alongside whatever's
loaded, for an at-a-glance sense of scale -- their toggle bar, placement
(side-by-side / world origin / object pivot), transparency, and the
geometry build behind them. Split out of object_editor.py, see the "Split
object_editor.py into theme files" chantier in
project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.{ui_helpers,geometry_helpers}, NOT from
object_editor.py itself -- see ui_helpers.py's module docstring for why.
"""

from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx
from panda3d.core import GeomNode, TransparencyAttrib

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
_REFERENCE_GAP = 1.5  # meters between reference objects (and the main shape), beyond their own bbox width

# Icon for each toggle in the top-left viewport button bar (see
# _draw_reference_shapes_toggles()) -- these read as "important, chunky
# toggles" next to the smaller bottom-left _draw_viewport_toggles() bar, so
# they're drawn with app.large_icon_font (2x _ICON_FONT_SIZE, see app.py's
# _load_icon_font()) instead of the normal merged-into-text-font icon glyphs.
_REFERENCE_ICONS = {"Cube (1x1x1)": fa_icons.ICON_FA_CUBE, "Smallest character": fa_icons.ICON_FA_CHILD,
                     "Tallest character": fa_icons.ICON_FA_MALE}


class ReferenceShapesMixin:
	def _get_reference_shape(self, label):
		"""Lazily parses (once) and caches a reference shape by label."""
		if label in self._reference_shapes:
			return self._reference_shapes[label]
		filename = dict(_REFERENCE_SHAPES)[label]
		path = _REFERENCE_EXAMPLES_DIR / filename
		try:
			shape_file = parse_shape(path.read_bytes())
		except (OSError, ShapeParseError) as exc:
			print(f"[object_editor] failed to load reference shape {label!r} ({path}): {exc}")
			shape_file = None
		self._reference_shapes[label] = shape_file
		return shape_file

	def _toggle_reference_shape(self, label):
		if label in self._reference_active:
			self._reference_active.discard(label)
		elif self._get_reference_shape(label) is not None:
			self._reference_active.add(label)
		self._rebuild_reference_shapes()

	def _set_reference_placement(self, label, placement):
		"""Clicking the already-active placement button turns it back off
		(back to the default side-by-side layout) instead of being stuck on."""
		current = self._reference_placement.get(label, "auto")
		self._reference_placement[label] = "auto" if current == placement else placement
		self._rebuild_reference_shapes()

	def _toggle_reference_transparency(self, label):
		if label in self._reference_transparent:
			self._reference_transparent.discard(label)
		else:
			self._reference_transparent.add(label)
		self._rebuild_reference_shapes()

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
		"""Lines up the currently-active reference shapes side by side, just
		past the main shape's own bbox (bottoms/centers aligned on X, so
		comparing sizes doesn't need any camera repositioning) -- unless a
		shape has its own placement mode set (see _set_reference_placement()),
		in which case it's placed at the world origin or the main object's
		pivot instead, and skipped when advancing the side-by-side cursor."""
		self._reference_root.remove_node()
		self._reference_root = self.render.attach_new_node("reference-root")

		main_bbox = shape_bbox(self.shape_file.value) if self.shape_file is not None else None
		cursor_x = (main_bbox.center.x + main_bbox.half_size.x + _REFERENCE_GAP) if main_bbox is not None else 0.0

		for label, _ in _REFERENCE_SHAPES:
			if label not in self._reference_active:
				continue
			shape_file = self._reference_shapes.get(label)
			if shape_file is None:
				continue
			bbox = shape_bbox(shape_file.value)
			node_path = self._reference_root.attach_new_node(label)
			placement = self._reference_placement.get(label, "auto")
			if placement == "origin":
				node_path.set_pos(self.render, 0, 0, 0)
			elif placement == "pivot":
				node_path.set_pos(self.render, self._object_pivot.get_pos(self.render))
			else:
				if bbox is not None:
					node_path.set_x(cursor_x + bbox.half_size.x - bbox.center.x)
			self._build_reference_geometry(shape_file.value, node_path)
			if label in self._reference_transparent:
				node_path.set_transparency(TransparencyAttrib.M_alpha)
				node_path.set_color_scale(1, 1, 1, _OBJECT_TRANSPARENCY_ALPHA)
			if placement == "auto" and bbox is not None:
				cursor_x += bbox.half_size.x * 2 + _REFERENCE_GAP

	def _draw_reference_shapes_toggles(self):
		"""Top-left viewport bar for the 3 scale-reference toggles (Cube /
		shortest / tallest character) -- square icon buttons at 2x
		_draw_viewport_toggles()'s icon size (app.large_icon_font), since
		these are a more prominent, deliberately-reached-for control. Once a
		reference shape is active, 3 more square buttons appear stacked
		vertically right below its toggle (placement: origin/pivot, plus a
		transparency toggle) -- each shape's column is its own imgui group so
		everything stays aligned regardless of how many buttons the column
		has."""
		display_size = imgui.get_io().display_size
		if display_size.y <= 0:
			return

		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##reference-shapes-toggles", flags=flags):
			for i, (label, _) in enumerate(_REFERENCE_SHAPES):
				if i > 0:
					imgui.same_line()
				imgui.push_id(f"ref-shape-{label}")
				with imgui_ctx.begin_group():
					if _icon_button(_REFERENCE_ICONS[label], label, label in self._reference_active,
					                square=True, large_font=large_font):
						self._toggle_reference_shape(label)
					if label in self._reference_active:
						placement = self._reference_placement.get(label, "auto")
						if _icon_button(fa_icons.ICON_FA_DOT_CIRCLE, "Place at 0,0,0",
						                placement == "origin", square=True, large_font=large_font):
							self._set_reference_placement(label, "origin")
						if _icon_button(fa_icons.ICON_FA_ANCHOR, "Place on the object's pivot",
						                placement == "pivot", square=True, large_font=large_font):
							self._set_reference_placement(label, "pivot")
						if _icon_button(fa_icons.ICON_FA_ADJUST, "50% transparent",
						                label in self._reference_transparent, square=True, large_font=large_font):
							self._toggle_reference_transparency(label)
				imgui.pop_id()
