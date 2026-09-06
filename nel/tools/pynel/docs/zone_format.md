# `.zone`/`.zonew`/`.zonel` format reference

Source of truth: `nel/src/3d/zone.cpp` (`CZone::serial`), `nel/src/3d/patch.cpp`
(`CPatch::serial`), and the `serial()` methods of every sub-structure listed
below. Investigated 2026-09-06 for `project-todos/pynel/zone_read_write.md`
(pynel read/write support), step 1.

All three extensions are the **same binary format** (`CZone::serial()`), at
different landscape-build pipeline stages: `.zone` = raw exported geometry,
`.zonew` = welded with neighbors, `.zonel` = lit (lumels + point lights
baked). One reader/writer covers all three.

**Convention confirmed**: `f.xmlSerial(v, "TAG")` / `f.xmlSerial(a,b,c,d,"TAG")`
reduce, in binary mode, to plain `f.serial(v)` / `f.serial(a,b,c,d)` --
`xmlPush`/`xmlPop` are no-op virtuals overridden only by an actual XML
stream (`nel/include/nel/misc/stream.h:730-781,920-927`). No tag bytes ever
land on disk for a binary `.zone` file. Every `"TAG"` argument below is
therefore irrelevant to the wire format and omitted.

Read-only scope note (pynel policy, `project-todos/pynel/zone_read_write.md`):
compressed `Lumels` are treated as an **opaque blob** (read back and
rewritten byte-for-byte, never decoded/recompressed) -- decoding the actual
lumel compression algorithm is out of scope for a round-trip reader/writer.

## `CZone::serial()` (`zone.cpp:438-506`)

