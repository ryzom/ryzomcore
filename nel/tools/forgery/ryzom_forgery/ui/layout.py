"""Two small, targeted layout primitives -- NOT a general-purpose HBox/VBox
layout engine, see project-todos/forgery/ui_module_consolidation.md for why
that's deliberately out of scope. ImGui has no built-in concept of vertically
centering items of different heights on the same same_line() row (a plain
text item is imgui.get_text_line_height() tall, a button/progress_bar is the
taller imgui.get_frame_height()) -- these two functions exist only because
Forgery actually needed them (project-todos/forgery/
unified_background_task_status_bar.md's status bar), not as a speculative
general toolkit.

Validated once against a real render before being trusted anywhere -- see
project-todos/forgery/ui_module_consolidation.md's own TEST steps (Claude
Code cannot render Panda3D/ImGui itself, see the
forgery-ui-polish-needs-screenshot memory note).
"""

from imgui_bundle import imgui


def center_x(content_width: float):
	"""Call instead of imgui.same_line() to start a `content_width`-wide
	group of items horizontally centered in the CURRENT window -- e.g. a
	label + a progress bar drawn right after this call, same_line()'d
	between them, whose combined width is `content_width`."""
	imgui.same_line(max((imgui.get_window_width() - content_width) / 2.0, 0.0))


def center_v(item_height: float, container_height: float = None):
	"""Call right before drawing an item `item_height` tall that sits on a
	same_line() row inside a window that auto-fits its OWN height to the
	tallest item on that row (see ui/panel.py's Panel(size=None) -- exactly
	sysinfo.py's status bar: the window ends up exactly imgui.get_frame_height()
	tall to fit its progress bar, taller than the plain text drawn beside
	it). `container_height` defaults to imgui.get_frame_height() for that
	reason -- pass it explicitly for any other container height. Nudges the
	cursor down so `item_height`'s own vertical CENTER lines up with the
	container's center, instead of sitting at the row's top (ImGui's
	default, which leaves a shorter item looking stuck to the top of a
	taller row, or a taller item hanging off the bottom of a shorter one).

	Deliberately a RELATIVE offset from wherever the cursor already is, not
	an absolute recompute from imgui.get_window_height() -- an earlier
	attempt at the latter forgot to account for the window's own
	WindowPadding and pushed content out of view entirely (reverted
	2026-09-18, see sysinfo.py's git history). A relative offset needs no
	knowledge of padding at all, only the item's height and the row's own
	tallest-item height, both of which imgui already reports accurately
	regardless of style/DPI scale."""
	if container_height is None:
		container_height = imgui.get_frame_height()
	offset = (container_height - item_height) / 2.0
	if offset > 0:
		imgui.set_cursor_pos_y(imgui.get_cursor_pos_y() + offset)
