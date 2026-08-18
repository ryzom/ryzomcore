"""Settings UI ("Paths" section) + scan logic for Forgery's generic search
paths: user-configured folders (recursive or not, .bnp-aware, see
search_paths.py) used to find .skel files compatible with the loaded shape's
own skinning bones (CSkeletonModel::remapSkinBones-equivalent matching),
.anim files compatible with a given skeleton, and to resolve textures not
found in the game's own indexed data tree (see shape_geometry.py's
load_panda_texture()).
"""

import threading
from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd

from pynel.ryzom_animation import AnimationParseError, parse_animation
from pynel.ryzom_shape import ShapeParseError, SkeletonShape, parse_shape

from ryzom_forgery import search_paths
from ryzom_forgery.asset_index import TEXTURE_FALLBACK_EXTENSIONS
from ryzom_forgery.search_paths import SearchPathDir


def _animation_bone_names(anim):
	"""The set of bone names an Animation actually has tracks for --
	pynel's Animation has no such field directly, only `id_by_name`, a dict
	of track keys shaped like "{bone_name}.pos"/".rotquat"/".scale" (see
	pynel.ryzom_animation's own docstring)."""
	names = set()
	for key in anim.id_by_name:
		bone_name, separator, _ = key.rpartition(".")
		if separator:
			names.add(bone_name)
	return names


def _icon_button(icon, tooltip, disabled=False):
	"""Same minimal icon-button-with-tooltip pattern as explorer.py's own
	_icon_button() -- each module keeps its own tiny copy rather than
	sharing one, matching how this codebase already does it (see also
	object_editor.py's own, more featureful version)."""
	imgui.begin_disabled(disabled)
	clicked = imgui.button(icon)
	imgui.end_disabled()
	if imgui.is_item_hovered():
		imgui.set_tooltip(tooltip)
	return clicked

# Long paths would otherwise push the Recursive/Remove buttons off the
# Settings panel -- ImGui doesn't wrap/reflow a same_line() row on its own,
# it just keeps extending past the window's edge, so the path text has to be
# shrunk to fit whatever pixel width is actually left over, not a fixed
# character count (a 40-char budget still overflows a narrow panel, and
# wastes space in a wide one). Truncated from the front (the tail of a path
# is usually the meaningful part, e.g. ".../data/textures"), full path
# always available as a tooltip on hover.
_ELLIPSIS = "..."


def _truncate_path_to_width(path, max_width):
	if imgui.calc_text_size(path).x <= max_width:
		return path
	if imgui.calc_text_size(_ELLIPSIS).x > max_width:
		return _ELLIPSIS

	# Binary search for the longest tail of `path` that still fits alongside
	# the ellipsis -- calc_text_size() isn't linear in character count
	# (proportional font), so this can't be computed directly.
	lo, hi, best = 0, len(path), _ELLIPSIS
	while lo <= hi:
		mid = (lo + hi) // 2
		candidate = _ELLIPSIS + path[-mid:] if mid > 0 else _ELLIPSIS
		if imgui.calc_text_size(candidate).x <= max_width:
			best = candidate
			lo = mid + 1
		else:
			hi = mid - 1
	return best


