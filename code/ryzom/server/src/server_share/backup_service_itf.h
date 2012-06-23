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

#ifndef BACKUP_SERVICE_ITF
#define BACKUP_SERVICE_ITF
#include "nel/misc/types_nl.h"
#ifdef NL_COMP_VC8
  #include <memory>
#endif
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/entity_id.h"
	
namespace BS
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CBackupServiceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CBackupServiceSkel>	TInterceptor;
	protected:
		CBackupServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CBackupServiceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CBackupServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void saveFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void loadFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CBackupServiceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// A module ask to save a file in the backup repository
		virtual void saveFile(NLNET::IModuleProxy *sender, const std::string &fileName, const NLNET::TBinBuffer &data) =0;
		// A module ask to load a file
		virtual void loadFile(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 requestId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CBackupServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CBackupServiceSkel	*_LocalModuleSkel;


	public:
		CBackupServiceProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CBackupServiceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CBackupServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// A module ask to save a file in the backup repository
		void saveFile(NLNET::IModule *sender, const std::string &fileName, const NLNET::TBinBuffer &data);
		// A module ask to load a file
		void loadFile(NLNET::IModule *sender, const std::string &fileName, uint32 requestId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_saveFile(NLNET::CMessage &__message, const std::string &fileName, const NLNET::TBinBuffer &data);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_loadFile(NLNET::CMessage &__message, const std::string &fileName, uint32 requestId);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CBackupServiceClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CBackupServiceClientSkel>	TInterceptor;
	protected:
		CBackupServiceClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CBackupServiceClientSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CBackupServiceClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void loadFileResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void fileUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CBackupServiceClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// The BS return for a load file request
		virtual void loadFileResult(NLNET::IModuleProxy *sender, uint32 requestId, const std::string &fileName, uint32 fileTimeStamp, const NLNET::TBinBuffer &data) =0;
		// A file listened by the client have been changed, BS resend the file content
		virtual void fileUpdate(NLNET::IModuleProxy *sender, const std::string &fileName, const std::vector < std::string > &content) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CBackupServiceClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CBackupServiceClientSkel	*_LocalModuleSkel;


	public:
		CBackupServiceClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CBackupServiceClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CBackupServiceClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// The BS return for a load file request
		void loadFileResult(NLNET::IModule *sender, uint32 requestId, const std::string &fileName, uint32 fileTimeStamp, const NLNET::TBinBuffer &data);
		// A file listened by the client have been changed, BS resend the file content
		void fileUpdate(NLNET::IModule *sender, const std::string &fileName, const std::vector < std::string > &content);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_loadFileResult(NLNET::CMessage &__message, uint32 requestId, const std::string &fileName, uint32 fileTimeStamp, const NLNET::TBinBuffer &data);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileUpdate(NLNET::CMessage &__message, const std::string &fileName, const std::vector < std::string > &content);
	



	};

}
	
#endif
