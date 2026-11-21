# Server tick and player position flow (GPMS)

Read from the code (2026-11-11); behaviours not measured are marked [HYPOTHESIS].

## The tick is global

- `ryzom/server/src/tick_service/tick_service.cpp` (`checkTockReceived`, `broadcastTick`): the tick service sends the next tick (`_GameCycle++`) to every registered service, then waits for their tocks. A service marked `Tocking` whose `TockMissingCount` exceeds its `Threshold` blocks the next tick for ALL services. Once the tocks are in, it still waits at least `_TickTimeStep` (0.1 s).
- Consequence: a service that is slow (e.g. the EGS spawning many entities) delays the tick of the whole shard, including the GPMS and players in other continents. The real duration of a tick is >= 100 ms and grows with the slowest service.
- `_GameTime += _GameTimeStep` (constant 0.1 s) at each tick whatever the real time: `getGameTimeStep()` is not a real duration.
- [HYPOTHESIS] Which services are `Tocking` and with which `Threshold` was not read.

## Path of a player position

1. Client sends `POSITION` once per estimated server tick; the frontend (`fe_receive_sub.cpp`) keeps action blocks whose `block.Cycle` > `LastReceivedGameCycle` and routes each with `block.Cycle`.
2. `cbClientPosition` (`gpm_service/client_messages.cpp`) is called per message, when the GPMS network loop processes it, and calls `CWorldPositionManager::movePlayer(player, x, y, z, heading, tick)` with `tick` = the cycle estimated by the client.
3. `CWorldPositionManager::update()` runs once per tick (`gpmsUpdate`). `TickRealTime` (speed budget) is set only there.

## Consequence for the speed budget

While the tick is stalled, `TickRealTime` does not move: every message processed meanwhile has `dt = 0` and the credit is not refilled, although real time passes. When the tick comes, `TickRealTime` jumps; the credit then refills but is capped by `SpeedBudgetMaxReserve` (2 s).

## Client side: how positions are sent (read 2026-11-11)

- `ryzom/client/src/main_loop.cpp` (~l.2368-2415): once per main loop pass, only if `NetMngr.getCurrentServerTick() > LastGameCycle`, the client pushes a `POSITION` generic action (`CUserEntity::sendToServer`, no motion condition), then `NetMngr.send(tick)` and `LastGameCycle = tick`.
- `CNetworkConnection::send(cycle)` (`network_connection.cpp` ~l.2665) stamps the last unstamped action block with `cycle` and sends a packet. `CNetworkConnection::push(CAction*)` (~l.2653) appends to the last unstamped block, so several pushes before a stamp share one block (one cycle).
- Blocks stay in `_Actions` until the server acknowledges them (`_LastReceivedAck`, ~l.1341) and `sendNormalMessage()` (~l.2038) puts every unacknowledged stamped block in each packet (up to ~480 bytes). If acks are late, each packet repeats old blocks.
- The client's tick estimate comes from the server packets it receives (`_CurrentServerTick = _Synchronize + _CurrentReceivedNumber*k`, ~l.1176-1179, 1366-1372). [HYPOTHESIS] If the server stops sending packets to a client, its tick stops and it sends no new `POSITION` (a silence of several seconds with regular GPMS ticks).
- Frontend (`fe_receive_sub.cpp` ~l.1295-1330): iterates over all blocks of a received packet in one pass (skipping cycles <= `LastReceivedGameCycle`) and routes each action to its service with `block.Cycle`. All positions of one packet therefore reach the GPMS in the same tick.
- Not explained: in the 2026-11-11 logs, bursts of 12-17 positions share the same `msgTick`, which means the same block cycle; how that many `POSITION` end up in one block was not found in this code.

### What the client uses as "server tick" (read 2026-11-11)

- `CNetworkConnection::getCurrentServerTick()` returns `_CurrentServerTick`, set in `network_connection.cpp` (~l.1366-1372) to `_Synchronize + _CurrentReceivedNumber`: the NUMBER of the last packet received from the server, not a clock. `POSITION` blocks are stamped with it (`main_loop.cpp` ~l.2368-2415).
- Consequence: if server packets stop reaching the client, its tick freezes and it stops producing action blocks, while its keepalive packets (`CNetworkConnection::send()`, no block) keep reaching the frontend (the frontend log shows no packet gap, only missing blocks). GPMS ticks stay regular.
- `CNetworkConnection::update()` (~l.2112-2150) also stops advancing client time when the frontend does not answer for more than 3 s with no data available.
- Measured 2026-11-11: when the stall ends, one block holds ~13-19 actions (13 `POSITION`, 1.2 m apart, i.e. 0.2 s of running each), instead of one. Only `main_loop.cpp:2387` pushes `POSITION`, once per change of `getCurrentServerTick()`; how 13 pushes land in one unstamped block was not found. A client log of each push (tick, `LastGameCycle`, size of the unstamped block, connection state) is needed.
