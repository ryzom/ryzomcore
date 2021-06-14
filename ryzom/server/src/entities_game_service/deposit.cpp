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
#include "deposit.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "zone_manager.h"
#include "entities_game_service.h"
#include "egs_globals.h"
#include "nel/misc/noise_value.h"
#include "nel/misc/variable.h"
#include "nel/misc/words_dictionary.h"
#include "game_share/time_weather_season/time_date_season_manager.h"
#include "game_share/people.h"
#include "egs_sheets/egs_sheets.h"
#include "phrase_manager/fg_prospection_phrase.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "game_share/send_chat.h"
#include "game_share/multi_target.h"
#include "phrase_manager/s_effect.h"
#include "projectile_stats.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;
using namespace RM_FAMILY;


const float SmallDepositAreaThreshold = 64.0f*64.0f;
const float SmallDepositAreaReference = 16.0f*32.0f;

// The ecotype zones (only valid at init, cleared after deposit built)
CEcotypeZones				CDeposit::_EcotypeZones;

NL_INSTANCE_COUNTER_IMPL(CAutoSpawnProperties);
NL_INSTANCE_COUNTER_IMPL(CQuantityConstraints);

// Verbose deposits debug variable
//bool VerboseDeposits = false;

// Total number of CRecentForageSite objects (debug info)
uint32 TotalNbRecentForageSites = 0;

/// Export deposit contents report
bool ExportDepositContents = false;

/// Verbose the items parsed when filtering deposits
bool VerboseDepositFiltering = false;


void cbChangeDepositUpdateFrequency( NLMISC::IVariable& v )
{
	/*const TGameCycle expectedTTL = 6000; // 10 min
	if ( DepositUpdateFrequency.get() != 0 )
		ForageSiteNbUpdatesToLive.set( (uint16)(expectedTTL / DepositUpdateFrequency.get()) );*/
	nlinfo( "TODO: change ForageSiteNbUpdatesToLive according to DepositUpdateFrequency" );
}


/*
 * Utility functions
 */

bool malformed( const char *field, const string& name )
{
	nlwarning( "FG: Malformed primitive %s, missing field '%s'", name.c_str(), field );
	return false;
}

/*
 * Get the enums of the families (using RM_FAMILY::toFamily(), assumes the string are ended by _value)
 * where value is the identifier number (see .typ).
 * Removes duplicates (with a warning).
 */
void convertRMFamiliesNames( const string& name, const vector<string>& src, vector<TRMFamily>& dest )
{
	for ( uint i=0; i!=src.size(); ++i )
	{
		if ( ! src[i].empty() )
		{
			string::size_type p = src[i].find_last_of( '_' );
			if ( p != string::npos )
			{
				TRMFamily family = toFamily( src[i].substr( p + 1 ) );
				if ( find( dest.begin(), dest.end(), family ) == dest.end() )
					dest.push_back( family );
				else
					nlwarning( "FG: Deposit %s: %s found twice", name.c_str(), src[i].c_str() ); 
			}
			else
			{
				nlwarning( "FG: Deposit %s: %s: not found", name.c_str(), src[i].c_str() );
			}
		}
	}
}

/*
 * Get the indices of the item parts (assumes the strings are ended by _index).
 * Ex: blade_0.
 * Removes duplicates (with a warning).
 */
void convertItemPartsNames( const string& name, const vector<string>& src, vector<uint>& dest )
{
	for ( uint i=0; i!=src.size(); ++i )
	{
		if ( ! src[i].empty() )
		{
			string::size_type p = src[i].find_last_of( '_' );
			if ( p != string::npos )
			{
				uint itemPart;
				NLMISC::fromString(src[i].substr( p + 1 ), itemPart);
				if ( find( dest.begin(), dest.end(), itemPart ) == dest.end() )
					dest.push_back( itemPart );
				else
					nlwarning( "FG: Deposit %s: %s found twice", name.c_str(), src[i].c_str() ); 
			}
			else
			{
				nlwarning( "FG: Deposit %s: %s: not found", name.c_str(), src[i].c_str() );
			}
		}
	}
}

/*
 * Get the civ enum.
 */
void convertCraftCivNames( const vector<string>& src, vector<ITEM_ORIGIN::EItemOrigin>& dest )
{
	dest.resize( src.size() );
	for ( uint i=0; i!=src.size(); ++i )
	{
		dest[i] = ITEM_ORIGIN::stringToEnum( src[i] );
	}
}


/*
 *
 */
inline void makeFullSItemCode( string& code )
{
	if ( code.find( ".sitem" ) == string::npos )
	{
		code += ".sitem";
	}
}


/*
 * Return always a forage site
 */
CRecentForageSite *CDeposit::findOrCreateForageSite( const NLMISC::CVector& pos )
{
	// Search in existing ones
	for ( CRecentForageSites::iterator ihs=_RecentForageSites.begin(); ihs!=_RecentForageSites.end(); ++ihs )
	{
		if ( (*ihs).contains( pos ) )
		{
			return &(*ihs);
		}
	}

	// Not found, create one
	++TotalNbRecentForageSites;
	CRecentForageSite newForageSite( this, pos );
	_RecentForageSites.push_back( newForageSite );
	return &_RecentForageSites.back(); // returning the pointer is valid because an element in std::list is never invalidated
}


/*
 *
 */
bool CRecentForageSite::contains( const NLMISC::CVector& pos ) const
{
	float r = ForageSiteRadius.get();
	return ((pos-_Pos).sqrnorm() < r*r);
}


/*
 * Display debug or stat info
 */
void CRecentForageSite::display( NLMISC::CLog& log ) const
{
	log.displayNL( "Forage site at %s: %u sources, stock %u, %u lowfreq updates remaining",
		_Pos.asString().c_str(), _NbActiveSources, _LowScopeStock, _TimeToLive );
}


//-----------------------------------------------------------------------------
// Parse primitive file for one ecotype
//-----------------------------------------------------------------------------
bool CEcotypeZone::build( const NLLIGO::CPrimZone* zone )
{
	*( (NLLIGO::CPrimZone*)this ) = *zone;

	// Read primitive name
	string name;
	if ( ! zone->getPropertyByName( "name", name ) )								return malformed( "name", name );

	// Read ecotype
	string ecotypeS;
	if ( ! zone->getPropertyByName( "ecotype", ecotypeS ) )							return malformed( "ecotype", name );
	_Ecotype = ECOSYSTEM::stringToEcosystem( ecotypeS ); // needs a case-unsensitive comparison, because the
	if ( _Ecotype == ECOSYSTEM::unknown )				 // World Editor lists names of files in CVS (in deposit_system/ecotypes)
		nlwarning( "%s: Ecotype %s unknown", name.c_str(), ecotypeS.c_str() );
	return true;
}


/*
 * Destructor
 */
CDeposit::~CDeposit()
{
	if ( _AutoSpawnSourcePt )
		delete _AutoSpawnSourcePt;
	_AutoSpawnSourcePt = NULL;
	if ( _QuantityConstraintsPt )
		delete _QuantityConstraintsPt;
	_QuantityConstraintsPt = NULL;

	// Avoid any bug, remove the deposit from zone manager
	CZoneManager::getInstance().unregisterDepositToAutoSpawnUpdate(this);
}


/*
 * Get the ecotype zone under the position.
 * This information is valid only at init time. After the deposits are built, this information
 * can be found only in deposits.
 * If not found, a NULL pointer is returned.
 */
CEcotypeZone *CDeposit::getEcotypeZone( const NLMISC::CVector& pos )
{
	// The ecotypes must not be overlapped: only the first one found is returned
	for ( CEcotypeZones::iterator it=_EcotypeZones.begin(); it!=_EcotypeZones.end(); ++it )
	{
		CEcotypeZone *ecotypeZone = (*it);
		if ( ecotypeZone->contains( pos ) )
		{
			return ecotypeZone;
		}
	}
	return NULL;
}


/*
 * Clear ecotype information, after having built the deposits
 */
