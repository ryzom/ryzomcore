/**
 * \file anim_build.cpp
 * \brief See anim_build.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8
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

#include <nel/3d/animation.h>
#include <nel/3d/track_keyframer.h>
#include <nel/3d/animated_material.h>

#include "material_build.h"
#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/control_keyframer.h"
#include "../pipeline_max/builtin/mtl_base.h"
#include "../pipeline_max/builtin/multi_mtl.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/storage_object.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace SCENELIB;
using namespace MATBUILD;

namespace SHAPEANIM {

#define NEL3D_APPDATA_AUTOMATIC_ANIMATION 1423062617
#define NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS 1423062587

// NeL material v14 param table (subset shared with material_build.cpp): block index among the
// material's ParamBlock2 references, and param id.
#define NLB_MAIN 1
#define NLB_TEXTURES 2
#define NLP_BEXPORTTEXTUREMATRIX 0x24
#define NLP_BENABLESLOT_1 0
#define NLP_TTEXTURE_1 0x10
#define MAX_TEX_STAGE 8

// Max ticks per second (GetTicksPerFrame*GetFrameRate = 4800), and the Interval sentinels.
static const sint32 TIME_NEG_INF = (sint32)0x80000000;
static const sint32 TIME_POS_INF = (sint32)0x7fffffff;
static inline float convertTime(sint32 ticks) { return (float)ticks / 4800.0f; }

// The StdUVGen coordinate params, in ParamBlock index order, and the NeL texture-matrix track
// each one drives (the subset the reference addTexTracks reads). U/V Offset are the corpus-proven
// pair (§10k); tiling/angle are by the standard StdUVGen order, unexercised so far.
struct SUVCoord
{
	int Index;                        // StdUVGen ParamBlock param index
	const char *(*NameFn)(uint stage); // CAnimatedMaterial track-name suffix
};
static const SUVCoord s_uvCoords[] = {
	{ 0, &NL3D::CAnimatedMaterial::getTexMatUTransName }, // U Offset -> UTrans
	{ 1, &NL3D::CAnimatedMaterial::getTexMatVTransName }, // V Offset -> VTrans
	{ 2, &NL3D::CAnimatedMaterial::getTexMatUScaleName }, // U Tiling -> UScale
	{ 3, &NL3D::CAnimatedMaterial::getTexMatVScaleName }, // V Tiling -> VScale
	{ 6, &NL3D::CAnimatedMaterial::getTexMatWRotName },   // W Angle  -> WRot
};

// Resolve the Linear Float controller animating StdUVGen coordinate param `coord` on this texmap.
// texmap ref 0 = StdUVGen ("Placement"); StdUVGen ref 0 = the old-style ParamBlock (0x8). A coord
// param's 0x0002 entry replaces its value chunk with an empty 0x0200 marker when animated, and its
// controller is the ParamBlock's reference, in animated-entry order (compact — one ref per
// animated param). See --uvgen-dump / §10k.
// Find the first UVGen (superclass 0xc20) in the texmap's reference subtree — the texmap is a
// plain BitmapTex (ref 0 = StdUVGen directly) or a NeL Multi Bitmap (ref 0 = delegate BitmapTex
// whose ref 0 is the StdUVGen), matching getControlerByName's recursive subanim walk.
static CSceneClass *findUVGen(CSceneClass *obj, int depth)
{
	if (!obj) return NULL;
	if (obj->classDesc()->superClassId() == 0x00000c20) return obj;
	if (depth <= 0) return NULL;
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(obj);
	for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		if (CSceneClass *r = findUVGen(dynamic_cast<CSceneClass *>(rm->getReference(i)), depth - 1))
			return r;
	return NULL;
}

static CControlKeyFramerBase *uvController(CSceneClass *texmap, int coord)
{
	CSceneClass *uvgen = findUVGen(texmap, 3);
	CReferenceMaker *urm = dynamic_cast<CReferenceMaker *>(uvgen);
	if (!urm || urm->nbReferences() == 0) return NULL;
	CSceneClass *pblock = dynamic_cast<CSceneClass *>(urm->getReference(0));
	CStorageContainer *pc = dynamic_cast<CStorageContainer *>(pblock);
	CReferenceMaker *prm = dynamic_cast<CReferenceMaker *>(pblock);
	if (!pc || !prm) return NULL;

	int refSlot = 0;
	for (CStorageContainer::TStorageObjectConstIt it = pc->chunks().begin(); it != pc->chunks().end(); ++it)
	{
		if (it->first != 0x0002) continue;
		CStorageContainer *e = dynamic_cast<CStorageContainer *>(it->second);
		if (!e) continue;
		sint32 idx = -1;
		bool animated = false;
		for (CStorageContainer::TStorageObjectConstIt st = e->chunks().begin(); st != e->chunks().end(); ++st)
		{
			if (st->first == 0x0003)
			{
				CStorageRaw *rw = dynamic_cast<CStorageRaw *>(st->second);
				if (rw && rw->Value.size() == 4) memcpy(&idx, rw->Value.data(), 4);
			}
			else if (st->first == 0x0200)
				animated = true;
		}
		if (animated)
		{
			if (idx == coord) return dynamic_cast<CControlKeyFramerBase *>(prm->getReference(refSlot));
			++refSlot;
		}
	}
	return NULL;
}

// Build a CTrackKeyFramerLinearFloat from a Linear Float controller (buildATrack typeFloat path:
// value passthrough, time = ticks/4800, range from the controller's Interval).
static NL3D::ITrack *buildFloatTrack(CControlKeyFramerBase *kf)
{
	CControlFloatLinear *c = dynamic_cast<CControlFloatLinear *>(kf);
	if (!c || kf->keyCount() == 0) return NULL;
	NL3D::CTrackKeyFramerLinearFloat *track = new NL3D::CTrackKeyFramerLinearFloat();
	const CStorageLinFloatKey *keys = c->keys();
	float firstK = 0.0f, lastK = 0.0f;
	for (uint i = 0; i < kf->keyCount(); ++i)
	{
		float t = convertTime(keys[i].Time);
		if (i == 0) firstK = t;
		lastK = t;
		NL3D::CKeyFloat k;
		k.Value = keys[i].Val;
		track->addKey(k, t);
	}
	sint32 rs = 0, re = 0;
	bool hasRange = kf->range(rs, re);
	bool valid = hasRange;
	if (valid && rs == TIME_NEG_INF && re == TIME_POS_INF) valid = false; // FOREVER
	if (valid && rs == TIME_NEG_INF && re == TIME_NEG_INF) valid = false; // NEVER
	if (valid) track->unlockRange(convertTime(rs), convertTime(re));
	else track->unlockRange(firstK, lastK);
	track->setLoopMode(false);
	return track;
}

// addTexTracks: per texmap per stage, add the UTrans/VTrans/... tracks whose coord is animated.
static uint addTexTracks(NL3D::CAnimation &animation, CSceneClass *texmap, uint stage, const std::string &mtlName)
{
	uint n = 0;
	for (uint k = 0; k < sizeof(s_uvCoords) / sizeof(s_uvCoords[0]); ++k)
	{
		CControlKeyFramerBase *c = uvController(texmap, s_uvCoords[k].Index);
		if (!c) continue;
		NL3D::ITrack *track = buildFloatTrack(c);
		if (!track) continue;
		std::string name = mtlName + s_uvCoords[k].NameFn(stage);
		if (animation.getTrackByName(name.c_str())) { delete track; continue; } // first-wins, like the reference
		animation.addTrack(name.c_str(), track);
		++n;
	}
	return n;
}

// The first NeL material reachable from `mtl` (itself if it is one, else the first sub-material,
// recursively). The reference's getValueByNameUsingParamBlock2 / getControlerByName recurse a
// material's subanims and return the first match, so a Multi/Sub-Object resolves the texmat params
// (bExportTextureMatrix, tTexture_N) from its first sub-material — which is how a Multi gets a
// texture-matrix track under ITS OWN name (e.g. waterfall01's Multi 'Material #83' emits
// 'Material #83.VTrans0' driven by sub-material #26's controller).
static CSceneClass *firstNelMaterial(CSceneClass *mtl)
{
	if (!mtl) return NULL;
	if (mtl->classDesc()->classId() == CLASSID_NEL_MTL) return mtl;
	if (CMultiMtl *mm = dynamic_cast<CMultiMtl *>(mtl))
		for (uint s = 0; s < mm->numSubMaterials(); ++s)
			if (CSceneClass *r = firstNelMaterial(mm->subMaterial(s))) return r;
	return NULL;
}

// addMtlTracks: the texture-matrix half (color tracks unimplemented — no corpus signal, §12.2).
// Emits this material's own tracks (params resolved through its first reachable NeL material, per
// the reference recursion), then recurses Multi sub-materials with the SAME parent prefix (the
// reference passes the base name, not the multi's, to sub-materials).
static uint addMtlTracks(NL3D::CAnimation &animation, CSceneClass *mtl, const std::string &parentName)
{
	if (!mtl) return 0;
	uint n = 0;
	std::string mtlName = parentName + materialName(mtl) + ".";

	// The reference recurses sub-materials BEFORE emitting its own texmat tracks (addMtlTracks:
	// the GetSubMtl loop precedes the bExportTextureMatrix block), so the insertion order — which
	// CAnimation preserves as the track-data index behind the name-sorted map — is subs first,
	// then this material's own.
	if (CMultiMtl *mm = dynamic_cast<CMultiMtl *>(mtl))
		for (uint s = 0; s < mm->numSubMaterials(); ++s)
			n += addMtlTracks(animation, mm->subMaterial(s), parentName);

	CSceneClass *nel = firstNelMaterial(mtl);
	if (nel)
	{
		std::vector<SPB2Block> blocks;
		readObjectPB2Blocks(nel, blocks);
		// bExportTextureMatrix and bEnableSlot_N can be inline constants OR controller-backed (the
		// export flag is animated with an On/Off controller on some materials — the reference reads
		// the live value at t=0). Default false (the NeL material script default).
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

bool isAnimToBeExported(INode &node)
{
	return getScriptAppDataInt(&node, NEL3D_APPDATA_AUTOMATIC_ANIMATION, 0) != 0;
}

uint buildMaterialAnim(INode &node, NL3D::CAnimation &animation)
{
	if (getScriptAppDataInt(&node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0) == 0)
		return 0;
	// Base name is bare for the exported shape node (its parent is the scene root).
	return addMtlTracks(animation, materialOf(node), std::string());
}

} /* namespace SHAPEANIM */

/* end of file */
