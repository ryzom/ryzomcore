"""Deterministic pastel color per icon glyph -- shared by every _icon_button()
copy in this codebase (see e.g. apps/object_editor_mixins/ui_helpers.py's own:
each module keeps its own tiny _icon_button rather than sharing one -- see
that pattern's own docstring note -- but they all pick their icon's color
through this one shared, pure lookup so the same icon glyph always gets the
same color everywhere, not a fresh random pick per launch (confirmed with
Nuno, 2026-09-03: seeded by the icon itself, not truly random).

Deliberately NOT applied to navcube.py's own gizmo buttons (status/mode
icons, direction pad, roll) -- those already use color meaningfully (orange
= currently forced/active, see navcube.py's own _ACTIVE_COLOR/_INACTIVE_COLOR),
so tinting them pastel too would make that existing signal harder to read.
"""

import hashlib

# Readable at normal icon size against both the app's dark viewport
# overlays and the light Settings-tab background -- and now that
# Col_.button/hovered/active are neutral gray (see app.py, was ImGui's own
# default blue) rather than a competing hue, back to this original, softer
# HSL(hue, ~40%, ~68%) pastel rendering (Nuno, 2026-09-03: "on aurait du
# faire ça des le debut" re: fixing the button background instead of
# picking louder icon colors -- see the 65%/60% version's own history in
# git log for the more saturated alternative this replaced). Hues taken
# from Nuno's given 6 (an Everforest-style accent set) + 6 filled into the
# remaining gaps around the hue wheel (roughly 20=orange, 62=olive,
# 205=sky, 245=indigo, 285=purple, 350=coral).
_PASTEL_PALETTE = (
	(0.902, 0.494, 0.502, 1.0),  # red      #e67e80
	(0.655, 0.753, 0.502, 1.0),  # green    #a7c080
	(0.859, 0.737, 0.498, 1.0),  # yellow   #dbbc7f
	(0.498, 0.733, 0.702, 1.0),  # blue     #7fbbb3
	(0.839, 0.600, 0.714, 1.0),  # magenta  #d699b6
	(0.514, 0.753, 0.573, 1.0),  # cyan     #83c092
	(0.808, 0.639, 0.553, 1.0),  # orange   #cea38d
	(0.800, 0.808, 0.553, 1.0),  # olive    #ccce8d
	(0.553, 0.702, 0.808, 1.0),  # sky      #8db3ce
	(0.573, 0.553, 0.808, 1.0),  # indigo   #928dce
	(0.745, 0.553, 0.808, 1.0),  # purple   #be8dce
	(0.808, 0.553, 0.596, 1.0),  # coral    #ce8d98
)


def darken(color, factor=0.5):
	"""`color` scaled toward black by `factor` (alpha untouched) -- for an
	icon glyph drawn over a light "active" button background (see
	ui_helpers.py's _icon_button()), where the normal pastel tint (picked
	for contrast against dark backgrounds) would otherwise wash out."""
	r, g, b, a = color
	return (r * factor, g * factor, b * factor, a)


def pastel_color_for(icon: str):
	"""Deterministic pastel RGBA for `icon` -- hashed on the glyph alone (the
	part before "##", if any, since several buttons reuse the same glyph
	with a different ImGui-ID suffix purely for uniqueness -- see e.g.
	live_data_setup_dialog.py's "##live-data-folder"), so every button using
	the same icon shares the same color app-wide."""
	glyph = icon.split("##", 1)[0]
	digest = hashlib.sha1(glyph.encode("utf-8")).digest()
	return _PASTEL_PALETTE[digest[0] % len(_PASTEL_PALETTE)]
