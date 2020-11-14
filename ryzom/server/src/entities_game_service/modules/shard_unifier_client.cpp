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

#include "nel/misc/eid_translator.h"
#include "nel/net/service.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"


#include "game_share/utils.h"
#include "server_share/mail_forum_itf.h"
#include "server_share/entity_locator_itf.h"
#include "game_share/character_sync_itf.h"
#include "game_share/msg_client_server.h"

//#include "pd_lib/pd_string_manager.h"

#include "shard_unifier_client.h"
#include "player_manager/player_manager_interface.h"
#include "player_manager/character_interface.h"
#include "guild_manager/guild_interface.h"
#include "guild_manager/guild_manager_interface.h"
#include "modules/char_name_mapper_client.h"
#include "modules/guild_unifier.h"

// Send to client if name is valide (true for valide)
void sendIfNameIsValide( uint32 userId, bool nameValide );

// second part of the character creation process
void cbCreateChar_part2(uint32 userId, const CCreateCharMsg &createCharMsg, bool ok);

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CHARSYNC;
using namespace MFS;
using namespace ENTITYLOC;


extern NLMISC::CVariable<uint32> FixedSessionId;

extern CVariable<bool> DontUseSU;
CVariable<bool> SimulateCharacterHasEditSession( "su", "SimulateCharacterHasEditSession", "If 1 simulate that character have alreay an edition sessiion", 0, 0, 1 ); 


