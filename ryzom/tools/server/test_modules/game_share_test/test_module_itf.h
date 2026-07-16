
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef TEST_MODULE_INTERFACE
#define TEST_MODULE_INTERFACE
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

namespace TST_MOD_ITF
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CTestModuleInterfaceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CTestModuleInterfaceSkel>	TInterceptor;
	protected:
		CTestModuleInterfaceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CTestModuleInterfaceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

	public:


		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {};
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {};
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {};
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CTestModuleInterfaceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void noParam_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void twoWayInvoke_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CTestModuleInterfaceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		virtual void noParam(NLNET::IModuleProxy *sender) =0;
		virtual uint32 twoWayInvoke(NLNET::IModuleProxy *sender, uint32 value1, uint32 value2) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CTestModuleInterfaceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CTestModuleInterfaceSkel	*_LocalModuleSkel;


	public:
		CTestModuleInterfaceProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CTestModuleInterfaceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CTestModuleInterfaceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		void noParam(NLNET::IModule *sender);
		uint32 twoWayInvoke(NLNET::IModule *sender, uint32 value1, uint32 value2);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_noParam(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_twoWayInvoke(NLNET::CMessage &__message, uint32 value1, uint32 value2);
	



	};

}
	
#endif
