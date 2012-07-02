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
// client include
#if !defined(RZ_CLIENT_DRONE)
#  include "net_manager.h"
#  include "network_connection.h"
#else
#  include "../client_drone/network_connection.h"
#  include "../client_drone/simulated_client.h"
#  include "../game_share/generic_xml_msg_mngr.h"
#endif

using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CGenericXmlMsgHeaderManager GenericMsgHeaderMngr;

/** the specialized route for frontend client transport */
class CFEClientRoute : public CGatewayRoute
{
public:

	// the network connection to use to send data
	CNetworkConnection	*NetConn;


	CFEClientRoute(IGatewayTransport *transport, CNetworkConnection	*netConn)
		: CGatewayRoute(transport),
		NetConn(netConn)
	{
		// warn the server that the transport is open
		CBitMemStream bms;
		GenericMsgHeaderMngr.pushNameToStream( "MODULE_GATEWAY:FEOPEN", bms );

		sendRawMessage(bms);
	}

	~CFEClientRoute()
	{
		// warn the server that the transport is closed
		CBitMemStream bms;
		GenericMsgHeaderMngr.pushNameToStream( "MODULE_GATEWAY:FECLOSE", bms );

		sendRawMessage(bms);
	}


	void sendMessage(const CMessage &message) const
	{
		// wrap the message in a transport message
		CBitMemStream wrapper;
		GenericMsgHeaderMngr.pushNameToStream( "MODULE_GATEWAY:GATEWAY_MSG", wrapper );
//		wrapper.serial(message.getName());
//		wrapper.serialBufferWithSize(const_cast<uint8*>(message.buffer())+message.getHeaderSize(), message.length()-message.getHeaderSize());
		wrapper.serialBufferWithSize(const_cast<uint8*>(message.buffer()), message.length());

		sendRawMessage(wrapper);
	}

	// send a bit mem stream to the server
	void sendRawMessage(CBitMemStream &message) const
	{
		NetConn->push(message);
	}
};




#define FE_CLIENT_CLASS_NAME "FEClient"
/** Transport for module gateway through front end service, client part. */
class CGatewayFEClientTransport : public IGatewayTransport
{
	friend class CFEClientRoute;
public:
	/// Invalid command
	class EInvalidCommand : public NLMISC::Exception
	{
	};

	/// The established route if any
	CFEClientRoute	*_Route;

	/// Status of the transport.
	bool			_Open;
	/// A flag that is false after opening the transport until we receive one message
	bool			_FirstMessageReceived;

	enum TMessageType
	{
		mt_route_open,
		mt_route_close,
		mt_gw_msg,
	};
	/// Storage for message pending because or bad ordering
	struct TWaitingMessage
	{
		TMessageType	MessageType;
		CBitMemStream	Message;
	};

	typedef map<uint8, TWaitingMessage>	TWaitingMessages;
	/// The list of message waiting because or bad ordering
	TWaitingMessages	_WaitingMessages;

	/// The message serial number that we are waiting to dispatch
	uint8				_NextAwaitedMessage;

#if !defined(RZ_CLIENT_DRONE)
	// store the unique active transport (only one transport of this type can be activated at a time)
	static CGatewayFEClientTransport *&OpenTransport()
	{
		static CGatewayFEClientTransport *openTransport = NULL;

		return openTransport;
	}
#else
	CSimulatedClient	*SimClient;
#endif

	/// Constructor
	CGatewayFEClientTransport(const IGatewayTransport::TCtorParam &param)
		: IGatewayTransport(param),
		_Route(NULL),
		_Open(false),
		_FirstMessageReceived(false),
		_NextAwaitedMessage(0)
	{
#if defined(RZ_CLIENT_DRONE)
		SimClient = CSimulatedClient::currentContext();
#endif
	}

	~CGatewayFEClientTransport()
	{
#if !defined(RZ_CLIENT_DRONE)
		if (OpenTransport() == this)
		{
			// the transport is still open, close it before destruction
			close();
		}
#else
		// close anyway
		close();
#endif
	}

	const std::string &getClassName() const
	{
		static string className(FE_CLIENT_CLASS_NAME);
		return className;
	}

	virtual void update()
	{
	}

	virtual uint32 getRouteCount() const
	{
		return _Route != NULL ? 1 : 0;
	}

