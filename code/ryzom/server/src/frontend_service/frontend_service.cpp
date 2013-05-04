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
#include <nel/misc/sheet_id.h>
#include <nel/misc/path.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/thread.h>

#include <nel/net/unified_network.h>
#include <nel/net/login_server.h>

#include "game_share/action_factory.h"
#include "game_share/action_position.h"
#include "game_share/action_sync.h"
#include "game_share/action_disconnection.h"
#include "game_share/action_association.h"
#include "game_share/action_association.h"
#include "game_share/action_login.h"
#include "game_share/action_generic.h"
#include "game_share/action_generic_multi_part.h"
#include "game_share/action_sint64.h"
#include "game_share/action_dummy.h"
#include "game_share/action_target_slot.h"
#include "game_share/simlag.h"
#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_version.h"
#include "game_share/entity_types.h"
#include "game_share/system_message.h"
#include "game_share/generic_xml_msg_mngr.h"

//#include "gpm_service/gpm_service.h"

#include "frontend_service.h"
#include "module_manager.h"
#include "client_host.h"
#include "fe_stat.h"

#include "id_impulsions.h"
#include "uid_impulsions.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace CLFECOMMON;

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}

//#define FE_MULTITHREADED

// callback for module security system
void cbEntityIdChanged(CClientHost *clienthost);

CGenericXmlMsgHeaderManager	GenericXmlMsgHeaderMngr;
NLNET::TServiceId SelfServiceId;

bool UseTickService = false;
uint32 Ticked = 0;

// Synchronization of flushing (sending) thread
volatile bool FlushInProgress = false;

// Allow beeping
bool AllowBeep = false;

// Client limit, to reduce memory usage
uint MaxNbClients = 1000;

// Size of UDP/IP/Ethernet headers: 8+20+14 = 42
const uint32 UDP_IP_ETH_HEADER_SIZE = 42;

CVariable<bool>		DontNeedBackend("FS", "DontNeedBackend", "Debug feature to allow client connection without backend (1=always allow connection , 0=backend must be up)", false, 0, true);

CVariable<bool>		UseSendThread("FS", "UseSendThread", "Use thread for sending", false, 0, true);

CVariable<bool>		UseWebPatchServer("FS", "UseWebPatchServer", "Use Web Server for patching", true, 0, true);
CVariable<bool>		AcceptClientsAtStartup("FS", "AcceptClientsAtStartup", "Set Frontend Accept mode (1=accept clients, 0=patching mode)", false, 0, true);
CVariable<string>	PatchingURLFooter("FS", "PatchingURLFooter", "Patching server listenning port + directory (default is ':43435/patch')", ":43435/patch", 0, true);
CVariable<string>	StartWebServerSysCommand("FS", "StartWebServerSysCommand", "System command to execute when to start Web Patch Server", "", 0, true);
CVariable<string>	StopWebServerSysCommand("FS", "StopWebServerSysCommand", "System command to execute when to stop Web Patch Server", "", 0, true);
CVariable<bool>		PublishFSHostAsIP("FS", "PublishFSHostAsIP", "Publish FSHost to the WS as IP:port instead of hostname:post", false, 0, true);
CVariable<uint32>	DelayBeforeUPDAlert("FS", "DelayBeforeUPDAlert", "Delay (in s) before raising an alert when the service has not received UPD packet", 10*60, 0, true);	// 10m, default

CVariable<bool>		VerboseFEStatsTime("fs", "VerboseFEStatsTime", "Verbose FESTATS and FETIME", false, 0, true);

std::string	getPatchingAddress()
{
	std::string	la;

	if (IService::getInstance()->haveArg('D'))
	{
		// use the command line param if set
		la = IService::getInstance()->getArg('D');
	}
	else if (IService::getInstance()->ConfigFile.exists ("ListenAddress"))
	{
		// use the config file param if set
		la = IService::getInstance()->ConfigFile.getVar ("ListenAddress").asString();
	}

	NLNET::CInetAddress	addr(la);

	return string("http://") + addr.ipAddress() + PatchingURLFooter.get();
}

/*
 * Conditional beep
 */
void beepIfAllowed( uint freq, uint duration )
{
	if ( AllowBeep )
		beep( freq, duration );
}


/***
 *** Global functions for easy multithreadization
 ***/


/*
 * Update task
 */

// Update entity container + Process vision + sort by distance + calculate priorities
void updatePriorities()
{
	H_AUTO(UpdatePriorities);
	//CFrontEndService::instance()->entityContainer().update(); // now called by the mirror notification callback

	CFrontEndService::instance()->PrioSub.update();
}


// Update clients states
void updateClientsStates()
{
	H_AUTO(RemoveDeadClients);
#ifndef SIMUL_CLIENTS
	CFrontEndService	*service = CFrontEndService::instance();
	service->updateClientsStates();
#endif
}


/*
 * Send task (see CFeSendSub::update())
 */

// Prepare headers
void prepareHeadersAndFillImpulses()
{
	H_AUTO(PrepareHeaders);

	CFrontEndService::instance()->SendWatch.start();

	CFrontEndService::instance()->sendSub()->prepareSendCycle();
	if ( ! CFrontEndService::instance()->receiveSub()->clientMap().empty() )
	{
		CFrontEndService::instance()->sendSub()->prepareHeadersAndFillImpulses();
	}
}


// Fill prioritized actions
void fillPrioritizedActionsToSend()
{
	H_AUTO(FillPrioritizedActions);
	if ( ! CFrontEndService::instance()->receiveSub()->clientMap().empty() )
	{
		CFrontEndService::instance()->sendSub()->fillPrioritizedActions();
	}
}


// Swap send buffers
void swapSendBuffers()
{
	H_AUTO(WaitAndSwapSendBuffers)
    // Wait for the end of flushMessagesToSend()
    while ( FlushInProgress )
	{
		nlSleep( 0 );
	}

	// Swap the buffers
	CFrontEndService::instance()->sendSub()->swapSendBuffers();
    FlushInProgress = true;

	CFrontEndService::instance()->SendWatch.stop();
}


// Flush messages
void flushMessagesToSend()
{
	CFrontEndService::instance()->sendSub()->flushMessages();
	FlushInProgress = false;
}

/*
 * Send thread & runnable
 */
class CSendRunnable : public IRunnable
{
public:

	CSendRunnable() : StopThread(false), SendBuffer(false)	{}

	volatile bool	StopThread;
	volatile bool	SendBuffer;

	virtual void	run()
	{
		while (!StopThread)
		{
			while (!StopThread && !SendBuffer)
				nlSleep(0);

			if (StopThread)
				break;

			SendBuffer = false;
			flushMessagesToSend();
		}

		StopThread = false;
	}

};

CSendRunnable	SendTask;
IThread*		SendThread = NULL;

/*
 * Receive task
 */

// Swap receive queues
void swapReadBuffers()
{
	H_AUTO(SwapReadBuffers);
	CFrontEndService::instance()->receiveSub()->swapReadQueues();
}


// Read incoming data from the current read queue
void readIncomingData()
{
	H_AUTO(ReadIncomingData);
	CFrontEndService::instance()->receiveSub()->readIncomingData();
}



/**************************
 *         INIT           *
 **************************/


/*
 * Send an impulsion message to a destination client (warning: channel is "level", in fact)
 */
