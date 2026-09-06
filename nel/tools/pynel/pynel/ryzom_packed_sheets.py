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

Supports `creature.packed_sheets` (CEntitySheet::FAUNA / CCharacterSheet),
`item.packed_sheets`/`sitem.packed_sheets` (CEntitySheet::ITEM / CItemSheet),
`animset_list.packed_sheets` (CEntitySheet::ANIMATION_SET_LIST / CAnimationSetListSheet
-- the Mode/Behaviour -> real .anim filename mapping, see mode2Anim()/computeAnimSet()
in ryzom/client/src/misc.cpp for how a name like "fyhc1_NORMAL__.animation_set" gets
composed and looked up against AnimationSetSheet.name), `world.packed_sheets`
(CEntitySheet::WORLD / CWorldSheet -- continent locations and the in-game map
hierarchy) and `continent.packed_sheets` (CEntitySheet::CONTINENT / CContinentSheet
-- per-continent PACS/decor/lighting/weather/villages).
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
ANIMATION_SET_LIST_TYPE = ENTITY_SHEET_TYPES.index("ANIMATION_SET_LIST")
WORLD_TYPE = ENTITY_SHEET_TYPES.index("WORLD")
CONTINENT_TYPE = ENTITY_SHEET_TYPES.index("CONTINENT")

# TypeVersion[] entry for "creature" in ryzom/client/src/sheet_manager.cpp
CREATURE_SHEET_VERSION = 17
# TypeVersion[] entry shared by "item" and "sitem" (both map to CItemSheet)
ITEM_SHEET_VERSION = 44
# TypeVersion[] entry for "animset_list" (CAnimationSetListSheet)
ANIMATION_SET_LIST_SHEET_VERSION = 25
# TypeVersion[] entry for "world" (CWorldSheet)
WORLD_SHEET_VERSION = 1
# TypeVersion[] entry for "continent" (CContinentSheet)
CONTINENT_SHEET_VERSION = 12

# CAnimationStateSheet::TAnimStateSheetId (ryzom/client/src/client_sheets/
# animation_set_list_sheet.h) -- order is the wire encoding (CAnimationStateSheet.state
# field), do not reorder. StaticStateCount itself is a sentinel, not a real state.
ANIM_STATE_NAMES = [
	"Idle", "Run", "Walk", "TurnLeft", "TurnRight", "Emote",
	"CastGoodBegin", "CastGoodSuccess", "CastGoodFail", "CastGoodFumble",
	"CastBadBegin", "CastBadSuccess", "CastBadFail", "CastBadFumble",
	"CastNeutralBegin", "CastNeutralSuccess", "CastNeutralFail", "CastNeutralFumble",
	"OffensiveCastInit", "OffensiveCastBegin", "OffensiveCastLoop", "OffensiveCastFail",
	"OffensiveCastFumble", "OffensiveCastSuccess", "OffensiveCastLink",
	"CurativeCastInit", "CurativeCastBegin", "CurativeCastLoop", "CurativeCastFail",
	"CurativeCastFumble", "CurativeCastSuccess", "CurativeCastLink",
	"MixedCastInit", "MixedCastBegin", "MixedCastLoop", "MixedCastFail",
	"MixedCastFumble", "MixedCastSuccess", "MixedCastLink",
	"AcidCastInit", "BlindCastInit", "ColdCastInit", "ElecCastInit", "FearCastInit",
	"FireCastInit", "HealHPCastInit", "MadCastInit", "PoisonCastInit", "RootCastInit",
	"RotCastInit", "ShockCastInit", "SleepCastInit", "SlowCastInit", "StunCastInit",
	"AcidCastLoop", "BlindCastLoop", "ColdCastLoop", "ElecCastLoop", "FearCastLoop",
	"FireCastLoop", "HealHPCastLoop", "MadCastLoop", "PoisonCastLoop", "RootCastLoop",
	"RotCastLoop", "ShockCastLoop", "SleepCastLoop", "SlowCastLoop", "StunCastLoop",
	"AcidCastFail", "BlindCastFail", "ColdCastFail", "ElecCastFail", "FearCastFail",
	"FireCastFail", "HealHPCastFail", "MadCastFail", "PoisonCastFail", "RootCastFail",
	"RotCastFail", "ShockCastFail", "SleepCastFail", "SlowCastFail", "StunCastFail",
	"AcidCastEnd", "BlindCastEnd", "ColdCastEnd", "ElecCastEnd", "FearCastEnd",
	"FireCastEnd", "HealHPCastEnd", "MadCastEnd", "PoisonCastEnd", "RootCastEnd",
	"RotCastEnd", "ShockCastEnd", "SleepCastEnd", "SlowCastEnd", "StunCastEnd",
	"DefaultAtkLow", "DefaultAtkMiddle", "DefaultAtkHigh",
	"PowerfulAtkLow", "PowerfulAtkMiddle", "PowerfulAtkHigh",
	"AreaAtkLow", "AreaAtkMiddle", "AreaAtkHigh",
	"Attack1", "Attack2", "FirstPersonAttack",
	"Impact", "Death", "DeathIdle",
	"LootInit", "LootEnd", "ProspectingInit", "ProspectingEnd", "CareInit", "CareEnd",
	"UseInit", "UseBegin", "UseLoop", "UseEnd",
	"StunBegin", "StunLoop", "StunEnd",
	"SitMode", "SitEnd", "StrafeLeft", "StrafeRight",
]

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
class Vector2:
	x: float
	y: float


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
class AnimationFXStickMode:
	"""CFXStickMode (client_sheets/fx_stick_mode.h) — TStickMode enum kept raw (no
	Python-side name table yet), see fx_stick_mode.h for the values."""
	mode: int
	user_bone_name: str


