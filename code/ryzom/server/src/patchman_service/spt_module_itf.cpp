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

#include "spt_module_itf.h"

namespace PATCHMAN
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CServerPatchTerminalSkel::TMessageHandlerMap &CServerPatchTerminalSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SPT_STATE"), &CServerPatchTerminalSkel::declareState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_MODULEDOWN"), &CServerPatchTerminalSkel::declareModuleDown_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_VERSION_NAME"), &CServerPatchTerminalSkel::declareVersionName_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_DOMAIN_INFO"), &CServerPatchTerminalSkel::declareDomainInfo_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_VERSION_ACK"), &CServerPatchTerminalSkel::ackVersionChange_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_SPA_EXEC_ACK"), &CServerPatchTerminalSkel::onSPAExecutedCommandAck_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_SPA_EXEC_RESULT"), &CServerPatchTerminalSkel::onSPAExecutedCommandResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CServerPatchTerminalSkel::onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CServerPatchTerminalSkel::declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	moduleName;
			nlRead(__message, serial, moduleName);
		NLMISC::CSString	state;
			nlRead(__message, serial, state);
		declareState(sender, moduleName, state);
	}

	void CServerPatchTerminalSkel::declareModuleDown_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	moduleName;
			nlRead(__message, serial, moduleName);
		declareModuleDown(sender, moduleName);
	}

	void CServerPatchTerminalSkel::declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	versionName;
			nlRead(__message, serial, versionName);
		uint32	clientVersion;
			nlRead(__message, serial, clientVersion);
		uint32	serverVersion;
			nlRead(__message, serial, serverVersion);
		declareVersionName(sender, versionName, clientVersion, serverVersion);
	}

	void CServerPatchTerminalSkel::declareDomainInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		uint32	installVersion;
			nlRead(__message, serial, installVersion);
		uint32	launchVersion;
			nlRead(__message, serial, launchVersion);
		declareDomainInfo(sender, domainName, installVersion, launchVersion);
	}

	void CServerPatchTerminalSkel::ackVersionChange_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		bool	success;
			nlRead(__message, serial, success);
		NLMISC::CSString	comment;
			nlRead(__message, serial, comment);
		ackVersionChange(sender, domainName, success, comment);
	}

	void CServerPatchTerminalSkel::onSPAExecutedCommandAck_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	result;
			nlRead(__message, serial, result);
		onSPAExecutedCommandAck(sender, result);
	}

	void CServerPatchTerminalSkel::onSPAExecutedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	originator;
			nlRead(__message, serial, originator);
		NLMISC::CSString	commandline;
			nlRead(__message, serial, commandline);
		NLMISC::CSString	result;
			nlRead(__message, serial, result);
		onSPAExecutedCommandResult(sender, originator, commandline, result);
	}
		// 
		// Message sent by SPM module to declare the state of a named module
		// This message is sent by the SPM for each connected SP / RE / RR type module on connection of SPT to SPM
		// This message is also sent by the SPM for each type the SPM receives a state update from a SP / RE / RR type module
	void CServerPatchTerminalProxy::declareState(NLNET::IModule *sender, const NLMISC::CSString &moduleName, const NLMISC::CSString &state)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), moduleName, state);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareState(__message, moduleName, state);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to declare module down for a connected SPA / RE / RR type module
	void CServerPatchTerminalProxy::declareModuleDown(NLNET::IModule *sender, const NLMISC::CSString &moduleName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareModuleDown(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), moduleName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareModuleDown(__message, moduleName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to define a named version
	void CServerPatchTerminalProxy::declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareVersionName(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), versionName, clientVersion, serverVersion);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareVersionName(__message, versionName, clientVersion, serverVersion);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to give info on a named domain
	void CServerPatchTerminalProxy::declareDomainInfo(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareDomainInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, installVersion, launchVersion);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareDomainInfo(__message, domainName, installVersion, launchVersion);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to acknowledge a version change attempt
	void CServerPatchTerminalProxy::ackVersionChange(NLNET::IModule *sender, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->ackVersionChange(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, success, comment);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_ackVersionChange(__message, domainName, success, comment);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
	void CServerPatchTerminalProxy::onSPAExecutedCommandAck(NLNET::IModule *sender, const NLMISC::CSString &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onSPAExecutedCommandAck(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_onSPAExecutedCommandAck(__message, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
	void CServerPatchTerminalProxy::onSPAExecutedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->onSPAExecutedCommandResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), originator, commandline, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_onSPAExecutedCommandResult(__message, originator, commandline, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareState(NLNET::CMessage &__message, const NLMISC::CSString &moduleName, const NLMISC::CSString &state)
	{
		__message.setType("SPT_STATE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (moduleName));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (state));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareModuleDown(NLNET::CMessage &__message, const NLMISC::CSString &moduleName)
	{
		__message.setType("SPT_MODULEDOWN");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (moduleName));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareVersionName(NLNET::CMessage &__message, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		__message.setType("SPT_VERSION_NAME");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (versionName));
			nlWrite(__message, serial, clientVersion);
			nlWrite(__message, serial, serverVersion);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareDomainInfo(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion)
	{
		__message.setType("SPT_DOMAIN_INFO");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, installVersion);
			nlWrite(__message, serial, launchVersion);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_ackVersionChange(NLNET::CMessage &__message, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment)
	{
		__message.setType("SPT_VERSION_ACK");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, success);
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (comment));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_onSPAExecutedCommandAck(NLNET::CMessage &__message, const NLMISC::CSString &result)
	{
		__message.setType("SPT_SPA_EXEC_ACK");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (result));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_onSPAExecutedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		__message.setType("SPT_SPA_EXEC_RESULT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (originator));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (commandline));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (result));


		return __message;
	}

}
