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
#include "fg_extraction_phrase.h"
#include "nel/misc/common.h"
#include "egs_globals.h"
#include "egs_sheets/egs_sheets.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "s_phrase_factory.h"
#include "player_manager/character.h"
#include "phrase_manager/phrase_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "forage_progress.h"
#include "deposit.h"
#include "progression/progression_pve.h"
#include "entities_game_service.h"

DEFAULT_SPHRASE_FACTORY( CFgExtractionPhrase, BRICK_TYPE::FORAGE_EXTRACTION );


NLMISC::CVariable<float> ForageQuantityBrick1( "egs", "ForageQuantityBrick1", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick2( "egs", "ForageQuantityBrick2", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick3( "egs", "ForageQuantityBrick3", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick4( "egs", "ForageQuantityBrick4", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick5( "egs", "ForageQuantityBrick5", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick6( "egs", "ForageQuantityBrick6", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick7( "egs", "ForageQuantityBrick7", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick8( "egs", "ForageQuantityBrick8", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick9( "egs", "ForageQuantityBrick9", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick10( "egs", "ForageQuantityBrick10", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick11( "egs", "ForageQuantityBrick11", "", 0.1f, 0, true );
NLMISC::CVariable<float> ForageQuantityBrick12( "egs", "ForageQuantityBrick12", "", 0.1f, 0, true );

const uint NbForageQuantityBricks = 12;

const float *ForageQuantityBrickValues [NbForageQuantityBricks] =
{
	&ForageQuantityBrick1.get(),
	&ForageQuantityBrick2.get(),
	&ForageQuantityBrick3.get(),
	&ForageQuantityBrick4.get(),
	&ForageQuantityBrick5.get(),
	&ForageQuantityBrick6.get(),
	&ForageQuantityBrick7.get(),
	&ForageQuantityBrick8.get(),
	&ForageQuantityBrick9.get(),
	&ForageQuantityBrick10.get(),
	&ForageQuantityBrick11.get(),
	&ForageQuantityBrick12.get()
};

using namespace NLMISC;
using namespace std;


/*
 * Constructor
 */
CFgExtractionPhrase::CFgExtractionPhrase()
{
	_ForageTime = 0; // not used
	_Source = NULL;
	_RequestedProps[CHarvestSource::S] = 0.025f;
	_RequestedProps[CHarvestSource::A] = 0; // by default, 0
	_RequestedProps[CHarvestSource::Q] = 0; // [ReduceDmg] as well
	_Props.Extraction.Absorption[CHarvestSource::S] = 0; // = _Props.Care.Deltas[CHarvestSource::DeltaD] = 0;
	_Props.Extraction.Absorption[CHarvestSource::A] = 0; // = _Props.Care.Deltas[CHarvestSource::DeltaE] = 0;
	_Props.Extraction.Absorption[CHarvestSource::S] = 0;
	_Props.Care.KamiAngerDec[CHarvestSource::ReservedForS] = 0; // = ObtainedProps[CHarvestSource::S];
	_Props.Care.KamiAngerDec[CHarvestSource::KamiOffNum] = 0;   // = ObtainedProps[CHarvestSource::A];
	_Props.Care.KamiAngerDec[CHarvestSource::KamiAngerDec] = 0, // = ObtainedProps[CHarvestSource::Q];
	_Props.Extraction.EcotypeSpec = ECOSYSTEM::common_ecosystem;
	_Props.Extraction.MaterialGroupFilter = RM_GROUP::Unknown;
	//_Props.Extraction.MaterialFamilyFilter = RM_FAMILY::Unknown;
	_Props.Extraction.QualitySlowFactor = ForageQualitySlowFactor.get();
	_PhraseType	= BRICK_TYPE::FORAGE_EXTRACTION;
	_IsStatic = true;
	_DeltaLvlAction = 0;
	_UsedSkill = SKILLS::unknown;
	_NbExtractions = 0;
	_LifeAbsorberRatio = 0;
	_SkillValue = 0;
	//_StopEndsForageSession = false; // intentional action stopping must not stop the forage progress
}


/*
 *
 */
bool CFgExtractionPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{
	H_AUTO(CFgExtractionPhrase_build);

	_ActorRowId = actorRowId;

	// Check grammar
	if ( ! validateSabrinaGrammar( bricks, _ActorRowId ) )
		return false;

	// Iterate on the bricks
	sint32 sabrinaCredit = 0, sabrinaCost = 0;
	float sabrinaRelativeCredit = 1.0f, sabrinaRelativeCost = 1.0f;
	for (std::vector<const CStaticBrick*>::const_iterator ib=bricks.begin(); ib!=bricks.end(); ++ib )
	{
		const CStaticBrick& brick = *(*ib);

		// Compute Sabrina credit and cost)
		if ( brick.SabrinaValue > 0 )
			sabrinaCost += brick.SabrinaValue;
		else
			sabrinaCredit -= brick.SabrinaValue;

		if( brick.SabrinaRelativeValue > 0.0f )
			sabrinaRelativeCost += brick.SabrinaRelativeValue;
		else
			sabrinaRelativeCredit -= brick.SabrinaRelativeValue;

		// Decode properties
		for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick.Params.begin(); ip!=brick.Params.end(); ++ip )
		{
			TBrickParam::IId* param = (*ip);
			switch ( param->id() )
			{
			case TBrickParam::FOCUS:
				INFOLOG("CR_FOCUS: %i",((CSBrickParamFocus *)param)->Focus);
				_FocusCost += ((CSBrickParamFocus *)param)->Focus;
				break;
			case TBrickParam::FG_ABS_S:
				INFOLOG("FG_ABS_S: %g",((CSBrickParamForageAbsorptionS *)param)->Absorption);
				_Props.Extraction.Absorption[CHarvestSource::S] = ((CSBrickParamForageAbsorptionS *)param)->Absorption;
				break;
			case TBrickParam::FG_ABS_A:
				INFOLOG("FG_ABS_S: %g",((CSBrickParamForageAbsorptionA *)param)->Absorption);
				_Props.Extraction.Absorption[CHarvestSource::A] = ((CSBrickParamForageAbsorptionA *)param)->Absorption;
				break;
			case TBrickParam::FG_ABS_Q:
				INFOLOG("FG_ABS_S: %g",((CSBrickParamForageAbsorptionQ *)param)->Absorption);
				_Props.Extraction.Absorption[CHarvestSource::Q] = ((CSBrickParamForageAbsorptionQ *)param)->Absorption;
				break;
			case TBrickParam::FG_SRC_PRD:
				INFOLOG("FG_SRC_PRD: %g",((CSBrickParamForagePeriod *)param)->Period);
				if ( ((CSBrickParamForagePeriod *)param)->Period != 0 )
					_RequestedProps[CHarvestSource::S] = 1.0f / (((CSBrickParamForagePeriod *)param)->Period * 10.0f); // period converted from second to tick
				else
					_RequestedProps[CHarvestSource::S] = 1.0f;
				break;
			case TBrickParam::FG_SRC_APT:
				INFOLOG("FG_SRC_APT: %g",((CSBrickParamForageAperture *)param)->Aperture);
				{
					uint index = (uint)(((CSBrickParamForageAperture *)param)->Aperture) - 1;
					if ( index < NbForageQuantityBricks )
						_RequestedProps[CHarvestSource::A] = ForageQuantityBaseRate.get() + *ForageQuantityBrickValues[index];
					else
						nlwarning( "Invalid index in brick %s", brick.Name.c_str() );
					break;
				}
			case TBrickParam::FG_QUALITY:
				INFOLOG("FG_QUALITY: %g",((CSBrickParamForageQuality *)param)->Quality);
				_RequestedProps[CHarvestSource::Q] = ((CSBrickParamForageQuality *)param)->Quality;
				break;
			case TBrickParam::FG_ABS_SRC_DMG:
				INFOLOG("FG_ABS_SRC_DMG: %u",((CSBrickParamForageAbsorbSourceDmg *)param)->Percent);
				_LifeAbsorberRatio = ((CSBrickParamForageAbsorbSourceDmg *)param)->Percent;
				break;
			case TBrickParam::FG_PRES:
				INFOLOG("FG_PRES: %g",((CSBrickParamForagePreservation *)param)->Pres);
				_Props.Care.Deltas[CHarvestSource::DeltaD] = ((CSBrickParamForagePreservation *)param)->Pres;
				_RequestedProps[CHarvestSource::S] = ForageCareSpeed.get();
				_RequestedProps[CHarvestSource::A] = 0;
				break;
			case TBrickParam::FG_STAB:
				INFOLOG("FG_STAB: %g",((CSBrickParamForageStability *)param)->Stab);
				_Props.Care.Deltas[CHarvestSource::DeltaE] = ((CSBrickParamForageStability *)param)->Stab;
				_RequestedProps[CHarvestSource::S] = ForageCareSpeed.get();
				_RequestedProps[CHarvestSource::A] = 0;
				break;
			/*case TBrickParam::FG_CR_STEALTH:
				INFOLOG("FG_CR_STEALTH: %g",((CSBrickParamForageCreatureStealth *)param)->Stealth);
				_Props.Care.Deltas[CHarvestSource::DeltaC] = ((CSBrickParamForageCreatureStealth *)param)->Stealth;
				_RequestedProps[CHarvestSource::S] = ForageCareSpeed.get();
				_RequestedProps[CHarvestSource::A] = 0;
				break;*/
			case TBrickParam::KAMI_OFFERING:
				INFOLOG("KAMI_OFFERING: %u",((CSBrickParamKamiOffering *)param)->Num);
				_Props.Care.KamiAngerDec[CHarvestSource::KamiOffNum] = (float)(((CSBrickParamKamiOffering *)param)->Num);
				_RequestedProps[CHarvestSource::S] = ForageKamiOfferingSpeed.get();
				_RequestedProps[CHarvestSource::A] = 0;
				break;
			case TBrickParam::KAMI_ANGER_DECREASE:
				INFOLOG("KAMI_ANGER_DECREASE: %f",((CSBrickParamKamiAngerDecrease *)param)->Delta);
				_Props.Care.KamiAngerDec[CHarvestSource::KamiAngerDec] = ((CSBrickParamKamiAngerDecrease *)param)->Delta;
				_RequestedProps[CHarvestSource::S] = ForageKamiOfferingSpeed.get();
				_RequestedProps[CHarvestSource::A] = 0;
				break;
			case TBrickParam::FG_REDUCE_DMG:
				INFOLOG("FG_REDUCE_DMG: %f",((CSBrickParamForageReduceDamage *)param)->Ratio);
				_RequestedProps[CHarvestSource::ReduceDmg] = ((CSBrickParamForageReduceDamage *)param)->Ratio;
				break;
			case TBrickParam::FG_ECT_SPC:
				INFOLOG("FG_ECT_SPC: %s",((CSBrickParamForageEcotypeSpec *)param)->Ecotype.c_str());
				_Props.Extraction.EcotypeSpec = ECOSYSTEM::stringToEcosystem( ((CSBrickParamForageEcotypeSpec *)param)->Ecotype );
				break;
			case TBrickParam::FG_RMGRP_FILT:
				INFOLOG("FG_RMGRP_FILT: %i",((CSBrickParamForageRMGroupFilter *)param)->Value);
				_Props.Extraction.MaterialGroupFilter = ((CSBrickParamForageRMGroupFilter *)param)->Value ;
				break;
			/*case TBrickParam::FG_RMFAM_FILT:
				INFOLOG("FG_RMFAM_FILT: %i",((CSBrickParamForageRMFamilyFilter *)param)->Value);
				_Props.Extraction.MaterialFamilyFilter = ((CSBrickParamForageRMFamilyFilter *)param)->Value ;
				break;*/
			default:;
			}
		}

		//nlerror( "TODO: Families" );
		//if ( brick.Family >= BRICK_FAMILIES::BeginForage

		//insertProgressingSkill( brick.Skill, brick.SheetId );
	}

	if ( buildToExecute )
	{
		// Get player entity
		CCharacter *player = (CCharacter *) CEntityBaseManager::getEntityBasePtr( _ActorRowId );
		if (!player)
			return false;

		_FocusCost = (sint32)(((float)_FocusCost) * (1.0f + player->wearMalus()));

		// Set the source (if the target is a source)
		if ( player->getTarget().getType() == RYZOMID::forageSource )
		{
			_SourceRowId = player->getTargetDataSetRow();
			_Source = CHarvestSourceManager::getInstance()->getEntity( _SourceRowId );
			if ( _Source )
			{
				if ( isCareAction() )
				{
					if ( _Source->foragers().empty() )
					{
						PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_NO_CARE_FIRST" );
						player->sendCloseTempInventoryImpulsion(); // close the item window that was auto-opened by the client
						return false;
					}
				}
				else
				{
					// Check that only one player can extract (but several ones can take care)
					if ( (! _Source->foragers().empty()) && (_Source->foragers().front() != player->getEntityRowId()) )
					{
						PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_ONLY_ONE_EXTRACTOR" );
						player->sendCloseTempInventoryImpulsion(); // close the item window that was auto-opened by the client
						return false;
					}

					_Props.Extraction.ObtainedProps[CHarvestSource::S] = _RequestedProps[CHarvestSource::S]; // if it varies, it looks like a bug to the player
					_Props.Extraction.ObtainedProps[CHarvestSource::A] = _Source->aperture();
					_Props.Extraction.ObtainedProps[CHarvestSource::Q] = _Source->quality();
				}
			}
			// else the phrase will be rejected in validate()
		}

		// Bonus: when using ecosystem specialization, decrease impact on some risks (TODO: linearly dependant on specialized skill value).
		if ( _Props.Extraction.EcotypeSpec != ECOSYSTEM::common_ecosystem )
		{
			// Check if the ecotype specialization matches
			if ( _Source && (! _Source->forageSite()->deposit()->matchEcotype( _Props.Extraction.EcotypeSpec )) )
			{
				PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_ECOTYPE_SPEC_NOT_MATCHING" );
				return false;
			}
			if ( _RequestedProps[CHarvestSource::A] != 0 )
			{
				// Extraction
				if ( _Props.Extraction.Absorption[CHarvestSource::A] < ForageExtractionAbsorptionEcoSpecMax.get() )
					_Props.Extraction.Absorption[CHarvestSource::A] = min( ForageExtractionAbsorptionEcoSpecMax.get(), _Props.Extraction.Absorption[CHarvestSource::A] * ForageExtractionAbsorptionEcoSpecFactor.get() );
				if ( _Props.Extraction.Absorption[CHarvestSource::Q] < ForageExtractionAbsorptionEcoSpecMax.get() )
				_Props.Extraction.Absorption[CHarvestSource::Q] = min( ForageExtractionAbsorptionEcoSpecMax.get(), _Props.Extraction.Absorption[CHarvestSource::Q] * ForageExtractionAbsorptionEcoSpecFactor.get() ); // especially useful for Harmful
			}
			else
			{
				// Care
				_Props.Care.Deltas[CHarvestSource::DeltaE] *= ForageExtractionCareEcoSpecFactor.get();
			}
		}

		// Get used/progressing skill

		// Bonus: when using raw material specialization, increase quality and decrease one impact
		if ( (_Props.Extraction.MaterialGroupFilter != RM_GROUP::Unknown) /*|| (_Props.Extraction.MaterialFamilyFilter != RM_FAMILY::Unknown)*/ )
		{
			// Check if the raw material specialization matches
			if ( _Source )
			{
				const CStaticItem *staticItem = CSheets::getForm( _Source->materialSheet() );
				if ( staticItem && staticItem->Mp )
				{
					if ( ! ((staticItem->Mp->getGroup() == _Props.Extraction.MaterialGroupFilter) /*||
						   (staticItem->Mp->Family == _Props.Extraction.MaterialFamilyFilter)*/) )
					{
						PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "FORAGE_MATERIAL_SPEC_NOT_MATCHING");
						return false;
					}
				}
			}
			if ( _RequestedProps[CHarvestSource::A] != 0 )
			{
				// Extraction
				_Props.Extraction.QualitySlowFactor *= ForageQualitySlowFactorMatSpecRatio.get();
				if ( _Props.Extraction.Absorption[CHarvestSource::S] < ForageExtractionAbsorptionMatSpecMax.get() )
					_Props.Extraction.Absorption[CHarvestSource::S] = min( ForageExtractionAbsorptionMatSpecMax.get(), _Props.Extraction.Absorption[CHarvestSource::S] * ForageExtractionAbsorptionMatSpecFactor.get() ); // especially useful for Harmful
			}
			else
			{
				// Care
				_Props.Care.Deltas[CHarvestSource::DeltaD] *= ForageExtractionCareMatSpecFactor.get();
			}
		}

		// Get skill from ecotype and calc delta level for success factor
		if ( _Source )
		{
			// TEMP: For Episode2 RMs, don't get the skill by ecotype but use the highest skill
			_UsedSkill = SKILLS::unknown;
			const CStaticItem *staticItem = CSheets::getForm( _Source->materialSheet() );
			if ( staticItem && staticItem->Mp )
			{
				const RM_FAMILY::TRMFamily FirstEp2Family = 717;
				const RM_FAMILY::TRMFamily LastEp2Family = 728;
				RM_FAMILY::TRMFamily fam = staticItem->Mp->Family;
				if ( (fam >= FirstEp2Family) && (fam <= LastEp2Family) )
				{
					const uint NbSkillsToTest = 5;
					const SKILLS::ESkills skillsToTest[NbSkillsToTest] =
						{ SKILLS::SHFDAEM, SKILLS::SHFFAEM, SKILLS::SHFJAEM, SKILLS::SHFLAEM, SKILLS::SHFPAEM };
					sint32 maxFoundSkillValue = 0;
					SKILLS::ESkills maxFoundSkill = SKILLS::unknown;
					for ( uint i=0; i!=NbSkillsToTest; ++i )
					{
						sint32 skillValue = player->getSkillValue(skillsToTest[i]);
						if ( skillValue > maxFoundSkillValue )
						{
							maxFoundSkillValue = skillValue;
							maxFoundSkill = skillsToTest[i];
						}
					}
					_UsedSkill = maxFoundSkill;
				}
			}

			if ( _UsedSkill == SKILLS::unknown )
				_UsedSkill = getForageSkillByEcotype( _Source->forageSite()->deposit()->ecotype() ); // warning: if changing the way of getting _UsedSkill, change the action report
			if ( _UsedSkill == SKILLS::unknown )
			{
				nlwarning( "FG: Deposit %s has no ecotype", _Source->forageSite()->deposit()->name().c_str() );
				return false;
			}
			sint32 skillValue = player->getSkillValue( _UsedSkill );
			// add modifier from consumable
			skillValue += player->forageSuccessModifier(_Source->forageSite()->deposit()->ecotype()) + player->forageSuccessModifier(ECOSYSTEM::common_ecosystem);
			skillValue = std::max(skillValue,(sint32)0);

			_SkillValue = (skillValue < 250) ? skillValue : 250;

			if ( _RequestedProps[CHarvestSource::A] != 0 )
			{
				// For extraction phrase, use requested quality as action level to determine success factor
				sint32 actionLevel = (sint32)(_RequestedProps[CHarvestSource::Q]);
				_DeltaLvlAction = _SkillValue - actionLevel;

				// Slow the quality depending on the requested quality and even more on the delta level
				_Props.Extraction.QualitySlowFactor *= (1.0f + _RequestedProps[CHarvestSource::Q]*ForageQualitySlowFactorQualityLevelRatio.get());
				if ( _DeltaLvlAction < -10 )
					_Props.Extraction.QualitySlowFactor *= (1.0f - ((float)_DeltaLvlAction)*ForageQualitySlowFactorDeltaLevelRatio.get());

				_Source->setBonusExtraExtractionTime( player->forageBonusExtractionTime() );
				_Source->recalcExtractionTime( _RequestedProps[CHarvestSource::Q] );
			}
			else
			{
				// For care, use total sabrina cost of the action
				_DeltaLvlAction = _SkillValue - sabrinaCost;
			}
		}
	}

	//egs_feinfo( "Forage phrase built" );
	bool phraseIsOk = testSabrinaBalance( (sint32)(sabrinaCredit * sabrinaRelativeCredit), (sint32)(sabrinaCost * sabrinaRelativeCost) );
	return phraseIsOk || (ForageDebug.get() == 10);
}


// 0..250 -> 1..15
inline uint16 calcLevelFromSkillValue( sint32 skillValue )
{
	return (min((skillValue * 15 / 250), 14)) + 1;
}


/*
 * evaluate phrase
 * \param evalReturnInfos struct that will receive evaluation results
 * \return true if eval has been made without errors
 */
bool CFgExtractionPhrase::evaluate()
{
	return true;
}


/*
 *
 */
bool CFgExtractionPhrase::validate()
{
	H_AUTO(CFgExtractionPhrase_validate);

	if ( ! HarvestSystemEnabled )
		return false;

	// Get entity
	CCharacter *player = (CCharacter*)CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if (!player)
	{
		egs_feinfo( "FG: Player E%u left game during extraction action", _ActorRowId.getIndex() );
		return false;
	}

	// Test if entity can use action
	if (player->canEntityUseAction() == false)
	{
		if (_CyclicPhrase)
		{
			idle( true );
		}
		else
		{
			return false;
		}
	}

	// Cannot forage if temp inventory is already in use for something else than foraging
	if (!player->canEnterTempInventoryMode(TEMP_INV_MODE::Forage))
	{
		return false;
	}

	// Entities cant forage if in combat
	TDataSetRow entityRowId = CPhraseManager::getInstance().getEntityEngagedMeleeBy( _ActorRowId );
	if ( ! entityRowId.isNull() )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "CANT_FORAGE_ENGAGED_IN_MELEE");
		return false;
	}

	// Test if the correct tool is in use (in the right hand!)
	bool correctToolFound = false;
	CGameItemPtr item = player->getItem( INVENTORIES::handling, INVENTORIES::right );
	if ( ! (item == 0) )
	{
		const CStaticItem *staticItem = CSheets::getForm( item->getSheetId() );
		if ( (staticItem) && (staticItem->Family == ITEMFAMILY::HARVEST_TOOL) )
			correctToolFound = true;
	}
	if ( ! correctToolFound )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "INVALID_FORAGE_TOOL");
		return false;
	}

	// test if tool is not worned
	if( item->getItemWornState() == ITEM_WORN_STATE::Worned )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "INVALID_FORAGE_TOOL");
		return false;
	}

	// Check focus
	const sint32 focus = player->getScores()._PhysicalScores[ SCORES::focus ].Current;
	if ( focus < _FocusCost )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "PHRASE_NOT_ENOUGH_FOCUS");
		if ( ForageDebug.get() == 0 )
		{
			return false;
		}
	}

	// Check death
	const sint32 hp = player->getScores()._PhysicalScores[ SCORES::hit_points ].Current;
	if (hp <= 0	||	player->getMode()==MBEHAV::DEATH)
	{
		return false;
	}

	// TODO: Check if on mount

	// Check if the target source was set in build()
	if ( _SourceRowId.isNull() || (! _Source) )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "FORAGE_EXTRACTION_NEEDS_TARGET_SOURCE");
		// Don't close forage window if launching a new action when the source disappeared
		// But don't give XP as it has already be given when the source disappeared
		// otherwise an exploit is possible
		//old: player->giveForageSessionResult();
		return false;
	}

	// Check that the target source has not changed
	const TDataSetRow& targetRowId = player->getTargetDataSetRow();
	if (!TheDataset.isAccessible(targetRowId))
	{
		//nlwarning( "FG: Player E%u target is not anymore in mirror", _ActorRowId.getIndex() );
		return false;
	}
	if ( targetRowId != _SourceRowId ) // has changed (datasetrows are different when reassigned)
	{
		return false;
	}

	// Check that the source still exists (assumes there's no tick between last validate() and apply())
	CHarvestSource *harvestSource = CHarvestSourceManager::getInstance()->getEntity( targetRowId );
	if ( ! harvestSource ) // sufficient because the manager is the only one that can remove a source
	{
		//_StopEndsForageSession = true; // this was the only case to allow player to take RM and get XP!
		return false; // has disappeared
	}

	// test if tool have enough quality
	sint depositQ = (sint)harvestSource->forageSite()->deposit()->maxQuality();

	if ((depositQ > 0) && (item->recommended()+49  < depositQ))
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "FORAGE_TOOL_QUALITY_TOO_LOW");
		return false;
	}

	// Check the distance from the player to the source (ignoring Z because for tunnel case, player couldn't target the source)
	const CEntityState& state = player->getState();
	CVector2f playerPos( (float)state.X / 1000.0f, (float)state.Y / 1000.0f );
	float sqrDist = (playerPos - _Source->pos()).sqrnorm();
	if ( sqrDist > 1.0f ) // allowed radius: 1 m
	{
		// Workaround: authorize high distance only if it's the first extraction by the player on the source,
		// to let the auto-move feature move the player to the source.
		// Otherwise he may get the error message because the client launches the action as soon as the
		// player reaches the source, whereas the pos is not updated on the server yet.
		if ( find ( harvestSource->foragers().begin(), harvestSource->foragers().end(), player->getEntityRowId() ) != harvestSource->foragers().end() )
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_TOO_FAR_FROM_SOURCE" );
			return false;
		}
	}

	// Check that the quality brick is high enough
	if ( (_RequestedProps[CHarvestSource::A] != 0) &&
		 (((sint)_RequestedProps[CHarvestSource::Q]) < (sint)harvestSource->forageSite()->deposit()->minQuality()) )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_QUALITY_TOO_LOW" );
		return false;
	}

	// Reduce Blowup Damage *not cyclic*
	if ( (_RequestedProps[CHarvestSource::A] == 0) &&
		 (_RequestedProps[CHarvestSource::ReduceDmg] != 0) )
	{
		if ( _NbExtractions != 0 )
			return false;
	}

	// OK
	//egs_feinfo( "Forage phrase validated" );
	return true;
}


