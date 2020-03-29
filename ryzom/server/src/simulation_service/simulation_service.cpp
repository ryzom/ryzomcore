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

// Local includes
#include "simulation_service.h"
#include "game_share/ring_session_manager_itf.h"
#include "simulated_ring_session_manager.h"
#include "simulation_random.h"	// for exponential()
#include "client_cfg.h"
#include "r2_share/object.h"

// Nel Misc and Net
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/net/module_manager.h"

// Debug
#include "game_share/handy_commands.h"	// misc commands for pdr bin->xml, e.g.
#include "nel/misc/sheet_id.h"	// must call CSheetId::init() for above command

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace RSMGR;

const TTime UpdatePeriod = 100;
CSimulationService *CSimulationService::s_pSimService = NULL;

void cbServiceUp( const string &serviceName, uint16 serviceId, void * );
void cbServiceDown( const string &serviceName, uint16 serviceId, void * );
void onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, NLNET::CMessage &message);

/**
 *	CallbackArray
 */
NLNET::TUnifiedCallbackItem CallbackArray[] = 
{
	{ "nop",	NULL },
};

/**
 *	CVariables
 */
CVariable<uint> SimTime( "SS", "SimTime", "Simulation time in seconds since Start", 0 );
CVariable<int> SleepTime( "SS", "SleepTime", "Sleep time in seconds last update", 0 );

CVariable<bool>	RequestQuit( "SS", "RequestQuit", "Set to true to start quitting", false );
CVariable<uint>	UserId( "SS", "UserId", "First UserId to use", 100, 0, true );
CVariable<uint>	NumActs( "SS", "NumActs", "Number of acts in current scenario", 0 );
CVariable<uint>	NumEditorsConnected( "SS", "NumEditorsConnected", "Number of simulated editors connected", 0 );
CVariable<uint>	NumClientsConnected( "SS", "NumClientsConnected", "Number of simulated clients connected", 0 );
CVariable<uint>	NumSessionsCreated( "SS", "NumSessionsCreated", "Number of editor sessions created", 0 );
CVariable<string> Scenario( "SS", "Scenario", "Scenario filename to use", "test", 0, true );
CVariable<uint>	CurrentEditor( "SS", "CurrentEditor", "Index of currently selected editor", 0 );
CVariable<uint> CurrentAct( "SS", "CurrentAct", "Index of act that current editor is running", 0 );
CVariable<uint>	SES( "SS", "SES", "SimEditorStatus", 0 );
CVariable<string> EditorState( "SS", "EditorState", "Current editor state", "" );
CVariable<string> Status( "SS", "Status", "Current status of simulation sequencing", "Uninitialized" );

// cfg file options
CVariable<uint>	NumClientsWanted( "SS", "NumClientsWanted", "Number of simulated clients to create", 0, 0, true );
CVariable<uint>	NumEditorsWanted( "SS", "NumEditorsWanted", "Number of simulated editors to create", 0, 0, true );
CVariable<bool>	SleepWhileWaiting( "SS", "SleepWhileWaiting", "Sleep client while waiting to join or connect", false, 0, true );

CVariable<bool>	WaitTilAllJoin( "SS", "WaitTilAllJoin", "Wait for all editors to join before continuing", false, 0, true );
CVariable<bool>	WaitTilAllLogin( "SS", "WaitTilAllLogin", "Wait for all editors to login before continuing", false, 0, true );
CVariable<bool>	WaitTilAllConnect( "SS", "WaitTilAllConnect", "Wait for all editors to connect before continuing", false, 0, true );

CVariable<bool>	AutoSimulate( "SS", "AutoSimulate", "Set to true to start state machine and scheduler automatically", false, 0, true );

CVariable<bool>	AutoStart( "SS", "AutoStart", "Set to true to start sim automatically", false, 0, true );
CVariable<bool>	AutoLogin( "SS", "AutoLogin", "Set to true to login to FES automatically", false, 0, true );
CVariable<bool>	AutoConnect( "SS", "AutoConnect", "Set to true to connect to DSS automatically", false, 0, true );
CVariable<bool>	AutoUpload( "SS", "AutoUpload", "Set to true to upload scenario rtdata automatically", false, 0, true );
CVariable<bool>	AutoRun( "SS", "AutoRun", "Set to true to start test run of scenario as animator automatically", false, 0, true );
CVariable<bool>	AutoNext( "SS", "AutoNext", "Set to true to change to next act of scenario automatically", false, 0, true );
CVariable<bool>	AutoEnd( "SS", "AutoEnd", "Set to true to end scenario and return to editor mode automatically", false, 0, true );

CVariable<bool>	UseScheduler( "SS", "UseScheduler", "Set to true to use scheduler", false, 0, true );

