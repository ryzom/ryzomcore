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
#include "fg_prospection_phrase.h"
#include "nel/misc/common.h"
#include "nel/misc/fast_floor.h"
#include "nel/net/unified_network.h"
#include "egs_globals.h"
#include "egs_sheets/egs_sheets.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "s_phrase_factory.h"
#include "player_manager/character.h"
#include "deposit.h"
#include "zone_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "weather_everywhere.h"
#include "world_instances.h"
#include "progression/progression_pve.h"
#include "server_share/used_continent.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "server_share/stl_allocator_checker.h"
//#include "dss_session_manager.h"

DEFAULT_SPHRASE_FACTORY( CFgProspectionPhrase, BRICK_TYPE::FORAGE_PROSPECTION );
//DEFAULT_SPHRASE_FACTORY( CFgSpyingPhrase, BRICK_TYPE::FORAGE_SPYING );
//DEFAULT_SPHRASE_FACTORY( CFgGardeningPhrase, BRICK_TYPE::FORAGE_GARDENING );

using namespace NLMISC;
using namespace std;



extern CVariable<uint32> ForageLocateDepositUpdateFrequency;
extern CHarvestSource AutoSpawnSourceIniProperties;

CVariable<sint32> ForageForcedSeason( "egs", "ForageForcedSeason", "-1=Use real season, 0=Spring, ..., 3=Winter", -1, 0, true );
CVariable<sint32> ForageForcedDaycycle( "egs", "ForageForcedDaycycle", "-1=Use real day cycle, 0=Day/Dawn, 1=Day/Day, 2=Day/Evening, 3=Night/Nightfall, 4=Night/Night", -1, 0, true );
CVariable<sint32> ForageForcedWeather( "egs", "ForageForcedWeather", "-1=Use real weather, 0=Best, ..., 3=Worst", -1, 0, true );

bool WarnIfOutsideOfRegion = true;


const char * NothingFoundReasonStrs[NFNbReasons] =
{
	NULL, // unknown => no string
	"FORAGE_NO_DEPOSIT_HERE",
	"FORAGE_NO_DEPOSIT_IN_SEASON",
	"FORAGE_NO_DEPOSIT_IN_CONTEXT", // TIMEOFDAY
	"FORAGE_NO_DEPOSIT_IN_WEATHER",
	"FORAGE_ECOTYPE_SPEC_NOT_MATCHING",
	"FORAGE_NO_DEPOSIT_MATERIAL_FILTER",
	"FORAGE_NO_LOCAL_RM_MATERIAL_FILTER",
	"FORAGE_NO_DEPOSIT_STATQUALITY",
	"FORAGE_NO_LOCAL_RM_STATQUALITY",
	"FORAGE_NO_DEPOSIT_EXACT_STATQUALITY",
	"FORAGE_NO_LOCAL_RM_EXACT_STATQUALITY",
	"FORAGE_SITE_DEPLETED",
	"FORAGE_DEPOSIT_DEPLETED",
	"FORAGE_CANT_ADD_SOURCE"
};


const float MaxForageRange = 100.0f; // 0 is forbidden
const float MaxForageAngle = (float)(2.0*Pi); // 0 is forbidden


/**
 * Helper for sending position validation requests to AIS
 */
class CSourceSpawnPosValidator
{
public:

	/// Constructor. Simple version, only one source per message, and no path checking.
	CSourceSpawnPosValidator( uint32 aiInstanceNumber );
	
	/// Constructor, multiple & path checking version
	CSourceSpawnPosValidator( uint32 aiInstanceNumber, const NLMISC::CVector& prospectingPos );

	/// Push pos and source datasetrow. Precondition: isValidationPossible().
	void		pushPosAndRow( const NLMISC::CVector2f& pos, const TDataSetRow& row );

	/// Send validation request. Precondition: isValidationPossible().
	void		sendRequest();
	
	/// Return true if the source(s) should not be spawned without validation (i.e. an AIS has been found for the specified instance)
	bool		isValidationPossible() const { return ForageValidateSourcesSpawnPos.get() && (_AisId.get() != 0); }

private:
	
	NLNET::TServiceId	_AisId;

	uint16				_SizePos;

	NLNET::CMessage		_MsgOut;

	uint32				_NbItems; // if ~0, only one call to pushPosAndRow() is possible
};


/*
 * Constructor
 */
CSourceSpawnPosValidator::CSourceSpawnPosValidator( uint32 aiInstanceNumber )
{
	_AisId = CWorldInstances::instance().getAISId( aiInstanceNumber );
	if ( _AisId.get() != 0 )
	{
		_MsgOut.setType( "S_SRC_SPWN_VLD" );
		_NbItems = ~0;
	}
}


/*
 * Constructor, multiple & path checking version
 */
CSourceSpawnPosValidator::CSourceSpawnPosValidator( uint32 aiInstanceNumber, const NLMISC::CVector& prospectingPos )
{
	_AisId = CWorldInstances::instance().getAISId( aiInstanceNumber );
	if ( _AisId.get() != 0 )
	{
		_MsgOut.setType( "SRC_SPWN_VLD" );
		//_MsgOut.serial( const_cast<CVector&>(prospectingPos) );
		_SizePos = (uint16)_MsgOut.reserve( sizeof(_NbItems) );
		_NbItems = 0;
	}
}

/*
 * Push pos and source datasetrow. Precondition: isValidationPossible().
 */
void		CSourceSpawnPosValidator::pushPosAndRow( const NLMISC::CVector2f& pos, const TDataSetRow& row )
{
	//nlassert( isValidationPossible() );
	_MsgOut.serial( const_cast<CVector2f&>(pos) );
	_MsgOut.serial( const_cast<TDataSetRow&>(row) );
	if ( _NbItems != ~0 )
		++_NbItems;
}


/*
 * Send validation request. Precondition: isValidationPossible().
 */
void		CSourceSpawnPosValidator::sendRequest()
{
	//nlassert( isValidationPossible() );
	if ( _NbItems != ~0 )
		_MsgOut.poke( _NbItems, (sint32)_SizePos );
	NLNET::CUnifiedNetwork::getInstance()->send( _AisId, _MsgOut ); // not via mirror because mirror not accessed by receiver
}



/*
 * CFgProspectionPhrase constructor
 */
CFgSearchPhrase::CFgSearchPhrase() : CForagePhrase()
{
	// Set default values
	_ForageTime = 150; // 15 s
	_ForageRange = 2.0f;
	_ForageAngle = 10.0f * (float)Pi / 180;
	_MultiSpotLimit = 1;
	_NbAttempts = 5; // by default, try up to 5 times to find a RM (except if prospection zone too small)
	_KnowledgePrecision = 0;
	_MaterialEnergy = 20;
	_FindExactMaterialEnergy = false;
	_EcotypeSpec = ECOSYSTEM::common_ecosystem;
	_MaterialGroupFilter = RM_GROUP::Unknown;
	_MaterialFamilyFilter = RM_FAMILY::Unknown;
	_CraftableItemPartFilter = ~0;
	_IsLocateDepositProspection = false;
	_PhraseType	= BRICK_TYPE::FORAGE_PROSPECTION;
	_IsStatic = true;
	_UsedSkillValue = 0;
}


/*
 *
 */
