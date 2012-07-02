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
#include "phrase_manager/s_link_effect.h"
#include "entity_manager/entity_manager.h"
#include "player_manager/character.h"
#include "creature_manager/creature.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "nel/misc/random.h"
#include "phrase_manager/s_link_effect.h"
#include "s_link_effect_dot.h"
#include "phrase_manager/phrase_manager.h"
#include "progression/progression_pvp.h"

#include "ai_share/ai_event_report.h"
#include "entity_structure/statistic.h"
#include "egs_sheets/egs_sheets.h"

using namespace std;
using namespace NLMISC;

extern CRandom				RandomGenerator;

//-----------------------------------------------
// CSLinkEffect getNoLinkDurationTime
//-----------------------------------------------
TGameCycle CSLinkEffect::getNoLinkDurationTime( EFFECT_FAMILIES::TEffectFamily family)
{
	switch(family)
	{
	case EFFECT_FAMILIES::Fear:
		return NoLinkTimeFear;
	case EFFECT_FAMILIES::Mezz:
		return NoLinkTimeSleep;
	case EFFECT_FAMILIES::Stun:
		return NoLinkTimeStun;
	case EFFECT_FAMILIES::Root:
		return NoLinkTimeRoot;
	case EFFECT_FAMILIES::Snare:
	case EFFECT_FAMILIES::SlowMove:
		return NoLinkTimeSnare;
	case EFFECT_FAMILIES::SlowAttack:
		return NoLinkTimeSlow;
	case EFFECT_FAMILIES::Blind:
		return NoLinkTimeBlind;
	case EFFECT_FAMILIES::Madness:
		return NoLinkTimeMadness;
	case EFFECT_FAMILIES::Dot:
		return NoLinkTimeDot;

	default:
		return NoLinkSurvivalAddTime;
	};
}

//-----------------------------------------------
// CSLinkEffect getUpdatePeriod
//-----------------------------------------------
TGameCycle CSLinkEffect::getUpdatePeriod( EFFECT_FAMILIES::TEffectFamily family)
{
	switch(family)
	{
	case EFFECT_FAMILIES::Fear:
		return UpdatePeriodFear;
	case EFFECT_FAMILIES::Mezz:
		return UpdatePeriodSleep;
	case EFFECT_FAMILIES::Stun:
		return UpdatePeriodStun;
	case EFFECT_FAMILIES::Root:
		return UpdatePeriodRoot;
	case EFFECT_FAMILIES::Snare:
	case EFFECT_FAMILIES::SlowMove:
		return UpdatePeriodSnare;
	case EFFECT_FAMILIES::SlowAttack:
		return UpdatePeriodSlow;
	case EFFECT_FAMILIES::Blind:
		return UpdatePeriodBlind;
	case EFFECT_FAMILIES::Madness:
		return UpdatePeriodMadness;
	case EFFECT_FAMILIES::Dot:
		return UpdatePeriodDot;
		
	default:
		return DefaultUpdatePeriod;
	};
}


