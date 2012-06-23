
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef PIPELINE_MODULE_PIPELINE_MASTER_ITF_H
#define PIPELINE_MODULE_PIPELINE_MASTER_ITF_H
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

namespace PIPELINE
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CModulePipelineMasterSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CModulePipelineMasterSkel>	TInterceptor;
	protected:
		CModulePipelineMasterSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CModulePipelineMasterSkel()
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

		typedef void (CModulePipelineMasterSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void slaveFinishedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void slaveAbortedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void slaveRefusedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void slaveReloadedSheets_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void slaveBuildReadySuccess_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void slaveBuildReadyFail_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void vectorPushString_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateDatabaseStatusByVector_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setAvailablePlugins_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CModulePipelineMasterSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void slaveFinishedBuildTask(NLNET::IModuleProxy *sender, uint8 errorLevel) =0;
		// 
		virtual void slaveAbortedBuildTask(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void slaveRefusedBuildTask(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void slaveReloadedSheets(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void slaveBuildReadySuccess(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void slaveBuildReadyFail(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void vectorPushString(NLNET::IModuleProxy *sender, const std::string &str) =0;
		// 
		virtual void updateDatabaseStatusByVector(NLNET::IModuleProxy *sender) =0;
		// 
		virtual void setAvailablePlugins(NLNET::IModuleProxy *sender, const std::vector<uint32> &pluginsAvailable) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CModulePipelineMasterProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CModulePipelineMasterSkel	*_LocalModuleSkel;


	public:
		CModulePipelineMasterProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CModulePipelineMasterSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CModulePipelineMasterProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void slaveFinishedBuildTask(NLNET::IModule *sender, uint8 errorLevel);
		// 
		void slaveAbortedBuildTask(NLNET::IModule *sender);
		// 
		void slaveRefusedBuildTask(NLNET::IModule *sender);
		// 
		void slaveReloadedSheets(NLNET::IModule *sender);
		// 
		void slaveBuildReadySuccess(NLNET::IModule *sender);
		// 
		void slaveBuildReadyFail(NLNET::IModule *sender);
		// 
		void vectorPushString(NLNET::IModule *sender, const std::string &str);
		// 
		void updateDatabaseStatusByVector(NLNET::IModule *sender);
		// 
		void setAvailablePlugins(NLNET::IModule *sender, const std::vector<uint32> &pluginsAvailable);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_slaveFinishedBuildTask(NLNET::CMessage &__message, uint8 errorLevel);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_slaveAbortedBuildTask(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_slaveRefusedBuildTask(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_slaveReloadedSheets(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_slaveBuildReadySuccess(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_slaveBuildReadyFail(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_vectorPushString(NLNET::CMessage &__message, const std::string &str);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateDatabaseStatusByVector(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setAvailablePlugins(NLNET::CMessage &__message, const std::vector<uint32> &pluginsAvailable);
	



	};

}
	
#endif
