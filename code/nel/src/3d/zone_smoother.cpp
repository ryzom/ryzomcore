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

#include "nel/3d/zone_smoother.h"


namespace NL3D
{


// ***************************************************************************
bool		CZoneSmoother::smoothTangent(const CVector &tgt, const CVector &int0, const CVector &int1, CVector &tgtres)
{
	// First, test anglethreshold.
	CVector		dir0= tgt-int0;
	CVector		dir1= int1-tgt;
	float		norm0= dir0.norm();
	float		norm1= dir1.norm();
	dir0.normalize();
	dir1.normalize();
	// If not so colinear...
	if(dir0*dir1<_CosThreshold)
		return false;

	// Then smooth.
	if(_ContinuityC1)
	{
		tgtres= (int0+int1)/2;
	}
	else
	{
		// Make it colinear, but not at the middle.
		// Respect the old distance ratio.
		tgtres= (int0*norm1 + int1*norm0)/ (norm0+norm1);
	}


	return true;
}


// ***************************************************************************
void			CZoneSmoother::smoothTangents(CZoneInfo zones[5], float angleThreshold, bool continuityC1)
{
	sint i,j;

	nlassert(zones[0].Patchs);

	// Local info.
	_CosThreshold= (float)cos(angleThreshold);
	_ContinuityC1= continuityC1;

	// 0. fill local Zone map.
	//========================
	_Zones.clear();
	for(i=0;i<5;i++)
	{
		// If not NULL
		if(zones[i].Patchs)
		{
			_Zones.insert( TZoneInfoMap::value_type(zones[i].ZoneId, zones[i]) );
		}
	}

	// 1. Fot all patchs of zones[0].
	//===============================
	std::vector<CPatchInfo>		&patchs= *(zones[0].Patchs);
	for(i=0;i<(sint)patchs.size();i++)
	{
		CPatchInfo	&pat0= patchs[i];

		// For all edges.
		for(j=0;j<4;j++)
		{
			if (pat0.getSmoothFlag (i))
			{
				CPatchInfo::CBindInfo	&bd= pat0.BindEdges[j];
				// If normal bind (1/1), we can do the smooth.
				if(bd.NPatchs==1)
				{
					// Retrieve the good neighbor zone.
					TZoneInfoMap::iterator	itZone= _Zones.find(bd.ZoneId);
					// If zone here.
					if(itZone!=_Zones.end())
					{
						CZoneInfo	&zi= itZone->second;
						CPatchInfo	&pat1= (*zi.Patchs)[bd.Next[0]];
						// Here, we have the 2 patchs, and we must smooth 2 tangents.
						CVector		tgtRes;
						sint		edge0= j;
						sint		edge1= bd.Edge[0];

						// Make a draw to understand (see Bezier patchs conventions).
						// The key is: Patchs are always CCW, so must smooth the patchs one front of the other.

						// a. First tangent.
						//==================
						if(smoothTangent(pat0.Patch.Tangents[edge0*2], pat0.Patch.Interiors[edge0],
							pat1.Patch.Interiors[(edge1+1)%4], tgtRes))
						{
							// Set the result on the 2 patchs.
							pat0.Patch.Tangents[edge0*2]= tgtRes;
							pat1.Patch.Tangents[edge1*2+1]= tgtRes;
						}

						// b. Second tangent.
						//==================
						if(smoothTangent(pat0.Patch.Tangents[edge0*2+1], pat0.Patch.Interiors[(edge0+1)%4],
							pat1.Patch.Interiors[edge1], tgtRes))
						{
							// Set the result on the 2 patchs.
							pat0.Patch.Tangents[edge0*2+1]= tgtRes;
							pat1.Patch.Tangents[edge1*2]= tgtRes;
						}

					}
				}
			}
		}
	}

	// End!!
}


} // NL3D
