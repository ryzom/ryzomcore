import fnmatch
import threading
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from panda3d.core import KeyboardButton, PNMImage, Texture as PandaTexture

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui, imgui_ctx

from pynel.ryzom_bnp import BnpReader, BnpError

from . import settings as app_settings
from .commands import CommandRegistry
from .icon_colors import pastel_color_for

BNP_EXTENSIONS = (".bnp", ".bnpe")
DEFAULT_FILTER = "*"
FILTER_PRESETS = ["*.shape", "*.skel", "*.anim", "*"]
# Leaf-file icons by extension -- a plain filename alone doesn't read at a
# glance as "this is a 3D mesh" vs "this is a skeleton" vs "this is an
# animation clip" the way folders/.bnp archives already have their own icon.
_LEAF_ICONS = {
	".shape": fa_icons.ICON_FA_CUBE,
	".skel": fa_icons.ICON_FA_PERSON,
	".anim": fa_icons.ICON_FA_FILM,
}
_DEFAULT_LEAF_ICON = fa_icons.ICON_FA_FILE
# Formats load_panda_texture() can actually decode (see shape_geometry.py) --
# a real thumbnail replaces the generic file icon for these in the Wexplorer
# (see _draw_leaf()'s show_thumbnail param).
_TEXTURE_EXTENSIONS = {".tga", ".dds", ".png", ".jpg", ".jpeg", ".bmp"}
_THUMBNAIL_SIZE = 16  # inline with the row's own text, not a separate big preview
# Downscaled to this before ever reaching the GPU -- a source texture can be
# up to 4096x4096 (a real one seen in testing took over a second to decode
# and 100+ms just to upload at full size, for a 16px inline icon); this many
# concurrent background decode threads at once, so expanding a folder with
# hundreds of files doesn't spawn hundreds of threads together.
_THUMBNAIL_DECODE_SIZE = 32
_MAX_CONCURRENT_THUMBNAIL_DECODES = 4
_MAX_VISIBLE_PATH_SUGGESTIONS = 8  # box scrolls instead of growing past this many rows
_FAVORITE_STAR_COLOR = (1.0, 0.8, 0.0, 1.0)
_NON_FAVORITE_STAR_COLOR = (0.5, 0.5, 0.5, 1.0)


