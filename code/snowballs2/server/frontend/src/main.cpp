// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef SNOWBALLS_CONFIG
#define SNOWBALLS_CONFIG ""
#endif // SNOWBALLS_CONFIG

#ifndef SNOWBALLS_LOGS
#define SNOWBALLS_LOGS ""
#endif // SNOWBALLS_LOGS

// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

#include <nel/misc/vector.h>

// We're using the NeL Service framework and layer 5
#include <nel/net/service.h>
#include <nel/net/login_server.h>

#include <map>
#include <utility>

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

using namespace NLMISC;
using namespace NLNET;
using namespace std;

CCallbackServer *Clients = 0;

//TSockId clientfrom;

/*
 * Keep a list of the players connected to that Frontend. Only map Id
 * to a Connection
 */

struct CPlayer
{
	enum PlayerState
	{
		IDENTIFYING,
		ONLINE
	};
	CPlayer(uint32 Id, TSockId Con) : id(Id), con(Con), State(IDENTIFYING) { }
	uint32   id;
	TSockId  con;
	PlayerState State;
};

typedef map<uint32, CPlayer> _pmap;

_pmap localPlayers;


/****************************************************************************
 * cbChatClient
 *
 * Receive chat messages from a client and send it to the Chat Service.
 ****************************************************************************/
void cbChatClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	string message;

	// Input from the client is stored.
	msgin.serial (message);

	// Prepare the message to send to the CHAT service
	CMessage msgout ("CHAT");
	msgout.serial (message);

	/*
	 * The incomming message from the client is sent to the CHAT service
	 * under the "CHAT" identification.
	 */
	CUnifiedNetwork::getInstance ()->send( "CHAT", msgout );

	nldebug("SB: Received CHAT message \"%s\" from client \"%s\"", message.c_str(), clientcb.hostAddress(from).asString().c_str());
}


/****************************************************************************
 * cdChatService
 *
 * Receive chat messages from the Chat Service to send it to all the clients.
 ****************************************************************************/
void cbChatService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	string  message;

	// Input: process the reply of the chat service
	msgin.serial (message);

	// Output: send the reply to the client
	CMessage msgout ("CHAT");
	msgout.serial (message);

	// Send the message to all connected clients
	Clients->send (msgout, InvalidSockId);

	nldebug( "SB: Sent chat message \"%s\" to all clients", message.c_str());
}


/****************************************************************************
 * cbPosClient
 *
 * Receive position messages from a client and send it to the Position Service.
 ****************************************************************************/
void cbPosClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  id;
	CVector pos;
	float   angle;
	uint32  state;

	// Input from the client is stored.
	msgin.serial( id );
	msgin.serial( pos );
	msgin.serial( angle );
	msgin.serial( state );

	// Prepare the message to send to the Position service
	CMessage msgout ("ENTITY_POS");
	msgout.serial( id );
	msgout.serial( pos );
	msgout.serial( angle );
	msgout.serial( state );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CUnifiedNetwork::getInstance ()->send( "POS", msgout );

	//nldebug( "SB: Received ENTITY_POS from the client");
}


/****************************************************************************
 * cbPosService
 *
 * Receive position messages from the Position Service to send it to all the
 * clients.
 ****************************************************************************/
void cbPosService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32  id;
	CVector pos;
	float   angle;
	uint32  state;

	// Input: process the reply of the position service
	msgin.serial( id );
	msgin.serial( pos );
	msgin.serial( angle );
	msgin.serial( state );

	// Output: send the reply to the client
	CMessage msgout( "ENTITY_POS" );
	msgout.serial( id );
	msgout.serial( pos );
	msgout.serial( angle );
	msgout.serial( state );

	// Send the message to all connected clients
	Clients->send(msgout, InvalidSockId);

	//nldebug( "SB: Sent ENTITY_POS message to all the connected clients");
}

/****************************************************************************
 * cbTeleportService
 *
 * Test
 ****************************************************************************/
void cbTeleportService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32 id;
	CVector position;
	msgin.serial(id);
	msgin.serial(position);
	CMessage msgout("ENTITY_TP");
	msgout.serial(id);
	msgout.serial(position);
	Clients->send(msgout, InvalidSockId);
	nldebug("SB: Sent ENTITY_TP message to all the connected clients");
}


/****************************************************************************
 * cbAddClient
 *
 * Receive an ADD_ENTITY message from a client and send it to the Position
 * Service.
 ****************************************************************************/
void cbAddClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32 id;
	string name;
	uint8 race;
	CVector start(1840.0f + ((float)(rand() % 100) / 10.0f), -970.0f + ((float)(rand() % 100) / 10.0f), -23.0f); // kaetemi_todo: from config

	// Input from the client is stored.
	msgin.serial(id);
	msgin.serial(name);
	msgin.serial(race);


        if(from->appId() != 0)
	{
        	CPlayer *p = (CPlayer *)(void *)from->appId();
		if(id == p->id)
			p->State = CPlayer::ONLINE;
	}

	// Prepare the message to send to the Position service
	CMessage msgout("ADD_ENTITY");
	msgout.serial(id);
	msgout.serial(name);
	msgout.serial(race);
	msgout.serial(start);

	/*
	 * The incoming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CUnifiedNetwork::getInstance()->send("POS", msgout);

	nldebug("SB: Received ADD_ENTITY from the client");

	// kaetemi_todo: from config
	msgout = CMessage("CHAT");
	std::string chat_msg(std::string(">>>> Welcome to Snowballs, ") + name + std::string("!"));
	msgout.serial(chat_msg);
	Clients->send(msgout, from);
}


/****************************************************************************
 * cdAddService
 *
 * Receive an ADD_ENTITY messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbAddService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	bool    all;
	uint32  to;
	uint32  id;
	string  name;
	uint8   race;
	CVector start;

	// Input: process the reply of the position service
	msgin.serial( all );
	msgin.serial( to );
	msgin.serial( id );
	msgin.serial( name );
	msgin.serial( race );
	msgin.serial( start );

	// Output: prepare the reply to the clients
	CMessage msgout( "ADD_ENTITY" );
	msgout.serial( id );
	msgout.serial( name );
	msgout.serial( race );
	msgout.serial( start );

	if ( all == true )
	{
		// Send the message to all connected clients
		Clients->send(msgout, InvalidSockId );

		nldebug( "SB: Sent ADD_ENTITY message to all the connected clients");
	}
	else
	{
		// Send the message about a former connected client to the new client
		_pmap::iterator ItPlayer;
		ItPlayer = localPlayers.find(to);
		if ( ItPlayer == localPlayers.end() )
		{
			nlwarning( "New player id %u not found !", to );
		}
		else
		{
			TSockId conToClient = ((*ItPlayer).second).con;
			Clients->send( msgout, conToClient );

			nldebug( "SB: Sent ADD_ENTITY about all the connected clients to the new client.");
		}
	}

}


/****************************************************************************
 * cbRemoveClient
 *
 * Receive an REMOVE_ENTITY message from a client and send it to the Position
 * Service.
 ****************************************************************************/
void cbRemoveClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  id;

	// Input from the client is stored.
	msgin.serial( id );

	// Prepare the message to send to the Position service
	CMessage msgout( "REMOVE_ENTITY" );
	msgout.serial( id );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CUnifiedNetwork::getInstance ()->send( "POS", msgout );

	nldebug( "SB: Received REMOVE_ENTITY from the client");
}


/****************************************************************************
 * cdRemoveService
 *
 * Receive an REMOVE_ENTITY messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbRemoveService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32  id;

	// Input: process the reply of the position service
	msgin.serial( id );

	// Output: send the reply to the client
	CMessage msgout( "REMOVE_ENTITY" );
	msgout.serial( id );

	// Send the message to all connected clients
	Clients->send( msgout, InvalidSockId );

	nldebug( "SB: Sent REMOVE_ENTITY message to all the connected clients");
}


/****************************************************************************
 * cdSnowballService
 *
 * Receive an SNOWBALL messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbSnowballService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32  id,
			playerId;
	CVector start,
			target;
	float   speed,
			explosionRadius;

	// Input: process the reply of the position service
	msgin.serial( id );
	msgin.serial( playerId );
	msgin.serial( start );
	msgin.serial( target );
	msgin.serial( speed );
	msgin.serial( explosionRadius );

	// Output: send the reply to the client
	CMessage msgout( "SNOWBALL" );
	msgout.serial( id );
	msgout.serial( playerId );
	msgout.serial( start );
	msgout.serial( target );
	msgout.serial( speed );
	msgout.serial( explosionRadius );

	// Send the message to all connected clients
	Clients->send( msgout, InvalidSockId );

	nldebug( "SB: Sent SNOWBALL message to all the connected clients");
}


/****************************************************************************
 * cbSnowballClient
 *
 * Receive an SNOWBALL message from a client and send it to the Position
 * Service.
 ****************************************************************************/
