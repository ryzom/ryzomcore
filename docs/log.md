# Changelog

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

## 2026-08-08 — ⚡ Lower UpdateTimeout to reduce admin command callback latency

Admin commands sent via `query_shard()` (`nel/tools/pynel/pynel/admin_modules_itf.py`, same
pattern exists in the PHP equivalent) execute and log instantly on the EGS, but the
`wait_callback` round-trip back to the calling script consistently took ~450ms regardless of
the command (even an unknown command returned in the same ~450ms), pointing at transport/dispatch
overhead rather than command execution time.

Traced the command path: client -> RAS (`admin_service`) -> AES (`admin_executor_service`,
a separate process, not a module inside the EGS) -> EGS (`entities_game_service`) -> AES -> RAS
-> client. Each of these three services is a `NLNET::IService` subclass whose main loop caps
network polling/dispatch to its `UpdateTimeout` config variable (`nel/src/net/service.cpp`),
which defaults to 100ms and is only read once at startup (no live-reload, a service restart is
required for a config change to take effect).

Measured empirically on the live shard by lowering `UpdateTimeout` to 10 one service at a time
and restarting: RAS alone brought ~450ms down to ~260-330ms; adding EGS made no measurable
difference (masked by the AES, the actual bottleneck); adding AES brought it down to ~45-50ms,
roughly a 10x improvement overall.

Added `UpdateTimeout = 10;` to `ryzom/server/tools/cfg_creator/templates/admin_service.cfg`,
`admin_executor_service.cfg`, and `entities_game_service.cfg` so shards generated from these
templates get the lower latency by default. Kept all three set (not just the AES) since the
EGS's lack of measured impact was likely masked by the AES bottleneck rather than proven
negligible, and the cost of a lower `UpdateTimeout` (more frequent network polling) is
negligible.

## 2026-08-04 — 🔧 Modernize sheets_packer_shard's compile definitions to TARGET_COMPILE_DEFINITIONS

