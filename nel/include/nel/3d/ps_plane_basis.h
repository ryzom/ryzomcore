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

#ifndef NL_PS_PLANE_BASIS_H
#define NL_PS_PLANE_BASIS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"
#include "nel/misc/traits_nl.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/ps_attrib_maker_helper.h"


namespace NL3D {





/** A basis for plane object, used by particle by face and shockwaves
 *  It's a like a 2x3 matrix, (with only the X and Y vector defined)
 */

struct CPlaneBasis
{
	NLMISC::CVector X ;
	NLMISC::CVector Y ;


	// default ctor
	CPlaneBasis() {}


	// construct this basis by giving a normal to the plane that contains it
	CPlaneBasis(const NLMISC::CVector &normal)
	{
		NLMISC::CMatrix mat;
		CPSUtil::buildSchmidtBasis(normal, mat) ;
		X = mat.getI() ;
		Y = mat.getJ() ;
	}

	// construct this basis by giving its X and Y vectors
	CPlaneBasis(const NLMISC::CVector &x, const NLMISC::CVector &y) : X(x), Y(y)
	{
	}

	/// compute the normal of the plane basis
	NLMISC::CVector getNormal(void) const
	{
		return X ^ Y ;
	}


	void serial(NLMISC::IStream &f)
	{
		f.serial(X, Y) ;
	}
} ;


// for map insertion

inline bool operator<(const CPlaneBasis &p1, const CPlaneBasis &p2)
{
	if (p1.X != p2.X) return p1.X < p2.X;
	return p1.Y < p2.Y;
}


} // NL3D

// special traits for optimization
namespace NLMISC
{
	NL_TRIVIAL_TYPE_TRAITS(NL3D::CPlaneBasis);
}



#endif // NL_PS_PLANE_BASIS_H

/* End of ps_plane_basis.h */
