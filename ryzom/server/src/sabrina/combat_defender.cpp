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
#include "phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;



//--------------------------------------------------------------
//					CCombatArmor constructor
//--------------------------------------------------------------
CCombatArmor::CCombatArmor( const CStaticItem *item, uint16 quality)
{
	if (!item || !item->Armor) return;

	Quality = quality;
	ArmorType = item->Armor->ArmorType;

	pair<uint8, uint16> armorProtec = PHRASE_UTILITIES::getArmorCharacteristics( item->Armor->ArmorType, quality);
	MaxBluntProtection = armorProtec.second;
	MaxPiercingProtection = armorProtec.second;
	MaxSlashingProtection = armorProtec.second;
	BluntProtectionFactor = armorProtec.first;
	PiercingProtectionFactor = armorProtec.first;
	SlashingProtectionFactor = armorProtec.first;	
	Skill = item->Skill;
	SkillValue = Quality * 10;
} // CCombatArmor //



//--------------------------------------------------------------
//				CCombatDefenderPlayer::getArmor
//--------------------------------------------------------------
bool CCombatDefenderPlayer::getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const 
{ 
	CCharacter *character = PlayerManager.getChar(_RowId);
	if ( !character) 
	{
		return false;
	}

	CGameItemPtr armorPtr = character->getItem(INVENTORIES::equipment, slot);
	if (armorPtr == NULL || armorPtr->getStaticForm() == NULL || armorPtr->getStaticForm()->Family != ITEMFAMILY::ARMOR)
		return false;

	armor.Quality = armorPtr->quality();
	armor.Skill = armorPtr->getStaticForm()->Skill;

	if( armor.Skill < 0 ) { nlwarning("<CCombatDefenderPlayer::getArmor> armor.Skill negative !!!!"); armor.SkillValue = 0; }
	else
	if ( armor.Skill < SKILLS::NUM_SKILLS )
	{
		armor.SkillValue = character->getSkills()._Skills[ armor.Skill ].Current;
	}
	else
		armor.SkillValue = 0;

	armor.MaxBluntProtection = armorPtr->getProtection(DMGTYPE::BLUNT).Max;
	armor.MaxPiercingProtection = armorPtr->getProtection(DMGTYPE::PIERCING).Max;
	armor.MaxSlashingProtection = armorPtr->getProtection(DMGTYPE::SLASHING).Max;
	armor.BluntProtectionFactor = armorPtr->getProtection(DMGTYPE::BLUNT).Factor;
	armor.PiercingProtectionFactor = armorPtr->getProtection(DMGTYPE::PIERCING).Factor;
	armor.SlashingProtectionFactor = armorPtr->getProtection(DMGTYPE::BLUNT).Factor;
	armor.ArmorType = armorPtr->getStaticForm()->Armor->ArmorType;

	return true;
} // CCombatDefenderPlayer::getArmor //
	

//--------------------------------------------------------------
//				CCombatDefenderPlayer::getShield
//--------------------------------------------------------------
bool CCombatDefenderPlayer::getShield(CCombatShield &shield) const 
{
	CCharacter *character = PlayerManager.getChar(_RowId);
	if ( !character) 
	{
		return false;
	}
	CGameItemPtr shieldPtr = character->getLeftHandItem();
	if (shieldPtr == NULL || shieldPtr->getStaticForm() == NULL || shieldPtr->getStaticForm()->Family != ITEMFAMILY::SHIELD )
		return false;

	shield.Quality = shieldPtr->quality();
	shield.Skill = shieldPtr->getStaticForm()->Skill;

	if ( shield.Skill < SKILLS::NUM_SKILLS )
	{
		shield.SkillValue = character->getSkills()._Skills[ shield.Skill ].Current;
	}
	else
		shield.SkillValue = 0;

	shield.MaxBluntProtection = shieldPtr->getProtection(DMGTYPE::BLUNT).Max;
	shield.MaxPiercingProtection = shieldPtr->getProtection(DMGTYPE::PIERCING).Max;
	shield.MaxSlashingProtection = shieldPtr->getProtection(DMGTYPE::SLASHING).Max;
	shield.BluntProtectionFactor = shieldPtr->getProtection(DMGTYPE::BLUNT).Factor;
	shield.PiercingProtectionFactor = shieldPtr->getProtection(DMGTYPE::PIERCING).Factor;
	shield.SlashingProtectionFactor = shieldPtr->getProtection(DMGTYPE::BLUNT).Factor;

	shield.ShieldType = shieldPtr->getStaticForm()->Shield->ShieldType;
	shield.ArmorType = shieldPtr->getStaticForm()->Shield->ArmorType;
	return true;
} // CCombatDefenderPlayer::getShield //



