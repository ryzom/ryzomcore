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

#include <nel/misc/stop_watch.h>
#include <nel/misc/bit_set.h>

#include <nel/net/login_server.h>
#include <nel/net/login_cookie.h>

#include "fe_types.h"
#include "fe_receive_sub.h"
#include "game_share/action_factory.h"
#include "game_share/action_position.h"
#include "game_share/action_sync.h"
#include "game_share/action_disconnection.h"
#include "game_share/action_association.h"
#include "game_share/action_generic.h"
#include "game_share/action_login.h"
#include "game_share/action_sint64.h"
#include "game_share/action_dummy.h"
#include "game_share/action_block.h"
#include "game_share/action_target_slot.h"
#include "game_share/tick_event_handler.h"
#include "game_share/synchronised_message.h"
#include "history.h"
#include "fe_send_sub.h"
#include "frontend_service.h"
#include "fe_stat.h"

#include "uid_impulsions.h"
#include "id_impulsions.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/system_message.h"
#ifdef ENABLE_MEM_DELTA
#include "../entities_game_service/mem_debug.h"
#else
#define MEM_DELTA_MULTI_LAST2(a, b)
#define MEM_DELTA_MULTI2_MID(a, b)
#define MEM_DELTA_MULTI2(a, b)
#endif

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace CLFECOMMON;


const TUid BaseAutoAllocUserid = 1000000000;
TUid CurrentAutoAllocUserid = BaseAutoAllocUserid;


bool verbosePacketLost = false;

extern void flushMessagesToSend();

//void cbGwTrConnection ( TClientId clientId);
void cbGwTrDisconnection ( TClientId clientId );


extern CVariable<string>	SaveShardRoot;
extern CVariable<bool>		VerboseFEStatsTime;


/*
 * Return the string for the message invalidity reasons
 */
string getBadMessageString( uint32 reasons )
{
	string s;
	if ( reasons & InsufficientSize )
		s += " Insufficient size;";
	if ( reasons & NotSystemLoginCode )
		s += " Not system login code;";
	if ( reasons & BadCookie )
		s += " Bad cookie;";
	if ( reasons & BadSystemCode )
		s += " Bad system code;";
	if ( reasons & HackedSizeInBuffer )
		s += " Hacked size in buffer;";
	if ( reasons & AccessClosed )
		s += " Access closed;";
	if ( reasons & IrrelevantSystemMessage )
		s += " Irrelevant System Message;";
	if ( reasons & MalformedAction )
		s += " Malformed action;";
	if ( reasons & UnknownExceptionType )
		s += " Unknown Exception;";
	if ( reasons & UnknownFormatType )
		s += " Unknown format type;";
	if ( reasons & UnauthorizedCharacterSlot )
		s += " Unauthorized character slot;";
	return s;
}


/*
 * Init
 */
void CFeReceiveSub::init( uint16 firstAcceptableFrontendPort, uint16 lastAcceptableFrontendPort, uint32 dgrammaxlength, CHistory *history, TClientIdCont *clientidcont )
{
	// Preconditions
	nlassert( firstAcceptableFrontendPort != 0 );
	nlassert( lastAcceptableFrontendPort >= firstAcceptableFrontendPort );
	nlassert( dgrammaxlength != 0 );
	nlassert( history && clientidcont );
	nlassert( (_ReceiveTask==NULL) && (_ReceiveThread==NULL) );

	// Start external datagram socket
	nlinfo( "FERECV: Starting external datagram socket" );
	_ReceiveTask = new CFEReceiveTask( firstAcceptableFrontendPort, lastAcceptableFrontendPort, dgrammaxlength );
	_CurrentReadQueue = &_Queue2;
	_ReceiveTask->setWriteQueue( &_Queue1 );
	nlassert( _ReceiveTask != NULL );
	_ReceiveThread = IThread::create( _ReceiveTask );
	nlassert( _ReceiveThread != NULL );
	_ReceiveThread->start();

	// Setup current message placeholder
	_CurrentInMsg = new TReceivedMessage();

	_History = history;
	
	_ClientIdCont = clientidcont;
	_ClientIdCont->resize( MaxNbClients+1 ); // because the client ids start at 1
	TClientIdCont::iterator iclient;
	for ( iclient=_ClientIdCont->begin(); iclient!=_ClientIdCont->end(); ++iclient )
	{
		*iclient = NULL;
	}

	// Setup connection stat log
	_ConnectionStatDisp = new CFileDisplayer( string("data_shard_local/connections.stat"), false, "ConnectionStats" );
	ConnectionStatLog = new CLog( CLog::LOG_NO );
	ConnectionStatLog->addDisplayer( _ConnectionStatDisp );
}


/*
 * Release
 */
void CFeReceiveSub::release()
{
	if ( ConnectionStatLog )
		delete ConnectionStatLog;
	if ( _ConnectionStatDisp )
		delete _ConnectionStatDisp;

	nlassert( _ReceiveTask != NULL );
	nlassert( _ReceiveThread != NULL );

	_ReceiveTask->requireExit();
#ifdef NL_OS_UNIX
	// Send dummy data to a bound udp socket, to wake-up the receive thread
	vector<CInetAddress> addrlist = CInetAddress::localAddresses();
	if ( ! addrlist.empty() )
	{
		CUdpSock tempSock;
		uint8 dummyByte = 0;
		addrlist[0].setPort( _ReceiveTask->DataSock->localAddr().port() );
		tempSock.sendTo( &dummyByte, sizeof(dummyByte), addrlist[0] );
	}
#else
	// Under Windows, closing the socket wakes-up recvfrom().
	_ReceiveTask->DataSock->close();
#endif
	_ReceiveThread->wait();

	delete _ReceiveThread;
	delete _ReceiveTask;
	_ReceiveTask = NULL;
	_ReceiveThread = NULL;

	nlassert( _CurrentInMsg != NULL );
	delete _CurrentInMsg;
	_CurrentInMsg = NULL;
}


// Receive time except CVision::update()
CStopWatch	RecvWatch;




/*
 * Update
 * Deprecated: replaced by modules (see initModules() in frontend_service.cpp)
 */
/*void CFeReceiveSub::update()
{
	swapReadQueues();

	readIncomingData();
}*/

/*
 * Read incoming data from the current read queue
 */
void CFeReceiveSub::readIncomingData()
{
	CFrontEndService::instance()->ReceiveWatch.start();
	RecvWatch.start();

	// Remove the clients who need to
	TClientsToRemove::iterator ictr;
	for ( ictr=_ClientsToRemove.begin(); ictr!=_ClientsToRemove.end(); )
	{
		if ( (*ictr).second == 0 )
		{
			// If the clientid has been removed
			removeClientById( (*ictr).first );
			ictr = _ClientsToRemove.erase( ictr );
		}
		else
		{
			--((*ictr).second); // wait, see addToRemoveList()
			++ictr;
		}
	}

	// Read queue of messages received from clients
	while ( ! _CurrentReadQueue->empty() )
	{
		//nlinfo( "Read queue size = %u", _CurrentReadQueue->size() );
		_CurrentReadQueue->front( _CurrentInMsg->data() );
		_CurrentReadQueue->pop();
		nlassert( ! _CurrentReadQueue->empty() );
		_CurrentReadQueue->front( _CurrentInMsg->VAddrFrom );
		_CurrentReadQueue->pop();
		_CurrentInMsg->vectorToAddress();

#ifndef MEASURE_RECEIVE_TASK
		handleIncomingMsg();
#endif
	}

	// Measure and display rate evenly (at a low frequency)
	static TTime lastdisplay = CTime::getLocalTime();
	if ( CTime::getLocalTime() - lastdisplay > 5000 )
	{
		// Message stats
		if ( VerboseFEStatsTime.get() )
			displayDatagramStats( lastdisplay );

		lastdisplay = CTime::getLocalTime();
		_RcvCounter = 0;
		CFrontEndService::instance()->sendSub()->sendCounter() = 0;

		// Hacking detection: message size
		uint nbrej = _ReceiveTask->nbNewRejectedDatagrams();
		if ( nbrej != 0 )
		{
			nlinfo( "FEHACK: Rejected big messages: %u", nbrej );
		}

		// Hacking detection: bad identification
		if ( ! _UnidentifiedFlyingClients.empty() )
		{
			nldebug( "FEHACK: Received messages from %u unidentified clients:", _UnidentifiedFlyingClients.size() );
			THackingAddrSet::iterator ias;
			for ( ias=_UnidentifiedFlyingClients.begin(); ias!=_UnidentifiedFlyingClients.end(); ++ias )
			{
				nlinfo( "FEHACK: * User %u %s --> %u msg, reasons:%s", (*ias).second.UserId, (*ias).first.asString().c_str(), (*ias).second.InvalidMsgCounter, getBadMessageString( (*ias).second.Reasons ).c_str() );
			}
			_UnidentifiedFlyingClients.clear();
		}
	}

	RecvWatch.stop();
	if ( VerboseFEStatsTime.get() && (RecvWatch.getDuration() > 20) ) // does not include the vision updating
	{
		nlinfo( "FETIME: Time of receiving: AllButVision peak=%u", RecvWatch.getDuration() );
	}
	CFrontEndService::instance()->ReceiveWatch.stop();
}