void cbSnowballClient ( CMessage& msgin, TSockId from, CCallbackNetBase& clientcb )
{
	uint32  playerId;
	CVector start,
			target;
	float   speed,
			explosionRadius;

	// Input from the client is stored.
	msgin.serial( playerId );
	msgin.serial( start );
	msgin.serial( target );
	msgin.serial( speed );
	msgin.serial( explosionRadius );

	// Prepare the message to send to the Position service
	CMessage msgout( "SNOWBALL" );
	msgout.serial( playerId );
	msgout.serial( start );
	msgout.serial( target );
	msgout.serial( speed );
	msgout.serial( explosionRadius );

	/*
	 * The incomming message from the client is sent to the Position service
	 * under the "POS" identification.
	 */
	CUnifiedNetwork::getInstance ()->send( "POS", msgout );

	nldebug( "SB: Received SNOWBALL from the client");
}


/****************************************************************************
 * cdHitService
 *
 * Receive an HIT messages from the Position Service to send it to all
 * the clients.
 ****************************************************************************/
void cbHitService (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	uint32  snowballId,
			victimId;
	bool    direct;

	// Input: process the reply of the position service
	msgin.serial( snowballId );
	msgin.serial( victimId );
	msgin.serial( direct );

	// Output: send the reply to the client
	CMessage msgout( "HIT" );
	msgout.serial( snowballId );
	msgout.serial( victimId );
	msgout.serial( direct );

	// Send the message to all connected clients
	Clients->send( msgout, InvalidSockId);

	nldebug( "SB: Sent HIT message to all the connected clients");
}


/**
 * Contains all callbacks from client
 */
TCallbackItem ClientCallbackArray[] =
{
	{ "ADD_ENTITY",    cbAddClient      },
	{ "ENTITY_POS",    cbPosClient      },
	{ "CHAT",          cbChatClient     },
	{ "REMOVE_ENTITY", cbRemoveClient   },
	{ "SNOWBALL",      cbSnowballClient },
};


/**
 * Contains *all* callbacks from the shard
 */
TUnifiedCallbackItem CallbackArray[] =
{
	{ "CHAT",			cbChatService		},
	{ "ADD_ENTITY",		cbAddService		},
	{ "ENTITY_POS",		cbPosService		},
	{ "ENTITY_TP",		cbTeleportService	},
	{ "REMOVE_ENTITY",	cbRemoveService		},
	{ "SNOWBALL",		cbSnowballService	},
	{ "HIT",			cbHitService		},
};


/****************************************************************************
 * Connection callback for the Chat service
 ****************************************************************************/
void onReconnectChat (const std::string &serviceName, TServiceId sid, void *arg)
{
	nldebug( "SB: Chat Service reconnected" );
}


/****************************************************************************
 * Disconnection callback for the Chat service
 ****************************************************************************/
void onDisconnectChat (const std::string &serviceName, TServiceId sid, void *arg)
{
	/* Note: messages already forwarded should get no reply, but it may occure
	 * (e.g. if the server reconnects before the forwarding of a message and
	 * the reconnection callbacks is called after that). Then onReconnectChat()
	 * may send messagess that have already been sent and the front-end may get
	 * the same message twice. This is partially handled in cbChatService.
	 */

	nldebug( "SB: Chat Service disconnecting: messages will be delayed until reconnection" );
}


/****************************************************************************
 * Connection callback for the Position service
 ****************************************************************************/
void onReconnectPosition (const std::string &serviceName, TServiceId sid, void *arg)
{
	nldebug( "SB: Position Service reconnected" );
}


/****************************************************************************
 * Disconnection callback for the Position service
 ****************************************************************************/
void onDisconnectPosition (const std::string &serviceName, TServiceId sid, void *arg)
{
	/* Note: messages already forwarded should get no reply, but it may occure
	 * (e.g. if the server reconnects before the forwarding of a message and
	 * the reconnection callbacks is called after that). Then onReconnectChat()
	 * may send messagess that have already been sent and the front-end may get
	 * the same message twice. This is partially handled in cbPositionService.
	 */

	nldebug( "SB: Position Service disconnecting: messages will be delayed until reconnection" );
}


