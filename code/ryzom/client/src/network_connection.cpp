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

#ifdef NL_OS_WINDOWS
#include <process.h>
#endif

#include "network_connection.h"
#include "client_cfg.h"

#include "game_share/action_factory.h"
#include "game_share/action.h"
#include "game_share/action_disconnection.h"
#include "game_share/action_position.h"
#include "game_share/action_sync.h"
#include "game_share/action_association.h"
#include "game_share/action_dummy.h"
#include "game_share/action_login.h"
#include "game_share/action_sint64.h"
#include "game_share/action_target_slot.h"
#include "game_share/action_generic.h"
#include "game_share/action_generic_multi_part.h"
#include "game_share/mode_and_behaviour.h"

#include "game_share/simlag.h"

#include "nel/misc/cdb.h"
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/cdb_branch.h"
#include "cdb_synchronised.h"

#include "nel/misc/variable.h"
#include "nel/misc/algo.h"
#include "nel/3d/u_driver.h"

#include "game_share/system_message.h"

#include "game_share/entity_types.h" // required for ifdef

#include "graph.h"

#include "global.h"
#include "far_tp.h"

#ifdef DISPLAY_ENTITIES
#include "../../../test/network/sb5000/client/graph.h"
#endif


// ***************************************************************************
// Smooth ServerTick setup
// The numner of Smooth Tick per Server Tick
#define SMOOTH_SERVER_TICK_PER_TICK		100
// The possible not corrected error between actual ServerTick and estimated one (NB: equal to packet sent frequency)
#define SMOOTH_SERVER_TICK_WINDOW		100
// The min Difference between the estimated and the actual one. If <=, the estimation is reseted.
#define SMOOTH_SERVER_TICK_DIFF_MIN		-1000
// The max Difference between the estimated and the actual one. If >=, clamp
#define SMOOTH_SERVER_TICK_DIFF_MAX		500
// When the estimation is late, the Max Acceleration factor
#define SMOOTH_SERVER_TICK_ACCEL		4.0f


const char * ConnectionStateCStr [9] = { "NotInitialised", "NotConnected", "Authenticate", "Login", "Synchronize", "Connected", "Probe", "Stalled", "Disconnect" };


// ***************************************************************************
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CLFECOMMON;


#undef MEASURE_RECEIVE_DATES
//#define SHOW_PROPERTIES_RECEIVED

#ifdef MEASURE_RECEIVE_DATES
#include <nel/misc/config_file.h>
#include <nel/misc/displayer.h>
#include <nel/misc/log.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/command.h>
// Stat: array of vectors of cycles when a pos is received, indexed by TCLEntityId
struct TRDateState
{
	TRDateState( TGameCycle gc, TGameCycle pdit, TTime ct ) : ServerCycle(gc), PredictedInterval(pdit), LocalTime(ct) {}

	TGameCycle	ServerCycle, PredictedInterval;
	TTicks		LocalTime;
};


extern CLFECOMMON::TCLEntityId WatchedEntitySlot;

////////////
// GLOBAL //
////////////


typedef vector<TRDateState> TReceiveDateLog;
TReceiveDateLog				ReceivePosDateLog [256];
bool						LogReceiveEnabled = false;
CConfigFile					NCConfigFile;
CFileDisplayer				*ReceiveLogDisp;
CLog						ReceiveLogger;


/*
 * initReceiveLog
 */
void			initReceiveLog()
{
	try
	{
		ReceiveLogDisp = new CFileDisplayer( getLogDirectory() + "ReceiveLog.log" );
		ReceiveLogger.addDisplayer( ReceiveLogDisp );
		//ReceiveLogger.displayNL( "Searching for param LogReceive in the config file..." );
		NCConfigFile.load( string( "client.cfg" ) );
		int slot = NCConfigFile.getVar( "LogReceive" ).asInt();
		if ( slot != 0 )
		{
			LogReceiveEnabled = true;
			ReceiveLogger.displayNL( "LogReceive is on" ); // only when enabled
		}
	}
	catch (const EConfigFile&)
	{}
}

/*
 * startReceiveLog (the slots logged are all if no selection, or only the selected slots (one at a time)
 */
void			startReceiveLog()
{
	sint i;
	for ( i=0; i!=256; ++i )
	{
		ReceivePosDateLog[i].clear();
	}

	LogReceiveEnabled = true;
}

/*
 * stopReceiveLog
 */
void			stopReceiveLog()
{
	LogReceiveEnabled = false;
}

/*
 * displayReceiveLog
 */
void			displayReceiveLog()
{
	if ( ! LogReceiveEnabled )
		return;

	nlinfo( "Dumping ReceiveLog" );
	ReceiveLogger.displayNL( "ReceiveLog (ServerCycle, PredictedInterval, LocalTime(ms)):" );

	// Display receive dates for all slots for which vector is not empty
	// (allows to trace several selected slots one after one, or all slots at the same time)
	sint i;
	for ( i=0; i!=256; ++i )
	{
		if ( ! ReceivePosDateLog[i].empty() )
		{
			ReceiveLogger.displayRawNL( "Entity %d: %u updates", i, ReceivePosDateLog[i].size() );
			TReceiveDateLog::iterator idl;
			for ( idl=ReceivePosDateLog[i].begin(); idl!=ReceivePosDateLog[i].end(); ++idl )
			{
				ReceiveLogger.displayRawNL( "%u\t%u\t%"NL_I64"u", (*idl).ServerCycle, (*idl).PredictedInterval, (*idl).LocalTime );
			}
		}
	}
	ReceiveLogger.displayRawNL( "ReceiveLog completed" );
}

NLMISC_COMMAND( startReceiveLog, "Starts logging the position receives (for all or only the watched entities when selected)", "" )
{
	nlinfo( "Starting ReceiveLog" );
	startReceiveLog();
	return true;
}

NLMISC_COMMAND( stopReceiveLog, "Stops logging the position receives", "" )
{
	stopReceiveLog();
	return true;
}

NLMISC_COMMAND( displayReceiveLog, "Flush the receive log into ReceiveLog.log", "" )
{
	displayReceiveLog();
	return true;
}

#endif // MEASURE_RECEIVE_DATES

extern NL3D::UDriver		*Driver;


CVariable<bool>	CheckXMLSignature("client", "CheckXMLSignature", "enable client to check msg/database.xml signature", true, 0, true);

CSlotGraph *PosUpdateIntervalGraph = NULL;
CSlotGraph *PosUpdatePredictionGraph = NULL;

CGraph MsPerTickGraph  ("mspertick (ms)",  10.0f, 570.0f, 400.0f, 100.0f, CRGBA(0,0,128,128), 1000, 400.0f, 2);
CGraph PingGraph       ("ping (ms)",       10.0f, 460.0f, 400.0f, 100.0f, CRGBA(128,0,0,128), 100000, 1000.0f, 2);
CGraph PacketLossGraph ("PacketLoss (pc)", 10.0f, 350.0f, 200.0f, 100.0f, CRGBA(0,128,0,128), 100000, 100.0f, 2);
CGraph UploadGraph     ("upload (byps)",   10.0f, 240.0f, 200.0f, 100.0f, CRGBA(0,128,128,128), 1000, 3000.0f, 2);
CGraph DownloadGraph   ("download (byps)", 10.0f, 130.0f, 200.0f, 100.0f, CRGBA(0,0,128,128), 1000, 3000.0f, 2);


//#define A a()					!!!
//#define B b()					!!! SO DANGEROUS !!!
//#define Parent parent()		!!!


bool			CNetworkConnection::_Registered = false;


/*
 * Percentile (in fact, quantile) function
 * Freely adapted from the Goose Library (http://www.gnu.org/software/goose)
 * at http://cvs.gnome.org/lxr/source/goose/src/stats/descriptive.cpp#90
 */

// Normal quantile function (see also percentilRev below)
/*float percentile( const multiset<uint32>& dataset, float p )
{
	//nlassert( ! dataset.empty() );
	//nlassert( (p >= 0) && (p <= 1) );
	uint ds = dataset.size();
	if ( ds == 1 )
		return (float)(*dataset.begin());
	float fpIndex = p * (float)(ds-1);
	sint ipIndex = (sint)fpIndex;
	multiset<uint32>::iterator it = dataset.begin(), itnext;
	for ( sint i=0; i!=ipIndex; ++i, ++it );
	itnext = it; ++itnext;
	return ((float)(ipIndex+1)-fpIndex)*(float)(*it) + (fpIndex-(float)ipIndex)*(float)(*itnext);
}*/

// Reversed quantile function = percentile(dataset, 1-p) (optimized for p>0.5 i.e. rp<=0.5)
float percentileRev( const multiset<uint32>& dataset, float rp )
{
	//nlassert( ! dataset.empty() );
	//nlassert( (rp >= 0) && (rp <= 1) );
	uint ds = (uint)dataset.size();
	if ( ds == 1 )
		return (float)(*dataset.begin());
	float fpIndex = rp * (float)(ds-1);
	sint ipIndex = (sint)fpIndex;
	multiset<uint32>::const_reverse_iterator it = dataset.rbegin(), itnext;
	for ( sint i=0; i!=ipIndex; ++i, ++it );
	itnext = it; ++itnext;
	return ((float)(ipIndex+1)-fpIndex)*(float)(*it) + (fpIndex-(float)ipIndex)*(float)(*itnext);
}


//

const uint MAX_POSUPDATETICKQUEUE_SIZE = 25;
const float PREDICTION_REV_PERCENTILE = 0.2f; // (1 - 0.8)

//

bool CNetworkConnection::LoggingMode = false;


/*
 * Constructor
 */
#ifdef ENABLE_INCOMING_MSG_RECORDER
CNetworkConnection::CNetworkConnection() : _NextMessageToReplay(true)
#else
CNetworkConnection::CNetworkConnection()
#endif
{
	_ConnectionState = NotInitialised;
	_ImpulseCallback = NULL;
	_ImpulseArg = NULL;
	_DataBase = NULL;

	reset();

	_QuitId = 0;

	initTicks();
}


/*
 * Destructor
 */
CNetworkConnection::~CNetworkConnection()
{
#ifdef MEASURE_RECEIVE_DATES
	delete ReceiveLogDisp;
#endif
	if ( _VisualPropertyTreeRoot ) // not allocated in local mode
	{
		_VisualPropertyTreeRoot->deleteBranches();
		delete _VisualPropertyTreeRoot;
		_VisualPropertyTreeRoot = NULL;
	}

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( _RecordIncomingMessagesOn )
		_RecordedMessagesOut.close();
	else if ( _ReplayIncomingMessagesOn )
		_RecordedMessagesIn.close();
#endif
}

NLMISC::CHashKeyMD5	getTextMD5(const std::string& filename)
{
	CIFile	fi;
	if (!fi.open(CPath::lookup(filename, false)))
		return NLMISC::CHashKeyMD5();

	std::vector<uint8>	buffer(fi.getFileSize());
	fi.serialBuffer(&(buffer[0]), (uint)buffer.size());

	std::vector<uint8>::iterator	it = buffer.begin();
	do
	{
		while (it != buffer.end() && *it != '\015')
			++it;

		if (it != buffer.end())
			it = buffer.erase(it);
	}
	while (it != buffer.end());

	return NLMISC::getMD5((&buffer[0]), (uint32)buffer.size());
}

/*
 * Init
 */
void	CNetworkConnection::init(const string &cookie, const string &addr)
{
	if (_ConnectionState != NotInitialised &&
		_ConnectionState != Disconnect)
	{
		nlwarning("Unable to init(): connection not properly closed yet.");
		return;
	}

	if (!_Registered)
	{
		CActionFactory::getInstance ()->registerAction (ACTION_POSITION_CODE, CActionPosition::create);
		CActionFactory::getInstance ()->registerAction (ACTION_SYNC_CODE, CActionSync::create);
		CActionFactory::getInstance ()->registerAction (ACTION_DISCONNECTION_CODE, CActionDisconnection::create);
		CActionFactory::getInstance ()->registerAction (ACTION_ASSOCIATION_CODE, CActionAssociation::create);
		CActionFactory::getInstance ()->registerAction (ACTION_DUMMY_CODE, CActionDummy::create);
		CActionFactory::getInstance ()->registerAction (ACTION_LOGIN_CODE, CActionLogin::create);
		CActionFactory::getInstance ()->registerAction (ACTION_TARGET_SLOT_CODE, CActionTargetSlot::create);
		CActionFactory::getInstance ()->registerAction (ACTION_GENERIC_CODE, CActionGeneric::create);
		CActionFactory::getInstance ()->registerAction (ACTION_GENERIC_MULTI_PART_CODE, CActionGenericMultiPart::create);
		CActionFactory::getInstance ()->registerAction (ACTION_SINT64, CActionSint64::create);
		_Registered = true;
	}

	initCookie(cookie, addr);

#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
	nlinfo( "Half-frequency mode" );
#else
	nlinfo( "Full-frequency mode" );
#endif

#ifdef MEASURE_RECEIVE_DATES
	initReceiveLog();
#endif

	// Register property nbbits
	CActionSint64::registerNumericPropertiesRyzom();

	// Init visual property tree
	_VisualPropertyTreeRoot = new TVPNodeClient();
	_VisualPropertyTreeRoot->buildTree();

#ifdef ENABLE_INCOMING_MSG_RECORDER
	_RecordIncomingMessagesOn = false;
	_ReplayIncomingMessagesOn = false;
	_NextClientTickToReplay = 0;
#endif

	// If the server run on window, those are the one to test
	_AltMsgXmlMD5 = NLMISC::getMD5("msg.xml");
	_AltDatabaseXmlMD5 = NLMISC::getMD5("database.xml");
	// If the server run on UNIX, those are the one to test
	_MsgXmlMD5 = getTextMD5("msg.xml");
	_DatabaseXmlMD5 = getTextMD5("database.xml");
}