class CShardUnifierClient : public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav <CModuleBase> > >,
	public IShardUnifierEvent,
	public CMailForumNotifierSkel,
	public CNameUnifierClientSkel,
	public CEntityLocatorClientSkel
{
public:
	typedef uint32	TCharId;
	// one instance
	static CShardUnifierClient	*_Instance;

	/// Init flag for the EID translator
	bool					_EIdTranslatorInitialized;

	// The module to call for character synchronisation
	TModuleProxyPtr			_CharacterSynch;

	// The list of best combat level to send for a tick loop
	map<CEntityId, sint32>	_BestCombatLevels;

	// The module to call for entity locator
	TModuleProxyPtr			_EntityLocator;


	struct TCreatePendingInfo
	{
		uint32			UserId;
		uint8			CharIndex;
		CCreateCharMsg	CreateCharMsg;
	};

	typedef map<TCharId, TCreatePendingInfo>	TCreateCharPending;
	// The character creation that wait Name unifier response
	TCreateCharPending		_CreateCharPending;


	struct TCharacterNameValidationInfo
	{
		uint32	UserId;
		uint8	CharIndex;
		string	Name;
		uint32	HomeSessionId;
	};

	typedef map<TCharId, TCharacterNameValidationInfo>	TCharacterNameValidations;

//	TCharacterNameValidations	_PendingNameToValidate;
	/// The pre-create char name validation pending
	TCharacterNameValidations	_PendingNameToValidate;

	typedef set<CEntityId>		TOnlineEntities;
	/// The list of character online (from the unified entity locator point of view)
	TOnlineEntities				_OnlineEntities;

	typedef map<string, set<CEntityId> >	TOnlineSpecialEntities;
	/// The map of special user onlines (GM and likes)
	TOnlineSpecialEntities		_OnlineSpecialEntities;


	///////////////////////////////////////////////////////////////////////////
	// methods 
	///////////////////////////////////////////////////////////////////////////

	CShardUnifierClient()
		:	_EIdTranslatorInitialized(false)
	{
		DROP_IF(_Instance != NULL, "IShardUnifierEvent already installed", return);

		CMailForumNotifierSkel::init(this);
		CNameUnifierClientSkel::init(this);
		CEntityLocatorClientSkel::init(this);

		_Instance = this;
	}

	~CShardUnifierClient()
	{
		if (_Instance == this)
			_Instance = NULL;

		// sanity check
		nlassert(_CharacterSynch == NULL);
	}

	static IShardUnifierEvent *getInstance()
	{
		// NB : this is not a singleton, do not instanciate the object now.
		return _Instance;
	}

	std::string buildModuleManifest() const
	{
		// return the manifest of this module
		return "locatorClient(shardId="+toString(IService::getInstance()->getShardId())+")";
	}

	void onModuleUpdate()
	{
		H_AUTO(CShardUnifier_onModuleUpdate);

		if (_CharacterSynch == NULL)
		{
			_BestCombatLevels.clear();
			return;
		}

		vector<TCharBestLevelInfo>	charLevelInfos;
		map<CEntityId, sint32>::iterator first(_BestCombatLevels.begin()), last(_BestCombatLevels.end());
		for(; first != last; ++first)
		{
			TCharBestLevelInfo cbli;
			cbli.setCharEId(first->first);
			cbli.setBestCombatLevel(first->second);

			charLevelInfos.push_back(cbli);
		}

		if (charLevelInfos.empty())
		{
			// nothing to do
			return;
		}

		CCharacterSyncProxy cs(_CharacterSynch);
		
		cs.updateCharsBestLevel(this, charLevelInfos);

		// clear the container for next loop
		_BestCombatLevels.clear();

	}

	void onModuleUp(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "CharacterSynchronisation")
		{
			WARN_IF(_CharacterSynch != NULL, ("CShardUnifierClient::onModuleUp : receiving module for class CharacterSynchronisation as '%s', but already have one as '%s', replacing", module->getModuleName().c_str(), _CharacterSynch->getModuleName().c_str()));
			_CharacterSynch = module;

			// register us in the server
			CNameUnifierProxy nu(module);
			nu.registerNameUnifierClient(this);


			if (IGuildManager::getInstance().guildLoaded())
			{
				// register guilds
				std::vector<CHARSYNC::CGuildInfo> guildInfos;
				IGuildManager::getInstance().fillGuildInfos(guildInfos);

				registerLoadedGuildNames(guildInfos);
			}
		}
		else if (module->getModuleClassName() == "EntityLocator")
		{
			WARN_IF(_EntityLocator != NULL, ("CShardUnifierClient::onModuleUp : receiving module for class EntityLocator as '%s', but already have one as '%s', replacing", module->getModuleName().c_str(), _EntityLocator->getModuleName().c_str()));
			_EntityLocator = module;

			// send the initial state
			vector<uint32>				connectedPlayers;
			vector<TConnectedCharInfo>	connectedChars;

			// fill the connected player list and connected char list
			{
				const IPlayerManager::TMapPlayers &players = IPlayerManager::getInstance().getPlayers();
				IPlayerManager::TMapPlayers::const_iterator first(players.begin()), last(players.end());
				for (; first != last; ++first)
				{
					
					connectedPlayers.push_back(first->first);

					ICharacter *activeChar = ICharacter::getInterface(IPlayerManager::getInstance().getActiveChar(first->first), true);
					 
					if (activeChar != NULL && activeChar->getEnterFlag())
					{
						// ok, we have a connected character to add
						TConnectedCharInfo cci;
						cci.setCharEId(activeChar->getCharId());
						cci.setLastDisconnectionDate(activeChar->getLastDisconnectionDate());
						connectedChars.push_back(cci);
					}
				}

			}

			nldebug("CShardUnifierClient::onModuleUp : send initial state with %u connecter player and %u connected chars to entity locator %s",
				connectedPlayers.size(),
				connectedChars.size(),
				_EntityLocator->getModuleName().c_str());
			
			// send the message to the entity locator
			CEntityLocatorProxy el(_EntityLocator);
			el.initState(this, connectedPlayers, connectedChars);
		}
	}

	void onModuleDown(IModuleProxy *module)
	{
		if (module == _CharacterSynch)
		{
			_CharacterSynch = NULL;

			// fail all validation and character creation pending
			while (!_PendingNameToValidate.empty())
			{
				TCharacterNameValidationInfo &cnvi = _PendingNameToValidate.begin()->second;

				sendIfNameIsValide( cnvi.UserId, false);

				_PendingNameToValidate.erase(_PendingNameToValidate.begin());
			}

			while (!_CreateCharPending.empty())
			{
				TCreatePendingInfo &cpi = _CreateCharPending.begin()->second;

				cbCreateChar_part2(cpi.UserId, cpi.CreateCharMsg, false);

				_CreateCharPending.erase(_CreateCharPending.begin());
			}
		}
		else if (module == _EntityLocator)
		{
			_EntityLocator = NULL;

			nldebug("Uni : onModuleDown : Entity locator down ! removing online %u player info", _OnlineEntities.size());

			// swap the container, so that _OnlineEntities is empty and we can still
			// iterate over the entities that was online from the SU point of view
			// This is needed because when the guild update the member online state
			// if call 'isCharacterOnlineAbroad' that check in the _OnlineEntities 
			// container.
			TOnlineEntities temp;
			temp.swap(_OnlineEntities);

			TOnlineEntities::iterator first(temp.begin()), last(temp.end());
			for (; first != last; ++first)
			{
				updateContactListAndGuilMembers(*first, false);
			}

			// clean the list on online character
			_OnlineEntities.clear();
			_OnlineSpecialEntities.clear();
		}
	}

	/// Force an update of all contact list and guild member list to set the online
	/// status. If the charEid is the one of a locally connected character, then
	/// the status is never set offline (this may be because we have lost to connection
	/// with the SU and all online char sent by the SU are considered offline).
	void updateContactListAndGuilMembers(const NLMISC::CEntityId &charEid, bool online)
	{
		// if the character is not connected here, update the contact list of
		// all characters
		IPlayerManager &pm = IPlayerManager::getInstance();
		if (pm.getActiveChar(uint32(charEid.getShortId()>>4)) == NULL)
		{
			// character not online there, update the contact lists
			const IPlayerManager::TMapPlayers &players = pm.getPlayers();
			IPlayerManager::TMapPlayers::const_iterator first(players.begin()), last(players.end());
			for (; first != last; ++first)
			{
				IPlayerManager::TUserId userId = first->first;
				ICharacter *character = ICharacter::getInterface(pm.getActiveChar(userId), true);
				if (character != NULL)
				{
					character->setContactOnlineStatus(charEid, online);
				}
			}
			// update the guild member list
			IGuildManager::getInstance().characterConnectionEvent(charEid, online);
		}
	}


//	bool onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		if (CMailForumNotifierSkel::onDispatchMessage(sender, message))
//			return true;
//		else if (CNameUnifierClientSkel::onDispatchMessage(sender, message))
//			return true;
//		
//		nlwarning("CShardUnifierClient : Unknown message '%s' received", message.getName().c_str());
//
//		return false;
//	}


	// Coroutine task for name validation
