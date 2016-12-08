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

#include "stdnet.h"
#include "nel/misc/time_nl.h"
#include "nel/net/module_gateway.h"
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_message.h"
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"

using namespace std;
using namespace NLMISC;



namespace NLNET
{

	// keep alive delay in seconds of inactivity
	// NB : it is useless to set it at a value less than 100" because
	// according to RFC 1122 (Requirements for Internet Hosts),
	// the TCP transmission time out is a least of 100" before
	// closing a connection without acknowledge.
	// That means modules seens from a dead connection will only be
	// removed after little more than 100".
	const uint32	KEEP_ALIVE_DELAY = 120;

	/** the specialized route for server transport */
	class CL3ServerRoute : public CGatewayRoute
	{
	public:
		/// The id of the socket in the server
		TSockId			SockId;

		/// Time stamp of last message received/emitted
		mutable uint32	LastCommTime;


		CL3ServerRoute(IGatewayTransport *transport)
			: CGatewayRoute(transport),
			LastCommTime(CTime::getSecondsSince1970())
		{
		}

		void sendMessage(const CMessage &message) const;
	};

#define LAYER3_SERVER_CLASS_NAME "L3Server"

	/** Gateway transport using layer 3 server */
	class CGatewayL3ServerTransport : public IGatewayTransport
	{
		friend class CL3ServerRoute;
	public:
		/// The callback server that receive connection and dispatch message
		auto_ptr<CCallbackServer>			_CallbackServer;

		/// A static mapper to retrieve transport from the CCallbackServer pointer
		typedef map<CCallbackNetBase*, CGatewayL3ServerTransport*>	TDispatcherIndex;
		static TDispatcherIndex				_DispatcherIndex;

		/// The table that keep track of all routes
		typedef std::map<TSockId, CL3ServerRoute*>	TRouteMap;
		TRouteMap	_Routes;


		/// Constructor
		CGatewayL3ServerTransport(const IGatewayTransport::TCtorParam &param)
			: IGatewayTransport(param)
		{
		}

		~CGatewayL3ServerTransport()
		{
			if (_CallbackServer.get() != NULL)
			{
				// the transport is still open, close it before destruction
				closeServer();
			}
		}

		const std::string &getClassName() const
		{
			static string className(LAYER3_SERVER_CLASS_NAME);
			return className;
		}

		virtual void update()
		{
			H_AUTO(L3S_update);
			// update the callback server
			if (_CallbackServer.get() != NULL)
				_CallbackServer->update2(100, 0);

			uint32 now = CTime::getSecondsSince1970();
			// check each connected client for keep alive
			TRouteMap::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CL3ServerRoute *route = first->second;

				if (now - route->LastCommTime > KEEP_ALIVE_DELAY)
				{
					nldebug("NETL6:L3Server: sending KeepAlive message");
					// send a keep alive message
					CMessage keepAlive("KA");
					route->sendMessage(keepAlive);

					// update the last event time
					route->LastCommTime = CTime::getSecondsSince1970();
				}

				// force a flush of the connection
				_CallbackServer->flush(route->SockId);
			}


		}

		virtual uint32 getRouteCount() const
		{
			return (uint32)_Routes.size();
		}

