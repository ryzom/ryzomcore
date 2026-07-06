// Anim export: .max -> .anim, replicating the NelExportAnimation path of the 3ds Max plugin
// (build_gamedata processes/anim) without 3ds Max, for non-biped rigs.
//
// Node selection replicates anim_export.ms: the node named "Bip01" (the database convention —
// non-biped animation rigs use a plain Box named Bip01 as the animated root precisely so the
// export script picks it up) plus every node whose NEL3D_APPDATA_EXPORT_NODE_ANIMATION AppData
// equals "1", in scene order. Each selected node contributes its own tracks under the bare
// value names ("pos"/"rotquat"/"scale", first-wins on collision) and every descendant's tracks
// under "<nodeName>.<valueName>" (flat, not nested), matching CExportNel::addAnimation /
// addNodeTracks / addBoneTracks.
//
// Track data comes from the typed keyframe controller classes in pipeline_max
// (builtin/control_keyframer.h); the NL3D track objects are built with the same key
// conversions as plugin_max/nel_mesh_lib/export_anim.cpp (buildATrack/createKeyFramer/
// buildNelKey) and serialized through the real NL3D CAnimation::serial, so the output format
// is exact by construction.
//
// Biped rigs (a real Vertical/Horizontal/Turn COM controller in the scene) go through the
// biped path: the figure rig is reconstructed via pipeline_max_rig, the animation keytracks on
// the Biped (0x9155) system object are decoded and TCB-evaluated (see biped_anim.h), and every
// biped node is oversampled once per frame across the union key range into LinearQuat/
// LinearVector tracks — replicating CExportNel::addBipedNodeTracks + overSampleBipedAnimation
// (NL3D_BIPED_OVERSAMPLING=30 at 30 fps == one sample per frame). Position tracks are emitted
// for the COM and the biped.getNode(#larm/#rarm/#spine/#tail) first links (clavicles, spine
// base, tail base), like the reference's mustExportBipedBonePos set.

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>

#include <nel/3d/animation.h>
#include <nel/3d/track_keyframer.h>
#include <nel/3d/transformable.h>
#include <nel/3d/register_3d.h>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../pipeline_max/storage_stream.h"
#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/dll_directory.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class_registry.h"

#include "../pipeline_max/builtin/builtin.h"
#include "../pipeline_max/update1/update1.h"
#include "../pipeline_max/epoly/epoly.h"
#include "../pipeline_max/biped/biped.h"

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"

#include "../pipeline_max_rig/biped_rig.h"
#include "../pipeline_max/biped/biped_driven.h"
#include "biped_anim.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

// Max time constants: 4800 ticks per second, 30 fps / 160 ticks per frame in every corpus
// scene. CExportNel::convertTime divides by GetTicksPerFrame()*GetFrameRate() = 4800.
static const float TICKS_PER_SECOND = 4800.0f;
// Max Interval sentinels (TIME_NegInfinity / TIME_PosInfinity)
static const sint32 TIME_NEG_INFINITY = (sint32)0x80000000;
static const sint32 TIME_POS_INFINITY = 0x7fffffff;

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
#define NEL3D_APPDATA_INSTANCE_NAME 1423062562
#define NEL3D_APPDATA_EXPORT_NODE_ANIMATION 1423062800
#define NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME 1423062801

static const NLMISC::CClassId CLASSID_PRS_CTRL(0x00002005, 0x00000000);
static const NLMISC::CClassId CLASSID_BIPED_VHT_CTRL(0x00009156, 0x00000000);

static inline float convertTime(sint32 ticks)
{
	return (float)ticks / TICKS_PER_SECOND;
}

// Read a string-valued NeL AppData script entry off a node (SubId match, same convention as
// pipeline_max_export_skel). Returns empty string when absent.
static std::string getNodeScriptAppDataString(INode *node, uint32 subId)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(node);
	if (!n) return std::string();
	PIPELINE::MAX::BUILTIN::STORAGE::CAppData *ad = n->appData();
	if (!ad) return std::string();
	for (auto it = ad->entries().begin(); it != ad->entries().end(); ++it)
	{
		if (it->first.SubId != subId) continue;
		PIPELINE::MAX::BUILTIN::STORAGE::CAppDataEntry *entry = it->second;
		CStorageRaw *raw = entry->value<CStorageRaw>();
		if (!raw) return std::string();
		std::string s(raw->Value.begin(), raw->Value.end());
		while (!s.empty() && (s[s.size() - 1] == '\0' || s[s.size() - 1] == '\n' || s[s.size() - 1] == '\r')) s.resize(s.size() - 1);
		return s;
	}
	return std::string();
}