/*
 *
 */
bool CFgExtractionPhrase::update()
{
	H_AUTO(CFgExtractionPhrase_update);

	CCharacter* player = PlayerManager.getChar( _ActorRowId );
	if ( ! player )
		return false;

	if ( state() == Validated )
	{
		// phrase can be idle when player try to forage when equiping tool
		if( idle() )
		{
			idle(false);

			// check if actor can use action
			CBypassCheckFlags bypassCheckFlags = CBypassCheckFlags::NoFlags;
			if (player->canEntityUseAction(bypassCheckFlags,false) == false)
			{
				if (_CyclicPhrase)
				{
					// go back to idle until player can use action
					idle(true);
				}
				else
				{
					// stop the phrase
					return false;
				}
			}

			// stop idle, now entity can use action so next step will be 'execute'
			return true;
		}
	}

	// Stop action if the source death has ended the forage session (either succesfully or not)
	return player->forageProgress() && (!player->forageProgress()->resultCanBeTaken()); // succesfully:resultCanBeTaken=true; unsuccesfully:forageProgress() deleted
}


/*
 *
 */
void CFgExtractionPhrase::execute()
{
	H_AUTO(CFgExtractionPhrase_execute);

	// Get character
	CCharacter* player = PlayerManager.getChar( _ActorRowId );
	if (!player)
		return;

	// Init phrase execution
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	nlassert( _Source );
	bool isTheExtractor = false;
	if ( _NbExtractions == 0 )
	{
		// Begin extraction cycle
		isTheExtractor = _Source->beginExtraction( _ActorRowId, (_SkillValue<10) && (ForageDebug.get() < 5) );
		//nldebug( "FG: Period: %u ticks", averageDeliveryPeriod() ); // now, we want time to be constant, so this is set only in 1st execute (otherwise the player thinks it's a bug)
	}
	_ExecutionEndDate = time + averageDeliveryPeriod();

	// Begin action (reset action properties, because a non-cyclic action can fit in between two execute() of our cyclic action)
	player->setCurrentAction( CLIENT_ACTION_TYPE::Forage, _ExecutionEndDate );
	player->staticActionInProgress( true, STATIC_ACT_TYPES::Forage );
	if ( _NbExtractions == 0 )
	{
		player->beginOrResumeForageSession( _Source->materialSheet(), _Source->rowId(), _UsedSkill, isTheExtractor /*, player->getRightHandItem()*/ ); // only in the 1st execute(), because another cyclic extraction cannot fit in between two execute(); forage tool already checked in validate()

		// Set behaviour
		bool isItCareAction = isCareAction();
		MBEHAV::CBehaviour behav( isItCareAction ? MBEHAV::CARE : MBEHAV::EXTRACTING );
		behav.ForageExtraction.Level = calcLevelFromSkillValue( _SkillValue );
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	}

	//egs_feinfo( "Extraction phrase execution" );
}


