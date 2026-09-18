"""A window docked to one edge of the screen, stretching to fill whatever's
left after other docked panels have claimed their own edge. Replaces the
hand-rolled imgui.set_next_window_pos()/set_next_window_size()/
set_next_window_size_constraints() + imgui_ctx.begin() dance app.py used to
repeat separately for its Explorer/panel/sysinfo windows (project-todos/
forgery/ui_module_consolidation.md, Nuno 2026-09-18: "on consolide l'ui en
creant des widgets reutilisables" -- then "on met [resizable] dans Panel et
il doit etre capable de le faire lui-meme": Panel owns positioning, sizing,
resize constraints, and the begin/end pair itself, not just the rect math).

Deliberately NOT a general-purpose layout engine (no wrapping, no nested
panels, no more than the 4 cardinal edges) -- just the one pattern Forgery
actually needs. A recursive Box/Panel system (project-todos/forgery/
box_panel_layout_system.md) exists separately in box.py, validated so far
only in an isolated test app -- THIS module is still what app.py actually
uses; migrating app.py to the new system is a distinct, not-yet-done
chantier, see box_panel_layout_system.md's own "Hors scope" note.

A panel auto-fitting its own content (size determined by what it draws,
rather than a fixed/resizable extent) was attempted and dropped 2026-09-18:
imgui.WindowFlags_.always_auto_resize on the sysinfo panel reliably crashed
Panda3D's own renderFrame() with a NaN bounding-sphere assertion (bisected --
reproducible with the flag, gone without it, mechanism not understood).
"""

from contextlib import contextmanager

from imgui_bundle import imgui, imgui_ctx


def dock_rect(available, side, size):
	"""`available`: the (x, y, w, h) rect still free for docking. `side`:
	"left"/"right"/"top"/"bottom". `size`: this panel's extent along the
	axis perpendicular to `side` (width for left/right, height for
	top/bottom). Returns (panel_rect, remaining_rect), both (x, y, w, h) --
	`remaining_rect` is what's left over for the NEXT panel in a chain to
	dock into, same idea as CSS's border-box layout collapsing from the
	outside in."""
	x, y, w, h = available
	if side == "left":
		return (x, y, size, h), (x + size, y, w - size, h)
	if side == "right":
		return (x + w - size, y, size, h), (x, y, w - size, h)
	if side == "top":
		return (x, y, w, size), (x, y + size, w, h - size)
	if side == "bottom":
		return (x, y + h - size, w, size), (x, y, w, h - size)
	raise ValueError(f"unknown side {side!r}, expected left/right/top/bottom")


class Panel:
	"""One docked region, self-sufficient: knows its own side/size/resize
	bounds, and `draw()` handles positioning, sizing, resize constraints,
	and the imgui begin/end pair -- the caller only draws CONTENT inside
	the `with` block, never touches imgui.set_next_window_*() itself.

	`size`: this panel's extent along the axis perpendicular to `side`
	(width for left/right, height for top/bottom) -- the ONLY size a
	docked panel ever has an opinion on; the parallel axis always stretches
	to fill `available` (see draw()).

	`resizable`: if True, the user can drag this panel's inner edge between
	`min_size`/`max_size` (both required when resizable) -- `size` is then
	updated automatically at the end of every draw() call to whatever the
	user just dragged it to, exactly like app.py used to do by hand
	(self.explorer_width = imgui.get_window_size().x). If False, `size` is
	fixed -- the caller may still reassign panel.size itself between
	frames, just not via dragging."""

	def __init__(self, side, size, resizable=False, min_size=None, max_size=None):
		self.side = side
		self.size = size
		self.resizable = resizable
		if resizable and (min_size is None or max_size is None):
			raise ValueError("a resizable Panel needs both min_size and max_size")
		self.min_size = min_size
		self.max_size = max_size

	def _size_constraints(self, rect):
		_x, _y, w, h = rect
		if self.side in ("left", "right"):
			return (self.min_size, h), (self.max_size, h)
		return (w, self.min_size), (w, self.max_size)

	@contextmanager
	def draw(self, available, title, flags=0):
		"""`with panel.draw(available, title) as remaining:` -- docks this
		panel's edge of `available` (the (x, y, w, h) rect still free),
		opens its ImGui window (sized/constrained/flagged appropriately),
		and yields `remaining` for the caller to both draw content against
		AND pass into the next panel's own draw() call, chaining the
		leftover space same as dock_rect() alone would. `title`: forwarded
		to imgui_ctx.begin() as-is (e.g. "Explorer", or
		f"{self.panel_title()}###panel" for a title that changes with
		whatever's loaded but keeps a stable ImGui identity) -- a
		"##"-prefixed title also hides the title bar, same convention as
		everywhere else in this codebase (e.g. sysinfo's own "##sysinfo").
		`flags`: extra imgui.WindowFlags_ this specific panel wants on top
		of the ones draw() always sets itself (no_move, no_collapse, and
		no_resize unless `resizable`)."""
		rect, remaining = dock_rect(available, self.side, self.size)
		x, y, w, h = rect
		imgui.set_next_window_pos((x, y))
		panel_flags = flags | imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
		if title.startswith("##") and "###" not in title:
			panel_flags |= imgui.WindowFlags_.no_title_bar.value
		if self.resizable:
			# Seeds the INITIAL size (e.g. explorer_width restored from
			# settings.toml) once, the very first time this window ever
			# opens (Cond_.first_use_ever) -- constraints alone leave imgui
			# free to pick its own arbitrary starting size within bounds
			# otherwise. Every frame after that, only the constraints apply
			# -- imgui remembers whatever the user last dragged it to on
			# its own, same as any other window.
			imgui.set_next_window_size((w, h), cond=imgui.Cond_.first_use_ever.value)
			imgui.set_next_window_size_constraints(*self._size_constraints(rect))
		else:
			imgui.set_next_window_size((w, h))
			panel_flags |= imgui.WindowFlags_.no_resize.value
		with imgui_ctx.begin(title, flags=panel_flags):
			yield remaining
			if self.resizable:
				size = imgui.get_window_size()
				self.size = size.y if self.side in ("top", "bottom") else size.x
