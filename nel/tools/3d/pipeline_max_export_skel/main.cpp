// Skel export: .max -> .skel, replicating the NelExportSkeleton path of the 3ds Max plugin
// (build_gamedata processes/skel) without 3ds Max.
// Reads Bip01, walks children in scene order, and emits the .skel binary in the same format
// NeL's CShapeStream + CSkeletonShape + CBoneBase produce (including skeleton LODs built from
// the NEL3D_APPDATA_BONE_LOD_DISTANCE AppData, same algorithm as CSkeletonShape::build).
// Non-biped bones take their local transforms from the PRS controller's Bezier Position /
// TCB Rotation / Bezier Scale sub-controllers (default values at chunks 0x2503/0x2504/0x2505);
// biped bones are reconstructed from the figure-mode records on their Biped (0x9155) system
// object (see the reconstruction section below and pipeline_max_design.md).

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/misc/matrix.h>

#include <algorithm>
#include <cmath>
#include <cstring>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

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
#include "../pipeline_max/biped/biped_driven.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace PIPELINE::MAX::BIPED;

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
	// InvBindPos world-matrix accumulation and for glTF output (so the mesh_export roundtrip
	// can reconstruct the identical worldTM without needing the reset to be applied twice).
	NLMISC::CVector OrigPos;
	NLMISC::CQuat OrigRot;
	NLMISC::CMatrix InvBindPos; // inverse of world TM
	float LodDisableDistance;
};

// Row-major 4x4 double matrix for accumulate-in-double mode. Only the operations we need:
// identity, TRS build from CVector/CQuat, multiply, affine inverse, extract IJKP as float.
// Not general (no projection support — none of our bones have it), but that's fine here.
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

// Global mode flag — flipped by --double CLI arg.
static bool g_useDouble = false;

// Find a chunk by id in a container's orphaned or m_Chunks list.
// Since parse claims some chunks (moves to typed fields), and others stay in orphanedChunks,
// we check both.
static IStorageObject *findChunkAnywhere(CSceneClass *sc, uint16 id)
{
	for (auto it = sc->orphanedChunks().begin(); it != sc->orphanedChunks().end(); ++it)
		if (it->first == id) return it->second;
	// Also check m_Chunks (may still have entries pre-clean/build)
	for (auto it = sc->chunks().begin(); it != sc->chunks().end(); ++it)
		if (it->first == id) return it->second;
	return NULL;
}

static bool readRawBytes(CSceneClass *sc, uint16 chunkId, void *dst, size_t nBytes)
{
	IStorageObject *chunk = findChunkAnywhere(sc, chunkId);
	if (!chunk) return false;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw) return false;
	if (raw->Value.size() < nBytes) return false;
	memcpy(dst, raw->Value.data(), nBytes);
	return true;
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
static bool isBipedBoneNode(INode *node)
{
	return dynamic_cast<CBipedDriven *>(node->getReference(0)) != NULL;
}

// NeL export properties live as AppData entries on the node (see pipeline_max_design.md §8);
// values are stored as strings written by the MaxScript utility panel.
#define NEL3D_APPDATA_BONE_LOD_DISTANCE 1423062615

// Read a float-valued NeL AppData script entry off a node. Matches by SubId only (the ClassId/
// SuperClassId key is always the MaxScript utility's), parses like the reference toFloatMax
// (first ',' becomes '.', dot-decimal parse). Returns def when absent or unparsable.
static float getNodeScriptAppDataFloat(INode *node, uint32 subId, float def)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(node);
	if (!n) return def;
	PIPELINE::MAX::BUILTIN::STORAGE::CAppData *ad = n->appData();
	if (!ad) return def;
	for (auto it = ad->entries().begin(); it != ad->entries().end(); ++it)
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
static NLMISC::CVector readNodeBoneDimensions(INode *node)
{
	NLMISC::CVector v = NLMISC::CVector::Null;
	CSceneClass *n = dynamic_cast<CSceneClass *>(node);
	if (!n) return v;
	IStorageObject *chunk = findChunkAnywhere(n, CHUNK_NODE_096C);
	if (!chunk) return v;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw || raw->Value.size() < 12) return v;
	memcpy(&v, raw->Value.data(), 12);
	return v;
}

// BipDriven Control (0x9154) stores its (biped_bone_id, link_index) pair as chunk 0x0200
// (8 bytes = 2 uint32s), now typed as CBipedDriven (see biped/biped_driven.h). The bone_id maps
// to the biped plugin's getIdLink table (12=pelvis, 9=spine, 11=head, etc.). We use it to
// distinguish "straight chain" bones (child.id == parent.id and child.link == parent.link + 1,
// e.g. Spine → Spine1) from "chain base" bones (child.id != parent.id, e.g. Pelvis → Spine
// crosses biped groups) — the two need different local-position rules.
static bool readBipDrivenIdLink(INode *node, uint32 &boneId, uint32 &linkIdx)
{
	CBipedDriven *tmCtrl = dynamic_cast<CBipedDriven *>(node->getReference(0));
	if (!tmCtrl) return false;
	if (!tmCtrl->hasBipedIdLink()) return false;
	boneId = tmCtrl->bipedBoneId();
	linkIdx = tmCtrl->bipedLinkIndex();
	return true;
}

static void getLocalTransform(CReferenceMaker *tmCtrl,
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

	if (posSc) readRawBytes(posSc, CHUNK_BEZIER_POS_VALUE, &pos, 12);
	if (rotSc && readRawBytes(rotSc, CHUNK_TCB_QUAT_VALUE, &rot, 16))
	{
		// Max stores rotation-controller values in the inverse convention relative to the node
		// TM rotation (the reference exporter never hit this because it read GetNodeTM matrices,
		// not controller values). Without this, every non-180deg/non-identity PRS rotation in the
		// corpus came out as the conjugate of the reference (xyz equal, w flipped) — 180deg and
		// identity rotations are self-conjugate, which is why they matched either way.
		rot.invert();
	}
	if (scaleSc) readRawBytes(scaleSc, CHUNK_BEZIER_SCALE_VALUE, &scale, 12);
}

