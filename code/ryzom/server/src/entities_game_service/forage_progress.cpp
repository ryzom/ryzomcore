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
#include "forage_progress.h"
#include "player_manager/character.h"
#include "harvest_source.h"
#include "player_manager/player_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entities_game_service.h"
#include "player_manager/player.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"

using namespace NLMISC;
using namespace std;


CVariable<float> ForageFocusAutoRegenRatio( "egs", "ForageFocusAutoRegenRatio", "Ratio of auto-regen of focus after an extraction", 1.0, 0, true );

NL_INSTANCE_COUNTER_IMPL(CForageProgress);

const uint NB_QUANTITY_STATS = 26;
const uint NB_QUALITY_STATS = 26;
static uint QuantityStats [NB_QUANTITY_STATS];
static uint QualityStats [NB_QUALITY_STATS];
static uint QualityStatsQtty [NB_QUALITY_STATS];


/*
 * Reset
 */
void CForageProgress::reset( const NLMISC::CSheetId& material,  const TDataSetRow& sourceRowId, sint32 initialFocus )
{
	_Material = material;
	_SourceRowId = sourceRowId;
	_Amount = 0;
	_Quality = 1.0f;
	_InitialFocus = initialFocus;
	_ResultCanBeTaken = false;
	_HasCastedUsefulCare = false;
}


/*
 * Add the result of an action
 */
void CForageProgress::fillFromExtraction( float quantity, float quality, CCharacter *player )
{
	_Amount += quantity;
	_Quality = quality;

	//if ( (_Amount*10.0f >= 65536.0) || (_Quality>=251.0f) )
	//	nlwarning( "Forage ext. progress out of bounds: %g %g", _Amount, _Quality );

	player->setExtractionProgressCounters( (uint16)(uint)(_Amount*10.0f), (uint16)(uint)(_Quality*10.0f) );

	//nldebug( "Progress #%u: (A %.2f Q %.2f)", (uint)_NbExtActions, _Amount, _Quality );
}


/*
 * Report the accumulated XP during an extraction session. Assumes extractor and source not null.
 */
