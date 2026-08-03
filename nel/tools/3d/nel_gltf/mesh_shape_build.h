/**
 * \file mesh_shape_build.h
 * \brief The mesh-path shape build of the export toolchain — pre-build CMeshBuild data to the
 * built CMeshBase (CMesh / CMeshMRM / CMeshMRMSkinned) or per-lod IMeshGeom, with the exact
 * branch rules the reference plugin uses. The direct shape exporter and the glTF reader each
 * replay this sequence on identical inputs to produce byte-identical .shape files; it is
 * defined once so the branch rules (isCompatible gate, morph-target override, the
 * skin-manager post-MRM vertex-count bail-out, optimizeMaterialUsage) cannot drift.
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

#ifndef NL_GLTF_MESH_SHAPE_BUILD_H
#define NL_GLTF_MESH_SHAPE_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mrm_parameters.h>

#include <string>
#include <vector>

namespace NL3D {
	class IMeshGeom;
}

namespace NLGLTF {

/// IMeshGeom for one multilod slot: CMeshMRMGeom (with an empty morph-target list) when the
/// slot wants MRM, else CMeshGeom — built against the PARENT context's material count. Caller
/// owns the returned pointer until CMeshMultiLod::build takes it. `params` is read only when
/// `wantMrm`.
NL3D::IMeshGeom *buildMeshGeom(NL3D::CMesh::CMeshBuild &mb, uint numMaxMaterial,
                               bool wantMrm, const NL3D::CMRMParameters &params);

/// The single-mesh shape build: CMeshMRMSkinned when compatible and no morph targets, CMeshMRM
/// when `wantMrm` (a non-empty bsList forces this branch via the isCompatible gate — how the
/// visage files ship as CMeshMRM-with-SkinWeights), plain CMesh otherwise;
/// optimizeMaterialUsage fills `materialRemap`. Returns NULL with *skipReason =
/// "skinned-maxverts" when MRM construction grows the vertex count past
/// NL3D_MESH_SKIN_MANAGER_MAXVERTICES (CMeshMRMSkinnedGeom::compileRunTime clears
/// _RuntimeCompiled) — the authoring must be fixed (LOD_MRM=0 or split the mesh). Does not
/// consume bsList (caller owns and deletes); NL3D exceptions propagate. `params` is read only
/// when `wantMrm`.
NL3D::CMeshBase *buildMeshShape(NL3D::CMeshBase::CMeshBaseBuild &bbm,
                                NL3D::CMesh::CMeshBuild &mb,
                                std::vector<NL3D::CMesh::CMeshBuild *> &bsList,
                                bool wantMrm, const NL3D::CMRMParameters &params,
                                std::vector<sint> &materialRemap,
                                std::string *skipReason);

} /* namespace NLGLTF */

#endif /* NL_GLTF_MESH_SHAPE_BUILD_H */

/* end of file */
