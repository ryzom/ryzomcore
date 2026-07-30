# Changelog

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
