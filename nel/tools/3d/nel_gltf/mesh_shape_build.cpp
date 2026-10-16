/**
 * \file mesh_shape_build.cpp
 * \brief See mesh_shape_build.h.
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

#include <nel/misc/types_nl.h>
#include "mesh_shape_build.h"

#include <nel/3d/mesh_mrm.h>
#include <nel/3d/mesh_mrm_skinned.h>
#include <nel/3d/mesh_geom.h>

using namespace NL3D;

namespace NLGLTF {

IMeshGeom *buildMeshGeom(CMesh::CMeshBuild &mb, uint numMaxMaterial,
                         bool wantMrm, const CMRMParameters &params)
{
	if (wantMrm)
	{
		std::vector<CMesh::CMeshBuild *> bsList; // morph targets: never on multilod slots
		CMeshMRMGeom *g = new CMeshMRMGeom;
		g->build(mb, bsList, numMaxMaterial, params);
		return g;
	}
	CMeshGeom *g = new CMeshGeom;
	g->build(mb, numMaxMaterial);
	return g;
}

CMeshBase *buildMeshShape(CMeshBase::CMeshBaseBuild &bbm,
                          CMesh::CMeshBuild &mb,
                          std::vector<CMesh::CMeshBuild *> &bsList,
                          bool wantMrm, const CMRMParameters &params,
                          std::vector<sint> &materialRemap,
                          std::string *skipReason)
{
	// LOD_MRM alone selects the MRM branch, matching the reference plugin
	// (export_mesh.cpp:360): a skinned node without it exports as a plain CMesh carrying
	// SkinWeights (ca_spaceship2/ship_tank_karavan in the reference corpus).
	if (wantMrm)
	{
		if (CMeshMRMSkinned::isCompatible(mb) && bsList.empty())
		{
			CMeshMRMSkinned *meshMRMSkinned = new CMeshMRMSkinned;
			meshMRMSkinned->build(bbm, mb, params);
			// CMeshMRMSkinned::isCompatible gates the INPUT vertex count, but MRM construction
			// can grow the vertex count at smoothing-group/material/bone boundaries past
			// NL3D_MESH_SKIN_MANAGER_MAXVERTICES=5000 (the skin-manager's fixed shared VB
			// size). CMeshMRMSkinnedGeom::compileRunTime logs the failure and clears
			// _RuntimeCompiled when that happens.
			if (!meshMRMSkinned->isRuntimeCompiled())
			{
				delete meshMRMSkinned;
				if (skipReason) *skipReason = "skinned-maxverts";
				return nullptr;
			}
			meshMRMSkinned->optimizeMaterialUsage(materialRemap);
			return meshMRMSkinned;
		}
		CMeshMRM *meshMRM = new CMeshMRM;
		meshMRM->build(bbm, mb, bsList, params);
		meshMRM->optimizeMaterialUsage(materialRemap);
		return meshMRM;
	}
	CMesh *m = new CMesh;
	m->build(bbm, mb);
	m->optimizeMaterialUsage(materialRemap);
	return m;
}

} /* namespace NLGLTF */

/* end of file */
