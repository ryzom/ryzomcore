// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


#include "../zone_lib/zone_utility.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <set>

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/common.h"
#include "nel/3d/quad_tree.h"
#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/3d/zone_smoother.h"
#include "nel/3d/zone_tgt_smoother.h"
#include "nel/3d/zone_corner_smoother.h"


using namespace NL3D;
using namespace NLMISC;
using namespace std;


#define WELD_LOG 1

FILE *fdbg;

std::string inputDir;
std::string inputExt;
std::string outputDir;
std::string outputExt;

float weldRadius = 1.1f;

/* Zone ID
	0   1   2
	3       4
	5   6   7
*/

// Define this to stop the welder on a source edge
// #define NL_DEBUG_WELD
#define NL_DEBUG_WELD_V0 (CVector(16320,-24064,0))
#define NL_DEBUG_WELD_V1 (CVector(16352,-24065,0))
#define NL_DEBUG_WELD_THRESHOLD 1.f

#ifdef NL_DEBUG_WELD
bool isTheSame (const CVector &v0, const CVector &v1)
{
	CVector delta = v0 - v1;
	delta.z = 0;
	return delta.norm() < NL_DEBUG_WELD_THRESHOLD;
}
#endif // NL_DEBUG_WELD

/**
 * CWeldableVertexInfos
 */
struct CWeldableVertexInfos
{
	uint16 IndexInZone; // base vertex
	sint PatchIndex;	// patch
	uint8 PatchVertex;	// 0,1,2,3

	CWeldableVertexInfos()
	{
		IndexInZone = 0;
		PatchIndex = 0;
		PatchVertex = 0;
	}

	bool operator< (const CWeldableVertexInfos& wvinf) const
	{
		if(IndexInZone<wvinf.IndexInZone)
			return true;
		if(IndexInZone>wvinf.IndexInZone)
			return false;
		if(PatchIndex<wvinf.PatchIndex)
			return true;
		if(PatchIndex>wvinf.PatchIndex)
			return false;
		return PatchVertex<wvinf.PatchVertex;
	}
};


struct	CVectorInfluence
{
	CVector		Vertex;
	float		Inf;
	bool		OnBorder;
};


struct	CAdjacentVertex
{
	CVector		Vertex;
	uint		IdOnCenterZone;
	bool		OnBorder;
};


/*******************************************************************\
						writeInstructions()
\*******************************************************************/
void writeInstructions()
{
	printf("zone_welder <input.zone><output.zone>[<weld threshold>]\n");
	printf("\t/? for this help\n");
}


/*******************************************************************\
						findPatchIndex()
\*******************************************************************/
bool getPatchAndEdge(const std::vector<CPatchInfo>& patchs,
					  uint16 baseVertex1, uint16 baseVertex2, 
					  uint16& patchIndex,
					  uint8& edgeIndex)
{
	uint ptch;
		
	for(ptch=0; ptch<patchs.size(); ptch++)
	{
		uint i;
		for(i=0; i<4; i++)
		{
			if(patchs[ptch].BaseVertices[i] == baseVertex1)
			{
#if WELD_LOG
				fprintf(fdbg,"patch %d continent bv %d : %d %d %d %d\n",
					ptch,
					baseVertex1,
					patchs[ptch].BaseVertices[0],
					patchs[ptch].BaseVertices[1],
					patchs[ptch].BaseVertices[2],
					patchs[ptch].BaseVertices[3]);
#endif
				
				if(patchs[ptch].BaseVertices[(i+1)%4] == baseVertex2)
				{
					edgeIndex = i;
					patchIndex = ptch;
					return true;
				}
				if(patchs[ptch].BaseVertices[(i-1)%4] == baseVertex2)
				{
					edgeIndex = (i-1)%4;
					patchIndex = ptch;
					return true;
				}
			}
		}
	}
	return false;
}

void CleanZone ( std::vector<CPatchInfo> &zoneInfos, uint zoneId, const CAABBoxExt &zoneBBox, float weldThreshold);

