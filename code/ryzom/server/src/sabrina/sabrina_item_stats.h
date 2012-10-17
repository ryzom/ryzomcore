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



#ifndef RY_SABRINA_ITEM_STATS_H
#define RY_SABRINA_ITEM_STATS_H

// nel misc
#include "nel/misc/types_nl.h"
// game_share
#include "game_share/skills.h"
#include "game_share/damage_types.h"
#include "game_share/armor_types.h"
#include "game_share/game_item_manager/game_item.h"


/**
 * Sabrina weapon stats description class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

struct CWeaponStats
{
	// construction
	CWeaponStats();
	CWeaponStats(const CGameItemPtr &itemPtr, uint16 quality);

	// debug routines
	std::string toString() const;

	// read accessors
	uint16					getSpeedInTicks()	const { return _SpeedInTicks; }
	uint32					getDamage()			const { return _Damage; }
	uint16					getQuality()		const { return _Quality; }
	DMGTYPE::EDamageType	getDmgType()		const { return _DmgType; }
	ITEMFAMILY::EItemFamily	getFamily()			const { return _Family; }
	float					getRange()			const { return _Range; }
	SKILLS::ESkills			getSkill()			const { return _Skill; }
	sint32					getSkillValue()		const { return _SkillValue; }

	// write accessors
	void setSpeedInTicks(uint16 ticks)				{ _SpeedInTicks=ticks; }
	void setDamage(uint32 damage)					{ _Damage=damage; }
	void setQuality(uint16 quality)					{ _Quality=quality; }
	void setDmgType(DMGTYPE::EDamageType dmgType)	{ _DmgType=dmgType; }
	void setFamily(ITEMFAMILY::EItemFamily family)	{ _Family=family; }
	void setRange(float range)						{ _Range=range; }
	void setSkill(SKILLS::ESkills skill)			{ _Skill=skill; }
	void setSkillValue(sint32 value)				{ _SkillValue=value; }

private:
	uint16					_SpeedInTicks;
	uint32					_Damage;	
	uint16					_Quality;
	DMGTYPE::EDamageType	_DmgType;
	ITEMFAMILY::EItemFamily	_Family;
	float					_Range;		// if > 0 range weapon or ammo, == 0 melee weapon	
	SKILLS::ESkills			_Skill;		// no meaning for ammos
	sint32					_SkillValue; // no meaning for ammos
};


/**
 * Sabrina armor description base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CArmorStats
{
public:
	// construction
	CArmorStats();
	CArmorStats( const CStaticItem *item, uint16 quality );

	// debug routines
	std::string toString(bool overrideTitle=false) const;

	// read accessors
	uint32 getQuality()							const { return _Quality; }
	SKILLS::ESkills	getSkill()					const { return _Skill; }
	ARMORTYPE::EArmorType getArmorType()		const { return _ArmorType; }
	float getProtectionFactor(uint32 index)		const; 
	uint32 getProtectionLimit(uint32 index)		const; 

	// write accessors
	void setQuality(uint32 quality)						{ _Quality=quality; }
	void setSkill(SKILLS::ESkills skill)				{ _Skill=skill; }
	void setArmorType(ARMORTYPE::EArmorType armorType)	{ _ArmorType=armorType; }
	void setProtectionFactor(uint32 index,float val); 
	void setProtectionLimit(uint32 index,uint32 val); 

private:
	uint32					_Quality;
	SKILLS::ESkills			_Skill;
	ARMORTYPE::EArmorType	_ArmorType;

	float _ProtectionFactor[DMGTYPE::NBTYPES];
	uint32 _ProtectionLimit[DMGTYPE::NBTYPES];
};



/**
 * Sabrina shield class (derived from CArmorStats)
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CShieldStats: public CArmorStats
{
public:
	// construction
	CShieldStats(): CArmorStats(), _ShieldType(SHIELDTYPE::NONE)
	{}

	// debug routines
	std::string toString() const
	{
		return std::string()+SHIELDTYPE::toString(_ShieldType)+CArmorStats::toString(true);
	}

	// read accessors
	SHIELDTYPE::EShieldType getShieldType() const { return _ShieldType; }

	// write accessors
	void setShieldType(SHIELDTYPE::EShieldType shieldType) { _ShieldType=shieldType; }

private:
	SHIELDTYPE::EShieldType _ShieldType;
};


#endif


