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
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"

using namespace std;
using namespace NLMISC;



namespace NLNET
{
	struct TSecurityDataDesc
	{
		TSecurityData	*SecurityData;

		TSecurityDataDesc()
			: SecurityData(NULL)
		{
		}

		void serial(CMemStream &s)
		{
			if (!s.isReading())
			{
				// write mode
				TSecurityData *sd = SecurityData;

				uint32 nbSecBlock = 0;
				sint32 tagCountPos = s.reserve(4);
				while (sd != NULL)
				{
					if (sd->DataTag == 0xff)
					{
						TUnknownSecurityData *usd = safe_cast<TUnknownSecurityData *>(sd);
						s.serial(usd->RealDataTag);
					}
					else
					{
						s.serial(sd->DataTag);
					}
					// reserve a place to store the size of the next element
					sint32 pos = s.reserve(4);
					s.serial(*sd);
					// store the size
					uint32 size = s.getPos()-pos-4;
					s.poke(size, pos);

					nbSecBlock++;
					sd = sd->NextItem;
				}

				// store the number of item
				s.poke(nbSecBlock, tagCountPos);
			}
			else
			{
				nlassert(SecurityData == NULL);
				TSecurityData **pLastSd = &SecurityData;
				// read mode
				uint32 nbSecBlock;
				s.serial(nbSecBlock);

				if (nbSecBlock == 0)
					SecurityData = NULL;

				for (uint i=0; i<nbSecBlock; ++i)
				{
					uint8 dataTag;
					s.serial(dataTag);
					uint32 blockSize;
					s.serial(blockSize);
					sint32 pos = s.getPos();
					TSecurityData *sd;

					try
					{
						TSecurityData::TCtorParam param(dataTag);
						sd = NLMISC_GET_FACTORY(TSecurityData, uint8).createObject(dataTag, param);

						if (sd == NULL)
						{
							// we don't know this type, create an unknow security block
							sd = new TUnknownSecurityData(dataTag, blockSize);
							sd->serial(s);
						}
						else
						{
							sd->serial(s);
						}
					}
					catch(const EStreamOverflow &)
					{
						// FAILED to read the security block, rewind to old pos and serial as unknow
						nlwarning("Error while reading stream for security data type %u", dataTag);

						s.seek(pos, NLMISC::IStream::begin);
						sd = new TUnknownSecurityData(dataTag, blockSize);
						sd->serial(s);
					}

					*pLastSd = sd;
					pLastSd = &(sd->NextItem);
				}
			}

		}
	};

	/// Sub message for module description
	struct TModuleDescCodec
	{
		TModuleId	ModuleProxyId;
		uint32		ModuleDistance;
		string		ModuleFullName;
		string		ModuleClass;
		string		ModuleManifest;
		TSecurityDataDesc	SecDesc;


		TModuleDescCodec()
		{
		}

		TModuleDescCodec(IModuleProxy *proxy)
		{
			ModuleProxyId = proxy->getModuleProxyId();
			ModuleDistance = proxy->getModuleDistance()+1;
			ModuleFullName = proxy->getModuleName();
			ModuleClass = proxy->getModuleClassName();
			ModuleManifest = proxy->getModuleManifest();
			SecDesc.SecurityData = const_cast<TSecurityData*>(proxy->getFirstSecurityData());
		}

		void serial(NLMISC::CMemStream &s)
		{
			s.serial(ModuleProxyId);
			s.serial(ModuleDistance);
			s.serial(ModuleFullName);
			s.serial(ModuleManifest);
			s.serial(ModuleClass);
			s.serial(SecDesc);
		}
	};

	/// message for module distance update
	struct TModuleDistanceChangeMsg
	{
		TModuleId	ModuleId;
		uint32		NewDistance;

		void serial(NLMISC::IStream &s)
		{
			s.serial(ModuleId);
			s.serial(NewDistance);
		}
	};

	/// message for module security update
	struct TModuleSecurityChangeMsg
	{
		TModuleId	ModuleId;

		TSecurityDataDesc	SecDesc;

		void serial(NLMISC::CMemStream &s)
		{
			s.serial(ModuleId);
			s.serial(SecDesc);
		}
	};
	/// Message for module removing
	struct TModuleRemMsg
	{
		vector<TModuleId>	RemovedModules;

		void serial(NLMISC::IStream &s)
		{
			s.serialCont(RemovedModules);
		}
	};

	/// Message for module operation
	struct TModuleOperationMsg
	{
		TModuleId	ModuleId;
		string		OperationName;

		CMessage	MessageBody;

		void serial(NLMISC::IStream &s)
		{
			s.serial(ModuleId);
			s.serial(OperationName);
			s.serial(MessageBody);
		}
	};


	/// message waiting next update for local dispatching
	struct TLocalMessage
	{
		TModuleId		SenderProxyId;
		TModuleId		AddresseProxyId;
		CMessage		Message;
	};


	/** Register the gateway in the module manager gateway registry
	 */
	void CModuleGateway::registerGateway()
	{
		IModuleManager::getInstance().registerModuleGateway(this);
	}
	/** Unregister the gateway in the module manager gateway registry
	 */
	void CModuleGateway::unregisterGateway()
	{
		IModuleManager::getInstance().unregisterModuleGateway(this);
	}


