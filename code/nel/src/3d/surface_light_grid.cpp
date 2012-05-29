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

#include "nel/3d/surface_light_grid.h"
#include "nel/misc/common.h"
#include "nel/3d/ig_surface_light.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/light_influence_interpolator.h"
#include "nel/3d/point_light_named.h"
#include "nel/3d/scene_group.h"


using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
CSurfaceLightGrid::CSurfaceLightGrid()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	Width= 0;
	Height= 0;
}


// ***************************************************************************
void		CSurfaceLightGrid::serial(NLMISC::IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	(void)f.serialVersion(0);
	f.serial(Origin);
	f.serial(Width);
	f.serial(Height);
	f.serial(Cells);
}


// ***************************************************************************
void		CSurfaceLightGrid::getStaticLightSetup(NLMISC::CRGBA sunAmbient, const CVector &localPos, std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution,
	CIGSurfaceLight &igsl, NLMISC::CRGBA &localAmbient) const
{
	// Get local coordinate to the grid.
	float	xfloat= (localPos.x - Origin.x) * igsl.getOOCellSize();
	float	yfloat= (localPos.y - Origin.y) * igsl.getOOCellSize();
	sint	wCell= Width-1;
	sint	hCell= Height-1;
	// fastFloor: use a precision of 256 to avoid doing OptFastFloorBegin.
	sint	wfixed= wCell<<8;
	sint	hfixed= hCell<<8;
	sint	xfixed= NLMISC::OptFastFloor(xfloat * 256);
	sint	yfixed= NLMISC::OptFastFloor(yfloat * 256);
	clamp(xfixed, 0, wfixed);
	clamp(yfixed, 0, hfixed);
	// compute the cell coord, and the subCoord for bilinear.
	sint	xCell, yCell, xSub, ySub;
	xCell= xfixed>>8;
	yCell= yfixed>>8;
	clamp(xCell, 0, wCell-1);
	clamp(yCell, 0, hCell-1);
	// Hence, xSub and ySub range is [0, 256].
	xSub= xfixed - (xCell<<8);
	ySub= yfixed - (yCell<<8);


	// Use a CLightInfluenceInterpolator to biLinear light influence
	CLightInfluenceInterpolator		interp;
	// Must support only 2 light per cell corner.
	nlassert(CSurfaceLightGrid::NumLightPerCorner==2);
	nlassert(CLightInfluenceInterpolator::NumLightPerCorner==2);
	// Get ref on array of PointLightNamed.
	CPointLightNamed	*igPointLights= NULL;;
	if( igsl._Owner->getPointLightList().size() >0 )
	{
		// const_cast, because will only change _IdInfluence, and
		// also because CLightingManager will call appendLightedModel()
		igPointLights= const_cast<CPointLightNamed*>(&(igsl._Owner->getPointLightList()[0]));
	}
	// For 4 corners.
	uint	x,y;
	uint	sunContribFixed= 0;
	uint	rLocalAmbientFixed= 0;
	uint	gLocalAmbientFixed= 0;
	uint	bLocalAmbientFixed= 0;
	uint	wLocalAmbientFixed= 0;
	for(y=0;y<2;y++)
	{
		for(x=0;x<2;x++)
		{
			// Prepare compute for PointLights.
			//-------------
			// get ref on TLI, and on corner.
			const CCellCorner						&cellCorner= Cells[ (yCell+y)*Width + xCell+x ];
			CLightInfluenceInterpolator::CCorner	&corner= interp.Corners[y*2 + x];
			// For all lights
			uint lid;
			for(lid= 0; lid<CSurfaceLightGrid::NumLightPerCorner; lid++)
			{
				// get the id of the light in the ig
				uint	igLightId= cellCorner.Light[lid];
				// If empty id, stop
				if(igLightId==0xFF)
					break;
				else
				{
					// Set pointer of the light in the corner
					corner.Lights[lid]= igPointLights + igLightId;
				}
			}
			// Reset Empty slots.
			for(; lid<CSurfaceLightGrid::NumLightPerCorner; lid++)
			{
				// set to NULL
				corner.Lights[lid]= NULL;
			}

			// BiLinear SunContribution.
			//-------------
			uint	xBi= (x==0)?256-xSub : xSub;
			uint	yBi= (y==0)?256-ySub : ySub;
			uint	mulBi= xBi * yBi;
			sunContribFixed+= cellCorner.SunContribution * mulBi;


			// BiLinear Ambient Contribution.
			//-------------
			// If FF, then take Sun Ambient => leave color and alpha To 0.
			if(cellCorner.LocalAmbientId!=0xFF)
			{
				CPointLight	&pl= igPointLights[cellCorner.LocalAmbientId];
				// take current ambient from pointLight
				CRGBA	ambCorner= pl.getAmbient();
				// Add with sun this one?
				if(pl.getAddAmbientWithSun())
					ambCorner.addRGBOnly(ambCorner, sunAmbient);
				// bilinear
				rLocalAmbientFixed+= ambCorner.R * mulBi;
				gLocalAmbientFixed+= ambCorner.G * mulBi;
				bLocalAmbientFixed+= ambCorner.B * mulBi;
				// increase the weight influence of igPointLights
				wLocalAmbientFixed+= 256 * mulBi;
			}
		}
	}
	// interpolate PointLights.
	interp.interpolate(pointLightList, xSub/256.f, ySub/256.f);

	// Final SunContribution
	sunContribution= sunContribFixed>>16;

	// Final Ambient Contribution of Ambient Lights
	CRGBA	tempAmbient;
	tempAmbient.R= rLocalAmbientFixed>>16;
	tempAmbient.G= gLocalAmbientFixed>>16;
	tempAmbient.B= bLocalAmbientFixed>>16;
	// must interpolate between SunAmbient and tempAmbient
	uint	uAmbFactor= wLocalAmbientFixed>>16;
	// Blend, but tempAmbient.r/g/b is already multiplied by weight.
	localAmbient.modulateFromuiRGBOnly(sunAmbient, 256 - uAmbFactor);
	localAmbient.addRGBOnly(localAmbient, tempAmbient);

}



} // NL3D