bool CFgSearchPhrase::build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute )
{
	H_AUTO(CFgSearchPhrase_build);
	
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
		
		// Compute Sabrina balance (and Sabrina cost)
		if ( brick.SabrinaValue > 0 )
			sabrinaCost += brick.SabrinaValue;
		else
			sabrinaCredit -= brick.SabrinaValue;

		if( brick.SabrinaRelativeValue > 0.0f )
			sabrinaRelativeCost += brick.SabrinaRelativeValue;
		else
			sabrinaRelativeCredit -= brick.SabrinaRelativeValue;
		
		// Decode property
		for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick.Params.begin(); ip!=brick.Params.end(); ++ip )
		{
			TBrickParam::IId* param = (*ip);
			switch ( param->id() )
			{
			case TBrickParam::FOCUS:
				INFOLOG("FOCUS: %i",((CSBrickParamFocus *)param)->Focus);
				_FocusCost += ((CSBrickParamFocus *)param)->Focus;
				break;
			case TBrickParam::FG_RANGE:
				INFOLOG("RANGE: %g",((CSBrickParamForageRange *)param)->Range);
				_ForageRange = ((CSBrickParamForageRange *)param)->Range;
				break;
			case TBrickParam::FG_LD_RANGE:
				INFOLOG("FG_LD_RANGE: %f",((CSBrickParamForageLocateDepositRange *)param)->Range);
				_ForageRange = ((CSBrickParamForageLocateDepositRange *)param)->Range;
				break;
			case TBrickParam::FG_ANGLE:
				INFOLOG("ANGLE: %u",((CSBrickParamForageAngle *)param)->Angle);
				_ForageAngle = ((float)(((CSBrickParamForageAngle *)param)->Angle)) * (float)Pi / 180.0f ;
				break;
			case TBrickParam::FG_MULTI:
				INFOLOG("MULTI: %u",((CSBrickParamForageMulti *)param)->Limit);
				_MultiSpotLimit = (uint16)((CSBrickParamForageMulti *)param)->Limit;
				break;
			case TBrickParam::FG_KNOW:
				INFOLOG("FG_KNOW: %i",((CSBrickParamForageKnowledge *)param)->Know);
				_KnowledgePrecision = ((CSBrickParamForageKnowledge *)param)->Know ;
				break;
			case TBrickParam::FG_TIME:
				INFOLOG("TIME: %u",((CSBrickParamForageTime *)param)->Time);
				_ForageTime = (TGameCycle)(((CSBrickParamForageTime *)param)->Time * 10.0f);
				break;
			case TBrickParam::FG_SRC_TIME:
				INFOLOG("FG_SRC_TIME: %f",((CSBrickParamForageSourceTime *)param)->Time);
				_SourceIniProperties.setProspectionExtraExtractionTime( (TGameCycle)(((CSBrickParamForageSourceTime *)param)->Time * 10.0f) ); // initial extraction time
				break;
			case TBrickParam::FG_STAT_ENERGY:
				INFOLOG("FG_STAT_ENERGY: %f",((CSBrickParamForageStatEnergy *)param)->StatEnergy);
				_MaterialEnergy = (uint8)(((CSBrickParamForageStatEnergy *)param)->StatEnergy);
				_FindExactMaterialEnergy = false;
				break;
			case TBrickParam::FG_STAT_ENERGY_ONLY:
				INFOLOG("FG_STAT_ENERGY_ONLY: %i",((CSBrickParamStatEnergyOnly *)param)->StatEnergyExact);
				_MaterialEnergy = (uint8)((CSBrickParamStatEnergyOnly *)param)->StatEnergyExact;
				_FindExactMaterialEnergy = true;
				break;
			/*case TBrickParam::FG_VIS_DIST:
				INFOLOG("FG_VIS_DIST: %f",((CSBrickParamForageVisDist *)param)->Dist);
				_SourceIniProperties.setDistVis( (sint32)(((CSBrickParamForageVisDist *)param)->Dist) );
				break;
			case TBrickParam::FG_VIS_STEALTH:
				INFOLOG("FG_VIS_STEALTH: %i",((CSBrickParamForageVisStealth *)param)->Mode);
				_SourceIniProperties.setStealthVis( VISIBILITY_RIGHTS::fromInt( ((CSBrickParamForageVisStealth *)param)->Mode ) );
				break;*/
			case TBrickParam::FG_ECT_SPC:
				INFOLOG("FG_ECT_SPC: %s",((CSBrickParamForageEcotypeSpec *)param)->Ecotype.c_str());
				_EcotypeSpec = ECOSYSTEM::stringToEcosystem( ((CSBrickParamForageEcotypeSpec *)param)->Ecotype );
				break;
			case TBrickParam::FG_RMGRP_FILT:
				INFOLOG("FG_RMGRP_FILT: %i",((CSBrickParamForageRMGroupFilter *)param)->Value);
				_MaterialGroupFilter = ((CSBrickParamForageRMGroupFilter *)param)->Value;
				break;
			case TBrickParam::FG_ATTEMPTS:
				INFOLOG("FG_ATTEMPTS: %i",((CSBrickParamForageAttempts *)param)->Nb);
				_NbAttempts = ((CSBrickParamForageAttempts *)param)->Nb;
				break;
			case TBrickParam::FG_RMFAM_FILT:
				INFOLOG("FG_RMFAM_FILT: %i",((CSBrickParamForageRMFamilyFilter *)param)->Value);
				_MaterialFamilyFilter = ((CSBrickParamForageRMFamilyFilter *)param)->Value;
				break;
			case TBrickParam::FG_ITEMPART_FILT:
				INFOLOG("FG_ITEMPART_FILT: %i",((CSBrickParamForageItemPartFilter *)param)->ItemPartIndex);
				_CraftableItemPartFilter = ((CSBrickParamForageItemPartFilter *)param)->ItemPartIndex;
				break;
			case TBrickParam::FG_SRC_LOCATOR:
				INFOLOG("FG_SRC_LOCATOR: %i",((CSBrickParamForageSourceLocator *)param)->Flag);
				_IsLocateDepositProspection = (((CSBrickParamForageSourceLocator *)param)->Flag == 1);
				break;
			default:;
			}
		}
		//nlerror( "TODO: Families" );
		//if ( brick.Family >= BRICK_FAMILIES::BeginForage
		
		//insertProgressingSkill( brick.Skill, brick.SheetId );
	}

	// Calculate actual focus cost
	CCharacter *player = (CCharacter*)CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if ( ! player )
	{
		return false;
	}
	_FocusCost = (sint32)(((float)_FocusCost) * (1.0f + player->wearMalus()));

	//egs_feinfo( "Forage phrase built" );
	if ( ForageDebug.get() == 10 )
	{
		_ForageTime = 10; // 1 s
		_ForageRange = 20; // 20 m
		_ForageAngle = (float)Pi;
		_MultiSpotLimit = 10; // up to 10 sources
		_SourceIniProperties.setLifetime( 1200 ); // 2 min (max)
		_SourceIniProperties.setProspectionExtraExtractionTime( 1200 ); // 2 min (max)
		testSabrinaBalance( (sint32)(sabrinaCredit * sabrinaRelativeCredit), (sint32)(sabrinaCost * sabrinaRelativeCost) );
		return true;
	}
	else
	{
		return testSabrinaBalance( (sint32)(sabrinaCredit * sabrinaRelativeCredit), (sint32)(sabrinaCost * sabrinaRelativeCost) );
	}
}


/*
 * evaluate phrase
 * \param evalReturnInfos struct that will receive evaluation results
 * \return true if eval has been made without errors
 */
bool CFgSearchPhrase::evaluate()
{
	return true;
}


/*
 *
 */
bool CFgSearchPhrase::validate()
{
	H_AUTO(CFgSearchPhrase_validate);
	
	if ( !HarvestSystemEnabled )
		return false;	
	// Get entity
	CCharacter *player = (CCharacter*)CEntityBaseManager::getEntityBasePtr( _ActorRowId );
	if (!player)
	{
		nlwarning("FG: Player character not found but his action still running!");
		return false;
	}

	// Test if entity can use action
	if (player->canEntityUseAction() == false)
	{
		return false;
	}
	
	// Entities cant forage if in combat
	TDataSetRow entityRowId = CPhraseManager::getInstance().getEntityEngagedMeleeBy( _ActorRowId );
	if (TheDataset.isAccessible(entityRowId))
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
	if ( ! (_IsLocateDepositProspection && player->getProspectionLocateDepositEffect()) ) // a deposit tracking cancellation does not take focus
	{
		const sint32 focus = player->getScores()._PhysicalScores[ SCORES::focus ].Current;
		if ( focus < _FocusCost)
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "PHRASE_NOT_ENOUGH_FOCUS");
			if ( ForageDebug.get() == 0 )
				return false;
		}
	}

	// Check death
	if (player->isDead())
	{
		return false;
	}

	// TODO: Check if on mount
	
	// OK
	//egs_feinfo( "Forage phrase validated" );
	return true;
}


/*
 *
 */
bool CFgSearchPhrase::update()
{
	return true;
}


/*
 *
 */
void CFgSearchPhrase::execute()
{
	H_AUTO(CFgSearchPhrase_execute);
	
	// Init phrase execution
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
	_ExecutionEndDate = time + _ForageTime;

	// Get character
	CCharacter* player = PlayerManager.getChar( _ActorRowId );
	if (!player)
		return;

	// Stopping a deposit location is immediate: no execution delay
	if ( _IsLocateDepositProspection && player->getProspectionLocateDepositEffect() )
		_ExecutionEndDate = time + 1;

	// Begin action
	player->setCurrentAction( CLIENT_ACTION_TYPE::Forage, _ExecutionEndDate );
	player->staticActionInProgress( true, STATIC_ACT_TYPES::Forage );

	// Get deposits and skill depending on current ecotype
	CVector playerPos( float(player->getState().X)/1000.0f, float(player->getState().Y)/1000.0f, 0.0f );
	CDeposit* aDepositAtPlayerPos = CZoneManager::getInstance().getFirstFoundDepositUnderPos( playerPos );
	if ( aDepositAtPlayerPos )
	{
		if ( aDepositAtPlayerPos->ecotype() != ECOSYSTEM::common_ecosystem )
		{
			SKILLS::ESkills usedSkill = getForageSkillByEcotype( aDepositAtPlayerPos->ecotype() );
			_UsedSkillValue = player->getSkillValue( usedSkill );

			// add modifier from consumable
			_UsedSkillValue += player->forageSuccessModifier(aDepositAtPlayerPos->ecotype()) + player->forageSuccessModifier(ECOSYSTEM::common_ecosystem);
			_UsedSkillValue = std::max(_UsedSkillValue,(sint32)0);
		}
		else
		{
			nlwarning( "FG: Deposit %s has no ecotype", aDepositAtPlayerPos->name().c_str() );
			return;
		}
	}
	// If player is not on a deposit but sources are on a deposit, _UsedSkillValue will remain 0 (=> low FX, short lifetime sources)

	// Set behaviour (warning: max prospection level = 250) 
	MBEHAV::CBehaviour behav( MBEHAV::PROSPECTING );
#ifdef NL_OS_WINDOWS
	if ( ForageDebug == 7 )
	{
		behav.ForageProspection.Range = ForageRange.get();
		behav.ForageProspection.Angle = ForageAngle.get();
		behav.ForageProspection.Level = ForageLevel.get();
	}
	else
#endif
	{
		behav.ForageProspection.Range = (_ForageRange > MaxForageRange) ? 127 : (uint8)(uint)((_ForageRange / MaxForageRange) * 127.0f); // 0..127
		behav.ForageProspection.Angle = (min((uint)((_ForageAngle / MaxForageAngle) * 4.0f), (uint)3)); // 0..3
		behav.ForageProspection.Level = (min((_UsedSkillValue * 5 / 250), 4)); // 0..4
	}
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	//egs_feinfo( "Forage phrase execution begins (time: %u s)", _ForageTime/10 );
}


/*
 * called at the end of the latency time
 */
