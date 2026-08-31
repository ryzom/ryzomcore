#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Sonnet 5 (Anthropic) <noreply@anthropic.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Read Ryzom/NeL .packed_sheets files (Georges sheet binary cache) and sheet_id.bin.

Format reverse-engineered from nel/include/nel/georges/load_form.h (header +
dependency blocks), nel/include/nel/misc/sheet_id.h / .cpp (CSheetId, sheet_id.bin),
ryzom/client/src/sheet_manager.cpp (CSheetManagerEntry::serial, TypeVersion[]) and
ryzom/client/src/client_sheets/character_sheet.cpp (CCharacterSheet::serial) — see
nel/tools/pynel/docs/packed_sheets.md for the full writeup.

Supports `creature.packed_sheets` (CEntitySheet::FAUNA / CCharacterSheet) and
`item.packed_sheets`/`sitem.packed_sheets` (CEntitySheet::ITEM / CItemSheet).
Other sheet types (sbrick, mission, ...) raise PackedSheetsParseError.

Read-only: the client always regenerates this cache from the source Georges sheets,
there's no reason for pynel to write it back.

Usage:
	from pynel import ryzom_packed_sheets as ps
	packed = ps.parse_creature_packed_sheets(Path("creature.packed_sheets").read_bytes())
	names = ps.parse_sheet_id_bin(Path("sheet_id.bin").read_bytes())
	for sheet_id, sheet in packed.entries.items():
		print(names.get(sheet_id, f"#{sheet_id}"), sheet.race, sheet.max_speed)
