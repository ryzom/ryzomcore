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



#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"

#include "game_share/ryzom_version.h"
#include "game_share/tick_event_handler.h"
#include "game_share/backup_service_interface.h"
#include "game_share/singleton_registry.h"
#include <time.h>

#include "tick_service.h"
#include <string>

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}


CTickService * TS = NULL;

NLNET::TServiceId SlowestService(0);
NLNET::TServiceId SlowestCandidate(0);
double SlowestTock = 0.0;
string WaitingForServices;

const string GAME_CYCLE_FILE = "game_cycle.ticks";

const NLMISC::TGameCycle INTERVAL_FOR_SAVING_GAME_CYCLE = 300;

// when true the tick service stops broadcasting ticks
bool Pause = false;

// In the Tick Service, set TotalSpeedLoop to the duration of a whole tick cycle
// (between send tick and the last tock received)
extern CVariable<sint32> TotalSpeedLoop;
extern CVariable<sint32> TickSpeedLoop;

static NLMISC::TTime BeginOfTickTime = 0;

CVariable<uint> TickSendingMode( "tick", "TickSendingMode", "0=Continuous 1=StepByStep 2=Fastest", 0,
	0, true );

/*CVariable<float> WaitForBSThreshold( "tick", "WaitForBSThreshold", "Threshold for BSAckDelay beyond which tick starts to slow down to wait for BS (expressed as a factor of _TickTimeStep)", 1.0f,
	0, true );
*/

//-----------------------------------------------
//	cbRegister
//
//-----------------------------------------------
static void cbRegister(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	if( serviceId.get() > 255 )
	{
		nlwarning("<cbRegister> Service %s id is too higher than 255 : %d, this service will be not registered",serviceName.c_str(),serviceId.get());
		return;
	}

	bool tocking = true;
	uint16 threshold = 0;
	msgin.serial( tocking );
	msgin.serial( threshold );
	TS->registerClient( serviceId, tocking, threshold );
		
	CMessage msgout( "REGISTERED" );
	TGameTime gameTime = TS->getGameTime();
	TGameTime gameTimeStep = TS->getGameTimeStep();
	TGameCycle gameCycle = TS->getGameCycle();
	
	msgout.serial( gameTime );
	msgout.serial( gameTimeStep );
	msgout.serial( gameCycle );

	CUnifiedNetwork::getInstance()->send(serviceId,msgout);

	if( TS->getClientCount() == 1 )
	{
		TS->FirstTime = true;
		BeginOfTickTime = CTime::getLocalTime();
		TS->checkTockReceived();
	}

} // cbRegister //



//-----------------------------------------------
//	cbTock
//
//-----------------------------------------------
static void cbTock(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	// Receive measures of client service
	TS->MainTimeMeasures.CurrentMirrorMeasures.push_back( CMirrorGameCycleTimeMeasureMS() );
	CMirrorGameCycleTimeMeasureMS& mm = TS->MainTimeMeasures.CurrentMirrorMeasures.back();
	msgin.serial( mm );
	mm.MSId = serviceId;

	// Process Tock
	//nlinfo( "TOCK rcvd at %.6f", CTime::ticksToSecond( CTime::getPerformanceTime() ) );
	SlowestCandidate=serviceId;
	TS->addTock( serviceId );

} // cbTock //




//-----------------------------------------------
//	cbHaltTick
//
//-----------------------------------------------
static void cbHaltTick(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	std::string	reason = "<UNKNOWN>";

	try
	{
		msgin.serial(reason);
	}
	catch(const Exception&)
	{
	}

	reason = toString("service %s:%d halted: %s", serviceName.c_str(), serviceId.get(), reason.c_str());

	TS->haltTick(reason);

} // cbHaltTick //

//-----------------------------------------------
//	cbResumeTick
//
//-----------------------------------------------
static void cbResumeTick(CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	TS->resumeTick();

} // cbHaltTick //


//-----------------------------------------------
//	callback table for input message 
//
//-----------------------------------------------
TUnifiedCallbackItem CbArray[] =
{
	{ "REGISTER",		cbRegister },
	{ "TOCK",			cbTock },

	{ "HALT_TICK",		cbHaltTick },
	{ "RESUME_TICK",	cbResumeTick },
};


/*
 * halt ticking
 */
void	CTickService::haltTick(const std::string& reason)
{
	if (CurrentMode == TickHalted)
		return;

	CurrentMode = TickHalted;

	nlwarning("***********************************************************");
	nlwarning("WARNING:");
	nlwarning("TICK SERVICE IS ENTERING HALTED MODE");
	nlwarning("HALT REQUEST IS '%s'", reason.c_str());
	nlwarning("***********************************************************");

	HaltedReason = reason;

	IService::setCurrentStatus("HALTED");
}