/*
 * Swap receive queues (to avoid high contention between the receive thread and the reading thread)
 */
void CFeReceiveSub::swapReadQueues()
{
	if ( _CurrentReadQueue == &_Queue1 )
	{
		_CurrentReadQueue = &_Queue2;
		_ReceiveTask->setWriteQueue( &_Queue1 );
		//nlinfo( "** Write1 Read2 ** Read=%p Write=%p", &_Queue2, &_Queue1 );
	}
	else
	{
		_CurrentReadQueue = &_Queue1;
		_ReceiveTask->setWriteQueue( &_Queue2 );
		//nlinfo( "** Read1 Write2 ** Read=%p Write=%p", &_Queue1, &_Queue2 );
	}
}



/*
 * Handles client addition/removal/identification, then calls handleReceivedMsg() (if not removal)
 */
void CFeReceiveSub::handleIncomingMsg()
{
	nlassert( _History && _CurrentInMsg );

#ifndef SIMUL_CLIENTS

	//nldebug( "FERECV: Handling incoming message" );

	// Retrieve client info or add one		
	THostMap::iterator ihm = _ClientMap.find( _CurrentInMsg->AddrFrom );
	if ( ihm == _ClientMap.end() )
	{
		if ( _CurrentInMsg->eventType() == TReceivedMessage::User )
		{
			// Handle message for a new client
			handleReceivedMsg( NULL );
		}
		else
		{
			nlinfo( "FERECV: Not removing already removed client" );
			return;
		}
	}
	else
	{
		// Already existing
		if ( _CurrentInMsg->eventType() == TReceivedMessage::RemoveClient )
		{
			// Remove client
			nlinfo( "FERECV: Disc event for client %u", GETCLIENTA(ihm)->clientId() );
			removeFromRemoveList(GETCLIENTA(ihm)->clientId() ); 
			removeClientById( GETCLIENTA(ihm)->clientId() );

			// Do not call handleReceivedMsg()
			return;
		}
		else
		{
			// Handle message
			H_AUTO(HandleRecvdMsgNotNew);
			handleReceivedMsg( GETCLIENTA(ihm) );
		}
	}

#endif
}



/*
 *
 */
bool		CFeReceiveSub::acceptUserIdConnection( TUid userId )
{
	for (THostMap::iterator it = CFrontEndService::instance()->receiveSub()->clientMap().begin(); it != CFrontEndService::instance()->receiveSub()->clientMap().end(); it++)
	{
		if ( GETCLIENTA(it)->Uid == userId)
		{
			nlwarning( "Can't accept connection of uid %u (already connected: client %hu)", userId, GETCLIENTA(it)->clientId() );
			return false;
		}
	}
	return true;
}



/*
 * Add client
 */
CClientHost *CFeReceiveSub::addClient( const NLNET::CInetAddress& addrfrom, TUid userId, const string &userName, const string &userPriv, const std::string & userExtended, const std::string & languageId, const NLNET::CLoginCookie &cookie, uint32 instanceId, uint8 authorizedCharSlot, bool sendCLConnect )
{
	MEM_DELTA_MULTI_LAST2(Client,AddClient);
	if ( ! acceptUserIdConnection( userId ) )
	{
		cbDisconnectClient( userId , "FS CFeReceiveSub::addClient"); // simulate a request by the login system (NeL Launcher bypass mode)
		return NULL;
	}

	CClientHost *clienthost;
	
	// Create client object and add it into the client map
	TClientId clientid = _ClientIdPool.getNewClientId();
	if ( clientid == InvalidClientId )
	{
		nlwarning( "Front-end is full, new client %s rejected", addrfrom.asString().c_str() );
		return NULL;
	}

	MEM_DELTA_MULTI2_MID(Client,BeforeNewCClientHost); // 0
	/// callback the gateway transport
//	cbGwTrConnection(clientid);

	THostMap::iterator cmPreviousEnd = _ClientMap.end();
	{
		MEM_DELTA_MULTI2(CClientHost,New);
		clienthost = new CClientHost( addrfrom, clientid );
	}
	nlassert( clienthost );
	nlinfo( "Adding client %u (uid %u name %s priv '%s') at %s", clientid, userId, userName.c_str(), userPriv.c_str(), addrfrom.asIPString().c_str() );
	ConnectionStatLog->displayNL( "Adding client %u (uid %u name %s priv '%s') at %s", clientid, userId, userName.c_str(), userPriv.c_str(), addrfrom.asIPString().c_str() );
	//MEM_DELTA_MULTI2_MID(Client,BeforeInsertClient); // = CClientHost

	if ( ! _ClientMap.insert( make_pair( addrfrom, clienthost ) ).second )
	{
		nlwarning( "Problem: Inserted twice the same address in the client map" );

		
	}
	MEM_DELTA_MULTI2_MID(Client,AfterInsertClient); // 24
	CFrontEndService::instance()->PrioSub.VisionProvider.DistanceSpreader.notifyClientAddition( cmPreviousEnd );
	CFrontEndService::instance()->PrioSub.Prioritizer.SortSpreader.notifyClientAddition( cmPreviousEnd );
	/*CFrontEndService::instance()->PrioSub.Prioritizer.PositionSpreader.notifyClientAddition( cmPreviousEnd );
	CFrontEndService::instance()->PrioSub.Prioritizer.OrientationSpreader.notifyClientAddition( cmPreviousEnd );
	CFrontEndService::instance()->PrioSub.Prioritizer.DiscreetSpreader.notifyClientAddition( cmPreviousEnd );*/
	// Set entity ids
	(*_ClientIdCont)[clientid] = clienthost;
	_History->addClient( clientid );
	clienthost->Uid = userId;
	clienthost->UserName = userName;
	clienthost->UserPriv = userPriv;
	clienthost->UserExtended = userExtended;
	clienthost->LanguageId = languageId;
	clienthost->LoginCookie = cookie;
	clienthost->InstanceId = instanceId;
	clienthost->AuthorizedCharSlot = authorizedCharSlot;

	clienthost->initSendCycle( false );
	CFrontEndService::instance()->sendSub()->setSendBufferAddress( clientid, &addrfrom );

	//This must be commented out when the GPMS always activates slot 0
	//CFrontEndService::instance()->PrioSub.Prioritizer.addEntitySeenByClient( clientid, 0 );

#ifdef MEASURE_SENDING
#error should not be active
	static uint8 buf [500];
	nlassert( CFrontEndService::instance()->sendSub()->clientBandwidth()/8 < 500 );
	CFrontEndService::instance()->sendSub()->outBox( clientid ).serialBuffer( buf, CFrontEndService::instance()->sendSub()->clientBandwidth()/8 );
	return clienthost;
#endif

	// Take current time as receive timestamp (set it at login to avoid immediate probe)
	clienthost->setReceiveTimeNow();
	MEM_DELTA_MULTI2_MID(Client,AfterReceiveTimeNow);

	// Send UserId to the EGS
	//nldebug( "%u: Sending user id %u for C%hu: to EGS", CTickEventHandler::getGameCycle(), userId, clientid );
	if ( sendCLConnect )
	{
		if ( ! CFrontEndService::instance()->ShardDown )
		{
			CMessage msgout( "CL_CONNECT" );
			msgout.serial( userId );
			string un = userName;
			msgout.serial( un );
			string up = userPriv;
			msgout.serial( up );
			string ux = userExtended;
			msgout.serial( ux );
			string langId = languageId;
			msgout.serial( langId );
			string addrStr = addrfrom.asIPString();
			msgout.serial( addrStr );
			CLoginCookie	logcook = cookie;
			nlinfo("WEB: sent cookie %s to EGS from player %d", cookie.toString().c_str(), userId);
			msgout.serial( logcook );
			sendMessageViaMirror( "EGS", msgout ); // because the EGS can despawn an entity
		}
		else
		{
			// The client object will be removed when the client exits
			return NULL;
		}
		
		MEM_DELTA_MULTI2_MID(Client,AfterSendCLConnect);
		TClientId mon = CFrontEndService::instance()->MonitoredClient;
		if ( mon != 0 && clientid == mon )
		{
			nlinfo( "CLTMON: Client %u is connecting", mon );
		}

		beepIfAllowed( 660, 150 );
		beepIfAllowed( 880, 150 );
	}

	//CFrontEndService::instance()->validateUserWebEntry(userName, cookie.toString());
	MEM_DELTA_MULTI2_MID(Client,BeforeReturn);

	return clienthost;
}


