/**
 * \file mesh_build.h
 * \brief CMeshBuild / CMeshBaseBuild construction from the evaluated Max mesh — the headless
 * counterpart of CExportNel::buildBaseMeshInterface + buildMeshInterface (plugin_max/
 * nel_mesh_lib/export_mesh.cpp), including the Max buildRenderNormals smoothing-group normal
 * computation and the per-corner UV/color extraction with material channel remapping.
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_MESH_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_MESH_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/3d/mesh.h>
#include <nel/3d/mrm_parameters.h>

#include "scene_lib.h"
#include "mesh_eval.h"
#include "material_build.h"

namespace MESHBUILD {

using namespace SCENELIB;

// buildBaseMeshInterface replication: flags, materials, default transformation.
void buildBaseMeshInterface(NL3D::CMeshBase::CMeshBaseBuild &buildMesh, MATBUILD::SMaxMeshBaseBuild &maxBaseBuild,
                            INode &node, SNodeTMCache &tmCache, const MAXMATH::Matrix3M &nodeBasis,
                            bool exportLighting);

// buildMeshInterface replication over the evaluated mesh (non-skinned path; vertices in the
// node-offset local space via objectToLocal).
void buildMeshInterface(const MESHEVAL::SEvalMesh &mesh, NL3D::CMesh::CMeshBuild &buildMesh,
                        const NL3D::CMeshBase::CMeshBaseBuild &buildBaseMesh,
                        const MATBUILD::SMaxMeshBaseBuild &maxBaseBuild,
                        INode &node, SNodeTMCache &tmCache);

// buildMRMParameters replication (appdata-driven).
void buildMRMParameters(CSceneClass *node, NL3D::CMRMParameters &params);

// getLocalMatrix: nodeTM * Inverse(parentTM) in Max float math.
MAXMATH::Matrix3M getLocalMatrix(INode &node, SNodeTMCache &tmCache);

} /* namespace MESHBUILD */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_MESH_BUILD_H */

/* end of file */
