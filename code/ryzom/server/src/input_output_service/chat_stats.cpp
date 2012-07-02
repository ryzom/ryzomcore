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
#include "chat_stats.h"
#include "nel/misc/debug.h"

using namespace std;


//-----------------------------------------------
//	addOccurence :
//
//-----------------------------------------------
void CChatStats::addOccurence( const string& str )
{
	map<string,uint32>::iterator itOcc = _Occurences.find( str );
	if( itOcc != _Occurences.end() )
	{
		(*itOcc).second++;
	}
	else
	{
		nlwarning("<CChatStats::addOccurence> The string %s is unknown",str.c_str());
	}

} // addOccurence //

