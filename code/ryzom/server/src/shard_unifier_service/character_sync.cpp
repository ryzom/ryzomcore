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

/** The character synchronizer module is responsible for 
 *	synchronizing the ring database with the actual character
 *	when they evolve in the EGS. 
 *	This include character names, guild membering, and best combat level.
 *	Obviously, the module is also responsible for creating and deleting
 *	character record in the database, according to user operation.
 *
 *	The module also implement the character name unifier witch
 *	replace the existing one in legacy Ryzom in the EGS.
 *	The char name unifier is used to guarantee the uniqueness 
 *	of each character name as well as the validity of 
 *	name using a set of rules about name content, length, 
 *	forbidden parts etc.
 */


#include "stdpch.h"
#include <functional>
#include "nel/misc/common.h"
#include "nel/misc/eid_translator.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"

#include "game_share/character_sync_itf.h"
#include "game_share/utils.h"
#include "game_share/shard_names.h"
#include "game_share/ryzom_entity_id.h"
#include "server_share/mysql_wrapper.h"

#include "database_mapping.h"
#include "character_sync.h"
#include "name_manager.h"
#include "entity_locator.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;
using namespace ENTITYLOC;
using namespace MSW;


CVariable<string> RingAccessLimits("r2", "RingAccessLimits", "Limiter for ring access levels - eg 'l5:p3:d0' ", string(), 0, true);

namespace CHARSYNC
{

	class CCharacterSync: 
		public CEmptyModuleCommBehav<CEmptyModuleServiceBehav<CEmptySocketBehav<CModuleBase> > >,
		public CCharacterSyncSkel,
		public CNameUnifierSkel,
		public ICharacterSync
	{
		// Database connection
		MSW::CConnection		_RingDB;

		// Name manager
		CNameManager			_NameManager;

		/// Client that need name table update
		set<TModuleProxyPtr>	_UnifierClients;

		typedef uint32			TCharId;

		/// utility struct to store info abount client eid begin filled
		struct TRunningEidInit
		{
			IModuleProxy				*Module;
			// the next eid info to send
			CNameManager::TCharSlot		NextEidToSend;
		};

		typedef list<TRunningEidInit>	TRunningEidInits;
		/// The list of running eid init
		TRunningEidInits	_RunningEidInits;
	
	public:

		CCharacterSync() 
		{
			CCharacterSyncSkel::init(this);
			CNameUnifierSkel::init(this);
		}


		bool initModule(const TParsedCommandLine &pcl)
		{
			// recall base class
			bool ret = CModuleBase::initModule(pcl);

			// init ring db
			const TParsedCommandLine *initRingDb = pcl.getParam("ring_db");
			if (initRingDb  == NULL)
			{
				nlwarning("RSM : missing ring db connection information");
				return false;
			}

			// connect to the database
			if (!_RingDB.connect(*initRingDb))
			{
				nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
				return false;
			}

			// init the name manager
			_NameManager._Database = &_RingDB;
			_NameManager.loadAllNames();

			// init the shard names table
			CShardNames::getInstance().init(IService::getInstance()->ConfigFile);
			
			return ret;
		}

		void onModuleUp(IModuleProxy *proxy)
		{
		}

		void onModuleDown(IModuleProxy *proxy)
		{
			if (_UnifierClients.find(proxy) != _UnifierClients.end())
			{
				// remove it of the name unifier client list
				_UnifierClients.erase(proxy);
			}
		}

		const std::string &getShardName(uint32 homeSessionId)
		{
			// implement me please !
			nlstop;

			static string emptyString;
			return emptyString;
		}

		void onModuleUpdate()
		{
			H_AUTO(CCharacterSync_onModuleUpdate);

			// update running EId update
			processRunningEidInits();

			// update the name manager
			if (!_NameManager._ReleasedNames.empty() || !_NameManager._ChangedNames.empty())
			{
				// build the vector to send
				vector<TCharId>		releasedNames(_NameManager._ReleasedNames.begin(), _NameManager._ReleasedNames.end());
				vector<TNameEntry>	changedNames;
				
				set<TCharId>::iterator first(_NameManager._ChangedNames.begin()), last(_NameManager._ChangedNames.end());
				for (;first != last; ++first)
				{
					TCharId charId = *first;
					CNameManager::TCharSlot cs(charId);
					changedNames.push_back(TNameEntry());
					TNameEntry &ne = changedNames.back();
					
					const CNameManager::TFullName *pname = _NameManager._Names.getA(cs);
					if (pname != NULL)
					{
						ucstring name;
						name.fromUtf8(CShardNames::getInstance().makeFullName(pname->Name, pname->HomeSessionId));
						ne.setName(name);
						ne.setUserId(cs.UserId);
						ne.setCharIndex(cs.CharIndex);
						
						// retrieve the account name
						ne.setUserName(_NameManager._AccountNames[cs.UserId]);
						
						ne.setShardId(pname->HomeSessionId.asInt());
					}
				}
				
				// now, broadcast the data to all the clients
				CNameUnifierClientProxy::broadcast_updateEIdTranslator(_UnifierClients.begin(), _UnifierClients.end(), this, releasedNames, changedNames);
				
				// cleanup the containers
				_NameManager._ReleasedNames.clear();
				_NameManager._ChangedNames.clear();
			}
		}

		CCharacterPtr lookupChar(CRingUserPtr &ringUser, uint32 charId)
		{
			// load the characters of the user
			if (!ringUser->loadCharacters(_RingDB, __FILE__, __LINE__))
			{
				nlwarning("CharacterSync::lookupChar : Failed to load the character of the ring user %u, character will not be added", ringUser->getObjectId());
				return CCharacterPtr();
			}
			// check if the character already exist or not
			return ringUser->getCharactersById(charId);
		}

		/** Get the name of a user */
		std::string getUserName(uint32 userId)
		{
			CRingUserPtr user = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			if (user == NULL)
			{
				static string emptyString;
				return emptyString;
			}

			return user->getUserName();
		}

		/** Get the name of a character */
		ucstring getCharacterName(uint32 charId)
		{
			CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				static ucstring emptyString;
				return emptyString;
			}

			ucstring ret;
			ret.fromUtf8(CShardNames::getInstance().makeFullName(character->getCharName(), TSessionId(character->getHomeMainlandSessionId())));
			return ret;
		}

		/// Try to find a shard id from a name and session id. Return 0 if not found
		virtual uint32 findCharId(const std::string &charName, uint32 homeSessionId)
		{
			return _NameManager.findCharId(charName, homeSessionId);
		}

		virtual uint32 findCharId(const std::string &charName)
		{
			return _NameManager.findCharId(charName);
		}



		void _renameCharacter(IModuleProxy *sender, uint32 charId, bool updateToClient)
		{
			CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, "Failed to find character "<<charId<<" in database for renaming", return);

			ucstring newName = _NameManager.renameCharacter(charId, character->getHomeMainlandSessionId());

			// update the database
			character->setCharName(newName.toUtf8());
			character->update(_RingDB);

