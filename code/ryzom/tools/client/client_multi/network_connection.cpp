/** \file network_connection.cpp
 * CNetworkConnection
 *
 * $Id: network_connection.cpp,v 1.4 2004/10/08 14:02:02 legros Exp $
 */



#include "stdpch.h"

#include <process.h>

#include "network_connection.h"

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

#include "game_share/simlag.h"

#include "cdb.h"
#include "cdb_leaf.h"
#include "cdb_branch.h"
#include "cdb_synchronised.h"

#include "game_share/system_message.h"

#include "game_share/entity_types.h" // required for ifdef

#include "graph.h"

#ifdef DISPLAY_ENTITIES
#include "../../../test/network/sb5000/client/graph.h"
#endif

#include <set>


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


// ***************************************************************************
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CLFECOMMON;


//#define MEASURE_RECEIVE_DATES
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
	TRDateState( TGameCycle gc, TTime ct ) : ServerCycle(gc), LocalTime(ct) {}

	TGameCycle	ServerCycle;
	TTicks		LocalTime;
};

////////////
// GLOBAL //
////////////


typedef vector<TRDateState> TReceiveDateLog;
TReceiveDateLog				ReceivePosDateLog [256];
bool						EnableLogReceive = false;
bool						LogReceive = false;
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
		ReceiveLogDisp = new CFileDisplayer( "ReceiveLog.log" );
		ReceiveLogger.addDisplayer( ReceiveLogDisp );
		ReceiveLogger.displayNL( "Searching for param LogReceive in the config file..." );
		NCConfigFile.load( string( "client.cfg" ) );
		if ( NCConfigFile.getVar( "LogReceive" ).asInt() != 0 )
		{
			EnableLogReceive = true;
		}
	}
	catch ( EConfigFile& )
	{}
	ReceiveLogger.displayNL( "LogReceive is %s", EnableLogReceive?"enabled":"disabled" );
}

/*
 * startReceiveLog
 */
void			startReceiveLog()
{
	if ( EnableLogReceive )
	{
		sint i;
		for ( i=0; i!=256; ++i )
		{
			ReceivePosDateLog[i].clear();
		}

		LogReceive = true;
	}
}

/*
 * stopReceiveLog
 */
void			stopReceiveLog()
{
	LogReceive = false;
}

/*
 * displayReceiveLog
 */
void			displayReceiveLog()
{
	if ( EnableLogReceive )
	{
		ReceiveLogger.displayNL( "ReceiveLog (ServerCycle, LocalTime(ms)):" );
		sint i;
		for ( i=0; i!=256; ++i )
		{
			ReceiveLogger.displayRawNL( "Entity %d: %u updates", i, ReceivePosDateLog[i].size() );
			TReceiveDateLog::iterator idl;
			for ( idl=ReceivePosDateLog[i].begin(); idl!=ReceivePosDateLog[i].end(); ++idl )
			{
				ReceiveLogger.displayRawNL( "%u\t%"NL_I64"u", (*idl).ServerCycle, (*idl).LocalTime );
			}
		}
	}
	ReceiveLogger.displayRawNL( "ReceiveLog completed" );

}

NLMISC_COMMAND( startReceiveLog, "Starts logging the position receives", "" )
{
	startReceiveLog();
	return true;
}

NLMISC_COMMAND( stopReceiveLog, "Stops logging the position receives", "" )
{
	stopReceiveLog();
	return true;
}

NLMISC_COMMAND( displayReceiveLog, "Flush the receive log in ReceiveLog.log", "" )
{
	displayReceiveLog();
	return true;
}

#endif

CSlotGraph *PosUpdateIntervalGraph = NULL;
CSlotGraph *PosUpdatePredictionGraph = NULL;

CGraph DownloadGraph ("download (byps)", 10.0f, 260.0f, 100.0f, 100.0f, CRGBA(0,0,128,128), 1000, 4000.0f);
CGraph UploadGraph ("upload (byps)", 10.0f, 360.0f, 100.0f, 100.0f, CRGBA(0,128,128,128), 1000, 4000.0f);

CGraph PingGraph ("ping (ms)", 10.0f, 460.0f, 100.0f, 100.0f, CRGBA(128,0,0,128), 100000, 1000.0f);
CGraph PacketLossGraph ("PacketLoss (pc)", 150.0f, 460.0f, 100.0f, 100.0f, CRGBA(0,128,0,128), 100000, 100.0f);


