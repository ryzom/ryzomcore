/**
 * \file parametric_mesh.h
 * \brief Ground-truth-exact topology generation for Max's parametric primitives (Box/Cylinder/
 * Sphere/Plane). Shared by pipeline_max_export_ig (cluster/portal geometry evaluation) and
 * pipeline_max_export_shape (a node whose base object is a parametric primitive exports as a
 * plain CMesh — the ~50 direct-shape-primitive references in the workspace corpus, §10i handoff).
 * Vertex order, face order and windings are pinned to the Max-side `gen_prim_mesh_dataset.ms`
 * ground-truth dataset (~/prim_mesh_dataset, 2026-07-06) — see the design doc §10g primitive
 * dataset round for the derivation.
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_PARAMETRIC_MESH_H
#define PIPELINE_MAX_EXPORT_COMMON_PARAMETRIC_MESH_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>
#include <nel/misc/vector.h>

#include <map>
#include <vector>

#include "old_param_block.h"

namespace PRIMMESH {

/// A parametric-primitive triangle: three vertex indices in the caller-provided vertex list plus
/// the 0-based material id and smoothing-group bitmask Max's own primitive builder assigns to it
/// (corpus-validated against ~/shape_export_dataset/manifest.txt: plain_box / primuv_box(_multiseg)
/// / primuv_cyl / primuv_sphere / primuv_plane). The 0-based `MatId` is the low half-word of Max's
/// Face::flags matID field (MAXScript `getFaceMatID` returns `MatId + 1`); `SmGroup` is the raw
/// smoothing-group bitmask.
///
/// Old callers that read only `V[]` (cmb's collision face builder, which fills its own MatId=0 and
/// full edge visibility per its own conventions) keep working — the new fields are additive.
struct SPrimTri
{
	uint32 V[3];
	uint32 MatId;   ///< 0-based; MAXScript convention = MatId + 1
	uint32 SmGroup; ///< smoothing-group bitmask, one bit per group
};

/// Well-known parametric-primitive class ids.
extern const NLMISC::CClassId CLASSID_BOX;      ///< {0x10, 0}
extern const NLMISC::CClassId CLASSID_CYLINDER; ///< {0x12, 0}
extern const NLMISC::CClassId CLASSID_SPHERE;   ///< {0x11, 0}
extern const NLMISC::CClassId CLASSID_PLANE;    ///< {0x081f1dfc, 0x77566f65}

/// Build a parametric primitive's mesh (object space) from its old-style ParamBlock parameters.
/// Vertices + tris are appended verbatim; returns true iff the class id is a recognised primitive.
/// `params` is the read output of OLDPARAMBLOCK::readPBlockParams on the object's ref-0 pblock.
///
/// Topology summary (see the design doc §10g primitive dataset round for the derivation):
///  - Box: bottom grid + top grid (ix fastest, x/y ascending) + hs-1 middle perimeter rings
///    walking P00 → +x → +y → −x → −y. Bottom/top cells (a, a+row, a+row+1)+(a+row+1, a+1, a) /
///    (a, a+1, a+1+row)+(a+1+row, a+row, a). Sides grouped per side (−y, +x, +y, −x), per level
///    bottom-up, per segment: (lo1, lo2, up2)+(up2, up1, lo1). Negative height flips every
///    winding; negative length/width only flip coordinates.
///  - Cylinder: bottom center, rings bottom-up at k·2π/sides from +x CCW, top center. Caps:
///    bottom (c, r[k+1], r[k]), top (tc, t[k], t[k+1]). Sides (lo[k], up[k+1], up[k])+
///    (lo[k], lo[k+1], up[k+1]).
///  - Sphere: top pole, rings top-down with meridians starting at +Y CCW, bottom pole. Fans +
///    quad rows (u[k], lo[k], lo[k+1])+(u[k], lo[k+1], u[k+1]).
///  - Plane: grid at z=0 (ix fastest); per cell (d, a, c)+(b, c, a).
///
/// When `uvVerts` is non-NULL, the "generate mapping coords" UVs are also emitted (Box + Plane
/// only for now — Cylinder/Sphere need per-primitive seam handling, design §10z-cinq). `uvVerts`
/// receives exactly `3 x tris.size()` entries, one UV per triangle corner in tri order (corner c
/// of tri t is `(*uvVerts)[t*3 + c]`); the caller builds a map channel whose face t maps its 3
/// corners to `t*3 + {0,1,2}`. No dedup here: the consuming mesh build dedups the final vertex
/// buffer on (pos, normal, UV), so the per-corner VALUE is what matters. Box UVs derive
/// geometrically per face (each face projects onto its two in-plane axes; validated against
/// ~/shape_export_dataset primuv_box(_multiseg), generalises to multi-segment by vertex coord);
/// Plane UVs are `(0.5+x/w, 0.5+y/l)`.
bool buildParametricMesh(const NLMISC::CClassId &cid,
                         const std::map<sint32, OLDPBLOCK::SParam> &params,
                         std::vector<NLMISC::CVector> &verts,
                         std::vector<SPrimTri> &tris,
                         std::vector<NLMISC::CVector> *uvVerts = NULL);

} /* namespace PRIMMESH */

#endif /* PIPELINE_MAX_EXPORT_COMMON_PARAMETRIC_MESH_H */

/* end of file */
