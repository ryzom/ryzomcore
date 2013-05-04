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

#ifndef MODULE_ADMIN_ITF_H
#define MODULE_ADMIN_ITF_H
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

#include "nel/misc/sstring.h"
	
#include "nel/misc/md5.h"
	
#include "nel/net/module_message.h"
	
#include "server_patch_types.h"
	
#include "file_manager.h"
	
namespace PATCHMAN
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFileReceiverSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CFileReceiverSkel>	TInterceptor;
	protected:
		CFileReceiverSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CFileReceiverSkel()
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

		typedef void (CFileReceiverSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void setupSubscriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void cbFileInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void cbFileData_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void cbFileDataFailure_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CFileReceiverSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void setupSubscriptions(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void cbFileInfo(NLNET::IModuleProxy *sender, const TFileInfoVector &files) =0;
		// 
		virtual void cbFileData(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data) =0;
		// 
		virtual void cbFileDataFailure(NLNET::IModuleProxy *sender, const std::string &fileName) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFileReceiverProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CFileReceiverSkel	*_LocalModuleSkel;


	public:
		CFileReceiverProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CFileReceiverSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CFileReceiverProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void setupSubscriptions(NLNET::IModule *sender);
		// 
		void cbFileInfo(NLNET::IModule *sender, const TFileInfoVector &files);
		// 
		void cbFileData(NLNET::IModule *sender, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data);
		// 
		void cbFileDataFailure(NLNET::IModule *sender, const std::string &fileName);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setupSubscriptions(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_cbFileInfo(NLNET::CMessage &__message, const TFileInfoVector &files);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_cbFileData(NLNET::CMessage &__message, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_cbFileDataFailure(NLNET::CMessage &__message, const std::string &fileName);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFileRepositorySkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CFileRepositorySkel>	TInterceptor;
	protected:
		CFileRepositorySkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CFileRepositorySkel()
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

		typedef void (CFileRepositorySkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void requestFileInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void requestFileData_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void getInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void subscribe_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void unsubscribe_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void unsubscribeAll_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CFileRepositorySkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// Request info concerning a particular file
		virtual void requestFileInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileName) =0;
		// Request a data block for a particular file
		virtual void requestFileData(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes) =0;
		// Ask for the info concerning files matching given filespec
		virtual void getInfo(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec) =0;
		// Ask for the info concerning files matching given filespec to be forwarded to me now
		// and for updates to be sent to me as they are generated
		virtual void subscribe(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec) =0;
		// Cancel subscription for given filespec
		virtual void unsubscribe(NLNET::IModuleProxy *sender, const NLMISC::CSString &fileSpec) =0;
		// Cancel all subscriptions for given filespec
		virtual void unsubscribeAll(NLNET::IModuleProxy *sender) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFileRepositoryProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CFileRepositorySkel	*_LocalModuleSkel;


	public:
		CFileRepositoryProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CFileRepositorySkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CFileRepositoryProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// Request info concerning a particular file
		void requestFileInfo(NLNET::IModule *sender, const NLMISC::CSString &fileName);
		// Request a data block for a particular file
		void requestFileData(NLNET::IModule *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes);
		// Ask for the info concerning files matching given filespec
		void getInfo(NLNET::IModule *sender, const NLMISC::CSString &fileSpec);
		// Ask for the info concerning files matching given filespec to be forwarded to me now
		// and for updates to be sent to me as they are generated
		void subscribe(NLNET::IModule *sender, const NLMISC::CSString &fileSpec);
		// Cancel subscription for given filespec
		void unsubscribe(NLNET::IModule *sender, const NLMISC::CSString &fileSpec);
		// Cancel all subscriptions for given filespec
		void unsubscribeAll(NLNET::IModule *sender);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_requestFileInfo(NLNET::CMessage &__message, const NLMISC::CSString &fileName);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_requestFileData(NLNET::CMessage &__message, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_getInfo(NLNET::CMessage &__message, const NLMISC::CSString &fileSpec);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_subscribe(NLNET::CMessage &__message, const NLMISC::CSString &fileSpec);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_unsubscribe(NLNET::CMessage &__message, const NLMISC::CSString &fileSpec);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_unsubscribeAll(NLNET::CMessage &__message);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdministeredModuleBaseSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CAdministeredModuleBaseSkel>	TInterceptor;
	protected:
		CAdministeredModuleBaseSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CAdministeredModuleBaseSkel()
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

		typedef void (CAdministeredModuleBaseSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void executeCommand_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void installVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void launchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CAdministeredModuleBaseSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		// Message sent by SPM module to request execution of a command
		virtual void executeCommand(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &cmdline) =0;
		// 
		virtual void installVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version) =0;
		// 
		virtual void launchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domainName, uint32 version) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CAdministeredModuleBaseProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CAdministeredModuleBaseSkel	*_LocalModuleSkel;


	public:
		CAdministeredModuleBaseProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CAdministeredModuleBaseSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CAdministeredModuleBaseProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		// Message sent by SPM module to request execution of a command
		void executeCommand(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &cmdline);
		// 
		void installVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version);
		// 
		void launchVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_executeCommand(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &cmdline);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_installVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_launchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchTerminalSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CServerPatchTerminalSkel>	TInterceptor;
	protected:
		CServerPatchTerminalSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerPatchTerminalSkel()
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

		typedef void (CServerPatchTerminalSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareModuleDown_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareDomainInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void ackVersionChange_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setInstallVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setLaunchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void executedCommandAck_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void executedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CServerPatchTerminalSkel>;
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
		// Message sent by SPM module to declare module down for a connected SPA / SPR / SPB type module
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
		// Message sent by SPM to inform us of the current installed version for a given domain
		virtual void setInstallVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version) =0;
		// 
		// Message sent by SPM to inform us of the current live version for a given domain
		virtual void setLaunchVersion(NLNET::IModuleProxy *sender, const NLMISC::CSString &domain, uint32 version) =0;
		// 
		// Message sent by SPM with result of command issuued via executeCommandOnModules()
		virtual void executedCommandAck(NLNET::IModuleProxy *sender, const NLMISC::CSString &result) =0;
		// 
		// Message sent by SPM with result of command issuued via executeCommandOnModules()
		virtual void executedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result) =0;


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
				CServerPatchTerminalSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
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
		// Message sent by SPM module to declare module down for a connected SPA / SPR / SPB type module
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
		// Message sent by SPM to inform us of the current installed version for a given domain
		void setInstallVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version);
		// 
		// Message sent by SPM to inform us of the current live version for a given domain
		void setLaunchVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version);
		// 
		// Message sent by SPM with result of command issuued via executeCommandOnModules()
		void executedCommandAck(NLNET::IModule *sender, const NLMISC::CSString &result);
		// 
		// Message sent by SPM with result of command issuued via executeCommandOnModules()
		void executedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);

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
		static const NLNET::CMessage &buildMessageFor_setInstallVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setLaunchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_executedCommandAck(NLNET::CMessage &__message, const NLMISC::CSString &result);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_executedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CServerPatchManagerSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CServerPatchManagerSkel>	TInterceptor;
	protected:
		CServerPatchManagerSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CServerPatchManagerSkel()
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

		typedef void (CServerPatchManagerSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void registerAdministeredModule_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void requestRefresh_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setInstallVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setLaunchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void executeCommandOnModules_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void executedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CServerPatchManagerSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		// Message sent by an administered module to register
		virtual void registerAdministeredModule(NLNET::IModuleProxy *sender, bool requireApplierUpdates, bool requireTerminalUpdates, bool requireDepCfgUpdates, bool isAdministered) =0;
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
		// Message sent by SPR / SPB / SPA type modules to declare their states
		// This message is forwarded to all connected SPT modules
		virtual void declareState(NLNET::IModuleProxy *sender, const NLMISC::CSString &state) =0;
		// 
		// Message sent by SPT module to define a new named version
		virtual void declareVersionName(NLNET::IModuleProxy *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion) =0;
		// 
		// Message sent by SPT module to request execution of a command on one or more modules
		// Note that the 'target' parameter may be a wildcard
		virtual void executeCommandOnModules(NLNET::IModuleProxy *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline) =0;
		// 
		// Message with result of command issuued via executeCommandOnSPA()
		virtual void executedCommandResult(NLNET::IModuleProxy *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result) =0;


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
				CServerPatchManagerSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
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
		// Message sent by an administered module to register
		void registerAdministeredModule(NLNET::IModule *sender, bool requireApplierUpdates, bool requireTerminalUpdates, bool requireDepCfgUpdates, bool isAdministered);
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
		// Message sent by SPR / SPB / SPA type modules to declare their states
		// This message is forwarded to all connected SPT modules
		void declareState(NLNET::IModule *sender, const NLMISC::CSString &state);
		// 
		// Message sent by SPT module to define a new named version
		void declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion);
		// 
		// Message sent by SPT module to request execution of a command on one or more modules
		// Note that the 'target' parameter may be a wildcard
		void executeCommandOnModules(NLNET::IModule *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline);
		// 
		// Message with result of command issuued via executeCommandOnSPA()
		void executedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerAdministeredModule(NLNET::CMessage &__message, bool requireApplierUpdates, bool requireTerminalUpdates, bool requireDepCfgUpdates, bool isAdministered);
	
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
		static const NLNET::CMessage &buildMessageFor_executeCommandOnModules(NLNET::CMessage &__message, const NLMISC::CSString &target, const NLMISC::CSString &commandline);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_executedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CDeploymentConfigurationSynchroniserSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CDeploymentConfigurationSynchroniserSkel>	TInterceptor;
	protected:
		CDeploymentConfigurationSynchroniserSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CDeploymentConfigurationSynchroniserSkel()
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

		typedef void (CDeploymentConfigurationSynchroniserSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void requestSync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void sync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CDeploymentConfigurationSynchroniserSkel>;
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
				CDeploymentConfigurationSynchroniserSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
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