class SearchPathsDialog:
	def __init__(self):
		self._config = search_paths.load()
		self._add_dir_dialog = None

		# Built by reload() (run on a background thread -- parsing every
		# .skel/.anim under a real data tree is far too slow to do on the
		# main/render thread, see _reload_worker()) -- each swapped in as a
		# whole fresh dict at the very end of a scan, never mutated in
		# place, so a frame reading them mid-scan just sees the previous
		# complete result, never a torn one.
		self._skeleton_entries = {}  # {name: search_paths.FoundEntry}
		self._skeleton_bones = {}  # {name: frozenset(bone names)} -- for compatible_for(), no full parse needed
		self._animation_entries = {}  # {name: search_paths.FoundEntry}
		self._animation_bones = {}  # {name: frozenset(bone names)} -- for compatible_animations_for()
		self._texture_entries = {}  # {name.lower(): search_paths.FoundEntry}
		self._scan_status = ""
		self._scanning = False
		self._scanned_once = False

	@property
	def scanning(self):
		return self._scanning

	def draw(self):
		"""Call once per ImGui frame, alongside the other always-polled
		dialogs (see object_editor.py's draw_panel())."""
		self._poll_add_dir_dialog()

	def _poll_add_dir_dialog(self):
		if self._add_dir_dialog is None or not self._add_dir_dialog.ready(0):
			return
		result = self._add_dir_dialog.result()
		self._add_dir_dialog = None
		if result and not any(entry.path == result for entry in self._config.dirs):
			self._config.dirs.append(SearchPathDir(path=result))
			search_paths.save(self._config)

	def ensure_scanned(self):
		"""Kicks off a first background scan the first time it's needed
		(see object_editor.py's _display_shape()) -- so a freshly-opened
		Skinning preview already has a usable Skeleton/Animation list
		without the user having to press Reload themselves first."""
		if not self._scanned_once:
			self._scanned_once = True
			self.reload()

	def reload(self):
		"""(Re)scans every configured folder in the background -- see
		_reload_worker(). A no-op while a scan is already running rather
		than piling up threads; the icon button is also disabled meanwhile
		(see draw_settings_content()/object_editor.py's Skinning preview)."""
		if self._scanning:
			return
		self._scanning = True
		self._scan_status = "Scanning..."
		threading.Thread(target=self._reload_worker, daemon=True).start()

	def _reload_worker(self):
		"""Runs off the main thread (see reload()). Builds every result
		dict fresh, consulting/refreshing the on-disk scan cache
		(search_paths.load_scan_cache()/save_scan_cache()) so a file whose
		(mtime, size) hasn't changed since it was last parsed is skipped
		entirely -- only genuinely new/changed .skel/.anim files actually
		get parsed. Texture entries need no parsing at all (just the file
		reference), so they're always cheap regardless of caching."""
		cache = search_paths.load_scan_cache()
		cache_dirty = False
		skeleton_entries, skeleton_bones = {}, {}
		animation_entries, animation_bones = {}, {}
		texture_entries = {}
		total = 0

		for found in search_paths.iter_all_entries(self._config):
			total += 1
			lower_name = found.name.lower()
			texture_entries.setdefault(lower_name, found)
			is_skel = lower_name.endswith(".skel")
			is_anim = lower_name.endswith(".anim")
			if not (is_skel or is_anim):
				continue

			try:
				mtime, size = found.cache_stat()
			except OSError:
				continue
			key = found.cache_key()
			cached = cache.get(key)
			if cached is not None and cached.get("mtime") == mtime and cached.get("size") == size:
				bones = frozenset(cached.get("bones", ()))
			else:
				bones = self._parse_bones(found, is_skel)
				if bones is None:
					continue
				cache[key] = {
					"mtime": mtime, "size": size, "type": "skel" if is_skel else "anim", "bones": sorted(bones),
				}
				cache_dirty = True

			if is_skel:
				skeleton_entries[found.name] = found
				skeleton_bones[found.name] = bones
			else:
				animation_entries[found.name] = found
				animation_bones[found.name] = bones

		if cache_dirty:
			search_paths.save_scan_cache(cache)

		self._skeleton_entries = skeleton_entries
		self._skeleton_bones = skeleton_bones
		self._animation_entries = animation_entries
		self._animation_bones = animation_bones
		self._texture_entries = texture_entries
		self._scan_status = (
			f"Found {len(skeleton_entries)} skeleton(s), {len(animation_entries)} animation(s) "
			f"in {total} file(s) scanned")
		self._scanning = False

	@staticmethod
	def _parse_bones(found, is_skel):
		"""Full parse, only for a cache miss -- the bone names a .skel
		defines (SkeletonShape.bone_map) or a .anim has tracks for
		(_animation_bone_names()). None on any parse failure (the file is
		just skipped, same as before caching existed)."""
		try:
			data = found.read_bytes()
		except OSError:
			return None
		if is_skel:
			try:
				shape_file = parse_shape(data)
			except ShapeParseError:
				return None
			if not isinstance(shape_file.value, SkeletonShape):
				return None
			return frozenset(shape_file.value.bone_map)
		try:
			anim = parse_animation(data)
		except AnimationParseError:
			return None
		return frozenset(_animation_bone_names(anim))

	def scanned_skeleton_names(self):
		return sorted(self._skeleton_bones)

	def compatible_for(self, bones_name):
		"""Names (sorted) of every scanned .skel whose bones are a
		superset of `bones_name` -- same compatibility test the engine
		itself applies at bind time (CSkeletonModel::remapSkinBones)."""
		wanted = set(bones_name)
		return sorted(name for name, bones in self._skeleton_bones.items() if wanted <= bones)

	def skeleton_for(self, name):
		"""Full parse of this one specific .skel, on demand -- only called
		once, when the user actually picks `name` (see
		object_editor.py's _apply_bone_preview_skeleton()), so there's no
		need to keep every scanned skeleton's full parsed data (inverse
		bind matrices etc.) in memory just for the combo/compatibility
		list, which only ever needed the bone names (see _skeleton_bones)."""
		entry = self._skeleton_entries.get(name)
		if entry is None:
			return None
		try:
			shape_file = parse_shape(entry.read_bytes())
		except (ShapeParseError, OSError):
			return None
		return shape_file.value if isinstance(shape_file.value, SkeletonShape) else None

	def scanned_animation_names(self):
		return sorted(self._animation_bones)

	def compatible_animations_for(self, skeleton):
		"""Names (sorted) of every scanned .anim whose own tracked bone
		names are all present in `skeleton` -- same idea as compatible_for()
		but the other way around (the animation's bones must be a subset of
		the skeleton's, not the other way -- an animation legitimately
		doesn't need to touch every bone)."""
		if skeleton is None:
			return []
		skeleton_bones = skeleton.bone_map.keys()
		return sorted(name for name, bones in self._animation_bones.items() if bones <= skeleton_bones)

	def animation_for(self, name):
		"""Full parse of this one specific .anim, on demand -- see
		skeleton_for()'s own note on why only the bone names are kept for
		every scanned entry, not the full parsed Animation."""
		entry = self._animation_entries.get(name)
		if entry is None:
			return None
		try:
			return parse_animation(entry.read_bytes())
		except (AnimationParseError, OSError):
			return None

	def find_texture(self, name):
		"""Same matching rules as AssetIndex.find_texture()/shape_geometry's
		own _find_local_texture_ref(): case-insensitive exact name first,
		then the same base name with each of TEXTURE_FALLBACK_EXTENSIONS --
		just against this dialog's own scanned (.bnp-aware) index instead of
		the game's own AssetIndex."""
		candidates = [name.lower()]
		stem = Path(name).stem.lower()
		candidates += [stem + extension for extension in TEXTURE_FALLBACK_EXTENSIONS]
		for candidate in candidates:
			match = self._texture_entries.get(candidate)
			if match is not None:
				return match
		return None

	def draw_settings_content(self):
		"""Embedded in object_editor.py's Settings tab, under its own
		"Paths" section -- these are app-wide search folders, not tied to
		any one shape."""
		imgui.text("Folders searched:")
		style = imgui.get_style()
		recursive_width = imgui.get_frame_height() + style.item_inner_spacing.x + imgui.calc_text_size("Recursive").x
		remove_width = imgui.calc_text_size(fa_icons.ICON_FA_TRASH).x + style.frame_padding.x * 2
		remove_index = None
		for index, entry in enumerate(self._config.dirs):
			imgui.push_id(str(index))
			path_width = imgui.get_content_region_avail().x - recursive_width - remove_width - style.item_spacing.x * 2
			imgui.text(_truncate_path_to_width(entry.path, max(path_width, 20)))
			if imgui.is_item_hovered():
				imgui.set_tooltip(entry.path)
			imgui.same_line()
			changed, entry.recursive = imgui.checkbox("Recursive", entry.recursive)
			if changed:
				search_paths.save(self._config)
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_TRASH, "Remove this folder"):
				remove_index = index
			imgui.pop_id()
		if remove_index is not None:
			del self._config.dirs[remove_index]
			search_paths.save(self._config)

		if _icon_button(fa_icons.ICON_FA_FOLDER_PLUS, "Add folder..."):
			self._add_dir_dialog = pfd.select_folder("Choose a search folder")
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_SYNC, "Reload", disabled=self._scanning):
			self.reload()

		if self._scan_status:
			imgui.text_disabled(self._scan_status)