/*
 * Sets the cookie and front-end address, resets the connection state.
 */
void	CNetworkConnection::initCookie(const string &cookie, const string &addr)
{
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ! ClientCfg.Local )
#endif
	{
		// set values of simulation for the udp socket
		CUdpSimSock::setSimValues(ClientCfg.ConfigFile);

		_FrontendAddress = addr;

		if (!cookie.empty ())
			_LoginCookie.setFromString (cookie);
		else if (ClientCfg.ConfigFile.exists ("UserId"))
		{
			uint32 uid = ClientCfg.ConfigFile.getVar ("UserId").asInt();
			_LoginCookie.set(0, 0, uid);
			Cookie = _LoginCookie.toString(); // to be able to do '/reconnect'
			nlinfo ("Set the cookie with the UserId %d set in config file", uid);
		}

		nlinfo ("Network initialisation with front end '%s' and cookie %s",_FrontendAddress.c_str(), _LoginCookie.toString().c_str ());
	}

	_ConnectionState = NotConnected;
}




namespace CLFECOMMON
{
	// Factory for TVPNodeBase::buildTree()
	TVPNodeBase	*NewNode()
	{
		return (TVPNodeBase*) new CNetworkConnection::TVPNodeClient();
	}
};


#ifdef ENABLE_INCOMING_MSG_RECORDER
/*
 * Start/stop recording the incoming messages (in online mode)
 */
void	CNetworkConnection::setRecordingMode( bool onOff, const std::string& filename )
{
	nlassert( ! ClientCfg.Local );
	if ( onOff )
	{
		if ( ! _RecordIncomingMessagesOn )
		{
			if ( _RecordedMessagesOut.open( filename ) )
			{
				nlinfo( "Beginning recording to %s", filename.c_str() );
				_RecordedMessagesOut.serialEnum( _ConnectionState );
				_RecordedMessagesOut.serial( _CurrentReceivedNumber );
				_RecordedMessagesOut.serial( _LastReceivedNumber );
				_RecordedMessagesOut.serial( _LastAckInLongAck );
				_RecordIncomingMessagesOn = true;
			}
			else
			{
				nlwarning( "Cannot open %s for recording", filename.c_str() );
			}
		}
	}
	else
	{
		if ( _RecordIncomingMessagesOn )
			_RecordedMessagesOut.close();
		_RecordIncomingMessagesOn = false;
	}
}


/*
 * Start/stop replaying the incoming messages (in offline mode)
 */
void	CNetworkConnection::setReplayingMode( bool onOff, const std::string& filename )
{
	nlassert( ClientCfg.Local );
	if ( onOff )
	{
		if ( ! _ReplayIncomingMessagesOn )
		{
			if ( _RecordedMessagesIn.open( filename ) )
			{
				nlinfo( "Beginning replaying of %s", filename.c_str() );
				_RecordedMessagesIn.serialEnum( _ConnectionState );
				_RecordedMessagesIn.serial( _CurrentReceivedNumber );
				_RecordedMessagesIn.serial( _LastReceivedNumber );
				_RecordedMessagesIn.serial( _LastAckInLongAck );
				// Preload first message
				if ( _RecordedMessagesIn.eof() )
				{
					// Nothing to load
					nlinfo( "Nothing to replay" );
					_RecordedMessagesIn.close();
					return;
				}
				else
				{
					_RecordedMessagesIn.serial( _NextClientTickToReplay );
					_RecordedMessagesIn.serialMemStream( _NextMessageToReplay );
					_CurrentClientTick = _NextClientTickToReplay;
					_CurrentServerTick = _CurrentClientTick + 10;
					nlinfo( "Setting initial replay tick: %u", _CurrentClientTick );
					_ReplayIncomingMessagesOn = true;
				}
			}
			else
			{
				nlwarning( "File %s for replay not found", filename.c_str() );
			}
		}
	}
	else
	{
		if ( _RecordIncomingMessagesOn )
			_RecordedMessagesIn.close();
		_ReplayIncomingMessagesOn = false;
	}
}
#endif


bool	CNetworkConnection::connect(string &result)
{
	if (_ConnectionState != NotConnected)
	{
		nlwarning("Unable to connect(): connection not properly initialised (maybe connection not closed).");
		return false;
	}

	// try to find where common data are located depending to where we'll connect
	// the goal is to use the same database.txt as the server one
	try
	{
		CConfigFile cfg;
		cfg.load (CPath::lookup("shards.cfg"));

		// do this process only if Use == 1
		if (cfg.getVar("Use").asInt() == 1)
		{
			CInetAddress fsaddr (_FrontendAddress);

			fsaddr.setPort(0);

			nlinfo ("Try to find a shard that have fsaddr = '%s'", fsaddr.asString().c_str());

			CConfigFile::CVar &shards = cfg.getVar("Shards");
			CInetAddress net;
			uint i;
			for (i = 0; i < shards.size(); i+=2)
			{
				try
				{
					net.setNameAndPort (shards.asString(i));
					nlinfo ("testAddr = '%s'", net.asString().c_str());
					if (net == fsaddr)
					{
						// ok, we found it, now overwrite files

						string srcPath = CPath::standardizeDosPath(CPath::getFullPath(shards.asString(i+1)));

						nlinfo ("srcPath = '%s'", srcPath.c_str());

						CConfigFile::CVar &needToCopy = cfg.getVar("NeedToCopy");
						for (uint j = 0; j < needToCopy.size(); j++)
						{
							string arg1 = srcPath + needToCopy.asString(j);
							string dstPath = CPath::standardizeDosPath(CPath::getFullPath(CPath::lookup (CFile::getFilename (needToCopy.asString(j))), false));
							try
							{
								if(arg1.empty () || dstPath.empty ())
								{
									nlinfo ("Can't copy, src or dst is empty");
								}
								if(arg1 != dstPath)
								{
									nlinfo ("Copying '%s' into '%s'", arg1.c_str(), dstPath.c_str());
									nlinfo ("executing 'copy /Y %s %s", arg1.c_str(), dstPath.c_str());
									string str = "copy /Y " + arg1 + " " + dstPath;
									int ret = system (str.c_str ());
									//int ret = _spawnlp (_P_WAIT, "copy", "copy", "/Y", arg1.c_str(), dstPath.c_str(), NULL);
									if (ret != 0)
									{
										nlwarning ("the copy command seems failed with the error code %d, errno %d: %s", ret, errno, strerror(errno));
									}
								}
								else
								{
									nlinfo ("Can't copy, same path '%s'", arg1.c_str());
								}
							}
							catch (const Exception &)
							{
								nlwarning ("Can't copy '%s' '%s', try the next file", arg1.c_str(), dstPath.c_str());
							}
						}
						break;
					}
				}
				catch (const Exception &e)
				{
					nlwarning (e.what ());
				}
			}
			if (i == shards.size())
			{
				nlwarning ("the fsaddr '%s' is not in the shards.cfg, can't copy data_common files", fsaddr.asString().c_str());
			}
		}
	}
	catch (const Exception &)
	{
		nlinfo ("There's no shards.cfg, or bad file format, can't copy common files");
	}

	nlinfo ("CNET[%p]: Connecting to '%s' with cookie '%s'", this, _FrontendAddress.c_str(), _LoginCookie.toString().c_str ());

	// then connect to the frontend using the udp sock
//ace faut faire la nouveau login client 	result = CLoginClient::connectToShard (_FrontendAddress, _Connection);

	nlassert (!_Connection.connected());

	try
	{
		//
		// S12: connect to the FES. Note: In UDP mode, it's the user that have to send the cookie to the front end
		//
		_Connection.connect (CInetAddress(_FrontendAddress));
	}
	catch (const ESocket &e)
	{
		result = toString ("FS refused the connection (%s)", e.what());
		return false;
	}

	_ConnectionState = Login;

	_LatestLoginTime = ryzomGetLocalTime ();
	_LatestSyncTime = _LatestLoginTime;
	_LatestProbeTime = _LatestLoginTime;

	nlinfo("CNET[%p]: Client connected to shard, attempting login", this);
	return true;
}

void	CNetworkConnection::setImpulseCallback(TImpulseCallback callback, void *argument)
{
	_ImpulseCallback = callback;
	_ImpulseArg = argument;
}

//

bool	CNetworkConnection::isConnected()
{
	return _ConnectionState == Connected;
}

#ifdef ENABLE_INCOMING_MSG_RECORDER
/*
 * Return true if there is some messages to replay (in replay mode)
 */
bool	CNetworkConnection::dataToReplayAvailable()
{
	// Test the next tick loaded with the curent client tick (set externally)
	//nldebug( "current=%u nextToReplay=%u", _CurrentClientTick, _NextClientTickToReplay );
	return ( _CurrentClientTick >= _NextClientTickToReplay ); // when true => authorize entering buildStream()...
}
#endif



/*
 * Set MsPerTick value
 */
void	CNetworkConnection::setMsPerTick(sint32 msPerTick)
{
	_MsPerTick = msPerTick;
}



//
//
//

bool	CNetworkConnection::update()
{
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( _NextClientTickToReplay == std::numeric_limits<uint32>::max() )
	{
		setReplayingMode( false );
		return false;
	}
#endif

	_UpdateTime = ryzomGetLocalTime ();
	_UpdateTicks = ryzomGetPerformanceTime();
	_ReceivedSync = false;
	_NormalPacketsReceived = 0;
	_TotalMessages = 0;

	//nldebug("CNET[%d]: begin update()", this);

	// If we are disconnected, bypass the real network update
	if ( _ConnectionState == Disconnect )
	{
		_ConnectionQuality = false; // to block the user entity
		return false;
	}

	// Yoyo. Update the Smooth ServerTick.
	updateSmoothServerTick();


	////// MEASURE_FE_SENDING (special test mode, not used in normal mode)
#ifdef MEASURE_FE_SENDING
	if ( _ConnectionState == Login )
	{
		//_Connection.setNonBlockingMode( true );
		sendSystemLogin();
		_ConnectionState = Connected;
	}

	// Receive
	CBitMemStream msgin( true );
	bool res = buildStream( msgin );

	if ( res )
	{
		static sint32 loopcount = 0;
		++loopcount;
		static TTicks lastdisplay = CTime::getPerformanceTime();
		TTicks tn = CTime::getPerformanceTime();
		TTime diff = CTime::ticksToSecond(tn - lastdisplay) * 1000.0;
		if ( diff > 2000 )
		{
			nlinfo("Reads by second: %.1f => LoopTime = %.2f ms LoopCount = %u Diff = %u ms",(float)loopcount * 1000.0f / (float)diff, (float)diff / loopcount, loopcount, diff);
			loopcount = 0;
			lastdisplay = tn;
		}
	}

	return res;
#endif

	if (!_Connection.connected())
	{
		//if(!ClientCfg.Local)
		//	nlwarning("CNET[%p]: update() attempted whereas socket is not connected !", this);
		return false;
	}

	try
	{
		// State automaton
		bool	stateBroke = false;
		do
		{
			switch (_ConnectionState)
			{
			case Login:
				// if receives System SYNC
				//    immediate state Synchronize
				// else
				//    sends System LoginCookie
				stateBroke = stateLogin();
				break;

			case Synchronize:
				// if receives System PROBE
				//    immediate state Probe
				// else if receives Normal
				//    immediate state Connected
				// else
				//    sends System ACK_SYNC
				stateBroke = stateSynchronize();
				break;

			case Connected:
				// if receives System PROBE
				//    immediate state Probe
				// else if receives Normal
				//	   sends Normal data
				stateBroke = stateConnected();
				break;

			case Probe:
				// if receives System SYNC
				//    immediate state SYNC
				// else if receives System PROBE
				//    decode PROBE
				// sends System ACK_PROBE
				stateBroke = stateProbe();
				break;

			case Stalled:
				// if receives System SYNC
				//    immediate state SYNC
				// else if receives System STALLED
				//    decode STALLED (nothing to do)
				// else if receives System PROBE
				//    immediate state PROBE
				stateBroke = stateStalled();
				break;

			case Quit:
				// if receives System SYNC
				//    immediate state Synchronize
				// else
				//    sends System LoginCookie
				stateBroke = stateQuit();
				break;

			default:
				// Nothing here !
				stateBroke = false; // will come here if a disconnection action is received inside a method that returns true
				break;
			}
		}
		while (stateBroke);// && _TotalMessages<5);
	}
	catch (const ESocket &)
	{
		_ConnectionState = Disconnect;
	}

	//updateBufferizedPackets ();

	PacketLossGraph.addOneValue (getMeanPacketLoss ());

	_ConnectionQuality = (getConnectionState() == Connected &&
						  _UpdateTime - _LastReceivedNormalTime < 2000 && _CurrentClientTick < _CurrentServerTick);

	return (_TotalMessages!=0);
}


