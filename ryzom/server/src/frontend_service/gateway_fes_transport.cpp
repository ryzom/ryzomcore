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
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_gateway.h"
#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/module_security.h"
#include "frontend_service.h"
#include "client_host.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CLFECOMMON;

extern CGenericXmlMsgHeaderManager GenericXmlMsgHeaderMngr;

/** the specialized route for frontend server transport */
class CFEServerRoute : public CGatewayRoute
{
public:
	/// ClientId of the client
	TClientId		ClientId;
	/// FS channel communication number
	uint8			Channel;
	/// Message numbering for client reordering
	mutable uint8	SerialNumber;

	CFEServerRoute(IGatewayTransport *transport, TClientId clientId, uint8 channel)
		: CGatewayRoute(transport),
			ClientId(clientId), 
			Channel(channel),
			SerialNumber(0)
	{
		// warn the client that the transport is open
		CMessage msg;
		sendRawMessage("MODULE_GATEWAY:FEOPEN", msg);
	}

	~CFEServerRoute() NL_OVERRIDE
	{
		// warn the client that the transport is closed
		CMessage msg;

		// check special case when application close
		if (!CActionFactory::isInstanceAllocated())
			return;
		sendRawMessage("MODULE_GATEWAY:FECLOSE", msg);
	}


	void sendMessage(const CMessage &message) const NL_OVERRIDE
	{
		sendRawMessage("MODULE_GATEWAY:GATEWAY_MSG", message);
	}
private:
	// send a message for the
	void sendRawMessage(const char *transportMessageName, const CMessage &message) const
	{
		// put the message in a bit mem stream for transmission
		CBitMemStream bms;
		GenericXmlMsgHeaderMngr.pushNameToStream( transportMessageName, bms );
		// serial the message number for reordering on client side
		bms.serial(SerialNumber);
		++SerialNumber;
		// put the message in the stream
		bms.serialBufferWithSize(const_cast<uint8*>(message.buffer()), message.length());

		// wrap the bit mem in a transport message (for inpulsion sender), that's a bit nut !?!
		// TODO : write a better module message sender, avoiding useless data copy
		CMessage wrapper("IMPULTION_ID");
		wrapper.serialMemStream( bms );

		// Send message to the client
		wrapper.invert();
		sendImpulsion( ClientId, wrapper, Channel, toString( "from FE to ClientId %u", ClientId ).c_str(), false );
	}
};




#define FE_SERVER_CLASS_NAME "FEServer"
/** Transport for module gateway through front end service, server part. */
class CGatewayFEServerTransport : public IGatewayTransport
{
	friend class CFEServerRoute;
public:
	/// The table that keep track of all routes (from the user id)
	typedef std::map<TClientId, CFEServerRoute*>	TRouteMap;
	TRouteMap	_Routes;

	// store the unique active transport (only one transport of this type can be activated at a time)
	static CGatewayFEServerTransport *&OpenTransport()
	{
		static CGatewayFEServerTransport *openTransport = NULL;

		return openTransport;
	}

	/// Constructor
	CGatewayFEServerTransport(const IGatewayTransport::TCtorParam &param) 
		: IGatewayTransport(param)
	{
		// The routes are game clients: they see only the modules that address them, not each other's, and the gateway
		// limits what they can disclose (see openServer)
		Firewalled = true;
		PeerInvisible = true;
	}

	~CGatewayFEServerTransport() NL_OVERRIDE
	{
		if (OpenTransport() == this)
		{
			// the transport is still open, close it before destruction
			closeServer();
		}
	}

	const std::string &getClassName() const NL_OVERRIDE
	{
		static string className(FE_SERVER_CLASS_NAME);
		return className;
	}

	virtual void update() NL_OVERRIDE
	{
	}

	virtual uint32 getRouteCount() const NL_OVERRIDE
	{
		return (uint32)_Routes.size();
	}

