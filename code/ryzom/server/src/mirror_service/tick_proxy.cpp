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



// net
#include <nel/net/unified_network.h>
#include <nel/net/service.h>

#include <nel/misc/command.h>
#include <nel/misc/variable.h>
#include <nel/misc/hierarchical_timer.h>

#include <game_share/tick_event_handler.h>

#include "tick_proxy.h"

#include <time.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;



// game time 
TGameTime CTickProxy::_GameTime = 0;
// game time step
TGameTime CTickProxy::_GameTimeStep = 0;
// game cycle
TGameCycle	CTickProxy::_GameCycle	= 0;

TAccurateTime			CTickProxy::_BeginOfTickTime = 0;

uint					CTickProxy::_NbTocked = 0;

std::vector<TServiceId>		CTickProxy::_Services;

std::vector<TServiceId>		CTickProxy::_TockedServices;

TServiceId					CTickProxy::_MasterTickService(0);

TTickTockState			CTickProxy::State = ExpectingMasterTick;

CMirrorGameCycleTimeMeasure	CTickProxy::TimeMeasures;

extern NLMISC::CLightMemDisplayer	RecentHistory; // get the one in tick_event_handler.obj

extern NLMISC::CLog				_QuickLog; // same


string WaitingForServices;

// user callbacks
void (*onTick)() = NULL;
void (*onSync)() = NULL;
extern void cbDoNextTask();

// In the Tick Proxy, set TotalSpeedLoop to the duration of a whole tick cycle on the machine
// (between master send tick and the end of the process after the last local tock received)
// For the Mirror Service, it's between state 1 and state 6 of the automaton (see ms_automaton.cpp).
extern CVariable<sint32> TotalSpeedLoop;


uint64 getPerfTime()
{
  double time = CTime::ticksToSecond( CTime::getPerformanceTime() );
  return (uint64)(time * 1000.0);
}


/*
 * Receive tick subscription from client service
 */
static void cbRegisterToTickSystem(CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	CTickProxy::addService( serviceId );

	/*if ( CTickProxy::alreadySyncd() )
		CTickProxy::sendSyncToClient( serviceId );*/

	cbDoNextTask();
}


//-----------------------------------------------
//	cbRegistered
//
//-----------------------------------------------
static void cbSyncFromMaster(CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	TGameTime gameTime = 0;
	msgin.serial( gameTime );
	CTickProxy::setGameTime( gameTime );

	TGameTime gameTimeStep = 0;
	msgin.serial( gameTimeStep );
	CTickProxy::setGameTimeStep( gameTimeStep );
	
	TGameCycle gameCycle = 0;
	msgin.serial( gameCycle );
	CTickProxy::setGameCycle( gameCycle );
	//nldebug( "TCK-%u: Master Sync", gameCycle );
	//time_t t; time( &t );
	_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: Master Sync", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, gameCycle );
	// user callback
	onSync();

} // cbRegistered //



//-----------------------------------------------
//	cbTick
//
//-----------------------------------------------
static void cbTickFromMaster(CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	CTickProxy::masterTickUpdate( serviceId );

} // cbTick //


//-----------------------------------------------
//	cbStepAndTick
//
//-----------------------------------------------
static void cbStepAndTick(CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	TGameTime gameTimeStep = 0;
	msgin.serial( gameTimeStep );
	CTickProxy::setGameTimeStep( gameTimeStep );

	CTickProxy::masterTickUpdate( serviceId );
	
} // cbStepAndTick //


// From a client service
void cbTock( CMessage& msgin, const string& serviceName, TServiceId serviceId )
{
	nlassert( CTickProxy::State == ExpectingLocalTocks );
	CTickProxy::receiveTockFromClient( msgin, serviceId );
}


//-----------------------------------------------
//	cbDisplayTime
//
//-----------------------------------------------
static void cbDisplayTime( CMessage& msgin, const string &serviceName, TServiceId serviceId )
{
	nlinfo("Time in the service : %f", (double)CTickProxy::getGameTime());

} // cbDisplayTime //



// array of callback items
TUnifiedCallbackItem cbTickProxyArray[] = 
{
	{ "REGISTER", cbRegisterToTickSystem },
	{ "REGISTERED", cbSyncFromMaster },
	{ "TICK", cbTickFromMaster },
	{ "TOCK", cbTock },
	{ "STEP_TICK", cbStepAndTick },
	{ "DISPLAY_TIME", cbDisplayTime },
};


void CTickProxy::addService( TServiceId serviceId )
{
	bool isFirstService = _Services.empty();
	_Services.push_back( serviceId );

	// Send sync
	if ( CTickProxy::alreadySyncd() )
		CTickProxy::sendSyncToClient( serviceId );

	if ( State == ExpectingLocalTocks )
	{
		// Simulate a first tock
		CMessage msgin( "", false );
		receiveTockFromClient( msgin, serviceId, false );
	}
}


