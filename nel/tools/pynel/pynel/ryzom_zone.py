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

"""Read Ryzom/NeL .zone/.zonew/.zonel landscape files (CZone::serial).

Format reverse-engineered from nel/src/3d/zone.cpp (CZone::serial) and
nel/src/3d/patch.cpp (CPatch::serial), plus the sub-object serializers in
point_light_named_array.cpp/point_light_named.cpp/point_light.cpp (same
classes as ryzom_ig.py's instance-group point lights) -- see
nel/tools/pynel/docs/zone_format.md for the full writeup.

All three extensions (.zone/.zonew/.zonel) are the same binary format at
different landscape-build pipeline stages (raw / welded / lit). Only
CZone::serial versions >= 3 are supported, matching NeL itself (which
rejects < 3 outright).

Compressed lumels (CPatch.compressed_lumels) are read/written as an opaque
byte blob, never decoded -- decoding the lumel compression is out of scope
for a round-trip reader/writer (see docs/zone_format.md).

Read-only for now: write support (round-trip serialization) is a later step
of project-todos/pynel/zone_read_write.md.

Usage:
	from pynel import ryzom_zone as rz
	zone = rz.load_zone("160_ab.zone")
	print(zone.zone_id, len(zone.patchs), len(zone.border_vertices))
"""

import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, List, Tuple, Union

MAGIC = b"ZONE"  # on-disk bytes for NELID("ENOZ") (reversed on little-endian)

# NL_PATCH_SMOOTH_FLAG_MASK (patch.h) -- default CPatch.flags for patch version < 4,
# when the field isn't present on disk at all.
DEFAULT_PATCH_FLAGS = 0x3C


class ZoneParseError(Exception):
	pass


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
class AABBox:
	"""CAABBoxExt (aabbox.h/.cpp) -- the stored center+half_size; the bounding
	radius is recomputed at load time (updateRadius()), never stored."""
	center: Vector3
	half_size: Vector3


@dataclass
class PackedVertex:
	"""CVector3s (patch.h) -- a vertex quantized to the parent Zone's
	patch_bias/patch_scale. Use unpack_vertex() to get world-space coordinates."""
	x: int
	y: int
	z: int


@dataclass
class BorderVertex:
	"""CBorderVertex (zone.h/.cpp) -- cross-zone vertex connectivity."""
	current_vertex: int
	neighbor_zone_id: int
	neighbor_vertex: int


@dataclass
class TileElement:
	"""CTileElement (tile_element.h/.cpp), element of Patch.tiles (OrderS*OrderT of them)."""
	flags: int
	tile: List[int]  # 3 entries


@dataclass
class TileColor:
	"""CTileColor (tile_color.h) -- element of Patch.tile_colors
	((OrderS+1)*(OrderT+1) of them). Older patch versions (<=6) also stored a
	per-corner light byte triplet here, discarded on read (never kept by the
	engine itself once re-copied into the current CTileColor)."""
	color565: int


@dataclass
class TileLightInfluence:
	"""CTileLightInfluence (tile_light_influence.h/.cpp) -- element of
	Patch.tile_light_influences ((OrderS/2+1)*(OrderT/2+1) of them, patch version >= 5 only)."""
	light: Tuple[int, int]
	packed_light_factor: int


@dataclass
class BindInfo:
	"""CPatchInfo::CBindInfo (zone.h/.cpp) -- one edge's binding info, element of PatchConnect.bind_edges."""
	n_patchs: int
	zone_id: int
	next: List[int]  # 4 entries
	edge: List[int]  # 4 entries


@dataclass
class PatchConnect:
	"""CZone::CPatchConnect (zone.h/.cpp), element of Zone.patch_connects."""
	error_size: float
	base_vertices: List[int]  # 4 entries
	bind_edges: List[BindInfo]  # 4 entries


