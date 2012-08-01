
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "module_pipeline_slave_itf.h"

namespace PIPELINE
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CModulePipelineSlaveSkel::TMessageHandlerMap &CModulePipelineSlaveSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("M_HELLO"), &CModulePipelineSlaveSkel::submitToMaster_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("GO_BT"), &CModulePipelineSlaveSkel::startBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("ABORT_BT"), &CModulePipelineSlaveSkel::abortBuildTask_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("ADD_FS_CACHE"), &CModulePipelineSlaveSkel::addFileStatusToCache_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RE_UPD_DB_ST"), &CModulePipelineSlaveSkel::masterUpdatedDatabaseStatus_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("RELOAD_SHEETS"), &CModulePipelineSlaveSkel::reloadSheets_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("BRT_ENTER"), &CModulePipelineSlaveSkel::enterBuildReadyState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("BRT_LEAVE"), &CModulePipelineSlaveSkel::leaveBuildReadyState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;
	}
	bool CModulePipelineSlaveSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CModulePipelineSlaveSkel::submitToMaster_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_submitToMaster_M_HELLO);
		submitToMaster(sender);
	}

	void CModulePipelineSlaveSkel::startBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_startBuildTask_GO_BT);
		std::string	projectName;
			nlRead(__message, serial, projectName);
		uint32	pluginId;
			nlRead(__message, serial, pluginId);
		startBuildTask(sender, projectName, pluginId);
	}

	void CModulePipelineSlaveSkel::abortBuildTask_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_abortBuildTask_ABORT_BT);
		abortBuildTask(sender);
	}

	void CModulePipelineSlaveSkel::addFileStatusToCache_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_addFileStatusToCache_ADD_FS_CACHE);
		std::string	macroPath;
			nlRead(__message, serial, macroPath);
		CFileStatus	fileStatus;
			nlRead(__message, serial, fileStatus);
		addFileStatusToCache(sender, macroPath, fileStatus);
	}

	void CModulePipelineSlaveSkel::masterUpdatedDatabaseStatus_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_masterUpdatedDatabaseStatus_RE_UPD_DB_ST);
		masterUpdatedDatabaseStatus(sender);
	}

	void CModulePipelineSlaveSkel::reloadSheets_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_reloadSheets_RELOAD_SHEETS);
		reloadSheets(sender);
	}

	void CModulePipelineSlaveSkel::enterBuildReadyState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_enterBuildReadyState_BRT_ENTER);
		enterBuildReadyState(sender);
	}

	void CModulePipelineSlaveSkel::leaveBuildReadyState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CModulePipelineSlaveSkel_leaveBuildReadyState_BRT_LEAVE);
		leaveBuildReadyState(sender);
	}
		// 
	void CModulePipelineSlaveProxy::submitToMaster(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->submitToMaster(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_submitToMaster(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::startBuildTask(NLNET::IModule *sender, const std::string &projectName, uint32 pluginId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->startBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), projectName, pluginId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_startBuildTask(__message, projectName, pluginId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::abortBuildTask(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->abortBuildTask(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_abortBuildTask(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::addFileStatusToCache(NLNET::IModule *sender, const std::string &macroPath, const CFileStatus &fileStatus)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->addFileStatusToCache(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), macroPath, fileStatus);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_addFileStatusToCache(__message, macroPath, fileStatus);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::masterUpdatedDatabaseStatus(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->masterUpdatedDatabaseStatus(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_masterUpdatedDatabaseStatus(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::reloadSheets(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reloadSheets(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reloadSheets(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::enterBuildReadyState(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->enterBuildReadyState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_enterBuildReadyState(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CModulePipelineSlaveProxy::leaveBuildReadyState(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->leaveBuildReadyState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_leaveBuildReadyState(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_submitToMaster(NLNET::CMessage &__message)
	{
		__message.setType("M_HELLO");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_startBuildTask(NLNET::CMessage &__message, const std::string &projectName, uint32 pluginId)
	{
		__message.setType("GO_BT");
			nlWrite(__message, serial, const_cast < std::string& > (projectName));
			nlWrite(__message, serial, pluginId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_abortBuildTask(NLNET::CMessage &__message)
	{
		__message.setType("ABORT_BT");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_addFileStatusToCache(NLNET::CMessage &__message, const std::string &macroPath, const CFileStatus &fileStatus)
	{
		__message.setType("ADD_FS_CACHE");
			nlWrite(__message, serial, const_cast < std::string& > (macroPath));
			nlWrite(__message, serial, const_cast < CFileStatus& > (fileStatus));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_masterUpdatedDatabaseStatus(NLNET::CMessage &__message)
	{
		__message.setType("RE_UPD_DB_ST");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_reloadSheets(NLNET::CMessage &__message)
	{
		__message.setType("RELOAD_SHEETS");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_enterBuildReadyState(NLNET::CMessage &__message)
	{
		__message.setType("BRT_ENTER");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CModulePipelineSlaveProxy::buildMessageFor_leaveBuildReadyState(NLNET::CMessage &__message)
	{
		__message.setType("BRT_LEAVE");


		return __message;
	}

}
