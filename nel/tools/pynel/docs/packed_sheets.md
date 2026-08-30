# `.packed_sheets` (`creature.packed_sheets` first) — investigation notes

Status: **`creature.packed_sheets` implemented** (`pynel/ryzom_packed_sheets.py`,
CLI `ryzom-packed-sheets`), validated against the real file (28545 entries,
consumed with no trailing bytes, no exception). Sample files used for this
investigation: `~/.local/share/Ryzom/ryzom_live/data/creature.packed_sheets`
and `sheet_id.bin` (inside `~/.local/share/Ryzom/ryzom_live/data/leveldesign.bnp`).

## Where the actual `.shape` comes from (NOT in `.creature`)

Investigated while dumping real entries (2026-08-30) — worth recording since
it's a natural next question and easy to get wrong:

- `CAutomatonListSheet` (`automaton_list.packed_sheets`) is **not** a shape
  table — it's an animation state machine (`CAutomatonStateSheet`: per-state
  transitions like `NextState`, `OnMoveForward`, `OnAtk`, breakability
  conditions). No shape field anywhere in it. Don't reach for it to resolve
  a creature's mesh.
- The real path (`ryzom/client/src/character_cl.cpp`,
  `CCharacterCL::shapeFromItem()`/`createItemInstance()`): for each non-empty
  `CharacterSheet.<slot>.id_item` (`Body`, `Legs`, `Arms`, `Hands`, `Feet`,
  `Head`, `Face`, `ObjectInRightHand`, `ObjectInLeftHand`), that string is an
  **`.item`/`.sitem` sheet name**, not a shape name directly — the real
  `.shape` filename is read from *that* sheet via `CItemSheet::getShape()`
  (with race/gender variants: `getShapeFyros()`, `getShapeMatisFemale()`,
  etc.) — confirmed on `fyhc1.creature` (a dressed NPC: `body` →
  `fy_civil01_gilet.item`, `legs` → `fy_civil01_pantabotte.item`, etc.).
- For a NPC with **all equipment slots empty** (confirmed on
  `basic_fyros_male.creature`), no shape is referenced anywhere in
  `creature.packed_sheets` at all — the base/naked body mesh must come from
  some other convention (possibly baked per-skeleton, or resolved from
  `id_anim_set_base_name`/`id_automaton` some other way) that hasn't been
  traced yet. Flagged here as an open question, not solved.
- **Next step, not started**: implement `item.packed_sheets`
  (`CItemSheet`, `ryzom/client/src/client_sheets/item_sheet.{h,cpp}`) the
  same way this doc covers `CCharacterSheet`, so `Equipment.id_item` can be
  resolved to a real `.shape` filename. Separate investigation/session, same
  pattern as this doc.

## What a `.packed_sheets` file is

A binary cache the client (and other services) build from a set of Georges
sheets (`.creature`, `.item`, `.sbrick`, ...) so they don't have to re-parse
XML on every launch — see `nel/include/nel/georges/load_form.h`'s
`loadForm()`. One `.packed_sheets` file per sheet extension
(`creature.packed_sheets`, `item.packed_sheets`, ...), built by
`ryzom/client/src/sheet_manager.cpp` (`CSheetManager::loadAllSheet`, table
`TypeVersion[]`).

**First target: `creature.packed_sheets` only** (`CCharacterSheet`, the
`FAUNA` entity type). Other sheet types (`item`, `sbrick`, `mission`, ...)
are out of scope for the first version — see "Other sheet types" below.

## Header format (`load_form.h::loadForm`)

```
magic            u32   NELID("PKSH") = b"PKSH" reversed on little-endian, PACKED_SHEET_HEADER
version          u32   PACKED_SHEET_VERSION = 5   (serialCheck: must match exactly)
stream version   ver   IStream::serialVersion(0) — 1 byte, or 0xFF + u32 if >= 0xFF
dependBlockSize  u32   byte size of the two blocks below (dictionary + dependency map)
dictionary       cont<string>       source .creature filenames used to build this cache
dependencies     u32 count, then for each:
                   CSheetId sheetId (u32)
                   cont<u32> dates  (one date per dictionary entry that sheet's parent-chain touches)
nbEntries        u32   number of sheets in the container
ver              u32   must equal the sheet class's getVersion() (17 for creature, see below)
container        map<CSheetId, CSheetManagerEntry>   the actual payload (serialCont on a std::map)
```

The dictionary/dependency blocks exist purely so the client can decide
whether to rebuild the cache (compare stored mtimes against real files) —
**irrelevant for a read-only pynel parser**, just skip over them. A pure
reader only needs: check magic+version, skip dictionary+dependencies (or
parse them for informational purposes), check `nbEntries`/`ver`, then read
the map.

`serialCont` on a `std::map` (and on `std::vector`): `sint32` count, then
each element in order (for a map: key then value, not a length-prefixed
pair struct).

## `CSheetId` (`nel/include/nel/misc/sheet_id.h`, `.cpp`)

On the wire, a `CSheetId` is **just a raw `u32`**:

```
bits 31..24 (8 bits, NL_SHEET_ID_TYPE_BITS)  Type   — index into a per-run file-extension table
bits 23..0  (24 bits, NL_SHEET_ID_ID_BITS)   Id     — short id, unique within Type
```

This id is **not self-describing** — turning it into a readable sheet name
(`"ge_bear_c1.creature"`) requires `sheet_id.bin`, a separate file (see
below). Without it, entries are only identifiable by raw `(type, id)` pair.

Confirmed (per user decision, 2026-08-30): **`sheet_id.bin` is in scope for
the first version** — the module should resolve `CSheetId` to real sheet
names, not leave raw ids.

## `sheet_id.bin` format (`CSheetId::loadSheetId()` in `sheet_id.cpp`)

No header, no magic — it's a single `serialCont` on
`std::map<uint32, std::string>` (sheet id → full filename with extension,
e.g. `"ge_bear_c1.creature"`), read as `sint32` count then repeated
`(u32 key, string value)` pairs, same as any other `serialCont` map.
Location: **not a loose file** — confirmed (2026-08-30) it ships inside
`leveldesign.bnp` (e.g.
`~/.local/share/Ryzom/ryzom_live/data/leveldesign.bnp`), read via
`ryzom_bnp.BnpReader(path).read_file("sheet_id.bin")` (pynel already has
`.bnp` read support, `pynel/ryzom_bnp.py`). The parser function should just
take raw `bytes`, not a path — let the caller decide whether that came from
a loose file or a `.bnp` entry.

Deriving `Type`/`getSheetType()` from a decoded name: `CFile::getExtension()`
of the string value, mapped dynamically per-file (the type→extension table is
built by whichever process wrote the file, not a fixed global table) — for a
read-only parser, it's simpler to just index `sheet_id.bin`'s map directly by
the raw `u32` id found in `creature.packed_sheets` and use the string value
as-is; no need to separately decode Type/Id bit-fields once you have the
direct id→name map.

## `CSheetManagerEntry::serial` (map value, `ryzom/client/src/sheet_manager.cpp`)

```
type       sint32 (serialEnum)   CEntitySheet::TType — see enum in entity_sheet.h
                                  (FAUNA = 1; full order matters for decoding, copy from entity_sheet.h)
Id         u32                   CSheetId — same raw id already known from the map key,
                                  serialized again here (initSheet() does `pES->Id.serial(s)`)
<sheet>    (type-specific)       for FAUNA: CCharacterSheet::serial (below)
```

`nbEntries`/`ver` guard for creature is version **17**
(`TypeVersion[]` table: `CTypeVersion("creature", 17)`, sheet_manager.cpp
~line 100). If `ver != 17` in a real file, either the file is stale (client
will have rebuilt it against a newer sheet class) or this doc is out of
date — cross-check against `TypeVersion[]` and `CCharacterSheet::serial`
before trusting a mismatch as a bug in the parser.

## `CCharacterSheet::serial` (`ryzom/client/src/client_sheets/character_sheet.cpp:679`)

Flat, linear serial — no version branches inside it (version gating happens
once, at the container level via the `ver` check above). Field order (exact,
copy directly into the Python reader):

```
Gender                  u8
Race                    sint32 (serialEnum, EGSPD::CPeople::TPeople)
IdSkelFilename          string  (ClientSheetsStrings — plain inline string, see below)
IdAnimSetBaseName       string
IdAutomaton             string
Scale                   f32
SoundFamily             u32
SoundVariation          u32
IdLodCharacterName      string
LodCharacterDistance    f32
Selectable              bool (u8)
Talkable                bool
Attackable              bool
Givable                 bool
Mountable               bool
Turn                    bool
SelectableBySpace       bool
HLState                 sint32 (serialEnum, LHSTATE::TLHState)
CharacterScalePos       f32
NamePosZLow             f32
NamePosZNormal          f32
NamePosZHigh            f32
IdFame                  string
Body                    CEquipment   ×9, in this order:
Legs                    CEquipment     Body, Legs, Arms, Hands, Feet, Head,
Arms                    CEquipment     Face, ObjectInRightHand, ObjectInLeftHand
Hands                   CEquipment
Feet                    CEquipment
Head                    CEquipment
Face                    CEquipment
ObjectInRightHand       CEquipment
ObjectInLeftHand        CEquipment
HairColor               sint8
Skin                    sint8
EyesColor               sint8
DistToFront             f32
DistToBack              f32
DistToSide              f32
ColRadius               f32
ColHeight               f32
ColLength               f32
ColWidth                f32
MaxSpeed                f32
ClipRadius              f32
ClipHeight              f32
IdAlternativeClothes    cont<string>   (ClientSheetsStrings.serial(f, vector<TSStringId>&) — plain strings, not ids)
HairItemList            cont<CEquipment>
GroundFX                cont<CGroundFXSheet>
DisplayOSD              bool
IdStaticFX              string
BodyToBone              CBodyToBoneSheet
AttackLists             u32 size, then `size` × string   (manual loop, not serialCont — same wire shape though)
DisplayInRadar          bool
DisplayOSDName          bool
DisplayOSDBars          bool
DisplayOSDForceOver     bool
Traversable             bool
RegionForce             sint8
ForceLevel              sint8
Level                   u16
ProjectileCastRay       cont<CCastRay>
R2Npc                   bool
```

