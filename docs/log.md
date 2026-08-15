# Changelog

## 2026-08-15 — 📝 Add player-friendly material options doc, rename object_viewer to object_editor

Added `nel/tools/forgery/docs/material_options.md`, a plain-language (non-technical,
written in French for the target audience) explanation of every material option exposed
by the 3dsMax "NelMaterial" editor (`nel/tools/3d/plugin_max/scripts/startup/nel_material.ms`)
and stored in a compiled `.shape`'s `CMaterial`: base colors, opacity, specular/glossiness,
self-illumination, two-sided, lighting modes, shader type (Normal/UserColor/LightMap/
Specular/Water/PerPixelLighting...), blend (alpha blend vs. additive), alpha test, z-bias,
z-write, the 4 usable texture slots and their per-shader meaning, texture coordinate
generation, multi-texturing (operation/arguments/constant color), water-specific settings,
user color, and the texture matrix export flag.

Catalogued by reading `nel_material.ms` (the MaxScript UI), `export_material.cpp` (how
each UI parameter is serialized into `CMaterial`), and `nel/include/nel/3d/material.h`
(the runtime enums) -- confirmed empirically against 600 real `.shape` files from
`ryzom-data` via pynel that a material never carries more than 4 textures in practice
(`IDRV_MAT_MAXTEXTURES = 4`), even though the 3dsMax editor exposes 8 texture slots in its
UI (slots 5-8 exist only for the Water shader's editor-only settings, never written to
`CMaterial`).

Also documented the "Nel Multi Bitmap" texture type (`nel_multi_set.ms`,
`export_material.cpp:1217-1256`): not an extra texture slot, but a per-slot feature
letting a single texture channel hold up to 8 alternate images, runtime-selected via
`CTextureMultiFile::selectTexture()` (`texture_multi_file.h`) based on context (season --
`continent.cpp` -- or item quality/variant -- `character_3d.cpp`/`player_cl.cpp`). Already
parseable by pynel (`_parse_texture_multi_file` in `ryzom_shape.py`, pre-existing).

Grounded that section with real Georges sheet fields, cross-referenced in
`ryzom-data/leveldesign/DFN`: creature sheets' `Texture` field
(`_creature_texture_equipment.typ`, -1 = "Season" special value falls back to
season-based selection instead of a fixed index), items' `map_variant` field
(`item_map.typ`, the 8 quality tiers Low/Medium/High/Super/XL/Suprem/Divine/Obiwan
Quality, matching `player_cl.cpp`'s `selectTextureSet((uint)item->MapVariant)` call), and
`_creature_texture.typ` (none/Lacustre/Desert/Jungle/Primr/goo, a creature's
ecosystem-dependent appearance).

Renamed `apps/object_viewer.py` to `apps/object_editor.py` (class `ObjectViewerApp` ->
`ObjectEditorApp`, window title, debug log prefixes, and every doc/comment mention) ahead
of adding material editing to it -- "viewer" no longer described its role once it becomes
able to edit a `.shape`'s material, not just inspect it.

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
