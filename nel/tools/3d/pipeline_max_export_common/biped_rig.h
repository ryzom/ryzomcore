/**
 * \file biped_rig.h
 * \brief Shared biped figure-mode rig decode + skeleton walk for the headless .max exporters
 * (pipeline_max_export_skel, pipeline_max_export_anim). Extracted verbatim from
 * pipeline_max_export_skel/main.cpp (2026-07-06); see pipeline_max_design.md for the decode
 * provenance. Behavior gated by the skel corpus test (byte-identical outputs).
 * \author Jan Boon (Kaetemi)
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

#ifndef PIPELINE_MAX_RIG_BIPED_RIG_H
#define PIPELINE_MAX_RIG_BIPED_RIG_H

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/misc/matrix.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../pipeline_max/storage_object.h"
#include "../pipeline_max/scene.h"
#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/class_directory_3.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"

namespace PMAX_RIG {

struct Bone
{
	std::string Name;
	sint32 FatherId; // -1 for root
	bool UnheritScale;
	// DefaultPos/RotQuat are what goes into the .skel file's default tracks — reset to Null/
	// Identity for the root bone (NeL buildSkeleton convention). DefaultScale is not reset.
	NLMISC::CVector DefaultPos;
	NLMISC::CQuat DefaultRotQuat;
	NLMISC::CVector DefaultScale;
	// OrigPos/RotQuat are the REAL local transform from the PRS controller — used for
	// InvBindPos world-matrix accumulation and for glTF output.
	NLMISC::CVector OrigPos;
	NLMISC::CQuat OrigRot;
	NLMISC::CMatrix InvBindPos; // inverse of world TM
	float LodDisableDistance;
	// The source scene node (set by walkNode/walkNodeD) — lets callers map bones back to
	// scene objects (the anim exporter needs the per-node controller and biped id/link).
	PIPELINE::MAX::BUILTIN::INode *Node;
	// World transform at figure time (float path; set by walkNode) — the anim evaluator derives
	// its attach offsets and link lengths from these.
	NLMISC::CMatrix WorldTM;
};

// Row-major 4x4 double matrix for accumulate-in-double mode.
struct Mat4D
{
	double m[16]; // row-major: m[row*4 + col]
	static Mat4D identity()
	{
		Mat4D r = { { 0 } };
		r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0;
		return r;
	}
	static Mat4D fromTRS(const NLMISC::CVector &t, const NLMISC::CQuat &q, const NLMISC::CVector &s)
	{
		// Rotation matrix from quaternion (row-major, standard formula), then scale columns, then translate.
		double x = q.x, y = q.y, z = q.z, w = q.w;
		double xx = x*x, yy = y*y, zz = z*z;
		double xy = x*y, xz = x*z, yz = y*z;
		double wx = w*x, wy = w*y, wz = w*z;
		Mat4D r = identity();
		r.m[0] = (1 - 2*(yy+zz)) * s.x;
		r.m[1] = (2*(xy - wz))   * s.y;
		r.m[2] = (2*(xz + wy))   * s.z;
		r.m[4] = (2*(xy + wz))   * s.x;
		r.m[5] = (1 - 2*(xx+zz)) * s.y;
		r.m[6] = (2*(yz - wx))   * s.z;
		r.m[8] = (2*(xz - wy))   * s.x;
		r.m[9] = (2*(yz + wx))   * s.y;
		r.m[10] = (1 - 2*(xx+yy)) * s.z;
		r.m[3] = t.x; r.m[7] = t.y; r.m[11] = t.z;
		return r;
	}
	Mat4D operator*(const Mat4D &o) const
	{
		Mat4D r = { { 0 } };
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				r.m[i*4+j] = m[i*4+0]*o.m[0*4+j] + m[i*4+1]*o.m[1*4+j] + m[i*4+2]*o.m[2*4+j] + m[i*4+3]*o.m[3*4+j];
		return r;
	}
	// Full 4x4 inverse via adjugate / determinant. Cofactor expansion.
	Mat4D inverse() const
	{
		double inv[16];
		inv[0]  =  m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
		inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
		inv[8]  =  m[4]*m[9]*m[15]  - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
		inv[12] = -m[4]*m[9]*m[14]  + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
		inv[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
		inv[5]  =  m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
		inv[9]  = -m[0]*m[9]*m[15]  + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
		inv[13] =  m[0]*m[9]*m[14]  - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
		inv[2]  =  m[1]*m[6]*m[15]  - m[1]*m[7]*m[14]  - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7]  - m[13]*m[3]*m[6];
		inv[6]  = -m[0]*m[6]*m[15]  + m[0]*m[7]*m[14]  + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7]  + m[12]*m[3]*m[6];
		inv[10] =  m[0]*m[5]*m[15]  - m[0]*m[7]*m[13]  - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7]  - m[12]*m[3]*m[5];
		inv[14] = -m[0]*m[5]*m[14]  + m[0]*m[6]*m[13]  + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6]  + m[12]*m[2]*m[5];
		inv[3]  = -m[1]*m[6]*m[11]  + m[1]*m[7]*m[10]  + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7]   + m[9]*m[3]*m[6];
		inv[7]  =  m[0]*m[6]*m[11]  - m[0]*m[7]*m[10]  - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7]   - m[8]*m[3]*m[6];
		inv[11] = -m[0]*m[5]*m[11]  + m[0]*m[7]*m[9]   + m[4]*m[1]*m[11] - m[4]*m[3]*m[9]  - m[8]*m[1]*m[7]   + m[8]*m[3]*m[5];
		inv[15] =  m[0]*m[5]*m[10]  - m[0]*m[6]*m[9]   - m[4]*m[1]*m[10] + m[4]*m[2]*m[9]  + m[8]*m[1]*m[6]   - m[8]*m[2]*m[5];
		double det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
		double invDet = 1.0 / det;
		Mat4D r = { { 0 } };
		for (int i = 0; i < 16; ++i) r.m[i] = inv[i] * invDet;
		return r;
	}
	// Extract IJKP as CVector (float) — columns 0..2 for basis vectors, column 3 for position.
	// Row-major means m[row*4+col]: I is column 0 = m[0], m[4], m[8].
	NLMISC::CVector getI() const { return NLMISC::CVector((float)m[0], (float)m[4], (float)m[8]); }
	NLMISC::CVector getJ() const { return NLMISC::CVector((float)m[1], (float)m[5], (float)m[9]); }
	NLMISC::CVector getK() const { return NLMISC::CVector((float)m[2], (float)m[6], (float)m[10]); }
	NLMISC::CVector getPos() const { return NLMISC::CVector((float)m[3], (float)m[7], (float)m[11]); }
};

// Global mode flag — flipped by --double CLI arg (export_skel).
extern bool g_useDouble;

// Biped plugin internal bone-id constants (0-based; see the .cpp for provenance).
enum EBipedBoneId
{
	BID_LARM = 0, BID_RARM = 1, BID_LFINGERS = 2, BID_RFINGERS = 3,
	BID_LLEG = 4, BID_RLEG = 5, BID_LTOES = 6, BID_RTOES = 7,
	BID_SPINE = 8, BID_TAIL = 9, BID_HEAD = 10, BID_PELVIS = 11,
	BID_VERTICAL = 12, BID_HORIZONTAL = 13, BID_TURN = 14, BID_FOOTPRINTS = 15,
	BID_NECK = 16, BID_PONY1 = 17, BID_PONY2 = 18,
	BID_PROP1 = 19, BID_PROP2 = 20, BID_PROP3 = 21,
	BID_RFINGERNUB = 22, BID_LFINGERNUB = 23, BID_LTOENUB = 24, BID_RTOENUB = 25,
	BID_TAILNUB = 26, BID_HEADNUB = 27, BID_PONY1NUB = 28, BID_NECKNUB = 29,
};

struct SBipedToe
{
	int NLinks;
	NLMISC::CVector Pos;
	NLMISC::CQuat Rot;
	std::vector<float> Lens;
};

struct SBipedFinger
{
	int NLinks;
	NLMISC::CVector Pos;
	NLMISC::CQuat Rot;
	std::vector<float> Lens;
};

struct SBipedChain
{
	bool HasMat;
	NLMISC::CQuat MatRot;
	NLMISC::CVector MatPos;
	std::vector<float> Lens;
	std::vector<NLMISC::CVector> Angles;
	SBipedChain() : HasMat(false), MatRot(NLMISC::CQuat::Identity), MatPos(NLMISC::CVector::Null) { }
};

struct SBipedRig
{
	PIPELINE::MAX::CSceneClass *Sys;
	bool HasCom;
	NLMISC::CVector ComPos;
	NLMISC::CQuat ComRot;
	NLMISC::CVector ComDisp;
	// The 0x0104 base-frame world matrix's own translation, read independently of the 0x006c COM
	// record above (see parseComRecord). Normally identical to ComPos: 0x006c's [4..6] is this
	// rig's figure-mode COM position, which coincides with the biped's current world position for
	// the overwhelming majority of corpus files — because EXITING Figure Mode unconditionally
	// recommits the figure value as current, healing any prior divergence even with no deliberate
	// edit (confirmed live in Max 9, pipeline_max_design.md §10n "Tenth"). An ordinary Move outside
	// Figure Mode is the only thing that diverges the two; per-file, whichever of {0x006c, 0x0104}
	// is correct depends entirely on whether the file was, at some later point for any reason,
	// re-entered into and exited from Figure Mode after that move — information the format does not
	// record. `--diff-rig` (pipeline_max_export_anim) confirmed 0x0104 is one of a FOUR-chunk
	// "current position" family that always changes together on an ordinary move: 0x0065[17] (inside
	// the neck-angle record, past what parseNeckAngles actually reads — see its own comment),
	// 0x0104[12..14] (this field), 0x0259[17] and 0x0260[5] (the 0x0258-0x0261 shadow bank, Part J).
	// Only 0x0104 is read here; the other three are documented but intentionally unclaimed (proven
	// redundant with this one in every case checked, not worth a second read path for the same
	// value). Used as the vertical fallback candidate — evidence tally in §10n found figure height
	// (0x006c) the correct GENERAL default (10+ independent exact matches vs. ~1 for 0x0104), so
	// this field is populated but deliberately NOT substituted for ComPos; see biped_anim.cpp.
	NLMISC::CVector BaseFramePos;
	bool HaveBaseFramePos;
	// A height-correction scalar at 0x0260[1] (0x006c's own [1] — same 12-float record layout — is
	// always 0, since figure mode needs no correction). Solves ship_tank_karavan_mort_idle's
	// previously-unexplained 5cm residual exactly: BaseFramePos.z + HeightCorrection reproduces the
	// shipped reference to 4e-8 (float32 noise). Confirmed NOT a live Character Studio computation —
	// an exhaustive interactive test (moving the biped, deleting the rig's only real key, adding and
	// removing a real vertical key) showed the Animation-Mode value is completely invariant except
	// when the vertical channel is genuinely (re)keyed, and 0x0260[1] closes the gap by simple
	// addition, no rotation or other transform needed (pipeline_max_design.md §10n "Thirteenth").
	// Verified 0 on every corpus file checked where BaseFramePos alone was already exact (decoupe)
	// and on files where the OTHER branch (figure height) is the correct source entirely (recruteur)
	// — consistent both times. Not yet corpus-swept for files where the correction is itself nonzero
	// besides this one; kept as documented, typed, unconsumed plumbing (like BaseFramePos) rather
	// than substituted into ComPos, since the "which branch" ambiguity between figure height and
	// base(+correction) is unchanged by this finding.
	float HeightCorrection;
	bool HaveHeightCorrection;
	// Move All Mode reference-frame transform (chunk 0x0117, a Y-up row-major affine 4x4 / 16
	// floats). Decoded + validated 2026-07-08 via three interactive Move All probes on
	// ship_tank_karavan_mort_idle (+10z -> [13]=10; x5/y10/z20 -> [12]=5,[13]=20,[14]=-10;
	// rotZ45 -> the 3x3 rotation block + induced translation): translation row [12,13,14] =
	// (x, z_up, -y), so the NeL-space Move All translation is (m12, -m14, m13). Identity on every
	// shipped corpus file (Move All was never used in the reference assets), so applying it is a
	// no-op there; it is additive on top of the current-position COM frame and must be applied for
	// any file that DOES carry a non-identity Move All. Only the translation is captured here (the
	// rotation part is identity corpus-wide; full-affine application is a documented TODO with no
	// corpus case to validate against). See pipeline_max_design.md §10n.
	NLMISC::CVector MoveAllTrans;
	bool HaveMoveAll;
	// True when the figure-mode COM record (0x006c[0..2]) carries a nonzero displacement — the
	// in-file signal that the biped's COM holds a committed non-figure pose, so the unkeyed COM
	// evaluates from the current-position frame (BaseFramePos + HeightCorrection + MoveAllTrans)
	// rather than figure height. Exactly zero on files whose COM sits at figure (recruteur/meca).
	bool ComDispNonZero;
	// Character Studio's <biped_ctrl>.dynamicsType (0 = "Biped Dynamics", the default — Character
	// Studio can live-compute airborne trajectories/balance even off-key; 1 = "Spline Dynamics" —
	// plain interpolation, no live physics), stored at chunk 0x0012 (confirmed via an isolated,
	// single-chunk A/B toggle on a real skeleton file — pipeline_max_design.md §10n "Ninth"/"Tenth").
	// Uniformly 0 on every real corpus file checked so far and confirmed NOT to affect the exported
	// COM position even when forced to 1 (§10n) — kept for completeness/future corpus-wide auditing,
	// not consumed by any decision in this library.
	int DynamicsType;
	bool HaveDynamicsType;
	// Linked-COM attach pair (2026-07-10, gen_biped_linkcom_probe round — design doc §10m-ter).
	// A biped COM LINKED under another node rides its parent rigidly; the constant exported local
	// TM is NOT stored literally but derived from a stored pair:
	//   L = LinkParentInvTM * BaseFrameTM     (NeL column convention)
	// LinkParentInvTM = chunk 0x0112 (12 floats: 3x3 rows + translation, PLAIN world coordinates
	// row-vector — unlike the Y-up biped records) = the INVERSE of the parent's world TM captured
	// when the link relationship was last established/edited (probe: linking, ordinary moves and
	// figure-mode edits of a linked COM all refresh the pair; identity on unlinked/root rigs).
	// BaseFrameTM = the full current-position frame: 0x0104's rotation (Y-up) with translation
	// 0x0104.t + the 0x0260[0..2] correction vector — the FULL-VECTOR generalization of
	// HeightCorrection below (whose 0x0260[1] is this vector's Y-up vertical component).
	// Float-exact on every probe file incl. the previously-unexplained kitin stun trilogy and the
	// mektoub rider (exported quat = conjugate of L.getRot()).
	NLMISC::CMatrix LinkParentInvTM;
	bool HaveLinkParentInv;
	NLMISC::CMatrix BaseFrameTM;
	bool HaveBaseFrameTM;
	bool HasThighZ;
	float ThighZ[2];
	std::vector<SBipedToe> Toes[2];
	int MaxLegLink;
	bool HasClavicleZ;
	float ClavicleZ[2];
	NLMISC::CVector ClavicleOff[2];
	float ClavicleA[2];
	float ClavicleB[2];
	std::vector<SBipedFinger> Fingers[2];
	SBipedChain Spine, Tail, Pony1, Pony2;
	std::vector<NLMISC::CVector> NeckAngles;
	NLMISC::CQuat PelvisWorldRot;
	bool HavePelvisWorldRot;
	NLMISC::CQuat LastSpineWorldRot;
	bool HaveLastSpineWorldRot;
	// Pelvis record (0x000d) attach translation in MatPos convention. Zero on the legacy corpus;
	// fresh-format (BodyType 0) rigs store the last-spine-link-end -> neck attach offset here
	// (Max 9 keeps the neck off the stored spine-link length by this amount; regen-corpus GT).
	NLMISC::CVector PelvisRecTrans;
	// Pelvis bone world matrix, captured by walkNode when the pelvis is walked (before any thigh).
	// Thighs anchor to the pelvis frame in world space: their node parent differs by era (legacy
	// corpus: pelvis; Max 9 triangle-pelvis rigs: lowest spine link), the attach rule doesn't.
	NLMISC::CMatrix PelvisWorldTM;
	bool HavePelvisWorldTM;
	// Last spine link's world matrix (last assignment during the walk wins) — fresh-format
	// clavicles anchor at the spine end in this frame.
	NLMISC::CMatrix LastSpineWorldTM;
	bool HaveLastSpineWorldTM;
	// Chunk 0x0115 = the biped bodyType enum (0=Skeleton, 1=Male, 2=Female, 3=Classic — proven by
	// a single-variable probe edit; previously misread as a "figure version" marker). Still used
	// as the legacy-vs-fresh decode gate, which works because the whole legacy corpus was authored
	// as Classic (3) while biped.createNew leaves the documented Skeleton default (0) — the gate's
	// real semantic is authoring era via body-type default, not a format version.
	int BodyType;
	float ToeBaseWorldZ;
	bool HaveToeBaseWorldZ;
	// Left ankle (last leg link) world height — fresh-format footsteps sit at ankle minus the
	// leg record's foot-height slot ([7]) instead of at the toe attach height.
	float AnkleWorldZ;
	bool HaveAnkleWorldZ;
	sint32 FootstepsBoneIdx;
	NLMISC::CMatrix FootstepsParentWorld;
	SBipedRig();
};

// Rigs per Biped system object; cleared per file (callers reset before each file).
extern std::map<PIPELINE::MAX::CSceneClass *, SBipedRig> g_bipedRigs;
// The rig currently being decoded by getBipedLocal / the chunk readers.
extern SBipedRig *g_rig;

// MAXScript regeneration capture (export_skel --maxscript).
struct SMsBone
{
	std::string Name;
	bool IsBiped;
	bool IsCom;
	uint32 Id, Link;
	sint32 FatherIdx;
	NLMISC::CVector WorldPos;
	NLMISC::CQuat WorldRot;
	PIPELINE::MAX::CSceneClass *Rig;
};
extern std::vector<SMsBone> g_msBones;

// The biped runtime's tiny constant twist (rad) baked into several procedural default frames.
extern const float BIPED_EPS_TWIST;

// Chunk access + node helpers
PIPELINE::MAX::IStorageObject *findChunkAnywhere(PIPELINE::MAX::CSceneClass *sc, uint16 id);
bool readCtrlDefault(PIPELINE::MAX::CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes);
bool isBipedBoneNode(PIPELINE::MAX::BUILTIN::INode *node);
bool isBipedComNode(PIPELINE::MAX::BUILTIN::INode *node);
PIPELINE::MAX::CSceneClass *bipedSystemOfCtrl(PIPELINE::MAX::BUILTIN::CReferenceMaker *tmCtrl);
bool readBipDrivenIdLink(PIPELINE::MAX::BUILTIN::INode *node, uint32 &boneId, uint32 &linkIdx);
NLMISC::CVector readNodeBoneDimensions(PIPELINE::MAX::BUILTIN::INode *node);
float getNodeScriptAppDataFloat(PIPELINE::MAX::BUILTIN::INode *node, uint32 subId, float def);
void getLocalTransform(PIPELINE::MAX::BUILTIN::CReferenceMaker *tmCtrl,
                       NLMISC::CVector &pos, NLMISC::CQuat &rot, NLMISC::CVector &scale);
NLMISC::CMatrix makeLocalTM(const NLMISC::CVector &pos, const NLMISC::CQuat &rot, const NLMISC::CVector &scale);
std::vector<PIPELINE::MAX::BUILTIN::INode *> orderedChildrenOf(PIPELINE::MAX::BUILTIN::INode *parent, PIPELINE::MAX::CSceneClassContainer *ssc);

// Rig chunk readers (through g_rig)
const float *bipedChunkFloats(uint16 chunkId, size_t minFloats, size_t *countOut = NULL);
const float *bipedSideHalf(uint16 chunkId, bool leftSide, size_t minFloats, size_t *halfOut = NULL);
NLMISC::CQuat chainAngleQuat(const NLMISC::CVector &a);
NLMISC::CQuat mirrorQuatLR(const NLMISC::CQuat &q);
uint32 floatBitsAsUint(float f);
bool readSideQuat(uint16 chunkId, bool leftSide, int off, NLMISC::CQuat &out);
int legHeadShift();
bool locateChainSub(const std::vector<SBipedToe> &toes, uint32 link, int &toeIdx, int &sub);
bool locateChainSub(const std::vector<SBipedFinger> &fingers, uint32 link, int &fi, int &sub);
bool chainPoseBaseDelta(uint16 chunkId, bool leftSide, int chainIdx, NLMISC::CQuat &delta);
bool chainPoseLinkAngle(uint16 chunkId, bool leftSide, int chainIdx, int sub, float &angle);

// Rig state
SBipedRig &rigFor(PIPELINE::MAX::CSceneClass *sys, PIPELINE::MAX::CSceneClassContainer *ssc);
void getBipedLocal(PIPELINE::MAX::BUILTIN::INode *node, const NLMISC::CMatrix &parentWorld,
                   NLMISC::CVector &pos, NLMISC::CQuat &rot, NLMISC::CVector &scale,
                   NLMISC::CQuat &worldRotOut);

// Skeleton walk
void walkNode(PIPELINE::MAX::BUILTIN::INode *node, sint32 fatherId, const NLMISC::CMatrix &parentWorld,
              PIPELINE::MAX::CSceneClassContainer *ssc,
              std::vector<Bone> &bones, std::set<std::string> &nameSet);
void walkNodeD(PIPELINE::MAX::BUILTIN::INode *node, sint32 fatherId, const Mat4D &parentWorld,
               PIPELINE::MAX::CSceneClassContainer *ssc,
               std::vector<Bone> &bones, std::set<std::string> &nameSet);
void patchFootstepsGround(std::vector<Bone> &bones);
// Non-biped skeleton walk: walkNode's arithmetic with DefaultPos taken from the reference's Max
// quotient matrix (max_scene decompMatrix) — a float-precision refinement over the raw controller
// position, no regression on any other field (see biped_rig.cpp).
void walkNodeMax(PIPELINE::MAX::BUILTIN::INode *node, sint32 fatherId,
                 PIPELINE::MAX::CSceneClassContainer *ssc,
                 std::vector<Bone> &bones, std::set<std::string> &nameSet);

// Biped file detection (ClassDirectory3 scan)
bool looksLikeBipedFile(PIPELINE::MAX::CClassDirectory3 &cd);

} /* namespace PMAX_RIG */

#endif /* PIPELINE_MAX_RIG_BIPED_RIG_H */

/* end of file */
