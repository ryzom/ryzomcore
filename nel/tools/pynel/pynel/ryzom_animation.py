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

"""Reader for Ryzom/NeL .anim files (CAnimation -- skeleton animation clips).

Format reverse-engineered from nel/src/3d/animation.cpp (CAnimation::serial,
the "NEL_ANIM" magic and the CTrackVector poly-ptr dispatch), plus the
serial() methods of the track types it can hold: CTrackDefaultVector/
CTrackDefaultQuat (track.h, a constant value -- same encoding as a
CSkeletonShape bone's own DefaultPos/DefaultRotQuat) and CTrackSampledVector/
CTrackSampledQuat (track_sampled_vector.cpp, track_sampled_quat.cpp,
track_sampled_common.cpp -- real keyframed data, quantized to compact
`sint16` for quaternions via CQuatPack, and organized into CTimeBlocks for
fast time lookup). The magic bytes are NELID("_LEN") followed by
NELID("MINA"), which -- because NELID reverses the 4-char literal when
packing it into a uint32 on a little-endian build -- serialize on disk as
the plainly readable ASCII string "NEL_ANIM" (verified against a real
ryzom-data .anim file's header bytes).

Also reads every real CTrackKeyFramer variant found in ryzom-data's own
.anim files (verified against the full 183-file corpus, 2026-09-05, see
project-todos/pynel/anim_read_extra_tracks.md): CTrackKeyFramerLinearVector/
Quat/Float, CTrackKeyFramerConstBool/String (held, never interpolated),
CTrackKeyFramerBezierVector/Float (cubic Bezier from stored tangents), and
CTrackKeyFramerTCBVector/Quat (Kochanek-Bartels, tangents/ease precomputed
once per track -- see _compile_tcb_track()/_compile_tcb_quat_track() --
TCBQuat's own on-disk key is a per-key angle-axis DELTA, not an absolute
quaternion, reconstructed the same way CTrackKeyFramerTCB<CKeyTCBQuat,...>
::compile() does). CTrackKeyFramerBezierQuat/TCBFloat/TCBInt/TCBRGBA and any
other ITrack subclass not listed above fail with a clear
AnimationParseError rather than silently producing wrong data -- none of
them appear anywhere in the corpus this was checked against.

Writing (build_animation()/dumps_animation()/save_animation(), see
project-todos/pynel/anim_write.md) only ever produces CTrackSampledVector/
Quat, CTrackDefaultVector/Quat, or CTrackKeyFramerLinearVector/Quat/Float
tracks (`track_format="sampled"` or `"keyframer_linear"`) -- Sampled
mirrors NL3D::CAnimationOptimizer (nel/src/3d/animation_optimizer.cpp), the
real tool that recompresses a 3dsMax-exported CTrackKeyFramerLinear*
animation into that format (the 3dsMax exporter itself, nel_mesh_lib/
export_anim.cpp, never produces a Sampled track directly) -- KeyFramerLinear
mirrors what that exporter actually writes, and what every real ryzom-data
.anim sampled so far (2026-09-05) actually uses. No write support for
Const*/Bezier*/TCB* -- not requested, read-only for those.

Usage:
	from ryzom_animation import load_animation
	anim = load_animation("walk.anim")
	print(anim.name, len(anim.tracks), anim.id_by_name)
"""

import argparse
import bisect
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Any, BinaryIO, Dict, List, Optional, Tuple, Union

import numpy

MAGIC = b"NEL_ANIM"  # on-disk bytes for NELID("_LEN") + NELID("MINA"), see CAnimation::serial


class AnimationParseError(Exception):
	pass


class AnimationWriteError(Exception):
	pass


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
class TimeBlock:
	time_offset: int
	key_offset: int
	times: List[int]  # per-key time, in samples relative to time_offset (not seconds)


@dataclass
class AngleAxis:
	"""NLMISC::CAngleAxis (quat.h) -- a rotation as an axis + angle (radians).
	Used only by CTrackKeyFramerTCBQuat, whose on-disk Value is one of these,
	not a Quaternion (see project-todos/pynel/anim_read_extra_tracks.md)."""
	axis: Vector3
	angle: float


@dataclass
class Keyframe:
	time: float
	value: Union[float, bool, str, Vector3, Quaternion, AngleAxis]

	# CTrackKeyFramerBezier* only (raw, from file) -- see key.h's CKeyBezier.
	in_tan: Optional[Union[float, Vector3]] = None
	out_tan: Optional[Union[float, Vector3]] = None
	step: Optional[bool] = None

	# CTrackKeyFramerTCB* only (raw, from file) -- see key.h's CKeyTCB.
	tension: Optional[float] = None
	continuity: Optional[float] = None
	bias: Optional[float] = None
	ease_to: Optional[float] = None
	ease_from: Optional[float] = None

	# CTrackKeyFramerTCB* only (derived, filled in once by _compile_tcb_track()
	# -- see project-todos/pynel/anim_read_extra_tracks.md's own "compile" note).
	tan_to: Optional[Union[float, Vector3]] = None
	tan_from: Optional[Union[float, Vector3]] = None
	ease0: Optional[float] = None
	ease1: Optional[float] = None
	ease_k: Optional[float] = None
	ease_k_over_ease0: Optional[float] = None
	ease_k_over_ease1: Optional[float] = None

	# CTrackKeyFramerTCBQuat only (derived) -- absolute quaternion (Quat) and
	# tangent quaternions (A/B, track_tcb.h's own naming), plus the raw
	# Value re-expressed as an axis normalized/made-positive per
	# CTrackKeyFramerTCB<CKeyTCBQuat,...>::compile()'s own LocalAngleAxis step.
	quat: Optional[Quaternion] = None
	tan_a: Optional[Quaternion] = None
	tan_b: Optional[Quaternion] = None
	local_angle_axis: Optional[AngleAxis] = None


@dataclass
class Track:
	"""A single animated channel (e.g. "Bip01 Head.rotquat", see
	Animation.id_by_name). `class_name` says which kind this is:
	- CTrackDefaultVector/CTrackDefaultQuat: a constant value the whole
	  clip -- only `value` is set.
	- CTrackSampledVector/CTrackSampledQuat: real keyframed data, pre-sampled
	  at a fixed rate -- `keys` (already decompressed/unpacked to plain
	  Vector3/Quaternion, see CQuatPack::unpack) plus the
	  CTrackSampledCommon timing fields; `time_blocks` groups them for fast
	  time lookup rather than storing an explicit time per key.
	- CTrackKeyFramerLinearVector/Quat/Float, CTrackKeyFramerConstBool/String,
	  CTrackKeyFramerBezierVector/Float, CTrackKeyFramerTCBVector/Quat: real
	  keyframed data, irregularly spaced -- `keyframes` (explicit (time,
	  value) pairs, extra per-key fields depending on class_name, see
	  Keyframe's own docstring and project-todos/pynel/
	  anim_read_extra_tracks.md).
	"""
	class_name: str
	value: Optional[Union[Vector3, Quaternion]] = None
	loop_mode: Optional[bool] = None
	begin_time: Optional[float] = None
	end_time: Optional[float] = None
	total_range: Optional[float] = None
	oo_total_range: Optional[float] = None
	delta_time: Optional[float] = None
	oo_delta_time: Optional[float] = None
	time_blocks: Optional[List[TimeBlock]] = None
	keys: Optional[List[Union[Vector3, Quaternion]]] = None
	keyframes: Optional[List[Keyframe]] = None  # CTrackKeyFramerLinear*/Const*/Bezier*/TCB* only
	range_lock: Optional[bool] = None  # CTrackKeyFramerLinear*/Const*/Bezier*/TCB* only
	compiled: bool = False  # CTrackKeyFramerTCB* only -- see _compile_tcb_track()


@dataclass
class Animation:
	name: str
	id_by_name: Dict[str, int]  # e.g. "Bip01 Head.rotquat" -> index into tracks
	tracks: List[Optional[Track]]
	min_end_time: Optional[float] = None  # version >= 1
	sss_shapes: Optional[List[str]] = None  # version >= 2


class _Reader:
	"""Minimal binary reader matching NeL's CIFile little-endian encoding."""

	def __init__(self, data: bytes):
		self._data = data
		self._pos = 0
		self.seen: Dict[int, Any] = {}

	def _take(self, size: int) -> bytes:
		end = self._pos + size
		if end > len(self._data):
			raise AnimationParseError(
				f"unexpected end of file at offset {self._pos} (needed {size} bytes)"
			)
		chunk = self._data[self._pos:end]
		self._pos = end
		return chunk

	def tell(self) -> int:
		return self._pos

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

	def u64(self) -> int:
		return struct.unpack("<Q", self._take(8))[0]

	def f32(self) -> float:
		return struct.unpack("<f", self._take(4))[0]

	def boolean(self) -> bool:
		return self.u8() != 0

	def string(self) -> str:
		length = self.u32()
		return self._take(length).decode("latin-1")

	def string_vector(self) -> List[str]:
		n = self.cont_len()
		return [self.string() for _ in range(n)]

	def vector3(self) -> Vector3:
		return Vector3(self.f32(), self.f32(), self.f32())

	def quaternion(self) -> Quaternion:
		return Quaternion(self.f32(), self.f32(), self.f32(), self.f32())

	def version(self) -> int:
		"""Mirrors IStream::serialVersion: one byte, or 0xFF + a uint32."""
		b = self.u8()
		if b == 0xFF:
			return self.u32()
		return b

	def check_magic(self, expected: bytes) -> None:
		got = self._take(len(expected))
		if got != expected:
			raise AnimationParseError(f"bad magic: expected {expected!r}, got {got!r}")

	def cont_len(self) -> int:
		"""Length prefix used by serialCont()/CObjectVector::serial() for generic containers."""
		return self.s32()

	def cont_uint_vector(self, item_size: int, fmt: str) -> List[int]:
		"""serialCont()/CObjectVector::serial() of a vector<uintN>: length-prefixed primitive vector."""
		n = self.cont_len()
		if n == 0:
			return []
		data = self._take(n * item_size)
		return list(struct.unpack(f"<{n}{fmt}", data))


# ---------------------------------------------------------------------------
# Polymorphic pointer dispatch (IStream::serialIStreamable, used for _TrackVector)
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
		raise AnimationParseError(
			f"unsupported track class {class_name!r} at offset {f.tell()}: cannot safely "
			f"skip a polymorphic object of unknown format"
		)
	obj = parser(f)
	f.seen[node] = obj
	return obj


# ---------------------------------------------------------------------------
# Tracks
# ---------------------------------------------------------------------------

_OO32767 = 1.0 / 32767  # CQuatPack::unpack's own scale factor (nel/src/3d/track_sampled_quat.cpp)


def _unpack_quat(x: int, y: int, z: int, w: int) -> Quaternion:
	"""Mirrors CQuatPack::unpack: scale the packed sint16s back to floats,
	then re-normalize (quantization alone doesn't round-trip to unit length)."""
	qx, qy, qz, qw = x * _OO32767, y * _OO32767, z * _OO32767, w * _OO32767
	length = math.sqrt(qx * qx + qy * qy + qz * qz + qw * qw)
	if length > 0:
		qx, qy, qz, qw = qx / length, qy / length, qz / length, qw / length
	return Quaternion(qx, qy, qz, qw)


def _parse_track_default_vector(f: _Reader) -> Track:
	f.version()
	return Track(class_name="CTrackDefaultVector", value=f.vector3())


_CLASS_PARSERS["CTrackDefaultVector"] = _parse_track_default_vector


