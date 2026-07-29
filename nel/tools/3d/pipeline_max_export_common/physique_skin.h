/**
 * \file physique_skin.h
 * \brief Decode Physique (and eventually Skin) per-node vertex weights into NeL CMeshBuild
 * skinning fields — the headless counterpart of CExportNel::buildSkinning for the Physique
 * path (plugin_max/nel_mesh_lib/export_skinning.cpp). Format: max_geometry_formats.md Part M;
 * design doc §10z-quat / §10z-six.
 *
 * Scope (2026-07-09 production path):
 *  - Physique mod-app 0x2500 → 0x2512 → 0x2504 → N × (0x2506 → 0x0989) per-vertex records.
 *  - boneRef = ~index into the Physique modifier's reference array (primary/rigid links);
 *    positive boneRefs (deformable cross-links, ~4 % of corpus) are tried as raw indices.
 *  - Skeleton bone-id map = scene-tree walk from the skeleton root (first resolvable bone's
 *    top-level ancestor), matching the reference's buildSkeletonShape mapId order via
 *    orderedChildrenOf.
 *  - Top-4 highest weights, normalize by sum — same as the reference.
 *  - Vertices that resolve to no valid skeleton bone fall back to skeleton root weight 1.0
 *    (covers the ConvertToRigid root-link promotion class approximately; see Part M §M.3).
 *
 * Skin modifier is detected but not decoded (zero corpus hits under Max 3-era assets).
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_PHYSIQUE_SKIN_H
#define PIPELINE_MAX_EXPORT_COMMON_PHYSIQUE_SKIN_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>
#include <nel/3d/mesh.h>

#include <map>
#include <string>
#include <vector>

#include "../pipeline_max/builtin/i_node.h"
#include "../pipeline_max/builtin/reference_maker.h"
#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/storage_object.h"

namespace PHYSIQUESKIN {

// Physique = Class_ID(0x100, 0) with SuperClassId 0x810 (OSModifier) — ClassId alone is shared
// with Placement/Output/Shadow Map. Skin = Class_ID(0x0095c6a3, 0x00015666).
extern const NLMISC::CClassId CLASSID_PHYSIQUE;
extern const NLMISC::CClassId CLASSID_SKIN;
const PIPELINE::MAX::TSClassId SCLASS_OSMODIFIER = 0x00000810;

bool isPhysiqueModifier(PIPELINE::MAX::CSceneClass *mod);
bool isSkinModifier(PIPELINE::MAX::CSceneClass *mod);

/// One resolved per-bone influence on a vertex (pre top-4 / normalize).
struct SBoneWeight
{
	PIPELINE::MAX::BUILTIN::INode *Bone;
	float Weight;
};

/// Decode Physique mod-app 0x2500 payload into per-vertex bone weights (mesh vertex order).
/// `mod` is the Physique scene object (for its reference bone table); `modApp` is the per-node
/// 0x2500 container on the OSM Derived wrapper. Returns false if the payload is unreadable.
/// Vertices that cannot resolve any bone get an empty list (caller may assign root fallback).
bool decodePhysiqueWeights(PIPELINE::MAX::CSceneClass *mod,
                           PIPELINE::MAX::CStorageContainer *modApp,
                           std::vector<std::vector<SBoneWeight> > &outVertWeights,
                           std::string *err = nullptr);

/// Walk the skeleton tree rooted at `root` (scene-order children) into a bone-id map + name list,
/// matching the reference's buildSkeletonShape mapId convention (duplicate names get "_Second").
void buildSkeletonBoneMap(PIPELINE::MAX::BUILTIN::INode *root,
                          PIPELINE::MAX::CSceneClassContainer *ssc,
                          std::map<PIPELINE::MAX::BUILTIN::INode *, sint32> &mapId,
                          std::vector<std::string> &bonesNames);

/// Climb to the top-level skeleton root (child of the scene root) from any bone node.
PIPELINE::MAX::BUILTIN::INode *skeletonRootOf(PIPELINE::MAX::BUILTIN::INode *bone);

/// Full Physique → CMeshBuild.SkinWeights + BonesNames. Looks for a Physique modifier on the
/// node (via the supplied mods/modApps parallel arrays from baseObjectOf). `ssc` is needed for
/// orderedChildrenOf during the skeleton walk. On success, `buildMesh.SkinWeights` has one entry
/// per vertex (same count as buildMesh.Vertices) and `BonesNames` is the full skeleton bone list
/// (mesh build later remaps to the used subset). Returns false with a reason in `err` when the
/// decode cannot produce usable weights (caller should skip the node).
bool applyPhysiqueSkinning(NL3D::CMesh::CMeshBuild &buildMesh,
                           PIPELINE::MAX::BUILTIN::INode &node,
                           const std::vector<PIPELINE::MAX::CSceneClass *> &mods,
                           const std::vector<PIPELINE::MAX::CStorageContainer *> &modApps,
                           PIPELINE::MAX::CSceneClassContainer *ssc,
                           std::string *err = nullptr);

} /* namespace PHYSIQUESKIN */

#endif /* PIPELINE_MAX_EXPORT_COMMON_PHYSIQUE_SKIN_H */

/* end of file */
