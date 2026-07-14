#!/usr/bin/env python3
"""Reader for Ryzom/NeL .ig (Instance Group) files.

Format reverse-engineered from nel/include/nel/3d/scene_group.h and
nel/src/3d/scene_group.cpp (CInstanceGroup::serial / CInstance::serial),
plus the sub-object serializers in nel/src/3d/cluster.cpp, portal.cpp,
point_light_named_array.cpp, point_light_named.cpp, point_light.cpp,
ig_surface_light.cpp and surface_light_grid.cpp.

Usage:
	from ryzom_ig import load_ig, save_ig
	ig = load_ig("street.ig")
	for inst in ig.instances:
		print(inst.instance_name, inst.shape_name, inst.pos)

	ig.instances[0].pos = Vector3(100.0, 0.0, 0.0)
	ig.global_pos = Vector3(0.0, 0.0, 0.0)
	save_ig("street_edited.ig", ig)

Writing always emits the latest format version for every sub-object
(same behaviour as the NeL engine: re-saving a file silently upgrades it),
so a value read from an older file is always safe to write back.
"""

import argparse
import math
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import BinaryIO, Dict, List, Tuple, Union

MAGIC = b"GRPT"  # on-disk bytes for NELID("TPRG") on a little-endian machine


class IgParseError(Exception):
	pass


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
class Quaternion:
	x: float
	y: float
	z: float
	w: float


@dataclass
class Plane:
	a: float
	b: float
	c: float
	d: float


@dataclass
class Rgba:
	r: int
	g: int
	b: int
	a: int


@dataclass
class AABBox:
	center: Vector3
	half_size: Vector3


@dataclass
class Cluster:
	name: str
	local_volume: List[Plane]
	local_bbox: AABBox
	father_visible: bool
	visible_from_father: bool
	sound_group: str
	env_fx_name: str
	audible_from_father: bool
	father_audible: bool
	portal_indices: List[int] = field(default_factory=list)


@dataclass
class Portal:
	poly: List[Vector3]
	name: str
	occlusion_model: str
	open_occlusion_model: str


@dataclass
class PointLightNamed:
	# Only the position is required; everything else defaults to a plain
	# white omnidirectional light, so a new one can be built with e.g.
	# PointLightNamed(position=Vector3(0, 0, 2)).
	position: Vector3
	ambient: Rgba = field(default_factory=lambda: Rgba(0, 0, 0, 255))
	diffuse: Rgba = field(default_factory=lambda: Rgba(255, 255, 255, 255))
	specular: Rgba = field(default_factory=lambda: Rgba(255, 255, 255, 255))
	attenuation_begin: float = 1.0
	attenuation_end: float = 10.0
	# CPointLight base fields
	add_ambient_with_sun: bool = False
	light_type: int = 0  # CPointLight::PointLight
	spot_direction: Vector3 = field(default_factory=lambda: Vector3(0.0, 1.0, 0.0))
	spot_angle_begin: float = math.pi / 4
	spot_angle_end: float = math.pi / 2
	# CPointLightNamed fields
	animated_light: str = ""
	default_ambient: Rgba = field(default_factory=lambda: Rgba(0, 0, 0, 255))
	default_diffuse: Rgba = field(default_factory=lambda: Rgba(255, 255, 255, 255))
	default_specular: Rgba = field(default_factory=lambda: Rgba(255, 255, 255, 255))
	light_group: int = 0


@dataclass
class PointLightGroup:
	animation_light: str
	light_group: int
	start_id: int
	end_id: int


@dataclass
class CellCorner:
	local_ambient_id: int
	sun_contribution: int
	light: Tuple[int, int]


@dataclass
class SurfaceLightGrid:
	origin: Vector2
	width: int
	height: int
	cells: List[CellCorner]


@dataclass
class RetrieverLightGrid:
	grids: List[SurfaceLightGrid]


@dataclass
class IgSurfaceLight:
	cell_size: float
	oo_cell_size: float
	retriever_grid_map: Dict[int, RetrieverLightGrid]


