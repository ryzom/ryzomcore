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

// net
#include "nel/net/unified_network.h"
#include "nel/net/service.h"

#include "nel/misc/command.h"
#include "nel/misc/hierarchical_timer.h"

#include "tick_event_handler.h"
#include "time_weather_season/time_and_season.h"
#include "tick_proxy_time_measure.h"
#include "timer.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


// game time
TGameTime CTickEventHandler::_GameTime = 0;
// game time step
TGameTime CTickEventHandler::_GameTimeStep = 0.1;
// game cycle
TGameCycle	CTickEventHandler::_GameCycle	= 0;
/// Shall we tock at the beginning of at the end of tickUpdate() ?
bool CTickEventHandler::_TockAtBeginOfTickUpdate = false;

NLMISC::CLightMemDisplayer	RecentHistory;

NLMISC::CLog				_QuickLog;


// user callbacks
void (*userCbUpdate)() = NULL;
void (*userCbSync)() = NULL;


// TickSpeedLoop variable. We store 1 min to get the average (see "help TickSpeedLoop")
CVariable<sint32> TickSpeedLoop("tick", "TickSpeedLoop", "Duration of the tick update (in ms)", -1, 600);

// TotalSpeedLoop variable. We store 1 min to get the average.
CVariable<sint32> TotalSpeedLoop("tick", "TotalSpeedLoop", "Time of game cycle (from TICK from end of UMM) (in ms)", -1, 600 );

// Time of state C1 is TickSpeedLoop.

// Time of state C2 (see ms_automaton.cpp)
CVariable<sint16> ProcessMirrorUpdatesSpeed("tick", "ProcessMirrorUpdatesSpeed", "", -1, 600 );

// Time of state C3 (see ms_automaton.cpp)
CVariable<sint16> ReceiveMessagesViaMirrorSpeed("tick", "ReceiveMessagesViaMirrorSpeed", "", -1, 600 );


TAccurateTime TimeBeforeTickUpdate = 0;


//-----------------------------------------------
//	cbRegistered
//
//-----------------------------------------------
static void cbRegistered(CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	TGameTime gameTime = 0;
	msgin.serial( gameTime );
	CTickEventHandler::setGameTime( gameTime );

	TGameTime gameTimeStep = 0;
	msgin.serial( gameTimeStep );
	CTickEventHandler::setGameTimeStep( gameTimeStep );

	TGameCycle gameCycle = 0;
	msgin.serial( gameCycle );
	CTickEventHandler::setGameCycle( gameCycle );

	// user callback
	if( userCbSync )
	{
		userCbSync();
	}

	CTimerManager::getInstance()->syncTick();

} // cbRegistered //



//-----------------------------------------------
//	cbTick
//
//-----------------------------------------------
static void cbTick(CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId)
{
	CTickEventHandler::tickUpdate( serviceId );
} // cbTick //



//-----------------------------------------------
//	cbStepAndTick
//
//-----------------------------------------------
static void cbStepAndTick(CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	TGameTime gameTimeStep = 0;
	msgin.serial( gameTimeStep );
	CTickEventHandler::setGameTimeStep( gameTimeStep );

	CTickEventHandler::tickUpdate( serviceId );

} // cbStepAndTick //



//-----------------------------------------------
//	cbDisplayTime
//
//-----------------------------------------------
static void cbDisplayTime( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	nlinfo("<CTickEventHandler::cbDisplayTime> : Time in the service : %f", (double)CTickEventHandler::getGameTime());

} // cbDisplayTime //



// array of callback items
TUnifiedCallbackItem cbArray[] =
{
	{ "REGISTERED", cbRegistered },
	{ "TICK", cbTick },
	{ "STEP_TICK", cbStepAndTick },
	{ "DISPLAY_TIME", cbDisplayTime },
};






//-----------------------------------------------
//	tickUpdate
//
//-----------------------------------------------
void CTickEventHandler::tickUpdate( TServiceId serviceId )
{
	// increment the time and the number of cycles
	_GameTime += (TGameTime)_GameTimeStep;
	_GameCycle++;
	//nldebug( "TCK-%u: Tick", getGameCycle() );
	time_t t; time( &t );
	_QuickLog.displayNL( "%s: %u: Tick", IDisplayer::dateToHumanString( t ), getGameCycle() );

	//nldebug( "--GC-%u-->", _GameCycle );

	if ( _TockAtBeginOfTickUpdate )
		sendTockBack( serviceId );

	// user update callback
	if ( userCbUpdate )
	{
		H_AUTO(TickUpdate);
		TimeBeforeTickUpdate = getAccurateTime();
		userCbUpdate();
		TickSpeedLoop = accTimeToMs( getAccurateTime() - TimeBeforeTickUpdate );
	}

	if ( ! _TockAtBeginOfTickUpdate )
		sendTockBack( serviceId );
}


//-----------------------------------------------
//	sendTockBack
//
//-----------------------------------------------
void CTickEventHandler::sendTockBack( NLNET::TServiceId serviceId )
{
	// Send back a tock
	CMessage msgout( "TOCK" );
	uint16 v = (TickSpeedLoop.get()<0xFFFF) ? (uint16)TickSpeedLoop.get() : 0xFFFF;
	msgout.serial( v );
	v = (uint16)ProcessMirrorUpdatesSpeed.get();
	msgout.serial( v );
	v = (uint16)ReceiveMessagesViaMirrorSpeed.get();
	msgout.serial( v );
	v = (TotalSpeedLoop.get()<0xFFFF) ? (uint16)TotalSpeedLoop.get() : 0xFFFF;
	msgout.serial( v );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout );
	TSockId host;
	CCallbackNetBase *cnb;
	cnb = CUnifiedNetwork::getInstance()->getNetBase(serviceId, host);
	if( cnb )
	{
		cnb->flush( host );
	}
	//nldebug( "TCK-%u: Tocked", getGameCycle() );
	time_t t; time( &t );
	_QuickLog.displayNL( "%s: %u: Tocked", IDisplayer::dateToHumanString( t ), getGameCycle() );
}