/*
 * Remove a client by iterator on THostMap (see also byId)
 * // DEPRECATED
 */
/*void CFeReceiveSub::removeClientByAddr( THostMap::iterator iclient )
{
	if ( iclient == _ClientMap.end() )
	{
		// It may have already been removed on purpose
		return;
	}
	nlinfo( "FERECV: Removing client %u at %s", GETCLIENTA(iclient)->clientId(), GETCLIENTA(iclient)->address().asIPString().c_str() );
	doRemoveClient( GETCLIENTA(iclient) );
	//nlassert( GETCLIENTA(iclient)->clientId() < _ClientIdCont->size() );
	//(*_ClientIdCont)[ GETCLIENTA(iclient)->clientId() ] = NULL;
	//delete GETCLIENTA(iclient);

	CFrontEndService::instance()->PrioSub.VisionProvider.DistanceSpreader.notifyClientRemoval( iclient );
	CFrontEndService::instance()->PrioSub.Prioritizer.PositionSpreader.notifyClientRemoval( iclient );
	CFrontEndService::instance()->PrioSub.Prioritizer.OrientationSpreader.notifyClientRemoval( iclient );
	CFrontEndService::instance()->PrioSub.Prioritizer.DiscreetSpreader.notifyClientRemoval( iclient );

	_ClientMap.erase( iclient );
}*/


/*
 *
 */
void CFeReceiveSub::setClientInLimboMode( CClientHost *client )
{
	LimboClients.insert( make_pair( client->Uid, CLimboClient( client ) ) );
	removeClientFromMap( client );
	client->resetClientVision();
	removeClientLinks( client );
	deleteClient( client );
}


/*
 *
 */
CClientHost *CFeReceiveSub::exitFromLimboMode( const CLimboClient& lc )
{
	nldebug( "Restoring user %u from limbo mode", lc.Uid );

	CClientHost *client = addClient( lc.AddrFrom, lc.Uid, lc.UserName, lc.UserPriv, lc.UserExtended, lc.LanguageId, lc.LoginCookie, 0xF, false );
	NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle();
	client->setFirstSentPacket( client->sendNumber()+1, tick );
	client->setSynchronizeState();
	client->QuitId = lc.QuitId;
	return client;
}


/*
 *
 */
void CFeReceiveSub::removeClientFromMap( CClientHost *client )
{
	THostMap::iterator icm = _ClientMap.find( clientIdCont()[client->clientId()]->address() );
	CFrontEndService::instance()->PrioSub.VisionProvider.DistanceSpreader.notifyClientRemoval( icm );
	CFrontEndService::instance()->PrioSub.Prioritizer.SortSpreader.notifyClientRemoval( icm );
	/*CFrontEndService::instance()->PrioSub.Prioritizer.PositionSpreader.notifyClientRemoval( icm );
	CFrontEndService::instance()->PrioSub.Prioritizer.OrientationSpreader.notifyClientRemoval( icm );
	CFrontEndService::instance()->PrioSub.Prioritizer.DiscreetSpreader.notifyClientRemoval( icm );*/

	_ClientMap.erase( client->address() );
}

/*
 * Remove a client by iterator on TClientIdCont (see also byAddr)
 */
void CFeReceiveSub::removeClientById( TClientId clientid, bool crashed )
{
	MEM_DELTA_MULTI2(RemClient,RemoveClientById);

	TClientIdCont::iterator ici = _ClientIdCont->begin() + clientid;
	nlassert( ici >= _ClientIdCont->begin() && ici < _ClientIdCont->end() );

	if ( (*ici) != NULL )
	{
		// Bypass disconnection process if already engaged (can occur if WS requests it several times but not necessary for this case if it can't be inserted twice in the remove list)
		if ( ! GETCLIENTI(ici)->isDisconnected() )
		{
			nlinfo( "FERECV: Removing client %hu at %s", GETCLIENTI(ici)->clientId(), GETCLIENTI(ici)->address().asIPString().c_str() );
			doRemoveClient( GETCLIENTI(ici), crashed );
			removeClientFromMap( GETCLIENTI(ici) );
		}
		else
		{
			// Not expected because cbDisconnectClient prevents from adding twice the same client
			nlwarning( "Bypassing extra-removal of client %hu (uid %u)", GETCLIENTI(ici)->clientId(), GETCLIENTI(ici)->Uid );
		}
	}

	//delete GETCLIENTI(ici);
	//*ici = NULL; // later
}


/*
 * Utility method to remove a client
 */
void CFeReceiveSub::doRemoveClient( CClientHost *client, bool crashed )
{
	MEM_DELTA_MULTI2(RemClient,DoRemoveClient);

	// Tell the other clients that this one has disconnected
	/// DEBUG BEN don't send the disconnection to the others yet
/*
	THostMap::iterator ihm;
	for ( ihm=_ClientMap.begin(); ihm!=_ClientMap.end(); ++ihm )
	{
		if ( GETCLIENTA(ihm) != client && _Vision->see(client->entityId(), GETCLIENTA(ihm)->entityId()))
		{
			// Creating a new action object for each one is required
			CAction *action = CActionFactory::getInstance()->create( ACTION_DISCONNECTION_CODE );
			action->FTEntityId = client->entityId();
			GETCLIENTA(ihm)->ImpulseEncoder.add(action, 1);
		}
	}
*/

	//
	// Remove the client from the EGS
	//

	if ( client->Uid != 0xFFFFFFFF )
	{
		// Handle the case when the id is not set yet (uid instead of id)
		CMessage msgout( "CL_DISCONNECT" );
		//Sid sid = client->sid();
		//msgout.serial( sid );
		msgout.serial( client->Uid );
		msgout.serial( crashed );
		sendMessageViaMirror( "EGS", msgout );
		nlinfo( "Sent CL_DISCONNECT for client %hu (uid %u) to EGS", client->clientId(), client->Uid );
		ConnectionStatLog->displayNL( "Sent CL_DISCONNECT for client %hu (uid %u) to EGS", client->clientId(), client->Uid );

		// unless user wants to reconnect, disconnect him from login service
		//if( ! findInReconnectList( client->Uid ) )
		//nldebug( "disconnecting client %d from login service", client->clientId() );
		CLoginServer::clientDisconnected( client->Uid );
		
	}

	CFrontEndService::instance()->sendSub()->disableSendBuffer( client->clientId() );

	// Here, we don't free the client's id yet (neither remove the entity/client in the history),
	// because CPrioSub::processVision() will need it when removing the corresponding couples.
	// It will call freeIdsOfRemovedClients() after that.
	_RemovedClientEntities.push_back( client );
	client->disconnect();

	TClientId mon = CFrontEndService::instance()->MonitoredClient;
	if ( mon != 0 && client->clientId() == mon )
	{
		nlinfo( "CLTMON: Client %u is disconnecting", mon );
	}


	/* TEMP
	 * End perf measures
	 */
	//CHTimer::display();

	/*
	 * Display prio stats
	 */
#ifdef MEASURE_FRONTEND_TABLES
	nlinfo( "-- Stats for client 1 --" );
	TEventPerSeenEntityCounterFrame::displayAll( "Dist", DistCntClt1 );
	TEventPerSeenEntityCounterFrame::displayAll( "Delta", DeltaCntClt1 );
	TEventPerSeenEntityCounterFrame::displayAll( "Prio", PrioCntClt1 );
	TEventPerSeenEntityCounterFrame::displayAll( "PosSent", PosSentCntClt1 );
#endif
}


