/**
 * \file main.cpp
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */
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
// biped path: the figure rig is reconstructed via pipeline_max_export_common, the animation keytracks on
// the Biped (0x9155) system object are decoded and TCB-evaluated (see biped_anim.h), and every
// biped node is oversampled once per frame across the union key range into LinearQuat/
// LinearVector tracks — replicating CExportNel::addBipedNodeTracks + overSampleBipedAnimation
// (NL3D_BIPED_OVERSAMPLING=30 at 30 fps == one sample per frame). Position tracks are emitted
// for the COM and the biped.getNode(#larm/#rarm/#spine/#tail) first links (clavicles, spine
// base, tail base), like the reference's mustExportBipedBonePos set.

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_stream.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/misc/algo.h>

#include <nel/3d/animation.h>
#include <nel/3d/camera.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/skeleton_model.h>
#include <nel/3d/track_keyframer.h>
#include <nel/3d/transformable.h>
#include <nel/3d/register_3d.h>

#include "../pipeline_max/storage_ole.h"

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
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"

#include "../pipeline_max_export_common/biped_rig.h"
#include "../pipeline_max_export_common/export_ids.h"
#include "../pipeline_max_export_common/max_load.h"
#include "../pipeline_max/biped/biped_driven.h"
#include "biped_anim.h"
#include "bip_file.h"
#include "biped_author.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

// Max time constants: 4800 ticks per second, 30 fps / 160 ticks per frame in every corpus
// scene. CExportNel::convertTime divides by GetTicksPerFrame()*GetFrameRate() = 4800.
static const float TICKS_PER_SECOND = 4800.0f;
// Max Interval sentinels (TIME_NegInfinity / TIME_PosInfinity)
static const sint32 TIME_NEG_INFINITY = (sint32)0x80000000;
static const sint32 TIME_POS_INFINITY = 0x7fffffff;

// NeL export AppData sub-ids (plugin_max/nel_mesh_lib/export_appdata.h)
// NeL export AppData sub-ids: pipeline_max_export_common/export_ids.h

// (PRS/LookAt TM controllers are the typed CControlPRS/CControlLookAt since §10j-dix)
static const NLMISC::CClassId CLASSID_BIPED_VHT_CTRL(0x00009156, 0x00000000);
static const NLMISC::CClassId CLASSID_MORPHER(0x17bb6854, 0xa5cba2a3);
static const NLMISC::CClassId CLASSID_PARAM_BLOCK_2(0x00000082, 0x00000000);
// Scripted-plugin class ids carry a per-script-edit PartB; match PartA only, like the
// reference exporter (export_nel.h NEL_PARTICLE_SYSTEM_CLASS_ID / BOOL_CONTROL_CLASS_ID).
static const uint32 CLASSID_PARTA_ONOFF_CTRL = 0x984b8d27;
// Camera superclass (isCamera in the reference)
static const TSClassId SCLASS_CAMERA = 0x00000020;

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
	for (PIPELINE::MAX::BUILTIN::STORAGE::CAppData::TMap::const_iterator it = ad->entries().begin(); it != ad->entries().end(); ++it)
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
	typeScale,
	typeFloat
};