@dataclass
class AnimationFXSheet:
	"""CAnimationFXSheet (animation_fx_sheet.h), element of AnimationFXSetSheet.fx."""
	ps_name: str
	stick_mode: AnimationFXStickMode
	user_param: List[float]  # 4 entries
	trajectory_anim: str
	color: Rgba
	scale_fx: bool
	repeat_mode: int  # CAnimationFXSheet::TRepeatMode (Loop=0/Respawn/RespawnAndCut)
	ray_ref_length: float


@dataclass
class AnimationFXSetSheet:
	"""CAnimationFXSetSheet (animation_fx_set_sheet.h) — not decoded for any real use
	yet (Patina doesn't render particle FX), only parsed here so the reader correctly
	advances past it inside CAnimationSheet."""
	fx: List[AnimationFXSheet]
	can_replace_stick_mode: List[bool]  # 4 entries
	can_replace_stick_bone: List[bool]  # 4 entries


@dataclass
class AnimationSheet:
	"""CAnimationSheet (client_sheets/animation_set_list_sheet.h) — one concrete
	animation choice within a AnimationStateSheet.animations list."""
	id_anim: str  # the real .anim filename, e.g. "fy_hom_normal_walk.anim"
	apply_character_scale_pos_factor: bool
	id_fx: str  # legacy single-fx name, superseded by fx_set
	head_controlable: bool
	virtual_rot: float
	fx_set: AnimationFXSetSheet
	reverse: bool
	hide_at_end_anim: bool
	next: List[int]  # alternative-animation indices into this same AnimationStateSheet.animations
	next_weight: List[int]  # parallel to next, relative pick weight
	job_restriction: int
	race_restriction: int  # EGSPD::CPeople::TPeople, raw


@dataclass
class AnimationStateSheet:
	"""CAnimationStateSheet (client_sheets/animation_set_list_sheet.h), element of
	AnimationSetSheet.animation_states -- one MBEHAV::EBehaviour-derived state
	(state_name/state are ANIM_STATE_NAMES[state])."""
	animations: List[AnimationSheet]
	state: int  # index into ANIM_STATE_NAMES
	state_name: str  # redundant with ANIM_STATE_NAMES[state], kept as an on-disk cross-check
	id_lod_character_animation: str
	display_objects: bool
	melee_impact_delay: float


@dataclass
class AnimationSetSheet:
	"""CAnimationSetSheet (client_sheets/animation_set_list_sheet.h), element of
	AnimationSetListSheet.anim_set_list -- name matches computeAnimSet()'s own
	composed lookup key, e.g. "fyhc1_NORMAL__.animation_set"."""
	name: str
	animation_states: List[AnimationStateSheet]  # sparse, indexed by AnimationStateSheet.state
	is_walk_essential: bool
	is_run_essential: bool


@dataclass
class AnimationSetListSheet:
	"""CAnimationSetListSheet (client_sheets/animation_set_list_sheet.h), the
	CEntitySheet::ANIMATION_SET_LIST payload of an animset_list.packed_sheets entry."""
	sheet_id: int
	anim_set_list: List[AnimationSetSheet]


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


@dataclass
class ContLoc:
	"""CWorldSheet::SContLoc (world_sheet.cpp), element of WorldSheet.cont_locs."""
	selection_name: str
	continent_name: str
	min_x: float
	min_y: float
	max_x: float
	max_y: float


