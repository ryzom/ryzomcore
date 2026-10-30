# Crash debug logs (`nlcrashdbg`)

Temporary logs added to investigate a crash report use the `nlcrashdbg` macro (`nel/include/nel/misc/debug.h`) instead of a plain `nlinfo`/`nlwarning`.

## How it works

- `nlcrashdbg(id, "format", args...)` is an `nlinfo` whose text starts with `CRASHDBG #<id> `. The format must be a string literal.
- `<id>` is the id of the first crash report of the crash group (the first report that was classified in the group). It identifies the group: when the id is fixed, the whole group is.
- `nlinfo` is active in release builds (`NL_NO_DEBUG` is undefined in `types_nl.h`), and goes to the memory displayer and, when `logInFile` is on, to `client.log`.
- The tool that sends crash reports reads `client.log`, filters the `CRASHDBG #<id>` lines and sends them with the report.

## Adding and removing

- Add: `nlcrashdbg(6539800, "SubmitSourceBuffer hr=0x%08x", (unsigned int)hr);`
- Remove all of them once the crash is fixed: `grep -rn nlcrashdbg` and delete the lines.
