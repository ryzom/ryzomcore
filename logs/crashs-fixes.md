# Crash fixes

## 2026-10-26 — 🐛 flushStringCache: do not throw on a write error

Crash found from a dump: a write error (disk full, locked file) while saving the
string cache threw out of `releaseMainLoop()`. The cache is only an
optimization, so it now logs a warning.

## 2026-10-26 — 🐛 CSourceXAudio2::updateState: handle a source without voice

Crash found from a dump (read at 0x0): a source marked as playing could have no
voice left (destroyed by `release()`). It is now marked as stopped.

## 2026-10-26 — 🐛 CDriverGL::setMode: do not divide by an empty mode size

Crash found from a dump: `setMode()` divided the relative cursor position by the
mode width and height, which divided by zero for an empty mode. It is now only
done when both are positive.

## 2026-10-26 — 🐛 CAudioMixerUser::initDevice: handle a mixer config that cannot be loaded

2 crash reports (read access violation at 0x0): `initDevice()` dereferenced the
form returned by `loadForm()` for `default.mixer_config`, which is NULL when the
file cannot be loaded. It now logs a warning and keeps the default
configuration (the file is optional).

## 2026-10-26 — 🐛 initLogin: an empty XMLLoginInterfaceFiles is a fatal error

The crash in `CDrawDraggedSheet::process()` on the login screen comes from a
client without the login interface: `initLogin()` only logged
"no xml login config files in client.cfg" and returned, so no pointer
view was created but the login loop kept drawing. It is now a `nlerror`, as
is an empty `XMLInterfaceFiles` in `initInGame()`, and a failed
`parseInterface()` in `initLogin()` and `initOutGame()` shows the existing XML
error message box.

## 2026-10-26 — 🐛 CDrawDraggedSheet: the widget pointer may not exist

3 crash reports on the login screen (read access violation at 0x141):
`CDrawDraggedSheet::process()` called `show()` on the pointer returned by
`CWidgetManager::getPointer()` without a NULL test. It now reads the pointer
once and skips drawing when there is none.

## 2026-10-26 — 🐛 CTextureFar: check the far bank tile size before reading it

15 crash reports (read access violation) in `CTextureFar::rebuildPatch()`,
probably from one or two players. The far bank tile pixels are read from a
file as 6 independent vectors, and `isFill()` only tests the first order,
while the pixels are read from order 1 or 2 for the smaller far textures.
The tile must now hold `sourceSize * sourceSize` pixels for the order in use,
otherwise a warning is logged and the layers of this tile are skipped. The
root cause of the bad tile is not confirmed.

## 2026-10-26 — 🐛 CGroupMap: validate the selected respawn point

Crash found from a dump: in the death / squad spawn map modes,
`CGroupMap::checkCoords()` indexed `_RespawnLM` with the value of
`UI:SAVE:RESPAWN_PT` (or `UI:TEMP:OUTPOST:SQUAD_RESPAWN_PT`) without checking
it against the list size. An out of range saved value now resets the
selection to the first respawn point and writes it back to the database, so
a point is always selected.

## 2026-10-26 — 🐛 CWidgetManager::checkCoords: ignore NULL windows

Crash found from a dump: `checkCoords()` dereferenced the entries of
`PrioritizedWindows` without a NULL test (the draw loop already had one, with
a TODO about NULL pointers in that list). Both loops of `checkCoords()` now
skip a NULL entry and `SMasterGroup::addWindow()` refuses a NULL window. A
dangling pointer (window destroyed without `delWindow`) is not covered.

## 2026-10-26 — 🐛 CBitmap::loadSize: validate sizes read from the file

Crash found from a dump (frames 0-7 missing): `loadSize()` is called on
downloaded web images by `CGroupHTML::ImageDownloadCB::finish()`. The DDS
branch allocated `new uint32[size]` with `size` read from the file, so a
corrupt header (size 0, below 4, or huge) wrote or read out of the array, or
threw `std::bad_alloc`. The DDS header size must now be in [16, 1024]. The JPG
branch also refuses a block size below 2, which made `seek(blockSize - 2)` go
backward forever.

