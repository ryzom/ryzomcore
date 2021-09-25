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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "welcome_service_itf.h"

namespace WS
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CWelcomeServiceSkel::TMessageHandlerMap &CWelcomeServiceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("WU"), &CWelcomeServiceSkel::welcomeUser_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("DU"), &CWelcomeServiceSkel::disconnectUser_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CWelcomeServiceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CWelcomeServiceSkel::welcomeUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CWelcomeServiceSkel_welcomeUser_WU);
		uint32	charId;
			nlRead(__message, serial, charId);
		std::string	userName;
			nlRead(__message, serial, userName);
		NLNET::CLoginCookie	cookie;
			nlRead(__message, serial, cookie);
		std::string	priviledge;
			nlRead(__message, serial, priviledge);
		std::string	exPriviledge;
			nlRead(__message, serial, exPriviledge);
		WS::TUserRole	mode;
			nlRead(__message, serial, mode);
		uint32	instanceId;
			nlRead(__message, serial, instanceId);
		welcomeUser(sender, charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);
	}

	void CWelcomeServiceSkel::disconnectUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CWelcomeServiceSkel_disconnectUser_DU);
		uint32	userId;
			nlRead(__message, serial, userId);
		disconnectUser(sender, userId);
	}
		// ask the welcome service to welcome a character
	void CWelcomeServiceProxy::welcomeUser(NLNET::IModule *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->welcomeUser(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_welcomeUser(__message, charId, userName, cookie, priviledge, exPriviledge, mode, instanceId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// ask the welcome service to disconnect a user
	void CWelcomeServiceProxy::disconnectUser(NLNET::IModule *sender, uint32 userId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->disconnectUser(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_disconnectUser(__message, userId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceProxy::buildMessageFor_welcomeUser(NLNET::CMessage &__message, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId)
	{
		__message.setType("WU");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < std::string& > (userName));
			nlWrite(__message, serial, const_cast < NLNET::CLoginCookie& > (cookie));
			nlWrite(__message, serial, const_cast < std::string& > (priviledge));
			nlWrite(__message, serial, const_cast < std::string& > (exPriviledge));
			nlWrite(__message, serial, mode);
			nlWrite(__message, serial, instanceId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceProxy::buildMessageFor_disconnectUser(NLNET::CMessage &__message, uint32 userId)
	{
		__message.setType("DU");
			nlWrite(__message, serial, userId);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CLoginServiceSkel::TMessageHandlerMap &CLoginServiceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("PUL"), &CLoginServiceSkel::pendingUserLost_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CLoginServiceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CLoginServiceSkel::pendingUserLost_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CLoginServiceSkel_pendingUserLost_PUL);
		NLNET::CLoginCookie	cookie;
			nlRead(__message, serial, cookie);
		pendingUserLost(sender, cookie);
	}
		// An awaited user did not connect before the allowed timeout expire
	void CLoginServiceProxy::pendingUserLost(NLNET::IModule *sender, const NLNET::CLoginCookie &cookie)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->pendingUserLost(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), cookie);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_pendingUserLost(__message, cookie);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CLoginServiceProxy::buildMessageFor_pendingUserLost(NLNET::CMessage &__message, const NLNET::CLoginCookie &cookie)
	{
		__message.setType("PUL");
			nlWrite(__message, serial, const_cast < NLNET::CLoginCookie& > (cookie));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CWelcomeServiceClientSkel::TMessageHandlerMap &CWelcomeServiceClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("RWS"), &CWelcomeServiceClientSkel::registerWS_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RWSOS"), &CWelcomeServiceClientSkel::reportWSOpenState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("WUR"), &CWelcomeServiceClientSkel::welcomeUserResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UCP"), &CWelcomeServiceClientSkel::updateConnectedPlayerCount_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CWelcomeServiceClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CWelcomeServiceClientSkel::registerWS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CWelcomeServiceClientSkel_registerWS_RWS);
		uint32	shardId;
			nlRead(__message, serial, shardId);
		uint32	fixedSessionId;
			nlRead(__message, serial, fixedSessionId);
		bool	isOnline;
			nlRead(__message, serial, isOnline);
		registerWS(sender, shardId, fixedSessionId, isOnline);
	}

	void CWelcomeServiceClientSkel::reportWSOpenState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CWelcomeServiceClientSkel_reportWSOpenState_RWSOS);
		bool	isOnline;
			nlRead(__message, serial, isOnline);
		reportWSOpenState(sender, isOnline);
	}

	void CWelcomeServiceClientSkel::welcomeUserResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CWelcomeServiceClientSkel_welcomeUserResult_WUR);
		uint32	userId;
			nlRead(__message, serial, userId);
		bool	ok;
			nlRead(__message, serial, ok);
		std::string	shardAddr;
			nlRead(__message, serial, shardAddr);
		std::string	errorMsg;
			nlRead(__message, serial, errorMsg);
		welcomeUserResult(sender, userId, ok, shardAddr, errorMsg);
	}

	void CWelcomeServiceClientSkel::updateConnectedPlayerCount_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CWelcomeServiceClientSkel_updateConnectedPlayerCount_UCP);
		uint32	nbOnlinePlayers;
			nlRead(__message, serial, nbOnlinePlayers);
		uint32	nbPendingPlayers;
			nlRead(__message, serial, nbPendingPlayers);
		updateConnectedPlayerCount(sender, nbOnlinePlayers, nbPendingPlayers);
	}
		// Register the welcome service in the ring session manager
		// The provided sessionId will be non-zero only for a shard with a fixed sessionId
	void CWelcomeServiceClientProxy::registerWS(NLNET::IModule *sender, uint32 shardId, uint32 fixedSessionId, bool isOnline)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerWS(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId, fixedSessionId, isOnline);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_registerWS(__message, shardId, fixedSessionId, isOnline);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// WS report it's current open state
	void CWelcomeServiceClientProxy::reportWSOpenState(NLNET::IModule *sender, bool isOnline)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportWSOpenState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), isOnline);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_reportWSOpenState(__message, isOnline);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// return for welcome user
	void CWelcomeServiceClientProxy::welcomeUserResult(NLNET::IModule *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->welcomeUserResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId, ok, shardAddr, errorMsg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_welcomeUserResult(__message, userId, ok, shardAddr, errorMsg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// transmits the current player counts
	void CWelcomeServiceClientProxy::updateConnectedPlayerCount(NLNET::IModule *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateConnectedPlayerCount(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), nbOnlinePlayers, nbPendingPlayers);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_updateConnectedPlayerCount(__message, nbOnlinePlayers, nbPendingPlayers);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_registerWS(NLNET::CMessage &__message, uint32 shardId, uint32 fixedSessionId, bool isOnline)
	{
		__message.setType("RWS");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serial, fixedSessionId);
			nlWrite(__message, serial, isOnline);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_reportWSOpenState(NLNET::CMessage &__message, bool isOnline)
	{
		__message.setType("RWSOS");
			nlWrite(__message, serial, isOnline);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_welcomeUserResult(NLNET::CMessage &__message, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
	{
		__message.setType("WUR");
			nlWrite(__message, serial, userId);
			nlWrite(__message, serial, ok);
			nlWrite(__message, serial, const_cast < std::string& > (shardAddr));
			nlWrite(__message, serial, const_cast < std::string& > (errorMsg));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CWelcomeServiceClientProxy::buildMessageFor_updateConnectedPlayerCount(NLNET::CMessage &__message, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
	{
		__message.setType("UCP");
			nlWrite(__message, serial, nbOnlinePlayers);
			nlWrite(__message, serial, nbPendingPlayers);


		return __message;
	}

}
