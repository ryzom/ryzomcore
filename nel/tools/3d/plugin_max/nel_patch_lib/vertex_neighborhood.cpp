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

#include "stdafx.h"
#include "vertex_neighborhood.h"

#define AVERAGE_NUM_POINT 1000

// **********************************************************************

CVertexNeighborhood vertexNeighborhoodGlobal;

// **********************************************************************

CVertexNeighborhood::CVertexNeighborhood()
{
	// Reserve a table for 1000 points
	_VectorIndex.reserve (2*AVERAGE_NUM_POINT+AVERAGE_NUM_POINT*5);
}

// **********************************************************************

void CVertexNeighborhood::build (const PatchMesh& patch)
{
	// Resize the table for the index entry
	_VectorIndex.resize (patch.numVerts*2, 0);

	// Count number of neighbor by vertex
	int i;
	for (i=0; i<patch.numEdges; i++)
	{
		if (patch.edges[i].v1!=-1)
			getNeighborCountRef (patch.edges[i].v1)++;
		if (patch.edges[i].v2!=-1)
			getNeighborCountRef (patch.edges[i].v2)++;
	}
	
	// Commpute the offset for each vertices
	uint finalSize=2*patch.numVerts;
	for (i=0; i<patch.numVerts; i++)
	{
		// Set the offset
		getNeighborIndexRef (i)=finalSize;

		// Increment this offset
		finalSize+=getNeighborCountRef (i);

		// Set size to 0
		getNeighborCountRef (i)=0;
	}

	// Resize the table for final size without erasing offsets
	_VectorIndex.resize (finalSize);

	// Fill the neighborhood info for each vertex
	for (i=0; i<patch.numEdges; i++)
	{
		if (patch.edges[i].v1!=-1)
		{
			// Get the vertex id
			uint vertexId=patch.edges[i].v1;

			// Add the edge to the list
			_VectorIndex[getNeighborIndexRef (vertexId)+getNeighborCountRef (vertexId)]=i;

			// Add a vertex in the list
			getNeighborCountRef (vertexId)++;
		}
		if (patch.edges[i].v2!=-1)
		{
			// Get the vertex id
			uint vertexId=patch.edges[i].v2;

			// Add the edge to the list
			_VectorIndex[getNeighborIndexRef (vertexId)+getNeighborCountRef (vertexId)]=i;

			// Add a vertex in the list
			getNeighborCountRef (vertexId)++;
		}
	}
}
