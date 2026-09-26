#!/usr/bin/env python3
# Copyright (C) 2026  Nuno Gonçalves (Ulukyn) <nuno@troispetits.net>
# Copyright (C) 2026  Claude Opus 5.5 (Anthropic) <noreply@anthropic.com>
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

"""Reader/writer for Ryzom client interface config files (.icfg).

Chantier: project-todos/pynel/icfg_read_write.md

Format reverse-engineered from ryzom/client/src/interface_v3/interface_manager.cpp
(CInterfaceManager::loadConfig / saveConfig, ICFG_STREAM_VERSION 11) and the
sub-object serializers it calls: people_interraction.cpp, filtered_chat_summary.cpp,
chat_window.cpp (free tellers), interface_config.cpp (desktop images, UI:SAVE
database), task_bar_manager.cpp, continent_manager.cpp (landmarks),
action_handler_help.cpp (info windows), sphrase_manager.cpp (macro memory),
group_in_scene_bubble.cpp, and the widget serialConfig() of group_map.cpp,
group_compas.cpp, group_editbox.cpp and CGroupContainerWindows.

Usage:
	from pynel.ryzom_icfg import load_icfg, save_icfg
	cfg = load_icfg("save/interface_player.icfg")
	for leaf in cfg.database:
		print(leaf.name, leaf.value)
	save_icfg("save/interface_player.icfg", cfg)

Every file version from 0 to 11 is read; writing always emits version 11
(same behaviour as the client: re-saving a file upgrades it).
"""

import argparse
import dataclasses
import json
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, List, Optional, Union

ICFG_STREAM_VERSION = 11
MAX_NUM_USER_CHATS = 5
MAX_DYN_CHAN_PER_PLAYER = 8
TASKBAR_NUM_BARS = 10
TASKBAR_NUM_SHORTCUTS_PER_BAR = 10
UI_ID_PREFIX = "ui:interface:"

# CGroupContainer constructor defaults, used when upgrading a SCont older than v10
DEFAULT_CONTAINER_MIN_W = 222
DEFAULT_CONTAINER_MAX_W = 320

COMPASS_TYPE_POS_TRACKER = 6


def _nelid(text: str) -> bytes:
	"""On-disk bytes of NELID("XXXX") on a little-endian machine."""
	return text.encode("ascii")[::-1]


class IcfgParseError(Exception):
	pass


@dataclass
class Vector2:
	x: float
	y: float


@dataclass
class FilteredChatSummary:
	src_guild: bool = False
	src_team: bool = False
	src_around_me: bool = False
	src_tell: bool = False
	src_system_info: bool = False
	target: int = 0
	src_universe: bool = False
	src_region: bool = False


@dataclass
class UserChats:
	chat_group: Optional[FilteredChatSummary] = None
	user_chats: List[Optional[FilteredChatSummary]] = field(
		default_factory=lambda: [None] * MAX_NUM_USER_CHATS
	)
	tab_index: int = 0
	the_user_chat: Optional[FilteredChatSummary] = None
	free_tellers: Optional[List[str]] = None


@dataclass
class ContainerImage:
	"""CInterfaceConfig::SCont -- saved state of one group container."""
	id: str
	container_mode: int = 0
	popuped: bool = False
	opened: bool = False
	x: int = 0
	y: int = 0
	w: int = 0
	h: int = 0
	min_w: int = DEFAULT_CONTAINER_MIN_W
	max_w: int = DEFAULT_CONTAINER_MAX_W
	active_savable: bool = True
	active: bool = False
	scroll_pos: int = 0
	locked: bool = False
	movable: bool = True
	bg_alpha: int = 255
	content_alpha: int = 255
	rollover_alpha_content: int = 255
	rollover_alpha_container: int = 255
	use_global_alpha: bool = True
	popup_x: int = -1
	popup_y: int = -1
	popup_w: int = -1
	popup_h: int = -1
	popup_max_h: int = 512
	backuped_position_valid: bool = False
	backup_x: int = 0
	backup_y: int = 0
	touch_flag: bool = False


@dataclass
class MapConfig:
	"""CGroupMap::serialConfig."""
	user_scale: float
	world_offset: Vector2
	old_offset: Optional[Vector2] = None
	h: Optional[int] = None


@dataclass
class ContainerWindowsConfig:
	"""CGroupContainerWindows::serialConfig."""
	show_desktops: bool


