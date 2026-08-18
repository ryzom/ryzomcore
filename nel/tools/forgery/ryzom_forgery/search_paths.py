"""Generic, .bnp-aware search-path scanning shared by every "look for a file
somewhere the user configured" feature in Forgery (finding .skel candidates
for the Skinning preview, resolving textures that aren't in the game's own
indexed data tree...). Configured once, in the Settings tab's "Paths"
section (see search_paths_dialog.py), reused by both -- pure scanning logic
here, no UI (SearchPathsDialog decides when to rescan, and runs it off the
main thread), just the on-disk scan-result cache (see load_scan_cache()/
save_scan_cache()) that makes a rescan of unchanged files near-instant.
"""

import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Iterator, List, Optional

from pynel.ryzom_bnp import BnpError, BnpReader

from .cache_dir import cache_dir
from .config_dir import config_dir
from .explorer import BNP_EXTENSIONS

_CONFIG_FILE_NAME = "search_paths_settings.json"
_SCAN_CACHE_FILE_NAME = "skel_anim_scan_cache.json"


@dataclass
class SearchPathDir:
	path: str
	recursive: bool = False


@dataclass
class SearchPathsConfig:
	dirs: List[SearchPathDir] = field(default_factory=list)


def load() -> SearchPathsConfig:
	path = config_dir() / _CONFIG_FILE_NAME
	try:
		data = json.loads(path.read_text())
	except (OSError, ValueError):
		return SearchPathsConfig()

	config = SearchPathsConfig()
	for entry in data.get("dirs", []):
		if isinstance(entry, dict) and "path" in entry:
			config.dirs.append(SearchPathDir(path=entry["path"], recursive=bool(entry.get("recursive", False))))
	return config


def save(config: SearchPathsConfig) -> None:
	directory = config_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _CONFIG_FILE_NAME).write_text(json.dumps(asdict(config), indent="\t"))


@dataclass
class FoundEntry:
	"""One matched file, wherever it actually lives: a plain file on disk
	(bnp_path is None) or an entry inside a .bnp archive (bnp_path set).
	`name` is always the bare file name, matching how .bnp entries are
	addressed (the .bnp format has no sub-folder concept)."""
	name: str
	fs_path: Optional[Path]
	bnp_path: Optional[Path]

	def read_bytes(self) -> bytes:
		if self.bnp_path is not None:
			return BnpReader(self.bnp_path).read_file(self.name)
		return self.fs_path.read_bytes()

	def cache_key(self) -> str:
		"""Stable string identity for this entry, used as the scan cache's
		key -- a .bnp entry is keyed off its archive (the .bnp format has no
		per-entry mtime of its own, see cache_stat())."""
		if self.bnp_path is not None:
			return f"{self.bnp_path}!{self.name}"
		return str(self.fs_path)

	def cache_stat(self):
		"""(mtime, size) of whatever actually owns these bytes on disk --
		the .bnp archive itself for an entry inside one (repacking it
		invalidates every entry it contains, which is the right call: there
		is no cheaper per-entry signal available), the plain file otherwise.
		Used to detect a file changed since it was last scanned/cached."""
		target = self.bnp_path if self.bnp_path is not None else self.fs_path
		stat = target.stat()
		return stat.st_mtime, stat.st_size


def _iter_bnp_entries(bnp_path: Path) -> Iterator[FoundEntry]:
	try:
		reader = BnpReader(bnp_path)
	except BnpError:
		return
	for entry in reader.list():
		yield FoundEntry(name=entry.name, fs_path=None, bnp_path=bnp_path)


def _iter_dir_entries(directory: Path, recursive: bool) -> Iterator[FoundEntry]:
	"""Every file directly in `directory`, plus every entry of any .bnp
	sitting directly in it (a .bnp is one filesystem node, not a
	"subfolder" the recursive flag should gate -- its own contents are
	always descended into). Subfolders are only walked when `recursive`."""
	try:
		children = list(directory.iterdir())
	except OSError:
		return
	for child in children:
		try:
			is_dir = child.is_dir()
		except OSError:
			continue
		if is_dir:
			if recursive:
				yield from _iter_dir_entries(child, recursive)
		elif child.suffix.lower() in BNP_EXTENSIONS:
			yield from _iter_bnp_entries(child)
		else:
			yield FoundEntry(name=child.name, fs_path=child, bnp_path=None)


def iter_all_entries(config: SearchPathsConfig) -> Iterator[FoundEntry]:
	"""Every file found (on disk or inside a .bnp) across every configured
	folder -- callers filter by suffix/name themselves (see
	SearchPathsDialog.reload(), which builds both the .skel candidate list
	and the texture-name index from a single pass over this)."""
	for entry in config.dirs:
		directory = Path(entry.path)
		if directory.is_dir():
			yield from _iter_dir_entries(directory, entry.recursive)


def load_scan_cache() -> dict:
	"""{cache_key: {"mtime": float, "size": int, "type": "skel"|"anim",
	"bones": [str, ...]}} -- the bone names a previously-parsed .skel/.anim
	had, keyed by FoundEntry.cache_key(), so a rescan can skip re-parsing
	any file whose (mtime, size) still matches (see SearchPathsDialog's
	background reload worker). Corrupt/missing cache is just an empty
	starting point, not an error -- everything gets re-parsed once and
	re-cached."""
	path = cache_dir() / _SCAN_CACHE_FILE_NAME
	try:
		return json.loads(path.read_text())
	except (OSError, ValueError):
		return {}


def save_scan_cache(cache: dict) -> None:
	directory = cache_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _SCAN_CACHE_FILE_NAME).write_text(json.dumps(cache))
