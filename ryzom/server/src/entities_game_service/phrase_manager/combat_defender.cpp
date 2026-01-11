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




#include "stdpch.h"
#include "combat_defender.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "phrase_manager/s_effect.h"
#include "player_manager/player.h"
#include "player_manager/gear_latency.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CCombatDefender);

// skill used when no weapon in hand (hand to hand combat)
extern SKILLS::ESkills	BarehandCombatSkill;

static const SLOT_EQUIPMENT::TSlotEquipment Slots[6] = 
{
	SLOT_EQUIPMENT::HEAD,
	SLOT_EQUIPMENT::CHEST,
	SLOT_EQUIPMENT::ARMS,
	SLOT_EQUIPMENT::HANDS,
	SLOT_EQUIPMENT::LEGS,
	SLOT_EQUIPMENT::FEET
};

//--------------------------------------------------------------
//				CCombatDefenderPlayer::getArmor
//--------------------------------------------------------------
bool CCombatDefenderPlayer::getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const 
{ 
	if ( !_Character) 
		return false;
	
	CGameItemPtr armorPtr = _Character->getItem(INVENTORIES::equipment, slot);
	if ( _Character->getGearLatency().isSlotLatent( INVENTORIES::equipment,slot ) )
		armorPtr = NULL;
	if (armorPtr == NULL || armorPtr->getStaticForm() == NULL || armorPtr->getStaticForm()->Family != ITEMFAMILY::ARMOR)
		return false;

	armor.Quality = (uint16)armorPtr->recommended();

	armor.MaxProtection[DMGTYPE::BLUNT] = (uint16)armorPtr->maxBluntProtection();
	armor.MaxProtection[DMGTYPE::PIERCING] = (uint16)armorPtr->maxPiercingProtection();
	armor.MaxProtection[DMGTYPE::SLASHING] = (uint16)armorPtr->maxSlashingProtection();
	armor.ProtectionFactor[DMGTYPE::BLUNT] = armorPtr->protectionFactor();
	armor.ProtectionFactor[DMGTYPE::PIERCING] = armorPtr->protectionFactor();
	armor.ProtectionFactor[DMGTYPE::SLASHING] = armorPtr->protectionFactor();

	armor.ArmorType = armorPtr->getStaticForm()->Armor->ArmorType;

	return true;
} // CCombatDefenderPlayer::getArmor //
	

//--------------------------------------------------------------
//				CCombatDefenderPlayer::getShield
//--------------------------------------------------------------
bool CCombatDefenderPlayer::getShield(CCombatShield &shield) const 
{
	if ( _Character == NULL) 
		return false;

	CGameItemPtr shieldPtr = _Character->getLeftHandItem();
	if ( _Character->getGearLatency().isLatent() )
		shieldPtr = NULL;
	if (shieldPtr == NULL || shieldPtr->getStaticForm() == NULL || shieldPtr->getStaticForm()->Family != ITEMFAMILY::SHIELD )
		return false;

	shield.Quality = (uint16)shieldPtr->recommended();
	shield.MaxProtection[DMGTYPE::BLUNT] = (uint16)shieldPtr->maxBluntProtection();
	shield.MaxProtection[DMGTYPE::PIERCING] = (uint16)shieldPtr->maxPiercingProtection();
	shield.MaxProtection[DMGTYPE::SLASHING] = (uint16)shieldPtr->maxSlashingProtection();
	shield.ProtectionFactor[DMGTYPE::BLUNT] = shieldPtr->protectionFactor();
	shield.ProtectionFactor[DMGTYPE::PIERCING] = shieldPtr->protectionFactor();
	shield.ProtectionFactor[DMGTYPE::SLASHING] = shieldPtr->protectionFactor();

	shield.ShieldType = shieldPtr->getStaticForm()->Shield->ShieldType;
	shield.ArmorType = shieldPtr->getStaticForm()->Shield->ArmorType;

	return true;
} // CCombatDefenderPlayer::getShield //


