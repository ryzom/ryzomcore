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

#ifndef RY_GUILD_LEADER_MODULE_H
#define RY_GUILD_LEADER_MODULE_H

#include "guild_high_officer_module.h"

/**
 * Guild leader gameplay module.  See CGuildMemberModule for cmethod comments
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildLeaderModule : public  CGuildHighOfficerModule
{
	NL_INSTANCE_COUNTER_DECL(CGuildLeaderModule);
public:
	
	CGuildLeaderModule( CGuildCharProxy & proxy, CGuildMember* guildMember)
		:CGuildHighOfficerModule(proxy,guildMember){}	
	
	virtual void setLeader( uint16 index,uint8 session);
	virtual void quitGuild();
	virtual bool canAffectGrade(EGSPD::CGuildGrade::TGuildGrade)const;
	virtual bool canInvite()const;
};


#endif // RY_GUILD_LEADER_MODULE_H

/* End of guild_leader_module.h */
