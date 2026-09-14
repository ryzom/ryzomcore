"""Shared render-mode/app-mode constants and pure helpers for `landscape_editor.py`
and its two mode mixins (`landscape_editor_edit_mode.py`/
`landscape_editor_view_mode.py`, project-todos/forgery/
landscape_editor__land_preview.md step 5) -- factored out here so both
mixins can use them without importing back from `landscape_editor.py`
itself (circular import), same reasoning as object_editor_mixins/
ui_helpers.py's own module docstring.
"""

from pynel import repository_paths

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
