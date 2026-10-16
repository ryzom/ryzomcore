// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef NL_MESH_LIGHTMAPPER_H
#define NL_MESH_LIGHTMAPPER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/lightmap_scene.h"

#include <string>

namespace NL3D
{

/**
 * Standalone mesh lightmapper — the lightmap computation the 3ds Max export plugin ran at
 * export time (nel_mesh_lib calc_lm/calc_lm_plane/calc_lm_rt), ported onto pure NeL types.
 * Operates on a CLightmapScene (the 1_export scene-graph intermediate): per receiver
 * geometry, computes the lightmap layers (one per light group/animation), packs them into
 * lightmap textures written to disk, rewrites the receiver's lightmap UV channel to the
 * packed atlas, and sets up the LightMap-shaded materials (textures, LMC compression,
 * light info map). The receiver's CMeshBuild/CMeshBaseBuild come out ready for the normal
 * shape build (CMesh::build & co — which is why the input is pre-build state).
 *
 * The algorithm is a faithful port of the original: same face grouping (material,
 * smoothing group, planarity), same lumel-space normalization, same plane packing, same
 * two-pass lighting with oversampling, same raytraced shadows with transparency sampling.
 * Do not "improve" it here — its output is validated against the reference export data.
 *
 * Not thread-safe (module-level state, like the original).
 */
class CMeshLightmapper
{
public:
	/// Build-step options (from the project build configuration, not from the scene file)
	struct COptions
	{
		/// Compute raytraced shadows
		bool			bShadow;
		/// Lumel size in world units (0.25 = 4 lumels per meter)
		float			rLumelSize;
		/// Oversampling factor (1, 2, 4 or 8)
		sint32			nOverSampling;
		/// Lighting mode: 0 = normal, 1 = soft shadows on directional lights
		sint32			nExportLighting;
		/// Output directory for the generated lightmap textures
		std::string		sExportLighting;
		/// Nearest-filter debug mode flag stored on the generated textures
		bool			bShowLumel;
		/// Write the per-lightmap light-list log file next to the textures
		bool			OutputLightmapLog;

		COptions()
		{
			bShadow = false;
			rLumelSize = 0.25f;
			nOverSampling = 1;
			nExportLighting = 0;
			bShowLumel = false;
			OutputLightmapLog = false;
		}
	};

	/** Compute and apply the lightmap of one receiver geometry.
	 *	\param pMB the geometry's mesh build (UV channel 1 in, atlas UVs out)
	 *	\param pMBB the receiver's base build (LightMap materials set up in place)
	 *	\param scene the scene-graph intermediate (lights + occluders)
	 *	\param geom the receiver geometry entry of pMB (per-node appdata, RT exclusions)
	 *	\param options the build-step options
	 *	\return true if a lightmap was computed and applied, false if the geometry has no
	 *	lightmapped faces or no valid lightmap mapping (same conditions as the original).
	 */
	static bool calculateLM(CMesh::CMeshBuild *pMB, CMeshBase::CMeshBaseBuild *pMBB,
		const CLightmapScene &scene, const CLightmapReceiverGeom &geom,
		const COptions &options);

	/// The object-to-world matrix of a mesh build (default transform composition)
	static NLMISC::CMatrix getObjectToWorldMatrix(const CMesh::CMeshBuild *pMB,
		const CMeshBase::CMeshBaseBuild *pMBB);

	/// Transform a mesh build's vertices and normals to world coordinates (in place)
	static void convertToWorldCoordinate(CMesh::CMeshBuild *pMB,
		CMeshBase::CMeshBaseBuild *pMBB,
		const NLMISC::CVector &translation = NLMISC::CVector(0.0f, 0.0f, 0.0f));
};

} // NL3D

#endif // NL_MESH_LIGHTMAPPER_H

/* End of mesh_lightmapper.h */
