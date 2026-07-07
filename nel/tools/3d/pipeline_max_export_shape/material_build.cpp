/**
 * \file material_build.cpp
 * \brief See material_build.h.
 *
 * NeL Material scene object (scripted plugin extending Standard): reference 0 = the delegate
 * Standard material, references 1..9 = the script's ParamBlock2s in declaration order
 * (nlbp, main, textures, slot1..slot4, slot5, slot6). The script version is the first dword
 * of the material's 0x0010 chunk (0x0010 = { u32 version, u32 pblockCount }); the whole
 * shape corpus is version 14. Parameter ids are positional per block (see the tables below).
 *
 * NeL Multi Bitmap texture (scripted plugin extending BitmapTexture): reference 0 = delegate
 * BitmapTex, reference 1 = its ParamBlock2 (bitmap1FileName..bitmap8FileName, ids 0..7).
 *
 * BitmapTex: references = { 0 = StdUVGen ("Placement"), 1 = ParamBlock2 (bmtex_params:
 * 0..3 = clipu/clipv/clipw/cliph, 4 = jitter, 5 = useJitter, 6 = apply, 7 = cropPlace,
 * 8 = filtering, 9 = monoOutput, 10 = rgbOutput, 11 = alphaSource, 12 = preMultAlpha,
 * 13 = bitmap (PBBitmap: 0x0003 container = BitmapInfo blob + UTF-16 file path + device
 * name), 14/15 = coords/output REFTARG aliases, 16 = legacy filename), 2 = TextureOutput,
 * 3 = ParamBlock2 (time) }.
 *
 * StdUVGen ("Placement"): no sub-references; state in legacy chunks — 0x9002 = flags dword
 * (bit 0 = U wrap, bit 1 = V wrap), 0x900b = map channel; offset/scale/angle chunks are only
 * written when non-default (not yet decoded — identity assumed, warned when present).
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
#include "material_build.h"

#include <cstdio>
#include <cstring>
#include <cmath>

#include <nel/misc/path.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_multi_file.h>
#include <nel/3d/texture_cube.h>

#include "../pipeline_max/builtin/node_impl.h"
#include "../pipeline_max/builtin/reference_maker.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace NLMISC;
using namespace NL3D;
using namespace MAXMATH;
using namespace SCENELIB;

namespace MATBUILD {

// ---------------------------------------------------------------------------------------------
// NeL material v14 parameter table (block index among the material's PB2 references, param id)

// blocks
#define NLB_NLBP 0
#define NLB_MAIN 1
#define NLB_TEXTURES 2
#define NLB_SLOT1 3

// nlbp params
#define NLP_BTWOSIDED 0
#define NLP_CAMBIENT 1
#define NLP_CDIFFUSE 2
#define NLP_POPACITY 3
#define NLP_CSPECULAR 4
#define NLP_PSPECULARLEVEL 5
#define NLP_PGLOSSINESS 6
#define NLP_CSELFILLUMCOLOR 7
#define NLP_PSELFILLUMAMOUNT 8
#define NLP_BUSESELFILLUMCOLOR 9

// main params (v14)
#define NLP_BLIGHTMAP 0
#define NLP_BUNLIGHTED 1
#define NLP_BSTAINEDGLASSWINDOW 2
#define NLP_BALPHATEST 3
#define NLP_BFORCEZWRITE 4
#define NLP_BFORCENOZWRITE 5
#define NLP_BWATER 6
#define NLP_BCOLORVERTEX 0x15
#define NLP_BALPHAVERTEX 0x16
#define NLP_IALPHAVERTEXCHANNEL 0x17
#define NLP_BALPHABLEND 0x18
#define NLP_IBLENDSRCFUNC 0x19
#define NLP_IBLENDDESTFUNC 0x1a
#define NLP_FZBIAS 0x1c
#define NLP_ILIGHTMAPCHANNEL 0x1d
#define NLP_ISHADERTYPE 0x1e
#define NLP_CUSERCOLOR 0x23
#define NLP_BEXPORTTEXTUREMATRIX 0x24

// textures params
#define NLP_BENABLESLOT_1 0
#define NLP_BTEXGEN_1 8
#define NLP_TTEXTURE_1 0x10

// slotN params
#define NLP_CCONSTANT 0
#define NLP_IRGBOPERATION 1
#define NLP_IRGBBLENDSOURCE 2
#define NLP_IRGBARG0 3
#define NLP_IRGBARG1 4
#define NLP_IRGBARG2 5
#define NLP_IRGBARG0OPERAND 6
#define NLP_IRGBARG1OPERAND 7
#define NLP_IRGBARG2OPERAND 8
#define NLP_IALPHAOPERATION 9
#define NLP_IALPHABLENDSOURCE 10
#define NLP_IALPHAARG0 11
#define NLP_IALPHAARG1 12
#define NLP_IALPHAARG2 13
#define NLP_IALPHAARG0OPERAND 14
#define NLP_IALPHAARG1OPERAND 15
#define NLP_IALPHAARG2OPERAND 16
#define NLP_ITEXTURESHADER 17
#define NLP_ICONSTANTALPHA 18

// bmtex_params (BitmapTex ParamBlock2 block 0)
#define BMT_CLIPU 0
#define BMT_CLIPV 1
#define BMT_CLIPW 2
#define BMT_CLIPH 3
#define BMT_APPLY 6
#define BMT_BITMAP 13

// ---------------------------------------------------------------------------------------------

std::string materialName(CSceneClass *mtl)
{
	CStorageRaw *raw = findRawChunk(mtl, 0x4001);
	if (!raw)
	{
		// materials with a 0x4000 base container carry the name inside it
		CStorageContainer *base = dynamic_cast<CStorageContainer *>(findChunk(mtl, 0x4000));
		if (base)
			raw = dynamic_cast<CStorageRaw *>(base->findStorageObject(0x4001));
	}
	if (!raw) return std::string();
	ucstring us;
	us.resize(raw->Value.size() / 2);
	if (!us.empty()) memcpy(&us[0], raw->Value.data(), us.size() * 2);
	return us.toUtf8();
}

// The scripted-plugin version chunk 0x0010 = { u32 scriptVersion, u32 pblockCount }
static bool scriptedPluginVersion(CSceneClass *sc, uint32 &version, uint32 &blockCount)
{
	CStorageRaw *raw = findRawChunk(sc, 0x0010);
	if (!raw || raw->Value.size() < 8) return false;
	memcpy(&version, raw->Value.data(), 4);
	memcpy(&blockCount, raw->Value.data() + 4, 4);
	return true;
}

static bool isNelMaterial(CSceneClass *mtl)
{
	return mtl && mtl->classDesc()->classId() == CLASSID_NEL_MTL;
}

static bool isMultiMaterial(CSceneClass *mtl)
{
	return mtl && mtl->classDesc()->classId() == CLASSID_MULTI_MTL;
}

// Sub-materials of a Multi/Sub-Object: reference 0 = its PB2, references 1..N = sub-materials.
static void getSubMaterials(CSceneClass *mtl, std::vector<CSceneClass *> &subs)
{
	subs.clear();
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mtl);
	if (!rm) return;
	uint count = 0;
	CStorageRaw *raw = findRawChunk(mtl, 0x4002);
	if (raw && raw->Value.size() >= 4)
	{
		uint32 n;
		memcpy(&n, raw->Value.data(), 4);
		count = n;
	}
	for (uint i = 1; i < rm->nbReferences() && subs.size() < count; ++i)
	{
		CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
		TSClassId scid = r ? r->classDesc()->superClassId() : 0;
		if (r && scid == 0xc00)
			subs.push_back(r);
	}
	if (subs.size() != count)
		fprintf(stderr, "WARNING: multi material '%s' with %u/%u sub materials\n",
		        materialName(mtl).c_str(), (uint)subs.size(), count);
}

// ---------------------------------------------------------------------------------------------
// NeL material PB2 access

struct SNelMtlParams
{
	std::vector<SPB2Block> Blocks;
	uint32 Version;

	bool getInt(uint block, uint16 id, int &out) const
	{
		const SPB2Param *p = findPB2Param(Blocks, block, id);
		if (!p || !p->HasConstant) return false;
		out = p->I;
		return true;
	}
	bool getFloat(uint block, uint16 id, float &out) const
	{
		const SPB2Param *p = findPB2Param(Blocks, block, id);
		if (!p || !p->HasConstant) return false;
		out = p->F[0];
		return true;
	}
	bool getColor(uint block, uint16 id, float out[3]) const
	{
		const SPB2Param *p = findPB2Param(Blocks, block, id);
		if (!p || !p->HasConstant) return false;
		out[0] = p->F[0];
		out[1] = p->F[1];
		out[2] = p->F[2];
		return true;
	}
	CSceneClass *getRef(uint block, uint16 id) const
	{
		const SPB2Param *p = findPB2Param(Blocks, block, id);
		if (!p) return NULL;
		return pb2RefValue(Blocks[block], *p);
	}
};

static int nelInt(const SNelMtlParams &np, uint block, uint16 id, int def)
{
	int v = def;
	np.getInt(block, id, v);
	return v;
}

static float nelFloat(const SNelMtlParams &np, uint block, uint16 id, float def)
{
	float v = def;
	np.getFloat(block, id, v);
	return v;
}

static CRGBA nelColor(const SNelMtlParams &np, uint block, uint16 id, CRGBA def)
{
	float c[3];
	if (!np.getColor(block, id, c)) return def;
	// convertColor: float 0..1 -> uint8 with +0.5 rounding
	float fR = c[0] * 255.f + 0.5f;
	float fG = c[1] * 255.f + 0.5f;
	float fB = c[2] * 255.f + 0.5f;
	clamp(fR, 0.f, 255.f);
	clamp(fG, 0.f, 255.f);
	clamp(fB, 0.f, 255.f);
	return CRGBA((uint8)fR, (uint8)fG, (uint8)fB);
}

// ---------------------------------------------------------------------------------------------
// Texture construction (buildATexture)

// StdUVGen state
struct SUVGen
{
	int MapChannel;
	uint32 Flags;
	bool WrapU, WrapV;
	SUVGen() : MapChannel(1), Flags(0x00000003), WrapU(true), WrapV(true) { }
};

static void readUVGen(CSceneClass *uvgen, SUVGen &out, const std::string &texName)
{
	if (!uvgen) return;
	CStorageRaw *raw = findRawChunk(uvgen, 0x900b);
	if (raw && raw->Value.size() >= 4)
	{
		uint32 chan;
		memcpy(&chan, raw->Value.data(), 4);
		out.MapChannel = (int)chan;
	}
	raw = findRawChunk(uvgen, 0x9002);
	if (raw && raw->Value.size() >= 4)
	{
		memcpy(&out.Flags, raw->Value.data(), 4);
		out.WrapU = (out.Flags & 0x1) != 0;
		out.WrapV = (out.Flags & 0x2) != 0;
	}
	// Offset/scale/angle state appears as extra chunks only when non-default; warn until the
	// UV transform decode lands so affected textures get bucketed by the harness.
	static const uint16 knownIds[] = { 0x9002, 0x9003, 0x9005, 0x9006, 0x9009, 0x900b, 0x2150, 0x2034, 0x2035, 0x2045, 0x2047, 0x204B, 0x21B0, 0 };
	const CStorageContainer::TStorageObjectContainer &orphans = uvgen->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		bool known = false;
		for (const uint16 *p = knownIds; *p; ++p)
			if (it->first == *p) { known = true; break; }
		if (!known)
			fprintf(stderr, "WARNING: texture '%s' UVGen carries undecoded chunk 0x%04x (UV transform not identity?)\n",
			        texName.c_str(), it->first);
	}
}

// The BitmapTex's file path: PB2 block 0 param 13 (PBBitmap), whose value rides in the record's
// trailing 0x0003 container = { BitmapInfo blob, UTF-16 file path, UTF-16 device name }.
static bool bmtexFileName(CSceneClass *bmtex, std::string &out)
{
	// The 0x0003 container is a sibling chunk of the PB2's 0x000e records.
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
			// children: [0] BitmapInfo blob, [1] UTF-16 path, [2] UTF-16 device
			uint idx = 0;
			for (CStorageContainer::TStorageObjectConstIt jt = c->chunks().begin(); jt != c->chunks().end(); ++jt, ++idx)
			{
				if (idx != 1) continue;
				CStorageRaw *raw = dynamic_cast<CStorageRaw *>(jt->second);
				if (!raw) return false;
				ucstring us;
				us.resize(raw->Value.size() / 2);
				if (!us.empty()) memcpy(&us[0], raw->Value.data(), us.size() * 2);
				out = us.toUtf8();
				while (!out.empty() && out[out.size() - 1] == '\0') out.resize(out.size() - 1);
				return !out.empty();
			}
		}
	}
	return false;
}

static std::string convertTexFileName(const std::string &path)
{
	// _AbsolutePath is false for the export pipeline: keep the basename only.
	return NLMISC::CFile::getFilename(path);
}

// buildATexture: returns the texture and fills the channel description.
static ITexture *buildATexture(CSceneClass *texmap, SMaterialDesc &remap, bool forceCubic, const std::string &mtlName)
{
	if (!texmap) return NULL;
	NLMISC::CClassId cid = texmap->classDesc()->classId();
	ITexture *pTexture = NULL;
	CSceneClass *bmtexDelegate = NULL;

	if (cid == CLASSID_NEL_BMTEX || cid == CLASSID_BMTEX)
	{
		// Crop values come from the (delegate) BitmapTex's pblock.
		CSceneClass *bmtex = texmap;
		if (cid == CLASSID_NEL_BMTEX)
		{
			// reference 0 = delegate BitmapTex
			CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(texmap);
			for (uint i = 0; rm && i < rm->nbReferences(); ++i)
			{
				CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
				if (r && r->classDesc()->classId() == CLASSID_BMTEX)
				{
					bmtex = r;
					break;
				}
			}
		}
		bmtexDelegate = bmtex;

		std::vector<SPB2Block> blocks;
		readObjectPB2Blocks(bmtex, blocks);
		int bApply = 0;
		{
			const SPB2Param *p = findPB2Param(blocks, 0, BMT_APPLY);
			if (p && p->HasConstant) bApply = p->I;
			else fprintf(stderr, "WARNING: material '%s': bmtex without apply param\n", mtlName.c_str());
		}
		if (bApply)
		{
			const SPB2Param *p;
			if ((p = findPB2Param(blocks, 0, BMT_CLIPU)) && p->HasConstant) remap.CropU = p->F[0];
			if ((p = findPB2Param(blocks, 0, BMT_CLIPV)) && p->HasConstant) remap.CropV = p->F[0];
			if ((p = findPB2Param(blocks, 0, BMT_CLIPW)) && p->HasConstant) remap.CropW = p->F[0];
			if ((p = findPB2Param(blocks, 0, BMT_CLIPH)) && p->HasConstant) remap.CropH = p->F[0];
		}

		if (cid == CLASSID_NEL_BMTEX)
		{
			// Texture set: bitmap1FileName..bitmap8FileName from the scripted plugin's own pblock.
			std::vector<SPB2Block> ownBlocks;
			readObjectPB2Blocks(texmap, ownBlocks);
			// its own pblock is the one with string params; the delegate's blocks may precede
			std::string fileName[8];
			uint numUsedSlots = 0;
			for (uint b = 0; b < ownBlocks.size(); ++b)
			{
				const SPB2Param *p0 = findPB2Param(ownBlocks, b, 0);
				if (!p0 || (p0->Type & 0x07ff) != PB2_STRING) continue;
				for (uint k = 0; k < 8; ++k)
				{
					const SPB2Param *p = findPB2Param(ownBlocks, b, (uint16)k);
					if (p && p->HasConstant) fileName[k] = p->S;
				}
				break;
			}
			for (uint l = 0; l < 8; ++l)
				if (!fileName[l].empty()) numUsedSlots = l + 1;
			// NB: the reference exporter's slot loop has an oddity: after the loop l == 8, so the
			// single-slot CTextureFile path is only taken when... l==1 never holds; it always
			// builds a CTextureMultiFile. Replicate that (CTextureMultiFile even for one slot).
			CTextureMultiFile *multi = new CTextureMultiFile(numUsedSlots);
			for (uint k = 0; k < numUsedSlots; ++k)
				if (!fileName[k].empty())
					multi->setFileName(k, convertTexFileName(fileName[k]).c_str());
			pTexture = multi;
		}
		else
		{
			std::string mapName;
			if (!bmtexFileName(bmtex, mapName))
				fprintf(stderr, "WARNING: material '%s': bitmap texture without file name\n", mtlName.c_str());
			CTextureFile *tf = new CTextureFile;
			tf->setFileName(convertTexFileName(mapName));
			pTexture = tf;
		}

		if (forceCubic)
		{
			// Duplicate into a cube map (specular shader side 2)
			const static CTextureCube::TFace tfNewOrder[6] = {
				CTextureCube::positive_z, CTextureCube::negative_z,
				CTextureCube::negative_x, CTextureCube::positive_x,
				CTextureCube::positive_y, CTextureCube::negative_y
			};
			CTextureCube *cube = new CTextureCube;
			for (uint side = 0; side < 6; side++)
			{
				if (dynamic_cast<CTextureMultiFile *>(pTexture))
					cube->setTexture(tfNewOrder[side], new CTextureMultiFile(*static_cast<CTextureMultiFile *>(pTexture)));
				else
					cube->setTexture(tfNewOrder[side], new CTextureFile(*static_cast<CTextureFile *>(pTexture)));
			}
			delete pTexture;
			pTexture = cube;
		}
	}
	else
	{
		fprintf(stderr, "WARNING: material '%s': texmap class %s not supported\n",
		        mtlName.c_str(), cid.toString().c_str());
		return NULL;
	}

	// UV channel + matrix from the UVGen (delegate's reference 0)
	SUVGen uvgen;
	{
		CSceneClass *uvgenObj = NULL;
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(bmtexDelegate);
		for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (r && r->classDesc()->superClassId() == 0xc20)
			{
				uvgenObj = r;
				break;
			}
		}
		readUVGen(uvgenObj, uvgen, mtlName);
	}
	remap.IndexInMaxMaterial = uvgen.MapChannel;
	remap.UVMatrix = Matrix3M::identity(); // non-identity UVGen transforms warn in readUVGen

	if (pTexture)
	{
		if (!uvgen.WrapU) pTexture->setWrapS(ITexture::Clamp);
		if (!uvgen.WrapV) pTexture->setWrapT(ITexture::Clamp);
	}
	return pTexture;
}

// ---------------------------------------------------------------------------------------------
// buildAMaterial

static void buildANelMaterial(CMaterial &material, SMaterialInfo &materialInfo, CSceneClass *mtl, bool exportLighting)
{
	std::string mtlName = materialName(mtl);

	SNelMtlParams np;
	readObjectPB2Blocks(mtl, np.Blocks);
	uint32 version = 0, blockCount = 0;
	scriptedPluginVersion(mtl, version, blockCount);
	np.Version = version;
	if (version != 14)
		fprintf(stderr, "WARNING: material '%s': NeL material script version %u (parameter table is v14)\n",
		        mtlName.c_str(), version);

	// Shader type
	int iShaderType = nelInt(np, NLB_MAIN, NLP_ISHADERTYPE, 0);
	if (iShaderType == SHADER_LIGHTMAP && !exportLighting)
		iShaderType = SHADER_NORMAL;
	bool isWaterOrLightmap = iShaderType == SHADER_WATER || iShaderType == SHADER_LIGHTMAP;
	bool isLightmap = iShaderType == SHADER_LIGHTMAP;

	// Lighted?
	int bUnlighted = nelInt(np, NLB_MAIN, NLP_BUNLIGHTED, 0);
	if (isLightmap)
		bUnlighted = 1;
	if (bUnlighted)
		material.initUnlit();
	else
		material.initLighted();

	// Shader
	switch (iShaderType)
	{
	case SHADER_NORMAL:
	case SHADER_BUMP:
	case SHADER_USER_COLOR:
	case SHADER_LIGHTMAP:
	case SHADER_SPECULAR:
	case SHADER_PER_PIXEL_LIGHTING:
	case SHADER_PER_PIXEL_LIGHTING_NO_SPEC:
		material.setShader((CMaterial::TShader)(iShaderType - 1));
		break;
	case SHADER_WATER:
		material.setShader(CMaterial::Normal);
		break;
	default:
		fprintf(stderr, "WARNING: material '%s': unknown iShaderType %d\n", mtlName.c_str(), iShaderType);
		material.setShader(CMaterial::Normal);
		break;
	}

	// Stained glass window flag
	material.setStainedGlassWindow(nelInt(np, NLB_MAIN, NLP_BSTAINEDGLASSWINDOW, 0) != 0);

	// Alpha
	int bAlphaBlend = nelInt(np, NLB_MAIN, NLP_BALPHABLEND, 0);
	material.setBlend(bAlphaBlend != 0);
	material.setSrcBlend((CMaterial::TBlend)(nelInt(np, NLB_MAIN, NLP_IBLENDSRCFUNC, 1) - 1));
	material.setDstBlend((CMaterial::TBlend)(nelInt(np, NLB_MAIN, NLP_IBLENDDESTFUNC, 1) - 1));
	if (isWaterOrLightmap)
		material.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);

	int bAlphaTest = nelInt(np, NLB_MAIN, NLP_BALPHATEST, 0);
	material.setAlphaTest(bAlphaTest != 0);

	int bAlphaVertex = nelInt(np, NLB_MAIN, NLP_BALPHAVERTEX, 0);
	if (isWaterOrLightmap)
		bAlphaVertex = 0;
	materialInfo.AlphaVertex = (bAlphaBlend || bAlphaTest) && (bAlphaVertex != 0);
	materialInfo.AlphaVertexChannel = (uint)nelInt(np, NLB_MAIN, NLP_IALPHAVERTEXCHANNEL, 0);

	// ZBuffer
	material.setZBias(nelFloat(np, NLB_MAIN, NLP_FZBIAS, 0.0f));
	if (bAlphaBlend)
		material.setZWrite(nelInt(np, NLB_MAIN, NLP_BFORCEZWRITE, 0) != 0);
	else
		material.setZWrite(nelInt(np, NLB_MAIN, NLP_BFORCENOZWRITE, 0) == 0);

	// Colors
	int bColorVertex = nelInt(np, NLB_MAIN, NLP_BCOLORVERTEX, 0);
	material.setLightedVertexColor(material.isLighted() && bColorVertex != 0);
	materialInfo.ColorVertex = bColorVertex != 0;

	// Diffuse + opacity
	CRGBA nelDiffuse = nelColor(np, NLB_NLBP, NLP_CDIFFUSE, CRGBA::White);
	float fOp = nelFloat(np, NLB_NLBP, NLP_POPACITY, 0.0f);
	{
		float fA = fOp * 255.f + 0.5f;
		clamp(fA, 0.f, 255.f);
		nelDiffuse.A = (uint8)fA;
	}
	if (bUnlighted)
	{
		material.setColor(nelDiffuse);
		material.setDiffuse(nelDiffuse);
	}
	else
	{
		material.setDiffuse(nelDiffuse);
		material.setOpacity(nelDiffuse.A);
	}

	// Self illum
	{
		CRGBA nelEmissive;
		int bSelfIllumColorOn = nelInt(np, NLB_NLBP, NLP_BUSESELFILLUMCOLOR, 0);
		if (bSelfIllumColorOn)
		{
			nelEmissive = nelColor(np, NLB_NLBP, NLP_CSELFILLUMCOLOR, CRGBA::Black);
		}
		else
		{
			float amount = nelFloat(np, NLB_NLBP, NLP_PSELFILLUMAMOUNT, 0.0f);
			float c[3] = { 0.0f, 0.0f, 0.0f };
			np.getColor(NLB_NLBP, NLP_CDIFFUSE, c);
			float fR = c[0] * amount * 255.f + 0.5f;
			float fG = c[1] * amount * 255.f + 0.5f;
			float fB = c[2] * amount * 255.f + 0.5f;
			clamp(fR, 0.f, 255.f);
			clamp(fG, 0.f, 255.f);
			clamp(fB, 0.f, 255.f);
			nelEmissive = CRGBA((uint8)fR, (uint8)fG, (uint8)fB);
		}
		material.setEmissive(nelEmissive);
	}

	// Ambient
	material.setAmbient(nelColor(np, NLB_NLBP, NLP_CAMBIENT, CRGBA::Black));

	// Specular
	{
		CRGBA nelSpecular = nelColor(np, NLB_NLBP, NLP_CSPECULAR, CRGBA::Black);
		float shininess = nelFloat(np, NLB_NLBP, NLP_PSPECULARLEVEL, 0.0f);
		clamp(shininess, 0.f, 1.f);
		CRGBAF fColor = nelSpecular;
		fColor *= shininess;
		nelSpecular = fColor;
		material.setSpecular(nelSpecular);

		shininess = nelFloat(np, NLB_NLBP, NLP_PGLOSSINESS, 0.0f);
		shininess = (float)pow(2.0, shininess * 10.0) * 4.f;
		material.setShininess(shininess);
	}

	// User color
	if (iShaderType == SHADER_USER_COLOR)
		material.setUserColor(nelColor(np, NLB_MAIN, NLP_CUSERCOLOR, CRGBA::Black));

	// Double sided
	material.setDoubleSided(nelInt(np, NLB_NLBP, NLP_BTWOSIDED, 0) != 0);

	// Textures
	int bExportTexMatAnim = nelInt(np, NLB_MAIN, NLP_BEXPORTTEXTUREMATRIX, 0);
	materialInfo.TextureMatrixEnabled = bExportTexMatAnim != 0;
	materialInfo.RemapChannel.clear();

	for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
	{
		if ((iShaderType == SHADER_USER_COLOR) && (i > 0)) break;
		if ((iShaderType == SHADER_LIGHTMAP) && (i > 0)) break;
		if ((iShaderType == SHADER_SPECULAR) && (i > 1)) break;

		int bEnableSlot = nelInt(np, NLB_TEXTURES, (uint16)(NLP_BENABLESLOT_1 + i), 0);
		if (!bEnableSlot) continue;

		CSceneClass *pTexmap = np.getRef(NLB_TEXTURES, (uint16)(NLP_TTEXTURE_1 + i));
		if (!pTexmap) continue;

		SMaterialDesc materialDesc;
		ITexture *pTexture = buildATexture(pTexmap, materialDesc, (i == 1) && (iShaderType == SHADER_SPECULAR), mtlName);

		int bTexGen = nelInt(np, NLB_TEXTURES, (uint16)(NLP_BTEXGEN_1 + i), 0);
		if (((i == 1) && (iShaderType == SHADER_SPECULAR)) || bTexGen)
			materialDesc.IndexInMaxMaterial = UVGEN_REFLEXION;

		materialInfo.RemapChannel.push_back(materialDesc);

		if (materialDesc.IndexInMaxMaterial >= 0)
		{
			uint j;
			for (j = 0; j < i; j++)
			{
				if (j < materialInfo.RemapChannel.size()
					&& materialInfo.RemapChannel[j].IndexInMaxMaterial == materialDesc.IndexInMaxMaterial)
					break;
			}
			materialInfo.UVRouting[i] = (j == i) ? i : j;
		}

		material.setTexture((uint8)i, pTexture);
		material.setTexCoordGen(i, materialDesc.IndexInMaxMaterial < 0);

		if (bExportTexMatAnim != 0)
		{
			material.enableUserTexMat(i);
			// uvMatrix2NelUVMatrix of the (identity until decoded) UV matrix
			material.setUserTexMat(i, NLMISC::CMatrix::Identity);
		}
	}

	// Lightmap channel
	if (iShaderType == SHADER_LIGHTMAP)
	{
		int iLightMapChannel = nelInt(np, NLB_MAIN, NLP_ILIGHTMAPCHANNEL, 0);
		uint size = (uint)materialInfo.RemapChannel.size();
		materialInfo.RemapChannel.resize(size + 1);
		if (materialInfo.RemapChannel.size() > 1)
			materialInfo.RemapChannel[size] = materialInfo.RemapChannel[0];
		materialInfo.RemapChannel[size].IndexInMaxMaterial = iLightMapChannel;
		materialInfo.RemapChannel[size].IndexInMaxMaterialAlternative = materialInfo.RemapChannel[0].IndexInMaxMaterial;
	}

	// Multitexture stage environments (SHADER_NORMAL only)
	for (uint stage = 0; stage < 4 && stage < IDRV_MAT_MAXTEXTURES; stage++)
	{
		if (iShaderType != SHADER_NORMAL) continue;
		uint block = NLB_SLOT1 + stage;

		int opRGB = nelInt(np, block, NLP_IRGBOPERATION, 0);
		int opRGBBlend = nelInt(np, block, NLP_IRGBBLENDSOURCE, 0);
		int opRGBArg0 = nelInt(np, block, NLP_IRGBARG0, 0);
		int opRGBArg1 = nelInt(np, block, NLP_IRGBARG1, 0);
		int opRGBArg2 = nelInt(np, block, NLP_IRGBARG2, 0);
		int opRGBArg0Operand = nelInt(np, block, NLP_IRGBARG0OPERAND, 0);
		int opRGBArg1Operand = nelInt(np, block, NLP_IRGBARG1OPERAND, 0);
		int opRGBArg2Operand = nelInt(np, block, NLP_IRGBARG2OPERAND, 0);
		if (opRGB < 5)
			material.texEnvOpRGB(stage, (CMaterial::TTexOperator)(opRGB - 1));
		else if (opRGB == 6)
			material.texEnvOpRGB(stage, CMaterial::Mad);
		else
			material.texEnvOpRGB(stage, (CMaterial::TTexOperator)(opRGBBlend + 3));
		material.texEnvArg0RGB(stage, (CMaterial::TTexSource)(opRGBArg0 - 1), (CMaterial::TTexOperand)(opRGBArg0Operand - 1));
		if (opRGBArg1 == 4)
			material.texEnvArg1RGB(stage, CMaterial::Texture, (CMaterial::TTexOperand)(opRGBArg1Operand - 1));
		else
			material.texEnvArg1RGB(stage, (CMaterial::TTexSource)(opRGBArg1), (CMaterial::TTexOperand)(opRGBArg1Operand - 1));
		if (opRGBArg2 == 4)
			material.texEnvArg2RGB(stage, CMaterial::Texture, (CMaterial::TTexOperand)(opRGBArg2Operand - 1));
		else
			material.texEnvArg2RGB(stage, (CMaterial::TTexSource)(opRGBArg2), (CMaterial::TTexOperand)(opRGBArg2Operand - 1));

		int opAlpha = nelInt(np, block, NLP_IALPHAOPERATION, 0);
		int opAlphaBlend = nelInt(np, block, NLP_IALPHABLENDSOURCE, 0);
		int opAlphaArg0 = nelInt(np, block, NLP_IALPHAARG0, 0);
		int opAlphaArg1 = nelInt(np, block, NLP_IALPHAARG1, 0);
		int opAlphaArg2 = nelInt(np, block, NLP_IALPHAARG2, 0);
		int opAlphaArg0Operand = nelInt(np, block, NLP_IALPHAARG0OPERAND, 0);
		int opAlphaArg1Operand = nelInt(np, block, NLP_IALPHAARG1OPERAND, 0);
		int opAlphaArg2Operand = nelInt(np, block, NLP_IALPHAARG2OPERAND, 0);
		if (opAlpha < 5)
			material.texEnvOpAlpha(stage, (CMaterial::TTexOperator)(opAlpha - 1));
		else if (opAlpha == 6)
			material.texEnvOpAlpha(stage, CMaterial::Mad);
		else
			material.texEnvOpAlpha(stage, (CMaterial::TTexOperator)(opAlphaBlend + 3));
		material.texEnvArg0Alpha(stage, (CMaterial::TTexSource)(opAlphaArg0 - 1), (CMaterial::TTexOperand)(opAlphaArg0Operand - 1));
		if (opAlphaArg1 == 4)
			material.texEnvArg1Alpha(stage, CMaterial::Texture, (CMaterial::TTexOperand)(opAlphaArg1Operand - 1));
		else
			material.texEnvArg1Alpha(stage, (CMaterial::TTexSource)(opAlphaArg1), (CMaterial::TTexOperand)(opAlphaArg1Operand - 1));
		if (opAlphaArg2 == 4)
			material.texEnvArg2Alpha(stage, CMaterial::Texture, (CMaterial::TTexOperand)(opAlphaArg2Operand - 1));
		else
			material.texEnvArg2Alpha(stage, (CMaterial::TTexSource)(opAlphaArg2), (CMaterial::TTexOperand)(opAlphaArg2Operand - 1));

		// Constant color + alpha
		CRGBA nelConstantColor = nelColor(np, block, NLP_CCONSTANT, CRGBA::White);
		nelConstantColor.A = (uint8)nelInt(np, block, NLP_ICONSTANTALPHA, 255);
		material.texConstantColor(stage, nelConstantColor);

		// Texture addressing mode
		int texEnvMode = nelInt(np, block, NLP_ITEXTURESHADER, 0);
		if (texEnvMode > 1)
		{
			material.enableTexAddrMode();
			material.setTexAddressingMode(stage, (CMaterial::TTexAddressingMode)(texEnvMode - 2));
		}
	}

	materialInfo.MaterialName = mtlName;
}

// Non-NeL material fallback: only the Standard-material path of the reference exporter's else
// branch matters in the corpus (one instance). No texture (the diffuse texmap of a StdMat2
// rides its Texmaps container — decoded when the corpus needs it), colors from the delegate
// blocks by name.
static void buildAStdMaterial(CMaterial &material, SMaterialInfo &materialInfo, CSceneClass *mtl)
{
	std::string mtlName = materialName(mtl);
	fprintf(stderr, "WARNING: material '%s': non-NeL material class %s; std material path incomplete\n",
	        mtlName.c_str(), mtl->classDesc()->classId().toString().c_str());
	material.initLighted();
	material.setShader(CMaterial::Normal);
	material.setAlphaTest(false);
	material.setBlend(false);
	material.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	material.setZFunc(CMaterial::lessequal);
	material.setZBias(0.f);
	materialInfo.MaterialName = mtlName;
}

static void buildAMaterial(CMaterial &material, SMaterialInfo &materialInfo, CSceneClass *mtl, bool exportLighting)
{
	if (isNelMaterial(mtl))
		buildANelMaterial(material, materialInfo, mtl, exportLighting);
	else
		buildAStdMaterial(material, materialInfo, mtl);
}

// ---------------------------------------------------------------------------------------------

bool hasWaterMaterial(INode &node)
{
	CSceneClass *mtl = materialOf(node);
	if (!mtl) return false;
	if (isMultiMaterial(mtl)) return false; // sub-materials not supported for water
	if (!isNelMaterial(mtl)) return false;
	SNelMtlParams np;
	readObjectPB2Blocks(mtl, np.Blocks);
	return nelInt(np, NLB_MAIN, NLP_BWATER, 0) != 0;
}

static bool needVP(CSceneClass *mtl, NL3D::CMaterial::TShader &shader)
{
	if (!mtl) return false;
	if (isMultiMaterial(mtl))
	{
		std::vector<CSceneClass *> subs;
		getSubMaterials(mtl, subs);
		for (uint i = 0; i < subs.size(); ++i)
			if (needVP(subs[i], shader)) return true;
		return false;
	}
	if (!isNelMaterial(mtl)) return false;
	SNelMtlParams np;
	readObjectPB2Blocks(mtl, np.Blocks);
	switch (nelInt(np, NLB_MAIN, NLP_ISHADERTYPE, 0))
	{
	case SHADER_PER_PIXEL_LIGHTING:
		shader = CMaterial::PerPixelLighting;
		return true;
	case SHADER_PER_PIXEL_LIGHTING_NO_SPEC:
		shader = CMaterial::PerPixelLightingNoSpec;
		return true;
	}
	return false;
}

bool hasMaterialWithShaderForVP(INode &node, NL3D::CMaterial::TShader &shader)
{
	return needVP(materialOf(node), shader);
}

void buildMaterials(std::vector<NL3D::CMaterial> &materials, SMaxMeshBaseBuild &maxBaseBuild, INode &node,
                    bool exportLighting)
{
	maxBaseBuild.FirstMaterial = (uint)materials.size();
	uint nMaterialCount = 0;

	CSceneClass *pNodeMat = materialOf(node);
	if (pNodeMat)
	{
		std::vector<CSceneClass *> subs;
		if (isMultiMaterial(pNodeMat))
			getSubMaterials(pNodeMat, subs);
		if (!subs.empty())
		{
			nMaterialCount = (uint)subs.size();
			materials.resize(materials.size() + nMaterialCount);
			maxBaseBuild.MaterialInfo.resize(nMaterialCount);
			for (uint nSub = 0; nSub < nMaterialCount; nSub++)
			{
				buildAMaterial(materials[maxBaseBuild.FirstMaterial + nSub], maxBaseBuild.MaterialInfo[nSub], subs[nSub], exportLighting);
				maxBaseBuild.NeedVertexColor |= maxBaseBuild.MaterialInfo[nSub].AlphaVertex | maxBaseBuild.MaterialInfo[nSub].ColorVertex;
			}
		}
		else
		{
			nMaterialCount = 1;
			materials.resize(materials.size() + 1);
			maxBaseBuild.MaterialInfo.resize(1);
			buildAMaterial(materials[maxBaseBuild.FirstMaterial], maxBaseBuild.MaterialInfo[0], pNodeMat, exportLighting);
			maxBaseBuild.NeedVertexColor |= maxBaseBuild.MaterialInfo[0].AlphaVertex | maxBaseBuild.MaterialInfo[0].ColorVertex;
		}
	}

	// Normalize UVRouting
	for (uint i = 0; i < maxBaseBuild.MaterialInfo.size(); i++)
	{
		for (uint j = 0; j < MAX_MAX_TEXTURE; j++)
		{
			uint8 routing = maxBaseBuild.MaterialInfo[i].UVRouting[j];
			if (maxBaseBuild.UVRouting[j] == 0xff)
			{
				maxBaseBuild.UVRouting[j] = routing;
			}
			else
			{
				if (routing != 0xff && routing != maxBaseBuild.UVRouting[j])
				{
					// Active the channel because someone needs it
					maxBaseBuild.UVRouting[j] = (uint8)j;
				}
			}
		}
	}

	// If no material exported
	if (nMaterialCount == 0)
	{
		materials.resize(materials.size() + 1);
		nMaterialCount = 1;
		maxBaseBuild.MaterialInfo.resize(1);
		materials[maxBaseBuild.FirstMaterial].initLighted();
		materials[maxBaseBuild.FirstMaterial].setLighting(true, CRGBA::Black, CRGBA::White, CRGBA::White, CRGBA::Black);
		maxBaseBuild.MaterialInfo[0].MaterialName = "Default";
	}

	maxBaseBuild.NumMaterials = nMaterialCount;
}

} /* namespace MATBUILD */

/* end of file */