void CDeposit::clearEcotypes()
{
	for ( CEcotypeZones::iterator iez=_EcotypeZones.begin(); iez!=_EcotypeZones.end(); ++iez )
	{
		delete (*iez);
	}
	_EcotypeZones.clear();
}


struct TCompareStaticItemPtrBySheetId : public std::binary_function<CStaticItem*,CStaticItem*,bool>
{
	bool operator() ( const CStaticItem* p1, const CStaticItem* p2 )
	{
		return (p1->SheetId < p2->SheetId);
	}
};


//-----------------------------------------------------------------------------
// Parse primitive file for one deposit
//-----------------------------------------------------------------------------
bool CDeposit::build( const NLLIGO::CPrimZone* zone )
{
	if ( IsRingShard )
		return false;
	
	//_Id  =id;
	*( (NLLIGO::CPrimZone*)this ) = *zone;

	// Read primitive name
	string name;
	if ( ! zone->getPropertyByName( "name", name ) )								return malformed( "name", name );
	_Name = name;

	// Read exact raw material codes to add
	vector<string> *exactRMCodesS = NULL;
	if ( ! (zone->getPropertyByName( "exact_mp_item", exactRMCodesS ) && exactRMCodesS) ) return malformed( "exact_mp_item", name );

	// Read raw material family filter
	vector<string> *rmFamilyFilterS = NULL;
	if ( ! (zone->getPropertyByName( "mps", rmFamilyFilterS ) && rmFamilyFilterS) )	return malformed( "mps", name );

	// Read item part / craft civ filter
	vector<string> *itemPartsFilterS = NULL, *craftCivS = NULL;
	if ( ! (zone->getPropertyByName( "item_parts", itemPartsFilterS ) && itemPartsFilterS) ) return malformed( "item_parts", name );
	if ( ! (zone->getPropertyByName( "craft_civ", craftCivS ) && craftCivS) )		return malformed( "craft_civ", name );

	// Read stat+quality filters
	string minEnergyS, maxEnergyS, minQualityS, maxQualityS;
	if ( ! zone->getPropertyByName( "deposit_statquality_min", minEnergyS ) )		return malformed( "deposit_statquality_min", name );
	if ( ! zone->getPropertyByName( "deposit_statquality_max", maxEnergyS ) )		return malformed( "deposit_statquality_max", name );
	if ( ! zone->getPropertyByName( "deposit_min_quality_250", minQualityS ) )		return malformed( "deposit_min_quality_250", name );
	if ( ! zone->getPropertyByName( "deposit_max_quality_250", maxQualityS ) )		return malformed( "deposit_max_quality_250", name );
	NLMISC::fromString(minQualityS, _MinQuality);
	NLMISC::fromString(maxQualityS, _MaxQuality);

	// Read quantity constraints
	string qttyLimitS, qttyRespawnTimeS;
	if ( ! zone->getPropertyByName( "deposit_quantity_limit", qttyLimitS ) )		return malformed( "deposit_quantity_limit", name );
	if ( ! zone->getPropertyByName( "deposit_quantity_respawn_time_ryzomdays", qttyRespawnTimeS ) ) return malformed( "deposit_quantity_respawn_time_ryzomdays", name );
	sint qttyLimit;
	NLMISC::fromString(qttyLimitS, qttyLimit);
	if ( qttyLimit > -1 )
	{
		sint qttyRespawnTime;
		NLMISC::fromString(qttyRespawnTimeS, qttyRespawnTime);
		if ( (qttyLimit == 0) || (qttyLimit > 0xFFFF) || (qttyRespawnTime < 1) || (qttyRespawnTime > 0xFFFF) )
			nlwarning( "Invalid limit or respawn time too high in %s", name.c_str() );
		else
		{
			_QuantityConstraintsPt = new CQuantityConstraints();
			_QuantityConstraintsPt->CurrentQuantity = (float)qttyLimit;
			_QuantityConstraintsPt->InitialQuantity = (uint16)qttyLimit;
			_QuantityConstraintsPt->RespawnTimeRyzomDays = qttyRespawnTime;
		}
	}

	// Get ecotype
	CEcotypeZone *ecotypeZone = getEcotypeZone( getBarycentre() );
	_Ecotype = ecotypeZone ? ecotypeZone->ecotype() : ECOSYSTEM::common_ecosystem;
	if ( ! ecotypeZone )
		nlwarning( "FG: Deposit %s has no ecotype", name.c_str() );

	_FilterPhase = 0;
	
	// Read season(s)
	string s1s, s2s, s3s, s4s;
	if ( ! zone->getPropertyByName( "while_season_spring", s1s ) )		return malformed( "while_season_spring", name );
	if ( ! zone->getPropertyByName( "while_season_summer", s2s ) )		return malformed( "while_season_summer", name );
	if ( ! zone->getPropertyByName( "while_season_automn", s3s ) )		return malformed( "while_season_automn", name );
	if ( ! zone->getPropertyByName( "while_season_winter", s4s ) )		return malformed( "while_season_winter", name );
	bool s1 = (s1s=="true"), s2 = (s2s=="true"), s3 = (s3s=="true"), s4 = (s4s=="true");
	if ( ! (s1 && s2 && s3 && s4) )
	{
		if ( s1 )	{	_SeasonFilter.push_back( EGSPD::CSeason::Spring ); _FilterPhase |= 0x1; }
		if ( s2 )	{	_SeasonFilter.push_back( EGSPD::CSeason::Summer ); _FilterPhase |= 0x2; }
		if ( s3 )	{	_SeasonFilter.push_back( EGSPD::CSeason::Autumn ); _FilterPhase |= 0x4; }
		if ( s4 )	{	_SeasonFilter.push_back( EGSPD::CSeason::Winter ); _FilterPhase |= 0x8; }
	}

	// Read weather
	string w1s, w2s, w3s, w4s;
	if ( ! zone->getPropertyByName( "while_weather_0_best", w1s ) )		return malformed( "while_weather_0_best", name );
	if ( ! zone->getPropertyByName( "while_weather_1_good", w2s ) )		return malformed( "while_weather_1_good", name );
	if ( ! zone->getPropertyByName( "while_weather_2_bad", w3s ) )		return malformed( "while_weather_2_bad", name );
	if ( ! zone->getPropertyByName( "while_weather_3_worst", w4s ) )	return malformed( "while_weather_3_worst", name );
	bool w1 = (w1s=="true"), w2 = (w2s=="true"), w3 = (w3s=="true"), w4 = (w4s=="true");
	if ( ! (w1 && w2 && w3 && w4) )
	{
		if ( w1 )	{	_WeatherFilter.push_back( CRyzomTime::best ); _FilterPhase |= 0x10; }
		if ( w2 )	{	_WeatherFilter.push_back( CRyzomTime::good ); _FilterPhase |= 0x20; }
		if ( w3 )	{	_WeatherFilter.push_back( CRyzomTime::bad );  _FilterPhase |= 0x40; }
		if ( w4 )	{	_WeatherFilter.push_back( CRyzomTime::worst ); _FilterPhase |= 0x80; }
	}

	// Read time of day
	string t1s, t2s;
	if ( ! zone->getPropertyByName( "while_its_day", t1s ) )			return malformed ( "while_its_day", name );
	if ( ! zone->getPropertyByName( "while_its_night", t2s ) )			return malformed ( "while_its_night", name );
	bool t1 = (t1s=="true"), t2 = (t2s=="true");
	if ( ! (t1 && t2) )
	{
		if ( t1 )
		{
			_TimeOfDayFilter.push_back( CRyzomTime::dawn );
			_TimeOfDayFilter.push_back( CRyzomTime::day );
			_TimeOfDayFilter.push_back( CRyzomTime::evening );
			_FilterPhase |= 0x100;
		}
		if ( t2 )
		{
			_TimeOfDayFilter.push_back( CRyzomTime::nightfall );
			_TimeOfDayFilter.push_back( CRyzomTime::night );
			_FilterPhase |= 0x200;
		}
	}

	// Read auto-spawn properties
	string assS;
	if ( ! zone->getPropertyByName( "auto_spawn_sources", assS ) )			return malformed( "auto_spawn_sources", name );
	if ( assS=="true" )
	{
		string aspS, asltS, asetS, amin;      // not really 'average' anymore
		if ( ! zone->getPropertyByName( "auto_spawn_average_period_s", aspS ) )	return malformed( "auto_spawn_average_period", name );
		if ( ! zone->getPropertyByName( "auto_spawn_lifetime_s", asltS ) )		return malformed( "auto_spawn_lifetime", name );
		if ( ! zone->getPropertyByName( "auto_spawn_extraction_time_s", asetS ) )	return malformed( "auto_spawn_sources", name );
		if ( ! zone->getPropertyByName( "auto_spawn_min_source", amin ) )	return malformed( "auto_spawn_min_source", name );
		_AutoSpawnSourcePt = new CAutoSpawnProperties;
		NLMISC::fromString(aspS, _AutoSpawnSourcePt->SpawnPeriodGc);
		_AutoSpawnSourcePt->SpawnPeriodGc *= 10;
		NLMISC::fromString(asltS, _AutoSpawnSourcePt->LifeTimeGc);
		_AutoSpawnSourcePt->LifeTimeGc *= 10;
		NLMISC::fromString(asetS, _AutoSpawnSourcePt->ExtractionTimeGc);
		_AutoSpawnSourcePt->ExtractionTimeGc *= 10;
		NLMISC::fromString(amin, _AutoSpawnSourcePt->MinimumSpawnedSources);
		// security!
		_AutoSpawnSourcePt->MinimumSpawnedSources = min(uint32(100), _AutoSpawnSourcePt->MinimumSpawnedSources);
	}
	else
	{
		_AutoSpawnSourcePt = NULL;
	}

	// Read source FX index
	string srcFXIndexS;
	if ( ! zone->getPropertyByName( "source_fx", srcFXIndexS ) )			return malformed( "source_fx", name );
	NLMISC::fromString(srcFXIndexS, _SourceFXIndex);

	// Read other initial properties
	string cpS, eS, ikaS, adpR;
	if ( ! zone->getPropertyByName( "can_prospect", cpS ) )					return malformed( "can_prospect", name );
	if ( ! zone->getPropertyByName( "enabled", eS ) )						return malformed( "enabled", name );
	if ( ! zone->getPropertyByName( "initial_kami_anger", ikaS ) )			return malformed( "initial_kami_anger", name );
	if ( ! zone->getPropertyByName( "can_have_depletion_risk", adpR ) )		return malformed( "can_have_depletion_risk", name );
	_CanProspect = (cpS=="true");
	_Enabled = (eS=="true");
	_AllowDepletionRisk = (adpR=="true");
	_KamiAnger = (float)atof( ikaS.c_str() );
	if ( (_KamiAnger != -1.0f) && (_KamiAnger < 0) )
		nlwarning( "Invalid initial_kami_anger %.1f in %s", _KamiAnger, name.c_str() );

	// Apply filters
	uint32 minEnergy, maxEnergy;
	NLMISC::fromString(minEnergyS, minEnergy);
	NLMISC::fromString(maxEnergyS, maxEnergy);
	if ( exactRMCodesS->empty() && rmFamilyFilterS->empty() && itemPartsFilterS->empty() )
	{
		nlwarning( "FG: Deposit %s: No RM, exactRms or item parts specified!", name.c_str() );
		return false;
	}
	else
		selectRMsByFilters( *exactRMCodesS, *rmFamilyFilterS, *itemPartsFilterS, *craftCivS, minEnergy, maxEnergy );

	nldebug( "FG: Built deposit %s %s %s %s %s", name.c_str(), _AutoSpawnSourcePt?"AUTO":"-", _CanProspect?"PRO":"-", _AllowDepletionRisk?"ADPR":"-", _Enabled?"ON":"OFF" );
	return true;
}


