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

Only the two track kinds above are supported; any other ITrack subclass
(e.g. CTrackKeyframer) fails with a clear AnimationParseError rather than
silently producing wrong data. Read-only -- there is no writer yet.

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

MAGIC = b"NEL_ANIM"  # on-disk bytes for NELID("_LEN") + NELID("MINA"), see CAnimation::serial


class AnimationParseError(Exception):
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
class Keyframe:
	time: float
	value: Union[float, Vector3, Quaternion]


@dataclass
class Track:
	"""A single animated channel (e.g. "Bip01 Head.rotquat", see
	Animation.id_by_name). `class_name` says which of the three supported
	kinds this is:
	- CTrackDefaultVector/CTrackDefaultQuat: a constant value the whole
	  clip -- only `value` is set.
	- CTrackSampledVector/CTrackSampledQuat: real keyframed data, pre-sampled
	  at a fixed rate -- `keys` (already decompressed/unpacked to plain
	  Vector3/Quaternion, see CQuatPack::unpack) plus the
	  CTrackSampledCommon timing fields; `time_blocks` groups them for fast
	  time lookup rather than storing an explicit time per key.
	- CTrackKeyFramerLinearVector/CTrackKeyFramerLinearQuat: real keyframed
	  data, irregularly spaced -- `keyframes` (explicit (time, value) pairs).
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
	keyframes: Optional[List[Keyframe]] = None  # CTrackKeyFramerLinear* only
	range_lock: Optional[bool] = None  # CTrackKeyFramerLinear* only


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


# ---------------------------------------------------------------------------
# Track evaluation
# ---------------------------------------------------------------------------


def _clamp(x: float, lo: float, hi: float) -> float:
	return lo if x < lo else hi if x > hi else x


def _slerp(a: Quaternion, b: Quaternion, t: float) -> Quaternion:
	ax, ay, az, aw = a.x, a.y, a.z, a.w
	bx, by, bz, bw = b.x, b.y, b.z, b.w
	dot = ax * bx + ay * by + az * bz + aw * bw
	if dot < 0.0:
		bx, by, bz, bw, dot = -bx, -by, -bz, -bw, -dot
	dot = _clamp(dot, -1.0, 1.0)
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


def _evaluate_keyframer(track: Track, time: float):
	keyframes = track.keyframes
	if not keyframes:
		raise ValueError(f"track {track.class_name!r} has no keyframes")
	if len(keyframes) == 1:
		return keyframes[0].value

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
		return keyframes[0].value
	if idx >= len(keyframes) - 1:
		if track.loop_mode and total_range > 0:
			# ITrackKeyFramer::eval(): looping past the last key blends back
			# toward the first one (dateNext == loop_end), unlike sampled tracks.
			t0 = times[idx]
			frac = 0.0 if loop_end <= t0 else _clamp((date - t0) / (loop_end - t0), 0.0, 1.0)
			return _lerp_value(keyframes[idx].value, keyframes[0].value, frac)
		return keyframes[-1].value
	t0, t1 = times[idx], times[idx + 1]
	frac = 0.0 if t1 <= t0 else _clamp((date - t0) / (t1 - t0), 0.0, 1.0)
	return _lerp_value(keyframes[idx].value, keyframes[idx + 1].value, frac)


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


def _bone_local_matrix(bone, anim: Optional[Animation], time: float) -> Tuple[tuple, Vector3]:
	"""The bone's local transform at `time` (and the Scale value used to build
	it, needed by the caller for CBone::compute()'s UnheritScale compensation
	below), animated where `anim` has a track for it (looked up as
	"{bone.name}.pos"/".rotquat"/".scale") and falling back to the bone's own
	default_* from the .skel otherwise. Matches NLMISC::CTransformable's
	`Local = T(Pos+Pivot) * R * S * T(-Pivot)`."""
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