/*
 * resume ticking
 */
void	CTickService::resumeTick()
{
	if (CurrentMode == TickRunning)
		return;

	// restart ticking
	CurrentMode = TickRunning;

	nlwarning("***********************************************************");
	nlwarning("WARNING:");
	nlwarning("TICK SERVICE IS RESUMED");
	nlwarning("HALT REQUEST WAS '%s'", HaltedReason.c_str());
	nlwarning("***********************************************************");

	// check in case tocks were received during halt
	checkTockReceived();

	IService::clearCurrentStatus("HALTED");
}



//-----------------------------------------------
//	registerClient
//
//-----------------------------------------------
void CTickService::registerClient( NLNET::TServiceId serviceId, bool tocking, uint16 threshold )
{ 
	if( serviceId.get() < _ClientInfos.size() )
	{
		if( _ClientInfos[serviceId.get()].Registered == true )
		{
			nlerror("<CTickService::registerClient> The service %d is alredy registered",serviceId.get());
		}

		_ClientInfos[serviceId.get()].Registered = true; 
		_ClientInfos[serviceId.get()].Tocking = tocking; 
		_ClientInfos[serviceId.get()].Threshold = threshold;
		_QuickLog.displayNL( "%u: +%hu", getGameCycle(), serviceId.get() );
	}
	else
	{
		nlwarning("<CTickService::registerClient> Service id too high : %d (max is %d)",serviceId.get(),_ClientInfos.size()-1);
	}

} // registerClient //



//-----------------------------------------------
//	addTock
//
//-----------------------------------------------
void CTickService::addTock( NLNET::TServiceId serviceId )
{
	time_t t; time( &t );
	_QuickLog.displayNL( "%s: %u: [<-%hu] TOCK-r", IDisplayer::dateToHumanString(t), getGameCycle(), serviceId.get() );
	if( serviceId.get() < _ClientInfos.size() )
	{
		if( _ClientInfos[serviceId.get()].Registered )
		{
			if( _ClientInfos[serviceId.get()].TockReceived )
			{
				if( _ClientInfos[serviceId.get()].Threshold == 0 )
				{
					nlwarning("<CTickService::addTock> Tock received more than once despite a threshold null");
				}
			}
			else
			{
				_ClientInfos[serviceId.get()].TockReceived = true;
				_QuickLog.displayNL( "%u: [<-%hu] TOCK-p", getGameCycle(), serviceId.get() );
			}

			if( _ClientInfos[serviceId.get()].TockMissingCount > 0 )
			{
				_ClientInfos[serviceId.get()].TockMissingCount--;
			}
			else
			{
				nlwarning("<CTickService::addTock> Receiving a tock, but there was no tock missing !");
			}
		}	
		else
		{
			nlwarning("(TICKS)<CTickService::addTock> Tock received from service %d which is not registered",serviceId.get());
		}
		checkTockReceived();
	}
	else
	{
		nlwarning("<CTickService::addTock> Service id too high : %d (max is %d)", serviceId.get(), _ClientInfos.size()-1);
	}

} // addTock //