@dataclass
class EditBoxConfig:
	"""CGroupEditBox::serialConfig."""
	input_string: str
	cursor_pos: int
	prev_num_line: int


@dataclass
class CompassConfig:
	"""CGroupCompas::serialConfig (a CCompassTarget)."""
	pos: Vector2
	name: str
	language: str
	target_type: int
	pos_tracker_raw: Optional[bytes] = None


@dataclass
class RawWidgetConfig:
	data: bytes


WidgetConfigData = Union[MapConfig, ContainerWindowsConfig, EditBoxConfig, CompassConfig, RawWidgetConfig]


@dataclass
class WidgetConfig:
	id: str
	config: WidgetConfigData


@dataclass
class DesktopExtra:
	top_window: str = ""
	widgets: List[WidgetConfig] = field(default_factory=list)


@dataclass
class DesktopImage:
	containers: List[ContainerImage] = field(default_factory=list)
	extra: Optional[DesktopExtra] = None


@dataclass
class DbLeaf:
	name: str
	value: int
	old_value: int


@dataclass
class TaskBarShortcut:
	sheet_type: int = 0
	db_sheet: str = ""
	macro_id: int = -1


@dataclass
class UserLandmark:
	pos: Vector2
	title: str
	type: int


@dataclass
class ContinentLandmarks:
	continent: str
	landmarks: List[UserLandmark] = field(default_factory=list)


@dataclass
class InfoWindowPos:
	x: int
	y: int


@dataclass
class MacroMemorySlot:
	memory_line: int
	memory_slot: int
	macro_id: int


@dataclass
class InterfaceConfig:
	version: int = ICFG_STREAM_VERSION
	user_chats: Optional[UserChats] = None
	current_mode: int = 0
	last_screen_w: Optional[int] = None
	last_screen_h: Optional[int] = None
	desktops: List[Optional[DesktopImage]] = field(default_factory=list)
	ui_db_save_version: int = 0
	database: Optional[List[DbLeaf]] = None
	taskbar: Optional[List[TaskBarShortcut]] = None
	landmarks: List[ContinentLandmarks] = field(default_factory=list)
	info_windows: Optional[List[InfoWindowPos]] = None
	macro_memory: Optional[List[MacroMemorySlot]] = None
	ignored_context_help: Optional[List[str]] = None
	dyn_chat: Optional[List[bool]] = None
	trailing: bytes = b""


