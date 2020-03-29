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
#error DEPRECATED

#ifndef EGS_SHUTDOWN_HANDLER_H
#define EGS_SHUTDOWN_HANDLER_H

#include "nel/misc/variable.h"
#include "nel/misc/time_nl.h"

#include "nel/misc/ucstring.h"


/**
 * This singleton handles shutdown events
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CShutdownHandler
{
public:

	/// \name Basic Management
	// @{

	/**
	 * Inits Handler
	 */
	static void		init();

	/**
	 * Update Handler
	 */
	static void		update();

	/**
	 * Release Handler
	 */
	static void		release();

	// @}




	/// \name Handler Commands Declaration
	// @{

	/**
	 * Start Shutdown Counter
	 */
	static void		startShutdown(sint shutdownCounter = -1, sint broadcastMessageRate = -1);

	/**
	 * Cancel Shutdown
	 */
	static void		cancelShutdown();

	/**
	 * Restart shard
	 * Actually reset WS ShardOpen variable to OpenForAll
	 */
	static void		restartShard();

	// @}





	/// \name Info queries
	// @{

	/**
	 * Get current shard state
	 */
	static std::string	getState();

	// @}


private:

	enum TState
	{
		Running = 0,
		ShuttingDown,
		Closed,
	};

	/// Shutdown State
	static TState			_State;

	/// Shutdown Timeout
	static NLMISC::TTime	_ShutdownTimeout;

	/// Shutdown Timeout
	static NLMISC::TTime	_NextBroadcastMessage;

	/// ShardOpen has been closed
	static bool				_ShardClosed;

	/// Broadcast Message Rate
	static uint				_BroadcastMessageRate;

	/// Broadcast Shutdown message
	static void				broadcastShutdownMessage();

	/// Broadcast message
	static void				broadcastMessage(const ucstring& message);

	/// Disconnect all players
	static void				disconnectPlayers();
};


typedef uint32 TSecTime; // unit of NLMISC::CTime::getSecondsSince1970()

/**
 * Handle daily automatic shutdown
 */
class CAutomaticShutdownHandler
{
public:

	/// Update Handler
	static void					update();

	/// Uses daily shutdown sequence time to compute next time
	static void					computePlannedShutdownTimes( NLMISC::CLog *log=NLMISC::InfoLog );


private:
	/// Game cycle of last config file check
	static NLMISC::TGameCycle	_LastGCChecked;

	/// Time of next planned shutdown sequence start
	static TSecTime				_NextPlannedShutdownStartTime;

	/// Time of next planned shutdown sequence end
	static TSecTime				_NextPlannedShutdownEndTime;
};


#endif // EGS_SHUTDOWN_HANDLER_H

/* End of shutdown_handler.h */