//--------------------------------------------------------------
//				CCombatDefenderPlayer::getDefenseValue
//--------------------------------------------------------------
sint32 CCombatDefenderPlayer::getDefenseValue(EGSPD::CPeople::TPeople attackerRace) const
{
	if ( !_Character) 
		return 0;
	
	if (_Character->dodgeAsDefense())
	{
		// lookup for effects on defense
		sint32 modifier = 0;

		// these modifiers are already managed as dodge modifier in CCharacter, see simple_effect.cpp
/*		const CSEffect* effect = _Character->lookForActiveEffect( EFFECT_FAMILIES::CombatDefenseModifier );
		if ( effect )
			modifier = effect->getParamValue();

		effect = _Character->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffDodge );
		if ( effect )
			modifier += effect->getParamValue(); // value must be < 0 on a debuff
*/
		modifier += _Character->getSkillModifierForRace(attackerRace);
		modifier += _Character->dodgeSuccessModifier();

		return (_Character->getCurrentDodgeLevel() + modifier);
	}
	else
	{
		// lookup for effects on defense
		sint32 modifier = 0;
		
		// these modifiers are already managed as parry modifier in CCharacter, see simple_effect.cpp
/*		const CSEffect *effect = _Character->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMelee );
		if ( effect )
			modifier = effect->getParamValue(); // value must be < 0 on a debuff

		effect = _Character->lookForActiveEffect( EFFECT_FAMILIES::CombatDefenseModifier );
		if ( effect )
			modifier += effect->getParamValue(); // value must be < 0 on a debuff

		effect = _Character->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
		if ( effect )
			modifier += effect->getParamValue(); // value must be < 0 on a debuff
*/

		const CSEffect *effect = _Character->lookForActiveEffect( EFFECT_FAMILIES::OutpostCombat );
		if ( effect )
			modifier += effect->getParamValue();
		
		modifier += _Character->getSkillModifierForRace(attackerRace);
		modifier += _Character->parrySuccessModifier();

		return (_Character->getCurrentParryLevel() + modifier);

/*		// get right weapon skill
		CGameItemPtr item = _Character->getRightHandItem();
		if (item != NULL && item->getStaticForm() != NULL && item->getStaticForm()->Family == ITEMFAMILY::MELEE_WEAPON && item->getStaticForm()->Skill < SKILLS::NUM_SKILLS)
		{
			return ( _Character->getSkillValue(item->getStaticForm()->Skill) + modifier);
		}
		else
		{
			/// TODO : get highest barehand combat skill
			return ( _Character->getSkillValue(BarehandCombatSkill) + modifier);
		}
*/
	}
} // getDefenseValue //

//--------------------------------------------------------------
//				CCombatDefenderPlayer::getBaseDefenseValue
//--------------------------------------------------------------
sint32 CCombatDefenderPlayer::getBaseDefenseValue() const
{
	if ( _Character == NULL ) 
		return SKILLS::unknown;

	if (_Character->dodgeAsDefense())
	{
		return _Character->getBaseDodgeLevel();
	}
	else
	{
		return _Character->getBaseParryLevel();
	}
} // getBaseDefenseValue //


//--------------------------------------------------------------
//				CCombatDefenderPlayer::getLeastProtectedSlot
//--------------------------------------------------------------
SLOT_EQUIPMENT::TSlotEquipment CCombatDefenderPlayer::getLeastProtectedSlot(DMGTYPE::EDamageType dmgType) const
{
	if (dmgType>DMGTYPE::BLUNT)
		return Slots[0];

	if ( !_Character) 
		return Slots[0];
	
	SLOT_EQUIPMENT::TSlotEquipment slot = Slots[0];
	float minProtection = 100.0f;

	for (uint i = 0 ; i < 6 ; ++i)
	{
		float protection = 0.0f;
		CGameItemPtr armorPtr = _Character->getItem(INVENTORIES::equipment, Slots[i]);
		if (armorPtr != NULL)
		{
			protection = armorPtr->protectionFactor();
		}

		if (protection<minProtection)
		{
			minProtection = protection;
			slot = Slots[i];
		}
	}	
	
	return slot;
} // getLeastProtectedSlot //

//--------------------------------------------------------------
//				CCombatDefenderPlayer::getMostProtectedSlot
//--------------------------------------------------------------
SLOT_EQUIPMENT::TSlotEquipment CCombatDefenderPlayer::getMostProtectedSlot(DMGTYPE::EDamageType dmgType) const
{
	if (dmgType>DMGTYPE::BLUNT)
		return Slots[0];
	
	if ( !_Character) 
		return Slots[0];
		
	SLOT_EQUIPMENT::TSlotEquipment slot = Slots[0];
	float maxProtection = 0.0f;
	
	for (uint i = 0 ; i < 6 ; ++i)
	{
		float protection = 0.0f;
		CGameItemPtr armorPtr = _Character->getItem(INVENTORIES::equipment, Slots[i]);
		if (armorPtr != NULL)
		{
			protection = armorPtr->protectionFactor();
		}
		
		if (protection>maxProtection)
		{
			maxProtection = protection;
			slot = Slots[i];
		}
	}	
	
	return slot;
} // getMostProtectedSlot //


