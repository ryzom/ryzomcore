# Changelog

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
