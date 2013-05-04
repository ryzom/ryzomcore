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
#include "stable.h"

// static members of CStable
CStable * CStable::_Instance = NULL;

using namespace std;
using namespace NLMISC;
using namespace NLNET;

NL_INSTANCE_COUNTER_IMPL(CStable);

//----------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------
CStable::CStable()
{
}


//----------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------
CStable::~CStable()
{
	_Stables.clear();
	delete _Instance;
	_Instance = NULL;
}

//----------------------------------------------------------------------------
// add a stable
//----------------------------------------------------------------------------
void CStable::addStable( const std::string& stableName, uint16 placeId, const std::string& continent, float x, float y, float z, float theta )
{
	TStableData sd;

	sd.StableName = stableName;
	sd.Continent = CONTINENT::toContinent( continent );
	sd.StableExitX = sint32(x * 1000);
	sd.StableExitY = sint32(y * 1000);
	sd.StableExitZ = sint32(z * 1000);
	sd.Theta = theta;

	if( ! isStable( placeId ) )
	{
		_Stables.insert( make_pair(placeId, sd) );
	}
	else
	{
		nlwarning("<CStable::addStable> Stable name %s (PlaceId %u) already exist !!!", stableName.c_str(), placeId );
	}
}

//----------------------------------------------------------------------------
// return stable entry point coordinate
//----------------------------------------------------------------------------
bool CStable::getStableData( uint16 placeId, TStableData& stableData ) const
{
	TStableContainer::const_iterator it = _Stables.find( placeId );
	if( it != _Stables.end() )
	{
		stableData = (*it).second;
		return true;
	}
	return false;
}