@dataclass
class MapChild:
	"""CWorldSheet::SMap::SChild (world_sheet.cpp), element of Map.children."""
	name: str
	zone_name: str  # click zone, resolved against a region_*.primitive


@dataclass
class Map:
	"""CWorldSheet::SMap (world_sheet.cpp), element of WorldSheet.maps."""
	name: str
	continent_name: str  # empty if this map is the world map itself
	bitmap_name: str
	min_x: float
	min_y: float
	max_x: float
	max_y: float
	children: List[MapChild]


@dataclass
class WorldSheet:
	"""CWorldSheet (ryzom/client/src/client_sheets/world_sheet.cpp), the
	CEntitySheet::WORLD payload of a world.packed_sheets entry -- in practice there
	is only ever one real entry, keyed by sheet name "ryzom.world"."""
	sheet_id: int
	name: str
	cont_locs: List[ContLoc]
	maps: List[Map]


@dataclass
class DirLightSetup:
	"""CDirLightSetup (dir_light_setup.h)."""
	ambiant: Rgba
	diffuse: Rgba
	specular: Rgba
	direction: Vector3


@dataclass
class FogMapBuild:
	"""CFogMapBuild (fog_map_build.cpp)."""
	map: List[str]  # 6 entries, TMapType::NumMap (Day, Night, Dusk, Distance, Depth, NoPrecipitation)
	zone_min: str
	zone_max: str


@dataclass
class Zc:
	"""CContinentParameters::CZC (continent_sheet.cpp) -- a "zone constructible" entry."""
	name: str
	force_load_dist: float
	load_dist: float
	unload_dist: float
	enable_ruins: bool


@dataclass
class VillageIG:
	"""CVillageSheet::CVillageIG (village_sheet.h)."""
	ig_name: str
	parent_name: str


@dataclass
class VillageSheet:
	"""CVillageSheet (village_sheet.cpp), element of ContinentSheet.villages."""
	zone: str  # zone name, same convention as ContinentParameters.zone_min/zone_max
	altitude: float
	force_load_dist: float
	load_dist: float
	unload_dist: float
	center_x: float
	center_y: float
	width: int
	height: int
	rotation: float
	name: str
	igs: List[VillageIG]


@dataclass
class WeatherFunctionSheet:
	"""CWeatherFunctionSheet (weather_function_sheet.cpp), one per EGSPD::CSeason
	value (Spring=0, Summer, Autumn, Winter) on ContinentSheet.weather_function."""
	vegetable_min_bend_intensity: float
	vegetable_max_bend_intensity: float
	vegetable_min_wind_frequency: float
	vegetable_max_wind_frequency: float
	vegetable_max_bend_offset: float
	vegetable_wind_intensity_that_start_bend_offset: float
	tree_min_wind_intensity: float
	tree_max_wind_intensity: float
	setup_names: List[str]
	setup_weights: List[int]


@dataclass
class ContinentParameters:
	"""CContinentParameters (continent_sheet.cpp), the ContinentSheet.continent field."""
	name: str
	pacs_r_bank: str
	pacs_gr: str
	landscape_ig: str
	sky_day: str
	sky_night: str
	sky_fog_part_name: str
	background_ig_name: str
	canopy_ig_file_name: List[str]  # 4 entries, one per season (EGSPD::CSeason::Invalid)
	micro_veget: str
	small_bank: str
	far_bank: str
	coarse_mesh_map: str
	entity_sun_contribution_power: float
	entity_sun_contribution_max_threshold: float
	landscape_light_day: DirLightSetup
	landscape_light_dusk: DirLightSetup
	landscape_light_night: DirLightSetup
	landscape_point_light_material: Rgba
	entity_light_day: DirLightSetup
	entity_light_dusk: DirLightSetup
	entity_light_night: DirLightSetup
	root_light_day: DirLightSetup
	root_light_dusk: DirLightSetup
	root_light_night: DirLightSetup
	zc_list: List[Zc]
	fog_map_build: FogMapBuild
	fog_start: float
	fog_end: float
	root_fog_start: float
	root_fog_end: float
	indoor: bool
	world_map: str
	localized_name: str
	micro_life_zones: List[str]
	zone_min: str  # a zone NAME, not raw coordinates -- see zone_name_to_world_pos()
	zone_max: str
	tile_color_mono: List[bool]  # 4 entries, one per season
	tile_color_factor: List[float]  # 4 entries, one per season
	static_lighting_factor: List[float]  # 4 entries, one per season
	sky_sheet: List[str]  # 4 entries, one per season
	force_displayed_season: List[int]  # 4 entries, EGSPD::CSeason::TSeason, kept raw


