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

#include "stdligo.h"
#include "zone_template.h"
#include "ligo_error.h"
#include "nel/ligo/ligo_config.h"

#include "nel/misc/stream.h"
#include "nel/misc/matrix.h"

using namespace std;
using namespace NLMISC;

namespace NLLIGO
{

const uint SnappedXFlag = 1;
const uint SnappedYFlag = 2;

// ***************************************************************************

inline void CZoneTemplate::snap (float& value, float snap)
{
	// Snap it
	value  = snap * (float) floor ( (value / snap) + 0.5f );
}

// ***************************************************************************

inline bool CZoneTemplate::snapOnGrid (float& value, float resolution, float snap)
{
	// Calc the floor
	float _floor = (float) ( resolution * floor (value / resolution) );
	nlassert (_floor<=value);

	// Calc the remainder
	float remainder = value - _floor;
	//nlassert ( (remainder>=0) && (remainder<resolution) );

	// Check the snape
	if ( remainder <= snap )
	{
		// Flag it
		value = _floor;

		// Floor is good
		return true;
	}
	else if ( (resolution - remainder) <= snap )
	{
		// Flag it
		value = _floor + resolution;

		// Floor + resolution is good
		return true;
	}
	return false;
}

// ***************************************************************************

inline bool CZoneTemplate::isSnapedOnGrid (float value, float resolution, float snap)
{
	// Snapped
	float snapped = value;
	return snapOnGrid (snapped, resolution, snap);
}

// ***************************************************************************

inline sint32 CZoneTemplate::getSnappedIndex (float value, float resolution, float snap)
{
	// Snapped
	float snapped = value;

	// This value must be snapped
	nlverify (snapOnGrid (snapped, resolution, snap));

	// Return the index
	return (sint32) floor ( (snapped / resolution) + 0.5f );
}

// ***************************************************************************

bool CZoneTemplate::build (const std::vector<NLMISC::CVector> &vertices, const std::vector< std::pair<uint, uint> > &indexes, const CLigoConfig &config, CLigoError &errors)
{
	// Clear the error message
	errors.clear ();

	// Make an boundary flag array
	vector<uint>		boundaryFlags;

	// Vertices count
	uint vertexCount = (uint)vertices.size();

	// Resize the array
	boundaryFlags.resize (vertexCount, 0);

	// *** Build the flag array and the snapped vertex array

	// For each vertices
	uint vertex;
	for (vertex = 0; vertex < vertexCount; vertex++)
	{
		// Snap the point on the X grid
		if (isSnapedOnGrid (vertices[vertex].x, config.CellSize, config.Snap))
			// Flag on X
			boundaryFlags[vertex]|=SnappedXFlag;

		// Snap the point on the Y grid
		if (isSnapedOnGrid (vertices[vertex].y, config.CellSize, config.Snap))
			// Flag on Y
			boundaryFlags[vertex]|=SnappedYFlag;
	}

	// *** Build the edge set
	multimap<uint, uint>	edgePair;
	multimap<uint, uint>	edgePairReverse;

	// Index count
	uint edgeCount = (uint)indexes.size();

	// For each vertices
	uint edge;
	for (edge = 0; edge < edgeCount; edge++)
	{
		// Ref on the pair
		const pair<uint, uint> &theEdge = indexes[edge];

		// Vertex snapped ?
		if ( boundaryFlags[theEdge.first] && boundaryFlags[theEdge.second] )
		{
			// Common coordinates
			uint common = boundaryFlags[theEdge.first] & boundaryFlags[theEdge.second];

			// Snapped on the same kind of coordinates ?
			if ( common )
			{
				// Keep this edge ?
				bool keep = false;

				// Snapped both on X ?
				if ( common & SnappedXFlag )
				{
					// Keep it
					keep = true;
				}

				// Snapped both on X ?
				if ( common & SnappedYFlag )
				{
					// Keep it
					keep = true;
				}

				// Keep this edge ?
				if (keep)
				{
					// Already inserted ?
					bool first = edgePair.find (theEdge.first) != edgePair.end();
					bool second = edgePairReverse.find (theEdge.second) != edgePairReverse.end();

					// First already inserted
					if (first || second)
					{
						// Error, two times the same vertex
						errors.MainError = CLigoError::VertexAlreadyUsed;

						if (first)
							errors.pushVertexError (CLigoError::VertexAlreadyUsed, theEdge.first, 0);

						if (second)
							errors.pushVertexError (CLigoError::VertexAlreadyUsed, theEdge.second, 0);

						return false;
					}

					if ((!first) && (!second))
					{
						// Add to the map
						edgePair.insert (map<uint, uint>::value_type(theEdge.first, theEdge.second));
						edgePairReverse.insert (map<uint, uint>::value_type(theEdge.second, theEdge.first));
					}
				}
			}
		}
	}

	// *** Build the list of non included vertices

	// For each vertices
	for (uint i=0; i<vertexCount; i++)
	{
		// Vertex is inserted ?
		if (edgePair.find (i) == edgePair.end())
		{
			// No, add an error message
			errors.pushVertexError (CLigoError::NotInserted, i, 0);
		}
		else
		{
			// No, add an error message
			errors.pushVertexError (CLigoError::Inserted, i, 0);
		}
	}

	// *** Build the linked list

	// No vertices found ?
	if (edgePair.begin() == edgePair.end())
	{
		// Error message
		errors.MainError = CLigoError::NoEdgeVertices;
		return false;
	}

	// Build the linked segments
	list<list<uint> >	segmentList;
	multimap<uint, uint>::iterator currentVert = edgePair.begin();

	// For each remaining segment
	while (currentVert != edgePair.end())
	{
		// Get next vert
		uint first = currentVert->first;
		uint next = currentVert->second;

		// New list
		segmentList.push_front (list<uint>());
		list<uint> &listVert = *segmentList.begin();

		// Put the first vertices of the edge list
		listVert.push_back (first);
		listVert.push_back (next);

		// Erase it and
		edgePair.erase (currentVert);

		// Erase the reverse one
		currentVert = edgePairReverse.find (next);
		nlassert (currentVert != edgePairReverse.end());
		edgePairReverse.erase (currentVert);

		// Look forward
		currentVert = edgePair.find (next);
		while (currentVert != edgePair.end())
		{
			// Backup
			//uint current = currentVert->first;
			next = currentVert->second;

			// Push the next vertex
			listVert.push_back (next);

			// Erase it and
			edgePair.erase (currentVert);

			// Erase the reverse one
			currentVert = edgePairReverse.find (next);
			nlassert (currentVert != edgePairReverse.end());
			edgePairReverse.erase (currentVert);

			// Look forward
			currentVert = edgePair.find (next);
		}

		// Edgelist ok ?
		if (next != first)
		{
			// No, look backward
			currentVert = edgePairReverse.find (first);
			while (currentVert != edgePairReverse.end())
			{
				// Backup
				uint current = currentVert->second;
				next = currentVert->first;

				// Push the next vertex
				listVert.push_front (current);

				// Erase it
				edgePairReverse.erase (currentVert);

				// Erase the reverse one
				currentVert = edgePair.find (current);
				nlassert (currentVert != edgePair.end());
				edgePair.erase (currentVert);

				// Look forward
				currentVert = edgePairReverse.find (current);
			}
		}

		// Next edge list
		currentVert = edgePair.begin();
	}

	// ** Error traitment

	// Ok
	bool ok = true;

	// Edge index
	uint edgeIndex = 0;

	// List ok ?
	list<list<uint> >::iterator iteList = segmentList.begin ();
	while (iteList != segmentList.end())
	{
		// Only one list
		list<uint> &listVert = *iteList;

		// First and last edge
		uint first = *listVert.begin();
		uint last = *(--listVert.end());

		// Opened edge ?
		if ( first != last )
		{
			// Opened edge
			errors.pushVertexError (CLigoError::OpenedEdge, first, edgeIndex);
			errors.pushVertexError (CLigoError::OpenedEdge, last, edgeIndex);

			// Main error
			errors.MainError = CLigoError::OpenedEdge;

			// Not ko
			ok = false;
		}

		// Next edge list
		edgeIndex++;
		iteList++;
	}

	if (segmentList.size () > 1)
	{
		// Main error
		errors.MainError = CLigoError::MultipleEdge;

		// Not ok
		ok = false;
	}

	// Ok ?
	if (ok)
	{
		// Only one list
		list<uint> &listVert = *segmentList.begin ();

		// Test vertex enchainement
		list<uint>::iterator vertIte = listVert.begin();

		// Current vertex id
		uint previous = *(--listVert.end());
		vertIte++;

		// Error vertex set
		set<uint> errored;

		// For each vertices
		while (vertIte != listVert.end ())
		{
			// Vertex id
			uint next = *vertIte;

			// Common flags
			uint commonFlags = boundaryFlags[previous]&boundaryFlags[next];

			// The both on X ?
			if ( commonFlags & SnappedXFlag )
			{
				// Get x index
				sint32 prevIndex = getSnappedIndex (vertices[previous].x, config.CellSize, config.Snap);
				sint32 nextIndex = getSnappedIndex (vertices[next].x, config.CellSize, config.Snap);

				// Not the same ?
				if (prevIndex != nextIndex)
				{
					// Vertex list error
					if (errored.insert (previous).second)
						errors.pushVertexError (CLigoError::VertexList, previous, 0);
					if (errored.insert (next).second)
						errors.pushVertexError (CLigoError::VertexList, next, 0);

					// Main error
					errors.MainError = CLigoError::VertexList;
				}
			}

			// Next vertex
			previous = next;
			vertIte++;
		}

		// No error ?
		if (errored.empty())
		{
			// Only one list
			nlassert (segmentList.size()==1);

			// First of the list
			vertIte = listVert.begin();

			// Remove first
			listVert.erase (vertIte);

			// Find a corner
			list<uint>::iterator firstIte = listVert.begin();
			while (firstIte != listVert.end())
			{
				// Corner ?
				if ( (boundaryFlags[*firstIte] & (SnappedXFlag|SnappedYFlag)) == (SnappedXFlag|SnappedYFlag) )
					// Yes, exit
					break;

				// Next
				firstIte++;
			}

			// Can't be the last
			if (firstIte == listVert.end())
			{
				// No corner found
				errors.MainError = CLigoError::NoCornerFound;

				return false;
			}

			// First of the segment
			vertIte = firstIte;

			// Current edge list
			std::vector<uint32> edge;

			// Push the first
			edge.push_back (*vertIte);

			// Next
			vertIte++;

			// End ?
			if (vertIte == listVert.end())
				// Start
				vertIte = listVert.begin();

			// Edge index
			uint edgeIndex = 0;

			// Build the edges
			for(;;)
			{
				// Add it
				edge.push_back (*vertIte);

				// Corner ?
				if ( (boundaryFlags[*vertIte] & (SnappedXFlag|SnappedYFlag)) == (SnappedXFlag|SnappedYFlag) )
				{
					// Get the index of start and end of the edge
					sint32 startX = getSnappedIndex (vertices[edge[0]].x, config.CellSize, config.Snap);
					sint32 startY = getSnappedIndex (vertices[edge[0]].y, config.CellSize, config.Snap);
					sint32 endX = getSnappedIndex (vertices[edge[edge.size()-1]].x, config.CellSize, config.Snap);
					sint32 endY = getSnappedIndex (vertices[edge[edge.size()-1]].y, config.CellSize, config.Snap);

					// Same point ?
					if ((startX==endX) && (startY==endY))
					{
						// Error, two times the same vertex
						errors.MainError = CLigoError::TwoCornerVertices;
						errors.pushVertexError (CLigoError::TwoCornerVertices, edge[0], 0);
						errors.pushVertexError (CLigoError::TwoCornerVertices, edge[edge.size()-1], 0);

						return false;
					}

					// Same point ?
					if ((abs(startX-endX)>1) || (abs(startY-endY)>1))
					{
						// Error, two times the same vertex
						errors.MainError = CLigoError::CornerIsMissing;
						errors.pushVertexError (CLigoError::CornerIsMissing, edge[0], 0);
						errors.pushVertexError (CLigoError::CornerIsMissing, edge[edge.size()-1], 0);

						return false;
					}

					// Get rotation
					uint rotation = 4;
					if ((endX-startX)==1)
					{
						if ((endY-startY)==0)
							rotation = 0;
					}
					else if ((endX-startX)==-1)
					{
						if ((endY-startY)==0)
							rotation = 2;
					}
					else if ((endX-startX)==0)
					{
						if ((endY-startY)==1)
							rotation = 1;
						else if ((endY-startY)==-1)
							rotation = 3;
					}

					// Checks
					nlassert (rotation != 4);

					// Build the vertex array
					vector<CVector> vertexArray;
					vertexArray.resize (edge.size());

					// Rotate matrix
					CMatrix mat;
					mat.identity();
					mat.rotateZ ((float)rotation * (float)Pi / 2);
					mat.setPos (CVector (vertices[edge[0]].x, vertices[edge[0]].y, 0));
					mat.invert ();

					// Rotate the array
					for (uint i=0; i<edge.size(); i++)
					{
						// Get the value on the edge
						vertexArray[i] = mat * vertices[edge[i]];
					}

					// Build the edge
					_Edges.resize (edgeIndex+1);

					// It must work without errors
					CLigoError errorBis;
					if (!_Edges[edgeIndex].build (vertexArray, edge, rotation, startX, startY, config, errorBis))
					{
						// Flat zone
						errors.MainError = CLigoError::FlatZone;

						return false;
					}

					// One more edge
					edgeIndex++;

					// Exit ?
					if (vertIte == firstIte)
						break;

					// Clear the temp edge
					edge.clear ();

					// Push back the last vertex
					edge.push_back (*vertIte);
				}

				// Next vertex
				vertIte++;

				// End ?
				if (vertIte == listVert.end())
					// Start
					vertIte = listVert.begin();
			}

			sint32 bestX = 0x7fffffff;
			sint32 bestY = 0x80000000;
			uint bestEdge = 0xffffffff;

			// Sort edges : the first as the lower x then greater y
			uint edgeId;
			for (edgeId=0; edgeId<_Edges.size(); edgeId++)
			{
				// Get the matrix
				CMatrix mat;
				_Edges[edgeId].buildMatrix (mat, config);

				// First vertex
				CVector pos = mat * _Edges[edgeId].getVertex (0);

				// Get X and Y
				sint32 x = getSnappedIndex (pos.x, config.CellSize, config.Snap);
				sint32 y = getSnappedIndex (pos.y, config.CellSize, config.Snap);

				// Best ?
				if ((x<bestX)||((x==bestX)&&(y>bestY)))
				{
					// This edgeId is best
					bestX=x;
					bestY=y;
					bestEdge = edgeId;
				}
			}

			// Check
			nlassert (bestEdge!=0xffffffff);

			// Reoder
			std::vector<CZoneEdge>	newEdge (_Edges.size());
			for (edgeId=0; edgeId<_Edges.size(); edgeId++)
			{
				// Copy the edge
				newEdge[edgeId]=_Edges[bestEdge++];

				// Next
				if (bestEdge==_Edges.size())
					bestEdge=0;
			}

			// Copy the final array
			_Edges=newEdge;

			// Return ok
			return true;
		}
	}

	// Errors.
	return false;
}

// ***************************************************************************

void CZoneTemplate::serial (NLMISC::IStream& s)
{
	// open an XML node
	s.xmlPush ("LIGO_ZONE_TEMPLATE");

		// An header file
		s.serialCheck (string ("LigoZoneTemplate") );

		// Node for the boundaries
		s.xmlPush ("EDGES");

			// Serial the Vertices
			s.serialCont (_Edges);

		// Node for the boundaries
		s.xmlPop ();

	// Close the node
	s.xmlPop ();
}

// ***************************************************************************

void CZoneTemplate::getMask (std::vector<bool> &mask, uint &width, uint &height)
{
	// Some constantes
	static const sint32 addX[4] = { 1, 0, -1, 0 };
	static const sint32 addY[4] = { 0, 1, 0, -1 };
	static const sint32 cellX[4] = { 0, -1, -1, 0 };
	static const sint32 cellY[4] = { 0, 0, -1, -1 };
	static const sint32 moveX[4] = { 0, 1, 0, -1 };
	static const sint32 moveY[4] = { -1, 0, 1, 0 };

	// Max
	sint32 xMax = 0x80000000;
	sint32 yMax = 0x80000000;

	// For each edges
	uint edges;
	for (edges=0; edges<_Edges.size(); edges++)
	{
		// Get the rotation
		uint32 rot = _Edges[edges].getRotation ();
		nlassert (rot<4);

		// Get X and Y max coordinates
		sint32 x = _Edges[edges].getOffsetX () + addX[rot];
		sint32 y = _Edges[edges].getOffsetY () + addY[rot];

		// Greater ?
		if (x > xMax)
			xMax = x;
		if (y > yMax)
			yMax = y;
	}

	// Build the array
	width = (uint32) xMax;
	height = (uint32) yMax;

	// Bit array for each cell
	vector<uint32> edgeArray (xMax*yMax, 0);

	// Resize it
	mask.resize (xMax*yMax, false);

	// Set of the cells in the mask
	set<pair<sint32, sint32> > setCell;

	// For each edge
	for (edges=0; edges<_Edges.size(); edges++)
	{
		// Get the rotation
		uint32 rot = _Edges[edges].getRotation ();
		nlassert (rot<4);

		// Get its x and y cell coordinate
		sint32 x = _Edges[edges].getOffsetX () + cellX[rot];
		sint32 y = _Edges[edges].getOffsetY () + cellY[rot];

		// Fill the edge array
		edgeArray[x+y*width] |= (1<<rot);

		// Insert the cell
		setCell.insert ( pair<sint32, sint32> (x, y) );
	}

	// Second set
	set<pair<sint32, sint32> > setCell2;

	// For each element in the set
	set<pair<sint32, sint32> >::iterator ite = setCell.begin();
	while (ite != setCell.end())
	{
		// For each direction
		for (uint dir=0; dir<4; dir++)
		{
			// Get its x and y cell coordinate
			sint32 x = ite->first;
			sint32 y = ite->second;

			// Edge in this direction ?
			while ( (edgeArray[x+y*width] & (1<<dir) ) == 0)
			{
				// Move in this direction
				x += moveX[dir];
				y += moveY[dir];

				// insert it
				setCell2.insert ( pair<sint32, sint32> (x, y) );

				// Some checks
				nlassert (x>=0);
				nlassert (x<(sint32)width);
				nlassert (y>=0);
				nlassert (y<(sint32)height);
			}
		}

		// Next one
		ite++;
	}

	// Merge the two set
	ite = setCell2.begin();
	while (ite != setCell2.end())
	{
		// Merge
		setCell.insert (*ite);

		// Next element
		ite++;
	}

	// Done, fill the array
	ite = setCell.begin();
	while (ite != setCell.end())
	{
		// Merge
		mask[ite->first+ite->second*width] = true;

		// Next element
		ite++;
	}
}

// ***************************************************************************

}
