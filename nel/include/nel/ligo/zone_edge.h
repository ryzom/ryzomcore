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

#ifndef NL_ZONE_EDGE_H
#define NL_ZONE_EDGE_H

// NeL include
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

// STL include
#include <vector>

namespace NLMISC
{
	class IStream;
	class CMatrix;
}

namespace NLLIGO
{

class CLigoError;
class CLigoConfig;

/**
 * A ZoneEdge descriptor
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CZoneEdge
{
public:

	/// Build a edge zone
	bool build (const std::vector<NLMISC::CVector> &theEdge, const std::vector<uint32> &theId, uint rotation,
				sint32 offsetX, sint32 offsetY, const CLigoConfig &config, CLigoError &errors);

	/// Serial
	void serial (NLMISC::IStream& s);

	/// Is symetrical ?
	bool isSymetrical (const CLigoConfig &config, CLigoError &errors) const;

	/// Is the same edge ?
	bool isTheSame (const CZoneEdge &other, const CLigoConfig &config, CLigoError &errors) const;

	/// Invert the edge
	void invert (const CLigoConfig &config);

	/// Return the vertex count
	uint getNumVertex () const { return (uint)_TheEdge.size(); }

	/// Return the vertex
	const NLMISC::CVector& getVertex (uint id) const { return _TheEdge[id]; }

	/// Return the matrix
	void buildMatrix (NLMISC::CMatrix& mat, const CLigoConfig &config) const;

	/// Get values
	uint32 getRotation () const { return _Rotation; }
	sint32 getOffsetX () const { return _OffsetX; }
	sint32 getOffsetY () const { return _OffsetY; }

private:

	/// The vector of position for this edge
	std::vector<NLMISC::CVector>	_TheEdge;

	/// Id of the vertices
	std::vector<uint32>				_Id;

	/// Rotation of the edge. Must be 0, 1, 2, 3. The rotation angle is Pi/2 * (double)_Rotation in CCW.
	uint32							_Rotation;

	/// X an Y offset of the edge. the position of the i-th vertex is rotate (_Rotation) * (theEdge[i], 0, 0) + (_OffsetX, _OffsetY, 0)
	sint32							_OffsetX;
	sint32							_OffsetY;
};

}

#endif // NL_ZONE_EDGE_H

/* End of zone_edge.h */