//	void nameValidationTask()
//	{
//		while (!getActiveModuleTask()->isTerminationRequested())
//		{
//			if (!_PendingNameToValidate.empty() && this->_CharacterSynch != NULL)
//			{
//				// validate a name
//				TCharacterNameValidationInfo &charInfo = _PendingNameToValidate.front();
//
//				CNameUnifierProxy nu(_CharacterSynch);
//
//				try
//				{
//					CValidateNameResult res = nu.validateCharacterName(this, charInfo.UserId, charInfo.CharIndex, charInfo.Name, FixedSessionId);
//					nlassert(res.getUserId() == charInfo.UserId);
//					nlassert(res.getCharIndex() == charInfo.CharIndex);
//
//					// send the response to the client
//					sendIfNameIsValide( charInfo.UserId, res.getResult() == TCharacterNameResult::cnr_ok );
//				}
//				catch(...)
//				{
//					// something wrong appended, consider name as unvalid
//					sendIfNameIsValide( charInfo.UserId, false );
//				}
//
//
//				_PendingNameToValidate.pop_front();
//			}
//			else if (!_PendingNameToValidate.empty() && this->_CharacterSynch == NULL)
//			{
//				// no name unifier to validate the name, consider it as invalid
//				TCharacterNameValidationInfo &charInfo = _PendingNameToValidate.front();
//				sendIfNameIsValide( charInfo.UserId, false );
//				_PendingNameToValidate.pop_front();
//			}
//
////			if (!_LoadedNameToValidate.empty() && this->_CharacterSynch != NULL)
////			{
////				// validate a name
////				TCharacterNameValidationInfo &charInfo = _LoadedNameToValidate.front();
////
////				CNameUnifierProxy nu(_CharacterSynch);
////
////				try
////				{
////					CValidateNameResult res = nu.validateCharacterName(this, charInfo.UserId, charInfo.CharIndex, charInfo.Name, FixedSessionId);
////					nlassert(res.getUserId() == charInfo.UserId);
////					nlassert(res.getCharIndex() == charInfo.CharIndex);
////
////					if (res.getResult() != TCharacterNameResult::cnr_ok)
////					{
////						// the name is bad, we need to rename the character
////						nu.renameCharacter(this, (charInfo.UserId<<4)+charInfo.CharIndex);
////						// the SU send back the response directly
////					}
////					else
////					{
////						// ok, nothing to do
////					}
////				}
////				catch(...)
////				{
////					// something wrong appended, consider name as VALID, so do nothing
////				}
////
////
////				_LoadedNameToValidate.pop_front();
////			}
////			else if (!_LoadedNameToValidate.empty() && this->_CharacterSynch == NULL)
////			{
////				// no name unifier to validate the name, consider it as VALID, so validate now
////				vector<TCharSyncResultEntry> charInfos(1);
////				charInfos[0].setCharId((_LoadedNameToValidate.front().UserId << 4) + _LoadedNameToValidate.front().CharIndex);
////				charInfos[0].setCharName(_LoadedNameToValidate.front().Name);
////				charInfos[0].setHomeSessionId(FixedSessionId);
////
////				userCharUpdatedAndValidated(NULL, _LoadedNameToValidate.front().UserId, charInfos);
////				_LoadedNameToValidate.pop_front();
////			}
//			
//			if (!_CreateCharPending.empty() && this->_CharacterSynch != NULL)
//			{
//				// validate a character creation
//
//				TCreatePendingInfo &cpi = _CreateCharPending.front();
//
//				CNameUnifierProxy nu(_CharacterSynch);
//
//				uint32 charId = (cpi.UserId<<4) + cpi.CharIndex;
//
//				try
//				{
//					CValidateNameResult res = nu.assignNameToCharacter(this, charId, cpi.CreateCharMsg.Name.toUtf8(), cpi.CreateCharMsg.Mainland);
//
//					// update the character name with the full name returned by SU
//					cpi.CreateCharMsg.Name = res.getFullName();
//					// call the rest of the creation work
//					cbCreateChar_part2(cpi.UserId, cpi.CreateCharMsg, res.getResult() == TCharacterNameResult::cnr_ok);
//				}
//				catch(...)
//				{
//					// something as failed, consider the name as bad
//					cbCreateChar_part2(cpi.UserId, cpi.CreateCharMsg, false);
//				}
//
//				_CreateCharPending.pop_front();
//			}
//			else if (!_CreateCharPending.empty() && this->_CharacterSynch == NULL)
//			{
//				// no name unifier to validate creation, considere it as invalid
//				TCreatePendingInfo &cpi = _CreateCharPending.front();
//				cbCreateChar_part2(cpi.UserId, cpi.CreateCharMsg, false);
//				_CreateCharPending.pop_front();
//			}
//
//			// wait next loop to work
//			getActiveModuleTask()->yield();
//
//			// flush the message queue
//			getActiveModuleTask()->processPendingMessage(this);
//		}
//	}
//
	bool initModule(const TParsedCommandLine &initInfo)
	{
		bool ret = CModuleBase::initModule(initInfo);
		// start the name unifier coroutine
//		NLNET_START_MODULE_TASK(CShardUnifierClient, nameValidationTask);

		return ret;
	}

	/////////////////////////////////////////////////////////////
	/////////// IShardUnifierEvent implementation
	/////////////////////////////////////////////////////////////

	virtual bool isEidTranslatorInitilazed()
	{
		return _EIdTranslatorInitialized;
	}


	virtual void onNewChar(const CHARSYNC::TCharInfo &charInfo) 
	{
		if (_CharacterSynch == NULL)
			return;

		CCharacterSyncProxy cs(_CharacterSynch);
		
		cs.addCharacter(this, charInfo);
	}

	virtual void onDeleteChar(uint32 charId)
	{
		if (_CharacterSynch == NULL)
			return;

		CCharacterSyncProxy cs(_CharacterSynch);
		
		cs.deleteCharacter(this, charId);
	}