/*
 *
 */
void CFeReceiveSub::removeClientLinks( CClientHost *clienthost )
{
	if ( ! clienthost->eId().isUnknownId() )
		EntityToClient.removeEId( clienthost->eId() );

	if ( clienthost->entityIndex().isValid() )
		EntityToClient.removeEntityIndex( clienthost->entityIndex() );
}


/*
 *
 */
void CFeReceiveSub::deleteClient( CClientHost *clienthost )
{
	nldebug( "deleting client %d", clienthost->clientId() );
	//CFrontEndService::instance()->unvalidateUserWebEntry(clienthost->UserName);

	// Callback gateway transport
	cbGwTrDisconnection(clienthost->clientId());

	(*_ClientIdCont)[ clienthost->clientId() ] = NULL;
	_History->removeClient( clienthost->clientId() );
	_ClientIdPool.releaseId( clienthost->clientId() );
	MEM_DELTA_MULTI2(CClientHost,Delete);
	delete clienthost;
}


/*
 * After a client has been removed from the tables, free its identifier
 */
void CFeReceiveSub::freeIdsOfRemovedClients()
{
	vector<CClientHost*>::iterator ic;
	for ( ic=_RemovedClientEntities.begin(); ic!=_RemovedClientEntities.end(); ++ic )
	{
		MEM_DELTA_MULTI2(RemClient,FreeIdsOfRemovedClients);
		CClientHost *clienthost = *ic;
		
		// Display info
		nlinfo( "FE: Freeing client %u (%s)", clienthost->clientId(), clienthost->address().asString().c_str() );
		clienthost->displayClientProperties( false );

		// Reset vision and links in tables
		clienthost->resetClientVision();
		removeClientLinks( clienthost );
		
		// Remove all about the client and delete object
		deleteClient( clienthost );

		beepIfAllowed( 880, 150 );
		beepIfAllowed( 660, 150 );

	}
	_RemovedClientEntities.clear();
}


NLMISC::CBitMemStream Msgin( true ); // global to avoid useless realloc

// This variable helps testing the bug that produced the warning
// "handleReceivedMsg : FERECV: client %d sent messages dated in future"
// after a Stall Mode recovery.
//CVariable<bool> BypassForceSynchronizeState("fs", "BypassForceSynchronizeState", "", false);


/*
 * handleReceivedMsg
 *
 * clienthost is NULL when handle message from a new client, in this case the first CAction *must* be a CActionLogin with a valid cookie
 *
 */
