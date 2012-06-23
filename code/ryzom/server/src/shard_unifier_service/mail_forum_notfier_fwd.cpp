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

/** The mail forum notifier forwarder is a module that receive 
 *	new mail and new forum message notification from the web.
 *	It then use the entity locator module (using a local interface)
 *	to retreive the shard hosting the character (if they are connected)
 *	and then forward the notification to the appropriate module
 *	in each shard.
 *
 *	In case of forum message notification, the module
 *	make use of the ring database to build a list of character
 *	to notify about the new message.
 *
 *	This module is designed to be instantiated once in the 
 *	context of the shard unifier service.
 *	At least, it must be collocated with the entity locator 
 *	module.
 *
 *	As it access the ring database using NOPE and generated 
 *	mapping, it should be run in the same context as other module
 *	that access or change the database. Otherwise, it may miss some
 *	database update and work with memory cached object that are
 *	out of date.
 */

#include "stdpch.h"

#include "nel/net/module_builder_parts.h"
#include "nel/net/callback_server.h"

#include "game_share/ryzom_entity_id.h"
#include "server_share/mail_forum_itf.h"
#include "game_share/utils.h"
#include "server_share/mysql_wrapper.h"

#include "database_mapping.h"
#include "entity_locator.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace ENTITYLOC;
using namespace RSMGR;


namespace MFS
{


	class CMailForumNotifierFwd: 
		public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public CMailForumWebItf,
		public ICharacterEventCb

	{

		// mysql ring database connection
		MSW::CConnection	_RingDb;

	public:

		CMailForumNotifierFwd()
		{
		}

		static const std::string &getInitStringHelp()
		{
			static string help("ring_db(host=<mysql_hostname> user=<user> password=<password> base=<database_name>) web(port=<port>)");
			return help;
		}

		bool initModule(const TParsedCommandLine &initInfo)
		{
			nldebug("MFS_FWD : initModule with '%s'", initInfo.toString().c_str());

			if (!CModuleBase::initModule(initInfo))
				return false;

			// init ring db
			const TParsedCommandLine *initRingDb = initInfo.getParam("ring_db");
			if (initRingDb  == NULL)
			{
				nlwarning("RSM : missing ring db connection information");
				return false;
			}

			// connect to the database
			if (!_RingDb.connect(*initRingDb))
			{
				nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
				return false;
			}


			// init web interface
			const TParsedCommandLine *web = initInfo.getParam("web");
			if (web == NULL)
			{
				nlwarning("Missing 'web' param in module init string");
				return false;
			}
			
			const TParsedCommandLine *portParam = web->getParam("port");
			if (portParam == NULL)
			{
				nlwarning("Missing 'web.port' param in module init string");
				return false;
			}

			// open the web interface
			uint16 port;
			NLMISC::fromString(portParam->ParamValue, port);

			openItf(port);

			return true;
		}

		void onModuleUp(IModuleProxy *proxy)
		{
			if (proxy->getModuleClassName() == "EntityLocator")
			{
				if (proxy->getModuleDistance() == 0)
				{
					nldebug("MFS_FWD : onModuleUp : using module '%s' as entity locator", proxy->getModuleName().c_str());
					// the module is local, use it as entity locator

					IEntityLocator *el = IEntityLocator::getInstance();
					nlassert(el != NULL);

					el->registerListener(this);
				}
			}
		}

		void onModuleDown(IModuleProxy *proxy)
		{
			if (proxy->getModuleClassName() == "EntityLocator")
			{
				if (proxy->getModuleDistance() == 0)
				{
					nldebug("MFS_FWD : onModuleDown : module '%s' no more the local entity locator", proxy->getModuleName().c_str());

					IEntityLocator *el = IEntityLocator::getInstance();
					nlassert(el != NULL);

					el->unregisterListener(this);
				}
			}
		}

		bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
		{
			nlwarning("CRingSessionManager : Unknown message '%s' received", message.getName().c_str());

			return false;
		}


		void onModuleUpdate()
		{
			H_AUTO(CMailForumNotifierFwd_onModuleUpdate);

			// update the php interface
			CMailForumWebItf::update();
		}


		/////////////////////////////////////////////////////////////
		//// CMailForumWebItf interface impl
		/////////////////////////////////////////////////////////////
		virtual void onUserConnection(NLNET::IModuleProxy *locatorHost, uint32 userId)
		{	/* nothing */	}
		virtual void onUserDisconnection(NLNET::IModuleProxy *locatorHost, uint32 userId)
		{	/* nothing */	}

		void onCharacterConnection(IModuleProxy *locatorHost, uint32 charId, uint32 lastDisconnectionDate)
		{
			// a character has just connected, check his mail and forum guild
			// to see if we need to send it a notification

			// load the character data
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, ("MFS:on_notifyMail : onCharacterConnection : failed to load character "+toString(charId)), return;)	;

			if (character->getGuildId() != 0)
			{
				breakable
				{
					// there is a guild, we need to check for updated forums
					// count the number of thread with last post date after the last disconnection date
					string query = "SELECT COUNT(*) FROM mfs_guild_thread WHERE last_post_date > '"+MSW::encodeDate(lastDisconnectionDate)+"'";

					BOMB_IF(!_RingDb.query(query), ("Database error, no guild forum notification"), break;);

					auto_ptr<MSW::CStoreResult> result = _RingDb.storeResult();

					BOMB_IF(result->getNumRows() != 1, ("Database error, no guild forum notification"), break;);

					result->fetchRow();

					uint32 count;
					result->getField(0, count);

					if (count > 0)
					{
						// there are new message in some thread, send the notification
						CMailForumNotifierProxy mfn(locatorHost);
						mfn.notifyForumMessage(this, character->getObjectId(), character->getGuildId(), 0);
					}
				}
			}

			// check if there are new mail
			string query = "SELECT COUNT(*) FROM mfs_mail WHERE date > '"+MSW::encodeDate(lastDisconnectionDate)+"' AND status = 'ms_new' AND erase_series = 0";
			BOMB_IF(!_RingDb.query(query), ("Database error, no mail notification"), return;);

			auto_ptr<MSW::CStoreResult> result = _RingDb.storeResult();

			BOMB_IF(result->getNumRows() != 1, ("Database error, no mail notification"), return;);

			result->fetchRow();

			uint32 count;
			result->getField(0, count);

			if (count > 0)
			{
				// there are new mail, send the notification
				CMailForumNotifierProxy mfn(locatorHost);
				mfn.notifyMail(this, character->getObjectId());
			}
		}

