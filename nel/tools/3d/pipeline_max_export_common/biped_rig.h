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
		Mat4D r{};
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
		Mat4D r{};
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
		Mat4D r{};
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
	// fresh-format (FigureVersion 0) rigs store the last-spine-link-end -> neck attach offset here
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
	int FigureVersion;
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
bool readRawBytes(PIPELINE::MAX::CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes);
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

// Biped file detection (ClassDirectory3 scan)
bool looksLikeBipedFile(PIPELINE::MAX::CClassDirectory3 &cd);

} /* namespace PMAX_RIG */

#endif /* PIPELINE_MAX_RIG_BIPED_RIG_H */

/* end of file */
