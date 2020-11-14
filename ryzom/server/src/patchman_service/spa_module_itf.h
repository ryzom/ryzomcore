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

#ifndef SPA_MODULE_ITF_H
#define SPA_MODULE_ITF_H
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/misc/string_conversion.h"

#include "nel/misc/sstring.h"
	
#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
namespace PATCHMAN
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchApplierSkel
	{
	protected:
		CServerPatchApplierSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerPatchApplierSkel()
		{
		}



	private:
		typedef void (CServerPatchApplierSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const;

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);

	private:
		
		void installVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void launchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void executeCommand_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void installVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version) =0;
		// 
		virtual void launchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version) =0;
		// 
		// Message sent by SPM to tell us to execute a comman locally
		virtual void executeCommand(NLNET::IModuleProxy *sender, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchApplierProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CServerPatchApplierSkel	*_LocalModuleSkel;


	public:
		CServerPatchApplierProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CServerPatchApplierSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CServerPatchApplierProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void installVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version);
		// 
		void launchVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version);
		// 
		// Message sent by SPM to tell us to execute a comman locally
		void executeCommand(NLNET::IModule *sender, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_installVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_launchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_executeCommand(NLNET::CMessage &__message, const NLMISC::CSString &cmdline, const NLMISC::CSString &originator);
	



	};

}
	
#endif
