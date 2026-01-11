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

#error DERPRECATED

#include "stdpch.h"

#include "shutdown_handler.h"

#include "nel/net/service.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"

#include "player_manager/player_manager.h"
#include "player_manager/player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


// Shutdown State
CShutdownHandler::TState	CShutdownHandler::_State = CShutdownHandler::Running;

// Shutdown Timeout
NLMISC::TTime				CShutdownHandler::_ShutdownTimeout = 0;

// Shutdown Timeout
NLMISC::TTime				CShutdownHandler::_NextBroadcastMessage;

// ShardOpen has been closed
bool						CShutdownHandler::_ShardClosed = false;

// Broadcast Message Rate
uint						CShutdownHandler::_BroadcastMessageRate = 0;

// Game cycle of last config file check
NLMISC::TGameCycle			CAutomaticShutdownHandler::_LastGCChecked = 0;

static const TSecTime MaxTime = ~0; //std::numeric_limits<TSecTime>::max()

// Time of next planned shutdown sequence start
TSecTime					CAutomaticShutdownHandler::_NextPlannedShutdownStartTime = MaxTime;

// Time of next planned shutdown sequence end
TSecTime					CAutomaticShutdownHandler::_NextPlannedShutdownEndTime = MaxTime;


/**
 * Shutdown Counter, in minutes
 */
CVariable<uint>		ShutdownCounter("egs", "ShutdownCounter", "Time to shutdown in minutes", 5, 0, true);

/**
 * Broadcast shutdown message rate in seconds
 */
CVariable<uint>		BroadcastShutdownMessageRate("egs", "BroadcastShutdownMessageRate", "Number of seconds between 2 shutdown message in seconds", 30, 0, true);

/**
 * Close shard Access at
 */
CVariable<uint>		CloseShardAccessAt("egs", "CloseShardAccessAt", "Time to shutdown to close shard access, in seconds", 60, 0, true);


// Callback for DailyShutdownSequenceTime
void cbChangeDailyShutdownSequenceTime( IVariable& var )
{
	CAutomaticShutdownHandler::computePlannedShutdownTimes();
}

/**
 * DailyShutdownSequenceTime
 */
CVariable<string>	DailyShutdownSequenceTime("egs","DailyShutdownSequenceTime", "Time of day when the service will start a shutdown sequence (ex: \"20:55\"). Set \"\" or -1 to disable)", string(), 0, true, cbChangeDailyShutdownSequenceTime, true );

/**
 * Daily Shutdown Counter, in minutes
 */
CVariable<uint>		DailyShutdownCounterMinutes("egs", "DailyShutdownCounterMinutes", "Time to shutdown in minutes", 1, 0, true);

/**
 * DailyShutdownBroadcastMessage
 */
CVariable<string>	DailyShutdownBroadcastMessage("egs","DailyShutdownBroadcastMessage", "Message to broadcast before daily shutdown", string("The shard will be shut down in 1 minute"), 0, true );

/**
 * CheckShutdownPeriodGC
 */
CVariable<uint>		CheckShutdownPeriodGC("egs","CheckShutdownPeriodGC", "Automatic shutdown sequence is tested every CheckShutdownPeriodGC game cycles", 50, 0, true );


/*
 * Inits Handler
 */
void	CShutdownHandler::init()
{
	_State = Running;
	_ShutdownTimeout = NLMISC::CTime::getLocalTime();
	_NextBroadcastMessage = NLMISC::CTime::getLocalTime();
	_ShardClosed = false;
}

/*
 * Update Handler
 */
void	CShutdownHandler::update()
{
	if (_State == ShuttingDown)
	{
		NLMISC::TTime	now = NLMISC::CTime::getLocalTime();

		// time to shutdown?
		if (_ShutdownTimeout <= now)
		{
			nlinfo("CShutdownHandler::update(): disconnect all players from shard");

			disconnectPlayers();

			_State = Closed;
			return;
		}

		// time to close shard access?
		if (_ShutdownTimeout-(CloseShardAccessAt*60*1000) <= now && !_ShardClosed)
		{
			nlinfo("CShutdownHandler::update(): close access to shard, SET_SHARD_OPEN sent to WS");

			// send WS setShardOpen message
			CMessage	msgShardOpen("SET_SHARD_OPEN");
			uint8		close = 0;
			msgShardOpen.serial(close);
			CUnifiedNetwork::getInstance()->send("WS", msgShardOpen);

			_ShardClosed = true;
		}

		// time to broadcast message?
		if (_NextBroadcastMessage <= now)
		{
			broadcastShutdownMessage();
		}
	}
}

/*
 * Release Handler
 */
void	CShutdownHandler::release()
{
}





/*
 * Start Shutdown Counter
 */
