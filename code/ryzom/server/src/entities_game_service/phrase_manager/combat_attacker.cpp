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
#include "combat_attacker.h"
#include "phrase_manager/phrase_manager.h"
#include "creature_manager/creature_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_utilities_functions.h"

#include "egs_sheets/egs_sheets.h"
#include "egs_sheets/egs_static_ai_action.h"

using namespace std;
using namespace NLMISC;

NL_INSTANCE_COUNTER_IMPL(CCombatAttacker);
NL_INSTANCE_COUNTER_IMPL(CCombatWeapon);
NL_INSTANCE_COUNTER_IMPL(CCombatAttackerAI);


extern CCreatureManager CreatureManager;

// skill used when no weapon in hand (hand to hand combat)
extern SKILLS::ESkills	BarehandCombatSkill;
extern CVariable<uint16> HandToHandReachValue;


//--------------------------------------------------------------
//					CCombatWeapon constructor
//--------------------------------------------------------------
static bool isDirectRangeAttack(ITEM_TYPE::TItemType it)
{
	// only pistols, rifles and autolaunchers
	return it== ITEM_TYPE::PISTOL || it== ITEM_TYPE::BOWPISTOL || 
		it== ITEM_TYPE::BOWRIFLE || it==ITEM_TYPE::RIFLE ||
		it== ITEM_TYPE::AUTOLAUCH;

	// NB: in particular, launcher are not direct range attack
}

//--------------------------------------------------------------
//					CCombatWeapon constructor
//--------------------------------------------------------------
CCombatWeapon::CCombatWeapon(CGameItemPtr itemPtr)
{
	init();

	if (itemPtr == NULL)
		return;

//	const static CSheetId StackSheet("stack.sitem");

	// if item is a stack, get the first child (for ammos)
//	if (itemPtr->getSheetId() == StackSheet)
//	{
//		nlassert(false);
////		if ( !itemPtr->getChildren().empty() && itemPtr->getChildren()[0] != NULL)
//		if ( itemPtr->getNumChildren() == 0 && itemPtr->getChildItem(0) != NULL)
//		{
////			itemPtr = itemPtr->getChildren()[0];
//			itemPtr = itemPtr->getChildItem(0);
//		}
//		else
//		{
//			nlwarning("<CCombatWeapon::CCombatWeapon> Param item ptr is an empty stack, or first ptr is stack is NULL, cancel");
//			return;	
//		}
//	}

	if( itemPtr->getStaticForm() == NULL )
		return;

	Family = itemPtr->getStaticForm()->Family;
	IsDirectRangeAttack= isDirectRangeAttack(itemPtr->getStaticForm()->Type);

	switch(Family)
	{
	case ITEMFAMILY::MELEE_WEAPON:
		if (!itemPtr->getStaticForm()->MeleeWeapon)
			return;
		DmgType = itemPtr->getStaticForm()->MeleeWeapon->DamageType;
		Skill = itemPtr->getStaticForm()->Skill;
		ReachValue = itemPtr->getStaticForm()->MeleeWeapon->ReachValue;
		break;

	case ITEMFAMILY::RANGE_WEAPON:
		if (!itemPtr->getStaticForm()->RangeWeapon)
			return;
		DmgType = itemPtr->getStaticForm()->RangeWeapon->DamageType;
		Skill = itemPtr->getStaticForm()->Skill;
		Range = itemPtr->range();
		nlassert(Area == NULL);
		Area = CAreaEffect::buildArea( itemPtr );
		AreaType = itemPtr->getStaticForm()->RangeWeapon->AreaType;
		break;

	case ITEMFAMILY::AMMO:
		if (!itemPtr->getStaticForm()->Ammo)
			return;
		DmgType = itemPtr->getStaticForm()->Ammo->DamageType;
		Range = itemPtr->range();
		break;

	default:
		return;
	};

	if (Skill == SKILLS::unknown)
	{
//		nlwarning("<CCombatWeapon::CCombatWeapon> Error : item %s skill is Unknown", itemPtr->getStaticForm()->Name.c_str());
	}

	// weapon hit rate is in hit/10s and we use ticks/hits....
	if (itemPtr->hitRate() != 0)
	{
		LatencyInTicks = uint16( (10.0f / itemPtr->hitRate())  / CTickEventHandler::getGameTimeStep());
	}
	
	Quality = (uint16)itemPtr->recommended();
	Damage = itemPtr->damageFactor();
} // CCombatWeapon //


