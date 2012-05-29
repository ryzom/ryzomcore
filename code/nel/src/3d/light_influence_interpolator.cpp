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

#include "nel/3d/light_influence_interpolator.h"
#include "nel/misc/debug.h"
#include "nel/3d/point_light_named.h"


namespace NL3D
{


// ***************************************************************************
void		CLightInfluenceInterpolator::interpolate(std::vector<CPointLightInfluence> &pointLightList, float subX, float subY)
{
	uint	crn;
	// UnRolled loops.
	nlassert(NumLightPerCorner==2);

	// Reset index for each light.
	for(crn= 0; crn<4; crn++)
	{
		CCorner		&corner= Corners[crn];
		// UnRolled.
		if(corner.Lights[0])
			corner.Lights[0]->_IdInInfluenceList= -1;
		if(corner.Lights[1])
			corner.Lights[1]->_IdInInfluenceList= -1;
	}

	// Compute biLinear influence on each corner
	Corners[0].Influence= (1-subX) * (1-subY);
	Corners[1].Influence= subX * (1-subY);
	Corners[2].Influence= (1-subX) * subY;
	Corners[3].Influence= subX * subY;

	// For each light of each corner
	for(crn= 0; crn<4; crn++)
	{
		CCorner		&corner= Corners[crn];
		// UnRolled.
		// light 0.
		if(corner.Lights[0])
		{
			if(corner.Lights[0]->_IdInInfluenceList==-1)
			{
				// append a PointLightInfluence
				pointLightList.push_back(CPointLightInfluence());
				sint	id= (sint)pointLightList.size()-1;
				// setup the PointLightInfluence
				corner.Lights[0]->_IdInInfluenceList= id;
				pointLightList[id].PointLight= corner.Lights[0];
				pointLightList[id].Influence= corner.Influence;
			}
			else
			{
				// get the PointLightInfluence
				sint	id= corner.Lights[0]->_IdInInfluenceList;
				// increment the influence of the PointLightInfluence
				pointLightList[id].Influence+= corner.Influence;
			}
		}
		// light 1.
		if(corner.Lights[1])
		{
			if(corner.Lights[1]->_IdInInfluenceList==-1)
			{
				// append a PointLightInfluence
				pointLightList.push_back(CPointLightInfluence());
				sint	id= (sint)pointLightList.size()-1;
				// setup the PointLightInfluence
				corner.Lights[1]->_IdInInfluenceList= id;
				pointLightList[id].PointLight= corner.Lights[1];
				pointLightList[id].Influence= corner.Influence;
			}
			else
			{
				// get the PointLightInfluence
				sint	id= corner.Lights[1]->_IdInInfluenceList;
				// increment the influence of the PointLightInfluence
				pointLightList[id].Influence+= corner.Influence;
			}
		}
	}
}



} // NL3D
