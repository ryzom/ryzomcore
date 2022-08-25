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



//#include "stdpch.h"
// nel
#include "nel/misc/debug.h"
// sabrina
#include "sabrina_actor_creature.h"

using namespace std;
using namespace NLMISC;


//------------------------------------------------------------------------------------------------------
// Constructor & destructor

//------------------------------------------------------------------------------------------------------
// CSabrinaActorCreature - ctor
//------------------------------------------------------------------------------------------------------
CSabrinaActorCreature::CSabrinaActorCreature(CEntityBase *parent):
	ISabrinaActor(parent)
{
	#ifdef NL_DEBUG
		nlassert(parent!=NULL);
	#endif

	_CreatureForm = CSheets::getCreaturesForm(parent->getSheetId());
	if ( _CreatureForm==NULL )
	{
		nlwarning( "CSabrinaActorCreature::CSabrinaActorCreature(): invalid creature form %s in entity %s", 
			parent->getSheetId().toString().c_str(), parent->getId().toString().c_str() );
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}
	
	_GlobalArmorStats.setQuality( _CreatureForm->Level );

	for (int i=0;i<DMGTYPE::NBTYPES;++i)
	{
		_GlobalArmorStats.setProtectionFactor(i,_CreatureForm->Protections[i].Factor);
		_GlobalArmorStats.setProtectionLimit(i,_CreatureForm->Protections[i].Max);
	}

	_GlobalArmorStats.setSkill( SKILLS::unknown );
	_GlobalArmorStats.setArmorType( ARMORTYPE::UNKNOWN );	// TODO: FIXME
	
	_DefenseSkillValue= _CreatureForm->Level * 10;	// TODO: FIXME
	_AttackSkillValue= _CreatureForm->Level * 10;	// TODO: FIXME

	_RightHandWeaponStats.setQuality( _CreatureForm->Level );											// TODO: FIXME
	_RightHandWeaponStats.setDamage( _CreatureForm->CreatureDamagePerHit * 260/(10+10*_CreatureForm->Level) );	// TODO: FIXME

	_RightHandWeaponStats.setDmgType( DMGTYPE::SLASHING );		// TODO: FIXME
	_RightHandWeaponStats.setSpeedInTicks( 30 );					// TODO: FIXME
	_RightHandWeaponStats.setFamily( ITEMFAMILY::MELEE_WEAPON );
	_RightHandWeaponStats.setSkill( SKILLS::SFM1H );				// TODO: FIXME: WHY SFM1H????

//	INFOLOG("AttackerAi :_RightHandWeaponStats.Quality = %u, _RightHandWeaponStats.Damage = %u", 
//		_RightHandWeaponStats.getQuality(), _RightHandWeaponStats.getDamage() );
}

CSabrinaActorCreature::CSabrinaActorCreature(const CSabrinaActorCreature& other):
	ISabrinaActor(_Parent)
{
	_CreatureForm			= other._CreatureForm;
	_AttackSkillValue		= other._AttackSkillValue;
	_DefenseSkillValue		= other._DefenseSkillValue;
	_DodgeSkillValue		= other._DodgeSkillValue;
	_GlobalArmorStats		= other._GlobalArmorStats;
	_RightHandWeaponStats	= other._RightHandWeaponStats;
	_LeftHandWeaponStats	= other._LeftHandWeaponStats;
}

//------------------------------------------------------------------------------------------------------
// ~CSabrinaActorCreature - dtor
//------------------------------------------------------------------------------------------------------
CSabrinaActorCreature::~CSabrinaActorCreature()
{
}

//------------------------------------------------------------------------------------------------------
// Management of the actor's own actions

//------------------------------------------------------------------------------------------------------
// cancelSabrinaAction - dtor
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::beginSabrinaAction(const ISabrinaPhraseModel* phrase)
{
// TODO
//	_CurrentActionPhrase->beginPhrase(phrase);
}

//------------------------------------------------------------------------------------------------------
// cancelSabrinaAction - dtor
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cancelSabrinaAction(SABRINA::TEventCode reason)
{
// TODO
//	_CurrentActionPhrase->abortPhrase(reason);
}


//------------------------------------------------------------------------------------------------------
// Virtual application of results of a Sabrina Action to a target

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionApplyBegin
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionApplyBegin()
{
// callback at start of application of a set of effects from a sabrina action
// eg: setup an event report record here
}

//------------------------------------------------------------------------------------------------------
// applyHeal
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::applyHeal(SABRINA::THealType healType,uint32 value)
{
// apply damage (to hp, sap or sta)
}

//------------------------------------------------------------------------------------------------------
// applyDamage
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::applyDamage(SABRINA::TDmgType damageType, uint32 value)
{
// apply damage (to hp, sap or sta)
}

//------------------------------------------------------------------------------------------------------
// applyBeginSecondaryEffect
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::applyBeginSecondaryEffect(SABRINA::TEffectType effectType, uint32 strength)
{
// apply secondary effects (eg Stun, debuff, etc)
}