//--------------------------------------------------------------
//					CCombatAttackerPlayer::getItem
//--------------------------------------------------------------
bool CCombatAttackerPlayer::getItem( TAttackerItem item, CCombatWeapon &weaponItem) const
{
	if(!_Character) 
		return false;

//	const static CSheetId StackSheet("stack.sitem");

	CGameItemPtr itemPtr;
	switch(item)
	{
	case RightHandItem:
		itemPtr = _Character->getRightHandItem();
		if ( itemPtr == NULL || itemPtr->getStaticForm() == NULL 
//			|| (itemPtr->getStaticForm()->Family != ITEMFAMILY::MELEE_WEAPON && itemPtr->getStaticForm()->Family != ITEMFAMILY::RANGE_WEAPON)
			)
		{
			return false;
		}
		break;

	case LeftHandItem:
		itemPtr = _Character->getLeftHandItem();
		if ( itemPtr == NULL || itemPtr->getStaticForm() == NULL 
//			|| (itemPtr->getStaticForm()->Family != ITEMFAMILY::MELEE_WEAPON && itemPtr->getStaticForm()->Family != ITEMFAMILY::RANGE_WEAPON)
			)
		{
			return false;
		}
		break;

	case Ammo:
		itemPtr = _Character->getLeftHandItem();
		// if item is a stack, get the first child (for ammos)
//		if (itemPtr != NULL && itemPtr->getSheetId() == StackSheet)
//		{
//			nlassert(false);
////			if ( !itemPtr->getChildren().empty() && itemPtr->getChildren()[0] != NULL)
//			if ( itemPtr->getNumChildren() != 0 && itemPtr->getChildItem(0) != NULL)
//			{
////				itemPtr = itemPtr->getChildren()[0];
//				itemPtr = itemPtr->getChildItem(0);
//			}
//		}		
		if ( itemPtr == NULL || itemPtr->getStaticForm() == NULL 
//			|| itemPtr->getStaticForm()->Family != ITEMFAMILY::AMMO
			)
		{
			return false;
		}
		break;

	default:
		return false;
	};

	weaponItem = CCombatWeapon(itemPtr);
	if( itemPtr->hitRate() == 0 )
	{
		if (weaponItem.Family == ITEMFAMILY::MELEE_WEAPON || weaponItem.Family == ITEMFAMILY::RANGE_WEAPON)
		{
			nlwarning("<CCombatAttackerPlayer::getItem> Error while building item, found an hit rate = 0 for item %s!", itemPtr->getSheetId().toString().c_str());
		}
	}

	return true;
} // CCombatAttackerPlayer::getItem //


//--------------------------------------------------------------
//					CCombatAttackerPlayer::getItem
//--------------------------------------------------------------
bool CCombatAttackerPlayer::checkAmmoAmount( uint32 qty ) const
{
//	static const CSheetId StackItem("stack.sitem");

	if(!_Character) return false;

	if (_Character->getAmmoItem() != NULL)
	{
		uint32 nbAmmo = 0;
//		if (_Character->getAmmoItem()->getSheetId() == StackItem)
//		{
////			nbAmmo = _Character->getAmmoItem()->getChildren().size(); //- _Ammos->getLockState();
//			nbAmmo = uint16(_Character->getAmmoItem()->getNumChildren()); //- _Ammos->getLockState();
//		}
//		else
//		{
//			nbAmmo = 1 ;//- _Ammos->getLockState();
//		}

		nbAmmo = _Character->getAmmoItem()->getStackSize();
		
		return (nbAmmo >= qty);
	}
	else
		return false;
} // CCombatAttackerPlayer::checkAmmoAmount //


