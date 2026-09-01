"""Tiny shared helper for imgui.begin_popup_modal() call sites -- every
modal popup in Forgery is meant to open centered on the main viewport
(imgui.begin_popup_modal() itself doesn't position the window at all,
defaulting to wherever ImGui last left the cursor/layout).
"""

from imgui_bundle import imgui


def center_next_popup(always=False):
	"""Call once, right before imgui.begin_popup_modal(), every frame the
	popup might be drawn. By default uses Cond_.appearing, so this only
	actually repositions the window the frame it first opens, letting the
	user drag it afterwards (while it stays open across frames) as
	expected -- it isn't forced back to center every single frame.

	Pass always=True for a popup that can open on literally the very
	first ImGui frame (e.g. object_editor.py's reopen-last-shape prompt,
	queued from __init__ before the first draw_ui() call, or the
	mandatory workspace setup popup) -- Panda3D's requested window
	geometry (self.win.requestProperties()) is only a request to the
	window manager, not synchronously applied, so get_main_viewport() can
	still report a stale/transient size on that very first frame;
	Cond_.appearing would then lock the popup off-center forever since it
	only centers once. always=True keeps recentering every frame instead,
	trading away drag-to-reposition for a popup that self-corrects once
	the real window geometry lands -- an acceptable trade for a
	short-lived confirmation nobody drags around."""
	viewport = imgui.get_main_viewport()
	center = (viewport.pos.x + viewport.size.x / 2.0, viewport.pos.y + viewport.size.y / 2.0)
	cond = imgui.Cond_.always.value if always else imgui.Cond_.appearing.value
	imgui.set_next_window_pos(center, cond, (0.5, 0.5))
