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

"""Read-only reader for Ryzom/NeL .shape files (3D meshes).

Format reverse-engineered from nel/src/3d/shape.cpp (CShapeStream, the
"SHAP" magic and the polymorphic-pointer dispatch), plus the serial()
methods of every concrete IShape subclass: CMesh (mesh.cpp/mesh_base.cpp),
CMeshMRM (mesh_mrm.cpp), CMeshMRMSkinned (mesh_mrm_skinned.cpp),
CMeshMultiLod (mesh_multi_lod.cpp), CSkeletonShape (skeleton_shape.cpp,
bone.cpp), CFlareShape (flare_shape.cpp), CWaterShape / CWaveMakerShape
(water_shape.cpp), CSegRemanenceShape (seg_remanence_shape.cpp) and
CParticleSystemShape (particle_system_shape.cpp). Shared building blocks
(CMaterial, CVertexBuffer, CIndexBuffer, CMatrix, CTrackDefault*, ITexture)
were verified against material.cpp, vertex_buffer.cpp, index_buffer.cpp,
matrix.cpp, track.h and texture_file.cpp.

Unlike .ig, this reader is READ-ONLY: .shape geometry (in particular
CMeshMRM's progressive multi-resolution deltas) has no practical use case
for round-trip editing, so no writer is provided.

Known limitations (all fail with a clear ShapeParseError rather than
silently producing wrong data):
  - Only NULL and CTextureFile textures are supported; other ITexture
    subclasses (CTextureBump, CTextureMultiFile, procedural textures...)
    are not decodable by this reader.
  - Mesh vertex programs (CMeshVPWindTree, CMeshVPPerPixelLight) are not
    supported; meshes using them will raise.
  - CLodCharacterTexture (an optional, rarely-used field) is not supported.
  - CWaterShape/CFlareShape almost always reference unsupported texture
    classes in real game data, so full parsing of those is mostly
    theoretical; the type will still be identified before the error.

Usage:
	from ryzom_shape import load_shape
	shape = load_shape("box.shape")
	print(shape.type_name, shape.value.num_vertices, shape.value.num_triangles)
"""

import argparse
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, BinaryIO, Dict, List, Optional, Tuple, Union

MAGIC = b"SHAP"


class ShapeParseError(Exception):
	pass


# ---------------------------------------------------------------------------
# Primitives
# ---------------------------------------------------------------------------


@dataclass
class Vector2:
	x: float
	y: float


@dataclass
class Vector3:
	x: float
	y: float
	z: float


@dataclass
class Quaternion:
	x: float
	y: float
	z: float
	w: float


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
class Matrix:
	"""NLMISC::CMatrix, using its state-bit-dependent sparse encoding."""
	state_bit: int
	scale: float
	rot: Optional[Tuple[float, ...]]  # 9 floats (a11..a33), row-major, or None
	trans: Optional[Tuple[float, float, float]]
	proj: Optional[Tuple[float, float, float, float]]


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0
		self.seen: Dict[int, Any] = {}

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise ShapeParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def tell(self) -> int:
		return self._pos

	def u8(self) -> int:
		return self._take(1)[0]

	def u16(self) -> int:
		return struct.unpack("<H", self._take(2))[0]

	def s16(self) -> int:
		return struct.unpack("<h", self._take(2))[0]

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

	def vector2(self) -> Vector2:
		return Vector2(self.f32(), self.f32())

	def vector3(self) -> Vector3:
		return Vector3(self.f32(), self.f32(), self.f32())

	def quaternion(self) -> Quaternion:
		return Quaternion(self.f32(), self.f32(), self.f32(), self.f32())

	def rgba(self) -> Rgba:
		return Rgba(self.u8(), self.u8(), self.u8(), self.u8())

	def version(self) -> int:
		b = self.u8()
		if b == 0xFF:
			return self.u32()
		return b

	def check_magic(self, expected: bytes) -> None:
		got = self._take(len(expected))
		if got != expected:
			raise ShapeParseError(f"bad magic: expected {expected!r}, got {got!r}")

	def cont_len(self) -> int:
		return self.s32()

	def read_uint_array(self, count: int, item_size: int, fmt: str) -> List[int]:
		"""Reads `count` primitives with the given struct format char, no length prefix."""
		if count == 0:
			return []
		data = self._take(count * item_size)
		return list(struct.unpack(f"<{count}{fmt}", data))

	def cont_uint_vector(self, item_size: int, fmt: str) -> List[int]:
		"""serialCont() of a vector<uintN>/vector<sintN>: length-prefixed primitive vector."""
		n = self.cont_len()
		return self.read_uint_array(n, item_size, fmt)

	def skip_pair_vector(self) -> None:
		"""serialCont() of a vector of two-uint32 structs with no per-element version
		(e.g. CMRMWedgeGeom Start/End, CVertexBlock VertexStart/NVertices)."""
		n = self.cont_len()
		if n:
			self._take(n * 8)

	def string_vector(self) -> List[str]:
		n = self.cont_len()
		return [self.string() for _ in range(n)]

	def aabbox(self) -> AABBox:
		self.version()  # CAABBox/CAABBoxExt version, always 0
		center = self.vector3()
		half_size = self.vector3()
		return AABBox(center, half_size)

	def track_default_vector(self) -> Vector3:
		self.version()
		return self.vector3()

	def track_default_quat(self) -> Quaternion:
		self.version()
		return self.quaternion()

	def track_default_rgba(self) -> Rgba:
		self.version()
		return self.rgba()

	def track_default_float(self) -> float:
		self.version()
		return self.f32()

	def track_default_int(self) -> int:
		self.version()
		return self.s32()

	def track_default_bool(self) -> bool:
		self.version()
		return self.boolean()

	def matrix(self) -> Matrix:
		self.version()
		state_bit = self.u32()
		scale = self.f32()  # always present when reading (see CMatrix::serial)
		has_rot = (state_bit & (2 | 4 | 8)) != 0
		has_trans = (state_bit & 1) != 0
		has_proj = (state_bit & 16) != 0
		rot = None
		if has_rot:
			rot = tuple(self.f32() for _ in range(9))
		trans = None
		if has_trans:
			trans = (self.f32(), self.f32(), self.f32())
		proj = None
		if has_proj:
			proj = (self.f32(), self.f32(), self.f32(), self.f32())
		return Matrix(state_bit, scale, rot, trans, proj)