@dataclass
class Patch:
	"""CPatch (patch.h/.cpp), the stored per-patch geometry+texture data --
	NOT the same as CPatchInfo (an in-memory build-only struct, never serialized)."""
	vertices: List[PackedVertex]  # 4 entries: the patch's 4 corners
	tangents: List[PackedVertex]  # 8 entries: 2 Bezier tangents per corner
	interiors: List[PackedVertex]  # 4 entries: Bezier interior control points
	tiles: List[TileElement]  # OrderS*OrderT entries
	tile_colors: List[TileColor]  # (OrderS+1)*(OrderT+1) entries, patch version >= 1
	order_s: int  # patch version >= 2 only, else 0 (not on disk)
	order_t: int  # patch version >= 2 only, else 0 (not on disk)
	compressed_lumels: bytes  # patch version >= 2 only, else b"" -- opaque, never decoded
	noise_rotation: int  # patch version >= 3 only, else 0
	corner_smooth_flag: int  # patch version >= 3 only, else 0
	flags: int  # patch version >= 4 only, else DEFAULT_PATCH_FLAGS
	tile_light_influences: List[TileLightInfluence]  # patch version >= 5 only, else []


@dataclass
class PointLightNamed:
	"""CPointLightNamed (point_light_named.cpp), extends CPointLight
	(point_light.cpp) -- same on-disk layout as ryzom_ig.py's PointLightNamed."""
	position: Vector3
	ambient: Rgba
	diffuse: Rgba
	specular: Rgba
	attenuation_begin: float
	attenuation_end: float
	add_ambient_with_sun: bool
	light_type: int  # CPointLight::TType (PointLight=0, SpotLight=1, AmbientLight=2)
	spot_direction: Vector3
	spot_angle_begin: float
	spot_angle_end: float
	animated_light: str
	default_ambient: Rgba
	default_diffuse: Rgba
	default_specular: Rgba
	light_group: int


@dataclass
class PointLightGroup:
	"""CPointLightNamedArray::CPointLightGroup (point_light_named_array.cpp)."""
	animation_light: str
	light_group: int
	start_id: int
	end_id: int


@dataclass
class Zone:
	"""CZone (zone.h/.cpp), the top-level .zone/.zonew/.zonel payload."""
	zone_id: int
	zone_bb: AABBox
	patch_bias: Vector3
	patch_scale: float
	num_vertices: int
	border_vertices: List[BorderVertex] = field(default_factory=list)
	patchs: List[Patch] = field(default_factory=list)
	patch_connects: List[PatchConnect] = field(default_factory=list)
	point_lights: List[PointLightNamed] = field(default_factory=list)  # zone version >= 4 only
	point_light_groups: List[PointLightGroup] = field(default_factory=list)  # zone version >= 4 only


def unpack_vertex(v: PackedVertex, bias: Vector3, scale: float) -> Vector3:
	"""Mirrors CVector3s::unpack() -- decodes a quantized patch vertex/tangent/
	interior point into world-space coordinates using the parent Zone's
	patch_bias/patch_scale."""
	return Vector3(v.x * scale + bias.x, v.y * scale + bias.y, v.z * scale + bias.z)


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise ZoneParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def u8(self) -> int:
		return self._take(1)[0]

	def s16(self) -> int:
		return struct.unpack("<h", self._take(2))[0]

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

	def vector3(self) -> Vector3:
		return Vector3(self.f32(), self.f32(), self.f32())

	def rgba(self) -> Rgba:
		return Rgba(self.u8(), self.u8(), self.u8(), self.u8())

	def version(self) -> int:
		"""Mirrors IStream::serialVersion: one byte, or 0xFF + a uint32."""
		b = self.u8()
		if b == 0xFF:
			return self.u32()
		return b

	def check_magic(self, expected: bytes) -> None:
		got = self._take(len(expected))
		if got != expected:
			raise ZoneParseError(f"bad magic: expected {expected!r}, got {got!r}")

	def cont_len(self) -> int:
		"""Length prefix used by serialCont() for generic containers."""
		return self.s32()

	def bytes_blob(self) -> bytes:
		"""Bulk vector<uint8> specialization: sint32 count + raw bytes, no
		per-element serialization (used for CPatch.compressed_lumels)."""
		length = self.cont_len()
		return self._take(length)

	def eof(self) -> bool:
		return self._pos >= len(self._data)

	@property
	def remaining(self) -> int:
		return len(self._data) - self._pos


