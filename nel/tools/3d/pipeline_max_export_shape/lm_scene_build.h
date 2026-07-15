/**
 * \file lm_scene_build.h
 * \brief Lightmap scene-graph writer support for the headless shape exporter — the 1_export
 * half of the standalone-lightmapper design (pipeline_max_design.md §11-lm). The exporter
 * collects, per source .max with lightmapped receivers, a NL3D::CLightmapScene: the lightmap
 * lights (decoded here), the shadow-casting occluder meshes, and the receivers as pre-build
 * CMeshBuild/CMeshBaseBuild plus the shape-build recipe. The standalone lightmapper consumes
 * the file at the 2_build stage; the exporter's own (unmapped) shape output is unchanged.
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

#ifndef PIPELINE_MAX_EXPORT_SHAPE_LM_SCENE_BUILD_H
#define PIPELINE_MAX_EXPORT_SHAPE_LM_SCENE_BUILD_H

#include <nel/misc/types_nl.h>
#include <nel/3d/lightmap_scene.h>

#include <map>
#include <set>
#include <string>

#include "scene_lib.h"

namespace LMSCENE {

/// Per-file collection state for the scene-graph writer (owned by exportFile).
struct SCollector
{
	NL3D::CLightmapScene Scene;

	// Scene-wide LOD relationships (filled by exportFile before the node loop):
	// every LOD-slave node name announced anywhere (original case), and the reverse map
	// slave-name(lower) -> names of the nodes that list it as their LOD.
	std::set<std::string> LodSlaveNames;
	std::map<std::string, std::set<std::string> > LodParents;

	// Names already present in Scene.Occluders (dedup)
	std::set<std::string> OccluderNames;

	// Routing flag for the receiver currently being built (set by exportFile per node)
	bool CurrentCoarse;

	SCollector() : CurrentCoarse(false) {}
};

/** Decode one lightmap light from a light node (superclass 0x30 object), replicating
 *	SLightBuild::convertFromMaxLight on stored data. Returns false when the node is not a
 *	light or is unchecked for lightmap export (NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT).
 *
 *	Storage notes (corpus-pinned; see the light decode in pipeline_max_export_ig plus this
 *	session's probes): old ParamBlock on reference 0 — param 0 = color (Point3 0..1),
 *	param 1 = intensity/multiplier (float; 1.5 on the animated-street braseros, 1.0 default),
 *	omni attenuation 6/7, spot hotspot/falloff (degrees) 4/5 and attenuation 9/10. Object
 *	flag words: 0x2562 = use attenuation, 0x2570 = cast shadows (PROVISIONAL — varies per
 *	light exactly like the artist shadow toggle: sun=1, accent omnis=0; validated in
 *	aggregate against the reference lightmaps). Empty chunk 0x2600 = ambient only.
 *
 *	EXCLUSION LIST (decoded off zo_cn_mairie's zo_mairie_omniamb/omniamb01 against the
 *	reference lightmap logs): light-object chunk 0x2800 = { 0x03e9 = uint32 count,
 *	0x03ea = count × uint32 NODE HANDLES, 0x03ec = uint32 flags (6 = exclude
 *	illumination+shadow observed corpus-wide) }. Handles resolve through node chunk
 *	0x0a32 (the persistent Max node handle) — nodeByHandle carries that map. The
 *	reference's convertFromMaxLight inserts every ExclList entry as an exclusion NAME
 *	regardless of the include/exclude flag bits; reproduced as-is.
 */
bool convertLightmapLight(NL3D::CLightmapLight &out, PIPELINE::MAX::BUILTIN::INode &node,
                          SCENELIB::SNodeTMCache &tmCache,
                          const std::map<uint32, std::string> &nodeByHandle);

/// Node chunk 0x0a32 = the persistent Max node handle (used by light exclusion lists).
bool nodeHandle(PIPELINE::MAX::BUILTIN::CNodeImpl *n, uint32 &out);

/** Fill a receiver geom's per-node lightmap appdata (lumel-size multiplier, 8-bit LMC
 *	compression colors) and its raytrace-world exclusion set (the original exporter's
 *	addChildLodNode/addParentLodNode semantics: every LOD-slave in the scene plus the nodes
 *	this geom is a LOD of, never the geom itself).
 */
void fillGeomAppData(NL3D::CLightmapReceiverGeom &geom, PIPELINE::MAX::BUILTIN::CNodeImpl *n,
                     const SCollector &col, const std::string &nodeName);

} /* namespace LMSCENE */

#endif /* PIPELINE_MAX_EXPORT_SHAPE_LM_SCENE_BUILD_H */

/* end of file */