void sendImpulsion( TClientId clientid, CMessage& msgin, uint8 channel, const char *extendedMsg, bool forceSingleShot )
{
	uint32 bytelen;
	msgin.serial( bytelen ); // sent with serialBufferWithSize(), the following is the buffer

	CActionGeneric *ag = (CActionGeneric *)CActionFactory::getInstance()->create( INVALID_SLOT, ACTION_GENERIC_CODE );
	ag->setFromMessage( msgin, bytelen );
	ag->AllowExceedingMaxSize = forceSingleShot;
	sint32 impulseBitSize = (sint32)CActionFactory::getInstance ()->size( ag );
	sint32 maxImpulseBitSize = CFrontEndService::instance()->getImpulseMaxBitSize( channel );
	if ( forceSingleShot || (impulseBitSize < maxImpulseBitSize) )
	{
		// Simple impulsion
#ifdef TRACE_SHARD_MESSAGES
		LOG_IMPULSION_INFO("FETRACE:%u: Preparing simple impulsion %p for C%hu, %s (len=%u, %d/%d bits)", CTickEventHandler::getGameCycle(), ag, clientid, extendedMsg, bytelen, impulseBitSize, maxImpulseBitSize);
#endif
		CFrontEndService::instance()->addImpulseToClient (clientid, ag, channel);
	}
	else
	{
		// MultiPart impulsion
		CActionFactory::getInstance()->remove((CActionImpulsion*&)ag);
		CActionGenericMultiPart *agmp = (CActionGenericMultiPart *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_MULTI_PART_CODE);
		sint32 minimumBitSizeForMP = CActionFactory::getInstance ()->size (agmp);

		sint32 availableSize = (maxImpulseBitSize - minimumBitSizeForMP) / 8; // (in bytes)
#ifdef NL_DEBUG
		nlassert( availableSize > 0 ); // the available size must be larger than the 'empty' size
#endif
		sint32 nbBlock = (bytelen + availableSize - 1) / availableSize;

		TClientIdCont	&cont = CFrontEndService::instance()->receiveSub()->clientIdCont();
		if (clientid >= cont.size() || cont[clientid] == NULL)
		{
			nlwarning ("Can't send impulse to client %u (doesn't exist)", clientid);
			CActionFactory::getInstance()->remove((CActionImpulsion*&)agmp);
			return;
		}
		uint8 num = cont[clientid]->ImpulseMultiPartNumber++;

#ifdef TRACE_SHARD_MESSAGES
		LOG_IMPULSION_INFO("FETRACE:%u: Preparing multipart impulsion %p for C%hu, %s (len=%u, num %d, nbpart %d, size per block %d)", CTickEventHandler::getGameCycle(), agmp, clientid, extendedMsg, bytelen, num, nbBlock, availableSize);
#endif
		for (sint32 i = 0; i < nbBlock; i++)
		{
			if (i != 0)
				agmp = (CActionGenericMultiPart *)CActionFactory::getInstance ()->create (INVALID_SLOT, ACTION_GENERIC_MULTI_PART_CODE);

			agmp->set(num, (uint16)i, msgin.buffer() + msgin.getPos(), bytelen, availableSize, (uint16)nbBlock);
//			nldebug ("num %d part %d avail %d nbblock %d", num, i, availableSize, nbBlock);
			CFrontEndService::instance()->addImpulseToClient (clientid, agmp, channel);
		}
	}
	//nldebug( "Contents of impulsion: '%s'", toHexaString(ImpulseBMS.bufferAsVector()).c_str() );
}


/*
 * Callback called when the input service gives a login mapping
 */
void cbMapIdToUid( CMessage& msgin, const string& serviceName, NLNET::TServiceId serviceId )
{
	// Serial in the CEntityId and the Uid
	TUid uid;
	CEntityId id;
	msgin.serial( uid );
	msgin.serial( id );
	
	// Search the Uid in client hosts
	CFrontEndService *fe = CFrontEndService::instance();
	CClientHost *clienthost = fe->receiveSub()->findClientHostByUid( uid );
	if ( clienthost != NULL )
	{
		nlinfo( "%u: Mapping CEntityId %s to uid %u for client %u", CTickEventHandler::getGameCycle(), id.toString().c_str(), uid, clienthost->clientId() );
		if( clienthost->eId() != CEntityId::Unknown )
		{
			nlwarning( "Client %u had already a CEntityId: %s !", uid, clienthost->eId().toString().c_str() );
			return;
		}
		clienthost->setEId( id );
		// update security for client module
		cbEntityIdChanged(clienthost);
		
		if ( fe->receiveSub()->EntityToClient.findEntityId( id ) )
		{
			nlwarning( "The CEntityId %s is already owned by another client!", id.toString().c_str() );
			return;
		}
		fe->receiveSub()->EntityToClient.addId( id, clienthost->clientId() );

		// The entity index will be set later, in CVisionProvider::processVision()
		// The forwarding to the client is done by an impulsion from the back-end

		// Send its discrete properties to the client. It is necessary because
		// before the FE knows the CEntityId of the client, the vision provider
		// cannot update its properties
		/*for ( uint32 p = FIRST_DISCREET_PROPINDEX; p < fe->PrioSub.PropTranslator.nbPropertiesByEntityId( id ); ++p )
		{
			clienthost->PropDispatcher.setPriority( 0, (TPropIndex)p, fe->PrioSub.VisionArray.prioLoc( clienthost->clientId(), 0, (TPropIndex)p ), (TPriority)HIGHEST_PRIORITY );
		}*/
	
	}
	else
	{
		nlwarning( "Uid not found for client login (%s %u)", id.toString().c_str(), uid );
	}
}


/*
 * Receive an impulsion to send to a client, using a uid (user account id).
 * This is used for messages sent to a client before he has choosen his id (entity id).
 */
void cbImpulsionUid (CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	// Serialize in Uid
	TUid uid;
	msgin.serial(uid);

	CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
	CClientHost *clienthost;

	// Find the client using the Uid (either in limbo or in online clients)
	clienthost = frs->findClientHostByUid( uid, true );
	if ( clienthost != NULL )
	{
		// Send message to the client
		sendImpulsion( clienthost->clientId(), msgin, 1, toString( "from %s to UId %u", serviceName.c_str(), uid ).c_str(), false );
	}
	else
	{
		// The client has left
		nldebug( "Invalid recipient client uid for impulsion (%u) from %s", uid, serviceName.c_str() );
	}
}


/*
 * Receive an impulsion to send to a client, using a id
 */
void cbImpulsionId (CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	// Serialize in CEntityId
	CEntityId id;
	msgin.serial(id);

	// Find the client using the id
	TClientId clientid = CFrontEndService::instance()->receiveSub()->EntityToClient.getClientId( id );
	if ( clientid != INVALID_CLIENT )
	{
		// Send message
		sendImpulsion( clientid, msgin, 1, toString( "from %s to Id %s", serviceName.c_str(), id.toString().c_str() ).c_str(), false );
	}
	else
	{
		// The client has left
		nldebug( "Invalid recipient client id for impulsion (%s)", id.toString().c_str() );
	}
}


/*
 * Receive an impulsion, to send to a client, using a id and level (abusively called channel)
 */
void cbImpulsionIdChannel (CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	// Serialize in CEntityId
	CEntityId id;
	uint8 channel;
	msgin.serial(id);
	msgin.serial(channel);

	// Find the client using the id
	TClientId clientid = CFrontEndService::instance()->receiveSub()->EntityToClient.getClientId( id );
	if ( clientid != INVALID_CLIENT )
	{
		if ( channel > 2 )
			nlwarning( "Invalid level %hu for impulsion from %s", (uint16)channel, serviceName.c_str() );
		// Send message
		sendImpulsion( clientid, msgin, channel, toString( "with level %hu from %s to Id %s", (uint16)channel, serviceName.c_str(), id.toString().c_str() ).c_str(), false );
	}
	else
	{
		nldebug( "Invalid recipient client id for impulsion (level %hu) (%s)", (uint16)channel, id.toString().c_str() );
	}
}


/*
 * Receive an impulsion, to send to a client, for a database update
 */
void cbImpulsionDatabase (CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	// Serialize in CEntityId
	CEntityId id;
	msgin.serial(id);

	// Find the client using the id
	TClientId clientid = CFrontEndService::instance()->receiveSub()->EntityToClient.getClientId( id );
	if ( clientid != INVALID_CLIENT )
	{
		// Send database message into level 2.
		// The level 2 has 4 subchannels => subsequents sendings are not garanteed to be delivered
		// in the same order.
		//
		// Warning: the channel has a limited size. If the database if flooded by updates, further
		// database impulsions will be blocked.
		sendImpulsion( clientid, msgin, 2, toString( "for database from %s to Id %s", serviceName.c_str(), id.toString().c_str() ).c_str(), false ); // TEMP: multipart allowed (because first sending with no constraints)
	}
	else
	{
		nldebug( "Invalid recipient client id for database impulsion (%s)", id.toString().c_str() );
	}
}


/*
 *
 */
