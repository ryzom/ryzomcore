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
#include "sabrina_actor_player.h"
#include "sabrina_phrase_description.h"

using namespace std;
using namespace NLMISC;

#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4355)	//	warning C4355: 'this' : used in base member initializer list
#endif

//------------------------------------------------------------------------------------------------------
// Constructor & destructor

//------------------------------------------------------------------------------------------------------
// CSabrinaActorPlayer - ctor
//------------------------------------------------------------------------------------------------------
CSabrinaActorPlayer::CSabrinaActorPlayer(CEntityBase *parent): ISabrinaActor(parent)
{
	_CurrentActionPhrase= new CSabrinaPhraseInstance(this);
}

CSabrinaActorPlayer::CSabrinaActorPlayer(const CSabrinaActorPlayer& other): ISabrinaActor(_Parent)
{
	_CurrentActionPhrase= new CSabrinaPhraseInstance(this);
	_MemoryBanks		= other._MemoryBanks;
	_ActiveMemoryBank	= other._ActiveMemoryBank;	
	_CyclicActionFlag	= other._CyclicActionFlag;
}

//------------------------------------------------------------------------------------------------------
// ~CSabrinaActorPlayer - dtor
//------------------------------------------------------------------------------------------------------
CSabrinaActorPlayer::~CSabrinaActorPlayer()
{
	// NOTE: we let the smart pointer delete the _CurrentActionPhrase when its not refferenced any more
}


//------------------------------------------------------------------------------------------------------
// Management of the actor's own actions

//------------------------------------------------------------------------------------------------------
// cancelSabrinaAction - dtor
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::beginSabrinaAction(const ISabrinaPhraseModel* phrase)
{
	_CurrentActionPhrase->beginPhrase(phrase);
}

//------------------------------------------------------------------------------------------------------
// cancelSabrinaAction - dtor
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cancelSabrinaAction(SABRINA::TEventCode reason)
{
	_CurrentActionPhrase->abortPhrase(reason);
}


//------------------------------------------------------------------------------------------------------
// Virtual application of results of a Sabrina Action to a target

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionApplyBegin
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionApplyBegin()
{
// TODO
// callback at start of application of a set of effects from a sabrina action
// eg: setup an event report record here
}

//------------------------------------------------------------------------------------------------------
// applyHeal
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::applyHeal(SABRINA::THealType healType, uint32 healQuantity)
{
// TODO
// apply damage (to hp, sap or sta)
}

//------------------------------------------------------------------------------------------------------
// applyDamage
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::applyDamage(SABRINA::TDmgType damageType, uint32 damage)
{
// TODO
// apply damage (to hp, sap or sta)
}

//------------------------------------------------------------------------------------------------------
// applyBeginSecondaryEffect
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::applyBeginSecondaryEffect(SABRINA::TEffectType effectType, uint32 strength)
{
// TODO
// apply secondary effects (eg Stun, debuff, etc)
}