def _parse_track_default_quat(f: _Reader) -> Track:
	f.version()
	return Track(class_name="CTrackDefaultQuat", value=f.quaternion())


_CLASS_PARSERS["CTrackDefaultQuat"] = _parse_track_default_quat


def _parse_time_block(f: _Reader) -> TimeBlock:
	f.version()
	time_offset = f.u16()
	key_offset = f.u32()
	times = f.cont_uint_vector(1, "B")
	return TimeBlock(time_offset=time_offset, key_offset=key_offset, times=times)


def _parse_time_blocks(f: _Reader) -> List[TimeBlock]:
	n = f.cont_len()
	return [_parse_time_block(f) for _ in range(n)]


def _parse_sampled_common(f: _Reader):
	"""Mirrors CTrackSampledCommon::serialCommon() -- shared timing fields for
	both CTrackSampledVector and CTrackSampledQuat."""
	f.version()
	loop_mode = f.boolean()
	begin_time = f.f32()
	end_time = f.f32()
	total_range = f.f32()
	oo_total_range = f.f32()
	delta_time = f.f32()
	oo_delta_time = f.f32()
	time_blocks = _parse_time_blocks(f)
	return loop_mode, begin_time, end_time, total_range, oo_total_range, delta_time, oo_delta_time, time_blocks


def _parse_track_sampled_vector(f: _Reader) -> Track:
	f.version()  # CTrackSampledVector::serial's own wrapper -- always delegates to serialCommon, no legacy branch
	(loop_mode, begin_time, end_time, total_range, oo_total_range,
	 delta_time, oo_delta_time, time_blocks) = _parse_sampled_common(f)
	n = f.cont_len()
	keys = [f.vector3() for _ in range(n)]
	return Track(
		class_name="CTrackSampledVector", loop_mode=loop_mode, begin_time=begin_time, end_time=end_time,
		total_range=total_range, oo_total_range=oo_total_range, delta_time=delta_time,
		oo_delta_time=oo_delta_time, time_blocks=time_blocks, keys=keys,
	)


_CLASS_PARSERS["CTrackSampledVector"] = _parse_track_sampled_vector


def _parse_track_sampled_quat(f: _Reader) -> Track:
	"""CTrackSampledQuat::serial has an extra legacy branch (version <= 0)
	that reads the same timing fields directly, without CTrackSampledCommon's
	own leading serialVersion(0) -- real/current data is version 1, but both
	are implemented for correctness."""
	ver = f.version()
	if ver <= 0:
		loop_mode = f.boolean()
		begin_time = f.f32()
		end_time = f.f32()
		total_range = f.f32()
		oo_total_range = f.f32()
		delta_time = f.f32()
		oo_delta_time = f.f32()
		time_blocks = _parse_time_blocks(f)
	else:
		(loop_mode, begin_time, end_time, total_range, oo_total_range,
		 delta_time, oo_delta_time, time_blocks) = _parse_sampled_common(f)
	n = f.cont_len()
	keys = [_unpack_quat(f.s16(), f.s16(), f.s16(), f.s16()) for _ in range(n)]
	return Track(
		class_name="CTrackSampledQuat", loop_mode=loop_mode, begin_time=begin_time, end_time=end_time,
		total_range=total_range, oo_total_range=oo_total_range, delta_time=delta_time,
		oo_delta_time=oo_delta_time, time_blocks=time_blocks, keys=keys,
	)


_CLASS_PARSERS["CTrackSampledQuat"] = _parse_track_sampled_quat


def _parse_keyframer_linear(f: _Reader, class_name: str, value_reader) -> Track:
	"""Mirrors ITrackKeyFramer<CKeyT>::serial() (track_keyframer.h): unlike
	the CTrackSampled* kind, keys here are stored as an explicit
	std::map<time, CKeyT> (irregularly spaced), not pre-sampled at a fixed
	rate. `value_reader` reads one CKeyT.Value (CKey<T>::serial is just its
	own leading version() + the raw value, no compression here)."""
	f.version()
	n = f.cont_len()
	keyframes = []
	for _ in range(n):
		time = f.f32()
		f.version()  # CKey<T>::serial's own leading serialVersion(0)
		keyframes.append(Keyframe(time=time, value=value_reader(f)))
	range_lock = f.boolean()
	range_begin = f.f32()
	range_end = f.f32()
	loop_mode = f.boolean()
	return Track(
		class_name=class_name, loop_mode=loop_mode, begin_time=range_begin, end_time=range_end,
		keyframes=keyframes, range_lock=range_lock,
	)


_CLASS_PARSERS["CTrackKeyFramerLinearQuat"] = lambda f: _parse_keyframer_linear(
	f, "CTrackKeyFramerLinearQuat", _Reader.quaternion)
_CLASS_PARSERS["CTrackKeyFramerLinearVector"] = lambda f: _parse_keyframer_linear(
	f, "CTrackKeyFramerLinearVector", _Reader.vector3)
_CLASS_PARSERS["CTrackKeyFramerLinearFloat"] = lambda f: _parse_keyframer_linear(
	f, "CTrackKeyFramerLinearFloat", _Reader.f32)

# CTrackKeyFramerConstBool/String: same container format as Linear (both
# CKeyT is the plain CKey<T> base -- version(0) + Value, see key.h) -- the
# difference is purely in evalKey() (hold, never interpolate, see
# _evaluate_keyframer()), not in how the track is stored on disk.
_CLASS_PARSERS["CTrackKeyFramerConstBool"] = lambda f: _parse_keyframer_linear(
	f, "CTrackKeyFramerConstBool", _Reader.boolean)
_CLASS_PARSERS["CTrackKeyFramerConstString"] = lambda f: _parse_keyframer_linear(
	f, "CTrackKeyFramerConstString", _Reader.string)


def _parse_keyframer_bezier(f: _Reader, class_name: str, value_reader) -> Track:
	"""Mirrors CKeyBezier<T>::serial() (key.h:128-142): same container as
	_parse_keyframer_linear(), but each key additionally carries InTan/OutTan
	(same type as Value) and a Step flag (bool)."""
	f.version()
	n = f.cont_len()
	keyframes = []
	for _ in range(n):
		time = f.f32()
		f.version()
		value = value_reader(f)
		in_tan = value_reader(f)
		out_tan = value_reader(f)
		step = f.boolean()
		keyframes.append(Keyframe(time=time, value=value, in_tan=in_tan, out_tan=out_tan, step=step))
	range_lock = f.boolean()
	range_begin = f.f32()
	range_end = f.f32()
	loop_mode = f.boolean()
	return Track(
		class_name=class_name, loop_mode=loop_mode, begin_time=range_begin, end_time=range_end,
		keyframes=keyframes, range_lock=range_lock,
	)


_CLASS_PARSERS["CTrackKeyFramerBezierVector"] = lambda f: _parse_keyframer_bezier(
	f, "CTrackKeyFramerBezierVector", _Reader.vector3)
_CLASS_PARSERS["CTrackKeyFramerBezierFloat"] = lambda f: _parse_keyframer_bezier(
	f, "CTrackKeyFramerBezierFloat", _Reader.f32)


def _parse_keyframer_tcb(f: _Reader, class_name: str, value_reader) -> Track:
	"""Mirrors CKeyTCB<T>::serial() (key.h:80-104): same container as
	_parse_keyframer_linear(), but each key additionally carries Tension/
	Continuity/Bias/EaseTo/EaseFrom (all f32) -- the raw authored TCB
	parameters. Tangents/ease factors are DERIVED from these (and from
	neighbor keys) by _compile_tcb_track(), not stored on disk."""
	f.version()
	n = f.cont_len()
	keyframes = []
	for _ in range(n):
		time = f.f32()
		f.version()
		value = value_reader(f)
		tension = f.f32()
		continuity = f.f32()
		bias = f.f32()
		ease_to = f.f32()
		ease_from = f.f32()
		keyframes.append(Keyframe(
			time=time, value=value, tension=tension, continuity=continuity, bias=bias,
			ease_to=ease_to, ease_from=ease_from))
	range_lock = f.boolean()
	range_begin = f.f32()
	range_end = f.f32()
	loop_mode = f.boolean()
	return Track(
		class_name=class_name, loop_mode=loop_mode, begin_time=range_begin, end_time=range_end,
		keyframes=keyframes, range_lock=range_lock,
	)


_CLASS_PARSERS["CTrackKeyFramerTCBVector"] = lambda f: _parse_keyframer_tcb(
	f, "CTrackKeyFramerTCBVector", _Reader.vector3)


def _read_angle_axis(f: _Reader) -> AngleAxis:
	"""Mirrors CAngleAxis::serial() (quat.h:50-54): Axis (CVector) then Angle (f32)."""
	axis = f.vector3()
	angle = f.f32()
	return AngleAxis(axis=axis, angle=angle)


_CLASS_PARSERS["CTrackKeyFramerTCBQuat"] = lambda f: _parse_keyframer_tcb(
	f, "CTrackKeyFramerTCBQuat", _read_angle_axis)


# ---------------------------------------------------------------------------
# Track evaluation
# ---------------------------------------------------------------------------


def _clamp(x: float, lo: float, hi: float) -> float:
	return lo if x < lo else hi if x > hi else x


def _slerp(a: Quaternion, b: Quaternion, t: float) -> Quaternion:
	"""CQuat::slerp() -- deliberately does NOT correct for `a`/`b` being on
	opposite hemispheres (dot < 0): the real engine's own implementation has
	that correction commented out (nel/include/nel/misc/quat.h, left with a
	"????" by the original author), relying entirely on exported animation
	data already having consecutive keys pre-baked onto the same hemisphere
	(see CTrackSampledQuat's own docstring on that guarantee). Reproducing
	that exactly -- rather than "fixing" it with the textbook shortest-path
	correction -- matters because this needs to match what the real client
	actually renders, including for the rarer tracks (typically
	CTrackKeyFramerLinearQuat, legacy biped exports) that aren't perfectly
	hemisphere-consistent: correcting it here produced visibly different
	(e.g. upside-down-looking) interpolated poses compared to the real
	client for exactly that data."""
	ax, ay, az, aw = a.x, a.y, a.z, a.w
	bx, by, bz, bw = b.x, b.y, b.z, b.w
	dot = _clamp(ax * bx + ay * by + az * bz + aw * bw, -1.0, 1.0)
	if dot > 0.9995:
		rx = ax + (bx - ax) * t
		ry = ay + (by - ay) * t
		rz = az + (bz - az) * t
		rw = aw + (bw - aw) * t
	else:
		theta0 = math.acos(dot)
		theta = theta0 * t
		sin_theta0 = math.sin(theta0)
		s0 = math.cos(theta) - dot * math.sin(theta) / sin_theta0
		s1 = math.sin(theta) / sin_theta0
		rx = ax * s0 + bx * s1
		ry = ay * s0 + by * s1
		rz = az * s0 + bz * s1
		rw = aw * s0 + bw * s1
	length = math.sqrt(rx * rx + ry * ry + rz * rz + rw * rw)
	if length > 0:
		rx, ry, rz, rw = rx / length, ry / length, rz / length, rw / length
	return Quaternion(rx, ry, rz, rw)


def _lerp_value(a, b, t: float):
	if isinstance(a, Quaternion):
		return _slerp(a, b, t)
	if isinstance(a, Vector3):
		return Vector3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t)
	return a + (b - a) * t


# ---------------------------------------------------------------------------
# Const/Bezier/TCB keyframer evaluation (see project-todos/pynel/
# anim_read_extra_tracks.md) -- Vector3/float-only arithmetic helpers first
# (Quaternion already has _lerp_value()/_slerp() above; Bezier/TCB Vector/
# Float need add/subtract/scale directly, not just lerp).
# ---------------------------------------------------------------------------


