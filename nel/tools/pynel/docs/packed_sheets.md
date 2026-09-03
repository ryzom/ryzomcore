# `.packed_sheets` — investigation notes

Status: **implemented** for `creature.packed_sheets`,
`item.packed_sheets`/`sitem.packed_sheets`, and `animset_list.packed_sheets`
(`pynel/ryzom_packed_sheets.py`, CLI `ryzom-packed-sheets`), validated
against real files: 28545 creature entries, 799 item entries, 8761 sitem
entries, all consumed with no trailing bytes and no exception; the
`Family`-keyed union in `CItemSheet` spot-checked across all 27
`ITEMFAMILY` values (decoded struct present exactly where the C++ switch
has a case, `None` everywhere else); 665 `CAnimationSetSheet` entries for
`animset_list.packed_sheets`, same no-trailing-bytes/no-exception bar, cross-
checked against the real client's own `computeAnimSet()` resolution for
`fy_hof_default__.animation_set`. Sample files used:
`~/.local/share/Ryzom/ryzom_live/data/{creature,item,sitem,animset_list}.packed_sheets`,
`sheet_id.bin` and `mode2animset.string_array` (both inside
`~/.local/share/Ryzom/ryzom_live/data/leveldesign.bnp`). Built for Patina's
Bind preview Mode picker ("pick an NPC's animation by its real game meaning
instead of a flat file list") -- see `nel/tools/forgery/docs/apps/object_editor.md`'s
own "Mode/animation" section.

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
- **Implemented**: `item.packed_sheets`/`sitem.packed_sheets` (`CItemSheet`,
  see the dedicated section below) — `Equipment.id_item` can now be resolved
  to a real `.shape` filename by looking up that name in `item_packed_sheets`'s
  entries (keyed by `sheet_id.bin`-resolved name) and reading `id_shape`/the
  race-gender variants. Confirmed end-to-end: `fyhc1.creature`'s `body` slot
  (`fy_civil01_gilet.item`) resolves to an `ItemSheet` whose `family=ARMOR`.

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

## `CItemSheet::serial` (`ryzom/client/src/client_sheets/item_sheet.cpp:681`), for `item.packed_sheets`/`sitem.packed_sheets`

Both the `item` and `sitem` extensions map to `CItemSheet`
(`sheet_manager.cpp`'s `readGeorges`: `extension == "sitem" || extension == "item"`),
sharing the same `TypeVersion[]` entry: version **44**. This is the class
that actually holds `.shape` filenames — see "Where the actual `.shape`
comes from" above; `CharacterSheet.<slot>.id_item` names one of these
sheets, and *this* sheet's `IdShape*` fields are the real mesh.

Flat, linear serial (no version branches beyond the one conditional switch
at the very end, gated on `Family`, itself a serialized field so decoding
is unambiguous). Field order:

```
IdShape                 string   (base shape; use this if no race/gender-specific one below applies)
IdShapeFemale           string
IdShapeFyros            string
IdShapeFyrosFemale      string
IdShapeMatis            string
IdShapeMatisFemale      string
IdShapeTryker           string
IdShapeTrykerFemale     string
IdShapeZorai            string
IdShapeZoraiFemale      string
SlotBF                  u64      bitfield, bit N set = usable in SLOTTYPE::TSlotType N
MapVariant               u32
Family                   sint32 (serialEnum, ITEMFAMILY::EItemFamily — see below, determines the union block at the end)
ItemType                 sint32 (serialEnum, ITEM_TYPE::TItemType)
IdIconMain               string
IdIconBack               string
IdIconOver               string
IdIconOver2              string
IconColor                rgba (4×u8)
IconBackColor            rgba
IconOverColor            rgba
IconOver2Color           rgba
IdIconText               string
IdAnimSet                string
Color                    sint8
HasFx                    bool
DropOrSell               bool
IsItemNoRent             bool
NeverHideWhenEquipped    bool
Stackable                u32
IsConsumable             bool
Bulk                     f32
EquipTime                u32
FX                       CItemFXSheet   (see below)
IdEffect1                string
IdEffect2                string
IdEffect3                string
IdEffect4                string
MpItemParts              cont<CMpItemPart>
CraftPlan                u32   (CSheetId, raw — resolve via sheet_id.bin like any other)
RequiredCharac            sint32 (serialEnum, CHARACTERISTICS::TCharacteristics)
RequiredCharacLevel      u16
RequiredSkill             sint32 (serialEnum, SKILLS::ESkills)
RequiredSkillLevel       u16
ItemOrigin                sint32 (serialEnum, ITEM_ORIGIN::EItemOrigin)
Scroll                   CScroll   (always present, regardless of Family — read unconditionally)
<union>                  present only if Family selects one of the cases below; absent (zero bytes) for
                          any Family not listed (incl. SCROLL, whose data already went out via Scroll above)
```

The trailing union, keyed on `Family` (values from `ITEMFAMILY::EItemFamily`,
`ryzom/common/src/game_share/item_family.h` — no gaps, plain sequential
auto-increment from `UNDEFINED=0`):

```
Family value        struct read       fields (all serialEnum unless noted)
COSMETIC             CCosmetic         VPValue: u32, Gender: enum
ARMOR                 CArmor            ArmorType: enum
MELEE_WEAPON          CMeleeWeapon      WeaponType: enum, Skill: enum, DamageType: enum, MeleeRange: sint32
RANGE_WEAPON          CRangeWeapon      WeaponType: enum, Skill: enum, RangeWeaponType: enum
AMMO                  CAmmo             Skill: enum, DamageType: enum, Magazine: sint32
RAW_MATERIAL           CMp               Ecosystem: enum, MpCategory: enum, HarvestSkill: enum, Family: enum,
                                         ItemPartBF: u64, UsedAsCraftRequirement: bool, MpColor: sint8, StatEnergy: u16
SHIELD                 CShield           ShieldType: enum
CRAFTING_TOOL/
HARVEST_TOOL/
TAMING_TOOL             CTool             Skill: enum, CraftingToolType: enum, CommandRange: sint32, MaxDonkey: sint32
GUILD_OPTION            CGuildOption      MoneyCost: u32, XPCost: sint32
PET_ANIMAL_TICKET       CPet              Slot: sint32
TELEPORT                CTeleport         Type: enum
CONSUMABLE              CConsumable       OverdoseTimer: u16, ConsumptionTime: u16, Properties: cont<string>
SCROLL                 (nothing — commented out in C++, Scroll field above already covers it)
any other value        (nothing)
```

`CMpItemPart` (element of `MpItemParts`):
```
OriginFilter   u8
Stats[34]      34 × u8   (RM_FABER_STAT_TYPE::NumRMStatType, static_assert'd == 34 in the C++ — if a future
                          file version bumps this, the version guard (44) will already have changed too)
```

`CScroll` (always present):
```
Texture       string
LuaCommand    string
WebCommand    string
Label         string
```

`CItemFXSheet` (`item_fx_sheet.cpp:76`, the `FX` field):
```
TrailMinSliceTime   f32
TrailMaxSliceTime   f32
AttackFXOffset      vector3
Trail               string   (private _Trail, but serialized as a plain string like everything else)
AdvantageFX         string
AttackFX             string
AttackFXRot         vector3
ImpactFXDelay        f32
StaticFXs           cont<CStaticFX>   (item_fx_sheet.cpp:122):
                       Name   string
                       Bone   string
                       Offset vector3
```

## `CAnimationSetListSheet::serial` (`ryzom/client/src/client_sheets/animation_set_list_sheet.cpp:726`), for `animset_list.packed_sheets`

Extension `animset_list` -> `TypeVersion[]` entry **25** (`sheet_manager.cpp`),
`CEntitySheet::TType` value `ANIMATION_SET_LIST` (=25 in the enum list above --
same header/`CSheetManagerEntry` wrapper as `.creature`/`.item`, only the
payload class differs). Investigated and implemented 2026-08-31 for "let
Patina pick an NPC's animation by its real game meaning (Mode/Behaviour)
instead of a flat file list" (Bind preview's Mode picker, see
`nel/tools/forgery/docs/apps/object_editor.md`) -- `parse_animation_set_list_packed_sheets()`/
`load_animation_set_list_packed_sheets()` in `ryzom_packed_sheets.py`,
validated against real data (665 `CAnimationSetSheet` entries, no trailing
bytes, no exception).

This is the format that ultimately maps `MBEHAV::EMode`/`MBEHAV::EBehaviour`
(`ryzom/common/src/game_share/mode_and_behaviour.h`) to real `.anim`
filenames -- see `character_cl.cpp`'s `computeAnimSet()` (calls
`::computeAnimSet()` in `misc.cpp:334`), which composes the animset *name*
to look up as `<AnimSetBaseName>_<mode2Anim(mode)>_<rightHandAnimSet>_
<leftHandAnimSet>` (or `<AnimSetBaseName>_default__` as a fallback), where
`AnimSetBaseName` comes from the creature/character sheet
(`CCharacterSheet.getAnimSetBaseName()`, already parsed by pynel's own
`CharacterSheet.id_anim_set_base_name`) and `mode2Anim()` (`misc.cpp:265`)
translates the `EMode` name to a name-fragment via a **separate** file,
`mode2animset.string_array` -- NOT part of this `.packed_sheets`, see its
own subsection below.

