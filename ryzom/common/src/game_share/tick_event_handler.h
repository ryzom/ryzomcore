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



#ifndef RY_TICK_EVENT_HANDLER_H
#define RY_TICK_EVENT_HANDLER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/net/naming_client.h"


typedef void (* TUserSyncCallback) ();


/**
 * Implements callbacks relative to the tick service
 * \author Nicolas Brigand
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CTickEventHandler
{
public :

	/**
	 * Get the current universal game time
	 */
	static const NLMISC::TGameTime &getGameTime() { return _GameTime; }

	/**
	 * Set the current universal game time
	 */
	static void setGameTime( NLMISC::TGameTime gameTime) { _GameTime = gameTime; }

	/**
	 * Get the current step time of the tick service
	 */
	static const NLMISC::TGameTime &getGameTimeStep() { return _GameTimeStep; }

	/**
	 * Set the current game time step
	 */
	static void setGameTimeStep( NLMISC::TGameTime gameTimeStep) { _GameTimeStep = gameTimeStep; }

	/**
	 * Get the number of ellapsed game cycles
	 */
	inline	static const NLMISC::TGameCycle &getGameCycle() { return _GameCycle;	}

	/**
	 * Get the number of ellapsed game cycles
	 */
	static void setGameCycle( NLMISC::TGameCycle gameCycle ) { _GameCycle = gameCycle; }

	/**
	 * Send a tock and update the game time
	 */
	static void tickUpdate( NLNET::TServiceId serviceId );

	static void	sendTockBack( NLNET::TServiceId serviceId );

	/**
	 * initialize the callback Array and get a pointer to the service's specific update function
	 *
	 * \param updateFunc will be call when we receive a new tick
	 * \param syncFunc will be call when the ticks send the syncro
	 * \param tockAtBeginOfTickUpdate indicates if we will send TOCK before calling updateFunc()
	 * instead of after.
	 */
	static void init(void (*updateFunc)(), void (*syncFunc)() = NULL, bool tockAtBeginOfTickUpdate=false);

	/**
	 * Set a callback to call when receiving the first game cycle (call this method in your init())
	 * \param syncFunc will be call when the ticks send the syncro
	 * \param allowReplaceCallback true if we allow the callback to be replaced
	 * \return previous callback if exist, otherwise NULL
	 */
	static TUserSyncCallback setSyncCallback( TUserSyncCallback syncFunc, bool allowReplaceCallback = false );

	static bool getTockAtBeginOfTickUpdate() { return _TockAtBeginOfTickUpdate; }

private :

	/// Time according to the game (used for determining day, night...) (double in seconds)
	static NLMISC::TGameTime _GameTime;

	/// Game time step
	static NLMISC::TGameTime _GameTimeStep;

	/// Number of game cycles
	static NLMISC::TGameCycle _GameCycle;

	/// Shall we tock at the beginning of at the end of tickUpdate() ?
	static bool _TockAtBeginOfTickUpdate;

	/**
	 * private constructor (singleton)
	 */
	CTickEventHandler() { }
};


#endif// RY_TICK_EVENT_HANDLER_H


