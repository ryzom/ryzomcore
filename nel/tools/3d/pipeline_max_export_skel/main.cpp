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

#include <cmath>

#include <gsf/gsf-infile-msole.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-utils.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <set>

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
	NLMISC::CVector DefaultPos;
	NLMISC::CQuat DefaultRotQuat;
	NLMISC::CVector DefaultScale;
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
	b.UnheritScale = true;
	b.LodDisableDistance = 0.0f;

	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	CReferenceMaker *tmCtrl = node->getReference(0);
	getLocalTransform(tmCtrl, realPos, realRot, realScale);
	realRot.normalize();

	NLMISC::CMatrix localTM = makeLocalTM(realPos, realRot, realScale);
	NLMISC::CMatrix worldTM = parentWorld * localTM;

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
	b.UnheritScale = true;
	b.LodDisableDistance = 0.0f;

	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	CReferenceMaker *tmCtrl = node->getReference(0);
	getLocalTransform(tmCtrl, realPos, realRot, realScale);
	realRot.normalize();

	Mat4D localTM = Mat4D::fromTRS(realPos, realRot, realScale);
	Mat4D worldTM = parentWorld * localTM;

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

	// _BoneMap: std::map<string, uint32>
	uint32 mapCount = (uint32)bones.size();
	of.serial(mapCount);
	for (uint32 i = 0; i < bones.size(); ++i)
	{
		std::string n = bones[i].Name;
		of.serial(n);
		of.serial(i);
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

int main(int argc, char **argv)
{
	// Parse args. --double before the positional args switches to double-precision accumulation.
	int argi = 1;
	if (argi < argc && std::string(argv[argi]) == "--double") { g_useDouble = true; ++argi; }
	if (argc - argi < 2) { std::cerr << "usage: export_skel [--double] <input.max> <output.skel>\n"; return 1; }
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

	g_object_unref(in);
	g_object_unref(src);
	gsf_shutdown();
	return 0;
}
