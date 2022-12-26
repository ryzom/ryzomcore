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
	
#include "mail_forum_itf.h"

namespace MFS
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CMailForumNotifierSkel::TMessageHandlerMap &CMailForumNotifierSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("MFS_NM"), &CMailForumNotifierSkel::notifyMail_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("MFS_NFM"), &CMailForumNotifierSkel::notifyForumMessage_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CMailForumNotifierSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CMailForumNotifierSkel::notifyMail_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CMailForumNotifierSkel_notifyMail_MFS_NM);
		uint32	charId;
			nlRead(__message, serial, charId);
		notifyMail(sender, charId);
	}

	void CMailForumNotifierSkel::notifyForumMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CMailForumNotifierSkel_notifyForumMessage_MFS_NFM);
		uint32	charId;
			nlRead(__message, serial, charId);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		uint32	threadId;
			nlRead(__message, serial, threadId);
		notifyForumMessage(sender, charId, guildId, threadId);
	}
		// A character have received a mail
	void CMailForumNotifierProxy::notifyMail(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->notifyMail(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_notifyMail(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A new message have been posted in a guild forum
		// the notifier client send a notification for each member character
	void CMailForumNotifierProxy::notifyForumMessage(NLNET::IModule *sender, uint32 charId, uint32 guildId, uint32 threadId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->notifyForumMessage(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, guildId, threadId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_notifyForumMessage(__message, charId, guildId, threadId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CMailForumNotifierProxy::buildMessageFor_notifyMail(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("MFS_NM");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CMailForumNotifierProxy::buildMessageFor_notifyForumMessage(NLNET::CMessage &__message, uint32 charId, uint32 guildId, uint32 threadId)
	{
		__message.setType("MFS_NFM");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, threadId);


		return __message;
	}

}
