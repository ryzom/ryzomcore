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
// Biped rigs (a real Vertical/Horizontal/Turn COM controller in the scene) are refused with
// exit code 2 — the biped animation phase needs the oversampling path, not implemented yet.

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
				double n = sqrt(ax * ax + ay * ay + az * az);
				if (n > 0.0)
				{
					k.Value.Axis.set((float)(ax / n), (float)(ay / n), (float)(az / n));
					double l = std::min(n, 1.0);
					double angle = 2.0 * asin(l);
					if (keys[i].AbsQuat[3] < 0.0f) angle = 2.0 * NLMISC::Pi - angle;
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

static void addAnimation(NL3D::CAnimation &animation, INode &node, const std::string &baseName, CSceneClassContainer *ssc)
{
	// Non-biped path of CExportNel::addAnimation: the node's own tracks under the bare base
	// name, then every child subtree via addBoneTracks. (NoteTrack/SSS/material/morph tracks:
	// not present in the non-biped corpus, see addNodeTracks.)
	addNodeTracks(animation, node, baseName);
	std::vector<INode *> kids = orderedChildrenOf(&node, ssc);
	for (INode *child : kids)
		addBoneTracks(animation, *child, baseName, ssc);
}

// ---------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "usage: pipeline_max_export_anim <input.max> <output.anim>\n";
		std::cerr << "exit codes: 0 ok, 1 error, 2 biped rig (unsupported), 3 nothing to export\n";
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

	// --- Refuse real biped rigs (V/H/T transform on a selected node or anywhere): the biped
	// export path oversamples through the biped controller, which is not implemented yet.
	for (auto it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		CSceneClass *sc = dynamic_cast<CSceneClass *>(it->second);
		if (sc && sc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL)
		{
			std::cerr << "SKIP: biped rig (Vertical/Horizontal/Turn controller present), biped anim export not implemented: " << maxFile << "\n";
			return 2;
		}
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
