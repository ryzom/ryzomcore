"""Generic ImGui button helpers and shared constants used across
ObjectEditorApp and its object_editor_mixins/ mixins.

Deliberately has ZERO dependency on object_editor.py itself: object_editor.py
can be loaded under two different sys.modules identities depending on launch
method (`python -m ryzom_forgery.apps.object_editor` registers it as
`__main__`, while the app-discovery code in ryzom_forgery/__init__.py imports
it as `ryzom_forgery.apps.object_editor` -- two separate module objects for
the same file). A mixin importing something back from object_editor.py hits
a circular-import ImportError when this happens, because the second,
package-qualified import re-executes object_editor.py from scratch, re-enters
its own mixin import before the mixin class is fully defined. Keeping these
helpers here, with mixins and object_editor.py both importing FROM this
module (never from each other), avoids the cycle entirely.
"""

from imgui_bundle import imgui

from ryzom_forgery.icon_colors import darken, pastel_color_for

_ACTIVE_COLOR = (0.7, 0.7, 0.7, 1.0)  # light gray -- default "on" highlight, matches the button family's own gray scale (app.py) instead of clashing with it as ImGui's stock blue used to

_VIEWPORT_TOGGLE_MARGIN_PX = 10
_OBJECT_TRANSPARENCY_ALPHA = 0.5

# Shape types pynel's save_shape() can actually write back out -- matches
# ryzom_shape.py's _SHAPE_CLASS_NAMES, the Save UI only shows for these.
_WRITABLE_SHAPE_TYPES = {"Mesh", "MeshMRM", "MeshMRMSkinned", "MeshMultiLod"}

_SYNC_NOW_COLOR = (0.85, 0.55, 0.15, 1.0)  # orange -- _draw_workspace_sync_settings()'s catch-up button

# _draw_bottom_bar()'s Save/Export/Quit buttons -- blue for the export
# buttons (not pink, to stay visually distinct from Quit's pink).
_SAVE_BUTTON_COLOR = (0.6, 0.85, 0.65, 1.0)  # pastel green
_EXPORT_AS_BUTTON_COLOR = (0.65, 0.8, 0.95, 1.0)  # light blue
_QUICK_EXPORT_BUTTON_COLOR = (0.4, 0.65, 0.9, 1.0)  # blue -- workspace-only quick export
_QUIT_BUTTON_COLOR = (0.9, 0.55, 0.7, 1.0)  # pink

# Shared convention for every strictly binary Oui/Non confirmation popup
# (e.g. _draw_load_shape_unsaved_popup(), _draw_replace_match_popup()) --
# lightgreen for "confirm", pink for "cancel".
_CONFIRM_YES_COLOR = (0.565, 0.933, 0.565, 1.0)  # lightgreen
_CONFIRM_NO_COLOR = (1.0, 0.753, 0.796, 1.0)  # pink

_STATUS_HINT_COLOR = (1.0, 0.6, 0.15, 1.0)  # orange, for material_options.md hints shown in the status bar
_TEXTURE_NORMAL_COLOR = (1.0, 1.0, 1.0, 1.0)
_TEXTURE_IN_WORKSPACE_COLOR = (0.4, 1.0, 0.4, 1.0)  # green -- this texture reference already lives in the active workspace's tex/

# CMaterial::TShader (material.h) -- Material.shader_type's value, not a flag
# bit. Specular: stage 0 is the diffuse texture, stage 1 is a dedicated
# specular/gloss map (flat texture or CTextureCube) whose color is added to
# stage 0's, modulated by stage 0's own alpha (driver_opengl_material.cpp's
# setupSpecularPass(): result = Tex0.rgb + Tex0.alpha * Tex1.rgb).
_IDRV_MAT_SHADER_SPECULAR = 4