			// send back the new name to the EGS
			CNameUnifierClientProxy nuc (sender);
			nuc.characterRenamed(this, charId, newName.toUtf8(), updateToClient);
		}


		bool csrRenamePlayer(uint32 csrCharId, const string &oldNameIn, const string &newNameIn)
		{
			// 1st, we must retrieve the full name of the csr character
			CCharacterPtr csrCharacter = CCharacter::load(_RingDB, csrCharId, __FILE__, __LINE__);
			BOMB_IF(csrCharacter == NULL, "Failed to load CSR character "<<csrCharId<<" to rename a player", return false);

			// resolve the player character full name
			string oldName;
			TSessionId playerSessionId;
			CShardNames::getInstance().parseRelativeName(TSessionId(csrCharacter->getHomeMainlandSessionId()), oldNameIn, oldName, playerSessionId);

			// retrieve the character id if any
			uint32 playerCharId = _NameManager.findCharId(oldName, playerSessionId.asInt());

			if (playerCharId == 0)
				return false;

			// ok, we have the correct char id !, resolve the new name (we don't realy now what the csr have typed)
			string newName;
			TSessionId notUsed;
			CShardNames::getInstance().parseRelativeName(playerSessionId, newNameIn, newName, notUsed);

			// ok, we can rename the character !
			ucstring ucNewName;
			ucNewName.fromUtf8(newName);
			// check the name validity
			if (_NameManager.isNameUsable(ucNewName, playerCharId >> 4, uint8(playerCharId & 0xf), playerSessionId.asInt()) != TCharacterNameResult::cnr_ok)
			{
				return false;
			}
			// assign the name
			if (!_NameManager.assignName(playerCharId, ucNewName, playerSessionId.asInt()))
			{
				return false;
			}

			// ok, save the name in the database
			CCharacterPtr playerCharacter = CCharacter::load(_RingDB, playerCharId, __FILE__, __LINE__);
			BOMB_IF(playerCharacter == NULL, "Failed to load renamed character "<<playerCharId<<" from database", return false);
			playerCharacter->setCharName(newName);
			playerCharacter->update(_RingDB);

			// inform EGS that the character is renamed
			CNameUnifierClientProxy::broadcast_characterRenamed(
				_UnifierClients.begin(), _UnifierClients.end(), 
				this, playerCharId, CShardNames::getInstance().makeFullName(newName, playerSessionId), false);

			// all done. New name will be dispatched at next module update 
			return true;
		}

		string makeRingAccessString(const CONTINENT::TRespawnPointCounters &respawnPoints)
		{
			map<char, uint32> access;
			CONTINENT::TRespawnPointCounters::const_iterator first(respawnPoints.begin()), last(respawnPoints.end());
			for (; first != last; ++first)
			{
				const CONTINENT::TContinentId &cont = first->first;
				const uint32 &count = first->second;
				if (cont == CONTINENT::FYROS)
					access['D'] += count;
				else if (cont == CONTINENT::ZORAI)
					access['J'] += count;
				if (cont == CONTINENT::BAGNE
					|| cont == CONTINENT::NEXUS
					|| cont == CONTINENT::ROUTE_GOUFFRE
					|| cont == CONTINENT::SOURCES
					|| cont == CONTINENT::TERRE)
					access['P'] += count;
				if (cont == CONTINENT::MATIS)
					access['F'] += count;
				if (cont == CONTINENT::TRYKER)
					access['L'] += count;
			}

			// parse the RingAccessLimits variable and build up a limits map...
			map<char, uint32> limitsMap;
			CSString s= RingAccessLimits.get();
			while (!s.empty())
			{
				CSString chunk= s.strtok(":").toUpper();
				if (chunk.empty())
					continue;
				char key= chunk[0];
				uint32 val= chunk.leftCrop(1).atoui();
				DROP_IF(chunk!=NLMISC::toString("%c%u",key,val),"RingAccessLimit IGNORING string chunk: '"+chunk+"'",continue);
				limitsMap[key]= val;
			}

			string ret("A1:");
			// build the resulting string
			{
				map<char, uint32>::iterator first(access.begin()), last(access.end());
				for (; first != last; ++first)
				{
					// default to no limit
					uint32 limit=~0u;

					// if there's a limit in the limis map then use it instead...
					if (limitsMap.find(first->first)!=limitsMap.end())
					{
						limit= limitsMap[first->first];
						nldebug("RingAccessLimit limiting %c to %u",first->first,limit);
					}

					// add the entry to the result string
					ret += first->first + toString("%u", min(first->second,limit));
					ret += ":";
				}
			}

			return ret;

		}

		/// Check coherency of a character regarding it's guild association

		void checkCharacter(CCharacterPtr character)
		{
			if (character == NULL)
				return;

			uint32 guildId = character->getGuildId();
			if (guildId == 0)
				return;

			// ok, we have a guild to check, get the high bits for the shard id
			uint32 guildShardId = guildId>>20;
			// compare this with the character hame mainland id
			if (guildShardId != character->getHomeMainlandSessionId())
			{
				nlwarning("CCharacterSync::checkCharacter : the character %u is associated to guild %u but this guild is on a different shard, removing the character from the guild",
					character->getObjectId(),
					guildId);
				// bad association, ask the shard owning the guild to remove the offending member
				IModuleProxy *guildHomeModule = IEntityLocator::getInstance()->getLocatorModuleForShard(guildShardId);

				if (guildHomeModule == NULL)
				{
					nlwarning("CCharacterSync::checkCharacter : can't find a module to remove %u character from guild %u (in shard %u)",
						character->getObjectId(),
						guildId,
						guildShardId);
				}
				else
				{
					CNameUnifierClientProxy cuc(guildHomeModule);
					cuc.removeCharFromGuild(this, character->getObjectId(), guildId);
				}
			}
		}


		//////////////////////////////////////////////////
		///// name unifier module interface callbacks
		//////////////////////////////////////////////////

		void processRunningEidInits()
		{
			if (_RunningEidInits.empty())
				return;

			TRunningEidInits::iterator firstRun(_RunningEidInits.begin()), lastRun(_RunningEidInits.end());
			for (; firstRun != lastRun; ++firstRun)
			{
				TRunningEidInit &rei = *firstRun;

				bool firstPacket = false;
				bool lastPacket = false;

				// check for first packed
				if (rei.NextEidToSend == CNameManager::TCharSlot(0,0))
				{
					firstPacket = true;
				}
				
				// send 200 eid each frames
				vector<TNameEntry>	nameEntries;

//				CNameManager::TNamesIndex::TAToBMap::const_iterator first(_NameManager._Names.getAToBMap().begin()), last(_NameManager._Names.getAToBMap().end());
				CNameManager::TNamesIndex::TBToAMap::const_iterator first(_NameManager._Names.getBToAMap().lower_bound(rei.NextEidToSend)), last(_NameManager._Names.getBToAMap().end());
				for (uint i=0; first != last && i<200; ++first, ++i)
				{
					nameEntries.push_back(TNameEntry());
					TNameEntry &ne = nameEntries.back();
					
					ne.setName(CShardNames::getInstance().makeFullName(first->second.Name, first->second.HomeSessionId));
					ne.setUserId(first->first.UserId);
					ne.setCharIndex(first->first.CharIndex);
					
					// retrieve the account name
					ne.setUserName(_NameManager._AccountNames[first->first.UserId]);
					
					ne.setShardId(first->second.HomeSessionId.asInt());
				}

				// check for last packet
				if (first == _NameManager._Names.getBToAMap().end())
				{
					lastPacket = true;
					rei.NextEidToSend = CNameManager::TCharSlot();
				}
				else
				{
					// update the running task
					rei.NextEidToSend = first->first;
				}
				
				// send it
				CNameUnifierClientProxy nuc(rei.Module);
				nuc.initEIdTranslator(this, firstPacket, lastPacket, nameEntries);
			}

			// cleanup loop
			{
				TRunningEidInits::iterator firstRun(_RunningEidInits.begin()), lastRun(_RunningEidInits.end());
				for (; firstRun != lastRun; ++firstRun)
				{
					TRunningEidInit &rei = *firstRun;
					if (rei.NextEidToSend == CNameManager::TCharSlot())
					{
						_RunningEidInits.erase(firstRun);
						break;
					}
				}
			}
		}

		// EGS register it's name unifier in order to receive
		// an updated eid to name translation table
		virtual void registerNameUnifierClient(NLNET::IModuleProxy *sender)
		{
			_UnifierClients.insert(sender);

			// add an eid init runner
			// we must do it with the eid init runner in order to
			// not stall EGS when it receive a full init (more than 100K character)
			// We only send 200 entity at a time, leaving EGS the time to digest 
			// them.
			TRunningEidInit	rei;
			rei.Module = sender;
			rei.NextEidToSend = CNameManager::TCharSlot(0,0);
			_RunningEidInits.push_back(rei);

//			// build the initial name table
//			vector<TNameEntry>	nameEntries;
//
//			CNameManager::TNamesIndex::TAToBMap::const_iterator first(_NameManager._Names.getAToBMap().begin()), last(_NameManager._Names.getAToBMap().end());
//			for (; first != last; ++first)
//			{
//				nameEntries.push_back(TNameEntry());
//				TNameEntry &ne = nameEntries.back();
//
//				ne.setName(CShardNames::getInstance().makeFullName(first->first.Name, first->first.HomeSessionId));
//				ne.setUserId(first->second.UserId);
//				ne.setCharIndex(first->second.CharIndex);
//
//				// retrieve the account name
//				ne.setUserName(_NameManager._AccountNames[first->second.UserId]);
//
//				ne.setShardId(first->first.HomeSessionId.asInt());
//			}
//
//			// send it
//			CNameUnifierClientProxy nuc(sender);
//			nuc.initEIdTranslator(this, nameEntries);

		}
		
		// EGS ask to validate a character name
		// If the NU validate the name, it temporary
		// lock it to the associated player.
		virtual  void validateCharacterName(NLNET::IModuleProxy *sender, uint32 userId, uint8 charIndex, const std::string &name, uint32 homeMainlandSessionId)
		{
			nldebug("CHARSYNC : validateCharacterName : module '%s' ask to validate name '%s' for user %u, character %u",
				sender->getModuleName().c_str(),
				name.c_str(),
				userId,
				charIndex);

			CValidateNameResult	ret;

			TCharacterNameResult result = _NameManager.isNameUsable(name, userId, charIndex, homeMainlandSessionId);

			// fill the return value
			ret.setResult(result);
			ret.setCharIndex(charIndex);
			ret.setUserId(userId);

			// send the response back to sender
			CNameUnifierClientProxy nuc(sender);
			nuc.validateCharacterNameResult(this, ret);
		}

		// EGS ask to assign a name to a character
		virtual void assignNameToCharacter(NLNET::IModuleProxy *sender, uint32 charId, const std::string &name, uint32 homeSessionId)
		{
			CValidateNameResult	ret;

			ret.setUserId(charId >> 4);
			ret.setCharIndex(uint8(charId & 0xf));

			ucstring ucName;
			ucName.fromUtf8(name);

			if (_NameManager.assignName(charId, ucName, homeSessionId))
			{
				// ok, the name assignment is validated
				ret.setResult(TCharacterNameResult::cnr_ok);
				ret.setFullName(ucstring::makeFromUtf8(CShardNames::getInstance().makeFullName(name, TSessionId(homeSessionId))));
			}
			else
			{
				// assignment refused
				nlinfo("VALID_NAME::CHARSYNC::assignNameToCharacter name %s assignement rejected", name.c_str());
				ret.setResult(TCharacterNameResult::cnr_invalid_name);
			}

			// send the response back to sender
			CNameUnifierClientProxy nuc(sender);
			nuc.assignCharacterNameResult(this, ret);
		}

		// EGS ask to rename a character.
		// Renaming consist of assigning a default randomly generated name to the character
		virtual void renameCharacter(NLNET::IModuleProxy *sender, uint32 charId)
		{
			_renameCharacter(sender, charId, true);
		}

		// EGS send info about the list of loaded guild.
		// The name unifier will update is internal name table if needed
		// and rename any guild having a conflicting name.
		// If any guild is renamed, then the name unifier send back
		// a guildRenamed message to EGS.
		virtual void registerLoadedGuildNames(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < CGuildInfo > &guildInfos) 
		{

			std::vector<uint32>			renamedGuildIds;
			std::map<uint32, ucstring>	guilds;

			// build the map of guilds
			for (uint i=0; i<guildInfos.size(); ++i)
			{
				const CGuildInfo &gi = guildInfos[i];

				guilds.insert(make_pair(gi.getGuildId(), gi.getGuildName()));
			}

			// register the guilds
			_NameManager.registerLoadedGuildNames(shardId, guilds, renamedGuildIds);

			// TODO : update renamed guild !

			// update the database with the new guild list
			CShardPtr shard = CShard::load(_RingDB, shardId, __FILE__, __LINE__);
			if (shard == NULL)
			{
				// no entry for this shard, create one
				shard = CShard::createTransient(__FILE__, __LINE__);

				shard->setObjectId(shardId);

				shard->create(_RingDB);
			}

			// load the associated guilds
			BOMB_IF(!shard->loadGuilds(_RingDB, __FILE__, __LINE__), "Failed to load the guilds of shard "<<shardId, return);

			vector<uint32> guildToRemove;
			// for each guild in database, check that they are in the loaded guild or remove them
			std::map < uint32,  CGuildPtr >::const_iterator first(shard->getGuilds().begin()), last(shard->getGuilds().end());
			for (; first != last; ++first)
			{
				uint32 guildId = first->first;
				
				if (guilds.find(guildId) == guilds.end())
				{
					// this guild no more exist, remove the record
					guildToRemove.push_back(guildId);
				}
			}
			
			// remove old guilds
			if (!guildToRemove.empty())
			{
				nlinfo("CCharacterSync:registerLoadedGuild : deleting %u old guilds from database", guildToRemove.size());

				for (uint i=0; i<guildToRemove.size(); ++i)
				{
					WARN_IF(!CGuild::removeById(_RingDB, guildToRemove[i]), "Failed to remove guild "<<guildToRemove[i]);
				}
			}

			uint32 nbCreateGuild = 0;
			// for each loaded guild, check that they are in database or create a new one
			for (uint i=0; i<guildInfos.size(); ++i)
			{
				uint32 guildId = guildInfos[i].getGuildId();
				CGuildPtr guild = shard->getGuildsById(guildId);
				if (guild == NULL)
				{
					// this guild is not in the database
					guild = CGuild::createTransient(__FILE__, __LINE__);

					guild->setObjectId(guildId);
					guild->setGuildName(guildInfos[i].getGuildName().toUtf8());
					guild->setShardId(shardId);

					guild->create(_RingDB);

					++nbCreateGuild;
				}
				else
				{
					// update the guild
					guild->setGuildName(guildInfos[i].getGuildName().toUtf8());
					guild->setShardId(shardId);
					guild->update(_RingDB);
				}
			}

			if (nbCreateGuild)
			{
				nlinfo("CCharacterSync:registerLoadedGuild : %u new guilds created in  database", nbCreateGuild);
			}
		}

		// EGS ask to name unifier to validate a new guild name
		virtual void validateGuildName(NLNET::IModuleProxy *sender, uint32 guildId, const ucstring &guildName)
		{
			TCharacterNameResult ret;
			// ask to name manager to validate the guild name
			ret = _NameManager.isGuildNameUsable(guildName, guildId);

			// send back the result to the client
			CNameUnifierClientProxy nuc(sender);

			nuc.validateGuildNameResult(this, guildId, guildName, ret);
		}

		// EGS add newly created guild info
		virtual void addGuild(NLNET::IModuleProxy *sender, uint32 shardId, uint32 guildId, const ucstring &guildName)
		{
			// register the new guild name in the name manager
			ucstring name;

			// check that the name is correct
			if (!_NameManager.assignGuildName(shardId, guildId, guildName))
			{
				// assignation has assigned a new name because of a conflict
				ucstring newName = _NameManager.getGuildName(guildId);

				// we need to warn EGS that the guild name has been changed by name manager
				CNameUnifierClientProxy nuc(sender);

				nuc.guildRenamed(this, guildId, newName);
				name = newName;
			}
			else
			{
				// ok, the name is valid
				name = guildName;
			}

			// create an entry in the database
			CGuildPtr guild = CGuild::createTransient(__FILE__, __LINE__);

			guild->setObjectId(guildId);
			guild->setGuildName(name.toUtf8());
			guild->setShardId(shardId);
			
			// store the record
			guild->create(_RingDB);
		}

		// EGS remove deleted guild info
		virtual void removeGuild(NLNET::IModuleProxy *sender, uint32 shardId, uint32 guildId)
		{
			// Release the name in the name manager
			_NameManager.releaseGuildName(shardId, guildId);

			// delete the record in the database
			WARN_IF(!CGuild::removeById(_RingDB, guildId), "Failed to erase guild "<<guildId<<" from database");
		}

		//////////////////////////////////////////////////
		///// char sync module interface callbacks
		//////////////////////////////////////////////////

		// A new character have been create by a client
		void addCharacter(NLNET::IModuleProxy *sender, const TCharInfo &charInfo)
		{
			nldebug("CharacterSync::addCharacter : module '%s' add character %s named '%s'", 
				sender->getModuleName().c_str(),
				charInfo.getCharEId().toString().c_str(),
				charInfo.getCharName().c_str());
			// decompose character eid
			uint32 userId = uint32(charInfo.getCharEId().getShortId() >> 4);
			uint32 charId = uint32(charInfo.getCharEId().getShortId());
			uint32 charIndex = charId & 0xf;

			string charName;
			TSessionId sessionId;
			CShardNames::getInstance().parseRelativeName(charInfo.getHomeSessionId(), charInfo.getCharName(), charName, sessionId);

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				nlwarning("CharacterSync::addCharacter : Failed to find a ring user record for user %u, character will not be added", userId);
				return;
			}

			// retrieve the character
			CCharacterPtr character = lookupChar(ru, charId);
			if (character != NULL)
			{
				// the character already exist, just update the record
				nlwarning("CharacterSync::addCharacter : the character %s already exist in the ring database, updating info", charInfo.getCharEId().toString().c_str());

				character->setCharName(charName);
				character->setBestCombatLevel(charInfo.getBestCombatLevel());
				character->setGuildId(charInfo.getGuildId());
				character->setHomeMainlandSessionId(charInfo.getHomeSessionId());
				character->setRace(charInfo.getRace());
				character->setCivilisation(charInfo.getCivilisation());
				character->setCult(charInfo.getCult());
				// save the change
				character->update(_RingDB);
			}
			else
			{
				// the character does not exit, create a new one
				character = CCharacter::createTransient(__FILE__, __LINE__);

				character->setObjectId(charId);
				character->setUserId(userId);
				character->setCharName(charName);
				character->setBestCombatLevel(charInfo.getBestCombatLevel());
				character->setGuildId(charInfo.getGuildId());
				character->setHomeMainlandSessionId(charInfo.getHomeSessionId());
				character->setRace(charInfo.getRace());
				character->setCivilisation(charInfo.getCivilisation());
				character->setCult(charInfo.getCult());
				character->setCreationDate(CTime::getSecondsSince1970());

				// save the new record
				character->create(_RingDB);
			}

			// make sure the name manager is synchronised with database
			_NameManager.assignName(character->getObjectId(), character->getCharName(), character->getHomeMainlandSessionId());
		}

		// A character have been deleted
		void deleteCharacter(NLNET::IModuleProxy *sender, uint32 charId)
		{
			nldebug("CharacterSync::deleteCharacter : module '%s' delete character %u", 
				sender->getModuleName().c_str(),
				charId);
			// decompose character eid
			uint32 userId = charId >> 4;
			uint32 charIndex = charId & 0xf;

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			DROP_IF(ru == NULL, "CharacterSync::deleteCharacter : Failed to find a ring user record for user "+toString(userId)+", character will not be deleted", return);

			// retrieve the character
			CCharacterPtr character = lookupChar(ru, charId);

			DROP_IF(character == NULL, "CharacterSync::deleteCharacter : Failed to find the character "<<charId<<" to delete, nothing to do", return);;

			// inform interested module that this character will be deleted
			NLMISC_BROADCAST_TO_LISTENER(ICharacterSyncCb, onBeforeCharacterDelete(charId));


			// 'erase' the mails associated to this character
			breakable
			{
				// first, create an erase series
				string query = "INSERT INTO mfs_erased_mail_series (erased_char_id, erased_char_name, erase_date) VALUES("+toString(charId)+", '"+character->getCharName()+"', '"+MSW::encodeDate(CTime::getSecondsSince1970())+"')";
				DROP_IF(!_RingDB.query(query), "ERROR : failed to create an mail erase series", break;);

				uint32 eraseSeries = _RingDB.getLastGeneratedId();

				// second, update the erase series in all active mail
				query = "UPDATE mfs_mail SET erase_series = "+toString(eraseSeries)+" WHERE erase_series = 0";
				DROP_IF(!_RingDB.query(query), "ERROR : failed to associate mail with the erase series "+toString(eraseSeries), break;);
			}

			// we found it, so remove it
			WARN_IF(!character->remove(_RingDB), ("Failed to remove char %u from the database", charId));

			// liberate the name in the name manager
			_NameManager.liberateName(charId);

		}

		// The name of a character have been changed
