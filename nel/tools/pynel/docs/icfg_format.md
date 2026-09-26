# `.icfg` — Ryzom client interface config format

Status: **implemented** (`pynel.ryzom_icfg`, `python -m pynel.ryzom_icfg dump`).
Source of truth: `ryzom/client/src/interface_v3/interface_manager.cpp`
(`CInterfaceManager::loadConfig()`/`saveConfig()`, `ICFG_STREAM_VERSION` 11)
and the sub-object serializers it calls, listed per block below.

`ryzom/client/data/gamedev/default/default_interface.icfg` is deprecated and
is not a reference for this format.

## Purpose

The client saves the whole in-game interface layout per character in
`save/interface_<character>.icfg` (and `save/shared_*.icfg` for shared
layouts): window positions/sizes/alpha for every desktop mode, the `UI:SAVE`
database branch, chat window filters, compass target, map zoom/offset, etc.

## API

```python
from pynel.ryzom_icfg import load_icfg, save_icfg, parse_icfg, dumps
cfg = load_icfg("interface_player.icfg")      # InterfaceConfig
save_icfg("interface_player.icfg", cfg)       # always written as version 11
```

- Every file version from 0 to 11 is read; writing always emits version 11
  and the latest version of every sub-object (same behaviour as the client).
- A version 11 file read then written back is byte-identical.
- `python -m pynel.ryzom_icfg dump <file.icfg>` prints the file as JSON
  (raw byte fields as hex).
- Round-trip check over a folder: `nel/tools/pynel/validate_icfg.py <folder>`.

## Encoding

NeL `IStream` little-endian encoding:

| Type | Encoding |
|---|---|
| `serialVersion` | `uint8`; `0xFF` means a `uint32` follows with the real value |
| `serialCheck(NELID("ABCD"))` | 4 bytes, the NELID reversed on disk (`"DCBA"`) |
| `bool`, `uint8` | 1 byte |
| `sint32`/`uint32`, `float` | 4 bytes |
| `sint64` | 8 bytes |
| `std::string` | `uint32` byte length + bytes (UTF-8) |
| `ucstring` | `uint32` character count + UTF-16 code units |
| `serialEnum` | `sint32` |
| `serialCont` | `sint32` element count + elements |
| `CVector2f` | 2 `float` |

## File layout

`ver` is the file version read first.

| # | Block | Condition | Serializer |
|---|---|---|---|
| 1 | `serialVersion(11)` | always | `interface_manager.cpp` |
| 2 | check `_ICU` + user chats | `ver >= 1` | `CPeopleInterraction::saveUserChatsInfos` |
| 3 | check `GFCI`, `uint32` mode count, `uint8` current mode | always | `interface_manager.cpp` |
| 4 | `sint32` last in-game screen W, H | `ver >= 10` | `interface_manager.cpp` |
| 5 | per mode: `uint32` length + desktop image | always | `CInterfaceConfig::CDesktopImage` |
| 6 | `uint32` `UI_DB_SAVE_VERSION` | `ver >= 9` | `interface_manager.cpp` |
| 7 | `UI:SAVE` database leaves | unless end of file | `CInterfaceConfig::dataBaseToStream` |
| 8 | task bar (deprecated, still serialized) | `ver >= 2` | `CTaskBarManager::serial` |
| 9 | user landmarks | always | `CContinentManager::serialUserLandMarks` |
| 10 | info window positions | `ver >= 5` | `CInterfaceHelp::serialInfoWindows` |
| 11 | macro memory slots | `ver >= 7` | `CSPhraseManager::serialMacroMemory` |
| 12 | ignored in-scene context help | `ver >= 8` | `CGroupInSceneBubbleManager::serialInSceneBubbleInfo` |
| 13 | dynamic chat channels | `ver >= 11` | `CPeopleInterraction::saveUserDynChatsInfos` |

Bytes after the last block are kept as-is (`InterfaceConfig.trailing`).

### User chats (block 2)

`serialVersion(2)`, check `TAHC`, then `bool present` + `CFilteredChatSummary`
for the chat group window and for each of the 5 user chats, check `TAHC`.
From version 1: `sint32` tab index, `bool present` + summary for the user
chat. From version 2: free tellers (`CChatGroupWindow::saveFreeTeller`) --
the client only writes them when the chat group window exists, so they are
absent when the next 4 bytes are the `GFCI` check.

