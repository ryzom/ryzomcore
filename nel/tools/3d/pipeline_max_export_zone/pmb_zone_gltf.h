/**
 * \file pmb_zone_gltf.h
 * \brief Entry points for the max2gltf writer (pipeline_max_export_zone/main.cpp compiled with
 * PMB_ZONE_NO_MAIN): the ligo zone process's whole-file flow returning its outputs as
 * (relative name, bytes) blobs, plus the authored patch sets for tessellated viewing proxies.
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

#ifndef PIPELINE_MAX_EXPORT_ZONE_PMB_ZONE_GLTF_H
#define PIPELINE_MAX_EXPORT_ZONE_PMB_ZONE_GLTF_H

#include <nel/misc/types_nl.h>
#include <nel/3d/zone.h>

#include <string>
#include <utility>
#include <vector>

#include "../pipeline_max_export_common/max_load.h"

/// One authored patch node (pre-symmetry/rotate, world space) for the nel_proxy viewing mesh.
struct SPmbZoneProxy
{
	std::string NodeName;
	bool Frozen; // frozen nodes are neighbor-reference bricks (visual context, not the export)
	std::vector<NL3D::CPatchInfo> Patches;
};

// Run the NeLLigoExportZone flow (protocol by the .max basename: zonematerial / zonetransition
// / zonespecial) over the caller's already-parsed scene and hand back every produced file —
// zones/<name>.zone, zoneligos/<name>.ligozone — as (relative name, bytes). `proxiesOut`
// (optional) receives the scene's RklPatch nodes as authored CPatchInfo sets for the
// tessellated viewing proxies. Returns the produced file count, 0 when the basename is not a
// ligo protocol name or the scene has nothing to export, -1 on failure (the same authoring
// errors the standalone tool refuses — e.g. multiple non-frozen NelPatchMesh).
int pmbExportZonesForGltf(const std::string &maxPath, PMAXLOAD::SLoadedMax &lm,
                          const std::string &bankPath,
                          float cellSize, float snap,
                          std::vector<std::pair<std::string, std::vector<uint8> > > &filesOut,
                          std::vector<SPmbZoneProxy> *proxiesOut);

#endif /* PIPELINE_MAX_EXPORT_ZONE_PMB_ZONE_GLTF_H */

/* end of file */
