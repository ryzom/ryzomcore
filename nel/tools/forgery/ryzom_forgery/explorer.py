import fnmatch
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from panda3d.core import KeyboardButton

from imgui_bundle import imgui, imgui_ctx

from pynel.ryzom_bnp import BnpReader, BnpError

from .commands import CommandRegistry

BNP_EXTENSIONS = (".bnp", ".bnpe")
DEFAULT_FILTER = "*"
FILTER_PRESETS = ["*.shape", "*"]


@dataclass
class ExplorerItem:
	"""A selectable entry in the explorer tree: either a real filesystem
	path, or a virtual file living inside a `.bnp` archive (in which case
	`bnp_path` is the archive's path and `name` is the entry name inside
	it, not a real path on disk).
	"""

	path: Path
	name: str
	bnp_path: Optional[Path] = None

	@property
	def suffix(self) -> str:
		return Path(self.name).suffix

	def read_bytes(self) -> bytes:
		if self.bnp_path is not None:
			return BnpReader(self.bnp_path).read_file(self.name)
		return self.path.read_bytes()


class Explorer:
	"""Standard left-panel file browser for Ryzom Forgery tool apps.

	A filesystem tree that also expands `.bnp` archives to browse their
	contents, with a name search filter, an extension filter, multi
	selection (ctrl+click to add/remove), and a right-click context menu
	built from a CommandRegistry.
	"""

	def __init__(self, app, root: Path, commands: CommandRegistry,
	             default_filter: str = DEFAULT_FILTER, extension_presets=None):
		self.app = app
		self.root = Path(root)
		self.commands = commands

		self.search = ""
		self.extension_filter = default_filter
		self.extension_presets = list(extension_presets) if extension_presets else list(FILTER_PRESETS)

		self._selection: dict[str, ExplorerItem] = {}

		# Listing a directory or a .bnp's header table is real disk I/O;
		# draw() runs every ImGui frame, so without caching this happened
		# 60 times/sec for every visible entry (this crushed the framerate
		# on a real, large Ryzom data tree). Cleared by refresh().
		self._dir_cache: dict = {}
		self._bnp_cache: dict = {}
		# Filtering a .bnp's entries was being redone every frame for every
		# .bnp row in the current directory -- even collapsed ones, just to
		# decide whether to show the row at all -- which was the real cost
		# in a folder full of .bnp archives. Cached per (bnp, filter state),
		# cleared whenever search/extension_filter actually change.
		self._bnp_visible_cache: dict = {}

		self.on_selection_changed = None  # optional callback(list[ExplorerItem])

	def selected_items(self) -> list:
		return list(self._selection.values())

	def refresh(self):
		"""Clear the cached directory/.bnp listings so the next draw() re-reads them from disk."""
		self._dir_cache.clear()
		self._bnp_cache.clear()
		self._bnp_visible_cache.clear()

	def draw(self):
		if imgui.button("Refresh"):
			self.refresh()

		imgui.push_item_width(150)
		previous_search = self.search
		previous_filter = self.extension_filter
		_, self.search = imgui.input_text("Search", self.search)

		# A single combo-like widget: its preview shows the current filter
		# (whether typed or picked), and the dropdown holds both a text box
		# for custom values and the preset list.
		if imgui.begin_combo("Filter", self.extension_filter):
			_, typed = imgui.input_text("##custom-filter", self.extension_filter)
			if typed != self.extension_filter:
				self.extension_filter = typed
			imgui.separator()
			for preset in self.extension_presets:
				clicked, _ = imgui.selectable(preset, preset == self.extension_filter)
				if clicked:
					self.extension_filter = preset
					imgui.close_current_popup()
			imgui.end_combo()
		imgui.pop_item_width()

		if self.search != previous_search or self.extension_filter != previous_filter:
			self._bnp_visible_cache.clear()

		imgui.text(str(self.root))
		imgui.separator()

		child_flags = imgui.WindowFlags_.horizontal_scrollbar.value
		with imgui_ctx.begin_child("explorer-tree", window_flags=child_flags):
			if self.root.parent != self.root:
				clicked, _ = imgui.selectable("..", False)
				if clicked:
					self.root = self.root.parent
					self._selection.clear()

			self._draw_dir(self.root)

	def _matches_filters(self, name: str) -> bool:
		if self.search and self.search.lower() not in name.lower():
			return False
		if self.extension_filter and not fnmatch.fnmatchcase(name.lower(), self.extension_filter.lower()):
			return False
		return True

	def _draw_dir(self, dir_path: Path):
		key = str(dir_path)
		entries = self._dir_cache.get(key)
		if entries is None:
			try:
				entries = sorted(dir_path.iterdir(), key=lambda p: (p.is_file(), p.name.lower()))
			except OSError:
				entries = []
			self._dir_cache[key] = entries

		for entry in entries:
			if entry.is_dir():
				self._draw_dir_node(entry)
			elif entry.suffix.lower() in BNP_EXTENSIONS:
				self._draw_bnp_node(entry)
			elif self._matches_filters(entry.name):
				self._draw_leaf(ExplorerItem(path=entry, name=entry.name))

	def _draw_dir_node(self, dir_path: Path):
		flags = imgui.TreeNodeFlags_.open_on_arrow.value
		opened = imgui.tree_node_ex(str(dir_path), flags, dir_path.name)
		if opened:
			self._draw_dir(dir_path)
			imgui.tree_pop()

	def _draw_bnp_node(self, bnp_path: Path):
		key = str(bnp_path)
		entries = self._bnp_cache.get(key)
		if entries is None:
			try:
				entries = BnpReader(bnp_path).list()
			except BnpError:
				entries = []
			self._bnp_cache[key] = entries

		visible_entries = self._bnp_visible_cache.get(key)
		if visible_entries is None:
			visible_entries = [e for e in entries if self._matches_filters(e.name)]
			self._bnp_visible_cache[key] = visible_entries

		if not visible_entries and not self._matches_filters(bnp_path.name):
			return

		flags = imgui.TreeNodeFlags_.open_on_arrow.value
		opened = imgui.tree_node_ex(str(bnp_path), flags, f"{bnp_path.name} [{len(entries)}]")
		if opened:
			# A .bnp's contents are always a flat list (no sub-folders in the
			# format), so it's a good fit for ImGuiListClipper: a .bnp with
			# thousands of entries was redrawing every single row every
			# frame, which is what tanked the framerate.
			clipper = imgui.ListClipper()
			clipper.begin(len(visible_entries))
			while clipper.step():
				for i in range(clipper.display_start, clipper.display_end):
					entry = visible_entries[i]
					self._draw_leaf(ExplorerItem(path=bnp_path, name=entry.name, bnp_path=bnp_path))
			imgui.tree_pop()

	def _draw_leaf(self, item: ExplorerItem):
		key = self._item_key(item)
		flags = imgui.TreeNodeFlags_.leaf.value | imgui.TreeNodeFlags_.no_tree_push_on_open.value
		if key in self._selection:
			flags |= imgui.TreeNodeFlags_.selected.value

		imgui.tree_node_ex(key, flags, item.name)

		if imgui.is_item_clicked():
			additive = self.app.mouseWatcherNode.isButtonDown(KeyboardButton.control())
			self._select(item, key, additive)

		if imgui.begin_popup_context_item(f"ctx-{key}"):
			if key not in self._selection:
				self._select(item, key, additive=False)
			self._draw_context_menu_items()
			imgui.end_popup()

	def _select(self, item: ExplorerItem, key: str, additive: bool):
		if not additive:
			self._selection.clear()

		if key in self._selection:
			del self._selection[key]
		else:
			self._selection[key] = item

		if self.on_selection_changed:
			self.on_selection_changed(self.selected_items())

	def _item_key(self, item: ExplorerItem) -> str:
		if item.bnp_path is not None:
			return f"{item.bnp_path}!{item.name}"
		return str(item.path)

	def _draw_context_menu_items(self):
		items = self.selected_items()
		for command in self.commands.commands_for_selection(items):
			clicked, _ = imgui.menu_item(command.label, "", False, True)
			if clicked:
				command.callback(items)
