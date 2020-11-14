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

#include "fe_stat.h"

// Fix the stupid Visual 6 Warning
void foo_fe_stat_cpp() {};

#ifdef MEASURE_FRONTEND_TABLES

#include <nel/misc/debug.h>
#include "game_share/tick_event_handler.h"
//#include <sstream>

using namespace std;
using namespace NLMISC;


/*
 * setGameTick
 */
void		TEventPerSeenEntityCounterFrame::setGameTick()
{
	Tick = CTickEventHandler::getGameCycle();
}


/*
 * display
 */
void		TEventPerSeenEntityCounterFrame::display( bool withvalue )
{
	//std::stringstream ss;
	string str;
	//ss << (int)Tick;
	str += NLMISC::toString(Tick);
	sint i;
	for ( i=0; i!=250; ++i )
	{
		//ss << "\t" << SeenEntities[i];
		str += "\t" + NLMISC::toString(SeenEntities[i]);
	}
	if( withvalue )
	{
		//ss << "\t" << Value;
		str += "\t" + NLMISC::toString(Value);
	}
	InfoLog->displayRawNL( "%s", str.c_str() );
}


/*
 * displayAll (static)
 */
void		TEventPerSeenEntityCounterFrame::displayAll( const char *name, std::vector<TEventPerSeenEntityCounterFrame>& vect, bool withvalue )
{
	InfoLog->displayRawNL( "%s:", name );
	std::vector<TEventPerSeenEntityCounterFrame>::iterator icv;
	for ( icv=vect.begin(); icv!=vect.end(); ++icv )
	{
		(*icv).display( withvalue );
	}
	vect.clear();
}

TEventPerSeenEntityCounterFrame					PropRecvrCntFrame1, PropRecvrCntFrame2,
												DistCntFrame,
												DeltaCntFrame,
												PrioCntFrame,
												PosSentCntFrame;

std::vector<TEventPerSeenEntityCounterFrame>	PropRecvrCntClt1,
												DistCntClt1,
												DeltaCntClt1,
												PrioCntClt1,
												PosSentCntClt1;


#endif
