"""Generic, .bnp-aware search-path scanning: the *only* place Forgery
resolves a named file from (material texture references, .skel/.anim
compatibility, panoply_files.txt -- no separate "data root"/asset index; a
plain folder here, with recursive on, covers that same case). The folder
list itself lives in the shared settings file (`ryzom_forgery.settings`,
`Settings.search_paths`) -- this module is just the pure scanning logic
(SearchPathsDialog decides when to rescan, and runs it off the main thread)
plus the on-disk scan-result cache (see load_scan_cache()/save_scan_cache())
that makes a rescan of unchanged files near-instant.
"""

import json
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator, List, Optional

from pynel.ryzom_bnp import BnpError, BnpReader

from .cache_dir import cache_dir
from .explorer import BNP_EXTENSIONS

_SCAN_CACHE_FILE_NAME = "skel_anim_scan_cache.json"
_BNP_TABLE_CACHE_FILE_NAME = "bnp_table_cache.json"

# A texture reference in a .shape's material stores no extension convention
# of its own -- the same base name routinely shows up on disk as any of
# these (DDS being the game's own runtime format, TGA/PNG common for
# source/edited art) -- tried in this order after the exact name misses.
TEXTURE_FALLBACK_EXTENSIONS = (".tga", ".png", ".dds")


@dataclass(slots=True)
class FoundEntry:
	"""One matched file, wherever it actually lives: a plain file on disk
	(bnp_path is None) or an entry inside a .bnp archive (bnp_path set).
	`name` is always the bare file name, matching how .bnp entries are
	addressed (the .bnp format has no sub-folder concept). `slots=True`:
	a real data tree scan constructs on the order of 10^5 of these, where
	a plain dataclass's per-instance `__dict__` is a measurable chunk of
	both construction time and memory."""
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


def _iter_bnp_entries(bnp_path: Path, bnp_table_cache: dict) -> Iterator[FoundEntry]:
	"""Entry names for `bnp_path`, from `bnp_table_cache` if its (mtime, size)
	still matches what's cached (skipping the open+seek+read of the
	archive's own table entirely), parsed fresh and cached back otherwise.
	Measured to be a comparatively small share of a real scan's time --
	constructing/indexing the resulting ~10^5 FoundEntry objects (see
	_iter_dir_entries()'s own note) dominates instead -- but still worth
	keeping: some real data trees have hundreds of archives, and re-opening
	each one on every single scan adds up on its own."""
	try:
		stat = bnp_path.stat()
	except OSError:
		return
	key = str(bnp_path)
	cached = bnp_table_cache.get(key)
	if cached is not None and cached.get("mtime") == stat.st_mtime and cached.get("size") == stat.st_size:
		names = cached.get("names", [])
	else:
		try:
			names = [entry.name for entry in BnpReader(bnp_path).list()]
		except BnpError:
			names = []
		bnp_table_cache[key] = {"mtime": stat.st_mtime, "size": stat.st_size, "names": names}
	for name in names:
		yield FoundEntry(name=name, fs_path=None, bnp_path=bnp_path)


