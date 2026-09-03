"""Curated .creature list + distilled reconstruction cache for Patina's
creature/NPC binder+assembler feature -- see
/repos/project-todos/ryzom-core/forgery-object-editor.md "Creature/NPC
binder + assembler in Patina" for the full design writeup.

A .creature sheet (creature.packed_sheets) doesn't carry .shape filenames
directly -- each of its Equipment slots (Body/Legs/Arms/Hands/Feet/Head/Face,
HairItemList) only names an .item/.sitem sheet, whose own IdShape*
race/gender-variant fields hold the real mesh filename (see
nel/tools/pynel/docs/packed_sheets.md; shape_from_item() below reproduces
CCharacterCL::shapeFromItem()'s fallback order). Resolving that chain for all
~28545 creatures via pynel.ryzom_packed_sheets costs ~6s cold (measured,
2026-08-30 -- creature.packed_sheets alone is ~5.2s of that, item/sitem the
rest), so this module works with a small, curated subset instead -- same
"bundled default + optional workspace override" pattern as panoply_config.py's
panoply.cfg -- and caches only the *distilled* result (what's needed to
reconstruct a creature visually), not the raw sheets.

Object-in-hand slots (weapons/shields) are deliberately excluded: their
in-game bind point is hardcoded in character_cl.cpp (box_arme/
box_arme_gauche/Box_bouclier), not data-driven -- out of scope for this
feature (confirmed with Nuno, 2026-08-30).
"""

import json
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

_BUNDLED_LIST_PATH = Path(__file__).parent / "creatures_ref.txt"
_BUNDLED_CACHE_PATH = Path(__file__).parent / "creatures_ref_cache.json"
_BUNDLED_SHAPE_SLOT_INDEX_PATH = Path(__file__).parent / "shape_slot_index.json"
_WORKSPACE_LIST_NAME = "creatures_ref.txt"
# workspace_dir/build/creatures_cache.json -- build/ is the same folder
# panoply_bake.py's bake_and_write() already uses for its own "next patch"
# output (not in workspaces.py's SUBDIRS, created on demand at point of use).
_WORKSPACE_CACHE_RELATIVE_PATH = ("build", "creatures_cache.json")
# workspace_dir/build/bind_slot_overrides.json -- a user's own manual
# corrections to the Bind preview's slot auto-detect (shape_slot_index.json,
# built offline from real item.packed_sheets data), for whenever that's
# wrong or missing for a given shape. Kept Forgery-side, not in the .shape
# file itself (confirmed 2026-08-30, Nuno asked about a "user field" in the
# .shape format -- CMeshBase::serialMeshBase(), mesh_base.cpp, has none;
# every field is individually versioned/serialized, adding one is an engine
# format-bump, out of scope here).
_WORKSPACE_SLOT_OVERRIDES_RELATIVE_PATH = ("build", "bind_slot_overrides.json")

# The 7 body-part equipment slots reconstructed for visual assembly --
# ObjectInRightHand/ObjectInLeftHand are deliberately excluded (see module
# docstring). Order matches CCharacterSheet's own field order.
BODY_SLOTS = ("body", "legs", "arms", "hands", "feet", "head", "face")

# Bone names an item is stuck to (case 2, "bound to one attach point", see
# forgery-object-editor.md) for right/left-hand equipment -- hardcoded in
# ryzom/client/src/character_cl.cpp's buildEquipment() (magician staffs use
# no offset, shields use Box_bouclier instead of box_arme_gauche), not data
# in any sheet or .skel file, so this is copied here verbatim rather than
# resolved from anything. Confirmed 2026-08-30 these are the ONLY real
# single-point equipment attach points -- see the CreatureRecord.body_to_bone
# field's own docstring for a correction of an earlier wrong assumption
# about BodyToBone also being attach-point data (it isn't).
WEAPON_ATTACH_POINTS = ("box_arme", "box_arme_gauche", "Box_bouclier")

