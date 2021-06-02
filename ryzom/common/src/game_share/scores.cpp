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

#include "nel/misc/string_conversion.h"
#include "scores.h"

using namespace std;
using namespace NLMISC;

namespace SCORES
{

	// The conversion table
	const CStringConversion<TScores>::CPair stringTable [] =
	{
		{ "HitPoints", hit_points },
		{ "Stamina", stamina },
		{ "Sap", sap },
		{ "Focus", focus },
	};

	CStringConversion<TScores> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  unknown);


	TScores toScore( const std::string &str )
	{
		return conversion.fromString(str);
	}

	const std::string& toString( TScores s )
	{
		return conversion.toString(s);
	}

	const std::string& toString( uint s )
	{
		return conversion.toString((TScores)s);
	}
}; // SCORES