	/** The standard gateway that interconnect module
	 *	across process.
	 */
	class CStandardGateway :
		public CModuleBase,
		public CModuleGateway,
		public CModuleSocket
	{
		typedef map<TModuleId, TModuleProxyPtr>		TModuleProxies;
		/// Module proxies managed by this gateway. The map key is the module proxy id
		TModuleProxies		_ModuleProxies;

		typedef CTwinMap<TStringId, TModuleProxyPtr>	TNamedProxyIdx;
		/// Index of name to proxy id
		TNamedProxyIdx		_NameToProxyIdx;

		/// A structure to hold foreign proxy information
		struct TKnownModuleInfo
		{
			TModuleId		ForeignProxyId;
			CGatewayRoute	*Route;
			uint32			ModuleDistance;
			TStringId		ModuleClassId;
		};

		typedef multimap<TStringId, TKnownModuleInfo>	TKnownModuleInfos;
		/** List of known foreign module info.
		 */
		TKnownModuleInfos	_KnownModules;

		typedef map<TModuleId, TModuleId>				TLocalModuleIndex;
		/// Translation table to find module proxies for locally plugged module
		/// The map key is the local module id, the data is the associated proxy id
		TLocalModuleIndex		_LocalModuleIndex;

		typedef map<std::string, IGatewayTransport*>		TTransportList;
		/// the list of active transport
		TTransportList		_Transports;

		typedef set<CGatewayRoute*>		TRouteList;
		// the list of available routes
		TRouteList		_Routes;

		/// The security plug-in (if any)
		CGatewaySecurity		*_SecurityPlugin;

		/// Ping counter for debug purpose
		uint32					_PingCounter;

		typedef std::list<TLocalMessage>	TLocalMessageList;
		/// List of local message waiting dispatching at next update
		TLocalMessageList		_LocalMessages;

	public:

		CStandardGateway()
			:	_SecurityPlugin(NULL),
				_PingCounter(0)
		{
		}

		~CStandardGateway()
		{
			// we need to unplug any plugged module
			while (!_PluggedModules.getAToBMap().empty())
			{
				_PluggedModules.getAToBMap().begin()->second->unplugModule(this);
			}

			// delete all transport
			while (!_Transports.empty())
			{
				deleteTransport(_Transports.begin()->first);
			}

			// delete security plug-in
			if (_SecurityPlugin != NULL)
				removeSecurityPlugin();

			// must be done before the other destructors are called
			unregisterSocket();
			unregisterGateway();
		}

		CModuleProxy *getModuleProxy(TModuleId proxyId)
		{
			TModuleProxies::iterator it(_ModuleProxies.find(proxyId));
			if (it == _ModuleProxies.end())
				return NULL;
			return static_cast<CModuleProxy*>(it->second.getPtr());
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

		/// Create and bind to this gateway a new transport
		virtual void createTransport(const std::string &transportClass, const std::string &instanceName)
		{
			if (_Transports.find(instanceName) != _Transports.end())
			{
				nlwarning("A transport with the name '%s' already exist in this gateway", instanceName.c_str());
				return;
			}

			IGatewayTransport::TCtorParam param;
			param.Gateway = this;
			IGatewayTransport *transport = NLMISC_GET_FACTORY(IGatewayTransport, std::string).createObject(transportClass, param);

			if (transport == NULL)
			{
				nlwarning("Failed to create a transport with the class '%s'", transportClass.c_str());
				return;
			}

			// Store the transport
//			TTransportInfo *ti = new TTransportInfo(transport);
			_Transports.insert(make_pair(instanceName, transport));
//			_TransportPtrIdx.insert(make_pair(transport, ti));

			nldebug("NETL6: Gateway transport %s (%s) created", instanceName.c_str(), transportClass.c_str());
		}

		/// Delete a transport (this will close any open route)
		virtual void deleteTransport(const std::string &instanceName)
		{
			TTransportList::iterator it(_Transports.find(instanceName));
			if (it == _Transports.end())
			{
				nlwarning("Unknown transport named '%s'", instanceName.c_str());
				return;
			}

			nldebug("NETL6: Gateway transport '%s' deleted", instanceName.c_str());
			// delete the transport
			IGatewayTransport *transport = it->second;
//			nlassert(_TransportPtrIdx.find(transport) != _TransportPtrIdx.end());
//			_TransportPtrIdx.erase(transport);
			delete transport;
//			delete it->second;
			_Transports.erase(it);
		}

		/// Activate/stop peer invisible mode on a transport
		virtual void	setTransportPeerInvisible(const std::string &transportInstanceName, bool peerInvisible)
		{
			TTransportList::iterator it(_Transports.find(transportInstanceName));
			if (it == _Transports.end())
			{
				nlwarning("Unknown transport named '%s'", transportInstanceName.c_str());
				return;
			}

			IGatewayTransport *transport= it->second;

			if (peerInvisible == transport->PeerInvisible)
				// nothing more to do
				return;

			// set the mode
			transport->PeerInvisible = peerInvisible;

			nldebug("NETL6: Gateway transport %s peer invisible mode %s", transportInstanceName.c_str(), peerInvisible? "ON" : "OFF");


			// For each route of this transport, we need to disclose/undisclose peer modules
			TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CGatewayRoute *route = *first;

				if (route->getTransport() == transport)
				{
					// this route need to be filtered
					TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
					for (; first != last; ++first)
					{
						IModuleProxy *proxy = first->second;
						if (proxy->getGatewayRoute() != NULL
							&& proxy->getGatewayRoute() != route
							&& proxy->getGatewayRoute()->getTransport() == transport)
						{
							// this module is on the same transport, but another route, remove/add it from the
							// route
							if (peerInvisible)
								undiscloseModuleToRoute(route, proxy);
							else
							{
								// check firewall rules
								if (!route->getTransport()->Firewalled)
									discloseModuleToRoute(route, proxy);
							}
						}
					}
				}
			}
		}

		/// Activate/stop firewalling mode on a transport
		virtual void	setTransportFirewallMode(const std::string &transportInstanceName, bool firewalled)
			throw (EGatewayFirewallBreak)
		{
			TTransportList::iterator it(_Transports.find(transportInstanceName));
			if (it == _Transports.end())
			{
				nlwarning("Unknown transport named '%s'", transportInstanceName.c_str());
				return;
			}

			IGatewayTransport *transport = it->second;

			if (firewalled == transport->Firewalled)
				// nothing to do
				return;

			if (firewalled && transport->getRouteCount() != 0)
				throw EGatewayFirewallBreak();

			/// set the firewall mode
			transport->Firewalled = firewalled;

			nldebug("NETL6: Gateway transport %s firewall mode %s", transportInstanceName.c_str(), firewalled? "ON" : "OFF");

			if (firewalled == false)
			{
				// we need to disclose all module not disclosed yet
				TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					CGatewayRoute *route = *first;

					if (route->getTransport() == transport)
					{
						// this route need to be unfiltered
						TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
						for (; first != last; ++first)
						{
							IModuleProxy *proxy = first->second;
							if (proxy->getGatewayRoute() == NULL || (proxy->getGatewayRoute() != route ))
							{
								// this module is on another route, disclose it if needed
								if (route->FirewallDisclosed.find(proxy->getModuleProxyId()) == route->FirewallDisclosed.end())
									discloseModuleToRoute(route, proxy);
							}
						}
					}
					// clear the firewall disclosed table
					route->FirewallDisclosed.clear();
				}
			}
		}

		/// Send a command to a transport
		virtual void transportCommand(const TParsedCommandLine &commandLine)
		{
			for (uint i=1; i<commandLine.SubParams.size(); ++i)
			{
				const TParsedCommandLine * subParam = commandLine.SubParams[i];

				std::string transportName = subParam->ParamName;
				TTransportList::const_iterator it(_Transports.find(transportName));
				if (it == _Transports.end())
				{
					nlwarning("Unknown transport named '%s', ignoring command.", transportName.c_str());
				}
				else if (subParam->SubParams.empty())
				{
					nlwarning("Can't find sub param list for transport '%s' command", transportName.c_str());
				}
				else
				{
					nldebug("NETL6: Gateway transport %s, sending command '%s'", transportName.c_str(), commandLine.toString().c_str());
					// ok, we have a valid transport, send the command
					IGatewayTransport *transport = it->second;
					if (!transport->onCommand(*subParam))
						return;
				}
			}
		}

		virtual IGatewayTransport *getGatewayTransport(const std::string &transportName) const
		{
			TTransportList::const_iterator it(_Transports.find(transportName));

			if (it == _Transports.end())
				return NULL;
			else
				return it->second;
		}

		virtual uint32	getTransportCount() const
		{
			return (uint32)_Transports.size();
		}

		virtual uint32	getRouteCount() const
		{
			return (uint32)_Routes.size();
		}

		virtual uint32 getReceivedPingCount() const
		{
			return _PingCounter;
		}

		virtual void onRouteAdded(CGatewayRoute *route)
		{
			nlassert(route != NULL);
			// Remember the new route
			nlassert(_Routes.find(route) == _Routes.end());
			_Routes.insert(route);

			// a new route is available, disclose known modules
			{
				TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
				for (; first != last; ++first)
				{
					IModuleProxy *modProx = first->second;

					// only transmit module desc coming from other routes
					// and other transport if peer invisible
					if (isModuleProxyVisible(modProx, route))
					{
						discloseModuleToRoute(route, modProx);
					}
				}
			}
		}

		/// A route is removed by a transport
		virtual void onRouteRemoved(CGatewayRoute *route)
		{
			nlassert(route != NULL);
			nlassert(_Routes.find(route) != _Routes.end());
			// we need to remove all the proxy that come from this route
//			CGatewayRoute::TForeignToLocalIdx::TAToBMap::const_iterator first(route->ForeignToLocalIdx.getAToBMap().begin()), last(route->ForeignToLocalIdx.getAToBMap().end());
//			for (; first != last; ++first)
			while (!route->ForeignToLocalIdx.getAToBMap().empty())
			{
				removeForeignModule(route, route->ForeignToLocalIdx.getAToBMap().begin()->first);
//				TModuleId localProxyId = first->second;
//				TModuleProxies::iterator it(_ModuleProxies.find(localProxyId));
//				nlassert(it != _ModuleProxies.end());
//
//				IModuleProxy *modProx = it->second;
//
//				// trigger an event in the gateway
//				onRemoveModuleProxy(modProx);
//
//				// remove proxy record from the proxy list
//				_ModuleProxies.erase(it);
//				_NameToProxyIdx.removeWithB(modProx);
//
//				// Release the proxy object
//				IModuleManager::getInstance().releaseModuleProxy(modProx->getModuleProxyId());
			}
			// cleanup the translation table
//			route->ForeignToLocalIdx.clear();

			// clear the route tracker
			_Routes.erase(route);

			// cleanup route state
			route->ForeignToLocalIdx.clear();
			route->PendingEvents.clear();
			route->FirewallDisclosed.clear();
			route->NextMessageType = CModuleMessageHeaderCodec::mt_invalid;
			route->NextSenderProxyId = 0;
			route->NextAddresseeProxyId = 0;
		}