void CFgProspectionPhrase::end()
{
	H_AUTO(CFgProspectionPhrase_end);
	
	CCharacter* player = PlayerManager.getChar(_ActorRowId); // checks isAccessible()
	if (!player)
		return;

	// Test forced failure
	if ( ! PHRASE_UTILITIES::forceActionFailure(player) )
	{
		// Always succeed, do not calculate successFactor
		// successFactor = (ForageDebug.get()<5) ? rollSuccessFactor( deltaLvl ) : 1.0f;

		// Don't send report for success
		//PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_SUCCESS"); // string localization not done
		//PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, "FORAGE_MISS");

		// Distance detection mode
		// As stopping the effect uses the same brick as starting it, it's cheaper to make a special action with it (and no other options) for the stop action!
		if ( _IsLocateDepositProspection )
		{
			if ( player->getProspectionLocateDepositEffect() )
				stopLocateDeposit( player );
			else
				startLocateDeposit( player );
		}
		else
		{
			// Set source life time, depending on _MultiSpotLimit ("MULTI") (previously: depending on player's prospection skill)
			// Skill	0	50	100		250
			// Time		10	35	1'		2'15
			//uint timeInSec = (_UsedSkillValue / 2) + 10;
			// Multi	1	2	5		10
			// Time		20	40	1'40	3'20  (see relation to extraction time)
			uint timeInSec = ((uint)_MultiSpotLimit) * 20;
			_SourceIniProperties.setLifetime( ((TGameCycle)(timeInSec)) * 10 );

			// Generate source(s)
			generateSources( player );
		}

		// Previously here: display stat about progression
	}

	player->clearCurrentAction();
	player->staticActionInProgress(false);

	//egs_feinfo( "Forage phrase end" );
}


/*
 * called when the action is interupted
 */
void CFgSearchPhrase::stop()
{
	H_AUTO(CFgSearchPhrase_stop);
	
	CCharacter* player = PlayerManager.getChar(_ActorRowId);
	if (!player)
		return;

	player->clearCurrentAction();
	player->staticActionInProgress(false);

	// Set behaviour (with all arguments to 0), only if execute() was called (no animation if stop() called after the first validate())
	if ( _ExecutionEndDate != 0 )
		PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, MBEHAV::PROSPECTING_END );

	// Don't send message
	//PHRASE_UTILITIES::sendDynamicSystemMessage(_ActorRowId, "FORAGE_CANCEL"); // string localization not done
	//egs_feinfo( "Forage phrase cancelled" ); // TEMP
}


/*
 *
 */
class CSEffectLocateDeposit : public CSEffect
{
public:
	inline CSEffectLocateDeposit( const TDataSetRow & creatorRowId,
		sint32 focusCost, // transmitted into effectValue
		uint8 power,
		const NLMISC::CVector2f& locatedPoint )
		: CSEffect ( creatorRowId, creatorRowId, EFFECT_FAMILIES::ForageLocateDeposit, false, focusCost, power ),
		_LocatedPoint(locatedPoint)
	{
		//_MagicFxType = MAGICFX::healtoMagicFx( _HealHp,_HealSap,_HealSta,true); /*TODO*/
	}

	/**
	 * apply the effects of the... effect
	 * \return true if the effect must be removed
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	///
	void endProspection( bool found=false )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _CreatorRowId, found ? "FORAGE_DEPOSIT_LOCATED" : "FORAGE_DEP_TRACKG_STOPPED" );
	}

private:

	NLMISC::CVector2f	_LocatedPoint;

};


/*
 *
 */
bool CSEffectLocateDeposit::update(CTimerEvent * event, bool )
{
	H_AUTO(CSEffectLocateDeposit_update);

	CCharacter *player = dynamic_cast<CCharacter*>(CEntityBaseManager::getEntityBasePtr( _CreatorRowId ));
	if ( ! player )
	{
		nlwarning("Invalid caster %u", _CreatorRowId.getIndex() );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	if ( ForageDebug.get() == 0 )
	{
		// Spend energies, or stop effect if not enough focus
		SCharacteristicsAndScores &focus = player->getScores()._PhysicalScores[SCORES::focus];
		if ( focus.Current() >= _Value)
			focus.Current = focus.Current - _Value;
		else
		{
			endProspection();
			player->setProspectionLocateDepositEffect( NULL );
			_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
			return true;
		}
	}

	// Calculate the new distance to the located point
	CVector2f playerPos;
	player->getState().getVector2f( playerPos );
	float dist = (_LocatedPoint - playerPos).norm();
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
	params[0].Int = (sint32)dist; // The 0 case is handled in the translation file
	PHRASE_UTILITIES::sendDynamicSystemMessage( _CreatorRowId, "FORAGE_DEPOSIT_DISTANCE", params );

	// Stop effect if we got nearer than 3 m of destination
	if ( dist < 3.0f )
	{
		endProspection( true );
		player->setProspectionLocateDepositEffect( NULL );
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		return true;
	}

	_UpdateTimer.setRemaining( ForageLocateDepositUpdateFrequency, event );

	return false;
}


/*
 *
 */
bool CFgProspectionPhrase::launch()
{
	// apply immediatly
	_ApplyDate = 0;
	return true;
}


/*
 *
 */
void CFgProspectionPhrase::apply()
{
	H_AUTO(CFgProspectionPhrase_apply);
	
	CCharacter* player = PlayerManager.getChar( _ActorRowId );
	if (!player)
		return;

	// Spend energies
	if ( ! (_IsLocateDepositProspection && player->getProspectionLocateDepositEffect()) ) // a deposit tracking cancellation does not spend focus
	{
		SCharacteristicsAndScores &focus = player->getScores()._PhysicalScores[SCORES::focus];
		if ( focus.Current != 0)
		{
			focus.Current = focus.Current - _FocusCost;
			if (focus.Current < 0)
				focus.Current = 0;
		}
	}

	//egs_feinfo( "Forage phrase applied" );

	// Set behaviour for animation 'state end'
	MBEHAV::CBehaviour behav = player->getBehaviour(); // keep arguments
	behav.Behaviour = MBEHAV::PROSPECTING_END;
	PHRASE_UTILITIES::sendUpdateBehaviour( _ActorRowId, behav );
	_LatencyEndDate = ForageSourceSpawnDelay.get(); // wait a short time before spawning the source(s) (to let animation/fx time)
}


#define MSG_REASON( r ) \
		if ( reasons.find( r ) != reasons.end() ) \
			reasonStr = NothingFoundReasonStrs[r];


/*
 * Generate one or more sources when applying the action. Return the number of sources spawned.
 */
uint CFgProspectionPhrase::generateSources( CCharacter *player )
{
	H_AUTO(CFgProspectionPhrase_generateSources);
	
	CVector playerPos( float(player->getState().X)/1000.0f, float(player->getState().Y)/1000.0f, float(player->getState().Z)/1000.0f ), pos;
	pos.z = 0; // to keep consistency with deposit maps
	bool fromNeighbourhood;
	fromNeighbourhood = (_ForageRange < 3.0f);
	if ( (_ForageRange < 3.0f) && (_ForageAngle < 0.35) ) // <21 degrees
		_NbAttempts = 1;

	// Get weather
	CRyzomTime::EWeather weather = WeatherEverywhere.getWeather( playerPos, CTimeDateSeasonManager::getRyzomTimeReference() );

	// Get AI instance for validation request to the AIS in charge of the continent
	CSourceSpawnPosValidator sourcePosValidator( player->getInstanceNumber(), playerPos );
		
	// Try to spawn several sources
	uint nbOfSources = _MultiSpotLimit; //RandomGenerator.rand( _MultiSpotLimit-1 ) + 1; // at least 1
#ifdef NL_DEBUG
	nldebug( "FG: Trying to generate %u sources...", nbOfSources );
#endif
	set<TNothingFoundReason> reasons;
	uint nbFound = 0;
	OptFastFloorBegin(); // for CNoiseValue called in selectRMAtPos()
	for ( uint iSource=0; iSource!=nbOfSources; ++iSource )
	{
		TNothingFoundReason reason;
		const CStaticDepositRawMaterial *rawMaterial;
		CDeposit *deposit, *depositForK;

		// Make several attempts to find a pos that matches the filters
		for ( uint iAttempt=0; iAttempt!=(uint)_NbAttempts; ++iAttempt )
		{
			// Randomize a position in the prospection cone
			float angle = RandomGenerator.frandPlusMinus( _ForageAngle/2.0f ); // relative to the player's heading
			float radius = max( 2.0f, RandomGenerator.frand( _ForageRange ) ); // at least 2 m

			// Calculate source position (TODO: Z)
			angle += player->getState().Heading();
			pos.x = playerPos.x + radius * (float)cos( angle );
			pos.y = playerPos.y + radius * (float)sin( angle );
#ifdef NL_DEBUG
			nldebug( "FG: Source pos: %s", pos.asString().c_str() );
#endif
			// Select a raw material
			rawMaterial = selectRMAtPos(
				pos, weather, _EcotypeSpec,
				_MaterialGroupFilter, _MaterialFamilyFilter, _CraftableItemPartFilter,
				_MaterialEnergy, _FindExactMaterialEnergy, fromNeighbourhood, true, &deposit, &depositForK, reason );
			if ( rawMaterial || (!deposit) )
				break; // stop attempts if RM found (rawMaterial) or impossible to find one (!deposit)
		}

		if ( rawMaterial )
		{
			// Find or open a forage site //nlassert( deposit && depositForK && forageSite );
			CRecentForageSite *forageSite = deposit->findOrCreateForageSite( pos );
			if ( forageSite->addActiveSource() )
			{
				// Quantity ratio: 10%-30% of MaxStackable (now ecotype spec does not impact on it but on extracted qtty)
				float quantityRatio = 0.10f + RandomGenerator.frand( 0.20f );

				// Make and spawn the source
				CHarvestSource *hsource = new CHarvestSource();
				//nldebug( "+++ %p from %p", hsource, forageSite );
				CVector2f pos2f = pos;
				if ( hsource->init( _SourceIniProperties, pos2f, forageSite, depositForK, rawMaterial, quantityRatio ) )
				{
					if ( _EcotypeSpec != ECOSYSTEM::common_ecosystem )
						hsource->setBonusForA( 20 );
					
					if ( hsource->spawnBegin( _KnowledgePrecision, player->getEntityRowId(), false ) )
					{
	#ifdef NL_DEBUG
						nldebug( "FG: Source spawned with RM %s, quantity %g (validation pending)", hsource->materialSheet().toString().c_str(), hsource->quantity() );
	#endif
						++nbFound;
						
						// add item special effect
						if ( player )
						{
							std::vector<SItemSpecialEffect> effects = player->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_FORAGE_NO_RISK);
							std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
							double addedQty = 0.;
							for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
							{
								float rnd = RandomGenerator.frand();
								if (rnd<it->EffectArgFloat[0])
								{
									hsource->setSafeSource(true);
									PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_FORAGE_NO_RISK, player);
								}
							}
						}
						
 						if ( sourcePosValidator.isValidationPossible() )
						{
							// Store pos for validation
							sourcePosValidator.pushPosAndRow( pos2f, hsource->rowId() );
						}
						else
						{
							// If no corresponding AIS is online, the position is assumed good and spawning is completed
							hsource->spawnEnd( true );
						}
					}
					else
					{
						nlwarning( "FG: Unable to spawn forage source (mirror range full?)" );
						forageSite->removeActiveSource();
						delete hsource;
						reasons.insert( NFCantSpawnSource );
					}
				}
				else
				{
					// The available quantity has fallen to 0
					forageSite->removeActiveSource();
					delete hsource;
					reasons.insert( NFDepositDepleted );
				}
			}
			else
			{
				// Would mean a lot of sources in the same place!
				nlwarning( "FG: Can't add more sources to forage site in %s", pos.asString().c_str() );
				reasons.insert( NFCantSpawnSource );
			}
		}
		else
		{
			reasons.insert( reason );
		}
	}
	OptFastFloorEnd();