## 2026-10-26 — 🐛 Help window: dangling source ctrl sheet

Crash found from a dump: `CInterfaceHelp::updateWindowSPhraseTexts()` called
`CDBCtrlSheet::getType()` on a destroyed control. `CInfoWindow::CtrlSheet`
(the sheet that opened an info window) was a raw pointer, never cleared if
that control was destroyed while the window stayed open. It is now a
`CRefPtr`, which becomes NULL by itself.

## 2026-10-06 — 🐛 Validate client-supplied values in EGS callbacks

Crash found from a dump: a `CLIENT:EXCHANGE:ADD` message with an invalid source
inventory (65535) made `CExchangeView::putItemInExchange()`
call `getItem()` on a NULL inventory (`CCharacter::getInventory()` returns NULL
for an unknown inventory). Hardened it and the other EGS callbacks that use
values read from client messages without checking them. `nlassert` does
nothing in release builds on Linux (`NLMISC_BREAKPOINT` is empty), so the code
went on with the bad value.

Exchange: `putItemInExchange()` and `putItemInFirstEmptyExchangeSlot()` check
the inventory, slot and exchange slot; `itemExchangeToBag()` checks the
exchange slot.

NULL pointers and out of range indexes: `sendItemInfos()` (guild and room
inventories) and `CGuild::getAndSyncItemInfoVersion()`, `buyPhraseByIndex()`,
`sellItem()`, `clientEventSetItemCustomText()` and its callback,
`cbClientPvPSetNeutralAllegianceGuild()`, `buildBuildingTradeList()`,
`CCharacterShoppingList::buyItem()` (faction points), `CRewardSharing`
(`userItemSelect()` for a user that is not a candidate, `userValidSelect()`
off by one), `cbClientMissionWake()` and `abandonMission()` (missing or invalid
template), `sendNpcMissionGiverIconDesc()` (missing creature),
`CR2GiveItem::giveItemGranted()` (character no more online),
`cbClientGuildSetGrade()` (grade >= `EndGuildGrade`).

Character creation: `checkCreateParams()` now refuses an unknown `People`
(`setStartStatistics()` dereferenced a NULL race sheet) and visual values that
do not fit the visual property bit fields (hair, colors, morph targets, eyes,
tattoo, gabarits). The official client only sends values inside these ranges.

`cbHarvestDeposit()` (`CLIENT:HARVEST:DEPOSIT`) returns early: the rest of the
function is dead code ending in `nlerror()`, which calls `abort()` on Linux.
The official client no longer sends this message, only a modified one could.

Also fixes `CTeamManager::joinLeagueDecline()`, which read the team invitor
instead of the league invitor when informing the players.

No debug/test steps: not reproducible on demand; validation relies on code-level
reasoning. Not compiled nor run when written.

## 2026-09-23 — 🐛 Clear client PACS primitive handles before releasing the container

`releasePACS()` (`ryzom/client/src/pacs_client.cpp`), the only place where the
client destroys its PACS move container, now first calls
`EntitiesMngr.removeCollision()` (entity `_Primitive` and user entity
`_CheckPrimitive`) and the new `CEntityManager::removeShapePrimitives()`
(the `Primitive` of every Lua shape instance, IG zone shapes included). No
client entity or shape can keep a handle on a primitive freed with its
container, whatever path leads to the release (`CContinent::select()` or
`CContinent::unselect()`).

Before this, IG zone shapes were skipped by `CEntityManager::removeInstances()`
in `CContinent::unselect()` and kept dangling primitive handles, later passed to
`removePrimitive()` of the next container (notably from
`CEntityManager::release()` on FarTP reselect). The ownership guard in
`CMoveContainer::removePrimitive()` only compares addresses, and primitives are
plain `new`/`delete` objects, so a freed address reused by a live primitive of
the new container would have let a stale handle remove it.

