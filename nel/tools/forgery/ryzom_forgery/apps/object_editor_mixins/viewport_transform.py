"""ObjectEditorApp mixin: floor grid/world/pivot axes helpers, object
transparency, the live wind preview, viewport toggle bar, the position/
rotation/scale transform panel, camera framing, and the main geometry
(re)build (_rebuild_geometry(), shared by shape load and geometry replace).
Split out of object_editor.py, see the "Split object_editor.py into theme
files" chantier in project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.{ui_helpers,geometry_helpers,
skin_state_helpers}, NOT from object_editor.py itself -- see ui_helpers.py's
module docstring for why.
"""

from math import ceil, cos, pi, radians, sin

import numpy

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, imgui_ctx
from panda3d.core import ClockObject, GeomNode, InternalName, NodePath, Point3, Quat, TransparencyAttrib, Vec3

from pynel.ryzom_shape import Mesh, MeshMRMSkinned, WindTreeParams

from ryzom_forgery.shape_geometry import iter_render_passes, shape_bbox, shape_geom, shape_stats
from ryzom_forgery.apps.object_editor_mixins.geometry_helpers import (
	_AXIS_LENGTH, _AXIS_MARGIN_FACTOR, _build_axes_geom, _build_geom, _build_grid_geom,
	_build_shadow_skin_ground_node, _build_shadow_skin_vdata, _build_sun_globe_geom, _build_vertex_data,
	_GRID_MARGIN_SQUARES, _GRID_SQUARES, _GRID_STEP, _is_shape_skinned, _MIN_GRID_SQUARES, _PIVOT_AXIS_COLORS,
	_SHADOW_SKIN_COLOR, _SHADOW_SKIN_GROUND_OFFSET, _SHADOW_SKIN_GROUND_Z, _uvs_need_repeat, _WORLD_AXIS_COLORS,
)
from ryzom_forgery.apps.object_editor_mixins.skin_state_helpers import (
	_build_mesh_skin_state, _build_shadow_skin_preview_state, _build_skin_state, _build_wind_state,
)
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_ACTIVE_COLOR, _capture_panel_pos, _icon_button, _OBJECT_TRANSPARENCY_ALPHA, _set_panel_pos,
	_VIEWPORT_TOGGLE_MARGIN_PX,
)

_LOCKED_COLOR = (0.45, 0.45, 0.45, 0.8)  # grey -- "on" highlight for a lock toggle specifically

# Shading mode cycling button (project-todos/forgery/
# object_editor__shading_modes.md) -- icon/label per state, in cycle order.
_SHADING_MODE_ICONS = {
	"shading": fa_icons.ICON_FA_SUN,
	"constant": fa_icons.ICON_FA_LIGHTBULB,
	"unshaded": fa_icons.ICON_FA_MASK,
}
_SHADING_MODE_LABELS = {
	"shading": "Shading",
	"constant": "Constant Shading",
	"unshaded": "Unshaded",
}

# Wireframe cycling button (project-todos/forgery/wireframe_cycle_states.md)
# -- same icon in all 3 states (only the tooltip/active highlight change),
# unlike the shading mode button above.
_WIREFRAME_MODE_LABELS = {
	"off": "Wireframe off",
	"overlay": "Wireframe (overlay on shaded render)",
	"pure": "Wireframe (pure, no texture)",
}

# _update_sun_light()'s Play rotation rate -- a full 360deg cycle every 12s,
# fast enough to actually watch a specular highlight sweep across a shape
# without feeling like a slideshow, slow enough to still land on a precise
# angle by eye if the user pauses right after the interesting part.
_SUN_PLAY_SPEED_DEG_PER_SEC = 30.0

# _draw_light_controls()'s Ambient/Sun intensity sliders.
_LIGHT_INTENSITY_MAX = 4.0

# _draw_transform_row()'s per-property drag_float() speed/range -- position
# in meters (no hard limit needed for a 3D scene), rotation in degrees (a
# full turn either way), scale as a multiplier (must stay positive).
_TRANSFORM_DRAG_PARAMS = {
	"position": (0.005, -10000.0, 10000.0),
	"rotation": (0.5, -360.0, 360.0),
	"scale": (0.005, 0.001, 1000.0),
}