//--------------------------------------------------------------
//				CCombatAttackerAI::CCombatAttackerAI
//--------------------------------------------------------------
CCombatAttackerAI::CCombatAttackerAI(const TDataSetRow &rowId, const CStaticAiAction *aiAction) : CCombatAttacker(rowId)
{
#ifdef NL_DEBUG
	nlassert(aiAction);
	nlassert(aiAction->getType() == AI_ACTION::Melee || aiAction->getType() == AI_ACTION::Range);
#endif
	initFromRowId(rowId);
	
	if ( aiAction->getData().Combat.DamageType != DMGTYPE::UNDEFINED)
		_RightHandWeapon.DmgType = aiAction->getData().Combat.DamageType;
	
	if (aiAction->getType() == AI_ACTION::Range)
	{
		_RightHandWeapon.Family = ITEMFAMILY::RANGE_WEAPON;
		_RightHandWeapon.IsDirectRangeAttack= false;	// default for backward compatibility
		_Ammo = _RightHandWeapon;
		_Ammo.Family = ITEMFAMILY::AMMO;
		_Ammo.LatencyInTicks = 0;
		_RightHandWeapon.Damage = 0;
	}
	
	nlassert(_RightHandWeapon.Area == NULL);
	_RightHandWeapon.Area = CAreaEffect::buildArea(aiAction);
}

//--------------------------------------------------------------
//				CCombatAttackerAI::initFromRowId
//--------------------------------------------------------------
void CCombatAttackerAI::initFromRowId( const TDataSetRow &rowId )
{
	CCreature  *entity = CreatureManager.getCreature(rowId);
	if (!entity)
		return;

#if !FINAL_VERSION
	nlassert(entity->getId().getType() == RYZOMID::creature);
#endif

	const CStaticCreatures * form = entity->getForm();
	if ( !form )
	{
		nlwarning( "<CCombatAttackerAI::CCombatAttackerAI> invalid creature form %s in entity %s", entity->_SheetId.toString().c_str(), entity->getId().toString().c_str() );
		return;
	}

	if (entity->getId().getType() != RYZOMID::npc)
	{
		nlassert(form->getAttackLevel() != 0);
		_SkillValue = form->getAttackLevel();
		
		_RightHandWeapon.Quality = (uint16)_SkillValue;
		_RightHandWeapon.Damage = (float)form->getCreatureDamagePerHit() * BotDamageFactor;
		
		_RightHandWeapon.DmgType = DMGTYPE::SLASHING;
		_RightHandWeapon.LatencyInTicks = (uint16)form->getAttackLatency();
		_RightHandWeapon.Family = ITEMFAMILY::MELEE_WEAPON;
		_RightHandWeapon.Skill = BarehandCombatSkill;
		_RightHandWeapon.SabrinaCost = (uint16)_SkillValue;
		_RightHandWeapon.ReachValue = form->getMeleeReachValue();
	}
	else
	{
		nlwarning("CCombatAttackerAI : constructor called for an NPC, should have use CCombatAttackerNpc");
		return;
	}

	INFOLOG("AttackerAi %s :_RightHandWeapon.Quality = %u, _RightHandWeapon.Damage = %u", entity->getId().toString().c_str(), _RightHandWeapon.Quality, _RightHandWeapon.Damage);
} // CCombatAttackerAI::initFromRowId //


//--------------------------------------------------------------
//					CCombatAttackerAI::getItem
//--------------------------------------------------------------
bool CCombatAttackerAI::getItem( TAttackerItem item, CCombatWeapon &weaponItem) const
{
	switch(item)
	{
	case RightHandItem:
		if ( _RightHandWeapon.Quality != 0)
		{
			weaponItem = _RightHandWeapon;
			return true;
		}
		else 
			return false;
		break;

	case LeftHandItem:
		if ( _LeftHandWeapon.Quality != 0)
		{
			weaponItem = _LeftHandWeapon;
			return true;
		}
		else 
			return false;
		break;

	case Ammo:
		if ( _Ammo.Quality != 0)
		{
			weaponItem = _Ammo;
			return true;
		}
		else 
			return false;
		break;

	default:
		return false;
	};
} // CCombatAttackerAI::getItem //