// Build a NeL track from a typed controller, or NULL when the controller has no keys or is not
// a supported keyframer for the requested value type. Mirrors CExportNel::buildATrack.
static NL3D::ITrack *buildATrack(CReferenceMaker *ctrl, TNelValueType type)
{
	CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(ctrl);
	if (!kf) return nullptr;
	uint numKeys = kf->keyCount();
	if (!numKeys) return nullptr;

	sint32 rangeStart = 0, rangeEnd = 0;
	bool hasRange = kf->range(rangeStart, rangeEnd);

	float firstKey = 0.0f, lastKey = 0.0f;

	// Linear Position -> LinearVector (pos or scale request, like the reference)
	if (CControlPosLinear *c = dynamic_cast<CControlPosLinear *>(kf))
	{
		if (type != typePos && type != typeScale) return nullptr;
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Linear Scale -> LinearVector via the scale-matrix diagonal
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Bezier Position -> BezierVector (tangents scaled to per-second)
	if (CControlPosBezier *c = dynamic_cast<CControlPosBezier *>(kf))
	{
		if (type != typePos && type != typeScale) return nullptr;
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
		if (type != typePos && type != typeScale) return nullptr;
		NL3D::CTrackKeyFramerBezierVector *track = new NL3D::CTrackKeyFramerBezierVector();
		const CStorageBezScaleKey *keys = c->keys();
		if (getenv("PMB_ANIM_DUMP_BEZSCALE"))
		{
			for (uint i = 0; i < numKeys; ++i)
			{
				fprintf(stderr, "bezscale t=%d f=%08x s=(%.9g,%.9g,%.9g) q=(%.9g,%.9g,%.9g,%.9g) it=(%.9g,%.9g,%.9g) ot=(%.9g,%.9g,%.9g) il=(%.9g,%.9g,%.9g) ol=(%.9g,%.9g,%.9g)\n", keys[i].Time, keys[i].Flags,
				        keys[i].S[0], keys[i].S[1], keys[i].S[2], keys[i].Q[0], keys[i].Q[1], keys[i].Q[2], keys[i].Q[3],
				        keys[i].InTan[0], keys[i].InTan[1], keys[i].InTan[2], keys[i].OutTan[0], keys[i].OutTan[1], keys[i].OutTan[2],
				        keys[i].InLen[0], keys[i].InLen[1], keys[i].InLen[2], keys[i].OutLen[0], keys[i].OutLen[1], keys[i].OutLen[2]);
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

	// Bezier Float -> BezierFloat (tangents unscaled, like the reference IBezFloatKey conversion)
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// TCB Position -> TCBVector
	if (CControlPosTCB *c = dynamic_cast<CControlPosTCB *>(kf))
	{
		if (type != typePos && type != typeScale) return nullptr;
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
		if (type != typeRotation) return nullptr;
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

	// TCB Scale -> TCBVector via the scale-matrix diagonal
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
		applyRange(track, hasRange, rangeStart, rangeEnd, firstKey, lastKey);
		return track;
	}

	// Any other typed controller/value-type combination: no track. (Corpus-wide survey
	// 2026-07-06: the nine typed keyframer classes cover every PRS/LookAt/morph-factor
	// controller in the anim corpus — no Linear/TCB Float, Bezier Rotation/Point3 or list
	// controllers appear in exportable roles.)
	return nullptr;
}

// ---------------------------------------------------------------------------------------------
// Note tracks (Max stores them in the 0x2140 chunk on the Animatable: 0x0130 note-track count,
// then per note key 0x0100 time ticks + 0x0110 flags + 0x0120 UTF-16 note string).

struct SNoteKey
{
	sint32 Time;
	std::string Text;
};

// Note keys of the node's FIRST note track (GetNoteTrack(0) in the reference — the corpus only
// ever carries one; warn if the count chunk disagrees since track boundaries within the flat
// key list are not marked).
static bool getNoteKeys(INode &node, std::vector<SNoteKey> &out)
{
	CStorageContainer *nt = dynamic_cast<CStorageContainer *>(node.noteTracks());
	if (!nt) return false;
	sint32 time = 0;
	bool haveTime = false;
	for (CStorageContainer::TStorageObjectConstIt it = nt->chunks().begin(); it != nt->chunks().end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) continue;
		switch (it->first)
		{
		case 0x0130: // note track count
			if (raw->Value.size() == 4)
			{
				uint32 cnt;
				memcpy(&cnt, nlVectorData(raw->Value), 4);
				if (cnt > 1)
					fprintf(stderr, "WARNING: node carries %u note tracks; key-to-track assignment is not marked in storage, exporting all keys as track 0\n", cnt);
			}
			break;
		case 0x0100: // key time (ticks)
			if (raw->Value.size() == 4)
			{
				memcpy(&time, nlVectorData(raw->Value), 4);
				haveTime = true;
			}
			break;
		case 0x0120: // note string, UTF-16
		{
			if (!haveTime) break;
			ucstring us;
			us.resize(raw->Value.size() / 2);
			if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
			SNoteKey k;
			k.Time = time;
			k.Text = us.toUtf8();
			out.push_back(k);
			break;
		}
		default: // 0x0110 key flags: not consumed
			break;
		}
	}
	return !out.empty();
}

// ---------------------------------------------------------------------------------------------
// SkeletonSpawnScript builder — port of CSSSBuild (plugin_max/nel_mesh_lib/export_anim.cpp).
// Bones flagged NEL3D_APPDATA_EXPORT_SSS_TRACK contribute their note-track scripts; compile()
// turns the per-bone spawn/kill commands into one state-script ConstString track plus the
// animation's SSS shape set.

struct CSSSBuild
{
	struct CKey
	{
		std::string Value;
		float Time;
	};
	struct CBoneScript
	{
		std::string BoneName;
		std::vector<CKey> Track;
	};
	std::vector<CBoneScript> Bones;

	void compile(NL3D::CAnimation &dest, const std::string &baseName);
};

struct CSpawnCmd
{
	std::string Cmd;
	std::string Shape;
	std::string Bone;

	bool operator<(const CSpawnCmd &o) const
	{
		if (Shape != o.Shape) return Shape < o.Shape;
		if (Cmd != o.Cmd) return Cmd < o.Cmd;
		return Bone < o.Bone;
	}
};

struct CSpawnObject
{
	uint FinalId;
	bool Spawned;
	CSpawnObject() : FinalId(0), Spawned(false) { }
};

static void commitSSSKey(float keyTime, std::map<CSpawnCmd, CSpawnObject> &objects,
                         NL3D::CTrackKeyFramerConstString *finalTrack, bool &insertedAt0)
{
	NL3D::CKeyString keyValue;
	for (std::map<CSpawnCmd, CSpawnObject>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		if (it->second.Spawned)
		{
			if (it->first.Cmd == "lspawn")
				keyValue.Value += NLMISC::toString("objl %d %s %s\n", it->second.FinalId, it->first.Shape.c_str(), it->first.Bone.c_str());
			else if (it->first.Cmd == "wspawn")
				keyValue.Value += NLMISC::toString("objw %d %s\n", it->second.FinalId, it->first.Shape.c_str());
		}
	}
	finalTrack->addKey(keyValue, keyTime);
	if (keyTime == 0) insertedAt0 = true;
}

void CSSSBuild::compile(NL3D::CAnimation &dest, const std::string &baseName)
{
	if (Bones.empty()) return;

	std::map<CSpawnCmd, CSpawnObject> objects;
	uint numObjs = 0;
	std::multimap<float, CSpawnCmd> keys;

	// first pass: object set + key timeline
	for (uint i = 0; i < Bones.size(); ++i)
	{
		CBoneScript &bone = Bones[i];
		for (uint j = 0; j < bone.Track.size(); ++j)
		{
			std::string &script = bone.Track[j].Value;
			std::set<CSpawnCmd> keySet;
			std::vector<std::string> lines;
			NLMISC::splitString(script, "\n", lines);
			for (uint k = 0; k < lines.size(); ++k)
			{
				std::string line = lines[k];
				line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
				std::vector<std::string> words;
				NLMISC::splitString(line, " ", words);
				if (words.size() >= 2)
				{
					bool ok = (words[0] == "wspawn" || words[0] == "lspawn" || words[0] == "wkill" || words[0] == "lkill");
					if (ok)
					{
						CSpawnCmd oi;
						oi.Cmd = words[0];
						oi.Shape = NLMISC::toLower(words[1]);
						if (oi.Shape.find('.') == std::string::npos)
							oi.Shape += ".shape";
						oi.Bone = bone.BoneName;
						if (keySet.insert(oi).second)
						{
							if (oi.Cmd != "wkill" && oi.Cmd != "lkill")
							{
								if (objects.find(oi) == objects.end())
									objects[oi].FinalId = numObjs++;
							}
							keys.insert(std::make_pair(bone.Track[j].Time, oi));
						}
					}
				}
			}
		}
	}

	// second pass: the global state-script track
	NL3D::CTrackKeyFramerConstString *finalTrack = new NL3D::CTrackKeyFramerConstString();
	float precTime = 0;
	bool firstKey = true;
	bool insertedAt0 = false;
	for (std::multimap<float, CSpawnCmd>::iterator kit = keys.begin(); kit != keys.end(); ++kit)
	{
		float keyTime = kit->first;
		CSpawnCmd keyValue = kit->second;
		if (!firstKey && precTime != keyTime)
		{
			commitSSSKey(precTime, objects, finalTrack, insertedAt0);
			precTime = keyTime;
		}
		if (firstKey) firstKey = false;
		precTime = keyTime;

		bool isSpawn = (keyValue.Cmd == "wspawn" || keyValue.Cmd == "lspawn");
		if (!isSpawn)
		{
			if (keyValue.Cmd == "wkill") keyValue.Cmd = "wspawn";
			if (keyValue.Cmd == "lkill") keyValue.Cmd = "lspawn";
		}
		std::map<CSpawnCmd, CSpawnObject>::iterator oit = objects.find(keyValue);
		if (oit != objects.end())
			oit->second.Spawned = isSpawn;
	}

	if (!firstKey)
	{
		commitSSSKey(precTime, objects, finalTrack, insertedAt0);
		if (!insertedAt0)
		{
			NL3D::CKeyString keyValue;
			finalTrack->addKey(keyValue, 0);
		}
	}
	else
	{
		delete finalTrack;
		finalTrack = nullptr;
	}

	if (finalTrack)
	{
		std::string name = baseName + NL3D::CSkeletonModel::getSpawnScriptValueName();
		dest.addTrack(name, finalTrack);

		std::set<std::string> shapeSet;
		for (std::map<CSpawnCmd, CSpawnObject>::iterator oit = objects.begin(); oit != objects.end(); ++oit)
			shapeSet.insert(oit->first.Shape);
		for (std::set<std::string>::iterator it = shapeSet.begin(); it != shapeSet.end(); ++it)
			dest.addSSSShape(*it);
	}
}

static void addSSSTrack(CSSSBuild &ssBuilder, INode &node)
{
	CSSSBuild::CBoneScript bs;
	bs.BoneName = ucstring(node.userName()).toUtf8();
	std::vector<SNoteKey> notes;
	getNoteKeys(node, notes);
	bs.Track.reserve(notes.size());
	for (uint i = 0; i < notes.size(); ++i)
	{
		CSSSBuild::CKey ks;
		ks.Value = notes[i].Text;
		ks.Time = convertTime(notes[i].Time);
		bs.Track.push_back(ks);
	}
	if (!bs.Track.empty())
		ssBuilder.Bones.push_back(bs);
}

// NoteTrack export (NEL3D_APPDATA_EXPORT_NOTE_TRACK): the raw note strings as a ConstString
// track named "NoteTrack" (no base-name prefix, like the reference).
static void addNoteTrack(NL3D::CAnimation &animation, INode &node)
{
	std::vector<SNoteKey> notes;
	if (!getNoteKeys(node, notes)) return;
	NL3D::CTrackKeyFramerConstString *st = new NL3D::CTrackKeyFramerConstString();
	float firstDate = 0, lastDate = 0;
	for (uint i = 0; i < notes.size(); ++i)
	{
		NL3D::CKeyString ks;
		if (i == 0) firstDate = convertTime(notes[i].Time);
		ks.Value = notes[i].Text;
		lastDate = convertTime(notes[i].Time);
		st->addKey(ks, lastDate);
	}
	st->unlockRange(firstDate, lastDate);
	if (animation.getTrackByName("NoteTrack"))
	{
		delete st;
		return;
	}
	animation.addTrack("NoteTrack", st);
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

// The node's object (reference slot 1), unwrapped: NULL-safe. Returned as CReferenceMaker
// (every scene object that matters here is one) so both classDesc() and getReference() work.
static CReferenceMaker *objectOfNode(INode &node)
{
	return dynamic_cast<CReferenceMaker *>(node.getReference(1));
}

// isCamera in the reference: the node's object has the camera superclass.
static bool nodeIsCamera(INode &node)
{
	CReferenceMaker *obj = objectOfNode(node);
	return obj && obj->classDesc()->superClassId() == SCLASS_CAMERA;
}

// The node's PRS/LookAt transform sub-controllers, replicating GetScaleController & co:
// PRS (0x2005) refs 0/1/2 = position/rotation/scale; LookAt (0x2006) refs 0/1/2/3 =
// target-node/position/roll/scale (rotation is computed, GetRotationController is NULL).
// Other transform types (lists, expressions, biped) have no keyframer sub-controllers the
// reference path would export.
static void addNodeTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName,
                          CSSSBuild *ssBuilder)
{
	CReferenceMaker *transform = node.getReference(0);
	CControlPRS *prs = dynamic_cast<CControlPRS *>(transform);
	CControlLookAt *lookAt = dynamic_cast<CControlLookAt *>(transform);

	if (prs || lookAt)
	{
		// Export order matches the reference: scale, rotation, position, then camera extras
		// (typed sub-controller slots on CControlPRS/CControlLookAt — §10j-dix).
		NL3D::ITrack *track = buildATrack(prs ? prs->scaleController() : lookAt->scaleController(), typeScale);
		addTrackChecked(animation, parentName + NL3D::ITransformable::getScaleValueName(), track);

		if (prs)
		{
			track = buildATrack(prs->rotationController(), typeRotation);
			addTrackChecked(animation, parentName + NL3D::ITransformable::getRotQuatValueName(), track);
		}

		track = buildATrack(prs ? prs->positionController() : lookAt->positionController(), typePos);
		addTrackChecked(animation, parentName + NL3D::ITransformable::getPosValueName(), track);

		// Camera roll + target position (reference: only when isCamera(node)).
		if (lookAt && nodeIsCamera(node))
		{
			track = buildATrack(lookAt->rollController(), typeFloat);
			addTrackChecked(animation, parentName + NL3D::CCamera::getRollValueName(), track);

			CNodeImpl *target = dynamic_cast<CNodeImpl *>(lookAt->targetNode());
			if (target)
			{
				if (CControlPRS *tprs = dynamic_cast<CControlPRS *>(target->getReference(0)))
				{
					track = buildATrack(tprs->positionController(), typePos);
					addTrackChecked(animation, parentName + NL3D::CCamera::getTargetValueName(), track);
				}
			}
		}
	}

	// SkeletonSpawnScript contribution (reference: checked on every node addNodeTracks visits)
	if (ssBuilder && getNodeScriptAppDataString(&node, NEL3D_APPDATA_EXPORT_SSS_TRACK) == "1")
		addSSSTrack(*ssBuilder, node);

	// Object FOV, material and light tracks are not exported: no corpus signal (no fov/
	// LightmapController tracks in any reference .anim, no NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS).
}

// ---------------------------------------------------------------------------------------------
// Morph tracks: the Morpher modifier (Morpher.dlm) on the node's derived object. Channel i
// (0..99): target node = morpher reference 101+i, factor controller = reference 0 of the
// channel's ParamBlock (morpher reference i+1). Track name = parentName + targetName +
// "MorphFactor", replicating CExportNel::addMorphTracks.
static void addMorphTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName)
{
	CDerivedObject *obj = dynamic_cast<CDerivedObject *>(objectOfNode(node));
	if (!obj) return;
	CReferenceMaker *morpher = nullptr;
	for (uint i = 0; i < obj->modifierCount() && !morpher; ++i)
	{
		CSceneClass *mod = obj->modifier(i);
		if (mod && mod->classDesc()->classId() == CLASSID_MORPHER)
			morpher = dynamic_cast<CReferenceMaker *>(mod);
	}
	if (!morpher) return;

	for (uint i = 0; i < 100; ++i)
	{
		CNodeImpl *target = dynamic_cast<CNodeImpl *>(morpher->getReference(101 + i));
		if (!target) continue;
		CReferenceMaker *pblock = morpher->getReference(i + 1);
		if (!pblock) continue;
		NL3D::ITrack *track = buildATrack(pblock->getReference(0), typeFloat);
		std::string name = parentName + ucstring(target->userName()).toUtf8() + "MorphFactor";
		addTrackChecked(animation, name, track);
	}
}

// ---------------------------------------------------------------------------------------------
// Particle-system tracks: the scripted "Particle Sys" plugin object (class PartA 0x58ce2893,
// see plugin_max/scripts/startup/nel_ps.ms). Its ParamBlock2 params, in script order:
// 0 ps_file_name (string), 1..4 PSParam0..3 (float, animatable), 5 PSTrigger (bool, animatable).
// The PB2's 0x000e param records carry [u16 id][u16 type][10 bytes][flag byte]: flag bit 0x40 =
// constant value follows (no controller); records WITHOUT 0x40 are controller-backed, and their
// controllers occupy the PB2's reference slots in record order (single-controller case verified
// against fy_hom_co_fu_tir/tir2/co_p_tir; no multi-controller PS exists in the corpus).

// On/Off (bool) controller -> ConstBool track, replicating buildOnOffTrack. Storage: orphan
// chunks 0x0130 key count, per key 0x0100 time + 0x0110 flags, 0x0140 boundary state. The
// value toggles at each key; 0x0140 = 1 observed on every key-bearing corpus instance,
// interpreted as the state BEFORE the first key (both boundary interpretations agree on the
// even-key-count corpus instances; an odd-count file would discriminate — none exists).
static NL3D::ITrack *buildOnOffTrack(CReferenceMaker *ctrl)
{
	std::vector<sint32> times;
	uint32 initState = 0;
	const CStorageContainer::TStorageObjectContainer &orphans = ctrl->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw || raw->Value.size() != 4) continue;
		uint32 v;
		memcpy(&v, nlVectorData(raw->Value), 4);
		if (it->first == 0x0100) times.push_back((sint32)v);
		else if (it->first == 0x0140) initState = v;
	}
	NL3D::CTrackKeyFramerConstBool *track = new NL3D::CTrackKeyFramerConstBool();
	bool state = initState != 0;
	for (uint i = 0; i < times.size(); ++i)
	{
		state = !state;
		NL3D::CKeyBool k;
		k.Value = state;
		track->addKey(k, convertTime(times[i]));
	}
	if (!times.empty())
		track->unlockRange(convertTime(times.front()), convertTime(times.back()));
	else
		// The reference wrote uninitialized stack floats as the range here (0-key On/Off,
		// GetTimeRange == NEVER); normalized to an empty [0,0] range — key data matches.
		track->unlockRange(0.0f, 0.0f);
	track->setLoopMode(false);
	return track;
}

