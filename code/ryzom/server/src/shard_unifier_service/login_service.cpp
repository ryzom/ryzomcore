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


#include "stdpch.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/login_cookie.h"

#include "game_share/welcome_service_itf.h"

#include "game_share/utils.h"
#include "server_share/mysql_wrapper.h"

#include "server_share/login_service_itf.h"
#include "database_mapping.h"
#include "nel_database_mapping.h"
#include "entity_locator.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace MSW;
using namespace RSMGR;
using namespace ENTITYLOC;

namespace LS
{

	class CLoginService : 
		public CEmptyModuleCommBehav<CEmptyModuleServiceBehav<CEmptySocketBehav<CModuleBase> > >,
		public WS::CLoginServiceSkel,
//		public LS::CLoginServiceSkel,
		public CLoginServiceWebItf,
		public ICharacterEventCb
	{
		
		/// Mysql ring database connection
		MSW::CConnection _RingDb;
		/// Mysql nel database connection
		MSW::CConnection _NelDb;

		typedef std::set<NLNET::TModuleProxyPtr>	TLSCLients;
		/// Login service client (mostly frontend)
		TLSCLients	_LSClients;

		typedef uint32	TUserId;
		typedef	map<TUserId, time_t>	TLoggedUsers;
		/// list of user that are only logged (not online) and checked for timeout
		/// If the timeout occur, the users are put back to offline (and their cookie
		///	erased)
		TLoggedUsers	_LoggedUsers;

		enum 
		{
			/// Default logged user timout (in second)
			DEFAULT_LOGGED_USER_TIMEOUT = 15*60,		// 15 mn (as in FE timeout for awaited users)
		};

		uint32			_LoggedUserTimeout;

	public:

		CLoginService()
		{
			_LoggedUserTimeout = DEFAULT_LOGGED_USER_TIMEOUT;
			CLoginServiceSkel::init(this);
		}

		static const std::string &getInitStringHelp()
		{
			static std::string help(CModuleBase::getInitStringHelp()+"db(host=<hostname> [port=<port>] user=<user> password=<password> base=<baseName>) web(port=<listenPort>) ");
			return help;
		}

		bool initModule(const TParsedCommandLine &pcl)
		{
			// recall base class
			if (!CModuleBase::initModule(pcl))
				return false;

			const TParsedCommandLine *initRingDb = pcl.getParam("ring_db");
			if (initRingDb == NULL)
			{
				nlwarning("LS : missing ring database connection information");
				return false;
			}
			const TParsedCommandLine *initNelDb = pcl.getParam("nel_db");
			if (initNelDb == NULL)
			{
				nlwarning("LS : missing nel database connection information");
				return false;
			}

			// connect to the databases
			if (!_RingDb.connect(*initRingDb))
			{
				nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
				return false;
			}
			if (!_NelDb.connect(*initNelDb))
			{
				nlwarning("Failed to connect to database using %s", initNelDb->toString().c_str());
				return false;
			}

			const TParsedCommandLine *initWeb = pcl.getParam("web");
			if (initWeb == NULL)
			{
				nlwarning("LS : missing web connection information");
				return false;
			}

			const TParsedCommandLine *webPort = initWeb->getParam("port");
			if (webPort == NULL)
			{
				nlwarning("LS : missing web.port connection information");
				return false;
			}

			// open the web interface
			uint16 port = 0;
			NLMISC::fromString(webPort->ParamValue, port);
			openItf(port);

			return true;
		}

		void onModuleUpdate()
		{
			H_AUTO(CLoginService_onModuleUpdate);

			// check that we are registered in the entity locator
			if (getSpeaker() == NULL && IEntityLocator::getInstance() != NULL)
				registerListener(IEntityLocator::getInstance());

			try
			{
				CLoginServiceWebItf::update();
			}
			catch (...)
			{
				nlwarning( "Recovered from exception in CLoginServiceWebItf::update()" );
			}

			// check for logged user to put back to offline
			uint32	now = NLMISC::CTime::getSecondsSince1970();

			TLoggedUsers::iterator first(_LoggedUsers.begin()), last(_LoggedUsers.end());
			for (; first != last; ++first)
			{
				if (first->second+_LoggedUserTimeout < now)
				{
					uint32 userId = first->first;
					nldebug("LS : update : user %u is inactive since %u second, setting it to offline", userId, _LoggedUserTimeout);
					// set this user offline !
					CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
					BOMB_IF(ru == NULL, "LS : updateModule : failed to load ring user "<<userId<<" that need to be set offline.", _LoggedUsers.erase(first); break);
				
					// set the status
					ru->setCurrentStatus(TCurrentStatus::cs_offline);
					// clear the cookie
					ru->setCookie("");

					// update the database
					ru->update(_RingDb);

					// clear the logged user entry
					_LoggedUsers.erase(first);

					// stop the check for this update
					break;
				}
			}
		}

//		bool onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
//		{
//			if (CLoginServiceSkel::onDispatchMessage(senderModuleProxy, message))
//				return true;
//
//			// manual dispatching
//
//			return false;
//		}

		virtual void				onModuleUp(IModuleProxy *proxy)
		{
			if (proxy->getModuleClassName() == "WelcomeService")
			{
				nlinfo("LS : adding WS '%s'", proxy->getModuleName().c_str());
				// this is one of our clients, store it
				_LSClients.insert(proxy);
			}
		}
		virtual void				onModuleDown(IModuleProxy *proxy)
		{
			TLSCLients::iterator it(_LSClients.find(proxy));
			if (it != _LSClients.end())
			{
				nlinfo("LS : removing WS '%s'", proxy->getModuleName().c_str());
				// we just lost a client
				_LSClients.erase(it);
			}
		}

		//////////////////////////////////////////////////
		///// login service from WS module interface callbacks
		//////////////////////////////////////////////////
		virtual void pendingUserLost(NLNET::IModuleProxy *sender, const NLNET::CLoginCookie &cookie)
		{
			nldebug("LS:pendingUserLost : WS '%s' report that user %u with cookie %s did not connect in the allowed time",
				sender->getModuleName().c_str(),
				cookie.getUserId(),
				cookie.toString().c_str());

			uint32 userId = cookie.getUserId();

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			BOMB_IF(ru == NULL, "LS:pendingUserLost : failed to load user "<<userId<<" from the database", return);

			// check user, it should be 'logged', and set it to offline.
			BOMB_IF(ru->getCurrentStatus() != TCurrentStatus::cs_logged, "LS:pendingUserLost : the user "<<userId<<" should be logged, but he is "<<ru->getCurrentStatus().toString(), return);

			if (ru->getCookie() != cookie.setToString())
			{
				// ignore this message because the user have relogged and obtained another cookie
				nldebug("LS:pendingUserLost : message ignored because user has obtained another cookie");
				return;
			}

			ru->setCurrentStatus(TCurrentStatus::cs_offline);
			ru->setCookie("");

			ru->update(_RingDb);

			// remove it from the list of logged user
			_LoggedUsers.erase(userId);
		}

		//////////////////////////////////////////////////
		///// entity locator callbacks
		//////////////////////////////////////////////////

		virtual void onUserConnection(NLNET::IModuleProxy *locatorHost, uint32 userId)
		{
			nldebug("LS: entity locator report user %u connection", userId);
			// set the user in 'online state' and remove it from the 'user to check for time out' list
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			BOMB_IF(ru == NULL, "LS : onUserConnection : failed to load ring user "<<userId, return);

			ru->setCurrentStatus(TCurrentStatus::cs_online);
			ru->update(_RingDb);

			// remove it of the logged checked user list
			_LoggedUsers.erase(userId);
		}

		virtual void onUserDisconnection(NLNET::IModuleProxy *locatorHost, uint32 userId)
		{
			nldebug("LS: entity locator report user %u disconnection", userId);
			// set the user in ':logged state' and put it in the 'user to check for time out' list
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			BOMB_IF(ru == NULL, "LS : onUserConnection : failed to load ring user "<<userId, return);

			ru->setCurrentStatus(TCurrentStatus::cs_logged);
			ru->update(_RingDb);

			// erase existing record if any
			_LoggedUsers.erase(userId);

			// insert the user in the logged user list
			_LoggedUsers.insert(make_pair(userId, NLMISC::CTime::getSecondsSince1970()));
		}

		virtual void onCharacterConnection(NLNET::IModuleProxy *locatorHost, uint32 charId, uint32 lastDisconnectionDate)
		{	/* nothing	*/	}
		virtual void onCharacterDisconnection(NLNET::IModuleProxy *locatorHost, uint32 charId)
		{	/* nothing */	}


		//////////////////////////////////////////////////
		///// Web interface callbacks
		//////////////////////////////////////////////////

		/// Connection callback : a new interface client connect
		virtual void on_CLoginServiceWeb_Connection(NLNET::TSockId from)
		{
		}

		virtual void on_CLoginServiceWeb_Disconnection(NLNET::TSockId from)
		{
			// nothing to do right now
		}
		
		/// A user has passed the auth, web sets the user online and asks for a cookie
		virtual void on_login(NLNET::TSockId from, uint32 userId, const std::string &ipAddress, uint32 domainId)
		{
			nldebug("CLoginService : receive login request from %s with user %u, address %s",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				userId,
				ipAddress.c_str());
			
			//1 check is user already online, if so, disconnect it
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				// invalid user !
				nldebug("on_login : invalid ring user %u", userId);
				loginResult(from, userId, "", 1, "Invalid user");
				return;
			}

			if (ru->getCurrentStatus() == TCurrentStatus::cs_online)
			{
				// this user seams online, we need to disconnect it from the shard

				// send a disconnect message to all LS client
				TLSCLients::iterator first(_LSClients.begin()), last(_LSClients.end());
				for (; first != last; ++first)
				{
					WS::CWelcomeServiceProxy ws(*first);

					ws.disconnectUser(this, userId);
				}

				// check in the entity locator that the player is really online
				if (IEntityLocator::getInstance() && IEntityLocator::getInstance()->isUserOnline(userId))
				{
					nldebug("LS : on_login : user %u already connected, disconnecting him and reject login", userId);
					loginResult(from, userId, "", 2, "User already online, please relog");
					return;
				}
				
				// in fact, perhaps not really online, allow him to connect.
				// a worst, the user is online but we have asked to the WS to 
				// disconnect him.

				// In fact, most of the time, this case is when the SU is stopped 
				// with online user, so the database is not sync with the
				// real user status.
			}

			// Now prevent from logging-in at the same time with a free GM's player account and a GM CS account
			CNelUserPtr nelUser = CNelUser::load(_NelDb, userId, __FILE__, __LINE__);
			BOMB_IF(nelUser == NULL, "on_login : invalid nel user %u" << userId, loginResult(from, userId, "", 5, "Invalid user"); return);
			if (nelUser->getGMId() != 0)
			{
				uint32 otherUserId = nelUser->getGMId();
				CRingUserPtr otherRu = CRingUser::load(_RingDb, otherUserId, __FILE__, __LINE__);
				if (otherRu == NULL)
				{
					nlwarning("on_login : Can't find ring user %u from account %u with GMId", otherUserId, userId);
				}
				//else if (otherRu->getCurrentStatus() != TCurrentStatus::cs_offline) // cs_logged and cs_online
				else if (otherRu->getCurrentStatus() == TCurrentStatus::cs_online)	// less strict check, only avoid csr/player account logged on the same time
				{
					// DON'T check in the entity locator that the player is really online
					//if (IEntityLocator::getInstance() && IEntityLocator::getInstance()->isUserOnline(otherUserId))
					if (IEntityLocator::getInstance() && IEntityLocator::getInstance()->isUserOnline(otherUserId))
					{
						nldebug("LS : on_login : user %u already connected, rejecting login of %u with GMId", otherUserId, userId);
						loginResult(from, userId, "", 3, toString("User %u (%u's GMId) already online", otherUserId, userId));
						return;
					}
				}
			}
			CSString query;
			query << "SELECT UId FROM user WHERE GMId = "<<userId<<"";
			BOMB_IF(!_NelDb.query(query), "on_login : Failed to request in database", loginResult(from, userId, "", 6, "Failed request"); return);
			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_NelDb.storeResult());
			for (uint32 i=0; i!=result->getNumRows(); ++i)
			{
				result->fetchRow();
				uint32 otherUserId;
				result->getField(0, otherUserId);
				
				CRingUserPtr otherRu = CRingUser::load(_RingDb, otherUserId, __FILE__, __LINE__);
				if (otherRu == NULL)
				{
					nlwarning("on_login : Can't find ring user %u which GMID is account %u", otherUserId, userId);
				}
				//else if (otherRu->getCurrentStatus() != TCurrentStatus::cs_offline) // cs_logged and cs_online
				else if (otherRu->getCurrentStatus() == TCurrentStatus::cs_online)	// less strict check, only avoid csr/player account logged on the same time
				{
					// DON'T check in the entity locator that the player is really online
					//if (IEntityLocator::getInstance() && IEntityLocator::getInstance()->isUserOnline(otherUserId))
					if (IEntityLocator::getInstance() && IEntityLocator::getInstance()->isUserOnline(otherUserId))
					{
						nldebug("LS : on_login : user %u already connected, rejecting login of %u which is the GMId of it", otherUserId, userId);
						loginResult(from, userId, "", 4, toString("GM user %u (having GMId=%u) already online", otherUserId, userId));
						return;
					}
				}
			}