//--------------------------------------------------------------
//				CCombatAttackerNpc::CCombatAttackerNpc
//--------------------------------------------------------------
CCombatAttackerNpc::CCombatAttackerNpc(const TDataSetRow &rowId, const CStaticAiAction *aiAction)
{
#ifdef NL_DEBUG
	nlassert(aiAction);
	nlassert(aiAction->getType() == AI_ACTION::Melee || aiAction->getType() == AI_ACTION::Range);
#endif
	initFromRowId(rowId);
	
	if ( aiAction->getData().Combat.DamageType != DMGTYPE::UNDEFINED)
		_RightHandWeapon.DmgType = aiAction->getData().Combat.DamageType;
	
	if (aiAction->getType() == AI_ACTION::Range)
	{
		_RightHandWeapon.Family = ITEMFAMILY::RANGE_WEAPON;
		// Leave IsDirectRangeAttack, according to the weapon hold by the NPC
		
		// check ammo 
		if (_Ammo.Family != ITEMFAMILY::AMMO)
		{
			_Ammo = _RightHandWeapon;
			_Ammo.Family = ITEMFAMILY::AMMO;
			_Ammo.LatencyInTicks = 0;
			_RightHandWeapon.Damage = 0;
		}
	}
	
	if (_RightHandWeapon.Area == NULL)
		_RightHandWeapon.Area = CAreaEffect::buildArea(aiAction);
}

//--------------------------------------------------------------
//				CCombatAttackerNpc::initFromRowId
//--------------------------------------------------------------
void CCombatAttackerNpc::initFromRowId( const TDataSetRow &rowId )
{
	_RowId = rowId;

	CCreature  *entity = CreatureManager.getCreature(rowId);
	if (!entity)
		return;

#if !FINAL_VERSION
	nlassert(entity->getId().getType() == RYZOMID::npc);
#endif

	const CStaticCreatures * form = entity->getForm();
	if ( !form )
	{
		nlwarning( "<CCombatAttackerNpc::CCombatAttackerNpc> invalid creature form %s in entity %s", entity->_SheetId.toString().c_str(), entity->getId().toString().c_str() );
		return;
	}

	nlassert(form->getAttackLevel() != 0);

	if ( entity->getRightHandItem() != NULL)
	{
		// get speed, dmg type, skill and family
		_RightHandWeapon = CCombatWeapon(entity->getRightHandItem());
		_RightHandWeapon.LatencyInTicks = (uint16)form->getAttackLatency();

		// check ammo 
		if (entity->getAmmoItem() != NULL && entity->getAmmoItem()->getStaticForm() != NULL && entity->getAmmoItem()->getStaticForm()->Family == ITEMFAMILY::AMMO)
		{
			_Ammo = CCombatWeapon(entity->getAmmoItem());
		}
	}
	else
	{
		_RightHandWeapon.LatencyInTicks = (uint16)form->getAttackLatency();
		_RightHandWeapon.Family = ITEMFAMILY::MELEE_WEAPON;
		_RightHandWeapon.Skill = BarehandCombatSkill;
		_RightHandWeapon.DmgType = DMGTYPE::BLUNT;
	}

	// for npc the only params used from the weapon are latency and dmg type
	_SkillValue = form->getAttackLevel();

	if (_RightHandWeapon.LatencyInTicks < 5)
		_RightHandWeapon.LatencyInTicks = HandToHandLatency;

	_RightHandWeapon.Quality = (uint16)_SkillValue;
	_RightHandWeapon.Damage = (float)form->getCreatureDamagePerHit() * BotDamageFactor;
	_RightHandWeapon.SabrinaCost = (uint16)_SkillValue;
	_RightHandWeapon.ReachValue = form->getMeleeReachValue();


	INFOLOG("CCombatAttackerNpc %s :_RightHandWeapon.Quality = %u, _RightHandWeapon.Damage = %u", entity->getId().toString().c_str(), _RightHandWeapon.Quality, _RightHandWeapon.Damage);
} // CCombatAttackerNpc::initFromRowId //
