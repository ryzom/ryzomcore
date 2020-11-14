/** \file simulated_client.cpp
 * Simulate a client network connection.
 *
 * $Id: simulated_client.cpp
 */

/////////////
// INCLUDE //
/////////////
#include "nel/net/module_manager.h"
#include "simulated_client.h"
#include "client_cfg.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "nel/misc/common.h"

#include "../client/dummy_progress.h"

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;


CScriptStack* CScriptStack::_Instance = NULL;

CVariable<bool> VerboseScript( "drone", "VerboseScript", "", false, 0, true );
CVariable<bool> AutoMove( "drone", "AutoMove", "", true, 0, true );


// Singleton
CGenericXmlMsgHeaderManager	GenericMsgHeaderMngr;

// Context for non-OO functions (callbacks...)
CSimulatedClient *CSimulatedClient::_CurrentContext = NULL;

// For cdb interface
CDummyProgress progressCallback;

const float SpeedMPerGc = 0.1f;

// impuse callbacks
void impulseCallBack(NLMISC::CBitMemStream &impulse, sint32 packet, void *arg);
void impulseUserChars(NLMISC::CBitMemStream &impulse);
void impulseNoUserChar(NLMISC::CBitMemStream &impulse);
void impulseServerReady(NLMISC::CBitMemStream &impulse);
void impulseUserChar(NLMISC::CBitMemStream &impulse);
void impulseTP(NLMISC::CBitMemStream &impulse);
void impulseCorrectPos(NLMISC::CBitMemStream &impulse);
void impulseServerQuitOk(NLMISC::CBitMemStream &impulse);
void impulseChat(NLMISC::CBitMemStream &impulse);
void impulseChat2(NLMISC::CBitMemStream &impulse);
void impulseTell(NLMISC::CBitMemStream &impulse);
void impulseTell2(NLMISC::CBitMemStream &impulse);
void impulseFarTell(NLMISC::CBitMemStream &impulse);
void impulseDynString(NLMISC::CBitMemStream &impulse);
void impulsePhraseSend(NLMISC::CBitMemStream &impulse);
void impulseStringResp(NLMISC::CBitMemStream &impulse);

// these callbacks are defined in client/gateway_fec_transport
void cbImpulsionGatewayOpen(NLMISC::CBitMemStream &bms);
void cbImpulsionGatewayMessage(NLMISC::CBitMemStream &bms);
void cbImpulsionGatewayClose(NLMISC::CBitMemStream &bms);


CSimulatedClient::CSimulatedClient( uint id ) :
	_UserId(~0),
	_CurrentLoginState(LSInitial),
//	_NearestSlot(0),
//	_NbUsedSlots(0),
	_ModuleGateway(NULL),
	_ModuleTest(NULL),
	_GatewayTransport(NULL),
	_ProcessingScript(false)
{
	setIdAndName(id);
}

CSimulatedClient::~CSimulatedClient()
{
	unregisterCommandsHandler();

	_CurrentContext = this;
	if (_ModuleGateway != NULL)
	{
		IModule *mod = _ModuleGateway;
		_ModuleGateway = NULL;

		IModuleManager::getInstance().deleteModule(mod);
	}
	if (_ModuleTest != NULL)
	{
		IModule *mod = _ModuleTest;
		_ModuleTest = NULL;

		IModuleManager::getInstance().deleteModule(mod);
	}
	_CurrentContext = NULL;
}

/*
 *
 */
std::string CSimulatedClient::name() const
{
	return _Name;
}

/*
 *
 */
void	CSimulatedClient::setIdAndName( uint id )
{
	_Id = id;
	if ( _Id == DISABLED_CLIENT )
		_Name = "[disabled]";
	else		
		_Name = NLMISC::toString( "%u", _Id );
}

/*
 * Send
 */
void CSimulatedClient::send(NLMISC::TGameCycle gameCycle)
{
	// wait till next server is received
	if (_NetworkConnection.lastSentCycle() < gameCycle)
	{
		//nlinfo ("Try to CNetManager::send(%d) _LastSentCycle=%d more than one time with the same game cycle, so we wait new game cycle to send", gameCycle, _LastSentCycle);
		// Don't loop to allow interlacing multiple clients
		/*while (_NetworkConnection.lastSentCycle() >= gameCycle)
		{
			// Update network.
			update();
			// Send dummy info
			send();
			// Do not take all the CPU.
			nlSleep(100);

			gameCycle = _NetworkConnection.getCurrentServerTick();
		}*/
		_NetworkConnection.send(gameCycle);
	}
}

/*
 * Send
 */
void CSimulatedClient::send()
{
	_NetworkConnection.send();
}

/*
 * Update network (return the game cycle delta between the previous call)
 */
