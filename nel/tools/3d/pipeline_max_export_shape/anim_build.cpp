/**
 * \file anim_build.cpp
 * \brief See anim_build.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
 * \author Grok 4.5
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

#include <nel/misc/types_nl.h>
#include "anim_build.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include <nel/3d/animation.h>
#include <nel/3d/animated_material.h>
#include <nel/3d/transformable.h>
#include <nel/misc/algo.h>
#include <nel/misc/ucstring.h>

#include "material_build.h"
#include "scene_lib.h"
#include "../pipeline_max_export_common/track_build.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/control_transform.h"
#include "../pipeline_max/builtin/mtl_base.h"
#include "../pipeline_max/builtin/multi_mtl.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/param_block.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/builtin/derived_object.h"
#include "../pipeline_max/storage_object.h"

#include "../pipeline_max_export_common/export_ids.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace SCENELIB;
using namespace MATBUILD;
using namespace TRACKBUILD;

namespace SHAPEANIM {

// NeL AppData sub-ids
// Lightmap animated light (calc_lm.h): NEL3D_APPDATA_LM = 41654684

// NeL material v14 param ids (subset)
#define NLB_MAIN 1
#define NLB_TEXTURES 2
#define NLP_BEXPORTTEXTUREMATRIX 0x24
#define NLP_BENABLESLOT_1 0
#define NLP_TTEXTURE_1 0x10
#define MAX_TEX_STAGE 8

// Class ids (PRS/LookAt are the typed CControlPRS/CControlLookAt since §10j-dix)
static const NLMISC::CClassId CLASSID_MORPHER(0x17bb6854, 0xa5cba2a3);
// Light superclasses / Omni class for light detection
static const TSClassId SCLASS_LIGHT = 0x00000030;

// ---------------------------------------------------------------------------------------------
// AppData helpers

static std::string getAppDataStr(INode &node, uint32 subId, const std::string &def = std::string())
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return def;
	std::string s = getScriptAppDataStr(n, subId, def);
	return s;
}

static int getAppDataInt(INode &node, uint32 subId, int def)
{
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);
	if (!n) return def;
	return getScriptAppDataInt(n, subId, def);
}

bool isAnimToBeExported(INode &node)
{
	// shape_export.ms isAnimToBeExported: AUTOMATIC_ANIMATION must be set and non-"0",
	// and the node must not be DONOTEXPORT / COLLISION / COLLISION_EXTERIOR.
	if (getAppDataInt(node, NEL3D_APPDATA_AUTOMATIC_ANIMATION, 0) == 0)
		return false;
	if (getAppDataStr(node, NEL3D_APPDATA_DONOTEXPORT, "") == "1")
		return false;
	if (getAppDataStr(node, NEL3D_APPDATA_COLLISION, "") == "1")
		return false;
	if (getAppDataStr(node, NEL3D_APPDATA_COLLISION_EXTERIOR, "") == "1")
		return false;
	return true;
}

// ---------------------------------------------------------------------------------------------
// Node transform tracks (addNodeTracks for a single selected root — bare track names)

static uint addNodeTracks(NL3D::CAnimation &animation, INode &node)
{
	uint n = 0;
	CReferenceMaker *transform = node.getReference(0);
	CControlPRS *prs = dynamic_cast<CControlPRS *>(transform);
	CControlLookAt *lookAt = dynamic_cast<CControlLookAt *>(transform);
	if (!prs && !lookAt) return 0;

	// Export order matches the reference: scale, rotation, position (typed sub-controller slots
	// on CControlPRS/CControlLookAt — §10j-dix).
	NL3D::ITrack *track = buildATrack(prs ? prs->scaleController() : lookAt->scaleController(), typeScale);
	if (track) { addTrackChecked(animation, NL3D::ITransformable::getScaleValueName(), track); ++n; }

	if (prs)
	{
		track = buildATrack(prs->rotationController(), typeRotation);
		if (track) { addTrackChecked(animation, NL3D::ITransformable::getRotQuatValueName(), track); ++n; }
	}

	track = buildATrack(prs ? prs->positionController() : lookAt->positionController(), typePos);
	if (track) { addTrackChecked(animation, NL3D::ITransformable::getPosValueName(), track); ++n; }

	return n;
}

// ---------------------------------------------------------------------------------------------
// Material texture-matrix tracks (§10k)

struct SUVCoord
{
	int Index;
	const char *(*NameFn)(uint stage);
};
static const SUVCoord s_uvCoords[] = {
	{ 0, &NL3D::CAnimatedMaterial::getTexMatUTransName },
	{ 1, &NL3D::CAnimatedMaterial::getTexMatVTransName },
	{ 2, &NL3D::CAnimatedMaterial::getTexMatUScaleName },
	{ 3, &NL3D::CAnimatedMaterial::getTexMatVScaleName },
	{ 6, &NL3D::CAnimatedMaterial::getTexMatWRotName },
};

static CSceneClass *findUVGen(CSceneClass *obj, int depth)
{
	if (!obj) return nullptr;
	if (obj->classDesc()->superClassId() == 0x00000c20) return obj;
	if (depth <= 0) return nullptr;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		if (CSceneClass *r = findUVGen(dynamic_cast<CSceneClass *>(rm->getReference(i)), depth - 1))
			return r;
	return nullptr;
}

static CControlKeyFramerBase *uvController(CSceneClass *texmap, int coord)
{
	// The StdUVGen coord params live on the UVGen's reference 0 = an old ParamBlock; an ANIMATED
	// param's controller occupies the block's compact reference slots in entry order — the typed
	// CParamBlock decodes that mapping (§10k; formerly an inline 0x0002 chunk walk here).
	CSceneClass *uvgen = findUVGen(texmap, 3);
	CReferenceMaker *urm = dynamic_cast<CReferenceMaker *>(uvgen);
	if (!urm || urm->nbReferences() == 0) return nullptr;
	CParamBlock *pblock = dynamic_cast<CParamBlock *>(urm->getReference(0));
	if (!pblock) return nullptr;
	return dynamic_cast<CControlKeyFramerBase *>(pblock->controllerForParam(coord));
}

static uint addTexTracks(NL3D::CAnimation &animation, CSceneClass *texmap, uint stage, const std::string &mtlName)
{
	uint n = 0;
	for (uint k = 0; k < sizeof(s_uvCoords) / sizeof(s_uvCoords[0]); ++k)
	{
		CControlKeyFramerBase *c = uvController(texmap, s_uvCoords[k].Index);
		if (!c) continue;
		NL3D::ITrack *track = buildATrack(c, typeFloat);
		if (!track) continue;
		std::string name = mtlName + s_uvCoords[k].NameFn(stage);
		if (animation.getTrackByName(name.c_str())) { delete track; continue; }
		animation.addTrack(name.c_str(), track);
		++n;
	}
	return n;
}

static CSceneClass *firstNelMaterial(CSceneClass *mtl)
{
	if (!mtl) return nullptr;
	if (mtl->classDesc()->classId() == CLASSID_NEL_MTL) return mtl;
	if (CMultiMtl *mm = dynamic_cast<CMultiMtl *>(mtl))
		for (uint s = 0; s < mm->numSubMaterials(); ++s)
			if (CSceneClass *r = firstNelMaterial(mm->subMaterial(s))) return r;
	return nullptr;
}

static uint addMtlTracks(NL3D::CAnimation &animation, CSceneClass *mtl, const std::string &parentName)
{
	if (!mtl) return 0;
	uint n = 0;
	std::string mtlName = parentName + materialName(mtl) + ".";

	// Reference: recurse sub-materials BEFORE own texmat tracks.
	if (CMultiMtl *mm = dynamic_cast<CMultiMtl *>(mtl))
		for (uint s = 0; s < mm->numSubMaterials(); ++s)
			n += addMtlTracks(animation, mm->subMaterial(s), parentName);

	CSceneClass *nel = firstNelMaterial(mtl);
	if (nel)
	{
		std::vector<SPB2Block> blocks;
		readObjectPB2Blocks(nel, blocks);
		if (resolveNelBoolAt0(blocks, NLB_MAIN, NLP_BEXPORTTEXTUREMATRIX, false))
		{
			for (uint i = 0; i < MAX_TEX_STAGE; ++i)
			{
				if (!resolveNelBoolAt0(blocks, NLB_TEXTURES, (uint16)(NLP_BENABLESLOT_1 + i), false)) continue;
				const SPB2Param *tex = findPB2Param(blocks, NLB_TEXTURES, (uint16)(NLP_TTEXTURE_1 + i));
				if (!tex) continue;
				CSceneClass *texmap = pb2RefValue(blocks[NLB_TEXTURES], *tex);
				if (texmap) n += addTexTracks(animation, texmap, i, mtlName);
			}
		}
	}
	return n;
}

// ---------------------------------------------------------------------------------------------
// Light tracks — LightmapController.<animatedLightName> from Color Point3 controller
// (CExportNel::addLightTracks). Only when NEL3D_APPDATA_LM_ANIMATED != 0 and the node object
// is a light (superclass 0x30).

static bool nodeIsLight(INode &node)
{
	CReferenceMaker *obj = dynamic_cast<CReferenceMaker *>(node.getReference(1));
	return obj && obj->classDesc()->superClassId() == SCLASS_LIGHT;
}

// Locate a Color / Point3 keyframer under the light object (getControlerByName(node,"Color")).
// Walks the node + object reference tree depth-first; prefers Bezier Color / Point3 / Position
// controllers that carry keys (the corpus light anims are all Bezier Color / Point3).
// Color controllers live on the light OBJECT (ParamBlock ref), not on the PRS transform.
// Prefer Point3/Color keyframers; never return a position-superclass controller (those are
// the node TM channels — lanterne-int1 wrongly matched ControlPosBezier when we walked the
// whole node first).
static CReferenceMaker *findColorController(CReferenceMaker *obj, int depth, std::set<CReferenceMaker *> &seen)
{
	if (!obj || depth < 0) return nullptr;
	if (!seen.insert(obj).second) return nullptr;

	if (CControlKeyFramerBase *kf = dynamic_cast<CControlKeyFramerBase *>(obj))
	{
		if (kf->keyCount() > 0)
		{
			if (dynamic_cast<CControlColorBezier *>(obj)
			    || dynamic_cast<CControlPoint3Bezier *>(obj)
			    || dynamic_cast<CControlPoint3TCB *>(obj))
				return obj;
		}
	}

	for (uint i = 0; i < obj->nbReferences(); ++i)
	{
		CReferenceMaker *r = obj->getReference(i);
		if (CReferenceMaker *found = findColorController(r, depth - 1, seen))
			return found;
	}
	return nullptr;
}

static std::string getAnimatedLightName(INode &node)
{
	std::string ret = getAppDataStr(node, NEL3D_APPDATA_LM_ANIMATED_LIGHT, "");
	if (ret == "Sun" || ret == "GlobalLight" || ret == "(Use NelLight Modifier)")
		ret.clear();
	return ret;
}

static uint addLightTracks(NL3D::CAnimation &animation, INode &node)
{
	if (!nodeIsLight(node)) return 0;
	if (getAppDataInt(node, NEL3D_APPDATA_LM_ANIMATED, 0) == 0) return 0;

	std::string name = "LightmapController." + getAnimatedLightName(node);

	// Search Color controller under the light object only (not the whole node — the PRS
	// sub-controllers are also Point3-layout keyframers and would false-match).
	CReferenceMaker *lightObj = dynamic_cast<CReferenceMaker *>(node.getReference(1));
	std::set<CReferenceMaker *> seen;
	CReferenceMaker *ctrl = findColorController(lightObj, 8, seen);
	if (!ctrl) return 0;

	NL3D::ITrack *track = buildATrack(ctrl, typeColor);
	if (!track) return 0;
	if (animation.getTrackByName(name.c_str()))
	{
		delete track;
		return 0;
	}
	animation.addTrack(name.c_str(), track);
	return 1;
}

// ---------------------------------------------------------------------------------------------
// Morph tracks

static uint addMorphTracks(NL3D::CAnimation &animation, INode &node)
{
	CDerivedObject *obj = dynamic_cast<CDerivedObject *>(node.getReference(1));
	if (!obj) return 0;
	CReferenceMaker *morpher = nullptr;
	for (uint i = 0; i < obj->modifierCount() && !morpher; ++i)
	{
		CSceneClass *mod = obj->modifier(i);
		if (mod && mod->classDesc()->classId() == CLASSID_MORPHER)
			morpher = dynamic_cast<CReferenceMaker *>(mod);
	}
	if (!morpher) return 0;

	uint n = 0;
	for (uint i = 0; i < 100; ++i)
	{
		CNodeImpl *target = dynamic_cast<CNodeImpl *>(morpher->getReference(101 + i));
		if (!target) continue;
		CReferenceMaker *pblock = morpher->getReference(i + 1);
		if (!pblock) continue;
		NL3D::ITrack *track = buildATrack(pblock->getReference(0), typeFloat);
		if (!track) continue;
		std::string name = ucstring(target->userName()).toUtf8() + "MorphFactor";
		if (animation.getTrackByName(name.c_str())) { delete track; continue; }
		animation.addTrack(name.c_str(), track);
		++n;
	}
	return n;
}

// ---------------------------------------------------------------------------------------------

uint buildNodeAnim(INode &node, NL3D::CAnimation &animation)
{
	// Replicates CExportNel::addAnimation for a single non-biped selected node
	// (shape process: NelExportAnimation #(node) out false).
	// Order: node tracks, object (FOV skipped — no corpus keys), materials, lights, PS
	// (skipped — no shape-process corpus), morph. Bare track names (selected root).
	uint n = 0;
	n += addNodeTracks(animation, node);

	if (getAppDataInt(node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) != 0)
		n += addMtlTracks(animation, materialOf(node), std::string());

	n += addLightTracks(animation, node);
	n += addMorphTracks(animation, node);
	return n;
}

} /* namespace SHAPEANIM */

/* end of file */
