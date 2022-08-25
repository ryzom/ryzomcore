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
	
#include "logger_service_itf.h"

namespace LGS
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CLoggerServiceSkel::TMessageHandlerMap &CLoggerServiceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("RC"), &CLoggerServiceSkel::registerClient_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("LG"), &CLoggerServiceSkel::reportLog_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CLoggerServiceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CLoggerServiceSkel::registerClient_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CLoggerServiceSkel_registerClient_RC);
		uint32	shardId;
			nlRead(__message, serial, shardId);
		std::vector < TLogDefinition >	logDef;
			nlRead(__message, serialCont, logDef);
		registerClient(sender, shardId, logDef);
	}

	void CLoggerServiceSkel::reportLog_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CLoggerServiceSkel_reportLog_LG);
		std::vector < TLogInfo >	logInfos;
			nlRead(__message, serialCont, logInfos);
		reportLog(sender, logInfos);
	}
		// A logger client register itself wy providing it's definition of 
		// the log content. It is mandatory that ALL client share
		// Exactly the same definition of log.
	void CLoggerServiceProxy::registerClient(NLNET::IModule *sender, uint32 shardId, const std::vector < TLogDefinition > &logDef)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerClient(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId, logDef);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_registerClient(__message, shardId, logDef);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A client send a log
	void CLoggerServiceProxy::reportLog(NLNET::IModule *sender, const std::vector < TLogInfo > &logInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportLog(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), logInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_reportLog(__message, logInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CLoggerServiceProxy::buildMessageFor_registerClient(NLNET::CMessage &__message, uint32 shardId, const std::vector < TLogDefinition > &logDef)
	{
		__message.setType("RC");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serialCont, const_cast < std::vector < TLogDefinition >& > (logDef));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CLoggerServiceProxy::buildMessageFor_reportLog(NLNET::CMessage &__message, const std::vector < TLogInfo > &logInfos)
	{
		__message.setType("LG");
			nlWrite(__message, serialCont, const_cast < std::vector < TLogInfo >& > (logInfos));


		return __message;
	}

}