//------------------------------------------------------------------------------------------------------
// applyEndSecondaryEffect
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::applyEndSecondaryEffect(SABRINA::TEffectType effectType)
{
// apply secondary effects (eg Stun, etc)
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionApplyEnd
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionApplyEnd()
{
// callback at end of application of a set of effects from a sabrina action
// eg: dispatch the en=vent report record now that complete set of event elements have been dealt with
}


//------------------------------------------------------------------------------------------------------
// Virtual Read Accessor Interface

//------------------------------------------------------------------------------------------------------
// getSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorCreature::getSkillValue(SKILLS::ESkills skill) const
{
	// lookup the value of a given skill
	nlwarning("Attempt to access specific skill '%s' for a creature - returning attack skill",SKILLS::toString(skill).c_str());
	return _AttackSkillValue;
}

//------------------------------------------------------------------------------------------------------
// getAttackSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorCreature::getAttackSkillValue() const
{
	// get a value for the attack skill
	return _AttackSkillValue;
}

//------------------------------------------------------------------------------------------------------
// getDefenseSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorCreature::getDefenseSkillValue() const
{
	// get a value for the defense skill
	return _DefenseSkillValue;
}

//------------------------------------------------------------------------------------------------------
// getDodgeSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorCreature::getDodgeSkillValue() const
{
	// get a value for the didge skill
	return _DodgeSkillValue;
}

//------------------------------------------------------------------------------------------------------
// getRightHandWeaponStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::getRightHandWeaponStats(CWeaponStats& stats) const
{
	// get the combat stats for the item in right hand
	// note that in the case of bear hand combat a set of bear hand stats are filled in here
	// returns false if an item that is not usable for combat is present in right hand
	stats=_RightHandWeaponStats;
	return true;
}

//------------------------------------------------------------------------------------------------------
// getLeftHandWeaponStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::getLeftHandWeaponStats(CWeaponStats& stats) const
{
	// get the combat stats for the item in left hand
	// returns false if the left hand is empty or the item in left hand is not a weapon or the player
	// doesn't have a skill permitting use of the given weapon in the left hand
	stats=_LeftHandWeaponStats;
	return true;
}

//------------------------------------------------------------------------------------------------------
// getAmmoStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::getAmmoStats(CWeaponStats& stats) const
{
	// get stats for ammo in left hand
	// returns false if no ammo available or ammo is incompatible with weapon
	stats=_LeftHandWeaponStats;
	return true;
}

//------------------------------------------------------------------------------------------------------
// getArmorProtectionStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::getArmorProtectionStats(SLOT_EQUIPMENT::TSlotEquipment slot,CArmorStats& protection) const
{
	// get stats for a given piece of armor
	// in the case where no armor is found the stats are initialised accordingly
	// returns false if no armor is present on given location
	protection=_GlobalArmorStats;
	return true;
}

//------------------------------------------------------------------------------------------------------
// getLeftHandProtectionStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::getLeftHandProtectionStats(CShieldStats& protection) const
{
	// get stats for the shield	(or likewise)
	// in the case where no shield is found the stats are initalised accordingly
	// returns false if the left hand is empty or the left hand item offers no protection
	protection=CShieldStats();
	return false;
}

//------------------------------------------------------------------------------------------------------
// getTarget
//------------------------------------------------------------------------------------------------------
ISabrinaActor* CSabrinaActorCreature::getTarget() const
{
	// get a pointer to the entity refferenced by this entity
	return NULL;
}


//------------------------------------------------------------------------------------------------------
// Logical test routines

//------------------------------------------------------------------------------------------------------
// isValidOffensiveTarget
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::isValidOffensiveTarget(ISabrinaActor* target) const
{
	// make sure that I'm allowed to perform offensive actions against the given target
	// take into account whether target is in PvP action with me, etc, etc
	return true;
}

//------------------------------------------------------------------------------------------------------
// isValidCurativeTarget
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorCreature::isValidCurativeTarget(ISabrinaActor* target) const
{
	// make sure I'm allowed to perform curative actions on target
	// take into account PvP rules with neutrals not allowed to assist, etc...
	return true;
}


//------------------------------------------------------------------------------------------------------
// Virtual callbacks for message sending on activation/ cancelation of Sabrina actions

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionBegin
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionBegin(const CSabrinaPhraseInstance* completedPhrase)
{
	// this routine must lock the items that are used in the phrase...
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionSuccess
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionSuccess(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code)
{
	// this routine is responsible for beginning a new sabrina action if a new action is lined up
	// this routine should deal with ammo & raw material consumption if need be...
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionFailure
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionFailure(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code)
{
	// this routine is called either by 'cbSabrinaActionBegin()' or when the result of the action is to be applied
	// this routine is also responsible for beginning a new sabrina action if a new action is lined up
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionCancel
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionCancel(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code)
{
	// this routine is called by cancelSabrinaAction()
	// this routine is also responsible for beginning a new sabrina action if a new action is lined up
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionCancel
//------------------------------------------------------------------------------------------------------
void CSabrinaActorCreature::cbSabrinaActionEnd(const CSabrinaPhraseInstance* completedPhrase)
{
// this callback is called after the postActionTime has elapsed and the action is completely finished
}

