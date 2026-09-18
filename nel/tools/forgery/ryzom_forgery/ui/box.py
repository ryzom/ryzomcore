"""Box: a nestable, self-sizing rectangle -- the real hbox/vbox primitive
Forgery's UI was missing (project-todos/forgery/box_panel_layout_system.md,
Nuno 2026-09-18: "l'interface c'est un vrai foutoir... on comprend
absolument rien a cette UI"). Everything else derives from this one class --
e.g. a Panel that ALSO opens a real ImGui window for itself, kept OUTSIDE
this module deliberately (no imgui_bundle import here at all) so Box's own
layout math stays testable in plain Python, no display/imgui needed (this
mattered in practice: a 50/50-split bug was caught simulating Box.layout()
directly, before ever opening a real window -- see this file's own git log,
2026-09-18).

NOTE: this is NOT the Panel app.py/sysinfo.py actually use today (see
ui/panel.py, a separate, older, flat 4-edge-only Panel) -- migrating the
real app to a Box-based system is a distinct, not-yet-done chantier
(box_panel_layout_system.md's own "Hors scope"). This Box class so far is
only validated in an isolated test app.

A Box has two axes (h, v), each one of: a fixed number of pixels, STRETCH,
or a Percent(value) of its PARENT's own extent on that axis. It has a
`margin` (empty space AROUND it, subtracted from what its parent granted it
before anything else) and a `padding` (empty space WITHIN it, subtracted
again before laying out its own children) -- giving it an `external_rect`
(what the parent used to grant it, minus margin -- the box's own visible
bounds) and an `internal_rect` (external_rect minus padding -- the space
its own children are laid out into).

A Box docks to one side of its PARENT's current internal_rect (`dock`:
"top"/"bottom"/"left"/"right"/"center"). The parent places its children ONE
AT A TIME, in declaration order, shrinking its own remaining rect by
whatever each child just consumed -- top/bottom children stack vertically
(consuming from the top/bottom edge), left/right children stack
horizontally (consuming from the left/right edge), center children are
centered in whatever's left without consuming any of it. Multiple children
docked to the SAME side that are BOTH stretched on that side's flow axis
split the space remaining at that point EQUALLY between them (confirmed
with Nuno, 2026-09-18 -- not "the first one takes everything"). A fixed
number and a Percent both count as "already claimed" space for this
purpose, resolved against the parent's ORIGINAL internal_rect extent, same
as a STRETCH share is -- never against the shrinking remaining rect, or
placing several such children in a row would compound each other's
percentages instead of each reading its own share off the real total.
"""

STRETCH = "stretch"

_DOCK_SIDES = ("top", "bottom", "left", "right", "center")


class Percent:
	"""A Box's h or v value: `value` percent of its PARENT's own
	internal_rect extent on that axis (0-100), resolved once against the
	parent's full extent -- see this module's own docstring for why never
	against the shrinking remaining rect."""

	def __init__(self, value):
		self.value = value

	def resolve(self, total):
		return total * self.value / 100.0

	def __repr__(self):
		return f"Percent({self.value})"


def _resolve_fixed(size, total):
	"""`size`: a Box's h or v value. Returns the resolved pixel size for a
	FIXED size (plain number or Percent), or None if `size` is STRETCH
	(resolved separately, once every fixed sibling's share of the budget is
	known -- see _layout_children())."""
	if size is STRETCH:
		return None
	if isinstance(size, Percent):
		return size.resolve(total)
	return size


