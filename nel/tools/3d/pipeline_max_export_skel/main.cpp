// Skel export: .max -> .skel for non-biped skeletons.
// Reads Bip01, walks children, extracts local transforms from the PRS controller's
// Bezier Position / TCB Rotation / Bezier Scale sub-controllers (default values at
// chunks 0x2503 / 0x2504 / 0x2505), computes world matrices for InvBindPos, emits
// the .skel binary in the same format NeL's CShapeStream + CSkeletonShape + CBoneBase
// produce.

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

#include "../pipeline_max/builtin/scene_impl.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;

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
static const NLMISC::CClassId CLASSID_BIP_DRIVEN(0x00009154, 0x00000000);
static bool isBipedBoneNode(INode *node)
{
	CSceneClass *tmCtrl = dynamic_cast<CSceneClass *>(node->getReference(0));
	if (!tmCtrl) return false;
	return tmCtrl->classDesc()->classId() == CLASSID_BIP_DRIVEN;
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
// (8 bytes = 2 uint32s). The bone_id maps to Autodesk's biped.getIdLink table (12=pelvis,
// 9=spine, 11=head, etc.). We use it to distinguish "straight chain" bones (child.id ==
// parent.id and child.link == parent.link + 1, e.g. Spine → Spine1) from "chain base" bones
// (child.id != parent.id, e.g. Pelvis → Spine crosses biped groups) — the two need different
// local-position rules.
#define CHUNK_BIP_DRIVEN_IDLINK 0x0200
static bool readBipDrivenIdLink(INode *node, uint32 &boneId, uint32 &linkIdx)
{
	CSceneClass *tmCtrl = dynamic_cast<CSceneClass *>(node->getReference(0));
	if (!tmCtrl) return false;
	if (tmCtrl->classDesc()->classId() != CLASSID_BIP_DRIVEN) return false;
	IStorageObject *chunk = findChunkAnywhere(tmCtrl, CHUNK_BIP_DRIVEN_IDLINK);
	if (!chunk) return false;
	CStorageRaw *raw = dynamic_cast<CStorageRaw *>(chunk);
	if (!raw || raw->Value.size() < 8) return false;
	memcpy(&boneId, raw->Value.data(), 4);
	memcpy(&linkIdx, raw->Value.data() + 4, 4);
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
	if (rotSc) readRawBytes(rotSc, CHUNK_TCB_QUAT_VALUE, &rot, 16);
	if (scaleSc) readRawBytes(scaleSc, CHUNK_BEZIER_SCALE_VALUE, &scale, 12);
}

// Autodesk biped internal bone-id constants — 0-based, one less than the MaxScript-facing IDs in
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
};

// Approximate local transform for a biped bone. Rules, in order:
//   1. Parent isn't a BipDriven biped bone (parent is COM/V-H-T, or non-biped): local pos = 0.
//      Matches ref for Bip01 Pelvis, most root-attached biped bones.
//   2. Spine base (id=9, link=0): position inherits from COM per SDK docs, so the offset relative
//      to the INode parent (Pelvis) is roughly the Spine's OWN length. Use self.096c.x.
//   3. Straight-chain default: local pos = (parent.096c.x, 0, 0). Correct for Spine1→Head, finger
//      sub-segments, everything the "position inherits from INode parent" rule applies to.
//   Rotation is left as identity — getting real biped rotations requires the biped's forward
//   kinematics using the rotation-inheritance rules (upper arms/legs/spine-base/feet inherit from
//   COM; clavicles from last spine link). Chain-crossing bones (clavicles, upper limbs, upper
//   legs, feet) still fall through the straight-chain rule and are approximate.
static void getBipedLocalTransformApprox(INode *node, NLMISC::CVector &pos, NLMISC::CQuat &rot, NLMISC::CVector &scale)
{
	pos = NLMISC::CVector::Null;
	rot = NLMISC::CQuat::Identity;
	scale = NLMISC::CVector(1, 1, 1);

	INode *parent = node->parent();
	if (!parent) return;
	if (!isBipedBoneNode(parent)) return; // parent isn't a BipDriven biped bone — rule 1

	uint32 selfId = 0, selfLink = 0;
	if (readBipDrivenIdLink(node, selfId, selfLink) && selfId == BID_SPINE && selfLink == 0)
	{
		// Rule 2: Spine base — position from COM, approximated by own X length.
		NLMISC::CVector selfDims = readNodeBoneDimensions(node);
		pos = NLMISC::CVector(selfDims.x, 0.0f, 0.0f);
		return;
	}

	// Rule 3: straight chain — child at end of parent along parent's X axis.
	NLMISC::CVector parentDims = readNodeBoneDimensions(parent);
	pos = NLMISC::CVector(parentDims.x, 0.0f, 0.0f);
}

