import json
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from pynel.ryzom_bnp import BnpReader, BnpError

from .explorer import BNP_EXTENSIONS

CACHE_FILENAME = ".ryzom_forgery_asset_index.json"
TEXTURE_FALLBACK_EXTENSIONS = (".tga", ".png", ".dds")


@dataclass
class AssetRef:
	"""Resolved location of a named asset: either a real file on disk, or
	a virtual entry inside a `.bnp`/`.bnpe` archive (in which case
	`bnp_path` is set and `name` is the entry name inside it)."""

	name: str
	path: Path
	bnp_path: Optional[Path] = None

	def read_bytes(self) -> bytes:
		if self.bnp_path is not None:
			return BnpReader(self.bnp_path).read_file(self.name)
		return self.path.read_bytes()


class AssetIndex:
	"""Indexes every file under a Ryzom data root -- both loose files and
	the contents of every `.bnp`/`.bnpe` archive found recursively -- by
	base name, so a tool can resolve a referenced asset (e.g. a texture
	named by a `.shape`'s material) regardless of which archive it's
	packed into.

	Scanning a real Ryzom install (thousands of `.bnp` files) from scratch
	takes a while, so the result is cached to disk (next to the root) and
	on the next build() only archives whose (size, mtime) changed are
	actually re-read.
	"""

	def __init__(self, root: Path, cache_path: Optional[Path] = None):
		self.root = Path(root)
		self.cache_path = Path(cache_path) if cache_path else self.root / CACHE_FILENAME
		self._by_name: dict = {}

	def __len__(self) -> int:
		return len(self._by_name)

	def build(self) -> None:
		cached_bnps = self._load_cache()

		by_name: dict = {}
		bnp_cache: dict = {}

		for path in self.root.rglob("*"):
			if not path.is_file():
				continue

			if path.suffix.lower() in BNP_EXTENSIONS:
				key = str(path)
				stat = path.stat()
				signature = [stat.st_size, int(stat.st_mtime)]

				cached_entry = cached_bnps.get(key)
				if cached_entry is not None and cached_entry.get("signature") == signature:
					entry_names = cached_entry["entries"]
				else:
					try:
						entry_names = [entry.name for entry in BnpReader(path).list()]
					except BnpError:
						entry_names = []

				bnp_cache[key] = {"signature": signature, "entries": entry_names}
				for entry_name in entry_names:
					by_name[entry_name.lower()] = AssetRef(name=entry_name, path=path, bnp_path=path)
			else:
				by_name[path.name.lower()] = AssetRef(name=path.name, path=path)

		self._by_name = by_name
		self._save_cache(bnp_cache)

	def find(self, name: str) -> Optional[AssetRef]:
		return self._by_name.get(name.lower())

	def find_texture(self, name: str) -> Optional[AssetRef]:
		ref = self.find(name)
		if ref is not None:
			return ref

		stem = Path(name).stem
		for extension in TEXTURE_FALLBACK_EXTENSIONS:
			ref = self.find(stem + extension)
			if ref is not None:
				return ref
		return None

	def _load_cache(self) -> dict:
		try:
			with open(self.cache_path) as fh:
				return json.load(fh)
		except (OSError, ValueError):
			return {}

	def _save_cache(self, bnp_cache: dict) -> None:
		try:
			with open(self.cache_path, "w") as fh:
				json.dump(bnp_cache, fh)
		except OSError:
			pass