def _add(a, b):
	if isinstance(a, Vector3):
		return Vector3(a.x + b.x, a.y + b.y, a.z + b.z)
	return a + b


def _sub(a, b):
	if isinstance(a, Vector3):
		return Vector3(a.x - b.x, a.y - b.y, a.z - b.z)
	return a - b


def _scale(a, f: float):
	if isinstance(a, Vector3):
		return Vector3(a.x * f, a.y * f, a.z * f)
	return a * f


def _bezier_evalkey(prev_kf: "Keyframe", next_kf: "Keyframe", frac: float, dt: float):
	"""Mirrors CTrackKeyFramerBezier<CKeyT,T>::evalKey() (track_bezier.h:52-92)
	for the Vector/Float case (BezierQuat is absent from real content seen
	so far, see anim_read_extra_tracks.md, and not implemented here)."""
	if prev_kf.step:
		return prev_kf.value
	s = frac
	s2, s3 = s * s, s * s * s
	u = 1.0 - s
	u2, u3 = u * u, u * u * u
	cp0 = _add(prev_kf.value, _scale(prev_kf.out_tan, dt / 3.0))
	cp1 = _add(next_kf.value, _scale(next_kf.in_tan, dt / 3.0))
	return _add(
		_add(_scale(prev_kf.value, u3), _scale(cp0, 3.0 * u2 * s)),
		_add(_scale(cp1, 3.0 * u * s2), _scale(next_kf.value, s3)))


def _compile_tcb_ease(keyframes: List["Keyframe"], loop_mode: bool) -> None:
	"""Mirrors CTCBTools::compileTCBEase() (track_tcb.h:47-91)."""
	n = len(keyframes)
	for i, key in enumerate(keyframes):
		next_i = i + 1
		if next_i == n:
			if loop_mode and n > 1:
				next_i = 0
			else:
				key.ease_k = 0.5  # force ease() to just return d
				continue
		nxt = keyframes[next_i]
		e0, e1 = key.ease_from, nxt.ease_to
		s = e0 + e1
		if s > 1.0:
			e0, e1 = e0 / s, e1 / s
		key.ease0, key.ease1 = e0, e1
		key.ease_k = 1.0 / (2.0 - e0 - e1)
		if e0:
			key.ease_k_over_ease0 = key.ease_k / e0
		if e1:
			key.ease_k_over_ease1 = key.ease_k / e1


def _tcb_ease(key: "Keyframe", d: float) -> float:
	"""Mirrors CTCBTools::ease() (track_tcb.h:93-109)."""
	if d == 0.0 or d == 1.0:
		return d
	if key.ease_k == 0.5:
		return d
	if d < key.ease0:
		return key.ease_k_over_ease0 * d * d
	elif d < 1.0 - key.ease1:
		return key.ease_k * (2.0 * d - key.ease0)
	else:
		d = 1.0 - d
		return 1.0 - key.ease_k_over_ease1 * d * d


def _hermite_basis(d: float) -> Tuple[float, float, float, float]:
	"""Mirrors CTCBTools::computeHermiteBasis() (track_tcb.h:111-123)."""
	d2, d3 = d * d, d * d * d
	a = 3.0 * d2 - 2.0 * d3
	return (1.0 - a, a, d3 - 2.0 * d2 + d, d3 - d2)


def _tcb_factors(key: "Keyframe", time_before: float, time: float, time_after: float,
                  range_delta: float, first_key: bool, end_key: bool, is_loop: bool):
	"""Mirrors CTCBTools::computeTCBFactors() (track_tcb.h:126-179)."""
	if is_loop or (not first_key and not end_key):
		if first_key:
			dtm = 0.5 * (range_delta + time_after - time)
			fp = range_delta / dtm
			fn = (time_after - time) / dtm
		elif end_key:
			dtm = 0.5 * (range_delta + time - time_before)
			fp = range_delta / dtm
			fn = (time - time_before) / dtm
		else:
			dtm = 0.5 * (time_after - time_before)
			fp = (time - time_before) / dtm
			fn = (time_after - time) / dtm
		c = abs(key.continuity)
		fp = fp + c - c * fp
		fn = fn + c - c * fn
	else:
		fp = 1.0
		fn = 1.0

	cm = 1.0 - key.continuity
	tm = 0.5 * (1.0 - key.tension)
	cp = 2.0 - cm
	bm = 1.0 - key.bias
	bp = 2.0 - bm
	tmcm, tmcp = tm * cm, tm * cp

	ksm = tmcm * bp * fp
	ksp = tmcp * bm * fp
	kdm = tmcp * bp * fn
	kdp = tmcm * bm * fn
	return ksm, ksp, kdm, kdp


def _compute_tcb_key(key_before: "Keyframe", key: "Keyframe", key_after: "Keyframe",
                      time_before: float, time: float, time_after: float,
                      range_delta: float, first_key: bool, end_key: bool, is_loop: bool) -> None:
	"""Mirrors CTrackKeyFramerTCB<CKeyT,T>::computeTCBKey() (track_tcb.h:311-328)."""
	ksm, ksp, kdm, kdp = _tcb_factors(key, time_before, time, time_after, range_delta, first_key, end_key, is_loop)
	delm = _sub(key.value, key_before.value)
	delp = _sub(key_after.value, key.value)
	key.tan_to = _add(_scale(delm, ksm), _scale(delp, ksp))
	key.tan_from = _add(_scale(delm, kdm), _scale(delp, kdp))


def _compute_tcb_key_linear(key0: "Keyframe", key1: "Keyframe") -> None:
	"""Mirrors computeTCBKeyLinear() (track_tcb.h:331-341) -- the 2-key,
	non-looping special case."""
	f0, f1 = 1.0 - key0.tension, 1.0 - key1.tension
	dv = _sub(key1.value, key0.value)
	key0.tan_from = _scale(dv, f0)
	key1.tan_to = _scale(dv, f1)


def _compute_tcb_first_key(key_first: "Keyframe", key_after: "Keyframe") -> None:
	"""Mirrors computeFirstKey() (track_tcb.h:344-349)."""
	tm = 0.5 * (1.0 - key_first.tension)
	key_first.tan_from = _scale(_sub(_scale(_sub(key_after.value, key_first.value), 3.0), key_after.tan_to), tm)


def _compute_tcb_last_key(key_last: "Keyframe", key_before: "Keyframe") -> None:
	"""Mirrors computeLastKey() (track_tcb.h:352-357)."""
	tm = 0.5 * (1.0 - key_last.tension)
	key_last.tan_to = _scale(_sub(_scale(_sub(key_last.value, key_before.value), 3.0), key_before.tan_from), tm)


def _tcb_evalkey(prev_kf: "Keyframe", next_kf: "Keyframe", frac: float):
	"""Mirrors CTrackKeyFramerTCB<CKeyT,T>::evalKey() (track_tcb.h:207-237)
	for the Vector/Float case (TCBQuat, a different specialization, is
	handled by _tcb_quat_evalkey() instead)."""
	date = _tcb_ease(prev_kf, frac)
	hb = _hermite_basis(date)
	return _add(
		_add(_scale(prev_kf.value, hb[0]), _scale(next_kf.value, hb[1])),
		_add(_scale(prev_kf.tan_from, hb[2]), _scale(next_kf.tan_to, hb[3])))


def _compile_tcb_track(track: Track) -> None:
	"""Mirrors CTrackKeyFramerTCB<CKeyT,T>::compile() (track_tcb.h:239-302)
	for the Vector/Float case -- computes and caches (on each Keyframe) the
	ease parameters and Hermite tangents _tcb_evalkey() needs, once per
	track (see Track.compiled)."""
	keyframes = track.keyframes
	n = len(keyframes)
	_compile_tcb_ease(keyframes, track.loop_mode)
	if n <= 1:
		return
	if n == 2 and not track.loop_mode:
		_compute_tcb_key_linear(keyframes[0], keyframes[1])
		return

	range_delta = 0.0 if track.range_lock else (
		(track.end_time - track.begin_time) - (keyframes[-1].time - keyframes[0].time))

	for i in range(n):
		prev_i, next_i = (i - 1) % n, (i + 1) % n
		is_first, is_last_wrap = i == 0, next_i == 0
		if track.loop_mode or (not is_first and not is_last_wrap):
			_compute_tcb_key(
				keyframes[prev_i], keyframes[i], keyframes[next_i],
				keyframes[prev_i].time, keyframes[i].time, keyframes[next_i].time,
				range_delta, is_first, is_last_wrap, track.loop_mode)

	if not track.loop_mode:
		_compute_tcb_first_key(keyframes[0], keyframes[1])
		_compute_tcb_last_key(keyframes[-1], keyframes[-2])


# --- TCBQuat: its own specialization (track_tcb.h:363-567) -- angle-axis
# deltas relative to the preceding key, absolute quaternions/tangent
# quaternions reconstructed by compiling, evaluated via CQuat::squadrev(). ---

_QUAT_EPSILON = 1e-6  # NLMISC::QuatEpsilon
_TWO_PI = 2.0 * math.pi


def _quat_mul(a: Quaternion, b: Quaternion) -> Quaternion:
	"""Mirrors CQuatT<T>::operator*() (quat.h:230-241)."""
	return Quaternion(
		(a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y),
		(a.w * b.y) + (a.y * b.w) + (a.z * b.x) - (a.x * b.z),
		(a.w * b.z) + (a.z * b.w) + (a.x * b.y) - (a.y * b.x),
		(a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
	)


def _quat_scale(q: Quaternion, f: float) -> Quaternion:
	return Quaternion(q.x * f, q.y * f, q.z * f, q.w * f)


def _quat_add(a: Quaternion, b: Quaternion) -> Quaternion:
	return Quaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w)


def _quat_neg(q: Quaternion) -> Quaternion:
	return Quaternion(-q.x, -q.y, -q.z, -q.w)


def _quat_conjugate(q: Quaternion) -> Quaternion:
	return Quaternion(-q.x, -q.y, -q.z, q.w)


def _quat_inverted(q: Quaternion) -> Quaternion:
	"""Mirrors CQuatT<T>::inverted() -- conjugate() / sqrnorm()."""
	sqr_norm = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w
	if sqr_norm <= 0:
		return Quaternion(0.0, 0.0, 0.0, 1.0)
	return _quat_scale(_quat_conjugate(q), 1.0 / sqr_norm)


def _quat_normalize(q: Quaternion) -> Quaternion:
	length = math.sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w)
	if length <= 0:
		return Quaternion(0.0, 0.0, 0.0, 1.0)
	return Quaternion(q.x / length, q.y / length, q.z / length, q.w / length)


def _quat_make_closest(q: Quaternion, ref: Quaternion) -> Quaternion:
	"""Mirrors CQuatT<T>::makeClosest()."""
	if (q.x * ref.x + q.y * ref.y + q.z * ref.z + q.w * ref.w) < 0:
		return _quat_neg(q)
	return q


def _quat_log(q: Quaternion) -> Quaternion:
	"""Mirrors CQuatT<T>::log() (quat.h:417-431)."""
	length = math.sqrt(q.x * q.x + q.y * q.y + q.z * q.z)
	if length < _QUAT_EPSILON:
		return Quaternion(0.0, 0.0, 0.0, 0.0)
	div = math.acos(_clamp(q.w, -1.0, 1.0)) / length
	return Quaternion(q.x * div, q.y * div, q.z * div, 0.0)