static NLMISC::CMatrix makeLocalTM(const NLMISC::CVector &pos, const NLMISC::CQuat &rot, const NLMISC::CVector &scale)
{
	NLMISC::CMatrix m;
	m.identity();
	m.setPos(pos);
	m.setRot(rot);
	m.scale(scale);
	return m;
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
	b.LodDisableDistance = 0.0f;

	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	CReferenceMaker *tmCtrl = node->getReference(0);
	if (isBipedBoneNode(node))
	{
		// Straight-chain biped approximation until we decode the full biped rig — see
		// getBipedLocalTransformApprox for what this does and doesn't cover.
		getBipedLocalTransformApprox(node, realPos, realRot, realScale);
		b.UnheritScale = true; // biped bones always unherit their parent's scale (see plugin_max/nel_mesh_lib)
	}
	else
	{
		getLocalTransform(tmCtrl, realPos, realRot, realScale);
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

	std::vector<INode *> kids = orderedChildrenOf(node, ssc);
	for (INode *child : kids) walkNode(child, myId, worldTM, ssc, bones, nameSet);
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
	b.LodDisableDistance = 0.0f;

	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	CReferenceMaker *tmCtrl = node->getReference(0);
	if (isBipedBoneNode(node))
	{
		// Straight-chain biped approximation until we decode the full biped rig — see
		// getBipedLocalTransformApprox for what this does and doesn't cover.
		getBipedLocalTransformApprox(node, realPos, realRot, realScale);
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
static void writeSkel(const std::string &path, const std::vector<Bone> &bones)
{
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

	// _Lods: vector<CLod> with just one lod
	sint32 lodCount = 1;
	of.serial(lodCount);
	uint8 lodVer = 0;
	of.serial(lodVer);
	float lodDistance = 0.0f;
	of.serial(lodDistance);
	// ActiveBones: vector<uint8> of 0xFF
	sint32 activeCount = (sint32)bones.size();
	of.serial(activeCount);
	for (sint32 i = 0; i < activeCount; ++i)
	{
		uint8 active = 0xFF;
		of.serial(active);
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
// the Autodesk rename of BipSlave_Control → BipDriven_Control across the corpus's Max versions.
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
	if (isBiped)
	{
		if (!allowBipedDegraded)
		{
			std::cerr << maxFile << ": biped skeleton — bind-pose extraction not yet implemented.\n"
			          << "  The biped rig computes local transforms procedurally in figure mode from\n"
			          << "  the state stored in the root Biped (0x9155) controller chunks; we don't\n"
			          << "  yet decode them. Pass --allow-biped-degraded to write a .skel with\n"
			          << "  correct hierarchy but identity local transforms (unusable for skinning).\n"
			          << "  See pipeline_max_design §12 for the milestone plan.\n";
			return 3;
		}
		std::cerr << maxFile << ": WARNING biped detected, exporting with identity local transforms\n";
	}
	CScene scene(&reg, &dll, &cd);
	{ GsfInput *s = gsf_infile_child_by_name(in, "Scene"); CStorageStream st(s); scene.serial(st); g_object_unref(s); }
	scene.parse(VersionUnknown);

	INode *root = scene.container()->scene()->rootNode();
	INode *bip01 = root->find(ucstring("Bip01"));
	if (!bip01) { std::cerr << "Bip01 not found in " << maxFile << "\n"; return 2; }

	std::vector<Bone> bones;
	std::set<std::string> nameSet;
	if (g_useDouble)
	{
		Mat4D rootMat = Mat4D::identity();
		walkNodeD(bip01, -1, rootMat, scene.container(), bones, nameSet);
	}
	else
	{
		NLMISC::CMatrix rootMat;
		rootMat.identity();
		walkNode(bip01, -1, rootMat, scene.container(), bones, nameSet);
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