@dataclass
class ContinentSheet:
	"""CContinentSheet (ryzom/client/src/client_sheets/continent_sheet.cpp), the
	CEntitySheet::CONTINENT payload of a continent.packed_sheets entry -- one per
	continent (Fyros, Matis, Tryker, Zorai, ...)."""
	sheet_id: int
	continent: ContinentParameters
	villages: List[VillageSheet]
	weather_function: List[WeatherFunctionSheet]  # 4 entries, one per EGSPD::CSeason value


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

	def f64(self) -> float:
		return struct.unpack("<d", self._take(8))[0]

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


def parse_mode2animset_string_array(data: bytes) -> Dict[str, str]:
	"""mode2animset.string_array: NOT a .packed_sheets file at all -- a raw Georges
	FORM, read straight from the XML tree at runtime by mode2Anim() (misc.cpp:265),
	never compiled into any binary cache. Confirmed real (2026-08-31): plain XML,
	<FORM><STRUCT><ARRAY Name="array"><ATOM Name="NORMAL" Value="default"/>...
	</ARRAY></STRUCT></FORM> -- one ATOM per MBEHAV::EMode name (mode_and_behaviour.h),
	its Value the animset name-fragment computeAnimSet() (misc.cpp:334) composes into
	"<AnimSetBaseName>_<fragment>_<rightHand>_<leftHand>". Returns {mode name: fragment},
	e.g. {"NORMAL": "default", "COMBAT": "combat", "SWIM": "swim", ...}."""
	import xml.etree.ElementTree as ET
	root = ET.fromstring(data)
	array = root.find(".//ARRAY[@Name='array']")
	if array is None:
		raise PackedSheetsParseError("mode2animset.string_array: no <ARRAY Name=\"array\"> node found")
	result: Dict[str, str] = {}
	for atom in array.findall("ATOM"):
		name = atom.get("Name")
		value = atom.get("Value")
		if name is not None and value is not None:
			result[name] = value
	return result


def load_mode2animset_string_array(path: Union[str, Path, BinaryIO]) -> Dict[str, str]:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_mode2animset_string_array(data)


def zone_name_to_world_pos(name: str) -> Vector2:
	"""Decodes a zone name (e.g. "160_ab") into its origin-corner world position,
	port of getPosFromZoneName() (ryzom/client/src/zone_util.cpp:33). Each zone tile
	is 160x160 units and its name gives its origin corner, not its extent -- for a
	true bounding box, callers must add 160 to both axes of the decoded max corner
	themselves (reproduces CContinent::getCorners(), continent.cpp:845-867; contrast
	CContinent::dumpVillagesLoadingZones(), continent.cpp:1202, a debug/screenshot
	helper that skips this +160 correction). Raises PackedSheetsParseError if name
	isn't a valid zone name (the C++ instead just returns false)."""
	stem = name.rsplit(".", 1)[0] if "." in name else name

	if "_" not in stem:
		raise PackedSheetsParseError(f"invalid zone name {name!r}: no '_' separator")
	row_str, x_str = stem.split("_", 1)

	if not row_str.isdigit():
		raise PackedSheetsParseError(f"invalid zone name {name!r}: {row_str!r} is not all digits")
	row = int(row_str)

	x_str = x_str.upper()
	if len(x_str) != 2 or not x_str.isalpha():
		raise PackedSheetsParseError(f"invalid zone name {name!r}: {x_str!r} is not exactly 2 letters")

	x = 160.0 * ((ord(x_str[0]) - ord("A")) * 26 + (ord(x_str[1]) - ord("A")))
	y = 160.0 * (-row)
	return Vector2(x, y)


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


def _parse_cont_loc(f: _Reader) -> ContLoc:
	selection_name = f.string()
	continent_name = f.string()
	min_x, min_y, max_x, max_y = f.f32(), f.f32(), f.f32(), f.f32()  # single multi-arg serial call in C++
	return ContLoc(
		selection_name=selection_name, continent_name=continent_name,
		min_x=min_x, min_y=min_y, max_x=max_x, max_y=max_y,
	)


def _parse_map_child(f: _Reader) -> MapChild:
	return MapChild(name=f.string(), zone_name=f.string())


