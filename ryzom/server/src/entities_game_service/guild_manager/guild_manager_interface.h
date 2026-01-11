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


#ifndef GUILD_MANAGER_INTERFACE_H
#define GUILD_MANAGER_INTERFACE_H

#include "nel/misc/ucstring.h"

#include "game_share/character_sync_itf.h"

namespace EGSPD
{
	class CGuildMemberPD;
	class CGuildPD;
	class CGuildContainerPD;
}

class CGuild;

class IGuildManager
{
public:

	static IGuildManager &getInstance();

	/// A member have been changed in some way
	virtual void	guildMemberChanged		(EGSPD::CGuildMemberPD *guildMemberPd) =0;
	/// the member list has been changed (ether add/replace or delete)
	virtual void	guildMemberListChanged	(EGSPD::CGuildPD *guildPd) =0;

	virtual void	updateGuildMembersStringIds() =0;
	virtual void	createGuildStep2		(uint32 guildId, const ucstring &guildName, CHARSYNC::TCharacterNameResult result) =0;
	virtual CGuild	*getGuildFromId			(uint32 guildId) =0;
//	virtual void	addGuildsAwaitingString	( const ucstring & guildStr, uint32 guildId ) =0;

	/// get raw access to the guild list (not const)
	virtual const EGSPD::CGuildContainerPD *getGuildContainer() const =0;

	/// A player entity have been removed from eid translator, check all guild member list
	virtual void	playerEntityRemoved(const NLMISC::CEntityId &eid) =0;


	/// check if guilds have been loaded
	virtual bool	guildLoaded() =0;

	/// fill a list with all guild descriptors
	virtual void	fillGuildInfos(std::vector<CHARSYNC::CGuildInfo> &guildInfos) =0;

	/// Check all guild member lists against the entity translator
	virtual void	checkGuildMemberLists()=0;

	// A character connect/disconnect on another shard, update the online tags
	virtual void	characterConnectionEvent(const NLMISC::CEntityId &eid, bool online) =0;

};

#endif // GUILD_MANAGER_INTERFACE_H
