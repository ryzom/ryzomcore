"""Panel and Widget: the ImGui-facing half of the Box layout system
(project-todos/forgery/box_panel_layout_system.md). Split out of box.py
deliberately -- Box itself stays free of any imgui_bundle import so its own
layout math is testable in plain Python with no display needed (this
mattered in practice: a 50/50-split bug was caught simulating Box.layout()
directly, before ever opening a real window). Everything here needs a real
ImGui context to mean anything, so it lives in its own module instead.

NOT the Panel app.py/sysinfo.py actually use today -- that's ui/panel.py, a
separate, older, flat 4-edge-only design. Migrating the real app to this
Box-based system is a distinct, not-yet-done chantier (see
box_panel_layout_system.md's own "Hors scope"). Everything below is
currently only exercised by the project's own disposable test harness
(.test_box_panel.py at the worktree root) -- promoted here, out of that
script, so validated fixes stop being at risk of getting silently dropped
the next time that script gets rewritten for a new test scenario (Nuno,
2026-09-18: "on sait maintenant pourquoi il y a tant de choses qui se
perdent avec toi").
"""

from imgui_bundle import imgui, imgui_ctx

from .box import Box, STRETCH
from .colors import pastel_color_for

# Shared side-panel resize bounds -- moved here from app.py (project-todos/
# forgery/box_ui_migration__explorer_panel.md) since this is the Box-based
# system's own canonical home for the convention going forward; app.py
# still imports these for its own not-yet-migrated ui/panel.py Panel
# (right panel).
_SIDE_PANEL_MIN_WIDTH = 150
_SIDE_PANEL_MAX_WIDTH = 900


class Panel(Box):
	"""A Box that ALSO opens a real ImGui window for itself, instead of
	just being a plain rectangle inside its parent's window -- Nuno,
	2026-09-18: "Panel c'est donc une Box mais qui en plus se place
	directement dans la fenetre et peut s'agrandir". Typically added as a
	direct child of a screen-sized root Box (dock against a screen edge),
	but nothing else about it differs from a plain Box -- same
	margin/padding/dock/children layout, all inherited unchanged.

	`padding` defaults to 4 here (Box itself defaults to 0) -- a Panel is a
	Box, so its OWN padding is the one and only padding source; ImGui's own
	WindowPadding style is forced to 0 in layout() below, so the two never
	stack into a double gap (Nuno, 2026-09-18: "on va avoir 2 padding...
	vire WindowPadding")."""

	def __init__(self, dock, h=STRETCH, v=STRETCH, resizable=False, min_size=None, max_size=None,
	             title="##panel", padding=4.0, extra_flags=0):
		super().__init__(dock=dock, h=h, v=v, padding=padding)
		self.resizable = resizable
		if resizable and (min_size is None or max_size is None):
			raise ValueError("a resizable Panel needs both min_size and max_size")
		self.min_size = min_size
		self.max_size = max_size
		self.title = title
		self.extra_flags = extra_flags

	def _size_constraints(self, rect):
		_x, _y, w, h = rect
		if self.dock in ("left", "right"):
			return (self.min_size, h), (self.max_size, h)
		return (w, self.min_size), (w, self.max_size)

	def layout(self, allocated):
		"""Same contract as Box.layout() (a parent Box calls this with the
		rect it granted this panel) -- but first opens/positions/sizes the
		real ImGui window at that rect, THEN delegates to Box.layout() for
		the margin/padding/children-placement part, using the REAL content
		area (not the outer window rect passed in -- a visible title bar
		eats into the top of that, so using it directly made the first row
		land partly under the title bar; found 2026-09-18, Nuno: "le titre
		a une hauteur, tu ne prend pas la hauteur en compte"). Resizable
		windows report their post-drag size one frame later than the drag
		itself (imgui.get_window_size(), same lag every resizable window
		here already accepts)."""
		x, y, w, h = allocated
		flags = imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value | self.extra_flags
		if self.title.startswith("##") and "###" not in self.title:
			flags |= imgui.WindowFlags_.no_title_bar.value
		imgui.set_next_window_pos((x, y))
		if self.resizable:
			# The FIRST-EVER size hint must be this panel's own requested
			# size (self.h/self.v, e.g. LeftPanel(size=200)) on its resize
			# axis, never `allocated`'s w/h -- for a standalone resizable
			# Panel (not nested inside a parent Box's own _layout_children(),
			# e.g. LeftPanel used directly via object_editor.py's
			# _draw_left_panel()), `allocated` is the FULL space the caller
			# handed it, not this panel's own size -- using it here made a
			# brand new window (no prior imgui.ini entry) default to the
			# entire available width/height instead of `size=`. The other
			# axis still fills `allocated` (never user-resizable, see
			# _size_constraints() below, which already draws this same
			# distinction).
			initial_size = (self.h, h) if self.dock in ("left", "right") else (w, self.v)
			imgui.set_next_window_size(initial_size, cond=imgui.Cond_.first_use_ever.value)
			imgui.set_next_window_size_constraints(*self._size_constraints(allocated))
		else:
			imgui.set_next_window_size((w, h))
			flags |= imgui.WindowFlags_.no_resize.value
		imgui.push_style_var(imgui.StyleVar_.window_padding.value, (0.0, 0.0))
		with imgui_ctx.begin(self.title, flags=flags):
			imgui.pop_style_var()
			if self.resizable:
				actual = imgui.get_window_size()
				if self.dock in ("left", "right"):
					self.h = actual.x
				else:
					self.v = actual.y
			content_pos = imgui.get_cursor_screen_pos()
			content_avail = imgui.get_content_region_avail()
			super().layout((content_pos.x, content_pos.y, content_avail.x, content_avail.y))


