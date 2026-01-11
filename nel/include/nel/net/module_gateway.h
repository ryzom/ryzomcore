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


#ifndef NL_FILE_MODULE_GATEWAY_H
#define NL_FILE_MODULE_GATEWAY_H

#include "nel/misc/twin_map.h"
#include "module_common.h"
#include "inet_address.h"
#include "message.h"
#include "module_message.h"

namespace NLNET
{
	class IGatewayTransport;
	class CGatewayRoute;
	class CGatewaySecurity;

	/** Interface for gateway.
	 *	A gateway is the part of the module layer that interconnect
	 *	module locally and give access to foreign module hosted in
	 *	other process.
	 *	Gateway can interconnect local module with themselves, as well as
	 *	connect with another gateway in another process or host.
	 *
	 *	Transport:
	 *	---------
	 *	Gateway connectivity is provided by 'transport' that can
	 *	be build on any support.
	 *	There are available transport for NeL Layer 3 in server
	 *	of client mode.
	 *
	 *	You can bind at run time, any number of transport to a
	 *	gateway.
	 *	Each of these transport can then receive specific command
	 *	from passed by the gateway.
	 *	Transport then create routes that are active connection
	 *	to foreign gateways.
	 *
	 *	When using the layer 3 transport, you can choose either
	 *	to instantiate a client mode transport or a server mode
	 *	transport.
	 *	In client mode, you can connect to one or more server, each
	 *	connection generating a new route.
	 *	In server mode, you can put your transport as 'open' on
	 *	a specified TCP port, then each client connection will
	 *	generate a new route.
	 *
	 *	These layer 3 transport are only one type of transport,
	 *	it it possible to build any type of transport (on raw TCP
	 *	socket, or over UDP, or using named pipe...).
	 *
	 *	Unique name generation :
	 *	------------------------
	 *	Gateway need to generate 'fully qualified module name' witch
	 *	must be unique in any gateway interconnection scheme.
	 *	By default, gateway use the following scheme to build
	 *	unique module name :
	 *		<hostname>:<pid>:<modulename>
	 *	In some case, you could not be sure that the host name will
	 *	be unique (think about your hundreds simultaneously connected
	 *	clients, some of them can have set the same name for their
	 *	computer !). In this case, you can set register a unique name
	 *	generator on you gateway that use another naming scheme.
	 *	For example, you can use you customer unique ID (with the
	 *	restriction that one ID can be in use once at the same time)
	 *	to build unique name :
	 *		<customerId>:<modulename>
	 *
	 *	Advanced transport options (i.e option settable on each transport):
	 *	--------------------------
	 *	- peer invisible : if activated on a transport, this option
	 *		will mask the modules of the other routes of the same
	 *		transport.
	 *		This is useful for players modules (you sure don't want
	 *		that all players can see all other clients modules),
	 *		or for client/server structure where you don't want
	 *		all client to see all module, bu only those that are
	 *		available from the client side.
	 *		Note that any module that come from another transport
	 *		will be disclosed to all the route.
	 *
	 *	- firewalled : if activated, this option will mask any module
	 *		to the connected unsecure route unless some module comming from another
	 *		transport (or a local module) will try to send a message
	 *		to a module on an unsecure route. In this case, the gateway
	 *		will disclose the information about this sender module (and only
	 *		this one) to the unsecure route.
	 *		In firewall mode, the gateway keep a list of disclosed module
	 *		for each route and stop any message that is addressed to an
	 *		undisclosed module (that can be considered as a hacking attempt).
	 *		Module that are disclosed by the unsecure routes are normaly
	 *		seen by all other module.
	 *
	 *	The two options above can be used in combination. This is a prefered
	 *	way of configuring transport for player connection : we don't want
	 *	player to see other player modules, and we don't want player to see
	 *	any server side module until one of them started a communication
	 *	with a player module.
	 */
	class IModuleGateway : public NLMISC::CRefCount
	{
	public:
		// Exception classes ===================================

		/// The gateway is already open while trying to open it.
		class EGatewayAlreadyOpen : public NLMISC::Exception
		{
		};

		/// When trying to open the gateway server, the TCP port is already in use
		class EGatewayPortInUse : public NLMISC::Exception
		{
		};

		/// The gateway is not open while trying to close it.
		class EGatewayNotOpen : public NLMISC::Exception
		{
		};

		/// A gateway is not connected while trying to communicate with
		class EGatewayNotConnected : public NLMISC::Exception
		{
		};

