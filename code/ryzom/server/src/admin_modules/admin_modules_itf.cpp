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

#include "stdpch.h"
#include "admin_modules_itf.h"

namespace ADMIN
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CAdminServiceSkel::TMessageHandlerMap &CAdminServiceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("USU"), &CAdminServiceSkel::upServiceUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("GU"), &CAdminServiceSkel::graphUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("HRGU"), &CAdminServiceSkel::highRezGraphUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("CR"), &CAdminServiceSkel::commandResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CAdminServiceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CAdminServiceSkel::upServiceUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminServiceSkel_upServiceUpdate_USU);
		std::vector < TServiceStatus >	serviceStatus;
			nlRead(__message, serialCont, serviceStatus);
		upServiceUpdate(sender, serviceStatus);
	}

	void CAdminServiceSkel::graphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminServiceSkel_graphUpdate_GU);
		TGraphDatas	graphDatas;
			nlRead(__message, serial, graphDatas);
		graphUpdate(sender, graphDatas);
	}

	void CAdminServiceSkel::highRezGraphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminServiceSkel_highRezGraphUpdate_HRGU);
		THighRezDatas	graphDatas;
			nlRead(__message, serial, graphDatas);
		highRezGraphUpdate(sender, graphDatas);
	}

	void CAdminServiceSkel::commandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminServiceSkel_commandResult_CR);
		uint32	commandId;
			nlRead(__message, serial, commandId);
		std::string	serviceAlias;
			nlRead(__message, serial, serviceAlias);
		std::string	result;
			nlRead(__message, serial, result);
		commandResult(sender, commandId, serviceAlias, result);
	}
		// An AES send an update of the list of service up
	void CAdminServiceProxy::upServiceUpdate(NLNET::IModule *sender, const std::vector < TServiceStatus > &serviceStatus)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->upServiceUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), serviceStatus);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_upServiceUpdate(__message, serviceStatus);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// An AES send graph data update
	void CAdminServiceProxy::graphUpdate(NLNET::IModule *sender, const TGraphDatas &graphDatas)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->graphUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), graphDatas);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_graphUpdate(__message, graphDatas);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// An AES send high rez graph data update
	void CAdminServiceProxy::highRezGraphUpdate(NLNET::IModule *sender, const THighRezDatas &graphDatas)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->highRezGraphUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), graphDatas);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_highRezGraphUpdate(__message, graphDatas);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AES send back the result of execution of a command
	void CAdminServiceProxy::commandResult(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->commandResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), commandId, serviceAlias, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_commandResult(__message, commandId, serviceAlias, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminServiceProxy::buildMessageFor_upServiceUpdate(NLNET::CMessage &__message, const std::vector < TServiceStatus > &serviceStatus)
	{
		__message.setType("USU");
			nlWrite(__message, serialCont, const_cast < std::vector < TServiceStatus >& > (serviceStatus));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminServiceProxy::buildMessageFor_graphUpdate(NLNET::CMessage &__message, const TGraphDatas &graphDatas)
	{
		__message.setType("GU");
			nlWrite(__message, serial, const_cast < TGraphDatas& > (graphDatas));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminServiceProxy::buildMessageFor_highRezGraphUpdate(NLNET::CMessage &__message, const THighRezDatas &graphDatas)
	{
		__message.setType("HRGU");
			nlWrite(__message, serial, const_cast < THighRezDatas& > (graphDatas));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminServiceProxy::buildMessageFor_commandResult(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &result)
	{
		__message.setType("CR");
			nlWrite(__message, serial, commandId);
			nlWrite(__message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(__message, serial, const_cast < std::string& > (result));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CAdminExecutorServiceSkel::TMessageHandlerMap &CAdminExecutorServiceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SCO"), &CAdminExecutorServiceSkel::setShardOrders_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SDS"), &CAdminExecutorServiceSkel::shutdownShard_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("CC"), &CAdminExecutorServiceSkel::controlCmd_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SCMD"), &CAdminExecutorServiceSkel::serviceCmd_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("CR"), &CAdminExecutorServiceSkel::commandResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("GU"), &CAdminExecutorServiceSkel::graphUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("HRGU"), &CAdminExecutorServiceSkel::highRezGraphUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SSU"), &CAdminExecutorServiceSkel::serviceStatusUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CAdminExecutorServiceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CAdminExecutorServiceSkel::setShardOrders_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_setShardOrders_SCO);
		std::string	shardName;
			nlRead(__message, serial, shardName);
		TShardOrders	shardOrders;
			nlRead(__message, serial, shardOrders);
		setShardOrders(sender, shardName, shardOrders);
	}

	void CAdminExecutorServiceSkel::shutdownShard_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_shutdownShard_SDS);
		std::string	shardName;
			nlRead(__message, serial, shardName);
		uint32	delay;
			nlRead(__message, serial, delay);
		shutdownShard(sender, shardName, delay);
	}

	void CAdminExecutorServiceSkel::controlCmd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_controlCmd_CC);
		uint32	commandId;
			nlRead(__message, serial, commandId);
		std::string	serviceAlias;
			nlRead(__message, serial, serviceAlias);
		std::string	command;
			nlRead(__message, serial, command);
		controlCmd(sender, commandId, serviceAlias, command);
	}

	void CAdminExecutorServiceSkel::serviceCmd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_serviceCmd_SCMD);
		uint32	commandId;
			nlRead(__message, serial, commandId);
		std::string	serviceAlias;
			nlRead(__message, serial, serviceAlias);
		std::string	command;
			nlRead(__message, serial, command);
		serviceCmd(sender, commandId, serviceAlias, command);
	}

	void CAdminExecutorServiceSkel::commandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_commandResult_CR);
		uint32	commandId;
			nlRead(__message, serial, commandId);
		std::string	serviceAlias;
			nlRead(__message, serial, serviceAlias);
		std::string	result;
			nlRead(__message, serial, result);
		commandResult(sender, commandId, serviceAlias, result);
	}

	void CAdminExecutorServiceSkel::graphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_graphUpdate_GU);
		TGraphDatas	graphDatas;
			nlRead(__message, serial, graphDatas);
		graphUpdate(sender, graphDatas);
	}

	void CAdminExecutorServiceSkel::highRezGraphUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_highRezGraphUpdate_HRGU);
		THighRezDatas	graphDatas;
			nlRead(__message, serial, graphDatas);
		highRezGraphUpdate(sender, graphDatas);
	}

	void CAdminExecutorServiceSkel::serviceStatusUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceSkel_serviceStatusUpdate_SSU);
		std::string	status;
			nlRead(__message, serial, status);
		serviceStatusUpdate(sender, status);
	}
		// AS send orders for a shard
	void CAdminExecutorServiceProxy::setShardOrders(NLNET::IModule *sender, const std::string &shardName, const TShardOrders &shardOrders)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setShardOrders(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardName, shardOrders);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setShardOrders(__message, shardName, shardOrders);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AS send a command to shutdown a shard with a delay
	void CAdminExecutorServiceProxy::shutdownShard(NLNET::IModule *sender, const std::string &shardName, uint32 delay)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->shutdownShard(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardName, delay);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_shutdownShard(__message, shardName, delay);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AS send a control command to this AES
	void CAdminExecutorServiceProxy::controlCmd(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->controlCmd(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), commandId, serviceAlias, command);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_controlCmd(__message, commandId, serviceAlias, command);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Send a command to a service.
	void CAdminExecutorServiceProxy::serviceCmd(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &command)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->serviceCmd(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), commandId, serviceAlias, command);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_serviceCmd(__message, commandId, serviceAlias, command);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// AES client send back the result of execution of a command
	void CAdminExecutorServiceProxy::commandResult(NLNET::IModule *sender, uint32 commandId, const std::string &serviceAlias, const std::string &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->commandResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), commandId, serviceAlias, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_commandResult(__message, commandId, serviceAlias, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A service send graph data update
	void CAdminExecutorServiceProxy::graphUpdate(NLNET::IModule *sender, const TGraphDatas &graphDatas)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->graphUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), graphDatas);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_graphUpdate(__message, graphDatas);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A service high rez graph data update
	void CAdminExecutorServiceProxy::highRezGraphUpdate(NLNET::IModule *sender, const THighRezDatas &graphDatas)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->highRezGraphUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), graphDatas);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_highRezGraphUpdate(__message, graphDatas);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A service send an update of of it's status string
	void CAdminExecutorServiceProxy::serviceStatusUpdate(NLNET::IModule *sender, const std::string &status)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->serviceStatusUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), status);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_serviceStatusUpdate(__message, status);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_setShardOrders(NLNET::CMessage &__message, const std::string &shardName, const TShardOrders &shardOrders)
	{
		__message.setType("SCO");
			nlWrite(__message, serial, const_cast < std::string& > (shardName));
			nlWrite(__message, serial, shardOrders);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_shutdownShard(NLNET::CMessage &__message, const std::string &shardName, uint32 delay)
	{
		__message.setType("SDS");
			nlWrite(__message, serial, const_cast < std::string& > (shardName));
			nlWrite(__message, serial, delay);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_controlCmd(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &command)
	{
		__message.setType("CC");
			nlWrite(__message, serial, commandId);
			nlWrite(__message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(__message, serial, const_cast < std::string& > (command));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_serviceCmd(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &command)
	{
		__message.setType("SCMD");
			nlWrite(__message, serial, commandId);
			nlWrite(__message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(__message, serial, const_cast < std::string& > (command));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_commandResult(NLNET::CMessage &__message, uint32 commandId, const std::string &serviceAlias, const std::string &result)
	{
		__message.setType("CR");
			nlWrite(__message, serial, commandId);
			nlWrite(__message, serial, const_cast < std::string& > (serviceAlias));
			nlWrite(__message, serial, const_cast < std::string& > (result));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_graphUpdate(NLNET::CMessage &__message, const TGraphDatas &graphDatas)
	{
		__message.setType("GU");
			nlWrite(__message, serial, const_cast < TGraphDatas& > (graphDatas));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_highRezGraphUpdate(NLNET::CMessage &__message, const THighRezDatas &graphDatas)
	{
		__message.setType("HRGU");
			nlWrite(__message, serial, const_cast < THighRezDatas& > (graphDatas));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceProxy::buildMessageFor_serviceStatusUpdate(NLNET::CMessage &__message, const std::string &status)
	{
		__message.setType("SSU");
			nlWrite(__message, serial, const_cast < std::string& > (status));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CAdminExecutorServiceClientSkel::TMessageHandlerMap &CAdminExecutorServiceClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SCMD"), &CAdminExecutorServiceClientSkel::serviceCmd_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SCMDNR"), &CAdminExecutorServiceClientSkel::serviceCmdNoReturn_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CAdminExecutorServiceClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CAdminExecutorServiceClientSkel::serviceCmd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceClientSkel_serviceCmd_SCMD);
		uint32	commandId;
			nlRead(__message, serial, commandId);
		std::string	command;
			nlRead(__message, serial, command);
		serviceCmd(sender, commandId, command);
	}

	void CAdminExecutorServiceClientSkel::serviceCmdNoReturn_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdminExecutorServiceClientSkel_serviceCmdNoReturn_SCMDNR);
		std::string	command;
			nlRead(__message, serial, command);
		serviceCmdNoReturn(sender, command);
	}
		// execute a command and return the result.
	void CAdminExecutorServiceClientProxy::serviceCmd(NLNET::IModule *sender, uint32 commandId, const std::string &command)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->serviceCmd(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), commandId, command);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_serviceCmd(__message, commandId, command);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Send a command to a service without waiting for the return value.
	void CAdminExecutorServiceClientProxy::serviceCmdNoReturn(NLNET::IModule *sender, const std::string &command)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->serviceCmdNoReturn(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), command);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_serviceCmdNoReturn(__message, command);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceClientProxy::buildMessageFor_serviceCmd(NLNET::CMessage &__message, uint32 commandId, const std::string &command)
	{
		__message.setType("SCMD");
			nlWrite(__message, serial, commandId);
			nlWrite(__message, serial, const_cast < std::string& > (command));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdminExecutorServiceClientProxy::buildMessageFor_serviceCmdNoReturn(NLNET::CMessage &__message, const std::string &command)
	{
		__message.setType("SCMDNR");
			nlWrite(__message, serial, const_cast < std::string& > (command));


		return __message;
	}

}
