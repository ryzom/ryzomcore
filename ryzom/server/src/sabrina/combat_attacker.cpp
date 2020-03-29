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
#include "phrase_utilities_functions.h"

using namespace std;
using namespace NLMISC;

bool UseWeaponDamageFactorModifier = true;

NLMISC_COMMAND(UseWeaponDamageFactorModifier,"toggle the use of the item damage factor (temporary patch item damage )","")
{
	UseWeaponDamageFactorModifier = !UseWeaponDamageFactorModifier;

	log.displayNL("UseWeaponDamageFactorModifier is %s",UseWeaponDamageFactorModifier?"used":"unused");
	return true;
}

//--------------------------------------------------------------
//					CCombatWeapon constructor
//--------------------------------------------------------------
CCombatWeapon::CCombatWeapon(CGameItemPtr itemPtr)
{
	if (itemPtr == NULL || itemPtr->getStaticForm() == NULL )
		return;

	Family = itemPtr->getStaticForm()->Family;

	switch(Family)
	{
	case ITEMFAMILY::MELEE_WEAPON:
		if (!itemPtr->getStaticForm()->MeleeWeapon)
			return;
		DmgType = itemPtr->getStaticForm()->MeleeWeapon->DamageType;
		Skill = itemPtr->getStaticForm()->Skill;
		break;

	case ITEMFAMILY::RANGE_WEAPON:
		if (!itemPtr->getStaticForm()->RangeWeapon)
			return;
		DmgType = itemPtr->getStaticForm()->RangeWeapon->DamageType;
		Skill = itemPtr->getStaticForm()->Skill;
		Range = itemPtr->getRange();
		break;

	case ITEMFAMILY::AMMO:
		if (!itemPtr->getStaticForm()->Ammo)
			return;
		DmgType = itemPtr->getStaticForm()->Ammo->DamageType;
		Range = itemPtr->getRange();
		break;

	default:
		break;
	};

	if (Skill == SKILLS::unknown)
	{
		nlwarning("<CCombatWeapon::CCombatWeapon> Error : item %s skill is Unknown", itemPtr->getStaticForm()->Name.c_str());
	}

	SpeedInTicks = uint16(itemPtr->getSpeed() / CTickEventHandler::getGameTimeStep());	
	Quality = itemPtr->quality();

	if ( UseWeaponDamageFactorModifier )
		Damage = itemPtr->getDamage() * 260 / (10 + Quality*10);
	else
		Damage = itemPtr->getDamage();


	// SkillValue must be init in the calling class
	SkillValue = 0;
} // CCombatWeapon //


//--------------------------------------------------------------
//					CCombatAttackerPlayer::getItem
//--------------------------------------------------------------
bool CCombatAttackerPlayer::getItem( TAttackerItem item, CCombatWeapon &weaponItem) const
{
	switch(item)
	{
	case RightHandItem:
		if ( _RightHandItem != NULL)
		{
			weaponItem = CCombatWeapon(_RightHandItem);
			return true;
		}
		else 
			return false;
		break;

	case LeftHandItem:
		if ( _LeftHandItem != NULL )
		{
			weaponItem = CCombatWeapon(_LeftHandItem);
			return true;
		}
		else 
			return false;
		break;

	case Ammo:
		if ( _Ammos != NULL)
		{
			weaponItem = CCombatWeapon( _Ammos );
			return true;
		}
		else 
			return false;
		break;

	default:
		return false;
	};
} // CCombatAttackerPlayer::getItem //


//--------------------------------------------------------------
//					CCombatAttackerPlayer::getItem
//--------------------------------------------------------------
bool CCombatAttackerPlayer::checkAmmoAmount( uint8 qty ) const
{
	static const CSheetId StackItem("stack.sitem");

	if (_Ammos != NULL)
	{
		uint16 nbAmmo = 0;
		if (_Ammos->getSheetId() == StackItem)
		{
			nbAmmo = _Ammos->getChildren().size(); //- _Ammos->getLockState();
		}
		else
		{
			nbAmmo = 1 ;//- _Ammos->getLockState();
		}
		
		return (nbAmmo >= (uint16)qty);
	}
	else
		return false;
} // CCombatAttackerPlayer::checkAmmoAmount //



//--------------------------------------------------------------
//				CCombatAttackerAI::CCombatAttackerAI
//--------------------------------------------------------------
CCombatAttackerAI::CCombatAttackerAI( const TDataSetRow &rowId ) : CCombatAttacker(rowId)
{
	CEntityBase  *entity = CEntityBaseManager::getEntityBasePtr(rowId);
	if (!entity)
		return;

	const CStaticCreatures * form = CSheets::getCreaturesForm( entity->_SheetId );
	if ( !form )
	{
		nlwarning( "<CCombatAttackerAI::CCombatAttackerAI> invalid creature form %s in entity %s", entity->_SheetId.toString().c_str(), entity->getId().toString().c_str() );
		return;
	}
	
	_RightHandWeapon.Quality = form->Level;
	_RightHandWeapon.Damage = form->CreatureDamagePerHit * 260/(10+10*form->Level);

	_RightHandWeapon.DmgType = DMGTYPE::SLASHING;
	_RightHandWeapon.SpeedInTicks = 30;
	_RightHandWeapon.Family = ITEMFAMILY::MELEE_WEAPON;
	_RightHandWeapon.Skill = SKILLS::SFM1H;

	INFOLOG("AttackerAi :_RightHandWeapon.Quality = %u, _RightHandWeapon.Damage = %u", _RightHandWeapon.Quality, _RightHandWeapon.Damage);
} // CCombatAttackerAI::CCombatAttackerAI //


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



