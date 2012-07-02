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

#ifndef NL_QUAD_H
#define NL_QUAD_H

#include "types_nl.h"
#include "vector.h"


namespace NLMISC {


// ***************************************************************************
/**
 * A simple quad of vertex.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CQuad
{
public:
	CVector		V0, V1, V2, V3;


public:

	/// Constructor
	CQuad() {}

	/// Constructor
	CQuad(const NLMISC::CVector &v0, const NLMISC::CVector &v1, const NLMISC::CVector &v2, const NLMISC::CVector &v3)
		: V0(v0), V1(v1), V2(v2), V3(v3)
	{}

	void set(const NLMISC::CVector &v0, const NLMISC::CVector &v1, const NLMISC::CVector &v2, const NLMISC::CVector &v3)
	{
		V0 = v0;
		V1 = v1;
		V2 = v2;
		V3 = v3;
	}

	const CQuad &operator = ( const CQuad& q)
	{
		V0 = q.V0;
		V1 = q.V1;
		V2 = q.V2;
		V3 = q.V3;
		return *this;
	}

};


} // NLMISC


#endif // NL_QUAD_H

/* End of quad.h */
