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

#ifndef RY_GUILD_HIGH_OFFICER_MODULE_H
#define RY_GUILD_HIGH_OFFICER_MODULE_H

#include "guild_officer_module.h"

class CStaticItem;

/**
 * Guild high officer gameplay module. See CGuildMemberModule for cmethod comments
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildHighOfficerModule : public  CGuildOfficerModule
{
	NL_INSTANCE_COUNTER_DECL(CGuildHighOfficerModule);
public:
	
	CGuildHighOfficerModule( CGuildCharProxy & proxy, CGuildMember* guildMember)
		:CGuildOfficerModule(proxy,guildMember){}	
	virtual bool canAffectGrade(EGSPD::CGuildGrade::TGuildGrade grade)const;
	virtual bool canInvite()const;
	virtual void buyGuildOption( const CStaticItem * form );
	virtual bool canTakeGuildItem()const;
	virtual bool canBuyOutpostBuilding() const;
	virtual void buyOutpostBuilding(NLMISC::CSheetId sid);
	virtual bool isOutpostAdmin() const;
	virtual COutpost::TChallengeOutpostErrors challengeOutpost(NLMISC::CSheetId outpostSheet, bool simulate);
	virtual void giveupOutpost(NLMISC::CSheetId outpostSheet);
	virtual void outpostSetSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 shopSquadIndex);
	virtual void outpostSetSquadSpawnZone(NLMISC::CSheetId outpostSheet, uint32 squadSlot, uint32 spawnZoneIndex);
	virtual void outpostInsertSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot);
	virtual void outpostRemoveSquad(NLMISC::CSheetId outpostSheet, uint32 squadSlot);
	virtual void outpostSetExpenseLimit(NLMISC::CSheetId outpostSheet, uint32 expenseLimit);
	virtual void outpostSetDefensePeriod(NLMISC::CSheetId outpostSheet, uint8 hour);
};

#endif // RY_GUILD_HIGH_OFFICER_MODULE_H

/* End of guild_high_officer_module.h */
