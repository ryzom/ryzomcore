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



#ifndef RYZOM_PROGRESSION_COMMON_H
#define RYZOM_PROGRESSION_COMMON_H

#include "game_share/action_nature.h"
#include "game_share/skills.h"


class CCharacter;

/**
 * TReportAction
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 * \structure contained information about performed action for progression management
 */
struct TReportAction
{
	TDataSetRow					ActorRowId;		// Actor
	TDataSetRow					TargetRowId;	// Target
	ACTNATURE::TActionNature	ActionNature;	// Action nature: must be exact: determine progression algorithm used and right for perform offensive action (fight and offensive_magic)
	sint32						DeltaLvl;		// Delta level in skill point (actor skill - target skill (or sabrina cost depending action type) )
	SKILLS::ESkills				Skill;			// Skill used by actor (useful only if PJ is actor), if unknown no xp will be gained
	uint16						SkillLevel;		// if not zero, use this value as the skill level instead of the real player skill level (needed for the enchantment special case)
	float						factor;			// for gain a part of xp due to a partial success action performed [ 0... 1.2 (for critical) ]
	uint32						Hp;				// > 0 due to attack suffer (PNJ attack) or adding by curative action
	uint32						Sta;			// > 0 reduce due to attack suffer (PNJ attack) or adding by curative action
	uint32						Sap;			// > 0 reduce due to attack suffer (PNJ attack) or adding by curative action
	uint32						Focus;			// > 0 reduce due to attack suffer (PNJ attack) or adding by curative action
	
	TReportAction()
	{
		ActorRowId = TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW );
		TargetRowId = TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW );
		ActionNature = ACTNATURE::UNKNOWN;
		DeltaLvl = 0;
		Skill = SKILLS::unknown;
		SkillLevel = 0;
		factor = 1.0f;
		Hp = 0;
		Sta = 0;
		Sap = 0;
		Focus = 0;
	}
};

struct CAutoQuarterItemDescription
{
	uint16		ItemIndex;
	uint16		QuantityNeeded;
	CCharacter*	PlayerNeedingTheItem;
};


#endif // RYZOM_PROGRESSION_COMMON_H

/* End of file progression_common.h */
