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
	
#include "command_executor_itf.h"

namespace CMDEXE
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CCommandExecutorSkel::TMessageHandlerMap &CCommandExecutorSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("CE_SC"), &CCommandExecutorSkel::sendCommand_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CCommandExecutorSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CCommandExecutorSkel::sendCommand_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCommandExecutorSkel_sendCommand_CE_SC);
		std::string	commandName;
			nlRead(__message, serial, commandName);
		NLMISC::CEntityId	senderEId;
			nlRead(__message, serial, senderEId);
		bool	haveTarget;
			nlRead(__message, serial, haveTarget);
		NLMISC::CEntityId	targetEId;
			nlRead(__message, serial, targetEId);
		std::string	arg;
			nlRead(__message, serial, arg);
		sendCommand(sender, commandName, senderEId, haveTarget, targetEId, arg);
	}
		// 
	void CCommandExecutorProxy::sendCommand(NLNET::IModule *sender, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->sendCommand(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), commandName, senderEId, haveTarget, targetEId, arg);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_sendCommand(__message, commandName, senderEId, haveTarget, targetEId, arg);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCommandExecutorProxy::buildMessageFor_sendCommand(NLNET::CMessage &__message, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg)
	{
		__message.setType("CE_SC");
			nlWrite(__message, serial, const_cast < std::string& > (commandName));
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (senderEId));
			nlWrite(__message, serial, haveTarget);
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (targetEId));
			nlWrite(__message, serial, const_cast < std::string& > (arg));


		return __message;
	}

}
