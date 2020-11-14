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



//////////////
//	INCLUDE	//
//////////////
#include "stdpch.h"
//
#include "magic_focus_item.h"
#include "game_item_manager/game_item.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLMISC;

//-----------------------------------------------
void CMagicFocusItemFactor::init(const CGameItemPtr& item)
{
	_IsMagicFocus = false;

	if (item == NULL)
		return;

	_RequiredLevel = (uint16)item->recommended();
	
	_ElementalCastingTimeFactor = item->getElementalCastingTimeFactor();
	_ElementalPowerFactor = item->getElementalPowerFactor();
	if (_ElementalCastingTimeFactor != 0.0f || _ElementalPowerFactor != 0.0f)
		_IsMagicFocus = true;
	
	_OffensiveAfflictionCastingTimeFactor = item->getOffensiveAfflictionCastingTimeFactor();
	_OffensiveAfflictionPowerFactor = item->getOffensiveAfflictionPowerFactor();
	if (_OffensiveAfflictionCastingTimeFactor != 0.0f || _OffensiveAfflictionPowerFactor != 0.0f )
		_IsMagicFocus = true;
	
	_HealCastingTimeFactor = item->getHealCastingTimeFactor();
	_HealPowerFactor = item->getHealPowerFactor();
	if (_HealCastingTimeFactor != 0.0f || _HealPowerFactor != 0.0f )
		_IsMagicFocus = true;
	
	_DefensiveAfflictionCastingTimeFactor = item->getDefensiveAfflictionCastingTimeFactor();
	_DefensiveAfflictionPowerFactor = item->getDefensiveAfflictionPowerFactor();
	if (_DefensiveAfflictionCastingTimeFactor != 0.0f || _DefensiveAfflictionPowerFactor != 0.0f )
		_IsMagicFocus = true;		
}

//-----------------------------------------------
float CMagicFocusItemFactor::getCastingTimeFactor( SKILLS::ESkills skill, uint16 spellPower) const
{
	if (spellPower > _RequiredLevel)
		return 0.0f;
	
	float returnVal = 0.0f;
	
	const std::string &str = SKILLS::toString(skill);
	if ( _ElementalCastingTimeFactor && str.substr(0,4) == _ElementalSkill)
	{
		returnVal = _ElementalCastingTimeFactor;
	}
	else if ( _OffensiveAfflictionCastingTimeFactor && str.substr(0,4) == _OffensiveAfflictionSkill)
	{
		returnVal = _OffensiveAfflictionCastingTimeFactor;
	}
	else if ( _HealCastingTimeFactor && str.substr(0,4) == _HealSkill)
	{
		returnVal = _HealCastingTimeFactor;
	}
	else if ( _DefensiveAfflictionCastingTimeFactor && str.substr(0,4) == _DefensiveAfflictionSkill)
	{
		returnVal = _DefensiveAfflictionCastingTimeFactor;
	}
	
	if (returnVal > 0.0f)
		_WearItem = true;
	
	return returnVal;
}

//-----------------------------------------------
float CMagicFocusItemFactor::getPowerFactor( SKILLS::ESkills skill, uint16 spellPower) const
{
	if (spellPower > _RequiredLevel)
		return 0.0f;
	
	float returnVal = 0.0f;
	
	const std::string &str = SKILLS::toString(skill);
	if ( _ElementalPowerFactor && str.substr(0,4) == _ElementalSkill)
	{
		returnVal = _ElementalPowerFactor;
	}
	if ( _OffensiveAfflictionPowerFactor && str.substr(0,4) == _OffensiveAfflictionSkill)
	{
		returnVal = _OffensiveAfflictionPowerFactor;
	}
	if ( _HealPowerFactor && str.substr(0,4) == _HealSkill)
	{
		returnVal = _HealPowerFactor;
	}
	if ( _DefensiveAfflictionPowerFactor && str.substr(0,4) == _DefensiveAfflictionSkill)
	{
		returnVal = _DefensiveAfflictionPowerFactor;
	}
				
	if (returnVal > 0.0f)
		_WearItem = true;
	
	return returnVal;
}