//		void updateCharName(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, const std::string &charName)
//		{
//			nldebug("CharacterSync::updateCharName : module '%s' update character %s name as '%s'", 
//				sender->getModuleName().c_str(),
//				charEId.toString().c_str(),
//				charName.c_str());
//			// decompose character eid
//			uint32 userId = uint32(charEId.getShortId() >> 4);
//			uint32 charId = uint32(charEId.getShortId());
//			uint32 charIndex = charId & 0xf;
//
//			// load the user
//			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
//			DROP_IF(ru == NULL, "CharacterSync::updateCharName: Failed to find a ring user record for user "<<toString(userId)<<", character will not be updated", return);
//
//			// retrieve the character
//			CCharacterPtr character = lookupChar(ru, charId);
//			DROP_IF(character == NULL, "CharacterSync::updateCharName: Failed to find the character '"<<charEId.toString()<<"', character will not be updated", return);
//
//			std::string oldName = character->getCharName();
//			// update the char and save
//			character->setCharName(charName);
//			character->update(_RingDB);
//
//			// callback char sync clients
//			NLMISC_BROADCAST_TO_LISTENER(ICharacterSyncCb, onCharacterNameUpdated(charId, oldName, charName));
//
//			// update the name manager
//			ucstring ucName;
//			ucName.fromUtf8(charName);
//			_NameManager.assignName(charId, ucName, character->getHomeMainlandSessionId());
//		}

		// A character guild have changed
		void updateCharGuild(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, uint32 guildId)
		{
			nldebug("CharacterSync::updateCharGuild : module '%s' update character %s guild as %u", 
				sender->getModuleName().c_str(),
				charEId.toString().c_str(),
				guildId);
			// decompose character eid
			uint32 userId = uint32(charEId.getShortId() >> 4);
			uint32 charId = uint32(charEId.getShortId());
			uint32 charIndex = charId & 0xf;

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			DROP_IF(ru == NULL, ("CharacterSync::updateCharGuild: Failed to find a ring user record for user "+toString(userId)+", character will not be updated"), return);

			// retrieve the character
			CCharacterPtr character = lookupChar(ru, charId);
			DROP_IF(character == NULL, ("CharacterSync::updateCharGuild: Failed to find the character '%s', character will not be updated", charEId.toString().c_str()), return);

			// update the char and save
			character->setGuildId(guildId);
			character->update(_RingDB);

			// Consistency checking : guild id shard and character home mainland must be the same
			checkCharacter(character);
		}

		// Update the respawn points count of a character
		virtual void updateCharRespawnPoints(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, const CONTINENT::TRespawnPointCounters &respawnPoints)
		{
			nldebug("CharacterSync::updateCharRespawnPoints : module '%s' update character %s %u respawn points counters", 
				sender->getModuleName().c_str(),
				charEId.toString().c_str(),
				respawnPoints.size());
			// decompose character eid
			uint32 userId = uint32(charEId.getShortId() >> 4);
			uint32 charId = uint32(charEId.getShortId());
			uint32 charIndex = charId & 0xf;

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			DROP_IF(ru == NULL, ("CharacterSync::updateCharRespawnPoints: Failed to find a ring user record for user "+toString(userId)+", character will not be updated"), return);

			// retrieve the character
			CCharacterPtr character = lookupChar(ru, charId);
			DROP_IF(character == NULL, ("CharacterSync::updateCharRespawnPoints: Failed to find the character '%s', character will not be updated", charEId.toString().c_str()), return);


			string ringAccess = makeRingAccessString(respawnPoints);
			// update the char and save
			character->setRingAccess(ringAccess);
			character->update(_RingDB);
		}

		// Update the newbie flag of a characters
		virtual void updateCharNewbieFlag(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, bool newbie)
		{
			nldebug("CharacterSync::updateCharNewbieFlag : module '%s' update newbie flag to %s for character %s", 
				sender->getModuleName().c_str(),
				newbie ? "true" : "false",
				charEId.toString().c_str());

			TCharId charId = uint32(charEId.getShortId());

			CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, "Failed to load character "<<charId<<" from database", return);

			character->setNewcomer(newbie);
			character->update(_RingDB);;
		}

		// The best level of a character has changed
		void updateCharsBestLevel(NLNET::IModuleProxy *sender, const std::vector < TCharBestLevelInfo > &charLevelInfos) 
		{
			nldebug("CharacterSync::updateCharsBestLevel : module '%s' update best level for %u characters", 
				sender->getModuleName().c_str(),
				charLevelInfos.size());

			std::vector < TCharBestLevelInfo >	fakeCharLevelInfos;

			for (uint i=0; i<charLevelInfos.size(); ++i)
			{
				// decompose character eid
				const TCharBestLevelInfo &cbli = charLevelInfos[i];
				uint32 userId = uint32(cbli.getCharEId().getShortId() >> 4);
				uint32 charId = uint32(cbli.getCharEId().getShortId());
				uint32 charIndex = charId & 0xf;

				// load the user
				CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
				DROP_IF(ru == NULL, ("CharacterSync::updateCharsBestLevel: Failed to find a ring user record for user "+toString(userId)+", character will not be updated"), continue);

				// retrieve the character
				CCharacterPtr character = lookupChar(ru, charId);
				DROP_IF(character == NULL, ("CharacterSync::updateCharsBestLevel: Failed to find the character '%s', character will not be updated", cbli.getCharEId().toString().c_str()), continue);

				// update the char
				character->setBestCombatLevel(cbli.getBestCombatLevel());
				character->update(_RingDB);
			}

			if (!fakeCharLevelInfos.empty())
			{
				updateCharsBestLevel(sender, fakeCharLevelInfos);
			}
		}

		// Update the allegiance of a characters
		void updateCharAllegiance(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, TCivilisation civilisation, TCult cult)
		{
			nldebug("CharacterSync::updateCharAllegiance : module '%s' updates character %s with cult '%s' and civ '%s'", 
				sender->getModuleName().c_str(),
				charEId.toString().c_str(),
				cult.toString().c_str(),
				civilisation.toString().c_str());
			// decompose character eid
			uint32 userId = uint32(charEId.getShortId() >> 4);
			uint32 charId = uint32(charEId.getShortId());

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				nlwarning("CharacterSync::updateCharAllegiance : Failed to find a ring user record for user %u, character will not be deleted", userId);
				return;
			}

			// retrieve the character
			CCharacterPtr character = lookupChar(ru, charId);
			if (character != NULL)
			{
				character->setCivilisation(civilisation);
				character->setCult(cult);
				character->update(_RingDB);
			}
			else
			{
				// the character does not exit, create a new one (the additional fields will be set later by syncUserChars)
				character = CCharacter::createTransient(__FILE__, __LINE__);

				character->setObjectId(charId);
				character->setCivilisation(civilisation);
				character->setCult(cult);

				// save the new record
				character->create(_RingDB);
			}
		}

		// Set HomeMainlandSessionId (when converting an old file)
		void updateCharHomeMainlandSessionId(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, TSessionId homeMainlandSessionId)
		{
			nldebug("CharacterSync::updateCharHomeMainlandSessionId : module '%s' updates character %s with '%u'", 
				sender->getModuleName().c_str(),
				charEId.toString().c_str(),
				homeMainlandSessionId.asInt());
			// decompose character eid
			uint32 userId = uint32(charEId.getShortId() >> 4);
			uint32 charId = uint32(charEId.getShortId());

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				nlwarning("CharacterSync::updateCharHomeMainlandSessionId : Failed to find a ring user record for user %u, character will not be deleted", userId);
				return;
			}

			// retrieve the character
			CCharacterPtr character = lookupChar(ru, charId);
			if (character != NULL)
			{
				if (character->getHomeMainlandSessionId() != homeMainlandSessionId.asInt())
				{
					character->setHomeMainlandSessionId(homeMainlandSessionId.asInt());
					character->update(_RingDB);

					// update the name
					_NameManager.assignName(character->getObjectId(), character->getCharName(), character->getHomeMainlandSessionId());
				}
			}
			else
			{
				// the character does not exit, create a new one (the additional fields will be set later by syncUserChars)
				character = CCharacter::createTransient(__FILE__, __LINE__);

				character->setObjectId(charId);
				character->setHomeMainlandSessionId(homeMainlandSessionId.asInt());

				// save the new record
				character->create(_RingDB);
			}
		}

		// The characters for a player have been loaded
		// EGS send the full list to SU to make
		// sure any divergence in the database is cleared
		void syncUserChars(NLNET::IModuleProxy *sender, uint32 userId, const std::vector < TCharInfo > &charInfos)
		{
			nldebug("CharacterSync::syncUserChars : module '%s' update %u characters", 
				sender->getModuleName().c_str(),
				charInfos.size());

			// prepare a proxy for sending response
			CNameUnifierClientProxy nuc(sender);

			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDB, userId, __FILE__, __LINE__);
			DROP_IF(ru == NULL, ("CharacterSync::syncUserChars: Failed to find a ring user record for user "+toString(userId)+", characters will not be synchronized"), nuc.userCharSyncFailed(this, userId); return);

			// load the characters
			DROP_IF(!ru->loadCharacters(_RingDB, __FILE__, __LINE__), ("CharacterSync::syncUserChars: Failed to load characters for ring user "+toString(userId)+", characters will not be synchronized"), nuc.userCharSyncFailed(this, userId); return);

			const map<uint32, CCharacterPtr>	characters = ru->getCharacters();

			// build a set of existing characters
			set<uint32>	charToRemove;
			{
				map<uint32, CCharacterPtr>::const_iterator first(characters.begin()), last(characters.end());
				for (; first != last; ++first)
				{
					charToRemove.insert(first->first);
				}
			}

			for (uint i=0; i<charInfos.size(); ++i)
			{
				const TCharInfo &charInfo = charInfos[i];
				// decompose character eid
				uint32 charUserId = uint32(charInfo.getCharEId().getShortId() >> 4);
				uint32 charId = uint32(charInfo.getCharEId().getShortId());
				uint32 charIndex = charId & 0xf;

				// remove this char from the remove set
				charToRemove.erase(charId);

				DROP_IF(charUserId != userId, ("CharacterSync::syncUserChars : the "+toString(i)+"th received character belong to user "+toString(charUserId)+" instead of user "+toString(userId)+", skiping"), continue);
				

				std::string oldName;

				// retrieve the character
				CCharacterPtr character = lookupChar(ru, charId);
				if (character != NULL)
				{
					// the character already exist, just update the record
					character->setBestCombatLevel(charInfo.getBestCombatLevel());
					character->setGuildId(charInfo.getGuildId());
					character->setRingAccess(makeRingAccessString(charInfo.getRespawnPoints()));
					character->setRace(charInfo.getRace());
					character->setCivilisation(charInfo.getCivilisation());
					character->setCult(charInfo.getCult());
					character->setNewcomer(charInfo.getNewcomer());
					


					// for session id, if the database contains '0', then assume the EGS have
					// the correct value
					if (character->getHomeMainlandSessionId() == 0)
						character->setHomeMainlandSessionId(charInfo.getHomeSessionId());

					// save the change
					character->update(_RingDB);
				}
				else
				{
					// the character does not exit, create a new one
					character = CCharacter::createTransient(__FILE__, __LINE__);

					string charName;
					TSessionId homeSession;
					CShardNames::getInstance().parseRelativeName(charInfo.getHomeSessionId(), charInfo.getCharName(), charName, homeSession);

					if (_NameManager.isNameUsable(charName, userId, uint8(charIndex), charInfo.getHomeSessionId()) != CHARSYNC::TCharacterNameResult::cnr_ok)
					{
						nlinfo("CharacterSync::syncUserChars : character %u use name '%s' (parsed from '%s') that is not usable, renaming it", 
							charId,
							charName.c_str(),
							charInfo.getCharName().c_str());

						// set a default name for now
						charName = _NameManager.generateDefaultName(charId, charInfo.getHomeSessionId()).toUtf8();
					}

					character->setObjectId(charId);
					character->setUserId(userId);
					character->setCharName(charName);
					character->setBestCombatLevel(charInfo.getBestCombatLevel());
					character->setGuildId(charInfo.getGuildId());
					character->setHomeMainlandSessionId(charInfo.getHomeSessionId());
					character->setRingAccess(makeRingAccessString(charInfo.getRespawnPoints()));
					character->setRace(charInfo.getRace());
					character->setCivilisation(charInfo.getCivilisation());
					character->setCult(charInfo.getCult());
					character->setNewcomer(charInfo.getNewcomer());
					character->setCreationDate(CTime::getSecondsSince1970());

					// save the new record
					character->create(_RingDB);

					// store the new name assoc
					_NameManager.assignName(character->getObjectId(), character->getCharName(), character->getHomeMainlandSessionId(), true);
				}

				// auto correct invalid char name in database
				if (character->getCharName().find("_default") != string::npos
					|| character->getCharName().empty())
				{
					string charName;
					TSessionId sessionId;
					CShardNames::getInstance().parseRelativeName(TSessionId(character->getHomeMainlandSessionId()), charInfo.getCharName(), charName, sessionId);
					
					// check that the name is valid before replacing it
					if (_NameManager.isNameUsable(charName, userId, uint8(charIndex), charInfo.getHomeSessionId()) == CHARSYNC::TCharacterNameResult::cnr_ok)
					{
						// we use the name provided by EGS instead of the database name
						character->setCharName(charName);
						character->update(_RingDB);
						// store the new name assoc
						_NameManager.assignName(character->getObjectId(), character->getCharName(), character->getHomeMainlandSessionId());
					}
				}

				checkCharacter(character);
			}

			// delete any no more existing chars
			while (!charToRemove.empty())
			{
				uint32 charId = *charToRemove.begin();
				CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
				if (character != NULL)
				{
					nldebug("syncUserChars : deleting unused character %u from database", charId);
					character->remove(_RingDB);
				}

				charToRemove.erase(charToRemove.begin());
			}

			// synchronize name manager
			uint8 charIndex = 0;
			for (uint i=0; i<charInfos.size(); ++i)
			{
				const TCharInfo &ci = charInfos[i];

				uint8 currentCharIndex = uint8(ci.getCharEId().getShortId()&0xf);

				CCharacterPtr character = lookupChar(ru, uint32(ci.getCharEId().getShortId()));
				BOMB_IF(character == NULL, "syncUserChars : cannot find character "<<uint32(ci.getCharEId().getShortId())<<" in the database !", continue);

				while (charIndex < currentCharIndex)
				{
					// erase any reserved name
					_NameManager.liberateName((userId<<4)+charIndex);
					++charIndex;
				}

//				// check that the name is valid
//				if (_NameManager.isNameUsable(character->getCharName(), userId, currentCharIndex, character->getHomeMainlandSessionId()) == CHARSYNC::TCharacterNameResult::cnr_ok)
//				{
//					_NameManager.assignName(uint32(ci.getCharEId().getShortId()), character->getCharName(), character->getHomeMainlandSessionId());
//				}
//				else
//				{
//					nlinfo("CharacterSync::syncUserChars : character %u use name '%s' that is not usable, renaming it", 
//						uint32(ci.getCharEId().getShortId()), 
//						character->getCharName().c_str());
//
//					// we need to rename this character !
//					_renameCharacter(sender, uint32(ci.getCharEId().getShortId()), false);
//				}

				charIndex = currentCharIndex+1;
			}

			while (charIndex < 16)
			{
				// erase any reserved name
				_NameManager.liberateName((userId<<4)+charIndex);
				++charIndex;
			}

			// inform our client that all name have been checked

			// build the result vector
			vector<TCharSyncResultEntry>	charEntries(ru->getCharacters().size());
			std::map<uint32, CCharacterPtr>::const_iterator first(ru->getCharacters().begin()), last(ru->getCharacters().end());
			for (uint i=0; first != last; ++first, ++i)
			{
				const CCharacterPtr &character = first->second;
				charEntries[i].setCharId(first->first);
				charEntries[i].setCharName(ucstring::makeFromUtf8(CShardNames::getInstance().makeFullName(character->getCharName(), TSessionId(character->getHomeMainlandSessionId()))));
				charEntries[i].setHomeSessionId(TSessionId(character->getHomeMainlandSessionId()));
				charEntries[i].setEditionSessionId(0);
				charEntries[i].setActiveAnimSessionId(0);
				{				
					CSString query;
					TSessionId editSession; 
					query << "SELECT session_id FROM sessions WHERE owner = "<<first->first;
					query << " AND session_type = 'st_edit'";
					if (_RingDB.query(query) )
					{
						std::auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDB.storeResult());

						bool sessionClosed = false;
						if (!result->getNumRows() == 0)
						{
							result->fetchRow();
							uint32 editSession;
							result->getField(0, editSession);
							charEntries[i].setEditionSessionId(editSession);
						}
						// get the row						
					}
				}
				


				// Find out if the character has an active session for which he is the DM (for resume button)
				bool isOwnerOfResumableSession = false;
				TSessionId currentSessionId = character->getCurrentSession();
				if (currentSessionId != 0)
				{
					CSessionPtr session = CSession::load(_RingDB, currentSessionId, __FILE__, __LINE__);
					if (session != NULL)
					{
						isOwnerOfResumableSession = ((session->getOwnerId() == first->first) &&
							(session->getSessionType() == RSMGR::TSessionType::st_anim) &&
							(session->getState() == RSMGR::TSessionState::ss_open));
						if (isOwnerOfResumableSession)
						{
							nldebug("Char %u can resume session %u", first->first, currentSessionId.asInt());
						}
					}
				}
				charEntries[i].setIsOwnerOfActiveAnimSession(isOwnerOfResumableSession);
			}
			nuc.userCharUpdatedAndValidated(this, userId, charEntries);
		}


		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CCharacterSync, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CCharacterSync, dump, "dump the internal state of the module", "no args");
			NLMISC_COMMAND_HANDLER_ADD(CCharacterSync, loadReservedNameFile, "Add the content of a reserved names file in the reserved names list", "<filename>");
			NLMISC_COMMAND_HANDLER_ADD(CCharacterSync, relocChar, "relocalise a character on the specified shard", "<charId | charEID> <newShardId | newShardName>");