//-----------------------------------------------
//	checkTockReceived
//
//-----------------------------------------------
void CTickService::checkTockReceived()
{
	// Do not tick if ticking is halted
	if (CurrentMode == TickHalted)
		return;

	// check if all the tocks have been received (or if missing is less than threshold )
	uint i;
	for( i=0; i<_ClientInfos.size(); i++ )
	{
		// if this client is supposed to send a tock
		if( _ClientInfos[i].Registered )
		{			
			// if tock was not received yet
			if( _ClientInfos[i].TockReceived == false )
			{
				// if we have to wait for this client
				if( _ClientInfos[i].Tocking == true )
				{
					// we check threshold
					if( _ClientInfos[i].TockMissingCount > _ClientInfos[i].Threshold )
					{
						return;
					}
				}
			}
		}
	}

	TotalSpeedLoop = (sint32)(CTime::getLocalTime() - BeginOfTickTime);
	// set the TickSpeedLoop var to the same value
	TickSpeedLoop = TotalSpeedLoop;
	MainTimeMeasures.CurrentTickServiceMeasure[PrevTotalTickDuration] = (uint16)TotalSpeedLoop.get();
	//nlinfo( "End of tick at %.6f", CTime::ticksToSecond( CTime::getPerformanceTime() ) );


	// Step by step mode
	if( TickSendingMode.get() == StepByStep )
	{
		if( _StepCount > 0 )
		{
			// broadcast the tick
			broadcastTick();
			_StepCount--;
		}
	}
	// Continuous or fastest mode
	else
	{
		TLocalTime currentTime = ((double)CTime::getLocalTime())/1000.0;

		SlowestService=SlowestCandidate;
		SlowestTock = currentTime - _TickSendTime;

		// if it's the first time we send a tick, we init the tick send time
		if( FirstTime )
		{
			_TickSendTime = currentTime - _TickTimeStep;
			_LastTickTimeStep = _TickTimeStep;
			FirstTime = false;
		}
	
		// setup the default value for the time step to use
		NLMISC::TLocalTime effectiveTimeStep= _TickTimeStep;

		// if the backup service is taking longer than _TickTimeStep to respond, then we need to slow down to a little slower than its pace
/*		NLMISC::TLocalTime lastBSAckDelay= ((NLMISC::TLocalTime)Bsi.getLastAckDelay())/1000.0;
		if ((WaitForBSThreshold>=1.0f) && (float)lastBSAckDelay>(float)_TickTimeStep*WaitForBSThreshold)
		{
			effectiveTimeStep= lastBSAckDelay+ (lastBSAckDelay-_TickTimeStep)/2;
		}
*/
		// smooth out BS update rate fluctuations with a simple filter and store away the time step value for next time round
		effectiveTimeStep= (effectiveTimeStep+3*_LastTickTimeStep)/4;
		_LastTickTimeStep= effectiveTimeStep;

		// if we have to wait before sending tick
		while ( ( currentTime < (_TickSendTime + effectiveTimeStep) ) && ( TickSendingMode.get() != Fastest) )
		{
			if ( currentTime < _TickSendTime )
			{
				nlinfo( "Backward time sync detected (about %.1f s)", _TickSendTime - currentTime );
				_TickSendTime = currentTime;
			}
			else
			{
				nlSleep( (uint32)((_TickSendTime + effectiveTimeStep - currentTime)*1000.0) );
			}
			currentTime = ((double)CTime::getLocalTime())/1000.0;
		}

		// increment the send time
		TLocalTime oldTime = _TickSendTime;
		_TickSendTime = ((double)CTime::getLocalTime())/1000.0;
		double dt = _TickSendTime - oldTime;

		// round off the send time to iron out high frequency effects
		if (_TickSendTime<oldTime+effectiveTimeStep)
			_TickSendTime=oldTime+effectiveTimeStep;

		if (effectiveTimeStep*1000.0>1.0)
		{
			dt = double(uint32(dt*1000.0)/ uint32(effectiveTimeStep*1000.0) * uint32(effectiveTimeStep*1000.0))/1000.0;
			_TickSendTime = oldTime + dt;
		}

		//nlinfo( " %"NL_I64"u  %"NL_I64"u", (TTime)((_TickSendTime-oldTime)*1000), (TTime)((d2-oldTime)*1000) );		

		// broadcast the tick
		broadcastTick();

		// update the tick time step
		//updateTickTimeStep
	}

} // checkTockReceived //



//-----------------------------------------------
//	broadcastTick
//
//-----------------------------------------------
void CTickService::broadcastTick()
{	
	if ( Pause == true ) return;

	MainTimeMeasures.beginNewCycle();
	BeginOfTickTime = CTime::getLocalTime();

	// increment the game time and cycle
	_GameTime += _GameTimeStep;
	_GameCycle++;
	CTickEventHandler::setGameCycle( _GameCycle ); // to display the good value in the variable

	// we send a tick to every registered client
	uint i;
	for( i=0; i<_ClientInfos.size(); i++ )
	{
		if( _ClientInfos[i].Registered )
		{
			CMessage msgout;
			if( _GameTimeStepHasChanged )
			{
				msgout.setType( "STEP_TICK" );
				msgout.serial( _GameTimeStep );
				_QuickLog.displayNL( "%u: [->%u] STEP_TICK", getGameCycle(), i );
			}
			else
			{
				msgout.setType( "TICK" );
				time_t t; time( &t );
				_QuickLog.displayNL( "%s: %u: [->%u] TICK", IDisplayer::dateToHumanString(t), getGameCycle(), i );
			}
			
			CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(i),msgout);
			TSockId host;
			CCallbackNetBase *cnb;
			cnb = CUnifiedNetwork::getInstance()->getNetBase(NLNET::TServiceId(i),host);
			if( cnb )
			{
				cnb->flush( host );
			}
			// we are now waiting for a tick from this client
			_ClientInfos[i].TockReceived = false;
			_ClientInfos[i].TockMissingCount++;
		}
	}
	_GameTimeStepHasChanged = false;

	CSingletonRegistry::getInstance()->tickUpdate();
} // broadcastTick //