	if ( nbFound != 0 )
	{
		if ( sourcePosValidator.isValidationPossible() )
		{
			// Send a request to the AIS in charge of the continent (if online), to check the positions.
			sourcePosValidator.sendRequest();
		}
		else
		{
			// If no corresponding AIS is online, report result to the player (should not occur in normal conditions)
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = (sint32)nbFound;
			PHRASE_UTILITIES::sendDynamicSystemMessage( player->getEntityRowId(), "FORAGE_FOUND_SOURCES", params );
		}
	}
	else
	{
		// Message to explain why nothing has been found
		const char *reasonStr = NULL;

		// RMs could be found but the site is depleted
		// => "Some raw materials used to be present here, but the place seems exhausted."
		MSG_REASON( NFSiteDepleted )

		// A matching deposit has been found but stat does not match exactly at the random points
		// => the player has to retry of move a little to find
		// => "Some raw materials were detected here, but they have a different class than the materials searched by your current prospecting action. Some matching materials were detected near by, though."
		else MSG_REASON( NFStatEnergyDifferentLocal )

		// A matching deposit has been found but stat is too high at the random points
		// => the player has to retry of move a little to find
		// => "Some raw materials were detected here, but they have a higher class than your current prospecting action allows to find. Some lower class materials were detected near by, though."
		else MSG_REASON( NFStatEnergyTooHighLocal )

		// A deposit with matching group/family filter has been found but the RMs at the random points didn't match
		// => the player has to retry of move a little to find
		// => "Some specified raw materials were detected near by, though."
		else MSG_REASON( NFNoLocalMaterialForFilter )

		// No deposit found with matching group/family filter at the random points
		// => "No specified raw materials are present in the area."
		else MSG_REASON( NFNoDepositForFilter )

		// No deposit found with exactly matching stat at the random points
		// => "You could detect the presence of raw materials, but they have a different class than the materials searched by your current prospecting action."
		else MSG_REASON( NFStatEnergyDifferent )
		
		// No deposit found with matching stat at the random points
		// => "You could detect the presence of raw materials, but they have a higher class than your current prospecting action allows to find."
		else MSG_REASON( NFStatEnergyTooHigh )

		// Ecotype spec not matching any deposit at the random points
		// => "The terrain specialization cannot be used here."
		else MSG_REASON( NFInvalidEcotype )

		// Deposits with time/weather not matching at the random points
		// => "No specified raw materials are present in the area in this season/at the moment/in these weather conditions."
		else MSG_REASON( NFInvalidCurrentSeason )
		else MSG_REASON( NFInvalidCurrentTimeOfDay )
		else MSG_REASON( NFInvalidCurrentWeather )

		// No deposit found at the random points
		// "No raw materials were detected in this area."
		else MSG_REASON( NFNoDepositHere )

		// The quantity in the deposit is 0
		// "Some raw materials used to be present here, but all have been pulled out and the deposit is now empty for a while."
		else MSG_REASON( NFDepositDepleted )

		// No more sources can be spawned!
		// "No more forage sources can be seen at the moment."
		else MSG_REASON( NFCantSpawnSource )

		if ( reasonStr )
			PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, reasonStr );
	}

	return nbFound;
}


/*
 *
 */
struct TDepositLoc
{
	TDepositLoc( float d=0, const NLMISC::CVector& np=NLMISC::CVector(), bool i=false ) : Distance(d), NearestPos(np), Inside(i) {}

	float	Distance;
	CVector	NearestPos;
	bool	Inside;
};


/*
 * Precondition: player->getProspectionLocateDepositEffect() is null
 */
void CFgProspectionPhrase::startLocateDeposit( CCharacter *player )
{
	H_AUTO(CFgProspectionPhrase_startLocateDeposit);
	
	// Get deposits in region
	CVector playerPos( float(player->getState().X)/1000.0f, float(player->getState().Y)/1000.0f, 0.0f ), pos;
	CRegion *currentRegion = CZoneManager::getInstance().getRegion( playerPos );
	if ( ! currentRegion )
		return; // warning in getRegion()

	// Eliminate deposits not within range
	map< CDeposit*, TDepositLoc > depositLocRepository;
	vector<CDeposit*> matchingDeposits;
	for ( vector<CDeposit*>::iterator itd=currentRegion->getDeposits().begin(); itd!=currentRegion->getDeposits().end(); ++itd )
	{
		CDeposit *deposit = (*itd);
		NLMISC::CVector locatedPos;
		float dist;
		bool inside = deposit->contains( playerPos, dist, locatedPos, false );
		if ( inside || (dist < _ForageRange) )
		{
			matchingDeposits.push_back( deposit );
			depositLocRepository.insert( make_pair( deposit, TDepositLoc( dist, locatedPos, inside ) ) );
		}
	}

	// Filter the deposits using current context and action's constraints
	CRyzomTime::EWeather weather = WeatherEverywhere.getWeather( playerPos, CTimeDateSeasonManager::getRyzomTimeReference() );
	vector<CDeposit*>::iterator matchingEnd;
	TNothingFoundReason reason = NFUnknown;
	filterDeposits( matchingDeposits, weather, _EcotypeSpec,
		_MaterialGroupFilter, _MaterialFamilyFilter, _CraftableItemPartFilter,
		_MaterialEnergy, _FindExactMaterialEnergy, matchingEnd, reason );
	if ( reason != NFUnknown )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _ActorRowId, NothingFoundReasonStrs[reason] );
		nldebug( "There is no deposit in range" );
	}
	else
	{
		// Retain the first found matching deposit in which we are, or the nearest matching deposit
		TDepositLoc *retainedLoc = NULL;
		float minDist = _ForageRange;
		for ( vector<CDeposit*>::iterator itd=matchingDeposits.begin(); itd!=matchingEnd; ++itd )
		{
			CDeposit *deposit = (*itd);
			TDepositLoc& depositLoc = depositLocRepository[deposit];
			if ( depositLoc.Inside )
			{
				depositLoc.Distance = 0;
				depositLoc.NearestPos = CVector2f( playerPos );
				retainedLoc = &depositLoc;
				break;
			}
			else if ( depositLoc.Distance < minDist )
			{
				minDist = depositLoc.Distance;
				retainedLoc = &depositLoc;
			}
		}

		// Start the effect
		CVector2f locatedPoint( retainedLoc->NearestPos );
		TReportAction report;
		sint32 effectFocusCostByUpdate = _FocusCost / ForageFocusRatioOfLocateDeposit.get();
		if ( _EcotypeSpec != ECOSYSTEM::common_ecosystem )
			effectFocusCostByUpdate /= 2; // lower focus consumption with terrain specialization
		CSEffectLocateDeposit *effect = new CSEffectLocateDeposit(
			player->getEntityRowId(),
			effectFocusCostByUpdate, 1 /*not used: power*/, locatedPoint );
		player->addSabrinaEffect( effect ); // the linked entity
		player->setProspectionLocateDepositEffect( effect );
	}
}


/*
 * Precondition: player->getProspectionLocateDepositEffect() not null
 */
void CFgProspectionPhrase::stopLocateDeposit( CCharacter *player )
{
	H_AUTO(CFgProspectionPhrase_stopLocateDeposit);
	
	CSEffectLocateDeposit *effect = (CSEffectLocateDeposit*)(CSEffect*)(player->getProspectionLocateDepositEffect());
	effect->endProspection();
	effect->stopEffect();
	player->setProspectionLocateDepositEffect( NULL );
}


/**
 *
 */
struct COpenForProspectionPred : public std::unary_function< CDeposit*, bool >
{
	/// Predicate
	bool operator() ( CDeposit* deposit ) const
	{
		return (deposit->enabled() && deposit->canProspect());
	}
};


/**
 *
 */
struct CContextMatchPred : public std::unary_function< CDeposit*, bool >
{
	enum TContextStatus { CSMatching=0, CSSeason=1, CSTimeOfDay=2, CSWeather=4 };

	/// Constructor
	CContextMatchPred( const CRyzomTime::EWeather& weather, uint *pStatusBitfield )
	{
		if ( ForageForcedSeason.get() != -1 )
			Season = (CRyzomTime::ESeason)(uint)(ForageForcedSeason.get());
		else
			Season = CTimeDateSeasonManager::getRyzomTimeReference().getRyzomSeason();
		if ( ForageForcedDaycycle.get() != -1 )
			TimeOfDay = (CRyzomTime::ETimeOfDay)(uint)ForageForcedDaycycle.get();
		else
			TimeOfDay = CTimeDateSeasonManager::getDayCycle();
		if ( ForageForcedWeather.get() != -1 )
			Weather = (CRyzomTime::EWeather)(uint)ForageForcedWeather.get();
		else
			Weather = weather;
		StatusBitfield = pStatusBitfield; // pointer because struct seems passed by value!!!
		*StatusBitfield = CSMatching;
	}

