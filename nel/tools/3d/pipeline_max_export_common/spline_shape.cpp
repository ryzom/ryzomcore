/**
 * \file spline_shape.cpp
 * \brief See spline_shape.h.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 * \author Claude Fable 5
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

#include <nel/misc/common.h>

#include "../pipeline_max/builtin/shape_object.h"

using namespace PIPELINE::MAX;
using namespace NLMISC;

namespace SPLINESHAPE {

const CClassId CLASSID_SPLINESHAPE(0x0000000a, 0x00000000);
const CClassId CLASSID_LINE(0x00001040, 0x00000000);
const CClassId CLASSID_RECTANGLE(0x00001065, 0x00000000);

bool isShapeObject(CSceneClass *obj)
{
	// Every superclass-0x40 object instantiates through the CShapeObject registration.
	return dynamic_cast<BUILTIN::CShapeObject *>(obj) != nullptr;
}

// Thin copy from the typed model: CShapeObject decoded the BezierShape/Spline3D chunk tree at
// parse (raw chunks stay authoritative in the scene class; see shape_object.h for the format).
bool decodeShapeObject(CSceneClass *shapeObj, SShape &out)
{
	out.Curves.clear();
	BUILTIN::CShapeObject *so = dynamic_cast<BUILTIN::CShapeObject *>(shapeObj);
	if (!so) return false;
	const std::vector<BUILTIN::CShapeObject::SSpline> &splines = so->splines();
	for (uint s = 0; s < splines.size(); ++s)
	{
		const BUILTIN::CShapeObject::SSpline &spline = splines[s];
		if (spline.Knots.empty()) continue;
		SSpline sp;
		sp.Closed = spline.closed();
		sp.Knots.reserve(spline.Knots.size());
		for (uint k = 0; k < spline.Knots.size(); ++k)
		{
			const BUILTIN::CShapeObject::SKnot &tk = spline.Knots[k];
			SKnot knot;
			knot.KType = tk.KType;
			knot.LType = tk.LType;
			knot.Du = tk.Du;
			knot.Knot = tk.Knot;
			knot.InVec = tk.InVec;
			knot.OutVec = tk.OutVec;
			knot.Flags = tk.Flags;
			sp.Knots.push_back(knot);
		}
		out.Curves.push_back(sp);
	}
	if (so->hasSteps())
	{
		out.Steps = so->steps();
		out.HaveSteps = true;
	}
	return !out.Curves.empty();
}

void buildRectangleKnots(float length, float width, float fillet, SSpline &out)
{
	out.Closed = true;
	out.Knots.clear();
	float l2 = length / 2.0f;
	float w2 = width / 2.0f;
	SKnot k;
	k.LType = 0; // LTYPE_CURVE
	k.Du = 0.f;
	k.Flags = 0;
	if (fillet > 0.f)
	{
		// SDK rectangl.cpp BuildShape, fillet > 0: 8 KTYPE_BEZIER knots walking CCW from the
		// top-right corner's lower fillet point; CIRCLE_VECTOR_LENGTH = 0.5517861843.
		const float CIRCLE_VECTOR_LENGTH = 0.5517861843f;
		float cf = fillet * CIRCLE_VECTOR_LENGTH;
		CVector wvec(fillet, 0.f, 0.f), lvec(0.f, fillet, 0.f);
		CVector cwvec(cf, 0.f, 0.f), clvec(0.f, cf, 0.f);
		k.KType = 2; // KTYPE_BEZIER
		CVector p(w2, l2, 0.f), p2, p3;
		p3 = p - lvec;
		k.Knot = p3; k.InVec = p3 - clvec; k.OutVec = p3 + clvec; out.Knots.push_back(k);
		p = p - wvec;
		k.Knot = p; k.InVec = p + cwvec; k.OutVec = p - cwvec; out.Knots.push_back(k);
		p = CVector(-w2, l2, 0.f); p2 = p + wvec;
		k.Knot = p2; k.InVec = p2 + cwvec; k.OutVec = p2 - cwvec; out.Knots.push_back(k);
		p = p - lvec;
		k.Knot = p; k.InVec = p + clvec; k.OutVec = p - clvec; out.Knots.push_back(k);
		p = CVector(-w2, -l2, 0.f); p3 = p + lvec;
		k.Knot = p3; k.InVec = p3 + clvec; k.OutVec = p3 - clvec; out.Knots.push_back(k);
		p = p + wvec;
		k.Knot = p; k.InVec = p - cwvec; k.OutVec = p + cwvec; out.Knots.push_back(k);
		p = CVector(w2, -l2, 0.f); p3 = p - wvec;
		k.Knot = p3; k.InVec = p3 - cwvec; k.OutVec = p3 + cwvec; out.Knots.push_back(k);
		p = p + lvec;
		k.Knot = p; k.InVec = p - clvec; k.OutVec = p + clvec; out.Knots.push_back(k);
	}
	else
	{
		// fillet == 0: 4 corner knots, handles degenerate on the knot (KTYPE_CORNER authored,
		// re-typed BEZIER_CORNER after ComputeBezPoints — geometry-identical either way).
		k.KType = 1; // KTYPE_CORNER
		const CVector corners[4] = {
			CVector(w2, l2, 0.f), CVector(-w2, l2, 0.f),
			CVector(-w2, -l2, 0.f), CVector(w2, -l2, 0.f)
		};
		for (int i = 0; i < 4; ++i)
		{
			k.Knot = k.InVec = k.OutVec = corners[i];
			out.Knots.push_back(k);
		}
	}
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
