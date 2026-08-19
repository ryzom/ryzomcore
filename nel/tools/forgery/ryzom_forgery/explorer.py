import fnmatch
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from panda3d.core import KeyboardButton

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx

from pynel.ryzom_bnp import BnpReader, BnpError

from . import settings as app_settings
from .commands import CommandRegistry

BNP_EXTENSIONS = (".bnp", ".bnpe")
DEFAULT_FILTER = "*"
FILTER_PRESETS = ["*.shape", "*.skel", "*.anim", "*"]
# Leaf-file icons by extension -- a plain filename alone doesn't read at a
# glance as "this is a 3D mesh" vs "this is a skeleton" vs "this is an
# animation clip" the way folders/.bnp archives already have their own icon.
_LEAF_ICONS = {
	".shape": fa_icons.ICON_FA_CUBE,
	# ICON_FA_BONE/ICON_FA_WALKING (tried first) are Font Awesome 5 additions --
	# the actual font file loaded here (app.py's _ICON_FONT_PATH,
	# fontawesome-webfont.ttf) is genuine FA 4.7, which doesn't have those
	# glyphs at all (silently invisible, not even a fallback box). Stuck to
	# icons confirmed present in the classic FA4 set instead.
	".skel": fa_icons.ICON_FA_MALE,
	".anim": fa_icons.ICON_FA_FILM,
}
_DEFAULT_LEAF_ICON = fa_icons.ICON_FA_FILE
_MAX_VISIBLE_PATH_SUGGESTIONS = 8  # box scrolls instead of growing past this many rows
_FAVORITE_STAR_COLOR = (1.0, 0.8, 0.0, 1.0)
_NON_FAVORITE_STAR_COLOR = (0.5, 0.5, 0.5, 1.0)


