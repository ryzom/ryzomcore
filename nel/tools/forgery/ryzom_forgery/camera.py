from math import asin, atan2, cos, degrees, radians, sin, tan

from panda3d.core import KeyboardButton, MouseButton, Point3, Quat, Vec3

# (heading, pitch) for each world axis, derived from _update_camera_pos()'s
# offset formula: heading=0/pitch=0 looks from -Y towards the target (the
# controller's own default), the rest follow the same right-handed, Z-up
# convention as the rest of the NeL/Ryzom tooling.
AXIS_VIEWS = {
	"+x": (90.0, 0.0),
	"-x": (-90.0, 0.0),
	"+y": (180.0, 0.0),
	"-y": (0.0, 0.0),
	"+z": (0.0, 90.0),
	"-z": (0.0, -90.0),
}

# Navigation-cube face label for each axis view (Ryzom Studio / 3ds Max naming).
AXIS_LABELS = {
	"+x": "RIGHT",
	"-x": "LEFT",
	"+y": "BACK",
	"-y": "FRONT",
	"+z": "TOP",
	"-z": "BOTTOM",
}


class OrbitCamera:
	"""Blender-style orbit camera controller.

	Left-mouse drag orbits around the target, middle-mouse drag pans the
	target freely, mouse wheel always zooms. Input is ignored while Dear
	ImGui wants the mouse (e.g. dragging a slider), so tool UI and viewport
	input don't fight over the same drag.
	"""

	def __init__(self, app, target=Point3(0, 0, 0), distance=20.0, heading=0.0, pitch=0.0):
		self.app = app
		self.target = Point3(target)
		self.distance = distance
		self.heading = heading
		self.pitch = pitch
		# What lookAt() treats as "up" when placing the camera. Normally the
		# world Z axis (giving the usual orbit-camera behaviour, always as
		# level as the current pitch allows), but rotate_step() overrides it to
		# the axis it just rotated around -- otherwise lookAt() would
		# re-derive its own "closest to world Z" up for the new position on
		# every call, silently levelling off whatever tilt rotate_step() was
		# trying to preserve.
		self.up_hint = Vec3(0, 0, 1)

		self.min_distance = 0.5
		self.max_distance = 2000.0
		self.min_pitch = -89.0
		self.max_pitch = 89.0

		self.orbit_speed = 200.0  # degrees per full mouse-width drag
		self.zoom_speed = 0.9  # multiplier applied to distance per wheel notch

		# Framing (target/distance) restored by reset() -- frame() keeps this
		# up to date with whatever object is loaded. Heading/pitch/up_hint
		# are NOT part of this: reset() always goes to a level front view
		# (see AXIS_VIEWS["-y"]) regardless of the view the object happened
		# to load with.
		self._default_target = Point3(self.target)
		self._default_distance = self.distance

		self._last_mouse = None

		# Panda3D prefixes button events with the held modifier (e.g.
		# "shift-wheel_up"), so the plain event alone would go silent
		# whenever shift is held for panning.
		self.app.accept("wheel_up", self._zoom, [1])
		self.app.accept("wheel_down", self._zoom, [-1])
		self.app.accept("shift-wheel_up", self._zoom, [1])
		self.app.accept("shift-wheel_down", self._zoom, [-1])
		self.app.taskMgr.add(self._update, "orbit-camera-update")

		self._update_camera_pos()

	def _mouse_captured_by_ui(self):
		imgui = getattr(self.app, "imgui", None)
		return imgui is not None and imgui.isMouseCaptured()

	def _zoom(self, direction):
		if self._mouse_captured_by_ui():
			return
		factor = self.zoom_speed if direction > 0 else (1.0 / self.zoom_speed)
		self.distance = max(self.min_distance, min(self.max_distance, self.distance * factor))
		self._update_camera_pos()

	def _update(self, task):
		mw = self.app.mouseWatcherNode
		# Ctrl held means Ctrl+drag acts on the object instead (see
		# ObjectManipulator below) -- without this, both would respond to
		# the same drag at once.
		if not mw.hasMouse() or self._mouse_captured_by_ui() or mw.isButtonDown(KeyboardButton.control()):
			self._last_mouse = None
			return task.cont

		mouse = mw.getMouse()
		mouse_pos = (mouse.getX(), mouse.getY())
		orbit_down = mw.isButtonDown(MouseButton.one())
		pan_down = mw.isButtonDown(MouseButton.two())
		dragging = orbit_down or pan_down

		if dragging and self._last_mouse is not None:
			dx = mouse_pos[0] - self._last_mouse[0]
			dy = mouse_pos[1] - self._last_mouse[1]
			if pan_down:
				self._pan(dx, dy)
			else:
				self._orbit(dx, dy)

		self._last_mouse = mouse_pos if dragging else None

		return task.cont

	def _orbit(self, dx, dy):
		self.heading -= dx * self.orbit_speed
		self.pitch = max(self.min_pitch, min(self.max_pitch, self.pitch + dy * self.orbit_speed))
		self._update_camera_pos()

	def _pan(self, dx, dy):
		# Exact pixel tracking: a world point at `distance` from the camera
		# must stay under the cursor for the whole drag. The world span
		# covered by the full [-1, 1] mouse range at that distance is
		# 2 * distance * tan(fov / 2), so half of that (i.e. no extra
		# factor of 2) is the world movement per unit of mouse delta.
		quat = self.app.camera.getQuat(self.app.render)
		fov = self.app.camLens.getFov()
		right_scale = self.distance * tan(radians(fov.x / 2))
		up_scale = self.distance * tan(radians(fov.y / 2))
		self.target -= quat.getRight() * dx * right_scale
		self.target -= quat.getUp() * dy * up_scale
		self._update_camera_pos()

	def frame(self, target, distance):
		"""Point the camera at `target` from `distance` away, keeping the
		current heading/pitch. Useful to frame a newly loaded object."""
		self.target = Point3(target)
		self.distance = max(self.min_distance, min(self.max_distance, distance))
		self._default_target = Point3(self.target)
		self._default_distance = self.distance
		self._update_camera_pos()

	def reset(self):
		"""Restore the last frame()-set framing (target/distance), looking
		from a level front view -- i.e. how a Ryzom object is conventionally
		viewed face-on -- not whatever heading/pitch the view happened to be
		at when the object was loaded."""
		self.target = Point3(self._default_target)
		self.distance = self._default_distance
		self.heading, self.pitch = AXIS_VIEWS["-y"]
		self.up_hint = Vec3(0, 0, 1)
		self._update_camera_pos()

	def rotate_step(self, direction, degrees_step=5.0):
		"""Rotate the view by a small step around the screen's current
		vertical axis (the camera's own up vector) -- not the world Z axis,
		so it stays correct at any pitch, and not the view's forward axis
		(that would just spin the image in place instead of orbiting).
		Only heading/pitch/up_hint change here -- target and distance (i.e.
		framing and zoom) are left untouched, this must be a pure rotation."""
		h_rad, p_rad = radians(self.heading), radians(self.pitch)
		offset = Vec3(
			self.distance * cos(p_rad) * sin(h_rad),
			-self.distance * cos(p_rad) * cos(h_rad),
			self.distance * sin(p_rad),
		)

		up_axis = self.app.camera.getQuat(self.app.render).getUp()
		rotation = Quat()
		rotation.setFromAxisAngle(degrees_step * direction, up_axis)
		new_offset = rotation.xform(offset)

		self.pitch = max(self.min_pitch, min(self.max_pitch, degrees(asin(new_offset.z / self.distance))))
		self.heading = degrees(atan2(new_offset.x, -new_offset.y))
		# A vector rotated around its own axis maps to itself, so up_axis is
		# still exactly the right "up" for the new position -- passing it
		# to lookAt() below is what keeps the current tilt instead of
		# levelling back off to world Z.
		self.up_hint = up_axis
		self._update_camera_pos()

	def snap_to_axis(self, axis):
		"""Snap the view to look straight along one of the 6 world axes
		(+x/-x/+y/-y/+z/-z), keeping the current target/distance."""
		heading, pitch = AXIS_VIEWS[axis]
		self.heading = heading
		self.pitch = pitch
		self.up_hint = Vec3(0, 0, 1)
		self._update_camera_pos()

	def _update_camera_pos(self):
		h_rad = radians(self.heading)
		p_rad = radians(self.pitch)
		offset = (
			self.distance * cos(p_rad) * sin(h_rad),
			-self.distance * cos(p_rad) * cos(h_rad),
			self.distance * sin(p_rad),
		)
		self.app.camera.setPos(self.target + offset)
		self.app.camera.lookAt(self.target, self.up_hint)


