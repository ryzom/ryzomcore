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

// nel
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"

#include "starting_point.h"

using namespace std;
using namespace NLMISC;

namespace RYZOM_STARTING_POINT
{
	// The conversion table
	const CStringConversion<TStartPoint>::CPair stringTable [] =
	{
		{ "stalli",		stalli },
		{ "borea",		borea },
		{ "nistia",		nistia },
		{ "rosilio",	rosilio },
		{ "miani",		miani },

		{ "qai_lo",		qai_lo },
		{ "sheng_wo",	sheng_wo },
		{ "men_xing",	men_xing },
		{ "koi_zun",	koi_zun },
		{ "yin_piang",	yin_piang },

		{ "aegus",		aegus },
		{ "kaemon",		kaemon },
		{ "sekovix",	sekovix },
		{ "phyxon",		phyxon },
		{ "galemus",	galemus },

		{ "aubermouth",	aubermouth },
		{ "barkdell",	barkdell },
		{ "hobwelly",	hobwelly },
		{ "waverton",	waverton },
		{ "dingleton",	dingleton },

		{ "starting_city",	starting_city },

	};

	CStringConversion<TStartPoint> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);

	// convert type id to type name string
	const std::string& toString( TStartPoint start_point )
	{
		return conversion.toString(start_point);
	}

	// convert type name to type enum value
	TStartPoint toStartPoint( const std::string& str )
	{
		return conversion.fromString(str);
	}
};