// ---------------------------------------------------------------------------------------------
// Max ScaleValue -> NeL scale vector, replicating CExportNel::buildNelKey for ILinScaleKey /
// IBezScaleKey / ITCBScaleKey: srtm = q.MakeMatrix(); mat = Inverse(srtm) * ScaleMatrix(s) *
// srtm; value = diagonal of mat. All in float, mimicking the Max SDK float operation order.
// For the identity quat this reduces to s bit-exactly (multiplications by exact 1.0/0.0).

struct Mat3f
{
	float m[3][3]; // row-major, row vectors (Max convention)
};

// Max Quat::MakeMatrix (row-vector convention)
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

// Matrix3 inverse via classic adjugate/determinant (3x3 part; Max's Matrix3 carries a
// translation row which is identity here).
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
	// Fast path shared with the reference math: identity quat leaves diag(s) exact.
	if (q[0] == 0.0f && q[1] == 0.0f && q[2] == 0.0f && (q[3] == 1.0f || q[3] == -1.0f))
		return NLMISC::CVector(s[0], s[1], s[2]);
	// Also the encoded -0 variants
	if (std::fabs(q[0]) == 0.0f && std::fabs(q[1]) == 0.0f && std::fabs(q[2]) == 0.0f)
		return NLMISC::CVector(s[0], s[1], s[2]);
	Mat3f srtm = quatMakeMatrix(q);
	Mat3f stm = { { { s[0], 0.0f, 0.0f }, { 0.0f, s[1], 0.0f }, { 0.0f, 0.0f, s[2] } } };
	// The reference code is `Inverse(srtm) * stm * srtm` with Max's Matrix3 operator*, which
	// composes left-to-right for row vectors — in this column-style convention that is
	// srtm * stm * Inverse(srtm). (Validated against the optimized fauna references: the
	// other order is off by ~3e-4 on animated non-identity-q scale keys, this one ~1e-6.)
	Mat3f mat = mat3Mul(mat3Mul(srtm, stm), mat3Inverse(srtm));
	return NLMISC::CVector(mat.m[0][0], mat.m[1][1], mat.m[2][2]);
}

// ---------------------------------------------------------------------------------------------
// Track building, replicating createKeyFramer's range/loop handling.

// Apply range/loop to a keyframer track. hasRange==false or a NEVER/FOREVER interval falls back
// to the first/last key times, like the reference (`(FOREVER==range)||(NEVER==range)`).
template <typename TTrack>
static void applyRange(TTrack *track, bool hasRange, sint32 rangeStart, sint32 rangeEnd, float firstKey, float lastKey)
{
	bool valid = hasRange;
	if (valid && rangeStart == TIME_NEG_INFINITY && rangeEnd == TIME_POS_INFINITY) valid = false; // FOREVER
	if (valid && rangeStart == TIME_NEG_INFINITY && rangeEnd == TIME_NEG_INFINITY) valid = false; // NEVER
	if (valid)
		track->unlockRange(convertTime(rangeStart), convertTime(rangeEnd));
	else
		track->unlockRange(firstKey, lastKey);
	// GetORT(ORT_AFTER): no anim in the non-biped corpus uses ORT_LOOP, and the storage bit for
	// the ORT is not located yet (see pipeline_max_design.md §10b) — loop mode stays false.
	track->setLoopMode(false);
}

// Max Bezier key flags: out tangent type at bits 10..12, BEZKEY_STEP == 2.
static inline bool bezKeyStep(uint32 flags)
{
	return ((flags >> 10) & 7) == 2;
}

enum TNelValueType
{
	typePos,
	typeRotation,
	typeScale
};

