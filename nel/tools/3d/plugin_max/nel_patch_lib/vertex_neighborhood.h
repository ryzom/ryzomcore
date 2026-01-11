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

#ifndef NL_VERTEX_NEIGHBORHOOD_H
#define NL_VERTEX_NEIGHBORHOOD_H

#include "nel/misc/types_nl.h"

#include <vector>

class PatchMesh;

/**
 * TODO Class description
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CVertexNeighborhood
{
public:
	// Default constructor reserve a good array.
	CVertexNeighborhood ();

	// build it 
	void build (const PatchMesh& patch);

	// Get neighbor index
	uint getNeighborCount (uint neighborId) const
	{
		return _VectorIndex[2*neighborId];
	}

	// Get neighbor index
	uint& getNeighborCountRef (uint neighborId)
	{
		return _VectorIndex[2*neighborId];
	}

	// Get neighbor list
	const uint* getNeighborList (uint neighborId) const
	{
		// Const iterator
		std::vector<uint>::const_iterator ite=_VectorIndex.begin();
		return (&*ite) + getNeighborIndex (neighborId);
	}

	// Get neighbor list
	uint* getNeighborList (uint neighborId)
	{
		return &_VectorIndex[getNeighborIndex (neighborId)];
	}

	// Get neighbor count
	uint getNeighborIndex (uint neighborId) const
	{
		return _VectorIndex[2*neighborId+1];
	}

	// Get neighbor count
	uint& getNeighborIndexRef (uint neighborId)
	{
		return _VectorIndex[2*neighborId+1];
	}
private:
	std::vector<uint>	_VectorIndex;
};

extern CVertexNeighborhood vertexNeighborhoodGlobal;

#endif // NL_VERTEX_NEIGHBORHOOD_H

/* End of vertex_neighborhood.h */
