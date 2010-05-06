/** \file actor_manager.cpp
 * 
 * the actor manager for AgS_Test
 *
 * $Id: ags_timer.cpp,v 1.3 2004/03/01 19:18:37 lecroart Exp $
 * $Log: ags_timer.cpp,v $
 * Revision 1.3  2004/03/01 19:18:37  lecroart
 * REMOVED: bad headers
 *
 * Revision 1.2  2003/01/28 20:36:51  miller
 * Changed header comments from 'Nel Network Services' to Ryzom
 *
 * Revision 1.1  2002/11/28 14:57:17  portier
 * #ADDED: ia_player.id
 *
 * Revision 1.21  2002/11/15 16:22:42  fleury
 * CHANGED : the OPS has been replaced by the EGS
 *
 * Revision 1.20  2002/08/30 08:47:34  miller
 * another quick test (non destructive)
 *
 * Revision 1.19  2002/08/30 08:46:09  miller
 * quick test (non destructive)
 *
 */




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