void	CShutdownHandler::startShutdown(sint shutdownCounter, sint broadcastMessageRate)
{
	nlinfo("CShutdownHandler::startShutdown(): starting count down to shutdown");

	if (_State != Running)
	{
		nlinfo("CShutdownHandler::startShutdown(): shutdown already started, left as is");
		return;
	}

	NLMISC::TTime	now = NLMISC::CTime::getLocalTime();

	nlinfo("CShutdownHandler::startShutdown(): counter set to %u seconds", ShutdownCounter.get());

	// time in ms
	_State = ShuttingDown;
	_ShutdownTimeout = now + (shutdownCounter > 0 ? shutdownCounter : ShutdownCounter)*60*1000;
	_BroadcastMessageRate = (broadcastMessageRate > 0 ? broadcastMessageRate : BroadcastShutdownMessageRate);
	_NextBroadcastMessage = now;
	_ShardClosed = false;
}

/*
 * Cancel Shutdown
 */
void	CShutdownHandler::cancelShutdown()
{
	nlinfo("CShutdownHandler::cancelShutdown(): cancelling shard shutdown");

	if (_State != ShuttingDown)
	{
		nlinfo("CShutdownHandler::cancelShutdown(): shard is not currently shutting down, shard left as is");
		return;
	}

	_State = Running;

	broadcastMessage(std::string("Shutting down cancelled"));

	if (_ShardClosed)
	{
		nlinfo("CShutdownHandler::cancelShutdown(): WS ShardOpen state modified, sending restore request");
		CMessage	msgShardOpen("RESTORE_SHARD_OPEN");
		CUnifiedNetwork::getInstance()->send("WS", msgShardOpen);
	}
}

/*
 * Restart shard
 * Actually reset WS ShardOpen variable to OpenForAll
 */
void	CShutdownHandler::restartShard()
{
	nlinfo("CShutdownHandler::restartShard(): restarting shard after shutdown");

	if (_State != Closed)
	{
		nlinfo("CShutdownHandler::restartShard(): shard is not closed");
		return;
	}

	_State = Running;

	broadcastMessage(std::string("Shard is now restarted and open to public."));

	// send SET_SHARD_OPEN to WS
	nlinfo("CShutdownHandler::restartShard(): WS ShardOpen state modified, sending restore request");
	CMessage	msgShardOpen("RESTORE_SHARD_OPEN");
	CUnifiedNetwork::getInstance()->send("WS", msgShardOpen);
}





/*
 * Get current shard state
 */
std::string	CShutdownHandler::getState()
{
	if (_State == Running)
	{
		return "Running";
	}
	else if (_State == ShuttingDown)
	{
		sint	shutdown = (sint)((_ShutdownTimeout-NLMISC::CTime::getLocalTime()) / 1000);

		if (_ShardClosed)
		{
			return NLMISC::toString("Shutdown in %d seconds, shard access closed", shutdown);
		}
		else
		{
			return NLMISC::toString("Shutdown in %d seconds", shutdown);
		}
	}
	else
	{
		return "Closed, ready to restart";
	}
}


/*
 * Broadcast message
 */
void	CShutdownHandler::broadcastMessage(const ucstring& message)
{
	_NextBroadcastMessage = NLMISC::CTime::getLocalTime() + _BroadcastMessageRate*1000;

	/// \todo handle ucstring somewhere here...
	PlayerManager.broadcastMessage(1, 0, 0, message.toString());
}

/*
 * Broadcast Shutdown message
 */
void	CShutdownHandler::broadcastShutdownMessage()
{
	nlinfo("CShutdownHandler::broadcastShutdownMessage(): broadcasting shutdown message");

	std::string		shutdownMessage;
	const sint		timeAccuracy = 10;

	sint	shutdown = (sint)((_ShutdownTimeout-NLMISC::CTime::getLocalTime()) / 1000);

	shutdown += (timeAccuracy/2);

	// clamp to 0
	if (shutdown < 0)
		shutdown = 0;

	shutdown = shutdown - (shutdown%timeAccuracy);

	if (shutdown < timeAccuracy)
	{
		broadcastMessage(std::string("Shutting down..."));
	}
	else
	{
		if (shutdown < 60)
		{
			broadcastMessage(NLMISC::toString("Shard shuts down in %d seconds", shutdown));
		}
		else
		{
			uint	min = shutdown/60;
			uint	sec = shutdown%60;
			broadcastMessage(NLMISC::toString("Shard shuts down in %dmn %ds", min, sec));
		}
	}
}


/*
 * Disconnect all players
 */