//	virtual void onUpdateCharName(const CEntityId &eid, const std::string &name)
//	{
//		if (_CharacterSynch == NULL)
//			return;
//
//		CCharacterSyncProxy cs(_CharacterSynch);
//
//		cs.updateCharName(this, eid, name);
//	}

	virtual void onUpdateCharGuild(const NLMISC::CEntityId &eid, uint32 guildId)
	{
		if (_CharacterSynch == NULL)
			return;

		CCharacterSyncProxy cs(_CharacterSynch);

		cs.updateCharGuild(this, eid, guildId);

	}

	virtual void onUpdateCharNewbieFlag	(const NLMISC::CEntityId &eid, bool newbie)
	{
		if (_CharacterSynch == NULL)
			return;

		CCharacterSyncProxy cs(_CharacterSynch);

		cs.updateCharNewbieFlag(this, eid, newbie);
	}

	virtual void onUpdateCharBestCombatLevel(const CEntityId &eid, uint32 bestCombatLevel)
	{
		_BestCombatLevels.insert(make_pair(eid, bestCombatLevel));
	}

	virtual void onUpdateCharAllegiance(const NLMISC::CEntityId &eid, PVP_CLAN::TPVPClan cult, PVP_CLAN::TPVPClan civ)
	{
		if (_CharacterSynch == NULL)
			return;

		pair<CHARSYNC::TCult, CHARSYNC::TCivilisation> charSyncAll = convertAllegiance(make_pair(cult, civ));

		CCharacterSyncProxy cs(_CharacterSynch);

		cs.updateCharAllegiance(this, eid, charSyncAll.second, charSyncAll.first);
	}


	virtual void onUpdateCharHomeMainland(const CEntityId &eid, TSessionId homeMainland)
	{
		if (_CharacterSynch == NULL)
			return;

		CCharacterSyncProxy cs(_CharacterSynch);

		cs.updateCharHomeMainlandSessionId(this, eid, homeMainland);
	}

	virtual void onUpdateCharRespawnPoints(const NLMISC::CEntityId &eid, const CONTINENT::TRespawnPointCounters &respawnPoints)
	{
		if (_CharacterSynch == NULL)
			return;

		CCharacterSyncProxy cs(_CharacterSynch);

		cs.updateCharRespawnPoints(this, eid, respawnPoints);
	}


	virtual void onUpdateCharacters(uint32 userId, const std::vector<CHARSYNC::TCharInfo> &charInfos)
	{
		if (_CharacterSynch == NULL)
		{
			// no name unifier, always validate the character names
			// no name unifier to validate the name, consider it as VALID, so validate now
			vector<TCharSyncResultEntry> retCharInfos(charInfos.size());

			for (uint i=0; i<charInfos.size(); ++i)
			{
				retCharInfos[i].setCharId(uint32(charInfos[i].getCharEId().getShortId()));
				retCharInfos[i].setCharName(charInfos[i].getCharName());
				retCharInfos[i].setHomeSessionId(charInfos[i].getHomeSessionId());	
				retCharInfos[i].setIsOwnerOfActiveAnimSession(0);
				retCharInfos[i].setEditionSessionId(SimulateCharacterHasEditSession.get());
			}

			userCharUpdatedAndValidated(NULL, userId, retCharInfos);
			return;
		}

		CCharacterSyncProxy cs(_CharacterSynch);

		cs.syncUserChars(this, userId, charInfos);
	}

	virtual void renameCharacter(const NLMISC::CEntityId &eid)
	{
		if (_CharacterSynch == NULL)
			return;

		CNameUnifierProxy nu(_CharacterSynch);

		nu.renameCharacter(this, uint32(eid.getShortId()));
	}

	virtual void validateGuildName(uint32 guildId, const ucstring &guildName)
	{
		if (_CharacterSynch == NULL)
		{
			// no SU, automaticly validate the name
			IGuildManager::getInstance().createGuildStep2(guildId, guildName, CHARSYNC::TCharacterNameResult::cnr_ok);
			return;
		}

		CNameUnifierProxy nu(_CharacterSynch);

		nu.validateGuildName(this, guildId, guildName);

	}

	virtual void registerLoadedGuildNames(const std::vector<CHARSYNC::CGuildInfo> &guildInfos)
	{
		if (_CharacterSynch == NULL)
			return;

		CNameUnifierProxy nu(_CharacterSynch);

		nu.registerLoadedGuildNames(this, IService::getInstance()->getShardId(), guildInfos);
	}

	virtual void addGuild						(uint32 guildId, const ucstring &guildName)
	{
		if (_CharacterSynch == NULL)
			return;
		CNameUnifierProxy nu(_CharacterSynch);

		nu.addGuild(this, IService::getInstance()->getShardId(), guildId, guildName);
	}

	virtual void removeGuild					(uint32 guildId)
	{
		if (_CharacterSynch == NULL)
			return;
		CNameUnifierProxy nu(_CharacterSynch);

		nu.removeGuild(this, IService::getInstance()->getShardId(), guildId);
	}

	virtual void validateCharacterNameBeforeCreate(uint32 userId, uint8 charIndex, const ucstring &name, uint32 homeSessionId)
	{
		if (DontUseSU.get() == 1)
		{
			// send the response to the client
			sendIfNameIsValide( userId, true );
		}
		else
		{
			// send the validation request to the SU
			if (_CharacterSynch == NULL)
			{
				// oups, no character sync to validate the name, always fail
				sendIfNameIsValide( userId, false );
				nlinfo("VALID_NAME::SU::validateCharacterNameBeforeCreate name %s rejected because we have no character sync to validate the name with using SU", name.toString().c_str());
				return;
			}

			CNameUnifierProxy nu(_CharacterSynch);
			nu.validateCharacterName(this, userId, charIndex, name.toUtf8(), homeSessionId);

			// push a name validation list, waiting for name unifier response
//			_PendingNameToValidate.push_back();
			TCharacterNameValidationInfo charInfo; // = _PendingNameToValidate.back();
			charInfo.UserId = userId;
			charInfo.CharIndex = charIndex;
			charInfo.Name = name.toUtf8();
			charInfo.HomeSessionId = homeSessionId;
			_PendingNameToValidate.insert(make_pair((userId<<4)+charIndex, charInfo));
		}
	}

	virtual bool validateCharacterCreation	(uint32 userId, uint8 charIndex, const CCreateCharMsg &createCharMsg)
	{
		if (DontUseSU.get() == 1)
		{
			cbCreateChar_part2(userId, createCharMsg, true);
			return true;
		}
		
		// send a name assignment to SU name unifier
		if ((_CharacterSynch == NULL) && (DontUseSU.get() == 0))
		{
			nlinfo("VALID_NAME::SU::validateCharacterCreation name %s rejected because we have no character sync to validate the name with using SU", createCharMsg.Name.toString().c_str());
			return false;
		}

		if (_CharacterSynch != NULL)
		{
			CNameUnifierProxy nu(_CharacterSynch);
			nu.assignNameToCharacter(this, (userId << 4)+charIndex, createCharMsg.Name.toUtf8(), createCharMsg.Mainland.asInt());
		}
		
		// store the pending assignement
		TCreatePendingInfo cpi;
		cpi.UserId = userId;
		cpi.CharIndex = charIndex;
		cpi.CreateCharMsg = createCharMsg;

		_CreateCharPending.insert(make_pair((userId<<4)+charIndex, cpi));

		return true;
	}


	void playerConnected(uint32 playerId)
	{
		if (_EntityLocator == NULL)
			return;

		CEntityLocatorProxy el(_EntityLocator);

		el.playerConnected(this, playerId);
	}

	void playerDisconnected(uint32 playerId)
	{
		if (_EntityLocator == NULL)
			return;

		CEntityLocatorProxy el(_EntityLocator);

		el.playerDisconnected(this, playerId);
	}

	void charConnected(const NLMISC::CEntityId &charId, uint32 lastDisconnectionDate)
	{
		if (_EntityLocator == NULL)
			return;

		CEntityLocatorProxy el(_EntityLocator);

		el.charConnected(this, charId, lastDisconnectionDate);
	}

	void charDisconnected(const NLMISC::CEntityId &charId)
	{
		if (_EntityLocator == NULL)
			return;

		CEntityLocatorProxy el(_EntityLocator);

		el.charDisconnected(this, charId);
	}

	// interface for entity locator
	virtual bool isCharacterOnlineAbroad(const CEntityId &charEid) const
	{
		TOnlineEntities::const_iterator it(_OnlineEntities.find(charEid));
		return it != _OnlineEntities.end();
	}

	const set<CEntityId> &getSpecialEntityOnlineAbroad(const std::string &privilege) const
	{
		static set<CEntityId>	emptySet;

		TOnlineSpecialEntities::const_iterator it(_OnlineSpecialEntities.find(privilege));
		if (it == _OnlineSpecialEntities.end())
			return emptySet;
		else
			return it->second;
	}


	///////////////////////////////////////////////////////////////
	///////// 	Mail forum notifier interface implementation
	////////////////////////////////////////////////////////////////
	// A character have received a mail
	void notifyMail(NLNET::IModuleProxy *sender, uint32 charId)
	{
		uint32 userId = charId >> 4;
		uint32 charIndex = charId & 0xf;

		ICharacter *ch = ICharacter::getInterface(IPlayerManager::getInstance().getChar(userId, charIndex), true);

		if (ch != NULL)
		{
			// send impulse to client...
			IPlayerManager::getInstance().sendImpulseToClient(ch->getCharId(), "CONNECTION:MAIL_AVAILABLE");
		}

	}

	// A new message have been posted in a guild forum
	// the notifier client send a notification for each member character
	void notifyForumMessage(NLNET::IModuleProxy *sender, uint32 charId, uint32 guildId, uint32 threadId) 
	{
		uint32 userId = charId >> 4;
		uint32 charIndex = charId & 0xf;

		ICharacter *ch = ICharacter::getInterface(IPlayerManager::getInstance().getChar(userId, charIndex), true);

		if (ch != NULL)
		{
			// send impulse to client...
			IPlayerManager::getInstance().sendImpulseToClient(ch->getCharId(), "CONNECTION:GUILD_MESSAGE_AVAILABLE");
		}
	}

	/////////////////////////////////////////////////////////////////
	// CNameUnifierClientSkel implementation
	/////////////////////////////////////////////////////////////////

	// The name unifier send the initial content for the Eid translator.
	// EGS need to wait until it receive this message before continuing
	// it's startup sequence in order to have coherent name in guild.
	virtual void initEIdTranslator(NLNET::IModuleProxy *sender, bool firstPacket, bool lastPacket, const std::vector < TNameEntry > &nameEntries)
	{
		H_AUTO(initEidTranslator);

		CEntityIdTranslator *eidt = CEntityIdTranslator::getInstance();

		if (firstPacket)
		{
			nldebug("EID : begin init of EID translator...");
			// clear the translator to remove any parasitics data
			eidt->clear();

			IService::getInstance()->addStatusTag("InitEId");
		}

		std::vector < uint32 > releasedNames;
		updateEIdTranslator(sender, releasedNames, nameEntries);


		if (lastPacket)
		{
			nldebug("EID : end init of EID translator, running post init process...");

			_EIdTranslatorInitialized = true;
			
			// ask the guild manager to check all guild member lists
			IGuildManager::getInstance().checkGuildMemberLists();
			// ask the player manager to check contact list of online players
			IPlayerManager::getInstance().checkContactLists();

			IService::getInstance()->removeStatusTag("InitEId");
		}
	}

	// The name unifier send an update for the EID translator.
	// releasedNames contains a list of charId whose names have been released
	// changedNames contains a list of add or update entries
	virtual void updateEIdTranslator(NLNET::IModuleProxy *sender, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames)
	{
		CEntityIdTranslator *eidt = CEntityIdTranslator::getInstance();

		// first, read the released names
		for (uint i=0; i<releasedNames.size(); ++i)
		{
			CEntityId eid(RYZOMID::player, uint64(releasedNames[i]), 0,0);
			if (eidt->isEntityRegistered(eid))
			{
				eidt->unregisterEntity(eid);

				// check guild member list
				IGuildManager::getInstance().playerEntityRemoved(eid);

				// check contact list
				IPlayerManager::getInstance().playerEntityRemoved(eid);
			}
		}

		// second, read the changed/add names
		for (uint i=0; i<changedNames.size(); ++i)
		{
			const TNameEntry &ne = changedNames[i];

			CEntityId eid(RYZOMID::player, uint64((ne.getUserId()<<4)+ne.getCharIndex()), 0,0);

			eidt->updateEntity(eid, capitalize( ne.getName()), ne.getCharIndex(), ne.getUserId(), ne.getUserName(), ne.getShardId());
			// restore the online state if needed
			if (IPlayerManager::getInstance().getChar(uint32(eid.getShortId()>>4), uint32(eid.getShortId()&0xf)))
				eidt->setEntityOnline(eid, true);
			ICharNameMapperClient::getInstance()->mapCharacterName(eid, ne.getName());
		}
	}

	// The name unifier send the result for validation
	// of a character name before creation.
	virtual void validateCharacterNameResult(NLNET::IModuleProxy *sender, const CValidateNameResult &nameResult)
	{
		// retrieve the name in the validation queue
		TCharId charId = (nameResult.getUserId()<<4)+nameResult.getCharIndex();
		TCharacterNameValidations::iterator it(_PendingNameToValidate.find(charId));

		BOMB_IF(it == _PendingNameToValidate.end(), "validateCharacterNameResult : received name validation for char "<<charId<<" but not in pending table !", return);

		TCharacterNameValidationInfo &cnvi = it->second;

		// send the result to the client
		sendIfNameIsValide( cnvi.UserId, nameResult.getResult() == TCharacterNameResult::cnr_ok);

		// cleanup the validation request
		_PendingNameToValidate.erase(it);
	}

	// The name unifier send the result for name assignment
	// of a new character name during creation.
	virtual void assignCharacterNameResult(NLNET::IModuleProxy *sender, const CValidateNameResult &nameResult)
	{
		TCharId charId = (nameResult.getUserId()<<4)+nameResult.getCharIndex();
		TCreateCharPending::iterator it(_CreateCharPending.find(charId));
		
		BOMB_IF(it == _CreateCharPending.end(), "assignCharacterNameResult : received a create char validation for char "<<charId<<" but not in the pending creation char table", return);

		TCreatePendingInfo &cpi = it->second;

		// set the full name for the new character as returned by SU
		cpi.CreateCharMsg.Name = nameResult.getFullName();
		// transmit the creation result
		cbCreateChar_part2(cpi.UserId, cpi.CreateCharMsg, nameResult.getResult() == TCharacterNameResult::cnr_ok);

		// cleanup the item
		_CreateCharPending.erase(it);
	}

	// The name unifier has renamed a character
	// EGS must do what it need to take the new name into account
	void characterRenamed(NLNET::IModuleProxy *sender, uint32 charId, const std::string &newName, bool sendSummary)
	{
		ICharacter *ich = ICharacter::getInterface(charId, false);

		if (ich != NULL)
		{
			ucstring ucName;
			ucName.fromUtf8(newName);

			ich->setName(ucName);
			ich->registerName();
		}

		if (sendSummary)
		{
			// warn the player manager that a character has been renamed
			IPlayerManager::getInstance().characterRenamed(charId, newName);
		}
	}

	// The name unifier has updated/validated/eventualy renamed
	// all the characters send by EGS for a user.
	// EGS can proceed to send the characters summary to client
	void userCharUpdatedAndValidated(NLNET::IModuleProxy *sender, uint32 userId, const vector<TCharSyncResultEntry> &charInfos)
	{
		IPlayerManager::TMapPlayers::const_iterator it(IPlayerManager::getInstance().getPlayers().find(userId));

		if (it != IPlayerManager::getInstance().getPlayers().end())
		{
			const IPlayerManager::SCPlayer &scp = it->second;
			// update the character with data from the SU
			uint32 bitfieldOwnerOfActiveAnimSession = 0;
			uint32 bitfieldOwnerOfEditSession = 0;
			for (uint i=0; i<charInfos.size(); ++i)
			{
				const TCharSyncResultEntry &charInfo = charInfos[i];
				ICharacter *character = ICharacter::getInterface(IPlayerManager::getInstance().getChar(userId, charInfo.getCharId()&0xf), false);
				if (character == NULL)
					continue;

				character->setName(charInfo.getCharName());
				character->setHomeMainlandSessionId(charInfo.getHomeSessionId());
				character->setActiveAnimSessionId(charInfo.getActiveAnimSessionId());
				bitfieldOwnerOfActiveAnimSession |= (((uint32)charInfo.getIsOwnerOfActiveAnimSession()) << i);
				bitfieldOwnerOfEditSession |= (((uint32)charInfo.getEditionSessionId() != 0) << i);
			}
			// ok, we can send the character summary
			sendCharactersSummary( scp.Player, false, bitfieldOwnerOfActiveAnimSession, bitfieldOwnerOfEditSession );
		}
	}

	// The name unifier has failed tp updated/validated/eventualy renamed
	// all the characters send by EGS for a user.
	// EGS can proceed to send the characters summary to client
	// but the character names are perhaps not good ?
	virtual void userCharSyncFailed(NLNET::IModuleProxy *sender, uint32 userId)
	{
		IPlayerManager::TMapPlayers::const_iterator it(IPlayerManager::getInstance().getPlayers().find(userId));

		if (it != IPlayerManager::getInstance().getPlayers().end())
		{
			const IPlayerManager::SCPlayer &scp = it->second;
			// we just validate the character, because SU has failed somewhere
			sendCharactersSummary( scp.Player);
		}
		
	}


	// The name unifier has renamed a guild to resolve a name conflict
	virtual void guildRenamed(NLNET::IModuleProxy *sender, uint32 guildId, const ucstring &newName)
	{
		IGuild *guild = IGuild::getGuildInterface(IGuildManager::getInstance().getGuildFromId(guildId));

		if (guild == NULL)
			return;

		guild->setNameWrap(newName);

		if (IGuildUnifier::getInstance() != NULL)
			IGuildUnifier::getInstance()->broadcastGuildUpdate(guild);

//		IGuildManager::getInstance().addGuildsAwaitingString(newName, guild->getIdWrap());

	}

	// The name unifier respond to EGS about guild name validation request
	virtual void validateGuildNameResult(NLNET::IModuleProxy *sender, uint32 guildId, const ucstring &guildName, TCharacterNameResult result)
	{
		// callback the guild manager

		IGuildManager::getInstance().createGuildStep2(guildId, guildName, result);
	}

	// The unifier has detected an invalid guild/character association
	// and ask to the EGS to remove the character from the guild
	virtual void removeCharFromGuild(NLNET::IModuleProxy *sender, uint32 charId, uint32 guildId)
	{
		nldebug("CShardUnifierClient::removeCharFromGuild : removing char %u from guild %u", charId, guildId);
		CGuild *guild = IGuildManager::getInstance().getGuildFromId(guildId);

		if (guild == NULL)
		{
			nlwarning("CShardUnifierClient:removeCharFromGuild : can't find guild %u to remove char %u",
				guildId, charId);
			return;
		}

		// get an interface pointer (because we don't want to import the CGuild definition)
		IGuild *ig = IGuild::getGuildInterface(guild);
		if (ig->isProxyWrap())
		{
			nlwarning("CShardUnifierClient:removeCharFromGuild : can't remove char %u from guild %u because its a proxy",
				charId, guildId);
			return;
		}

		// remove the member
		ig->removeMember(CEntityId(RYZOMID::player, charId, 0,0));
	}



	// The name unifier respond to EGS after a char sync
	// The result message contains the list of all characters
	// with there unified name and home session id from the
	// ring database
	virtual void characterSyncResult(NLNET::IModule *sender, const std::vector < TCharSyncResultEntry > &charInfos)
	{
	}


	/////////////////////////////////////////////////////////////////
	// CEntityLocatorClient implementation
	/////////////////////////////////////////////////////////////////

	// The entity locator send a list of connection event to EGS
	virtual void connectionEvents(NLNET::IModuleProxy *sender, const std::vector < TCharConnectionEvent > &events)
	{
		nldebug("Uni : connectionEvents: receive %u connection event", events.size());
		// update the list of 'foreign online' character
		for (uint i=0; i<events.size(); ++i)
		{
			const TCharConnectionEvent &cci = events[i];
			
			CEntityId charEid(RYZOMID::player, cci.getCharId(), 0, 0);
			if (cci.getConnection() == true)
			{
				nldebug("  Char %s has connected", charEid.toString().c_str());
				// this character connected
				_OnlineEntities.insert(charEid);

				if (!cci.getPrivilege().empty())
				{
					// this is a special user, store it in the appropriate container
					_OnlineSpecialEntities[cci.getPrivilege()].insert(charEid);
				}

				// Update LastPlayedDate
				ICharacter *character = ICharacter::getInterface(IPlayerManager::getInstance().getActiveChar(uint32(charEid.getShortId()>>4)), true);
				if (character)
					character->setLastConnectionDate(cci.getlastConnectionDate());

			}
			else
			{
				nldebug("  Char %s has disconnected", charEid.toString().c_str());

				// Update LastPlayedDate
				ICharacter *character = ICharacter::getInterface(IPlayerManager::getInstance().getActiveChar(uint32(charEid.getShortId()>>4)), true);
				if (character)
					character->setLastConnectionDate(0);

				// this character disconnected
				_OnlineEntities.erase(charEid);

				// check all 'special' entities (do it 'brute force' coz there are a very small number of special player)
				TOnlineSpecialEntities::iterator first(_OnlineSpecialEntities.begin()), last(_OnlineSpecialEntities.end());
				for (; first != last; ++first)
				{
					first->second.erase(charEid);
				}
			}

			updateContactListAndGuilMembers(charEid, cci.getConnection());
//
//			// if the character is not connected here, update the contact list of
//			// all characters
//			IPlayerManager &pm = IPlayerManager::getInstance();
//			if (pm.getActiveChar(cci.getCharId()>>4) == NULL)
//			{
//				// character not online there, update the contact lists
//				const IPlayerManager::TMapPlayers &players = pm.getPlayers();
//				IPlayerManager::TMapPlayers::const_iterator first(players.begin()), last(players.end());
//				for (; first != last; ++first)
//				{
//					IPlayerManager::TUserId userId = first->first;
//					ICharacter *character = ICharacter::getInterface(pm.getActiveChar(userId), true);
//					if (character != NULL)
//					{
//						character->setContactOnlineStatus(charEid, cci.getConnection());
//					}
//				}
//			}
//
//			// update the guild member list
//			IGuildManager::getInstance().characterConnectionEvent(charEid, cci.getConnection());
		}
	}

	/*************************************************************************/
	/* Commands handler														 */
	/*************************************************************************/
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CShardUnifierClient, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CShardUnifierClient, dump, "Dump the module internal state", "[verbose]");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump)
	{
		NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

		log.displayNL("-----------------------------");
		log.displayNL("Dumping shard unifier client :");
		log.displayNL("-----------------------------");

		log.displayNL("There are %u online entities in the domain", _OnlineEntities.size());

		if (args.size() >0 && args[0] == "verbose")
		{
			// dump the complete list
			TOnlineEntities::iterator first(_OnlineEntities.begin()), last(_OnlineEntities.end());
			for (; first != last; ++first)
			{
				const CEntityId &eid = *first;
				log.displayNL("  Entity %s '%s'", eid.toString().c_str(), NLMISC::CEntityIdTranslator::getInstance()->getByEntity(eid).toUtf8().c_str());
			}

		}
		
		log.displayNL("There are %u privileges list of online entities", _OnlineSpecialEntities.size());
		if (args.size() >0 && args[0] == "verbose")
		{
			TOnlineSpecialEntities::iterator first(_OnlineSpecialEntities.begin()), last(_OnlineSpecialEntities.end());
			for( ; first != last; ++first)
			{
				log.displayNL(" There are %u entities with privileges '%s'", first->second.size(), first->first.c_str());

				set<CEntityId>::iterator f2(first->second.begin()), l2(first->second.end());
				for (; f2 != l2; ++f2)
				{
					log.displayNL("   Entity %s '%s'", f2->toString().c_str(), NLMISC::CEntityIdTranslator::getInstance()->getByEntity(*f2).toUtf8().c_str());
				}
			}
		}

		return true;
	}


};

