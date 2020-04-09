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

#include "mp_category.h"

using namespace std;

namespace MP_CATEGORY
{
	static map< string, TMPCategory > StringToMPCategory;
	static map< TMPCategory, string > MPCategoryToString;

	static bool mapInitialized = false;
	const static string undefined = "Undefined";

	//-----------------------------------------------
	// initMap :
	//-----------------------------------------------
	void initMap()
	{
		mapInitialized = true;

		StringToMPCategory.insert( make_pair( string("Undefined"), Undefined) );
		MPCategoryToString.insert( make_pair( Undefined, string("Undefined") ) );

		StringToMPCategory.insert( make_pair( string("Exotic"), Exotic) );
		MPCategoryToString.insert( make_pair( Exotic, string("Exotic") ) );

		StringToMPCategory.insert( make_pair( string("Faber"), Faber) );
		MPCategoryToString.insert( make_pair( Faber, string("Faber") ) );

		StringToMPCategory.insert( make_pair( string("Upgrade"), Upgrade) );
		MPCategoryToString.insert( make_pair( Upgrade, string("Upgrade") ) );
	} // initMap //


	//-----------------------------------------------
	// stringToMPCategory :
	//-----------------------------------------------
	TMPCategory stringToMPCategory(const string &str)
	{
		if ( !mapInitialized)
			initMap();

		map< string, TMPCategory >::const_iterator it = StringToMPCategory.find( str );
		if (it != StringToMPCategory.end() )
		{
			return (*it).second;
		}
		else
		{
			return Undefined;
		}
	} // stringToMPCategory //


	//-----------------------------------------------
	// mpCategoryToString :
	//-----------------------------------------------
	const string & mpCategoryToString (TMPCategory type)
	{
		if ( !mapInitialized)
			initMap();

		map< TMPCategory, string >::const_iterator it = MPCategoryToString.find( type );
		if (it != MPCategoryToString.end() )
		{
			return (*it).second;
		}
		else
		{
			return undefined;
		}
	} // mpCategoryToString  //
} // MP_CATEGORY