#define A a()
#define B b()
#define Parent parent()


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
	uint ds = dataset.size();
	if ( ds == 1 )
		return (float)(*dataset.begin());
	float fpIndex = rp * (float)(ds-1);
	sint ipIndex = (sint)fpIndex;
	multiset<uint32>::reverse_iterator it = dataset.rbegin(), itnext;
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

	_CurrentSendNumber = 0;
	_LastReceivedNumber = 0;
	_LastReceivedTime = 0;
	_AckBitMask = 0;

	_Synchronize = 0;
	_InstantPing = 10000;
	_BestPing = 10000;
	_LCT = 100;
	_MachineTimeAtTick = ryzomGetLocalTime ();
	_MachineTicksAtTick = CTime::getPerformanceTime();
	_LastSentSync = 0;

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

	_CurrentSmoothServerTick= 0;
	_SSTLastLocalTime= 0;
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
		_VisualPropertyTreeRoot->deleteBranches();

#ifdef ENABLE_INCOMING_MSG_RECORDER
	if ( _RecordIncomingMessagesOn )
		_RecordedMessagesOut.close();
	else if ( _ReplayIncomingMessagesOn )
		_RecordedMessagesIn.close();
#endif
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
			nlinfo ("Set the cookie with the UserId %d set in config file", uid);
		}

		nlinfo ("Network initialisation with front end '%s' and cookie %s",_FrontendAddress.c_str(), _LoginCookie.toString().c_str ());
	}

#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
	nlinfo( "Half-frequency mode" );
#else
	nlinfo( "Full-frequency mode" );
#endif

	_ConnectionState = NotConnected;
	
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
			sint i;
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
						for (sint j = 0; j < needToCopy.size(); j++)
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
							catch (Exception &)
							{
								nlwarning ("Can't copy '%s' '%s', try the next file", arg1.c_str(), dstPath.c_str());
							}
						}
						break;
					}
				}
				catch (Exception &e)
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
	catch (Exception &)
	{
		nlinfo ("There's no shards.cfg, or bad file format, can't copy common files");
	}

	nlinfo ("CNET[%p]: Connecting to '%s' with cookie '%s'", this, _FrontendAddress.c_str(), _LoginCookie.toString().c_str ());

	// then connect to the frontend using the udp sock
	result = CLoginClient::connectToShard (_FrontendAddress, _Connection);

	if(!result.empty())
		return false;

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



//
//
//

bool	CNetworkConnection::update()
{
	if ( _NextClientTickToReplay == ~0 )
	{
		setReplayingMode( false );
		return false;
	}

	_UpdateTime = ryzomGetLocalTime ();
	_UpdateTicks = CTime::getPerformanceTime();
	_ReceivedSync = false;
	_NormalPacketsReceived = 0;
	_TotalMessages = 0;

	//nldebug("CNET[%d]: begin update()", this);


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

		default:
			// Nothing here !
			break;
		}
	}
	while (stateBroke);// && _TotalMessages<5);

	//updateBufferizedPackets ();

	PacketLossGraph.addOneValue (getMeanPacketLoss ());

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
			_NextClientTickToReplay = ~0;
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

	uint32	length = message.length();
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

	nlinfo("CNET[%p]: sent LOGIN cookie=%s", this, _LoginCookie.toString().c_str());
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
				uint8	message;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync
					_ConnectionState = Synchronize;
					nlwarning("CNET[%p]:  login->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_PROBE_CODE:
					// receive probe, decode probe and state probe
					_ConnectionState = Probe;
					_Changes.push_back(CChange(0, ProbeReceived));
					nlwarning("CNET[%p]: login->probe", this);
					receiveSystemProbe(msgin);
					return true;
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Login", this, message);
					break;
				}
			}
			else
			{
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
	if ( _ReplayIncomingMessagesOn )
	{
		TGameCycle dummyTick;
		TTime dummyTime;
		msgin.serial( dummyTick );
		msgin.serial( dummyTime );
		return;
	}
	
	_LatestSyncTime = _UpdateTime;
	TTime		stime;
	msgin.serial(_Synchronize);
	msgin.serial(stime);
	_ReceivedSync = true;

	_MsPerTick = 100;			// initial values
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
#pragma message ("HALF_FREQUENCY_SENDING_TO_CLIENT")
	_CurrentServerTick = _Synchronize+_CurrentReceivedNumber*2;
#else
#pragma message ("FULL_FREQUENCY_SENDING_TO_CLIENT")
	_CurrentServerTick = _Synchronize+_CurrentReceivedNumber;
#endif
	_CurrentClientTick = _CurrentServerTick;
	_CurrentClientTime = _UpdateTime;

	//nlinfo( "CNET[%p]: received SYNC %"NL_I64"u %"NL_I64"u - _CurrentReceivedNumber=%d _CurrentServerTick=%d", this, (uint64)_Synchronize, (uint64)stime, _CurrentReceivedNumber, _CurrentServerTick );

	sendSystemAckSync();
}

