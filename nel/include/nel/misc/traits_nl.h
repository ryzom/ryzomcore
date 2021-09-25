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

#ifndef NL_TRAITS_H
#define NL_TRAITS_H


#include "rgba.h"
#include "vector.h"

namespace NLMISC
{

/** Class that gives information about a type. Useful to do some optimization in templates functions / class
  * This class is intended to be specialized and taylored for each type of interest
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2004
  */
template <class T>
struct CTraits
{
	enum { HasTrivialCtor = false };     // if true, the default ctor does nothing useful (example  : built-in types or Plain Old Datas structs)
	enum { HasTrivialDtor = false };     // the dtor does nothing useful and is not worth calling. Useful to optimize containers clean-up
	enum { SupportRawCopy = false };     // the object supports raw copy with memcpy
	// to be completed ..
};


#define NL_TRIVIAL_TYPE_TRAITS(type)     \
template <>								 \
struct CTraits<type>                     \
{                                        \
	enum { HasTrivialCtor = true };      \
	enum { HasTrivialDtor = true };      \
	enum { SupportRawCopy = true };      \
};

// integral types
NL_TRIVIAL_TYPE_TRAITS(bool);
#ifdef NL_COMP_VC6
NL_TRIVIAL_TYPE_TRAITS(sint8);
NL_TRIVIAL_TYPE_TRAITS(uint8);
#endif // NL_COMP_VC6
NL_TRIVIAL_TYPE_TRAITS(sint16);
NL_TRIVIAL_TYPE_TRAITS(uint16);
NL_TRIVIAL_TYPE_TRAITS(sint32);
NL_TRIVIAL_TYPE_TRAITS(uint32);
NL_TRIVIAL_TYPE_TRAITS(sint64);
NL_TRIVIAL_TYPE_TRAITS(uint64);
#ifdef NL_COMP_VC6
NL_TRIVIAL_TYPE_TRAITS(sint);
NL_TRIVIAL_TYPE_TRAITS(uint);
#endif // NL_COMP_VC6

// characters
NL_TRIVIAL_TYPE_TRAITS(char);
NL_TRIVIAL_TYPE_TRAITS(unsigned char);

// numeric types
NL_TRIVIAL_TYPE_TRAITS(float);
NL_TRIVIAL_TYPE_TRAITS(double);

// misc
NL_TRIVIAL_TYPE_TRAITS(CVector);
NL_TRIVIAL_TYPE_TRAITS(CRGBA);

//.. to be completed

} // NLMISC

#endif