def _parse_map(f: _Reader) -> Map:
	name = f.string()
	continent_name = f.string()
	bitmap_name = f.string()
	min_x = f.f32()
	min_y = f.f32()
	max_x = f.f32()
	max_y = f.f32()
	n_children = f.cont_len()
	children = [_parse_map_child(f) for _ in range(n_children)]
	return Map(
		name=name, continent_name=continent_name, bitmap_name=bitmap_name,
		min_x=min_x, min_y=min_y, max_x=max_x, max_y=max_y, children=children,
	)


def _parse_world_sheet(f: _Reader, sheet_id: int) -> WorldSheet:
	name = f.string()
	n_cont_locs = f.cont_len()
	cont_locs = [_parse_cont_loc(f) for _ in range(n_cont_locs)]
	n_maps = f.cont_len()
	maps = [_parse_map(f) for _ in range(n_maps)]
	return WorldSheet(sheet_id=sheet_id, name=name, cont_locs=cont_locs, maps=maps)


def _parse_dir_light_setup(f: _Reader) -> DirLightSetup:
	return DirLightSetup(
		ambiant=f.rgba(), diffuse=f.rgba(), specular=f.rgba(), direction=_parse_vector3(f),
	)


def _parse_fog_map_build(f: _Reader) -> FogMapBuild:
	map_ = [f.string() for _ in range(6)]  # TMapType::NumMap
	zone_min = f.string()
	zone_max = f.string()
	return FogMapBuild(map=map_, zone_min=zone_min, zone_max=zone_max)


def _parse_zc(f: _Reader) -> Zc:
	return Zc(
		name=f.string(), force_load_dist=f.f32(), load_dist=f.f32(),
		unload_dist=f.f32(), enable_ruins=f.boolean(),
	)


def _parse_village_ig(f: _Reader) -> VillageIG:
	return VillageIG(ig_name=f.string(), parent_name=f.string())


def _parse_village_sheet(f: _Reader) -> VillageSheet:
	zone = f.string()
	altitude = f.f32()
	force_load_dist = f.f32()
	load_dist = f.f32()
	unload_dist = f.f32()
	center_x = f.f32()
	center_y = f.f32()
	width = f.u32()
	height = f.u32()
	rotation = f.f32()
	name = f.string()
	n_igs = f.cont_len()
	igs = [_parse_village_ig(f) for _ in range(n_igs)]
	return VillageSheet(
		zone=zone, altitude=altitude, force_load_dist=force_load_dist, load_dist=load_dist,
		unload_dist=unload_dist, center_x=center_x, center_y=center_y, width=width,
		height=height, rotation=rotation, name=name, igs=igs,
	)


def _parse_weather_function_sheet(f: _Reader) -> WeatherFunctionSheet:
	vegetable_min_bend_intensity = f.f32()
	vegetable_max_bend_intensity = f.f32()
	vegetable_min_wind_frequency = f.f32()
	vegetable_max_wind_frequency = f.f32()
	vegetable_max_bend_offset = f.f32()
	vegetable_wind_intensity_that_start_bend_offset = f.f32()
	tree_min_wind_intensity = f.f32()
	tree_max_wind_intensity = f.f32()
	n_setup_names = f.cont_len()
	setup_names = [f.string() for _ in range(n_setup_names)]
	n_setup_weights = f.cont_len()
	setup_weights = [f.u32() for _ in range(n_setup_weights)]
	return WeatherFunctionSheet(
		vegetable_min_bend_intensity=vegetable_min_bend_intensity,
		vegetable_max_bend_intensity=vegetable_max_bend_intensity,
		vegetable_min_wind_frequency=vegetable_min_wind_frequency,
		vegetable_max_wind_frequency=vegetable_max_wind_frequency,
		vegetable_max_bend_offset=vegetable_max_bend_offset,
		vegetable_wind_intensity_that_start_bend_offset=vegetable_wind_intensity_that_start_bend_offset,
		tree_min_wind_intensity=tree_min_wind_intensity, tree_max_wind_intensity=tree_max_wind_intensity,
		setup_names=setup_names, setup_weights=setup_weights,
	)


