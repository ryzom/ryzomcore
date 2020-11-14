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


#ifndef RY_WEAPON_DAMAGE_TABLE_H
#define RY_WEAPON_DAMAGE_TABLE_H


#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"


/// max skill value (entity)
const uint16		MaxSkillValue = 250;
	/// max reference skill value (weapon)
const uint16		MaxRefenceSkillValue = 250;

/**
 * Singleton giving reference weapon damage
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CWeaponDamageTable : public NLMISC::CSingleton<CWeaponDamageTable>
{	
public:
//	CWeaponDamageTable();

	/// Destructor
	virtual ~CWeaponDamageTable() {}

	/**
	 * get reference damage for given entity skill and weapon quality (=recommended skill)
	 * \param reference the skill value recommended by the weapon
	 * \param entitySkill value of the entity's skill
	 * \return damage reference value (float)
	 */
	inline float getRefenceDamage( sint32 reference, sint32 skill )
	{
		if (reference>MaxRefenceSkillValue) reference = MaxRefenceSkillValue;
		if (skill>MaxSkillValue) skill = MaxSkillValue;

		return	_DamageTable[reference][skill];
	}

	/// init method of the class
	void init();

private:
	/// huge array giving the damage values
	float						_DamageTable[1+MaxRefenceSkillValue][1+MaxSkillValue];
};

 
#endif // RY_WEAPON_DAMAGE_TABLE_H

/* End of weapon_damage_table.h */