//-----------------------------------------------
//	cbTicksUp
//
//-----------------------------------------------
void cbTicksUp(const std::string &serviceName, TServiceId id, void *arg)
{
	if ( CUnifiedNetwork::getInstance()->isServiceLocal( id ) )
	{
		nlinfo ("Tick Event Handler: Local MS is up, I can start");

		// register to tick service (or tick proxy)
		CMessage msgout("REGISTER");
		bool tocking = true;
		uint16 threshold = 0;
		CConfigFile::CVar *cvTocking = IService::getInstance()->ConfigFile.getVarPtr("Tocking");
		if(cvTocking)
			tocking = (cvTocking->asInt()==0) ? false : true;

		if(!tocking)
		{
			CConfigFile::CVar *cvThreshold = IService::getInstance()->ConfigFile.getVarPtr("Threshold");
			if(cvThreshold)
				threshold = cvThreshold->asInt();
		}

		nlinfo("This service %s and has a threshold of %d",(tocking?"tocks":"doesn't tock"), threshold);
		msgout.serial( tocking );
		msgout.serial( threshold );
		CUnifiedNetwork::getInstance()->send( id, msgout );
	}

} // cbTicksUp //

void cbTicksDown(const std::string &serviceName, TServiceId id, void *arg)
{
	if ( CUnifiedNetwork::getInstance()->isServiceLocal( id ) )
	{
		TickSpeedLoop = -1;
	}
}

//--------------------------------------------------------------
//	init
//
//--------------------------------------------------------------
void CTickEventHandler::init( void (*updateFunc)(), void (*syncFunc)(), bool tockAtBeginOfTickUpdate )
{
	// set the callback
	userCbUpdate = updateFunc;
	nlassert( !userCbSync ); // the callback must be init first by this method (mirror must be init
	userCbSync = syncFunc;
	_TockAtBeginOfTickUpdate = tockAtBeginOfTickUpdate;

	// we unactive tick message to avoid flooding
	DebugLog->addNegativeFilter ("TICK");
	DebugLog->addNegativeFilter ("TOCK");
	DebugLog->addNegativeFilter ("14+5");

	CUnifiedNetwork::getInstance()->addCallbackArray(cbArray,sizeof(cbArray)/sizeof(cbArray[0]));

	CUnifiedNetwork::getInstance()->setServiceUpCallback ("MS", cbTicksUp, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback ("MS", cbTicksDown, NULL);

	RecentHistory.setParam( 6 );
	_QuickLog.addDisplayer( &RecentHistory, false );

} // init


/*
 * Set a callback to call when receiving the first game cycle (call this method in your init())
 * \param syncFunc will be call when the ticks send the syncro
 * \param allowReplaceCallback true if we allow the callback to be replaced
 * \return previous callback if exist, otherwise NULL
 */
TUserSyncCallback CTickEventHandler::setSyncCallback( TUserSyncCallback syncFunc, bool allowReplaceCallback )
{
	if( userCbSync )
	{
		// sync callback is already set, check if replacement is allowed
		nlassert( allowReplaceCallback );
		TUserSyncCallback previousSyncFunc = userCbSync;
		userCbSync = syncFunc;
		return previousSyncFunc;
	}
	else
	{
		userCbSync = syncFunc;
		return NULL;
	}

} // setSyncCallback //


NLMISC_CATEGORISED_DYNVARIABLE(tick, NLMISC::TGameCycle, TickGameCycle, "game cycle (in tick)")
{
	// we can only read the value
	if (get)
		*pointer = CTickEventHandler::getGameCycle ();
}

NLMISC_CATEGORISED_DYNVARIABLE(tick, NLMISC::TGameTime, TickGameTime, "game time (in second)")
{
	// we can only read the value
	if (get)
		*pointer = CTickEventHandler::getGameTime ();
}

NLMISC_CATEGORISED_DYNVARIABLE(tick, bool, ExpediteTock, "ExpediteTock")
{
	if (get)
		*pointer = CTickEventHandler::getTockAtBeginOfTickUpdate();
}

NLMISC_CATEGORISED_COMMAND(tick,displayTickRecentHistory,"Display the history of tick events","")
{
	RecentHistory.write( &log );
	return true;
}


NLMISC_CATEGORISED_COMMAND(tick,setDayTime,"set the current time of the day","<hour> <minute>")
{
	if ( args.size() != 2) return false;

	float newHour;
	fromString(args[0], newHour);
	if ( newHour > 23.0f || newHour < 0.0f )
	{
		log.displayNL( "hour must be < 24" );
		return true;
	}
	float newMinute;
	fromString(args[1], newMinute);
	if ( newMinute > 59.0f || newMinute < 0.0f )
	{
		log.displayNL( "minutes must be < 60" );
		return true;
	}
	newHour+= newMinute / 60.0f;

	float prevHour = (float)fmod(CTickEventHandler::getGameCycle() / float(RYZOM_HOURS_IN_TICKS), 24.0f );

	if ( newHour >= prevHour )
	{
		CTickEventHandler::setGameCycle( (NLMISC::TGameCycle) (CTickEventHandler::getGameCycle() + (newHour-prevHour) * RYZOM_HOURS_IN_TICKS) );
	}
	else
	{
		CTickEventHandler::setGameCycle( (NLMISC::TGameCycle) (CTickEventHandler::getGameCycle() + (24.0f + newHour - prevHour) * RYZOM_HOURS_IN_TICKS) );
	}
	return true;
}

