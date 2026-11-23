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
#include "../pipeline_max/builtin/derived_object.h"
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
// Mapping. The mapping matrix takes object space into the map space of the gizmo: map to object is
// C = S * Ra * Rn * G * Inverse(ctx) (row vectors, the left factor first), tm = Inverse(C), p_map = p * tm.
// G is the gizmo PRS at tick 0 with its own scale, ctx the mod-context TM, Rn a pre-rotation about Z of the
// newer gizmo convention (an empty 0x0100 chunk on the modifier; pi for planar and box, pi/2 for cylindrical,
// spherical and ball, none for face), Ra the axis parameter (X: -pi/2 about Y, Y: +pi/2 about X, Z: none),
// and S the gizmo size (width, length, height), halved on x and y for planar and cylindrical and on all axes
// for spherical, ball and box. The face normal n goes through the linear part of tm. Corner values:
//   planar: (1/2 + p.x/2, 1/2 + p.y/2, p.z)
//   cylindrical: (1/2 + atan2(p.y, p.x) / 2pi, 1/2 + p.z, |p.xy|); on a face across the seam (corners more
//     than 1/2 apart in u) the corners more than 1/2 above the lowest take u - 1, each on a map vert of its
//     own; with the cap on, a face whose normal is mostly along z takes (1/2 + p.x/2, 1/2 + p.y/2) instead,
//     without the seam fix
//   spherical: u as cylindrical, v = 1/2 + asin(p.z / |p|) / pi (not validated: only water shapes use it)
//   box: by the main axis of n (ties go x, then y, then z): +X (1/2 + p.y/2, 1/2 + p.z/2), -X (1/2 - p.y/2,
//     1/2 + p.z/2), +Y (1/2 - p.x/2, 1/2 + p.z/2), -Y (1/2 + p.x/2, 1/2 + p.z/2), +Z (1/2 + p.x/2, 1/2 + p.y/2),
//     -Z (1/2 - p.x/2, 1/2 + p.y/2); the side table is validated on lightmap UVs, the 1/2 offset is inferred
//   face: tm is not used; the face's hidden edge (corners i and i + 1) is the diagonal of a unit square, its
//     end with the lower vertex index at (0, 0), the other at (1, 1), the third corner at (1, 0) or (0, 1)
// then u, v and w times their tiles, and 1 - value where flipped (flips are not seen in the corpus).
// The channel is parameter 13 (a 0 read as 1), or channel 0 when parameter 8 is 1. At face level only the
// selected faces are mapped and the others keep their corners. The map verts are numbered as the modifier
// numbers them, which Edit Mesh map records above index (pinned by the counts those record, and by the
// map verts an Edit Mesh copied from MA_appart_palmier): see the comment in applyUvwMap; box and face
// mapping give every corner a map vert of its own. Validated against the reference exports:
// planar (Rectangle01, BatA01 with axis Y), cylindrical (de_horizon_dome on two channels, one with a scaled
// gizmo, MA_appart_palmier on a face selection, TR_Piece_NPC_fond_marin with the cap), face
// (mag_impact_pierce, Mag_Impact_Slash), and the box sides on the lightmap UVs (sanctuaire_part10/11,
// Gen_Bt_Aub_entree).

static Point3M linearPart(const Point3M &n, const Matrix3M &tm)
{
	Point3M r;
	r.x = n.x * tm.m[0][0] + n.y * tm.m[1][0] + n.z * tm.m[2][0];
	r.y = n.x * tm.m[0][1] + n.y * tm.m[1][1] + n.z * tm.m[2][1];
	r.z = n.x * tm.m[0][2] + n.y * tm.m[1][2] + n.z * tm.m[2][2];
	return r;
}

static Point3M faceNormalOf(const Point3M &a, const Point3M &b, const Point3M &c)
{
	Point3M e1, e2, n;
	e1.x = b.x - a.x; e1.y = b.y - a.y; e1.z = b.z - a.z;
	e2.x = c.x - a.x; e2.y = c.y - a.y; e2.z = c.z - a.z;
	n.x = e1.y * e2.z - e1.z * e2.y;
	n.y = e1.z * e2.x - e1.x * e2.z;
	n.z = e1.x * e2.y - e1.y * e2.x;
	const float len = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
	if (len > 0.f) { n.x /= len; n.y /= len; n.z /= len; }
	return n;
}