void cbImpulsionMultiDatabase (CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId)
{
	enum {
		SendDeltasToAll = 1,			// From class CCDBGroup; must match ;)
	};

	uint nbSent = 0;

	// Read message flag
	uint8	sendFlags = 0;				// Make warning C4700 silent
	msgin.serial (sendFlags);

	if ((sendFlags & SendDeltasToAll) == SendDeltasToAll)
	{
		// Broadcast to every connected client to this frontend
		// Store current position in input message
		sint32 msgPosBeforeData = msgin.getPos();

		// All connected clients
		THostMap::iterator ihm;
		for (ihm =CFrontEndService::instance()->receiveSub()->clientMap().begin();
			 ihm!=CFrontEndService::instance()->receiveSub()->clientMap().end(); ++ihm )
		{
			// See cbImpulsionDatabase
			CClientHost *destClient = GETCLIENTA(ihm);

			// Check if client is still online and has chosen a player to play with
			if (!destClient->isDisconnected () &&
				!destClient->eId ().isUnknownId())
			{
				// Set message position back
				msgin.seek( msgPosBeforeData, NLMISC::IStream::begin );


				sendImpulsion( destClient->clientId(), msgin, 2, "", false ); // multipart allowed
				// One more...
				++nbSent;
			}
		}
	}
	else
	{
		// Explicit list of clients is specified
		// Read recipient list
		vector<CEntityId> recipientIds;
		msgin.serialCont( recipientIds );

		// Store current position in input message
		sint32 msgPosAfterRecipients = msgin.getPos();

		for ( vector<CEntityId>::const_iterator itr=recipientIds.begin(); itr!=recipientIds.end(); ++itr )
		{
			// Find the client using the id
			const CEntityId& id = *itr;
			TClientId clientid = CFrontEndService::instance()->receiveSub()->EntityToClient.getClientId( id );
			if ( clientid != INVALID_CLIENT )
			{
				// Set message position back
				msgin.seek( msgPosAfterRecipients, NLMISC::IStream::begin );

				// See cbImpulsionDatabase
				sendImpulsion( clientid, msgin, 2, "", false ); // multipart allowed

				++nbSent;
			}
			// Skip recipients that are not connected on this front-end
		}
	}

	//if ( nbSent != 0 )
	//	nldebug( "Sent CDB_MULTI_IMPULSION to %u clients", nbSent );
}


/*
 * Set the names requested to the IOS
 */
void cbSetEntityNames( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	uint32 len;
	msgin.serial( len );
	for ( uint32 i=0; i!=len; ++i )
	{
		TDataSetRow entityIndex;
		string name;
		msgin.serial( entityIndex );
		msgin.serial( name );
		CFrontEndService::instance()->EntityNames[ entityIndex.getIndex() ] = name;
	}
	nlinfo( "Got %u names from service %hu", len, serviceId.get() );
}


/*
 * Server request frontend to disconnect a client
 */
void cbServerDisconnectClient( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	uint32	userId;
	msgin.serial(userId);

	cbDisconnectClient(userId, serviceName);
}

/*
 * Server request frontend to disconnect a client
 */
void cbDisconnectAllClients( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	for (THostMap::iterator it = CFrontEndService::instance()->receiveSub()->clientMap().begin (); it != CFrontEndService::instance()->receiveSub()->clientMap().end (); it++)
	{
		// Ask the client to disconnect
		CActionDisconnection *act = (CActionDisconnection*)(CActionFactory::getInstance()->create(INVALID_SLOT, ACTION_DISCONNECTION_CODE));
		GETCLIENTA(it)->ImpulseEncoder.add(act, 0);

		// Prepare removal of client on the front-end
		CFrontEndService::instance()->receiveSub()->removeFromRemoveList( GETCLIENTA(it)->clientId() ); // prevent to insert it twice
		CFrontEndService::instance()->receiveSub()->addToRemoveList( GETCLIENTA(it)->clientId() );
	}
}

/*
 * Return the name of an entity (if previously retrieved or "" if no name)
 */
std::string				getEntityName( const TDataSetRow& entityIndex ) 
{
	TEntityNamesMap::const_iterator itn = CFrontEndService::instance()->EntityNames.find( entityIndex.getIndex() );
	if ( itn != CFrontEndService::instance()->EntityNames.end() )
	{
		return (*itn).second;
	}
	else
	{
		return string("");
	}
}


/*
 * Welcome Service request frontend to switch to Accept Clients mode
 */
void setAcceptClients()
{
	CFrontEndService::instance()->AcceptClients = true;
	NLMISC::ICommand::execute("stopWebPatchServer", *InfoLog);
}

/*
 * Welcome Service request frontend to switch to Accept Clients mode
 */
void cbAcceptClients( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	nlinfo("cbAcceptClients: asked by service %s %d to accept clients (patching is no longer available on this server)", serviceName.c_str(), serviceId.get());
	setAcceptClients();
}



/*
 *
 */
TUnifiedCallbackItem cbFrontEndArray [] =
{
	{ "IMPULSION_ID", cbImpulsionId },
	{ "IMPULS_CH_ID", cbImpulsionIdChannel },
	{ "CDB_IMPULSION", cbImpulsionDatabase },
	{ "CDB_MULTI_IMPULSION", cbImpulsionMultiDatabase },
	{ "IMPULSION_UID", cbImpulsionUid },
	{ "CL_ID", cbMapIdToUid },
	{ "ENTITY_NAMES", cbSetEntityNames },
	{ "DISCONNECT_CLIENT", cbServerDisconnectClient },		// Called by server to disconnect a client
	{ "DISCONNECT_ALL_CLIENTS", cbDisconnectAllClients },	// Called by server to disconnect all clients at once

	{ "FS_ACCEPT", cbAcceptClients }
};



//
void cfcbPriorityMode( CConfigFile::CVar& var )
{
	uint8 pm = var.asInt();
	if ( pm == 0 )
	{
		nlwarning( "PriorityMode 0 (DistanceOnly) is not obsolete and not supported!" );
	}
	//CFrontEndService::instance()->PrioSub.Prioritizer.setPrioStrategy( (CPrioritizer::TPrioStrategy)1 );
	//nlinfo( "\tPriorityMode is now %hu", (uint16)pm );
	nlinfo( "Priority strategy is DistanceDelta" );
}


//
void cfcbTotalBandwidth( CConfigFile::CVar& var )
{
	uint32 tb = var.asInt();
	CFrontEndService::instance()->_SendSub.setTotalBandwidth( tb );
	// Not implemented anymore
	//nlinfo( "\tTotalBandwidth is now %u", tb );
	/*if ( CFrontEndService::instance()->_SendSub.clientBandwidth() != 0 )
	    nlinfo( "\tTotalbandwidth corresponds to %0.2f client buffers",
		    (float)CFrontEndService::instance()->_SendSub.totalBandwidth()/(float)CFrontEndService::instance()->_SendSub.clientBandwidth() );
	*/
}


//
void cfcbClientBandwidth( CConfigFile::CVar& var )
{
	uint32 cb = var.asInt();
	if ( cb != 0 )
	{
#ifdef INCLUDE_FE_STATS_IN_PACKETS
		cb += STAT_HEADER_SIZE - UDP_IP_ETH_HEADER_SIZE;
#else
		cb -= UDP_IP_ETH_HEADER_SIZE;
#endif
	    CFrontEndService::instance()->_SendSub.setClientBandwidth( cb );
	    nlinfo( "\tClientBandwidth is now %u bytes per game cycle, including UDP/IP/Ethernet headers (%u)", cb + UDP_IP_ETH_HEADER_SIZE, UDP_IP_ETH_HEADER_SIZE );
#ifdef INCLUDE_FE_STATS_IN_PACKETS
		nlinfo( "\t(also including stat header of %u bytes)", STAT_HEADER_SIZE );
#endif
	}
}


//
void cfcbClientTimeOut( CConfigFile::CVar& var )
{
	uint32 ct = var.asInt();
	CFrontEndService::instance()->_ClientTimeOut = ct;
	nlinfo( "\tClientTimeOut is now %u", ct );
}


//
void cfcbLimboTimeOut( CConfigFile::CVar& var )
{
	uint32 ct = var.asInt();
	CFrontEndService::instance()->_LimboTimeOut = ct;
	nlinfo( "\tLimboTimeOut is now %u", ct );
}


