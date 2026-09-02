"""Shared same-name collision tracking for the workspace watcher's
extension-triggered groups (imports -> shapes/, textures -> build/dds/,
synced assets -> the external mirror) -- see the workspace-watcher chantier
in project-todos/ryzom-core/forgery-object-editor.md for the full
background. Each group flattens its matched files to a name-only output
(no subfolder), so two different source files sharing that name (case-
insensitive) would otherwise silently collide there. One DuplicateNameGuard
instance per group, each with its own comparison key (stem for a group
whose output extension differs from its input, full filename for a group
that copies the file as-is) and its own notion of "current known files" --
never compared across groups.
"""


class DuplicateNameGuard:
	"""Tracks one group's currently known source files by `key_of(path)`
	(caller-supplied, e.g. `lambda p: p.stem.lower()`), and calls
	`on_conflict(path_a, path_b)` the moment two different existing files
	share a key -- never more than once per unresolved pair. The caller is
	expected to skip processing (import/convert/sync) any path this
	reports as conflicting, and to keep calling update()/remove() as
	real filesystem events settle so a resolved conflict (one side
	deleted/renamed away) starts processing again on its own."""

	def __init__(self, key_of, on_conflict):
		self._key_of = key_of
		self._on_conflict = on_conflict
		self._known = {}  # {key: path} -- exactly one known file, no conflict
		self._conflicts = {}  # {key: (path_a, path_b)} -- reported, not yet resolved

	def reset(self):
		self._known = {}
		self._conflicts = {}

	def scan(self, paths):
		"""Full rebuild from `paths` (already filtered to this group's own
		extensions/exclusions by the caller) -- for startup/reconcile.
		Returns the list of paths that are conflict-free and therefore safe
		to process; the caller must leave every other path untouched. Only
		the first two paths found per colliding key are reported -- a 3rd
		file sharing that key is left unprocessed and unreported until the
		first pair is resolved and scan()/update() runs again."""
		self.reset()
		buckets = {}
		for path in paths:
			buckets.setdefault(self._key_of(path), []).append(path)
		safe = []
		for key, group in buckets.items():
			if len(group) == 1:
				self._known[key] = group[0]
				safe.append(group[0])
			else:
				pair = (group[0], group[1])
				self._conflicts[key] = pair
				self._on_conflict(*pair)
		return safe

	def update(self, path):
		"""Incremental check for one settled created/renamed-to path (still
		exists on disk). Returns True if `path` is safe to process, False
		if it's conflicting (freshly reported, or an already-reported,
		still-unresolved key -- not re-reported in that case)."""
		key = self._key_of(path)
		if key in self._conflicts:
			return False
		existing = self._known.get(key)
		if existing is not None and existing != path:
			self._conflicts[key] = (existing, path)
			del self._known[key]
			self._on_conflict(existing, path)
			return False
		self._known[key] = path
		return True

	def known_paths(self):
		"""Every currently tracked, conflict-free path -- for a caller that
		needs to (re-)act on the whole safe set outside of scan()'s own
		return value (e.g. workspace_sync.py's sync_now()/
		refresh_fully_synced(), reusing whatever the last scan()/update()
		calls already built instead of re-scanning the filesystem)."""
		return list(self._known.values())

	def remove(self, path):
		"""Call once `path` is gone (deleted, or the source side of a
		rename). Returns the *other* path of a conflict `path` was part of,
		if that survivor still exists on disk -- now safe, and the caller's
		responsibility to process it (it was never processed while the
		conflict stood). Returns None otherwise (path wasn't conflicting, or
		had no surviving counterpart)."""
		key = self._key_of(path)
		if self._known.get(key) == path:
			del self._known[key]
			return None
		pair = self._conflicts.get(key)
		if pair is not None and path in pair:
			del self._conflicts[key]
			survivor = pair[1] if pair[0] == path else pair[0]
			if survivor.exists():
				self._known[key] = survivor
				return survivor
		return None
