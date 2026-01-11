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



#ifndef RY_MAGIC_FOCUS_ITEM_H
#define RY_MAGIC_FOCUS_ITEM_H

//
#include "game_share/skills.h"

class CGameItemPtr;

class CMagicFocusItemFactor
{
public:
	CMagicFocusItemFactor()
	{
		clear();
	}

	void clear()
	{
		_ElementalCastingTimeFactor = 0.0f;
		_ElementalPowerFactor = 0.0f;
		_OffensiveAfflictionCastingTimeFactor = 0.0f;
		_OffensiveAfflictionPowerFactor = 0.0f;
		_HealCastingTimeFactor = 0.0f;
		_HealPowerFactor = 0.0f;
		_DefensiveAfflictionCastingTimeFactor = 0.0f;
		_DefensiveAfflictionPowerFactor = 0.0f;
		_RequiredLevel = 0;
		_WearItem = false;
		_IsMagicFocus = false;
	}

	/// init values for given item stats
	void init(const CGameItemPtr& item);

	/// get casting time factor according to magic skill used
	float getCastingTimeFactor( SKILLS::ESkills skill, uint16 spellPower) const;

	/// get power time factor according to magic skill used
	float getPowerFactor( SKILLS::ESkills skill, uint16 spellPower) const;

	/// get required 
	inline uint16 requiredLevel() const { return _RequiredLevel; }

	inline bool wearItem() const { return _WearItem; }

	/// is item a magic focus item
	inline bool isMagicFocus() const { return _IsMagicFocus; }

private:
	static  std::string _ElementalSkill;
	static  std::string _OffensiveAfflictionSkill;
	static  std::string _DefensiveAfflictionSkill;
	static  std::string _HealSkill;
	
	// factors
	float	_ElementalCastingTimeFactor;
	float	_ElementalPowerFactor;
	float	_OffensiveAfflictionCastingTimeFactor;
	float	_OffensiveAfflictionPowerFactor;
	float	_HealCastingTimeFactor;
	float	_HealPowerFactor;
	float	_DefensiveAfflictionCastingTimeFactor;
	float	_DefensiveAfflictionPowerFactor;
	
	// item required level
	uint16	_RequiredLevel;
	
	/** 
	 * bool indicating of the item has been used and thus must wear
	 * it's a mutable value as it's changed when an accessor on factors returns a non null value
	 */
	mutable bool	_WearItem;
	
	/// is item a magic focus
	bool	_IsMagicFocus;
};

#endif // RY_MAGIC_FOCUS_ITEM_H

/* End of magic_focus_item.h */