//			NLMISC_COMMAND_HANDLER_ADD(CCharacterSync, changeCharOwner, "change the owner of a character in the database. NB : you need to manually change the account_xxx_yy.pdr files", "<charId>|<charEID> <newUserId>|<newUserName>");
		NLMISC_COMMAND_HANDLER_TABLE_END

//		NLMISC_CLASS_COMMAND_DECL(changeCharOwner)
//		{
//			if (args.size() != 2)
//				return false;
//
//			uint32 charId;
//			uint32 srdUserId;
//			uint32 dstUserId;
//
//			if (args[0].size() > 1 && args[0][0] == '(')
//			{
//				// parameter is an EID
//				CEntityId eid(args[0]);
//				charId = uint32(eid.getShortId());
//			}
//			else
//			{
//				NLMISC::fromString(args[0], charId);
//			}
//
//			srcUserId = charId >> 4;
//
//			uint32 dstUserId;
//			NLMISC::fromString(args[1], dstUserId);
//
//			if (dstUserId == 0)
//			{
//				// try to retrieve the user by name
//				CSString req;
//				req << "SELECT user_id FROM ring_users WHERE user_name = '" << args[1] << "'";
//				if (!_RingDB.query(req))
//				{
//					log.displayNL("Can't find user '%s'", args[1].c_str());
//					return true;
//				}
//
//				std::auto_ptr<CStoreResult> result = std::auto_ptr<CStoreResult>(_RingDB.storeResult());
//
//				result->fetchRow();
//				result->getField(0, dstUserId);
//
//				if (dstUserId == 0)
//				{
//					log.displayNL("Can't find user '%s'", args[1].c_str());
//					return true;
//				}
//			}
//
//			CRingUserPtr dstUser = CRingUser::load(_RingDB, dstUserId, __FILE__, __LINE__);
//			if (dstUserId == NULL)
//			{
//				log.displayNL("Can't load dest user %u from database", dstUserId);
//				return true;
//			}
//
//			CRingUserPtr srcUser = CRingUser::load(_RingDB, srcUserId, __FILE__, __LINE__);
//			if (srcUserId == NULL)
//			{
//				log.displayNL("Can't load src user %u from database", srcUserId);
//				return true;
//			}
//
//			IEntityLocator *el = IEntityLocator::getInstance();
//			if (el == NULL)
//			{
//				log.displayNL("Can't reloc character because entity locator is not found");
//				return true;
//			}
//
//			if (el->isUserOnline(srcUserId))
//			{
//				log.displayNL("Can't reloc character because src user %u owner of char %u is online",
//					userId,
//					charId);
//				return true;
//			}
//			if (el->isUserOnline(dstUserId))
//			{
//				log.displayNL("Can't reloc character because dst user %u is online",
//					userId);
//				return true;
//			}
//
//			// ok, try to load the character
//			CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
//			if (character == NULL)
//			{
//				log.displayNL("Can't reloc character %u of user %u because character not found in database",
//					charId,
//					userId);
//				return true;
//			}
//
//			// load all characters of dst users
//			dstUser->loadCharacters(_RingDB, __FILE__, __LINE__);
//			// make sure the dest slot is empty
//
//			// update the home mainland
//			character->setHomeMainlandSessionId(shardId);
//			character->update(_RingDB);
//
//			log.displayNL("The character %u of user %u has been relocated on shard %u", 
//				charId,
//				userId,
//				shardId);
//
//			return true;
//		}

		NLMISC_CLASS_COMMAND_DECL(relocChar)
		{
			if (args.size() != 2)
				return false;

			uint32 charId;
			uint32 userId;

			if (args[0].size() > 1 && args[0][0] == '(')
			{
				// parameter is an EID
				CEntityId eid(args[0]);
				charId = uint32(eid.getShortId());
			}
			else
			{
				NLMISC::fromString(args[0], charId);
			}

			userId = charId >> 4;

			uint32 shardId;
			NLMISC::fromString(args[1], shardId);

			if (shardId == 0)
			{
				// try to retrieve the shard by name
				CShardNames &cn = CShardNames::getInstance();
				const CShardNames::TSessionNames &shardNames = cn.getSessionNames();

				for (uint i=0; i<shardNames.size(); ++i)
				{
					if (nlstricmp(args[1].c_str(), shardNames[i].DisplayName.c_str()) == 0)
					{
						// ok, we found it
						shardId = shardNames[i].SessionId.asInt();
					}
				}
			}

			if (shardId == 0)
			{
				log.displayNL("Can't reloc character because new shard '%s' is invalid (either as a shard id or shard name)",
					args[1].c_str());
				return true;

			}

			IEntityLocator *el = IEntityLocator::getInstance();
			if (el == NULL)
			{
				log.displayNL("Can't reloc character because entity locator is not found");
				return true;
			}

			if (el->isUserOnline(userId))
			{
				log.displayNL("Can't reloc character because user %u owner of char %u is online",
					userId,
					charId);
				return true;
			}

			// ok, try to load the character
			CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				log.displayNL("Can't reloc character %u of user %u because character not found in database",
					charId,
					userId);
				return true;
			}

			// update the home mainland
			character->setHomeMainlandSessionId(shardId);
			character->update(_RingDB);


			// update the entity translator
			CEntityId eid(RYZOMID::player, charId);

			ucstring charName;
			sint8 charSlot;
			uint32 tempUserId;
			string userName;
			bool online;
			NLMISC::CEntityIdTranslator::getInstance()->getEntityIdInfo(eid, charName, charSlot, tempUserId, userName, online);
			NLMISC::CEntityIdTranslator::getInstance()->updateEntity(eid, charName, charSlot, userId, userName, shardId);

			vector<TCharId>		releasedNames;
			vector<TNameEntry>	changedNames;

			TNameEntry nameEntry;
			nameEntry.setCharIndex(charSlot);
			nameEntry.setName(charName);
			nameEntry.setShardId(shardId);
			nameEntry.setUserId(userId);
			nameEntry.setUserName(userName);
			changedNames.push_back(nameEntry);

			// now, broadcast the data to all the clients
			CNameUnifierClientProxy::broadcast_updateEIdTranslator(_UnifierClients.begin(), _UnifierClients.end(), this, releasedNames, changedNames);

			log.displayNL("The character %u of user %u has been relocated on shard %u", 
				charId,
				userId,
				shardId);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			if (args.size() > 1)
				return false;

			log.displayNL("---------------------------");
			log.displayNL("Dumping character sync    :");
			log.displayNL("---------------------------");

			{
				log.displayNL("  Listing %u character synch clients :", _UnifierClients.size());
				set<TModuleProxyPtr>::iterator first(_UnifierClients.begin()), last(_UnifierClients.end());
				for (; first != last; ++first)
				{
					TModuleProxyPtr client = *first;
					log.displayNL("    '%s' (class '%s')", 
						client->getModuleName().c_str(), 
						client->getModuleClassName().c_str());
				}
			}
			log.displayNL("");
			_NameManager.cmdHandler_dump("", args, log, quiet, human);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(loadReservedNameFile)
		{
			if (args.size() != 1)
				return false;

			string filename = args[0];

			if (!_NameManager.loadReservedNames(filename.c_str()))
			{
				log.displayNL("Failed to load the file '%s'", filename.c_str());
			}
			else
			{
				log.displayNL("The content of the file '%s' has been added to the reserved names list", filename.c_str());
			}

			return true;
		}

	};

	NLNET_REGISTER_MODULE_FACTORY(CCharacterSync, "CharacterSynchronisation");

} // namespace CHARSYNC


