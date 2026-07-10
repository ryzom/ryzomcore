/**
 * \file biped_rig.cpp
 * \brief See biped_rig.h — extracted verbatim from pipeline_max_export_skel/main.cpp.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7
 * \author Claude Sonnet 5
 * \author Claude Fable 5
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

#include "biped_rig.h"

#include <nel/misc/common.h>

#include <algorithm>
#include <cmath>

// M_PI is not standard C++ (MSVC 9.0 / VS2008 does not define it in <cmath> without _USE_MATH_DEFINES);
// define it portably to the same double value glibc uses so both toolchains agree bit-for-bit.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <cstdio>
#include <cstring>

#include "../pipeline_max/builtin/storage/app_data.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/biped/biped_driven.h"

#include "max_scene.h" // non-biped walkNodeMax: DefaultPos from the Max quotient matrix

namespace PMAX_RIG {

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BIPED;



// Row-major 4x4 double matrix for accumulate-in-double mode. Only the operations we need:
// identity, TRS build from CVector/CQuat, multiply, affine inverse, extract IJKP as float.
// Not general (no projection support — none of our bones have it), but that's fine here.


// Global mode flag — flipped by --double CLI arg.
bool g_useDouble = false;

// Find a chunk by id in a container's orphaned or m_Chunks list.
// Since parse claims some chunks (moves to typed fields), and others stay in orphanedChunks,
// we check both.
IStorageObject *findChunkAnywhere(CSceneClass *sc, uint16 id)
{
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = sc->orphanedChunks().begin(); it != sc->orphanedChunks().end(); ++it)
		if (it->first == id) return it->second;
	// Also check m_Chunks (may still have entries pre-clean/build)
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = sc->chunks().begin(); it != sc->chunks().end(); ++it)
		if (it->first == id) return it->second;
	return NULL;
}

bool readRawBytes(CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes)
{
	IStorageObject *chunk = findChunkAnywhere(sc, chunkId);
	if (!chunk) return false;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw) return false;
	if (raw->Value.size() < nBytes) return false;
	memcpy(dst, nlVectorData(raw->Value), nBytes);
	return true;
}

// Read a controller's default-value chunk (0x2503/0x2504/0x2505). The keyframe controllers are
// typed now (CControlKeyFramerBase claims the default-value chunk out of the orphan list), so
// prefer the typed accessor and keep the raw orphan scan as the fallback for any controller
// class that is still an unknown pass-through.
bool readCtrlDefault(CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes)
{
	PIPELINE::MAX::BUILTIN::CControlKeyFramerBase *ctrl = dynamic_cast<PIPELINE::MAX::BUILTIN::CControlKeyFramerBase *>(sc);
	if (ctrl)
	{
		uint size = 0;
		const uint8 *data = ctrl->defaultValue(size);
		if (data && size >= nBytes)
		{
			memcpy(dst, data, nBytes);
			return true;
		}
	}
	return readRawBytes(sc, chunkId, dst, nBytes);
}

// PRS controller chunk ids (super 0x900b/c/d Bezier variants)
#define CHUNK_BEZIER_POS_VALUE 0x2503 // CVector
#define CHUNK_TCB_QUAT_VALUE   0x2504 // CQuat
#define CHUNK_BEZIER_SCALE_VALUE 0x2505 // CVector + CQuat (28 bytes) — take first CVector

// Node-level chunks (on every CNodeImpl, orphaned):
//   0x096a  12 bytes  CVector — pivot-like offset; observed all-zero on biped bones and small
//                                garbage-looking values on non-biped roots. Not used.
//   0x096b  16 bytes  CQuat — offset rotation; identity on biped bones, real rotation on some
//                              non-biped roots. Not used yet.
//   0x096c  28 bytes  CVector + CQuat — on non-biped nodes the CVector is scale (~1,1,1) and the
//                              quat is identity; on biped bones the CVector is the bone's own
//                              physical dimensions in Max biped's convention (X=length along the
//                              bone, Y=width, Z=depth), which for straight-chain descendants
//                              equals the LOCAL POSITION OFFSET of the child from this bone.
#define CHUNK_NODE_096C 0x096c

// Returns true if the node's TM controller (getReference(0)) is a biped BipDriven_Control
// (ClassId {0x9154, 0}). Uses classDesc() from the scene class registry which was populated by
// CSceneClassContainer::createChunkById at load time, so this is O(1).
bool isBipedBoneNode(INode *node)
{
	return dynamic_cast<CBipedDriven *>(node->getReference(0)) != NULL;
}

// NeL export properties live as AppData entries on the node (see pipeline_max_design.md §8);
// values are stored as strings written by the MaxScript utility panel.
#define NEL3D_APPDATA_BONE_LOD_DISTANCE 1423062615

// Read a float-valued NeL AppData script entry off a node. Matches by SubId only (the ClassId/
// SuperClassId key is always the MaxScript utility's), parses like the reference toFloatMax
// (first ',' becomes '.', dot-decimal parse). Returns def when absent or unparsable.
float getNodeScriptAppDataFloat(INode *node, uint32 subId, float def)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(node);
	if (!n) return def;
	PIPELINE::MAX::BUILTIN::STORAGE::CAppData *ad = n->appData();
	if (!ad) return def;
	for (PIPELINE::MAX::BUILTIN::STORAGE::CAppData::TMap::const_iterator it = ad->entries().begin(); it != ad->entries().end(); ++it)
	{
		if (it->first.SubId != subId) continue;
		PIPELINE::MAX::BUILTIN::STORAGE::CAppDataEntry *entry = it->second;
		CStorageRaw *raw = entry->value<CStorageRaw>();
		if (!raw) return def;
		std::string s(raw->Value.begin(), raw->Value.end());
		while (!s.empty() && (s[s.size() - 1] == '\0' || s[s.size() - 1] == '\n' || s[s.size() - 1] == '\r')) s.resize(s.size() - 1);
		std::string::size_type comma = s.find(',');
		if (comma != std::string::npos) s[comma] = '.';
		char *end = NULL;
		double v = strtod(s.c_str(), &end);
		if (end == s.c_str()) return def;
		return (float)v;
	}
	return def;
}

// Read 0x096c off the node itself and return the CVector part (Max biped bone dimensions).
// Returns (0,0,0) if the chunk isn't present.
NLMISC::CVector readNodeBoneDimensions(INode *node)
{
	NLMISC::CVector v = NLMISC::CVector::Null;
	CSceneClass *n = dynamic_cast<CSceneClass *>(node);
	if (!n) return v;
	IStorageObject *chunk = findChunkAnywhere(n, CHUNK_NODE_096C);
	if (!chunk) return v;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw || raw->Value.size() < 12) return v;
	memcpy(&v, nlVectorData(raw->Value), 12);
	return v;
}

// BipDriven Control (0x9154) stores its (biped_bone_id, link_index) pair as chunk 0x0200
// (8 bytes = 2 uint32s), now typed as CBipedDriven (see biped/biped_driven.h). The bone_id maps
// to the biped plugin's getIdLink table (12=pelvis, 9=spine, 11=head, etc.). We use it to
// distinguish "straight chain" bones (child.id == parent.id and child.link == parent.link + 1,
// e.g. Spine → Spine1) from "chain base" bones (child.id != parent.id, e.g. Pelvis → Spine
// crosses biped groups) — the two need different local-position rules.
bool readBipDrivenIdLink(INode *node, uint32 &boneId, uint32 &linkIdx)
{
	CBipedDriven *tmCtrl = dynamic_cast<CBipedDriven *>(node->getReference(0));
	if (!tmCtrl) return false;
	if (!tmCtrl->hasBipedIdLink()) return false;
	boneId = tmCtrl->bipedBoneId();
	linkIdx = tmCtrl->bipedLinkIndex();
	return true;
}

void getLocalTransform(CReferenceMaker *tmCtrl,
                              NLMISC::CVector &pos, NLMISC::CQuat &rot, NLMISC::CVector &scale)
{
	pos = NLMISC::CVector::Null;
	rot = NLMISC::CQuat::Identity;
	scale = NLMISC::CVector(1, 1, 1);

	if (!tmCtrl) return;
	if (tmCtrl->nbReferences() < 3) return;

	CReferenceMaker *posCtrl = tmCtrl->getReference(0);
	CReferenceMaker *rotCtrl = tmCtrl->getReference(1);
	CReferenceMaker *scaleCtrl = tmCtrl->getReference(2);

	CSceneClass *posSc = dynamic_cast<CSceneClass *>(posCtrl);
	CSceneClass *rotSc = dynamic_cast<CSceneClass *>(rotCtrl);
	CSceneClass *scaleSc = dynamic_cast<CSceneClass *>(scaleCtrl);

	if (posSc) readCtrlDefault(posSc, CHUNK_BEZIER_POS_VALUE, &pos, 12);
	if (rotSc && readCtrlDefault(rotSc, CHUNK_TCB_QUAT_VALUE, &rot, 16))
	{
		// Max stores rotation-controller values in the inverse convention relative to the node
		// TM rotation (the reference exporter never hit this because it read GetNodeTM matrices,
		// not controller values). Without this, every non-180deg/non-identity PRS rotation in the
		// corpus came out as the conjugate of the reference (xyz equal, w flipped) — 180deg and
		// identity rotations are self-conjugate, which is why they matched either way.
		rot.invert();
	}
	if (scaleSc) readCtrlDefault(scaleSc, CHUNK_BEZIER_SCALE_VALUE, &scale, 12);
}

// Biped plugin internal bone-id constants — 0-based, one less than the MaxScript-facing IDs in
// biped.getIdLink docs. Confirmed by dumping 0x0200 across fy_hom_skel: Bip01 Spine → id=8,
// Bip01 Neck → id=16, Bip01 Head → id=10, Bip01 Pelvis → id=11, L Clavicle group id=0, etc.


NLMISC::CMatrix makeLocalTM(const NLMISC::CVector &pos, const NLMISC::CQuat &rot, const NLMISC::CVector &scale)
{
	NLMISC::CMatrix m;
	m.identity();
	m.setPos(pos);
	m.setRot(rot);
	m.scale(scale);
	return m;
}

////////////////////////////////////////////////////////////////////////
// Biped figure-mode bind-pose reconstruction.
//
// The Max biped computes figure-mode transforms procedurally from state stored in the Biped
// (0x9155) system object. Reverse-engineering (2026-07, see pipeline_max_design.md) established:
//   * The biped internal frame is Y-up; NeL/Max world is Z-up. Position conversion (x,y,z)->(x,-z,y);
//     rotation basis change C=[[1,0,0],[0,0,-1],[0,1,0]].
//   * Each bone's world local-X axis points exactly at its child (aim==exact). Straight-continuation
//     bones therefore have identity local rotation; only direction-changing joints carry real rotation.
//   * Direction-changing joint rotations are stored as quaternions at fixed offsets in per-limb record
//     chunks, in either world or pelvis-relative frame, under a fixed component permutation+sign.
//     Validated stable across templates (fy/ma/tr, male/female): Thigh (0x0069[2], pelvis-rel),
//     UpperArm (0x006a[2], pelvis-rel), Foot (0x0069[28], world), Head (0x0064[0], pelvis-rel).
//   * Right side is the left mirror: q_R = (x, y, -z, -w).
//   * Structure records on the 0x9155 object (2026-07-05 decode session, validated corpus-wide):
//       0x000b spine, 0x000d pelvis, 0x000e tail, 0x0013 ponytail1 — layout
//       [0]=? [1..n]=per-link lengths [4x4 base-attach matrix]; n = chunkFloats - 17.
//       0x000f legs: per side half, [0..9] params ([1] = thigh side offset in the pelvis frame),
//       [10]=nToes(int), then per toe [nLinks(int)][4x4 matrix][nLinks lengths].
//       0x0010 arms: [0..15] params ([10]=clavicle Z offset), [16]=nFingers, then per finger
//       [nLinks(int)][4x4 matrix][nLinks lengths]; L half then R half (117 floats each on 5x3 hands).
//       0x006c COM record: [4..6] = COM position (Y-up), [8..11] = COM world rotation quat (Y-up).
//     Per-link rotation-angle records (3 floats (a1,a2,a3) per link, [0]=int count of floats):
//       0x0067 spine, 0x0068 tail, 0x006d ponytail1; neck angles inline in the head record 0x0064
//       ([0..3] head quat, [7]=int count, [8..] triples). Composition (empirical, corpus-validated):
//       R = Rx(a3) * Rz(-a1) * Ry(a2); chain-base local rotation = matrixRot * R.
//     Matrix conversion for these records: the 3x3 rows are the NeL I/J/K basis columns directly.
//       Chain bases (spine/tail/pony): local pos = (m13, m12, -m14).
//       Toe bases: local pos = (m12, m13, -m14), local rot additionally z-flipped (x,y,-z,-w);
//       right side = mirror of the left half's data (the R half's own matrices use yet another
//       basis and are not parsed).
//   * Base frame: COM world from 0x006c (fallback 0x0104); Pelvis local rot = (0.5,0.5,0.5,-0.5).
//   * Multiple bipeds can exist in one file (e.g. tr_mo_kitin_queen has Bip01 + Bip02): every
//     BipDriven Control and every Vertical/Horizontal/Turn COM controller references ITS OWN
//     0x9155 system object as getReference(0), so all state is kept per-system in SBipedRig.
//
// This reconstruction supplies correct local (pos,rot) for the decoded roles and falls back to the
// straight-chain approximation for the not-yet-decoded ones (Clavicle/Hand 2-DOF, props, footsteps),
// so partial coverage never regresses a bone below the previous approximation.

const NLMISC::CClassId CLASSID_BIPED_SYS(0x00009155, 0x00000000);
const NLMISC::CClassId CLASSID_BIPED_VHT_CTRL(0x00009156, 0x00000000);







SBipedRig::SBipedRig() : Sys(NULL), HasCom(false), ComPos(NLMISC::CVector::Null), ComRot(NLMISC::CQuat::Identity),
		ComDisp(NLMISC::CVector::Null), BaseFramePos(NLMISC::CVector::Null), HaveBaseFramePos(false),
		HeightCorrection(0.0f), HaveHeightCorrection(false),
		MoveAllTrans(NLMISC::CVector::Null), HaveMoveAll(false), ComDispNonZero(false),
		DynamicsType(0), HaveDynamicsType(false),
		HaveLinkParentInv(false), HaveBaseFrameTM(false),
		HasThighZ(false), MaxLegLink(2),
		HasClavicleZ(false),
		PelvisWorldRot(NLMISC::CQuat::Identity), HavePelvisWorldRot(false),
		LastSpineWorldRot(NLMISC::CQuat::Identity), HaveLastSpineWorldRot(false),
		PelvisRecTrans(NLMISC::CVector::Null), HavePelvisWorldTM(false),
		HaveLastSpineWorldTM(false),
		BodyType(3),
		ToeBaseWorldZ(0.0f), HaveToeBaseWorldZ(false),
		AnkleWorldZ(0.0f), HaveAnkleWorldZ(false),
		FootstepsBoneIdx(-1)
	{
		ThighZ[0] = ThighZ[1] = 0.0f;
		ClavicleZ[0] = ClavicleZ[1] = 0.0f;
		ClavicleA[0] = ClavicleA[1] = 0.0f;
		ClavicleB[0] = ClavicleB[1] = 0.0f;
	}


// Rigs per Biped system object; cleared per file.
std::map<CSceneClass *, SBipedRig> g_bipedRigs;
// The rig currently being decoded by getBipedLocal (set by walkNode before the call). This keeps
// the joint-decode helpers below (bipedChunkFloats & co) signature-compatible.
SBipedRig *g_rig = NULL;

// Fetch a raw float array from a chunk on the current rig's biped object (orphaned or m_Chunks).
// Returns NULL if absent or too short. Optionally returns the total float count.
const float *bipedChunkFloats(uint16 chunkId, size_t minFloats, size_t *countOut)
{
	if (!g_rig || !g_rig->Sys) return NULL;
	IStorageObject *chunk = findChunkAnywhere(g_rig->Sys, chunkId);
	if (!chunk) return NULL;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw || raw->Value.size() < minFloats * 4) return NULL;
	if (countOut) *countOut = raw->Value.size() / 4;
	return reinterpret_cast<const float *>(nlVectorData(raw->Value));
}

// Paired pose records (legs 0x0069, arms 0x006a) and the paired sections of the structure records
// (0x000f, 0x0010) store the RIGHT side in the first half and the LEFT side in the second half.
// Established 2026-07-06 via the Max 9 differential dataset (single-bone rotations: R Hand lands at
// [28..31], L Hand at [half+28..]; R Clavicle at 0x0010[6..7], L at [half+6..7]). The earlier
// assumption of left-first was undetectable on symmetric rigs (L == mirror(R)) and is exactly what
// broke on the asymmetric-edited ones (bird-rig L forearm, kami_guide_4 L toe).
// Returns a pointer to the given side's half (NULL when missing) and the half length.
const float *bipedSideHalf(uint16 chunkId, bool leftSide, size_t minFloats, size_t *halfOut)
{
	size_t n = 0;
	const float *f = bipedChunkFloats(chunkId, 2 * minFloats, &n);
	if (!f) return NULL;
	size_t half = n / 2;
	if (half < minFloats) return NULL;
	if (halfOut) *halfOut = half;
	return leftSide ? (f + half) : f;
}

// The biped runtime bakes a tiny constant twist into several of its procedural default frames —
// measured 0.0008 rad (0.0456 deg) on the Max 9 differential dataset, invariant across rig
// structure, pose and scale, and confirmed present in the corpus era (19 corpus hands become
// bit-exact with it, 0 without). Appears in the hand default (Rx(-/+(pi/2 - eps)) off the forearm)
// and as an Rz(-eps) factor inside the clavicle orientation.
const float BIPED_EPS_TWIST = 0.0008f;

uint32 floatBitsAsUint(float f)
{
	uint32 u;
	memcpy(&u, &f, 4);
	return u;
}

// Quat from a record matrix whose 3x3 rows are the NeL I/J/K basis columns directly.
NLMISC::CQuat matRowsIJKQuat(const float *m)
{
	NLMISC::CMatrix rm;
	rm.identity();
	rm.setRot(NLMISC::CVector(m[0], m[1], m[2]),
	          NLMISC::CVector(m[4], m[5], m[6]),
	          NLMISC::CVector(m[8], m[9], m[10]));
	NLMISC::CQuat q = rm.getRot();
	q.normalize();
	return q;
}

// Empirical 3-DOF per-link angle composition (see the header comment): R = Rx(a3) * Rz(-a1) * Ry(a2).
NLMISC::CQuat chainAngleQuat(const NLMISC::CVector &a)
{
	NLMISC::CQuat qx(NLMISC::CAngleAxis(NLMISC::CVector(1.0f, 0.0f, 0.0f), a.z));
	NLMISC::CQuat qz(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), -a.x));
	NLMISC::CQuat qy(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 1.0f, 0.0f), a.y));
	NLMISC::CQuat r = qx * qz * qy;
	r.normalize();
	return r;
}

// Fresh-format (BodyType 0) chain layout: element positions include the first-order shift
// [Rz(-eps) - I] . A(own angles) . (own length, 0, 0) — the runtime lays the matrix-based chains
// (spine/tail/pony) out with an eps-twisted tip per element while the orientations use the
// corrected angles. Confirmed exactly on the regen corpus (tr_mo_c03 base: dx +183um dy -49um
// both predicted; kakty/chonari/ryzerb/capryni link steps match to ~2um across bend angles
// 0..90 deg). Legacy rigs (BodyType 3) don't have this — call sites gate on BodyType.
NLMISC::CVector chainEpsShift(const NLMISC::CVector &angles, float len)
{
	NLMISC::CQuat a = chainAngleQuat(angles);
	NLMISC::CMatrix am;
	am.identity();
	am.setRot(a);
	NLMISC::CVector v = am.mulVector(NLMISC::CVector(len, 0.0f, 0.0f));
	NLMISC::CQuat qe(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), -BIPED_EPS_TWIST));
	NLMISC::CMatrix em;
	em.identity();
	em.setRot(qe);
	return em.mulVector(v) - v;
}

// The leg pose-record half's head grows by 2 floats per leg link beyond 3 (observed: 3-link
// rigs put the thigh quat at [2], foot quat at [28]; 4-link rigs at [4]/[30] — confirmed on the
// Max 9 dataset's s_leg4 AND the corpus mounts/birds). The knee ([0]) and horse ankle ([1])
// hinge slots do NOT shift. Everything at or after the thigh quat does.
int legHeadShift()
{
	if (!g_rig) return 0;
	int s = 2 * (g_rig->MaxLegLink - 2);
	return s > 0 ? s : 0;
}

// Read a stored quat (x,y,z,w) from a side's half at the given in-half offset.
bool readSideQuat(uint16 chunkId, bool leftSide, int off, NLMISC::CQuat &out)
{
	const float *f = bipedSideHalf(chunkId, leftSide, (size_t)off + 4);
	if (!f) return false;
	out.set(f[off + 0], f[off + 1], f[off + 2], f[off + 3]);
	float n = sqrtf(out.x*out.x + out.y*out.y + out.z*out.z + out.w*out.w);
	if (n < 1e-6f) return false;
	out.x /= n; out.y /= n; out.z /= n; out.w /= n;
	return true;
}

// Mirror a left-side world/local rotation to the right side: (x,y,z,w) -> (x,y,-z,-w).
NLMISC::CQuat mirrorQuatLR(const NLMISC::CQuat &q)
{
	return NLMISC::CQuat(q.x, q.y, -q.z, -q.w);
}

// The biped-internal Y-up basis change as a rotation: C = [[1,0,0],[0,0,-1],[0,1,0]] = Rx(+90deg).
const NLMISC::CQuat QUAT_C(0.70710678f, 0.0f, 0.0f, 0.70710678f);
// 180-degree axis rotations used by the per-side upperarm frames.
const NLMISC::CQuat QUAT_I(1.0f, 0.0f, 0.0f, 0.0f); // Rx(pi)
const NLMISC::CQuat QUAT_J(0.0f, 1.0f, 0.0f, 0.0f); // Ry(pi)

// Thigh: stored per side at [2..5] of the 0x0069 half, pelvis-relative, gathered as
// q = (s2, s3, -s0, s1) — which yields the LEFT-convention rotation; the right half is
// mirror-encoded, so the right side applies the LR mirror to its own half's decode. (The
// differential dataset's canonical poses are mirror-degenerate for exactly these quat shapes,
// which is why per-side "no mirror" also passed there; the corpus discriminates.)
bool thighWorldRot(bool leftSide, NLMISC::CQuat &worldRot)
{
	if (!g_rig || !g_rig->HavePelvisWorldRot) return false;
	int base = 2 + legHeadShift();
	const float *f = bipedSideHalf(0x0069, leftSide, (size_t)base + 4);
	if (!f) return false;
	NLMISC::CQuat q(f[base + 2], f[base + 3], -f[base + 0], f[base + 1]);
	q.normalize();
	if (!leftSide) q = mirrorQuatLR(q);
	worldRot = g_rig->PelvisWorldRot * q;
	worldRot.normalize();
	return true;
}

// Foot: stored per side at [28..31] of the 0x0069 half as the ABSOLUTE foot orientation in the
// COM-relative Y-up frame: R_foot = R_com * C * qmat(s)^T — both sides direct, NO mirror (each
// half stores its own side's absolute orientation; verified exact on the corpus for both sides
// including the free-footed bird rigs). Replaces the old "world quat, planted feet only"
// reading, whose fixed perm/sign silently baked in the canonical -90deg COM yaw and broke on
// tilted-COM rigs and non-planted feet.
bool footWorldRot(bool leftSide, NLMISC::CQuat &worldRot)
{
	if (!g_rig || !g_rig->HasCom) return false;
	NLMISC::CQuat s;
	if (!readSideQuat(0x0069, leftSide, 28 + legHeadShift(), s)) return false;
	s.invert(); // qmat(s)^T = qmat(conj(s))
	worldRot = g_rig->ComRot * QUAT_C * s;
	worldRot.normalize();
	return true;
}

// UpperArm: stored per side at [2..5] of the 0x006a half, pelvis-relative with per-side fixed
// 180-degree frame factors:  L: R = R_pelvis * Rx(pi) * qmat(s)^T * Ry(pi)
//                            R: R = R_pelvis * Ry(pi) * qmat(s)^T * Rx(pi)
// (differential dataset, exact at 7e-7 across baseline + sweeps + single-side rotations; the
// previous perm/sign gather was off by ~4 degrees even on the dataset baseline).
bool upperArmWorldRot(bool leftSide, NLMISC::CQuat &worldRot)
{
	if (!g_rig || !g_rig->HavePelvisWorldRot) return false;
	NLMISC::CQuat s;
	if (!readSideQuat(0x006a, leftSide, 2, s)) return false;
	s.invert();
	worldRot = leftSide ? (g_rig->PelvisWorldRot * QUAT_I * s * QUAT_J)
	                    : (g_rig->PelvisWorldRot * QUAT_J * s * QUAT_I);
	worldRot.normalize();
	return true;
}

// Hand: the hand's procedural default is the forearm frame twisted by -/+(pi/2 - eps) about its
// own X; the stored quat at [28..31] of the 0x006a half is a local post-multiplied delta in the
// Y-up conversion frame:  R_hand = R_forearm * Rx(-/+(pi/2-eps)) * C * qmat(s)^T * C^T.
// (Differential dataset: composition exact at 4e-7 on the sweeps; eps confirmed corpus-era.)
bool handWorldRot(bool leftSide, const NLMISC::CQuat &forearmWorldRot, NLMISC::CQuat &worldRot)
{
	NLMISC::CQuat s;
	if (!readSideQuat(0x006a, leftSide, 28, s)) return false;
	s.invert();
	float twist = (float)(M_PI / 2.0) - BIPED_EPS_TWIST;
	if (leftSide) twist = -twist;
	NLMISC::CQuat tw(NLMISC::CAngleAxis(NLMISC::CVector(1.0f, 0.0f, 0.0f), twist));
	NLMISC::CQuat cInv = QUAT_C; cInv.invert();
	NLMISC::CQuat delta = QUAT_C * s * cInv;
	if (!leftSide) delta = mirrorQuatLR(delta); // right half is mirror-encoded (see thighWorldRot)
	worldRot = forearmWorldRot * tw * delta;
	worldRot.normalize();
	return true;
}

// Head: stored at 0x0064[0..3], pelvis-relative, identity component order (corpus-validated).
bool headWorldRot(NLMISC::CQuat &worldRot)
{
	if (!g_rig || !g_rig->HavePelvisWorldRot) return false;
	const float *f = bipedChunkFloats(0x0064, 4);
	if (!f) return false;
	NLMISC::CQuat q(f[0], f[1], f[2], f[3]);
	float n = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
	if (n < 1e-6f) return false;
	q.x /= n; q.y /= n; q.z /= n; q.w /= n;
	worldRot = g_rig->PelvisWorldRot * q;
	worldRot.normalize();
	return true;
}

// Hinge joints (SDK: GetHingeVal / GetHorseAnkleVal — knee, elbow and the 4-link-leg horse ankle
// are 1-DOF). The figure value sits at the head of the side's pose-record half: the knee/elbow
// interior angle at [0] (local z-rotation = [0] - pi), the horse ankle at [1] (local z-rotation
// = [1] as-is). Bit-exact across the era-matched corpus except one small known deviation
// (kitin-family calf 5.0deg; the bird-rig L forearm 0.5deg resolved with the R-first half fix).
// Returns false when the record is missing.
bool hingeLocalRot(uint16 chunkId, bool leftSide, int slot, bool interior, NLMISC::CQuat &out)
{
	const float *f = bipedSideHalf(chunkId, leftSide, (size_t)slot + 1);
	if (!f) return false;
	float a = f[slot] - (interior ? (float)M_PI : 0.0f);
	out = NLMISC::CQuat(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), a));
	out.normalize();
	return true;
}

// Per-finger/toe pose block inside the 0x0069 / 0x006a half: blocks of 10 floats starting at
// in-half offset 46 — [46+10k .. 49+10k] = chain k's base-link delta quat (local post-multiplied:
// local = baseLocal * qmat(s)^T), [54+10k], [55+10k], ... = the non-base links' absolute bend
// angles in radians (local rotation = Rz(angle); the record matrices hold only the creation-time
// state, the pose block is authoritative). Differential dataset, exact at 1e-6 either side.
bool chainPoseBaseDelta(uint16 chunkId, bool leftSide, int chainIdx, NLMISC::CQuat &delta)
{
	int shift = (chunkId == 0x0069) ? legHeadShift() : 0;
	if (!readSideQuat(chunkId, leftSide, 46 + shift + 10 * chainIdx, delta)) return false;
	delta.invert();
	return true;
}

bool chainPoseLinkAngle(uint16 chunkId, bool leftSide, int chainIdx, int sub, float &angle)
{
	if (sub < 1) return false;
	int shift = (chunkId == 0x0069) ? legHeadShift() : 0;
	int off = 54 + shift + 10 * chainIdx + (sub - 1);
	const float *f = bipedSideHalf(chunkId, leftSide, (size_t)off + 1);
	if (!f) return false;
	angle = f[off];
	return true;
}

// Parse a chain structure record (spine 0x000b / tail 0x000e / pony1 0x0013): [0]=?,
// [1..n]=per-link lengths, then the 4x4 base-attach matrix; n = total - 17. The companion angle
// record ([0]=int float-count, then (a1,a2,a3) per link) fills Angles.
void parseChainRecord(uint16 structId, uint16 angleId, SBipedChain &out)
{
	size_t n = 0;
	const float *s = bipedChunkFloats(structId, 17, &n);
	if (s && n >= 17)
	{
		size_t k = n - 17;
		if (k <= 64)
		{
			out.Lens.assign(s + 1, s + 1 + k);
			const float *m = s + 1 + k;
			out.MatRot = matRowsIJKQuat(m);
			out.MatPos = NLMISC::CVector(m[13], m[12], -m[14]);
			out.HasMat = true;
		}
	}
	size_t an = 0;
	const float *a = angleId ? bipedChunkFloats(angleId, 1, &an) : NULL;
	if (a && an >= 1)
	{
		uint32 cnt = floatBitsAsUint(a[0]);
		if (cnt <= 3 * 64 && 1 + cnt <= an)
			for (uint32 i = 0; i + 2 < cnt; i += 3)
				out.Angles.push_back(NLMISC::CVector(a[1 + i], a[2 + i], a[3 + i]));
	}
}

// Parse the leg record (0x000f): per-side thigh offsets + per-toe base matrices/lengths from
// each side's half (both halves share the encoding; see the Toes comment in SBipedRig).
void parseLegRecord(SBipedRig &rig)
{
	size_t n = 0;
	const float *f = bipedChunkFloats(0x000f, 12, &n);
	if (!f) return;
	size_t half = n / 2;
	rig.ThighZ[0] = f[1];
	rig.ThighZ[1] = (half + 1 < n) ? f[half + 1] : f[1];
	rig.HasThighZ = true;
	for (int side = 0; side < 2; ++side)
	{
		size_t base = side ? half : 0;
		size_t end = side ? n : half;
		size_t i = base + 10;
		if (i >= end) break;
		uint32 nToes = floatBitsAsUint(f[i]); ++i;
		if (nToes > 16) continue;
		for (uint32 t = 0; t < nToes; ++t)
		{
			if (i >= end) break;
			uint32 nl = floatBitsAsUint(f[i]); ++i;
			if (nl < 1 || nl > 16 || i + 16 + nl > end) break;
			SBipedToe toe;
			toe.NLinks = (int)nl;
			const float *m = f + i; i += 16;
			// toe base: rows-as-IJK quat with the (x,y,-z,-w) z-flip; pos (x,y,-z)
			toe.Rot = mirrorQuatLR(matRowsIJKQuat(m));
			toe.Pos = NLMISC::CVector(m[12], m[13], -m[14]);
			toe.Lens.assign(f + i, f + i + nl);
			i += nl;
			rig.Toes[side].push_back(toe);
		}
	}
}

// Parse the arm record (0x0010): per-side clavicle params + per-finger base matrices/lengths
// (from the first/right half; see parseLegRecord).
// Finger matrices use the Y-up conversion (position (x,-z,y); rotation C*M^T), unlike the
// toe/chain records — validated per record type, not assumed uniform.
void parseArmRecord(SBipedRig &rig)
{
	size_t n = 0;
	const float *f = bipedChunkFloats(0x0010, 17, &n);
	if (!f) return;
	size_t half = n / 2;
	rig.ClavicleZ[0] = f[10];
	rig.ClavicleA[0] = f[6];
	rig.ClavicleB[0] = f[7];
	rig.ClavicleOff[0] = NLMISC::CVector(f[9], f[8], -f[10]);
	if (half + 10 < n)
	{
		rig.ClavicleZ[1] = f[half + 10];
		rig.ClavicleA[1] = f[half + 6];
		rig.ClavicleB[1] = f[half + 7];
		rig.ClavicleOff[1] = NLMISC::CVector(-f[half + 9], -f[half + 8], f[half + 10]);
	}
	else
	{
		rig.ClavicleZ[1] = rig.ClavicleZ[0];
		rig.ClavicleA[1] = rig.ClavicleA[0];
		rig.ClavicleB[1] = rig.ClavicleB[0];
		rig.ClavicleOff[1] = NLMISC::CVector(-rig.ClavicleOff[0].x, -rig.ClavicleOff[0].y, -rig.ClavicleOff[0].z);
	}
	rig.HasClavicleZ = true;
	for (int side = 0; side < 2; ++side)
	{
		size_t base = side ? half : 0;
		size_t end = side ? n : half;
		if (base + 16 >= end) break;
		uint32 nFingers = floatBitsAsUint(f[base + 16]);
		if (nFingers > 16) continue;
		size_t i = base + 17;
		for (uint32 fi = 0; fi < nFingers; ++fi)
		{
			if (i >= end) break;
			uint32 nl = floatBitsAsUint(f[i]); ++i;
			if (nl < 1 || nl > 16 || i + 16 + nl > end) break;
			SBipedFinger fing;
			fing.NLinks = (int)nl;
			const float *m = f + i; i += 16;
			fing.Pos = NLMISC::CVector(m[12], -m[14], m[13]);
			NLMISC::CVector I(m[0], -m[2], m[1]);
			NLMISC::CVector J(m[4], -m[6], m[5]);
			NLMISC::CVector K(m[8], -m[10], m[9]);
			NLMISC::CMatrix rm; rm.identity(); rm.setRot(I, J, K);
			fing.Rot = rm.getRot();
			fing.Rot.normalize();
			fing.Lens.assign(f + i, f + i + nl);
			i += nl;
			rig.Fingers[side].push_back(fing);
		}
	}
}

// Parse the COM record (0x006c: [4..6] position Y-up, [8..11] world rotation quat Y-up), with
// 0x0104 (Y-up 4x4, canonical -90degZ rotation assumed) as the fallback when 0x006c is absent.
// 0x0104 is additionally read into BaseFramePos independently of whether 0x006c succeeded — see
// the field comment in biped_rig.h and pipeline_max_design.md §10n: the two normally agree, and
// figure height (0x006c) is the well-supported general default where they don't (evidence tally,
// §10n "Seventh") — a per-file divergence is healed by any later Figure Mode entry/exit for any
// reason (§10n "Tenth", confirmed live in Max), which is why no byte-level rule distinguishes the
// files where it stayed diverged.
void parseComRecord(SBipedRig &rig)
{
	size_t n = 0;
	const float *c = bipedChunkFloats(0x006c, 12, &n);
	if (c)
	{
		rig.ComRot = NLMISC::CQuat(-c[8], c[10], -c[9], c[11]);
		rig.ComRot.normalize();
		// [0..2] = current V/H/T displacement in the COM frame; the Bip01 node's world position
		// is the figure COM plus this displacement rotated into the world. Exact on every
		// era-matched corpus file (z-component discriminated by tr_mo_kitifly/kitikil).
		rig.ComDisp = NLMISC::CVector(c[0], -c[2], c[1]);
		// A nonzero figure-mode ComDisp signals the COM holds a committed non-figure pose (the
		// current-position frame is then what Max evaluates for the unkeyed COM); exactly zero when
		// the COM sits at figure. Discriminates base-wins (mort_idle, decoupe) from figure-wins
		// (recruteur, meca) — see the field comment and pipeline_max_design.md §10n.
		rig.ComDispNonZero = (c[0] != 0.0f) || (c[1] != 0.0f) || (c[2] != 0.0f);
		NLMISC::CMatrix rm; rm.identity(); rm.setRot(rig.ComRot);
		rig.ComPos = NLMISC::CVector(c[4], -c[6], c[5]) + rm.mulVector(rig.ComDisp);
		rig.HasCom = true;
	}
	size_t nb = 0;
	const float *com = bipedChunkFloats(0x0104, 15, &nb);
	if (com)
	{
		rig.BaseFramePos = NLMISC::CVector(com[12], -com[14], com[13]);
		rig.HaveBaseFramePos = true;
		// Full current-position frame as a matrix. NOTE this is the full basis-change similarity
		// B·Mᵀ·Bᵀ (both sides), NOT the one-sided C·Mᵀ world-rotation rule of §10 item 1 — a TM
		// composed against other TMs must have both its input and output spaces converted; the
		// resulting NeL columns are (I, -K, J) of the §10 per-vector forms. The translation gains
		// the 0x0260[0..2] correction below. Pairs with 0x0112 for the linked-COM attach
		// (biped_rig.h field comment).
		rig.BaseFrameTM.identity();
		rig.BaseFrameTM.setRot(
			NLMISC::CVector(com[0], -com[2], com[1]),
			NLMISC::CVector(-com[8], com[10], -com[9]),
			NLMISC::CVector(com[4], -com[6], com[5]));
		rig.BaseFrameTM.setPos(rig.BaseFramePos);
		rig.HaveBaseFrameTM = true;
		if (!rig.HasCom)
		{
			rig.ComPos = rig.BaseFramePos;
			rig.ComRot = NLMISC::CQuat(0.0f, 0.0f, -0.70710678f, 0.70710678f);
			rig.HasCom = true;
		}
	}
	// Height-correction at 0x0260[0..2] — same 12-float record layout as 0x006c (whose own [0..2]
	// is the ComDisp there; 0x0260's [0..2] is an additive CORRECTION on the current-position
	// translation). Originally decoded as the scalar [1] alone (the vertical component — closes
	// ship_tank_karavan_mort_idle's 5cm residual exactly, §10n "Thirteenth"); the linkcom probe
	// round (§10m-ter) generalized it to the full vector: 0x0104.t + this = the exact
	// current-position translation (float-exact on the kitin run/stun/queen linked COMs).
	size_t nh = 0;
	const float *hc = bipedChunkFloats(0x0260, 3, &nh);
	if (hc)
	{
		rig.HeightCorrection = hc[1];
		rig.HaveHeightCorrection = true;
		if (rig.HaveBaseFrameTM)
			rig.BaseFrameTM.setPos(rig.BaseFramePos + NLMISC::CVector(hc[0], -hc[2], hc[1]));
	}
	// Linked-COM parent snapshot (0x0112, 12 floats: 3x3 rows + translation in PLAIN world
	// coordinates — not Y-up like the biped records): the INVERSE of the parent's world TM
	// captured when the link relationship was last established/edited. Identity on unlinked/root
	// rigs; the linked-COM local is LinkParentInvTM * BaseFrameTM (see biped_rig.h).
	size_t nl = 0;
	const float *lp = bipedChunkFloats(0x0112, 12, &nl);
	if (lp)
	{
		rig.LinkParentInvTM.identity();
		rig.LinkParentInvTM.setRot(
			NLMISC::CVector(lp[0], lp[1], lp[2]),
			NLMISC::CVector(lp[3], lp[4], lp[5]),
			NLMISC::CVector(lp[6], lp[7], lp[8]));
		rig.LinkParentInvTM.setPos(NLMISC::CVector(lp[9], lp[10], lp[11]));
		rig.HaveLinkParentInv = true;
	}
	// Move All Mode reference-frame transform (0x0117, Y-up affine 4x4; translation row [12,13,14]
	// = (x, z_up, -y) -> NeL (m12, -m14, m13)). Identity on every shipped file (no-op there); read
	// so any future Move-All file exports the offset. See the field comment / §10n.
	size_t nm = 0;
	const float *ma = bipedChunkFloats(0x0117, 16, &nm);
	if (ma)
	{
		rig.MoveAllTrans = NLMISC::CVector(ma[12], -ma[14], ma[13]);
		rig.HaveMoveAll = true;
	}
	// <biped_ctrl>.dynamicsType, confirmed at this offset via an isolated single-chunk A/B toggle
	// on a real skeleton (§10n "Ninth"/"Tenth") — read for corpus-wide auditing; not consumed by
	// any decision here (already confirmed not to affect the exported COM position, §10n).
	size_t nd = 0;
	const float *dt = bipedChunkFloats(0x0012, 1, &nd);
	if (dt)
	{
		rig.DynamicsType = (int)dt[0]; // stored as an actual float (0.0/1.0), not raw int bits
		rig.HaveDynamicsType = true;
	}
}

// Parse the pelvis record (0x000d): [0]=int 3, [1]=pelvis width param, [2..17]=4x4 attach matrix.
// The matrix translation is zero on the legacy corpus; fresh-format rigs store the offset between
// the last spine link's end (its stored length) and the actual neck attach point here (regen-corpus
// GT: fy_hom neck sits at Lens.back() - 0.0789 with 0x000d trans = (-0.0789, 0.00025, ~0)).
void parsePelvisRecord(SBipedRig &rig)
{
	size_t n = 0;
	const float *f = bipedChunkFloats(0x000d, 17, &n);
	if (!f) return;
	// Variable-length head ([0]=int record type/count, then 1..2 width params — one on humanoid
	// rigs, two on the kami-guide family), 4x4 matrix always at the END (like parseChainRecord).
	const float *m = f + (n - 16);
	rig.PelvisRecTrans = NLMISC::CVector(m[13], m[12], -m[14]);
}

// Parse neck angles. The authoritative source is the neck record 0x0065 ([0]=int count of angle
// floats = 3 per link, [1..count] = (a1,a2,a3) triples, then a 4x4 base-attach matrix) — the Max 9
// differential dataset shows neck edits land there, and 48/169 corpus files carry stale copies in
// the head record's [8..] region (tr_mo_arma/bul, the ge_ kami pair). Fall back to the head-record
// copy (0x0064: [7]=int count, [8..] triples) only when 0x0065 is absent.
void parseNeckAngles(SBipedRig &rig)
{
	size_t n = 0;
	const float *k = bipedChunkFloats(0x0065, 2, &n);
	if (k)
	{
		uint32 cnt = floatBitsAsUint(k[0]);
		if (cnt <= 3 * 64 && 1 + cnt <= n)
		{
			for (uint32 i = 0; i + 2 < cnt; i += 3)
				rig.NeckAngles.push_back(NLMISC::CVector(k[1 + i], k[2 + i], k[3 + i]));
			return;
		}
	}
	const float *h = bipedChunkFloats(0x0064, 8, &n);
	if (!h) return;
	uint32 cnt = floatBitsAsUint(h[7]);
	if (cnt > 3 * 64 || 8 + cnt > n) return;
	for (uint32 i = 0; i + 2 < cnt; i += 3)
		rig.NeckAngles.push_back(NLMISC::CVector(h[8 + i], h[9 + i], h[10 + i]));
}

// Locate (toe index, link-within-toe) from a cumulative BID_L/RTOES link index. Returns false if
// the record doesn't cover the link.
bool locateChainSub(const std::vector<SBipedToe> &toes, uint32 link, int &toeIdx, int &sub)
{
	int rem = (int)link;
	for (size_t t = 0; t < toes.size(); ++t)
	{
		if (rem < toes[t].NLinks) { toeIdx = (int)t; sub = rem; return true; }
		rem -= toes[t].NLinks;
	}
	return false;
}

bool locateChainSub(const std::vector<SBipedFinger> &fingers, uint32 link, int &fi, int &sub)
{
	int rem = (int)link;
	for (size_t t = 0; t < fingers.size(); ++t)
	{
		if (rem < fingers[t].NLinks) { fi = (int)t; sub = rem; return true; }
		rem -= fingers[t].NLinks;
	}
	return false;
}

// Compute the local (pos,rot) for a biped bone, given its already-computed parent world rotation.
// Also captures the Pelvis world rotation for later pelvis-relative joints. scale is always identity
// for biped bones. Sets *worldRotOut to this bone's world rotation (for children to consume).
// g_rig must point at the bone's rig (set by walkNode from the BipDriven's system reference).
void getBipedLocal(INode *node, const NLMISC::CMatrix &parentWorld,
                          NLMISC::CVector &pos, NLMISC::CQuat &rot, NLMISC::CVector &scale,
                          NLMISC::CQuat &worldRotOut)
{
	NLMISC::CQuat parentWorldRot = parentWorld.getRot();
	pos = NLMISC::CVector::Null;
	rot = NLMISC::CQuat::Identity;
	scale = NLMISC::CVector(1, 1, 1);
	SBipedRig &rig = *g_rig;

	INode *parent = node->parent();
	uint32 id = 0, link = 0;
	bool haveId = readBipDrivenIdLink(node, id, link);

	// --- Determine world rotation and (for matrix-based roles) an explicit local transform. ---
	NLMISC::CQuat worldRot = parentWorldRot; // default: straight-chain inherits parent direction
	bool haveLocalDirect = false;            // matrix-based roles set pos+rot directly
	bool havePosOverride = false;            // roles that set pos but derive rot from worldRot
	NLMISC::CVector posOverride = NLMISC::CVector::Null;

	bool isLeft = (id == BID_LARM || id == BID_LLEG || id == BID_LFINGERS || id == BID_LTOES);
	NLMISC::CQuat wq;
	bool decoded = false;

	// Every paired decode reads the bone's own side's half (R half first — see bipedSideHalf);
	// no mirror step anywhere. This is what fixes the asymmetric-edited rigs.
	if (haveId && (id == BID_LLEG || id == BID_RLEG) && link == 0) decoded = thighWorldRot(id == BID_LLEG, wq); // Thigh
	else if (haveId && (id == BID_LLEG || id == BID_RLEG) && (int)link == rig.MaxLegLink) decoded = footWorldRot(id == BID_LLEG, wq); // Foot (last leg link)
	else if (haveId && (id == BID_LARM || id == BID_RARM) && link == 1) decoded = upperArmWorldRot(id == BID_LARM, wq); // UpperArm
	else if (haveId && (id == BID_LARM || id == BID_RARM) && link == 3) decoded = handWorldRot(id == BID_LARM, parentWorldRot, wq); // Hand (forearm = walk parent)
	else if (haveId && id == BID_HEAD && link == 0) decoded = headWorldRot(wq); // Head
	else if (haveId && (id == BID_LLEG || id == BID_RLEG) && link == 1) // Calf: knee hinge
	{
		NLMISC::CQuat hq;
		if (hingeLocalRot(0x0069, id == BID_LLEG, 0, true, hq))
		{
			rot = hq;
			worldRot = parentWorldRot * rot;
			NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
			pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
			haveLocalDirect = true;
		}
	}
	else if (haveId && (id == BID_LLEG || id == BID_RLEG) && (int)link == 2 && rig.MaxLegLink == 3) // HorseLink ankle hinge
	{
		NLMISC::CQuat hq;
		if (hingeLocalRot(0x0069, id == BID_LLEG, 1, false, hq))
		{
			rot = hq;
			worldRot = parentWorldRot * rot;
			NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
			pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
			haveLocalDirect = true;
		}
	}
	else if (haveId && (id == BID_LARM || id == BID_RARM) && link == 2) // Forearm: elbow hinge
	{
		NLMISC::CQuat hq;
		if (hingeLocalRot(0x006a, id == BID_LARM, 0, true, hq))
		{
			rot = hq;
			worldRot = parentWorldRot * rot;
			NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
			pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
			haveLocalDirect = true;
		}
	}

	if (decoded)
	{
		worldRot = wq;
		if (haveId && (id == BID_LLEG || id == BID_RLEG) && link == 0 && rig.HasThighZ)
		{
			// Thigh position: pure side offset in the PELVIS frame, per-side leg record value.
			// The walk parent depends on the era (legacy corpus: pelvis; Max 9 triangle-pelvis
			// rigs: lowest spine link), so when the parent isn't the pelvis, anchor through the
			// pelvis world matrix and convert into the parent frame. The parent-is-pelvis path
			// stays the direct local value to keep the legacy corpus bit-stable.
			NLMISC::CVector t(0.0f, 0.0f, (id == BID_LLEG) ? rig.ThighZ[1] : -rig.ThighZ[0]);
			uint32 pid = 0, plink = 0;
			bool parentIsPelvis = parent && readBipDrivenIdLink(parent, pid, plink) && pid == BID_PELVIS;
			if (!parentIsPelvis && rig.HavePelvisWorldTM)
			{
				NLMISC::CVector wp = rig.PelvisWorldTM * t;
				NLMISC::CMatrix pinvM = parentWorld;
				pinvM.invert();
				posOverride = pinvM * wp;
			}
			else
			{
				posOverride = t;
			}
			havePosOverride = true;
		}
	}
	else if (haveId && id == BID_PELVIS) // Pelvis: constant COM->pelvis frame reorientation
	{
		worldRot = parentWorldRot * NLMISC::CQuat(0.5f, 0.5f, 0.5f, -0.5f);
		// The pelvis sits at the figure COM; the Bip01 node is displaced from it by the current
		// V/H/T value, so the pelvis' local position undoes that displacement.
		posOverride = -rig.ComDisp;
		havePosOverride = true;
	}
	else if (haveId && (id == BID_LARM || id == BID_RARM) && link == 0 && rig.HasClavicleZ)
	{
		// Clavicle: position is a pure Z (side) offset off the last spine link (per-side arm
		// record [10]); rotation is the full 2-DOF orientation relative to the LAST SPINE LINK
		// (not the neck, the walk parent — SDK: clavicle inherits from the last spine link):
		//   rel_L = Rx(-a) * Rz(-eps) * B(b),  B(b) = 180deg about (cos phi, 0, sin phi),
		//   phi = pi/4 + b/2;  rel_R = LR-mirror of rel computed from the right side's (a,b).
		// (Differential dataset: exact at 2e-6 on both sides across both DOF sweeps; the old
		// b-only formula ignored [6] and the eps twist.)
		int si = isLeft ? 1 : 0;
		float a = rig.ClavicleA[si], bb = rig.ClavicleB[si];
		float phi = (float)(M_PI / 4.0) + bb * 0.5f;
		NLMISC::CQuat qb(cosf(phi), 0.0f, sinf(phi), 0.0f);
		NLMISC::CQuat qa(NLMISC::CAngleAxis(NLMISC::CVector(1.0f, 0.0f, 0.0f), -a));
		NLMISC::CQuat qe(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), -BIPED_EPS_TWIST));
		NLMISC::CQuat rel = qa * qe * qb;
		if (!isLeft) rel = mirrorQuatLR(rel);
		rel.normalize();
		const NLMISC::CQuat &spineRef = rig.HaveLastSpineWorldRot ? rig.LastSpineWorldRot : parentWorldRot;
		worldRot = spineRef * rel;
		worldRot.normalize();
		posOverride = rig.ClavicleOff[isLeft ? 1 : 0];
		// Fresh-format rigs anchor the clavicle at the END OF THE LAST SPINE LINK, offset by
		// (f[9], f[8], +-f[10]) in the LAST SPINE LINK's frame — not the walk parent's (neck)
		// frame. On straight rigs the two coincide; the kitin family's 22.5-degree neck bend
		// discriminates (regen GT reproduced to 3e-4 under the spine-frame rule).
		if (rig.BodyType == 0 && rig.HaveLastSpineWorldTM && !rig.Spine.Lens.empty())
		{
			NLMISC::CVector spineEnd = rig.LastSpineWorldTM * NLMISC::CVector(rig.Spine.Lens.back(), 0.0f, 0.0f);
			NLMISC::CMatrix rl;
			rl.identity();
			rl.setRot(rig.LastSpineWorldTM.getRot());
			NLMISC::CVector wp = spineEnd + rl.mulVector(posOverride);
			NLMISC::CMatrix pinvM = parentWorld;
			pinvM.invert();
			posOverride = pinvM * wp;
		}
		havePosOverride = true;
	}
	else if (haveId && (id == BID_LFINGERS || id == BID_RFINGERS) && !(rig.Fingers[0].empty() && rig.Fingers[1].empty())) // fingers
	{
		// NOTE: both sides decode the FIRST (right) half's matrices — per-side decode regresses
		// (the other half can go stale; verified both directions). On the corpus's one fresh-format
		// rig (tr_mo_kitin_queen, BodyType 0) the staleness is even per-record: its x24-scaled
		// FINGER bases are current only in the LEFT half while its TOE bases are current only in
		// the right — no consistent selection rule is derivable from one file, so the legacy
		// right-half read stays until more fresh-format corpus material exists. The pose-block
		// deltas remain per-side.
		int srcHalf = rig.Fingers[0].empty() ? 1 : 0;
		const std::vector<SBipedFinger> &fingers = rig.Fingers[srcHalf];
		int fi = 0, sub = 0;
		if (locateChainSub(fingers, link, fi, sub))
		{
			const SBipedFinger &fing = fingers[fi];
			if (sub == 0)
			{
				// finger base: creation-time local transform from the arm-record matrix, composed
				// with the pose block's per-side base delta (local post-multiply; the right half's
				// delta is mirror-encoded, so the mirror wraps the composed local).
				NLMISC::CQuat delta;
				bool haveDelta = chainPoseBaseDelta(0x006a, isLeft, fi, delta);
				rot = fing.Rot;
				if (haveDelta) rot = rot * delta;
				if (isLeft) { pos = fing.Pos; }
				else { pos = NLMISC::CVector(fing.Pos.x, fing.Pos.y, -fing.Pos.z); rot = mirrorQuatLR(rot); }
				rot.normalize();
				haveLocalDirect = true;
				worldRot = parentWorldRot * rot;
			}
			else
			{
				// finger link: absolute bend angle from the pose block, local rotation Rz(angle).
				float angle;
				if (chainPoseLinkAngle(0x006a, isLeft, fi, sub, angle))
				{
					rot = NLMISC::CQuat(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), angle));
					rot.normalize();
					worldRot = parentWorldRot * rot;
					// position stays the straight-chain rule: the parent bone's 0x096c dimensions
					// reflect per-bone figure scaling (rubber-banding), which the record lengths
					// do NOT (observed on tr_mo_kitin_queen, figure-scaled ~24x).
					NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
					pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
					haveLocalDirect = true;
				}
			}
		}
	}
	else if (haveId && (id == BID_LTOES || id == BID_RTOES) && !(rig.Toes[0].empty() && rig.Toes[1].empty())) // toes
	{
		bool tLeft = (id == BID_LTOES);
		int tSrcHalf = rig.Toes[0].empty() ? 1 : 0;
		const std::vector<SBipedToe> &toes = rig.Toes[tSrcHalf]; // right half for both sides (see fingers)
		int ti = 0, sub = 0;
		if (locateChainSub(toes, link, ti, sub))
		{
			const SBipedToe &toe = toes[ti];
			if (sub == 0)
			{
				// toe base: creation-time local transform from the leg-record matrix, composed
				// with the pose block's per-side base delta (local post-multiply; the mirrored
				// side's delta is mirror-encoded, so the mirror wraps the composed local).
				// Which side the ROTATION matrix stores directly flipped between eras: legacy
				// corpus files hold the LEFT toe rotation direct (right = LR mirror), fresh-format
				// (Max 9) files hold the RIGHT one direct (left = LR mirror) — regen corpus GT,
				// discriminated by tr_mo_kakty (R toe) and tr_mo_kami_guide_4 (both sides). The
				// POSITION keeps the left-direct/right-z-negate rule in BOTH eras (ryzerb-family
				// GT: record z -0.4599, L toe -0.4599, R toe +0.4599).
				NLMISC::CQuat delta;
				bool haveDelta = chainPoseBaseDelta(0x0069, tLeft, ti, delta);
				rot = toe.Rot;
				if (haveDelta) rot = rot * delta;
				bool rotDirectSide = (rig.BodyType == 0) ? !tLeft : tLeft;
				if (!rotDirectSide) rot = mirrorQuatLR(rot);
				if (tLeft) pos = toe.Pos;
				else pos = NLMISC::CVector(toe.Pos.x, toe.Pos.y, -toe.Pos.z);
				rot.normalize();
				haveLocalDirect = true;
				worldRot = parentWorldRot * rot;
			}
			else
			{
				// toe link: absolute bend angle from the pose block (same layout as fingers).
				float angle;
				if (chainPoseLinkAngle(0x0069, tLeft, ti, sub, angle))
				{
					rot = NLMISC::CQuat(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), angle));
					rot.normalize();
					worldRot = parentWorldRot * rot;
					NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
					pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
					haveLocalDirect = true;
				}
			}
		}
	}
	else if (haveId && id == BID_SPINE && link == 0 && rig.Spine.HasMat)
	{
		// Spine base: attach matrix from the spine record (COM-relative), composed with the
		// per-link angles when present. Fresh-format (BodyType 0) base-link a1 angles carry
		// a baked +BIPED_EPS_TWIST the runtime subtracts (regen corpus: stored a1 differs from
		// the legacy twin's by exactly eps on every matrix-based chain base; GT poses exclude it).
		rot = rig.Spine.MatRot;
		if (!rig.Spine.Angles.empty())
		{
			NLMISC::CVector a0 = rig.Spine.Angles[0];
			if (rig.BodyType == 0) a0.x -= BIPED_EPS_TWIST;
			rot = rot * chainAngleQuat(a0);
			if (rig.BodyType == 0 && !rig.Spine.Lens.empty())
				pos += chainEpsShift(a0, rig.Spine.Lens[0]);
		}
		rot.normalize();
		pos += rig.Spine.MatPos;
		haveLocalDirect = true;
		worldRot = parentWorldRot * rot;
	}
	else if (haveId && id == BID_SPINE && link > 0 && link < rig.Spine.Angles.size())
	{
		rot = chainAngleQuat(rig.Spine.Angles[link]);
		worldRot = parentWorldRot * rot;
		// Spine link position: the previous link's stored length (like tail/pony links).
		if (link - 1 < rig.Spine.Lens.size())
		{
			pos = NLMISC::CVector(rig.Spine.Lens[link - 1], 0.0f, 0.0f);
			if (rig.BodyType == 0 && link < rig.Spine.Lens.size())
				pos += chainEpsShift(rig.Spine.Angles[link], rig.Spine.Lens[link]);
		}
		else
		{
			NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
			pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
		}
		haveLocalDirect = true;
	}
	else if (haveId && id == BID_NECK && link < rig.NeckAngles.size())
	{
		rot = chainAngleQuat(rig.NeckAngles[link]);
		worldRot = parentWorldRot * rot;
		if (link == 0 && !rig.Spine.Lens.empty())
		{
			// Neck base sits at the end of the last spine link (its stored length — the last
			// spine link's own 0x096c dims don't track it; confirmed on kami_keep_2), displaced
			// by the pelvis record's attach translation (zero on the legacy corpus; fresh-format
			// rigs keep the neck off the stored spine length by this offset — regen-corpus GT).
			pos = NLMISC::CVector(rig.Spine.Lens.back(), 0.0f, 0.0f) + rig.PelvisRecTrans;
		}
		else
		{
			NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
			pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
		}
		haveLocalDirect = true;
	}
	else if (haveId && (id == BID_TAIL || id == BID_PONY1 || id == BID_PONY2) && link == 0)
	{
		const SBipedChain &chain = (id == BID_TAIL) ? rig.Tail : (id == BID_PONY1) ? rig.Pony1 : rig.Pony2;
		if (chain.HasMat)
		{
			rot = chain.MatRot;
			if (!chain.Angles.empty())
			{
				// Fresh-format base-link a1 carries the baked +eps (see the spine base).
				NLMISC::CVector a0 = chain.Angles[0];
				if (rig.BodyType == 0) a0.x -= BIPED_EPS_TWIST;
				rot = rot * chainAngleQuat(a0);
			}
			rot.normalize();
			pos = chain.MatPos;
			haveLocalDirect = true;
			worldRot = parentWorldRot * rot;
			// The tail record is PELVIS-relative (like the thigh offset). The legacy corpus parents
			// the tail node to the pelvis, so parent-frame composition was correct by construction;
			// Max 9 triangle-pelvis rigs parent it to the lowest spine link — anchor through the
			// pelvis world matrix there. Pony records stay head-relative (parented to the head in
			// both eras).
			if (id == BID_TAIL)
			{
				uint32 pid = 0, plink = 0;
				bool parentIsPelvis = parent && readBipDrivenIdLink(parent, pid, plink) && pid == BID_PELVIS;
				if (!parentIsPelvis && rig.HavePelvisWorldTM)
				{
					worldRot = rig.PelvisWorldTM.getRot() * rot;
					worldRot.normalize();
					NLMISC::CQuat pinv = parentWorldRot;
					pinv.invert();
					rot = pinv * worldRot;
					rot.normalize();
					NLMISC::CVector wp = rig.PelvisWorldTM * pos;
					NLMISC::CMatrix pinvM = parentWorld;
					pinvM.invert();
					pos = pinvM * wp;
				}
			}
		}
	}
	else if (haveId && (id == BID_TAIL || id == BID_PONY1 || id == BID_PONY2) && link > 0)
	{
		const SBipedChain &chain = (id == BID_TAIL) ? rig.Tail : (id == BID_PONY1) ? rig.Pony1 : rig.Pony2;
		if (link < chain.Angles.size())
		{
			rot = chainAngleQuat(chain.Angles[link]);
			worldRot = parentWorldRot * rot;
			// Link position: the previous link's stored length. Unlike fingers/toes, the chain
			// links' 0x096c dimensions do NOT track the actual figure spacing (observed on
			// ca_hom_armor01 ponytails and every tail rig) — the record lengths are authoritative.
			if (link - 1 < chain.Lens.size())
				pos = NLMISC::CVector(chain.Lens[link - 1], 0.0f, 0.0f);
			else
			{
				NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
				pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
			}
			haveLocalDirect = true;
		}
	}
	else if (haveId && (id == BID_TAILNUB || id == BID_PONY1NUB || id == BID_NECKNUB))
	{
		// Chain end-effector dummies: constant 180deg-about-(1,1,0)/sqrt2 local rotation on every
		// corpus instance (113 tail nubs, 135 pony1 nubs, 62 neck dummies checked). Tail/pony nub
		// position = last link's stored length.
		NLMISC::CQuat nubRot(-0.70710678f, -0.70710678f, 0.0f, 0.0f);
		worldRot = parentWorldRot * nubRot;
		const SBipedChain *chain = (id == BID_TAILNUB) ? &rig.Tail : (id == BID_PONY1NUB) ? &rig.Pony1 : NULL;
		if (chain && !chain->Lens.empty())
		{
			posOverride = NLMISC::CVector(chain->Lens.back(), 0.0f, 0.0f);
			havePosOverride = true;
		}
	}
	else if (haveId && (id == BID_RFINGERNUB || id == BID_LTOENUB))
	{
		// Finger/toe end-effector marker (no children, so the "aim at child" rule that gives
		// straight-chain bones identity local rotation doesn't apply here). Confirmed constant
		// 180-deg-about-Z local rotation across all 9 biped templates checked (fy/tr/zo/ma male,
		// fy/tr/zo/ca female, ca male) for ids 22 (R finger nub) and 24 (L toe nub); the mirror
		// ids 23 (L finger nub) / 25 (R toe nub) are already exact via the identity default below,
		// so only these two need the override.
		worldRot = parentWorldRot * NLMISC::CQuat(0.0f, 0.0f, -1.0f, 0.0f);
	}

	// (Nub positions use the straight-chain default: the parent finger/toe tip's 0x096c length is
	// the figure-scaled value; the record lengths are unscaled template values.)

	// --- Positions for the non-matrix roles (straight-chain + known offsets). ---
	if (!haveLocalDirect)
	{
		// local rotation = parentWorldRot^-1 * worldRot
		NLMISC::CQuat pinv = parentWorldRot; pinv.invert();
		rot = pinv * worldRot;
		rot.normalize();

		if (havePosOverride)
		{
			pos = posOverride;
		}
		else if (haveId && id == BID_SPINE && link == 0)
		{
			// Spine base without a spine record: position from COM ~ own length along X.
			NLMISC::CVector selfDims = readNodeBoneDimensions(node);
			pos = NLMISC::CVector(selfDims.x, 0.0f, 0.0f);
		}
		else if (haveId && (id == BID_LARM || id == BID_RARM) && link == 0)
		{
			// Clavicle without a parsed arm record: fall back to the per-side offset vector
			// (zero when the record is missing entirely).
			pos = rig.ClavicleOff[isLeft ? 1 : 0];
		}
		else if (!parent || !isBipedBoneNode(parent))
		{
			// Root-attached biped bone (Pelvis etc.): local pos = 0.
			pos = NLMISC::CVector::Null;
		}
		else
		{
			// Straight-chain: child sits at parent's length along parent local X.
			NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
			pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
		}
	}

	worldRotOut = worldRot;

	// Capture Pelvis world rotation for pelvis-relative joints (Pelvis precedes them in the walk).
	if (haveId && id == BID_PELVIS)
	{
		rig.PelvisWorldRot = worldRot;
		rig.HavePelvisWorldRot = true;
	}
	// Capture the last spine link's world rotation for the clavicle reference frame. Spine links
	// are walked in order, so the last assignment wins — by the time a clavicle (a descendant of
	// the last spine link through the neck) is processed, this holds the correct frame.
	if (haveId && id == BID_SPINE)
	{
		rig.LastSpineWorldRot = worldRot;
		rig.HaveLastSpineWorldRot = true;
	}
}

// --- MAXScript regeneration support (--maxscript) ---------------------------------------------
// Captures, during the walk, everything needed to emit a Max 9 MAXScript that recreates the biped
// from OUR decoded reconstruction: structure counts via biped.createNew, then per-bone figure-mode
// world transforms via biped.setTransform. Running the emitted script in Max 9 regenerates a
// "clean" fresh-format rig whose figure state should match the original — cross-validating the
// decode in the encode direction (Max re-derives its own records from our values).

std::vector<SMsBone> g_msBones;

// Build ordered children list per parent by scanning the CSceneClassContainer in scene order.
// INode::children() is a std::set keyed by pointer, so its iteration order is unstable across
// runs — Max preserves original scene order (which is how bones get numbered in the .skel).
std::vector<INode *> orderedChildrenOf(INode *parent, CSceneClassContainer *ssc)
{
	std::vector<INode *> out;
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		// CRootNode inherits INode but doesn't override parent() — INode::parent() nlerrors,
		// so we must only call parent() on CNodeImpl (which does override it).
		CNodeImpl *n = dynamic_cast<CNodeImpl *>(it->second);
		if (n && n->parent() == parent) out.push_back(n);
	}
	return out;
}

// Resolve the Biped (0x9155) system object owning a node's TM controller. Both the per-bone
// BipDriven Control (0x9154) and the COM's Vertical/Horizontal/Turn controller (0x9156) reference
// their system object as getReference(0). Returns NULL for non-biped controllers.
CSceneClass *bipedSystemOfCtrl(CReferenceMaker *tmCtrl)
{
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(tmCtrl);
	if (!tmsc) return NULL;
	NLMISC::CClassId cid = tmsc->classDesc()->classId();
	if (cid != CBipedDriven::ClassId && cid != CLASSID_BIPED_VHT_CTRL) return NULL;
	CSceneClass *sys = dynamic_cast<CSceneClass *>(tmCtrl->getReference(0));
	if (!sys || sys->classDesc()->classId() != CLASSID_BIPED_SYS) return NULL;
	return sys;
}

// True if the node is a biped COM node (TM controller is Vertical/Horizontal/Turn).
bool isBipedComNode(INode *node)
{
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node->getReference(0));
	return tmsc && tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL;
}

// Highest leg link index (last leg segment = Foot) among this rig's BipDriven bones. The
// SDK-documented leg chain is Thigh(0), Calf(1), [HorseLink(2) only on 4-link mount/horse rigs],
// Foot(last). Defaults to 2 (3-link legs) when no leg bones are found.
void computeMaxLegLink(SBipedRig &rig, CSceneClassContainer *ssc)
{
	int maxLink = -1;
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
	{
		INode *node = dynamic_cast<INode *>(it->second);
		if (!node) continue;
		CBipedDriven *bd = dynamic_cast<CBipedDriven *>(node->getReference(0));
		if (!bd || !bd->hasBipedIdLink()) continue;
		if (dynamic_cast<CSceneClass *>(bd->getReference(0)) != rig.Sys) continue;
		uint32 id = bd->bipedBoneId();
		if (id == BID_LLEG || id == BID_RLEG)
			maxLink = std::max(maxLink, (int)bd->bipedLinkIndex());
	}
	if (maxLink >= 0) rig.MaxLegLink = maxLink;
}

// Get (or lazily parse) the rig state for a Biped system object.
SBipedRig &rigFor(CSceneClass *sys, CSceneClassContainer *ssc)
{
	std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.find(sys);
	if (it != g_bipedRigs.end()) return it->second;
	SBipedRig &rig = g_bipedRigs[sys];
	rig.Sys = sys;
	g_rig = &rig; // bipedChunkFloats reads through g_rig during parsing
	{
		const float *v = bipedChunkFloats(0x0115, 1);
		if (v) rig.BodyType = (int)floatBitsAsUint(v[0]);
	}
	parseComRecord(rig);
	parseLegRecord(rig);
	parseArmRecord(rig);
	parsePelvisRecord(rig);
	parseChainRecord(0x000b, 0x0067, rig.Spine);
	parseChainRecord(0x000e, 0x0068, rig.Tail);
	parseChainRecord(0x0013, 0x006d, rig.Pony1);
	parseChainRecord(0x0014, 0x006e, rig.Pony2);
	parseNeckAngles(rig);
	computeMaxLegLink(rig, ssc);
	return rig;
}

// walkNode has two overloads for the two accumulation modes. They're structurally identical
// apart from the parent-world matrix type and the InvBindPos build step, so they're kept
// side-by-side rather than templated for readability.
void walkNode(INode *node, sint32 fatherId, const NLMISC::CMatrix &parentWorld,
                     CSceneClassContainer *ssc,
                     std::vector<Bone> &bones, std::set<std::string> &nameSet)
{
	Bone b;
	std::string name = ucstring(node->userName()).toUtf8();
	if (!nameSet.insert(name).second) name += "_Second";
	b.Name = name;
	b.FatherId = fatherId;
	// Default for standard PRS controllers is FALSE (bone inherits parent's scale).
	// buildSkeleton flips this to true only when the controller's inheritance flags have
	// any of INHERIT_SCL_X|Y|Z set, or when the parent is a biped node. For the non-biped
	// files we handle, the default holds — no controller-flag reader implemented yet.
	b.UnheritScale = false;
	// Bone LOD disable distance: NeL AppData on the node (skeleton LODs are built from these
	// in writeSkel, same algorithm as CSkeletonShape::build).
	b.LodDisableDistance = std::max(0.0f, getNodeScriptAppDataFloat(node, NEL3D_APPDATA_BONE_LOD_DISTANCE, 0.0f));

	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	CReferenceMaker *tmCtrl = node->getReference(0);
	CSceneClass *bipedSys = bipedSystemOfCtrl(tmCtrl);
	if (bipedSys)
	{
		SBipedRig &rig = rigFor(bipedSys, ssc);
		g_rig = &rig;
		if (isBipedComNode(node) && rig.HasCom)
		{
			// COM node (Bip01, or a nested Bip02 on multi-biped rigs): world transform comes from
			// the rig's COM record; local = parentWorld^-1 * world.
			realScale = NLMISC::CVector(1, 1, 1);
			NLMISC::CQuat pinv = parentWorld.getRot(); pinv.invert();
			realRot = pinv * rig.ComRot;
			NLMISC::CMatrix pinvM = parentWorld; pinvM.invert();
			realPos = pinvM * rig.ComPos;
			b.UnheritScale = true; // reference exporter sets it on the COM node too (checked vs ref bytes)
		}
		else if (isBipedBoneNode(node))
		{
			// Figure-mode biped reconstruction (see getBipedLocal). Supplies correct local
			// transforms for the decoded roles; not-yet-decoded joints inherit their parent's
			// direction.
			NLMISC::CQuat worldRotOut;
			getBipedLocal(node, parentWorld, realPos, realRot, realScale, worldRotOut);
			b.UnheritScale = true; // biped bones always unherit their parent's scale (see plugin_max/nel_mesh_lib)
		}
		else
		{
			getLocalTransform(tmCtrl, realPos, realRot, realScale);
		}
	}
	else
	{
		getLocalTransform(tmCtrl, realPos, realRot, realScale);
		// PRS children of a biped COM node don't inherit the COM's rotation: their stored
		// rotation value is world-frame (position stays parent-relative). Verified bit-exact on
		// every corpus 'name' tag marker, including tilted-COM rigs (tr_mo_c03).
		INode *parent = node->parent();
		if (parent && isBipedComNode(parent))
		{
			NLMISC::CQuat pinv = parentWorld.getRot(); pinv.invert();
			realRot = pinv * realRot;
		}
		// The reference exporter's getNELUnHeritFatherScale also sets UnheritScale when the
		// PARENT is a biped node — so plain PRS bones hanging off biped bones (Footsteps, the
		// 'name'/'cheveux'/weapon-box markers) unherit too, not just the biped bones themselves.
		// Surfaced by the float-level field validation: 862 corpus bones had this flag wrong,
		// invisible in the aggregate byte-match percentage.
		if (parent && (isBipedComNode(parent) || isBipedBoneNode(parent)))
			b.UnheritScale = true;
	}
	realRot.normalize();

	NLMISC::CMatrix localTM = makeLocalTM(realPos, realRot, realScale);
	NLMISC::CMatrix worldTM = parentWorld * localTM;

	// REAL local transform preserved for glTF emission (see the writeGltf comment).
	b.OrigPos = realPos;
	b.OrigRot = realRot;

	if (fatherId < 0)
	{
		b.DefaultPos = NLMISC::CVector::Null;
		b.DefaultRotQuat = NLMISC::CQuat::Identity;
	}
	else
	{
		b.DefaultPos = realPos;
		b.DefaultRotQuat = realRot;
	}
	b.DefaultScale = realScale;

	NLMISC::CMatrix invBind;
	invBind.identity();
	invBind.setRot(worldTM.getI(), worldTM.getJ(), worldTM.getK());
	invBind.setPos(worldTM.getPos());
	invBind.invert();
	b.InvBindPos = invBind;

	b.Node = node;
	b.WorldTM = worldTM;

	sint32 myId = (sint32)bones.size();
	bones.push_back(b);

	// MAXScript regeneration capture (see SMsBone).
	{
		SMsBone mb;
		mb.Name = name;
		mb.IsBiped = bipedSys && isBipedBoneNode(node);
		mb.IsCom = bipedSys && isBipedComNode(node);
		mb.Id = 0; mb.Link = 0;
		if (mb.IsBiped) readBipDrivenIdLink(node, mb.Id, mb.Link);
		mb.FatherIdx = fatherId;
		mb.WorldPos = worldTM.getPos();
		mb.WorldRot = worldTM.getRot();
		mb.WorldRot.normalize();
		mb.Rig = bipedSys;
		g_msBones.push_back(mb);
	}

	// Footsteps / ground-level bookkeeping (see patchFootstepsGround). The corpus-era Footsteps
	// node carries a plain PRS controller (not a BipDriven with BID_FOOTPRINTS like later plugin
	// versions), so it is identified as a COM child by name.
	if (bipedSys && isBipedBoneNode(node))
	{
		SBipedRig &rig = g_bipedRigs[bipedSys];
		uint32 fid = 0, flink = 0;
		if (readBipDrivenIdLink(node, fid, flink))
		{
			if (fid == BID_PELVIS)
			{
				// Pelvis world matrix for the thigh world-space anchor (pelvis precedes the
				// thighs in walk order on every corpus and regen rig).
				rig.PelvisWorldTM = worldTM;
				rig.HavePelvisWorldTM = true;
			}
			if (fid == BID_SPINE)
			{
				// Last assignment wins: by clavicle time this holds the last spine link.
				rig.LastSpineWorldTM = worldTM;
				rig.HaveLastSpineWorldTM = true;
			}
			if (fid == BID_FOOTPRINTS)
			{
				rig.FootstepsBoneIdx = myId;
				rig.FootstepsParentWorld = parentWorld;
			}
			else if (fid == BID_LTOES && flink == 0 && !rig.HaveToeBaseWorldZ)
			{
				rig.ToeBaseWorldZ = worldTM.getPos().z;
				rig.HaveToeBaseWorldZ = true;
			}
			else if (fid == BID_LLEG && (int)flink == rig.MaxLegLink && !rig.HaveAnkleWorldZ)
			{
				rig.AnkleWorldZ = worldTM.getPos().z;
				rig.HaveAnkleWorldZ = true;
			}
		}
	}
	else
	{
		INode *fparent = node->parent();
		if (fparent && isBipedComNode(fparent) && name.find("Footsteps") != std::string::npos)
		{
			CSceneClass *psys = bipedSystemOfCtrl(fparent->getReference(0));
			if (psys)
			{
				SBipedRig &rig = g_bipedRigs[psys];
				rig.FootstepsBoneIdx = myId;
				rig.FootstepsParentWorld = parentWorld;
			}
		}
	}

	std::vector<INode *> kids = orderedChildrenOf(node, ssc);
	for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci) walkNode(*ci, myId, worldTM, ssc, bones, nameSet);
}

// The Footsteps marker sits at ground level: its world height equals the (left) toe attach height
// (exact on the Max 9 differential dataset across ankle/height/foot-scale variants; the value is
// derived, not stored — which is why exhaustive scans never found e.g. ca_spaceship's 7.73).
// The Footsteps bone precedes the toes in walk order, so patch it after the walk: keep its world
// X/Y, set world Z to the toe attach height, recompute local pos + InvBindPos.
void patchFootstepsGround(std::vector<Bone> &bones)
{
	for (std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.begin(); it != g_bipedRigs.end(); ++it)
	{
		SBipedRig &rig = it->second;
		if (rig.FootstepsBoneIdx < 0) continue;
		// Ground height rule by era: legacy = the (left) toe attach height; fresh-format = the
		// (left) ankle height minus the leg record's foot-height slot ([7]) — regen corpus GT
		// (fy_hom -0.011485 exact; ca_spaceship 7.8084 - 0.4040; the toe rule was ~0.5m off on
		// the deep-ankle monster families).
		float groundZ = 0.0f;
		bool haveGround = false;
		if (rig.BodyType == 0 && rig.HaveAnkleWorldZ)
		{
			g_rig = &rig;
			const float *lf = bipedSideHalf(0x000f, false, 8);
			if (lf)
			{
				groundZ = rig.AnkleWorldZ - lf[7];
				haveGround = true;
			}
		}
		if (!haveGround && rig.HaveToeBaseWorldZ)
		{
			groundZ = rig.ToeBaseWorldZ;
			haveGround = true;
		}
		if (!haveGround) continue;
		Bone &b = bones[(size_t)rig.FootstepsBoneIdx];
		NLMISC::CMatrix localTM = makeLocalTM(b.OrigPos, b.OrigRot, b.DefaultScale);
		NLMISC::CMatrix worldTM = rig.FootstepsParentWorld * localTM;
		NLMISC::CVector wp = worldTM.getPos();
		wp.z = groundZ;
		NLMISC::CMatrix pinv = rig.FootstepsParentWorld;
		pinv.invert();
		NLMISC::CVector newLocal = pinv * wp;
		b.OrigPos = newLocal;
		b.DefaultPos = newLocal;
		NLMISC::CMatrix newLocalTM = makeLocalTM(newLocal, b.OrigRot, b.DefaultScale);
		NLMISC::CMatrix newWorld = rig.FootstepsParentWorld * newLocalTM;
		NLMISC::CMatrix invBind;
		invBind.identity();
		invBind.setRot(newWorld.getI(), newWorld.getJ(), newWorld.getK());
		invBind.setPos(newWorld.getPos());
		invBind.invert();
		b.InvBindPos = invBind;
		// Keep the world-transform views coherent with the patch: WorldTM feeds the anim
		// exporter's attach-offset derivation and g_msBones feeds the maxscript/manifest dumps
		// (both are index-parallel with bones).
		b.WorldTM = newWorld;
		if ((size_t)rig.FootstepsBoneIdx < g_msBones.size() && g_msBones[(size_t)rig.FootstepsBoneIdx].Name == b.Name)
			g_msBones[(size_t)rig.FootstepsBoneIdx].WorldPos = newWorld.getPos();
	}
}

// Double-precision variant: parent-world is Mat4D, local build + multiply + invert happen in
// double, cast to float only when constructing the final float32 CMatrix + serialize.
void walkNodeD(INode *node, sint32 fatherId, const Mat4D &parentWorld,
                      CSceneClassContainer *ssc,
                      std::vector<Bone> &bones, std::set<std::string> &nameSet)
{
	Bone b;
	std::string name = ucstring(node->userName()).toUtf8();
	if (!nameSet.insert(name).second) name += "_Second";
	b.Name = name;
	b.FatherId = fatherId;
	// Default for standard PRS controllers is FALSE (bone inherits parent's scale).
	// buildSkeleton flips this to true only when the controller's inheritance flags have
	// any of INHERIT_SCL_X|Y|Z set, or when the parent is a biped node. For the non-biped
	// files we handle, the default holds — no controller-flag reader implemented yet.
	b.UnheritScale = false;
	b.LodDisableDistance = std::max(0.0f, getNodeScriptAppDataFloat(node, NEL3D_APPDATA_BONE_LOD_DISTANCE, 0.0f));

	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	CReferenceMaker *tmCtrl = node->getReference(0);
	CSceneClass *bipedSys = bipedSystemOfCtrl(tmCtrl);
	if (bipedSys && isBipedBoneNode(node))
	{
		// Figure-mode biped reconstruction (see getBipedLocal). Extract the parent world rotation
		// from the double-precision parent matrix. (Biped files never take the walkNodeD path —
		// main() routes them through walkNode — but keep the branch coherent regardless.)
		g_rig = &rigFor(bipedSys, ssc);
		NLMISC::CMatrix pm; pm.identity();
		pm.setRot(parentWorld.getI(), parentWorld.getJ(), parentWorld.getK());
		pm.setPos(parentWorld.getPos());
		NLMISC::CQuat worldRotOut;
		getBipedLocal(node, pm, realPos, realRot, realScale, worldRotOut);
		b.UnheritScale = true; // biped bones always unherit their parent's scale (see plugin_max/nel_mesh_lib)
	}
	else
	{
		getLocalTransform(tmCtrl, realPos, realRot, realScale);
	}
	realRot.normalize();

	Mat4D localTM = Mat4D::fromTRS(realPos, realRot, realScale);
	Mat4D worldTM = parentWorld * localTM;

	b.OrigPos = realPos;
	b.OrigRot = realRot;

	if (fatherId < 0)
	{
		b.DefaultPos = NLMISC::CVector::Null;
		b.DefaultRotQuat = NLMISC::CQuat::Identity;
	}
	else
	{
		b.DefaultPos = realPos;
		b.DefaultRotQuat = realRot;
	}
	b.DefaultScale = realScale;

	// Invert in double, then extract to float. State bits come from CMatrix's identity+setRot+setPos.
	Mat4D invWorld = worldTM.inverse();
	NLMISC::CMatrix invBind;
	invBind.identity();
	invBind.setRot(invWorld.getI(), invWorld.getJ(), invWorld.getK());
	invBind.setPos(invWorld.getPos());
	b.InvBindPos = invBind;

	b.Node = node;
	{
		NLMISC::CMatrix w; w.identity();
		w.setRot(worldTM.getI(), worldTM.getJ(), worldTM.getK());
		w.setPos(worldTM.getPos());
		b.WorldTM = w;
	}

	sint32 myId = (sint32)bones.size();
	bones.push_back(b);

	std::vector<INode *> kids = orderedChildrenOf(node, ssc);
	for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci) walkNodeD(*ci, myId, worldTM, ssc, bones, nameSet);
}

// Non-biped skeleton walk (the all-PRS fauna files). Identical to walkNode's non-biped arithmetic —
// direct PRS-controller values for DefaultRot/Scale, NeL CMatrix world accumulation for InvBindPos —
// with ONE reference-faithful refinement: DefaultPos is taken from the Max quotient matrix's
// translation, decompMatrix(nodeTM·Inverse(parentTM)) via max_scene (which is what
// CExportNel::buildSkeleton / getNELBoneLocalTM actually computes for a non-biped bone), not the raw
// controller position. That single change measurably improves bit-exactness against the reference
// (fauna corpus, of 222 bones: DefaultPos bit-exact 47→84 on the x64/SSE build, 47→53 on the VS2008
// x87 reference build) with NO regression on any other field on either codegen. DefaultRot/Scale and
// InvBindPos stay on the direct/NeL path deliberately: routing them through our decomp_affine port
// (which is not bit-identical to Max's SDK object code) only adds ULP noise on skeletons' many
// near-identity local rotations — see pipeline_max_design.md's skel float-precision note.
static void walkNodeMaxRec(INode *node, sint32 fatherId, MAXSCENE::SNodeTMCache &cache,
                           const NLMISC::CMatrix &nelParentWorld,
                           CSceneClassContainer *ssc, std::vector<Bone> &bones, std::set<std::string> &nameSet)
{
	Bone b;
	std::string name = ucstring(node->userName()).toUtf8();
	if (!nameSet.insert(name).second) name += "_Second";
	b.Name = name;
	b.FatherId = fatherId;
	b.UnheritScale = false; // non-biped standard PRS bones inherit their father's scale
	b.LodDisableDistance = std::max(0.0f, getNodeScriptAppDataFloat(node, NEL3D_APPDATA_BONE_LOD_DISTANCE, 0.0f));

	// Direct PRS-controller values + NeL CMatrix world (the shipped walkNode arithmetic).
	NLMISC::CVector realPos, realScale; NLMISC::CQuat realRot;
	getLocalTransform(node->getReference(0), realPos, realRot, realScale);
	realRot.normalize();
	NLMISC::CMatrix localNel = makeLocalTM(realPos, realRot, realScale);
	NLMISC::CMatrix worldNel = nelParentWorld * localNel;

	// DefaultPos from the reference's Max quotient matrix (getLocalMatrix = nodeTM·Inverse(parentTM)).
	MAXMATH::Matrix3M localMax = MAXSCENE::getLocalMatrix(*node, cache);
	NLMISC::CVector qScale, qPos; NLMISC::CQuat qRot;
	MAXSCENE::decompMatrix(qScale, qRot, qPos, localMax);

	b.OrigPos = qPos;   // glTF carries the real local; DefaultPos is what the .skel stores
	b.OrigRot = realRot;
	if (fatherId < 0) { b.DefaultPos = NLMISC::CVector::Null; b.DefaultRotQuat = NLMISC::CQuat::Identity; }
	else { b.DefaultPos = qPos; b.DefaultRotQuat = realRot; }
	b.DefaultScale = realScale;

	NLMISC::CMatrix ibp; ibp.identity();
	ibp.setRot(worldNel.getI(), worldNel.getJ(), worldNel.getK()); ibp.setPos(worldNel.getPos());
	ibp.invert();
	b.InvBindPos = ibp;
	b.Node = node;
	b.WorldTM = worldNel;

	sint32 myId = (sint32)bones.size();
	bones.push_back(b);

	std::vector<INode *> kids = orderedChildrenOf(node, ssc);
	for (std::vector<INode *>::iterator ci = kids.begin(); ci != kids.end(); ++ci)
		walkNodeMaxRec(*ci, myId, cache, worldNel, ssc, bones, nameSet);
}

void walkNodeMax(INode *node, sint32 fatherId, CSceneClassContainer *ssc,
                 std::vector<Bone> &bones, std::set<std::string> &nameSet)
{
	MAXSCENE::SNodeTMCache cache;
	NLMISC::CMatrix root; root.identity();
	walkNodeMaxRec(node, fatherId, cache, root, ssc, bones, nameSet);
}

// Biped ClassIds are confirmed from the character-studio MAXScript reference:
//   bipedSystem                    {9155,0}  — the biped system (root, superclass 0x60/Object)
//   Vertical_Horizontal_Turn       {9156,0}  — COM/body controller (superclass 0x9008/ControlTransform)
//   BipDriven_Control              {9154,0}  — per-body-part controller (was BipSlave_Control pre-2022)
//   Biped_SubAnim                  {0x6b147369, 0x078c6b2a}  — sub-anim (superclass 0x9003/ControlFloat)
//   biped_object                   {9125,0}  — per-body-part geometry (superclass 0x10/GeomObject)
const NLMISC::CClassId CLASSID_BIPED_SYSTEM (0x00009155, 0x00000000);
const NLMISC::CClassId CLASSID_BIPED_VHT    (0x00009156, 0x00000000);
const NLMISC::CClassId CLASSID_BIPED_DRIVEN (0x00009154, 0x00000000);
const NLMISC::CClassId CLASSID_BIPED_SUBANIM(0x6b147369, 0x078c6b2a);
const NLMISC::CClassId CLASSID_BIPED_OBJECT (0x00009125, 0x00000000);

// Detect a biped file by scanning the ClassDirectory3 for any of the four Biped ClassIds. The
// DllDirectory-based check would false-positive because biped.dlc is loaded even when no biped
// exists in the scene (Max loads all installed plugins); only the ClassDirectory3 entry appears
// when a biped class is actually instantiated. Using ClassId not display name so this survives
// the plugin's rename of BipSlave_Control → BipDriven_Control across the corpus's Max versions.
bool looksLikeBipedFile(CClassDirectory3 &cd)
{
	for (PIPELINE::MAX::CStorageContainer::TStorageObjectConstIt it = cd.chunks().begin(); it != cd.chunks().end(); ++it)
	{
		const CClassEntry *entry = dynamic_cast<const CClassEntry *>(it->second);
		if (!entry) continue;
		NLMISC::CClassId cid = entry->classId();
		if (cid == CLASSID_BIPED_SYSTEM || cid == CLASSID_BIPED_VHT ||
		    cid == CLASSID_BIPED_DRIVEN || cid == CLASSID_BIPED_OBJECT)
			return true;
	}
	return false;
}


} /* namespace PMAX_RIG */

/* end of file */
