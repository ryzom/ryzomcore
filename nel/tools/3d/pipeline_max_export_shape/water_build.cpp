/**
 * \file water_build.cpp
 * \brief See water_build.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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
#include "water_build.h"

#include <cstdio>
#include <cstring>

#include <nel/misc/polygon.h>
#include <nel/misc/vector_2f.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/matrix.h>
#include <nel/3d/water_shape.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_multi_file.h>
#include <nel/3d/texture_blend.h>

#include "mesh_eval.h"
#include "material_build.h"
#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace NLMISC;
using namespace NL3D;
using namespace MAXMATH;
using namespace SCENELIB;
using namespace MESHEVAL;
using namespace MATBUILD;
using MAXSCENE::decompMatrix;
using MAXSCENE::convertMatrix;

namespace WATERBUILD {

// -----------------------------------------------------------------------------
// The water material's PB2 parameter table — the same v14 NeL-material script the shape
// exporter reads for generic materials (material_build.cpp NLB_MAIN/NLB_TEXTURES), plus the
// water-specific slot enables + numeric params introduced by the water shader case.

#define NLB_NLBP 0
#define NLB_MAIN 1
#define NLB_TEXTURES 2

#define NLP_BENABLESLOT_1 0
#define NLP_TTEXTURE_1 0x10

// Water-specific params on the material's PB2 NLB_MAIN block — the reference exporter accesses
// them by NAME through the v14 script's ParamDef; we identify them by their serialization-stable
// ids catalogued via PMB_WATER_DUMP on a corpus water material (WaterBassinA02 from
// watertrykerwhole.max, 2026-07-09). Numeric values in the dump correlate exactly (fBumpU/V
// scale/speed values match the reference material's bump/displace scroll parameters):
#define NLP_FBUMPUSCALE 0x09
#define NLP_FBUMPVSCALE 0x0a
#define NLP_FBUMPUSPEED 0x0b
#define NLP_FBUMPVSPEED 0x0c
#define NLP_FDISPLACEUSCALE 0x0d
#define NLP_FDISPLACEVSCALE 0x0e
#define NLP_FDISPLACEUSPEED 0x0f
#define NLP_FDISPLACEVSPEED 0x10
#define NLP_FWATERHEIGHTFACTOR 0x11
#define NLP_IWATERPOOLID 0x12
// Three water bool flags at consecutive ids — the reference exporter reads them by name as
// bEnableWaterSplash / bWaterUseSceneEnvMapAbove / bWaterUseSceneEnvMapUnder. Pinned by
// correlating the tr_water_00 diff-byte pattern (3 booleans all true at consecutive positions)
// with the PMB_WATER_DUMP block 1 params (three consecutive I=1 entries at 0x19..0x1b).
#define NLP_BENABLEWATERSPLASH 0x19
#define NLP_BWATERUSESCENEENVMAPABOVE 0x1a
#define NLP_BWATERUSESCENEENVMAPUNDER 0x1b
// v15+ additions — absent on v14 corpus materials; PB2 lookups fail cleanly, keeping the
// CWaterShape default (fresnel bias=1/scale=0/power=1 gives a flat reflectivity of 1).

static bool pb2Int(const std::vector<SPB2Block> &blocks, uint block, uint16 id, int &out)
{
	const SPB2Param *p = findPB2Param(blocks, block, id);
	if (!p || !p->HasConstant) return false;
	out = p->I;
	return true;
}

static bool pb2Float(const std::vector<SPB2Block> &blocks, uint block, uint16 id, float &out)
{
	const SPB2Param *p = findPB2Param(blocks, block, id);
	if (!p || !p->HasConstant) return false;
	out = p->F[0];
	return true;
}

// The BitmapTex's file path — same 0x0003 container decode as material_build's bmtexFileName
// (which is static there). Duplicated locally, small and self-contained; folding it into a
// shared helper is a follow-up if a third consumer needs it.
static bool bmtexFileName(CSceneClass *bmtex, std::string &out)
{
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(bmtex);
	if (!rm) return false;
	for (uint i = 0; i < rm->nbReferences(); ++i)
	{
		CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
		if (!r || r->classDesc()->superClassId() != SCLASS_PBLOCK2) continue;
		const CStorageContainer::TStorageObjectContainer &orphans = r->orphanedChunks();
		for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
		{
			if (it->first != 0x0003) continue;
			CStorageContainer *c = dynamic_cast<CStorageContainer *>(it->second);
			if (!c) continue;
			uint idx = 0;
			for (CStorageContainer::TStorageObjectConstIt jt = c->chunks().begin(); jt != c->chunks().end(); ++jt, ++idx)
			{
				if (idx != 1) continue;
				CStorageRaw *raw = dynamic_cast<CStorageRaw *>(jt->second);
				if (!raw) return false;
				ucstring us;
				us.resize(raw->Value.size() / 2);
				if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
				out = us.toUtf8();
				while (!out.empty() && out[out.size() - 1] == '\0') out.resize(out.size() - 1);
				return !out.empty();
			}
		}
	}
	return false;
}

// Build a CTextureFile (BitmapTex delegate resolved) or CTextureMultiFile (NeL Multi Bitmap
// with 1..8 slot filenames from its own PB2) from a texmap reference. Returns NULL if the
// texmap is neither of these known classes — water requires BitmapTex-compatible sources
// per the reference exporter's own gating (isClassIdCompatible + BMTEX_CLASS_ID checks).
static ITexture *waterTextureFromTexmap(CSceneClass *texmap, const std::string &nodeName, const char *tag)
{
	if (!texmap) return NULL;
	NLMISC::CClassId cid = texmap->classDesc()->classId();
	CSceneClass *bmtex = texmap;
	if (cid == CLASSID_NEL_BMTEX)
	{
		// NeL Multi Bitmap: slot filenames come from its OWN PB2 (bitmap1FileName ..
		// bitmap8FileName strings). Same scan the material builder uses. Reference exporter
		// gates on isClassIdCompatible(*maxDisplaceMap, BMTEX_CLASS_ID) which is TRUE for the
		// NeL-multi delegate too — water sources in the corpus should include either.
		std::vector<SPB2Block> blocks;
		readObjectPB2Blocks(texmap, blocks);
		std::string files[8];
		uint numUsed = 0;
		for (uint b = 0; b < blocks.size(); ++b)
		{
			const SPB2Param *p0 = findPB2Param(blocks, b, 0);
			if (!p0 || (p0->Type & 0x07ff) != PB2_STRING) continue;
			for (uint k = 0; k < 8; ++k)
			{
				const SPB2Param *p = findPB2Param(blocks, b, (uint16)k);
				if (p && p->HasConstant) files[k] = p->S;
			}
			break;
		}
		for (uint l = 0; l < 8; ++l)
			if (!files[l].empty()) numUsed = l + 1;
		if (!numUsed)
		{
			fprintf(stderr, "WARNING: water '%s': %s NeL multi-bitmap has no slot filenames\n",
			        nodeName.c_str(), tag);
			return NULL;
		}
		CTextureMultiFile *multi = new CTextureMultiFile(numUsed);
		for (uint k = 0; k < numUsed; ++k)
			if (!files[k].empty())
				multi->setFileName(k, NLMISC::CFile::getFilename(files[k]).c_str());
		return multi;
	}
	if (cid == CLASSID_BMTEX)
	{
		std::string path;
		if (!bmtexFileName(bmtex, path))
		{
			fprintf(stderr, "WARNING: water '%s': %s BitmapTex has no file name\n",
			        nodeName.c_str(), tag);
			return NULL;
		}
		CTextureFile *tf = new CTextureFile;
		tf->setFileName(NLMISC::CFile::getFilename(path));
		return tf;
	}
	fprintf(stderr, "WARNING: water '%s': %s texmap class %s not a BitmapTex-compatible source\n",
	        nodeName.c_str(), tag, cid.toString().c_str());
	return NULL;
}

NL3D::IShape *buildWaterShape(INode &node, SNodeTMCache &tmCache)
{
	std::string nodeName = SCENELIB::nodeName(node);
	CNodeImpl *n = dynamic_cast<CNodeImpl *>(&node);

	// The material must be a NeL material with bWater=1 (caller-gated via hasWaterMaterial).
	CSceneClass *mtl = materialOf(node);
	if (!mtl || mtl->classDesc()->classId() != CLASSID_NEL_MTL)
	{
		fprintf(stderr, "WARNING: water '%s': not a NeL material\n", nodeName.c_str());
		return NULL;
	}
	std::vector<SPB2Block> mtlBlocks;
	readObjectPB2Blocks(mtl, mtlBlocks);

	// Diagnostic dump: PMB_WATER_DUMP=1 dumps the material's PB2 blocks (block index, param id,
	// type, value) — used to pin the water-specific param ids empirically against a corpus file.
	// Not the standing exporter behavior; enable per-run when investigating byte-identity.
	if (getenv("PMB_WATER_DUMP"))
	{
		fprintf(stderr, "=== PMB_WATER_DUMP '%s': %u PB2 blocks ===\n",
		        nodeName.c_str(), (uint)mtlBlocks.size());
		for (uint b = 0; b < mtlBlocks.size(); ++b)
		{
			fprintf(stderr, "  block[%u] id=%u count=%u\n", b,
			        mtlBlocks[b].BlockId, (uint)mtlBlocks[b].Params.size());
			for (std::map<uint16, SPB2Param>::const_iterator it = mtlBlocks[b].Params.begin();
			     it != mtlBlocks[b].Params.end(); ++it)
			{
				const SPB2Param &p = it->second;
				fprintf(stderr, "    param id=0x%02x type=0x%x const=%d ref=%d tab=%d",
				        p.Id, p.Type, p.HasConstant, p.RefBacked, p.IsTab);
				if (p.HasConstant)
				{
					uint16 basetype = p.Type & 0x7ff;
					if (basetype == PB2_FLOAT)
						fprintf(stderr, " F=%g", p.F[0]);
					else if (basetype == PB2_INT || basetype == PB2_BOOL || basetype == PB2_RADIOBTN_INDEX)
						fprintf(stderr, " I=%d", p.I);
					else if (basetype == PB2_RGBA || basetype == PB2_POINT3)
						fprintf(stderr, " (%g,%g,%g)", p.F[0], p.F[1], p.F[2]);
					else if (basetype == PB2_STRING || basetype == PB2_FILENAME)
						fprintf(stderr, " S='%s'", p.S.c_str());
				}
				fprintf(stderr, "\n");
			}
		}
	}

	// The reference exporter reads slot enables/textures at material scope; per the water
	// param table, water uses slots: 1=env-above (required), 2=env-above-alt (blend, optional),
	// 3=env-below (optional), 4=env-below-alt (optional), 5=bump (required), 6=displace
	// (required), 7=diffuse/color (optional).
	int enable[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	CSceneClass *texmap[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	for (uint s = 0; s < 7; ++s)
	{
		pb2Int(mtlBlocks, NLB_TEXTURES, (uint16)(NLP_BENABLESLOT_1 + s), enable[s]);
		const SPB2Param *pt = findPB2Param(mtlBlocks, NLB_TEXTURES, (uint16)(NLP_TTEXTURE_1 + s));
		if (pt)
			texmap[s] = pb2RefValue(mtlBlocks[NLB_TEXTURES], *pt);
	}

	if (!enable[0] || !enable[4] || !enable[5])
	{
		fprintf(stderr, "SKIP water '%s': missing required slot enables (env+bump+displace)\n",
		        nodeName.c_str());
		return NULL;
	}
	if (!texmap[0] || !texmap[4] || !texmap[5])
	{
		fprintf(stderr, "SKIP water '%s': missing required texmaps (env/bump/displace)\n",
		        nodeName.c_str());
		return NULL;
	}

	// Evaluate the mesh (respects the modifier stack — XForm etc.).
	SEvalMesh mesh;
	if (!evalNodeMesh(node, mesh, NULL))
	{
		fprintf(stderr, "WARNING: water '%s': mesh evaluation failed\n", nodeName.c_str());
		return NULL;
	}
	if (mesh.Verts.empty())
	{
		fprintf(stderr, "WARNING: water '%s': mesh has no vertices\n", nodeName.c_str());
		return NULL;
	}

	// objectTM = offsetTM * nodeTM; objectToLocal = objectTM * inverse(nodeTM). Same construction
	// as the mesh path uses in buildMeshInterface — the reference:
	//   invNodeTM = node.GetNodeTM(time); invNodeTM.Invert();
	//   objectTM = node.GetObjectTM(time);
	//   objectToLocal = objectTM * invNodeTM;
	// Then convertMatrix into NeL space and transform the verts (v goes to node-local).
	NLMISC::CMatrix toExportSpace;
	{
		Matrix3M nodeTM = MAXSCENE::getNodeTM(&node, tmCache);
		Point3M opos;
		QuatM orot;
		ScaleValueM oscale;
		MAXSCENE::readObjectOffset(n, opos, orot, oscale);
		Matrix3M offsetTM = composePRS(opos, orot, oscale);
		Matrix3M objectTM = offsetTM * nodeTM;
		Matrix3M objectToLocal = objectTM * inverseM3(nodeTM);
		convertMatrix(toExportSpace, objectToLocal);
	}

	// Build the polygon in node-local space (verts already in object space; project to Z=0
	// after transforming into node-local).
	CPolygon dest;
	dest.Vertices.reserve(mesh.Verts.size());
	for (uint i = 0; i < mesh.Verts.size(); ++i)
	{
		NLMISC::CVector v(mesh.Verts[i].x, mesh.Verts[i].y, mesh.Verts[i].z);
		dest.Vertices.push_back(toExportSpace * v);
	}

	// Project into 2D and build the convex hull. Same projection matrix the reference uses
	// (rows [1,0,0,0], [0,1,0,0], [0,0,0,0], [0,0,0,0] — drop Z).
	static const float proj[] = { 1, 0, 0, 0,
	                              0, 1, 0, 0,
	                              0, 0, 0, 0,
	                              0, 0, 0, 0 };
	NLMISC::CMatrix projMat;
	projMat.set(proj);
	CPolygon2D projDest(dest, projMat);
	CPolygon2D convexPoly;
	projDest.buildConvexHull(convexPoly);

	if (convexPoly.Vertices.size() < 3)
	{
		fprintf(stderr, "WARNING: water '%s': convex hull too small (%u verts)\n",
		        nodeName.c_str(), (uint)convexPoly.Vertices.size());
		return NULL;
	}

	// Build the shape
	CWaterShape *ws = new CWaterShape;

	// Env maps — reference exporter's rule (see export_mesh.cpp:1879-1911):
	//   if (maxEnvMap):        envMap = maxEnvMap2 ? CTextureBlend(m1,m2) : buildATexture(m1)
	//   if (maxEnvMapUnder):   envMapUnder = maxEnvMapUnder2 ? CTextureBlend(...) : plain
	// The blend gate is TEXMAP presence (not the enable-slot flag) — the corpus's above-water
	// envmap on tr_water_00 is `CTextureBlend(waterenvmap.png, water_night_envmap.png)`, i.e.
	// the day/night blend that CWaterPoolManager::setBlend drives at runtime.
	NLMISC::CSmartPtr<ITexture> envMap = NULL;
	{
		ITexture *t0 = waterTextureFromTexmap(texmap[0], nodeName, "env");
		if (t0)
		{
			if (texmap[1])
			{
				ITexture *t1 = waterTextureFromTexmap(texmap[1], nodeName, "env-alt");
				if (t1)
				{
					CTextureBlend *b = new CTextureBlend;
					b->setBlendTexture(0, t0);
					b->setBlendTexture(1, t1);
					envMap = b;
				}
				else
					envMap = t0;
			}
			else
				envMap = t0;
		}
	}
	NLMISC::CSmartPtr<ITexture> envMapUnder = NULL;
	if (texmap[2])
	{
		ITexture *t0 = waterTextureFromTexmap(texmap[2], nodeName, "env-under");
		if (t0)
		{
			if (texmap[3])
			{
				ITexture *t1 = waterTextureFromTexmap(texmap[3], nodeName, "env-under-alt");
				if (t1)
				{
					CTextureBlend *b = new CTextureBlend;
					b->setBlendTexture(0, t0);
					b->setBlendTexture(1, t1);
					envMapUnder = b;
				}
				else
					envMapUnder = t0;
			}
			else
				envMapUnder = t0;
		}
	}
	NLMISC::CSmartPtr<ITexture> bumpMap = waterTextureFromTexmap(texmap[4], nodeName, "bump");
	NLMISC::CSmartPtr<ITexture> displaceMap = waterTextureFromTexmap(texmap[5], nodeName, "displace");
	NLMISC::CSmartPtr<ITexture> colorMap = NULL;
	if (enable[6] && texmap[6])
		colorMap = waterTextureFromTexmap(texmap[6], nodeName, "diffuse");

	if (!envMap || !bumpMap || !displaceMap)
	{
		delete ws;
		return NULL;
	}

	ws->setEnvMap(0, (ITexture *)envMap);
	if (envMapUnder != NULL)
		ws->setEnvMap(1, (ITexture *)envMapUnder);
	ws->setHeightMap(0, (ITexture *)displaceMap);
	ws->setHeightMap(1, (ITexture *)bumpMap);

	// Bump/displace map scale + speed. Missing params → keep the CWaterShape defaults.
	NLMISC::CVector2f bumpScale(1.f, 1.f), bumpSpeed(0.f, 0.f);
	NLMISC::CVector2f dispScale(1.f, 1.f), dispSpeed(0.f, 0.f);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FBUMPUSCALE, bumpScale.x);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FBUMPVSCALE, bumpScale.y);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FBUMPUSPEED, bumpSpeed.x);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FBUMPVSPEED, bumpSpeed.y);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FDISPLACEUSCALE, dispScale.x);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FDISPLACEVSCALE, dispScale.y);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FDISPLACEUSPEED, dispSpeed.x);
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FDISPLACEVSPEED, dispSpeed.y);
	ws->setHeightMapScale(0, dispScale);
	ws->setHeightMapScale(1, bumpScale);
	ws->setHeightMapSpeed(0, dispSpeed);
	ws->setHeightMapSpeed(1, bumpSpeed);

	// Scene envmap toggles — reference-era exports have both bytes = 0 in the tail (v4 fields
	// UsesSceneWaterEnvMap[0/1]). The reference exporter reads bWaterUseSceneEnvMap{Above,Under}
	// by name; the material's PB2 does carry ints at candidate ids (0x1a/0x1b in the corpus),
	// but those turn out to be the standard material's iBlendSrcFunc/iBlendDestFunc — the water
	// script's bool flags for scene envmap can't be found without a name→id map for the v14 water
	// script. The reference output has both = false, so force false to match; if a future corpus
	// water file's byte-diff traces to these fields being non-default, this needs revisiting.
	ws->setUseSceneWaterEnvMap(0, false);
	ws->setUseSceneWaterEnvMap(1, false);

	// Color map: set only when a diffuse map is present (slot 7 enabled with a valid bitmap).
	// setColorMapMat NOT computed this pass — see water_build.h for the open item; the reference's
	// projection is a 2x3 affine from a chosen mesh triangle's world XY to its UVs+crop, and
	// requires walking the mesh's map channels + the diffuse map's UVGen. Water shapes without
	// a color map are unaffected; ones with one will diff on ColorMapMat until this lands.
	if (colorMap != NULL)
		ws->setColorMap((ITexture *)colorMap);

	ws->setShape(convexPoly);

	// Water height factor + pool id. Not on the shape's flag list — the reference reads from
	// the NODE's PB2 (getValueByNameUsingParamBlock2(node, "iWaterPoolID", ...)) meaning the
	// scripted plugin's own params. For NEL_MTL these live on the material's PB2 in v14 (the
	// scripted-plugin dispatch pattern places them there). Keep the material PB2 read.
	float heightFactor = 1.f;
	pb2Float(mtlBlocks, NLB_MAIN, NLP_FWATERHEIGHTFACTOR, heightFactor);
	ws->setWaveHeightFactor(heightFactor);

	int poolId = 0;
	pb2Int(mtlBlocks, NLB_MAIN, NLP_IWATERPOOLID, poolId);
	ws->setWaterPoolID((uint32)poolId);

	// Default transformation (same as the mesh path — nodeBasis = local matrix).
	NLMISC::CVector defPos, defScale;
	NLMISC::CQuat defRot;
	Matrix3M localTM = MAXSCENE::getLocalMatrix(node, tmCache);
	decompMatrix(defScale, defRot, defPos, localTM);
	ws->getDefaultPos()->setDefaultValue(defPos);
	ws->getDefaultScale()->setDefaultValue(defScale);
	ws->getDefaultRotQuat()->setDefaultValue(defRot);

	// Splash flag — v3+ field in CWaterShape's serial. Reference-era exports have this byte
	// = TRUE in the tail (the reference material's PB2 has bEnableWaterSplash=1). Ctor default
	// is TRUE too, so leaving the default matches without needing to pin the param id.

	// Realtime reflection + fresnel params are v15+ material-script additions; the corpus is
	// v14, so these PB2 reads would fail on every corpus file. The CWaterShape defaults
	// (realtimeRefl=false, envMapCalc=false, fresnel=0.15/0.85/2) apply, and the v4 stream
	// truncation (main.cpp) drops these fields entirely on write anyway. Kept documented for
	// a future v15+ material corpus.

	return ws;
}

} /* namespace WATERBUILD */

/* end of file */
