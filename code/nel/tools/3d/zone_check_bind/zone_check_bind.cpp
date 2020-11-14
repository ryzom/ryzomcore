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

#include "zone_utility.h"
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



//=========================================================================================================================


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


// A pair of patch ident. This is used to identify errors (involves a pair of patchs)
typedef std::pair<CPatchIdent, CPatchIdent> CPatchIdentPair;

// for set insertion
static inline bool operator < (const CPatchIdentPair &lhs, const CPatchIdentPair &rhs)
{
	return lhs.first != rhs.first ? lhs.first < rhs.first
								  : lhs.second < rhs.second;
}





typedef std::vector<CPatchVertexInfo *> TPVVect;
typedef CQuadGrid<CPatchVertexInfo>   TPVQuadGrid;


//=========================================================================================================================
//=========================================================================================================================
//=========================================================================================================================


/** Load the given zone (name without extension)
  * return a pointer to the zone, or NULL if not found
  * Throw an exception if a read error occurred
  */
static CZone *LoadZone(uint16 xPos, uint16 yPos, std::string zoneExt)
{
	std::string zoneName;
	::getZoneNameByCoord(xPos, yPos, zoneName);
	CUniquePtr<CZone> zone(new CZone);
	std::string lookedUpZoneName = CPath::lookup(zoneName + zoneExt, false, false, false);
	if (lookedUpZoneName.empty()) return NULL;
	CIFile iF;
	if (!iF.open(lookedUpZoneName))
	{
		throw EFileNotOpened(lookedUpZoneName);
	}
	zone->serial(iF);
	iF.close();
	return zone.release();
}

//===========================================================================================================================
/**  Test whether 2 vertices could be welded
  */
static inline bool CanWeld(const CVector &v1, const CVector &v2, float weldThreshold)
{
	return (v1 - v2).norm() < weldThreshold;
}


//===========================================================================================================================
/**	Get all vertices that are near the given one
  */
static void GetCandidateVertices(const CVector &pos,
								 TPVQuadGrid &qg,
								 TPVVect &dest,
								 uint patchToExclude,
								 uint patchToExcludeZone,
								 float weldThreshold
								)
{
	dest.clear();
	CVector half(weldThreshold, weldThreshold, weldThreshold);
	qg.select(pos - half, pos + half);
	for (TPVQuadGrid::CIterator it = qg.begin(); it != qg.end(); ++it)
	{
		if ( ::CanWeld((*it).Pos, pos, weldThreshold) )
		{
			if (! ((*it).ZoneIndex == patchToExcludeZone && (*it).PatchIndex == patchToExclude) )
			{
				dest.push_back(&(*it));
			}
		}
	}
}

//===========================================================================================================================
/**	Search a vertex of a patch that can be welded with the given vertex
  * return -1 if none
  */
static sint GetWeldableVertex(const CBezierPatch &bp, const CVector &pos, float weldThreshold)
{
	for (uint k = 0; k < 4; ++k)
	{
		if ( ::CanWeld(bp.Vertices[k], pos, weldThreshold) )
		{
			return k;
		}
	}
	return -1;
}


//=========================================================================================================================
/**  Check a zone and report the total number of errors
  */