def _parse_aabbox(f: _Reader) -> AABBox:
	f.version()  # CAABBox version, always 0
	center = f.vector3()
	half_size = f.vector3()
	return AABBox(center, half_size)


def _parse_packed_vertex(f: _Reader) -> PackedVertex:
	return PackedVertex(f.s16(), f.s16(), f.s16())


def _parse_border_vertex(f: _Reader) -> BorderVertex:
	f.version()  # always 0
	current_vertex = f.u16()
	neighbor_zone_id = f.u16()
	neighbor_vertex = f.u16()
	return BorderVertex(current_vertex, neighbor_zone_id, neighbor_vertex)


def _parse_tile_element(f: _Reader) -> TileElement:
	flags = f.u16()
	tile = [f.u16() for _ in range(3)]
	return TileElement(flags, tile)


def _parse_tile_color(f: _Reader) -> TileColor:
	return TileColor(f.u16())


def _parse_tile_color_old_v6(f: _Reader) -> TileColor:
	color565 = f.u16()
	f.u8()  # lightX -- discarded, matches the engine's own copy-into-CTileColor behavior
	f.u8()  # lightY
	f.u8()  # lightZ
	return TileColor(color565)


def _parse_tile_light_influence(f: _Reader) -> TileLightInfluence:
	light0 = f.u8()
	light1 = f.u8()
	packed_light_factor = f.u8()
	return TileLightInfluence((light0, light1), packed_light_factor)


def _parse_bind_info(f: _Reader) -> BindInfo:
	f.version()  # always 0
	n_patchs = f.u8()
	zone_id = f.u16()
	next_ = [f.u16() for _ in range(4)]
	edge = [f.u8() for _ in range(4)]
	return BindInfo(n_patchs, zone_id, next_, edge)


def _parse_patch_connect(f: _Reader) -> PatchConnect:
	version = f.version()

	if version < 1:
		f.u8()  # OldOrderS, unused once ErrorSize/BaseVertices/BindEdges are read
		f.u8()  # OldOrderT
		error_size = f.f32()
	else:
		error_size = f.f32()

	base_vertices = [f.u16() for _ in range(4)]
	bind_edges = [_parse_bind_info(f) for _ in range(4)]

	return PatchConnect(error_size=error_size, base_vertices=base_vertices, bind_edges=bind_edges)


def _parse_patch(f: _Reader) -> Patch:
	version = f.version()
	if version < 2:
		raise ZoneParseError(f"unsupported CPatch version {version} (< 2 not supported)")

	vertices = [_parse_packed_vertex(f) for _ in range(4)]
	tangents = [_parse_packed_vertex(f) for _ in range(8)]
	interiors = [_parse_packed_vertex(f) for _ in range(4)]

	n_tiles = f.cont_len()
	tiles = [_parse_tile_element(f) for _ in range(n_tiles)]

	tile_colors: List[TileColor] = []
	if version >= 1:
		n_tile_colors = f.cont_len()
		if version <= 6:
			tile_colors = [_parse_tile_color_old_v6(f) for _ in range(n_tile_colors)]
		else:
			tile_colors = [_parse_tile_color(f) for _ in range(n_tile_colors)]

	order_s = 0
	order_t = 0
	compressed_lumels = b""
	if version >= 2:
		order_s = f.u8()
		order_t = f.u8()
		compressed_lumels = f.bytes_blob()

	noise_rotation = 0
	corner_smooth_flag = 0
	if version >= 3:
		noise_rotation = f.u8()
		corner_smooth_flag = f.u8()

	flags = DEFAULT_PATCH_FLAGS
	if version >= 4:
		flags = f.u8()

	tile_light_influences: List[TileLightInfluence] = []
	if version >= 5:
		n_influences = f.cont_len()
		tile_light_influences = [_parse_tile_light_influence(f) for _ in range(n_influences)]

	return Patch(
		vertices=vertices, tangents=tangents, interiors=interiors, tiles=tiles,
		tile_colors=tile_colors, order_s=order_s, order_t=order_t,
		compressed_lumels=compressed_lumels, noise_rotation=noise_rotation,
		corner_smooth_flag=corner_smooth_flag, flags=flags,
		tile_light_influences=tile_light_influences,
	)


