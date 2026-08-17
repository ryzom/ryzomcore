from math import asin, atan2, cos, degrees, radians, sin, tan

from panda3d.core import ClockObject, KeyboardButton, MouseButton, Point3, Quat, Vec3

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
	target freely, right-mouse drag zooms smoothly (same direction as the
	wheel -- dragging up zooms in), mouse wheel always zooms in discrete
	notches. Input is ignored while Dear ImGui wants the mouse (e.g. dragging
	a slider), so tool UI and viewport input don't fight over the same drag.
	"""

	def __init__(self, app, target=Point3(0, 0, 0), distance=20.0, heading=0.0, pitch=0.0):
		self.app = app
		self.target = Point3(target)
		self.distance = distance
		self.heading = heading
		self.pitch = pitch
		# What lookAt() treats as "up" when placing the camera -- always the
		# world Z axis (giving the usual orbit-camera behaviour, always as
		# level as the current pitch allows).
		self.up_hint = Vec3(0, 0, 1)

		self.min_distance = 0.5
		self.max_distance = 2000.0
		self.min_pitch = -89.0
		self.max_pitch = 89.0

		self.orbit_speed = 200.0  # degrees per full mouse-width drag
		self.zoom_speed = 0.9  # multiplier applied to distance per wheel notch
		self.zoom_drag_speed = 6.0  # exponent scale applied to zoom_speed per full mouse-height right-drag

		self.anim_duration = 0.25  # seconds -- snap_to_axis()/roll_step() animate over this
		self._anim = None  # in-flight animation state (see _start_anim()/_advance_anim()), or None

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

	def _zoom_drag(self, dy):
		"""Same exponential falloff as wheel zoom (zoom_speed), just driven
		continuously by right-mouse drag distance instead of discrete notches
		-- dragging up zooms in, matching wheel_up's own direction (_zoom(1)
		also uses factor=zoom_speed<1, i.e. distance shrinks)."""
		factor = pow(self.zoom_speed, dy * self.zoom_drag_speed)
		self.distance = max(self.min_distance, min(self.max_distance, self.distance * factor))
		self._update_camera_pos()

	def _start_anim(self, end_heading, end_pitch, end_up):
		"""Kicks off (or redirects, if one's already in flight) a smooth
		transition to an explicit (end_heading, end_pitch, end_up) target --
		used by snap_to_axis() to retarget the view. Heading/pitch/up_hint
		are lerped independently, which is fine for an arbitrary retarget
		(there's no single "true" path between two unrelated views anyway),
		but is NOT geometrically correct for a fixed-axis rotation -- see
		_start_axis_anim() for that case. Advanced/applied by
		_advance_anim(), called from _update() every frame."""
		# Shortest path in heading (mod 360), not the raw numeric delta, so
		# e.g. 350 -> 0 animates as +10 rather than spinning -350 degrees the
		# long way around.
		delta = (end_heading - self.heading + 180.0) % 360.0 - 180.0
		self._anim = {
			"kind": "retarget",
			"start_heading": self.heading, "end_heading": self.heading + delta,
			"start_pitch": self.pitch, "end_pitch": end_pitch,
			"start_up": Vec3(self.up_hint), "end_up": Vec3(end_up),
			"elapsed": 0.0,
		}

	def _start_axis_anim(self, axis, angle):
		"""Kicks off a smooth, geometrically exact rotation by `angle`
		degrees around `axis`, applied to the CURRENT offset (position -
		target) and up_hint -- used by step_to_face()'s "up"/"down" case and
		roll_step(), which both ARE a single well-defined fixed-axis
		rotation from wherever the view currently is, unlike _start_anim()'s
		retarget-to-an-unrelated-view case. Interpolating the rotation ANGLE
		itself (see _advance_anim()) traces the exact circular arc of that
		rotation, rather than _start_anim()'s independent heading/pitch/up
		lerps, which don't correspond to any single real rotation and were
		visibly animating through the wrong intermediate orientations even
		though they still landed on the correct final one."""
		h_rad, p_rad = radians(self.heading), radians(self.pitch)
		start_offset = Vec3(
			self.distance * cos(p_rad) * sin(h_rad),
			-self.distance * cos(p_rad) * cos(h_rad),
			self.distance * sin(p_rad),
		)
		# setFromAxisAngle() (_advance_anim()) asserts its axis is unit
		# length -- `axis` here is usually self.up_hint or a live getRight(),
		# already unit in principle, but re-normalize defensively rather
		# than trust that floating-point error never nudges it far enough
		# over repeated chained rotations to trip that assert.
		normalized_axis = Vec3(axis)
		normalized_axis.normalize()
		self._anim = {
			"kind": "axis",
			"start_offset": start_offset, "start_up": Vec3(self.up_hint),
			"axis": normalized_axis, "angle": angle,
			"elapsed": 0.0,
		}

	def _advance_anim(self):
		anim = self._anim
		anim["elapsed"] += ClockObject.get_global_clock().get_dt()
		t = min(1.0, anim["elapsed"] / self.anim_duration)

		if anim["kind"] == "axis":
			# Constant angular velocity (no ease-in/ease-out) -- step_to_face()
			# no-ops new steps while one's already in flight, so holding a
			# button chains a run of these back to back. Smoothstep's zero
			# velocity at both ends of EVERY step would decelerate to a stop
			# and re-accelerate at each 90-degree boundary, reading as a
			# stutter instead of one continuous spin; a constant rate carries
			# the same speed across the boundary into the next step, with
			# nothing to smooth in the first place for a single isolated step.
			eased = t
			rotation = Quat()
			rotation.setFromAxisAngle(anim["angle"] * eased, anim["axis"])
			offset = rotation.xform(anim["start_offset"])
			self.pitch = degrees(asin(max(-1.0, min(1.0, offset.z / self.distance))))
			self.heading = degrees(atan2(offset.x, -offset.y))
			up = rotation.xform(anim["start_up"])
			up.normalize()
			self.up_hint = up
		else:
			eased = t * t * (3.0 - 2.0 * t)  # smoothstep -- fine here, snap_to_axis() retargets never chain
			self.heading = anim["start_heading"] + (anim["end_heading"] - anim["start_heading"]) * eased
			self.pitch = anim["start_pitch"] + (anim["end_pitch"] - anim["start_pitch"]) * eased
			up = anim["start_up"] + (anim["end_up"] - anim["start_up"]) * eased
			# This "retarget" case's start/end up vectors are never exact
			# opposites in practice, so the lerp never passes through the
			# zero vector -- a plain lerp-then-normalize is enough, no need
			# for a full slerp with its antipodal-vector handling.
			if up.length_squared() > 1e-9:
				up.normalize()
				self.up_hint = up

		if t >= 1.0:
			self._anim = None
		self._update_camera_pos()

	def _update(self, task):
		if self._anim is not None:
			self._advance_anim()

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
		zoom_down = mw.isButtonDown(MouseButton.three())
		dragging = orbit_down or pan_down or zoom_down

		if dragging:
			self._anim = None  # a manual drag overrides any in-flight snap/roll animation

		if dragging and self._last_mouse is not None:
			dx = mouse_pos[0] - self._last_mouse[0]
			dy = mouse_pos[1] - self._last_mouse[1]
			if zoom_down:
				self._zoom_drag(dy)
			elif pan_down:
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

	def snap_to_axis(self, axis):
		"""Snap the view to look straight along one of the 6 world axes
		(+x/-x/+y/-y/+z/-z), keeping the current target/distance. Animated,
		like step_to_face()/roll_step() (see _start_anim())."""
		heading, pitch = AXIS_VIEWS[axis]
		self._start_anim(heading, pitch, Vec3(0, 0, 1))

	def step_to_face(self, direction):
		"""Steps the view by 90 degrees in the given screen direction
		("up"/"down"/"left"/"right") -- e.g. the navcube's arrow buttons.
		Both cases rotate the current offset (position - target) AND
		up_hint by 90 degrees around whichever axis is CURRENTLY rendered on
		screen as the relevant one -- "left"/"right" around self.up_hint
		(the current "up"), "up"/"down" around the camera's actual current
		right axis (self.app.camera.getQuat().getRight()) -- and re-derive
		heading/pitch from the result, rather than jumping to the next
		AXIS_VIEWS entry via a heading/pitch lookup table. This matters for
		two reasons:

		- Heading has a coordinate singularity at the poles (+z/-z) where it
		  becomes a meaningless atan2(0, 0)-style artifact, so re-deriving a
		  rotation axis from it would rotate a further step around whatever
		  arbitrary axis that artifact heading implies -- not the axis
		  actually on screen. Reading the live, already-rendered right axis
		  sidesteps that entirely.
		- Rotating up_hint (rather than resetting it to a hardcoded world
		  Z), regardless of direction, means a step never forces the view
		  back to "Z is up" -- if a previous "up"/"down" step left some
		  other axis reading as up (see its own rotation for why), further
		  "left"/"right" steps keep rotating around THAT axis instead of
		  snapping the roll back to Z, matching a real physical cube: the
		  face you had "up" stays "up" through a pure yaw.

		No-ops while a previous step's animation is still in flight (held
		buttons fire this repeatedly via ImGui's button_repeat) -- letting a
		later repeat interrupt/restart an in-flight rotation was producing
		inconsistent speed and left the view stuck mid-rotation if the mouse
		was released before that restarted animation finished. Ignoring
		repeats until the current step completes instead gives one clean,
		full 90-degree step every anim_duration for as long as the button is
		held, and releasing mid-step just lets that last step finish."""
		if self._anim is not None:
			return

		if direction in ("left", "right"):
			angle = -90.0 if direction == "right" else 90.0
			self._start_axis_anim(self.up_hint, angle)
			return

		# Rotating up_hint by this SAME rotation (see _start_axis_anim()),
		# rather than resetting it to world Z on arrival, matters here
		# specifically: at a pole (+z/-z) forward is exactly +/-Z, so a
		# hardcoded Z up_hint would be degenerate (parallel to forward), and
		# Panda's lookAt() falls back to some other, uncontrolled axis for
		# "up" in that case -- which is what made a FURTHER up/down step
		# read as rotating around the wrong axis.
		right_axis = self.app.camera.getQuat(self.app.render).getRight()
		angle = 90.0 if direction == "up" else -90.0
		self._start_axis_anim(right_axis, angle)

	def roll_step(self, direction, degrees_step=90.0):
		"""Rolls the view by a fixed step around its own forward axis --
		camera position and look direction don't change, only what reads as
		"up" on screen rotates (e.g. the navcube's 2 top-corner buttons).
		Animated via _start_axis_anim(), like step_to_face()'s "up"/"down"
		case -- rotating the current offset around the (parallel) forward
		axis is a no-op, so position/look direction naturally stay put while
		up_hint (perpendicular to forward) rotates. Independent of
		step_to_face()/snap_to_axis(), which move to a different face
		instead of spinning the current one. No-ops while a previous step's
		animation is still in flight -- see step_to_face()'s docstring."""
		if self._anim is not None:
			return

		h_rad, p_rad = radians(self.heading), radians(self.pitch)
		offset = Vec3(
			self.distance * cos(p_rad) * sin(h_rad),
			-self.distance * cos(p_rad) * cos(h_rad),
			self.distance * sin(p_rad),
		)
		forward = -offset
		forward.normalize()
		self._start_axis_anim(forward, degrees_step * direction)

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
	"""Rotates/moves/scales a NodePath (the shape being inspected) via
	Ctrl+drag -- Ctrl+left-mouse spins it in place, Ctrl+middle-mouse moves it
	in world space (tracking the cursor the same screen-space-accurate way
	OrbitCamera's own pan does), Ctrl+right-mouse scales it (dragging up
	grows it, same "up" direction as OrbitCamera's own right-drag zoom-in, so
	both gestures make the object read bigger on screen). Mutually exclusive
	with OrbitCamera's plain (no-Ctrl) drag controls -- both check for the
	Ctrl modifier so only one responds to a given drag.

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
		self.scale_speed = 0.9  # exponent base, matches OrbitCamera.zoom_speed
		self.scale_drag_speed = 6.0  # exponent scale per full mouse-height drag, matches OrbitCamera.zoom_drag_speed
		self.min_scale = 0.01
		self.max_scale = 100.0

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
		scale_down = mw.isButtonDown(MouseButton.three())
		dragging = rotate_down or move_down or scale_down

		if dragging and self._last_mouse is not None:
			dx = mouse_pos[0] - self._last_mouse[0]
			dy = mouse_pos[1] - self._last_mouse[1]
			if move_down:
				self._move(dx, dy)
			elif scale_down:
				self._scale(dy)
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

	def _scale(self, dy):
		# Same exponential feel as OrbitCamera._zoom_drag(), just applied to
		# the pivot's own scale instead of the camera distance -- dragging up
		# grows the object, down shrinks it. Assumes a uniform scale (the
		# pivot only ever gets set_scale()'d here, never per-axis).
		factor = pow(1.0 / self.scale_speed, dy * self.scale_drag_speed)
		new_scale = max(self.min_scale, min(self.max_scale, self.pivot.get_scale().x * factor))
		self.pivot.set_scale(new_scale)
