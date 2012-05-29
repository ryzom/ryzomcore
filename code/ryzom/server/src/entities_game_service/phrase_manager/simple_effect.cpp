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
// net
#include "nel/net/message.h"
//
#include "simple_effect.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_manager/entity_base.h"
#include "phrase_manager/effect_factory.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "entity_manager/entity_manager.h"

extern CPlayerManager PlayerManager;

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//--------------------------------------------------------------
//		CSimpleEffect::update()
//--------------------------------------------------------------
bool CSimpleEffect::update(CTimerEvent * event, bool applyEffect)
{
	if (applyEffect)
	{
#ifdef NL_DEBUG
		_Applied = true;
#endif

		CCharacter *playerChar = PlayerManager.getChar(_TargetRowId);
		if (!playerChar)
			return false;

		// manage some special things according to effect family
		switch(_Family) 
		{
		case EFFECT_FAMILIES::CombatDefenseModifier:
			playerChar->incParryModifier(_Value);
		case EFFECT_FAMILIES::CombatDebuffDodge:
			playerChar->incDodgeModifier(_Value);
			break;

		case EFFECT_FAMILIES::CombatDebuffCombatSkills:
		case EFFECT_FAMILIES::DebuffSkillMelee:
			playerChar->incParryModifier(_Value);
			break;

		default:;
		}
	}

	return false;
}

//--------------------------------------------------------------
//		CSimpleEffect::removed()
//--------------------------------------------------------------
void CSimpleEffect::removed()
{
#ifdef NL_DEBUG
	if (!_Applied)
	{
		nlwarning("Effect removed but was never applied ! problem");
	}
	if (_IsRemoved)
	{
		nlwarning("Effect removed () method called but _IsRemoved == true ! problem");
	}
#endif

	CCharacter *playerChar = PlayerManager.getChar(_TargetRowId);
	if (playerChar)
	{
		// manage some special things according to effect family
		switch(_Family) 
		{
		case EFFECT_FAMILIES::CombatDefenseModifier:
			playerChar->incParryModifier( -_Value);
		case EFFECT_FAMILIES::CombatDebuffDodge:
			playerChar->incDodgeModifier( -_Value);
			break;
			
		case EFFECT_FAMILIES::CombatDebuffCombatSkills:
		case EFFECT_FAMILIES::DebuffSkillMelee:
			playerChar->incParryModifier( -_Value);
			break;
			
		default:;
		}
	}

	sendEffectEndMessages();
} // removed //


CEffectTFactory<CSimpleEffect> *CSlowMagicEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::SlowMagic);
CEffectTFactory<CSimpleEffect> *CSlowMeleeEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::SlowMelee);
CEffectTFactory<CSimpleEffect> *CSlowRangeEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::SlowRange);
CEffectTFactory<CSimpleEffect> *CSlowAttackEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::SlowAttack);
CEffectTFactory<CSimpleEffect> *CMadnessMagicEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::MadnessMagic);
CEffectTFactory<CSimpleEffect> *CMadnessMeleeEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::MadnessMelee);
CEffectTFactory<CSimpleEffect> *CMadnessRangeEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::MadnessRange);
CEffectTFactory<CSimpleEffect> *CMadnessEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::Madness);
CEffectTFactory<CSimpleEffect> *CMezzEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::Mezz);
CEffectTFactory<CSimpleEffect> *CRootEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::Root);
CEffectTFactory<CSimpleEffect> *CFearEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::Fear);
CEffectTFactory<CSimpleEffect> *CBlindEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::Blind);
CEffectTFactory<CSimpleEffect> *CDebuffMagicEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffSkillMagic);
CEffectTFactory<CSimpleEffect> *CDebuffMeleeEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffSkillMelee);
CEffectTFactory<CSimpleEffect> *CDebuffRangeEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffSkillRange);

CEffectTFactory<CSimpleEffect> *CDebuffAcidResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistAcid);
CEffectTFactory<CSimpleEffect> *CDebuffColdResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistCold);
CEffectTFactory<CSimpleEffect> *CDebuffRotResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistRot);
CEffectTFactory<CSimpleEffect> *CDebuffFireResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistFire);
CEffectTFactory<CSimpleEffect> *CDebuffElectricyResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistElectricity);
CEffectTFactory<CSimpleEffect> *CDebuffPoisonResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistPoison);
CEffectTFactory<CSimpleEffect> *CDebuffShockResistEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::DebuffResistSchock);

CEffectTFactory<CSimpleEffect> *CInvulnerabilityEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::PowerInvulnerability);

CEffectTFactory<CSimpleEffect> *CReflectDamageEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::ReflectDamage);
CEffectTFactory<CSimpleEffect> *CReverseDamageEffectFactory = new CEffectTFactory<CSimpleEffect>(EFFECT_FAMILIES::ReverseDamage);


