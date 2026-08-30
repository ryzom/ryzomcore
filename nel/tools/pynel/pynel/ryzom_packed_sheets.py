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

Only `creature.packed_sheets` (CEntitySheet::FAUNA / CCharacterSheet) is supported
so far. Other sheet types (item, sbrick, mission, ...) raise PackedSheetsParseError.

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

# TypeVersion[] entry for "creature" in ryzom/client/src/sheet_manager.cpp
CREATURE_SHEET_VERSION = 17


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
	dictionary: List[str] = field(default_factory=list)  # source .creature filenames (informational)
	entries: Dict[int, CharacterSheet] = field(default_factory=dict)  # keyed by raw CSheetId


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

	def f32(self) -> float:
		return struct.unpack("<f", self._take(4))[0]

	def boolean(self) -> bool:
		return self.u8() != 0

	def string(self) -> str:
		length = self.u32()
		return self._take(length).decode("latin-1")

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


def parse_creature_packed_sheets(data: bytes) -> PackedSheets:
	f = _Reader(data)

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
	f.check_u32(CREATURE_SHEET_VERSION, "creature sheet class version")

	n_map = f.cont_len()
	if n_map != n_entries:
		raise PackedSheetsParseError(f"entry count mismatch: header says {n_entries}, map has {n_map}")

	entries: Dict[int, CharacterSheet] = {}
	for _ in range(n_map):
		sheet_id = f.u32()  # map key (CSheetId)

		sheet_type = f.s32()  # CSheetManagerEntry::serial: serialEnum(TType)
		if sheet_type != FAUNA_TYPE:
			type_name = ENTITY_SHEET_TYPES[sheet_type] if 0 <= sheet_type < len(ENTITY_SHEET_TYPES) else str(sheet_type)
			raise PackedSheetsParseError(
				f"unsupported sheet type {type_name!r} for CSheetId {sheet_id} "
				f"(only FAUNA/CCharacterSheet is implemented, see docs/packed_sheets.md)"
			)

		entry_sheet_id = f.u32()  # CEntitySheet::Id, serialized again by initSheet()
		entries[sheet_id] = _parse_character_sheet(f, entry_sheet_id)

	if not f.eof():
		raise PackedSheetsParseError(f"{f.remaining} trailing bytes after parsing .packed_sheets content")

	return PackedSheets(dictionary=dictionary, entries=entries)


def load_creature_packed_sheets(path: Union[str, Path, BinaryIO]) -> PackedSheets:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_creature_packed_sheets(data)


def load_sheet_id_bin(path: Union[str, Path, BinaryIO]) -> Dict[int, str]:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_sheet_id_bin(data)


def _dump(packed: PackedSheets, names: Dict[int, str]) -> None:
	print(f"dictionary: {len(packed.dictionary)} source .creature files")
	print(f"entries: {len(packed.entries)}")
	for sheet_id, sheet in sorted(packed.entries.items(), key=lambda kv: names.get(kv[0], "")):
		name = names.get(sheet_id, f"#{sheet_id}")
		print(f"  {name}  race={sheet.race} gender={sheet.gender} scale={sheet.scale:.3f} "
			f"max_speed={sheet.max_speed:.3f} skel={sheet.id_skel_filename!r}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom creature.packed_sheets files")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a creature.packed_sheets file")
	p_dump.add_argument("path", type=Path)
	p_dump.add_argument("--sheet-id-bin", type=Path, default=None,
		help="loose sheet_id.bin path, to resolve CSheetId to readable names")
	p_dump.add_argument("--bnp", type=Path, default=None,
		help="leveldesign.bnp path to read sheet_id.bin from, instead of --sheet-id-bin")

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	if args.command == "dump":
		packed = load_creature_packed_sheets(args.path)
		names: Dict[int, str] = {}
		if args.sheet_id_bin:
			names = load_sheet_id_bin(args.sheet_id_bin)
		elif args.bnp:
			from pynel.ryzom_bnp import BnpReader
			names = parse_sheet_id_bin(BnpReader(args.bnp).read_file("sheet_id.bin"))
		_dump(packed, names)


if __name__ == "__main__":
	_main()
