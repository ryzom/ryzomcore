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
#include "ligo_error.h"

namespace NLLIGO
{

// ***************************************************************************

CLigoError::CLigoError()
{
	clear ();
}

// ***************************************************************************

void CLigoError::pushVertexError (TError code, uint id, uint edge)
{
	// Check if this vertex exist in the error list
	uint vertex;
	for (vertex=0; vertex<_VertexError.size(); vertex++)
	{
		if (_VertexError[vertex].Id == id)
		{
			_VertexError[vertex] = CVertex (code, id, edge);
			break;
		}
	}

	// Not found ?
	if (vertex == _VertexError.size())
	{
		// Add a vertex error
		_VertexError.push_back (CVertex (code, id, edge));
	}
}

// ***************************************************************************

uint CLigoError::numVertexError () const
{
	return (uint)_VertexError.size ();
}

// ***************************************************************************

CLigoError::TError CLigoError::getVertexError (uint error, uint &id, uint &edge) const
{
	const CVertex &vertex = _VertexError[error];
	id = vertex.Id;
	edge = vertex.Edge;
	return vertex.Code;
}

// ***************************************************************************

void CLigoError::clear ()
{
	MainError = NoError;
	_VertexError.clear ();
}

// ***************************************************************************

const char* CLigoError::_StringError[CLigoError::ErrorCount]=
{
	"No error",						// NoError
	"No vertex in the edge list",	// NoEdgeVertices
	"Opened edges detected",		// OpenedEdge
	"Multiple edge on the boundary",// MultipleEdge
	"Vertex list invalid. One vertex should be a corner",	// VertexList
	"The vertex has not been inserted in the edge list",	// NotInserted
	"The vertex has been inserted in the edge list",		// Inserted
	"Flat zone, all vertices are in the same corner",		// FlatZone
	"The zone must have 4 edges to define a material",	// MustHave4Edges
	"A edge of the zone is not symetrical",		// NotSymetrical
	"Not the same number of vertex",	// NotSameVerticesNumber
	"Some vertices are not the same",	// NotSameVertex
	"No corner found",					// NoCornerFound
	"A edge has two times the same corner",	// TwoCornerVertices
	"A corner is missing in this edge", // CornerIsMissing
	"A boundary vertex is used by multiple edges", // VertexAlreadyUsed
	"Unknown error",						// UnknownError
};

// ***************************************************************************

const char* CLigoError::getStringError (TError errorCode)
{
	if (errorCode<=ErrorCount)
	{
		return _StringError[errorCode];
	}
	else
	{
		return _StringError[UnknownError];
	}
}

// ***************************************************************************

}