	/// Predicate
	bool operator() ( CDeposit* deposit )
	{
		if ( ! deposit->matchSeason( Season ) )
		{
			*StatusBitfield |= CSSeason;
			return false;
		}
		else if ( ! deposit->matchTimeOfDay( TimeOfDay ) )
		{
			*StatusBitfield |= CSTimeOfDay;
			return false;
		}
		else if ( ! deposit->matchWeather( Weather ) )
		{
			*StatusBitfield |= CSWeather;
			return false;
		}
		return true;
	}

	CRyzomTime::ESeason		Season;
	CRyzomTime::ETimeOfDay	TimeOfDay;
	CRyzomTime::EWeather	Weather;

	uint					*StatusBitfield;
};


/*
 * Spawn a source automatically from the deposit, if it matches the current context
 */
bool CFgProspectionPhrase::autoSpawnSource( const NLMISC::CVector& pos, CDeposit *deposit )
{
	H_AUTO(CFgProspectionPhrase_autoSpawnSource);
	
	// Get weather
	CRyzomTime::EWeather weather = WeatherEverywhere.getWeather( pos, CTimeDateSeasonManager::getRyzomTimeReference() );

	// Check weather, season, time of day...
	uint status;
	CContextMatchPred contextCheck( weather, &status );
	if ( ! contextCheck( deposit ) )
		return false;

	// Get AI instance for validation request to the AIS in charge of the continent (position -> continent by name -> instance number (may be slow))
	CContinent *continent = CZoneManager::getInstance().getContinent( pos );
	uint32 aiInstance = CUsedContinent::instance().getInstanceForContinent( (CONTINENT::TContinent)continent->getId() );
	CSourceSpawnPosValidator sourcePosValidator( aiInstance );

	// Get a random raw material among the ones availables in this deposit
	bool isDepleted = false;
	const CStaticDepositRawMaterial *rawMaterial = deposit->getRMAtPos( pos, true, isDepleted );
	if ( rawMaterial )
	{
		// Find or open a forage site //nlassert( deposit );
		CRecentForageSite *forageSite = deposit->findOrCreateForageSite( pos );
		if ( forageSite->addActiveSource() )
		{
			// Make and spawn the source
			CHarvestSource *hsource = new CHarvestSource();
			//nldebug( "+++ %p from %p", hsource, forageSite );
			float quantityRatio = 0.10f + RandomGenerator.frand( 0.10f ); // 10%-20% of MaxStackable
			const CAutoSpawnProperties *props = deposit->getAutoSpawnProperties();
			if ( props )
			{
				/* Add a random of 30 sec in lifetime (300 ticks).
					This is to smooth the respawn across time for deposits that have a 
					big number of "minimal auto spawned source".
					By doing this, the sources will smoothly unspawn in 30 seconds, and then respawn 
					one by one each tick.
				*/
				TGameCycle	lifeTime= (TGameCycle)props->LifeTimeGc;
				lifeTime+= (TGameCycle)RandomGenerator.rand(300);
				AutoSpawnSourceIniProperties.setLifetime( lifeTime );
				AutoSpawnSourceIniProperties.setProspectionExtraExtractionTime( 0 /*(TGameCycle)props->ExtractionTimeGc*/ );
			}
			else
				nlwarning( "FG: Auto-spawn for deposit with auto-spawn off" );

			// NOTE: Unlike with prospection, the depositForK is not the one for which K is the lowest
			// at the position, but it's the deposit for auto-spawn
			CVector2f pos2f = pos;
			hsource->init( AutoSpawnSourceIniProperties, pos2f, forageSite, deposit, rawMaterial, quantityRatio );
			
			// Bkup deposit for auto spawn
			hsource->setDepositAutoSpawn(deposit);

			TDataSetRow dsr;
			if ( hsource->spawnBegin( 0, dsr, true ) ) // minimal knowledge
			{
#ifdef NL_DEBUG
				nldebug( "FG: Source auto-spawned in %s at %s with RM %s, quantity %g (validation pending)", deposit->name().c_str(), pos.toString().c_str(), hsource->materialSheet().toString().c_str(), hsource->quantity() );
#endif
 				if ( sourcePosValidator.isValidationPossible() )
				{
					// Store pos for validation & send request
					sourcePosValidator.pushPosAndRow( pos2f, hsource->rowId() );
					sourcePosValidator.sendRequest();
				}
				else
				{
					// If no corresponding AIS is online, the position is assumed good and spawning is completed
					hsource->spawnEnd( true );
				}
				return true;
			}
			else
			{
				forageSite->removeActiveSource();
				delete hsource;
			}
		}
	}
	return false;
}


/*
 * Reorder matchingDeposits so that [begin(), matchingEnd] is the range matching the filters.
 * If no deposit matches the filter, return a reason different from NFUnknown.
 */
void CFgProspectionPhrase::filterDeposits(
	std::vector<CDeposit*>& matchingDeposits,
	const CRyzomTime::EWeather& weather,
	const TEcotype& ecotypeSpec,
	const RM_GROUP::TRMGroup& groupFilter,
	const RM_FAMILY::TRMFamily& familyFilter,
	uint craftableItemPartFilter,
	uint8 statEnergy,
	bool findExactStatEnergy,
	std::vector<CDeposit*>::iterator& matchingEnd,
	TNothingFoundReason& reason )
{
	H_AUTO(CFgProspectionPhrase_filterDeposits);
	
	// Filter the deposits with 'enabled' and 'can prospect' status
	matchingEnd = partition( matchingDeposits.begin(), matchingDeposits.end(), COpenForProspectionPred() );
	if ( matchingEnd == matchingDeposits.begin() )
	{
		reason = NFNoDepositHere;
		return;
	}

	// Filter the remaining deposits with ecotype specialization (if any)
	if ( ecotypeSpec != ECOSYSTEM::common_ecosystem )
	{
		matchingEnd = partition( matchingDeposits.begin(), matchingEnd, bind2nd( mem_fun(&CDeposit::matchEcotype), ecotypeSpec ) );
		if ( matchingEnd == matchingDeposits.begin() )
		{
			reason = NFInvalidEcotype;
			return;
		}
	}

	// Filter the remaining deposits with group and family filters, and item part filter
	if ( craftableItemPartFilter != ~0 )
	{
		matchingEnd = partition( matchingDeposits.begin(), matchingEnd, bind2nd( mem_fun(&CDeposit::hasRMForItemPart), craftableItemPartFilter ) );
	}
	if ( familyFilter != RM_FAMILY::Unknown )
	{
		matchingEnd = partition( matchingDeposits.begin(), matchingEnd, bind2nd( mem_fun(&CDeposit::hasFamily), familyFilter ) );
	}
	else
	{
		if ( groupFilter != RM_GROUP::Unknown )
		{
			matchingEnd = partition( matchingDeposits.begin(), matchingEnd, bind2nd( mem_fun(&CDeposit::hasGroup), groupFilter ) );
		}
	}
	if ( matchingEnd == matchingDeposits.begin() )
	{
		reason = NFNoDepositForFilter;
		return;
	}

	/*nldebug( "5 first matching deposits before stat energy filter:" );
	for ( uint i=0; i!=min((uint)(matchingEnd-matchingDeposits.begin()), (uint)5); ++i )
	{
		matchingDeposits[i]->displayContent( NLMISC::DebugLog );
	}*/

	// Filter the remaining deposits with max stat energy

	matchingEnd = partition( matchingDeposits.begin(), matchingEnd,
		bind2nd( mem_fun( findExactStatEnergy ? &CDeposit::hasExactStatEnergy : &CDeposit::hasLowerStatEnergy ), statEnergy ) );
	if ( matchingEnd == matchingDeposits.begin() )
	{
		if ( findExactStatEnergy )
			reason = NFStatEnergyDifferent;
		else
			reason = NFStatEnergyTooHigh; // can get here also if the deposit have no RM in their vector!
		return;
	}

	// Filter the deposits with current context (match season & weather & time of day)
	uint statusBitfield;
	CContextMatchPred contextMatchPred( weather, &statusBitfield );
	matchingEnd = partition( matchingDeposits.begin(), matchingEnd, contextMatchPred ); // warning: contextMatchPred is passed by value!!!
#ifdef NL_DEBUG
	nldebug( "FG: Matching deposits: ALL %u, CONTEXT %u...", matchingDeposits.size(), matchingEnd - matchingDeposits.begin() );
#endif
	if ( matchingEnd == matchingDeposits.begin() )
	{
		if ( (statusBitfield & CContextMatchPred::CSWeather) != 0 ) // if there's a deposit with matching season and time, report invalid weather
			reason = NFInvalidCurrentWeather;
		else if ( (statusBitfield & CContextMatchPred::CSTimeOfDay) != 0 ) // if there's a deposit with matching season, report time
			reason = NFInvalidCurrentTimeOfDay;
		else // if ( (statusBitfield & CContextMatchPred::CSSeason) != 0 ) // if there's no deposit with matching season, report season
			reason = NFInvalidCurrentSeason;
		return;
	}
}