def _parse_point_light(f: _Reader) -> dict:
	"""CPointLight::serial (base class of CPointLightNamed) -- same layout as
	ryzom_ig.py's identically-named helper."""
	version = f.version()

	add_ambient_with_sun = f.boolean() if version >= 2 else False

	if version >= 1:
		light_type = f.s32()  # serialEnum -> sint32
		spot_direction = f.vector3()
		spot_angle_begin = f.f32()
		spot_angle_end = f.f32()
	else:
		light_type = 0
		spot_direction = Vector3(0.0, 1.0, 0.0)
		spot_angle_begin = 0.7853981633974483  # pi/4
		spot_angle_end = 1.5707963267948966  # pi/2

	position = f.vector3()
	ambient = f.rgba()
	diffuse = f.rgba()
	specular = f.rgba()
	attenuation_begin = f.f32()
	attenuation_end = f.f32()

	return dict(
		add_ambient_with_sun=add_ambient_with_sun, light_type=light_type,
		spot_direction=spot_direction, spot_angle_begin=spot_angle_begin,
		spot_angle_end=spot_angle_end, position=position, ambient=ambient,
		diffuse=diffuse, specular=specular, attenuation_begin=attenuation_begin,
		attenuation_end=attenuation_end,
	)


def _parse_point_light_named(f: _Reader) -> PointLightNamed:
	version = f.version()
	base = _parse_point_light(f)

	animated_light = f.string()
	default_ambient = f.rgba()
	default_diffuse = f.rgba()
	default_specular = f.rgba()
	light_group = f.u32() if version >= 1 else 0

	return PointLightNamed(
		**base, animated_light=animated_light, default_ambient=default_ambient,
		default_diffuse=default_diffuse, default_specular=default_specular,
		light_group=light_group,
	)


def _parse_point_light_group(f: _Reader) -> PointLightGroup:
	f.version()  # always 0
	animation_light = f.string()
	light_group = f.u32()
	start_id = f.u32()
	end_id = f.u32()
	return PointLightGroup(animation_light, light_group, start_id, end_id)


def _parse_point_light_array(f: _Reader) -> Tuple[List[PointLightNamed], List[PointLightGroup]]:
	version = f.version()

	n_lights = f.cont_len()
	point_lights = [_parse_point_light_named(f) for _ in range(n_lights)]

	groups: List[PointLightGroup] = []
	n_groups = f.cont_len()
	if version >= 1:
		groups = [_parse_point_light_group(f) for _ in range(n_groups)]
	else:
		# deprecated map<string, CPointLightGroupV0> -> (StartId, EndId) only
		for _ in range(n_groups):
			name = f.string()
			start_id = f.u32()
			end_id = f.u32()
			groups.append(PointLightGroup(name, 0, start_id, end_id))

	return point_lights, groups


def parse_zone(data: bytes) -> Zone:
	f = _Reader(data)

	version = f.version()
	if version < 3:
		raise ZoneParseError(f"unsupported .zone version {version} (< 3 not supported by NeL itself)")

	f.check_magic(MAGIC)

	zone_id = f.u16()
	zone_bb = _parse_aabbox(f)
	patch_bias = f.vector3()
	patch_scale = f.f32()
	num_vertices = f.s32()

	n_border = f.cont_len()
	border_vertices = [_parse_border_vertex(f) for _ in range(n_border)]

	n_patchs = f.cont_len()
	patchs = [_parse_patch(f) for _ in range(n_patchs)]

	n_connects = f.cont_len()
	patch_connects = [_parse_patch_connect(f) for _ in range(n_connects)]

	point_lights: List[PointLightNamed] = []
	point_light_groups: List[PointLightGroup] = []
	if version >= 4:
		point_lights, point_light_groups = _parse_point_light_array(f)

	if not f.eof():
		raise ZoneParseError(f"{f.remaining} trailing bytes after parsing .zone content")

	return Zone(
		zone_id=zone_id, zone_bb=zone_bb, patch_bias=patch_bias, patch_scale=patch_scale,
		num_vertices=num_vertices, border_vertices=border_vertices, patchs=patchs,
		patch_connects=patch_connects, point_lights=point_lights, point_light_groups=point_light_groups,
	)


