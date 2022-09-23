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



#ifndef RY_SABRINA_ACTOR_CREATURE_H
#define RY_SABRINA_ACTOR_CREATURE_H

// Sabrina
#include "sabrina_actor.h"
#include "sabrina_pointers.h"

/**
 * AI specialisation of Sabrina actor class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CSabrinaActorCreature: public ISabrinaActor
{
public:
	//------------------------------------------------------------------------------------------------------
	// ctor and dtor
	CSabrinaActorCreature(CEntityBase *parent);
	CSabrinaActorCreature(const CSabrinaActorCreature& other);
	virtual ~CSabrinaActorCreature();


	//------------------------------------------------------------------------------------------------------
	// Management of the creature's own actions

	virtual void beginSabrinaAction(const ISabrinaPhraseModel* phrase);
	virtual void cancelSabrinaAction(SABRINA::TEventCode reason);


	//------------------------------------------------------------------------------------------------------
	// Virtual application of results of a Sabrina Action to a target

	virtual void cbSabrinaActionApplyBegin();
	virtual void applyHeal(SABRINA::THealType healType,uint32 value);
	virtual void applyDamage(SABRINA::TDmgType damageType, uint32 value);
	virtual void applyBeginSecondaryEffect(SABRINA::TEffectType effectType, uint32 strength);
	virtual void applyEndSecondaryEffect(SABRINA::TEffectType effectType);
	virtual void cbSabrinaActionApplyEnd();


	//------------------------------------------------------------------------------------------------------
	// Virtual Read Accessor Interface

	virtual sint32 getSkillValue(SKILLS::ESkills skill) const;
	virtual sint32 getAttackSkillValue() const;
	virtual sint32 getDefenseSkillValue() const;
	virtual sint32 getDodgeSkillValue() const;
	virtual bool getRightHandWeaponStats(CWeaponStats& stats) const;
	virtual bool getLeftHandWeaponStats(CWeaponStats& stats) const;
	virtual bool getAmmoStats(CWeaponStats& stats) const;
	virtual bool getArmorProtectionStats(SLOT_EQUIPMENT::TSlotEquipment slot,CArmorStats& protection) const;
	virtual bool getLeftHandProtectionStats(CShieldStats& protection) const;
	virtual ISabrinaActor* getTarget() const;


	//------------------------------------------------------------------------------------------------------
	// Logical test routines

	virtual bool isValidOffensiveTarget(ISabrinaActor* target) const;
	virtual bool isValidCurativeTarget(ISabrinaActor* target) const;


	//------------------------------------------------------------------------------------------------------
	// Virtual callbacks for message sending on activation/ cancelation of Sabrina actions

	virtual void cbSabrinaActionBegin(const CSabrinaPhraseInstance* completedPhrase);
	virtual void cbSabrinaActionSuccess(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code);
	virtual void cbSabrinaActionFailure(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code);
	virtual void cbSabrinaActionCancel(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code);
	virtual void cbSabrinaActionEnd(const CSabrinaPhraseInstance* completedPhrase);

private:
	//------------------------------------------------------------------------------------------------------
	// private data

	/// creature form
	const CStaticCreatures *_CreatureForm;

	/// defense value (parry or dodge)
	sint32			_AttackSkillValue;

	/// defense value (parry or dodge)
	sint32			_DefenseSkillValue;

	/// defense value (parry or dodge)
	sint32			_DodgeSkillValue;

	/// Global armor
	CArmorStats		_GlobalArmorStats;

	/// Equivalent Weapon stats
	CWeaponStats	_RightHandWeaponStats;
	CWeaponStats	_LeftHandWeaponStats;
};


#endif