/*
 * Select a raw material that can be found at the specified position. (static)
 * OptFastFloorBegin()/OptFastFloorEnd() must enclose one or more calls to this method.
 *
 * \param sourcePos The specified position
 * \param matchingDeposits The deposits found at the position (modified by filtering, only compatible deposits remain)
 * \param weather The current weather
 * \param ecotypeSpec The ecotype specialization, or ECOSYSTEM::common
 * \param groupFilter If not Unknown, searches only a RM from the group (ignored if familyFilter is not Unknown)
 * \param familyFilter If not Unknown, searches only a RM from the family
 * \param craftableItemPartFilter If not ~0, searches only a RM useful to craft the specified item part
 * \param maxStatEnergy Discards RM that have a higher energy than specified 
 * \param fromNeighboorhood If true, selects randomly a RM from a small area, otherwise returns only a RM at the exact pos
 * \param testIfSiteDepleted Set it to false for map generation.
 *
 * \param deposit If a RM is found, deposit will point to the deposit containing the RM. If no RM is found,
 *   deposit will be NULL if finding a RM with the specified filters is impossible in this area in
 *   the current context (season, weather, time of day...), or non-NULL if another attempt is needed to find one.
 * \param depositForK The deposit at the position, matching the context, that has the low kami anger level
 *   (NULL if deposit is NULL).
 * \param reason The reason why nothing has been found, if the method returns NULL.
 * \return If a RM is found, returns it, otherwise NULL.
 */
const CStaticDepositRawMaterial *CFgProspectionPhrase::selectRMAtPos(
	const NLMISC::CVector& sourcePos,
	const CRyzomTime::EWeather& weather,
	const TEcotype& ecotypeSpec,
	const RM_GROUP::TRMGroup& groupFilter,
	const RM_FAMILY::TRMFamily& familyFilter,
	uint craftableItemPartFilter,
	uint8 statEnergy,
	bool findExactStatEnergy,
	bool fromNeighbourHood,
	bool testIfSiteDepleted,
	CDeposit **deposit,
	CDeposit **depositForK,
	TNothingFoundReason& reason,
	bool ignoreContextMatch )
{
	H_AUTO(CFgProspectionPhrase_selectRMAtPos);
	
	*deposit = NULL;
	reason = NFUnknown;

	// Find all the deposits at this position
	vector<CDeposit*> matchingDeposits;
	CZoneManager::getInstance().getDepositsUnderPos( sourcePos, matchingDeposits, WarnIfOutsideOfRegion );
	if ( matchingDeposits.empty() )
	{
		reason = NFNoDepositHere;
		return NULL;
	}

	// Filter the deposits using availability ('enabled', 'can prospect', current context) and action
	// criteria (ecotype specialization, group/family filters, item part filter, stat energy)
	vector<CDeposit*>::iterator matchingEnd;
	if ( ignoreContextMatch )
	{
		matchingEnd = matchingDeposits.end();
	}
	else
	{
		filterDeposits( matchingDeposits, weather, ecotypeSpec, groupFilter, familyFilter,
			craftableItemPartFilter, statEnergy, findExactStatEnergy, matchingEnd, reason );
		if ( reason != NFUnknown )
		{
			return NULL;
		}
	}

	// Now we are sure the deposit(s) before matchingEnd contain(s) raw materials compliant with the filters (not exclusively).
	// Select a random deposit among them. If not found, another attempt will be necessary.
	uint nbMatching = (uint)(matchingEnd - matchingDeposits.begin());
	*deposit = matchingDeposits[RandomGenerator.rand((uint16)(nbMatching-1))];

	// Find the deposit, among the matching ones, for which the kami anger level is the lowest (nlassert(matchingEnd!=matchingDeposits.begin())
	*depositForK = *min_element( matchingDeposits.begin(), matchingEnd, CHasLowerKamiAngerPred() );

	// Get a random raw material among the ones available at this position
	bool isDepleted = false;
	const CStaticDepositRawMaterial *rawMaterial = /*fromNeighbourHood ? (*deposit)->getRandomRMAtPos( sourcePos, testIfSiteDepleted, isDepleted ) :*/
		(*deposit)->getRMAtPos( sourcePos, testIfSiteDepleted, isDepleted ); // now, always use getRMAtPos()
	if ( rawMaterial )
	{
		// Check if the RM found at the position matches the filters (except context, already excluded)
		const CAllStaticItems& allItems = CSheets::getItemMapForm();
		CAllStaticItems::const_iterator it = allItems.find( rawMaterial->MaterialSheet );
		if ( (it != allItems.end()) && (*it).second.Mp )
		{
			const CStaticItem& staticItem = (*it).second;

			// Check family
			if ( familyFilter != RM_FAMILY::Unknown )
			{
				if ( staticItem.Mp->Family != familyFilter )
				{
					reason = NFNoLocalMaterialForFilter;
					return NULL;
				}
			}
			// Check group
			else if ( groupFilter != RM_GROUP::Unknown )
			{
				if ( staticItem.Mp->getGroup() != groupFilter )
				{
					reason = NFNoLocalMaterialForFilter;
					return NULL;
				}
			}
			// Check craftable item part
			if ( craftableItemPartFilter != ~0 )
			{
				const CMP::TMpFaberParameters *mpFaberParam = staticItem.Mp->getMpFaberParameters( craftableItemPartFilter );
				if ( ! (mpFaberParam && (mpFaberParam->Durability != 0)) )
				{
					reason = NFNoLocalMaterialForFilter;
					return NULL;
				}
			}
			// Check stat energy
			if ( findExactStatEnergy )
			{
				if ( staticItem.Mp->StatEnergy != statEnergy )
				{
					reason = NFStatEnergyDifferentLocal;
					return NULL;
				}
			}
			else
			{
				if ( staticItem.Mp->StatEnergy > statEnergy )
				{
					reason = NFStatEnergyTooHighLocal;
					return NULL;
				}
			}
		}
		else
		{
#ifdef NL_DEBUG
			nlwarning( "FG: Raw material %s not found in item map", rawMaterial->MaterialSheet.toString().c_str() );
#endif
			return NULL; // should not occur
		}
	}
	if ( isDepleted )
		reason = NFSiteDepleted;
	return rawMaterial;
}


/*
 * static
 */
void CFgProspectionPhrase::displayRMNearBy( CCharacter *player, bool onlyAtExactPos, bool extendedInfo, NLMISC::CLog *log )
{
	H_AUTO(CFgProspectionPhrase_displayRMNearBy);
	
	CVector pos( float(player->getState().X)/1000.0f, float(player->getState().Y)/1000.0f, 0.0f );

	// Find all the deposits at this position
	if ( onlyAtExactPos )
		log->displayNL( "All possible RMs at exact pos (see 'nearby version' for context info):" );
	vector<CDeposit*> matchingDeposits;
	CZoneManager::getInstance().getDepositsUnderPos( pos, matchingDeposits );
	OptFastFloorBegin();
	uint nbDeposits = 0;
	for( vector<CDeposit*>::const_iterator it=matchingDeposits.begin(); it!=matchingDeposits.end(); ++it )
	{
		if ( onlyAtExactPos )
		{
			bool isDepleted = false;
			const CStaticDepositRawMaterial *rawMaterial = (*it)->getRMAtPos( pos, false, isDepleted );
			if ( rawMaterial )
			{
				log->displayRawNL( "%s", rawMaterial->MaterialSheet.toString().c_str() );
				++nbDeposits;
			}
		}
		else
		{
			(*it)->displayContent( log, extendedInfo );
		}
	}
	log->displayRawNL( "(within %u deposits at this position)", nbDeposits );
	OptFastFloorEnd();
}



NLMISC_DYNVARIABLE( uint32, RyzomSeason, "Get season number (0=Spring)" )
{
	if ( get )
		*pointer = CTimeDateSeasonManager::getRyzomTimeReference().getRyzomSeason();
}


/*--------------------------------------------------------------------------------------------------------*/
#define DEPOSIT_MAP_GENERATION 

#ifdef DEPOSIT_MAP_GENERATION

#include "game_share/bmp4image.h"
#include <nel/misc/words_dictionary.h>

typedef std::map< std::string, pair< pair< float, float >, uint > > CSUMap;

CWordsDictionary *WordDic = NULL;

/*
 * Utility class for makeDepositMap command
 */
class CRMColorMapping
{
public:

	///
	CRMColorMapping() : _ColorIndex(0) {}

	///
	uint16	getColorMapping( const NLMISC::CSheetId rmId )
	{
		if ( rmId == CSheetId::Unknown )
		{
			return 0;
		}
		else
		{
			string sheetCode = rmId.toString();
			string sheetCodeLeft = sheetCode.substr( 0, 5 ); // only family
			float qualityLightFactor = 0.25f + (((float)(uint)(sheetCode[9] - 'b' + 1)) / 5.0f / 2.0f); // from 0.25 to 0.75
			if ( qualityLightFactor > 1.0f )
			{
				nlwarning( "Invalid quality for %s", sheetCode.c_str() );
				return 0;
			}
			CSUMap::iterator it = _Mapping.find( sheetCodeLeft );
			if ( it != _Mapping.end() )
			{
				// Return existing color for rmId (with shade expressing quality)
				++((*it).second.second);
				pair<float,float> colorInfo = (*it).second.first;
				CRGBA color;
				color.buildFromHLS( colorInfo.first, qualityLightFactor, colorInfo.second );
				return CTGAImage::getColor16( color.R, color.G, color.B );
			}
			else
			{
				// Map a new color for new rmId
				static const float HuePalette [7] = { 0.0f, 30.0f, 60.0f, 125.0f, 184.0f, 245.0f, 294.0f };
				float colorIndex = (float)(_ColorIndex++);
				float h = HuePalette[_ColorIndex % 7];
				if ( _ColorIndex > 6 )
					h += ((float)(_ColorIndex / 7)) * 13.0f;
				float s = 0.5f + RandomGenerator.frand( 0.5f );
				_Mapping.insert( make_pair( sheetCodeLeft, make_pair( make_pair( h, s ), 1 ) ) );
				CRGBA color;
				color.buildFromHLS( h, qualityLightFactor, s );
				return CTGAImage::getColor16( color.R, color.G, color.B );
			}
		}
	}