//-----------------------------------------------
//	cbClientDisconnection
//
//-----------------------------------------------
void cbClientDisconnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg)
{
	TS->unregisterClient( serviceId );
	TS->checkTockReceived();

} // cbClientDisconnection //



//-----------------------------------------------
//	unregisterClient
//
//-----------------------------------------------
void CTickService::unregisterClient( NLNET::TServiceId serviceId )
{
	if( serviceId.get() < _ClientInfos.size() )
	{
		if ( _ClientInfos[serviceId.get()].Registered )
		{
			_ClientInfos[serviceId.get()].Registered = false;
			_ClientInfos[serviceId.get()].TockReceived = true;
			_ClientInfos[serviceId.get()].Tocking = true;
			_ClientInfos[serviceId.get()].Threshold = 0;
			_ClientInfos[serviceId.get()].TockMissingCount = 0;
			_QuickLog.displayNL( "%u: -%hu", getGameCycle(), serviceId.get() );
		}
		else
		{
			nlinfo("<CTickService::unregisterClient> Service %d disconnection detected but this service is not registered : nothing to do",serviceId.get());
		}
	}
	else
	{
		nlwarning("<CTickService::unregisterClient> Service %d  : service id too big ! (max is %d)",serviceId.get(),_ClientInfos.size()-1);
	}

} // unregisterClient //



//-----------------------------------------------
//	getClientCount
//
//-----------------------------------------------
uint16 CTickService::getClientCount()
{
	uint16 clientCount = 0;

	uint i;
	for( i=0; i<_ClientInfos.size(); i++ )
	{
		if( _ClientInfos[i].Registered )
		{	
			clientCount++;
		}
	}

	return clientCount;

} // getClientCount //



//-----------------------------------------------
//	displayGameTime() 
//
//-----------------------------------------------
void CTickService::displayGameTime() const
{
	CMessage msgout( "DISPLAY_TIME" );
	uint i;
	for( i=0; i<_ClientInfos.size(); i++ )
	{
		if( _ClientInfos[i].Registered )
		{
			CUnifiedNetwork::getInstance()->send(NLNET::TServiceId(i),msgout);
		}
	}

	nlinfo("(TICKS)<CTickService::displayTime> : Game time in the Tick Service : %f sec (local: %f ms)", (double)_GameTime,(double)CTime::getLocalTime());

} // displayGameTime //





/*
 * Initialise the service
 */
void CTickService::init()
{
	setVersion (RYZOM_VERSION);

	CurrentMode = TickRunning;

	CSingletonRegistry::getInstance()->init();
	
	// keep pointer on class	
	TS = this;

	_StepCount = 1;

	// maximum 256 clients can be connected
	_ClientInfos.resize(256);


	try
	{
		_GameTime = ConfigFile.getVar("GameTime").asFloat();
	}
	catch(const Exception &)
	{
		// init the game time
		_GameTime = 0.0f;
	}

	try
	{
		_GameCycle = ConfigFile.getVar("GameCycle").asInt();
		_SavedGameCycle = _GameCycle;
	}
	catch(const Exception &)
	{
		// init the game cycle from file
		loadGameCycle();
	}
	CTickEventHandler::setGameCycle( _GameCycle ); // to display the good value in the variable

	try
	{
		_TickTimeStep = ConfigFile.getVar("TickTimeStep").asFloat();
	}
	catch(const Exception &)
	{
		// tick service time step between two ticks
		_TickTimeStep = 0.1f;
	}
	
	try
	{
		_GameTimeStep = ConfigFile.getVar("GameTimeStep").asFloat();
	}
	catch(const Exception &)
	{
		// game time between two ticks
		_GameTimeStep = 0.1f;
	}

	// thus the time step will be sent along with the first tick
	_GameTimeStepHasChanged = true;

	// local time when last tick was sent
	_TickSendTime = 0;
	FirstTime = true;

	CUnifiedNetwork::getInstance()->setServiceDownCallback("*", cbClientDisconnection, NULL);

	RecentHistory.setParam( 20 );
	_QuickLog.addDisplayer( &RecentHistory, false );

	_RangeMirrorManager.init();

} // init //


/*
 * Release
 */
