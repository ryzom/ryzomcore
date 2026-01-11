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

#include "module_admin_itf.h"

namespace PATCHMAN
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CFileReceiverSkel::TMessageHandlerMap &CFileReceiverSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("FR_SETUP_SUBS"), &CFileReceiverSkel::setupSubscriptions_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_FILE_INFO"), &CFileReceiverSkel::cbFileInfo_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_FILE_DATA"), &CFileReceiverSkel::cbFileData_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_FILE_ERR"), &CFileReceiverSkel::cbFileDataFailure_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CFileReceiverSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CFileReceiverSkel::setupSubscriptions_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileReceiverSkel_setupSubscriptions_FR_SETUP_SUBS);
		setupSubscriptions(sender);
	}

	void CFileReceiverSkel::cbFileInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileReceiverSkel_cbFileInfo_FR_FILE_INFO);
		TFileInfoVector	files;
			nlRead(__message, serialCont, files);
		cbFileInfo(sender, files);
	}

	void CFileReceiverSkel::cbFileData_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileReceiverSkel_cbFileData_FR_FILE_DATA);
		std::string	fileName;
			nlRead(__message, serial, fileName);
		uint32	startOffset;
			nlRead(__message, serial, startOffset);
		NLNET::TBinBuffer	data;
			nlRead(__message, serial, data);
		cbFileData(sender, fileName, startOffset, data);
	}

	void CFileReceiverSkel::cbFileDataFailure_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileReceiverSkel_cbFileDataFailure_FR_FILE_ERR);
		std::string	fileName;
			nlRead(__message, serial, fileName);
		cbFileDataFailure(sender, fileName);
	}
		// 
	void CFileReceiverProxy::setupSubscriptions(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setupSubscriptions(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setupSubscriptions(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CFileReceiverProxy::cbFileInfo(NLNET::IModule *sender, const TFileInfoVector &files)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->cbFileInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), files);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_cbFileInfo(__message, files);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CFileReceiverProxy::cbFileData(NLNET::IModule *sender, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->cbFileData(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, startOffset, data);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_cbFileData(__message, fileName, startOffset, data);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CFileReceiverProxy::cbFileDataFailure(NLNET::IModule *sender, const std::string &fileName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->cbFileDataFailure(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_cbFileDataFailure(__message, fileName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileReceiverProxy::buildMessageFor_setupSubscriptions(NLNET::CMessage &__message)
	{
		__message.setType("FR_SETUP_SUBS");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileReceiverProxy::buildMessageFor_cbFileInfo(NLNET::CMessage &__message, const TFileInfoVector &files)
	{
		__message.setType("FR_FILE_INFO");
			nlWrite(__message, serialCont, const_cast < TFileInfoVector& > (files));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileReceiverProxy::buildMessageFor_cbFileData(NLNET::CMessage &__message, const std::string &fileName, uint32 startOffset, const NLNET::TBinBuffer &data)
	{
		__message.setType("FR_FILE_DATA");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, startOffset);
			nlWrite(__message, serial, const_cast < NLNET::TBinBuffer& > (data));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileReceiverProxy::buildMessageFor_cbFileDataFailure(NLNET::CMessage &__message, const std::string &fileName)
	{
		__message.setType("FR_FILE_ERR");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CFileRepositorySkel::TMessageHandlerMap &CFileRepositorySkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("FR_REQUEST_INFO"), &CFileRepositorySkel::requestFileInfo_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_REQUEST_DATA"), &CFileRepositorySkel::requestFileData_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_GET_INFO"), &CFileRepositorySkel::getInfo_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_SUBSCRIBE"), &CFileRepositorySkel::subscribe_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_UNSUBSCRIBE"), &CFileRepositorySkel::unsubscribe_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("FR_UNSUBSCRIBE_ALL"), &CFileRepositorySkel::unsubscribeAll_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CFileRepositorySkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CFileRepositorySkel::requestFileInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileRepositorySkel_requestFileInfo_FR_REQUEST_INFO);
		NLMISC::CSString	fileName;
			nlRead(__message, serial, fileName);
		requestFileInfo(sender, fileName);
	}

	void CFileRepositorySkel::requestFileData_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileRepositorySkel_requestFileData_FR_REQUEST_DATA);
		NLMISC::CSString	fileName;
			nlRead(__message, serial, fileName);
		uint32	startOffset;
			nlRead(__message, serial, startOffset);
		uint32	numBytes;
			nlRead(__message, serial, numBytes);
		requestFileData(sender, fileName, startOffset, numBytes);
	}

	void CFileRepositorySkel::getInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileRepositorySkel_getInfo_FR_GET_INFO);
		NLMISC::CSString	fileSpec;
			nlRead(__message, serial, fileSpec);
		getInfo(sender, fileSpec);
	}

	void CFileRepositorySkel::subscribe_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileRepositorySkel_subscribe_FR_SUBSCRIBE);
		NLMISC::CSString	fileSpec;
			nlRead(__message, serial, fileSpec);
		subscribe(sender, fileSpec);
	}

	void CFileRepositorySkel::unsubscribe_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileRepositorySkel_unsubscribe_FR_UNSUBSCRIBE);
		NLMISC::CSString	fileSpec;
			nlRead(__message, serial, fileSpec);
		unsubscribe(sender, fileSpec);
	}

	void CFileRepositorySkel::unsubscribeAll_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CFileRepositorySkel_unsubscribeAll_FR_UNSUBSCRIBE_ALL);
		unsubscribeAll(sender);
	}
		// Request info concerning a particular file
	void CFileRepositoryProxy::requestFileInfo(NLNET::IModule *sender, const NLMISC::CSString &fileName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->requestFileInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_requestFileInfo(__message, fileName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Request a data block for a particular file
	void CFileRepositoryProxy::requestFileData(NLNET::IModule *sender, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->requestFileData(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, startOffset, numBytes);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_requestFileData(__message, fileName, startOffset, numBytes);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask for the info concerning files matching given filespec
	void CFileRepositoryProxy::getInfo(NLNET::IModule *sender, const NLMISC::CSString &fileSpec)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->getInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileSpec);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_getInfo(__message, fileSpec);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask for the info concerning files matching given filespec to be forwarded to me now
		// and for updates to be sent to me as they are generated
	void CFileRepositoryProxy::subscribe(NLNET::IModule *sender, const NLMISC::CSString &fileSpec)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->subscribe(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileSpec);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_subscribe(__message, fileSpec);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Cancel subscription for given filespec
	void CFileRepositoryProxy::unsubscribe(NLNET::IModule *sender, const NLMISC::CSString &fileSpec)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->unsubscribe(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileSpec);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_unsubscribe(__message, fileSpec);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Cancel all subscriptions for given filespec
	void CFileRepositoryProxy::unsubscribeAll(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->unsubscribeAll(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_unsubscribeAll(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileRepositoryProxy::buildMessageFor_requestFileInfo(NLNET::CMessage &__message, const NLMISC::CSString &fileName)
	{
		__message.setType("FR_REQUEST_INFO");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (fileName));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileRepositoryProxy::buildMessageFor_requestFileData(NLNET::CMessage &__message, const NLMISC::CSString &fileName, uint32 startOffset, uint32 numBytes)
	{
		__message.setType("FR_REQUEST_DATA");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (fileName));
			nlWrite(__message, serial, startOffset);
			nlWrite(__message, serial, numBytes);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileRepositoryProxy::buildMessageFor_getInfo(NLNET::CMessage &__message, const NLMISC::CSString &fileSpec)
	{
		__message.setType("FR_GET_INFO");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (fileSpec));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileRepositoryProxy::buildMessageFor_subscribe(NLNET::CMessage &__message, const NLMISC::CSString &fileSpec)
	{
		__message.setType("FR_SUBSCRIBE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (fileSpec));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileRepositoryProxy::buildMessageFor_unsubscribe(NLNET::CMessage &__message, const NLMISC::CSString &fileSpec)
	{
		__message.setType("FR_UNSUBSCRIBE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (fileSpec));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CFileRepositoryProxy::buildMessageFor_unsubscribeAll(NLNET::CMessage &__message)
	{
		__message.setType("FR_UNSUBSCRIBE_ALL");


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CAdministeredModuleBaseSkel::TMessageHandlerMap &CAdministeredModuleBaseSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("ADMIN_EXEC"), &CAdministeredModuleBaseSkel::executeCommand_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("ADMIN_SETNEXT"), &CAdministeredModuleBaseSkel::installVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("ADMIN_SETLIVE"), &CAdministeredModuleBaseSkel::launchVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CAdministeredModuleBaseSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CAdministeredModuleBaseSkel::executeCommand_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdministeredModuleBaseSkel_executeCommand_ADMIN_EXEC);
		NLMISC::CSString	originator;
			nlRead(__message, serial, originator);
		NLMISC::CSString	cmdline;
			nlRead(__message, serial, cmdline);
		executeCommand(sender, originator, cmdline);
	}

	void CAdministeredModuleBaseSkel::installVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdministeredModuleBaseSkel_installVersion_ADMIN_SETNEXT);
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		uint32	version;
			nlRead(__message, serial, version);
		installVersion(sender, domainName, version);
	}

	void CAdministeredModuleBaseSkel::launchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CAdministeredModuleBaseSkel_launchVersion_ADMIN_SETLIVE);
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		uint32	version;
			nlRead(__message, serial, version);
		launchVersion(sender, domainName, version);
	}
		// 
		// Message sent by SPM module to request execution of a command
	void CAdministeredModuleBaseProxy::executeCommand(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &cmdline)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executeCommand(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), originator, cmdline);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executeCommand(__message, originator, cmdline);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CAdministeredModuleBaseProxy::installVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->installVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_installVersion(__message, domainName, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
	void CAdministeredModuleBaseProxy::launchVersion(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->launchVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_launchVersion(__message, domainName, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdministeredModuleBaseProxy::buildMessageFor_executeCommand(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &cmdline)
	{
		__message.setType("ADMIN_EXEC");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (originator));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (cmdline));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdministeredModuleBaseProxy::buildMessageFor_installVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version)
	{
		__message.setType("ADMIN_SETNEXT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CAdministeredModuleBaseProxy::buildMessageFor_launchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 version)
	{
		__message.setType("ADMIN_SETLIVE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, version);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CServerPatchTerminalSkel::TMessageHandlerMap &CServerPatchTerminalSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SPT_STATE"), &CServerPatchTerminalSkel::declareState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_MODULEDOWN"), &CServerPatchTerminalSkel::declareModuleDown_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_VERSION_NAME"), &CServerPatchTerminalSkel::declareVersionName_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_DOMAIN_INFO"), &CServerPatchTerminalSkel::declareDomainInfo_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_VERSION_ACK"), &CServerPatchTerminalSkel::ackVersionChange_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_SETNEXT"), &CServerPatchTerminalSkel::setInstallVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_SETLIVE"), &CServerPatchTerminalSkel::setLaunchVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_EXEC_ACK"), &CServerPatchTerminalSkel::executedCommandAck_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPT_EXEC_RESULT"), &CServerPatchTerminalSkel::executedCommandResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CServerPatchTerminalSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CServerPatchTerminalSkel::declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_declareState_SPT_STATE);
		NLMISC::CSString	moduleName;
			nlRead(__message, serial, moduleName);
		NLMISC::CSString	state;
			nlRead(__message, serial, state);
		declareState(sender, moduleName, state);
	}

	void CServerPatchTerminalSkel::declareModuleDown_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_declareModuleDown_SPT_MODULEDOWN);
		NLMISC::CSString	moduleName;
			nlRead(__message, serial, moduleName);
		declareModuleDown(sender, moduleName);
	}

	void CServerPatchTerminalSkel::declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_declareVersionName_SPT_VERSION_NAME);
		NLMISC::CSString	versionName;
			nlRead(__message, serial, versionName);
		uint32	clientVersion;
			nlRead(__message, serial, clientVersion);
		uint32	serverVersion;
			nlRead(__message, serial, serverVersion);
		declareVersionName(sender, versionName, clientVersion, serverVersion);
	}

	void CServerPatchTerminalSkel::declareDomainInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_declareDomainInfo_SPT_DOMAIN_INFO);
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		uint32	installVersion;
			nlRead(__message, serial, installVersion);
		uint32	launchVersion;
			nlRead(__message, serial, launchVersion);
		declareDomainInfo(sender, domainName, installVersion, launchVersion);
	}

	void CServerPatchTerminalSkel::ackVersionChange_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_ackVersionChange_SPT_VERSION_ACK);
		NLMISC::CSString	domainName;
			nlRead(__message, serial, domainName);
		bool	success;
			nlRead(__message, serial, success);
		NLMISC::CSString	comment;
			nlRead(__message, serial, comment);
		ackVersionChange(sender, domainName, success, comment);
	}

	void CServerPatchTerminalSkel::setInstallVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_setInstallVersion_SPT_SETNEXT);
		NLMISC::CSString	domain;
			nlRead(__message, serial, domain);
		uint32	version;
			nlRead(__message, serial, version);
		setInstallVersion(sender, domain, version);
	}

	void CServerPatchTerminalSkel::setLaunchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_setLaunchVersion_SPT_SETLIVE);
		NLMISC::CSString	domain;
			nlRead(__message, serial, domain);
		uint32	version;
			nlRead(__message, serial, version);
		setLaunchVersion(sender, domain, version);
	}

	void CServerPatchTerminalSkel::executedCommandAck_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_executedCommandAck_SPT_EXEC_ACK);
		NLMISC::CSString	result;
			nlRead(__message, serial, result);
		executedCommandAck(sender, result);
	}

	void CServerPatchTerminalSkel::executedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchTerminalSkel_executedCommandResult_SPT_EXEC_RESULT);
		NLMISC::CSString	originator;
			nlRead(__message, serial, originator);
		NLMISC::CSString	commandline;
			nlRead(__message, serial, commandline);
		NLMISC::CSString	result;
			nlRead(__message, serial, result);
		executedCommandResult(sender, originator, commandline, result);
	}
		// 
		// Message sent by SPM module to declare the state of a named module
		// This message is sent by the SPM for each connected SP / RE / RR type module on connection of SPT to SPM
		// This message is also sent by the SPM for each type the SPM receives a state update from a SP / RE / RR type module
	void CServerPatchTerminalProxy::declareState(NLNET::IModule *sender, const NLMISC::CSString &moduleName, const NLMISC::CSString &state)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), moduleName, state);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareState(__message, moduleName, state);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to declare module down for a connected SPA / SPR / SPB type module
	void CServerPatchTerminalProxy::declareModuleDown(NLNET::IModule *sender, const NLMISC::CSString &moduleName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareModuleDown(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), moduleName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareModuleDown(__message, moduleName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to define a named version
	void CServerPatchTerminalProxy::declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareVersionName(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), versionName, clientVersion, serverVersion);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareVersionName(__message, versionName, clientVersion, serverVersion);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to give info on a named domain
	void CServerPatchTerminalProxy::declareDomainInfo(NLNET::IModule *sender, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareDomainInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, installVersion, launchVersion);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareDomainInfo(__message, domainName, installVersion, launchVersion);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM module to acknowledge a version change attempt
	void CServerPatchTerminalProxy::ackVersionChange(NLNET::IModule *sender, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->ackVersionChange(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domainName, success, comment);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_ackVersionChange(__message, domainName, success, comment);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM to inform us of the current installed version for a given domain
	void CServerPatchTerminalProxy::setInstallVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setInstallVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domain, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setInstallVersion(__message, domain, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM to inform us of the current live version for a given domain
	void CServerPatchTerminalProxy::setLaunchVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setLaunchVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domain, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setLaunchVersion(__message, domain, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM with result of command issuued via executeCommandOnModules()
	void CServerPatchTerminalProxy::executedCommandAck(NLNET::IModule *sender, const NLMISC::CSString &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executedCommandAck(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executedCommandAck(__message, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPM with result of command issuued via executeCommandOnModules()
	void CServerPatchTerminalProxy::executedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executedCommandResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), originator, commandline, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executedCommandResult(__message, originator, commandline, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareState(NLNET::CMessage &__message, const NLMISC::CSString &moduleName, const NLMISC::CSString &state)
	{
		__message.setType("SPT_STATE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (moduleName));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (state));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareModuleDown(NLNET::CMessage &__message, const NLMISC::CSString &moduleName)
	{
		__message.setType("SPT_MODULEDOWN");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (moduleName));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareVersionName(NLNET::CMessage &__message, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		__message.setType("SPT_VERSION_NAME");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (versionName));
			nlWrite(__message, serial, clientVersion);
			nlWrite(__message, serial, serverVersion);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_declareDomainInfo(NLNET::CMessage &__message, const NLMISC::CSString &domainName, uint32 installVersion, uint32 launchVersion)
	{
		__message.setType("SPT_DOMAIN_INFO");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, installVersion);
			nlWrite(__message, serial, launchVersion);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_ackVersionChange(NLNET::CMessage &__message, const NLMISC::CSString &domainName, bool success, const NLMISC::CSString &comment)
	{
		__message.setType("SPT_VERSION_ACK");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domainName));
			nlWrite(__message, serial, success);
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (comment));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_setInstallVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version)
	{
		__message.setType("SPT_SETNEXT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domain));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_setLaunchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version)
	{
		__message.setType("SPT_SETLIVE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domain));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_executedCommandAck(NLNET::CMessage &__message, const NLMISC::CSString &result)
	{
		__message.setType("SPT_EXEC_ACK");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (result));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchTerminalProxy::buildMessageFor_executedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		__message.setType("SPT_EXEC_RESULT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (originator));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (commandline));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (result));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CServerPatchManagerSkel::TMessageHandlerMap &CServerPatchManagerSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("SPM_REGISTER"), &CServerPatchManagerSkel::registerAdministeredModule_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_REFRESH"), &CServerPatchManagerSkel::requestRefresh_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_SETNEXT"), &CServerPatchManagerSkel::setInstallVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_SETLIVE"), &CServerPatchManagerSkel::setLaunchVersion_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_STATE"), &CServerPatchManagerSkel::declareState_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_VERSION_NAME"), &CServerPatchManagerSkel::declareVersionName_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_EXEC"), &CServerPatchManagerSkel::executeCommandOnModules_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("SPM_EXEC_RESULT"), &CServerPatchManagerSkel::executedCommandResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CServerPatchManagerSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CServerPatchManagerSkel::registerAdministeredModule_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_registerAdministeredModule_SPM_REGISTER);
		bool	requireApplierUpdates;
			nlRead(__message, serial, requireApplierUpdates);
		bool	requireTerminalUpdates;
			nlRead(__message, serial, requireTerminalUpdates);
		bool	requireDepCfgUpdates;
			nlRead(__message, serial, requireDepCfgUpdates);
		bool	isAdministered;
			nlRead(__message, serial, isAdministered);
		registerAdministeredModule(sender, requireApplierUpdates, requireTerminalUpdates, requireDepCfgUpdates, isAdministered);
	}

	void CServerPatchManagerSkel::requestRefresh_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_requestRefresh_SPM_REFRESH);
		requestRefresh(sender);
	}

	void CServerPatchManagerSkel::setInstallVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_setInstallVersion_SPM_SETNEXT);
		NLMISC::CSString	domain;
			nlRead(__message, serial, domain);
		uint32	version;
			nlRead(__message, serial, version);
		setInstallVersion(sender, domain, version);
	}

	void CServerPatchManagerSkel::setLaunchVersion_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_setLaunchVersion_SPM_SETLIVE);
		NLMISC::CSString	domain;
			nlRead(__message, serial, domain);
		uint32	version;
			nlRead(__message, serial, version);
		setLaunchVersion(sender, domain, version);
	}

	void CServerPatchManagerSkel::declareState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_declareState_SPM_STATE);
		NLMISC::CSString	state;
			nlRead(__message, serial, state);
		declareState(sender, state);
	}

	void CServerPatchManagerSkel::declareVersionName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_declareVersionName_SPM_VERSION_NAME);
		NLMISC::CSString	versionName;
			nlRead(__message, serial, versionName);
		uint32	clientVersion;
			nlRead(__message, serial, clientVersion);
		uint32	serverVersion;
			nlRead(__message, serial, serverVersion);
		declareVersionName(sender, versionName, clientVersion, serverVersion);
	}

	void CServerPatchManagerSkel::executeCommandOnModules_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_executeCommandOnModules_SPM_EXEC);
		NLMISC::CSString	target;
			nlRead(__message, serial, target);
		NLMISC::CSString	commandline;
			nlRead(__message, serial, commandline);
		executeCommandOnModules(sender, target, commandline);
	}

	void CServerPatchManagerSkel::executedCommandResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CServerPatchManagerSkel_executedCommandResult_SPM_EXEC_RESULT);
		NLMISC::CSString	originator;
			nlRead(__message, serial, originator);
		NLMISC::CSString	commandline;
			nlRead(__message, serial, commandline);
		NLMISC::CSString	result;
			nlRead(__message, serial, result);
		executedCommandResult(sender, originator, commandline, result);
	}
		// 
		// Message sent by an administered module to register
	void CServerPatchManagerProxy::registerAdministeredModule(NLNET::IModule *sender, bool requireApplierUpdates, bool requireTerminalUpdates, bool requireDepCfgUpdates, bool isAdministered)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerAdministeredModule(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), requireApplierUpdates, requireTerminalUpdates, requireDepCfgUpdates, isAdministered);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_registerAdministeredModule(__message, requireApplierUpdates, requireTerminalUpdates, requireDepCfgUpdates, isAdministered);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to request a refresh of state info etc
	void CServerPatchManagerProxy::requestRefresh(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->requestRefresh(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_requestRefresh(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to request a change of install version for a given domain
		// This message is forwarded to all SPA modules of the given domain
	void CServerPatchManagerProxy::setInstallVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setInstallVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domain, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setInstallVersion(__message, domain, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to request a change of launch version for a given domain
		// This message is forwarded to all SPA modules of the given domain
	void CServerPatchManagerProxy::setLaunchVersion(NLNET::IModule *sender, const NLMISC::CSString &domain, uint32 version)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setLaunchVersion(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), domain, version);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_setLaunchVersion(__message, domain, version);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPR / SPB / SPA type modules to declare their states
		// This message is forwarded to all connected SPT modules
	void CServerPatchManagerProxy::declareState(NLNET::IModule *sender, const NLMISC::CSString &state)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareState(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), state);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareState(__message, state);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to define a new named version
	void CServerPatchManagerProxy::declareVersionName(NLNET::IModule *sender, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->declareVersionName(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), versionName, clientVersion, serverVersion);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_declareVersionName(__message, versionName, clientVersion, serverVersion);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message sent by SPT module to request execution of a command on one or more modules
		// Note that the 'target' parameter may be a wildcard
	void CServerPatchManagerProxy::executeCommandOnModules(NLNET::IModule *sender, const NLMISC::CSString &target, const NLMISC::CSString &commandline)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executeCommandOnModules(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), target, commandline);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executeCommandOnModules(__message, target, commandline);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// Message with result of command issuued via executeCommandOnSPA()
	void CServerPatchManagerProxy::executedCommandResult(NLNET::IModule *sender, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->executedCommandResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), originator, commandline, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_executedCommandResult(__message, originator, commandline, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_registerAdministeredModule(NLNET::CMessage &__message, bool requireApplierUpdates, bool requireTerminalUpdates, bool requireDepCfgUpdates, bool isAdministered)
	{
		__message.setType("SPM_REGISTER");
			nlWrite(__message, serial, requireApplierUpdates);
			nlWrite(__message, serial, requireTerminalUpdates);
			nlWrite(__message, serial, requireDepCfgUpdates);
			nlWrite(__message, serial, isAdministered);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_requestRefresh(NLNET::CMessage &__message)
	{
		__message.setType("SPM_REFRESH");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_setInstallVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version)
	{
		__message.setType("SPM_SETNEXT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domain));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_setLaunchVersion(NLNET::CMessage &__message, const NLMISC::CSString &domain, uint32 version)
	{
		__message.setType("SPM_SETLIVE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (domain));
			nlWrite(__message, serial, version);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_declareState(NLNET::CMessage &__message, const NLMISC::CSString &state)
	{
		__message.setType("SPM_STATE");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (state));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_declareVersionName(NLNET::CMessage &__message, const NLMISC::CSString &versionName, uint32 clientVersion, uint32 serverVersion)
	{
		__message.setType("SPM_VERSION_NAME");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (versionName));
			nlWrite(__message, serial, clientVersion);
			nlWrite(__message, serial, serverVersion);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_executeCommandOnModules(NLNET::CMessage &__message, const NLMISC::CSString &target, const NLMISC::CSString &commandline)
	{
		__message.setType("SPM_EXEC");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (target));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (commandline));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CServerPatchManagerProxy::buildMessageFor_executedCommandResult(NLNET::CMessage &__message, const NLMISC::CSString &originator, const NLMISC::CSString &commandline, const NLMISC::CSString &result)
	{
		__message.setType("SPM_EXEC_RESULT");
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (originator));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (commandline));
			nlWrite(__message, serial, const_cast < NLMISC::CSString& > (result));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	

	const CDeploymentConfigurationSynchroniserSkel::TMessageHandlerMap &CDeploymentConfigurationSynchroniserSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;
			
			res = handlers.insert(std::make_pair(std::string("DEPCFG_REQUEST"), &CDeploymentConfigurationSynchroniserSkel::requestSync_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			res = handlers.insert(std::make_pair(std::string("DEPCFG_SYNC"), &CDeploymentConfigurationSynchroniserSkel::sync_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			
			init = true;
		}

		return handlers;			
	}
	bool CDeploymentConfigurationSynchroniserSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	
	void CDeploymentConfigurationSynchroniserSkel::requestSync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CDeploymentConfigurationSynchroniserSkel_requestSync_DEPCFG_REQUEST);
		requestSync(sender);
	}

	void CDeploymentConfigurationSynchroniserSkel::sync_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CDeploymentConfigurationSynchroniserSkel_sync_DEPCFG_SYNC);
		NLNET::TBinBuffer	dataBlob;
			nlRead(__message, serial, dataBlob);
		sync(sender, dataBlob);
	}
		// 
		// Request for a copy of another module's CDeploymentConfiguration singleton
	void CDeploymentConfigurationSynchroniserProxy::requestSync(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->requestSync(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_requestSync(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// 
		// A copy of the data from the CDeploymentConfiguration singleton
	void CDeploymentConfigurationSynchroniserProxy::sync(NLNET::IModule *sender, const NLNET::TBinBuffer &dataBlob)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->sync(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), dataBlob);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing 
			NLNET::CMessage __message;
			
			buildMessageFor_sync(__message, dataBlob);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CDeploymentConfigurationSynchroniserProxy::buildMessageFor_requestSync(NLNET::CMessage &__message)
	{
		__message.setType("DEPCFG_REQUEST");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CDeploymentConfigurationSynchroniserProxy::buildMessageFor_sync(NLNET::CMessage &__message, const NLNET::TBinBuffer &dataBlob)
	{
		__message.setType("DEPCFG_SYNC");
			nlWrite(__message, serial, const_cast < NLNET::TBinBuffer& > (dataBlob));


		return __message;
	}

}