		void onCharacterDisconnection(IModuleProxy *locatorHost, uint32 charId)
		{
			// nothing to do right now.
		}

		/////////////////////////////////////////////////////////////
		//// CMailForumWebItf interface impl
		/////////////////////////////////////////////////////////////

		/// Connection callback : a new interface client connect
		void on_CMailForumWeb_Connection(NLNET::TSockId from)	{}
		/// Disconnection callback : one of the interface client disconnect
		void on_CMailForumWeb_Disconnection(NLNET::TSockId from){}


		// A character have received a mail
		void on_notifyMail(NLNET::TSockId from, uint32 charId)
		{
			nldebug("MFS:on_notifyMail : mail notification for character %u", charId);
			// look for a shard hosting this character
			IEntityLocator *el = IEntityLocator::getInstance();
			if (el == NULL)
			{
				nlwarning("MFS_FW : on_notifyMail : no entity locator available, notification will not be sent");
				return;
			}

			// get the host
			IModuleProxy *proxy = el->getLocatorModuleForChar(charId);

			if (proxy == NULL)
			{
				// this character not hosted now, just return
				return;
			}

			// create a mail forum proxy
			CMailForumNotifierProxy mfn(proxy);
			
			mfn.notifyMail(this, charId);
		}

		// A new message have been posted in a guild forum 
		void on_notifyForumMessage(NLNET::TSockId from, uint32 guildId, uint32 forumId)
		{
			nldebug("MFS:on_notifyForumMessage: forum message notification for guild %u, forum %u", guildId, forumId);

			// retrieve the entity locator
			IEntityLocator *el = IEntityLocator::getInstance();
			if (el == NULL)
			{
				nlwarning("MFS:on_notifyForumMessage : no entity locator available, notification lost");
				return;
			}
			
			// load the guild and members from the database
			CGuildPtr guild = CGuild::load(_RingDb, guildId, __FILE__, __LINE__);
			DROP_IF(guild == NULL, ("MFS:on_notifyForumMessage : failed to load the guild "+toString(guildId)+" from the database, notification lost"), return;);
			DROP_IF(guild->loadCharacters(_RingDb, __FILE__, __LINE__) == false, ("MFS:on_notifyForumMessage : failed to load members of the guild "+toString(guildId)+" from the database, notification lost"), return);


			// loop over each member to build a list of host to notify
			for (uint i=0; i<guild->getCharacters().size(); ++i)
			{
				CCharacterPtr member = guild->getCharactersByIndex(i);
				if (member != NULL)
				{
					// ok, there is a member here

					// get the host
					IModuleProxy *proxy = el->getLocatorModuleForChar(member->getObjectId());

					if (proxy == NULL)
					{
						// this character not hosted now, just return
						continue;
					}

					// create a mail forum proxy
					CMailForumNotifierProxy mfn(proxy);
					// send the notification
					mfn.notifyForumMessage(this, member->getObjectId(), guildId, forumId);
				}
			}

		}

		/////////////////////////////////////////////////////////////
		//// commands
		/////////////////////////////////////////////////////////////
	
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CMailForumNotifierFwd, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CMailForumNotifierFwd, dump, "dump the module internal state", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CMailForumNotifierFwd, simMailNotify, "Simulate a mail notification", "<charId>");
		NLMISC_COMMAND_HANDLER_TABLE_END


		NLMISC_CLASS_COMMAND_DECL(simMailNotify)
		{
			if (args.size() != 1)
				return false;

			uint32 charId;
			NLMISC::fromString(args[0], charId);

			log.displayNL("Simulating new mail notification for character %u", charId);

			IEntityLocator *el = IEntityLocator::getInstance();
			if (el == NULL)
			{
				log.displayNL("ERROR : No entity locator, notification can't be sent");
				return true;
			}

			IModuleProxy *proxy = el->getLocatorModuleForChar(charId);

			if (proxy == NULL)
			{
				log.displayNL("The character seam's to be offline, notification can't be sent");
				return true;
			}

			// create a mail forum proxy
			CMailForumNotifierProxy mfn(proxy);
			// send the notification
			mfn.notifyMail(this, charId);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			// call the base class dump
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			return true;
		}
	}; 
	
	NLNET_REGISTER_MODULE_FACTORY(CMailForumNotifierFwd, "MailForumNotifierFwd");



} // namespace RSMGR

// force module linking
void forceEntityLocatorLink()
{
}