# CItemSheet.slot_bf's bit index (SLOTTYPE::TSlotType, ryzom/common/src/
# game_share/slot_types.h -- NOTE the enum starts at UNDEFINED=0, so every
# real slot's bit is one past what its declaration-order position alone
# would suggest: UNDEFINED=0 HEADDRESS=1 HEAD=2 FACE=3 EARS=4 NECKLACE=5
# SHOULDER=6 BACK=7 CHEST=8 ARMS=9 WRIST=10 HANDS=11 FINGERS=12 LEGS=13
# ANKLE=14 FEET=15 RIGHT_HAND=16 LEFT_HAND=17 TWO_HANDS=18
# RIGHT_HAND_EXCLUSIVE=19 AMMO=20 -- confirmed against real item.packed_sheets
# data 2026-08-30 (FY_HOM_casque01.shape -> bit 2/HEAD,
# fy_HOM_armor01_gilet.shape -> bit 8/CHEST, fy_HOM_armor01_pantabottes.shape
# -> bit 13/LEGS, fy_HOM_armor01_bottes.shape -> bit 15/FEET) after an
# initial off-by-one version of this table (missing the UNDEFINED=0 slot)
# wrongly put every CHEST-slot item (bit 8) under "arms" instead of "body".
# Bit -> the BODY_SLOTS entry it corresponds to, per
# SLOTTYPE::convertTypeToVisualSlot()'s own switch (slot_types.cpp).
# RIGHT_HAND/LEFT_HAND/TWO_HANDS/RIGHT_HAND_EXCLUSIVE and every other bit
# convertTypeToVisualSlot() maps to HIDDEN_SLOT are deliberately absent here
# -- those are hand/weapon items, out of BODY_SLOTS' scope (see
# WEAPON_ATTACH_POINTS above), the slot-override auto-detect
# (build_shape_slot_index() below) just won't match them.
_SLOT_BIT_TO_BODY_SLOT = {
	2: "head", 3: "face", 8: "body", 9: "arms", 11: "hands", 13: "legs", 15: "feet",
}


@dataclass
class CreatureRecord:
	"""Distilled reconstruction data for one .creature -- everything needed
	to load its skeleton and attach every resolved body-part shape, nothing
	else (not the full CharacterSheet -- sounds/collisions/loot state/etc
	are irrelevant here)."""
	name: str
	skel: str
	race: int
	gender: int
	slots: Dict[str, str] = field(default_factory=dict)  # BODY_SLOTS entry -> resolved .shape name; missing = no shape for that slot
	hair: List[str] = field(default_factory=list)  # resolved .shape names, one per HairItemList entry
	# CCharacterSheet.BodyToBone's 10 body-part-name -> real bone-name mapping
	# (head/chest/left_arm/right_arm/left_hand/right_hand/left_leg/right_leg/
	# left_foot/right_foot -> e.g. "Bip01 Head"). CORRECTION (2026-08-30): this
	# is NOT equipment attach-point data -- earlier assumed so, wrongly. Real
	# use is CCharacterCL::getBoneNameFromBodyPart() (character_cl.cpp),
	# combat hit-location lookup (which bone an attack's Localisation maps
	# to), unrelated to how items get positioned. Kept here in case a future
	# feature needs it (e.g. visualizing hit locations), but the Bind
	# preview's attach-point combo must NOT be built from this -- see
	# WEAPON_ATTACH_POINTS below for the actual (very short) real list.
	body_to_bone: Dict[str, str] = field(default_factory=dict)
	# CCharacterSheet.id_anim_set_base_name -- the fragment computeAnimSet()
	# (ryzom/client/src/misc.cpp:334) composes into the real AnimSet lookup
	# key, "<this>_<mode fragment>_<right hand>_<left hand>". NOT the same as
	# `name`/the .creature sheet stem -- confirmed 2026-08-31 on real data:
	# every one of the 8 curated creatures' own base name is just "fy_hom" or
	# "fy_hof" (by gender only, same for every race -- all humanoid skeletons
	# share the same base locomotion animset). See
	# creature_anim.resolve_animation()/creatures_anim_cache.json for how
	# this turns into a real .anim filename per MBEHAV::EMode.
	anim_set_base_name: str = ""

	def to_dict(self) -> dict:
		return asdict(self)

	@staticmethod
	def from_dict(data: dict) -> "CreatureRecord":
		return CreatureRecord(
			name=data["name"], skel=data["skel"], race=data["race"], gender=data["gender"],
			slots=dict(data.get("slots", {})), hair=list(data.get("hair", [])),
			body_to_bone=dict(data.get("body_to_bone", {})),
			anim_set_base_name=data.get("anim_set_base_name", ""),
		)


