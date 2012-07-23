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



#include "stdpch.h"

#include "nel/misc/debug.h"
#include "continent.h"
// nel
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;

namespace CONTINENT
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TContinent)
		NL_STRING_CONVERSION_TABLE_ENTRY(FYROS)
		NL_STRING_CONVERSION_TABLE_ENTRY(ZORAI)
		NL_STRING_CONVERSION_TABLE_ENTRY(TRYKER)
		NL_STRING_CONVERSION_TABLE_ENTRY(MATIS)
		NL_STRING_CONVERSION_TABLE_ENTRY(BAGNE)
		NL_STRING_CONVERSION_TABLE_ENTRY(NEXUS)
		NL_STRING_CONVERSION_TABLE_ENTRY(ROUTE_GOUFFRE)
		NL_STRING_CONVERSION_TABLE_ENTRY(SOURCES)
		NL_STRING_CONVERSION_TABLE_ENTRY(TERRE)
		NL_STRING_CONVERSION_TABLE_ENTRY(FYROS_ISLAND)
		NL_STRING_CONVERSION_TABLE_ENTRY(FYROS_NEWBIE)
		NL_STRING_CONVERSION_TABLE_ENTRY(TRYKER_ISLAND)
		NL_STRING_CONVERSION_TABLE_ENTRY(TRYKER_NEWBIE)
		NL_STRING_CONVERSION_TABLE_ENTRY(ZORAI_NEWBIE)
		NL_STRING_CONVERSION_TABLE_ENTRY(MATIS_NEWBIE)
		NL_STRING_CONVERSION_TABLE_ENTRY(ZORAI_ISLAND)
		NL_STRING_CONVERSION_TABLE_ENTRY(MATIS_ISLAND)
		NL_STRING_CONVERSION_TABLE_ENTRY(TESTROOM)
		NL_STRING_CONVERSION_TABLE_ENTRY(INDOORS)
		NL_STRING_CONVERSION_TABLE_ENTRY(NEWBIELAND)
		NL_STRING_CONVERSION_TABLE_ENTRY(R2_ROOTS)
		NL_STRING_CONVERSION_TABLE_ENTRY(R2_DESERT)
		NL_STRING_CONVERSION_TABLE_ENTRY(R2_LAKES)
		NL_STRING_CONVERSION_TABLE_ENTRY(R2_FOREST)
		NL_STRING_CONVERSION_TABLE_ENTRY(R2_JUNGLE)
		NL_STRING_CONVERSION_TABLE_ENTRY(CORRUPTED_MOOR)
		NL_STRING_CONVERSION_TABLE_ENTRY(KITINIERE)
		NL_STRING_CONVERSION_TABLE_ENTRY(UNKNOWN)
	NL_END_STRING_CONVERSION_TABLE(TContinent, ContinentConversion, UNKNOWN)



	//-----------------------------------------------
	// toContinent:
	//-----------------------------------------------
	TContinent toContinent(const std::string &str)
	{
		return ContinentConversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TContinent continent)
	{
		return ContinentConversion.toString(continent);
	}
}; // CONTINENT
