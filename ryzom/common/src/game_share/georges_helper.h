// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef CL_GEORGES_HELPER_H
#define CL_GEORGES_HELPER_H

#include "nel/misc/types_nl.h"

namespace NLGEORGES
{
	class UFormElm;
}

namespace NLMISC
{
	class CVector;
	class CRGBA;
}

// an utility struct that convert georges item / item elt to types
struct CGeorgesHelper
{
	/** Convert an item to a vector.
	  * \return true it the conversion succeed.
	  */
	static bool convert(NLMISC::CVector &dest, const NLGEORGES::UFormElm &src);
	/** Convert an item to a color
	  * \return true it the conversion succeed.
	  */
	static bool convert(NLMISC::CRGBA &dest, const NLGEORGES::UFormElm &src);


	/** Convert a child item to a vector.
	  * \return true it the conversion succeed. dest is undefined if it failed
	  */
	static bool getValueByName(NLMISC::CVector &dest, const NLGEORGES::UFormElm &item, const char *name);
	/** Convert a child item to a color
	  * \return true it the conversion succeed. dest is undefined if it failed
	  */
	static bool getValueByName(NLMISC::CRGBA &dest, const NLGEORGES::UFormElm &item, const char *name);

};

#endif