		/// Firewall mode is activated but there is already open route !
		class EGatewayFirewallBreak : public NLMISC::Exception
		{
		};

		virtual ~IModuleGateway() {}

		/** Register the gateway in the module manager gateway registry
		 */
		virtual void registerGateway() =0;
		/** Unregister the gateway in the module manager gateway registry
		 */
		virtual void unregisterGateway() =0;

		//@{
		//@name Gateway general information and control
		/// Return the local name of the gateway
		virtual const std::string &getGatewayName() const =0;
		/// Return the Fully Qualified Gateway Name (FQGN)
		virtual const std::string &getFullyQualifiedGatewayName() const =0;
		//@}

		//@{
		//@name Gateway transport management

		/// Create and bind to this gateway a new transport
		virtual void	createTransport(const std::string &transportClass, const std::string &instanceName) =0;
		/// Delete a transport (this will close any open route)
		virtual void	deleteTransport(const std::string &transportInstanceName) =0;

		/// Activate/stop peer invisible mode on a transport
		virtual void	setTransportPeerInvisible(const std::string &transportInstanceName, bool peerInvisible) =0;

		/// Activate/stop firewalling mode on a transport
		virtual void	setTransportFirewallMode(const std::string &transportInstanceName, bool firewalled) =0;

		/// Send a command to a transport
		virtual void	transportCommand(const TParsedCommandLine &commandLine) =0;

		/// Return a pointer on the named transport interface, or NULL if the transport is unknown.
		virtual IGatewayTransport *getGatewayTransport(const std::string &transportName) const =0;

		/// Return the number of transport currently active on this gateway
		virtual uint32	getTransportCount() const =0;

		/// Return the number route available
		virtual uint32	getRouteCount() const =0;

		/// Return the number of ping received. This is incremented by special "GW_PING" message for unit testing
		virtual uint32	getReceivedPingCount() const =0;
		//@}

		//@{
		//@name Gateway transport callback
		/// A new route a added by a transport
		virtual void onRouteAdded(CGatewayRoute *route) =0;

		/// A route is removed by a transport
		virtual void onRouteRemoved(CGatewayRoute *route) =0;

		/// A transport have received a message
		virtual void onReceiveMessage(CGatewayRoute *from, const CMessage &msgin) =0;
		//@}

		//@{
		//@name Gateway security plug-in management
		/** create a security plug-in.
		 *	There must be no security plug-in currently created.
		 */
		virtual void createSecurityPlugin(const std::string &className) = 0;
		/** Send a command to the security plug-in */
		virtual void sendSecurityCommand(const TParsedCommandLine &command) =0;
		/** Remove the security plug-in.
		 */
		virtual void removeSecurityPlugin() = 0;
		//@}

		//@{
		//@name Module management
		/** Callback called when the gateway has received some new module
		 *	and eventually, need to disclose the module information to
		 *	the connected gateway.
		 *	The default behavior is to disclose the module to all
		 *	connected gateway.
		 */
		virtual void onAddModuleProxy(IModuleProxy *addedModule) =0;
		/** Callback called when a module become unavailable, either
		 *	because it is unplugged from it's socket, or, the
		 *	gateway that disclosed it has been disconnected.
		 */
		virtual void onRemoveModuleProxy(IModuleProxy *removedModule) =0;

		/** Disclose module information to a connected gateway.
		 *	This can also be this gateway itself.
		 */
		virtual void discloseModule(IModuleProxy *moduleProxy) =0;

		/** Retrieve the proxy for a locally plugged module.
		 *	Each local module plugged in a gateway has an associated
		 *	proxy. This method return this proxy or NULL if the
		 *	module is not plugged here.
		 */
		virtual IModuleProxy *getPluggedModuleProxy(IModule *pluggedModule) =0;

		/// Return the number of proxies managed by this gateway
		virtual uint32	getProxyCount() const =0;

		/// Fill a vector with the list of proxies managed here. The module are filled in ascending proxy id order.
		virtual void	getModuleProxyList(std::vector<IModuleProxy*> &resultList) const =0;
		//@}

		//@{
		//@name Module messaging
		/** Callback called when a message arrive from a gateway and need
		 *	to be dispatched.
		 *	The default behavior is to route the message in any case to
		 *	the destination module.
		 *	You can override this callback to add some message filtering
		 *	or hacking feature.
		 */
//		virtual void onReceiveModuleMessage(TModuleGatewayProxyPtr &senderGateway, TModuleMessagePtr &message) =0;

