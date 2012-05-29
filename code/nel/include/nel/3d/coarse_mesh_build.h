// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_COARSE_MESH_BUILD_H
#define NL_COARSE_MESH_BUILD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bitmap.h"

#include <vector>
#include <map>

namespace NL3D
{

class CMeshGeom;
class CMeshBase;
class ITexture;

/**
 * Class used to build the coarse meshes
 *
 * The build method take an array of CMeshGeom an a bitmap.
 * This class will compute a single texture used to map every
 * coarse mesh passed in the array.
 *
 * Each mesh geom should be a coarse mesh.
 *
 * The uv texture coordinates of those coarse meshes will be adjusted to fit
 * in a single texture.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCoarseMeshBuild
{
public:

	/// Coarse mesh descriptor
	class CCoarseMeshDesc
	{
	public:
		// Constructor
		CCoarseMeshDesc (CMeshGeom *meshGeom, const CMeshBase *baseMesh)
		{
			MeshGeom=meshGeom;
			MeshBase=baseMesh;
		}

		/**
		  * The pointer on the geom mesh of the coarse mesh.
		  * This mesh will be modified.
		  */
		CMeshGeom			*MeshGeom;

		/**
		  * The pointer on the base mesh used by the coarse mesh.
		  * This class won't be modified.
		  */
		const CMeshBase		*MeshBase;
	};

	/// Statistics about the build process
	class CStats
	{
	public:
		/**
		  * Ratio of texture used. Between 0~1. 0, the texture is empty, 1
		  * the texture is totaly used.
		  */
		float	TextureUsed;
	};

	/**
	  * The build method. This method will build a single texture and adjust
	  * the UV texture coordinates of each mesh geom.
	  *
	  * \param coarseMeshes is a vector of coarse mesh to compute.
	  * \param bitmap will receive the final bitmap.
	  * \param start will receive various statistics about the build process.
	  * \param mulArea is the mul factor used to increase to theorical global texture size.
	  */
	bool		build (const std::vector<CCoarseMeshDesc>& coarseMeshes, std::vector<NLMISC::CBitmap> &bitmaps, CStats& stats, float mulArea);

private:

	class CBitmapDesc
	{
	public:
		// Coordinates of the bitmap in the global bitmap
		float		U;
		float		V;

		// Coordinates ratio
		float		FactorU;
		float		FactorV;

		// Pointer on the textures
		std::vector<NLMISC::CBitmap>	Bitmaps;

		// Pointer on the texture
		std::string	Name;
	};

	typedef std::map<std::string, CBitmapDesc>	MapBitmapDesc;

	// Expand the border of a bitmap.
	void		expand (NLMISC::CBitmap& bitmap);

	// Build the bitmap. Return false if the bitmap size is too small
	bool		buildBitmap (const std::vector<CCoarseMeshDesc>& coarseMeshes, std::vector<NLMISC::CBitmap> &bitmaps, CStats& stats, MapBitmapDesc& desc, float mulArea);

	// Remap the coordinate
	void		remapCoordinates (const std::vector<CCoarseMeshDesc>& coarseMeshes, const MapBitmapDesc& desc, uint outputBitmapCount);
};


} // NL3D


#endif // NL_COARSE_MESH_BUILD_H

/* End of coarse_mesh_build.h */
