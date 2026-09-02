"""ObjectEditorApp mixin: shared texture/color preview widgets -- thumbnail/
preview buttons, color swatches, the Multi Bitmap editor, the workspace
texture-name combo, copy-to-workspace helpers, and browse-dialog polling.
Called into by both the Materials/Textures tabs and the Panoply section
(not moved here -- those callers stay in object_editor.py/panoply_ui.py
until their own chantier steps). Split out of object_editor.py, see the
"Split object_editor.py into theme files" chantier in
project-todos/ryzom-core/forgery-object-editor.md.

Imports from object_editor_mixins.ui_helpers, NOT from object_editor.py
itself -- see ui_helpers.py's module docstring for why.
"""

import subprocess
from pathlib import Path

from imgui_bundle import icons_fontawesome_4 as fa_icons, imgui, portable_file_dialogs as pfd
from panda3d.core import PNMImage, Texture as PandaTexture

from pynel.ryzom_shape import Rgba

from ryzom_forgery import settings as app_settings
from ryzom_forgery.shape_geometry import (
	load_panda_texture, resolve_texture_ref, rgba_to_color, solid_color_texture, texture_to_pnm_image,
)
from ryzom_forgery.workspaces import reveal_in_system_file_manager
from ryzom_forgery.apps.object_editor_mixins.ui_helpers import (
	_icon_button, _IDRV_MAT_SHADER_SPECULAR, _MULTI_BITMAP_SLOT_LABELS, _STATUS_HINT_COLOR,
	_TEXTURE_IN_WORKSPACE_COLOR, _TEXTURE_NORMAL_COLOR,
)

_PREVIEW_BG_COLOR = (0.0, 0.0, 0.0, 1.0)
_COLOR_POPUP_ID = "material-color-picker"
_DIFFUSE_COLOR_POPUP_ID = "material-diffuse-picker"
# Same 3 extensions the texture browse dialog offers (_start_texture_browse()).
_WORKSPACE_TEXTURE_EXTENSIONS = {".tga", ".dds", ".png"}


def _multi_bitmap_slot_label(index):
	if 0 <= index < len(_MULTI_BITMAP_SLOT_LABELS):
		quality, ecosystem, season = _MULTI_BITMAP_SLOT_LABELS[index]
		labels = " / ".join(label for label in (quality, ecosystem, season) if label)
		return labels or str(index)
	return str(index)