CVariable<float> MeanTimeBetweenLogins( "SS", "MeanTimeBetweenLogins", "Mean time between logins to FES for editors (in sec)", 2.0, 0, true );
CVariable<float> MeanTimeBetweenConnections( "SS", "MeanTimeBetweenConnections", "Mean time between connections to DSS for editors (in sec)", 2.0, 0, true );
CVariable<float> MeanTimeBetweenUploads( "SS", "MeanTimeBetweenUploads", "Mean time between uploads for editors (in sec)", 2.0, 0, true );

/*-----------------------------------------------------------------*\
						SERVICE CLASS
\*-----------------------------------------------------------------*/
CSimulationService::CSimulationService() :
	_NumClients( 0 ),
	_NumEditors( 0 ),
	_bConnected( false ),
	_bSimComplete( false ),
	_bDSSup( false ),
	_bAISup( false ),
	_ScenarioStub( NULL ),
	_ScenarioRtData( NULL ),
	_sss( sssUninitialized ),
	_minSes( sesUninitialized ),
	_StartTime( 0 )
{
	nlassert( !s_pSimService );
	s_pSimService = this;
	Status = "Uninitialized";
	EditorState = SimEditorStateNames[sssUninitialized];

	// initialize transition times and mean arrival times for scheduler
	_LastTransitionTimeToState.resize(sesLASTSTATE);
	_MeanArrivalTimeAtState.resize(sesLASTSTATE);
	
	TLocalTime currentTime = GetCurrentTime();
	for( SimEditorState ses = sesUninitialized; ses < sesLASTSTATE; ses = (SimEditorState)(ses+1) )
	{
		_LastTransitionTimeToState[ses] = currentTime;
		_MeanArrivalTimeAtState[ses] = 0;
	}

	myRandSeed(314159);
}

CSimulationService::~CSimulationService()
{
//	release();
}


//---------------------------------------------------
// Service Init :
// 
//---------------------------------------------------
void CSimulationService::init()
{
	// DEBUG: must call this before running console helper handy command pdrBin2Xml
	CSheetId::init();

	// load config file 
	ClientCfg.init( "r2_shard/cfg/simulation_service.cfg" );

	_MeanArrivalTimeAtState[sesLoggingIn] = (uint)MeanTimeBetweenLogins * 1000;
	_MeanArrivalTimeAtState[sesConnecting] = (uint)MeanTimeBetweenConnections * 1000;
	_MeanArrivalTimeAtState[sesLoadingScenario] = (uint)MeanTimeBetweenUploads * 1000;

//	uint32 updateTimeout = NLNET::IService::getInstance()->ConfigFile.getVar("UpdateTimeout").asInt();
//	setUpdateTimeout(updateTimeout);

	// initialize callbacks for impulse messages
	CSimulatedClient::initNetwork();
	CSimulatedEditor::initImpulseCallbacks();

	// create the sim clients and sim editors
	if( NumEditorsWanted )
	{
		_Editors.resize( NumEditorsWanted );
		for( uint i = 0; i < NumEditorsWanted; ++i )
		{
			_Editors[i] = new CSimulatedEditor(i);
		}
		_NumEditors = NumEditorsWanted;
		nlinfo( "%d sim editors created", _NumEditors );
	}

	if( NumClientsWanted )
	{
		_Clients.resize( NumClientsWanted );
		for( uint i = 0; i < NumClientsWanted; ++i )
		{
			_Clients[i] = new CSimulatedClient(i);
		}
		_NumClients = NumClientsWanted;
		nlinfo( "%d sim clients created", _NumClients );
	}

	// initialize callbacks for service up / down
	CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbServiceUp, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbServiceDown, NULL);

//	CMirrors::init(cbTick, NULL, cbTickRelease);

	_sss = sssInitialized;
	Status = "Initialized";

	EditorState = SimEditorStateNames[sssInitialized];
	
//	if( AutoStart )
//		start();
}

// connect sim clients to server
void CSimulationService::connectSimClients()
{
	if( !_NumClients )
		return;

	for( uint i = 0; i < _NumClients; ++i )
	{
		// create remote gateway and module on FES
		_Clients[i]->start();
		NumClientsConnected = NumClientsConnected + 1;
	}
	nlinfo( "%d sim clients connected", NumClientsConnected );
}