		/// A transport have received a message
		virtual void onReceiveMessage(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveMessage);
			// dispatch the message
			if (from->NextMessageType != CModuleMessageHeaderCodec::mt_invalid)
			{
				// this message must be dispatched to a module
				onReceiveModuleMessage(from, msgin);
			}
			// Not a module message, dispatch the gateway message
			else if (msgin.getName() == "MOD_OP")
			{
				onReceiveModuleMessageHeader(from, msgin);
			}
			else if (msgin.getName() == "MOD_UPD")
			{
				onReceiveModuleUpdate(from, msgin);
			}
//			else if (msgin.getName() == "MOD_ADD")
//			{
//				onReceiveModuleAdd(from, msgin);
//			}
//			else if (msgin.getName() == "MOD_REM")
//			{
//				onReceiveModuleRemove(from, msgin);
//			}
//			else if (msgin.getName() == "MOD_DST_UPD")
//			{
//				onReceiveModuleDistanceUpdate(from, msgin);
//			}
		}


		/***********************************/
		/* security plug-in management*/
		/***********************************/
		/** create a security plug-in.
		 *	There must be no security plug-in currently created.
		 */
		virtual void createSecurityPlugin(const std::string &className)
		{
			if (_SecurityPlugin != NULL)
			{
				nlwarning("NLNETL5 : CStandardGateway::createSecurityPlugin : plug-in already created ");
				return;
			}

			CGatewaySecurity::TCtorParam params;
			params.Gateway = this;
			CGatewaySecurity *gs = NLMISC_GET_FACTORY(CGatewaySecurity, std::string).createObject(className, params);
			if (gs == NULL)
			{
				nlwarning("NLNETL5 : CStandardGateway::createSecurityPlugin : can't create a security plug-in for class '%s'", className.c_str());
				return;
			}

			// store the security plug-in
			_SecurityPlugin = gs;

			// update security for all existing proxies
			TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
			for (; first != last; ++first)
			{
				IModuleProxy *proxy = first->second;
				_SecurityPlugin->onNewProxy(proxy);
			}
		}
		/** Send a command to the security plug-in */
		virtual void sendSecurityCommand(const TParsedCommandLine &command)
		{
			if (_SecurityPlugin != NULL)
			{
				nlwarning("NLNETL5 : CStandardGateway::sendSecurityCommand : plug-in NOT created ");
				return;
			}

			_SecurityPlugin->onCommand(command);
		}

		/** Remove the security plug-in.
		 */
		virtual void removeSecurityPlugin()
		{
			if (_SecurityPlugin == NULL)
			{
				nlwarning("NLNETL5 : CStandardGateway::removeSecurityPlugin : plug-in not created");
				return;
			}

			// delete the plug-in (this can remove some security data)
			_SecurityPlugin->onDelete();
			delete _SecurityPlugin;
			_SecurityPlugin = NULL;
		}

		/** Set a security data block. If a bloc of the same type
		 *	already exist in the list, the new one will replace the
		 *	existing one.
		 */
		void setSecurityData(IModuleProxy *proxy, TSecurityData *securityData)
		{
			nlassert(proxy->getModuleGateway() == this);
			nlassert(securityData->NextItem == NULL);

			CModuleProxy *modProx = dynamic_cast<CModuleProxy*>(proxy);
			nlassert(modProx != NULL);

			// look in the existing security for a matching type and remove it
			removeSecurityData(proxy, securityData->DataTag);

			// now, store the security data
			securityData->NextItem = modProx->_SecurityData;
			modProx->_SecurityData = securityData;


		}

		/** Clear a block of security data
		 *	The block is identified by the data tag
		 */
		bool removeSecurityData(IModuleProxy *proxy, uint8 dataTag)
		{
			nlassert(proxy->getModuleGateway() == this);

			CModuleProxy *modProx = dynamic_cast<CModuleProxy*>(proxy);
			nlassert(modProx != NULL);

			bool ret = false;
			TSecurityData *prevSec = NULL;
			TSecurityData *currentSec = modProx->_SecurityData;
			while (currentSec != NULL)
			{
				if (currentSec->DataTag == dataTag)
				{
					if (prevSec != NULL)
						prevSec->NextItem = currentSec->NextItem;
					else
						modProx->_SecurityData = currentSec->NextItem;

					TSecurityData *toDelete = currentSec;
					currentSec = currentSec->NextItem;
					toDelete->NextItem = NULL;
					delete toDelete;
					ret = true;
				}
				else
				{
					prevSec = currentSec;
					currentSec = currentSec->NextItem;
				}
			}

			return ret;
		}

		void replaceAllSecurityDatas(IModuleProxy *proxy, TSecurityData *securityData)
		{
			nlassert(proxy->getModuleGateway() == this);

			CModuleProxy *modProx = dynamic_cast<CModuleProxy*>(proxy);
			nlassert(modProx != NULL);
			nlassert(modProx->_SecurityData != securityData);

			if (modProx->_SecurityData != NULL)
				delete modProx->_SecurityData;

			modProx->_SecurityData = securityData;
		}

		/** Ask the gateway to resend the security data.
		 *	The plug-in call this method after having changed
		 *	the security info for a plug-in outside of the
		 *	onNewProxy call.
		 */
		void forceSecurityUpdate(IModuleProxy *proxy)
		{
			TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CGatewayRoute *route = *first;

				if (isModuleProxyVisible(proxy, route))
				{
					updateModuleSecurityDataToRoute(route, proxy);
				}
			}
		}

		/***********************************/
		/* Inter gateway message reception */
		/***********************************/

		/** A gateway receive module operation */
		void onReceiveModuleMessage(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleMessage);
			// clean the message type now, any return path will be safe
			from->NextMessageType = CModuleMessageHeaderCodec::mt_invalid;

			// Retrieve sender and destination proxy and recall gateway send method
			IModuleProxy *senderProxy;
			IModuleProxy *addresseeProxy;

			TModuleProxies::iterator it;

			// sender proxy
			it = _ModuleProxies.find(from->NextSenderProxyId);
			if (it == _ModuleProxies.end())
			{
				nlwarning("Can't dispatch the module message, sender proxy %u is not in this gateway", from->NextSenderProxyId);
				return;
			}
			senderProxy = it->second;
			// addressee proxy
			it = _ModuleProxies.find(from->NextAddresseeProxyId);
			if (it == _ModuleProxies.end())
			{
				nlwarning("Can't dispatch the module message '%s', sender proxy %u is not in this gateway", msgin.getName().c_str(), from->NextAddresseeProxyId);
				return;
			}
			addresseeProxy = it->second;

			// give the message to the gateway (either for local dispatch or for forwarding)
			sendModuleMessage(senderProxy, addresseeProxy, msgin);
		}

		// A gateway receive a module message header
		void onReceiveModuleMessageHeader(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleMessageHeader);
			if (from->NextMessageType != CModuleMessageHeaderCodec::mt_invalid)
			{
				// juste warn (but that is VERY BAD)
				nlwarning("Receiving a new module message header without having received the previous module message !");
			}

			// store the message information in the route
			CModuleMessageHeaderCodec::decode(
				msgin,
				from->NextMessageType,
				from->NextSenderProxyId,
				from->NextAddresseeProxyId);

			// translate sender id
			const TModuleId *pmoduleId = from->ForeignToLocalIdx.getB(from->NextSenderProxyId);
			if (pmoduleId  == NULL)
			{
				nlwarning("The sender proxy %u is unknown in the translation table, can't dispatch the message !", from->NextSenderProxyId);
				from->NextMessageType = CModuleMessageHeaderCodec::mt_invalid;
				return;
			}
			from->NextSenderProxyId = *pmoduleId;
			// now, wait the message body
		}

		/** A gateway receive a general update message */
		void onReceiveModuleUpdate(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleUpdate);
			while (uint32(msgin.getPos()) != msgin.length())
			{
				CGatewayRoute::TPendingEventType type = CGatewayRoute::pet_disclose_module;
//				msgin.serialShortEnum(type);
				nlRead(msgin, serialShortEnum, type);

				switch (type)
				{
				case CGatewayRoute::pet_disclose_module:

					onReceiveModuleAdd(from, msgin);

					break;
				case CGatewayRoute::pet_undisclose_module:

					onReceiveModuleRemove(from, msgin);

					break;
				case CGatewayRoute::pet_update_distance:

					onReceiveModuleDistanceUpdate(from, msgin);

					break;
				case CGatewayRoute::pet_update_security:

					onReceiveModuleSecurityUpdate(from, msgin);

					break;
				default:
					// should not append
					nlstop;
				}
			}
		}

		/** A gateway send new modules information */
		void onReceiveModuleAdd(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleAdd);
			TModuleDescCodec modDesc;
			nlRead(msgin, serial, modDesc);

			// for each received module info
			TStringId modNameId = CStringMapper::map(modDesc.ModuleFullName);
			/// store the module information
			TKnownModuleInfo modInfo;
			modInfo.ForeignProxyId = modDesc.ModuleProxyId;
			modInfo.ModuleClassId = CStringMapper::map(modDesc.ModuleClass);
			modInfo.ModuleDistance = modDesc.ModuleDistance;
			modInfo.Route = from;

			nldebug("Gateway '%s' : store module info for '%s' (foreign ID %u) @ %u hop",
				getGatewayName().c_str(),
				modDesc.ModuleFullName.c_str(),
				modDesc.ModuleProxyId,
				modDesc.ModuleDistance);

			// Store module information
			_KnownModules.insert(make_pair(modNameId, modInfo));

			if (_NameToProxyIdx.getB(modNameId) != NULL)
			{
				// a proxy for this module already exist,
				IModuleProxy *modProx = *(_NameToProxyIdx.getB(modNameId));

				// fill the id translation table
//					from->ForeignToLocalIdx.insert(make_pair(modDesc.ModuleProxyId, modProx->getModuleProxyId()));
				from->ForeignToLocalIdx.add(modDesc.ModuleProxyId, modProx->getModuleProxyId());

				// check if this route is better
				if (modProx->getModuleDistance() > modInfo.ModuleDistance)
				{
					// update module distance and swap route
					CModuleProxy *proxy = static_cast<CModuleProxy*>(modProx);

					nldebug("Gateway '%s' : Use a shorter path for '%s' from %u to %u hops",
						getGatewayName().c_str(),
						modDesc.ModuleFullName.c_str(),
						proxy->_Distance,
						modInfo.ModuleDistance);

					proxy->_Distance = modInfo.ModuleDistance;
					proxy->_Route = modInfo.Route;

					sendModuleDistanceUpdate(proxy);
				}

				// update the security if needed
				if (modDesc.SecDesc.SecurityData != NULL)
				{
					CModuleProxy *proxy = static_cast<CModuleProxy *>(modProx);
					if (_SecurityPlugin != NULL)
					{
						_SecurityPlugin->onNewSecurityData(from, proxy, modDesc.SecDesc.SecurityData);
					}
					else
					{
						if (proxy->_SecurityData != NULL)
							delete proxy->_SecurityData;
						proxy->_SecurityData = modDesc.SecDesc.SecurityData;
					}
				}
			}
			else
			{
				// we need to create a new proxy
				// create a module proxy
				IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
					this,
					from,
					modDesc.ModuleDistance,
					NULL,
					modDesc.ModuleClass,
					modDesc.ModuleFullName,
					modDesc.ModuleManifest,
					modDesc.ModuleProxyId);

				// set the module security
				CModuleProxy *proxy = static_cast<CModuleProxy *>(modProx);
				proxy->_SecurityData = modDesc.SecDesc.SecurityData;
				// let the security plug-in add/remove security data
				if (_SecurityPlugin != NULL)
					_SecurityPlugin->onNewProxy(proxy);

				// store the proxy in the proxy list
				_ModuleProxies.insert(make_pair(modProx->getModuleProxyId(), modProx));
				_NameToProxyIdx.add(modNameId, modProx);

				// Fill the proxy id translation table for this route