def bundled_list_path() -> Path:
	"""The bundled default creatures_ref.txt's own path -- source the "copy
	to workspace" button copies from."""
	return _BUNDLED_LIST_PATH


def bundled_cache_path() -> Path:
	"""The bundled pre-generated cache for the bundled list's creatures --
	ships inside the Forgery package itself, so the default 8 reference
	creatures cost zero first-generation time."""
	return _BUNDLED_CACHE_PATH


def workspace_list_path(workspace_dir) -> Path:
	"""Where a workspace's own creatures_ref.txt override would live, if it
	existed -- used by the "copy to workspace" button as its destination."""
	return Path(workspace_dir) / _WORKSPACE_LIST_NAME


def workspace_cache_path(workspace_dir) -> Path:
	"""Where the rebuilt cache for a workspace-overridden list lives."""
	return Path(workspace_dir).joinpath(*_WORKSPACE_CACHE_RELATIVE_PATH)


def resolve_list_path(workspace_dir: Optional[Path]) -> Path:
	"""Same override-if-present pattern as panoply_config._resolve_cfg_path()."""
	if workspace_dir is not None:
		candidate = workspace_list_path(workspace_dir)
		if candidate.is_file():
			return candidate
	return _BUNDLED_LIST_PATH


def resolve_cache_path(workspace_dir: Optional[Path]) -> Path:
	"""Cache path matching resolve_list_path(): the workspace's own build/
	cache if a workspace override list exists, else the bundled pre-generated
	cache (already covers the bundled list's names, never needs rebuilding)."""
	if workspace_dir is not None and workspace_list_path(workspace_dir).is_file():
		return workspace_cache_path(workspace_dir)
	return _BUNDLED_CACHE_PATH


def load_creature_entries(path: Path) -> List[Tuple[str, str]]:
	"""Parses a creatures_ref.txt: "<sheet> [display name]" per line
	(whitespace-separated). `sheet` is the .creature sheet's stem, no
	extension -- entries here are always .creature (Nuno, 2026-08-30). The
	rest of the line is an optional free-form display label (may contain
	spaces, e.g. "Fyros Male") shown everywhere in Patina's UI and used as
	the cache key -- falls back to `sheet` itself when omitted. Blank lines
	and '#'-prefixed comments ignored. Returns (sheet, label) pairs."""
	if not path.is_file():
		return []
	entries = []
	for line in path.read_text(encoding="utf-8").splitlines():
		line = line.strip()
		if not line or line.startswith("#"):
			continue
		parts = line.split(None, 1)
		sheet = parts[0]
		label = parts[1].strip() if len(parts) > 1 else sheet
		entries.append((sheet, label))
	return entries


def save_cache(path: Path, records: Dict[str, CreatureRecord]) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	payload = {name: record.to_dict() for name, record in records.items()}
	path.write_text(json.dumps(payload, indent="\t", sort_keys=True), encoding="utf-8")


def load_cache(path: Path) -> Dict[str, CreatureRecord]:
	if not path.is_file():
		return {}
	payload = json.loads(path.read_text(encoding="utf-8"))
	return {name: CreatureRecord.from_dict(data) for name, data in payload.items()}