// this must be done before SEM receives onModuleUp from CEM
void CSimulationService::createEditorSessions()
{
	if( !_NumEditors )
		return;

	if( NumSessionsCreated )
	{
		nlwarning( "SS: Editor Sessions already created!" );
		return;
	}

	// get our Sim Ring Session manager
	IModuleManager &mm = IModuleManager::getInstance();
	IModule *myRSModule = mm.getLocalModule( "SimRingSessionManager" );
	CRingSessionManager *myRSM = safe_cast<RSMGR::CRingSessionManager *>(myRSModule);
	nlassert( myRSM );

	Status = "Creating sessions";
	for( uint i = 0; i < _NumEditors; ++i )
	{
		// have SimRSM create a session for each editor
		// this must be done before the SEM receives onModuleUp from the CEM,
		uint userId = UserId + _Editors[i]->id(); 
		uint sessionId = userId;
		myRSM->on_startSession( NULL, userId, sessionId );

		NumSessionsCreated = NumSessionsCreated + 1;
	}
	nlinfo( "%s sessions created", NumSessionsCreated.toString(true).c_str() );
	Status = "Sessions created";
}

void CSimulationService::connectSimEditors()
{
	if( !_NumEditors )
		return;

	Status = "Connecting editors";
	for( uint i = 0; i < _NumEditors; ++i )
	{
		uint userId = UserId + _Editors[i]->id(); 

//		_Editors[i]->start();
		NumEditorsConnected = NumEditorsConnected + 1;
	}
	nlinfo( "%s sim editors connected", NumEditorsConnected.toString(true).c_str() );
	Status = "Editors connected";
}

////////////////// state machines ////////////////////
//

void CSimulationService::onTick()
{
	if( _sss < sssInitialized )
		return;

	if( AutoSimulate )
		simulate();
}

// advance simulation state machine
bool CSimulationService::simulate()
{
	uint editorId = 0;
	SimEditorState newMinSes = sesLASTSTATE;

	switch( _sss )
	{
	case sssUninitialized:
		nlassert( false );
		break;
	case sssInitialized:
		_sss = sssAwaitingDSS;
		Status = "Awaiting DSS";
		break;
	case sssAwaitingDSS:
		if( _bDSSup )
		{
			_sss = sssStartingSimulation;
			Status = "Starting Simulation";
		}
		break;
	case sssStartingSimulation:
	case sssRunning:
		while ( editorId < _NumEditors )
		{
			SimEditorState ses = simulateEditor( editorId );
			if( newMinSes > ses )
				newMinSes = ses;

			// do a max of one login per tick
			if( ses == sesAwaitingLogin )
			{
				break;
			}
			// do a max of one connect per tick
			if( ses == sesAwaitingConnection )
			{
				break;
			}
			
			editorId++;
		}
		_minSes = newMinSes;

		//
		if( UseScheduler )
			processSchedule();

		if( _minSes >= sesScenarioUploaded )
		{
			_sss = sssRunning;
			Status = "Running Simulation";
		}
		break;
	case sssQuitting:
		// AJM TODO: quitting state
		nlassert( false );
		break;
	default:
		nlinfo( "state %d nop", _sss );
	}

	// update status variables
	nlassert( CurrentEditor < _NumEditors );
	CSimulatedEditor *ed = _Editors[CurrentEditor];
	nlassert( ed );

	SES = ed->getState();
	EditorState = SimEditorStateNames[SES];

	return true;
}

void CSimulationService::transition( CSimulatedEditor *ed, SimEditorState toState )
{
	NLMISC::TLocalTime currentTime = GetCurrentTime();
	if( _LastTransitionTimeToState[toState] < currentTime )
		_LastTransitionTimeToState[toState] = currentTime;
	ed->setState( toState );
}