//
void cfcbDisplayInfo( CConfigFile::CVar& var )
{
	nlinfo ("\tDisplayInfo is now %s", var.asInt()!=0?"on":"off");
	if ( var.asInt() != 0 )
	{
		InfoLog->removeFilter ("FE:");
	}
	else
	{
		InfoLog->addNegativeFilter ("FE:");
	}
}


void cfcbClientMonitor( CConfigFile::CVar& var )
{
	TClientId clientid = var.asInt();
	CFrontEndService::instance()->monitorClient( clientid );
	if ( clientid != 0 )
	{
		if ( clientid <= MAX_NB_CLIENTS )
			nlinfo( "\tNow monitoring client %hu", clientid );
		else
			nlinfo( "Invalid client id for monitoring" );
	}
	else
	{
		nlinfo( "\tClient monitoring is now off" );
	}
}


void cfcbAllowBeep( CConfigFile::CVar& var )
{
	AllowBeep = (var.asInt() == 1);
	nlinfo( "\tAllowBeep is now %hu", (uint16)AllowBeep );
}


void cfcbGameCycleRatio( CConfigFile::CVar& var )
{
	sint gcratio = var.asInt();
	CFrontEndService::instance()->setGameCycleRatio( gcratio );
	nlinfo( "\tGameCycleRatio is now %d", gcratio );
}


void cfcbCalcDistanceExecutionPeriod( CConfigFile::CVar& var )
{
	sint period = var.asInt();
	CFrontEndService::instance()->PrioSub.VisionProvider.DistanceSpreader.ExecutionPeriod = period;
	nlinfo( "\tCalcDistanceExecutionPeriod is now %d", period );
}


void cfcbSortPrioExecutionPeriod( CConfigFile::CVar& var )
{
	sint period = var.asInt();
	CFrontEndService::instance()->PrioSub.Prioritizer.SortSpreader.ExecutionPeriod = period;
	nlinfo( "\tSortPrioExecutionPeriod is now %d", period );
}


void cfcbDistanceDeltaRatioForPos( CConfigFile::CVar& var )
{
	uint32 ddratio = var.asInt();
	CFrontEndService::instance()->PrioSub.Prioritizer.setDistanceDeltaRatioForPos( ddratio );
	nlinfo( "\tDistanceDeltaRatioForPos is now %u", ddratio );
}

//

/*void cfcbPositionPrioExecutionPeriod( CConfigFile::CVar& var )
{
	sint period = var.asInt();
	CFrontEndService::instance()->PrioSub.Prioritizer.PositionSpreader.ExecutionPeriod = period;
	nlinfo( "\tPositionPrioExecutionPeriod is now %d", period );
}


void cfcbOrientationPrioExecutionPeriod( CConfigFile::CVar& var )
{
	sint period = var.asInt();
	CFrontEndService::instance()->PrioSub.Prioritizer.OrientationSpreader.ExecutionPeriod = period;
	nlinfo( "\tOrientationPrioExecutionPeriod is now %d", period );
}


void cfcbDiscreetPrioExecutionPeriod( CConfigFile::CVar& var )
{
	sint period = var.asInt();
	CFrontEndService::instance()->PrioSub.Prioritizer.DiscreetSpreader.ExecutionPeriod = period;
	nlinfo( "\tDiscreetPrioExecutionPeriod is now %d", period );
}*/


/*
 * Init variable from config file, at startup
 */
void initConfigVar( CConfigFile& cf, const char *varname, uint& var, const uint maxvalue )
{
	var = cf.getVar( varname ).asInt();
	if ( var > maxvalue )
	{
		nlwarning( "%s=%u exceeding maximum", varname, var );
		var = maxvalue;
	}
	nlinfo( "\t%s = %u", varname, var );
}


/*
 * Install callback and init variable from config file
 */
void installConfigVar( CConfigFile& cf, const char *varname, void (*callback) (CConfigFile::CVar&) )
{
	cf.setCallback( varname, callback );
	callback( cf.getVar( varname ) );
}


/*
 *  Login system callback
 */
void cbDisconnectClient (TUid userId, const std::string &reqServiceName)
{
	for (THostMap::iterator it = CFrontEndService::instance()->receiveSub()->clientMap().begin (); it != CFrontEndService::instance()->receiveSub()->clientMap().end (); it++)
	{
		if ( GETCLIENTA(it)->Uid == userId)
		{
			// Ask the client to disconnect
			CActionDisconnection *act = (CActionDisconnection*)(CActionFactory::getInstance()->create(INVALID_SLOT, ACTION_DISCONNECTION_CODE));
			GETCLIENTA(it)->ImpulseEncoder.add(act, 0);
			nlinfo( "%s asked to remove player %hu (uid %u)", 
				reqServiceName.c_str(), 
				GETCLIENTA(it)->clientId(), 
				userId );

			// Prepare removal of client on the front-end
			CFrontEndService::instance()->receiveSub()->removeFromRemoveList( GETCLIENTA(it)->clientId() ); // prevent to insert it twice
			CFrontEndService::instance()->receiveSub()->addToRemoveList( GETCLIENTA(it)->clientId() );
			
			return;
		}
	}
	nlwarning( "%s asked to remove an unknown player (uid %u)", reqServiceName.c_str(), userId );
}


/*
 * Get the last target entity position that was sent to a client at a specified tick in the past
 */
