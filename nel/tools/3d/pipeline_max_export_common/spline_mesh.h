/**
 * \file spline_mesh.h
 * \brief Max Shape-superclass (spline) → TriObject mesh conversion, replicating what the
 * reference exporter triggers via os.obj->ConvertToType(triObjectClassID) on a shape node:
 *  - OPEN splines convert to an EMPTY mesh (0 verts / 0 faces) — the corpus references store
 *    literally empty CMesh shapes for them (tr_wea_hache2m_trail_00, shape02/03).
 *  - CLOSED splines convert to a capped mesh: tessellate each spline to a polygon (steps /
 *    optimize / adaptive from the shape's interpolation params), project onto the XY plane
 *    (Max's shape objects are conceptually 2D in their local XY — corpus-proven: the two
 *    closed splines whose XY projection self-intersects, shape02/03, export EMPTY from real
 *    Max 2010 while every other projection choice caps them), then ear-clip with the
 *    MINIMUM-INTERIOR-ANGLE valid-ear rule (ties → LAST minimal ear in ring order; polygon
 *    with negative XY area processes the REVERSED ring; triangles emitted (prev, ear, next)
 *    verbatim — cap winding follows the spline direction). Verified face-for-face against the
 *    corpus references (shape01/04/05/06/07 + mirrors — 61 triangles, zero deviation).
 *  - Rectangle (0x1065) generates its knots from the pblock length/width/fillet per the SDK
 *    sample source (rectangl.cpp BuildShape) — it stores no BezierShape chunks.
 *
 * See pipeline_max_design.md (spline-mesh session) for the derivation and GT method.
 * \author Jan Boon (Kaetemi)
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_SPLINE_MESH_H
#define PIPELINE_MAX_EXPORT_COMMON_SPLINE_MESH_H

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>

#include <vector>

#include "spline_shape.h"

namespace SPLINEMESH {

/// One cap triangle (indices into the tessellated vertex list).
struct SCapTri
{
	uint32 V[3];
};

/// Result of the shape→mesh conversion. Empty (0 verts / 0 faces) is a VALID result — open
/// splines and failed caps both produce it, matching Max.
struct SSplineMesh
{
	std::vector<NLMISC::CVector> Verts;
	std::vector<SCapTri> Faces;
};

/// Tessellate one spline to its polygon points (no closing duplicate). `steps` / `optimize` /
/// `adaptive` are the shape's interpolation params (BezierShape chunks 0x1050/0x1060/0x1070).
/// steps == 0 → knots only. Line-type segments (and, under optimize, geometrically straight
/// curve segments) contribute no intermediate points.
void tessellateSpline(const SPLINESHAPE::SSpline &spline, sint steps, bool optimize,
                      bool adaptive, std::vector<NLMISC::CVector> &outPts);

/// Cap one closed polygon with the corpus-proven Max morph-cap rule. Returns false when the
/// capper gets stuck (self-intersecting XY projection — Max produces an empty mesh then).
/// Emitted triangles index into `pts`.
bool capPolygon(const std::vector<NLMISC::CVector> &pts, std::vector<SCapTri> &outTris);

/// Full conversion: decoded shape (+ interpolation params) → mesh. Open or failed-cap →
/// empty mesh, exactly like Max's TriObject conversion. Only the FIRST closed curve caps
/// (multi-curve hole capping has no corpus instance; a warning is emitted by the caller).
bool buildSplineMesh(const SPLINESHAPE::SShape &shape, sint steps, bool optimize, bool adaptive,
                     SSplineMesh &out);

} /* namespace SPLINEMESH */

#endif /* PIPELINE_MAX_EXPORT_COMMON_SPLINE_MESH_H */

/* end of file */