`ryzom/server/tools/sheets_packer_shard/CMakeLists.txt` used
`ADD_DEFINITIONS(-DNO_EGS_VARS)` / `ADD_DEFINITIONS(-DNO_AI_COMP)`
(directory-scoped, legacy). On `main/yubo-dev`, this target had already been
migrated to `TARGET_COMPILE_DEFINITIONS(sheets_packer_shard PRIVATE
NO_EGS_VARS DNO_AI_COMP)` — target-scoped, but with a typo
(`DNO_AI_COMP` instead of `NO_AI_COMP`) that silently broke the
`#ifndef NO_AI_COMP` guards in `ai_service/sheets.cpp`, causing
`sheets_packer_shard` to try linking against `CFightScriptCompReader`/
`CFightSelectFilter` (only defined in `ai_script_comp.cpp`, not part of this
target's sources) — `undefined reference` at link time. Applied the same
`TARGET_COMPILE_DEFINITIONS` modernization here on `fixes`, correctly
spelled, so this commit can be merged into `main/yubo-dev` to fix the typo
there via the merge rather than editing that branch directly.

## 2026-08-03 — 🐛 Fix MSVC operator< ambiguity for unqualified NUM_SKILLS in skill_manager.cpp

`skill_manager.cpp` has `using namespace SKILLS;` at file scope, so its two
`i < NUM_SKILLS` loop bounds use the unqualified name — the earlier sweeps
only searched for the qualified `SKILLS::NUM_SKILLS` form and missed these.
Cast both to `(uint)`.

Commit: 🐛 Fix operator< ambiguity for unqualified NUM_SKILLS

## 2026-08-03 — 🐛 Fix MSVC operator== ambiguity for sint skillValue vs SKILLS:: constants

Same family of issue, but this time against specific `SKILLS::` constants
(`SF`/`SM`/`SC`/`SH`) rather than `NUM_SKILLS` — `sint skillValue ==
SKILLS::SF` and friends, in `group_phrase_skill_filter.cpp` and
`group_skills.cpp` (8 sites total). Cast the enum side explicitly at each.
Left `itemSkill`/`compareSkill` comparisons against `SKILLS::` constants in
`sphrase_manager.cpp` untouched — those variables are themselves
`SKILLS::ESkills`, so same-enum-type comparisons, never ambiguous.

Commit: 🐛 Fix operator== ambiguity for skillValue vs SKILLS constants

## 2026-08-03 — 🐛 Fix MSVC operator< ambiguity for ITEM_TYPE::UNDEFINED comparisons

Same family of issue as the `NUM_*` enum-bound sweep below, this time for
`ITEM_TYPE::TItemType` in `bot_chat_page_trade.cpp`: `index<ITEM_TYPE::
UNDEFINED` (an `sint` compared against the enum bound) and a
`nlctassert(ITEM_TYPE::UNDEFINED<=128)` static assertion. Cast the enum
side explicitly at both sites.

Commit: 🐛 Fix operator< ambiguity for ITEM_TYPE::UNDEFINED

## 2026-08-02 — 🐛 Fix the same MSVC ambiguity for !=, ==, <=, >, >= against NUM_* enum bounds too

Follow-up to the `operator<`-only sweep below: the exact same MSVC
ambiguity (against unrelated `operator!=`/`operator==` overloads reachable
at that scope) also hits `!=`, `==`, `<=`, `>`, `>=` comparisons against the
same `NUM_*` enum bounds, in both operand orders (`x != SKILLS::NUM_SKILLS`
and `SKILLS::NUM_SKILLS == x`). Searched `ryzom/client`, `ryzom/common` and
`nel` for every remaining comparison operator against any of the same
7 enum bounds and cast the enum side explicitly, in both directions.

Commit: fix: resolve remaining MSVC-ambiguous !=/==/<=/>/>= comparisons against NUM_* enum bounds

## 2026-08-02 — 🐛 Fix MSVC operator< ambiguity for every NUM_* enum-bound loop/comparison in the client

Same family of issue as the two fixes below, but instead of patching one
call site at a time as each one surfaced during the Windows/MSVC
cross-build, searched `ryzom/client`, `ryzom/common` and `nel` for every
remaining `< SOMENAMESPACE::NUM_*` comparison against an enum bound
(`SKILLS::NUM_SKILLS`, `SCORES::NUM_SCORES`, `RM_FABER_TYPE::NUM_FABER_TYPE`,
`CHARACTERISTICS::NUM_CHARACTERISTICS`, `MAGICFX::NUM_SPELL_POWER`,
`JOBS::NUM_CAREER_DB_SLOTS`, `INVENTORIES::NUM_ALL_INVENTORY`) across 20
files and cast the enum bound explicitly at each site — MSVC treats these
as ambiguous against unrelated `operator<` overloads reachable at that
scope (e.g. `CProjectileBuild`, `CClientDate`, `CSessionId`, ...), GCC
doesn't. `ryzom/server` wasn't touched (not built for the Windows client
target, so not hit here, but the same fix would apply if anyone brings it
up under MSVC too).

Commit: fix: resolve remaining MSVC-ambiguous operator< comparisons against NUM_* enum bounds

## 2026-08-02 — 🐛 Fix another MSVC operator< ambiguity in action_handler_help.cpp

Same family of issue as the `CHARACTERISTICS` loop fix below: `for (skillNb = 0;
skillNb < SKILLS::NUM_SKILLS; ++skillNb)` in `action_handler_help.cpp`
(comparing `uint` against the `SKILLS::ESkills` enum) is ambiguous under MSVC
19.20 (encountered bringing up the Windows/MSVC cross-build) but not GCC.
Cast the loop bound to `uint` explicitly to remove the ambiguity.

Commit: fix: resolve another MSVC-ambiguous operator< comparison (action_handler_help.cpp)

## 2026-08-02 — 🐛 Fix ambiguous operator< on MSVC in CINCarac serialization loops

`CInCarac::serialBitMemStream()` (and its two neighbours) in `msg_client_server.h`
looped with `for (int i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i)`. GCC
resolves the `int < enum` comparison via the built-in `operator<` without issue, but
MSVC (19.20, encountered while bringing up a Windows/MSVC cross-build) reports it as
ambiguous against `operator<(const CSessionId&, uint32)` from `r2_basic_types.h`,
since both a user-defined conversion path and the built-in enum-to-int promotion are
considered equally viable candidates. Cast the loop bound to `int` explicitly to
remove the ambiguity — behavior is unchanged on every compiler.

Commit: fix: resolve MSVC-ambiguous operator< in CHARACTERISTICS loop bounds

## 2026-07-30 — 🐛 Stop despawning entity on client sheet change

`CBot::setClientSheet()` (used by the `setClientSheet()` AI script command) used to call
`sheetChanged()`, which fully despawns and respawns the bot on the AIS: a new `EntityId`
and mirror row are allocated, the world map link is redone, and EGS is notified of a
despawn/respawn — all just to update the purely cosmetic client-facing sheet. This threw
away the entity's live game state (aggro, current target, timers) even though the server
sheet, position and identity never actually changed. `setClientSheet()` now updates the
mirror's `Sheet` property directly on the existing entity instead, preserving its identity
and state; a real sheet/stat change (`setSheet()`) still goes through the full
despawn/respawn since the underlying creature type can change.

## 2026-07-30 — 🐛 Preserve entity state across client sheet swap

Following the server-side fix above, the client still reacted to any client-sheet update
by fully destroying and recreating the local `CEntityCL` for that slot (`remove()` then
`create()`), which made the entity disappear for a couple of seconds and, since
`remove()` calls `slotRemoved()` on every other entity, could permanently drop the
player's target/selection on that slot with no way to re-target it.

Added `CEntityManager::changeEntitySheet()` / `updatePendingSheetChanges()`: the
replacement entity is built off-screen first, then swapped into the same slot in a single
step (the slot is never left empty and `slotRemoved()` is never called), so any
target/selection on that slot survives the appearance change. If the slot turns out to
really be taken over by a different underlying server entity (different `DataSetIndex`),
the code still falls back to the previous full `remove()`+`create()` path.

Since the server has no reason to resend properties that didn't change just because the
sheet did, the replacement's position/orientation are copied from the old entity, and all
other per-slot visual properties (equipment/colors, mode/alive state, contextual
attackable/selectable bits, HP bars, target lists, guild, faction, pvp, mount/rider...)
are re-applied from the per-slot CDB right after the swap.

## 2026-07-30 — 🐛 Respect Turn sheet flag when facing target

`CCharacterCL::applyBehaviour()` and `CCharacterCL::beginCast()` unconditionally called
`dir()` to snap an entity's facing direction toward its combat/cast target, ignoring the
sheet's `Properties.Turn` flag (`_CanTurn`) even though the neighbouring `front()` call
already respected it. Since `dir()` (not `front()`) is what actually drives the rendered
orientation for these entities, an entity with `Turn=false` (e.g. a static decoration)
would still visibly snap to face the player as soon as it attacked or cast a spell. `dir()`
is now only called in these two spots when `_CanTurn` is true.