`CFilteredChatSummary`: `serialVersion(2)`, check `USHC`, `bool` guild, team,
around me, tell, system info, `serialEnum` target, `bool` universe (v1+),
`bool` region (v2+).

Free tellers: `serialVersion(3)`, `uint32` count, one title per teller
(`ucstring` before version 3, `std::string` from version 3; version 1 also
stores an extra `uint32` and one `std::string` id per teller).

### Desktop image (block 5)

`serialVersion(2)` (absent for files of version 2 or older), `serialCont` of
container images (`CInterfaceConfig::SCont`), then optional extra data up to
the end of the buffer.

`SCont` version 10: `sint32` min W, max W, short id (the `ui:interface:`
prefix is stripped on disk and restored on read), `uint8` container mode.
Mode 0 stores the full container state (popup, opened, position, size,
active, scroll, alphas, popup geometry, lock/move flags, backup position,
touch flag); mode 1 stores only opened, active and active-savable. Older
versions store the full id and fewer fields; when upgraded, min/max W take
the `CGroupContainer` defaults (222/320).

Extra data: top window id (desktop version 1+), `uint32` count, then per
widget: `std::string` id, `uint32` chunk size, chunk.

### Widget config chunks

The file does not store the widget type: the client resolves the id to the
live widget built from the interface XML and calls its virtual
`serialConfig()`. Some ids are generated at runtime (chat windows from the
`chat_id` template, free tellers `free_chat_<name>` from
`contact_chat_friend`), so no exhaustive id table exists. pynel picks the
decoder as follows:

1. Known ids force the decoder: the three `...:map_content:actual_map`
   (`map`), `ui:interface:mp3_player` (`container_windows`),
   `ui:interface:compass` (`compas`).
2. Other ids: a decoder is accepted only if it consumes exactly the chunk
   size and its checks pass; exactly one match is required.
3. Otherwise the chunk is kept as raw bytes (`RawWidgetConfig`).

| Widget | Chunk |
|---|---|
| `CGroupMap` | `serialVersion(3)`, `float` user scale, `CVector2f` world offset (old offset before v2), `sint32` H before v3 |
| `CGroupContainerWindows` | `serialVersion(0)`, `bool` show desktops |
| `CGroupEditBox` | `serialVersion(1)`, input text (`ucstring` in v0), `sint32` cursor pos, `sint32` previous line count |
| `CGroupCompas` | `serialVersion(0)`, `CCompassTarget`: check `CTAR`, `serialVersion(1)`, `CVector2f` pos, name, `std::string` language, `serialEnum` type, check `_END` |

Compass target types: 0 North, 1 Selection, 2 Home, 3 Respawn,
4 ContinentLandMark, 5 UserLandMark, 6 PosTracker. A PosTracker target
serializes a polymorphic `CPositionState` pointer (`serialPolyPtr`); its
bytes (up to the end of the chunk, `_END` included) are kept raw in
`CompassConfig.pos_tracker_raw`.

### Other blocks

- `UI:SAVE` leaf (`SDBLeaf`): `serialVersion(1)`, `std::string` name,
  `sint64` value, `sint64` old value (v1+).
- Task bar: `serialVersion(0)`, 10 × 10 shortcuts of `serialVersion(0)`,
  `serialEnum` sheet type, `std::string` DB sheet, `sint32` macro id.
- Landmarks: `serialVersion(1)`, `uint32` continent count, per continent
  `std::string` name + `serialCont` of (`CVector2f` pos, `ucstring` title,
  `uint8` type). The current client writes an empty block (landmarks moved
  to `landmarks.xml`); pynel writes back whatever was read.
- Info windows: `serialVersion(0)`, `serialCont` of (`serialVersion(0)`,
  `sint32` X, `sint32` Y).
- Macro memory: `serialCont` of (`uint8` line, `uint8` slot, `uint32` macro id).
- Ignored context help: `serialCont` of `std::string`.
- Dynamic chats: `serialVersion(1)`, check `OMGY`, `bool present`, then
  `serialVersion(0)`, check `USHC`, 8 `bool` (one per dynamic channel).
