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
	
#include "guild_unifier_itf.h"

namespace GU
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CGuildUnifierClientSkel::TMessageHandlerMap &CGuildUnifierClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("GR"), &CGuildUnifierClientSkel::guildReady_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RFG"), &CGuildUnifierClientSkel::receiveForeignGuild_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UML"), &CGuildUnifierClientSkel::updateMemberList_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UMI"), &CGuildUnifierClientSkel::updateMemberInfo_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UG"), &CGuildUnifierClientSkel::updateGuild_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("GD"), &CGuildUnifierClientSkel::guildDeleted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("MGM"), &CGuildUnifierClientSkel::messageToGuildMembers_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CGuildUnifierClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CGuildUnifierClientSkel::guildReady_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_guildReady_GR);
		guildReady(sender);
	}

	void CGuildUnifierClientSkel::receiveForeignGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_receiveForeignGuild_RFG);
		std::vector < CGuildDesc >	guilds;
			nlRead(__message, serialCont, guilds);
		receiveForeignGuild(sender, guilds);
	}

	void CGuildUnifierClientSkel::updateMemberList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_updateMemberList_UML);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		std::vector < CGuildMemberDesc >	members;
			nlRead(__message, serialCont, members);
		updateMemberList(sender, guildId, members);
	}

	void CGuildUnifierClientSkel::updateMemberInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_updateMemberInfo_UMI);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		CGuildMemberDesc	membersInfo;
			nlRead(__message, serial, membersInfo);
		updateMemberInfo(sender, guildId, membersInfo);
	}

	void CGuildUnifierClientSkel::updateGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_updateGuild_UG);
		CGuildDesc	guildInfo;
			nlRead(__message, serial, guildInfo);
		updateGuild(sender, guildInfo);
	}

	void CGuildUnifierClientSkel::guildDeleted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_guildDeleted_GD);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		guildDeleted(sender, guildId);
	}

	void CGuildUnifierClientSkel::messageToGuildMembers_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CGuildUnifierClientSkel_messageToGuildMembers_MGM);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		std::string	messageName;
			nlRead(__message, serial, messageName);
		TVectorParamCheck	params;
			nlRead(__message, serialCont, params);
		messageToGuildMembers(sender, guildId, messageName, params);
	}
		// A client says to others clients that it is ready to send/receive guild data
	void CGuildUnifierClientProxy::guildReady(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->guildReady(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_guildReady(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The server send it local guilds to the client
	void CGuildUnifierClientProxy::receiveForeignGuild(NLNET::IModule *sender, const std::vector < CGuildDesc > &guilds)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->receiveForeignGuild(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guilds);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_receiveForeignGuild(__message, guilds);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The member list have changed, each guild unifier receive a copy of the new list
	void CGuildUnifierClientProxy::updateMemberList(NLNET::IModule *sender, uint32 guildId, const std::vector < CGuildMemberDesc > &members)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateMemberList(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId, members);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_updateMemberList(__message, guildId, members);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A member in the guild has changed, update it's info
	void CGuildUnifierClientProxy::updateMemberInfo(NLNET::IModule *sender, uint32 guildId, const CGuildMemberDesc &membersInfo)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateMemberInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId, membersInfo);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_updateMemberInfo(__message, guildId, membersInfo);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The guild has been saved, the guild host send an update of the guild status (with fames, but no members)
	void CGuildUnifierClientProxy::updateGuild(NLNET::IModule *sender, const CGuildDesc &guildInfo)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateGuild(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildInfo);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_updateGuild(__message, guildInfo);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A guild have been deleted
	void CGuildUnifierClientProxy::guildDeleted(NLNET::IModule *sender, uint32 guildId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->guildDeleted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_guildDeleted(__message, guildId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Send a message to all the guild members
	void CGuildUnifierClientProxy::messageToGuildMembers(NLNET::IModule *sender, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->messageToGuildMembers(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId, messageName, params);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_messageToGuildMembers(__message, guildId, messageName, params);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_guildReady(NLNET::CMessage &__message)
	{
		__message.setType("GR");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_receiveForeignGuild(NLNET::CMessage &__message, const std::vector < CGuildDesc > &guilds)
	{
		__message.setType("RFG");
			nlWrite(__message, serialCont, const_cast < std::vector < CGuildDesc >& > (guilds));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_updateMemberList(NLNET::CMessage &__message, uint32 guildId, const std::vector < CGuildMemberDesc > &members)
	{
		__message.setType("UML");
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serialCont, const_cast < std::vector < CGuildMemberDesc >& > (members));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_updateMemberInfo(NLNET::CMessage &__message, uint32 guildId, const CGuildMemberDesc &membersInfo)
	{
		__message.setType("UMI");
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, const_cast < CGuildMemberDesc& > (membersInfo));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_updateGuild(NLNET::CMessage &__message, const CGuildDesc &guildInfo)
	{
		__message.setType("UG");
			nlWrite(__message, serial, const_cast < CGuildDesc& > (guildInfo));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_guildDeleted(NLNET::CMessage &__message, uint32 guildId)
	{
		__message.setType("GD");
			nlWrite(__message, serial, guildId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CGuildUnifierClientProxy::buildMessageFor_messageToGuildMembers(NLNET::CMessage &__message, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params)
	{
		__message.setType("MGM");
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, const_cast < std::string& > (messageName));
			nlWrite(__message, serialCont, const_cast < TVectorParamCheck& > (params));


		return __message;
	}

}
