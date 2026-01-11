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

#ifndef NL_LIGO_ERROR_H
#define NL_LIGO_ERROR_H

#include "nel/misc/types_nl.h"

#include <vector>

namespace NLLIGO
{

/**
 * Error handling
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CLigoError
{
public:
	enum TError
	{
		/// No error
		NoError				=0,

		/// No vertices in the edge list
		NoEdgeVertices,

		/// Opened edges detected
		OpenedEdge,

		/// Multiple edge on the boundary
		MultipleEdge,

		/// Vertex list invalid. One vertex should be a corner.
		VertexList,

		/// The vertex has not been inserted in the edge list
		NotInserted,

		/// The vertex has been inserted in the edge list
		Inserted,

		/// Flat zone, all vertex are in the same corner
		FlatZone,

		/// 4 edge to define a material
		MustHave4Edges,

		/// 4 edge to define a material
		NotSymetrical,

		/// Not same number of vertices
		NotSameVerticesNumber,

		/// Vertex is not the same
		NotSameVertex,

		/// No corner found
		NoCornerFound,

		/// Two corner vertices
		TwoCornerVertices,

		/// Corner is missing
		CornerIsMissing,

		/// Boundary vertex used by multiple edge
		VertexAlreadyUsed,

		/// Unknown error
		UnknownError,

		/// Error count
		ErrorCount
	};

	/// Constructor
	CLigoError();

	/// Clear the container
	void clear ();

	/// Push a vertex error code
	void pushVertexError (TError code, uint id, uint edge=0xffffffff);

	/// Get num vertex error
	uint numVertexError () const;

	/// Get a vertex error
	TError getVertexError (uint error, uint &id, uint &edge) const;

	/// Get an error string
	static const char* getStringError (TError errorCode);

public:

	/// Main error code
	TError		MainError;

private:

	/// Vertex error
	class CVertex
	{
	public:

		/// Constructor
		CVertex (TError code, uint id, uint edge)
		{
			Code = code;
			Id = id;
			Edge = edge;
		}
	public:

		/// Error code on this vertex
		TError	Code;

		/// Vertex id
		uint	Id;

		/// Edge id (no always used)
		uint	Edge;
	};

	/// Vertex error
	std::vector<CVertex>		_VertexError;

	/// Error string
	static const char* _StringError[];
};

}

#endif // NL_LIGO_ERROR_H

/* End of ligo_error.h */
