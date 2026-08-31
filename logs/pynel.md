# Changelog

## 2026-08-30 — ✨ Add item/sitem.packed_sheets support to ryzom_packed_sheets, pynel 0.7.0

Extends `pynel.ryzom_packed_sheets` (see the entry below for the initial
`creature.packed_sheets` support) with `item.packed_sheets`/
`sitem.packed_sheets` (`CEntitySheet::ITEM` / `CItemSheet`, both extensions
share the same class and `TypeVersion[]` entry, version 44) —
`parse_item_packed_sheets()`/`load_item_packed_sheets()`, new `ItemSheet`
dataclass plus its sub-structures (`ItemFX`, `StaticFX`, `MpItemPart`,
`Scroll`, `Rgba`) and the 11 `Family`-keyed union variants
(`Cosmetic`/`Armor`/`MeleeWeapon`/`RangeWeapon`/`Ammo`/`Mp`/`Shield`/`Tool`/
`GuildOption`/`Pet`/`Teleport`/`Consumable`, dispatched off
`ITEMFAMILY::EItemFamily` exactly like the real `CItemSheet::serial` switch).
CLI `ryzom-packed-sheets dump` now auto-detects creature vs. item from the
filename (or `--kind`).

Directly motivated by a gap found while validating `creature.packed_sheets`
last entry (below): `CharacterSheet.<slot>.id_item` only names an
`.item`/`.sitem` sheet, not a real `.shape` — the actual mesh filename
(`IdShape` + 4 race × 2 gender variants) only lives in *this* sheet. That
loop is now closed: `fyhc1.creature`'s `body` slot (`fy_civil01_gilet.item`)
resolves through `item.packed_sheets` to a real `ARMOR`-family `ItemSheet`.

Header/dependency-block parsing refactored into a shared
`_parse_packed_sheets_header()`/`_parse_entity_map()` pair, reused by both
`parse_creature_packed_sheets()` and the new `parse_item_packed_sheets()` —
same header, same map-of-`(type, id, payload)` shape, only the payload
parser and expected type/version differ.

Format documented in `docs/packed_sheets.md` (new sections: full
`CItemSheet::serial` field order, the `Family` → union-struct dispatch
table, `CItemFXSheet`). Validated against all three real files on the
maintainer's machine: 799 `item.packed_sheets` entries and 8761
`sitem.packed_sheets` entries, both fully consumed with no trailing bytes
and no exception; the union dispatch spot-checked across all 27
`ITEMFAMILY` values in the real `sitem.packed_sheets` (decoded struct
present exactly where `CItemSheet::serial`'s switch has a case for that
family, `None` everywhere else, matching the C++ exactly).

## 2026-08-30 — ✨ Add ryzom_packed_sheets (creature.packed_sheets + sheet_id.bin), pynel 0.6.0

New `pynel.ryzom_packed_sheets` module (CLI `ryzom-packed-sheets`): reads
`creature.packed_sheets` (the client's binary cache of `.creature` Georges
sheets, `NLGEORGES::loadForm()`/`CSheetManagerEntry`/`CCharacterSheet`) and
`sheet_id.bin` (raw `CSheetId` -> readable sheet name, `map<uint32,string>`,
ships inside `leveldesign.bnp`). Read-only: the client always regenerates
this cache from the source sheets, so there's no reason to write it back.

Format fully reverse-engineered from `nel/include/nel/georges/load_form.h`,
`nel/include/nel/misc/sheet_id.{h,cpp}`, `ryzom/client/src/sheet_manager.cpp`
and `ryzom/client/src/client_sheets/character_sheet.cpp` — see
`nel/tools/pynel/docs/packed_sheets.md` for the full writeup, including
version guards (`PACKED_SHEET_VERSION=5`, creature class version 17) and the
exact `CCharacterSheet::serial` field order (~50 fields incl. 9 equipment
slots, ground FX, body-to-bone mapping, attack lists).

Only `CEntitySheet::FAUNA`/`CCharacterSheet` is implemented — the other ~25
sheet types the format supports (`.item`, `.sbrick`, `.mission`, ...) raise
`PackedSheetsParseError` and are left for future sessions, one per type, same
investigation pattern.