//--------------------------------------------------------------
//					CCombatDefenderAI constructor
//--------------------------------------------------------------
CCombatDefenderAI::CCombatDefenderAI(const TDataSetRow &rowId) : CCombatDefender(rowId), CreatureForm(0)
{
	CEntityBase * entity = getEntity();
	if(!entity) 
		return;

	/// TODO : specialize this class for NPCs other than creature

	CreatureForm = CSheets::getCreaturesForm(entity->getType());
	if (!CreatureForm)
		return;

	_GlobalArmor.Quality = CreatureForm->Level;
	_GlobalArmor.MaxBluntProtection = CreatureForm->Protections[(uint)DMGTYPE::BLUNT].Max;
	_GlobalArmor.MaxPiercingProtection = CreatureForm->Protections[(uint)DMGTYPE::PIERCING].Max;
	_GlobalArmor.MaxSlashingProtection = CreatureForm->Protections[(uint)DMGTYPE::SLASHING].Max;
	_GlobalArmor.BluntProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::BLUNT].Factor;
	_GlobalArmor.PiercingProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::PIERCING].Factor;
	_GlobalArmor.SlashingProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::SLASHING].Factor;
	
	_GlobalArmor.MaxRotProtection = CreatureForm->Protections[(uint)DMGTYPE::ROT].Max;
	_GlobalArmor.RotProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::ROT].Factor;
	_GlobalArmor.MaxAcidProtection = CreatureForm->Protections[(uint)DMGTYPE::ACID].Max;
	_GlobalArmor.AcidProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::ACID].Factor;
	_GlobalArmor.MaxColdProtection = CreatureForm->Protections[(uint)DMGTYPE::COLD].Max;
	_GlobalArmor.ColdProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::COLD].Factor;
	_GlobalArmor.MaxFireProtection = CreatureForm->Protections[(uint)DMGTYPE::FIRE].Max;
	_GlobalArmor.FireProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::FIRE].Factor;
	_GlobalArmor.MaxPoisonProtection = CreatureForm->Protections[(uint)DMGTYPE::POISON].Max;
	_GlobalArmor.PoisonProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::POISON].Factor;
	_GlobalArmor.MaxElectricityProtection = CreatureForm->Protections[(uint)DMGTYPE::ELECTRICITY].Max;
	_GlobalArmor.ElectricityProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::ELECTRICITY].Factor;
	_GlobalArmor.MaxShockProtection = CreatureForm->Protections[(uint)DMGTYPE::SHOCK].Max;
	_GlobalArmor.ShockProtectionFactor = CreatureForm->Protections[(uint)DMGTYPE::SHOCK].Factor;

	_GlobalArmor.Skill = SKILLS::unknown;
	_GlobalArmor.SkillValue = CreatureForm->Level * 10;
	_GlobalArmor.ArmorType = ARMORTYPE::UNKNOWN;
	
	_DefenseValue = entity->getSkills()._Skills[ SKILLS::SDD ].Current;

/*	pair< const CStaticItem *, uint16> item;

	// head
	item = getArmorOnSlot( SLOT_EQUIPMENT::HEAD );
	if (item.first != NULL)
	{
		_HeadArmor = CCombatArmor( item.first, item.second);
		_HeadArmor.SkillValue = _HeadArmor.Quality * 10;
	}
	
	//chest
	item = getArmorOnSlot( SLOT_EQUIPMENT::CHEST );
	if (item.first != NULL)
	{
		_ChestArmor = CCombatArmor( item.first, item.second);
		_ChestArmor.SkillValue = _ChestArmor.Quality * 10;
	}

	//arms
	item = getArmorOnSlot( SLOT_EQUIPMENT::ARMS );
	if (item.first != NULL)
	{
		_ArmsArmor = CCombatArmor( item.first, item.second);
		_ArmsArmor.SkillValue = _ArmsArmor.Quality * 10;
	}

	//hands
	item = getArmorOnSlot( SLOT_EQUIPMENT::HANDS );
	if (item.first != NULL)
	{
		_HandsArmor = CCombatArmor( item.first, item.second);
		_HandsArmor.SkillValue = _HandsArmor.Quality * 10;
	}

	//legs
	item = getArmorOnSlot( SLOT_EQUIPMENT::LEGS );
	if (item.first != NULL)
	{
		_LegsArmor = CCombatArmor( item.first, item.second);
		_LegsArmor.SkillValue = _LegsArmor.Quality * 10;
	}

	//feet
	item = getArmorOnSlot( SLOT_EQUIPMENT::FEET );
	if (item.first != NULL)
	{
		_FeetArmor = CCombatArmor( item.first, item.second);
		_FeetArmor.SkillValue = _FeetArmor.Quality * 10;
	}

	// FIX TEMP
	if (_ChestArmor.SkillValue > _DefenseValue)
		_DefenseValue = _ChestArmor.SkillValue;
*/
} // CCombatDefenderAI constructor //