def _parse_continent_parameters(f: _Reader) -> ContinentParameters:
	name = f.string()
	pacs_r_bank = f.string()
	pacs_gr = f.string()
	landscape_ig = f.string()
	sky_day = f.string()
	sky_night = f.string()
	sky_fog_part_name = f.string()
	background_ig_name = f.string()
	canopy_ig_file_name = [f.string() for _ in range(4)]  # EGSPD::CSeason::Invalid, fixed loop
	micro_veget = f.string()
	small_bank = f.string()
	far_bank = f.string()
	coarse_mesh_map = f.string()
	entity_sun_contribution_power = f.f32()
	entity_sun_contribution_max_threshold = f.f32()
	landscape_light_day = _parse_dir_light_setup(f)
	landscape_light_dusk = _parse_dir_light_setup(f)
	landscape_light_night = _parse_dir_light_setup(f)
	landscape_point_light_material = f.rgba()
	entity_light_day = _parse_dir_light_setup(f)
	entity_light_dusk = _parse_dir_light_setup(f)
	entity_light_night = _parse_dir_light_setup(f)
	root_light_day = _parse_dir_light_setup(f)
	root_light_dusk = _parse_dir_light_setup(f)
	root_light_night = _parse_dir_light_setup(f)
	n_zc = f.cont_len()
	zc_list = [_parse_zc(f) for _ in range(n_zc)]
	fog_map_build = _parse_fog_map_build(f)
	fog_start = f.f32()
	fog_end = f.f32()
	root_fog_start = f.f32()
	root_fog_end = f.f32()
	indoor = f.boolean()
	world_map = f.string()
	localized_name = f.string()
	n_micro_life_zones = f.cont_len()
	micro_life_zones = [f.string() for _ in range(n_micro_life_zones)]
	zone_min = f.string()
	zone_max = f.string()

	tile_color_mono: List[bool] = []
	tile_color_factor: List[float] = []
	static_lighting_factor: List[float] = []
	sky_sheet: List[str] = []
	force_displayed_season: List[int] = []
	for _ in range(4):  # EGSPD::CSeason::Invalid, fixed loop, fields interleaved per iteration
		tile_color_mono.append(f.boolean())
		tile_color_factor.append(f.f32())
		static_lighting_factor.append(f.f32())
		sky_sheet.append(f.string())
		force_displayed_season.append(f.s32())  # serialEnum

	return ContinentParameters(
		name=name, pacs_r_bank=pacs_r_bank, pacs_gr=pacs_gr, landscape_ig=landscape_ig,
		sky_day=sky_day, sky_night=sky_night, sky_fog_part_name=sky_fog_part_name,
		background_ig_name=background_ig_name, canopy_ig_file_name=canopy_ig_file_name,
		micro_veget=micro_veget, small_bank=small_bank, far_bank=far_bank,
		coarse_mesh_map=coarse_mesh_map, entity_sun_contribution_power=entity_sun_contribution_power,
		entity_sun_contribution_max_threshold=entity_sun_contribution_max_threshold,
		landscape_light_day=landscape_light_day, landscape_light_dusk=landscape_light_dusk,
		landscape_light_night=landscape_light_night, landscape_point_light_material=landscape_point_light_material,
		entity_light_day=entity_light_day, entity_light_dusk=entity_light_dusk,
		entity_light_night=entity_light_night, root_light_day=root_light_day,
		root_light_dusk=root_light_dusk, root_light_night=root_light_night,
		zc_list=zc_list, fog_map_build=fog_map_build, fog_start=fog_start, fog_end=fog_end,
		root_fog_start=root_fog_start, root_fog_end=root_fog_end, indoor=indoor,
		world_map=world_map, localized_name=localized_name, micro_life_zones=micro_life_zones,
		zone_min=zone_min, zone_max=zone_max, tile_color_mono=tile_color_mono,
		tile_color_factor=tile_color_factor, static_lighting_factor=static_lighting_factor,
		sky_sheet=sky_sheet, force_displayed_season=force_displayed_season,
	)


def _parse_continent_sheet(f: _Reader, sheet_id: int) -> ContinentSheet:
	continent = _parse_continent_parameters(f)
	n_villages = f.cont_len()
	villages = [_parse_village_sheet(f) for _ in range(n_villages)]
	weather_function = [_parse_weather_function_sheet(f) for _ in range(4)]  # EGSPD::CSeason::Invalid, fixed loop
	return ContinentSheet(
		sheet_id=sheet_id, continent=continent, villages=villages, weather_function=weather_function,
	)


def _parse_fx_stick_mode(f: _Reader) -> AnimationFXStickMode:
	mode = f.s32()  # serialEnum
	user_bone_name = f.string()  # CStringMapper::localSerialString -- plain string on disk
	return AnimationFXStickMode(mode=mode, user_bone_name=user_bone_name)