def shape_from_item(item, race: int, gender: int) -> str:
	"""Reproduces CCharacterCL::shapeFromItem()'s race/gender variant
	fallback (ryzom/client/src/character_cl.cpp): race-specific shape for
	the given gender, then (female only) the generic female shape, then the
	plain generic shape."""
	if gender == 0:  # GSGENDER::male
		sheet = {
			0: item.id_shape_fyros, 1: item.id_shape_matis,
			2: item.id_shape_tryker, 3: item.id_shape_zorai,
		}.get(race, "")
	else:  # female
		sheet = {
			0: item.id_shape_fyros_female, 1: item.id_shape_matis_female,
			2: item.id_shape_tryker_female, 3: item.id_shape_zorai_female,
		}.get(race, "")
		if not sheet:
			sheet = item.id_shape_female
	if not sheet:
		sheet = item.id_shape
	return sheet


def build_cache(
	entries: List[Tuple[str, str]], creature_packed_sheets: Path, item_packed_sheets: Path,
	sitem_packed_sheets: Path, sheet_id_names: Dict[int, str],
) -> Dict[str, "CreatureRecord"]:
	"""Builds distilled CreatureRecords for `entries` ((sheet, label) pairs,
	see load_creature_entries()) from real .packed_sheets files. This is the
	~6s-cold-load path (pynel.ryzom_packed_sheets parses the whole file,
	there's no per-entry random access, see
	nel/tools/pynel/docs/packed_sheets.md) -- callers should run it off the
	UI thread and/or only when the source creatures_ref.txt actually
	changed, never eagerly on every launch. Returned dict is keyed by label,
	not sheet."""
	from pynel import ryzom_packed_sheets as ps

	# Lowercased keys, never exact-case matched against: sheet_id.bin/Georges
	# sheet field casing is inconsistent across Ryzom's own data (e.g.
	# CCharacterSheet::build() lowercases Equipment.id_item via
	# toLowerAscii() but does NOT do the same for the skeleton filename --
	# confirmed on real data, 2026-08-30: a .creature's "3d data.Skel" field
	# read "FY_HOF_skel.skel" while the real file on disk is
	# "fy_hof_skel.skel"). Never assume any name here matches another
	# source's casing -- always lowercase both sides before comparing.
	name_to_id = build_name_to_id(sheet_id_names)

	creature_data = ps.load_creature_packed_sheets(creature_packed_sheets)
	item_data = ps.load_item_packed_sheets(item_packed_sheets)
	sitem_data = ps.load_item_packed_sheets(sitem_packed_sheets)
	items_by_id = {**item_data.entries, **sitem_data.entries}

	records: Dict[str, CreatureRecord] = {}
	for sheet_stem, label in entries:
		full_name = f"{sheet_stem}.creature"
		sheet_id = name_to_id.get(full_name.lower())
		sheet = creature_data.entries.get(sheet_id) if sheet_id is not None else None
		if sheet is None:
			print(f"[creature_ref] {full_name!r} (label {label!r}) not found in creature.packed_sheets, skipped")
			continue
		records[label] = creature_record_from_sheet(label, sheet, items_by_id, name_to_id)

	return records


def build_name_to_id(sheet_id_names: Dict[int, str]) -> Dict[str, int]:
	"""Lowercased sheet-name -> CSheetId lookup, shared by build_cache() and
	creature_full_index.py -- see build_cache()'s own docstring note on why
	lowercased (sheet_id.bin/Georges casing is inconsistent)."""
	name_to_id: Dict[str, int] = {}
	for sheet_id, sheet_name in sheet_id_names.items():
		name_to_id.setdefault(sheet_name.lower(), sheet_id)
	return name_to_id


