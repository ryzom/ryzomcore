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

#ifndef NL_U_GLOBAL_POSITION_H
#define NL_U_GLOBAL_POSITION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NLPACS
{


/**
 * An estimation of the position of a point on a specified surface.
 * The estimated position is LOCAL reference to the retriever axis.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class ULocalPosition
{
public:
	/// The id of the surface corresponding to the local position.
	sint32							Surface;

	/// The best position estimation of the point snapped on the surface. This is a CLocalRetriever local axis position.
	NLMISC::CVector					Estimation;

	// set this global position from another global position, but keep the old z coordinate
	void								setKeepZ(const ULocalPosition &other)
	{
		Surface = other.Surface;
		Estimation.set(other.Estimation.x,
					   other.Estimation.y,
					   Estimation.z);
	}
};


/**
 * The global position in the global retriever.
 * Contains an instance id and a local position within the instance.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class UGlobalPosition
{
public:

	enum TType
	{
		Unspecified = 0,
		Landscape,
		Interior
	};

	/// The id of the instance referred by this global position.
	sint32								InstanceId;

	/// The local position within the CLocalRetriever referred by the instance (cf. InstanceId)
	ULocalPosition						LocalPosition;

	/// default constructor
	UGlobalPosition()
	{
		InstanceId = -1;
	}

	// set this global position from another global position, but keep the old z coordinate
	void								setKeepZ(const UGlobalPosition &other)
	{
		InstanceId = other.InstanceId;
		LocalPosition.setKeepZ(other.LocalPosition);
	}
};


} // NLPACS


#endif // NL_U_GLOBAL_POSITION_H

/* End of u_global_position.h */
