"""Bottom-right 3D navigation gizmo for Ryzom Forgery tool apps.

A small cube, rendered in its own DisplayRegion, that mirrors the main
OrbitCamera's orientation in real time. Clicking a face snaps the main view
to look straight along that axis. Rotation itself is only ever driven by the
main viewport's own controls (drag, snap, 90-degree buttons) -- this gizmo
is a read-only mirror plus 6 click targets, it never rotates on its own.
"""

from math import cos, pi, radians, sin

from panda3d.core import (
	BitMask32, Camera, CollisionHandlerQueue, CollisionNode, CollisionPolygon,
	CollisionRay, CollisionTraverser, Geom, GeomNode, GeomTriangles, GeomVertexData,
	GeomVertexFormat, GeomVertexWriter, KeyboardButton, LineSegs, MouseButton, NodePath, PerspectiveLens, Point3,
	TextNode, TransparencyAttrib, Vec3,
)

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx

from .camera import object_targeted

_AXES = ("+x", "-x", "+y", "-y", "+z", "-z")

# (normal, u, v) basis per face, with u x v == normal (right-handed): used
# both to build each face's quad geometry and its collision polygon.
_FACE_BASIS = {
	"+x": (Vec3(1, 0, 0), Vec3(0, 1, 0), Vec3(0, 0, 1)),
	"-x": (Vec3(-1, 0, 0), Vec3(0, 0, 1), Vec3(0, 1, 0)),
	"+y": (Vec3(0, 1, 0), Vec3(0, 0, 1), Vec3(1, 0, 0)),
	"-y": (Vec3(0, -1, 0), Vec3(1, 0, 0), Vec3(0, 0, 1)),
	"+z": (Vec3(0, 0, 1), Vec3(1, 0, 0), Vec3(0, 1, 0)),
	"-z": (Vec3(0, 0, -1), Vec3(0, 1, 0), Vec3(1, 0, 0)),
}

_OUTER_FACE_ALPHA = 0.35  # the outer cube represents the scene -- see through it to the inner (object) cube
_FACE_COLOR = (0.28, 0.28, 0.31, _OUTER_FACE_ALPHA)
_FACE_COLOR_HOVER = (0.85, 0.65, 0.2, _OUTER_FACE_ALPHA)
_EDGE_COLOR = (1.0, 1.0, 1.0, 1.0)  # always retinted via set_color_scale() in _update(), see below
_HALF = 1.0

# Inner cube: represents the object being inspected (spun independently of
# the outer, scene-representing cube once object rotation is wired up), at
# half the outer cube's size so both stay clearly distinguishable.
_INNER_HALF = _HALF / 2.0
_INNER_FACE_COLOR = (1.0, 1.0, 1.0, 1.0)  # always retinted via set_color_scale(), see below
_INNER_EDGE_COLOR = (1.0, 1.0, 1.0, 1.0)  # always retinted via set_color_scale(), see below

# Which of the outer (scene/camera) or inner (object) cube is currently
# being acted on -- Ctrl held means the object -- is shown by swapping these
# two colors between the outer cube's edges and the inner cube's faces, via
# set_color_scale() rather than set_color(): LineSegs-built edge geometry
# turned out not to respond to a plain set_color() override (its baked-in
# per-vertex color always won), while set_color_scale() (a separate
# ColorScaleAttrib, multiplied in regardless of the underlying color source)
# reliably repaints it. Edges get a darkened version of whichever of the two
# is current, so they read as edges rather than blending into the faces.
_ACTIVE_COLOR = (0.95, 0.6, 0.15, 1.0)  # orange: whichever of the two is currently being manipulated
_INACTIVE_COLOR = (0.5, 0.75, 0.95, 1.0)  # blue: the other one
_EDGE_DARKEN = 0.6


def _darken(color, factor=_EDGE_DARKEN):
	r, g, b, a = color
	return (r * factor, g * factor, b * factor, a)