	void dump(NLMISC::CLog &log) const
	{
		IModuleManager &mm = IModuleManager::getInstance();
		log.displayNL("  Frontend service transport, client part");
		if (!_Open)
		{
			log.displayNL("  The connection is currently closed.");
		}
		else
		{
			log.displayNL("  The connection is open :");

			if (_Route == NULL)
			{
				log.displayNL("    There is no route.");
			}
			else
			{
				CFEClientRoute *route = _Route;
				log.displayNL("    The route has %u entries in the proxy translation table :",
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

	void onCommand(const CMessage &/* command */) throw (IGatewayTransport::EInvalidCommand)
	{
		// nothing done for now
		throw EInvalidCommand();
	}
	/// The gateway send a textual command to the transport
	bool onCommand(const TParsedCommandLine &command) throw (IGatewayTransport::EInvalidCommand)
	{
		if (command.SubParams.size() < 1)
			throw  EInvalidCommand();

		const std::string &commandName = command.SubParams[0]->ParamName;
		if (commandName == "open")
		{
			if (_Open)
			{
				nlwarning("The transport is already open");
				return false;
			}
#if !defined(RZ_CLIENT_DRONE)
			if (OpenTransport() != NULL)
			{
				nlwarning("A transport is already open, only one transport can be open at a time");
				return false;
			}
#endif

			open();
		}
		else if (commandName == "close")
		{
			close();
		}
		else
			return false;

		return true;
	}

	/// Open the connection by intercepting client gateway message
	void open() throw (ETransportError)
	{
		if (_Open)
		{
			nlwarning("Transport already open");
			return;
		}
#if !defined(RZ_CLIENT_DRONE)
		if (OpenTransport() != NULL)
			throw ETransportError("connect : a transport is already connected !");

		// set this transport as the open transport
		OpenTransport() = this;
#else
		// associate this transport with the simulated client for later dispatching
		SimClient->setGatewayTransport(this);
#endif

		_Open = true;
		_FirstMessageReceived = false;
		// create a route
		onConnection();
	}

	/// Close the server, this will close the listing socket and any active connection
	void close()
	{
		if (!_Open)
		{
			nlwarning("Transport not open");
			return;
		}
#if !defined(RZ_CLIENT_DRONE)
		if (OpenTransport() != this)
			throw ETransportError("close : The connection is not open");

#else
		// associate this transport with the simulated client for later dispatching
		SimClient->setGatewayTransport(NULL);
#endif

		if( _Route != NULL)
			onDisconnection();

		_Open = false;
		_FirstMessageReceived = false;

#if !defined(RZ_CLIENT_DRONE)
		// this transport is no longer open
		OpenTransport() = NULL;
#endif
	}


	/***************************************************/
	/** Event management                              **/
	/***************************************************/

	// handle the connection of the client
	void onConnection ( )
	{
		if (_Route != NULL)
		{
			nlwarning("onConnection : route already created !");
			return;
		}

		CNetworkConnection *netConn;
#if defined(RZ_CLIENT_DRONE)
		netConn = &SimClient->getNetworkConnection();
#else
		netConn = &NetMngr;
#endif
		// Create a new route for this connection
		_Route = new CFEClientRoute(this, netConn);

		// callback the gateway
		_Gateway->onRouteAdded(_Route);
	}

	// handle the deconnection of a new client on the client
	void onDisconnection ()
	{
		if (_Route == NULL)
		{
			nlwarning("onDisconnection : route not created !");
			return;
		}

		// callback the gateway that this route is no more
		_Gateway->onRouteRemoved(_Route);

		// delete the route
		delete _Route;
		_Route = NULL;
	}

	// Called to dispatch an incoming message to the gateway
	void onDispatchMessage(NLMISC::CBitMemStream &bms)
	{
		if (_Route == NULL)
		{
			nlwarning("onDispatchMessage : no route created, message will be discarded");
			return;
		}

		// now, we have received a message
		_FirstMessageReceived = true;

		// Build a CMessage from the bit mem stream
//		string msgName;
//		bms.serial(msgName);

		// create an input stream for dispatching
		CMessage msg("", true);
		uint32 len;
		bms.serial(len);
		bms.serialBuffer(msg.bufferToFill(len), len);
		msg.readType();

		// forward to gateway
		_Gateway->onReceiveMessage(_Route, msg);
	}

	// a message has been processed, try to process waiting message
	void processPendingMessage()
	{
		if (_WaitingMessages.empty())
			return;

		TWaitingMessages::iterator it(_WaitingMessages.find(_NextAwaitedMessage));
		while (it != _WaitingMessages.end())
		{
			TWaitingMessage &wm = _WaitingMessages.find(_NextAwaitedMessage)->second;

			switch (wm.MessageType)
			{
			case mt_route_open:
				impulsionGatewayOpen(wm.Message, false);
				break;
			case mt_route_close:
				impulsionGatewayClose(wm.Message, false);
				break;
			case mt_gw_msg:
				impulsionGatewayMessage(wm.Message, false);
				break;
			}

			// remove the processed message
			_WaitingMessages.erase(it);
			// advance to next message, _NextAwaitedMessage is incremented by impulsion handler
			it = _WaitingMessages.find(_NextAwaitedMessage);
		}
	}

	// impulsion gateway open callback handler
	void impulsionGatewayOpen(NLMISC::CBitMemStream &bms, bool readSerialNumber)
	{
		uint8 serialNumber;
		if (readSerialNumber)
			bms.serial(serialNumber);
		else
			serialNumber = _NextAwaitedMessage;

		if (serialNumber != _NextAwaitedMessage)
		{
			// store the message for later processing
			nlassert(_WaitingMessages.find(serialNumber) == _WaitingMessages.end());
			_WaitingMessages[serialNumber].Message.swap(bms);
//			_WaitingMessages[serialNumber].Message = bms;
			_WaitingMessages[serialNumber].MessageType = mt_route_open;

			return;
		}

		// advance to next message for next reception
		++_NextAwaitedMessage;

		if (_Route != NULL && _FirstMessageReceived)
		{
			// there is already a route here, and it have received some message,
			// we need to delete it
			onDisconnection();
		}

		if (_Route == NULL)
			// if there is no route, create one
			onConnection();

		// try to process some messages
		if (readSerialNumber)
			processPendingMessage();
	}

	// impulsion gateway message callback handler
	void impulsionGatewayMessage(NLMISC::CBitMemStream &bms, bool readSerialNumber)
	{
		uint8 serialNumber;
		if (readSerialNumber)
			bms.serial(serialNumber);
		else
			serialNumber = _NextAwaitedMessage;

		if (serialNumber != _NextAwaitedMessage)
		{
			// store the message for later processing
			nlassert(_WaitingMessages.find(serialNumber) == _WaitingMessages.end());
			_WaitingMessages[serialNumber].Message.swap(bms);
//			_WaitingMessages[serialNumber].Message = bms;
			_WaitingMessages[serialNumber].MessageType = mt_gw_msg;

			return;
		}

		// advance to next message for next reception
		++_NextAwaitedMessage;

		onDispatchMessage(bms);

		if (readSerialNumber)
			processPendingMessage();
	}


	// impulsion gateway open callback handler
	void impulsionGatewayClose(NLMISC::CBitMemStream &bms, bool readSerialNumber)
	{
		uint8 serialNumber;
		if (readSerialNumber)
			bms.serial(serialNumber);
		else
			serialNumber = _NextAwaitedMessage;

		if (serialNumber != _NextAwaitedMessage)
		{
			// store the message for later processing
			nlassert(_WaitingMessages.find(serialNumber) == _WaitingMessages.end());
			_WaitingMessages[serialNumber].Message.swap(bms);
//			_WaitingMessages[serialNumber].Message = bms;
			_WaitingMessages[serialNumber].MessageType = mt_route_close;

			return;
		}

		// advance to next message for next reception
		++_NextAwaitedMessage;

		if (_Route == NULL)
		{
			// there is no route open
			return;
		}

		onDisconnection();

		if (readSerialNumber)
			processPendingMessage();
	}


	/***************************************************/

	static CGatewayFEClientTransport *getCurrentTransport()
	{
#if defined(RZ_CLIENT_DRONE)
		// the current transport is set to the current context of the client drone
		if (CSimulatedClient::currentContext() == NULL)
			return NULL;
		if (CSimulatedClient::currentContext()->getGatewayTransport() == NULL)
			return NULL;

		return static_cast<CGatewayFEClientTransport*>(CSimulatedClient::currentContext()->getGatewayTransport());
#else
		// normal client mode, there is only one transport
		return OpenTransport();
#endif

	}
};



void cbImpulsionGatewayOpen(NLMISC::CBitMemStream &bms)
{
	if (CGatewayFEClientTransport::getCurrentTransport() != NULL)
		CGatewayFEClientTransport::getCurrentTransport()->impulsionGatewayOpen(bms, true);
}


// impulsion gateway message callback handler
void cbImpulsionGatewayMessage(NLMISC::CBitMemStream &bms)
{
	if (CGatewayFEClientTransport::getCurrentTransport() != NULL)
		CGatewayFEClientTransport::getCurrentTransport()->impulsionGatewayMessage(bms, true);
}

// impulsion gateway close callback handler
void cbImpulsionGatewayClose(NLMISC::CBitMemStream &bms)
{
	if (CGatewayFEClientTransport::getCurrentTransport() != NULL)
		CGatewayFEClientTransport::getCurrentTransport()->impulsionGatewayClose(bms, true);
}



// register this class in the transport factory
NLMISC_REGISTER_OBJECT(IGatewayTransport, CGatewayFEClientTransport, std::string, string(FE_CLIENT_CLASS_NAME));