Validated against the real file on the maintainer's machine: parses all
28545 entries of a live `creature.packed_sheets` with no trailing bytes and
no exception, names resolved correctly via `sheet_id.bin`, and full-field
dumps of two real NPCs cross-checked by hand (a bare-bodied
`basic_fyros_male.creature` and an equipped `fyhc1.creature`).

While inspecting `fyhc1.creature`'s equipment, confirmed that a
`CharacterSheet`'s `.shape` filenames are **not** in `creature.packed_sheets`
itself: non-empty `Equipment.id_item` fields are `.item`/`.sitem` sheet
names, and the real shape only shows up one level further via
`CItemSheet::getShape()` (race/gender variants) — `item.packed_sheets` isn't
implemented yet, noted as the natural next step in `docs/packed_sheets.md`.
Also corrected an initial wrong assumption that `automaton_list.packed_sheets`
would hold shape references — it's actually an animation state-machine
(`CAutomatonStateSheet`), unrelated to meshes.

## 2026-08-29 — ✨ Add repository_paths (shared 4-repo checkout locations)

New `pynel.repository_paths` module: a small per-user JSON settings file
(`config_dir()/"repository_paths.json"`, own OS-detection reimplementation since pynel
has zero dependencies and must not depend on Forgery) mapping the 4 git repositories a
Ryzom Core contributor typically checks out side by side --
`REPOSITORIES = ("ryzom-core", "ryzom-data", "ryzom-private-data", "ryzom-docker")` --
to their local paths on a given machine. `load()`/`save()`/`set_path()`/`get()`/
`is_valid()` (configured AND currently a real directory).