def _quat_exp(q: Quaternion) -> Quaternion:
	"""Mirrors CQuatT<T>::exp() (quat.h:435-449)."""
	length = math.sqrt(q.x * q.x + q.y * q.y + q.z * q.z)
	if length < _QUAT_EPSILON:
		return Quaternion(0.0, 0.0, 0.0, 1.0)
	len1 = math.sin(length) / length
	return Quaternion(q.x * len1, q.y * len1, q.z * len1, math.cos(length))


def _quat_ln_dif(q0: Quaternion, q1: Quaternion) -> Quaternion:
	"""Mirrors CQuatT<T>::lnDif() (quat.h:452-458)."""
	return _quat_log(_quat_normalize(_quat_mul(_quat_inverted(q0), q1)))


def _quat_squad(q0: Quaternion, tgt_q0: Quaternion, tgt_q1: Quaternion, q1: Quaternion, t: float) -> Quaternion:
	"""Mirrors CQuatT<T>::squad() (quat.h:346-352)."""
	return _slerp(_slerp(q0, q1, t), _slerp(tgt_q0, tgt_q1, t), 2.0 * (1.0 - t) * t)


def _quat_squadrev(rot: "AngleAxis", q0: Quaternion, tgt_q0: Quaternion, tgt_q1: Quaternion, q1: Quaternion, t: float) -> Quaternion:
	"""Mirrors CQuatT<T>::squadrev() (quat.h:356-406) -- handles the
	multi-revolution case (an angle-axis representing more than a half turn)
	by subdividing into squad/slerp/squad segments; falls through to a
	plain squad() otherwise."""
	omega = rot.angle * 0.5
	if omega < math.pi - _QUAT_EPSILON:
		return _quat_squad(q0, tgt_q0, tgt_q1, q1, t)

	qaxis = Quaternion(rot.axis.x, rot.axis.y, rot.axis.z, 0.0)
	nrevs = omega / math.pi
	s = t * 2.0 * nrevs

	if s < 1.0:
		pp = _quat_mul(q0, qaxis)
		return _quat_squad(q0, tgt_q0, pp, pp, s)

	v = s - (2.0 * nrevs - 1.0)
	if v <= 0.0:
		while s >= 2.0:
			s -= 2.0
		pp = _quat_mul(q0, qaxis)
		return _slerp(q0, pp, s)

	qq = _quat_neg(_quat_mul(q1, qaxis))
	return _quat_squad(qq, qq, tgt_q1, q1, v)


def _quat_from_angle_axis(aa: "AngleAxis") -> Quaternion:
	half = aa.angle * 0.5
	s = math.sin(half)
	return Quaternion(aa.axis.x * s, aa.axis.y * s, aa.axis.z * s, math.cos(half))


def _normalize_vector3(v: Vector3) -> Vector3:
	length = math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z)
	if length <= 0:
		return v
	return Vector3(v.x / length, v.y / length, v.z / length)


def _rotate_vector_by_quat_inverse(q: Quaternion, v: Vector3) -> Vector3:
	"""Rotates v by q's inverse -- CMatrix::setRot(q).invert() * v, but for a
	pure rotation matrix the inverse is exactly its transpose (orthogonal
	matrix), avoiding a full 4x4 invert for this."""
	m = _mat_rotate(q)
	return Vector3(
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
		m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z,
	)


def _compute_tcb_quat_key(key_before: "Keyframe", key: "Keyframe", key_after: "Keyframe",
                           time_before: float, time: float, time_after: float,
                           range_delta: float, first_key: bool, end_key: bool, is_loop: bool) -> None:
	"""Mirrors the CKeyTCBQuat specialization's own computeTCBKey()
	(track_tcb.h:499-563)."""
	qm = Quaternion(0.0, 0.0, 0.0, 1.0)
	qp = Quaternion(0.0, 0.0, 0.0, 1.0)

	if not first_key or is_loop:
		if key.local_angle_axis.angle > _TWO_PI - _QUAT_EPSILON:
			axis = key.local_angle_axis.axis
			qm = _quat_log(Quaternion(axis.x, axis.y, axis.z, 0.0))
		else:
			qprev = _quat_make_closest(key_before.quat, key.quat)
			qm = _quat_ln_dif(qprev, key.quat)

	if not end_key or is_loop:
		if key_after.local_angle_axis.angle > _TWO_PI - _QUAT_EPSILON:
			axis = key_after.local_angle_axis.axis
			qp = _quat_log(Quaternion(axis.x, axis.y, axis.z, 0.0))
		else:
			qnext = _quat_make_closest(key_after.quat, key.quat)
			qp = _quat_ln_dif(key.quat, qnext)

	if first_key and not is_loop:
		qm = qp
	if end_key and not is_loop:
		qp = qm

	ksm, ksp, kdm, kdp = _tcb_factors(key, time_before, time, time_after, range_delta, first_key, end_key, is_loop)

	qb = _quat_scale(_quat_add(_quat_scale(qm, 1.0 - ksm), _quat_scale(qp, -ksp)), 0.5)
	qa = _quat_scale(_quat_add(_quat_scale(qm, kdm), _quat_scale(qp, kdp - 1.0)), 0.5)
	qa = _quat_exp(qa)
	qb = _quat_exp(qb)

	key.tan_a = _quat_mul(key.quat, qa)
	key.tan_b = _quat_mul(key.quat, qb)


def _compile_tcb_quat_track(track: Track) -> None:
	"""Mirrors the CKeyTCBQuat specialization's own compile()
	(track_tcb.h:410-491): each key's on-disk Value is a WORLD-space
	angle-axis DELTA relative to the preceding key -- reconstructed here
	into a per-key absolute Quat by walking the keys in order, plus the
	tangent quaternions (A/B, this module's tan_a/tan_b) used by
	_tcb_quat_evalkey()."""
	keyframes = track.keyframes
	n = len(keyframes)
	_compile_tcb_ease(keyframes, track.loop_mode)

	prev_quat = None
	for i, key in enumerate(keyframes):
		aa: AngleAxis = key.value
		if i == 0:
			local_axis, local_angle = aa.axis, aa.angle
		else:
			local_axis = _rotate_vector_by_quat_inverse(prev_quat, aa.axis)
			local_angle = aa.angle
		local_axis = _normalize_vector3(local_axis)
		if local_angle < 0.0:
			local_axis = Vector3(-local_axis.x, -local_axis.y, -local_axis.z)
			local_angle = -local_angle
		key.local_angle_axis = AngleAxis(axis=local_axis, angle=local_angle)

		relative_quat = _quat_from_angle_axis(key.local_angle_axis)
		key.quat = relative_quat if i == 0 else _quat_mul(prev_quat, relative_quat)
		prev_quat = key.quat

	if n <= 1:
		return
	range_delta = 0.0 if track.range_lock else (
		(track.end_time - track.begin_time) - (keyframes[-1].time - keyframes[0].time))

	for i in range(n):
		prev_i, next_i = (i - 1) % n, (i + 1) % n
		is_first, is_last_wrap = i == 0, next_i == 0
		_compute_tcb_quat_key(
			keyframes[prev_i], keyframes[i], keyframes[next_i],
			keyframes[prev_i].time, keyframes[i].time, keyframes[next_i].time,
			range_delta, is_first, is_last_wrap, track.loop_mode)


def _tcb_quat_evalkey(prev_kf: "Keyframe", next_kf: "Keyframe", frac: float) -> Quaternion:
	"""Mirrors the CKeyTCBQuat specialization's own evalKey() (track_tcb.h:379-408)."""
	date = _tcb_ease(prev_kf, frac)
	return _quat_squadrev(next_kf.local_angle_axis, prev_kf.quat, prev_kf.tan_a, next_kf.tan_b, next_kf.quat, date)


def _sampled_key_times(track: Track) -> List[float]:
	"""Per-key times, relative to `track.begin_time` (i.e. frame * delta_time),
	rebuilt from the TimeBlock frame indices -- CTrackSampledCommon::evalTime()'s
	own frame-quantized dichotomy search is just a compactness optimization over
	this same data, so a plain bisect on this flat list gives the same result."""
	times: List[float] = []
	for block in track.time_blocks or []:
		for t in block.times:
			frame = block.time_offset + t
			times.append(frame * track.delta_time)
	return times


def _evaluate_sampled(track: Track, time: float):
	keys = track.keys
	if not keys:
		raise ValueError(f"track {track.class_name!r} has no keys")
	if len(keys) == 1:
		return keys[0]

	local_time = time - track.begin_time
	total_range = track.total_range or 0.0
	if track.loop_mode and total_range > 0 and (local_time < 0 or local_time >= total_range):
		local_time -= math.floor(local_time / total_range) * total_range
		if local_time < 0 or local_time >= total_range:
			local_time = 0.0

	times = _sampled_key_times(track)
	idx = bisect.bisect_right(times, local_time) - 1
	if idx < 0:
		return keys[0]
	if idx >= len(keys) - 1:
		# past the last key: CTrackSampledCommon::evalTime() holds the last key's
		# value rather than blending back to the first one across the loop seam.
		return keys[-1]
	t0, t1 = times[idx], times[idx + 1]
	frac = 0.0 if t1 <= t0 else _clamp((local_time - t0) / (t1 - t0), 0.0, 1.0)
	return _lerp_value(keys[idx], keys[idx + 1], frac)


_TCB_COMPILERS = {
	"CTrackKeyFramerTCBVector": _compile_tcb_track,
	"CTrackKeyFramerTCBFloat": _compile_tcb_track,
	"CTrackKeyFramerTCBQuat": _compile_tcb_quat_track,
}


def _evaluate_keyframer(track: Track, time: float):
	keyframes = track.keyframes
	if not keyframes:
		raise ValueError(f"track {track.class_name!r} has no keyframes")

	tcb_compiler = _TCB_COMPILERS.get(track.class_name)
	if tcb_compiler is not None and not track.compiled:
		tcb_compiler(track)
		track.compiled = True
	is_tcb_quat = track.class_name == "CTrackKeyFramerTCBQuat"

	def result_value(kf: "Keyframe"):
		# CTrackKeyFramerTCBQuat's on-disk kf.value is a raw AngleAxis DELTA
		# (see _compile_tcb_quat_track()'s own docstring) -- every result
		# this function returns must be the reconstructed absolute
		# Quaternion (kf.quat) instead, never the raw kf.value.
		return kf.quat if is_tcb_quat else kf.value

	if len(keyframes) == 1:
		return result_value(keyframes[0])

	# ITrackKeyFramer::compile(): when RangeLock is set (the common/default case),
	# the serialized RangeBegin/RangeEnd are ignored and the loop bounds are
	# re-derived from the first/last keyframe's own time instead.
	if track.range_lock:
		loop_start = keyframes[0].time
		loop_end = keyframes[-1].time
	else:
		loop_start = track.begin_time
		loop_end = track.end_time
	total_range = loop_end - loop_start

	date = time
	if track.loop_mode and total_range > 0 and (date < loop_start or date >= loop_end):
		local = date - loop_start
		local -= math.floor(local / total_range) * total_range
		date = loop_start + local
		if date < loop_start or date >= loop_end:
			date = loop_start

	times = [k.time for k in keyframes]
	idx = bisect.bisect_right(times, date) - 1
	if idx < 0:
		return result_value(keyframes[0])
	if idx >= len(keyframes) - 1:
		if track.loop_mode and total_range > 0:
			# ITrackKeyFramer::eval(): at the exact loop seam (`date` was
			# already wrapped into [loop_start, loop_end) above and lands
			# exactly on the last keyframe's own time), the engine's search
			# for a `previous` key fails to find one at this exact point --
			# `evalKey()` then takes its previous==NULL branch and returns
			# the wrapped-to `next` key's value (the first keyframe)
			# directly, with NO interpolation at all, unlike every other
			# segment. Blending toward it (what this used to do) is a real,
			# if narrow, divergence from the engine right at the seam.
			return result_value(keyframes[0])
		return result_value(keyframes[-1])
	t0, t1 = times[idx], times[idx + 1]
	frac = 0.0 if t1 <= t0 else _clamp((date - t0) / (t1 - t0), 0.0, 1.0)
	prev_kf, next_kf = keyframes[idx], keyframes[idx + 1]

	if track.class_name.startswith("CTrackKeyFramerConst"):
		# evalKey() never blends for a Const track, even with both previous
		# and next present (track_keyframer.h:441-453) -- holds `previous`.
		return prev_kf.value
	if track.class_name.startswith("CTrackKeyFramerBezier"):
		return _bezier_evalkey(prev_kf, next_kf, frac, t1 - t0)
	if track.class_name == "CTrackKeyFramerTCBQuat":
		return _tcb_quat_evalkey(prev_kf, next_kf, frac)
	if track.class_name.startswith("CTrackKeyFramerTCB"):
		return _tcb_evalkey(prev_kf, next_kf, frac)
	return _lerp_value(prev_kf.value, next_kf.value, frac)