def creature_record_from_sheet(label: str, sheet, items_by_id: dict, name_to_id: Dict[str, int]) -> "CreatureRecord":
	"""Distills one already-parsed CharacterSheet into a CreatureRecord --
	factored out of build_cache() so creature_full_index.py's uncurated,
	every-entry build can reuse the exact same field-by-field logic instead
	of drifting from it."""
	def resolve_item_shape(id_item: str, race: int, gender: int) -> str:
		if not id_item:
			return ""
		item_id = name_to_id.get(id_item.lower())
		item = items_by_id.get(item_id) if item_id is not None else None
		if item is None:
			return ""
		return shape_from_item(item, race, gender)

	slots = {}
	for slot_name in BODY_SLOTS:
		equipment = getattr(sheet, slot_name)
		shape = resolve_item_shape(equipment.id_item, sheet.race, sheet.gender)
		if shape:
			slots[slot_name] = shape

	hair = []
	for equipment in sheet.hair_item_list:
		shape = resolve_item_shape(equipment.id_item, sheet.race, sheet.gender)
		if shape:
			hair.append(shape)

	body_to_bone = {
		part: getattr(sheet.body_to_bone, part)
		for part in (
			"head", "chest", "left_arm", "right_arm", "left_hand",
			"right_hand", "left_leg", "right_leg", "left_foot", "right_foot",
		)
	}

	return CreatureRecord(
		name=label, skel=sheet.id_skel_filename, race=sheet.race, gender=sheet.gender,
		slots=slots, hair=hair, body_to_bone=body_to_bone,
		anim_set_base_name=sheet.id_anim_set_base_name,
	)


_ITEM_SHAPE_FIELDS = (
	"id_shape", "id_shape_female",
	"id_shape_fyros", "id_shape_fyros_female",
	"id_shape_matis", "id_shape_matis_female",
	"id_shape_tryker", "id_shape_tryker_female",
	"id_shape_zorai", "id_shape_zorai_female",
)


def build_shape_slot_index(item_data, sitem_data) -> Dict[str, str]:
	"""Distills a shape-filename-stem (lowercased, no extension) ->
	BODY_SLOTS entry index from every item.packed_sheets/sitem.packed_sheets
	entry's slot_bf + IdShape* fields -- lets the Bind preview
	auto-preselect which body part a loaded equipment .shape replaces (see
	object_editor.py's _auto_detect_bind_slot()) instead of requiring a
	manual pick every time. `item_data`/`sitem_data` are
	pynel.ryzom_packed_sheets.load_item_packed_sheets() results. A shape
	that maps to more than one distinct BODY_SLOTS entry across every item
	referencing it (or to none, e.g. RIGHT_HAND/LEFT_HAND-only items) is
	left out entirely -- ambiguous, the manual combo remains the only way
	to bind those. Covers the FULL item table (not just the curated
	creatures_ref.txt subset): the whole point is recognizing any shape a
	user might load, not just the 8 reference creatures' own equipment.
	Not run at Patina runtime -- offline-generated (like
	creatures_ref_cache.json) into the bundled shape_slot_index.json via
	save_shape_slot_index(), see bundled_shape_slot_index_path()."""
	shape_to_slots: Dict[str, set] = {}
	for item in {**item_data.entries, **sitem_data.entries}.values():
		body_slots = {
			slot_name for bit, slot_name in _SLOT_BIT_TO_BODY_SLOT.items()
			if item.slot_bf & (1 << bit)
		}
		if len(body_slots) != 1:
			continue
		slot_name = next(iter(body_slots))
		for field_name in _ITEM_SHAPE_FIELDS:
			shape = getattr(item, field_name)
			if not shape:
				continue
			stem = Path(shape).stem.lower()
			shape_to_slots.setdefault(stem, set()).add(slot_name)

	return {stem: next(iter(slots)) for stem, slots in shape_to_slots.items() if len(slots) == 1}


def save_shape_slot_index(path: Path, index: Dict[str, str]) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	path.write_text(json.dumps(index, indent="\t", sort_keys=True), encoding="utf-8")


def bundled_shape_slot_index_path() -> Path:
	"""The bundled pre-generated shape->BODY_SLOTS index (see
	build_shape_slot_index()) -- ships inside the Forgery package itself,
	same "distilled cache, zero runtime .packed_sheets parsing" pattern as
	bundled_cache_path()."""
	return _BUNDLED_SHAPE_SLOT_INDEX_PATH


