"""Freshness check + in-memory memoization for live Panoply recoloring
(Phase A Step 3, see .todo/forgery-object-editor.md). No disk I/O of its
own -- callers (Step 4's wiring into the texture resolution path) supply
already-resolved search_paths.FoundEntry-like objects (anything with a
cache_stat() -> (mtime, size), same duck type search_paths.py's own scan
cache relies on) for the already-baked variant (if any), the base texture,
and each relevant mask; this module only decides whether that baked variant
is good enough to use as-is, and memoizes whatever a live recompute produces
so it isn't redone every frame for an unchanged combination.
"""

from typing import Dict, Optional


def is_baked_stale(baked_ref, base_ref, mask_refs: Dict[str, object]) -> bool:
	"""True if the already-baked variant needs replacing with a live
	recompute: `baked_ref` is None (nothing resolved on disk at all), or its
	mtime is older than the base texture's or any of `mask_refs`'s (the base
	or a mask was edited since that variant was baked)."""
	if baked_ref is None:
		return True
	baked_mtime, _ = baked_ref.cache_stat()
	base_mtime, _ = base_ref.cache_stat()
	if base_mtime > baked_mtime:
		return True
	for mask_ref in mask_refs.values():
		mask_mtime, _ = mask_ref.cache_stat()
		if mask_mtime > baked_mtime:
			return True
	return False


class LiveColorizeCache:
	"""Memoizes one computed image per (base texture, axis selection, source
	mtimes) combination -- keying on the sources' own mtimes (rather than
	just base name + axis values) means an edited base texture or mask
	naturally misses the cache instead of serving a now-stale result,
	without needing any explicit invalidation call. Grows at most one entry
	per distinct combination actually viewed in a session, which stays small
	in practice (a handful of axis picks across a shape's own textures) --
	no eviction needed."""

	def __init__(self):
		self._entries = {}

	@staticmethod
	def make_key(base_name: str, dims: Dict[str, str], base_ref, mask_refs: Dict[str, object]):
		axis_items = tuple(sorted(dims.items()))
		base_mtime, _ = base_ref.cache_stat()
		mask_mtimes = tuple(sorted((axis, ref.cache_stat()[0]) for axis, ref in mask_refs.items()))
		return (base_name, axis_items, base_mtime, mask_mtimes)

	def get(self, key) -> Optional[object]:
		return self._entries.get(key)

	def set(self, key, image) -> None:
		self._entries[key] = image