/*
 * Supports any service id, even one not added before (ignored then)
 */
void CTickProxy::removeService( TServiceId serviceId )
{
	vector<TServiceId>::iterator it = find( _Services.begin(), _Services.end(), serviceId );
	if ( it == _Services.end() )
		return; // ignore a service not registered
	
	_Services.erase( it );

	// Simulate Tock from leaving service if needed
	if ( State == ExpectingLocalTocks )
	{
		if ( find( _TockedServices.begin(), _TockedServices.end(), serviceId ) != _TockedServices.end() )
		{
			// The service already tocked and we are still in this state, it means we're waiting for another service to tock
			nlassert( _NbTocked != 0 );
			--_NbTocked; // decrement because _Service.size() is being decremented

			// If other tocks are coming, they will call doNextTask. However, it is possible that no
			// other tock is expected, if the tock did not triggered a master tock (changing State).
			if ( _NbTocked == _Services.size() )
				cbDoNextTask();
			else
				nldebug( "TCK- Service %hu already tocked, waiting for %u other tocks", serviceId.get(), _Services.size() - _NbTocked );
		}
		else
		{
			// The service didn't tock yet, interpret its quitting as a tock
			cbDoNextTask();
		}
	}
}


void CTickProxy::receiveTockFromClient( CMessage& msgin, TServiceId senderId, bool real )
{
	// Receive measures of client service
	TimeMeasures.ServiceMeasures.push_back( CServiceGameCycleTimeMeasure() );
	CServiceGameCycleTimeMeasure& stm = TimeMeasures.ServiceMeasures.back();
	stm.ClientServiceId = senderId;
	if ( real )
	{
		stm.ServiceMeasure[TickTockInterval] = accTimeToMs( getAccurateTime() - _BeginOfTickTime );
		msgin.serial( stm.ServiceMeasure[TickUpdateDuration] );
		msgin.serial( stm.ServiceMeasure[PrevProcessMirrorUpdateDuration] );
		msgin.serial( stm.ServiceMeasure[PrevReceiveMsgsViaMirrorDuration] );
		msgin.serial( stm.ServiceMeasure[PrevTotalGameCycleDuration] );
	}
	else
	{
		for ( uint i=0; i!=NbServiceTimeMeasureTypes; ++i )
			stm.ServiceMeasure[i] = 0;
	}
	
	//nldebug( "TCK-%u: %hu tocking", getGameCycle(), senderId );
	//time_t t; time( &t );
	_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: %hu tocking", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, getGameCycle(), senderId.get() );
	++_NbTocked;
	_TockedServices.push_back( senderId );

	cbDoNextTask();
}


void CTickProxy::masterTickUpdate( TServiceId serviceId )
{
	H_AUTO(masterTickUpdate);

	// increment the time and the number of cycles
	_GameTime += (TGameTime)_GameTimeStep;
	_GameCycle++;
	CTickEventHandler::setGameCycle( _GameCycle ); // set it for shared code

	//nldebug( "TCK-%u: Master Tick", getGameCycle() );
	//time_t t; time( &t );
	_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: Master Tick", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, getGameCycle() );
	
	//nldebug( "--GC-%u-->", _GameCycle );

	TimeMeasures.ServiceMeasures.clear();
	_BeginOfTickTime = getAccurateTime();
	{
		H_AUTO(TickUpdate);
		onTick();
	}
}


void CTickProxy::sendSyncToClient( TServiceId serviceId )
{
	CMessage msgout( "REGISTERED" );
	msgout.serial( _GameTime );
	msgout.serial( _GameTimeStep );
	msgout.serial( _GameCycle );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout );
	//nldebug( "TCK-%u: Sync %hu", getGameCycle(), serviceId );
	//time_t t; time( &t );
	_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: Sync %hu", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, getGameCycle(), serviceId.get() );
}

void CTickProxy::sendSyncs()
{
	vector<TServiceId>::const_iterator its;
	for ( its=_Services.begin(); its!=_Services.end(); ++its )
	{
		CMessage msgout( "REGISTERED" );
		msgout.serial( _GameTime );
		msgout.serial( _GameTimeStep );
		msgout.serial( _GameCycle );
		CUnifiedNetwork::getInstance()->send( (*its), msgout );
		//nldebug( "TCK-%u: Sync %hu", getGameCycle(), *its );
		//time_t t; time( &t );
		_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: Sync %hu", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, getGameCycle(), its->get());
	}
}

