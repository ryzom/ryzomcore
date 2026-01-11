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

#ifndef NL_QUAD_EFFECT_H
#define NL_QUAD_EFFECT_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2d.h"

namespace NL3D {



class IDriver;

/**
 * This class allow to create a sequence of small quads that tesselate a poly.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CQuadEffect
{
public:
	/// a vector of 2d points
	typedef std::vector<NLMISC::CVector2f> TPoint2DVect;

	// a vector of couple of float used with rasters (first is the left pos, and second is the right pos)
	typedef std::vector< std::pair< float, float> > TRasters;


	/** Make raters from the given clipped polygon.
	  * \param poly a polygon that describe the area where datas are taken from.
	  * \param quadWidth width of the quad
	  * \param quadHeight height of the quad
	  * \param dest a vector that will be filled with the given rasters
	  * \param startY will be filled with the start y position on screen
	  */
	static void makeRasters(const TPoint2DVect &poly
							, float quadWidth, float quadHeight
							, TRasters &dest, float &startY
						   );

	/** Tesselate the given clipped polygon ,  by using the given quad dimensions
	  * The coordinates of the poly are given in screen coordinate.
	  * \param poly a polygon that describe the area where datas are taken from.
	  * \param quadWidth width of the quad
	  * \param quadHeight height of the quad
	  * \param dest a vector that will contains the pos of all the quads that cover the poly
	  */
	static void processPoly(const TPoint2DVect &poly
							, float quadWidth, float quadHeight
							, TPoint2DVect &dest
						   );

};


} // NL3D


#endif // NL_QUAD_EFFECT_H

/* End of quad_effect.h */
