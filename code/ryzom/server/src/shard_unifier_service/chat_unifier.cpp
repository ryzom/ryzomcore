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

#include "nel/net/module_builder_parts.h"
#include "nel/net/callback_server.h"

#include "game_share/ryzom_entity_id.h"
#include "server_share/chat_unifier_itf.h"
#include "game_share/utils.h"
#include "server_share/mysql_wrapper.h"

#include "database_mapping.h"
#include "character_sync.h"
#include "entity_locator.h"
#include "character_sync.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;
using namespace CHATUNI;
using namespace ENTITYLOC;
using namespace CHARSYNC;


namespace CHATUNI
{


	class CChatUnifierServer: 
		public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public CChatUnifierSkel
	{
		// mysql ring database connection
		MSW::CConnection	_RingDb;

		typedef uint32 TShardId;

		typedef CTwinMap<TModuleProxyPtr, TShardId>		TChatClients;

		// the chat clients
		TChatClients			_ChatClients;

		// A vector of chat clients, ready for broadcast
		vector<IModuleProxy*>	_ChatClientsForBroadcast;

	public:

		CChatUnifierServer()
		{
			CChatUnifierSkel::init(this);
		}

		static const std::string &getInitStringHelp()
		{
			static string help("ring_db(host=<mysql_hostname> user=<user> password=<password> base=<database_name>)");
			return help;
		}

		bool initModule(const TParsedCommandLine &initInfo)
		{
			nldebug("CHATUNI : initModule with '%s'", initInfo.toString().c_str());

			bool ret = CModuleBase::initModule(initInfo);

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

			return ret;
		}

		void onModuleUp(IModuleProxy *proxy)
		{
//			if (proxy->getModuleClassName() == "EntityLocator" && proxy->getModuleDistance() == 0)
//				_EntityLocator = proxy->getLocalModule();

			// look in the manifest to check for clients
			const std::string &manifest = proxy->getModuleManifest();

			// parse the manifest. We are looking for "charUnifierClient(shardId=<shardId>)
			TParsedCommandLine pcl;
			if (!pcl.parseParamList(manifest))
			{
				// failed to parse the manifest, this module certainly is not one of our client
				return;
			}
			
			const TParsedCommandLine *shardIdParam = pcl.getParam("chatUnifierClient.shardId");
			if (shardIdParam == NULL)
			{
				// not one of our client
				return;
			}

			// ok, we have a valid client, add it to the list
			uint32 shardId;
			NLMISC::fromString(shardIdParam->ParamValue, shardId);

			// check that we don't have conflict
			const TModuleProxyPtr *pproxy = _ChatClients.getA(shardId);
			if (pproxy != NULL)
			{
				nlwarning("CChatUnifierServer:onModuleUp : module '%s' claims to be client for shard %u, but already have module '%s' for it, second ignored.",
					proxy->getModuleName().c_str(),
					shardId,
					(*pproxy)->getModuleName().c_str());

				return;
			}

			nldebug("CChatUnifierServer:onModuleUp : added module '%s' as chat client for shard %u", proxy->getModuleName().c_str(), shardId);

			// ok, insert the new client
			_ChatClients.add(proxy, shardId);

			rebuildBroadcastList();
		}

		void onModuleDown(IModuleProxy *proxy)
		{
//			if (proxy->getModuleDistance() == 0 && proxy->getLocalModule() == _EntityLocator)
//				_EntityLocator = NULL;
			
			// check if it's one of our clients
			const TShardId *pshardId = _ChatClients.getB(proxy);
			if (pshardId != NULL)
			{
				nldebug("CChatUnifierServer:onModuleDown : removing module '%s' as chat client for shard %u", proxy->getModuleName().c_str(), *pshardId);

				_ChatClients.removeWithA(proxy);
				rebuildBroadcastList();
			}
		}

//		bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//		{
//			if (CChatUnifierSkel::onDispatchMessage(sender, message))
//				return true;
//
//			nlwarning("CRingSessionManager : Unknown message '%s' received", message.getName().c_str());
//
//			return false;
//		}

		void rebuildBroadcastList()
		{
			_ChatClientsForBroadcast.clear();
			_ChatClientsForBroadcast.reserve(_ChatClients.getAToBMap().size());

			TChatClients::TAToBMap::const_iterator first(_ChatClients.getAToBMap().begin()), last(_ChatClients.getAToBMap().end());
			for (; first != last; ++first)
			{
				_ChatClientsForBroadcast.push_back(first->first);
			}
		}


		/////////////////////////////////////////////////////////////
		//// CChatUnifierSkel interface impl
		/////////////////////////////////////////////////////////////

		// IOS forward a tell message to the unifier
		// If IOS can't find the player locally, it forward
		// the tell to the unifier
		void sendFarTell(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text)
		{
			nldebug("CU : sendFarTell : module '%s' send far tell '%s' from character %s to '%s'",
				sender->getModuleName().c_str(),
				text.toUtf8().c_str(),
				senderCharId.toString().c_str(),
				destName.toUtf8().c_str());

			CChatUnifierClientProxy cucSender(sender);
			// ask the entity locator to find the addressee charId from it's name
			IEntityLocator *el = IEntityLocator::getInstance();
			if (el == NULL)
			{
				// no entity locator, can't forward messages
				cucSender.recvFarTellFail(this, senderCharId, destName, TFailInfo::fi_no_entity_locator);
nldebug("sendFarTell : no entity locator");
				return;
			}
			uint32 hostShardId = el->getShardIdForChar(destName);

			if (hostShardId == 0)
			{
				// the character is not online
				cucSender.recvFarTellFail(this, senderCharId, destName, TFailInfo::fi_char_offline);
nldebug("sendFarTell : no valid host shard id for addressee '%s'", destName.toUtf8().c_str());
				return;
			}

			// This player is online, we can forward the tell to him

			// lookup in the IOS proxies
			const TModuleProxyPtr *pproxy = _ChatClients.getA(hostShardId);
			if (pproxy == NULL)
			{
				// no IOS for the hosting shard !
				cucSender.recvFarTellFail(this, senderCharId, destName, TFailInfo::fi_no_ios_module);
nldebug("sendFatTell : no module proxy for shard %u", hostShardId);
				return;
			}

			ICharacterSync *charSync = ICharacterSync::getInstance();

			if (charSync == NULL)
			{
				// no character synchronizer to retrieve sender name !
				cucSender.recvFarTellFail(this, senderCharId, destName, TFailInfo::fi_no_char_sync);
nldebug("sendFarTell : can't finc character sync singleton");
				return;
			}

			ucstring senderName = charSync->getCharacterName(uint32(senderCharId.getShortId()));

			if (senderName.empty())
			{
				// no character synchronizer to retrieve sender name !
				cucSender.recvFarTellFail(this, senderCharId, destName, TFailInfo::fi_sender_char_unknown);
nldebug("sendFarTell : can't get character name from sender char id %s", senderCharId.toString().c_str());
				return;
			}

			// ok, we can send the far tell
			CChatUnifierClientProxy cucDest(*pproxy);
			cucDest.recvFarTell(this, senderCharId, senderName, havePrivilege, destName, text);
		}

		// IOS forward a guild chat message to the unifier
//		void sendFarGuildChat(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, uint32 guildId, const ucstring &text)
//		{
//			nldebug("CU : sendFarGuildChat : module '%s' send far guild chat '%s' from character %s to guild %u chat",
//				sender->getModuleName().c_str(),
//				text.toUtf8().c_str(),
//				senderCharId.toString().c_str(),
//				guildId);
//
//
//			// retrieve the sender name
//			ICharacterSync *charSync = ICharacterSync::getInstance();
//
//			if (charSync == NULL)
//			{
//				// no character synchronizer to retrieve sender name !
//				return;
//			}
//
//			ucstring senderName = charSync->getCharacterName(uint32(senderCharId.getShortId()));
//
//			if (senderName.empty())
//			{
//				// no character synchronizer to retrieve sender name !
//				return;
//			}
//
//			// ok, we can send the guild message
//			// broadcast the message to all the IOS client we known.
//			CChatUnifierClientProxy::broadcast_recvFarGuildChat(_ChatClientsForBroadcast.begin(), _ChatClientsForBroadcast.end(), this, senderName, guildId, text);
//		}

		// IOS forward a guild chat message to the unifier
		// This version use a translated string name as chat content
//		void sendFarGuildChat2(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, uint32 guildId, const ucstring &phraseName)
//		{
//			nldebug("CU : sendFarGuildChat2 : module '%s' send far guild chat phrase '%s' from character %s to guild %u chat",
//				sender->getModuleName().c_str(),
//				phraseName.toUtf8().c_str(),
//				senderCharId.toString().c_str(),
//				guildId);
//
//
//			// retrieve the sender name
//			ICharacterSync *charSync = ICharacterSync::getInstance();
//
//			if (charSync == NULL)
//			{
//				// no character synchronizer to retrieve sender name !
//				return;
//			}
//
//			ucstring senderName = charSync->getCharacterName(uint32(senderCharId.getShortId()));
//
//			if (senderName.empty())
//			{
//				// no character synchronizer to retrieve sender name !
//				return;
//			}
//
//			// ok, we can send the guild message
//			// broadcast the message to all the IOS client we known.
//			CChatUnifierClientProxy::broadcast_recvFarGuildChat2(_ChatClientsForBroadcast.begin(), _ChatClientsForBroadcast.end(), this, senderName, guildId, phraseName);
//		}

		// IOS forward a guild chat message to the unifier
		// This version use a translated string ID as chat content
//		void sendFarGuildChat2Ex(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, uint32 guildId, uint32 phraseId)
//		{
//			nldebug("CU : sendFarGuildChat2Ex : module '%s' send far guild chat phrase Id '%s' from character %s to guild %u chat",
//				sender->getModuleName().c_str(),
//				phraseId,
//				senderCharId.toString().c_str(),
//				guildId);
//
//
//			// retrieve the sender name
//			ICharacterSync *charSync = ICharacterSync::getInstance();
//
//			if (charSync == NULL)
//			{
//				// no character synchronizer to retrieve sender name !
//				return;
//			}
//
//			ucstring senderName = charSync->getCharacterName(uint32(senderCharId.getShortId()));
//
//			if (senderName.empty())
//			{
//				// no character synchronizer to retrieve sender name !
//				return;
//			}
//
//			// ok, we can send the guild message
//			// broadcast the message to all the IOS client we known.
//			CChatUnifierClientProxy::broadcast_recvFarGuildChat2Ex(_ChatClientsForBroadcast.begin(), _ChatClientsForBroadcast.end(), this, senderName, guildId, phraseId);
//		}


		/////////////////////////////////////////////////////////////
		//// commands
		/////////////////////////////////////////////////////////////
	
		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CChatUnifierServer, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CChatUnifierServer, dump, "dump the module internal state", "no param");
		NLMISC_COMMAND_HANDLER_TABLE_END


		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			// call the base class dump
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			log.displayNL("-----------------------------------");
			log.displayNL("Dumping chat unifier server state :");
			log.displayNL("-----------------------------------");
	
			log.displayNL("List of %u chat unifier client :", _ChatClients.getAToBMap().size());
			TChatClients::TAToBMap::const_iterator first(_ChatClients.getAToBMap().begin()), last(_ChatClients.getAToBMap().end());
			for (; first != last; ++first)
			{
				log.displayNL("  Module '%s' for shard %u", first->first->getModuleName().c_str(), first->second);
			}

			log.displayNL("-----------------------------------");

			return true;
		}
	}; 
	
	NLNET_REGISTER_MODULE_FACTORY(CChatUnifierServer, "ChatUnifierServer");



} // namespace RSMGR

// force module linking
void forceChatUnifierLink()
{
}