			NLNET::CInetAddress addr(ipAddress);
			//2 generate a cookie and set the player status and cookie in database
			NLNET::CLoginCookie cookie(addr.internalIPAddress(), userId);
			ru->setCookie(cookie.setToString());
			ru->setCurrentStatus(TCurrentStatus::cs_logged);
			ru->setCurrentActivity(TCurrentActivity::ca_none);
			ru->setCurrentDomainId(domainId);

			// save the user record
			ru->update(_RingDb);

			// erase existing record if any
			_LoggedUsers.erase(userId);
			// insert the user in the user to wait table
			_LoggedUsers.insert(make_pair(userId, NLMISC::CTime::getSecondsSince1970()));


			//3 call the return method to the web
			loginResult(from, userId, ru->getCookie(), 0, "");
		}

		virtual void on_logout(NLNET::TSockId from, uint32 userId)
		{
			nldebug("CLoginService : receive logout request from %s with user %u",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				userId);

			// load the ring user
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				// invalid user !
				nldebug("on_logout : invalid user %u", userId);
				logoutResult(from, 1, "unkown user");
				return;
			}

			if (ru->getCurrentStatus() == TCurrentStatus::cs_offline)
			{
				// the user is offline, could not logout 
				logoutResult(from, 2, "user already offline");
				return;
			}
			if (ru->getCurrentStatus() == TCurrentStatus::cs_online)
			{
				// the user is online (in game), ignore the disconnect but return ok
				logoutResult(from, 0, "");
				return;
			}

			// ok, the user is logged, we can put it offline
			ru->setCurrentStatus( TCurrentStatus::cs_offline);
			ru->setCookie("");
			ru->setCurrentDomainId(-1);
			ru->update(_RingDb);

			logoutResult(from, 0, "");
		}



		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CLoginService, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CLoginService, openWebInterface, "Open the web interface", "<listenPort>");
//			NLMISC_COMMAND_HANDLER_ADD(CLoginService, closeWebInterface, "Close the web interface", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CLoginService, LoggedUserTimeout, "get or set the logged user timeout in second", "[<newValue in second>]");
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(LoggedUserTimeout)
		{
			if (args.size() > 1)
				return false;

			if (!args.empty())
			{
				NLMISC::fromString(args[0], _LoggedUserTimeout);
			}

			// output the value
			log.displayNL("LoggedUserTimeout = %u", _LoggedUserTimeout);

			return true;
		}

		
		//		NLMISC_CLASS_COMMAND_DECL(closeWebInterface)
//		{
//			if (args.size() != 0)
//				return false;
//
//			
//		}

		NLMISC_CLASS_COMMAND_DECL(openWebInterface)
		{
			if (args.size() != 1)
				return false;

			uint16 port;
			NLMISC::fromString(args[0], port);
			log.displayNL("Opening web interface on port %u", port);
			openItf(port);

			return true;
		}
	};

	NLNET_REGISTER_MODULE_FACTORY(CLoginService, "LoginService");

} // namespace LS