void CForageProgress::reportXP( CCharacter *extractor, const CHarvestSource *source )
{
	H_AUTO(CForageProgress_reportXP);
	
#if !FINAL_VERSION
	nlassert(extractor);
	nlassert(source);
#endif

	// Don't earn any XP for anyone if the quantity is 0
	if ( amount() == 0 )
		return;

	// Count XP depending on class
	float xpBonusFactor = 0.9f - ForageProspectionXPBonusRatio.get() +
		((((float)(source->getStatQuality()))-5.0f)/(75.0f*5.0f)); // basic gives -10%, supreme gives +10% bonus (5 -> 0.0, 80 -> 0.2)
	float nbParticipants = 1.0f; // at least the extractor

	// Count XP for prospection (if there was one)
	const TDataSetRow& prospectorDataSetRow = source->getProspectorDataSetRow(); // can be invalid (auto-spawned source)
	CCharacter *prospector = PlayerManager.getChar( prospectorDataSetRow ); // checks isAccessible()
	if ( prospector )
	{
		xpBonusFactor += ForageProspectionXPBonusRatio.get();
		if ( prospector != extractor )
			++nbParticipants;
	}

	// Find out how many care casters were useful
	vector<CCharacter*> usefulCareCasters;
	const CHarvestSource::CForagers& foragers = source->foragers();
	CHarvestSource::CForagers::const_iterator itf = foragers.begin(); // the first one is the only one able to extract
	for ( ++itf; itf!=foragers.end(); ++itf )
	{
		const TDataSetRow& ccRowId = (*itf);
		CCharacter *careCaster = PlayerManager.getChar( ccRowId );
		if ( careCaster && careCaster->forageProgress() )
		{
			// Check if this care caster has not changed his target
			if ( careCaster->forageProgress()->sourceRowId() == source->rowId() )
			{
				// Check if this care caster has teamed up with the extractor!
				if ( (careCaster->getTeamId() != CTEAM::InvalidTeamId) && (careCaster->getTeamId() == extractor->getTeamId()) )
				{
					// Check if he casted some useful care
					if ( careCaster->forageProgress()->hasCastedUsefulCare() )
					{
						usefulCareCasters.push_back( careCaster );
						if ( careCaster != prospector ) // don't count the prospector twice
							++nbParticipants;
					}
				}
				else
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage( careCaster->getEntityRowId(), "FORAGE_CARE_XP_NEEDS_TEAM" );
				}
			}
		}
	}

	// Count XP for number of participants and nasty events
	xpBonusFactor += (nbParticipants-1.0f) * ForageExtractionNbParticipantsXPBonusRatio.get(); // everyone gets 10% more per additional forager
	float nbNastyEvents = (float)(source->nbEventTriggered());
	xpBonusFactor -= nbNastyEvents * ForageExtractionNastyEventXPMalusRatio.get();

	// Count XP per participant
	float xpPerParticipant = ForageExtractionXPFactor.get() * xpBonusFactor / nbParticipants;

	// Give XP to extractor
	reportXPTo( extractor, xpPerParticipant, extractor->getRightHandItem() );
	bool isProspectorCredited = false;
	if ( extractor == prospector )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = 0;
		PHRASE_UTILITIES::sendDynamicSystemMessage( prospector->getEntityRowId(), "FORAGE_XP_PROSPECTION", params );
		isProspectorCredited = true;
	}
	if ( source->wasProspected() )
	{
		// Give energy back!
		SCharacteristicsAndScores& focus = extractor->getScores()._PhysicalScores[SCORES::focus];
		if ( focus.Current < initialFocus() )
			focus.Current = focus.Current() + (sint32)(ForageFocusAutoRegenRatio.get() * (float)(initialFocus()-focus.Current));
	}

	// Give XP to care casters
	for ( vector<CCharacter*>::iterator icc=usefulCareCasters.begin(); icc!=usefulCareCasters.end(); ++icc )
	{
		CCharacter *careCaster = (*icc);
		reportXPTo( careCaster, xpPerParticipant, careCaster->getRightHandItem() );
		if ( prospector == careCaster )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = 0;
			PHRASE_UTILITIES::sendDynamicSystemMessage( prospector->getEntityRowId(), "FORAGE_XP_PROSPECTION", params );
			isProspectorCredited = true;
		}
		if ( source->wasProspected() && careCaster->forageProgress() )
		{
			// Give energy back!
			SCharacteristicsAndScores& focus = careCaster->getScores()._PhysicalScores[SCORES::focus];
			if ( focus.Current < careCaster->forageProgress()->initialFocus() )
				focus.Current = focus.Current() + (sint32)(ForageFocusAutoRegenRatio.get() * (float)(careCaster->forageProgress()->initialFocus()-focus.Current));
		}
	}

	// Give XP to "standalone" prospector (if any)
	if ( prospector && (! isProspectorCredited) )
	{
		reportXPTo( prospector, xpPerParticipant, NULL );
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = 1;
		PHRASE_UTILITIES::sendDynamicSystemMessage( prospector->getEntityRowId(), "FORAGE_XP_PROSPECTION", params );
	}
}


/*
 * Report the accumulated XP to a participant
 */