		/** Send a message to a module.
		 */
		virtual void sendModuleProxyMessage(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, const NLNET::CMessage &message) =0;

		/** Send a message to the module plugged in this gateway.
		 *	You can override this method to change the dispatching, add filtering,
		 *	message hacking or interceptor.
		 */
		virtual void dispatchModuleMessage(IModuleProxy *senderProxy, IModuleProxy *addresseeProxy, const CMessage &message) =0;
		//@}
	};

	/** Intermediate class must be used as base class
	 *	for implementing gateway.
	 */
	class CModuleGateway : public IModuleGateway
	{
	protected:
		/** Register the gateway in the module manager gateway registry
		 */
		virtual void registerGateway();
		/** Unregister the gateway in the module manager gateway registry
		 */
		virtual void unregisterGateway();
	};

	/** Interface class for gateway transport.
	 *	A gateway transport is an object associated to a standard gateway
	 *	at run time and that provide a mean to interconnect with
	 *	other gateway.
	 *	As each transport mode as it's own command requirement,
	 *	a generic command system is provided for sending command message
	 *	to the transport implementation.
	 *
	 *	At time of writing, NeL come with 2 transport : one based on layer 3 client, and one
	 *	based on layer 3 server. In a short time, there will be transport using layer 5.
	 */
	class IGatewayTransport
	{
	protected:
		/// Back pointer to the gateway hosting this transport
		IModuleGateway			*_Gateway;
	public:
		/// Invalid transport command
		class EInvalidCommand : public NLMISC::Exception
		{
		public:
			EInvalidCommand() {}
			EInvalidCommand(const char *err) : Exception(err) {}
		};

		/// Error in the transport
		class ETransportError : public NLMISC::Exception
		{
		public:
			ETransportError(const char *err) : Exception(err) {}
		};

		//////////////// property used by gateway (not managed by transport)
		/// Flag for firewall mode
		bool				Firewalled;
		/// flag for peer invisible mode
		bool				PeerInvisible;

		/// Constructor param needed by the factory (see nel/misc/factory.h)
		struct TCtorParam
		{
			IModuleGateway *Gateway;
		};

		/// Constructor, establish link with the associated gateway
		IGatewayTransport(const TCtorParam &param)
			: Firewalled(false),
			PeerInvisible(false)
		{
			_Gateway = param.Gateway;
		}

		virtual ~IGatewayTransport() {}

		/// Return the class name from the transport factory
		virtual const std::string &getClassName() const =0;

		/// The gateway send a command message to the transport
		virtual void onCommand(const CMessage &command) = 0;
		/// The gateway send a textual command to the transport
		virtual bool onCommand(const TParsedCommandLine &command) = 0;

		/// The gateway update the transport regularly
		virtual void update() =0;

		/// Return the number of route currently open by the transport
		virtual uint32 getRouteCount() const =0;

		/// Dump debug information in the specified log stream
		virtual void dump(NLMISC::CLog &log) const =0;
	};

	/** Base class for gateway route.
	 *	Route are provided by transport.
	 *	Transport provide a mean to build route
	 *	between gateway.
	 *	Route show the list of foreign gateway that are
	 *	reachable with it and are use to send
	 *	message to these gateways.
	 *
	 *	The route store proxy id translation table, i.e,
	 *	for each module proxy that come from this route
	 *	we store association of the local proxy ID with
	 *	the foreign proxy ID, that is the proxy that
	 *	represent the module at the outbound of the route.
	 *
	 *	Note that even if the route object is created
	 *	by the transport, the translation table is
	 *	feed and managed by the gateway implementation.
	 */
	class CGatewayRoute
	{
	protected:
#ifdef NL_DEBUG
		/// A debug flag that trigger an assert if true and something delete this object.
		mutable bool		_AssertOnDelete;
		friend struct		CAutoAssertSetter;
#endif
		/// The transport that manage this route
		IGatewayTransport	*_Transport;
	public:
		/// The local foreign(A) <=> local(B) proxy id translation table
//		typedef std::map<TModuleId, TModuleId>	TForeignToLocalIdx;
		typedef NLMISC::CTwinMap<TModuleId, TModuleId>	TForeignToLocalIdx;
		TForeignToLocalIdx	ForeignToLocalIdx;


		enum TPendingEventType
		{
			pet_disclose_module,
			pet_undisclose_module,
			pet_update_distance,
			pet_update_security,
		};

		struct TPendingEvent
		{
			TPendingEventType	EventType;
//			IModuleProxy		*ModuleProxy;
			TModuleId			ModuleId;
		};
		/// A list of pending event on this route
		std::list<TPendingEvent>	PendingEvents;

