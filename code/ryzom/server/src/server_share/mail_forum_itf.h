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

#ifndef MAIL_FORUM_ITF
#define MAIL_FORUM_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/callback_adaptor.h"

#include "nel/misc/entity_id.h"
	
namespace MFS
{
	

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CMailForumNotifierSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CMailForumNotifierSkel>	TInterceptor;
	protected:
		CMailForumNotifierSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CMailForumNotifierSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CMailForumNotifierSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void notifyMail_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void notifyForumMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CMailForumNotifierSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// A character have received a mail
		virtual void notifyMail(NLNET::IModuleProxy *sender, uint32 charId) =0;
		// A new message have been posted in a guild forum
		// the notifier client send a notification for each member character
		virtual void notifyForumMessage(NLNET::IModuleProxy *sender, uint32 charId, uint32 guildId, uint32 threadId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CMailForumNotifierProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CMailForumNotifierSkel	*_LocalModuleSkel;


	public:
		CMailForumNotifierProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CMailForumNotifierSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CMailForumNotifierProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// A character have received a mail
		void notifyMail(NLNET::IModule *sender, uint32 charId);
		// A new message have been posted in a guild forum
		// the notifier client send a notification for each member character
		void notifyForumMessage(NLNET::IModule *sender, uint32 charId, uint32 guildId, uint32 threadId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_notifyMail(NLNET::CMessage &__message, uint32 charId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_notifyForumMessage(NLNET::CMessage &__message, uint32 charId, uint32 guildId, uint32 threadId);
	



	};
	// Callback interface used by web server during 'outgame' operation

	class CMailForumWebItf 
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;	

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"MFS_NM",	CMailForumWebItf::cb_notifyMail	},
				{	"MFS_NFM",	CMailForumWebItf::cb_notifyForumMessage	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CMailForumWeb__cbConnection);
			CMailForumWebItf *_this = reinterpret_cast<CMailForumWebItf *>(arg);

			_this->on_CMailForumWeb_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CMailForumWeb__cbDisconnection);
			CMailForumWebItf *_this = reinterpret_cast<CMailForumWebItf *>(arg);

			_this->on_CMailForumWeb_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the 
		 *	interface).
		 */
		CMailForumWebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
		{
			if (replacementAdaptor == NULL)
			{
				// use default callback server
				_CallbackServer = std::auto_ptr<ICallbackServerAdaptor>(new CNelCallbackServerAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackServer = std::auto_ptr<ICallbackServerAdaptor>(replacementAdaptor);
			}
		}

		virtual ~CMailForumWebItf()
		{
		}

		/// Open the interface socket in the specified port
		void openItf(uint16 port)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;



			getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);

			_CallbackServer->setConnectionCallback (_cbConnection, this);
			_CallbackServer->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackServer->init(port);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch method invokation.
		 */
		void update()
		{
			H_AUTO(CMailForumWeb_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CMailForumWeb : Exception launch in callback server update");
			}
		}

		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error

		void invokeResult(NLNET::TSockId dest, uint32 resultCode, const std::string &resultString)
		{
			H_AUTO(invokeResult_invokeResult);
#ifdef NL_DEBUG
			nldebug("CMailForumWeb::invokeResult called");
#endif
			NLNET::CMessage message("MFS_RET");
			nlWrite(message, serial, resultCode);
			nlWrite(message, serial, const_cast < std::string& > (resultString));

			_CallbackServer->send(message, dest);
		}

		static void cb_notifyMail (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(notifyMail_on_notifyMail);
#ifdef NL_DEBUG
			nldebug("CMailForumWeb::cb_notifyMail received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CMailForumWebItf *callback = (CMailForumWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CMailForumWeb::cb_notifyMail : calling on_notifyMail");
#endif


			callback->on_notifyMail(from, charId);

		}

		static void cb_notifyForumMessage (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(notifyForumMessage_on_notifyForumMessage);
#ifdef NL_DEBUG
			nldebug("CMailForumWeb::cb_notifyForumMessage received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CMailForumWebItf *callback = (CMailForumWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	guildId;
			uint32	forumId;
			nlRead(message, serial, guildId);
			nlRead(message, serial, forumId);


#ifdef NL_DEBUG
			nldebug("CMailForumWeb::cb_notifyForumMessage : calling on_notifyForumMessage");
#endif


			callback->on_notifyForumMessage(from, guildId, forumId);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CMailForumWeb_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CMailForumWeb_Disconnection(NLNET::TSockId from) =0;


		// A character have received a mail
		virtual void on_notifyMail(NLNET::TSockId from, uint32 charId) =0;

		// A new message have been posted in a guild forum 
		virtual void on_notifyForumMessage(NLNET::TSockId from, uint32 guildId, uint32 forumId) =0;

	};
	
		// Callback interface used by web server during 'outgame' operation

	/** This is the client side of the interface 
	 *	Derive from this class to invoke method on the callback server
	 */	

	class CMailForumWebClientItf 
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"MFS_RET",	CMailForumWebClientItf::cb_invokeResult	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CMailForumWebClientItf *_this = reinterpret_cast<CMailForumWebClientItf *>(arg);

			_this->on_CMailForumWebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_invokeResult"), std::string("MFS_RET")));

				initialized = true;
			}
			
			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;
			

			static std::string emptyString;
			
			return emptyString;

		}
		
		CMailForumWebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
		{
			if (adaptorReplacement == NULL)
			{
				// use the default Nel adaptor
				_CallbackClient = std::auto_ptr < ICallbackClientAdaptor >(new CNelCallbackClientAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackClient = std::auto_ptr < ICallbackClientAdaptor >(adaptorReplacement);
			}
		}

		/// Connect the interface client to the callback server at the specified address and port
		virtual void connectItf(NLNET::CInetAddress address)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;

			static bool callbackAdded = false;
			if (!callbackAdded)
			{

				getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
			}

			_CallbackClient->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackClient->connect(address);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch invokation returns.
		 */
		virtual void update()
		{
			H_AUTO(CMailForumWeb_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CMailForumWeb : Exception launch in callback client update");
			}
		}

		// A character have received a mail

		void notifyMail(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CMailForumWebClient::notifyMail called");
#endif
			NLNET::CMessage message("MFS_NM");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// A new message have been posted in a guild forum 

		void notifyForumMessage(uint32 guildId, uint32 forumId)
		{
#ifdef NL_DEBUG
			nldebug("CMailForumWebClient::notifyForumMessage called");
#endif
			NLNET::CMessage message("MFS_NFM");
			nlWrite(message, serial, guildId);
			nlWrite(message, serial, forumId);

			_CallbackClient->send(message);
		}

		static void cb_invokeResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CMailForumWebClient::cb_invokeResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CMailForumWebClientItf *callback = (CMailForumWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	resultCode;
			std::string	resultString;
			nlRead(message, serial, resultCode);
			nlRead(message, serial, resultString);


#ifdef NL_DEBUG
			nldebug("CMailForumWebClient::cb_invokeResult : calling on_invokeResult");
#endif

			callback->on_invokeResult(from, resultCode, resultString);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CMailForumWebClient_Disconnection(NLNET::TSockId from) =0;


		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error
		virtual void on_invokeResult(NLNET::TSockId from, uint32 resultCode, const std::string &resultString) =0;

	};

}
	
#endif