//-----------------------------------------------
// CSLinkEffect update
//-----------------------------------------------
bool CSLinkEffect::update(CTimerEvent * event, bool)
{
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffect update> Invalid target %u",_TargetRowId.getIndex() );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	// -> effect is removed when target receive the invulnerability power
	// if target is now protected, cancel the effect
/*	const CSEffect *effect = target->lookForActiveEffect(EFFECT_FAMILIES::PowerInvulnerability);
	if (effect)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}
*/

	// if link exists pay price and test distance
	if (_LinkExists)
	{
		bool endEffect = false;

		CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
		if ( !caster )
		{
			nlwarning("<CSLinkEffectDot update> Invalid caster %u",_CreatorRowId.getIndex() );
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			return true;
		}

		// test range
		if ( ! PHRASE_UTILITIES::testRange(*caster, *target, _MaxDistance) )
		{
			endEffect = true;
		}

		// if caster if a player get used focus item
		CCharacter *player = NULL;
		if (caster->getId().getType() == RYZOMID::player)
		{
			CCharacter *player = dynamic_cast<CCharacter *> (caster);
			if (player)
			{
				_Focus.init(player->getRightHandItem());
			}
		}

		// price is modified by focus item if any
		sint32 price = _CostPerUpdate;
		if (_Focus.isMagicFocus())
		{
			price = sint32( price / (1.0f + _Focus.getPowerFactor(_Skill, _SpellPower) ) );
		}

		// pay price
		SCharacteristicsAndScores & score = caster->getScores()._PhysicalScores[_EnergyCost];
		if ( score.Current != 0)
		{
			if ( score.Current >  price )
			{
				score.Current = score.Current - price;	
			}
			else
				endEffect = true;
		}
		else
			endEffect = true;

		// degrade items
		if (player != NULL)
		{
			if (_Focus.wearItem())
				player->wearRightHandItem();

			player->wearArmor();
			player->wearShield();
			player->wearJewels();
		}

		// set next update timer
		if (!endEffect)
		{
			//_UpdateTimer.setRemaining( NLMISC::TGameCycle( _UpdatePeriod / CTickEventHandler::getGameTimeStep() ), event );
			_UpdateTimer.setRemaining( CSLinkEffect::getUpdatePeriod(_Family), event );
		}
		else
		{
			// break link
			caster->removeLink( this, 1.0f );
		}
	}
	// no more link
	else
	{
		//
	}

	return false;
} // CSLinkEffect::update //

//-----------------------------------------------
// CSLinkEffect removed
//-----------------------------------------------
void CSLinkEffect::removed()
{
	if (_LinkExists)
	{
		CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
		if ( !caster )
		{
			nlwarning("<CSLinkEffectDot removed> Invalid caster %u",_CreatorRowId.getIndex() );
			return;
		}
		caster->removeLink( this, 1.0f );
	}
	
	sendEffectEndMessages();
} // CSLinkEffect::removed //