/*
 * Receive available data and convert it to a bitmemstream
 */
bool	CNetworkConnection::buildStream( CBitMemStream &msgin )
{
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( _ReplayIncomingMessagesOn )
	{
		// Replay message
		statsReceive( _NextMessageToReplay.length() );
		msgin.clear();
		memcpy( msgin.bufferToFill( _NextMessageToReplay.length() ), _NextMessageToReplay.buffer(), _NextMessageToReplay.length() );
		//nldebug( "Reading message for tick %u (size %u)", _NextClientTickToReplay, msgin.length() );

		// Preload next message
		if ( _RecordedMessagesIn.eof() )
		{
			// Nothing more to load
			_NextClientTickToReplay = std::numeric_limits<uint32>::max();
			nlinfo( "Nothing more to replay, end of replaying" );
		}
		else
		{
			_RecordedMessagesIn.serial( _NextClientTickToReplay );
			_NextMessageToReplay.clear();
			_RecordedMessagesIn.serialMemStream( _NextMessageToReplay );
		}

		return true;
	}
#endif

	uint32 len = 65536;
	if ( _Connection.receive( (uint8*)_ReceiveBuffer, len, false ) )
	{
		// Compute some statistics
		statsReceive( len );

		// Fill the message
		msgin.clear();
		memcpy( msgin.bufferToFill( len ), _ReceiveBuffer, len );

#ifdef ENABLE_INCOMING_MSG_RECORDER
		if ( _RecordIncomingMessagesOn )
		{
			_RecordedMessagesOut.serial( _CurrentClientTick );
			_RecordedMessagesOut.serialMemStream( msgin ); // shouldn't msgin's bufpos be resetted?
		}
#endif
		return true;
	}
	else
	{
		// A receiving error means the front-end is down
		_ConnectionState = Disconnect;
		disconnect(); // won't send a disconnection msg because state is already Disconnect
		nlwarning( "DISCONNECTION" );
		return false;
	}
}




//
// Client automaton states methods
//




//
// Login state
//


// sends system login cookie
void	CNetworkConnection::sendSystemLogin()
{
	CBitMemStream	message;

	buildSystemHeader(message);

	uint8			login = SYSTEM_LOGIN_CODE;
	message.serial(login);

	if (_LoginCookie.isValid())
		message.serial(_LoginCookie);
	else
	{
		uint32	fakeCookie = 0xDEADBEEF;
		message.serial(fakeCookie);
		message.serial(fakeCookie);
		message.serial(fakeCookie);
	}
	message.serial( ClientCfg.LanguageCode );

	// Try to send login, and handle the case when a firewall blocks the sending
	uint32 length = message.length();
	static TTime attemptStartTime = CTime::getLocalTime();
	try
	{
		//sendUDP (&(_Connection), message.buffer(), length);
		_Connection.send( message.buffer(), length );
	}
	catch (const ESocket &e)
	{
#ifdef NL_OS_WINDOWS
		// An exception (10004: Blocking operation interrupted) may occur if a firewall such as Kerio is
		// running (note: ZoneAlarm blocks connect() until a decision is made by the user).
		// Handle true network errors with a nlerror dialog box
		if ( string(e.what()).find( "10004:" ) == string::npos ) // Code of WSAEINTR
		{
			// nlerror( "Cannot login: %s", e.what() );
		}
#endif
		// The first time, display info for the user to configure his personal firewall
		static bool exceptionThrown = false;
		if ( ! exceptionThrown )
		{
			exceptionThrown = true;
			throw EBlockedByFirewall();
		}

		// Next time, disconnect if the time-out expired
		//nldebug( "Attempt interrupted at %u ms", (uint32)(CTime::getLocalTime()-attemptStartTime) );
		TTime currentTime = CTime::getLocalTime();
		if ( currentTime - attemptStartTime > 15000 ) // let 15 seconds for the user to allow the connection
		{
			nldebug( "Login failed at %u ms", (uint32)(CTime::getLocalTime()-attemptStartTime) );
			//nlerror( "Cannot login (check your firewall's settings?): %s", e.what() );
			throw EBlockedByFirewall();
		}
	}

	statsSend( length );
	nlinfo( "CNET[%p]: sent LOGIN cookie=%s", this, _LoginCookie.toString().c_str() );
	//nlinfo( "CNET[%p]: sent LOGIN cookie=%s at attempt %u at %u ms", this, _LoginCookie.toString().c_str(), nbAttempts, (uint32)(CTime::getLocalTime()-attemptStartTime) );
}

bool	CNetworkConnection::stateLogin()
{
	// if receives System SYNC
	//    immediate state Synchronize
	// else
	//    sends System LoginCookie

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ClientCfg.Local && !_ReplayIncomingMessagesOn )
		return false;
	while ( (_ReplayIncomingMessagesOn && dataToReplayAvailable()) ||
			_Connection.dataAvailable() )
#else
	while ( _Connection.dataAvailable() )// && _TotalMessages<5)
#endif
	{
		_DecodedHeader = false;
		CBitMemStream	msgin (true);
		if (buildStream(msgin) && decodeHeader(msgin))
		{
			if (_SystemMode)
			{
				uint8	message = 0;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync
					_ConnectionState = Synchronize;
					nldebug("CNET[%p]:  login->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Stalled;
					nldebug("CNET[%p]: login->stalled", this);
					receiveSystemStalled(msgin);
					return true;
					break;
				case SYSTEM_PROBE_CODE:
					// receive probe, decode probe and state probe
					_ConnectionState = Probe;
					_Changes.push_back(CChange(0, ProbeReceived));
					nldebug("CNET[%p]: login->probe", this);
					receiveSystemProbe(msgin);
					return true;
					break;
				case SYSTEM_SERVER_DOWN_CODE:
					disconnect(); // will send disconnection message
					nlwarning( "BACK-END DOWN" );
					return false; // exit now from loop, don't expect a new state
					break;
				default:
					//msgin.displayStream("DBG:BEN:stateLogin:msgin");
					nlwarning("CNET[%p]: received system %d in state Login", this, message);
					break;
				}
			}
			else
			{
				//msgin.displayStream("DBG:BEN:stateLogin:msgin");
				nlwarning("CNET[%p]: received normal in state Login", this);
			}
		}
	}

	// send ack sync if received sync or last sync timed out
	if (_UpdateTime-_LatestLoginTime > 300)
	{
		sendSystemLogin();
		_LatestLoginTime = _UpdateTime;
	}

	return false;
}



//
// Sync state
//

void	CNetworkConnection::receiveSystemSync(CBitMemStream &msgin)
{
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( _ReplayIncomingMessagesOn )
	{
		TGameCycle dummyTick;
		TTime dummyTime;
		msgin.serial( dummyTick );
		msgin.serial( dummyTime );
		return;
	}
#endif

	_LatestSyncTime = _UpdateTime;
	TTime		stime;
	msgin.serial(_Synchronize);
	msgin.serial(stime);
	msgin.serial(_LatestSync);

	if (CheckXMLSignature.get())
	{
		bool	xmlInvalid = false;

		CHashKeyMD5	checkMsgXml;
		CHashKeyMD5	checkDatabaseXml;

		try
		{
			msgin.serialBuffer(checkMsgXml.Data, sizeof(checkMsgXml.Data));
			msgin.serialBuffer(checkDatabaseXml.Data, sizeof(checkDatabaseXml.Data));

			// Since cannot now easily if the server run on Windows or unix, try the both methods
			xmlInvalid = (checkMsgXml != _MsgXmlMD5 || checkDatabaseXml != _DatabaseXmlMD5);
			if(xmlInvalid)
				xmlInvalid = (checkMsgXml != _AltMsgXmlMD5 || checkDatabaseXml != _AltDatabaseXmlMD5);
		}
		catch (const NLMISC::Exception&)
		{
		}

		static bool	alreadyWarned = false;
		if (xmlInvalid && !alreadyWarned)
		{
			alreadyWarned = true;
			Driver->systemMessageBox("msg.xml and database.xml files are invalid (server version signature is different)", "XML files invalid");

			nlwarning("XML signature is invalid:");
			nlwarning("msg.xml client:%s,%s server:%s", _AltMsgXmlMD5.toString().c_str(), _MsgXmlMD5.toString().c_str(),
				checkMsgXml.toString().c_str());
			nlwarning("database.xml client:%s,%s server:%s", _AltDatabaseXmlMD5.toString().c_str(), _DatabaseXmlMD5.toString().c_str(),
				checkDatabaseXml.toString().c_str());
		}
	}

	_ReceivedSync = true;

	setMsPerTick(100);
	//_MsPerTick = 100;			// initial values

#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
//#pragma message ("HALF_FREQUENCY_SENDING_TO_CLIENT")
	_CurrentServerTick = _Synchronize+_CurrentReceivedNumber*2;
#else
//#pragma message ("FULL_FREQUENCY_SENDING_TO_CLIENT")
	_CurrentServerTick = _Synchronize+_CurrentReceivedNumber;
#endif
	_CurrentClientTick = uint32(_CurrentServerTick - (_LCT+_MsPerTick)/_MsPerTick);
	_CurrentClientTime = _UpdateTime - (_LCT+_MsPerTick);

	//nlinfo( "CNET[%p]: received SYNC %"NL_I64"u %"NL_I64"u - _CurrentReceivedNumber=%d _CurrentServerTick=%d", this, (uint64)_Synchronize, (uint64)stime, _CurrentReceivedNumber, _CurrentServerTick );

	sendSystemAckSync();
}

// sends system sync acknowledge
void	CNetworkConnection::sendSystemAckSync()
{
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( _ReplayIncomingMessagesOn )
		return;
#endif

	CBitMemStream	message;

	buildSystemHeader(message);

	uint8			sync = SYSTEM_ACK_SYNC_CODE;
	message.serial(sync);
	message.serial(_LastReceivedNumber);
	message.serial(_LastAckInLongAck);
	message.serial(_LongAckBitField);
	message.serial(_LatestSync);

	uint32	length = message.length();
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

	_LatestSyncTime = _UpdateTime;

	//nlinfo("CNET[%p]: sent ACK_SYNC, _LastReceivedNumber=%d _LastAckInLongAck=%d", this, _LastReceivedNumber, _LastAckInLongAck);

/*	// display long ack
	uint	i;
	uint	bfsize = _LongAckBitField.size();
	uint	bbuffer = 0;
	string	buffer;
	static const char	htable[] = "0123456789ABCDEF";
	for (i=0; i<bfsize; ++i)
	{
		if (i>0 && (i&3)==0)
		{
			buffer += htable[bbuffer];
			bbuffer = 0;
		}

		bbuffer = bbuffer*2 + (_LongAckBitField.get((_LastReceivedNumber-i)&(bfsize-1)) ? 1 : 0);
	}

	buffer += htable[bbuffer];
	nlinfo("CNET[%p]: ACK=%s", buffer.c_str());
*/}

bool	CNetworkConnection::stateSynchronize()
{
	// if receives System PROBE
	//    immediate state Probe
	// else if receives Normal
	//    immediate state Connected
	// sends System ACK_SYNC
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ClientCfg.Local && !_ReplayIncomingMessagesOn )
		return false;
	while ( (_ReplayIncomingMessagesOn && dataToReplayAvailable()) ||
			_Connection.dataAvailable() )
#else
	while (_Connection.dataAvailable())// && _TotalMessages<5)