void cbGetTargetPosAtTick( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
{
	// Read input
	TDataSetRow clientEntityIndex;
	TGameCycle tick;
	msgin.serial( clientEntityIndex );
	msgin.serial( tick );

	// Search
	bool found = false;
	TCoord x,y;
	TClientId clientId = CFrontEndService::instance()->receiveSub()->EntityToClient.getClientId( clientEntityIndex );
	if ( clientId != INVALID_CLIENT )
	{
		CFrontEndService::instance()->history()->getTargetPosAtTick( clientId, tick, x, y );
		found = (x != 0); // && (y != 0); // not needed
	}

	// Write output
	CMessage msgout( "TG_POS_AT" );
	msgout.serial( found );
	if ( found )
		msgout.serial( x, y );
	CUnifiedNetwork::getInstance()->send( serviceId, msgout ); // direct (reply)
}


static bool IosUp = false, EgsUp = false;

/*
 * Callback called at service connexion
 */
void cbServiceUp( const string& serviceName, NLNET::TServiceId serviceId, void * )
{
	if ( serviceName == "IOS" )
		IosUp = true;
	else if ( serviceName == "EGS" )
		EgsUp = true;
	else if ( serviceName == "WS" )
	{
		if (UseWebPatchServer)
		{
			// send patch server address
			CMessage			msgAddr("FEPA");

			// serial address as string
			std::string			patchAddress = getPatchingAddress();
			msgAddr.serial(patchAddress);

			// serial current frontend state
			bool				acceptClients = CFrontEndService::instance()->AcceptClients;
			msgAddr.serial(acceptClients);

			// send it to incoming WS
			CUnifiedNetwork::getInstance()->send(serviceId, msgAddr);

			nldebug("WS %d welcome service is up, sent server patch URI '%s'", serviceId.get(), patchAddress.c_str());
			if (CFrontEndService::instance()->AcceptClients)
			{
				nldebug("Note: frontend is yet in accept clients mode, patching is no longer available unless frontend is restarted", serviceId.get(), patchAddress.c_str());
			}
		}


		// send number of players on FS
		CMessage			msgPlr("NBPLAYERS2");
		uint32				nbClients = (uint32)CFrontEndService::instance()->receiveSub()->clientMap().size();
		uint32				nbPendingClients = CLoginServer::getNbPendingUsers();
		
		msgPlr.serial(nbClients);
		msgPlr.serial(nbPendingClients);

		// send it to incoming WS
		CUnifiedNetwork::getInstance()->send(serviceId, msgPlr);

		nldebug("WS %d welcome service is up, reported %d online users", serviceId.get(), nbClients);
	}

	if ( EgsUp )
	{
		CFrontEndService::instance()->ShardDown = false;
		nlinfo( "Leaving SERVER_DOWN mode" );
	}
}


/*
 * Callback called when service becomes down
 */
void cbServiceDown( const string& serviceName, NLNET::TServiceId serviceId, void * )
{
	if ( serviceName == "GPMS" )
	{
		// Reset the slots of the clients before the property_receiver deletes the entities
		CFrontEndService::instance()->PrioSub.VisionProvider.resetVision();
	}
	else if ( serviceName == "IOS" )
	{
		IosUp = false;
	}
	else if ( serviceName == "EGS" )
	{
		EgsUp = false;
	}

	if ( ! (EgsUp) ) // now the IOS shutdown is tolerated
	{
		if ( ! CFrontEndService::instance()->ShardDown ) // prevent from doing it several times
		{
			/*while( (ihm=CFrontEndService::instance()->receiveSub()->clientMap().begin()) != CFrontEndService::instance()->receiveSub()->clientMap().end() )
			{
				// Cannot use cbDisconnect() because we need the client to be removed at once, otherwise
				// the property receiver could raise an assert in assignIndexToId(clientsid)
				//CFrontEndService::instance()->receiveSub()->removeFromRemoveList( GETCLIENTA(ihm)->clientId() ); 
				//CFrontEndService::instance()->receiveSub()->removeClientById( GETCLIENTA(ihm)->clientId() );
			}*/
			
			CFrontEndService::instance()->ShardDown = true;
			nlinfo( "Entering SERVER_DOWN mode" );

			// remove all limbo clients
			uint	clientsInLimbo = (uint)CFrontEndService::instance()->receiveSub()->LimboClients.size();
			CFrontEndService::instance()->receiveSub()->LimboClients.clear();
			nlinfo("Removed %d clients in limbo mode", clientsInLimbo);
		}
	}

	beepIfAllowed( 440, 150 );
	beepIfAllowed( 400, 150 );
}


/*
 * Impulse ack callbacks
 */
void	cbAckUnknown(CClientHost *client, CAction *action)
{
	nldebug("FERECV: client %d acked action %d", client->clientId(), action->Code);
}


void	cbAckDummy(CClientHost *client, CAction *action)
{
	CActionDummy	*dummy = (CActionDummy*)(action);
	nldebug("FERECV: Client %d acked dummy %d", client->clientId(), dummy->Dummy1);
}


/*
 * Init module manager
 */

const sint FE_MAIN = 0;
const sint FE_FLUSH = 1;

void	CFrontEndService::initModuleManagers()
{
/*
	CModuleManager::init(16);

	// Warning: with several modules, the front-end service shows 100% of CPU in the
	// Windows Task Manager. This is normal, and the service is still nice to other programs.
	// For more information, see comment in CModuleManager::waitAllReady().

	CModuleManager	*mainMgr = new CModuleManager("FE_MAIN", false);
	CModuleManager  *flushMgr = new CModuleManager("FE_FLUSH", true);
	_ModuleManagers.push_back(mainMgr);
	_ModuleManagers.push_back(flushMgr);

#ifdef MEASURE_SENDING
	nlinfo( "MEASURE_SENDING enabled" );
	mainMgr->addModule(0, swapReadBuffers);
	mainMgr->addModule(1, readIncomingData);
	mainMgr->addModule(2, swapSendBuffers);
	flushMgr->addWait(2);
	flushMgr->addModule(3, flushMessagesToSend);
#else
	mainMgr->addModule(0, swapReadBuffers);
	mainMgr->addModule(1, readIncomingData);
	mainMgr->addModule(2, ::updateClientsStates);
	mainMgr->addModule(3, updatePriorities);
	mainMgr->addModule(4, prepareHeadersAndFillImpulses);
	mainMgr->addModule(5, fillPrioritizedActionsToSend);
	mainMgr->addModule(6, swapSendBuffers); // waits for the end of flushMessagesToSend using the boolean FlushInProgress (with addWait there would be a deadlock at the beginning)
	flushMgr->addWait(6);
	flushMgr->addModule(7, flushMessagesToSend);
#endif // MEASURE_SENDING
*/
}


/*
 *
 */
inline void cbUpdate()
{
	CFrontEndService::instance()->onTick();
}


/*
 *
 */
inline void	CFrontEndService::onTick()
{
	LastTickTime = CTime::getLocalTime();

	if (StalledMode)
	{
		setClientsToSynchronizeState();
	}

	if ( _GCCount == 0 )
	{
		// Run manager's task
		UserLWatch.start();
		//H_BEFORE(UserUpdate);
		if ( CFrontEndService::instance()->entityContainer().mirrorIsReady() )
		{
			//_ModuleManagers[0]->runOnce();

			swapReadBuffers();
			readIncomingData();
			::updateClientsStates();
			updatePriorities();
			prepareHeadersAndFillImpulses();
			fillPrioritizedActionsToSend();
			swapSendBuffers(); // waits for the end of flushMessagesToSend using the boolean FlushInProgress (with addWait there would be a deadlock at the beginning)

			SendTask.SendBuffer = true;

			if (SendThread == NULL)
			{
				H_AUTO(FlushMessagesToSend);
				flushMessagesToSend();
			}
		}
		//H_AFTER(UserUpdate);
		UserLWatch.stop();
		TMsDuration userduration = UserLWatch.getDuration();
		UserDurationPAverage = UserLWatch.getPartialAverage();
		//nlinfo( "Sent actions: %u", CFrontEndService::instance()->SentActionsLastCycle );

		_GCCount = _GCRatio;
	}

	--_GCCount;
}


/*
 * Initialization
 */

//CDebugDisplayer	FEDebugDisplayer("frontend_service_straight.log", true, "FE_DD");

void CFrontEndService::init()
{
	setVersion (RYZOM_VERSION);

	nlinfo( "Initializing front-end service..." );

	try
	{
		// Load config
		nlinfo( "Loading configuration from %s", ConfigFile.getFilename().c_str() );
		initConfigVar( ConfigFile, "ClientLimit", MaxNbClients, MAX_NB_CLIENTS );
		installConfigVar( ConfigFile, "ClientTimeOut", cfcbClientTimeOut );
		installConfigVar( ConfigFile, "LimboTimeOut", cfcbLimboTimeOut );
		installConfigVar( ConfigFile, "DisplayInfo", cfcbDisplayInfo );
		installConfigVar( ConfigFile, "ClientMonitor", cfcbClientMonitor );
		installConfigVar( ConfigFile, "AllowBeep", cfcbAllowBeep );
		installConfigVar( ConfigFile, "GameCycleRatio", cfcbGameCycleRatio );
		installConfigVar( ConfigFile, "CalcDistanceExecutionPeriod", cfcbCalcDistanceExecutionPeriod );
		installConfigVar( ConfigFile, "SortPrioExecutionPeriod", cfcbSortPrioExecutionPeriod );
		installConfigVar( ConfigFile, "DistanceDeltaRatioForPos", cfcbDistanceDeltaRatioForPos );
		/*installConfigVar( ConfigFile, "PositionPrioExecutionPeriod", cfcbPositionPrioExecutionPeriod );
		installConfigVar( ConfigFile, "OrientationPrioExecutionPeriod", cfcbOrientationPrioExecutionPeriod );
		installConfigVar( ConfigFile, "DiscreetPrioExecutionPeriod", cfcbDiscreetPrioExecutionPeriod );*/

#ifdef NL_OS_WINDOWS
		CConfigFile::CVar *lowerCPUPriority = ConfigFile.getVarPtr( "LowerCPUPriority" );
		if ( lowerCPUPriority && (lowerCPUPriority->asInt() == 1) )
		{
			SetPriorityClass( GetCurrentProcess(), /*BELOW_NORMAL_PRIORITY_CLASS*/ 0x00004000 ); // not the constant because I don't have the latest version of the Platform SDK!
			nlinfo( "Starting with LowerCPUPriority" );
		}
#endif

		CSheetId::init( false );

		// The filename is used with george's sheets to know which properties correspond to the entity types
		//	nlinfo( "Loading georges forms..." );
		//CFrontEndPropertyReceiver::init( string("single"), &PrioSub.PropTranslator, &_ReceiveSub.EntityToClient );
		nlinfo( "Initializing entity container and mirror..." );
		_EntityContainer.init( &_ReceiveSub.EntityToClient, cbUpdate );

		// Init the login system
		nlinfo( "Initializing login subsystem..." );
		CLoginServer::init( "", cbDisconnectClient ); 

//		// Init front end listening port
		CInetAddress listenAddr(CLoginServer::getListenAddress());
		uint16 frontendPort = listenAddr.port();

		if (frontendPort == 0)
		{
			// set a default listen port
			frontendPort = 47851;
		}
//		if (IService::getInstance()->haveArg('p'))
//		{
//			// use the command line param if set
//			frontendPort = NLMISC::fromString(IService::getInstance()->getArg('p').c_str());
//		}
//		else
//		{
//			// use the config file param otherwise
//			frontendPort = ConfigFile.getVar( "FrontendPort" ).asInt();
//		}

		CConfigFile::CVar *plafp = ConfigFile.getVarPtr( "LastAcceptableFSUDPPort" );
		uint16 lastAcceptableFrontendPort = frontendPort + 10; // default: try up to 10 ports
		if ( plafp )
		{
			if ( plafp->asInt() >= frontendPort )
				lastAcceptableFrontendPort = plafp->asInt(); 
			else
				nlwarning( "Invalid LastAcceptableFSUDPPort: %u (port from ListenAddress: %hu); using %hu", plafp->asInt(), frontendPort, lastAcceptableFrontendPort );
		}

		// Initialize the receiving subsystem and find an available port
		_DgramLength = ConfigFile.getVar( "DatagramLength" ).asInt();
		nlinfo( "\tDatagramLength = %u bytes", _DgramLength );
		nlinfo( "Initializing receiving subsystem..." );
		_ReceiveSub.init( frontendPort, lastAcceptableFrontendPort, _DgramLength, &_History, &_SendSub.clientIdCont() );
		frontendPort = _ReceiveSub.dataSock()->localAddr().port();
		listenAddr.setPort( frontendPort );
		CLoginServer::setListenAddress( PublishFSHostAsIP.get() ? listenAddr.asIPString() : (listenAddr.hostName() + ":" + NLMISC::toString( listenAddr.port() )) ); // note: asString() returns more information
		CInetAddress::RetrieveNames = false;

		StalledMode = false;
		LastTickTime = 0;
		ShardDown = true; // waiting for connection of the EGS
		CUnifiedNetwork::getInstance()->setServiceUpCallback( "*", cbServiceUp, NULL );
		CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbServiceDown, 0);
		SelfServiceId = getServiceId();

		std::string	patchAddress = getPatchingAddress();
		nlinfo("Patch Server base address is '%s'", patchAddress.c_str());

		// set accept mode
		AcceptClients = AcceptClientsAtStartup.get();
		if (!AcceptClients)
		{
			nlinfo("FS doesn't accept client at startup, must be told to do so (default patching mode)");
			NLMISC::ICommand::execute("startWebPatchServer", *InfoLog);
		}
		else
		{
			nlinfo("FS accepts client at startup (patching mode unavailable)");
			NLMISC::ICommand::execute("stopWebPatchServer", *InfoLog);
		}

		// Register actions
		CActionFactory::getInstance()->registerAction( ACTION_POSITION_CODE, CActionPosition::create );
		CActionFactory::getInstance()->registerAction( ACTION_SYNC_CODE, CActionSync::create );
		CActionFactory::getInstance()->registerAction( ACTION_DISCONNECTION_CODE, CActionDisconnection::create );
		CActionFactory::getInstance()->registerAction( ACTION_ASSOCIATION_CODE, CActionAssociation::create );
		CActionFactory::getInstance()->registerAction( ACTION_LOGIN_CODE, CActionLogin::create );
		CActionFactory::getInstance()->registerAction( ACTION_GENERIC_CODE, CActionGeneric::create );
		CActionFactory::getInstance()->registerAction( ACTION_GENERIC_MULTI_PART_CODE, CActionGenericMultiPart::create );
		CActionFactory::getInstance()->registerAction( ACTION_SINT64, CActionSint64::create );
		CActionFactory::getInstance()->registerAction( ACTION_TARGET_SLOT_CODE, CActionTargetSlot::create );
//#ifdef DISABLE_PRIOTABLE
		CActionFactory::getInstance()->registerAction( ACTION_DUMMY_CODE, CActionDummy::create );
//#endif

		// Init send subsystem
		nlinfo( "Initializing sending subsystem..." );
#ifdef HALF_FREQUENCY_SENDING_TO_CLIENT
		nlinfo( " Half-frequency mode" );
#else
		nlinfo( " Full-frequency mode" );
#endif
		_SendSub.init( _ReceiveSub.dataSock(), &_ReceiveSub.clientMap(), &_History, &PrioSub );
		installConfigVar( ConfigFile, "ClientBandwidth", cfcbClientBandwidth );
		installConfigVar( ConfigFile, "TotalBandwidth", cfcbTotalBandwidth );

		// Init history
		nlinfo( "Initializing history..." );
		_History.init( MaxNbClients+1 ); // clientids begin at 1

		// Init priority table
		nlinfo( "Initializing priority tables..." );
		PrioSub.init( /*&_SendSub.clientIdCont()*/ &_History, &_ReceiveSub.EntityToClient );
		installConfigVar( ConfigFile, "PriorityMode", cfcbPriorityMode );

		// Register property nbbits
		CActionSint64::registerNumericPropertiesRyzom();
		CActionFactory::getInstance()->initVolatileProperties();

		setUpdateTimeout(100);

		// Init the XML message manager
		nlinfo( "Initializing XML message manager..." );
		const string& pathXmlMsg = CPath::lookup( "msg.xml" );
		GenericXmlMsgHeaderMngr.init( pathXmlMsg );
		initImpulsionId();
		initImpulsionUid();

		// Init module manager
		nlinfo( "Initializing module manager..." );
		initModuleManagers();

		if (UseSendThread)
		{
			// Init send thread
			nlinfo("UseSendThread on, initialise send thread");
			SendThread = IThread::create(&SendTask);
			SendThread->start();
		}

		// add a callback in the LoginServer to be warned when a cookie become acceptable
		CLoginServer::addNewCookieCallback(newCookieCallback);

		nlinfo( "Waiting for the local Mirror Service..." );

		CActionGeneric::ServerSide = true;
	}
	catch (const Exception &e)
	{
		nlerror( "Error: %s", e.what() );
	}
}



