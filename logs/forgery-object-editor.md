# Changelog

## 2026-08-16 — 🐛 Respect CMaterial::ZWRITE for blended materials in object_editor

Follow-up to the transparency work below: `apt_snowglobe.shape`'s glass sphere
(material 5, `flags=0x90` -- BLEND|LIGHTING, no ZWRITE bit) intermittently hid geometry
behind it (its base pedestal, and the grass patch under the reindeer) depending on draw
order, reproducing across app restarts but not always within a session -- confusing to
track down since toggling the material color override (which forces a full
`_reapply_material()`) sometimes "fixed" it, and it always looked fine from inside the
globe (camera past the glass, so it's simply not in the view frustum).

Root cause: `_apply_material()` never touched depth-write state, so every material wrote
depth (Panda3D's default), including ones the shape data explicitly marks as not writing
it (`CMaterial::flags & IDRV_MAT_ZWRITE`, checked in
`driver_opengl_material.cpp:497`'s `enableZWrite()`) -- exactly the ones meant to be
translucent and therefore not occlude what's behind them. Whichever object happened to
z-sort before the glass in a given run got its depth buffer values overwritten by the
(otherwise correctly alpha-blended) glass sphere, so anything drawn after failed the
depth test and never appeared. Ruled out an alpha-channel decoding bug first (checked
`apt_snowglobe_alpha.tga` -- actually a `.dds` via the extension fallback again -- alpha
sampled uniformly ~0.15, consistent with a deliberately subtle glass tint, not a decode
failure) before landing on this.

Added `_IDRV_MAT_ZWRITE` and `node_path.set_depth_write(bool(material.flags &
_IDRV_MAT_ZWRITE))` (default `True` when there's no material), matching `_two_sided`'s
existing pattern of following the shape's own flag instead of a fixed default.

## 2026-08-16 — 🐛 Render material transparency (blend/alpha-test) in object_editor

`_apply_material()` (`apps/object_editor.py`) only ever set up an opaque `Material`
(diffuse/ambient/emissive/specular) and a texture -- `CMaterial::flags` (BLEND,
ALPHA_TEST, DOUBLE_SIDED) were parsed by pynel but completely ignored by the viewer, so
every shape rendered fully opaque and double-sided regardless of what the material
actually specified. Found via `12thanniv_flacon.shape` (glass, should be translucent) and
`ge_mission_xmass_tree.shape` (foliage, should be alpha-tested cutout).

Added `_IDRV_MAT_BLEND`/`_IDRV_MAT_DOUBLE_SIDED`/`_IDRV_MAT_ALPHA_TEST` flag constants and
a `_TBLEND_TO_PANDA_OPERAND` table mapping `CMaterial::TBlend` (material.h) to
`panda3d.core.ColorBlendAttrib.Operand`, in the enum's own declaration order. `flags &
BLEND` now sets an explicit `ColorBlendAttrib` built from the material's actual
`src_blend`/`dst_blend` (not always the ordinary alpha srcalpha/invsrcalpha case -- e.g.
`12thanniv_flacon_body.tga`'s glass turned out to use `one/one`, i.e. genuine additive
blending, confirmed against the shape's own parsed data via the .agentcom bridge); `flags &
ALPHA_TEST` sets an `AlphaTestAttrib(M_greater, alpha_test_threshold)`, matching the
engine's own `glAlphaFunc(GL_GREATER, threshold)` (`driver_opengl_material.cpp:456`).
`set_two_sided()` now also follows `flags & DOUBLE_SIDED` instead of always being forced
on -- forcing it made additive-blend materials double their contribution from unwanted
backface overdraw, visibly whitening `12thanniv_flacon_body`. An initial version also
forced `TransparencyAttrib.M_alpha` alongside the explicit `ColorBlendAttrib`; removed
after user request, since it's redundant (the explicit attrib already wins) and not
correct for non-alpha blend funcs like the flacon's.

Also fixed two adjacent bugs hit while testing: `load_panda_texture()`
(`ryzom_forgery/shape_geometry.py`) crashed the whole app with an uncaught `AssertionError`
(`linear_size == header.pitch`, `panda/src/gobj/texture.cxx:8921`) on a malformed DDS
header (`ge_mission_xmass_tree_star.tga`, which -- like several textures here -- actually
resolves to a `.dds` file via `AssetIndex.find_texture()`'s extension fallback); now caught
and treated as a normal decode failure. And the DDS-loading path had no equivalent of the
PNM path's `image.flip(False, True, False)` (NeL vs Panda3D V-origin), so any texture that
happened to resolve to `.dds` rendered upside-down (confirmed on the xmas tree's foliage,
tip pointing the wrong way) while `.tga`/`.png`-backed ones didn't; the flip is now done
once, uniformly, on the UV coordinates in `_build_geom()` instead of per-format at texture
load time.

## 2026-08-16 — 🐛 Fix object_editor's default camera distance being too close

`ObjectEditorApp._load_shape()` framed a newly loaded shape's camera at `radius * 3.0`,
which the user found consistently too close (had to manually zoom out ~6 mouse-wheel
notches on every load to reach a comfortable distance). Since each wheel notch scales
distance by `1/zoom_speed` (`camera.py`'s `_zoom()`, `zoom_speed=0.9`), 6 notches is a
`(1/0.9)^6 ≈ 1.88x` factor; changed the framing multiplier to `radius * 5.65` (`3.0 *
1.88`, rounded) to match that as the new default.

## 2026-08-16 — ✨ Let object_editor convert a simple material to Multi Bitmap

A material imported from `.obj`/`.dae` (via the new `shape_importer.py`) always lands as
a plain single-texture material -- there's no way for those formats to express a Multi
Bitmap variant set. Added a "convert" icon button (fa-clone; fa-images, tried first,
turned out to be missing from imgui_bundle's bundled `fontawesome-webfont.ttf` -- added
in FA 4.7, this font predates it, confirmed by hand-parsing the ttf's `cmap` table since
`fontTools` wasn't installed) next to each simple-material row in the Materials tab:
`_convert_to_multi_bitmap()` swaps slot 0's `CTextureFile` for a `CTextureMultiFile`
seeded with a single set (index 0 = the current texture), so the material immediately
moves into the Multi Bitmap section, ready for more slots to be filled in.

Confirmed this is safe to save with only one slot filled: `CTextureMultiFile::getTexIndex()`
(`nel/src/3d/texture_multi_file.cpp:55-67`) already falls back to index 0 whenever the
engine asks for a season/quality index past the end of `_FileNames` (`selectTexture()`
itself never validates the index, just stores it) -- so a partially-filled Multi Bitmap
material just always shows its slot 0 texture until more slots are added, no crash or
out-of-bounds read regardless of what the game requests.

## 2026-08-16 — ✨ Add .obj/.dae -> .shape import to Forgery

Added `ryzom_forgery/shape_import.py` and `apps/shape_importer.py` (CLI:
`shape_importer.py INPUT.{obj,dae} OUTPUT.shape`), the reverse of the existing
`shape_exporter.py`/`shape_export.py`. Only `CMesh` can be produced this way -- unlike
`CMeshMRM`, pynel's `dumps()` writes `CMesh` geometry field-by-field rather than copying
bytes from an already-parsed file, so it's the only shape type buildable from scratch.
That means no LOD levels on an imported shape; investigated whether an existing tool
could add them afterward (`CMRMBuilder`, `nel/include/nel/3d/mrm_builder.h`): it operates
on the engine-native `CMesh::CMeshBuild` struct, not anything 3dsMax-specific, so it's
architecturally reusable, but no standalone CLI wrapping it exists today (the only
current caller is the 3dsMax-plugin-only `pipeline_max_export_shape`) -- writing one is a
separate, C++-only chantier (`CMRMBuilder` has no Python binding), noted for later.

`.obj`/`.mtl` are hand-parsed (same reasoning as the hand-written `.obj` exporter: simple
dependency-free text format) -- vertices/normals/texcoords, faces grouped by `usemtl`
(fan-triangulated if >3 verts per face, negative/relative indices resolved), materials
read from the referenced `mtllib` (Kd/Ka/Ks/Ns/d/Tr/map_Kd). `.dae` goes through
`pycollada` (already a dependency for `.dae` export) -- iterates
`doc.scene.objects("geometry")`'s bound `BoundTriangleSet`s (skipping non-triangle
primitives), reading each `Triangle`'s already-dereferenced vertex/normal/texcoord
values and its bound `Material.effect` (diffuse/ambient/specular/shininess/transparency,
falling back to a neutral default when a property is a texture `Map` rather than a plain
color). Both formats share an extracted `_assemble_mesh()` (dedup vertices into one
shared buffer, build the single matrix block/materials/bbox/MeshBase/MeshGeom) and a
generalized `_build_material()`.

Material defaults for properties the source format doesn't specify are grounded in
`nel/include/nel/3d/material.h`'s own documented default-construction values (line 273):
shader=Normal(0), src/dst blend=srcalpha(2)/invsrcalpha(3), z_function=lessequal(5),
flags=ZWRITE|LIGHTING|DOUBLE_SIDED.

Validated via the bridge with a full round-trip per format: export a real `.shape`
(`zo_paneau_armure.shape`) to `.obj`/`.dae`, import it back with the new importer, save
as a new `.shape`, re-parse. Both formats landed on identical numbers (5 materials, 66
verts, 38 tris, all 5 texture names correctly recovered), matching the original. Found
and fixed two bugs in the process: a generator-exhaustion bug in the bbox min/max
computation (`xs`/`ys`/`zs` consumed twice), and `.dae`'s `Triangle.material` being a
`collada.material.Material` rather than directly an `Effect` (needed `.effect`). User
confirmed both imported shapes render correctly and are editable in object_editor.py's
Materials tab like any other shape.

## 2026-08-16 — 🐛 Fix CMeshMRM rendering the wrong (coarsest) LOD, and resolve geomorph placeholder wedges

Found while investigating Forgery's object_editor showing too few materials/an oddly
shaped flag on `zo_paneau_armure.shape`: `ryzom_forgery/shape_geometry.py`'s
`_passes_from_mrm_geom` read `geom.lods[0]`, assuming it was the finest (most detailed)
LOD. Verified against `nel/src/3d/mesh_mrm.cpp` (`CMeshMRMGeom::chooseLod`'s alphaMRM
math, and the "just first lod is loaded" progressive-streaming comment) and pynel's own
`MeshMRMGeom.num_triangles` property (already using `lods[-1]`) that `lods[0]` is
actually the *coarsest* LOD. Fixed to `lods[-1]`. Confirmed empirically: 18 of 41 real
multi-LOD `.shape` files sampled have a different material set between `lods[0]` and
`lods[-1]`, always fewer materials/triangles in `lods[0]` (e.g. `zo_paneau_armure.shape`:
3 materials/6 tris vs. 5 materials/38 tris).

Switching to the finer LOD surfaced a second, deeper bug: some of its vertices rendered
collapsed at the origin. Root-caused to `CMeshMRM`'s geomorph mechanism (see
`mrm_builder.cpp`'s wedge-decal step and `CMeshMRMGeom::applyGeomorph` in
`mesh_mrm.cpp`): the progressive mesh reserves a block of empty placeholder wedges
(position/normal/uv all zero, a default `CWedge()`) shared for smooth blending between
adjacent LODs, which pynel was silently discarding instead of parsing (fixed on the
`ryzom/pynel` branch: new `MrmLod.geomorphs`). `shape_geometry.py` gained
`_resolve_lod_geomorphs()`, which substitutes each placeholder wedge's channels with its
geomorph "end" wedge (the correct static resolution, matching `applyGeomorph`'s blend at
alphaLod=0) before yielding render passes for a LOD. Verified via the bridge: the
previously-collapsed flag vertices on `zo_paneau_armure.shape` now resolve to real,
coherent positions matching the rest of the geometry.

## 2026-08-16 — ✨ Add a Multi Bitmap material editor to Forgery's object_editor

Added editing of a `.shape`'s materials to `object_editor.py`, focused on the "Multi
Bitmap" mechanism (`CTextureMultiFile`, see `docs/material_options.md`'s dedicated
section): a per-material texture slot can hold up to 8 alternate images, of which one is
selected at runtime based on context (season, item quality, creature ecosystem...).

The panel gained a "Materials" / "All Properties" tab bar (the latter is the pre-existing
generic `draw_properties()` tree, unchanged). The new Materials tab lists every material,
grouped simple-texture materials first, then Multi Bitmap ones:
- Multi Bitmap materials are shown per *slot index* (0-7) rather than per material --
  picking a slot is a whole-shape appearance choice, so its "Select" button (green when
  active) switches every Multi Bitmap material to that index at once. Each slot's label
  shows all three known Georges/engine numbering conventions for that index side by side
  (quality tiers from `item_map.typ`, creature ecosystem from `_creature_texture.typ`,
  and season from `EGSPD::CSeason` in `ryzom/common/src/game_share/season.h` -- which one
  actually applies depends on the shape). An "Expand" toggle reveals a per-material row
  to hand-edit that slot's texture (editable filename + a native file-browse button).
- Simple-texture materials get the same editable filename + browse row.
- Every material row has a color-swatch button (opens a picker, plus a "No color"
  option) that swaps that material's 3D geometry to a flat color -- lighting/material are
  explicitly disabled for that override (`set_material_off`/`set_light_off`), since a lit
  NodePath shades from its attached Material rather than `set_color()`. This is a
  viewer-only visualization, never written to the `.shape`.
- Icon-only buttons (Font Awesome 4, the only icon font `imgui_bundle` ships a `.ttf`
  for -- merged into the default font in the shared `ryzom_forgery/app.py`, so any
  Forgery tool app can use `icons_fontawesome_4` glyphs) replace the earlier text
  buttons, each with a hover tooltip for discoverability.
- The doc-derived Multi Bitmap explanation (from `material_options.md`, via a new
  `ryzom_forgery/material_docs.py` that splits the doc on its `## Title {#key}` headers)
  shows in the status bar in orange on hover, rather than as an ImGui tooltip (too wide
  for that use).
- "Save" (asks for a one-time per-session overwrite confirmation, then writes back to
  the loaded `.shape`'s path) and "Save As..." (always prompts for a new path) persist
  edits via `pynel.ryzom_shape.save_shape` -- confirmed via the bridge that an edited
  Multi Bitmap selection survives a save + re-parse round-trip.
- Every typed or browsed texture file name is lower-cased at the point of entry too
  (on top of pynel's own read/write normalization, from the `ryzom/pynel` branch), so
  mixed-case names from real data don't linger once touched.

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

## 2026-08-15 — 🐛 Support legacy (pre-1) CIndexBuffer format in pynel's .shape parser

`_parse_index_buffer` (`nel/tools/pynel/pynel/ryzom_shape.py`) previously raised
`ShapeParseError` on any `.shape` whose `CIndexBuffer` predates version 1 (NeL's
"primitive block" format, replaced by the flat index-buffer format in version 1) --
found via Forgery's new `shape_exporter.py` failing on a real production asset
(`sfx/mp_ressources_gen.shape`).

Added support for that `ver < 1` case, mirroring the exact byte layout of
`CIndexBuffer::serial`'s `ver < 1` branch in `nel/src/3d/index_buffer.cpp`: three
length-prefixed sections (lines, triangles, quads), each a `(count, capacity)` header
followed by its index vector. Only the triangle section carries renderable indices
(`_NbIndexes = triangle_count * 3`); the line and quad sections are read and discarded
to stay positioned correctly in the stream.