/****************************************************************************
 * Connection callback for a client
 ****************************************************************************/
void onConnectionClient (TSockId from, const CLoginCookie &cookie)
{
	uint32 id;

	id = cookie.getUserId();

	nlinfo( "The client with unique Id %u is connected", id );

	// Add new client to the list of player managed by this FrontEnd
	pair<_pmap::iterator, bool> player = localPlayers.insert( make_pair( id, CPlayer( id, from )));
	
	// store the player info in appId

	_pmap::iterator it = player.first;
	CPlayer *p = &((*it).second);
	from->setAppId((uint64)(uintptr_t)p);

	// Output: send the IDENTIFICATION number to the new connected client
	CMessage msgout( "IDENTIFICATION" );
	msgout.serial( id );

	// Send the message to connected client "from"
	Clients->send( msgout, from );

	nldebug( "SB: Sent IDENTIFICATION message to the new client");
}


/****************************************************************************
 * Disconnection callback for a client
 ****************************************************************************/
void onDisconnectClient ( TSockId from, void *arg )
{
	uint32 id;

	uintptr_t i = from->appId();

	if(i == 0)
		return;

	CPlayer *p = (CPlayer *)(void *)i;
	id = p->id;

	nlinfo( "A client with unique Id %u has disconnected", id );

	// tell the login system that this client is disconnected
	CLoginServer::clientDisconnected ( id );

	// remove the player from the local player list
	localPlayers.erase( id );

	// don't send remove messages for entities that haven't been created.
	if(p->State == CPlayer::ONLINE)
	{
		// Output: send the REMOVE_ENTITY to the position manager.
		CMessage msgout( "REMOVE_ENTITY" );
		msgout.serial( id );

		// Send the message to the position manager
		CUnifiedNetwork::getInstance ()->send( "POS", msgout);
		nldebug( "SB: Sent REMOVE_ENTITY message to the position manager.");
	}

}


/****************************************************************************
 * CFrontEndService
 ****************************************************************************/
class CFrontEndService : public IService
{
public:

	// Initialisation
	void init()
	{

		// Create the server where the client must connect into
		// In a real game, it should be an UDP server with specific protocol to manage packet lost and so on.

		Clients = new CCallbackServer ();
		nlassert (Clients != 0);

		// Set the callbacks for that connection (comming from the Chat service)
		Clients->addCallbackArray (ClientCallbackArray, sizeof(ClientCallbackArray)/sizeof(ClientCallbackArray[0]));

		// Set the callbacks for the client disconnection of the Frontend
		Clients->setDisconnectionCallback (onDisconnectClient, 0);
		
		Clients->init (37000);

		// Connect the frontend to the login system
		CLoginServer::init( *Clients, onConnectionClient);

		/*
		 * Set the callback function when the Chat service reconnect to the
		 * frontend
		 */
		CUnifiedNetwork::getInstance ()->setServiceUpCallback ("CHAT", onReconnectChat, 0);

		/*
		 * Set the callback function when the Chat service disconnect from
		 * frontend
		 */
		CUnifiedNetwork::getInstance ()->setServiceDownCallback ("CHAT", onDisconnectChat, 0);

		/*
		 * Set the callback function when the Position service reconnect to the
		 * frontend
		 */
		CUnifiedNetwork::getInstance ()->setServiceUpCallback ("POS", onReconnectPosition, 0);

		/*
		 * Set the callback function when the Position service disconnect from
		 * frontend
		 */
		CUnifiedNetwork::getInstance ()->setServiceDownCallback ("POS", onDisconnectPosition, 0);
	}

	bool update()
	{
		// Manage messages from clients
		Clients->update ();

		// we want to continue
		return true;
	}

	void release()
	{
		delete Clients;
		Clients = NULL;
	}
};


/****************************************************************************   
 * SNOWBALLS FRONTEND SERVICE MAIN Function
 *
 * This call create a main function for a service:
 *
 *    - based on the "CFrontEndService" class
 *    - having the short name "FS"
 *    - having the long name "frontend_service"
 *    - listening on the port "0" (dynamically determined)
 *    - and shard callback set to "CallbackArray"
 *
 ****************************************************************************/
NLNET_SERVICE_MAIN (CFrontEndService, "FS", "frontend_service", 0, CallbackArray, SNOWBALLS_CONFIG, SNOWBALLS_LOGS)


/* end of file */