//-----------------------------------------------
// CSLinkEffectOffensive updateOffensive
//-----------------------------------------------
bool CSLinkEffectOffensive::updateOffensive(CTimerEvent * event, bool sendReportForXP)
{
	if ( CSLinkEffect::update(event,true) )
		return true;

	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _TargetRowId );
	if ( !target )
	{
		nlwarning("<CSLinkEffectDot update> Invalid target %u",_TargetRowId.getIndex() );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	// if target is now protected, cancel the effect
	CSEffect *effect = target->lookForActiveEffect(EFFECT_FAMILIES::PowerAntiMagicShield);
	if (effect)
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	// test target is still valid for a link (can happen in PVP or duel)
	string errorCode;
	if ( !PHRASE_UTILITIES::validateSpellTarget(_CreatorRowId, _TargetRowId, ACTNATURE::OFFENSIVE_MAGIC, errorCode, true ) )
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	if (_LinkExists)
	{
		CEntityBase * caster = CEntityBaseManager::getEntityBasePtr( _CreatorRowId );
		if ( !caster )
		{
			nlwarning("<CSLinkEffectDot update> Invalid caster %u",_CreatorRowId.getIndex() );
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			return true;
		}
	
		// test resistance
		if ( !_FirstResist && !EntitiesNoResist)
		{
			uint32 resistValue = 0;
			if (_Family == EFFECT_FAMILIES::Dot)
			{
				CSLinkEffectDot *dot = dynamic_cast<CSLinkEffectDot*> (this);
				if (dot)
				{
					resistValue = target->getMagicResistance(dot->getDamageType());
				}
			}
			else
			{
				resistValue = target->getMagicResistance(_Family);
			}

			sint skillValue = 0;
			if ( caster->getId().getType() == RYZOMID::player )
			{
				CCharacter * pC = (CCharacter *) caster;
				skillValue = pC->getSkillValue( _Skill );
			}
			else
			{
				const CStaticCreatures * form = caster->getForm();
				if ( !form )
				{
					nlwarning( "<MAGIC>invalid creature form %s in entity %s", caster->getType().toString().c_str(), caster->getId().toString().c_str() );
					_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
					return true;
				}	
				skillValue = form->getAttackLevel();
			}

			const CSEffect* debuff = caster->lookForActiveEffect( EFFECT_FAMILIES::DebuffSkillMagic );
			if ( debuff )
				skillValue -= debuff->getParamValue();
			const CSEffect * outPostBuff = caster->lookForActiveEffect( EFFECT_FAMILIES::OutpostMagic );
			if ( outPostBuff )
				skillValue += outPostBuff->getParamValue();

			// cap skill values with brick power
			if ( (sint32)_Power < skillValue )
				skillValue = (sint32)_Power;
			
			if ( caster->getId().getType() == RYZOMID::player )
			{
				CCharacter * pC = dynamic_cast<CCharacter *>( caster );
				if( pC )
				{
					// boost magic skill for low level chars
					sint sb = (sint)MagicSkillStartValue.get();
					skillValue = max( sb, skillValue );

					// add magic boost from consumable
					skillValue += pC->magicSuccessModifier();
				}
			}

			// test resistance
			const uint8 roll = (uint8)RandomGenerator.rand( 99 );
			_ResistFactor = CStaticSuccessTable::getSuccessFactor(SUCCESS_TABLE_TYPE::MagicResistLink, skillValue - resistValue, roll);

			// increase target resistance
			if (_ResistFactor > 0.0f)
			{
				if (_Family == EFFECT_FAMILIES::Dot)
				{
					CSLinkEffectDot *dot = dynamic_cast<CSLinkEffectDot*> (this);
					if (dot)
					{
						target->incResistModifier(dot->getDamageType(), _ResistFactor);
					}
				}
				else
				{
					target->incResistModifier(_Family,_ResistFactor);
				}
			}

			// delta level for XP gain 
			// opponent must be a creature or an npc to gain xp
			_Report.DeltaLvl = skillValue - resistValue;
			if (target->getId().getType() != RYZOMID::player && caster->getId().getType() == RYZOMID::player)
			{
				CCreature *creature = dynamic_cast<CCreature*> (target);
				if (!creature)
				{
					nlwarning("Entity %s type is creature but dynamic_cast in CCreature * returns NULL ?!", target->getId().toString().c_str());
					_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
					return true;
				}
				CCharacter * pC = dynamic_cast<CCharacter*> (caster);
				if (!pC)
				{
					nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", caster->getId().toString().c_str());
					_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
					return true;
				}

				const CStaticCreatures* form = creature->getForm();
				if (form)
					_Report.DeltaLvl =  pC->getSkillValue(_Skill) - form->getXPLevel();
				else
					sendReportForXP = false;
			}
			else
				sendReportForXP = false;
		}
		else
		{
			_FirstResist = false;
			_ResistFactor = 1.0f;
		}
		
		bool end = true;
		// resist if factor <= 0
		if ( _ResistFactor > 0.0f  )
		{
			end = false;
			if ( _ResistFactor > 1.0f )
			{
				_ResistFactor = 1.0f;
			}

			// send report for XP			
			_Report.factor = _ResistFactor;

			if (sendReportForXP)
			{
				PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( _Report );
				PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(_Report);
			}
		}
		else
		{
			PHRASE_UTILITIES::sendSpellResistMessages( _CreatorRowId, _TargetRowId);
		}

		//////////////////////////////////////////////////////////////////////////
		// TEMPORARY : SEND AGGRO MESSAGE FOR EVERY UPDATE OF OFFENSIVE LINKS
		//////////////////////////////////////////////////////////////////////////	
		CAiEventReport report;
		report.Originator = _CreatorRowId;
		report.Target = _TargetRowId;
		report.Type = ACTNATURE::OFFENSIVE_MAGIC;		
		report.AggroAdd = -0.01f;
		CPhraseManager::getInstance().addAiEventReport(report);
		//////////////////////////////////////////////////////////////////////////
				
		if (end)
		{
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			return true;
		}
	}
	else
	{
		// no link -> no possible resist ?
	}

	return false;
} // CSLinkEffectOffensive::updateOffensive //