	void dump(NLMISC::CLog &log) const NL_OVERRIDE
	{
		IModuleManager &mm = IModuleManager::getInstance();
		log.displayNL("  Frontend service transport, SERVER part");
		if (OpenTransport() != this)
		{
			log.displayNL("  The server is currently closed.");
		}
		else
		{
			log.displayNL("  The server is open and support %u routes :", _Routes.size());
			TRouteMap::const_iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				uint32 userId = first->first;
				CFEServerRoute *route = first->second;
				log.displayNL("    + route to user %u, %u entries in the proxy translation table :",
					userId,
					route->ForeignToLocalIdx.getAToBMap().size());

				{
					CGatewayRoute::TForeignToLocalIdx::TAToBMap::const_iterator first(route->ForeignToLocalIdx.getAToBMap().begin()), last(route->ForeignToLocalIdx.getAToBMap().end());
					for (; first != last; ++first)
					{
						IModuleProxy *modProx = mm.getModuleProxy(first->second);

						log.displayNL("      - Proxy '%s' : local proxy id %u => foreign module id %u",
							modProx != NULL ? modProx->getModuleName().c_str() : "ERROR, invalid module",
							first->second,
							first->first);
					}
				}
			}
		}
	}

	void onCommand(const CMessage &command) NL_OVERRIDE
	{
		// nothing done for now
		throw EInvalidCommand();
	}
	/// The gateway send a textual command to the transport
	bool onCommand(const TParsedCommandLine &command) NL_OVERRIDE
	{
		if (command.SubParams.size() < 1)
			throw  EInvalidCommand();

		const std::string &commandName = command.SubParams[0]->ParamName;
		if (commandName == "open")
		{
			if (OpenTransport() != NULL)
			{
				nlwarning("Another transport of this type is already open, only one can be open at a time");
				return false;
			}

			openServer();
		}
		else if (commandName == "close")
		{
			closeServer();
		}
		else
			return false;

		return true;
	}

	/// Open the server by intercepting client gateway message
	void openServer()
	{
		if (OpenTransport() != NULL)
			throw ETransportError("openServer : a transport is already open !");

		// a game client would see every module and other clients' modules, and could disclose modules under any name
		if (!Firewalled || !PeerInvisible)
		{
			// the command log alone would hide it: the frontend runs on, with no route for the clients' modules
			nlwarning("Frontend gateway transport not opened: it must be Firewalled and PeerInvisible, its routes are game clients (gw.transportOptions fes(PeerInvisible Firewalled))");
			throw ETransportError("openServer : the frontend transport must be Firewalled and PeerInvisible");
		}

		// set this transport as the open transport
		OpenTransport() = this;

		// Clients connected now are not told: a FEOPEN without a route has no serial number for the client to read
		// (it would throw and end the client), and no serial could be right, since the client numbers the messages per
		// transport while each new route starts at 0. A client opens its route by sending FEOPEN when its own gateway
		// transport opens (the ring editor modules); a client whose transport was already open keeps no route here
		// until it opens it again.
		uint nbClients = (uint)CFrontEndService::instance()->receiveSub()->clientMap().size();
		if (nbClients != 0)
			nlwarning("Frontend gateway transport opened with %u clients connected: those whose own gateway transport is open (ring editors) have no route until they open it again", nbClients);
	}

	/// Close the server, this will close the listing socket and any active connection
	void closeServer()
	{
		if (OpenTransport() != this)
			throw ETransportError("closeServer : The server is not open");

		// close all client connections
		while (!_Routes.empty())
		{
			CFEServerRoute *route = _Routes.begin()->second;

			// warn the gateway
			_Gateway->onRouteRemoved(route);

			// delete route and cleanup
			_Routes.erase(_Routes.begin());
			delete route;
		}

		// this transport is no longer open
		OpenTransport() = NULL;
	}


	/***************************************************/
	/** Event management                              **/
	/***************************************************/

	// handle the connection of a new client on the server
	void onConnection ( TClientId clientId)
	{
		if (_Routes.find(clientId) != _Routes.end())
		{
			nlwarning("Connection of client %u, we already have a gateway route for him !", clientId);
			return;
		}

		// Create a new route for this connection
		CFEServerRoute* route = new CFEServerRoute(this, clientId, 1);

		// store the route information
		_Routes.insert(make_pair(clientId, route));

		// callback the gateway
		_Gateway->onRouteAdded(route);
	}

	// handle the deconnection of a new client on the server
	void onDisconnection ( TClientId clientId)
	{
		TRouteMap::iterator it(_Routes.find(clientId));
		if (it == _Routes.end())
		{
			nldebug("Disconnection of client %u, no gateway route for him", clientId);
			return;
		}

		// callback the gateway that this route is no more
		_Gateway->onRouteRemoved(it->second);

		// delete the route
		CFEServerRoute *route = it->second;
		_Routes.erase(it);
		delete route;
	}

	// Called to dispatch an incoming message to the gateway
	void onDispatchMessage(NLMISC::CBitMemStream &bms, TClientId clientId)
	{
		TRouteMap::iterator it(_Routes.find(clientId));
		if (it == _Routes.end())
		{
			nlwarning("Dispatching message from client %u, no gateway route for him, message will be discarded", clientId);
			return;
		}

		// Build a CMessage from the bit mem stream
//		string msgName;
//		bms.serial(msgName);

		// create an input stream for dispatching
		CMessage msg("", true);
		uint32 len;
		bms.serial(len);
		// the length comes from the client: it must fit in what is left of the message before the buffer is allocated
		if (len > bms.length() - (uint32)bms.getPos())
		{
			CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
			CClientHost *client = clientId < frs->clientIdCont().size() ? frs->clientIdCont()[clientId] : NULL;
			nlwarning("FEHACK: client %u sent a gateway message of %u bytes in %u, discarded", (uint)clientId, len, bms.length() - (uint32)bms.getPos());
			frs->rejectReceivedMessage( HackedSizeInBuffer, client != NULL ? client->Uid : 0 );
			return;
		}
		// The header is the client's too. The client writes it in binary: 4 bytes, the format, then the length and the
		// characters of the name. An empty message would be read from the uninitialised 1000-byte buffer that
		// bufferToFill(0) leaves in place.
		const uint32 minHeaderSize = 4 + 1 + 4;
		if (len < minHeaderSize)
		{
			CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
			CClientHost *client = clientId < frs->clientIdCont().size() ? frs->clientIdCont()[clientId] : NULL;
			nlwarning("FEHACK: client %u sent a gateway message of %u bytes, shorter than a message header, discarded", (uint)clientId, len);
			frs->rejectReceivedMessage( InsufficientSize, client != NULL ? client->Uid : 0 );
			return;
		}
		bms.serialBuffer(msg.bufferToFill(len), len);
		// readType asserts (in setType) that the name is not empty, and ignores a short format, which the L5 transport of
		// the service the message goes to fails on (nlerror in readTypeAtCurrentPos). The header is read on a stream of
		// its own: the message has no read length before readType.
		CMessage::TFormat format;
		uint32 nameLength;
		{
			NLMISC::CMemStream header(true);
			header.fill(msg.buffer(), minHeaderSize);
			header.seek(4, NLMISC::IStream::begin);
			header.serial(format);
			header.serial(nameLength);
		}
		if (format.StringMode || !format.LongFormat || nameLength == 0 || nameLength > len - minHeaderSize)
		{
			CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
			CClientHost *client = clientId < frs->clientIdCont().size() ? frs->clientIdCont()[clientId] : NULL;
			nlwarning("FEHACK: client %u sent a gateway message with a header of format %u and a name of %u bytes in %u, discarded", (uint)clientId, (uint)msg.buffer()[4], nameLength, len - minHeaderSize);
			frs->rejectReceivedMessage( MalformedAction, client != NULL ? client->Uid : 0 );
			return;
		}
		msg.readType();

		// forward to gateway
		_Gateway->onReceiveMessage(it->second, msg);
	}

	// Uid impulsion gateway open callback handler
	void impulsionUidGatewayOpen(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
	{
		CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();

		// TODO optimise Uid to client retrieval
		CClientHost *clientHost = frs->findClientHostByUid(uid, false);
		if (clientHost == NULL)
		{
			nlwarning("impulsionUidGatewayOpen : can't find CClientHost for uid %u, discards message", uid);
			return;
		}

		onConnection(clientHost->clientId());
	}
	// Id impulsion gateway open callback handler
	void impulsionIdGatewayOpen( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
	{
		// retrieve uid
		CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
		TClientId clientId = frs->EntityToClient.getClientId(sender);
		if (clientId == INVALID_CLIENT)
		{
			nlwarning("impulsionIdGatewayOpen : can't find clientId for EID %s, discards message", sender.toString().c_str());
			return;
		}

		onConnection(clientId);
	}

	// Uid impulsion gateway message callback handler
	void impulsionUidGatewayMessage(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
	{
		CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();

		// TODO optimise Uid to client retrieval
		CClientHost *clientHost = frs->findClientHostByUid(uid, false);
		if (clientHost == NULL)
		{
			nlwarning("impulsionUidGatewayMessage : can't find CClientHost for uid %u, discards message", uid);
			return;
		}

		onDispatchMessage(bms, clientHost->clientId());
	}

	// Id impulsion gateway message callback handler
	void impulsionIdGatewayMessage( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
	{
		// retrieve uid
		CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
		TClientId clientId = frs->EntityToClient.getClientId(sender);
		if (clientId == INVALID_CLIENT)
		{
			nlwarning("impulsionIdGatewayMessage : can't find clientId for EID %s, discards message", sender.toString().c_str());
			return;
		}

		onDispatchMessage(bms, clientId);
	}

	// Uid impulsion gateway open callback handler
	void impulsionUidGatewayClose(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
	{
		CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();

		// TODO optimise Uid to client retrieval
		CClientHost *clientHost = frs->findClientHostByUid(uid, false);
		if (clientHost == NULL)
		{
			nlwarning("impulsionUidGatewayClose : can't find CClientHost for uid %u, discards message", uid);
			return;
		}

		onDisconnection(clientHost->clientId());
	}
	// Id impulsion gateway close callback handler
	void impulsionIdGatewayClose( CEntityId& sender, CBitMemStream &bms, TGameCycle gamecycle, uint16 serviceId )
	{
		// retrieve uid
		CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();
		TClientId clientId = frs->EntityToClient.getClientId(sender);
		if (clientId == INVALID_CLIENT)
		{
			nlwarning("impulsionIdGatewayClose : can't find clientId for EID %s, discards message", sender.toString().c_str());
			return;
		}

		onDisconnection(clientId);
	}

	CFEServerRoute *getClientRoute(TClientId cid)
	{
		TRouteMap::iterator it(_Routes.find(cid));

		if (it == _Routes.end())
			return NULL;
		else
			return it->second;
	}

};

/***************************************************/
/** static callback forwarder                     **/
/***************************************************/

// Forwarder to the real method
void cbGwTrDisconnection ( TClientId clientId )
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->onDisconnection(clientId);
}

// Uid impulsion gateway open callback handler
void cbImpulsionUidGatewayOpen(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->impulsionUidGatewayOpen(uid, bms, gameCycle);
}

// Id impulsion gateway open callback handler
void cbImpulsionIdGatewayOpen( CEntityId& sender, CBitMemStream &bms, TGameCycle gameCycle, uint16 serviceId )
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->impulsionIdGatewayOpen(sender, bms, gameCycle, serviceId);
}

// Uid impulsion gateway message callback handler
void cbImpulsionUidGatewayMessage(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->impulsionUidGatewayMessage(uid, bms, gameCycle);
}

// Id impulsion gateway message callback handler
void cbImpulsionIdGatewayMessage( CEntityId& sender, CBitMemStream &bms, TGameCycle gameCycle, uint16 serviceId )
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->impulsionIdGatewayMessage(sender, bms, gameCycle, serviceId);
}

// Uid impulsion gateway close callback handler
void cbImpulsionUidGatewayClose(uint32 uid, NLMISC::CBitMemStream &bms, NLMISC::TGameCycle gameCycle)
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->impulsionUidGatewayClose(uid, bms, gameCycle);
}

