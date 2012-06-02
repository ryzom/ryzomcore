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

#include "entity_manager/entity_base.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "entity_manager/entity_manager.h"
#include "game_item_manager/game_item.h"

#include "phrase_manager/area_effect.h"


extern CPlayerManager PlayerManager;


class CCombatPhrase;
class CStaticAiAction;

struct CCombatWeapon
{
	NL_INSTANCE_COUNTER_DECL(CCombatWeapon);
public:
	/// default ctor
	inline CCombatWeapon() { init(); }

	/// copy constructor
	inline CCombatWeapon(const CCombatWeapon &weapon) { init(); (*this) = weapon; }

	/// ctor
	explicit CCombatWeapon(CGameItemPtr itemPtr);

	/// dtor
	~CCombatWeapon() 
	{
		if (Area != NULL)
			delete Area;
	}
	
	/// operator = (because of the Area pointer)
	inline CCombatWeapon &operator=(const CCombatWeapon &weapon)
	{
		LatencyInTicks = weapon.LatencyInTicks;
		Damage = weapon.Damage;
		Quality = weapon.Quality;
		DmgType = weapon.DmgType;
		Family = weapon.Family;
		Range = weapon.Range;
		Skill = weapon.Skill;
		SabrinaCost = weapon.SabrinaCost;
		AreaType = weapon.AreaType;
		ReachValue = weapon.ReachValue;
		IsDirectRangeAttack = weapon.IsDirectRangeAttack;
		
		if (weapon.Area != NULL)
		{
			if ( Area != NULL )
				(*Area) = (*weapon.Area);
			else
				Area = new CAreaEffect(*weapon.Area);
		}
		else if (Area != NULL)
		{
			delete Area;
			Area = NULL;
		}
		return *this;
	}

	std::string toString() const
	{
		const std::string temp = NLMISC::toString("Damage = %f, Quality = %u, Skill = %s, dmgType = %s, Family = %s, SabrinaCost=%u", Damage, Quality, SKILLS::toString(Skill).c_str(),DMGTYPE::toString(DmgType).c_str(), ITEMFAMILY::toString(Family).c_str(), SabrinaCost );
		return temp;
	}

	/************************************************************************/
	/* do not forget to update the operator= if attributes change
	/************************************************************************/
	uint16					LatencyInTicks;
	float					Damage;	
	uint16					Quality;
	DMGTYPE::EDamageType	DmgType;
	ITEMFAMILY::EItemFamily	Family;
	float					Range; // if > 0 range weapon or ammo, == 0 melee weapon	
	SKILLS::ESkills			Skill;
	uint16					SabrinaCost; // sabrina cost of the weapon
	bool					IsDirectRangeAttack;
	CAreaEffect				*Area;
	RANGE_WEAPON_TYPE::TRangeWeaponType AreaType;

//	CGameItemRefPtr			ItemRefPtr;

	uint8					ReachValue;

private:
	void init()
	{
		LatencyInTicks = 0;
		Damage = 0;
		Quality = 0;
		DmgType = DMGTYPE::UNDEFINED;
		Range = 0;
		Skill = SKILLS::unknown;
		Family = ITEMFAMILY::UNDEFINED;
		IsDirectRangeAttack = false;		// default for backward compatibility
		SabrinaCost = 0;
		Area = NULL;
		AreaType = RANGE_WEAPON_TYPE::Unknown;
		ReachValue = 0;
	}
};


