import dataclasses

from imgui_bundle import imgui

DEFAULT_MAX_ITEMS = 8


def draw_properties(value, max_items=DEFAULT_MAX_ITEMS):
	"""Draws a read-only property tree for any dataclass instance (nested
	dataclasses become expandable tree nodes). Large lists/dicts/bytes
	(anything over max_items, e.g. raw vertex/LOD arrays) are shown as an
	item count instead of being dumped -- keeps this generic enough to
	reuse on any pynel-parsed structure without per-type exclusion lists.
	"""
	if not dataclasses.is_dataclass(value):
		imgui.text(str(value))
		return

	for field in dataclasses.fields(value):
		_draw_field(field.name, getattr(value, field.name), max_items)


def _draw_field(name, value, max_items):
	if isinstance(value, bytes):
		imgui.text(f"{name}: <{len(value)} bytes>")
		return

	if isinstance(value, dict):
		imgui.text(f"{name}: {{{len(value)} entries}}")
		return

	if isinstance(value, (list, tuple)):
		_draw_list_field(name, value, max_items)
		return

	if dataclasses.is_dataclass(value):
		if imgui.tree_node_ex(f"##{name}", 0, name):
			draw_properties(value, max_items)
			imgui.tree_pop()
		return

	imgui.text(f"{name}: {_format_scalar(value)}")


def _draw_list_field(name, items, max_items):
	if len(items) > max_items or not any(dataclasses.is_dataclass(item) for item in items):
		imgui.text(f"{name}: [{len(items)} items]")
		return

	if imgui.tree_node_ex(f"##{name}", 0, f"{name} [{len(items)}]"):
		for index, item in enumerate(items):
			if dataclasses.is_dataclass(item):
				if imgui.tree_node_ex(f"##{name}-{index}", 0, f"[{index}]"):
					draw_properties(item, max_items)
					imgui.tree_pop()
			else:
				imgui.text(f"[{index}]: {_format_scalar(item)}")
		imgui.tree_pop()


def _format_scalar(value) -> str:
	if isinstance(value, float):
		return f"{value:.4f}"
	return str(value)
