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
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
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
#include "../pipeline_max/builtin/mtl_base.h"
#include "../pipeline_max/builtin/multi_mtl.h"
#include "../pipeline_max_export_common/old_param_block.h"

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
	// Materials/texmaps are typed in pipeline_max proper (BUILTIN::CMtlBase, registered for
	// superclasses 0xc00/0xc10): the 0x4000 base + 0x4001 name decode lives there now.
	if (CMtlBase *mb = dynamic_cast<CMtlBase *>(mtl))
		return mb->name();
	// Fallback for any non-CMtlBase carrier (defensive; should not occur for materials).
	CStorageRaw *raw = findRawChunk(mtl, 0x4001);
	if (!raw)
	{
		CStorageContainer *base = dynamic_cast<CStorageContainer *>(findChunk(mtl, 0x4000));
		if (base)
			raw = dynamic_cast<CStorageRaw *>(base->findStorageObject(0x4001));
	}
	if (!raw) return std::string();
	ucstring us;
	us.resize(raw->Value.size() / 2);
	if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
	return us.toUtf8();
}

// The scripted-plugin version chunk 0x0010 = { u32 scriptVersion, u32 pblockCount }
static bool scriptedPluginVersion(CSceneClass *sc, uint32 &version, uint32 &blockCount)
{
	CStorageRaw *raw = findRawChunk(sc, 0x0010);
	if (!raw || raw->Value.size() < 8) return false;
	memcpy(&version, nlVectorData(raw->Value), 4);
	memcpy(&blockCount, nlVectorData(raw->Value) + 4, 4);
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
		memcpy(&n, nlVectorData(raw->Value), 4);
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
		if (!p) return nullptr;
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

// StdUVGen state + UV transform matrix construction
//
// The offset/tiling/angle state lives on the UVGen's reference 0 = an old-style ParamBlock
// (superclass 0x8), keyed by declared index (§10z-cinq stduvgen dataset — verified against
// ~/shape_export_dataset/stduvgen_{baseline,offset,tile2x3,angle45}):
//   0 U Offset, 1 V Offset, 2 U Tiling, 3 V Tiling, 4 U Angle, 5 V Angle, 6 W Angle.
// For animated params the pblock's 0x0002 value chunk is replaced by an empty 0x0200 marker
// (§10k / anim_build.cpp) and the controller sits on the pblock's reference slot. We take the
// baked-in static value from the pblock's own value chunk when present, else the default (0 for
// offset/angle, 1 for tiling) — the animation track drives the runtime matrix regardless. This
// matches the reference plugin's texmap->GetUVTransform(channelMatrix) at t=0 for the static
// non-animated case (~48-byte-per-animated-material shape T3 gap per §10z round 3).
struct SUVGen
{
	int MapChannel;
	uint32 Flags;
	bool WrapU, WrapV;
	float UOffset, VOffset, WOffset;
	float UTiling, VTiling, WTiling;
	float UAngle, VAngle, WAngle;
	SUVGen() : MapChannel(1), Flags(0x00000003), WrapU(true), WrapV(true),
	           UOffset(0.f), VOffset(0.f), WOffset(0.f),
	           UTiling(1.f), VTiling(1.f), WTiling(1.f),
	           UAngle(0.f), VAngle(0.f), WAngle(0.f) { }
};

// Row-vector Matrix3M helpers for the Max UV transform chain. Max's Matrix3::Scale/RotateX/etc.
// post-multiply (this = this * op); operating on row vectors, that means transforms accumulate in
// application order (v' = v * S * R * T scales first, then rotates, then translates).
static Matrix3M m3Scale(float sx, float sy, float sz)
{
	Matrix3M r = Matrix3M::identity();
	r.m[0][0] = sx; r.m[1][1] = sy; r.m[2][2] = sz;
	return r;
}
static Matrix3M m3RotateX(float a)
{
	Matrix3M r = Matrix3M::identity();
	float c = cosf(a), s = sinf(a);
	r.m[1][1] = c; r.m[1][2] = s;
	r.m[2][1] = -s; r.m[2][2] = c;
	return r;
}
static Matrix3M m3RotateY(float a)
{
	Matrix3M r = Matrix3M::identity();
	float c = cosf(a), s = sinf(a);
	r.m[0][0] = c; r.m[0][2] = -s;
	r.m[2][0] = s; r.m[2][2] = c;
	return r;
}
static Matrix3M m3RotateZ(float a)
{
	Matrix3M r = Matrix3M::identity();
	float c = cosf(a), s = sinf(a);
	r.m[0][0] = c; r.m[0][1] = s;
	r.m[1][0] = -s; r.m[1][1] = c;
	return r;
}
static Matrix3M m3Translate(float tx, float ty, float tz)
{
	Matrix3M r = Matrix3M::identity();
	r.m[3][0] = tx; r.m[3][1] = ty; r.m[3][2] = tz;
	return r;
}

// Max SDK StdUVGen::GetUVTransform semantics: tm = identity; tm.Scale(tiling); tm.RotateX(uAngle);
// tm.RotateY(vAngle); tm.RotateZ(wAngle); tm.Translate(offset).
static Matrix3M uvgenToMatrix3(const SUVGen &g)
{
	Matrix3M tm = Matrix3M::identity();
	tm = tm * m3Scale(g.UTiling, g.VTiling, g.WTiling);
	tm = tm * m3RotateX(g.UAngle);
	tm = tm * m3RotateY(g.VAngle);
	tm = tm * m3RotateZ(g.WAngle);
	tm = tm * m3Translate(g.UOffset, g.VOffset, g.WOffset);
	return tm;
}

// CExportNel::uvMatrix2NelUVMatrix (export_misc.cpp:869): copy the Max rows as NeL basis vectors +
// translation, then apply the similarity transform dest = C * dest * C where
// C.rot = (I, -J, K) and C.pos = J = (0,1,0) — this handles the V-axis flip between Max's UV
// convention and NeL's. Reproduces the reference operation-for-operation.
static NLMISC::CMatrix uvMatrix2NelUVMatrix(const Matrix3M &m)
{
	NLMISC::CVector I(m.m[0][0], m.m[0][1], m.m[0][2]);
	NLMISC::CVector J(m.m[1][0], m.m[1][1], m.m[1][2]);
	NLMISC::CVector K(m.m[2][0], m.m[2][1], m.m[2][2]);
	NLMISC::CVector P(m.m[3][0], m.m[3][1], m.m[3][2]);
	NLMISC::CMatrix dest;
	dest.identity();
	dest.setRot(I, J, K);
	dest.setPos(P);
	NLMISC::CMatrix convert;
	convert.setRot(NLMISC::CVector::I, -NLMISC::CVector::J, NLMISC::CVector::K);
	convert.setPos(NLMISC::CVector::J);
	dest = convert * dest * convert;
	return dest;
}

static void readUVGen(CSceneClass *uvgen, SUVGen &out, const std::string &texName)
{
	if (!uvgen) return;
	CStorageRaw *raw = findRawChunk(uvgen, 0x900b);
	if (raw && raw->Value.size() >= 4)
	{
		uint32 chan;
		memcpy(&chan, nlVectorData(raw->Value), 4);
		out.MapChannel = (int)chan;
	}
	raw = findRawChunk(uvgen, 0x9002);
	if (raw && raw->Value.size() >= 4)
	{
		memcpy(&out.Flags, nlVectorData(raw->Value), 4);
		out.WrapU = (out.Flags & 0x1) != 0;
		out.WrapV = (out.Flags & 0x2) != 0;
	}

	// UV transform: the offset/tiling/angle values live on reference 0 = an old ParamBlock. Read
	// its static values; animated params (0x0200 marker) contribute nothing to the baked-in
	// matrix — the animation track drives the runtime matrix each frame (§10k). Defaults from
	// the SUVGen ctor (0 for offset/angle, 1 for tiling).
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(uvgen);
	if (rm && rm->nbReferences() > 0)
	{
		CSceneClass *pblock = dynamic_cast<CSceneClass *>(rm->getReference(0));
		if (pblock && pblock->classDesc()->superClassId() == SCLASS_PBLOCK)
		{
			std::map<sint32, OLDPBLOCK::SParam> params;
			OLDPBLOCK::readOldParamBlock(pblock, params);
			// Only override the default if the pblock actually carried a stored value for that
			// index (paramFloat returns 0 for missing, which would zero out tiling defaults).
			std::map<sint32, OLDPBLOCK::SParam>::const_iterator it;
			if ((it = params.find(0)) != params.end()) out.UOffset = it->second.V[0];
			if ((it = params.find(1)) != params.end()) out.VOffset = it->second.V[0];
			if ((it = params.find(2)) != params.end()) out.UTiling = it->second.V[0];
			if ((it = params.find(3)) != params.end()) out.VTiling = it->second.V[0];
			if ((it = params.find(4)) != params.end()) out.UAngle = it->second.V[0];
			if ((it = params.find(5)) != params.end()) out.VAngle = it->second.V[0];
			if ((it = params.find(6)) != params.end()) out.WAngle = it->second.V[0];
			// TODO: W Offset (index 8?) — deferred until a corpus asset exercises it.
			(void)texName;
		}
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
				if (!us.empty()) memcpy(&us[0], nlVectorData(raw->Value), us.size() * 2);
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
	if (!texmap) return nullptr;
	NLMISC::CClassId cid = texmap->classDesc()->classId();
	ITexture *pTexture = nullptr;
	CSceneClass *bmtexDelegate = nullptr;

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
		return nullptr;
	}

	// UV channel + matrix from the UVGen (delegate's reference 0)
	SUVGen uvgen;
	{
		CSceneClass *uvgenObj = nullptr;
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
	remap.UVMatrix = uvgenToMatrix3(uvgen);

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

	// Textures. bExportTextureMatrix may be an inline constant OR controller-backed (the artist
	// keyed the flag with an On/Off controller on some materials — combes_plateaux waterfalls);
	// the reference reads the live value at t=0, so resolve the controller here too — otherwise
	// enableUserTexMat is skipped and the material carries no user texture matrix for the anim to
	// drive (the "waterfall not moving" class).
	int bExportTexMatAnim = resolveNelBoolAt0(np.Blocks, NLB_MAIN, NLP_BEXPORTTEXTUREMATRIX, false) ? 1 : 0;
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
			// Bake in the StdUVGen static transform (§10z-cinq / this session): offset/tiling/
			// angle from the pblock. For animated params the runtime anim track drives the
			// matrix each frame; the baked value here reflects the pblock's own static value
			// (0 for animated offset in the corpus waterfalls). The uvMatrix2NelUVMatrix
			// convention flips the V axis to match NeL's UV origin (see helper above).
			material.setUserTexMat(i, uvMatrix2NelUVMatrix(materialDesc.UVMatrix));
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
static CRGBA convertColorF(const float c[3])
{
	float fR = c[0] * 255.f + 0.5f;
	float fG = c[1] * 255.f + 0.5f;
	float fB = c[2] * 255.f + 0.5f;
	clamp(fR, 0.f, 255.f);
	clamp(fG, 0.f, 255.f);
	clamp(fB, 0.f, 255.f);
	return CRGBA((uint8)fR, (uint8)fG, (uint8)fB);
}

static void dumpMaterialStructure(CSceneClass *mtl)
{
	CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mtl);
	fprintf(stderr, "MTLDUMP '%s' class %s, %u refs\n", materialName(mtl).c_str(),
	        mtl->classDesc()->classId().toString().c_str(), rm ? rm->nbReferences() : 0);
	for (uint i = 0; rm && i < rm->nbReferences(); ++i)
	{
		CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
		if (!r) { fprintf(stderr, "  ref %u: null\n", i); continue; }
		fprintf(stderr, "  ref %u: class %s sclass 0x%x '%s'\n", i,
		        r->classDesc()->classId().toString().c_str(),
		        (uint)r->classDesc()->superClassId(),
		        ucstring(r->classDesc()->displayName()).toUtf8().c_str());
		if (r->classDesc()->superClassId() == SCLASS_PBLOCK2)
		{
			std::vector<SPB2Block> blocks;
			// hack: wrap the single pblock via its owner walk — read it directly
			SPB2Block blk;
			if (readPB2Block(r, blk))
			{
				fprintf(stderr, "    pb2 blockId %u, %u params\n", blk.BlockId, (uint)blk.Params.size());
				for (uint k = 0; k < blk.Params.size(); ++k)
				{
					const SPB2Param &pp = blk.Params[k];
					fprintf(stderr, "      id %u type 0x%x %s F(%g %g %g %g) I=%d S='%s'",
					        pp.Id, pp.Type, pp.HasConstant ? "const" : "ctrl/ref",
					        pp.F[0], pp.F[1], pp.F[2], pp.F[3], pp.I, pp.S.c_str());
					if (pp.IsTab)
					{
						fprintf(stderr, " tab[");
						for (uint e = 0; e < pp.TabI.size(); ++e)
							fprintf(stderr, "%s%d", e ? " " : "", pp.TabI[e]);
						fprintf(stderr, "]");
					}
					fprintf(stderr, "\n");
				}
			}
		}
		// raw chunks of this ref
		{
			const CStorageContainer::TStorageObjectContainer &orphans = r->orphanedChunks();
			for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
			{
				CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
				fprintf(stderr, "    chunk 0x%04x sz %d:", it->first, raw ? (int)raw->Value.size() : -1);
				if (raw)
					for (uint b = 0; b < raw->Value.size() && b < 32; ++b)
						fprintf(stderr, " %02x", raw->Value[b]);
				fprintf(stderr, "\n");
			}
		}
		if (r->classDesc()->superClassId() != SCLASS_PBLOCK2)
		{
			// second level (shader/texmaps): show its refs + pblocks
			CReferenceMaker *rm2 = dynamic_cast<CReferenceMaker *>(r);
			for (uint j = 0; rm2 && j < rm2->nbReferences(); ++j)
			{
				CSceneClass *r2 = dynamic_cast<CSceneClass *>(rm2->getReference(j));
				if (!r2) { fprintf(stderr, "    ref %u.%u: null\n", i, j); continue; }
				fprintf(stderr, "    ref %u.%u: class %s sclass 0x%x '%s'\n", i, j,
				        r2->classDesc()->classId().toString().c_str(),
				        (uint)r2->classDesc()->superClassId(),
				        ucstring(r2->classDesc()->displayName()).toUtf8().c_str());
				if (r2->classDesc()->superClassId() == SCLASS_PBLOCK2)
				{
					SPB2Block blk;
					if (readPB2Block(r2, blk))
					{
						fprintf(stderr, "      pb2 blockId %u, %u params\n", blk.BlockId, (uint)blk.Params.size());
						for (uint k = 0; k < blk.Params.size(); ++k)
						{
							const SPB2Param &pp = blk.Params[k];
							fprintf(stderr, "        id %u type 0x%x %s F(%g %g %g %g) I=%d S='%s'\n",
							        pp.Id, pp.Type, pp.HasConstant ? "const" : "ctrl/ref",
							        pp.F[0], pp.F[1], pp.F[2], pp.F[3], pp.I, pp.S.c_str());
						}
					}
				}
			}
		}
	}
}

// Standard material (StdMat/StdMat2, (0x2,0)) — the reference's "not a nel material" branch.
// Storage: refs = { 0 = null, 1 = Texmaps (sclass 0x1080), 2 = shader (sclass 0x10b0, Blinn
// (0x38,0) in the corpus), then the material's own PB2s by blockId: 0 = std2_shader
// (shader_type, wire, twoSided, face_map, faceted), 1 = std2_extended (opacityType, opacity,
// ...), 2 = std2_sampling, 3 = std_maps (0 = mapEnables BOOL_TAB[24], 1 = maps TEXMAP_TAB[24]
// with per-element PB2 ref slots, 2 = amounts, 3 = texlock), 4 = std2_dynamics, then the
// sampler }. The shader's own PB2 (Blinn block 0): 0 = ambient, 1 = diffuse, 2 = specular,
// 3-5 locks, 6 = useSelfIllumColor, 7 = selfIllumAmount, 8 = selfIllumColor,
// 9 = specularLevel, 10 = glossiness, 11 = soften. The reference resolves these by param NAME
// through live ParamDefs; the (blockId, paramId) pairs are serialization-stable, which is what
// we key on headless (values cross-checked against the corpus references).
#define STD2_BLOCK_SHADER 0
#define STD2_BLOCK_EXTENDED 1
#define STD2_BLOCK_MAPS 3
#define STD2_SHADER_TWOSIDED 2
#define STD2_EXT_OPACITYTYPE 0
#define STD2_EXT_OPACITY 1
#define STD2_MAPS_ENABLES 0
#define STD2_MAPS_MAPS 1
#define SHDR_AMBIENT 0
#define SHDR_DIFFUSE 1
#define SHDR_SPECULAR 2
#define SHDR_USE_SELF_ILLUM_COLOR 6
#define SHDR_SELF_ILLUM_AMNT 7
#define SHDR_SELF_ILLUM_COLOR 8
#define SHDR_SPEC_LVL 9
#define SHDR_GLOSSINESS 10
// Max map slot indices
#define MAPSLOT_DI 1
#define MAPSLOT_SP 2
#define MAPSLOT_OP 6

static const SPB2Block *blockById(const std::vector<SPB2Block> &blocks, uint16 blockId)
{
	for (uint i = 0; i < blocks.size(); ++i)
		if (blocks[i].BlockId == blockId) return &blocks[i];
	return nullptr;
}

static void buildAStdMaterial(CMaterial &material, SMaterialInfo &materialInfo, CSceneClass *mtl, bool exportLighting)
{
	std::string mtlName = materialName(mtl);
	if (getenv("PMB_DUMP_MTL"))
		dumpMaterialStructure(mtl);

	bool isStdMat = mtl->classDesc()->classId() == CLASSID_STDMAT
		|| mtl->classDesc()->classId() == NLMISC::CClassId(0x00000001, 0x00000000);
	if (!isStdMat)
		fprintf(stderr, "WARNING: material '%s': unhandled material class %s (std fallback)\n",
		        mtlName.c_str(), mtl->classDesc()->classId().toString().c_str());

	// Own PB2 blocks + shader's PB2 block
	std::vector<SPB2Block> blocks;
	readObjectPB2Blocks(mtl, blocks);
	std::vector<SPB2Block> shaderBlocks;
	{
		CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mtl);
		for (uint i = 0; rm && i < rm->nbReferences(); ++i)
		{
			CSceneClass *r = dynamic_cast<CSceneClass *>(rm->getReference(i));
			if (r && r->classDesc()->superClassId() == 0x10b0)
			{
				readObjectPB2Blocks(r, shaderBlocks);
				break;
			}
		}
	}
	const SPB2Block *mapsBlk = blockById(blocks, STD2_BLOCK_MAPS);
	const SPB2Block *extBlk = blockById(blocks, STD2_BLOCK_EXTENDED);
	const SPB2Block *shBlk = blockById(blocks, STD2_BLOCK_SHADER);
	const SPB2Block *shaderBlk = shaderBlocks.empty() ? nullptr : &shaderBlocks[0];

	material.initLighted();

	// Enabled texmaps in the diffuse/opacity/specular slots
	CSceneClass *pDifTexmap = nullptr, *pOpaTexmap = nullptr, *pSpeTexmap = nullptr;
	if (mapsBlk)
	{
		const SPB2Param *en = nullptr, *maps = nullptr;
		{
			std::map<uint16, SPB2Param>::const_iterator it = mapsBlk->Params.find(STD2_MAPS_ENABLES);
			if (it != mapsBlk->Params.end()) en = &it->second;
			it = mapsBlk->Params.find(STD2_MAPS_MAPS);
			if (it != mapsBlk->Params.end()) maps = &it->second;
		}
		if (en && maps)
		{
			// TEXMAP_TAB inline element values are scene-container storage indices (-1 = none)
			for (uint slot = 0; slot < en->TabI.size() && slot < maps->TabI.size(); ++slot)
			{
				if (!en->TabI[slot]) continue;
				sint32 idx = maps->TabI[slot];
				if (idx < 0) continue;
				CSceneClass *tex = dynamic_cast<CSceneClass *>(mtl->container()->getByStorageIndex((sint32)idx));
				if (!tex) continue;
				if (getenv("PMB_DUMP_MTL"))
					fprintf(stderr, "  map slot %u -> storage #%d class %s\n", slot, idx,
					        tex->classDesc()->classId().toString().c_str());
				if (slot == MAPSLOT_DI) pDifTexmap = tex;
				else if (slot == MAPSLOT_OP) pOpaTexmap = tex;
				else if (slot == MAPSLOT_SP) pSpeTexmap = tex;
			}
		}
	}

	// NeL-scripted flags don't exist on standard materials: reference defaults apply
	// (bLightMap 0, bAlphaTest stays 1, force-z flags 0, bUnlighted/alpha/color vertex 0).
	const int bAlphaTest = 1;

	if (pSpeTexmap)
		material.setShader(CMaterial::Specular);
	else
		material.setShader(CMaterial::Normal);

	material.setStainedGlassWindow(false);
	material.setAlphaTest(false);
	material.setBlend(false);

	if (pDifTexmap)
	{
		NLMISC::CClassId tcid = pDifTexmap->classDesc()->classId();
		if (tcid == CLASSID_BMTEX || tcid == CLASSID_NEL_BMTEX)
		{
			SMaterialDesc texChannel;
			ITexture *pTexture = buildATexture(pDifTexmap, texChannel, false, mtlName);
			materialInfo.RemapChannel.resize(1);
			if (texChannel.IndexInMaxMaterial < 0)
			{
				materialInfo.RemapChannel[0].IndexInMaxMaterial = UVGEN_MISSING;
				materialInfo.RemapChannel[0].UVMatrix = Matrix3M::identity();
			}
			else
				materialInfo.RemapChannel[0] = texChannel;
			material.setTexture(0, pTexture);

			if (pOpaTexmap)
			{
				if (bAlphaTest)
				{
					material.setAlphaTest(true);
					material.setZWrite(true);
				}
				else
				{
					material.setBlend(true);
					material.setZWrite(false);
				}
			}

			if (texChannel.IndexInMaxMaterial >= 0)
				materialInfo.UVRouting[0] = 0;
		}
		else
		{
			fprintf(stderr, "WARNING: material '%s': diffuse texmap class %s not supported\n",
			        mtlName.c_str(), tcid.toString().c_str());
		}
	}

	if (pSpeTexmap)
		fprintf(stderr, "WARNING: material '%s': specular texture cube not implemented\n", mtlName.c_str());

	// Blend mode from opacityType
	{
		int opacityType = 0;
		if (extBlk)
		{
			std::map<uint16, SPB2Param>::const_iterator it = extBlk->Params.find(STD2_EXT_OPACITYTYPE);
			if (it != extBlk->Params.end() && it->second.HasConstant) opacityType = it->second.I;
		}
		if (opacityType == 0)
			material.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
		else
			material.setBlendFunc(CMaterial::srcalpha, CMaterial::one);
	}

	material.setZFunc(CMaterial::lessequal);
	material.setZBias(0.f);

	if (isStdMat && shaderBlk)
	{
		// Colors, self illumination and opacity from the shader's pblock
		float dif[3] = { 0.5f, 0.5f, 0.5f };
		const SPB2Param *pp;
		std::map<uint16, SPB2Param>::const_iterator it;
		if ((it = shaderBlk->Params.find(SHDR_DIFFUSE)) != shaderBlk->Params.end() && it->second.HasConstant)
			memcpy(dif, it->second.F, 12);
		CRGBA nelDiffuse = convertColorF(dif);
		float fOp = 0.0f;
		if (extBlk && (it = extBlk->Params.find(STD2_EXT_OPACITY)) != extBlk->Params.end() && it->second.HasConstant)
			fOp = it->second.F[0];
		float fA = fOp * 255.f + 0.5f;
		clamp(fA, 0.f, 255.f);
		nelDiffuse.A = (uint8)fA;
		material.setColor(nelDiffuse);

		if (fOp < 0.99f)
		{
			if (bAlphaTest)
			{
				material.setAlphaTest(true);
				material.setZWrite(true);
			}
			else
			{
				material.setBlend(true);
				material.setZWrite(false);
			}
		}

		CRGBA nelEmissive;
		int bSelfIllumColorOn = 0;
		if ((it = shaderBlk->Params.find(SHDR_USE_SELF_ILLUM_COLOR)) != shaderBlk->Params.end() && it->second.HasConstant)
			bSelfIllumColorOn = it->second.I;
		if (bSelfIllumColorOn)
		{
			float c[3] = { 0, 0, 0 };
			if ((it = shaderBlk->Params.find(SHDR_SELF_ILLUM_COLOR)) != shaderBlk->Params.end() && it->second.HasConstant)
				memcpy(c, it->second.F, 12);
			nelEmissive = convertColorF(c);
		}
		else
		{
			float amount = 0.0f;
			if ((it = shaderBlk->Params.find(SHDR_SELF_ILLUM_AMNT)) != shaderBlk->Params.end() && it->second.HasConstant)
				amount = it->second.F[0];
			float c[3] = { dif[0] * amount, dif[1] * amount, dif[2] * amount };
			nelEmissive = convertColorF(c);
		}

		float amb[3] = { 0, 0, 0 };
		if ((it = shaderBlk->Params.find(SHDR_AMBIENT)) != shaderBlk->Params.end() && it->second.HasConstant)
			memcpy(amb, it->second.F, 12);
		CRGBA nelAmbient = convertColorF(amb);

		float spe[3] = { 0, 0, 0 };
		if ((it = shaderBlk->Params.find(SHDR_SPECULAR)) != shaderBlk->Params.end() && it->second.HasConstant)
			memcpy(spe, it->second.F, 12);
		CRGBA nelSpecular = convertColorF(spe);

		float shininess = 0.0f;
		if ((it = shaderBlk->Params.find(SHDR_SPEC_LVL)) != shaderBlk->Params.end() && it->second.HasConstant)
			shininess = it->second.F[0];
		CRGBAF fColor = nelSpecular;
		fColor *= shininess;
		nelSpecular = fColor;

		shininess = 0.0f;
		if ((it = shaderBlk->Params.find(SHDR_GLOSSINESS)) != shaderBlk->Params.end() && it->second.HasConstant)
			shininess = it->second.F[0];
		shininess = (float)pow(2.0, shininess * 10.0) * 4.f;

		material.setLighting(true, nelEmissive, nelAmbient, nelDiffuse, nelSpecular, shininess);

		int bDoubleSided = 0;
		if (shBlk && (it = shBlk->Params.find(STD2_SHADER_TWOSIDED)) != shBlk->Params.end() && it->second.HasConstant)
			bDoubleSided = it->second.I;
		material.setDoubleSided(bDoubleSided != 0);
	}

	(void)exportLighting;
	materialInfo.MaterialName = mtlName;
}

static void buildAMaterial(CMaterial &material, SMaterialInfo &materialInfo, CSceneClass *mtl, bool exportLighting)
{
	if (isNelMaterial(mtl))
	{
		// Max 3-era NeL Material (script version 2, snowballs / early NeL): a thin wrapper
		// over a Standard material (ref 0) plus one NeL-flag ParamBlock2 (ref 1). Colors and
		// textures live on the Standard delegate — same structure the live Max exporter
		// resolves via getValueByNameUsingParamBlock2 sub-anim recursion. Script version 14+
		// (the Ryzom corpus) carries the multi-block nlbp/main/textures/slots table instead.
		uint32 version = 0, blockCount = 0;
		scriptedPluginVersion(mtl, version, blockCount);
		if (version > 0 && version < 14)
		{
			CReferenceMaker *rm = dynamic_cast<CReferenceMaker *>(mtl);
			CSceneClass *delegate = nullptr;
			if (rm && rm->nbReferences() > 0)
				delegate = dynamic_cast<CSceneClass *>(rm->getReference(0));
			if (delegate && (delegate->classDesc()->classId() == CLASSID_STDMAT
				|| delegate->classDesc()->classId() == NLMISC::CClassId(0x00000001, 0x00000000)
				|| delegate->classDesc()->classId() == NLMISC::CClassId(0x00000002, 0x00000000)))
			{
				buildAStdMaterial(material, materialInfo, delegate, exportLighting);
				// Prefer the NeL material's own name when present.
				std::string n = materialName(mtl);
				if (!n.empty()) materialInfo.MaterialName = n;
				return;
			}
			fprintf(stderr, "WARNING: material '%s': NeL material script version %u without Standard delegate; trying v14 table\n",
			        materialName(mtl).c_str(), version);
		}
		buildANelMaterial(material, materialInfo, mtl, exportLighting);
	}
	else
		buildAStdMaterial(material, materialInfo, mtl, exportLighting);
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