// Id impulsion gateway open callback handler
void cbImpulsionIdGatewayClose( CEntityId& sender, CBitMemStream &bms, TGameCycle gameCycle, uint16 serviceId )
{
	if (CGatewayFEServerTransport::OpenTransport() != NULL)
		CGatewayFEServerTransport::OpenTransport()->impulsionIdGatewayClose(sender, bms, gameCycle, serviceId);
}


// register this class in the transport factory
NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayFEServerTransport, std::string, string(FE_SERVER_CLASS_NAME));


////////////////////////////////////////////////////////
// Security plug-in for the gateway
////////////////////////////////////////////////////////

#define FE_SECURITY_CLASS_NAME "FESecurity"

class CFESecurity : public CGatewaySecurity
{
public:
	static CFESecurity		*Instance;

	CFESecurity(const TCtorParam params)
		: CGatewaySecurity (params)
	{
		nlassert(Instance == NULL);
		Instance = this;
	}

	~CFESecurity() NL_OVERRIDE
	{
		Instance = NULL;
	}

	/** A new proxy is available, the security plug-in can add security data */
	void onNewProxy(IModuleProxy *proxy) NL_OVERRIDE
	{
		if (proxy->getGatewayRoute() == NULL)
			return;

		CFEServerRoute *route = dynamic_cast<CFEServerRoute *>(proxy->getGatewayRoute());
		if (route != NULL)
		{
			// retrieve client information
			TClientId cid = route->ClientId;
			CFeReceiveSub *frs = CFrontEndService::instance()->receiveSub();

			CClientHost *ch = frs->getClientHost(cid);
			TClientInfo *ci = new TClientInfo(TSecurityData::TCtorParam(rmst_client_info));

			// store client information in a security bloc
			if( ch == NULL)
			{
				ci->UserPriv = "";
				ci->ExtendedPriv = "";
				ci->ClientEid = CEntityId::Unknown;
				ci->UserId = 0xffffffff;
			}
			else
			{
				ci->UserPriv = ch->UserPriv;
				ci->ExtendedPriv = ch->UserExtended;
				ci->ClientEid = ch->eId();
				ci->UserId = ch->Uid;
			}

			/// store the security data in the proxy, in place of any the client sent: the services trust what a proxy
			/// carries
			replaceAllSecurityDatas(proxy, ci);
		}
	}

