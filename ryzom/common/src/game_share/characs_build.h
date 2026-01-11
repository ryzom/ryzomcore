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



#ifndef CL_CHARAC_BUILD
#define CL_CHARAC_BUILD

#include "characteristics.h"

namespace NLGEORGES
{
	class UFormElm;
}


/** Tool fct : this read a set of characteristics from a sheet, and put the result in an array.
  * The characs can be asolute, or delta values.
  */
void loadCharacteristicsFromSheet(const NLGEORGES::UFormElm &rootNode, std::string prefix, sint8 dest[CHARACTERISTICS::NUM_CHARACTERISTICS]);



#endif