void CForageProgress::reportXPTo( CCharacter *player, float factor, CGameItemPtr toolUsed )
{
	H_AUTO(CForageProgress_reportXPTo);
	
	// Calculate delta level between obtained quality (level) and extractor's level, add bonus for quantity
	sint32 deltaLevel = player->getSkillBaseValue( _UsedSkill ) - ((sint32)quality());
	deltaLevel -= (sint32)(((float)(amount() - 1)) * ForageQuantityXPDeltaLevelBonusRate.get());

	// Action report for xp gain
	TReportAction report;
	report.ActorRowId = player->getEntityRowId();
	report.ActionNature = ACTNATURE::SEARCH_MP;
	report.DeltaLvl = deltaLevel;
	report.factor = factor; // factor=10 and deltaLevel=0 => clientXP=1000
	report.Skill = _UsedSkill;

	// send a message if tol is worned
	if ( (toolUsed != NULL) && (toolUsed->getItemWornState() == ITEM_WORN_STATE::Worned) )
	{
		const CStaticItem * form = toolUsed->getStaticForm();
		if( form )
		{
			const string msgName = ITEM_WORN_STATE::getMessageForState(ITEM_WORN_STATE::Worned);
			if ( !msgName.empty())
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
				params[0].SheetId = toolUsed->getSheetId();
				PHRASE_UTILITIES::sendDynamicSystemMessage( report.ActorRowId, msgName, params);
			}
		}
	}
	else
	{
		// compute wear of used tool (must be in right hand)
		if ( toolUsed != NULL )
			player->wearRightHandItem();
		
		// report Xp Gain unless used tool is worned
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( report, true, false ); // don't scale delta level at low level
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(report);
	}

	// wear armor, shield and jewels
	player->wearArmor();
	player->wearShield();
	player->wearJewels();


	// Store stats about progression
	CSheetId hl, hr;
	uint32 qualityl, qualityr;
	
	CGameItemPtr item = player->getLeftHandItem();
	if( item == 0 )
	{
		qualityl = 0;
	}
	else
	{
		hl = item->getSheetId();
		qualityl = item->quality();
	}
	item = player->getRightHandItem();
	if( item == 0 )
	{
		qualityr = 0;
	}
	else
	{
		hr = item->getSheetId();
		qualityr = item->quality();
	}
	//Bsi.append( StatPath, NLMISC::toString("[EAE] %s %s %d %s %d %1.2f", player->getId().toString().c_str(), hl.toString().c_str(), qualityl, hr.toString().c_str(), qualityr, report.factor) );
	//EgsStat.displayNL("[EAE] %s %s %d %s %d %1.2f", player->getId().toString().c_str(), hl.toString().c_str(), qualityl, hr.toString().c_str(), qualityr, report.factor);
//	EGSPD::executeActionMagic(player->getId(), hl.toString(), qualityl, hr.toString(), qualityr, report.factor);
}


/*
 * Give forage result and XP. Return true if there is some raw material to take.
 */
bool CForageProgress::giveForageResult( CCharacter *player, const CHarvestSource *source )
{
	H_AUTO(CForageProgress_giveForageResult);
	
	// It would be nice to display "DeltaHP" on target but when the source disappears, it can't be displayed

	// Report XP, check result
	reportXP( player, source );
	if ( (! source->foragers().empty()) && (source->foragers().front() == player->getEntityRowId()) ) // only the first one can take
	{
		_ResultCanBeTaken = (amount() != 0) && (quality() != 0);
		if ( _ResultCanBeTaken )
		{
			// Stats
			uint statIndex = (amount() > NB_QUANTITY_STATS-1) ? NB_QUANTITY_STATS-1 : amount();
			++QuantityStats[statIndex];
			statIndex = (quality() > (NB_QUALITY_STATS-1)*10) ? NB_QUALITY_STATS-1 : quality() / 10;
			++QualityStats[statIndex];
			QualityStatsQtty[statIndex] += amount();
		}
	}
	else
		_ResultCanBeTaken = false;
	return _ResultCanBeTaken;
}


NLMISC_COMMAND( displayForageStats, "Display QT & QL stats", "" )
{
	for ( uint i=0; i!=NB_QUANTITY_STATS; ++i )
	{
		log.displayNL( "QT=%u %u", i, QuantityStats[i] );
	}
	for ( uint i=0; i!=NB_QUALITY_STATS; ++i )
	{
		log.displayNL( "QL<%u %u ext", (i+1)*10, QualityStats[i] );
	}
	for ( uint i=0; i!=NB_QUALITY_STATS; ++i )
	{
		log.displayNL( "QL<%u %u qtt", (i+1)*10, QualityStatsQtty[i] );
	}
	return true;
}

NLMISC_COMMAND( resetForageStats, "Reset QT & QL stats", "" )
{
	for ( uint i=0; i!=NB_QUANTITY_STATS; ++i )
	{
		QuantityStats[i] = 0;
	}
	for ( uint i=0; i!=NB_QUALITY_STATS; ++i )
	{
		QualityStats[i] = 0;
	}
	for ( uint i=0; i!=NB_QUALITY_STATS; ++i )
	{
		QualityStatsQtty[i] = 0;
	}
	return true;
}
