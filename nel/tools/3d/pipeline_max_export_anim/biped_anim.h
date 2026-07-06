/**
 * \file biped_anim.h
 * \brief Biped animation-key decode + evaluation for the headless .anim exporter.
 *
 * The biped stores its animation on the Biped (0x9155) system object as per-keytrack chunk
 * pairs (data chunk, time chunk). Decoded 2026-07-06 against direct-reference .anim exports;
 * see pipeline_max_design.md §10c for the full keytrack chunk map, record layouts and the
 * evaluation model. Summary:
 *
 *   0x012c/d horizontal  (COM xy; rec 10 floats: pos Y-up at [0..2])
 *   0x012e/f turn        (COM rot; hdr 3, rec 13: quat Y-up Max-conv at [4..7])
 *   0x0130/1 vertical    (COM z; TWO banks, hdr 1 each, rec 13: z at [2])
 *   0x0132/3 pelvis      (hdr 3, rec 7: quat at [0..3]; world = COM * Rx(pi)*conj(s)*(-.5,-.5,.5,-.5))
 *   0x0134/5 R arm       (hdr 4, rec 110)   0x0136/7 L arm
 *   0x0138/9 R leg       (hdr 4, rec 110)   0x013a/b L leg
 *   0x013c/d spine       (hdr 4, rec 10: 9*, 9 angles)
 *   0x013e/f head        (hdr 3, rec 12: quat4, 0,0,0, 3*, neck a1,a2,a3)
 *   0x0142/3 tail        (hdr 4, rec 10: 9*, 9 angles)
 *   0x0147/8 pony1       (hdr 4, rec 7: 6*, 6 angles)
 *   0x0149/a pony2       (like pony1)
 *
 * Limb record (110 floats): [0] hinge angle (elbow/knee), [1] ballistic tension, [2..5] upper
 * quat (COM-relative), [9]/[10] clavicle (a, b-delta) on arms, [11] pivot int, [12] IK blend,
 * [14..16]+1 end-effector pos body(COM Y-up), [18..20]+1 end-effector pos WORLD (Y-up),
 * [22..24]+0 unit vector, [28..31] foot/hand quat, [46+10k..49+10k] finger/toe base delta quat,
 * [54+10k],[55+10k] bend angles. ([50+10k..53+10k] etc are caches — ignore.)
 *
 * Time chunk: hdr 7 dwords (count, ?, ?, ?, ?, trackType, ?), then count x 10 dwords:
 * (time_ticks, index, p0..p4, tens, cont, bias) — TCB in Max UI units (25 = default);
 * tail/pony time recs are 26 dwords (per-link TCB param sets), times still at [0].
 *
 * Interpolation: per-channel TCB in STORED space (Max/NeL TCB formulas — see NeL track_tcb.h —
 * with the biped boundary rule tanFrom(first) = (1-t)(v1-v0), tanTo(last) = (1-t)(vlast-vprev)),
 * quats via squad on the raw stored key quats, then per-frame conversion with the time-varying
 * frames (COM/pelvis/parent). Legs/arms with IK blend keys additionally solve a 2-bone IK toward
 * the interpolated world end-effector position (approximation for in-between frames; exact at
 * keys — see the wiki's open-work note).
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

#ifndef PIPELINE_MAX_EXPORT_ANIM_BIPED_ANIM_H
#define PIPELINE_MAX_EXPORT_ANIM_BIPED_ANIM_H

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>
#include <nel/misc/quat.h>

#include <map>
#include <string>
#include <vector>

#include "../pipeline_max_rig/biped_rig.h"

namespace BIPANIM {

// Double-precision quaternion (x,y,z,w), standard column-vector convention.
struct QuatD
{
	double x, y, z, w;
	QuatD() : x(0), y(0), z(0), w(1) { }
	QuatD(double x_, double y_, double z_, double w_) : x(x_), y(y_), z(z_), w(w_) { }
	QuatD(const NLMISC::CQuat &q) : x(q.x), y(q.y), z(q.z), w(q.w) { }
	NLMISC::CQuat toFloat() const { return NLMISC::CQuat((float)x, (float)y, (float)z, (float)w); }
};

QuatD qMul(const QuatD &a, const QuatD &b);
QuatD qConj(const QuatD &q);
QuatD qNorm(const QuatD &q);
NLMISC::CVectorD qRotate(const QuatD &q, const NLMISC::CVectorD &v);
QuatD qAxisAngle(const NLMISC::CVectorD &axis, double angle);

// One keyframe channel with Max-style TCB interpolation (see header comment).
struct TCBScalarKey { sint32 Time; double Value; float Tens, Cont, Bias, EaseTo, EaseFrom; };
struct TCBScalarChannel
{
	std::vector<TCBScalarKey> Keys;
	std::vector<double> TanFrom, TanTo;
	void compile();
	double eval(double timeTicks) const;
	bool empty() const { return Keys.empty(); }
	sint32 firstTime() const { return Keys.front().Time; }
	sint32 lastTime() const { return Keys.back().Time; }
};

struct TCBQuatKey { sint32 Time; QuatD Quat; float Tens, Cont, Bias, EaseTo, EaseFrom; };
struct TCBQuatChannel
{
	std::vector<TCBQuatKey> Keys;
	std::vector<QuatD> A, B; // squad control points
	// Finger/toe BASE delta channels interpolate with per-segment cosine ease + slerp, ignoring
	// the TCB params entirely (zero slope through every key) — pinned by the differential anim
	// dataset's a_fk_toe (angle = 25 deg * sin^2(pi*u/2) exact at every quarter frame).
	bool CosineEase;
	TCBQuatChannel() : CosineEase(false) { }
	void compile();
	QuatD eval(double timeTicks) const;
	bool empty() const { return Keys.empty(); }
};

// Vector channel = 3 scalar channels sharing times/params.
struct TCBVec3Channel
{
	TCBScalarChannel X, Y, Z;
	void compile() { X.compile(); Y.compile(); Z.compile(); }
	NLMISC::CVectorD eval(double t) const { return NLMISC::CVectorD(X.eval(t), Y.eval(t), Z.eval(t)); }
	bool empty() const { return X.empty(); }
};

// One parsed keytrack: raw records + times + TCB params.
struct SBipKeyTrack
{
	std::vector<sint32> Times;
	std::vector<std::vector<float> > Recs;   // rec_size floats per key
	std::vector<float> Tens, Cont, Bias;     // internal units (-1..1)
	std::vector<float> EaseTo, EaseFrom;     // 0..1 (UI 0..50 / 50)
	bool empty() const { return Times.empty(); }
};

// All keytracks of one biped system object.
struct SBipAnimKeys
{
	SBipKeyTrack Horizontal, Turn, Vertical, Pelvis;
	SBipKeyTrack ArmR, ArmL, LegR, LegL;
	SBipKeyTrack Spine, Head, Tail, Pony1, Pony2;
	// Union of key ranges (ticks) over the keytracks of the EXISTING BipDriven bones — the COM
	// (BIPBODY) controller contributes nothing, matching the reference's buildBipedInformation
	// (its h/v/t keys are invisible to IKeyControl; verified against fy_hof_co_fus_tir, whose
	// turn/vertical tracks run past every limb track yet the reference range stops at the
	// limbs). false when no track has keys.
	bool HasRange;
	sint32 RangeMin, RangeMax;
	SBipAnimKeys() : HasRange(false), RangeMin(0), RangeMax(0) { }
};

// Parse the keytracks off a rig's system object. g_rig-independent (reads sys directly).
void parseBipAnimKeys(PIPELINE::MAX::CSceneClass *sys, SBipAnimKeys &out);

// Evaluated transform of one node at one time.
struct SBipNodeState
{
	QuatD WorldRot;
	NLMISC::CVectorD WorldPos; // only valid when HasPos
	bool HasPos;
	SBipNodeState() : HasPos(false) { }
};

// Per-rig animation evaluator. Built from the figure-time bone walk (PMAX_RIG) + parsed keys.
class CBipedAnimEval
{
public:
	// bones: figure-time walk output (Bone::Node / Bone::WorldTM set). rigSys: the 0x9155 object.
	CBipedAnimEval(PIPELINE::MAX::CSceneClass *rigSys, PMAX_RIG::SBipedRig &rig,
	               const std::vector<PMAX_RIG::Bone> &bones,
	               const std::map<PIPELINE::MAX::BUILTIN::INode *, size_t> &boneOfNode);

	// Evaluate every biped node of this rig at the given time (ticks). Results keyed by INode*.
	// Nodes without animation keys stay at their figure pose.
	void evalAt(double timeTicks, std::map<PIPELINE::MAX::BUILTIN::INode *, SBipNodeState> &out);

	const SBipAnimKeys &keys() const { return m_Keys; }

private:
	struct SNodeInfo
	{
		PIPELINE::MAX::BUILTIN::INode *Node;
		PIPELINE::MAX::BUILTIN::INode *Parent;
		bool IsCom;
		bool HasIdLink;
		uint32 Id, Link;
		// figure-time data
		QuatD FigWorldRot;
		NLMISC::CVectorD FigWorldPos;
		QuatD FigLocalRot;
		NLMISC::CVectorD FigLocalPos;
	};

	PIPELINE::MAX::CSceneClass *m_Sys;
	PMAX_RIG::SBipedRig *m_Rig;
	SBipAnimKeys m_Keys;
	std::vector<SNodeInfo> m_Nodes; // walk order (parents before children)
	std::map<PIPELINE::MAX::BUILTIN::INode *, size_t> m_NodeIdx;

	// figure-frame constants
	QuatD m_FigComRot;
	NLMISC::CVectorD m_FigComPos;
	// figure pelvis state — thigh positions anchor to the pelvis frame (the node hierarchy
	// differs by era: fresh Max 9 rigs parent thighs to the lowest spine link).
	QuatD m_FigPelvisRot;
	NLMISC::CVectorD m_FigPelvisPos;
	bool m_HaveFigPelvis;

	// compiled channels (lazy)
	TCBVec3Channel m_ChHorizontal;    // Y-up stored (x, y_up, z_up)
	TCBScalarChannel m_ChVertical;    // z (Y-up [2])
	TCBQuatChannel m_ChTurn;
	TCBQuatChannel m_ChPelvis;
	TCBQuatChannel m_ChUpper[2];      // [0]=R, [1]=L  (limb rec [2..5])
	TCBScalarChannel m_ChHinge[2];    // elbow
	TCBQuatChannel m_ChEnd[2];        // hand
	TCBScalarChannel m_ChClavA[2], m_ChClavB[2];
	TCBQuatChannel m_ChLegUpper[2];   // thigh
	TCBScalarChannel m_ChLegHinge[2]; // knee
	TCBScalarChannel m_ChLegAnkle[2]; // horse ankle (4-link legs)
	TCBQuatChannel m_ChLegEnd[2];     // foot
	TCBScalarChannel m_ChIkBlend[2][2];   // [arm/leg][side]
	TCBVec3Channel m_ChIkTarget[2][2];    // world end-effector pos (Y-up stored)
	std::vector<TCBScalarChannel> m_ChSpineAng, m_ChTailAng, m_ChPony1Ang, m_ChPony2Ang, m_ChNeckAng;
	TCBQuatChannel m_ChHead;
	std::vector<TCBQuatChannel> m_ChFingerBase[2], m_ChToeBase[2];
	std::vector<TCBScalarChannel> m_ChFingerBend[2], m_ChToeBend[2];

	void buildChannels();
	static void quatChannelFrom(const SBipKeyTrack &tr, int off, TCBQuatChannel &out);
	static void scalarChannelFrom(const SBipKeyTrack &tr, int off, TCBScalarChannel &out);
};

} /* namespace BIPANIM */

#endif /* PIPELINE_MAX_EXPORT_ANIM_BIPED_ANIM_H */

/* end of file */