//					from->ForeignToLocalIdx.insert(make_pair(modDesc.ModuleProxyId, modProx->getModuleProxyId()));
				from->ForeignToLocalIdx.add(modDesc.ModuleProxyId, modProx->getModuleProxyId());

				// trigger an event in the gateway
				onAddModuleProxy(modProx);
			}
		}

		void onReceiveModuleRemove(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleRemove);
			TModuleId	moduleId;
			nlRead(msgin, serial, moduleId);

			removeForeignModule(from, moduleId);
		}

		void onReceiveModuleDistanceUpdate(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleDistanceUpdate);
			TModuleId moduleId;
			uint32	newDistance;

			nlRead(msgin, serial, moduleId);
			nlRead(msgin, serial, newDistance);

			// translate the module id
			const TModuleId *pModuleId = from->ForeignToLocalIdx.getB(moduleId);
			if (pModuleId == NULL)
			{
				nlwarning("Receive a module distance update for foreign module %u, but no translation available", moduleId);
				return;
			}

			TModuleId localId = *pModuleId;

			// now, retrieve the module info and update
			TModuleProxies::iterator it2(_ModuleProxies.find(localId));
			nlassert(it2 != _ModuleProxies.end());
			CModuleProxy *proxy = static_cast<CModuleProxy*>(it2->second.getPtr());

			pair<TKnownModuleInfos::iterator, TKnownModuleInfos::iterator> range;
			range = _KnownModules.equal_range(proxy->_FullyQualifiedModuleName);

			for (; range.first != range.second; ++range.first)
			{
				TKnownModuleInfo &kmi = range.first->second;
				if (kmi.Route == from)
				{
					// we found the module info, update the data
					nldebug("Gateway '%s' : updating distance from %u to %u hop for module '%s'",
						getGatewayName().c_str(),
						kmi.ModuleDistance,
						newDistance,
						CStringMapper::unmap(range.first->first).c_str());
					kmi.ModuleDistance = newDistance;
					break;
				}
			}
			nlassert(range.first != range.second);

			// check if the changed module is the one currently in use
			if (proxy->_Route == from)
			{
				// two task : first, if the new distance is greater, look
				// in available route for a shorter path,
				// second, send a module distance update for this module.
				if (proxy->_Distance < newDistance)
				{
					// look for a shorter path
					range = _KnownModules.equal_range(proxy->_FullyQualifiedModuleName);

					for (; range.first != range.second; ++range.first)
					{
						TKnownModuleInfo &kmi = range.first->second;
						if (kmi.ModuleDistance < proxy->_Distance)
						{
							nldebug("Gateway '%s' : proxy '%s' use a new path from %u to %u hop",
								getGatewayName().c_str(),
								proxy->getModuleName().c_str(),
								proxy->_Distance,
								kmi.ModuleDistance);
							// this path is shorter, use it now
							proxy->_Route = kmi.Route;
							proxy->_ForeignModuleId = kmi.ForeignProxyId;
							proxy->_Distance = kmi.ModuleDistance;
							break;
						}
					}
					if (range.first == range.second)
					{
						// no shorter path found, update the proxy
						nldebug("Gateway '%s' : proxy '%s' path distance changed from %u to %u hop",
							getGatewayName().c_str(),
							proxy->getModuleName().c_str(),
							proxy->_Distance,
							newDistance);

						proxy->_Distance = newDistance;
					}
				}
				else
				{
					// the new distance is shorter, just update
					nldebug("Gateway '%s' : proxy '%s' path distance reduced from %u to %u hop",
						getGatewayName().c_str(),
						proxy->getModuleName().c_str(),
						proxy->_Distance,
						newDistance);

					proxy->_Distance = newDistance;
				}

				// send the distance update
				sendModuleDistanceUpdate(proxy);
			}
		}

		void onReceiveModuleSecurityUpdate(CGatewayRoute *from, const CMessage &msgin)
		{
			H_AUTO(CModuleGetaway_onReceiveModuleSecurityUpdate);
//			TModuleId foreignModuleId;
//			TSecurityData *modSec;
			TModuleSecurityChangeMsg secChg;

			nlRead(msgin, serial, secChg);
//			msgin.serial(foreignModuleId);
//			msgin.serialPolyPtr(modSec);

			const TModuleId *pModuleId = from->ForeignToLocalIdx.getB(secChg.ModuleId);
			if (pModuleId == NULL)
			{
				nlwarning("LNETL6 : receive module security update for unknown module foreign proxy %u", secChg.ModuleId);
				return;
			}

			TModuleId moduleId = *pModuleId;

			CModuleProxy *modProx = getModuleProxy(moduleId);
			if (modProx == NULL)
			{
				nlwarning("LNETL6 : receive module security update for unknown module proxy %u, foreign %u", moduleId, secChg.ModuleId);
				return;
			}

			// allow the security plug-in to affect the data
			if( _SecurityPlugin != NULL)
			{
				// let the plug-in update the security data
				_SecurityPlugin->onNewSecurityData(from, modProx, secChg.SecDesc.SecurityData);
			}
			else
			{
				// update the security data in the proxy
				replaceAllSecurityDatas(modProx, secChg.SecDesc.SecurityData);
			}

			// warn local module about new security data
			{
				TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;

					module->onModuleSecurityChange(modProx);
				}
			}

			// update the security to peers
			{
				TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					CGatewayRoute *route = *first;
					if (isModuleProxyVisible(modProx, route))
					{
						updateModuleSecurityDataToRoute(route, modProx);
					}
				}
			}
		}


		virtual void onAddModuleProxy(IModuleProxy *addedModule)
		{
			H_AUTO(CModuleGetaway_onAddmoduleProxy);
			// disclose module to local modules
			discloseModule(addedModule);

			// and send module info to any route

			// for each route
			TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CGatewayRoute *route = *first;
				// only send info to other routes
				if (isModuleProxyVisible(addedModule, route))
				{
					discloseModuleToRoute(route, addedModule);
				}
			}
		}

		virtual void onRemoveModuleProxy(IModuleProxy *removedModule)
		{
			H_AUTO(CModuleGetaway_onRemoveModuleProxy);
			// for each route
			{
				// for each route
				TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					CGatewayRoute *route = *first;
					// only send info to other routes
					if (isModuleProxyVisible(removedModule, route))
					{
						undiscloseModuleToRoute(route, removedModule);
					}
				}
			}

			// warn any locally plugged module
			{
				TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;
					if (removedModule->getGatewayRoute() != NULL
						|| module->getModuleId() != removedModule->getForeignModuleId())
					{
						module->_onModuleDown(removedModule);
					}
				}
			}
		}

		virtual void discloseModule(IModuleProxy *moduleProxy)
			throw (EGatewayNotConnected)
		{
			nlassert(moduleProxy->getModuleGateway() == this);

			// warn any plugged module
			TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
			for (; first != last; ++first)
			{
				IModule *module = first->second;
				if (moduleProxy->getGatewayRoute() != NULL
					|| module->getModuleId() != moduleProxy->getForeignModuleId())
				{
					module->_onModuleUp(moduleProxy);
				}
			}
		}

		virtual IModuleProxy *getPluggedModuleProxy(IModule *pluggedModule)
		{
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(pluggedModule->getModuleId()));

			if (it == _LocalModuleIndex.end())
				return NULL;
			else
			{
				TModuleProxies::iterator it2(_ModuleProxies.find(it->second));
				nlassert(it2 != _ModuleProxies.end());
				return it2->second;
			}
		}

		virtual uint32	getProxyCount() const
		{
			return (uint32)_ModuleProxies.size();
		}

		/// Fill a vector with the list of proxies managed here. The module are filled in ascending proxy id order.
		virtual void	getModuleProxyList(std::vector<IModuleProxy*> &resultList) const
		{
			TModuleProxies::const_iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
			for (; first != last; ++first)
			{
				resultList.push_back(first->second);
			}
		}


		virtual void sendModuleMessage(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, const NLNET::CMessage &message)
		{
			H_AUTO(CModuleGetaway_sendModuleMessage);
			// manage firewall
			if (addresseeProxy->getGatewayRoute()
				&& addresseeProxy->getGatewayRoute()->getTransport()->Firewalled)
			{
				CGatewayRoute *route = addresseeProxy->getGatewayRoute();
				// the destination route is firewalled, we need to
				// disclose the sender module if it's not already done
				if (route->FirewallDisclosed.find(senderProxy->getModuleProxyId()) == route->FirewallDisclosed.end())
				{
					discloseModuleToRoute(route, senderProxy);
					route->FirewallDisclosed.insert(senderProxy->getModuleProxyId());
				}
			}

			// check for visibility rules
			if (!isModuleProxyVisible(addresseeProxy, senderProxy->getGatewayRoute()))
			{
				nlwarning("Module %u '%s' try to send message to %u '%s' but addressee is not visible, message discarded",
					senderProxy->getModuleProxyId(),
					senderProxy->getModuleName().c_str(),
					addresseeProxy->getModuleProxyId(),
					addresseeProxy->getModuleName().c_str());
				return;
			}

			if (addresseeProxy->getGatewayRoute() == NULL)
			{
				// the module is local, just forward the call to the dispatcher
				nlassert(senderProxy != NULL);
				nlassert(_ModuleProxies.find(senderProxy->getModuleProxyId()) != _ModuleProxies.end());

				// invert the message for immediate dispatching if needed
				if (!message.isReading())
					const_cast<CMessage&>(message).invert();

				// check if the module support immediate dispatching
				TModuleId addresseeModId = addresseeProxy->getForeignModuleId();

				const TModulePtr *adrcp = _PluggedModules.getB(addresseeModId);
				if (adrcp == NULL)
				{
					nlwarning("sendModuleMessage : can't find addressee module %u that is not plugged here !", addresseeModId);
					return;
				}

				IModule *addreseeMod = *adrcp;
				if (!addreseeMod->isImmediateDispatchingSupported())
				{
					// dispatch the message at next gateway update
					// this provide a coherent behavior between local and distant module message exchange

					_LocalMessages.push_back(TLocalMessage());
					TLocalMessage &lm = _LocalMessages.back();
					lm.SenderProxyId = senderProxy->getModuleProxyId();
					lm.AddresseProxyId = addresseeProxy->getModuleProxyId();

					nldebug("NETL6 : gateway '%s' : queuing local message '%s' from proxy %u to proxy %u",
						getModuleName().c_str(),
						message.getName().c_str(),
						lm.SenderProxyId,
						lm.AddresseProxyId);

					if (message.hasLockedSubMessage())
					{
						lm.Message.assignFromSubMessage(message);
					}
					else
					{
						lm.Message = message;
					}
				}
				else
				{
					// immediate dispatching
					dispatchModuleMessage(senderProxy, addresseeProxy, message);
				}
			}
			else
			{
				// the module is distant, send the message via the route
				// create a message for sending
				CMessage msgHeader("MOD_OP");
				CModuleMessageHeaderCodec::encode(
					msgHeader,
					CModuleMessageHeaderCodec::mt_oneway,
					senderProxy->getModuleProxyId(),
					addresseeProxy->getForeignModuleId());

				// send any pending module info
				sendPendingModuleUpdate(addresseeProxy->getGatewayRoute());

				// send the header
				addresseeProxy->getGatewayRoute()->sendMessage(msgHeader);
				// send the message
				addresseeProxy->getGatewayRoute()->sendMessage(message);
			}
		}
		virtual void dispatchModuleMessage(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, const CMessage &message)
		{
			H_AUTO(CModuleGetaway_dispatchModuleMessage);
			CMessage::TMessageType msgType = message.getType();
			// retrieve the address module from the proxy
			nlassert(addresseeProxy->getGatewayRoute() == NULL);
			// As the addressee is local, the foreign proxy id is the local module id (a bit triky...)
			TModuleId addresseeModId = addresseeProxy->getForeignModuleId();

			const TModulePtr *adrcp = _PluggedModules.getB(addresseeModId);
			if (adrcp == NULL)
			{
				nlwarning("dispatchModuleMessage : dispatching a message to module %u that is not plugged here !", addresseeModId);
				return;
			}

			IModule *addreseeMod = *adrcp;

			// finally, transmit the message to the module
//			addreseeMod->onProcessModuleMessage(senderProxy, message);
			try
			{
				addreseeMod->onReceiveModuleMessage(senderProxy, message);
			}
			catch(...)
			{
				nlwarning("An exception was thrown while dispatching message '%s' from '%s' to '%s'",
					message.getName().c_str(),
					senderProxy->getModuleName().c_str(),
					addresseeProxy->getModuleName().c_str());

				if (msgType == CMessage::Request)
				{
					// send back an exception message
					CMessage except;
					except.setType("EXCEPT", CMessage::Except);
					senderProxy->sendModuleMessage(addreseeMod, except);
				}
			}
		}
		/***********************************************************
		 ** Module methods
		 ***********************************************************/
		bool	initModule(const TParsedCommandLine &initInfo)
		{
			bool ret = CModuleBase::initModule(initInfo);

			// no options for now

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
			H_AUTO(CModuleGetaway_onModuleUpdate);
			// send waiting local messages
			while (!_LocalMessages.empty())
			{
				TLocalMessage &lm = _LocalMessages.front();

				IModuleProxy *senderProx = getModuleProxy(lm.SenderProxyId);
				IModuleProxy *addresseeProx = getModuleProxy(lm.AddresseProxyId);

				if (senderProx == NULL)
				{
					nlwarning("CStandardGateway : local message dispatching : Failed to retrieve proxy for sender module %u while dispatching message '%s' to %u",
						lm.SenderProxyId,
						lm.Message.getName().c_str(),
						lm.AddresseProxyId);
				}
				else if (addresseeProx == NULL)
				{
					nlwarning("CStandardGateway : local message dispatching : Failed to retrieve proxy for addressee module %u while dispatching message '%s' from %u",
						lm.AddresseProxyId,
						lm.Message.getName().c_str(),
						lm.SenderProxyId);
				}
				else
				{
					// we can dispatch the message
					dispatchModuleMessage(senderProx, addresseeProx, lm.Message);
				}

				_LocalMessages.pop_front();
			}

			// send pending module un/disclosure
			{
				TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
				for (; first != last; ++first)
				{
					CGatewayRoute *route = *first;
					sendPendingModuleUpdate(route);
				}
			}
			// update the transports
			{
				TTransportList::iterator first(_Transports.begin()), last(_Transports.end());
				for (; first != last; ++first)
				{
					IGatewayTransport *transport = first->second;

					transport->update();
				}
			}
		}

		void				onApplicationExit()
		{
			// delete all transport
			while (!_Transports.empty())
			{
				deleteTransport(_Transports.begin()->first);
			}
		}

		void				onModuleUp(IModuleProxy * /* moduleProxy */)
		{
		}
		void				onModuleDown(IModuleProxy * /* moduleProxy */)
		{
		}
		bool				onProcessModuleMessage(IModuleProxy * /* senderModuleProxy */, const CMessage &message)
		{
			// simple message for debug and unit testing
			if (message.getName() == "DEBUG_MOD_PING")
			{
				_PingCounter++;

				return true;
			}

			return false;
		}

		void				onModuleSecurityChange(IModuleProxy * /* moduleProxy */)
		{
		}

		void	onModuleSocketEvent(IModuleSocket * /* moduleSocket */, TModuleSocketEvent /* eventType */)
		{
		}

		/***********************************************************
		 ** Socket methods
		 ***********************************************************/

		const std::string &getSocketName()
		{
			return getModuleName();
		}

		void _sendModuleMessage(IModule *senderModule, TModuleId destModuleProxyId, const NLNET::CMessage &message )
			throw (EModuleNotReachable, EModuleNotPluggedHere)
		{
			// the socket implementation already checked that the module is plugged here
			// just check that the destination module effectively from here
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(senderModule->getModuleId()));
			nlassert(it != _LocalModuleIndex.end());

			// get the sender proxy
			TModuleProxies::iterator it2(_ModuleProxies.find(it->second));
			nlassert(it2 != _ModuleProxies.end());

			IModuleProxy *senderProx = it2->second;

			// get the addressee proxy
			it2 = _ModuleProxies.find(destModuleProxyId);
			nlassert(it2 != _ModuleProxies.end());

			IModuleProxy *destProx = it2->second;


			sendModuleMessage(senderProx, destProx, message);
		}

		virtual void _broadcastModuleMessage(IModule *senderModule, const NLNET::CMessage &message)
			throw (EModuleNotPluggedHere)
		{
			H_AUTO(CModuleGetaway__broadcastModuleMessage);
			// send the message to all proxies (except the sender module)
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(senderModule->getModuleId()));
			nlassert(it != _LocalModuleIndex.end());

			TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
			for (; first != last; ++first)
			{
				IModuleProxy *proxy = first->second;

				proxy->sendModuleMessage(senderModule, message);
			}
		}

		void onModulePlugged(IModule *pluggedModule)
		{
			nldebug("NETL6: Gateway %s : plugging module '%s' id=%u",
				getModuleName().c_str(),
				pluggedModule->getModuleName().c_str(),
				pluggedModule->getModuleId());

			// A module has just been plugged here, we need to disclose it to the
			// other module, and disclose other module to it.

			// create a proxy for this module
			IModuleProxy *modProx = IModuleManager::getInstance().createModuleProxy(
					this,
					NULL,	// the module is local, so there is no route
					0,		// the module is local, distance is 0
					pluggedModule,	// the module is local, so store the module pointer
					pluggedModule->getModuleClassName(),
					pluggedModule->getModuleFullyQualifiedName(),
					pluggedModule->getModuleManifest(),
					pluggedModule->getModuleId()	// the module is local, foreign id is the module id
					);

			// and store it in the proxies container
			_ModuleProxies.insert(make_pair(modProx->getModuleProxyId(), modProx));
			_NameToProxyIdx.add(CStringMapper::map(modProx->getModuleName()), modProx);

			// and also in the local module index
			_LocalModuleIndex.insert(make_pair(pluggedModule->getModuleId(), modProx->getModuleProxyId()));


			// trigger the new module proxy event
			onAddModuleProxy(modProx);
//			// disclose the new module to other modules
//			discloseModule(modProx);
//
			// second, disclose already known proxies in the gateway to the plugged module
			{
				TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
				for (; first != last; ++first)
				{
					IModuleProxy *modProx = first->second;

					// do not send a moduleUp on the module himself !
					// either the gateway is non null (distant module), or the
					// foreign module id is different of the local module (for local proxy,
					// the foreign module id store the local module id).
					if (modProx->getGatewayRoute() != NULL || modProx->getForeignModuleId() != pluggedModule->getModuleId())
					{
						pluggedModule->_onModuleUp(modProx);
					}
				}
			}


		}
		/// Called just after a module as been effectively unplugged from a socket
		void				onModuleUnplugged(IModule *unpluggedModule)
		{
			nldebug("NETL6: Gateway %s : unplugging module '%s' id=%u",
				getModuleName().c_str(),
				unpluggedModule->getModuleName().c_str(),
				unpluggedModule->getModuleId());

			// remove the proxy info
			TLocalModuleIndex::iterator it(_LocalModuleIndex.find(unpluggedModule->getModuleId()));
			nlassert(it != _LocalModuleIndex.end());
			TModuleProxies::iterator it2(_ModuleProxies.find(it->second));
			nlassert(it2 != _ModuleProxies.end());

			IModuleProxy *modProx = it2->second;

			// warn the unplugged module that all proxies in this gateway become unavailable
			{
				TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
				for (; first != last; ++first)
				{
					IModuleProxy *modProx = first->second;

					if (modProx->getGatewayRoute() != NULL
						|| modProx->getForeignModuleId() != unpluggedModule->getModuleId())
					{
						unpluggedModule->_onModuleDown(modProx);
					}
				}
			}

			/// the gateway do the rest of the job
			onRemoveModuleProxy(modProx);

			TModuleId localProxyId = modProx->getModuleProxyId();
			// remove reference to the proxy
			_ModuleProxies.erase(it2);
			_NameToProxyIdx.removeWithB(modProx);
			_LocalModuleIndex.erase(it);

			// check in the local message queue if some message are to/from
			// this module
			TLocalMessageList::iterator first(_LocalMessages.begin()), last(_LocalMessages.end());
			for (; first != last; ++first)
			{
				TLocalMessage &lm = *first;
				if (lm.AddresseProxyId == localProxyId
					|| lm.SenderProxyId == localProxyId)
				{
					// erase this message !
					nlwarning("CStandardGateway : while unplugging module %u from the gateway, locale message '%s' from proxy %u to proxy %u is lost",
						unpluggedModule->getModuleId(),
						lm.Message.getName().c_str(),
						lm.SenderProxyId,
						lm.AddresseProxyId);
					TLocalMessageList::iterator next = first;
					++next;
					if (next == last)
					{
						_LocalMessages.erase(first);
						break;
					}
					else
					{
						_LocalMessages.erase(first);
						first = next;
					}
				}
			}


			// release the module proxy
			IModuleManager::getInstance().releaseModuleProxy(localProxyId);

		}

		////////////////////////////////////////////////////
		// Gateway internal methods
		////////////////////////////////////////////////////

		void removeForeignModule(CGatewayRoute *route, TModuleId foreignModuleId)
		{
			// translate the module id
			const TModuleId *pModuleId = route->ForeignToLocalIdx.getB(foreignModuleId);
			if (pModuleId == NULL)
			{
				// oups !
				nlwarning("removeForeignModule : unknown foreign module id %u", foreignModuleId);
				return;
			}

			TModuleId proxyId = *pModuleId;

			// retrieve the module proxy
			TModuleProxies::iterator it2(_ModuleProxies.find(proxyId));
			if (it2 == _ModuleProxies.end())
			{
				// oups !
				nlwarning("Gateway '%s' : removeForeignModule : can't find proxy for id %u coming from foreign id %u",
					getGatewayName().c_str(),
					proxyId,
					foreignModuleId);

				// still remove the idx
				route->ForeignToLocalIdx.removeWithA(foreignModuleId);
				return;
			}
			CModuleProxy *modProx = static_cast<CModuleProxy *>(it2->second.getPtr());

			// remove module information
			pair<TKnownModuleInfos::iterator, TKnownModuleInfos::iterator> range;
			range = _KnownModules.equal_range(modProx->_FullyQualifiedModuleName);
			nlassert(range.first != range.second);
			bool found = false;
			for (;range.first != range.second; ++range.first)
			{
				TKnownModuleInfo &kmi = range.first->second;

				if (kmi.Route == route)
				{
					nldebug("Gateway '%s' : removing foreign module info for '%s'",
						getGatewayName().c_str(),
						CStringMapper::unmap(range.first->first).c_str());
					// we have found the info relative to this module
					_KnownModules.erase(range.first);
					found = true;
					break;
				}
			}
			nlassert(found == true);
			// NB : stl debug mode don't allow to test with range.first when range;first is erased.
//				nlassert(range.first != range.second);

			// check if there is another view of this module
			// if so, we keep the proxy and, eventually, we update the distance
			range = _KnownModules.equal_range(modProx->_FullyQualifiedModuleName);
			if (range.first != range.second)
			{
				// clean the translation table
				route->ForeignToLocalIdx.removeWithA(foreignModuleId);

				// we keep the proxy, choose the best route
				TKnownModuleInfos::iterator best(_KnownModules.end());

				for (; range.first != range.second; ++range.first)
				{
					if (best == _KnownModules.end()
						|| best->second.ModuleDistance > range.first->second.ModuleDistance)
						best = range.first;
				}
				nlassert(best != _KnownModules.end());
				TKnownModuleInfo &kmi = best->second;

				if (modProx->_Route != kmi.Route)
				{
					// the best route has changed, update the proxy

					nldebug("Gateway '%s' : use a new route for module '%s' from %u to %u hop",
						getGatewayName().c_str(),
						modProx->getModuleName().c_str(),
						modProx->_Distance,
						kmi.ModuleDistance);

					// update the proxy data
					modProx->_Route = kmi.Route;
					modProx->_ForeignModuleId = kmi.ForeignProxyId;
					if (modProx->_Distance != kmi.ModuleDistance)
					{
						// the distance has changed, update and send the new distance to other gateway
						modProx->_Distance = kmi.ModuleDistance;
						sendModuleDistanceUpdate(modProx);
					}
				}
			}
			else
			{
				// do not remove proxy for local module from her !
				if (modProx->_Route != NULL)
				{
					// this module is no longer reachable, remove the proxy

					// trigger an event in the gateway
					onRemoveModuleProxy(modProx);

					// remove from the proxy list
					_NameToProxyIdx.removeWithB(modProx);
					_ModuleProxies.erase(it2);
					// release the proxy
					IModuleManager::getInstance().releaseModuleProxy(proxyId);
				}
				// clean the translation table
				route->ForeignToLocalIdx.removeWithA(foreignModuleId);
			}
		}

		void sendModuleDistanceUpdate(IModuleProxy *proxy)
		{
			// in fact, don't send immediately, store update in each
			// route and wait the next update or module message sending
			// to effectively send the update

			// for each route
			TRouteList::iterator first(_Routes.begin()), last(_Routes.end());
			for (; first != last; ++first)
			{
				CGatewayRoute *route = *first;
				if (isModuleProxyVisible(proxy, route))
				{
					updateModuleDistanceToRoute(route, proxy);
//					// TODO : optimize by batch sending
//					TModuleDistanceChangeMsg mdu;
//
//					mdu.ModuleId = proxy->getModuleProxyId();
//					mdu.NewDistance = proxy->getModuleDistance()+1;
//
//					CMessage msg("MOD_DST_UPD");
//					msg.serial(mdu);
//
//					sendPendingModuleUpdate(route);
//					route->sendMessage(msg);
				}
			}
		}

		/// Check if a module can be seen by a route
		bool isModuleProxyVisible(IModuleProxy *proxy, CGatewayRoute *route)
		{
			if (route == NULL)
			{
				// no route, we can see the proxy
				return true;
			}
			// check firewall rules
			if (route->getTransport()->Firewalled)
			{
				if (route->FirewallDisclosed.find(proxy->getModuleProxyId()) == route->FirewallDisclosed.end())
					return false;
			}

			// if the module is local, then, it can be seen
			if (proxy->getGatewayRoute() == NULL)
				return true;

			// if the module is on the same route, it can't be seen (it is seen by the route outbound)
			if (proxy->getGatewayRoute() == route)
				return false;

			IGatewayTransport *transport = route->getTransport();
			// if the module is on a different transport, it can be seen
			if (proxy->getGatewayRoute()->getTransport() != transport)
			{
				// we also need to check if this module is known in this route
//				CGatewayRoute::TForeignToLocalIdx::iterator it(route->ForeignToLocalIdx.find(proxy->getForeignModuleId()));
				if (route->ForeignToLocalIdx.getA(proxy->getModuleProxyId()) != NULL)
					// this module is known in this route, so not invisible
					return false;

				// ok, we can see
				return true;
			}

			// if the transport in not in peer invisible, it can be seen
			if (!transport->PeerInvisible)
				return true;

			// not visible
			return false;
		}

		/// Disclose module information to a gateway route
		void discloseModuleToRoute(CGatewayRoute *route, IModuleProxy *proxy)
		{
//			route->PendingUndisclosure.erase(proxy->getModuleProxyId());
			CGatewayRoute::TPendingEvent pe;
			pe.EventType = CGatewayRoute::pet_disclose_module;
			pe.ModuleId = proxy->getModuleProxyId();
			route->PendingEvents.push_back(pe);
//			route->PendingDisclosure.insert(proxy);
		}
		/// Undisclose module information to a gateway route
		void undiscloseModuleToRoute(CGatewayRoute *route, IModuleProxy *proxy)
		{
//			route->PendingDisclosure.erase(proxy);
//			route->PendingUndisclosure.insert(proxy->getModuleProxyId());
//			route->FirewallDisclosed.erase(proxy->getModuleProxyId());

			CGatewayRoute::TPendingEvent pe;
			pe.EventType = CGatewayRoute::pet_undisclose_module;
			pe.ModuleId = proxy->getModuleProxyId();
			route->PendingEvents.push_back(pe);

			route->FirewallDisclosed.erase(proxy->getModuleProxyId());
		}

		/// the distance of a module need to be update to peers
		void updateModuleDistanceToRoute(CGatewayRoute *route, IModuleProxy *proxy)
		{
			CGatewayRoute::TPendingEvent pe;
			pe.EventType = CGatewayRoute::pet_update_distance;
			pe.ModuleId = proxy->getModuleProxyId();
			route->PendingEvents.push_back(pe);
		}
		/// The security data need to be updated to peers
		void updateModuleSecurityDataToRoute(CGatewayRoute *route, IModuleProxy *proxy)
		{
			CGatewayRoute::TPendingEvent pe;
			pe.EventType = CGatewayRoute::pet_update_security;
			pe.ModuleId = proxy->getModuleProxyId();
			route->PendingEvents.push_back(pe);
		}

		void sendPendingModuleUpdate(CGatewayRoute *route)
		{
			if (route->PendingEvents.empty())
				return;

			CMessage updateMsg("MOD_UPD");

			// compil all update in a single message
			while (!route->PendingEvents.empty())
			{
				CGatewayRoute::TPendingEvent &pe = route->PendingEvents.front();
				switch (pe.EventType)
				{
				case CGatewayRoute::pet_disclose_module:
					{
						IModuleProxy *proxy = getModuleProxy(pe.ModuleId);
						if (proxy == NULL)
							break;

						// store the update type
						updateMsg.serialShortEnum(pe.EventType);

						// encode the message data
						TModuleDescCodec modDesc(proxy);
						updateMsg.serial(modDesc);
//						modDesc.encode(proxy, updateMsg);
					}

					break;
				case CGatewayRoute::pet_undisclose_module:
					{
						// store the update type
						updateMsg.serialShortEnum(pe.EventType);

						// store the module id
						updateMsg.serial(pe.ModuleId);
					}

					break;
				case CGatewayRoute::pet_update_distance:
					{
						IModuleProxy *proxy = getModuleProxy(pe.ModuleId);
						if (proxy == NULL)
							break;

						// store the update type
						updateMsg.serialShortEnum(pe.EventType);

						// store module ID and distance
						updateMsg.serial(pe.ModuleId);
						uint32 distance = proxy->getModuleDistance()+1;
						updateMsg.serial(distance);
					}
					break;
				case CGatewayRoute::pet_update_security:
					{
						IModuleProxy *proxy = getModuleProxy(pe.ModuleId);
						if (proxy == NULL)
							break;

						// store the update type
						updateMsg.serialShortEnum(pe.EventType);

						// store module ID and security data
						TModuleSecurityChangeMsg secChg;
						secChg.ModuleId = pe.ModuleId;
						secChg.SecDesc.SecurityData = const_cast<TSecurityData*>(proxy->getFirstSecurityData());
						updateMsg.serial(secChg);
//						updateMsg.serial(pe.ModuleId);
//						TSecurityData *modSec = const_cast<TSecurityData*>(proxy->getFirstSecurityData());
//						updateMsg.serialPolyPtr(modSec);
					}
					break;
				default:
					// should not append
					nlstop;
				}

				route->PendingEvents.pop_front();
			}

			// now send the message
			route->sendMessage(updateMsg);

//			// send pending module proxy un/disclosure
//			if (!route->PendingDisclosure.empty())
//			{
//				// disclose new module
//				TModuleAddMsg message;
//				message.Modules.resize(route->PendingDisclosure.size());
//
//				std::set<IModuleProxy*>::iterator first(route->PendingDisclosure.begin()), last(route->PendingDisclosure.end());
//				for (uint i=0; first != last; ++i, ++first)
//				{
//					TModuleDescMsg &modDesc = message.Modules[i];
//					IModuleProxy *addedModule = *first;
//
//					modDesc.ModuleProxyId = addedModule->getModuleProxyId();
//					modDesc.ModuleClass	 = addedModule->getModuleClassName();
//					modDesc.ModuleFullName = addedModule->getModuleName();
//					modDesc.ModuleDistance = addedModule->getModuleDistance()+1;
//				}
//				route->PendingDisclosure.clear();
//
//				CMessage buffer("MOD_ADD");
//				buffer.serial(message);
//
//				route->sendMessage(buffer);
//			}
//			if (!route->PendingUndisclosure.empty())
//			{
//				// disclose new module
//				TModuleRemMsg message;
//				std::copy(route->PendingUndisclosure.begin(), route->PendingUndisclosure.end(), back_insert_iterator<vector<TModuleId> >(message.RemovedModules));
//				route->PendingUndisclosure.clear();
//
//				CMessage buffer("MOD_REM");
//				buffer.serial(message);
//
//				route->sendMessage(buffer);
//			}
		}

		void getModuleList(std::vector<IModuleProxy*> &resultList)
		{
			TModuleProxies::iterator first(_ModuleProxies.begin()), last(_ModuleProxies.end());
			for (; first != last; ++first)
			{
				resultList.push_back(first->second);
			}
		}


		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CStandardGateway, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, dump, "dump various information about the gateway statue", "")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportListAvailableClass, "list the available transport class", "no param")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportAdd, "add a new transport to this gateway", "<transportClass> <instanceName>")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportOptions, "set a gateway level option on a transport", "<transportClass> ( [PeerInvisible] [Firewalled] )")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportCmd, "send a command to a transport", "[<transportName> ( <cmd specific to transport> )]*")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, transportRemove, "remove an existing transport instance", "<transportName>")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, securityListAvailableClass, "list the available security class", "no param")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, securityCreate, "create a security plug-in", "<securityClassName>")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, securityCommand, "send a command to the security plug-in", "<cmd specific to plug-in>")
			NLMISC_COMMAND_HANDLER_ADD(CStandardGateway, securityRemove, "remove the security plug-in", "no parameter")
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(securityRemove)
		{
			nlunreferenced(rawCommandString);
			nlunreferenced(args);
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (_SecurityPlugin == NULL)
			{
				log.displayNL("No security plug-in !");
				return true;
			}

			removeSecurityPlugin();

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(securityCommand)
		{
			nlunreferenced(args);
			nlunreferenced(quiet);
			nlunreferenced(human);

			TParsedCommandLine command;

			if (!command.parseParamList(rawCommandString))
			{
				log.displayNL("Invalid command line");
				return false;
			}

			if (command.SubParams.size() < 2)
			{
				log.displayNL("Invalid command line");
				return false;
			}

			if (_SecurityPlugin == NULL)
			{
				log.displayNL("No security plug-in !");
				return true;
			}

			sendSecurityCommand(*command.SubParams[1]);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(securityCreate)
		{
			nlunreferenced(rawCommandString);
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (args.size() != 1)
				return false;

			if (_SecurityPlugin != NULL)
			{
				log.displayNL("The gateway already have a security plug-in ! Remove it first");
				return true;
			}

			log.displayNL("Creating a security plug-in '%s' in gateway '%s'",
				args[0].c_str(),
				getModuleName().c_str());
			createSecurityPlugin(args[0]);

			return true;

		}
		NLMISC_CLASS_COMMAND_DECL(securityListAvailableClass)
		{
			nlunreferenced(rawCommandString);
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (args.size() != 0)
				return false;

			vector<string> list;
			NLMISC_GET_FACTORY(CGatewaySecurity, std::string).fillFactoryList(list);

			log.displayNL("List of %u available security class :", list.size());

			for (uint i=0; i<list.size(); ++i)
			{
				log.displayNL("   '%s'", list[i].c_str());
			}

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportRemove)
		{
			nlunreferenced(rawCommandString);
			nlunreferenced(log);
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (args.size() != 1)
				return false;

			deleteTransport(args[0]);
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportCmd)
		{
			nlunreferenced(args);
			nlunreferenced(quiet);
			nlunreferenced(human);

			TParsedCommandLine pcl;
			if (!pcl.parseParamList(rawCommandString))
			{
				log.displayNL("Invalid parameter string, parse error");
				return false;
			}

			transportCommand(pcl);
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportOptions)
		{
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (args.size() < 1)
				return false;

			// parse the params
			TParsedCommandLine cl;
			if (!cl.parseParamList(rawCommandString))
				return false;

			if (cl.SubParams.size() != 2)
				return false;

			string transName = cl.SubParams[1]->ParamName;
			if (_Transports.find(transName) == _Transports.end())
			{
				log.displayNL("unknown transport '%s'", transName.c_str());
				return false;
			}

//			IGatewayTransport *transport = _Transports.find(transName)->second;

			// check for peer invisible
			if (cl.SubParams[1]->getParam("PeerInvisible"))
				setTransportPeerInvisible(transName, true);
			else
				setTransportPeerInvisible(transName, false);

			// check for firewall mode
			if (cl.SubParams[1]->getParam("Firewalled"))
				setTransportFirewallMode(transName, true);
			else
				setTransportFirewallMode(transName, false);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportAdd)
		{
			nlunreferenced(rawCommandString);
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (args.size() != 2)
				return false;

			if (_Transports.find(args[1]) != _Transports.end())
			{
				log.displayNL("A transport with that name already exist !");
				return true;
			}

			createTransport(args[0], args[1]);
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(transportListAvailableClass)
		{
			nlunreferenced(rawCommandString);
			nlunreferenced(quiet);
			nlunreferenced(human);

			if (args.size() != 0)
				return false;

			vector<string> list;
			NLMISC_GET_FACTORY(IGatewayTransport, std::string).fillFactoryList(list);

			log.displayNL("List of %u available transport class :", list.size());

			for (uint i=0; i<list.size(); ++i)
			{
				log.displayNL("   '%s'", list[i].c_str());
			}
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			if (!args.empty())
				return false;

			// recall the dump for the module class
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			log.displayNL("-----------------------------");
			log.displayNL("Dumping gateway information :");
			log.displayNL("-----------------------------");

			log.displayNL("The gateway has %u locally plugged module :", _PluggedModules.getAToBMap().size());
			{
				TPluggedModules::TAToBMap::const_iterator first(_PluggedModules.getAToBMap().begin()), last(_PluggedModules.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;
					log.displayNL("    ID:%5u : \tName = '%s' \tclass = '%s'",
						module->getModuleId(),
						module->getModuleName().c_str(),
						module->getModuleClassName().c_str());
				}
			}


			log.displayNL("The gateway as %u transport activated :", _Transports.size());
			{
				TTransportList::iterator first(_Transports.begin()), last(_Transports.end());
				for (; first != last; ++first)
				{
					const string &name = first->first;
					IGatewayTransport *transport = first->second;

					log.displayNL("Transport '%s' (transport class is '%s') :",
						name.c_str(),
						transport->getClassName().c_str());
					log.displayNL("  * %s", transport->PeerInvisible ? "Peer module are NON visible" : "Peer modules are visible");
					log.displayNL("  * %s", transport->Firewalled ? "Firewall ON" : "Firewall OFF");
					transport->dump(log);
				}
			}

			log.displayNL("------------------------------");
			log.displayNL("------- End of dump ----------");
			log.displayNL("------------------------------");
			return true;
		}

	};


	// register the module factory
	NLNET_REGISTER_MODULE_FACTORY(CStandardGateway, "StandardGateway");


	/** Set a security data block. If a bloc of the same type
	 *	already exist in the list, the new one will replace the
	 *	existing one.
	 */
	void CGatewaySecurity::setSecurityData(IModuleProxy *proxy, TSecurityData *securityData)
	{
		// forward the call to standard gateway
		CStandardGateway *sg = static_cast<CStandardGateway*>(_Gateway);
		sg->setSecurityData(proxy, securityData);
	}

	/** Clear a block of security data
	 *	The block is identified by the data tag
	 */
	bool CGatewaySecurity::removeSecurityData(IModuleProxy *proxy, uint8 dataTag)
	{
		// forward the call to standard gateway
		CStandardGateway *sg = static_cast<CStandardGateway*>(_Gateway);
		return sg->removeSecurityData(proxy, dataTag);
	}

	/** Replace the complete set of security data with the new one.
	 *	Security data allocated on the proxy are freed,
	 */
	void CGatewaySecurity::replaceAllSecurityDatas(IModuleProxy *proxy, TSecurityData *securityData)
	{
		// forward the call to standard gateway
		CStandardGateway *sg = static_cast<CStandardGateway*>(_Gateway);
		sg->replaceAllSecurityDatas(proxy, securityData);
	}

	/** Ask the gateway to resend the security data.
	 *	The plug-in call this method after having changed
	 *	the security info for a plug-in outside of the
	 *	onNewProxy call.
	 */
	void CGatewaySecurity::forceSecurityUpdate(IModuleProxy *proxy)
	{
		// forward the call to standard gateway
		CStandardGateway *sg = static_cast<CStandardGateway*>(_Gateway);
		sg->forceSecurityUpdate(proxy);
	}



	void forceGatewayLink()
	{
	}

} // namespace NLNET