class Box:
	def __init__(self, dock="top", h=STRETCH, v=STRETCH, margin=0.0, padding=0.0):
		if dock not in _DOCK_SIDES:
			raise ValueError(f"unknown dock {dock!r}, expected one of {_DOCK_SIDES}")
		self.dock = dock
		self.h = h
		self.v = v
		self.margin = margin
		self.padding = padding
		self.children = []
		# Set by layout() -- (x, y, w, h) each, in the same coordinate space
		# as whatever rect was passed in (screen space for a Panel's root
		# Box, parent-relative for everything nested under it once that
		# parent itself already resolved into screen space).
		self.external_rect = (0.0, 0.0, 0.0, 0.0)
		self.internal_rect = (0.0, 0.0, 0.0, 0.0)

	def add(self, box):
		"""Appends `box` as a child, laid out AFTER every child already
		added -- declaration order is placement order. Returns `box`, so
		building a tree can chain: parent.add(Box(...)).add(Box(...))."""
		self.children.append(box)
		return box

	def layout(self, allocated):
		"""`allocated`: the (x, y, w, h) rect the parent granted this box
		(already excludes whatever space its OWN dock consumed from the
		parent's remaining rect). Computes external_rect (allocated minus
		margin) and internal_rect (external minus padding), then lays out
		every child into internal_rect."""
		x, y, w, h = allocated
		m = self.margin
		self.external_rect = (x + m, y + m, max(w - 2 * m, 0.0), max(h - 2 * m, 0.0))
		ex, ey, ew, eh = self.external_rect
		p = self.padding
		self.internal_rect = (ex + p, ey + p, max(ew - 2 * p, 0.0), max(eh - 2 * p, 0.0))
		self._layout_children()

	def _layout_children(self):
		# Pass 1: the vertical budget (top+bottom children share the SAME
		# internal_rect height) and horizontal budget (left+right children
		# share the SAME internal_rect width), each split into "already
		# claimed by fixed-size (or Percent) children" and "however many
		# stretched children divide whatever's left" -- computed ONCE
		# against the ORIGINAL internal_rect, never against the shrinking
		# `remaining` rect placement itself uses, or every sibling placed
		# after the first would divide an already-smaller pot instead of an
		# equal share of the real total (bug found 2026-09-18 simulating
		# this exact method against Nuno's own test tree before ever
		# opening imgui: a 50/50 split came out 50/25 without this fix).
		_x, _y, internal_w, internal_h = self.internal_rect
		v_fixed = 0.0
		v_stretch_count = 0
		h_fixed = 0.0
		h_stretch_count = 0
		for c in self.children:
			if c.dock in ("top", "bottom"):
				resolved = _resolve_fixed(c.v, internal_h)
				if resolved is None:
					v_stretch_count += 1
				else:
					v_fixed += resolved
			elif c.dock in ("left", "right"):
				resolved = _resolve_fixed(c.h, internal_w)
				if resolved is None:
					h_stretch_count += 1
				else:
					h_fixed += resolved
		v_share = max(internal_h - v_fixed, 0.0) / v_stretch_count if v_stretch_count else 0.0
		h_share = max(internal_w - h_fixed, 0.0) / h_stretch_count if h_stretch_count else 0.0

		remaining = self.internal_rect
		for child in self.children:
			remaining = self._place_child(child, remaining, v_share, h_share, internal_w, internal_h)

	@staticmethod
	def _place_child(child, remaining, v_share, h_share, internal_w, internal_h):
		x, y, w, h = remaining
		if child.dock in ("top", "bottom"):
			resolved_v = _resolve_fixed(child.v, internal_h)
			child_h = v_share if resolved_v is None else resolved_v
			child_h = min(child_h, h)
			resolved_h = _resolve_fixed(child.h, internal_w)
			child_w = w if resolved_h is None else min(resolved_h, w)
			# Cross-axis (horizontal) alignment for a fixed-width child on a
			# vertical-flow side: left-started by default -- no cross-axis
			# centering in this first version, see the chantier's own note.
			child_x = x
			if child.dock == "top":
				child.layout((child_x, y, child_w, child_h))
				return (x, y + child_h, w, h - child_h)
			else:  # bottom
				child.layout((child_x, y + h - child_h, child_w, child_h))
				return (x, y, w, h - child_h)
		elif child.dock in ("left", "right"):
			resolved_h = _resolve_fixed(child.h, internal_w)
			child_w = h_share if resolved_h is None else resolved_h
			child_w = min(child_w, w)
			resolved_v = _resolve_fixed(child.v, internal_h)
			child_h = h if resolved_v is None else min(resolved_v, h)
			child_y = y
			if child.dock == "left":
				child.layout((x, child_y, child_w, child_h))
				return (x + child_w, y, w - child_w, h)
			else:  # right
				child.layout((x + w - child_w, child_y, child_w, child_h))
				return (x, y, w - child_w, h)
		else:  # center -- doesn't consume any of the parent's flow space
			resolved_h = _resolve_fixed(child.h, internal_w)
			child_w = w if resolved_h is None else min(resolved_h, w)
			resolved_v = _resolve_fixed(child.v, internal_h)
			child_h = h if resolved_v is None else min(resolved_v, h)
			child_x = x + (w - child_w) / 2.0
			child_y = y + (h - child_h) / 2.0
			child.layout((child_x, child_y, child_w, child_h))
			return remaining