/*
 * Select the raw materials, using the specified filters and _Ecotype
 */
void CDeposit::selectRMsByFilters( std::vector<std::string>& exactRMCodesS, const std::vector<std::string>& rmFamilyFilterS, const std::vector<std::string>& itemPartsFilterS, const std::vector<std::string>& craftCivS, uint minEnergy, uint maxEnergy )
{
	vector<TRMFamily> rmFamilyFilter;
	convertRMFamiliesNames( _Name, rmFamilyFilterS, rmFamilyFilter );
	vector<uint> itemPartsFilter;
	convertItemPartsNames( _Name, itemPartsFilterS, itemPartsFilter );
	vector<ITEM_ORIGIN::EItemOrigin> craftCivFilter;
	convertCraftCivNames( craftCivS, craftCivFilter );
	for_each( exactRMCodesS.begin(), exactRMCodesS.end(), makeFullSItemCode );

	if ( VerboseDepositFiltering )
		nldebug( "%s", _Name.c_str() );

	// Sort the items by sheetId so that the deposits are not dependant on the hash map ordering
	const CAllStaticItems& allItems = CSheets::getItemMapForm();
	vector< const CStaticItem* > sortedItems( allItems.size() );
	uint i = 0;
	for ( CAllStaticItems::const_iterator it=allItems.begin(); it!=allItems.end(); ++it, ++i )
	{
		sortedItems[i] = &((*it).second);
	}
	std::sort( sortedItems.begin(), sortedItems.end(), TCompareStaticItemPtrBySheetId() );

	for ( vector< const CStaticItem* >::const_iterator it=sortedItems.begin(); it!=sortedItems.end(); ++it )
	{
		// Eliminate non raw materials
		const CStaticItem& staticItem = *(*it);
		if ( ! staticItem.Mp )
			continue;
		const string& sheetName = staticItem.SheetId.toString();
		if ( (sheetName.size() < (CREATURE_OR_DEPOSIT_MP_CHAR+1)) || (sheetName[0] != 'm' ) )
			continue;

		// Keep raw material directly if matching one of specified exact raw material codes (including creature's raw materials)
		if ( find( exactRMCodesS.begin(), exactRMCodesS.end(), sheetName ) == exactRMCodesS.end() )
		{
			// Eliminate non 'forage' raw materials
			if ( VerboseDepositFiltering )
				nldebug( "FG: Submitting %s", sheetName.c_str() );
			if ( (sheetName[CREATURE_OR_DEPOSIT_MP_CHAR] != 'd') )
				continue;

			// Eliminate raw materials of incompatible ecosystem
			if ( (_Ecotype != ECOSYSTEM::common_ecosystem) && (staticItem.Mp->Ecosystem != ECOSYSTEM::common_ecosystem) &&
				 (staticItem.Mp->Ecosystem != _Ecotype) )
			{
				if ( VerboseDepositFiltering ) nldebug( "-Ecotype %s", ECOSYSTEM::toString( staticItem.Mp->Ecosystem ).c_str() );
				continue;
			}

			// Match energy filter (TEMP: 25 is the minimum maxEnergy threshold) (include boundaries)
			if ( (staticItem.Mp->StatEnergy < minEnergy) || (staticItem.Mp->StatEnergy > maxEnergy /*max(maxEnergy,(uint)25)*/) )
			{
				if ( VerboseDepositFiltering ) nldebug( "-StatEnergy %hu", staticItem.Mp->StatEnergy );
				continue;
			}

			// Match rmFamilyFilter or itemPartsFilter
			if ( find( rmFamilyFilter.begin(), rmFamilyFilter.end(), staticItem.Mp->Family ) == rmFamilyFilter.end() )
			{
				// If not found in rmFamilyFilter, try if one of itemPartsFilter is matched along with civ 
				bool found = false;
				for ( vector<uint>::iterator ipf=itemPartsFilter.begin(); ipf!=itemPartsFilter.end(); ++ipf )
				{
					uint itemPartIndex = (*ipf);

					// Find if the current RM has the item part matching one of the filter
					const CMP::TMpFaberParameters *mpFaberParam = staticItem.Mp->getMpFaberParameters( itemPartIndex );
					if ( mpFaberParam && (mpFaberParam->Durability != 0) )
					{
						if ( (craftCivFilter.empty()) || // no civ constraint
							 (mpFaberParam->CraftCivSpec == ITEM_ORIGIN::COMMON) || // RM matches all civs
							 (find( craftCivFilter.begin(), craftCivFilter.end(), mpFaberParam->CraftCivSpec ) != craftCivFilter.end()) ) // RM matches civ constraint
						{
							if ( VerboseDepositFiltering ) nldebug( "+StatEnergy %hu +ItemPart %c +CivSpec %s", staticItem.Mp->StatEnergy, 'A' + (char)itemPartIndex, ITEM_ORIGIN::enumToString( mpFaberParam->CraftCivSpec ).c_str() );
							found = true;
							break; // RM is selected, no need to test other item parts
						}
						else
							if ( VerboseDepositFiltering ) nldebug( "-CivSpec %s", ITEM_ORIGIN::enumToString( mpFaberParam->CraftCivSpec ).c_str() );
					}
					else
					{
						if ( VerboseDepositFiltering ) nldebug( "-ItemPart %c", 'A' + (char)itemPartIndex );
					}
				}
				if ( ! found )
					continue;
			}
			/*else
			{
				if ( VerboseDepositFiltering ) nldebug( "+rmFamilyFilter %u", staticItem.Mp->Family );
			}*/
		}
		/*else
		{
			if ( VerboseDepositFiltering ) nldebug( "+exact_mp_item %s", sheetName.c_str() );
		}*/

		// Select if matching (do not check duplicate, keep them if item part matches explicit family)
		CStaticDepositRawMaterial rm;
		rm.MaterialSheet = staticItem.SheetId;
		_RawMaterials.push_back( rm );
		if ( VerboseDepositFiltering )
			nldebug( "SELECTED %s", sheetName.c_str() );

		// Export deposit contents report if requested
		if ( ExportDepositContents )
		{
			static FILE *depositReportFile;
			static bool depositReportCreated = false;
			if ( ! depositReportCreated )
			{
				depositReportCreated = true;
				depositReportFile = fopen( "deposit_contents.csv", "wt" ); // fclose() auto?
				if ( depositReportFile )
				{
					fprintf( depositReportFile, "Deposit;RM;When in year;When in day;Weather;\n" );
				}
			}
			if ( depositReportFile )
				fprintf( depositReportFile, "%s;%s;%s;%s;%s;\n", _Name.c_str(), rm.MaterialSheet.toString().c_str(), getSeasonStr().c_str(), getTimeOfDayStr().c_str(), getWeatherStr().c_str() );
		}
	}
	if ( _RawMaterials.empty() )
		nlwarning( "FG: Selected 0 items in deposit %s", _Name.c_str() );
	else
		nldebug( "FG: Selected %u RM (on %u items) in deposit %s", _RawMaterials.size(), allItems.size(), _Name.c_str() );
}