#endif
	{
		_DecodedHeader = false;
		CBitMemStream	msgin (true);
		if (buildStream(msgin) && decodeHeader(msgin))
		{
			if (_SystemMode)
			{
				uint8	message = 0;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_PROBE_CODE:
					// receive probe, decode probe and state probe
					_ConnectionState = Probe;
					//nldebug("CNET[%p]: synchronize->probe", this);
					_Changes.push_back(CChange(0, ProbeReceived));
					receiveSystemProbe(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Stalled;
					//nldebug("CNET[%p]: synchronize->stalled", this);
					receiveSystemStalled(msgin);
					return true;
					break;
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync
					receiveSystemSync(msgin);
					break;
				case SYSTEM_SERVER_DOWN_CODE:
					disconnect(); // will send disconnection message
					nlwarning( "BACK-END DOWN" );
					return false; // exit now from loop, don't expect a new state
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Synchronize", this, message);
					break;
				}
			}
			else
			{
				_ConnectionState = Connected;
				//nlwarning("CNET[%p]: synchronize->connected", this);
				_Changes.push_back(CChange(0, ConnectionReady));
				_ImpulseDecoder.reset();
				receiveNormalMessage(msgin);
				return true;
			}
		}
	}

	// send ack sync if received sync or last sync timed out
	if (_UpdateTime-_LatestSyncTime > 300)
		sendSystemAckSync();

	return false;
}


#ifdef SHOW_PROPERTIES_RECEIVED
uint8 propReceived [18];
#endif
#ifdef MEASURE_RECEIVE_DATES
TTime currentTime;
#endif

//
// Connected state
//

void	CNetworkConnection::receiveNormalMessage(CBitMemStream &msgin)
{
	//nlinfo("CNET[%p]: received normal message Packet=%d Ack=%d", this, _LastReceivedNumber, _LastReceivedAck);

	vector<CAction *> actions;
	_ImpulseDecoder.decode(msgin, _CurrentReceivedNumber, _LastReceivedAck, _CurrentSendNumber, actions);
#ifdef SHOW_PROPERTIES_RECEIVED
	for ( uint8 p=0; p!=18; ++p )
		propReceived[p] = 0;
	propReceived[2] = actions.size();
#endif

	++_NormalPacketsReceived;


	// we can now remove all old action that are acked
	while (!_Actions.empty() && _Actions.front().FirstPacket != 0 && _Actions.front().FirstPacket <= _LastReceivedAck)
	{
		// warning, CActionBlock automatically remove() actions when deleted
		_Actions.pop_front();
	}

	// now, read actions
	/*vector<CAction *> commonActions;
	CActionFactory::getInstance()->unpack (msgin, commonActions, getCurrentServerTick()+1 ); // +1 because the current tick is set a few lines further

	actions.insert(actions.end(), commonActions.begin(), commonActions.end());*/

	//_PropertyDecoder.ack(_LastReceivedNumber, _LastReceivedAck);


	//
	// update game time and ticks from network infos
	//

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ! _ReplayIncomingMessagesOn )
#endif
	{
		// convert the number of the packet that we just received into tick
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
		nlassert(_CurrentReceivedNumber*2+_Synchronize > _CurrentServerTick);
		_CurrentServerTick = _CurrentReceivedNumber*2+_Synchronize;
#else
		nlassert(_CurrentReceivedNumber+_Synchronize > _CurrentServerTick);
		_CurrentServerTick = _CurrentReceivedNumber+_Synchronize;
#endif
		//nldebug( "_CurrentServerTick=%d _CurrentReceivedNumber=%d _Synchronize=%d", _CurrentServerTick, _CurrentReceivedNumber, _Synchronize );
	}

	// remove useless stamps in queue
	while (!_PacketStamps.empty() && _LastReceivedAck > _PacketStamps.front().first)
		_PacketStamps.pop_front();

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( (! _ReplayIncomingMessagesOn) && (_PacketStamps.empty() || _PacketStamps.front().first > _LastReceivedAck) )
#else
	if (_PacketStamps.empty() || _PacketStamps.front().first > _LastReceivedAck)
#endif
	{
		//nlwarning("Frontend ack'ed message %d not stamp dated", _LastReceivedAck);
	}
	else
	{
		// get the send time of the acknowledged packet
		TTime	ackedPacketTime = _PacketStamps.front().second;

		// update ping
		uint32	ping = (uint32)(_UpdateTime-ackedPacketTime);
		_InstantPing = ping;
		if (ping < _BestPing)
			_BestPing = ping;

		PingGraph.addOneValue (float(ping));

		// earliest estimation of server packet send time and latest estimation (based on ping and acknowledge)
		TTime	earliest = ackedPacketTime + _BestPing/2,
				latest = _UpdateTime - _BestPing/2;

		// compute number of ticks between frame currently played by client and packet received from server
		sint32	numStepTick = (sint32)(_CurrentServerTick-_CurrentClientTick);

		// if enough steps and times are valid
		if (numStepTick > 0 && earliest > _CurrentClientTime && latest > _CurrentClientTime)
		{
			// exact formula for _MsPerTick = (_CurrentServerTime-_CurrentClientTime)/numStepTick
			// where _CurrentServerTime is the actual server packet send time
			// but as the exact time is unknown, we use a predictive time window instead, based on
			// the acknwoledged packet send time, the received packet time and the ping

			// adjust if estimation of _MsPerTick is too small
			if ((TTime)(_CurrentClientTime+_MsPerTick*numStepTick) < earliest)
				setMsPerTick((sint32)(earliest-_CurrentClientTime)/numStepTick);
				//_MsPerTick = (sint32)(earliest-_CurrentClientTime)/numStepTick;

			// adjust if estimation of _MsPerTick is too large
			if ((TTime)(_CurrentClientTime+_MsPerTick*numStepTick) > latest)
				setMsPerTick((sint32)(latest-_CurrentClientTime)/numStepTick);
				//_MsPerTick = (sint32)(latest-_CurrentClientTime)/numStepTick;

			// _MsPerTick should be positive here -- seems to crash yet
			/// \todo we should instead of putting 1, returning in probe mode because it means that we had a very big lag
			if (_MsPerTick == 0)
			{
				nlwarning ("_MsPerTick is 0 because server tick is too big %d compare to the client tick is %d", _CurrentServerTick, _CurrentClientTick);
				setMsPerTick(1);
				//_MsPerTick = 1;
			}
		}
		else if (numStepTick <= 0)
		{
			setMsPerTick((sint32)_LCT);
			//_MsPerTick = (sint32)_LCT;
		}
		MsPerTickGraph.addOneValue (float(_MsPerTick));

	}

#ifdef MEASURE_RECEIVE_DATES
	currentTime = ryzomGetLocalTime ();
#endif

	// Decode the actions received in the impulsions
	uint i;
	for (i = 0; i < actions.size (); i++)
	{
		switch (actions[i]->Code)
		{

		case ACTION_DISCONNECTION_CODE:
			{
				// Self disconnection
				nlwarning( "You were disconnected by the server" );
				disconnect(); // will send disconnection message
				LoginSM.pushEvent( CLoginStateMachine::ev_conn_dropped );
			}
			break;
		case ACTION_GENERIC_CODE:
			{
				genericAction((CActionGeneric *)actions[i]);
			}
			break;
		case ACTION_GENERIC_MULTI_PART_CODE:
			{
				genericAction((CActionGenericMultiPart *)actions[i]);
			}
			break;
		case ACTION_DUMMY_CODE:
			{
				CActionDummy	*dummy = ((CActionDummy*)actions[i]);
				nldebug("CNET[%d] Received Dummy %d", this, dummy->Dummy1);
				// Nothing to do
			}
			break;
		}

		CActionFactory::getInstance()->remove(actions[i]);
	}


	// Decode the visual properties
	decodeVisualProperties( msgin );

	_LastReceivedNormalTime = _UpdateTime;

#ifdef DISPLAY_ENTITIES
	DownloadGraph.addValue ((float)(msgin.length()));
	DpfGraph.addValue ((float)(msgin.length()));
#endif
#ifdef SHOW_PROPERTIES_RECEIVED

	string str = "Received: ";
//	stringstream ss;
//	ss << "Received: ";
	if ( propReceived[2] != 0 )
		str += NLMISC::toString(propReceived[2]) + " impuls. ";
//		ss << propReceived[2] << " impuls. ";
	if ( propReceived[0] != 0 )
		str += NLMISC::toString(propReceived[0]) + " pos; ";
//	ss << propReceived[0] << " pos; ";
	if ( propReceived[3] != 0 )
		str += NLMISC::toString(propReceived[3]) + " orient; ";
//	ss << propReceived[3] << " orient; ";
	uint sum = propReceived[4] + propReceived[5] + propReceived[6] + propReceived[7] + propReceived[8] + propReceived[9];
	if ( sum != 0 )
		str += NLMISC::toString(sum) + " discreet; ";
//	ss << sum << " discreet; ";
	if ( propReceived[16] != 0 )
		str += NLMISC::toString(propReceived[16]) + "assoc; ";
//	ss << propReceived[16] << "assoc; ";
	if ( propReceived[17] != 0 )
		str += NLMISC::toString(propReceived[17]) + "disac; ";
//	ss << propReceived[17] << "disac; ";
	str += "TOTAL: " + NLMISC::toString(propReceived[2]) + " + " + NLMISC::toString(propReceived[0] + propReceived[3] + sum);
	//ss << "TOTAL: " << propReceived[2] << " + " << propReceived[0] + propReceived[3] + sum;
	nlwarning( "%s", str.c_str() );
#endif
}


void	CNetworkConnection::decodeVisualProperties( CBitMemStream& msgin )
{
	try
	{
		//nldebug( "pos: %d  len: %u", msgin.getPos(), msgin.length() );
		while ( true )
		{
			//nlinfo( "Reading pass %u, BEFORE HEADER: pos: %d  len: %u", ++i, msgin.getPosInBit(), msgin.length() * 8 );

			// Check if there is a new block to read
			if ( msgin.getPosInBit() + (sizeof(TCLEntityId)*8) > msgin.length()*8 )
				return;



			// Header
			TCLEntityId slot;
			msgin.serialAndLog1( slot );

			uint32 associationBits;
			msgin.serialAndLog2( associationBits, 2 );
			//nlinfo( "slot %hu AB: %u", (uint16)slot, associationBits );
			if ( associationBitsHaveChanged( slot, associationBits ) &&  (!IgnoreEntityDbUpdates || slot==0))
			{
			   //displayBitStream( msgin, beginbitpos, msgin.getPosInBit() );
//			   nlinfo ("Disassociating S%hu (AB %u)", (uint16)slot, associationBits );
				if ( _PropertyDecoder.isUsed( slot ) )
				{
					TSheetId sheet = _PropertyDecoder.sheet( slot );
					TIdMap::iterator it = _IdMap.find( sheet );
					if ( it != _IdMap.end() )
						_IdMap.erase(it);
					_PropertyDecoder.removeEntity( slot );

					CChange theChange( slot, RemoveOldEntity );
					_Changes.push_back( theChange );
				}
				else
				{
//					nlinfo( "Cannot disassociate slot %hu: sheet not received yet", (uint16)slot );
				}
			}

			// Read the timestamp delta if there's one (otherwise take _CurrentServerTick)
			TGameCycle timestamp;
			bool timestampIsThere;
			msgin.serialBitAndLog( timestampIsThere );
			if ( timestampIsThere )
			{
				uint32 timestampDelta;
				msgin.serialAndLog2( timestampDelta, 4 );
				timestamp = _CurrentServerTick - timestampDelta;
				//nldebug( "TD: %u (S%hu)", timestampDelta, (uint16)slot );
			}
			else
			{
				timestamp = _CurrentServerTick;
			}

			// Tree
			//nlinfo( "AFTER HEADER: posBit: %d pos: %d  len: %u", msgin.getPosInBit(), msgin.getPos(), msgin.length() );

			TVPNodeClient *currentNode = _VisualPropertyTreeRoot;
			msgin.serialBitAndLog( currentNode->a()->BranchHasPayload );
			if ( currentNode->a()->BranchHasPayload )
			{
				CActionPosition *ap = (CActionPosition*)CActionFactory::getInstance()->create( slot, ACTION_POSITION_CODE );
				ap->unpack( msgin );
				_PropertyDecoder.receive( _CurrentReceivedNumber, ap );
#ifdef SHOW_PROPERTIES_RECEIVED
				++propReceived[PROPERTY_POSITION];
#endif

				/*
				 * Set into property database
				 */

				// TEMP
				if ( ap->Position[0]==0 || ap->Position[1]==0 )
					nlwarning( "S%hu: Receiving an invalid position", (uint16)slot );

				if (_DataBase != NULL &&  (!IgnoreEntityDbUpdates || slot==0))
				{
					CCDBNodeBranch	*nodeRoot;
					nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode((uint16)0));
					if(nodeRoot)
					{
						CCDBNodeLeaf	*node;
						node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode(0));
						nlassert(node != NULL);
						node->setValue64(ap->Position[0]);
						node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode(1));
						nlassert(node != NULL);
						node->setValue64(ap->Position[1]);
						node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode(2));
						nlassert(node != NULL);
						node->setValue64(ap->Position[2]);

						if ( LoggingMode )
						{
							nlinfo( "recvd position (%d,%d) for slot %hu, date %u", (sint32)(ap->Position[0]), (sint32)(ap->Position[1]), (uint16)slot, timestamp );
						}
					}
				}

				bool interior = ap->Interior;

				CActionFactory::getInstance()->remove( (CAction*&)ap );


				/*
				 * Statistical prediction of time before next position update: set PredictedInterval
				 */

				//nlassert( MAX_POSUPDATETICKQUEUE_SIZE > 1 );
				deque<TGameCycle>& puTicks = _PosUpdateTicks[slot];
				multiset<TGameCycle>& puIntervals = _PosUpdateIntervals[slot];

				// Flush the old element of tick queue and of the interval sorted set
				if ( puTicks.size() == MAX_POSUPDATETICKQUEUE_SIZE )
				{
					puIntervals.erase( puIntervals.find( puTicks[1] - puTicks[0] ) ); // erase only one element, not all corresponding to the value
					puTicks.pop_front();
				}

				// Add a new element to the tick queue and possibly to the interval sorted set
				// Still to choose: _CurrentServerTick or timestamp ?
				TGameCycle latestInterval = 0;
				if ( ! puTicks.empty() )
				{
					latestInterval =  timestamp - puTicks.back();
					puIntervals.insert( latestInterval );

					if ( PosUpdateIntervalGraph )
						PosUpdateIntervalGraph->addOneValue( slot, (float)latestInterval );
				}
				puTicks.push_back( timestamp );

				nlassert( puTicks.size() == puIntervals.size()+1 );

				// Prediction function : Percentile(25 last, 0.8) + 1
				TGameCycle predictedInterval;
				if ( puIntervals.empty() )
				{
					predictedInterval = 0;
				}
				else
				{
					predictedInterval = (TGameCycle)(percentileRev( puIntervals, PREDICTION_REV_PERCENTILE ) + 1);

					//if ( predictedInterval > 100 )
					//	nlwarning( "Slot %hu: Predicted interval %u exceeds 100 ticks", (uint16)slot, predictedInterval );

					if ( PosUpdatePredictionGraph )
						PosUpdatePredictionGraph->addOneValue( slot, (float)predictedInterval );
				}

				//nlinfo( "Slot %hu: Interval=%u Predicted=%u", (uint16)slot, latestInterval, predictedInterval );

				/*
				 * Add into the changes vector
				 */
				CChange thechange( slot, PROPERTY_POSITION, timestamp );
				thechange.PositionInfo.PredictedInterval = predictedInterval;
				thechange.PositionInfo.IsInterior = interior;
				_Changes.push_back( thechange );

