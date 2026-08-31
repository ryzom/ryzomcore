# `.shape` format reference

Source of truth: `nel/src/3d/shape.cpp` (`CShapeStream`, the `"SHAP"` magic
and the polymorphic-pointer dispatch), plus the `serial()` methods of each
concrete `IShape` subclass -- see the per-class source files below. Reader/
writer implementation: `nel/tools/pynel/pynel/ryzom_shape.py`.

A `.shape` file holds one 3D mesh -- a character body part (`characters_shapes.bnp`,
e.g. `fy_hof_cheveux_shave01.shape`), a static prop, an animated skeleton, or
one of a handful of special-purpose shapes (water, flares, particle
systems). It's the format read by `nel/tools/forgery`'s Patina viewer/editor
and every 3D exporter in that suite.

## 1. Container structure

Every `.shape` file is: `"SHAP"` magic, a root node id (`uint64`), then one
polymorphic-pointer node -- a class-name string followed by that class's own
`serial()` payload. `pynel.ryzom_shape.parse_shape()`/`load_shape()` return a
`ShapeFile(type_name, value)`, where `value` is one of the dataclasses below.

## 2. Supported shape classes

| Class | Source | Read | Write |
|---|---|---|---|
| `CMesh` | `mesh.cpp`/`mesh_base.cpp` | yes | yes, fully editable |
| `CMeshMRM` | `mesh_mrm.cpp` | yes | materials only, geometry copied back byte-for-byte |
| `CMeshMRMSkinned` | `mesh_mrm_skinned.cpp` | yes | yes, fully editable (see §4) |
| `CMeshMultiLod` | `mesh_multi_lod.cpp` | yes | materials only, geometry copied back byte-for-byte |
| `CSkeletonShape` | `skeleton_shape.cpp`, `bone.cpp` | yes | read-only |
| `CFlareShape` | `flare_shape.cpp` | yes* | read-only |
| `CWaterShape` / `CWaveMakerShape` | `water_shape.cpp` | yes* | read-only |
| `CSegRemanenceShape` | `seg_remanence_shape.cpp` | yes | read-only |
| `CParticleSystemShape` | `particle_system_shape.cpp` | yes | read-only |

\* `CWaterShape`/`CFlareShape` almost always reference texture classes this
reader can't decode (see §3), so full parsing of those two is mostly
theoretical in practice -- the type is still identified before the error.

Shared building blocks (`CMaterial`, `CVertexBuffer`, `CIndexBuffer`,
`CMatrix`, `CTrackDefault*`, `ITexture`) were verified against
`material.cpp`, `vertex_buffer.cpp`, `index_buffer.cpp`, `matrix.cpp`,
`track.h` and `texture_file.cpp`.

Reading also covers `CTextureFile`/`CTextureMultiFile`/`CTextureCube`, the
`CMeshVPWindTree`/`CMeshVPPerPixelLight` vertex programs, and the optional
`CLodCharacterTexture` field.

## 3. Known limitations

- Other, less common `ITexture` subclasses (`CTextureBump`, procedural
  textures...) are not decodable -- fails with a clear `ShapeParseError`
  rather than producing wrong data.
- `CTextureFile`/`CTextureMultiFile` file names are lower-cased on read
  (texture names aren't case-sensitive, and real data mixes casing), so
  parsing a shape and immediately re-dumping it without any other edit can
  still change those names' case in the output -- the one point where this
  reader isn't byte-exact by construction.
- `dumps()`'s `CMeshBase` writer (`_write_mesh_base()`, shared by `CMesh`
  and `CMeshMRMSkinned`) always upgrades to `CMeshBase` version 10
  regardless of the source file's own version -- real data seen so far uses
  version 8. This means a full-file round-trip (parse, re-dump with zero
  edits) is **not** byte-exact even for the two "fully editable" classes,
  only their geometry is (see §4 for how that was actually verified for
  `CMeshMRMSkinned`).

## 4. `CMeshMRMSkinned` -- the character customization format

Every hairstyle/clothing/accessory piece in `characters_shapes.bnp` seen so
far is `CMeshMRMSkinned` (`MeshMRMSkinnedGeom` in `ryzom_shape.py`). Its
geometry is a **compacted vertex format** (`PackedVertex`), not the plain
float `CVertexBuffer` used by `CMesh`/`CMeshMRM`:

- `pos`: 3x `int16`, decompacted as `pos * geom.decompact_scale` (one shared
  float scale per mesh) -- `PackedVertex.decompact_pos()`/`with_pos()` (the
  latter is the inverse, added for [[pynel-mrmskinned-geometry-writer]], used
  to write edited positions back).
- `normal`: 3x `int16`, decompacted as `normal / 32767.0`.
- `uv`: 2x `int16`, decompacted as `uv / 8192.0`.
- `matrices`/`weights`: 4x `uint8` each -- standard 4-bone weighted skin.

Per-lod render data (`MeshMRMSkinnedLod`) references this shared
`packed_vertices` list via 16-bit indices in each `rdr_pass` (unlike
`CMesh`'s render passes, which use 32-bit indices into a `CIndexBuffer`
wrapper -- a different, simpler encoding, not shared code between the two
writers).

### `CShadowSkin` -- the shadow-casting proxy mesh

Trailing the packed vertex buffer, every `CMeshMRMSkinnedGeom` carries a
`CShadowSkin` section (`nel/include/nel/3d/shadow_skin.h`): a **separate,
simplified mesh** built at export time
(`CMeshGeom::buildShadowSkin()` in `mesh.cpp`) and used only to render the
character's shadow (`CShadowSkin::applySkin()` +
`CMeshMRMSkinnedInstance::renderShadowSkinGeom()`/`renderShadowSkinPrimitives()`)
-- never the real visible mesh.

Its vertices (`CShadowVertex`: a position plus a single `MatrixId`) use
**one-bone rigid skinning**, unlike the real mesh's 4-bone weighted skin --
a shadow doesn't need that precision, so this is a cheaper mesh to
transform every frame for shadow rendering.

pynel doesn't model `CShadowSkin` field-by-field: nothing needs to edit it,
so it's read as one opaque byte span
(`MeshMRMSkinnedGeom._raw_shadow_skin`) and re-emitted unchanged by the
writer. Same treatment for a smaller trailing block per lod,
`MatrixInfluences`/`InfluencedVertices[4]` (`MeshMRMSkinnedLod._raw_matrix_influences`)
-- also fully discarded by the original read-only parser, also nothing
pynel needs to edit.

**Known gap, not yet addressed** (see [[forgery-object-editor]]'s "FUTURE:
CShadowSkin investigation" note): editing `packed_vertices`' positions (e.g.
via `ryzom_forgery.shape_geometry.conform_hairstyle_boundary()`) leaves the
old `CShadowSkin` bytes untouched -- if a real geometry edit changes the
mesh's silhouette enough to matter, the shadow proxy would then be stale
relative to the edited mesh. Whether that's ever visually significant
hasn't been investigated.

### Round-trip fidelity, as actually verified

Full-file `dumps()` output is not byte-exact (see §3) -- but the *geometry*
this chantier's writer actually touches is: parsing
`fy_hof_cheveux_shave01.shape`/`tr_hof_cheveux_shave01.shape`, calling
`_write_mesh_mrm_skinned_geom()` directly and comparing against each file's
captured raw geometry bytes came back byte-exact on both.

Editing positions and writing them back necessarily loses a little
precision, from the `int16` compaction: for
`fy_hof_cheveux_shave01.shape` (`decompact_scale≈0.0002441`), the
worst-case 3D rounding error is `sqrt(3) * decompact_scale/2 ≈ 0.000211` --
an edited-then-reparsed position landing within that bound isn't a bug, it's
the format's own precision limit.

## 5. `CMesh` -- the fully-editable classic format

`CMesh`/`CMeshGeom` (`mesh.cpp`) is the simplest of the mesh classes: a
plain `CVertexBuffer` (float positions/normals/UVs, one channel per
attribute) plus per-material-block, per-matrix-block render passes with
32-bit indices. `dumps()` rewrites it field-by-field, so any part of its
geometry is freely editable -- this is the format `nel/tools/forgery`'s
`shape_import.py`/`shape_export.py` target when producing new shapes (e.g.
importing a `.dae`/`.fbx`), rather than `CMeshMRMSkinned`.

## Usage

```python
from pynel.ryzom_shape import load_shape, save_shape

shape = load_shape("box.shape")
print(shape.type_name, shape.value.num_vertices, shape.value.num_triangles)

save_shape("box_edited.shape", shape)  # only for the writable classes, see §2
```

CLI: `ryzom-shape dump <file>` (also `set-texture`, `add-variant` -- see the
module's `_build_arg_parser()` for the full command list).
