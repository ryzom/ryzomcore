"""Settings UI ("Paths" section) + scan logic for Forgery's generic search
paths: user-configured, priority-ordered folders (recursive or not,
.bnp-aware, see search_paths.py) -- the *only* place Forgery resolves
`.shape`/.skel/.anim/texture files from (no separate "data root"/asset
index; a plain folder here, with recursive on, covers that same case).
Finds .skel files compatible with the loaded shape's own skinning bones
(CSkeletonModel::remapSkinBones-equivalent matching), .anim files compatible
with a given skeleton, resolves material texture references (see
shape_geometry.py's load_panda_texture()), and picks up Ryzom's
"panoply_files.txt" (see panoply.py) if one is found among them.
"""

import threading
import time
from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd

from pynel.ryzom_animation import AnimationParseError, parse_animation
from pynel.ryzom_shape import ShapeParseError, SkeletonShape, parse_shape

from ryzom_forgery import panoply, search_paths
from ryzom_forgery import settings as app_settings
from ryzom_forgery.settings import SearchPathDir

_PANOPLY_FILE_NAME = "panoply_files.txt"


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
		self._dirs = app_settings.load().search_paths
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
		self._panoply_variants = {}  # {base texture stem: {race: [user color, ...]}}, see panoply.py
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
		if result and not any(entry.path == result for entry in self._dirs):
			self._dirs.append(SearchPathDir(path=result))
			self._save()

	def _save(self):
		"""Re-loads the shared settings file fresh and overwrites only the
		`search_paths` section with our own current state -- see
		export_dialog.py's own _save() for why (other components persist
		their own section independently)."""
		fresh = app_settings.load()
		fresh.search_paths = self._dirs
		app_settings.save(fresh)

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
		dict fresh, consulting/refreshing two on-disk caches: the .bnp table
		listing itself (search_paths.load_bnp_table_cache()/
		save_bnp_table_cache() -- opening+reading every .bnp's own table is
		the dominant cost of a scan across a real data tree, far more than
		iterating directories or indexing the resulting entries, so this is
		the one that actually matters for scan speed) and the parsed .skel/
		.anim bone-name cache (search_paths.load_scan_cache()/
		save_scan_cache()) so a file whose (mtime, size) hasn't changed
		since it was last parsed is skipped entirely."""
		start_time = time.monotonic()
		bnp_table_cache = search_paths.load_bnp_table_cache()
		cache = search_paths.load_scan_cache()
		cache_dirty = False
		skeleton_entries, skeleton_bones = {}, {}
		animation_entries, animation_bones = {}, {}
		texture_entries = {}
		panoply_variants = {}
		total = 0

		for found in search_paths.iter_all_entries(self._dirs, bnp_table_cache):
			total += 1
			lower_name = found.name.lower()
			texture_entries.setdefault(lower_name, found)
			if lower_name == _PANOPLY_FILE_NAME and not panoply_variants:
				try:
					panoply_variants = panoply.parse_panoply_files(found.read_bytes().decode("latin-1"))
				except OSError:
					pass
				continue
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
		search_paths.save_bnp_table_cache(bnp_table_cache)

		self._skeleton_entries = skeleton_entries
		self._skeleton_bones = skeleton_bones
		self._animation_entries = animation_entries
		self._animation_bones = animation_bones
		self._panoply_variants = panoply_variants
		self._texture_entries = texture_entries
		elapsed = time.monotonic() - start_time
		panoply_note = f", panoply_files.txt found ({len(panoply_variants)} textures)" if panoply_variants else ""
		self._scan_status = (
			f"{total} asset(s) scanned in {elapsed:.2f}s -- {len(skeleton_entries)} skeleton(s), "
			f"{len(animation_entries)} animation(s){panoply_note}")
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
		"""Resolves `name` against this dialog's own scanned (.bnp-aware)
		texture index -- see search_paths.find_texture() for the matching
		rules (case-insensitive, extension fallback)."""
		return search_paths.find_texture(self._texture_entries, name)

	def panoply_variants_for(self, base_texture_name):
		"""{axis: [value, ...]} (panoply.AXES) of panoply variants
		panoply_files.txt lists for `base_texture_name` (e.g.
		"tr_hof_armor00_handupside_c1.tga") -- an axis with an empty list
		means this texture has no mask for it (e.g. armor never has
		hair/eyes). {} (every axis empty) if no panoply_files.txt was found
		by the last scan, or this texture has no variants in it at all."""
		stem = Path(base_texture_name).stem.lower()
		return self._panoply_variants.get(stem, {})

	def draw_settings_content(self):
		"""Embedded in object_editor.py's Settings tab, under its own
		"Paths" section -- these are app-wide search folders, not tied to
		any one shape. Order matters: the first folder that has a given
		file wins (iter_all_entries()/find_texture() both just take the
		first match, in list order) -- the up/down buttons let a folder be
		promoted/demoted in priority instead of only add/remove."""
		imgui.text("Folders searched (top = highest priority):")
		style = imgui.get_style()
		recursive_width = imgui.get_frame_height() + style.item_inner_spacing.x + imgui.calc_text_size("Recursive").x
		reorder_width = imgui.calc_text_size(fa_icons.ICON_FA_ARROW_UP).x + style.frame_padding.x * 2
		remove_width = imgui.calc_text_size(fa_icons.ICON_FA_TRASH).x + style.frame_padding.x * 2
		remove_index = None
		move_up_index = None
		move_down_index = None
		for index, entry in enumerate(self._dirs):
			imgui.push_id(str(index))
			path_width = (imgui.get_content_region_avail().x - recursive_width - remove_width
			              - 2 * reorder_width - style.item_spacing.x * 4)
			imgui.text(_truncate_path_to_width(entry.path, max(path_width, 20)))
			if imgui.is_item_hovered():
				imgui.set_tooltip(entry.path)
			imgui.same_line()
			changed, entry.recursive = imgui.checkbox("Recursive", entry.recursive)
			if changed:
				self._save()
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_ARROW_UP, "Move up (higher priority)", disabled=index == 0):
				move_up_index = index
			imgui.same_line()
			if _icon_button(
					fa_icons.ICON_FA_ARROW_DOWN, "Move down (lower priority)", disabled=index == len(self._dirs) - 1):
				move_down_index = index
			imgui.same_line()
			if _icon_button(fa_icons.ICON_FA_TRASH, "Remove this folder"):
				remove_index = index
			imgui.pop_id()
		if move_up_index is not None:
			self._dirs[move_up_index - 1], self._dirs[move_up_index] = \
				self._dirs[move_up_index], self._dirs[move_up_index - 1]
			self._save()
		if move_down_index is not None:
			self._dirs[move_down_index + 1], self._dirs[move_down_index] = \
				self._dirs[move_down_index], self._dirs[move_down_index + 1]
			self._save()
		if remove_index is not None:
			del self._dirs[remove_index]
			self._save()

		if _icon_button(fa_icons.ICON_FA_PLUS, "Add folder..."):
			self._add_dir_dialog = pfd.select_folder("Choose a search folder")
		imgui.same_line()
		if _icon_button(fa_icons.ICON_FA_SYNC, "Reload", disabled=self._scanning):
			self.reload()

		if self._scan_status:
			imgui.text_disabled(self._scan_status)
