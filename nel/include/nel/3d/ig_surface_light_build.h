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

#ifndef NL_IG_SURFACE_LIGHT_BUILD_H
#define NL_IG_SURFACE_LIGHT_BUILD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"
#include "nel/3d/surface_light_grid.h"
#include "nel/3d/mesh.h"


namespace NL3D
{

	class CInstanceGroup;

// ***************************************************************************
/**
 * Class used by NL3D::CInstanceLighter to build CIGSurfaceLight with PACS surfaces
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CIGSurfaceLightBuild
{
public:

	enum	{MaxOverSamples= 16};

public:

	/** A surface cell corner Information.
	 */
	struct	CCellCorner
	{
		// Is this CellCorner in the surface polygon or not?
		bool				InSurface;
		// Computed by CInstanceLighter
		bool				Dilated;
		// Copy of SunContribution computed by CInstanceLighter.
		uint8				SunContribution;
		// Number of overSamples. At least one if InSurface. NB: Put here for packing.
		uint8				NumOverSamples;

		// World Position of this corner.
		NLMISC::CVector		CenterPos;

		// OverSamples. NB: there can be any number of overSamples (3, 4, 5,7 ...) because
		// some may be out of surface and thus disabled
		NLMISC::CVector		OverSamples[MaxOverSamples];


		/** This is Temp Light information used during CInstanceLighter::light().
		 *	Yes, void* is ugly, but it is to avoid too much dependencies.
		 */
		void				*LightInfo[CSurfaceLightGrid::NumLightPerCorner];
		void				*LocalAmbientLight;
	};


	/** A surface Lighting Information. Filled by CInstanceLighter
	 */
	struct	CSurface
	{
		// Origin and Size of the grid.
		// NB: The grid must be valid an not empty (ie Width>=2 and Height>=2).
		NLMISC::CVector2f			Origin;
		uint32						Width;
		uint32						Height;
		// The CellCorner info.
		std::vector<CCellCorner>	Cells;
	};


	/** A LocalRetriever Lighting Information.
	 *
	 */
	struct	CRetrieverLightGrid
	{
		// There is localRetriver.getNumSurfaces() Grids.
		std::vector<CSurface>		Grids;
	};
	typedef std::map<uint, CRetrieverLightGrid>		TRetrieverGridMap;
	typedef TRetrieverGridMap::iterator				ItRetrieverGridMap;


	// The requested CellSize (setuped by CInstanceLighter)
	float					CellSize;
	// Array of surfaces retrieved in PACS for an IG
	TRetrieverGridMap		RetrieverGridMap;


public:

	/// Debug: build a colored Grid mesh of SunContribution.
	void			buildSunDebugMesh(CMesh::CMeshBuild &meshBuild, CMeshBase::CMeshBaseBuild &meshBaseBuild, const NLMISC::CVector &deltaPos= NLMISC::CVector::Null);

	/// Debug: build a colored Grid mesh of PointLight. R= pointLight1 id. G= PointLight2 id. B= The multiplier used to show Ids.
	void			buildPLDebugMesh(CMesh::CMeshBuild &meshBuild, CMeshBase::CMeshBaseBuild &meshBaseBuild, const NLMISC::CVector &deltaPos, const CInstanceGroup &igOut);

private:
	void			addDebugMeshFaces(CMesh::CMeshBuild &meshBuild, CSurface &surface, uint vId0,
		const std::vector<NLMISC::CRGBA>	&colors);

};


} // NL3D


#endif // NL_IG_SURFACE_LIGHT_BUILD_H

/* End of ig_surface_light_build.h */
