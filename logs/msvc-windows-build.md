# Changelog

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