//--------------------------------------------------------------
//				CCombatDefenderPlayer::getAveragestProtectedSlot
//--------------------------------------------------------------
SLOT_EQUIPMENT::TSlotEquipment CCombatDefenderPlayer::getAveragestProtectedSlot(DMGTYPE::EDamageType dmgType) const
{
	if (dmgType>DMGTYPE::BLUNT)
		return Slots[0];
	
	if ( !_Character) 
		return Slots[0];
		
	float meanProtection = 0.0f;
	for (uint i = 0 ; i < 6 ; ++i)
	{
		CGameItemPtr armorPtr = _Character->getItem(INVENTORIES::equipment, Slots[i]);
		if (armorPtr != NULL)
		{
			meanProtection += armorPtr->protectionFactor();
		}
	}

	meanProtection /= 6.0f;

	// get the closest armored slot
	SLOT_EQUIPMENT::TSlotEquipment slot = Slots[0];
	double minDelta = 1000.0f;
	for (uint i = 0 ; i < 6 ; ++i)
	{
		float delta = meanProtection;
		CGameItemPtr armorPtr = _Character->getItem(INVENTORIES::equipment, Slots[i]);
		if (armorPtr != NULL)
		{
			delta -= armorPtr->protectionFactor();
		}
		if (fabs(delta) < minDelta)
		{
			minDelta = fabs(delta);
			slot = Slots[i];
		}
	}
	
	return slot;
} // getAveragestProtectedSlot //


//--------------------------------------------------------------
//					CCombatDefenderAI constructor
//--------------------------------------------------------------
CCombatDefenderAI::CCombatDefenderAI(const TDataSetRow &rowId) : CCombatDefender(rowId), _CreatureForm(0)
{
	CEntityBase * entity = getEntity();
	if(!entity) 
		return;

	/// TODO : specialize this class for NPCs other than creature

	_CreatureForm  = entity->getForm();
	if (!_CreatureForm)
	{
		nlwarning("Failed to get form for entity sheet %s", entity->getType().toString().c_str());
		return;
	}

	for (uint i = 0 ; i < DMGTYPE::NBTYPES ; ++i)
	{
		_GlobalArmor.MaxProtection[i] = _CreatureForm->getProtections()[i].Max;
		_GlobalArmor.ProtectionFactor[i] = _CreatureForm->getProtections()[i].Factor;
	}

	_GlobalArmor.Quality = _CreatureForm->getDefenseLevel();
	_GlobalArmor.ArmorType = ARMORTYPE::UNKNOWN;

//	nldebug("Creature %s, armor : %s",_CreatureForm->Name.c_str(), _GlobalArmor.toString().c_str());
	
	_DefenseValue = _CreatureForm->getDefenseLevel();

} // CCombatDefenderAI constructor //


//--------------------------------------------------------------
//					CCombatDefenderAI::getArmor
//--------------------------------------------------------------
bool CCombatDefenderAI::getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const
{
	if ( _GlobalArmor.Quality > 0)
		armor = _GlobalArmor;
	else
		return false;
	
	return true;
} // CCombatDefenderAI::getArmor //


//--------------------------------------------------------------
//					CCombatDefenderAI::getShield
//--------------------------------------------------------------
bool CCombatDefenderAI::getShield(CCombatShield &shield) const
{
	/// todo : manage shield for npcs (creature never have shield)
	return false;
} // CCombatDefenderAI::getShield //


//--------------------------------------------------------------
//					CCombatDefenderAI::getDefenseValue
//--------------------------------------------------------------
sint32 CCombatDefenderAI::getDefenseValue(EGSPD::CPeople::TPeople) const
{ 
	sint32 modifier = 0;
	CEntityBase *entity = getEntity();
	if (entity)
	{			
		// do not use smart pointers for local use only
		const CSEffect *effect;
		if ( !entity->dodgeAsDefense())
		{
			effect = entity->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMelee );
			if ( effect )
				modifier += effect->getParamValue(); // value must be < 0 on a debuff

			effect = entity->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffCombatSkills );
			if ( effect )
				modifier += effect->getParamValue(); // value must be < 0 on a debuff
		}
		else
		{
			effect = entity->lookForActiveEffect( EFFECT_FAMILIES::CombatDebuffDodge );
			if ( effect )
				modifier += effect->getParamValue(); // value must be < 0 on a debuff
		}

		effect = entity->lookForActiveEffect( EFFECT_FAMILIES::CombatDefenseModifier );
		if ( effect )
			modifier += effect->getParamValue(); // value must be < 0 on a debuff
	}

	return (_DefenseValue + modifier);
} // getDefenseValue //