static void addParticleSystemTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName)
{
	CReferenceMaker *obj = objectOfNode(node);
	if (!obj) return;
	if (obj->classDesc()->classId().a() != CLASSID_PARTA_NEL_PS) return;

	// Find the object's ParamBlock2 and parse its param records.
	CReferenceMaker *pb2 = nullptr;
	for (uint i = 0; i < obj->nbReferences() && !pb2; ++i)
	{
		CReferenceMaker *r = obj->getReference(i);
		if (r && r->classDesc()->classId() == CLASSID_PARAM_BLOCK_2)
			pb2 = r;
	}
	if (!pb2) return;

	// Param id -> PB2 reference slot, assigned in 0x000e record order for controller-backed
	// (non-constant) params.
	std::map<uint16, uint> ctrlSlotOfParam;
	{
		uint slot = 0;
		const CStorageContainer::TStorageObjectContainer &orphans = pb2->orphanedChunks();
		for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		{
			if (it->first != 0x000e) continue;
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			if (!raw || raw->Value.size() < 15) continue;
			uint16 paramId, paramType;
			memcpy(&paramId, nlVectorData(raw->Value), 2);
			memcpy(&paramType, nlVectorData(raw->Value) + 2, 2);
			uint8 flag = raw->Value[14];
			if (!(flag & 0x40))
				ctrlSlotOfParam[paramId] = slot++;
		}
	}

	// PSParam0..3 = params 1..4 (float keyframers)
	for (uint k = 0; k < 4; ++k)
	{
		std::map<uint16, uint>::iterator it = ctrlSlotOfParam.find((uint16)(1 + k));
		if (it == ctrlSlotOfParam.end()) continue;
		NL3D::ITrack *track = buildATrack(pb2->getReference(it->second), typeFloat);
		std::string name = parentName + NL3D::CParticleSystemModel::getPSParamName((uint)NL3D::CParticleSystemModel::PSParam0 + k);
		addTrackChecked(animation, name, track);
	}

	// PSTrigger = param 5 (On/Off controller)
	{
		std::map<uint16, uint>::iterator it = ctrlSlotOfParam.find((uint16)5);
		if (it == ctrlSlotOfParam.end()) return;
		CReferenceMaker *ctrl = pb2->getReference(it->second);
		if (!ctrl) return;
		if (ctrl->classDesc()->classId().a() != CLASSID_PARTA_ONOFF_CTRL)
		{
			fprintf(stderr, "WARNING: PSTrigger controller is not an On/Off controller, skipped\n");
			return;
		}
		NL3D::ITrack *track = buildOnOffTrack(ctrl);
		addTrackChecked(animation, parentName + "PSTrigger", track);
	}
}