class LeftPanel(Panel):
	"""A Panel docked to the left edge, with the resize bounds every side
	panel in this app shares -- factors out what's genuinely identical
	between apps (position/size/resize mechanics) so each app can own its
	own independent left-panel instance and fill it with its own content
	(box_ui_migration__explorer_panel.md, Nuno 2026-09-19: "chaque App va
	la remplir")."""

	def __init__(self, size, title="##panel", padding=2.0, extra_flags=0):
		super().__init__(dock="left", h=size, resizable=True,
		                  min_size=_SIDE_PANEL_MIN_WIDTH, max_size=_SIDE_PANEL_MAX_WIDTH,
		                  title=title, padding=padding, extra_flags=extra_flags)


DEFAULT_WIDGET_MARGIN = 1.0


class Widget(Box):
	"""A Box that draws REAL ImGui content into its own internal_rect
	instead of laying out further Box children -- override draw_content()
	in a subclass (see ButtonWidget/LabelWidget/TextInputWidget below).
	Nuno, 2026-09-18: "il faut que les box puissent gerer aussi comme
	enfants d'autres widgets".

	`margin` defaults to `DEFAULT_WIDGET_MARGIN` here (Box itself defaults
	to 0) -- every VISIBLE widget (a button, a list, a text label, an
	entry...) gets a little breathing room around it by default, instead
	of every call site having to remember to pass its own margin (Nuno,
	2026-09-20: "je pense [qu'il faut] un margin par defaut... autour des
	widgets qui sont visible"). A plain container `Box` (a row grouping
	several widgets, a root) is unaffected -- it isn't a `Widget`, still
	defaults to 0. Pass `margin=0` explicitly to opt a specific widget out
	(see sysinfo.py's LabelWidget/ProgressBarWidget, sized to an exact
	pixel width via `imgui.calc_text_size()` -- adding margin there would
	shrink their content area past that exact size).

	**Sizing a fixed-size row/widget around this default margin**: a row
	(or a `dock="left"`/`"right"` widget's own `h`) meant to hold exactly
	one natural-size widget -- `imgui.get_frame_height()` for a single-line
	control, an icon glyph's own square size, ... -- must reserve
	`natural_size + 2 * DEFAULT_WIDGET_MARGIN`, NOT `natural_size` alone.
	`imgui.get_frame_height()` is already ImGui's own no-slack size for
	that control; carving margin OUT of an already-tight reservation makes
	`internal_rect` smaller than the control's natural size, but most
	ImGui controls (an `input_text`, a `button` given a size smaller than
	its own natural content) don't shrink to fit -- they render at their
	natural size regardless, overflowing `internal_rect` and visually
	erasing the margin entirely, or (for an icon button given too little
	room) rendering the icon truncated/cropped. Found via
	`.test_widget_margin.py` (Nuno 2026-09-20: "4 pixel en top, mais 7 en
	bottom" then "y a plus de margin la" once the row height itself was
	fixed) -- see `explorer.py`/`workspace_setup_dialog.py`'s own
	`row_height`/icon-button sizing for the applied fix."""

	def __init__(self, margin=DEFAULT_WIDGET_MARGIN, **kwargs):
		super().__init__(margin=margin, **kwargs)

	def draw_content(self, w, h):
		"""Override in a subclass -- `w`, `h` are this widget's own
		RESOLVED internal_rect size in pixels, already computed by the
		time this runs, regardless of whether self.h/self.v were a fixed
		number, STRETCH, or a Percent (never numeric by themselves)."""
		raise NotImplementedError

	def layout(self, allocated):
		super().layout(allocated)
		x, y, w, h = self.internal_rect
		imgui.set_cursor_screen_pos((x, y))
		self.draw_content(w, h)


