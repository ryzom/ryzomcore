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

// precompiled headers must be first
#include "stdpch.h"

// NeL MISC
#include "nel/misc/variable.h"

// Local
#include "death_penalties.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "player_manager/item_service_manager.h"
#include "server_share/r2_variables.h"

NL_INSTANCE_COUNTER_IMPL(CDeathPenalties);

NLMISC::CVariable<bool> DissableDPInRing("egs", "DissableDPInRing", "Enable or dissable DP system in the Ring (should be dissabled unless this provokes bugs", true,0,true);

void CDeathPenalties::updataDb(CCharacter& user)
{
	if ( _DeathXPToGain == 0 )
//		user._PropertyDatabase.setProp( "USER:DEATH_XP_MALUS", 255 );
		CBankAccessor_PLR::getUSER().setDEATH_XP_MALUS(user._PropertyDatabase, 255 );
	else
//		user._PropertyDatabase.setProp( "USER:DEATH_XP_MALUS", uint8( 254 * (1 - _CurrentDeathXP / _DeathXPToGain ) ) );
		CBankAccessor_PLR::getUSER().setDEATH_XP_MALUS(user._PropertyDatabase, checkedCast<uint8>( 254 * (1 - _CurrentDeathXP / _DeathXPToGain ) ) );
}

void CDeathPenalties::addDeath(CCharacter& user, float deathPenaltyFactor )
{
	// we don't add death penalty in the ring
	if (IsRingShard && DissableDPInRing)
	{
		return;
	}

	// note that NB death is no more useful, but we keep it in serial until we have a better serial system
	SKILLS::ESkills expSkill;
	double maxXP = user.getSkills().getMaxXPToGain(expSkill);
	_NbDeath++;

	const double maxMalus = maxXP * user.getSkillBaseValue(expSkill);
	double xpMalus = ((double)deathPenaltyFactor) * DeathXPFactor * maxMalus;
	
	if ( xpMalus + _DeathXPToGain <= maxMalus )
		_DeathXPToGain += xpMalus;
	else
	{
		// cap death xp gain
		xpMalus -= (maxMalus - _DeathXPToGain);
		_DeathXPToGain = maxMalus;

		// remove penalty surplus from already paid back Xp debt
		if ( _CurrentDeathXP >= xpMalus )
			_CurrentDeathXP -= xpMalus;
		else
			_CurrentDeathXP = 0.0;
	}
	updataDb(user);
	
	if( _CurrentDeathXP || _DeathXPToGain )
	{
		SM_STATIC_PARAMS_2(params,STRING_MANAGER::integer,STRING_MANAGER::integer);
		params[0].Int = sint32(10*_CurrentDeathXP);
		params[1].Int = sint32(10*_DeathXPToGain);
		CCharacter::sendDynamicSystemMessage( user.getId(),"DEATH_XP_DEATH",params);
	}
}

uint32 CDeathPenalties::updateResorption( CCharacter& user )
{
	// we don't add death penalty in the ring
	if (IsRingShard && DissableDPInRing)
	{
		return 1800;
	}

	uint32 currentTime = NLMISC::CTime::getSecondsSince1970();
	if ( _NbDeath && _BonusUpdateTime!=0 && _BonusUpdateTime<currentTime )
	{
		SKILLS::ESkills expSkill;
		double maxXP = user.getSkills().getMaxXPToGain(expSkill);
		
		double timeFactor = (double)(currentTime - _BonusUpdateTime) / (60.*60.*24. * user.getDPLossDuration());
		
		const double maxMalus = maxXP * user.getSkillBaseValue(expSkill);
		double xpBonus = ((double)timeFactor) * maxMalus;
		
		addXP(user, SKILLS::unknown, xpBonus, xpBonus);
	}
	_BonusUpdateTime = currentTime;
	if ( _NbDeath )
		return 600;
	else
		return 1800;
}

void CDeathPenalties::addXP( CCharacter& user, SKILLS::ESkills usedSkill, double & xp )
{
	// we don't add death penalty in the ring
	if (IsRingShard && DissableDPInRing)
	{
		return;
	}

	if ( _NbDeath )
	{
		// we multiply gained xp by used skill level
		const double skillBaseValue = user.getSkillBaseValue(usedSkill);
		if (skillBaseValue <= 0)
		{
			nlwarning("Skill %s base value for char %s is <= 0 !!",SKILLS::toString(usedSkill).c_str(), user.getId().toString().c_str());
			xp = 0;
			return;
		}

		double xpBeforeDeathPenalty = xp;
		double xpUsedForDeathPenalty = xp * _DeathPenaltyFactor;
		xp -= xpUsedForDeathPenalty;

		double tempXp = xpUsedForDeathPenalty * skillBaseValue;
		addXP(user, usedSkill, tempXp, xpBeforeDeathPenalty);
		xpUsedForDeathPenalty = tempXp / skillBaseValue;

		xp += xpUsedForDeathPenalty;
	}
}

void CDeathPenalties::addXP( CCharacter& user, SKILLS::ESkills usedSkill, double & xp, double xpRaw )
{
	// we don't add death penalty in the ring
	if (IsRingShard && DissableDPInRing)
	{
		return;
	}

	_CurrentDeathXP += xp;
	
	if ( _CurrentDeathXP >= _DeathXPToGain )
	{
		// no more death penalties, only keep the xp surplus
		xp = _CurrentDeathXP - _DeathXPToGain;

		reset(user);
		
		if( _CurrentDeathXP || _DeathXPToGain )
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage(user.getEntityRowId(), "PROGRESS_DEATH_PENALTY_COMPLETE");
		}

		// consume SpeedUpDPLoss services
		while (1)
		{
			const CStaticItem * form = CItemServiceManager::getInstance()->removePersistentService(ITEM_SERVICE_TYPE::SpeedUpDPLoss, &user);
			if (form == NULL)
				break;

			SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
			params[0].SheetId = form->SheetId;
			PHRASE_UTILITIES::sendDynamicSystemMessage(user.getEntityRowId(), "ITEM_SERVICE_CONSUMED", params);
		}
	}
	else
	{
		// Don't send message to user if the xp was penalty resorption
		if (usedSkill!=SKILLS::unknown)
		{
			SM_STATIC_PARAMS_4(params, STRING_MANAGER::integer, STRING_MANAGER::skill, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].Int = sint32(100*xpRaw);
			params[1].Enum = usedSkill;
			params[2].Int = sint32(10*xp);
			params[3].Int = sint32(10*(_DeathXPToGain-_CurrentDeathXP) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(user.getEntityRowId(), "PROGRESS_DEATH_PENALTY_PAYBACK", params);
		}
		
		xp = 0.0;
		updataDb(user);
	}
}

CDeathPenaltiesTimerEvent::CDeathPenaltiesTimerEvent(CCharacter *parent)
{
	_Parent = parent;
}

void CDeathPenaltiesTimerEvent::timerCallback(CTimer* owner)
{
	H_AUTO(CDeathPenaltiesTimerEvent);
	
	uint32 nextUpdate = _Parent->updateDeathPenaltyResorption();
	owner->setRemaining(nextUpdate, this);
}
