
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
