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
#include "nel/net/module_gateway.h"
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_message.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"
#include "nel/net/net_log.h"


using namespace std;
using namespace NLMISC;



namespace NLNET
{
	typedef uint8	TL5TransportId;

	struct TTransportDesc
	{
		TL5TransportId		TransportId;
		string				SubNetName;
		bool				InResponse;

		void serial(NLMISC::IStream &s)
		{
			s.serial(TransportId);
			s.serial(SubNetName);
			s.serial(InResponse);
		}
	};

	/** the specialized route for l5 transport */
	class CL5Route : public CGatewayRoute
	{
	public:
		/// the service ID of the outbound service
		TServiceId		ServiceId;

		/// The transport ID at the outbound
		TL5TransportId	ForeignTransportId;


		CL5Route(IGatewayTransport *transport)
			: CGatewayRoute(transport)
		{
		}

		void sendMessage(const CMessage &message) const;
	};

	/** Utility class that generate 8bits unique transport id.
	 *	The total L5 transport instance is limited to 256.
	 *	This really should be enough or you have a problem in
	 *	your design !
	 *	The allocator keep released ID as long as possible
	 *	and reallocated them only when all other ids
	 *	have been used/allocated.
	 */
	class CTransportIdAllocator
	{
		NLMISC_SAFE_SINGLETON_DECL(CTransportIdAllocator);
	private:
		deque<TL5TransportId>	_FreeIds;
		set<TL5TransportId>		_UsedIds;

		CTransportIdAllocator()
		{
			// fill the list of free ids;
			for (TL5TransportId i=0; i<TL5TransportId(UINT64_CONSTANT(0xffffffffffffffff)); ++i)
			{
				_FreeIds.push_back(i);
			}
		}

	public:
		TL5TransportId allocateId()
		{
			nlassert(!_FreeIds.empty());
			TL5TransportId ret = _FreeIds.front();
			_FreeIds.pop_front();
			nlassert(_UsedIds.find(ret) == _UsedIds.end());
			_UsedIds.insert(ret);
			return ret;
		}

		void releaseId(TL5TransportId id)
		{
			nlassert(_UsedIds.find(id) != _UsedIds.end());
			_UsedIds.erase(id);
			_FreeIds.push_back(id);
		}
	};

	NLMISC_SAFE_SINGLETON_IMPL(CTransportIdAllocator);

#define LAYER5_CLASS_NAME "L5Transport"

	/** Gateway transport using layer 5 */
	class CGatewayL5Transport : public IGatewayTransport
	{
		friend class CL5Route;
	public:
		/// This transport ID
		TL5TransportId	_TransportId;

		/// current open status
		bool			_Open;
		/// Subnet name
		string			_SubNetName;

		typedef std::map<TServiceId, CL5Route*>	TRouteMap;
		/// The table that keep track of all routes
		TRouteMap	_Routes;

		typedef std::map<TL5TransportId, CGatewayL5Transport*>	TTransportDispatcher;
		/// Global index of transport use to dispatch received message
		static TTransportDispatcher	_TransportDispatcher;

		/// Constructor
		CGatewayL5Transport(const IGatewayTransport::TCtorParam &param)
			: IGatewayTransport(param),
			_Open(false)
		{
			// allocate a transport unique ID
			_TransportId = CTransportIdAllocator::getInstance().allocateId();

			// store the transport in the dispatcher
			_TransportDispatcher.insert(make_pair(_TransportId, this));

			// L5 transport is always peer invisible
			PeerInvisible = true;
		}

		~CGatewayL5Transport()
		{
			if (_Open)
			{
				// the transport is still open, close it before destruction
				close();
			}

			// remove the transport from the dispatcher
			nlassert(_TransportDispatcher.find(_TransportId) != _TransportDispatcher.end());
			_TransportDispatcher.erase(_TransportId);
			// release the unique id
			CTransportIdAllocator::getInstance().releaseId(_TransportId);
		}

		const std::string &getClassName() const
		{
			static string className(LAYER5_CLASS_NAME);
			return className;
		}

		virtual void update()
		{
		}

