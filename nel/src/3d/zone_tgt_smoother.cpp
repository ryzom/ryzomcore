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

#include "nel/3d/zone_tgt_smoother.h"
#include "nel/misc/plane.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{



// ***************************************************************************
void		CZoneTgtSmoother::makeVerticesCoplanar(std::vector<CZoneInfo>  &zones)
{
	sint	i,j, numZone;
	sint	centerZoneId;

	nlassert(zones.size()>=1);
	centerZoneId= zones[0].ZoneId;

	// 0. CenterZone.
	//===============
	// First, make connectivity patch/vertex
	for(i=0;i<(sint)zones[0].Patchs.size();i++)
	{
		CPatchInfo		&pa= zones[0].Patchs[i];

		for(j=0;j<4;j++)
		{
			sint	vtx= pa.BaseVertices[j];
			CPatchId	pid;
			pid.ZoneId= centerZoneId;
			pid.PatchId= i;
			pid.Patch= &pa;
			pid.IdVert= j;
			VertexMap[vtx].Patchs.push_back(pid);
		}
	}
	// Second, what vertices of this zone are one border?
	for(i=0;i<(sint)zones[0].BorderVertices.size();i++)
	{
		CBorderVertex	&bv= zones[0].BorderVertices[i];
		sint	vtx= bv.CurrentVertex;
		VertexMap[vtx].OnBorder= true;
	}

	// 1. Neighbor zones.
	//===================
	map<sint, sint>		tempMap;
	for(numZone= 1; numZone<(sint)zones.size(); numZone++)
	{
		sint	adjZoneId= zones[numZone].ZoneId;

		tempMap.clear();
		// Tests which vertices points on the center zone.
		for(i=0;i<(sint)zones[numZone].BorderVertices.size();i++)
		{
			CBorderVertex	&bv= zones[numZone].BorderVertices[i];

			if(bv.NeighborZoneId== centerZoneId)
			{
				tempMap[bv.CurrentVertex]= bv.NeighborVertex;
			}
		}
		// Tests patchs which points on center zone.
		for(i=0;i<(sint)zones[numZone].Patchs.size();i++)
		{
			CPatchInfo		&pa= zones[numZone].Patchs[i];

			for(j=0;j<4;j++)
			{
				sint	vtx= pa.BaseVertices[j];
				if(tempMap.find(vtx)!=tempMap.end())
				{
					CPatchId	pid;
					pid.ZoneId= adjZoneId;
					pid.PatchId= i;
					pid.Patch= &pa;
					pid.IdVert= j;
					// Fill the vertex of the center zone.
					VertexMap[tempMap[vtx]].Patchs.push_back(pid);
				}
			}
		}

	}

	// 2. Process each vertex.
	//========================
	ItVertexMap		itVert;
	for(itVert= VertexMap.begin(); itVert!=VertexMap.end(); itVert++)
	{
		CVertexInfo		&vert= itVert->second;

		// a. verify if coplanar is possible.
		//===================================

		// \todo yoyo: later: do it too on non border vertices if wanted (with a normal threshold...).
		if(!vert.OnBorder)
			continue;
		// \todo yoyo: later: formula with 3, 5 ... patchs around the vertex.
		if(vert.Patchs.size()!=4)
			continue;

		// Test if there is no bind 1/x on this patch, around this vertex.
		// \todo yoyo: later: binds should works...
		std::list<CPatchId>::iterator	itPatch;
		bool	bindFound= false;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			// Tests the two edges around the vertex (before: e0, and after: e1).
			sint	e0= (itPatch->IdVert+4-1)%4;
			sint	e1= itPatch->IdVert;

			if(itPatch->Patch->BindEdges[e0].NPatchs!= 1 || itPatch->Patch->BindEdges[e1].NPatchs!= 1)
			{
				bindFound= true;
				break;
			}
		}
		if(bindFound)
			continue;



		// b. maps patchs on tangents.
		//=========================
		vector<CTangentId>	tangents;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			// The edges, before and after the veterx.
			sint	edgeNum[2]= {(itPatch->IdVert+4-1)%4, itPatch->IdVert };
			// The tangents, before and after the veterx.
			sint	tgtNum[2]=  {(itPatch->IdVert*2+8-1)%8, itPatch->IdVert*2 };

			// For the 2 edges around this vertex.
			for(sint ed= 0; ed<2;ed++)
			{
				sint	patchId, zoneId, edgeId;
				sint	tgt;

				// get neighbor edge id.
				zoneId= pa.BindEdges[ edgeNum[ed] ].ZoneId;
				patchId= pa.BindEdges[ edgeNum[ed] ].Next[0];
				edgeId= pa.BindEdges[ edgeNum[ed] ].Edge[0];
				// Search if tangent already inserted, mapped to this "neighbor edge".
				for(tgt= 0; tgt<(sint)tangents.size();tgt++)
				{
					if(tangents[tgt].ZoneId==zoneId && tangents[tgt].PatchId==patchId && tangents[tgt].EdgeId==edgeId)
						break;
				}
				// If not found, add the tangent, and map ME to it.
				if(tgt==(sint)tangents.size())
				{
					CTangentId	tangent;
					// Set OUR edge Id.
					tangent.ZoneId= itPatch->ZoneId;
					tangent.PatchId= itPatch->PatchId;
					tangent.EdgeId= edgeNum[ed];
					// Get the tangent, before or after the vertex.
					tangent.Tangent= pa.Patch.Tangents[ tgtNum[ed] ];
					// Which patchs this edge share. (0 is those which insert this tgt)
					tangent.Patchs[0]= &pa;
					tangents.push_back(tangent);
				}
				else
				{
					// Which patchs this edge share. (0 is those which access this tgt)
					tangents[tgt].Patchs[1]= &pa;
				}
				// Map the patch to this tangent.
				itPatch->Tangents[ed]= tgt;

			}
		}

		// There should be 4 tangents.
		if (tangents.size()!=4)
		{
			nlinfo ("ERROR: vertex %d should have 4 tangents. It got %d. (MAXINDICES +1!!)", itVert->first, tangents.size());
			continue;
		}


		// c. get the vertex.
		//===================
		CVector		vertexValue;
		itPatch= vert.Patchs.begin();
		vertexValue= itPatch->Patch->Patch.Vertices[itPatch->IdVert];


		// d. project the tangents.
		//=========================
		// better coplanar than Max... (with orthogonal angles: use p0/p1).
		for(i=0;i<(sint)tangents.size();i++)
		{
			// For following tangents, search the opposite.
			// Begin at i+1 so we are sure to do this only one time.
			for(j=i+1;j<(sint)tangents.size();j++)
			{
				if(tangents[i].isOppositeOf(tangents[j]))
				{
					CVector		&tgt0= tangents[i].Tangent;
					CVector		&tgt1= tangents[j].Tangent;
					// Colinear the tangents. Must keep the length of vectors.
					float		l0= (tgt0-vertexValue).norm();
					float		l1= (tgt1-vertexValue).norm();
					// Average the tangents. Normalize them before, to keep much as possible the orientation.
					CVector		d0= (vertexValue-tgt0).normed();
					CVector		d1= (tgt1-vertexValue).normed();
					CVector		dir= (d0+d1).normed();

					// Copy to tangents.
					tgt0= vertexValue-dir*l0;
					tgt1= vertexValue+dir*l1;
				}
			}
		}


		// e. assign tangents to patchs, rebuild interior.
		//==============================
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			// The tangents, before and after the vertex.
			sint	tgtNum[2]=  {(itPatch->IdVert*2+8-1)%8, itPatch->IdVert*2 };
			sint	t0= tgtNum[0];
			sint	t1= tgtNum[1];
			sint	smoothEdge0= pa.getSmoothFlag (t0/2);
			sint	smoothEdge1= pa.getSmoothFlag (t1/2);

			// Smooth this edge ?
			if (smoothEdge0)
				pa.Patch.Tangents[t0]= tangents[itPatch->Tangents[0]].Tangent;
			if (smoothEdge1)
				pa.Patch.Tangents[t1]= tangents[itPatch->Tangents[1]].Tangent;

			// Setup the coplanared interior. just the sum of 2 vector tangents.
			if (smoothEdge0&&smoothEdge1)
				pa.Patch.Interiors[itPatch->IdVert]= pa.Patch.Tangents[t0] + pa.Patch.Tangents[t1] - vertexValue;
		}
	}
}