//------------------------------------------------------------------------------------------------------
// applyEndSecondaryEffect
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::applyEndSecondaryEffect(SABRINA::TEffectType effectType)
{
// TODO
// apply secondary effects (eg Stun, etc)
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionApplyEnd
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionApplyEnd()
{
// TODO
// callback at end of application of a set of effects from a sabrina action
// eg: dispatch the en=vent report record now that complete set of event elements have been dealt with
}


//------------------------------------------------------------------------------------------------------
// Virtual Read Accessor Interface

//------------------------------------------------------------------------------------------------------
// getSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorPlayer::getSkillValue(SKILLS::ESkills skill) const
{
	return 10;// TODO
// lookup the value of a given skill
}

//------------------------------------------------------------------------------------------------------
// getAttackSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorPlayer::getAttackSkillValue() const
{
	return 10;// TODO
// get a value for the attack skill
}

//------------------------------------------------------------------------------------------------------
// getDefenseSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorPlayer::getDefenseSkillValue() const
{
	return 10;// TODO
// get a value for the defense skill
}

//------------------------------------------------------------------------------------------------------
// getDodgeSkillValue
//------------------------------------------------------------------------------------------------------
sint32 CSabrinaActorPlayer::getDodgeSkillValue() const
{
	return false;// TODO
// get a value for the didge skill
}

//------------------------------------------------------------------------------------------------------
// getRightHandWeaponStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::getRightHandWeaponStats(CWeaponStats& stats) const
{
	return false;// TODO
// get the combat stats for the item in right hand
// note that in the case of bear hand combat a set of bear hand stats are filled in here
// returns false if an item that is not usable for combat is present in right hand
}

//------------------------------------------------------------------------------------------------------
// getLeftHandWeaponStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::getLeftHandWeaponStats(CWeaponStats& stats) const
{
	return false;// TODO
// get the combat stats for the item in left hand
// returns false if the left hand is empty or the item in left hand is not a weapon or the player
// doesn't have a skill permitting use of the given weapon in the left hand
}

//------------------------------------------------------------------------------------------------------
// getAmmoStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::getAmmoStats(CWeaponStats& stats) const
{
	return false;// TODO
// get stats for ammo in left hand
// returns false if no ammo available or ammo is incompatible with weapon
}

//------------------------------------------------------------------------------------------------------
// getArmorProtectionStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::getArmorProtectionStats(SLOT_EQUIPMENT::TSlotEquipment slot,CArmorStats& protection) const
{
	return false;// TODO
// get stats for a given piece of armor
// in the case where no armor is found the stats are initialised accordingly
// returns false if no armor is present on given location
}

//------------------------------------------------------------------------------------------------------
// getLeftHandProtectionStats
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::getLeftHandProtectionStats(CShieldStats& protection) const
{
	return false;// TODO
// get stats for the shield	(or likewise)
// in the case where no shield is found the stats are initalised accordingly
// returns false if the left hand is empty or the left hand item offers no protection
}

//------------------------------------------------------------------------------------------------------
// getTarget
//------------------------------------------------------------------------------------------------------
ISabrinaActor* CSabrinaActorPlayer::getTarget() const
{
	if (_Parent->getTarget()==NULL)
		return NULL;
	return _Parent->getTarget()->getSabrinaActor();
}


//------------------------------------------------------------------------------------------------------
// Logical test routines

//------------------------------------------------------------------------------------------------------
// isValidOffensiveTarget
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::isValidOffensiveTarget(ISabrinaActor* target) const
{
	return true;// TODO
// make sure that I'm allowed to perform offensive actions against the given target
// take into account whether target is in PvP action with me, etc, etc
}

//------------------------------------------------------------------------------------------------------
// isValidCurativeTarget
//------------------------------------------------------------------------------------------------------
bool CSabrinaActorPlayer::isValidCurativeTarget(ISabrinaActor* target) const
{
	return true;// TODO
// make sure I'm allowed to perform curative actions on target
// take into account PvP rules with neutrals not allowed to assist, etc...
}


//------------------------------------------------------------------------------------------------------
// Virtual callbacks for message sending on activation/ cancelation of Sabrina actions

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionBegin
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionBegin(const CSabrinaPhraseInstance* completedPhrase)
{
// TODO
// this routine must lock the items that are used in the phrase...
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionSuccess
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionSuccess(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code)
{
// TODO
// this routine is responsible for beginning a new sabrina action if a new action is lined up
// this routine should deal with ammo & raw material consumption if need be...
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionFailure
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionFailure(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code)
{
// TODO
// this routine is called either by 'cbSabrinaActionBegin()' or when the result of the action is to be applied
// this routine is also responsible for beginning a new sabrina action if a new action is lined up
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionCancel
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionCancel(const CSabrinaPhraseInstance* completedPhrase, SABRINA::TEventCode code)
{
// TODO
// this routine is called by cancelSabrinaAction()
// this routine is also responsible for beginning a new sabrina action if a new action is lined up
}

//------------------------------------------------------------------------------------------------------
// cbSabrinaActionCancel
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::cbSabrinaActionEnd(const CSabrinaPhraseInstance* completedPhrase)
{
// TODO
// this callback is called after the postActionTime has elapsed and the action is completely finished
}


//------------------------------------------------------------------------------------------------------
// Management of the player's phrase memory

//------------------------------------------------------------------------------------------------------
// static utility methods...
//------------------------------------------------------------------------------------------------------

static uint64 IdStringToUint64(const std::string& memoryBankName)
{
	uint32 stringLen= memoryBankName.size();
	if (stringLen>7)
	{
		nlwarning("IdIdStringToUint64(): Invalid memory bank name - should be < 8 characters: '%s'",memoryBankName.c_str());
		#ifdef NL_DEBUG
			nlstop
		#endif
		stringLen= 7;
	}

	uint64 bankId=0;
	char *dest=	(char*)&bankId;
	for (uint32 i=0;i<stringLen;++i)
		dest[i]= (memoryBankName[i]>='a' && memoryBankName[i]<='z')? memoryBankName[i]^'a'^'A': memoryBankName[i];

	return bankId;
}

static std::string IdUint64ToString(uint32 bankId)
{
	uint32 i;
	// map a string onto the bank id
	char *src= (char*)&bankId;
	std::string result;

	// copy out text from uint64 string representation and make sure there's a n ASCIIZ terminator
	for (i=0;i<8 && src[i]!=0;++i) 
	{
		result+=src[i];
	}
	if (i==8)
	{
		nlwarning("IdUint64ToString(): No ASCIIZ terminator found in string: %s",result.c_str());
		#ifdef NL_DEBUG
			nlstop
		#endif
	}

	return result;
}

//------------------------------------------------------------------------------------------------------
// addMemoryBank
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::addMemoryBank(const std::string& memoryBankName)
{
	// build the 64 bit name representation
	uint64 bankId= IdStringToUint64(memoryBankName);

	// make sure the name is unique
	#ifdef NL_DEBUG
		for (uint32 i=_MemoryBanks.size();i--;)
		{
			nlassert(_MemoryBanks[i].BankId!= bankId);
		}
	#endif

	// create the new memory bank and setup its name
	_MemoryBanks.resize(_MemoryBanks.size()+1);
	_MemoryBanks[_MemoryBanks.size()-1].BankId= bankId;
}

//------------------------------------------------------------------------------------------------------
// memorize
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::memorize(const std::string& memoryBankName, uint32 memorySlot, ISabrinaPhraseDescription* phrase)
{
	// make sure the slot is valid
	if(memorySlot>=SABRINA::SLOTS_PER_MEMORY_BANK)
	{
		nlwarning("CSabrinaActorPlayer::memorize(): Invalid memory slot (should be <%d): %d",SABRINA::SLOTS_PER_MEMORY_BANK,memorySlot);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}

	// build the 64 bit name representation
	uint64 bankId= IdStringToUint64(memoryBankName);

	// locate the memory bank in question and setup the memory slot
	for (uint32 i=_MemoryBanks.size();i--;)
	{
		if (_MemoryBanks[i].BankId==bankId)
		{
			_MemoryBanks[i].MemorySlots[memorySlot]= phrase;
			return;
		}
	}

	// if we're here it's because the memory bank couldn't be found
	nlwarning("CSabrinaActorPlayer::memorize(): Failed to locate memory bank: %s",memoryBankName.c_str());
	#ifdef NL_DEBUG
		nlstop
	#endif
}

//------------------------------------------------------------------------------------------------------
// setActiveMemoryBank
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::setActiveMemoryBank(const std::string& memoryBankName)
{
	// build the 64 bit name representation
	uint64 bankId= IdStringToUint64(memoryBankName);

	// locate the memory bank in question and setup the memory slot
	for (uint32 i=_MemoryBanks.size();i--;)
	{
		if (_MemoryBanks[i].BankId==bankId)
		{
			// record the active memory bank for later use...
			_ActiveMemoryBank= i;

			// if we're well placed for performing an action and there's no active action 
			// and tbere's a default action for the new active memory bank then start a new action
			if (_CyclicActionFlag==true && !_CurrentActionPhrase->isActive() && _MemoryBanks[i].DefaultAction!=~0u)
				executeAction(_MemoryBanks[i].DefaultAction);

			return;
		}
	}

	// if we're here it's because the memory bank couldn't be found
	nlwarning("CSabrinaActorPlayer::setActiveMemoryBank(): Failed to locate memory bank: %s",memoryBankName.c_str());
	#ifdef NL_DEBUG
		nlstop
	#endif

	// it's not obvious what to do here - just about anything we do will be wrong!
	// - maybe we should just disconnect the player because we now have a de-sync between his client 
	// and the server!
	_ActiveMemoryBank= ~0u;
}

//------------------------------------------------------------------------------------------------------
// setDefaultAction
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::setDefaultAction(uint32 memorySlot)
{
	// make sure the active memory bank is valid
	if (_ActiveMemoryBank>_MemoryBanks.size())
	{
		nlwarning("CSabrinaActorPlayer::setDefaultAction(): Failed due to invalid active memory bank: %d",_ActiveMemoryBank);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}

	// make sure the slot is valid
	if(memorySlot>=SABRINA::SLOTS_PER_MEMORY_BANK)
	{
		nlwarning("CSabrinaActorPlayer::memorize(): Invalid memory slot (should be <%d): %d",SABRINA::SLOTS_PER_MEMORY_BANK,memorySlot);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}

	// record the default action
	_MemoryBanks[_ActiveMemoryBank].DefaultAction= memorySlot;

	// if we're well placed for performing an action and there's no active action then start a new action
	if (_CyclicActionFlag==true && !_CurrentActionPhrase->isActive())
		executeAction(memorySlot);
}

//------------------------------------------------------------------------------------------------------
// executeAction
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::executeAction(uint32 memorySlot)
{
	// make sure the active memory bank is valid
	if (_ActiveMemoryBank>_MemoryBanks.size())
	{
		nlwarning("CSabrinaActorPlayer::setDefaultAction(): Failed due to invalid active memory bank: %d",_ActiveMemoryBank);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}

	// make sure the slot is valid
	if(memorySlot>=SABRINA::SLOTS_PER_MEMORY_BANK)
	{
		nlwarning("CSabrinaActorPlayer::memorize(): Invalid memory slot (should be <%d): %d",SABRINA::SLOTS_PER_MEMORY_BANK,memorySlot);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}

	// execute the phrase
	_CurrentActionPhrase->beginPhrase(_MemoryBanks[_ActiveMemoryBank].MemorySlots[memorySlot]->getPhraseModel());
}

//------------------------------------------------------------------------------------------------------
// setCyclicActionFlag
//------------------------------------------------------------------------------------------------------
void CSabrinaActorPlayer::setCyclicActionFlag(bool value)
{
	// record the value for later use..
	_CyclicActionFlag= value;

	// if ther's no active action and the 'value' is set to true then start a new cyclic action
	if (value==true && !_CurrentActionPhrase->isActive())
	{
		// make sure the active memory bank is valid
		if (_ActiveMemoryBank>_MemoryBanks.size())
		{
			nlwarning("CSabrinaActorPlayer::setCyclicActionFlag(): Failed to launch new action due to invalid active memory bank: %d",_ActiveMemoryBank);
			#ifdef NL_DEBUG
				nlstop
			#endif
			return;
		}

		// if a default action exists for the active memory bank then launch it
		if (_MemoryBanks[_ActiveMemoryBank].DefaultAction!=~0u)
			executeAction(_MemoryBanks[_ActiveMemoryBank].DefaultAction);
		else
			nlwarning("CSabrinaActorPlayer::setCyclicActionFlag(): Cyclic action flag is set to true but there is no default action for active memory bank");
	}
}



#if 0
--------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------
PLAYER
--------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------

sint32 getSkillValue( SKILLS::ESkills skill) const { return _RightHandWeapon.SkillValue; }

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

	_RightHandItem = player->getRightHandItem();
	_LeftHandItem = player->getLeftHandItem();
	_Ammos = player->getAmmoItem();

inline sint32 getSkillValue( SKILLS::ESkills skill) const
{
	CCharacter *character = PlayerManager.getChar(_RowId);
	if (!character || skill >= SKILLS::NUM_SKILLS) return 0;
	return character->getSkills()._Skills[ skill ].Current;
}

virtual sint32 getDefenseValue()
{
	CCharacter *character = PlayerManager.getChar(_RowId);
	if ( !character) 
	{
		return 0;
	}
	
	if (character->dodgeAsDefense())
	{
		return character->getSkills()._Skills[ SKILLS::SDD ].Current;
	}
	else
	{
		// get right weapon skill
		CGameItemPtr item = character->getRightHandItem();
		if (item != NULL && item->getStaticForm() != NULL && item->getStaticForm()->Family == ITEMFAMILY::MELEE_WEAPON && item->getStaticForm()->Skill < SKILLS::NUM_SKILLS)
			return character->getSkills()._Skills[ item->getStaticForm()->Skill ].Current;
		else
//				return character->getSkills()._Skills[ SKILLS::BareHandCombat ].Current; TODO; skill missing
			return 0;
	}
}


virtual void damageOnShield(uint32 dmg)
{
	CCharacter *character = PlayerManager.getChar(_RowId);
	if ( !character) 
	{
		return;
	}

	CGameItemPtr shieldPtr = character->getLeftHandItem();
	if (shieldPtr == NULL || shieldPtr->getStaticForm() == NULL || shieldPtr->getStaticForm()->Family != ITEMFAMILY::SHIELD )
		return;
	
	shieldPtr->removeHp(dmg);
}

virtual void damageOnArmor(SLOT_EQUIPMENT::TSlotEquipment slot, uint32 dmg)
{
	CCharacter *character = PlayerManager.getChar(_RowId);
	if ( !character) 
	{
		return;
	}

	CGameItemPtr armorPtr = character->getItem(INVENTORIES::equipment, slot);
	if (armorPtr == NULL || armorPtr->getStaticForm() == NULL || armorPtr->getStaticForm()->Family != ITEMFAMILY::ARMOR)
		return;
	
	armorPtr->removeHp(dmg);
}


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
//					CCombatAttackerPlayer::checkAmmoAmount
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

	if( (unsigned)armor.Skill >= SKILLS::NUM_SKILLS ) 
	{
		nlwarning("<CCombatDefenderPlayer::getArmor> armor.Skill negative !!!!");
		armor.SkillValue = 0; 
	}
	else
	{
		armor.SkillValue = character->getSkills()._Skills[ armor.Skill ].Current;
	}

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
//		CEntityPhrases : clear
//--------------------------------------------------------------
void CEntityPhrases::clear()
{
	if (CyclicAction != NULL)
	{
		delete CyclicAction;
		CyclicAction = NULL;
	}

	const TPhraseList::iterator itEnd = Fifo.end();
	for (TPhraseList::iterator it = Fifo.begin() ; it != itEnd ; ++it)
	{
		if (*it != NULL)
		{
			delete (*it);
			(*it) = NULL;
		}
	}
	Fifo.clear();
	CyclicInProgress = false;
	DefaultAttackUsed = false;
} // clear //

//--------------------------------------------------------------
//		CEntityPhrases::setCyclicAction()
//--------------------------------------------------------------
void CEntityPhrases::setCyclicAction( CSPhrase *phrase)
{
	if (!phrase)
		return;

	// test new sentence validity (if not valid, keep old sentence)
	string errorCode;
	if (phrase->evaluate(NULL) == false || phrase->validate() == false)
	{
		DEBUGLOG("<CEntityPhrases::setCyclicAction> Invalid sentence tested, error code = ");
		/*// if error code begins with '(' do not send it !
		if ( !errorCode.empty() && errorCode[0] != '(' )
		{
			// inform player
			PHRASE_UTILITIES::sendSimpleMessage( sentence->getPlayerId(), errorCode );
		}
		*/
		// delete the sentence
		delete phrase;
		return;
	}

	if ( CyclicAction != NULL)
	{
		// if in progress, set repeat mode to false and insert this sentence in front of the sentence fifo
		if ( CyclicInProgress )
		{
			Fifo.push_front(CyclicAction);
			CyclicInProgress = false;
		}
		// not in progress, simply delete it
		else
		{
			delete CyclicAction;
		}
	}

	CyclicAction = phrase;
	
	//const CBrick *rootBrick = sentence->getRootBrick();
	//if ( rootBrick != NULL && rootBrick->name() == "Default attack")
	//	DefaultAttackUsed = true;
	//else
		DefaultAttackUsed = false;
} // setCyclicAction //

//--------------------------------------------------------------
//		CEntityPhrases::stopCyclicAction()
//--------------------------------------------------------------
void CEntityPhrases::stopCyclicAction(const TDataSetRow &entityRowId)
{
	if ( CyclicAction != NULL)
	{
		CCharacter *character = PlayerManager.getChar(entityRowId);
		// if in progress, set repeat mode to false and insert this sentence in front of the sentence fifo
		if ( CyclicInProgress )
		{
			Fifo.push_front(CyclicAction);
			CyclicInProgress = false;
			DefaultAttackUsed = false;			
		}
		// not in progress, simply delete it
		else
		{
			delete CyclicAction;
		}

		CyclicAction = NULL;

		if (character)
		{
			character->writeCycleCounterInDB();
		}
	}
} // stopCyclicAction //


//--------------------------------------------------------------
//		CEntityPhrases::createDefaultAttackIfCombat()
//--------------------------------------------------------------
void CEntityPhrases::createDefaultAttackIfCombat( const TDataSetRow &actingEntityRowId )
{
	const CEntityId &actingEntityId = TheDataset.getEntityId(actingEntityRowId);

	if (CyclicAction != NULL)
		return;

	if (DefaultAttackUsed)
		return;

	// check entity has engaged a combat
	TDataSetRow targetRowId = PhraseManager->getEntityEngagedMeleeBy(actingEntityRowId);
	if ( !targetRowId.isValid() || !TheDataset.isDataSetRowStillValid(targetRowId) )
	{
		targetRowId = PhraseManager->getEntityEngagedRangeBy(actingEntityRowId);
	}

	if ( !targetRowId.isValid() || !TheDataset.isDataSetRowStillValid(targetRowId))
		return;
	
	// create new sentence
	DEBUGLOG("Create default attack for entity %s", actingEntityId.toString().c_str() );

	vector<CSheetId>	bricks;
	bricks.push_back( CSheetId("test_default_attack.sbrick") );
	CSPhrase *phrase = PhraseManager->buildSabrinaPhrase( actingEntityRowId, targetRowId, bricks );
	if (phrase)
	{
		CyclicAction = phrase;
	}
	else
		nlwarning("<CEntityPhrases::createDefaultAttackIfCombat> Failed to create default attack phrase for entity %s",actingEntityId.toString().c_str() );

	DefaultAttackUsed = true;
} // createDefaultAttackIfCombat //


//--------------------------------------------------------------
//		CEntityPhrases::cancelAllPhrasesButFirstOne()
//--------------------------------------------------------------
void CEntityPhrases::cancelAllPhrasesButFirstOne()
{
	if (CyclicInProgress)
	{
		TPhraseList::iterator it;
		for (it = Fifo.begin() ; it != Fifo.end() ; ++it)
		{
			// delete the sentence
			if ( (*it) != NULL )
				delete (*it);
		}
		
		// clear every sentences
		Fifo.clear();
	}
	else
	{
		if (CyclicAction)
		{
			delete CyclicAction;
			CyclicAction = NULL;				
		}

		// skip the first sentence if any
		TPhraseList::iterator it = Fifo.begin();
		TPhraseList::iterator itSec;
		if (it != Fifo.end())
			itSec = ++it;
		else
			return;

		// clear every other sentences
		for ( ; it != Fifo.end() ; ++it)
		{
			// remove the sentence
			if ( (*it) != NULL )
				delete (*it);
		}
		
		// clear every sentences
		Fifo.erase(itSec, Fifo.end());
	}
} // CEntityPhrases::cancelAllPhrasesButFirstOne //

//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
void CEntityPhrases::cancelTopSentence(bool staticOnly)
{
	if (CyclicInProgress && CyclicAction && (!staticOnly || CyclicAction->isStatic()) )
	{
		CyclicAction->stop();
		delete CyclicAction;
		CyclicAction = NULL;
	}
	else
	{
		TPhraseList::iterator it = Fifo.begin();
		if (it != Fifo.end())
		{
			if ( ((*it) != NULL) && (!staticOnly || (*it)->isStatic()))
			{
				(*it)->stop();
				delete (*it);
				Fifo.pop_front();
			}			
		}
	}
} // CEntityPhrases::cancelTopSentence //

//--------------------------------------------------------------
//		CEntityPhrases::cancelTopSentence()
//--------------------------------------------------------------
bool CEntityPhrases::addPhraseFifo( CSPhrase *phrase)
{
	if (!phrase)
		return false;

	// check the fifo queue isn't already at max size
	if ( Fifo.size() >= _QueueMaxSize )
	{			
		if ( FIFOFullReplaceOrDiscard == true)
		{
			if ( (*Fifo.begin()) != 0)
			{
				DEBUGLOG("<CEntityPhrases::addPhraseFifo> FIFO is full (contains %u elts), delete 1st phrase and push new one",_QueueMaxSize );
				delete (*Fifo.begin());
			}
			else
			{
				DEBUGLOG("<CEntityPhrases::addPhraseFifo> FIFO is full (contains %u elts), 1st phrase is NULL push new one",_QueueMaxSize );
			}

			Fifo.pop_front();
		}
		else
		{
			DEBUGLOG("<CEntityPhrases::addPhraseFifo> FIFO is full (contains %u elts), do not add new phrase",_QueueMaxSize );
			delete phrase;
			return false;
		}
	}

	Fifo.push_back(phrase);
	
	return true;
} // CEntityPhrases::cancelTopSentence //


//-----------------------------------------------
//			clearMeleeEngagedEntities()
//-----------------------------------------------
void CPhraseManager::clearMeleeEngagedEntities()
{ 
	TRowRowMap::const_iterator it;
	const TRowRowMap::const_iterator itEnd = _MapEntityToEngagedEntityInMeleeCombat.end();
	for (it = _MapEntityToEngagedEntityInMeleeCombat.begin() ; it != itEnd ; ++it)
	{
		// cancel all combat sentences for that entity
		cancelAllCombatSentences( (*it).first, false );
	}

	_MapEntityToMeleeAggressors.clear();
	
	_MapEntityToEngagedEntityInMeleeCombat.clear();
} // clearMeleeEngagedEntities //


//-----------------------------------------------
//			defaultAttackSabrina()
//-----------------------------------------------
void CPhraseManager::defaultAttackSabrina( const TDataSetRow &attackerRowId, const TDataSetRow &targetRowId )
{
	CEntityId attackerId = TheDataset.getEntityId(attackerRowId);
	CEntityId targetId = TheDataset.getEntityId(targetRowId);

	DEBUGLOG("<CPhraseManager::attacks> entity %d attacks entity %d", attackerId.toString().c_str(), targetId.toString().c_str() );

	CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( attackerId );
	
	if (entity == NULL)
	{
		nlwarning("<CPhraseManager::attacks> Invalid entity Id %s", attackerId.toString().c_str() );
		return;
	}

	// cancel entity static action
	if ( entity->getId().getType() == RYZOMID::player ) 
	{
		( (CCharacter*)entity)->cancelStaticActionInProgress();
	}
	entity->cancelStaticEffects();

	// check if attacker is already in combat
	CEntityId id ;// = getEntityEngagedMeleeBy( attackerId );
	if (id == CEntityId::Unknown )
	{
		//id = getEntityEngagedRangeBy( attackerId );
	}

	if (id != CEntityId::Unknown )
	{
		TMapIdToPhraseStruc::iterator it = _Phrases.find( attackerRowId );
		if (it == _Phrases.end() )
		{
			// error should not happens
			nlwarning("<CPhraseManager::defaultAttackSabrina> ERROR Cannot find entity sentences for entity Id %s but entity is engaged !, should never occurs (serious)", attackerId.toString().c_str() );
			return;
		}
		CEntityPhrases &entityPhrases = (*it).second;

		// check if the engaged target and the new one are different
		if (id != targetId)
		{
			// close first combat, program the action for disengage on end //
			entityPhrases.cancelAllPhrasesButFirstOne();
			
			if (entityPhrases.CyclicInProgress)
			{
			//	entityPhrases.CyclicAction->disengageOnEnd(true);
			}
			else
			{
				TPhraseList::iterator it = entityPhrases.Fifo.begin();
				if (it != entityPhrases.Fifo.end() && (*it) != NULL);
				{
			//		(*it)->disengageOnEnd(true);
				}
			}
		}

		vector<CSheetId>	bricks;
		bricks.push_back( CSheetId("test_default_attack.sbrick") );
		CSPhrase *phrase = CPhraseManager::buildSabrinaPhrase( attackerRowId, targetRowId, bricks );
		if (phrase)
		{
			(*it).second.setCyclicAction( phrase );
		}
		else
		{
			nlwarning("<CPhraseManager::attacks> ERROR while creating default attack sentence for entity Id %s", attackerId.toString().c_str() );
			return;
		}
	}
	else
	{
		// attacks
		vector<CSheetId>	bricks;
		bricks.push_back( CSheetId("test_default_attack.sbrick") );
		//executePhrase(attackerId, targetId, bricks);
		CSPhrase *phrase = CPhraseManager::buildSabrinaPhrase( attackerRowId, targetRowId, bricks );
		if (!phrase)
		{
			nlwarning("Error when creating default sabrina attack, cancel");
			return;
		}

		TMapIdToPhraseStruc::iterator it = _Phrases.find( attackerRowId );
		if (it == _Phrases.end() )
		{	
			// new entry
			CEntityPhrases entityPhrases;
			
			entityPhrases.setCyclicAction(phrase);
			entityPhrases.CyclicInProgress = true;
			
			_Phrases.insert( make_pair(attackerRowId, entityPhrases) );
		}
		else
		{
			CEntityPhrases &entityPhrases = (*it).second;
			entityPhrases.setCyclicAction(phrase);
		}
	}		
} // defaultAttackSabrina //

//-----------------------------------------------
//			engageMelee()
//-----------------------------------------------
void CPhraseManager::engageMelee( const TDataSetRow &entity1, const TDataSetRow &entity2 ) 
{
	//disengage from precedent combat if any, without deleting combat phrase
	disengage(entity1, true, true, false);

	_MapEntityToInitiatedCombat.insert( make_pair(entity1, CCombat(entity1, entity2, true) ) );

	_MapEntityToEngagedEntityInMeleeCombat.insert( make_pair(entity1,entity2) );

	TRowSetRowMap::iterator it = _MapEntityToMeleeAggressors.find( entity2 );


	// add the aggressor to target aggressors
	CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entity2 );
	if ( targetEntity )
	{
//		targetEntity->addAgressor( entity1 );
	}

	if (it != _MapEntityToMeleeAggressors.end() )
	{
		(*it).second.insert( entity1 );
	}
	else
	{
		set<TDataSetRow> agg;
		agg.insert( entity1 );
		_MapEntityToMeleeAggressors.insert( make_pair( entity2, agg) );
	}

	// check mode if player
	if (TheDataset.getEntityId(entity1).getType() == RYZOMID::player)
	{
		CCharacter *character = PlayerManager.getChar(entity1);
		if ( character && character->getMode() != MBEHAV::COMBAT )
			character->setMode(MBEHAV::COMBAT);
	}

	// send message to clients to indicate the new combat
	PHRASE_UTILITIES::sendEngageMessages( TheDataset.getEntityId(entity1), TheDataset.getEntityId(entity2) );
} // engageMelee //


//-----------------------------------------------
//			engageRange()
//-----------------------------------------------
void CPhraseManager::engageRange( const TDataSetRow &entity1, const TDataSetRow &entity2 ) 
{
	//disengage from precedent combat if any
	disengage(entity1, true, true, false);

	_MapEntityToInitiatedCombat.insert( make_pair(entity1, CCombat(entity1, entity2, false) ) );
	
	_MapEntityToEngagedEntityInRangeCombat.insert( make_pair(entity1,entity2) );

	// add the aggressor to target aggressors
	CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entity2 );
	if ( targetEntity )
	{
//		targetEntity->addAgressor( entity1 );
	}

	TRowSetRowMap::iterator it = _MapEntityToRangeAggressors.find( entity2 );
	if (it != _MapEntityToRangeAggressors.end() )
	{
		(*it).second.insert( entity1 );
	}
	else
	{
		set<TDataSetRow> agg;
		agg.insert( entity1 );
		_MapEntityToRangeAggressors.insert( make_pair( entity2, agg) );
	}

	// change entity mode for COMBAT
	CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( entity1 );
	if (entity == NULL)
	{
		nlwarning("<CBrickSentenceManager::engageRange> Invalid entity rowId %u", entity1.getIndex() );
		return;
	}

	// check mode if player
	if (TheDataset.getEntityId(entity1).getType() == RYZOMID::player)
	{
		CCharacter *character = PlayerManager.getChar(entity1);
		if ( character && character->getMode() != MBEHAV::COMBAT )
			character->setMode(MBEHAV::COMBAT);
	}
	//entity->setMode( MBEHAV::COMBAT );
	
	// send message to clients to indicate the new combat
	PHRASE_UTILITIES::sendEngageMessages( TheDataset.getEntityId(entity1), TheDataset.getEntityId(entity2) );
} // engageRange //


//-----------------------------------------------
//			disengage()
//-----------------------------------------------
void CPhraseManager::disengage( const TDataSetRow &entityRowId,  bool sendChatMsg, bool disengageCreature, bool cancelCombatSentence)
{
	CEntityId entityId = TheDataset.getEntityId(entityRowId);
	// only disengage players unless specified
	if (entityId.getType() != RYZOMID::player && !disengageCreature )
	{
		nlwarning("<CPhraseManager::disengage> Tried to disengage bot %s, cancel",entityId.toString().c_str() );
		return;
	}
	
	//CEntityId entityTarget;
	TDataSetRow entityTargetRowId;

	CEntityBase* entityPtr = PHRASE_UTILITIES::entityPtrFromId( entityId );
	if (!entityPtr)
	{
		//nlwarning ("<CPhraseManager::disengage> WARNING invalid entityId %s",entityId.toString().c_str() );
		return;
	}

	// if player and in mode combat, change mode to normal
	if (entityId.getType() == RYZOMID::player && entityPtr->getMode() == MBEHAV::COMBAT)
	{
		entityPtr->setMode( MBEHAV::NORMAL, false, false );
	}

	_MapEntityToInitiatedCombat.erase(entityRowId);
	
	TRowRowMap::iterator it = _MapEntityToEngagedEntityInMeleeCombat.find( entityRowId );

	// was in melee combat
	if (it != _MapEntityToEngagedEntityInMeleeCombat.end() )
	{
		 entityTargetRowId = (*it).second;
		_MapEntityToEngagedEntityInMeleeCombat.erase( entityRowId );

		DEBUGLOG("<CPhraseManager::disengage> Disengage entity Id %s from MELEE combat", entityId.toString().c_str() );

		// remove this entity from the aggressors of its previous target entity
		TRowSetRowMap::iterator itAgg = _MapEntityToMeleeAggressors.find( entityTargetRowId );
		if (itAgg != _MapEntityToMeleeAggressors.end() )
		{
			(*itAgg).second.erase( entityRowId );
			
			// if last aggressor, remove entry
			if ((*itAgg).second.empty() )
			{
				_MapEntityToMeleeAggressors.erase( itAgg );
			}
		}
		else
			nlwarning("<CPhraseManager::disengage> Error in _MapEntityToMeleeAggressors, should have found aggressor for entity %s",TheDataset.getEntityId(entityTargetRowId).toString().c_str() );

		// remove the aggressor from target aggressors
		CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entityTargetRowId );
		if ( targetEntity )
		{
			//targetEntity->removeAgressor( entityId );
		}
	}
	// was in range combat
	else
	{
		it = _MapEntityToEngagedEntityInRangeCombat.find( entityRowId );
		if (it != _MapEntityToEngagedEntityInRangeCombat.end() )
		{
			 entityTargetRowId = (*it).second;
			_MapEntityToEngagedEntityInRangeCombat.erase( entityRowId );
			DEBUGLOG("<CPhraseManager::disengage> Disengage entity Id %s from RANGE combat", entityId.toString().c_str() );		

			CEntityBase* entity = PHRASE_UTILITIES::entityPtrFromId( entityRowId );
			if (entity == NULL)
			{
				nlwarning("<CPhraseManager::disengage> Invalid entity Id %s", entityId.toString().c_str() );		
			}
			else
			{
				// change entity mode for Normal mode
				entity->setMode( MBEHAV::NORMAL, true );
			}
		}
		else
			return; // not engaged in combat

		// remove this entity from the aggressors of its previous target entity
		TRowSetRowMap::iterator itAgg = _MapEntityToRangeAggressors.find( entityTargetRowId );
		if (itAgg != _MapEntityToRangeAggressors.end() )
		{
			(*itAgg).second.erase( entityRowId );
			
			// if last aggressor, remove entry
			if ((*itAgg).second.empty() )
				_MapEntityToRangeAggressors.erase( itAgg );
		}
		else
			nlwarning("<CPhraseManager::disengage> Error in _MapEntityToRangeAggressors, should have found aggressor for entity %s",TheDataset.getEntityId(entityTargetRowId).toString().c_str() );

		// remove the aggressor from target aggressors
		CEntityBase* targetEntity = PHRASE_UTILITIES::entityPtrFromId( entityTargetRowId );
		if ( targetEntity )
		{
//			targetEntity->removeAgressor( entityId );
		}
	}
	
	INFOLOG("<CPhraseManager::disengage> Disengaging entity %s, was in combat with %s", entityId.toString().c_str(), TheDataset.getEntityId(entityTargetRowId).toString().c_str());

	if (cancelCombatSentence)
	{
		// cancel all combat sentences for that entity
		cancelAllCombatSentences( entityRowId, false);
	}

	// send message to players
	if (sendChatMsg)
		PHRASE_UTILITIES::sendDisengageMessages( entityId, TheDataset.getEntityId(entityTargetRowId));
} // disengage //

//--------------------------------------------------------------
//						cancelAllCombatSentences()  
//--------------------------------------------------------------
bool CPhraseManager::cancelAllCombatSentences( const TDataSetRow &entityRowId, bool disengageOnEndOnly)
{
	bool returnValue = true;
	
	// find the player execution list if any
	TMapIdToPhraseStruc::iterator itEntityPhrase = _Phrases.find( entityRowId );
	if ( itEntityPhrase != _Phrases.end() )
	{
		CEntityPhrases &entityPhrases = (*itEntityPhrase).second;

		// manage cyclic sentence
		if ( entityPhrases.CyclicAction != NULL /*&& entityPhrases.CyclicAction->getType() == BRICK_TYPE::COMBAT*/)
		{
			//if (entityPhrases.CyclicInProgress)
			{
				entityPhrases.stopCyclicAction(entityRowId);
			}
			/*else
			{
				entityPhrases.CyclicAction->stop();
				delete entityPhrases.CyclicAction;
				entityPhrases.CyclicAction = NULL;
				entityPhrases.CyclicInProgress = false;
				// if entity is a player, write the cyclic counter in DB
				CCharacter *character = PlayerManager.getChar(entityRowId);
				if (character)
				{
					character-writeCycleCounterInDB();
				}
			}
			*/
		}

		// non-cyclic sentence
		vector<TPhraseList::iterator> delVect;
		// get the first sentence if any
		TPhraseList &phrases = entityPhrases.Fifo;
				
		TPhraseList::iterator it = phrases.begin();
		if (it != phrases.end() /*&& (*it)->getType() == BRICK_TYPE::COMBAT*/)
		{
			CCombatPhrase *combatPhrase = dynamic_cast<CCombatPhrase*> (*it);
			if (combatPhrase != 0)
			{
				// if the sentence is currently executed, do nothing, else, erase it like all the others
				if ( combatPhrase->beingProcessed() == false && !combatPhrase->disengageOnEnd() )
				{
					combatPhrase->stop();

					// remove the sentence
					delVect.push_back(it);
					if ( combatPhrase != NULL )
						delete combatPhrase;
				}
				else if (disengageOnEndOnly)
				{
					combatPhrase->disengageOnEnd( true );
					returnValue = false;
				}

				if ( it == phrases.begin() )
				{
					CCharacter *character = PlayerManager.getChar(entityRowId);
					if (character)
					{
						character->writeExecPhraseInDB(0, combatPhrase->nextCounter());
					}
				}
				
				++it;
			}
		}

		for ( ; it != phrases.end() ; ++it)
		{
			if ( dynamic_cast<CCombatPhrase*> (*it) != 0)
			{
				// remove the sentence
				delVect.push_back(it);
				if ( (*it) != NULL )
					delete (*it);
			}
		}
		
		// clear every sentences
		vector<TPhraseList::iterator>::iterator itdel;
		for ( itdel = delVect.begin() ; itdel != delVect.end() ; ++itdel)
		{
			phrases.erase( *itdel );
		}
	}

	return returnValue;
} // cancelAllCombatSentences //


//--------------------------------------------------------------
//						breakCast()  
//--------------------------------------------------------------
void CPhraseManager::breakCast( sint32 attackSkillValue, CEntityBase * entity, CEntityBase * defender)
{
	nlassert(entity);
	nlassert(defender);
	// try to get a magic phrase being cast (it the phrase at the bginning of the queue
	TMapIdToPhraseStruc::iterator it = _Phrases.find( defender->getEntityRowId() );
	if ( it != _Phrases.end() )
	{
		if ( (*it).second.Fifo.begin() != (*it).second.Fifo.end() )
		{
			CMagicPhrase * magicPhrase = dynamic_cast< CMagicPhrase * > ( *( (*it).second.Fifo.begin() ) );
			if ( magicPhrase )
			{
				// compute average skill value of the phrase
				sint skillValue = 0;
				for ( uint i = 0; i < magicPhrase->getSkills().size(); i++ )
				{
					SSkill * skill = entity->getSkills().getSkillStruct( magicPhrase->getSkills()[i] );
					if ( skill )
					{
						skillValue+= skill->Current;
					}
					else
					{
						nlwarning("<CMagicPhrase apply> invalid skill %d",magicPhrase->getSkills()[i] );
						return;
					}
				}
				//test if the spell is broken
				const uint8 chances = PHRASE_UTILITIES::getSuccessChance( (attackSkillValue - skillValue - magicPhrase->getBreakResist() )/10 );
				const uint8 roll = (uint8) RandomGenerator.rand(99);
				float successFactor = PHRASE_UTILITIES::getSucessFactor(chances, roll);
				if ( successFactor >= 1 )
					(*it).second.cancelTopSentence();
			}
		}
	}
}// breakCast


//--------------------------------------------------------------
//						breakCast()  
//--------------------------------------------------------------
bool CPhraseManager::harvestDefault(const TDataSetRow &actorRowId, const CSheetId &rawMaterialSheet, uint16 minQuality, uint16 maxQuality, uint16 quantity, bool deposit )
{
	vector<CSheetId> bricks;
	const CSheetId quarteringBrick("root_harvest_default.sbrick");
	const CSheetId foragingBrick("root_harvest_default.sbrick");

	if (quarteringBrick == CSheetId::Unknown)
	{
		nlwarning("ERROR : cannot find quartering brick : root_harvest_default.sbrick.");
		return false;
	}
	if (foragingBrick == CSheetId::Unknown)
	{
		nlwarning("ERROR : cannot find foraging brick : root_harvest_default.sbrick.");
		return false;
	}

	if (deposit)
		bricks.push_back( foragingBrick );
	else
		bricks.push_back( quarteringBrick );
			
	TDataSetRow nullId;
	
	CSPhrase *phrase = buildSabrinaPhrase(actorRowId, nullId, bricks);
	CHarvestPhrase *harvestPhrase = dynamic_cast<CHarvestPhrase*> (phrase);
	if (!phrase)
	{
		return false;
	}
	if (!harvestPhrase)
	{
		delete phrase;
		return false;
	}

	harvestPhrase->minQuality(minQuality);
	harvestPhrase->maxQuality(maxQuality);
	harvestPhrase->quantity(quantity);
	harvestPhrase->setRawMaterial(rawMaterialSheet);
	harvestPhrase->deposit(deposit);

	TMapIdToPhraseStruc::iterator it = _Phrases.find( actorRowId );
	// actor doesn't already have phrases
	if (it == _Phrases.end() )
	{	
		// new entry
		CEntityPhrases entityPhrases;
		
		entityPhrases.addPhraseFifo(phrase);
		
		_Phrases.insert( make_pair(actorRowId, entityPhrases) );
	}
	// actor already have phrases in the manager, just add the new one
	else
	{
		CEntityPhrases &entityPhrases = (*it).second;
		entityPhrases.addPhraseFifo(phrase);
	}

	return true;
} // harvestDefault //


//--------------------------------------------------------------
//				cancelStaticActionInProgress()  
//--------------------------------------------------------------
void CPhraseManager::cancelStaticActionInProgress(const TDataSetRow &actorRowId)
{
	TMapIdToPhraseStruc::iterator it = _Phrases.find( actorRowId );
	if (it != _Phrases.end() )
	{
		(*it).second.cancelTopSentence(true);
	}
} // cancelStaticActionInProgress //

#endif
