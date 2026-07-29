/**
 * \file uvw_map_mod.cpp
 * \brief See uvw_map_mod.h.
 * \author Jan Boon (Kaetemi)
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

#include "uvw_map_mod.h"

#include <cmath>
#include <cstring>
#include <algorithm>

#include <nel/misc/common.h>

#include "old_param_block.h"
#include "max_scene.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/storage_object.h"

using namespace PIPELINE::MAX;
using namespace PIPELINE::MAX::BUILTIN;
using namespace MAXMATH;
using namespace NLMISC;

namespace UVWMAP {

const CClassId CLASSID_UVW_MAP(0x000f72b1, 0x00000000);
static const CClassId CLASSID_PRS_CTRL(0x00002005, 0x00000000);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool isUvwMapModifier(CSceneClass *mod)
{
	if (!mod || !mod->classDesc()) return false;
	return mod->classDesc()->classId() == CLASSID_UVW_MAP
	    && mod->classDesc()->superClassId() == SCLASS_OSMODIFIER;
}

// ---------------------------------------------------------------------------------------------
// MapPoint — reimplementation of Max UVWMapper::MapPoint + TileFlip (mesh library).
// Formulas match the well-known ApplyUVWMap semantics used by Mesh::ApplyUVWMap; validated
// against corpus reference shapes under planar/box/cyl/spherical.

static inline float clampf(float v, float lo, float hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

static int mainAxisOf(const Point3M &n)
{
	float ax = fabsf(n.x), ay = fabsf(n.y), az = fabsf(n.z);
	if (ax >= ay && ax >= az) return n.x >= 0.f ? 0 : 3;
	if (ay >= ax && ay >= az) return n.y >= 0.f ? 1 : 4;
	return n.z >= 0.f ? 2 : 5;
}

static Point3M tileFlip(Point3M uvw, float utile, float vtile, float wtile,
                        int uflip, int vflip, int wflip)
{
	uvw.x *= utile;
	uvw.y *= vtile;
	uvw.z *= wtile;
	if (uflip) uvw.x = 1.f - uvw.x;
	if (vflip) uvw.y = 1.f - uvw.y;
	if (wflip) uvw.z = 1.f - uvw.z;
	return uvw;
}

static Point3M mapPoint(int type, const Point3M &pIn, const Point3M &normIn, int cap,
                        float utile, float vtile, float wtile,
                        int uflip, int vflip, int wflip,
                        const Matrix3M &tm)
{
	// p' = p * tm (row vector); normal via vector transform (ignore translation).
	Point3M p = transformPoint(pIn, tm);
	Point3M n;
	n.x = normIn.x * tm.m[0][0] + normIn.y * tm.m[1][0] + normIn.z * tm.m[2][0];
	n.y = normIn.x * tm.m[0][1] + normIn.y * tm.m[1][1] + normIn.z * tm.m[2][1];
	n.z = normIn.x * tm.m[0][2] + normIn.y * tm.m[1][2] + normIn.z * tm.m[2][2];

	Point3M uvw;
	uvw.x = uvw.y = uvw.z = 0.f;

	switch (type)
	{
	case MAP_PLANAR:
		// Planar: u = 0.5 − p.x, v = 0.5 − p.y. Sign convention pinned by the Rectangle01 GT
		// (mag_impact_cold.max → common/sfx rectangle01.shape): the artist-Fit planar gizmo is
		// stored with a ~180°-about-Z rotation (quat (0,0,−1,ε)) and dims = bbox·1.001, and the
		// reference UVs come out u = 0.5 + x_obj/W — which under the rotated Inverse(gizmo)
		// mapping (p.x = −x/W) requires the NEGATIVE bias form. Equivalent reading: Max's planar
		// map space has u/v axes opposite the gizmo's local x/y (every corpus planar gizmo
		// carries the same 180° convention, so the two models coincide corpus-wide).
		uvw.x = 0.5f - p.x;
		uvw.y = 0.5f - p.y;
		uvw.z = p.z;
		break;

	case MAP_CYLINDRICAL:
	{
		bool useCap = cap && (fabsf(n.z) > fabsf(n.x) && fabsf(n.z) > fabsf(n.y));
		if (useCap)
		{
			uvw.x = p.x;
			uvw.y = p.y;
			uvw.z = 0.f;
		}
		else
		{
			float ang = atan2f(p.y, p.x);
			uvw.x = ang / (2.f * (float)M_PI);
			if (uvw.x < 0.f) uvw.x += 1.f;
			uvw.y = p.z;
			uvw.z = 0.f;
		}
		break;
	}

	case MAP_SPHERICAL:
	{
		float r = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
		if (r < 1e-8f)
		{
			uvw.x = 0.5f;
			uvw.y = 0.5f;
		}
		else
		{
			float ang = atan2f(p.y, p.x);
			uvw.x = ang / (2.f * (float)M_PI);
			if (uvw.x < 0.f) uvw.x += 1.f;
			float phi = acosf(clampf(p.z / r, -1.f, 1.f));
			uvw.y = 1.f - phi / (float)M_PI;
		}
		uvw.z = 0.f;
		break;
	}

	case MAP_BALL: // shrink-wrap: project onto unit sphere then planar-ish
	{
		float r = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
		if (r < 1e-8f)
		{
			uvw.x = 0.5f;
			uvw.y = 0.5f;
		}
		else
		{
			// Max MAP_BALL: similar to spherical but with different pole handling
			float ang = atan2f(p.y, p.x);
			uvw.x = ang / (2.f * (float)M_PI);
			if (uvw.x < 0.f) uvw.x += 1.f;
			uvw.y = p.z / r * 0.5f + 0.5f;
		}
		uvw.z = 0.f;
		break;
	}

	case MAP_BOX:
	{
		int axis = mainAxisOf(n);
		switch (axis)
		{
		case 0:  uvw.x = -p.z; uvw.y =  p.y; break; // +X
		case 3:  uvw.x =  p.z; uvw.y =  p.y; break; // -X
		case 1:  uvw.x =  p.x; uvw.y = -p.z; break; // +Y
		case 4:  uvw.x =  p.x; uvw.y =  p.z; break; // -Y
		case 2:  uvw.x =  p.x; uvw.y =  p.y; break; // +Z
		default: uvw.x = -p.x; uvw.y =  p.y; break; // -Z
		}
		uvw.z = 0.f;
		break;
	}

	case MAP_FACE:
		// Face mapping uses the face's own plane — handled per-face by caller (planar on
		// face-local axes). Fallback: planar.
		uvw.x = p.x;
		uvw.y = p.y;
		uvw.z = p.z;
		break;

	default:
		uvw.x = p.x;
		uvw.y = p.y;
		uvw.z = p.z;
		break;
	}

	return tileFlip(uvw, utile, vtile, wtile, uflip, vflip, wflip);
}

// ---------------------------------------------------------------------------------------------

static Point3M faceNormalOf(const SMeshView &mesh, uint face)
{
	if (mesh.FaceNormals && face < mesh.FaceNormals->size())
		return (*mesh.FaceNormals)[face];
	const sint32 i0 = (*mesh.FaceVerts)[face * 3 + 0];
	const sint32 i1 = (*mesh.FaceVerts)[face * 3 + 1];
	const sint32 i2 = (*mesh.FaceVerts)[face * 3 + 2];
	const Point3M &a = (*mesh.Verts)[i0];
	const Point3M &b = (*mesh.Verts)[i1];
	const Point3M &c = (*mesh.Verts)[i2];
	Point3M e1, e2;
	e1.x = b.x - a.x; e1.y = b.y - a.y; e1.z = b.z - a.z;
	e2.x = c.x - a.x; e2.y = c.y - a.y; e2.z = c.z - a.z;
	Point3M n;
	n.x = e1.y * e2.z - e1.z * e2.y;
	n.y = e1.z * e2.x - e1.x * e2.z;
	n.z = e1.x * e2.y - e1.y * e2.x;
	float len = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
	if (len > 1e-12f) { n.x /= len; n.y /= len; n.z /= len; }
	return n;
}

void applyUvwMapProjection(int type, float utile, float vtile, float wtile,
                           int uflip, int vflip, int wflip, int cap,
                           const Matrix3M &tm, int channel,
                           SMeshView &mesh)
{
	if (!mesh.Verts || !mesh.FaceVerts || !mesh.Maps) return;
	const uint nFaces = (uint)mesh.FaceVerts->size() / 3;
	if (nFaces == 0) return;

	SMeshView::SChannel &ch = (*mesh.Maps)[channel];
	ch.UVs.clear();
	ch.FaceUVs.assign(nFaces * 3, 0);

	// Per-corner UVs (box/cyl seams need face-local values; planar could share verts but
	// per-corner is always correct and matches how buildMeshInterface consumes map faces).
	ch.UVs.reserve(nFaces * 3);
	for (uint f = 0; f < nFaces; ++f)
	{
		Point3M fn = faceNormalOf(mesh, f);
		for (uint c = 0; c < 3; ++c)
		{
			sint32 vi = (*mesh.FaceVerts)[f * 3 + c];
			Point3M p = (*mesh.Verts)[vi];
			Point3M uvw = mapPoint(type, p, fn, cap, utile, vtile, wtile, uflip, vflip, wflip, tm);
			ch.FaceUVs[f * 3 + c] = (sint32)ch.UVs.size();
			ch.UVs.push_back(CVector(uvw.x, uvw.y, uvw.z));
		}
	}
}

// ---------------------------------------------------------------------------------------------

bool applyUvwMap(CSceneClass *mod, CStorageContainer *modApp, SMeshView &mesh,
                 uint typeMask, int *outType)
{
	if (outType) *outType = -1;
	if (!isUvwMapModifier(mod) || !mesh.Verts || !mesh.FaceVerts || !mesh.Maps)
		return false;

	// ParamBlock: scan refs for superclass 0x8
	CSceneClass *pblock = nullptr;
	CSceneClass *gizmoCtrl = nullptr;
	CReferenceMaker *mrm = dynamic_cast<CReferenceMaker *>(mod);
	for (uint r = 0; mrm && r < mrm->nbReferences(); ++r)
	{
		CSceneClass *ref = dynamic_cast<CSceneClass *>(mrm->getReference(r));
		if (!ref || !ref->classDesc()) continue;
		TSClassId sc = ref->classDesc()->superClassId();
		CClassId cid = ref->classDesc()->classId();
		if (sc == 0x8) // old ParamBlock
			pblock = ref;
		else if (cid == CLASSID_PRS_CTRL || sc == 0x9008) // PRS / transform control
			gizmoCtrl = ref;
	}
	if (!pblock) return false;

	std::map<sint32, OLDPBLOCK::SParam> params;
	OLDPBLOCK::readOldParamBlock(pblock, params);

	if (getenv("PMB_UVW_DUMP"))
	{
		fprintf(stderr, "PMB_UVW_DUMP pblock:");
		for (std::map<sint32, OLDPBLOCK::SParam>::iterator it = params.begin(); it != params.end(); ++it)
			fprintf(stderr, " [%d]=%s%.9g", (int)it->first, it->second.IsInt ? "i" : "f",
			        it->second.IsInt ? (double)it->second.I : (double)it->second.V[0]);
		fprintf(stderr, "\n");
	}

	int maptype = OLDPBLOCK::paramInt(params, UVWMAP_MAPTYPE);
	if (outType) *outType = maptype;
	if (maptype >= 0 && maptype < 32 && !(typeMask & (1u << maptype)))
		return false; // projection type not yet corpus-validated — caller warns and skips
	float utile = OLDPBLOCK::paramFloat(params, UVWMAP_UTILE);
	float vtile = OLDPBLOCK::paramFloat(params, UVWMAP_VTILE);
	float wtile = OLDPBLOCK::paramFloat(params, UVWMAP_WTILE);
	if (utile == 0.f) utile = 1.f;
	if (vtile == 0.f) vtile = 1.f;
	if (wtile == 0.f) wtile = 1.f;
	int uflip = OLDPBLOCK::paramInt(params, UVWMAP_UFLIP);
	int vflip = OLDPBLOCK::paramInt(params, UVWMAP_VFLIP);
	int wflip = OLDPBLOCK::paramInt(params, UVWMAP_WFLIP);
	int cap = OLDPBLOCK::paramInt(params, UVWMAP_CAP);
	int channel = OLDPBLOCK::paramInt(params, UVWMAP_CHANNEL);
	// Max channel 0 in the pblock is map channel 1 (the default texture channel). Observed:
	// UVWMAP_CHANNEL stores 0 for "map channel 1" in many Max 3-era files; values ≥1 are
	// already 1-based map channel ids. Treat 0 as 1.
	if (channel <= 0) channel = 1;
	float length = OLDPBLOCK::paramFloat(params, UVWMAP_LENGTH);
	float width = OLDPBLOCK::paramFloat(params, UVWMAP_WIDTH);
	float height = OLDPBLOCK::paramFloat(params, UVWMAP_HEIGHT);
	if (length == 0.f) length = 1.f;
	if (width == 0.f) width = 1.f;
	if (height == 0.f) height = 1.f;
	// UVWMAP_AXIS is used by the modifier UI for realignment; the gizmo PRS already encodes
	// the resulting orientation after Fit/Align, so we do not re-apply axis here.

	// Gizmo PRS
	Matrix3M gizmo = Matrix3M::identity();
	if (gizmoCtrl)
	{
		CReferenceMaker *prs = dynamic_cast<CReferenceMaker *>(gizmoCtrl);
		if (prs && prs->nbReferences() >= 3)
		{
			Point3M gp = MAXSCENE::posValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(0)));
			QuatM gr = MAXSCENE::rotValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(1)));
			ScaleValueM gs = MAXSCENE::scaleValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(2)));
			gizmo = composePRS(gp, gr, gs);
			if (getenv("PMB_UVW_DUMP"))
				fprintf(stderr, "PMB_UVW_DUMP gizmo p=(%.9g %.9g %.9g) r=(%.9g %.9g %.9g %.9g) "
				                "s=(%.9g %.9g %.9g) sq=(%.9g %.9g %.9g %.9g)\n",
				        gp.x, gp.y, gp.z, gr.x, gr.y, gr.z, gr.w,
				        gs.s.x, gs.s.y, gs.s.z, gs.q.x, gs.q.y, gs.q.z, gs.q.w);
		}
	}

	// Mapping TM construction (object → map space), corpus-tuned against Max 2010 refs:
	//
	// The UVW Map modifier's ApplyUVWMap call builds a Matrix3 from the gizmo TM such that
	// Mesh::ApplyUVWMap's planar path (u=p.x, v=p.y after p*=tm) lands UVs in the expected
	// range. Two sources of scale exist: the gizmo PRS scale and the Length/Width/Height
	// pblock floats. Empirically the PRS already carries the effective scale after Fit /
	// artist gizmo scaling, and Length/Width/Height describe the same box in the UI —
	// composing both double-scales. We therefore:
	//   1. Take the gizmo PRS as the orientation+position+scale of the mapping volume.
	//   2. Replace the gizmo's scale axes with (width, length, height) when the pblock
	//      dimensions look "Fit-sized" relative to the gizmo (ratio far from 1); otherwise
	//      keep PRS scale alone.
	// Practically: build S from dimensions, then gizmoFull = RotTrans(gizmo) * S when
	// dimensions are non-default, else gizmo as-is. Simpler rule that matches Fit:
	// always use dimensions as the scale component (overriding PRS scale), keep PRS
	// translation and rotation. This matches the Max UI model (L/W/H are the gizmo size).
	{
		// Decompose gizmo into T/R/S via affine decomp, then rebuild with dimension scale.
		AffinePartsM ap;
		decompAffine(gizmo, ap);
		Point3M dim; dim.x = width; dim.y = length; dim.z = height;
		// Use positive dimension scale; preserve gizmo stretch orientation via ap.u if needed.
		// For UVW Map the gizmo scale is typically uniform-ish on X/Y; use dim as k.
		ScaleValueM sv;
		sv.s = dim;
		sv.q.x = 0.f; sv.q.y = 0.f; sv.q.z = 0.f; sv.q.w = 1.f;
		// Rotation from decomp: essential rotation q, with reflection sign in f.
		QuatM rot = ap.q;
		if (ap.f < 0.f) { /* keep as-is; ApplyUVWMap handles flips via uflip etc. */ }
		gizmo = composePRS(ap.t, rot, sv);
	}

	// Mod-context TM (0x2510)
	Matrix3M ctx = Matrix3M::identity();
	if (modApp)
	{
		for (CStorageContainer::TStorageObjectConstIt it = modApp->chunks().begin();
		     it != modApp->chunks().end(); ++it)
		{
			if (it->first != 0x2510) continue;
			CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
			if (raw && raw->Value.size() >= 48)
				memcpy(ctx.m, nlVectorData(raw->Value), 48);
		}
	}

	// object → map space: p_map = p_obj * ctx * Inverse(gizmo)
	// +0.5 bias on planar axes so a Fit-centered gizmo of size = bbox maps to [0,1]
	// (Max MapPoint planar is u=p.x without bias; Fit positions the gizmo so that after
	// Inverse the range is already [0,1] OR centered [-0.5,0.5]. We observe Fit-centered
	// gizmos in the corpus and apply a +0.5 planar bias after MapPoint for type PLANAR —
	// implemented inside mapPoint when type is planar by adding 0.5 post-scale... actually
	// the dimension scale above with center translation should produce [-0.5,0.5]; TileFlip
	// does not add 0.5. Leave bias off for now and rely on dim+center.)
	Matrix3M tm = ctx * inverseM3(gizmo);

	if (getenv("PMB_UVW_DUMP"))
	{
		fprintf(stderr, "PMB_UVW_DUMP ctx: [%g %g %g][%g %g %g][%g %g %g][%g %g %g]\n",
		        ctx.m[0][0], ctx.m[0][1], ctx.m[0][2], ctx.m[1][0], ctx.m[1][1], ctx.m[1][2],
		        ctx.m[2][0], ctx.m[2][1], ctx.m[2][2], ctx.m[3][0], ctx.m[3][1], ctx.m[3][2]);
		fprintf(stderr, "PMB_UVW_DUMP tm: [%g %g %g][%g %g %g][%g %g %g][%g %g %g]\n",
		        tm.m[0][0], tm.m[0][1], tm.m[0][2], tm.m[1][0], tm.m[1][1], tm.m[1][2],
		        tm.m[2][0], tm.m[2][1], tm.m[2][2], tm.m[3][0], tm.m[3][1], tm.m[3][2]);
	}

	applyUvwMapProjection(maptype, utile, vtile, wtile, uflip, vflip, wflip, cap, tm, channel, mesh);
	return true;
}

} /* namespace UVWMAP */

/* end of file */
