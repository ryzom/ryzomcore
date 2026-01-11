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

#include "std3d.h"

#include "nel/3d/ig_surface_light.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************
CIGSurfaceLight::CIGSurfaceLight()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_Owner= NULL;
	_CellSize= 1;
	_OOCellSize= 1;
}

// ***************************************************************************
void			CIGSurfaceLight::setOwner(CInstanceGroup *owner)
{
	nlassert(owner!=NULL);
	_Owner= owner;
}

// ***************************************************************************
void			CIGSurfaceLight::build(const TRetrieverGridMap &retrieverGridMap, float cellSize,
	const std::vector<uint>	&plRemap)
{
	_RetrieverGridMap= retrieverGridMap;
	nlassert(cellSize>0);
	_CellSize= cellSize;
	_OOCellSize= 1.f/_CellSize;

	// remap indices.
	ItRetrieverGridMap	it;
	for(it= _RetrieverGridMap.begin(); it!=_RetrieverGridMap.end(); it++)
	{
		// For all grids of this retriever
		for(uint iGrid= 0; iGrid<it->second.Grids.size(); iGrid++)
		{
			CSurfaceLightGrid	&grid= it->second.Grids[iGrid];

			// For all cells of this grid.
			for(uint iCell= 0; iCell<grid.Cells.size(); iCell++)
			{
				CSurfaceLightGrid::CCellCorner	&cell= grid.Cells[iCell];

				// For all point light id.
				for(uint lid= 0; lid<CSurfaceLightGrid::NumLightPerCorner; lid++)
				{
					if(cell.Light[lid] == 0xFF)
						break;
					else
						cell.Light[lid]= plRemap[cell.Light[lid]];
				}

				// remap ambient light
				if(cell.LocalAmbientId!=0xFF)
					cell.LocalAmbientId= plRemap[cell.LocalAmbientId];
			}
		}
	}
}

// ***************************************************************************
void			CIGSurfaceLight::clear()
{
	_RetrieverGridMap.clear();
	_CellSize= 1;
	_OOCellSize= 1;
}

// ***************************************************************************
void			CIGSurfaceLight::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	/*
	Version 1:
		- The retriever grid map is now a map<uint,CRetrieverLightGrid>. Discard compatibility but.
	*/
	sint	ver= f.serialVersion(1);

	f.serial(_CellSize);
	f.serial(_OOCellSize);
	if(ver<1)
	{
		std::map<std::string, CRetrieverLightGrid>		oldFormatRetrieverGridMap;
		f.serialCont(oldFormatRetrieverGridMap);
		_RetrieverGridMap.clear();
	}
	else
	{
		f.serialCont(_RetrieverGridMap);
	}
}

// ***************************************************************************
bool			CIGSurfaceLight::getStaticLightSetup(NLMISC::CRGBA sunAmbient, uint retrieverIdentifier, sint surfaceId, const CVector &localPos,
	std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &localAmbient)
{
	nlassert(_Owner);

	// default
	sunContribution= 255;
	localAmbient= sunAmbient;

	ItRetrieverGridMap	it;
	it= _RetrieverGridMap.find(retrieverIdentifier);
	// If bad ident
	if(it==_RetrieverGridMap.end())
		return false;
	CRetrieverLightGrid		&rlg= it->second;
	// if bad surfaceId
	if(surfaceId<0 || surfaceId>= (sint)rlg.Grids.size())
		return false;

	// Else, ok, get it.
	rlg.Grids[surfaceId].getStaticLightSetup(sunAmbient, localPos, pointLightList, sunContribution, *this, localAmbient);
	return true;
}




} // NL3D
