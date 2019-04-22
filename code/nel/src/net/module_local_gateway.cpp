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

using namespace std;
using namespace NLMISC;



namespace NLNET
{


	/** A simple gateway that interconnect module locally
	 *	For testing purpose and simple case.
	 */
	class CLocalGateway :
		public CModuleBase,
		public CModuleGateway,
		public CModuleSocket
	{
		// the proxy that represent this gateway
//		TModuleGatewayProxyPtr		_ThisProxy;

		typedef CTwinMap<TModuleProxyPtr, TStringId>	TModuleProxies;
		// The modules proxies
		TModuleProxies		_ModuleProxies;


	public:

		CLocalGateway()
		{

		}

		~CLocalGateway()
		{
			// we need to unplug any plugged module
			while (!_PluggedModules.getAToBMap().empty())
			{
				_PluggedModules.getAToBMap().begin()->second->unplugModule(this);
			}

			// must be done before the other destructors are called
			unregisterSocket();
			unregisterGateway();
		}

		/***********************************************************
		 ** Gateway methods
		 ***********************************************************/
		virtual const std::string &getGatewayName() const
		{
			return getModuleName();
		}
		virtual const std::string &getFullyQualifiedGatewayName() const
		{
			return getModuleFullyQualifiedName();
		}
		/// Return the gateway proxy of this gateway
//		virtual TModuleGatewayProxyPtr &getGatewayProxy()
//		{
//			nlassert(!_ThisProxy.isNull());
//			return _ThisProxy;
//		}

		/// Create and bind to this gateway a new transport
		virtual void createTransport(const std::string &/* transportClass */, const std::string &/* instanceName */)
		{
		}
		/// Delete a transport (this will close any open route)
		virtual void deleteTransport(const std::string &/* instanceName */)
		{
		}

		/// Activate/stop peer invisible mode on a transport
		virtual void	setTransportPeerInvisible(const std::string &/* transportInstanceName */, bool /* peerInvisible */)
		{
			// unsupported
			nlstop;
		}

		/// Activate/stop firewalling mode on a transport
		virtual void	setTransportFirewallMode(const std::string &/* transportInstanceName */, bool /* firewalled */)
		{
			// unsupported
			nlstop;
		}

		/// Send a command to a transport
		virtual void transportCommand(const TParsedCommandLine &/* commandLine */)
		{
		}
		virtual IGatewayTransport *getGatewayTransport(const std::string &/* transportName */) const
		{
			// there are no transport here
			return NULL;
		}

		virtual uint32	getTransportCount() const
		{
			return 0;
		}

		virtual uint32	getRouteCount() const
		{
			return 0;
		}

		virtual uint32 getReceivedPingCount() const
		{
			return 0;
		}

		virtual void onRouteAdded(CGatewayRoute * /* route */)
		{
		}

		/// A route is removed by a transport
		virtual void onRouteRemoved(CGatewayRoute * /* route */)
		{
		}

		/// A transport have received a message
		virtual void onReceiveMessage(CGatewayRoute * /* from */, const CMessage &/* msgin */)
		{
		}

		virtual void createSecurityPlugin(const std::string &/* className */)
		{
		}
		virtual void sendSecurityCommand(const TParsedCommandLine &/* command */)
		{
		}
		virtual void removeSecurityPlugin()
		{
		}


//		virtual  bool isGatewayServerOpen()
//		{
//			return false;
//		}
//
//		virtual CInetAddress getGatewayServerAddress()
//		{
//			CInetAddress invalid;
//
//			return invalid;
//		}
//		virtual void getGatewayClientList(std::vector<TModuleGatewayProxyPtr> gatewayList)
//		{
//			return;
//		}
//		virtual void openGatewayServer(uint16 listeningPort)
//		{
//			nlstop;
//		}
//		virtual void closeGatewayServer()
//		{
//			nlstop;
//		}
//		virtual void shutdownGatewayServer()
//		{
//			nlstop;
//		}
//		virtual void onGatewayServerOpen()
//		{
//		}
//		virtual void onGatewayServerClose()
//		{
//		}
//		virtual TModuleGatewayConstant onClientGatewayConnect(TModuleGatewayProxyPtr &clientGateway)
//		{
//			return mgc_reject_connection;
//		}
//		virtual void onClientGatewayDisconnect(TModuleGatewayProxyPtr &clientGateway)
//		{
//		}
//		virtual void getGatewayServerList(std::vector<TModuleGatewayProxyPtr> serverList)
//		{
//			return;
//		}
//		virtual bool isGatewayConnected()
//		{
//			return false;
//		}
//		virtual void connectGateway(CInetAddress serverAddress)
//		{
//			nlstop;
//		}
//		virtual void disconnectGateway(TModuleGatewayProxyPtr &serverGateway)
//		{
//			nlstop;
//		}
//		virtual void onGatewayConnection(const TModuleGatewayProxyPtr &serverGateway, TModuleGatewayConstant connectionResult)
//		{
//			nlstop;
//		}
//		virtual void onGatewayDisconnection(const TModuleGatewayProxyPtr &serverGateway)
//		{
//			nlstop;
//		}
		virtual void onAddModuleProxy(IModuleProxy *addedModule)
		{
			// always disclose module to local modules
			discloseModule(addedModule);
		}
		virtual void onRemoveModuleProxy(IModuleProxy * /* removedModule */)
		{
		}
		virtual void discloseModule(IModuleProxy *moduleProxy)
		{
			// check that the module is plugged here
			nlassert(_ModuleProxies.getB(moduleProxy) != NULL);

//			CModuleProxy *modProx = dynamic_cast<CModuleProxy *>(moduleProxy);
//			nlassert(modProx != NULL);
			nlassert(moduleProxy->getModuleGateway() == this);

			// warn any plugged module
			TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
			for (; first != last; ++first)
			{
				IModule *module = first->second;
				if (module->getModuleId() != moduleProxy->getForeignModuleId())
				{
					module->_onModuleUp(moduleProxy);
				}
			}
		}
		virtual IModuleProxy *getPluggedModuleProxy(IModule * /* pluggedModule */)
		{
			return NULL;
		}

		virtual uint32	getProxyCount() const
		{
			return (uint32)_ModuleProxies.getAToBMap().size();
		}

		/// Fill a vector with the list of proxies managed here. The module are filled in ascending proxy id order.
		virtual void	getModuleProxyList(std::vector<IModuleProxy*> &resultList) const
		{
			map<TModuleId, IModuleProxy*> index;
			{
				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
				for (; first != last; ++first)
				{
					index.insert(make_pair(first->first->getModuleProxyId(), first->first));
				}
			}

			// now build the vector
			map<TModuleId, IModuleProxy*>::iterator first(index.begin()), last(index.end());
			for( ; first != last; ++first)
			{
				resultList.push_back(first->second);
			}
		}


//		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, TModuleMessagePtr &message)
//		{
//		}
		virtual void sendModuleProxyMessage(IModuleProxy * /* senderProxy */, IModuleProxy * /* addresseeProxy */, const CMessage &/* message */)
		{
		}
		virtual void dispatchModuleMessage(IModuleProxy * /* senderProxy */, IModuleProxy * /* addresseeProxy */, const CMessage &/* message */)
		{
			nlstop;
//			TModuleId sourceId = message->getSenderModuleProxyId();
//			TModuleProxies::TAToBMap::const_iterator firstSource(_ModuleProxies.getAToBMap().begin()), lastSource(_ModuleProxies.getAToBMap().end());
//			for (; firstSource != lastSource && firstSource->first->getForeignModuleId() != sourceId; ++firstSource) {}
//			nlassert(  firstSource != lastSource );
//
//			TPluggedModules::iterator first(_PluggedModules.begin()), last(_PluggedModules.end());
//			TModuleId destId = message->getAddresseeModuleProxyId();
//			for (; first != last && (*first)->getModuleId() != destId; ++first) {}
//			if (first != last)
//			{
//				(*first)->onProcessModuleMessage(firstSource->first, message);
//			}
		}
		/***********************************************************
		 ** Module methods
		 ***********************************************************/
		bool	initModule(const TParsedCommandLine &initInfo)
		{
			bool ret = CModuleBase::initModule(initInfo);

			// in fact, this gateway is so simple, that it have no option !

			registerSocket();
			registerGateway();

			return ret;
		}

		std::string			buildModuleManifest() const
		{
			return string();
		}

		void				onServiceUp(const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */)
		{
		}
		void				onServiceDown(const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */)
		{
		}
		void				onModuleUpdate()
		{
		}
		void				onApplicationExit()
		{
		}
		void				onModuleUp(IModuleProxy * /* moduleProxy */)
		{
		}
		void				onModuleDown(IModuleProxy * /* moduleProxy */)
		{
		}
		bool				onProcessModuleMessage(IModuleProxy * /* senderModuleProxy */, const CMessage &/* message */)
		{
			return false;
		}
		void				onModuleSecurityChange(IModuleProxy * /* moduleProxy */)
		{
		}
		void				onModuleSocketEvent(IModuleSocket * /* moduleSocket */, TModuleSocketEvent /* eventType */)
		{
		}

		/***********************************************************
		 ** Socket methods
		 ***********************************************************/

		const std::string &getSocketName()
		{
			return getModuleName();
		}

		void _sendModuleMessage(IModule *senderModule, TModuleId destModuleProxyId, const NLNET::CMessage &message)
		{
			TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
			for (; first != last && first->first->getModuleProxyId() != destModuleProxyId; ++first) {}
			if (first != last) {  first->first->sendModuleMessage(senderModule, message); return;}
			throw EModuleNotReachable();
		}
		virtual void _broadcastModuleMessage(IModule * /* senderModule */, const NLNET::CMessage &/* message */)
		{
			nlstop;
		}

		void onModulePlugged(IModule *pluggedModule)
		{
			// A module has just been plugged here, we need to disclose it to the
			// other module, and disclose other module to it.

			// create a proxy for this module
			IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
					this,
					NULL,	// the module is local, so there is no route
					0,		// the module is local, distance is 0
					pluggedModule,	// the module is local, so store the module pointer
					pluggedModule->getModuleClassName(),
					getGatewayName()+"/"+pluggedModule->getModuleFullyQualifiedName(),
					pluggedModule->getModuleManifest(),
//					_ThisProxy,
					pluggedModule->getModuleId());

			// and store it
			_ModuleProxies.add(modProx, CStringMapper::map(modProx->getModuleName()));

			// disclose the new module to other modules
			discloseModule(modProx);

			// second, disclose already plugged proxy to the new one
			{
				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
				for (; first != last; ++first)
				{
					if (first->first->getModuleName() != pluggedModule->getModuleFullyQualifiedName())
						pluggedModule->_onModuleUp(first->first);
				}
			}


		}
		/// Called just after a module as been effectively unplugged from a socket
		void				onModuleUnplugged(IModule *unpluggedModule)
		{
			// remove the proxy info
			TModuleProxies::TBToAMap::const_iterator it(_ModuleProxies.getBToAMap().find(CStringMapper::map(getGatewayName()+"/"+unpluggedModule->getModuleFullyQualifiedName())));
			nlassert(it != _ModuleProxies.getBToAMap().end());

			IModuleProxy *modProx = it->second;
			// warn all connected module that a module become unavailable
			{
				TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;
					if (module->getModuleFullyQualifiedName() != modProx->getModuleName())
						module->_onModuleDown(it->second);
				}
			}

			// warn the unplugged module that all plugged modules are become unavailable
			{
				TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
				for (; first != last; ++first)
				{
					if (first->first->getModuleName() != unpluggedModule->getModuleFullyQualifiedName())
						unpluggedModule->_onModuleDown(first->first);
				}
			}

			TModuleId localProxyId = modProx->getModuleProxyId();
			// remove reference to the proxy
			_ModuleProxies.removeWithA(modProx);

			// release the module proxy
			IModuleManager::getInstance().releaseModuleProxy(localProxyId);

		}

		void getModuleList(std::vector<IModuleProxy*> &resultList)
		{
			TModuleProxies::TAToBMap::const_iterator first(_ModuleProxies.getAToBMap().begin()), last(_ModuleProxies.getAToBMap().end());
			for (; first != last; ++first)
			{
				resultList.push_back(first->first);
			}
		}

	};


	// register the module factory
	NLNET_REGISTER_MODULE_FACTORY(CLocalGateway, "LocalGateway");

	void forceLocalGatewayLink()
	{
	}

} // namespace NLNET

