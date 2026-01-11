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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "stdpch.h"
	
#include "entity_locator_itf.h"

namespace ENTITYLOC
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CEntityLocatorSkel::TMessageHandlerMap &CEntityLocatorSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("EL_IS"), &CEntityLocatorSkel::initState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("EL_PC"), &CEntityLocatorSkel::playerConnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("EL_PD"), &CEntityLocatorSkel::playerDisconnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("EL_CC"), &CEntityLocatorSkel::charConnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("EL_CD"), &CEntityLocatorSkel::charDisconnected_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CEntityLocatorSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}

	
	void CEntityLocatorSkel::initState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CEntityLocatorSkel_initState_EL_IS);
		std::vector < uint32 >	connectedUsers;
			nlRead(__message, serialCont, connectedUsers);
		std::vector < TConnectedCharInfo >	connectedChars;
			nlRead(__message, serialCont, connectedChars);
		initState(sender, connectedUsers, connectedChars);
	}

	void CEntityLocatorSkel::playerConnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CEntityLocatorSkel_playerConnected_EL_PC);
		uint32	userId;
			nlRead(__message, serial, userId);
		playerConnected(sender, userId);
	}

	void CEntityLocatorSkel::playerDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CEntityLocatorSkel_playerDisconnected_EL_PD);
		uint32	userId;
			nlRead(__message, serial, userId);
		playerDisconnected(sender, userId);
	}

	void CEntityLocatorSkel::charConnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CEntityLocatorSkel_charConnected_EL_CC);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		uint32	lastDisconnectionDate;
			nlRead(__message, serial, lastDisconnectionDate);
		charConnected(sender, charEId, lastDisconnectionDate);
	}

	void CEntityLocatorSkel::charDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CEntityLocatorSkel_charDisconnected_EL_CD);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		charDisconnected(sender, charEId);
	}
		// The locator client send the initial state of active player and character connections
	void CEntityLocatorProxy::initState(NLNET::IModule *sender, const std::vector < uint32 > &connectedUsers, const std::vector < TConnectedCharInfo > &connectedChars)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->initState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), connectedUsers, connectedChars);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_initState(__message, connectedUsers, connectedChars);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A player has connected on a shard
	void CEntityLocatorProxy::playerConnected(NLNET::IModule *sender, uint32 userId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->playerConnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_playerConnected(__message, userId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A player has disconnected from a shard
	void CEntityLocatorProxy::playerDisconnected(NLNET::IModule *sender, uint32 userId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->playerDisconnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_playerDisconnected(__message, userId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A character has connected
	void CEntityLocatorProxy::charConnected(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, uint32 lastDisconnectionDate)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->charConnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId, lastDisconnectionDate);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_charConnected(__message, charEId, lastDisconnectionDate);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A character has disconnected
	void CEntityLocatorProxy::charDisconnected(NLNET::IModule *sender, const NLMISC::CEntityId &charEId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->charDisconnected(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_charDisconnected(__message, charEId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CEntityLocatorProxy::buildMessageFor_initState(NLNET::CMessage &__message, const std::vector < uint32 > &connectedUsers, const std::vector < TConnectedCharInfo > &connectedChars)
	{
		__message.setType("EL_IS");
			nlWrite(__message, serialCont, const_cast < std::vector < uint32 >& > (connectedUsers));
			nlWrite(__message, serialCont, const_cast < std::vector < TConnectedCharInfo >& > (connectedChars));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CEntityLocatorProxy::buildMessageFor_playerConnected(NLNET::CMessage &__message, uint32 userId)
	{
		__message.setType("EL_PC");
			nlWrite(__message, serial, userId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CEntityLocatorProxy::buildMessageFor_playerDisconnected(NLNET::CMessage &__message, uint32 userId)
	{
		__message.setType("EL_PD");
			nlWrite(__message, serial, userId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CEntityLocatorProxy::buildMessageFor_charConnected(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, uint32 lastDisconnectionDate)
	{
		__message.setType("EL_CC");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));
			nlWrite(__message, serial, lastDisconnectionDate);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CEntityLocatorProxy::buildMessageFor_charDisconnected(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId)
	{
		__message.setType("EL_CD");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CEntityLocatorClientSkel::TMessageHandlerMap &CEntityLocatorClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("ELC_CE"), &CEntityLocatorClientSkel::connectionEvents_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CEntityLocatorClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}

	
	void CEntityLocatorClientSkel::connectionEvents_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CEntityLocatorClientSkel_connectionEvents_ELC_CE);
		std::vector < TCharConnectionEvent >	events;
			nlRead(__message, serialCont, events);
		connectionEvents(sender, events);
	}
		// The entity locator send a list of connection event to EGS
	void CEntityLocatorClientProxy::connectionEvents(NLNET::IModule *sender, const std::vector < TCharConnectionEvent > &events)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->connectionEvents(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), events);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_connectionEvents(__message, events);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CEntityLocatorClientProxy::buildMessageFor_connectionEvents(NLNET::CMessage &__message, const std::vector < TCharConnectionEvent > &events)
	{
		__message.setType("ELC_CE");
			nlWrite(__message, serialCont, const_cast < std::vector < TCharConnectionEvent >& > (events));


		return __message;
	}

}
