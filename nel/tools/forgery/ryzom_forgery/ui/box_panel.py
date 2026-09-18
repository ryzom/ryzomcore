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
			imgui.set_next_window_size((w, h), cond=imgui.Cond_.first_use_ever.value)
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


class Widget(Box):
	"""A Box that draws REAL ImGui content into its own internal_rect
	instead of laying out further Box children -- override draw_content()
	in a subclass (see ButtonWidget/LabelWidget/TextInputWidget below).
	Nuno, 2026-09-18: "il faut que les box puissent gerer aussi comme
	enfants d'autres widgets"."""

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
	def __init__(self, hint="", **kwargs):
		super().__init__(**kwargs)
		self.hint = hint
		self.value = ""

	def draw_content(self, w, h):
		imgui.set_next_item_width(w)
		_changed, self.value = imgui.input_text_with_hint("##input", self.hint, self.value)