class ObjectManipulator:
	"""Rotates/moves a NodePath (the shape being inspected) via Ctrl+drag --
	Ctrl+left-mouse spins it in place, Ctrl+middle-mouse moves it in world
	space, tracking the cursor the same screen-space-accurate way
	OrbitCamera's own pan does. Mutually exclusive with OrbitCamera's plain
	(no-Ctrl) drag controls -- both check for the Ctrl modifier so only one
	responds to a given drag.

	Rotation is applied as an incremental quaternion multiplied onto
	whatever orientation the pivot already has, rather than tracked as a
	separate heading/pitch pair set via setHpr() -- the pivot may already be
	seeded with the shape's own CMaterial::DefaultRotQuat (object_editor.py's
	_display_shape()), which can have a real roll component (an object tilted
	sideways, not just turned or tipped), and setHpr(h, p, 0) would silently
	discard that on the first drag."""

	def __init__(self, app, pivot, orbit_camera):
		self.app = app
		self.pivot = pivot
		self.orbit_camera = orbit_camera  # only read for its .distance, to scale _move() the same way OrbitCamera._pan() does

		self.rotate_speed = 200.0  # degrees per full mouse-width drag, matches OrbitCamera.orbit_speed

		self._last_mouse = None
		self.app.taskMgr.add(self._update, "object-manipulator-update")

	def _mouse_captured_by_ui(self):
		imgui = getattr(self.app, "imgui", None)
		return imgui is not None and imgui.isMouseCaptured()

	def _update(self, task):
		mw = self.app.mouseWatcherNode
		if not mw.hasMouse() or self._mouse_captured_by_ui() or not mw.isButtonDown(KeyboardButton.control()):
			self._last_mouse = None
			return task.cont

		mouse = mw.getMouse()
		mouse_pos = (mouse.getX(), mouse.getY())
		rotate_down = mw.isButtonDown(MouseButton.one())
		move_down = mw.isButtonDown(MouseButton.two())
		dragging = rotate_down or move_down

		if dragging and self._last_mouse is not None:
			dx = mouse_pos[0] - self._last_mouse[0]
			dy = mouse_pos[1] - self._last_mouse[1]
			if move_down:
				self._move(dx, dy)
			else:
				self._rotate(dx, dy)

		self._last_mouse = mouse_pos if dragging else None
		return task.cont

	def _rotate(self, dx, dy):
		# Fully camera-relative (trackball-style): horizontal drag turns the
		# object around the camera's current up axis (yaw), vertical drag
		# around its current right axis (pitch) -- neither is the world Z
		# axis, so both track the current view at any camera angle. Composed
		# and applied ON TOP of the pivot's existing orientation, not
		# replacing it, so a shape already tilted by default_rot_quat stays
		# tilted as you spin it further.
		#
		# Panda3D's Quat * Quat composes as "this" happening AFTER "other" in
		# WORLD space when written other * this -- i.e. old.xform(delta.xform(v))
		# for `old * delta`, which applies delta in old's *local* frame first.
		# We want the opposite (delta expressed in the fixed world/camera
		# frame, applied on top of whatever the object's current orientation
		# is), so it must be `pivot.getQuat() * delta`, not `delta * pivot.getQuat()`
		# -- the reversed order silently rotated around the object's own
		# (already-tilted) axes instead of the camera-fixed ones.
		cam_quat = self.app.camera.getQuat(self.app.render)
		yaw = Quat()
		yaw.setFromAxisAngle(dx * self.rotate_speed, cam_quat.getUp())
		pitch = Quat()
		pitch.setFromAxisAngle(-dy * self.rotate_speed, cam_quat.getRight())
		delta = pitch * yaw
		self.pivot.setQuat(self.pivot.getQuat() * delta)

	def _move(self, dx, dy):
		# Same screen-space-accurate scale as OrbitCamera._pan(), just added
		# instead of subtracted: grabbing and dragging the object itself
		# should follow the cursor, unlike panning the camera's target
		# (which moves opposite the drag so the *world* seems to follow it).
		quat = self.app.camera.getQuat(self.app.render)
		fov = self.app.camLens.getFov()
		distance = self.orbit_camera.distance
		right_scale = distance * tan(radians(fov.x / 2))
		up_scale = distance * tan(radians(fov.y / 2))
		pos = self.pivot.get_pos()
		pos += quat.getRight() * dx * right_scale
		pos += quat.getUp() * dy * up_scale
		self.pivot.set_pos(pos)
