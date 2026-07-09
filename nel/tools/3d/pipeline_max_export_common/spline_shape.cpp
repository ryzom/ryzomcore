/**
 * \file spline_shape.cpp
 * \brief See spline_shape.h.
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

#include "spline_shape.h"

#include <cstring>

#include <nel/misc/common.h>

#include "../pipeline_max/storage_object.h"

using namespace PIPELINE::MAX;
using namespace NLMISC;

namespace SPLINESHAPE {

const CClassId CLASSID_SPLINESHAPE(0x0000000a, 0x00000000);
const CClassId CLASSID_LINE(0x00001040, 0x00000000);
const CClassId CLASSID_RECTANGLE(0x00001065, 0x00000000);

// Compact on-disk knot size (corpus-verified: every trail remanence Line/SplineShape uses 52).
static const uint KNOT_BYTES = 52;

bool isShapeObject(CSceneClass *obj)
{
	if (!obj || !obj->classDesc()) return false;
	return obj->classDesc()->superClassId() == SCLASS_SHAPE;
}

bool decodeKnotsRaw(const uint8 *data, uint size, uint32 numKnots, std::vector<SKnot> &out)
{
	out.clear();
	if (!data || numKnots == 0) return false;
	if ((uint64)numKnots * KNOT_BYTES > size) return false;
	out.reserve(numKnots);
	for (uint32 i = 0; i < numKnots; ++i)
	{
		const uint8 *p = data + (size_t)i * KNOT_BYTES;
		SKnot k;
		sint32 ktype = 0, ltype = 0;
		float du = 0.f;
		float px, py, pz, ix, iy, iz, ox, oy, oz;
		uint32 flags = 0;
		memcpy(&ktype, p + 0, 4);
		memcpy(&ltype, p + 4, 4);
		memcpy(&du, p + 8, 4);
		// First Point3 = knot point (matches remanence InterpPiece3D endpoints after objectToLocal).
		// Second / third = handle vectors (in/out; order matches SDK SplineKnotAssy after the
		// knot-first on-disk packing observed in the corpus — see Part N).
		memcpy(&px, p + 12, 4); memcpy(&py, p + 16, 4); memcpy(&pz, p + 20, 4);
		memcpy(&ix, p + 24, 4); memcpy(&iy, p + 28, 4); memcpy(&iz, p + 32, 4);
		memcpy(&ox, p + 36, 4); memcpy(&oy, p + 40, 4); memcpy(&oz, p + 44, 4);
		memcpy(&flags, p + 48, 4);
		k.KType = ktype;
		k.LType = ltype;
		k.Du = du;
		k.Knot = CVector(px, py, pz);
		k.InVec = CVector(ix, iy, iz);
		k.OutVec = CVector(ox, oy, oz);
		k.Flags = flags;
		out.push_back(k);
	}
	return true;
}

// Walk a container's children; when a container holds both 0x2900 and 0x290a, decode one spline.
static void collectSplines(CStorageContainer *cont, SShape &out)
{
	if (!cont) return;

	const CStorageContainer::TStorageObjectContainer &ch = cont->chunks();
	CStorageRaw *rawKnots = NULL;
	uint32 numKnots = 0;
	uint32 closed = 0;
	bool haveNum = false;

	for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
	{
		if (it->first == 0x2900)
		{
			CStorageRaw *r = dynamic_cast<CStorageRaw *>(it->second);
			if (r && r->Value.size() >= 4)
			{
				memcpy(&numKnots, nlVectorData(r->Value), 4);
				haveNum = true;
			}
		}
		else if (it->first == 0x2904)
		{
			CStorageRaw *r = dynamic_cast<CStorageRaw *>(it->second);
			if (r && r->Value.size() >= 4)
				memcpy(&closed, nlVectorData(r->Value), 4);
		}
		else if (it->first == 0x290a)
		{
			rawKnots = dynamic_cast<CStorageRaw *>(it->second);
		}
	}

	if (haveNum && rawKnots && numKnots > 0)
	{
		SSpline sp;
		sp.Closed = (closed != 0);
		if (decodeKnotsRaw(nlVectorData(rawKnots->Value), (uint)rawKnots->Value.size(), numKnots, sp.Knots))
			out.Curves.push_back(sp);
	}

	// Recurse into nested containers (BezierShape → spline list → spline).
	for (CStorageContainer::TStorageObjectConstIt it = ch.begin(); it != ch.end(); ++it)
	{
		CStorageContainer *child = dynamic_cast<CStorageContainer *>(it->second);
		if (child)
			collectSplines(child, out);
	}
}

// Also walk orphaned chunks on a scene class (typed classes leave geometry there).
static void collectFromSceneClass(CSceneClass *sc, SShape &out)
{
	if (!sc) return;

	// Prefer orphaned (post-parse) then chunks (pre-clean or raw-unknown).
	const CStorageContainer::TStorageObjectContainer *lists[2] = {
		&sc->orphanedChunks(),
		&sc->chunks()
	};
	for (int li = 0; li < 2; ++li)
	{
		for (CStorageContainer::TStorageObjectConstIt it = lists[li]->begin();
		     it != lists[li]->end(); ++it)
		{
			CStorageContainer *child = dynamic_cast<CStorageContainer *>(it->second);
			if (child)
				collectSplines(child, out);
		}
	}
}

bool decodeShapeObject(CSceneClass *shapeObj, SShape &out)
{
	out.Curves.clear();
	if (!shapeObj) return false;
	collectFromSceneClass(shapeObj, out);
	// Deduplicate if the same spline was found via both orphaned and chunks (rare).
	// Prefer the first complete curve list; if we got multiples of the same knot count
	// from double-walk, keep unique by knot-point signature of the first knot.
	if (out.Curves.size() > 1)
	{
		std::vector<SSpline> uniq;
		for (uint i = 0; i < out.Curves.size(); ++i)
		{
			bool dup = false;
			for (uint j = 0; j < uniq.size(); ++j)
			{
				if (uniq[j].Knots.size() != out.Curves[i].Knots.size()) continue;
				if (uniq[j].Knots.empty()) { dup = true; break; }
				if (uniq[j].Knots[0].Knot == out.Curves[i].Knots[0].Knot
				    && uniq[j].Knots.back().Knot == out.Curves[i].Knots.back().Knot)
				{
					dup = true;
					break;
				}
			}
			if (!dup) uniq.push_back(out.Curves[i]);
		}
		out.Curves.swap(uniq);
	}
	return !out.Curves.empty();
}

bool pieceEndpoints(const SShape &shape, uint curveIndex, std::vector<CVector> &ends)
{
	ends.clear();
	if (curveIndex >= shape.Curves.size()) return false;
	const SSpline &sp = shape.Curves[curveIndex];
	if (sp.Knots.size() < 2) return false;
	// NumberOfPieces = n-1 (open) or n (closed). Remanence samples piece ends:
	//   k=0 → InterpPiece(0,0,0) = knot[0]
	//   k>0 → InterpPiece(k-1,1) = knot[k]
	// so the endpoint list is simply the knot points (closed: remanence uses open-piece
	// count from NumberOfPieces which for closed includes the wrap — check closed later
	// if a closed remanence appears; every corpus trail is open with 2 knots).
	ends.reserve(sp.Knots.size());
	for (uint i = 0; i < sp.Knots.size(); ++i)
		ends.push_back(sp.Knots[i].Knot);
	return true;
}

} /* namespace SPLINESHAPE */

/* end of file */
