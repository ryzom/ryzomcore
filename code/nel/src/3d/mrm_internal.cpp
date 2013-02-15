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

#include "nel/3d/mrm_internal.h"
#include "nel/misc/common.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
sint	CMRMSewingMesh::mustCollapseEdge(uint lod, const CMRMEdge &edge, uint &vertToCollapse) const
{
	nlassert(lod<_Lods.size());
	for(uint i=0;i<_Lods[lod].EdgeToCollapse.size();i++)
	{
		if(edge==_Lods[lod].EdgeToCollapse[i])
		{
			// the vertex which must be collapsed is v0.
			vertToCollapse= _Lods[lod].EdgeToCollapse[i].v0;
			return i;
		}
	}
	// not found
	return -1;
}


// ***************************************************************************
sint	CMRMSewingMesh::getNumCollapseEdge(uint lod) const
{
	nlassert(lod<_Lods.size());
	return (sint)_Lods[lod].EdgeToCollapse.size();
}


// ***************************************************************************
void	CMRMSewingMesh::build(const CMesh::CInterface &meshInt, uint nWantedLods, uint divisor)
{
	/* The polygon is MRM-egde-like reduced (pop an edge when needed)
		At each lod we store what edge is collapsed.
	*/
	_Lods.clear();
	_Lods.resize(nWantedLods);

	// build edge list
	std::vector<CMRMEdge>	edgeList;
	uint	nMaxEdges= (uint)meshInt.Vertices.size();
	edgeList.resize(nMaxEdges);
	for(uint i=0;i<nMaxEdges;i++)
	{
		edgeList[i].v0= i;
		edgeList[i].v1= (i+1)%nMaxEdges;
	}

	// build how many edges the coarsest lod will have. At least 3 edge for a correct cross section
	sint	nBaseEdges= nMaxEdges/divisor;
	nBaseEdges=max(nBaseEdges,3);

	// must fill all LODs, from end to start. do not proces last lod since it will be the coarsest mesh (no collapse)
	for(uint lod=nWantedLods-1;lod>0;lod--)
	{
		// Linear.
		sint	nCurEdges= (sint)floor( 0.5f + nBaseEdges + (nMaxEdges-nBaseEdges) * (float)(lod-1)/(nWantedLods-1) );
		nCurEdges=max(nCurEdges,3);

		// the current edge list is reduced until same size as wanted
		while(nCurEdges<(sint)edgeList.size())
		{
			// search the smallest edge
			float	bestDist= FLT_MAX;
			uint	bestEdgeId= 0;
			for(uint j=0;j<edgeList.size();j++)
			{
				uint	precEdgeId= (uint)((j + edgeList.size() -1) % edgeList.size());
				CVector	edgeDelta= (meshInt.Vertices[edgeList[j].v1].Pos - meshInt.Vertices[edgeList[j].v0].Pos);

				// compute dist between 2 verts
				float	dist= edgeDelta.norm();

				// compute dir of prec and cur edges
				CVector	curEdgeDir= edgeDelta;
				CVector	precEdgeDir= (meshInt.Vertices[edgeList[precEdgeId].v1].Pos - meshInt.Vertices[edgeList[precEdgeId].v0].Pos);
				curEdgeDir.normalize();
				precEdgeDir.normalize();

				// compute how linear they are from -1 to 1.
				float	angularPart= curEdgeDir * precEdgeDir;

				/* The more the edge is linear to the previous edge, the more we can collapse it.
					If totaly linear (curEdgeDir*precEdgeDir==1), it should be a good idea to collapse it now.
					But the fact is that this is an interface and we don't know what kind of mesh share those edges
					(I mean it is possible that neighbors faces are absolutely not coplanar).
					So don't add to much importance on Angular Part
				*/
				const float	angularBias= 1;	// if 0, 2 linear edges will collapse asap.
				dist*= angularBias + 1-angularPart;

				// take min dist
				if(dist<bestDist)
				{
					bestDist= dist;
					bestEdgeId= j;
				}
			}

			// mark as remove it
			_Lods[lod].EdgeToCollapse.push_back(edgeList[bestEdgeId]);
			// changes vert ids of the prec edge. eg: edge(12) is deleted=> 01 12 23... becomes 02 23 (NB: 1 is collapsed to 2)
			uint	precEdgeId= (uint)((bestEdgeId+edgeList.size()-1)%edgeList.size());
			edgeList[precEdgeId].v1= edgeList[bestEdgeId].v1;
			// and erase the edge from the current list
			edgeList.erase( edgeList.begin()+bestEdgeId );
		}
	}

}


} // NL3D