void CFeReceiveSub::handleReceivedMsg( CClientHost *clienthost )
{
	// Preconditions
	//nlassert( _History );
	//nlassert( _CurrentInMsg && (! _CurrentInMsg->data().empty()) );
	//nlinfo( "RECEIVING NOW (%u)", CTickEventHandler::getGameCycle() );

	// Prepare message to read
	uint32 currentsize = _CurrentInMsg->userSize();
	//nlinfo( "currentsize: %u", currentsize );

	memcpy( Msgin.bufferToFill( currentsize ), _CurrentInMsg->userDataR(), currentsize );
	
	try
	{
		uint32	receivednumber;
		bool	systemMode;
		uint32	sendnumberack;
		uint32	ackbitmask;

		// Read received number	and mode bit
		Msgin.serialAndLog1( receivednumber );
		Msgin.serialAndLog1( systemMode );

		// for debug purpose
		/*if (receivednumber < 1000)
			nlinfo("FSRCV-DBG: received packet %d (%s) from client %d %s", receivednumber, systemMode ? "SYSTEM" : "NORMAL", clienthost ? clienthost->clientId() : -1, clienthost ? clienthost->eId().toString().c_str() : "<Unknown>");*/

		// Check incoming client login
		if (clienthost == NULL)
		{
			MEM_DELTA_MULTI_LAST2(ReceivedMsg,AddClient);
			if ( ! _AccessOpen || ! CFrontEndService::instance()->AcceptClients )
			{
				rejectReceivedMessage( AccessClosed );
				return;
			}

			CLoginCookie lc;
			bool validCookie = false;
			string result;

			uint8 code = 255;

			if ( systemMode )
				Msgin.serialAndLog1(code);

			if ( (!systemMode) || (code != SYSTEM_LOGIN_CODE) )
			{
				rejectReceivedMessage( NotSystemLoginCode );
				return;
			}

			Msgin.serial(lc);

			string userName = "NotSet";
			string userPriv;
			string userExtended;
			uint32 instanceId, charSlot;
			//if ( removeFromReconnectList( lc.getUserId() ) )
			//	validCookie = true;
			//else
			result = CLoginServer::isValidCookie (lc, userName, userPriv, userExtended, instanceId, charSlot);
			validCookie = result.empty();
			if ( ! validCookie )
			{
				// Not a valid client
				rejectReceivedMessage( BadCookie, lc.getUserId() );
				return;
			}

			uint32 uid;
			if ( lc.getUserId() == 0xDEADBEEF )
			{
				// The client has neither been authenticated nor provided a user id, but is allowed to connect (dev mode)
				nlinfo ("%s using AutoAllocUserid", _CurrentInMsg->AddrFrom.asString().c_str() );
				string filename = CPath::standardizePath( SaveShardRoot.get() ) + CPath::standardizePath( IService::getInstance()->SaveFilesDirectory.get() ) + "auto_uid_map.bin";

				// Get previously allocated user ids 
				if ( _AutoUidMap.empty() )
				{
					// Load from file
					try
					{
						CIFile file( filename );
						sint v = file.serialVersion( 0 );
						file.serialCont( _AutoUidMap );
					}
					catch (const Exception&)
					{
						nlinfo( "No AutoAllocUserid data found yet" );
					}
					
					// Init CurrentAutoAllocUserid
					TUid maxUid = 0;
					for ( TAutoUidMap::const_iterator itaum=_AutoUidMap.begin(); itaum!=_AutoUidMap.end(); ++itaum )
					{
						if ( (*itaum).second > maxUid )
							maxUid = (*itaum).second;
					}
					CurrentAutoAllocUserid = std::max( BaseAutoAllocUserid, maxUid + 1 );
				}
				
				// Look up the address
				TAutoUidMap::iterator itaum = _AutoUidMap.find( _CurrentInMsg->AddrFrom.internalIPAddress() );
				if ( itaum != _AutoUidMap.end() )
				{
					// This ip address is already known: if not already connected, give the same user id back.
					// (two clients either on the same machine or connected through a gateway won't get the same user id)
					uid = (*itaum).second;
					while ( findClientHostByUid( uid ) != NULL )
					{
						uid = CurrentAutoAllocUserid++;
					}
					(*itaum).second = uid; // store the new user id if the previous one is not available
				}
				else
				{
					// This ip address is new to us, give a new user id and store it
					do { uid = CurrentAutoAllocUserid++; }
					while ( findClientHostByUid( uid ) != NULL );
					_AutoUidMap.insert( std::make_pair( _CurrentInMsg->AddrFrom.internalIPAddress(), uid ) );
						
				}

				// Save the allocated user ids
				try
				{
					COFile file( filename );
					file.serialVersion( 0 );
					file.serialCont( _AutoUidMap );
				}
				catch (const Exception &e)
				{
					nlwarning( "Could not save AutoAllocUserid data: %s", e.what() );
				}
			}
			else
				uid = lc.getUserId();

			string languageId;
			Msgin.serial(languageId);

			// ALWAYS REMOVE CLIENT FROM LIMBO!
			LimboClients.erase(uid);

			clienthost = addClient( _CurrentInMsg->AddrFrom, uid, userName, userPriv, userExtended, languageId, lc, instanceId, (uint8)charSlot );

			// Check if the addition worked
			if ( clienthost != NULL )
			{
				nldebug( "FERECV: received LOGIN(%s) from client %d", lc.toString().c_str(), clienthost->clientId() );

				NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle();
				clienthost->setFirstSentPacket( clienthost->sendNumber()+1, tick );
				clienthost->setSynchronizeState();

				computeStats( clienthost, receivednumber, currentsize, false );
			}
			return;
		}

		// Take current time as receive timestamp
		clienthost->setReceiveTimeNow();

		// Check system messages
		if ( systemMode )
		{
			H_AUTO(CheckSystemMsgs);
			uint8	code;
			Msgin.serialAndLog1(code);

			if (code == SYSTEM_DISCONNECTION_CODE)
			{
				H_AUTO(SystemDisconnectionCode);
				// Make stats and set client's receive number
				// and don't acknowledge packet number because this is a system message
				computeStats( clienthost, receivednumber, currentsize, false );

				nlinfo( "FERECV: Client %u is disconnecting", clienthost->clientId() );
				removeFromRemoveList( clienthost->clientId() ); 
				// false because the client, in this case, didn't crashed
				removeClientById( clienthost->clientId(), false );
				// actions are automatically removed when deleting blocks
				return;
			}

			if (code == SYSTEM_QUIT_CODE)
			{
				H_AUTO(SystemQuitCode);			
				uint32		quitId;

				Msgin.serialAndLog1(quitId);

				nlinfo("Received QUIT %d from client %d (current client QuitId = %d)", quitId, clienthost->clientId(), clienthost->QuitId);

				if (quitId > clienthost->QuitId)
				{
					clienthost->QuitId = quitId;

					// Make stats and set client's receive number
					// and don't acknowledge packet number because this is a system message
					computeStats( clienthost, receivednumber, currentsize, false );

					CFrontEndService::instance()->sendSub()->disableSendBuffer( clienthost->clientId() );

					// Send disconnection to EGS...
					CMessage msgout1( "CL_DISCONNECT" );
					msgout1.serial( clienthost->Uid );
					bool crashed = false;
					msgout1.serial( crashed );
					sendMessageViaMirror( "EGS", msgout1 );

					// ...and then a new connection to EGS
					CMessage msgout2( "CL_CONNECT" );
					msgout2.serial( clienthost->Uid );
					msgout2.serial( clienthost->UserName );
					msgout2.serial( clienthost->UserPriv );
					msgout2.serial( clienthost->UserExtended );
					msgout2.serial( clienthost->LanguageId );
					string addrStr = clienthost->address().asIPString();
					msgout2.serial( addrStr );
					msgout2.serial( const_cast<CInetAddress&>(clienthost->address()) );
					msgout2.serial( clienthost->LoginCookie );
					sendMessageViaMirror( "EGS", msgout2 );

					nlinfo( "Client %hu (uid %u) reset", clienthost->clientId(), clienthost->Uid );

					// Put client aside, then remove it
					setClientInLimboMode( clienthost );
				}

				return;
			}

			// Make stats and set client's receive number
			// and don't acknowledge packet number because this is a system message
			computeStats( clienthost, receivednumber, currentsize, false );

			// Skip double messages or swapped packets
			if ( receivednumber < clienthost->receiveNumber()+1 )
			{
				nldebug("FERECV: Received an old packet from client %u, expected %d, received %d", clienthost->clientId(), clienthost->receiveNumber()+1, receivednumber);
				computeStats( clienthost, receivednumber, currentsize, false );
				return;
			}
			else if ( (receivednumber > clienthost->receiveNumber()+1) && (!systemMode) )
			{
				LOG_PACKET_LOST("FERECV: Lost packet(s) from client %u, expected %d, received %d", clienthost->clientId(), clienthost->receiveNumber()+1, receivednumber);
			}

			{
//			if (BypassForceSynchronizeState.get() == 1)
//			{
//				if ((code == SYSTEM_ACK_SYNC_CODE) && (clienthost->ConnectionState == CClientHost::ForceSynchronize))
//				{
//					clienthost->setSynchronizeState();
//					nldebug("Accepting Ack_Sync during Stall Recovery");
//				}
//			}

			H_AUTO(SwitchConnectionState);	
			switch (clienthost->ConnectionState)
			{
			case CClientHost::Synchronize:
			//case CClientHost::ForceSynchronize: // in ForceSynchronize, don't let Synchronize be replaced by Connected (ignore this ack_sync, subsequent synchronization will bring a new ack_sync)
				// in synchronize, only accept ACK_SYNC
				if (code == SYSTEM_ACK_SYNC_CODE)
				{
					H_AUTO(SystemAckSyncCode);
					//nldebug("FERECV: received ACK_SYNC from client %d", clienthost->clientId());
					clienthost->setConnectedState();

					// decode long ack
					uint32	frontack;
					uint32	backack;
					CBitSet	longackbitfield;
					uint32	syncCode;

					Msgin.serialAndLog1(frontack);
					Msgin.serialAndLog1(backack);
					// use this bitfield as a rolling buffer, as long as frontack-backack < NumBitsInLongAck
					Msgin.serial(longackbitfield);

					Msgin.serialAndLog1(syncCode);

					// check the received sync is correct
					if (syncCode == clienthost->LastSentSync)
					{
						// process ack
						uint	ack = std::max(backack, clienthost->LastReceivedAck+1);
						//nlinfo("BEN-DBG: ACK_SYNC, start reading longackbitfield from client %d", clienthost->clientId());
						for (; ack<=frontack; ++ack)
						{
							bool	ackvalue = longackbitfield[ack&(NumBitsInLongAck-1)];
							//nlinfo("BEN-DBG: ACK_SYNC, ack %d: %s", ack, (ackvalue ? "true" : "false"));
							_History->ack(clienthost->clientId(), ack, ackvalue);
							if (ackvalue)
							{
								clienthost->ImpulseEncoder.ack(ack);
							}
						}
						//nlinfo("BEN-DBG: ACK_SYNC, end reading longackbitfield from client %d", clienthost->clientId());

						clienthost->LastReceivedAck = frontack;

						++clienthost->LastSentSync;
					}
					else
					{
						rejectReceivedMessage( IrrelevantSystemMessage );
					}
				}
				else if (code != SYSTEM_LOGIN_CODE && code != SYSTEM_ACK_PROBE_CODE)
				{
					nlwarning("FERECV: Received from client %d (state Synchronize) system message %s (%d), expected ACK_SYNC (%d)", clienthost->clientId(), CLFECOMMON::SystemMessagesNames[code&31], code, SYSTEM_ACK_SYNC_CODE);
				}
				break;

			case CClientHost::Probe:
				// in probe, only accept ACK_PROBE
				if (code == SYSTEM_ACK_PROBE_CODE)
				{
					H_AUTO(SystemAckProbeCode);
					nldebug("FERECV: received ACK_PROBE from client %d", clienthost->clientId());
					uint32			numProbes, probe;
					Msgin.serialAndLog1(numProbes);
					// get next received probe, and compare to the previous one
					// if one step increment consecutive counter, else reset consecutive counter to 1
					for (; numProbes>0; --numProbes)
					{
						Msgin.serialAndLog1(probe);
						if (probe == clienthost->LastReceivedProbe+1)
						{
							nldebug("FERECV: received probe %d, OK", probe);
							clienthost->NumConsecutiveProbes++;
						}
						else
						{
							nldebug("FERECV: received probe %d, expected %d", probe, clienthost->LastReceivedProbe+1);
							clienthost->NumConsecutiveProbes = 1;
						}
						clienthost->LastReceivedProbe = probe;
					}

					if (clienthost->NumConsecutiveProbes >= 5)
					{
						nldebug("FERECV: received %d consecutive probes, resynchronize...", clienthost->NumConsecutiveProbes);
						clienthost->setSynchronizeState();
						// Setup SYNC for the client
						NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle();
						clienthost->setFirstSentPacket( clienthost->sendNumber()+1, tick );
						// clean up history, packets numbers aso
					}
				}
				else
				{
					nlinfo("FERECV: Received from client %d (state Probe) system message %s (%d), expected ACK_PROBE (%d)", clienthost->clientId(), CLFECOMMON::SystemMessagesNames[code&31], code, SYSTEM_ACK_PROBE_CODE);
				}
				break;

			default:

//				if ((code == SYSTEM_ACK_SYNC_CODE) && (clienthost->ConnectionState == CClientHost::ForceSynchronize))
//					nldebug("Ignoring Ack_Sync during Stall Recovery");
				//rejectReceivedMessage( BadSystemCode, uid ); // occurs in normal behaviour
				break;
			}
			}
			return;
		}

		// Skip double messages or swapped packets
		if ( receivednumber < clienthost->receiveNumber()+1 )
		{
			nldebug("FERECV: Received an old packet from client %u, expected %d, received %d", clienthost->clientId(), clienthost->receiveNumber()+1, receivednumber);
			computeStats( clienthost, receivednumber, currentsize, false );
			return;
		}
		else if ( (receivednumber > clienthost->receiveNumber()+1) && (!systemMode) )
		{
			LOG_PACKET_LOST("FERECV: Lost packet(s) from client %u, expected %d, received %d", clienthost->clientId(), clienthost->receiveNumber()+1, receivednumber);
		}


		if ( clienthost->ConnectionState != CClientHost::Connected )
		{
			// discard incoming normal message if client is not in connected state
			//nlwarning("FERECV: Received from client %d normal message whereas state is %d", clienthost->clientId(), clienthost->ConnectionState);
			// Make stats and set client's receive number
			// only acknowledge packet number for normal messages in connected state
			computeStats( clienthost, receivednumber, currentsize, false );
			return;
		}

		// Read acknowledgement numbers and submit them to the history
		Msgin.serialAndLog1( sendnumberack );
		Msgin.serialAndLog1( ackbitmask );

		/*if (receivednumber < 1000)
			nlinfo("FSRCV-DBG: received normal packet %d (%s) from client %d %s, Ack=%d AckBitMask=%08X", receivednumber, systemMode ? "SYSTEM" : "NORMAL", clienthost ? clienthost->clientId() : -1, clienthost ? clienthost->eId().toString().c_str() : "<Unknown>", sendnumberack, ackbitmask);*/

		list<CActionBlock>	blocks;
		{
			H_AUTO(Unpack);
			while ( (uint32)Msgin.length()*8 - (uint32)Msgin.getPosInBit() >= (uint32)CActionBlock::getHeaderSizeInBits() )
			{
				blocks.push_back(CActionBlock());
				blocks.back().serial(Msgin);
				if ( ! blocks.back().Success )
				{
					computeStats( clienthost, receivednumber, currentsize, false );
					rejectReceivedMessage( MalformedAction, clienthost->Uid );
					return;
				}
			}
		}

		/*if ( blocks.size() == 0 )
		{
			//nlwarning( "Client %s sent 0 action (hacker?)", clienthost ? clienthost->address().asIPString().c_str() : "<null>" );
			// Make stats and set client's receive number
			// only acknowledge packet number for good normal messages
			//computeStats( clienthost, receivednumber, currentsize, false );
			//return;
		}*/
		/* //OBSOLETE: now it is handled by a system msg SYSTEM_DISCONNECTION_CODE
		// if client sends disconnection, remove it, and leave
		else if (!(*blocks.begin()).Actions.empty() && (*blocks.begin()).Actions[0]->Code==ACTION_DISCONNECTION_CODE)
		{
			// actions are automatically removed when deleting blocks
			nlinfo( "FERECV: Client %u is disconnecting", clienthost->clientId() );
			removeFromRemoveList( clienthost->clientId() ); 
			removeClientById( clienthost->clientId() );
			// Make stats and set client's receive number
			// only acknowledge packet number for good normal messages
			computeStats( clienthost, receivednumber, currentsize, false );
			return;
		}*/
		{
			H_AUTO(ReadAcks);
			// procede with acks
			if (sendnumberack != 0) //clienthost->LastReceivedAck != 0xFFFFFFFF)
			{
				const uint	AckBits = sizeof(ackbitmask)*8;
				uint		numAcks;
				sint		i = 0;

				if (clienthost->LastReceivedAck != 0xFFFFFFFF)
				{
					numAcks = sendnumberack - clienthost->LastReceivedAck;

					if (numAcks > AckBits+1)
					{
						// we lost too many acks !!
						// must resync completely !!
						// TODO: ignore all incoming packets from this host until we complete resync
						nlwarning("FE:BEN: Too many ACK from client %d lost [Last=%d,Current=%d], setting to probe state", clienthost->clientId(), clienthost->LastReceivedAck, sendnumberack);
						clienthost->setProbeState();
						// Make stats and set client's receive number
						// only acknowledge packet number for good normal messages. Here a probe issue, don't ack message
						computeStats( clienthost, receivednumber, currentsize, false );
						return;
					}

					i = -(sint)(numAcks)+1;
					if (i < -(sint)(AckBits))
						i = -(sint)(AckBits);
				}

				// scan ack bits
				bool	hasNegAck = false;
				for (; i<=0; ++i)
				{
					if (i == 0 || (ackbitmask&(1<<(-1-i))) != 0)
					{
						// positive ack

						// acknowledges sent impulses at for that packet
						clienthost->ImpulseEncoder.ack(sendnumberack+i);
						_History->ack(clienthost->clientId(), sendnumberack+i, true);
					}
					else
					{
						// negative ack
						//nlinfo("FE:BEN: neg ACK from client %d (packet %d)", clienthost->clientId(), sendnumberack+i);
						_History->ack(clienthost->clientId(), sendnumberack+i, false);
						hasNegAck = true;
					}
				}

				//if (hasNegAck)
				//	nlinfo("FE:BEN: Client=%d Packet=%d Ack=%d AckBits=%d LastAck=%d", clienthost->clientId(), receivednumber, sendnumberack, ackbitmask, clienthost->LastReceivedAck);
			}
			/*
			else if (sendnumberack != 0)
			{
				clienthost->ImpulseEncoder.ack(sendnumberack);
				_History->ack(clienthost->clientId(), sendnumberack, true);
			}
			*/
			/*
			if ( sendnumberack != 0 )
			{
				_History->ack( clienthost->clientId(), sendnumberack, ackbitmask, sizeof(ackbitmask)*8 );
			}
			*/

			clienthost->LastReceivedAck = sendnumberack;
		}

		{
			H_AUTO(Switch);
			uint	j;

			/*static char	debugdisplay[256000];
			static char	debugcat[128000];
			debugdisplay[0] = '\0';*/

			uint	numActions = 0;

			// Make stats and set client's receive number
			// only acknowledge packet number for good normal messages
			// now we can acknowledge the message safely
			computeStats( clienthost, receivednumber, currentsize, true );

			// Iterate on the action blocks (each block corresponds to one game cycle)
			for (list<CActionBlock>::iterator it=blocks.begin(); it!=blocks.end(); ++it)
			{
				CActionBlock	&block = (*it);
				if (block.Cycle <= clienthost->LastReceivedGameCycle)
				{
					//sprintf(debugcat, "old-block[%d][%d actions] ", block.Cycle, block.Actions.size());
					//strcat(debugdisplay, debugcat);
					continue;
				}

				clienthost->LastReceivedGameCycle = block.Cycle;
				//sprintf(debugcat, "new-block[%d][%d actions] ", block.Cycle, block.Actions.size());
				//strcat(debugdisplay, debugcat);

				// Iterator on the actions in a block
				for (j=0; j<block.Actions.size(); ++j)
				{
					CAction	*action = block.Actions[j];
					++numActions;

					switch ( action->Code )
					{
					case ACTION_GENERIC_CODE: // forward an impulsion
						{
							CActionGeneric *ag = static_cast<CActionGeneric*>( action );
							//vector<uint8> &vect = ag->get().bufferAsVector ();
							uint8 msgtype;
							if ( clienthost->eId().isUnknownId() )
							{
								//nlinfo( "FERECV: Forwarding IMPULSION_UID (%d bytes) from client %u to IOS", ag->get().length(), clienthost->clientId() );
								routeImpulsionUidFromClient( ag->get(), clienthost->Uid, block.Cycle );
							}
							else
							{
								//msgout.setType( "IMPULSION_ID" );
								msgtype = 1;
								//nlinfo( "FERECV: Forwarding IMPULSION_ID (%d bytes) from client %u to IOS", ag->get().length(), clienthost->clientId() );
								routeImpulsionIdFromClient( ag->get(), clienthost->eId(), block.Cycle );
							}
						}
						break;

					case ACTION_GENERIC_MULTI_PART_CODE:
						{
							CActionGenericMultiPart *agmp = static_cast<CActionGenericMultiPart*>( action );

							// manage a generic action (big one that comes by blocks)
							vector<uint8> &v = agmp->PartCont;

							// add it
							if (clienthost->GenericMultiPartTemp.size() <= agmp->Number)
								clienthost->GenericMultiPartTemp.resize (agmp->Number+1);

							clienthost->GenericMultiPartTemp[agmp->Number].set(agmp, clienthost);
						}
						break;

					case ACTION_TARGET_SLOT_CODE:
						{
							// Convert clientid,slot to id
							CActionTargetSlot *ats = static_cast<CActionTargetSlot*>(action);
							TDataSetRow targetindex;
							switch ( ats->Slot )
							{
							case INVALID_SLOT :	// Untargetting
								targetindex = TDataSetRow::createFromRawIndex( INVALID_DATASET_INDEX );
								break;
							case 0:				// Self targetting
								targetindex = clienthost->entityIndex();
								break;
							default:			// Targetting an entity in the vision
								targetindex = CFrontEndService::instance()->PrioSub.VisionArray.getEntityIndex( clienthost->clientId(), ats->Slot );
							}
							routeImpulsionIdSlotFromClient( ats->TargetOrPickup, targetindex, clienthost->entityIndex(), block.Cycle );
						}
						break;

					// TODO: jarter
					case ACTION_DUMMY_CODE:
						{
							CActionDummy	*dummy = static_cast<CActionDummy*>(action);
							if (dummy->Dummy1 - clienthost->LastDummy != 1)
							{
								nlwarning("******* PB: LastDummy=%d dummy->Dummy1=%d", clienthost->LastDummy, dummy->Dummy1);
							}
							clienthost->LastDummy = dummy->Dummy1;
						}
						break;

					default:
						nlwarning( "Received unknown action code %d from client %d", action->Code, clienthost->clientId() );
					}
				}
			}

			if (clienthost->LastReceivedGameCycle > CTickEventHandler::getGameCycle())
			{
				nlwarning("FERECV: client %d sent messages dated in future (tick=%d whereas server is at %d)", clienthost->clientId(), clienthost->LastReceivedGameCycle, CTickEventHandler::getGameCycle());
			}
		}
	}
	catch(const EStreamOverflow& )
	{
		TUid userId = clienthost ? clienthost->Uid : 0;
		rejectReceivedMessage( InsufficientSize, userId );
	}
	catch(const EInvalidDataStream& )
	{
		TUid userId = clienthost ? clienthost->Uid : 0;
		rejectReceivedMessage( HackedSizeInBuffer, userId );
	}
	catch (const Exception& )
	{
		TUid userId = clienthost ? clienthost->Uid : 0;
		rejectReceivedMessage( UnknownExceptionType, userId );
	}
	catch ( ... )
	{
		TUid userId = clienthost ? clienthost->Uid : 0;
		rejectReceivedMessage( UnknownFormatType, userId );
	}

		//nlinfo("FERECV: received packet %d from client %d (%d actions decoded): %s", receivednumber, clienthost->clientId(), numActions, debugcat);

	// warning: actions in CActionBlock are automatically removed when deleting block 
}


