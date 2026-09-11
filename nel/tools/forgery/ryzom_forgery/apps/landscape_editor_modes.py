"""Shared render-mode/app-mode constants and pure helpers for `landscape_editor.py`
and its two mode mixins (`landscape_editor_edit_mode.py`/
`landscape_editor_view_mode.py`, project-todos/forgery/
landscape_editor__land_preview.md step 5) -- factored out here so both
mixins can use them without importing back from `landscape_editor.py`
itself (circular import), same reasoning as object_editor_mixins/
ui_helpers.py's own module docstring.
"""

from pynel import repository_paths

# Render mode bar (project-todos/forgery/landscape_editor__zone_render_modes.md
# step 6) -- WELD/LIGHT each accept one or more "real" extensions (a zone
# doesn't need its own standalone .zonew to count as welded for [WELD]: a
# shipped .zonel necessarily went through welding already, even when the
# intermediate .zonew was never kept -- real shipped live_data installs only
# ship the final pipeline stage, confirmed 2026-09-09, Nuno), falling back
# (see _resolve_zone_for_mode()) to a genuinely earlier stage, rendered as a
# purple->pink gradient (zone_geometry.build_zone_geom_from_cache(fallback=True) --
# a first grayscale attempt was indistinguishable from the viewport's own
# gray background, Nuno 2026-09-09) rather than the real elevation-colored
# one. POLY has no entry in _MODE_REAL_EXTENSIONS -- it always shows each
# zone's own default (highest-priority, region_loader.py's own
# .zonel > .zonew > .zone) ZoneRef, exactly the pre-chantier behavior.
_RENDER_MODES = ("POLY", "WELD", "LIGHT")
_MODE_REAL_EXTENSIONS = {"WELD": (".zonew", ".zonel"), "LIGHT": (".zonel",)}
_MODE_FALLBACK_EXTENSIONS = {"WELD": (".zone",), "LIGHT": (".zonew", ".zone")}


def _resolve_zone_for_mode(default_ref, ext_map, mode):
	"""Which ZoneRef to actually load for `mode`, and whether it's a real
	match (True) or a fallback (False, caller should render it gray) -- see
	_MODE_REAL_EXTENSIONS/_MODE_FALLBACK_EXTENSIONS module comment. POLY
	(no real-extensions entry) always returns `default_ref` unchanged."""
	real_exts = _MODE_REAL_EXTENSIONS.get(mode)
	if real_exts is None:
		return default_ref, True
	for ext in real_exts:
		if ext in ext_map:
			return ext_map[ext], True
	for fallback_ext in _MODE_FALLBACK_EXTENSIONS[mode]:
		if fallback_ext in ext_map:
			return ext_map[fallback_ext], False
	return None, False


# Global Visualisation/Edition mode (project-todos/forgery/
# landscape_editor__land_preview.md step 1) -- a single automatic switch for
# the whole app, not a per-continent/per-button choice like
# _resolve_zone_for_mode() above. "edition" whenever ryzom-data is configured
# and points at an existing directory (pynel.repository_paths.is_valid()),
# "visualisation" otherwise -- deliberately ignores whether that ryzom-data
# checkout actually has any .land in it (later steps filter the continent
# combo for that; an unconfigured/missing ryzom-data is the only thing that
# forces visualisation mode).
_MODE_VISUALISATION = "visualisation"
_MODE_EDITION = "edition"
_MODE_BADGE_COLOR = {_MODE_VISUALISATION: (0.4, 0.7, 1.0, 1.0), _MODE_EDITION: (0.4, 1.0, 0.4, 1.0)}
# "Release"/"Dev" (Nuno 2026-09-10) -- not "Visualisation"/"Édition" as
# originally named (step 1): those stay the internal mode identifiers
# (_MODE_VISUALISATION/_MODE_EDITION, and Settings.landscape_editor_mode's
# own stored values), only the UI label changed.
_MODE_BADGE_LABEL = {_MODE_VISUALISATION: "Release", _MODE_EDITION: "Dev"}
_OTHER_MODE = {_MODE_VISUALISATION: _MODE_EDITION, _MODE_EDITION: _MODE_VISUALISATION}


def _detect_app_mode():
	"""Pure auto-detection from ryzom-data's own configuration -- only ever
	used as the default the very first time (Settings.landscape_editor_mode
	still unset), see LandscapeEditorApp._resolve_app_mode()'s own docstring
	for the manual Release/Dev toggle (step 4) that normally takes over from
	here."""
	return _MODE_EDITION if repository_paths.is_valid("ryzom-data") else _MODE_VISUALISATION
