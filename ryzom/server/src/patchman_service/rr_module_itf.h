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

#error deprecated
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef RR_MODULE_ITF_H
#define RR_MODULE_ITF_H
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/misc/string_conversion.h"

#include "nel/misc/md5.h"
	
//#include "repository.h"
	
#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
namespace PATCHMAN
{
	
	class TFileRecord;

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TFileRecord
	{
	private:
		// 
		std::string	_FileName;
		// 
		NLMISC::CHashKeyMD5	_Checksum;
	public:
		// 
		const std::string &getFileName() const
		{
			return _FileName;
		}

		std::string &getFileName()
		{
			return _FileName;
		}


		void setFileName(const std::string &value)
		{
			if (_FileName != value)
			{


				_FileName = value;

				
			}
		}
			// 
		const NLMISC::CHashKeyMD5 &getChecksum() const
		{
			return _Checksum;
		}

		NLMISC::CHashKeyMD5 &getChecksum()
		{
			return _Checksum;
		}


		void setChecksum(const NLMISC::CHashKeyMD5 &value)
		{
			if (_Checksum != value)
			{


				_Checksum = value;

				
			}
		}
	
		bool operator == (const TFileRecord &other) const
		{
			return _FileName == other._FileName
				&& _Checksum == other._Checksum;
		}

		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_FileName);
			s.serial(_Checksum);

		}
		

	private:
	

	};


	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRepositoryReceiverSkel
	{
	protected:
		CRepositoryReceiverSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CRepositoryReceiverSkel()
		{
		}



	private:
		typedef void (CRepositoryReceiverSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const
		{
			static TMessageHandlerMap handlers;
			static bool init = false;

			if (!init)
			{
				std::pair < TMessageHandlerMap::iterator, bool > res;
				
				res = handlers.insert(std::make_pair(std::string("RR_LIST"), &CRepositoryReceiverSkel::fileList_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("RR_FILE_BEGIN"), &CRepositoryReceiverSkel::beginFile_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("RR_FILE_DATA"), &CRepositoryReceiverSkel::fileData_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("RR_FILE_END"), &CRepositoryReceiverSkel::fileEnd_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				init = true;
			}

			return handlers;			
		}

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	private:
		
		void fileList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			uint32	version;
			nlRead(__message, serial, version);
			TFileInfoVector	files;
			nlRead(__message, serialCont, files);
			fileList(sender, version, files);
		}

		void beginFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			uint32	fileSize;
			nlRead(__message, serial, fileSize);
			beginFile(sender, fileName, fileSize);
		}

		void fileData_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			std::string	data;
			nlRead(__message, serial, data);
			fileData(sender, fileName, data);
		}

		void fileEnd_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			fileEnd(sender, fileName);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void fileList(NLNET::IModuleProxy *sender, uint32 version, const TFileInfoVector &files) =0;
		// 
		virtual void beginFile(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 fileSize) =0;
		// 
		virtual void fileData(NLNET::IModuleProxy *sender, const std::string &fileName, const std::string &data) =0;
		// 
		virtual void fileEnd(NLNET::IModuleProxy *sender, const std::string &fileName) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRepositoryReceiverProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CRepositoryReceiverSkel	*_LocalModuleSkel;


	public:
		CRepositoryReceiverProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CRepositoryReceiverSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CRepositoryReceiverProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void fileList(NLNET::IModule *sender, uint32 version, const TFileInfoVector &files)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileList(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), version, files);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileList(__message, version, files);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void beginFile(NLNET::IModule *sender, const std::string &fileName, uint32 fileSize)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->beginFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, fileSize);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_beginFile(__message, fileName, fileSize);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void fileData(NLNET::IModule *sender, const std::string &fileName, const std::string &data)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileData(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, data);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileData(__message, fileName, data);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void fileEnd(NLNET::IModule *sender, const std::string &fileName)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileEnd(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileEnd(__message, fileName);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileList(NLNET::CMessage &__message, uint32 version, const TFileInfoVector &files)
		{
			__message.setType("RR_LIST");
			nlWrite(__message, serial, version);
			nlWrite(__message, serialCont, const_cast < TFileInfoVector& > (files));


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_beginFile(NLNET::CMessage &__message, const std::string &fileName, uint32 fileSize)
		{
			__message.setType("RR_FILE_BEGIN");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, fileSize);


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileData(NLNET::CMessage &__message, const std::string &fileName, const std::string &data)
		{
			__message.setType("RR_FILE_DATA");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, const_cast < std::string& > (data));


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileEnd(NLNET::CMessage &__message, const std::string &fileName)
		{
			__message.setType("RR_FILE_END");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));


			return __message;
		}




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFileReceiverSkel
	{
	protected:
		CFileReceiverSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CFileReceiverSkel()
		{
		}



	private:
		typedef void (CFileReceiverSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const
		{
			static TMessageHandlerMap handlers;
			static bool init = false;

			if (!init)
			{
				std::pair < TMessageHandlerMap::iterator, bool > res;
				
				res = handlers.insert(std::make_pair(std::string("FR_FILE_INFO"), &CFileReceiverSkel::fileInfo_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("FR_FILE_DATA"), &CFileReceiverSkel::fileData_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("FR_FILE_ERR"), &CFileReceiverSkel::fileDataFailure_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				init = true;
			}

			return handlers;			
		}

	protected:
		bool onDispatchMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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

	private:
		
		void fileInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			TFileInfoVector	files;
			nlRead(__message, serialCont, files);
			fileInfo(sender, files);
		}

		void fileData_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			uint32	startOffset;
			nlRead(__message, serial, startOffset);
			std::string	data;
			nlRead(__message, serial, data);
			fileData(sender, fileName, startOffset, data);
		}

		void fileDataFailure_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			fileDataFailure(sender, fileName);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void fileInfo(NLNET::IModuleProxy *sender, const TFileInfoVector &files) =0;
		// 
		virtual void fileData(NLNET::IModuleProxy *sender, const std::string &fileName, uint32 startOffset, const std::string &data) =0;
		// 
		virtual void fileDataFailure(NLNET::IModuleProxy *sender, const std::string &fileName) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFileReceiverProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CFileReceiverSkel	*_LocalModuleSkel;


	public:
		CFileReceiverProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CFileReceiverSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CFileReceiverProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void fileInfo(NLNET::IModule *sender, const TFileInfoVector &files)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileInfo(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), files);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileInfo(__message, files);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void fileData(NLNET::IModule *sender, const std::string &fileName, uint32 startOffset, const std::string &data)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileData(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, startOffset, data);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileData(__message, fileName, startOffset, data);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void fileDataFailure(NLNET::IModule *sender, const std::string &fileName)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileDataFailure(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileDataFailure(__message, fileName);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileInfo(NLNET::CMessage &__message, const TFileInfoVector &files)
		{
			__message.setType("FR_FILE_INFO");
			nlWrite(__message, serialCont, const_cast < TFileInfoVector& > (files));


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileData(NLNET::CMessage &__message, const std::string &fileName, uint32 startOffset, const std::string &data)
		{
			__message.setType("FR_FILE_DATA");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, startOffset);
			nlWrite(__message, serial, const_cast < std::string& > (data));


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileDataFailure(NLNET::CMessage &__message, const std::string &fileName)
		{
			__message.setType("FR_FILE_ERR");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));


			return __message;
		}




	};

}
	
#endif
