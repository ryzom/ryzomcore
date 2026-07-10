/**
 * \file interface_build.h
 * \brief Interface-mesh border weld — the headless counterpart of
 * CExportNel::applyInterfaceToMeshBuild (plugin_max/nel_mesh_lib/export_mesh_interface.cpp).
 *
 * Nodes with NEL3D_APPDATA_INTERFACE_FILE name another .max whose meshes are flat border
 * polygons ("interfaces"). Every vertex of the exported mesh within
 * NEL3D_APPDATA_INTERFACE_THRESHOLD of an interface vertex (in world space) is border-welded:
 *
 *  - The MRM bookkeeping is filled either way: CMeshBuild::Interfaces (the interface polygons
 *    in object space), InterfaceLinks (per-vertex interface/vertex ids), InterfaceVertexFlag,
 *    plus the CMeshMRMSkinned packed-vertex align of every linked vertex (the reference
 *    pack/unpack round-trip that snaps linked positions onto the packed grid so interface
 *    vertices agree across separately-exported meshes).
 *  - Normal correction, two variants gated by
 *    NEL3D_APPDATA_GET_INTERFACE_NORMAL_FROM_SCENE_OBJECTS (corpus: 1 on 265 of 286
 *    instances — the SCENE variant is dominant):
 *      0 → interface normals: welding corners snap POSITION to the interface vertex and take
 *          its polygon-edge normal (prev/next edge × polygon avg normal, normalized sum).
 *      1 → scene normals: for each welding corner, collect ALL scene faces (tree order from
 *          the scene root, evaluated world-space) sharing a smoothing group with the corner's
 *          face and having any vertex within threshold of the corner; the corner normal
 *          becomes the area-weighted sum of those face normals (positions do NOT snap).
 *
 * The interface .max resolves through DBPATH (authored "R:\graphics\..." form, ".max" appended
 * when the extension is missing); loaded scenes and their border polys are cached per path.
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_INTERFACE_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_INTERFACE_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/matrix.h>
#include <nel/3d/mesh.h>

#include "scene_lib.h"

namespace IFACEBUILD {

/// Whether the node carries a (non-empty) interface file appdata.
bool useInterfaceMesh(PIPELINE::MAX::BUILTIN::INode &node);

/// Apply the interface weld to a built CMeshBuild. `toWorldMat` maps the buildMesh's vertex
/// space to world (Identity for skinned meshes — their VBs are already world — else
/// worldObjectTM * Inverse(toExportSpace), same as the reference call sites). Reads the
/// interface file / threshold / scene-normals appdata off `node`; no-op when absent, warns
/// and skips on invalid threshold or unresolvable interface file. `tmCache` is the CURRENT
/// scene's TM cache (for the scene-normals variant's face sweep).
void applyInterfaceToMeshBuild(PIPELINE::MAX::BUILTIN::INode &node,
                               NL3D::CMesh::CMeshBuild &buildMesh,
                               const NLMISC::CMatrix &toWorldMat,
                               SCENELIB::SNodeTMCache &tmCache);

/// Drop all cached interface scenes (per-process cache; call between files if memory matters).
void clearInterfaceCache();

} /* namespace IFACEBUILD */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_INTERFACE_BUILD_H */

/* end of file */