/*
 * Do not accept the current message (hacking detection)
 */
void CFeReceiveSub::rejectReceivedMessage( TBadMessageFormatType bmft, TUid userId )
{
	//nlwarning("FEHACK: Incoming client failed identification: systemMode=%s, code=%d, isValidCookie=%s", (systemMode ? "true" : "false"), code, result.c_str());
	pair<THackingAddrSet::iterator,bool> res = _UnidentifiedFlyingClients.insert( make_pair( _CurrentInMsg->AddrFrom, THackingDesc() ) );
	THackingDesc& desc = (*(res.first)).second;
	if ( ! res.second )
	{
		++desc.InvalidMsgCounter; // increment the counter
	}

	// Set the userId and the reasons flag
	desc.UserId = userId;
	desc.Reasons |= bmft;
}


/*
 * Compute stats about received datagrams
 */
void CFeReceiveSub::computeStats( CClientHost *clienthost, uint32 currentcounter, uint32 currentsize, bool updateAcknowledge )
{
	// Host stats
	clienthost->computeHostStats( *_CurrentInMsg, currentcounter, updateAcknowledge );

	// Global stats
	++_RcvCounter;
	_RcvBytes += currentsize;
}



/*
 * Display datagram loss statistics
 */
void CFeReceiveSub::displayDatagramStats( TTime lastdisplay )
{
	uint32 pcsum=0, pcnb=0;
	THostMap::const_iterator ihm;
	for ( ihm=_ClientMap.begin(); ihm!=_ClientMap.end(); ++ihm )
	{
		// Avoid clients that have not sent anything yet
		if ( GETCLIENTA(ihm)->receiveNumber() != 0xFFFFFFFF )
		{
			// Calc percent
			uint32 diff = GETCLIENTA(ihm)->receiveNumber()-GETCLIENTA(ihm)->firstReceiveNumber()+1;
			nlassertex( diff!=0, ("receiveNumber: %u; firstReceiveNumber: %u", GETCLIENTA(ihm)->receiveNumber(), GETCLIENTA(ihm)->firstReceiveNumber()) );
			pcsum += GETCLIENTA(ihm)->datagramLost() * 100 / diff;
			++pcnb;

			// Reset
			GETCLIENTA(ihm)->resetDatagramLost();
			GETCLIENTA(ihm)->firstReceiveNumber() = GETCLIENTA(ihm)->receiveNumber();
		}
	}
	CFrontEndService *fe = CFrontEndService::instance();
	float loadavg = (pcnb != 0.0f) ? ((float)pcsum/(float)pcnb) : 0.0f;

	/*float reenableRatio = 0.0f;
	if ( fe->PrioSub.PropDispatcher.NbSetPrio != 0 )
	{
		reenableRatio = (float)fe->PrioSub.PropDispatcher.NbReenable / (float)fe->PrioSub.PropDispatcher.NbSetPrio * 100.0f;
		fe->PrioSub.PropDispatcher.NbSetPrio = 0;
		fe->PrioSub.PropDispatcher.NbReenable = 0;
	}*/

	uint32	sentMsg = fe->sendSub()->sendCounter();
	uint32  backendrecvtime = fe->BackEndRecvWatch1.getPartialAverage() + fe->BackEndRecvWatch2.getPartialAverage() + fe->BackEndRecvWatch3.getPartialAverage();
	nlinfo( "FESTATS: Ticks: %u - In msg: %u (%u Bps) - Out msg: %u - Loss average: %.2f - Clients: %u - RBackEnd=%u (PVision=%u) User=%u Recv=%u Fill=%u - Cycle=%u ms",
			CTickEventHandler::getGameCycle(),
			_RcvCounter,
			(_RcvBytes-_PrevRcvBytes)*1000/sint32(CTime::getLocalTime()-lastdisplay),
			sentMsg,
			loadavg,
			_ClientMap.size(),
			backendrecvtime,
			fe->ProcessVisionWatch.getPartialAverage(),
			fe->UserDurationPAverage,
			fe->ReceiveWatch.getPartialAverage(),
			fe->SendWatch.getPartialAverage(),
			fe->CycleWatch.getPartialAverage() );

	_PrevRcvBytes = _RcvBytes;
}