static float cylinderU(const Point3M &p)
{
	return 0.5f + atan2f(p.y, p.x) / (2.f * (float)M_PI);
}

// A face across the seam (its corners more than half a turn apart in u): the corners more than half a turn
// above the lowest one take u - 1, each on a map vert of its own.
static void seamFix(Point3M uvw[3], bool own[3])
{
	const float minU = std::min(uvw[0].x, std::min(uvw[1].x, uvw[2].x));
	for (int c = 0; c < 3; ++c)
	{
		if (uvw[c].x - minU > 0.5f)
		{
			uvw[c].x -= 1.f;
			own[c] = true;
		}
	}
}

// The corner values of one face, before tiling. False when the face cannot be mapped (face mapping without
// one hidden edge). own[c] is set for a corner that takes a map vert of its own (across the seam).
static bool mapFace(int type, int cap, const Point3M p[3], const Point3M &n, const uint32 v[3], uint32 flags,
                    Point3M uvw[3], bool own[3])
{
	for (int c = 0; c < 3; ++c) { uvw[c].x = uvw[c].y = uvw[c].z = 0.f; own[c] = false; }
	switch (type)
	{
	case MAP_PLANAR:
		for (int c = 0; c < 3; ++c)
		{
			uvw[c].x = 0.5f + 0.5f * p[c].x;
			uvw[c].y = 0.5f + 0.5f * p[c].y;
			uvw[c].z = p[c].z;
		}
		return true;
	case MAP_CYLINDRICAL:
		if (cap && fabsf(n.z) > fabsf(n.x) && fabsf(n.z) > fabsf(n.y))
		{
			for (int c = 0; c < 3; ++c)
			{
				uvw[c].x = 0.5f + 0.5f * p[c].x;
				uvw[c].y = 0.5f + 0.5f * p[c].y;
			}
			return true;
		}
		for (int c = 0; c < 3; ++c)
		{
			uvw[c].x = cylinderU(p[c]);
			uvw[c].y = 0.5f + p[c].z;
			uvw[c].z = sqrtf(p[c].x * p[c].x + p[c].y * p[c].y);
		}
		seamFix(uvw, own);
		return true;
	case MAP_SPHERICAL:
		for (int c = 0; c < 3; ++c)
		{
			const float r = sqrtf(p[c].x * p[c].x + p[c].y * p[c].y + p[c].z * p[c].z);
			uvw[c].x = cylinderU(p[c]);
			uvw[c].y = r > 0.f ? 0.5f + asinf(std::max(-1.f, std::min(1.f, p[c].z / r))) / (float)M_PI : 0.5f;
		}
		seamFix(uvw, own);
		return true;
	case MAP_BOX:
	{
		const float ax = fabsf(n.x), ay = fabsf(n.y), az = fabsf(n.z);
		const int side = (ax >= ay && ax >= az) ? (n.x >= 0.f ? 0 : 3)
			: ((ay >= az) ? (n.y >= 0.f ? 1 : 4) : (n.z >= 0.f ? 2 : 5));
		for (int c = 0; c < 3; ++c)
		{
			const Point3M &q = p[c];
			switch (side)
			{
			case 0: uvw[c].x = 0.5f + 0.5f * q.y; uvw[c].y = 0.5f + 0.5f * q.z; break; // +X
			case 3: uvw[c].x = 0.5f - 0.5f * q.y; uvw[c].y = 0.5f + 0.5f * q.z; break; // -X
			case 1: uvw[c].x = 0.5f - 0.5f * q.x; uvw[c].y = 0.5f + 0.5f * q.z; break; // +Y
			case 4: uvw[c].x = 0.5f + 0.5f * q.x; uvw[c].y = 0.5f + 0.5f * q.z; break; // -Y
			case 2: uvw[c].x = 0.5f + 0.5f * q.x; uvw[c].y = 0.5f + 0.5f * q.y; break; // +Z
			default: uvw[c].x = 0.5f - 0.5f * q.x; uvw[c].y = 0.5f + 0.5f * q.y; break; // -Z
			}
		}
		return true;
	}
	case MAP_FACE:
	{
		int hidden = -1;
		for (int e = 0; e < 3; ++e)
		{
			if (flags & (1u << e)) continue;
			if (hidden >= 0) return false; // more than one hidden edge
			hidden = e;
		}
		if (hidden < 0) return false;
		const int i = hidden, j = (hidden + 1) % 3, k = (hidden + 2) % 3;
		if (v[j] < v[i])
		{
			uvw[j].x = 0.f; uvw[j].y = 0.f;
			uvw[i].x = 1.f; uvw[i].y = 1.f;
			uvw[k].x = 1.f; uvw[k].y = 0.f;
		}
		else
		{
			uvw[i].x = 0.f; uvw[i].y = 0.f;
			uvw[j].x = 1.f; uvw[j].y = 1.f;
			uvw[k].x = 0.f; uvw[k].y = 1.f;
		}
		return true;
	}
	default:
		return false;
	}
}