#ifdef MEASURE_RECEIVE_DATES
				// Stat log
				if ( LogReceiveEnabled && (WatchedEntitySlot == 256) || (WatchedEntitySlot == slot) )
				{
					TRDateState ds( timestamp, predictedInterval, currentTime );
					ReceivePosDateLog[slot].push_back( ds );
				}
#endif

			}

			currentNode = currentNode->b();
			msgin.serialBitAndLog( currentNode->BranchHasPayload );
			if ( currentNode->BranchHasPayload )
			{
				msgin.serialBitAndLog( currentNode->a()->BranchHasPayload );
				if ( currentNode->a()->BranchHasPayload )
				{
					CActionSint64 *ac = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( slot, PROPERTY_ORIENTATION );
					ac->unpack( msgin );

					// Process orientation
					CChange thechange(slot, PROPERTY_ORIENTATION, timestamp);
					_Changes.push_back( thechange );
	#ifdef SHOW_PROPERTIES_RECEIVED
					++propReceived[PROPERTY_ORIENTATION];
	#endif
					if (_DataBase != NULL &&  (!IgnoreEntityDbUpdates || slot==0))
					{
						CCDBNodeBranch	*nodeRoot;
						nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode(0));
						if ( nodeRoot )
						{
							CCDBNodeLeaf *node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode( PROPERTY_ORIENTATION ));
							nlassert(node != NULL);
							node->setValue64(ac->getValue());
							if ( LoggingMode )
							{
								nlinfo( "CLIENT: recvd property %hu (%s) for slot %hu, date %u", (uint16)PROPERTY_ORIENTATION, getPropText(PROPERTY_ORIENTATION), (uint16)slot, timestamp );
							}
							//nldebug("CLPROPNET[%p]: received property %d for entity %d: %"NL_I64"u", this, action->PropIndex, action->CLEntityId, action->getValue());
						}
					}

					CActionFactory::getInstance()->remove( (CAction*&)ac );
				}

				TVPNodeClient::SlotContext.NetworkConnection = this;
				TVPNodeClient::SlotContext.Slot = slot;
				TVPNodeClient::SlotContext.Timestamp = timestamp;

				// Discreet properties
				currentNode->b()->decodeDiscreetProperties( msgin );
			}
		}
	}
	catch (const EStreamOverflow&)
	{
		// End of stream (saves useless bits)
	}
}

/*
 *
 */

static vector<TCLEntityId>	TargetSlotsList(256);

void	CNetworkConnection::decodeDiscreetProperty( CBitMemStream& msgin, TPropIndex propIndex )
{
	//nldebug( "Reading discreet property %hu at bitpos %d", (uint16)propIndex, msgin.getPosInBit() );

	TCLEntityId slot = TVPNodeClient::SlotContext.Slot;

	// \todo BEN this is temp, put it somewhere in database
	if (propIndex == PROPERTY_TARGET_LIST)
	{
		uint8	listSize;
		msgin.serial(listSize);

		TargetSlotsList.resize(listSize);
		if (listSize > 0)
			msgin.serialBuffer(&(TargetSlotsList[0]), listSize);

		// Set target list value in database
		if (_DataBase != NULL && (!IgnoreEntityDbUpdates || slot==0))
		{
			CCDBNodeBranch	*nodeRoot;
			nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode(0));
			if ( nodeRoot )
			{
				CCDBNodeBranch *nodeEntity = dynamic_cast<CCDBNodeBranch*>(nodeRoot->getNode(slot));
				nlassert(nodeEntity != NULL);

				uint	writeProp = PROPERTY_TARGET_LIST;
				uint	place = 0;

				if (listSize >= 32)
				{
					listSize = 32;
				}
				else
				{
					TargetSlotsList.push_back(INVALID_SLOT);
					++listSize;
				}

				CCDBNodeLeaf *nodeProp = NULL;

				uint	i;
				uint64	value = 0;
				for (i=0; i<listSize; ++i)
				{
					if (place == 0)
						value = 0;

					value += (((uint64)TargetSlotsList[i]) << (place*8));

					++place;
					if (place == 8)
					{
						nodeProp = dynamic_cast<CCDBNodeLeaf*>(nodeEntity->getNode(writeProp));
						nlassert(nodeProp != NULL);
						nodeProp->setValue64(value);
						++writeProp;
						place = 0;
					}
				}

				if (place != 0)
				{
					nodeProp = dynamic_cast<CCDBNodeLeaf*>(nodeEntity->getNode(writeProp));
					nlassert(nodeProp != NULL);
					nodeProp->setValue64(value);
				}
			}

			if ( LoggingMode )
			{
				nlinfo( "CLIENT: recvd property %hu (%s) for slot %hu, date %u", (uint16)propIndex, getPropText(propIndex), (uint16)slot, TVPNodeClient::SlotContext.Timestamp );
			}
		}

		CChange thechange( slot, propIndex, TVPNodeClient::SlotContext.Timestamp );
		_Changes.push_back( thechange );

		return;
	}

	CActionSint64 *ac = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( slot, propIndex );
	ac->unpack( msgin );

#ifdef SHOW_PROPERTIES_RECEIVED
		++propReceived[propIndex];
#endif

	switch ( propIndex )
	{
		case PROPERTY_SHEET:
		{
			// Special case for sheet
//			nlinfo ("Associating S%hu", (uint16)slot );
			if ( _PropertyDecoder.isUsed( slot ) )
			{
				TSheetId sheet = _PropertyDecoder.sheet( slot );
				TIdMap::iterator it = _IdMap.find(sheet);
				if ( it != _IdMap.end() )
					_IdMap.erase(it);
				_PropertyDecoder.removeEntity( slot );
			}

			TSheetId newSheetId = (TSheetId)(ac->getValue() & 0xffffffff);
			_IdMap.insert( make_pair( newSheetId, slot) );
			_PropertyDecoder.addEntity( slot, newSheetId );

			// Reset the position update statistical data
			_PosUpdateTicks[slot].clear();
			_PosUpdateIntervals[slot].clear();

			// Read optional alias block
			uint32 alias = 0;
			bool aliasBit = false;
			msgin.serialBitAndLog( aliasBit );
			if ( aliasBit )
				msgin.serialAndLog1( alias );

			// Set information
			CChange thechange( slot, AddNewEntity );
			thechange.NewEntityInfo.DataSetIndex = (TClientDataSetIndex)((ac->getValue() >> 32) & 0xffffffff);
			thechange.NewEntityInfo.Alias = alias;
			_Changes.push_back( thechange );
			break;
		}
		case PROPERTY_MODE:
		{
			// Special case for mode: push theta or pos, then mode
			uint64 mode44 = ac->getValue();
			uint32 modeTimestamp = _CurrentServerTick - (uint32)(((mode44 >> 8) & 0xF));

			// Push the mode Before the position or the orientation
			CChange thechangeMode( slot, PROPERTY_MODE, modeTimestamp );
			_Changes.push_back( thechangeMode );

			// Set mode value in database
			if (_DataBase != NULL &&  (!IgnoreEntityDbUpdates || slot==0))
			{
				CCDBNodeBranch	*nodeRoot;
				nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode(0));
				if ( nodeRoot )
				{
					CCDBNodeLeaf *node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode( propIndex ));
					nlassert(node != NULL);
					node->setValue64(ac->getValue() & 0xFF); // (8 bits)
					if ( LoggingMode )
					{
						nlinfo( "CLIENT: recvd property %hu (%s) for slot %hu, date %u", (uint16)propIndex, getPropText(propIndex), (uint16)slot, TVPNodeClient::SlotContext.Timestamp );
					}
				}
			}

			// Set the position or orientation received along with the mode in the database
			uint8 modeEnum = (uint8)(mode44 & 0xFF);
			if ( modeEnum == MBEHAV::COMBAT_FLOAT )
			{
				// Set theta
				if (_DataBase != NULL &&  (!IgnoreEntityDbUpdates || slot==0))
				{
					CCDBNodeBranch	*nodeRoot;
					nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode(0));
					if ( nodeRoot )
					{
						CCDBNodeLeaf *node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode( PROPERTY_ORIENTATION ));
						nlassert(node != NULL);
						node->setValue64( (mode44 >> 12) /*&& 0xFFFFFFFF*/ );
					}
				}
			}
			else
			{
				// Set 2D position (the position at TVPNodeClient::SlotContext.Timestamp is not sent at the same time as the position for Mode)
				if ( _DataBase != NULL &&  (!IgnoreEntityDbUpdates || slot==0))
				{
					uint16 x16 = (uint16)((ac->getValue() >> 12) & 0xFFFF);
					uint16 y16 = (uint16)((ac->getValue() >> 28) & 0xFFFF);
					if ( ! (x16==0 && y16==0) ) // don't set the position if it was not initialized yet
					{
						sint32 x, y;
						_PropertyDecoder.decodeAbsPos2D( x, y, x16, y16 );

						CCDBNodeBranch	*nodeRoot;
						nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode(0));
						if ( nodeRoot )
						{
							CCDBNodeLeaf	*node;
							node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode(0));
							nlassert(node != NULL);
							node->setValue64( x );
							node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode(1));
							nlassert(node != NULL);
							node->setValue64( y );
						}
					}
					else
					{
						nldebug( "%u: S%hu: Received mode with null pos", _CurrentServerTick, (uint16)slot ); // TEMP
					}
				}
			}
			break;
		}