void	CShutdownHandler::disconnectPlayers()
{
	nlinfo("CShutdownHandler::disconnectPlayers(): disconnecting all players");

	CMessage	msgout("DISCONNECT_ALL_CLIENTS");
	CUnifiedNetwork::getInstance()->send("FS", msgout);

/*
	const CPlayerManager::TMapPlayers&	playerMap = PlayerManager.getPlayers();

	// browse through all players to disconnect them
	CPlayerManager::TMapPlayers::const_iterator	it;
	for (it = playerMap.begin(); it != playerMap.end(); )
	{
		const CPlayerManager::SCPlayer&	player = (*it).second;
		++it;

		if (player.Player != NULL)
		{
			uint32	userId = player.Player->getUserId();
			PlayerManager.savePlayer(userId);
			PlayerManager.disconnectPlayer(userId);
		}
	}
*/
}


/*
 * Update.
 * If the shutdown sequence is started, it can be canceled by changing DailyShutdownSequenceTime.
 */
void CAutomaticShutdownHandler::update()
{
	if ( CTickEventHandler::getGameCycle() - _LastGCChecked >= CheckShutdownPeriodGC )
	{
		_LastGCChecked = CTickEventHandler::getGameCycle();
		TSecTime nowSec = NLMISC::CTime::getSecondsSince1970();
		
		// Time to start an automatic shutdown sequence?
		if ( nowSec >= _NextPlannedShutdownStartTime )
		{
			string msg = DailyShutdownBroadcastMessage.get();
			PlayerManager.broadcastMessage( 1, 0, 0, msg );
			nlinfo( msg.c_str() );
			_NextPlannedShutdownEndTime = _NextPlannedShutdownStartTime + (DailyShutdownCounterMinutes.get()*60);
			_NextPlannedShutdownStartTime = MaxTime;
		}
		// Time to shutdown?
		else if ( nowSec >= _NextPlannedShutdownEndTime )
		{
			_NextPlannedShutdownEndTime = MaxTime;
			IService::getInstance()->exit();
		}
	}
}


/*
 * Use daily shutdown sequence time to compute next time
 */
void	CAutomaticShutdownHandler::computePlannedShutdownTimes( NLMISC::CLog *log )
{
	string dailyTimeStr = DailyShutdownSequenceTime.get();
	if ( dailyTimeStr.empty() || (dailyTimeStr == "-1") )
	{
		// No automatic shutdown sequence
		_NextPlannedShutdownStartTime = MaxTime;
		_NextPlannedShutdownEndTime = MaxTime;
		if ( log )
			log->displayNL( "Automatic shutdown sequence disabled" );
	}
	else
	{
		// Setup next automatic shutdown sequence
		time_t currentTime = time( NULL );
		struct tm *localTime = localtime( &currentTime );
		struct tm shutdownTime = *localTime;
		string::size_type cp = dailyTimeStr.find( ':' );
		shutdownTime.tm_hour = atoi( dailyTimeStr.substr( 0, cp ).c_str() );
		shutdownTime.tm_min = 0;
		if ( cp != string::npos )
			shutdownTime.tm_min = atoi( dailyTimeStr.substr( cp+1 ).c_str() );
		shutdownTime.tm_sec = 0;
		shutdownTime.tm_isdst = -1;
		_NextPlannedShutdownStartTime = nl_mktime( &shutdownTime );
		char *dayWhenStr;
		if ( _NextPlannedShutdownStartTime > (TSecTime)currentTime )
		{
			dayWhenStr = "today";
		}
		else
		{
			dayWhenStr = "tomorrow";
			++shutdownTime.tm_mday;
			_NextPlannedShutdownStartTime = nl_mktime( &shutdownTime );
		}
		_NextPlannedShutdownEndTime = _NextPlannedShutdownStartTime + (DailyShutdownCounterMinutes.get()*60);
		if ( log )
			log->displayNL( "Next automatic shutdown sequence will begin %s at %02u:%02u with %u-minute delay", dayWhenStr, shutdownTime.tm_hour, shutdownTime.tm_min, DailyShutdownCounterMinutes.get() );
	}
}


NLMISC_DYNVARIABLE(std::string, ShutdownState, "Current shutdown state of the shard, as a string (Read only)")
{
	// read or write the variable
	if (get)
		*pointer = CShutdownHandler::getState();
}


// Commands

NLMISC_COMMAND(startShutdown, "Ask the EGS to shutdown the whole shard", "[time to shutdown, minutes] [time between 2 messages, seconds]")
{
	if (args.size() > 2)
		return false;

	sint	counter = -1;
	sint	msgRate = -1;

	if (args.size() >= 1)
		counter = atoi(args[0].c_str());

	if (args.size() >= 2)
		msgRate = atoi(args[1].c_str());

	CShutdownHandler::startShutdown(counter, msgRate);
	return true;
}

NLMISC_COMMAND(cancelShutdown, "Cancel current shutdown in progress", "")
{
	CShutdownHandler::cancelShutdown();
	return true;
}

NLMISC_COMMAND(restartShard, "Restart the shard after a shutdown", "")
{
	CShutdownHandler::restartShard();
	return true;
}

