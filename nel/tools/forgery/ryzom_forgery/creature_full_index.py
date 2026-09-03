"""Full (uncurated) creature/skel/animation index, built from a Ryzom Live
install's own game data under settings.live_data_path (see live_data.py) --
see /repos/project-todos/ryzom-core/forgery-object-editor.md's "Real
skel/anim lookup for the Skinning preview" chantier for the design.

Distinct from creature_ref.py's own curated cache (creatures_ref_cache.json/
creatures_anim_cache.json, a small hand-picked reference-creature list
bundled with Forgery itself): this one covers EVERY creature.packed_sheets
entry (28545 on a real install, measured 2026-09-03), generated on demand
from live_data_path and never bundled -- too big and install-specific to
ship.

Two indexes are built from the same parse pass:
- `creatures`: sheet name -> creature_ref.CreatureRecord.to_dict() (skel,
  anim_set_base_name, slot/hair shapes) for every real .creature.
- `shapes`: lowercased shape-filename-stem -> [[creature name, skel,
  anim_set_base_name], ...] -- the actual answer to "which real skel/anim
  set does this loaded .shape belong to" (see creature_bind.py's Skinning
  preview), built by inverting `creatures`.
Plus `anim_cache` (creature_ref.build_anim_cache_from_bytes()'s own result,
covering every anim_set_base_name found, not just the 2 curated ones).

Cache staleness is sha1-based, not the mtime pattern used elsewhere in this
codebase (e.g. search_paths_dialog.py's scan cache) -- deliberate exception
for this cache only: these source files are fixed game data, never
user-edited, so a content hash is the right staleness signal (confirmed
with Nuno, 2026-09-03). See `is_stale()`/`build_and_save()`.
"""

import hashlib
import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from . import creature_ref
from .config_dir import config_dir

_INDEX_PATH = config_dir() / "live_data_index.json"

# Read in this order so a progress callback's stage text progresses the
# same way for every build, matching what's shown to the user.
SOURCE_FILE_NAMES = (
	"creature.packed_sheets", "item.packed_sheets", "sitem.packed_sheets",
	"sheet_id.bin", "animset_list.packed_sheets", "mode2animset.string_array",
)


def index_path() -> Path:
	return _INDEX_PATH


def _resolve_source_bytes(live_data_dir: Path, name: str) -> Optional[bytes]:
	"""`name` loose under `live_data_dir` if present, else the first .bnp
	archive (in the same folder) found to contain it -- sheet_id.bin in
	particular ships packed inside leveldesign.bnp on a real install rather
	than loose (confirmed 2026-09-03), so the loose check alone isn't
	enough."""
	loose = live_data_dir / name
	if loose.is_file():
		return loose.read_bytes()

	from pynel import ryzom_bnp

	for bnp_path in sorted(live_data_dir.glob("*.bnp")):
		try:
			reader = ryzom_bnp.BnpReader(bnp_path)
		except Exception:
			continue
		for entry in reader.list():
			if entry.name.lower() == name.lower():
				return reader.read_file(entry.name)
	return None


def compute_source_hashes(live_data_dir: Path, progress: Optional[dict] = None) -> Dict[str, str]:
	"""sha1 of every SOURCE_FILE_NAMES entry actually found under
	`live_data_dir` -- missing ones are simply absent from the result
	(callers compare against a stored set, a missing key is as much a
	mismatch as a changed hash)."""
	hashes = {}
	for name in SOURCE_FILE_NAMES:
		if progress is not None:
			progress["stage"] = f"Hashing {name}..."
		data = _resolve_source_bytes(live_data_dir, name)
		if data is not None:
			hashes[name] = hashlib.sha1(data).hexdigest()
	return hashes


def is_stale(live_data_dir: Path) -> bool:
	"""True if the on-disk index is missing, or any SOURCE_FILE_NAMES file's
	real content no longer matches what it was generated from."""
	if not _INDEX_PATH.is_file():
		return True
	try:
		stored = json.loads(_INDEX_PATH.read_text(encoding="utf-8"))
	except (OSError, json.JSONDecodeError):
		return True
	stored_hashes = stored.get("source_hashes", {})
	current_hashes = compute_source_hashes(live_data_dir)
	# Missing entirely (a source file that couldn't be found this time) is
	# also staleness -- surfaces as a build error the user can act on,
	# rather than silently keeping a possibly-outdated index forever.
	return stored_hashes != current_hashes


