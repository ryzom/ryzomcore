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

#ifndef SPM_MODULE_ITF_H
#define SPM_MODULE_ITF_H
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/misc/string_conversion.h"

#include "nel/misc/sstring.h"
	
#include "nel/net/module_message.h"
	
#include "server_patch_types.h"
	
#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
namespace PATCHMAN
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchManagerSkel
	{
	protected:
		CServerPatchManagerSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerPatchManagerSkel()
		{
		}



	private:
		typedef void (CServerPatchManagerSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const;

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);

	private:
		
		void requestRefresh_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setInstallVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setLaunchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void executeCommandOnSPA_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void spaExecutedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		// Message sent by SPT module to request a refresh of state info etc
		virtual void requestRefresh(NLNET::IModuleProxy *sender) =0;
		// 
		// Message sent by SPT module to request a change of install version for a given domain
		// This message is forwarded to all SPA modules of the given domain
		virtual void setInstallVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version) =0;
		// 
		// Message sent by SPT module to request a change of launch version for a given domain
		// This message is forwarded to all SPA modules of the given domain
		virtual void setLaunchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version) =0;
		// 
		// Message sent by RE / RR / SPA type modules to declare their states
		// This message is forwarded to all connected SPT modules
		virtual void declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &state) =0;
		// 
		// Message sent by SPT module to define a new named version
		virtual void declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion) =0;
		// 
		// Message sent by SPT module to request execution of a command on one or more modules
		// Note that the 'target' parameter may be a wildcard
		virtual void executeCommandOnSPA(NLNET::IModuleProxy *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline) =0;
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
		virtual void spaExecutedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchManagerProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CServerPatchManagerSkel	*_LocalModuleSkel;


	public:
		CServerPatchManagerProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CServerPatchManagerSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CServerPatchManagerProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		// Message sent by SPT module to request a refresh of state info etc
		void requestRefresh(NLNET::IModule *sender);
		// 
		// Message sent by SPT module to request a change of install version for a given domain
		// This message is forwarded to all SPA modules of the given domain
		void setInstallVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version);
		// 
		// Message sent by SPT module to request a change of launch version for a given domain
		// This message is forwarded to all SPA modules of the given domain
		void setLaunchVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version);
		// 
		// Message sent by RE / RR / SPA type modules to declare their states
		// This message is forwarded to all connected SPT modules
		void declareState(NLNET::IModule *sender, const NLMISC::CSString &state);
		// 
		// Message sent by SPT module to define a new named version
		void declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
		// 
		// Message sent by SPT module to request execution of a command on one or more modules
		// Note that the 'target' parameter may be a wildcard
		void executeCommandOnSPA(NLNET::IModule *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline);
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
		void spaExecutedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_requestRefresh(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setInstallVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setLaunchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_declareState(NLNET::CMessage &__message, const NLMISC::CSString &state);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_declareVersionName(NLNET::CMessage &__message, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_executeCommandOnSPA(NLNET::CMessage &__message, const NLMISC::CSString &target, const NLMISC::CSString &commandline);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_spaExecutedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CDeploymentConfigurationSynchroniserSkel
	{
	protected:
		CDeploymentConfigurationSynchroniserSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CDeploymentConfigurationSynchroniserSkel()
		{
		}



	private:
		typedef void (CDeploymentConfigurationSynchroniserSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const;

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);

	private:
		
		void requestSync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void sync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		// Request for a copy of another module's CDeploymentConfiguration singleton
		virtual void requestSync(NLNET::IModuleProxy *sender) =0;
		// 
		// A copy of the data from the CDeploymentConfiguration singleton
		virtual void sync(NLNET::IModuleProxy *sender, const NLNET::TBinBuffer &dataBlob) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CDeploymentConfigurationSynchroniserProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CDeploymentConfigurationSynchroniserSkel	*_LocalModuleSkel;


	public:
		CDeploymentConfigurationSynchroniserProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CDeploymentConfigurationSynchroniserSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CDeploymentConfigurationSynchroniserProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		// Request for a copy of another module's CDeploymentConfiguration singleton
		void requestSync(NLNET::IModule *sender);
		// 
		// A copy of the data from the CDeploymentConfiguration singleton
		void sync(NLNET::IModule *sender, const NLNET::TBinBuffer &dataBlob);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_requestSync(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_sync(NLNET::CMessage &__message, const NLNET::TBinBuffer &dataBlob);
	



	};

}
	
#endif