// advance editor state machine
SimEditorState CSimulationService::simulateEditor( uint editorId )
{
//	nldebug("simulateEditor");
	
	nlassert( editorId < _NumEditors );
	CSimulatedEditor *ed = _Editors[editorId];
	nlassert( ed );

	SimEditorState ses = ed->getState();
	switch( ses )
	{
	case sesUninitialized:
		ed->init();
		break;
	case sesInitialized:
		if( AutoStart )
		{
			if( !_bStarted )
			{
				SimTime = 0;
				_bStarted = true;
			}
			transition( ed, sesStartingSession );
		}
		else
			Status = "Ready to start";
		break;
	case sesStartingSession:
		if( startSession0( editorId ) )
			transition( ed, sesAwaitingSessionStart );
		break;
	case sesAwaitingSessionStart:
		if( receivedSessionCreated.find( UserId + editorId ) != receivedSessionCreated.end() )
		{
			receivedSessionCreated.erase( UserId + editorId );
			transition( ed, sesSessionStarted );
		}
		break;
	case sesSessionStarted:
		if( joinSession0( editorId ) )
			transition( ed, sesSessionJoined );
		break;
	case sesSessionJoined:
		if( WaitTilAllJoin && (_minSes < sesSessionJoined) )
			break;
		if( !AutoLogin )
		{
			Status = "Ready to login";
			break;
		}
		if( !UseScheduler )
			transition( ed, sesLoggingIn );
		else 
		{
			transition( ed, sesScheduledForLogin );
			scheduleEvent( editorId, sesLoggingIn );
		}
		break;
	case sesScheduledForLogin:
		break;
	case sesLoggingIn:
		ed->login();
		transition( ed, sesAwaitingLogin );
		break;
	case sesAwaitingLogin:
		if( ed->isLoggedIn() )
		{
			nlinfo( "SimEditor %d is logged in to FES", editorId );
			transition( ed, sesLoggedIn );
		}
		else if( SleepWhileWaiting )
		{
			nlinfo( "SimEditor %d is sleeping to await login to FES", editorId );
			Sleep(10);
		}
		break;
	case sesLoggedIn:
		if( WaitTilAllLogin && (_minSes < sesLoggedIn) )
			break;
		if( !AutoConnect )
		{
			Status = "Ready to connect";
			break;
		}
		if( !UseScheduler )
			transition( ed, sesConnecting );
		else
		{
			transition( ed, sesScheduledForConnection );
			scheduleEvent( editorId, sesConnecting );
		}
		break;
	case sesScheduledForConnection:
		break;
	case sesConnecting:
		ed->connectToDSS();
		transition( ed, sesAwaitingConnection );
		break;
	case sesAwaitingConnection:
		if( ed->isConnected() )
		{
			nlinfo( "SimEditor %d is connected to DSS", editorId );
			NumEditorsConnected = NumEditorsConnected + 1;
			transition( ed, sesConnected );
		}
		else if( SleepWhileWaiting )
		{
			nlinfo( "SimEditor %d is sleeping to await connection to DSS", editorId );
			Sleep(10);
		}
		break;
	case sesConnected:
		if( WaitTilAllConnect && (_minSes < sesConnected) )
			break;
		if( !AutoUpload )
		{
			Status = "Ready to upload";
			break;
		}
		if( !UseScheduler )
			transition( ed, sesLoadingScenario );
		else 
		{
			transition( ed, sesScheduledForUpload );
			scheduleEvent( editorId, sesLoadingScenario );
		}
		break;
	case sesLoadingScenario:
		if( loadScenario0( editorId ) )
			transition( ed, sesUploadingScenario );
		else
		{
			nlwarning( "Failed to load scenario!" );
			delete _ScenarioStub;
			_ScenarioStub = NULL;
			delete _ScenarioRtData;
			_ScenarioRtData = NULL;
			AutoUpload.set( false );
			transition( ed, sesConnected );
		}
		break;
	case sesScheduledForUpload:
		break;
	case sesUploadingScenario:
		// AJM TODO: verify scenario actually uploaded
		if( uploadScenario0( editorId ) )
			transition( ed, sesScenarioUploaded );
		break;
	case sesScenarioUploaded:
		transition( ed, sesRunningAsEditor );
		break;
	case sesRunningAsEditor:
		CurrentAct = 1;
		if( !AutoRun )
		{
			Status = "Ready to run";
			break;
		}
		ed->runScenario();
		transition( ed, sesRunningAsAnimator );
		AutoEnd = 0;	// prevent cycling between run and end
		break;
	case sesRunningAsAnimator:
		if( NumActs > CurrentAct )
		{
			if( !AutoNext && !AutoEnd )
			{
				Status = "Ready for next act or end";
				break;
			}

			if( AutoNext )
			{
				CurrentAct = CurrentAct + 1;
				ed->startAct( CurrentAct );
				transition( ed, sesRunningAsAnimator );
			}
			else if( AutoEnd )
			{
				ed->endScenario();
				transition( ed, sesEndingScenario );
				AutoRun = 0;	// avoid cycling between run and end
			}
		}
		else if( !AutoEnd )
		{
			Status = "Ready to end";
		}
		else
		{
			ed->endScenario();
			transition( ed, sesEndingScenario );
			AutoRun = 0;	// avoid cycling between run and end
		}
		break;
	case sesEndingScenario:
		if( receivedStopTest.find( UserId + editorId ) != receivedStopTest.end() )
		{
			receivedStopTest.erase( UserId + editorId );
			transition( ed, sesRunningAsEditor );
		}
		break;
	case sesQuitting:
		// AJM TODO: quitting
		nlassert( false );
	default:
		nlinfo( "editor %d, state %d nop", editorId, ses );
	}

	// AJM TODO: update SimClients too in simulation mode
	if( ses >= sesLoggingIn )
		ed->update();

	return ed->getState();
}

