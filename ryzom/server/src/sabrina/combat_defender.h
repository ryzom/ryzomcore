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



#ifndef RY_COMBAT_DEFENDER_H
#define RY_COMBAT_DEFENDER_H

// misc
#include "nel/misc/types_nl.h"
// game_share
#include "game_share/skills.h"
#include "game_share/damage_types.h"
#include "game_share/armor_types.h"
#include "game_share/game_item_manager/game_item.h"

//
#include "entity_base.h"
#include "character.h"
#include "player_manager.h"
#include "entity_manager.h"

extern CPlayerManager PlayerManager;

struct CCombatArmor
{
	CCombatArmor()
	{
		Quality = 0;
		MaxBluntProtection = 0;
		MaxPiercingProtection = 0;
		MaxSlashingProtection = 0;
		BluntProtectionFactor = 0.0f;
		PiercingProtectionFactor = 0.0f;
		SlashingProtectionFactor = 0.0f;
		
		MaxRotProtection = 0;
		RotProtectionFactor = 0.0f;
		MaxAcidProtection = 0;
		AcidProtectionFactor = 0.0f;
		MaxColdProtection = 0;
		ColdProtectionFactor = 0.0f;
		MaxFireProtection = 0;
		FireProtectionFactor = 0.0f;
		MaxPoisonProtection = 0;
		PoisonProtectionFactor = 0.0f;
		MaxElectricityProtection = 0;
		ElectricityProtectionFactor = 0.0f;
		MaxShockProtection = 0;
		ShockProtectionFactor = 0.0f;

		Skill = SKILLS::unknown;
		SkillValue = 0;
		ArmorType = ARMORTYPE::UNKNOWN;
	}

	CCombatArmor( const CStaticItem *item, uint16 quality);

	std::string toString() const
	{
		const std::string temp = NLMISC::toString("Quality= %u, SkillValue= %u, MaxB =%u,FactorB =%f, MaxP=%u,FactorP=%f, MaxS=%u, FactorS=%f", Quality, SkillValue, MaxBluntProtection, BluntProtectionFactor, MaxPiercingProtection, PiercingProtectionFactor, MaxSlashingProtection, SlashingProtectionFactor);
		return temp;
	}

	uint16			Quality;
	uint16			MaxBluntProtection;
	uint16			MaxPiercingProtection;
	uint16			MaxSlashingProtection;
	float			BluntProtectionFactor;
	float			PiercingProtectionFactor;
	float			SlashingProtectionFactor;

	uint16			MaxRotProtection;
	float			RotProtectionFactor;
	uint16			MaxAcidProtection;
	float			AcidProtectionFactor;
	uint16			MaxColdProtection;
	float			ColdProtectionFactor;
	uint16			MaxFireProtection;
	float			FireProtectionFactor;
	uint16			MaxPoisonProtection;
	float			PoisonProtectionFactor;
	uint16			MaxElectricityProtection;
	float			ElectricityProtectionFactor;
	uint16			MaxShockProtection;
	float			ShockProtectionFactor;

	SKILLS::ESkills	Skill;
	sint32			SkillValue;
	ARMORTYPE::EArmorType ArmorType;
};

struct CCombatShield : public CCombatArmor
{
	CCombatShield() : CCombatArmor()
	{
		ShieldType = SHIELDTYPE::NONE;
	};

	SHIELDTYPE::EShieldType ShieldType;
};


