#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Mouse cursor -> world position on the Z=0 ground plane, for any Forgery
app built on ForgeryApp (project-todos/forgery/
landscape_editor__cursor_zone_status.md step 2). ZERO dependency on any app
module -- same reasoning as zone_geometry.py's own docstring.

Not a real raycast against the loaded terrain (no height, just X/Y on the
Z=0 plane) -- see the chantier's own scope notes.
"""

from typing import Optional, Tuple

from panda3d.core import Point3


def mouse_ground_position(app) -> Optional[Tuple[float, float]]:
	"""(x, y) where the ray through the mouse cursor crosses Z=0 in world
	space, or None if the cursor isn't over the 3D viewport (same
	`mouseWatcherNode.hasMouse()` check camera.py's own OrbitCamera uses) or
	the view ray is (near enough) parallel to the ground plane -- looking
	exactly along the horizon, never actually reachable through the orbit
	camera's own pitch clamp, but guarded here since this module doesn't
	assume any particular camera controller."""
	mw = app.mouseWatcherNode
	if not mw.hasMouse():
		return None

	near_point = Point3()
	far_point = Point3()
	app.camLens.extrude(mw.getMouse(), near_point, far_point)
	near_point = app.render.get_relative_point(app.camera, near_point)
	far_point = app.render.get_relative_point(app.camera, far_point)

	direction = far_point - near_point
	if abs(direction.z) < 1e-6:
		return None
	t = -near_point.z / direction.z
	return near_point.x + direction.x * t, near_point.y + direction.y * t
