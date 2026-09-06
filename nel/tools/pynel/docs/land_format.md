# `.land` format reference

Source of truth: `nel/include/nel/ligo/zone_region.h`,
`nel/src/ligo/zone_region.cpp` (`CZoneRegion::serial`, `SZoneUnit::serial`,
`SZoneUnit2::serial`). Investigated 2026-09-06 for
`project-todos/pynel/land_pipeline.md` step 1.

A `.land` is the top-level landscape composition file: a flat row-major grid
of "bricks", each brick being one ordinary `.zone` file (see
`zone_format.md`) placed at a grid cell with a position/rotation/flip. No
extra geometry lives in `.land` itself.

**On-disk encoding**: unlike `.zone`/`.zonew`/`.zonel` (raw binary),
`CZoneRegion::serial()` is generic and adapts to the stream type passed --
the real tool (`world_editor`) always calls it through `CIXml`/`COXml`, so
in practice `.land` on disk is always **plain XML**. `pynel.ryzom_land`
parses it with `xml.etree.ElementTree`.

## `CZoneRegion::serial()` (`zone_region.cpp:154-181`)

```
xmlPush ("LAND")
    version       serialVersion(1)             -- 0 (legacy) or 1 (current)
    CHECK         serialCheck(NELID("DNAL"))   -- fixed magic, verified on read, not real data
    MIN_X         sint32
    MIN_Y         sint32
    MAX_X         sint32
    MAX_Y         sint32
    if version == 1:
        VECTOR    cont<SZoneUnit2>             -- current, per-cell dates included
    if version == 0:
        VECTOR    cont<SZoneUnit>              -- legacy, upgraded to SZoneUnit2 in memory
                                                   (DateLow/DateHigh set to 0)
xmlPop ()
```

Grid size is `(MAX_X - MIN_X + 1) * (MAX_Y - MIN_Y + 1)` cells, row-major:
`index = (x - MIN_X) + (y - MIN_Y) * (1 + MAX_X - MIN_X)`
(`CZoneRegion::getName`/`getPosX`/etc., `zone_region.cpp:184-223`).

`CHECK`'s value is `NELID("DNAL")` = 1145979212 (the reversed-bytes
convention also used by `.zone`'s `"ZONE"`/`.packed_sheets`' `"PKSH"`
magics) -- serialized through `xmlSerial(uint32)` rather than raw bytes
like those, so it appears as a decimal number in the XML, not text.

## `SZoneUnit`/`SZoneUnit2` (one grid cell, `zone_region.h`/`.cpp:36-139`)

```
NAME           string    -- zone brick's bare name, or STRING_UNUSED for an empty cell
X              uint8     -- PosX
Y              uint8     -- PosY
ROT            uint8     -- 0-3, quarter turns
FLIP           uint8     -- 0/1
(4x, i = 0..3):
    MAT_NAMES  string    -- SharingMatNames[i], border-sharing metadata, or STRING_UNUSED
    CUR_EDGES  uint8     -- SharingCutEdges[i]
(SZoneUnit2 only, i.e. version == 1):
    LOW        uint32    -- DateLow, purely informative last-modified timestamp
    HIGH       uint32    -- DateHigh
```

`SZoneUnit2::serial` also writes its own inner sub-version byte
(`f.serialVersion(0)`, `zone_region.cpp:98`) ahead of these fields --
always `0` today, unrelated to the outer `LAND/VERSION`.

**`STRING_UNUSED` is `"< UNUSED >"`** (`zone_region.h:32`), NOT an empty
string -- this is `SZoneUnit`'s own default constructor value
(`zone_region.cpp:36-49`) and what real tooling actually writes for an
empty grid cell. Confirmed against real data
(`ryzom-data/leveldesign/landscape/primes_racines/bagne.land`). A genuinely
empty string (e.g. an unset `MAT_NAMES` entry with no sharing material) is
still possible and encoded as the self-closed `<S/>` tag -- `pynel.ryzom_land`
accepts both without erroring.

`STRING_OUT_OF_BOUND` (`"< OOB >"`, `zone_region.h:31`) is a **runtime-only**
sentinel returned by `CZoneRegion::getName()` for a coordinate outside
`[MinX..MaxX]x[MinY..MaxY]` -- it never appears on disk, not modeled in
`pynel.ryzom_land`.

## `pynel.ryzom_land`

`ryzom_land.py`: `ZoneRegion`/`ZoneUnit` dataclasses, `load_land()`/
`parse_land()` (read), `save_land()`/`build_land()` (write). Round-trip
only -- no geometric transformation. A v0 file is read and silently
upgraded to v1 on write (`DateLow`/`DateHigh` default to 0), same
convention as `ryzom_zone.py`'s version handling. `STRING_UNUSED` is
exported as `pynel.ryzom_land.STRING_UNUSED` for callers checking whether a
cell is populated.

Confirmed exhaustive against the real `.land` corpus in `ryzom-data`
(2026-09-06): 27 files found, all XML except two test/dev files never used
in-game (`jungle/testroom.land`, `jungle/sans_crevasse_matis.land`), which
are binary and explicitly out of scope.

Orchestrating `land_export`/`zone_elevation`/`zone_dependencies`/
`zone_ig_lighter` from a `.land` (the actual landscape build pipeline) is
covered separately in `docs/zone_tools.md` and
`project-todos/pynel/land_pipeline.md` -- not part of this format reference.