void CTickProxy::sendTicks()
{
	nlassert( CTickProxy::State == ExpectingMasterTick );

	vector<TServiceId>::const_iterator its;
	for ( its=_Services.begin(); its!=_Services.end(); ++its )
	{
		CMessage msgout( "TICK" );
		CUnifiedNetwork::getInstance()->send( (*its), msgout ); // can produce the warning "Can't find selected connection id 0 to send message to METS because connection is not valid or connected, find a valid connection id", if the service is disconnecting but we aren't aware yet

		//nldebug( "TCK-%u: Tick %hu", getGameCycle(), *its );
		//time_t t; time( &t );
		_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: Tick %hu", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, getGameCycle(), its->get() );
	}
	// nldebug( "Now expecting local tocks" );
	State = ExpectingLocalTocks;
}


void CTickProxy::displayExpectedTocks( /*NLMISC::CDisplayer *log*/ )
{
	WaitingForServices.clear();
	vector<TServiceId>::const_iterator its;
	for ( its=_Services.begin(); its!=_Services.end(); ++its )
	{
		if ( find( _TockedServices.begin(), _TockedServices.end(), (*its) ) == _TockedServices.end() )
		{
			WaitingForServices += toString( "%hu ", its->get() );
		}
	}
	//log->displayNL( "%s", s.c_str() );
}


void CTickProxy::sendMasterTock()
{
	sendTockBack( _MasterTickService );
	_NbTocked = 0;
	_TockedServices.clear();
	State = ExpectingMasterTick;
}


void CTickProxy::setEndOfTick()
{
	TotalSpeedLoop = accTimeToMs( getAccurateTime() - _BeginOfTickTime );
}



//-----------------------------------------------
//	sendTockBack
//
//-----------------------------------------------
void CTickProxy::sendTockBack( TServiceId serviceId )
{
	// send back a tock
	CMessage msgout( "TOCK" );
	msgout.serial( TimeMeasures );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout );
	TSockId host;
	CCallbackNetBase *cnb;
	cnb = CUnifiedNetwork::getInstance()->getNetBase((TServiceId)serviceId, host);
	if( cnb )
	{
		cnb->flush( host );
		//nlinfo( "TOCK sent at %.6f", CTime::ticksToSecond( CTime::getPerformanceTime() ) );
	}
	//nldebug( "TCK-%u: Tocked Master", getGameCycle() );
	static uint32 prev = 0;
	if ( getGameCycle() == prev )
		nlwarning( "Tocked master twice in the same tick!" );
	prev = getGameCycle();
	//time_t t; time( &t );
	_QuickLog.displayNL( "%"NL_I64"u: TCK-%u: Tocked Master", getPerfTime() /*IDisplayer::dateToHumanString( t )*/, getGameCycle() );
}



//-----------------------------------------------
//	cbTicksUp
//
//-----------------------------------------------
void cbTPTicksUp(const std::string &serviceName, TServiceId id, void *arg)
{
	nlinfo ("TICKS is up, I can start");

	// register to tick service
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

	CTickProxy::setMasterTickService( id );
	
} // cbTicksUp //

void cbTPTicksDown(const std::string &serviceName, TServiceId id, void *arg)
{
	nlinfo ("TICKS is down");

	TotalSpeedLoop = -1;
	CTickProxy::setMasterTickService( TServiceId(0) );
}

//--------------------------------------------------------------
//	init
//
//--------------------------------------------------------------
void CTickProxy::init( void (*updateFunc)(),
					   void (*syncFunc)() )
{
	// set the callbacks
	onTick = updateFunc;
	onSync = syncFunc;
	nlassert( updateFunc );
	nlassert( syncFunc );
	
	// Hide tick messages to avoid flooding
	DebugLog->addNegativeFilter ("TICK");
	DebugLog->addNegativeFilter ("TOCK");
	DebugLog->addNegativeFilter ("14+5");

	CUnifiedNetwork::getInstance()->addCallbackArray(cbTickProxyArray,sizeof(cbTickProxyArray)/sizeof(cbTickProxyArray[0]));

	CUnifiedNetwork::getInstance()->setServiceUpCallback ("TICKS", cbTPTicksUp, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback ("TICKS", cbTPTicksDown, NULL);

	RecentHistory.setParam( 100 );
	_QuickLog.addDisplayer( &RecentHistory, false );
	
} // init


NLMISC_DYNVARIABLE(string, WaitingForServices, "Services that haven't tocked yet")
{
	// we can only read the value
	if (get)
	{
		CTickProxy::displayExpectedTocks();
		*pointer = WaitingForServices;
	}
}


NLMISC_DYNVARIABLE(NLMISC::TGameCycle, TickGameCycleProxy, "game cycle (in tick)")
{
	// we can only read the value
	if (get)
		*pointer = CTickProxy::getGameCycle ();
}

NLMISC_DYNVARIABLE(NLMISC::TGameTime, TickGameTimeProxy, "game time (in second)")
{
	// we can only read the value
	if (get)
		*pointer = CTickProxy::getGameTime ();
}


NLMISC_COMMAND(displayTickProxyRecentHistory,"Display the history of tick events","")
{
	RecentHistory.write( &log );
	return true;
}

