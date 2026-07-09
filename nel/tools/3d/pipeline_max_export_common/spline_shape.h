/**
 * \file spline_shape.h
 * \brief Decode Max Shape-superclass objects (SplineShape, Line, Rectangle, …) into knot
 * positions for consumers that need curve endpoints — primarily CSegRemanence export
 * (InterpPiece3D at piece ends) and any future spline-tessellation path (cmb XRef fallback,
 * residual mesh-eval shape-class nodes).
 *
 * On-disk format (corpus-verified against Max 3→9→2010 assets; see max_geometry_formats.md
 * Part N): under the shape object's BezierShape tree, each Spline3D is a container holding
 *   0x2900  uint32 numKnots
 *   0x2904  uint32 closed (0/1)
 *   0x290a  numKnots × 52-byte compact knot records
 *   0x290d  trailing uint32
 * Compact knot (52 B) = ktype i32 + ltype i32 + du f32 + Point3 p0 + Point3 p1 + Point3 p2
 * + flags u32. The **first Point3** is the knot point matching ShapeObject::InterpPiece3D
 * endpoints at u=0/1 (corpus-verified against reference remanence corners after objectToLocal).
 *
 * SuperClassId SHAPE = 0x40. Common ClassIds: SplineShape (0x0a,0), Line (0x1040,0),
 * Rectangle (0x1065,0).
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_SPLINE_SHAPE_H
#define PIPELINE_MAX_EXPORT_COMMON_SPLINE_SHAPE_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>
#include <nel/misc/vector.h>

#include <vector>

#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/storage_object.h"

namespace SPLINESHAPE {

/// SuperClassId for all Shape objects (SplineShape, Line, Rectangle, Circle, …).
const PIPELINE::MAX::TSClassId SCLASS_SHAPE = 0x00000040;

extern const NLMISC::CClassId CLASSID_SPLINESHAPE; // (0x0000000a, 0)
extern const NLMISC::CClassId CLASSID_LINE;        // (0x00001040, 0)
extern const NLMISC::CClassId CLASSID_RECTANGLE;   // (0x00001065, 0)

/// True when the scene class is a Shape-superclass object (spline geometry).
bool isShapeObject(PIPELINE::MAX::CSceneClass *obj);

/// One knot of a Spline3D (compact on-disk form).
struct SKnot
{
	sint32 KType;  // KTYPE_AUTO=0, CORNER=1, BEZIER=2, BEZIER_CORNER=3
	sint32 LType;  // LTYPE_CURVE=0, LTYPE_LINE=1
	float Du;      // parameter value
	NLMISC::CVector Knot;   // the knot point (InterpPiece3D endpoints)
	NLMISC::CVector InVec;  // second Point3 on disk (handle; absolute or relative depending on era)
	NLMISC::CVector OutVec; // third Point3 on disk
	uint32 Flags;
};

/// One curve (Spline3D) inside a BezierShape.
struct SSpline
{
	bool Closed;
	std::vector<SKnot> Knots;
};

/// Full shape: ordered list of curves. Remanence requires exactly one curve with ≥1 segment.
struct SShape
{
	std::vector<SSpline> Curves;
};

/// Decode all Spline3D records under a shape scene object (walks orphaned + claimed chunk trees
/// looking for the 0x2900/0x290a sibling pattern). Returns false if no spline data found.
bool decodeShapeObject(PIPELINE::MAX::CSceneClass *shapeObj, SShape &out);

/// Decode a single 0x290a raw payload given numKnots from 0x2900.
bool decodeKnotsRaw(const uint8 *data, uint size, uint32 numKnots, std::vector<SKnot> &out);

/// Piece-endpoint positions for curve 0 — the sequence remanence samples via
/// InterpPiece3D(time, 0, k, u) at the piece ends: knot[0], knot[1], … knot[n-1]
/// (closed curves do not re-append knot[0] here; remanence uses NumberOfPieces = n-1 for open).
/// Returns false if curveIndex is out of range or the curve has fewer than 2 knots.
bool pieceEndpoints(const SShape &shape, uint curveIndex, std::vector<NLMISC::CVector> &ends);

} /* namespace SPLINESHAPE */

#endif /* PIPELINE_MAX_EXPORT_COMMON_SPLINE_SHAPE_H */

/* end of file */