	void onNewSecurityData(CGatewayRoute *from, IModuleProxy *proxy, TSecurityData *firstSecurityData) NL_OVERRIDE
	{
		CFEServerRoute *route = dynamic_cast<CFEServerRoute *>(from);
		if (route != NULL)
		{
			// don't accept any security info coming from the FE route !
			nlwarning("HACK : FE Gateway transport : security module : client %u try to send module security data !", route->ClientId);
			delete firstSecurityData;
		}
		else
		{
			// ok, let the security data pass
			replaceAllSecurityDatas(proxy, firstSecurityData);
		}
	}


	void onDelete() NL_OVERRIDE
	{
		// remove any client security info
		vector<IModuleProxy*> proxies;
		_Gateway->getModuleProxyList(proxies);

		for (uint i=0; i<proxies.size(); ++i)
		{
			IModuleProxy *proxy = proxies[i];
			if (proxy->getGatewayRoute() != NULL)
			{
				CFEServerRoute *route = dynamic_cast<CFEServerRoute *>(proxy->getGatewayRoute());
				if (route != NULL)
				{
					// remove security data for client
					removeSecurityData(proxy, rmst_client_info);
				}
			}	
		}
	}


	void entityIdChanged(CClientHost *clienthost)
	{
		vector<IModuleProxy*>	proxies;
		_Gateway->getModuleProxyList(proxies);

		if (CGatewayFEServerTransport::OpenTransport() == NULL)
			return;

		CFEServerRoute *route = CGatewayFEServerTransport::OpenTransport()->getClientRoute(clienthost->clientId());

		if (route == NULL)
			return;

		for (uint i=0; i<proxies.size(); ++i)
		{
			CModuleProxy *proxy = static_cast<CModuleProxy*>(proxies[i]);

			if (proxy->getGatewayRoute() == route)
			{
				// we have found a proxy for this client
				TClientInfo *ci = new TClientInfo(TSecurityData::TCtorParam(rmst_client_info));

				// store client information in a security bloc
				ci->UserPriv = clienthost->UserPriv;
				ci->ExtendedPriv = clienthost->UserExtended;
				ci->ClientEid = clienthost->eId();
				ci->UserId = clienthost->Uid;

				/// store the security data in the proxy
				setSecurityData(proxy, ci);
			}
		}
	}
};

NLMISC_REGISTER_OBJECT(CGatewaySecurity, CFESecurity, std::string, string(FE_SECURITY_CLASS_NAME));

CFESecurity		*CFESecurity::Instance = NULL;


void cbEntityIdChanged(CClientHost *clienthost)
{
	if (CFESecurity::Instance == NULL)
		return;

	CFESecurity::Instance->entityIdChanged(clienthost);
}