_shape_slot_index_cache: Optional[Dict[str, str]] = None


def load_shape_slot_index() -> Dict[str, str]:
	"""Lazily loads+caches the bundled shape->BODY_SLOTS index for the
	process's lifetime -- a plain JSON read, no .packed_sheets parsing at
	runtime (see build_shape_slot_index())."""
	global _shape_slot_index_cache
	if _shape_slot_index_cache is None:
		if _BUNDLED_SHAPE_SLOT_INDEX_PATH.is_file():
			_shape_slot_index_cache = json.loads(_BUNDLED_SHAPE_SLOT_INDEX_PATH.read_text(encoding="utf-8"))
		else:
			_shape_slot_index_cache = {}
	return _shape_slot_index_cache


def workspace_slot_overrides_path(workspace_dir) -> Path:
	"""Where a workspace's own manual slot-override corrections live --
	shape-stem-lowercased -> BODY_SLOTS entry (or "hair"), the same shape
	as the bundled shape_slot_index.json but small/user-edited, layered on
	top of it (see load_effective_slot_overrides())."""
	return Path(workspace_dir).joinpath(*_WORKSPACE_SLOT_OVERRIDES_RELATIVE_PATH)


def load_slot_overrides(path: Path) -> Dict[str, str]:
	if not path.is_file():
		return {}
	return json.loads(path.read_text(encoding="utf-8"))


def save_slot_overrides(path: Path, overrides: Dict[str, str]) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	path.write_text(json.dumps(overrides, indent="\t", sort_keys=True), encoding="utf-8")


_BUNDLED_ANIM_CACHE_PATH = Path(__file__).parent / "creatures_anim_cache.json"

# MBEHAV::EMode names (ryzom/common/src/game_share/mode_and_behaviour.h)
# Patina's Bind preview exposes for animation preview -- a curated subset,
# restricted to modes mode2animset.string_array actually maps to a
# *distinct* fragment (UNKNOWN_MODE and NORMAL both -> "default" on real
# data, 2026-08-31, so only NORMAL is kept; COMBAT_FLOAT/MOUNT_SWIM/
# SWIM_DEATH dropped too -- variants of COMBAT/MOUNT_NORMAL/DEATH, not
# meaningfully different for a static standing-reference preview).
ANIM_MODES = ("NORMAL", "COMBAT", "SWIM", "SIT", "MOUNT_NORMAL", "REST", "DEATH")


def build_anim_cache(
	records: Dict[str, "CreatureRecord"], animset_list_packed_sheets_path: Path, mode2animset_path: Path,
) -> Dict[str, Dict[str, Dict[str, str]]]:
	"""Distills, for every distinct CreatureRecord.anim_set_base_name in
	`records`, {ANIM_MODES entry: {TAnimStateSheetId name: real .anim
	filename}} -- the same name-composition computeAnimSet() uses
	(ryzom/client/src/misc.cpp:334: "<base>_<mode fragment>_<right hand>_
	<left hand>", right/left hand left empty here -- no weapon-in-hand
	override for the curated reference creatures), resolved through the
	real animset_list.packed_sheets/mode2animset.string_array data (see
	nel/tools/pynel/docs/packed_sheets.md's own "animset_list.packed_sheets"
	section). Only the FIRST animation of each state's Animations list is
	kept (drops the Next/NextWeight random-variety alternatives) -- good
	enough for "preview this creature's Idle/Walk/Run", not a live-game
	accurate weighted choice. A mode missing from the real data entirely
	(confirmed 2026-08-31: EAT/ALERT/HUNGRY have no fy_hom_*/fy_hof_*
	animset on the maintainer's install) is silently skipped, same as the
	real client's own "fall back to default" behavior, just without the
	fallback itself -- the UI simply won't offer that mode's absent states.
	Not run at Patina runtime -- offline-generated (like
	shape_slot_index.json) into the bundled creatures_anim_cache.json via
	save_anim_cache()."""
	return build_anim_cache_from_bytes(
		records, Path(animset_list_packed_sheets_path).read_bytes(), Path(mode2animset_path).read_bytes())


