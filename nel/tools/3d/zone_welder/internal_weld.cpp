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

#include "../zone_lib/zone_utility.h"
//   
#include "nel/misc/types_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/aabbox.h"
//
#include "nel/3d/quad_grid.h"
#include "nel/3d/bezier_patch.h"
#include "nel/3d/zone.h"

//
#include <vector>
#include <algorithm>
#include <memory>
#include <set>




using namespace NLMISC;
using namespace NL3D;

// a patch vertex info, for quadtree insertion
struct CPatchVertexInfo
{
	uint	ZoneIndex;	  // this zone index, 0 for the midlle zone, and from 1 to 8 for the zones around
	uint	PatchIndex;   // the Patch of this zone
	uint	PatchVertex;  // the vertex of thi patch 0..3

	CVector Pos;
	CPatchVertexInfo() {}
	CPatchVertexInfo(uint zoneIndex,
					 uint patchIndex,
					 uint patchVertex,
					 const CVector &pos					 
					) 
					 : ZoneIndex(zoneIndex),
					   PatchIndex(patchIndex),
					   PatchVertex(patchVertex),
					   Pos(pos)
	{
	}
};

typedef std::vector<CPatchVertexInfo *> TPVVect;
typedef CQuadGrid<CPatchVertexInfo>   TPVQuadGrid;

// ***************************************************************************

void bind_1_1 (std::vector<CPatchInfo> &zoneInfos, uint patch0, uint edge0, uint patch1, uint edge1, uint zoneId)
{
	// Bind type
	zoneInfos[patch0].BindEdges[edge0].NPatchs = 1;
	zoneInfos[patch1].BindEdges[edge1].NPatchs = 1;

	// Zone ID
	zoneInfos[patch0].BindEdges[edge0].ZoneId = zoneId;
	zoneInfos[patch1].BindEdges[edge1].ZoneId = zoneId;

	// Next
	zoneInfos[patch0].BindEdges[edge0].Next[0] = patch1;
	zoneInfos[patch1].BindEdges[edge1].Next[0] = patch0;

	// Edge
	zoneInfos[patch0].BindEdges[edge0].Edge[0] = edge1;
	zoneInfos[patch1].BindEdges[edge1].Edge[0] = edge0;
}

// ***************************************************************************

void bind_1_2 (std::vector<CPatchInfo> &zoneInfos, uint patch, uint edge, uint patch0, uint edge0, uint patch1, uint edge1, uint zoneId)
{
	// Bind type
	zoneInfos[patch].BindEdges[edge].NPatchs = 2;
	zoneInfos[patch0].BindEdges[edge0].NPatchs = 5;
	zoneInfos[patch1].BindEdges[edge1].NPatchs = 5;

	// Zone ID
	zoneInfos[patch].BindEdges[edge].ZoneId = zoneId;
	zoneInfos[patch0].BindEdges[edge0].ZoneId = zoneId;
	zoneInfos[patch1].BindEdges[edge1].ZoneId = zoneId;

	// Next
	zoneInfos[patch].BindEdges[edge].Next[0] = patch0;
	zoneInfos[patch].BindEdges[edge].Next[1] = patch1;
	zoneInfos[patch0].BindEdges[edge0].Next[0] = patch;
	zoneInfos[patch1].BindEdges[edge1].Next[0] = patch;

	// Edge
	zoneInfos[patch].BindEdges[edge].Edge[0] = edge0;
	zoneInfos[patch].BindEdges[edge].Edge[1] = edge1;
	zoneInfos[patch0].BindEdges[edge0].Edge[0] = edge;
	zoneInfos[patch1].BindEdges[edge1].Edge[0] = edge;
}

// ***************************************************************************