// ---------------------------------------------------------------------------------------------

bool applyUvwMap(CSceneClass *mod, CStorageContainer *modApp, SMeshView &mesh,
                 uint typeMask, int *outType, int *outChannel, uint *outUnmapped)
{
	if (outType) *outType = -1;
	if (outChannel) *outChannel = -1;
	if (outUnmapped) *outUnmapped = 0;
	if (!isUvwMapModifier(mod) || !mesh.Verts || !mesh.FaceVerts || !mesh.Maps)
		return false;

	// ParamBlock and gizmo: scan the references for the old ParamBlock (superclass 0x8) and the PRS
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
	if (params.find(UVWMAP_MAPTYPE) != params.end() && outType)
		*outType = OLDPBLOCK::paramInt(params, UVWMAP_MAPTYPE);
	for (sint32 i = 0; i <= UVWMAP_AXIS; ++i)
		if (params.find(i) == params.end())
			return false; // an animated or missing parameter: the caller warns

	const int maptype = OLDPBLOCK::paramInt(params, UVWMAP_MAPTYPE);
	const bool implemented = maptype == MAP_PLANAR || maptype == MAP_CYLINDRICAL || maptype == MAP_SPHERICAL
		|| maptype == MAP_BOX || maptype == MAP_FACE;
	if (!implemented || !(typeMask & (1u << maptype)))
		return false; // projection type not supported (ball, the newer types): the caller warns
	float utile = OLDPBLOCK::paramFloat(params, UVWMAP_UTILE);
	float vtile = OLDPBLOCK::paramFloat(params, UVWMAP_VTILE);
	float wtile = OLDPBLOCK::paramFloat(params, UVWMAP_WTILE);
	const int uflip = OLDPBLOCK::paramInt(params, UVWMAP_UFLIP);
	const int vflip = OLDPBLOCK::paramInt(params, UVWMAP_VFLIP);
	const int wflip = OLDPBLOCK::paramInt(params, UVWMAP_WFLIP);
	const int cap = OLDPBLOCK::paramInt(params, UVWMAP_CAP);
	const int kind = OLDPBLOCK::paramInt(params, UVWMAP_CHANNEL);
	int mapChannel = OLDPBLOCK::paramInt(params, UVWMAP_MAP_CHANNEL);
	if (mapChannel <= 0) mapChannel = 1;
	const int channel = kind != 0 ? 0 : mapChannel;
	if (outChannel) *outChannel = channel;
	float length = OLDPBLOCK::paramFloat(params, UVWMAP_LENGTH);
	float width = OLDPBLOCK::paramFloat(params, UVWMAP_WIDTH);
	float height = OLDPBLOCK::paramFloat(params, UVWMAP_HEIGHT);
	if (maptype != MAP_FACE && (length == 0.f || width == 0.f || height == 0.f))
	{
		// the mapping matrix would not invert
		fprintf(stderr, "WARNING: UVW Map with a zero size (%g, %g, %g); 1 used for it\n", width, length, height);
		if (length == 0.f) length = 1.f;
		if (width == 0.f) width = 1.f;
		if (height == 0.f) height = 1.f;
	}
	if (uflip || vflip || wflip)
		fprintf(stderr, "WARNING: UVW Map flip taken as 1 - value after the tiling; not validated\n");
	const int axis = OLDPBLOCK::paramInt(params, UVWMAP_AXIS);

	// the newer gizmo convention: an empty 0x0100 chunk on the modifier; an empty 0x0110 marks the newer fit,
	// which with a real-world map size would move planar maps (not seen)
	bool newStyle = false;
	const CStorageContainer::TStorageObjectContainer &orphans = mod->orphanedChunks();
	for (CStorageContainer::TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		const CStorageRaw *raw = dynamic_cast<const CStorageRaw *>(it->second);
		if (raw && raw->Value.empty() && it->first == 0x0100)
			newStyle = true;
		if (raw && raw->Value.empty() && it->first == 0x0110)
			fprintf(stderr, "WARNING: UVW Map with the newer fit marker; a real-world map size is not evaluated\n");
	}

	// gizmo PRS with its own scale
	Matrix3M gizmo = Matrix3M::identity();
	if (CReferenceMaker *prs = dynamic_cast<CReferenceMaker *>(gizmoCtrl))
	{
		if (prs->nbReferences() >= 3)
		{
			Point3M gp = MAXSCENE::posValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(0)));
			QuatM gr = MAXSCENE::rotValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(1)));
			ScaleValueM gs = MAXSCENE::scaleValueAt0(dynamic_cast<CSceneClass *>(prs->getReference(2)));
			gizmo = composePRS(gp, gr, gs);
			if (getenv("PMB_UVW_DUMP"))
				fprintf(stderr, "PMB_UVW_DUMP gizmo p=(%.9g %.9g %.9g) r=(%.9g %.9g %.9g %.9g) "
				                "s=(%.9g %.9g %.9g)\n", gp.x, gp.y, gp.z, gr.x, gr.y, gr.z, gr.w,
				        gs.s.x, gs.s.y, gs.s.z);
		}
	}
	Matrix3M ctx = Matrix3M::identity();
	CDerivedObject::modAppContextTM(modApp, &ctx.m[0][0]);

	Matrix3M s = Matrix3M::identity();
	if (maptype != MAP_FACE)
	{
		const bool halfZ = maptype == MAP_SPHERICAL || maptype == MAP_BALL || maptype == MAP_BOX;
		s.m[0][0] = width * 0.5f;
		s.m[1][1] = length * 0.5f;
		s.m[2][2] = halfZ ? height * 0.5f : height;
	}
	Matrix3M ra = Matrix3M::identity();
	if (axis == 0) ra = rotateAxisM3(ra, 1, -M_PI * 0.5);
	else if (axis == 1) ra = rotateAxisM3(ra, 0, M_PI * 0.5);
	Matrix3M rn = Matrix3M::identity();
	if (newStyle && maptype != MAP_FACE)
		rn = rotateAxisM3(rn, 2, (maptype == MAP_PLANAR || maptype == MAP_BOX) ? M_PI : M_PI * 0.5);
	const Matrix3M tm = inverseM3(s * ra * rn * gizmo * inverseM3(ctx));

	// The map verts. Mapping the whole mesh (or a selection without an incoming channel) gives one map vert
	// per vertex and map faces parallel to the faces; a face selection keeps the incoming channel and gives
	// the selected faces one new map vert per vertex they use, numbered in face order after the incoming
	// ones. A corner whose value differs from the one its map vert already took gets a map vert of its own.
	// Map verts no face uses any more are dropped at the end, keeping the order of the others.
	const uint nFaces = (uint)mesh.FaceVerts->size() / 3;
	const uint nVerts = (uint)mesh.Verts->size();
	const bool bySel = mesh.FaceSel && mesh.FaceSel->size() == nFaces;
	SMeshView::SChannel &ch = (*mesh.Maps)[channel];
	std::vector<bool> valueSet;
	if (!bySel || ch.FaceUVs.size() != (size_t)nFaces * 3)
	{
		if (bySel)
			fprintf(stderr, "WARNING: UVW Map on a face selection without an incoming channel %d; the other faces share "
			        "the map verts of their vertices, (0, 0, 0) where no selected face maps one (not validated)\n", channel);
		ch.UVs.assign(nVerts, CVector(0.f, 0.f, 0.f));
		ch.FaceUVs.assign(mesh.FaceVerts->begin(), mesh.FaceVerts->end());
		valueSet.assign(nVerts, false);
	}
	else
	{
		valueSet.assign(ch.UVs.size(), true);
		std::vector<sint32> vmap(nVerts, -1);
		for (uint f = 0; f < nFaces; ++f)
		{
			if (!(*mesh.FaceSel)[f])
				continue;
			for (uint c = 0; c < 3; ++c)
			{
				const sint32 v = (*mesh.FaceVerts)[f * 3 + c];
				if (v < 0 || (uint)v >= nVerts)
					continue;
				if (vmap[v] < 0)
				{
					vmap[v] = (sint32)ch.UVs.size();
					ch.UVs.push_back(CVector(0.f, 0.f, 0.f));
					valueSet.push_back(false);
				}
				ch.FaceUVs[f * 3 + c] = vmap[v];
			}
		}
	}
	uint unmapped = 0;
	for (uint f = 0; f < nFaces; ++f)
	{
		if (bySel && !(*mesh.FaceSel)[f])
			continue;
		Point3M p[3], obj[3];
		uint32 v[3];
		for (uint c = 0; c < 3; ++c)
		{
			v[c] = (uint32)(*mesh.FaceVerts)[f * 3 + c];
			obj[c] = (*mesh.Verts)[v[c]];
			p[c] = transformPoint(obj[c], tm);
		}
		const Point3M n = linearPart(faceNormalOf(obj[0], obj[1], obj[2]), tm);
		const uint32 flags = mesh.FaceFlags && f < mesh.FaceFlags->size() ? (*mesh.FaceFlags)[f] : 7;
		Point3M uvw[3];
		bool own[3];
		if (!mapFace(maptype, cap, p, n, v, flags, uvw, own))
		{
			++unmapped;
			for (uint c = 0; c < 3; ++c) { uvw[c].x = uvw[c].y = uvw[c].z = 0.f; own[c] = false; }
		}
		if (maptype == MAP_BOX || maptype == MAP_FACE)
			own[0] = own[1] = own[2] = true; // a map vert per corner
		for (uint c = 0; c < 3; ++c)
		{
			uvw[c].x *= utile;
			uvw[c].y *= vtile;
			uvw[c].z *= wtile;
			if (uflip) uvw[c].x = 1.f - uvw[c].x;
			if (vflip) uvw[c].y = 1.f - uvw[c].y;
			if (wflip) uvw[c].z = 1.f - uvw[c].z;
			const CVector value(uvw[c].x, uvw[c].y, uvw[c].z);
			sint32 &t = ch.FaceUVs[f * 3 + c];
			if (t < 0 || (uint)t >= ch.UVs.size())
				continue;
			if (!valueSet[t] && !own[c])
			{
				ch.UVs[t] = value;
				valueSet[t] = true;
			}
			else if (own[c] || !(ch.UVs[t] == value))
			{
				t = (sint32)ch.UVs.size();
				ch.UVs.push_back(value);
				valueSet.push_back(true);
			}
		}
	}

	// drop the map verts no face uses
	{
		std::vector<sint32> remap(ch.UVs.size(), -1);
		for (uint i = 0; i < ch.FaceUVs.size(); ++i)
			if (ch.FaceUVs[i] >= 0 && (uint)ch.FaceUVs[i] < ch.UVs.size())
				remap[ch.FaceUVs[i]] = 0;
		uint kept = 0;
		for (uint i = 0; i < ch.UVs.size(); ++i)
		{
			if (remap[i] < 0)
				continue;
			remap[i] = (sint32)kept;
			ch.UVs[kept++] = ch.UVs[i];
		}
		ch.UVs.resize(kept);
		for (uint i = 0; i < ch.FaceUVs.size(); ++i)
			if (ch.FaceUVs[i] >= 0 && (uint)ch.FaceUVs[i] < remap.size())
				ch.FaceUVs[i] = remap[ch.FaceUVs[i]];
	}
	if (outUnmapped) *outUnmapped = unmapped;
	return true;
}

} /* namespace UVWMAP */

/* end of file */
