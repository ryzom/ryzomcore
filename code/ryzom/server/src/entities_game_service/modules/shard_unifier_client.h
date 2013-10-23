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

#ifndef SHARD_UNIFIER_CLIENT_H
#define SHARD_UNIFIER_CLIENT_H


#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "game_share/character_sync_itf.h"
#include "game_share/pvp_clan.h"

class CCreateCharMsg;

class IShardUnifierEvent
{
public:

	static IShardUnifierEvent *getInstance();
	
	// interface for character synchronization
	virtual bool isEidTranslatorInitilazed		() =0;
	virtual void onNewChar						(const CHARSYNC::TCharInfo &charInfo) = 0;
	virtual void onDeleteChar					(uint32 charId) =0;
	virtual void onUpdateCharGuild				(const NLMISC::CEntityId &eid, uint32 guildId) = 0;
	virtual void onUpdateCharNewbieFlag			(const NLMISC::CEntityId &eid, bool newbie) = 0;
	virtual void onUpdateCharBestCombatLevel	(const NLMISC::CEntityId &eid, uint32 bestCombatLevel) = 0;
	virtual void onUpdateCharAllegiance			(const NLMISC::CEntityId &eid, PVP_CLAN::TPVPClan cult, PVP_CLAN::TPVPClan civ) = 0;
	virtual void onUpdateCharHomeMainland		(const NLMISC::CEntityId &eid, TSessionId homeMainland) = 0;
	virtual void onUpdateCharRespawnPoints		(const NLMISC::CEntityId &eid, const CONTINENT::TRespawnPointCounters &respawnPoints) = 0;
	virtual void onUpdateCharacters				(uint32 userId, const std::vector<CHARSYNC::TCharInfo> &charInfos) = 0;

	// interface for name unifier (player part)
	virtual void validateCharacterNameBeforeCreate	(uint32 userId, uint8 charIndex, const ucstring &name, uint32 homeSessionId) =0;
	virtual bool validateCharacterCreation			(uint32 userId, uint8 charIndex, const CCreateCharMsg &createCharMsg) =0;
	virtual void renameCharacter					(const NLMISC::CEntityId &eid) =0;

	// interface for name unifier (guild part)
	virtual void validateGuildName				(uint32 guildId, const ucstring &guildName) =0;
	virtual void registerLoadedGuildNames		(const std::vector<CHARSYNC::CGuildInfo> &guildInfos) =0;
	virtual void addGuild						(uint32 guildId, const ucstring &guildName) =0;
	virtual void removeGuild					(uint32 guildId) =0;

	// interface for player and char status
	virtual void playerConnected		(uint32 playerId) =0;
	virtual void playerDisconnected		(uint32 playerId) =0;
	virtual void charConnected			(const NLMISC::CEntityId &charId, uint32 lastDisconnectionDate) =0;
	virtual void charDisconnected		(const NLMISC::CEntityId &charId) =0;
	
	// interface for entity locator
	virtual bool isCharacterOnlineAbroad(const NLMISC::CEntityId &charEid) const =0;
	virtual const std::set<NLMISC::CEntityId> &getSpecialEntityOnlineAbroad(const std::string &privilege) const = 0;


	// utility
	static std::pair<CHARSYNC::TCult, CHARSYNC::TCivilisation> convertAllegiance(std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance);

};


#endif //SHARD_UNIFIER_CLIENT_H
