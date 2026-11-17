# GPMS player speed check (`CWorldPositionManager::movePlayer`)

`ryzom/server/src/gpm_service/world_position_manager.cpp`. Position messages of a player come from the frontend (see `server-tick-and-player-position.md`); `movePlayer` limits how far the player moves per message.

## Speed budget

Each entity moved by a player (the mount has its own) keeps a `CWorldEntity::CSpeedBudget`:

- **Credit** (seconds of motion at the current speed limit). A message of distance `d` (x,y only, z is scaled with it) is accepted if `d <= credit * limit * SecuritySpeedFactor`, otherwise the move is cut to that distance, the credit goes to 0 and the client is corrected (`TP:CORRECT` beyond 1 m, logged `PlayerAbuse`). A null limit (sit, afk, stunned) gives a null distance.
- **Refill**: real time elapsed between two messages of the entity (arrival time, `CTime::getLocalTime()`), never the tick carried by the message (estimated by the client, identical for several messages) nor the GPMS tick alone (it stalls with the server).
- **Cap** = min(`SpeedBudgetMaxReserve`, `SpeedBudgetFloor` + moved time) + stall surplus. Moved time grows by `SpeedBudgetGrowth` per second of accepted motion; it goes back to 0 after `SpeedBudgetStillMessages` consecutive messages without motion, counted only for messages of a new game cycle (a burst of stale messages of the same tick says nothing about standing still). A silence of messages never resets it.
- **Stall surplus**: the tick is global (the tick service waits for all services), so a tick late by more than `SpeedBudgetStallThreshold` beyond the game time step is a stall of the whole server. `update()` adds such lateness to `StallTotal`; `movePlayer` also counts the lateness of the tick not yet arrived. The cap of an entity grows by the stall seen since its last message, at most `SpeedBudgetMaxStall`.
- **Reset** (credit = floor, moved time = 0): entity creation, `teleport()` (which `acquireControl`/`leaveControl` call).
- z is not covered by the budget; the 1 m correction threshold is unchanged.

## Variables (`gpms` service, changeable at runtime)

| Variable | Default | Meaning |
|---|---|---|
| `SpeedBudgetFloor` | 0.2 | seconds always allowed |
| `SpeedBudgetMaxReserve` | 2.0 | max seconds a moving player accumulates |
| `SpeedBudgetGrowth` | 1.0 | reserve gained per second of accepted motion |
| `SpeedBudgetStillMessages` | 3 | messages without motion before the reserve is reset |
| `SpeedBudgetStallThreshold` | 0.05 | lateness (s) of a tick counted as a server stall |
| `SpeedBudgetMaxStall` | 5.0 | max stall surplus (s) |
| `SpeedRunLogMinSeconds` | 60 | min length of a run for the `SpeedRun` log |

`SecuritySpeedFactor` (default 1.0) still scales the limit.

## `SpeedRun` log

After a run (messages with motion, ended by `SpeedBudgetStillMessages` still messages) of at least `SpeedRunLogMinSeconds`, if the distance asked exceeds what the speed limit alone allows, `nlinfo("SpeedRun ...")` gives entity, time, distance, average speed, average limit and the excess in %. A run where the budget cut a move is dropped (the asked distance is counted from a server position that did not follow).

## Known limits

- A client that stays silent more than the cap (about 2 s) and then sends its positions in one burst is cut and corrected, giving several seconds of rubber-banding. Measured under heavy server load: the client stops producing action blocks for 8-14 s (its tick is the number of the last server packet received) and then sends one block with ~13 positions. Accepted by design: it comes from the client, and a modified client could do anything.
- During a server stall a modified client can move up to `limit * SecuritySpeedFactor * stall` (at most `SpeedBudgetMaxStall`); stalls cannot be provoked by a client.
