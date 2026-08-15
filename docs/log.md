# Changelog

## 2026-08-15 — ✨ Add shape_exporter CLI app to Forgery

Added `nel/tools/forgery/apps/shape_exporter.py`, a command-line counterpart to the
object viewer's "Export to..." commands: `shape_exporter.py INPUT.shape OUTPUT.ext`
converts a single `.shape` to `.obj`/`.dae`/`.stl`/`.gltf`/`.glb`, the output format
picked from `OUTPUT`'s extension. Reuses `ryzom_forgery/shape_export.py`'s
`EXPORT_FORMATS` list and per-format writers directly, no GUI/Panda3D window involved.

Texture handling mirrors the GUI's two modes (`ryzom_forgery/export_config.py`), picked
via an optional `--data-root` (a Ryzom data tree, indexed with `AssetIndex` to resolve a
`.shape` material's texture file name to an actual file): without it, textures are left
as a reference to their original file name (`reference_only`); with it, defaults to
copying a decoded `.png` next to the export (`copy_png`), overridable with
`--texture-mode`.
