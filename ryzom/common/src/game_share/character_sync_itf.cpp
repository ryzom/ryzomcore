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

#include "stdpch.h"

#include "character_sync_itf.h"

namespace CHARSYNC
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CCharacterSyncSkel::TMessageHandlerMap &CCharacterSyncSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("CS_ACH"), &CCharacterSyncSkel::addCharacter_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_DCH"), &CCharacterSyncSkel::deleteCharacter_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPDCG"), &CCharacterSyncSkel::updateCharGuild_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPDCR"), &CCharacterSyncSkel::updateCharRespawnPoints_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPDCL"), &CCharacterSyncSkel::updateCharsBestLevel_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPCNF"), &CCharacterSyncSkel::updateCharNewbieFlag_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPDCA"), &CCharacterSyncSkel::updateCharAllegiance_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPDCHMSI"), &CCharacterSyncSkel::updateCharHomeMainlandSessionId_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CS_UPDCS"), &CCharacterSyncSkel::syncUserChars_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CCharacterSyncSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	void CCharacterSyncSkel::addCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_addCharacter_CS_ACH);
		TCharInfo	charInfo;
			nlRead(__message, serial, charInfo);
		addCharacter(sender, charInfo);
	}

	void CCharacterSyncSkel::deleteCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_deleteCharacter_CS_DCH);
		uint32	charId;
			nlRead(__message, serial, charId);
		deleteCharacter(sender, charId);
	}

	void CCharacterSyncSkel::updateCharGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_updateCharGuild_CS_UPDCG);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		updateCharGuild(sender, charEId, guildId);
	}

	void CCharacterSyncSkel::updateCharRespawnPoints_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_updateCharRespawnPoints_CS_UPDCR);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		CONTINENT::TRespawnPointCounters	respawnPoints;
			nlRead(__message, serialCont, respawnPoints);
		updateCharRespawnPoints(sender, charEId, respawnPoints);
	}

	void CCharacterSyncSkel::updateCharsBestLevel_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_updateCharsBestLevel_CS_UPDCL);
		std::vector < TCharBestLevelInfo >	charLevelInfos;
			nlRead(__message, serialCont, charLevelInfos);
		updateCharsBestLevel(sender, charLevelInfos);
	}

	void CCharacterSyncSkel::updateCharNewbieFlag_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_updateCharNewbieFlag_CS_UPCNF);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		bool	newbie;
			nlRead(__message, serial, newbie);
		updateCharNewbieFlag(sender, charEId, newbie);
	}

	void CCharacterSyncSkel::updateCharAllegiance_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_updateCharAllegiance_CS_UPDCA);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		TCivilisation	civilisation;
			nlRead(__message, serial, civilisation);
		TCult	cult;
			nlRead(__message, serial, cult);
		updateCharAllegiance(sender, charEId, civilisation, cult);
	}

	void CCharacterSyncSkel::updateCharHomeMainlandSessionId_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_updateCharHomeMainlandSessionId_CS_UPDCHMSI);
		NLMISC::CEntityId	charEId;
			nlRead(__message, serial, charEId);
		TSessionId	homeMainlandSessionId;
			nlRead(__message, serial, homeMainlandSessionId);
		updateCharHomeMainlandSessionId(sender, charEId, homeMainlandSessionId);
	}

	void CCharacterSyncSkel::syncUserChars_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CCharacterSyncSkel_syncUserChars_CS_UPDCS);
		uint32	userId;
			nlRead(__message, serial, userId);
		std::vector < TCharInfo >	charInfos;
			nlRead(__message, serialCont, charInfos);
		syncUserChars(sender, userId, charInfos);
	}
		// A new character have been create by a client
	void CCharacterSyncProxy::addCharacter(NLNET::IModule *sender, const TCharInfo &charInfo)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->addCharacter(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charInfo);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_addCharacter(__message, charInfo);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A character have been deleted
	void CCharacterSyncProxy::deleteCharacter(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->deleteCharacter(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_deleteCharacter(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// A character guild have changed
	void CCharacterSyncProxy::updateCharGuild(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, uint32 guildId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateCharGuild(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId, guildId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateCharGuild(__message, charEId, guildId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the respawn points count of a character
	void CCharacterSyncProxy::updateCharRespawnPoints(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, const CONTINENT::TRespawnPointCounters &respawnPoints)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateCharRespawnPoints(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId, respawnPoints);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateCharRespawnPoints(__message, charEId, respawnPoints);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the best level for a set of characters
	void CCharacterSyncProxy::updateCharsBestLevel(NLNET::IModule *sender, const std::vector < TCharBestLevelInfo > &charLevelInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateCharsBestLevel(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charLevelInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateCharsBestLevel(__message, charLevelInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the newbie flag of a characters
	void CCharacterSyncProxy::updateCharNewbieFlag(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, bool newbie)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateCharNewbieFlag(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId, newbie);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateCharNewbieFlag(__message, charEId, newbie);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Update the allegiance of a characters
	void CCharacterSyncProxy::updateCharAllegiance(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, TCivilisation civilisation, TCult cult)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateCharAllegiance(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId, civilisation, cult);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateCharAllegiance(__message, charEId, civilisation, cult);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The home mainland has changed (used when converting a character file from an old version)
	void CCharacterSyncProxy::updateCharHomeMainlandSessionId(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, TSessionId homeMainlandSessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateCharHomeMainlandSessionId(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charEId, homeMainlandSessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateCharHomeMainlandSessionId(__message, charEId, homeMainlandSessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The characters for a player have been loaded
		// EGS send the full list to SU to make
		// sure any divergence in the database is cleared
		// SU send back the list of character with there
		// unified names and home session ID
	void CCharacterSyncProxy::syncUserChars(NLNET::IModule *sender, uint32 userId, const std::vector < TCharInfo > &charInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->syncUserChars(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId, charInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_syncUserChars(__message, userId, charInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_addCharacter(NLNET::CMessage &__message, const TCharInfo &charInfo)
	{
		__message.setType("CS_ACH");
			nlWrite(__message, serial, const_cast < TCharInfo& > (charInfo));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_deleteCharacter(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("CS_DCH");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_updateCharGuild(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, uint32 guildId)
	{
		__message.setType("CS_UPDCG");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));
			nlWrite(__message, serial, guildId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_updateCharRespawnPoints(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, const CONTINENT::TRespawnPointCounters &respawnPoints)
	{
		__message.setType("CS_UPDCR");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));
			nlWrite(__message, serialCont, const_cast < CONTINENT::TRespawnPointCounters& > (respawnPoints));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_updateCharsBestLevel(NLNET::CMessage &__message, const std::vector < TCharBestLevelInfo > &charLevelInfos)
	{
		__message.setType("CS_UPDCL");
			nlWrite(__message, serialCont, const_cast < std::vector < TCharBestLevelInfo >& > (charLevelInfos));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_updateCharNewbieFlag(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, bool newbie)
	{
		__message.setType("CS_UPCNF");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));
			nlWrite(__message, serial, newbie);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_updateCharAllegiance(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, TCivilisation civilisation, TCult cult)
	{
		__message.setType("CS_UPDCA");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));
			nlWrite(__message, serial, civilisation);
			nlWrite(__message, serial, cult);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_updateCharHomeMainlandSessionId(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, TSessionId homeMainlandSessionId)
	{
		__message.setType("CS_UPDCHMSI");
			nlWrite(__message, serial, const_cast < NLMISC::CEntityId& > (charEId));
			nlWrite(__message, serial, homeMainlandSessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CCharacterSyncProxy::buildMessageFor_syncUserChars(NLNET::CMessage &__message, uint32 userId, const std::vector < TCharInfo > &charInfos)
	{
		__message.setType("CS_UPDCS");
			nlWrite(__message, serial, userId);
			nlWrite(__message, serialCont, const_cast < std::vector < TCharInfo >& > (charInfos));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CNameUnifierSkel::TMessageHandlerMap &CNameUnifierSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("NU_RNUC"), &CNameUnifierSkel::registerNameUnifierClient_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_VCN"), &CNameUnifierSkel::validateCharacterName_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_ANTC"), &CNameUnifierSkel::assignNameToCharacter_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_RC"), &CNameUnifierSkel::renameCharacter_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_RLGN"), &CNameUnifierSkel::registerLoadedGuildNames_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_VGN"), &CNameUnifierSkel::validateGuildName_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_AG"), &CNameUnifierSkel::addGuild_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NU_RG"), &CNameUnifierSkel::removeGuild_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CNameUnifierSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	void CNameUnifierSkel::registerNameUnifierClient_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &/* __message */)
	{
		H_AUTO(CNameUnifierSkel_registerNameUnifierClient_NU_RNUC);
		registerNameUnifierClient(sender);
	}

	void CNameUnifierSkel::validateCharacterName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_validateCharacterName_NU_VCN);
		uint32	userId;
			nlRead(__message, serial, userId);
		uint8	charIndex;
			nlRead(__message, serial, charIndex);
		std::string	name;
			nlRead(__message, serial, name);
		uint32	homeMainlandSessionId;
			nlRead(__message, serial, homeMainlandSessionId);
		validateCharacterName(sender, userId, charIndex, name, homeMainlandSessionId);
	}

	void CNameUnifierSkel::assignNameToCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_assignNameToCharacter_NU_ANTC);
		uint32	charId;
			nlRead(__message, serial, charId);
		std::string	name;
			nlRead(__message, serial, name);
		uint32	homeSessionId;
			nlRead(__message, serial, homeSessionId);
		assignNameToCharacter(sender, charId, name, homeSessionId);
	}

	void CNameUnifierSkel::renameCharacter_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_renameCharacter_NU_RC);
		uint32	charId;
			nlRead(__message, serial, charId);
		renameCharacter(sender, charId);
	}

	void CNameUnifierSkel::registerLoadedGuildNames_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_registerLoadedGuildNames_NU_RLGN);
		uint32	chardId;
			nlRead(__message, serial, chardId);
		std::vector < CGuildInfo >	guildInfos;
			nlRead(__message, serialCont, guildInfos);
		registerLoadedGuildNames(sender, chardId, guildInfos);
	}

	void CNameUnifierSkel::validateGuildName_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_validateGuildName_NU_VGN);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		ucstring	guildName;
			nlRead(__message, serial, guildName);
		validateGuildName(sender, guildId, guildName);
	}

	void CNameUnifierSkel::addGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_addGuild_NU_AG);
		uint32	shardId;
			nlRead(__message, serial, shardId);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		ucstring	guildName;
			nlRead(__message, serial, guildName);
		addGuild(sender, shardId, guildId, guildName);
	}

	void CNameUnifierSkel::removeGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierSkel_removeGuild_NU_RG);
		uint32	shardId;
			nlRead(__message, serial, shardId);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		removeGuild(sender, shardId, guildId);
	}
		// EGS register it's name unifier in order to receive
		// an updated eid to name translation table
	void CNameUnifierProxy::registerNameUnifierClient(NLNET::IModule *sender)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerNameUnifierClient(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_registerNameUnifierClient(__message);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS ask to validate a character name
		// If the NU valide the name, it temporary
		// lock it to the associated player.
		// This function is called before character creation.
	void CNameUnifierProxy::validateCharacterName(NLNET::IModule *sender, uint32 userId, uint8 charIndex, const std::string &name, uint32 homeMainlandSessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->validateCharacterName(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId, charIndex, name, homeMainlandSessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_validateCharacterName(__message, userId, charIndex, name, homeMainlandSessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS ask to assign a name to a character
		// This function is called during character creation
	void CNameUnifierProxy::assignNameToCharacter(NLNET::IModule *sender, uint32 charId, const std::string &name, uint32 homeSessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->assignNameToCharacter(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, name, homeSessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_assignNameToCharacter(__message, charId, name, homeSessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS ask to rename a character.
		// Renaming consist of assigning a default ramdomly generated name to the character
	void CNameUnifierProxy::renameCharacter(NLNET::IModule *sender, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->renameCharacter(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_renameCharacter(__message, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS send info about the list of loaded guild.
		// The name unifier will update is internal name table if needed
		// and rename any guild having a conflicting name.
		// If any guild is renamed, then the name unifier send back
		// a guildRenamed message to EGS.
	void CNameUnifierProxy::registerLoadedGuildNames(NLNET::IModule *sender, uint32 chardId, const std::vector < CGuildInfo > &guildInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerLoadedGuildNames(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), chardId, guildInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_registerLoadedGuildNames(__message, chardId, guildInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS ask to the name unifier to validate a new guild name
	void CNameUnifierProxy::validateGuildName(NLNET::IModule *sender, uint32 guildId, const ucstring &guildName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->validateGuildName(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId, guildName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_validateGuildName(__message, guildId, guildName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS add newly created guild info
	void CNameUnifierProxy::addGuild(NLNET::IModule *sender, uint32 shardId, uint32 guildId, const ucstring &guildName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->addGuild(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId, guildId, guildName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_addGuild(__message, shardId, guildId, guildName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// EGS remove deleted guild info
	void CNameUnifierProxy::removeGuild(NLNET::IModule *sender, uint32 shardId, uint32 guildId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->removeGuild(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId, guildId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_removeGuild(__message, shardId, guildId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_registerNameUnifierClient(NLNET::CMessage &__message)
	{
		__message.setType("NU_RNUC");


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_validateCharacterName(NLNET::CMessage &__message, uint32 userId, uint8 charIndex, const std::string &name, uint32 homeMainlandSessionId)
	{
		__message.setType("NU_VCN");
			nlWrite(__message, serial, userId);
			nlWrite(__message, serial, charIndex);
			nlWrite(__message, serial, const_cast < std::string& > (name));
			nlWrite(__message, serial, homeMainlandSessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_assignNameToCharacter(NLNET::CMessage &__message, uint32 charId, const std::string &name, uint32 homeSessionId)
	{
		__message.setType("NU_ANTC");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < std::string& > (name));
			nlWrite(__message, serial, homeSessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_renameCharacter(NLNET::CMessage &__message, uint32 charId)
	{
		__message.setType("NU_RC");
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_registerLoadedGuildNames(NLNET::CMessage &__message, uint32 chardId, const std::vector < CGuildInfo > &guildInfos)
	{
		__message.setType("NU_RLGN");
			nlWrite(__message, serial, chardId);
			nlWrite(__message, serialCont, const_cast < std::vector < CGuildInfo >& > (guildInfos));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_validateGuildName(NLNET::CMessage &__message, uint32 guildId, const ucstring &guildName)
	{
		__message.setType("NU_VGN");
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, const_cast < ucstring& > (guildName));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_addGuild(NLNET::CMessage &__message, uint32 shardId, uint32 guildId, const ucstring &guildName)
	{
		__message.setType("NU_AG");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, const_cast < ucstring& > (guildName));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierProxy::buildMessageFor_removeGuild(NLNET::CMessage &__message, uint32 shardId, uint32 guildId)
	{
		__message.setType("NU_RG");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serial, guildId);


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CNameUnifierClientSkel::TMessageHandlerMap &CNameUnifierClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("NUC_IET"), &CNameUnifierClientSkel::initEIdTranslator_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_UET"), &CNameUnifierClientSkel::updateEIdTranslator_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_VCNR"), &CNameUnifierClientSkel::validateCharacterNameResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_ACNR"), &CNameUnifierClientSkel::assignCharacterNameResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_CR"), &CNameUnifierClientSkel::characterRenamed_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_UCUAV"), &CNameUnifierClientSkel::userCharUpdatedAndValidated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_UCSF"), &CNameUnifierClientSkel::userCharSyncFailed_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_GR"), &CNameUnifierClientSkel::guildRenamed_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_VGNR"), &CNameUnifierClientSkel::validateGuildNameResult_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("NUC_RCFG"), &CNameUnifierClientSkel::removeCharFromGuild_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CNameUnifierClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
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


	void CNameUnifierClientSkel::initEIdTranslator_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_initEIdTranslator_NUC_IET);
		bool	firstPacket;
			nlRead(__message, serial, firstPacket);
		bool	lastPacket;
			nlRead(__message, serial, lastPacket);
		std::vector < TNameEntry >	nameEntries;
			nlRead(__message, serialCont, nameEntries);
		initEIdTranslator(sender, firstPacket, lastPacket, nameEntries);
	}

	void CNameUnifierClientSkel::updateEIdTranslator_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_updateEIdTranslator_NUC_UET);
		std::vector < uint32 >	releasedNames;
			nlRead(__message, serialCont, releasedNames);
		std::vector < TNameEntry >	changedNames;
			nlRead(__message, serialCont, changedNames);
		updateEIdTranslator(sender, releasedNames, changedNames);
	}

	void CNameUnifierClientSkel::validateCharacterNameResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_validateCharacterNameResult_NUC_VCNR);
		CValidateNameResult	nameResult;
			nlRead(__message, serial, nameResult);
		validateCharacterNameResult(sender, nameResult);
	}

	void CNameUnifierClientSkel::assignCharacterNameResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_assignCharacterNameResult_NUC_ACNR);
		CValidateNameResult	nameResult;
			nlRead(__message, serial, nameResult);
		assignCharacterNameResult(sender, nameResult);
	}

	void CNameUnifierClientSkel::characterRenamed_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_characterRenamed_NUC_CR);
		uint32	charId;
			nlRead(__message, serial, charId);
		std::string	newName;
			nlRead(__message, serial, newName);
		bool	sendSummary;
			nlRead(__message, serial, sendSummary);
		characterRenamed(sender, charId, newName, sendSummary);
	}

	void CNameUnifierClientSkel::userCharUpdatedAndValidated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_userCharUpdatedAndValidated_NUC_UCUAV);
		uint32	userId;
			nlRead(__message, serial, userId);
		std::vector < TCharSyncResultEntry >	charInfos;
			nlRead(__message, serialCont, charInfos);
		userCharUpdatedAndValidated(sender, userId, charInfos);
	}

	void CNameUnifierClientSkel::userCharSyncFailed_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_userCharSyncFailed_NUC_UCSF);
		uint32	userId;
			nlRead(__message, serial, userId);
		userCharSyncFailed(sender, userId);
	}

	void CNameUnifierClientSkel::guildRenamed_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_guildRenamed_NUC_GR);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		ucstring	newName;
			nlRead(__message, serial, newName);
		guildRenamed(sender, guildId, newName);
	}

	void CNameUnifierClientSkel::validateGuildNameResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_validateGuildNameResult_NUC_VGNR);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		ucstring	guildName;
			nlRead(__message, serial, guildName);
		TCharacterNameResult	result;
			nlRead(__message, serial, result);
		validateGuildNameResult(sender, guildId, guildName, result);
	}

	void CNameUnifierClientSkel::removeCharFromGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CNameUnifierClientSkel_removeCharFromGuild_NUC_RCFG);
		uint32	charId;
			nlRead(__message, serial, charId);
		uint32	guildId;
			nlRead(__message, serial, guildId);
		removeCharFromGuild(sender, charId, guildId);
	}
		// The name unifier send the initial content for the Eid translator.
		// EGS need to wait until it receive this message before continuing
		// it's startup sequence in order to have coherent name in guild.
	void CNameUnifierClientProxy::initEIdTranslator(NLNET::IModule *sender, bool firstPacket, bool lastPacket, const std::vector < TNameEntry > &nameEntries)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->initEIdTranslator(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), firstPacket, lastPacket, nameEntries);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_initEIdTranslator(__message, firstPacket, lastPacket, nameEntries);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier send an update for the EID translator.
		// releasedNames contains a list of charId whose names have been released
		// changedNames contains a list of add or update entries
	void CNameUnifierClientProxy::updateEIdTranslator(NLNET::IModule *sender, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->updateEIdTranslator(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), releasedNames, changedNames);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_updateEIdTranslator(__message, releasedNames, changedNames);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier send the result for validation
		// of a character name before creation.
	void CNameUnifierClientProxy::validateCharacterNameResult(NLNET::IModule *sender, const CValidateNameResult &nameResult)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->validateCharacterNameResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), nameResult);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_validateCharacterNameResult(__message, nameResult);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier send the result for name assignment
		// of a new character name during creation.
	void CNameUnifierClientProxy::assignCharacterNameResult(NLNET::IModule *sender, const CValidateNameResult &nameResult)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->assignCharacterNameResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), nameResult);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_assignCharacterNameResult(__message, nameResult);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier has renamed a character
		// EGS must do what it need to take the new name into account
	void CNameUnifierClientProxy::characterRenamed(NLNET::IModule *sender, uint32 charId, const std::string &newName, bool sendSummary)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->characterRenamed(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, newName, sendSummary);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_characterRenamed(__message, charId, newName, sendSummary);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier has updated/validated/eventualy renamed
		// all the characters send by EGS for a user.
		// EGS can proceed to send the characters summary to client
		// The result message contains the list of all characters
		// with their unified name and home session id from the
		// ring database
	void CNameUnifierClientProxy::userCharUpdatedAndValidated(NLNET::IModule *sender, uint32 userId, const std::vector < TCharSyncResultEntry > &charInfos)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->userCharUpdatedAndValidated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId, charInfos);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_userCharUpdatedAndValidated(__message, userId, charInfos);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier has failed tp updated/validated/eventualy renamed
		// all the characters send by EGS for a user.
		// EGS can proceed to send the characters summary to client
		// but the character names are perhaps not good ?
	void CNameUnifierClientProxy::userCharSyncFailed(NLNET::IModule *sender, uint32 userId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->userCharSyncFailed(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), userId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_userCharSyncFailed(__message, userId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier has renamed a guild to resolve a name conflict
	void CNameUnifierClientProxy::guildRenamed(NLNET::IModule *sender, uint32 guildId, const ucstring &newName)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->guildRenamed(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId, newName);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_guildRenamed(__message, guildId, newName);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The name unifier respond to EGS about guild name validation request
	void CNameUnifierClientProxy::validateGuildNameResult(NLNET::IModule *sender, uint32 guildId, const ucstring &guildName, TCharacterNameResult result)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->validateGuildNameResult(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), guildId, guildName, result);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_validateGuildNameResult(__message, guildId, guildName, result);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The unifier has detected an invalid guild/character association
		// and ask to the EGS to remove the character from the guild
	void CNameUnifierClientProxy::removeCharFromGuild(NLNET::IModule *sender, uint32 charId, uint32 guildId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->removeCharFromGuild(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, guildId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_removeCharFromGuild(__message, charId, guildId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_initEIdTranslator(NLNET::CMessage &__message, bool firstPacket, bool lastPacket, const std::vector < TNameEntry > &nameEntries)
	{
		__message.setType("NUC_IET");
			nlWrite(__message, serial, firstPacket);
			nlWrite(__message, serial, lastPacket);
			nlWrite(__message, serialCont, const_cast < std::vector < TNameEntry >& > (nameEntries));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_updateEIdTranslator(NLNET::CMessage &__message, const std::vector < uint32 > &releasedNames, const std::vector < TNameEntry > &changedNames)
	{
		__message.setType("NUC_UET");
			nlWrite(__message, serialCont, const_cast < std::vector < uint32 >& > (releasedNames));
			nlWrite(__message, serialCont, const_cast < std::vector < TNameEntry >& > (changedNames));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_validateCharacterNameResult(NLNET::CMessage &__message, const CValidateNameResult &nameResult)
	{
		__message.setType("NUC_VCNR");
			nlWrite(__message, serial, const_cast < CValidateNameResult& > (nameResult));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_assignCharacterNameResult(NLNET::CMessage &__message, const CValidateNameResult &nameResult)
	{
		__message.setType("NUC_ACNR");
			nlWrite(__message, serial, const_cast < CValidateNameResult& > (nameResult));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_characterRenamed(NLNET::CMessage &__message, uint32 charId, const std::string &newName, bool sendSummary)
	{
		__message.setType("NUC_CR");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, const_cast < std::string& > (newName));
			nlWrite(__message, serial, sendSummary);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_userCharUpdatedAndValidated(NLNET::CMessage &__message, uint32 userId, const std::vector < TCharSyncResultEntry > &charInfos)
	{
		__message.setType("NUC_UCUAV");
			nlWrite(__message, serial, userId);
			nlWrite(__message, serialCont, const_cast < std::vector < TCharSyncResultEntry >& > (charInfos));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_userCharSyncFailed(NLNET::CMessage &__message, uint32 userId)
	{
		__message.setType("NUC_UCSF");
			nlWrite(__message, serial, userId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_guildRenamed(NLNET::CMessage &__message, uint32 guildId, const ucstring &newName)
	{
		__message.setType("NUC_GR");
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, const_cast < ucstring& > (newName));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_validateGuildNameResult(NLNET::CMessage &__message, uint32 guildId, const ucstring &guildName, TCharacterNameResult result)
	{
		__message.setType("NUC_VGNR");
			nlWrite(__message, serial, guildId);
			nlWrite(__message, serial, const_cast < ucstring& > (guildName));
			nlWrite(__message, serial, result);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CNameUnifierClientProxy::buildMessageFor_removeCharFromGuild(NLNET::CMessage &__message, uint32 charId, uint32 guildId)
	{
		__message.setType("NUC_RCFG");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, guildId);


		return __message;
	}

}