		void dump(NLMISC::CLog &log) const
		{
			IModuleManager &mm = IModuleManager::getInstance();
			log.displayNL("  NeL Net layer 3 transport, SERVER mode");
			if (_CallbackServer.get() == NULL)
			{
				log.displayNL("  The server is currently closed.");
			}
			else
			{
				log.displayNL("  The server is open on '%s' and support %u routes :",
					_CallbackServer->listenAddress().asString().c_str(),
					_Routes.size());
				TRouteMap::const_iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					TSockId	sockId = first->first;
					CL3ServerRoute *route = first->second;
					log.displayNL("    + route to '%s', %u entries in the proxy translation table :",
						sockId->getTcpSock()->remoteAddr().asString().c_str(),
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

				log.displayNL("  Dumping send buffers states");
				_CallbackServer->displaySendQueueStat(&log);
				log.displayNL("  Dumping receive buffers states");
				_CallbackServer->displayReceiveQueueStat(&log);
			}
		}

		void onCommand(const CMessage &/* command */) throw (EInvalidCommand)
		{
			// nothing done for now
			throw EInvalidCommand();
		}
		/// The gateway send a textual command to the transport
		bool onCommand(const TParsedCommandLine &command) throw (EInvalidCommand)
		{
			if (command.SubParams.size() < 1)
				throw  EInvalidCommand();

			const std::string &commandName = command.SubParams[0]->ParamName;
			if (commandName == "open")
			{
				const TParsedCommandLine *portParam = command.getParam("port");
				if (portParam == NULL)
					throw EInvalidCommand();

				uint16 port;
				fromString(portParam->ParamValue, port);

				openServer(port);
			}
			else if (commandName == "close")
			{
				closeServer();
			}
			else
				return false;

			return true;
		}

		/// Open the server by starting listing for incoming connection on the specified port
		void openServer(uint16 port) throw (ETransportError)
		{
			if (_CallbackServer.get() != NULL)
				throw ETransportError("openServer : The server is already open");

			// create a new callback server
			auto_ptr<CCallbackServer> cbs = auto_ptr<CCallbackServer> (new CCallbackServer());

			// register the callbacks
			cbs->setConnectionCallback(cbConnection, static_cast<IGatewayTransport*>(this));
			cbs->setDisconnectionCallback(cbDisconnection, static_cast<IGatewayTransport*>(this));
			cbs->setDefaultCallback(cbDispatchMessage);

			// open the server
			cbs->init(port);

			_CallbackServer = cbs;

			// register it in the dispatcher
			_DispatcherIndex.insert(make_pair(_CallbackServer.get(), this));
		}

		/// Close the server, this will close the listing socket and any active connection
		void closeServer()
		{
			if (_CallbackServer.get() == NULL)
				throw ETransportError("closeServer : The server is not open");

			// close all client connections
			while (!_Routes.empty())
			{
				CL3ServerRoute *route = _Routes.begin()->second;

				// close the connection
				_CallbackServer->disconnect(route->SockId);
				// callback the gateway
				_Gateway->onRouteRemoved(route);

				// delete route and cleanup
				_Routes.erase(_Routes.begin());
				delete route;
			}

			// Remove the dispatcher info
			_DispatcherIndex.erase(_CallbackServer.get());

			// release the callback server
			delete _CallbackServer.release();
		}


		/***************************************************/
		/** Event management                              **/
		/***************************************************/

		// handle the connection of a new client on the server
		void onConnection ( TSockId from)
		{
			H_AUTO(L3S_onConnection);
			nlassert(_Routes.find(from) == _Routes.end());

			// Create a new route for this connection
			CL3ServerRoute* route = new CL3ServerRoute(this);
			route->SockId = from;

			// update the last event time
			route->LastCommTime = CTime::getSecondsSince1970();

			// store the route information
			_Routes.insert(make_pair(from, route));

			// callback the gateway
			_Gateway->onRouteAdded(route);
		}

		// handle the deconnection of a new client on the server
		void onDisconnection ( TSockId from)
		{
			H_AUTO(L3S_onDisconnection);
			TRouteMap::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			// callback the gateway that this route is no more
			_Gateway->onRouteRemoved(it->second);

			// delete the route
			CL3ServerRoute *route = it->second;
			_Routes.erase(it);
			delete route;
		}

		// Called to dispatch an incoming message to the gateway
		void onDispatchMessage(const CMessage &msgin, TSockId from, CCallbackNetBase &/* netbase */)
		{
			H_AUTO(L3S_onDispatchMessage);
			TRouteMap::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			// update the last event time
			it->second->LastCommTime = CTime::getSecondsSince1970();

			if (msgin.getName() == "KA")
			{
				// this is just a server prob, ignore it
				return;
			}

			_Gateway->onReceiveMessage(it->second, msgin);

		}


		/***************************************************/
		/** static callback forwarder                     **/
		/***************************************************/
		// Forwarder to the real method
		static void cbConnection ( TSockId from, void *arg )
		{
			nlassert(arg != NULL);
			CGatewayL3ServerTransport *transport = dynamic_cast<CGatewayL3ServerTransport *>(static_cast<IGatewayTransport*>(arg));
			nlassert(transport != NULL);

			transport->onConnection(from);
		}

		// Forwarder to the real method
		static void cbDisconnection ( TSockId from, void *arg )
		{
			nlassert(arg != NULL);
			CGatewayL3ServerTransport *transport = dynamic_cast<CGatewayL3ServerTransport *>(static_cast<IGatewayTransport*>(arg));
			nlassert(transport != NULL);

			transport->onDisconnection(from);
		}

		// Forward to the real method, do the dispatching to the correct CGatewayL3ServerTransport instance
		static void cbDispatchMessage (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
		{
			// retrieve the transport instance
			TDispatcherIndex::iterator it(_DispatcherIndex.find(&netbase));
			nlassert(it != _DispatcherIndex.end());

			// forward the call
			it->second->onDispatchMessage(msgin, from, netbase);
		}

	};

	CGatewayL3ServerTransport::TDispatcherIndex	CGatewayL3ServerTransport::_DispatcherIndex;

	// register this class in the transport factory
	NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayL3ServerTransport, std::string, string(LAYER3_SERVER_CLASS_NAME));