/*
	// OLD CODE FOR 3DS MAX LIKE COPLANAR.
	// WORKS, BUT NOT AS GOOD AS REAL COPLANAR.

		// b. build the plane.
		//====================
		CVector		planeNormal(0,0,0);
		CVector		planeCenter;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			CVector			a,b,c, pvect;
			sint			t0= (itPatch->IdVert*2+8-1)%8;
			sint			t1= itPatch->IdVert*2;

			// CCW order.
			a= pa.Patch.Tangents[t0];
			b= pa.Patch.Vertices[itPatch->IdVert];
			c= pa.Patch.Tangents[t1];
			pvect= (b-a)^(c-b);
			planeNormal+= pvect.normed();

			// yes, done 4 times... :(
			planeCenter= b;
		}
		// Average of all normals...
		planeNormal.normalize();
		CPlane		plane;
		plane.make(planeNormal, planeCenter);


		// c. projects the tangents, rebuild interior.
		//============================================
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			sint			t0= (itPatch->IdVert*2+8-1)%8;
			sint			t1= itPatch->IdVert*2;
			pa.Patch.Tangents[t0]= plane.project(pa.Patch.Tangents[t0]);
			pa.Patch.Tangents[t1]= plane.project(pa.Patch.Tangents[t1]);

			// Setup the coplanared interior. just the sum of 2 vector tangents.
			pa.Patch.Interiors[itPatch->IdVert]= pa.Patch.Tangents[t0] + pa.Patch.Tangents[t1] - planeCenter;
		}
*/


} // NL3D