NLNET_REGISTER_MODULE_FACTORY(CShardUnifierClient, "ShardUnifierClient");


CShardUnifierClient *CShardUnifierClient::_Instance = NULL;

IShardUnifierEvent *IShardUnifierEvent::getInstance()
{
	return CShardUnifierClient::getInstance();
}

pair<CHARSYNC::TCult, CHARSYNC::TCivilisation> IShardUnifierEvent::convertAllegiance(std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance)
{
	std::pair<CHARSYNC::TCult, CHARSYNC::TCivilisation> ret;
	switch (allegiance.first)
	{
	case PVP_CLAN::Kami:		ret.first = CHARSYNC::TCult::c_kami;	break;
	case PVP_CLAN::Karavan:		ret.first = CHARSYNC::TCult::c_karavan;	break;
	default:					ret.first = CHARSYNC::TCult::c_neutral;
	}
	switch (allegiance.second)
	{
	case PVP_CLAN::Fyros:		ret.second = CHARSYNC::TCivilisation::c_fyros;	break;
	case PVP_CLAN::Matis:		ret.second = CHARSYNC::TCivilisation::c_matis;	break;
	case PVP_CLAN::Tryker:		ret.second = CHARSYNC::TCivilisation::c_tryker;	break;
	case PVP_CLAN::Zorai:		ret.second = CHARSYNC::TCivilisation::c_zorai;	break;
	default:					ret.second = CHARSYNC::TCivilisation::c_neutral;
	}

	return ret;
}