# Reference triad drawn through the cube: NeL/Ryzom convention, Z is up.
# Standard RGB-for-XYZ color coding so the axes read at a glance.
_AXIS_COLORS = {"x": (0.95, 0.25, 0.25, 1.0), "y": (0.3, 0.85, 0.3, 1.0), "z": (0.3, 0.55, 1.0, 1.0)}
_AXIS_VECTORS = {"x": Vec3(1, 0, 0), "y": Vec3(0, 1, 0), "z": Vec3(0, 0, 1)}
_AXIS_TIP_DISTANCE = _HALF * 1.6
# A cube corner sits sqrt(3) * _HALF from center; with a 30 degree FOV
# (15 degree half-angle, tan ~= 0.268) the cube only fits the frame with some
# margin once the camera is this far back -- closer than that and corners
# clip outside the DisplayRegion.
_CAM_DISTANCE = 9.5
_PANEL_SIZE_PX = 130
_PANEL_MARGIN_PX = 12
_PICK_MASK = BitMask32.bit(1)

_UI_GAP_PX = 6
# Auto-resize: the control bar's window sizes itself exactly to its buttons
# (real ImGui layout, no guessed widths/padding), and draw_controls() repositions
# it every frame from that measured size to keep it centered above the gizmo.
_UI_FLAGS = (
	imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_resize.value
	| imgui.WindowFlags_.no_title_bar.value | imgui.WindowFlags_.no_scrollbar.value
	| imgui.WindowFlags_.no_collapse.value | imgui.WindowFlags_.always_auto_resize.value)


def _face_corners(axis, half=_HALF):
	normal, u, v = _FACE_BASIS[axis]
	center = normal * half
	u, v = u * half, v * half
	# CCW winding as seen from the +normal side, so the face's own normal
	# (used both for shading and for the collision polygon's plane) faces
	# outward, away from the cube.
	return [center - u - v, center + u - v, center + u + v, center - u + v]


def _make_face_geom(axis, half=_HALF, color=_FACE_COLOR, name_prefix="navcube-face"):
	normal, _u, _v = _FACE_BASIS[axis]
	corners = _face_corners(axis, half)

	vformat = GeomVertexFormat.get_v3n3c4()
	vdata = GeomVertexData(f"{name_prefix}-{axis}", vformat, Geom.UH_static)
	vdata.set_num_rows(4)

	vertex_writer = GeomVertexWriter(vdata, "vertex")
	normal_writer = GeomVertexWriter(vdata, "normal")
	color_writer = GeomVertexWriter(vdata, "color")
	for corner in corners:
		vertex_writer.add_data3(corner)
		normal_writer.add_data3(normal)
		color_writer.add_data4(*color)

	triangles = GeomTriangles(Geom.UH_static)
	triangles.add_vertices(0, 1, 2)
	triangles.add_vertices(0, 2, 3)
	triangles.close_primitive()

	geom = Geom(vdata)
	geom.add_primitive(triangles)
	return geom


def _make_face_collision(axis, half=_HALF):
	return CollisionPolygon(*[Point3(corner) for corner in _face_corners(axis, half)])


def _draw_rotate_icon(draw_list, center, radius, color, clockwise):
	"""Draws a circular-arrow roll icon (an open arc + an arrowhead at its
	leading end) into an ImGui draw list. Hand-drawn rather than a font glyph
	-- Font Awesome 4 (the only icon font this project ships) has no
	dedicated rotate-left/rotate-right glyph. Angles are negated from the
	"natural" -50/230 degree sweep to mirror the icon vertically (ImGui's
	y-down screen space would otherwise draw it upside down relative to how
	a clock face reads)."""
	arc_start, arc_end = (radians(50.0), radians(-230.0)) if clockwise else (radians(-230.0), radians(50.0))
	draw_list.path_clear()
	draw_list.path_arc_to(center, radius, arc_start, arc_end, 16)
	draw_list.path_stroke(color, False, 1.6)

	tip_angle = arc_end
	# Points along the arc's own direction of travel at its tip, so the
	# arrowhead continues the curve instead of pointing back the way it
	# came. Derived from the arc's actual sweep direction (not just the
	# `clockwise` flag) so it can't drift out of sync if the arc bounds
	# above ever change again.
	sweep_sign = 1.0 if arc_end > arc_start else -1.0
	tangent = tip_angle + sweep_sign * (pi / 2.0)
	tip = (center[0] + radius * cos(tip_angle), center[1] + radius * sin(tip_angle))
	wing = radius * 0.7
	spread = radians(150.0)
	p1 = (tip[0] + wing * cos(tangent + spread), tip[1] + wing * sin(tangent + spread))
	p2 = (tip[0] + wing * cos(tangent - spread), tip[1] + wing * sin(tangent - spread))
	draw_list.add_triangle_filled(tip, p1, p2, color)