// sends system sync acknowledge
void	CNetworkConnection::sendSystemAckSync()
{
	if ( _ReplayIncomingMessagesOn )
		return;

	CBitMemStream	message;

	buildSystemHeader(message);

	uint8			sync = SYSTEM_ACK_SYNC_CODE;
	message.serial(sync);
	message.serial(_LastReceivedNumber);
	message.serial(_LastAckInLongAck);
	message.serial(_LongAckBitField);

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
				uint8	message;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_PROBE_CODE:
					// receive probe, decode probe and state probe
					_ConnectionState = Probe;
					nlwarning("CNET[%p]: synchronize->probe", this);
					_Changes.push_back(CChange(0, ProbeReceived));
					receiveSystemProbe(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Stalled;
					nlwarning("CNET[%p]: synchronize->stalled", this);
					receiveSystemStalled(msgin);
					return true;
					break;
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync
					receiveSystemSync(msgin);
					break;
				default:
					nlwarning("CNET[%p]: received system %d in state Synchronize", this, message);
					break;
				}
			}
			else
			{
				_ConnectionState = Connected;
				nlwarning("CNET[%p]: synchronize->connected", this);
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
		nlwarning("Frontend ack'ed message %d not stamp dated", _LastReceivedAck);
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
				_MsPerTick = (sint32)(earliest-_CurrentClientTime)/numStepTick;

			// adjust if estimation of _MsPerTick is too large
			if ((TTime)(_CurrentClientTime+_MsPerTick*numStepTick) > latest)
				_MsPerTick = (sint32)(latest-_CurrentClientTime)/numStepTick;

			// _MsPerTick should be positive here -- seems to crash yet
			/// \todo we should instead of putting 1, returning in probe mode because it means that we had a very big lag
			if (_MsPerTick == 0)
			{
				nlwarning ("_MsPerTick is 0 because server tick is too big %d compare to the client tick is %d", _CurrentServerTick, _CurrentClientTick);
				_MsPerTick = 1;
			}
		}
		else if (numStepTick <= 0)
		{
			_MsPerTick = (sint32)_LCT;
		}
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
				_ConnectionState = Disconnect;
				nlwarning( "You were disconnected by the server" );
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

#ifdef DISPLAY_ENTITIES
	DownloadGraph.addValue ((float)(msgin.length()));
	DpfGraph.addValue ((float)(msgin.length()));
#endif
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


//#define displayByteBits(a,b,c)
void displayByteBits( uint8 b, uint nbits, sint beginpos, bool displayBegin )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? "1" : "0";
	}
	nlinfo( "%s", s1.c_str() );
	if ( displayBegin )
	{
		for ( i=nbits; i>beginpos+1; --i )
		{
			s2 += " ";
		}
		s2 += "^";
		nlinfo( "%s beginpos=%u", s2.c_str(), beginpos );
	}
}


//#define displayDwordBits(a,b,c)
void displayDwordBits( uint32 b, uint nbits, sint beginpos )
{
	string s1, s2;
	sint i;
	for ( i=nbits-1; i!=-1; --i )
	{
		s1 += ( (b >> i) & 1 ) ? "1" : "0";
	}
	nlinfo( "%s", s1.c_str() );
	for ( i=nbits; i>beginpos+1; --i )
	{
		s2 += " ";
	}
	s2 += "^";
	nlinfo( "%s beginpos=%u", s2.c_str(), beginpos );
}