//--------------------------------------------------------------
//					CCombatDefenderAI::getArmor
//--------------------------------------------------------------
bool CCombatDefenderAI::getArmor( SLOT_EQUIPMENT::TSlotEquipment slot, CCombatArmor &armor ) const
{
	/*
	switch(slot)
	{
	case SLOT_EQUIPMENT::HEAD:
		if (_HeadArmor.Quality > 0)
			armor = _HeadArmor;
		else
			return false;
		break;

	case SLOT_EQUIPMENT::CHEST:
		if (_ChestArmor.Quality > 0)
			armor = _ChestArmor;
		else
			return false;
		break;

	case SLOT_EQUIPMENT::ARMS:
		if (_ArmsArmor.Quality > 0)
			armor = _ArmsArmor;
		else
			return false;
		break;
	
	case SLOT_EQUIPMENT::LEGS:
		if (_LegsArmor.Quality > 0)
			armor = _LegsArmor;
		else
			return false;
		break;

	case SLOT_EQUIPMENT::FEET:
		if (_FeetArmor.Quality > 0)
			armor = _FeetArmor;
		else
			return false;
		break;

	case SLOT_EQUIPMENT::HANDS:
		if (_HandsArmor.Quality > 0)
			armor = _HandsArmor;
		else
			return false;
		break;

	default:
		return false;
	};
	*/
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
/*	if (_Shield.Quality > 0)
	{
		shield = _Shield;
		return true;
	}
	else
	*/
		return false;
} // CCombatDefenderAI::getShield //


//--------------------------------------------------------------
//				CCombatDefenderAI::getArmorOnSlot()  
//--------------------------------------------------------------
pair<const CStaticItem *, uint16> CCombatDefenderAI::getArmorOnSlot( SLOT_EQUIPMENT::TSlotEquipment slot )
{
	CEntityBase * entity = getEntity();
	if (!entity) return make_pair( (const CStaticItem*)(NULL),0);

	const CEquipmentSlots &items = entity->getEquipment();
	CSheetId sheet;
	sint16	 quality = 0;

	switch( slot )
	{
	case SLOT_EQUIPMENT::HEAD:
		{			
			sheet = items.Head.IdSheet;
			quality = items.Head.Quality;
		}
		break;

	case SLOT_EQUIPMENT::ARMS:
		{
			sheet = items.Arms.IdSheet;
			quality = items.Arms.Quality;
		}
		break;

	case SLOT_EQUIPMENT::HANDS:
		{
			sheet = items.Hands.IdSheet;
			quality = items.Hands.Quality;
		}
		break;

	case SLOT_EQUIPMENT::CHEST:
		{
			sheet = items.Chest.IdSheet;
			quality = items.Chest.Quality;
		}
		break;

	case SLOT_EQUIPMENT::LEGS:
		{
			sheet = items.Legs.IdSheet;
			quality = items.Legs.Quality;
		}
		break;

	case SLOT_EQUIPMENT::FEET:
		{
			sheet = items.Feet.IdSheet;
			quality = items.Feet.Quality;
		}
		break;

	default:
		nlwarning("<CCombatAttackerAI::getArmorOnSlot> slot id %d not managed by getItemOnSlot", slot);
		return make_pair( (const CStaticItem*)(NULL),0);
	};

	if (sheet != CSheetId::Unknown)
	{
		const CStaticItem *item = CSheets::getForm(sheet);
		return make_pair( item, quality);
	}
	else
	{
		return make_pair( (const CStaticItem *)NULL, 0);
	}
} // CCombatDefenderAI::getArmorOnSlot //
