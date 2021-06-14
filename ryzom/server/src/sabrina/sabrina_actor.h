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



#ifndef RY_SABRINA_ACTOR_H
#define RY_SABRINA_ACTOR_H

// nel misc
#include "nel/misc/types_nl.h"
// game_share
#include "game_share/base_types.h"
// entity_game_service
#include "entity_base.h"
// Sabrina
#include "sabrina_phrase_instance.h"
#include "sabrina_item_stats.h"
#include "sabrina_pointers.h"
//
#include "position_manager/sabrina_list_link.h"

class ISabrinaActor;
typedef CListLink<ISabrinaActor> TSabrinaActorLink;

/**
 * Sabrina actor base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class ISabrinaActor
{
public:
	//------------------------------------------------------------------------------------------------------
	// ctor and dtor
	ISabrinaActor(CEntityBase *parent);
	ISabrinaActor(const ISabrinaActor& other);
	virtual ~ISabrinaActor();
	// ctor 
//	explicit ISabrinaActor(CEntityBase *parent): _Link(this), _Parent(parent), _CurrentActionPhrase(NULL) {}
	// dtor 
//	virtual ~ISabrinaActor() { resetRefferences(); }

	// reset all refferences to this actor (eg after death, teleportation, etc) 
	void resetRefferences(SABRINA::TEventCode reason);
	

	//------------------------------------------------------------------------------------------------------
	// Generic Read Accessor Interface

	CEntityBase*					getEntity()			const;
	TDataSetRow						getEntityRowId()	const;
	const std::set<CSabrinaPhraseInstancePtr>& getTargeters() const;


	//------------------------------------------------------------------------------------------------------
	// Management of actor's targeter list

	void addTargeter(CSabrinaPhraseInstance* targeter);
	void removeTargeter(CSabrinaPhraseInstance* targeter);


	//------------------------------------------------------------------------------------------------------
	// Management of the actor's own actions

	// begin a new sabrina action
	// if there is a curent action already in process then the new action is ignored and a warning is displayed
	virtual void beginSabrinaAction(const ISabrinaPhraseModel* phrase)=0;

	// cancel the current sabrina action
	virtual void cancelSabrinaAction(SABRINA::TEventCode reason)=0;


	//------------------------------------------------------------------------------------------------------
	// Debug stuff

	// get a string describing current values of state variables
	std::string stateString() const;


	//------------------------------------------------------------------------------------------------------
	// Virtual application of results of a Sabrina Action to a target

	// callback at start of application of a set of effects from a sabrina action
	// eg: setup an event report record here
	virtual void cbSabrinaActionApplyBegin() {}

	// apply damage (to hp, sap or sta)
	virtual void applyHeal(SABRINA::THealType healType, uint32 healQuantity)=0;

	// apply damage (to hp, sap or sta)
	virtual void applyDamage(SABRINA::TDmgType damageType, uint32 damage)=0;

	// apply secondary effects (eg Stun, debuff, etc)
	virtual void applyBeginSecondaryEffect(SABRINA::TEffectType effectType, uint32 strength)=0;

	// apply secondary effects (eg Stun, etc)
	virtual void applyEndSecondaryEffect(SABRINA::TEffectType effectType)=0;

	// callback at end of application of a set of effects from a sabrina action
	// eg: dispatch the en=vent report record now that complete set of event elements have been dealt with
	virtual void cbSabrinaActionApplyEnd() {}


	//------------------------------------------------------------------------------------------------------
	// Virtual Read Accessor Interface

	// lookup the value of a given skill
	virtual sint32 getSkillValue(SKILLS::ESkills skill) const=0;

	// get a value for the attack skill
	virtual sint32 getAttackSkillValue() const=0;

	// get a value for the defense skill
	virtual sint32 getDefenseSkillValue() const=0;

	// get a value for the didge skill
	virtual sint32 getDodgeSkillValue() const=0;

	// get the combat stats for the item in right hand
	// note that in the case of bear hand combat a set of bear hand stats are filled in here
	// returns false if an item that is not usable for combat is present in right hand
	virtual bool getRightHandWeaponStats(CWeaponStats& stats) const=0;

	// get the combat stats for the item in left hand
	// returns false if the left hand is empty or the item in left hand is not a weapon or the player
	// doesn't have a skill permitting use of the given weapon in the left hand
	virtual bool getLeftHandWeaponStats(CWeaponStats& stats) const=0;

	// get stats for ammo in left hand
	// returns false if no ammo available or ammo is incompatible with weapon
	virtual bool getAmmoStats(CWeaponStats& stats) const=0;

	// get stats for a given piece of armor
	// in the case where no armor is found the stats are initialised accordingly
	// returns false if no armor is present on given location
	virtual bool getArmorProtectionStats(SLOT_EQUIPMENT::TSlotEquipment slot,CArmorStats& protection) const=0;

	// get stats for the shield	(or likewise)
	// in the case where no shield is found the stats are initalised accordingly
	// returns false if the left hand is empty or the left hand item offers no protection
	virtual bool getLeftHandProtectionStats(CShieldStats& protection) const=0;

	// get a pointer to the entity refferenced by this entity
	virtual ISabrinaActor* getTarget() const=0;


	//------------------------------------------------------------------------------------------------------
	// Logical test routines

	// make sure that I'm allowed to perform offensive actions against the given target
	// take into account whether target is in PvP action with me, etc, etc
	virtual bool isValidOffensiveTarget(ISabrinaActor* target) const=0;

	// make sure I'm allowed to perform curative actions on target
	// take into account PvP rules with neutrals not allowed to assist, etc...
	virtual bool isValidCurativeTarget(ISabrinaActor* target) const=0;


	//------------------------------------------------------------------------------------------------------
	// Virtual callbacks for message sending on activation/ cancelation of Sabrina actions

	// * if this routine should lock the items that are used in the phrase then we need
	//   to add a 'locked items' module to the phrase instance in order to facilitate
	//   unlocking ...
	virtual void cbSabrinaActionBegin(const CSabrinaPhraseInstance* completedPhrase) {}

	// this routine should deal with ammo & raw material consumption if need be...
	virtual void cbSabrinaActionSuccess(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code) {}

	// this routine is called either by 'cbSabrinaActionBegin()' or when the result of the action is to be applied
	virtual void cbSabrinaActionFailure(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code) {}

	// this routine is called by cancelSabrinaAction()
	virtual void cbSabrinaActionCancel(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode reason) {}

	// this callback is called after the postActionTime has elapsed and the action is completely finished
	// this routine is responsible for beginning a new sabrina action if a new action is lined up
	virtual void cbSabrinaActionEnd(const CSabrinaPhraseInstance* completedPhrase) {}


protected:
	//------------------------------------------------------------------------------------------------------
	// private data

	// the player or ai entity base object that I represent
	CEntityBase*							_Parent;

	// the vector of phrases that are currently active on me
	std::set<CSabrinaPhraseInstancePtr>		_PhrasesTargetingMe;

	// structure used to chain to other sabrina actors (used by the position manager)
	TSabrinaActorLink						_Link;
};


//------------------------------------------------------------------------------------------------------
// ctors and dtors

inline ISabrinaActor::ISabrinaActor(CEntityBase* parent)
{
	_Parent=parent; 
}

inline ISabrinaActor::ISabrinaActor(const ISabrinaActor& other)
{
	_Parent					= other._Parent;
	_PhrasesTargetingMe		= other._PhrasesTargetingMe;
}

inline ISabrinaActor::~ISabrinaActor() 
{
	resetRefferences(SABRINA::PlayerLoggedOff); 
}


//------------------------------------------------------------------------------------------------------
// reset all refferences to this actor (eg after death, teleportation, etc) 

inline void ISabrinaActor::resetRefferences(SABRINA::TEventCode reason)
{
	// abort all active actinos that refference us
	while (!_PhrasesTargetingMe.empty())
	{
		// note that the abortAction() method will end up removing the entry from _PhrasesTargettingMe
		(*_PhrasesTargetingMe.begin())->abortPhrase(reason);
	}
}


//------------------------------------------------------------------------------------------------------
// Generic Read Accessor Interface

inline CEntityBase* ISabrinaActor::getEntity() const 
{
	return _Parent; 
}

inline TDataSetRow ISabrinaActor::getEntityRowId() const 
{
	return _Parent->getEntityRowId(); 
}

inline const std::set<CSabrinaPhraseInstancePtr>& ISabrinaActor::getTargeters() const 
{ 
	return _PhrasesTargetingMe; 
}


//------------------------------------------------------------------------------------------------------
// Management of actor's targeter list

inline void ISabrinaActor::addTargeter(CSabrinaPhraseInstance* targeter) 
{
	_PhrasesTargetingMe.insert(targeter); 
}

inline void ISabrinaActor::removeTargeter(CSabrinaPhraseInstance* targeter) 
{
	_PhrasesTargetingMe.erase(targeter); 
}


//------------------------------------------------------------------------------------------------------
// Debug stuff

inline std::string ISabrinaActor::stateString()	const
{
	std::string result;

	result+= "id "+ NLMISC::toString(this->getEntityRowId());
	if (getTarget()!=NULL)
		result+= ", tgt "+NLMISC::toString(this->getTarget()->getEntityRowId());
	else
		result+= ", tgt NULL";

	result+= ", tgtBy(";
	std::set<CSabrinaPhraseInstancePtr>::iterator it;
	for (it=getTargeters().begin();it!=getTargeters().end();++it)
	{
		if (it!=getTargeters().begin())
			result+=",";
		result+=NLMISC::toString((*it)->getActor()->getEntityRowId());
	}
	result+= ")";

	result+= ", SkillAtk "+ NLMISC::toString(this->getAttackSkillValue());
	result+= ", SkillDef "+ NLMISC::toString(this->getDefenseSkillValue());
	result+= ", SkillDdg "+ NLMISC::toString(this->getDodgeSkillValue());

	return result;
}


//------------------------------------------------------------------------------------------------------
#endif
