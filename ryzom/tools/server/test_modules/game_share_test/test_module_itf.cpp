
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "test_module_itf.h"

namespace TST_MOD_ITF
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CTestModuleInterfaceSkel::TMessageHandlerMap &CTestModuleInterfaceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("TMI_NP"), &CTestModuleInterfaceSkel::noParam_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("TMI_TWI"), &CTestModuleInterfaceSkel::twoWayInvoke_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CTestModuleInterfaceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CTestModuleInterfaceSkel::noParam_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CTestModuleInterfaceSkel_noParam_TMI_NP);
		noParam(sender);
	}

	void CTestModuleInterfaceSkel::twoWayInvoke_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CTestModuleInterfaceSkel_twoWayInvoke_TMI_TWI);
		uint32	value1;
			nlRead(__message, serial, value1);
		uint32	value2;
			nlRead(__message, serial, value2);
		uint32 __ret = twoWayInvoke(sender, value1, value2);
		// encode the return message
		NLNET::CMessage __retMsg;
		__retMsg.setType("R_TMI_TWI", NLNET::CMessage::Response);
		nlWrite(__retMsg, serial, __ret);

		// and send back the response
		sender->sendModuleMessage(static_cast<NLNET::IModule*>(_Interceptor.getRegistrar()), __retMsg);

	}
	void CTestModuleInterfaceProxy::noParam(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->noParam(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_noParam(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
	uint32 CTestModuleInterfaceProxy::twoWayInvoke(NLNET::IModule *sender, uint32 value1, uint32 value2)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			return _LocalModuleSkel->twoWayInvoke(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), value1, value2);
		}
		else
		{
			// send the message for remote dispatching and execution 

			NLNET::CMessage __message;
			
			buildMessageFor_twoWayInvoke(__message, value1, value2);

			NLNET::CMessage __retMsg;
			sender->invokeModuleOperation(_ModuleProxy, __message, __retMsg);

			// check the return message type
			if (__retMsg.getName() != "R_TMI_TWI")
				throw NLNET::IModule::EInvokeBadReturn();

			uint32 __ret;
			nlRead(__retMsg, serial, __ret);

			return __ret;
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CTestModuleInterfaceProxy::buildMessageFor_noParam(NLNET::CMessage &__message)
	{
		__message.setType("TMI_NP");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CTestModuleInterfaceProxy::buildMessageFor_twoWayInvoke(NLNET::CMessage &__message, uint32 value1, uint32 value2)
	{
		__message.setType("TMI_TWI", NLNET::CMessage::Request);
			nlWrite(__message, serial, value1);
			nlWrite(__message, serial, value2);


		return __message;
	}

}