// add a state transition event to the schedule
void CSimulationService::scheduleEvent( uint editorId, SimEditorState toState )
{
	SimScheduleEvent newEvent( editorId, toState );

	NLMISC::TLocalTime currentTime = GetCurrentTime();
	NLMISC::TLocalTime lastTime = _LastTransitionTimeToState[toState];
	NLMISC::TLocalTime meanTime = _MeanArrivalTimeAtState[toState];
	
	// interarrival time is exponentially distributed
	float exponTime = exponential();
	nldebug( "** scheduling event: current time: %d", (uint)GetCurrentTime() );
	NLMISC::TLocalTime interarrivalTime = meanTime * exponTime;
	NLMISC::TLocalTime scheduleTime = lastTime + interarrivalTime;
	if( scheduleTime < currentTime )
		scheduleTime = currentTime;

	nldebug( " editor %d will transition to %s at %d (in %d ms)", editorId, SimEditorStateNames[toState].c_str(), (uint)scheduleTime, (uint)(scheduleTime - currentTime) );
	nldebug( " mean time %d, exponential rand: %.2f, interarrival time: %d", (uint)meanTime, exponTime, (uint)interarrivalTime );
	nldebug( " last event scheduled at %d (%d ms ago)", (uint)lastTime, (uint)(currentTime - lastTime) );
	_Schedule.insert(SimScheduleEntryType(scheduleTime, newEvent));
	_LastTransitionTimeToState[toState] = scheduleTime;
}

// process the schedule, executing any events whose time has arrived
void CSimulationService::processSchedule()
{
	TLocalTime currentTime = GetCurrentTime();
/*
	static TLocalTime foo;
	if( currentTime - foo > 1000 )
	{
		foo = currentTime;
		nldebug( "current time: %d", (uint)GetCurrentTime() );
	}
*/
	SimScheduleType::iterator it = _Schedule.begin();
	SimScheduleType::iterator itEnd = _Schedule.end();

	while( it != itEnd )
	{
		if( it->first > currentTime )
			break;
		uint editorId = it->second.editorId;
		nlassert( editorId < _NumEditors );
		CSimulatedEditor *ed = _Editors[editorId];
		nlassert( ed );
		SimEditorState toState = it->second.toState;

		TLocalTime deltaTime = currentTime - it->first;
		nldebug( "** event firing: current time: %d", (uint)GetCurrentTime() );
		nldebug( " editor %d transitioning to %s, scheduled for %d (%d ms late)", ed->id(), SimEditorStateNames[toState].c_str(), (uint)it->first, (uint)deltaTime );
		transition( ed, toState );

		SimScheduleType::iterator itRemove = it++;
		_Schedule.erase( itRemove );
	}
}

// ask the RSM to startup an empty scenario for this user
// this must be done before SEM receives onModuleUp from CEM
bool CSimulationService::startSession0(  uint editorId )
{
	// can't proceed til DSS is started
	if( !_bDSSup )
		return false;

	IModuleManager &mm = IModuleManager::getInstance();
	IModule *myRSModule = mm.getLocalModule( "SimRingSessionManager" );
	CRingSessionManager *myRSM = safe_cast<RSMGR::CRingSessionManager *>(myRSModule);
	nlassert( myRSM );

//	Status = toString( "Starting session %d", editorId );
	uint userId = UserId + editorId; 
	uint sessionId = userId;
	myRSM->on_startSession( NULL, userId, sessionId );
	return true;
}

// ask the RSM to join this user's slot 0 character to his default scenario
// this must be done before SEM receives onModuleUp from CEM
// but after the sessionCreated message is received
bool CSimulationService::joinSession0( uint editorId )
{
	// get our Sim Ring Session manager
	IModuleManager &mm = IModuleManager::getInstance();
	IModule *myRSModule = mm.getLocalModule( "SimRingSessionManager" );
	CRingSessionManager *myRSM = safe_cast<RSMGR::CRingSessionManager *>(myRSModule);
	nlassert( myRSM );

//	Status = toString( "Joining session %d", editorId );
	uint userId = UserId + editorId;
	uint sessionId = userId;
	uint charId = userId<<4;

	myRSM->on_joinSession( NULL, charId, sessionId );
	return true;
}

// load scenario high-level data file from disk
//  build a scenario stub (description only)
// load scenario rtdata file from disk and check for validity
bool CSimulationService::loadScenario0( uint editorId )
{
	nlassert( editorId < _NumEditors );
	CSimulatedEditor *ed = _Editors[editorId];
	nlassert( ed );

	if( !_ScenarioStub )
	{
		_ScenarioStub = ed->loadScenarioStub( Scenario );
		if( !_ScenarioStub )
			return false;
	}
	if( !_ScenarioRtData )
	{
		_ScenarioRtData = ed->loadRtData( Scenario );
		if( !_ScenarioRtData )
			return false;
	}

	CurrentAct = 0;
	NumActs = ed->getNumActs( _ScenarioRtData );
	if( !NumActs )
	{
		nlwarning( "No acts in scenario rtdata!" );
		return false;
	}

	return true;
}