def _iter_dir_entries(directory: Path, recursive: bool, bnp_table_cache: dict) -> Iterator[FoundEntry]:
	"""Every file directly in `directory`, plus every entry of any .bnp
	sitting directly in it (a .bnp is one filesystem node, not a
	"subfolder" the recursive flag should gate -- its own contents are
	always descended into). Subfolders are only walked when `recursive`.

	Iterative (an explicit stack), not a recursive generator calling itself
	via `yield from` per subdirectory level: CPython's `yield from` has a
	real per-item forwarding cost, and a self-recursive version pays it once
	per directory *depth* for every single entry near the bottom of a real
	tree. This way every entry only ever crosses one `yield from` hop
	(into _iter_bnp_entries, for a .bnp's contents) regardless of how deep
	it sits.

	os.scandir(), not Path.iterdir(): a DirEntry's is_dir() reuses the file
	type readdir() itself already returned (on platforms that support it),
	so checking it doesn't cost a separate stat() syscall per child the way
	Path.is_dir() does -- that per-child stat was the actual dominant cost
	of a real tree scan, well past FoundEntry construction or the .bnp table
	reads. Path objects are only built for entries actually turned into a
	FoundEntry (or pushed back onto `pending`), never for every child just
	to check its type."""
	pending = [str(directory)]
	while pending:
		try:
			scan = os.scandir(pending.pop())
		except OSError:
			continue
		with scan:
			for child in scan:
				try:
					is_dir = child.is_dir()
				except OSError:
					continue
				if is_dir:
					if recursive:
						pending.append(child.path)
					continue
				if os.path.splitext(child.name)[1].lower() in BNP_EXTENSIONS:
					yield from _iter_bnp_entries(Path(child.path), bnp_table_cache)
				else:
					yield FoundEntry(name=child.name, fs_path=Path(child.path), bnp_path=None)


def iter_all_entries(dirs: List, bnp_table_cache: Optional[dict] = None) -> Iterator[FoundEntry]:
	"""Every file found (on disk or inside a .bnp) across every configured
	folder (a list of `ryzom_forgery.settings.SearchPathDir`, in priority
	order -- see find_texture()'s note on why that order matters) --
	callers filter by suffix/name themselves (see SearchPathsDialog.reload(),
	which builds both the .skel candidate list and the texture-name index
	from a single pass over this). `bnp_table_cache`, if given, is read from
	and populated in place with each .bnp's entry-name listing (see
	load_bnp_table_cache()/save_bnp_table_cache()) -- pass None (the
	default) for a one-shot, uncached call (e.g. build_texture_index(),
	where nothing persists across calls anyway)."""
	if bnp_table_cache is None:
		bnp_table_cache = {}
	for entry in dirs:
		directory = Path(entry.path)
		if directory.is_dir():
			yield from _iter_dir_entries(directory, entry.recursive, bnp_table_cache)


def build_texture_index(dirs: List) -> dict:
	"""{name.lower(): FoundEntry} for every file across `dirs` -- a cheap,
	uncached one-shot equivalent of SearchPathsDialog's own
	background-scanned texture index, for callers that don't need a
	persistent/incremental one (e.g. the CLI shape_exporter.py, one process
	per export)."""
	entries = {}
	for found in iter_all_entries(dirs):
		entries.setdefault(found.name.lower(), found)
	return entries


def find_texture(entries_by_lower_name: dict, name: str) -> Optional[FoundEntry]:
	"""Case-insensitive exact match first, then the same base name with each
	of TEXTURE_FALLBACK_EXTENSIONS -- shared matching rule used by both
	SearchPathsDialog.find_texture() (cached/background-scanned index) and
	one-shot callers indexing via build_texture_index()."""
	candidates = [name.lower()]
	stem = Path(name).stem.lower()
	candidates += [stem + extension for extension in TEXTURE_FALLBACK_EXTENSIONS]
	for candidate in candidates:
		match = entries_by_lower_name.get(candidate)
		if match is not None:
			return match
	return None


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


def load_bnp_table_cache() -> dict:
	"""{str(bnp_path): {"mtime": float, "size": int, "names": [str, ...]}}
	-- avoids re-opening+re-reading a .bnp's own table on every scan for
	archives that haven't changed (see _iter_bnp_entries()'s own note on
	how much this actually saves vs. the rest of a scan). Corrupt/missing
	cache is just an empty starting point, not an error."""
	path = cache_dir() / _BNP_TABLE_CACHE_FILE_NAME
	try:
		return json.loads(path.read_text())
	except (OSError, ValueError):
		return {}


def save_bnp_table_cache(cache: dict) -> None:
	directory = cache_dir()
	directory.mkdir(parents=True, exist_ok=True)
	(directory / _BNP_TABLE_CACHE_FILE_NAME).write_text(json.dumps(cache))
