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

// game_share
#include "game_share/damage_types.h"
#include "game_share/armor_types.h"

//
#include "game_item_manager/game_item.h"
#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "entity_manager/entity_manager.h"

extern CPlayerManager PlayerManager;

struct CCombatArmor
{
	CCombatArmor() { init(); }

	void init()
	{
		Quality = 0;
		for (uint i = 0 ; i < DMGTYPE::NBTYPES ; ++i)
		{
			MaxProtection[i] = 0;
			ProtectionFactor[i] = 0.0f;
		}
	
		ArmorType = ARMORTYPE::UNKNOWN;
	}

	std::string toString() const
	{
		const std::string temp = NLMISC::toString("Quality= %u, MaxB =%u,FactorB =%f, MaxP=%u,FactorP=%f, MaxS=%u, FactorS=%f", 
													Quality, MaxProtection[DMGTYPE::BLUNT], ProtectionFactor[DMGTYPE::BLUNT], 
													MaxProtection[DMGTYPE::PIERCING], ProtectionFactor[DMGTYPE::PIERCING], 
													MaxProtection[DMGTYPE::SLASHING], ProtectionFactor[DMGTYPE::SLASHING]
													);
		return temp;
	}

	uint16			Quality;

	uint16			MaxProtection[DMGTYPE::NBTYPES];
	float			ProtectionFactor[DMGTYPE::NBTYPES];

	ARMORTYPE::EArmorType ArmorType;
};

struct CCombatShield : public CCombatArmor
{
	CCombatShield() : CCombatArmor()
	{
		ShieldType = SHIELDTYPE::NONE;
	};

	SHIELDTYPE::EShieldType ShieldType;
	//CGameItemRefPtr			ItemRefPtr;
};