	void CL3ServerRoute::sendMessage(const CMessage &message) const
	{
		H_AUTO(L3SRoute_sendMessage);
		NLNET_AUTO_DELTE_ASSERT;

		CGatewayL3ServerTransport *trpt = static_cast<CGatewayL3ServerTransport*>(_Transport);

		// send the message
		trpt->_CallbackServer->send(message, SockId);

		// update the last time
		LastCommTime = CTime::getSecondsSince1970();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/// Layer 3 client transport
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	class CL3ClientRoute : public CGatewayRoute
	{
	public:
		/// The server address for this route
		CInetAddress				ServerAddr;
		/// The Client callback
		mutable CCallbackClient		CallbackClient;
		/// Time stamp of last message received/emitted
		mutable uint32				LastCommTime;

		/// The last time we try to reconnect (in case of disconnection)
		uint32						LastConnectionRetry;

		// conn id
		uint32						ConnId;

		CL3ClientRoute(IGatewayTransport *transport, CInetAddress serverAddr,uint32 connId)
			: CGatewayRoute(transport),
			ServerAddr(serverAddr),
			LastCommTime(CTime::getSecondsSince1970()),
			LastConnectionRetry(0),
			ConnId(connId)
		{
		}

		void sendMessage(const CMessage &message) const
		{
			NLNET_AUTO_DELTE_ASSERT;
			H_AUTO(L3CRoute_sendMessage);
			if (CallbackClient.connected())
			{
				// update the last comme time
				LastCommTime = CTime::getSecondsSince1970();

				CallbackClient.send(message);
			}
		}
	};

#define LAYER3_CLIENT_CLASS_NAME "L3Client"

	/** Gateway transport using layer 3 client */
	class CGatewayL3ClientTransport : public IGatewayTransport
	{
		friend class CL3ClientRoute;
	public:
		/// A static mapper to retrieve transport from the CCallbackServer pointer
		typedef map<CCallbackNetBase*, CGatewayL3ClientTransport*>	TDispatcherIndex;
		static TDispatcherIndex				_DispatcherIndex;

		/// Storage for active connection
		typedef map<TSockId, CL3ClientRoute*>	TClientRoutes;
		TClientRoutes			_Routes;

		/// Indexed storage of active connection (used for stable connId)
		/// a NULL TSockeId mean a free connection slot.
		typedef vector<TSockId>		TClientRouteIds;
		TClientRouteIds			_RouteIds;
		/// A list of free slot ready for use
		typedef vector<TClientRouteIds::difference_type>	TFreeRouteIds;
		TFreeRouteIds			_FreeRoutesIds;

		/// the route to delete outside of the update loop
		list<CL3ClientRoute*>	_RouteToRemove;

		/// Retry interval for reconnection
		uint32					_RetryInterval;

		enum
		{
			/// Default time interval (in seconds) between to reconnection attempts
			RETRY_INTERVAL =  5,
			/// A minimum value in case or configuration error
			MIN_RETRY_INTERVAL = 1,
		};

		/// Constructor
		CGatewayL3ClientTransport(const IGatewayTransport::TCtorParam &param)
			: IGatewayTransport(param),
			_RetryInterval(RETRY_INTERVAL)
		{
		}

		~CGatewayL3ClientTransport()
		{
			deletePendingRoute();

			// close all open connection
			for (uint i=0; i<_RouteIds.size(); ++i)
			{
				if (_RouteIds[i] != NULL)
				{
					// close this open connection
					close(i);
				}
			}
		}

		void deletePendingRoute()
		{
			H_AUTO(L3C_deletePendingRoute);
			// delete any route pending
			while (!_RouteToRemove.empty())
			{
				CL3ClientRoute *route = _RouteToRemove.front();
				_DispatcherIndex.erase(&(route->CallbackClient));
				_Routes.erase(route->CallbackClient.getSockId());

				_RouteIds[route->ConnId] = NULL;
				_FreeRoutesIds.push_back(route->ConnId);
				delete route;
				_RouteToRemove.pop_front();
			}
		}

		const std::string &getClassName() const
		{
			static string className(LAYER3_CLIENT_CLASS_NAME);
			return className;
		}

		virtual void update()
		{
			H_AUTO(L3C_update);
			// delete any route pending
			deletePendingRoute();

			uint32 now = CTime::getSecondsSince1970();
			// update the client connection
			TClientRoutes::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CL3ClientRoute *route = first->second;

				if (!route->CallbackClient.connected())
				{
					// this route is not connected, try a reconnect ?
					if (route->LastConnectionRetry + _RetryInterval < now)
					{
						route->LastConnectionRetry = now;
						try
						{
							nldebug("Connecting to %s...", route->ServerAddr.asString().c_str());
							route->CallbackClient.connect(route->ServerAddr);
							nldebug("Connected to %s", route->ServerAddr.asString().c_str());
							_Gateway->onRouteAdded(route);
						}
						catch(...)
						{
							nlinfo("Server %s still not available for connection", route->ServerAddr.asString().c_str());
						}
					}
				}
				else
				{
					route->CallbackClient.update2(100, 0);

					// check dead connection. For client, we use a little longer timer to
					// avoid cross checking of client and server. If server is alive, then we receive
					// the server keep alive packet a little before we need to send the client one, thus
					// reseting the keep alive timer.
					if (now - route->LastCommTime > (KEEP_ALIVE_DELAY+5))
					{
						nldebug("NETL6:L3Client: sending KeepAlive message");

						// send a keep alive message
						CMessage keepAlive("KA");

						route->sendMessage(keepAlive);
					}

					// force a flush of the connection
					route->CallbackClient.flush();
				}
			}
		}

		virtual uint32 getRouteCount() const
		{
			return (uint32)_Routes.size();
		}

		void dump(NLMISC::CLog &log) const
		{
			IModuleManager &mm = IModuleManager::getInstance();
			log.displayNL("  NeL Net layer 3 transport, CLIENT mode");

			log.displayNL("  There are actually %u active route :", _Routes.size());

			TClientRoutes::const_iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CL3ClientRoute *route = first->second;
				log.displayNL("    + route to '%s', %s, %u entries in the proxy translation table :",
					route->ServerAddr.asString().c_str(),
					route->CallbackClient.connected() ? "connected" : "NOT CONNECTED",
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

				log.displayNL("     Dumping send buffer state");
				route->CallbackClient.displaySendQueueStat(&log);
				log.displayNL("     Dumping receive buffer state");
				route->CallbackClient.displayReceiveQueueStat(&log);
			}
		}

		void onCommand(const CMessage &/* command */) throw (EInvalidCommand)
		{
			// nothing done for now
			throw EInvalidCommand();
		}
		/// The gateway send a textual command to the transport
		bool onCommand(const TParsedCommandLine &command) throw (EInvalidCommand)
		{
			if (command.SubParams.size() < 1)
				throw  EInvalidCommand();

			const std::string &commandName = command.SubParams[0]->ParamName;
			if (commandName == "connect")
			{
				const TParsedCommandLine *addrParam = command.getParam("addr");
				if (addrParam == NULL)
					throw EInvalidCommand();

				CInetAddress addr(addrParam->ParamValue);

				connect(addr);
			}
			else if (commandName == "close")
			{
				const TParsedCommandLine *conIdParam= command.getParam("connId");
				if (conIdParam == NULL)
					throw EInvalidCommand();

				uint32	connId;
				fromString(conIdParam->ParamValue, connId);

				close(connId);
			}
			else if (commandName == "retryInterval")
			{
				uint32 interval;
				fromString(command.SubParams[0]->ParamValue, interval);
				_RetryInterval = std::max(uint32(MIN_RETRY_INTERVAL), interval);

				nldebug("CGatewayL3ClientTransport : setting retry interval to %u", _RetryInterval);
			}
			else
				return false;

			return true;

		}

		/// connect to a server
		void connect(CInetAddress &addr)
		{
			H_AUTO(L3C_connect);
			uint32 connId;

			// affect a connection id
			if (_FreeRoutesIds.empty())
			{
				connId = (uint32)_RouteIds.size();
				_RouteIds.push_back(InvalidSockId);
			}
			else
			{
				connId = (uint32)_FreeRoutesIds.back();
				_FreeRoutesIds.pop_back();
			}

			auto_ptr<CL3ClientRoute> route = auto_ptr<CL3ClientRoute>(new CL3ClientRoute(this, addr, connId));

			// set the callbacks
			route->CallbackClient.setDisconnectionCallback(cbDisconnection, static_cast<IGatewayTransport*>(this));
			route->CallbackClient.setDefaultCallback(cbDispatchMessage);

			try
			{
				nldebug("CGatewayL3ClientTransport : Connecting to %s...", addr.asString().c_str());
				route->LastConnectionRetry = CTime::getSecondsSince1970();
				// connect to the server
				route->CallbackClient.connect(addr);
				nldebug("CGatewayL3ClientTransport : Connected to %s with connId %u", addr.asString().c_str(), connId);
			}
			catch (const ESocketConnectionFailed &)
			{
				nlinfo("CGatewayL3ClientTransport : Failed to connect to server %s, retrying in %u seconds", addr.asString().c_str(), _RetryInterval);
			}

			// store the route
			_Routes.insert(make_pair(route->CallbackClient.getSockId(), route.get()));
			_RouteIds[connId] = route->CallbackClient.getSockId();

			// register it in the dispatcher
			_DispatcherIndex.insert(make_pair(&route->CallbackClient, this));

			// release the auto ptr
			CL3ClientRoute *rt = route.release();

			// callback the gateway about the new route
			if (rt->CallbackClient.connected())
				_Gateway->onRouteAdded(rt);
		}

		// handle the connection of a new client on the server
		void close ( uint32 connId)
		{
			H_AUTO(L3C_close);
			// some basic checks on connId
			if (connId >= _RouteIds.size())
			{
				nlwarning("CGatewayL3ClientTransport : Invalid connectionId %u, max is %u", connId, _RouteIds.size()-1);
				return;
			}

			if (_RouteIds[connId] == NULL)
			{
				nlwarning("CGatewayL3ClientTransport : Invalid connectionId %u, the connection is unused now.", connId);
				return;
			}


			deletePendingRoute();

			// retrieve the connection to close
			TClientRoutes::iterator it(_Routes.find(_RouteIds[connId]));
			nlassert(it != _Routes.end());

			CL3ClientRoute *route = it->second;

			nldebug("CGatewayL3ClientTransport : Closing connection %u to %s", connId, route->ServerAddr.asString().c_str());


			if (route->CallbackClient.connected())
			{
				// callback gateway
				_Gateway->onRouteRemoved(route);

				// close the connection
				route->CallbackClient.disconnect();
			}

			// cleanup memory, index ...
			_DispatcherIndex.erase(&(route->CallbackClient));
			_Routes.erase(it);
			delete route;
			_RouteIds[connId] = NULL;
			_FreeRoutesIds.push_back(connId);
		}

		/***************************************************/
		/** Event management                              **/
		/***************************************************/

		// handle the deconnection of a the client from the server
		void onDisconnection ( TSockId from)
		{
			H_AUTO(L3C_onDisconnection);
			// nothing to do, as route as kept persistent and try to reconnect

			TClientRoutes::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			nldebug("CGatewayL3ClientTransport : Disconnection from %s", it->second->ServerAddr.asString().c_str());


			// callback the gateway that this route is no more
			_Gateway->onRouteRemoved(it->second);

			// update the last connection try to 'now'
			it->second->LastConnectionRetry = CTime::getSecondsSince1970();

//			// delete the route
//			CL3ClientRoute *route = it->second;
//
//			_RouteToRemove.push_back(route);
		}

		// Called to dispatch an incoming message to the gateway
		void onDispatchMessage(const CMessage &msgin, TSockId from, CCallbackNetBase &/* netbase */)
		{
			H_AUTO(L3C_onDispatchMessage);
			TClientRoutes::iterator it(_Routes.find(from));
			nlassert(it != _Routes.end());

			// update last comm time
			it->second->LastCommTime = CTime::getSecondsSince1970();

			if (msgin.getName() == "KA")
			{
				// this is just a server prob, ignore it
				return;
			}

			_Gateway->onReceiveMessage(it->second, msgin);
		}


		/***************************************************/
		/** static callback forwarder                     **/
		/***************************************************/

		// Forwarder to the real method
		static void cbDisconnection ( TSockId from, void *arg )
		{
			nlassert(arg != NULL);
			CGatewayL3ClientTransport *transport = dynamic_cast<CGatewayL3ClientTransport *>(static_cast<IGatewayTransport*>(arg));
			nlassert(transport != NULL);

			transport->onDisconnection(from);
		}

		// Forward to the real method, do the dispatching to the correct CGatewayL3ServerTransport instance
		static void cbDispatchMessage (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
		{
			// retrieve the transport instance
			TDispatcherIndex::iterator it(_DispatcherIndex.find(&netbase));
			nlassert(it != _DispatcherIndex.end());

			// forward the call
			it->second->onDispatchMessage(msgin, from, netbase);
		}

	};

	CGatewayL3ClientTransport::TDispatcherIndex	CGatewayL3ClientTransport::_DispatcherIndex;

	// register this class in the transport factory
	NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayL3ClientTransport, std::string, string(LAYER3_CLIENT_CLASS_NAME));


	void forceGatewayTransportLink()
	{
	}

} // namespace NLNET