// Ordered children in scene (container) order — INode::children() is pointer-keyed and
// unordered; same approach as pipeline_max_export_skel.
static std::vector<INode *> orderedChildrenOf(INode *parent, CSceneClassContainer *ssc)
{
	std::vector<INode *> out;
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (n && n->parent() == parent) out.push_back(n);
	}
	return out;
}

struct SBipedSampled;
static void addBipedNodeTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName,
                               bool root, CSceneClassContainer *ssc, const SBipedSampled &sampled,
                               CSSSBuild &ssBuilder);

static void addBoneTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName, CSceneClassContainer *ssc, CSSSBuild &ssBuilder, const SBipedSampled *sampled)
{
	// A nested biped COM under a non-biped node re-enters the biped path with the selection's
	// shared sample context — CExportNel::addBoneTracks does exactly this on BIPBODY children
	// (the mektoub_selle rider rig: Bip01male under selle-assise/selle inside the mount's
	// subtree; the rider's uniquely-named bones — Finger3/4, Ponytail2, its dummies — only
	// export through this branch). The reference then ALSO recurses children unconditionally,
	// but every node the biped branch reaches is already covered by addBipedNodeTracks'
	// own recursion and the duplicates are discarded by first-wins — insertion order (and so
	// the output bytes) is identical without the redundant second visit.
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node.getReference(0));
	if (sampled && tmsc && tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL)
	{
		addBipedNodeTracks(animation, node, parentName, false, ssc, *sampled, ssBuilder);
		return;
	}
	// Track names are FLAT: parentName + nodeName + "." for this node's own tracks, but the
	// recursion passes the ORIGINAL parentName down (matching CExportNel::addBoneTracks).
	std::string name = parentName + ucstring(node.userName()).toUtf8() + ".";
	addNodeTracks(animation, node, name, &ssBuilder);
	std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
	for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci)
		{ INode *child = *ci; addBoneTracks(animation, *child, parentName, ssc, ssBuilder, sampled); }
}


// root = the selected node's parent is the scene root (CNelExport::exportAnim: GetParentNode()
// == GetRootNode()) — biped tracks lose the bare names and get "<nodeName>."-prefixed when the
// selected biped hangs under another node (e.g. the aquatic-mount rigs' Bip01 under stick_1).
// ---------------------------------------------------------------------------------------------
// Biped export path.

struct SBipedSampled
{
	std::vector<sint32> Times; // sample times (ticks)
	std::map<INode *, std::vector<NLMISC::CQuat> > Rot;   // local rotation per sample
	std::map<INode *, std::vector<NLMISC::CVector> > Pos; // local position per sample
	// Global sampled range (ticks). Every biped track's unlockRange uses this — verified
	// against the direct references: the reference exporter's per-controller GetTimeRange is
	// NEVER/FOREVER on biped controllers, so every track falls back to its first/last sampled
	// key, which is the global [BipedRangeMin..BipedRangeMax] span.
	sint32 RangeMin, RangeMax;
	std::set<INode *> PosExport; // nodes that get a pos track
};