/**
 * Helper for CDeposit::hasFamily()
 */
struct CIsOfFamilyPred : public std::binary_function< CStaticDepositRawMaterial, RM_FAMILY::TRMFamily, bool >
{
	/// Predicate
	bool operator() ( const CStaticDepositRawMaterial& rm, RM_FAMILY::TRMFamily family ) const
	{
		const CAllStaticItems& allItems = CSheets::getItemMapForm();
		CAllStaticItems::const_iterator it = allItems.find( rm.MaterialSheet );
		if ( it != allItems.end() )
		{
			const CStaticItem& staticItem = (*it).second;
			return staticItem.Mp && (staticItem.Mp->Family == family);
		}
		else
			return false;
	}
};


/**
 * Helper for CDeposit::hasFamily()
 */
struct CIsOfGroupPred : public std::binary_function< CStaticDepositRawMaterial, RM_GROUP::TRMGroup, bool >
{
	/// Predicate
	bool operator() ( const CStaticDepositRawMaterial& rm, RM_GROUP::TRMGroup group ) const
	{
		const CAllStaticItems& allItems = CSheets::getItemMapForm();
		CAllStaticItems::const_iterator it = allItems.find( rm.MaterialSheet );
		if ( it != allItems.end() )
		{
			const CStaticItem& staticItem = (*it).second;
			return staticItem.Mp && (staticItem.Mp->getGroup() == group);
		}
		else
			return false;
	}
};


/**
 * Helper for CDeposit::hasRMForItemPart()
 */
struct CCanCraftItemPartPred : public std::binary_function< CStaticDepositRawMaterial, uint, bool >
{
	/// Predicate
	bool operator() ( const CStaticDepositRawMaterial& rm, uint itemPartIndex ) const
	{
		const CAllStaticItems& allItems = CSheets::getItemMapForm();
		CAllStaticItems::const_iterator it = allItems.find( rm.MaterialSheet );
		if ( it != allItems.end() )
		{
			const CStaticItem& staticItem = (*it).second;
			if ( staticItem.Mp )
			{
				const CMP::TMpFaberParameters *mpFaberParam = staticItem.Mp->getMpFaberParameters( itemPartIndex );
				return (mpFaberParam && (mpFaberParam->Durability != 0));
			}
			else
				return false;
		}
		else
			return false;
	}
};


/**
 * Helper for CDeposit::hasFamily()
 */
struct CMatchStatEnergyPred : public std::binary_function< CStaticDepositRawMaterial, uint8, bool >
{
	/// Predicate
	bool operator() ( const CStaticDepositRawMaterial& rm, uint8 maxStatEnergy ) const
	{
		const CAllStaticItems& allItems = CSheets::getItemMapForm();
		CAllStaticItems::const_iterator it = allItems.find( rm.MaterialSheet );
		if ( it != allItems.end() )
		{
			const CStaticItem& staticItem = (*it).second;
			return staticItem.Mp && (staticItem.Mp->StatEnergy <= maxStatEnergy); // include boundary
		}
		else
		{
			nlwarning( "%s not found", rm.MaterialSheet.toString().c_str() );
			return true;
		}
	}
};


/**
 * Helper for CDeposit::hasFamily()
 */
struct CMatchExactStatEnergyPred : public std::binary_function< CStaticDepositRawMaterial, uint8, bool >
{
	/// Predicate
	bool operator() ( const CStaticDepositRawMaterial& rm, uint8 maxStatEnergy ) const
	{
		const CAllStaticItems& allItems = CSheets::getItemMapForm();
		CAllStaticItems::const_iterator it = allItems.find( rm.MaterialSheet );
		if ( it != allItems.end() )
		{
			const CStaticItem& staticItem = (*it).second;
			return staticItem.Mp && (staticItem.Mp->StatEnergy == maxStatEnergy);
		}
		else
			return false;
	}
};


/*
 * Return true if the deposit contains at least one RM of the specified family
 */
bool CDeposit::hasFamily( RM_FAMILY::TRMFamily family ) const
{
	return find_if( _RawMaterials.begin(), _RawMaterials.end(), bind2nd( CIsOfFamilyPred(), family ) ) != _RawMaterials.end();
}

/*
 * Return true if the deposit contains at least one RM of the specified group
 */
bool CDeposit::hasGroup( RM_GROUP::TRMGroup group ) const
{
	return find_if( _RawMaterials.begin(), _RawMaterials.end(), bind2nd( CIsOfGroupPred(), group ) ) != _RawMaterials.end();
}

/*
 * Return true if the deposit contains at least one RM than can craft the specified item part
 */
bool CDeposit::hasRMForItemPart( uint itemPartIndex ) const
{
	return find_if( _RawMaterials.begin(), _RawMaterials.end(), bind2nd( CCanCraftItemPartPred(), itemPartIndex ) ) != _RawMaterials.end();
}

/*
 * Return true if the deposit contains at least one RM with energy lower_eq than the specified value
 */
bool CDeposit::hasLowerStatEnergy( uint8 maxStatEnergy ) const
{
	return find_if( _RawMaterials.begin(), _RawMaterials.end(), bind2nd( CMatchStatEnergyPred(), maxStatEnergy ) ) != _RawMaterials.end(); 
}

/*
 * Return true if the deposit contains at least one RM with energy equalling the specifing value
 */