/**
 * Base class for combat attackers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAttacker
{
	NL_INSTANCE_COUNTER_DECL(CCombatAttacker);
public:

	enum TAttackerItem
	{
		RightHandItem =0,
		LeftHandItem,
		Ammo,

		Unknown,
	};

public:
	CCombatAttacker()
	{}

	CCombatAttacker( const TDataSetRow &rowId) : _RowId(rowId)
	{}

	virtual ~CCombatAttacker()
	{}

	inline bool isValid() const
	{
		return (TheDataset.isAccessible(_RowId));
	}

	virtual void lockRightItem() = 0;
	virtual void lockLeftItem() = 0;
	virtual void lockAmmos(uint16 nb = 1) = 0;
	virtual void unlockRightItem() = 0;
	virtual void unlockLeftItem() = 0;
	virtual void unlockAmmos(uint16 nb = 1) = 0;

	virtual void damageItem(double hpLost, bool right = true) = 0; 

	virtual sint32 getSkillValue( SKILLS::ESkills skill, EGSPD::CPeople::TPeople targetRace) const = 0;
	virtual sint32 getSkillBaseValue( SKILLS::ESkills skill) const = 0;

	inline CEntityBase *getEntity() { return CEntityBaseManager::getEntityBasePtr(_RowId); }
	inline const TDataSetRow &getEntityRowId() const { return _RowId; }

	virtual bool checkAmmoAmount( uint32 qty = 1) const = 0;
	virtual void consumeAmmos( uint32 qty = 1) = 0;

	virtual bool getItem( TAttackerItem item, CCombatWeapon &weaponItem) const = 0;

	virtual sint32 adversaryDodgeModifier() const { return 0; }
	virtual sint32 adversaryParryModifier() const { return 0; }

protected:
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
		CCharacter * character = PlayerManager.getChar(_RowId);
		if(!character)
		{
			nlwarning("Failed to find character to build CCombatAttackerPlayer !");
			return;
		}

#if !FINAL_VERSION
		nlassert(character->getId().getType() == RYZOMID::player);
#endif

		_Character = character;

//		_RightHandItem = _Character->getRightHandItem();
//		_LeftHandItem = _Character->getLeftHandItem();
//		_Ammos = _Character->getAmmoItem();
	}

	virtual ~CCombatAttackerPlayer()
	{}

inline void lockRightItem() 
	{
/*		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;
		if (player->getRightHandItem() != NULL) player->getRightHandItem()->setLockState(player->getRightHandItem()->getLockState() + 1 ); 
*/	}

	inline void lockLeftItem() 
	{
/*		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;
		if (player->getLeftHandItem() != NULL) player->getLeftHandItem()->setLockState(player->getLeftHandItem()->getLockState() + 1 ); 
*/	}

	inline void lockAmmos(uint16 nb = 1) 
	{
/*		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;
		if (player->getAmmoItem() != NULL) player->getAmmoItem()->setLockState(player->getAmmoItem()->getLockState() + nb ); 
*/	}

	inline void unlockRightItem() 
	{
/*		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;
		if (player->getRightHandItem() != NULL) player->getRightHandItem()->setLockState(player->getRightHandItem()->getLockState() - 1); 
*/	}

	inline void unlockLeftItem() 
	{
/*		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;
		if (player->getLeftHandItem() != NULL) player->getLeftHandItem()->setLockState(player->getLeftHandItem()->getLockState() - 1); 
*/	}

	inline void unlockAmmos(uint16 nb = 1) 
	{ 
/*		CCharacter *player = PlayerManager.getChar(_RowId);
		if(!player) return;
		if (player->getAmmoItem() != NULL) player->getAmmoItem()->setLockState(player->getAmmoItem()->getLockState() - nb); 
*/	}

	virtual bool checkAmmoAmount( uint32 qty = 1) const;

	virtual void consumeAmmos( uint32 qty = 1) 
	{
		if(!_Character) return;
		_Character->consumeAmmo(qty);
	}

	virtual void damageItem(double hpLost, bool right = true) 
	{
		if (_Character == NULL)
			return;

		CGameItemPtr item;
		if (right)
			item = _Character->getRightHandItem();
		else
			item = _Character->getLeftHandItem();

		if (item != NULL)
			item->removeHp(hpLost);
	}
	
	virtual bool getItem( TAttackerItem item, CCombatWeapon &weaponItem) const;

	inline sint32 getSkillValue( SKILLS::ESkills skill, EGSPD::CPeople::TPeople targetRace ) const
	{
		if (!_Character || skill >= SKILLS::NUM_SKILLS || skill<0) 
			return 0;

		// add skill mod according to target race
		return _Character->getSkillValue(skill) + _Character->getSkillModifierForRace(targetRace);
	}

	inline sint32 getSkillBaseValue( SKILLS::ESkills skill) const
	{
		if (!_Character || skill >= SKILLS::NUM_SKILLS || skill<0) 
			return 0;
		
		return _Character->getSkillBaseValue(skill);
	}

	inline sint32 adversaryDodgeModifier() const
	{
		if (_Character)
			return _Character->adversaryDodgeModifier();
		else
			return 0;
	}
	
	inline sint32 adversaryParryModifier() const
	{
		if (_Character)
			return _Character->adversaryParryModifier();
		else
			return 0;
	}

private:
	/// keep a ref pointer on original character
	CCharacterRefPtr	_Character;
};


/**
 * Derived class for AI attackers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAttackerAI : public CCombatAttacker
{
	NL_INSTANCE_COUNTER_DECL(CCombatAttackerAI);
public:
	CCombatAttackerAI() {}

	explicit CCombatAttackerAI(const TDataSetRow &rowId, const CStaticAiAction *aiAction);

	inline explicit CCombatAttackerAI(const TDataSetRow &rowId) : CCombatAttacker(rowId)
	{
		initFromRowId(rowId);
	}

	virtual ~CCombatAttackerAI()
	{}

	inline void consumeAmmos( uint32 qty = 1) {}
	inline void damageItem(double, bool = true) {}
	inline void lockRightItem() {}
	inline void lockLeftItem() {}
	inline void lockAmmos(uint16 nb = 1) {}
	inline void unlockRightItem() {}
	inline void unlockLeftItem() {}
	inline void unlockAmmos(uint16 nb = 1) {}

	inline sint32 getSkillValue( SKILLS::ESkills skill, EGSPD::CPeople::TPeople) const { return _SkillValue; }
	inline sint32 getSkillBaseValue( SKILLS::ESkills skill) const { return _SkillValue; }
	
	inline bool checkAmmoAmount( uint32 qty = 1) const { return true; }

	virtual bool getItem( TAttackerItem item, CCombatWeapon &weaponItem) const;	

protected:
	// init from rowId
	virtual void initFromRowId(const TDataSetRow &rowId);

protected:
	// right hand item
	CCombatWeapon	_RightHandWeapon;
	
	// left hand item
	CCombatWeapon	_LeftHandWeapon;

	// ammo
	CCombatWeapon	_Ammo;

	/// attack skill value
	sint32			_SkillValue;
};


/**
 * Specialized class for NPC attackers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CCombatAttackerNpc : public CCombatAttackerAI
{
public:
	CCombatAttackerNpc() : CCombatAttackerAI()
	{}
	explicit CCombatAttackerNpc(const TDataSetRow &rowId, const CStaticAiAction *aiAction);

	inline explicit CCombatAttackerNpc(const TDataSetRow &rowId)
	{
		initFromRowId(rowId);
	}

	virtual ~CCombatAttackerNpc()
	{}

protected:
	// init from rowId
	virtual void initFromRowId(const TDataSetRow &rowId);	
};

#endif // RY_COMBAT_ATTACKER_H


/* End of combat_attacker.h */