		virtual uint32 getRouteCount() const
		{
			return (uint32)_Routes.size();
		}

		void dump(NLMISC::CLog &log) const
		{
			IModuleManager &mm = IModuleManager::getInstance();
			log.displayNL("  NeL Net layer 5 transport");
			if (!_Open)
			{
				log.displayNL("  The transport is currently closed.");
			}
			else
			{
				log.displayNL("  The transport is open and support %u routes :",
					_Routes.size());
				TRouteMap::const_iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					TServiceId	sid = first->first;
					CL5Route *route = first->second;
					log.displayNL("    + route to service %hu('%s'), %u entries in the proxy translation table :",
						sid.get(),
						CUnifiedNetwork::getInstance()->getServiceName(sid).c_str(),
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
				string subNetName;

				/// look for an optional sub network name
				const TParsedCommandLine *netName = command.SubParams[0]->getParam("SubNet");
				if (netName != NULL)
				{
					subNetName = netName->ParamValue;
				}

				open(subNetName);
			}
			else if (commandName == "close")
			{
				close();
			}
			else
				return false;

			return true;
		}


		/// Open the server by establishing route with all known services
		void open(const std::string &subNetName) throw (ETransportError)
		{
			H_AUTO(L5_open);

			static TUnifiedCallbackItem L5TransportCallback[] =
			{
				{"GW_L5_MSG",	CGatewayL5Transport::cbDispatchL5Message	},
				{"GW_L5_ADDTP",	CGatewayL5Transport::cbL5AddTransport		},
				{"GW_L5_REMTP",	CGatewayL5Transport::cbL5RemoveTransport	},
			};

			if (_Open == true)
				throw ETransportError("Transport already open");

			_SubNetName = subNetName;


			CUnifiedNetwork *un = CUnifiedNetwork::getInstance();

			static bool callbackRegistered = false;

			if (!callbackRegistered)
			{
				LNETL6_DEBUG("LNETL6: L5 transport open : registering callbacks");
				// set the service con/disconnect callback
				un->setServiceUpCallback("*", CGatewayL5Transport::cbOnServiceUp);
				un->setServiceDownCallback("*", CGatewayL5Transport::cbOnServiceDown);
				// set the message callback
				un->addCallbackArray(L5TransportCallback, sizeof(L5TransportCallback) / sizeof(TUnifiedCallbackItem));

				callbackRegistered = true;
			}


			// create route and open route for each existing service
			const vector<TServiceId>	&connList = un->getConnectionList();

			set<TServiceId>	uniqueService(connList.begin(), connList.end());

			while (!uniqueService.empty())
			{
				TServiceId sid = *(uniqueService.begin());
				uniqueService.erase(uniqueService.begin());

				if ( un->isConnectionConnected(sid))
				{
					// send transport descriptor to other service
					onServiceUp(un->getServiceName(sid), sid);
				}
				else
				{
					// the Connection is not established right now. We wait for the ServiceUp callback
				}

			}

			_Open = true;
		}

		/// Close the server, this will close all route
		void close()
		{
			H_AUTO(L5_close);

			if (_Open == false)
				throw ETransportError("closeServer : The server is not open");

			// close all client connections
			while (!_Routes.empty())
			{
				CL5Route *route = _Routes.begin()->second;
				TServiceId sid = route->ServiceId;
				onServiceDown(CUnifiedNetwork::getInstance()->getServiceName(sid), sid);
			}

			_Open = false;
		}


		/***************************************************/
		/** Event management                              **/
		/***************************************************/

		void onServiceUp(const std::string &serviceName, TServiceId sid)
		{
			H_AUTO(L5_onServiceUp);

			LNETL6_DEBUG("LNETL6: L5 transport onServiceUp('%s')", serviceName.c_str());
			// send the transport descriptor to the new service
			TTransportDesc	desc;
			desc.SubNetName = _SubNetName;
			desc.TransportId = _TransportId;
			desc.InResponse = false;

			CMessage msg("GW_L5_ADDTP");
			msg.serial(desc);

			CUnifiedNetwork::getInstance()->send(sid, msg);

			// the route will be created by receiving this message
		}

		void onServiceDown(const std::string &/* serviceName */, TServiceId sid)
		{
			H_AUTO(L5_onServicedown);

			LNETL6_DEBUG("LNETL6: L5 transport onServiceDown('%hu')", sid.get());
			// retrieve the route
			TRouteMap::iterator it(_Routes.find(sid));
			if (it == _Routes.end())
			{
				nlinfo("Transport L5 : service down, can't find a route for the service");
				return;
			}
			CL5Route *route = it->second;

			// warn the gateway
			_Gateway->onRouteRemoved(route);

			// release the route
			_Routes.erase(it);
			delete route;
		}

		// Called to dispatch an incoming message to the gateway
		void onDispatchMessage(const CMessage &msgin, TServiceId sid)
		{
			H_AUTO(L5_onDispatchMessage);

			LNETL6_DEBUG("LNETL6: L5 transport onDispatchMessage from service %hu", sid.get());
			/// retrieve the route for dispatching
			TRouteMap::iterator it(_Routes.find(sid));
			if (it == _Routes.end())
			{
				nlwarning("Gateway '%s' : Can't find route for service %hu for dispatching, message is discarded",
					_Gateway->getGatewayName().c_str(),
					sid.get());
				return;
			}

			// read the message size
			uint32 msgLen;
			nlRead(msgin, serial, msgLen);

			// lock the sub message
			msgin.lockSubMessage(msgLen);

			_Gateway->onReceiveMessage(it->second, msgin);

			// unlock the sub message
			msgin.unlockSubMessage();
		}

		void onAddTransport(TServiceId sid, TTransportDesc &desc)
		{
			H_AUTO(L5_onAddTransport);

			LNETL6_DEBUG("LNETL6: L5 transport onAddTransport from service %hu", sid.get());
			// we need to create a route for this transport
			// create a new route and send the route open message

			if (_Routes.find(sid) != _Routes.end())
			{
				LNETL6_DEBUG("LNETL6: L5 transport onAddTransport a route for this service alredy exist");
				return;
			}

			CL5Route *route = new CL5Route(this);

			route->ServiceId = sid;
			route->ForeignTransportId = desc.TransportId;

			// store the route infos
			_Routes.insert(make_pair(sid, route));

			// notify the gateway about the new route
			_Gateway->onRouteAdded(route);

			if (desc.InResponse == false)
			{
				// we need to send back this transport info to this service
				TTransportDesc desc;
				desc.InResponse = true;
				desc.SubNetName = _SubNetName;
				desc.TransportId = _TransportId;

				CMessage msg("GW_L5_ADDTP");
				msg.serial(desc);

				CUnifiedNetwork::getInstance()->send(sid, msg);
			}
		}

		void onRemoveTransport(TServiceId sid, TTransportDesc &desc)
		{
			H_AUTO(L5_onRemoveTransport);

			LNETL6_DEBUG("LNETL6: L5 transport onRemoveTransport from service %hu", sid.get());
			// Remove the route
			TRouteMap::iterator it(_Routes.find(sid));
			if (it == _Routes.end())
			{
				nlwarning("onRemoveTransport : can't find a route to the transport %hu on service %u",
					desc.TransportId,
					sid.get());
				return;
			}

			CL5Route *route = it->second;

			// notify the gateway about the removed route
			_Gateway->onRouteRemoved(route);

			// erase the route info and delete the route
			_Routes.erase(it);
			delete route;
		}

		/***************************************************/
		/** static callback forwarder                     **/
		/***************************************************/
		/// callback from layer 5

		static void cbL5AddTransport(CMessage &msgin, const std::string &/* serviceName */, TServiceId sid)
		{
			LNETL6_DEBUG("LNETL6: L5 transport cbL5AddTransport from service %hu", sid.get());
			// Receive a transport descriptor from another service, create
			// a route for it

			TTransportDesc	desc;

			msgin.serial(desc);

			// for each existing transport here, check if they are in the
			// same sub net, if so, callback them for route creation

			TTransportDispatcher::iterator first(_TransportDispatcher.begin()), last(_TransportDispatcher.end());
			for (; first != last; ++first)
			{
				CGatewayL5Transport *transport = first->second;
				if (transport->_Open
					&& transport->_SubNetName == desc.SubNetName
					&& (sid != IService::getInstance()->getServiceId()
						|| desc.TransportId != transport->_TransportId))
				{
					// this one is on the same subnet
					transport->onAddTransport(sid, desc);
				}
			}
		}

		static void cbL5RemoveTransport(CMessage &msgin, const std::string &/* serviceName */, TServiceId sid)
		{
			LNETL6_DEBUG("LNETL6: L5 transport cbL5RemoveTransport from service %hu", sid.get());
			// Receive a transport descriptor from another service, delete
			// the route for it

			TTransportDesc	desc;

			msgin.serial(desc);

			// for each existing transport here, check if they are in the
			// same sub net, if so, callback them for route creation

			TTransportDispatcher::iterator first(_TransportDispatcher.begin()), last(_TransportDispatcher.end());
			for (; first != last; ++first)
			{
				CGatewayL5Transport *transport = first->second;
				if (transport->_Open
					&& transport->_SubNetName == desc.SubNetName
					&& desc.TransportId != transport->_TransportId)
				{
					// this one is on the same subnet
					transport->onRemoveTransport(sid, desc);
				}
			}
		}

		static void cbDispatchL5Message (CMessage &msgin, const std::string &serviceName, TServiceId sid)
		{
			LNETL6_DEBUG("LNETL6: L5 transport cbDispatch called, receiving from %s", serviceName.c_str());
			// dispatch the message to the route associated with the service
			// the first info in the message is the transport id
			TL5TransportId transportId;
			msgin.serial(transportId);

			// look for a corresponding transport
			TTransportDispatcher::iterator it(_TransportDispatcher.find(transportId));
			if (it == _TransportDispatcher.end())
			{
				nlwarning("ReceiveL5Message, can't find transport id %u for dispatching, message is discarded",
					transportId);
				return;
			}

			CGatewayL5Transport *transport = it->second;
			transport->onDispatchMessage(msgin, sid);
		}

		static  void cbOnServiceUp (const std::string &serviceName, TServiceId sid, void * /* arg */)
		{
			LNETL6_DEBUG("LNETL6: L5 transport cbOnServiceUp called, service up for %s", serviceName.c_str());
			// callback all open transport about the new service
			TTransportDispatcher::iterator first(_TransportDispatcher.begin()), last(_TransportDispatcher.end());
			for (; first != last; ++first)
			{
				CGatewayL5Transport *transport = first->second;

				if (transport->_Open)
					transport->onServiceUp(serviceName, sid);
			}
		}

		static  void cbOnServiceDown (const std::string &serviceName, TServiceId sid, void * /* arg */)
		{
			LNETL6_DEBUG("LNETL6: L5 transport cbOnServicedown called, service down for %s", serviceName.c_str());
			// callback all open transport about the removed service
			TTransportDispatcher::iterator first(_TransportDispatcher.begin()), last(_TransportDispatcher.end());
			for (; first != last; ++first)
			{
				CGatewayL5Transport *transport = first->second;

				if (transport->_Open)
					transport->onServiceDown(serviceName, sid);
			}
		}

	};

	CGatewayL5Transport::TTransportDispatcher	CGatewayL5Transport::_TransportDispatcher;

	// register this class in the transport factory
	NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayL5Transport, std::string, string(LAYER5_CLASS_NAME));

	void CL5Route::sendMessage(const CMessage &message) const
	{
		NLNET_AUTO_DELTE_ASSERT;
		H_AUTO(L5Route_sendMessage);

		CGatewayL5Transport *trpt = static_cast<CGatewayL5Transport*>(_Transport);

		// create a transport message
		CMessage wrapper("GW_L5_MSG");
		// serial the transport identifier
		wrapper.serial(trpt->_TransportId);;

		// insert the message in the wrapper
		nlWrite(wrapper, serialMessage, message);
		// send the message
		CUnifiedNetwork::getInstance()->send(ServiceId, wrapper);
	}



	void forceGatewayL5TransportLink()
	{
	}

} // namespace NLNET