bool CDeposit::hasExactStatEnergy( uint8 statEnergy ) const
{
	return find_if( _RawMaterials.begin(), _RawMaterials.end(), bind2nd( CMatchExactStatEnergyPred(), statEnergy ) ) != _RawMaterials.end(); 
}


/*
 *	Called by lowFreqUpdate and autoSpawnUpdate
 */
void CDeposit::autoSpawnSource(const CVector &cornerMin, const CVector &cornerMax)
{
	/*
		Yoyo: This method may fail because it selects a random position in a box and then check if it's in the zone.
		We can do better by making a list of triangles of the concave polygon, then selecting randomly and
		carefully in this list of triangle.
	*/

	// Randomize a position in the deposit
	CVector pos;
	pos.z = 0;
	const uint NB_ATTEMPTS = 5;
	uint iAttempt;
	for ( iAttempt=0; iAttempt!=NB_ATTEMPTS; ++iAttempt )
	{
		pos.x = cornerMin.x + RandomGenerator.frand( cornerMax.x - cornerMin.x );
		pos.y = cornerMin.y + RandomGenerator.frand( cornerMax.y - cornerMin.y );
		if ( contains( pos ) )
			break;
	}
	
	// If a valid position could not be found, abort spawning of this source
	if ( iAttempt == NB_ATTEMPTS )
		return;
	
	// Spawn a source
	CFgProspectionPhrase::autoSpawnSource( pos, this );
}


/*
 * Update deposit (especially recent forage sites)
 */
void CDeposit::lowFreqUpdate()
{
	if ( !HarvestSystemEnabled )
		return;
	// Update recent forage sites
	for ( CRecentForageSites::iterator it=_RecentForageSites.begin(); it!=_RecentForageSites.end(); )
	{
		CRecentForageSite& forageSite = (*it);
		if ( forageSite.lowFreqUpdate() )
		{
			++it;
		}
		else
		{
			--TotalNbRecentForageSites;
			it = _RecentForageSites.erase( it );
		}
	}

	// Decrease kami anger level
	decKamiAnger( ForageKamiAngerDecreasePerHour.get() / 36000.0f * ((float)DepositUpdateFrequency.get()) );

	// Auto-spawn a source from time to time (if the deposit has this flag)
	if ( _AutoSpawnSourcePt && _Enabled )
	{
		// For Speed Test
		//TTime	testYoyoLT0= CTime::getLocalTime();
		//uint	countBefore= _CurrentNbAutoSpawnedSources;

		uint spawnPeriodLFUpdates = (AutoSpawnForageSourcePeriodOverride.get() != 0) ? AutoSpawnForageSourcePeriodOverride.get() / DepositUpdateFrequency.get() : _AutoSpawnSourcePt->SpawnPeriodGc / DepositUpdateFrequency.get();
		
		// get deposit bbox
		CVector cornerMin, cornerMax;
		getAABox( cornerMin, cornerMax );
		
		// Previously, the rythm of spawning was randomized. Now it's constant.
		//TGameCycle spawnAvgPeriod = (AutoSpawnForageSourceAveragePeriodOverride.get() != 0) ? AutoSpawnForageSourceAveragePeriodOverride.get() : _AutoSpawnSourcePt->SpawnAveragePeriodGc;
		//sint32 r = RandomGenerator.rand( (uint16)(spawnAvgPeriod / DepositUpdateFrequency.get()) );
		//if ( r == 0 )
		bool	spawnSourceBecauseOfFrequency= (CTickEventHandler::getGameCycle() / DepositUpdateFrequency.get()) % spawnPeriodLFUpdates == 0;

		// Avoid infinite loop if the spawn of source is too buggy: 
		// count first the number of sources to force spawn then run this count, and forget the ones that fail.
		uint	numSourceToForceSpawn= 0;
		if(_CurrentNbAutoSpawnedSources < _AutoSpawnSourcePt->MinimumSpawnedSources)
			numSourceToForceSpawn= _AutoSpawnSourcePt->MinimumSpawnedSources - _CurrentNbAutoSpawnedSources;
		uint	numSourceSpawned= 0;
		while ( spawnSourceBecauseOfFrequency || numSourceSpawned<numSourceToForceSpawn)
		{
			// this method may fail, but don't cares and continues
			autoSpawnSource(cornerMin, cornerMax);
			numSourceSpawned++;

			// If continue this loop, won't be because of frequency update, but because of minimum source requirement
			spawnSourceBecauseOfFrequency= false;
		}

		// For Speed Test
		//TTime	testYoyoLT1= CTime::getLocalTime();
		//nlinfo("*** [%03d]AutoSpawnLOWFREQ Time(%d->%d): %d ms", CTickEventHandler::getGameCycle()%1000, countBefore, _CurrentNbAutoSpawnedSources, testYoyoLT1 - testYoyoLT0);
	}

	// Beware: can return before
}


/*
 *	Update deposit that need to auto spawn because number of harvest sources is too low
 */
void CDeposit::autoSpawnUpdate()
{
	if ( !HarvestSystemEnabled )
		return;

	// Auto-spawn a source from time to time (if the deposit has this flag)
	if ( _AutoSpawnSourcePt && _Enabled && _CurrentNbAutoSpawnedSources < _AutoSpawnSourcePt->MinimumSpawnedSources)
	{
		// For Speed Test
		//TTime	testYoyoLT0= CTime::getLocalTime();
		//uint	countBefore= _CurrentNbAutoSpawnedSources;
		
		// get deposit bbox
		CVector cornerMin, cornerMax;
		getAABox( cornerMin, cornerMax );
		
		// Avoid infinite loop if the spawn of source is too buggy: 
		// count first the number of sources to force spawn then run this count, and forget the ones that fail.
		uint	numSourceToForceSpawn= _AutoSpawnSourcePt->MinimumSpawnedSources - _CurrentNbAutoSpawnedSources;

		// for all sources to spawn
		for( uint i= 0; i<numSourceToForceSpawn; i++)
		{
			// this method may fail, but don't cares and continues
			autoSpawnSource(cornerMin, cornerMax);
		}
		// For Speed Test
		//TTime	testYoyoLT1= CTime::getLocalTime();
		//nlinfo("*** [%03d]AutoSpawnFORCE Time(%d->%d): %d ms", CTickEventHandler::getGameCycle()%1000, countBefore, _CurrentNbAutoSpawnedSources, testYoyoLT1 - testYoyoLT0);
	}
	
	// Beware: can return before
}


/*
 *	Auto Spawn Deposit mgt
 */
void CDeposit::decreaseAutoSpawnedSources()
{
	BOMB_IF(_CurrentNbAutoSpawnedSources==0, "nb auto spawned sources should be > 0!", return);
	_CurrentNbAutoSpawnedSources--;

	// If the deposit has not enough ressources, must refill at next tick update
	// NB: don't do it at next lowFreqUpdate(), to avoid big jump in CPU each 30 sec
	// as soon an autospawned harvest source is unspawned, the deposit will refill at next tick.
	if(_AutoSpawnSourcePt && _CurrentNbAutoSpawnedSources < _AutoSpawnSourcePt->MinimumSpawnedSources)
	{
		CZoneManager::getInstance().registerDepositToAutoSpawnUpdate(this);
	}
}

void CDeposit::increaseAutoSpawnedSources()
{
	_CurrentNbAutoSpawnedSources++;
}