static uint CheckZone(std::string middleZoneFile, float weldThreshold, float middleEdgeWeldThreshold)
{
	uint numErrors = 0;
	uint k, l, m, n, p, q;	// some loop counters
	// This avoid reporting errors twice (for readability)
	std::set<CPatchIdentPair> errorPairs;

	////////////////////////////
	// Load the zones around  //
	////////////////////////////

		CUniquePtr<CZone>		zones[9];
		std::string					zoneNames[9];
		CZoneInfo					zoneInfos[9];
		uint16  xPos, yPos;
		const sint16 posOffs[][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1} };

		std::string middleZoneName = CFile::getFilenameWithoutExtension(middleZoneFile);
		::getZoneCoordByName(middleZoneName.c_str(), xPos, yPos);
		try
		{
			std::string ext = CFile::getExtension(middleZoneFile);
			zones[0].reset(::LoadZone(xPos, yPos, ext.empty() ? "" : "." + ext));
			if (zones[0].get() == NULL)
			{
				nlwarning("Can't load zone  %s", middleZoneName.c_str());
				return 0;
			}
			for (uint k = 1; k < 9; ++k)
			{
				zones[k].reset(::LoadZone(xPos + posOffs[k][0], yPos + posOffs[k][1], ext.empty() ? "" : "." + ext));
			}
		}
		catch (const NLMISC::Exception &e)
		{
			nlinfo("Zones loading failed : %d", e.what());
			return 0;
		}

	///////////////////////////////
	// retrieve datas from zones //
	///////////////////////////////

		for (k = 0; k < 9; ++k)
		{
			::getZoneNameByCoord(xPos + posOffs[k][0], yPos + posOffs[k][1], zoneNames[k]);
			if (zones[k].get() != NULL) zones[k]->retrieve(zoneInfos[k]);
		}

		// fill the quad grid
		CAABBox zoneBBox = zones[0]->getZoneBB().getAABBox();
		float zoneSize = 2.f * weldThreshold + std::max(zoneBBox.getMax().x - zoneBBox.getMin().x,
														 zoneBBox.getMax().y - zoneBBox.getMin().y);
		TPVQuadGrid qg;
		const uint numQGElt = 128;
		qg.create(numQGElt, zoneSize / numQGElt);

		// insert vertices in quadgrid
		for (k = 0; k < 9; ++k)
		{
			for (l = 0; l < zoneInfos[k].Patchs.size(); ++l)
			{
				CPatchInfo &patch = zoneInfos[k].Patchs[l];
				// for each base vertex of the patch
				for (m = 0; m < 4; ++m)
				{
					CVector &pos = patch.Patch.Vertices[m];
					CBSphere s(pos, weldThreshold);
					if (zoneBBox.intersect(s)) // does this vertex and its zone of influence intersect the bbox ?
					{
						CVector half(weldThreshold, weldThreshold, weldThreshold);
						// yes, insert it in the tree
						qg.insert(pos - half, pos + half, CPatchVertexInfo(k, l, m, pos));
					}
				}
			}
		}

	/////////////////////////////////////////////////
	// check whether each patch is correctly bound //
	/////////////////////////////////////////////////

	for (l = 0; l < zoneInfos[0].Patchs.size(); ++l)
	{
		CPatchInfo &patch = zoneInfos[0].Patchs[l];
		// deals with each border
		for (m = 0; m < 4; ++m)
		{
			// if this border is said to be bound, no need to test..
			if (patch.BindEdges[m].NPatchs == 0)
			{
				// maps from an index to  a (s, t) couple
				static const float indexToST[][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};

				// index of this border vertices
				const uint vIndex[]  = { m, (m + 1) & 0x03 };

				bool errorFound = false;

				static TPVVect verts[2];

				// Get vertices from other patch that could be welded with this patch boder's vertices.
				for (q = 0; q < 2; ++q)
				{
					//nlinfo("pos = %f, %f, %f", patch.Patch.Vertices[vIndex[q]].x, patch.Patch.Vertices[vIndex[q]].y, patch.Patch.Vertices[vIndex[q]].z);
					::GetCandidateVertices(patch.Patch.Vertices[vIndex[q]], qg, verts[q], l, 0, weldThreshold);
				}


				///////////////////////////
				// 1 - 1 connectivity ?  //
				///////////////////////////
				// If there is a patch that is present in the 2 lists, then this is a 1-1 error
				for (n = 0; n < verts[0].size() && !errorFound; ++n)
				{
					for (p = 0; p < verts[1].size() && !errorFound; ++p)
					{
						if (verts[0][n]->ZoneIndex == verts[1][p]->ZoneIndex
							&& verts[0][n]->PatchIndex == verts[1][p]->PatchIndex)
						{
							CPatchIdent pi1(0, l);
							CPatchIdent pi2(verts[0][n]->ZoneIndex, verts[0][n]->PatchIndex);
							CPatchIdentPair errPair = std::make_pair(pi1, pi2);
							//
							if (std::find(errorPairs.begin(), errorPairs.end(), errPair) == errorPairs.end()) // error already displayed ?
							{
								nlinfo("**** Patch %d of zone %s has 1 - 1 connectivity error, try binding it with patch %d of zone %s",
										l + 1, middleZoneName.c_str(), verts[0][n]->PatchIndex + 1, zoneNames[verts[0][n]->ZoneIndex].c_str());
								errorPairs.insert(std::make_pair(pi2, pi1));
								++numErrors;
							}
							errorFound = true;
						}
					}
				}
				if (errorFound) continue;

				//////////////////////////
				// 1 - 2 connectivity ? //
				//////////////////////////

				// get the position at the middle of that border
				CVector middlePos = patch.Patch.eval( 0.5f * (indexToST[vIndex[0]][0] + indexToST[vIndex[1]][0]),
													  0.5f * (indexToST[vIndex[0]][1] + indexToST[vIndex[1]][1]) );

				// for each vertex of this border
				for (q = 0; q < 2 && !errorFound; ++q)
				{
					for (n = 0; n < verts[q].size() && !errorFound; ++n)
					{
						const CPatchVertexInfo &pv = *(verts[q][n]);
						// ref to the patch that share a vertex with this one
						const CBezierPatch &bPatch = zoneInfos[pv.ZoneIndex].Patchs[pv.PatchIndex].Patch;
						sint vertIndex = ::GetWeldableVertex(bPatch, pv.Pos, weldThreshold);
						nlassert(vertIndex != -1); // should found one..
						// Follow this patch edge and see if the next / previous vertex could be welded with the middle
						const CVector &nextVertPos = bPatch.Vertices[(vertIndex +  1) & 0x03];
						const CVector &prevVertPos = bPatch.Vertices[(vertIndex -  1) & 0x03];
						if (::CanWeld(nextVertPos, middlePos, middleEdgeWeldThreshold)
							|| ::CanWeld(prevVertPos, middlePos, middleEdgeWeldThreshold)
						   )
						{
							CPatchIdent pi1(0, l);
							CPatchIdent pi2(pv.ZoneIndex, pv.PatchIndex);
							CPatchIdentPair errPair = std::make_pair(pi1, pi2);
							//
							if (std::find(errorPairs.begin(), errorPairs.end(), errPair) == errorPairs.end()) // error already displayed ?
							{
								nlinfo("**** Patch %d of zone %s has 1 - 2 connectivity error, try binding it with patch %d of zone %s",
										l + 1, middleZoneName.c_str(), pv.PatchIndex + 1, zoneNames[pv.ZoneIndex].c_str());
								errorPairs.insert(std::make_pair(pi2, pi1));
								++numErrors;
							}

							errorFound = true;
							break;
						}
					}
				}
				if (errorFound) continue;

				//////////////////////////
				// 1 - 4 connectivity ? //
				//////////////////////////

				// compute points along the border.
				CVector borderPos[5];
				float lambda = 0.f;
				for (n = 0; n < 5; ++n)
				{
					borderPos[n] = patch.Patch.eval((1.f - lambda) * indexToST[vIndex[0]][0] + lambda * indexToST[vIndex[1]][0],
													(1.f - lambda) * indexToST[vIndex[0]][1] + lambda * indexToST[vIndex[1]][1]);
					lambda += 0.25f;
				}

				// Try to find a patch that shares 2 consecutives vertices
				for (k = 0; k < 4 && !errorFound; ++k)
				{
					::GetCandidateVertices(borderPos[k], qg, verts[0], l, 0, middleEdgeWeldThreshold);
					for (p = 0; p < verts[0].size() && !errorFound; ++p)
					{
						const CPatchVertexInfo &pv = *(verts[0][p]);
						// ref to the patch that share a vertex with this one
						const CBezierPatch &bPatch = zoneInfos[pv.ZoneIndex].Patchs[pv.PatchIndex].Patch;
						sint vertIndex = ::GetWeldableVertex(bPatch, pv.Pos, weldThreshold);
						nlassert(vertIndex != -1); // should found one..
						// Follow this patch edge and see if the next/ previous  vertex could be welded with the next point
						const CVector &nextVertPos = bPatch.Vertices[(vertIndex +  1) & 0x03];
						const CVector &prevVertPos = bPatch.Vertices[(vertIndex -  1) & 0x03];

						if (::CanWeld(nextVertPos, borderPos[k + 1], middleEdgeWeldThreshold)
							|| 	::CanWeld(prevVertPos, borderPos[k + 1], middleEdgeWeldThreshold)
						   )
						{
							CPatchIdent pi1(0, l);
							CPatchIdent pi2(pv.ZoneIndex, pv.PatchIndex);
							CPatchIdentPair errPair = std::make_pair(pi1, pi2);
							//
							if (std::find(errorPairs.begin(), errorPairs.end(), errPair) == errorPairs.end()) // error already displayed ?
							{
								nlinfo("**** Patch %d of zone %s has 1 - 4 connectivity error, try binding it with patch %d of zone %s",
									   l + 1, middleZoneName.c_str(), pv.PatchIndex + 1, zoneNames[pv.ZoneIndex].c_str());
								++numErrors;
								errorPairs.insert(std::make_pair(pi2, pi1));
							}
							errorFound = true;
						}
					}
				}
			}
		}
	}
	////////////////////////////////
	////////////////////////////////
	if (numErrors != 0)
	{
		nlinfo("%d errors found", numErrors);
	}
	return numErrors;
}