# Widget ids whose serialConfig() class is known from the interface XML.
KNOWN_WIDGET_TYPES = {
	"ui:interface:map:content:map_content:actual_map": MapConfig,
	"ui:interface:respawn_map:content:map_content:actual_map": MapConfig,
	"ui:interface:squad_spawn_map:content:map_content:actual_map": MapConfig,
	"ui:interface:mp3_player": ContainerWindowsConfig,
	"ui:interface:compass": CompassConfig,
}


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	@property
	def pos(self) -> int:
		return self._pos

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if size < 0 or end > len(self._data):
			raise IcfgParseError(
				f"unexpected end of data at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def raw(self, size: int) -> bytes:
		return self._take(size)

	def peek(self, size: int) -> bytes:
		return self._data[self._pos:self._pos + size]

	def u8(self) -> int:
		return self._take(1)[0]

	def u32(self) -> int:
		return struct.unpack("<I", self._take(4))[0]

	def s32(self) -> int:
		return struct.unpack("<i", self._take(4))[0]

	def s64(self) -> int:
		return struct.unpack("<q", self._take(8))[0]

	def f32(self) -> float:
		return struct.unpack("<f", self._take(4))[0]

	def boolean(self) -> bool:
		return self.u8() != 0

	def string(self) -> str:
		length = self.u32()
		return self._take(length).decode("utf-8", "surrogateescape")

	def ucstring(self) -> str:
		length = self.u32()
		return self._take(length * 2).decode("utf-16-le", "surrogatepass")

	def vector2(self) -> Vector2:
		return Vector2(self.f32(), self.f32())

	def version(self) -> int:
		"""Mirrors IStream::serialVersion: one byte, or 0xFF + a uint32."""
		b = self.u8()
		if b == 0xFF:
			return self.u32()
		return b

	def check(self, nelid: str) -> None:
		offset = self._pos
		got = self._take(4)
		if got != _nelid(nelid):
			raise IcfgParseError(f"bad check at offset {offset}: expected {nelid!r}, got {got!r}")

	def cont_len(self) -> int:
		"""Length prefix used by serialCont()."""
		length = self.s32()
		if length < 0:
			raise IcfgParseError(f"negative container length {length} at offset {self._pos - 4}")
		return length

	def eof(self) -> bool:
		return self._pos >= len(self._data)

	@property
	def remaining(self) -> int:
		return len(self._data) - self._pos


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding."""

	def __init__(self):
		self._chunks: List[bytes] = []

	def raw(self, data: bytes) -> None:
		self._chunks.append(data)

	def u8(self, value: int) -> None:
		self._chunks.append(struct.pack("<B", value))

	def u32(self, value: int) -> None:
		self._chunks.append(struct.pack("<I", value))

	def s32(self, value: int) -> None:
		self._chunks.append(struct.pack("<i", value))

	def s64(self, value: int) -> None:
		self._chunks.append(struct.pack("<q", value))

	def f32(self, value: float) -> None:
		self._chunks.append(struct.pack("<f", value))

	def boolean(self, value: bool) -> None:
		self.u8(1 if value else 0)

	def string(self, value: str) -> None:
		data = value.encode("utf-8", "surrogateescape")
		self.u32(len(data))
		self._chunks.append(data)

	def ucstring(self, value: str) -> None:
		data = value.encode("utf-16-le", "surrogatepass")
		self.u32(len(data) // 2)
		self._chunks.append(data)

	def vector2(self, v: Vector2) -> None:
		self.f32(v.x)
		self.f32(v.y)

	def version(self, value: int) -> None:
		"""Mirrors IStream::serialVersion when writing."""
		if value < 0xFF:
			self.u8(value)
		else:
			self.u8(0xFF)
			self.u32(value)

	def check(self, nelid: str) -> None:
		self._chunks.append(_nelid(nelid))

	def cont_len(self, length: int) -> None:
		self.s32(length)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

def _parse_chat_summary(f: _Reader) -> FilteredChatSummary:
	ver = f.version()
	f.check("USHC")
	summary = FilteredChatSummary(
		src_guild=f.boolean(),
		src_team=f.boolean(),
		src_around_me=f.boolean(),
		src_tell=f.boolean(),
		src_system_info=f.boolean(),
		target=f.s32(),
	)
	if ver >= 1:
		summary.src_universe = f.boolean()
	if ver >= 2:
		summary.src_region = f.boolean()
	return summary


def _parse_optional_chat_summary(f: _Reader) -> Optional[FilteredChatSummary]:
	if f.boolean():
		return _parse_chat_summary(f)
	return None


def _parse_free_tellers(f: _Reader) -> List[str]:
	ver = f.version()
	if ver == 1:
		f.u32()
	count = f.u32()
	titles = []
	for _ in range(count):
		if ver == 1:
			f.string()
		titles.append(f.ucstring() if ver < 3 else f.string())
	return titles


def _parse_user_chats(f: _Reader) -> UserChats:
	chats = UserChats()
	ver = f.version()
	f.check("TAHC")
	chats.chat_group = _parse_optional_chat_summary(f)
	chats.user_chats = [_parse_optional_chat_summary(f) for _ in range(MAX_NUM_USER_CHATS)]
	f.check("TAHC")
	if ver >= 1:
		chats.tab_index = f.s32()
		chats.the_user_chat = _parse_optional_chat_summary(f)
	# The client only writes free tellers when the chat group window exists:
	# when absent, the next bytes are the "GFCI" header check.
	if ver >= 2 and f.peek(4) != _nelid("GFCI"):
		chats.free_tellers = _parse_free_tellers(f)
	return chats


def _parse_container(f: _Reader) -> ContainerImage:
	ver = f.version()
	cont = ContainerImage(id="")
	if ver >= 10:
		cont.min_w = f.s32()
		cont.max_w = f.s32()
	if ver >= 8:
		cont.id = UI_ID_PREFIX + f.string()
		cont.container_mode = f.u8()
		if cont.container_mode == 0:
			cont.popuped = f.boolean()
			cont.opened = f.boolean()
			cont.x = f.s32()
			cont.y = f.s32()
			cont.w = f.s32()
			cont.h = f.s32()
			cont.active = f.boolean()
			cont.scroll_pos = f.s32()
			cont.bg_alpha = f.u8()
			cont.content_alpha = f.u8()
			cont.rollover_alpha_content = f.u8()
			cont.popup_x = f.s32()
			cont.popup_y = f.s32()
			cont.popup_w = f.s32()
			cont.popup_h = f.s32()
			cont.use_global_alpha = f.boolean()
			cont.locked = f.boolean()
			cont.movable = f.boolean()
			cont.active_savable = f.boolean()
			cont.popup_max_h = f.s32()
			cont.rollover_alpha_container = f.u8()
			if ver >= 9:
				cont.backuped_position_valid = f.boolean()
				if cont.backuped_position_valid:
					cont.backup_x = f.s32()
					cont.backup_y = f.s32()
				cont.touch_flag = f.boolean()
		elif cont.container_mode == 1:
			cont.opened = f.boolean()
			cont.active = f.boolean()
			cont.active_savable = f.boolean()
		return cont

	cont.id = f.string()
	cont.popuped = f.boolean()
	cont.opened = f.boolean()
	cont.x = f.s32()
	cont.y = f.s32()
	cont.w = f.s32()
	cont.h = f.s32()
	cont.active = f.boolean()
	cont.scroll_pos = f.s32()
	if ver >= 1:
		cont.bg_alpha = f.u8()
		cont.content_alpha = f.u8()
		cont.rollover_alpha_content = f.u8()
		cont.popup_x = f.s32()
		cont.popup_y = f.s32()
		cont.popup_w = f.s32()
		cont.popup_h = f.s32()
	if ver >= 2:
		cont.use_global_alpha = f.boolean()
	if ver >= 3:
		cont.locked = f.boolean()
	if ver >= 4:
		cont.movable = f.boolean()
	if ver >= 5:
		cont.active_savable = f.boolean()
	if ver >= 6:
		cont.popup_max_h = f.s32()
	if ver >= 7:
		cont.rollover_alpha_container = f.u8()
	else:
		cont.rollover_alpha_container = cont.rollover_alpha_content
	return cont


def _decode_map(f: _Reader) -> MapConfig:
	ver = f.version()
	if ver > 3:
		raise IcfgParseError(f"unknown map config version {ver}")
	user_scale = f.f32()
	if ver < 2:
		cfg = MapConfig(user_scale=user_scale, world_offset=Vector2(0.0, 0.0), old_offset=f.vector2())
	else:
		cfg = MapConfig(user_scale=user_scale, world_offset=f.vector2())
	if ver < 3:
		cfg.h = f.s32()
	return cfg


def _decode_container_windows(f: _Reader) -> ContainerWindowsConfig:
	ver = f.version()
	if ver != 0:
		raise IcfgParseError(f"unknown container_windows config version {ver}")
	value = f.u8()
	if value > 1:
		raise IcfgParseError(f"invalid bool {value}")
	return ContainerWindowsConfig(show_desktops=value == 1)


def _decode_edit_box(f: _Reader) -> EditBoxConfig:
	ver = f.version()
	if ver > 1:
		raise IcfgParseError(f"unknown edit_box config version {ver}")
	text = f.ucstring() if ver < 1 else f.string()
	return EditBoxConfig(input_string=text, cursor_pos=f.s32(), prev_num_line=f.s32())


def _decode_compass(f: _Reader) -> CompassConfig:
	ver = f.version()
	if ver != 0:
		raise IcfgParseError(f"unknown compas config version {ver}")
	f.check("CTAR")
	tver = f.version()
	if tver > 1:
		raise IcfgParseError(f"unknown compass target version {tver}")
	pos = f.vector2()
	name = f.ucstring() if tver < 1 else f.string()
	language = f.string()
	target_type = f.s32()
	cfg = CompassConfig(pos=pos, name=name, language=language, target_type=target_type)
	if target_type == COMPASS_TYPE_POS_TRACKER:
		cfg.pos_tracker_raw = f.raw(f.remaining)
	else:
		f.check("_END")
	return cfg


_WIDGET_DECODERS = {
	MapConfig: _decode_map,
	ContainerWindowsConfig: _decode_container_windows,
	EditBoxConfig: _decode_edit_box,
	CompassConfig: _decode_compass,
}


def _try_decode(decoder, data: bytes) -> Optional[WidgetConfigData]:
	f = _Reader(data)
	try:
		result = decoder(f)
	except (IcfgParseError, UnicodeDecodeError):
		return None
	if not f.eof():
		return None
	return result


def _decode_widget(widget_id: str, data: bytes) -> WidgetConfigData:
	known = KNOWN_WIDGET_TYPES.get(widget_id)
	if known is not None:
		result = _try_decode(_WIDGET_DECODERS[known], data)
		return result if result is not None else RawWidgetConfig(data)

	matches = [r for r in (_try_decode(d, data) for d in _WIDGET_DECODERS.values()) if r is not None]
	if len(matches) == 1:
		return matches[0]
	return RawWidgetConfig(data)


def _parse_desktop(data: bytes, has_version: bool) -> DesktopImage:
	f = _Reader(data)
	ver = f.version() if has_version else 0
	desktop = DesktopImage()
	desktop.containers = [_parse_container(f) for _ in range(f.cont_len())]
	if f.eof():
		return desktop

	extra = DesktopExtra()
	if ver >= 1:
		extra.top_window = f.string()
	for _ in range(f.u32()):
		widget_id = f.string()
		chunk_size = f.u32()
		extra.widgets.append(WidgetConfig(widget_id, _decode_widget(widget_id, f.raw(chunk_size))))
	if not f.eof():
		raise IcfgParseError(f"{f.remaining} unexpected bytes at the end of a desktop image")
	desktop.extra = extra
	return desktop


def _parse_db_leaf(f: _Reader) -> DbLeaf:
	ver = f.version()
	name = f.string()
	value = f.s64()
	old_value = f.s64() if ver >= 1 else value
	return DbLeaf(name, value, old_value)


def _parse_taskbar(f: _Reader) -> List[TaskBarShortcut]:
	f.version()
	shortcuts = []
	for _ in range(TASKBAR_NUM_BARS * TASKBAR_NUM_SHORTCUTS_PER_BAR):
		f.version()
		shortcuts.append(TaskBarShortcut(sheet_type=f.s32(), db_sheet=f.string(), macro_id=f.s32()))
	return shortcuts


def _parse_landmarks(f: _Reader) -> List[ContinentLandmarks]:
	f.version()
	continents = []
	for _ in range(f.u32()):
		cont = ContinentLandmarks(continent=f.string())
		for _ in range(f.cont_len()):
			cont.landmarks.append(UserLandmark(pos=f.vector2(), title=f.ucstring(), type=f.u8()))
		continents.append(cont)
	return continents


def _parse_info_windows(f: _Reader) -> List[InfoWindowPos]:
	f.version()
	windows = []
	for _ in range(f.cont_len()):
		f.version()
		windows.append(InfoWindowPos(x=f.s32(), y=f.s32()))
	return windows


def _parse_dyn_chat(f: _Reader) -> Optional[List[bool]]:
	ver = f.version()
	f.check("OMGY")
	if not f.boolean() or ver < 1:
		return None
	f.version()
	f.check("USHC")
	return [f.boolean() for _ in range(MAX_DYN_CHAN_PER_PLAYER)]


def parse_icfg(data: bytes) -> InterfaceConfig:
	"""Parse the bytes of a .icfg file."""
	f = _Reader(data)
	cfg = InterfaceConfig()
	ver = cfg.version = f.version()
	if ver > ICFG_STREAM_VERSION:
		raise IcfgParseError(f"unsupported .icfg version {ver} (newest known: {ICFG_STREAM_VERSION})")

	if ver >= 1:
		f.check("_ICU")
		cfg.user_chats = _parse_user_chats(f)

	f.check("GFCI")
	num_modes = f.u32()
	cfg.current_mode = f.u8()
	if ver >= 10:
		cfg.last_screen_w = f.s32()
		cfg.last_screen_h = f.s32()

	for _ in range(num_modes):
		length = f.u32()
		cfg.desktops.append(_parse_desktop(f.raw(length), ver > 2) if length > 0 else None)

	if ver >= 9:
		cfg.ui_db_save_version = f.u32()

	if not f.eof():
		cfg.database = [_parse_db_leaf(f) for _ in range(f.u32())]

	if ver >= 2:
		cfg.taskbar = _parse_taskbar(f)
	cfg.landmarks = _parse_landmarks(f)
	if ver >= 5:
		cfg.info_windows = _parse_info_windows(f)
	if ver >= 7:
		cfg.macro_memory = [
			MacroMemorySlot(memory_line=f.u8(), memory_slot=f.u8(), macro_id=f.u32())
			for _ in range(f.cont_len())
		]
	if ver >= 8:
		cfg.ignored_context_help = [f.string() for _ in range(f.cont_len())]
	if ver >= 11:
		cfg.dyn_chat = _parse_dyn_chat(f)

	cfg.trailing = f.raw(f.remaining)
	return cfg


def load_icfg(path: Union[str, Path, BinaryIO]) -> InterfaceConfig:
	"""Read a .icfg file from a path or an open binary file object."""
	if isinstance(path, (str, Path)):
		return parse_icfg(Path(path).read_bytes())
	return parse_icfg(path.read())


# ---------------------------------------------------------------------------
# Writing
# ---------------------------------------------------------------------------

def _write_chat_summary(f: _Writer, summary: FilteredChatSummary) -> None:
	f.version(2)
	f.check("USHC")
	f.boolean(summary.src_guild)
	f.boolean(summary.src_team)
	f.boolean(summary.src_around_me)
	f.boolean(summary.src_tell)
	f.boolean(summary.src_system_info)
	f.s32(summary.target)
	f.boolean(summary.src_universe)
	f.boolean(summary.src_region)


def _write_optional_chat_summary(f: _Writer, summary: Optional[FilteredChatSummary]) -> None:
	f.boolean(summary is not None)
	if summary is not None:
		_write_chat_summary(f, summary)


def _write_user_chats(f: _Writer, chats: UserChats) -> None:
	f.version(2)
	f.check("TAHC")
	_write_optional_chat_summary(f, chats.chat_group)
	for summary in chats.user_chats:
		_write_optional_chat_summary(f, summary)
	f.check("TAHC")
	f.s32(chats.tab_index)
	_write_optional_chat_summary(f, chats.the_user_chat)
	if chats.free_tellers is not None:
		f.version(3)
		f.u32(len(chats.free_tellers))
		for title in chats.free_tellers:
			f.string(title)


def _write_container(f: _Writer, cont: ContainerImage) -> None:
	f.version(10)
	f.s32(cont.min_w)
	f.s32(cont.max_w)
	short_id = cont.id[len(UI_ID_PREFIX):] if cont.id.startswith(UI_ID_PREFIX) else cont.id
	f.string(short_id)
	f.u8(cont.container_mode)
	if cont.container_mode == 0:
		f.boolean(cont.popuped)
		f.boolean(cont.opened)
		f.s32(cont.x)
		f.s32(cont.y)
		f.s32(cont.w)
		f.s32(cont.h)
		f.boolean(cont.active)
		f.s32(cont.scroll_pos)
		f.u8(cont.bg_alpha)
		f.u8(cont.content_alpha)
		f.u8(cont.rollover_alpha_content)
		f.s32(cont.popup_x)
		f.s32(cont.popup_y)
		f.s32(cont.popup_w)
		f.s32(cont.popup_h)
		f.boolean(cont.use_global_alpha)
		f.boolean(cont.locked)
		f.boolean(cont.movable)
		f.boolean(cont.active_savable)
		f.s32(cont.popup_max_h)
		f.u8(cont.rollover_alpha_container)
		f.boolean(cont.backuped_position_valid)
		if cont.backuped_position_valid:
			f.s32(cont.backup_x)
			f.s32(cont.backup_y)
		f.boolean(cont.touch_flag)
	elif cont.container_mode == 1:
		f.boolean(cont.opened)
		f.boolean(cont.active)
		f.boolean(cont.active_savable)


def _encode_widget(config: WidgetConfigData) -> bytes:
	if isinstance(config, RawWidgetConfig):
		return config.data
	f = _Writer()
	if isinstance(config, MapConfig):
		f.version(3)
		f.f32(config.user_scale)
		f.vector2(config.world_offset)
	elif isinstance(config, ContainerWindowsConfig):
		f.version(0)
		f.boolean(config.show_desktops)
	elif isinstance(config, EditBoxConfig):
		f.version(1)
		f.string(config.input_string)
		f.s32(config.cursor_pos)
		f.s32(config.prev_num_line)
	elif isinstance(config, CompassConfig):
		f.version(0)
		f.check("CTAR")
		f.version(1)
		f.vector2(config.pos)
		f.string(config.name)
		f.string(config.language)
		f.s32(config.target_type)
		if config.target_type == COMPASS_TYPE_POS_TRACKER:
			f.raw(config.pos_tracker_raw or b"")
		else:
			f.check("_END")
	else:
		raise TypeError(f"unknown widget config type {type(config).__name__}")
	return f.getvalue()


def _encode_desktop(desktop: DesktopImage) -> bytes:
	f = _Writer()
	f.version(2)
	f.cont_len(len(desktop.containers))
	for cont in desktop.containers:
		_write_container(f, cont)
	if desktop.extra is not None:
		f.string(desktop.extra.top_window)
		f.u32(len(desktop.extra.widgets))
		for widget in desktop.extra.widgets:
			data = _encode_widget(widget.config)
			f.string(widget.id)
			f.u32(len(data))
			f.raw(data)
	return f.getvalue()


def dumps(cfg: InterfaceConfig) -> bytes:
	"""Serialize an InterfaceConfig to the .icfg binary format (version 11)."""
	f = _Writer()
	f.version(ICFG_STREAM_VERSION)

	f.check("_ICU")
	_write_user_chats(f, cfg.user_chats if cfg.user_chats is not None else UserChats())

	f.check("GFCI")
	f.u32(len(cfg.desktops))
	f.u8(cfg.current_mode)
	f.s32(cfg.last_screen_w if cfg.last_screen_w is not None else 0)
	f.s32(cfg.last_screen_h if cfg.last_screen_h is not None else 0)

	for desktop in cfg.desktops:
		data = _encode_desktop(desktop) if desktop is not None else b""
		f.u32(len(data))
		f.raw(data)

	f.u32(cfg.ui_db_save_version)

	database = cfg.database or []
	f.u32(len(database))
	for leaf in database:
		f.version(1)
		f.string(leaf.name)
		f.s64(leaf.value)
		f.s64(leaf.old_value)

	f.version(0)
	taskbar = cfg.taskbar or []
	for i in range(TASKBAR_NUM_BARS * TASKBAR_NUM_SHORTCUTS_PER_BAR):
		shortcut = taskbar[i] if i < len(taskbar) else TaskBarShortcut()
		f.version(0)
		f.s32(shortcut.sheet_type)
		f.string(shortcut.db_sheet)
		f.s32(shortcut.macro_id)

	f.version(1)
	f.u32(len(cfg.landmarks))
	for cont in cfg.landmarks:
		f.string(cont.continent)
		f.cont_len(len(cont.landmarks))
		for landmark in cont.landmarks:
			f.vector2(landmark.pos)
			f.ucstring(landmark.title)
			f.u8(landmark.type)

	f.version(0)
	info_windows = cfg.info_windows or []
	f.cont_len(len(info_windows))
	for window in info_windows:
		f.version(0)
		f.s32(window.x)
		f.s32(window.y)

	macro_memory = cfg.macro_memory or []
	f.cont_len(len(macro_memory))
	for slot in macro_memory:
		f.u8(slot.memory_line)
		f.u8(slot.memory_slot)
		f.u32(slot.macro_id)

	ignored = cfg.ignored_context_help or []
	f.cont_len(len(ignored))
	for name in ignored:
		f.string(name)

	f.version(1)
	f.check("OMGY")
	f.boolean(cfg.dyn_chat is not None)
	if cfg.dyn_chat is not None:
		f.version(0)
		f.check("USHC")
		for value in cfg.dyn_chat:
			f.boolean(value)

	f.raw(cfg.trailing)
	return f.getvalue()


def save_icfg(path: Union[str, Path, BinaryIO], cfg: InterfaceConfig) -> None:
	"""Serialize an InterfaceConfig and write it to a path or open binary file object."""
	data = dumps(cfg)
	if isinstance(path, (str, Path)):
		Path(path).write_bytes(data)
	else:
		path.write(data)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def _to_json(value):
	if isinstance(value, bytes):
		return value.hex()
	if dataclasses.is_dataclass(value):
		result = {}
		if isinstance(value, tuple(_WIDGET_DECODERS) + (RawWidgetConfig,)):
			result["_type"] = type(value).__name__
		for f in dataclasses.fields(value):
			result[f.name] = _to_json(getattr(value, f.name))
		return result
	if isinstance(value, list):
		return [_to_json(v) for v in value]
	return value


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom client interface config (.icfg) files")
	sub = parser.add_subparsers(dest="command", required=True)
	p_dump = sub.add_parser("dump", help="print a .icfg file as JSON")
	p_dump.add_argument("path", type=Path)
	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()
	cfg = load_icfg(args.path)
	if args.command == "dump":
		print(json.dumps(_to_json(cfg), indent="\t", ensure_ascii=False))


if __name__ == "__main__":
	_main()
