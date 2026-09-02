"""ObjectEditorApp mixin: the Bind preview -- the Skinning preview's own
bone/animation loading for the main shape, plus the Bind preview proper
(creature cache, creature/slot/attach-point selection, full-body assembly
and its per-frame live re-skin/animation, and the floating control panels
for both). Split out of object_editor.py, see the "Split object_editor.py
into theme files" chantier in project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

import io
import shutil
import subprocess
import threading
import time
from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, imgui_ctx, portable_file_dialogs as pfd
from panda3d.core import ClockObject, GeomNode, Mat4, Quat

from pynel.ryzom_animation import (
	AnimationParseError, animation_duration, evaluate_all_bone_world_matrices, parse_animation,
)
from pynel.ryzom_shape import MeshMRMSkinned, ShapeParseError, SkeletonShape, parse_shape

from ryzom_forgery import creature_ref
from ryzom_forgery import panoply
from ryzom_forgery import settings as app_settings
from ryzom_forgery.shape_geometry import iter_render_passes, shape_bbox, shape_geom
from ryzom_forgery.apps.object_editor_mixins.geometry_helpers import (
	_AXIS_LENGTH, _AXIS_MARGIN_FACTOR, _build_axes_geom, _build_geom, _build_vertex_data, _is_shape_skinned,
)
from ryzom_forgery.apps.object_editor_mixins.skin_state_helpers import (
	_build_mrm_skin_state, _build_skin_state, _MrmSkinState, _reskin_mrm_state, _reskin_state,
)
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import _icon_button, _VIEWPORT_TOGGLE_MARGIN_PX

_COMPATIBLE_COLOR = (0.35, 0.85, 0.35, 1.0)  # green -- "this .skel matches the loaded shape's bones"

# A visually distinct palette (bright orange/lime/violet) for the Bind
# preview's attach-point marker (_apply_loaded_shape_to_creature()) -- the
# actual target the loaded shape's own pivot needs to be moved onto for a
# correct in-game grip point, see that marker's own docstring (2026-08-31,
# Nuno: "je dois OBLIGATOIREMENT visualiser ce point de préhension [...] je
# veux savoir précisement ce que je dois changer... le pivot de mon arme
# n'est pas sur le point d'attache"). Built via geometry_helpers._build_axes_geom(),
# same as the world/pivot/reference axes (see that module for their own
# colors/constants).
_ATTACH_POINT_AXIS_COLORS = {"x": (1.0, 0.55, 0.05, 1.0), "y": (0.55, 0.95, 0.15, 1.0), "z": (0.65, 0.3, 0.95, 1.0)}


def _nel_matrix_to_panda_mat4(m):
	"""Converts one of pynel.ryzom_animation.evaluate_all_bone_world_matrices()'s
	raw 4x4 matrices (a tuple of rows, m[row][col], NeL's row-major/column-3-
	translation convention -- v' = M * v as a column vector, same convention
	_mat_translate()/_mat_mul() in ryzom_animation.py use) into a Panda3D
	Mat4, whose own convention is the transpose of that (row-vector on the
	left, v' = v * M, translation in the LAST ROW -- see
	shape_geometry.py-adjacent uv_matrix_to_panda_mat4()'s own Mat4(...)
	calls for another example of this same row/last-row-translation shape).
	NOT independently verified against a real bind visually yet (no GUI in
	this dev environment) -- if a bound shape ends up offset/misoriented,
	this transpose direction is the first thing to double check."""
	return Mat4(
		m[0][0], m[1][0], m[2][0], 0.0,
		m[0][1], m[1][1], m[2][1], 0.0,
		m[0][2], m[1][2], m[2][2], 0.0,
		m[0][3], m[1][3], m[2][3], 1.0,
	)


class CreatureBindMixin:
	def _on_load_skeleton_command(self, items):
		if not items:
			return
		item = items[0]
		self._load_skeleton_bytes(item.read_bytes(), item.name)

	def _load_skeleton_bytes(self, data, name):
		"""Parses `data` as a .skel, applying it as the Skinning preview's
		skeleton if valid -- shared by the Explorer's right-click "Load as
		bone-preview skeleton" command and the Skinning preview's own "load
		from disk" icon button (_poll_skeleton_file_dialog())."""
		try:
			shape_file = parse_shape(data)
		except ShapeParseError as exc:
			print(f"[object_editor] cannot parse skeleton {name}: {exc}")
			return
		if not isinstance(shape_file.value, SkeletonShape):
			print(f"[object_editor] {name} is not a CSkeletonShape")
			return
		self._apply_bone_preview_skeleton(shape_file.value, name)

	def _apply_bone_preview_skeleton(self, skeleton, name):
		"""Shared by the Explorer's right-click "Load as bone-preview
		skeleton" command and the Skinning preview's own Skeleton combo
		(see _draw_bone_preview_controls()) -- this is the *main* loaded
		shape's own skeleton, read every frame by _update_skin_preview()'s
		live re-skin task. NOT used by the Bind preview's creature picker
		(_select_bind_creature() has its own separate _bind_skeleton field
		instead) -- the two used to share this one, which made picking a
		Bind-preview creature silently redirect the main shape's per-frame
		re-skin to the creature's skeleton too (2026-08-30)."""
		self._bone_preview_skeleton = skeleton
		self._bone_preview_skeleton_name = name
		# The main shape might itself be CMeshMRMSkinned and was rendering its
		# rigid bind-pose fallback for lack of a skeleton (see
		# _rebuild_geometry()) -- now that one just got picked, rebuild so it
		# renders skinned instead.
		if self.shape_file is not None:
			self._rebuild_geometry()

	def _on_load_animation_command(self, items):
		if not items:
			return
		item = items[0]
		self._apply_bone_preview_animation_bytes(item.read_bytes(), item.name)

	def _apply_bone_preview_animation_bytes(self, data, name):
		"""Parses `data` as a .anim, applying it as the Skinning preview's
		animation if valid -- shared by the Explorer's right-click "Load as
		bone-preview animation" command, the Animation combo, and the "load
		from disk" icon button (_poll_animation_file_dialog())."""
		try:
			anim = parse_animation(data)
		except AnimationParseError as exc:
			print(f"[object_editor] cannot parse animation {name}: {exc}")
			return
		self._apply_bone_preview_animation(anim, name)

	def _apply_bone_preview_animation(self, anim, name):
		self._bone_preview_animation = anim
		self._bone_preview_animation_name = name
		self._bone_preview_animation_duration = animation_duration(anim)
		self._bone_preview_time = 0.0

	def _unload_bone_preview(self):
		self._bone_preview_skeleton = None
		self._bone_preview_skeleton_name = ""
		self._bone_preview_animation = None
		self._bone_preview_animation_name = ""
		self._bone_preview_animation_duration = 0.0
		self._bone_preview_time = 0.0
		if self.shape_file is not None:
			self._rebuild_geometry()

	def _bone_world_matrices_for(self, names):
		"""{bone name: 4x4 world matrix} for every name in `names` that's
		actually in the loaded skeleton, at the current preview time/
		animation -- {} if no skeleton is loaded. Computes the *whole*
		skeleton's world matrices in one pass (pynel's
		evaluate_all_bone_world_matrices(), O(bone count) instead of
		evaluate_bone_world_matrix() once per name, which independently
		re-walks each bone's own ancestor chain -- called every frame for a
		loaded skinned shape's re-skin (_update_skin_preview()), where that
		redundant work was the dominant per-frame cost past a handful of
		bones)."""
		skeleton = self._bone_preview_skeleton
		if skeleton is None:
			return {}
		all_matrices = evaluate_all_bone_world_matrices(skeleton, self._bone_preview_animation, self._bone_preview_time)
		return {name: all_matrices[name] for name in names if name in skeleton.bone_map}

	def _update_skin_preview(self, task):
		"""Per-frame: re-skins the loaded shape's geometry in place (see
		_build_skin_state()) whenever it's a CMeshMRMSkinned with a skeleton
		loaded -- vectorized (numpy), unlike pynel.ryzom_skin's plain-Python
		skin_vertex()/skin_mesh() (fine for one-off/CLI use, far too slow
		called per-vertex every frame for a real character mesh -- see
		_update_wind()'s own note on exactly this same tradeoff). No-op
		otherwise, same pattern as _update_wind()/_update_skin_preview_time().
		The actual blend math lives in the module-level _reskin_state(), shared
		with _update_assembled_creature_skin()'s own per-body-part re-skin."""
		state = self._skin_state
		if state is None or state.vdata is None:
			return task.cont
		bone_world_matrices = self._bone_world_matrices_for(state.bone_names)
		_reskin_state(state, bone_world_matrices)
		return task.cont

	def _update_bound_shape_rotation(self, task):
		"""Per-frame: keeps the Bind preview's loaded-shape-on-creature
		copy's own local offset (self._assembled_creature_loaded_shape_content_root,
		see _build_assembled_shape_geometry()) matching the main shape's
		CURRENT position/rotation/scale edits -- a cheap matrix copy, not a
		geometry rebuild, so a live Ctrl+drag or Properties-panel edit on
		the main shape shows up on its bound copy immediately too (bug
		found 2026-08-30, Nuno: rotation alone wasn't enough, "pos et scale
		marchent pas" either).

		model_root.get_mat(self.render) is exactly this offset: _object_pivot
		is parented directly to self.render at true (0,0,0)/scale-1 (only
		ever rotated, seeded from base.default_rot_quat on load, see
		_display_shape()), and model_root itself carries any pivot-locked
		edits on top (_transform_node()) -- so their COMPOSED matrix
		relative to render *is* "whatever the shape's own intrinsic
		placement + every edit since load add up to", with no absolute
		world position baked in to fight with wherever the bound copy is
		already correctly anchored (skinned onto the creature, or set to an
		attach-point bone -- see _apply_loaded_shape_to_creature()).
		content_root.set_mat(matrix) (single-argument form, no coordinate-
		space conversion) applies that composed matrix as content_root's
		own LOCAL transform within its already-correctly-anchored parent,
		exactly like _build_assembled_shape_geometry()'s own initial
		set_quat(base.default_rot_quat) did for rotation alone -- this
		fully supersedes that one-time snapshot every frame from here on.

		EXCEPTION: the skinned slot-override case
		(self._assembled_creature_loaded_shape_is_skinned_override) is left
		alone entirely -- position, rotation, AND scale. Verified 2026-08-31
		(transform.cpp:946 CTransform::updateWorldMatrixFromFather()): a
		skinned instance's own Default{Pos,RotQuat,Scale} has ZERO effect in
		the real game, not just position/scale -- the skin binding to the
		creature's own skeleton alone fully determines every vertex's final
		position (see _build_assembled_shape_geometry()'s own matching
		is_skinned check). Layering ANY of the main shape's own edits on top
		here would just double-offset/rotate already-correctly-placed
		vertices away from a result the real client could ever produce (bug
		found 2026-08-31, Nuno: "t'as fais du caca sur les shape skinnés..
		ils sont pas du tout au bon endroit", then "t'es sur que la rotation
		dois s'appliquer aussi?" -- no, confirmed).

		No-op whenever nothing is currently bound."""
		content_root = self._assembled_creature_loaded_shape_content_root
		if (content_root is not None and not content_root.is_empty()
				and not self._assembled_creature_loaded_shape_is_skinned_override):
			content_root.set_mat(self.model_root.get_mat(self.render))
		return task.cont

	def _update_bind_anim_time(self, task):
		"""Per-frame: advances self._bind_anim_time while playing -- the clock
		driving the Bind preview's live playback (Play button,
		_draw_bind_controls()). _update_assembled_creature_skin() re-skins
		every body-part shape against this time every frame. Same loop-by-
		duration pattern as _update_skin_preview_time()'s own clock."""
		if self._bind_anim_playing and self._bind_anim_duration > 0:
			dt = ClockObject.get_global_clock().get_dt()
			self._bind_anim_time = (self._bind_anim_time + dt) % self._bind_anim_duration
		return task.cont

	def _update_assembled_creature_skin(self, task):
		"""Per-frame: re-skins every live-capable body-part shape of the
		assembled Bind-preview creature (_assembled_creature_skin_states,
		built by _build_assembled_shape_geometry() for each skinned slot --
		either a _SkinState, for CMeshMRMSkinned's packed-vertex format, or a
		_MrmSkinState, for a plain skinned CMeshMRM's own geom.skin_weights
		layout, e.g. *_visage.shape face pieces), plus the loaded shape's own
		skinned-override placement if any
		(_assembled_creature_loaded_shape_skin_state), plus the loaded
		shape's own rigid attach-point placement if any (weapons don't have a
		_SkinState at all -- skeleton=None is passed for that case in
		_build_assembled_shape_geometry() -- but still need their WORLD
		POSITION refreshed every frame, otherwise a weapon stays frozen at
		whatever bone matrix build time happened to compute, bug found
		2026-08-31, Nuno: "l'arme qui ne bouge pas"). No-op whenever no
		animation is currently resolved (self._bind_animation is None -- no
		Mode picked, or resolution/parsing failed, see
		_rebuild_assembled_creature()) or no creature skeleton is loaded.

		Bone world matrices are computed ONCE per frame for the whole
		skeleton and reused by every re-skin call below -- same
		"whole-skeleton batch, not per-bone" reasoning
		_bone_world_matrices_for() already documents, now paying off across
		several shapes at once instead of just the main shape's own."""
		if self._bind_animation is None or self._bind_skeleton is None:
			return task.cont
		bone_world_matrices = evaluate_all_bone_world_matrices(
			self._bind_skeleton, self._bind_animation, self._bind_anim_time)
		self._assembled_creature_bone_matrices = bone_world_matrices
		for state in self._assembled_creature_skin_states.values():
			if isinstance(state, _MrmSkinState):
				_reskin_mrm_state(state, bone_world_matrices)
			else:
				_reskin_state(state, bone_world_matrices)
		loaded_state = self._assembled_creature_loaded_shape_skin_state
		if loaded_state is not None:
			if isinstance(loaded_state, _MrmSkinState):
				_reskin_mrm_state(loaded_state, bone_world_matrices)
			else:
				_reskin_state(loaded_state, bone_world_matrices)
		if (not self._bind_slot_override and self._bind_attach_point
				and self._bind_attach_point in self._bind_skeleton.bone_map
				and self._assembled_creature_loaded_shape_node is not None):
			bone_matrix = bone_world_matrices[self._bind_attach_point]
			panda_mat = _nel_matrix_to_panda_mat4(bone_matrix)
			self._assembled_creature_loaded_shape_node.set_mat(panda_mat)
			if not self._attach_point_axes_np.is_empty():
				self._attach_point_axes_np.set_mat(panda_mat)
		return task.cont

	def _update_skin_preview_time(self, task):
		"""Per-frame: advances self._bone_preview_time while playing -- the
		clock driving the Skinning preview's animation (_update_skin_preview()
		re-skins the loaded shape's geometry against it every frame)."""
		if self._bone_preview_playing and self._bone_preview_animation_duration > 0:
			dt = ClockObject.get_global_clock().get_dt()
			self._bone_preview_time = (self._bone_preview_time + dt) % self._bone_preview_animation_duration
		return task.cont

	def _draw_bone_preview_controls(self):
		"""Floating "Skinning preview" window: picks the skeleton/animation
		driving the loaded shape's own live re-skin (self._bone_preview_skeleton,
		read every frame by _update_skin_preview()) -- only shown for a
		CMeshMRMSkinned, the one shape type that actually needs live
		re-skinning; a rigid shape has nothing for this panel to drive.
		Deliberately separate from _draw_bind_controls()'s own
		self._bind_skeleton (Bind preview's creature picker) -- they used to
		share one field, which made picking a Bind-preview creature silently
		redirect the main shape's per-frame re-skin too (2026-08-30, see
		_bind_skeleton's own docstring). Positioned the same way as
		_draw_wind_controls(), stacked right below it (both top-right of the
		viewport, flush against the panel)."""
		if self.shape_file is None or not _is_shape_skinned(self.shape_file.value):
			return

		shape_bones = set(shape_geom(self.shape_file.value).bones_name)

		display_width = imgui.get_io().display_size.x
		win_w, win_h = self._bone_preview_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		if self._wind_state is not None:
			y += self._wind_panel_size[1] + _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("Skinning preview", flags=flags):
			# Every .skel found by the last scan (see SearchPathsDialog.reload(),
			# triggered by the reload icon below), sorted first/highlighted
			# green when it's actually compatible with the loaded shape's own
			# skinning bones -- picking an incompatible one is still allowed,
			# same as the engine itself doesn't refuse an incompatible bind,
			# just remaps missing bones to the root (CSkeletonModel::remapSkinBones).
			scanned_names = self.search_paths_dialog.scanned_skeleton_names()
			compatible_names = set(self.search_paths_dialog.compatible_for(shape_bones))
			# Compatible ones first (still sorted among themselves), then the
			# rest -- the match everyone actually wants shouldn't be buried
			# alphabetically among dozens of irrelevant skeletons.
			ordered_names = sorted(compatible_names) + sorted(name for name in scanned_names if name not in compatible_names)
			imgui.set_next_item_width(220)
			preview = self._bone_preview_skeleton_name or "(choose a skeleton)"
			if imgui.begin_combo("##skeleton-combo", preview):
				for name in ordered_names:
					compatible = name in compatible_names
					if compatible:
						imgui.push_style_color(imgui.Col_.text.value, _COMPATIBLE_COLOR)
					clicked, _ = imgui.selectable(name, name == self._bone_preview_skeleton_name)
					if compatible:
						imgui.pop_style_color()
					if clicked:
						self._apply_bone_preview_skeleton(self.search_paths_dialog.skeleton_for(name), name)
				imgui.end_combo()
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##skeleton-file", "Load a skeleton from disk..."):
				self._skeleton_file_dialog = pfd.open_file("Choose a .skel file", "", ["Ryzom skeleton", "*.skel"])
			imgui.same_line()
			if _icon_button(
					fa_icons.ICON_FA_SYNC, "Rescan the configured search paths",
					disabled=self.search_paths_dialog.scanning):
				self.search_paths_dialog.reload()
			if self.search_paths_dialog.scanning:
				imgui.text_disabled("Scanning search paths...")

			# Same idea as the Skeleton combo above, but the other way around
			# (see SearchPathsDialog.compatible_animations_for()): an
			# animation is compatible when ITS OWN tracked bones are all
			# present in the *currently chosen* skeleton -- so this list is
			# empty until a skeleton is picked.
			animation_names = self.search_paths_dialog.scanned_animation_names()
			compatible_animations = set(self.search_paths_dialog.compatible_animations_for(self._bone_preview_skeleton))
			ordered_animations = (
				sorted(compatible_animations)
				+ sorted(name for name in animation_names if name not in compatible_animations))
			imgui.set_next_item_width(220)
			preview = self._bone_preview_animation_name or "(none, bind pose)"
			if imgui.begin_combo("##animation-combo", preview):
				for name in ordered_animations:
					compatible = name in compatible_animations
					if compatible:
						imgui.push_style_color(imgui.Col_.text.value, _COMPATIBLE_COLOR)
					clicked, _ = imgui.selectable(name, name == self._bone_preview_animation_name)
					if compatible:
						imgui.pop_style_color()
					if clicked:
						self._apply_bone_preview_animation(self.search_paths_dialog.animation_for(name), name)
				imgui.end_combo()
			imgui.same_line()
			if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##animation-file", "Load an animation from disk..."):
				self._animation_file_dialog = pfd.open_file("Choose a .anim file", "", ["Ryzom animation", "*.anim"])

			if self._bone_preview_animation is not None:
				icon = fa_icons.ICON_FA_PAUSE if self._bone_preview_playing else fa_icons.ICON_FA_PLAY
				if _icon_button(icon, "Play/pause"):
					self._bone_preview_playing = not self._bone_preview_playing
				imgui.same_line()
				imgui.set_next_item_width(160)
				changed, new_time = imgui.slider_float(
					"##bone-preview-time", self._bone_preview_time,
					0.0, max(self._bone_preview_animation_duration, 0.001), "%.2f s")
				if changed:
					self._bone_preview_time = new_time

			if _icon_button(fa_icons.ICON_FA_TIMES, "Unload skeleton/animation, stop preview"):
				self._unload_bone_preview()
			self._bone_preview_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)

	def _ensure_bind_creatures(self):
		"""Lazily loads the curated creature list + its distilled cache (see
		creature_ref.py) for the Bind preview's creature combo -- cheap once
		loaded: the bundled cache is a plain pre-generated JSON read, no
		.packed_sheets parsing involved for the 8 default reference
		creatures. Invalidated (see _on_active_workspace_changed()) whenever
		the active workspace changes, since a different workspace may have
		its own creatures_ref.txt override resolving to a different cache.

		A WORKSPACE override list's own cache never ships pre-generated
		(unlike the bundled list's) -- kicks off a background rebuild (see
		_start_bind_cache_rebuild()) when it's missing or older than the
		list file. 2026-08-30 gap found by Nuno: this was never wired up at
		all -- editing/copying creatures_ref.txt into a workspace produced a
		permanently empty combo (no cache ever existed for it to read), not
		a partial/stale one."""
		if self._bind_cache_rebuild is not None and self._bind_cache_rebuild["done"]:
			if self._bind_cache_rebuild["error"]:
				self._save_status = f"Creature cache rebuild failed: {self._bind_cache_rebuild['error']}"
				print(f"[object_editor] {self._save_status}")
			self._bind_cache_rebuild = None
			self._bind_creatures = {}  # force the reload below, picking up whatever the rebuild just wrote

		if self._bind_creatures:
			return self._bind_creatures
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		list_path = creature_ref.resolve_list_path(workspace_dir)
		cache_path = creature_ref.resolve_cache_path(workspace_dir)
		entries = creature_ref.load_creature_entries(list_path)
		cache = creature_ref.load_cache(cache_path)
		self._bind_creatures = {label: cache[label] for _sheet, label in entries if label in cache}

		is_workspace_list = workspace_dir is not None and list_path == creature_ref.workspace_list_path(workspace_dir)
		if is_workspace_list and self._bind_cache_rebuild is None:
			cache_mtime = cache_path.stat().st_mtime if cache_path.is_file() else None
			list_mtime = list_path.stat().st_mtime
			if cache_mtime is None or list_mtime > cache_mtime:
				self._start_bind_cache_rebuild(workspace_dir, entries)
		return self._bind_creatures

	def _resolve_data_bytes(self, name):
		"""Resolves `name` (a plain file name, e.g. "creature.packed_sheets"
		or "sheet_id.bin") against the same scanned search-path index
		find_texture() already uses for .shape/.skel/texture names -- despite
		the name, it indexes every file found (any extension, .bnp contents
		included) across the user's configured search folders, so it works
		unchanged for these too. None if not found by the last scan."""
		entry = self.search_paths_dialog.find_texture(name)
		return entry.read_bytes() if entry is not None else None

	def _start_bind_cache_rebuild(self, workspace_dir, entries):
		"""Starts a background thread rebuilding the workspace's own
		creatures_cache.json from real .packed_sheets data (see
		creature_ref.build_cache(), ~1-6s depending on how much of
		creature.packed_sheets's 28545 entries needs parsing -- far too slow
		for the main/UI thread). Call from the main thread only, same
		threading split as _start_panoply_bake()/_run_panoply_bake(): only
		_run_bind_cache_rebuild() (pure file I/O/pynel parsing) runs off it."""
		progress = {"done": False, "error": None}
		self._bind_cache_rebuild = progress
		thread = threading.Thread(target=self._run_bind_cache_rebuild, args=(workspace_dir, list(entries), progress), daemon=True)
		thread.start()

	def _run_bind_cache_rebuild(self, workspace_dir, entries, progress):
		"""Background-thread body for _start_bind_cache_rebuild() -- never
		touches imgui, only the `progress` dict passed in (plain dict field
		writes, same safe-under-the-GIL reasoning as _run_panoply_bake()) and
		creature_ref.build_cache()/save_cache()'s own file I/O. Writes to the
		`progress` local, NOT self._bind_cache_rebuild -- switching the
		active workspace mid-rebuild resets that attribute to a fresh dict
		(or None, see _on_active_workspace_changed()), so writing through it
		here could crash this thread (None isn't subscriptable) or mark an
		unrelated newer rebuild as done before its own thread finished
		(found 2026-08-30 via code review, matching this exact reachable
		case -- unlike panoply bake's own self._bake_progress, which has no
		equivalent reset-while-running path)."""
		try:
			from pynel import ryzom_packed_sheets as ps
			needed = ("creature.packed_sheets", "item.packed_sheets", "sitem.packed_sheets", "sheet_id.bin")
			raw = {name: self._resolve_data_bytes(name) for name in needed}
			missing = [name for name, data in raw.items() if data is None]
			if missing:
				raise FileNotFoundError(f"not found in scanned search paths: {', '.join(missing)}")
			sheet_id_names = ps.parse_sheet_id_bin(raw["sheet_id.bin"])
			records = creature_ref.build_cache(
				entries, io.BytesIO(raw["creature.packed_sheets"]), io.BytesIO(raw["item.packed_sheets"]),
				io.BytesIO(raw["sitem.packed_sheets"]), sheet_id_names)
			creature_ref.save_cache(creature_ref.workspace_cache_path(workspace_dir), records)
		except Exception as exc:
			progress["error"] = str(exc)
			print(f"[object_editor] creature cache rebuild failed: {exc}")
		finally:
			progress["done"] = True

	def _resolve_scanned_skeleton_name(self, wanted):
		"""Case-insensitive match of `wanted` against the scanned .skel
		index. creature.packed_sheets stores a skeleton filename in
		whatever case the source .creature sheet happened to use (e.g.
		"FY_HOF_skel.skel"), which doesn't necessarily match the real
		file's on-disk name (confirmed on a real install, 2026-08-30:
		"fy_hof_skel.skel", all lowercase) -- SearchPathsDialog itself
		indexes/looks up skeletons by exact name (see skeleton_for()), so
		this is needed before calling it. Returns the actually-scanned
		name, or None if nothing matches even case-insensitively."""
		wanted_lower = wanted.lower()
		for name in self.search_paths_dialog.scanned_skeleton_names():
			if name.lower() == wanted_lower:
				return name
		return None

	@staticmethod
	def _resolve_bone_name(skeleton, wanted):
		"""Case-insensitive match of `wanted` against `skeleton.bone_map` --
		BodyToBone/weapon attach-point names (creature_ref.bind_attach_points())
		aren't guaranteed to match the skeleton's own bone-name casing
		exactly, same class of mismatch as _resolve_scanned_skeleton_name().
		None if `skeleton` is None or nothing matches even case-insensitively."""
		if skeleton is None:
			return None
		wanted_lower = wanted.lower()
		for name in skeleton.bone_map:
			if name.lower() == wanted_lower:
				return name
		return None

	def _select_bind_creature(self, name):
		"""Picks `name` in the Bind preview's creature combo: loads its
		skeleton (via the same scanned-search-path index the Skinning
		preview uses, see SearchPathsDialog.skeleton_for(), through
		_resolve_scanned_skeleton_name()'s case-insensitive match), and
		shows the assembled creature right away (2026-08-30, Nuno: no
		reason to make picking a creature and toggling it visible two
		separate steps).
		Deliberately does NOT reset _bind_slot_override or
		_bind_attach_point -- comparing how the same loaded shape/weapon
		looks across several creatures is a real use case, so switching
		creature must keep both (2026-08-30, Nuno: "quand je change de pnj
		ça fais sauter la selection du slot" -- an earlier version of this
		method wrongly cleared _bind_slot_override here, meant to fix a
		DIFFERENT bug -- the loaded shape rendering twice, once standalone
		and once on the creature -- that's now fixed at its real source
		instead, see _apply_loaded_shape_to_creature()'s unconditional
		model_root hide. 2026-08-31, Nuno: same complaint for
		_bind_attach_point -- safe to keep too, since WEAPON_ATTACH_POINTS
		is a small fixed list shared by every humanoid skeleton used here,
		not arbitrary skeleton-specific bone names)."""
		self._bind_creature_name = name
		record = self._bind_creatures.get(name)
		if record is None:
			# "(none)": deselecting the creature entirely (name == "" from
			# the combo's own "(none)" entry, or an otherwise-unknown name)
			# -- nothing to show, so unlike a real pick this does NOT force
			# _show_assembled_creature back on.
			self._bind_skeleton = None
			self._bind_skeleton_name = ""
			self._show_assembled_creature = False
			self._rebuild_assembled_creature()
			return
		self._show_assembled_creature = True
		scanned_name = self._resolve_scanned_skeleton_name(record.skel)
		skeleton = self.search_paths_dialog.skeleton_for(scanned_name) if scanned_name is not None else None
		self._bind_skeleton = skeleton
		self._bind_skeleton_name = scanned_name or ""
		# Forces the main shape's own Panoply "skin" (race) selection to
		# match the picked creature's race (CreatureRecord.race, same 0-3
		# EGSPD::CPeople::TPeople indexing as panoply.RACES's own
		# ("fy","ma","tr","zo") order) -- without this, previewing e.g.
		# fy_HOM_armor01_bottes.shape bound to a Fyros creature could still
		# render with whatever race variant (say "zo") happened to be
		# selected from earlier, a mismatched skin tone on the wrong body
		# (bug found 2026-08-30, Nuno: "il faut que tu force le skin
		# panoply en fonction du pnj qu'on charge"). Harmless no-op if the
		# loaded shape's textures have no "skin" variants at all --
		# _reapply_all_materials() just won't find anything to change.
		if 0 <= record.race < len(panoply.RACES):
			self._panoply_selection["skin"] = panoply.RACES[record.race]
			self._reapply_all_materials()
		# Deliberately not routed through _apply_bone_preview_skeleton() --
		# that would also reassign the *main* shape's live-re-skin skeleton
		# (_bone_preview_skeleton) to this creature's, see _bind_skeleton's
		# own docstring for why that's wrong. No need to touch
		# _rebuild_geometry()/model_root here at all: whichever of the loaded
		# shape's three placements applies (see
		# _apply_loaded_shape_to_creature()) is entirely resolved by
		# _rebuild_assembled_creature() below, which calls that method itself.
		self._rebuild_assembled_creature()

	def _copy_creatures_ref_to_workspace(self):
		"""Copies the bundled default creatures_ref.txt into the active
		workspace, as an editable starting point -- same copy-then-edit
		mechanism as _copy_panoply_cfg_to_workspace(), see that method's own
		docstring. Picked from the gear icon in _draw_bind_controls()."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		dest = creature_ref.workspace_list_path(workspace_dir)
		try:
			shutil.copyfile(creature_ref.bundled_list_path(), dest)
		except OSError as exc:
			self._save_status = f"Could not copy creatures_ref.txt: {exc}"
			print(f"[object_editor] {self._save_status}")
			return
		self._save_status = f"Copied creatures_ref.txt to workspace ({dest})"
		print(f"[object_editor] {self._save_status}")
		# Re-resolve against the new override next _ensure_bind_creatures()
		# call -- note this doesn't yet (re)build a cache for names added to
		# the copy beyond the bundled 8 (see the chantier notes) -- those
		# just won't show up in the combo until that's wired up.
		self._bind_creatures = {}

	def _save_bind_slot_override(self):
		"""Persists the Bind preview's CURRENT slot-combo pick
		(self._bind_slot_override) for the currently loaded shape into the
		active workspace's own bind_slot_overrides.json (see
		creature_ref.workspace_slot_overrides_path()) and self._bind_slot_overrides
		(the in-memory copy _auto_detect_bind_slot() actually reads), so
		it's remembered the next time this shape loads -- higher priority
		there than the bundled shape_slot_index.json. Called ONLY from
		_write_shape(), when the shape itself is actually saved (2026-08-30,
		Nuno: "que ça ne sauve le slot QUE si on sauve le shape" -- picking
		a slot to preview/compare shouldn't silently persist a correction
		the user never actually confirmed by saving). An empty
		self._bind_slot_override removes any existing entry (picking
		"(none)" before saving is a deliberate correction too, e.g. for a
		shape the bundled index got wrong). No-op if there's no active
		workspace or no shape loaded -- nothing to key the entry by, or
		nowhere to write it."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None or not self._shape_source_name:
			return
		stem = Path(self._shape_source_name).stem.lower()
		if self._bind_slot_override:
			self._bind_slot_overrides[stem] = self._bind_slot_override
		else:
			self._bind_slot_overrides.pop(stem, None)
		creature_ref.save_slot_overrides(
			creature_ref.workspace_slot_overrides_path(workspace_dir), self._bind_slot_overrides)

	def _auto_detect_bind_slot(self):
		"""Auto-preselects the Bind preview's slot-override combo from, in
		priority order: (1) the active workspace's own manual corrections
		(self._bind_slot_overrides, loaded once by
		_on_active_workspace_changed() -- see _write_shape()'s own note on
		when this actually gets saved back to disk); (2) the loaded
		shape's own equipment-slot data (SLOTTYPE::TSlotType /
		CItemSheet.slot_bf, via creature_ref.load_shape_slot_index()'s
		bundled index) -- Nuno, 2026-08-30: "je charge
		fy_HOM_armor01_bottes.shape [...] qu'il sache déjà [...] quoi
		remplacer". Called from _display_shape() for every shape load.
		Silent no-op (leaves the combo on "(none)") when neither source has
		an answer, or the shape maps to a hand/weapon slot (out of
		BODY_SLOTS' scope) -- the manual combo/attach-point list remains
		available as a fallback in all cases."""
		self._bind_slot_override = ""
		if not self._shape_source_name:
			return
		stem = Path(self._shape_source_name).stem.lower()
		slot = self._bind_slot_overrides.get(stem)
		if not slot:
			slot = creature_ref.load_shape_slot_index().get(stem)
		if slot:
			self._bind_slot_override = slot
		# Cheap (no disk I/O): reuses whatever's already built for every
		# other slot, see _apply_loaded_shape_to_creature()'s own docstring
		# for why this, and not the expensive _rebuild_assembled_creature(),
		# is what belongs on the per-shape-load path.
		self._apply_loaded_shape_to_creature()

	def _rebuild_assembled_creature(self):
		"""(Re)builds the full-body reference under _assembled_creature_root
		from the currently-picked Bind-preview creature -- skeleton + every
		resolved body-part shape (creature_ref.BODY_SLOTS), plus its first
		hairstyle option if any (HairItemList lists interchangeable style
		*choices*, not several worn at once -- showing just one avoids
		stacking every alternative on top of each other). Static bind-pose
		only, no animation (this is a standing reference, not a preview of
		movement).

		EXPENSIVE (disk read + shape parse + skin per slot, ~seconds for the
		full body pre-vectorization, still visible cost after) -- only call
		this when the creature SELECTION itself changes (picking a creature
		in the combo, toggling the assembled-creature visibility on), never
		on every loaded shape. Where the loaded shape itself fits in (a slot
		override, an attach-point weapon, or neither) is layered on top
		afterwards by _apply_loaded_shape_to_creature(), which this method
		itself calls once at the end -- 2026-08-30, Nuno: routing every
		shape load through this full rebuild (to keep that in sync) made
		ordinary Explorer browsing laggy the moment a Bind-preview creature
		was shown, even though only the loaded shape's own placement
		actually needed to change."""
		self._assembled_creature_root.remove_node()
		self._assembled_creature_root = self.render.attach_new_node("assembled-creature-root")
		self._assembled_creature_base_nodes = {}
		self._assembled_creature_skin_states = {}
		self._assembled_creature_loaded_shape_node = None
		self._assembled_creature_loaded_shape_content_root = None
		# Carries the whole creature's pivot offset ONCE, so every child shape
		# below -- whether positioned at local-space identity (a skinned
		# body-part mesh, its bone_world_matrices already IS its final
		# position) or at a specific bone's matrix (a rigid, non-skinned one,
		# see the slot_name handling below) -- ends up in the same space as
		# the main loaded object, without each needing its own set_pos.
		self._assembled_creature_root.set_pos(self.render, self._object_pivot.get_pos(self.render))
		self._assembled_creature_bone_matrices = {}  # see _frame_camera()
		self._bind_animation = None
		self._bind_anim_duration = 0.0
		self._bind_anim_time = 0.0
		if not self._show_assembled_creature:
			self._apply_loaded_shape_to_creature()
			return
		record = self._bind_creatures.get(self._bind_creature_name)
		skeleton = self._bind_skeleton
		if record is None or skeleton is None:
			self._apply_loaded_shape_to_creature()
			return

		# Bind preview's Mode combo (_draw_bind_controls): poses the standing
		# reference against the real animation MBEHAV::EMode resolves to for
		# this creature (creature_ref.resolve_animation(), via the bundled
		# creatures_anim_cache.json -- see nel/tools/pynel/docs/
		# packed_sheets.md's "animset_list.packed_sheets" section for the
		# full chain), e.g. show it crouched in "SIT" or squared-up in
		# "COMBAT" instead of always the skeleton's raw bind pose. Resolved
		# once here into self._bind_animation -- _update_assembled_creature_skin()
		# reads it every frame (self._bind_anim_time, advanced by
		# _update_bind_anim_time() while the Play button is active) to
		# actually animate every live-capable body-part shape, not just pose
		# a static snapshot at t=0 (2026-08-31, Nuno: "Possible d'avoir un
		# bouton pour lancer l'animation?").
		if self._bind_mode:
			anim_name = creature_ref.resolve_animation(record, self._bind_mode, "Idle")
			if anim_name is not None:
				entry = self.search_paths_dialog.find_texture(anim_name)
				if entry is not None:
					try:
						self._bind_animation = parse_animation(entry.read_bytes())
						self._bind_anim_duration = animation_duration(self._bind_animation)
					except AnimationParseError as exc:
						print(f"[object_editor] Bind preview: cannot parse animation {anim_name!r}: {exc}")
				else:
					print(f"[object_editor] Bind preview: animation {anim_name!r} not found in scanned search paths")

		bone_world_matrices = evaluate_all_bone_world_matrices(skeleton, self._bind_animation, self._bind_anim_time)
		self._assembled_creature_bone_matrices = bone_world_matrices
		shape_entries = list(record.slots.items())  # (slot_name, shape_name)
		if record.hair:
			shape_entries.append(("hair", record.hair[0]))

		built_count = 0
		t_total_start = time.perf_counter()
		for slot_name, shape_name in shape_entries:
			t0 = time.perf_counter()
			entry = self.search_paths_dialog.find_texture(shape_name)
			t1 = time.perf_counter()
			if entry is None:
				print(f"[object_editor] assembled creature: {shape_name!r} not found in scanned search paths")
				continue
			try:
				data = entry.read_bytes()
				t2 = time.perf_counter()
				shape_file = parse_shape(data)
				t3 = time.perf_counter()
			except (ShapeParseError, OSError) as exc:
				print(f"[object_editor] assembled creature: cannot parse {shape_name!r}: {exc}")
				continue
			node_path = self._assembled_creature_root.attach_new_node(shape_name)
			# No special positioning needed here: face/head pieces (e.g.
			# *_visage.shape) turned out to be real skinned CMeshMRM (not
			# CMeshMRMSkinned, see _is_shape_skinned()) -- once
			# _build_assembled_shape_geometry() skins them like any other
			# body part, they land in the right place on their own, same
			# identity-plus-root-offset transform as everything else.
			t4 = time.perf_counter()
			_content_root, skin_state = self._build_assembled_shape_geometry(
				shape_file.value, skeleton, bone_world_matrices, node_path, slot_name)
			t5 = time.perf_counter()
			self._assembled_creature_base_nodes[slot_name] = node_path
			if skin_state is not None:
				self._assembled_creature_skin_states[slot_name] = skin_state
			built_count += 1
		t_total_end = time.perf_counter()
		self._apply_loaded_shape_to_creature()

	def _apply_loaded_shape_to_creature(self):
		"""(Re)shows wherever the currently loaded shape (self.shape_file)
		belongs relative to the Bind-preview creature -- exactly one of:
		(a) skinned into a body-part slot (self._bind_slot_override),
		hiding that slot's own default piece; (b) rigid, positioned at a
		chosen attach-point bone (self._bind_attach_point) -- a hand-held
		weapon; or (c) neither -- an "undefined" catch-all, shown at the
		creature root's own identity transform (the same place it would
		sit standalone). Simplified 2026-08-30 (Nuno: "tu fais une
		structure tres simple displayed_creature avec tous les slots [...]
		et meme un slot undefined si aucun slot n'est choisi") -- one
		unconditional rule replaces the previous scattered
		show()/hide() calls: self.model_root (the loaded shape's own
		standalone rendering, from _rebuild_geometry()) is shown whenever
		no creature is actually being displayed, and hidden -- ALWAYS,
		unconditionally -- whenever one is, because the loaded shape then
		always has a place inside the creature group instead (one of the
		three cases above). Exactly one of the two ever renders the loaded
		shape at a time, never both (bug found 2026-08-30, screenshot: two
		disconnected hand pieces "floating" was model_root rendering
		standalone AND unposed, at the same time as an identical copy
		correctly skinned into the creature).

		Cheap: no disk I/O, reuses _assembled_creature_base_nodes built by
		_rebuild_assembled_creature() -- safe to call every time just the
		loaded shape or its binding choice changes (auto-detect on every
		shape load, manual slot/attach-point combo pick) without
		re-parsing the other ~6 body-part shapes from disk each time -- see
		_rebuild_assembled_creature()'s own docstring for the perf
		regression this split was introduced to fix."""
		if self._assembled_creature_loaded_shape_node is not None:
			self._assembled_creature_loaded_shape_node.remove_node()
			self._assembled_creature_loaded_shape_node = None
			self._assembled_creature_loaded_shape_content_root = None
			self._assembled_creature_loaded_shape_skin_state = None
		for node_path in self._assembled_creature_base_nodes.values():
			node_path.show()
		# Only ever (re)shown in the attach-point branch below -- removed
		# here unconditionally first so every OTHER case (no creature, slot
		# override, "undefined") starts from "not shown" rather than needing
		# its own explicit hide.
		self._attach_point_axes_np.remove_node()

		creature_shown = self._bind_skeleton is not None and bool(self._assembled_creature_bone_matrices)
		if self.shape_file is None or not creature_shown:
			self.model_root.show()
			# Pivot gizmo back under _object_pivot (its normal home) --
			# see below for why it moves to content_root while bound.
			if self._pivot_axes_np.get_parent() != self._object_pivot:
				self._pivot_axes_np.reparent_to(self._object_pivot)
				self._pivot_axes_np.clear_transform()
			self._frame_camera()
			return
		self.model_root.hide()

		override_slot = self._bind_slot_override
		if override_slot:
			base_node = self._assembled_creature_base_nodes.get(override_slot)
			if base_node is not None:
				base_node.hide()
			node_path = self._assembled_creature_root.attach_new_node(f"{override_slot}-override")
			content_root, self._assembled_creature_loaded_shape_skin_state = self._build_assembled_shape_geometry(
				self.shape_file.value, self._bind_skeleton, self._assembled_creature_bone_matrices, node_path, override_slot)
			self._assembled_creature_loaded_shape_is_skinned_override = _is_shape_skinned(self.shape_file.value)
		elif self._bind_attach_point and self._bind_attach_point in self._bind_skeleton.bone_map:
			# Rigid, not skinned (skeleton=None forces iter_render_passes()'s
			# own bind-pose fallback -- see _build_assembled_shape_geometry(),
			# even for a shape that happens to be technically skinned data,
			# same reasoning _rebuild_geometry()'s attach-point case used to
			# rely on before this consolidation), then positioned by hand at
			# the chosen bone's current world matrix.
			node_path = self._assembled_creature_root.attach_new_node("attach-point")
			content_root, self._assembled_creature_loaded_shape_skin_state = self._build_assembled_shape_geometry(
				self.shape_file.value, None, None, node_path, "attach-point")
			self._assembled_creature_loaded_shape_is_skinned_override = False
			bone_matrix = self._assembled_creature_bone_matrices[self._bind_attach_point]
			node_path.set_mat(_nel_matrix_to_panda_mat4(bone_matrix))
			# Visual target marker for the bone itself, distinct from the
			# object's own pivot axes (_pivot_axes_np) -- lets the user
			# directly compare "where my shape's own local origin currently
			# is" (the pivot gizmo, moves with position/rotation/scale edits)
			# against "where the real client will actually stick it"
			# (this marker, fixed at the bone), instead of having to guess
			# (2026-08-31, Nuno: "je dois OBLIGATOIREMENT visualiser ce
			# point de préhension [...] le pivot de mon arme n'est pas sur
			# le point d'attache"). Sized off the loaded shape's own bbox,
			# same reasoning _rebuild_viewport_helpers() uses for the other
			# axes -- a fixed size would be comically large for a dagger and
			# invisible for a two-handed sword.
			bbox = shape_bbox(self.shape_file.value)
			axis_length = (max(bbox.half_size.x, bbox.half_size.y, bbox.half_size.z, 0.1) * _AXIS_MARGIN_FACTOR
			               if bbox is not None else _AXIS_LENGTH)
			self._attach_point_axes_np = self._assembled_creature_root.attach_new_node(
				_build_axes_geom(_ATTACH_POINT_AXIS_COLORS, axis_length))
			self._attach_point_axes_np.set_light_off()
			self._attach_point_axes_np.set_mat(_nel_matrix_to_panda_mat4(bone_matrix))
		else:
			# "undefined": doesn't correspond to any creature part -- shown
			# rigid (same skeleton=None reasoning as the attach-point case
			# above, no creature bone to skin an unrelated shape against) at
			# _assembled_creature_root's own identity transform, i.e. the
			# same place model_root would otherwise sit.
			node_path = self._assembled_creature_root.attach_new_node("undefined")
			content_root, self._assembled_creature_loaded_shape_skin_state = self._build_assembled_shape_geometry(
				self.shape_file.value, None, None, node_path, "undefined")
			self._assembled_creature_loaded_shape_is_skinned_override = False

		self._assembled_creature_loaded_shape_node = node_path
		# _update_bound_shape_rotation() keeps this synced with
		# self._object_pivot's LIVE rotation every frame from here on --
		# the set_quat() from base.default_rot_quat inside
		# _build_assembled_shape_geometry() above was only ever the initial
		# value (matches _object_pivot's own at load time), not a live link.
		self._assembled_creature_loaded_shape_content_root = content_root
		# Pivot gizmo moves here too, off _object_pivot (its standalone-view
		# home, still at the pre-bind world position) -- content_root IS
		# "the object's own local origin", already correctly positioned
		# wherever the bound copy actually sits (skinned slot, attach-point
		# bone, or undefined-at-creature-root) and kept live in sync by
		# _update_bound_shape_rotation(). Without this the gizmo still
		# rotated with edits (it inherited _object_pivot's rotation) but
		# stayed frozen at the pre-bind position, completely disconnected
		# from the actually-rendered mesh (bug found 2026-08-31, Nuno: "mon
		# objet tourne autour d'un axe qui n'est pas le pivot [...] il est
		# resté à sa position avant le bind").
		self._pivot_axes_np.reparent_to(content_root)
		self._pivot_axes_np.clear_transform()
		self._frame_camera()

	def _build_assembled_shape_geometry(self, shape_value, skeleton, bone_world_matrices, parent_node_path, slot_name):
		"""Same idea as _build_reference_geometry() (a non-editable preview
		shape, material_id=None so it never touches the main shape's own
		override-color/multi-bitmap-slot state), but skinned against
		`skeleton`/`bone_world_matrices` like the main shape's own
		_rebuild_geometry() does for a CMeshMRMSkinned -- body-part shapes
		are themselves skinned meshes bound to the whole creature skeleton,
		not static props."""
		materials = getattr(shape_value, "materials", None)
		is_skinned = _is_shape_skinned(shape_value)
		# Same CMeshBase::DefaultRotQuat the main shape's own standalone
		# view seeds _object_pivot with (_display_shape()) -- the engine's
		# real per-instance rotation, layered on TOP of whatever skinning
		# already did, not replaced by it. Missing here made a bound/
		# overridden shape with a non-identity export rotation (confirmed
		# real on some armor pieces) render mis-rotated once shown on a
		# creature (bug found 2026-08-30, Nuno: "les rotations ne sont plus
		# prises en compte"). Applied to a dedicated child node, not
		# `parent_node_path` itself, so it composes correctly regardless of
		# how the CALLER positions parent_node_path afterwards (e.g. the
		# attach-point case's own set_mat() in
		# _apply_loaded_shape_to_creature()).
		#
		# ONLY for a RIGID (not is_skinned) shape, though -- verified
		# 2026-08-31 against the real client's own world-matrix update,
		# transform.cpp:946 CTransform::updateWorldMatrixFromFather():
		# `if(!isSkinned() && _AncestorSkeletonModel) _WorldMatrix =
		# parentWM * _LocalMatrix；` -- when isSkinned() is true, this whole
		# block (and so _LocalMatrix, built from Default{Pos,RotQuat,Scale})
		# is skipped entirely; a skinned mesh's final vertex positions come
		# purely from applySkin()'s bone-matrix math
		# (skeleton_model.h:computeBoneMatrixes3x4(), no instance transform
		# involved at all -- confirmed no PreMul variant is used for regular
		# skinning). So for a skinned piece, DefaultRotQuat (and Pos/Scale)
		# has ZERO effect in the real game, not just position/scale --
		# applying it here for a skinned body-part/override was itself a
		# bug (an earlier "fix" for a rotation report that happened to look
		# right only because that particular shape's default_rot_quat was
		# identity, Nuno: "t'es sur que la rotation dois s'appliquer
		# aussi?").
		base = getattr(shape_value, "base", None)
		content_root = parent_node_path.attach_new_node(f"{slot_name}-content")
		if base is not None and not is_skinned:
			rot = base.default_rot_quat
			content_root.set_quat(Quat(rot.w, rot.x, rot.y, rot.z))
		# Live re-skin support (_update_assembled_creature_skin(), Bind
		# preview's Play button, 2026-08-31: "vu que tout le corps + cheveux
		# bougent, ben le visage doit bouger aussi") -- two different
		# per-vertex data layouts depending on the real shape type, so two
		# different _SkinState-like builders/reskin functions
		# (_update_assembled_creature_skin() dispatches on isinstance()):
		# CMeshMRMSkinned's packed-vertex format (_build_skin_state()/
		# _reskin_state()) vs. a plain skinned CMeshMRM's own
		# geom.skin_weights layout (_build_mrm_skin_state()/
		# _reskin_mrm_state(), e.g. *_visage.shape face pieces).
		skin_state = None
		if is_skinned and skeleton is not None:
			geom_value = shape_geom(shape_value)
			if isinstance(shape_value, MeshMRMSkinned):
				skin_state = _build_skin_state(geom_value, skeleton)
			else:
				skin_state = _build_mrm_skin_state(geom_value, skeleton)
		vdata = None
		pass_count = 0
		for vertex_buffer, material_id, indices in iter_render_passes(
				shape_value, skeleton=skeleton if is_skinned else None,
				bone_world_matrices=bone_world_matrices if is_skinned else None):
			if not indices:
				continue
			if vdata is None:
				vdata = _build_vertex_data(vertex_buffer, dynamic=skin_state is not None)
			geom = _build_geom(vdata, indices)
			geom_node = GeomNode(f"assembled-pass-{material_id}")
			geom_node.add_geom(geom)
			node_path = content_root.attach_new_node(geom_node)
			material = materials[material_id] if materials and material_id < len(materials) else None
			self._apply_material_common(node_path, material)
			self._apply_material_texture(node_path, material, None)
			texture_names = [t.file_name if t is not None else None for t in material.textures] if material is not None else None
			pass_count += 1
		if skin_state is not None:
			skin_state.vdata = vdata
		return content_root, skin_state

	def _draw_bind_controls(self):
		"""Floating "Bind preview" window: previews the loaded shape on one
		of Patina's curated reference creatures. Two real cases (confirmed
		2026-08-30, correcting an earlier wrong design -- see
		/repos/project-todos/ryzom-core/forgery-object-editor.md "Creature/
		NPC binder + assembler in Patina"):

		- **Skinned shape** (most equipment, including armor -- confirmed
		  most "armor" pieces are skinned meshes, not rigid props): it's
		  already bound to the whole skeleton, not one point. What's
		  actually useful here is picking which creature *slot* it should
		  stand in for (e.g. preview `fy_HOM_armor01_bottes.shape` in place
		  of the creature's own default `fy_HOM_civil01_bottes.shape` for
		  "feet") -- see `_bind_slot_override`, consumed by
		  _rebuild_assembled_creature().
		- **Non-skinned shape**: the real "bound to one attach point" case,
		  but only for the 2 hand slots in practice -- creature_ref.
		  WEAPON_ATTACH_POINTS (box_arme/box_arme_gauche/Box_bouclier),
		  hardcoded in character_cl.cpp, not general equipment (CONFIRMED
		  WRONG EARLIER: CCharacterSheet.BodyToBone is NOT attach-point
		  data, see CreatureRecord.body_to_bone's own docstring -- almost
		  nothing in real content actually needs a single attach point).

		Shown for any loaded shape (both cases share the creature picker),
		stacked below _draw_bone_preview_controls() ("Skinning preview",
		shown only for a skinned shape) when that one is also showing."""
		if self.shape_file is None:
			return

		is_skinned = _is_shape_skinned(self.shape_file.value)
		creatures = self._ensure_bind_creatures()

		display_width = imgui.get_io().display_size.x
		win_w, win_h = self._bind_panel_size
		x = display_width - self.panel_width - _VIEWPORT_TOGGLE_MARGIN_PX - win_w
		y = _VIEWPORT_TOGGLE_MARGIN_PX
		if self._wind_state is not None:
			y += self._wind_panel_size[1] + _VIEWPORT_TOGGLE_MARGIN_PX
		if is_skinned:
			# Skinning preview is shown above for a skinned shape (same
			# precondition already checked there) -- stack below its
			# current size (updated earlier this same frame, draw_panel()
			# calls it before this method).
			y += self._bone_preview_panel_size[1] + _VIEWPORT_TOGGLE_MARGIN_PX
		imgui.set_next_window_pos((x, y))
		flags = (imgui.WindowFlags_.no_move.value | imgui.WindowFlags_.no_collapse.value
		         | imgui.WindowFlags_.always_auto_resize.value)
		with imgui_ctx.begin("Bind preview", flags=flags):
			if self._bind_cache_rebuild is not None and not self._bind_cache_rebuild["done"]:
				imgui.text_disabled("Rebuilding creature cache...")
			imgui.set_next_item_width(220)
			preview = self._bind_creature_name or "(choose a creature)"
			if imgui.begin_combo("##bind-creature-combo", preview):
				clicked, _ = imgui.selectable("(none)", self._bind_creature_name == "")
				if clicked:
					self._select_bind_creature("")
				for name in sorted(creatures):
					clicked, _ = imgui.selectable(name, name == self._bind_creature_name)
					if clicked:
						self._select_bind_creature(name)
				imgui.end_combo()
			imgui.same_line()
			workspace_dir = self.workspace_setup_dialog.active_workspace_dir
			has_override = workspace_dir is not None and creature_ref.workspace_list_path(workspace_dir).is_file()
			if has_override:
				editor_path = app_settings.load().text_editor_path
				tooltip = "Edit this workspace's creatures_ref.txt in the configured text editor"
				if _icon_button(f"{fa_icons.ICON_FA_EDIT}##creatures-ref", tooltip):
					if not editor_path:
						self.request_settings_attention("Tools", "text_editor_path")
					else:
						subprocess.Popen([editor_path, str(creature_ref.workspace_list_path(workspace_dir))])
			else:
				tooltip = "Copy the bundled creatures_ref.txt into this workspace, to track more creatures"
				if _icon_button(f"{fa_icons.ICON_FA_COG}##creatures-ref", tooltip, disabled=workspace_dir is None):
					self._copy_creatures_ref_to_workspace()

			# Poses the standing reference against a real animation instead of
			# the skeleton's raw bind pose (creature_ref.resolve_animation(),
			# see _rebuild_assembled_creature()'s own note on the full
			# Mode->AnimSet->.anim resolution chain). Disabled without a
			# creature picked -- nothing to pose.
			imgui.set_next_item_width(220)
			preview = self._bind_mode or "(bind pose)"
			imgui.begin_disabled(not self._bind_creature_name)
			if imgui.begin_combo("##bind-mode-combo", preview):
				clicked, _ = imgui.selectable("(bind pose)", self._bind_mode == "")
				if clicked:
					self._bind_mode = ""
					self._rebuild_assembled_creature()
				for mode_name in creature_ref.ANIM_MODES:
					clicked, _ = imgui.selectable(mode_name, mode_name == self._bind_mode)
					if clicked:
						self._bind_mode = mode_name
						self._rebuild_assembled_creature()
				imgui.end_combo()
			imgui.end_disabled()

			# Live playback (Play button, 2026-08-31: "Possible d'avoir un
			# bouton pour lancer l'animation?") -- only shown once a Mode
			# actually resolved to a real, parsed animation (self._bind_animation,
			# see _rebuild_assembled_creature()). Re-skins every live-capable
			# body-part shape every frame while playing (_update_assembled_creature_skin()),
			# same Play/Pause + time-slider layout as the Skinning preview's
			# own animation controls above.
			if self._bind_animation is not None:
				icon = fa_icons.ICON_FA_PAUSE if self._bind_anim_playing else fa_icons.ICON_FA_PLAY
				if _icon_button(f"{icon}##bind-anim-play", "Play/pause"):
					self._bind_anim_playing = not self._bind_anim_playing
				imgui.same_line()
				imgui.set_next_item_width(160)
				changed, new_time = imgui.slider_float(
					"##bind-anim-time", self._bind_anim_time,
					0.0, max(self._bind_anim_duration, 0.001), "%.2f s")
				if changed:
					self._bind_anim_time = new_time

			if is_skinned:
				slot_names = list(creature_ref.BODY_SLOTS) + ["hair"]
				imgui.set_next_item_width(220)
				preview = self._bind_slot_override or "(none)"
				if imgui.begin_combo("##bind-slot-override-combo", preview):
					clicked, _ = imgui.selectable("(none)", self._bind_slot_override == "")
					if clicked:
						self._bind_slot_override = ""
						self._apply_loaded_shape_to_creature()
					for slot_name in slot_names:
						clicked, _ = imgui.selectable(slot_name, slot_name == self._bind_slot_override)
						if clicked:
							self._bind_slot_override = slot_name
							was_hidden = not self._show_assembled_creature
							self._show_assembled_creature = True
							# Turning visibility on here (rather than via the
							# ICON_FA_MALE toggle) needs the expensive full
							# rebuild first -- _assembled_creature_base_nodes
							# is empty/stale while hidden (see
							# _rebuild_assembled_creature()'s early-return).
							if was_hidden:
								self._rebuild_assembled_creature()
							else:
								self._apply_loaded_shape_to_creature()
					imgui.end_combo()
			else:
				imgui.set_next_item_width(220)
				preview = self._bind_attach_point or "(none)"
				if imgui.begin_combo("##bind-attach-point-combo", preview):
					clicked, _ = imgui.selectable("(none)", self._bind_attach_point == "")
					if clicked:
						self._bind_attach_point = ""
						self._apply_loaded_shape_to_creature()
					for curated_name in creature_ref.WEAPON_ATTACH_POINTS:
						resolved_name = self._resolve_bone_name(self._bind_skeleton, curated_name)
						clicked, _ = imgui.selectable(curated_name, curated_name == self._bind_attach_point)
						if clicked:
							if resolved_name is None:
								print(f"[object_editor] Bind preview: attach point {curated_name!r} not found in "
								      f"the loaded skeleton's own bones")
							self._bind_attach_point = resolved_name if resolved_name is not None else curated_name
							self._apply_loaded_shape_to_creature()
					imgui.end_combo()

			if _icon_button(
					fa_icons.ICON_FA_MALE, "Show this creature fully assembled as a standing reference",
					self._show_assembled_creature, disabled=not self._bind_creature_name):
				self._show_assembled_creature = not self._show_assembled_creature
				self._rebuild_assembled_creature()

			self._bind_panel_size = (imgui.get_window_size().x, imgui.get_window_size().y)