/**
 * Base class for combat defenders
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatDefender
{
public:
	CCombatDefender() {}

	CCombatDefender(const TDataSetRow &rowId) : _RowId(rowId)
	{
	}

	virtual ~CCombatDefender()
	{}

	virtual sint32 getSkillValue( SKILLS::ESkills skill) const = 0;

	inline CEntityBase *getEntity() { return CEntityBaseManager::getEntityBasePtr(_RowId); }
	inline const TDataSetRow &getEntityRowId() const { return _RowId; }

	virtual bool getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const = 0;
	virtual bool getShield(CCombatShield &shield) const = 0;

	virtual void damageOnShield(uint32 dmg) = 0;
	virtual void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg) = 0;

	virtual sint32 getDefenseValue() = 0;

public:
	/// row id
	TDataSetRow		_RowId;
};


/**
 * Base class for combat player defenders
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatDefenderPlayer : public CCombatDefender
{
public:
	CCombatDefenderPlayer()
	{}

	CCombatDefenderPlayer(const TDataSetRow &rowId) : CCombatDefender(rowId)
	{}

	virtual ~CCombatDefenderPlayer()
	{}

	inline sint32 getSkillValue( SKILLS::ESkills skill) const
	{
		CCharacter *character = PlayerManager.getChar(_RowId);
		if (!character || skill >= SKILLS::NUM_SKILLS) return 0;
		return character->getSkills()._Skills[ skill ].Current;
	}

	virtual bool getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const ;
	
	virtual bool getShield(CCombatShield &shield) const;

	virtual sint32 getDefenseValue()
	{
		CCharacter *character = PlayerManager.getChar(_RowId);
		if ( !character) 
		{
			return 0;
		}
		
		if (character->dodgeAsDefense())
		{
			return character->getSkills()._Skills[ SKILLS::SDD ].Current;
		}
		else
		{
			// get right weapon skill
			CGameItemPtr item = character->getRightHandItem();
			if (item != NULL && item->getStaticForm() != NULL && item->getStaticForm()->Family == ITEMFAMILY::MELEE_WEAPON && item->getStaticForm()->Skill < SKILLS::NUM_SKILLS)
				return character->getSkills()._Skills[ item->getStaticForm()->Skill ].Current;
			else
//				return character->getSkills()._Skills[ SKILLS::BareHandCombat ].Current; TODO; skill missing
				return 0;
		}
	}

	virtual void damageOnShield(uint32 dmg)
	{
		CCharacter *character = PlayerManager.getChar(_RowId);
		if ( !character) 
		{
			return;
		}

		CGameItemPtr shieldPtr = character->getLeftHandItem();
		if (shieldPtr == NULL || shieldPtr->getStaticForm() == NULL || shieldPtr->getStaticForm()->Family != ITEMFAMILY::SHIELD )
			return;
		
		shieldPtr->removeHp(dmg);
	}

	virtual void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg)
	{
		CCharacter *character = PlayerManager.getChar(_RowId);
		if ( !character) 
		{
			return;
		}

		CGameItemPtr armorPtr = character->getItem(INVENTORIES::equipment, slot);
		if (armorPtr == NULL || armorPtr->getStaticForm() == NULL || armorPtr->getStaticForm()->Family != ITEMFAMILY::ARMOR)
			return;
		
		armorPtr->removeHp(dmg);
	}
};


/**
 * Base class for combat AI defenders
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatDefenderAI : public CCombatDefender
{
public:
	CCombatDefenderAI() : CreatureForm(0)
	{}

	CCombatDefenderAI(const TDataSetRow &rowId);

	virtual ~CCombatDefenderAI()
	{}

	inline sint32 getSkillValue( SKILLS::ESkills skill) const { return _DefenseValue; }
	inline sint32 getDefenseValue() { return _DefenseValue; }

	virtual bool getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const;

	virtual bool getShield(CCombatShield &shield) const;

	inline void damageOnShield(uint32 dmg) {}
	inline void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg) {}

	std::pair<const CStaticItem *, uint16> getArmorOnSlot( SLOT_EQUIPMENT::TSlotEquipment slot );

public:
	/// creature form
	const CStaticCreatures *CreatureForm;

	/// defense value (parry or dodge)
	sint32			_DefenseValue;

	/// Global armor
	CCombatShield	_GlobalArmor;

	/*
	/// shield
	CCombatShield	_Shield;

	/// head armor
	CCombatArmor	_HeadArmor;
	/// legs armor
	CCombatArmor	_LegsArmor;
	/// arms armor
	CCombatArmor	_ArmsArmor;
	/// chest armor
	CCombatArmor	_ChestArmor;
	/// feet armor
	CCombatArmor	_FeetArmor;
	/// hands armor
	CCombatArmor	_HandsArmor;
	*/
};

#endif // RY_COMBAT_DEFENDER_H

/* End of combat_defender.h */