def build_anim_cache_from_bytes(
	records: Dict[str, "CreatureRecord"], animset_list_packed_sheets_bytes: bytes, mode2animset_bytes: bytes,
) -> Dict[str, Dict[str, Dict[str, str]]]:
	"""Same as build_anim_cache(), but from already-read bytes -- needed by
	creature_full_index.py, whose source files may come from inside a .bnp
	archive (no real on-disk path to hand build_anim_cache() instead)."""
	from pynel import ryzom_packed_sheets as ps

	mode_fragments = ps.parse_mode2animset_string_array(mode2animset_bytes)
	packed = ps.parse_animation_set_list_packed_sheets(animset_list_packed_sheets_bytes)
	if len(packed.entries) != 1:
		raise ValueError(f"expected exactly 1 top-level ANIMATION_SET_LIST entry, got {len(packed.entries)}")
	sheet = next(iter(packed.entries.values()))
	anim_sets_by_name = {a.name.lower(): a for a in sheet.anim_set_list}

	base_names = {record.anim_set_base_name for record in records.values() if record.anim_set_base_name}
	result: Dict[str, Dict[str, Dict[str, str]]] = {}
	for base_name in base_names:
		per_mode: Dict[str, Dict[str, str]] = {}
		for mode_name in ANIM_MODES:
			fragment = mode_fragments.get(mode_name)
			if not fragment:
				continue
			composed = f"{base_name}_{fragment}__.animation_set".lower()
			anim_set = anim_sets_by_name.get(composed)
			if anim_set is None:
				continue
			states = {}
			for state in anim_set.animation_states:
				if not state.animations:
					continue
				state_name = (
					ps.ANIM_STATE_NAMES[state.state] if 0 <= state.state < len(ps.ANIM_STATE_NAMES) else str(state.state))
				states[state_name] = state.animations[0].id_anim
			if states:
				per_mode[mode_name] = states
		if per_mode:
			result[base_name] = per_mode
	return result


def save_anim_cache(path: Path, cache: Dict[str, Dict[str, Dict[str, str]]]) -> None:
	path.parent.mkdir(parents=True, exist_ok=True)
	path.write_text(json.dumps(cache, indent="\t", sort_keys=True), encoding="utf-8")


def bundled_anim_cache_path() -> Path:
	"""The bundled pre-generated {anim_set_base_name: {mode: {state: .anim}}}
	cache -- ships inside the Forgery package itself, same "distilled cache,
	zero runtime .packed_sheets parsing" pattern as bundled_cache_path()/
	bundled_shape_slot_index_path()."""
	return _BUNDLED_ANIM_CACHE_PATH


_anim_cache_cache: Optional[Dict[str, Dict[str, Dict[str, str]]]] = None


def load_anim_cache() -> Dict[str, Dict[str, Dict[str, str]]]:
	"""Lazily loads+caches the bundled animation cache for the process's
	lifetime -- a plain JSON read, no .packed_sheets parsing at runtime."""
	global _anim_cache_cache
	if _anim_cache_cache is None:
		if _BUNDLED_ANIM_CACHE_PATH.is_file():
			_anim_cache_cache = json.loads(_BUNDLED_ANIM_CACHE_PATH.read_text(encoding="utf-8"))
		else:
			_anim_cache_cache = {}
	return _anim_cache_cache


def resolve_animation(record: "CreatureRecord", mode_name: str, state_name: str = "Idle") -> Optional[str]:
	"""The real .anim filename for `record` playing `mode_name` in
	`state_name` (default "Idle", the standing-reference case Bind preview
	needs first), or None if the bundled cache has nothing for that
	combination (mode not offered for this creature's own anim_set_base_name,
	or that particular state has no animation in it)."""
	cache = load_anim_cache()
	per_mode = cache.get(record.anim_set_base_name, {})
	states = per_mode.get(mode_name, {})
	return states.get(state_name)