/*
 * Remove from the list of clients
 */
void		CFeReceiveSub::removeFromRemoveList( TClientId clientid )
{
	TClientsToRemove::iterator icr;
	for ( icr=_ClientsToRemove.begin(); icr!=_ClientsToRemove.end(); ++icr )
	{
		if ( (*icr).first == clientid )
		{
			_ClientsToRemove.erase( icr );
			return;
		}
	}
}

uint32	 CFeReceiveSub::getNbClient()
{
	return (uint32)_ClientMap.size();
}


/*
 * Find a client host by Uid (slow)
 */
CClientHost *CFeReceiveSub::findClientHostByUid( TUid uid, bool exitFromLimbo )
{
	// First, find in limbo clients
	map<TUid,CLimboClient>::iterator ilc = LimboClients.find( uid );
	if ( exitFromLimbo && (ilc != LimboClients.end()) )
	{
		CLimboClient& lc = (*ilc).second;
		CClientHost *clienthost = exitFromLimboMode( lc );
		//clienthost->QuitId = lc.
		LimboClients.erase( ilc );
		return clienthost;
	}
	else
	{
		// If not found, find in online clients
		THostMap::iterator ihm;
		for ( ihm=clientMap().begin(); ihm!=clientMap().end(); ++ihm )
		{
			if ( GETCLIENTA(ihm)->Uid == uid )
			{
				return GETCLIENTA(ihm);
			}
		}
		return NULL;
	}
}