// Sample the whole biped subtree once per frame. Returns false when no biped keys exist.
// \a overrideKeys — when non-NULL (e.g. keys loaded from a Character Studio .bip), replaces
// the figure file's (usually empty) keytracks on every rig.
static bool sampleBipedSubtree(INode &root, CSceneClassContainer *ssc, SBipedSampled &out,
                               const BIPANIM::SBipAnimKeys *overrideKeys = nullptr)
{
	using namespace PMAX_RIG;

	// figure-time walk (fills g_bipedRigs and per-bone figure world transforms)
	g_bipedRigs.clear();
	g_rig = nullptr;
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
		BIPANIM::CBipedAnimEval *ev = new BIPANIM::CBipedAnimEval(it->first, it->second, bones, boneOf, overrideKeys);
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

	// sample times: one per frame (160 ticks) across [min, max] — exactly the reference's
	// overSampleBipedAnimation loop: samples at min + k*step while <= max, NO final clamped
	// sample at max (the loop's inner std::min clamp is dead code — verified against
	// references with off-frame range ends, e.g. fy_hof_co_a1md_attente2's tick 5638: the
	// reference's last sample sits at 5600).
	const sint32 step = 160;
	for (sint32 tt = rangeMin; tt <= rangeMax; tt += step)
		out.Times.push_back(tt);
	// unlockRange span = first/last sampled key (the reference's FOREVER/NEVER fallback path;
	// every direct reference has track range == sampled key span).
	out.RangeMin = out.Times.front();
	out.RangeMax = out.Times.back();

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
			// A LINKED biped COM (nested under another node — the kitin family's Bip02 under
			// Bip01 Spine, the mektoub rider's Bip01male under the saddle) rides its parent
			// RIGIDLY in Character Studio: the reference's exported local TM is CONSTANT per
			// file, and the COM's own h/v/t channels do NOT place a linked rig in world.
			// Rig-internal locals are invariant under this re-anchoring (both sides of every
			// child's local shift together), so only the COM's exported track changes.
			// The constant is derived from a stored pair (gen_biped_linkcom_probe round,
			// design doc §10m-ter; float-exact on kitin run/stun/queen + the mektoub rider):
			//   L = 0x0112 (inverse of the parent's world TM at link/edit time)
			//     * [0x0104 rotation | 0x0104.t + 0x0260[0..2] correction]
			// with the exported quat = the conjugate of L's rotation. Fallback: the figure
			// attach from the walked WorldTMs (position-close on figure-consistent files)
			// when the chunk pair is missing.
			if (PMAX_RIG::isBipedComNode(node) && parent)
			{
				std::map<INode *, size_t>::const_iterator bi = boneOf.find(node), pi = boneOf.find(parent);
				if (bi != boneOf.end() && pi != boneOf.end())
				{
					const PMAX_RIG::SBipedRig *rig = nullptr;
					CSceneClass *sys = PMAX_RIG::bipedSystemOfCtrl(dynamic_cast<CReferenceMaker *>(node->getReference(0)));
					if (sys)
					{
						std::map<CSceneClass *, PMAX_RIG::SBipedRig>::iterator rit = PMAX_RIG::g_bipedRigs.find(sys);
						if (rit != PMAX_RIG::g_bipedRigs.end()) rig = &rit->second;
					}
					if (rig && rig->HaveLinkParentInv && rig->HaveBaseFrameTM)
					{
						NLMISC::CMatrix loc = rig->LinkParentInvTM * rig->BaseFrameTM;
						out.Rot[node].push_back(loc.getRot());
						out.Pos[node].push_back(loc.getPos());
					}
					else
					{
						NLMISC::CMatrix loc = bones[pi->second].WorldTM.inverted() * bones[bi->second].WorldTM;
						out.Rot[node].push_back(loc.getRot());
						out.Pos[node].push_back(loc.getPos());
					}
					continue;
				}
			}
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

	// pos-track set (COM + clavicles/spine/tail bases, the reference's mustExportBipedBonePos)
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
	}

	for (size_t i = 0; i < evals.size(); ++i) delete evals[i];
	return true;
}

// Build a LinearQuat track from sampled local rotations (normalize + makeClosest, reference
// createBipedKeyFramer).
static NL3D::ITrack *buildSampledQuatTrack(const std::vector<sint32> &times, const std::vector<NLMISC::CQuat> &vals,
                                           sint32 rangeStart, sint32 rangeEnd)
{
	if (vals.empty()) return nullptr;
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
	if (vals.empty()) return nullptr;
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

// --com-node-prefix: give a COM exported with an empty base name its node-name prefix
// ("Bip01.pos"). The Snowballs 2003 refs have this shape (COM prefixed, bones bare — the
// era result when the selected COM was not a direct child of the scene root, so root was
// false with an empty parent prefix). The Ryzom per-node refs are bare "pos"/"rotquat",
// so this must never be default.
static bool g_comNodePrefix = false;

// Replicates CExportNel::addBipedNodeTracks: biped nodes emit their sampled tracks under
// parentName + name + "." ; the COM root uses parentName VERBATIM — bare "pos"/"rotquat"
// when the caller passed no prefix (the Ryzom per-node exports; all 2004 refs). The
// Snowballs 2003 shape (COM "Bip01.pos", bones bare) is opt-in via --com-node-prefix.
// Non-biped children go through addBoneTracks.
static void addBipedNodeTracks(NL3D::CAnimation &animation, INode &node, const std::string &parentName,
                               bool root, CSceneClassContainer *ssc, const SBipedSampled &sampled,
                               CSSSBuild &ssBuilder)
{
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node.getReference(0));
	bool isBiped = tmsc && (tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL ||
	                        tmsc->classDesc()->classId() == PIPELINE::MAX::BIPED::CBipedDriven::ClassId);
	if (isBiped)
	{
		std::string name = root ? parentName : (parentName + ucstring(node.userName()).toUtf8() + ".");
		if (root && name.empty() && g_comNodePrefix)
			name = ucstring(node.userName()).toUtf8() + ".";
		// scale: biped nodes have none. rotation:
		std::map<INode *, std::vector<NLMISC::CQuat> >::const_iterator rit = sampled.Rot.find(&node);
		if (rit != sampled.Rot.end())
		{
			NL3D::ITrack *track = buildSampledQuatTrack(sampled.Times, rit->second, sampled.RangeMin, sampled.RangeMax);
			addTrackChecked(animation, name + NL3D::ITransformable::getRotQuatValueName(), track);
		}
		// position (COM + clavicles/spine/tail bases only):
		if (sampled.PosExport.count(&node))
		{
			std::map<INode *, std::vector<NLMISC::CVector> >::const_iterator pit = sampled.Pos.find(&node);
			if (pit != sampled.Pos.end())
			{
				NL3D::ITrack *track = buildSampledPosTrack(sampled.Times, pit->second, sampled.RangeMin, sampled.RangeMax);
				addTrackChecked(animation, name + NL3D::ITransformable::getPosValueName(), track);
			}
		}
		// SkeletonSpawnScript contribution (the reference checks it in addNodeTracks, which
		// runs for biped nodes too)
		if (getNodeScriptAppDataString(&node, NEL3D_APPDATA_EXPORT_SSS_TRACK) == "1")
			addSSSTrack(ssBuilder, node);
		std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
		for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci)
			{ INode *child = *ci; addBipedNodeTracks(animation, *child, parentName, false, ssc, sampled, ssBuilder); }
	}
	else
	{
		addBoneTracks(animation, node, parentName, ssc, ssBuilder, &sampled);
	}
}

// Optional BIP override keys for the current export (set from main via --bip).
static const BIPANIM::SBipAnimKeys *g_bipOverrideKeys = nullptr;