def _parse_animation_fx_sheet(f: _Reader) -> AnimationFXSheet:
	ps_name = f.string()
	stick_mode = _parse_fx_stick_mode(f)
	user_param = [f.f32() for _ in range(4)]
	trajectory_anim = f.string()
	color = f.rgba()
	scale_fx = f.boolean()
	repeat_mode = f.s32()  # serialEnum
	ray_ref_length = f.f32()
	return AnimationFXSheet(
		ps_name=ps_name, stick_mode=stick_mode, user_param=user_param,
		trajectory_anim=trajectory_anim, color=color, scale_fx=scale_fx,
		repeat_mode=repeat_mode, ray_ref_length=ray_ref_length,
	)


def _parse_animation_fx_set_sheet(f: _Reader) -> AnimationFXSetSheet:
	n = f.cont_len()
	fx = [_parse_animation_fx_sheet(f) for _ in range(n)]
	# CAnimationFXSetSheet::serial: interleaved per index, NOT two separate arrays --
	# f.serial(CanReplaceStickMode[k]); f.serial(CanReplaceStickBone[k]); for k in 0..3.
	can_replace_stick_mode = []
	can_replace_stick_bone = []
	for _ in range(4):  # CAnimationFXSetSheet::MaxNumFX
		can_replace_stick_mode.append(f.boolean())
		can_replace_stick_bone.append(f.boolean())
	return AnimationFXSetSheet(
		fx=fx, can_replace_stick_mode=can_replace_stick_mode, can_replace_stick_bone=can_replace_stick_bone,
	)


def _parse_animation_sheet(f: _Reader) -> AnimationSheet:
	id_anim = f.string()  # CStaticStringMapper::serial -- plain string on disk
	apply_character_scale_pos_factor = f.boolean()
	id_fx = f.string()
	head_controlable = f.boolean()
	virtual_rot = f.f64()
	fx_set = _parse_animation_fx_set_sheet(f)
	reverse = f.boolean()
	hide_at_end_anim = f.boolean()
	n_next = f.cont_len()
	next_ = [f.s8() for _ in range(n_next)]
	n_next_weight = f.cont_len()
	next_weight = [f.u16() for _ in range(n_next_weight)]
	job_restriction = f.u32()
	race_restriction = f.s32()  # serialEnum
	return AnimationSheet(
		id_anim=id_anim, apply_character_scale_pos_factor=apply_character_scale_pos_factor,
		id_fx=id_fx, head_controlable=head_controlable, virtual_rot=virtual_rot, fx_set=fx_set,
		reverse=reverse, hide_at_end_anim=hide_at_end_anim, next=next_, next_weight=next_weight,
		job_restriction=job_restriction, race_restriction=race_restriction,
	)


def _parse_animation_state_sheet(f: _Reader) -> AnimationStateSheet:
	n = f.cont_len()
	animations = [_parse_animation_sheet(f) for _ in range(n)]
	state = f.u16()
	state_name = f.string()
	id_lod_character_animation = f.string()  # CStaticStringMapper::serial -- plain string
	display_objects = f.boolean()
	melee_impact_delay = f.f32()
	return AnimationStateSheet(
		animations=animations, state=state, state_name=state_name,
		id_lod_character_animation=id_lod_character_animation,
		display_objects=display_objects, melee_impact_delay=melee_impact_delay,
	)


def _parse_animation_set_sheet(f: _Reader) -> AnimationSetSheet:
	name = f.string()
	n = f.cont_len()
	animation_states = [_parse_animation_state_sheet(f) for _ in range(n)]
	is_walk_essential = f.boolean()
	is_run_essential = f.boolean()
	return AnimationSetSheet(
		name=name, animation_states=animation_states,
		is_walk_essential=is_walk_essential, is_run_essential=is_run_essential,
	)


def _parse_animation_set_list_sheet(f: _Reader, sheet_id: int) -> AnimationSetListSheet:
	n = f.cont_len()
	anim_set_list = [_parse_animation_set_sheet(f) for _ in range(n)]
	return AnimationSetListSheet(sheet_id=sheet_id, anim_set_list=anim_set_list)


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


def parse_animation_set_list_packed_sheets(data: bytes) -> PackedSheets:
	f = _Reader(data)
	dictionary, n_map = _parse_packed_sheets_header(
		f, ANIMATION_SET_LIST_SHEET_VERSION, "animation set list sheet class version")
	entries = _parse_entity_map(
		f, n_map, ANIMATION_SET_LIST_TYPE, _parse_animation_set_list_sheet,
		"ANIMATION_SET_LIST/CAnimationSetListSheet")

	if not f.eof():
		raise PackedSheetsParseError(f"{f.remaining} trailing bytes after parsing .packed_sheets content")

	return PackedSheets(dictionary=dictionary, entries=entries)