/*
 * called at the end of the latency time
 */
void CFgExtractionPhrase::end()
{
	H_AUTO(CFgExtractionPhrase_end);

	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (!player)
		return;

	player->clearCurrentAction();
	player->staticActionInProgress(false);

	//egs_feinfo( "Forage phrase end" );
}


/*
 * called when the action is interupted (if validate returns false for example)
 */
void CFgExtractionPhrase::stop()
{
	H_AUTO(CFgExtractionPhrase_stop);

	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (!player)
		return;

	player->clearCurrentAction();
	player->staticActionInProgress(false);

	// Set behaviour, only if execute() was called (no animation if stop() called after the first validate())
	// Also must stop forage if the current behaviour is a Forage one
	MBEHAV::EBehaviour	behav= player->getBehaviour().Behaviour;
	if ( _ExecutionEndDate != 0 || behav==MBEHAV::CARE || behav==MBEHAV::EXTRACTING)
	{
		MBEHAV::EBehaviour	newBehav;
		// End the animation according to the current activated
		if(behav==MBEHAV::CARE)
			newBehav= MBEHAV::CARE_END;
		else if(behav==MBEHAV::EXTRACTING)
			newBehav= MBEHAV::EXTRACTING_END;
		// Legacy code. Don't know when this can happens (maybe not possible).
		else
			newBehav= isCareAction() ? MBEHAV::CARE_END : MBEHAV::EXTRACTING_END;
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, newBehav );
	}

	// Don't send message
	//PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "FORAGE_CANCEL"); // string localization not done
	//egs_feinfo( "Forage phrase cancelled" ); // TEMP
}