// Biped plugin internal bone-id constants — 0-based, one less than the MaxScript-facing IDs in
// biped.getIdLink docs. Confirmed by dumping 0x0200 across fy_hom_skel: Bip01 Spine → id=8,
// Bip01 Neck → id=16, Bip01 Head → id=10, Bip01 Pelvis → id=11, L Clavicle group id=0, etc.
enum EBipedBoneId
{
	BID_LARM = 0, BID_RARM = 1, BID_LFINGERS = 2, BID_RFINGERS = 3,
	BID_LLEG = 4, BID_RLEG = 5, BID_LTOES = 6, BID_RTOES = 7,
	BID_SPINE = 8, BID_TAIL = 9, BID_HEAD = 10, BID_PELVIS = 11,
	BID_VERTICAL = 12, BID_HORIZONTAL = 13, BID_TURN = 14, BID_FOOTPRINTS = 15,
	BID_NECK = 16, BID_PONY1 = 17, BID_PONY2 = 18,
	BID_PROP1 = 19, BID_PROP2 = 20, BID_PROP3 = 21,
	// Extension ids beyond the 22 MaxScript-documented groups (0-21), found empirically 2026-07 by
	// scanning chunk 0x0200 across the full biped skel corpus (9 templates: fy/tr/zo/ma male,
	// fy/tr/zo/ca female, ca male). Stable across all of them:
	BID_RFINGERNUB = 22, // end-effector dummy under each "R Finger*2" tip (link = which finger, 0-4)
	BID_LFINGERNUB = 23, // end-effector dummy under each "L Finger*2" tip (link = which finger, 0-4)
	BID_LTOENUB = 24,    // end-effector dummy under "Bip01 L Toe0"
	BID_RTOENUB = 25,    // end-effector dummy under "Bip01 R Toe0"
	BID_TAILNUB = 26,    // end-effector dummy after the last tail link
	BID_HEADNUB = 27,    // head-top dummy (identity local rotation on all 191 corpus instances)
	BID_PONY1NUB = 28,   // end-effector dummy after the last ponytail1 link
	BID_NECKNUB = 29,    // neck accessory dummy (ca_hom "Dummy23")
};

static NLMISC::CMatrix makeLocalTM(const NLMISC::CVector &pos, const NLMISC::CQuat &rot, const NLMISC::CVector &scale)
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

static const NLMISC::CClassId CLASSID_BIPED_SYS(0x00009155, 0x00000000);
static const NLMISC::CClassId CLASSID_BIPED_VHT_CTRL(0x00009156, 0x00000000);

struct SBipedToe
{
	int NLinks;
	NLMISC::CVector Pos;   // base local position (left side, foot frame)
	NLMISC::CQuat Rot;     // base local rotation (left side)
	std::vector<float> Lens;
};

struct SBipedFinger
{
	int NLinks;
	NLMISC::CVector Pos;   // base local position (left side, hand frame)
	NLMISC::CQuat Rot;     // base local rotation (left side)
	std::vector<float> Lens;
};

struct SBipedChain
{
	bool HasMat;
	NLMISC::CQuat MatRot;   // base local rotation from the record matrix (before angle compose)
	NLMISC::CVector MatPos; // base local position
	std::vector<float> Lens;
	std::vector<NLMISC::CVector> Angles; // per-link (a1,a2,a3)
	SBipedChain() : HasMat(false), MatRot(NLMISC::CQuat::Identity), MatPos(NLMISC::CVector::Null) { }
};

struct SBipedRig
{
	CSceneClass *Sys;
	// COM (Bip01-equivalent node world transform)
	bool HasCom;
	NLMISC::CVector ComPos;
	NLMISC::CQuat ComRot;
	// Current V/H/T displacement of the COM node relative to the figure COM, in the COM's local
	// frame (0x006c[0..2] as (d0,-d2,d1)). Bip01World = figureCom + ComRot*Disp; the Pelvis stays
	// at the figure COM, so its local position is -Disp.
	NLMISC::CVector ComDisp;
	// legs + toes. Structure-record scalars are per side (R half first — see bipedSideHalf);
	// the toe/finger base matrices are still parsed from the first (right) half and mirrored,
	// as before (the left half's own matrices use yet another basis and are not parsed).
	bool HasThighZ;
	float ThighZ[2]; // [0] = right, [1] = left
	std::vector<SBipedToe> Toes;
	int MaxLegLink;
	// arms + fingers
	bool HasClavicleZ;
	float ClavicleZ[2]; // [0] = right, [1] = left
	// Full clavicle local position (parent frame): arm record ([9], [8], [10]) with per-side
	// signs — [8]/[9] are the x/y components (zero on humanoids, real offsets on monster rigs
	// like tr_mo_c05/fy_mo_frahar), [10] the side offset. Left half stores (-x, -y, z).
	NLMISC::CVector ClavicleOff[2]; // [0] = right, [1] = left
	float ClavicleA[2]; // arm record [6] per side: extra rotation about the spine2-frame -X, radians
	float ClavicleB[2]; // arm record [7] per side: base rotation = 180deg about (cos phi, 0, sin phi), phi = pi/4 + b/2
	std::vector<SBipedFinger> Fingers;
	// chains
	SBipedChain Spine, Tail, Pony1, Pony2;
	std::vector<NLMISC::CVector> NeckAngles;
	// runtime state, captured during the walk
	NLMISC::CQuat PelvisWorldRot;
	bool HavePelvisWorldRot;
	NLMISC::CQuat LastSpineWorldRot; // last spine link's world rotation (clavicle reference frame)
	bool HaveLastSpineWorldRot;
	float ToeBaseWorldZ;             // L toe0 attach world height = ground level (footsteps Z)
	bool HaveToeBaseWorldZ;
	// Footsteps bone bookkeeping for the post-walk ground-level patch (the Footsteps bone is
	// walked before the toes, so its Z can only be fixed up afterwards).
	sint32 FootstepsBoneIdx;
	NLMISC::CMatrix FootstepsParentWorld;
	SBipedRig() : Sys(NULL), HasCom(false), ComPos(NLMISC::CVector::Null), ComRot(NLMISC::CQuat::Identity),
		ComDisp(NLMISC::CVector::Null), HasThighZ(false), MaxLegLink(2),
		HasClavicleZ(false),
		PelvisWorldRot(NLMISC::CQuat::Identity), HavePelvisWorldRot(false),
		LastSpineWorldRot(NLMISC::CQuat::Identity), HaveLastSpineWorldRot(false),
		ToeBaseWorldZ(0.0f), HaveToeBaseWorldZ(false),
		FootstepsBoneIdx(-1)
	{
		ThighZ[0] = ThighZ[1] = 0.0f;
		ClavicleZ[0] = ClavicleZ[1] = 0.0f;
		ClavicleA[0] = ClavicleA[1] = 0.0f;
		ClavicleB[0] = ClavicleB[1] = 0.0f;
	}
};

