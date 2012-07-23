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
// Ligo include
#include "zone_edge.h"
#include "nel/ligo/ligo_config.h"
#include "ligo_error.h"

// NeL include
#include "nel/misc/matrix.h"

using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************

bool CZoneEdge::build (const std::vector<NLMISC::CVector> &theEdge, const std::vector<uint32> &theId, uint rotation,
				sint32 offsetX, sint32 offsetY, const CLigoConfig &config, CLigoError &errors)
{
	// Some checks
// no need, it s an uint	nlassert (rotation>=0);
	nlassert (rotation<=3);
	nlassert (theEdge.size() == theId.size());

	// Cancels errors
	errors.clear ();

	// Errors ?
	bool ok = true;

	// Check first position
	CVector toCheck (theEdge[0].x, theEdge[0].y, 0);
	if ((float)fabs (toCheck.norm())>config.Snap)
	{
		// Vertex error
		errors.pushVertexError (CLigoError::UnknownError, 0);
		ok = false;
	}

	// Check last position
	uint lastIndex = (uint)theEdge.size()-1;
	toCheck  = CVector (theEdge[lastIndex].x, theEdge[lastIndex].y, 0);
	if (((toCheck-CVector (config.CellSize, 0, 0)).norm())>config.Snap)
	{
		// Vertex error
		errors.pushVertexError (CLigoError::UnknownError, 0);
		ok = false;
	}

	// No error ? Build!
	if (ok)
	{
		_TheEdge = theEdge;
		_Rotation = rotation;
		_OffsetX = offsetX;
		_OffsetY = offsetY;
		_Id = theId;
	}

	return ok;
}

// ***************************************************************************

bool CZoneEdge::isSymetrical (const CLigoConfig &config, CLigoError &errors) const
{
	// Cancels errors
	errors.clear ();

	// Errors ?
	bool ok = true;

	// For each internal vertices
	uint vert;
	for (vert=0; vert<_TheEdge.size(); vert++)
	{
		// Symmetrical value
		CVector sym = CVector (config.CellSize-_TheEdge[vert].x, _TheEdge[vert].y, _TheEdge[vert].z);

		// Others vertices
		uint vert2;
		for (vert2=0; vert2<_TheEdge.size(); vert2++)
		{
			// Not the same ?
			if (vert != vert2)
			{
				// Snapped ?
				if ((_TheEdge[vert2]-sym).norm() <= config.Snap)
				{
					// Good, next one
					break;
				}
			}
		}

		// Not found ?
		if (vert2>=_TheEdge.size())
		{
			// Error
			ok = false;

			// Push error message
			errors.pushVertexError (CLigoError::NotSymetrical, _Id[vert]);
			errors.MainError = CLigoError::NotSymetrical;
		}
	}

	// Return error code
	return ok;
}

// ***************************************************************************

bool CZoneEdge::isTheSame (const CZoneEdge &other, const CLigoConfig &config, CLigoError &errors) const
{
	// Same vertex count ?
	if (_TheEdge.size() != other._TheEdge.size())
	{
		// Error
		errors.MainError = CLigoError::NotSameVerticesNumber;
		return false;
	}

	// Errors ?
	bool ok = true;

	// For each internal vertices
	uint vert;
	for (vert=0; vert<_TheEdge.size(); vert++)
	{
		// The same ?
		const CVector &pos0 = _TheEdge[vert];
		const CVector &pos1 = other._TheEdge[vert];
		if ((pos0-pos1).norm() > config.Snap)
		{
			// Error
			ok = false;

			// Push error message
			errors.pushVertexError (CLigoError::NotSameVertex, other._Id[vert]);
			errors.MainError = CLigoError::NotSameVertex;
		}
	}

	// Return error code
	return ok;
}

// ***************************************************************************

void CZoneEdge::serial (NLMISC::IStream& s)
{
	// Serial the version
	/*sint ver =*/ s.serialVersion (0);

	s.xmlPush ("VERTICES");
		s.serialCont (_TheEdge);
	s.xmlPop ();

	s.xmlPush ("VERTICES_ID");
		s.serialCont (_Id);
	s.xmlPop ();

	s.xmlSerial (_Rotation, "ROTATION");

	s.xmlSerial (_OffsetX, _OffsetY, "OFFSET");
}

// ***************************************************************************

void CZoneEdge::invert (const CLigoConfig &config)
{
	// Copy the array
	const std::vector<NLMISC::CVector>	copy = _TheEdge;

	// For each internal vertices
	uint vert;
	for (vert=0; vert<_TheEdge.size(); vert++)
	{
		// Invert
		const CVector &pos = copy[_TheEdge.size()-vert-1];
		_TheEdge[vert] = CVector (config.CellSize - pos.x, pos.y, pos.z);
	}
}

// ***************************************************************************

void CZoneEdge::buildMatrix (NLMISC::CMatrix& mat, const CLigoConfig &config) const
{
	// Build a transformation matrix
	mat.identity();
	mat.rotateZ ((float)Pi*(float)_Rotation/2.f);
	mat.setPos (CVector (config.CellSize*(float)_OffsetX, config.CellSize*(float)_OffsetY, 0));
}

// ***************************************************************************

}