// nel command handler forwarded from EGS from client GM
NLMISC_COMMAND( renamePlayer, "rename a player", "<CSR eId><player name><new playerName>" )
{
	CHARSYNC::CCharacterSync *cs = dynamic_cast<CHARSYNC::CCharacterSync	*>(CHARSYNC::ICharacterSync::getInstance());
	if (cs == NULL)
	{
		return true;
	}

	BOMB_IF(args.size() != 3, "renamePlayer must receive 3 argument, "<<args.size()<<" received instead", return false);
	CEntityId csrId(args[0]);
	BOMB_IF(csrId == CEntityId::Unknown, "Invalid csr EID '"<<args[0]<<"', evaluated to unknown EID", return false);
	BOMB_IF(csrId.getType() != RYZOMID::player, "Invalid csr EID '"<<args[0]<<"', type must be player, found '"<<RYZOMID::toString(RYZOMID::TTypeId(csrId.getType()))<<"'", return false);

	// extract the csr charId
	uint32 csrCharId = uint32(csrId.getShortId());

	return static_cast<CHARSYNC::CCharacterSync*>(CHARSYNC::ICharacterSync::getInstance())->csrRenamePlayer(csrCharId, args[1], args[2]);
}


// force module linking
void forceCharSyncLink()
{
}