Sub-structures:

```
CEquipment (character_sheet.h):
    IdItem        string   (ClientSheetsStrings — item/shape name, lowercased)
    Texture       sint8
    Color         sint8
    IdBindPoint   string

CGroundFXSheet (ground_fx_sheet.cpp):
    GroundID      u32
    IdFXName      string

CBodyToBoneSheet (body_to_bone_sheet.cpp), all string, in this order:
    Head, Chest, LeftArm, RightArm, LeftHand, RightHand,
    LeftLeg, RightLeg, LeftFoot, RightFoot

CCastRay (character_sheet.h, nested class):
    Origin   vector3 (3×f32)
    Pos      vector3 (3×f32)
```

### `ClientSheetsStrings` (`NLMISC::CStaticStringMapper`)

Despite the name suggesting an interned/deduplicated table, **on disk it's
just a plain string per field** — `CStaticStringMapper::serial()`
(`nel/src/misc/string_mapper.cpp:230`) does `f.serial(tmp)` on a
`std::string` directly, no shared dictionary section, no id references to
resolve. Any field documented above as `string` is a normal
length-prefixed (`u32` length + raw bytes, `latin-1`) string — same
primitive pynel's `_Reader.string()`/`_Writer.string()` in `ryzom_ig.py`
already implements. **No special handling needed** — this table only exists
to deduplicate strings in the *client's* memory at runtime, not on disk.

## `CEntitySheet::TType` enum (`entity_sheet.h`, for decoding the `type` field)

Copy this exact order (it's the wire encoding, gaps included — `SPELL`,
`SPELL_LIST`, `CAST_FX` are obsolete but still occupy enum slots):

```
CHAR=0, FAUNA, FLORA, OBJECT, FX, BUILDING, ITEM, PLANT, MISSION,
RACE_STATS, PACT, LIGHT_CYCLE, WEATHER_SETUP, CONTINENT, WORLD,
WEATHER_FUNCTION_PARAMS, UNKNOWN, BOTCHAT, MISSION_ICON, SBRICK, SPHRASE,
SKILLS_TREE, UNBLOCK_TITLES, SUCCESS_TABLE, AUTOMATON_LIST,
ANIMATION_SET_LIST, SPELL, SPELL_LIST, CAST_FX, EMOT, ANIMATION_FX,
ID_TO_STRING_ARRAY, FORAGE_SOURCE, CREATURE_ATTACK, ANIMATION_FX_SET,
ATTACK_LIST, SKY, TEXT_EMOT, OUTPOST, OUTPOST_SQUAD, OUTPOST_BUILDING,
FACTION, TypeCount
```

For `creature.packed_sheets`, every entry's `type` is expected to be
`FAUNA` (=1) — worth asserting on read, and erroring clearly (not silently
misparsing) if some other type shows up in a real file, since only
`CCharacterSheet` is implemented in the first version.

## Other sheet types (deliberately out of scope for v1)

`CSheetManagerEntry::serial` (sheet_manager.cpp:290) is a big switch over
~25 `CEntitySheet` subclasses (`CItemSheet`, `CBuildingSheet`,
`CSBrickSheet`, ...), each its own class with its own `serial()`
(`ryzom/client/src/client_sheets/*.cpp`) and its own version number in
`TypeVersion[]`. Each one is a separate, similarly-sized investigation —
not attempted here. When picking up another `.packed_sheets` extension
later, the pattern to follow is exactly this doc: find the class in
`sheet_manager.cpp`'s `readGeorges`/`serial` switch, read its `serial()`
top to bottom, note its `TypeVersion[]` entry, write it up before coding.

## Suggested plan for implementation

1. Reuse `ryzom_ig.py`'s `_Reader`/`_Writer` pattern (already handles
   `u8`/`u32`/`s32`/`f32`/`boolean`/`string`/`version`/`check_magic`/
   `cont_len`) — no new primitives needed beyond what's already there,
   just a local copy or a shared helper if one gets extracted.
2. New module `ryzom_packed_sheets.py`:
   - `parse_sheet_id_bin(data) -> dict[int, str]` for `sheet_id.bin`.
   - dataclasses for `CharacterSheet` + `Equipment`/`GroundFX`/
     `BodyToBone`/`CastRay`.
   - `parse_creature_packed_sheets(data) -> dict[int, CharacterSheet]`
     (or keyed by resolved name when `sheet_id.bin` is supplied)
     following the header + `CSheetManagerEntry` + `CCharacterSheet::serial`
     layout above.
3. Read-only first (matches the `.cmb` precedent in `roadmap.md` — get
   parsing solid and tested before considering write/round-trip, which may
   not even be needed here since the client always regenerates this cache
   from source `.creature` sheets anyway).
4. Test against the real file at
   `~/.local/share/Ryzom/ryzom_live/data/creature.packed_sheets` (and its
   matching `sheet_id.bin`) — no synthetic fixture available for this
   format, so this *is* the round-trip check: parse must consume the whole
   file with no leftover bytes and no exception.