//-----------------------------------------------------------------------------
// character take MP, kami eat this fool character ?
//-----------------------------------------------------------------------------
#if 0
//void CDeposit::characterTakeRM( const CEntityId& charId, uint32 depositIndexContent, uint16 quantity )
//{
//	// check if depositIndexContent is in valid range
//	if( depositIndexContent < _DepositContent.size() )
//	{
//		// process kami guardian reaction and fame impact
//		CDepositRawMaterialSeasonParameters * seasonParameters;
//		switch( CTimeDateSeasonManager::getRyzomTimeReference().getRyzomSeason() )
//		{
//			case EGSPD::CSeason::Spring:
//				seasonParameters = &_DepositContent[ depositIndexContent ]->SpringParams;
//				break;
//			case EGSPD::CSeason::Summer:
//				seasonParameters = &_DepositContent[ depositIndexContent ]->SummerParams;
//				break;
//			case EGSPD::CSeason::Autumn:
//				seasonParameters = &_DepositContent[ depositIndexContent ]->AutumnParams;
//				break;
//			case EGSPD::CSeason::Winter:
//				seasonParameters = &_DepositContent[ depositIndexContent ]->WinterParams;
//				break;
//			default:
//				nlstop; //=> debug CTimeDateSeasonManager....
//		}
//
//		if( seasonParameters->AngryLevel >= _DepositContent[ depositIndexContent ]->CurrentQuantity )
//		{
//			sint32	fameImpact;
//			uint32	kamiImpact;
//
//			CMessage msgString("STATIC_STRING");
//			msgString.serial( const_cast< CEntityId& > ( charId ) );
//			// serial exclude set of empty exclude set
//			set< CEntityId > empty;
//			msgString.serialCont( empty );
//			
//			if( seasonParameters->BlackKamiLevel >= _DepositContent[ depositIndexContent ]->CurrentQuantity )
//			{
//				// kami impact for AIS when Kami expulse a black kami
//				kamiImpact = 300;
//				
//				// set fame impact
//				fameImpact = -15;
//
//				string str("WOS_KAMI_BLACK_KAMI");
//				msgString.serial(str);
//			}
//			else if( seasonParameters->FuryLevel >= _DepositContent[ depositIndexContent ]->CurrentQuantity )
//			{
//				// kami impact for AIS when Kami is fury against harvester
//				kamiImpact = 200;
//
//				// set fame impact
//				fameImpact = -10;
//
//				string str("WOS_KAMI_FURY");
//				msgString.serial( str );
//			}
//			else if( seasonParameters->AngryLevel >= _DepositContent[ depositIndexContent ]->CurrentQuantity )
//			{
//				// kami impact for AIS when Kami is angry against harvester
//				kamiImpact = 100;
//
//				// set fame impact
//				fameImpact = -5;
//
//				string str("WOS_KAMI_ANGRY");
//				msgString.serial( str );
//			}
//			sendMessageViaMirror( "IOS", msgString );
//			
//			// Send message to AIS for kami impact
//			CMessage msgout("KAMI_IMPACT");
//			// TODO : if this code reactivated, add the instance number and send to the corect AIS
//			msgout.serial( const_cast<CEntityId&> (charId) );
//			msgout.serial( _Alias );
//			msgout.serial( kamiImpact );
//			sendMessageViaMirror( "AIS", msgout );
//
//
//			// send message to EGS for fame impact
//			// TODO boris : update fame with new fame system
///*			CCharacter * c = PlayerManager.getChar( charId );
//			uint8 fameType = FAMES::kami;
//			if( c )
//			{
//				c->fameChange( fameType, fameImpact );
//			}
//*/		}
//
//		// update deposit quantity
//		if( _DepositContent[ depositIndexContent ]->CurrentQuantity < quantity )
//		{
//			_DepositContent[ depositIndexContent ]->CurrentQuantity = 0;
//		}
//		else
//		{
//			_DepositContent[ depositIndexContent ]->CurrentQuantity -= quantity;
//		}
//	}
//	else
//	{
//		nlwarning("FG: <CDeposit::characterTakeRM> Received invalid depositIndexContent %d (deposit content size id %d)", depositIndexContent, _DepositContent.size() );
//	}
//}
#endif


string CDeposit::getSeasonStr() const
{
	string s;
	if ( _SeasonFilter.empty() )
		return "All year";
	for ( uint i=0; i!=_SeasonFilter.size(); ++i )
	{
		if ( ! s.empty() )
			s += " ";
		s += EGSPD::CSeason::toString( _SeasonFilter[i] );
	}
	return s;
}

string CDeposit::getTimeOfDayStr() const
{
	string s;
	if ( _TimeOfDayFilter.empty() )
		return "Night & day";
	for ( uint i=0; i!=_TimeOfDayFilter.size(); ++i )
	{
		if ( ! s.empty() )
			s += " ";
		s += toString( "%u", (uint)_TimeOfDayFilter[i] );
	}
	return s;
}

string CDeposit::getWeatherStr() const
{
	string s;
	if ( _WeatherFilter.empty() )
		return "All weathers";
	for ( uint i=0; i!=_WeatherFilter.size(); ++i )
	{
		if ( ! s.empty() )
			s += " ";
		s += toString( "%u", (uint)_WeatherFilter[i] );
	}
	return s;
}


//-----------------------------------------------------------------------------
// display deposit content
//-----------------------------------------------------------------------------
void CDeposit::displayContent( NLMISC::CLog * log, bool extendedInfo, NLMISC::CWordsDictionary *itemDictionary )
{
	const CAllStaticItems& allItems = CSheets::getItemMapForm();
	uint32 materialNumber = 0;
	log->displayRawNL( "---- DEPOSIT %s ----", _Name.c_str() );
	log->displayRawNL( "\t Centre: %s", getBarycentre().toString().c_str() );
	if ( getContents().empty() )
		log->displayRawNL( "\tNo RM was selected in this deposit" );
	log->displayRawNL( "\t%u RMs:", getContents().size() );
	for( std::vector< CStaticDepositRawMaterial >::const_iterator it = getContents().begin(); it != getContents().end(); ++it )
	{
		if ( extendedInfo )
			log->displayRawNL( "\tRM #%d", materialNumber );
		++materialNumber;
		CSString sheetCode = (*it).MaterialSheet.toString();
		string fullNameInfo;
		if ( itemDictionary )
		{
			CVectorSString result;
			sheetCode = sheetCode.splitTo( '.' );
			itemDictionary->lookup( sheetCode, result );
			if ( ! result.empty() )
				fullNameInfo = string(" ") + result[0];
		}
		if ( fullNameInfo.empty() )
			log->displayRawNL( "\t\tRM Id: %s", sheetCode.c_str() );
		else
			log->displayRawNL( "\t\tRM Id: %s", fullNameInfo.c_str() );
		//log->displayNL( "\t\tMax Amount: %d", (*it).MaxAmount );
		if ( extendedInfo )
		{
			CAllStaticItems::const_iterator itItem = allItems.find( (*it).MaterialSheet );
			if ( itItem != allItems.end() && (*itItem).second.Mp )
			{
				const CStaticItem& staticItem = (*itItem).second;
				log->displayRawNL( "\t\tFamily: %u", staticItem.Mp->Family );
				log->displayRawNL( "\t\tGroup: %u", staticItem.Mp->getGroup() );
				log->displayRawNL( "\t\tEcosystem: %s", ECOSYSTEM::toString( staticItem.Mp->Ecosystem ).c_str() );
				//log->displayRawNL( "\t\tRarity: %hu", staticItem.Mp->Rarity );
				log->displayRawNL( "\t\tStatEnergy: %hu", staticItem.Mp->StatEnergy );
			}
		}
	}
	log->displayRawNL( "\tEcotype: %s", ECOSYSTEM::toString( _Ecotype ).c_str() );
	string ws;
	if ( _WeatherFilter.empty() )
		ws = "all";
	else
	{
		for( vector<CRyzomTime::EWeather>::const_iterator iwf=_WeatherFilter.begin(); iwf!=_WeatherFilter.end(); ++iwf )
			ws += toString( "%u:", *iwf );
	}
	log->displayRawNL( "\tWeathers: %s", ws.c_str() );
	string ts;
	if ( _TimeOfDayFilter.empty() )
		ts = "all";
	else
	{
		for( vector<CRyzomTime::ETimeOfDay>::const_iterator itf=_TimeOfDayFilter.begin(); itf!=_TimeOfDayFilter.end(); ++itf )
			ts += toString( "%u:", *itf );
	}
	log->displayRawNL( "\tTimes of day: %s", ts.c_str() );
	string ss;
	if ( _SeasonFilter.empty() )
		ss = "all";
	else
	{
		for( vector<CRyzomTime::ESeason>::const_iterator isf=_SeasonFilter.begin(); isf!=_SeasonFilter.end(); ++isf )
			ws += toString( "%u:", *isf );
	}
	log->displayRawNL( "\tSeasons: %s", ss.c_str() );
	log->displayRaw( "\tKami anger: %.1f", _KamiAnger );
	if ( _KamiAnger == -1.0f )
		log->displayRawNL( " disabled" );
	else
		log->displayRawNL( " on %.1f, %.1f", ForageKamiAngerThreshold1.get(), ForageKamiAngerThreshold2.get() );
	if ( _AutoSpawnSourcePt )
		log->displayRawNL( "\tAutoSpawnSource: %s", _AutoSpawnSourcePt?"ON":"OFF" );
	if ( ! _CanProspect )
		log->displayRawNL( "\tCanProspect: %d", _CanProspect );
	if ( ! _Enabled )
		log->displayRawNL( "\tEnabled: %d", _Enabled );
	if ( ! _AllowDepletionRisk )
		log->displayRawNL( "\tAllowDepletionRisk: %d", _AllowDepletionRisk );
}