// upload scenario stub and rtdata to server
bool CSimulationService::uploadScenario0( uint editorId )
{
	nlassert( editorId < _NumEditors );
	CSimulatedEditor *ed = _Editors[editorId];
	nlassert( ed );

	nlassert( _ScenarioStub );
	nlassert( _ScenarioRtData );
	
	ed->uploadScenario( _ScenarioStub, _ScenarioRtData );

	return true;
}

// end scenario and return to edition mode
bool CSimulationService::endScenario0( uint editorId )
{
	nlassert( editorId < _NumEditors );
	CSimulatedEditor *ed = _Editors[editorId];
	nlassert( ed );

	ed->endScenario();

	return true;
}

//---------------------------------------------------
// Service update :
// 
//---------------------------------------------------
bool CSimulationService::update()
{
	if( _sss < sssInitialized )
		return false;

	if( _bStarted )
		SimTime = (uint)(GetCurrentTime() - _StartTime) / 1000;

	TTime t0 = CTime::getLocalTime();
	
	if( AutoSimulate )
		simulate();

//	TTime t0 = CTime::getLocalTime();

	if( _minSes >= sesLoggedIn )	// wait til logged into dss, then quit when all clients/editors disconnect
	{

		static uint nbQuit = 0;
		if( nbQuit >= (_NumClients+_NumEditors) )
			return false;

		// Update editors
		for( uint i = 0; i < _NumEditors; ++i )
		{
			CSimulatedEditor *pEditor = _Editors[i];
			CSimulatedClient::TLoginState loginStateBeforeUpdating = pEditor->getCurrentLoginState();
			bool posInitialized = pEditor->UserEntity.isInitialized();

			if ( loginStateBeforeUpdating < CSimulatedClient::LSQuitting )
			{
				if( !pEditor->update() )
					++nbQuit;
			}

			if( pEditor->getCurrentLoginState() != loginStateBeforeUpdating )
				nlinfo( "Editor %s to state %u", pEditor->name().c_str(), pEditor->getCurrentLoginState() );
			if( pEditor->UserEntity.isInitialized() != posInitialized )
				nlinfo( "Editor %s placed at %s", pEditor->name().c_str(), pEditor->UserEntity.pos().asVector().toString().c_str() );

			if( RequestQuit )
				pEditor->requestQuit();
		}

		// Update clients
		for( uint i = 0; i < _NumClients; ++i )
		{
			CSimulatedClient::TLoginState loginStateBeforeUpdating = _Clients[i]->getCurrentLoginState();
			bool posInitialized = _Clients[i]->UserEntity.isInitialized();

			if ( loginStateBeforeUpdating < CSimulatedClient::LSQuitting )
			{
				if( !_Clients[i]->update() )
					++nbQuit;
			}

			if( _Clients[i]->getCurrentLoginState() != loginStateBeforeUpdating )
				nlinfo( "Client %s to state %u", _Clients[i]->name().c_str(), _Clients[i]->getCurrentLoginState() );
			if( _Clients[i]->UserEntity.isInitialized() != posInitialized )
				nlinfo( "Client %s placed at %s", _Clients[i]->name().c_str(), _Clients[i]->UserEntity.pos().asVector().toString().c_str() );

			if( RequestQuit )
				_Clients[i]->requestQuit();
		}
	}
	
	TTime t1 = CTime::getLocalTime();
	TTime elapsed = t1 - t0;
	t0 = t1;
	if ( elapsed < UpdatePeriod )
	{
		nlSleep( (uint)(UpdatePeriod - elapsed) );
	}

	SleepTime = (int)(UpdatePeriod - elapsed);

	return true;
}

//---------------------------------------------------
// Service release :
// 
//---------------------------------------------------
void CSimulationService::release()	
{
	for( uint i = 0; i < _NumClients; ++i )
	{
		_Clients[i]->requestQuit();
		delete _Clients[i];
	}
	_Clients.clear();

	for( uint i = 0; i < _NumEditors; ++i )
	{
		_Editors[i]->requestQuit();
		delete _Editors[i];
	}
	_Editors.clear();

	delete _ScenarioStub;
	delete _ScenarioRtData;
}

// --------------------------------------------------

// debug test method
void CSimulationService::test()
{
	CSimulatedEditor *pFirstEditor = _Editors[0];
	if( !pFirstEditor )
	{
		nlwarning( "no editors running!" );
	}

	pFirstEditor->test();
}