class ViewportTransformMixin:
	def _rebuild_viewport_helpers(self, bbox):
		"""(Re)builds the floor grid + world/pivot axes geometry, sized and
		centered to `bbox` (the just-loaded shape's bounding box, or None
		before any shape is loaded / for a shape with no renderable geometry --
		falls back to a fixed default size then). Called from __init__ and
		from _rebuild_geometry() every time the shape (and so its bbox)
		changes, so these always scale to cover whatever's on screen instead
		of a fixed size that's tiny for a building and huge for a trinket."""
		if bbox is not None:
			radius = max(bbox.half_size.x, bbox.half_size.y, bbox.half_size.z, 0.1)
			grid_squares = max(
				_MIN_GRID_SQUARES,
				ceil(max(bbox.half_size.x, bbox.half_size.y) * 2 / _GRID_STEP) + _GRID_MARGIN_SQUARES)
			# bbox is in model_root's own local space (pre-pivot-rotation) --
			# same reasoning as _rebuild_geometry()'s camera framing: must go
			# through model_root's current transform to land in world space.
			local_center = Point3(bbox.center.x, bbox.center.y, bbox.center.z)
			world_center = self.render.get_relative_point(self.model_root, local_center)
			grid_center_x, grid_center_y = world_center.x, world_center.y
			# Fixed at world Z=0 (not the bbox's own bottom) so it reads as an
			# actual ground reference -- whether the object floats above it or
			# sinks below it is exactly the thing this grid is meant to show.
			grid_z = 0.0
			axis_length = radius * _AXIS_MARGIN_FACTOR
		else:
			grid_squares = _GRID_SQUARES
			grid_center_x = grid_center_y = grid_z = 0.0
			axis_length = _AXIS_LENGTH

		self._grid_np.remove_node()
		self._grid_np = self.render.attach_new_node(
			_build_grid_geom(grid_center_x, grid_center_y, grid_z, grid_squares))
		self._grid_np.set_light_off()
		if not self._grid_visible:
			self._grid_np.hide()

		self._world_axes_np.remove_node()
		self._world_axes_np = self.render.attach_new_node(_build_axes_geom(_WORLD_AXIS_COLORS, axis_length))
		self._world_axes_np.set_light_off()
		if not self._world_axes_visible:
			self._world_axes_np.hide()

		self._pivot_axes_np.remove_node()
		self._pivot_axes_np = self._object_pivot.attach_new_node(_build_axes_geom(_PIVOT_AXIS_COLORS, axis_length))
		self._pivot_axes_np.set_light_off()
		if not self._pivot_axes_visible:
			self._pivot_axes_np.hide()

		# Sun gizmo (a small globe marker, see _build_sun_globe_geom()) --
		# world-space like the world axes (the sun's own orientation,
		# self.sun_light_np, is world-space too, parented to self.render in
		# app.py), synced every frame by _update_sun_light() (position
		# follows the object, orientation matches the sun) -- this only
		# (re)builds the globe mesh itself, sized/placed off the current
		# shape's bbox like the pivot/world axes. Visibility mirrors
		# self._light_panel_open (no separate toggle -- see
		# _draw_panel_taskbar()).
		self._sun_gizmo_np.remove_node()
		self._sun_gizmo_np = self.render.attach_new_node("sun-gizmo")
		self._sun_gizmo_np.attach_new_node(_build_sun_globe_geom(axis_length))
		self._sun_gizmo_np.set_light_off()
		if not self._light_panel_open:
			self._sun_gizmo_np.hide()

	def _toggle_grid(self):
		self._grid_visible = not self._grid_visible
		(self._grid_np.show if self._grid_visible else self._grid_np.hide)()

	def _toggle_world_axes(self):
		self._world_axes_visible = not self._world_axes_visible
		(self._world_axes_np.show if self._world_axes_visible else self._world_axes_np.hide)()

	def _toggle_pivot_axes(self):
		self._pivot_axes_visible = not self._pivot_axes_visible
		(self._pivot_axes_np.show if self._pivot_axes_visible else self._pivot_axes_np.hide)()

	def _toggle_shadow_skin_preview(self):
		self._shadow_skin_visible = not self._shadow_skin_visible
		if self._shadow_skin_np is not None:
			(self._shadow_skin_np.show if self._shadow_skin_visible else self._shadow_skin_np.hide)()

	def _apply_object_transparency(self):
		if self._object_transparent:
			self.model_root.set_transparency(TransparencyAttrib.M_alpha)
			self.model_root.set_color_scale(1, 1, 1, _OBJECT_TRANSPARENCY_ALPHA)
		else:
			self.model_root.clear_transparency()
			self.model_root.clear_color_scale()

	def _toggle_object_transparency(self):
		self._object_transparent = not self._object_transparent
		self._apply_object_transparency()

	def _apply_object_wireframe(self):
		"""Applies self._wireframe_mode to model_root (project-todos/forgery/
		wireframe_cycle_states.md) -- independent of transparency/shading,
		all combinable:
		- "off": no override.
		- "overlay": set_render_mode_filled_wireframe() -- wireframe drawn on
		  top of the normal shaded/textured render, never replacing it
		  (project-todos/forgery/object_editor__wireframe_overlay.md).
		- "pure": set_render_mode_wireframe() + set_texture_off() -- wireframe
		  only, no texture, no fill."""
		if self._wireframe_mode == "off":
			self.model_root.clear_render_mode()
			self.model_root.clear_texture()
		elif self._wireframe_mode == "overlay":
			self.model_root.clear_texture()
			self.model_root.set_render_mode_filled_wireframe((0, 0, 0, 1), 1)
		else:  # "pure"
			self.model_root.set_render_mode_wireframe(1)
			self.model_root.set_texture_off(1)

	def _set_object_wireframe_mode(self, mode):
		self._wireframe_mode = mode
		self._apply_object_wireframe()

	def _cycle_object_wireframe(self):
		"""Left-click behavior for the wireframe button -- advances to the
		next state (project-todos/forgery/wireframe_cycle_states.md, general
		cycling-button convention: right-click instead jumps straight to a
		chosen state, see _draw_viewport_toggles()'s own popup). Independent
		of transparency/shading -- all combinable."""
		next_mode = {"off": "overlay", "overlay": "pure", "pure": "off"}
		self._set_object_wireframe_mode(next_mode[self._wireframe_mode])

	def _apply_shading_mode(self):
		"""Applies self._shading_mode to model_root (project-todos/forgery/
		object_editor__shading_modes.md) -- independent of transparency/
		wireframe above, all combinable:
		- "shading" (normal): no override, inherits the scene's own ambient +
		  self.sun_light_np exactly like every other Forgery app.
		- "constant": self.sun_light_np specifically turned off on model_root
		  (the single-light overload of set_light_off(), not the all-lights
		  one) -- the globally-set ambient light (self.ambient_light_np, set
		  on self.render in app.py) still lights it, just with no
		  directional light/shadowing.
		- "unshaded": all lighting off, all textures off, flat
		  self._unshaded_color painted over the whole model instead."""
		if self._shading_mode == "shading":
			self.model_root.clear_light()
			self.model_root.clear_texture()
			self.model_root.clear_color()
		elif self._shading_mode == "constant":
			self.model_root.clear_light()
			self.model_root.set_light_off(self.sun_light_np, 1)
			self.model_root.clear_texture()
			self.model_root.clear_color()
		else:  # "unshaded"
			self.model_root.set_light_off(1)
			self.model_root.set_texture_off(1)
			r, g, b = self._unshaded_color
			self.model_root.set_color(r, g, b, 1.0, 1)

	def _transition_shading_mode(self, new_mode):
		"""Actually changes self._shading_mode to `new_mode`, boosting/
		restoring the ambient light intensity around the edges of "constant"
		(project-todos/forgery/object_editor__shading_modes.md, Nuno
		2026-09-11) -- entering "constant" from anywhere else saves the
		current self._ambient_light_intensity and maxes it out
		(_LIGHT_INTENSITY_MAX), leaving "constant" for anywhere else restores
		it. Deliberately only in the actual mode-change entry points
		(_cycle_shading_mode()/this), never in _apply_shading_mode() itself,
		which only ever re-applies the CURRENT mode after _rebuild_geometry()
		and must never re-trigger a save/boost while already in "constant"."""
		old_mode = self._shading_mode
		if old_mode != "constant" and new_mode == "constant":
			self._saved_ambient_light_intensity = self._ambient_light_intensity
			self._ambient_light_intensity = _LIGHT_INTENSITY_MAX
			self._apply_light_settings()
		elif old_mode == "constant" and new_mode != "constant":
			self._ambient_light_intensity = self._saved_ambient_light_intensity
			self._apply_light_settings()
		self._shading_mode = new_mode
		self._apply_shading_mode()

	def _cycle_shading_mode(self):
		"""Left-click behavior for the shading mode button -- advances to the
		next state (project-todos/forgery/object_editor__shading_modes.md,
		Nuno 2026-09-11's general cycling-button convention: right-click
		instead jumps straight to a chosen state, see _draw_viewport_toggles()'s
		own popup)."""
		next_mode = {"shading": "constant", "constant": "unshaded", "unshaded": "shading"}
		self._transition_shading_mode(next_mode[self._shading_mode])

	def _set_shading_mode(self, mode):
		self._transition_shading_mode(mode)

	def _update_wind(self, task):
		"""Per-frame wind animation, ported from nel/src/3d/meshvp_wind_tree.cpp
		(CMeshVPWindTree::setupPerMesh()/setupPerInstanceConstants()) and its
		wind_tree_vp.glsl vertex decode -- see _build_wind_state()'s docstring.
		No-op whenever the loaded shape has nothing to animate, or the user
		paused it (self._wind_animate)."""
		state = self._wind_state
		if state is None or not self._wind_animate or state.vdata is None:
			return task.cont

		dt = ClockObject.get_global_clock().get_dt()
		params = state.params

		# Wind direction is a WORLD-space vector (matches the engine: the same
		# global wind blows every instance the same way, see
		# CScene::getGlobalWindDirection()) -- converted to the object's own
		# local space the same way setupPerInstanceConstants() does, via the
		# inverse of the pivot's current *rotation only* (a pure direction,
		# translation doesn't apply), so Ctrl-dragging the object to face a
		# different way doesn't change how the wind reads relative to it.
		direction_rad = radians(self._wind_direction_deg)
		wind_dir_world = Vec3(cos(direction_rad), sin(direction_rad), 0.0)
		pivot_quat = self._object_pivot.get_quat(self.render)
		inv_pivot_quat = Quat(pivot_quat)
		inv_pivot_quat.invert_in_place()
		wind_dir_local = inv_pivot_quat.xform(wind_dir_world)

		def speed_cos(t):
			return cos(2.0 * pi * t)

		max_delta = []
		for i in range(3):
			state.current_time[i] = (
				state.current_time[i] + dt * (params.frequency[i] + params.frequency_wind_factor[i] * self._wind_power)
			) % 1.0
			max_delta.append(Vec3(
				wind_dir_local.x * params.power_xy[i] * self._wind_power,
				wind_dir_local.y * params.power_xy[i] * self._wind_power,
				params.power_z[i] * self._wind_power,
			))

		f0 = speed_cos(state.current_time[0]) + params.bias[0]
		wind_l1 = numpy.array([max_delta[0].x, max_delta[0].y, max_delta[0].z], dtype=numpy.float32) * f0

		wind_l2 = numpy.empty((4, 3), dtype=numpy.float32)
		wind_l3 = numpy.empty((4, 3), dtype=numpy.float32)
		for k in range(4):
			f2 = speed_cos(state.current_time[1] + k * 0.25) + params.bias[1]
			wind_l2[k] = (max_delta[1].x * f2, max_delta[1].y * f2, max_delta[1].z * f2)
			f3 = speed_cos(state.current_time[2] + k * 0.25) + params.bias[2]
			wind_l3[k] = (max_delta[2].x * f3, max_delta[2].y * f3, max_delta[2].z * f3)

		offset = (state.factors[:, 0:1] * wind_l1
		          + wind_l2[state.idx2] * state.factors[:, 1:2]
		          + wind_l3[state.idx3] * state.factors[:, 2:3])
		new_positions = state.base_positions + offset

		# A per-row GeomVertexRewriter loop (tried first) means one
		# Python-level set_row()/set_data3() call pair per vertex, every
		# frame -- fine for a handful of vertices, but a real prop-sized mesh
		# (e.g. 28k verts on ooc_summer_raceline.shape) dropped this from
		# 60fps to ~38fps, all in Python call overhead rather than actual
		# work. GeomVertexArrayData supports the buffer protocol directly
		# (confirmed writable via modify_array()), so a numpy view onto it
		# lets every vertex's position be written in one vectorized C-level
		# assignment instead -- but modify_array() itself (not just writing
		# into the buffer it returns) is what bumps the array's modification
		# stamp Panda3D's GSG checks to know the GPU-side vertex buffer needs
		# re-uploading; caching the numpy view across frames and only writing
		# into the same buffer (tried first) silently stopped animating
		# anything after the first frame, since nothing told the renderer the
		# data had changed. modify_array() itself is cheap (no data copy), so
		# it's called fresh every frame -- only the row layout (stride/offset)
		# is cached, since that can't change for this vdata's lifetime.
		array_data = state.vdata.modify_array(0)
		if state.vertex_pos_offset is None:
			array_format = array_data.get_array_format()
			state.vertex_stride = array_format.get_stride() // 4
			state.vertex_pos_offset = array_format.get_column(InternalName.get_vertex()).get_start() // 4

		view = numpy.frombuffer(array_data, dtype=numpy.float32).reshape(-1, state.vertex_stride)
		pos_offset = state.vertex_pos_offset
		view[:, pos_offset:pos_offset + 3] = new_positions

		return task.cont

	def _draw_wind_controls(self):
		"""Viewer-only wind preview controls (strength/direction/play-pause),
		see _update_wind() -- only shown when the loaded shape actually has
		wind data to animate (self._wind_state), since there's nothing to
		preview otherwise, and only while self._wind_panel_open (toggled
		from the right-edge taskbar, see _draw_panel_taskbar()) -- force-
		closed the moment the loaded shape stops having wind data, so a
		shape switch never leaves a stale open panel with nothing to show.
		Not a shape property, so it's a floating window rather than part of
		the side panel; positioned only once, the first time it's ever
		opened (Cond_.once -- unlike Cond_.appearing, this does NOT refire
		on every later close/reopen, so a dragged position sticks for the
		rest of the session), then left free to be dragged anywhere."""
		if self._wind_state is None:
			self._wind_panel_open = False
			return
		if not self._wind_panel_open:
			return

		display_width = imgui.get_io().display_size.x
		taskbar_w = self._panel_taskbar_size[0]
		win_w, win_h = self._wind_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX * 2 - taskbar_w - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		_set_panel_pos(self._wind_panel_pos, x, y)
		flags = imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.always_auto_resize.value
		with imgui_ctx.begin("Wind preview", flags=flags):
			_, self._wind_animate = imgui.checkbox("Animate", self._wind_animate)
			imgui.set_next_item_width(160)
			_, self._wind_power = imgui.slider_float("Strength", self._wind_power, 0.0, 1.0)
			imgui.set_next_item_width(160)
			_, self._wind_direction_deg = imgui.slider_float("Direction", self._wind_direction_deg, 0.0, 360.0, "%.0f deg")
			self._wind_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)
			self._wind_panel_pos = _capture_panel_pos()

	def _apply_light_settings(self):
		"""Pushes the current Ambient/Sun settings (see
		_draw_light_controls()) onto the real Panda3D light nodes app.py
		created (self.ambient_light/self.sun_light/self.sun_light_np) --
		color x intensity, the sun's heading/pitch as its HPR. Called once
		at startup and after every manual edit; _update_sun_light() drives
		the sun's own color/HPR directly itself while Play is active,
		instead of going through here every frame."""
		r, g, b = self._ambient_light_color
		self.ambient_light.set_color((
			r * self._ambient_light_intensity, g * self._ambient_light_intensity,
			b * self._ambient_light_intensity, 1))

		r, g, b = self._sun_light_color
		self.sun_light.set_color((
			r * self._sun_light_intensity, g * self._sun_light_intensity,
			b * self._sun_light_intensity, 1))
		self.sun_light_np.set_hpr(self._sun_heading_deg, self._sun_pitch_deg, 0)

	def _update_sun_light(self, task):
		"""Per-frame. While Play is active (self._sun_playing), advances the
		sun's heading at a fixed rate and re-applies it via
		_apply_light_settings() -- same intensity as the manual baseline at
		every heading, no day/night dimming (tried, dropped: it made Play
		and a manually-set heading disagree on how the same angle looks,
		confusing rather than useful -- 2026-09-02, Nuno). Pitch is never
		touched here, only ever set manually (see _draw_light_controls()).

		Independently of Play, also keeps the sun gizmo (see
		_rebuild_viewport_helpers()) in sync every frame: shown/hidden with
		self._light_panel_open, positioned at the object's own world
		position (so it visually follows the loaded shape, even though the
		sun's orientation itself is world-space, not object-relative) and
		oriented to match self.sun_light_np's current HPR exactly."""
		if self._sun_playing:
			dt = ClockObject.get_global_clock().get_dt()
			self._sun_heading_deg = (self._sun_heading_deg + dt * _SUN_PLAY_SPEED_DEG_PER_SEC) % 360.0
			self._apply_light_settings()

		if self._light_panel_open:
			self._sun_gizmo_np.show()
			self._sun_gizmo_np.set_pos(self.render, self._object_pivot.get_pos(self.render))
			self._sun_gizmo_np.set_hpr(self.render, self.sun_light_np.get_hpr(self.render))
		else:
			self._sun_gizmo_np.hide()

		return task.cont

	def _draw_light_controls(self):
		"""Viewer-only Ambient/Sun controls (see _apply_light_settings()/
		_update_sun_light()) -- lets a material's ambient/specular/emissive/
		shininess (see materials.py's own Lighting section) actually be
		previewed: those 4 fields were already read/applied to the preview
		but the scene's only 2 lights were fixed/uncontrollable, so editing
		them would never visibly change anything. Always applicable (unlike
		the wind/bone-preview panels, lighting isn't shape-specific), but
		still only shown while self._light_panel_open (toggled from the
		right-edge taskbar, see _draw_panel_taskbar()). Positioned only
		once, the first time it's ever opened (see _draw_wind_controls()'s
		own comment on Cond_.once), then left free to be dragged anywhere."""
		if not self._light_panel_open:
			return

		display_width = imgui.get_io().display_size.x
		taskbar_w = self._panel_taskbar_size[0]
		win_w, win_h = self._light_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX * 2 - taskbar_w - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX + 220.0
		_set_panel_pos(self._light_panel_pos, x, y)
		flags = imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.always_auto_resize.value
		with imgui_ctx.begin("Lighting", flags=flags):
			imgui.text("Ambient")
			imgui.set_next_item_width(160)
			changed_a, self._ambient_light_color = imgui.color_edit3(
				"Color##ambient-light", self._ambient_light_color)
			imgui.set_next_item_width(160)
			changed_b, self._ambient_light_intensity = imgui.slider_float(
				"Intensity##ambient-light", self._ambient_light_intensity, 0.0, _LIGHT_INTENSITY_MAX)

			imgui.separator()
			imgui.text("Sun")
			imgui.set_next_item_width(160)
			changed_c, self._sun_light_color = imgui.color_edit3("Color##sun-light", self._sun_light_color)
			imgui.set_next_item_width(160)
			changed_d, self._sun_light_intensity = imgui.slider_float(
				"Intensity##sun-light", self._sun_light_intensity, 0.0, _LIGHT_INTENSITY_MAX)
			imgui.set_next_item_width(160)
			changed_e, self._sun_heading_deg = imgui.slider_float(
				"Heading##sun-light", self._sun_heading_deg, 0.0, 360.0, "%.0f deg")
			imgui.set_next_item_width(160)
			changed_f, self._sun_pitch_deg = imgui.slider_float(
				"Pitch##sun-light", self._sun_pitch_deg, -90.0, 90.0, "%.0f deg")
			icon = fa_icons.ICON_FA_PAUSE if self._sun_playing else fa_icons.ICON_FA_PLAY
			if _icon_button(f"{icon}##sun-light-play", "Play/pause the sun's day-night cycle"):
				self._sun_playing = not self._sun_playing

			if changed_a or changed_b or changed_c or changed_d or changed_e or changed_f:
				self._apply_light_settings()
			self._light_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)
			self._light_panel_pos = _capture_panel_pos()

	def _draw_info_panel(self):
		"""Floating "Shape info" panel (panel_improvements.md) -- read-only
		summary stats (shape_geometry.shape_stats()) about the currently
		loaded shape, recomputed fresh every frame it's open (no caching --
		a loaded shape doesn't change often enough for this to matter).
		Toggled from the taskbar (see _draw_panel_taskbar()), force-closed
		the moment no shape is loaded, same "applicable or not" convention
		as the other floating panels."""
		if self.shape_file is None:
			self._info_panel_open = False
			return
		if not self._info_panel_open:
			return

		materials = getattr(self.shape_file.value, "materials", None)
		source_path = self._shape_source_path if self._shape_source_path is not None else None
		stats = shape_stats(self.shape_file.value, materials, source_path)

		display_width = imgui.get_io().display_size.x
		taskbar_w = self._panel_taskbar_size[0]
		win_w, win_h = self._info_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX * 2 - taskbar_w - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX + 280.0
		_set_panel_pos(self._info_panel_pos, x, y)
		flags = imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.always_auto_resize.value
		with imgui_ctx.begin("Shape info", flags=flags):
			imgui.text(f"Type: {stats['type']}")
			imgui.text(f"Triangles: {stats['triangles']}")
			imgui.text(f"Vertices: {stats['vertices']}")
			imgui.text(f"LODs: {stats['lods']}")
			imgui.text(f"Bones: {stats.get('bones', 0)}")
			imgui.text(f"Materials: {stats['materials']}")
			if "textures" in stats:
				imgui.text(f"Textures: {stats['textures']}")
			if "bbox_size" in stats:
				sx, sy, sz = stats["bbox_size"]
				imgui.text(f"Bbox size: {sx:.2f} x {sy:.2f} x {sz:.2f} m")
			if "file_size_bytes" in stats:
				imgui.text(f"File size: {stats['file_size_bytes'] / 1024:.1f} KB")
			self._info_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)
			self._info_panel_pos = _capture_panel_pos()

	def _set_viewport_background_color(self, index):
		color = self._viewport_bg_colors[index]
		self.setBackgroundColor(*color)
		self._viewport_bg_active = index

	def _draw_background_color_bar(self):
		"""Small floating vertical swatch-button bar bottom-left of the 3D
		viewport, stacked directly above _draw_viewport_toggles()'s own bar
		(same x, same self-measurement trick for the y offset): black, grey,
		chroma-key green, then 2 free slots -- right-click any swatch to
		recolor it via a small color-picker popup, left-click to apply it as
		the viewport's background."""
		display_size = imgui.get_io().display_size
		win_h = display_size.y
		if win_h <= 0:
			return

		toggle_width, toggle_height = self._viewport_toggle_size
		width, height = self._background_color_bar_size
		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - toggle_height - _VIEWPORT_TOGGLE_MARGIN_PX - height
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		labels = ["Black", "Grey", "Green screen", "Custom 1 (right-click to change)", "Custom 2 (right-click to change)"]
		size = (imgui.get_frame_height(), imgui.get_frame_height())
		with imgui_ctx.begin("##background-color-bar", flags=flags):
			for i, color in enumerate(self._viewport_bg_colors):
				imgui.push_style_color(imgui.Col_.button.value, color)
				r, g, b, a = color
				imgui.push_style_color(imgui.Col_.button_hovered.value,
				                        (min(r + 0.1, 1.0), min(g + 0.1, 1.0), min(b + 0.1, 1.0), a))
				imgui.push_style_color(imgui.Col_.button_active.value,
				                        (max(r - 0.1, 0.0), max(g - 0.1, 0.0), max(b - 0.1, 0.0), a))
				if self._viewport_bg_active == i:
					imgui.push_style_color(imgui.Col_.border.value, _ACTIVE_COLOR)
					imgui.push_style_var(imgui.StyleVar_.frame_border_size.value, 2.0)
				clicked = imgui.button(f"##bg-color-{i}", size)
				if self._viewport_bg_active == i:
					imgui.pop_style_var()
					imgui.pop_style_color()
				imgui.pop_style_color(3)
				if clicked:
					self._set_viewport_background_color(i)
				if imgui.is_item_hovered():
					imgui.set_tooltip(labels[i])
				if i >= 3 and imgui.begin_popup_context_item(f"##bg-color-{i}-picker"):
					changed, new_color = imgui.color_edit3(f"##bg-color-{i}-edit", color[:3])
					if changed:
						self._viewport_bg_colors[i] = (new_color[0], new_color[1], new_color[2], 1.0)
						if self._viewport_bg_active == i:
							self.setBackgroundColor(*self._viewport_bg_colors[i])
					imgui.end_popup()
			self._background_color_bar_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _draw_viewport_toggles(self):
		"""Small floating icon-button bar bottom-left of the 3D viewport (same
		positioning pattern as navcube.py's draw_controls()): floor grid,
		world axes, object-pivot axes, 50% object transparency."""
		display_size = imgui.get_io().display_size
		win_h = display_size.y
		if win_h <= 0:
			return

		width, height = self._viewport_toggle_size
		x = self.explorer_width + _VIEWPORT_TOGGLE_MARGIN_PX
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - height
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##viewport-toggles", flags=flags):
			if _icon_button(fa_icons.ICON_FA_TABLE, "Floor grid (1m squares, sized to the object)",
			                self._grid_visible, square=True, large_font=large_font):
				self._toggle_grid()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_COMPASS, "World X/Y/Z axes",
			                self._world_axes_visible, square=True, large_font=large_font):
				self._toggle_world_axes()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_CROSSHAIRS, "Object pivot X/Y/Z axes",
			                self._pivot_axes_visible, square=True, large_font=large_font):
				self._toggle_pivot_axes()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_CIRCLE_HALF_STROKE, "50% object transparency",
			                self._object_transparent, square=True, large_font=large_font):
				self._toggle_object_transparency()
			imgui.same_line()
			wireframe_tooltip = f"{_WIREFRAME_MODE_LABELS[self._wireframe_mode]} (right-click to choose)"
			if _icon_button(fa_icons.ICON_FA_DRAW_POLYGON, wireframe_tooltip,
			                self._wireframe_mode != "off", square=True, large_font=large_font):
				self._cycle_object_wireframe()
			if imgui.begin_popup_context_item("##wireframe-mode-popup"):
				for mode in ("off", "overlay", "pure"):
					clicked, _ = imgui.selectable(_WIREFRAME_MODE_LABELS[mode], self._wireframe_mode == mode)
					if clicked:
						self._set_object_wireframe_mode(mode)
				imgui.end_popup()
			imgui.same_line()
			# Cycling button (project-todos/forgery/
			# object_editor__shading_modes.md, Nuno 2026-09-11's general
			# convention for every cycling button in Forgery): left-click
			# advances to the next state, right-click opens a popup to jump
			# straight to any state.
			shading_tooltip = f"{_SHADING_MODE_LABELS[self._shading_mode]} (right-click to choose)"
			if _icon_button(_SHADING_MODE_ICONS[self._shading_mode], shading_tooltip, self._shading_mode != "shading",
			                square=True, large_font=large_font):
				self._cycle_shading_mode()
			if imgui.begin_popup_context_item("##shading-mode-popup"):
				for mode in ("shading", "constant", "unshaded"):
					clicked, _ = imgui.selectable(_SHADING_MODE_LABELS[mode], self._shading_mode == mode)
					if clicked:
						self._set_shading_mode(mode)
				imgui.end_popup()
			if self._shading_mode == "unshaded":
				imgui.same_line()
				imgui.set_next_item_width(imgui.get_frame_height() * 2)
				changed, new_color = imgui.color_edit3("##unshaded-color", self._unshaded_color,
				                                        imgui.ColorEditFlags_.no_inputs.value)
				if changed:
					self._unshaded_color = tuple(new_color)
					self._apply_shading_mode()
			if self._shadow_skin_np is not None or self._shadow_skin_visible:
				imgui.same_line()
				shadow_animated = self._shadow_skin_state is not None and self._shadow_skin_state.bone_names is not None
				shadow_tooltip = (
					"CShadowSkin ground-shadow preview (animated to the current pose)"
					if shadow_animated else
					"CShadowSkin ground-shadow preview (static bind pose -- load a skeleton in Skinning "
					"preview to animate it)")
				if _icon_button(fa_icons.ICON_FA_CLONE, shadow_tooltip,
				                self._shadow_skin_visible, square=True, large_font=large_font):
					self._toggle_shadow_skin_preview()
			self._viewport_toggle_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _draw_panel_taskbar(self):
		"""Vertical icon bar, flush against the side panel's left edge --
		one square toggle per floating viewport panel (Wind, Skinning
		preview, Bind preview, Lighting), same _icon_button(square=True)
		convention as _draw_viewport_toggles()'s own bar, just stacked
		vertically (no same_line() between icons) instead of horizontally.
		A panel not applicable to the currently loaded shape gets its icon
		disabled rather than hidden, so this bar's own layout never jumps
		around as shapes are loaded/switched -- each _draw_..._controls()
		method still force-closes itself the moment it stops being
		applicable (see e.g. _draw_wind_controls()'s own comment)."""
		display_size = imgui.get_io().display_size
		if display_size.y <= 0:
			return

		wind_applicable = self._wind_state is not None
		bone_preview_applicable = self.shape_file is not None and _is_shape_skinned(self.shape_file.value)
		bind_applicable = self.shape_file is not None

		width, height = self._panel_taskbar_size
		x = display_size.x - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - width
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		large_font = (self.large_icon_font, self.large_icon_font_size) if self.large_icon_font is not None else None
		with imgui_ctx.begin("##panel-taskbar", flags=flags):
			if _icon_button(fa_icons.ICON_FA_WIND, "Wind preview", self._wind_panel_open,
			                square=True, large_font=large_font, disabled=not wind_applicable):
				self._wind_panel_open = not self._wind_panel_open
			if _icon_button(fa_icons.ICON_FA_BONE, "Skinning preview", self._bone_preview_panel_open,
			                square=True, large_font=large_font, disabled=not bone_preview_applicable):
				self._bone_preview_panel_open = not self._bone_preview_panel_open
			if _icon_button(fa_icons.ICON_FA_USERS, "Bind preview", self._bind_panel_open,
			                square=True, large_font=large_font, disabled=not bind_applicable):
				self._bind_panel_open = not self._bind_panel_open
			if _icon_button(fa_icons.ICON_FA_SUN, "Lighting", self._light_panel_open,
			                square=True, large_font=large_font):
				self._light_panel_open = not self._light_panel_open
			if _icon_button(fa_icons.ICON_FA_CIRCLE_INFO, "Shape info", self._info_panel_open,
			                square=True, large_font=large_font, disabled=self.shape_file is None):
				self._info_panel_open = not self._info_panel_open
			self._panel_taskbar_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _transform_node(self, prop):
		"""The NodePath position/rotation/scale editing for `prop`
		("position"/"rotation"/"scale") currently applies to -- _object_pivot
		by default (moves the object along with it, the existing Ctrl+drag
		behavior), or model_root itself when that row's pivot lock is on
		(see _draw_transform_panel()): edits then change the object's own
		local offset within the pivot, so the pivot's world transform stays
		put instead of moving with it."""
		return self.model_root if self.transform_locks[prop]["pivot"] else self._object_pivot

	def _get_transform_values(self, prop):
		"""Current (x, y, z) for `prop`, from whichever node _transform_node()
		says currently owns it. Rotation is Euler degrees around each world
		axis (NodePath.get_hpr()'s heading/pitch/roll are Z/X/Y respectively),
		not the raw quaternion -- matches the panel's X/Y/Z fields."""
		return self._node_transform_values(self._transform_node(prop), prop)

	def _node_transform_values(self, node, prop):
		"""Like _get_transform_values(), but for an explicit `node` instead
		of whichever one _transform_node() currently owns `prop` -- reset
		needs both _object_pivot's and model_root's own current values
		regardless of which one is presently the active pivot."""
		if prop == "position":
			v = node.get_pos()
			return (v.x, v.y, v.z)
		if prop == "rotation":
			h, p, r = node.get_hpr()
			return (p, r, h)
		v = node.get_scale()
		return (v.x, v.y, v.z)

	def _set_node_transform(self, node, prop, values):
		"""Sets `node`'s `prop` to `values` -- same (x, y, z)/(p, r, h)
		order as _node_transform_values()."""
		if prop == "position":
			node.set_pos(*values)
		elif prop == "rotation":
			p, r, h = values
			node.set_hpr(h, p, r)
		else:
			node.set_scale(*values)

	def _quat_to_prh(self, quat):
		"""(pitch, roll, heading) for `quat` -- same order _node_transform_values()
		uses for rotation. Panda's NodePath.get_hpr() has no quaternion
		equivalent that's proven safe here, so this goes through a scratch,
		unattached, never-rendered NodePath instead."""
		scratch = NodePath("scratch")
		scratch.set_quat(quat)
		h, p, r = scratch.get_hpr()
		return (p, r, h)

	def _reset_node_transform(self, node, prop, target):
		"""Resets `node`'s `prop` to `target` (same order as
		_node_transform_values()), except any axis currently locked
		(self.transform_locks[prop]), which keeps its current value instead
		-- the same per-axis lock _set_transform_axis() already enforces
		for manual edits and camera.py's Ctrl-drag."""
		locks = self.transform_locks[prop]
		current = self._node_transform_values(node, prop)
		values = [current[i] if locks["xyz"[i]] else target[i] for i in range(3)]
		self._set_node_transform(node, prop, values)

	def _set_transform_axis(self, prop, axis_index, value):
		"""Sets one axis (0=X, 1=Y, 2=Z) of `prop` to `value`, on whichever
		node _transform_node() currently owns it -- a no-op if that axis is
		locked (see _draw_transform_panel()); ObjectManipulator (camera.py)
		enforces the same lock for Ctrl+drag. For `prop == "scale"` with the
		Scale row's link toggle active (`self._scale_axes_linked`, see
		_draw_transform_row()), the other two (unlocked) scale axes are set
		to the same `value` too -- whichever axis is edited drives the other
		two, see project-todos/forgery/transform_panel_scale_link.md."""
		if self.transform_locks[prop]["xyz"[axis_index]]:
			return
		node = self._transform_node(prop)
		if prop == "position":
			values = list(node.get_pos())
			values[axis_index] = value
			node.set_pos(*values)
		elif prop == "rotation":
			h, p, r = node.get_hpr()
			values = [p, r, h]
			values[axis_index] = value
			p, r, h = values
			node.set_hpr(h, p, r)
		else:
			values = list(node.get_scale())
			values[axis_index] = value
			if self._scale_axes_linked:
				locks = self.transform_locks["scale"]
				for other_index, axis_name in enumerate("xyz"):
					if other_index != axis_index and not locks[axis_name]:
						values[other_index] = value
			node.set_scale(*values)

	def _reset_transform(self, prop):
		"""Resets `prop` to its default -- position/scale to identity,
		rotation to the shape's own baseline (_object_pivot_base_quat, see
		_display_shape()) -- on BOTH _object_pivot and model_root, regardless
		of which one that row's pivot lock currently targets, so the result
		always reads as a clean reset rather than only clearing whichever of
		the two happens to be locked right now. Any axis currently locked
		(see _draw_transform_panel()) keeps its current value instead of
		being reset, same as a manual edit on that axis would be a no-op.
		Used by reset_object_transform() (the gizmo's Home button, a
		deliberately total reset); the panel's own per-row Reset button uses
		_reset_transform_row() instead, which only touches the currently
		selected reference frame."""
		if prop == "position":
			pivot_target = model_target = (0, 0, 0)
		elif prop == "rotation":
			pivot_target = self._quat_to_prh(self._object_pivot_base_quat)
			model_target = (0, 0, 0)  # Quat()'s default constructor is the identity rotation
		else:
			pivot_target = model_target = (1, 1, 1)
		self._reset_node_transform(self._object_pivot, prop, pivot_target)
		self._reset_node_transform(self.model_root, prop, model_target)

	def _reset_transform_row(self, prop):
		"""Resets `prop` to its default -- position/scale to identity,
		rotation to the shape's own baseline (_object_pivot_base_quat) if
		that's the currently selected node, identity otherwise -- only on
		whichever node _transform_node() currently owns (pivot-locked or
		not), leaving the other reference frame untouched. Any axis
		currently locked keeps its current value instead, same as
		_reset_transform(). Bound to the panel's own per-row Reset button;
		see _reset_transform() for the gizmo's Home button, which resets
		both frames at once instead."""
		node = self._transform_node(prop)
		if prop == "position":
			target = (0, 0, 0)
		elif prop == "rotation":
			is_pivot = node is self._object_pivot
			target = self._quat_to_prh(self._object_pivot_base_quat) if is_pivot else (0, 0, 0)
		else:
			target = (1, 1, 1)
		self._reset_node_transform(node, prop, target)

	def reset_object_transform(self):
		"""Resets position, rotation, and scale together -- bound to the
		gizmo's Home button while the object is targeted (see
		navcube.py's draw_controls())."""
		for prop in ("position", "rotation", "scale"):
			self._reset_transform(prop)

	def _draw_transform_panel(self):
		"""Position/Rotation/Scale editor, floating in the viewport flush
		against the left edge of the navcube gizmo's own pixel rect, its
		bottom edge aligned with _draw_viewport_toggles()'s bottom-left icon
		bar (same y formula, own height). One row per property: a pivot-lock
		toggle, X/Y/Z axis-lock toggles, X/Y/Z value fields (live,
		see _get_transform_values()/_set_transform_axis()), and a
		row reset button (_reset_transform()). Hidden until a shape (with a
		pivot worth editing) is loaded."""
		if self.shape_file is None:
			return
		display_size = imgui.get_io().display_size
		win_h = display_size.y
		if win_h <= 0:
			return
		left, _right, _top, _bottom = self.nav_cube._panel_px
		width, height = self._transform_panel_size
		x = left - _VIEWPORT_TOGGLE_MARGIN_PX - width
		y = win_h - self.sysinfo_height - _VIEWPORT_TOGGLE_MARGIN_PX - height
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
		         | imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.no_title_bar.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("##transform-panel", flags=flags):
			# A skinned shape's own Default{Pos,RotQuat,Scale} has ZERO
			# effect in the real game (verified 2026-08-31,
			# transform.cpp:946 CTransform::updateWorldMatrixFromFather():
			# the whole _WorldMatrix = parentWM * _LocalMatrix composition
			# is skipped entirely when isSkinned() -- final vertex
			# positions come purely from skin binding to whatever skeleton
			# it's on). Editing/saving these here would silently do nothing
			# in-game, so the panel is grayed out rather than left looking
			# functional (2026-08-31, Nuno: "il faudrait alors que ce soit
			# grisé ou alors virer la fenetre d'edition des axes" -- after
			# confirming this the hard way on a skinned armor piece).
			is_skinned = _is_shape_skinned(self.shape_file.value)
			if is_skinned:
				imgui.text_disabled("Skinned shape: position/rotation/scale")
				imgui.text_disabled("have no effect in-game, editing disabled.")
			imgui.begin_disabled(is_skinned)
			self._draw_transform_row("position", "Pos", "%.3f")
			self._draw_transform_row("rotation", "Rot", "%.2f")
			self._draw_transform_row("scale", "Scl", "%.3f")
			imgui.end_disabled()
			self._transform_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _draw_transform_row(self, prop, label, value_format):
		"""One _draw_transform_panel() row -- see its own docstring. `label`
		is the row's own text label (not editable); `value_format` sets the
		X/Y/Z fields' display precision (position/scale: float32 is good to
		~7 significant digits, well past what's ever meaningful in meters or
		a scale multiplier here -- 3 decimals; rotation: 2 decimals, since a
		float32 quaternion's angular resolution is far finer than 0.01
		degrees already)."""
		imgui.push_id(f"xform-{prop}")
		locks = self.transform_locks[prop]

		imgui.text(label)
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_ANCHOR,
		                "Lock pivot: edits move the object within a fixed pivot, instead of moving the pivot itself",
		                locks["pivot"], square=True, active_color=_LOCKED_COLOR):
			locks["pivot"] = not locks["pivot"]

		values = list(self._get_transform_values(prop))
		for axis_index, axis_name in enumerate("XYZ"):
			axis_locked = locks[axis_name.lower()]
			imgui.same_line()
			if _icon_button(axis_name, f"Lock {axis_name} (never changes, drag or typed)",
			                axis_locked, square=True, active_color=_LOCKED_COLOR):
				locks[axis_name.lower()] = not axis_locked

			imgui.same_line()
			imgui.set_next_item_width(70)
			imgui.begin_disabled(axis_locked)
			v_speed, v_min, v_max = _TRANSFORM_DRAG_PARAMS[prop]
			changed, new_value = imgui.drag_float(
				f"##{prop}-{axis_name}", values[axis_index], v_speed=v_speed, v_min=v_min, v_max=v_max,
				format=value_format)
			imgui.end_disabled()
			if changed:
				self._set_transform_axis(prop, axis_index, new_value)

		if prop == "scale":
			imgui.same_line()
			linked = self._scale_axes_linked
			# Icon-only toggle, matching the pivot/axis lock buttons above --
			# see object_editor.py's own _scale_axes_linked docstring for what
			# it changes in _set_transform_axis(). Toggling it never touches
			# the current X/Y/Z values by itself, even if they already differ
			# (project-todos/forgery/transform_panel_scale_link.md's own
			# decision) -- alignment only happens on the next edited axis.
			icon = fa_icons.ICON_FA_LINK if linked else fa_icons.ICON_FA_LINK_SLASH
			if _icon_button(icon, "Link X/Y/Z: editing one axis sets the other two (unlocked) axes to match",
			                linked, square=True, active_color=_LOCKED_COLOR):
				self._scale_axes_linked = not linked

		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_ARROW_ROTATE_LEFT, f"Reset {label.lower()} (current reference frame only)", square=True):
			self._reset_transform_row(prop)

		imgui.pop_id()

	def _frame_camera(self):
		"""Frames the camera around everything currently visible: the main
		loaded shape (if any) plus, when shown, the assembled creature's
		rough extent -- from its skeleton's own bone world positions (see
		_rebuild_assembled_creature()'s _assembled_creature_bone_matrices),
		a coarse stand-in for a proper bbox of its skinned meshes but good
		enough for camera framing (union of bone positions always covers
		the actual mesh, since nothing skins further from its own bones
		than a reasonable creature's proportions). Called after either the
		main shape or the assembled creature changes -- was previously
		inline in _rebuild_geometry(), now also reused by
		_rebuild_assembled_creature() (2026-08-30: framing only ever
		considered the main object, so toggling the assembled creature on
		didn't bring it into view)."""
		mins = []
		maxs = []

		if self.shape_file is not None:
			bbox = shape_bbox(self.shape_file.value)
			if bbox is not None:
				# shape_bbox() is in model_root's own local space (pre-pivot-
				# rotation) -- go through model_root's current transform
				# (includes _object_pivot's rotation, e.g. a seeded
				# default_rot_quat) to get a world/render-space point.
				local_center = Point3(bbox.center.x, bbox.center.y, bbox.center.z)
				center = self.render.get_relative_point(self.model_root, local_center)
				half = bbox.half_size
				mins.append((center.x - half.x, center.y - half.y, center.z - half.z))
				maxs.append((center.x + half.x, center.y + half.y, center.z + half.z))

		if self._show_assembled_creature and self._assembled_creature_bone_matrices:
			pivot_pos = self._object_pivot.get_pos(self.render)
			xs = [m[0][3] + pivot_pos.x for m in self._assembled_creature_bone_matrices.values()]
			ys = [m[1][3] + pivot_pos.y for m in self._assembled_creature_bone_matrices.values()]
			zs = [m[2][3] + pivot_pos.z for m in self._assembled_creature_bone_matrices.values()]
			mins.append((min(xs), min(ys), min(zs)))
			maxs.append((max(xs), max(ys), max(zs)))

		if not mins:
			return

		min_x, min_y, min_z = (min(m[i] for m in mins) for i in range(3))
		max_x, max_y, max_z = (max(m[i] for m in maxs) for i in range(3))
		center = Point3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2)
		radius = max(max_x - min_x, max_y - min_y, max_z - min_z, 0.2) / 2
		self.orbit_camera.frame(center, radius * 5.65)

	def _rebuild_geometry(self):
		"""(Re)builds the 3D render passes + camera framing from
		self.shape_file.value -- shared by _display_shape() and by replacing
		a Mesh's geometry in place (which intentionally does NOT go through
		_reset_shape_state(), so material edits survive it)."""
		self.model_root.remove_node()
		self.model_root = self._object_pivot.attach_new_node("shape-root")
		self._apply_object_transparency()
		self._apply_object_wireframe()
		self._apply_shading_mode()
		self.shape_error = None
		self._material_node_paths = {}
		self._wind_state = None
		self._skin_state = None
		self._shadow_skin_state = None
		# Parented to self.render (world space, see the build block further
		# down for why), NOT model_root -- unlike model_root itself, it's not
		# torn down by the remove_node() above, so it must be removed here
		# explicitly before a fresh one is (maybe) built for the new shape.
		if self._shadow_skin_np is not None:
			self._shadow_skin_np.remove_node()
			self._shadow_skin_np = None
		self._texture_needs_repeat = False

		geom_value = shape_geom(self.shape_file.value)
		# CMeshMRMSkinnedGeom has no vertex_program field at all (no wind
		# animation support for skinned characters/creatures in NeL).
		wind_params = getattr(geom_value, "vertex_program", None)
		wind_params = wind_params if isinstance(wind_params, WindTreeParams) else None

		# CMeshMRMSkinned reuses the Skinning preview's own loaded skeleton/
		# animation (see _bone_world_matrices_for()), independent of any
		# attach bone chosen there. Without one loaded yet,
		# iter_render_passes() below still renders -- its own rigid bind-pose
		# fallback, matching the real client's behavior for an unskinned
		# CMeshMRMSkinned instance.
		is_skinned = _is_shape_skinned(self.shape_file.value)
		skeleton = self._bone_preview_skeleton if is_skinned else None
		bone_world_matrices = self._bone_world_matrices_for(geom_value.bones_name) if skeleton is not None else None
		# Live per-frame re-skin (_update_skin_preview()) supports
		# CMeshMRMSkinned's own packed-vertex format (_build_skin_state()) and
		# a plain skinned CMesh (_build_mesh_skin_state(), see the
		# "mesh_skinning_preview" chantier) -- a plain skinned CMeshMRM
		# (geom.skinned, see _is_shape_skinned()) still renders skinned below
		# via iter_render_passes(), just statically at the current pose
		# rather than re-skinned live every frame if the animation/time
		# changes. Extending the live path to that format too is future work.
		if isinstance(self.shape_file.value, MeshMRMSkinned) and skeleton is not None:
			self._skin_state = _build_skin_state(geom_value, skeleton)
		elif isinstance(self.shape_file.value, Mesh) and geom_value.skinned and skeleton is not None:
			self._skin_state = _build_mesh_skin_state(geom_value, skeleton)

		# CShadowSkin ground-shadow preview (opt-in, see
		# _toggle_shadow_skin_preview()) -- unlike self._skin_state above,
		# this does NOT require a skeleton: without one, it's shown flattened
		# onto the ground at its raw bind-pose position (no bone transform),
		# same fallback idea as iter_render_passes()' own rigid bind-pose
		# render further below. _shadow_skin_state (built with a skeleton
		# when one's loaded) is what makes it track the current pose live
		# every frame instead of staying static -- see
		# _build_shadow_skin_preview_state()'s own docstring. None whenever
		# geom.shadow_skin is empty.
		#
		# Parented to self.render (WORLD space), NOT model_root, unlike every
		# other per-shape geometry built in this method: _reskin_shadow_skin_state()
		# (the per-frame task) writes already-ground-projected WORLD
		# positions straight into this node's vdata -- parenting under
		# model_root would apply the object pivot's own transform ON TOP of
		# those, incorrectly moving/rotating a shadow that's supposed to stay
		# flat on the world ground plane regardless of how the loaded shape
		# itself is being rotated/positioned in the editor. Built here rather
		# than _rebuild_viewport_helpers() because its triangle list is
		# specific to the just-(re)loaded shape's own CShadowSkin, same as
		# self._skin_state.
		if isinstance(self.shape_file.value, MeshMRMSkinned) and geom_value.shadow_skin.vertices:
			self._shadow_skin_state = _build_shadow_skin_preview_state(geom_value, skeleton)
			shadow_vdata = _build_shadow_skin_vdata(geom_value.shadow_skin)
			self._shadow_skin_state.vdata = shadow_vdata
			shadow_node = _build_shadow_skin_ground_node(shadow_vdata, geom_value.shadow_skin)
			self._shadow_skin_np = self.render.attach_new_node(shadow_node)
			self._shadow_skin_np.set_light_off()
			self._shadow_skin_np.set_color(*_SHADOW_SKIN_COLOR)
			if not self._shadow_skin_visible:
				self._shadow_skin_np.hide()

		vdata = None
		pass_count = 0
		for vertex_buffer, material_id, indices in iter_render_passes(
				self.shape_file.value, skeleton=skeleton, bone_world_matrices=bone_world_matrices):
			if not indices:
				continue
			print(f"[object_editor] pass {pass_count}: material={material_id} "
			      f"verts={vertex_buffer.num_verts} tris={len(indices) // 3} "
			      f"channels={list(vertex_buffer.channels.keys())}")
			if vdata is None:
				# Built once, not per-pass: every pass indexes into the exact
				# same vertex array (see iter_render_passes()), and the wind/
				# skin preview needs a single vdata to update per frame rather
				# than one per pass.
				vdata = _build_vertex_data(vertex_buffer, dynamic=wind_params is not None or self._skin_state is not None)
				if wind_params is not None:
					self._wind_state = _build_wind_state(wind_params, vertex_buffer)
				self._texture_needs_repeat = _uvs_need_repeat(vertex_buffer.channels.get("TexCoord0"))
			geom = _build_geom(vdata, indices)
			geom_node = GeomNode(f"pass-{pass_count}")
			geom_node.add_geom(geom)
			node_path = self.model_root.attach_new_node(geom_node)
			self._material_node_paths.setdefault(material_id, []).append(node_path)
			self._apply_material(node_path, material_id)
			pass_count += 1

		if self._wind_state is not None:
			self._wind_state.vdata = vdata
		if self._skin_state is not None:
			self._skin_state.vdata = vdata

		if pass_count == 0:
			self.shape_error = f"No renderable 3D geometry for shape type {self.shape_file.type_name!r}"

		bbox = shape_bbox(self.shape_file.value)
		self._frame_camera()
		self._rebuild_viewport_helpers(bbox)

		self._rebuild_reference_shapes()