class ButtonWidget(Widget):
	def __init__(self, label, on_click=None, **kwargs):
		super().__init__(**kwargs)
		self.label = label
		self.on_click = on_click

	def draw_content(self, w, h):
		if imgui.button(self.label, (w, h)) and self.on_click is not None:
			self.on_click()


class LabelWidget(Widget):
	def __init__(self, text, color=None, disabled=False, **kwargs):
		super().__init__(**kwargs)
		self.text = text
		self.color = color
		self.disabled = disabled

	def draw_content(self, w, h):
		if self.disabled:
			imgui.text_disabled(self.text)
		elif self.color is not None:
			imgui.text_colored(self.color, self.text)
		else:
			imgui.text(self.text)


class ProgressBarWidget(Widget):
	def __init__(self, fraction, **kwargs):
		super().__init__(**kwargs)
		self.fraction = fraction

	def draw_content(self, w, h):
		imgui.progress_bar(self.fraction, (w, h))


class TextInputWidget(Widget):
	"""`value`/`on_change` are external state -- this Widget doesn't own its
	own text between frames (an instance rebuilt fresh every frame, as every
	Box tree here is, would otherwise reset it to "" each time)."""

	def __init__(self, hint="", value="", on_change=None, **kwargs):
		super().__init__(**kwargs)
		self.hint = hint
		self.value = value
		self.on_change = on_change

	def draw_content(self, w, h):
		imgui.set_next_item_width(w)
		changed, new_value = imgui.input_text_with_hint("##input", self.hint, self.value)
		if changed and self.on_change is not None:
			self.on_change(new_value)


class IconButtonWidget(Widget):
	"""Icon-only button (Font Awesome glyph) with a hover tooltip, tinted a
	deterministic pastel color (ui/colors.py) unless `color` is given --
	generalizes the `_icon_button` free function copied across several
	modules (explorer.py among them) into a real Box/Widget tree node, sized
	by its allotted rect rather than its own natural size."""

	def __init__(self, icon, tooltip=None, on_click=None, color=None, disabled=False, **kwargs):
		super().__init__(**kwargs)
		self.icon = icon
		self.tooltip = tooltip
		self.on_click = on_click
		self.color = color
		self.disabled = disabled

	def draw_content(self, w, h):
		imgui.push_style_color(imgui.Col_.text.value, self.color if self.color is not None else pastel_color_for(self.icon))
		imgui.begin_disabled(self.disabled)
		clicked = imgui.button(self.icon, (w, h))
		imgui.end_disabled()
		imgui.pop_style_color()
		if self.tooltip and imgui.is_item_hovered():
			imgui.set_tooltip(self.tooltip)
		if clicked and self.on_click is not None:
			self.on_click()


