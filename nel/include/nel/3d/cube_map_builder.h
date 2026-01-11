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


#ifndef CUBE_MAP_BUILDER_H
#define CUBE_MAP_BUILDER_H

#include "nel/misc/rgba.h"

namespace NLMISC
{
	class CVector;
}

namespace NL3D
{

class CTextureCube;

/** A cube map functor should return a color from a vector (with each coordinate ranging from [-1..1]
  */
struct ICubeMapFunctor
{
	virtual ~ICubeMapFunctor() {}
	virtual NLMISC::CRGBA operator()(const NLMISC::CVector &v) = 0;
};

/** Build a cube map by using the given functor. This also avoid headaches :)
  * Each face is encoded in a memory texture
  * \param mapSize the size of each tface of the cube map
  * \param f a functor that helps to build the cube map.
  * \param luminanceOnly When set to true, a luminance cube map is build. The luminance is taken from the alpha component of the color produced by the functor.
  *                      Warning : this isn't supported anywhere.
  * \param shareName a prefix for sharename. If not empty this allow each face of the cube map to be sharable (a number is appended to the given string)
  */
CTextureCube *BuildCubeMap(sint mapSize, ICubeMapFunctor &f, bool luminanceOnly = false, const std::string &shareName = "");

}


#endif

