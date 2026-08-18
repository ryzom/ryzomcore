# Changelog

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
