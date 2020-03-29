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



#ifndef RY_SABRINA_ACTOR_PLAYER_H
#define RY_SABRINA_ACTOR_PLAYER_H

// nel misc
#include "nel/misc/types_nl.h"
// sabrina
#include "sabrina_pointers.h"
#include "sabrina_actor.h"


//------------------------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------------------------

namespace SABRINA
{
	const uint32 SLOTS_PER_MEMORY_BANK = 20;
}


//------------------------------------------------------------------------------------------------------
// CSabrinaActorPlayer - Player specialisation of Sabrina actor class
//------------------------------------------------------------------------------------------------------

class CSabrinaActorPlayer: public ISabrinaActor
{
public:
	//------------------------------------------------------------------------------------------------------
	// ctor and dtor
	CSabrinaActorPlayer(CEntityBase *parent);
	CSabrinaActorPlayer(const CSabrinaActorPlayer& other);
	virtual ~CSabrinaActorPlayer();

	//------------------------------------------------------------------------------------------------------
	// Virtual application of results of a Sabrina Action to a target

	virtual void cbSabrinaActionApplyBegin();
	virtual void applyHeal(SABRINA::THealType healType, uint32 healQuantity);
	virtual void applyDamage(SABRINA::TDmgType damageType, uint32 damage);
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
	// Management of the player's own actions

	virtual void beginSabrinaAction(const ISabrinaPhraseModel* phrase);
	virtual void cancelSabrinaAction(SABRINA::TEventCode reason);


	//------------------------------------------------------------------------------------------------------
	// Virtual callbacks for message sending on activation/ cancelation of Sabrina actions

	virtual void cbSabrinaActionBegin(const CSabrinaPhraseInstance* completedPhrase);
	virtual void cbSabrinaActionSuccess(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code);
	virtual void cbSabrinaActionFailure(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code);
	virtual void cbSabrinaActionCancel(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code);
	virtual void cbSabrinaActionEnd(const CSabrinaPhraseInstance* completedPhrase);


	//------------------------------------------------------------------------------------------------------
	// Cyclic action management methods

	/// set the cyclic action
	void setCyclicAction( ISabrinaPhraseModel *phrase);

	/// stop the cyclic action
	void stopCyclicAction(const TDataSetRow &entityRowId);


	//------------------------------------------------------------------------------------------------------
	// Management of the player's phrase memory

	// add memory banks (used at player connection to creat memory set)
	// note that memory bank names are limited to 7 letters
	void addMemoryBank(const std::string& memoryBankName);

	// memorize a phrase in a slot in a memory bank
	void memorize(const std::string& memoryBankName, uint32 memorySlot, ISabrinaPhraseDescription* phrase);

	// set the 'active memory bank' variable
	void setActiveMemoryBank(const std::string& memoryBankName);
	
	// set the default action within the active memory bank (~0u to reset to no default action)
	// deppending on the action type this may or may not be a cyclic action
	void setDefaultAction(uint32 memorySlot);

	// execute an action from within the active memory bank
	void executeAction(uint32 memorySlot);

	// the client and/ or server has indicated that they now (or no longer) are well placed for 
	// performing the current default action (the action must be cyclic)
	void setCyclicActionFlag(bool value);


	//------------------------------------------------------------------------------------------------------
	// Message Management for player information messages...

	// TODO...

private:
	// the player's current active action phrase
	CSabrinaPhraseInstancePtr _CurrentActionPhrase;

	// structure for the player's phrase memory...
	struct CMemoryBank
	{
		uint64										BankId;			// used as a 0..7 character asciiz string
		std::vector<ISabrinaPhraseDescriptionPtr>	MemorySlots;		//	MemorySlots[SABRINA::SLOTS_PER_MEMORY_BANK];
		uint32										DefaultAction;	// ~0u if none

		CMemoryBank():
			BankId(0),
			MemorySlots(SABRINA::SLOTS_PER_MEMORY_BANK),
			DefaultAction(~0u)
		{
		}
		// copy constructor (needed to protect smart pointers...)
		CMemoryBank(const CMemoryBank& other):
			BankId(other.BankId),
			MemorySlots(other.MemorySlots),
			DefaultAction(other.DefaultAction)
		{
		}
	};
	// the player's set of memory banks (one per right hand item type)
	std::vector<CMemoryBank> _MemoryBanks;
	// index of the active memory bank (within _MemoryBanks)
	uint32 _ActiveMemoryBank;	

	// a flag used to determine whether or not to launch a new action automatically when the
	// current action terminates
	bool _CyclicActionFlag;		// set to true if the client is well placed and default action is cyclic
};


#endif
