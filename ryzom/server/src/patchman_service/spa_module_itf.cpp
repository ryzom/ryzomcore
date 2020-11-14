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

#include "spa_module_itf.h"

namespace PATCHMAN
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CServerPatchApplierSkel::TMessageHandlerMap &CServerPatchApplierSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SPA_SETNEXT"), &CServerPatchApplierSkel::installVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPA_SETLIVE"), &CServerPatchApplierSkel::launchVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPA_EXEC"), &CServerPatchApplierSkel::executeCommand_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CServerPatchApplierSkel::onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CServerPatchApplierSkel::installVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		uint32	version;
			nlRead(__message, serial, version);
		installVersion(sender, domainName, version);
	}

	void CServerPatchApplierSkel::launchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		uint32	version;
			nlRead(__message, serial, version);
		launchVersion(sender, domainName, version);
	}

	void CServerPatchApplierSkel::executeCommand_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	cmdline;
			nlRead(__message, serial, cmdline);
		NLMISC::CSString	originator;
			nlRead(__message, serial, originator);
		executeCommand(sender, cmdline, originator);
	}
		// 
	void CServerPatchApplierProxy::installVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->installVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_installVersion(__message, domainName, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CServerPatchApplierProxy::launchVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->launchVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_launchVersion(__message, domainName, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM to tell us to execute a comman locally
	void CServerPatchApplierProxy::executeCommand(NLNET::IModule *sender, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executeCommand(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), cmdline, originator);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executeCommand(__message, cmdline, originator);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchApplierProxy::buildMessageFor_installVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version)
	{
		__message.setType("SPA_SETNEXT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchApplierProxy::buildMessageFor_launchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version)
	{
		__message.setType("SPA_SETLIVE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchApplierProxy::buildMessageFor_executeCommand(NLNET::CMessage &__message, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator)
	{
		__message.setType("SPA_EXEC");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (cmdline));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (originator));


		return __message;
	}

}