// Build a NeL track from a typed controller, or NULL when the controller has no keys or is not
// a supported keyframer for the requested value type. Mirrors CExportNel::buildATrack.
static NL3D::ITrack *buildATrack(CReferenceMaker *ctrl, TNelValueType type)
{
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (!kf) return NULL;
	uint numKeys = kf->keyCount();
	if (!numKeys) return NULL;

	sint32 rangeStart = 0, rangeEnd = 0;
	bool hasRange = kf->range(rangeStart, rangeEnd);

	float firstKey = 0.0f, lastKey = 0.0f;

	// Linear Position -> LinearVector (pos or scale request, like the reference)
	if (CControlPosLinear *c = dynamic_cast<CControlPosLinear *>(kf))
	{
		if (type != typePos && type != typeScale) return NULL;
		NL3D::CTrackKeyFramerLinearVector *track = new NL3D::CTrackKeyFramerLinearVector();
		const CStorageLinPoint3Key *keys = c->keys();
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyVector k;
			k.Value.set(keys[i].Val[0], keys[i].Val[1], keys[i].Val[2]);
			track->addKey(k, t);
		}
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Linear Rotation -> LinearQuat (w negated)
	if (CControlRotLinear *c = dynamic_cast<CControlRotLinear *>(kf))
	{
		if (type != typeRotation) return NULL;
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Linear Scale -> LinearVector via the scale-matrix diagonal
	if (CControlScaleLinear *c = dynamic_cast<CControlScaleLinear *>(kf))
	{
		if (type != typePos && type != typeScale) return NULL;
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Bezier Position -> BezierVector (tangents scaled to per-second)
	if (CControlPosBezier *c = dynamic_cast<CControlPosBezier *>(kf))
	{
		if (type != typePos && type != typeScale) return NULL;
		NL3D::CTrackKeyFramerBezierVector *track = new NL3D::CTrackKeyFramerBezierVector();
		const CStorageBezPoint3Key *keys = c->keys();
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Bezier Scale -> BezierVector via the scale-matrix diagonal (tangents NOT scaled — the
	// reference IBezScaleKey conversion keeps them as-is)
	if (CControlScaleBezier *c = dynamic_cast<CControlScaleBezier *>(kf))
	{
		if (type != typePos && type != typeScale) return NULL;
		NL3D::CTrackKeyFramerBezierVector *track = new NL3D::CTrackKeyFramerBezierVector();
		const CStorageBezScaleKey *keys = c->keys();
		if (getenv("PMB_ANIM_DUMP_BEZSCALE"))
		{
			for (uint i = 0; i < numKeys; ++i)
			{
				fprintf(stderr, "bezscale t=%d f=%08x s=(%.9g,%.9g,%.9g) q=(%.9g,%.9g,%.9g,%.9g) it=(%.9g,%.9g,%.9g) ot=(%.9g,%.9g,%.9g) ex=[", keys[i].Time, keys[i].Flags,
				        keys[i].S[0], keys[i].S[1], keys[i].S[2], keys[i].Q[0], keys[i].Q[1], keys[i].Q[2], keys[i].Q[3],
				        keys[i].InTan[0], keys[i].InTan[1], keys[i].InTan[2], keys[i].OutTan[0], keys[i].OutTan[1], keys[i].OutTan[2]);
				for (int j = 0; j < 22; ++j) fprintf(stderr, "%.9g,", keys[i].Extra[j]);
				fprintf(stderr, "]\n");
			}
		}
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// TCB Position -> TCBVector
	if (CControlPosTCB *c = dynamic_cast<CControlPosTCB *>(kf))
	{
		if (type != typePos && type != typeScale) return NULL;
		NL3D::CTrackKeyFramerTCBVector *track = new NL3D::CTrackKeyFramerTCBVector();
		const CStorageTCBPoint3Key *keys = c->keys();
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// TCB Rotation -> TCBQuat (relative angle-axis, angle negated)
	if (CControlRotTCB *c = dynamic_cast<CControlRotTCB *>(kf))
	{
		if (type != typeRotation) return NULL;
		NL3D::CTrackKeyFramerTCBQuat *track = new NL3D::CTrackKeyFramerTCBQuat();
		const CStorageTCBRotKey *keys = c->keys();
		if (getenv("PMB_ANIM_DUMP_TCBROT"))
		{
			for (uint i = 0; i < numKeys; ++i)
				fprintf(stderr, "tcbrot t=%d f=%08x abs=(%.9g,%.9g,%.9g,%.9g) rel-axis=(%.9g,%.9g,%.9g) angle=%.9g\n",
				        keys[i].Time, keys[i].Flags,
				        keys[i].AbsQuat[0], keys[i].AbsQuat[1], keys[i].AbsQuat[2], keys[i].AbsQuat[3],
				        keys[i].Axis[0], keys[i].Axis[1], keys[i].Axis[2], keys[i].Angle);
		}
		for (uint i = 0; i < numKeys; ++i)
		{
			float t = convertTime(keys[i].Time);
			if (i == 0) firstKey = t;
			lastKey = t;
			NL3D::CKeyTCBQuat k;
			if (keys[i].Flags & 0x10)
			{
				// Key flag 0x10 marks the stored relative angle-axis as a stale cache (observed
				// with garbage axis + zero angle on single-key tracks in pr_mo_phytopsy_attack);
				// Max rederives the value from the absolute quat. Only the key0 derivation is
				// known (relative-to-identity == absolute); the optimized reference confirms the
				// final rotation is the stored absolute quat's conjugate bit-exactly, which this
				// axis/angle reproduces through NeL's eval to within 1 ULP.
				double ax = keys[i].AbsQuat[0], ay = keys[i].AbsQuat[1], az = keys[i].AbsQuat[2];
				double aw = keys[i].AbsQuat[3];
				// Max normalizes the quat to a positive w before deriving (observed on
				// box_arme_gauche in ca_hof_mort: axis = -xyz, angle = 2*acos(-w) for w < 0;
				// the asin path below stays for w >= 0 where it was validated byte-identical).
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
				if (i > 0)
					fprintf(stderr, "WARNING: TCB rot key %u has the stale-cache flag on a non-first key; absolute-quat derivation only validated for key 0\n", i);
			}
			else
			{
				// Max renormalizes the key's axis before GetKey returns it, with the norm
				// computed in double (classic C float->double promotion) and the division
				// rounded per component to float. The stored axis is already unit to within
				// 1-2 ULP, so this is a no-op for most keys, but the reference exports carry
				// the renormalized bits (surfaced by de/la_sky_dome, 1-ULP axis deltas on 2 keys).
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Bezier Float and any other typed controller: no track for PRS value types (the reference
	// only consumes float controllers for camera/material/morph tracks, none of which are in the
	// non-biped anim scope).
	return NULL;
}

// ---------------------------------------------------------------------------------------------
// Node walk, replicating addNodeTracks / addBoneTracks.

static void addTrackChecked(NL3D::CAnimation &animation, const std::string &name, NL3D::ITrack *track)
{
	if (!track) return;
	if (animation.getTrackByName(name.c_str()))
	{
		// First track wins on name collision, like the reference exporter.
		delete track;
		return;
	}
	animation.addTrack(name, track);
}

// The PRS transform's sub-controllers: refs 0/1/2 = position/rotation/scale. Only the plain
// Position/Rotation/Scale transform (0x2005) is handled — matching GetPositionController & co
// on a PRS; other transform types (lists, expressions, biped) have no keyframer sub-controllers
// the reference path would export for non-biped rigs.
static void addNodeTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName)
{
	CReferenceMaker *transform = node.getReference(0);
	if (!transform) return;
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(transform);
	if (!tmsc || tmsc->classDesc()->classId() != CLASSID_PRS_CTRL) return;

	// Export order matches the reference: scale, rotation, position.
	NL3D::ITrack *track = buildATrack(transform->getReference(2), typeScale);
	addTrackChecked(animation, parentName + NL3D::ITransformable::getScaleValueName(), track);

	track = buildATrack(transform->getReference(1), typeRotation);
	addTrackChecked(animation, parentName + NL3D::ITransformable::getRotQuatValueName(), track);

	track = buildATrack(transform->getReference(0), typePos);
	addTrackChecked(animation, parentName + NL3D::ITransformable::getPosValueName(), track);

	// Camera roll/target, object FOV, material, light, particle-system and morph tracks are not
	// exported: none exist in the non-biped anim corpus (no cameras/lights/PS flagged, no
	// morpher modifiers, no NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS).
}

// Ordered children in scene (container) order — INode::children() is pointer-keyed and
// unordered; same approach as pipeline_max_export_skel.
static std::vector<INode *> orderedChildrenOf(INode *parent, CSceneClassContainer *ssc)
{
	std::vector<INode *> out;
	for (auto it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (n && n->parent() == parent) out.push_back(n);
	}
	return out;
}

static void addBoneTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName, CSceneClassContainer *ssc)
{
	// Track names are FLAT: parentName + nodeName + "." for this node's own tracks, but the
	// recursion passes the ORIGINAL parentName down (matching CExportNel::addBoneTracks).
	std::string name = parentName + ucstring(node.userName()).toUtf8() + ".";
	addNodeTracks(animation, node, name);
	std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
	for (INode *child : kids)
		addBoneTracks(animation, *child, parentName, ssc);
}

static void addBipedAnimation(NL3D::CAnimation &animation, INode &node, const std::string &baseName, CSceneClassContainer *ssc);

static void addAnimation(NL3D::CAnimation &animation, INode &node, const std::string &baseName, CSceneClassContainer *ssc)
{
	// Biped COM root: the oversampling path (CExportNel::addBipedNodeTracks).
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node.getReference(0));
	if (tmsc && tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL)
	{
		addBipedAnimation(animation, node, baseName, ssc);
		return;
	}
	// Non-biped path of CExportNel::addAnimation: the node's own tracks under the bare base
	// name, then every child subtree via addBoneTracks. (NoteTrack/SSS/material/morph tracks:
	// not present in the non-biped corpus, see addNodeTracks.)
	addNodeTracks(animation, node, baseName);
	std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
	for (INode *child : kids)
		addBoneTracks(animation, *child, baseName, ssc);
}

// ---------------------------------------------------------------------------------------------
// Biped export path.

struct SBipedSampled
{
	std::vector<sint32> Times; // sample times (ticks)
	std::map<INode *, std::vector<NLMISC::CQuat> > Rot;   // local rotation per sample
	std::map<INode *, std::vector<NLMISC::CVector> > Pos; // local position per sample
	std::map<INode *, std::pair<sint32, sint32> > RotRange; // per-node keytrack span (rot)
	std::map<INode *, std::pair<sint32, sint32> > PosRange;
	std::set<INode *> PosExport; // nodes that get a pos track
};

// Track span of the keytrack a bone id maps to; falls back to the global range.
static bool trackSpan(const BIPANIM::SBipAnimKeys &keys, uint32 id, bool isCom, bool comPos,
                      sint32 &mn, sint32 &mx)
{
	const BIPANIM::SBipKeyTrack *tr = NULL;
	if (isCom)
	{
		if (comPos)
		{
			// COM position: union of horizontal/vertical/turn (the TM controller's range)
			bool have = false;
			const BIPANIM::SBipKeyTrack *all[3] = { &keys.Horizontal, &keys.Vertical, &keys.Turn };
			for (int i = 0; i < 3; ++i)
			{
				if (all[i]->empty()) continue;
				if (!have) { mn = all[i]->Times.front(); mx = all[i]->Times.back(); have = true; }
				else { mn = std::min(mn, all[i]->Times.front()); mx = std::max(mx, all[i]->Times.back()); }
			}
			return have;
		}
		tr = &keys.Turn;
	}
	else switch (id)
	{
	case PMAX_RIG::BID_LARM: case PMAX_RIG::BID_LFINGERS: case PMAX_RIG::BID_LFINGERNUB: tr = &keys.ArmL; break;
	case PMAX_RIG::BID_RARM: case PMAX_RIG::BID_RFINGERS: case PMAX_RIG::BID_RFINGERNUB: tr = &keys.ArmR; break;
	case PMAX_RIG::BID_LLEG: case PMAX_RIG::BID_LTOES: case PMAX_RIG::BID_LTOENUB: tr = &keys.LegL; break;
	case PMAX_RIG::BID_RLEG: case PMAX_RIG::BID_RTOES: case PMAX_RIG::BID_RTOENUB: tr = &keys.LegR; break;
	case PMAX_RIG::BID_SPINE: tr = &keys.Spine; break;
	case PMAX_RIG::BID_TAIL: case PMAX_RIG::BID_TAILNUB: tr = &keys.Tail; break;
	case PMAX_RIG::BID_HEAD: case PMAX_RIG::BID_NECK: case PMAX_RIG::BID_HEADNUB: case PMAX_RIG::BID_NECKNUB: tr = &keys.Head; break;
	case PMAX_RIG::BID_PELVIS: tr = &keys.Pelvis; break;
	case PMAX_RIG::BID_PONY1: case PMAX_RIG::BID_PONY1NUB: tr = &keys.Pony1; break;
	default: tr = NULL; break;
	}
	if (!tr || tr->empty()) return false;
	mn = tr->Times.front();
	mx = tr->Times.back();
	return true;
}

// Sample the whole biped subtree once per frame. Returns false when no biped keys exist.
static bool sampleBipedSubtree(INode &root, CSceneClassContainer *ssc, SBipedSampled &out)
{
	using namespace PMAX_RIG;

	// figure-time walk (fills g_bipedRigs and per-bone figure world transforms)
	g_bipedRigs.clear();
	g_rig = NULL;
	g_msBones.clear();
	std::vector<Bone> bones;
	std::set<std::string> nameSet;
	NLMISC::CMatrix rootMat; rootMat.identity();
	walkNode(&root, -1, rootMat, ssc, bones, nameSet);
	patchFootstepsGround(bones);

	std::map<INode *, size_t> boneOf;
	for (size_t i = 0; i < bones.size(); ++i)
		if (bones[i].Node) boneOf[bones[i].Node] = i;

	// one evaluator per rig
	std::vector<BIPANIM::CBipedAnimEval *> evals;
	bool hasRange = false;
	sint32 rangeMin = 0, rangeMax = 0;
	for (std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.begin(); it != g_bipedRigs.end(); ++it)
	{
		g_rig = &it->second;
		BIPANIM::CBipedAnimEval *ev = new BIPANIM::CBipedAnimEval(it->first, it->second, bones, boneOf);
		evals.push_back(ev);
		if (ev->keys().HasRange)
		{
			if (!hasRange) { rangeMin = ev->keys().RangeMin; rangeMax = ev->keys().RangeMax; hasRange = true; }
			else { rangeMin = std::min(rangeMin, ev->keys().RangeMin); rangeMax = std::max(rangeMax, ev->keys().RangeMax); }
		}
	}
	if (!hasRange)
	{
		for (size_t i = 0; i < evals.size(); ++i) delete evals[i];
		return false;
	}

	// sample times: one per frame (160 ticks) across [min, max], last sample clamped to max —
	// replicating overSampleBipedAnimation's loop shape.
	const sint32 step = 160;
	for (sint32 tt = rangeMin; tt <= rangeMax; tt += step)
	{
		sint32 tc = std::min(tt, rangeMax);
		out.Times.push_back(tc);
		if (tc == rangeMax) break;
	}
	if (out.Times.empty() || out.Times.back() != rangeMax) out.Times.push_back(rangeMax);

	// per-sample evaluation
	std::map<INode *, BIPANIM::SBipNodeState> state;
	for (size_t s = 0; s < out.Times.size(); ++s)
	{
		state.clear();
		for (size_t e = 0; e < evals.size(); ++e)
			evals[e]->evalAt((double)out.Times[s], state);
		// convert to local transforms
		for (std::map<INode *, BIPANIM::SBipNodeState>::iterator it = state.begin(); it != state.end(); ++it)
		{
			INode *node = it->first;
			INode *parent = node->parent();
			BIPANIM::QuatD w = it->second.WorldRot;
			NLMISC::CVectorD wp = it->second.WorldPos;
			BIPANIM::QuatD localR = w;
			NLMISC::CVectorD localP = wp;
			std::map<INode *, BIPANIM::SBipNodeState>::iterator ps = parent ? state.find(parent) : state.end();
			if (ps != state.end())
			{
				localR = BIPANIM::qMul(BIPANIM::qConj(ps->second.WorldRot), w);
				localP = BIPANIM::qRotate(BIPANIM::qConj(ps->second.WorldRot), wp - ps->second.WorldPos);
			}
			localR = BIPANIM::qNorm(localR);
			// The decode formulas already produce NeL-convention rotations (the same values the
			// reference exporter's decompMatrix + w-negation yields) — no further negation.
			NLMISC::CQuat q((float)localR.x, (float)localR.y, (float)localR.z, (float)localR.w);
			out.Rot[node].push_back(q);
			out.Pos[node].push_back(NLMISC::CVector((float)localP.x, (float)localP.y, (float)localP.z));
		}
	}

	// pos-track set + per-node ranges
	for (std::map<INode *, std::vector<NLMISC::CQuat> >::iterator it = out.Rot.begin(); it != out.Rot.end(); ++it)
	{
		INode *node = it->first;
		bool isCom = PMAX_RIG::isBipedComNode(node);
		uint32 id = 0, link = 0;
		bool hasId = PMAX_RIG::readBipDrivenIdLink(node, id, link);
		if (isCom)
			out.PosExport.insert(node);
		else if (hasId && link == 0 && (id == PMAX_RIG::BID_LARM || id == PMAX_RIG::BID_RARM || id == PMAX_RIG::BID_SPINE || id == PMAX_RIG::BID_TAIL))
			out.PosExport.insert(node);
		// default range: the whole sampled span (refined below from the node's keytrack)
		out.RotRange[node] = std::make_pair(out.Times.front(), out.Times.back());
		out.PosRange[node] = std::make_pair(out.Times.front(), out.Times.back());
	}

	// refine per-node ranges using each node's keytrack span
	{
		size_t e = 0;
		for (std::map<CSceneClass *, SBipedRig>::iterator rit = g_bipedRigs.begin(); rit != g_bipedRigs.end(); ++rit, ++e)
		{
			if (e >= evals.size()) break;
			const BIPANIM::SBipAnimKeys &keys = evals[e]->keys();
			for (std::map<INode *, std::vector<NLMISC::CQuat> >::iterator it = out.Rot.begin(); it != out.Rot.end(); ++it)
			{
				INode *node = it->first;
				if (PMAX_RIG::bipedSystemOfCtrl(node->getReference(0)) != rit->first) continue;
				bool isCom = PMAX_RIG::isBipedComNode(node);
				uint32 id = 0, link = 0;
				PMAX_RIG::readBipDrivenIdLink(node, id, link);
				sint32 mn, mx;
				if (trackSpan(keys, id, isCom, false, mn, mx)) out.RotRange[node] = std::make_pair(mn, mx);
				if (trackSpan(keys, id, isCom, true, mn, mx)) out.PosRange[node] = std::make_pair(mn, mx);
			}
		}
	}

	for (size_t i = 0; i < evals.size(); ++i) delete evals[i];
	return true;
}

// Build a LinearQuat track from sampled local rotations (normalize + makeClosest, reference
// createBipedKeyFramer).
static NL3D::ITrack *buildSampledQuatTrack(const std::vector<sint32> &times, const std::vector<NLMISC::CQuat> &vals,
                                           sint32 rangeStart, sint32 rangeEnd)
{
	if (vals.empty()) return NULL;
	NL3D::CTrackKeyFramerLinearQuat *track = new NL3D::CTrackKeyFramerLinearQuat();
	NLMISC::CQuat prev;
	for (size_t i = 0; i < times.size() && i < vals.size(); ++i)
	{
		NLMISC::CQuat q = vals[i];
		q.normalize();
		if (i > 0) q.makeClosest(prev);
		prev = q;
		NL3D::CKeyQuat k;
		k.Value = q;
		track->addKey(k, convertTime(times[i]));
	}
	track->unlockRange(convertTime(rangeStart), convertTime(rangeEnd));
	track->setLoopMode(false);
	return track;
}

static NL3D::ITrack *buildSampledPosTrack(const std::vector<sint32> &times, const std::vector<NLMISC::CVector> &vals,
                                          sint32 rangeStart, sint32 rangeEnd)
{
	if (vals.empty()) return NULL;
	NL3D::CTrackKeyFramerLinearVector *track = new NL3D::CTrackKeyFramerLinearVector();
	for (size_t i = 0; i < times.size() && i < vals.size(); ++i)
	{
		NL3D::CKeyVector k;
		k.Value = vals[i];
		track->addKey(k, convertTime(times[i]));
	}
	track->unlockRange(convertTime(rangeStart), convertTime(rangeEnd));
	track->setLoopMode(false);
	return track;
}

// Replicates CExportNel::addBipedNodeTracks: biped nodes emit their sampled tracks under
// parentName + name + "." (bare for the root), non-biped children go through addBoneTracks.
static void addBipedNodeTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName,
                               bool root, CSceneClassContainer *ssc, const SBipedSampled &sampled)
{
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node.getReference(0));
	bool isBiped = tmsc && (tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL ||
	                        tmsc->classDesc()->classId() == PIPELINE::MAX::BIPED::CBipedDriven::ClassId);
	if (isBiped)
	{
		std::string name = root ? parentName : (parentName + ucstring(node.userName()).toUtf8() + ".");
		// scale: biped nodes have none. rotation:
		std::map<INode *, std::vector<NLMISC::CQuat> >::const_iterator rit = sampled.Rot.find(&node);
		if (rit != sampled.Rot.end())
		{
			std::pair<sint32, sint32> rr = sampled.RotRange.find(&node)->second;
			NL3D::ITrack *track = buildSampledQuatTrack(sampled.Times, rit->second, rr.first, rr.second);
			addTrackChecked(animation, name + NL3D::ITransformable::getRotQuatValueName(), track);
		}
		// position (COM + clavicles/spine/tail bases only):
		if (sampled.PosExport.count(&node))
		{
			std::map<INode *, std::vector<NLMISC::CVector> >::const_iterator pit = sampled.Pos.find(&node);
			if (pit != sampled.Pos.end())
			{
				std::pair<sint32, sint32> pr = sampled.PosRange.find(&node)->second;
				NL3D::ITrack *track = buildSampledPosTrack(sampled.Times, pit->second, pr.first, pr.second);
				addTrackChecked(animation, name + NL3D::ITransformable::getPosValueName(), track);
			}
		}
		std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
		for (INode *child : kids)
			addBipedNodeTracks(animation, *child, parentName, false, ssc, sampled);
	}
	else
	{
		addBoneTracks(animation, node, parentName, ssc);
	}
}

static void addBipedAnimation(NL3D::CAnimation &animation, INode &node, const std::string &baseName, CSceneClassContainer *ssc)
{
	SBipedSampled sampled;
	if (!sampleBipedSubtree(node, ssc, sampled))
	{
		// no biped keys at all: fall back to the non-biped path (nothing to oversample)
		addNodeTracks(animation, node, baseName);
		std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
		for (INode *child : kids)
			addBoneTracks(animation, *child, baseName, ssc);
		return;
	}
	addBipedNodeTracks(animation, node, baseName, true, ssc, sampled);
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "usage: pipeline_max_export_anim <input.max> <output.anim>\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export\n";
		return 1;
	}
	const char *maxFile = argv[1];
	const char *animOut = argv[2];

	gsf_init();
	NL3D::registerSerial3d();

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	GsfInput *src = gsf_input_stdio_new(maxFile, NULL);
	if (!src) { std::cerr << "ERROR: cannot open " << maxFile << "\n"; return 1; }
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	g_object_unref(src);
	if (!in) { std::cerr << "ERROR: not an OLE compound file: " << maxFile << "\n"; return 1; }

	CDllDirectory dll;
	{ GsfInput *s = gsf_infile_child_by_name(in, "DllDirectory"); if (!s) { std::cerr << "ERROR: no DllDirectory stream\n"; return 1; } CStorageStream st(s); dll.serial(st); g_object_unref(s); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ GsfInput *s = gsf_infile_child_by_name(in, "ClassDirectory3"); if (!s) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return 1; } CStorageStream st(s); cd.serial(st); g_object_unref(s); }
	cd.parse(VersionUnknown);

	CScene scene(&reg, &dll, &cd);
	{ GsfInput *s = gsf_infile_child_by_name(in, "Scene"); if (!s) { std::cerr << "ERROR: no Scene stream\n"; return 1; } CStorageStream st(s); scene.serial(st); g_object_unref(s); }
	scene.parse(VersionUnknown);
	g_object_unref(in);

	CSceneClassContainer *ssc = scene.container();

	// --- Build the selection: $Bip01 first (case-insensitive, like MaxScript), then every node
	// with EXPORT_NODE_ANIMATION == "1", in scene order.
	std::vector<INode *> selection;
	std::set<INode *> selected;
	INode *rootNode = ssc->scene()->rootNode();
	INode *bip01 = rootNode ? rootNode->find(ucstring("Bip01")) : NULL;
	// MaxScript $Bip01 finds the node anywhere in the scene, not only top-level — search the
	// whole container if the root-level find missed.
	if (!bip01)
	{
		for (auto it = ssc->chunks().begin(); it != ssc->chunks().end() && !bip01; ++it)
		{
			CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
			if (n && NLMISC::toLower(ucstring(n->userName()).toUtf8()) == "bip01") bip01 = n;
		}
	}
	if (bip01 && selected.insert(bip01).second) selection.push_back(bip01);
	for (auto it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (!n) continue;
		if (getNodeScriptAppDataString(n, NEL3D_APPDATA_EXPORT_NODE_ANIMATION) == "1")
			if (selected.insert(n).second) selection.push_back(n);
	}

	if (selection.empty())
	{
		std::cerr << "WARNING: no node animated to export in " << maxFile << "\n";
		return 3;
	}

	// --- Export each selected node (CNelExport::exportAnim, scene=false).
	NL3D::CAnimation animFile;
	for (INode *node : selection)
	{
		std::string nodeName;
		std::string prefixe = getNodeScriptAppDataString(node, NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME);
		if (!prefixe.empty() && atoi(prefixe.c_str()) != 0)
		{
			nodeName = getNodeScriptAppDataString(node, NEL3D_APPDATA_INSTANCE_NAME);
			if (nodeName.empty())
				nodeName = ucstring(node->userName()).toUtf8();
			nodeName += ".";
		}
		addAnimation(animFile, *node, nodeName, ssc);
	}

	// --- Serialize through the real NL3D CAnimation.
	try
	{
		NLMISC::COFile file;
		if (!file.open(animOut))
		{
			std::cerr << "ERROR: cannot open output " << animOut << "\n";
			return 1;
		}
		animFile.serial(file);
		file.close();
	}
	catch (const NLMISC::Exception &e)
	{
		std::cerr << "ERROR: serial failed: " << e.what() << "\n";
		return 1;
	}

	return 0;
}

/* end of file */