# Multi Bitmap slot index -> (quality label, ecosystem label, season label),
# the three known Georges/engine conventions documented in
# docs/material_options.md (item_map.typ's map_variant, _creature_texture.typ,
# and EGSPD::CSeason in ryzom/common/src/game_share/season.h) -- which one
# actually applies depends on the shape, so all three are always shown together.
_MULTI_BITMAP_SLOT_LABELS = [
	("Low Quality", "Forest", "Spring"),  # "Forest": labelled "none" in _creature_texture.typ, but means Forest in practice
	("Medium Quality", "Lacustre", "Summer"),
	("High Quality", "Desert", "Autumn"),
	("Super Quality", "Jungle", "Winter"),
	("XL Quality", "Primr", None),
	("Suprem Quality", "goo", None),
	("Divine Quality", None, None),
	("Obiwan Quality", None, None),
]


def _icon_button(icon, tooltip, active=False, square=False, large_font=None, active_color=_ACTIVE_COLOR, disabled=False):
	"""An icon-only button (Font Awesome glyph, see ryzom_forgery.app's
	_load_icon_font) with a hover tooltip, since an icon alone isn't always
	self-explanatory. `active` highlights it (toggle-button style, in
	`active_color`) when the feature it controls is currently on. `square`
	sizes it to the current font's frame height on both axes, so a row of
	these all matches -- without it, imgui.button()'s default auto-size
	makes each button only as wide as its own glyph, which visibly varies
	between Font Awesome icons. `large_font`, if given, is an (ImFont, size)
	pair (app.large_icon_font, app.large_icon_font_size) pushed around just
	the button glyph itself -- NOT the tooltip below, which needs the normal
	text font: that font is icon-glyphs-only, so a tooltip drawn under it
	renders as blank/invisible text instead of readable words. The glyph
	itself is tinted a deterministic pastel color (see icon_colors.py) --
	independent of `active`'s own button-background highlight, both can be
	on at once (confirmed with Nuno, 2026-09-03: no conflict between the
	two signals). While `active`, that pastel is darkened first -- the
	light `active_color` background (gray by default, see _ACTIVE_COLOR)
	would otherwise wash out a normal light pastel glyph (Nuno, 2026-09-03)."""
	icon_color = pastel_color_for(icon)
	if active:
		imgui.push_style_color(imgui.Col_.button.value, active_color)
		icon_color = darken(icon_color)
	imgui.push_style_color(imgui.Col_.text.value, icon_color)
	if large_font is not None:
		imgui.push_font(*large_font)
	imgui.begin_disabled(disabled)
	size = (imgui.get_frame_height(), imgui.get_frame_height()) if square else (0, 0)
	clicked = imgui.button(icon, size)
	imgui.end_disabled()
	if large_font is not None:
		imgui.pop_font()
	imgui.pop_style_color()
	if active:
		imgui.pop_style_color()
	# allow_when_disabled: ImGui's default IsItemHovered() reports False
	# for a disabled item, so without this a disabled icon button (e.g.
	# _draw_panel_taskbar()'s Wind/Skinning/Bind entries when not
	# applicable to the loaded shape) never shows its tooltip -- exactly
	# when a user hovering a grayed-out button most wants to know why
	# (found by Nuno, 2026-09-03).
	if imgui.is_item_hovered(imgui.HoveredFlags_.allow_when_disabled.value):
		imgui.set_tooltip(tooltip)
	return clicked


def _colored_button(label, color):
	"""A plain text button (unlike _icon_button above) tinted `color`, with
	lighter/darker hover/active variants derived from it -- see
	_draw_import_conflict_popup()'s 4 choices, color-coded by how "safe"
	each one is."""
	r, g, b, a = color
	imgui.push_style_color(imgui.Col_.button.value, color)
	imgui.push_style_color(imgui.Col_.button_hovered.value, (min(r + 0.1, 1.0), min(g + 0.1, 1.0), min(b + 0.1, 1.0), a))
	imgui.push_style_color(imgui.Col_.button_active.value, (max(r - 0.1, 0.0), max(g - 0.1, 0.0), max(b - 0.1, 0.0), a))
	imgui.push_style_color(imgui.Col_.text.value, (0.0, 0.0, 0.0, 1.0))
	clicked = imgui.button(label)
	imgui.pop_style_color(4)
	return clicked
