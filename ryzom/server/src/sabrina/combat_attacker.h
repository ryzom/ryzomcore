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



#ifndef RY_COMBAT_ATTACKER_H
#define RY_COMBAT_ATTACKER_H

// nel misc
#include "nel/misc/types_nl.h"
// game_share
#include "game_share/tick_event_handler.h"
#include "game_share/game_item_manager/game_item.h"
//
#include "entity_base.h"
#include "character.h"
#include "player_manager.h"
#include "entity_manager.h"

extern CPlayerManager PlayerManager;


struct CCombatWeapon
{
	CCombatWeapon()
	{
		SpeedInTicks = 0;
		Damage = 0;
		Quality = 0;
		DmgType = DMGTYPE::UNDEFINED;
		Range = 0;
		Skill = SKILLS::unknown;
		SkillValue = 0;
		Family = ITEMFAMILY::UNDEFINED;
	}

	CCombatWeapon(CGameItemPtr itemPtr);

	std::string toString() const
	{
		const std::string temp = NLMISC::toString("Damage = %u, Quality = %u, SkillValue = %u, dmgType = %s, Family = %s", Damage, Quality, SkillValue,DMGTYPE::toString(DmgType).c_str(), ITEMFAMILY::toString(Family).c_str() );
		return temp;
	}

	uint16					SpeedInTicks;
	uint16					Damage;	
	uint16					Quality;
	DMGTYPE::EDamageType	DmgType;
	ITEMFAMILY::EItemFamily	Family;
	float					Range; // if > 0 range weapon or ammo, == 0 melee weapon	
	SKILLS::ESkills			Skill; // no meaning for ammos
	sint32					SkillValue; // no meaning for ammos
};


/**
 * Base class for combat attackers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAttacker
{
public:
	enum TAttackerItem
	{
		RightHandItem =0,
		LeftHandItem,
		Ammo,

		Unknown,
	};

public:
	CCombatAttacker() {}

	CCombatAttacker( const TDataSetRow &rowId) : _RowId(rowId)
	{}

	virtual ~CCombatAttacker()
	{}

	virtual void lockRightItem() = 0;
	virtual void lockLeftItem() = 0;
	virtual void lockAmmos(uint16 nb = 1) = 0;
	virtual void unlockRightItem() = 0;
	virtual void unlockLeftItem() = 0;
	virtual void unlockAmmos(uint16 nb = 1) = 0;

	virtual void damageItem(bool right = true) = 0;

	virtual sint32 getSkillValue( SKILLS::ESkills skill) const = 0;

	inline CEntityBase *getEntity() { return CEntityBaseManager::getEntityBasePtr(_RowId); }
	inline const TDataSetRow &getEntityRowId() const { return _RowId; }

	virtual bool checkAmmoAmount( uint8 qty = 1) const = 0;
	virtual void consumeAmmos( uint8 qty = 1) = 0;

	virtual bool getItem( TAttackerItem item, CCombatWeapon &weaponItem) const = 0;

public:
	/// row id
	TDataSetRow		_RowId;
};


/**
 * Derived class for player attackers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAttackerPlayer : public CCombatAttacker
{
public:
	CCombatAttackerPlayer(const TDataSetRow &rowId) : CCombatAttacker(rowId)
	{
		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;

		_RightHandItem = player->getRightHandItem();
		_LeftHandItem = player->getLeftHandItem();
		_Ammos = player->getAmmoItem();
	}

	virtual ~CCombatAttackerPlayer()
	{}

	inline void lockRightItem() { if (_RightHandItem != NULL) _RightHandItem->setLockState(_RightHandItem->getLockState() + 1 ); }
	inline void lockLeftItem() { if (_LeftHandItem != NULL) _LeftHandItem->setLockState(_LeftHandItem->getLockState() + 1 ); }
	inline void lockAmmos(uint16 nb = 1) { if (_Ammos != NULL) _Ammos->setLockState(_Ammos->getLockState() + nb ); }

	inline void unlockRightItem() { if (_RightHandItem != NULL) _RightHandItem->setLockState(_RightHandItem->getLockState() - 1); }
	inline void unlockLeftItem() { if (_LeftHandItem != NULL) _LeftHandItem->setLockState(_LeftHandItem->getLockState() - 1); }
	inline void unlockAmmos(uint16 nb = 1) { if (_Ammos != NULL) _Ammos->setLockState(_Ammos->getLockState() - nb); }

	virtual bool checkAmmoAmount( uint8 qty = 1) const;

	virtual void consumeAmmos( uint8 qty = 1) 
	{
		CCharacter *character = PlayerManager.getChar(_RowId);
		if(!character) return;
		character->consumeAmmo(qty);
	}

	inline void damageItem(bool right = true) {}
	
	virtual bool getItem( TAttackerItem item, CCombatWeapon &weaponItem) const;

	inline sint32 getSkillValue( SKILLS::ESkills skill) const
	{
		CCharacter *character = PlayerManager.getChar(_RowId);
		if (!character || skill >= SKILLS::NUM_SKILLS) return 0;
		return character->getSkills()._Skills[ skill ].Current;
	}

	
public:
	/// right hand item
	CGameItemPtr	_RightHandItem;

	/// left hand item
	CGameItemPtr	_LeftHandItem;

	/// current ammos
	CGameItemPtr	_Ammos;
};


/**
 * Derived class for AI attackers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAttackerAI : public CCombatAttacker
{
public:
	CCombatAttackerAI() 
	{}

	CCombatAttackerAI(const TDataSetRow &rowId);

	virtual ~CCombatAttackerAI()
	{}

	inline void consumeAmmos( uint8 qty = 1) {}
	inline void damageItem(bool right = true) {}
	inline void lockRightItem() {}
	inline void lockLeftItem() {}
	inline void lockAmmos(uint16 nb = 1) {}
	inline void unlockRightItem() {}
	inline void unlockLeftItem() {}
	inline void unlockAmmos(uint16 nb = 1) {}

	inline sint32 getSkillValue( SKILLS::ESkills skill) const { return _RightHandWeapon.SkillValue; }

	inline bool checkAmmoAmount( uint8 qty = 1) const { return true; }

	virtual bool getItem( TAttackerItem item, CCombatWeapon &weaponItem) const;	

public:
	// right hand item
	CCombatWeapon	_RightHandWeapon;
	
	// left hand item
	CCombatWeapon	_LeftHandWeapon;

	// ammo
	CCombatWeapon	_Ammo;
};


#endif // RY_COMBAT_ATTACKER_H

/* End of combat_attacker.h */
