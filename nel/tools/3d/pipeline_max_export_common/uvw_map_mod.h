/**
 * \file uvw_map_mod.h
 * \brief Apply Max UVW Map modifier (Class_ID 0xf72b1) to an evaluated mesh — the headless
 * counterpart of Mesh::ApplyUVWMap driven by the UVW Map OSM. Format: old ParamBlock indices
 * (istdplug.h UVWMAP_*) + gizmo PRS (modifier ref) + mod-context TM 0x2510. See
 * max_geometry_formats.md Part O and pipeline_max_design §10z-onze.
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

#ifndef PIPELINE_MAX_EXPORT_COMMON_UVW_MAP_MOD_H
#define PIPELINE_MAX_EXPORT_COMMON_UVW_MAP_MOD_H

#include <nel/misc/types_nl.h>
#include <nel/misc/class_id.h>
#include <nel/misc/vector.h>

#include <map>
#include <vector>

#include "max_math.h"
#include "../pipeline_max/scene_class.h"
#include "../pipeline_max/storage_object.h"

namespace UVWMAP {

/// UVW Map OSM — Class_ID(UVWMAPOSM_CLASS_ID, 0) = (0xf72b1, 0), SuperClassId 0x810.
extern const NLMISC::CClassId CLASSID_UVW_MAP;
const PIPELINE::MAX::TSClassId SCLASS_OSMODIFIER = 0x00000810;

// istdplug.h UVWMAP_* ParamBlock indices
enum
{
	UVWMAP_MAPTYPE = 0,
	UVWMAP_UTILE = 1,
	UVWMAP_VTILE = 2,
	UVWMAP_WTILE = 3,
	UVWMAP_UFLIP = 4,
	UVWMAP_VFLIP = 5,
	UVWMAP_WFLIP = 6,
	UVWMAP_CAP = 7,
	UVWMAP_CHANNEL = 8,
	UVWMAP_LENGTH = 9,
	UVWMAP_WIDTH = 10,
	UVWMAP_HEIGHT = 11,
	UVWMAP_AXIS = 12
};

// object.h mapping types for ApplyUVWMap
enum
{
	MAP_PLANAR = 0,
	MAP_CYLINDRICAL = 1,
	MAP_SPHERICAL = 2,
	MAP_BALL = 3,
	MAP_BOX = 4,
	MAP_FACE = 5
};

bool isUvwMapModifier(PIPELINE::MAX::CSceneClass *mod);

/// Minimal mesh view the applicator needs (avoids depending on SEvalMesh).
struct SMeshView
{
	std::vector<MAXMATH::Point3M> *Verts;
	/// Per-face corner indices into Verts (size = 3 * numFaces).
	std::vector<sint32> *FaceVerts; // [f*3+c] = vertex index
	/// Optional face normals (one per face) — used by MAP_BOX / capped cylindrical.
	std::vector<MAXMATH::Point3M> *FaceNormals; // may be NULL → computed on the fly
	/// Map channels: channel id → (UV verts, face corner indices into those UV verts).
	struct SChannel
	{
		std::vector<NLMISC::CVector> UVs;
		std::vector<sint32> FaceUVs; // [f*3+c]
	};
	std::map<int, SChannel> *Maps;
};

/// Apply UVW Map to \a mesh. Reads pblock + gizmo from \a mod and 0x2510 from \a modApp.
/// Returns false if the modifier cannot be decoded (caller should warn and skip).
/// \a typeMask gates which projection types are applied (bit 1<<type): types outside the mask
/// return false with \a outType set — the corpus-validated set grows per type (planar is pinned
/// by the Rectangle01 GT; the rest await their own corpus/GT validation).
bool applyUvwMap(PIPELINE::MAX::CSceneClass *mod, PIPELINE::MAX::CStorageContainer *modApp,
                 SMeshView &mesh, uint typeMask = 0xFFFFFFFF, int *outType = 0);

/// Pure projection (no scene wiring) — for tests and for reuse by parametric-prim UV paths.
/// \a tm maps object-space points into map space (row-vector: p' = p * tm).
/// Writes one UV per face corner into \a outUVs (size 3*numFaces) and sets face indices 0..N-1.
void applyUvwMapProjection(int type, float utile, float vtile, float wtile,
                           int uflip, int vflip, int wflip, int cap,
                           const MAXMATH::Matrix3M &tm, int channel,
                           SMeshView &mesh);

} /* namespace UVWMAP */

#endif /* PIPELINE_MAX_EXPORT_COMMON_UVW_MAP_MOD_H */

/* end of file */