//=========================================================================================================================
int main(int argc, char* argv[])
{
	NLMISC::createDebug();
	InfoLog->addNegativeFilter("adding the path");

	if (argc < 4)
	{
		std::string appName = CFile::getFilename(std::string(argv[0]));
		nlinfo("usage : %s <zonesDirectory><weldTheshold><middleEdgeWeldTheshold>\n", appName.empty() ? "zone_check_bind" : appName.c_str());
		return -1;
	}

	float weldThreshold, middleEdgeWeldThreshold;

	if (!fromString(argv[2], weldThreshold))
	{
		nlinfo("invalid weldThreshold");
		return -1;
	}

	if (!fromString(argv[3], middleEdgeWeldThreshold))
	{
		nlinfo("invalid middleEdgeWeldThreshold");
		return -1;
	}

	std::string zonePaths(argv[1]);


	if (zonePaths.empty())
	{
		nlinfo("Need a zone path");
		return -1;
	}

	// Filter addSearchPath
	CPath::addSearchPath(zonePaths);

	// Contains all the zone in the directory
	std::vector<std::string> zoneNames;

	CPath::getPathContent(zonePaths, true, false, true, zoneNames);

	uint numErrors = 0;
	// check'em
	for (uint k = 0; k < zoneNames.size(); ++k)
	{
		nlinfo("============================================================================");
		nlinfo("Checking : %s", zoneNames[k].c_str());
		numErrors += ::CheckZone(zoneNames[k], weldThreshold, middleEdgeWeldThreshold);
	}
	nlinfo("=======================");
	nlinfo("=======================");
	nlinfo("%d errors were found", numErrors);
}