static void addAnimation(NL3D::CAnimation &animation, INode &node, const std::string &baseName, bool root, CSceneClassContainer *ssc)
{
	// One SkeletonSpawnScript builder per exported selection node, like the reference.
	CSSSBuild ssBuilder;

	// The reference builds ONE CAnimationBuildCtx over the whole selected subtree up front
	// (buildBipedInformation + overSampleBipedAnimation) regardless of the selected node's own
	// controller class, so a nested biped COM anywhere in the subtree (the mektoub_selle rider
	// under the saddle nodes) samples and exports through the same shared context and range.
	SBipedSampled sampled;
	bool hasBiped = sampleBipedSubtree(node, ssc, sampled, g_bipOverrideKeys);

	// Biped COM root: the oversampling path (CExportNel::addBipedNodeTracks).
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node.getReference(0));
	if (tmsc && tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL)
	{
		if (hasBiped)
		{
			addBipedNodeTracks(animation, node, baseName, root, ssc, sampled, ssBuilder);
		}
		else
		{
			// no biped keys at all: fall back to the non-biped path (nothing to oversample)
			addNodeTracks(animation, node, baseName, &ssBuilder);
			std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
			for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci)
				{ INode *child = *ci; addBoneTracks(animation, *child, baseName, ssc, ssBuilder, nullptr); }
		}
	}
	else
	{
		// Non-biped path of CExportNel::addAnimation: the node's own tracks under the bare base
		// name, then particle-system and morph tracks, then every child subtree via
		// addBoneTracks. (Object FOV, material and light tracks: no corpus signal.)
		addNodeTracks(animation, node, baseName, &ssBuilder);
		addParticleSystemTracks(animation, node, baseName);
		addMorphTracks(animation, node, baseName);
		std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
		for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci)
			{ INode *child = *ci; addBoneTracks(animation, *child, baseName, ssc, ssBuilder, hasBiped ? &sampled : nullptr); }
	}

	// NoteTrack export (a string track used to create events)
	if (getNodeScriptAppDataString(&node, NEL3D_APPDATA_EXPORT_NOTE_TRACK) == "1")
		addNoteTrack(animation, node);

	// Compile the SkeletonSpawnScript builder
	ssBuilder.compile(animation, baseName);
}

// ---------------------------------------------------------------------------------------------

// Evaluate the biped at quarter-frame steps over [0, maxFrame] and dump world transforms in the
// differential anim dataset's manifest SAMPLE format (MAXScript-convention quats): the compare
// side of the Max 9 ground truth (~/biped_anim_dataset). World state straight from evalAt — no
// local conversion, exactly what biped.getTransform reported.
static int dumpBipedSamples(INode &root, CSceneClassContainer *ssc, const char *outPath, double maxFrame)
{
	using namespace PMAX_RIG;
	g_bipedRigs.clear();
	g_rig = nullptr;
	g_msBones.clear();
	std::vector<Bone> bones;
	std::set<std::string> nameSet;
	NLMISC::CMatrix rootMat; rootMat.identity();
	walkNode(&root, -1, rootMat, ssc, bones, nameSet);
	patchFootstepsGround(bones);
	std::map<INode *, size_t> boneOf;
	for (size_t i = 0; i < bones.size(); ++i)
		if (bones[i].Node) boneOf[bones[i].Node] = i;
	std::vector<BIPANIM::CBipedAnimEval *> evals;
	for (std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.begin(); it != g_bipedRigs.end(); ++it)
	{
		g_rig = &it->second;
		evals.push_back(new BIPANIM::CBipedAnimEval(it->first, it->second, bones, boneOf));
	}
	FILE *fp = fopen(outPath, "w");
	if (!fp) { std::cerr << "ERROR: cannot open " << outPath << "\n"; return 1; }
	std::map<INode *, BIPANIM::SBipNodeState> state;
	for (double f = 0.0; f <= maxFrame + 1e-9; f += 0.25)
	{
		state.clear();
		for (size_t e = 0; e < evals.size(); ++e)
			evals[e]->evalAt(f * 160.0, state);
		for (size_t i = 0; i < bones.size(); ++i)
		{
			if (!bones[i].Node) continue;
			std::map<INode *, BIPANIM::SBipNodeState>::iterator it = state.find(bones[i].Node);
			if (it == state.end()) continue;
			const BIPANIM::SBipNodeState &st = it->second;
			fprintf(fp, "  SAMPLE\t%.9g\t%s\tpos\t%.9g,%.9g,%.9g\trot\t%.9g,%.9g,%.9g,%.9g\n",
			        f, bones[i].Name.c_str(),
			        st.WorldPos.x, st.WorldPos.y, st.WorldPos.z,
			        -st.WorldRot.x, -st.WorldRot.y, -st.WorldRot.z, st.WorldRot.w);
		}
	}
	fclose(fp);
	for (size_t i = 0; i < evals.size(); ++i) delete evals[i];
	return 0;
}

// ============================================================================
// --diff-rig: unbiased, full per-chunk byte/float diff of every Biped (0x9155)
// system object between two .max files, for differential-dataset work (compare
// a baseline against each single- or multi-toggle variant with no assumption
// about which chunk matters — everything that changed is reported, not just
// the fields we already have a theory about).
// ============================================================================
struct SLoadedRigScene
{
	CDllDirectory *Dll;
	CClassDirectory3 *Cd;
	CScene *Scene;
};