class TextureWidgetsMixin:
	def _get_preview_texture_ref(self, name):
		"""Resolves a texture by name (like _apply_material does for
		rendering) into an imgui.ImTextureRef for UI thumbnails/tooltips,
		cached. Returns None if there's no name or it can't be decoded."""
		if not name:
			return None
		tex_ref = self._preview_texture_refs.get(name)
		if tex_ref is not None:
			return tex_ref
		panda_texture = load_panda_texture(
			name, cache=self._texture_cache, search_dirs=self._texture_search_dirs,
			repeat=self._texture_needs_repeat, finder=self.search_paths_dialog.find_texture)
		if panda_texture is None:
			return None
		# p3dimgui's loadTexture() mutates whatever Texture it's given
		# in-place (backend.py: texture.store(pnm); pnm.flip(...); texture.load(pnm)
		# -- flips it vertically and reloads it into the SAME object, for
		# ImGui's own OpenGL-vs-Panda row-order convention). `panda_texture`
		# here is the exact same cached object _apply_material_texture() put
		# on the live 3D material (self._texture_cache is shared) -- passing
		# it straight to loadTexture() would silently flip the 3D-rendered
		# texture too, the first time this shape's preview thumbnail/tooltip
		# is ever drawn. A throwaway copy absorbs the mutation instead.
		#
		# That copy is also where alpha gets dropped: most of these textures
		# are mostly transparent everywhere (e.g. a shape cut from a square
		# atlas), and alpha blending genuinely hides the real color underneath
		# low-alpha pixels -- no backdrop color can undo that (see
		# _draw_image_opaque_bg(), still used as a fallback for whatever's
		# left translucent). Safe to do on this copy specifically now that
		# it's confirmed independent of the live 3D material's own texture
		# (see above) -- doing this to the shared one would have made the
		# actual 3D-rendered object opaque too.
		#
		# Done via a PNMImage round-trip (store() -> remove_alpha() ->
		# load()), not Texture.set_format(F_rgb) directly: set_format() only
		# relabels the already-stored ram image's component count, it doesn't
		# actually repack the pixel bytes from 4 to 3 per pixel -- every pixel
		# after the first then reads shifted by one channel, which looked
		# exactly like a scrambled checkerboard for any texture that actually
		# had an alpha channel. store() decodes through the real pixel data
		# (works for a compressed source too), giving a real RGB buffer to
		# reload from.
		preview_texture = panda_texture.make_copy()
		pnm_image = PNMImage()
		preview_texture.store(pnm_image)
		if pnm_image.has_alpha():
			pnm_image.remove_alpha()
			preview_texture = PandaTexture()
			preview_texture.load(pnm_image)
		tex_ref = self.imgui.loadTexture(preview_texture)
		self._preview_texture_refs[name] = tex_ref
		return tex_ref

	@staticmethod
	def _draw_image_opaque_bg(tex_ref, size):
		"""imgui.image() (unlike image_button()) has no bg_col param -- draws
		an opaque black rect behind it by hand instead, so a texture with
		transparency (routinely most of the image, e.g. a banner cut out of a
		square atlas) doesn't blend into the panel's own dark background and
		become hard to actually see. Same reasoning as the bg_col passed to
		image_button() everywhere else in this file."""
		pos = imgui.get_cursor_screen_pos()
		width, height = size
		imgui.get_window_draw_list().add_rect_filled(
			pos, (pos.x + width, pos.y + height), imgui.get_color_u32(_PREVIEW_BG_COLOR))
		imgui.image(tex_ref, size)

	def _draw_thumbnail_button(self, str_id, tex_ref, tooltip, size=24):
		"""Common image_button + hover-zoom-tooltip widget: shared by real
		texture thumbnails and solid-color swatches, so a plain material
		color gets the exact same button chrome/hover style as a real
		texture instead of a hand-approximated look-alike. Both the thumbnail
		and the hover preview get an opaque white backdrop (see
		_draw_image_opaque_bg()) -- textures are routinely mostly transparent
		(e.g. a shape cut out of a square atlas), which otherwise blends into
		the panel and makes the preview hard to read for no benefit here."""
		clicked = imgui.image_button(str_id, tex_ref, (size, size), bg_col=_PREVIEW_BG_COLOR)
		if imgui.is_item_hovered():
			if imgui.begin_tooltip():
				self._draw_image_opaque_bg(tex_ref, (192, 192))
				if tooltip:
					imgui.text(tooltip)
				imgui.end_tooltip()
		return clicked

	def _draw_texture_preview_button(self, str_id, name, size=24):
		"""An image_button thumbnail for texture `name` (a plain color-button
		placeholder if there's no name or it can't be resolved), with a
		bigger preview shown in a tooltip on hover. Returns True if clicked."""
		tex_ref = self._get_preview_texture_ref(name)
		if tex_ref is None:
			return imgui.color_button(str_id, (0.5, 0.5, 0.5, 0.4), 0, (size, size))
		return self._draw_thumbnail_button(str_id, tex_ref, name, size)

	def _draw_texture_preview_static(self, name, size=24, badge=None, subdir="tex", str_id="##preview"):
		"""Same thumbnail button (size, frame, hover-zoom tooltip) as
		_draw_texture_preview_button, but a plain reveal-in-file-manager
		action instead of whatever a real texture-picker button would do
		(the text field/browse icon next to it are the actual editing
		controls for that) -- disabled only when there's no real file to
		reveal (unresolvable, or living inside a `.bnp`, which has no
		files of its own on disk). A plain imgui.image() looked visibly
		smaller: image_button adds the usual button frame padding around
		the image that a bare image() doesn't get, and disabling it
		(rather than swapping widget types) keeps that same size.

		`badge` (e.g. a material index), if given, is overlaid directly on
		the thumbnail -- see _draw_preview_badge() -- instead of a separate
		imgui.text() label, which would cost horizontal space this panel
		doesn't have to spare. `subdir` ("tex" or "masks", see
		_is_texture_in_workspace()) decides which workspace folder
		"already in the workspace" is checked against, for the green
		border (see _draw_preview_workspace_border()). `str_id` must be
		overridden by the caller whenever more than one of these can be
		drawn in the same push_id() scope (e.g. a material row's diffuse
		AND specular preview side by side) -- imgui.image_button()'s own ID
		is otherwise just this literal label text, identical every time."""
		tex_ref = self._get_preview_texture_ref(name)
		if tex_ref is None:
			imgui.dummy((size, size))
		else:
			resolved = self._resolve_texture(name)
			can_reveal = resolved is not None and resolved.fs_path is not None
			imgui.begin_disabled(not can_reveal)
			if imgui.image_button(str_id, tex_ref, (size, size), bg_col=_PREVIEW_BG_COLOR) and can_reveal:
				reveal_in_system_file_manager(resolved.fs_path)
			imgui.end_disabled()
			if self._is_texture_in_workspace(name, subdir):
				self._draw_preview_workspace_border()
			if imgui.is_item_hovered(imgui.HoveredFlags_.allow_when_disabled.value):
				if imgui.begin_tooltip():
					self._draw_image_opaque_bg(tex_ref, (192, 192))
					imgui.text(name)
					if can_reveal:
						imgui.text_disabled("Click to reveal in file manager")
					imgui.end_tooltip()
		if badge is not None:
			self._draw_preview_badge(badge)

	@staticmethod
	def _draw_preview_badge(text, scale=0.7):
		"""Overlays `text` (e.g. "3") in the bottom-right corner of
		whichever preview button/thumbnail/color-swatch was just drawn, at
		`scale`x the normal font size -- takes no extra horizontal layout
		space, unlike a separate imgui.text() next to it, which is the
		point: these rows are already tight on width. A 1px black shadow
		keeps it legible over a bright/busy thumbnail.

		add_text()'s explicit-size overload draws at any size regardless
		of the currently pushed font scale, but calc_text_size() always
		measures at the *current* (full) size -- so the small size's own
		footprint is estimated by scaling that measurement down, rather
		than actually re-measuring at `scale` (glyph metrics scale
		linearly with size in ImGui's font atlas, so this is exact, not
		an approximation)."""
		font = imgui.get_font()
		full_size = imgui.get_font_size()
		small_size = full_size * scale
		full_text_size = imgui.calc_text_size(text)
		text_w, text_h = full_text_size.x * scale, full_text_size.y * scale

		item_max = imgui.get_item_rect_max()
		pos = (item_max.x - text_w - 1, item_max.y - text_h - 1)
		shadow_pos = (pos[0] + 1, pos[1] + 1)
		draw_list = imgui.get_window_draw_list()
		draw_list.add_text(font, small_size, shadow_pos, imgui.get_color_u32((0.0, 0.0, 0.0, 1.0)), text)
		draw_list.add_text(font, small_size, pos, imgui.get_color_u32((1.0, 1.0, 1.0, 1.0)), text)

	@staticmethod
	def _draw_preview_workspace_border(thickness=2):
		"""Outlines whichever preview button/thumbnail was just drawn in
		_TEXTURE_IN_WORKSPACE_COLOR -- same green already used for a texture
		reference's own text field (see _draw_simple_material_row()/
		_draw_multi_bitmap_editor()), now also on the thumbnail itself.
		Manually drawn (not a pushed Col_.border style color): image_button()
		only actually shows a border when the current style's frame border
		size is non-zero, which isn't guaranteed."""
		item_min = imgui.get_item_rect_min()
		item_max = imgui.get_item_rect_max()
		color = imgui.get_color_u32(_TEXTURE_IN_WORKSPACE_COLOR)
		imgui.get_window_draw_list().add_rect(item_min, item_max, color, thickness=thickness)

	def _get_color_texture_ref(self, color):
		"""A 1x1 solid-color Texture (see shape_geometry.solid_color_texture)
		wrapped as an imgui.ImTextureRef, cached by color."""
		key = tuple(round(c, 3) for c in color)
		tex_ref = self._color_texture_refs.get(key)
		if tex_ref is not None:
			return tex_ref
		tex_ref = self.imgui.loadTexture(solid_color_texture(color))
		self._color_texture_refs[key] = tex_ref
		return tex_ref

	def _draw_color_preview_button(self, str_id, color, size=24):
		"""Same image_button widget as _draw_texture_preview_button, but for
		a plain color (e.g. an untextured material's own diffuse color) --
		filled into a tiny solid-color texture rather than hand-drawing a
		look-alike border/hover style around a plain color_button."""
		tex_ref = self._get_color_texture_ref(color)
		tooltip = f"RGBA {tuple(round(c, 2) for c in color)}"
		return self._draw_thumbnail_button(str_id, tex_ref, tooltip, size)

	def _draw_material_color_button(self, material_id, preview_name=...):
		"""A material's color-override button: a flat color swatch once an
		override is set, otherwise a texture preview thumbnail (see
		_draw_texture_preview_button). Either way, clicking it opens the same
		picker (plus a "No color" option) that sets/clears the override,
		visualizing on the 3D model exactly which faces use this material.

		`preview_name`, if given, overrides which texture the preview shows
		-- for a Multi Bitmap slot row, that's the slot's own texture, not
		necessarily the material's currently active one (the default,
		looked up from slot 0 when `preview_name` is left as the sentinel)."""
		override_color = self._material_override_colors.get(material_id)
		if override_color is not None:
			# A forced/manual color: plain color_button, deliberately not the
			# same image_button look as real shape data (texture or diffuse
			# color) below -- that difference in chrome is what marks it as
			# an override at a glance.
			clicked = imgui.color_button(f"##color-{material_id}", override_color, 0, (24, 24))
		else:
			materials = getattr(self.shape_file.value, "materials", None)
			material = materials[material_id] if materials and material_id < len(materials) else None
			if preview_name is ...:
				texture = material.textures[0] if material is not None and material.textures else None
				preview_name = texture.file_name if texture is not None else None
				if preview_name:
					preview_name = self._resolve_panoply_texture_name(preview_name)
			if preview_name:
				clicked = self._draw_texture_preview_button(f"##color-{material_id}", preview_name)
			else:
				# No texture at all -- this material is just a flat color
				# (e.g. anlor_stick.shape's untextured wood/metal parts), so
				# show its own diffuse color rather than a generic placeholder.
				diffuse = rgba_to_color(material.diffuse) if material is not None else (0.5, 0.5, 0.5, 0.4)
				clicked = self._draw_color_preview_button(f"##color-{material_id}", diffuse)
		if clicked:
			imgui.open_popup(f"{_COLOR_POPUP_ID}-{material_id}")

		if imgui.begin_popup(f"{_COLOR_POPUP_ID}-{material_id}"):
			if imgui.button("No color"):
				self._set_material_override_color(material_id, None)
				imgui.close_current_popup()
			imgui.separator()
			current = override_color if override_color is not None else (1.0, 1.0, 1.0, 1.0)
			changed, new_color = imgui.color_picker4(f"##picker-{material_id}", current)
			if changed:
				self._set_material_override_color(material_id, new_color)
			imgui.end_popup()

	@staticmethod
	def _set_material_diffuse_color(material, color):
		r, g, b, a = color.x, color.y, color.z, color.w
		material.diffuse = Rgba(round(r * 255), round(g * 255), round(b * 255), round(a * 255))

	def _draw_texture_color_button(self, material_id, material):
		"""Textures tab only: when a material has no texture at all, its
		swatch button lets you edit CMaterial::diffuse directly -- a real,
		saved modification of the shape (unlike the Materials tab's color
		button, which is a temporary visualization override, never saved).
		Once a texture is set, this just previews it: the material is a
		texture now, not a color, so it's no longer clickable for color."""
		badge = str(material_id)
		texture = material.textures[0] if material.textures else None
		if texture is not None and texture.file_name:
			self._draw_texture_preview_static(texture.file_name, badge=badge)
			return

		diffuse = rgba_to_color(material.diffuse)
		if self._draw_color_preview_button(f"##diffuse-{material_id}", diffuse):
			imgui.open_popup(f"{_DIFFUSE_COLOR_POPUP_ID}-{material_id}")
		self._draw_preview_badge(badge)

		if imgui.begin_popup(f"{_DIFFUSE_COLOR_POPUP_ID}-{material_id}"):
			changed, new_color = imgui.color_picker4(f"##diffuse-picker-{material_id}", diffuse)
			if changed:
				self._set_material_diffuse_color(material, new_color)
				self._reapply_material(material_id)
			imgui.end_popup()

	def _start_texture_browse(self, key, on_result):
		"""Opens a native file picker for a texture; on_result(file_name) is
		called with just the chosen file's base name once picked, matching
		how texture references are stored (name only, no path). Starts
		wherever the shape itself came from, if known -- a texture is far
		more likely to sit next to (or under) it than anywhere else."""
		start_dir = str(self._shape_source_path.parent) if self._shape_source_path is not None else ""
		dialog = pfd.open_file("Choose texture", start_dir, ["Textures", "*.tga *.dds *.png"])
		self._texture_browse_dialogs[key] = (dialog, on_result)

	def _poll_texture_browse_dialogs(self):
		for key in list(self._texture_browse_dialogs.keys()):
			dialog, on_result = self._texture_browse_dialogs[key]
			if not dialog.ready(0):
				continue
			del self._texture_browse_dialogs[key]
			result = dialog.result()
			if result:
				# Texture names aren't case-sensitive; lower-cased consistently
				# rather than mixing whatever case a file happens to be on disk.
				on_result(Path(result[0]).name.lower())

	def _multi_bitmap_entries(self):
		"""(material_id, texture) for every material whose slot-0 texture is a
		CTextureMultiFile (see docs/material_options.md, "Plusieurs images
		alternatives par emplacement (Multi Bitmap)"). Only slot 0 is covered
		since it's the only slot _apply_material actually renders today."""
		materials = getattr(self.shape_file.value, "materials", None)
		if not materials:
			return []
		entries = []
		for material_id, material in enumerate(materials):
			texture = material.textures[0] if material.textures else None
			if texture is not None and texture.class_name == "CTextureMultiFile" and texture.file_names:
				entries.append((material_id, texture))
		return entries

	@staticmethod
	def _ensure_multi_bitmap_slot(texture, index):
		"""Pads texture.file_names up to `index` with "" if it's shorter --
		matches how the 3dsMax exporter itself leaves gaps for skipped slots
		(see e.g. xmas_tr_mektoub_selle.shape's slot 3, an empty string)."""
		while len(texture.file_names) <= index:
			texture.file_names.append("")

	def _select_multi_bitmap_slot(self, entries, index):
		"""The "Select" button: switches every Multi Bitmap material of the
		shape to the same slot index at once -- picking "Medium Quality" is a
		whole-object appearance choice, not a per-material one. Also syncs
		stage 1's specular map when the material is in Specular mode and
		stage 1 is itself multi-file (real content: a CTextureCube whose 6
		faces are each their own CTextureMultiFile, tracking the same
		per-slot dye choice as stage 0, e.g. nospec/spec_base/spec_luxe) --
		without this, switching slots only ever changed the diffuse texture,
		leaving the specular cubemap permanently stuck on whichever variant
		happened to be selected when the shape was authored (e.g. index 0 /
		nospec.png, which shows no specular at all regardless of slot)."""
		materials = self.shape_file.value.materials
		for material_id, texture in entries:
			self._ensure_multi_bitmap_slot(texture, index)
			texture.selected_index = index
			texture.file_name = texture.file_names[index]

			material = materials[material_id]
			if (material.shader_type == _IDRV_MAT_SHADER_SPECULAR and len(material.textures) > 1
					and material.textures[1] is not None):
				stage1 = material.textures[1]
				faces = stage1.sub_textures if stage1.class_name == "CTextureCube" else [stage1]
				for face in faces or []:
					if face is not None and face.file_names:
						self._ensure_multi_bitmap_slot(face, index)
						face.selected_index = index
						face.file_name = face.file_names[index]

			self._reapply_material(material_id)

	def _draw_multi_bitmap_editor(self):
		entries = self._multi_bitmap_entries()
		if not entries:
			return
		entries_by_id = dict(entries)

		imgui.separator()
		imgui.text("Multi Bitmap")
		doc = self.material_docs.get("multi-bitmap")
		hovered_hint = doc.summary if (doc and imgui.is_item_hovered()) else None

		representative = entries_by_id.get(0, entries[0][1])
		slot_count = max(len(_MULTI_BITMAP_SLOT_LABELS), max(len(t.file_names) for _, t in entries))

		for index in range(slot_count):
			imgui.push_id(f"mb-slot-{index}")

			is_active = representative.selected_index == index
			if is_active:
				imgui.push_style_color(imgui.Col_.button.value, (0.2, 0.65, 0.2, 1.0))
				imgui.push_style_color(imgui.Col_.button_hovered.value, (0.25, 0.7, 0.25, 1.0))
				imgui.push_style_color(imgui.Col_.button_active.value, (0.15, 0.55, 0.15, 1.0))
			if _icon_button(fa_icons.ICON_FA_CHECK, "Select this set for the whole shape"):
				self._select_multi_bitmap_slot(entries, index)
			if is_active:
				imgui.pop_style_color(3)
			imgui.same_line()

			expanded = index in self._multi_bitmap_expanded
			expand_icon = fa_icons.ICON_FA_CHEVRON_DOWN if expanded else fa_icons.ICON_FA_CHEVRON_RIGHT
			if _icon_button(expand_icon, "Collapse" if expanded else "Expand: edit per-material"):
				if expanded:
					self._multi_bitmap_expanded.discard(index)
				else:
					self._multi_bitmap_expanded.add(index)
			imgui.same_line()

			imgui.text(_multi_bitmap_slot_label(index))
			if doc and imgui.is_item_hovered():
				hovered_hint = doc.summary

			preview = representative.file_names[index] if index < len(representative.file_names) else ""
			imgui.text_disabled(preview or "(empty)")

			if expanded:
				for material_id, texture in entries:
					self._ensure_multi_bitmap_slot(texture, index)
					imgui.push_id(f"mb-mat-{material_id}")

					slot_name = texture.file_names[index]
					badge = str(material_id)
					if slot_name:
						self._draw_texture_preview_static(slot_name, badge=badge)
					else:
						imgui.dummy((24, 24))
						self._draw_preview_badge(badge)
					imgui.same_line()

					def _set_slot(value, material_id=material_id, texture=texture, index=index):
						texture.file_names[index] = value
						if texture.selected_index == index:
							texture.file_name = value
							self._reapply_material(material_id)

					current_value = texture.file_names[index]
					in_workspace = self._is_texture_in_workspace(current_value)
					imgui.push_style_color(
						imgui.Col_.text.value, _TEXTURE_IN_WORKSPACE_COLOR if in_workspace else _TEXTURE_NORMAL_COLOR)
					changed, new_value = self._draw_texture_name_combo(f"##combo-{index}", current_value)
					imgui.pop_style_color()
					if imgui.is_item_hovered() and current_value:
						source_path = self._texture_source_path_text(current_value)
						tooltip = source_path or current_value
						if in_workspace:
							tooltip += "\n(in the active workspace)"
						imgui.set_tooltip(tooltip)
					if changed and new_value != current_value:
						_set_slot(new_value)
					imgui.same_line()
					if _icon_button(fa_icons.ICON_FA_FOLDER_OPEN, "Browse for a texture file"):
						def _on_result(file_name, texture=texture, index=index, _set_slot=_set_slot):
							self._ensure_multi_bitmap_slot(texture, index)
							_set_slot(file_name)
						self._start_texture_browse(("multi-bitmap", material_id, index), _on_result)
					imgui.same_line()
					self._draw_texture_copy_button(current_value, _set_slot)

					material = self.shape_file.value.materials[material_id]
					if (material.shader_type == _IDRV_MAT_SHADER_SPECULAR and len(material.textures) > 1
							and material.textures[1] is not None):
						def _set_specular_slot(value, material=material, index=index):
							stage1 = material.textures[1]
							faces = stage1.sub_textures if stage1.class_name == "CTextureCube" else [stage1]
							should_reapply = False
							for face in faces or []:
								if face is None:
									continue
								while len(face.file_names) <= index:
									face.file_names.append("")
								face.file_names[index] = value
								if face.selected_index == index:
									face.file_name = value
									should_reapply = True
							if should_reapply:
								self._reapply_material(material_id)

						current_specular = self._specular_preview_name_for_slot(material, index) or ""
						self._draw_texture_preview_static(current_specular, badge="S", str_id="##preview-specular")
						imgui.same_line()
						specular_in_workspace = self._is_texture_in_workspace(current_specular)
						imgui.push_style_color(
							imgui.Col_.text.value,
							_TEXTURE_IN_WORKSPACE_COLOR if specular_in_workspace else _TEXTURE_NORMAL_COLOR)
						changed, new_value = self._draw_texture_name_combo(f"##specular-combo-{index}", current_specular)
						imgui.pop_style_color()
						if imgui.is_item_hovered() and current_specular:
							source_path = self._texture_source_path_text(current_specular)
							tooltip = source_path or current_specular
							if specular_in_workspace:
								tooltip += "\n(in the active workspace)"
							imgui.set_tooltip(tooltip)
						if changed and new_value != current_specular:
							_set_specular_slot(new_value)
						imgui.same_line()
						if _icon_button(f"{fa_icons.ICON_FA_FOLDER_OPEN}##specular", "Browse for a specular/gloss map file"):
							def _on_specular_result(file_name, _set_specular_slot=_set_specular_slot):
								_set_specular_slot(file_name)
							self._start_texture_browse(("specular-multi", material_id, index), _on_specular_result)
						imgui.same_line()
						self._draw_texture_copy_button(current_specular, _set_specular_slot, str_id_suffix="##specular")

					imgui.pop_id()

			imgui.pop_id()
			imgui.separator()

		if hovered_hint:
			self.sysinfo.set_status(hovered_hint, color=_STATUS_HINT_COLOR)
			self._multi_bitmap_hint_shown = True
		elif self._multi_bitmap_hint_shown:
			self.sysinfo.set_status("")
			self._multi_bitmap_hint_shown = False

	def _texture_source_path_text(self, file_name):
		"""Human-readable full path of whatever file `file_name` actually
		resolves to (a plain path, or `<bnp path> (<entry name>)` for a
		texture living inside a .bnp) -- see the texture combo's tooltip
		(_draw_texture_name_combo() callers), so hovering it shows exactly
		which source file it takes rather than just the bare name already
		visible in the combo itself. None if it doesn't resolve to anything."""
		ref = self._resolve_texture(file_name)
		if ref is None:
			return None
		if ref.fs_path is not None:
			return str(ref.fs_path)
		if ref.bnp_path is not None:
			return f"{ref.bnp_path} ({ref.name})"
		return None

	def _resolve_texture(self, file_name):
		"""resolve_texture_ref(), pre-bound to this shape's own
		_texture_search_dirs/finder -- the same lookup the viewport itself
		uses to display a texture."""
		if not file_name:
			return None
		return resolve_texture_ref(file_name, self._texture_search_dirs, self.search_paths_dialog.find_texture)

	def _workspace_texture_names(self, subdir="tex"):
		"""Sorted list of texture file names sitting in the active
		workspace's `subdir` folder (default "tex") -- rescanned fresh on
		every call (a single Path.iterdir(), cheap enough to just always
		redo rather than cache/invalidate) so _draw_texture_name_combo()
		always shows what's actually on disk the moment its dropdown opens."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return []
		try:
			entries = list((workspace_dir / subdir).iterdir())
		except OSError:
			return []
		return sorted(entry.name for entry in entries if entry.suffix.lower() in _WORKSPACE_TEXTURE_EXTENSIONS)

	def _draw_texture_name_combo(self, imgui_id, current_value):
		"""Combo box listing the active workspace's tex/ textures (see
		_workspace_texture_names()) -- returns (changed, new_value), same
		shape as imgui.input_text()'s return value, so call sites only
		needed to swap the widget itself. current_value is always shown as
		the combo's own preview text even when it isn't one of the listed
		names (not yet copied into the workspace, or set via Browse from
		somewhere else) -- picking a listed entry is the only way this
		widget itself changes it; Browse/Copy stay the way to set anything
		else, same buttons as before, unchanged."""
		imgui.set_next_item_width(220)
		changed = False
		new_value = current_value
		if imgui.begin_combo(imgui_id, current_value):
			# Empty entry first -- the only way to clear a slot from this
			# combo alone (Browse only ever sets a real file; nothing here
			# could remove one without this).
			clicked, _ = imgui.selectable("(none)", current_value == "")
			if clicked:
				new_value = ""
				changed = True
			for name in self._workspace_texture_names():
				clicked, _ = imgui.selectable(name, name == current_value)
				if clicked:
					new_value = name
					changed = True
			imgui.end_combo()
		return changed, new_value

	def _texture_copy_destination(self, ref, subdir="tex"):
		"""<active workspace>/<subdir>/<ref's own bare name>, or None without
		an active workspace configured. `subdir` defaults to "tex" (a real
		material texture); pass "masks" for a Panoply mask file instead (see
		_draw_panoply_masks_for()). A `.dds` source always lands as `.png`
		instead (see _copy_texture_to_workspace()) -- `.dds` is a compiled,
		non-editable format (also auto-(re)generated into dds/ from tex/ by
		Patina itself, see tex_dds_sync.py), never something the workspace's
		own editable folders should hold."""
		workspace_dir = self.workspace_setup_dialog.active_workspace_dir
		if workspace_dir is None:
			return None
		name = ref.name
		if Path(name).suffix.lower() == ".dds":
			name = f"{Path(name).stem}.png"
		return workspace_dir / subdir / name

	def _is_texture_in_workspace(self, file_name, subdir="tex"):
		"""True if `file_name` currently resolves to a file already sitting
		in the active workspace's own `subdir` folder -- used to color-code
		texture references in the UI (see _draw_texture_copy_button())."""
		ref = self._resolve_texture(file_name)
		if ref is None or ref.fs_path is None:
			return False
		dest = self._texture_copy_destination(ref, subdir)
		return dest is not None and ref.fs_path.resolve() == dest.resolve()

	def _copy_texture_to_workspace(self, file_name, subdir="tex"):
		"""Resolves `file_name` (absolute path or bare name alike) and
		copies it into the active workspace's `subdir` folder, unless it's
		already sitting there. A `.dds` source is decoded and written out as
		`.png` instead of a raw byte copy (see _texture_copy_destination()
		-- `.dds` is never an editable workspace source). Returns the
		resulting bare (lowercased, matching how a typed/browsed texture
		name is normalized elsewhere) file name to store back on the
		material, or None if nothing could be done -- no active workspace,
		unresolvable/undecodable reference, or a write failure (logged, not
		raised; the caller just leaves the material's current reference
		untouched in that case)."""
		ref = self._resolve_texture(file_name)
		if ref is None:
			print(f"[object_editor] could not resolve texture {file_name!r} to copy")
			return None
		dest = self._texture_copy_destination(ref, subdir)
		if dest is None:
			return None
		if not (ref.fs_path is not None and ref.fs_path.resolve() == dest.resolve()):
			try:
				dest.parent.mkdir(parents=True, exist_ok=True)
				if Path(ref.name).suffix.lower() == ".dds":
					panda_texture = load_panda_texture(ref.name, finder=lambda _name, _ref=ref: _ref)
					if panda_texture is None:
						print(f"[object_editor] could not decode {ref.name!r} to convert to .png")
						return None
					texture_to_pnm_image(panda_texture).write(str(dest))
				else:
					dest.write_bytes(ref.read_bytes())
			except OSError as exc:
				print(f"[object_editor] could not copy texture {ref.name!r} to workspace: {exc}")
				return None
		return dest.name.lower()

	def _draw_texture_copy_button(self, current_value, on_copied, subdir="tex", str_id_suffix=""):
		"""Small icon button, next to a texture reference field: copies
		that one texture into the active workspace's `subdir` folder and
		rewrites the reference to the resulting bare name via
		`on_copied(new_name)` -- deliberately opt-in per texture rather
		than an automatic copy-everything-on-save, so the workspace's
		tex/ (or masks/) only ever holds files the user actually chose to
		bring in for editing. Once it's already there, this becomes an
		"Edit" button instead (see _draw_texture_edit_button()) -- copying
		it again would be a no-op, so there's nothing useful left for this
		button to do besides launch an editor on it. `str_id_suffix` must
		be overridden by the caller whenever more than one of these can be
		drawn in the same push_id() scope (e.g. a material row's diffuse
		AND specular texture side by side) -- the icon glyph alone is
		otherwise the same imgui ID every time."""
		if self._is_texture_in_workspace(current_value, subdir):
			self._draw_texture_edit_button(current_value, str_id_suffix=str_id_suffix)
			return
		disabled = not current_value or self.workspace_setup_dialog.active_workspace_dir is None
		imgui.begin_disabled(disabled)
		if _icon_button(f"{fa_icons.ICON_FA_DOWNLOAD}{str_id_suffix}", f"Copy this file into the active workspace's {subdir}/"):
			new_name = self._copy_texture_to_workspace(current_value, subdir)
			if new_name is not None:
				on_copied(new_name)
		imgui.end_disabled()

	def _draw_texture_edit_button(self, current_value, str_id_suffix=""):
		"""Launches the user's configured external image editor (Settings
		tab -> Tools) on `current_value`'s resolved file -- shown instead
		of the copy button once a texture already lives in the active
		workspace. Always clickable (2026-08-29): if no editor is
		configured yet, jumps to Settings > Tools and flashes the field
		instead of just sitting disabled with a tooltip (see
		ForgeryApp.request_settings_attention()). `str_id_suffix`: see
		_draw_texture_copy_button()'s own docstring."""
		editor_path = app_settings.load().image_editor_path
		if _icon_button(f"{fa_icons.ICON_FA_EDIT}{str_id_suffix}", "Edit this texture in the configured image editor"):
			if not editor_path:
				self.request_settings_attention("Tools", "image_editor_path")
			else:
				resolved = self._resolve_texture(current_value)
				if resolved is not None and resolved.fs_path is not None:
					subprocess.Popen([editor_path, str(resolved.fs_path)])
