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
	
#include "backup_service_itf.h"

namespace BS
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CBackupServiceSkel::TMessageHandlerMap &CBackupServiceSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("BSSF"), &CBackupServiceSkel::saveFile_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("BSLF"), &CBackupServiceSkel::loadFile_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CBackupServiceSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CBackupServiceSkel::saveFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CBackupServiceSkel_saveFile_BSSF);
		std::string	fileName;
			nlRead(__message, serial, fileName);
		NLNET::TBinBuffer	data;
			nlRead(__message, serial, data);
		saveFile(sender, fileName, data);
	}

	void CBackupServiceSkel::loadFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CBackupServiceSkel_loadFile_BSLF);
		std::string	fileName;
			nlRead(__message, serial, fileName);
		uint32	requestId;
			nlRead(__message, serial, requestId);
		loadFile(sender, fileName, requestId);
	}
		// A module ask to save a file in the backup repository
	void CBackupServiceProxy::saveFile(NLNET::IModule *sender, const std::string &fileName, const NLNET::TBinBuffer &data)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->saveFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, data);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_saveFile(__message, fileName, data);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A module ask to load a file
	void CBackupServiceProxy::loadFile(NLNET::IModule *sender, const std::string &fileName, uint32 requestId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->loadFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, requestId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_loadFile(__message, fileName, requestId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CBackupServiceProxy::buildMessageFor_saveFile(NLNET::CMessage &__message, const std::string &fileName, const NLNET::TBinBuffer &data)
	{
		__message.setType("BSSF");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, const_cast < NLNET::TBinBuffer& > (data));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CBackupServiceProxy::buildMessageFor_loadFile(NLNET::CMessage &__message, const std::string &fileName, uint32 requestId)
	{
		__message.setType("BSLF");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, requestId);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CBackupServiceClientSkel::TMessageHandlerMap &CBackupServiceClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("BSLFR"), &CBackupServiceClientSkel::loadFileResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("BSFU"), &CBackupServiceClientSkel::fileUpdate_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CBackupServiceClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CBackupServiceClientSkel::loadFileResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CBackupServiceClientSkel_loadFileResult_BSLFR);
		uint32	requestId;
			nlRead(__message, serial, requestId);
		std::string	fileName;
			nlRead(__message, serial, fileName);
		uint32	fileTimeStamp;
			nlRead(__message, serial, fileTimeStamp);
		NLNET::TBinBuffer	data;
			nlRead(__message, serial, data);
		loadFileResult(sender, requestId, fileName, fileTimeStamp, data);
	}

	void CBackupServiceClientSkel::fileUpdate_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CBackupServiceClientSkel_fileUpdate_BSFU);
		std::string	fileName;
			nlRead(__message, serial, fileName);
		std::vector < std::string >	content;
			nlRead(__message, serialCont, content);
		fileUpdate(sender, fileName, content);
	}
		// The BS return for a load file request
	void CBackupServiceClientProxy::loadFileResult(NLNET::IModule *sender, uint32 requestId, const std::string &fileName, uint32 fileTimeStamp, const NLNET::TBinBuffer &data)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->loadFileResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), requestId, fileName, fileTimeStamp, data);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_loadFileResult(__message, requestId, fileName, fileTimeStamp, data);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A file listened by the client have been changed, BS resend the file content
	void CBackupServiceClientProxy::fileUpdate(NLNET::IModule *sender, const std::string &fileName, const std::vector < std::string > &content)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->fileUpdate(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, content);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_fileUpdate(__message, fileName, content);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CBackupServiceClientProxy::buildMessageFor_loadFileResult(NLNET::CMessage &__message, uint32 requestId, const std::string &fileName, uint32 fileTimeStamp, const NLNET::TBinBuffer &data)
	{
		__message.setType("BSLFR");
			nlWrite(__message, serial, requestId);
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, fileTimeStamp);
			nlWrite(__message, serial, const_cast < NLNET::TBinBuffer& > (data));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CBackupServiceClientProxy::buildMessageFor_fileUpdate(NLNET::CMessage &__message, const std::string &fileName, const std::vector < std::string > &content)
	{
		__message.setType("BSFU");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serialCont, const_cast < std::vector < std::string >& > (content));


		return __message;
	}

}