// Rigs per Biped system object; cleared per file.
static std::map<CSceneClass *, SBipedRig> g_bipedRigs;
// The rig currently being decoded by getBipedLocal (set by walkNode before the call). This keeps
// the joint-decode helpers below (bipedChunkFloats & co) signature-compatible.
static SBipedRig *g_rig = NULL;

// Fetch a raw float array from a chunk on the current rig's biped object (orphaned or m_Chunks).
// Returns NULL if absent or too short. Optionally returns the total float count.
static const float *bipedChunkFloats(uint16 chunkId, size_t minFloats, size_t *countOut = NULL)
{
	if (!g_rig || !g_rig->Sys) return NULL;
	IStorageObject *chunk = findChunkAnywhere(g_rig->Sys, chunkId);
	if (!chunk) return NULL;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw || raw->Value.size() < minFloats * 4) return NULL;
	if (countOut) *countOut = raw->Value.size() / 4;
	return reinterpret_cast<const float *>(raw->Value.data());
}

// Paired pose records (legs 0x0069, arms 0x006a) and the paired sections of the structure records
// (0x000f, 0x0010) store the RIGHT side in the first half and the LEFT side in the second half.
// Established 2026-07-06 via the Max 9 differential dataset (single-bone rotations: R Hand lands at
// [28..31], L Hand at [half+28..]; R Clavicle at 0x0010[6..7], L at [half+6..7]). The earlier
// assumption of left-first was undetectable on symmetric rigs (L == mirror(R)) and is exactly what
// broke on the asymmetric-edited ones (bird-rig L forearm, kami_guide_4 L toe).
// Returns a pointer to the given side's half (NULL when missing) and the half length.
static const float *bipedSideHalf(uint16 chunkId, bool leftSide, size_t minFloats, size_t *halfOut = NULL)
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
static const float BIPED_EPS_TWIST = 0.0008f;

static inline uint32 floatBitsAsUint(float f)
{
	uint32 u;
	memcpy(&u, &f, 4);
	return u;
}

// Quat from a record matrix whose 3x3 rows are the NeL I/J/K basis columns directly.
static NLMISC::CQuat matRowsIJKQuat(const float *m)
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
static NLMISC::CQuat chainAngleQuat(const NLMISC::CVector &a)
{
	NLMISC::CQuat qx(NLMISC::CAngleAxis(NLMISC::CVector(1.0f, 0.0f, 0.0f), a.z));
	NLMISC::CQuat qz(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 0.0f, 1.0f), -a.x));
	NLMISC::CQuat qy(NLMISC::CAngleAxis(NLMISC::CVector(0.0f, 1.0f, 0.0f), a.y));
	NLMISC::CQuat r = qx * qz * qy;
	r.normalize();
	return r;
}

// The leg pose-record half's head grows by 2 floats per leg link beyond 3 (observed: 3-link
// rigs put the thigh quat at [2], foot quat at [28]; 4-link rigs at [4]/[30] — confirmed on the
// Max 9 dataset's s_leg4 AND the corpus mounts/birds). The knee ([0]) and horse ankle ([1])
// hinge slots do NOT shift. Everything at or after the thigh quat does.
static int legHeadShift()
{
	if (!g_rig) return 0;
	int s = 2 * (g_rig->MaxLegLink - 2);
	return s > 0 ? s : 0;
}

// Read a stored quat (x,y,z,w) from a side's half at the given in-half offset.
static bool readSideQuat(uint16 chunkId, bool leftSide, int off, NLMISC::CQuat &out)
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
static NLMISC::CQuat mirrorQuatLR(const NLMISC::CQuat &q)
{
	return NLMISC::CQuat(q.x, q.y, -q.z, -q.w);
}

// The biped-internal Y-up basis change as a rotation: C = [[1,0,0],[0,0,-1],[0,1,0]] = Rx(+90deg).
static const NLMISC::CQuat QUAT_C(0.70710678f, 0.0f, 0.0f, 0.70710678f);
// 180-degree axis rotations used by the per-side upperarm frames.
static const NLMISC::CQuat QUAT_I(1.0f, 0.0f, 0.0f, 0.0f); // Rx(pi)
static const NLMISC::CQuat QUAT_J(0.0f, 1.0f, 0.0f, 0.0f); // Ry(pi)

// Thigh: stored per side at [2..5] of the 0x0069 half, pelvis-relative, gathered as
// q = (s2, s3, -s0, s1) — which yields the LEFT-convention rotation; the right half is
// mirror-encoded, so the right side applies the LR mirror to its own half's decode. (The
// differential dataset's canonical poses are mirror-degenerate for exactly these quat shapes,
// which is why per-side "no mirror" also passed there; the corpus discriminates.)
static bool thighWorldRot(bool leftSide, NLMISC::CQuat &worldRot)
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
static bool footWorldRot(bool leftSide, NLMISC::CQuat &worldRot)
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
static bool upperArmWorldRot(bool leftSide, NLMISC::CQuat &worldRot)
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
static bool handWorldRot(bool leftSide, const NLMISC::CQuat &forearmWorldRot, NLMISC::CQuat &worldRot)
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
static bool headWorldRot(NLMISC::CQuat &worldRot)
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
static bool hingeLocalRot(uint16 chunkId, bool leftSide, int slot, bool interior, NLMISC::CQuat &out)
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
static bool chainPoseBaseDelta(uint16 chunkId, bool leftSide, int chainIdx, NLMISC::CQuat &delta)
{
	int shift = (chunkId == 0x0069) ? legHeadShift() : 0;
	if (!readSideQuat(chunkId, leftSide, 46 + shift + 10 * chainIdx, delta)) return false;
	delta.invert();
	return true;
}

