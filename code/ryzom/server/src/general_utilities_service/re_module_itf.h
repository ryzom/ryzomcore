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

#ifndef RE_MODULE_ITF_H
#define RE_MODULE_ITF_H
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
#include "nel/misc/string_conversion.h"

#include "nel/misc/entity_id.h"
	
#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
namespace GUS_SCM
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRepositoryEmitterSkel
	{
	protected:
		CRepositoryEmitterSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CRepositoryEmitterSkel()
		{
		}



	private:
		typedef void (CRepositoryEmitterSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;


		const TMessageHandlerMap &getMessageHandlers() const
		{
			static TMessageHandlerMap handlers;
			static bool init = false;

			if (!init)
			{
				std::pair < TMessageHandlerMap::iterator, bool > res;
				
				res = handlers.insert(std::make_pair(std::string("RE_REQUEST"), &CRepositoryEmitterSkel::requestFile_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("RE_DATA_ACK"), &CRepositoryEmitterSkel::fileDataAck_skel));
				// if this assert, you have a doubly message name in your interface definition !
				nlassert(res.second);
				
				res = handlers.insert(std::make_pair(std::string("RE_ERR_DUPMOD"), &CRepositoryEmitterSkel::duplicateModuleError_skel));
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
		
		void requestFile_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			requestFile(sender, fileName);
		}

		void fileDataAck_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			std::string	fileName;
			nlRead(__message, serial, fileName);
			bool	status;
			nlRead(__message, serial, status);
			fileDataAck(sender, fileName, status);
		}

		void duplicateModuleError_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
		{
			duplicateModuleError(sender);
		}

	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void requestFile(NLNET::IModuleProxy *sender, const std::string &fileName) =0;
		// 
		virtual void fileDataAck(NLNET::IModuleProxy *sender, const std::string &fileName, bool status) =0;
		// 
		virtual void duplicateModuleError(NLNET::IModuleProxy *sender) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRepositoryEmitterProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CRepositoryEmitterSkel	*_LocalModuleSkel;


	public:
		CRepositoryEmitterProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "RepositoryEmitter");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				_LocalModuleSkel = dynamic_cast < CRepositoryEmitterSkel* > (_LocalModule.getPtr());
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CRepositoryEmitterProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void requestFile(NLNET::IModule *sender, const std::string &fileName)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->requestFile(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_requestFile(__message, fileName);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void fileDataAck(NLNET::IModule *sender, const std::string &fileName, bool status)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->fileDataAck(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), fileName, status);
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_fileDataAck(__message, fileName, status);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}
		// 
		void duplicateModuleError(NLNET::IModule *sender)
		{
			if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
			{
				// immediate local synchronous dispatching
				_LocalModuleSkel->duplicateModuleError(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
			}
			else
			{
				// send the message for remote dispatching and execution or local queing 
				NLNET::CMessage __message;
				
				buildMessageFor_duplicateModuleError(__message);

				_ModuleProxy->sendModuleMessage(sender, __message);
			}
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_requestFile(NLNET::CMessage &__message, const std::string &fileName)
		{
			__message.setType("RE_REQUEST");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_fileDataAck(NLNET::CMessage &__message, const std::string &fileName, bool status)
		{
			__message.setType("RE_DATA_ACK");
			nlWrite(__message, serial, const_cast < std::string& > (fileName));
			nlWrite(__message, serial, status);


			return __message;
		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_duplicateModuleError(NLNET::CMessage &__message)
		{
			__message.setType("RE_ERR_DUPMOD");


			return __message;
		}




	};

}
	
#endif