Entity primitives are recreated as before by `CEntityManager::changeContinent()`.
Shape primitives are not recreated: a shape surviving its container keeps its
3D instance without collision. Shape primitives of a hibernated container
(indoor switch) are intentionally left in place and remain covered only by the
`removePrimitive()` ownership guard.

No debug/test steps: the crash is not reproducible on demand; validation relies
on the code-level reasoning (single container destruction point). Documented in
`docs/client-pacs-primitives.md`.

## 2026-09-22 — 🐛 Fix PACS removePrimitive use-after-free crash on FarTP

PACS removePrimitive crash chantier (`fix_pacs_remove_primitive_crash.md`): the Windows client crashed with EXCEPTION_ACCESS_VIOLATION_READ in `NLPACS::CMoveContainer::removePrimitive` (`freePrimitive`/`delete`, move_container.cpp:1499) during a FarTP character reselect (stack: removePrimitive <- releaseMainLoopReselect release.cpp:257 <- CFarTP::disconnectFromPreviousShard far_tp.cpp:1169). `removePrimitive` never checked that the primitive belonged to the container before dereferencing and deleting it, so a dangling `UMovePrimitive` handle (primitive already freed by a previous container, released by `releasePACS()` on every continent switch) hit the unconditional walk + delete. `removePrimitive` now returns early when the primitive is not in `_PrimitiveSet` (pointer comparison only, no dereference), making removal idempotent and safe across container recreations; a `nlwarning` traces the ignored dangling handles (client entity/continent lifecycle cleanup stays a separate topic). Valid removal paths are unchanged.

## 2026-09-06 — 🐛 Fix Linux CPU affinity stack corruption and uninitialized main thread handle

While validating `zone_lighter`/`zone_welder` orchestration from pynel (`project-todos/pynel/zone_read_write.md` step 4), `zone_lighter` crashed on every invocation, right at its first line of `light()` (`nel/src/3d/zone_lighter.cpp:929`, `currentThread->getCPUMask()`), before reaching any lighting logic.

Two distinct bugs in `nel/src/misc/p_thread.cpp`, both Linux-only (the Windows equivalents in `win_thread.cpp` were never affected):

1. **Stack corruption in the CPU-affinity functions.** `CPThread::getCPUMask()`/`setCPUMask()` and `CPProcess::getCPUMask()`/`setCPUMask()` all called glibc's `pthread_getaffinity_np`/`pthread_setaffinity_np`/`sched_getaffinity`/`sched_setaffinity` with `sizeof(uint64)` (8 bytes) as the `cpu_set_t` size, while reinterpreting a local `uint64` as a `cpu_set_t*`. A real `cpu_set_t` on this system is 128 bytes (`CPU_SETSIZE` = 1024 bits) — glibc read/wrote past the 8-byte stack buffer believing it had a full `cpu_set_t`, corrupting the caller's stack. `CPThread::getCPUMask()`/`setCPUMask()` even carried a `nlwarning("This code does not work. May cause a segmentation fault...")` acknowledging the bug without ever fixing it. Fixed all four functions to use a real `cpu_set_t` (`CPU_ZERO`/`CPU_SET`/`CPU_ISSET`), converting to/from the existing `uint64` bitmask API for the first 64 CPUs.
2. **`CPThread::_ThreadHandle` never initialized.** Fixing bug 1 alone didn't stop the crash: the minidump still pointed inside `pthread_getaffinity_np` itself. `CPThread`'s constructor never sets `_ThreadHandle`, and `CPMainThread` (the static global wrapper representing the main thread, `p_thread.cpp:44`) is zero-initialized before its constructor runs — so `_ThreadHandle` was `0`, an invalid `pthread_t`, for every call made from the main thread. Fixed by explicitly setting `_ThreadHandle = pthread_self();` in `CPMainThread`'s constructor.

Validated end-to-end: rebuilt `zone_welder`/`zone_lighter` via `ryzom-docker`'s tools build, ran the full pipeline on a real `.zone` (welded, then lighted) — no crash, `Number of CPU used: 1` logged correctly, and the resulting `.zonel` parses successfully with `pynel.ryzom_zone.load_zone()`.
