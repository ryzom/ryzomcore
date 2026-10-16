# Logger Service — Reading `.binlog` files

The logger service centralizes logs sent by every other service on the shard
and periodically writes them to disk in a compact binary format
(`CLogStorage`, see `log_storage.h`). These files are **not** plain text and
cannot be read with `cat`/`less`/a text editor — they have to be loaded back
through the logger service itself, using its admin console.

## Where the files live

Log files are written under:

```
<SaveFilesDirectory>/logs/
```

(`SaveFilesDirectory` is the service's own config variable; `CLogStorage::getLogRoot()`
in `log_storage.h` resolves it). Two kinds of files are produced:

- `minutely_YYYY-MM-DD_HH-MM-SS.binlog` — rolling snapshot of the last minute of logs.
- `hourly_YYYY-MM-DD_HH-MM-SS.binlog` — full hourly dump, this is what `queryLogs`
  (see below) scans through.

## Accessing the admin console

The logger service, like other services, runs inside a `screen` session. Join it
with your usual shard tooling (e.g. `shard join` / `shard dev`), then select the
logger_service window/pane. You get an interactive console where you can type the
commands below directly.

## Commands

Defined in `logger_service.cpp` (`NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN`):

| Command | Args | What it does |
|---|---|---|
| `loadLogs` | `<filename>` | Loads one `.binlog` file from disk and immediately dumps every entry (date, log name, text, all parameters) to the console. Simplest way to just read one file. |
| `queryLogs` | `<query>` or `help` | Runs a Log Query Language (LQL) query across the `hourly_*.binlog` files, asynchronously, and writes matching entries to a result file (see below). `queryLogs help` prints the LQL quick reference (same content as below, pulled from the `LogQueryLanguageHelp` config variable). |
| `interruptQuery` | — | Stops the currently running query. |
| `displayLog` | `[duration(s)=10]` | Dumps the last N seconds of logs still held in memory (no file access, no query needed). |
| `displayLogFormat` | — | Prints the definition of every known log type and its parameters — useful to know what to query for. |
| `saveLogs` | — | Forces an immediate hourly save of the in-memory logs to a new `.binlog`, then dumps it. |
| `dump` | — | Generic module state dump (connected clients, in-memory log count). |

### Quick read of a single file

```
loadLogs /path/to/save/logs/hourly_2026-07-20_18-00-00.binlog
```

This prints every entry straight to the console, e.g.:

```
2026-07-20 18:03:12 : Item_Create : ... : itemId=[...] userId=...
```

## Log Query Language (LQL)

Used by `queryLogs`. Full reference (from `patchman_cfg/default/logger_service.cfg`,
also printable in-service via `queryLogs help`):

### General form

```
(options) predicate (logicalCombiner predicate)*
```

### Options (optional, placed first)

- `full_context` — also extract every log that belongs to the same context as a selected log.
- `output_prefix = <prefix>` — prefix prepended to the result file name (see below). Value can be a bare identifier or a double-quoted string.

### Logical combiners

- `and` / `or`. `and` has higher priority than `or`. Use parentheses to force grouping:
  ```
  (predicate1 or predicate2) and predicate3
  ```

### Predicate form

```
<paramName | paramType> <operator> <constant>
```

- **paramName**: any parameter name used by some log (e.g. `userId`). Every log carrying that param is tested.
- **paramType**: test any parameter of a given type regardless of its name, written `{typeName}`. Available types: `uint32`, `uint64`, `sint32`, `float`, `string`, `entityId`, `itemId`, `sheetId`. `entityId`, `itemId`, `sheetId` are usually the most useful.
- **operator**: `<`, `<=`, `>`, `>=`, `=` (or `==`), `!=`, `like` (substring match).
- **constant**:
  - `uint32`: decimal or hex (`0x...`)
  - `sint32`: decimal prefixed with `-`
  - `string`: double-quoted, e.g. `"Item_Create"`
  - `entityId`: NeL entity id format, e.g. `(0x1234:12:34:56)`
  - `sheetId`: bare sheet name, e.g. `foo.sitem`
  - `itemId`: as printed by the ryzom tools, e.g. `[123:0x123456:1234]`

### Special (hardcoded) parameter names

- `LogName` — match by log type name instead of a parameter value:
  ```
  LogName = "Item_Create"
  ```
- `LogDate` — restrict the time range scanned. Accepted formats:
  - literal date: `YYYY-MM-DD`
  - literal date+time: `YYYY-MM-DD HH:MM` or `YYYY-MM-DD HH:MM:SS`
  - `yesterday`
  - relative: `<count> secs|mins|hours|days|weeks|months|years`

  If a query has **no** `LogDate` predicate at all, it's automatically limited to the
  last 24 hours (and a warning line is added to the result file).
- `ShardId` — restrict to logs from a specific shard id (numeric).

### Examples

All item creations in the last 2 days:
```
LogDate > 2 days and LogName = "Item_Create"
```

Everything about a specific character entity since yesterday, with full context:
```
full_context LogDate > yesterday and {entityId} = (0x1234:12:34:56)
```

All logs mentioning a given item id, from shard 3 only, written to a custom result file:
```
output_prefix = incident42_ ShardId = 3 and {itemId} = [123:0x123456:1234]
```

Everything older than 3 weeks that mentions "timeout" in any string parameter:
```
LogDate < 3 weeks and {string} like "timeout"
```

Combine several conditions with explicit grouping:
```
LogDate > 2026-07-01 and (LogName = "Item_Create" or LogName = "Item_Destroy")
```

### Running a query and getting the result

```
queryLogs LogDate > yesterday and LogName = "Item_Create"
```

The console immediately replies `Started query N` — the query runs in a background
thread, it does not block the console. Progress (files being read, current step,
final log/error) is written to the service's own log output (`nlinfo`/status line),
and can also be seen through the service's current status. Once finished, the
selected entries are written as plain text to:

```
<output_prefix><LogQueryResultFile>
```

`LogQueryResultFile` is a config variable (`lgs` section) defaulting to
`log_query_result.txt`, resolved relative to the service's working directory.
Use `output_prefix = ` to avoid overwriting the result of a previous query.

To stop a long-running query:
```
interruptQuery
```