/*		case PROPERTY_GUILD_SYMBOL:
			nlinfo("GuildSymbol received...");
*/
		default:
		{
			// special for Bars, always take _CurrentServerTick timestamp (delta timestamp decoded is mainly for position purpose...)
			NLMISC::TGameCycle	timeStamp= TVPNodeClient::SlotContext.Timestamp;
			/* YOYO: i don't know what to do with others property that really use the gamecycle and are maybe buggued:
				ENTITY_MOUNTED_ID,RIDER_ENTITY_ID,BEHAVIOUR,TARGET_LIST,TARGET_ID,VISUAL_FX
				But bars timestamp accuracy is very important (else could take DB property with falsly newer timestamp)
			*/
			if(propIndex== PROPERTY_BARS)
				timeStamp= _CurrentServerTick;

			// Process property
			CChange thechange( slot, propIndex, timeStamp );
			_Changes.push_back( thechange );
			if (_DataBase != NULL && (!IgnoreEntityDbUpdates || slot==0) )
			{
				CCDBNodeBranch	*nodeRoot;
				nodeRoot = dynamic_cast<CCDBNodeBranch*>(_DataBase->getNode(0));
				if ( nodeRoot )
				{
					CCDBNodeLeaf *node = dynamic_cast<CCDBNodeLeaf*>(nodeRoot->getNode(slot)->getNode( propIndex ));
					nlassert(node != NULL);
					node->setValue64(ac->getValue());
					if ( LoggingMode )
					{
						nlinfo( "CLIENT: recvd property %hu (%s) for slot %hu, date %u", (uint16)propIndex, getPropText(propIndex), (uint16)slot, TVPNodeClient::SlotContext.Timestamp );
					}
					//nldebug("CLPROPNET[%p]: received property %d for entity %d: %"NL_I64"u", this, action->PropIndex, action->CLEntityId, action->getValue());
				}
			}
		}
	}
	CActionFactory::getInstance()->remove( (CAction*&)ac );

#ifdef SHOW_PROPERTIES_RECEIVED
//	stringstream ss;
//	ss << "Received: ";
//	if ( propReceived[2] != 0 )
//		ss << propReceived[2] << " impuls. ";
//	if ( propReceived[0] != 0 )
//		ss << propReceived[0] << " pos; ";
//	if ( propReceived[3] != 0 )
//		ss << propReceived[3] << " orient; ";
//	uint sum = propReceived[4] + propReceived[5] + propReceived[6] + propReceived[7] + propReceived[8] + propReceived[9];
//	if ( sum != 0 )
//		ss << sum << " discreet; ";
//	if ( propReceived[16] != 0 )
//		ss << propReceived[16] << "assoc; ";
//	if ( propReceived[17] != 0 )
//		ss << propReceived[17] << "disac; ";
//	ss << "TOTAL: " << propReceived[2] << " + " << propReceived[0] + propReceived[3] + sum;

	string str = "Received: ";
	if ( propReceived[2] != 0 )
		str += NLMISC::toString(propReceived[2]) + " impuls. ";
	if ( propReceived[0] != 0 )
		str += NLMISC::toString(propReceived[0]) + " pos; ";
	if ( propReceived[3] != 0 )
		str += NLMISC::toString(propReceived[3]) + " orient; ";
	uint sum = propReceived[4] + propReceived[5] + propReceived[6] + propReceived[7] + propReceived[8] + propReceived[9];
	if ( sum != 0 )
		str += NLMISC::toString(sum) + " discreet; ";
	if ( propReceived[16] != 0 )
		str += NLMISC::toString(propReceived[16]) + "assoc; ";
	if ( propReceived[17] != 0 )
		str += NLMISC::toString(propReceived[17]) + "disac; ";
	str += "TOTAL: " + NLMISC::toString(propReceived[2]) + " + " + NLMISC::toString(propReceived[0] + propReceived[3] + sum);

	nlwarning( "%s", str.c_str() );
#endif
}


void	CNetworkConnection::sendNormalMessage()
{
	//nlinfo("CNET[%p]: send normal message Packet=%d Ack=%d AckBits=%08X", this, _CurrentSendNumber, _LastReceivedNumber, _AckBitMask);

	//
	// Create the message to send to the server
	//

	CBitMemStream	message;

	bool			systemMode = false;

	message.serial (_CurrentSendNumber);
	message.serial (systemMode);
	message.serial (_LastReceivedNumber);
	message.serial (_AckBitMask);

	uint			numPacked = 0;

	// pack each block
	TGameCycle						lastPackedCycle = 0;
	list<CActionBlock>::iterator	itblock;

	//nldebug("CNET[%p]: sending message %d", this, _CurrentSendNumber);

	for (itblock=_Actions.begin(); itblock!=_Actions.end(); ++itblock)
	{
		CActionBlock	&block = *itblock;

		// if block contains action that are not already stamped, don't send it now
		if (block.Cycle == 0)
			break;

		// Prevent to send a message too big
		//if (message.getPosInBit() + (*itblock).bitSize() > FrontEndInputBufferSize) // hard version
		//	break;

		if (block.FirstPacket == 0)
			block.FirstPacket = _CurrentSendNumber;

		//nlassertex((*itblock).Cycle > lastPackedCycle, ("(*itblock).Cycle=%d lastPackedCycle=%d", (*itblock).Cycle, lastPackedCycle));

		lastPackedCycle = block.Cycle;

		block.serial(message);
		++numPacked;

		//nldebug("CNET[%p]: packed block %d, message is currently %d bits long", this, block.Cycle, message.getPosInBit());

		// Prevent to send a message too big
		//if (message.getPosInBit() + (*itblock).bitSize() > FrontEndInputBufferSize) // hard version
		if ( message.getPosInBit() > 480*8 ) // easy version
			break;
	}

	//_PropertyDecoder.send (_CurrentSendNumber, _LastReceivedNumber);
	uint32	length = message.length();
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);
	// remember send time
	_LastSendTime = CTime::getLocalTime();

	_PacketStamps.push_back(make_pair(_CurrentSendNumber, _UpdateTime));

	_CurrentSendNumber++;
}

bool	CNetworkConnection::stateConnected()
{
	// if receives System PROBE
	//    immediate state Probe
	// else if receives Normal
	//	   sends Normal data

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ! _ReplayIncomingMessagesOn )
#endif
	{
		// Prevent to increment the client time when the front-end does not respond
		static TTime previousTime = ryzomGetLocalTime ();
		TTime now = ryzomGetLocalTime ();
		TTime diff = now - previousTime;
		previousTime = now;
		if ( (diff > 3000) && (! _Connection.dataAvailable()) )
		{
			return false;
		}

		// update the current time;
		while (_CurrentClientTime < (TTime)(_UpdateTime - _MsPerTick - _LCT) && _CurrentClientTick < _CurrentServerTick)
		{
			_CurrentClientTime += _MsPerTick;

			_CurrentClientTick++;

			_MachineTimeAtTick = _UpdateTime;
			_MachineTicksAtTick = _UpdateTicks;
		}

		if (_CurrentClientTick >= _CurrentServerTick && !_Connection.dataAvailable())
		{
			return false;
		}
	}

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ClientCfg.Local && !_ReplayIncomingMessagesOn )
		return false;
	while ( (_ReplayIncomingMessagesOn && dataToReplayAvailable()) ||
			_Connection.dataAvailable() )
#else
	while (_Connection.dataAvailable())// && _TotalMessages<5)
#endif
	{
		_DecodedHeader = false;
		CBitMemStream	msgin (true);

		if (buildStream(msgin) && decodeHeader(msgin))
		{
			if (_SystemMode)
			{
				uint8	message = 0;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_PROBE_CODE:
					// receive probe, and goto state probe
					_ConnectionState = Probe;
					// reset client impulse & vars
/*
					_ImpulseDecoder.reset();
					_PropertyDecoder.clear();
					_PacketStamps.clear();
					// clears sent actions
					while (!_Actions.empty())
						CActionFactory::getInstance()->remove(_Actions.front().Actions),
					_Actions.clear();
					_AckBitMask = 0;
					_LastReceivedNumber = 0xffffffff;
*/
					//nldebug("CNET[%p]: connected->probe", this);
					_Changes.push_back(CChange(0, ProbeReceived));
					receiveSystemProbe(msgin);
					return true;
					break;
				case SYSTEM_SYNC_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Synchronize;
					//nldebug("CNET[%p]: connected->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Stalled;
					//nldebug("CNET[%p]: connected->stalled", this);
					receiveSystemStalled(msgin);
					return true;
					break;
				case SYSTEM_SERVER_DOWN_CODE:
					disconnect(); // will send disconnection message
					nlwarning( "BACK-END DOWN" );
					return false; // exit now from loop, don't expect a new state
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Connected", this, message);
					break;
				}
			}
			else
			{
				receiveNormalMessage(msgin);
			}

		}
	}

	return false;
}




//
// Probe state
//

void	CNetworkConnection::receiveSystemProbe(CBitMemStream &msgin)
{
	_LatestProbeTime = _UpdateTime;
	msgin.serial(_LatestProbe);
	_LatestProbes.push_back(_LatestProbe);

	//nldebug("CNET[%p]: received PROBE %d", this, _LatestProbe);
}

// sends system sync acknowledge
void	CNetworkConnection::sendSystemAckProbe()
{
	CBitMemStream	message;

	buildSystemHeader(message);

	uint8			probe = SYSTEM_ACK_PROBE_CODE;
	uint32			numprobes = (uint32)_LatestProbes.size();

	message.serial(probe);
	message.serial(numprobes);

	uint	i;
	for (i=0; i<numprobes; ++i)
		message.serial(_LatestProbes[i]);

	_LatestProbes.clear();

	uint32	length = message.length();
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

	//nlinfo("CNET[%p]: sent ACK_PROBE (%d probes)", this, numprobes);
}

bool	CNetworkConnection::stateProbe()
{
	// if receives System SYNC
	//    immediate state SYNC
	// else if receives System PROBE
	//    decode PROBE
	// sends System ACK_PROBE
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ClientCfg.Local && !_ReplayIncomingMessagesOn )
		return false;
	while ( (_ReplayIncomingMessagesOn && dataToReplayAvailable()) ||
			_Connection.dataAvailable() )
#else
	while (_Connection.dataAvailable())// && _TotalMessages<5)
#endif
	{
		_DecodedHeader = false;
		CBitMemStream	msgin (true);
		if (buildStream(msgin) && decodeHeader(msgin))
		{
			if (_SystemMode)
			{
				uint8	message = 0;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync and state synchronize
					_ConnectionState = Synchronize;
					//nldebug("CNET[%p]: probe->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive sync, decode sync and state synchronize
					_ConnectionState = Stalled;
					//nldebug("CNET[%p]: probe->stalled", this);
					receiveSystemStalled(msgin);
					return true;
					break;
				case SYSTEM_PROBE_CODE:
					// receive sync, decode sync
					receiveSystemProbe(msgin);
					break;
				case SYSTEM_SERVER_DOWN_CODE:
					disconnect(); // will send disconnection message
					nlwarning( "BACK-END DOWN" );
					return false; // exit now from loop, don't expect a new state
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Probe", message);
					break;
				}
			}
			else
			{
				nlwarning("CNET[%p]: received normal in state Probe", this);
			}
		}
	}

	// send ack sync if received sync or last sync timed out
	if (!_LatestProbes.empty() || _UpdateTime-_LatestProbeTime > 300)
	{
		sendSystemAckProbe();
		_LatestProbeTime = _UpdateTime;
	}
	else
		nlSleep(10);

	return false;
}





//
// Stalled state
//

void	CNetworkConnection::receiveSystemStalled(CBitMemStream &/* msgin */)
{
	nldebug("CNET[%p]: received STALLED", this);
}

bool	CNetworkConnection::stateStalled()
{
	// if receives System SYNC
	//    immediate state SYNC
	// else if receives System STALLED
	//    decode STALLED (nothing to do)
	// else if receives System PROBE
	//    immediate state PROBE
#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ClientCfg.Local && !_ReplayIncomingMessagesOn )
		return false;
	while ( (_ReplayIncomingMessagesOn && dataToReplayAvailable()) ||
			_Connection.dataAvailable() )
#else
	while (_Connection.dataAvailable())// && _TotalMessages<5)
#endif
	{
		_DecodedHeader = false;
		CBitMemStream	msgin (true);
		if (buildStream(msgin) && decodeHeader(msgin))
		{
			if (_SystemMode)
			{
				uint8	message = 0;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync and state synchronize
					_ConnectionState = Synchronize;
					nldebug("CNET[%p]: stalled->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_PROBE_CODE:
					// receive sync, decode sync
					_ConnectionState = Probe;
					nldebug("CNET[%p]: stalled->probe", this);
					receiveSystemProbe(msgin);
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled
					receiveSystemStalled(msgin);
					break;
				case SYSTEM_SERVER_DOWN_CODE:
					disconnect(); // will send disconnection message
					nlwarning( "BACK-END DOWN" );
					return false; // exit now from loop, don't expect a new state
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Stalled", message);
					break;
				}
			}
			else
			{
				nlwarning("CNET[%p]: received normal in state Stalled", this);
			}
		}
	}

	return false;
}





//
// encoding / decoding methods
//

