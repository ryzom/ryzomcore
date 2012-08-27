
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "module_pipeline_master_itf.h"

namespace PIPELINE
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CModulePipelineMasterSkel::TMessageHandlerMap &CModulePipelineMasterSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("RE_BT_DONE"), &CModulePipelineMasterSkel::slaveFinishedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_BT_ABORTED"), &CModulePipelineMasterSkel::slaveAbortedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_BT_REFUSED"), &CModulePipelineMasterSkel::slaveRefusedBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("TL_ERR_LOG"), &CModulePipelineMasterSkel::slaveLoggedToolError_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_SHEETS_OK"), &CModulePipelineMasterSkel::slaveReloadedSheets_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_BRT_OK"), &CModulePipelineMasterSkel::slaveBuildReadySuccess_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_BRT_FAIL"), &CModulePipelineMasterSkel::slaveBuildReadyFail_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("VEC_PUSH_STR"), &CModulePipelineMasterSkel::vectorPushString_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("UPD_DB_ST"), &CModulePipelineMasterSkel::updateDatabaseStatusByVector_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SET_A_PLG"), &CModulePipelineMasterSkel::setAvailablePlugins_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;
	}
	bool CModulePipelineMasterSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CModulePipelineMasterSkel::slaveFinishedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveFinishedBuildTask_RE_BT_DONE);
		uint8	errorLevel;
			nlRead(__message, serial, errorLevel);
		std::string	errorMessage;
			nlRead(__message, serial, errorMessage);
		slaveFinishedBuildTask(sender, errorLevel, errorMessage);
	}

	void CModulePipelineMasterSkel::slaveAbortedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveAbortedBuildTask_RE_BT_ABORTED);
		slaveAbortedBuildTask(sender);
	}

	void CModulePipelineMasterSkel::slaveRefusedBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveRefusedBuildTask_RE_BT_REFUSED);
		slaveRefusedBuildTask(sender);
	}

	void CModulePipelineMasterSkel::slaveLoggedToolError_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveLoggedToolError_TL_ERR_LOG);
		uint8	type;
			nlRead(__message, serial, type);
		std::string	macroPath;
			nlRead(__message, serial, macroPath);
		std::string	time;
			nlRead(__message, serial, time);
		std::string	error;
			nlRead(__message, serial, error);
		slaveLoggedToolError(sender, type, macroPath, time, error);
	}

	void CModulePipelineMasterSkel::slaveReloadedSheets_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveReloadedSheets_RE_SHEETS_OK);
		slaveReloadedSheets(sender);
	}

	void CModulePipelineMasterSkel::slaveBuildReadySuccess_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveBuildReadySuccess_RE_BRT_OK);
		slaveBuildReadySuccess(sender);
	}

	void CModulePipelineMasterSkel::slaveBuildReadyFail_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_slaveBuildReadyFail_RE_BRT_FAIL);
		slaveBuildReadyFail(sender);
	}

	void CModulePipelineMasterSkel::vectorPushString_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_vectorPushString_VEC_PUSH_STR);
		std::string	str;
			nlRead(__message, serial, str);
		vectorPushString(sender, str);
	}

	void CModulePipelineMasterSkel::updateDatabaseStatusByVector_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_updateDatabaseStatusByVector_UPD_DB_ST);
		updateDatabaseStatusByVector(sender);
	}

	void CModulePipelineMasterSkel::setAvailablePlugins_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineMasterSkel_setAvailablePlugins_SET_A_PLG);
		std::vector<uint32>	pluginsAvailable;
			nlRead(__message, serialCont, pluginsAvailable);
		setAvailablePlugins(sender, pluginsAvailable);
	}
		// 
	void CModulePipelineMasterProxy::slaveFinishedBuildTask(NLNET::IModule *sender, uint8 errorLevel, const std::string &errorMessage)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveFinishedBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), errorLevel, errorMessage);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveFinishedBuildTask(__message, errorLevel, errorMessage);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveAbortedBuildTask(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveAbortedBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveAbortedBuildTask(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveRefusedBuildTask(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveRefusedBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveRefusedBuildTask(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveLoggedToolError(NLNET::IModule *sender, uint8 type, const std::string &macroPath, const std::string &time, const std::string &error)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveLoggedToolError(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), type, macroPath, time, error);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveLoggedToolError(__message, type, macroPath, time, error);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveReloadedSheets(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveReloadedSheets(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveReloadedSheets(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveBuildReadySuccess(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveBuildReadySuccess(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveBuildReadySuccess(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::slaveBuildReadyFail(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->slaveBuildReadyFail(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_slaveBuildReadyFail(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::vectorPushString(NLNET::IModule *sender, const std::string &str)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->vectorPushString(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), str);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_vectorPushString(__message, str);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::updateDatabaseStatusByVector(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateDatabaseStatusByVector(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateDatabaseStatusByVector(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineMasterProxy::setAvailablePlugins(NLNET::IModule *sender, const std::vector<uint32> &pluginsAvailable)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setAvailablePlugins(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), pluginsAvailable);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setAvailablePlugins(__message, pluginsAvailable);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveFinishedBuildTask(NLNET::CMessage &__message, uint8 errorLevel, const std::string &errorMessage)
	{
		__message.setType("RE_BT_DONE");
			nlWrite(__message, serial, errorLevel);
			nlWrite(__message, serial, const_cast < std::string& > (errorMessage));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveAbortedBuildTask(NLNET::CMessage &__message)
	{
		__message.setType("RE_BT_ABORTED");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveRefusedBuildTask(NLNET::CMessage &__message)
	{
		__message.setType("RE_BT_REFUSED");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveLoggedToolError(NLNET::CMessage &__message, uint8 type, const std::string &macroPath, const std::string &time, const std::string &error)
	{
		__message.setType("TL_ERR_LOG");
			nlWrite(__message, serial, type);
			nlWrite(__message, serial, const_cast < std::string& > (macroPath));
			nlWrite(__message, serial, const_cast < std::string& > (time));
			nlWrite(__message, serial, const_cast < std::string& > (error));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveReloadedSheets(NLNET::CMessage &__message)
	{
		__message.setType("RE_SHEETS_OK");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveBuildReadySuccess(NLNET::CMessage &__message)
	{
		__message.setType("RE_BRT_OK");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_slaveBuildReadyFail(NLNET::CMessage &__message)
	{
		__message.setType("RE_BRT_FAIL");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_vectorPushString(NLNET::CMessage &__message, const std::string &str)
	{
		__message.setType("VEC_PUSH_STR");
			nlWrite(__message, serial, const_cast < std::string& > (str));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_updateDatabaseStatusByVector(NLNET::CMessage &__message)
	{
		__message.setType("UPD_DB_ST");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineMasterProxy::buildMessageFor_setAvailablePlugins(NLNET::CMessage &__message, const std::vector<uint32> &pluginsAvailable)
	{
		__message.setType("SET_A_PLG");
			nlWrite(__message, serialCont, const_cast < std::vector<uint32>& > (pluginsAvailable));


		return __message;
	}

}
