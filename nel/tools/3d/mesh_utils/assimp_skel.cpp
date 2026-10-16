// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2026  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <nel/misc/types_nl.h>
#include "assimp_skel.h"

#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/vector.h>
#include <nel/misc/quat.h>
#include <nel/misc/matrix.h>
#include <nel/pipeline/tool_logger.h>

#include <nel/3d/shape.h>
#include <nel/3d/skeleton_shape.h>
#include <nel/3d/bone.h>

#include <assimp/scene.h>

#include <set>
#include <string>
#include <vector>
#include <cstring>

#define NL_NODE_INTERNAL_TYPE aiNode
#define NL_SCENE_INTERNAL_TYPE aiScene
#include "scene_context.h"

// Match the case-insensitive Bip01 lookup used by NeL's Max exporter (INode::find).
static bool nameEqualsCi(const char *a, const char *b)
{
	for (; *a && *b; ++a, ++b)
	{
		char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
		char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
		if (ca != cb) return false;
	}
	return !*a && !*b;
}

// Search the aiNode tree for the first node whose name matches (case-insensitive). Used to
// locate "Bip01" as the skeleton root when scene_meta doesn't declare one.
static const aiNode *findByNameCi(const aiNode *node, const char *target)
{
	if (node->mName.length && nameEqualsCi(node->mName.C_Str(), target)) return node;
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		const aiNode *r = findByNameCi(node->mChildren[i], target);
		if (r) return r;
	}
	return NULL;
}

// Locate the root of the bone hierarchy. Priority: (1) scene_meta node flagged TBoneRoot,
// (2) case-insensitive "Bip01" descendant of the scene root, (3) NULL (no skeleton to emit).
static const aiNode *findSkelRoot(CMeshUtilsContext &context)
{
	// (1) Explicit TBoneRoot
	for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
	{
		CNodeMeta &meta = context.SceneMeta.Nodes[it->first];
		if (meta.ExportBone == TBoneRoot && it->second.InternalNode)
			return it->second.InternalNode;
	}
	// (2) Bip01 by name (case-insensitive)
	const aiNode *bip = findByNameCi(context.InternalScene->mRootNode, "Bip01");
	if (bip) return bip;
	return NULL;
}

// Extract the basis vectors I, J, K and translation P from an aiMatrix4x4 as NLMISC::CVector.
// Assimp matrices are row-major and stored transpose to OpenGL, matching the Max/NeL convention
// where GetRow returns (a1i, a2i, a3i, a4i) — column vectors of the transform's basis.
static NLMISC::CVector aiColumnI(const aiMatrix4x4 &m) { return NLMISC::CVector(m.a1, m.b1, m.c1); }
static NLMISC::CVector aiColumnJ(const aiMatrix4x4 &m) { return NLMISC::CVector(m.a2, m.b2, m.c2); }
static NLMISC::CVector aiColumnK(const aiMatrix4x4 &m) { return NLMISC::CVector(m.a3, m.b3, m.c3); }
static NLMISC::CVector aiTrans(const aiMatrix4x4 &m)   { return NLMISC::CVector(m.a4, m.b4, m.c4); }

// Ordered children of a node (aiNode preserves mChildren order — no need for pointer-sorted set).
static void collectChildren(const aiNode *node, std::vector<const aiNode *> &out)
{
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		out.push_back(node->mChildren[i]);
}

// Look up a float-valued NeL extras key on an aiNode. Assimp's glTF loader stores JSON
// numbers as AI_DOUBLE (with-decimal) or AI_UINT64/INT32 (integer-looking); we accept
// any numeric type and downcast to float. Returns true iff the key was found.
// See wiki: nel_gltf_extras.md for the catalogue of nel_* keys.
static bool getNelFloatMeta(const aiNode *node, const char *key, float &out)
{
	if (!node->mMetaData) return false;
	for (unsigned i = 0; i < node->mMetaData->mNumProperties; ++i)
	{
		if (strcmp(node->mMetaData->mKeys[i].C_Str(), key) != 0) continue;
		const aiMetadataEntry &v = node->mMetaData->mValues[i];
		switch (v.mType)
		{
		case AI_DOUBLE: out = (float)*(const double *)v.mData; return true;
		case AI_FLOAT:  out =        *(const float *)v.mData;  return true;
		case AI_INT32:  out = (float)*(const int32_t *)v.mData; return true;
		case AI_UINT32: out = (float)*(const uint32_t *)v.mData; return true;
		case AI_UINT64: out = (float)*(const uint64_t *)v.mData; return true;
		case AI_INT64:  out = (float)*(const int64_t *)v.mData; return true;
		default: return false;
		}
	}
	return false;
}