/*
 * After mirror system is ready
 */
void	CFrontEndService::postInit()
{
	nlinfo( "Subscribing to mirror properties..." );
	_EntityContainer.initMirror();

	// Init modules
	nlinfo( "Starting modules..." );
	//CModuleManager::startAll();

	// Init impulse encoder callbacks
	CImpulseEncoder::setReceivedCallback(cbAckUnknown, -1);
	CImpulseEncoder::setReceivedCallback(cbAckDummy, ACTION_DUMMY_CODE);

	// and the remaining ones...
	setSimlagValues (ConfigFile.getVar("Lag").asInt(), ConfigFile.getVar("PacketLoss").asInt(), ConfigFile.getVar("PacketDuplication").asInt(), ConfigFile.getVar("PacketDisordering").asInt());
}


/**************************
 *       RELEASE          *
 **************************/



/*
 *
 */
void CFrontEndService::release()
{
	SendTask.StopThread = true;

	_ReceiveSub.release();

	//CModuleManager::stopAll();

	//
	//uint	i;
	//for (i=0; i<_ModuleManagers.size(); ++i)
	//	delete _ModuleManagers[i];

	//_ModuleManagers.clear();
	//CModuleManager::release();

	CActionFactory::release();

	InfoLog->removeFilter( "FEHTIMER" );
	InfoLog->removeFilter( "FE" );

	if (SendThread != NULL)
	{
		nlinfo("Release send thread, waiting for task to terminate");
		// stop send thread
		while (SendTask.StopThread)
			nlSleep(1);

		delete SendThread;
		SendThread = NULL;
	}
}



/**************************
 *        UPDATE          *
 **************************/


//#define LOOP_IN_MAIN_THREAD
//#define TEST_THREAD_CPU_USAGE

// This variable helps testing the bug that produced the warning
// "handleReceivedMsg : FERECV: client %d sent messages dated in future"
// after a Stall Mode recovery.
//CVariable<bool> SimulateStalled("fs", "SimulateStalled", "", false );