@dataclass
class Instance:
	# Only shape_name and pos are required; everything else defaults to an
	# unrotated, unscaled, parentless instance, so a new one can be built
	# with e.g. Instance(shape_name="fyros_house.shape", pos=Vector3(0, 0, 0)).
	shape_name: str
	pos: Vector3
	rot: Quaternion = field(default_factory=lambda: Quaternion(0.0, 0.0, 0.0, 1.0))
	scale: Vector3 = field(default_factory=lambda: Vector3(1.0, 1.0, 1.0))
	parent: int = -1
	clusters: List[int] = field(default_factory=list)
	instance_name: str = ""
	dont_add_to_scene: bool = False
	avoid_static_light_pre_compute: bool = False
	dont_cast_shadow: bool = False
	static_light_enabled: bool = False
	sun_contribution: int = 0
	light: Tuple[int, int] = (0xFF, 0xFF)
	local_ambient_id: int = 0xFF
	dont_cast_shadow_for_interior: bool = False
	dont_cast_shadow_for_exterior: bool = False
	visible: bool = True


@dataclass
class InstanceGroup:
	real_time_sun_contribution: bool
	surface_light: Union[IgSurfaceLight, None]
	point_lights: List[PointLightNamed]
	point_light_groups: List[PointLightGroup]
	global_pos: Union[Vector3, None]
	clusters: List[Cluster]
	portals: List[Portal]
	instances: List[Instance]


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise IgParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def u8(self) -> int:
		return self._take(1)[0]

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

	def vector2(self) -> Vector2:
		return Vector2(self.f32(), self.f32())

	def quaternion(self) -> Quaternion:
		return Quaternion(self.f32(), self.f32(), self.f32(), self.f32())

	def plane(self) -> Plane:
		return Plane(self.f32(), self.f32(), self.f32(), self.f32())

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
			raise IgParseError(f"bad magic: expected {expected!r}, got {got!r}")

	def cont_len(self) -> int:
		"""Length prefix used by serialCont() for generic containers."""
		return self.s32()

	def eof(self) -> bool:
		return self._pos >= len(self._data)

	@property
	def remaining(self) -> int:
		return len(self._data) - self._pos


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding."""

	def __init__(self):
		self._chunks: List[bytes] = []

	def u8(self, value: int) -> None:
		self._chunks.append(struct.pack("<B", value))

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

	def vector2(self, v: Vector2) -> None:
		self.f32(v.x)
		self.f32(v.y)

	def quaternion(self, q: Quaternion) -> None:
		self.f32(q.x)
		self.f32(q.y)
		self.f32(q.z)
		self.f32(q.w)

	def plane(self, p: Plane) -> None:
		self.f32(p.a)
		self.f32(p.b)
		self.f32(p.c)
		self.f32(p.d)

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

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)


def _write_aabbox(f: _Writer, box: AABBox) -> None:
	f.version(0)
	f.vector3(box.center)
	f.vector3(box.half_size)


def _write_cluster(f: _Writer, cluster: Cluster) -> None:
	f.version(3)
	f.string(cluster.name)
	f.cont_len(len(cluster.local_volume))
	for plane in cluster.local_volume:
		f.plane(plane)
	_write_aabbox(f, cluster.local_bbox)
	f.boolean(cluster.father_visible)
	f.boolean(cluster.visible_from_father)
	f.string(cluster.sound_group)
	f.string(cluster.env_fx_name)
	f.boolean(cluster.audible_from_father)
	f.boolean(cluster.father_audible)


def _write_portal(f: _Writer, portal: Portal) -> None:
	f.version(1)
	f.cont_len(len(portal.poly))
	for point in portal.poly:
		f.vector3(point)
	f.string(portal.name)
	f.string(portal.occlusion_model)
	f.string(portal.open_occlusion_model)


def _write_point_light(f: _Writer, light: PointLightNamed) -> None:
	f.version(2)
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
	f.version(1)
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
	f.version(1)
	f.cont_len(len(point_lights))
	for light in point_lights:
		_write_point_light_named(f, light)
	f.cont_len(len(groups))
	for group in groups:
		_write_point_light_group(f, group)


def _write_cell_corner(f: _Writer, cell: CellCorner) -> None:
	f.version(1)
	f.u8(cell.local_ambient_id)
	f.u8(cell.sun_contribution)
	f.u8(cell.light[0])
	f.u8(cell.light[1])


def _write_surface_light_grid(f: _Writer, grid: SurfaceLightGrid) -> None:
	f.version(0)
	f.vector2(grid.origin)
	f.u32(grid.width)
	f.u32(grid.height)
	f.cont_len(len(grid.cells))
	for cell in grid.cells:
		_write_cell_corner(f, cell)


def _write_retriever_light_grid(f: _Writer, retriever: RetrieverLightGrid) -> None:
	f.version(0)
	f.cont_len(len(retriever.grids))
	for grid in retriever.grids:
		_write_surface_light_grid(f, grid)


def _write_ig_surface_light(f: _Writer, surface_light: IgSurfaceLight) -> None:
	f.version(1)
	f.f32(surface_light.cell_size)
	f.f32(surface_light.oo_cell_size)
	f.cont_len(len(surface_light.retriever_grid_map))
	for key, retriever in surface_light.retriever_grid_map.items():
		f.u32(key)
		_write_retriever_light_grid(f, retriever)


def _write_instance(f: _Writer, inst: Instance) -> None:
	f.version(7)
	f.boolean(inst.visible)
	f.boolean(inst.dont_cast_shadow_for_exterior)
	f.boolean(inst.dont_cast_shadow_for_interior)
	f.u8(inst.local_ambient_id)
	f.boolean(inst.avoid_static_light_pre_compute)
	f.boolean(inst.dont_cast_shadow)
	f.boolean(inst.static_light_enabled)
	f.u8(inst.sun_contribution)
	f.u8(inst.light[0])
	f.u8(inst.light[1])
	f.string(inst.instance_name)
	f.boolean(inst.dont_add_to_scene)
	f.cont_len(len(inst.clusters))
	for cluster_id in inst.clusters:
		f.u32(cluster_id)
	f.string(inst.shape_name)
	f.vector3(inst.pos)
	f.quaternion(inst.rot)
	f.vector3(inst.scale)
	f.s32(inst.parent)


def dumps(ig: InstanceGroup) -> bytes:
	"""Serialize an InstanceGroup back to the .ig binary format.

	Every sub-object is written at its latest known format version,
	which is exactly what the NeL engine does when it resaves a file.
	"""
	f = _Writer()
	f.write_magic(MAGIC)
	f.version(5)

	f.boolean(ig.real_time_sun_contribution)

	surface_light = ig.surface_light if ig.surface_light is not None else IgSurfaceLight(1.0, 1.0, {})
	_write_ig_surface_light(f, surface_light)

	_write_point_light_array(f, ig.point_lights, ig.point_light_groups)

	global_pos = ig.global_pos if ig.global_pos is not None else Vector3(0.0, 0.0, 0.0)
	f.vector3(global_pos)

	f.cont_len(len(ig.clusters))
	for cluster in ig.clusters:
		_write_cluster(f, cluster)

	f.cont_len(len(ig.portals))
	for portal in ig.portals:
		_write_portal(f, portal)

	for cluster in ig.clusters:
		f.u32(len(cluster.portal_indices))
		for index in cluster.portal_indices:
			f.s32(index)

	f.cont_len(len(ig.instances))
	for inst in ig.instances:
		_write_instance(f, inst)

	return f.getvalue()


def save_ig(path: Union[str, Path, BinaryIO], ig: InstanceGroup) -> None:
	"""Serialize an InstanceGroup and write it to a path or open binary file object."""
	data = dumps(ig)
	if isinstance(path, (str, Path)):
		Path(path).write_bytes(data)
	else:
		path.write(data)


def _parse_aabbox(f: _Reader) -> AABBox:
	f.version()  # CAABBox version, always 0
	center = f.vector3()
	half_size = f.vector3()
	return AABBox(center, half_size)


def _parse_cluster(f: _Reader) -> Cluster:
	version = f.version()

	name = f.string() if version >= 1 else ""

	n_planes = f.cont_len()
	local_volume = [f.plane() for _ in range(n_planes)]

	local_bbox = _parse_aabbox(f)
	father_visible = f.boolean()
	visible_from_father = f.boolean()

	sound_group = ""
	env_fx_name = ""
	if version >= 2:
		sound_group = f.string()
		env_fx_name = f.string()

	audible_from_father = False
	father_audible = False
	if version >= 3:
		audible_from_father = f.boolean()
		father_audible = f.boolean()

	return Cluster(
		name=name,
		local_volume=local_volume,
		local_bbox=local_bbox,
		father_visible=father_visible,
		visible_from_father=visible_from_father,
		sound_group=sound_group,
		env_fx_name=env_fx_name,
		audible_from_father=audible_from_father,
		father_audible=father_audible,
	)


def _parse_portal(f: _Reader) -> Portal:
	version = f.version()

	n_points = f.cont_len()
	poly = [f.vector3() for _ in range(n_points)]

	name = f.string()

	occlusion_model = ""
	open_occlusion_model = ""
	if version >= 1:
		occlusion_model = f.string()
		open_occlusion_model = f.string()

	return Portal(
		poly=poly,
		name=name,
		occlusion_model=occlusion_model,
		open_occlusion_model=open_occlusion_model,
	)


def _parse_point_light(f: _Reader) -> dict:
	"""CPointLight::serial (base class of CPointLightNamed)."""
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
		add_ambient_with_sun=add_ambient_with_sun,
		light_type=light_type,
		spot_direction=spot_direction,
		spot_angle_begin=spot_angle_begin,
		spot_angle_end=spot_angle_end,
		position=position,
		ambient=ambient,
		diffuse=diffuse,
		specular=specular,
		attenuation_begin=attenuation_begin,
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
		**base,
		animated_light=animated_light,
		default_ambient=default_ambient,
		default_diffuse=default_diffuse,
		default_specular=default_specular,
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
		for _ in range(n_groups):
			groups.append(_parse_point_light_group(f))
	else:
		# deprecated map<string, CPointLightGroupV0> -> (StartId, EndId) only
		for _ in range(n_groups):
			name = f.string()
			start_id = f.u32()
			end_id = f.u32()
			groups.append(PointLightGroup(name, 0, start_id, end_id))

	return point_lights, groups


def _parse_cell_corner(f: _Reader) -> CellCorner:
	version = f.version()
	local_ambient_id = f.u8() if version >= 1 else 0xFF
	sun_contribution = f.u8()
	light = (f.u8(), f.u8())
	return CellCorner(local_ambient_id, sun_contribution, light)


def _parse_surface_light_grid(f: _Reader) -> SurfaceLightGrid:
	f.version()  # always 0
	origin = f.vector2()
	width = f.u32()
	height = f.u32()
	n_cells = f.cont_len()
	cells = [_parse_cell_corner(f) for _ in range(n_cells)]
	return SurfaceLightGrid(origin, width, height, cells)


def _parse_retriever_light_grid(f: _Reader) -> RetrieverLightGrid:
	f.version()  # always 0
	n_grids = f.cont_len()
	grids = [_parse_surface_light_grid(f) for _ in range(n_grids)]
	return RetrieverLightGrid(grids)


def _parse_ig_surface_light(f: _Reader) -> IgSurfaceLight:
	version = f.version()
	cell_size = f.f32()
	oo_cell_size = f.f32()

	retriever_grid_map: Dict[int, RetrieverLightGrid] = {}
	n_entries = f.cont_len()
	if version == 0:
		# deprecated map<string, CRetrieverLightGrid>
		for _ in range(n_entries):
			key = f.string()
			retriever_grid_map[key] = _parse_retriever_light_grid(f)
	else:
		for _ in range(n_entries):
			key = f.u32()
			retriever_grid_map[key] = _parse_retriever_light_grid(f)

	return IgSurfaceLight(cell_size, oo_cell_size, retriever_grid_map)


def _parse_instance(f: _Reader) -> Instance:
	version = f.version()

	visible = f.boolean() if version >= 7 else True
	dont_cast_shadow_for_exterior = f.boolean() if version >= 6 else False
	dont_cast_shadow_for_interior = f.boolean() if version >= 5 else False
	local_ambient_id = f.u8() if version >= 4 else 0xFF

	avoid_static_light_pre_compute = False
	dont_cast_shadow = False
	static_light_enabled = False
	sun_contribution = 0
	light = (0xFF, 0xFF)
	if version >= 3:
		avoid_static_light_pre_compute = f.boolean()
		dont_cast_shadow = f.boolean()
		static_light_enabled = f.boolean()
		sun_contribution = f.u8()
		light = (f.u8(), f.u8())

	instance_name = ""
	dont_add_to_scene = False
	if version >= 2:
		instance_name = f.string()
		dont_add_to_scene = f.boolean()

	clusters: List[int] = []
	if version >= 1:
		n_clusters = f.cont_len()
		clusters = [f.u32() for _ in range(n_clusters)]

	shape_name = f.string()
	pos = f.vector3()
	rot = f.quaternion()
	scale = f.vector3()
	parent = f.s32()

	return Instance(
		shape_name=shape_name,
		pos=pos,
		rot=rot,
		scale=scale,
		parent=parent,
		clusters=clusters,
		instance_name=instance_name,
		dont_add_to_scene=dont_add_to_scene,
		avoid_static_light_pre_compute=avoid_static_light_pre_compute,
		dont_cast_shadow=dont_cast_shadow,
		static_light_enabled=static_light_enabled,
		sun_contribution=sun_contribution,
		light=light,
		local_ambient_id=local_ambient_id,
		dont_cast_shadow_for_interior=dont_cast_shadow_for_interior,
		dont_cast_shadow_for_exterior=dont_cast_shadow_for_exterior,
		visible=visible,
	)


def parse_ig(data: bytes) -> InstanceGroup:
	"""Parse the content of a .ig file already loaded into memory."""
	f = _Reader(data)
	f.check_magic(MAGIC)

	version = f.version()

	real_time_sun_contribution = True
	if version >= 5:
		real_time_sun_contribution = f.boolean()

	surface_light = None
	if version >= 4:
		surface_light = _parse_ig_surface_light(f)

	point_lights: List[PointLightNamed] = []
	point_light_groups: List[PointLightGroup] = []
	if version >= 3:
		point_lights, point_light_groups = _parse_point_light_array(f)

	global_pos = None
	if version >= 2:
		global_pos = f.vector3()

	clusters: List[Cluster] = []
	portals: List[Portal] = []
	if version >= 1:
		n_clusters = f.cont_len()
		clusters = [_parse_cluster(f) for _ in range(n_clusters)]

		n_portals = f.cont_len()
		portals = [_parse_portal(f) for _ in range(n_portals)]

		# Cluster <-> portal links, in cluster order.
		for cluster in clusters:
			n_linked = f.u32()
			cluster.portal_indices = [f.s32() for _ in range(n_linked)]

	n_instances = f.cont_len()
	instances = [_parse_instance(f) for _ in range(n_instances)]

	return InstanceGroup(
		real_time_sun_contribution=real_time_sun_contribution,
		surface_light=surface_light,
		point_lights=point_lights,
		point_light_groups=point_light_groups,
		global_pos=global_pos,
		clusters=clusters,
		portals=portals,
		instances=instances,
	)


def load_ig(path: Union[str, Path, BinaryIO]) -> InstanceGroup:
	"""Load and parse a .ig file from a path or an open binary file object."""
	if isinstance(path, (str, Path)):
		data = Path(path).read_bytes()
	else:
		data = path.read()
	return parse_ig(data)


def _parse_floats(text: str, count: int) -> Tuple[float, ...]:
	parts = text.split(",")
	if len(parts) != count:
		raise argparse.ArgumentTypeError(f"expected {count} comma-separated numbers, got {text!r}")
	try:
		return tuple(float(p) for p in parts)
	except ValueError as exc:
		raise argparse.ArgumentTypeError(f"not a number in {text!r}: {exc}")


def _parse_vector3_arg(text: str) -> Vector3:
	return Vector3(*_parse_floats(text, 3))


def _parse_quaternion_arg(text: str) -> Quaternion:
	return Quaternion(*_parse_floats(text, 4))


def _parse_rgba_arg(text: str) -> Rgba:
	parts = text.split(",")
	if len(parts) == 3:
		parts.append("255")
	if len(parts) != 4:
		raise argparse.ArgumentTypeError(f"expected r,g,b[,a], got {text!r}")
	try:
		r, g, b, a = (int(p) for p in parts)
	except ValueError as exc:
		raise argparse.ArgumentTypeError(f"not an integer in {text!r}: {exc}")
	return Rgba(r, g, b, a)


def _find_instance(ig: InstanceGroup, selector: str) -> Instance:
	"""Resolve a CLI instance selector: a 0-based index, or a unique instance name."""
	try:
		return ig.instances[int(selector)]
	except ValueError:
		pass
	except IndexError:
		raise SystemExit(f"no instance at index {selector}")

	matches = [inst for inst in ig.instances if inst.instance_name == selector]
	if not matches:
		raise SystemExit(f"no instance named {selector!r}")
	if len(matches) > 1:
		raise SystemExit(
			f"instance name {selector!r} is ambiguous ({len(matches)} matches), use a 0-based index instead"
		)
	return matches[0]


def _dump(ig: InstanceGroup, list_instances: bool) -> None:
	print(f"clusters: {len(ig.clusters)}")
	print(f"portals: {len(ig.portals)}")
	print(f"point lights: {len(ig.point_lights)}")
	print(f"instances: {len(ig.instances)}")
	if ig.global_pos is not None:
		print(f"global pos: {ig.global_pos}")

	if list_instances:
		for i, inst in enumerate(ig.instances):
			label = inst.instance_name or "(unnamed)"
			print(f"  [{i}] {label}: shape={inst.shape_name!r} pos={inst.pos} parent={inst.parent}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read and edit Ryzom .ig files")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .ig file")
	p_dump.add_argument("path", type=Path)
	p_dump.add_argument("--instances", action="store_true", help="list every instance")

	p_add = sub.add_parser("add-instance", help="insert a new instance")
	p_add.add_argument("path", type=Path)
	p_add.add_argument("shape", help="shape file name, e.g. fyros_house.shape")
	p_add.add_argument("--pos", required=True, type=_parse_vector3_arg, metavar="X,Y,Z")
	p_add.add_argument("--rot", type=_parse_quaternion_arg, metavar="X,Y,Z,W", help="default: 0,0,0,1")
	p_add.add_argument("--scale", type=_parse_vector3_arg, metavar="X,Y,Z", help="default: 1,1,1")
	p_add.add_argument("--parent", type=int, default=-1, help="parent instance index, default: -1 (none)")
	p_add.add_argument("--name", default="", dest="instance_name", help="instance name")
	p_add.add_argument("-o", "--output", type=Path, required=True)

	p_rm = sub.add_parser("remove-instance", help="delete an instance")
	p_rm.add_argument("path", type=Path)
	p_rm.add_argument("instance", help="instance name or 0-based index")
	p_rm.add_argument("-o", "--output", type=Path, required=True)

	p_pos = sub.add_parser("set-pos", help="change an instance's position")
	p_pos.add_argument("path", type=Path)
	p_pos.add_argument("instance", help="instance name or 0-based index")
	p_pos.add_argument("value", type=_parse_vector3_arg, metavar="X,Y,Z")
	p_pos.add_argument("-o", "--output", type=Path, required=True)

	p_rot = sub.add_parser("set-rot", help="change an instance's rotation")
	p_rot.add_argument("path", type=Path)
	p_rot.add_argument("instance", help="instance name or 0-based index")
	p_rot.add_argument("value", type=_parse_quaternion_arg, metavar="X,Y,Z,W")
	p_rot.add_argument("-o", "--output", type=Path, required=True)

	p_scale = sub.add_parser("set-scale", help="change an instance's scale")
	p_scale.add_argument("path", type=Path)
	p_scale.add_argument("instance", help="instance name or 0-based index")
	p_scale.add_argument("value", type=_parse_vector3_arg, metavar="X,Y,Z")
	p_scale.add_argument("-o", "--output", type=Path, required=True)

	p_gpos = sub.add_parser("set-global-pos", help="change the group's global position")
	p_gpos.add_argument("path", type=Path)
	p_gpos.add_argument("value", type=_parse_vector3_arg, metavar="X,Y,Z")
	p_gpos.add_argument("-o", "--output", type=Path, required=True)

	p_add_light = sub.add_parser("add-point-light", help="insert a new named point light")
	p_add_light.add_argument("path", type=Path)
	p_add_light.add_argument("--pos", required=True, type=_parse_vector3_arg, metavar="X,Y,Z")
	p_add_light.add_argument("--diffuse", type=_parse_rgba_arg, metavar="R,G,B[,A]", help="default: 255,255,255,255")
	p_add_light.add_argument("--ambient", type=_parse_rgba_arg, metavar="R,G,B[,A]", help="default: 0,0,0,255")
	p_add_light.add_argument("--attenuation-begin", type=float, default=1.0)
	p_add_light.add_argument("--attenuation-end", type=float, default=10.0)
	p_add_light.add_argument("-o", "--output", type=Path, required=True)

	p_rm_light = sub.add_parser("remove-point-light", help="delete a named point light")
	p_rm_light.add_argument("path", type=Path)
	p_rm_light.add_argument("index", type=int, help="0-based index in the point light list")
	p_rm_light.add_argument("-o", "--output", type=Path, required=True)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()
	ig = load_ig(args.path)

	if args.command == "dump":
		_dump(ig, args.instances)
		return

	if args.command == "add-instance":
		inst = Instance(shape_name=args.shape, pos=args.pos, instance_name=args.instance_name, parent=args.parent)
		if args.rot is not None:
			inst.rot = args.rot
		if args.scale is not None:
			inst.scale = args.scale
		ig.instances.append(inst)
		print(f"added instance {len(ig.instances) - 1}: shape={inst.shape_name!r} pos={inst.pos}")

	elif args.command == "remove-instance":
		inst = _find_instance(ig, args.instance)
		ig.instances.remove(inst)
		print(f"removed instance: shape={inst.shape_name!r} name={inst.instance_name!r}")

	elif args.command == "set-pos":
		inst = _find_instance(ig, args.instance)
		inst.pos = args.value
		print(f"{inst.instance_name or inst.shape_name}: pos -> {inst.pos}")

	elif args.command == "set-rot":
		inst = _find_instance(ig, args.instance)
		inst.rot = args.value
		print(f"{inst.instance_name or inst.shape_name}: rot -> {inst.rot}")

	elif args.command == "set-scale":
		inst = _find_instance(ig, args.instance)
		inst.scale = args.value
		print(f"{inst.instance_name or inst.shape_name}: scale -> {inst.scale}")

	elif args.command == "set-global-pos":
		ig.global_pos = args.value
		print(f"global pos -> {ig.global_pos}")

	elif args.command == "add-point-light":
		light = PointLightNamed(
			position=args.pos,
			attenuation_begin=args.attenuation_begin,
			attenuation_end=args.attenuation_end,
		)
		if args.diffuse is not None:
			light.diffuse = args.diffuse
			light.default_diffuse = args.diffuse
		if args.ambient is not None:
			light.ambient = args.ambient
			light.default_ambient = args.ambient
		ig.point_lights.append(light)
		print(f"added point light {len(ig.point_lights) - 1}: pos={light.position}")

	elif args.command == "remove-point-light":
		if not 0 <= args.index < len(ig.point_lights):
			raise SystemExit(f"no point light at index {args.index}")
		removed = ig.point_lights.pop(args.index)
		print(f"removed point light: pos={removed.position}")

	save_ig(args.output, ig)
	print(f"wrote {args.output}")


if __name__ == "__main__":
	_main()