def load_zone(path: Union[str, Path, BinaryIO]) -> Zone:
	if hasattr(path, "read"):
		data = path.read()
	else:
		data = Path(path).read_bytes()
	return parse_zone(data)


# Current on-disk versions this writer always emits, matching the engine's own
# behaviour of silently upgrading a re-saved file to the latest format for every
# sub-object -- a Zone read from an older file is always safe to write back.
_ZONE_VERSION = 5
_PATCH_VERSION = 7
_PATCH_CONNECT_VERSION = 1
_POINT_LIGHT_ARRAY_VERSION = 1
_POINT_LIGHT_NAMED_VERSION = 1
_POINT_LIGHT_VERSION = 2


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding."""

	def __init__(self):
		self._chunks: List[bytes] = []

	def u8(self, value: int) -> None:
		self._chunks.append(struct.pack("<B", value))

	def s16(self, value: int) -> None:
		self._chunks.append(struct.pack("<h", value))

	def u16(self, value: int) -> None:
		self._chunks.append(struct.pack("<H", value))

	def u32(self, value: int) -> None:
		self._chunks.append(struct.pack("<I", value))

	def s32(self, value: int) -> None:
		self._chunks.append(struct.pack("<i", value))

	def f32(self, value: float) -> None:
		self._chunks.append(struct.pack("<f", value))

	def boolean(self, value: bool) -> None:
		self.u8(1 if value else 0)

	def string(self, value: str) -> None:
		raw = value.encode("latin-1")
		self.u32(len(raw))
		self._chunks.append(raw)

	def vector3(self, v: Vector3) -> None:
		self.f32(v.x)
		self.f32(v.y)
		self.f32(v.z)

	def rgba(self, c: Rgba) -> None:
		self.u8(c.r)
		self.u8(c.g)
		self.u8(c.b)
		self.u8(c.a)

	def version(self, value: int) -> None:
		"""Mirrors IStream::serialVersion when writing."""
		if value < 0xFF:
			self.u8(value)
		else:
			self.u8(0xFF)
			self.u32(value)

	def write_magic(self, magic: bytes) -> None:
		self._chunks.append(magic)

	def cont_len(self, length: int) -> None:
		self.s32(length)

	def bytes_blob(self, data: bytes) -> None:
		self.cont_len(len(data))
		self._chunks.append(data)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)


def _write_aabbox(f: _Writer, box: AABBox) -> None:
	f.version(0)
	f.vector3(box.center)
	f.vector3(box.half_size)


def _write_packed_vertex(f: _Writer, v: PackedVertex) -> None:
	f.s16(v.x)
	f.s16(v.y)
	f.s16(v.z)


def _write_border_vertex(f: _Writer, v: BorderVertex) -> None:
	f.version(0)
	f.u16(v.current_vertex)
	f.u16(v.neighbor_zone_id)
	f.u16(v.neighbor_vertex)


def _write_tile_element(f: _Writer, t: TileElement) -> None:
	f.u16(t.flags)
	for value in t.tile:
		f.u16(value)


def _write_tile_color(f: _Writer, c: TileColor) -> None:
	f.u16(c.color565)


def _write_tile_light_influence(f: _Writer, t: TileLightInfluence) -> None:
	f.u8(t.light[0])
	f.u8(t.light[1])
	f.u8(t.packed_light_factor)


def _write_bind_info(f: _Writer, b: BindInfo) -> None:
	f.version(0)
	f.u8(b.n_patchs)
	f.u16(b.zone_id)
	for value in b.next:
		f.u16(value)
	for value in b.edge:
		f.u8(value)


def _write_patch_connect(f: _Writer, pc: PatchConnect) -> None:
	f.version(_PATCH_CONNECT_VERSION)
	f.f32(pc.error_size)
	for value in pc.base_vertices:
		f.u16(value)
	for bind_info in pc.bind_edges:
		_write_bind_info(f, bind_info)


def _write_patch(f: _Writer, p: Patch) -> None:
	f.version(_PATCH_VERSION)

	for v in p.vertices:
		_write_packed_vertex(f, v)
	for v in p.tangents:
		_write_packed_vertex(f, v)
	for v in p.interiors:
		_write_packed_vertex(f, v)

	f.cont_len(len(p.tiles))
	for tile in p.tiles:
		_write_tile_element(f, tile)

	f.cont_len(len(p.tile_colors))
	for color in p.tile_colors:
		_write_tile_color(f, color)

	f.u8(p.order_s)
	f.u8(p.order_t)
	f.bytes_blob(p.compressed_lumels)

	f.u8(p.noise_rotation)
	f.u8(p.corner_smooth_flag)

	f.u8(p.flags)

	f.cont_len(len(p.tile_light_influences))
	for influence in p.tile_light_influences:
		_write_tile_light_influence(f, influence)


def _write_point_light(f: _Writer, light: PointLightNamed) -> None:
	"""CPointLight::serial (base class of CPointLightNamed) -- carries its own
	version byte, written before its fields."""
	f.version(_POINT_LIGHT_VERSION)
	f.boolean(light.add_ambient_with_sun)
	f.s32(light.light_type)
	f.vector3(light.spot_direction)
	f.f32(light.spot_angle_begin)
	f.f32(light.spot_angle_end)
	f.vector3(light.position)
	f.rgba(light.ambient)
	f.rgba(light.diffuse)
	f.rgba(light.specular)
	f.f32(light.attenuation_begin)
	f.f32(light.attenuation_end)


def _write_point_light_named(f: _Writer, light: PointLightNamed) -> None:
	f.version(_POINT_LIGHT_NAMED_VERSION)
	_write_point_light(f, light)

	f.string(light.animated_light)
	f.rgba(light.default_ambient)
	f.rgba(light.default_diffuse)
	f.rgba(light.default_specular)
	f.u32(light.light_group)


def _write_point_light_group(f: _Writer, group: PointLightGroup) -> None:
	f.version(0)
	f.string(group.animation_light)
	f.u32(group.light_group)
	f.u32(group.start_id)
	f.u32(group.end_id)


def _write_point_light_array(
	f: _Writer, point_lights: List[PointLightNamed], groups: List[PointLightGroup]
) -> None:
	f.version(_POINT_LIGHT_ARRAY_VERSION)

	f.cont_len(len(point_lights))
	for light in point_lights:
		_write_point_light_named(f, light)

	f.cont_len(len(groups))
	for group in groups:
		_write_point_light_group(f, group)


def build_zone(zone: Zone) -> bytes:
	"""Serializes a Zone back to bytes, always at the latest on-disk version for
	every sub-object (matching the engine's own re-save-upgrades behaviour) --
	so a Zone read from an older-version file is always safe to write back.
	No geometric transformation is performed; this is a pure round-trip writer."""
	f = _Writer()

	f.version(_ZONE_VERSION)
	f.write_magic(MAGIC)

	f.u16(zone.zone_id)
	_write_aabbox(f, zone.zone_bb)
	f.vector3(zone.patch_bias)
	f.f32(zone.patch_scale)
	f.s32(zone.num_vertices)

	f.cont_len(len(zone.border_vertices))
	for border_vertex in zone.border_vertices:
		_write_border_vertex(f, border_vertex)

	f.cont_len(len(zone.patchs))
	for patch in zone.patchs:
		_write_patch(f, patch)

	f.cont_len(len(zone.patch_connects))
	for patch_connect in zone.patch_connects:
		_write_patch_connect(f, patch_connect)

	# CZone version 5 >= 4, so the point light section is always present on write.
	_write_point_light_array(f, zone.point_lights, zone.point_light_groups)

	return f.getvalue()


def save_zone(path: Union[str, Path, BinaryIO], zone: Zone) -> None:
	data = build_zone(zone)
	if hasattr(path, "write"):
		path.write(data)
	else:
		Path(path).write_bytes(data)