def evaluate_track(track: Track, time: float):
	"""Evaluates `track` at `time` (seconds), returning a plain float, Vector3,
	or Quaternion -- interpolating (slerp for rotations, lerp otherwise) between
	the two surrounding keys. A constant track (CTrackDefaultVector/Quat) always
	returns its single value regardless of `time`. Outside the track's own
	range, the value clamps to the nearest end unless `track.loop_mode` is set."""
	if track.value is not None:
		return track.value
	if track.keyframes is not None:
		return _evaluate_keyframer(track, time)
	if track.keys is not None:
		return _evaluate_sampled(track, time)
	raise ValueError(f"track {track.class_name!r} has neither a constant value, "
	                  f"keyframes, nor sampled keys")


# ---------------------------------------------------------------------------
# Bone pose evaluation
# ---------------------------------------------------------------------------

# NLMISC::CMatrix, as 4 rows of 4 floats (row-major, matches
# CMatrix::mulPoint()'s a11*x+a12*y+a13*z+a14 layout exactly): applying a
# matrix to a point is `_mat_point(m, p)`, composing two is `_mat_mul(a, b)`.


def _mat_identity():
	return (
		(1.0, 0.0, 0.0, 0.0),
		(0.0, 1.0, 0.0, 0.0),
		(0.0, 0.0, 1.0, 0.0),
		(0.0, 0.0, 0.0, 1.0),
	)


def _mat_translate(v) -> tuple:
	return (
		(1.0, 0.0, 0.0, v.x),
		(0.0, 1.0, 0.0, v.y),
		(0.0, 0.0, 1.0, v.z),
		(0.0, 0.0, 0.0, 1.0),
	)


def _mat_scale(v) -> tuple:
	return (
		(v.x, 0.0, 0.0, 0.0),
		(0.0, v.y, 0.0, 0.0),
		(0.0, 0.0, v.z, 0.0),
		(0.0, 0.0, 0.0, 1.0),
	)


def _mat_rotate(q) -> tuple:
	"""CMatrix::setRot(const CQuat&), replicated exactly (same a_ij formulas)."""
	x2, y2, z2 = q.x + q.x, q.y + q.y, q.z + q.z
	xx, xy, xz = q.x * x2, q.x * y2, q.x * z2
	yy, yz, zz = q.y * y2, q.y * z2, q.z * z2
	wx, wy, wz = q.w * x2, q.w * y2, q.w * z2
	return (
		(1.0 - (yy + zz), xy - wz, xz + wy, 0.0),
		(xy + wz, 1.0 - (xx + zz), yz - wx, 0.0),
		(xz - wy, yz + wx, 1.0 - (xx + yy), 0.0),
		(0.0, 0.0, 0.0, 1.0),
	)


def _mat_mul(a: tuple, b: tuple) -> tuple:
	return tuple(
		tuple(sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4))
		for i in range(4)
	)


def _mat_point(m: tuple, v) -> Vector3:
	return Vector3(
		m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
		m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
		m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3],
	)


def _mat_get_pos(m: tuple) -> Vector3:
	return Vector3(m[0][3], m[1][3], m[2][3])


