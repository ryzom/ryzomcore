# Hairstyle/hat clipping

`ryzom/client/src/hairstyle_hat_clip.{h,cpp}` builds, per character and only when both
`SLOTTYPE::HAT_SLOT` and `SLOTTYPE::HEAD_SLOT` are equipped, a private (non-shape-bank-shared)
copy of the hairstyle mesh with the portion inside the hat's volume actually cut away —
a real geometric clip, computed once at equip time, not a shader/alpha trick.

## Trigger point

The computation is driven from `CPlayerCL::updateVisible()` (per-frame, via
`CPlayerCL::updateClippedHairInstance()`), not from `updateVisualPropertyVpa()` (a
property-change callback). At the point `updateVisualPropertyVpa` runs, the character's
skeleton is not yet fully configured for the frame: individual bone world matrices can
still be at their default identity value, and the character's own scale
(`_CharacterScalePos`, applied via `skeleton()->setScale()`) is not guaranteed to be
reflected in bone world matrices yet. `updateVisible()` runs after the character has been
fully built and after the frame's skeleton state is otherwise settled; the geometry is
still explicitly forced fresh via `USkeleton::forceComputeBone()` before use, since neither
call site runs after the scene's own animate/render pass.

## Coordinate spaces

A hairstyle's raw vertex data (as stored in its `.shape` file) is in the skeleton's
bind-pose / reference-pose space (a standing character at the world origin). A hat's raw
vertex data is in its own small local space near its own object pivot, since it's a rigid
"stick" object only ever positioned at render time by attaching it to a bone. These are
unrelated coordinate spaces; comparing raw vertex positions from each directly does not
work.

Both are brought into the skeleton's *current* world space before any geometric test:
the hat via the target bone's `CBone::getWorldMatrix()` (a single rigid transform, the
same one `USkeleton::stickObject()` uses for real rendering); the hairstyle via real
per-vertex weighted skinning (`CBone::getWorldMatrix() * CBoneBase::InvBindPos` per skin
weight, blended), since different hairstyle vertices can be weighted to different bones.

World-space coordinates on a large continent (tens of thousands of units from the map
origin) exceed a 32-bit float's precision budget for the sub-centimeter differences the
containment test needs — everything is recentered on the target bone's own world position
right after the transform, before any geometry math runs.

This world-space version of the geometry is used only to decide what's inside/outside and
to locate edge crossings; the mesh that actually gets built and skinned stays in the
hairstyle's original bind-pose space throughout, interpolated only by the blend factor
`t` found during the world-space search.

## Clipping volume: `<hatShapeName>_mask.shape`

A hat's actual rendered geometry is often thin/open (e.g. a flat brim ring plus a small
dome, no solid shell over the sides) — a hairstyle poking out past the brim doesn't
necessarily cross the hat's own surface at all, so testing containment against the
visible hat mesh itself under-detects what should be hidden.

`buildClippedHairstyleInstance()` first looks for `<hatShapeName without .shape>_mask.shape`
next to the hat shape (same lookup mechanism, so same search paths/.bnp). If found, that
shape — never rendered, purpose-built as a generously-oversized clipping volume, sharing
the hat's own local pivot/origin — is used as the containment volume instead of the real
hat geometry. If no mask exists, the real hat shape is used directly. Either way, the
actually displayed hat instance is completely unaffected.

## Containment test: generalized winding number

