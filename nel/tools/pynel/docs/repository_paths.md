# repository_paths — local checkout locations

Status: **implemented** (`pynel.repository_paths`, library only, no CLI).

## Why this exists

A working Ryzom Core contributor typically has 4 git repositories checked
out side by side: `ryzom-core`, `ryzom-data`, `ryzom-private-data`,
`ryzom-docker`. Multiple tools built on pynel need to know where these live
on a given machine -- e.g. Forgery/Patina's real Panoply bake
(`ryzom_forgery.panoply_bake`, see
`/repos/project-todos/ryzom-core/panoply-runtime-tint.md`) needs
`ryzom-data`'s real `characters.hlsbank`/`panoply_files.txt` (at
`final_bnps/characters_maps_hr/`) as the source it appends new baked items
into. Rather than each tool inventing its own "where's ryzom-data" setting,
this one small module lives in pynel (which every such tool already depends
on) so a user configures it once, in one place.

Deliberately **not** part of `ryzom_forgery.settings` (Forgery's own
tomlkit-based settings file) -- this is meant to be usable by any pynel-based
tool, including ones with no Forgery/Panda3D dependency at all (e.g. a
future ryztart integration), so it can't depend on Forgery. Plain `json`,
matching pynel's own zero-external-dependency policy (`dependencies = []`
in `pyproject.toml`).

## Storage

One JSON file at `config_dir()/"repository_paths.json"` --
`config_dir()` is the standard per-OS user config directory (`%APPDATA%` on
Windows, `~/Library/Application Support` on macOS, `$XDG_CONFIG_HOME` or
`~/.config` elsewhere) under a `ryzom_tools` subfolder -- deliberately not
`ryzom_forgery` (a specific tool) or `pynel` (an implementation detail),
since this is meant to be shared across every tool in the Ryzom toolchain.
Reimplements the OS-detection logic already in
`ryzom_forgery.config_dir`/`cache_dir` rather than importing it -- the
dependency only ever goes Forgery -> pynel, never the other way.

```json
{
	"ryzom-data": "/home/user/repos/ryzom-data",
	"ryzom-core": "/home/user/repos/ryzom-core"
}
```

An unconfigured repository is simply absent from the dict -- never an empty
string.

## API

- `REPOSITORIES` -- the 4 valid keys, `("ryzom-core", "ryzom-data", "ryzom-private-data", "ryzom-docker")`. Every other function validates against this.
- `load() -> Dict[str, str]` -- everything currently configured. Corrupt/missing file is just an empty dict, not an error.
- `save(paths: Dict[str, str])` -- overwrites the whole file (silently drops any key not in `REPOSITORIES`).
- `set_path(repo_name, path)` -- updates one repo's path, leaving the others as they were (`load()` + mutate + `save()`) -- the usual call from a settings UI folder picker.
- `get(repo_name) -> Optional[Path]` -- the configured path, or `None` if unset. Doesn't check the path exists.
- `is_valid(repo_name) -> bool` -- configured **and** currently a real directory -- what a caller should actually gate risky operations on (a stale/moved checkout is common enough not to trust a bare configured string).

## Usage

```python
from pynel import repository_paths

if not repository_paths.is_valid("ryzom-data"):
	... # tell the user to configure it first, do nothing else
else:
	data_root = repository_paths.get("ryzom-data")
	hlsbank_path = data_root / "final_bnps" / "characters_maps_hr" / "characters.hlsbank"
```

Forgery/Patina exposes a settings UI for this (Settings tab -> "Paths" ->
one folder picker per repo, `object_editor._draw_repository_paths_settings()`)
-- see `docs/apps/object_editor.md`.
