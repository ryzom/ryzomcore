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
#include "creature_size.h"

using namespace std;

namespace CREATURE_SIZE
{
	static map< string, ECreatureSize > StringToSizeMap;
	static map< ECreatureSize, string > SizeToStringMap;	

	static bool mapInitialized = false;
	const static string unknown_str = "UNKNOWN";


	//-----------------------------------------------
	// initMap :
	//-----------------------------------------------
	void initMap()
	{
		mapInitialized = true;

		StringToSizeMap.insert( make_pair( string("SMALL"), SMALL) );
		SizeToStringMap.insert( make_pair( SMALL, string("SMALL") ) );

		StringToSizeMap.insert( make_pair( string("HOMIN"), HOMIN) );
		SizeToStringMap.insert( make_pair( HOMIN, string("HOMIN") ) );

		StringToSizeMap.insert( make_pair( string("BIG"), BIG) );
		SizeToStringMap.insert( make_pair( BIG, string("BIG") ) );

	} // initMap //

	//-----------------------------------------------
	// stringToCreatureSize :
	//-----------------------------------------------
	ECreatureSize stringToCreatureSize(const string &str)
	{
		if ( !mapInitialized)
			initMap();

		map< string, ECreatureSize >::const_iterator it = StringToSizeMap.find( str );
		if (it != StringToSizeMap.end() )
		{
			return (*it).second;
		}
		else
		{
			return UNKNOWN;
		}
	} // stringToCreatureSize //


	//-----------------------------------------------
	// creatureSizeToString :
	//-----------------------------------------------
	const string &creatureSizeToString(ECreatureSize size)
	{
		if ( !mapInitialized)
			initMap();

		map< ECreatureSize, string >::const_iterator it = SizeToStringMap.find( size );
		if (it != SizeToStringMap.end() )
		{
			return (*it).second;
		}
		else
		{
			return unknown_str;
		}
	} // creatureSizeToString //

}; // CREATURE_SIZE
