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

#ifndef RY_GUILD_OFFICER_MODULE_H
#define RY_GUILD_OFFICER_MODULE_H

#include "guild_manager/guild_member_module.h"

/**
 * Guild officer gameplay module.  See CGuildMemberModule for cmethod comments
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildOfficerModule: public  CGuildMemberModule
{
	NL_INSTANCE_COUNTER_DECL(CGuildOfficerModule);
public:

	CGuildOfficerModule( CGuildCharProxy & proxy, CGuildMember* guildMember)
		:CGuildMemberModule(proxy,guildMember){}	
	virtual bool canAffectGrade(EGSPD::CGuildGrade::TGuildGrade grade)const;
	virtual bool canInvite()const;

	// Function to check if the member can pick a mission.
	// By default only Officers and above can pick a guild mission.
	// So we don't need to implement this function for the other grades
	virtual bool canPickMission(TAIAlias alias)
	{
		return true;
	}
};
;


#endif // RY_GUILD_OFFICER_MODULE_H

/* End of guild_officer_module.h */