static bool getNelBoolMeta(const aiNode *node, const char *key, bool &out)
{
	if (!node->mMetaData) return false;
	for (unsigned i = 0; i < node->mMetaData->mNumProperties; ++i)
	{
		if (strcmp(node->mMetaData->mKeys[i].C_Str(), key) != 0) continue;
		const aiMetadataEntry &v = node->mMetaData->mValues[i];
		if (v.mType == AI_BOOL) { out = *(const bool *)v.mData; return true; }
		return false;
	}
	return false;
}

// Recursively build CBoneBase entries. Mirrors pipeline_max_export_skel::walkNode:
// * Local T/R/S preferred from nel_* extras (bit-exact preservation from source); falls back
//   to decomposing aiNode.mTransformation if none present (e.g., artist-authored glTF).
// * Root bone (fatherId==-1) gets DefaultPos and DefaultRotQuat zeroed (NeL buildSkeleton
//   convention); DefaultScale is preserved.
// * InvBindPos = inverse of parentWorld * localTM, rebuilt via identity+setRot(I,J,K)+setPos(P)
//   to match Max exporter's state bits.
static void walkSkelNode(CMeshUtilsContext &context, const aiNode *node,
                         sint32 fatherId, const NLMISC::CMatrix &parentWorld,
                         std::vector<NL3D::CBoneBase> &bones, std::set<std::string> &nameSet)
{
	NL3D::CBoneBase b;

	std::string name = node->mName.C_Str();
	if (!nameSet.insert(name).second) name += "_Second";
	b.Name = name;
	b.FatherId = fatherId;
	b.UnheritScale = false; // Default for non-biped PRS controllers; matches pipeline_max_export_skel default.
	b.LodDisableDistance = 0.0f;

	// Prefer nel_tx/ty/tz + nel_rx/ry/rz/rw + nel_sx/sy/sz extras when present — they
	// preserve the source's float32 bits exactly and avoid the TRS->matrix->Decompose
	// roundtrip that costs 5-16% byte parity on the .skel output. Falls back to Decompose
	// for glTFs authored elsewhere (Blender export, etc.) that don't carry our extras.
	//
	// Settings.IgnoreNelExtras forces the Decompose path even when the extras are present —
	// used for validation, to confirm the fallback works and to measure its accuracy floor.
	NLMISC::CVector realPos, realScale;
	NLMISC::CQuat realRot;
	float tx, ty, tz, rx, ry, rz, rw, sx, sy, sz;
	bool haveTrans = false, haveRot = false, haveScl = false;
	if (!context.Settings.IgnoreNelExtras)
	{
		haveTrans = getNelFloatMeta(node, "nel_tx", tx) & getNelFloatMeta(node, "nel_ty", ty) & getNelFloatMeta(node, "nel_tz", tz);
		haveRot   = getNelFloatMeta(node, "nel_rx", rx) & getNelFloatMeta(node, "nel_ry", ry) & getNelFloatMeta(node, "nel_rz", rz) & getNelFloatMeta(node, "nel_rw", rw);
		haveScl   = getNelFloatMeta(node, "nel_sx", sx) & getNelFloatMeta(node, "nel_sy", sy) & getNelFloatMeta(node, "nel_sz", sz);
	}
	if (haveTrans && haveRot && haveScl)
	{
		realPos = NLMISC::CVector(tx, ty, tz);
		realRot = NLMISC::CQuat(rx, ry, rz, rw);
		realScale = NLMISC::CVector(sx, sy, sz);
	}
	else
	{
		aiVector3D aiScl, aiPos;
		aiQuaternion aiRot;
		node->mTransformation.Decompose(aiScl, aiRot, aiPos);
		realPos = NLMISC::CVector(aiPos.x, aiPos.y, aiPos.z);
		realRot = NLMISC::CQuat(aiRot.x, aiRot.y, aiRot.z, aiRot.w);
		realScale = NLMISC::CVector(aiScl.x, aiScl.y, aiScl.z);
	}
	realRot.normalize();

	// Read NeL-specific per-bone flags from extras when present (also gated by IgnoreNelExtras).
	if (!context.Settings.IgnoreNelExtras)
	{
		bool metaUnherit;
		if (getNelBoolMeta(node, "nel_unheritScale", metaUnherit)) b.UnheritScale = metaUnherit;
		float metaLod;
		if (getNelFloatMeta(node, "nel_lodDisableDistance", metaLod)) b.LodDisableDistance = metaLod;
	}

	// Build local matrix and accumulate world (float, matching pipeline_max_export_skel default).
	NLMISC::CMatrix localTM;
	localTM.identity();
	localTM.setPos(realPos);
	localTM.setRot(realRot);
	localTM.scale(realScale);
	NLMISC::CMatrix worldTM = parentWorld * localTM;

	if (fatherId < 0)
	{
		b.DefaultPos.setDefaultValue(NLMISC::CVector::Null);
		b.DefaultRotQuat.setDefaultValue(NLMISC::CQuat::Identity);
	}
	else
	{
		b.DefaultPos.setDefaultValue(realPos);
		b.DefaultRotQuat.setDefaultValue(realRot);
	}
	b.DefaultRotEuler.setDefaultValue(NLMISC::CVector::Null);
	b.DefaultScale.setDefaultValue(realScale);
	b.DefaultPivot.setDefaultValue(NLMISC::CVector::Null);
	b.SkinScale = NLMISC::CVector(1, 1, 1);

	// InvBindPos via convertMatrix state bits (identity + setRot(I,J,K) + setPos(P), then invert).
	NLMISC::CMatrix invBind;
	invBind.identity();
	invBind.setRot(worldTM.getI(), worldTM.getJ(), worldTM.getK());
	invBind.setPos(worldTM.getPos());
	invBind.invert();
	b.InvBindPos = invBind;

	sint32 myId = (sint32)bones.size();
	bones.push_back(b);

	std::vector<const aiNode *> kids;
	collectChildren(node, kids);
	for (const aiNode *child : kids)
		walkSkelNode(context, child, myId, worldTM, bones, nameSet);
}