/*
 * Update
 */
bool CFrontEndService::update()
{
#ifdef LOOP_IN_MAIN_THREAD
	while ( true );
#endif
#ifdef TEST_THREAD_CPU_USAGE
	double sum=0.0;
	uint32 n = 0;
	while ( true )
	{
		++n;
		TTicks ticks = CTime::getPerformanceTime();
		uint32 k;
		for ( uint32 i=0; i!=0x10000000; ++i )
		{
			k=i*2;
		}
		double t = CTime::ticksToSecond( CTime::getPerformanceTime()-ticks );
		sum += t;
		nlinfo( "Time: %.3f ms, average: %.3f ms, k=%u", t*1000.0, 1000.0*sum/(double)n, k );
	}
#endif

/*
	map<TUid,CLimboClient>::iterator	it;
	TTime								timeout = CTime::getLocalTime()-_LimboTimeOut;
	for (it=_ReceiveSub.LimboClients.begin(); it!=_ReceiveSub.LimboClients.end(); )
	{
		map<TUid,CLimboClient>::iterator	itr = it++;
		// removes limbo client if it timed out
		if ((*it).second.Timeout < timeout)
			_ReceiveSub.LimboClients.erase(itr);
	}
*/

	// If the back-end is down, set all clients to "server down" state
	// (this breaks the tick synchronisation of the clients, anyway the clients disconnect after receiving "server down")
	if ( !DontNeedBackend && ShardDown )
	{
		static TTime lastSent = CTime::getLocalTime();
		TTime now = CTime::getLocalTime();
		if ( now - lastSent > 2000 )
		{
			sendServerProblemStateToClients( CClientHost::ServerDown );
			lastSent = now;
		}
	}

	// If not received a tick for 2 seconds, set all clients to stalled state
	// (this breaks the tick synchronisation of the clients, they will need a SYNC afterwards)
	if ((CTime::getLocalTime() - LastTickTime > 2000) /*|| (SimulateStalled.get() == 1)*/)
	{
		if (!StalledMode)
		{
			nlinfo( "Entering STALLED mode" );
			StalledMode = true;
			LastStallTime = 0;
		}

		if ((CTime::getLocalTime() - LastStallTime > 1000) /*|| (SimulateStalled.get() == 1)*/)
		{
			LastStallTime = CTime::getLocalTime();
			sendServerProblemStateToClients( CClientHost::Stalled );
		}
	}

	// update the pending user list
	CLoginServer::refreshPendingList();

	// Display statistics
	CycleWatch.stop();
	static TTime lastdisplay = CTime::getLocalTime();
	if ( VerboseFEStatsTime.get() && (CTime::getLocalTime() - lastdisplay > 1000) )
	{
		TMsDuration cyclepartialaverage = CycleWatch.getPartialAverage();
		nlinfo( "FETIME: Time in ms: Receive=%u Send=%u UserLoop=%u Cycle=%u (%.1f cps)",
				ReceiveWatch.getPartialAverage(), SendWatch.getPartialAverage(), UserDurationPAverage,
				cyclepartialaverage, 1000.0f/(float)cyclepartialaverage );
		lastdisplay = CTime::getLocalTime();
	}
	CycleWatch.start();


	uint32 now = CTime::getSecondsSince1970();
	bool noRecentComm = now - CFEReceiveTask::LastUDPPacketReceived > DelayBeforeUPDAlert;
	// check for potential communication problem
	if (CLoginServer::getNbPendingUsers() != 0
		|| this->_ReceiveSub.getNbClient() != 0)
	{
		if (noRecentComm)
		{
			// raise the alert for a limited time
			addStatusTag("CheckUDPComm");
		}
	}

	if (!noRecentComm)
	{
		// clear the UDP alert tag (if set)
		removeStatusTag("CheckUDPComm");
	}

	return true;
}


/*
 * Add an impulse to a specific client
 */
void	CFrontEndService::addImpulseToClient(TClientId client, CActionImpulsion *action, uint level)
{
	TClientIdCont	&cont = _ReceiveSub.clientIdCont();

	if (client >= cont.size() || cont[client] == NULL)
	{
		nlwarning ("Can't send impulse to client %d (doesn't exist)", client);
		return;
	}

	cont[client]->ImpulseEncoder.add(action, level);
}

/*
 * Add an impulse to a specific entity provided it is a client
 */
void	CFrontEndService::addImpulseToEntity( const TEntityIndex& entity, CActionImpulsion *action, uint level )
{
	TClientId	client = _ReceiveSub.EntityToClient.getClientId( entity );

	if (client != INVALID_CLIENT)
	{
		addImpulseToClient(client, action, level);
	}
	else
	{
		nlwarning ("Can't send impulse to E%u (doesn't exist or not a client)", entity.getIndex());
		return;
	}
}


/*
 * Remove clients that do not send datagrams anymore
 */
void CFrontEndService::updateClientsStates()
{
	TTime	ctime = CTime::getLocalTime();

	THostMap::iterator iclient, icremove;
	for ( iclient=_ReceiveSub.clientMap().begin(); iclient!=_ReceiveSub.clientMap().end(); )
	{
		CClientHost	*client = GETCLIENTA(iclient);
		// Check if receive time is not too old
		TTime	cltime = ctime - client->receiveTime();
/*
		// TEMP BEN
		CActionDummy	*dummy = (CActionDummy*)(CActionFactory::getInstance()->create(ACTION_DUMMY_CODE));
		dummy->Dummy1 = ++(client->LastSentDummy);
		addImpulseToClient(client->clientId(), dummy, 1);
		// TEMP BEN
*/
		switch (client->ConnectionState)
		{
		default:
			if (cltime > _ClientLagTime)
			{
				//nlwarning( "Client %u lags (no incoming data for %u ms), setting probe mode", GETCLIENTA(iclient)->clientId(), _ClientLagTime );
				client->setProbeState();
			}
			else
			{
				++iclient;
			}
			break;
		case CClientHost::Probe:
			if ( cltime > _ClientTimeOut )
			{
				nlinfo( "Client %u %s seems dead (no incoming data for %u ms)", GETCLIENTA(iclient)->clientId(), GETCLIENTA(iclient)->address().asString().c_str(), _ClientTimeOut );
				icremove = iclient;
				++iclient;
				_ReceiveSub.removeFromRemoveList( GETCLIENTA(icremove)->clientId() ); 
				_ReceiveSub.removeClientById( GETCLIENTA(icremove)->clientId() );
			}
			else
			{
				++iclient;
			}
			break;
		}
	}
}


/*
 * Set clients to stalled or "server down" mode (and send stalled msg immediately)
 */
void CFrontEndService::sendServerProblemStateToClients( uint connectionState )
{
	uint8 systemMsgCode;
	switch ( connectionState )
	{
	case CClientHost::ServerDown:
		systemMsgCode = SYSTEM_SERVER_DOWN_CODE;
		break;
	case CClientHost::Stalled:
		systemMsgCode = SYSTEM_STALLED_CODE;
		break;
	default:
		nlstop;
	}
	//nldebug( "Sending %s to clients", SystemMessagesNames[systemMsgCode] );
	//uint numClients = 0;
	THostMap::iterator iclient;
	for ( iclient=_ReceiveSub.clientMap().begin(); iclient!=_ReceiveSub.clientMap().end(); ++iclient )
	{
		// Send the stalled or server down code. Note: client->setupSystemHeader() calls getNextSendNumber().
		// If this is called more than once per tick (or less), the client's CurrentServerTick will get shifted,
		// leading to the warning "handleReceivedMsg : FERECV: client %d sent messages dated in future".
		// To avoid this, the FS *must* resynchronize the clients when exiting from stalled mode
		// (see setClientsToSynchronizeState()).
		CClientHost	*client = GETCLIENTA(iclient);
		client->ConnectionState = connectionState;
		_SendSub.enableSendBuffer( client->clientId() );
		TOutBox& outbox = _SendSub.outBox( client->clientId() );
		outbox.resetBufPos();
		client->setupSystemHeader( outbox, systemMsgCode );
		
		//outbox.displayStream(toString("STALLED:displayOutbox for %u:", client->clientId()).c_str());
		//++numClients;
	}
	swapSendBuffers(); // waits for the end of the previous background flushing
	flushMessagesToSend(); // flushes (blocking operation in the main thread)
	
	//nldebug( "Sent %s to %u clients", SystemMessagesNames[systemMsgCode], numClients );
}