def _icon_button(icon, tooltip):
	"""An icon-only button (Font Awesome glyph, see ryzom_forgery.app's
	_load_icon_font) with a hover tooltip, since an icon alone isn't always
	self-explanatory."""
	clicked = imgui.button(icon)
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked


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

		self._favorites = app_settings.load().explorer_favorites

		self._path_input = str(self.root)  # edited text of the path bar, synced from self.root when it changes elsewhere
		self._path_input_root = self.root  # which self.root the above was last synced from
		self._path_suggestions: list[Path] = []  # this frame's folder-name autocomplete candidates
		self._path_suggestion_index = -1  # keyboard-highlighted suggestion, -1 = none
		self._focus_path_input = False  # consumed once, right before the next input_text("##path", ...) call

		self._current_bnp: Optional[Path] = None  # set while browsing a .bnp's flat entry list; None = browsing self.root
		self._show_hidden = False  # dotfiles/dotfolders, toggled via the list's right-click context menu

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
		self.extra_toolbar = None  # optional callback(), draws extra icon buttons next to Refresh

	def selected_items(self) -> list:
		return list(self._selection.values())

	def refresh(self):
		"""Clear the cached directory/.bnp listings so the next draw() re-reads them from disk."""
		self._dir_cache.clear()
		self._bnp_cache.clear()
		self._bnp_visible_cache.clear()

	def _navigate_to(self, path: Path):
		self.root = path
		self._current_bnp = None
		self._selection.clear()

	def _enter_bnp(self, bnp_path: Path):
		self._current_bnp = bnp_path
		self._selection.clear()

	def _exit_bnp(self):
		self._current_bnp = None
		self._selection.clear()

	def _draw_path_bar(self):
		# _path_input is the text being edited; resynced from self.root
		# whenever something else changed it (favorites, "..", autocomplete
		# pick) rather than every frame, so mid-edit keystrokes aren't clobbered.
		if self._path_input_root != self.root:
			self._path_input = str(self.root)
			self._path_input_root = self.root
			self._path_suggestion_index = -1

		if self._focus_path_input:
			imgui.set_keyboard_focus_here()
			self._focus_path_input = False

		imgui.set_next_item_width(-1)
		flags = imgui.InputTextFlags_.enter_returns_true.value
		submitted, new_text = imgui.input_text("##path", self._path_input, flags)
		if new_text != self._path_input:
			self._path_suggestion_index = -1
		self._path_input = new_text

		input_active = imgui.is_item_active()
		if input_active:
			self._path_suggestions = self._compute_path_suggestions()

		if input_active and self._path_suggestions:
			if imgui.is_key_pressed(imgui.Key.down_arrow):
				self._path_suggestion_index = min(self._path_suggestion_index + 1, len(self._path_suggestions) - 1)
			elif imgui.is_key_pressed(imgui.Key.up_arrow):
				self._path_suggestion_index = max(self._path_suggestion_index - 1, -1)

		if submitted:
			if 0 <= self._path_suggestion_index < len(self._path_suggestions):
				self._select_suggestion(self._path_suggestions[self._path_suggestion_index])
			else:
				self._try_navigate_to(self._path_input)

		# Whether to keep showing the box this frame can't just be
		# `input_active`: clicking a suggestion below moves focus off the
		# input on that very click, before the click itself is delivered to
		# the Selectable, which closed the box out from under the click and
		# ate it. Keeping the box open as long as it's hovered/focused too
		# (a plain geometric/z-order check, unaffected by that focus race)
		# lets the click land normally, like any other Selectable elsewhere.
		box_interacted = False
		if self._path_suggestions:
			box_interacted = self._draw_path_suggestions_popup()
		if not input_active and not box_interacted:
			self._path_suggestions = []

	def _try_navigate_to(self, text: str):
		candidate = Path(text).expanduser()
		if candidate.is_dir():
			self._navigate_to(candidate)

	def _select_suggestion(self, candidate: Path):
		"""Picking a suggestion (click or Enter) navigates there, appends a
		trailing `/` (it's necessarily a directory) and refocuses the path
		bar with fresh suggestions for it already showing -- so typing a
		path can flow continuously, slash by slash, without touching the mouse."""
		self._navigate_to(candidate)
		self._path_input = f"{candidate}/"
		self._path_input_root = self.root  # matches self.root already -- keeps the trailing "/" from being wiped next frame
		self._path_suggestions = []
		self._path_suggestion_index = -1
		self._focus_path_input = True

	def _compute_path_suggestions(self) -> list:
		"""Folder-name autocomplete candidates: sibling directories whose
		name starts with whatever's typed after the last `/` -- or, if the
		text itself already ends with `/`, every child of that directory."""
		typed = self._path_input
		if not typed:
			return []

		if typed.endswith(("/", "\\")):
			parent = Path(typed)
			prefix = ""
		else:
			parent = Path(typed).parent
			prefix = Path(typed).name
		try:
			if not parent.is_dir():
				return []
			candidates = sorted(
				(p for p in parent.iterdir()
				 if p.is_dir() and p.name.lower().startswith(prefix.lower()) and not self._is_hidden(p.name)),
				key=lambda p: p.name.lower())
		except OSError:
			return []

		if prefix and len(candidates) == 1 and candidates[0].name.lower() == prefix.lower():
			return []
		return candidates

	def _draw_path_suggestions_popup(self) -> bool:
		"""Bordered box right under the path bar, in the normal layout flow
		(a freestanding overlay window was tried first, but clicking inside
		it never registered -- moving focus off the path input mid-click
		seemingly broke selectable hit-testing for that same click). Returns
		whether the box is currently hovered/focused, so the caller can keep
		it open across that same click despite the input having lost focus."""
		style = imgui.get_style()
		visible_rows = min(len(self._path_suggestions), _MAX_VISIBLE_PATH_SUGGESTIONS)
		# A little extra beyond the exact line-height sum -- without it, the
		# child's own content region came out a couple pixels short of what
		# it actually needed, permanently showing a scrollbar even when all
		# rows already fit.
		height = imgui.get_text_line_height_with_spacing() * visible_rows + style.window_padding.y * 2 + 4
		imgui.push_style_color(imgui.Col_.child_bg.value, (0.15, 0.15, 0.15, 1.0))
		with imgui_ctx.begin_child(
				"##path-suggestions", size=(0, height),
				child_flags=imgui.ChildFlags_.borders.value):
			interacted = imgui.is_window_hovered() or imgui.is_window_focused()
			for i, candidate in enumerate(self._path_suggestions):
				clicked, _ = imgui.selectable(candidate.name, i == self._path_suggestion_index)
				if clicked:
					self._select_suggestion(candidate)
		imgui.pop_style_color()
		return interacted

	def _draw_favorites(self):
		is_favorite = str(self.root) in self._favorites
		imgui.push_style_color(imgui.Col_.text.value, _FAVORITE_STAR_COLOR if is_favorite else _NON_FAVORITE_STAR_COLOR)
		tooltip = "Remove current folder from favorites" if is_favorite else "Add current folder to favorites"
		if _icon_button(fa_icons.ICON_FA_STAR, tooltip):
			self._toggle_favorite(str(self.root))
		imgui.pop_style_color()

		imgui.same_line()
		if imgui.begin_combo("##favorites", "Favorites"):
			if not self._favorites:
				imgui.text_disabled("(none yet -- click the star to add the current folder)")
			for favorite in list(self._favorites):
				imgui.push_id(favorite)
				imgui.push_style_color(imgui.Col_.text.value, _FAVORITE_STAR_COLOR)
				if _icon_button(fa_icons.ICON_FA_STAR, "Remove from favorites"):
					self._toggle_favorite(favorite)
				imgui.pop_style_color()
				imgui.same_line()
				clicked, _ = imgui.selectable(Path(favorite).name or favorite, False)
				if imgui.is_item_hovered():
					imgui.set_tooltip(favorite)
				if clicked:
					self._navigate_to(Path(favorite))
					imgui.close_current_popup()
				imgui.pop_id()
			imgui.end_combo()

	def _toggle_favorite(self, path: str):
		if path in self._favorites:
			self._favorites.remove(path)
		else:
			self._favorites.append(path)
		# Re-loads fresh and overwrites only our own section -- other
		# components (export, search paths, data_root) persist independently
		# and may have changed their own section since our own __init__.
		fresh = app_settings.load()
		fresh.explorer_favorites = self._favorites
		app_settings.save(fresh)

	def draw(self):
		if _icon_button(fa_icons.ICON_FA_SYNC, "Refresh"):
			self.refresh()
		if self.extra_toolbar is not None:
			imgui.same_line()
			self.extra_toolbar()

		previous_search = self.search
		previous_filter = self.extension_filter

		imgui.set_next_item_width(150)
		_, self.search = imgui.input_text_with_hint("##search", "Search...", self.search)
		imgui.same_line()

		# A single combo-like widget: its preview shows the current filter
		# (whether typed or picked), and the dropdown holds both a text box
		# for custom values and the preset list.
		imgui.set_next_item_width(150)
		if imgui.begin_combo("##filter", self.extension_filter):
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

		if self.search != previous_search or self.extension_filter != previous_filter:
			self._bnp_visible_cache.clear()

		self._draw_path_bar()
		self._draw_favorites()
		imgui.separator()

		child_flags = imgui.WindowFlags_.horizontal_scrollbar.value
		with imgui_ctx.begin_child("explorer-list", window_flags=child_flags):
			if self._current_bnp is not None:
				self._draw_bnp_contents(self._current_bnp)
			else:
				self._draw_dir_contents(self.root)
			self._draw_list_context_menu()

	def _draw_list_context_menu(self):
		"""Right-click on empty space in the list (not on an item, so it
		doesn't fight with _draw_leaf's own per-item context menu)."""
		flags = imgui.PopupFlags_.mouse_button_right.value | imgui.PopupFlags_.no_open_over_items.value
		if imgui.begin_popup_context_window("explorer-list-context", flags):
			_, self._show_hidden = imgui.menu_item("Show hidden files/folders", "", self._show_hidden, True)
			imgui.end_popup()

	def _is_hidden(self, name: str) -> bool:
		return not self._show_hidden and name.startswith(".")

	def _matches_filters(self, name: str) -> bool:
		if self.search and self.search.lower() not in name.lower():
			return False
		if self.extension_filter and not fnmatch.fnmatchcase(name.lower(), self.extension_filter.lower()):
			return False
		return True

	def _draw_dir_contents(self, dir_path: Path):
		"""Flat, single-click listing of `dir_path`'s own entries only --
		clicking a sub-folder or `.bnp` navigates into it (no tree/expand)."""
		if dir_path.parent != dir_path:
			clicked, _ = imgui.selectable("..", False)
			if clicked:
				self._navigate_to(dir_path.parent)

		key = str(dir_path)
		entries = self._dir_cache.get(key)
		if entries is None:
			try:
				entries = sorted(dir_path.iterdir(), key=lambda p: (p.is_file(), p.name.lower()))
			except OSError:
				entries = []
			self._dir_cache[key] = entries

		for entry in entries:
			if self._is_hidden(entry.name):
				continue
			if entry.is_dir():
				clicked, _ = imgui.selectable(f"{fa_icons.ICON_FA_FOLDER} {entry.name}", False)
				if clicked:
					self._navigate_to(entry)
			elif entry.suffix.lower() in BNP_EXTENSIONS:
				if not self._matches_filters(entry.name) and not self._bnp_has_visible_entries(entry):
					continue
				clicked, _ = imgui.selectable(f"{fa_icons.ICON_FA_ARCHIVE} {entry.name}", False)
				if clicked:
					self._enter_bnp(entry)
			elif self._matches_filters(entry.name):
				self._draw_leaf(ExplorerItem(path=entry, name=entry.name))

	def _bnp_has_visible_entries(self, bnp_path: Path) -> bool:
		return bool(self._bnp_visible_entries(bnp_path))

	def _bnp_visible_entries(self, bnp_path: Path) -> list:
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
		return visible_entries

	def _draw_bnp_contents(self, bnp_path: Path):
		clicked, _ = imgui.selectable("..", False)
		if clicked:
			self._exit_bnp()

		visible_entries = self._bnp_visible_entries(bnp_path)

		# A .bnp's contents are always a flat list (no sub-folders in the
		# format), so it's a good fit for ImGuiListClipper: a .bnp with
		# thousands of entries was redrawing every single row every frame,
		# which is what tanked the framerate.
		clipper = imgui.ListClipper()
		clipper.begin(len(visible_entries))
		while clipper.step():
			for i in range(clipper.display_start, clipper.display_end):
				entry = visible_entries[i]
				self._draw_leaf(ExplorerItem(path=bnp_path, name=entry.name, bnp_path=bnp_path))

	def _draw_leaf(self, item: ExplorerItem):
		key = self._item_key(item)
		flags = imgui.TreeNodeFlags_.leaf.value | imgui.TreeNodeFlags_.no_tree_push_on_open.value
		if key in self._selection:
			flags |= imgui.TreeNodeFlags_.selected.value

		icon = _LEAF_ICONS.get(item.suffix.lower(), _DEFAULT_LEAF_ICON)
		imgui.tree_node_ex(key, flags, f"{icon} {item.name}")

		if imgui.is_item_clicked():
			additive = self.app.mouseWatcherNode.isButtonDown(KeyboardButton.control())
			self._select(item, key, additive)
			if imgui.is_mouse_double_clicked(0):
				# Runs whatever the context menu's own first entry would --
				# same commands, same order (CommandRegistry.commands_for_selection()
				# already preserves registration order), just without making
				# the user right-click then pick it explicitly.
				commands = self.commands.commands_for_selection([item])
				if commands:
					commands[0].callback([item])

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
