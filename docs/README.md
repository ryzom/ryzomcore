# ryzom-core docs

- [hairstyle_hat_clip.md](hairstyle_hat_clip.md) — per-character clipping of a hairstyle
  mesh against an equipped hat's volume.

- [build-version-info.md](build-version-info.md): build-time generation of the client version string and build date, and when `macosx-build/build.sh` reconfigures.
- [chat.md](chat.md): chat sharing of items, actions, positions and macros, quotes, mentions, emoji and the chat window, from client request to IOS delivery.
- [client-pacs-primitives.md](client-pacs-primitives.md): lifecycle of the client PACS move container and of every `UMovePrimitive` handle held by client objects.
- [macos-build.md](macos-build.md): macOS client build scripts and the two-Mac `lipo` assembly pipeline (fv/steam).
- [crash-debug-logs.md](crash-debug-logs.md): the `nlcrashdbg` macro for temporary crash investigation logs, tagged with the crash group id so the crash report tool can pick them up in `client.log`.
- [needcheck-flag.md](needcheck-flag.md): `CPath::requestFileCheck`, the empty `needcheck.flag` file in the logs folder that tells the launcher to rescan the files when a required file is missing, and where it is called.
- [server-tick-and-player-position.md](server-tick-and-player-position.md): global tick (tick service tocks) and path of a client position to `movePlayer`.
- [gpms-player-speed-check.md](gpms-player-speed-check.md): speed budget of `CWorldPositionManager::movePlayer`, its variables, server-stall detection and known limits.