void CTickService::release()
{

	if ( saveGameCycle() )
	{
//		nlinfo( "Game cycle %u saved to %s", _GameCycle, GAME_CYCLE_FILE.c_str() );
	}

	CSingletonRegistry::getInstance()->release();
}


/*
 * Update
 */
bool CTickService::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();

	if (CurrentMode == TickHalted)
	{
		WaitingForServices = "TICK HALTED: "+HaltedReason;
	}
	else
	{
		WaitingForServices = "";
		// check if all the tocks have been received (or if missing is less than threshold )
		uint i;
		for( i=0; i<_ClientInfos.size(); i++ )
		{
			// if this client is supposed to send a tock
			if( _ClientInfos[i].Registered )
			{	
				// if tock was not received yet
				if( _ClientInfos[i].TockReceived == false )
				{
					// if we have to wait for this client
					if( _ClientInfos[i].Tocking == true )
					{
						// we check threshold
						if( _ClientInfos[i].TockMissingCount > _ClientInfos[i].Threshold )
						{
							WaitingForServices += toString(i) + " ";
						}
					}
				}
			}
		}

		// don't saveGameCycle if tick halted
		// because BS probably sent an error report, and it would be hazardous
		// to send BS new commands!!
		if ( _GameCycle > _SavedGameCycle + INTERVAL_FOR_SAVING_GAME_CYCLE )
		{
			saveGameCycle();
		}
	}

	return true;
}


/*
 * Save to file
 */
bool CTickService::saveGameCycle()
{
	_SavedGameCycle = _GameCycle; // do it anyway
	try
	{
		CBackupMsgSaveFile msg( GAME_CYCLE_FILE, CBackupMsgSaveFile::SaveFile, Bsi );
		msg.DataMsg.serialVersion( 0 );
		msg.DataMsg.serial( _GameCycle );
		Bsi.sendFile(msg);

/*
		COFile file;
		if ( file.open( SaveShardRoot.get() + IService::getInstance()->SaveFilesDirectory.toString() + GAME_CYCLE_FILE ) )
		{
			file.serialVersion( 0 );
			file.serial( _GameCycle );
			file.close();
		}
		else
			throw EFileNotOpened( SaveShardRoot.get() + IService::getInstance()->SaveFilesDirectory.toString() + GAME_CYCLE_FILE );
		return true;
*/
		return true;
	}
	catch (const Exception &e)
	{
		nlwarning( "Can't save game cycle: %s", e.what() );
		return false;
	}
}

struct TTickFileCallback : public IBackupFileReceiveCallback
{
	CTickService *TickService;
	TTickFileCallback(CTickService *tickService)
		: TickService(tickService)
	{
	}

	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		TickService->tickFileCallback(fileDescription, dataStream);
	}

};

void CTickService::tickFileCallback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
{
	if (!fileDescription.FileName.empty())
	{
		dataStream.serialVersion( 0 );
		dataStream.serial( _GameCycle );
		_SavedGameCycle = _GameCycle;
	}
	else
	{
		_GameCycle = 0;
		_SavedGameCycle = 0;
		nlwarning("No tick file found, starting with fresh tick");
	}
}

/*
 * Load from file
 */
bool CTickService::loadGameCycle()
{
	TTickFileCallback *cb = new TTickFileCallback(this);
	Bsi.syncLoadFile(GAME_CYCLE_FILE, cb);

	return true;

//	try
//	{
//		CIFile file;
//		if ( file.open( Bsi.getLocalPath() + GAME_CYCLE_FILE ) )
//		{
//			file.serialVersion( 0 );
//			file.serial( _GameCycle );
//			file.close();
//			_SavedGameCycle = _GameCycle;
//		}
//		else
//			throw EFileNotOpened( Bsi.getLocalPath() + GAME_CYCLE_FILE );
//		nlinfo( "Loaded game cycle %u from %s, will be saved every %u game cycles", _GameCycle, (CPath::standardizePath(IService::getInstance()->SaveFilesDirectory.toString()) + GAME_CYCLE_FILE).c_str(), INTERVAL_FOR_SAVING_GAME_CYCLE );
//		return true;
//	}
//	catch (const Exception &e)
//	{
//		nlwarning( "Can't load game cycle: %s", e.what() );
//		_GameCycle = 0;
//		_SavedGameCycle = _GameCycle;
//		return false;
//	}
}


/*
 *
 */
CTickServiceGameCycleTimeMeasure::CTickServiceGameCycleTimeMeasure() : HistoryMain( CTickService::getInstance()->getServiceId(), NLNET::TServiceId(std::numeric_limits<uint16>::max()), false ) {}


/*
 *
 */
