/**
 * \file track_build.cpp
 * \brief See track_build.h.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "track_build.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include <nel/3d/animation.h>
#include <nel/3d/track_keyframer.h>
#include <nel/misc/rgba.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>

#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/reference_maker.h"

using namespace PIPELINE::MAX::BUILTIN;

namespace TRACKBUILD {

// Max Interval sentinels
static const sint32 TIME_NEG_INFINITY = (sint32)0x80000000;
static const sint32 TIME_POS_INFINITY = 0x7fffffff;

static inline float convertTime(sint32 ticks)
{
	return (float)ticks / TICKS_PER_SECOND;
}

static inline bool bezKeyStep(uint32 flags)
{
	return ((flags >> 10) & 7) == 2;
}

// Max ScaleValue -> NeL scale vector (Inverse(srtm)*stm*srtm diagonal, Max row-vector order).
struct Mat3f { float m[3][3]; };

static Mat3f quatMakeMatrix(const float q[4])
{
	float x = q[0], y = q[1], z = q[2], w = q[3];
	float xx = x * x, yy = y * y, zz = z * z;
	float xy = x * y, xz = x * z, yz = y * z;
	float wx = w * x, wy = w * y, wz = w * z;
	Mat3f r;
	r.m[0][0] = 1.0f - 2.0f * (yy + zz); r.m[0][1] = 2.0f * (xy + wz);        r.m[0][2] = 2.0f * (xz - wy);
	r.m[1][0] = 2.0f * (xy - wz);        r.m[1][1] = 1.0f - 2.0f * (xx + zz); r.m[1][2] = 2.0f * (yz + wx);
	r.m[2][0] = 2.0f * (xz + wy);        r.m[2][1] = 2.0f * (yz - wx);        r.m[2][2] = 1.0f - 2.0f * (xx + yy);
	return r;
}

static Mat3f mat3Mul(const Mat3f &a, const Mat3f &b)
{
	Mat3f r;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j];
	return r;
}

static Mat3f mat3Inverse(const Mat3f &a)
{
	Mat3f r;
	r.m[0][0] = a.m[1][1] * a.m[2][2] - a.m[1][2] * a.m[2][1];
	r.m[0][1] = a.m[0][2] * a.m[2][1] - a.m[0][1] * a.m[2][2];
	r.m[0][2] = a.m[0][1] * a.m[1][2] - a.m[0][2] * a.m[1][1];
	r.m[1][0] = a.m[1][2] * a.m[2][0] - a.m[1][0] * a.m[2][2];
	r.m[1][1] = a.m[0][0] * a.m[2][2] - a.m[0][2] * a.m[2][0];
	r.m[1][2] = a.m[0][2] * a.m[1][0] - a.m[0][0] * a.m[1][2];
	r.m[2][0] = a.m[1][0] * a.m[2][1] - a.m[1][1] * a.m[2][0];
	r.m[2][1] = a.m[0][1] * a.m[2][0] - a.m[0][0] * a.m[2][1];
	r.m[2][2] = a.m[0][0] * a.m[1][1] - a.m[0][1] * a.m[1][0];
	float det = a.m[0][0] * r.m[0][0] + a.m[0][1] * r.m[1][0] + a.m[0][2] * r.m[2][0];
	float ooDet = 1.0f / det;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			r.m[i][j] *= ooDet;
	return r;
}

static NLMISC::CVector maxScaleValueToNel(const float s[3], const float q[4])
{
	if (q[0] == 0.0f && q[1] == 0.0f && q[2] == 0.0f && (q[3] == 1.0f || q[3] == -1.0f))
		return NLMISC::CVector(s[0], s[1], s[2]);
	if (std::fabs(q[0]) == 0.0f && std::fabs(q[1]) == 0.0f && std::fabs(q[2]) == 0.0f)
		return NLMISC::CVector(s[0], s[1], s[2]);
	Mat3f srtm = quatMakeMatrix(q);
	Mat3f stm = { { { s[0], 0.0f, 0.0f }, { 0.0f, s[1], 0.0f }, { 0.0f, 0.0f, s[2] } } };
	Mat3f mat = mat3Mul(mat3Mul(srtm, stm), mat3Inverse(srtm));
	return NLMISC::CVector(mat.m[0][0], mat.m[1][1], mat.m[2][2]);
}

template <typename TTrack>
static void applyRange(TTrack *track, bool hasRange, sint32 rangeStart, sint32 rangeEnd,
                       float firstKey, float lastKey, bool loopMode)
{
	bool valid = hasRange;
	if (valid && rangeStart == TIME_NEG_INFINITY && rangeEnd == TIME_POS_INFINITY) valid = false;
	if (valid && rangeStart == TIME_NEG_INFINITY && rangeEnd == TIME_NEG_INFINITY) valid = false;
	if (valid)
		track->unlockRange(convertTime(rangeStart), convertTime(rangeEnd));
	else
		track->unlockRange(firstKey, lastKey);
	// ORT_LOOP → true; ORT_CONSTANT/CYCLE/default → false (export_anim.cpp createKeyFramer).
	// Full ORT bit decode from the controller is still open (§10b); typeColor light tracks
	// in the corpus all need loop=true (single residual byte when forced false).
	track->setLoopMode(loopMode);
}

// Provisional loop-mode rule until the controller ORT storage is fully decoded:
// color tracks (light-group LightmapController) loop; everything else constant.
static inline bool defaultLoopMode(TNelValueType type)
{
	return type == typeColor;
}

// Helper: Bezier Point3 keys → BezierVector (pos/scale) or BezierRGBA (color).
static NL3D::ITrack *buildBezPoint3Track(const CStorageBezPoint3Key *keys, uint numKeys,
                                         bool hasRange, sint32 rs, sint32 re, TNelValueType type)
{
	float firstKey = 0.0f, lastKey = 0.0f;
	if (type == typeColor)
	{
		NL3D::CTrackKeyFramerBezierRGBA *track = new NL3D::CTrackKeyFramerBezierRGBA();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyBezierVector k;
			k.Value.set(keys[i].Val[0], keys[i].Val[1], keys[i].Val[2]);
			k.InTan.set(TICKS_PER_SECOND * keys[i].InTan[0], TICKS_PER_SECOND * keys[i].InTan[1], TICKS_PER_SECOND * keys[i].InTan[2]);
			k.OutTan.set(TICKS_PER_SECOND * keys[i].OutTan[0], TICKS_PER_SECOND * keys[i].OutTan[1], TICKS_PER_SECOND * keys[i].OutTan[2]);
			k.Step = bezKeyStep(keys[i].Flags);
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rs, re, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}
	if (type != typePos && type != typeScale) return nullptr;
	NL3D::CTrackKeyFramerBezierVector *track = new NL3D::CTrackKeyFramerBezierVector();
	for (uint i = 0; i < numKeys; ++i)
	{
		float t = convertTime(keys[i].Time);
		if (i == 0) firstKey = t;
		lastKey = t;
		NL3D::CKeyBezierVector k;
		k.Value.set(keys[i].Val[0], keys[i].Val[1], keys[i].Val[2]);
		k.InTan.set(TICKS_PER_SECOND * keys[i].InTan[0], TICKS_PER_SECOND * keys[i].InTan[1], TICKS_PER_SECOND * keys[i].InTan[2]);
		k.OutTan.set(TICKS_PER_SECOND * keys[i].OutTan[0], TICKS_PER_SECOND * keys[i].OutTan[1], TICKS_PER_SECOND * keys[i].OutTan[2]);
		k.Step = bezKeyStep(keys[i].Flags);
		track->addKey(k, t);
	}
	applyRange(track, hasRange, rs, re, firstKey, lastKey, defaultLoopMode(type));
	return track;
}

static NL3D::ITrack *buildLinPoint3Track(const CStorageLinPoint3Key *keys, uint numKeys,
                                         bool hasRange, sint32 rs, sint32 re, TNelValueType type)
{
	float firstKey = 0.0f, lastKey = 0.0f;
	if (type == typeColor)
	{
		// Reference: R/G/B = (uint8)val — Max Linear Point3 color keys store 0..255 floats
		// (Bezier path stores 0..1 and converts at eval via copyToValue *255).
		NL3D::CTrackKeyFramerLinearRGBA *track = new NL3D::CTrackKeyFramerLinearRGBA();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyRGBA k;
			k.Value.R = (uint8)keys[i].Val[0];
			k.Value.G = (uint8)keys[i].Val[1];
			k.Value.B = (uint8)keys[i].Val[2];
			k.Value.A = 255;
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rs, re, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}
	if (type != typePos && type != typeScale) return nullptr;
	NL3D::CTrackKeyFramerLinearVector *track = new NL3D::CTrackKeyFramerLinearVector();
	for (uint i = 0; i < numKeys; ++i)
	{
		float t = convertTime(keys[i].Time);
		if (i == 0) firstKey = t;
		lastKey = t;
		NL3D::CKeyVector k;
		k.Value.set(keys[i].Val[0], keys[i].Val[1], keys[i].Val[2]);
		track->addKey(k, t);
	}
	applyRange(track, hasRange, rs, re, firstKey, lastKey, defaultLoopMode(type));
	return track;
}

static NL3D::ITrack *buildTCBPoint3Track(const CStorageTCBPoint3Key *keys, uint numKeys,
                                         bool hasRange, sint32 rs, sint32 re, TNelValueType type)
{
	float firstKey = 0.0f, lastKey = 0.0f;
	if (type == typeColor)
	{
		NL3D::CTrackKeyFramerTCBRGBA *track = new NL3D::CTrackKeyFramerTCBRGBA();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyTCBVector k;
			k.Value.set(keys[i].Val[0], keys[i].Val[1], keys[i].Val[2]);
			k.Tension = keys[i].Tens;
			k.Continuity = keys[i].Cont;
			k.Bias = keys[i].Bias;
			k.EaseTo = keys[i].EaseIn;
			k.EaseFrom = keys[i].EaseOut;
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rs, re, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}
	if (type != typePos && type != typeScale) return nullptr;
	NL3D::CTrackKeyFramerTCBVector *track = new NL3D::CTrackKeyFramerTCBVector();
	for (uint i = 0; i < numKeys; ++i)
	{
		float t = convertTime(keys[i].Time);
		if (i == 0) firstKey = t;
		lastKey = t;
		NL3D::CKeyTCBVector k;
		k.Value.set(keys[i].Val[0], keys[i].Val[1], keys[i].Val[2]);
		k.Tension = keys[i].Tens;
		k.Continuity = keys[i].Cont;
		k.Bias = keys[i].Bias;
		k.EaseTo = keys[i].EaseIn;
		k.EaseFrom = keys[i].EaseOut;
		track->addKey(k, t);
	}
	applyRange(track, hasRange, rs, re, firstKey, lastKey, defaultLoopMode(type));
	return track;
}

NL3D::ITrack *buildATrack(CReferenceMaker *ctrl, TNelValueType type)
{
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (!kf) return nullptr;
	uint numKeys = kf->keyCount();
	if (!numKeys) return nullptr;

	sint32 rangeStart = 0, rangeEnd = 0;
	bool hasRange = kf->range(rangeStart, rangeEnd);
	float firstKey = 0.0f, lastKey = 0.0f;

	// Linear Position / Point3
	if (CControlPosLinear *c = dynamic_cast<CControlPosLinear *>(kf))
		return buildLinPoint3Track(c->keys(), numKeys, hasRange, rangeStart, rangeEnd, type);

	// Linear Rotation
	if (CControlRotLinear *c = dynamic_cast<CControlRotLinear *>(kf))
	{
		if (type != typeRotation) return nullptr;
		NL3D::CTrackKeyFramerLinearQuat *track = new NL3D::CTrackKeyFramerLinearQuat();
		const CStorageLinRotKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyQuat k;
			k.Value.set(keys[i].Quat[0], keys[i].Quat[1], keys[i].Quat[2], -keys[i].Quat[3]);
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	// Linear Scale
	if (CControlScaleLinear *c = dynamic_cast<CControlScaleLinear *>(kf))
	{
		if (type != typePos && type != typeScale) return nullptr;
		NL3D::CTrackKeyFramerLinearVector *track = new NL3D::CTrackKeyFramerLinearVector();
		const CStorageLinScaleKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyVector k;
			k.Value = maxScaleValueToNel(keys[i].S, keys[i].Q);
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	// Bezier Position / Point3 / Color
	if (CControlPosBezier *c = dynamic_cast<CControlPosBezier *>(kf))
		return buildBezPoint3Track(c->keys(), numKeys, hasRange, rangeStart, rangeEnd, type);
	if (CControlPoint3Bezier *c = dynamic_cast<CControlPoint3Bezier *>(kf))
		return buildBezPoint3Track(c->keys(), numKeys, hasRange, rangeStart, rangeEnd, type);
	if (CControlColorBezier *c = dynamic_cast<CControlColorBezier *>(kf))
		return buildBezPoint3Track(c->keys(), numKeys, hasRange, rangeStart, rangeEnd, type);

	// Bezier Scale
	if (CControlScaleBezier *c = dynamic_cast<CControlScaleBezier *>(kf))
	{
		if (type != typePos && type != typeScale) return nullptr;
		NL3D::CTrackKeyFramerBezierVector *track = new NL3D::CTrackKeyFramerBezierVector();
		const CStorageBezScaleKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyBezierVector k;
			k.Value = maxScaleValueToNel(keys[i].S, keys[i].Q);
			k.InTan.set(keys[i].InTan[0], keys[i].InTan[1], keys[i].InTan[2]);
			k.OutTan.set(keys[i].OutTan[0], keys[i].OutTan[1], keys[i].OutTan[2]);
			k.Step = bezKeyStep(keys[i].Flags);
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	// Bezier Float
	if (CControlFloatBezier *c = dynamic_cast<CControlFloatBezier *>(kf))
	{
		if (type != typeFloat) return nullptr;
		NL3D::CTrackKeyFramerBezierFloat *track = new NL3D::CTrackKeyFramerBezierFloat();
		const CStorageBezFloatKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyBezierFloat k;
			k.Value = keys[i].Val;
			k.InTan = keys[i].InTan;
			k.OutTan = keys[i].OutTan;
			k.Step = bezKeyStep(keys[i].Flags);
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	// Linear Float
	if (CControlFloatLinear *c = dynamic_cast<CControlFloatLinear *>(kf))
	{
		if (type != typeFloat) return nullptr;
		NL3D::CTrackKeyFramerLinearFloat *track = new NL3D::CTrackKeyFramerLinearFloat();
		const CStorageLinFloatKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyFloat k;
			k.Value = keys[i].Val;
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	// TCB Position / Point3
	if (CControlPosTCB *c = dynamic_cast<CControlPosTCB *>(kf))
		return buildTCBPoint3Track(c->keys(), numKeys, hasRange, rangeStart, rangeEnd, type);
	if (CControlPoint3TCB *c = dynamic_cast<CControlPoint3TCB *>(kf))
		return buildTCBPoint3Track(c->keys(), numKeys, hasRange, rangeStart, rangeEnd, type);

	// TCB Rotation
	if (CControlRotTCB *c = dynamic_cast<CControlRotTCB *>(kf))
	{
		if (type != typeRotation) return nullptr;
		NL3D::CTrackKeyFramerTCBQuat *track = new NL3D::CTrackKeyFramerTCBQuat();
		const CStorageTCBRotKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyTCBQuat k;
			if (keys[i].Flags & 0x10)
			{
				double ax = keys[i].AbsQuat[0], ay = keys[i].AbsQuat[1], az = keys[i].AbsQuat[2];
				double aw = keys[i].AbsQuat[3];
				if (aw < 0.0) { ax = -ax; ay = -ay; az = -az; aw = -aw; }
				double n = sqrt(ax * ax + ay * ay + az * az);
				if (n > 0.0)
				{
					k.Value.Axis.set((float)(ax / n), (float)(ay / n), (float)(az / n));
					double angle;
					if (keys[i].AbsQuat[3] < 0.0f) angle = 2.0 * acos(std::min(1.0, aw));
					else angle = 2.0 * asin(std::min(n, 1.0));
					k.Value.Angle = -(float)angle;
				}
				else
				{
					k.Value.Axis.set(1.0f, 0.0f, 0.0f);
					k.Value.Angle = -0.0f;
				}
			}
			else
			{
				double ax = keys[i].Axis[0], ay = keys[i].Axis[1], az = keys[i].Axis[2];
				double n = sqrt(ax * ax + ay * ay + az * az);
				if (n > 0.0)
					k.Value.Axis.set((float)(ax / n), (float)(ay / n), (float)(az / n));
				else
					k.Value.Axis.set(keys[i].Axis[0], keys[i].Axis[1], keys[i].Axis[2]);
				k.Value.Angle = -keys[i].Angle;
			}
			k.Tension = keys[i].Tens;
			k.Continuity = keys[i].Cont;
			k.Bias = keys[i].Bias;
			k.EaseTo = keys[i].EaseIn;
			k.EaseFrom = keys[i].EaseOut;
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	// TCB Scale
	if (CControlScaleTCB *c = dynamic_cast<CControlScaleTCB *>(kf))
	{
		if (type != typePos && type != typeScale) return nullptr;
		NL3D::CTrackKeyFramerTCBVector *track = new NL3D::CTrackKeyFramerTCBVector();
		const CStorageTCBScaleKey *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyTCBVector k;
			k.Value = maxScaleValueToNel(keys[i].S, keys[i].Q);
			k.Tension = keys[i].Tens;
			k.Continuity = keys[i].Cont;
			k.Bias = keys[i].Bias;
			k.EaseTo = keys[i].EaseIn;
			k.EaseFrom = keys[i].EaseOut;
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey, defaultLoopMode(type));
		return track;
	}

	return nullptr;
}

void addTrackChecked(NL3D::CAnimation &animation, const std::string &name, NL3D::ITrack *track)
{
	if (!track) return;
	if (animation.getTrackByName(name.c_str()))
	{
		delete track;
		return;
	}
	animation.addTrack(name, track);
}

} /* namespace TRACKBUILD */

/* end of file */