Containment is tested via the generalized winding number (signed solid angle sum over
the clipping volume's triangles, seen from the query point, divided by 4π): close to 1
inside, close to 0 outside. This is more robust than ray casting against a mesh that
isn't perfectly watertight (a common state for a hand-modeled clipping volume). The exact
same function is reused, via bisection, to locate where a hairstyle edge crosses the
clipping volume's surface — no explicit triangle-vs-mesh intersection math is needed.

## Clipping algorithm

For each hairstyle triangle, its 3 corners are walked in their original order; corners
outside the clipping volume are kept, and a new vertex is inserted wherever the edge
crosses the volume's surface (found via the bisection above). This produces a 0, 3- or
4-vertex polygon per triangle without needing separate "fully in / fully out / mixed"
cases, and preserves the original winding order (hence face normal, hence backface
culling) for every resulting sub-triangle. Skin weights for newly created vertices are
found by merging the two endpoint vertices' (bone, weight) pairs, weighted by the same
blend factor, keeping at most 4 bones (the format's limit), renormalized to sum to 1.

This assumes the clipping volume's surface crosses a given hairstyle triangle at most
once (a single entering + exiting edge pair) — true for the common case of a volume
silhouette cutting across an unfolded triangle. A triangle crossed by a more convoluted
intersection is only approximated.

## Source geometry extraction

Hairstyles in this codebase are `CMeshMRMSkinned`; hats are plain `CMesh` — sibling
classes (both derive from `CMeshBase`), with different geometry APIs, both supported.
Both sources are loaded independently of the shape bank (`CShapeStream` + `CIFile`,
mirroring `CShapeBank::load()`'s own file-loading code without the caching step), since
`CMeshGeom::retrieveVertices()`/`retrieveTriangles()` (used for the plain-`CMesh` case)
refuse to read an already-rendered ("resident") vertex/index buffer.

For `CMeshMRMSkinned`, the finest level of detail is `getNbLod() - 1`, not lod index `0`
(`CMeshMRMSkinnedGeom::chooseLod()` maps a "close up, maximum detail" view to the *last*
lod index, and index `0` to the coarsest). `CMeshMRMSkinnedGeom::getVertexBuffer()` and
`getSkinWeights()` return this raw packed vertex data as-is: for a given lod, wedge index
`i`, for `i < lod.geomorphs.size()`, is only a blend placeholder meant to be interpolated
with wedge `geomorphs[i].End` at render time for smooth LOD transitions — using it as-is
for a static render produces garbage attributes (this manifested as a wildly deformed
mesh with triangles stretching toward the world origin). Those wedges are resolved to
their `.End` target before use, the same approach `ryzom_forgery.shape_geometry`'s
`finest_skinned_lod()` (Forgery repo) takes for the same underlying file format.

## Output mesh and instantiation

The clipped geometry is rebuilt into a plain `CMesh` (`CMesh::CMeshBuild` +
`CMesh::build()`) — even when the source hairstyle was `CMeshMRMSkinned` — with no LOD of
its own. This is an accepted simplification: the resulting mesh is a private, per-character
instance, never shared or seen from a distance the way a real shared hairstyle asset is.

The mesh is instantiated directly (`CMesh::createInstance(CScene&)`, bypassing
`CShapeBank` entirely, obtained by bridging from the client's usual `NL3D::UScene*` handle
via `((CSceneUser*)Scene)->getScene()`) so it is never shared with any other character,
then bound to the skeleton normally (`USkeleton::bindSkin()`). Its materials are copied
as-is from the source hairstyle, but still need the same panoply colour-slot resolution
(`CColorSlotManager::setInstanceSlot()`, race skin tone / user colour / hair colour / eye
colour selecting into a shared multi-file texture) the normal equip path applies via
`CCharacterCL::applyColorSlot()` — otherwise the texture never resolves to an actual file.
The caller (`CPlayerCL::updateClippedHairInstance()`) replicates this using the values
already recorded on the normal (shared) hairstyle instance
(`_Instances[HEAD_SLOT].AC{Skin,User,Hair,Eyes}`).

The `CMesh*` behind the resulting instance is never registered anywhere else, so it is
cleaned up automatically (`CSmartPtr` reference counting, via the `CTransformShape::Shape`
member) once its one instance is deleted from the scene — `CPlayerCL` still deletes the
instance itself explicitly (`Scene->deleteInstance`) whenever superseded or on character
destruction, since nothing else in the entity lifecycle would otherwise do so for this
one, unlike every other equipment slot.

## Normal hairstyle instance visibility

While a clipped instance is active, the normal (shared) `_Instances[HEAD_SLOT]` instance
is hidden rather than unequipped, so the underlying equip/loading machinery for that slot
is untouched. Because that instance's `Loading` → `Current` promotion happens in a generic
per-frame loop (`CEntityCL::SInstanceCL::updateCurrentFromLoading()`,
`ryzom/client/src/entity_cl.cpp`) that can run independently of
`updateClippedHairInstance()`, a plain `.hide()` call is not always enough by itself (the
instance may still be `Loading` at that point). `SInstanceCL::KeepHiddenWhenLoaded` is set
whenever a clipped instance is active, so the normal instance comes up already hidden
whenever that promotion does happen, regardless of timing.