bool	CNetworkConnection::decodeHeader(CBitMemStream &msgin, bool checkMessageNumber)
{
	if (_DecodedHeader)
		return true;

	// simulate packet loss
#if !FINAL_VERSION
	if(uint((rand()%100)) < ClientCfg.SimulatePacketLossRatio)
		return false;
#endif

	++_TotalMessages;

	_LastReceivedTime = _UpdateTime;

	msgin.serial (_CurrentReceivedNumber);
	msgin.serial (_SystemMode);

	if ((sint)_CurrentReceivedNumber > (sint)_LastReceivedPacketInBothModes && checkMessageNumber)
	{
		_TotalLostPackets += (_CurrentReceivedNumber-_LastReceivedPacketInBothModes - 1);
		_LastReceivedPacketInBothModes = _CurrentReceivedNumber;
	}

	_MeanPackets.update((float)_CurrentReceivedNumber, ryzomGetLocalTime ());

	/// \todo remove
	//nlinfo("DBG:BEN: decodeHeader, packet=%d, %s", _CurrentReceivedNumber, _SystemMode ? "SYSTEM" : "NORMAL");
	///

	if (_SystemMode)
	{
	}
	else
	{
		msgin.serial (_LastReceivedAck);

#ifdef INCLUDE_FE_STATS_IN_PACKETS
		// receive debug info in the message header
		uint32			UserLWatch,
						CycleWatch,
						ReceiveWatch,
						SendWatch;
		float			PriorityAmount;
		uint16			SeenEntities;
		//float			HpT;
		msgin.serial (UserLWatch);
		msgin.serial (CycleWatch);
		msgin.serial (ReceiveWatch);
		msgin.serial (SendWatch);
		msgin.serial (PriorityAmount);
		msgin.serial (SeenEntities);
		//msgin.serial (HpT);
#endif

#ifdef DISPLAY_ENTITIES
		UserLWatchGraph.addOneValue ((float)UserLWatch);
		CycleWatchGraph.addOneValue ((float)CycleWatch);
		ReceiveWatchGraph.addOneValue ((float)ReceiveWatch);
		SendWatchGraph.addOneValue ((float)SendWatch);
		PriorityAmountGraph.addOneValue(PriorityAmount);
		SeenEntitiesGraph.addOneValue(SeenEntities);
		//HPThreshold = HpT;
#else
		static sint counter = 128;
		--counter;
		if ( counter == 0 )
		{
//			nlinfo( "User:%u Cycle:%u Rcv:%u Snd:%u PrioAmount:%.2f",
//					UserLWatch, CycleWatch, ReceiveWatch, SendWatch, PriorityAmount );
			counter = 128;
		}
#endif // DISPLAY_ENTITIES
	}

	if (!checkMessageNumber)
		return true;

	// display info on this packet
	//nlinfo("CNET[%p] received packet %d, %s mode - LastReceivedAck=%d", this, _CurrentReceivedNumber, _SystemMode ? "SYTEM" : "NORMAL", _LastReceivedAck);

	// todo doesn't work if we receive the packet in bad order or 2 same packet

	if (_CurrentReceivedNumber > _LastReceivedNumber+1)
	{
		// we lost some messages...
		nldebug ("CNET[%p] lost messages server->client [%d; %d]", this, _LastReceivedNumber+1, _CurrentReceivedNumber-1);
		_MeanLoss.update((float)(_CurrentReceivedNumber-_LastReceivedNumber-1), ryzomGetLocalTime ());
	}
	else if (_CurrentReceivedNumber == _LastReceivedNumber)
	{
		// we receive the same packet that the last one
		nldebug ("CNET[%p] awaiting packet %d, received packet %d", this, _LastReceivedNumber+1, _CurrentReceivedNumber);
		return false;
	}
	else if (_CurrentReceivedNumber < _LastReceivedNumber)
	{
		// it's an older message than the current
		nldebug ("CNET[%p] received an old message, awaiting packet %d, received packet %d", this, _LastReceivedNumber+1, _CurrentReceivedNumber);
		return false;
	}

	// don't acknowledge system messages and normal messages in
	// because this will disturb impulsion from frontend, that will interpret it as if previous messages were ok
	bool	ackBool = (!_SystemMode && (_ConnectionState == Connected || _ConnectionState == Synchronize));
	uint	ackBit = (ackBool ? 1 : 0);

	if (_CurrentReceivedNumber - _LastReceivedNumber < 32)
	{
		_AckBitMask <<= _CurrentReceivedNumber - _LastReceivedNumber;
		_AckBitMask |= _LastAckBit << (_CurrentReceivedNumber - _LastReceivedNumber - 1);
	}
	else
	{
		_AckBitMask = (_CurrentReceivedNumber - _LastReceivedNumber == 32 && _LastAckBit != 0) ? 0x80000000 : 0x00000000;
	}

	_LastAckBit = ackBit;

	// encode long ack bitfield
	TPacketNumber	i;
	for (i=_LastReceivedNumber+1; i<_CurrentReceivedNumber; ++i)
		_LongAckBitField.clear(i&(NumBitsInLongAck-1));

	_LongAckBitField.set(_CurrentReceivedNumber&(NumBitsInLongAck-1), ackBool);


	// no more than NumBitsInLongAck ack in field
	if ((TPacketNumber)_LastAckInLongAck <= (TPacketNumber)(_CurrentReceivedNumber-NumBitsInLongAck))
		_LastAckInLongAck = _CurrentReceivedNumber-NumBitsInLongAck+1;

	_LastReceivedNumber = _CurrentReceivedNumber;

	_DecodedHeader = true;

	return true;
}

void	CNetworkConnection::buildSystemHeader(NLMISC::CBitMemStream &msgout)
{
	msgout.serial (_CurrentSendNumber);
	bool			systemMode = true;
	msgout.serial (systemMode);

	_PacketStamps.push_back(make_pair(_CurrentSendNumber, _UpdateTime));

	++_CurrentSendNumber;
}

//
//
//


void	CNetworkConnection::setDataBase(CCDBNodeBranch *database)
{
	_DataBase = database;
}


//
//
//

void	CNetworkConnection::push(CBitMemStream &msg)
{
	sint32	maxImpulseBitSize = 230*8;

	CActionGeneric *ag = (CActionGeneric *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_CODE);
	if( ag == NULL ) //TODO: see that with oliver...
		return;

	uint	bytelen = msg.length();
	sint32	impulseMinBitSize = (sint32)CActionFactory::getInstance ()->size( ag );
	sint32	impulseBitSize = impulseMinBitSize + (4 + bytelen)*8;

	if (impulseBitSize < maxImpulseBitSize)
	{
		ag->set(msg);
		push(ag);
	}
	else
	{
		CAction	*casted = ag;
		CActionFactory::getInstance()->remove(casted);
		ag = NULL;

		// MultiPart impulsion
		CActionGenericMultiPart *agmp = (CActionGenericMultiPart *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_MULTI_PART_CODE);
		sint32 minimumBitSizeForMP = CActionFactory::getInstance ()->size (agmp);

		sint32 availableSize = (maxImpulseBitSize - minimumBitSizeForMP) / 8; // (in bytes)
#ifdef NL_DEBUG
		nlassert( availableSize > 0 ); // the available size must be larger than the 'empty' size
#endif
		sint32 nbBlock = (bytelen + availableSize - 1) / availableSize;

		uint8 num = _ImpulseMultiPartNumber++;

		for (sint32 i = 0; i < nbBlock; i++)
		{
			if (i != 0)
				agmp = (CActionGenericMultiPart *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_MULTI_PART_CODE);

			agmp->set(num, (uint16)i, msg.buffer(), bytelen, availableSize, (uint16)nbBlock);
			push(agmp);
		}
	}
}

void	CNetworkConnection::pushTarget(TCLEntityId slot, LHSTATE::TLHState targetOrPickup )
{
	CActionTargetSlot *ats = (CActionTargetSlot*)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_TARGET_SLOT_CODE);
	nlassert (ats != NULL);
	ats->Slot = slot;
	switch ( targetOrPickup ) // ensure the value is good for the FE
	{
		case LHSTATE::NONE: ats->TargetOrPickup = 0; break;
		case LHSTATE::LOOTABLE: ats->TargetOrPickup = 1; break;
		case LHSTATE::HARVESTABLE: ats->TargetOrPickup = 2; break;
		default:
		break;
	}

	ats->TargetOrPickup = (uint32)targetOrPickup;
	push(ats);
}


void	CNetworkConnection::push(CAction *action)
{
	if (_Actions.empty() || _Actions.back().Cycle != 0)
	{
		//nlinfo("-BEEN- push back 2 [size=%d, cycle=%d]", _Actions.size(), _Actions.empty() ? 0 : _Actions.back().Cycle);
		_Actions.push_back(CLFECOMMON::CActionBlock());
	}

	_Actions.back().Actions.push_back(action);
}


//

void	CNetworkConnection::send(TGameCycle cycle)
{
	try
	{
		// check the current game cycle was not already sent
		nlassertex(_LastSentCycle < cycle, ("Client=%p, _LastSentCycle=%d, cycle=%d", this, _LastSentCycle, cycle));

	/*
		if (_LastSentCycle == cycle)	// delay send till next tick
			return;
	*/
		_LastSentCycle = cycle;

		// if no actions were sent at this cyle, create a new block
		if (_Actions.empty() || _Actions.back().Cycle != 0)
		{
	//		nlinfo("-BEEN- push back 1 [size=%d, cycle=%d]", _Actions.size(), _Actions.empty() ? 0 : _Actions.back().Cycle);
	//		_Actions.push_back();
		}
		else
		{
			CActionBlock	&block = _Actions.back();
/*
			CAction	*dummy = CActionFactory::getInstance()->create(INVALID_SLOT, ACTION_DUMMY_CODE);
			((CActionDummy*)dummy)->Dummy1 = _DummySend++;
			push(dummy);
*/

			_Actions.back().Cycle = cycle;

			// check last block isn't bigger than maximum allowed
			uint	i;
			uint	bitSize = 32+8;		// block size is 32 (cycle) + 8 (number of actions
			for (i=0; i<block.Actions.size(); ++i)
			{
				bitSize += CActionFactory::getInstance()->size(block.Actions[i]);
				if (bitSize >= 480*8)
					break;
			}

			if (i<block.Actions.size())
			{
				nldebug( "Postponing %u actions exceeding max size in block %d (block size is %d bits long)", block.Actions.size()-i, cycle, bitSize );

				// last block is bigger than allowed

				// allocate a new block
				_Actions.push_back(CActionBlock());
				CActionBlock	&newBlock = _Actions.back();

				// reset block stamp
				newBlock.Cycle = 0;

				// copy remaining actions in new block
				newBlock.Actions.insert(newBlock.Actions.begin(),
										block.Actions.begin()+i,
										block.Actions.end());

				// remove remaining actions of block
				block.Actions.erase(block.Actions.begin()+i, block.Actions.end());
			}

			//nlinfo("-BEEN- setcycle [size=%d, cycle=%d]", _Actions.size(), _Actions.empty() ? 0 : _Actions.back().Cycle);
		}

		if (_ConnectionState == Connected)
		{
			sendNormalMessage();
		}
	}
	catch (const ESocket &/*e*/)
	{
		_ConnectionState = Disconnect;
		disconnect(); // won't send disconnection message as state is already Disconnect
	}
}


void	CNetworkConnection::send()
{
	try
	{
		// Send is temporised, that is the packet may not be actually sent.
		// We don't care, since:
		// - this packet has no new data (not ticked send)
		// - a next send() will send packet if time elapsed enough
		// - a next send(tick) will really be sent
		// This way, we can say that at most 15 packets will be delivered each second
		// (5 send(tick), and 10 send() -- if you take getLocalTime() inaccuracy into account)
		if (_ConnectionState == Connected && CTime::getLocalTime()-_LastSendTime > 100)
		{
			sendNormalMessage();
		}
	}
	catch (const ESocket &/*e*/)
	{
		_ConnectionState = Disconnect;
	}
}

//
//
//

// sends system sync acknowledge
void	CNetworkConnection::sendSystemDisconnection()
{
	CBitMemStream	message;

	buildSystemHeader(message);

	uint8			disconnection = SYSTEM_DISCONNECTION_CODE;

	message.serial(disconnection);

	uint32	length = message.length();

	if (_Connection.connected())
		_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

	updateBufferizedPackets ();
	nlinfo("CNET[%p]: sent DISCONNECTION", this);
}

void	CNetworkConnection::disconnect()
{
#ifdef MEASURE_RECEIVE_DATES
	if ( LogReceiveEnabled )
	{
		displayReceiveLog();
	}
#endif

	if (_ConnectionState == NotInitialised ||
		_ConnectionState == NotConnected ||
		_ConnectionState == Authenticate ||
		_ConnectionState == Disconnect)
	{
		//nlwarning("Unable to disconnect(): not connected yet, or already disconnected.");
		return;
	}

	sendSystemDisconnection();
	_Connection.close();
	_ConnectionState = Disconnect;
}