/**
 * Base class for combat defenders
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatDefender : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CCombatDefender);
public:

	CCombatDefender(const TDataSetRow &rowId)
	{
		_TargetIndex = 0;
		if ( !TheDataset.isAccessible(rowId))
		{
			nlwarning("<CCombatDefender> ERROR Get an invalid row id as param for the constructor, should never happens");
			nlstop;
			return;
		}

		_RowId = rowId;
	}

	virtual ~CCombatDefender()
	{}

	inline bool isValid() const
	{
		return (TheDataset.isAccessible(_RowId));
	}

	virtual sint32 getSkillValue( SKILLS::ESkills skill, EGSPD::CPeople::TPeople attackerRace) const = 0;
	virtual sint32 getSkillBaseValue( SKILLS::ESkills skill) const = 0;

	inline CEntityBase *getEntity() const { return CEntityBaseManager::getEntityBasePtr(_RowId); }
	inline const TDataSetRow &getEntityRowId() const { return _RowId; }

	virtual bool getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const = 0;
	virtual bool getShield(CCombatShield &shield) const = 0;

	virtual void damageOnShield(uint32 dmg) = 0;
	virtual void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg) = 0;

	virtual sint32 getDefenseValue(EGSPD::CPeople::TPeople attackerRace) const = 0;
	virtual sint32 getBaseDefenseValue() const = 0;

	virtual SLOT_EQUIPMENT::TSlotEquipment getLeastProtectedSlot(DMGTYPE::EDamageType dmgType) const = 0;
	virtual SLOT_EQUIPMENT::TSlotEquipment getMostProtectedSlot(DMGTYPE::EDamageType dmgType) const = 0;
	virtual SLOT_EQUIPMENT::TSlotEquipment getAveragestProtectedSlot(DMGTYPE::EDamageType dmgType) const = 0;

	inline uint getTargetIndex() { return _TargetIndex;} const
	inline void setTargetIndex( uint idx ) { _TargetIndex = idx; }

//	virtual sint32 dodgeModifier() const { return 0; }
//	virtual sint32 parryModifier() const { return 0; }

protected:
	/// row id
	TDataSetRow		_RowId;
	// index of the defender in the target list in case of multi target attack
	uint			_TargetIndex;
};

typedef NLMISC::CSmartPtr<CCombatDefender> CCombatDefenderPtr;

/**
 * Base class for combat player defenders
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatDefenderPlayer : public CCombatDefender
{
public:
	explicit CCombatDefenderPlayer(const TDataSetRow &rowId) : CCombatDefender(rowId)
	{
		_Character = PlayerManager.getChar(_RowId);
	}

	virtual ~CCombatDefenderPlayer()
	{}

	inline sint32 getSkillValue( SKILLS::ESkills skill, EGSPD::CPeople::TPeople attackerRace) const
	{
		if (!_Character || skill >= SKILLS::NUM_SKILLS || skill<0) return 0;
		// add skill mod according to target race
		return _Character->getSkillValue(skill) + _Character->getSkillModifierForRace(attackerRace);
	}

	inline sint32 getSkillBaseValue( SKILLS::ESkills skill) const
	{
		if (!_Character || skill >= SKILLS::NUM_SKILLS || skill<0) return 0;
		return _Character->getSkillBaseValue(skill);
	}

	virtual bool getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const ;
	
	virtual bool getShield(CCombatShield &shield) const;

	virtual sint32 getDefenseValue(EGSPD::CPeople::TPeople attackerRace) const;
	virtual sint32 getBaseDefenseValue() const;

	virtual void damageOnShield(uint32 dmg)
	{
		if ( !_Character) return;

		CGameItemPtr shieldPtr = _Character->getLeftHandItem();
		if (shieldPtr == NULL || shieldPtr->getStaticForm() == NULL || shieldPtr->getStaticForm()->Family != ITEMFAMILY::SHIELD )
			return;
		
		shieldPtr->removeHp(dmg);
	}

	virtual void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg)
	{
		if ( !_Character) return;

		CGameItemPtr armorPtr = _Character->getItem(INVENTORIES::equipment, slot);
		if (armorPtr == NULL || armorPtr->getStaticForm() == NULL || armorPtr->getStaticForm()->Family != ITEMFAMILY::ARMOR)
			return;
		
		armorPtr->removeHp(dmg);
	}

	virtual SLOT_EQUIPMENT::TSlotEquipment getLeastProtectedSlot(DMGTYPE::EDamageType dmgType) const;
	virtual SLOT_EQUIPMENT::TSlotEquipment getMostProtectedSlot(DMGTYPE::EDamageType dmgType) const;
	virtual SLOT_EQUIPMENT::TSlotEquipment getAveragestProtectedSlot(DMGTYPE::EDamageType dmgType) const;

/*	inline sint32 dodgeModifier() const
	{
		if (_Character)
			return _Character->dodgeModifier();
		else
			return 0;
	}
	
	inline sint32 parryModifier() const
	{
		if (_Character)
			return _Character->parryModifier();
		else
			return 0;
	}
*/
protected:
	/// keep a ref pointer on original character
	CCharacterRefPtr	_Character;
	
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
	explicit CCombatDefenderAI(const TDataSetRow &rowId);

	virtual ~CCombatDefenderAI()
	{}

	inline sint32 getSkillValue( SKILLS::ESkills skill, EGSPD::CPeople::TPeople ) const { return _DefenseValue; }
	inline sint32 getSkillBaseValue( SKILLS::ESkills skill) const { return _DefenseValue; }
	
	virtual sint32 getDefenseValue(EGSPD::CPeople::TPeople) const;
	virtual sint32 getBaseDefenseValue() const { return _DefenseValue; }

	virtual bool getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const;

	virtual bool getShield(CCombatShield &shield) const;

	inline void damageOnShield(uint32 dmg) {}
	inline void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg) {}

	virtual SLOT_EQUIPMENT::TSlotEquipment getLeastProtectedSlot(DMGTYPE::EDamageType dmgType) const
	{ return SLOT_EQUIPMENT::HEAD; }
	
	virtual SLOT_EQUIPMENT::TSlotEquipment getMostProtectedSlot(DMGTYPE::EDamageType dmgType) const
	{ return SLOT_EQUIPMENT::CHEST;	}

	virtual SLOT_EQUIPMENT::TSlotEquipment getAveragestProtectedSlot(DMGTYPE::EDamageType dmgType) const
	{ return SLOT_EQUIPMENT::CHEST;	}

protected:
	/// creature form
	const CStaticCreatures*	_CreatureForm;

	/// defense value (parry or dodge)
	sint32			_DefenseValue;

	/// Global armor
	CCombatShield	_GlobalArmor;
};

#endif // RY_COMBAT_DEFENDER_H

/* End of combat_defender.h */