float GenRand = 1.0f;
float GenFreq = 0.05f;

NLMISC_COMMAND( setDepositRand, "Set CNoiseValue params for deposits", "<rand> <freq>" )
{
	if ( args.size() < 2 )
		return false;
	GenRand = (float)atof( args[0].c_str() );
	GenFreq = (float)atof( args[1].c_str() );
	return true;
}


/*
 *
 */
class CIndexNoiseValue : public CNoiseValue
{
public:

	///
	CIndexNoiseValue( uint arraySize, uint phase, float freq ) : CNoiseValue(0.0f, 1.0f, freq), _MaxIndex((float)(arraySize)), _Phase(1210.191f*phase, 523.8883f*phase, 403.57614f*phase) {}

	/// Evaluate the value corresponding to the pos
	uint	eval( const CVector& pos ) const
	{
		uint indexOfMax = 0;
		float maxNoise = 0;

		// 1 dephased noise per possible value, and take the max result (otherwise with '(CNoiseValue::eval( pos ) * _MaxIndex)', the result would be biased)
		for( uint i=0; i!=_MaxIndex; ++i )
		{
			float f = CNoiseValue::eval(
				pos + _Phase + CVector(5245.346f*i, 785.67985f*i, 7842.783367f*i) );
			if ( f > maxNoise )
			{
				maxNoise = f;
				indexOfMax = i;
			}
		}
		return indexOfMax;
	}

private:

	float		_MaxIndex;

	CVector		_Phase;
};


/*
 * Compute freq according to deposit size (small deposits under 64x64 have a proportionally higher frequency)
 */
inline float getFreqFromDepositArea( float depositBBoxArea )
{
	if ( depositBBoxArea < SmallDepositAreaThreshold )
	{
		const float gradient = (0.05f-0.3f)*2.0f/(SmallDepositAreaThreshold-SmallDepositAreaReference);
		return gradient*depositBBoxArea + (GenFreq - gradient*(SmallDepositAreaThreshold));
	}
	else
	{
		return GenFreq;
	}
}


/*
 * Get a random RM from the neighbourhood of the specified position.
 * OptFastFloorBegin()/OptFastFloorEnd() must enclose one or more calls to this method.
 */
const CStaticDepositRawMaterial *CDeposit::getRandomRMAtPos( const NLMISC::CVector& pos, bool testIfSiteDepleted, bool& isDepleted )
{
	const float cellWidth = 1.0f;
	const uint16 nbNeighbours = 5; // center + 4 neighbour cells

	//nldebug( "Selecting RM in deposit" );

	if ( getContents().empty() )
		return NULL;

	// Test is local zone is depleted
	if ( testIfSiteDepleted )
	{
		for ( CRecentForageSites::iterator ihs=_RecentForageSites.begin(); ihs!=_RecentForageSites.end(); ++ihs )
		{
			if ( (*ihs).isDepleted() && (*ihs).contains( pos ) )
			{
				//nldebug( "Harvest site depleted" );
				isDepleted = true;
				return NULL;
			}
		}
	}

	// Compute freq according to deposit size
	float freq = getFreqFromDepositArea( getAreaOfAABox() );

	// Get raw materials in neighbourhood
	uint neighbourhoud [nbNeighbours]; 
 	CVector cellPos = pos;
	CIndexNoiseValue noiseIndex( getContentSize(), _FilterPhase, freq ); // the phrase prevents to have the same map for two deposits with identical size but different seasons, etc.
	neighbourhoud[0] = noiseIndex.eval( cellPos );
	cellPos.x -= cellWidth;
	cellPos.y -= cellWidth;
	neighbourhoud[1] = noiseIndex.eval( cellPos );
	cellPos.x = pos.x + cellWidth;
	neighbourhoud[2] = noiseIndex.eval( cellPos );
	cellPos.y = pos.y + cellWidth;
	neighbourhoud[3] = noiseIndex.eval( cellPos );
	cellPos.x = pos.x - cellWidth;
	neighbourhoud[4] = noiseIndex.eval( cellPos );
	//nldebug( "Indices: %u %u %u %u", neighbourhoud[0], neighbourhoud[1], neighbourhoud[2], neighbourhoud[3] );
	
	// Select a random RM among neighbourhood
	uint rmIndex = neighbourhoud[RandomGenerator.rand( nbNeighbours-1 )];
	return &(getContents()[rmIndex]);

/*
	if( seasonParameters->AngryLevel >= _DepositContent[ infos.DepositIndexContent ]->CurrentQuantity )
	{
		CMessage msgout("STATIC_STRING");
		msgout.serial( const_cast< CEntityId& > ( charId ) );
		// serial exclude set of empty exclude set
		set< CEntityId > empty;
		msgout.serialCont( empty );
		string str("EGS_KAMI_ALERTE");
		msgout.serial( str );
		sendMessageViaMirror( "IOS", msgout );
	}		
*/
}


/*
 * Get the RM at the specified position.
 * OptFastFloorBegin()/OptFastFloorEnd() must enclose one or more calls to this method.
 * \param testIfSiteDepleted Set it to false for map generation.
 */
const CStaticDepositRawMaterial *CDeposit::getRMAtPos( const NLMISC::CVector& pos, bool testIfSiteDepleted, bool& isDepleted )
{
	//const float cellWidth = 5.0f;

	//nldebug( "Selecting RM in deposit" );

	if ( getContents().empty() )
		return NULL;

	// Test is local zone is depleted
	if ( testIfSiteDepleted )
	{
		for ( CRecentForageSites::iterator ihs=_RecentForageSites.begin(); ihs!=_RecentForageSites.end(); ++ihs )
		{
			if ( (*ihs).isDepleted() && (*ihs).contains( pos ) )
			{
				//nldebug( "Harvest site depleted" );
				isDepleted = true;
				return NULL;
			}
		}
	}

	// Compute freq according to deposit size
	float freq = getFreqFromDepositArea( getAreaOfAABox() );

	// Get raw material at position
	CIndexNoiseValue noiseIndex( getContentSize(), _FilterPhase, freq );
	uint rmIndex = noiseIndex.eval( pos );
	//nldebug( "Index: %u", rmIndex );
	if ( rmIndex >= getContentSize() )
	{
		nlwarning( "FG: CIndexNoiseValue returned a value out of range" );
		rmIndex = getContentSize()-1;
	}
	return &(getContents()[rmIndex]);
}


/*
 * Return the current quantity. If 0 and the respawn time is elapsed, unlock. 
 */
float CQuantityConstraints::getCurrentQuantity()
{
	if ( CurrentQuantity == 0 )
	{
		// Time to unlock?
		const CRyzomTime& ryzomTime = CTimeDateSeasonManager::getRyzomTimeReference();
		if ( ryzomTime.getRyzomDay() < NextRespawnDay )
		{
			// Still locked
			return 0;
		}
		else
		{
			// Unlock and retry
			CurrentQuantity = InitialQuantity;
		}
	}
	return CurrentQuantity;
}