//
// Quit state
//

void	CNetworkConnection::receiveSystemAckQuit(CBitMemStream &/* msgin */)
{
	nldebug("CNET[%p]: received ACK_QUIT", this);
	_ReceivedAckQuit = true;

}

bool	CNetworkConnection::stateQuit()
{

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( ClientCfg.Local && !_ReplayIncomingMessagesOn )
		return false;
	while ( (_ReplayIncomingMessagesOn && dataToReplayAvailable()) ||
			_Connection.dataAvailable() )
#else
	while (_Connection.dataAvailable())// && _TotalMessages<5)
#endif
	{
		_DecodedHeader = false;
		CBitMemStream	msgin (true);
		if (buildStream(msgin) && decodeHeader(msgin, false))
		{
			if (_SystemMode)
			{
				uint8	message = 0;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync and state synchronize
					reset();
					_ConnectionState = Synchronize;
					nldebug("CNET[%p]: quit->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
/*
				case SYSTEM_PROBE_CODE:
					// receive sync, decode sync
					_ConnectionState = Probe;
					nldebug("CNET[%p]: stalled->probe", this);
					receiveSystemProbe(msgin);
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled
					_ConnectionState = Stalled;
					receiveSystemStalled(msgin);
					return true;
					break;
*/
				case SYSTEM_SERVER_DOWN_CODE:
					disconnect(); // will send disconnection message
					nlwarning( "BACK-END DOWN" );
					return false; // exit now from loop, don't expect a new state
					break;
				case SYSTEM_ACK_QUIT_CODE:
					// receive ack quit -> reset connection state
					receiveSystemAckQuit(msgin);
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Stalled", message);
					break;
				}
			}
			else
			{
				nlwarning("CNET[%p]: received normal in state Stalled", this);
			}
		}
	}

	// send quit if not yet received a ack quit
	if (!_ReceivedAckQuit && _UpdateTime-_LatestQuitTime > 100)
	{
		sendSystemQuit();
		_LatestQuitTime = _UpdateTime;
	}

	return false;
}

/*
 * Quit the game till the connection is reset
 */
void	CNetworkConnection::quit()
{
	nlinfo("CNetworkConnection::quit() called, setting to quitting state.");

	++_QuitId;
	_ConnectionState = Quit;
	_ReceivedAckQuit = false;
	_LatestQuitTime = _UpdateTime;

	sendSystemQuit();
}

void	CNetworkConnection::reset()
{
	_CurrentSendNumber = 0;
	_LastReceivedNumber = 0;
	_LastReceivedTime = 0;
	_LastReceivedNormalTime = 0;
	_AckBitMask = 0;
	_LastAckBit = 0;

	_Synchronize = 0;
	_InstantPing = 10000;
	_BestPing = 10000;
	_LCT = 100;
	_MachineTimeAtTick = ryzomGetLocalTime ();
	_MachineTicksAtTick = CTime::getPerformanceTime();
	_LastSentSync = 0;
	_LatestSync = 0;

	_PropertyDecoder.init (256);

	_DummySend = 0;
	_LongAckBitField.resize(1024);
	_LastAckInLongAck = 0;
	_LastSentCycle = 0;

	_TotalReceivedBytes = 0;
	_PartialReceivedBytes = 0;
	_TotalSentBytes = 0;
	_PartialSentBytes = 0;
	_MeanPackets.MeanPeriod = 5000;
	_MeanLoss.MeanPeriod = 5000;

	_LastReceivedPacketInBothModes = 0;
	_TotalLostPackets = 0;
	_ConnectionQuality = false;

	_CurrentSmoothServerTick= 0;
	_SSTLastLocalTime= 0;
}

void	CNetworkConnection::initTicks()
{
	_CurrentClientTick = 0;
	_CurrentServerTick = 0;
	_MsPerTick = 100;
	_LCT = 1000;
}

void	CNetworkConnection::reinit()
{
	// Reset data
	_ImpulseDecoder.reset();
	if (_DataBase)
		_DataBase->resetData(_CurrentServerTick, true);
	_LongAckBitField.clearAll();
	_PacketStamps.clear();
	_Actions.clear();
	_Changes.clear();
	_GenericMultiPartTemp.clear();
	_IdMap.clear();
	reset();
	initTicks();

	// Reuse the udp socket
	_Connection.~CUdpSimSock();
	new (&_Connection) CUdpSimSock();
}

// sends system sync acknowledge
void	CNetworkConnection::sendSystemQuit()
{
	CBitMemStream	message;

	buildSystemHeader(message);

	uint8			quit = SYSTEM_QUIT_CODE;

	message.serial(quit);
	message.serial(_QuitId);

	uint32	length = message.length();
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

	updateBufferizedPackets ();
	nlinfo("CNET[%p]: sent QUIT", this);
}

//

void	CNetworkConnection::displayAllocationStats()
{
	nlinfo("CNET[%p]: %d queued blocks, %d changes", this, _Actions.size(), _Changes.size());
}

string	CNetworkConnection::getAllocationStats()
{
	char	buf[128];
	sprintf(buf, "CNET[%p]: %u queued blocks, %u changes", this, (uint)_Actions.size(), (uint)_Changes.size());
	return string(buf);
}


//
//
//

void CNetworkConnection::genericAction (CActionGeneric *ag)
{
	// manage a generic action
	CBitMemStream &bms = ag->get ();

	//nldebug("CNET: Calling impulsion callback (size %u) :'%s'", this, bms.length(), toHexaString(bms.bufferAsVector()).c_str());
	//nldebug("CNET[%p]: Calling impulsion callback (size %u)", this, bms.length());

	if (_ImpulseCallback != NULL)
		_ImpulseCallback(bms, _LastReceivedNumber, _ImpulseArg);
}

void CNetworkConnection::CGenericMultiPartTemp::set (CActionGenericMultiPart *agmp, CNetworkConnection *parent)
{
	if (NbBlock == 0xFFFFFFFF)
	{
		// new GenericMultiPart
		NbBlock = agmp->NbBlock;
		NbCurrentBlock = 0;
		TempSize = 0;
		Temp.clear();
		Temp.resize(NbBlock);
		BlockReceived.resize(NbBlock);
		for (uint i = 0; i < NbBlock; i++)
			BlockReceived[i] = false;
	}

	nlassert (NbBlock == agmp->NbBlock);
	nlassert (NbBlock > agmp->Part);

	// check if the block was already received
	if (BlockReceived[agmp->Part])
	{
		nlwarning ("CLMPNET[%p]: This part is already received, discard it", this);
		return;
	}

	Temp[agmp->Part] = agmp->PartCont;
	BlockReceived[agmp->Part] = true;

	NbCurrentBlock++;
	TempSize += (uint32)agmp->PartCont.size();

	if (NbCurrentBlock == NbBlock)
	{
		// reform the total action

		//nldebug("CLMPNET[%p]: Received a TOTAL generic action MP size: number %d nbblock %d", this,  agmp->Number, NbBlock);

		CBitMemStream bms(true);

		uint8 *ptr = bms.bufferToFill (TempSize);

		for (uint i = 0; i < Temp.size (); i++)
		{
			memcpy (ptr, &(Temp[i][0]), Temp[i].size());
			ptr += Temp[i].size();
		}

		NbBlock = 0xFFFFFFFF;

		//nldebug("CLMPNET[%p]: Received a generic action size %d", this, bms.length());
		// todo interface api, call a user callback

		if (parent->_ImpulseCallback != NULL)
			parent->_ImpulseCallback(bms, parent->_LastReceivedNumber, parent->_ImpulseArg);

	}
}

void CNetworkConnection::genericAction (CActionGenericMultiPart *agmp)
{
	// manage a generic action (big one that comes by blocks)
	vector<uint8> &v = agmp->PartCont;

	//nldebug("CLMPNET[%p]: Received a generic action MP size %d: number %d part %d nbblock %d", this, v.size(), agmp->Number, agmp->Part, agmp->NbBlock);

	// add it

	if (_GenericMultiPartTemp.size() <= agmp->Number)
	{
		_GenericMultiPartTemp.resize (agmp->Number+1);
	}

	_GenericMultiPartTemp[agmp->Number].set(agmp, this);

}


CNetworkConnection::TVPNodeClient::TSlotContext							CNetworkConnection::TVPNodeClient::SlotContext;

/*
 * Return the average billed upload rate in kbps, including all headers (UDP+IP+Ethernet)
 */
void CNetworkConnection::statsSend(uint32 bytes)
{
	_TotalSentBytes += bytes;
	_PartialSentBytes += bytes;
	_MeanUpload.update((float)bytes, ryzomGetLocalTime ());

	UploadGraph.addValue ((float)bytes);
}


/*
 * Return the average billed download rate in kbps, including all headers (UDP+IP+Ethernet)
 */
void CNetworkConnection::statsReceive(uint32 bytes)
{
	_TotalReceivedBytes += bytes;
	_PartialReceivedBytes += bytes;
	_MeanDownload.update((float)bytes, ryzomGetLocalTime ());

	DownloadGraph.addValue ((float)bytes);
}


NLMISC_COMMAND( displayPosUpdateGraph, "Display position update interval graph", "0|<slot>" )
{
	// Stop graph in all cases
	if ( PosUpdateIntervalGraph )
	{
		delete PosUpdateIntervalGraph;
		delete PosUpdatePredictionGraph;
		PosUpdateIntervalGraph = NULL;
		PosUpdatePredictionGraph = NULL;
	}

	// Start graph if argument is not 0
	if ( (args.size() != 0) && (args[0] != "0") )
	{
		uint8 slot;
		fromString(args[0], slot);
		PosUpdateIntervalGraph = new CSlotGraph( "Interval", 350, 2, 100, 200, CRGBA(128,0,0,64), 1000, 20, true, slot );
		PosUpdatePredictionGraph = new CSlotGraph( "                    Prediction", 350, 2, 100, 200, CRGBA(0,0,128,64), 1000, 20, true, slot );
	}
	return true;
}


// ***************************************************************************
sint64		CNetworkConnection::convertToSmoothServerTick(NLMISC::TGameCycle t) const
{
	return t*SMOOTH_SERVER_TICK_PER_TICK;
}


// ***************************************************************************
void		CNetworkConnection::updateSmoothServerTick()
{
	// Get deltaT
	NLMISC::TTime	t= ryzomGetLocalTime ();
	sint32	deltaT= (sint32)(t - _SSTLastLocalTime);
	_SSTLastLocalTime= t;

	// Get the actual ServerTick not smoothed value
	sint64	actualST= _CurrentServerTick*SMOOTH_SERVER_TICK_PER_TICK;

	// Special bound cases
	if( _CurrentSmoothServerTick < actualST+SMOOTH_SERVER_TICK_DIFF_MIN )
	{
		// Reset (possible jump to future!)
		_CurrentSmoothServerTick= actualST;
	}
	else if( _CurrentSmoothServerTick>= actualST+SMOOTH_SERVER_TICK_DIFF_MAX )
	{
		// Clamp (no reset, no back to past!)
		_CurrentSmoothServerTick= actualST+SMOOTH_SERVER_TICK_DIFF_MAX;
	}
	else
	{
		// Compute the Factor of acceleration according to error difference (FIXED 16:16)
		sint64	factor;
		sint64	errorDiff= _CurrentSmoothServerTick-actualST;
		// If the estimation is in the TimeWindow, no acceleration
		if( errorDiff>=-SMOOTH_SERVER_TICK_WINDOW && errorDiff<=SMOOTH_SERVER_TICK_WINDOW)
		{
			factor= 65536;
		}
		// If the estimation is late, accelerate
		else if( errorDiff<0 )
		{
			float	f= (float)(errorDiff+SMOOTH_SERVER_TICK_WINDOW)/(SMOOTH_SERVER_TICK_DIFF_MIN+SMOOTH_SERVER_TICK_WINDOW);
			f*= SMOOTH_SERVER_TICK_ACCEL;
			factor= sint64(f*65536);
		}
		// Else the estimation is too early, slowDown
		else
		{
			float	f= (float)(errorDiff-SMOOTH_SERVER_TICK_WINDOW)/(SMOOTH_SERVER_TICK_DIFF_MAX-SMOOTH_SERVER_TICK_WINDOW);
			f= 1-f;
			factor= sint64(f*65536);
		}

		// Update the Smooth
		_CurrentSmoothServerTick+= ((deltaT*SMOOTH_SERVER_TICK_PER_TICK*factor)/getMsPerTick()) >> 16;
	}
}
