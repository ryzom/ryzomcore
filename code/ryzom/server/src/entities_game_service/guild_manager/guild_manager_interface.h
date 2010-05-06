

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