class ShapeWriteError(Exception):
	pass


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding.

	Polymorphic pointer node ids are freshly assigned starting from a very
	high base, never reused from the original file: since some geometry
	sections are copied verbatim as opaque bytes (see `_raw_geom` on
	MeshMRM/MeshMRMSkinned/MeshMultiLod), any node ids embedded in those
	untouched bytes must stay distinct from the ids we actively assign here.
	"""

	_NEXT_ID_BASE = 1 << 62

	def __init__(self):
		self._chunks: List[bytes] = []
		self._written_ids: Dict[int, int] = {}  # id(python object) -> node id
		self._next_id = self._NEXT_ID_BASE

	def u8(self, v: int) -> None:
		self._chunks.append(struct.pack("<B", v))

	def u16(self, v: int) -> None:
		self._chunks.append(struct.pack("<H", v))

	def s16(self, v: int) -> None:
		self._chunks.append(struct.pack("<h", v))

	def u32(self, v: int) -> None:
		self._chunks.append(struct.pack("<I", v))

	def s32(self, v: int) -> None:
		self._chunks.append(struct.pack("<i", v))

	def u64(self, v: int) -> None:
		self._chunks.append(struct.pack("<Q", v))

	def f32(self, v: float) -> None:
		self._chunks.append(struct.pack("<f", v))

	def boolean(self, v: bool) -> None:
		self.u8(1 if v else 0)

	def string(self, s: str) -> None:
		raw = s.encode("latin-1")
		self.u32(len(raw))
		self._chunks.append(raw)

	def raw(self, data: bytes) -> None:
		self._chunks.append(data)

	def vector2(self, v: Vector2) -> None:
		self.f32(v.x)
		self.f32(v.y)

	def vector3(self, v: Vector3) -> None:
		self.f32(v.x)
		self.f32(v.y)
		self.f32(v.z)

	def quaternion(self, q: Quaternion) -> None:
		self.f32(q.x)
		self.f32(q.y)
		self.f32(q.z)
		self.f32(q.w)

	def rgba(self, c: Rgba) -> None:
		self.u8(c.r)
		self.u8(c.g)
		self.u8(c.b)
		self.u8(c.a)

	def version(self, v: int) -> None:
		if v < 0xFF:
			self.u8(v)
		else:
			self.u8(0xFF)
			self.u32(v)

	def write_magic(self, magic: bytes) -> None:
		self._chunks.append(magic)

	def cont_len(self, n: int) -> None:
		self.s32(n)

	def uint_array(self, values: List[int], item_size: int, fmt: str) -> None:
		if not values:
			return
		self._chunks.append(struct.pack(f"<{len(values)}{fmt}", *values))

	def cont_uint_vector(self, values: List[int], item_size: int, fmt: str) -> None:
		self.cont_len(len(values))
		self.uint_array(values, item_size, fmt)

	def pair_vector(self, pairs: List[Tuple[int, int]]) -> None:
		self.cont_len(len(pairs))
		for a, b in pairs:
			self.u32(a)
			self.u32(b)

	def string_vector(self, names: List[str]) -> None:
		self.cont_len(len(names))
		for name in names:
			self.string(name)

	def aabbox(self, box: AABBox) -> None:
		self.version(0)
		self.vector3(box.center)
		self.vector3(box.half_size)

	def track_default_vector(self, v: Vector3) -> None:
		self.version(0)
		self.vector3(v)

	def track_default_quat(self, q: Quaternion) -> None:
		self.version(0)
		self.quaternion(q)

	def track_default_rgba(self, c: Rgba) -> None:
		self.version(0)
		self.rgba(c)

	def track_default_float(self, v: float) -> None:
		self.version(0)
		self.f32(v)

	def track_default_int(self, v: int) -> None:
		self.version(0)
		self.s32(v)

	def matrix(self, m: Matrix) -> None:
		self.version(0)
		self.u32(m.state_bit)
		has_scale_uniform = (m.state_bit & (4 | 8)) == 4
		# Mirrors CMatrix::serial's write path: Scale33 is only meaningful
		# (and only round-trips) when hasScaleUniform() is true.
		self.f32(m.scale if has_scale_uniform else 1.0)
		has_rot = (m.state_bit & (2 | 4 | 8)) != 0
		if has_rot:
			for v in m.rot:
				self.f32(v)
		has_trans = (m.state_bit & 1) != 0
		if has_trans:
			for v in m.trans:
				self.f32(v)
		has_proj = (m.state_bit & 16) != 0
		if has_proj:
			for v in m.proj:
				self.f32(v)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)

	def write_poly_ptr(self, obj, class_name: Optional[str], write_body) -> None:
		"""Mirrors IStream::serialIStreamable on write: node id, then class name
		+ body only the first time a given python object is written."""
		if obj is None:
			self.u64(0)
			return
		key = id(obj)
		existing = self._written_ids.get(key)
		if existing is not None:
			self.u64(existing)
			return
		node = self._next_id
		self._next_id += 1
		self._written_ids[key] = node
		self.u64(node)
		self.string(class_name)
		write_body(self, obj)


# ---------------------------------------------------------------------------
# Polymorphic pointer dispatch (IStream::serialIStreamable)
# ---------------------------------------------------------------------------

_CLASS_PARSERS: Dict[str, Any] = {}


def _read_poly_ptr(f: _Reader):
	"""Mirrors IStream::serialIStreamable: node id, then class name + body if new."""
	node = f.u64()
	if node == 0:
		return None
	if node in f.seen:
		return f.seen[node]
	class_name = f.string()
	parser = _CLASS_PARSERS.get(class_name)
	if parser is None:
		raise ShapeParseError(
			f"unsupported class {class_name!r} at offset {f.tell()}: cannot safely "
			f"skip a polymorphic object of unknown format"
		)
	obj = parser(f)
	f.seen[node] = obj
	return obj


# ---------------------------------------------------------------------------
# Textures
# ---------------------------------------------------------------------------


@dataclass
class Texture:
	class_name: str
	file_name: Optional[str] = None
	allow_degradation: Optional[bool] = None
	file_names: Optional[List[str]] = None  # CTextureMultiFile: all candidate files
	selected_index: Optional[int] = None  # CTextureMultiFile: _CurrSelectedTexture
	sub_textures: Optional[List[Optional["Texture"]]] = None  # CTextureCube: 6 faces


def _parse_itexture_base(f: _Reader) -> None:
	ver = f.version()
	f.s32()  # _UploadFormat (serialEnum)
	f.s32()  # _WrapS
	f.s32()  # _WrapT
	f.s32()  # _MinFilter
	f.s32()  # _MagFilter
	if ver >= 1:
		f.boolean()  # _LoadGrayscaleAsAlpha


def _parse_texture_file(f: _Reader) -> Texture:
	ver = f.version()
	_parse_itexture_base(f)
	file_name = f.string()
	allow_degradation = f.boolean() if ver >= 1 else None
	return Texture(class_name="CTextureFile", file_name=file_name, allow_degradation=allow_degradation)


_CLASS_PARSERS["CTextureFile"] = _parse_texture_file


def _parse_texture_multi_file(f: _Reader) -> Texture:
	f.version()
	_parse_itexture_base(f)
	n = f.cont_len()
	file_names = [f.string() for _ in range(n)]
	curr_selected = f.u32()
	selected_name = file_names[curr_selected] if 0 <= curr_selected < len(file_names) else None
	return Texture(
		class_name="CTextureMultiFile", file_name=selected_name,
		file_names=file_names, selected_index=curr_selected,
	)


_CLASS_PARSERS["CTextureMultiFile"] = _parse_texture_multi_file


def _parse_texture_cube(f: _Reader) -> Texture:
	ver = f.version()
	_parse_itexture_base(f)
	sub_textures = [_read_poly_ptr(f) for _ in range(6)]
	if ver == 1:
		f.boolean()  # legacy, unused
	return Texture(class_name="CTextureCube", sub_textures=sub_textures)


_CLASS_PARSERS["CTextureCube"] = _parse_texture_cube


def _read_texture_ptr(f: _Reader) -> Optional[Texture]:
	return _read_poly_ptr(f)


def _write_itexture_base(f: _Writer) -> None:
	f.version(1)
	f.s32(0)  # _UploadFormat
	f.s32(0)  # _WrapS
	f.s32(0)  # _WrapT
	f.s32(0)  # _MinFilter
	f.s32(0)  # _MagFilter
	f.boolean(False)  # _LoadGrayscaleAsAlpha


def _write_texture_file(f: _Writer, tex: Texture) -> None:
	f.version(1)
	_write_itexture_base(f)
	f.string(tex.file_name or "")
	f.boolean(bool(tex.allow_degradation))


def _write_texture_multi_file(f: _Writer, tex: Texture) -> None:
	f.version(0)
	_write_itexture_base(f)
	file_names = tex.file_names or []
	f.string_vector(file_names)
	selected = tex.selected_index
	if selected is None:
		# Only derive an index when none was captured (e.g. a Texture built by
		# hand); a captured index -- even one out of range in the original
		# file -- is preserved verbatim rather than "fixed".
		try:
			selected = file_names.index(tex.file_name) if tex.file_name else 0
		except ValueError:
			selected = 0
	f.u32(selected)


def _write_texture_cube(f: _Writer, tex: Texture) -> None:
	f.version(2)
	_write_itexture_base(f)
	sub_textures = tex.sub_textures or [None] * 6
	for i in range(6):
		_write_texture_ptr(f, sub_textures[i] if i < len(sub_textures) else None)


_TEXTURE_WRITERS = {
	"CTextureFile": _write_texture_file,
	"CTextureMultiFile": _write_texture_multi_file,
	"CTextureCube": _write_texture_cube,
}


def _write_texture_ptr(f: _Writer, tex: Optional[Texture]) -> None:
	if tex is None:
		f.u64(0)
		return
	writer = _TEXTURE_WRITERS.get(tex.class_name)
	if writer is None:
		raise ShapeWriteError(f"cannot write unsupported texture class {tex.class_name!r}")
	f.write_poly_ptr(tex, tex.class_name, writer)


# ---------------------------------------------------------------------------
# CMaterial
# ---------------------------------------------------------------------------


@dataclass
class TexEnv:
	"""CTexEnv: raw texture-blending state, kept for lossless round-trip."""
	op_rgb: int
	src_arg0_rgb: int
	op_arg0_rgb: int
	src_arg1_rgb: int
	op_arg1_rgb: int
	op_alpha: int
	src_arg0_alpha: int
	op_arg0_alpha: int
	src_arg1_alpha: int
	op_arg1_alpha: int
	constant_color: Rgba
	src_arg2_rgb: Optional[int] = None  # only present if material ver>=9
	op_arg2_rgb: Optional[int] = None
	src_arg2_alpha: Optional[int] = None
	op_arg2_alpha: Optional[int] = None


@dataclass
class MaterialLightMap:
	factor: Rgba
	diffuse: Rgba
	ambient: Optional[Rgba]  # only present for the modern (ver>=7 material) format
	texture: Optional[Texture]


@dataclass
class Material:
	shader_type: int
	flags: int
	src_blend: int
	dst_blend: int
	z_function: int
	z_bias: float
	color: Rgba
	emissive: Rgba
	ambient: Rgba
	diffuse: Rgba
	specular: Rgba
	shininess: float
	alpha_test_threshold: float
	tex_coord_gen_mode: int
	textures: List[Optional[Texture]]
	tex_envs: List[Optional[TexEnv]] = field(default_factory=list)
	light_maps: List[MaterialLightMap] = field(default_factory=list)
	light_maps_mulx2: Optional[bool] = None
	tex_addr_mode: Optional[List[int]] = None
	tex_user_mat: Dict[int, Matrix] = field(default_factory=dict)


def _parse_tex_env(f: _Reader, version: int) -> TexEnv:
	op_rgb = f.u8()
	src_arg0_rgb = f.u8()
	op_arg0_rgb = f.u8()
	src_arg1_rgb = f.u8()
	op_arg1_rgb = f.u8()
	src_arg2_rgb = None
	op_arg2_rgb = None
	if version >= 1:
		src_arg2_rgb = f.u8()
		op_arg2_rgb = f.u8()
	op_alpha = f.u8()
	src_arg0_alpha = f.u8()
	op_arg0_alpha = f.u8()
	src_arg1_alpha = f.u8()
	op_arg1_alpha = f.u8()
	src_arg2_alpha = None
	op_arg2_alpha = None
	if version >= 1:
		src_arg2_alpha = f.u8()
		op_arg2_alpha = f.u8()
	constant_color = f.rgba()
	return TexEnv(
		op_rgb=op_rgb, src_arg0_rgb=src_arg0_rgb, op_arg0_rgb=op_arg0_rgb,
		src_arg1_rgb=src_arg1_rgb, op_arg1_rgb=op_arg1_rgb,
		op_alpha=op_alpha, src_arg0_alpha=src_arg0_alpha, op_arg0_alpha=op_arg0_alpha,
		src_arg1_alpha=src_arg1_alpha, op_arg1_alpha=op_arg1_alpha,
		constant_color=constant_color,
		src_arg2_rgb=src_arg2_rgb, op_arg2_rgb=op_arg2_rgb,
		src_arg2_alpha=src_arg2_alpha, op_arg2_alpha=op_arg2_alpha,
	)


IDRV_MAT_MAXTEXTURES = 4
IDRV_MAT_TEX_ADDR = 0x00000400
IDRV_MAT_USER_TEX_FIRST_BIT = 20


def _parse_material(f: _Reader) -> Material:
	ver = f.version()

	shader_type = f.s32()
	flags = f.u32()
	src_blend = f.s32()
	dst_blend = f.s32()
	z_function = f.s32()
	z_bias = f.f32()
	color = f.rgba()
	emissive = f.rgba()
	ambient = f.rgba()
	diffuse = f.rgba()
	specular = f.rgba()

	shininess = f.f32() if ver >= 2 else 0.0
	alpha_test_threshold = f.f32() if ver >= 5 else 0.0
	tex_coord_gen_mode = f.u16() if ver >= 8 else 0

	textures: List[Optional[Texture]] = []
	tex_envs: List[Optional[TexEnv]] = []
	tex_env_version = 1 if ver >= 9 else 0
	for _ in range(IDRV_MAT_MAXTEXTURES):
		tex = _read_texture_ptr(f)
		textures.append(tex)
		if ver >= 1:
			tex_envs.append(_parse_tex_env(f, tex_env_version))
		else:
			tex_envs.append(None)

	light_maps: List[MaterialLightMap] = []
	light_maps_mulx2: Optional[bool] = None
	if ver >= 3:
		if ver >= 7:
			n = f.u32()
			for _ in range(n):
				f.version()  # CLightMap::serial2 version (fixed at 1 when written)
				factor = f.rgba()
				diffuse_lm = f.rgba()
				ambient_lm = f.rgba()
				tex = _read_texture_ptr(f)
				light_maps.append(MaterialLightMap(factor=factor, diffuse=diffuse_lm, ambient=ambient_lm, texture=tex))
			light_maps_mulx2 = f.boolean()
		else:
			n = f.cont_len()
			for _ in range(n):
				factor = f.rgba()  # deprecated CLightMap::serial
				tex = _read_texture_ptr(f)
				light_maps.append(MaterialLightMap(factor=factor, diffuse=Rgba(255, 255, 255, 255), ambient=None, texture=tex))

	tex_addr_mode: Optional[List[int]] = None
	if ver >= 4 and (flags & IDRV_MAT_TEX_ADDR):
		tex_addr_mode = [f.u8() for _ in range(IDRV_MAT_MAXTEXTURES)]

	tex_user_mat: Dict[int, Matrix] = {}
	if ver >= 6:
		for i in range(IDRV_MAT_MAXTEXTURES):
			if flags & (0x00100000 << i):
				tex_user_mat[i] = f.matrix()

	return Material(
		shader_type=shader_type,
		flags=flags,
		src_blend=src_blend,
		dst_blend=dst_blend,
		z_function=z_function,
		z_bias=z_bias,
		color=color,
		emissive=emissive,
		ambient=ambient,
		diffuse=diffuse,
		specular=specular,
		shininess=shininess,
		alpha_test_threshold=alpha_test_threshold,
		tex_coord_gen_mode=tex_coord_gen_mode,
		textures=textures,
		tex_envs=tex_envs,
		light_maps=light_maps,
		light_maps_mulx2=light_maps_mulx2,
		tex_addr_mode=tex_addr_mode,
		tex_user_mat=tex_user_mat,
	)


_DEFAULT_TEX_ENV = TexEnv(
	op_rgb=0, src_arg0_rgb=0, op_arg0_rgb=0, src_arg1_rgb=0, op_arg1_rgb=0,
	op_alpha=0, src_arg0_alpha=0, op_arg0_alpha=0, src_arg1_alpha=0, op_arg1_alpha=0,
	constant_color=Rgba(255, 255, 255, 255),
)


def _write_tex_env(f: _Writer, env: Optional[TexEnv], version: int) -> None:
	env = env or _DEFAULT_TEX_ENV
	f.u8(env.op_rgb)
	f.u8(env.src_arg0_rgb)
	f.u8(env.op_arg0_rgb)
	f.u8(env.src_arg1_rgb)
	f.u8(env.op_arg1_rgb)
	if version >= 1:
		f.u8(env.src_arg2_rgb or 0)
		f.u8(env.op_arg2_rgb or 0)
	f.u8(env.op_alpha)
	f.u8(env.src_arg0_alpha)
	f.u8(env.op_arg0_alpha)
	f.u8(env.src_arg1_alpha)
	f.u8(env.op_arg1_alpha)
	if version >= 1:
		f.u8(env.src_arg2_alpha or 0)
		f.u8(env.op_arg2_alpha or 0)
	f.rgba(env.constant_color)


def _write_material_light_map(f: _Writer, lm: MaterialLightMap) -> None:
	f.version(1)
	f.rgba(lm.factor)
	f.rgba(lm.diffuse)
	f.rgba(lm.ambient if lm.ambient is not None else Rgba(0, 0, 0, 0))
	_write_texture_ptr(f, lm.texture)


def _write_material(f: _Writer, mat: Material) -> None:
	"""Always writes the latest CMaterial format (version 9)."""
	f.version(9)
	f.s32(mat.shader_type)
	f.u32(mat.flags)
	f.s32(mat.src_blend)
	f.s32(mat.dst_blend)
	f.s32(mat.z_function)
	f.f32(mat.z_bias)
	f.rgba(mat.color)
	f.rgba(mat.emissive)
	f.rgba(mat.ambient)
	f.rgba(mat.diffuse)
	f.rgba(mat.specular)
	f.f32(mat.shininess)
	f.f32(mat.alpha_test_threshold)
	f.u16(mat.tex_coord_gen_mode)

	textures = (list(mat.textures) + [None] * IDRV_MAT_MAXTEXTURES)[:IDRV_MAT_MAXTEXTURES]
	tex_envs = (list(mat.tex_envs) + [None] * IDRV_MAT_MAXTEXTURES)[:IDRV_MAT_MAXTEXTURES]
	for i in range(IDRV_MAT_MAXTEXTURES):
		_write_texture_ptr(f, textures[i])
		_write_tex_env(f, tex_envs[i], version=1)

	f.u32(len(mat.light_maps))
	for lm in mat.light_maps:
		_write_material_light_map(f, lm)
	f.boolean(bool(mat.light_maps_mulx2))

	if mat.flags & IDRV_MAT_TEX_ADDR:
		tam = (mat.tex_addr_mode or [0] * IDRV_MAT_MAXTEXTURES)
		for i in range(IDRV_MAT_MAXTEXTURES):
			f.u8(tam[i] if i < len(tam) else 0)

	for i in range(IDRV_MAT_MAXTEXTURES):
		if mat.flags & (0x00100000 << i):
			user_mat = mat.tex_user_mat.get(i)
			if user_mat is None:
				raise ShapeWriteError(
					f"material flags enable a user texture matrix for stage {i}, "
					f"but no matrix was captured for it"
				)
			f.matrix(user_mat)


# ---------------------------------------------------------------------------
# CMaterialBase (animated materials) and CTexAnimTracks
# ---------------------------------------------------------------------------


@dataclass
class MaterialBase:
	name: str
	default_ambient: Rgba
	default_diffuse: Rgba
	default_specular: Rgba
	default_shininess: float
	default_emissive: Rgba
	default_opacity: float
	default_texture: int
	animated_textures: Dict[int, Optional[Texture]] = field(default_factory=dict)
	# Per texture-stage (UTrans, VTrans, UScale, VScale); DefaultWRot is never
	# actually serialized by the engine (a real quirk of CTexAnimTracks::serial).
	tex_anim_tracks: Dict[int, Tuple[float, float, float, float]] = field(default_factory=dict)


def _parse_material_base(f: _Reader) -> MaterialBase:
	ver = f.version()
	name = f.string()
	default_ambient = f.track_default_rgba()
	default_diffuse = f.track_default_rgba()
	default_specular = f.track_default_rgba()
	default_shininess = f.track_default_float()
	default_emissive = f.track_default_rgba()
	default_opacity = f.track_default_float()
	default_texture = f.track_default_int()

	animated_textures: Dict[int, Optional[Texture]] = {}
	n = f.cont_len()  # _AnimatedTextures map<uint32, poly-ptr ITexture>
	for _ in range(n):
		key = f.u32()
		animated_textures[key] = _read_texture_ptr(f)

	tex_anim_tracks: Dict[int, Tuple[float, float, float, float]] = {}
	if ver > 0:
		for i in range(IDRV_MAT_MAXTEXTURES):
			f.version()  # CTexAnimTracks version
			# NB: engine bug/quirk -- only 4 of the 5 declared tracks are
			# actually serialized (DefaultWRot is skipped).
			u_trans = f.track_default_float()
			v_trans = f.track_default_float()
			u_scale = f.track_default_float()
			v_scale = f.track_default_float()
			tex_anim_tracks[i] = (u_trans, v_trans, u_scale, v_scale)

	return MaterialBase(
		name=name,
		default_ambient=default_ambient,
		default_diffuse=default_diffuse,
		default_specular=default_specular,
		default_shininess=default_shininess,
		default_emissive=default_emissive,
		default_opacity=default_opacity,
		default_texture=default_texture,
		animated_textures=animated_textures,
		tex_anim_tracks=tex_anim_tracks,
	)


def _write_material_base(f: _Writer, mb: MaterialBase) -> None:
	"""Always writes the latest CMaterialBase format (version 1)."""
	f.version(1)
	f.string(mb.name)
	f.track_default_rgba(mb.default_ambient)
	f.track_default_rgba(mb.default_diffuse)
	f.track_default_rgba(mb.default_specular)
	f.track_default_float(mb.default_shininess)
	f.track_default_rgba(mb.default_emissive)
	f.track_default_float(mb.default_opacity)
	f.track_default_int(mb.default_texture)

	f.cont_len(len(mb.animated_textures))
	for key, tex in mb.animated_textures.items():
		f.u32(key)
		_write_texture_ptr(f, tex)

	for i in range(IDRV_MAT_MAXTEXTURES):
		f.version(0)  # CTexAnimTracks version
		u_trans, v_trans, u_scale, v_scale = mb.tex_anim_tracks.get(i, (0.0, 0.0, 1.0, 1.0))
		f.track_default_float(u_trans)
		f.track_default_float(v_trans)
		f.track_default_float(u_scale)
		f.track_default_float(v_scale)


# ---------------------------------------------------------------------------
# CVertexBuffer
# ---------------------------------------------------------------------------

_VALUE_NAMES = [
	"Position", "Normal",
	"TexCoord0", "TexCoord1", "TexCoord2", "TexCoord3",
	"TexCoord4", "TexCoord5", "TexCoord6", "TexCoord7",
	"PrimaryColor", "SecondaryColor", "Weight", "PaletteSkin", "Fog", "Empty",
]
_NUM_VALUE = 16

# TType -> (struct format char, component count)
_TYPE_INFO = {
	0: ("d", 1), 1: ("f", 1), 2: ("h", 1),
	3: ("d", 2), 4: ("f", 2), 5: ("h", 2),
	6: ("d", 3), 7: ("f", 3), 8: ("h", 3),
	9: ("d", 4), 10: ("f", 4), 11: ("h", 4),
	12: ("B", 4),
}


@dataclass
class VertexBuffer:
	name: str
	num_verts: int
	vertex_color_format: int
	channels: Dict[str, List[Any]] = field(default_factory=dict)
	types: List[int] = field(default_factory=lambda: [0] * _NUM_VALUE)  # TType per channel slot

	@property
	def positions(self) -> Optional[List[Tuple[float, ...]]]:
		return self.channels.get("Position")

	@property
	def normals(self) -> Optional[List[Tuple[float, ...]]]:
		return self.channels.get("Normal")


def _decode_channel_value(f: _Reader, ttype: int, channel_name: str, vertex_color_format: int):
	fmt, count = _TYPE_INFO[ttype]
	size = struct.calcsize("<" + fmt * count)
	values = struct.unpack("<" + fmt * count, f._take(size))
	if ttype == 12 and channel_name in ("PrimaryColor", "SecondaryColor"):
		if vertex_color_format == 1:  # TBGRA
			b, g, r, a = values
		else:  # TRGBA
			r, g, b, a = values
		return Rgba(r, g, b, a)
	return values


def _parse_vertex_buffer_header(f: _Reader) -> Tuple[int, List[int], int, int, VertexBuffer]:
	"""Returns (flags, types, preferred_memory, num_verts, VertexBuffer-with-empty-channels)."""
	ver = f.version()

	if ver < 1:
		raise ShapeParseError("legacy (pre-1) CVertexBuffer header format is not supported")

	flags = f.u16()
	types = []
	for i in range(_NUM_VALUE):
		t = f.u8()
		types.append(t)

	num_verts = f.u32()
	vertex_color_format = f.u8() if ver >= 2 else 0
	name = ""
	if ver >= 3:
		f.s32()  # _PreferredMemory (serialEnum)
		name = f.string()

	vb = VertexBuffer(name=name, num_verts=num_verts, vertex_color_format=vertex_color_format, types=types)
	for i in range(_NUM_VALUE):
		if flags & (1 << i):
			vb.channels[_VALUE_NAMES[i]] = []
	return flags, types, vertex_color_format, num_verts, vb


def _parse_vertex_buffer_subset(f: _Reader, flags: int, types: List[int], vertex_color_format: int, vb: VertexBuffer, start: int, end: int) -> None:
	ver = f.version()
	active = [i for i in range(_NUM_VALUE) if flags & (1 << i)]
	for _vid in range(start, end):
		for i in active:
			value = _decode_channel_value(f, types[i], _VALUE_NAMES[i], vertex_color_format)
			vb.channels[_VALUE_NAMES[i]].append(value)
	if ver >= 2:
		f._take(8)  # _UVRouting: 8 raw uint8


def _parse_vertex_buffer(f: _Reader) -> VertexBuffer:
	"""Full CVertexBuffer::serial (header + all vertex data in one shot)."""
	ver = f.version()
	if ver < 2:
		raise ShapeParseError("legacy (pre-2) CVertexBuffer format is not supported")
	flags, types, vertex_color_format, num_verts, vb = _parse_vertex_buffer_header(f)
	_parse_vertex_buffer_subset(f, flags, types, vertex_color_format, vb, 0, num_verts)
	return vb


def _encode_channel_value(f: _Writer, ttype: int, channel_name: str, vertex_color_format: int, value) -> None:
	fmt, count = _TYPE_INFO[ttype]
	if ttype == 12 and channel_name in ("PrimaryColor", "SecondaryColor"):
		c = value
		if vertex_color_format == 1:  # TBGRA
			packed = (c.b, c.g, c.r, c.a)
		else:  # TRGBA
			packed = (c.r, c.g, c.b, c.a)
		f.raw(struct.pack("<" + fmt * count, *packed))
	else:
		f.raw(struct.pack("<" + fmt * count, *value))


def _write_vertex_buffer_header(f: _Writer, vb: VertexBuffer) -> int:
	"""Writes the header; returns the `flags` bitmask needed by the subset writer."""
	f.version(3)
	flags = 0
	for i in range(_NUM_VALUE):
		if _VALUE_NAMES[i] in vb.channels:
			flags |= (1 << i)
	f.u16(flags)
	for i in range(_NUM_VALUE):
		f.u8(vb.types[i])
	f.u32(vb.num_verts)
	f.u8(vb.vertex_color_format)
	f.s32(0)  # _PreferredMemory
	f.string(vb.name)
	return flags


def _write_vertex_buffer_subset(f: _Writer, flags: int, types: List[int], vertex_color_format: int, vb: VertexBuffer, start: int, end: int) -> None:
	f.version(2)
	active = [i for i in range(_NUM_VALUE) if flags & (1 << i)]
	for vid in range(start, end):
		for i in active:
			value = vb.channels[_VALUE_NAMES[i]][vid]
			_encode_channel_value(f, types[i], _VALUE_NAMES[i], vertex_color_format, value)
	f.raw(bytes(8))  # _UVRouting


def _write_vertex_buffer(f: _Writer, vb: VertexBuffer) -> None:
	"""Full CVertexBuffer::serial (header + all vertex data in one shot)."""
	f.version(2)
	flags = _write_vertex_buffer_header(f, vb)
	_write_vertex_buffer_subset(f, flags, vb.types, vb.vertex_color_format, vb, 0, vb.num_verts)


# ---------------------------------------------------------------------------
# CIndexBuffer / CRdrPass / CMatrixBlock
# ---------------------------------------------------------------------------


def _parse_index_buffer(f: _Reader) -> List[int]:
	ver = f.version()
	if ver < 1:
		raise ShapeParseError("legacy (pre-1) CIndexBuffer (primitive block) format is not supported")
	nb_indexes = f.u32()
	f.u32()  # _Capacity
	indexes = f.cont_uint_vector(4, "I")
	f.s32()  # _PreferredMemory (serialEnum)
	if ver == 1:
		for _ in range(3):  # PreferredCount legacy bools
			f.boolean()
	return indexes[:nb_indexes]


@dataclass
class RdrPass:
	material_id: int
	indices: List[int]


def _parse_rdr_pass(f: _Reader) -> RdrPass:
	f.version()
	material_id = f.u32()
	indices = _parse_index_buffer(f)
	return RdrPass(material_id=material_id, indices=indices)


@dataclass
class MatrixBlock:
	matrix_id: List[int]
	num_matrix: int
	rdr_passes: List[RdrPass]


def _parse_matrix_block(f: _Reader) -> MatrixBlock:
	f.version()
	matrix_id = [f.u32() for _ in range(16)]
	num_matrix = f.u32()
	n = f.cont_len()
	rdr_passes = [_parse_rdr_pass(f) for _ in range(n)]
	return MatrixBlock(matrix_id=matrix_id, num_matrix=num_matrix, rdr_passes=rdr_passes)


def _write_index_buffer(f: _Writer, indices: List[int]) -> None:
	f.version(2)
	f.u32(len(indices))  # _NbIndexes
	f.u32(len(indices))  # _Capacity
	f.cont_uint_vector(indices, 4, "I")
	f.s32(0)  # _PreferredMemory


def _write_rdr_pass(f: _Writer, rp: RdrPass) -> None:
	f.version(0)
	f.u32(rp.material_id)
	_write_index_buffer(f, rp.indices)


def _write_matrix_block(f: _Writer, mb: MatrixBlock) -> None:
	f.version(0)
	matrix_id = (list(mb.matrix_id) + [0] * 16)[:16]
	for v in matrix_id:
		f.u32(v)
	f.u32(mb.num_matrix)
	f.cont_len(len(mb.rdr_passes))
	for rp in mb.rdr_passes:
		_write_rdr_pass(f, rp)


# ---------------------------------------------------------------------------
# CMeshMorpher / CBlendShape
# ---------------------------------------------------------------------------


@dataclass
class BlendShape:
	name: str
	delta_pos: List[Vector3]
	delta_norm: List[Vector3]
	delta_uv: List[Tuple[float, float]]
	delta_col: List[Rgba]
	delta_tg_space: List[Vector3]
	vert_refs: List[int]


@dataclass
class MeshMorpher:
	blend_shapes: List[BlendShape]


def _parse_blend_shape(f: _Reader) -> BlendShape:
	ver = f.version()
	name = f.string()
	n = f.cont_len()
	delta_pos = [f.vector3() for _ in range(n)]
	n = f.cont_len()
	delta_norm = [f.vector3() for _ in range(n)]
	n = f.cont_len()
	delta_uv = [(f.f32(), f.f32()) for _ in range(n)]
	n = f.cont_len()
	delta_col = [f.rgba() for _ in range(n)]
	delta_tg_space: List[Vector3] = []
	if ver >= 1:
		n = f.cont_len()
		delta_tg_space = [f.vector3() for _ in range(n)]
	vert_refs = f.cont_uint_vector(4, "I")
	return BlendShape(
		name=name,
		delta_pos=delta_pos,
		delta_norm=delta_norm,
		delta_uv=delta_uv,
		delta_col=delta_col,
		delta_tg_space=delta_tg_space,
		vert_refs=vert_refs,
	)


def _parse_mesh_morpher(f: _Reader) -> MeshMorpher:
	f.version()
	n = f.cont_len()
	return MeshMorpher(blend_shapes=[_parse_blend_shape(f) for _ in range(n)])


def _write_blend_shape(f: _Writer, bs: BlendShape) -> None:
	f.version(1)
	f.string(bs.name)
	f.cont_len(len(bs.delta_pos))
	for v in bs.delta_pos:
		f.vector3(v)
	f.cont_len(len(bs.delta_norm))
	for v in bs.delta_norm:
		f.vector3(v)
	f.cont_len(len(bs.delta_uv))
	for u, v in bs.delta_uv:
		f.f32(u)
		f.f32(v)
	f.cont_len(len(bs.delta_col))
	for c in bs.delta_col:
		f.rgba(c)
	f.cont_len(len(bs.delta_tg_space))
	for v in bs.delta_tg_space:
		f.vector3(v)
	f.cont_uint_vector(bs.vert_refs, 4, "I")


def _write_mesh_morpher(f: _Writer, mm: Optional[MeshMorpher]) -> None:
	f.version(0)
	blend_shapes = mm.blend_shapes if mm else []
	f.cont_len(len(blend_shapes))
	for bs in blend_shapes:
		_write_blend_shape(f, bs)


# ---------------------------------------------------------------------------
# CLodCharacterTexture (distant-LOD baked character texture, rare)
# ---------------------------------------------------------------------------


@dataclass
class LodCharacterTexture:
	width: int
	height: int
	texture: List[Tuple[int, int, int, int]]  # per-pixel (T, U, V, Q)


def _parse_lod_character_texture(f: _Reader) -> LodCharacterTexture:
	f.version()
	width = f.u32()
	height = f.u32()
	n = f.cont_len()
	texture = [(f.u8(), f.u8(), f.u8(), f.u8()) for _ in range(n)]
	return LodCharacterTexture(width=width, height=height, texture=texture)


def _read_lod_character_texture_ptr(f: _Reader) -> Optional[LodCharacterTexture]:
	"""serialPtr(): statically-typed pointer, no class name on disk."""
	node = f.u64()
	if node == 0:
		return None
	if node in f.seen:
		return f.seen[node]
	obj = _parse_lod_character_texture(f)
	f.seen[node] = obj
	return obj


def _write_lod_character_texture_ptr(f: _Writer, lct: Optional[LodCharacterTexture]) -> None:
	"""serialPtr(): statically-typed pointer, no class name on disk."""
	if lct is None:
		f.u64(0)
		return
	key = id(lct)
	existing = f._written_ids.get(key)
	if existing is not None:
		f.u64(existing)
		return
	node = f._next_id
	f._next_id += 1
	f._written_ids[key] = node
	f.u64(node)
	f.version(0)
	f.u32(lct.width)
	f.u32(lct.height)
	f.cont_len(len(lct.texture))
	for t, u, v, q in lct.texture:
		f.u8(t)
		f.u8(u)
		f.u8(v)
		f.u8(q)


# ---------------------------------------------------------------------------
# CMeshBase (shared by CMesh, CMeshMultiLod)
# ---------------------------------------------------------------------------


@dataclass
class LightMapInfoList:
	light_group: int
	animated_light: str
	stage_list: List[Tuple[int, int]] = field(default_factory=list)  # (mat_id, stage_id)


@dataclass
class MeshBase:
	materials: List[Material]
	default_pos: Vector3
	default_pivot: Vector3
	default_rot_euler: Vector3
	default_rot_quat: Quaternion
	default_scale: Vector3
	is_lightable: Optional[bool] = None
	use_lighting_local_attenuation: Optional[bool] = None
	auto_anim: Optional[bool] = None
	dist_max: Optional[float] = None
	lod_character_texture: Optional[LodCharacterTexture] = None
	animated_morph_names: List[str] = field(default_factory=list)
	animated_materials: Dict[int, MaterialBase] = field(default_factory=dict)
	light_infos: List[LightMapInfoList] = field(default_factory=list)
	collision_mesh_generation: Optional[int] = None


def _parse_mesh_base(f: _Reader) -> MeshBase:
	ver = f.version()
	if ver < 1:
		raise ShapeParseError("legacy (pre-1) CMeshBase format is not supported")

	animated_morph_names: List[str] = []
	if ver >= 2:
		n = f.cont_len()  # _AnimatedMorph: vector<CMorphBase>, each just a Name string
		animated_morph_names = [f.string() for _ in range(n)]

	default_pos = f.track_default_vector()
	default_pivot = f.track_default_vector()
	default_rot_euler = f.track_default_vector()
	default_rot_quat = f.track_default_quat()
	default_scale = f.track_default_vector()

	n = f.cont_len()
	materials = [_parse_material(f) for _ in range(n)]

	animated_materials: Dict[int, MaterialBase] = {}
	n = f.cont_len()  # _AnimatedMaterials: map<uint32, CMaterialBase>
	for _ in range(n):
		key = f.u32()
		animated_materials[key] = _parse_material_base(f)

	light_infos: List[LightMapInfoList] = []
	if ver >= 8:
		n = f.cont_len()  # _LightInfos: vector<CLightMapInfoList>
		for _ in range(n):
			f.version()
			light_group = f.u32()
			animated_light = f.string()
			m = f.cont_len()  # StageList: vector<CMatStage>
			stage_list = []
			for _ in range(m):
				f.version()
				mat_id = f.u32()
				stage_id = f.u32()
				stage_list.append((mat_id, stage_id))
			light_infos.append(LightMapInfoList(light_group=light_group, animated_light=animated_light, stage_list=stage_list))
	else:
		n = f.cont_len()  # old TLightInfoMapV7, discarded
		for _ in range(n):
			raise ShapeParseError("legacy (pre-8) CMeshBase _LightInfos format is not supported")

	is_lightable = f.boolean() if ver >= 3 else None
	use_lighting_local_attenuation = f.boolean() if ver >= 4 else None
	auto_anim = f.boolean() if ver >= 5 else None
	dist_max = f.f32() if ver >= 6 else None
	lod_character_texture = _read_lod_character_texture_ptr(f) if ver >= 7 else None
	collision_mesh_generation = f.s32() if ver >= 9 else None

	return MeshBase(
		materials=materials,
		default_pos=default_pos,
		default_pivot=default_pivot,
		default_rot_euler=default_rot_euler,
		default_rot_quat=default_rot_quat,
		default_scale=default_scale,
		is_lightable=is_lightable,
		use_lighting_local_attenuation=use_lighting_local_attenuation,
		auto_anim=auto_anim,
		dist_max=dist_max,
		lod_character_texture=lod_character_texture,
		animated_morph_names=animated_morph_names,
		animated_materials=animated_materials,
		light_infos=light_infos,
		collision_mesh_generation=collision_mesh_generation,
	)


def _write_mesh_base(f: _Writer, base: MeshBase) -> None:
	"""Always writes the latest CMeshBase format (version 10)."""
	f.version(10)
	f.string_vector(base.animated_morph_names)

	f.track_default_vector(base.default_pos)
	f.track_default_vector(base.default_pivot)
	f.track_default_vector(base.default_rot_euler)
	f.track_default_quat(base.default_rot_quat)
	f.track_default_vector(base.default_scale)

	f.cont_len(len(base.materials))
	for mat in base.materials:
		_write_material(f, mat)

	f.cont_len(len(base.animated_materials))
	for key, mb in base.animated_materials.items():
		f.u32(key)
		_write_material_base(f, mb)

	f.cont_len(len(base.light_infos))
	for li in base.light_infos:
		f.version(0)
		f.u32(li.light_group)
		f.string(li.animated_light)
		f.cont_len(len(li.stage_list))
		for mat_id, stage_id in li.stage_list:
			f.version(0)
			f.u32(mat_id)
			f.u32(stage_id)

	f.boolean(base.is_lightable if base.is_lightable is not None else True)
	f.boolean(bool(base.use_lighting_local_attenuation))
	f.boolean(bool(base.auto_anim))
	f.f32(base.dist_max if base.dist_max is not None else -1.0)
	_write_lod_character_texture_ptr(f, base.lod_character_texture)
	f.s32(base.collision_mesh_generation if base.collision_mesh_generation is not None else 0)


# ---------------------------------------------------------------------------
# IMeshVertexProgram (CMeshVPWindTree, CMeshVPPerPixelLight)
# ---------------------------------------------------------------------------


@dataclass
class WindTreeParams:
	frequency: Tuple[float, float, float]
	frequency_wind_factor: Tuple[float, float, float]
	power_xy: Tuple[float, float, float]
	power_z: Tuple[float, float, float]
	bias: Tuple[float, float, float]
	specular_lighting: bool


def _parse_mesh_vp_wind_tree(f: _Reader) -> WindTreeParams:
	f.version()
	frequency = []
	frequency_wind_factor = []
	power_xy = []
	power_z = []
	bias = []
	for _ in range(3):  # HrcDepth
		frequency.append(f.f32())
		frequency_wind_factor.append(f.f32())
		power_xy.append(f.f32())
		power_z.append(f.f32())
		bias.append(f.f32())
	specular_lighting = f.boolean()
	return WindTreeParams(
		frequency=tuple(frequency), frequency_wind_factor=tuple(frequency_wind_factor),
		power_xy=tuple(power_xy), power_z=tuple(power_z), bias=tuple(bias),
		specular_lighting=specular_lighting,
	)


_CLASS_PARSERS["CMeshVPWindTree"] = _parse_mesh_vp_wind_tree


@dataclass
class PerPixelLightParams:
	specular_lighting: bool


def _parse_mesh_vp_per_pixel_light(f: _Reader) -> PerPixelLightParams:
	f.version()
	return PerPixelLightParams(specular_lighting=f.boolean())


_CLASS_PARSERS["CMeshVPPerPixelLight"] = _parse_mesh_vp_per_pixel_light


def _write_mesh_vp_wind_tree(f: _Writer, params: WindTreeParams) -> None:
	f.version(0)
	for i in range(3):
		f.f32(params.frequency[i])
		f.f32(params.frequency_wind_factor[i])
		f.f32(params.power_xy[i])
		f.f32(params.power_z[i])
		f.f32(params.bias[i])
	f.boolean(params.specular_lighting)


def _write_mesh_vp_per_pixel_light(f: _Writer, params: PerPixelLightParams) -> None:
	f.version(0)
	f.boolean(params.specular_lighting)


_VERTEX_PROGRAM_WRITERS = {
	WindTreeParams: ("CMeshVPWindTree", _write_mesh_vp_wind_tree),
	PerPixelLightParams: ("CMeshVPPerPixelLight", _write_mesh_vp_per_pixel_light),
}


def _write_vertex_program_ptr(f: _Writer, vp) -> None:
	if vp is None:
		f.u64(0)
		return
	entry = _VERTEX_PROGRAM_WRITERS.get(type(vp))
	if entry is None:
		raise ShapeWriteError(f"cannot write unsupported mesh vertex program {type(vp).__name__}")
	class_name, writer = entry
	f.write_poly_ptr(vp, class_name, writer)


# ---------------------------------------------------------------------------
# CMeshGeom / CMesh
# ---------------------------------------------------------------------------


@dataclass
class MeshGeom:
	bones_name: List[str]
	mesh_morpher: Optional[MeshMorpher]
	vertex_buffer: VertexBuffer
	matrix_blocks: List[MatrixBlock]
	bbox: AABBox
	skinned: bool
	vertex_program: Union[WindTreeParams, PerPixelLightParams, None] = None

	@property
	def num_vertices(self) -> int:
		return self.vertex_buffer.num_verts

	@property
	def num_triangles(self) -> int:
		return sum(len(rp.indices) // 3 for mb in self.matrix_blocks for rp in mb.rdr_passes)


def _parse_mesh_geom(f: _Reader) -> MeshGeom:
	ver = f.version()
	bones_name = f.string_vector() if ver >= 4 else []
	vertex_program = _read_poly_ptr(f) if ver >= 3 else None
	mesh_morpher = _parse_mesh_morpher(f) if ver >= 1 else None
	vbuffer = _parse_vertex_buffer(f)
	n = f.cont_len()
	matrix_blocks = [_parse_matrix_block(f) for _ in range(n)]
	bbox = f.aabbox()
	skinned = f.boolean()
	return MeshGeom(
		bones_name=bones_name,
		mesh_morpher=mesh_morpher,
		vertex_buffer=vbuffer,
		matrix_blocks=matrix_blocks,
		bbox=bbox,
		skinned=skinned,
		vertex_program=vertex_program,
	)


_CLASS_PARSERS["CMeshGeom"] = _parse_mesh_geom


def _write_mesh_geom(f: _Writer, geom: MeshGeom) -> None:
	"""Always writes the latest CMeshGeom format (version 4)."""
	f.version(4)
	f.string_vector(geom.bones_name)
	_write_vertex_program_ptr(f, geom.vertex_program)
	_write_mesh_morpher(f, geom.mesh_morpher)
	_write_vertex_buffer(f, geom.vertex_buffer)
	f.cont_len(len(geom.matrix_blocks))
	for mb in geom.matrix_blocks:
		_write_matrix_block(f, mb)
	f.aabbox(geom.bbox)
	f.boolean(geom.skinned)


@dataclass
class Mesh:
	base: MeshBase
	geom: MeshGeom

	@property
	def materials(self) -> List[Material]:
		return self.base.materials

	@property
	def bbox(self) -> AABBox:
		return self.geom.bbox

	@property
	def num_vertices(self) -> int:
		return self.geom.num_vertices

	@property
	def num_triangles(self) -> int:
		return self.geom.num_triangles


def _parse_mesh(f: _Reader) -> Mesh:
	ver = f.version()
	if ver < 6:
		raise ShapeParseError("legacy (pre-6) CMesh format is not supported")
	base = _parse_mesh_base(f)
	geom = _parse_mesh_geom(f)
	return Mesh(base=base, geom=geom)


def _write_mesh(f: _Writer, mesh: Mesh) -> None:
	"""Always writes the latest CMesh format (version 6)."""
	f.version(6)
	_write_mesh_base(f, mesh.base)
	_write_mesh_geom(f, mesh.geom)


_CLASS_PARSERS["CMesh"] = _parse_mesh


# ---------------------------------------------------------------------------
# CMeshMRM / CMeshMRMGeom
# ---------------------------------------------------------------------------


@dataclass
class MrmLevelDetail:
	max_face_used: int
	min_face_used: int
	distance_finest: float
	distance_middle: float
	distance_coarsest: float
	oo_distance_delta: float
	distance_pow: float


@dataclass
class MrmLod:
	n_wedges: int
	rdr_passes: List[RdrPass]

	@property
	def num_triangles(self) -> int:
		return sum(len(rp.indices) // 3 for rp in self.rdr_passes)


@dataclass
class MeshMRMGeom:
	bones_name: List[str]
	mesh_morpher: Optional[MeshMorpher]
	skinned: bool
	bbox: AABBox
	level_detail: MrmLevelDetail
	num_wedges: int
	vertex_buffer: VertexBuffer
	skin_weights: List[Tuple[Tuple[int, int, int, int], Tuple[float, float, float, float]]]
	lods: List[MrmLod]
	vertex_program: Union[WindTreeParams, PerPixelLightParams, None] = None

	@property
	def num_vertices(self) -> int:
		return self.num_wedges

	@property
	def num_triangles(self) -> int:
		return self.lods[-1].num_triangles if self.lods else 0


def _parse_skin_weight(f: _Reader):
	matrix_id = tuple(f.u32() for _ in range(4))
	weight = tuple(f.f32() for _ in range(4))
	return (matrix_id, weight)


def _skip_shadow_skin(f: _Reader) -> None:
	"""CShadowSkin: a simplified shadow-casting proxy mesh, not real content -- skip it."""
	n = f.cont_len()  # Vertices: vector<CShadowVertex>, each version(0)+CVector(12)+MatrixId(4)=17 bytes
	for _ in range(n):
		f.version()
		f.vector3()
		f.u32()
	f.cont_uint_vector(4, "I")  # Triangles


def _parse_mesh_mrm_geom(f: _Reader) -> MeshMRMGeom:
	ver = f.version()
	if ver < 5:
		raise ShapeParseError("legacy (pre-5) CMeshMRMGeom format is not supported")

	bones_name = f.string_vector() if ver >= 3 else []
	vertex_program = _read_poly_ptr(f) if ver >= 2 else None
	mesh_morpher = _parse_mesh_morpher(f) if ver >= 1 else None

	skinned = f.boolean()
	bbox = f.aabbox()
	max_face_used = f.u32()
	min_face_used = f.u32()
	distance_finest = f.f32()
	distance_middle = f.f32()
	distance_coarsest = f.f32()
	oo_distance_delta = f.f32()
	distance_pow = f.f32()
	level_detail = MrmLevelDetail(
		max_face_used, min_face_used, distance_finest, distance_middle,
		distance_coarsest, oo_distance_delta, distance_pow,
	)

	n_lods = f.cont_len()
	lod_infos = []
	for _ in range(n_lods):
		f.version()
		start_add_wedge = f.u32()
		end_add_wedges = f.u32()
		lod_infos.append((start_add_wedge, end_add_wedges))

	num_wedges = f.u32()
	flags, types, vertex_color_format, hdr_num_verts, vbuffer = _parse_vertex_buffer_header(f)

	skin_weights: List[Any] = []
	if ver >= 4:
		n = f.cont_len()
		skin_weights = [_parse_skin_weight(f) for _ in range(n)]

	_skip_shadow_skin(f)

	# Lod offset table: relative sint32 offsets, only used for seeking; read & ignore
	for _ in range(n_lods):
		f.s32()

	lods: List[MrmLod] = []
	for i in range(n_lods):
		start_wedge, end_wedge = lod_infos[i]
		lod_ver = f.version()  # CLod version
		n_wedges_lod = f.u32()
		m = f.cont_len()
		rdr_passes = [_parse_rdr_pass(f) for _ in range(m)]
		f.skip_pair_vector()  # Geomorphs: vector<CMRMWedgeGeom(Start,End)>
		f.cont_uint_vector(4, "I")  # MatrixInfluences
		for _ in range(4):  # InfluencedVertices[4]
			f.cont_uint_vector(4, "I")
		if lod_ver >= 1:
			f.skip_pair_vector()  # SkinVertexBlocks: vector<(VertexStart,NVertices)>

		# vertex data for this lod's wedge range
		f.version()  # serialLodVertexData version
		_parse_vertex_buffer_subset(f, flags, types, vertex_color_format, vbuffer, start_wedge, end_wedge)

		lods.append(MrmLod(n_wedges=n_wedges_lod, rdr_passes=rdr_passes))

	return MeshMRMGeom(
		bones_name=bones_name,
		mesh_morpher=mesh_morpher,
		skinned=skinned,
		bbox=bbox,
		level_detail=level_detail,
		num_wedges=num_wedges,
		vertex_buffer=vbuffer,
		skin_weights=skin_weights,
		lods=lods,
		vertex_program=vertex_program,
	)


_CLASS_PARSERS["CMeshMRMGeom"] = _parse_mesh_mrm_geom


@dataclass
class MeshMRM:
	base: MeshBase
	geom: MeshMRMGeom
	# Raw bytes of everything after CMeshBase, exactly as read: CMeshMRM's
	# progressive multi-resolution geometry is not reconstructed field-by-field
	# on write (that belongs to the 3D export pipeline), it is copied verbatim.
	_version: int = 0
	_raw_geom: bytes = b""

	@property
	def materials(self) -> List[Material]:
		return self.base.materials

	@property
	def bbox(self) -> AABBox:
		return self.geom.bbox

	@property
	def num_vertices(self) -> int:
		return self.geom.num_vertices

	@property
	def num_triangles(self) -> int:
		return self.geom.num_triangles


def _parse_mesh_mrm(f: _Reader) -> MeshMRM:
	ver = f.version()
	base = _parse_mesh_base(f)
	geom_start = f.tell()
	geom = _parse_mesh_mrm_geom(f)
	raw_geom = f._data[geom_start:f.tell()]
	return MeshMRM(base=base, geom=geom, _version=ver, _raw_geom=raw_geom)


_CLASS_PARSERS["CMeshMRM"] = _parse_mesh_mrm


# ---------------------------------------------------------------------------
# CMeshMRMSkinned / CMeshMRMSkinnedGeom
# ---------------------------------------------------------------------------

_MRM_SKINNED_UV_FACTOR = 8192.0
_MRM_SKINNED_NORMAL_FACTOR = 32767.0


@dataclass
class PackedVertex:
	pos: Tuple[int, int, int]
	normal: Tuple[int, int, int]
	uv: Tuple[int, int]
	matrices: Tuple[int, int, int, int]
	weights: Tuple[int, int, int, int]

	def decompact_pos(self, decompact_scale: float) -> Tuple[float, float, float]:
		return tuple(c * decompact_scale for c in self.pos)

	def decompact_normal(self) -> Tuple[float, float, float]:
		return tuple(c / _MRM_SKINNED_NORMAL_FACTOR for c in self.normal)

	def decompact_uv(self) -> Tuple[float, float]:
		return tuple(c / _MRM_SKINNED_UV_FACTOR for c in self.uv)


def _parse_packed_vertex(f: _Reader) -> PackedVertex:
	f.version()
	pos = (f.s16(), f.s16(), f.s16())
	normal = (f.s16(), f.s16(), f.s16())
	uv = (f.s16(), f.s16())
	matrices = []
	weights = []
	for _ in range(4):
		matrices.append(f.u8())
		weights.append(f.u8())
	return PackedVertex(pos=pos, normal=normal, uv=uv, matrices=tuple(matrices), weights=tuple(weights))


@dataclass
class MeshMRMSkinnedLod:
	n_wedges: int
	rdr_passes: List[RdrPass]

	@property
	def num_triangles(self) -> int:
		return sum(len(rp.indices) // 3 for rp in self.rdr_passes)


def _parse_mrm_skinned_rdr_pass(f: _Reader) -> RdrPass:
	f.version()
	material_id = f.u32()
	indices = f.cont_uint_vector(2, "H")
	return RdrPass(material_id=material_id, indices=indices)


def _parse_mesh_mrm_skinned_lod(f: _Reader) -> MeshMRMSkinnedLod:
	f.version()
	n_wedges = f.u32()
	m = f.cont_len()
	rdr_passes = [_parse_mrm_skinned_rdr_pass(f) for _ in range(m)]
	f.skip_pair_vector()  # Geomorphs
	f.cont_uint_vector(4, "I")  # MatrixInfluences
	for _ in range(4):  # InfluencedVertices[4]
		f.cont_uint_vector(4, "I")
	return MeshMRMSkinnedLod(n_wedges=n_wedges, rdr_passes=rdr_passes)


@dataclass
class MeshMRMSkinnedGeom:
	bones_name: List[str]
	bbox: AABBox
	level_detail: MrmLevelDetail
	packed_vertices: List[PackedVertex]
	decompact_scale: float
	lods: List[MeshMRMSkinnedLod]

	@property
	def num_vertices(self) -> int:
		return len(self.packed_vertices)

	@property
	def num_triangles(self) -> int:
		return self.lods[-1].num_triangles if self.lods else 0


def _parse_mesh_mrm_skinned_geom(f: _Reader) -> MeshMRMSkinnedGeom:
	f.version()
	bones_name = f.string_vector()
	bbox = f.aabbox()
	max_face_used = f.u32()
	min_face_used = f.u32()
	distance_finest = f.f32()
	distance_middle = f.f32()
	distance_coarsest = f.f32()
	oo_distance_delta = f.f32()
	distance_pow = f.f32()
	level_detail = MrmLevelDetail(
		max_face_used, min_face_used, distance_finest, distance_middle,
		distance_coarsest, oo_distance_delta, distance_pow,
	)

	f.version()  # CPackedVertexBuffer version
	n = f.cont_len()
	packed_vertices = [_parse_packed_vertex(f) for _ in range(n)]
	decompact_scale = f.f32()

	_skip_shadow_skin(f)

	n = f.cont_len()
	lods = [_parse_mesh_mrm_skinned_lod(f) for _ in range(n)]

	return MeshMRMSkinnedGeom(
		bones_name=bones_name,
		bbox=bbox,
		level_detail=level_detail,
		packed_vertices=packed_vertices,
		decompact_scale=decompact_scale,
		lods=lods,
	)


_CLASS_PARSERS["CMeshMRMSkinnedGeom"] = _parse_mesh_mrm_skinned_geom


@dataclass
class MeshMRMSkinned:
	base: MeshBase
	geom: MeshMRMSkinnedGeom
	_version: int = 0
	_raw_geom: bytes = b""

	@property
	def materials(self) -> List[Material]:
		return self.base.materials

	@property
	def bbox(self) -> AABBox:
		return self.geom.bbox

	@property
	def num_vertices(self) -> int:
		return self.geom.num_vertices

	@property
	def num_triangles(self) -> int:
		return self.geom.num_triangles


def _parse_mesh_mrm_skinned(f: _Reader) -> MeshMRMSkinned:
	ver = f.version()
	base = _parse_mesh_base(f)
	geom_start = f.tell()
	geom = _parse_mesh_mrm_skinned_geom(f)
	raw_geom = f._data[geom_start:f.tell()]
	return MeshMRMSkinned(base=base, geom=geom, _version=ver, _raw_geom=raw_geom)


_CLASS_PARSERS["CMeshMRMSkinned"] = _parse_mesh_mrm_skinned


# ---------------------------------------------------------------------------
# CMeshMultiLod
# ---------------------------------------------------------------------------


@dataclass
class MeshSlot:
	mesh_geom: Union[MeshGeom, MeshMRMGeom, None]
	a: float
	b: float
	dist_max: float
	end_polygon_count: float
	blend_length: float
	flags: int


def _parse_mesh_slot(f: _Reader) -> MeshSlot:
	f.version()
	mesh_geom = _read_poly_ptr(f)
	a = f.f32()
	b = f.f32()
	dist_max = f.f32()
	end_polygon_count = f.f32()
	blend_length = f.f32()
	flags = f.u8()
	return MeshSlot(
		mesh_geom=mesh_geom, a=a, b=b, dist_max=dist_max,
		end_polygon_count=end_polygon_count, blend_length=blend_length, flags=flags,
	)


@dataclass
class MeshMultiLod:
	base: MeshBase
	static_lod: bool
	slots: List[MeshSlot]
	_version: int = 0
	_raw_geom: bytes = b""

	@property
	def materials(self) -> List[Material]:
		return self.base.materials


def _parse_mesh_multi_lod(f: _Reader) -> MeshMultiLod:
	ver = f.version()
	base = _parse_mesh_base(f)
	geom_start = f.tell()
	static_lod = f.boolean()
	n = f.cont_len()
	slots = [_parse_mesh_slot(f) for _ in range(n)]
	raw_geom = f._data[geom_start:f.tell()]
	return MeshMultiLod(base=base, static_lod=static_lod, slots=slots, _version=ver, _raw_geom=raw_geom)


_CLASS_PARSERS["CMeshMultiLod"] = _parse_mesh_multi_lod


# ---------------------------------------------------------------------------
# CSkeletonShape
# ---------------------------------------------------------------------------


@dataclass
class Bone:
	name: str
	inv_bind_pos: Matrix
	father_id: int
	unherit_scale: bool
	lod_disable_distance: float
	default_pos: Vector3
	default_rot_euler: Vector3
	default_rot_quat: Quaternion
	default_scale: Vector3
	default_pivot: Vector3
	skin_scale: Optional[Vector3]


def _parse_bone(f: _Reader) -> Bone:
	ver = f.version()
	name = f.string()
	inv_bind_pos = f.matrix()
	father_id = f.s32()
	unherit_scale = f.boolean()
	lod_disable_distance = f.f32() if ver >= 1 else 0.0
	default_pos = f.track_default_vector()
	default_rot_euler = f.track_default_vector()
	default_rot_quat = f.track_default_quat()
	default_scale = f.track_default_vector()
	default_pivot = f.track_default_vector()
	skin_scale = f.vector3() if ver >= 2 else None
	return Bone(
		name=name, inv_bind_pos=inv_bind_pos, father_id=father_id,
		unherit_scale=unherit_scale, lod_disable_distance=lod_disable_distance,
		default_pos=default_pos, default_rot_euler=default_rot_euler,
		default_rot_quat=default_rot_quat, default_scale=default_scale,
		default_pivot=default_pivot, skin_scale=skin_scale,
	)


@dataclass
class SkeletonLod:
	distance: float
	active_bones: List[int]


def _parse_skeleton_lod(f: _Reader) -> SkeletonLod:
	f.version()
	distance = f.f32()
	active_bones = f.cont_uint_vector(1, "B")
	return SkeletonLod(distance=distance, active_bones=active_bones)


@dataclass
class SkeletonShape:
	bones: List[Bone]
	bone_map: Dict[str, int]
	lods: List[SkeletonLod]


def _parse_skeleton_shape(f: _Reader) -> SkeletonShape:
	ver = f.version()
	n = f.cont_len()
	bones = [_parse_bone(f) for _ in range(n)]
	n = f.cont_len()
	bone_map = {}
	for _ in range(n):
		key = f.string()
		bone_map[key] = f.u32()
	lods: List[SkeletonLod] = []
	if ver >= 1:
		n = f.cont_len()
		lods = [_parse_skeleton_lod(f) for _ in range(n)]
	return SkeletonShape(bones=bones, bone_map=bone_map, lods=lods)


_CLASS_PARSERS["CSkeletonShape"] = _parse_skeleton_shape


# ---------------------------------------------------------------------------
# CFlareShape
# ---------------------------------------------------------------------------

MAX_FLARE_NUM = 10


@dataclass
class FlareShape:
	color: Rgba
	persistence: float
	spacing: float
	attenuable: bool
	attenuation_range: Optional[float]
	first_flare_keep_size: bool
	textures: List[Optional[Texture]]
	sizes: List[float]
	positions: List[float]
	infinite_dist: bool
	max_view_dist: Optional[float]
	max_view_dist_ratio: Optional[float]
	dazzle_enabled: bool
	dazzle_color: Optional[Rgba]
	dazzle_attenuation_range: Optional[float]
	dist_max: Optional[float]
	occlusion_test_mesh_name: Optional[str] = None
	scale_when_disappear: Optional[bool] = None
	size_disappear: Optional[float] = None
	angle_disappear: Optional[float] = None
	occlusion_test_mesh_inherit_scale_rot: Optional[bool] = None
	look_at_mode: Optional[bool] = None


def _parse_flare_shape(f: _Reader) -> FlareShape:
	ver = f.version()
	color = f.rgba()
	persistence = f.f32()
	spacing = f.f32()
	attenuable = f.boolean()
	attenuation_range = f.f32() if attenuable else None
	first_flare_keep_size = f.boolean()  # always present; default overridden in-memory if ver<=4

	textures: List[Optional[Texture]] = []
	sizes: List[float] = []
	positions: List[float] = []
	for _ in range(MAX_FLARE_NUM):
		textures.append(_read_texture_ptr(f))
		sizes.append(f.f32())
		positions.append(f.f32())

	infinite_dist = f.boolean()
	max_view_dist = None
	max_view_dist_ratio = None
	if not infinite_dist:
		max_view_dist = f.f32()
		max_view_dist_ratio = f.f32()

	dazzle_enabled = f.boolean()
	dazzle_color = None
	dazzle_attenuation_range = None
	if dazzle_enabled:
		dazzle_color = f.rgba()
		dazzle_attenuation_range = f.f32()

	infinite_dist = f.boolean()  # NB: engine quirk, serialized twice; this value wins

	dist_max = f.f32() if ver >= 2 else None

	shape = FlareShape(
		color=color, persistence=persistence, spacing=spacing,
		attenuable=attenuable, attenuation_range=attenuation_range,
		first_flare_keep_size=first_flare_keep_size, textures=textures,
		sizes=sizes, positions=positions, infinite_dist=infinite_dist,
		max_view_dist=max_view_dist, max_view_dist_ratio=max_view_dist_ratio,
		dazzle_enabled=dazzle_enabled, dazzle_color=dazzle_color,
		dazzle_attenuation_range=dazzle_attenuation_range, dist_max=dist_max,
	)

	if ver >= 4:
		shape.occlusion_test_mesh_name = f.string()
		shape.scale_when_disappear = f.boolean()
		shape.size_disappear = f.f32()
		shape.angle_disappear = f.f32()
		shape.occlusion_test_mesh_inherit_scale_rot = f.boolean()
		shape.look_at_mode = f.boolean()

	return shape


_CLASS_PARSERS["CFlareShape"] = _parse_flare_shape


# ---------------------------------------------------------------------------
# CWaterShape / CWaveMakerShape
# ---------------------------------------------------------------------------


@dataclass
class WaterShape:
	polygon: List[Vector2]
	water_pool_id: int
	env_maps: List[Optional[Texture]]
	bump_maps: List[Optional[Texture]]
	color_map: Optional[Texture]
	height_map_scale: List[Vector2]
	height_map_speed: List[Vector2]
	color_map_mat_column0: Vector2
	color_map_mat_column1: Vector2
	color_map_mat_pos: Vector2
	default_pos: Vector3
	default_scale: Vector3
	default_rot_quat: Quaternion
	transition_ratio: float
	wave_height_factor: float
	compute_lightmap: Optional[bool] = None
	dist_max: Optional[float] = None
	splash_enabled: Optional[bool] = None
	uses_scene_water_env_map: Optional[Tuple[bool, bool]] = None


def _parse_water_shape(f: _Reader) -> WaterShape:
	ver = f.version()
	f.version()  # CPolygon2D version
	n = f.cont_len()
	polygon = [f.vector2() for _ in range(n)]

	water_pool_id = f.u32()

	env_maps = [_read_texture_ptr(f), _read_texture_ptr(f)]
	bump_maps = [_read_texture_ptr(f), _read_texture_ptr(f)]
	color_map = _read_texture_ptr(f)

	height_map_scale = [f.vector2(), f.vector2()]
	height_map_speed = [f.vector2(), f.vector2()]

	color_map_mat_column0 = f.vector2()
	color_map_mat_column1 = f.vector2()
	color_map_mat_pos = f.vector2()

	default_pos = f.track_default_vector()
	default_scale = f.track_default_vector()
	default_rot_quat = f.track_default_quat()

	transition_ratio = f.f32()
	wave_height_factor = f.f32()

	shape = WaterShape(
		polygon=polygon, water_pool_id=water_pool_id, env_maps=env_maps,
		bump_maps=bump_maps, color_map=color_map, height_map_scale=height_map_scale,
		height_map_speed=height_map_speed, color_map_mat_column0=color_map_mat_column0,
		color_map_mat_column1=color_map_mat_column1, color_map_mat_pos=color_map_mat_pos,
		default_pos=default_pos, default_scale=default_scale, default_rot_quat=default_rot_quat,
		transition_ratio=transition_ratio, wave_height_factor=wave_height_factor,
	)

	if ver >= 1:
		shape.compute_lightmap = f.boolean()
	if ver >= 2:
		shape.dist_max = f.f32()
	if ver >= 3:
		shape.splash_enabled = f.boolean()
	if ver >= 4:
		shape.uses_scene_water_env_map = (f.boolean(), f.boolean())

	return shape


_CLASS_PARSERS["CWaterShape"] = _parse_water_shape


@dataclass
class WaveMakerShape:
	period: float
	radius: float
	intensity: float
	pool_id: int
	impulsion_mode: bool


def _parse_wave_maker_shape(f: _Reader) -> WaveMakerShape:
	f.version()
	period = f.f32()
	radius = f.f32()
	intensity = f.f32()
	pool_id = f.u32()
	impulsion_mode = f.boolean()
	return WaveMakerShape(period, radius, intensity, pool_id, impulsion_mode)


_CLASS_PARSERS["CWaveMakerShape"] = _parse_wave_maker_shape


# ---------------------------------------------------------------------------
# CSegRemanenceShape
# ---------------------------------------------------------------------------


@dataclass
class SegRemanenceShape:
	num_slices: int
	slice_time: float
	corners: List[Vector3]
	material: Material
	bbox: AABBox
	texture_shifting: bool
	animated_material: Optional[MaterialBase]
	roll_up_ratio: Optional[float] = None
	default_pos: Optional[Vector3] = None
	default_rot_quat: Optional[Quaternion] = None
	default_scale: Optional[Vector3] = None


def _parse_seg_remanence_shape(f: _Reader) -> SegRemanenceShape:
	ver = f.version()
	num_slices = f.u32()
	slice_time = f.f32()
	n = f.cont_len()
	corners = [f.vector3() for _ in range(n)]
	material = _parse_material(f)
	bbox = f.aabbox()
	texture_shifting = f.boolean()

	node = f.u64()
	animated_material = None
	if node != 0:
		if node in f.seen:
			animated_material = f.seen[node]
		else:
			animated_material = _parse_material_base(f)
			f.seen[node] = animated_material

	shape = SegRemanenceShape(
		num_slices=num_slices, slice_time=slice_time, corners=corners,
		material=material, bbox=bbox, texture_shifting=texture_shifting,
		animated_material=animated_material,
	)

	if ver >= 1:
		shape.roll_up_ratio = f.f32()
	if ver >= 2:
		shape.default_pos = f.track_default_vector()
		shape.default_rot_quat = f.track_default_quat()
		shape.default_scale = f.track_default_vector()

	return shape


_CLASS_PARSERS["CSegRemanenceShape"] = _parse_seg_remanence_shape


# ---------------------------------------------------------------------------
# CParticleSystemShape
# ---------------------------------------------------------------------------


@dataclass
class ParticleSystemShape:
	proto_size: int
	user_param_default: List[float]
	default_pos: Optional[Vector3] = None
	default_scale: Optional[Vector3] = None
	default_rot_quat: Optional[Quaternion] = None
	max_view_dist: Optional[float] = None
	destroy_when_out_of_frustum: Optional[bool] = None
	destroy_model_when_out_of_range: Optional[bool] = None
	use_precomputed_bbox: Optional[bool] = None
	precomputed_bbox: Optional[AABBox] = None
	sharing: Optional[bool] = None


def _parse_particle_system_shape(f: _Reader) -> ParticleSystemShape:
	ver = f.version()
	proto_size = f.cont_len()
	f._take(proto_size)  # opaque particle-system prototype blob, not decoded

	user_param_default: List[float] = []
	if ver > 1:
		user_param_default = [f.track_default_float() for _ in range(4)]

	shape = ParticleSystemShape(proto_size=proto_size, user_param_default=user_param_default)

	if ver > 2:
		shape.default_pos = f.track_default_vector()
		shape.default_scale = f.track_default_vector()
		shape.default_rot_quat = f.track_default_quat()
	if ver > 3:
		shape.max_view_dist = f.f32()
		shape.destroy_when_out_of_frustum = f.boolean()
		shape.destroy_model_when_out_of_range = f.boolean()
	if ver > 4:
		shape.use_precomputed_bbox = f.boolean()
		if shape.use_precomputed_bbox:
			shape.precomputed_bbox = f.aabbox()
	if ver > 5:
		shape.sharing = f.boolean()

	return shape


_CLASS_PARSERS["CParticleSystemShape"] = _parse_particle_system_shape


# ---------------------------------------------------------------------------
# Top level
# ---------------------------------------------------------------------------

ShapeValue = Union[
	Mesh, MeshMRM, MeshMRMSkinned, MeshMultiLod, SkeletonShape,
	FlareShape, WaterShape, WaveMakerShape, SegRemanenceShape, ParticleSystemShape,
]


@dataclass
class ShapeFile:
	type_name: str
	value: ShapeValue


def parse_shape(data: bytes) -> ShapeFile:
	f = _Reader(data)
	f.check_magic(MAGIC)
	value = _read_poly_ptr(f)
	if value is None:
		raise ShapeParseError("shape pointer is NULL")
	type_name = type(value).__name__
	return ShapeFile(type_name=type_name, value=value)


def load_shape(path: Union[str, Path, BinaryIO]) -> ShapeFile:
	"""Load and parse a .shape file from a path or an open binary file object."""
	if isinstance(path, (str, Path)):
		data = Path(path).read_bytes()
	else:
		data = path.read()
	return parse_shape(data)


_ROOT_NODE_ID = 1

_SHAPE_CLASS_NAMES = {
	"Mesh": "CMesh",
	"MeshMRM": "CMeshMRM",
	"MeshMRMSkinned": "CMeshMRMSkinned",
	"MeshMultiLod": "CMeshMultiLod",
}


def dumps(shape_file: ShapeFile) -> bytes:
	"""Serialize a ShapeFile back to the .shape binary format.

	Only Mesh, MeshMRM, MeshMRMSkinned and MeshMultiLod can be written
	(CSkeletonShape, CFlareShape, CWaterShape, CWaveMakerShape,
	CSegRemanenceShape and CParticleSystemShape are read-only).

	CMesh is rewritten field-by-field, so its geometry (vertices, triangles,
	materials) is fully editable. For the other three, only the shared
	CMeshBase (materials/textures, via `.materials`) is reconstructed from
	the parsed data -- their geometry is copied back byte-for-byte exactly
	as it was read. CMeshMRM's progressive multi-resolution deltas are not
	meant to be hand-edited; that belongs to the 3D export pipeline.
	"""
	value = shape_file.value
	type_name = shape_file.type_name
	class_name = _SHAPE_CLASS_NAMES.get(type_name)
	if class_name is None:
		raise ShapeWriteError(f"writing is not supported for shape type {type_name!r}")

	f = _Writer()
	f.write_magic(MAGIC)
	f.u64(_ROOT_NODE_ID)
	f.string(class_name)

	if type_name == "Mesh":
		_write_mesh(f, value)
	else:
		# MeshMRM / MeshMRMSkinned / MeshMultiLod: editable materials, opaque geometry.
		f.version(value._version)
		_write_mesh_base(f, value.base)
		f.raw(value._raw_geom)

	return f.getvalue()


def save_shape(path: Union[str, Path, BinaryIO], shape_file: ShapeFile) -> None:
	"""Serialize a ShapeFile and write it to a path or an open binary file object."""
	data = dumps(shape_file)
	if isinstance(path, (str, Path)):
		Path(path).write_bytes(data)
	else:
		path.write(data)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def _describe_texture(t: Optional[Texture]) -> str:
	if t is None:
		return "-"
	if t.class_name == "CTextureMultiFile":
		return f"{t.file_name!r} (variants: {t.file_names})"
	return t.file_name or t.class_name


def _dump_materials(materials: List[Material]) -> None:
	print(f"materials: {len(materials)}")
	for i, mat in enumerate(materials):
		tex_descs = [_describe_texture(t) for t in mat.textures]
		print(f"  [{i}] shader={mat.shader_type} textures={tex_descs}")


def _dump(shape: ShapeFile) -> None:
	print(f"type: {shape.type_name}")
	v = shape.value

	if isinstance(v, (Mesh, MeshMRM, MeshMRMSkinned)):
		print(f"bbox: center={v.bbox.center} half_size={v.bbox.half_size}")
		print(f"vertices: {v.num_vertices}")
		print(f"triangles: {v.num_triangles}")
		_dump_materials(v.materials)
	elif isinstance(v, MeshMultiLod):
		_dump_materials(v.materials)
		print(f"lod slots: {len(v.slots)}")
		for i, slot in enumerate(v.slots):
			geom = slot.mesh_geom
			counts = f"vertices={geom.num_vertices} triangles={geom.num_triangles}" if geom else "empty"
			print(f"  [{i}] dist_max={slot.dist_max} {counts}")
	elif isinstance(v, SkeletonShape):
		print(f"bones: {len(v.bones)}")
		for i, bone in enumerate(v.bones):
			print(f"  [{i}] {bone.name!r} father={bone.father_id}")
		print(f"lods: {len(v.lods)}")
	elif isinstance(v, FlareShape):
		print(f"color: {v.color}")
		print(f"textures: {[_describe_texture(t) for t in v.textures]}")
	elif isinstance(v, WaterShape):
		print(f"polygon points: {len(v.polygon)}")
		print(f"water pool id: {v.water_pool_id}")
	elif isinstance(v, WaveMakerShape):
		print(f"period={v.period} radius={v.radius} pool_id={v.pool_id}")
	elif isinstance(v, SegRemanenceShape):
		print(f"corners: {len(v.corners)}")
		print(f"bbox: center={v.bbox.center} half_size={v.bbox.half_size}")
		print(f"material textures: {[_describe_texture(t) for t in v.material.textures]}")
	elif isinstance(v, ParticleSystemShape):
		print(f"opaque particle-system data: {v.proto_size} bytes (not decoded)")
		if v.precomputed_bbox:
			print(f"precomputed bbox: center={v.precomputed_bbox.center} half_size={v.precomputed_bbox.half_size}")


def _find_material(shape: ShapeFile, index: int) -> Material:
	materials = getattr(shape.value, "materials", None)
	if materials is None:
		raise SystemExit(f"shape type {shape.type_name!r} has no materials")
	if not 0 <= index < len(materials):
		raise SystemExit(f"no material at index {index} (shape has {len(materials)} materials)")
	return materials[index]


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read and edit Ryzom .shape files")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .shape file")
	p_dump.add_argument("path", type=Path)

	p_set_tex = sub.add_parser("set-texture", help="set a material's texture slot to a plain file texture")
	p_set_tex.add_argument("path", type=Path)
	p_set_tex.add_argument("material", type=int, help="0-based material index")
	p_set_tex.add_argument("slot", type=int, help=f"texture slot, 0-{IDRV_MAT_MAXTEXTURES - 1}")
	p_set_tex.add_argument("filename", help="texture file name, e.g. my_texture.tga")
	p_set_tex.add_argument("-o", "--output", type=Path, required=True)

	p_add_variant = sub.add_parser(
		"add-texture-variant", help="append a file name to an existing CTextureMultiFile slot"
	)
	p_add_variant.add_argument("path", type=Path)
	p_add_variant.add_argument("material", type=int, help="0-based material index")
	p_add_variant.add_argument("slot", type=int, help=f"texture slot, 0-{IDRV_MAT_MAXTEXTURES - 1}")
	p_add_variant.add_argument("filename", help="variant file name to append")
	p_add_variant.add_argument("-o", "--output", type=Path, required=True)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	try:
		shape = load_shape(args.path)
	except ShapeParseError as exc:
		raise SystemExit(f"cannot parse {args.path}: {exc}")

	if args.command == "dump":
		_dump(shape)
		return

	mat = _find_material(shape, args.material)
	if not 0 <= args.slot < len(mat.textures):
		raise SystemExit(f"invalid texture slot {args.slot} (material has {len(mat.textures)} slots)")

	if args.command == "set-texture":
		mat.textures[args.slot] = Texture(class_name="CTextureFile", file_name=args.filename, allow_degradation=False)
		print(f"material {args.material} slot {args.slot}: texture -> {args.filename!r}")

	elif args.command == "add-texture-variant":
		tex = mat.textures[args.slot]
		if tex is None or tex.class_name != "CTextureMultiFile":
			found = tex.class_name if tex else "no texture"
			raise SystemExit(
				f"material {args.material} slot {args.slot} is not a CTextureMultiFile (found {found}); "
				f"use set-texture first to give it one"
			)
		tex.file_names.append(args.filename)
		print(f"material {args.material} slot {args.slot}: variants -> {tex.file_names}")

	try:
		save_shape(args.output, shape)
	except ShapeWriteError as exc:
		raise SystemExit(f"cannot write {args.output}: {exc}")
	print(f"wrote {args.output}")


if __name__ == "__main__":
	_main()
