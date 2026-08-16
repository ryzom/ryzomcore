# Changelog

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
  (on top of pynel's own read/write normalization), so mixed-case names from real data
  don't linger once touched.

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
adjacent LODs, which `pynel` was silently discarding (`f.skip_pair_vector() # Geomorphs`)
instead of parsing. Added `_Reader.pair_vector()` and wired it into
`_parse_mesh_mrm_geom`/`_parse_mesh_mrm_skinned_lod` (new `MrmLod.geomorphs`/
`MeshMRMSkinnedLod.geomorphs` fields). `shape_geometry.py` (Forgery) gained
`_resolve_lod_geomorphs()`, which substitutes each placeholder wedge's channels with its
geomorph "end" wedge (the correct static resolution, matching `applyGeomorph`'s blend at
alphaLod=0) before yielding render passes for a LOD. Verified via the bridge: the
previously-collapsed flag vertices on `zo_paneau_armure.shape` now resolve to real,
coherent positions matching the rest of the geometry.

## 2026-08-16 — ✨ Normalize .shape texture and .bnp entry names to lowercase in pynel

Texture file names aren't case-sensitive, and real Ryzom data mixes casing (e.g.
`"ZO_flag_AS.TGA"` vs `"zo-toit2.tga"`). `pynel` now normalizes to lowercase
consistently: `ryzom_shape.py`'s `_parse_texture_file`/`_parse_texture_multi_file` and
their writer counterparts lower-case `Texture.file_name`/`file_names` on both read and
write (so a hand-built `Texture` gets normalized too, not just round-tripped ones); the
module docstring now notes this isn't a byte-exact reader on that one point.

`ryzom_bnp.py` goes the same way for archive entry names: `_read_table`/`_encode_name`
now lower-case on both read and write, replacing the previous "compare
case-insensitively but preserve original case on disk" behavior documented in the module
docstring (updated to match) -- `BnpReader.find()`'s case-insensitive comparison was
already in place, this just makes the stored/listed names consistent too instead of only
the lookup.

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

## 2026-08-08 — ⚡ Lower UpdateTimeout to reduce admin command callback latency

Admin commands sent via `query_shard()` (`nel/tools/pynel/pynel/admin_modules_itf.py`, same
pattern exists in the PHP equivalent) execute and log instantly on the EGS, but the
`wait_callback` round-trip back to the calling script consistently took ~450ms regardless of
the command (even an unknown command returned in the same ~450ms), pointing at transport/dispatch
overhead rather than command execution time.

Traced the command path: client -> RAS (`admin_service`) -> AES (`admin_executor_service`,
a separate process, not a module inside the EGS) -> EGS (`entities_game_service`) -> AES -> RAS
-> client. Each of these three services is a `NLNET::IService` subclass whose main loop caps
network polling/dispatch to its `UpdateTimeout` config variable (`nel/src/net/service.cpp`),
which defaults to 100ms and is only read once at startup (no live-reload, a service restart is
required for a config change to take effect).

Measured empirically on the live shard by lowering `UpdateTimeout` to 10 one service at a time
and restarting: RAS alone brought ~450ms down to ~260-330ms; adding EGS made no measurable
difference (masked by the AES, the actual bottleneck); adding AES brought it down to ~45-50ms,
roughly a 10x improvement overall.

Added `UpdateTimeout = 10;` to `ryzom/server/tools/cfg_creator/templates/admin_service.cfg`,
`admin_executor_service.cfg`, and `entities_game_service.cfg` so shards generated from these
templates get the lower latency by default. Kept all three set (not just the AES) since the
EGS's lack of measured impact was likely masked by the AES bottleneck rather than proven
negligible, and the cost of a lower `UpdateTimeout` (more frequent network polling) is
negligible.

## 2026-08-04 — 🔧 Modernize sheets_packer_shard's compile definitions to TARGET_COMPILE_DEFINITIONS