"""

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, Dict, List, Union

MAGIC = b"HSKP"  # on-disk bytes for NELID("PKSH") (PACKED_SHEET_HEADER) on a little-endian machine
PACKED_SHEET_VERSION = 5

# CEntitySheet::TType (ryzom/client/src/client_sheets/entity_sheet.h) — order is the wire
# encoding, gaps (obsolete SPELL/SPELL_LIST/CAST_FX) included, do not reorder/compact.
ENTITY_SHEET_TYPES = [
	"CHAR", "FAUNA", "FLORA", "OBJECT", "FX", "BUILDING", "ITEM", "PLANT", "MISSION",
	"RACE_STATS", "PACT", "LIGHT_CYCLE", "WEATHER_SETUP", "CONTINENT", "WORLD",
	"WEATHER_FUNCTION_PARAMS", "UNKNOWN", "BOTCHAT", "MISSION_ICON", "SBRICK", "SPHRASE",
	"SKILLS_TREE", "UNBLOCK_TITLES", "SUCCESS_TABLE", "AUTOMATON_LIST",
	"ANIMATION_SET_LIST", "SPELL", "SPELL_LIST", "CAST_FX", "EMOT", "ANIMATION_FX",
	"ID_TO_STRING_ARRAY", "FORAGE_SOURCE", "CREATURE_ATTACK", "ANIMATION_FX_SET",
	"ATTACK_LIST", "SKY", "TEXT_EMOT", "OUTPOST", "OUTPOST_SQUAD", "OUTPOST_BUILDING",
	"FACTION",
]
FAUNA_TYPE = ENTITY_SHEET_TYPES.index("FAUNA")
ITEM_TYPE = ENTITY_SHEET_TYPES.index("ITEM")

# TypeVersion[] entry for "creature" in ryzom/client/src/sheet_manager.cpp
CREATURE_SHEET_VERSION = 17
# TypeVersion[] entry shared by "item" and "sitem" (both map to CItemSheet)
ITEM_SHEET_VERSION = 44

# ITEMFAMILY::EItemFamily (ryzom/common/src/game_share/item_family.h) — plain sequential
# auto-increment from UNDEFINED=0, no gaps. Order is the wire encoding, do not reorder.
ITEM_FAMILY_NAMES = [
	"UNDEFINED", "SERVICE", "ARMOR", "MELEE_WEAPON", "RANGE_WEAPON", "AMMO", "RAW_MATERIAL",
	"SHIELD", "CRAFTING_TOOL", "HARVEST_TOOL", "TAMING_TOOL", "TRAINING_TOOL", "AI", "BRICK",
	"FOOD", "JEWELRY", "CORPSE", "CARRION", "BAG", "STACK", "DEAD_SEED", "TELEPORT",
	"GUILD_FLAG", "LIVING_SEED", "LITTLE_SEED", "MEDIUM_SEED", "BIG_SEED", "VERY_BIG_SEED",
	"MISSION_ITEM", "CRYSTALLIZED_SPELL", "ITEM_SAP_RECHARGE", "PET_ANIMAL_TICKET",
	"GUILD_OPTION", "HANDLED_ITEM", "COSMETIC", "CONSUMABLE", "XP_CATALYSER", "SCROLL",
	"SCROLL_R2", "COMMAND_TICKET", "GENERIC_ITEM",
]


class PackedSheetsParseError(Exception):
	pass


@dataclass
class Equipment:
	"""CCharacterSheet::CEquipment (character_sheet.h)."""
	id_item: str
	texture: int
	color: int
	id_bind_point: str


@dataclass
class GroundFX:
	"""CGroundFXSheet (ground_fx_sheet.cpp)."""
	ground_id: int
	id_fx_name: str


@dataclass
class BodyToBone:
	"""CBodyToBoneSheet (body_to_bone_sheet.cpp)."""
	head: str
	chest: str
	left_arm: str
	right_arm: str
	left_hand: str
	right_hand: str
	left_leg: str
	right_leg: str
	left_foot: str
	right_foot: str


@dataclass
class Vector3:
	x: float
	y: float
	z: float


@dataclass
class Rgba:
	r: int
	g: int
	b: int
	a: int


@dataclass
class CastRay:
	"""CCharacterSheet::CCastRay (character_sheet.h, nested class)."""
	origin: Vector3
	pos: Vector3


@dataclass
class CharacterSheet:
	"""CCharacterSheet (ryzom/client/src/client_sheets/character_sheet.cpp), the
	CEntitySheet::FAUNA payload of a creature.packed_sheets entry."""
	sheet_id: int  # raw CSheetId (u32); resolve via sheet_id.bin for a readable name
	gender: int
	race: int  # EGSPD::CPeople::TPeople, kept raw (no Python-side name table yet)
	id_skel_filename: str
	id_anim_set_base_name: str
	id_automaton: str
	scale: float
	sound_family: int
	sound_variation: int
	id_lod_character_name: str
	lod_character_distance: float
	selectable: bool
	talkable: bool
	attackable: bool
	givable: bool
	mountable: bool
	turn: bool
	selectable_by_space: bool
	hl_state: int  # LHSTATE::TLHState, kept raw
	character_scale_pos: float
	name_pos_z_low: float
	name_pos_z_normal: float
	name_pos_z_high: float
	id_fame: str
	body: Equipment
	legs: Equipment
	arms: Equipment
	hands: Equipment
	feet: Equipment
	head: Equipment
	face: Equipment
	object_in_right_hand: Equipment
	object_in_left_hand: Equipment
	hair_color: int
	skin: int
	eyes_color: int
	dist_to_front: float
	dist_to_back: float
	dist_to_side: float
	col_radius: float
	col_height: float
	col_length: float
	col_width: float
	max_speed: float
	clip_radius: float
	clip_height: float
	id_alternative_clothes: List[str]
	hair_item_list: List[Equipment]
	ground_fx: List[GroundFX]
	display_osd: bool
	id_static_fx: str
	body_to_bone: BodyToBone
	attack_lists: List[str]
	display_in_radar: bool
	display_osd_name: bool
	display_osd_bars: bool
	display_osd_force_over: bool
	traversable: bool
	region_force: int
	force_level: int
	level: int
	projectile_cast_ray: List[CastRay]
	r2_npc: bool


@dataclass
class PackedSheets:
	"""Generic container: entries holds CharacterSheet for creature.packed_sheets,
	ItemSheet for item.packed_sheets/sitem.packed_sheets, keyed by raw CSheetId."""
	dictionary: List[str] = field(default_factory=list)  # source Georges filenames (informational)
	entries: Dict[int, object] = field(default_factory=dict)


@dataclass
class MpItemPart:
	"""CItemSheet::CMpItemPart (item_sheet.h) — element of ItemSheet.mp_item_parts."""
	origin_filter: int
	stats: List[int]  # RM_FABER_STAT_TYPE::NumRMStatType (34) entries, one per stat type


@dataclass
class Scroll:
	"""CItemSheet::CScroll — always present regardless of Family."""
	texture: str
	lua_command: str
	web_command: str
	label: str


@dataclass
class StaticFX:
	"""CItemFXSheet::CStaticFX (item_fx_sheet.h)."""
	name: str
	bone: str
	offset: Vector3


@dataclass
class ItemFX:
	"""CItemFXSheet (item_fx_sheet.cpp), the ItemSheet.fx field."""
	trail_min_slice_time: float
	trail_max_slice_time: float
	attack_fx_offset: Vector3
	trail: str
	advantage_fx: str
	attack_fx: str
	attack_fx_rot: Vector3
	impact_fx_delay: float
	static_fxs: List[StaticFX]


# CItemSheet's Family-specific union members (item_sheet.h). Exactly one of these (or
# none) is present on ItemSheet.family_data, selected by ItemSheet.family — see
# docs/packed_sheets.md for the Family -> struct dispatch table.
@dataclass
class Cosmetic:
	vp_value: int
	gender: int


@dataclass
class Armor:
	armor_type: int


@dataclass
class MeleeWeapon:
	weapon_type: int
	skill: int
	damage_type: int
	melee_range: int


@dataclass
class RangeWeapon:
	weapon_type: int
	skill: int
	range_weapon_type: int


@dataclass
class Ammo:
	skill: int
	damage_type: int
	magazine: int


@dataclass
class Mp:
	ecosystem: int
	mp_category: int
	harvest_skill: int
	family: int  # RM_FAMILY::TRMFamily -- unrelated to ItemSheet.family (ITEMFAMILY)
	item_part_bf: int
	used_as_craft_requirement: bool
	mp_color: int
	stat_energy: int


@dataclass
class Shield:
	shield_type: int


@dataclass
class Tool:
	"""Shared by CRAFTING_TOOL/HARVEST_TOOL/TAMING_TOOL families."""
	skill: int
	crafting_tool_type: int
	command_range: int
	max_donkey: int


@dataclass
class GuildOption:
	money_cost: int
	xp_cost: int


@dataclass
class Pet:
	slot: int


@dataclass
class Teleport:
	type: int


@dataclass
class Consumable:
	overdose_timer: int
	consumption_time: int
	properties: List[str]


@dataclass
class ItemSheet:
	"""CItemSheet (ryzom/client/src/client_sheets/item_sheet.cpp), the
	CEntitySheet::ITEM payload of an item.packed_sheets/sitem.packed_sheets entry."""
	sheet_id: int  # raw CSheetId (u32); resolve via sheet_id.bin for a readable name
	id_shape: str
	id_shape_female: str
	id_shape_fyros: str
	id_shape_fyros_female: str
	id_shape_matis: str
	id_shape_matis_female: str
	id_shape_tryker: str
	id_shape_tryker_female: str
	id_shape_zorai: str
	id_shape_zorai_female: str
	slot_bf: int
	map_variant: int
	family: int  # ITEMFAMILY::EItemFamily, see ITEM_FAMILY_NAMES; selects family_data below
	item_type: int
	id_icon_main: str
	id_icon_back: str
	id_icon_over: str
	id_icon_over2: str
	icon_color: Rgba
	icon_back_color: Rgba
	icon_over_color: Rgba
	icon_over2_color: Rgba
	id_icon_text: str
	id_anim_set: str
	color: int
	has_fx: bool
	drop_or_sell: bool
	is_item_no_rent: bool
	never_hide_when_equipped: bool
	stackable: int
	is_consumable: bool
	bulk: float
	equip_time: int
	fx: ItemFX
	id_effect1: str
	id_effect2: str
	id_effect3: str
	id_effect4: str
	mp_item_parts: List[MpItemPart]
	craft_plan: int  # raw CSheetId
	required_charac: int
	required_charac_level: int
	required_skill: int
	required_skill_level: int
	item_origin: int
	scroll: Scroll
	family_data: object  # one of Cosmetic/Armor/MeleeWeapon/.../Consumable, or None


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise PackedSheetsParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def u8(self) -> int:
		return self._take(1)[0]

	def s8(self) -> int:
		return struct.unpack("<b", self._take(1))[0]

	def u16(self) -> int:
		return struct.unpack("<H", self._take(2))[0]

	def u32(self) -> int:
		return struct.unpack("<I", self._take(4))[0]

	def s32(self) -> int:
		return struct.unpack("<i", self._take(4))[0]

	def u64(self) -> int:
		return struct.unpack("<Q", self._take(8))[0]

	def f32(self) -> float:
		return struct.unpack("<f", self._take(4))[0]

	def boolean(self) -> bool:
		return self.u8() != 0

	def string(self) -> str:
		length = self.u32()
		return self._take(length).decode("latin-1")

	def rgba(self) -> Rgba:
		return Rgba(self.u8(), self.u8(), self.u8(), self.u8())

	def cont_len(self) -> int:
		"""Length prefix used by serialCont() for generic containers."""
		return self.s32()

	def check_magic(self, expected: bytes) -> None:
		got = self._take(len(expected))
		if got != expected:
			raise PackedSheetsParseError(f"bad magic: expected {expected!r}, got {got!r}")

	def check_u32(self, expected: int, what: str) -> None:
		got = self.u32()
		if got != expected:
			raise PackedSheetsParseError(f"bad {what}: expected {expected}, got {got}")

	def skip_stream_version(self) -> None:
		"""Mirrors IStream::serialVersion: one byte, or 0xFF + a u32 — value unused here."""
		b = self.u8()
		if b == 0xFF:
			self.u32()

	def eof(self) -> bool:
		return self._pos >= len(self._data)

	@property
	def remaining(self) -> int:
		return len(self._data) - self._pos


def parse_sheet_id_bin(data: bytes) -> Dict[int, str]:
	"""sheet_id.bin: no header, just serialCont on std::map<uint32, std::string>
	(sheet id -> filename with extension, e.g. "ge_bear_c1.creature")."""
	f = _Reader(data)
	count = f.cont_len()
	result: Dict[int, str] = {}
	for _ in range(count):
		sheet_id = f.u32()
		name = f.string()
		result[sheet_id] = name

	if not f.eof():
		raise PackedSheetsParseError(f"{f.remaining} trailing bytes after parsing sheet_id.bin")

	return result


def _parse_equipment(f: _Reader) -> Equipment:
	id_item = f.string()
	texture = f.s8()
	color = f.s8()
	id_bind_point = f.string()
	return Equipment(id_item=id_item, texture=texture, color=color, id_bind_point=id_bind_point)


def _parse_ground_fx(f: _Reader) -> GroundFX:
	ground_id = f.u32()
	id_fx_name = f.string()
	return GroundFX(ground_id=ground_id, id_fx_name=id_fx_name)


def _parse_body_to_bone(f: _Reader) -> BodyToBone:
	return BodyToBone(
		head=f.string(), chest=f.string(),
		left_arm=f.string(), right_arm=f.string(),
		left_hand=f.string(), right_hand=f.string(),
		left_leg=f.string(), right_leg=f.string(),
		left_foot=f.string(), right_foot=f.string(),
	)


def _parse_vector3(f: _Reader) -> Vector3:
	return Vector3(f.f32(), f.f32(), f.f32())


def _parse_cast_ray(f: _Reader) -> CastRay:
	return CastRay(origin=_parse_vector3(f), pos=_parse_vector3(f))


def _parse_character_sheet(f: _Reader, sheet_id: int) -> CharacterSheet:
	gender = f.u8()
	race = f.s32()  # serialEnum
	id_skel_filename = f.string()
	id_anim_set_base_name = f.string()
	id_automaton = f.string()
	scale = f.f32()
	sound_family = f.u32()
	sound_variation = f.u32()
	id_lod_character_name = f.string()
	lod_character_distance = f.f32()
	selectable = f.boolean()
	talkable = f.boolean()
	attackable = f.boolean()
	givable = f.boolean()
	mountable = f.boolean()
	turn = f.boolean()
	selectable_by_space = f.boolean()
	hl_state = f.s32()  # serialEnum
	character_scale_pos = f.f32()
	name_pos_z_low = f.f32()
	name_pos_z_normal = f.f32()
	name_pos_z_high = f.f32()
	id_fame = f.string()

	body = _parse_equipment(f)
	legs = _parse_equipment(f)
	arms = _parse_equipment(f)
	hands = _parse_equipment(f)
	feet = _parse_equipment(f)
	head = _parse_equipment(f)
	face = _parse_equipment(f)
	object_in_right_hand = _parse_equipment(f)
	object_in_left_hand = _parse_equipment(f)

	hair_color = f.s8()
	skin = f.s8()
	eyes_color = f.s8()

	dist_to_front = f.f32()
	dist_to_back = f.f32()
	dist_to_side = f.f32()

	col_radius = f.f32()
	col_height = f.f32()
	col_length = f.f32()
	col_width = f.f32()
	max_speed = f.f32()

	clip_radius = f.f32()
	clip_height = f.f32()

	n_alt_clothes = f.cont_len()
	id_alternative_clothes = [f.string() for _ in range(n_alt_clothes)]

	n_hair_items = f.cont_len()
	hair_item_list = [_parse_equipment(f) for _ in range(n_hair_items)]

	n_ground_fx = f.cont_len()
	ground_fx = [_parse_ground_fx(f) for _ in range(n_ground_fx)]

	display_osd = f.boolean()
	id_static_fx = f.string()
	body_to_bone = _parse_body_to_bone(f)

	n_attack_lists = f.u32()  # manual loop in C++, not serialCont, but same wire shape
	attack_lists = [f.string() for _ in range(n_attack_lists)]

	display_in_radar = f.boolean()
	display_osd_name = f.boolean()
	display_osd_bars = f.boolean()
	display_osd_force_over = f.boolean()
	traversable = f.boolean()

	region_force = f.s8()
	force_level = f.s8()
	level = f.u16()

	n_cast_rays = f.cont_len()
	projectile_cast_ray = [_parse_cast_ray(f) for _ in range(n_cast_rays)]

	r2_npc = f.boolean()

	return CharacterSheet(
		sheet_id=sheet_id, gender=gender, race=race,
		id_skel_filename=id_skel_filename, id_anim_set_base_name=id_anim_set_base_name,
		id_automaton=id_automaton, scale=scale, sound_family=sound_family,
		sound_variation=sound_variation, id_lod_character_name=id_lod_character_name,
		lod_character_distance=lod_character_distance, selectable=selectable, talkable=talkable,
		attackable=attackable, givable=givable, mountable=mountable, turn=turn,
		selectable_by_space=selectable_by_space, hl_state=hl_state,
		character_scale_pos=character_scale_pos, name_pos_z_low=name_pos_z_low,
		name_pos_z_normal=name_pos_z_normal, name_pos_z_high=name_pos_z_high, id_fame=id_fame,
		body=body, legs=legs, arms=arms, hands=hands, feet=feet, head=head, face=face,
		object_in_right_hand=object_in_right_hand, object_in_left_hand=object_in_left_hand,
		hair_color=hair_color, skin=skin, eyes_color=eyes_color,
		dist_to_front=dist_to_front, dist_to_back=dist_to_back, dist_to_side=dist_to_side,
		col_radius=col_radius, col_height=col_height, col_length=col_length, col_width=col_width,
		max_speed=max_speed, clip_radius=clip_radius, clip_height=clip_height,
		id_alternative_clothes=id_alternative_clothes, hair_item_list=hair_item_list,
		ground_fx=ground_fx, display_osd=display_osd, id_static_fx=id_static_fx,
		body_to_bone=body_to_bone, attack_lists=attack_lists, display_in_radar=display_in_radar,
		display_osd_name=display_osd_name, display_osd_bars=display_osd_bars,
		display_osd_force_over=display_osd_force_over, traversable=traversable,
		region_force=region_force, force_level=force_level, level=level,
		projectile_cast_ray=projectile_cast_ray, r2_npc=r2_npc,
	)


def _parse_mp_item_part(f: _Reader) -> MpItemPart:
	origin_filter = f.u8()
	stats = [f.u8() for _ in range(34)]  # RM_FABER_STAT_TYPE::NumRMStatType
	return MpItemPart(origin_filter=origin_filter, stats=stats)


def _parse_scroll(f: _Reader) -> Scroll:
	return Scroll(texture=f.string(), lua_command=f.string(), web_command=f.string(), label=f.string())


def _parse_static_fx(f: _Reader) -> StaticFX:
	return StaticFX(name=f.string(), bone=f.string(), offset=_parse_vector3(f))


def _parse_item_fx(f: _Reader) -> ItemFX:
	trail_min_slice_time = f.f32()
	trail_max_slice_time = f.f32()
	attack_fx_offset = _parse_vector3(f)
	trail = f.string()
	advantage_fx = f.string()
	attack_fx = f.string()
	attack_fx_rot = _parse_vector3(f)
	impact_fx_delay = f.f32()
	n_static_fxs = f.cont_len()
	static_fxs = [_parse_static_fx(f) for _ in range(n_static_fxs)]
	return ItemFX(
		trail_min_slice_time=trail_min_slice_time, trail_max_slice_time=trail_max_slice_time,
		attack_fx_offset=attack_fx_offset, trail=trail, advantage_fx=advantage_fx, attack_fx=attack_fx,
		attack_fx_rot=attack_fx_rot, impact_fx_delay=impact_fx_delay, static_fxs=static_fxs,
	)


# Family -> union member parser (item_sheet.cpp:740 switch). Families not listed here
# (including SCROLL, whose data already went out via ItemSheet.scroll) carry no union data.
def _parse_family_data(f: _Reader, family: int):
	name = ITEM_FAMILY_NAMES[family] if 0 <= family < len(ITEM_FAMILY_NAMES) else None

	if name == "COSMETIC":
		return Cosmetic(vp_value=f.u32(), gender=f.s32())
	if name == "ARMOR":
		return Armor(armor_type=f.s32())
	if name == "MELEE_WEAPON":
		return MeleeWeapon(weapon_type=f.s32(), skill=f.s32(), damage_type=f.s32(), melee_range=f.s32())
	if name == "RANGE_WEAPON":
		return RangeWeapon(weapon_type=f.s32(), skill=f.s32(), range_weapon_type=f.s32())
	if name == "AMMO":
		return Ammo(skill=f.s32(), damage_type=f.s32(), magazine=f.s32())
	if name == "RAW_MATERIAL":
		ecosystem = f.s32()
		mp_category = f.s32()
		harvest_skill = f.s32()
		mp_family = f.s32()
		item_part_bf = f.u64()
		used_as_craft_requirement = f.boolean()
		mp_color = f.s8()
		stat_energy = f.u16()
		return Mp(ecosystem=ecosystem, mp_category=mp_category, harvest_skill=harvest_skill,
			family=mp_family, item_part_bf=item_part_bf,
			used_as_craft_requirement=used_as_craft_requirement, mp_color=mp_color, stat_energy=stat_energy)
	if name == "SHIELD":
		return Shield(shield_type=f.s32())
	if name in ("CRAFTING_TOOL", "HARVEST_TOOL", "TAMING_TOOL"):
		return Tool(skill=f.s32(), crafting_tool_type=f.s32(), command_range=f.s32(), max_donkey=f.s32())
	if name == "GUILD_OPTION":
		return GuildOption(money_cost=f.u32(), xp_cost=f.s32())
	if name == "PET_ANIMAL_TICKET":
		return Pet(slot=f.s32())
	if name == "TELEPORT":
		return Teleport(type=f.s32())
	if name == "CONSUMABLE":
		overdose_timer = f.u16()
		consumption_time = f.u16()
		n_properties = f.cont_len()
		properties = [f.string() for _ in range(n_properties)]
		return Consumable(overdose_timer=overdose_timer, consumption_time=consumption_time, properties=properties)

	return None  # SCROLL (data already in ItemSheet.scroll) and any other/unknown family


def _parse_item_sheet(f: _Reader, sheet_id: int) -> ItemSheet:
	id_shape = f.string()
	id_shape_female = f.string()
	id_shape_fyros = f.string()
	id_shape_fyros_female = f.string()
	id_shape_matis = f.string()
	id_shape_matis_female = f.string()
	id_shape_tryker = f.string()
	id_shape_tryker_female = f.string()
	id_shape_zorai = f.string()
	id_shape_zorai_female = f.string()
	slot_bf = f.u64()
	map_variant = f.u32()
	family = f.s32()  # serialEnum
	item_type = f.s32()  # serialEnum
	id_icon_main = f.string()
	id_icon_back = f.string()
	id_icon_over = f.string()
	id_icon_over2 = f.string()
	icon_color = f.rgba()
	icon_back_color = f.rgba()
	icon_over_color = f.rgba()
	icon_over2_color = f.rgba()
	id_icon_text = f.string()
	id_anim_set = f.string()
	color = f.s8()
	has_fx = f.boolean()
	drop_or_sell = f.boolean()
	is_item_no_rent = f.boolean()
	never_hide_when_equipped = f.boolean()
	stackable = f.u32()
	is_consumable = f.boolean()
	bulk = f.f32()
	equip_time = f.u32()

	fx = _parse_item_fx(f)

	id_effect1 = f.string()
	id_effect2 = f.string()
	id_effect3 = f.string()
	id_effect4 = f.string()

	n_mp_item_parts = f.cont_len()
	mp_item_parts = [_parse_mp_item_part(f) for _ in range(n_mp_item_parts)]

	craft_plan = f.u32()  # CSheetId

	required_charac = f.s32()  # serialEnum
	required_charac_level = f.u16()
	required_skill = f.s32()  # serialEnum
	required_skill_level = f.u16()

	item_origin = f.s32()  # serialEnum

	scroll = _parse_scroll(f)

	family_data = _parse_family_data(f, family)

	return ItemSheet(
		sheet_id=sheet_id, id_shape=id_shape, id_shape_female=id_shape_female,
		id_shape_fyros=id_shape_fyros, id_shape_fyros_female=id_shape_fyros_female,
		id_shape_matis=id_shape_matis, id_shape_matis_female=id_shape_matis_female,
		id_shape_tryker=id_shape_tryker, id_shape_tryker_female=id_shape_tryker_female,
		id_shape_zorai=id_shape_zorai, id_shape_zorai_female=id_shape_zorai_female,
		slot_bf=slot_bf, map_variant=map_variant, family=family, item_type=item_type,
		id_icon_main=id_icon_main, id_icon_back=id_icon_back, id_icon_over=id_icon_over,
		id_icon_over2=id_icon_over2, icon_color=icon_color, icon_back_color=icon_back_color,
		icon_over_color=icon_over_color, icon_over2_color=icon_over2_color,
		id_icon_text=id_icon_text, id_anim_set=id_anim_set, color=color, has_fx=has_fx,
		drop_or_sell=drop_or_sell, is_item_no_rent=is_item_no_rent,
		never_hide_when_equipped=never_hide_when_equipped, stackable=stackable,
		is_consumable=is_consumable, bulk=bulk, equip_time=equip_time, fx=fx,
		id_effect1=id_effect1, id_effect2=id_effect2, id_effect3=id_effect3, id_effect4=id_effect4,
		mp_item_parts=mp_item_parts, craft_plan=craft_plan, required_charac=required_charac,
		required_charac_level=required_charac_level, required_skill=required_skill,
		required_skill_level=required_skill_level, item_origin=item_origin, scroll=scroll,
		family_data=family_data,
	)


def _parse_packed_sheets_header(f: _Reader, expected_class_version: int, version_what: str):
	"""Header + dependency blocks common to every .packed_sheets file
	(load_form.h::loadForm). Returns (dictionary, entry_count)."""
	f.check_magic(MAGIC)
	f.check_u32(PACKED_SHEET_VERSION, "PACKED_SHEET_VERSION")
	f.skip_stream_version()

	depend_block_size = f.u32()
	depend_block_start = f._pos

	n_dict = f.cont_len()
	dictionary = [f.string() for _ in range(n_dict)]

	n_deps = f.u32()
	for _ in range(n_deps):
		f.u32()  # CSheetId of the dependent sheet
		n_dates = f.cont_len()
		for _ in range(n_dates):
			f.u32()  # modification date

	consumed = f._pos - depend_block_start
	if consumed != depend_block_size:
		raise PackedSheetsParseError(
			f"dependency block size mismatch: header says {depend_block_size}, consumed {consumed}"
		)

	n_entries = f.u32()
	f.check_u32(expected_class_version, version_what)

	n_map = f.cont_len()
	if n_map != n_entries:
		raise PackedSheetsParseError(f"entry count mismatch: header says {n_entries}, map has {n_map}")

	return dictionary, n_map


def _parse_entity_map(f: _Reader, n_map: int, expected_type: int, parse_payload, what: str) -> Dict[int, object]:
	entries: Dict[int, object] = {}
	for _ in range(n_map):
		sheet_id = f.u32()  # map key (CSheetId)

		sheet_type = f.s32()  # CSheetManagerEntry::serial: serialEnum(TType)
		if sheet_type != expected_type:
			type_name = ENTITY_SHEET_TYPES[sheet_type] if 0 <= sheet_type < len(ENTITY_SHEET_TYPES) else str(sheet_type)
			raise PackedSheetsParseError(
				f"unsupported sheet type {type_name!r} for CSheetId {sheet_id} "
				f"(only {what} is implemented, see docs/packed_sheets.md)"
			)

		entry_sheet_id = f.u32()  # CEntitySheet::Id, serialized again by initSheet()
		entries[sheet_id] = parse_payload(f, entry_sheet_id)

	return entries


def parse_creature_packed_sheets(data: bytes) -> PackedSheets:
	f = _Reader(data)
	dictionary, n_map = _parse_packed_sheets_header(f, CREATURE_SHEET_VERSION, "creature sheet class version")
	entries = _parse_entity_map(f, n_map, FAUNA_TYPE, _parse_character_sheet, "FAUNA/CCharacterSheet")

	if not f.eof():
		raise PackedSheetsParseError(f"{f.remaining} trailing bytes after parsing .packed_sheets content")

	return PackedSheets(dictionary=dictionary, entries=entries)


def parse_item_packed_sheets(data: bytes) -> PackedSheets:
	"""Parses both item.packed_sheets and sitem.packed_sheets — same class (CItemSheet),
	same version (44), only the source Georges extension differs."""
	f = _Reader(data)
	dictionary, n_map = _parse_packed_sheets_header(f, ITEM_SHEET_VERSION, "item sheet class version")
	entries = _parse_entity_map(f, n_map, ITEM_TYPE, _parse_item_sheet, "ITEM/CItemSheet")

	if not f.eof():
		raise PackedSheetsParseError(f"{f.remaining} trailing bytes after parsing .packed_sheets content")

	return PackedSheets(dictionary=dictionary, entries=entries)


def load_creature_packed_sheets(path: Union[str, Path, BinaryIO]) -> PackedSheets:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_creature_packed_sheets(data)


def load_item_packed_sheets(path: Union[str, Path, BinaryIO]) -> PackedSheets:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_item_packed_sheets(data)


def load_sheet_id_bin(path: Union[str, Path, BinaryIO]) -> Dict[int, str]:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_sheet_id_bin(data)


def _guess_kind(path: Path) -> str:
	stem = path.name.lower()
	if stem.startswith("creature"):
		return "creature"
	if stem.startswith("sitem") or stem.startswith("item"):
		return "item"
	raise PackedSheetsParseError(
		f"cannot guess sheet kind from filename {path.name!r}, pass --kind explicitly"
	)


def _dump_creature(packed: PackedSheets, names: Dict[int, str]) -> None:
	print(f"dictionary: {len(packed.dictionary)} source .creature files")
	print(f"entries: {len(packed.entries)}")
	for sheet_id, sheet in sorted(packed.entries.items(), key=lambda kv: names.get(kv[0], "")):
		name = names.get(sheet_id, f"#{sheet_id}")
		print(f"  {name}  race={sheet.race} gender={sheet.gender} scale={sheet.scale:.3f} "
			f"max_speed={sheet.max_speed:.3f} skel={sheet.id_skel_filename!r}")


def _dump_item(packed: PackedSheets, names: Dict[int, str]) -> None:
	print(f"dictionary: {len(packed.dictionary)} source .item/.sitem files")
	print(f"entries: {len(packed.entries)}")
	for sheet_id, sheet in sorted(packed.entries.items(), key=lambda kv: names.get(kv[0], "")):
		name = names.get(sheet_id, f"#{sheet_id}")
		family = ITEM_FAMILY_NAMES[sheet.family] if 0 <= sheet.family < len(ITEM_FAMILY_NAMES) else sheet.family
		print(f"  {name}  family={family} shape={sheet.id_shape!r} "
			f"stackable={sheet.stackable} bulk={sheet.bulk:.2f}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom .packed_sheets files (creature, item, sitem)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .packed_sheets file")
	p_dump.add_argument("path", type=Path)
	p_dump.add_argument("--kind", choices=("creature", "item"), default=None,
		help="sheet kind; guessed from the filename (creature*/item*/sitem*) if omitted")
	p_dump.add_argument("--sheet-id-bin", type=Path, default=None,
		help="loose sheet_id.bin path, to resolve CSheetId to readable names")
	p_dump.add_argument("--bnp", type=Path, default=None,
		help="leveldesign.bnp path to read sheet_id.bin from, instead of --sheet-id-bin")

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	if args.command == "dump":
		kind = args.kind or _guess_kind(args.path)

		names: Dict[int, str] = {}
		if args.sheet_id_bin:
			names = load_sheet_id_bin(args.sheet_id_bin)
		elif args.bnp:
			from pynel.ryzom_bnp import BnpReader
			names = parse_sheet_id_bin(BnpReader(args.bnp).read_file("sheet_id.bin"))

		if kind == "creature":
			_dump_creature(load_creature_packed_sheets(args.path), names)
		else:
			_dump_item(load_item_packed_sheets(args.path), names)


if __name__ == "__main__":
	_main()