class ComboWidget(Widget):
	"""A combo trigger sized by its allotted rect, whose dropdown content is
	drawn by `draw_popup()` once opened -- the popup itself is a floating
	ImGui overlay window, positioned by ImGui, not by Box; there is nothing
	for Box's layout to manage inside it, so `draw_popup` stays plain,
	direct ImGui like before this system existed."""

	def __init__(self, combo_id, preview, draw_popup, disabled=False, disabled_tooltip=None, **kwargs):
		super().__init__(**kwargs)
		self.combo_id = combo_id
		self.preview = preview
		self.draw_popup = draw_popup
		self.disabled = disabled
		self.disabled_tooltip = disabled_tooltip

	def draw_content(self, w, h):
		imgui.begin_disabled(self.disabled)
		imgui.set_next_item_width(w)
		if imgui.begin_combo(self.combo_id, self.preview):
			self.draw_popup()
			imgui.end_combo()
		imgui.end_disabled()
		if self.disabled and self.disabled_tooltip and imgui.is_item_hovered():
			imgui.set_tooltip(self.disabled_tooltip)


class AutoHeightWidget(Widget):
	"""A Widget whose own height on the NEXT frame is whatever it actually
	drew THIS frame (measured via the ImGui cursor's vertical delta) --
	for content whose real height genuinely isn't knowable before drawing
	it (an expandable tree, a conditional suggestions list) and can change
	frame to frame. `Panel.resizable` already accepts this exact one-frame
	lag for its own size (box_panel.py, 2026-09-18) -- same principle, one
	level down. `v` starts at `initial_height` until the first real
	measurement lands. Subclasses override `draw_flow_content(w)`, never
	`draw_content`."""

	def __init__(self, initial_height=0.0, **kwargs):
		super().__init__(v=initial_height, **kwargs)

	def draw_content(self, w, h):
		start = imgui.get_cursor_screen_pos()
		self.draw_flow_content(w)
		end = imgui.get_cursor_screen_pos()
		# `end.y - start.y` is the CONTENT height, measured inside
		# internal_rect (margin+padding already subtracted from the
		# allocated rect by the time draw_content runs) -- but `v` is read
		# back by the PARENT next frame as this widget's own ALLOCATED size,
		# which THIS widget's own layout() will subtract margin+padding from
		# AGAIN. Storing the bare content height here would shrink by
		# 2*(margin+padding) every single frame (a runaway feedback loop,
		# not just a one-off offset) -- adding it back once here cancels
		# that exactly, keeping the real content height stable while the
		# margin/padding still visually apply.
		self.v = max(end.y - start.y, 0.0) + 2 * self.margin + 2 * self.padding

	def draw_flow_content(self, w):
		raise NotImplementedError


class SeparatorWidget(AutoHeightWidget):
	def draw_flow_content(self, w):
		imgui.separator()