	///
	void	displayFrequencies( NLMISC::CLog& log )
	{
		uint total = 0;
		for ( CSUMap::const_iterator it=_Mapping.begin(); it!=_Mapping.end(); ++it )
			total += (*it).second.second;
		log.displayNL( "Frequencies of the %u RM families (%u total pts):", _Mapping.size(), total );
		for ( CSUMap::const_iterator it=_Mapping.begin(); it!=_Mapping.end(); ++it )
		{
			log.displayRawNL( "%s (hue %u) -> %u pts (%.1f%%)", (*it).first.c_str(), (uint)((*it).second.first.first), (*it).second.second, (float)(*it).second.second * 100.0f / (float)total );
		}
	}

private:

	///
	CSUMap		_Mapping;

	///
	uint		_ColorIndex;
};


/*
 * makeDepositMap command
 */
NLMISC_COMMAND( makeDepositMap, "Write a map of the raw materials at a position", "<_playerRowId | posX,posY | depositName> <outputFilename.tga> [<resolution=0.5> [<width=200> [<neighboorhoodMode=0> [<displayColorFrequencies=0> [<onlySpecifiedDeposit>]]]]" )
{
	H_AUTO(makeDepositMap);
	
	// Default arguments
	float resolution = 0.50f; // 0.5 m
	float zoneWidth = 200.0f; // 200 m
	bool neighbourhood = false;
	bool displayColorFrequencies = false;
	bool onlySpecifiedDeposit = false;

	// Read arguments
	if ( args.size() < 2 )
		return false;
	string outputFilename = args[1].c_str();
	if ( args.size() > 2 )
	{
		resolution = (float)atof( args[2].c_str() );
		if ( args.size() > 3 )
		{
			zoneWidth = (float)atof( args[3].c_str() );
			if ( args.size() > 4 )
			{
				neighbourhood = (args[4] == "1");
				if ( args.size() > 5 )
				{
					displayColorFrequencies = (args[5] == "1");
					if ( args.size() > 6 )
						onlySpecifiedDeposit = (args[6] == "1");
				}
			}
		}
	}
	uint16 dim = (uint16)(uint)(zoneWidth / resolution);
	const CDeposit *depositToScan = NULL;
	
	// Compute bounds
	TDataSetRow playerRowId;
	CVector leftCornerPos, rightCornerPos;
	string::size_type p;
	if ( (p = args[0].find( ',' )) != string::npos )
	{
		// Get specified position
		CVector centerPos;
		centerPos.x = (float)atof( args[0].substr( 0, p ).c_str() );
		centerPos.y = (float)atof( args[0].substr( p+1 ).c_str() );
		leftCornerPos.x = centerPos.x - dim;
		leftCornerPos.y = centerPos.y - dim;
		leftCornerPos.z = 0;
		rightCornerPos.x = centerPos.x + dim;
		rightCornerPos.y = centerPos.y + dim;
		rightCornerPos.z = 0;
	}
	else if ( (p = args[0].find( '_' )) == 0 )
	{
		// Get player position (if rowId given)
		TDataSetIndex entityIndex;
		NLMISC::fromString(args[0].substr( 1 ), entityIndex);
		playerRowId = TDataSetRow::createFromRawIndex(entityIndex);
		if ( TheDataset.isAccessible(playerRowId) )
		{
			CVector centerPos;
			CCharacter* player = PlayerManager.getChar( playerRowId );
			if ( ! player )
			{
				log.displayNL( "%u is not a player", playerRowId.getIndex() );
				return true;
			}
			centerPos.x = float(player->getState().X)/1000.0f;
			centerPos.y = float(player->getState().Y)/1000.0f;
			leftCornerPos.x = centerPos.x - dim;
			leftCornerPos.y = centerPos.y - dim;
			leftCornerPos.z = 0;
			rightCornerPos.x = centerPos.x + dim;
			rightCornerPos.y = centerPos.y + dim;
			rightCornerPos.z = 0;
		}
		else
		{
			log.displayNL( "Player not found" );
			return true;
		}
	}
	else
	{
		// Get deposit coordinates (if name given)
		string depositName = args[0];
		const std::vector<CDeposit*>& depositList = CZoneManager::getInstance().getDeposits();
		for ( std::vector<CDeposit*>::const_iterator idl=depositList.begin(); idl!=depositList.end(); ++idl )
		{
			const CDeposit *deposit = (*idl);
			if ( deposit && (deposit->name() == depositName) )
			{
				deposit->getAABox( leftCornerPos, rightCornerPos );
				depositToScan = deposit;
				break;
			}
		}
	}

	// Init dictionary if not already done
	if ( ! WordDic )
	{
		WordDic = new CWordsDictionary(); // never deleted (for test only)
		WordDic->init( "egs_deposit_map_dictionary.cfg" );
	}

	// Setup output image
	CRMColorMapping colorMapping;
	CTGAImage image;
	image.setup( (uint16)((rightCornerPos.x - leftCornerPos.x) / resolution) + 1,
		(uint16)((rightCornerPos.y - leftCornerPos.y) / resolution) + 1, outputFilename, 0, 0 );
	image.setupForCol();
	set<CDeposit*> deposits;

	// Scan area and write image
	WarnIfOutsideOfRegion = false;
	map<const CStaticDepositRawMaterial*, uint> foundMaterials;
	CVector pos;
	pos.z = 0;
	uint c;
	OptFastFloorBegin(); // for CNoiseValue called in selectRMAtPos()
	for ( pos.y=leftCornerPos.y; pos.y<=rightCornerPos.y; pos.y+=resolution )
	{
		for ( c=0, pos.x=leftCornerPos.x; pos.x<=rightCornerPos.x; pos.x+=resolution, ++c )
		{
			TNothingFoundReason reason;
			CDeposit *deposit, *depositForK;
			const CStaticDepositRawMaterial *rawMaterial =
				CFgProspectionPhrase::selectRMAtPos( pos, CRyzomTime::unknownWeather, ECOSYSTEM::common_ecosystem, RM_GROUP::Unknown, RM_FAMILY::Unknown, ~0, 255, false, neighbourhood, false, &deposit, &depositForK, reason, true );
			if ( (!depositToScan) || (depositToScan == deposit) )
			{
				if ( deposit )
					deposits.insert( deposit );
				image.set16( c, colorMapping.getColorMapping( rawMaterial ? rawMaterial->MaterialSheet : CSheetId::Unknown ) );
				if ( rawMaterial )
					++foundMaterials[ rawMaterial ];
			}
			else
			{
				image.set16( c, colorMapping.getColorMapping( CSheetId::Unknown ) );
			}
		}
		image.writeLine();
	}
	OptFastFloorEnd();
	WarnIfOutsideOfRegion = true;

	log.displayNL( "Written %s. Found %u matching deposits", outputFilename.c_str(), deposits.size() );

	// Display color frequencies (raw materials merged by families)
	if ( displayColorFrequencies )
		colorMapping.displayFrequencies( log );

	// Display raw material frequencies
	uint totalOccurrences = 0;
	for ( map<const CStaticDepositRawMaterial*, uint>::const_iterator ifm=foundMaterials.begin(); ifm!=foundMaterials.end(); ++ifm )
	{
		const uint &nbOccurrences = (*ifm).second;
		totalOccurrences += nbOccurrences;
	}
	log.displayNL( "Summary for this deposit (%u total pts):", totalOccurrences );
	for ( map<const CStaticDepositRawMaterial*, uint>::const_iterator ifm=foundMaterials.begin(); ifm!=foundMaterials.end(); ++ifm )
	{
		const uint &nbOccurrences = (*ifm).second;
		log.displayRawNL( "%s -> %u pts (%.1f%%)", (*ifm).first->MaterialSheet.toString().c_str(),
			nbOccurrences, ((float)nbOccurrences) * 100.0f / (float)totalOccurrences );
	}

	// Display deposit specifications
	log.displayNL( "Deposits:" );
	for ( set<CDeposit*>::const_iterator idp=deposits.begin(); idp!=deposits.end(); ++idp )
	{
		(*idp)->displayContent( &log, false, WordDic );
	}
	return true;
}


/**
 * CBackgroundTask
 */
class CBackgroundTask : public NLMISC::IRunnable
{
public:
	/// Constructor
	CBackgroundTask() : _Stopping(false), _Complete(false) {}

	/// Destructor
	virtual ~CBackgroundTask()
	{
		if ( _Thread )
		{
			if ( ! _Complete )
			{
				pleaseStop();
				_Thread->wait();
			}
			delete _Thread;
			_Thread = NULL;
		}
	}

	/// Start (called in main thread)
	void	start()
	{
		if ( _Thread && (!_Complete) )
			return;
		_Stopping = false;
		_Complete = false;
		_Thread = NLMISC::IThread::create( this );
		_Thread->start();
	}

	/// Ask for stop and wait until terminated (called in main thread)
	void	terminateTask()
	{
		pleaseStop();
		_Thread->wait();
		delete _Thread;
		_Thread = NULL;
	}

	///
	bool	isRunning() const { return (_Thread != NULL) && (!_Complete); }

	///
	bool	isStopping() const { return _Stopping; }

	///
	bool	isComplete() const { return _Complete; }

	///
	virtual void run() = 0;

protected:

	void	setComplete() { _Complete = true; }

private:

	void	pleaseStop() { _Stopping = true; }

	volatile bool	_Stopping;
	volatile bool	_Complete;

	NLMISC::IThread	*_Thread;
};


/**
 * CDepositMapsBatchTask
 */
class CDepositMapsBatchTask : public CBackgroundTask
{
public:

	/// Constructor
	CDepositMapsBatchTask() : CBackgroundTask(), _CurrentMap(0) {}

	///
	void	setInputFilename( const std::string& ifn ) { _InputFilename = ifn; }

	///
	uint	currentMap() const { return _CurrentMap; }

	///
	virtual void run();

private:

	std::string		_InputFilename;
	uint			_CurrentMap;
};


/*
 *
 */
