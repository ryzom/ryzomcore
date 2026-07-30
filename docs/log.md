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
