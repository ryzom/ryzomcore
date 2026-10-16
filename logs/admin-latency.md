# Changelog

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