def _orbit_offset(heading, pitch, distance):
	"""Same offset formula as OrbitCamera._update_camera_pos(), so the
	gizmo's fixed-distance camera turns in lockstep with the real one."""
	h_rad, p_rad = radians(heading), radians(pitch)
	return Vec3(
		distance * cos(p_rad) * sin(h_rad),
		-distance * cos(p_rad) * cos(h_rad),
		distance * sin(p_rad),
	)


class NavigationCube:
	"""Small clickable 3D orientation gizmo rendered in its own DisplayRegion
	in the bottom-right corner, plus the reset/rotate-90 buttons that go with
	it. `draw_controls()` must be called once per frame from an app's
	draw_panel() to draw those buttons; the gizmo itself renders on its own
	via a Panda3D task, independent of ImGui.
	"""

	def __init__(self, app, orbit_camera, object_pivot):
		self.app = app
		self.orbit_camera = orbit_camera
		self.object_pivot = object_pivot  # mirrored live by the inner cube -- see _update()

		self.scene = NodePath("navcube-scene")
		self._face_nodes = {}
		for axis in _AXES:
			geom_node = GeomNode(f"navcube-face-{axis}")
			geom_node.add_geom(_make_face_geom(axis))
			face_np = self.scene.attach_new_node(geom_node)
			# The outer cube represents the scene, seen through to the inner
			# (object) cube -- see through() needs both the blend mode and a
			# sort/bin that draws it after opaque geometry, or the inner cube
			# (drawn later, same default bin) could show through it wrongly
			# depending on draw order; M_dual handles the depth-write side of
			# that (opaque-looking passes still write depth, the blended pass
			# doesn't) without needing a manual bin/sort here.
			face_np.set_transparency(TransparencyAttrib.M_dual)

			collision_node = CollisionNode(f"navcube-hit-{axis}")
			collision_node.add_solid(_make_face_collision(axis))
			collision_node.set_into_collide_mask(_PICK_MASK)
			collision_node.set_from_collide_mask(BitMask32.all_off())
			face_np.attach_new_node(collision_node)

			self._face_nodes[axis] = face_np

		self._build_inner_cube()

		self._outer_edges_np = self._build_edges()
		self._build_axes()
		self._build_axis_labels()

		lens = PerspectiveLens()
		lens.set_fov(30)
		camera_node = Camera("navcube-cam", lens)
		self.cam_np = self.scene.attach_new_node(camera_node)

		self.region = app.win.make_display_region(0, 0, 0, 0)
		# Below ShowBase's own 2D UI DisplayRegion (makeCamera2d()'s default
		# sort=10, where p3dimgui's overlay lives -- see backend.py, it
		# parents onto base.pixel2d), so this region -- despite being a
		# separate 3D DisplayRegion, not part of the ImGui draw list at all
		# -- renders BEFORE the UI and never paints over ImGui windows or
		# tooltips that happen to overlap its rect (e.g. draw_controls()'s
		# button bar, positioned right above it).
		self.region.set_sort(5)
		self.region.set_clear_color_active(True)
		self.region.set_clear_color((0.0, 0.0, 0.0, 1.0))
		self.region.set_clear_depth_active(True)
		self.region.set_camera(self.cam_np)

		self._traverser = CollisionTraverser("navcube-picker")
		self._pick_queue = CollisionHandlerQueue()
		self._ray = CollisionRay()
		picker_node = CollisionNode("navcube-picker-ray")
		picker_node.add_solid(self._ray)
		picker_node.set_from_collide_mask(_PICK_MASK)
		picker_node.set_into_collide_mask(BitMask32.all_off())
		picker_np = self.cam_np.attach_new_node(picker_node)
		self._traverser.add_collider(picker_np, self._pick_queue)

		# Pixel rect of the gizmo's panel: (left, right, top, bottom), y
		# measured from the window's top like Panda3D's own pointer coords.
		# Recomputed every frame in _update() since the window can resize.
		self._panel_px = (0, 0, 0, 0)
		self._hovered_axis = None
		# Measured size of the control bar window, from the previous frame
		# it was drawn (see draw_controls()) -- seeded with a rough guess so
		# it's already close on the very first frame.
		self._controls_size = (150.0, 26.0)

		app.taskMgr.add(self._update, "navcube-update")
		app.accept("mouse1", self._on_click)

	def _build_inner_cube(self):
		"""Smaller, fully opaque cube inside the (now semi-transparent) outer
		one: the outer cube represents the scene/viewing angle (what the
		gizmo has always shown), the inner one will represent the object
		being inspected's own rotation once that's wired up separately from
		the camera. `self.inner_np` is the pivot to rotate for that -- not
		rotated by anything yet, just built here. Faces and edges are kept
		as two separate NodePaths (`_inner_faces_np`/`_inner_edges_np`) so
		_update() can tint them two different shades (edges darker) instead
		of one flat color for the whole cube."""
		self.inner_np = self.scene.attach_new_node("navcube-inner")

		self._inner_faces_np = self.inner_np.attach_new_node("navcube-inner-faces")
		for axis in _AXES:
			geom_node = GeomNode(f"navcube-inner-face-{axis}")
			geom_node.add_geom(_make_face_geom(axis, half=_INNER_HALF, color=_INNER_FACE_COLOR, name_prefix="navcube-inner-face"))
			face_np = self._inner_faces_np.attach_new_node(geom_node)
			face_np.set_light_off()

		self._inner_edges_np = self._build_edges(
			half=_INNER_HALF, color=_INNER_EDGE_COLOR, name="navcube-inner-edges", parent=self.inner_np)

	def _build_edges(self, half=_HALF, color=_EDGE_COLOR, name="navcube-edges", parent=None):
		verts = [Vec3(sx, sy, sz) for sx in (-half, half) for sy in (-half, half) for sz in (-half, half)]
		# Index into the 8 corners above (order: x is the slowest-varying bit,
		# then y, then z), listing the 12 edges of the cube.
		edge_pairs = [
			(0, 1), (2, 3), (4, 5), (6, 7),  # edges along z
			(0, 2), (1, 3), (4, 6), (5, 7),  # edges along y
			(0, 4), (1, 5), (2, 6), (3, 7),  # edges along x
		]

		lines = LineSegs(name)
		lines.set_thickness(1.5)
		lines.set_color(*color)
		for a, b in edge_pairs:
			lines.move_to(verts[a])
			lines.draw_to(verts[b])

		edges_np = (parent or self.scene).attach_new_node(lines.create())
		edges_np.set_light_off()
		# Nudged slightly outward so the wireframe doesn't z-fight with the
		# face quads it sits right on top of.
		edges_np.set_scale(1.01)
		return edges_np

	def _build_axes(self):
		# Positive half only (center -> tip): the negative half would just
		# be redundant sticks poking out the opposite, already-labelled
		# side of the cube.
		lines = LineSegs("navcube-axes")
		lines.set_thickness(2.0)
		for axis, vector in _AXIS_VECTORS.items():
			lines.set_color(*_AXIS_COLORS[axis])
			lines.move_to(Point3(0, 0, 0))
			lines.draw_to(vector * _AXIS_TIP_DISTANCE)

		axes_np = self.scene.attach_new_node(lines.create())
		axes_np.set_light_off()

	def _build_axis_labels(self):
		"""X/Y/Z letters at each axis tip, drawn right in the 3D gizmo scene
		(not as an ImGui overlay -- that requires projecting through the
		lens every frame and turned out unreliable). Billboarded to the
		gizmo's own camera so they stay readable as it orbits."""
		for axis, vector in _AXIS_VECTORS.items():
			text_node = TextNode(f"navcube-label-{axis}")
			text_node.set_text(axis.upper())
			text_node.set_align(TextNode.A_center)
			text_node.set_text_color(*_AXIS_COLORS[axis])

			label_np = self.scene.attach_new_node(text_node)
			label_np.set_scale(0.5)
			label_np.set_pos(vector * (_AXIS_TIP_DISTANCE + 0.25))
			label_np.set_billboard_point_eye()
			label_np.set_light_off()
			label_np.set_depth_write(False)
			label_np.set_bin("fixed", 10)

	def _layout(self):
		# ImGui windows (the button bar, Explorer, Panel, ...) are all
		# positioned in imgui.get_io().display_size units -- using the raw
		# window/framebuffer size (win.get_x_size()) here instead would
		# silently disagree with that under any DPI scaling, throwing the
		# button bar and the gizmo's own DisplayRegion out of alignment.
		display_size = imgui.get_io().display_size
		win_w, win_h = display_size.x, display_size.y
		if win_w <= 0 or win_h <= 0:
			return

		size = _PANEL_SIZE_PX
		bottom_px = win_h - self.app.sysinfo_height - _PANEL_MARGIN_PX
		top_px = bottom_px - size
		# Anchored to the right edge of the 3D viewport (i.e. the left edge
		# of the right-hand panel), not the window's right edge, so it stays
		# over the viewport instead of the panel as the panel gets resized.
		right_px = win_w - self.app.panel_width - _PANEL_MARGIN_PX
		left_px = right_px - size
		self._panel_px = (left_px, right_px, top_px, bottom_px)

		# DisplayRegion dimensions are fractions of the window with the
		# origin at the bottom-left, unlike the pixel rect above (top-left
		# origin) -- hence the 1 - .../win_h flips.
		self.region.set_dimensions(
			left_px / win_w, right_px / win_w,
			1.0 - bottom_px / win_h, 1.0 - top_px / win_h)

	def _update(self, task):
		self._layout()

		heading, pitch = self.orbit_camera.heading, self.orbit_camera.pitch
		self.cam_np.set_pos(_orbit_offset(heading, pitch, _CAM_DISTANCE))
		# Mirror the main camera's up_hint too, not just heading/pitch, so the
		# two can't silently disagree if up_hint is ever driven away from
		# world Z by something other than snap_to_axis()/reset().
		self.cam_np.look_at(Point3(0, 0, 0), self.orbit_camera.up_hint)

		# Mirrors the object's actual current orientation (whatever
		# object_editor.py's _object_pivot is at -- seeded from the shape's
		# own default_rot_quat on load, then spun further via Ctrl+drag).
		self.inner_np.set_quat(self.object_pivot.get_quat())

		self._hovered_axis = None if self._mouse_captured_by_ui() else self._pick_axis_at_mouse()
		for axis, face_np in self._face_nodes.items():
			face_np.set_color(_FACE_COLOR_HOVER if axis == self._hovered_axis else _FACE_COLOR)

		# Ctrl held (or app.target_mode forcing one or the other, see the
		# mode icon in draw_controls()) means a drag acts on the object
		# (inner cube) instead of the camera (outer cube) -- see
		# ObjectManipulator/OrbitCamera in camera.py. Swapping which of the
		# two reads as "active" (orange) is the only way to tell which one a
		# drag is about to affect.
		object_active = object_targeted(self.app)
		outer_color = _INACTIVE_COLOR if object_active else _ACTIVE_COLOR
		inner_color = _ACTIVE_COLOR if object_active else _INACTIVE_COLOR
		self._outer_edges_np.set_color_scale(outer_color)
		self._inner_faces_np.set_color_scale(inner_color)
		self._inner_edges_np.set_color_scale(_darken(inner_color))

		return task.cont

	def _mouse_captured_by_ui(self):
		imgui_backend = getattr(self.app, "imgui", None)
		return imgui_backend is not None and imgui_backend.isMouseCaptured()

	def _pointer_in_panel(self):
		if not self.app.mouseWatcherNode.hasMouse():
			return None
		pointer = self.app.win.get_pointer(0)
		mx, my = pointer.get_x(), pointer.get_y()
		left, right, top, bottom = self._panel_px
		if not (left <= mx <= right and top <= my <= bottom):
			return None
		local_x = (mx - left) / (right - left) * 2.0 - 1.0
		local_y = -((my - top) / (bottom - top) * 2.0 - 1.0)
		return local_x, local_y

	def _pick_axis_at_mouse(self):
		local = self._pointer_in_panel()
		if local is None:
			return None

		self._ray.set_from_lens(self.cam_np.node(), local[0], local[1])
		self._traverser.traverse(self.scene)
		if self._pick_queue.get_num_entries() == 0:
			return None

		self._pick_queue.sort_entries()
		hit_name = self._pick_queue.get_entry(0).get_into_node().get_name()
		return hit_name.replace("navcube-hit-", "")

	def _on_click(self):
		if self._mouse_captured_by_ui():
			return
		axis = self._pick_axis_at_mouse()
		if axis is not None:
			self.orbit_camera.snap_to_axis(axis)

	def _icon_button(self, icon, tooltip):
		"""A square, auto-sized (to match a normal text button's height)
		icon-only button with a hover tooltip -- Font Awesome glyph, already
		merged into ImGui's default font by ForgeryApp._load_icon_font() by
		the time any tool app constructs a NavigationCube."""
		size = imgui.get_frame_height()
		clicked = imgui.button(icon, (size, size))
		if imgui.is_item_hovered():
			imgui.set_tooltip(tooltip)
		return clicked

	def _roll_button(self, clockwise):
		"""Square icon button showing a hand-drawn circular-arrow roll icon
		(see _draw_rotate_icon()) instead of a font glyph."""
		size = imgui.get_frame_height()
		clicked = imgui.button("##roll-cw" if clockwise else "##roll-ccw", (size, size))
		rect_min, rect_max = imgui.get_item_rect_min(), imgui.get_item_rect_max()
		center = ((rect_min.x + rect_max.x) / 2.0, (rect_min.y + rect_max.y) / 2.0)
		color = imgui.get_color_u32(imgui.Col_.text.value)
		_draw_rotate_icon(imgui.get_window_draw_list(), center, size / 2.0 - 5.0, color, clockwise)
		if imgui.is_item_hovered():
			imgui.set_tooltip("Roll view 90° clockwise" if clockwise else "Roll view 90° counter-clockwise")
		return clicked

	def _draw_status_icon(self):
		"""Small clickable square, bottom-right of the directional pad,
		showing which mouse-drag mode is currently active: a pointer by
		default, or rotate/move/scale while the corresponding mouse button is
		held -- matching OrbitCamera/ObjectManipulator's own left/middle/right
		drag bindings (Ctrl or not; either way the same button does the same
		kind of action, so it's not distinguished here). Clicking it cycles
		app.forced_drag_mode (pointer -> move -> rotate -> scale -> pointer);
		while forced (icon turned orange), OrbitCamera/ObjectManipulator let
		a plain left-click-drag alone perform that action instead of needing
		the matching button held, in whichever of the two contexts (Ctrl or
		not) is currently active."""
		size = imgui.get_frame_height()
		forced = self.app.forced_drag_mode

		clicked = imgui.button("##drag-mode-status", (size, size))
		if clicked:
			self.app.forced_drag_mode = {None: "move", "move": "rotate", "rotate": "scale"}.get(forced)

		if forced is not None:
			mode = forced
		else:
			mode = None
			mw = self.app.mouseWatcherNode
			imgui_backend = getattr(self.app, "imgui", None)
			captured = imgui_backend is not None and imgui_backend.isMouseCaptured()
			if not captured and mw.hasMouse():
				if mw.isButtonDown(MouseButton.one()):
					mode = "rotate"
				elif mw.isButtonDown(MouseButton.two()):
					mode = "move"
				elif mw.isButtonDown(MouseButton.three()):
					mode = "scale"

		rect_min, rect_max = imgui.get_item_rect_min(), imgui.get_item_rect_max()
		center = ((rect_min.x + rect_max.x) / 2.0, (rect_min.y + rect_max.y) / 2.0)
		draw_list = imgui.get_window_draw_list()
		color = imgui.get_color_u32(_ACTIVE_COLOR if forced is not None else imgui.Col_.text.value)
		if mode == "rotate":
			# No dedicated Font Awesome 4 rotate glyph -- see _draw_rotate_icon().
			_draw_rotate_icon(draw_list, center, size / 2.0 - 5.0, color, clockwise=True)
		else:
			icon = {
				"move": fa_icons.ICON_FA_ARROWS_ALT,
				"scale": fa_icons.ICON_FA_EXPAND,
			}.get(mode, fa_icons.ICON_FA_MOUSE_POINTER)
			text_size = imgui.calc_text_size(icon)
			draw_list.add_text((center[0] - text_size.x / 2.0, center[1] - text_size.y / 2.0), color, icon)
		if imgui.is_item_hovered():
			if forced is None:
				imgui.set_tooltip("Current drag mode: left = rotate, middle = move, right = scale\nClick to force a mode (plain left-click alone)")
			else:
				imgui.set_tooltip(f"Forced drag mode: {forced} (plain left-click alone)\nClick to cycle, or back to pointer to restore normal left/middle/right")

	def _draw_mode_icon(self):
		"""Small clickable square, bottom-left of the directional pad
		(mirroring _draw_status_icon()'s bottom-right placement), showing
		and controlling app.target_mode: whether a drag targets the camera
		or the object. Clicking it cycles Ctrl-decides (keyboard icon by
		default, white) -> camera (orange) -> object (orange) -> Ctrl-decides;
		forced to one or the other, every drag acts on that one regardless
		of Ctrl (see camera.py's object_targeted(), which both
		OrbitCamera/ObjectManipulator and this gizmo's own cube colors
		read). While Ctrl-decides, the icon still reflects Ctrl's live state
		(camera/object glyph, but white, not orange -- not forced, just
		showing what Ctrl currently means)."""
		size = imgui.get_frame_height()
		mode = self.app.target_mode

		clicked = imgui.button("##target-mode", (size, size))
		if clicked:
			self.app.target_mode = {None: "camera", "camera": "object"}.get(mode)

		if mode is not None:
			live_mode = mode
		else:
			live_mode = "object" if self.app.mouseWatcherNode.isButtonDown(KeyboardButton.control()) else "camera"

		rect_min, rect_max = imgui.get_item_rect_min(), imgui.get_item_rect_max()
		center = ((rect_min.x + rect_max.x) / 2.0, (rect_min.y + rect_max.y) / 2.0)
		draw_list = imgui.get_window_draw_list()
		color = imgui.get_color_u32(_ACTIVE_COLOR if mode is not None else imgui.Col_.text.value)
		icon = {"camera": fa_icons.ICON_FA_CAMERA, "object": fa_icons.ICON_FA_CUBE}.get(live_mode, fa_icons.ICON_FA_KEYBOARD)
		text_size = imgui.calc_text_size(icon)
		draw_list.add_text((center[0] - text_size.x / 2.0, center[1] - text_size.y / 2.0), color, icon)
		if imgui.is_item_hovered():
			if mode is None:
				imgui.set_tooltip("Drag target: Ctrl held decides camera/object\nClick to force one")
			else:
				imgui.set_tooltip(f"Drag target forced to: {mode}\nClick to cycle, or back to keyboard icon for Ctrl to decide")

	def draw_controls(self):
		"""Draws the reset-view / face-step / roll buttons above the gizmo
		(the XYZ legend is drawn straight into the 3D gizmo scene itself, see
		_build_axis_labels()), laid out as --
		     ccw  ^   cw
		     <  HOME  >
		  [mode] v  [status]
		Each arrow snaps to the adjacent face in that direction (always lands
		on one of the 6 axis-aligned views, see OrbitCamera.step_to_face());
		the 2 top corners instead roll the current face view 90°
		(OrbitCamera.roll_step()) without changing which face is shown. The
		bottom-left/right icons are the odd ones out -- [mode]
		(_draw_mode_icon()) toggles app.target_mode (camera/object drag
		target), [status] (_draw_status_icon()) shows/forces
		app.forced_drag_mode; see their own docstrings.
		Call once per frame from draw_panel() -- this is a separate floating
		(borderless), auto-sized window positioned from the gizmo's own pixel
		rect, not part of the docked panel's own layout."""
		left, right, top, _bottom = self._panel_px
		center_x = (left + right) / 2.0
		width, height = self._controls_size
		imgui.set_next_window_pos((center_x - width / 2.0, top - height - _UI_GAP_PX))
		with imgui_ctx.begin("##navcube-controls", flags=_UI_FLAGS):
			# Held-down buttons call the OrbitCamera step every frame (via
			# is_item_active(), true continuously while the mouse is down on
			# the item) rather than relying on ImGui's button_repeat -- that
			# fires clicks on its own fixed timer, independent of how long
			# OrbitCamera's own step animation takes, which could leave a
			# stutter of a frame or more between one step finishing and the
			# next repeat-fired click arriving. step_to_face()/roll_step()
			# already no-op while a step is still in flight, so polling every
			# frame just means the next step starts on the exact frame the
			# previous one finishes, however long that took.
			self._roll_button(clockwise=False)
			if imgui.is_item_active():
				self.orbit_camera.roll_step(-1)
			imgui.same_line()
			self._icon_button(fa_icons.ICON_FA_ARROW_UP, "Show the face above")
			if imgui.is_item_active():
				self.orbit_camera.step_to_face("up")
			imgui.same_line()
			self._roll_button(clockwise=True)
			if imgui.is_item_active():
				self.orbit_camera.roll_step(1)

			# Row 2: Left, Home (reset), Right.
			self._icon_button(fa_icons.ICON_FA_ARROW_LEFT, "Show the face to the left")
			if imgui.is_item_active():
				self.orbit_camera.step_to_face("left")
			imgui.same_line()
			# Ctrl held (or app.target_mode forcing one or the other) means
			# Home targets the object instead of the camera -- mirrors the
			# same modifier ObjectManipulator/OrbitCamera use to pick which
			# one a drag acts on.
			if self._icon_button(fa_icons.ICON_FA_HOME, "Reset view (Ctrl: reset object rotation)"):
				if object_targeted(self.app):
					self.app.reset_object_rotation()
				else:
					self.orbit_camera.reset()
			imgui.same_line()
			self._icon_button(fa_icons.ICON_FA_ARROW_RIGHT, "Show the face to the right")
			if imgui.is_item_active():
				self.orbit_camera.step_to_face("right")

			# Row 3: drag-target mode icon, Down, drag-mode status icon --
			# same 3 columns as rows 1-2, so unlike those two icons' first
			# appearance (before this row had a real bottom-left button),
			# no manual indent is needed to center Down under the middle
			# column anymore.
			self._draw_mode_icon()
			imgui.same_line()
			self._icon_button(fa_icons.ICON_FA_ARROW_DOWN, "Show the face below")
			if imgui.is_item_active():
				self.orbit_camera.step_to_face("down")
			imgui.same_line()
			self._draw_status_icon()

			window_size = imgui.get_window_size()
			self._controls_size = (window_size.x, window_size.y)
