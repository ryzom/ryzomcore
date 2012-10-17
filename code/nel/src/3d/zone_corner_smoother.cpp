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

#include "nel/3d/zone_corner_smoother.h"
#include "nel/3d/patchuv_locator.h"


namespace NL3D {


// ***************************************************************************
CZoneCornerSmoother::CZoneCornerSmoother()
{
}


// ***************************************************************************
void	CZoneCornerSmoother::buildPatchBindInfo(CPatch &pa, const CZone::CPatchConnect &pc, bool smoothEdge[4], bool cornerOnBind[4])
{
	uint	edge, corner;

	/*
		Some terminology here: an edge is supposed going from Corner=edge, to corner=(edge+1)&3.
		eg: edge 0 goes from corner0 to corner1.
	*/

	for(corner=0; corner<4; corner++)
		cornerOnBind[corner]= false;

	// for 4 edges.
	for(edge=0; edge<4; edge++)
	{
		// Is this edge smoothed??
		smoothEdge[edge]= pa.getSmoothFlag(edge);

		// build bindInfo.
		CPatch::CBindInfo	bindInfo;
		CPatchUVLocator		patchUvLocator;
		pa.getBindNeighbor(edge, bindInfo);
		// if neighbor(s) is present.
		if(bindInfo.Zone)
		{
			patchUvLocator.build(&pa, edge, bindInfo);
			// if not sameEdgeOnOrder (NB: all special cases of bind 1/X X/1 managed :) ), not smoothed!
			if( !patchUvLocator.sameEdgeOrder() )
				smoothEdge[edge]= false;

			// Manage bind 1/4 for the 2 patchs on the center of the bind.
			if(bindInfo.MultipleBindNum==4 && (bindInfo.MultipleBindId==1 || bindInfo.MultipleBindId==2) )
			{
				// easy, this edge starts and ends on a bind...
				cornerOnBind[edge]= true;
				cornerOnBind[(edge+1)&3]= true;
			}
			// else for case bind 1/2, and for case of patch 0 and patch 3 of the bind 1/4.
			else if(bindInfo.MultipleBindNum>=2)
			{
				// Beware of the mirroring!! (make a draw...)
				/*
					----------|-----------
						      |
						      | |
						    1 | |
						  ^   | v
						  |   |
						  |   *-----------
						  |   |
						  |   | |
						    0 | |
						      | v
						      |
					----------|-----------
				*/
				// If we are the patch0 on the neighbor, then we start on a bind, else we ends.
				if(bindInfo.MultipleBindId==0)
					cornerOnBind[edge]= true;
				else
					cornerOnBind[(edge+1)&3]= true;
			}

		}
	}
}


// ***************************************************************************
void	CZoneCornerSmoother::updateVertex(uint idVert, uint corner, bool smoothEdge[4], bool cornerOnBind[4])
{
	// get or insert into map (with default).
	CVertexSmoothInfo	&vert= VertexMap[idVert];

	// inc the number of patch binded to this point.
	vert.NPatchShared++;

	// get the smooth flag of edge before and after this corner.
	uint	e0= (4+corner-1)&3;
	uint	e1= corner;
	// if any one of those edge is not smoothed, then this vertex is not smoothed.
	if( !smoothEdge[e0] || !smoothEdge[e1] )
		vert.Smoothed= false;


	// Are we a vertex on a bind??
	if(cornerOnBind[corner])
		vert.VertexOnBind= true;
}


// ***************************************************************************
void	CZoneCornerSmoother::computeAllCornerSmoothFlags(CZone *zone, std::vector<CZone*> neighborZones)
{
	nlassert(zone);
	sint	npatchs= zone->getNumPatchs();
	sint	i;

	VertexMap.clear();
	IdVertexMap.clear();

	// for all patchs of the center zone, build the vertexMap.
	//==================
	for(i=0; i<npatchs; i++)
	{
		CPatch						&pa= (CPatch&)*(((const CZone*)zone)->getPatch(i));
		const CZone::CPatchConnect	&pc= *(zone->getPatchConnect(i));
		uint	corner;

		// build bind info for 4 edges and 4 vertices.
		bool	smoothEdge[4];
		bool	cornerOnBind[4];
		buildPatchBindInfo(pa, pc, smoothEdge, cornerOnBind);

		// for 4 corners.
		for(corner=0; corner<4; corner++)
		{
			// get the vertex id for this patch.
			uint	idVert= pc.BaseVertices[corner];

			// update this vertex smooth info.
			updateVertex(idVert, corner, smoothEdge, cornerOnBind);

			// for Bind with neighbor zones, must insert it in the map CTessVertex* -> VertexId.
			IdVertexMap[pa.getCornerVertex(corner)]= idVert;
		}
	}


	// for all patchs of all neigbhors zone, update for vertices that are connected to the centerZone.
	//==================
	for(uint nbZone=0; nbZone<neighborZones.size(); nbZone++)
	{
		CZone	*neighborZone= neighborZones[nbZone];
		nlassert(neighborZone);
		for(i=0; i<neighborZone->getNumPatchs(); i++)
		{
			CPatch						&pa= (CPatch&)*(((const CZone*)neighborZone)->getPatch(i));
			const CZone::CPatchConnect	&pc= *(neighborZone->getPatchConnect(i));
			uint	corner;

			// build bind info for 4 edges and 4 vertices.
			bool	smoothEdge[4];
			bool	cornerOnBind[4];
			buildPatchBindInfo(pa, pc, smoothEdge, cornerOnBind);

			// for 4 corners.
			for(corner=0; corner<4; corner++)
			{
				// try to find the vertex of the centerZone binded to this corner.
				ItIdVertexMap	it= IdVertexMap.find(pa.getCornerVertex(corner));

				// If this patch is binded on a vertex of the centerZone, must update this vertex.
				if(it != IdVertexMap.end())
				{
					// get the vertex id for this patch.
					uint	idVert= it->second;

					// update this vertex smooth info.
					updateVertex(idVert, corner, smoothEdge, cornerOnBind);
				}

			}
		}
	}


	// for all patchs of the center zone, build the finalSmooth.
	//==================
	for(i=0; i<npatchs; i++)
	{
		CPatch						&pa= (CPatch&)*(((const CZone*)zone)->getPatch(i));
		const CZone::CPatchConnect	&pc= *(zone->getPatchConnect(i));
		uint	corner;

		// for 4 corners.
		for(corner=0; corner<4; corner++)
		{
			uint	idVert= pc.BaseVertices[corner];
			// get from map.
			CVertexSmoothInfo	&vert= VertexMap[idVert];

			// the vertex is smoothed if all edges around him are smoothed, AND
			// if it has 4 patchs around him or if it is a bind.
			bool	finalSmooth;
			finalSmooth= vert.Smoothed && (vert.VertexOnBind || vert.NPatchShared==4);

			// update the patch.
			pa.setCornerSmoothFlag(corner, finalSmooth);
		}

	}


}



} // NL3D