class TreeListWidget(Widget):
	"""Confines its content to a real ImGui child window sized to its own
	allotted width -- unlike plain flow-drawing content (a bare
	`imgui.tree_node_ex`/`imgui.selectable` sequence, whose rows stretch to
	the ENCLOSING window's content width), a child window gives every row a
	well-defined right edge that actually respects whatever padding/margin
	this widget's own parent Box applied (Nuno, 2026-09-20: a tree drawn
	without this confinement didn't respect the panel's own padding on its
	right edge, while a properly `begin_child`-confined list already did --
	see explorer.py's `_ExplorerListWidget`, which already did this before
	this class existed).

	One widget, two modes (Nuno, 2026-09-20: "un seul widget qui puisse se
	comporter soit en arbre soit en flat"):
	- `auto_height=False` (default): `v` behaves like any other Widget's
	  (STRETCH by default, or fixed/Percent) -- fills whatever height its
	  parent gives it, scrolling internally for overflow (pass
	  `extra_window_flags` WITHOUT `no_scrollbar` for this). Same category
	  as explorer.py's own file-list widget.
	- `auto_height=True`: sizes itself to its own content across frames
	  instead (start `v` at a small numeric guess, e.g. 0.0) -- every frame,
	  the real content height is read from INSIDE the child window via
	  `imgui.get_scroll_max_y()` (the child's own scrollable overflow)
	  added to its given height, and stored back into `self.v` for the
	  NEXT frame's layout (same one-frame-lag principle as
	  `AutoHeightWidget`/`Panel.resizable` elsewhere in this module) --
	  never via the outer cursor delta `AutoHeightWidget` itself uses,
	  which wouldn't work here: a fixed-size child window always advances
	  the outer cursor by exactly the size it was given, regardless of how
	  much content is actually inside it. Pass `extra_window_flags` WITH
	  `no_scrollbar` for this mode -- the reserved space is meant to grow
	  to fit everything, not scroll.

	`draw_rows(w)` draws the actual content (a recursive expand-in-place
	tree, a flat click-to-navigate list, or anything else) inside the
	already-sized child window -- this class only owns the confinement and
	height bookkeeping, not the navigation model itself (tree vs flat stays
	the caller's own logic for now -- box_ui_migration__explorer_panel.md's
	own follow-up chantier is the real per-mode unification)."""

	# A begin_child() size.y of 0 (or negative) doesn't mean "auto-fit to
	# content" in Dear ImGui -- it means "fill whatever's left in the
	# PARENT window", which silently breaks auto_height=True's whole
	# measurement scheme (a child that always auto-fills never overflows,
	# so get_scroll_max_y() would always read 0, so `v` would never grow
	# past its own starting guess). A tiny positive floor keeps begin_child
	# in its normal fixed-size-with-real-scrolling mode instead, even on
	# the very first auto_height frame (v starting at 0.0).
	_MIN_CHILD_HEIGHT = 1.0

	def __init__(self, draw_rows, child_id, extra_window_flags=0, child_flags=0, child_bg_color=None,
	             auto_height=False, max_height=None, **kwargs):
		super().__init__(**kwargs)
		self.draw_rows = draw_rows
		self.child_id = child_id
		self.extra_window_flags = extra_window_flags
		self.child_flags = child_flags
		self.child_bg_color = child_bg_color
		self.auto_height = auto_height
		# Caps `auto_height=True`'s growth (e.g. a suggestions list capped
		# at N visible rows, scrolling -- via extra_window_flags NOT
		# including no_scrollbar -- beyond that) instead of growing
		# forever. Ignored when auto_height=False.
		self.max_height = max_height

	def draw_content(self, w, h):
		if self.auto_height:
			fixed_h = max(h, self._MIN_CHILD_HEIGHT)
			if self.max_height is not None:
				fixed_h = min(fixed_h, self.max_height)
		else:
			fixed_h = h
		if self.child_bg_color is not None:
			imgui.push_style_color(imgui.Col_.child_bg.value, self.child_bg_color)
		with imgui_ctx.begin_child(
				self.child_id, size=(w, fixed_h), child_flags=self.child_flags, window_flags=self.extra_window_flags):
			self.draw_rows(w)
			if self.auto_height:
				measured = fixed_h + imgui.get_scroll_max_y()
				if self.max_height is not None:
					measured = min(measured, self.max_height)
				# Same round-trip fix as AutoHeightWidget.draw_content(): `v`
				# is read back next frame as this widget's own ALLOCATED
				# size, which layout() subtracts margin+padding from AGAIN --
				# without adding it back here, margin/padding would shrink
				# the reserved height by 2*(margin+padding) every frame
				# (a runaway feedback loop, not a one-off offset).
				self.v = max(measured, 0.0) + 2 * self.margin + 2 * self.padding
		if self.child_bg_color is not None:
			imgui.pop_style_color()
