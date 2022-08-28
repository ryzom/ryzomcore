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
	
#include "char_name_mapper_itf.h"

namespace CNM
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CCharNameMapperSkel::TMessageHandlerMap &CCharNameMapperSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("MCN"), &CCharNameMapperSkel::mapCharNames_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CCharNameMapperSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CCharNameMapperSkel::mapCharNames_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharNameMapperSkel_mapCharNames_MCN);
		std::vector < TCharNameInfo >	charNameInfos;
			nlRead(__message, serialCont, charNameInfos);
		mapCharNames(sender, charNameInfos);
	}
		// 
	void CCharNameMapperProxy::mapCharNames(NLNET::IModule *sender, const std::vector < TCharNameInfo > &charNameInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->mapCharNames(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charNameInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_mapCharNames(__message, charNameInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharNameMapperProxy::buildMessageFor_mapCharNames(NLNET::CMessage &__message, const std::vector < TCharNameInfo > &charNameInfos)
	{
		__message.setType("MCN");
			nlWrite(__message, serialCont, const_cast < std::vector < TCharNameInfo >& > (charNameInfos));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CCharNameMapperClientSkel::TMessageHandlerMap &CCharNameMapperClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("MCN"), &CCharNameMapperClientSkel::charNamesMapped_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CCharNameMapperClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CCharNameMapperClientSkel::charNamesMapped_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharNameMapperClientSkel_charNamesMapped_MCN);
		std::vector < TCharMappedInfo >	charMappedInfos;
			nlRead(__message, serialCont, charMappedInfos);
		charNamesMapped(sender, charMappedInfos);
	}
		// 
	void CCharNameMapperClientProxy::charNamesMapped(NLNET::IModule *sender, const std::vector < TCharMappedInfo > &charMappedInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->charNamesMapped(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charMappedInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_charNamesMapped(__message, charMappedInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharNameMapperClientProxy::buildMessageFor_charNamesMapped(NLNET::CMessage &__message, const std::vector < TCharMappedInfo > &charMappedInfos)
	{
		__message.setType("MCN");
			nlWrite(__message, serialCont, const_cast < std::vector < TCharMappedInfo >& > (charMappedInfos));


		return __message;
	}

}