void bind_1_4 (std::vector<CPatchInfo> &zoneInfos, uint patch, uint edge, uint patch0, uint edge0, uint patch1, uint edge1, uint patch2, uint edge2, uint patch3, uint edge3, uint zoneId)
{
	// Bind type
	zoneInfos[patch].BindEdges[edge].NPatchs = 4;
	zoneInfos[patch0].BindEdges[edge0].NPatchs = 5;
	zoneInfos[patch1].BindEdges[edge1].NPatchs = 5;
	zoneInfos[patch2].BindEdges[edge2].NPatchs = 5;
	zoneInfos[patch3].BindEdges[edge3].NPatchs = 5;

	// Zone ID
	zoneInfos[patch].BindEdges[edge].ZoneId = zoneId;
	zoneInfos[patch0].BindEdges[edge0].ZoneId = zoneId;
	zoneInfos[patch1].BindEdges[edge1].ZoneId = zoneId;
	zoneInfos[patch2].BindEdges[edge2].ZoneId = zoneId;
	zoneInfos[patch3].BindEdges[edge3].ZoneId = zoneId;

	// Next
	zoneInfos[patch].BindEdges[edge].Next[0] = patch0;
	zoneInfos[patch].BindEdges[edge].Next[1] = patch1;
	zoneInfos[patch].BindEdges[edge].Next[2] = patch2;
	zoneInfos[patch].BindEdges[edge].Next[3] = patch3;
	zoneInfos[patch0].BindEdges[edge0].Next[0] = patch;
	zoneInfos[patch1].BindEdges[edge1].Next[0] = patch;
	zoneInfos[patch2].BindEdges[edge2].Next[0] = patch;
	zoneInfos[patch3].BindEdges[edge3].Next[0] = patch;

	// Edge
	zoneInfos[patch].BindEdges[edge].Edge[0] = edge0;
	zoneInfos[patch].BindEdges[edge].Edge[1] = edge1;
	zoneInfos[patch].BindEdges[edge].Edge[2] = edge2;
	zoneInfos[patch].BindEdges[edge].Edge[3] = edge3;
	zoneInfos[patch0].BindEdges[edge0].Edge[0] = edge;
	zoneInfos[patch1].BindEdges[edge1].Edge[0] = edge;
	zoneInfos[patch2].BindEdges[edge2].Edge[0] = edge;
	zoneInfos[patch3].BindEdges[edge3].Edge[0] = edge;
}

// ***************************************************************************

/**  Test whether 2 vertices could be welded */

static inline bool CanWeld(const CVector &v1, const CVector &v2, float weldThreshold)
{
	return (v1 - v2).norm() < weldThreshold;
}

// ***************************************************************************

uint getOtherCountAndPos (const std::vector<CPatchInfo> &zoneInfo, uint patch, uint edge, uint &otherPos)
{
	// Must be a multiple patch bind
	if (zoneInfo[patch].BindEdges[edge].NPatchs == 5)
	{
		uint i;
		const CPatchInfo &otherPatchRef = zoneInfo[zoneInfo[patch].BindEdges[edge].Next[0]];
		uint otherEdge = zoneInfo[patch].BindEdges[edge].Edge[0];
		for (i=0; i<otherPatchRef.BindEdges[otherEdge].NPatchs; i++)
		{
			if ( (otherPatchRef.BindEdges[otherEdge].Next[i] == patch) && (otherPatchRef.BindEdges[otherEdge].Edge[i] == edge) )
			{
				otherPos = i;
				return otherPatchRef.BindEdges[otherEdge].NPatchs;
			}
		}
	}
	return 1;
}

// ***************************************************************************

/**	Get all vertices that are near the given one */

static void GetCandidateVertices(const CVector &pos,
								 TPVQuadGrid &qg,
								 TPVVect &dest,
								 uint patchToExclude,
								 uint patchToExcludeZone,
								 float weldThreshold,
								 bool exclude
								)
{
	dest.clear();	
	const CVector half(weldThreshold, weldThreshold, weldThreshold);
	float weldThresholdSrt = weldThreshold * weldThreshold;
	qg.select(pos - half, pos + half);
	for (TPVQuadGrid::CIterator it = qg.begin(); it != qg.end(); ++it)
	{
		if ( ::CanWeld((*it).Pos, pos, weldThreshold) )
		{
			if ( (!exclude) || (! ((*it).ZoneIndex == patchToExcludeZone && (*it).PatchIndex == patchToExclude) ) )
			{
				// Final distance test
				if ( (pos - (*it).Pos).sqrnorm () <= weldThresholdSrt )
					dest.push_back(&(*it));
			}
		}
	}
}

// ***************************************************************************

bool isBinded (std::vector<CPatchInfo> &zoneInfos, uint patch0, uint edge0, uint patch1, uint edge1)
{
	// Binded ?
	if ( (zoneInfos[patch0].BindEdges[edge0].NPatchs != 0) && (zoneInfos[patch1].BindEdges[edge1].NPatchs != 0) )
	{
		// Binded ?
		return (zoneInfos[patch0].BindEdges[edge0].Next[0] == patch1) ||
			(zoneInfos[patch1].BindEdges[edge1].Next[0] == patch0);
	}
	return false;
}