def _matrix_field_to_dense(m) -> tuple:
	"""Converts a parsed ryzom_shape.Matrix (CMatrix's sparse on-disk
	encoding) to the dense 4x4 form used here -- same conversion as
	pynel.ryzom_skin's own _matrix_field_to_dense(), duplicated rather than
	imported (this module stays decoupled from ryzom_skin, see its own
	docstring on the same convention in reverse)."""
	if m is None:
		return _mat_identity()
	rot = m.rot or (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
	trans = m.trans or (0.0, 0.0, 0.0)
	return (
		(rot[0], rot[1], rot[2], trans[0]),
		(rot[3], rot[4], rot[5], trans[1]),
		(rot[6], rot[7], rot[8], trans[2]),
		(0.0, 0.0, 0.0, 1.0),
	)


def _invert_matrix(m: tuple) -> tuple:
	"""General inverse of a 4x4 matrix whose bottom row is (0,0,0,1) --
	CMatrix::inverted()'s "speed 34" path (matrix.cpp:1126-1171): the 3x3
	part inverted via the standard cofactor/adjugate formula (handles a
	non-uniform scale baked into it too, same as CMatrix::slowInvert33(),
	used whenever MAT_SCALEANY is set -- true for InvBindPos), then the
	translation re-derived from that inverted 3x3 (matrix.cpp:1150-1158)
	rather than naively negated. Only used to reconstruct a root bone's real
	bind orientation from InvBindPos (see _bone_local_matrix()) -- this
	project never needs to invert a MAT_PROJ matrix."""
	a11, a12, a13, tx = m[0]
	a21, a22, a23, ty = m[1]
	a31, a32, a33, tz = m[2]

	c11 = a22 * a33 - a23 * a32
	c12 = -(a21 * a33 - a23 * a31)
	c13 = a21 * a32 - a22 * a31
	c21 = -(a12 * a33 - a13 * a32)
	c22 = a11 * a33 - a13 * a31
	c23 = -(a11 * a32 - a12 * a31)
	c31 = a12 * a23 - a13 * a22
	c32 = -(a11 * a23 - a13 * a21)
	c33 = a11 * a22 - a12 * a21

	det = a11 * c11 + a12 * c12 + a13 * c13
	if det == 0:
		return _mat_identity()  # matches CMatrix::inverted()'s own fallback
	inv_det = 1.0 / det

	r11, r12, r13 = c11 * inv_det, c21 * inv_det, c31 * inv_det
	r21, r22, r23 = c12 * inv_det, c22 * inv_det, c32 * inv_det
	r31, r32, r33 = c13 * inv_det, c23 * inv_det, c33 * inv_det

	ntx = r11 * -tx + r12 * -ty + r13 * -tz
	nty = r21 * -tx + r22 * -ty + r23 * -tz
	ntz = r31 * -tx + r32 * -ty + r33 * -tz

	return (
		(r11, r12, r13, ntx),
		(r21, r22, r23, nty),
		(r31, r32, r33, ntz),
		(0.0, 0.0, 0.0, 1.0),
	)


def _bone_local_matrix(bone, anim: Optional[Animation], time: float) -> Tuple[tuple, Vector3]:
	"""The bone's local transform at `time` (and the Scale value used to build
	it, needed by the caller for CBone::compute()'s UnheritScale compensation
	below), animated where `anim` has a track for it (looked up as
	"{bone.name}.pos"/".rotquat"/".scale") and falling back to the bone's own
	default_* from the .skel otherwise. Matches NLMISC::CTransformable's
	`Local = T(Pos+Pivot) * R * S * T(-Pivot)`.

	Exception: a root bone (father_id<0) with no animation track overriding
	its position/rotation. The Max exporter deliberately writes DefaultPos/
	DefaultRotQuat as identity for the root (nel/tools/3d/plugin_max/
	nel_mesh_lib/export_skinning.cpp:307-313 -- "Root must be exported with
	Identity because path are setuped interactively in the root of the
	skeleton"): real gameplay supplies the character's actual world
	orientation externally instead of ever using this placeholder. But
	InvBindPos is baked from the bone's REAL bind-time orientation in Max,
	not this placeholder -- so naively using the placeholder here (as if it
	were the real bind pose) leaves every bone's skin matrix off by that
	real orientation, since it never cancels out against InvBindPos. The fix
	mirrors what the placeholder is meant to be overridden BY: reconstruct
	the real bind orientation as the inverse of InvBindPos, exactly what a
	"facing the same way it was bound" placement would evaluate to."""
	if bone.father_id < 0:
		pos_idx = anim.id_by_name.get(f"{bone.name}.pos") if anim is not None else None
		rot_idx = anim.id_by_name.get(f"{bone.name}.rotquat") if anim is not None else None
		has_pos_track = pos_idx is not None and anim.tracks[pos_idx] is not None
		has_rot_track = rot_idx is not None and anim.tracks[rot_idx] is not None
		if not has_pos_track and not has_rot_track:
			return _invert_matrix(_matrix_field_to_dense(bone.inv_bind_pos)), bone.default_scale

	pos, rot, scale = bone.default_pos, bone.default_rot_quat, bone.default_scale
	pivot = bone.default_pivot
	if anim is not None:
		pos_idx = anim.id_by_name.get(f"{bone.name}.pos")
		if pos_idx is not None and anim.tracks[pos_idx] is not None:
			pos = evaluate_track(anim.tracks[pos_idx], time)
		rot_idx = anim.id_by_name.get(f"{bone.name}.rotquat")
		if rot_idx is not None and anim.tracks[rot_idx] is not None:
			rot = evaluate_track(anim.tracks[rot_idx], time)
		scale_idx = anim.id_by_name.get(f"{bone.name}.scale")
		if scale_idx is not None and anim.tracks[scale_idx] is not None:
			scale = evaluate_track(anim.tracks[scale_idx], time)
	neg_pivot = Vector3(-pivot.x, -pivot.y, -pivot.z)
	pos_pivot = Vector3(pos.x + pivot.x, pos.y + pivot.y, pos.z + pivot.z)
	m = _mat_translate(pos_pivot)
	m = _mat_mul(m, _mat_rotate(rot))
	m = _mat_mul(m, _mat_scale(scale))
	m = _mat_mul(m, _mat_translate(neg_pivot))
	return m, scale


def _unherit_scale_comp(father_scale: Vector3, local_trans: Vector3) -> tuple:
	"""CBone::compute()'s UnheritScale compensation matrix: scales
	`local_trans` (the child's own local translation) by `1/father_scale`
	around `local_trans` itself, so a parent's non-uniform scale (typical of
	3dsMax biped rigs, baked into bone matrices to represent bone
	length/thickness) doesn't visually stretch the child's own geometry --
	only its position relative to the father is affected, matching the
	engine's real skinning behavior."""
	inv = Vector3(1.0 / father_scale.x, 1.0 / father_scale.y, 1.0 / father_scale.z)
	new_trans = Vector3(
		local_trans.x - inv.x * local_trans.x,
		local_trans.y - inv.y * local_trans.y,
		local_trans.z - inv.z * local_trans.z,
	)
	sm = _mat_scale(inv)
	return (
		(sm[0][0], sm[0][1], sm[0][2], new_trans.x),
		(sm[1][0], sm[1][1], sm[1][2], new_trans.y),
		(sm[2][0], sm[2][1], sm[2][2], new_trans.z),
		(0.0, 0.0, 0.0, 1.0),
	)


def evaluate_bone_world_matrix(skeleton, bone_name: str,
                                anim: Optional[Animation] = None,
                                time: float = 0.0) -> tuple:
	"""The world-space 4x4 matrix (see `_mat_*` above) of the bone named
	`bone_name` in `skeleton` (a `pynel.ryzom_shape.SkeletonShape`) at `time`,
	composed by walking up `father_id` to the root -- replicating
	`CBone::compute()` exactly, including its UnheritScale handling (the
	default for every bone) which keeps a parent's non-uniform scale from
	stretching its children's own local geometry. `anim` is optional --
	without it (or where it has no track for a given bone) every bone uses its
	`.skel` default pose, so this also works to preview a static bind pose."""
	index = skeleton.bone_map.get(bone_name)
	if index is None:
		raise ValueError(f"no such bone: {bone_name!r}")

	chain = []
	while index is not None and index >= 0:
		bone = skeleton.bones[index]
		chain.append(bone)
		index = bone.father_id if bone.father_id >= 0 else None
	chain.reverse()  # root first

	local_skeleton_matrix = None
	parent_scale = None
	for bone in chain:
		local, scale = _bone_local_matrix(bone, anim, time)
		if local_skeleton_matrix is None:
			local_skeleton_matrix = local
		elif bone.unherit_scale:
			comp = _unherit_scale_comp(parent_scale, _mat_get_pos(local))
			local_skeleton_matrix = _mat_mul(_mat_mul(local_skeleton_matrix, comp), local)
		else:
			local_skeleton_matrix = _mat_mul(local_skeleton_matrix, local)
		parent_scale = scale
	return local_skeleton_matrix


def evaluate_all_bone_world_matrices(skeleton, anim: Optional[Animation] = None, time: float = 0.0) -> dict:
	"""{bone name: world-space 4x4 matrix} for every bone in `skeleton` at
	`time` -- same math as evaluate_bone_world_matrix() (CBone::compute(),
	UnheritScale included), but computed once for the whole skeleton in a
	single O(bone count) pass instead of calling evaluate_bone_world_matrix()
	once per bone: that function independently re-walks each bone's own
	father_id chain up to the root every time it's called, so bones that
	share most of their ancestor chain (e.g. every bone of one limb, sharing
	the spine/root) redo the exact same parent matrix multiplications over
	and over. Here each bone's world matrix is computed exactly once and
	memoized (by index, via a closure over `computed`, recursing into the
	father first if it isn't cached yet) -- correct regardless of whether
	`skeleton.bones` happens to already be parent-before-child ordered.
	Meant for a whole animated character re-skinned every frame (see
	object_editor.py's _update_skin_preview()), where evaluate_bone_world_matrix()
	called once per bone becomes the dominant per-frame cost for anything
	past a handful of bones.

	2026-08-31, Nuno: "10 fps de perdu" in Patina's Bind preview live
	playback (which calls this once per frame for a ~60-100 bone skeleton)
	traced to THIS function -- 6.5ms/frame, pure Python. Per-bone LOCAL
	matrix composition (T*R*S*T, 4 chained _mat_mul() calls each, 64
	multiply-adds via nested Python loops per call) has no cross-bone
	dependency at all, unlike the hierarchy walk below -- so it's vectorized
	here as ONE batched numpy computation across every bone at once (closed-
	form: the 3x3 rotation-scale block is R's columns scaled by `scale`,
	translation is `RS @ (-pivot) + pos + pivot` -- see _bone_local_matrix()'s
	own T(pos+pivot)*R*S*T(-pivot) docstring, this is the same math with the
	chained-matmul intermediate matrices multiplied out by hand instead of
	built at runtime). Only the parent-before-child hierarchy accumulation
	(inherently sequential -- a child needs its parent's already-computed
	world matrix) stays a per-bone loop, now just one or two 4x4 matmuls per
	bone instead of the full local-matrix build too. Track evaluation itself
	(evaluate_track(), varying keyframe structures per bone) stays per-bone
	Python -- not the measured bottleneck, and not easily batchable without a
	much bigger rewrite."""
	bones = skeleton.bones
	n = len(bones)
	if n == 0:
		return {}

	positions = numpy.empty((n, 3), dtype=numpy.float64)
	rotations = numpy.empty((n, 4), dtype=numpy.float64)  # x, y, z, w
	scales = numpy.empty((n, 3), dtype=numpy.float64)
	pivots = numpy.empty((n, 3), dtype=numpy.float64)
	root_override_indices = []  # see _bone_local_matrix()'s own root-bone/InvBindPos note

	for i, bone in enumerate(bones):
		pos, rot, scale = bone.default_pos, bone.default_rot_quat, bone.default_scale
		pivot = bone.default_pivot
		if bone.father_id < 0:
			pos_idx = anim.id_by_name.get(f"{bone.name}.pos") if anim is not None else None
			rot_idx = anim.id_by_name.get(f"{bone.name}.rotquat") if anim is not None else None
			has_pos_track = pos_idx is not None and anim.tracks[pos_idx] is not None
			has_rot_track = rot_idx is not None and anim.tracks[rot_idx] is not None
			if not has_pos_track and not has_rot_track:
				root_override_indices.append(i)
				positions[i] = (0.0, 0.0, 0.0)
				rotations[i] = (0.0, 0.0, 0.0, 1.0)
				scales[i] = (scale.x, scale.y, scale.z)
				pivots[i] = (0.0, 0.0, 0.0)
				continue
		if anim is not None:
			pos_idx = anim.id_by_name.get(f"{bone.name}.pos")
			if pos_idx is not None and anim.tracks[pos_idx] is not None:
				pos = evaluate_track(anim.tracks[pos_idx], time)
			rot_idx = anim.id_by_name.get(f"{bone.name}.rotquat")
			if rot_idx is not None and anim.tracks[rot_idx] is not None:
				rot = evaluate_track(anim.tracks[rot_idx], time)
			scale_idx = anim.id_by_name.get(f"{bone.name}.scale")
			if scale_idx is not None and anim.tracks[scale_idx] is not None:
				scale = evaluate_track(anim.tracks[scale_idx], time)
		positions[i] = (pos.x, pos.y, pos.z)
		rotations[i] = (rot.x, rot.y, rot.z, rot.w)
		scales[i] = (scale.x, scale.y, scale.z)
		pivots[i] = (pivot.x, pivot.y, pivot.z)

	# CMatrix::setRot(const CQuat&), vectorized -- same a_ij formulas as
	# _mat_rotate() above, computed for every bone's quaternion at once.
	qx, qy, qz, qw = rotations[:, 0], rotations[:, 1], rotations[:, 2], rotations[:, 3]
	x2, y2, z2 = qx + qx, qy + qy, qz + qz
	xx, xy, xz = qx * x2, qx * y2, qx * z2
	yy, yz, zz = qy * y2, qy * z2, qz * z2
	wx, wy, wz = qw * x2, qw * y2, qw * z2
	rot_matrices = numpy.empty((n, 3, 3), dtype=numpy.float64)
	rot_matrices[:, 0, 0] = 1.0 - (yy + zz)
	rot_matrices[:, 0, 1] = xy - wz
	rot_matrices[:, 0, 2] = xz + wy
	rot_matrices[:, 1, 0] = xy + wz
	rot_matrices[:, 1, 1] = 1.0 - (xx + zz)
	rot_matrices[:, 1, 2] = yz - wx
	rot_matrices[:, 2, 0] = xz - wy
	rot_matrices[:, 2, 1] = yz + wx
	rot_matrices[:, 2, 2] = 1.0 - (xx + yy)

	rs_matrices = rot_matrices * scales[:, None, :]  # R * S: scale each column
	trans = numpy.einsum("nij,nj->ni", rs_matrices, -pivots) + positions + pivots

	local_matrices = numpy.zeros((n, 4, 4), dtype=numpy.float64)
	local_matrices[:, :3, :3] = rs_matrices
	local_matrices[:, :3, 3] = trans
	local_matrices[:, 3, 3] = 1.0

	for i in root_override_indices:
		local_matrices[i] = numpy.array(_invert_matrix(_matrix_field_to_dense(bones[i].inv_bind_pos)), dtype=numpy.float64)

	computed = {}  # index -> (world_matrix numpy (4,4), local_scale numpy (3,)) -- local_scale is what a child's own UnheritScale reads

	def compute(index):
		cached = computed.get(index)
		if cached is not None:
			return cached
		bone = bones[index]
		local = local_matrices[index]
		scale = scales[index]
		if bone.father_id is None or bone.father_id < 0:
			world = local
		else:
			parent_world, parent_scale = compute(bone.father_id)
			if bone.unherit_scale:
				inv = 1.0 / parent_scale
				local_trans = local[:3, 3]
				comp = numpy.eye(4, dtype=numpy.float64)
				comp[[0, 1, 2], [0, 1, 2]] = inv
				comp[:3, 3] = local_trans - inv * local_trans
				world = parent_world @ comp @ local
			else:
				world = parent_world @ local
		computed[index] = (world, scale)
		return computed[index]

	# Converted back to plain nested tuples of Python floats (not numpy
	# scalars) -- matches evaluate_bone_world_matrix()'s own return type,
	# what every caller (e.g. pynel.ryzom_skin.bone_skin_matrices_for_mesh(),
	# object_editor.py's _nel_matrix_to_panda_mat4()) already expects.
	return {bone.name: tuple(tuple(row) for row in compute(index)[0].tolist()) for index, bone in enumerate(bones)}


def animation_duration(anim: Animation) -> float:
	"""The clip's overall length in seconds: the latest `end_time` among its
	tracks (constant tracks have none and don't count -- they don't bound the
	clip's length)."""
	duration = 0.0
	for track in anim.tracks:
		if track is not None and track.end_time is not None:
			duration = max(duration, track.end_time)
	return duration


# ---------------------------------------------------------------------------
# Top level
# ---------------------------------------------------------------------------


def parse_animation(data: bytes) -> Animation:
	f = _Reader(data)
	f.check_magic(MAGIC)
	version = f.version()
	name = f.string()

	n = f.cont_len()
	id_by_name: Dict[str, int] = {}
	for _ in range(n):
		key = f.string()
		id_by_name[key] = f.u32()

	track_count = f.cont_len()
	tracks = [_read_poly_ptr(f) for _ in range(track_count)]

	min_end_time = f.f32() if version >= 1 else None
	sss_shapes = f.string_vector() if version >= 2 else None

	return Animation(name=name, id_by_name=id_by_name, tracks=tracks,
	                  min_end_time=min_end_time, sss_shapes=sss_shapes)


def load_animation(path: Union[str, Path, BinaryIO]) -> Animation:
	"""Load and parse a .anim file from a path or an open binary file object."""
	if isinstance(path, (str, Path)):
		data = Path(path).read_bytes()
	else:
		data = path.read()
	return parse_animation(data)


# ---------------------------------------------------------------------------
# Writer (see project-todos/pynel/anim_write.md)
# ---------------------------------------------------------------------------

# NL3D::CAnimationOptimizer::CAnimationOptimizer()'s own defaults
# (animation_optimizer.cpp) -- the real tool this module's writer mirrors.
_SAMPLE_FRAME_RATE = 30.0
_QUAT_THRESHOLD = 1.0 - 0.000001  # dot-product threshold, "high precision" tier
_VECTOR_THRESHOLD = 0.0001  # distance threshold, "high precision" tier


class _Writer:
	"""Minimal binary writer matching NeL's COFile little-endian encoding --
	same primitives as pynel.ryzom_shape._Writer, duplicated locally (same
	convention as this module's own _Reader vs. ryzom_shape.py's)."""

	def __init__(self):
		self._chunks: List[bytes] = []
		self._written_ids: Dict[int, int] = {}
		self._next_id = 1

	def u8(self, v: int) -> None:
		self._chunks.append(struct.pack("<B", v))

	def s16(self, v: int) -> None:
		self._chunks.append(struct.pack("<h", v))

	def u16(self, v: int) -> None:
		self._chunks.append(struct.pack("<H", v))

	def s32(self, v: int) -> None:
		self._chunks.append(struct.pack("<i", v))

	def u32(self, v: int) -> None:
		self._chunks.append(struct.pack("<I", v))

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

	def vector3(self, v: Vector3) -> None:
		self.f32(v.x)
		self.f32(v.y)
		self.f32(v.z)

	def quaternion(self, q: Quaternion) -> None:
		self.f32(q.x)
		self.f32(q.y)
		self.f32(q.z)
		self.f32(q.w)

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

	def cont_uint_vector(self, values: List[int], fmt: str) -> None:
		self.cont_len(len(values))
		if values:
			self._chunks.append(struct.pack(f"<{len(values)}{fmt}", *values))

	def string_vector(self, names: List[str]) -> None:
		self.cont_len(len(names))
		for name in names:
			self.string(name)

	def getvalue(self) -> bytes:
		return b"".join(self._chunks)

	def write_poly_ptr(self, obj, class_name: Optional[str], write_body) -> None:
		"""Mirrors IStream::serialIStreamable on write: node id, then class
		name + body only the first time a given python object is written --
		same idiom as pynel.ryzom_shape._Writer.write_poly_ptr()."""
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


def _pack_quat(q: Quaternion) -> Tuple[int, int, int, int]:
	"""Inverse of _unpack_quat()/CQuatPack::pack: scales an already
	unit-length quaternion's components by 32767 and rounds to the nearest
	int16, clamped to [-32767, 32767] (the same symmetric range
	_unpack_quat() divides by)."""
	def pack_component(c: float) -> int:
		return max(-32767, min(32767, round(c * 32767)))
	return pack_component(q.x), pack_component(q.y), pack_component(q.z), pack_component(q.w)


def _sample_track(times: List[float], values: List, begin_time: float, end_time: float, is_quat: bool):
	"""Mirrors CAnimationOptimizer::sampleQuatTrack()/sampleVectorTrack():
	resamples an irregular (times, values) source track (assimp's own
	NodeAnimTrack shape) at a fixed 30Hz rate between begin_time/end_time,
	via _lerp_value() (already SLERPs a Quaternion, lerps a Vector3). The
	last sample is forced to exactly end_time (float precision, same as the
	reference). A quaternion sample is renormalized then, if its dot product
	with the previous sample is negative, negated (CQuat::makeClosest) so
	consecutive samples stay on the same hemisphere -- required since
	_slerp() deliberately never corrects for that itself (see its own
	docstring). Returns (time_list, key_list): time_list is [0, 1, ...,
	num_samples-1] (integer frame indices, NOT seconds -- CTrackSampledCommon's
	own convention, see buildCommon())."""
	duration = end_time - begin_time
	num_samples = max(1, math.ceil(duration * _SAMPLE_FRAME_RATE))
	time_list = list(range(num_samples))
	key_list = []
	segment = 0
	for i in range(num_samples):
		t = end_time if i == num_samples - 1 else (begin_time if num_samples <= 1 else begin_time + i * duration / (num_samples - 1))
		while segment < len(times) - 2 and t > times[segment + 1]:
			segment += 1
		t0, t1 = times[segment], times[min(segment + 1, len(times) - 1)]
		frac = 0.0 if t1 == t0 else _clamp((t - t0) / (t1 - t0), 0.0, 1.0)
		value = _lerp_value(values[segment], values[min(segment + 1, len(values) - 1)], frac)
		if is_quat:
			length = math.sqrt(value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w)
			if length > 0:
				value = Quaternion(value.x / length, value.y / length, value.z / length, value.w / length)
			if key_list:
				prev = key_list[-1]
				dot = value.x * prev.x + value.y * prev.y + value.z * prev.z + value.w * prev.w
				if dot < 0:
					value = Quaternion(-value.x, -value.y, -value.z, -value.w)
		key_list.append(value)
	return time_list, key_list


def _is_nearly_constant(key_list: List, is_quat: bool) -> bool:
	"""Mirrors testConstantQuatTrack()/testConstantVectorTrack(): true if
	every sample is within threshold of the first one."""
	ref = key_list[0]
	for key in key_list:
		if is_quat:
			dot = _clamp(ref.x * key.x + ref.y * key.y + ref.z * key.z + ref.w * key.w, -1.0, 1.0)
			if dot < 0:
				dot = -dot
			if dot < _QUAT_THRESHOLD:
				return False
		else:
			dx, dy, dz = key.x - ref.x, key.y - ref.y, key.z - ref.z
			if math.sqrt(dx * dx + dy * dy + dz * dz) > _VECTOR_THRESHOLD:
				return False
	return True


def _optimize_keys(time_list: List[int], key_list: List, is_quat: bool):
	"""Mirrors optimizeQuatTrack()/optimizeVectorTrack(): greedy reduction
	keeping the first and last key always, and any intermediate key that
	can't be reconstructed (within threshold) by interpolating between the
	last kept key and the next one, or whose frame gap from the last kept
	key would otherwise exceed 255 (CTrackSampledCommon's own per-block
	limit, see buildCommon()). No-op for <=2 keys. Uses the mathematically
	correct interpolation fraction `(timeCur-timeRef)/(timeNext-timeRef)`
	rather than the reference C++'s own `timeNext/timeRef` (looks like a
	bug there -- see project-todos/pynel/anim_write.md) -- this step only
	decides which keys to DROP, so a different fraction changes at most the
	compression ratio, never the correctness of a key that IS kept (always
	its real sampled value)."""
	num_samples = len(key_list)
	if num_samples <= 2:
		return time_list, key_list

	opt_times = [time_list[0]]
	opt_keys = [key_list[0]]
	time_ref, key_ref = time_list[0], key_list[0]

	for i in range(1, num_samples - 1):
		key_cur, key_next = key_list[i], key_list[i + 1]
		time_cur, time_next = time_list[i], time_list[i + 1]
		must_add = False

		if time_next - time_ref > 255:
			must_add = True
		elif is_quat and (
				(key_ref.x * key_cur.x + key_ref.y * key_cur.y + key_ref.z * key_cur.z + key_ref.w * key_cur.w) < 0
				or (key_ref.x * key_next.x + key_ref.y * key_next.y + key_ref.z * key_next.z + key_ref.w * key_next.w) < 0):
			must_add = True
		else:
			ref_vs_cur = _is_nearly_constant([key_ref, key_cur], is_quat)
			ref_vs_next = _is_nearly_constant([key_ref, key_next], is_quat)
			if ref_vs_cur and ref_vs_next:
				must_add = False
			else:
				frac = 0.0 if time_next == time_ref else (time_cur - time_ref) / (time_next - time_ref)
				interpolated = _lerp_value(key_ref, key_next, frac)
				if not _is_nearly_constant([key_cur, interpolated], is_quat):
					must_add = True

		if must_add:
			opt_times.append(time_cur)
			opt_keys.append(key_cur)
			time_ref, key_ref = time_cur, key_cur

	opt_times.append(time_list[-1])
	opt_keys.append(key_list[-1])
	return opt_times, opt_keys


def _build_time_blocks(time_list: List[int]) -> List[TimeBlock]:
	"""Mirrors CTrackSampledCommon::buildCommon() (nel/src/3d/
	track_sampled_common.cpp:100-193) -- groups `time_list` (a strictly
	increasing list of frame numbers starting at 0, consecutive gaps <=255,
	guaranteed by _optimize_keys()) into TimeBlocks of at most 256 keys
	each, splitting whenever the next key's frame number would land more
	than 255 samples past the current block's own TimeOffset."""
	num_keys = len(time_list)
	if num_keys == 0:
		return []
	if num_keys == 1:
		return [TimeBlock(time_offset=0, key_offset=0, times=[0])]

	blocks = []
	block_start_index = 0
	block_time_offset = time_list[0]
	for i in range(1, num_keys):
		if time_list[i] - block_time_offset > 255:
			blocks.append(TimeBlock(
				time_offset=block_time_offset, key_offset=block_start_index,
				times=[time_list[j] - block_time_offset for j in range(block_start_index, i)]))
			block_start_index = i
			block_time_offset = time_list[i]
	blocks.append(TimeBlock(
		time_offset=block_time_offset, key_offset=block_start_index,
		times=[time_list[j] - block_time_offset for j in range(block_start_index, num_keys)]))
	return blocks


def _write_time_block(f: _Writer, block: TimeBlock) -> None:
	f.version(0)
	f.u16(block.time_offset)
	f.u32(block.key_offset)
	f.cont_uint_vector(block.times, "B")


def _write_sampled_common(f: _Writer, track: Track) -> None:
	f.version(0)
	f.boolean(track.loop_mode)
	f.f32(track.begin_time)
	f.f32(track.end_time)
	f.f32(track.total_range)
	f.f32(track.oo_total_range)
	f.f32(track.delta_time)
	f.f32(track.oo_delta_time)
	f.cont_len(len(track.time_blocks))
	for block in track.time_blocks:
		_write_time_block(f, block)


def _write_track_sampled_vector(f: _Writer, track: Track) -> None:
	"""Always writes the latest CTrackSampledVector format (version 1) --
	_parse_track_sampled_vector()'s own comment confirms it always delegates
	to serialCommon regardless of version (no legacy branch for Vector)."""
	f.version(1)
	_write_sampled_common(f, track)
	f.cont_len(len(track.keys))
	for v in track.keys:
		f.vector3(v)


def _write_track_sampled_quat(f: _Writer, track: Track) -> None:
	"""Always writes the latest CTrackSampledQuat format (version 1, the
	serialCommon branch -- see _parse_track_sampled_quat()'s own version<=0
	legacy branch, never written here)."""
	f.version(1)
	_write_sampled_common(f, track)
	f.cont_len(len(track.keys))
	for q in track.keys:
		x, y, z, w = _pack_quat(q)
		f.s16(x)
		f.s16(y)
		f.s16(z)
		f.s16(w)


def _write_track_default_vector(f: _Writer, track: Track) -> None:
	f.version(0)
	f.vector3(track.value)


def _write_track_default_quat(f: _Writer, track: Track) -> None:
	f.version(0)
	f.quaternion(track.value)


def _fix_quat_hemisphere_sequence(values: List[Quaternion]) -> List[Quaternion]:
	"""Applies CQuat::makeClosest() against the previous value, walking the
	sequence in order -- the same hemisphere-continuity fix the real 3dsMax
	exporter applies when writing rotation keyframes (export_anim.cpp:1487)
	and CAnimationOptimizer applies when sampling (see _sample_track()) --
	needed because _slerp() deliberately never corrects for that itself
	(see its own docstring)."""
	fixed = []
	prev = None
	for v in values:
		if prev is not None and (v.x * prev.x + v.y * prev.y + v.z * prev.z + v.w * prev.w) < 0:
			v = Quaternion(-v.x, -v.y, -v.z, -v.w)
		fixed.append(v)
		prev = v
	return fixed


def _write_track_keyframer_linear(f: _Writer, track: Track, value_writer) -> None:
	"""Mirrors ITrackKeyFramer<CKeyT>::serial() (track_keyframer.h:239-249):
	version(0), then serialCont() of the map<time, CKeyT> (count + per-entry
	time/CKey<T>::serial -- its own version(0) + the raw value, no
	compression/tangent), then range_lock/range_begin/range_end/loop_mode."""
	f.version(0)
	f.cont_len(len(track.keyframes))
	for keyframe in track.keyframes:
		f.f32(keyframe.time)
		f.version(0)  # CKey<T>::serial's own leading serialVersion(0)
		value_writer(f, keyframe.value)
	f.boolean(track.range_lock)
	f.f32(track.begin_time)
	f.f32(track.end_time)
	f.boolean(track.loop_mode)


def _write_track_keyframer_linear_vector(f: _Writer, track: Track) -> None:
	_write_track_keyframer_linear(f, track, _Writer.vector3)


def _write_track_keyframer_linear_quat(f: _Writer, track: Track) -> None:
	_write_track_keyframer_linear(f, track, _Writer.quaternion)


def _write_track_keyframer_linear_float(f: _Writer, track: Track) -> None:
	_write_track_keyframer_linear(f, track, _Writer.f32)


_CLASS_WRITERS = {
	"CTrackSampledVector": _write_track_sampled_vector,
	"CTrackSampledQuat": _write_track_sampled_quat,
	"CTrackDefaultVector": _write_track_default_vector,
	"CTrackDefaultQuat": _write_track_default_quat,
	"CTrackKeyFramerLinearVector": _write_track_keyframer_linear_vector,
	"CTrackKeyFramerLinearQuat": _write_track_keyframer_linear_quat,
	"CTrackKeyFramerLinearFloat": _write_track_keyframer_linear_float,
}


def _write_track(f: _Writer, track: Track) -> None:
	writer = _CLASS_WRITERS.get(track.class_name)
	if writer is None:
		raise AnimationWriteError(f"writing is not supported for track class {track.class_name!r}")
	writer(f, track)


def _build_sampled_track(times: List[float], values: List, is_quat: bool, loop_mode: bool) -> Track:
	"""Builds one Track (CTrackSampledVector/Quat, or CTrackDefaultVector/Quat
	if the sampled data turns out nearly constant -- see anim_write.md) from
	a single channel's raw (times, values) keys."""
	begin_time, end_time = times[0], times[-1]
	if end_time <= begin_time:
		end_time = begin_time  # single-key/degenerate source channel
	time_list, key_list = _sample_track(times, values, begin_time, end_time, is_quat)

	if _is_nearly_constant(key_list, is_quat):
		class_name = "CTrackDefaultQuat" if is_quat else "CTrackDefaultVector"
		return Track(class_name=class_name, value=key_list[0])

	time_list, key_list = _optimize_keys(time_list, key_list, is_quat)
	time_blocks = _build_time_blocks(time_list)
	num_frames = time_list[-1] - time_list[0]
	delta_time = (end_time - begin_time) / num_frames if num_frames > 0 else 0.0
	total_range = end_time - begin_time
	class_name = "CTrackSampledQuat" if is_quat else "CTrackSampledVector"
	return Track(
		class_name=class_name, loop_mode=loop_mode, begin_time=begin_time, end_time=end_time,
		total_range=total_range, oo_total_range=(1.0 / total_range if total_range else 0.0),
		delta_time=delta_time, oo_delta_time=(1.0 / delta_time if delta_time else 0.0),
		time_blocks=time_blocks, keys=key_list,
	)


def _build_keyframer_linear_track(times: List[float], values: List, is_quat: bool, loop_mode: bool) -> Track:
	"""Builds one Track (CTrackKeyFramerLinearVector/Quat) directly from a
	channel's raw (times, values) keys -- no resampling, the source's own
	irregular key times are kept as-is (matches export_anim.cpp::
	createKeyFramer, the real 3dsMax exporter's own track format). Rotation
	values get the same hemisphere-continuity fix the exporter itself
	applies (see _fix_quat_hemisphere_sequence())."""
	if is_quat:
		values = _fix_quat_hemisphere_sequence(values)
	class_name = "CTrackKeyFramerLinearQuat" if is_quat else "CTrackKeyFramerLinearVector"
	keyframes = [Keyframe(time=t, value=v) for t, v in zip(times, values)]
	return Track(
		class_name=class_name, loop_mode=loop_mode, begin_time=times[0], end_time=times[-1],
		keyframes=keyframes, range_lock=False,
	)


_TRACK_BUILDERS = {
	"sampled": _build_sampled_track,
	"keyframer_linear": _build_keyframer_linear_track,
}


def build_animation(
		name: str, node_tracks: Dict[str, Dict[str, Tuple[List[float], List]]],
		track_format: str = "sampled", loop_mode: bool = False,
) -> Animation:
	"""Builds an in-memory Animation from raw per-bone channels (see
	project-todos/pynel/anim_write.md).

	`node_tracks`: {bone_name: {"pos": (times, [Vector3, ...]), "rotquat":
	(times, [Quaternion, ...]), "scale": (times, [Vector3, ...])}} -- times
	in seconds. A channel missing from a bone's dict gets no track at all
	(CBone.compute() already falls back to default_pos/default_rot_quat/
	default_scale for any bone with no track, same as an entirely
	unanimated bone).

	`track_format`:
	- "sampled" (default): CTrackSampledVector/Quat, resampled onto a fixed
	  30Hz grid (see _sample_track()), collapsing to CTrackDefaultVector/Quat
	  where the data is nearly constant. Real ryzom-data .anim files
	  observed so far (2026-09-05) never actually use this format -- see
	  anim_write.md's own note -- but it's what NL3D::CAnimationOptimizer
	  produces and the engine reads it correctly.
	- "keyframer_linear": CTrackKeyFramerLinearVector/Quat, the source's own
	  keys kept as-is (no resampling) -- the format the real 3dsMax exporter
	  itself actually writes, and what every real ryzom-data .anim sampled
	  so far uses.

	Doesn't serialize anything itself -- returns an in-memory Animation
	object, symmetric with parse_animation(); see dumps_animation() for the
	binary encoding step."""
	builder = _TRACK_BUILDERS.get(track_format)
	if builder is None:
		raise AnimationWriteError(f"unknown track_format {track_format!r} (expected one of {sorted(_TRACK_BUILDERS)})")

	id_by_name: Dict[str, int] = {}
	tracks: List[Optional[Track]] = []

	for bone_name, channels in node_tracks.items():
		for channel_name, is_quat in (("pos", False), ("rotquat", True), ("scale", False)):
			if channel_name not in channels:
				continue
			times, values = channels[channel_name]
			if not times:
				continue
			track = builder(times, values, is_quat, loop_mode)
			id_by_name[f"{bone_name}.{channel_name}"] = len(tracks)
			tracks.append(track)

	return Animation(name=name, id_by_name=id_by_name, tracks=tracks, min_end_time=None, sss_shapes=None)


def dumps_animation(anim: Animation) -> bytes:
	"""Serialize an Animation back to the .anim binary format. Only
	CTrackSampledVector/CTrackSampledQuat/CTrackDefaultVector/
	CTrackDefaultQuat/CTrackKeyFramerLinearVector/CTrackKeyFramerLinearQuat
	tracks can be written (see
	project-todos/pynel/anim_write.md) -- a track of another class_name
	raises AnimationWriteError. Always writes the latest CAnimation format
	(version 2, min_end_time + sss_shapes both present -- sss_shapes empty,
	nothing in this module ever populates it)."""
	f = _Writer()
	f.write_magic(MAGIC)
	f.version(2)
	f.string(anim.name)

	f.cont_len(len(anim.id_by_name))
	for key, value in anim.id_by_name.items():
		f.string(key)
		f.u32(value)

	f.cont_len(len(anim.tracks))
	for track in anim.tracks:
		class_name = track.class_name if track is not None else None
		f.write_poly_ptr(track, class_name, _write_track)

	f.f32(anim.min_end_time if anim.min_end_time is not None else animation_duration(anim))
	f.string_vector(anim.sss_shapes or [])

	return f.getvalue()


def save_animation(path: Union[str, Path, BinaryIO], anim: Animation) -> None:
	"""Serialize and write an Animation to a .anim file at path (or an open
	binary file object)."""
	data = dumps_animation(anim)
	if isinstance(path, (str, Path)):
		Path(path).write_bytes(data)
	else:
		path.write(data)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def _dump(anim: Animation) -> None:
	print(f"name: {anim.name!r}")
	print(f"tracks: {len(anim.tracks)}")
	if anim.min_end_time is not None:
		print(f"min_end_time: {anim.min_end_time}")
	if anim.sss_shapes:
		print(f"sss_shapes: {anim.sss_shapes}")
	for track_name, index in sorted(anim.id_by_name.items(), key=lambda kv: kv[1]):
		track = anim.tracks[index] if 0 <= index < len(anim.tracks) else None
		if track is None:
			print(f"  [{index}] {track_name}: (null track)")
			continue
		if track.keys is not None:
			print(f"  [{index}] {track_name}: {track.class_name}, {len(track.keys)} keys, "
			      f"{track.begin_time:.3f}-{track.end_time:.3f}s")
		elif track.keyframes is not None:
			print(f"  [{index}] {track_name}: {track.class_name}, {len(track.keyframes)} keyframes, "
			      f"{track.begin_time:.3f}-{track.end_time:.3f}s")
		else:
			print(f"  [{index}] {track_name}: {track.class_name}, constant value = {track.value}")


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Read Ryzom .anim files (skeleton animation clips)")
	sub = parser.add_subparsers(dest="command", required=True)

	p_dump = sub.add_parser("dump", help="print a summary of a .anim file")
	p_dump.add_argument("path", type=Path)

	p_eval = sub.add_parser("eval", help="evaluate one track at a given time")
	p_eval.add_argument("path", type=Path)
	p_eval.add_argument("track_name")
	p_eval.add_argument("time", type=float)

	p_pose = sub.add_parser("pose", help="evaluate a bone's world matrix at a given time")
	p_pose.add_argument("skel_path", type=Path)
	p_pose.add_argument("anim_path", type=Path)
	p_pose.add_argument("bone_name")
	p_pose.add_argument("time", type=float)

	return parser


def _main() -> None:
	args = _build_arg_parser().parse_args()

	if args.command == "pose":
		from pynel.ryzom_shape import ShapeParseError, load_shape

		try:
			skeleton = load_shape(args.skel_path).value
			anim = load_animation(args.anim_path)
		except (ShapeParseError, AnimationParseError) as exc:
			raise SystemExit(f"cannot parse: {exc}")
		m = evaluate_bone_world_matrix(skeleton, args.bone_name, anim, args.time)
		for row in m:
			print("  ".join(f"{v:10.4f}" for v in row))
		return

	try:
		anim = load_animation(args.path)
	except AnimationParseError as exc:
		raise SystemExit(f"cannot parse {args.path}: {exc}")

	if args.command == "dump":
		_dump(anim)
	elif args.command == "eval":
		index = anim.id_by_name.get(args.track_name)
		if index is None:
			raise SystemExit(f"no such track: {args.track_name!r}")
		track = anim.tracks[index]
		if track is None:
			raise SystemExit(f"track {args.track_name!r} is null")
		print(evaluate_track(track, args.time))


if __name__ == "__main__":
	_main()
