# `.cfg` — NeL `CConfigFile` format

Status: **implemented** (`pynel.config_file`, `ryzom-cfg` CLI). Source of
truth: `nel/src/misc/config_file/cf_lexical.lpp`/`cf_gramatical.ypp` (the
real flex/bison grammar) and `config_file.cpp` (`CConfigFile::load()`/
`reparse()`).

## Why this exists

`CConfigFile` is NeL's generic `.cfg` format, used across the whole engine
-- `client.cfg` (`ryzom/client/src/network_connection.cpp`'s
`NCConfigFile.load("client.cfg")`) and `panoply_maker.cpp`'s
`panoply_*.cfg` both load through the exact same class. Confirmed with Nuno
(2026-08-29) this is meant to replace ryztart's own hand-rolled `.cfg`
parser, which doesn't cover the full format -- so unlike most of this
port's other pieces, this module isn't panoply-specific and lives in pynel
(no Forgery/Panda3D/numpy dependency) rather than in
`ryzom_forgery/panoply_maker.py`.

## Format

- `// line` and `/* block */` comments.
- `name = expr;` or `name += expr;` statements.
- `expr` is either `{ v1, v2, ... }` (array, trailing comma allowed, `{}`
  empty array allowed) or a single value -- a bare (non-array) assignment
  is still stored as a 1-element value list, confirmed against a real
  production file (`current_panoply.cfg`'s `additionnal_paths = "...";`,
  no braces).
- A value is an int/real/hex literal, a string literal (`"..."`, **no
  escape sequences** -- the lexer's string rule is `\"[^\"\n]*\"`, so a
  literal `"` inside a string is unrepresentable), `-x`/`+x` (unary),
  `a+b`/`a-b`/`a*b`/`a/b`, `(expr)`, or a reference to another
  already-defined variable by name.
- `+=` extends an existing array: if the variable was *also* assigned
  earlier in the *same* file, the new values are appended at the end; if
  it came from an earlier-loaded file (see `RootConfigFilename` below),
  the new values are **prepended** instead -- ported faithfully from
  `cf_gramatical.ypp`'s `ADD_ASSIGN` action, though real usage of `+=` was
  not found in any `.cfg` file checked while writing this (`panoply_*.cfg`
  nor `client.cfg`).
- `RootConfigFilename = "other.cfg";`: after parsing a file, `CConfigFile`
  looks for this variable and, if present, loads that file too -- but its
  `=` assignments only fill in variables **not already set** (the
  first-loaded file's own values always win; see `CVar::Root`/
  `FromLocalFile`/`cf_OverwriteExistingVariable` in `config_file.cpp`).
  This is the real mechanism behind `client.cfg` (few overrides) falling
  back to `client_default.cfg` (everything else) -- validated against
  both real files (292 variables merged, chain followed automatically).

**Not ported**: `#fileline` (an internal multi-file line-tracking marker
emitted by an in-engine preprocessor, never hand-authored). A file using
it raises `ConfigError` rather than silently mis-parsing.

## Format-preserving edits

`Document` keeps the file's exact original text. `set()` only rewrites the
touched statement's value text (located via token spans), splices it in,
then re-tokenizes so every span stays accurate for the next edit -- every
other statement, comment, and blank line is left byte-for-byte untouched.
`dumps()`/`save()` round-trips byte-identically to the original file when
nothing was `set()` -- validated against the 7 real `panoply_*.cfg` files
(incl. `current_panoply.cfg`). New variables are appended at the end of
the file. A regex ordering bug (`REAL`'s pattern tried the
trailing-dot-no-fraction form before the fraction-required form, so `0.1`
tokenized as `0.` + `1` -- plain regex alternation is first-match, not
flex's longest-match) was found and fixed during this validation.

## CLI (`ryzom-cfg`)

```bash
# Print every variable, following RootConfigFilename
ryzom-cfg dump client.cfg

# Set one variable's value(s) in place, preserving everything else
ryzom-cfg set panoply_common.cfg skin_hues 30 40 35 221
```

## Library usage

```python
from pynel import config_file

# Merged multi-file view (follows RootConfigFilename automatically)
cfg = config_file.ConfigFile()
cfg.load("client.cfg")
print(cfg.get("ScreenWidth"))

# Single-file, format-preserving edit
doc = config_file.Document.load("panoply_common.cfg")
print(doc.get("user_hues"))
doc.set("user_hues", [10, 30, 78, 153, 212, 345, 35, 235])
doc.save()
```

## Known gaps

- `#fileline` not supported (see above).
- Binary string arithmetic (`"a" + "b"`) raises `ConfigError` rather than
  replicating the original C++'s undefined/no-op behavior for that case --
  not observed in any real `.cfg` checked.
- Integer division (`DIVIDE` on two ints) uses Python's `int(a / b)`
  truncation, not verified against the C++'s actual int/int division
  behavior -- unused in every real `.cfg` file checked, low-priority gap.