// ***************************************************************************

CVector evalPatchEdge (CPatchInfo &patch, uint edge, float lambda)
{
	// index of this border vertices
	static const float indexToST[][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
	const uint vIndex[]  = { edge, (edge + 1) & 0x03 };

	return patch.Patch.eval((1.f - lambda) * indexToST[vIndex[0]][0] + lambda * indexToST[vIndex[1]][0],
													(1.f - lambda) * indexToST[vIndex[0]][1] + lambda * indexToST[vIndex[1]][1]);
}

// ***************************************************************************

void getFirst (CVector &a, CVector &b, CVector &c, CVector &d)
{
	// ab
	CVector ab = (a+b)/2.f;

	// bc
	CVector bc = (b+c)/2.f;

	// cd
	CVector cd = (c+d)/2.f;

	b = ab;
	c = (ab + bc) / 2.f;
	d = ( (bc + cd) / 2.f + c ) / 2.f;
}

// ***************************************************************************

void getSecond (CVector &a, CVector &b, CVector &c, CVector &d)
{
	// ab
	CVector ab = (a+b)/2.f;

	// bc
	CVector bc = (b+c)/2.f;

	// cd
	CVector cd = (c+d)/2.f;

	c = cd;
	b = (bc + cd) / 2.f;
	a = ( (ab + bc) / 2.f + b ) / 2.f;
}

// ***************************************************************************

void CleanZone ( std::vector<CPatchInfo> &zoneInfos, uint zoneId, const CAABBoxExt &zoneBBox, float weldThreshold)
{
	uint l, m, n, p, q;	// some loop counters

	///////////////////////////////
	// retrieve datas from zones //
	///////////////////////////////

	// fill the quad grid
	float zoneSize = 2.f * weldThreshold + std::max(zoneBBox.getMax().x - zoneBBox.getMin().x,
													 zoneBBox.getMax().y - zoneBBox.getMin().y);

	TPVQuadGrid qg;
	const uint numQGElt = 128;
	qg.create (numQGElt, zoneSize / numQGElt);

	for (l = 0; l < zoneInfos.size(); ++l)
	{
		CPatchInfo &patch = zoneInfos[l];
		// for each base vertex of the patch
		for (m = 0; m < 4; ++m)
		{
			CVector &pos = patch.Patch.Vertices[m];

			// yes, insert it in the tree
			const CVector half(weldThreshold, weldThreshold, weldThreshold);
			qg.insert(pos - half, pos + half, CPatchVertexInfo(zoneId, l, m, pos));
		}
	}

	/////////////////////////////////////////////////
	// check whether each patch is correctly bound //
	/////////////////////////////////////////////////
	uint pass = 0;
	while (1)
	{
		uint bind1Count = 0;
		uint bind2Count = 0;
		uint bind4Count = 0;
		for (l = 0; l < zoneInfos.size(); ++l)
		{	
			// Ref on patch
			CPatchInfo &patch = zoneInfos[l];

			// deals with each border
			for (m = 0; m < 4; ++m)
			{			
				const uint vIndex[]  = { m, (m + 1) & 0x03 };

				// if this border is said to be bound, no need to test..
				if (patch.BindEdges[m].NPatchs == 0)
				{
					static TPVVect verts[2];

					// Get vertices from other patch that could be welded with this patch boder's vertices.
					for (q = 0; q < 2; ++q)
					{
						::GetCandidateVertices(patch.Patch.Vertices[vIndex[q]], qg, verts[q], l, zoneId, weldThreshold, true);
					}
								
					uint bindCount;
					for (bindCount = 1; bindCount<5; bindCount<<=1)
					{
						// Float middle
						float middle = 1.f / (float)bindCount;  // 0 = 0.5;  1 = 0.25
						
						// Try to find a patch that shares 2 consecutives vertices
						static TPVVect binded4[5];
						binded4[0] = verts[0];
						binded4[bindCount] = verts[1];

						// Compute points along the border and found list of vertex binded to it.
						float lambda = middle;
						for (n = 1; n <bindCount; ++n)
						{
							CVector borderPos = evalPatchEdge (patch, m, lambda);
							::GetCandidateVertices(borderPos, qg, binded4[n], l, zoneId, weldThreshold, true);
							lambda += middle;
						}

						// Binded patches and edges
						uint neighborPatches[4];
						uint neighborEdges[4];

						// Patch binded
						for (q = 0; q < bindCount; q++)
						{
							for (n = 0; n < binded4[q].size(); n++)
							{
								for (p = 0; p < binded4[q+1].size(); p++)
								{
									// Ref on the two patches
									const CPatchVertexInfo &pv0 = *(binded4[q][n]);
									const CPatchVertexInfo &pv1 = *(binded4[q+1][p]);

									// Direct or indirect ?
									// Vertex id
									uint v0 = pv0.PatchVertex;
									uint v1 = pv1.PatchVertex;
									if ( pv0.ZoneIndex == pv1.ZoneIndex && pv0.PatchIndex == pv1.PatchIndex )
									{
										// Direct edge ?
										if ( ( ( pv0.PatchVertex - pv1.PatchVertex) & 3 ) == 1 )
										{
											// Patch id
											uint patchId2 = pv0.PatchIndex;

											// Edge id
											uint edge = v1;

											// Edge not binded ?
											if (zoneInfos[patchId2].BindEdges[edge].NPatchs == 0)
											{
												// Save the patch
												neighborPatches[q] = patchId2;
												neighborEdges[q] = edge;
												goto exit24;
											}
										}
									}
								}
							}
							if (n == binded4[q].size())
								// No bind found, stop
								break;
exit24:;
						}

						// Find all binded patches ?
						if (q == bindCount)
						{
							// Check The patches are binded together
							for (q=0; q<bindCount-1; q++)
							{
								if (!isBinded (zoneInfos, neighborPatches[q], (neighborEdges[q]-1)&3, neighborPatches[q+1], (neighborEdges[q+1]+1)&3))
									break;
							}

							// Not breaked ?
							if (q == (bindCount-1) )
							{
								// Bind it
								if (bindCount == 1)
								{
									bind_1_1 (zoneInfos, l, m, neighborPatches[0], neighborEdges[0], zoneId);
									bind1Count++;
								}
								else if (bindCount == 2)
								{
									bind_1_2 (zoneInfos, l, m, neighborPatches[0], neighborEdges[0], neighborPatches[1], neighborEdges[1], zoneId);
									bind2Count++;
								}
								else
								{
									bind_1_4 (zoneInfos, l, m, neighborPatches[0], neighborEdges[0], neighborPatches[1], neighborEdges[1], 
										neighborPatches[2], neighborEdges[2], neighborPatches[3], neighborEdges[3], zoneId);
									bind4Count++;
								}
								// Exit connectivity loop
								break;
							}
						}
					}
				}
			}
		}

		// Print binds
		if (bind1Count || bind2Count || bind4Count)
		{
			printf ("Internal bind pass %d: ", pass);
			if (bind1Count)
				printf ("bind1-1 %d; \n", bind1Count);
			if (bind2Count)
				printf ("bind1-2 %d; \n", bind2Count);
			if (bind4Count)
				printf ("bind1-4 %d; \n", bind4Count);
		}
		else
			// No more bind, stop
			break;

		// Next pass
		pass++;
	}

	// Insert vertex binded in the map
	for (l = 0; l < zoneInfos.size(); ++l)
	{
		CPatchInfo &patch = zoneInfos[l];

		// for each edge
		int edge;
		for (edge = 0; edge < 4; ++edge)
		{
			// Binded ?
			uint bindCount = patch.BindEdges[edge].NPatchs;
			if ( (bindCount == 2) || (bindCount == 4) )
			{
				// Start
				float middle = 1.f / (float)bindCount;  // 0 = 0.5;  1 = 0.25
				float lambda = middle;

				// For all binded vertices
				uint vert;
				for (vert = 1; vert < bindCount; vert++)
				{
					// Eval the binded position
					CVector borderPos = evalPatchEdge (patch, edge, lambda);

					// yes, insert it in the tree
					const CVector half(weldThreshold, weldThreshold, weldThreshold);
					qg.insert (borderPos - half, borderPos + half, CPatchVertexInfo(zoneId, l, 5, borderPos));

					// New position
					lambda += middle;
				}
			}
		}
	}

	// Weld all the vertices !
	uint weldCount = 0;
	for (l = 0; l < zoneInfos.size(); ++l)
	{
		CPatchInfo &patch = zoneInfos[l];

		// for each edge
		int vert;
		for (vert = 0; vert < 4; ++vert)
		{
			// Not on an opened edge ?
			if ( (patch.BindEdges[vert].NPatchs != 0) && (patch.BindEdges[(vert-1)&3].NPatchs != 0) )
			{
				// Welded ?
				bool welded = false;

				// Get the vertex to weld
				static TPVVect toWeld;
				CVector pos = patch.Patch.Vertices[vert];
				::GetCandidateVertices (pos, qg, toWeld, l, zoneId, weldThreshold, false);

				// Weld it
				CVector average (0,0,0);
				uint w;
				bool absolutePosition = false;
				for (w = 0; w < toWeld.size (); w++)
				{
					// Welded vertex ?
					if (toWeld[w]->PatchVertex == 5)
					{
						absolutePosition = true;
						average = toWeld[w]->Pos;
					}

					// Add it;
					if (!absolutePosition)
						average += toWeld[w]->Pos;

					// Not the same ?
					float dist = (pos - toWeld[w]->Pos).norm();
					if ( (pos - toWeld[w]->Pos).sqrnorm() > 0.0001 )
						welded = true;
				}

				// Average
				if (!absolutePosition)
					average /= (float)toWeld.size ();

				// Weld ?
				if (welded)
				{
					// Welded
					weldCount++;

					// Set the pos
					for (w = 0; w < toWeld.size (); w++)
					{						
						if (toWeld[w]->PatchVertex != 5)
						{
							toWeld[w]->Pos = average;
							zoneInfos[toWeld[w]->PatchIndex].Patch.Vertices[toWeld[w]->PatchVertex] = average;
						}
					}
				}
			}
		}
	}

	if (weldCount)
		printf ("Internal vertices welded: %d\n", weldCount);

	// Weld all the Tangents !
	weldCount = 0;
	for (l = 0; l < zoneInfos.size(); ++l)
	{
		CPatchInfo &patch = zoneInfos[l];

		// for each edge
		int edge;
		for (edge = 0; edge < 4; ++edge)
		{
			// Binded ?
			uint bindCount = patch.BindEdges[edge].NPatchs;
			if ( /*(bindCount == 1) || */(bindCount == 5) )
			{
				// Neighbor patch
				uint otherPatch = patch.BindEdges[edge].Next[0];
				uint otherEdge = patch.BindEdges[edge].Edge[0];
				nlassert (otherPatch<zoneInfos.size ());
				nlassert (otherEdge<4);

				// Get the vertices
				CVector A, B, C, D;
				A = zoneInfos[otherPatch].Patch.Vertices[otherEdge];
				B = zoneInfos[otherPatch].Patch.Tangents[otherEdge*2];
				C = zoneInfos[otherPatch].Patch.Tangents[otherEdge*2+1];
				D = zoneInfos[otherPatch].Patch.Vertices[(otherEdge+1)&3];

				// Pos 
				uint otherPos;
				uint otherCount = getOtherCountAndPos (zoneInfos, l, edge, otherPos);
				nlassert ( ( (bindCount == 1) && (otherCount == 1) ) || ( (bindCount == 5) && ( (otherCount == 2) || (otherCount == 4) ) ) );

				// Calc tangents
				if (otherCount == 2)
				{
					if (otherPos == 0)
						getFirst (A, B, C, D);
					else
						getSecond (A, B, C, D);
				}
				else if (otherCount == 4)
				{
					if (otherPos == 0)
					{
						getFirst (A, B, C, D);
						getFirst (A, B, C, D);
					}
					else if (otherPos == 1)
					{
						getFirst (A, B, C, D);
						getSecond (A, B, C, D);
					}
					else if (otherPos == 2)
					{
						getSecond (A, B, C, D);
						getFirst (A, B, C, D);
					}
					else if (otherPos == 3)
					{
						getSecond (A, B, C, D);
						getSecond (A, B, C, D);
					}
				}

				// 2 tangents
				uint tang;
				for (tang=0; tang<2; tang++)
				{
					nlassert (2*edge+tang < 8);

					// Eval the binded position
					const CVector &tangVect = (tang==0) ? C : B;

					// Next offset
					float dist = (patch.Patch.Tangents[2*edge+tang] - tangVect).norm();
					if ( (patch.Patch.Tangents[2*edge+tang] - tangVect).sqrnorm() > 0.0001 )
						weldCount++;
					
					// Fix it!
					if (bindCount == 1)
					{
						patch.Patch.Tangents[2*edge+tang] += tangVect;
						patch.Patch.Tangents[2*edge+tang] /= 2;
					}
					else
						patch.Patch.Tangents[2*edge+tang] = tangVect;
				}
			}
		}
	}

	if (weldCount)
		printf ("Internal tangents welded: %d\n", weldCount);
}