		/// A list of module proxy pending disclosure
//		std::set<IModuleProxy*>	PendingDisclosure;
		/// A list of module proxy pending undisclosure
//		std::set<TModuleId>		PendingUndisclosure;
		/// firewall disclosed module (empty in not firewalled mode)
		std::set<TModuleId>		FirewallDisclosed;

		//@{
		/// @name Information on the next module message to dispatch

		/// next message type, set to CModuleMessageHeaderCodec::mt_invalid when no module message are awaited
		CModuleMessageHeaderCodec::TMessageType	NextMessageType;
		/// Id of the sender proxy
		TModuleId		NextSenderProxyId;
		/// Id of the addressee proxy
		TModuleId		NextAddresseeProxyId;
		//@}

		/// constructor, must provide the transport
		CGatewayRoute(IGatewayTransport *transport)
			:	_Transport(transport),
				NextMessageType(CModuleMessageHeaderCodec::mt_invalid)
		{
#ifdef NL_DEBUG
			_AssertOnDelete = false;
#endif
		}

		virtual ~CGatewayRoute()
		{
#ifdef NL_DEBUG
			nlassert(!_AssertOnDelete);
#endif
		}

		/// Return the transport that hold this route
		IGatewayTransport *getTransport() { return _Transport; };
		/// Send a message via the route
		virtual void sendMessage(const CMessage &message) const =0;
	};

#ifdef NL_DEBUG
	struct CAutoAssertSetter
	{
		const CGatewayRoute	&GatewayRoute;
		CAutoAssertSetter(const CGatewayRoute &gwr)
			:	GatewayRoute(gwr)
		{
			GatewayRoute._AssertOnDelete = true;
		}

		~CAutoAssertSetter()
		{
			GatewayRoute._AssertOnDelete = false;
		}
	};

 #define NLNET_AUTO_DELTE_ASSERT	CAutoAssertSetter __autoDeleteAssert(static_cast<const CGatewayRoute&>(*this))
#else
 #define NLNET_AUTO_DELTE_ASSERT
#endif


	class CGatewaySecurity
	{
	protected:
		IModuleGateway	*_Gateway;
	public:
		struct TCtorParam
		{
			IModuleGateway	*Gateway;
		};

		CGatewaySecurity (const TCtorParam &params)
			: _Gateway(params.Gateway)
		{
		}

		virtual ~CGatewaySecurity() { }

		/** the gateway send a command to the security module */
		virtual void onCommand(const TParsedCommandLine &/* command */)	{}

		/** A new proxy is available, the security plug-in can add security data */
		virtual void onNewProxy(IModuleProxy *proxy) =0;

		/** A proxy receive new security datas, the security plug-in can
		 *	check the data validity, and eventually, reject the one or mode data block
		 *	then it must update the proxy security with the new security data.
		 *	If the security plug-in reject one or more security data, it has
		 *	the duty for deleting the rejected object.
		 *	Furthermore, it must free the existing security data on the proxy
		 *	if is replace them with the new one.
		 *	Any other combination is fine unless the security plug-in do a correct
		 *	management of freeing unused security data.
		 */
		virtual void onNewSecurityData(CGatewayRoute *from, IModuleProxy *proxy, TSecurityData *firstSecurityData) = 0;

		/** Called just before delete, the security plug-in must
		 *	remove any security data that it added to the proxies.
		 */
		virtual void onDelete() =0;

		/** Set a security data block. If a bloc of the same type
		 *	already exist in the list, the new one will replace the
		 *	existing one.
		 */
		void setSecurityData(IModuleProxy *proxy, TSecurityData *securityData);

		/** Clear a block of security data
		 *	The block is identified by the data tag
		 *	Return true if at least one item have been removed, false otherwise.
		 */
		bool removeSecurityData(IModuleProxy *proxy, uint8 dataTag);

		/** Replace the complete set of security data with the new one.
		 *	Security data allocated on the proxy are freed,
		 */
		void replaceAllSecurityDatas(IModuleProxy *proxy, TSecurityData *securityData);

		/** Ask the gateway to resend the security data.
		 *	The plug-in call this method after having changed
		 *	the security info for a plug-in outside of the
		 *	onNewProxy call.
		 */
		void forceSecurityUpdate(IModuleProxy *proxy);
	};


} // namespace NLNET


#endif // NL_FILE_MODULE_GATEWAY_H