def build_index(live_data_dir: Path, progress: Optional[dict] = None) -> dict:
	"""The actual build -- pure computation, no file writes (see
	build_and_save()) so this stays easy to call from a test/one-off script
	too. `progress`, if given, is updated in place as a plain dict (same
	"safe enough under the GIL, no lock" pattern as panoply_ui.py's
	_bake_progress) so a caller on another thread can poll it for UI
	feedback -- see live_data_index_dialog.py."""
	def _stage(text):
		if progress is not None:
			progress["stage"] = text

	blobs = {}
	for name in SOURCE_FILE_NAMES:
		_stage(f"Reading {name}...")
		data = _resolve_source_bytes(live_data_dir, name)
		if data is None:
			raise FileNotFoundError(f"{name!r} not found under {live_data_dir} (loose or inside a .bnp)")
		blobs[name] = data

	source_hashes = {name: hashlib.sha1(data).hexdigest() for name, data in blobs.items()}

	from pynel import ryzom_packed_sheets as ps

	_stage("Parsing creature.packed_sheets...")
	creature_data = ps.parse_creature_packed_sheets(blobs["creature.packed_sheets"])
	_stage("Parsing item.packed_sheets...")
	item_data = ps.parse_item_packed_sheets(blobs["item.packed_sheets"])
	_stage("Parsing sitem.packed_sheets...")
	sitem_data = ps.parse_item_packed_sheets(blobs["sitem.packed_sheets"])
	items_by_id = {**item_data.entries, **sitem_data.entries}
	_stage("Parsing sheet_id.bin...")
	sheet_id_names = ps.parse_sheet_id_bin(blobs["sheet_id.bin"])
	name_to_id = creature_ref.build_name_to_id(sheet_id_names)

	_stage("Building creature index...")
	total = len(creature_data.entries)
	records: Dict[str, creature_ref.CreatureRecord] = {}
	for i, (sheet_id, sheet) in enumerate(creature_data.entries.items()):
		name = sheet_id_names.get(sheet_id, f"#{sheet_id}")
		records[name] = creature_ref.creature_record_from_sheet(name, sheet, items_by_id, name_to_id)
		if progress is not None:
			progress["index_i"], progress["index_total"] = i + 1, total

	_stage("Inverting shape -> creature index...")
	shape_index: Dict[str, List[Tuple[str, str, str]]] = {}
	items = list(records.items())
	for i, (name, record) in enumerate(items):
		for shape in list(record.slots.values()) + record.hair:
			# "none.shape" is CCharacterSheet's own placeholder for "no item
			# equipped in this slot" (confirmed 2026-09-03: the only
			# "none*" value seen across a full real index build), not a
			# real shape -- indexing it would make every one of its
			# ~34000 occurrences (one per empty slot) look like a match for
			# a shape literally named "none.shape", pure noise.
			if not shape or shape.lower() == "none.shape":
				continue
			# Keyed by stem (extension stripped), matching creatures_for_shape()'s
			# own Path(...).stem.lower() lookup -- storing the raw filename
			# (with its ".shape" extension still attached) here meant that
			# lookup NEVER matched anything, for any shape, ever (found by
			# Nuno, 2026-09-03, tracing why fo_carnitree.shape came back
			# empty despite being genuinely indexed).
			key = Path(shape).stem.lower()
			shape_index.setdefault(key, []).append((name, record.skel, record.anim_set_base_name))
		if progress is not None:
			progress["invert_i"], progress["invert_total"] = i + 1, len(items)

	_stage("Resolving real animation lists...")
	anim_cache = creature_ref.build_anim_cache_from_bytes(
		records, blobs["animset_list.packed_sheets"], blobs["mode2animset.string_array"])

	return {
		"source_hashes": source_hashes,
		"creatures": {name: record.to_dict() for name, record in records.items()},
		"shapes": {shape: [list(entry) for entry in entries] for shape, entries in shape_index.items()},
		"anim_cache": anim_cache,
	}


def build_and_save(live_data_dir: Path, progress: Optional[dict] = None) -> None:
	data = build_index(live_data_dir, progress)
	_INDEX_PATH.parent.mkdir(parents=True, exist_ok=True)
	_INDEX_PATH.write_text(json.dumps(data), encoding="utf-8")


_index_cache: Optional[dict] = None


def load_index() -> dict:
	"""Lazily loads+caches the on-disk index for the process's lifetime --
	same pattern as creature_ref.load_anim_cache(). {} if never built yet."""
	global _index_cache
	if _index_cache is None:
		if _INDEX_PATH.is_file():
			try:
				_index_cache = json.loads(_INDEX_PATH.read_text(encoding="utf-8"))
			except (OSError, json.JSONDecodeError):
				_index_cache = {}
		else:
			_index_cache = {}
	return _index_cache


def invalidate_loaded_index() -> None:
	"""Call after build_and_save() completes so the next load_index() picks
	up the fresh file instead of a stale in-memory copy (or the pre-first-build
	empty {})."""
	global _index_cache
	_index_cache = None


def creatures_for_shape(shape_name: str) -> List[Tuple[str, str, str]]:
	"""Real (creature name, skel, anim_set_base_name) tuples for a loaded
	shape's filename (any case/extension -- matched by stem, lowercased) --
	[] if the shape isn't referenced by any real .creature's equipment
	(e.g. a custom/mod shape, or one this index hasn't been built for yet).
	See creature_bind.py's _draw_bone_preview_controls()."""
	stem = Path(shape_name).stem.lower()
	shapes = load_index().get("shapes", {})
	return [tuple(entry) for entry in shapes.get(stem, [])]