static bool chainPoseLinkAngle(uint16 chunkId, bool leftSide, int chainIdx, int sub, float &angle)
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
static void parseChainRecord(uint16 structId, uint16 angleId, SBipedChain &out)
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

// Parse the leg record (0x000f): per-side thigh offsets + per-toe base matrices/lengths (from
// the first/right half; the left half's matrices use another basis and are mirrored instead).
static void parseLegRecord(SBipedRig &rig)
{
	size_t n = 0;
	const float *f = bipedChunkFloats(0x000f, 12, &n);
	if (!f) return;
	rig.ThighZ[0] = f[1];
	rig.ThighZ[1] = (n / 2 + 1 < n) ? f[n / 2 + 1] : f[1];
	rig.HasThighZ = true;
	size_t i = 10;
	uint32 nToes = floatBitsAsUint(f[i]); ++i;
	if (nToes > 16) return;
	for (uint32 t = 0; t < nToes; ++t)
	{
		if (i >= n) return;
		uint32 nl = floatBitsAsUint(f[i]); ++i;
		if (nl < 1 || nl > 16 || i + 16 + nl > n) return;
		SBipedToe toe;
		toe.NLinks = (int)nl;
		const float *m = f + i; i += 16;
		// toe base: rows-as-IJK quat with the (x,y,-z,-w) z-flip; pos (x,y,-z)
		toe.Rot = mirrorQuatLR(matRowsIJKQuat(m));
		toe.Pos = NLMISC::CVector(m[12], m[13], -m[14]);
		toe.Lens.assign(f + i, f + i + nl);
		i += nl;
		rig.Toes.push_back(toe);
	}
}

// Parse the arm record (0x0010): per-side clavicle params + per-finger base matrices/lengths
// (from the first/right half; see parseLegRecord).
// Finger matrices use the Y-up conversion (position (x,-z,y); rotation C*M^T), unlike the
// toe/chain records — validated per record type, not assumed uniform.
static void parseArmRecord(SBipedRig &rig)
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
	uint32 nFingers = floatBitsAsUint(f[16]);
	if (nFingers > 16) return;
	size_t i = 17;
	for (uint32 fi = 0; fi < nFingers; ++fi)
	{
		if (i >= n) return;
		uint32 nl = floatBitsAsUint(f[i]); ++i;
		if (nl < 1 || nl > 16 || i + 16 + nl > n) return;
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
		rig.Fingers.push_back(fing);
	}
}

// Parse the COM record (0x006c: [4..6] position Y-up, [8..11] world rotation quat Y-up), with
// 0x0104 (Y-up 4x4, canonical -90degZ rotation assumed) as the fallback.
static void parseComRecord(SBipedRig &rig)
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
		NLMISC::CMatrix rm; rm.identity(); rm.setRot(rig.ComRot);
		rig.ComPos = NLMISC::CVector(c[4], -c[6], c[5]) + rm.mulVector(rig.ComDisp);
		rig.HasCom = true;
		return;
	}
	const float *com = bipedChunkFloats(0x0104, 15, &n);
	if (com)
	{
		rig.ComPos = NLMISC::CVector(com[12], -com[14], com[13]);
		rig.ComRot = NLMISC::CQuat(0.0f, 0.0f, -0.70710678f, 0.70710678f);
		rig.HasCom = true;
	}
}

// Parse neck angles. The authoritative source is the neck record 0x0065 ([0]=int count of angle
// floats = 3 per link, [1..count] = (a1,a2,a3) triples, then a 4x4 base-attach matrix) — the Max 9
// differential dataset shows neck edits land there, and 48/169 corpus files carry stale copies in
// the head record's [8..] region (tr_mo_arma/bul, the ge_ kami pair). Fall back to the head-record
// copy (0x0064: [7]=int count, [8..] triples) only when 0x0065 is absent.
static void parseNeckAngles(SBipedRig &rig)
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
static bool locateChainSub(const std::vector<SBipedToe> &toes, uint32 link, int &toeIdx, int &sub)
{
	int rem = (int)link;
	for (size_t t = 0; t < toes.size(); ++t)
	{
		if (rem < toes[t].NLinks) { toeIdx = (int)t; sub = rem; return true; }
		rem -= toes[t].NLinks;
	}
	return false;
}

