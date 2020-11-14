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

#ifndef RY_GUILD_MEMBER_MODULE_H
#define RY_GUILD_MEMBER_MODULE_H

#include "gameplay_module_lib/gameplay_module_lib.h"
#include "guild_manager/guild_char_proxy.h"
#include "game_share/outpost.h"
#include "outpost_manager/outpost.h"

class CGuildCharProxy;
class CGuild;
class CGuildMember;
class CStaticItem;
class CStaticOutpostBuildingSheet;
class CMissionGuild;
class COutpost;

/**
 * Guild member gameplay module.
 * Only instanciated when a member is online.
 * Links the online character class (via CGuildCharProxy) and the persistent guild member class (CGuildMember)
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildMemberModule : public IModule
{
	NL_INSTANCE_COUNTER_DECL(CGuildMemberModule);
public:
	/// pseudo factory for guild member module
	static CGuildMemberModule * createModule( CGuildCharProxy& proxy, CGuildMember* guildMember );
	/// ctor
	CGuildMemberModule( CGuildCharProxy& proxy, CGuildMember* guildMember );
	/// Check if the guild is a proxyfied guild
	bool isGuildProxy();
	/// set the leader of the guild
	virtual void setLeader( uint16 index,uint8 session);
	/// user quit the guild
	virtual void quitGuild();
	/// set the grade of a member
	void setGrade( uint16 index,uint8 session, EGSPD::CGuildGrade::TGuildGrade grade)const;
	/// send a message to all guild members
	void sendMessageToGuildMembers( const std::string &  msg, const TVectorParamCheck & params )const;
	/// invite character in guild
	void inviteCharacterInGuild(CCharacter * invitedCharacter)const;
	/// invite targeted player character in the guild
	void inviteTargetInGuild()const;
	/// kicks a member from the guild
	void kickMember( uint16 index,uint8 session )const;
	/// set message of the day
	void setMOTD(const std::string& motd);	
	
	/// return true if the member can take something in the guild inventory
	virtual bool canTakeGuildItem()const;
	/// return true if the member can invite a player
	virtual bool canInvite()const;
	/// buy a guild option for the guild
	virtual void buyGuildOption( const CStaticItem * form );

	/// return true if the guild member is an outpost admin (USER:OUTPOST_ADMIN in client database)
	virtual bool isOutpostAdmin() const;

	/// the guild attacks an outpost
	virtual COutpost::TChallengeOutpostErrors challengeOutpost(NLMISC::CSheetId outpostSheet, bool simulate = false);
	/// the guild giveup the outpost (ownership or attack)
	virtual void giveupOutpost(NLMISC::CSheetId outpostSheet);
	/// set a squad in the given slot
	virtual void outpostSetSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 shopSquadIndex);
	/// set spawn zone of the squad from the given slot
	virtual void outpostSetSquadSpawnZone(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 spawnZoneIndex);
	/// insert a default squad before the given slot
	virtual void outpostInsertSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot);
	/// remove the squad from the given slot
	virtual void outpostRemoveSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot);
	/// set the expense limit
	virtual void outpostSetExpenseLimit(NLMISC::CSheetId outpostSheet, uint32 expenseLimit);
	/// set the hour of the defense in the defense day
	virtual void outpostSetDefensePeriod(NLMISC::CSheetId outpostSheet, uint8 hour);
	
	/// buy a new building
	virtual bool canBuyOutpostBuilding() const;
	virtual void buyOutpostBuilding(NLMISC::CSheetId sid);

	/// user wanna pick a mission
	CMissionGuild * pickMission( TAIAlias alias );

	// Function to check if the member can pick a mission. By default only Officer and above can pick a guild mission
	virtual bool canPickMission(TAIAlias alias)
	{
		return false;
	}

	/// set the version of last sent info of items in guild inventory
	void setLastSentInfoVersion(uint32 slot, uint8 infoVersion)
	{
		_LastGuildItemInfoVersionsSent[slot] = infoVersion;
	}

	/// return the version of last sent info of items in guild inventory
	uint8 getLastSentInfoVersion(uint32 slot)
	{
		return _LastGuildItemInfoVersionsSent[slot];
	}

protected:
	/// clear online guild properties for a given player
	void clearOnlineGuildProperties();
	/// handler called on parent ( player ) destruction
	void onParentDestructionHandler();
	/// return true if the member can affect another member of the specified grade
	virtual bool canAffectGrade(EGSPD::CGuildGrade::TGuildGrade)const;
	
	/// "core" guild structure
	NLMISC::CRefPtr<CGuildMember>	_GuildMemberCore;

private:
	void _inviteCharacterInGuild(CGuildCharProxy& invitor, CGuildCharProxy& target)const;

	/// version of last sent info of items in guild inventory (only for online players who are in a guild)
	std::vector<uint8>	_LastGuildItemInfoVersionsSent;
};

#include "egs_utils.h"
#include "guild_manager/guild_member.h"
#include "guild_manager/guild.h"

#endif // RY_GUILD_MEMBER_MODULE_H

/* End of guild_member.h */