void CSimulationService::start()
{
	AutoStart = true;
}

void CSimulationService::login()
{
	AutoLogin = true;
}

void CSimulationService::connect()
{
	AutoConnect = true;
}

void CSimulationService::upload()
{
	AutoUpload = true;
}

void CSimulationService::run()
{
	AutoRun = true;
}

void CSimulationService::next()
{
	AutoNext = true;
}

void CSimulationService::end()
{
	AutoEnd = true;
}


// create and upload scenario on current editor
void CSimulationService::createScenario( const std::string scenarioName )
{
	if( (CurrentEditor < 0) || (CurrentEditor >= NumEditorsConnected) )
	{
		nlwarning( "Editor %d not connected!", CurrentEditor );
	}
	CSimulatedEditor *pEditor = _Editors[CurrentEditor];
	nlassert( pEditor );
	
	pEditor->testCreateScenario( scenarioName );
}

// run scenario on current editor
void CSimulationService::runScenario()
{
	if( (CurrentEditor < 0) || (CurrentEditor >= NumEditorsConnected) )
	{
		nlwarning( "Editor %d not connected!", CurrentEditor );
	}
	CSimulatedEditor *pEditor = _Editors[CurrentEditor];
	nlassert( pEditor );
	
	pEditor->runScenario();
}

// end scenario on current editor
void CSimulationService::endScenario()
{
	if( (CurrentEditor < 0) || (CurrentEditor >= NumEditorsConnected) )
	{
		nlwarning( "Editor %d not connected!", CurrentEditor );
	}
	CSimulatedEditor *pEditor = _Editors[CurrentEditor];
	nlassert( pEditor );
	
	pEditor->endScenario();
}

void CSimulationService::joinSession( uint sessionId )
{
	if( (CurrentEditor < 0) || (CurrentEditor >= NumEditorsConnected) )
	{
		nlwarning( "Editor %d not connected!", CurrentEditor );
	}
	if( (sessionId < UserId) || (sessionId >= (UserId + NumSessionsCreated)) )
	{
		nlwarning( "Session %d not created!", sessionId );
	}

	// get our Sim Ring Session manager
	IModuleManager &mm = IModuleManager::getInstance();
	IModule *myRSModule = mm.getLocalModule( "SimRingSessionManager" );
	CRingSessionManager *myRSM = safe_cast<RSMGR::CRingSessionManager *>(myRSModule);
	nlassert( myRSM );

	CSimulatedEditor *pEditor = _Editors[CurrentEditor];
	nlassert( pEditor );
	uint currentUserId = UserId + pEditor->id();
	uint charId = currentUserId << 4;
	myRSM->on_joinSession( NULL, charId, sessionId );	// charId, sessionId
}

void CSimulationService::displayEditorStates()
{
	for( uint i = 0; i < _NumEditors; ++i )
	{
		nlinfo( "Editor %d (user %d): %s", i, UserId+i, toString(SimEditorStateNames[_Editors[i]->getState()]).c_str() );
	}

}

void CSimulationService::displaySchedule()
{
	SimScheduleType::iterator it = _Schedule.begin();
	SimScheduleType::iterator itEnd = _Schedule.end();
	TLocalTime currentTime = GetCurrentTime();
	while( it != itEnd )
	{
		nlinfo( "in %.2f sec: Editor %d transition to %s", 0.001f * (float)(it->first - currentTime), it->second.editorId, SimEditorStateNames[it->second.toState].c_str() );
		++it;
	}
}

/****************************************************************\
 ****************************************************************
						Callback functions
 ****************************************************************
\****************************************************************/
// Callback called at service connection
void cbServiceUp( const string &serviceName, uint16 serviceId, void * )
{
	nlinfo( "Service '%s' (%d) up", serviceName.c_str(), serviceId );

	if( !strcmp( serviceName.c_str(), "DSS" ) )
		CSimulationService::getSS().setDSSup( true );
	if( !strcmp( serviceName.c_str(), "AIS" ) )
		CSimulationService::getSS().setAISup( true );
}

// Callback called at service down
void cbServiceDown( const string &serviceName, uint16 serviceId, void * )
{
	nlinfo( "Service %s (%d) down", serviceName.c_str(), serviceId );

	if( !strcmp( serviceName.c_str(), "AIS" ) )
		CSimulationService::getSS().setAISup( false );
}

// Callback called on each tick message from tick service
void cbTick()
{
//	if( CMirrors::Mirror.mirrorIsReady() )
//	CSimulationService:;getSS().onTick();
}

void onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, NLNET::CMessage &message)
{
	nlassert( senderModuleProxy );
	std::string senderModuleName = senderModuleProxy->getModuleClassName();
	const char *szSenderName = senderModuleName.c_str();
	std::string operationName = message.getName();
	const char *szOpName = operationName.c_str();
	nlinfo( "CSimulationService: onProcessModuleMessage: %s", szOpName );
}

// simulated Web Interface callback
void CSimulationService::onInvokeResult( uint32 resultCode, const std::string &resultString )
{
	nldebug("SimRSMWeb: invokeResult = %d, %s", resultCode, resultString.c_str());

	const char *szResult = resultString.c_str();
	uint sessionId;

	const char szSessionCreated[] = "sessionCreated ";
	const char szStartSession[] = "startSession ";

	switch( resultCode )
	{
	case 0:	// success
		if( !strncmp( szResult, szSessionCreated, strlen(szSessionCreated) ) )
		{
			// session created successfully
			fromString( &szResult[strlen(szSessionCreated)], sessionId );
			if( receivedSessionCreated.find( sessionId ) == receivedSessionCreated.end() )
			{
				nlinfo( "SimRSMWeb: sessionCreated %d response received", sessionId);
				onReceiveSessionCreated( sessionId );
			}
			else
			{
				nlwarning( "sessionCreated message received twice for %d", sessionId );
			}
		}
		else if( !strncmp( szResult, szStartSession, strlen(szStartSession) ) )
		{
			// session started already
			fromString( &szResult[strlen(szStartSession)], sessionId );
			nlinfo( "SimRSMWeb: startSession %d response received", sessionId);
			onReceiveSessionCreated( sessionId );
		}
		break;
	default:
		break;
	}
}

void CSimulationService::onReceiveSessionCreated( uint sessionId )
{
	nlinfo( "onReceiveSessionCreated %d", sessionId );
	receivedSessionCreated.insert( sessionId );
}

void CSimulationService::onReceiveStopTest( uint editorId )
{
	nlinfo( "onReceiveStopTest %d", editorId );
	receivedStopTest.insert( UserId + editorId );
}

/****************************************************************\
 ****************************************************************
						Service register
 ****************************************************************
\****************************************************************/
NLNET_SERVICE_MAIN( CSimulationService, "SS", "simulation_service", 0, EmptyCallbackArray, "", "" )
// last two params are config file directory, log file directory

//-------------------------------------------------------------------------

NLMISC_COMMAND( test, "Test command", "" )
{
	CSimulationService::getSS().test();
	return true;
}

NLMISC_COMMAND( start, "Create editor sessions on server", "" )
{
	CSimulationService::getSS().start();
	return true;
}

NLMISC_COMMAND( login, "Login sim editors to frontend server", "" )
{
	CSimulationService::getSS().login();
	return true;
}

NLMISC_COMMAND( connect, "Connect sim editors to dynamic scenario server", "" )
{
	CSimulationService::getSS().connect();
	return true;
}

NLMISC_COMMAND( upload, "Upload sim editor scenarios to dynamic scenario server", "" )
{
	CSimulationService::getSS().upload();
	return true;
}

NLMISC_COMMAND( run, "Request sim editors to start test", "" )
{
	CSimulationService::getSS().run();
	return true;
}

NLMISC_COMMAND( next, "Request sim editors to start next act", "" )
{
	CSimulationService::getSS().run();
	return true;
}

NLMISC_COMMAND( end, "Request sim editors to stop test", "" )
{
	CSimulationService::getSS().end();
	return true;
}


NLMISC_COMMAND( createScenario, "Load, create and upload a scenario on the current editor", "<filename>" )
{
	if( !args.empty() )
		Scenario = args[0];
	
	CSimulationService::getSS().createScenario( Scenario );
	return true;
}

NLMISC_COMMAND( runScenario, "Run the scenario on the current editor", "" )
{
	CSimulationService::getSS().runScenario();
	return true;
}

NLMISC_COMMAND( endScenario, "Ending the scenario on the current editor", "" )
{
	CSimulationService::getSS().endScenario();
	return true;
}

NLMISC_COMMAND( joinSession, "Join the given session with the current editor", "<sessionId>" )
{
	uint sessionId = UserId;
	if( !args.empty() )
		NLMISC::fromString(args[0], sessionId);
	if( sessionId >= (UserId + NumSessionsCreated) )
	{
		nlinfo( "Session %d not running!", sessionId );
		return false;
	}
	CSimulationService::getSS().joinSession( sessionId );
	return true;
}

NLMISC_COMMAND( displayEditorStates, "Display the current state of each simulated editor", "" )
{
	CSimulationService::getSS().displayEditorStates();
	return true;
}

NLMISC_COMMAND( displaySchedule, "Display the current schedule of simulated editor events", "" )
{
	CSimulationService::getSS().displaySchedule();
	return true;
}