```
CAnimationSetListSheet (animation_set_list_sheet.cpp:726, the top-level packed entry):
  AnimSetList   cont<CAnimationSetSheet>

CAnimationSetSheet (animation_set_list_sheet.cpp:603):
  Name              string    -- e.g. "fyhc1_NORMAL__.animation_set" (see naming formula above)
  AnimationStates   cont<CAnimationStateSheet>   -- indexed by TAnimStateSheetId (see enum below), sparse: a
                                                     state absent from the source sheet is just missing from
                                                     the array (see CAnimationSetSheet::build()'s `resize` +
                                                     `AnimationStates[stateId].State == UnknownState` skip-
                                                     if-absent logic -- the array is NOT one entry per
                                                     TAnimStateSheetId value, only entries actually built get
                                                     a slot, indices beyond that padded with UnknownState==0
                                                     default-constructed ones from the `resize`)
  IsWalkEssential   bool
  IsRunEssential    bool

CAnimationStateSheet (animation_set_list_sheet.cpp:477):
  Animations              cont<CAnimationSheet>   -- one entry per available *variant* for this state (weighted
                                                       alternatives, see CAnimationSheet.Next/NextWeight below --
                                                       most states have exactly 1)
  State                   u16   (TAnimStateSheetId, see enum below)
  <state name>            string   -- write side re-derives this from State via getAnimationStateName();
                                       on READ it's a plain string field serialized right after State (NOT an
                                       index into it) -- pynel should just consume+discard it, or keep as a
                                       sanity cross-check against the State enum's own name table below
  IdLodCharacterAnimation TSStringId  (CStaticStringMapper -- see note below, serializes as a plain string)
  DisplayObjects          bool
  MeleeImpactDelay        f32

CAnimationSheet (animation_set_list_sheet.cpp:313):
  IdAnim                       TSStringId (CStaticStringMapper)  -- the real .anim filename, e.g. "fy_hom_normal_walk.anim"
  ApplyCharacterScalePosFactor bool
  IdFX                         TSStringId (CStaticStringMapper)  -- legacy single-fx name, superseded by FXSet below
  HeadControlable              bool
  VirtualRot                   f64
  FXSet                        CAnimationFXSetSheet (own CEntitySheet, see below -- out of scope for a first
                                                       anim-name-only pass, Patina doesn't render particle FX
                                                       at all today)
  Reverse                      bool
  HideAtEndAnim                bool
  Next                         cont<s8>     -- up to 7 alternative-animation indices (into this same
                                                CAnimationStateSheet.Animations array), chosen randomly at
                                                runtime for variety; -1 slots are skipped at build time, so
                                                this container only ever holds real (>=0) indices
  NextWeight                   cont<u16>    -- parallel to Next, relative pick weight for each alternative
  JobRestriction                u32
  RaceRestriction                s32  (serialEnum -- EGSPD::CPeople::TPeople, same enum pynel's CharacterSheet.race
                                        field already uses raw/unresolved)
```

