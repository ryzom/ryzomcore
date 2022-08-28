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




#include "ags_timer.h"
#include "game_share/tick_event_handler.h"

namespace AGS_TEST
{
	CAGSTimer::CAGSTimer(uint32 dt /*= 0*/)
	{
		_dt = dt;
	}

	void CAGSTimer::set(uint32 dt)
	{
		_start = (uint32)CTickEventHandler::getGameCycle();
		_dt = dt;
	}

	void CAGSTimer::add(uint32 dt)
	{
		_start = (uint32)CTickEventHandler::getGameCycle();
		_dt += dt;
	}

	bool CAGSTimer::test()
	{
		uint32 curent = (uint32) CTickEventHandler::getGameCycle();

		uint32 elapsed = curent - _start;
			
		return ( elapsed >= _dt );
	}

}
