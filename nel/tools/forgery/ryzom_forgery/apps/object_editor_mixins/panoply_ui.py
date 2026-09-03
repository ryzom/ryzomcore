"""ObjectEditorApp mixin: the Panoply integration -- global axis-picker
section (skin/user/hair/eyes), live-preview texture resolution/freshness
tracking, mask creation, and the real offline bake (single texture / whole
shape) with its progress popup. Split out of object_editor.py, see the
"Split object_editor.py into theme files" chantier in
project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

import shutil
import subprocess
import threading
from pathlib import Path

from imgui_bundle import icons_fontawesome_6 as fa_icons, imgui
from panda3d.core import PNMImage

from pynel import repository_paths

from ryzom_forgery import dds_export, panoply, panoply_bake, panoply_colorize, panoply_config, panoply_live
from ryzom_forgery import panoply_texture
from ryzom_forgery import settings as app_settings
from ryzom_forgery.popup_utils import center_next_popup
from ryzom_forgery.shape_geometry import load_panda_texture, resolve_texture_ref
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import _icon_button

_BAKE_PROGRESS_POPUP_ID = "Baking Panoply variants"


class PanoplyUIMixin:
	def _panoply_dims_for(self, base_name):
		"""Shape-wide panoply axis selection (_panoply_selection, see
		_draw_global_panoply_section()) narrowed down to just the axes
		`base_name` actually has a variant for, per panoply_files.txt --
		{} if nothing is selected, or this texture has no variants at all
		(e.g. a buckle/weapon that doesn't change color). Shared by
		_resolve_panoply_texture_name() (what file name to try loading) and
		_ensure_live_panoply_texture() (what to try computing live if that
		file turns out to be missing/stale)."""
		if not self._panoply_selection or not base_name:
			return {}
		available = self.search_paths_dialog.panoply_variants_for(base_name)
		return {
			axis: value for axis, value in self._panoply_selection.items()
			if value in available.get(axis, ())
		}

	def _resolve_panoply_texture_name(self, base_name):
		"""`base_name` unchanged, unless _panoply_dims_for() finds one or
		more applicable axis picks -- only ever affects what gets loaded for
		rendering/preview, the shape's own material data (base_name itself)
		is never modified. When the resulting variant is missing or stale on
		disk (older than the base texture or a mask it was built from --
		panoply_live.is_baked_stale()), and every mask this selection needs
		is actually available, pre-computes it live in memory
		(_ensure_live_panoply_texture()) and inserts it into
		self._texture_cache under the resolved name -- every caller of
		load_panda_texture(resolved_name, cache=self._texture_cache, ...)
		(3D render, thumbnails, popups) then picks it up transparently
		through that cache's own name lookup, no special case of their own
		needed. Never writes to disk. Also records `base_name`/`dims` under
		the resolved name in self._panoply_texture_sources, so
		_update_texture_freshness() can keep re-checking this combination
		later even though this method itself only runs again when the
		material actually gets re-applied."""
		dims = self._panoply_dims_for(base_name)
		if not dims:
			return base_name
		resolved_name = panoply.variant_file_name(base_name, **dims)
		self._panoply_texture_sources[resolved_name] = (base_name, dims)
		self._ensure_live_panoply_texture(base_name, resolved_name, dims)
		return resolved_name

	def _resolve_panoply_refs(self, base_name, dims):
		"""Every search_paths.FoundEntry a live recompute of this
		(base_name, dims) combination would need or want to know about: the
		base texture (None if it can't be resolved at all), each dims
		axis's mask (only the ones that actually resolve -- missing ones
		are just absent from the dict, not None entries), and whatever's
		already on disk for the baked variant name itself (None if
		nothing's there). Shared by _ensure_live_panoply_texture() (decides
		whether/how to (re)compute) and _update_texture_freshness() (decides
		whether a previously-resolved combination needs re-checking) so
		both always resolve names exactly the same way."""
		finder = self.search_paths_dialog.find_texture
		base_ref = resolve_texture_ref(base_name, self._texture_search_dirs, finder)
		stem = Path(base_name).stem
		mask_refs = {}
		for axis in dims:
			mask_ref = resolve_texture_ref(f"{stem}_{axis}.tga", self._texture_search_dirs, finder)
			if mask_ref is not None:
				mask_refs[axis] = mask_ref
		resolved_name = panoply.variant_file_name(base_name, **dims)
		baked_ref = resolve_texture_ref(resolved_name, self._texture_search_dirs, finder)
		return base_ref, mask_refs, baked_ref

	@staticmethod
	def _panoply_freshness_signature(base_ref, mask_refs, baked_ref):
		"""(base mtime, sorted mask mtimes, baked mtime-or-None) for a
		resolved combination's current sources -- not "is this stale"
		(panoply_live.is_baked_stale() already answers that), but "has
		anything actually changed since I last looked", which is what
		_update_texture_freshness() needs to avoid re-evicting/recomputing
		an unstale-but-not-baked live combination every single tick."""
		base_mtime = base_ref.cache_stat()[0] if base_ref is not None else None
		mask_mtimes = tuple(sorted((axis, ref.cache_stat()[0]) for axis, ref in mask_refs.items()))
		baked_mtime = baked_ref.cache_stat()[0] if baked_ref is not None else None
		return base_mtime, mask_mtimes, baked_mtime

	def _ensure_live_panoply_texture(self, base_name, resolved_name, dims):
		"""Best-effort: if `resolved_name` isn't already cached and the
		already-baked file for it turns out to be missing/stale
		(panoply_live.is_baked_stale()), recolors the base texture live in
		memory (panoply_colorize.py, parameters from panoply_config.py) and
		pre-inserts the result into self._texture_cache -- see
		_resolve_panoply_texture_name()'s docstring for why that's enough
		for every caller to pick it up. A no-op whenever the base texture or
		any mask this selection needs can't be resolved, or this texture's
		race doesn't actually have one of the selected axes (e.g. no "eyes"
		for zorai) -- falls through to whatever load_panda_texture() itself
		finds on disk instead, same as if this method didn't exist. Records
		the freshness signature seen at this point either way (even when
		the baked file was fine as-is), so _update_texture_freshness() has
		a baseline to compare later ticks against."""
		if resolved_name in self._texture_cache:
			return
		base_ref, mask_refs, baked_ref = self._resolve_panoply_refs(base_name, dims)
		if base_ref is None or set(mask_refs) != set(dims):
			return  # missing the base texture or a mask one of the selected axes needs -- can't compute this live

		self._panoply_texture_signatures[resolved_name] = self._panoply_freshness_signature(base_ref, mask_refs, baked_ref)
		if not panoply_live.is_baked_stale(baked_ref, base_ref, mask_refs):
			return  # the baked file on disk is fine as-is -- load_panda_texture() will load it normally

		stem = Path(base_name).stem
		cache_key = panoply_live.LiveColorizeCache.make_key(base_name, dims, base_ref, mask_refs)
		result_rgba = self._live_panoply_cache.get(cache_key)
		if result_rgba is None:
			base_rgba = panoply_texture.ref_to_rgba_array(base_ref)
			if base_rgba is None:
				return
			race = panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower())
			axis_masks = []
			for axis in panoply.AXES:
				if axis not in dims:
					continue
				params = panoply_config.get_color_params(axis, panoply.color_id_for(axis, dims[axis]), race)
				if params is None:
					return
				mask_rgba = panoply_texture.ref_to_rgba_array(mask_refs[axis])
				if mask_rgba is None:
					return
				axis_masks.append((mask_rgba[..., 0], params))  # mask weight lives in the red channel
			result_rgba = panoply_colorize.colorize(base_rgba, axis_masks)
			self._live_panoply_cache.set(cache_key, result_rgba)

		self._texture_cache[resolved_name] = panoply_texture.rgba_array_to_texture(result_rgba)

	_TEXTURE_FRESHNESS_CHECK_INTERVAL = 1.0  # seconds

	def _update_texture_freshness(self, task):
		"""Runs about once a second, two checks in one pass:

		1. Every Panoply-affected texture name currently in play
		(self._panoply_texture_sources), re-resolved against its current
		on-disk sources and compared to the signature recorded when it was
		last resolved (_panoply_freshness_signature()) -- on a mismatch (a
		base texture, mask, or baked file was added/edited/removed since),
		evicts it from self._texture_cache, which forces
		_resolve_panoply_texture_name()/_ensure_live_panoply_texture() to
		run again for it on the next material re-apply below.

		2. Every other (non-Panoply) name already in self._texture_cache --
		a plain material texture, or whichever Multi Bitmap slot is
		currently selected (see _pad_multi_bitmap_slots()/_apply_material_
		texture() -- only the selected slot's name is ever actually loaded/
		cached, so this needs no Multi-Bitmap-specific handling) -- re-
		resolved and compared by mtime against self._texture_freshness_
		mtimes (seeded in _apply_material_texture() at load time, not here,
		so the very first tick after a texture is first cached never sees a
		spurious "changed"). Deliberately restricted to files inside the
		active workspace (FoundEntry.fs_path, skipping .bnp entries and
		anything outside it) -- textures from other search paths (mods, the
		Ryzom install, etc.) are left alone, so the workspace stays the one
		place edits get picked up live, same idea as the auto-export
		imports/ watcher only ever touching the active workspace.

		Either check evicting anything forces every material to re-apply
		once, at the end. This is Forgery's whole answer to "I edited a
		texture in Gimp, now what" -- picked automatically, with no manual
		reload action: see .todo/forgery-object-editor.md's Phase A Step 5
		for why a manual "Reload" button was decided against once the
		Panoply half of this existed."""
		if task.time - self._texture_freshness_last_check < self._TEXTURE_FRESHNESS_CHECK_INTERVAL:
			return task.cont
		self._texture_freshness_last_check = task.time

		changed = False
		if self._panoply_cfg_changed:
			# The workspace's panoply.cfg (or its absence) changed -- see
			# _on_panoply_cfg_settled(). Every already-rendered Panoply
			# texture was colorized with whatever parameters were active at
			# the time, and neither _texture_cache nor _live_panoply_cache's
			# keys depend on those parameters (see LiveColorizeCache.make_key()),
			# so nothing else would ever notice this changed -- evict every
			# Panoply-tracked entry and drop the whole live-colorize cache,
			# forcing a full recompute against the new colors below.
			self._panoply_cfg_changed = False
			for resolved_name in list(self._panoply_texture_sources):
				self._texture_cache.pop(resolved_name, None)
			self._panoply_texture_signatures.clear()
			self._live_panoply_cache.clear()
			changed = True

		for resolved_name, (base_name, dims) in list(self._panoply_texture_sources.items()):
			if resolved_name not in self._texture_cache:
				continue
			base_ref, mask_refs, baked_ref = self._resolve_panoply_refs(base_name, dims)
			if base_ref is None or set(mask_refs) != set(dims):
				continue
			signature = self._panoply_freshness_signature(base_ref, mask_refs, baked_ref)
			if signature != self._panoply_texture_signatures.get(resolved_name):
				del self._texture_cache[resolved_name]
				self._panoply_texture_signatures.pop(resolved_name, None)
				changed = True

		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is not None:
			finder = self.search_paths_dialog.find_texture
			for name in list(self._texture_freshness_mtimes):
				if name not in self._texture_cache or name in self._panoply_texture_sources:
					self._texture_freshness_mtimes.pop(name, None)
					continue
				ref = resolve_texture_ref(name, self._texture_search_dirs, finder)
				if ref is None or ref.fs_path is None or not ref.fs_path.is_relative_to(workspace_dir):
					continue
				try:
					mtime = ref.cache_stat()[0]
				except OSError:
					# Resolved a moment ago (resolve_texture_ref() above), but
					# gone by the time we stat it -- deleted/renamed between
					# the two, e.g. externally or via a workspace file-manager
					# action. Leave the cached texture as the last-known-good
					# one instead of crashing the whole render loop over it.
					continue
				if mtime != self._texture_freshness_mtimes[name]:
					del self._texture_cache[name]
					self._texture_freshness_mtimes.pop(name, None)
					changed = True

		if changed:
			self._reapply_all_materials()
		return task.cont

	def _shape_texture_names(self):
		"""Every distinct, currently-active texture base name across the
		whole shape's materials (slot 0 -- for a Multi Bitmap material,
		texture.file_name is whichever quality/season/ecosystem set is
		presently selected, same one _apply_material_texture() renders) --
		the set _draw_global_panoply_section() checks against
		panoply_files.txt to decide which axis buttons to offer."""
		materials = getattr(self.shape_file.value, "materials", None) or []
		names = set()
		for material in materials:
			texture = material.textures[0] if material.textures else None
			if texture is not None and texture.file_name:
				names.add(texture.file_name)
		return names

	_PANOPLY_AXIS_LABELS = {"skin": "Skin", "user": "User color", "hair": "Hair", "eyes": "Eyes"}

	def _copy_panoply_cfg_to_workspace(self):
		"""Copies the bundled default panoply.cfg (panoply_config.py) as-is
		into the active workspace's root, as an editable starting point --
		once there, panoply_config._resolve_cfg_path() always prefers it
		over the bundled default from then on (both for the live-preview
		colors and any real bake -- see _bake_panoply_real()). Picked from
		the gear icon next to "Panoply:" in _draw_global_panoply_section()."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		dest = panoply_config.workspace_cfg_path(workspace_dir)
		try:
			shutil.copyfile(panoply_config.bundled_cfg_path(), dest)
		except OSError as exc:
			self._save_status = f"Could not copy panoply.cfg: {exc}"
			print(f"[object_editor] {self._save_status}")
			return
		self._save_status = f"Copied panoply.cfg to workspace ({dest})"
		print(f"[object_editor] {self._save_status}")

	def _on_panoply_cfg_settled(self, path):
		"""Registered onto the shared WorkspaceWatcher for the workspace
		root's own panoply.cfg -- a root-level file's relative_to(workspace_dir)
		has a single part, which WorkspaceWatcher._dispatch() treats as its
		own "subdir" key, so this fires whenever <workspace>/panoply.cfg
		itself settles (created, edited, or deleted), same mechanism as the
		"tex"/"imports" subfolder registrations just needs no subfolder here.
		Runs off that watcher's background thread -- only sets a flag,
		_update_texture_freshness() (main thread, ~once a second) does the
		actual cache eviction + material re-apply on the next tick. Without
		this, an edited workspace panoply.cfg would never be picked up by
		the live preview: panoply_config.py's own cache invalidates itself
		lazily by mtime, but nothing else would know to recompute/re-cache
		the already-rendered Panoply textures for the new colors."""
		self._panoply_cfg_changed = True

	def _draw_global_panoply_section(self):
		"""Ryzom's pre-baked texture recolors (see panoply.py), shape-wide:
		each axis's pick (skin tone, user/craft color, hair color, eye color)
		applies to every one of the shape's textures at once, independently
		of the other axes -- skin is a race skin-tone difference (Fyros
		tanned, Matis pale, Tryker in between, Zorai blue), user color is an
		item's craft color, both meant to be uniform across a whole equipped
		piece, not picked per texture; hair/eyes are the same idea for a
		head/face shape. A texture with no mask for a given axis (e.g. an
		armor piece has no hair/eyes masks, a hairstyle has no user masks)
		just keeps rendering its own base name for that axis -- expected, not
		a missing-data problem (see _resolve_panoply_texture_name()). Shown
		once, at the top of the Textures and Materials tabs alike. Purely a
		render override -- never edits the shape's own material data.

		Buttons offered on each axis are the union across every texture the
		shape actually uses right now (no single texture's own list is
		authoritative for what to show, since different textures can support
		different axes/values). No real texture on disk is ever the base
		name alone with just one axis picked (e.g. "..._FY.tga" doesn't
		exist, only "..._FY_U1.tga" does -- panoply_maker's masks are always
		baked together whenever more than one applies to the same texture),
		so the first click on an empty selection backfills every other
		available axis with its first value too, instead of resolving to a
		texture that isn't actually on disk (rendering as blank/white)."""
		texture_names = self._shape_texture_names()
		available = {axis: set() for axis in panoply.AXES}
		for name in texture_names:
			for axis, values in self.search_paths_dialog.panoply_variants_for(name).items():
				available[axis].update(values)

		imgui.text("Panoply:")
		imgui.same_line()
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		has_override = workspace_dir is not None and panoply_config.workspace_cfg_path(workspace_dir).is_file()
		if has_override:
			editor_path = app_settings.load().text_editor_path
			if _icon_button(fa_icons.ICON_FA_PEN_TO_SQUARE, "Edit this workspace's panoply.cfg in the configured text editor"):
				if not editor_path:
					self.request_settings_attention("Tools", "text_editor_path")
				else:
					subprocess.Popen([editor_path, str(panoply_config.workspace_cfg_path(workspace_dir))])
		else:
			tooltip = "Copy the bundled panoply.cfg into this workspace, to edit its colors"
			if _icon_button(fa_icons.ICON_FA_GEAR, tooltip, disabled=workspace_dir is None):
				self._copy_panoply_cfg_to_workspace()
		imgui.same_line()
		if _icon_button(
			fa_icons.ICON_FA_FIRE, "Bake real Panoply variants for every texture of this shape (writes tex/ + build/)",
			disabled=workspace_dir is None,
		):
			self._bake_panoply_real_all()
		if not any(available.values()):
			imgui.same_line()
			imgui.text_disabled("no variants detected for this shape's textures")
			imgui.separator()
			return

		axis_values = {}
		for axis in panoply.AXES:
			values = panoply.RACES if axis == "skin" else sorted(available[axis])
			axis_values[axis] = [value for value in values if value in available[axis]]

		# Auto-default once per shape (2026-08-29): without this, a freshly
		# loaded shape with real variants (_panoply_selection still empty,
		# per _reset_shape_state()) rendered blank/white until the user
		# clicked a button themselves -- same backfill-with-first-value
		# logic as an empty-selection click below, just applied proactively
		# the first time real variants are detected instead of waiting for
		# a click. _panoply_selection_defaulted (not "is _panoply_selection
		# empty") is what gates this, so it fires exactly once per shape --
		# a later click on "skin" that deliberately clears back to the
		# shape's own base texture (see below) must stay cleared, not get
		# re-defaulted right back on the next frame.
		if not self._panoply_selection_defaulted and any(axis_values.values()):
			for axis, values in axis_values.items():
				if values:
					self._panoply_selection[axis] = values[0]
			self._panoply_selection_defaulted = True
			self._reapply_all_materials()

		# Bind preview (_select_bind_creature()) forces "skin" to the picked
		# creature's own race -- same condition _apply_loaded_shape_to_creature()
		# uses for "is a creature actually being shown". Without disabling
		# the other race buttons here, clicking one manually while a
		# creature is shown silently re-mismatches skin tone vs body again
		# (the exact bug this forcing was added to fix) with no visual sign
		# anything is different, which reads as "the force is buggy/
		# unreliable" rather than "this is locked to the creature's race"
		# (2026-08-30, Nuno: "quand tu force le skin de panoply il faut
		# griser les autres.. sinon on pense qu'il y a un bug").
		skin_forced_by_creature = self._bind_skeleton is not None and bool(self._assembled_creature_bone_matrices)

		for axis in panoply.AXES:
			values = axis_values[axis]
			if not values:
				continue
			selected = self._panoply_selection.get(axis)
			imgui.text(f"{self._PANOPLY_AXIS_LABELS[axis]}:")
			for value in values:
				imgui.same_line()
				active = value == selected
				label = value if axis == "skin" else f"{axis[0]}{value}"
				tooltip = f"{self._PANOPLY_AXIS_LABELS[axis]} {value!r}"
				locked = axis == "skin" and skin_forced_by_creature and not active
				if locked:
					tooltip += f" (locked to {self._bind_creature_name}'s own race -- deselect the creature to change)"
				if _icon_button(label, tooltip, active=active, disabled=locked):
					if active:
						# Only "skin" can be turned back off (falling back to
						# the shape's own base texture, its meaningful "no
						# override" state) -- user/hair/eyes never had a
						# "none" state to begin with (no real texture on disk
						# for e.g. just "..._U1.tga" alone, always paired with
						# a skin), so clicking their already-active button is
						# a no-op, not a way to leave the shape half-resolved.
						if axis == "skin":
							self._panoply_selection.clear()
						else:
							continue
					else:
						was_empty = not self._panoply_selection
						self._panoply_selection[axis] = value
						if was_empty:
							for other_axis, other_values in axis_values.items():
								if other_axis != axis and other_values:
									self._panoply_selection[other_axis] = other_values[0]
					self._reapply_all_materials()
		imgui.separator()

	def _create_panoply_mask(self, base_texture_name, axis):
		"""Creates a new mask file for `axis` -- solid black (0 weight
		everywhere, no effect until painted over), at the same pixel
		dimensions as `base_texture_name`'s own texture -- directly in the
		active workspace's masks/ folder. Picked from the "+" button in
		_draw_panoply_masks_for()."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		base_panda_texture = load_panda_texture(
			base_texture_name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
			finder=self.search_paths_dialog.find_texture)
		width = base_panda_texture.get_x_size() if base_panda_texture is not None else 256
		height = base_panda_texture.get_y_size() if base_panda_texture is not None else 256
		image = PNMImage(width, height)
		image.fill(0, 0, 0)
		stem = Path(base_texture_name).stem
		dest = workspace_dir / "masks" / f"{stem}_{axis}.tga"
		try:
			dest.parent.mkdir(parents=True, exist_ok=True)
			image.write(str(dest))
		except OSError as exc:
			self._save_status = f"Could not create mask {dest.name}: {exc}"
			print(f"[object_editor] {self._save_status}")
			return
		self._save_status = f"Created mask {dest.name}"
		print(f"[object_editor] {self._save_status}")

	def _bake_panoply_real(self, base_texture_name, on_variant=None):
		"""Real offline Panoply bake (panoply_bake.py -- the exact,
		non-live-preview port of panoply_maker.cpp, see
		/repos/project-todos/ryzom-core/panoply-runtime-tint.md) for one
		base texture: writes every color variant into the active workspace's
		tex/, and an updated .hlsinfo/panoply_files.txt/characters.hlsbank
		into its build/ (started from, never overwriting, the real
		production files under ryzom-data's final_bnps/characters_maps_hr/,
		see panoply_bake.bake_and_write()'s docstring). Distinct from the
		approximate live-preview already driving the axis pickers in
		_draw_global_panoply_section() -- this produces the real baked files
		panoply_maker.exe would, not just a render override. Restricted to
		plain on-disk files (FoundEntry.fs_path) -- a base texture or mask
		living only inside a .bnp can't be baked from here.

		Runs on _start_panoply_bake()'s background thread (2026-08-29) --
		never touches imgui or Settings-attention (the active-workspace/
		ryzom-data checks live in _start_panoply_bake() instead, on the main
		thread, before the thread is even started -- neither imgui calls nor
		request_settings_attention() are safe off the main thread).
		`on_variant`, if given, is forwarded straight to
		panoply_bake.bake_and_write() for live progress reporting. Returns
		the list of written texture paths (empty if nothing was written)."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		entry = self.search_paths_dialog.find_texture(base_texture_name)
		if entry is None or entry.fs_path is None:
			self._save_status = f"Can't bake {base_texture_name}: not a plain file on disk"
			print(f"[object_editor] {self._save_status}")
			return []

		stem = Path(base_texture_name).stem
		race = panoply_config.RACE_PREFIX_TO_TABLE.get(stem[:2].lower())
		axes = panoply_bake.axes_for_source(stem, race)

		def _mask_loader(mask_ext):
			mask_entry = self.search_paths_dialog.find_texture(f"{stem}_{mask_ext}.tga")
			if mask_entry is None or mask_entry.fs_path is None:
				return None
			return panoply_bake.load_mask_luminance(mask_entry.fs_path)

		data_root = repository_paths.get("ryzom-data")
		hlsbank_source = data_root / "final_bnps" / "characters_maps_hr" / "characters.hlsbank"
		panoply_files_source = data_root / "final_bnps" / "characters_maps_hr" / "panoply_files.txt"

		try:
			base_rgba = dds_export.load_rgba(str(entry.fs_path))
			written = panoply_bake.bake_and_write(
				stem, base_rgba, axes, _mask_loader, workspace_dir / "tex", workspace_dir / "build",
				hlsbank_source, panoply_files_source, on_variant=on_variant,
			)
		except (OSError, RuntimeError) as exc:
			self._save_status = f"Bake failed for {base_texture_name}: {exc}"
			print(f"[object_editor] {self._save_status}")
			return []

		return written

	def _panoply_texture_has_mask(self, base_texture_name):
		"""Same "has at least one resolvable mask" check as
		_draw_panoply_masks_for()'s own mask_names list -- shared by
		_bake_panoply_real_all() below and that method's fire button."""
		stem = Path(base_texture_name).stem
		return any(self.search_paths_dialog.find_texture(f"{stem}_{axis}.tga") is not None for axis in panoply.AXES)

	def _bake_panoply_real_all(self):
		"""Starts a background bake (see _start_panoply_bake()) of every
		texture of the currently loaded shape that has at least one
		resolvable Panoply mask -- the "bake all" counterpart to the
		per-texture fire button in _draw_panoply_masks_for(), shown next to
		the gear/edit button in _draw_global_panoply_section() since baking
		is naturally a shape-wide action from the user's point of view, even
		though _bake_panoply_real() itself only ever handles one base
		texture at a time (each texture can have its own distinct set of
		masks)."""
		names = [name for name in self._shape_texture_names() if self._panoply_texture_has_mask(name)]
		self._start_panoply_bake(names)

	def _start_panoply_bake(self, texture_names):
		"""Starts a background thread that bakes every name in
		`texture_names` (in order) via _bake_panoply_real(), updating
		self._bake_progress as it goes so _draw_bake_progress_popup() can
		render live feedback (current texture/variant, an overall progress
		bar) -- baking is CPU-bound (numpy) and slow enough (~1s/variant on
		the real machine, 2026-08-29 cross-validation) that running it on
		the main thread would freeze the whole UI for however long the bake
		takes. Call from the main thread only: the checks below do imgui/
		request_settings_attention calls, which aren't safe off the main
		thread -- only _run_panoply_bake() (pure numpy/file I/O) actually
		runs in the background thread."""
		if self._bake_progress is not None and not self._bake_progress["done"]:
			return  # a bake is already running
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return
		if not repository_paths.is_valid("ryzom-data"):
			self.request_settings_attention("Paths", "ryzom-data")
			return
		if not texture_names:
			return

		self._bake_progress = {
			"texture_index": 0, "texture_total": len(texture_names), "texture_name": texture_names[0],
			"variant_suffix": "", "variant_index": 0, "variant_total": 0,
			"done": False, "error": None,
		}
		thread = threading.Thread(target=self._run_panoply_bake, args=(list(texture_names),), daemon=True)
		thread.start()

	def _run_panoply_bake(self, texture_names):
		"""Background-thread body for _start_panoply_bake() above -- never
		touches imgui or Settings-attention, only self._bake_progress (plain
		dict field writes, safe enough under the GIL for this simple
		main-thread-polls-it use, no lock needed) and _bake_panoply_real()'s
		own numpy/file I/O."""
		total_written = 0
		for index, name in enumerate(texture_names):
			progress = self._bake_progress
			progress["texture_index"] = index
			progress["texture_name"] = name
			progress["variant_suffix"] = ""
			progress["variant_index"] = 0
			progress["variant_total"] = 0

			def _on_variant(suffix, variant_index, variant_total, progress=progress):
				progress["variant_suffix"] = suffix
				progress["variant_index"] = variant_index
				progress["variant_total"] = variant_total

			written = self._bake_panoply_real(name, on_variant=_on_variant)
			total_written += len(written)

		self._bake_progress["done"] = True
		self._save_status = f"Baked {total_written} variant(s) across {len(texture_names)} texture(s) to tex/ + build/"
		print(f"[object_editor] {self._save_status}")

	def _draw_bake_progress_popup(self):
		"""Modal, opened/kept open for the whole lifetime of a background
		bake (see _start_panoply_bake()) -- shows the current texture/
		variant being baked and an overall progress bar, closes itself once
		done (or on error, after the user acknowledges it). Read-only view
		of self._bake_progress, which the background thread updates -- no
		imgui calls happen on that thread, only here on the main one."""
		progress = self._bake_progress
		if progress is None:
			return
		if not imgui.is_popup_open(_BAKE_PROGRESS_POPUP_ID):
			imgui.open_popup(_BAKE_PROGRESS_POPUP_ID)
		center_next_popup()
		flags = imgui.WindowFlags_.always_auto_resize.value
		opened, _ = imgui.begin_popup_modal(_BAKE_PROGRESS_POPUP_ID, None, flags)
		if not opened:
			return

		if progress["error"] is not None:
			imgui.text_colored((1.0, 0.4, 0.4, 1.0), f"Bake failed: {progress['error']}")
			if imgui.button("OK"):
				self._bake_progress = None
				imgui.close_current_popup()
			imgui.end_popup()
			return

		imgui.text(f"Texture {progress['texture_index'] + 1}/{progress['texture_total']}: {progress['texture_name']}")
		if progress["variant_total"]:
			imgui.text(f"Variant {progress['variant_index'] + 1}/{progress['variant_total']}: {progress['variant_suffix']}")
		else:
			imgui.text("Variant: (starting...)")
		texture_total = max(progress["texture_total"], 1)
		texture_fraction = progress["texture_index"] / texture_total
		variant_fraction = ((progress["variant_index"] + 1) / progress["variant_total"] / texture_total
		                     if progress["variant_total"] else 0.0)
		imgui.progress_bar(min(1.0, texture_fraction + variant_fraction), (300, 0))
		if progress["done"]:
			if imgui.button("OK"):
				self._bake_progress = None
				imgui.close_current_popup()
		imgui.end_popup()

	def _draw_panoply_masks_for(self, base_texture_name):
		"""Below a material's own texture row: thumbnails of whichever
		grayscale masks (mask weight in the red channel) already resolve for
		it -- e.g. for "tr_hom_armor00_epaule_c1.tga",
		"tr_hom_armor00_epaule_c1_skin.png"/"..._user.png" sitting in a
		sibling "mask/" folder next to the base texture, resolved through
		the same search-paths lookup as any other texture (so wherever the
		user's search paths actually cover that "mask/" subfolder) --
		regardless of whether panoply_files.txt happens to list this texture
		at all. A "+" button offers to create any axis (panoply.AXES) that
		doesn't have a mask yet, directly in the workspace -- the way a
		plain, not-yet-panoplied texture gets its first mask."""
		if not base_texture_name:
			return
		stem = Path(base_texture_name).stem
		mask_names = []
		missing_axes = []
		for axis in panoply.AXES:
			mask_name = f"{stem}_{axis}.tga"
			if self.search_paths_dialog.find_texture(mask_name) is not None:
				mask_names.append(mask_name)
			else:
				missing_axes.append(axis)
		imgui.text_disabled("Masks:")
		for mask_name in mask_names:
			imgui.same_line()
			imgui.push_id(mask_name)
			self._draw_texture_preview_static(mask_name, subdir="masks")
			imgui.same_line()
			# No material field to rewrite on copy (unlike a real texture
			# reference) -- a mask is only ever derived from the base
			# texture's own name + axis, never stored anywhere editable, so
			# on_copied is a no-op; the copy alone is enough for the next
			# frame's find_texture() to resolve to it (the active
			# workspace's own folders are already in the search paths).
			self._draw_texture_copy_button(mask_name, lambda new_name: None, subdir="masks")
			imgui.pop_id()

		if mask_names:
			imgui.same_line()
			disabled = self.workspace_setup_dialog.active_workspace_dir is None
			if _icon_button(fa_icons.ICON_FA_FIRE, "Bake real Panoply variants (writes tex/ + tex/hlsinfo/)", disabled=disabled):
				self._start_panoply_bake([base_texture_name])

		if missing_axes:
			imgui.same_line()
			imgui.push_id(f"add-mask-{stem}")
			disabled = self.workspace_setup_dialog.active_workspace_dir is None
			if _icon_button(fa_icons.ICON_FA_PLUS, "Add a mask for a missing axis", disabled=disabled):
				imgui.open_popup("add-mask-popup")
			if imgui.begin_popup("add-mask-popup"):
				for axis in missing_axes:
					clicked, _ = imgui.selectable(self._PANOPLY_AXIS_LABELS[axis], False)
					if clicked:
						self._create_panoply_mask(base_texture_name, axis)
						imgui.close_current_popup()
				imgui.end_popup()
			imgui.pop_id()