Current version **5**, only version >= 3 supported (`throw EOlderStream` if
< 3 -- matches pynel's own scope per the chantier). Field order:

```
magic         serialCheck(NELID("ENOZ"))   -- on disk: "ZONE" (same reversed-bytes
                                               convention as "PKSH"/"HSKP")
ZoneId        u16
ZoneBB        CAABBoxExt                   -- see below
PatchBias     CVector (3xf32)
PatchScale    f32
NumVertices   sint32
BorderVertices  cont<CBorderVertex>        -- see below
Patchs          cont<CPatch>               -- NOT CPatchInfo (in-memory build struct,
                                               never serialized) -- see below
PatchConnects   cont<CZone::CPatchConnect> -- see below
(if ver >= 4):
    _PointLightArray  CPointLightNamedArray  -- see below
```

`CAABBoxExt::serial` (`aabbox.cpp:334`) just calls `CAABBox::serial`
(`aabbox.cpp:199`): `version(u8=0)` + `Center` (CVector, 3xf32) + `HalfSize`
(CVector, 3xf32) -- 25 bytes total. The bounding radius is recomputed at
load time (`updateRadius()`), not stored.

`CBorderVertex::serial()` (`zone.cpp:396-407`), version 0, per element:
```
version(u8=0)
CurrentVertex     u16
NeighborZoneId    u16
NeighborVertex    u16
```
7 bytes/element.

## `CPatch::serial()` (`patch.cpp:1579-1706`)

Current version **7** (`patch.cpp:52`), rejects ver < 2. Field order:

```
Vertices[4]       4x CVector3s          -- single multi-arg f.serial(v0,v1,v2,v3) call
Tangents[8]       8x CVector3s          -- two calls of 4 each: Tangents[0..3], Tangents[4..7]
Interiors[4]      4x CVector3s
Tiles             cont<CTileElement>    -- count == OrderS*OrderT, self-described by the
                                            container's own length prefix
(if ver >= 1):
    TileColors    (see version split below)
(if ver >= 2):
    OrderS        u8
    OrderT        u8
    CompressedLumels  cont<u8>          -- bulk vector<uint8> specialization (stream.cpp:331-353):
                                            sint32 count + raw byte blob, no per-element
                                            serialization. Treat as opaque (see policy note above).
(if ver >= 3):
    NoiseRotation      u8
    _CornerSmoothFlag  u8
(if ver >= 4):
    Flags         u8
(if ver >= 5):
    TileLightInfluences  cont<CTileLightInfluence>   -- see below
```

`CVector3s` (`patch.h:89-118`) -- a quantized vertex, packed/unpacked against
the parent `CZone`'s `PatchBias`/`PatchScale`:
```
x  sint16
y  sint16
z  sint16
```
(one `f.serial(x,y,z)` call per instance -- decode to world space via
`x*PatchScale + PatchBias.x` etc., matching `CVector3s::unpack()`.)

There is **no `CBezierPatch::serial()`** in the stored format -- these 16
`CVector3s` (4 corners + 8 tangents + 4 interiors) directly replace the
in-memory `CBezierPatch` that `CPatchInfo` (the build-time struct) uses.

`CTileElement::serial()` (`tile_element.cpp:76`), no version, per element:
```
Flags     u16
Tile[3]   3x u16
```
8 bytes/element.

`TileColors`, version-dependent:
- patch ver <= 6: reads `vector<CTileColorOldPatchVersion6>` (`Color565` u16
  + `lightX,lightY,lightZ` 3x u8 = 5 bytes/element), then copied in memory
  into `TileColors`.
- patch ver >= 7 (the real/current case): `cont<CTileColor>`, each element
  just `Color565` **u16** (2 bytes).

`CTileLightInfluence::serial()` (`tile_light_influence.cpp:61-67`), no
version, per element:
```
Light[0]            u8
Light[1]            u8
PackedLightFactor    u8
```
3 bytes/element. Absent from the stream entirely if patch ver < 5 (defaults
reconstructed in memory instead).

## `CZone::CPatchConnect::serial()` (`zone.cpp:408-422`)

Current version 1:
```
version(u8=1)
(if ver < 1): OldOrderS u8, OldOrderT u8, ErrorSize f32
(if ver >= 1, the real case): ErrorSize f32
BaseVertices[4]   4x u16
BindEdges[4]      4x CPatchInfo::CBindInfo   -- fixed array, 4 sequential calls,
                                                 NOT a cont<>
```

`CPatchInfo::CBindInfo::serial()` (`zone.cpp:423-435`), version 0, per
element (note: **does carry its own version byte**, easy to miss on a quick
read):
```
version(u8=0)
NPatchs     u8
ZoneId      u16
Next[4]     4x u16
Edge[4]     4x u8
```
16 bytes/element, x4 per `CPatchConnect`.

## `CPointLightNamedArray::serial()` (`point_light_named_array.cpp:162-182`)

Current version 1:
```
version(u8=1)
_PointLights   cont<CPointLightNamed>          -- see below
(if ver == 0, legacy, not expected in real files):
    map<string, CPointLightGroupV0>            -- StartId/EndId, 2x u32
(if ver >= 1, the real case):
    _PointLightGroupMap  cont<CPointLightGroup>
        version(u8=0)
        AnimationLight   string
        LightGroup       u32
        StartId          u32
        EndId            u32
```

`CPointLightNamed::serial()` (`point_light_named.cpp:67-88`), extends
`CPointLight`, current version 1:
```
version(u8=1)
CPointLight::serial(f)     -- parent class payload, see below
AnimatedLight              string
_DefaultAmbient            CRGBA (4x u8, R/G/B/A order)
_DefaultDiffuse            CRGBA
_DefaultSpecular           CRGBA
(if ver >= 1, the real case):
    LightGroup             u32
```

`CPointLight::serial()` (`point_light.cpp:232-270`), current version 2:
```
version(u8=2)
(if ver >= 2, the real case):
    _AddAmbientWithSun     bool (u8)
(if ver >= 1, the real case):
    _Type                  sint32 (serialEnum, TType: PointLight=0, SpotLight, AmbientLight)
    _SpotDirection         CVector (3xf32)
    _SpotAngleBegin        f32
    _SpotAngleEnd          f32
_Position       CVector (3xf32)
_Ambient        CRGBA
_Diffuse        CRGBA
_Specular       CRGBA
_AttenuationBegin   f32
_AttenuationEnd     f32
```

`CRGBA` -- confirmed R,G,B,A byte order (u8 each), same convention as
pynel's existing `Rgba` dataclass (`packed_sheets.md`'s `rgba()` primitive).

## Status: implemented and validated (2026-09-06)

`pynel.ryzom_zone.parse_zone()`/`load_zone()` (read-only) implement
everything above. Validated against all 5093 real `.zonel` files found in
every `*_zones.bnp` pack (`~/repos/live_data`, one pack per continent, e.g.
`matis_zones.bnp`) -- 5093/5093 parsed with no trailing bytes and no
exception, confirming: the `CZone`/`CPatch`/`CPatchConnect`/`CBindInfo`
layout, patch version 7 (the real/current case) for `TileColors`, and real
`_PointLightArray` data (non-empty `point_lights` observed on several
zones, e.g. 2-8 lights per zone on some `62_a*.zonel` entries).

Remaining open point: the lumel-compression block layout
(`NL_BLOCK_LUMEL_COMPRESSED_SIZE=8` bytes/tile-block, decode logic in
`patch_lightmap.cpp`) was deliberately **not** traced bit-for-bit -- pynel
treats it as an opaque blob (see policy note above), read back and
rewritten byte-for-byte. Only relevant if Forgery ever needs to visualize/
edit lumels directly, out of scope for this chantier.
