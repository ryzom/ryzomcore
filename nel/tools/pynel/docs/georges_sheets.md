# Georges sheets (`.creature`, `.item`, `.sitem`, `.sbrick`, ...) — investigation notes

Status: **not implemented**. This is a scoping/investigation doc, written
ahead of a dedicated session to add `ryzom_georges.py` to pynel — read this
first before starting that work, don't re-derive it from scratch.

## Why this doc exists

Sheets in `ryzom-private-data/game_element/` (creature, items, brick,
missions, etc.) use NeL's "Georges" form format, which has an **inheritance
system**: a sheet can declare a `PARENT`, and its actual, effective
property set is the result of walking up a chain of parent files and
merging. Two problems fall out of that for anyone editing sheets by hand:

1. You can't know a sheet's full effective property set by reading the
   file alone — you have to mentally trace the parent chain.
2. When a property has an unexpected value, it's not obvious *which* file
   in the chain set it, or whether the current file overrides it locally.

**Goal for the eventual tool** (stated by the project owner, 2026-08-06):
a sheet editor that shows, per property, exactly where its effective value
comes from in the parent chain — the same idea as a browser's CSS
DevTools "computed" panel, which shows which rule (and which stylesheet)
last set a given property, with overridden values struck through. This is
the actual design target for `ryzom_georges.py` — not just read/write, but
provenance tracking per field.

## Format overview (confirmed by reading real files)

Plain XML, one `<FORM>` per file:

```xml
<?xml version="1.0"?>
<FORM Revision="$Revision: 1.1 $" State="modified">
  <PARENT Filename="_tryker_female.creature"/>   <!-- optional -->
  <STRUCT>
    <STRUCT Name="Basics">
      <ATOM Name="Race" Value="tryker"/>
      <STRUCT Name="MovementSpeeds">
        <ATOM Name="WalkSpeed" Value="1.66"/>
      </STRUCT>
    </STRUCT>
    <ARRAY Name="HairItem">
      <STRUCT Name="long">
        <ATOM Name="Item" Value="tr_cheveux_long01.item"/>
      </STRUCT>
    </ARRAY>
  </STRUCT>
</FORM>
```

- `ATOM` — a leaf key/value pair.
- `STRUCT` — a named (or, at the top level, unnamed) group of child
  elements (`ATOM`/`STRUCT`/`ARRAY`).
- `ARRAY` — an ordered list of elements (commonly `STRUCT`).
- `PARENT Filename="..."` — the file this sheet inherits from, found by
  filename search (same NLMISC::CPath-style search-path mechanism as
  `.primitive`'s `LigoConfigFile`, `.ig`, etc. — not a relative path from
  the current file).

## Confirmed real inheritance chain (worth using as the test fixture)

```
creature/npc/testroom/testroom_trfc01c2.creature
  PARENT → _tryker_female.creature
creature/npc/parent/_basics_3d/_tryker_female.creature
  PARENT → _homin_attack.creature
creature/npc/parent/_basics_3d/_homin_attack.creature
  (no PARENT found in a quick check — verify whether it chains further,
  e.g. to _formuli.creature, before assuming this is the root)
```

Leading-underscore filenames (`_tryker_female.creature`,
`_formuli.creature`) are the convention for parent/base sheets not meant
to be used as leaf/instance sheets directly.

## Why this is a bigger job than `.primitive` (already done)

`.primitive` (see `ryzom_primitive.py`) is a flat XML tree with no
inheritance — read it, you have the whole picture. Georges forms are not:
the effective value of any given field can come from any ancestor in the
chain, and merging isn't a simple "child dict overrides parent dict":

- `nel/src/georges/form_elm.cpp` — **3107 lines**. Defines
  `CFormElmStruct`, `CFormElmVirtualStruct`, `CFormElmArray`,
  `CFormElmAtom` and the actual value-resolution/merge logic across the
  parent chain. This is the core algorithm to understand before writing a
  Python equivalent — don't guess at "child overrides parent" semantics
  without reading how arrays and virtual structs actually merge (arrays in
  particular are unlikely to be a naive "concat" or "replace").
- `nel/src/georges/form_dfn.cpp` — **909 lines**. `.dfn` files define the
  schema (structure/types) *per sheet type* (`.creature`, `.item`, ...),
  somewhat like `world_editor_classes.xml` does for primitive classes, but
  more tightly integrated into parsing itself — probably needed to know
  each field's declared type, not just infer it from the literal XML.
- `nel/include/nel/georges/{form,form_elm,form_dfn,type,header,load_form}.h`
  — the public headers or these; `form.h`'s `CForm`/`CForm::CParent`
  is the top-level parent-chain container.

## Suggested plan for the dedicated session

1. Read `form_elm.cpp`'s merge/resolution logic in full before writing
   any Python — specifically how `CFormElmStruct`/`CFormElmArray` combine
   a node with its `ParentNode` (search `ParentNode`, `ParentDfn`,
   `ParentIndex` in `form_elm.h`/`.cpp`).
2. Find and read a `.dfn` file (search `ryzom-private-data`/`ryzom-data`
   for `*.dfn`) alongside `form_dfn.cpp` to understand the schema format,
   since sheet TYPE (`.creature` vs `.item` vs ...) likely maps to a
   specific `.dfn`.
3. Design the Python data model around **provenance**, not just merged
   values — e.g. `EffectiveField(value, source_file, overridden_by)` —
   since that's the actual product goal, not an afterthought bolted onto
   a plain merge function.
4. Follow `ryzom_primitive.py`'s validated approach: implement, then test
   parse (and, once round-trip write is in scope, round-trip) against
   *all* real sheets under `ryzom-private-data/game_element/` — that
   directory has thousands of real `.creature`/`.item`/etc. files across
   many sheet types, a good breadth check the way the 1074-file primitive
   sweep was.

## Non-goals for a first version

- Writing/editing is a later step — get read + provenance resolution
  correct and tested first, the same order `ryzom_primitive.py` was *not*
  built in (that one did read+write together, but it's a much simpler
  format with no cross-file resolution to get wrong).
- No need to reimplement `.dfn`-based validation (rejecting
  schema-invalid edits) for a first version — that can come once basic
  provenance-aware read/write is solid.
