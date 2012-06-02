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

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdpch.h"

// NeL MISC

// Local
#include "character.h"
#include "phrase_manager/s_effect.h"
#include "persistent_effect.h"
#include "phrase_manager/mod_magic_protection_effet.h"
#include "phrase_manager/chg_charac_effect.h"
#include "phrase_manager/enchant_weapon_effect.h"
#include "phrase_manager/power_shielding_effect.h"
#include "phrase_manager/mod_defense_effect.h"
#include "phrase_manager/mod_craft_success_effect.h"
#include "phrase_manager/mod_melee_success_effect.h"
#include "phrase_manager/mod_range_success_effect.h"
#include "phrase_manager/mod_magic_success_effect.h"
#include "phrase_manager/mod_forage_success_effect.h"
#include "phrase_manager/aura_effect.h"
#include "phrase_manager/bleed_effect.h"
#include "phrase_manager/bounce_effect.h"
#include "phrase_manager/change_move_speed_effect.h"
#include "phrase_manager/damage_aura_effect.h"
#include "phrase_manager/damage_shield_effect.h"
#include "phrase_manager/nolink_dot_effect.h"
#include "phrase_manager/nolink_hot_effect.h"
#include "phrase_manager/redirect_attacks_effect.h"
#include "phrase_manager/regen_modifier_effect.h"
#include "phrase_manager/shoot_again_effect.h"
#include "phrase_manager/simple_effect.h"
#include "phrase_manager/stun_effect.h"



//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// methods CPersistentEffect
//-------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CPersistentEffect::CPersistentEffect(CCharacter* theCharacter)
{
	_TheCharacter= theCharacter;

	static bool intialized = false;
	if(intialized == false)
	{
		intialized = true;
		NLMISC_REGISTER_CLASS(CAuraRootEffect)
		NLMISC_REGISTER_CLASS(CAuraBaseEffect)
		NLMISC_REGISTER_CLASS(CBleedEffect)
		NLMISC_REGISTER_CLASS(CBounceEffect)
		NLMISC_REGISTER_CLASS(CChangeMoveSpeedEffect)
		NLMISC_REGISTER_CLASS(CChgCharacEffect)
		NLMISC_REGISTER_CLASS(CDamageAuraEffect)
		NLMISC_REGISTER_CLASS(CDamageShieldEffect)
		NLMISC_REGISTER_CLASS(CModCraftSuccessEffect)
		NLMISC_REGISTER_CLASS(CModDefenseEffect)
		NLMISC_REGISTER_CLASS(CModForageSuccessEffect)
		NLMISC_REGISTER_CLASS(CModMagicProtectionEffect)
		NLMISC_REGISTER_CLASS(CModMagicSuccessEffect)
		NLMISC_REGISTER_CLASS(CModMeleeSuccessEffect)
		NLMISC_REGISTER_CLASS(CModRangeSuccessEffect)
		NLMISC_REGISTER_CLASS(CNoLinkDOTEffect)
		NLMISC_REGISTER_CLASS(CNoLinkHoTEffect)
		NLMISC_REGISTER_CLASS(CShieldingEffect)
		NLMISC_REGISTER_CLASS(CRedirectAttacksEffect)
		NLMISC_REGISTER_CLASS(CRegenModifierEffect)
		NLMISC_REGISTER_CLASS(CShootAgainEffect)
		NLMISC_REGISTER_CLASS(CSimpleEffect)
		NLMISC_REGISTER_CLASS(CStunEffect)

	}
}


//-----------------------------------------------------------------------------
CPersistentEffect::~CPersistentEffect()
{
}

//-----------------------------------------------------------------------------
void CPersistentEffect::collectPersistentEffect()
{
	std::vector< CSEffectPtr >& activeEffect = const_cast<std::vector< CSEffectPtr >&>(_TheCharacter->getAllActiveEffects());
	for(uint32 i = 0; i < activeEffect.size(); ++i)
	{
		if(activeEffect[i]->getFamily() >= EFFECT_FAMILIES::BeginPowerEffects && activeEffect[i]->getFamily() <= EFFECT_FAMILIES::EndPowerEffects)
		{
			CSTimedEffect * effect = dynamic_cast<CSTimedEffect *>(&(*activeEffect[i]));
			if(effect)
				_PersistentEffects.push_back(effect);
		}
	}
}

//-----------------------------------------------------------------------------
void CPersistentEffect::clear()
{
	_PersistentEffects.clear();
}

//-----------------------------------------------------------------------------
void CPersistentEffect::activate()
{
	for(uint32 i = 0; i < _PersistentEffects.size(); ++i)
	{
		CSmartPtr<CSTimedEffect> effect = _PersistentEffects[i];
		effect->activate();
	}
	clear();
}

//-----------------------------------------------------------------------------
void CPersistentEffect::writePdr(CSTimedEffect * effect, CPersistentDataRecord& pdr) const
{
	CPersistentDataRecord::TToken token = pdr.addString(effect->getClassName());
	pdr.pushStructBegin(token);
	effect->store(pdr);
	pdr.pushStructEnd(token);
}

//-----------------------------------------------------------------------------
void CPersistentEffect::readPdr(CPersistentDataRecord& pdr)
{
	// make sure we don't have something completely broken in the code
	BOMB_IF(pdr.isEndOfStruct(),"CPersistentEffect::readPdr(): There's a bug in the code - I don't know what to do!",throw(NLMISC::Exception()));

	// we're expecting a start of clause marker - make sure that it's what we have
	if (!pdr.isStartOfStruct())
	{
		// this isn't a start of clause marker so just skip it, BOMB in debug mode and drop out
		STOP("CPersistentEffect::readPdr() failed to find start of struct in pdr");
		pdr.popNextArg(pdr.peekNextToken());
		return;
	}

	// open the next clause in the pdr and record its type in 'token' -WARNING - this clause but be closed before exiting the routine
	CPersistentDataRecord::TToken token = pdr.peekNextToken();
	pdr.popStructBegin(token);

	// use the value of 'token' with the NLMISC class registry / factory to instantiate a new object of the correct type
	CSString className = pdr.lookupString(token);
	CSmartPtr<CSTimedEffect> effect = CSmartPtr<CSTimedEffect>(safe_cast<CSTimedEffect *>(CClassRegistry::create(className)));

	// did we fail to instantiate an object of the correct type?
	if(effect == 0)
	{
		// instantiation of the object failed so display a nasty message, BOMB in debug mode and skip the structure's data
		string stopMsg = toString("CPersistentEffect::readPdr() failed for className='%s'",className.c_str());
		STOP(stopMsg);
		while (!pdr.isEndOfStruct())
		{
			pdr.skipData();
		}
		// close the clause opened earlier and drop out
		pdr.popStructEnd(token);
		return;
	}

	// have the new effect object extract its own data from the pdr and connect it up to the character
	effect->apply(pdr);
	effect->disableEvent();
	_PersistentEffects.push_back(effect);

	// close the clause opened earlier
	pdr.popStructEnd(token);
}


//-----------------------------------------------------------------------------
// Persistent data for CPersistentEffect
//-----------------------------------------------------------------------------

#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily
#define PERSISTENT_CLASS CPersistentEffect

#define PERSISTENT_PRE_STORE\
	const_cast<CPersistentEffect *>(this)->collectPersistentEffect();\

#define PERSISTENT_POST_STORE\
	const_cast<CPersistentEffect *>(this)->clear();\

#define PERSISTENT_DATA\
	LSTRUCT_VECT(_PersistentEffects, VECT_LOGIC(_PersistentEffects), writePdr(_PersistentEffects[i], pdr), readPdr(pdr))\

//#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"