/*
 * |<entityId>
 * If the first argument is an entity id, it is used to look-up the player (the provided dynamic id
 * is ignored, you can provide 0).
 * entityId not implemented, because using the 'clientByUserId' command is easier.
 */
NLMISC_COMMAND( displayPlayerInfo, "Display the properties of a player", "[<clientid> [<sortByDistance(1/0)=1> [<displayAllPropertiesSent(1/0)=0>]]]" )
{
	// Get the values
	if (args.size() > 0)
	{
		// Try to interpret the first argument as an entity id (beginning by '(')
		/*CEntityId entityId;
		if ( (!args[0].empty()) && (args[0][0] == '(') )
		{
			entityId.fromString( args[0].c_str() );
		}*/
		// If the first argument is not a valid entity id, interpret it as a client id
		TClientId clientid;
		/*if ( entityId.isUnknownId() )
		{*/
			 NLMISC::fromString(args[0], clientid);
		/*}
		else
		{
			// Use the dynamic id of this frontend
			entityId.setDynamicId( IService::getInstance()->getServiceId() );
		}*/

		bool sbd = true;
		bool allProps = false;
		if ( args.size() > 1 )
		{
			sbd = (args[1] == string("1"));
			if ( args.size() > 2 )
				allProps = (args[2] == string("1"));
		}
		CClientHost *clienthost;
		if ( (clientid <= MaxNbClients) && ((clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid]) != NULL) )
		{
			clienthost->displayClientProperties( true, allProps, sbd, &log );
		}
		else
		{
			log.displayNL( "There is no such a client id" );
		}
	}
	else
	{
		log.displayNL( "List of all clients and their properties:" );
		THostMap::iterator ihm;
		for ( ihm=CFrontEndService::instance()->receiveSub()->clientMap().begin();
			 ihm!=CFrontEndService::instance()->receiveSub()->clientMap().end(); ++ihm )
		{
			GETCLIENTA(ihm)->displayClientProperties( false, false, false, &log );
		}
	}

	return true;
}


NLMISC_COMMAND( slotToEntityId, "Get the entityid of a slot of a client (and possibly display info)", "<clientid> <slot> <0/1>" )
{
	if ( args.size() < 2 )
		return false;

	TClientId clientid;
	NLMISC::fromString(args[0], clientid);
	if ( clientid <= MaxNbClients )
	{
		TCLEntityId slot;
		NLMISC::fromString(args[1], slot);
		TEntityIndex entityIndex = CFrontEndService::instance()->PrioSub.VisionArray.getEntityIndex( clientid, slot );
		if ( entityIndex.getIndex() < (uint32)TheDataset.maxNbRows() )
		{
			CEntity* entity = TheEntityContainer->getEntity( entityIndex );
			if ( entity )
			{
				CMirrorPropValueRO<TYPE_SHEET> propSheet( TheDataset, entityIndex, DSPropertySHEET );
				log.displayNL( "Slot %hu of client C%hu is %s, sheet %u", (uint16)slot, clientid, TheDataset.getEntityId( entityIndex).toString().c_str(), (uint32)propSheet() );
				if ( args.size() == 3 )
				{
					if ( args[2] == "1" )
					{
						CFrontEndService::instance()->PrioSub.VisionProvider.displayEntityInfo( *entity, entityIndex, &log );
					}
				}
			}
			else
				log.displayNL( "Unable to find entity" );
		}
		else
		{
			log.displayNL( "Found invalid entity index" );
		}
	}
	return true;
}


NLMISC_COMMAND( displayPlayers, "display the client ids used, and short info about them", "" )
{
	THostMap::iterator ihm;
	for ( ihm=CFrontEndService::instance()->receiveSub()->clientMap().begin();
	     ihm!=CFrontEndService::instance()->receiveSub()->clientMap().end(); ++ihm )
	{
		GETCLIENTA(ihm)->displayShortProps(&log);
	}
	return true;
}


NLMISC_COMMAND( displayShortStats, "Display some stats", "" )
{
	CFrontEndService *fe = CFrontEndService::instance();
	log.displayNL( "Clients: %u", fe->receiveSub()->clientMap().size() );
	return true;
}


NLMISC_DYNVARIABLE( uint32, NbPlayers, "Number of connected players" )
{
	// We can only read the value
	if ( get )
		*pointer = (uint32)CFrontEndService::instance()->receiveSub()->clientMap().size();
}


NLMISC_COMMAND( displayMonitoredClientInfo, "Display info about the monitored client", "" )
{
	CFrontEndService *fe = CFrontEndService::instance();
	CClientHost *client = fe->receiveSub()->clientIdCont()[fe->MonitoredClient];
	if ( client )
	{
		client->displayClientProperties( true, false, true, &log );
	}

	return true;
}


NLMISC_COMMAND( openAccess, "Open/close the access for new clients", "<1/0>" )
{
	bool b, get;
	if ( args.size() == 0 )
	{
		b = CFrontEndService::instance()->receiveSub()->accessOpen();
		get = true;
	}
	else
	{
		b = (args[0] == "1");
		CFrontEndService::instance()->receiveSub()->openAccess( b );
		get = false;
	}
	log.displayNL( "Access is %s %s", get?"currently":"now", b?"open":"closed" );
	return true;
}


NLMISC_COMMAND( resetAutoAllocUserid, "Reset the auto-incrementable user id (use with caution)", "" )
{
	CurrentAutoAllocUserid = BaseAutoAllocUserid;
	return true;
}


NLMISC_COMMAND( clientByUserId, "Get a client id by user id", "<userId>" )
{
	TUid userId;
	if ( args.empty() )
		return false;
	else
		NLMISC::fromString(args[0], userId);

	CClientHost *clienthost = CFrontEndService::instance()->receiveSub()->findClientHostByUid( userId );
	if ( clienthost )
	{
		log.displayNL( "Found C%hu E%u %s %s", clienthost->clientId(), clienthost->entityIndex().getIndex(), clienthost->eId().toString().c_str(), clienthost->address().asIPString().c_str() );
	}
	else
	{
		log.displayNL( "Client not found" );
	}
	return true;
}


NLMISC_COMMAND( dumpClientInfo, "Dump all client info into a file", "<fileName> <clientid> [<sortByDistance(1/0)=1>]" )
{
	string filename;
	TClientId clientid;
	bool sbd = true;
	if ( args.size() < 2 )
		return false;
	else
	{
		filename = args[0];
		NLMISC::fromString(args[1], clientid);
		if ( args.size() > 2 )
			sbd = (args[2]==string("1"));
	}

	CClientHost *clienthost;
	if ( (clientid <= MaxNbClients) && ((clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid]) != NULL) )
	{
		CFileDisplayer *fd = new CFileDisplayer( filename, false, filename.c_str() );
		CLog flog( CLog::LOG_NO );
		flog.addDisplayer( fd );
		clienthost->displayClientProperties( true, true, sbd, &flog );
		delete fd;
		log.displayNL( "File %s saved with info of C%hu", filename.c_str(), clientid );
		// TODO: send the same command to the client
	}
	else
	{
		log.displayNL( "There is no such a client id" );
	}
	return true;
}


NLMISC_COMMAND( retrieveEntityNames, "Request the IOS to send the names of all online entities, for next calls to other commands", "" )
{
	CMessage msgout( "RETR_ENTITY_NAMES" );
	CUnifiedNetwork::getInstance()->send( "IOS", msgout );
	return true;
}


NLMISC_COMMAND( clearEntityNames, "Clear the entity names after retrieveEntityNames", "" )
{
	CFrontEndService::instance()->EntityNames.clear();
	return true;
}


NLMISC_COMMAND( verbosePacketLost, "Turn on/off or check the state of verbose logging of packet lost", "" )
{
	if ( args.size() == 1 )
	{
		if ( args[0] == string("on") || args[0] == string("1") )
			verbosePacketLost=true;
		else if ( args[0] == string("off") || args[0] == string("0") )
			verbosePacketLost=false;
	}

	log.displayNL( "verbosePacketLost is %s", verbosePacketLost ? "on" : "off" );
	return true;
}
