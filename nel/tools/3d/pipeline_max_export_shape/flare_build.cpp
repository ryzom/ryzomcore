/**
 * \file flare_build.cpp
 * \brief See flare_build.h.
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
#include "flare_build.h"

#include <cstdio>

#include <nel/misc/rgba.h>
#include <nel/misc/path.h>
#include <nel/3d/flare_shape.h>
#include <nel/3d/texture_file.h>

#include "../pipeline_max/builtin/node_impl.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace NLMISC;
using namespace NL3D;
using namespace MAXMATH;
using namespace SCENELIB;
using MAXSCENE::decompMatrix;

namespace FLAREBUILD {

// The nel_flare scripted plugin (plugin_max/scripts/startup/nel_flare.ms) declares its PB2 params
// WITHOUT explicit `id:` tags, so param IDs equal declaration order (0-based, same convention
// pipeline_max_export_pacs_prim/main.cpp already validated for nel_pacs_box/cyl — max_geometry_
// formats §K.1). Declaration order in nel_flare.ms:
//   0..9   texFileName0..9  (STRING)
//   10..19 flareUsed0..9    (BOOL)
//   20..29 size0..9         (FLOAT)
//   30..39 pos0..9          (FLOAT)
//   40 ColorParam           (RGBA/color)
//   41 PersistenceParam     (FLOAT)
//   42 Spacing              (FLOAT)
//   43 AttenuationRange     (FLOAT)
//   44 Attenuable           (BOOL)
//   45 FirstFlareKeepSize   (BOOL)
//   46 HasDazzle            (BOOL)
//   47 DazzleColor          (RGBA)
//   48 DazzleAttenuationRange (FLOAT)
//   49 MaxViewDist          (FLOAT)
//   50 MaxViewDistRatio     (FLOAT)
//   51 occlusionTestMesh    (STRING)
//   52 occlusionTestMeshInheritScaleRot (BOOL)
//   53 sizeDisappear        (FLOAT)
//   54 angleDisappear       (FLOAT)
//   55 scaleWhenDisappear   (BOOL)
//   56 lookAtMode           (BOOL)

#define FLP_TEXFILENAME0 0
#define FLP_FLAREUSED0   10
#define FLP_SIZE0        20
#define FLP_POS0         30
#define FLP_COLORPARAM   40
#define FLP_PERSISTENCE  41
#define FLP_SPACING      42
#define FLP_ATTEN_RANGE  43
#define FLP_ATTENUABLE   44
#define FLP_FIRSTFLAREKEEPSIZE 45
#define FLP_HASDAZZLE    46
#define FLP_DAZZLE_COLOR 47
#define FLP_DAZZLE_ATTEN 48
#define FLP_MAXVIEWDIST  49
#define FLP_MAXVIEWDISTRATIO 50
#define FLP_OCCLTEST_MESH 51
#define FLP_OCCLTEST_INHERITSCALEROT 52
#define FLP_SIZEDISAPPEAR 53
#define FLP_ANGLEDISAPPEAR 54
#define FLP_SCALEWHENDISAPPEAR 55
#define FLP_LOOKATMODE   56

// Convert a float 0..1 RGB triple into CRGBA the way the reference does (mult by 255, no +0.5:
// per export_flare.cpp's `(uint) (255.f * col.x)` — plain truncation cast; do NOT use the rounded
// convertColor helper the material_build path uses).
static CRGBA colorFromF3(const float c[3])
{
	int r = (int)(255.f * c[0]);
	int g = (int)(255.f * c[1]);
	int b = (int)(255.f * c[2]);
	if (r < 0) r = 0; if (r > 255) r = 255;
	if (g < 0) g = 0; if (g > 255) g = 255;
	if (b < 0) b = 0; if (b > 255) b = 255;
	return CRGBA((uint8)r, (uint8)g, (uint8)b);
}

// Since the scripted flare object stores its PB2 across ONE reference (the plugin's own pblock
// declared with `parameters pblock`, the standard MAXScript scripted-plugin case), we look up
// every param in block 0 of the object's PB2 reference list.
static const SPB2Param *fp(const std::vector<SPB2Block> &blocks, uint16 id)
{
	return findPB2Param(blocks, 0, id);
}

NL3D::IShape *buildFlareShape(INode &node, SNodeTMCache &tmCache)
{
	std::string nodeName = SCENELIB::nodeName(node);
	CSceneClass *base = baseObjectOf(node, nullptr, nullptr);
	if (!base)
	{
		fprintf(stderr, "WARNING: flare '%s': no base object\n", nodeName.c_str());
		return nullptr;
	}
	std::vector<SPB2Block> blocks;
	readObjectPB2Blocks(base, blocks);
	if (blocks.empty())
	{
		fprintf(stderr, "WARNING: flare '%s': no ParamBlock2 on base object\n", nodeName.c_str());
		return nullptr;
	}

	// Build the shape and populate its state. Match the reference's ordering (`nlwarning FAILED
	// CFlareShape Spacing` on missing spacing) — spacing missing => still build (reference does
	// so, just warns), all other params fall back to CFlareShape ctor defaults.
	CFlareShape *fshape = new CFlareShape;

	// Color (block 0 param 40 — reference reads by name "ColorParam"). float RGB triple in [0,1].
	{
		const SPB2Param *p = fp(blocks, FLP_COLORPARAM);
		if (p && p->HasConstant)
			fshape->setColor(colorFromF3(p->F));
	}
	// Persistence, spacing, attenuation
	{
		const SPB2Param *p = fp(blocks, FLP_PERSISTENCE);
		if (p && p->HasConstant) fshape->setPersistence(p->F[0]);
	}
	{
		const SPB2Param *p = fp(blocks, FLP_SPACING);
		if (p && p->HasConstant) fshape->setFlareSpacing(p->F[0]);
		else fprintf(stderr, "WARNING: flare '%s': FAILED CFlareShape Spacing\n", nodeName.c_str());
	}
	{
		const SPB2Param *pa = fp(blocks, FLP_ATTENUABLE);
		if (pa && pa->HasConstant && pa->I != 0)
		{
			fshape->setAttenuable(true);
			const SPB2Param *pr = fp(blocks, FLP_ATTEN_RANGE);
			if (pr && pr->HasConstant) fshape->setAttenuationRange(pr->F[0]);
		}
	}
	{
		const SPB2Param *p = fp(blocks, FLP_FIRSTFLAREKEEPSIZE);
		bool val = (p && p->HasConstant) ? (p->I != 0) : false;
		fshape->setFirstFlareKeepSize(val);
	}
	// Dazzle
	{
		const SPB2Param *pd = fp(blocks, FLP_HASDAZZLE);
		if (pd && pd->HasConstant && pd->I != 0)
		{
			fshape->enableDazzle();
			const SPB2Param *pc = fp(blocks, FLP_DAZZLE_COLOR);
			if (pc && pc->HasConstant)
				fshape->setDazzleColor(colorFromF3(pc->F));
			const SPB2Param *pr = fp(blocks, FLP_DAZZLE_ATTEN);
			if (pr && pr->HasConstant) fshape->setDazzleAttenuationRange(pr->F[0]);
		}
	}
	// Max view dist + ratio
	{
		const SPB2Param *p = fp(blocks, FLP_MAXVIEWDIST);
		if (p && p->HasConstant) fshape->setMaxViewDist(p->F[0]);
	}
	{
		const SPB2Param *p = fp(blocks, FLP_MAXVIEWDISTRATIO);
		if (p && p->HasConstant) fshape->setMaxViewDistRatio(p->F[0]);
	}
	// Per-flare-slot size, pos, texture — reference loops uses < MaxFlareNum (== 10)
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
		{
			const SPB2Param *p = fp(blocks, (uint16)(FLP_SIZE0 + k));
			if (p && p->HasConstant) fshape->setSize(k, p->F[0]);
		}
		{
			const SPB2Param *p = fp(blocks, (uint16)(FLP_POS0 + k));
			if (p && p->HasConstant) fshape->setRelativePos(k, p->F[0]);
		}
		const SPB2Param *pu = fp(blocks, (uint16)(FLP_FLAREUSED0 + k));
		bool used = (pu && pu->HasConstant) ? (pu->I != 0) : false;
		if (used)
		{
			const SPB2Param *pt = fp(blocks, (uint16)(FLP_TEXFILENAME0 + k));
			if (pt && pt->HasConstant && !pt->S.empty())
			{
				// The reference calls CFile::getFilename to strip authored-era paths (drive +
				// R:\graphics\... prefix) down to the basename, since flare textures are
				// resolved by the runtime through the search paths, not by absolute path. Same
				// convention as material_build and water_build.
				CTextureFile *tf = new CTextureFile(NLMISC::CFile::getFilename(pt->S));
				fshape->setTexture(k, tf);
			}
			else
			{
				fshape->setTexture(k, nullptr);
			}
		}
		else
		{
			fshape->setTexture(k, nullptr);
		}
	}
	// Occlusion test mesh — a shape file name (basename only) + inherit-scale-rot flag
	{
		const SPB2Param *pt = fp(blocks, FLP_OCCLTEST_MESH);
		if (pt && pt->HasConstant)
			fshape->setOcclusionTestMeshName(NLMISC::CFile::getFilename(pt->S));
	}
	{
		const SPB2Param *p = fp(blocks, FLP_OCCLTEST_INHERITSCALEROT);
		if (p && p->HasConstant)
			fshape->setOcclusionTestMeshInheritScaleRot(p->I != 0);
	}
	// Disappear behaviour
	{
		const SPB2Param *p = fp(blocks, FLP_SCALEWHENDISAPPEAR);
		if (p && p->HasConstant) fshape->setScaleWhenDisappear(p->I != 0);
	}
	{
		const SPB2Param *p = fp(blocks, FLP_SIZEDISAPPEAR);
		if (p && p->HasConstant) fshape->setSizeDisappear(p->F[0]);
	}
	{
		const SPB2Param *p = fp(blocks, FLP_ANGLEDISAPPEAR);
		if (p && p->HasConstant) fshape->setAngleDisappear(p->F[0]);
	}
	// LookAt mode
	{
		const SPB2Param *p = fp(blocks, FLP_LOOKATMODE);
		if (p && p->HasConstant) fshape->setLookAtMode(p->I != 0);
	}

	// Default position — the reference exporter's buildFlare only sets DefaultPos (not
	// DefaultRotQuat / DefaultScale — the flare is a point-emitter, rotation/scale don't apply).
	// `getLocalMatrix(localTM, node, time)` in the reference computes nodeTM * inverse(parentTM),
	// same as MAXSCENE::getLocalMatrix here.
	{
		Matrix3M localTM = MAXSCENE::getLocalMatrix(node, tmCache);
		// Matrix3M rows 0..2 = basis vectors, row 3 = translation (row-vector convention).
		float tx = localTM.m[3][0], ty = localTM.m[3][1], tz = localTM.m[3][2];
		fshape->getDefaultPos()->setDefaultValue(CVector(tx, ty, tz));
	}
	return fshape;
}

} // namespace FLAREBUILD

/* end of file */