static bool locateChainSub(const std::vector<SBipedFinger> &fingers, uint32 link, int &fi, int &sub)
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
static void getBipedLocal(INode *node, const NLMISC::CQuat &parentWorldRot,
                          NLMISC::CVector &pos, NLMISC::CQuat &rot, NLMISC::CVector &scale,
                          NLMISC::CQuat &worldRotOut)
{
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
			// Thigh position: pure side offset in the pelvis frame, per-side leg record value.
			posOverride = NLMISC::CVector(0.0f, 0.0f, (id == BID_LLEG) ? rig.ThighZ[1] : -rig.ThighZ[0]);
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
		havePosOverride = true;
	}
	else if (haveId && (id == BID_LFINGERS || id == BID_RFINGERS) && !rig.Fingers.empty()) // fingers
	{
		int fi = 0, sub = 0;
		if (locateChainSub(rig.Fingers, link, fi, sub))
		{
			const SBipedFinger &fing = rig.Fingers[fi];
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
	else if (haveId && (id == BID_LTOES || id == BID_RTOES) && !rig.Toes.empty()) // toes
	{
		bool tLeft = (id == BID_LTOES);
		int ti = 0, sub = 0;
		if (locateChainSub(rig.Toes, link, ti, sub))
		{
			const SBipedToe &toe = rig.Toes[ti];
			if (sub == 0)
			{
				// toe base: creation-time local transform from the leg-record matrix, composed
				// with the pose block's per-side base delta (local post-multiply; the right half's
				// delta is mirror-encoded, so the mirror wraps the composed local).
				NLMISC::CQuat delta;
				bool haveDelta = chainPoseBaseDelta(0x0069, tLeft, ti, delta);
				rot = toe.Rot;
				if (haveDelta) rot = rot * delta;
				if (tLeft) { pos = toe.Pos; }
				else { pos = NLMISC::CVector(toe.Pos.x, toe.Pos.y, -toe.Pos.z); rot = mirrorQuatLR(rot); }
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
		// per-link angles when present.
		rot = rig.Spine.MatRot;
		if (!rig.Spine.Angles.empty()) rot = rot * chainAngleQuat(rig.Spine.Angles[0]);
		rot.normalize();
		pos = rig.Spine.MatPos;
		haveLocalDirect = true;
		worldRot = parentWorldRot * rot;
	}
	else if (haveId && id == BID_SPINE && link > 0 && link < rig.Spine.Angles.size())
	{
		rot = chainAngleQuat(rig.Spine.Angles[link]);
		worldRot = parentWorldRot * rot;
		// Spine link position: the previous link's stored length (like tail/pony links).
		if (link - 1 < rig.Spine.Lens.size())
			pos = NLMISC::CVector(rig.Spine.Lens[link - 1], 0.0f, 0.0f);
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
			// spine link's own 0x096c dims don't track it; confirmed on kami_keep_2).
			pos = NLMISC::CVector(rig.Spine.Lens.back(), 0.0f, 0.0f);
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
			if (!chain.Angles.empty()) rot = rot * chainAngleQuat(chain.Angles[0]);
			rot.normalize();
			pos = chain.MatPos;
			haveLocalDirect = true;
			worldRot = parentWorldRot * rot;
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

// Build ordered children list per parent by scanning the CSceneClassContainer in scene order.
// INode::children() is a std::set keyed by pointer, so its iteration order is unstable across
// runs — Max preserves original scene order (which is how bones get numbered in the .skel).
static std::vector<INode *> orderedChildrenOf(INode *parent, CSceneClassContainer *ssc)
{
	std::vector<INode *> out;
	for (auto it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
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
static CSceneClass *bipedSystemOfCtrl(CReferenceMaker *tmCtrl)
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
static bool isBipedComNode(INode *node)
{
	CSceneClass *tmsc = dynamic_cast<CSceneClass *>(node->getReference(0));
	return tmsc && tmsc->classDesc()->classId() == CLASSID_BIPED_VHT_CTRL;
}

// Highest leg link index (last leg segment = Foot) among this rig's BipDriven bones. The
// SDK-documented leg chain is Thigh(0), Calf(1), [HorseLink(2) only on 4-link mount/horse rigs],
// Foot(last). Defaults to 2 (3-link legs) when no leg bones are found.
static void computeMaxLegLink(SBipedRig &rig, CSceneClassContainer *ssc)
{
	int maxLink = -1;
	for (auto it = ssc->chunks().begin(); it != ssc->chunks().end(); ++it)
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
static SBipedRig &rigFor(CSceneClass *sys, CSceneClassContainer *ssc)
{
	std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.find(sys);
	if (it != g_bipedRigs.end()) return it->second;
	SBipedRig &rig = g_bipedRigs[sys];
	rig.Sys = sys;
	g_rig = &rig; // bipedChunkFloats reads through g_rig during parsing
	parseComRecord(rig);
	parseLegRecord(rig);
	parseArmRecord(rig);
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
static void walkNode(INode *node, sint32 fatherId, const NLMISC::CMatrix &parentWorld,
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
			getBipedLocal(node, parentWorld.getRot(), realPos, realRot, realScale, worldRotOut);
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

	sint32 myId = (sint32)bones.size();
	bones.push_back(b);

	// Footsteps / ground-level bookkeeping (see patchFootstepsGround). The corpus-era Footsteps
	// node carries a plain PRS controller (not a BipDriven with BID_FOOTPRINTS like later plugin
	// versions), so it is identified as a COM child by name.
	if (bipedSys && isBipedBoneNode(node))
	{
		SBipedRig &rig = g_bipedRigs[bipedSys];
		uint32 fid = 0, flink = 0;
		if (readBipDrivenIdLink(node, fid, flink))
		{
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
	for (INode *child : kids) walkNode(child, myId, worldTM, ssc, bones, nameSet);
}

// The Footsteps marker sits at ground level: its world height equals the (left) toe attach height
// (exact on the Max 9 differential dataset across ankle/height/foot-scale variants; the value is
// derived, not stored — which is why exhaustive scans never found e.g. ca_spaceship's 7.73).
// The Footsteps bone precedes the toes in walk order, so patch it after the walk: keep its world
// X/Y, set world Z to the toe attach height, recompute local pos + InvBindPos.
static void patchFootstepsGround(std::vector<Bone> &bones)
{
	for (std::map<CSceneClass *, SBipedRig>::iterator it = g_bipedRigs.begin(); it != g_bipedRigs.end(); ++it)
	{
		SBipedRig &rig = it->second;
		if (rig.FootstepsBoneIdx < 0 || !rig.HaveToeBaseWorldZ) continue;
		Bone &b = bones[(size_t)rig.FootstepsBoneIdx];
		NLMISC::CMatrix localTM = makeLocalTM(b.OrigPos, b.OrigRot, b.DefaultScale);
		NLMISC::CMatrix worldTM = rig.FootstepsParentWorld * localTM;
		NLMISC::CVector wp = worldTM.getPos();
		wp.z = rig.ToeBaseWorldZ;
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
	}
}

// Double-precision variant: parent-world is Mat4D, local build + multiply + invert happen in
// double, cast to float only when constructing the final float32 CMatrix + serialize.
static void walkNodeD(INode *node, sint32 fatherId, const Mat4D &parentWorld,
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
		NLMISC::CQuat worldRotOut;
		getBipedLocal(node, pm.getRot(), realPos, realRot, realScale, worldRotOut);
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

	sint32 myId = (sint32)bones.size();
	bones.push_back(b);

	std::vector<INode *> kids = orderedChildrenOf(node, ssc);
	for (INode *child : kids) walkNodeD(child, myId, worldTM, ssc, bones, nameSet);
}

// Serialize a CSkeletonShape file (SHAP magic + CShapeStream + CSkeletonShape v1 + CBoneBase v2 + CLod v0).
static void writeSkel(const std::string &path, const std::vector<Bone> &bonesIn)
{
	// CSkeletonShape::build semantics, reproduced exactly (nel/src/3d/skeleton_shape.cpp):
	// 1. A bone must be LOD-disabled no later than its father: inherit/clamp LodDisableDistance.
	// 2. One lod per distinct non-zero distance (ascending) + the base lod; a lod disables every
	//    bone whose distance is non-zero and <= the lod's distance.
	std::vector<Bone> bones = bonesIn;
	for (size_t i = 0; i < bones.size(); ++i)
	{
		sint32 fa = bones[i].FatherId;
		if (fa >= 0 && bones[(size_t)fa].LodDisableDistance != 0.0f)
		{
			float fatherDist = bones[(size_t)fa].LodDisableDistance;
			if (bones[i].LodDisableDistance == 0.0f) bones[i].LodDisableDistance = fatherDist;
			else bones[i].LodDisableDistance = std::min(bones[i].LodDisableDistance, fatherDist);
		}
	}
	std::set<float> distSet;
	for (size_t i = 0; i < bones.size(); ++i)
		if (bones[i].LodDisableDistance > 0.0f)
			distSet.insert(bones[i].LodDisableDistance);

	NLMISC::COFile of(path);
	// SHAP magic — serialCheck writes NELID("PAHS") which is "SHAP" big-endian
	of.serialCheck(NELID("PAHS"));

	// serialPolyPtr for CSkeletonShape: write u64 id=1 (first ptr encountered), then class name.
	uint64 nodeId = 1;
	of.serial(nodeId);
	std::string className = "CSkeletonShape";
	of.serial(className);

	// CSkeletonShape::serial: serialVersion(1) + serialCont(_Bones) + serialCont(_BoneMap) + serialCont(_Lods)
	uint8 skelVersion = 1;
	of.serial(skelVersion);

	// _Bones: sint32 count + each CBoneBase
	sint32 boneCount = (sint32)bones.size();
	of.serial(boneCount);
	for (const Bone &b : bones)
	{
		uint8 boneVer = 2;
		of.serial(boneVer);
		std::string name = b.Name;
		of.serial(name);
		NLMISC::CMatrix invBind = b.InvBindPos;
		invBind.serial(of);
		sint32 father = b.FatherId;
		of.serial(father);
		bool unherit = b.UnheritScale;
		of.serial(unherit);
		float lodDist = b.LodDisableDistance;
		of.serial(lodDist);
		// CTrackDefaultVector DefaultPos: version(0) + CVector
		uint8 tVer = 0;
		of.serial(tVer); NLMISC::CVector v = b.DefaultPos; of.serial(v);
		of.serial(tVer); NLMISC::CVector euler = NLMISC::CVector::Null; of.serial(euler); // DefaultRotEuler
		of.serial(tVer); NLMISC::CQuat q = b.DefaultRotQuat; of.serial(q);
		of.serial(tVer); NLMISC::CVector s = b.DefaultScale; of.serial(s);
		of.serial(tVer); NLMISC::CVector pivot = NLMISC::CVector::Null; of.serial(pivot);
		// SkinScale (ver>=2)
		NLMISC::CVector skinScale(1, 1, 1);
		of.serial(skinScale);
	}

	// _BoneMap: std::map<string, uint32>, iterated in std::map's sorted key order (not bone
	// insertion order — that was a bug that manifested as a mismatch in the _BoneMap byte
	// range against mesh_export's output, which uses CSkeletonShape::build's actual std::map).
	sint32 mapCount = (sint32)bones.size();
	of.serial(mapCount);
	std::map<std::string, uint32> boneMap;
	for (uint32 i = 0; i < bones.size(); ++i) boneMap[bones[i].Name] = i;
	for (auto it = boneMap.begin(); it != boneMap.end(); ++it)
	{
		std::string n = it->first;
		of.serial(n);
		uint32 idx = it->second;
		of.serial(idx);
	}

	// _Lods: base lod (all bones active) + one lod per distinct LodDisableDistance
	sint32 lodCount = (sint32)(1 + distSet.size());
	of.serial(lodCount);
	{
		uint8 lodVer = 0;
		of.serial(lodVer);
		float lodDistance = 0.0f;
		of.serial(lodDistance);
		sint32 activeCount = (sint32)bones.size();
		of.serial(activeCount);
		for (sint32 i = 0; i < activeCount; ++i)
		{
			uint8 active = 0xFF;
			of.serial(active);
		}
	}
	for (std::set<float>::iterator it = distSet.begin(); it != distSet.end(); ++it)
	{
		uint8 lodVer = 0;
		of.serial(lodVer);
		float lodDistance = *it;
		of.serial(lodDistance);
		sint32 activeCount = (sint32)bones.size();
		of.serial(activeCount);
		for (sint32 i = 0; i < activeCount; ++i)
		{
			float dist = bones[(size_t)i].LodDisableDistance;
			uint8 active = (lodDistance >= dist && dist != 0.0f) ? 0x00 : 0xFF;
			of.serial(active);
		}
	}
}

// Format a float for JSON such that assimp's parser sees it as a floating-point number and
// not an integer. Assimp's glTF loader picks the smallest integer type first when the JSON
// token has no '.' or 'e', so we force decimal notation on integer-valued floats.
// %.9g round-trips float32 exactly (24-bit mantissa needs 9 decimal digits).
static std::string formatFloat(float v)
{
	char buf[32];
	if (std::isnan(v) || std::isinf(v))
	{
		// glTF spec forbids these in TRS values; use 0 to keep the file valid rather than
		// emitting an invalid token. Should not occur for real skeleton data.
		return "0.0";
	}
	snprintf(buf, sizeof(buf), "%.9g", (double)v);
	// Append ".0" if the printed form has no decimal marker so the reader picks a float type.
	if (!strchr(buf, '.') && !strchr(buf, 'e') && !strchr(buf, 'E'))
	{
		size_t n = strlen(buf);
		if (n + 2 < sizeof(buf)) { buf[n] = '.'; buf[n+1] = '0'; buf[n+2] = 0; }
	}
	return buf;
}

// Escape a name for embedding in JSON. Skeleton node names are typically simple identifiers
// (no quotes, backslashes, newlines), but escape defensively.
static std::string jsonEscape(const std::string &s)
{
	std::string out;
	out.reserve(s.size() + 2);
	for (char c : s)
	{
		switch (c)
		{
		case '"':  out += "\\\""; break;
		case '\\': out += "\\\\"; break;
		case '\n': out += "\\n";  break;
		case '\r': out += "\\r";  break;
		case '\t': out += "\\t";  break;
		default:
			if ((unsigned char)c < 0x20)
			{
				char buf[8]; snprintf(buf, sizeof(buf), "\\u%04x", (int)(unsigned char)c);
				out += buf;
			}
			else out += c;
		}
	}
	return out;
}

// Emit the skeleton as glTF 2.0. Skeleton-only — no meshes, no skins, no materials. Each bone
// becomes a node with its local translation/rotation/scale. Children lists are built from the
// bones' FatherId. mesh_export can walk this and reconstruct the same bone data using the
// same NeL CMatrix operations, producing a byte-identical .skel modulo glTF float rasterization.
//
// float precision: JSON stores numbers as doubles; we write with %.9g which round-trips a
// float32 exactly (float32 has ~7 decimal digits, 9 is enough).
//
// The root bone's DefaultPos/DefaultRotQuat are already zeroed by walkNode's "if fatherId<0"
// branch (NeL buildSkeleton convention). If the downstream reader needs the REAL root
// transform, it would have to be encoded separately — we don't need it for skel round-trip
// since our .skel writer applies the same reset.
static void writeGltf(const std::string &path, const std::vector<Bone> &bones)
{
	// Build per-parent children lists
	std::vector<std::vector<size_t> > children(bones.size());
	for (size_t i = 0; i < bones.size(); ++i)
		if (bones[i].FatherId >= 0)
			children[(size_t)bones[i].FatherId].push_back(i);

	FILE *fp = fopen(path.c_str(), "w");
	if (!fp) { std::cerr << "cannot open " << path << " for writing\n"; return; }

	fprintf(fp,
		"{\n"
		"  \"asset\": {\"version\": \"2.0\", \"generator\": \"pipeline_max_export_skel\"},\n"
		"  \"scene\": 0,\n"
		"  \"scenes\": [{\"nodes\": [0]}],\n"
		"  \"nodes\": [\n");

	for (size_t i = 0; i < bones.size(); ++i)
	{
		const Bone &b = bones[i];
		fprintf(fp, "    {");
		fprintf(fp, "\"name\": \"%s\"", jsonEscape(b.Name).c_str());
		// Use the REAL local transform (OrigPos/OrigRot), NOT the reset DefaultPos/DefaultRotQuat.
		// The root reset is applied by the .skel writer, and mesh_export re-applies it on its
		// side; feeding reset values through glTF would zero out the source's InvBindPos noise
		// pattern and give a false byte-diff between the two paths.
		fprintf(fp, ", \"translation\": [%s, %s, %s]",
			formatFloat(b.OrigPos.x).c_str(), formatFloat(b.OrigPos.y).c_str(), formatFloat(b.OrigPos.z).c_str());
		fprintf(fp, ", \"rotation\": [%s, %s, %s, %s]",
			formatFloat(b.OrigRot.x).c_str(), formatFloat(b.OrigRot.y).c_str(),
			formatFloat(b.OrigRot.z).c_str(), formatFloat(b.OrigRot.w).c_str());
		fprintf(fp, ", \"scale\": [%s, %s, %s]",
			formatFloat(b.DefaultScale.x).c_str(), formatFloat(b.DefaultScale.y).c_str(), formatFloat(b.DefaultScale.z).c_str());
		if (!children[i].empty())
		{
			fprintf(fp, ", \"children\": [");
			for (size_t k = 0; k < children[i].size(); ++k)
				fprintf(fp, "%s%zu", k ? ", " : "", children[i][k]);
			fprintf(fp, "]");
		}
		// Extras: NeL-specific per-node data that either isn't representable in stock glTF (the
		// two convertMatrix flags) or has to be preserved bit-exact for our .skel roundtrip
		// (the T/R/S floats). See wiki: nel_gltf_extras.md for the full catalogue.
		//
		// Rationale for nel_tx/ty/... instead of nel_translation: assimp maps JSON arrays into
		// aiMetadata differently across versions, and mapping scalar JSON numbers to typed
		// metadata entries is the most portable path. Cost is a few extra keys per node.
		fprintf(fp, ", \"extras\": {");
		fprintf(fp, "\"nel_tx\": %s, \"nel_ty\": %s, \"nel_tz\": %s",
			formatFloat(b.OrigPos.x).c_str(), formatFloat(b.OrigPos.y).c_str(), formatFloat(b.OrigPos.z).c_str());
		fprintf(fp, ", \"nel_rx\": %s, \"nel_ry\": %s, \"nel_rz\": %s, \"nel_rw\": %s",
			formatFloat(b.OrigRot.x).c_str(), formatFloat(b.OrigRot.y).c_str(),
			formatFloat(b.OrigRot.z).c_str(), formatFloat(b.OrigRot.w).c_str());
		fprintf(fp, ", \"nel_sx\": %s, \"nel_sy\": %s, \"nel_sz\": %s",
			formatFloat(b.DefaultScale.x).c_str(), formatFloat(b.DefaultScale.y).c_str(), formatFloat(b.DefaultScale.z).c_str());
		fprintf(fp, ", \"nel_unheritScale\": %s", b.UnheritScale ? "true" : "false");
		fprintf(fp, ", \"nel_lodDisableDistance\": %s", formatFloat(b.LodDisableDistance).c_str());
		fprintf(fp, "}");
		fprintf(fp, "}%s\n", (i + 1 < bones.size()) ? "," : "");
	}

	fprintf(fp,
		"  ]\n"
		"}\n");
	fclose(fp);
}

// Biped ClassIds are confirmed from the character-studio MAXScript reference:
//   bipedSystem                    {9155,0}  — the biped system (root, superclass 0x60/Object)
//   Vertical_Horizontal_Turn       {9156,0}  — COM/body controller (superclass 0x9008/ControlTransform)
//   BipDriven_Control              {9154,0}  — per-body-part controller (was BipSlave_Control pre-2022)
//   Biped_SubAnim                  {0x6b147369, 0x078c6b2a}  — sub-anim (superclass 0x9003/ControlFloat)
//   biped_object                   {9125,0}  — per-body-part geometry (superclass 0x10/GeomObject)
static const NLMISC::CClassId CLASSID_BIPED_SYSTEM (0x00009155, 0x00000000);
static const NLMISC::CClassId CLASSID_BIPED_VHT    (0x00009156, 0x00000000);
static const NLMISC::CClassId CLASSID_BIPED_DRIVEN (0x00009154, 0x00000000);
static const NLMISC::CClassId CLASSID_BIPED_SUBANIM(0x6b147369, 0x078c6b2a);
static const NLMISC::CClassId CLASSID_BIPED_OBJECT (0x00009125, 0x00000000);

// Detect a biped file by scanning the ClassDirectory3 for any of the four Biped ClassIds. The
// DllDirectory-based check would false-positive because biped.dlc is loaded even when no biped
// exists in the scene (Max loads all installed plugins); only the ClassDirectory3 entry appears
// when a biped class is actually instantiated. Using ClassId not display name so this survives
// the plugin's rename of BipSlave_Control → BipDriven_Control across the corpus's Max versions.
static bool looksLikeBipedFile(CClassDirectory3 &cd)
{
	for (auto it = cd.chunks().begin(); it != cd.chunks().end(); ++it)
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

int main(int argc, char **argv)
{
	// Args: [--double] [--gltf <path>] [--allow-biped-degraded] <input.max> <output.skel>
	//   --double                 world-matrix accumulation in double instead of float
	//   --gltf <path>            also write a glTF 2.0 skeleton-only file next to the .skel; used for
	//                            the mesh_export roundtrip validator (Blender-importable,
	//                            assimp-readable)
	//   --allow-biped-degraded   proceed on biped files with identity local transforms + a warning
	//                            (bind pose is stored inside proprietary Biped controller chunks and
	//                            we don't yet decode them, so the output has correct names + hierarchy
	//                            but wrong per-bone InvBindPos); default is to refuse with an error so
	//                            silent-broken outputs don't slip into the corpus.
	int argi = 1;
	const char *gltfOut = NULL;
	bool allowBipedDegraded = false;
	while (argi < argc && argv[argi][0] == '-' && argv[argi][1] == '-')
	{
		if (std::string(argv[argi]) == "--double") { g_useDouble = true; ++argi; }
		else if (std::string(argv[argi]) == "--gltf" && argi + 1 < argc) { gltfOut = argv[argi + 1]; argi += 2; }
		else if (std::string(argv[argi]) == "--allow-biped-degraded") { allowBipedDegraded = true; ++argi; }
		else break;
	}
	if (argc - argi < 2) { std::cerr << "usage: export_skel [--double] [--gltf <path>] [--allow-biped-degraded] <input.max> <output.skel>\n"; return 1; }
	const char *maxFile = argv[argi];
	const char *skelOut = argv[argi + 1];

	g_set_prgname(argv[0]);
	gsf_init();

	CSceneClassRegistry reg;
	CBuiltin::registerClasses(&reg);
	UPDATE1::CUpdate1::registerClasses(&reg);
	EPOLY::CEPoly::registerClasses(&reg);
	BIPED::CBiped::registerClasses(&reg);

	GsfInput *src = gsf_input_stdio_new(maxFile, NULL);
	if (!src) { std::cerr << "cannot open " << maxFile << "\n"; return 1; }
	GsfInfile *in = gsf_infile_msole_new(src, NULL);
	if (!in) { std::cerr << "not an OLE file\n"; return 1; }

	CDllDirectory dll;
	{ GsfInput *s = gsf_infile_child_by_name(in, "DllDirectory"); CStorageStream st(s); dll.serial(st); g_object_unref(s); }
	dll.parse(VersionUnknown);
	CClassDirectory3 cd(&dll);
	{ GsfInput *s = gsf_infile_child_by_name(in, "ClassDirectory3"); CStorageStream st(s); cd.serial(st); g_object_unref(s); }
	cd.parse(VersionUnknown);

	bool isBiped = looksLikeBipedFile(cd);
	(void)allowBipedDegraded; // legacy flag, now a no-op — biped bind pose is reconstructed
	CScene scene(&reg, &dll, &cd);
	{ GsfInput *s = gsf_infile_child_by_name(in, "Scene"); CStorageStream st(s); scene.serial(st); g_object_unref(s); }
	scene.parse(VersionUnknown);

	INode *root = scene.container()->scene()->rootNode();
	INode *bip01 = root->find(ucstring("Bip01"));
	if (!bip01) { std::cerr << "Bip01 not found in " << maxFile << "\n"; return 2; }

	// Per-system rig state (COM, per-limb records) is parsed lazily during the walk: each biped
	// bone's TM controller references its own Biped (0x9155) system object as getReference(0),
	// which handles files with multiple bipeds (e.g. tr_mo_kitin_queen's Bip01 + Bip02) correctly.
	g_bipedRigs.clear();
	g_rig = NULL;

	std::vector<Bone> bones;
	std::set<std::string> nameSet;
	if (g_useDouble && !isBiped)
	{
		Mat4D rootMatD = Mat4D::identity();
		walkNodeD(bip01, -1, rootMatD, scene.container(), bones, nameSet);
	}
	else
	{
		NLMISC::CMatrix rootMat; rootMat.identity();
		walkNode(bip01, -1, rootMat, scene.container(), bones, nameSet);
		patchFootstepsGround(bones);
	}

	std::cout << "Extracted " << bones.size() << " bones from " << maxFile << "\n";
	for (size_t i = 0; i < bones.size() && i < 10; ++i)
	{
		std::cout << "  [" << i << "] " << bones[i].Name << " father=" << bones[i].FatherId
		          << " pos=" << bones[i].DefaultPos.toString()
		          << " rotQ=(" << bones[i].DefaultRotQuat.x << "," << bones[i].DefaultRotQuat.y << "," << bones[i].DefaultRotQuat.z << "," << bones[i].DefaultRotQuat.w << ")"
		          << " scale=" << bones[i].DefaultScale.toString()
		          << "\n";
	}

	writeSkel(skelOut, bones);
	std::cout << "Wrote " << skelOut << "\n";

	if (gltfOut)
	{
		writeGltf(gltfOut, bones);
		std::cout << "Wrote " << gltfOut << "\n";
	}

	g_object_unref(in);
	g_object_unref(src);
	gsf_shutdown();
	return 0;
}