static bool loadRigScene(const char *path, CSceneClassRegistry *reg, SLoadedRigScene &out)
{
	CStorageOleIn in;
	if (!in.open(path)) { std::cerr << "ERROR: not an OLE compound file: " << path << "\n"; return false; }
	out.Dll = new CDllDirectory();
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory in " << path << "\n"; return false; } CStorageStream st(b); out.Dll->serial(st); }
	out.Dll->parse(VersionUnknown);
	out.Cd = new CClassDirectory3(out.Dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 in " << path << "\n"; return false; } CStorageStream st(b); out.Cd->serial(st); }
	out.Cd->parse(VersionUnknown);
	out.Scene = new CScene(reg, out.Dll, out.Cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene in " << path << "\n"; return false; } CStorageStream st(b); out.Scene->serial(st); }
	out.Scene->parse(VersionUnknown);
	return true;
}

struct SBipedSysDump
{
	int Index;
	// Duplicate chunk ids within one object are rare on the 0x9155 system object and are
	// collapsed here (last-wins) — fine for a differential-dataset diagnostic where we're
	// comparing near-identical files, not for anything roundtrip-authoritative.
	std::map<uint16, std::vector<uint8> > Chunks;
};

static void collectBipedSysDumps(CSceneClassContainer *ssc, std::vector<SBipedSysDump> &out)
{
	static const NLMISC::CClassId CLASSID_SYS(0x00009155, 0x00000000);
	int idx = 0;
	for (CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it, ++idx)
	{
		CSceneClass *sc = dynamic_cast<CSceneClass *>(it->second);
		if (!sc || sc->classDesc()->classId() != CLASSID_SYS) continue;
		SBipedSysDump d;
		d.Index = idx;
		// Read via chunks(), not orphanedChunks(): the pre-clean container still holds every
		// original chunk, INCLUDING the keytrack pairs the typed CBipedSystem parse lifts out of
		// the orphan list (biped_coverage.py / gen_biped_fields_diff.py depend on complete dumps).
		const CStorageContainer::TStorageObjectContainer &all = sc->chunks();
		for (CStorageContainer::TStorageObjectConstIt jt = all.begin(); jt != all.end(); ++jt)
		{
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(jt->second);
			if (raw) d.Chunks[jt->first] = raw->Value;
		}
		out.push_back(d);
	}
}

static void diffBipedSysDumps(const std::vector<SBipedSysDump> &a, const std::vector<SBipedSysDump> &b, FILE *fp)
{
	size_t n = a.size() > b.size() ? a.size() : b.size();
	for (size_t i = 0; i < n; ++i)
	{
		if (i >= a.size()) { fprintf(fp, "SYSOBJ %zu only in B\n", i); continue; }
		if (i >= b.size()) { fprintf(fp, "SYSOBJ %zu only in A\n", i); continue; }
		const SBipedSysDump &da = a[i], &db = b[i];
		std::set<uint16> ids;
		for (std::map<uint16, std::vector<uint8> >::const_iterator it = da.Chunks.begin(); it != da.Chunks.end(); ++it) ids.insert(it->first);
		for (std::map<uint16, std::vector<uint8> >::const_iterator it = db.Chunks.begin(); it != db.Chunks.end(); ++it) ids.insert(it->first);
		for (std::set<uint16>::iterator idit = ids.begin(); idit != ids.end(); ++idit)
		{
			uint16 id = *idit;
			std::map<uint16, std::vector<uint8> >::const_iterator ia = da.Chunks.find(id);
			std::map<uint16, std::vector<uint8> >::const_iterator ib = db.Chunks.find(id);
			if (ia == da.Chunks.end()) { fprintf(fp, "SYSOBJ %zu chunk 0x%04x only in B (%zu bytes)\n", i, id, ib->second.size()); continue; }
			if (ib == db.Chunks.end()) { fprintf(fp, "SYSOBJ %zu chunk 0x%04x only in A (%zu bytes)\n", i, id, ia->second.size()); continue; }
			if (ia->second == ib->second) continue;
			fprintf(fp, "SYSOBJ %zu chunk 0x%04x DIFFERS (A %zu bytes, B %zu bytes)\n", i, id, ia->second.size(), ib->second.size());
			if (ia->second.size() == ib->second.size() && ia->second.size() >= 4 && (ia->second.size() % 4) == 0)
			{
				// Compare raw bit patterns, not float value equality — a +0.0/-0.0 flip or a
				// differing NaN payload is byte-real and worth reporting even though it would
				// compare "equal" (or uninformatively as "nan") under floating-point ==.
				size_t nf = ia->second.size() / 4;
				const float *fa = reinterpret_cast<const float *>(nlVectorData(ia->second));
				const float *fb = reinterpret_cast<const float *>(nlVectorData(ib->second));
				const uint32 *ua = reinterpret_cast<const uint32 *>(nlVectorData(ia->second));
				const uint32 *ub = reinterpret_cast<const uint32 *>(nlVectorData(ib->second));
				for (size_t k = 0; k < nf; ++k)
					if (ua[k] != ub[k])
						fprintf(fp, "    [%zu] %.9g -> %.9g (delta %.9g) [bits 0x%08x -> 0x%08x]\n", k, fa[k], fb[k], fb[k] - fa[k], ua[k], ub[k]);
			}
		}
	}
}

// --dump-rig: dump every Biped (0x9155) system object's chunks (floats + first ints) from a
// single file, for inspecting a specific chunk's contents (e.g. 0x0117, the Move All Mode
// reference-frame matrix) without needing a second file to diff against.
static int runDumpRig(const char *path, const char *outPath)
{
	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	SLoadedRigScene a;
	if (!loadRigScene(path, &reg, a)) return 1;
	std::vector<SBipedSysDump> da;
	collectBipedSysDumps(a.Scene->container(), da);

	FILE *fp = outPath ? fopen(outPath, "w") : stdout;
	if (!fp) { std::cerr << "ERROR: cannot open " << outPath << " for writing\n"; return 1; }
	fprintf(fp, "DUMP-RIG %s (%zu sysobjs)\n", path, da.size());
	for (size_t i = 0; i < da.size(); ++i)
	{
		fprintf(fp, "SYSOBJ %zu (%zu chunks)\n", i, da[i].Chunks.size());
		for (std::map<uint16, std::vector<uint8> >::const_iterator it = da[i].Chunks.begin(); it != da[i].Chunks.end(); ++it)
		{
			const std::vector<uint8> &v = it->second;
			fprintf(fp, "  chunk 0x%04x bytes=%zu", it->first, v.size());
			if (v.size() >= 4 && (v.size() % 4) == 0)
			{
				// PMB_DUMP_RIG_FULL: no float/int cap (keytrack record inspection)
				static const bool s_full = getenv("PMB_DUMP_RIG_FULL") != nullptr;
				size_t nf = v.size() / 4;
				size_t fCap = s_full ? nf : 32, iCap = s_full ? nf : 8;
				const float *f = reinterpret_cast<const float *>(nlVectorData(v));
				const sint32 *iv = reinterpret_cast<const sint32 *>(nlVectorData(v));
				fprintf(fp, " floats=[");
				for (size_t k = 0; k < nf && k < fCap; ++k) fprintf(fp, "%.9g,", f[k]);
				fprintf(fp, "] ints=[");
				for (size_t k = 0; k < nf && k < iCap; ++k) fprintf(fp, "%d,", iv[k]);
				fprintf(fp, "]");
			}
			fprintf(fp, "\n");
		}
	}
	if (outPath) fclose(fp);
	return 0;
}

static int runDiffRig(const char *pathA, const char *pathB, const char *outPath)
{
	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	SLoadedRigScene a, b;
	if (!loadRigScene(pathA, &reg, a)) return 1;
	if (!loadRigScene(pathB, &reg, b)) return 1;

	std::vector<SBipedSysDump> da, db;
	collectBipedSysDumps(a.Scene->container(), da);
	collectBipedSysDumps(b.Scene->container(), db);

	FILE *fp = fopen(outPath, "w");
	if (!fp) { std::cerr << "ERROR: cannot open " << outPath << " for writing\n"; return 1; }
	fprintf(fp, "DIFF-RIG A=%s (%zu sysobjs) B=%s (%zu sysobjs)\n", pathA, da.size(), pathB, db.size());
	diffBipedSysDumps(da, db, fp);
	fclose(fp);
	return 0;
}

// ---------------------------------------------------------------------------------------------
// Entry point for the max2gltf writer (pipeline_max_export_gltf compiles this file with
// PMB_ANIM_NO_MAIN): run exactly the standalone flow — $Bip01-first + EXPORT_NODE_ANIMATION
// selection in scene order, addAnimation per node — and hand back the serialized CAnimation.
// The bytes ride the glTF as the lossless nel_anim blob (dual representation per the plan:
// sampled interop channels are a later additive tier; the blob is the byte-exact carrier).
// Loads the .max through this tool's own loader/registry, independent of the caller's state.
// Returns 1 with animOut filled, 3 when nothing to export, -1 on load/serial failure.
int pmbExportAnimForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                         std::vector<uint8> &animOut,
                         std::vector<std::string> *bareNodesOut)
{
	NL3D::registerSerial3d(); // internally guarded

	CSceneClassContainer *ssc = lm.Scene->container();

	// Selection: $Bip01 first (case-insensitive), then every EXPORT_NODE_ANIMATION == "1" node
	// in scene order — identical to main below.
	std::vector<INode *> selection;
	std::set<INode *> selected;
	INode *rootNode = ssc->scene()->rootNode();
	INode *bip01 = rootNode ? rootNode->find(ucstring("Bip01")) : nullptr;
	if (!bip01)
	{
		for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end() && !bip01; ++it)
		{
			CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
			if (n && NLMISC::toLower(ucstring(n->userName()).toUtf8()) == "bip01") bip01 = n;
		}
	}
	if (bip01 && selected.insert(bip01).second) selection.push_back(bip01);
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (!n) continue;
		if (getNodeScriptAppDataString(n, NEL3D_APPDATA_EXPORT_NODE_ANIMATION) == "1")
			if (selected.insert(n).second) selection.push_back(n);
	}
	if (selection.empty())
		return 3;

	NL3D::CAnimation animFile;
	for (std::vector<INode *>::iterator si = selection.begin(); si != selection.end(); ++si)
	{
		INode *node = *si;
		std::string nodeName;
		std::string prefixe = getNodeScriptAppDataString(node, NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME);
		if (!prefixe.empty() && atoi(prefixe.c_str()) != 0)
		{
			nodeName = getNodeScriptAppDataString(node, NEL3D_APPDATA_INSTANCE_NAME);
			if (nodeName.empty())
				nodeName = ucstring(node->userName()).toUtf8();
			nodeName += ".";
		}
		bool root = node->parent() == rootNode;
		// Selection nodes exported with an EMPTY prefix own the bare "pos"/"rotquat"/"scale"
		// and "<target>MorphFactor" track names (the Ryzom per-node convention) — the glTF
		// sampled-channel tier needs to know which nodes those are.
		if (bareNodesOut && nodeName.empty())
			bareNodesOut->push_back(ucstring(node->userName()).toUtf8());
		addAnimation(animFile, *node, nodeName, root, ssc);
	}

	try
	{
		NLMISC::CMemStream ms;
		animFile.serial(ms);
		animOut.assign(ms.buffer(), ms.buffer() + ms.length());
	}
	catch (const NLMISC::Exception &e)
	{
		fprintf(stderr, "ERROR: anim serial failed for %s: %s\n", maxPath.c_str(), e.what());
		return -1;
	}
	return 1;
}

