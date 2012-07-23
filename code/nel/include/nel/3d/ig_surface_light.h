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

#ifndef NL_IG_SURFACE_LIGHT_H
#define NL_IG_SURFACE_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/3d/surface_light_grid.h"


namespace NL3D
{


class	CInstanceGroup;


// ***************************************************************************
/**
 * Class owned by CInstanceGroup to get StaticLightSetup for dynamic models.
 *	It owns a map of SurfaceIdent -> CSurfaceLightGrid
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CIGSurfaceLight
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

public:
	struct	CRetrieverLightGrid
	{
		// There is localRetriver.getNumSurfaces() Grids.
		NLMISC::CObjectVector<CSurfaceLightGrid>	Grids;

		void	serial(NLMISC::IStream &f)
		{
			(void)f.serialVersion(0);
			f.serial(Grids);
		}
	};
	typedef std::map<uint32, CRetrieverLightGrid>	TRetrieverGridMap;
	typedef TRetrieverGridMap::iterator				ItRetrieverGridMap;

public:

	/// Constructor. A CIGSurfaceLight MUST be owned by an CInstanceGroup (nlassert(_Owner!=NULL));
	CIGSurfaceLight();
	void			setOwner(CInstanceGroup *owner);

	/**	build. called by CInstanceLighter.
	 *	\param retrieverGridMap is the gridMap builded by CInstanceLighter
	 *	\param cellSize is the size in meter of a cell
	 *	\param plRemap is the array returned by CInstanceGroup::build() to remap indices
	 */
	void			build(const TRetrieverGridMap &retrieverGridMap, float cellSize,
		const std::vector<uint>	&plRemap);

	/// clear.
	void			clear();

	// serial
	void			serial(NLMISC::IStream &f);

	/** Get StaticLightSetup Infos from the grid
	 *	\param retrieverIdentifier value returned by CGlobalRetriever::getLocalRetrieverId(globalPos).
	 *	\param surfaceId the id of the surface in the LocalRetriever
	 *	\param localPos position to retrieve info.
	 *	Position local to the LocalRetriever!! ie gp.LocalPosition.Estimated
	 *	\param pointLightList list of pointlight to append pointLight influecnes
	 *	\return false if retrieverIdentifier / surfaceId is not found. In this case pointLightList is not
	 *	modified, and sunContribution is set to 255, and localAmbient is set to 0
	 */
	bool			getStaticLightSetup(NLMISC::CRGBA sunAmbient, uint retrieverIdentifier, sint surfaceId, const CVector &localPos,
		std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient);


	float			getCellSize() const {return _CellSize;}
	float			getOOCellSize() const {return _OOCellSize;}

	// debug
	const TRetrieverGridMap &getRetrieverGridMap() const {return _RetrieverGridMap;}

// *********************
private:
	friend	class	CSurfaceLightGrid;

private:
	CInstanceGroup		*_Owner;
	float				_CellSize;
	float				_OOCellSize;
	TRetrieverGridMap	_RetrieverGridMap;

};


} // NL3D


#endif // NL_IG_SURFACE_LIGHT_H

/* End of ig_surface_light.h */