/*
 * Set clients to synchronize state
 */
void CFrontEndService::setClientsToSynchronizeState()
{
	nlinfo( "Leaving STALLED mode" );
	StalledMode = false;

	THostMap::iterator iclient;
	for ( iclient=_ReceiveSub.clientMap().begin(); iclient!=_ReceiveSub.clientMap().end(); ++iclient )
	{
		CClientHost	*client = GETCLIENTA(iclient);

		// Set to Force Synchronize state to ensure an ack_sync received from client
		// will not overwrite the state with Connected
		client->setForceSynchronizeState();
	}
}

void CFrontEndService::newCookieCallback(const NLNET::CLoginCookie &cookie)
{
	// update the last UPD packet date 
	if (CFEReceiveTask::LastUDPPacketReceived == 0)
	{
		// set the date to 'now'. If no UPD traffic is detected after
		// a predetermined timer, an alert is set in the service status line.
		CFEReceiveTask::LastUDPPacketReceived = CTime::getSecondsSince1970();
	}
}



/*
 *
 */

NLMISC_DYNVARIABLE( bool, StalledMode, "true if no tick for 2 seconds" )
{
	if ( get )
		*pointer = CFrontEndService::instance()->StalledMode;
}


NLMISC_DYNVARIABLE( uint32, UserTime, "User duration partial average" )
{
	if ( get )
		*pointer = CFrontEndService::instance()->UserDurationPAverage;
}


NLMISC_DYNVARIABLE( uint32, SentActions, "User duration partial average" )
{
	if ( get )
		*pointer = CFrontEndService::instance()->SentActionsLastCycle;
}


bool Stats = false;

// This assumes the negative filter "FESTATS" is in the config file
NLMISC_COMMAND( switchStats, "Switch stats", "" )
{
	Stats = ! Stats;
	if ( Stats )
		InfoLog->removeFilter( "FESTATS" );
	else
		InfoLog->addNegativeFilter( "FESTATS" );
	return true;
}


NLMISC_COMMAND( monitorClient, "Set the client id to monitor", "<clientid>" )
{
	if (args.size() != 1) return false;
	TClientId clientid;
	NLMISC::fromString(args[0], clientid);
	if ( clientid <= MAX_NB_CLIENTS )
		CFrontEndService::instance()->monitorClient( clientid );
	return true;
}


NLMISC_COMMAND( disconnectClient, "Disconnect a client", "<clientid>" )
{
	if (args.size() != 1) return false;
	
	TClientId clientid;
	NLMISC::fromString(args[0], clientid);
	
	CClientHost *clienthost;
	if ( (clientid <= MaxNbClients) && ((clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid]) != NULL) )
	{
		cbDisconnectClient( clienthost->Uid, "FS user command" );
	}
	else
	{
		log.displayNL( "There is no such a client id" );
	}
	return true;
}


NLMISC_COMMAND( verboseImpulsions, "Turn on/off or check the state of verbose logging of impulsions", "" )
{
	if ( args.size() == 1 )
	{
		if ( args[0] == string("on") || args[0] == string("1") )
			verboseImpulsions=true;
		else if ( args[0] == string("off") || args[0] == string("0") )
			verboseImpulsions=false;
	}

	log.displayNL( "verboseImpulsions is %s", verboseImpulsions ? "on" : "off" );
	return true;
}


NLMISC_COMMAND( getTargetPosAtTick, "Get the last target entity position that was sent to a client at a specified tick in the past", "<clientId> <tick>" )
{
	if ( args.size() < 2 )
		return false;
	TClientId clientId;
	NLMISC::fromString(args[0], clientId);
	NLMISC::TGameCycle tick;
	NLMISC::fromString(args[1], tick);

	// Search
	bool found = false;
	TCoord x,y;
	if ( (clientId != INVALID_CLIENT) && (clientId <= MAX_NB_CLIENTS) )
	{
		CFrontEndService::instance()->history()->getTargetPosAtTick( clientId, tick, x, y );
		log.displayNL( "Position of target of C%hu was %d %d", clientId, x, y );
	}
	else
	{
		log.displayNL( "Invalid client id" );
	}

	return true;
}


class CImpulseStatPred
{
public:

	typedef	std::pair<float, CClientHost*>	TItem;
	bool	operator () (const TItem& a, const TItem& b) const
	{
		return a.first < b.first;
	}

};

NLMISC_COMMAND( dumpImpulseStats, "Dump Impulse stat to XML log", "<logfile> [[-]<num> [efficiency|queue]]")
{
	if (args.size() < 1)
		return false;

	sint	maxdump = -1;
	if (args.size() >= 2)
	{
		NLMISC::fromString(args[1], maxdump);
		maxdump = abs(maxdump);
	}

	bool	reverse = (args.size() >= 2 && args[1][0] == '-');

	std::string	criterion;
	if (args.size() >= 3)
		criterion = args[2];

	uint	ucriterion = 0;
	if (criterion == "efficiency")
		ucriterion = 1;
	if (criterion == "queue")
		ucriterion = 2;

	std::vector<std::pair<float, CClientHost*> >	result;

	THostMap&	clientmap = CFrontEndService::instance()->receiveSub()->clientMap();
	for (THostMap::iterator it = clientmap.begin (); it != clientmap.end (); it++)
	{
		CClientHost*	client = GETCLIENTA(it);
		float			comp = 0.0f;

		if (ucriterion == 1)
		{
			comp = client->ImpulseEncoder.leastEfficiencyRatio();
		}
		else if (ucriterion == 2)
		{
			comp = -(float)client->ImpulseEncoder.biggestQueueSize();
		}

		if (reverse)
			comp = -comp;

		result.push_back(std::make_pair<float, CClientHost*>(comp, client));
	}

	if (ucriterion != 0 && !result.empty())
	{
		std::sort(result.begin(), result.end(), CImpulseStatPred());
	}


	std::string	filename = args[0];
	COFile	file;
	COXml	xml;

	if (!file.open(filename) || !xml.init(&file))
		return false;

	xml.xmlPush("ImpulseDump");

	uint	num;
	for (num=0; num<result.size() && (maxdump <= 0 || num < uint(maxdump)); ++num)
	{
		CClientHost*	client = result[num].second;
		client->ImpulseEncoder.dump(xml);
	}

	xml.xmlPop();

	return true;
}



/*
 * Patching setup
 * Apache start/stop commands
 */

// Start Apache server
NLMISC_COMMAND( startWebPatchServer, "start Web server for patching", "")
{
	if (!UseWebPatchServer.get())
		return true;

	nlinfo("Execute startWebPatchServer...");

	if (!StartWebServerSysCommand.get().empty())
	{
		system(StartWebServerSysCommand.get().c_str());
	}

	return true;
}

// Stop Apache server
NLMISC_COMMAND( stopWebPatchServer, "stop Web server for patching", "")
{
	if (!UseWebPatchServer.get())
		return true;

	nlinfo("Execute stopWebPatchServer...");

	if (!StopWebServerSysCommand.get().empty())
	{
		system(StopWebServerSysCommand.get().c_str());
	}

	return true;
}

// Stop Apache server
NLMISC_COMMAND( acceptClients, "tells manually frontend to accept clients", "")
{
	setAcceptClients();
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CFrontEndService, dump)
{
	if (args.size() != 0)
		return false;

	log.displayNL("Frontend internal state :");
	log.displayNL("Listen address: %s", CLoginServer::getListenAddress().c_str() );
	log.displayNL("UDP sock listening on %s", _ReceiveSub.dataSock()->localAddr().asString().c_str() );
	return true;
}


/*
 * Declare a service with the class CFrontEndService, the names "FS" (short) and "frontend_service" (long).
 * The port is set to 37000 and the main callback array is CallbackArray.
 * Note: this is the port towards the other services of the shard (TCP communication).
 */
NLNET_SERVICE_MAIN( CFrontEndService, "FS", "frontend_service", 0, cbFrontEndArray, "", "" )