void CTickServiceGameCycleTimeMeasure::beginNewCycle()
{
	if ( ! CurrentMirrorMeasures.empty() )
	{
		// Update stats
		for ( TMirrorMeasures::const_iterator imm=CurrentMirrorMeasures.begin(); imm!=CurrentMirrorMeasures.end(); ++imm )
		{
			storeMeasureToHistory( HistoryByMirror, imm->MirrorMeasure, imm->MSId, TServiceId(0) );

			for ( std::vector<CServiceGameCycleTimeMeasure>::const_iterator ism=(*imm).ServiceMeasures.begin(); ism!=(*imm).ServiceMeasures.end(); ++ism )
			{
				storeMeasureToHistory( HistoryByService, (*ism).ServiceMeasure, (*ism).ClientServiceId, (*imm).MSId );
			}
		}

		//nlinfo( "AV: C=%hu H=%hu", CurrentTickServiceMeasure[0], HistoryMain.Stats[MHTMax][0] );
		HistoryMain.updateStats( CurrentTickServiceMeasure );
		//nlinfo( "AP: C=%hu H=%hu", CurrentTickServiceMeasure[0], HistoryMain.Stats[MHTMax][0] );

		// Clear current
		CurrentMirrorMeasures.clear();
	}
}



/*
 *
 */
void CTickServiceGameCycleTimeMeasure::resetMeasures()
{
	HistoryByMirror.clear();
	HistoryByService.clear();
	HistoryMain.reset( false );
}


/*
 *
 */
void CTickServiceGameCycleTimeMeasure::displayStats( NLMISC::CLog *log )
{
	log->displayNL( "Shard timings at GC %u:", CTickEventHandler::getGameCycle() );
	log->displayRawNL( "___AVG__" );
	displayStat( log, MHTSum );	
	log->displayRawNL( "___MAX___" );
	displayStat( log, MHTMax );	
	log->displayRawNL( "___MIN___" );
	displayStat( log, MHTMin );	
}




const char * ServiceTimeMeasureTypeToCString [NbServiceTimeMeasureTypes] =
{ "TickTockInterval", "TickUpdateDuration", "PrevProcessMirrorUpdateDuration", "PrevReceiveMsgsViaMirrorDuration", "PrevTotalGameCycleDuration" };

const char * MirrorTimeMeasureTypeToCString [NbMirrorTimeMeasureTypes] =
{ "WaitAllReceivedDeltaDuration", "BuildAndSendDeltaDuration", "PrevApplyReceivedDeltaDuration", "PrevSendUMMDuration" };

const char * TickServiceTimeMeasureTypeToCString [NbTickServiceTimeMeasureTypes] =
{ "PrevTotalTickDuration" };


/*
 *
 */
void CTickServiceGameCycleTimeMeasure::displayStat( NLMISC::CLog *log, TTimeMeasureHistoryStat stat )
{
	log->displayRawNL( "\tTICKS, %u measures:", HistoryMain.NbMeasures );
	uint divideBy = (HistoryMain.NbMeasures==0) ? 0 : ((stat==MHTSum) ? HistoryMain.NbMeasures : 1);
	HistoryMain.Stats[stat].displayStat( log, TickServiceTimeMeasureTypeToCString, divideBy );
	{
		CMirrorTimeMeasure gatheredStats [NbTimeMeasureHistoryStats] = { 0, std::numeric_limits<uint16>::max(), 0 };
		for ( std::vector<CMirrorTimeMeasureHistory>::const_iterator ihm=HistoryByMirror.begin(); ihm!=HistoryByMirror.end(); ++ihm )
		{
			log->displayRawNL( "\tMS-%hu, %u measures:", (*ihm).ServiceId.get(), (*ihm).NbMeasures );
			divideBy = (stat==MHTSum) ? (*ihm).NbMeasures : 1;
			(*ihm).Stats[stat].displayStat( log, MirrorTimeMeasureTypeToCString, divideBy );
			gatheredStats[MHTSum] += (*ihm).Stats[stat] / (*ihm).NbMeasures;
			gatheredStats[MHTMin].copyLowerValues( (*ihm).Stats[stat] );
			gatheredStats[MHTMax].copyHigherValues( (*ihm).Stats[stat] );
		}
		if ( HistoryByMirror.size() > 1 )
		{
			log->displayRawNL( "\tAll mirror services:" );
			gatheredStats[stat].displayStat( log, MirrorTimeMeasureTypeToCString, (stat==MHTSum) ? (uint)HistoryByMirror.size() : 1 ); // not displaying the 3 stats
		}
	}
	{
		CServiceTimeMeasure gatheredStats [NbTimeMeasureHistoryStats] = { 0, std::numeric_limits<uint16>::max(), 0 };
		for ( std::vector<CServiceTimeMeasureHistory>::const_iterator ihs=HistoryByService.begin(); ihs!=HistoryByService.end(); ++ihs )
		{
			log->displayRawNL( "\t%s (on MS-%hu), %u measures:", CUnifiedNetwork::getInstance()->getServiceUnifiedName( (*ihs).ServiceId ).c_str(), (*ihs).ParentServiceId.get(), (*ihs).NbMeasures );
			divideBy = (stat==MHTSum) ? (*ihs).NbMeasures : 1;
			(*ihs).Stats[stat].displayStat( log, ServiceTimeMeasureTypeToCString, divideBy );
			gatheredStats[MHTSum] += (*ihs).Stats[stat] / (*ihs).NbMeasures;
			gatheredStats[MHTMin].copyLowerValues( (*ihs).Stats[stat] );
			gatheredStats[MHTMax].copyHigherValues( (*ihs).Stats[stat] );
		}
		if ( HistoryByService.size() > 1 )
		{
			log->displayRawNL( "\tAll client services:" );
			gatheredStats[stat].displayStat( log, ServiceTimeMeasureTypeToCString, (stat==MHTSum) ? (uint)HistoryByService.size() : 1 ); // not displaying the 3 stats
		}
	}
}


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN( CTickService, "TICKS", "tick_service", 0, CbArray, "", "" );