Motivated by Forgery/Patina's real Panoply bake (`ryzom_forgery.panoply_bake`, see the
Forgery log below): it needs `ryzom-data`'s real `characters.hlsbank`/
`panoply_files.txt` (`final_bnps/characters_maps_hr/`) as the source to append newly
baked items into, and rather than inventing a Forgery-only setting for that, this lives
in pynel so any tool built on it (Forgery, a future ryztart integration) resolves
"where is ryzom-data" the same way, configured once. Deliberately outside
`ryzom_forgery.settings` (Forgery's own tomlkit-based file) for that reason.

Validated in-sandbox (no external dependency involved): set/get/is_valid round-trip,
unconfigured-repo/unknown-repo-name handling all confirmed. Documented in
`docs/repository_paths.md` (new) + `README.md`. Bumped pynel to 0.5.0.

## 2026-08-29 — ✨ Add .hlsinfo writer + CConfigFile port (ryzom-cfg)

Two additions to pynel, part of the panoply_maker Python port:

- **`.hlsinfo` writer**: `dumps_hlsinfo()`/`save_hlsinfo()` added to
  `hls_bank_texture_info.py`, symmetric with the existing reader. Round-trip
  (`parse_hlsinfo` -> `dumps_hlsinfo`) validated byte-identical against all 10 real
  `.hlsinfo` files available. Needed so the upcoming Forgery `panoply_maker` port can
  write its output in the same format the native tool produces.
- **`config_file.py`, new module**: full port of NeL's `CConfigFile` format (`.cfg` --
  `client.cfg`, `panoply_*.cfg`, and every other NeL/Ryzom tool config), from the real
  flex/bison grammar (`nel/src/misc/config_file/cf_lexical.lpp`/`cf_gramatical.ypp`).
  Scope grew beyond panoply during this port: `client.cfg` loads through the exact same
  `CConfigFile` class, and this is meant to replace ryztart's own hand-rolled `.cfg`
  parser (which doesn't cover the full format), so it's a generic reusable module, not
  panoply-specific. Covers comments, arrays, arithmetic expressions (`+ - * /`,
  parentheses, unary minus), variable references, `+=` (array extension), and the
  `RootConfigFilename`/`Root`/`FromLocalFile` multi-file override semantics (e.g.
  `client.cfg` falling back to `client_default.cfg`) -- `#fileline` (an internal
  multi-file line-tracking marker, never hand-authored) is explicitly not ported.
  `Document` keeps a file's exact original text and only rewrites the touched
  statement's value on `set()`, so `dumps()`/`save()` round-trips byte-identically when
  nothing was changed -- validated on the 7 real `panoply_*.cfg` files. `ConfigFile`
  (the multi-file merged view) validated against the real `client.cfg` ->
  `client_default.cfg` chain (292 variables merged, `RootConfigFilename` followed
  automatically). A regex-ordering bug (`REAL`'s pattern tried the no-fraction form
  before the fraction-required one, so `0.1` tokenized as `0.` + `1` -- plain regex
  alternation is first-match, not flex's longest-match) was found and fixed during this
  validation. New `ryzom-cfg` console script (`dump`/`set` subcommands).

Registered both in `pyproject.toml`, bumped pynel to 0.4.0, documented in
`docs/config_file.md` (new) and `README.md`.

## 2026-08-29 — ✨ Add ryzom-hlsbank CLI (dump + add)

New `pynel.hls_texture_bank` CLI, registered as the `ryzom-hlsbank` console script.
`dump <bank.hlsbank>` lists every color texture and item instance in a `.hlsbank` file,
useful to audit which armor names are actually present (e.g. checking for items missing
due to lost `.hlsinfo` sources). `add <bank.hlsbank> <hlsinfo...> -o <output.hlsbank>`
appends new entries from one or more `.hlsinfo` files into an existing bank, wrapping the
already-validated `append_texture_info()` writer (`pynel.hls_texture_bank`) and the
`.hlsinfo` reader (`pynel.hls_bank_texture_info`) added earlier. This is the pynel
equivalent of the native `hls_bank_maker`, but incremental (append-only, no need for
every historical item's sources) instead of full-rebuild-only. Validated against a real
`.hlsinfo` (`tr_hom_civil01_hand_r1.hlsinfo`): appends cleanly, leaves the existing 1574
color textures / 19733 instances of `characters.hlsbank` byte-for-byte unchanged, adds 32
new instances. Bumped pynel to 0.3.0.

## 2026-08-18 — 🐛 Fix skinning pose bugs in pynel animation eval

Three bugs in `ryzom_animation.py`'s bone pose evaluation, found while validating Forgery's
new Skinning preview against real creature data (`tr_mo_zerx.shape`/`.skel`/`_baillement.anim`,
via `nel/tools/pynel/pynel/ryzom_bnp.py`'s `BnpReader` against the shipped `.bnp` archives).

- **Root bone bind pose**: the Max exporter (`nel/tools/3d/plugin_max/nel_mesh_lib/
  export_skinning.cpp:307-313`) deliberately writes a skeleton's root bone `DefaultPos`/
  `DefaultRotQuat` as identity -- "path are setuped interactively in the root of the
  skeleton", i.e. real placement/orientation is meant to come from game code, not the
  `.skel` file. But `InvBindPos` is baked from the bone's real bind-time orientation in Max,
  not this placeholder. `_bone_local_matrix()` now reconstructs that real orientation as the
  inverse of `InvBindPos` (new `_invert_matrix()`/`_matrix_field_to_dense()` helpers, ported
  from `CMatrix::inverted()`) whenever no animation track overrides the root's position/
  rotation. Without this, every bone's skin matrix carried a constant, skeleton-wide rotation
  offset inherited from the root -- visible as the whole mesh rigidly misoriented (confirmed
  numerically: `World(bind) * InvBindPos` was a constant non-identity matrix across every
  bone tested, not just the root, since the offset propagates down the whole hierarchy).
- **`_slerp()`**: no longer corrects for the two quaternions being on opposite hemispheres.
  The real engine's `CQuat::slerp()` (`nel/include/nel/misc/quat.h`) has that correction
  commented out, relying entirely on exported animation data already having consecutive keys
  pre-baked onto the same hemisphere -- "fixing" it in the port made playback diverge from
  what the real client renders for the rarer tracks (legacy `CTrackKeyFramerLinearQuat`,
  typical of biped exports) that aren't perfectly hemisphere-consistent.
- **`_evaluate_keyframer()` loop seam**: at the exact point an animation wraps back to its
  first key, the real engine (`ITrackKeyFramer::eval()`, `track_keyframer.h`) doesn't
  interpolate at all -- it snaps directly to the first key's value (`previous` stays `NULL`
  at that exact point). The port previously blended toward it instead.

Also added `evaluate_all_bone_world_matrices()`: computes every bone's world matrix in a
single O(bone count) pass (each bone's local matrix computed once and memoized, parent
resolved via recursion instead of independently re-walked), instead of calling
`evaluate_bone_world_matrix()` once per bone -- O(bone count × hierarchy depth), redoing the
same shared-ancestor matrix multiplications over and over. Wired into Forgery's per-frame
re-skin (`object_editor.py`'s `_bone_world_matrices_for()`), where the old approach was
costing 5-30+ fps on real creatures with 30-90 bones.

## 2026-08-18 — ✨ Add CPU linear-blend skinning for CMeshMRMSkinned

New module `pynel/ryzom_skin.py`: `skin_mesh(geom, skeleton, bone_world_matrices)` resolves
every vertex's final position/normal from up to 4 weighted bone influences
(`nel/src/3d/mesh_mrm_skinned.cpp:1163-1240` confirms a `PackedVertex.matrices[i]` is a direct
index into the mesh's own `bones_name` list, not an indirection through anything else).
`BoneSkinMatrix = WorldMatrix(bone) * InvBindPos(bone)` (`bone.cpp:228-241`) -- the caller
supplies `WorldMatrix` per bone already evaluated (typically via
`ryzom_animation.evaluate_bone_world_matrix()`), keeping this module decoupled from
`ryzom_animation` entirely (only depends on `ryzom_shape`). `ryzom_shape.Matrix`'s sparse
on-disk encoding is converted to a dense 4x4 for `InvBindPos` -- `Scale33`/`proj` are dropped,
confirmed unused by the engine's own `mulPoint()`/`mulVector()`.

Validated against a real non-humanoid creature (`fo_carnitree.shape`/`.skel`, 35 bones, 1255
vertices): the skinned bind-pose position range matches the shape's own stored bbox on all
three axes almost to the last decimal, and every normal comes out unit-length.

The first validation attempt used a humanoid (fyros female) shape instead, and consistently
skinned ~0.95m off from its bbox regardless of which skeleton variant or animation was fed in.
Traced to a real, non-buggy property of that rig: `Bip01` (root bone) has `default_pos=(0,0,0)`
in the `.skel`, while its `InvBindPos` implies a genuine ~0.95m bind height -- the client
re-applies the character's true height separately, through the "Gabarit" system
(`ryzom/client/src/gabarit.cpp`, a per-bone scale blended from body-type `.skel` variants), not
through the root bone's own position. `skin_mesh()` itself has no bug here; it faithfully
reproduces whatever `WorldMatrix` it's given -- humanoid `.skel` files just aren't
self-sufficient for a correct bind pose without also replicating Gabarit, which is out of
scope for this module.

## 2026-08-18 — ✨ Add bone world-pose evaluation to pynel's .anim reader

`evaluate_bone_world_matrix(skeleton, bone_name, anim, time)` in `ryzom_animation.py` (+ CLI
`ryzom-anim pose SKEL.skel ANIM.anim BONE TIME`), closing out the ".anim read + track
evaluation" chantier (Step 3-4, now fully done). Composes a bone's world-space 4x4 matrix at
an arbitrary time by walking up `father_id` from `pynel.ryzom_shape.SkeletonShape`, using each
ancestor's `.pos`/`.rotquat`/`.scale` animation track when the clip has one (falling back to
the bone's own `default_*` from the `.skel` otherwise) -- so it also works with `anim=None` to
preview a static bind pose.

Matrix math (`_mat_translate`/`_mat_rotate`/`_mat_scale`/`_mat_mul`) replicates
`NLMISC::CMatrix`'s exact row-major layout and quaternion-to-rotation formulas
(`CMatrix::setRot(CQuat)`/`mulPoint`, `misc/matrix.cpp`), not a generic linear-algebra
convention picked independently -- needed since composing matrices in the wrong convention
would silently produce plausible-looking but wrong results.

**Found only by testing against a real skeleton, not anticipated in the original plan**: a
naive `WorldParent * Local` composition gave non-orthonormal rotation submatrices (row
magnitudes around 11 instead of 1). Root cause is `CBone::UnheritScale` (`bone.cpp:160-226`,
true by default for every bone): a child bone inherits its father's translation but *not* its
non-uniform scale -- 3dsMax biped rigs bake bone length/thickness into that scale, and without
this compensation it would visibly stretch every child bone's own geometry. The engine inserts
a compensation matrix (`T*(1/FatherScale)*T⁻¹`, scaling the child's local translation point
by the inverse father scale around itself) between parent and child whenever `UnheritScale` is
set -- replicated exactly rather than approximated.

Validated via the bridge on a real Fyros skeleton+animation pair
(`fy_hof_skel_mid_fat.skel` + `fy_hof_emot_20_anniversary_dance.anim`): `Bip01 R Hand`'s
composed rotation submatrix is properly orthonormal at two different times, and the root bone
`Bip01` (no father, no UnheritScale compensation needed) resolves to the identity matrix as
expected. No in-engine reference renderer was available for a pixel-perfect comparison;
judged sufficient without one.

## 2026-08-17 — ✨ Add track evaluation to pynel's .anim reader

`evaluate_track(track, time)` in `ryzom_animation.py` (+ CLI `ryzom-anim eval FILE.anim
TRACK TIME`), evaluating any of the module's track kinds at an arbitrary time, with each
kind's own real looping/interpolation semantics replicated from the C++ rather than a single
approximate generic algorithm:

- Constant tracks (`CTrackDefault*`): return the value directly, ignoring `time`.
- `CTrackSampled*`: per-key times rebuilt from the `TimeBlock` frame indices (the C++'s own
  frame-quantized dichotomy search in `CTrackSampledCommon::evalTime()` is only a compactness
  optimization over this same data -- a plain bisect on the flat list gives the same result),
  lerp for vectors, slerp for quaternions. Past the last key, the value holds rather than
  blending back to the first key across the loop seam -- matches what `evalTime()` actually
  does, not a limitation on this side.
- `CTrackKeyFramerLinear*`: loop bounds re-derived from `RangeLock`/the first-last keyframe's
  own time, replicating `ITrackKeyFramer::compile()` (the serialized RangeBegin/RangeEnd are
  only authoritative when `RangeLock` is false, which isn't the common case). Looping past the
  last key blends explicitly back to the first one (`dateNext == loopEnd`), per
  `ITrackKeyFramer::eval()` -- a genuinely different loop behavior from the sampled tracks
  above, not an inconsistency.

Validated via the bridge on the same two real files as Step 1: a looping 8.1s Bip01 rotation
track gives near-identical quaternions at t=0.0 and t=8.1 (confirms the wrap), a mid-clip time
gives a properly normalized interpolated quaternion, and a material UV-scroll float track
interpolates linearly as expected.

## 2026-08-17 — ✨ Add .anim (CAnimation) read support to pynel

New module `nel/tools/pynel/pynel/ryzom_animation.py` (self-contained, own `_Reader`/
`_CLASS_PARSERS` like `ryzom_ig.py`, not sharing `ryzom_shape.py`'s -- matches the existing
per-format module convention). CLI: `ryzom-anim dump FILE.anim`.

Format reverse-engineered from `nel/src/3d/animation.cpp` (`CAnimation::serial`) and the
`serial()` of every track kind it can hold. The 8-byte magic (`NELID("_LEN")` then
`NELID("MINA")`) turned out to serialize on disk as the plainly readable ASCII string
`"NEL_ANIM"` -- confirmed directly against a real `ryzom-data` `.anim` file's header bytes
before writing a single line of parsing code, rather than trusting the macro's byte order in
the abstract.

`CAnimation` itself: name, a `{track name -> index}` map (e.g. `"Bip01 Head.rotquat"`), and a
poly-ptr vector of tracks -- same class-name-dispatch mechanism `ryzom_shape.py` already uses
for shapes, duplicated here rather than imported (see above). Six track classes implemented,
matching what two real files actually contain (a 64-track Bip01 dance animation and a
3-track material UV-scroll animation, both parse cleanly end to end via the bridge):

- `CTrackDefaultVector`/`CTrackDefaultQuat`: a single constant value for the whole clip.
- `CTrackSampledVector`/`CTrackSampledQuat`: real keyframed data, pre-sampled at a fixed rate
  and organized into `CTimeBlock`s for fast time lookup. Rotations are additionally quantized
  to `sint16` via `CQuatPack` (9 bytes/key instead of 16) -- unpacked back to plain
  `Quaternion` at parse time (divide by 32767, renormalize, mirroring `CQuatPack::unpack`
  exactly) rather than exposing the compressed form, so callers never need to know packing was
  involved.
- `CTrackKeyFramerLinearVector`/`Quat`/`Float`: **not in the original plan** (which only
  anticipated the two kinds above) -- discovered by running the parser against real files and
  hitting `unsupported track class` errors twice, each time cross-checked against
  `track_keyframer.h`/`key.h` before adding support. Explicit irregularly-spaced
  `(time, value)` keyframes (a `std::map<float, CKeyT>`), structurally simpler than the
  sampled kind (no compression, no TimeBlocks) but a genuinely different on-disk shape,
  captured as `Track.keyframes` (`List[Keyframe]`) rather than reusing the sampled kind's
  `Track.keys` field.

Read-only, no writer -- not needed for this chantier's goal (Forgery previewing an object
attached to an animated bone), and NeL's own `CAnimation` writing side wasn't audited.

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

## 2026-08-31 — ✨ Add CMeshMRMSkinned geometry writer to pynel, pynel 0.8.0

`dumps()`/`save_shape()` previously only reconstructed `CMeshBase` (materials) for
`CMeshMRMSkinned` -- the geometry was always copied back byte-for-byte from the raw
bytes captured at parse time, so editing `geom.packed_vertices` (or anything else in
the parsed geometry) had no effect on the written file. This is the format every
character customization piece in `characters_shapes.bnp` uses
(hairstyles/clothing/accessories), so it blocked any tool wanting to actually edit one.

Added `_write_packed_vertex()`, `_write_mrm_skinned_rdr_pass()`,
`_write_mesh_mrm_skinned_lod()`, `_write_mesh_mrm_skinned_geom()`,
`_write_mesh_mrm_skinned()`, wired into `dumps()` as `CMeshMRMSkinned`'s own branch
(`CMeshMRM`/`CMeshMultiLod` keep the old raw-copy fallback, untouched). Two
sub-structures that were previously read and silently discarded --
`CShadowSkin` (`_skip_shadow_skin()`, a simplified single-bone-rigid-skinned
shadow-casting proxy mesh, not the real geometry -- see
`nel/include/nel/3d/shadow_skin.h`) and each lod's trailing
`MatrixInfluences`/`InfluencedVertices[4]` -- are now captured as opaque raw byte
spans (`MeshMRMSkinnedGeom._raw_shadow_skin`, `MeshMRMSkinnedLod._raw_matrix_influences`)
and re-emitted verbatim, rather than modeled field-by-field (nothing needs to edit
them). Every version byte in this whole geometry subtree turned out to be `0` on real
data (probed via temporary debug prints, removed again), so no per-version branching
was needed in the new writer.

Also added `PackedVertex.with_pos(pos, decompact_scale)`, the inverse of
`decompact_pos()` -- compacts a float position back to the format's `int16`
representation, raising `ShapeWriteError` if a component doesn't fit.

Validated: geometry-only round-trip (parse a real `.shape`, re-dump with zero edits,
compare against the captured raw bytes) is byte-exact on two real files
(`fy_hof_cheveux_shave01.shape`, `tr_hof_cheveux_shave01.shape`). Editing positions and
writing them back necessarily loses a little precision from the `int16` compaction --
observed gaps after a real edit-then-reparse round-trip were within the format's own
worst-case quantization bound (`sqrt(3) * decompact_scale/2`), not a bug.

Also added `nel/tools/pynel/docs/shape_format.md` -- the `.shape` format had no
dedicated doc despite being the most complex parser in the project (every other
format module has one, e.g. `pacs_format.md`, `packed_sheets.md`); this covers the
container structure, per-class read/write coverage, the `CMeshMRMSkinned` compacted
vertex format, and the `CShadowSkin` explanation above in one place.