/*
 * Consume
 *
 * Last possible consumption => lock deposit for RespawnTimeRyzomDays.
 * Does not unlock the deposit (see getCurrentQuantity() instead)
 * Argument by value.
 */
float CQuantityConstraints::consumeQuantity( float consumed )
{
	if ( CurrentQuantity > 0 )
	{
		float newQuantity = CurrentQuantity - consumed;
		if ( newQuantity <= 0 )
		{
			// Lock if the deposit is now empty (and wasn't before)
			const CRyzomTime& ryzomTime = CTimeDateSeasonManager::getRyzomTimeReference();
			NextRespawnDay = ryzomTime.getRyzomDay() + (uint32)RespawnTimeRyzomDays;

			// Give only the remaining quantity
			consumed = CurrentQuantity;
			newQuantity = 0;
		}

		// Consume
		CurrentQuantity = newQuantity;
		return consumed;
	}
	else
		return 0;
}


/*
 * Display forage sites info
 */
void CDeposit::displayRecentForageSites( NLMISC::CLog& log ) const
{
	if ( ! _RecentForageSites.empty() )
	{
		log.displayNL( "Deposit %s:", _Name.c_str() );
		for ( CRecentForageSites::const_iterator it=_RecentForageSites.begin(); it!=_RecentForageSites.end(); ++it )
		{
			const CRecentForageSite& forageSite = (*it);
			forageSite.display( log );
		}
	}
}


/*
 * Empty all forage sites
 */
void CDeposit::depleteAllRecentForageSites()
{
	for ( CRecentForageSites::iterator it=_RecentForageSites.begin(); it!=_RecentForageSites.end(); ++it )
	{
		CRecentForageSite& forageSite = (*it);
		forageSite.depleteAll();
	}
}


/*
 * Increment the kami anger level (delta < 768). React if a threshold is reached. Return the kami anger level.
 */
float CDeposit::incKamiAnger( float delta, const std::vector<TDataSetRow>& foragers )
{
	if ( _KamiAnger == -1.0f )
		 return -1.0f;

	if ( ForageKamiAngerOverride.get() != 0 )
		_KamiAnger = ForageKamiAngerOverride.get();
	else
		_KamiAnger += delta;
	if ( _KamiAnger > ForageKamiAngerThreshold1.get() )
	{
		if ( _KamiAnger < ForageKamiAngerThreshold2.get() )
		{
			// Only a warning (and small punishment ?)
			for ( vector<TDataSetRow>::const_iterator itf=foragers.begin(); itf!=foragers.end(); ++itf )
			{
				// Send a warning message
				const TDataSetRow& rowId = (*itf);
				TDataSetRow noRow;
				npcTellToPlayer( noRow, rowId, "FORAGE_KAMI_ANGER_WARNING", false );
			}
		}
		else
		{
			// Full punishment
			if( HarvestAreaEffectOn )
			{
				for ( vector<TDataSetRow>::const_iterator itf=foragers.begin(); itf!=foragers.end(); ++itf )
				{
					const TDataSetRow& rowId = (*itf);
					CEntityBase *entity = CEntityBaseManager::getEntityBasePtr( rowId );	
					if ( entity )
					{
						// Send a warning message
						TDataSetRow noRow;
						npcTellToPlayer( noRow, rowId, "FORAGE_KAMI_ANGER_PUNISH", false );

						// Set target list for FX (must be done before behaviour)
						CMirrorPropValueList<uint32> targets( TheDataset, rowId, DSPropertyTARGET_LIST );
						targets.clear();
						targets.push_front( 0 ); // null distance (self)
						uint32 urowId = *((uint32*)&rowId);
						targets.push_front( urowId );

						// Set behaviour for FX (must be done after target list)
						MBEHAV::CBehaviour behav( MBEHAV::CAST_OFF_SUCCESS );
						behav.Spell.SpellMode = MAGICFX::Bomb;
						behav.Spell.SpellId = MAGICFX::Piercing;// + ForageKamiAngerPunishFX.get();
						behav.Spell.SpellIntensity = 5;
						behav.Spell.Resist = 0;
						behav.Spell2.SelfSpell = 1; // 'self offensive cast' does not plays the cast anim, only impact & FX
						PHRASE_UTILITIES::sendUpdateBehaviour( rowId, behav );

						// tmp nico : stats about projectiles
						projStatsIncrement();

						// Damage
						CHarvestSource::hitEntity( RYZOMID::creature, entity, ForageKamiAngerPunishDamage.get(), ForageKamiAngerPunishDamage.get(), false );

						//Bsi.append( StatPath, NLMISC::toString("[FKWP] %s '%s' %.1f", entity->getId().toString().c_str(), name().c_str(), _KamiAnger) );
						//EgsStat.displayNL("[FKWP] %s '%s' %.1f", entity->getId().toString().c_str(), name().c_str(), _KamiAnger);
//						EGSPD::forageKamiWrathPunishment(entity->getId(), name(), _KamiAnger);
					}
				}
			}
			// Don't reset the kami anger level to the min value, but nearly to the threshold 2 value!
			_KamiAnger = ForageKamiAngerThreshold2.get() - 1.0f; // +2 will reach the threshold 2 again
		}
	}
	return _KamiAnger;
}


//NLMISC_VARIABLE( bool, VerboseDeposits, "Verbose info for deposits" );

NLMISC_COMMAND( forageDisplayRecentForageSitesNb, "Display the number of recent forage sites", "" )
{
	log.displayNL( "%u forage sites", TotalNbRecentForageSites );
	return true;
}

NLMISC_COMMAND( forageDisplayRecentForageSitesInfo, "Display the info on all recent forage sites", "" )
{
	const std::vector<CDeposit*>& deposits = CZoneManager::getInstance().getDeposits();
	std::vector<CDeposit*>::const_iterator it;
	for ( it=deposits.begin(); it!=deposits.end(); ++it )
	{
		CDeposit *deposit = (*it);
		deposit->displayRecentForageSites( log );
	}
	return true;
}

NLMISC_COMMAND( forageEmptyAllRecentForageSites, "Empty all forage sites", "" )
{
	const std::vector<CDeposit*>& deposits = CZoneManager::getInstance().getDeposits();
	std::vector<CDeposit*>::const_iterator it;
	for ( it=deposits.begin(); it!=deposits.end(); ++it )
	{
		CDeposit *deposit = (*it);
		deposit->depleteAllRecentForageSites();
	}
	return true;
}

NLMISC_COMMAND( forageDisplayDeposit, "Display info about one or all the deposits", "<name=ALL> <extendedInfo=0>" )
{
	string depName = "ALL";
	bool extendedInfo = false;
	if ( args.size() > 0 )
	{
		depName = args[0];
		if ( args.size() > 1 )
			extendedInfo = (args[1] == "1");
	}
	CZoneManager::getInstance().dumpDeposits( log, depName, extendedInfo );
	return true;
}

struct TCompareDepositsByHighestKamiAnger : public std::binary_function<CDeposit*,CDeposit*,bool>
{
	bool operator() ( const CDeposit* d1, const CDeposit* d2 )
	{
		return d1->kamiAnger() > d2->kamiAnger();
	}
};

NLMISC_COMMAND( forageDisplayKamiAngerLevels, "Display the N deposits with the highest anger levels", "<N=10>" )
{
	uint nb = 10;
	if ( args.size() > 0 )
		NLMISC::fromString(args[0], nb);

	std::vector<CDeposit*> newDepositList = CZoneManager::getInstance().getDeposits();
	std::sort( newDepositList.begin(), newDepositList.end(), TCompareDepositsByHighestKamiAnger() );

	log.displayNL( "%u first deposits by kami anger:", nb );
	for ( uint i=0; i!=nb && i<newDepositList.size(); ++i )
	{
		log.displayRawNL( "%.1f\t%s", newDepositList[i]->kamiAnger(), newDepositList[i]->name().c_str() );
	}
	return true;
}

NLMISC_VARIABLE( bool, VerboseDepositFiltering, "Verbose the items parsed when filtering deposits" );
NLMISC_VARIABLE( bool, ExportDepositContents, "When loading the deposit primitives, export contents report" );
