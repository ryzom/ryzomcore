/**
 * \file parametric_mesh.cpp
 * \brief See parametric_mesh.h.
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7 (1M context)
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

#include "parametric_mesh.h"

#include <cmath>
#include <algorithm>

#include <nel/misc/common.h>

namespace PRIMMESH {

const NLMISC::CClassId CLASSID_BOX(0x00000010, 0x00000000);
const NLMISC::CClassId CLASSID_CYLINDER(0x00000012, 0x00000000);
const NLMISC::CClassId CLASSID_SPHERE(0x00000011, 0x00000000);
const NLMISC::CClassId CLASSID_PLANE(0x081f1dfc, 0x77566f65);

// Box "generate mapping coords" UV for one vertex, by face type. Each face projects onto its two
// in-plane axes; validated against ~/shape_export_dataset primuv_box (and generalises to
// primuv_box_multiseg because the projection is by vertex coordinate, not face index). w=width,
// l=length, h=height (the Box pblock params; signed — the formula tracks how positions are built).
enum TBoxUvFace { BX_BOTTOM, BX_TOP, BX_SIDE_Y, BX_SIDE_X };
static inline NLMISC::CVector boxUvFor(const NLMISC::CVector &p, TBoxUvFace ft, float l, float w, float h)
{
	float u = 0.0f, v = 0.0f;
	switch (ft)
	{
		case BX_BOTTOM:  u = 0.5f - p.x / w; v = 0.5f + p.y / l; break; // -Z face (U mirrored vs top)
		case BX_TOP:     u = 0.5f + p.x / w; v = 0.5f + p.y / l; break;
		case BX_SIDE_Y:  u = 0.5f + p.x / w; v = p.z / h; break;       // ±Y faces share this mapping
		case BX_SIDE_X:  u = 0.5f + p.y / l; v = p.z / h; break;       // ±X faces share this mapping
	}
	return NLMISC::CVector(u, v, 0.0f);
}

bool buildParametricMesh(const NLMISC::CClassId &cid,
                         const std::map<sint32, OLDPBLOCK::SParam> &params,
                         std::vector<NLMISC::CVector> &verts,
                         std::vector<SPrimTri> &tris,
                         std::vector<NLMISC::CVector> *uvVerts)
{
	using OLDPBLOCK::paramFloat;
	using OLDPBLOCK::paramInt;

	// Box (0x10, 0): params 0/1/2 = length/width/height, 3/4/5 = w/l/h segments.
	if (cid == CLASSID_BOX)
	{
		float l = paramFloat(params, 0), w = paramFloat(params, 1), h = paramFloat(params, 2);
		sint ws = std::max(1, paramInt(params, 3));
		sint ls = std::max(1, paramInt(params, 4));
		sint hs = std::max(1, paramInt(params, 5));
		float dx = w / ws, dy = l / ls, dz = h / hs;
		float x0 = -w / 2.0f, y0 = -l / 2.0f;
		sint row = ws + 1;
		sint gridN = (ws + 1) * (ls + 1);
		verts.clear();
		for (sint iy = 0; iy <= ls; ++iy)
			for (sint ix = 0; ix <= ws; ++ix)
				verts.push_back(NLMISC::CVector(x0 + dx * ix, y0 + dy * iy, 0.0f));
		for (sint iy = 0; iy <= ls; ++iy)
			for (sint ix = 0; ix <= ws; ++ix)
				verts.push_back(NLMISC::CVector(x0 + dx * ix, y0 + dy * iy, h));
		std::vector<sint> perIx, perIy;
		for (sint ix = 0; ix < ws; ++ix) { perIx.push_back(ix); perIy.push_back(0); }
		for (sint iy = 0; iy < ls; ++iy) { perIx.push_back(ws); perIy.push_back(iy); }
		for (sint ix = ws; ix > 0; --ix) { perIx.push_back(ix); perIy.push_back(ls); }
		for (sint iy = ls; iy > 0; --iy) { perIx.push_back(0); perIy.push_back(iy); }
		sint perN = (sint)perIx.size();
		for (sint r = 1; r < hs; ++r)
			for (sint k = 0; k < perN; ++k)
				verts.push_back(NLMISC::CVector(x0 + dx * perIx[k], y0 + dy * perIy[k], dz * r));
		#define BOX_RING(v, k) ((v) == 0 ? perIy[(k) % perN] * row + perIx[(k) % perN] \
			: ((v) == hs ? gridN + perIy[(k) % perN] * row + perIx[(k) % perN] \
			: 2 * gridN + ((v) - 1) * perN + ((k) % perN)))
		tris.clear();
		if (uvVerts) uvVerts->clear();
		bool flip = h < 0.0f;
		// Per-side (matId, smoothing-group) tables — corpus-validated against
		// ~/shape_export_dataset/manifest.txt (plain_box, primuv_box, primuv_box_multiseg): Max's
		// Box primitive assigns a distinct matId + smoothing group per face type.
		//   Bottom (z=0):  matId 1, sg 0x02  (MAXScript mid=2)
		//   Top (z=h):     matId 0, sg 0x04  (mid=1)
		//   -Y side:       matId 4, sg 0x08  (mid=5)
		//   +X side:       matId 3, sg 0x10  (mid=4)
		//   +Y side:       matId 5, sg 0x20  (mid=6)
		//   -X side:       matId 2, sg 0x40  (mid=3)
		// Perimeter side order in the generator is (-y, +x, +y, -x) — same order as the sideStart
		// table below — so side-index k (0..3) → (matId, sg) = sideAttr[k].
		static const uint32 sideMat[4] = { 4, 3, 5, 2 };
		static const uint32 sideSg[4]  = { 0x08, 0x10, 0x20, 0x40 };
		uint32 curMat = 0;
		uint32 curSg = 0;
		// FT = the box-mapping face type for the UV; emitted per corner in the SAME order the geom
		// tri is pushed (flip-aware) so map-face t's corner c is (*uvVerts)[t*3 + c].
		#define BOX_TRI(a, b, c, FT) do { \
			sint _a = (a), _b = (b), _c = (c); \
			sint _o0 = flip ? _b : _a, _o1 = flip ? _a : _b; \
			SPrimTri _t = { { (uint32)_o0, (uint32)_o1, (uint32)_c }, curMat, curSg }; \
			tris.push_back(_t); \
			if (uvVerts) { \
				uvVerts->push_back(boxUvFor(verts[_o0], FT, l, w, h)); \
				uvVerts->push_back(boxUvFor(verts[_o1], FT, l, w, h)); \
				uvVerts->push_back(boxUvFor(verts[_c], FT, l, w, h)); \
			} \
		} while (0)
		// Negative height inverts the cap roles: the z=0 grid is now functionally the TOP face
		// (facing +Z after the winding flip) and the z=h grid is the BOTTOM. Swap the face-type
		// used for UV generation AND the matId/sg pair on each so a wall-mounted upside-down box
		// (h < 0, box01.max) uv-maps the same as its right-side-up sibling. Validated against
		// box01.shape (h < 0): the z=0 face's UVs now match the BX_TOP formula ref carries.
		TBoxUvFace ftLow = flip ? BX_TOP : BX_BOTTOM;
		TBoxUvFace ftHigh = flip ? BX_BOTTOM : BX_TOP;
		uint32 matLow = flip ? 0 : 1;
		uint32 sgLow = flip ? 0x04 : 0x02;
		uint32 matHigh = flip ? 1 : 0;
		uint32 sgHigh = flip ? 0x02 : 0x04;
		curMat = matLow; curSg = sgLow;
		for (sint iy = 0; iy < ls; ++iy)
			for (sint ix = 0; ix < ws; ++ix)
			{
				sint a = iy * row + ix;
				BOX_TRI(a, a + row, a + row + 1, ftLow);
				BOX_TRI(a + row + 1, a + 1, a, ftLow);
			}
		curMat = matHigh; curSg = sgHigh;
		for (sint iy = 0; iy < ls; ++iy)
			for (sint ix = 0; ix < ws; ++ix)
			{
				sint a = gridN + iy * row + ix;
				BOX_TRI(a, a + 1, a + 1 + row, ftHigh);
				BOX_TRI(a + 1 + row, a + row, a, ftHigh);
			}
		sint sideStart[4] = { 0, ws, ws + ls, 2 * ws + ls };
		sint sideLen[4] = { ws, ls, ws, ls };
		for (sint sd = 0; sd < 4; ++sd)
		{
			curMat = sideMat[sd];
			curSg = sideSg[sd];
			// sd 0/2 = -Y/+Y faces (span x,z); sd 1/3 = +X/-X faces (span y,z).
			TBoxUvFace ft = (sd == 0 || sd == 2) ? BX_SIDE_Y : BX_SIDE_X;
			for (sint v = 0; v < hs; ++v)
				for (sint k = 0; k < sideLen[sd]; ++k)
				{
					sint pk = sideStart[sd] + k;
					sint lo1 = BOX_RING(v, pk), lo2 = BOX_RING(v, pk + 1);
					sint up1 = BOX_RING(v + 1, pk), up2 = BOX_RING(v + 1, pk + 1);
					BOX_TRI(lo1, lo2, up2, ft);
					BOX_TRI(up2, up1, lo1, ft);
				}
		}
		#undef BOX_TRI
		#undef BOX_RING
		return true;
	}

	// Cylinder (0x12, 0): params 0/1 = radius/height, 2/3 = height/cap segments, 4 = sides.
	if (cid == CLASSID_CYLINDER)
	{
		float r = paramFloat(params, 0), h = paramFloat(params, 1);
		sint hs = std::max(1, paramInt(params, 2));
		sint sides = std::max(3, paramInt(params, 4));
		float dz = h / hs;
		verts.clear();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, 0.0f));
		for (sint v = 0; v <= hs; ++v)
			for (sint k = 0; k < sides; ++k)
			{
				double a = 2.0 * NLMISC::Pi * k / sides;
				verts.push_back(NLMISC::CVector(r * (float)cos(a), r * (float)sin(a), dz * v));
			}
		uint32 tc = (uint32)verts.size();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, h));
		#define CYL_RING(v, k) (1 + (v) * sides + ((k) % sides))
		tris.clear();
		// Cylinder matId/sg tables (~/shape_export_dataset primuv_cyl):
		//   bottom cap: matId 1, sg 0x01 (mid=2)
		//   sides:      matId 2, sg 0x08 (mid=3, all sides share one smoothing group so a cylinder
		//                                  renders smooth around its perimeter)
		//   top cap:    matId 0, sg 0x01 (mid=1)
		uint32 curMat = 0;
		uint32 curSg = 0;
		#define CYL_TRI(a, b, c) { SPrimTri t = { { (uint32)(a), (uint32)(b), (uint32)(c) }, curMat, curSg }; tris.push_back(t); }
		curMat = 1; curSg = 0x01;
		for (sint k = 0; k < sides; ++k)
			CYL_TRI(0, CYL_RING(0, k + 1), CYL_RING(0, k))
		curMat = 2; curSg = 0x08;
		for (sint v = 0; v < hs; ++v)
			for (sint k = 0; k < sides; ++k)
			{
				CYL_TRI(CYL_RING(v, k), CYL_RING(v + 1, k + 1), CYL_RING(v + 1, k))
				CYL_TRI(CYL_RING(v, k), CYL_RING(v, k + 1), CYL_RING(v + 1, k + 1))
			}
		curMat = 0; curSg = 0x01;
		for (sint k = 0; k < sides; ++k)
			CYL_TRI(tc, CYL_RING(hs, k), CYL_RING(hs, k + 1))
		#undef CYL_TRI
		#undef CYL_RING
		return true;
	}

	// Sphere (0x11, 0): params 0/1 = radius/segments.
	if (cid == CLASSID_SPHERE)
	{
		float r = paramFloat(params, 0);
		sint segs = std::max(4, paramInt(params, 1));
		sint rows = segs / 2;
		verts.clear();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, r));
		for (sint i = 1; i < rows; ++i)
		{
			double phi = NLMISC::Pi * i / rows;
			float z = r * (float)cos(phi);
			float rr = r * (float)sin(phi);
			for (sint k = 0; k < segs; ++k)
			{
				double a = NLMISC::Pi / 2.0 + 2.0 * NLMISC::Pi * k / segs;
				verts.push_back(NLMISC::CVector(rr * (float)cos(a), rr * (float)sin(a), z));
			}
		}
		uint32 bp = (uint32)verts.size();
		verts.push_back(NLMISC::CVector(0.0f, 0.0f, -r));
		#define SPH_RING(i, k) (1 + ((i) - 1) * segs + ((k) % segs))
		tris.clear();
		// Sphere: matId 1, sg 0x01 uniformly across the surface (~/shape_export_dataset primuv_sphere).
		#define SPH_TRI(a, b, c) { SPrimTri t = { { (uint32)(a), (uint32)(b), (uint32)(c) }, 1, 0x01 }; tris.push_back(t); }
		for (sint k = 0; k < segs; ++k)
			SPH_TRI(0, SPH_RING(1, k), SPH_RING(1, k + 1))
		for (sint i = 1; i < rows - 1; ++i)
			for (sint k = 0; k < segs; ++k)
			{
				SPH_TRI(SPH_RING(i, k), SPH_RING(i + 1, k), SPH_RING(i + 1, k + 1))
				SPH_TRI(SPH_RING(i, k), SPH_RING(i + 1, k + 1), SPH_RING(i, k + 1))
			}
		for (sint k = 0; k < segs; ++k)
			SPH_TRI(bp, SPH_RING(rows - 1, k + 1), SPH_RING(rows - 1, k))
		#undef SPH_TRI
		#undef SPH_RING
		return true;
	}

	// Plane (0x081f1dfc, 0x77566f65): params 0/1 = length/width, 2/3 = length/width segments.
	if (cid == CLASSID_PLANE)
	{
		float l = paramFloat(params, 0), w = paramFloat(params, 1);
		sint ls = std::max(1, paramInt(params, 2));
		sint ws = std::max(1, paramInt(params, 3));
		float dx = w / ws, dy = l / ls;
		float x0 = -w / 2.0f, y0 = -l / 2.0f;
		sint row = ws + 1;
		verts.clear();
		for (sint iy = 0; iy <= ls; ++iy)
			for (sint ix = 0; ix <= ws; ++ix)
				verts.push_back(NLMISC::CVector(x0 + dx * ix, y0 + dy * iy, 0.0f));
		tris.clear();
		if (uvVerts) uvVerts->clear();
		// Plane: matId 0, sg 0x01 uniformly (~/shape_export_dataset primuv_plane). UV per corner =
		// (0.5+x/w, 0.5+y/l) — planar mapping over the plane's own XY extent.
		#define PLANE_UV(IDX) (uvVerts ? uvVerts->push_back(NLMISC::CVector(0.5f + verts[IDX].x / w, 0.5f + verts[IDX].y / l, 0.0f)) : ((void)0))
		for (sint iy = 0; iy < ls; ++iy)
			for (sint ix = 0; ix < ws; ++ix)
			{
				uint32 a = iy * row + ix;
				uint32 b = a + 1;
				uint32 c = a + row + 1;
				uint32 d = a + row;
				SPrimTri t1 = { { d, a, c }, 0, 0x01 };
				tris.push_back(t1);
				PLANE_UV(d); PLANE_UV(a); PLANE_UV(c);
				SPrimTri t2 = { { b, c, a }, 0, 0x01 };
				tris.push_back(t2);
				PLANE_UV(b); PLANE_UV(c); PLANE_UV(a);
			}
		#undef PLANE_UV
		return true;
	}

	return false;
}

} /* namespace PRIMMESH */

/* end of file */
