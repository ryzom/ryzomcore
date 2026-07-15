/**
 * \file map_extender_mod.h
 * \brief Apply the custom Map Extender OSM (mapext198m3.dlm, Class_ID (0x2ec82081, 0x045a6271))
 * from its derived-object mod-app cache — the headless UV recovery path for the §9 garbage-UV
 * class. The plugin object itself stores no projection settings (empty 0x39bf/0x0100 across the
 * whole corpus); the computed map lives flat in the LocalModData cache
 * (0x2500 → 0x2512 → Mesh-like 0x03e8.. map channel). See max_geometry_formats.md Part P and
 * pipeline_max_design §10z-quatorze.
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_MAP_EXTENDER_MOD_H
#define PIPELINE_MAX_EXPORT_COMMON_MAP_EXTENDER_MOD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>
#include <nel/misc/vector.h>

#include <string>
#include <vector>

#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/storage_object.h"

namespace MAPEXT {

/// Map Extender OSM — Class_ID (0x2ec82081, 0x045a6271), SuperClassId 0x810, DLL mapext198m3.dlm.
extern const NLMISC::CClassId CLASSID_MAP_EXTENDER;
const PIPELINE::MAX::TSClassId SCLASS_OSMODIFIER = 0x00000810;

bool isMapExtenderModifier(PIPELINE::MAX::CSceneClass *mod);

/// Decoded map channel recovered from the mod-app cache.
struct SMapChannel
{
	int Channel; ///< Max map channel (1 = primary UVW; 0 = vertex color; corpus is 1 or 2)
	std::vector<NLMISC::CVector> UVs; ///< nVerts × Point3 UVW
	/// Per-face corner indices into UVs; size = 3 * numFaces. Parallel to the mesh face list
	/// that was current when the modifier last evaluated (must match eval mesh face count).
	std::vector<uint32> FaceUVs;
	uint32 numFaces() const { return (uint32)(FaceUVs.size() / 3); }
};

/// Read the Map Extender LocalModData cache from \a modApp (the 0x2500 container for this
/// modifier slot). Returns false if the cache is missing or malformed. Does not require the
/// plugin object (\a mod) beyond class-id identification — all functional data is in the cache.
/// Optional \a err receives a short reason on failure.
bool readMapExtenderCache(PIPELINE::MAX::CStorageContainer *modApp, SMapChannel &out,
                          std::string *err = 0);

/// Apply the cache onto a mesh's map channel. \a currentFaceCount is the evaluated mesh's face
/// count after lower modifiers; if it does not match the cache face count the apply is refused
/// (stale cache / stack mismatch) and false is returned.
/// On success, \a outChannel receives the channel index and \a outUVs / \a outFaceUVs are filled.
bool applyMapExtender(PIPELINE::MAX::CSceneClass *mod, PIPELINE::MAX::CStorageContainer *modApp,
                      uint currentFaceCount,
                      int &outChannel,
                      std::vector<NLMISC::CVector> &outUVs,
                      std::vector<uint32> &outFaceUVs,
                      std::string *err = 0);

} /* namespace MAPEXT */

#endif /* PIPELINE_MAX_EXPORT_COMMON_MAP_EXTENDER_MOD_H */

/* end of file */