// Derive the output .skel filename from the source file's base name (matches how the Max
// exporter names its output — <sceneName>.skel).
static std::string skelOutputName(const std::string &sourceFilePath)
{
	std::string base = NLMISC::CFile::getFilenameWithoutExtension(sourceFilePath);
	return base + ".skel";
}

void exportSkels(CMeshUtilsContext &context)
{
	const aiNode *skelRoot = findSkelRoot(context);
	if (!skelRoot)
		return; // No skeleton in this scene — that's fine.

	std::vector<NL3D::CBoneBase> bones;
	std::set<std::string> nameSet;
	NLMISC::CMatrix parentWorld;
	parentWorld.identity();
	walkSkelNode(context, skelRoot, -1, parentWorld, bones, nameSet);

	if (bones.empty()) return;

	NL3D::CSkeletonShape shape;
	shape.build(bones);

	std::string skelPath = NLMISC::CPath::standardizePath(context.Settings.DestinationDirectoryPath, true)
		+ skelOutputName(context.Settings.SourceFilePath);
	context.ToolLogger.writeDepend(NLPIPELINE::BUILD, skelPath.c_str(), "*");

	NLMISC::COFile f;
	if (f.open(skelPath, false, false, true))
	{
		try
		{
			NL3D::CShapeStream stream(&shape);
			stream.serial(f);
			f.close();
		}
		catch (...)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Skeleton '%s' serialization failed", skelPath.c_str());
		}
	}
	else
	{
		tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
			"Unable to open '%s' for writing", skelPath.c_str());
	}
}

/* end of file */