/*
 *
 */
bool CFgExtractionPhrase::launch()
{
	// apply immediatly
	_ApplyDate = 0;
	return true;
}


/*
 *
 */
void CFgExtractionPhrase::apply()
{
	H_AUTO(CFgExtractionPhrase_apply);

	CCharacter* player = PlayerManager.getChar( _ActorRowId );
	if (!player)
		return;

	// Spend energies
	SCharacteristicsAndScores &focus = player->getScores()._PhysicalScores[SCORES::focus];
	if ( focus.Current != 0)
	{
		focus.Current = focus.Current - _FocusCost;
		if (focus.Current < 0)
			focus.Current = 0;
	}

	// If not forced failure, roll success factor
	float successFactor;
	if ( PHRASE_UTILITIES::forceActionFailure(player) )
		successFactor = 0.0f;
	else if ( _DeltaLvlAction < -10 ) // don't fail in progressing normally
		successFactor = (ForageDebug.get()<5) ? rollSuccessFactor( _DeltaLvlAction ) : 1.0f;
	else
		successFactor = 0.5f;

	// Extract/impact source
	applyExtraction( player, successFactor );

	//egs_feinfo( "Forage phrase applied" );
}


/*
 *
 */
void CFgExtractionPhrase::applyExtraction( CCharacter *player, float successFactor )
{
	H_AUTO(CFgExtractionPhrase_applyExtraction);

	nlassert( _Source );
	if ( ! player->forageProgress() )
		return;

	// Apply action
	CHarvestSource::TRealTimeProp propDrop = CHarvestSource::NoDrop;
	if ( ! isCareAction() )
	{
		// Activate or not "life absorption by player"
		uint16 previousAmount = player->forageProgress()->amount();

		// Compute extraction
		_Source->extractMaterial( _RequestedProps, _Props.Extraction.Absorption, ForageQualityCeilingFactor.get(), _Props.Extraction.QualitySlowFactor, _Props.Extraction.ObtainedProps, successFactor, _LifeAbsorberRatio, player->getEntityRowId(), propDrop );

		// Fill obtained material
#ifdef NL_DEBUG
		nldebug( "FG: Player requests (dA %.2f Q %.1f), gets (dA %.2f Q %.1f) of %s", _RequestedProps[CHarvestSource::A], _RequestedProps[CHarvestSource::Q], _Props.Extraction.ObtainedProps[CHarvestSource::A], _Props.Extraction.ObtainedProps[CHarvestSource::Q], _Source->materialSheet().toString().c_str() );
#endif
		if ( player->forageProgress() ) // can have been reset if extractMaterial() killed the player
			player->forageProgress()->fillFromExtraction( _Props.Extraction.ObtainedProps[CHarvestSource::A], _Props.Extraction.ObtainedProps[CHarvestSource::Q], player );
		else
			return;

		// Report result of action
		if ( propDrop != CHarvestSource::NoDrop )
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_MISS" );
			switch ( propDrop )
			{
			case CHarvestSource::A: PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_DROP_QUANTITY" ); break;
			case CHarvestSource::Q: PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_DROP_QUALITY" ); break;
			default:;
			}
		}

		// Display "DeltaHP" on target
		if ( player->forageProgress()->amount() > previousAmount )
		{
			MBEHAV::CBehaviour behav( MBEHAV::EXTRACTING ); // in case stop() was called (ex: cancelStaticActionInProgress(), reset the extracting behaviour)
			behav.DeltaHP = (sint16)(player->forageProgress()->amount() |
				(player->forageProgress()->quality() << 8));
			behav.ForageExtraction.Level = calcLevelFromSkillValue( _SkillValue );
			PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
		}
	}
	else
	{
		// Apply care
		bool isUseful;
		_Source->takeCare( _Props.Care.Deltas, &isUseful );

		player->forageProgress()->fillFromCare( isUseful );

		// Make offering to Kami, if requested
		if ( _Props.Care.KamiAngerDec[CHarvestSource::KamiOffNum] != 0 )
		{
			doKamiOffering( player );
		}

		// Reduce blowing up damage, if requested
		float reduceDmgAmount = _RequestedProps[CHarvestSource::ReduceDmg];
		if ( reduceDmgAmount != 0 )
		{
			_Source->reduceBlowingUpDmg( reduceDmgAmount );
		}
	}

	++_NbExtractions;
}

