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

#include "spm_module_itf.h"

namespace PATCHMAN
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CServerPatchManagerSkel::TMessageHandlerMap &CServerPatchManagerSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SPM_REFRESH"), &CServerPatchManagerSkel::requestRefresh_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_SETNEXT"), &CServerPatchManagerSkel::setInstallVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_SETLIVE"), &CServerPatchManagerSkel::setLaunchVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_STATE"), &CServerPatchManagerSkel::declareState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_VERSION_NAME"), &CServerPatchManagerSkel::declareVersionName_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_SPA_EXEC"), &CServerPatchManagerSkel::executeCommandOnSPA_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_SPA_EXEC_RESULT"), &CServerPatchManagerSkel::spaExecutedCommandResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CServerPatchManagerSkel::onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CServerPatchManagerSkel::requestRefresh_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		requestRefresh(sender);
	}

	void CServerPatchManagerSkel::setInstallVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	domain;
			nlRead(__message, serial, domain);
		uint32	version;
			nlRead(__message, serial, version);
		setInstallVersion(sender, domain, version);
	}

	void CServerPatchManagerSkel::setLaunchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	domain;
			nlRead(__message, serial, domain);
		uint32	version;
			nlRead(__message, serial, version);
		setLaunchVersion(sender, domain, version);
	}

	void CServerPatchManagerSkel::declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	state;
			nlRead(__message, serial, state);
		declareState(sender, state);
	}

	void CServerPatchManagerSkel::declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	versionName;
			nlRead(__message, serial, versionName);
		uint32	clientVersion;
			nlRead(__message, serial, clientVersion);
		uint32	serverVersion;
			nlRead(__message, serial, serverVersion);
		declareVersionName(sender, versionName, clientVersion, serverVersion);
	}

	void CServerPatchManagerSkel::executeCommandOnSPA_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	target;
			nlRead(__message, serial, target);
		NLMISC::CSString	commandline;
			nlRead(__message, serial, commandline);
		executeCommandOnSPA(sender, target, commandline);
	}

	void CServerPatchManagerSkel::spaExecutedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLMISC::CSString	originator;
			nlRead(__message, serial, originator);
		NLMISC::CSString	commandline;
			nlRead(__message, serial, commandline);
		NLMISC::CSString	result;
			nlRead(__message, serial, result);
		spaExecutedCommandResult(sender, originator, commandline, result);
	}
		// 
		// Message sent by SPT module to request a refresh of state info etc
	void CServerPatchManagerProxy::requestRefresh(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->requestRefresh(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_requestRefresh(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to request a change of install version for a given domain
		// This message is forwarded to all SPA modules of the given domain
	void CServerPatchManagerProxy::setInstallVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setInstallVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domain, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setInstallVersion(__message, domain, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to request a change of launch version for a given domain
		// This message is forwarded to all SPA modules of the given domain
	void CServerPatchManagerProxy::setLaunchVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setLaunchVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domain, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setLaunchVersion(__message, domain, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by RE / RR / SPA type modules to declare their states
		// This message is forwarded to all connected SPT modules
	void CServerPatchManagerProxy::declareState(NLNET::IModule *sender, const NLMISC::CSString &state)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), state);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareState(__message, state);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to define a new named version
	void CServerPatchManagerProxy::declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
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
		// Message sent by SPT module to request execution of a command on one or more modules
		// Note that the 'target' parameter may be a wildcard
	void CServerPatchManagerProxy::executeCommandOnSPA(NLNET::IModule *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executeCommandOnSPA(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), target, commandline);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executeCommandOnSPA(__message, target, commandline);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
	void CServerPatchManagerProxy::spaExecutedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->spaExecutedCommandResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), originator, commandline, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_spaExecutedCommandResult(__message, originator, commandline, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_requestRefresh(NLNET::CMessage &__message)
	{
		__message.setType("SPM_REFRESH");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_setInstallVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version)
	{
		__message.setType("SPM_SETNEXT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domain));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_setLaunchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version)
	{
		__message.setType("SPM_SETLIVE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domain));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_declareState(NLNET::CMessage &__message, const NLMISC::CSString &state)
	{
		__message.setType("SPM_STATE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (state));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_declareVersionName(NLNET::CMessage &__message, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		__message.setType("SPM_VERSION_NAME");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (versionName));
			nlWrite(__message, serial, clientVersion);
			nlWrite(__message, serial, serverVersion);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_executeCommandOnSPA(NLNET::CMessage &__message, const NLMISC::CSString &target, const NLMISC::CSString &commandline)
	{
		__message.setType("SPM_SPA_EXEC");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (target));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (commandline));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_spaExecutedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		__message.setType("SPM_SPA_EXEC_RESULT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (originator));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (commandline));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (result));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CDeploymentConfigurationSynchroniserSkel::TMessageHandlerMap &CDeploymentConfigurationSynchroniserSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("DEPCFG_REQUEST"), &CDeploymentConfigurationSynchroniserSkel::requestSync_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("DEPCFG_SYNC"), &CDeploymentConfigurationSynchroniserSkel::sync_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CDeploymentConfigurationSynchroniserSkel::onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CDeploymentConfigurationSynchroniserSkel::requestSync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		requestSync(sender);
	}

	void CDeploymentConfigurationSynchroniserSkel::sync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		NLNET::TBinBuffer	dataBlob;
			nlRead(__message, serial, dataBlob);
		sync(sender, dataBlob);
	}
		// 
		// Request for a copy of another module's CDeploymentConfiguration singleton
	void CDeploymentConfigurationSynchroniserProxy::requestSync(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->requestSync(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_requestSync(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// A copy of the data from the CDeploymentConfiguration singleton
	void CDeploymentConfigurationSynchroniserProxy::sync(NLNET::IModule *sender, const NLNET::TBinBuffer &dataBlob)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->sync(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), dataBlob);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_sync(__message, dataBlob);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CDeploymentConfigurationSynchroniserProxy::buildMessageFor_requestSync(NLNET::CMessage &__message)
	{
		__message.setType("DEPCFG_REQUEST");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CDeploymentConfigurationSynchroniserProxy::buildMessageFor_sync(NLNET::CMessage &__message, const NLNET::TBinBuffer &dataBlob)
	{
		__message.setType("DEPCFG_SYNC");
			nlWrite(__message, serial, const_cast < NLNET::TBinBuffer& > (dataBlob));


		return __message;
	}

}