def parse_world_packed_sheets(data: bytes) -> PackedSheets:
	f = _Reader(data)
	dictionary, n_map = _parse_packed_sheets_header(f, WORLD_SHEET_VERSION, "world sheet class version")
	entries = _parse_entity_map(f, n_map, WORLD_TYPE, _parse_world_sheet, "WORLD/CWorldSheet")

	if not f.eof():
		raise PackedSheetsParseError(f"{f.remaining} trailing bytes after parsing .packed_sheets content")

	return PackedSheets(dictionary=dictionary, entries=entries)


def parse_continent_packed_sheets(data: bytes) -> PackedSheets:
	f = _Reader(data)
	dictionary, n_map = _parse_packed_sheets_header(f, CONTINENT_SHEET_VERSION, "continent sheet class version")
	entries = _parse_entity_map(f, n_map, CONTINENT_TYPE, _parse_continent_sheet, "CONTINENT/CContinentSheet")

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


def load_animation_set_list_packed_sheets(path: Union[str, Path, BinaryIO]) -> PackedSheets:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_animation_set_list_packed_sheets(data)


def load_world_packed_sheets(path: Union[str, Path, BinaryIO]) -> PackedSheets:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_world_packed_sheets(data)


def load_continent_packed_sheets(path: Union[str, Path, BinaryIO]) -> PackedSheets:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_continent_packed_sheets(data)


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
	if stem.startswith("animset_list"):
		return "animset_list"
	if stem.startswith("world"):
		return "world"
	if stem.startswith("continent"):
		return "continent"
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


def _dump_animset_list(packed: PackedSheets, names: Dict[int, str]) -> None:
	print(f"dictionary: {len(packed.dictionary)} source .animset_list files")
	print(f"entries: {len(packed.entries)}")
	for sheet_id, sheet in sorted(packed.entries.items(), key=lambda kv: names.get(kv[0], "")):
		name = names.get(sheet_id, f"#{sheet_id}")
		print(f"  {name}  anim_sets={len(sheet.anim_set_list)}")
		for anim_set in sheet.anim_set_list:
			states = ", ".join(
				ANIM_STATE_NAMES[s.state] if 0 <= s.state < len(ANIM_STATE_NAMES) else str(s.state)
				for s in anim_set.animation_states if s.animations)
			print(f"    {anim_set.name!r}  states=[{states}]")


def _dump_world(packed: PackedSheets, names: Dict[int, str]) -> None:
	print(f"dictionary: {len(packed.dictionary)} source .world files")
	print(f"entries: {len(packed.entries)}")
	for sheet_id, sheet in sorted(packed.entries.items(), key=lambda kv: names.get(kv[0], "")):
		name = names.get(sheet_id, f"#{sheet_id}")
		print(f"  {name}  continents={len(sheet.cont_locs)} maps={len(sheet.maps)}")
		for cont_loc in sheet.cont_locs:
			print(f"    {cont_loc.selection_name!r} -> {cont_loc.continent_name!r} "
				f"bounds=({cont_loc.min_x:.1f},{cont_loc.min_y:.1f})-({cont_loc.max_x:.1f},{cont_loc.max_y:.1f})")


def _dump_continent(packed: PackedSheets, names: Dict[int, str]) -> None:
	print(f"dictionary: {len(packed.dictionary)} source .continent files")
	print(f"entries: {len(packed.entries)}")
	for sheet_id, sheet in sorted(packed.entries.items(), key=lambda kv: names.get(kv[0], "")):
		name = names.get(sheet_id, f"#{sheet_id}")
		c = sheet.continent
		print(f"  {name}  villages={len(sheet.villages)} indoor={c.indoor} "
			f"zone_min={c.zone_min!r} zone_max={c.zone_max!r}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(
		description="Read Ryzom .packed_sheets files (creature, item, sitem, animset_list, world, continent)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .packed_sheets file")
	p_dump.add_argument("path", type=Path)
	p_dump.add_argument("--kind", choices=("creature", "item", "animset_list", "world", "continent"), default=None,
		help="sheet kind; guessed from the filename "
			"(creature*/item*/sitem*/animset_list*/world*/continent*) if omitted")
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
		elif kind == "animset_list":
			_dump_animset_list(load_animation_set_list_packed_sheets(args.path), names)
		elif kind == "world":
			_dump_world(load_world_packed_sheets(args.path), names)
		elif kind == "continent":
			_dump_continent(load_continent_packed_sheets(args.path), names)
		else:
			_dump_item(load_item_packed_sheets(args.path), names)


if __name__ == "__main__":
	_main()
