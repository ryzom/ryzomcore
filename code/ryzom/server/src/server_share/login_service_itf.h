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

#ifndef LOGIN_SERVICE_ITF
#define LOGIN_SERVICE_ITF
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

#include "nel/net/login_cookie.h"
	
namespace LS
{
	

	class CLoginServiceWebItf 
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;	

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"LG",	CLoginServiceWebItf::cb_login	},
				{	"LO",	CLoginServiceWebItf::cb_logout	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CLoginServiceWeb__cbConnection);
			CLoginServiceWebItf *_this = reinterpret_cast<CLoginServiceWebItf *>(arg);

			_this->on_CLoginServiceWeb_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CLoginServiceWeb__cbDisconnection);
			CLoginServiceWebItf *_this = reinterpret_cast<CLoginServiceWebItf *>(arg);

			_this->on_CLoginServiceWeb_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the 
		 *	interface).
		 */
		CLoginServiceWebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
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

		virtual ~CLoginServiceWebItf()
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
			H_AUTO(CLoginServiceWeb_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CLoginServiceWeb : Exception launch in callback server update");
			}
		}

		// Return the cookie generated for this user session
		// Eventualy, return an empty string as cookie in case of error
		//   resultCode : 0 - ok, login success
		//       1 - invalid user
		//       2 - user already online, must relog
		//  errorString contain a stringified description in case of error

		void loginResult(NLNET::TSockId dest, uint32 userId, const std::string &cookie, uint32 resultCode, const std::string &errorString)
		{
			H_AUTO(loginResult_loginResult);
#ifdef NL_DEBUG
			nldebug("CLoginServiceWeb::loginResult called");
#endif
			NLNET::CMessage message("LGR");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, const_cast < std::string& > (cookie));
			nlWrite(message, serial, resultCode);
			nlWrite(message, serial, const_cast < std::string& > (errorString));

			_CallbackServer->send(message, dest);
		}
		// Return an error code for the logout attemp
		// If return is not 0, then reason contains a debug string
		// Return values : 0 - ok
		//                 1 - invalid user
		//                 2 - user already offline

		void logoutResult(NLNET::TSockId dest, uint32 errorCode, const std::string &reason)
		{
			H_AUTO(logoutResult_logoutResult);
#ifdef NL_DEBUG
			nldebug("CLoginServiceWeb::logoutResult called");
#endif
			NLNET::CMessage message("LGOR");
			nlWrite(message, serial, errorCode);
			nlWrite(message, serial, const_cast < std::string& > (reason));

			_CallbackServer->send(message, dest);
		}

		static void cb_login (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(login_on_login);
#ifdef NL_DEBUG
			nldebug("CLoginServiceWeb::cb_login received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CLoginServiceWebItf *callback = (CLoginServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			std::string	ipAddress;
			uint32	domainId;
			nlRead(message, serial, userId);
			nlRead(message, serial, ipAddress);
			nlRead(message, serial, domainId);


#ifdef NL_DEBUG
			nldebug("CLoginServiceWeb::cb_login : calling on_login");
#endif


			callback->on_login(from, userId, ipAddress, domainId);

		}

		static void cb_logout (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(logout_on_logout);
#ifdef NL_DEBUG
			nldebug("CLoginServiceWeb::cb_logout received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());
			
			CLoginServiceWebItf *callback = (CLoginServiceWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			nlRead(message, serial, userId);


#ifdef NL_DEBUG
			nldebug("CLoginServiceWeb::cb_logout : calling on_logout");
#endif


			callback->on_logout(from, userId);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CLoginServiceWeb_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CLoginServiceWeb_Disconnection(NLNET::TSockId from) =0;


		// The web side as authentified an user
		// and inform the LS that the user is now online
		// and authentified for the indicated ring domain
		// LS then generate a cookie that will be
		// used to authenticate the user when it will
		// conect to the front end later
		virtual void on_login(NLNET::TSockId from, uint32 userId, const std::string &ipAddress, uint32 domainId) =0;

		// The web side says that the user is no more actif (has logged out)
		// If the user is not in game (status cs_online), then 
		// it's status is set to cs_offline and the cookie cleared
		// Otherwise, the status is unchanged and the cookie is still valid.
		virtual void on_logout(NLNET::TSockId from, uint32 userId) =0;

	};
	
	
	/** This is the client side of the interface 
	 *	Derive from this class to invoke method on the callback server
	 */	

	class CLoginServiceWebClientItf 
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"LGR",	CLoginServiceWebClientItf::cb_loginResult	},
				{	"LGOR",	CLoginServiceWebClientItf::cb_logoutResult	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CLoginServiceWebClientItf *_this = reinterpret_cast<CLoginServiceWebClientItf *>(arg);

			_this->on_CLoginServiceWebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_loginResult"), std::string("LGR")));
			messageNames.insert(std::make_pair(std::string("on_logoutResult"), std::string("LGOR")));

				initialized = true;
			}
			
			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;
			

			static std::string emptyString;
			
			return emptyString;

		}
		
		CLoginServiceWebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
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
			H_AUTO(CLoginServiceWeb_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CLoginServiceWeb : Exception launch in callback client update");
			}
		}

		// The web side as authentified an user
		// and inform the LS that the user is now online
		// and authentified for the indicated ring domain
		// LS then generate a cookie that will be
		// used to authenticate the user when it will
		// conect to the front end later

		void login(uint32 userId, const std::string &ipAddress, uint32 domainId)
		{
#ifdef NL_DEBUG
			nldebug("CLoginServiceWebClient::login called");
#endif
			NLNET::CMessage message("LG");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, const_cast < std::string& > (ipAddress));
			nlWrite(message, serial, domainId);

			_CallbackClient->send(message);
		}
		// The web side says that the user is no more actif (has logged out)
		// If the user is not in game (status cs_online), then 
		// it's status is set to cs_offline and the cookie cleared
		// Otherwise, the status is unchanged and the cookie is still valid.

		void logout(uint32 userId)
		{
#ifdef NL_DEBUG
			nldebug("CLoginServiceWebClient::logout called");
#endif
			NLNET::CMessage message("LO");
			nlWrite(message, serial, userId);

			_CallbackClient->send(message);
		}

		static void cb_loginResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CLoginServiceWebClient::cb_loginResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CLoginServiceWebClientItf *callback = (CLoginServiceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			std::string	cookie;
			uint32	resultCode;
			std::string	errorString;
			nlRead(message, serial, userId);
			nlRead(message, serial, cookie);
			nlRead(message, serial, resultCode);
			nlRead(message, serial, errorString);


#ifdef NL_DEBUG
			nldebug("CLoginServiceWebClient::cb_loginResult : calling on_loginResult");
#endif

			callback->on_loginResult(from, userId, cookie, resultCode, errorString);
		}

		static void cb_logoutResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CLoginServiceWebClient::cb_logoutResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());
			
			CLoginServiceWebClientItf *callback = (CLoginServiceWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	errorCode;
			std::string	reason;
			nlRead(message, serial, errorCode);
			nlRead(message, serial, reason);


#ifdef NL_DEBUG
			nldebug("CLoginServiceWebClient::cb_logoutResult : calling on_logoutResult");
#endif

			callback->on_logoutResult(from, errorCode, reason);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CLoginServiceWebClient_Disconnection(NLNET::TSockId from) =0;


		// Return the cookie generated for this user session
		// Eventualy, return an empty string as cookie in case of error
		//   resultCode : 0 - ok, login success
		//       1 - invalid user
		//       2 - user already online, must relog
		//  errorString contain a stringified description in case of error
		virtual void on_loginResult(NLNET::TSockId from, uint32 userId, const std::string &cookie, uint32 resultCode, const std::string &errorString) =0;

		// Return an error code for the logout attemp
		// If return is not 0, then reason contains a debug string
		// Return values : 0 - ok
		//                 1 - invalid user
		//                 2 - user already offline
		virtual void on_logoutResult(NLNET::TSockId from, uint32 errorCode, const std::string &reason) =0;

	};

}
	
#endif