void	displayBitStream( CBitMemStream& msg, sint beginbitpos, sint endbitpos )
{
	sint beginpos = beginbitpos/8;
	sint endpos = endbitpos/8;
	nlinfo( "beginpos %d endpos %d beginbitpos %d endbitpos %d", beginpos, endpos, beginbitpos, endbitpos );
	displayByteBits( *(msg.buffer()+beginpos), 8, 8-(beginbitpos-beginpos*8), true );
	const uint8 *p;
	for ( p=msg.buffer()+beginpos+1; p<msg.buffer()+endpos-1; ++p )
	{
		displayByteBits( *p, 8, 0, false );
	}
	if ( endpos > beginpos )
	{
		displayByteBits( *(msg.buffer()+endpos), 8, 0, false );
	}
}

void	CNetworkConnection::decodeVisualProperties( CBitMemStream& msgin )
{
	try
	{
		//nldebug( "pos: %d  len: %u", msgin.getPos(), msgin.length() );
		uint i = 0; 
		while ( true )
		{
			//nlinfo( "Reading pass %u, BEFORE HEADER: pos: %d  len: %u", ++i, msgin.getPosInBit(), msgin.length() * 8 );
			sint beginbitpos = msgin.getPosInBit();

			// Check if there is a new block to read
			if ( msgin.getPosInBit() + (sizeof(TCLEntityId)*8) > msgin.length()*8 )
				return;



			// Header
			TCLEntityId slot;
			msgin.serialAndLog1( slot );

			uint32 associationBits;
			msgin.serialAndLog2( associationBits, 2 );
			//nlinfo( "slot %hu AB: %u", (uint16)slot, associationBits );
			if ( associationBitsHaveChanged( slot, associationBits ) )
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

			uint32 timestampDelta;
			msgin.serialAndLog2( timestampDelta, 4 );
			TGameCycle timestamp = _CurrentServerTick - timestampDelta;
			
			// Tree
			//nlinfo( "AFTER HEADER: posBit: %d pos: %d  len: %u", msgin.getPosInBit(), msgin.getPos(), msgin.length() );			

			TVPNodeClient *currentNode = _VisualPropertyTreeRoot;
			msgin.serialBitAndLog( currentNode->A->BranchHasPayload );
			if ( currentNode->A->BranchHasPayload )
			{
				CActionPosition *ap = (CActionPosition*)CActionFactory::getInstance()->create( slot, ACTION_POSITION_CODE );
				ap->unpack( msgin );
				_PropertyDecoder.receive( _CurrentReceivedNumber, ap );
	#ifdef MEASURE_RECEIVE_DATES
				// Stat log
				if ( LogReceive )
				{
					TRDateState ds( timestamp, currentTime );
					ReceivePosDateLog[slot].push_back( ds );
				}
	#endif
	#ifdef SHOW_PROPERTIES_RECEIVED
				++propReceived[PROPERTY_POSITION];
	#endif

				/*
				 * Set into property database
				 */

				// TEMP
				if ( ap->Position[0]==0 || ap->Position[1]==0 )
					nlwarning( "S%hu: Receiving an invalid position", (uint16)slot );

				if (_DataBase != NULL)
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
				CChange thechange(slot, PROPERTY_POSITION, timestamp, predictedInterval);
				_Changes.push_back( thechange );
				
			}

			currentNode = currentNode->B;
			msgin.serialBitAndLog( currentNode->BranchHasPayload );
			if ( currentNode->BranchHasPayload )
			{
				msgin.serialBitAndLog( currentNode->A->BranchHasPayload );
				if ( currentNode->A->BranchHasPayload )
				{
					CActionSint64 *ac = (CActionSint64*)CActionFactory::getInstance()->createByPropIndex( slot, PROPERTY_ORIENTATION );
					ac->unpack( msgin );

					// Process orientation
					CChange thechange(slot, PROPERTY_ORIENTATION, timestamp);
					_Changes.push_back( thechange );
	#ifdef SHOW_PROPERTIES_RECEIVED
					++propReceived[PROPERTY_ORIENTATION];
	#endif
					if (_DataBase != NULL)
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
				currentNode->B->decodeDiscreetProperties( msgin );
			}
		}
	}
	catch ( EStreamOverflow& )
	{
		// End of stream (saves useless bits)
	}
}

/*
 *
 */
void	CNetworkConnection::decodeDiscreetProperty( CBitMemStream& msgin, TPropIndex propIndex )
{
	//nldebug( "Reading discreet property %hu at bitpos %d", (uint16)propIndex, msgin.getPosInBit() );
	TCLEntityId slot = TVPNodeClient::SlotContext.Slot;
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

			TSheetId newSheetId = (TSheetId)(ac->getValue());
			_IdMap.insert( make_pair( newSheetId, slot) );
			_PropertyDecoder.addEntity( slot, newSheetId );

			// Reset the position update statistical data
			_PosUpdateTicks[slot].clear();
			_PosUpdateIntervals[slot].clear();

			CChange thechange(slot, AddNewEntity);
			_Changes.push_back( thechange );
			break;
		}
		case PROPERTY_MODE:
		{
			// Special case for mode: push theta or pos, then mode
			uint64 mode44 = ac->getValue();
			uint32 modeTimestamp = _CurrentServerTick - (uint32)(((mode44 >> 8) & 0xF));
			uint8 modeEnum = (uint8)(mode44 & 0xFF);
			sint32 x, y;
			bool pushpos;
			if ( modeEnum == MBEHAV::COMBAT_FLOAT )
			{
				//nldebug( "S%hu: Received mode COMBAT_FLOAT with Theta", (uint16)slot ); // TEMP
				CChange thechangeTheta( slot, PROPERTY_ORIENTATION, modeTimestamp );
				_Changes.push_back( thechangeTheta );
			}
			else
			{
				uint16 x16 = (ac->getValue() >> 12) & 0xFFFF;
				uint16 y16 = (ac->getValue() >> 28) & 0xFFFF;
				if ( ! (x16==0 && y16==0) ) // don't set the position if it was not initialized yet
				{
					_PropertyDecoder.decodeAbsPos2D( x, y, x16, y16 );
					nldebug( "%u: S%hu: Received mode with pos %d %d (x16=%hu y16=%u)", _CurrentServerTick, (uint16)slot, x, y, x16, y16 ); // TEMP
					CChange thechangePos( slot, PROPERTY_POSITION, modeTimestamp, 0 );
					_Changes.push_back( thechangePos );
					pushpos = true;
				}
				else
				{
					nldebug( "%u: S%hu: Received mode with null pos", _CurrentServerTick, (uint16)slot ); // TEMP
					pushpos = false;
				}
			}
			CChange thechangeMode( slot, PROPERTY_MODE, modeTimestamp );
			_Changes.push_back( thechangeMode );

			// Set mode value in database
			if (_DataBase != NULL)
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

			if ( modeEnum == MBEHAV::COMBAT_FLOAT )
			{
				// Set theta
				if (_DataBase != NULL)
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
				if ( pushpos && (_DataBase != NULL) )
				{
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
			}
			break;
		}
		default:
		{
			// Process property
			CChange thechange( slot, propIndex, TVPNodeClient::SlotContext.Timestamp );
			_Changes.push_back( thechange );
			if (_DataBase != NULL)
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
//	nlwarning( "%s", ss.str().c_str() );

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

	for (itblock=_Actions.begin(); itblock!=_Actions.end(); ++itblock)
	{
		// if block contains action that are not already stamped, don't send it now
		if ((*itblock).Cycle == 0)
			break;

		// Prevent to send a message too big
		//if (message.getPosInBit() + (*itblock).bitSize() > FrontEndInputBufferSize) // hard version
		if ( message.getPosInBit() > 480*8 ) // easy version
			break;

		if ((*itblock).FirstPacket == 0)
			(*itblock).FirstPacket = _CurrentSendNumber;

		//nlassertex((*itblock).Cycle > lastPackedCycle, ("(*itblock).Cycle=%d lastPackedCycle=%d", (*itblock).Cycle, lastPackedCycle));
		
		lastPackedCycle = (*itblock).Cycle;

		(*itblock).serial(message);
		++numPacked;
	}

	//_PropertyDecoder.send (_CurrentSendNumber, _LastReceivedNumber);
	uint32	length = message.length();
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

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
				uint8	message;
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
					nlwarning("CNET[%p]: connected->probe", this);
					_Changes.push_back(CChange(0, ProbeReceived));
					receiveSystemProbe(msgin);
					return true;
					break;
				case SYSTEM_SYNC_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Synchronize;
					nlwarning("CNET[%p]: connected->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled and state stalled
					_ConnectionState = Stalled;
					nlwarning("CNET[%p]: connected->stalled", this);
					receiveSystemStalled(msgin);
					return true;
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
	uint32			numprobes = _LatestProbes.size();

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
				uint8	message;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync and state synchronize
					_ConnectionState = Synchronize;
					nlwarning("CNET[%p]: probe->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_STALLED_CODE:
					// receive sync, decode sync and state synchronize
					_ConnectionState = Stalled;
					nlwarning("CNET[%p]: probe->stalled", this);
					receiveSystemStalled(msgin);
					return true;
					break;
				case SYSTEM_PROBE_CODE:
					// receive sync, decode sync
					receiveSystemProbe(msgin);
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

	return false;
}





//
// Stalled state
//

void	CNetworkConnection::receiveSystemStalled(CBitMemStream &msgin)
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
				uint8	message;
				msgin.serial(message);

				switch (message)
				{
				case SYSTEM_SYNC_CODE:
					// receive sync, decode sync and state synchronize
					_ConnectionState = Synchronize;
					nlwarning("CNET[%p]: stalled->synchronize", this);
					receiveSystemSync(msgin);
					return true;
					break;
				case SYSTEM_PROBE_CODE:
					// receive sync, decode sync
					_ConnectionState = Probe;
					nlwarning("CNET[%p]: stalled->probe", this);
					receiveSystemProbe(msgin);
					break;
				case SYSTEM_STALLED_CODE:
					// receive stalled, decode stalled
					receiveSystemStalled(msgin);
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

	return false;
}





//
// encoding / decoding methods
//

bool	CNetworkConnection::decodeHeader(CBitMemStream &msgin)
{
	if (_DecodedHeader)
		return true;

	++_TotalMessages;

	_LastReceivedTime = _UpdateTime;

	msgin.serial (_CurrentReceivedNumber);
	msgin.serial (_SystemMode);

	_MeanPackets.update((float)_CurrentReceivedNumber, ryzomGetLocalTime ());

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

	// display info on this packet
	//nlinfo("CNET[%p] received packet %d, %s mode - LastReceivedAck=%d", this, _CurrentReceivedNumber, _SystemMode ? "SYTEM" : "NORMAL", _LastReceivedAck);

	// todo doesn't work if we receive the packet in bad order or 2 same packet

	if (_CurrentReceivedNumber > _LastReceivedNumber+1)
	{
		// we lost some messages...
		nlwarning ("CNET[%p] lost messages server->client [%d; %d]", this, _LastReceivedNumber+1, _CurrentReceivedNumber-1);
		_MeanLoss.update((float)(_CurrentReceivedNumber-_LastReceivedNumber-1), ryzomGetLocalTime ());
	}
	else if (_CurrentReceivedNumber == _LastReceivedNumber)
	{
		// we receive the same packet that the last one
		nlwarning ("CNET[%p] awaiting packet %d, received packet %d", this, _LastReceivedNumber+1, _CurrentReceivedNumber);
		return false;
	}
	else if (_CurrentReceivedNumber < _LastReceivedNumber)
	{
		// it's an older message than the current
		nlwarning ("CNET[%p] received an old message, awaiting packet %d, received packet %d", this, _LastReceivedNumber+1, _CurrentReceivedNumber);
		return false;
	}

	// don't acknowledge system messages and normal messages in 
	// because this will disturb impulsion from frontend, that will interpret it as if previous messages were ok
	bool	ackBool = (_SystemMode || (_ConnectionState != Connected && _ConnectionState != Synchronize) ? false : true);
	uint	ackBit = (ackBool ? 1 : 0);

	if (_CurrentReceivedNumber - _LastReceivedNumber < 32)
	{
		_AckBitMask <<= _CurrentReceivedNumber - _LastReceivedNumber;
		_AckBitMask |= ackBit << (_CurrentReceivedNumber - _LastReceivedNumber - 1);
	}
	else 
	{
		_AckBitMask = (_CurrentReceivedNumber - _LastReceivedNumber == 32) ? 0x80000000 : 0x00000000;
	}

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
	CActionGeneric *ag = (CActionGeneric *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_CODE);
	if( ag != NULL ) //TODO: see that with oliver...
	{
//	nlassert (ag != NULL);
		ag->set(msg);
		push(ag);
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

			CAction	*dummy = CActionFactory::getInstance()->create(INVALID_SLOT, ACTION_DUMMY_CODE);
			((CActionDummy*)dummy)->Dummy1 = _DummySend++;
			push(dummy);

			_Actions.back().Cycle = cycle;
			//nlinfo("-BEEN- setcycle [size=%d, cycle=%d]", _Actions.size(), _Actions.empty() ? 0 : _Actions.back().Cycle);
		}

		if (_ConnectionState == Connected)
			sendNormalMessage();
	}
	catch (ESocket &e)
	{
		_ConnectionState = Disconnect;
	}
}


void	CNetworkConnection::send()
{
	try
	{
		if (_ConnectionState == Connected)
			sendNormalMessage();
	}
	catch (ESocket &e)
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
	_Connection.send (message.buffer(), length);
	//sendUDP (&(_Connection), message.buffer(), length);
	statsSend(length);

	updateBufferizedPackets ();
	nlinfo("CNET[%p]: sent DISCONNECTION", this);
}

void	CNetworkConnection::disconnect()
{
#ifdef MEASURE_RECEIVE_DATES
	if ( LogReceive )
	{
		displayReceiveLog();
	}
#endif

	if (_ConnectionState == NotInitialised ||
		_ConnectionState == NotConnected ||
		_ConnectionState == Authenticate ||
		_ConnectionState == Disconnect)
	{
		nlwarning("Unable to disconnect(): not connected yet, or already disconnected.");
		return;
	}

	sendSystemDisconnection();
	_Connection.close();
	_ConnectionState = Disconnect;
}

//

void	CNetworkConnection::displayAllocationStats()
{
	nlinfo("CNET[%p]: %d queued blocks, %d changes", this, _Actions.size(), _Changes.size());
}

string	CNetworkConnection::getAllocationStats()
{
	char	buf[128];
	sprintf(buf, "CNET[%p]: %d queued blocks, %d changes", this, _Actions.size(), _Changes.size());
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
	TempSize += agmp->PartCont.size();

	if (NbCurrentBlock == NbBlock)
	{
		// reform the total action

		nldebug("CLMPNET[%p]: Received a TOTAL generic action MP size: number %d nbblock %d", this,  agmp->Number, NbBlock);

		CBitMemStream bms(true);

		uint8 *ptr = bms.bufferToFill (TempSize);

		for (uint i = 0; i < Temp.size (); i++)
		{
			memcpy (ptr, &(Temp[i][0]), Temp[i].size());
			ptr += Temp[i].size();
		}

		NbBlock = 0xFFFFFFFF;

		nldebug("CLMPNET[%p]: Received a generic action size %d", this, bms.length());
		// todo interface api, call a user callback

		if (parent->_ImpulseCallback != NULL)
			parent->_ImpulseCallback(bms, parent->_LastReceivedNumber, parent->_ImpulseArg);

	}
}

void CNetworkConnection::genericAction (CActionGenericMultiPart *agmp)
{
	// manage a generic action (big one that comes by blocks)
	vector<uint8> &v = agmp->PartCont;

	nldebug("CLMPNET[%p]: Received a generic action MP size %d: number %d part %d nbblock %d", this, v.size(), agmp->Number, agmp->Part, agmp->NbBlock);

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
		uint8 slot = (uint8)atoi( args[0].c_str() );
		PosUpdateIntervalGraph = new CSlotGraph( "Interval", 350, 2, 100, 200, CRGBA(128,0,0,64), 1000, 20, true, slot );
		PosUpdatePredictionGraph = new CSlotGraph( "                    Prediction", 350, 2, 100, 200, CRGBA(0,0,128,64), 1000, 20, true, slot );
	}
	return true;
}


// ***************************************************************************
sint32		CNetworkConnection::convertToSmoothServerTick(NLMISC::TGameCycle t) const
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
	sint32	actualST= _CurrentServerTick*SMOOTH_SERVER_TICK_PER_TICK;

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
		sint32	factor;
		sint32	errorDiff= _CurrentSmoothServerTick-actualST;
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
			factor= sint32(f*65536);
		}
		// Else the estimation is too early, slowDown
		else
		{
			float	f= (float)(errorDiff-SMOOTH_SERVER_TICK_WINDOW)/(SMOOTH_SERVER_TICK_DIFF_MAX-SMOOTH_SERVER_TICK_WINDOW);
			f= 1-f;
			factor= sint32(f*65536);
		}

		// Update the Smooth
		_CurrentSmoothServerTick+= ((deltaT*SMOOTH_SERVER_TICK_PER_TICK*factor)/getMsPerTick()) >> 16;
	}
}