#ifndef PMB_ANIM_NO_MAIN
int main(int argc, char **argv)
{
	if (argc >= 5 && std::string(argv[1]) == "--diff-rig")
		return runDiffRig(argv[2], argv[3], argv[4]);
	if (argc >= 3 && std::string(argv[1]) == "--dump-rig")
		return runDumpRig(argv[2], argc >= 4 ? argv[3] : nullptr);
	if (argc >= 5 && std::string(argv[1]) == "--author-jump")
		return BIPAUTHOR::runAuthorJump(argv[2], argv[3], argv[4]);

	const char *dumpSamples = nullptr;
	double dumpMaxFrame = 60.0;
	const char *bipFile = nullptr;
	int argi = 1;
	while (argi < argc && argv[argi][0] == '-' && argv[argi][1] == '-')
	{
		if (std::string(argv[argi]) == "--dump-samples" && argi + 1 < argc) { dumpSamples = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--dump-max-frame" && argi + 1 < argc) { dumpMaxFrame = atof(argv[argi + 1]); argi += 2; }
		else if (std::string(argv[argi]) == "--bip" && argi + 1 < argc) { bipFile = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--com-node-prefix") { g_comNodePrefix = true; ++argi; }
		else break;
	}
	if (argc - argi < 2)
	{
		std::cerr << "usage: pipeline_max_export_anim [--bip <take.bip>] [--com-node-prefix] [--dump-samples <out.txt> [--dump-max-frame <f>]] <input.max> <output.anim>\n";
		std::cerr << "       pipeline_max_export_anim --diff-rig <A.max> <B.max> <out.txt>\n";
		std::cerr << "       pipeline_max_export_anim --author-jump <skel.max> <idle_source.max> <out.max>\n";
		std::cerr << "  --bip loads Character Studio motion keys from a .bip take (Snowballs workflow:\n";
		std::cerr << "        figure .max + bip/*.bip) and overrides the figure file's keytracks.\n";
		std::cerr << "  --com-node-prefix names a bare-based COM's tracks '<nodeName>.pos/.rotquat'\n";
		std::cerr << "        (Snowballs 2003 anim shape; Ryzom per-node refs are bare 'pos'/'rotquat').\n";
		std::cerr << "exit codes: 0 ok, 1 error, 3 nothing to export\n";
		return 1;
	}
	const char *maxFile = argv[argi];
	const char *animOut = argv[argi + 1];

	BIPANIM::SBipAnimKeys bipKeys;
	if (bipFile)
	{
		std::string err;
		if (!BIPANIM::loadBipFile(bipFile, bipKeys, err))
		{
			std::cerr << "ERROR: --bip " << bipFile << ": " << err << "\n";
			return 1;
		}
		g_bipOverrideKeys = &bipKeys;
		std::cerr << "BIP " << bipFile << ": range [" << bipKeys.RangeMin << ".." << bipKeys.RangeMax
		          << "] ticks\n";
	}

	NL3D::registerSerial3d();

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	CStorageOleIn in;
	if (!in.open(maxFile)) { std::cerr << "ERROR: not an OLE compound file: " << maxFile << "\n"; return 1; }

	CDllDirectory dll;
	{ std::vector<uint8> b; if (!in.readStream("DllDirectory", b)) { std::cerr << "ERROR: no DllDirectory stream\n"; return 1; } CStorageStream st(b); dll.serial(st); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ std::vector<uint8> b; if (!in.readStream("ClassDirectory3", b)) { std::cerr << "ERROR: no ClassDirectory3 stream\n"; return 1; } CStorageStream st(b); cd.serial(st); }
	cd.parse(VersionUnknown);

	CScene scene(&reg, &dll, &cd);
	{ std::vector<uint8> b; if (!in.readStream("Scene", b)) { std::cerr << "ERROR: no Scene stream\n"; return 1; } CStorageStream st(b); scene.serial(st); }
	scene.parse(VersionUnknown);

	CSceneClassContainer *ssc = scene.container();

	// --- Build the selection: $Bip01 first (case-insensitive, like MaxScript), then every node
	// with EXPORT_NODE_ANIMATION == "1", in scene order.
	std::vector<INode *> selection;
	std::set<INode *> selected;
	INode *rootNode = ssc->scene()->rootNode();
	INode *bip01 = rootNode ? rootNode->find(ucstring("Bip01")) : nullptr;
	// MaxScript $Bip01 finds the node anywhere in the scene, not only top-level — search the
	// whole container if the root-level find missed.
	if (!bip01)
	{
		for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end() && !bip01; ++it)
		{
			CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
			if (n && NLMISC::toLower(ucstring(n->userName()).toUtf8()) == "bip01") bip01 = n;
		}
	}
	if (dumpSamples)
	{
		if (!bip01) { std::cerr << "ERROR: --dump-samples needs a Bip01\n"; return 1; }
		return dumpBipedSamples(*bip01, ssc, dumpSamples, dumpMaxFrame);
	}
	if (bip01 && selected.insert(bip01).second) selection.push_back(bip01);
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
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
	for (std::vector<INode *>::iterator si = selection.begin(); si != selection.end(); ++si)
	{
		INode *node = *si;
		std::string nodeName;
		std::string prefixe = getNodeScriptAppDataString(node, NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME);
		if (!prefixe.empty() && atoi(prefixe.c_str()) != 0)
		{
			nodeName = getNodeScriptAppDataString(node, NEL3D_APPDATA_INSTANCE_NAME);
			if (nodeName.empty())
				nodeName = ucstring(node->userName()).toUtf8();
			nodeName += ".";
		}
		// root flag: the selected node's parent is the scene root (see addAnimation)
		bool root = node->parent() == rootNode;
		if (getenv("PMB_ANIM_DUMP_SELECTION"))
			fprintf(stderr, "PMB_ANIM_DUMP_SELECTION: node '%s' parent '%s' root=%d prefixe='%s'\n",
			        ucstring(node->userName()).toUtf8().c_str(),
			        node->parent() ? ucstring(node->parent()->userName()).toUtf8().c_str() : "(null)",
			        (int)root, prefixe.c_str());
		addAnimation(animFile, *node, nodeName, root, ssc);
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
#endif /* PMB_ANIM_NO_MAIN */

/* end of file */