`CStaticStringMapper::serial(f, id)` (`nel/src/misc/string_mapper.cpp:230`)
is NOT an indexed string-table reference despite the name -- on read it's
just `f.serial(std::string)` then `add()`s it to an in-memory dedup map; on
disk it's a plain length-prefixed string like any other, no separate lookup
table to parse. Every `TSStringId` field above ((`IdAnim`, `IdFX`,
`IdLodCharacterAnimation`) is byte-for-byte a plain pynel `f.string()`.

`CAnimationFXSetSheet`/`CAnimationFXSheet` (own `.cpp`s, `animation_fx_set_
sheet.cpp:59`/`animation_fx_sheet.cpp:95`) are themselves `CEntitySheet`
subclasses (their own `TypeVersion[]` entries: `animation_fx_set`=3,
`animation_fx`=4) -- only worth a full write-up if/when Patina ever renders
particle FX; not needed for the "pick the right `.anim` by Mode" goal, which
only needs `CAnimationSheet.IdAnim` and doesn't have to decode `FXSet` at
all (still needs to be *skipped* correctly byte-wise via `f.serialCont(FX)`
+ `2*MaxNumFX` bools -- `MaxNumFX=4` -- even without decoding its contents,
since it sits in the middle of `CAnimationSheet`'s own field order above).

### `TAnimStateSheetId` enum (`animation_set_list_sheet.h:108`, for decoding `CAnimationStateSheet.State`)

Copy this exact order (the wire encoding -- `StaticStateCount` is the
sentinel, not a real value):

```
Idle=0, Run, Walk, TurnLeft, TurnRight, Emote,
CastGoodBegin, CastGoodSuccess, CastGoodFail, CastGoodFumble,
CastBadBegin, CastBadSuccess, CastBadFail, CastBadFumble,
CastNeutralBegin, CastNeutralSuccess, CastNeutralFail, CastNeutralFumble,
OffensiveCastInit, OffensiveCastBegin, OffensiveCastLoop, OffensiveCastFail,
OffensiveCastFumble, OffensiveCastSuccess, OffensiveCastLink,
CurativeCastInit, CurativeCastBegin, CurativeCastLoop, CurativeCastFail,
CurativeCastFumble, CurativeCastSuccess, CurativeCastLink,
MixedCastInit, MixedCastBegin, MixedCastLoop, MixedCastFail,
MixedCastFumble, MixedCastSuccess, MixedCastLink,
AcidCastInit, BlindCastInit, ColdCastInit, ElecCastInit, FearCastInit,
FireCastInit, HealHPCastInit, MadCastInit, PoisonCastInit, RootCastInit,
RotCastInit, ShockCastInit, SleepCastInit, SlowCastInit, StunCastInit,
AcidCastLoop, BlindCastLoop, ColdCastLoop, ElecCastLoop, FearCastLoop,
FireCastLoop, HealHPCastLoop, MadCastLoop, PoisonCastLoop, RootCastLoop,
RotCastLoop, ShockCastLoop, SleepCastLoop, SlowCastLoop, StunCastLoop,
AcidCastFail, BlindCastFail, ColdCastFail, ElecCastFail, FearCastFail,
FireCastFail, HealHPCastFail, MadCastFail, PoisonCastFail, RootCastFail,
RotCastFail, ShockCastFail, SleepCastFail, SlowCastFail, StunCastFail,
AcidCastEnd, BlindCastEnd, ColdCastEnd, ElecCastEnd, FearCastEnd,
FireCastEnd, HealHPCastEnd, MadCastEnd, PoisonCastEnd, RootCastEnd,
RotCastEnd, ShockCastEnd, SleepCastEnd, SlowCastEnd, StunCastEnd,
DefaultAtkLow, DefaultAtkMiddle, DefaultAtkHigh,
PowerfulAtkLow, PowerfulAtkMiddle, PowerfulAtkHigh,
AreaAtkLow, AreaAtkMiddle, AreaAtkHigh,
Attack1, Attack2, FirstPersonAttack,
Impact, Death, DeathIdle,
LootInit, LootEnd, ProspectingInit, ProspectingEnd, CareInit, CareEnd,
UseInit, UseBegin, UseLoop, UseEnd,
StunBegin, StunLoop, StunEnd,
SitMode, SitEnd, StrafeLeft, StrafeRight,
StaticStateCount
```

For a first Patina implementation, only a handful of these actually matter
for "preview this NPC standing/walking/etc": `Idle`, `Run`, `Walk`,
`SitMode`, `Death`, plus whatever `MBEHAV::EMode` the user picks resolves an
animset name to in the first place -- most of the `*Cast*`/attack states are
only reachable via `MBEHAV::EBehaviour` (combat/magic), a separate, likely
later step.

### `mode2animset.string_array` -- NOT part of `.packed_sheets`

`mode2Anim()` (`ryzom/client/src/misc.cpp:265`) reads this file directly as
a raw Georges **form** at runtime, via `NLGEORGES::UFormLoader::loadForm()`
(`readStringArray()`, `misc.cpp:207`) -- a plain `{node name -> string
value}` map under a root `"array"` node. This is a genuinely different file
category from everything else in this doc: not compiled into any
`.packed_sheets`, not a `CEntitySheet` at all, loaded straight from the
Georges form tree. Maps each `MBEHAV::EMode` name (`"NORMAL"`, `"COMBAT"`,
`"SWIM"`, ...) to the animset name-fragment `computeAnimSet()`'s naming
formula uses. pynel has no Georges-form (as opposed to compiled
`.packed_sheets`) reader of any kind yet -- needs its own small investigation
of the raw `.string_array` file's on-disk shape (likely a simple
UTF-8/XML-ish Georges form, not NeL binary-serialized like everything else
in this doc) before it can be parsed.

## Other sheet types (deliberately out of scope beyond `.creature`/`.item`/`.sitem`/`.animset_list`)

`CSheetManagerEntry::serial` (sheet_manager.cpp:290) is a big switch over
~25 `CEntitySheet` subclasses (`CBuildingSheet`, `CSBrickSheet`, ...), each
its own class with its own `serial()` (`ryzom/client/src/client_sheets/*.cpp`)
and its own version number in `TypeVersion[]`. Each one is a separate,
similarly-sized investigation — not attempted here. When picking up another
`.packed_sheets` extension later, the pattern to follow is exactly this doc:
find the class in `sheet_manager.cpp`'s `readGeorges`/`serial` switch, read
its `serial()` top to bottom, note its `TypeVersion[]` entry, write it up
before coding.

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