sint CSimulatedClient::updateNetwork()
{
	TGameCycle serverTick = _NetworkConnection.getCurrentServerTick();

	// Read incoming data
	_NetworkConnection.update();

	processVision();

	// Check if we can send another dated block
	if (_NetworkConnection.getCurrentServerTick() != serverTick)
	{
		if ( UserEntity.isInitialized() && (_CurrentLoginState < LSQuitRequested) )
		{
			// Push current pos
			CBitMemStream out;
			UserEntity.sendToServer(out);
			_NetworkConnection.push(out);
		}

		// Send
		send( _NetworkConnection.getCurrentServerTick() );
	}
	else
	{
		// Don't send, as we don't want to wait 30 ms because we interlace all clients' connections
		//send(); // send dummy info
	}
	
	// Update the DT T0 and T1 global variables			
	updateClientTime();

	return (sint)_NetworkConnection.getCurrentServerTick() - serverTick;
}

/*
 *
 */
CEntityDrone* CSimulatedClient::getSlot( CLFECOMMON::TCLEntityId slot )
{
	if (!_NetworkConnection.getPropertyDecoder().isUsed(slot))
		return NULL;

	return &_VisionSlots[slot];
}

/*
 *
 */
void CSimulatedClient::processVision()
{
	const std::vector<CNetworkConnection::CChange>& changes = _NetworkConnection.getChanges();
	for (std::vector<CNetworkConnection::CChange>::const_iterator itc=changes.begin(); itc!=changes.end(); ++itc)
	{
		switch (itc->Property)
		{
		case CNetworkConnection::AddNewEntity:
			break;
		case CNetworkConnection::RemoveOldEntity:
			break;
		case CLFECOMMON::PROPERTY_POSITION:
//			{
//				CLFECOMMON::TCLEntityId newNearestSlot = getNearestEntity();
//				if (newNearestSlot != _NearestSlot)
//				{
//					_NearestSlot = newNearestSlot;
//					double dist = (getPosition(_NearestSlot) - UserEntity.pos()).norm();
//					nldebug("Client %s: Now the nearest entity is slot %u at %f m (%s)", name().c_str(), _NearestSlot, dist, getPosition(_NearestSlot).asVector().toString().c_str());
//				}
//			}
			break;
		default:  // etc.
			break;
		}
	}

//	// Slot count
//	uint newNbUsedSlots = 0;
//	for (uint slot=1; slot!=256; ++slot)
//	{
//		if (_NetworkConnection.getPropertyDecoder().isUsed(slot))
//			++newNbUsedSlots;
//	}
//	if (newNbUsedSlots != _NbUsedSlots)
//	{
//		_NbUsedSlots = newNbUsedSlots;
//		nldebug("Client %s now has %u slots", name().c_str(), _NbUsedSlots);
//	}

	_NetworkConnection.clearChanges();
}

/*
 *
 */
void	CSimulatedClient::processScript()
{
	if (_Script.empty())
		return;

	CSString lineToExecute = _Script.front(); //"0.getNearestEntity; echo"
	lineToExecute = lineToExecute.replace("this.", (name()+".").c_str());
	_ProcessingScript = true;
	if (VerboseScript.get() && (lineToExecute.substr(0, 6) != "quiet "))
		nlinfo("Client %s runs: %s", name().c_str(), lineToExecute.c_str());
	if (ICommand::execute(lineToExecute, NLMISC::InfoLog(), true))
		_Script.pop_front();
	else
		_Script.clear();
	_ProcessingScript = false;
}

/*
 *
 */
CVectorD		CSimulatedClient::getPosition( CLFECOMMON::TCLEntityId slot ) const
{
	double x = ((double)getVisualProp(slot, CLFECOMMON::PROPERTY_POSX)) / 1000.0;
	double y = ((double)getVisualProp(slot, CLFECOMMON::PROPERTY_POSY)) / 1000.0;
	double z = ((double)getVisualProp(slot, CLFECOMMON::PROPERTY_POSZ)) / 1000.0;
	return CVectorD(x, y, z);
}

/*
 *
 */
CLFECOMMON::TCLEntityId	CSimulatedClient::getNearestEntity( NLMISC::CSheetId requiredSheet ) const
{
	double minDist = DBL_MAX;
	uint nearestSlot = ~0;
	for (uint slot=1; slot!=256; ++slot) // slot 0 is UserEntity
	{
		if (!_NetworkConnection.getPropertyDecoder().isUsed(slot))
			continue;
		if ((requiredSheet != CSheetId::Unknown) &&
			((uint32)getVisualProp(slot, CLFECOMMON::PROPERTY_SHEET) != requiredSheet.asInt()))
			continue;

		CVectorD pos = getPosition(slot);
		double dist = (pos - UserEntity.pos()).norm();
		if (dist < minDist)
		{
			minDist = dist;
			nearestSlot = slot;
		}
	}
	return nearestSlot;
}


