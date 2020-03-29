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

#ifndef SPT_MODULE_ITF_H
#define SPT_MODULE_ITF_H
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/misc/string_conversion.h"

#include "nel/misc/sstring.h"
	
#include "server_patch_types.h"
	
#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
namespace PATCHMAN
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchTerminalSkel
	{
	protected:
		CServerPatchTerminalSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerPatchTerminalSkel()
		{
		}



	private:
		typedef void (CServerPatchTerminalSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const;

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);

	private:
		
		void declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareModuleDown_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareDomainInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void ackVersionChange_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onSPAExecutedCommandAck_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void onSPAExecutedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		// Message sent by SPM module to declare the state of a named module
		// This message is sent by the SPM for each connected SP / RE / RR type module on connection of SPT to SPM
		// This message is also sent by the SPM for each type the SPM receives a state update from a SP / RE / RR type module
		virtual void declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &moduleName, const NLMISC::CSString &state) =0;
		// 
		// Message sent by SPM module to declare module down for a connected SPA / RE / RR type module
		virtual void declareModuleDown(NLNET::IModuleProxy *sender, const NLMISC::CSString &moduleName) =0;
		// 
		// Message sent by SPM module to define a named version
		virtual void declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion) =0;
		// 
		// Message sent by SPM module to give info on a named domain
		virtual void declareDomainInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion) =0;
		// 
		// Message sent by SPM module to acknowledge a version change attempt
		virtual void ackVersionChange(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment) =0;
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
		virtual void onSPAExecutedCommandAck(NLNET::IModuleProxy *sender, const NLMISC::CSString &result) =0;
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
		virtual void onSPAExecutedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchTerminalProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CServerPatchTerminalSkel	*_LocalModuleSkel;


	public:
		CServerPatchTerminalProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CServerPatchTerminalSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CServerPatchTerminalProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		// Message sent by SPM module to declare the state of a named module
		// This message is sent by the SPM for each connected SP / RE / RR type module on connection of SPT to SPM
		// This message is also sent by the SPM for each type the SPM receives a state update from a SP / RE / RR type module
		void declareState(NLNET::IModule *sender, const NLMISC::CSString &moduleName, const NLMISC::CSString &state);
		// 
		// Message sent by SPM module to declare module down for a connected SPA / RE / RR type module
		void declareModuleDown(NLNET::IModule *sender, const NLMISC::CSString &moduleName);
		// 
		// Message sent by SPM module to define a named version
		void declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
		// 
		// Message sent by SPM module to give info on a named domain
		void declareDomainInfo(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion);
		// 
		// Message sent by SPM module to acknowledge a version change attempt
		void ackVersionChange(NLNET::IModule *sender, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment);
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
		void onSPAExecutedCommandAck(NLNET::IModule *sender, const NLMISC::CSString &result);
		// 
		// Message sent by SPA with result of command issuued via executeCommandOnSPA()
		void onSPAExecutedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_declareState(NLNET::CMessage &__message, const NLMISC::CSString &moduleName, const NLMISC::CSString &state);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_declareModuleDown(NLNET::CMessage &__message, const NLMISC::CSString &moduleName);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_declareVersionName(NLNET::CMessage &__message, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_declareDomainInfo(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_ackVersionChange(NLNET::CMessage &__message, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onSPAExecutedCommandAck(NLNET::CMessage &__message, const NLMISC::CSString &result);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_onSPAExecutedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);
	



	};

}
	
#endif