//
// Variables
//

NLMISC_VARIABLE(NLNET::TServiceId, SlowestService, "SId of the slowest service");

NLMISC_VARIABLE(double, SlowestTock, "Time in millisecond to receive the slowest tock");

NLMISC_VARIABLE(string, WaitingForServices, "Services that haven't tocked yet");
	
NLMISC_DYNVARIABLE(TGameTime, GameTime, "Current game time in second")
{
	if (get) *pointer = TS->getGameTime();
}

NLMISC_DYNVARIABLE(TLocalTime, TickTimeStep, "Current tick time step in second")
{
	if (get) *pointer = TS->getTickTimeStep();
	else TS->setTickTimeStep(*pointer);
}

NLMISC_DYNVARIABLE(string, TickMode, "Current ticking mode")
{
	if (get)
	{
		if (TS->CurrentMode == CTickService::TickRunning)
			*pointer = "Normal";
		else
			*pointer = "HALTED: "+TS->HaltedReason;
	}
}


//
// Commands
//

NLMISC_COMMAND(haltTick, "Halt tick server", "[reason]" )
{
	if (args.size() > 1)
		return false;

	log.displayNL("TICK SERVICE HALTED MANUALLY.");
	log.displayNL("PLEASE RESTART TICK SERVICE USING resumeTick COMMAND.");

	std::string	reason = "MANUAL HALT";
	if (args.size() == 1)
		reason += ": "+args[0];

	TS->haltTick(reason);
	return true;
}

NLMISC_COMMAND(resumeTick, "Resume tick server", "" )
{
	log.displayNL("TICK SERVICE RESUMED MANUALLY.");
	TS->resumeTick();
	return true;
}


NLMISC_COMMAND(displayTimingsOfShard, "Display all timings", "" )
{
	TS->MainTimeMeasures.displayStats( &log );
	return true;
}

NLMISC_COMMAND(resetTimingsOfShard, "Reset all timings", "" )
{
	TS->MainTimeMeasures.resetMeasures();
	return true;
}

NLMISC_COMMAND(displayGameTime, "The tick service and all connected services display its current game time", "")
{
	TS->displayGameTime();
	return true;

}

NLMISC_COMMAND(send,"enable send (in the step by step mode)"," ")
{
	TS->enableSend();
	return true;

}

NLMISC_COMMAND(clients,"Give the number of clients"," ")
{
	log.displayNL("Number of connected client : %d", TS->getClientCount());
	return true;

}

/*
NLMISC_COMMAND(getServicesState,"Give the current state of all services"," ")
{
// todo end the code
	uint i;
	for( i=0; i<_ClientInfos.size(); i++ )
	{
		// if this client is supposed to send a tock
		if( _ClientInfos[i].Registered )
		{
			// if tock was not received yet
			if( _ClientInfos[i].TockReceived == false )
			{
				// if we have to wait for this client
				if( _ClientInfos[i].Tocking == true )
				{
					// we check threshold
					if( _ClientInfos[i].TockMissingCount > _ClientInfos[i].Threshold )
					{
						return;
					}
				}
			}
		}
	}
	return true;
}
*/