/*
 **
 */
struct CNonNullGameItemPtrPred : std::unary_function<CGameItemPtr,bool>
{
	bool operator() ( const CGameItemPtr& p ) { return (p != NULL); }
};


/*
 *
 */
void CFgExtractionPhrase::doKamiOffering( CCharacter *player )
{
	H_AUTO(CFgExtractionPhrase_doKamiOffering);

	// Count the number of non empty slots
//	const vector<CGameItemPtr> &theBag = player->getInventory()[INVENTORIES::bag]()->getChildren();
	CInventoryPtr theBag = player->getInventory(INVENTORIES::bag);
//	uint nbNonEmptySlots = count_if( theBag.begin(), theBag.end(), CNonNullGameItemPtrPred() );
	uint nbNonEmptySlots = theBag->getFreeSlotCount();
	uint nbOfferingsToDo = (uint)(_Props.Care.KamiAngerDec[CHarvestSource::KamiOffNum]);
	if ( nbNonEmptySlots < nbOfferingsToDo )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_KAMI_OFFERING_NOT_ENOUGH_ITEMS" );
		//nldebug( "Can't do %u offering(s), only %u non-empty slots in bag", nbOfferingsToDo, nbNonEmptySlots );
	}
	else
	{
		for ( uint n=0; n!=nbOfferingsToDo; ++n )
		{
			// Pick a random item in the player's bag! (consider a stack as an item)
			uint iRandomNonEmptySlot = RandomGenerator.rand( nbNonEmptySlots-1 );
			uint iNonEmptySlot = 0;
//			for ( vector<CGameItemPtr>::const_iterator ib=theBag.begin(); ib!=theBag.end(); ++ib )
			for ( uint i=0; i<theBag->getSlotCount(); ++i)
			{
				CGameItemPtr item = theBag->getItem(i);
				if ( item == NULL )
					continue;

				if ( iNonEmptySlot == iRandomNonEmptySlot )
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
					params[0].SheetId = item->getSheetId();
					PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_KAMI_OFFERING_ITEM", params );

					/*Bsi.append( StatPath, NLMISC::toString("[FKIO] %s '%s' %.1f %.1f %s",
						player->getId().toString().c_str(),
						_Source->depositForK()->name().c_str(),
						_Source->depositForK()->kamiAnger(),
						_Props.Care.KamiAngerDec[CHarvestSource::KamiAngerDec],
						(*ib)->getSheetId().toString().c_str()) );*/

					/*
					EgsStat.displayNL("[FKIO] %s '%s' %.1f %.1f %s",
						player->getId().toString().c_str(),
						_Source->depositForK()->name().c_str(),
						_Source->depositForK()->kamiAnger(),
						_Props.Care.KamiAngerDec[CHarvestSource::KamiAngerDec],
						(*ib)->getSheetId().toString().c_str());
					*/
//					EGSPD::forageKamiItemOffering(player->getId(), _Source->depositForK()->name(), _Source->depositForK()->kamiAnger(), _Props.Care.KamiAngerDec[CHarvestSource::KamiAngerDec], item->getSheetId().toString());

					// TODO: quantity, filter, etc.
//					player->destroyItem( INVENTORIES::bag, ib-theBag.begin(), 1/*(*ib).quantity()*/, false );
					theBag->deleteItem(i);
					--nbNonEmptySlots;
					break;
				}
				++iNonEmptySlot;
			}
		}

		// Decrease kami anger level
		_Source->depositForK()->decKamiAnger( _Props.Care.KamiAngerDec[CHarvestSource::KamiAngerDec] );
	}
}