/*******************************************************************\
							weldZones()
\*******************************************************************/
void weldZones(const char *center)
{
	uint i,j;
	
	// load zone in the center
	CIFile zoneFile(inputDir+center+inputExt);
	CZone zone;
	zone.serial(zoneFile);
	zoneFile.close();
	
	// retrieving infos from the center zone
	uint16 centerZoneId = zone.getZoneId();
	std::vector<CPatchInfo> centerZonePatchs;
	std::vector<CBorderVertex> centerZoneBorderVertices;
	zone.retrieve(centerZonePatchs, centerZoneBorderVertices);

	std::vector<CPatchInfo>::iterator itptch;
	std::vector<CBorderVertex>::iterator itbv;

	// if no id yet, we add a correct id
	if(centerZoneId==0) 
	{
		centerZoneId = createZoneId(center);
		
		// edge neighbour : current zone
		for(itptch = centerZonePatchs.begin(); itptch!=centerZonePatchs.end(); itptch++)
		{
			for(j=0; j<4; j++)
			{
				(*itptch).BindEdges[j].ZoneId = centerZoneId;
			}
		}

		// border vertices neighbour : current zone
		for(itbv = centerZoneBorderVertices.begin(); itbv<centerZoneBorderVertices.end(); itbv++)
		{
			(*itbv).NeighborZoneId = centerZoneId;
		}
		
	}
#if WELD_LOG
	fprintf(fdbg,"id(center) = %d\n",centerZoneId);
#endif

	// ***	Clean internal zone
	// *	Bind 1-1 1-2 1-4 internal patches that are not binded
	// *	Make a global welded on vertices
	// *	Force tangents position
	CleanZone ( centerZonePatchs, centerZoneId, zone.getZoneBB(), weldRadius);

	// Yoyo was here: Smooth the tangents of the zone.
	//================================================
	// NB: do it only for edges sharing 2 patchs of centerZone. (don't care adjacent zones).
	// smoothing with adjacent zones is done with a better smoothing tool: CZoneTgtSmoother, see below, 
	// after the weld of the zone.
	{
		CZoneSmoother	zonesmoother;
		CZoneSmoother::CZoneInfo	smoothZones[5];
		smoothZones[0].ZoneId= centerZoneId;
		smoothZones[0].Patchs= &centerZonePatchs;
		// 30deg ???
		zonesmoother.smoothTangents(smoothZones, (float)(Pi/6));
	}



	// load 8 adjacent adjZones
	bool		adjZoneFileFound[8];
	CZone		adjZones[8];
	CZoneInfo	adjZoneInfos[8];
	uint16		adjZonesId[8];
	std::vector<std::string> adjZonesName;
	getAdjacentZonesName(center, adjZonesName);
	for(i=0; i<8; i++)
	{
		if(adjZonesName[i]=="empty") continue;
		
		adjZoneFileFound[i] = true;
		CIFile f;
		try
		{
			std::string ss(outputDir+adjZonesName[i]+outputExt);
			if (f.open(ss))
			{
				printf("reading file %s\n", ss.c_str());
				adjZones[i].serial(f);
				adjZones[i].retrieve(adjZoneInfos[i].Patchs, adjZoneInfos[i].BorderVertices);
				adjZoneInfos[i].ZoneId= adjZonesId[i] = adjZones[i].getZoneId();
				f.close();
			}
			else
			{
				// nlwarning ("WARNING File not found: %s\n", ss.c_str());
				adjZonesName[i]="empty";
			}
		}
		catch(const exception &e)
		{
			nlwarning ("ERROR %s\n", e.what ());
			adjZoneFileFound[i] = false;
		}
	}
	
	// QuadTree for storing adjZones points
	CQuadTree<CWeldableVertexInfos> quadTrees[8];
	
	// new base, to change from XZ to XY (Nel speaking)
	CMatrix	base;
	CVector	I(1,0,0);
	CVector	J(0,0,-1);
	CVector	K(0,1,0);
	base.setRot(I,J,K, true);

	
	
	uint ptch;

	uint16 weldCount = 0;

	// Error messages
	vector<string>		errorMessage;

	for(i=0; i<8; i++)
	{
		if(adjZonesName[i]=="empty") continue;
		if(!adjZoneFileFound[i]) continue;

		// setting quad tree
		uint qTreeDepth = 5;
		CAABBoxExt bb = adjZones[i].getZoneBB();
		quadTrees[i].create (5, bb.getCenter(), 2*bb.getRadius());
		quadTrees[i].changeBase(base);

		// retrieving infos from the current adjacent zone
		std::vector<CPatchInfo>		&adjZonePatchs= adjZoneInfos[i].Patchs;
		std::vector<CBorderVertex>	&adjZoneBorderVertices= adjZoneInfos[i].BorderVertices;


		// if no id yet, we add a correct id
		nlassert(adjZonesId[i]!=0);
		if(adjZonesId[i]==0) 
		{
			adjZonesId[i] = createZoneId(getName (adjZonesName[i]));
			adjZoneInfos[i].ZoneId= adjZonesId[i];
			
			// edge neighbour : current zone
			for(itptch = adjZonePatchs.begin(); itptch!=adjZonePatchs.end(); itptch++)
			{
				for(j=0; j<4; j++)
				{
					(*itptch).BindEdges[j].ZoneId = adjZonesId[i];
				}
			}

			// border vertices neighbour : current zone
			for(itbv = adjZoneBorderVertices.begin(); itbv!=adjZoneBorderVertices.end(); itbv++)
			{
				(*itbv).NeighborZoneId = adjZonesId[i];
			}
			
		}
#if WELD_LOG
		fprintf(fdbg,"------------------------------------------\n");
		fprintf(fdbg,"id(%d) = %d\n",i,adjZonesId[i]);
#endif

		// an edge of current adjacent patch with neighbour zoneId==center zoneId is
		// set to no neighbour.
		for(ptch = 0; ptch<adjZonePatchs.size(); ptch++)
		{
			for(j=0; j<4; j++)
			{
				if(adjZonePatchs[ptch].BindEdges[j].ZoneId == centerZoneId)
				{
					adjZonePatchs[ptch].BindEdges[j].NPatchs = 0;
				}
			}
		}

		fprintf(fdbg,"(before) zone %u bordervertices size : %u\n",i,(uint)adjZoneBorderVertices.size());

		// delete border vertices of the adjacent zone if their neighbour zoneId
		// is equal to current zone zoneId
		std::vector<CBorderVertex>::iterator itborder = adjZoneBorderVertices.begin();
		while(itborder != adjZoneBorderVertices.end())
		{
			if((*itborder).NeighborZoneId == centerZoneId)
			{
				itborder = adjZoneBorderVertices.erase(itborder);
			}
			else
				itborder++;
		}
		fprintf(fdbg,"(after) zone %u bordervertices size : %u\n",i,(uint)adjZoneBorderVertices.size());

		// A set for storing base vertex index already added in the quad tree
		std::set<uint16> adjBaseVertexIndexSet;

		// if point in adjacent zone is not in the set :
		// -> add it in the set
		// -> add it in the quad
		for(ptch = 0; ptch<adjZonePatchs.size(); ptch++)
		{
			for(j=0; j<4; j++)
			{
				CWeldableVertexInfos wvinf;
				wvinf.IndexInZone = adjZonePatchs[ptch].BaseVertices[j]; // useful ????
				wvinf.PatchIndex = ptch;
				wvinf.PatchVertex = j;
				if(adjBaseVertexIndexSet.find(wvinf.IndexInZone) == adjBaseVertexIndexSet.end())
				{
					adjBaseVertexIndexSet.insert(wvinf.IndexInZone);
					CVector bboxmin;
					CVector bboxmax;
					bboxmin.x = adjZonePatchs[ptch].Patch.Vertices[j].x;
					bboxmin.y = adjZonePatchs[ptch].Patch.Vertices[j].y;
					bboxmin.z = adjZonePatchs[ptch].Patch.Vertices[j].z;
					bboxmax = bboxmin;
					quadTrees[i].insert(bboxmin,bboxmax,wvinf);
				}
			}
		}

		quadTrees[i].clearSelection();


		float bboxRadius = 10; //TEMP !!
		
		std::set<uint16> centerBaseVertexIndexSet;
		std::set<uint16> currentAdjBaseVertexIndexSet;

		for(ptch=0; ptch<centerZonePatchs.size(); ptch++) // for all patchs in center zone
		{
			// stores infos for edge part
			CWeldableVertexInfos nearVertexInfos[4];
			
			bool toWeld[4];
			
			CVector bboxmin;
			CVector bboxmax;
			
			
			// for every points in center patch we look for close points in adjacent patch
			for(j=0; j<4; j++) // 4 patch vertices (in center zone)
			{
				toWeld[j] = false;

				
				// already 'checked for welding' vertices are stored in a set
				centerBaseVertexIndexSet.insert(centerZonePatchs[ptch].BaseVertices[j]);

				//fprintf(fdbg,"%d - %d) CZBV(%d)\n",i,baseVertexIndexSet.size(),centerZonePatchs[ptch].BaseVertices[j]);

				bboxmin.x = centerZonePatchs[ptch].Patch.Vertices[j].x - bboxRadius;
				bboxmin.y = centerZonePatchs[ptch].Patch.Vertices[j].y - bboxRadius;
				bboxmin.z = centerZonePatchs[ptch].Patch.Vertices[j].z - bboxRadius;

				bboxmax.x = centerZonePatchs[ptch].Patch.Vertices[j].x + bboxRadius;
				bboxmax.y = centerZonePatchs[ptch].Patch.Vertices[j].y + bboxRadius;
				bboxmax.z = centerZonePatchs[ptch].Patch.Vertices[j].z + bboxRadius;
			
				//quadTrees[i].select(bboxmin,bboxmax);
				quadTrees[i].selectAll();	// TEMP !!!

				// current vertex coordinates in center zone
				CVector vctr;
				vctr.x = centerZonePatchs[ptch].Patch.Vertices[j].x;
				vctr.y = centerZonePatchs[ptch].Patch.Vertices[j].y;
				vctr.z = centerZonePatchs[ptch].Patch.Vertices[j].z;
				
				CWeldableVertexInfos wvinf;
				float minDistance = weldRadius + 1;  // rq: we weld only if we found a distance
													 //     inferior to weldRadius
				
				CQuadTree<CWeldableVertexInfos>::CIterator itqdt = quadTrees[i].begin();
				// for all points near of current vertex in adjacent zone..
				while (itqdt != quadTrees[i].end()) 
				{
					CVector vadj;
					vadj.x = adjZonePatchs[(*itqdt).PatchIndex].Patch.Vertices[(*itqdt).PatchVertex].x;
					vadj.y = adjZonePatchs[(*itqdt).PatchIndex].Patch.Vertices[(*itqdt).PatchVertex].y;
					vadj.z = adjZonePatchs[(*itqdt).PatchIndex].Patch.Vertices[(*itqdt).PatchVertex].z;
					
					CVector adjToCenter;
					adjToCenter.x = vctr.x - vadj.x;
					adjToCenter.y = vctr.y - vadj.y;
					adjToCenter.z = vctr.z - vadj.z;
					float dist = adjToCenter.norm();
					
					// if dist min we keep infos on this vertex(adj zone)
					// we keep the closest.
					if(dist<weldRadius && dist<minDistance) 
					{
#ifdef NL_DEBUG_WELD
						nlverify (!isTheSame (centerZonePatchs[ptch].Patch.Vertices[j], NL_DEBUG_WELD_V0));
						nlverify (!isTheSame (centerZonePatchs[ptch].Patch.Vertices[j], NL_DEBUG_WELD_V1));
#endif // NL_DEBUG_WELD
						minDistance = dist;
						wvinf = (*itqdt);
					}
					itqdt++;
				}

				quadTrees[i].clearSelection();

				if(minDistance<weldRadius) // i.e if we have found 2 vertices to weld
				{				
					// we save CBorderVertex info, and add it into the adjacent zone
					CBorderVertex adjBorderV;
					adjBorderV.CurrentVertex = wvinf.IndexInZone;
					adjBorderV.NeighborZoneId = centerZoneId;
					adjBorderV.NeighborVertex = centerZonePatchs[ptch].BaseVertices[j];
					nearVertexInfos[j] = wvinf;

					// we save CBorderVertex info, and add it into the center zone
					CBorderVertex centerBorderV;
					centerBorderV.CurrentVertex = centerZonePatchs[ptch].BaseVertices[j];
					centerBorderV.NeighborZoneId = adjZonesId[i];
					centerBorderV.NeighborVertex = wvinf.IndexInZone;

					toWeld[j] = true;

					if(centerBaseVertexIndexSet.find(centerZonePatchs[ptch].BaseVertices[j]) != centerBaseVertexIndexSet.end())
					{
						if(currentAdjBaseVertexIndexSet.find(wvinf.IndexInZone) == currentAdjBaseVertexIndexSet.end())
						{
							currentAdjBaseVertexIndexSet.insert(wvinf.IndexInZone);
							adjZoneBorderVertices.push_back(adjBorderV);
							centerZoneBorderVertices.push_back(centerBorderV);

							weldCount++;
#if WELD_LOG
							fprintf(fdbg,"%d) weld vertices : zone%d.(patch%d.vertex%d).baseVertex%d to centerZone.(patch%d.vertex%d).baseVertex%d\n",
								weldCount,i,wvinf.PatchIndex,wvinf.PatchVertex,wvinf.IndexInZone,ptch,j,centerZonePatchs[ptch].BaseVertices[j]);
#endif
						}
					}
				}
			}
			

			
			// then we bind edges (made of weldable vertices) and modify tangents
			
			for(j=0; j<4; j++) 
			{
#ifdef NL_DEBUG_WELD
					if (
						(isTheSame (centerZonePatchs[ptch].Patch.Vertices[j], NL_DEBUG_WELD_V0) || 
						 isTheSame (centerZonePatchs[ptch].Patch.Vertices[(j+1)%4], NL_DEBUG_WELD_V0) ) &&
						(isTheSame (centerZonePatchs[ptch].Patch.Vertices[j], NL_DEBUG_WELD_V1) || 
						 isTheSame (centerZonePatchs[ptch].Patch.Vertices[(j+1)%4], NL_DEBUG_WELD_V1) )
						 )
						 nlstop;
#endif // NL_DEBUG_WELD
				// if vertex has been welded...
				if(toWeld[j] == false) continue;
				// ...we look if next vertex(i.e if the edge) in center zone has to be welded
				if(toWeld[(j+1)%4] == false) continue;
								
				// We know the two adjacent base vertices
				// we look for the adjacent patch and the edge containing these vertices
				uint8 edgeIndex;
				uint16 patchIndex;
				if(! getPatchAndEdge(adjZonePatchs,
									 nearVertexInfos[j].IndexInZone,
									 nearVertexInfos[(j+1)%4].IndexInZone,
									 patchIndex,
									 edgeIndex))
				{
#if WELD_LOG
					fprintf(fdbg,"* Error * : Can't find patch containing the following edge : %d - %d\n",
						nearVertexInfos[j].IndexInZone,
						nearVertexInfos[(j+1)%4].IndexInZone);
#endif
					nlwarning ("ERROR : zone_welder : Can't find patch containing the following edge : %d - %d\n",
						nearVertexInfos[j].IndexInZone,
						nearVertexInfos[(j+1)%4].IndexInZone);
					continue;
				}

#if WELD_LOG
				fprintf(fdbg,"weld edges : zone%d.patch%d.edge%d(%d-%d) to centerZone.patch%d.edge%d(%d-%d)\n",
					i,
					patchIndex,
					edgeIndex,
					nearVertexInfos[j].IndexInZone,
					nearVertexInfos[(j+1)%4].IndexInZone,
					ptch,
					j,
					centerZonePatchs[ptch].BaseVertices[j],
					centerZonePatchs[ptch].BaseVertices[(j+1)%4] );
				fprintf(fdbg,"center patch %d : %d %d %d %d\n\n",
					ptch,
					centerZonePatchs[ptch].BaseVertices[0],
					centerZonePatchs[ptch].BaseVertices[1],
					centerZonePatchs[ptch].BaseVertices[2],
					centerZonePatchs[ptch].BaseVertices[3]);
#endif

				// Check the edge find is not binded
				if (adjZonePatchs[patchIndex].BindEdges[edgeIndex].NPatchs!=0)
				{
					// Build an error message
					char buf[2048];
					stringstream sserror;

					// Zone name
					string nameCenter, nameAdj;
					getZoneNameByCoord (centerZoneId&0xff, (centerZoneId>>8)+1, nameCenter);
					getZoneNameByCoord (adjZonesId[i]&0xff, (adjZonesId[i]>>8)+1, nameAdj);

					// Main message
					smprintf(buf, 2048,
						"Bind Error: try to bind the patch n %d in zone n %s with patch n %d in zone %s\n"
						"This patch is already binded with the following patches : ", ptch+1, nameAdj.c_str(), 
						patchIndex+1, nameCenter.c_str() );
					sserror << buf;

					// Sub message
					for (uint i=0; i<adjZonePatchs[patchIndex].BindEdges[edgeIndex].NPatchs; i++)
					{
						// Last patch ?
						bool last=(i==(uint)(adjZonePatchs[patchIndex].BindEdges[edgeIndex].NPatchs-1));

						// Sub message
						smprintf(buf, 2048,
							"patch n %d%s", adjZonePatchs[patchIndex].BindEdges[edgeIndex].Next[i]+1, last?"\n":",");

						// Concat the message
						sserror << buf;
					}

					// Add an error message
					errorMessage.push_back(sserror.str());
				}
				else
				{
#ifdef NL_DEBUG_WELD
					if (
						(isTheSame (centerZonePatchs[ptch].Patch.Vertices[j], NL_DEBUG_WELD_V0) || 
						 isTheSame (centerZonePatchs[ptch].Patch.Vertices[(j+1)%4], NL_DEBUG_WELD_V0) ) &&
						(isTheSame (centerZonePatchs[ptch].Patch.Vertices[j], NL_DEBUG_WELD_V1) || 
						 isTheSame (centerZonePatchs[ptch].Patch.Vertices[(j+1)%4], NL_DEBUG_WELD_V1) )
						 )
						 nlstop;
#endif // NL_DEBUG_WELD
					centerZonePatchs[ptch].BindEdges[j].NPatchs = 1;
					centerZonePatchs[ptch].BindEdges[j].ZoneId = adjZonesId[i];
					centerZonePatchs[ptch].BindEdges[j].Next[0] = patchIndex;   
					centerZonePatchs[ptch].BindEdges[j].Edge[0] = edgeIndex;  

					// adjacent zone edge
					adjZonePatchs[patchIndex].BindEdges[edgeIndex].NPatchs = 1;
					adjZonePatchs[patchIndex].BindEdges[edgeIndex].ZoneId = centerZoneId;
					adjZonePatchs[patchIndex].BindEdges[edgeIndex].Next[0] = ptch;
					adjZonePatchs[patchIndex].BindEdges[edgeIndex].Edge[0] = j;

					// force the same smooth flag
					bool smoothFlag = centerZonePatchs[ptch].getSmoothFlag (j);
					smoothFlag &= adjZonePatchs[patchIndex].getSmoothFlag (edgeIndex);
					centerZonePatchs[ptch].setSmoothFlag (j, smoothFlag);
					adjZonePatchs[patchIndex].setSmoothFlag (edgeIndex, smoothFlag);

					// tangent become the mean or both tangents (adj and center)
					// Here we cross the mean because adjacent edges are counter-oriented
					// due to the patchs constant orientation.
					CVector		middle0= (centerZonePatchs[ptch].Patch.Tangents[2*j]+
						adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex+1])/2;
					CVector		middle1= (centerZonePatchs[ptch].Patch.Tangents[2*j+1]+
						adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex])/2;

					centerZonePatchs[ptch].Patch.Tangents[2*j] = 
					adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex+1] = middle0;
						
					centerZonePatchs[ptch].Patch.Tangents[2*j+1] = 
					adjZonePatchs[patchIndex].Patch.Tangents[2*edgeIndex] = middle1;
				}
			}
			
		}

	}


	// Yoyo: compute the mean on vertices beetween zones.
	//====================================
	// do it before "make coplanar beetween zones", because CZoneTgtSmoother use tangents and vertices to smooth.
	{
		// build all input vertices for center and adjacents zones 
		//------------------

		// For center zone rebuild vertices.
		vector<CVector>		centerVertices;
		// for all patch, fill the array of vertices.
		for(ptch=0; ptch<centerZonePatchs.size(); ptch++)
		{
			CPatchInfo	&pa= centerZonePatchs[ptch];
			for(uint corner= 0; corner<4; corner++)
			{
				uint	idVert= pa.BaseVertices[corner];

				// write this vertex in array.
				centerVertices.resize( max((uint)centerVertices.size(), idVert+1) );
				centerVertices[idVert]= pa.Patch.Vertices[corner];
			}
		}

		// For all adjacent zone rebuild vertices.
		map<uint16, vector<CAdjacentVertex> >		adjVertices;
		for(i=0;i<8;i++)
		{
			if(adjZonesName[i]=="empty") continue;
			if(!adjZoneFileFound[i]) continue;

			// create the entry in the map.
			vector<CAdjacentVertex> &verts= adjVertices[adjZonesId[i]];

			// for all patch, fill the array of vertices.
			std::vector<CPatchInfo>		&adjZonePatchs= adjZoneInfos[i].Patchs;
			for(ptch=0; ptch<adjZonePatchs.size(); ptch++)
			{
				CPatchInfo	&pa= adjZonePatchs[ptch];
				for(uint corner= 0; corner<4; corner++)
				{
					uint	idVert= pa.BaseVertices[corner];

					// write this vertex in array.
					verts.resize( max((uint)verts.size(), idVert+1) );
					verts[idVert].Vertex= pa.Patch.Vertices[corner];
					verts[idVert].OnBorder= false;
				}
			}

			// for all borderVertices with centerZoneId, fill verts neighbor info.
			std::vector<CBorderVertex>	&adjZoneBorderVertices= adjZoneInfos[i].BorderVertices;
			uint	bv;
			for(bv=0; bv<adjZoneBorderVertices.size(); bv++)
			{
				CBorderVertex	&adjBV= adjZoneBorderVertices[bv];
				if(adjBV.NeighborZoneId == centerZoneId)
				{
					verts[adjBV.CurrentVertex].OnBorder= true;
					verts[adjBV.CurrentVertex].IdOnCenterZone= adjBV.NeighborVertex;
				}
			}

		}

		// compute the mean on border vertices
		//------------------


		// create / reset the result vertices.
		vector<CVectorInfluence>	outVertices;
		outVertices.resize(centerVertices.size());
		for(i=0; i<outVertices.size(); i++)
		{
			outVertices[i].Vertex= centerVertices[i];
			outVertices[i].Inf= 1;
			outVertices[i].OnBorder= false;
		}


		// For all borderVertices of centerZone, choose the good vertex, add neighbor influence
		uint	bv;
		for(bv=0; bv<centerZoneBorderVertices.size(); bv++)
		{
			CBorderVertex	&centerBV= centerZoneBorderVertices[bv];
			uint	centerVert= centerBV.CurrentVertex;
			if( adjVertices.find(centerBV.NeighborZoneId) != adjVertices.end() )
			{
				outVertices[centerVert].Vertex+= adjVertices[centerBV.NeighborZoneId][centerBV.NeighborVertex].Vertex;
				outVertices[centerVert].Inf++;
				outVertices[centerVert].OnBorder= true;
			}
		}
		// normalize influence.
		for(i=0; i<outVertices.size(); i++)
		{
			if(outVertices[i].Inf!=1)
			{
				outVertices[i].Vertex/= outVertices[i].Inf;
				outVertices[i].Inf= 1;
			}
		}


		// for all zones, get the new vertices.
		//------------------

		// For center zone, for all patchs, copy from outVertices.
		for(ptch=0; ptch<centerZonePatchs.size(); ptch++)
		{
			CPatchInfo	&pa= centerZonePatchs[ptch];
			for(uint corner= 0; corner<4; corner++)
			{
				uint	idVert= pa.BaseVertices[corner];

				if(outVertices[idVert].OnBorder)
				{
					// copy the vertex.
					pa.Patch.Vertices[corner]= outVertices[idVert].Vertex;
				}
			}
		}


		// For all borderVertices of adjacentZone, copy from outVertices.
		for(i=0;i<8;i++)
		{
			if(adjZonesName[i]=="empty") continue;
			if(!adjZoneFileFound[i]) continue;

			// get the entry in the map.
			vector<CAdjacentVertex> &verts= adjVertices[adjZonesId[i]];

			// for all patch, get vertices which are n Border of the cetnerZone.
			std::vector<CPatchInfo>		&adjZonePatchs= adjZoneInfos[i].Patchs;
			for(ptch=0; ptch<adjZonePatchs.size(); ptch++)
			{
				CPatchInfo	&pa= adjZonePatchs[ptch];
				for(uint corner= 0; corner<4; corner++)
				{
					uint	idVert= pa.BaseVertices[corner];

					if(verts[idVert].OnBorder)
					{
						pa.Patch.Vertices[corner]= outVertices[verts[idVert].IdOnCenterZone].Vertex;
					}
				}
			}

		}

	}



	// Yoyo: make coplanar beetween zones.
	//====================================
	{
		std::vector<CZoneInfo>	zones;
		CZoneInfo	zinf;

		// center.
		zinf.ZoneId= centerZoneId;
		zinf.Patchs= centerZonePatchs;
		zinf.BorderVertices= centerZoneBorderVertices;
		zones.push_back(zinf);

		// adjs.
		for(i=0;i<8;i++)
		{
			if(adjZonesName[i]=="empty") continue;
			if(!adjZoneFileFound[i]) continue;
			zones.push_back(adjZoneInfos[i]);
		}

		CZoneTgtSmoother	tgtsmoother;
		tgtsmoother.makeVerticesCoplanar(zones);

		// retrieve center zone result.
		centerZonePatchs= zones[0].Patchs;
		centerZoneBorderVertices= zones[0].BorderVertices;

		// retrieve adj zone result.
		sint	numZone= 1;
		for(i=0;i<8;i++)
		{
			if(adjZonesName[i]=="empty") continue;
			if(!adjZoneFileFound[i]) continue;

			adjZoneInfos[i]= zones[numZone];
			numZone++;
		}
	}


	// Yoyo: compute corner smooth info.
	//====================================
	// CANNOT DO IT HERE, BECAUSE THE CURRENT ZONE MAY NOT BE CORRECLTY WELDED.
	// MUST DO IT IN ZONE_LIGHTER.
	/*{
		// build a landscape, because CZoneCornerSmooth use compiled zones.
		CLandscape	land;
		CZoneCornerSmoother		zcs;

		land.init();

		// add center zone.
		zone.build(centerZoneId, centerZonePatchs, centerZoneBorderVertices);
		land.addZone(zone);
		CZone	*centerZone= land.getZone(centerZoneId);

		// add adjacent zones.
		vector<CZone*>			nbZones;
		for(i=0;i<8;i++)
		{
			if(adjZonesName[i]=="empty") continue;
			if(!adjZoneFileFound[i]) continue;

			std::vector<CPatchInfo>		&adjZonePatchs= adjZoneInfos[i].Patchs;
			std::vector<CBorderVertex>	&adjZoneBorderVertices= adjZoneInfos[i].BorderVertices;

			adjZones[i].build(adjZonesId[i], adjZonePatchs, adjZoneBorderVertices);
			land.addZone(adjZones[i]);

			CZone	*nbZone= land.getZone(adjZonesId[i]);
			if(nbZone)
				nbZones.push_back(nbZone);
		}

		// now, do the zoneCornerSmoother.
		if(centerZone)
		{
			// go.
			zcs.computeAllCornerSmoothFlags(centerZone, nbZones);
			
			// get result from the compiled zone, and copy in the uncompiled one (ie in centerZonePatchs).
			for(i=0;i<centerZonePatchs.size();i++)
			{
				const CPatch	&paSrc= *((const CZone*)centerZone)->getPatch(i);
				CPatchInfo		&paDst= centerZonePatchs[i];
				for(uint corner=0; corner<4; corner++)
					paDst.setCornerSmoothFlag(corner, paSrc.getCornerSmoothFlag(corner));
			}
		}
	}*/

	// Some errors ?
	if (errorMessage.empty())
	{
		// Save adjacent zones.
		//=====================
		for(i=0;i<8;i++)
		{
			if(adjZonesName[i]=="empty") continue;
			if(!adjZoneFileFound[i]) continue;

			std::vector<CPatchInfo>		&adjZonePatchs= adjZoneInfos[i].Patchs;
			std::vector<CBorderVertex>	&adjZoneBorderVertices= adjZoneInfos[i].BorderVertices;

			adjZones[i].build(adjZonesId[i], adjZonePatchs, adjZoneBorderVertices);
	#if WELD_LOG
			fprintf(fdbg,"[%d] binds :\n", i);
			adjZones[i].debugBinds(fdbg);
	#endif
			std::string strtmp;

			//strtmp = outputPath;
			strtmp = outputDir+adjZonesName[i]+outputExt;
			COFile adjSave(strtmp);
			printf("writing file %s\n",strtmp.c_str());
			adjZones[i].serial(adjSave);

		}

		// Save center zone.
		//==================
		zone.build(centerZoneId, centerZonePatchs, centerZoneBorderVertices);
		std::string strtmp;
		strtmp = outputDir+center+outputExt;

		COFile centerSave(strtmp);
		printf("writing file %s\n",strtmp.c_str());
		zone.serial(centerSave);
	}
	else
	{
		// Main message
		nlwarning ("ERROR weld failed. Correct errors below: (indices are MAX indices (+1))\n");

		// For each message
		for (uint i=0; i<errorMessage.size(); i++)
		{
			// Message
			nlwarning ("%s", errorMessage[i].c_str());
		}
	}

}



/*******************************************************************\
							main()
\*******************************************************************/
int main(sint argc, char **argv)
{
	// no zone file in argument
	if(argc<3)
	{
		writeInstructions();
		return 0;
	}
	
	// help
	if(strcmp(argv[1],"/?")==0)
	{
		writeInstructions();
		return 0;
	}

#if WELD_LOG
	fdbg = nlfopen("log.txt","wt");
	fprintf(fdbg,"Center zone : %s\n",argv[1]);
#endif

	printf("Center zone : %s\n",argv[1]);

	inputDir = getDir (argv[1]);
	inputExt = getExt (argv[1]);
	outputDir = getDir (argv[2]);
	outputExt = getExt (argv[2]);

	if(argc == 4)
	{
		NLMISC::fromString(argv[3], weldRadius);
	}

	std::string center=getName(argv[1]).c_str();
	weldZones(center.c_str());
	
#if WELD_LOG
	fclose(fdbg);
#endif

	return 0;
}