void CDepositMapsBatchTask::run()
{
	H_AUTO(CDepositMapsBatchTask_run);
	
	// Open files
	CSString pathName = "deposit_maps";
	if ( ! CFile::isExists( pathName ) )
		CFile::createDirectory( pathName );
	pathName += "/";
	FILE *outputF = fopen( (pathName + "deposit_maps.html").c_str(), "w" );
	if ( ! outputF )
	{
		nlwarning( "Can't create file %sdeposit_maps.html", pathName.c_str() );
		return;
	}
	FILE *inputF = fopen( _InputFilename.c_str(), "r" );
	if ( ! inputF )
	{
		fprintf( outputF, "File %s not found", _InputFilename.c_str() );
		fclose( outputF );
		return;
	}
	
	// Process
	_CurrentMap = 1;
	CLightMemDisplayer disp;
	disp.setParam( ~0 ); // no limit
	CLog log;
	log.addDisplayer( &disp );
	char inputLine [1024];
	CSString line;
	CVectorSString fields;
	while ( ! feof( inputF ) )
	{
		if ( isStopping() || (! fgets( inputLine, 1024, inputF )) )
			break;

		line = CSString( inputLine );
		if ( (! line.empty()) && (line[line.size()-1]=='\n') )
			line.resize( line.size()-1 );
		fields.clear();
		line.splitBySeparator( '\t', fields );
		enum TField { FRegion, FPlace, FPos };
		if ( fields.size() >= 3 )
		{
			fields[FPos] = fields[FPos].replace( " ", "" );
			CSString imageFilename = fields[FRegion] + "-" + fields[FPlace] + ".tga";
			imageFilename = imageFilename.replace( " ", "" );
			CSString cmdLine = "makeDepositMap " + fields[FPos] + " " + pathName + imageFilename + " 1 600";
			egs_feinfo( cmdLine.c_str() );
			log.displayNL( ("\n\n<B><A HREF=\"" + imageFilename + "\">" + imageFilename + "</A></B>\n").c_str() );
			ICommand::execute( cmdLine, log );
			++_CurrentMap;
		}
	}

	// Convert log to html & write it
	egs_feinfo( "Writing deposit_maps.html..." );
	CSString res;
	disp.write( res );
	res = res.replace( "\n", "<BR>\n" );
	fprintf( outputF, "%s", res.c_str() );

	// Close files
	fclose( inputF );
	fclose( outputF );
	delete WordDic;
	WordDic = NULL;
	egs_feinfo( "End of makeDepositMapsBatch." );
	
	setComplete();
}


/**
 * CCheckDepositContentTask
 */
class CCheckDepositContentTask : public CBackgroundTask
{
public:

	CCheckDepositContentTask() : _MustCreateMapOnProblem(false), _ResolutionForErroneousSmallDeposits(0.5f) {}

	///
	virtual void run();

	float	_ResolutionForErroneousSmallDeposits;
	bool	_MustCreateMapOnProblem;
};


CVariable<float> ForageMinimumRatioRmAreaInDeposit( "egs", "ForageMinimumRatioRmAreaInDeposit", "", 0.33f, 0, true );


/*
 *
 */
void CCheckDepositContentTask::run()
{
	H_AUTO(CCheckDepositContentTask_run);
	
	CVector pos;
	pos.z = 0;
	const std::vector<CDeposit*>& depositList = CZoneManager::getInstance().getDeposits();
	for ( std::vector<CDeposit*>::const_iterator idl=depositList.begin(); idl!=depositList.end(); ++idl )
	{
		if ( isStopping() )
			break;

		const CDeposit *scannedDeposit = (*idl);

		// Get AABox
		CVector cornerMin, cornerMax;
		scannedDeposit->getAABox( cornerMin, cornerMax );
		bool isSmallDeposit = (scannedDeposit->getAreaOfAABox() < 64.0f*64.0f);

		map<const CStaticDepositRawMaterial*, uint> foundMaterials;

		float resolution = 1.0f;
		do
		{
			// Scan deposit zone
			WarnIfOutsideOfRegion = false;
			OptFastFloorBegin(); // for CNoiseValue called in selectRMAtPos()
			for ( float y=cornerMin.y; y<=cornerMax.y; y+=resolution )
			{
				pos.y = y;
				for ( float x=cornerMin.x; x<=cornerMax.x; x+=resolution )
				{
					pos.x = x;
					TNothingFoundReason reason;
					CDeposit *deposit, *depositForK;
					const CStaticDepositRawMaterial *rawMaterial =
						CFgProspectionPhrase::selectRMAtPos( pos, CRyzomTime::unknownWeather, ECOSYSTEM::common_ecosystem, RM_GROUP::Unknown, RM_FAMILY::Unknown, ~0, 255, false, false, false, &deposit, &depositForK, reason, true );
					if ( rawMaterial && (deposit == scannedDeposit) )
					{
						++foundMaterials[ rawMaterial ];
					}
				}
			}
			OptFastFloorEnd();
			WarnIfOutsideOfRegion = true;

			// Check results
			float totalArea = 0;
			if ( foundMaterials.size() != scannedDeposit->getContentSize() )
			{
				if ( isSmallDeposit && (resolution > _ResolutionForErroneousSmallDeposits) )
				{
					resolution = _ResolutionForErroneousSmallDeposits; // retry with thinner resolution
					continue;
				}
				if ( _MustCreateMapOnProblem )
					InfoLog->displayRawNL( "=================================================" );
				egs_feinfo( "Error in %s: %u RMs found instead of %u [ptWidth=%gm]", scannedDeposit->name().c_str(), foundMaterials.size(), scannedDeposit->getContentSize(), resolution );
				if ( _MustCreateMapOnProblem )
					ICommand::execute( string("makeDepositMap ") + scannedDeposit->name() + string(" ") + scannedDeposit->name() + string(".tga 1 0 0 1 1"), *InfoLog );
			}
			else
			{
				bool mustRetry = false;
				for ( map<const CStaticDepositRawMaterial*, uint>::const_iterator ifm=foundMaterials.begin(); ifm!=foundMaterials.end(); ++ifm )
				{
					const uint &nbOccurrences = (*ifm).second;
					totalArea += (float)nbOccurrences;
				}
				float rmAverageArea = totalArea / (float)scannedDeposit->getContentSize();
				for ( map<const CStaticDepositRawMaterial*, uint>::const_iterator ifm=foundMaterials.begin(); ifm!=foundMaterials.end(); ++ifm )
				{
					const uint &nbOccurrences = (*ifm).second;
					if ( (float)nbOccurrences < rmAverageArea * ForageMinimumRatioRmAreaInDeposit.get() )
					{
						if ( isSmallDeposit && (resolution > _ResolutionForErroneousSmallDeposits) )
						{
							resolution = _ResolutionForErroneousSmallDeposits; // retry with thinner resolution
							mustRetry = true;
							break;
						}
						if ( _MustCreateMapOnProblem )
							InfoLog->displayRawNL( "=================================================" );
						egs_feinfo( "Warning in %s: %s is spread on only %.1f%% of the deposit (%u of %u pts [ptWidth=%gm]), should be about %.1f%%",
							scannedDeposit->name().c_str(), (*ifm).first->MaterialSheet.toString().c_str(),
							(float)nbOccurrences * 100.0f / totalArea, nbOccurrences, (uint)totalArea, resolution,
							rmAverageArea * 100.0f / totalArea);
						if ( _MustCreateMapOnProblem )
							ICommand::execute( string("makeDepositMap ") + scannedDeposit->name() + string(" ") + scannedDeposit->name() + NLMISC::toString("[%g].tga 1 0 0 1 1", resolution), *InfoLog );
					}
				}
				if ( mustRetry )
					continue;
			}
			break;
		}
		while ( true );
	}
	egs_feinfo( "End. %u deposits processed", depositList.size() );
	
	setComplete();
}


CDepositMapsBatchTask DepositMapsBatchTask;
CCheckDepositContentTask CheckDepositsTask;


/*
 *
 */
NLMISC_COMMAND( makeDepositMapsBatch, "", "[stop | <inputFilename>]" )
{
	// Read arguments
	string inputFilename;
	if ( ! args.empty() )
		inputFilename = args[0];

	// Manage task status
	if ( inputFilename == "stop" )
	{
		if ( DepositMapsBatchTask.isRunning() )
		{
			DepositMapsBatchTask.terminateTask();
			log.displayNL( "Task stopped" );
		}
		else
			log.displayNL( "Task is not running" );
	}
	else if ( DepositMapsBatchTask.isRunning() )
	{
		log.displayNL( "Task is running, current processed map %u", DepositMapsBatchTask.currentMap() );
	}
	else if ( ! inputFilename.empty() )
	{
		DepositMapsBatchTask.setInputFilename( inputFilename );
		DepositMapsBatchTask.start();
		log.displayNL( "Task started" );
	}
	else
		log.displayNL( "Task is not running" );
	return true;
}


/*
 *
 */
NLMISC_COMMAND( forageCheckDepositContent, "Check that all RMs set in deposits are available in sufficient places", "<createMapOnProblem=1>|stop resolutionForErroneousSmallDeposits=0.25" )
{
	bool mustCreateMapOnProblem = true;
	float resolutionForErroneousSmallDeposits = 0.25f;
	if ( ! args.empty() )
	{
		if ( args.size() > 1 )
			resolutionForErroneousSmallDeposits = (float)atof( args[0].c_str() );
		if ( args[0] == "stop" )
		{
			if ( CheckDepositsTask.isRunning() )
			{
				CheckDepositsTask.terminateTask();
				log.displayNL( "Task stopped" );
			}
			else
				log.displayNL( "Task is not running" );
			return true;
		}
		else if ( args[0] == "1" )
		{
			mustCreateMapOnProblem = true;
		}
	}

	if ( CheckDepositsTask.isRunning() )
	{
		log.displayNL( "Task is already running" );
	}
	else
	{
		CheckDepositsTask._MustCreateMapOnProblem = mustCreateMapOnProblem;
		CheckDepositsTask._ResolutionForErroneousSmallDeposits = resolutionForErroneousSmallDeposits;
		CheckDepositsTask.start();
		log.displayNL( "Task started" );
	}
	return true;
}


#endif

