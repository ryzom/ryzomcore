
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef COMMAND_EXECUTOR_ITF
#define COMMAND_EXECUTOR_ITF
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
	
namespace CMDEXE
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCommandExecutorSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CCommandExecutorSkel>	TInterceptor;
	protected:
		CCommandExecutorSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CCommandExecutorSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {};
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {};
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {};
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CCommandExecutorSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void sendCommand_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CCommandExecutorSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void sendCommand(NLNET::IModuleProxy *sender, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCommandExecutorProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CCommandExecutorSkel	*_LocalModuleSkel;


	public:
		CCommandExecutorProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CCommandExecutorSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CCommandExecutorProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void sendCommand(NLNET::IModule *sender, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_sendCommand(NLNET::CMessage &__message, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg);
	



	};

}
	
#endif
