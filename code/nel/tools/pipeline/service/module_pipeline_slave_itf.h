
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef PIPELINE_MODULE_PIPELINE_SLAVE_ITF_H
#define PIPELINE_MODULE_PIPELINE_SLAVE_ITF_H
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

#include "metadata_storage.h"
	
namespace PIPELINE
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CModulePipelineSlaveSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CModulePipelineSlaveSkel>	TInterceptor;
	protected:
		CModulePipelineSlaveSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CModulePipelineSlaveSkel()
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

		typedef void (CModulePipelineSlaveSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void submitToMaster_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void startBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void abortBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void addFileStatusToCache_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void masterUpdatedDatabaseStatus_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reloadSheets_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void enterBuildReadyState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void leaveBuildReadyState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CModulePipelineSlaveSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void submitToMaster(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void startBuildTask(NLNET::IModuleProxy *sender, const std::string &projectName, uint32 pluginId) =0;
		// 
		virtual void abortBuildTask(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void addFileStatusToCache(NLNET::IModuleProxy *sender, const std::string &macroPath, const CFileStatus &fileStatus) =0;
		// 
		virtual void masterUpdatedDatabaseStatus(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void reloadSheets(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void enterBuildReadyState(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void leaveBuildReadyState(NLNET::IModuleProxy *sender) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CModulePipelineSlaveProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CModulePipelineSlaveSkel	*_LocalModuleSkel;


	public:
		CModulePipelineSlaveProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CModulePipelineSlaveSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CModulePipelineSlaveProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void submitToMaster(NLNET::IModule *sender);
		// 
		void startBuildTask(NLNET::IModule *sender, const std::string &projectName, uint32 pluginId);
		// 
		void abortBuildTask(NLNET::IModule *sender);
		// 
		void addFileStatusToCache(NLNET::IModule *sender, const std::string &macroPath, const CFileStatus &fileStatus);
		// 
		void masterUpdatedDatabaseStatus(NLNET::IModule *sender);
		// 
		void reloadSheets(NLNET::IModule *sender);
		// 
		void enterBuildReadyState(NLNET::IModule *sender);
		// 
		void leaveBuildReadyState(NLNET::IModule *sender);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_submitToMaster(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_startBuildTask(NLNET::CMessage &__message, const std::string &projectName, uint32 pluginId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_abortBuildTask(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_addFileStatusToCache(NLNET::CMessage &__message, const std::string &macroPath, const CFileStatus &fileStatus);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_masterUpdatedDatabaseStatus(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reloadSheets(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_enterBuildReadyState(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_leaveBuildReadyState(NLNET::CMessage &__message);
	



	};

}
	
#endif