NLMISC_COMMAND(displayRecentHistory,"Display the history of tick events","")
{
	TS->RecentHistory.write( &log );
	return true;
}

NLMISC_COMMAND( pause, "Pause the tick service", "" )
{
	Pause = !Pause;
	if( Pause )
		nlinfo("Pause On");
	else
	{
		nlinfo("Pause Off");
		TS->checkTockReceived();
	}
	return true;
}




/*
 */

/*
// ADDED by ben: BS test purpose, you can remove this
NLMISC_COMMAND( sendFileBS, "send file to BS", "<file to send> <file name to be written>")
{
	if (args.size() != 2)
		return false;

	CIFile	fi;

	if (!fi.open(args[0]))
		return false;

	std::vector<uint8>		buffer(fi.getFileSize());
	fi.serialBuffer(&(buffer[0]), buffer.size());

	CBackupMsgSaveFile		msgSave;
	msgSave.Data.serialBuffer(&(buffer[0]), buffer.size());
	msgSave.FileName = args[1];

	Bsi.sendFile(msgSave);

	return true;
}

NLMISC_COMMAND( sendFileAndCheckBS, "send file to BS", "<file to send> <file name to be written>")
{
	if (args.size() != 2)
		return false;

	CIFile	fi;

	if (!fi.open(args[0]))
		return false;

	std::vector<uint8>		buffer(fi.getFileSize());
	fi.serialBuffer(&(buffer[0]), buffer.size());

	CBackupMsgSaveFile		msgSave;
	msgSave.Data.serialBuffer(&(buffer[0]), buffer.size());
	msgSave.FileName = args[1];

	Bsi.sendFile(msgSave, true);

	return true;
}

NLMISC_COMMAND( appendFileBS, "append file to BS", "<file to send> <file name to be written>")
{
	if (args.size() != 2)
		return false;

	CIFile	fi;

	if (!fi.open(args[0]))
		return false;

	std::vector<uint8>		buffer(fi.getFileSize());
	fi.serialBuffer(&(buffer[0]), buffer.size());

	CBackupMsgSaveFile		msgSave;
	msgSave.Data.serialBuffer(&(buffer[0]), buffer.size());
	msgSave.FileName = args[1];

	Bsi.append(msgSave);

	return true;
}

NLMISC_COMMAND( appendLineBS, "append line to file via BS", "<line to be sent> <file name to be written>")
{
	if (args.size() != 2)
		return false;

	Bsi.append(args[0], args[1]);

	return true;
}

NLMISC_COMMAND( deleteFileBS, "delete file via BS", "<file name to be deleted> [backup file]")
{
	if (args.size() < 1 || args.size() > 2)
		return false;

	bool backupFile = false;

	if (args.size() == 2)
	{
		NLMISC::fromString(args[1], backupFile);
	}

	Bsi.deleteFile(args[0], backupFile);

	return true;
}


class CFileReceiveCb: public IBackupFileReceiveCallback
{
public:

	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		nlinfo("received file '%s'", fileDescription.FileName.c_str());
	}

};

NLMISC_COMMAND( loadFileBS, "load a file from BS", "<file to be loaded>")
{
	if (args.size() != 1)
		return false;

	Bsi.requestFile(args[0], new CFileReceiveCb());

	return true;
}
*/


class CFileClassReceiveCb: public IBackupFileClassReceiveCallback
{
public:
	
	virtual void callback(const CFileDescriptionContainer& list)
	{
		nlinfo("received fileclass");
		uint	i;
		for (i=0; i<list.size(); ++i)
			nlinfo("file: %s", list[i].FileName.c_str());
		nlinfo("end of class");
	}
	
};

NLMISC_COMMAND( loadFileClassBS, "load a fileclass from BS", "<directory> <classes to be loaded>")
{
	if (args.size() <= 1)
		return false;

	std::string						directory;
	std::vector<CBackupFileClass>	classes;
	CBackupFileClass				bclass;

	directory = args[0];

	uint	i;
	for (i=1; i<args.size(); ++i)
		bclass.Patterns.push_back(args[i]);

	classes.push_back(bclass);

	Bsi.requestFileClass(directory, classes, new CFileClassReceiveCb());
	
	return true;
}

//NLMISC_COMMAND( dumpVectorMemory, "Dump memory used in vector", "")
//{
//	static char buffer[1024*16];
//	_vector_memory_usage::dump_memory(buffer, 1024*16);
//
//	log.displayNL("Vector memory usage : \n%s", buffer);
//
//	return true;
//}