/*
 *
 */
void CSimulatedClient::selectCharAndEnter( uint charIndex )
{
	CBitMemStream out;
	nlverify (GenericMsgHeaderMngr.pushNameToStream ("CONNECTION:SELECT_CHAR", out));

	CSelectCharMsg	SelectCharMsg;
	SelectCharMsg.c = (uint8)charIndex;
	out.serial(SelectCharMsg);
	_NetworkConnection.push(out);

	CBitMemStream out2;
	nlverify(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:ENTER", out2));
	_NetworkConnection.push(out2);

	send( _NetworkConnection.getCurrentServerTick() );
}

/*
 *
 */
void CSimulatedClient::requestCommandA( const std::string& arg )
{
	CBitMemStream out;
	nlverify(GenericMsgHeaderMngr.pushNameToStream("COMMAND:ADMIN", out));

	string::size_type p = arg.find( ' ');
	string cmd = arg.substr( 0, p );
	string args = arg.substr( p + 1 );
	bool onTarget = false;
	out.serial (onTarget);
	out.serial (cmd);
	out.serial (args);
	_NetworkConnection.push (out);
}

/*
 *
 */
void CSimulatedClient::setInitPos( NLMISC::CBitMemStream &impulse )
{
	COfflineEntityState posState;
	uint8 serverSeasonValue;
	uint32 userRole;
	bool isInRingSession;
	TSessionId highestMainlandSessionId;
	uint32 firstConnectedTime;
	uint32 playedTime;
	CUserCharMsg::read( impulse, posState, serverSeasonValue, userRole, isInRingSession, highestMainlandSessionId, firstConnectedTime, playedTime );
	
	UserEntity.pos(CVectorD((float)posState.X/1000.0f, (float)posState.Y/1000.0f, (float)posState.Z/1000.0f));
	UserEntity.front(CVector((float)cos(posState.Heading), (float)sin(posState.Heading), 0.f));
	UserEntity.dir(UserEntity.front());
	_NetworkConnection.setReferencePosition(UserEntity.pos());
}

/*
 *
 */
void CSimulatedClient::sendClientReady()
{
	CSimulatedClient::currentContext()->setCurrentLoginState( CSimulatedClient::LSClientReady );
	//checkHandshake( impulse );

	CBitMemStream out3;
	nlverify(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:READY", out3));
	out3.serial(ClientCfg.LanguageCode);
	_NetworkConnection.push(out3);

	// Enqueue PostEnterCommands
	CConfigFile::CVar *postEnterCommands = ClientCfg.ConfigFile.getVarPtr( "PostEnterCommands" );
	if (postEnterCommands)
	{
		for (uint i=0; i!=postEnterCommands->size(); ++i)
			_PendingCommands.push_back( postEnterCommands->asString() );
	}
}

/*
 *
 */
void CSimulatedClient::disconnect()
{
	 _NetworkConnection.disconnect();
}

/*
 *
 */
void CSimulatedClient::tp( const CVectorD& dest, bool ackTP )
{
	// Change pos
	_NetworkConnection.setReferencePosition(dest);
	UserEntity.pos(dest);

	// Rotate direction (in case of position correction due to collision)
	_Direction = (TDirection)(((uint)_Direction) + 1);
	if ( _Direction > DMaxDir )
		_Direction = (TDirection)0;

	// Acknowledge (for a real TP)
	if ( ackTP )
	{
		CBitMemStream out;
		nlverify(GenericMsgHeaderMngr.pushNameToStream("TP:ACK", out));
		_NetworkConnection.push(out);
	}
}

/*
 *
 */
bool CSimulatedClient::autoLogin (const string &cookie, const string &fsaddr, bool firstConnection)
{
	string UsedFSAddr;
	if(!fsaddr.empty())
	{
		// If we have a front end address from command line, use this one
		UsedFSAddr = fsaddr;
	}
	else
	{
		// Otherwise, use the front end address from configfile
		UsedFSAddr = ClientCfg.FSHost;
	}

	if (UsedFSAddr.find(":") == string::npos)
		UsedFSAddr += string(":47851");

	// Connection
	if (firstConnection)
	{
		_NetworkConnection.init (cookie, UsedFSAddr);
		string result;
		_NetworkConnection.connect (result);

		if (!result.empty())
		{
			nlerror ("Connection failed: %s", result.c_str());
			return false;
		}

		// Ok the client is connected

		// Set the impulse callback.
		_NetworkConnection.setImpulseCallback( impulseCallBack );
		// Set the database.
		//_NetworkConnection.setDataBase( _IngameDbMngr.getNodePtr() );

		// init the string manager cache.
		//STRING_MANAGER::CStringManagerClient::instance()->initCache(UsedFSAddr, ClientCfg.LanguageCode);
	}

	return true;
}

/*
 *
 */
bool CSimulatedClient::updateEnterGame()
{
	try
	{
		// Update network
		updateNetwork();

		// Process log-in
		switch ( _CurrentLoginState )
		{
		case LSUserCharsReceiving:
			_NetworkConnection.flushSendBuffer(); // clear sending buffer that may contain prevous QUIT_GAME when getting back to the char selection screen
			selectCharAndEnter( ClientCfg.SelectCharacter );
			_CurrentLoginState = LSUserCharsReceived;
			break;
		case LSClientReady:
			return true;
		case LSNoUserChar:
			nlwarning( "Client %s has no character at index %d", name().c_str(), ClientCfg.SelectCharacter );
			setIdAndName( DISABLED_CLIENT );
			return false;
		default:;
		}

		// Check we are still connected
		if ( _NetworkConnection.getConnectionState() == CNetworkConnection::Disconnect )
		{
			nlwarning( "Client %s disconnected by the server at state %u", name().c_str(), _CurrentLoginState );
			setIdAndName( DISABLED_CLIENT );
			return false;
		}
	}
	catch ( EBlockedByFirewall& )
	{
		nlerror( "Client %s blocked by firewall at state %u", name().c_str(), _CurrentLoginState );
		setIdAndName( DISABLED_CLIENT );
		return false;
	}
	return true;
}

/*
 *
 */
bool CSimulatedClient::updateInGame()
{
	// Update network
	sint gcDelta = updateNetwork();

	// Check we are still connected, otherwise quit
	if (_NetworkConnection.getConnectionState() == CNetworkConnection::Disconnect)
	{
		if ( _CurrentLoginState < LSQuitRequested )
			nlwarning( "Client %s disconnected by the server while in game", name().c_str() );
		setIdAndName( DISABLED_CLIENT );
		_CurrentLoginState = LSQuitting;
		nlinfo( "Client %s: quitting", name().c_str() );
		return false;
	}

	// Process script
	processScript();

	// Apply pending "a" commands (from the command line)
	for ( std::vector<std::string>::const_iterator ipc=_PendingCommands.begin(); ipc!=_PendingCommands.end(); ++ipc )
	{
		nldebug( "Client %s: %s", name().c_str(), (*ipc).c_str() );
		requestCommandA( (*ipc) );
	}
	_PendingCommands.clear();

	// Move
	if ( AutoMove.get() && (gcDelta != 0) && UserEntity.isInitialized() && (_CurrentLoginState < LSQuitRequested) )
	{
		CVectorD pos = UserEntity.pos();
		// TODO: handle position corrections by the server
		switch ( _Direction )
		{
		case DNE: pos.y -= SpeedMPerGc * ((float)gcDelta);
				  pos.x += SpeedMPerGc * ((float)gcDelta); break;
		case DSE: pos.x += SpeedMPerGc * ((float)gcDelta);
				  pos.y += SpeedMPerGc * ((float)gcDelta); break;
		case DSW: pos.y += SpeedMPerGc * ((float)gcDelta);
				  pos.x -= SpeedMPerGc * ((float)gcDelta); break;
		case DNW: pos.x -= SpeedMPerGc * ((float)gcDelta);
				  pos.y -= SpeedMPerGc * ((float)gcDelta); break;
		}
		UserEntity.pos( pos );
	}

	return true;
}

/*
 *
 */
/*static*/ void CSimulatedClient::initNetwork()
{
	GenericMsgHeaderMngr.init( CPath::lookup("msg.xml") );
	GenericMsgHeaderMngr.setCallback( "CONNECTION:USER_CHARS", impulseUserChars );
	GenericMsgHeaderMngr.setCallback( "CONNECTION:NO_USER_CHAR", impulseNoUserChar);
	GenericMsgHeaderMngr.setCallback( "CONNECTION:USER_CHAR", impulseUserChar );
	GenericMsgHeaderMngr.setCallback( "CONNECTION:READY", impulseServerReady );
	GenericMsgHeaderMngr.setCallback( "CONNECTION:SERVER_QUIT_OK", impulseServerQuitOk );
	GenericMsgHeaderMngr.setCallback( "TP:DEST", impulseTP );
	GenericMsgHeaderMngr.setCallback( "TP:CORRECT", impulseCorrectPos );

	GenericMsgHeaderMngr.setCallback("STRING:CHAT",				impulseChat);
	GenericMsgHeaderMngr.setCallback("STRING:CHAT2",			impulseChat2);
	GenericMsgHeaderMngr.setCallback("STRING:TELL",				impulseTell);
	GenericMsgHeaderMngr.setCallback("STRING:TELL2",			impulseTell2);
	GenericMsgHeaderMngr.setCallback("STRING:FAR_TELL",			impulseFarTell);
	GenericMsgHeaderMngr.setCallback("STRING:DYN_STRING",		impulseDynString);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:PHRASE_SEND", impulsePhraseSend);
	GenericMsgHeaderMngr.setCallback("STRING_MANAGER:STRING_RESP", impulseStringResp);

	GenericMsgHeaderMngr.setCallback( "MODULE_GATEWAY:FEOPEN",		cbImpulsionGatewayOpen);
	GenericMsgHeaderMngr.setCallback( "MODULE_GATEWAY:GATEWAY_MSG", cbImpulsionGatewayMessage );
	GenericMsgHeaderMngr.setCallback( "MODULE_GATEWAY:FECLOSE",		cbImpulsionGatewayClose );
	srand( (uint)CTime::getLocalTime() );
}


/*
 *
 */
bool CSimulatedClient::start()
{
	_CurrentContext = this;
	//_IngameDbMngr.init( CPath::lookup("database.xml"), progressCallback );

	// login (authentication)
	/*string ClientApp = ClientCfg.ConfigFile.getVar("Application").asString(0);
	string resCL = checkLogin( "olivierc", "miller", ClientApp );
	if (!resCL.empty())
	{
		nlwarning( "%u: Can't login: %s", _Id, resCL.c_str() );
		return false;
	}*/

	// connection
	NLNET::CLoginCookie cookie;
	if (ClientCfg.ConfigFile.exists ("UserId"))
		_UserId = ClientCfg.ConfigFile.getVar ("UserId").asInt() + CSimulatedClient::currentContext()->id();
	else
		_UserId = _Id;
	cookie.set(0, 0x1234567, _UserId);

	bool res = autoLogin( cookie.setToString(), "", true );
	_Direction = (TDirection)(uint)frand( DMaxDir+1 );
	if ( _Direction > DMaxDir )
		_Direction = DMaxDir;

	// set the unique name root
	IModuleManager::getInstance().setUniqueNameRoot(toString("U%u", _Id));
	string gwName = toString("gw%u", _Id);
	string modName = toString("testClient%u", _Id);

	// create a local gateway module
	_ModuleGateway = IModuleManager::getInstance().createModule("StandardGateway", gwName, "");
	nlassert(_ModuleGateway != NULL);

	// create a remote gateway module on the FES
	CCommandRegistry::getInstance().execute(gwName+".transportAdd FEClient fec", *InfoLog);
	CCommandRegistry::getInstance().execute(gwName+".transportCmd fec(open)", *InfoLog);
	CCommandRegistry::getInstance().execute(gwName+".plug "+gwName, *InfoLog);

	// create a local test module
	_ModuleTest = IModuleManager::getInstance().createModule("FEClientModuleTest", modName, "");
	CCommandRegistry::getInstance().execute(modName+".plug "+gwName, *InfoLog);
	
	IModuleManager::getInstance().setUniqueNameRoot("");
	
	registerCommandsHandler();


	// Read SimulatedClientScript
	CConfigFile::CVar *script = ClientCfg.ConfigFile.getVarPtr( "SimulatedClientScript" );
	if (script)
	{
		for (uint i=0; i!=script->size(); ++i)
			_Script.push_back( script->asString(i) );
	}


	_CurrentContext = NULL;
	return res;
}

/*
 *
 */
bool CSimulatedClient::update()
{
	if ( _Id == DISABLED_CLIENT )
		return false;

	_CurrentContext = this;
	bool res = false;
	if ( _CurrentLoginState < LSClientReady )
	{
		if ( ! updateEnterGame() )
			goto exitUpdate;
	}
	else
	{
		if ( ! updateInGame() )
			goto exitUpdate;
	}

	res = true;
exitUpdate:
	_CurrentContext = NULL;
	return res;
}

/*
 * After this, update of network must still be done to send the request and receive the answer.
 */
void CSimulatedClient::requestQuit()
{
	if ( _CurrentLoginState < LSUserCharsReceiving)
	{
		// use low level disconnection
		_NetworkConnection.disconnect();
	}
	else if ( _CurrentLoginState < LSQuitRequested )
	{
		nlinfo("Client %s: Quit Request", name().c_str() );
		CBitMemStream out;
		nlverify(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:CLIENT_QUIT_REQUEST", out));
		bool bypassDisconnectionTimer = false;
		out.serial(bypassDisconnectionTimer); // must always be written because of msg.xml (or could have a special handler in the FS)
		uint32 u32;
		uint16 u16;
		out.serial(u32);
		out.serial(u16);
		_NetworkConnection.push(out);
		_CurrentLoginState = LSQuitRequested;
	}
}

void CSimulatedClient::sendMsgToServer(const std::string &sMsg, uint8 u8)
{
	CBitMemStream out;
	nlverify(GenericMsgHeaderMngr.pushNameToStream(sMsg, out));
	out.serial(u8);
	_NetworkConnection.push(out);
}

std::string makeCharNameFromUId(uint32 userId, uint slot)
{
	// Convert digits to alphanumeric chars
	std::string s = toString("%u%u", userId, slot);
	for (uint i=0; i!=s.size(); ++i)
	{
		s[i] = 'a' + s[i] - '0';
	}
	return "U" + s;
}

/*
 *
 */
void CSimulatedClient::requestCreateChar()
{
	string charname = makeCharNameFromUId(_UserId, ClientCfg.SelectCharacter);
	nlinfo("Client %s: Creating char %u-%u (%s)...", name().c_str(), _UserId, ClientCfg.SelectCharacter, charname.c_str());

	// Create the message for the server to create the character.
	CBitMemStream out;
	nlverify(GenericMsgHeaderMngr.pushNameToStream("CONNECTION:CREATE_CHAR", out));

	CCharacterSummary CS;
	CS.People = EGSPD::CPeople::Fyros;
	CS.Mainland = ClientCfg.CreateOnMainland;
	CS.Title = CHARACTER_TITLE::Refugee;
	CS.CharacterSlot = ClientCfg.SelectCharacter;

	CS.Name = ucstring( charname );

	CCreateCharMsg CreateCharMsg;
	CreateCharMsg.setupFromCharacterSummary(CS);
	CreateCharMsg.Slot = CS.CharacterSlot;
	CreateCharMsg.NbPointFighter	= 2;
	CreateCharMsg.NbPointCaster		= 1;
	CreateCharMsg.NbPointCrafter	= 1;
	CreateCharMsg.NbPointHarvester	= 1;
	CreateCharMsg.StartPoint = RYZOM_STARTING_POINT::fyros_start;
	CreateCharMsg.serialBitMemStream (out);
	//noUserChar = userChar = false;

	_NetworkConnection.push(out);
	_NetworkConnection.send(_NetworkConnection.getCurrentServerTick());

	// Enqueue PostCreateEnterCommands
	CConfigFile::CVar *postCreateCommands = ClientCfg.ConfigFile.getVarPtr( "PostCreateEnterCommands" );
	if (postCreateCommands)
	{
		for (uint i=0; i!=postCreateCommands->size(); ++i)
			_PendingCommands.push_back( postCreateCommands->asString(i) );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

// these callbacks are defined in client/gateway_fec_transport
void cbImpulsionGatewayOpen(NLMISC::CBitMemStream &bms);
void cbImpulsionGatewayMessage(NLMISC::CBitMemStream &bms);
void cbImpulsionGatewayClose(NLMISC::CBitMemStream &bms);

/*
 *
 */
void impulseCallBack(NLMISC::CBitMemStream &impulse, sint32 packet, void *arg)
{
	GenericMsgHeaderMngr.execute(impulse);
}

/*
 *
 */
void impulseUserChars(NLMISC::CBitMemStream &impulse)
{
	uint8 serverPeopleActive, serverCareerActive;
	impulse.serial(serverPeopleActive);
	impulse.serial(serverCareerActive);
	// read characters summary	
	std::vector<CCharacterSummary> CharacterSummaries;
	CharacterSummaries.clear();
	impulse.serialCont (CharacterSummaries);
	//readPrivileges(impulse);
	//impulse.serialCont(Mainlands);
	if (CharacterSummaries[0].SheetId == CSheetId::Unknown)
		CSimulatedClient::currentContext()->requestCreateChar();
	else
		CSimulatedClient::currentContext()->setCurrentLoginState( CSimulatedClient::LSUserCharsReceiving );
}

/*
 *
 */
void impulseNoUserChar(NLMISC::CBitMemStream &impulse)
{
	//uint8 serverPeopleActive, serverCareerActive;
	//impulse.serial(ServerPeopleActive);
	//impulse.serial(ServerCareerActive);
	//readPrivileges(impulse);
	//CharacterSummaries.clear();
	CSimulatedClient::currentContext()->setCurrentLoginState( CSimulatedClient::LSNoUserChar );
}

/*
 *
 */
void impulseServerReady(NLMISC::CBitMemStream &impulse)
{
	// Must be sent now to insure it's transmitted by the FS by Id, not by Uid
	CSimulatedClient::currentContext()->sendClientReady();
}

/*
 *
 */
void impulseUserChar(NLMISC::CBitMemStream &impulse)
{
	CSimulatedClient::currentContext()->setInitPos( impulse );
}

/*
 *
 */
void impulseTP(NLMISC::CBitMemStream &impulse)
{
	sint32 x, y, z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	CVectorD dest = CVectorD((float)x/1000.0f, (float)y/1000.0f, (float)z/1000.0f);

	CSimulatedClient::currentContext()->tp( dest, true );

	bool useHeading;
	impulse.serialBit( useHeading );
	if( useHeading )
	{
		float angle;
		impulse.serial(angle);
		CVector ori = CVector((float)cos(angle), (float)sin(angle), 0.0f);
		ori.normalize();
		CSimulatedClient::currentContext()->UserEntity.dir( ori );
		CSimulatedClient::currentContext()->UserEntity.front( ori );
	}
}

/*
 *
 */
void impulseCorrectPos(NLMISC::CBitMemStream &impulse)
{
	sint32 x, y, z;
	impulse.serial(x);
	impulse.serial(y);
	impulse.serial(z);
	CVectorD dest = CVectorD((float)x/1000.0f, (float)y/1000.0f, (float)z/1000.0f);

	CSimulatedClient::currentContext()->tp( dest, false );
}

/*
 *
 */
void impulseChat(NLMISC::CBitMemStream &impulse)
{
	nldebug("Client %s received CHAT", CSimulatedClient::currentContext()->name().c_str() );
}

/*
 *
 */
void impulseTell(NLMISC::CBitMemStream &impulse)
{
	nldebug("Client %s received TELL", CSimulatedClient::currentContext()->name().c_str() );
}

/*
 *
 */
void impulseFarTell(NLMISC::CBitMemStream &impulse)
{
	nldebug("Client %s received FAR_TELL", CSimulatedClient::currentContext()->name().c_str() );
}

/*
 *
 */
void impulseChat2(NLMISC::CBitMemStream &impulse)
{
	CChatMsg2 chatMsg;
	impulse.serial( chatMsg );
	//nldebug("Client %s received CHAT2 %u %s", CSimulatedClient::currentContext()->name().c_str(), chatMsg.PhraseId, chatMsg.CustomTxt.toString().c_str() );
}

/*
 *
 */
void impulseTell2(NLMISC::CBitMemStream &impulse)
{
	nldebug("Client %s received TELL2", CSimulatedClient::currentContext()->name().c_str() );
}

/*
 *
 */
void impulseDynString(NLMISC::CBitMemStream &impulse)
{
	uint32 phraseID;
	impulse.serial(phraseID);
	//nldebug("Client %s received DYN_STRING %u", CSimulatedClient::currentContext()->name().c_str(), phraseID );
}

/*
 *
 */
void impulsePhraseSend(NLMISC::CBitMemStream &impulse)
{
	uint32 dynId, stringId;
	impulse.serial(dynId);
	impulse.serial(stringId);
	CSimulatedClient::currentContext()->requestString(stringId);
	//nldebug("Client %s received PHRASE_SEND %u %u", CSimulatedClient::currentContext()->name().c_str(), dynId, stringId);
}

/*
 *
 */
void CSimulatedClient::requestString(uint32 stringId)
{
	NLMISC::CBitMemStream bms;
	const std::string msgType = "STRING_MANAGER:STRING_RQ";
	if( GenericMsgHeaderMngr.pushNameToStream(msgType,bms) )
	{
		bms.serial( stringId );
		_NetworkConnection.push( bms );
	}
}

/*
 *
 */
void impulseStringResp(NLMISC::CBitMemStream &impulse)
{
	uint32 stringId;
	string	strUtf8;
	impulse.serial(stringId);
	impulse.serial(strUtf8);
	nldebug("Client %s received STRING_RESP %u %s", CSimulatedClient::currentContext()->name().c_str(), stringId, strUtf8.c_str());
}

/*
 *
 */
void impulseServerQuitOk(NLMISC::CBitMemStream &impulse)
{
	CSimulatedClient::currentContext()->disconnect();
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, getNbUsedSlots)
{
	uint nbUsedSlots = 0;
	for (uint slot=1; slot!=256; ++slot)
	{
		if (_NetworkConnection.getPropertyDecoder().isUsed(slot))
			++nbUsedSlots;
	}
	CScriptStack::getInstance()->top() = (sint)nbUsedSlots;
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, getNearestEntity)
{
	CSheetId posFilter = (args.size() == 0) ? CSheetId::Unknown : CSheetId(args[0].c_str());
	CScriptStack::getInstance()->top() = (sint)getNearestEntity(posFilter);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, getVisualProp)
{
	if(args.size() == 0)
		return false;

	CLFECOMMON::TCLEntityId slot = atoi(args[0].c_str());
	CLFECOMMON::TPropIndex propIndex = (args.size() > 1) ? atoi(args[1].c_str()) : (CLFECOMMON::TPropIndex)CScriptStack::getInstance()->top().atoi();
	CScriptStack::getInstance()->top() = (sint)getVisualProp(slot, propIndex);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, a)
{
	if(args.size() == 0)
		return false;

	string arg;
	for (uint i = 0; i < args.size(); i++)
	{
		if (i>0 && !arg.empty())
			arg += ' ';
		if (args[i].find(' ') != std::string::npos)
		{
			arg += "\"" + args[i] + "\"";
		}
		else
		{
			arg += args[i];
		}
	}

	requestCommandA(arg);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, a_value)
{
	if(args.size() == 0)
		return false;

	string arg;
	for (uint i = 0; i < args.size(); i++)
	{
		if (i>0 && !arg.empty())
			arg += ' ';
		if (args[i].find(' ') != std::string::npos)
		{
			arg += "\"" + args[i] + "\"";
		}
		else
		{
			arg += args[i];
		}
	}
	arg += " " + CScriptStack::getInstance()->top();

	requestCommandA(arg);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, target)
{
	CLFECOMMON::TCLEntityId slot = (args.size() > 0) ? atoi(args[0].c_str()) : (CLFECOMMON::TCLEntityId)CScriptStack::getInstance()->top().atoi();
	_NetworkConnection.pushTarget(slot, LHSTATE::NONE);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, continueMission)
{
	if (args.size() == 0)
		return false;

	int intId = atoi(args[0].c_str());
	sendMsgToServer("BOTCHAT:CONTINUE_MISSION", (uint8)intId);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, sleep)
{
	string ts = (args.size() > 0) ? args[0].c_str() : CScriptStack::getInstance()->top();
	TTime until = CTime::getLocalTime() + (atoi(ts.c_str())*1000);
	if (!_ProcessingScript)
		_Script.push_front(string());
	_Script.front() = toString("quiet this.sleepUntil %"NL_I64"d", until);
	if (_ProcessingScript)
		_Script.push_front(string()); // keep our sleepUntil line
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, sleepUntil)
{
	if (args.size() == 0)
		return false;
	TTime until;
	NLMISC::fromString(args[0], until); // atoi() is not 64-bit-compliant
	if (!_ProcessingScript)
		_Script.push_front(toString("this.sleepUntil %"NL_I64"d", until));
	else if (CTime::getLocalTime() < until)
		_Script.push_front(string()); // keep our sleepUntil line
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, repeat)
{
	if (!_ProcessingScript)
		return false;
	if (args.size() == 0)
		return false;
	uint counter = atoi(args[0].c_str());

	for (CScriptList::iterator it=_Script.begin(); it!=_Script.end(); ++it)
	{
		if ((*it) == "this.endrepeat")
		{
			if (counter > 1)
			{
				// Change endrepeat to repeat n-1
				(*it) = toString("this.repeat %u", counter-1);

				// Insert a copy of lines to repeat + endrepeat
				CScriptList::iterator itnext = it;
				++itnext;
				CScriptList::const_iterator torepit=_Script.begin();
				++torepit; // skip front (this.repeat)
				for (; torepit!=it; ++torepit)
				{
					_Script.insert(itnext, *torepit);
				}
				_Script.insert(itnext, "this.endrepeat");
			}
			else
			{
				// Remove endrepeat
				_Script.erase(it);
			}
			break;
		}
	}
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, displayScript)
{
	for (CScriptList::const_iterator it=_Script.begin(); it!=_Script.end(); ++it)
	{
		log.displayNL((string("> ") + *it).c_str());
	}
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CSimulatedClient, disconnect)
{
	requestQuit();
	return true;
}

NLMISC_COMMAND(quiet, "Don't verbose the following piece of script", "{args}")
{
	string line;
	for (vector<string>::const_iterator ita=args.begin(); ita!=args.end(); ++ita)
	{
		if (ita!=args.begin())
			line += " ";
		line += *ita;
	}
	return ICommand::execute(line, log, true);
}

NLMISC_COMMAND(echo, "Display the latest stored value (top of stack)", "[<value]")
{
	if (args.size() > 0)
		log.displayNL(args[0].c_str());
	else
		log.displayNL("Result=%s", CScriptStack::getInstance()->top().c_str());
	return true;
}

NLMISC_COMMAND(getRandomInt, "Store a random int between a and b INCLUDED", "<a> <b>")
{
	if (args.size() < 2)
		return false;

	sint a = atoi(args[0].c_str());
	sint b = atoi(args[1].c_str());
	sint r = min((rand() * (b+1-a) / RAND_MAX) + a, b);
	CScriptStack::getInstance()->top() = r;
	return true;
}

NLMISC_COMMAND(getDataFromTable, "TODO", "<tableName> [<index>]")
{
	if (args.size() == 0)
		return false;

	string tableName = args[0];
	CConfigFile::CVar *table = ClientCfg.ConfigFile.getVarPtr(tableName);
	if (!table)
	{
		log.displayNL("Table %s not found in .cfg", tableName.c_str());
		return false;
	}
	uint index = (args.size() > 1) ? atoi(args[1].c_str()) : CScriptStack::getInstance()->top().atoi();
	string row = table->asString(index);
	CScriptStack::getInstance()->top() = row;
	return true;
}