def _icon_button(icon, tooltip, color=None):
	"""An icon-only button (Font Awesome glyph, see ryzom_forgery.app's
	_load_icon_font) with a hover tooltip, since an icon alone isn't always
	self-explanatory. Tinted a deterministic pastel color (see icon_colors.py)
	unless `color` is given -- needed by _draw_favorites()'s star, whose
	gold/gray already means something (favorited or not), which a pastel
	tint would otherwise silently override."""
	imgui.push_style_color(imgui.Col_.text.value, color if color is not None else pastel_color_for(icon))
	clicked = imgui.button(icon)
	imgui.pop_style_color()
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
		# Wexplorer texture thumbnails (see _get_thumbnail_ref()) -- own
		# state, separate from any shape-editing app's own texture state
		# (e.g. object_editor.py's _texture_cache, which is shape-load-
		# lifecycle-scoped), keyed by absolute file path. Never cleared: a
		# workspace's own tex/ folder isn't expected to grow into the
		# thousands the way a full Ryzom data tree might. Decoding a real
		# texture file (some Ryzom textures are 4096x4096) can take over a
		# second synchronously -- _decode_thumbnail_worker() does the actual
		# file read + decode + downscale off the main/render thread (own
		# background thread per texture, same reasoning as
		# search_paths_dialog.py's own _reload_worker()); _thumbnail_lock
		# guards the two dicts below since both threads touch them.
		self._thumbnail_lock = threading.Lock()
		self._thumbnail_pending: set = set()  # paths currently being decoded in the background
		self._thumbnail_ready: dict = {}  # path -> decoded (already downscaled) PNMImage, or None if decode failed -- consumed (popped) by _get_thumbnail_ref() on the main thread, which does the actual (cheap, small-image) GPU upload
		self._thumbnail_tex_refs: dict = {}  # path -> uploaded ImTextureRef, main-thread-only

		self.on_selection_changed = None  # optional callback(list[ExplorerItem])
		self.extra_header = None  # optional callback(), drawn first, above the Wexplorer (see draw())
		self.extra_toolbar = None  # optional callback(), draws extra icon buttons next to Refresh
		# [(label, Path), ...] -- each drawn as its own expandable tree section
		# above Favorites, always reachable regardless of where the Explorer is
		# currently browsing (e.g. object_editor.py's active-workspace
		# folders). Unlike the main flat/click-to-navigate view
		# (_draw_dir_contents()), sub-folders expand in place here.
		self.pinned_folders = []
		# Optional callable() -> {category_name: [Path, ...]} -- when set
		# (e.g. object_editor.py's active workspace, see the
		# forgery-workspace-projects chantier), drawn the same way as
		# pinned_folders above (persistent, expandable, above Favorites)
		# but flat per category rather than mirroring real subfolders: a
		# workspace's files are grouped by virtual category (shapes/
		# textures/3d files/masks/anims/skels/others -- see
		# virtual_categories.py) regardless of where they actually live on
		# disk. Deliberately decoupled from virtual_categories.py itself
		# (no import here) -- the host app owns the scan/caching/exclusion-
		# rules concerns, this just draws whatever dict it hands back, in
		# the order its keys already come in (virtual_categories.scan_workspace()
		# builds them in a stable, deliberate category order).
		self.virtual_categories_source = None

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

	def _draw_pinned_folders(self):
		for label, path in self.pinned_folders:
			if not path.is_dir():
				continue
			visible_count = sum(1 for entry in self._list_dir(path) if not self._is_hidden(entry.name))
			imgui.push_id(str(path))
			opened = imgui.tree_node_ex(
				"##pinned-root", imgui.TreeNodeFlags_.open_on_arrow.value,
				f"{fa_icons.ICON_FA_FOLDER_OPEN} {label} ({visible_count})")
			if opened:
				self._draw_tree_children(path)
				imgui.tree_pop()
			imgui.pop_id()

	def _draw_pinned_virtual_categories(self):
		"""Same visual slot/pattern as _draw_pinned_folders() above (one
		expandable tree section each), but for virtual_categories_source()'s
		category -> files buckets instead of real subfolders -- each
		category is a flat list (no further nesting: the whole point is
		that a file's real location on disk no longer matters), reusing
		_draw_leaf() as-is so selecting/double-click-to-load/the
		right-click command menu all behave exactly like every other entry
		in the Wexplorer."""
		if self.virtual_categories_source is None:
			return
		buckets = self.virtual_categories_source()
		for category, paths in buckets.items():
			imgui.push_id(f"virtual-{category}")
			opened = imgui.tree_node_ex(
				"##virtual-category-root", imgui.TreeNodeFlags_.open_on_arrow.value,
				f"{fa_icons.ICON_FA_FOLDER_OPEN} {category} ({len(paths)})")
			if opened:
				for entry in sorted(paths, key=lambda path: path.name.lower()):
					self._draw_leaf(ExplorerItem(path=entry, name=entry.name), show_thumbnail=True)
				imgui.tree_pop()
			imgui.pop_id()

	def _draw_tree_children(self, dir_path: Path):
		"""Recursive, expand-in-place listing of `dir_path`'s own entries --
		unlike _draw_dir_contents() (the main view: flat, click-to-navigate,
		affected by the shared search/extension_filter), the Wexplorer is a
		persistent browse-everything pane: every file shows regardless of
		extension/search (not filtered by the main arborescence's own search
		box), each with its own icon (_draw_leaf()'s _LEAF_ICONS, extended
		with a real thumbnail for texture files -- see
		_draw_leaf()'s show_thumbnail param / _get_thumbnail_ref()).
		Otherwise reuses _draw_leaf() as-is, so selecting/double-click-to-
		load/the right-click command menu all behave exactly like the main
		view."""
		for entry in self._list_dir(dir_path):
			if self._is_hidden(entry.name):
				continue
			if entry.is_dir():
				visible_count = sum(1 for child in self._list_dir(entry) if not self._is_hidden(child.name))
				opened = imgui.tree_node_ex(
					str(entry), imgui.TreeNodeFlags_.open_on_arrow.value,
					f"{fa_icons.ICON_FA_FOLDER} {entry.name} ({visible_count})")
				if opened:
					self._draw_tree_children(entry)
					imgui.tree_pop()
			elif entry.suffix.lower() in BNP_EXTENSIONS:
				opened = imgui.tree_node_ex(
					str(entry), imgui.TreeNodeFlags_.open_on_arrow.value, f"{fa_icons.ICON_FA_BOX_ARCHIVE} {entry.name}")
				if opened:
					for bnp_entry in self._bnp_entries(entry):
						self._draw_leaf(
							ExplorerItem(path=entry, name=bnp_entry.name, bnp_path=entry), show_thumbnail=True)
					imgui.tree_pop()
			else:
				self._draw_leaf(ExplorerItem(path=entry, name=entry.name), show_thumbnail=True)

	def _draw_favorites(self):
		is_favorite = str(self.root) in self._favorites
		tooltip = "Remove current folder from favorites" if is_favorite else "Add current folder to favorites"
		star_color = _FAVORITE_STAR_COLOR if is_favorite else _NON_FAVORITE_STAR_COLOR
		if _icon_button(fa_icons.ICON_FA_STAR, tooltip, color=star_color):
			self._toggle_favorite(str(self.root))

		imgui.same_line()
		if imgui.begin_combo("##favorites", "Favorites"):
			if not self._favorites:
				imgui.text_disabled("(none yet -- click the star to add the current folder)")
			for favorite in list(self._favorites):
				imgui.push_id(favorite)
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
		# Top-to-bottom order: host app's own header (e.g. object_editor.py's
		# active-workspace row), the Wexplorer (pinned folders, always fully
		# reachable), the Refresh/Import toolbar, then everything that
		# filters/navigates the main flat arborescence below (search+filter,
		# path bar, Favorites).
		if self.extra_header is not None:
			self.extra_header()
			imgui.separator()

		self._draw_pinned_folders()
		self._draw_pinned_virtual_categories()
		imgui.separator()

		if _icon_button(fa_icons.ICON_FA_ARROWS_ROTATE, "Refresh"):
			self.refresh()
		if self.extra_toolbar is not None:
			imgui.same_line()
			self.extra_toolbar()
		imgui.separator()

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

	def _list_dir(self, dir_path: Path) -> list:
		"""dir_path's own entries, sorted (folders/.bnp first, alphabetical) --
		cached (see __init__'s _dir_cache comment: draw() runs every ImGui
		frame, real disk I/O isn't free at that rate), cleared by refresh()."""
		key = str(dir_path)
		entries = self._dir_cache.get(key)
		if entries is None:
			try:
				entries = sorted(dir_path.iterdir(), key=lambda p: (p.is_file(), p.name.lower()))
			except OSError:
				entries = []
			self._dir_cache[key] = entries
		return entries

	def _draw_dir_contents(self, dir_path: Path):
		"""Flat, single-click listing of `dir_path`'s own entries only --
		clicking a sub-folder or `.bnp` navigates into it (no tree/expand)."""
		if dir_path.parent != dir_path:
			clicked, _ = imgui.selectable("..", False)
			if clicked:
				self._navigate_to(dir_path.parent)

		for entry in self._list_dir(dir_path):
			if self._is_hidden(entry.name):
				continue
			if entry.is_dir():
				clicked, _ = imgui.selectable(f"{fa_icons.ICON_FA_FOLDER} {entry.name}", False)
				if clicked:
					self._navigate_to(entry)
			elif entry.suffix.lower() in BNP_EXTENSIONS:
				if not self._matches_filters(entry.name) and not self._bnp_has_visible_entries(entry):
					continue
				clicked, _ = imgui.selectable(f"{fa_icons.ICON_FA_BOX_ARCHIVE} {entry.name}", False)
				if clicked:
					self._enter_bnp(entry)
			elif self._matches_filters(entry.name):
				self._draw_leaf(ExplorerItem(path=entry, name=entry.name))

	def _bnp_entries(self, bnp_path: Path) -> list:
		"""Every entry in `bnp_path`, unfiltered -- cached (a .bnp's own
		header table is real disk I/O, see __init__'s _bnp_cache comment)."""
		key = str(bnp_path)
		entries = self._bnp_cache.get(key)
		if entries is None:
			try:
				entries = BnpReader(bnp_path).list()
			except BnpError:
				entries = []
			self._bnp_cache[key] = entries
		return entries

	def _bnp_has_visible_entries(self, bnp_path: Path) -> bool:
		return bool(self._bnp_visible_entries(bnp_path))

	def _bnp_visible_entries(self, bnp_path: Path) -> list:
		key = str(bnp_path)
		visible_entries = self._bnp_visible_cache.get(key)
		if visible_entries is None:
			visible_entries = [e for e in self._bnp_entries(bnp_path) if self._matches_filters(e.name)]
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

	def _get_thumbnail_ref(self, path: Path):
		"""Resolves+decodes+caches an ImTextureRef thumbnail for the image
		file at `path`. Decoding is kicked off on a background thread the
		first time a path is requested (see _decode_thumbnail_worker()) --
		some real Ryzom textures are 4096x4096, over a second to decode
		synchronously, which stalled the whole frame the first time a folder
		full of them was expanded. Returns None (falls back to the generic
		file icon) while the decode is still pending or failed; once ready,
		the main thread does the actual GPU upload here (cheap: the
		decoded image is already downscaled, see _THUMBNAIL_DECODE_SIZE)."""
		key = str(path)
		tex_ref = self._thumbnail_tex_refs.get(key)
		if tex_ref is not None:
			return tex_ref

		with self._thumbnail_lock:
			if key in self._thumbnail_ready:
				pnm_image = self._thumbnail_ready.pop(key)
			elif key in self._thumbnail_pending:
				return None
			elif len(self._thumbnail_pending) >= _MAX_CONCURRENT_THUMBNAIL_DECODES:
				return None  # retried next frame once a decode slot frees up
			else:
				self._thumbnail_pending.add(key)
				threading.Thread(target=self._decode_thumbnail_worker, args=(key,), daemon=True).start()
				return None

		if pnm_image is None:
			return None
		texture = PandaTexture()
		texture.load(pnm_image)
		# loadTexture() flips the given Texture in-place (see
		# object_editor.py's _get_preview_texture_ref() for the same note) --
		# harmless here since this Texture is never used for anything besides
		# this one imgui thumbnail (own dedicated cache, not shared with any
		# live 3D material).
		tex_ref = self.app.imgui.loadTexture(texture)
		self._thumbnail_tex_refs[key] = tex_ref
		return tex_ref

	def _decode_thumbnail_worker(self, key: str):
		"""Runs off the main/render thread (own thread per texture, see
		_get_thumbnail_ref()): resolves+decodes the image at `key` via
		shape_geometry.load_panda_texture() (an absolute path short-circuits
		its usual by-name search-dir resolution, see resolve_texture_ref()),
		then downscales it to _THUMBNAIL_DECODE_SIZE (a 16px inline icon
		never needs the source's full resolution) before handing it back --
		the GPU upload itself still has to happen on the main thread (Panda3D
		graphics calls aren't thread-safe), but is fast once the image is
		this small. Imported locally, not at module level: shape_geometry.py
		-> search_paths.py -> explorer.py (for BNP_EXTENSIONS) is already a
		cycle, which a top-level import here would turn into an unimportable
		one (explorer.py wouldn't have defined BNP_EXTENSIONS yet by the
		time search_paths.py needs it)."""
		from .shape_geometry import load_panda_texture

		panda_texture = load_panda_texture(key)
		pnm_image = None
		if panda_texture is not None:
			pnm_image = PNMImage()
			panda_texture.store(pnm_image)
			if pnm_image.get_x_size() > _THUMBNAIL_DECODE_SIZE or pnm_image.get_y_size() > _THUMBNAIL_DECODE_SIZE:
				downscaled = PNMImage(_THUMBNAIL_DECODE_SIZE, _THUMBNAIL_DECODE_SIZE, pnm_image.get_num_channels())
				downscaled.quick_filter_from(pnm_image)
				pnm_image = downscaled

		with self._thumbnail_lock:
			self._thumbnail_pending.discard(key)
			self._thumbnail_ready[key] = pnm_image

	def _draw_leaf(self, item: ExplorerItem, show_thumbnail: bool = False):
		key = self._item_key(item)
		flags = imgui.TreeNodeFlags_.leaf.value | imgui.TreeNodeFlags_.no_tree_push_on_open.value
		if key in self._selection:
			flags |= imgui.TreeNodeFlags_.selected.value

		# Thumbnails aren't available for a file living inside a .bnp (no
		# standalone path on disk to decode -- item.path is the archive's own
		# path, not the entry's).
		thumbnail_ref = None
		if show_thumbnail and item.bnp_path is None and item.suffix.lower() in _TEXTURE_EXTENSIONS:
			thumbnail_ref = self._get_thumbnail_ref(item.path)

		if thumbnail_ref is not None:
			imgui.image(thumbnail_ref, (_THUMBNAIL_SIZE, _THUMBNAIL_SIZE))
			imgui.same_line()
			imgui.tree_node_ex(key, flags, item.name)
		else:
			icon = _LEAF_ICONS.get(item.suffix.lower(), _DEFAULT_LEAF_ICON)
			imgui.tree_node_ex(key, flags, f"{icon} {item.name}")

		if imgui.is_item_hovered():
			tooltip = str(item.path) if item.bnp_path is None else f"{item.path} ! {item.name}"
			imgui.set_tooltip(tooltip)

		if imgui.is_item_clicked():
			additive = self.app.mouseWatcherNode.isButtonDown(KeyboardButton.control())
			self._select(item, key, additive)
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