`ryzom/server/tools/sheets_packer_shard/CMakeLists.txt` used
`ADD_DEFINITIONS(-DNO_EGS_VARS)` / `ADD_DEFINITIONS(-DNO_AI_COMP)`
(directory-scoped, legacy). On `main/yubo-dev`, this target had already been
migrated to `TARGET_COMPILE_DEFINITIONS(sheets_packer_shard PRIVATE
NO_EGS_VARS DNO_AI_COMP)` — target-scoped, but with a typo
(`DNO_AI_COMP` instead of `NO_AI_COMP`) that silently broke the
`#ifndef NO_AI_COMP` guards in `ai_service/sheets.cpp`, causing
`sheets_packer_shard` to try linking against `CFightScriptCompReader`/
`CFightSelectFilter` (only defined in `ai_script_comp.cpp`, not part of this
target's sources) — `undefined reference` at link time. Applied the same
`TARGET_COMPILE_DEFINITIONS` modernization here on `fixes`, correctly
spelled, so this commit can be merged into `main/yubo-dev` to fix the typo
there via the merge rather than editing that branch directly.

## 2026-08-03 — 🐛 Fix MSVC operator< ambiguity for unqualified NUM_SKILLS in skill_manager.cpp

`skill_manager.cpp` has `using namespace SKILLS;` at file scope, so its two
`i < NUM_SKILLS` loop bounds use the unqualified name — the earlier sweeps
only searched for the qualified `SKILLS::NUM_SKILLS` form and missed these.
Cast both to `(uint)`.

Commit: 🐛 Fix operator< ambiguity for unqualified NUM_SKILLS

## 2026-08-03 — 🐛 Fix MSVC operator== ambiguity for sint skillValue vs SKILLS:: constants

Same family of issue, but this time against specific `SKILLS::` constants
(`SF`/`SM`/`SC`/`SH`) rather than `NUM_SKILLS` — `sint skillValue ==
SKILLS::SF` and friends, in `group_phrase_skill_filter.cpp` and
`group_skills.cpp` (8 sites total). Cast the enum side explicitly at each.
Left `itemSkill`/`compareSkill` comparisons against `SKILLS::` constants in
`sphrase_manager.cpp` untouched — those variables are themselves
`SKILLS::ESkills`, so same-enum-type comparisons, never ambiguous.

Commit: 🐛 Fix operator== ambiguity for skillValue vs SKILLS constants

## 2026-08-03 — 🐛 Fix MSVC operator< ambiguity for ITEM_TYPE::UNDEFINED comparisons

Same family of issue as the `NUM_*` enum-bound sweep below, this time for
`ITEM_TYPE::TItemType` in `bot_chat_page_trade.cpp`: `index<ITEM_TYPE::
UNDEFINED` (an `sint` compared against the enum bound) and a
`nlctassert(ITEM_TYPE::UNDEFINED<=128)` static assertion. Cast the enum
side explicitly at both sites.

Commit: 🐛 Fix operator< ambiguity for ITEM_TYPE::UNDEFINED

## 2026-08-02 — 🐛 Fix the same MSVC ambiguity for !=, ==, <=, >, >= against NUM_* enum bounds too

Follow-up to the `operator<`-only sweep below: the exact same MSVC
ambiguity (against unrelated `operator!=`/`operator==` overloads reachable
at that scope) also hits `!=`, `==`, `<=`, `>`, `>=` comparisons against the
same `NUM_*` enum bounds, in both operand orders (`x != SKILLS::NUM_SKILLS`
and `SKILLS::NUM_SKILLS == x`). Searched `ryzom/client`, `ryzom/common` and
`nel` for every remaining comparison operator against any of the same
7 enum bounds and cast the enum side explicitly, in both directions.

Commit: fix: resolve remaining MSVC-ambiguous !=/==/<=/>/>= comparisons against NUM_* enum bounds

## 2026-08-02 — 🐛 Fix MSVC operator< ambiguity for every NUM_* enum-bound loop/comparison in the client

Same family of issue as the two fixes below, but instead of patching one
call site at a time as each one surfaced during the Windows/MSVC
cross-build, searched `ryzom/client`, `ryzom/common` and `nel` for every
remaining `< SOMENAMESPACE::NUM_*` comparison against an enum bound
(`SKILLS::NUM_SKILLS`, `SCORES::NUM_SCORES`, `RM_FABER_TYPE::NUM_FABER_TYPE`,
`CHARACTERISTICS::NUM_CHARACTERISTICS`, `MAGICFX::NUM_SPELL_POWER`,
`JOBS::NUM_CAREER_DB_SLOTS`, `INVENTORIES::NUM_ALL_INVENTORY`) across 20
files and cast the enum bound explicitly at each site — MSVC treats these
as ambiguous against unrelated `operator<` overloads reachable at that
scope (e.g. `CProjectileBuild`, `CClientDate`, `CSessionId`, ...), GCC
doesn't. `ryzom/server` wasn't touched (not built for the Windows client
target, so not hit here, but the same fix would apply if anyone brings it
up under MSVC too).

Commit: fix: resolve remaining MSVC-ambiguous operator< comparisons against NUM_* enum bounds

## 2026-08-02 — 🐛 Fix another MSVC operator< ambiguity in action_handler_help.cpp

Same family of issue as the `CHARACTERISTICS` loop fix below: `for (skillNb = 0;
skillNb < SKILLS::NUM_SKILLS; ++skillNb)` in `action_handler_help.cpp`
(comparing `uint` against the `SKILLS::ESkills` enum) is ambiguous under MSVC
19.20 (encountered bringing up the Windows/MSVC cross-build) but not GCC.
Cast the loop bound to `uint` explicitly to remove the ambiguity.

Commit: fix: resolve another MSVC-ambiguous operator< comparison (action_handler_help.cpp)

## 2026-08-02 — 🐛 Fix ambiguous operator< on MSVC in CINCarac serialization loops

`CInCarac::serialBitMemStream()` (and its two neighbours) in `msg_client_server.h`
looped with `for (int i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i)`. GCC
resolves the `int < enum` comparison via the built-in `operator<` without issue, but
MSVC (19.20, encountered while bringing up a Windows/MSVC cross-build) reports it as
ambiguous against `operator<(const CSessionId&, uint32)` from `r2_basic_types.h`,
since both a user-defined conversion path and the built-in enum-to-int promotion are
considered equally viable candidates. Cast the loop bound to `int` explicitly to
remove the ambiguity — behavior is unchanged on every compiler.

Commit: fix: resolve MSVC-ambiguous operator< in CHARACTERISTICS loop bounds

## 2026-07-30 — 🐛 Stop despawning entity on client sheet change

`CBot::setClientSheet()` (used by the `setClientSheet()` AI script command) used to call
`sheetChanged()`, which fully despawns and respawns the bot on the AIS: a new `EntityId`
and mirror row are allocated, the world map link is redone, and EGS is notified of a
despawn/respawn — all just to update the purely cosmetic client-facing sheet. This threw
away the entity's live game state (aggro, current target, timers) even though the server
sheet, position and identity never actually changed. `setClientSheet()` now updates the
mirror's `Sheet` property directly on the existing entity instead, preserving its identity
and state; a real sheet/stat change (`setSheet()`) still goes through the full
despawn/respawn since the underlying creature type can change.

## 2026-07-30 — 🐛 Preserve entity state across client sheet swap

Following the server-side fix above, the client still reacted to any client-sheet update
by fully destroying and recreating the local `CEntityCL` for that slot (`remove()` then
`create()`), which made the entity disappear for a couple of seconds and, since
`remove()` calls `slotRemoved()` on every other entity, could permanently drop the
player's target/selection on that slot with no way to re-target it.

Added `CEntityManager::changeEntitySheet()` / `updatePendingSheetChanges()`: the
replacement entity is built off-screen first, then swapped into the same slot in a single
step (the slot is never left empty and `slotRemoved()` is never called), so any
target/selection on that slot survives the appearance change. If the slot turns out to
really be taken over by a different underlying server entity (different `DataSetIndex`),
the code still falls back to the previous full `remove()`+`create()` path.

Since the server has no reason to resend properties that didn't change just because the
sheet did, the replacement's position/orientation are copied from the old entity, and all
other per-slot visual properties (equipment/colors, mode/alive state, contextual
attackable/selectable bits, HP bars, target lists, guild, faction, pvp, mount/rider...)
are re-applied from the per-slot CDB right after the swap.

## 2026-07-30 — 🐛 Respect Turn sheet flag when facing target

`CCharacterCL::applyBehaviour()` and `CCharacterCL::beginCast()` unconditionally called
`dir()` to snap an entity's facing direction toward its combat/cast target, ignoring the
sheet's `Properties.Turn` flag (`_CanTurn`) even though the neighbouring `front()` call
already respected it. Since `dir()` (not `front()`) is what actually drives the rendered
orientation for these entities, an entity with `Turn=false` (e.g. a static decoration)
would still visibly snap to face the player as soon as it attacked or cast a spell. `dir()`
is now only called in these two spots when `_CanTurn` is true.
