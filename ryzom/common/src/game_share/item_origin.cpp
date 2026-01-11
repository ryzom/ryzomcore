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
#include "nel/misc/common.h"
#include "item_origin.h"

using namespace std;
using namespace NLMISC;

namespace ITEM_ORIGIN
{

// ***************************************************************************

/* Warning : if you add an enum here, you MUST add it in translation/work/en.uxt prefixed with "io" like this :
ioUnknown			[Unknown]
ioCommon			[Common]
*/
static const string StringArray[NUM_ITEM_ORIGIN+1]=
{
	"Common",
	"Fyros",
	"Matis",
	"Tryker",
	"Zorai",
	"Refugee",
	"Tribe",
	"Kami",
	"Karavan",
	"Unknown",
};

// ***************************************************************************
EItemOrigin stringToEnum(const std::string &str)
{
	for(uint i=0;i<NUM_ITEM_ORIGIN;i++)
	{
		if(nlstricmp(StringArray[i], str)==0)
			return (EItemOrigin)i;
	}
	return UNKNOWN;
}

// ***************************************************************************
const std::string & enumToString (EItemOrigin e)
{
	nlassert((sint)e<=NUM_ITEM_ORIGIN);
	return StringArray[e];
}

// ***************************************************************************
EGSPD::CPeople::TPeople itemOriginStringToPeopleEnum( const std::string &str )
{
	switch( ITEM_ORIGIN::stringToEnum( str ) )
	{
		case COMMON:
			return EGSPD::CPeople::Common;
		case FYROS:
			return EGSPD::CPeople::Fyros;
		case MATIS:
			return EGSPD::CPeople::Matis;
		case TRYKER:
			return EGSPD::CPeople::Tryker;
		case ZORAI:
			return EGSPD::CPeople::Zorai;
		case REFUGEE:
			return EGSPD::CPeople::Common;
		case TRIBE:
			return EGSPD::CPeople::Tribe;
		case KAMI:
			return EGSPD::CPeople::Kami;
		case KARAVAN:
			return EGSPD::CPeople::Karavan;
		default:
			return EGSPD::CPeople::EndPeople;
	}
}

}
