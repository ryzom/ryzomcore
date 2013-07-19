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



#include "StdAfx.h"
#include "srg_utilities.h"
#include "game_share/protection_type.h"
#include "nel/misc/string_conversion.h"

/*
 * V3
 */

// Flags: overridden by the config file values

/// Generate only new materials sheets, don't modify existing
bool GenOnlyNewRawMaterials = true;

/// Allow to modify existing material sheets
enum TExistingRMAction { BrowseRM, ModifyRM, SkipRM } ExistingRMAction = BrowseRM;

/// Skip RM for creatures
bool SkipRawMaterialsForCreatures = false;

/// Browse creature sheets and set their raw material properties using raw materials generated or loaded
bool AssignRawMaterialsHarvestToCreatureSheets = true;

/// Prevent from overwriting raw material assignment to creatures when already done
bool AssignOnlyToUnassignedCreatures = true;

/// Skip RM for deposits
bool SkipRawMaterialsForDeposits = false;

/// Skip non-mission raw materials
bool GenOnlyMissionRawMaterials = false;

/// Generate deposit sheets using raw materials generated or loaded
bool AssignRawMaterialsHarvestToDepositSheets = true;

/// Browse all deposits found (to produce doc), including not generated ones
bool BrowseOtherDeposits = false;

/// Substitute invalid raw materials assignments in deposits with valid similar assignments
bool FixDeposits = false;

/// Write even blank properties (to erase old assignments)
bool EraseOldCreatureAssignments = false;

/// Test (for creatures: test previous assignment only, on deposit: test current assignment)
bool TestExistingAssigments = true;

/// Print the names of raw materials in a file
bool OutputNameList = false;

/// Display families and properties
bool DisplayFamAndProp = false;

/// Produce html documentation
bool ProduceDoc = true;

/// If not ~0, print all RMs of zone level
uint32 GetSelectionUntilLevel = ~0;

/// Max number of MP by family (by ecosystem and levelzone)
uint32 MaxNbRMByFamilyEZ = 10;

/// If true, sort by decreasing originality
bool SortByOriginality = false;

/// Max number of MP useful for one craft slot
uint32 MaxNbRMByCraftSlotE = 100;

/// Check if sheets (when loading them in browse mode) have valid durability
bool CheckDurability = false;

/// Generate files for WorldEditor listbox for families
bool GenerateDepositSystemMpFiles = true;

// Discard rm with originality lower than this value
//uint32 OriginalityMinThreshold = 0;

// Number of combinations to randomize per raw material (obsolete)
//uint32 NbFaberCombinations = 10;


const uint32 NB_RAW_MATERIAL_FAMILIES_PER_CREATURE = 9;
const uint32 NB_RAW_MATERIALS_PER_DEPOSIT = 30;

const string CommonCreatureCode = "zz";

CRulesFilter propertySetFilter, familyFaberFilter, familyCreatureFilter, creatureFamilyFilter, familyColorsFilter;

vector< CRulesFilter > ecosystemDepositFilters;

const char * locationNames		[NB_LOCATIONS] = { "InDeposits", "InCreatures" };


FILE *GraphFile;
CFileDisplayer ListDisplayer( "families_output.txt", true );
CLog ListLog;

uint32 nbMaterialSheetsProcessed = 0, nbRejectedProperties = 0, nbExistingSheetsLoaded = 0, nbExistingSheetsModified = 0, nbSheetsProcessed = 0, nbNewSheetsGenerated = 0;


typedef map< string, PROTECTION_TYPE::TProtectionType > CProtectionTypeMap;
CProtectionTypeMap	JewelProtectionTypeByFamilyEcosystem;
//bool HasSpecializedJewelProtectionTypeByCiv [NbCiv] = { false, false, false, false, false };
//set< PROTECTION_TYPE::TProtectionType > midProtectionTypesUsed;


CRMData						SortableData;

CTitles						Titles;

CMainStat					MainStat;

CRulesStr2Filter			CustomizedPropertiesSheetName;

//vector< CGenRawMaterial >	Repository;

/*
 *
 */
CSString	makeAbbrevName( CSString name, const CSkeletonMap& alreadyDone )
{
	CSString abbrevName = name.left( 6 ).toUpper();
	if ( (name.size() == abbrevName.size() + 1) &&
		 (abbrevName[abbrevName.size()-1] == 'E') && (tolower(name[name.size()-1]) == 'r') )
		abbrevName[abbrevName.size()-1] = 'R';

	CSkeletonMap::const_iterator isk;
	for ( isk=alreadyDone.begin(); isk!=alreadyDone.end(); ++isk )
	{
		if ( (abbrevName == (*isk).second.AbbrevName) && (name != (*isk).second.Name) )
			nlerror( "Duplicate abbrevName %s for %s and %s", abbrevName.c_str(), (*isk).second.Name.c_str(), name.c_str() );
	}
	return abbrevName;
}


/*
 * No duplicate check (multiple families have the same prefix name, ex: Abhaya)
 */
CSString	makeAbbrevName( CSString name )
{
	CSString abbrevName = name.left( 6 ).toUpper();
	if ( (name.size() == abbrevName.size() + 1) &&
		 (abbrevName[abbrevName.size()-1] == 'E') && (tolower(name[name.size()-1]) == 'r') )
		abbrevName[abbrevName.size()-1] = 'R';
	string::size_type p = abbrevName.find( " " );
	if ( p != string::npos )
		abbrevName = abbrevName.left( p );

	return abbrevName;
}


/// DISABLED
#if 0
/*
 * Returns false if the RM must NOT be generated.
 */
bool	CFaberCharacteristics::randomizeValues( TFaberInterestLevel interestLevel, TFaberInterestLevel nbInterestLevels, uint iVariant, float widthRatio, float peakOccurRatio, float baseBoost, TEcosystem iEcosystem, uint iFreq, uint iFamily )
{
	if ( FaberElement == ~0 )
		return true;
	if ( interestLevel == NAInterestLevel ) // no random values
		return true;

	uint32 rPeak = ~0;
	uint32 rCharac;

	// Boost 0, 1 or 2 characs, depending on iVariant and interestLevel
	uint rBoostedCharac [2] = { ~0, ~0 };
	if ( iVariant == 2 ) // A-E:1>=90%; F:5>=90%
	{
		do
		{
			rBoostedCharac[0] = getRandomValue( NbCharacs-1 );
		}
		while ( ! CharacSlotFilter[rBoostedCharac[0]][FaberElement] );
		/*if ( interestLevel == nbInterestLevels-1 ) // TODO
		{
			do
			{
				rBoostedCharac[1] = getRandomValue( NbCharacs-1 ); // currently, can loop infinitely if there is only one matching CharactSlotFilter
			}
			while ( (rBoostedCharac[1] == rBoostedCharac[0]) || (! CharacSlotFilter[rBoostedCharac[1]][FaberElement]) );
		}*/
	}

	// Browse characs
	for ( rCharac=0; rCharac!=NbCharacs; ++rCharac )
	{
		if ( ! CharacSlotFilter[rCharac][FaberElement] )
			continue;

		// Special cases
		if ( (FaberElement == ITEM_PART_JEWEL_GEM) && (rCharac == JewelProtection) )
		{
			if ( ! randomizeJewelProtection( (TStatQuality)interestLevel, iEcosystem, iFreq, iFamily ) )
				return false;
		}
		else // general case
		{
			// Randomize variation of interest level (different for each characteristic)
			/*TFaberInterestLevel deltaLevel = (sint)getRandomValue( nbInterestLevels ) - ((sint)(nbInterestLevels) / 2);*/
			TFaberInterestLevel actualLevel = interestLevel; /*max( (sint)0, min( (sint)nbInterestLevels-1, ((sint)interestLevel) + deltaLevel ) );*/
			sint factor;
			
			// Boosted charac?
			if ( (rCharac == rBoostedCharac[0]) )
			{
				Values[rCharac] = 90.0f + frand( 10.0f );
			}
			else
			{
				// Handle reversed interest
				if ( PositiveCharacs[rCharac] )
					factor = actualLevel;
				else
					factor = (nbInterestLevels - 1) - actualLevel;

				// Get a peak value?
				bool isPeak = false;
				if ( peakOccurRatio != 0.0f)
				{
					float peakDice = frand( 1.0f / peakOccurRatio );
					isPeak = (peakDice < 1.0f);
					if ( isPeak )
						rPeak = rCharac;
				}
				
				// Force a value higher than min? (if higher_or_eq than average, or if reversed interest)
				bool greaterThanMin = (actualLevel > nbInterestLevels/2) || (!PositiveCharacs[rCharac]);

				// Calculate min and max
				float minValue;
				float maxValue = isPeak ? PeakCharacValues[rCharac] : MaxCharacValues[rCharac];
				if ( greaterThanMin )
				{
					minValue = MinCharacValues[rCharac];
					maxValue -= minValue;
				}

				// Calculate value
				float normalSlice = maxValue / (float)nbInterestLevels;
				float enlargedSlice = normalSlice * widthRatio;
				float baseOfSlice = max( 0.0f, (((float)factor) * normalSlice) + (normalSlice-enlargedSlice)/2.0f ) + baseBoost;
				float value = min( baseOfSlice + frand( enlargedSlice ), maxValue );
				//nldebug( "%d/%d range=%.1f slice=%.1f enlarged=%.1f base=%.1f value=%.1f", factor, nbInterestLevels, maxValue, normalSlice, enlargedSlice, baseOfSlice, value );
				if ( greaterThanMin )
				{
					value += minValue;
				}

				// Prevent to get a null durability
				if ( rCharac == Durability )
					if ( value < 1.0f )
						value = 1.0f;

				Values[rCharac] = value;
				//nldebug( "%s: %s (level %s)", sCharacs[rCharac], isPeak?"PEAK":"normal",
				//	PositiveCharacs[rCharac] ? sInterestLevels[level] : sInterestLevels[(NbFaberInterestLevels - 1) - level] );
			}
		}
	}

	calcQualitativeValues();
	//nldebug( "%s (IL=%u)", CraftParts[FaberElement].Name.c_str(), interestLevel );

	/*// Display only "best"
	if ( interestLevel == nbInterestLevels-1 )
		nldebug( " %s %s Durability=%g Weight=%g, DMG=%g, Speed=%g, SapLoad=%g, Range=%g%s",
		faberElems[FaberElement].c_str(), sNomenclaturedInterestLevels[getNomenclaturedInterestLevel( interestLevel, nbInterestLevels )],
		Values[Durability], Values[Weight], Values[DMG], Values[Speed], Values[SapLoad], Values[Range],
		(rPeak!=~0) ? toString( " (PEAK for %s)", sCharacs[rPeak] ).c_str() : "" );*/

	return true;
}
#endif


/*
 * Returns false if the RM must NOT be generated.
 */
sint32	CFaberCharacteristics::computeValues( TStatQuality statQuality, const TFamInfo& famInfo, sint remarkableIndicesSetBaseIndex,
											  TEcosystem iEcosystem, uint iFreq, uint iFamily )
{
	nlassert( FaberElement != ~0 );
	nlassert( statQuality != NAInterestLevel );

	// Special case (jewels)
	if ( FaberElement == ITEM_PART_JEWEL_GEM )
	{
		computeJewelProtection( statQuality, iEcosystem, iFamily );
	}

	// First set all stats to the medium value for the required quality
	static float MediumStatsByStatQuality [NbStatQualities] = { 20.0f, 35.0f, 50.0f, 65.0f, 80.0f };
	float statEnergyAvg = MediumStatsByStatQuality[statQuality];
	for ( uint rCharac=0; rCharac!=NbCharacs; ++rCharac )
	{
		if ( CharacSlotFilter[rCharac][FaberElement] )
			Values[rCharac] = statEnergyAvg;
	}

	// Special case for RM with no boost or lowering
	if ( remarkableIndicesSetBaseIndex == -1 )
		return (sint32)statEnergyAvg;

	// Calculate the number of characs in filtered list
	/*uint nbFilteredCharacs = 0;
	for ( uint rCharac=0; rCharac!=NbCharacs; ++rCharac )
	{
		if ( CharacSlotFilter[rCharac][FaberElement] )
			++nbFilteredCharacs;
	}*/

	vector<bool> RemarkableByCharac( NbCharacs, false );

	// Boost one stat up (+40)
	float remaining = 0;
	/*Obsolete: remarkable indices were relative marked characs, not to the whole charac set
	uint rBestCharac = ~0;
	uint indexOfRemarkableCharacInFiltered = famInfo.RemarkableStatIndex[remarkableIndicesSetBaseIndex+RBest] % nbFilteredCharacs;
	sint indexOfCharacInFiltered = -1;
	for ( uint rCharac=0; rCharac!=NbCharacs; ++rCharac )
	{
		if ( CharacSlotFilter[rCharac][FaberElement] )
			++indexOfCharacInFiltered;
		if ( indexOfCharacInFiltered = indexOfRemarkableCharacInFiltered )
		{
			RemarkableByCharac[rCharac] = true;
			rBestCharac = rCharac;
			Values[rCharac] += 40.0f;
			if ( Values[rCharac] > 100.0f )
				remaining = Values[rCharac] - 100.0f;
		}
	}
	nlassert( rBestCharac != ~0 ); // we should have found the best index
	*/
	uint rBestCharac = famInfo.RemarkableStatIndex[remarkableIndicesSetBaseIndex+RBest];
	RemarkableByCharac[rBestCharac] = true;
	Values[rBestCharac] += 40.0f;

	// Lower two stats (-20, -20)
	for ( uint i=RWorst1; i<=RWorst2; ++i )
	{
		if ( famInfo.RemarkableStatIndex[remarkableIndicesSetBaseIndex+i] == -1 )
		{
			// Case (+20, -20) instead of (+40, -20, -20) because only two characs in filtered list
			nlassert( i==RWorst2 );
			Values[rBestCharac] -= 20.0f; // +40 already applied
			break; // -20 already applied when i==Worst1
		}

		/*indexOfRemarkableCharacInFiltered = famInfo.RemarkableStatIndex[remarkableIndicesSetBaseIndex+i] % nbFilteredCharacs;
		indexOfCharacInFiltered = -1;
		for ( uint rCharac=0; rCharac!=NbCharacs; ++rCharac )
		{
			if ( CharacSlotFilter[rCharac][FaberElement] )
				++indexOfCharacInFiltered;
			if ( indexOfCharacInFiltered = indexOfRemarkableCharacInFiltered )
			{
				if ( RemarkableByCharac[rCharac] )
					nlerror( "Twice the same remarkable charac" );
				RemarkableByCharac[rCharac] = true;
				Values[rCharac] -= 20.0f;
				nlassert( ! (Values[rCharac] < 0) );
			}
		}*/
		uint rCharac = famInfo.RemarkableStatIndex[remarkableIndicesSetBaseIndex+i];
		if ( RemarkableByCharac[rCharac] )
			nlerror( "Twice the same remarkable charac" );
		RemarkableByCharac[rCharac] = true;
		Values[rCharac] -= 20.0f;
		if ( (Values[rCharac] < 1.0f) && (rCharac == Durability) )
			Values[rCharac] = 1.0f; // prevent from having a null durability (= no item part)
		nlassert( ! (Values[rCharac] < 0) );
	}

	// Clamp max of Best charac (after lowering, because it can modify the Best charac)
	if ( Values[rBestCharac] > 100.0f )
	{
		remaining = Values[rBestCharac] - 100.0f;
		Values[rBestCharac] = 100.0f;
	}

	// Raise other with optional remaining (divided by the number to raise)
	if ( remaining > 0 )
	{
		float nbToRaise = 0;
		for ( uint rCharac=0; rCharac!=NbCharacs; ++rCharac )
		{
			if ( CharacSlotFilter[rCharac][FaberElement] && (!RemarkableByCharac[rCharac]) )
				++nbToRaise;
		}
		nlassert( nbToRaise != 0 );
		float remainingSlice = remaining / nbToRaise;
		for ( uint rCharac=0; rCharac!=NbCharacs; ++rCharac )
		{
			if ( CharacSlotFilter[rCharac][FaberElement] && (!RemarkableByCharac[rCharac]) )
			{
				Values[rCharac] += remainingSlice;
			}
		}
	}

	return (sint32)statEnergyAvg;
}


//// DISABLED
#if 0
/*
 * Returns false if the RM must NOT be generated.
 */
bool	CFaberCharacteristics::randomizeJewelProtection( TStatQuality statQuality, TEcosystem iEcosystem, uint iFreq, uint iFamily )
{
	PROTECTION_TYPE::TProtectionType protectionType;

	// Test if the family-ecosystem already has a jewel protection type
	string feKey = familyCodes[iFamily] + ecosystemCodes[iEcosystem];
	CProtectionTypeMap::iterator it = JewelProtectionTypeByFamilyEcosystem.find( feKey );
	if ( it != JewelProtectionTypeByFamilyEcosystem.end() )
	{
		protectionType = (*it).second;
	}
	else
	{
		/*if ( iFreq > 2 ) // 100% of "freq>=3" RM
		{
			protectionType = PROTECTION_TYPE::None;
		}
		else if ( iFreq > 1 )
		{
			uint iRandomPercent = getRandomValue( 100 );
			if ( iRandomPercent < 50 ) // 50% of "freq 2" RM
			{
				protectionType = PROTECTION_TYPE::None;
			}
			else // 50% of "freq 2" RM
			{
				uint iProType = getRandomValue( 3 );
				protectionType = (PROTECTION_TYPE::TProtectionType)(PROTECTION_TYPE::Cold + iProType);
			}
		}*/
		if ( iEcosystem == CommonEcosystem )
		{
			uint iRandomPercent = getRandomValue( 100 );
			if ( iRandomPercent < 50 ) // 50% of "freq 2" RM
			{
				protectionType = PROTECTION_TYPE::None;
			}
			else // 50% of "freq 2" RM
			{
				uint iProType = getRandomValue( 3 );
				protectionType = (PROTECTION_TYPE::TProtectionType)(PROTECTION_TYPE::Cold + iProType);
			}
		}
		else
		{
			// Get the civ speciality corresponding to the rm family (if any)
			/*const TFamInfo& rmFamily = FamSet[families[iFamily]];
			TCiv civ = AllCiv; //nlinfo( "%s %u", rmFamily.CompatibleCraftParts.c_str(), rmFamily.Properties.size() );
			uint iP = rmFamily.getPropIndexByCraftPart( 'A' + (char)ITEM_PART_JEWEL_GEM );
			if ( iP != ~0 )
				civ = rmFamily.Civs[iP];*/

			// Ensure there is one speciality protection type per civ
			TCiv civ = EcosystemToCiv[iEcosystem];
			if ( /*(civ != AllCiv)*/ (iEcosystem >= Desert) && (iEcosystem <= Jungle) && (!HasSpecializedJewelProtectionTypeByCiv[civ]) )
			{
				switch ( /*civ*/iEcosystem )
				{
				case /*Fyros*/Desert: protectionType = PROTECTION_TYPE::Fire; break;
				case /*Matis*/Forest: protectionType = PROTECTION_TYPE::Poison; break;
				case /*Tryker*/Lacustre: protectionType = PROTECTION_TYPE::Shockwave; break;
				case /*Zorai*/Jungle: protectionType = PROTECTION_TYPE::Electricity; break;
				}
				HasSpecializedJewelProtectionTypeByCiv[civ] = true;
			}
			else
			{
				uint iProType;
				// Ensure all values are used
				do
				{
					iProType = getRandomValue( PROTECTION_TYPE::None-PROTECTION_TYPE::Madness );
					protectionType = (PROTECTION_TYPE::TProtectionType)(PROTECTION_TYPE::Madness + iProType);
				}
				while ( (midProtectionTypesUsed.size() < PROTECTION_TYPE::None-PROTECTION_TYPE::Madness)
					&& midProtectionTypesUsed.find( protectionType ) != midProtectionTypesUsed.end() );
				midProtectionTypesUsed.insert( protectionType );
			}
		}
		JewelProtectionTypeByFamilyEcosystem.insert( make_pair( feKey, protectionType ) );
	}

	// Disable the protection if the interest level does not match the protection type (cold-rot always, madness-mm from C, fire-electric from E)
	/*if ( protectionType >= PROTECTION_TYPE::Madness )
	{
		if ( statQuality == Basic ) // reject Basic
		{
			protectionType = PROTECTION_TYPE::None;
			//return false; // do not suppress the RM as it can be used for another item part
		}
	}
	else if ( protectionType >= PROTECTION_TYPE::Fire )
	{
		if ( statQuality < Choice ) // reject Basic, Fine
		{
			protectionType = PROTECTION_TYPE::None;
			//return false;
		}
	}*/

	Values[JewelProtection] = (float)(uint)protectionType;
	return true;
}
#endif


/*
 *
 */
void	CFaberCharacteristics::computeJewelProtection( TStatQuality statQuality, TEcosystem iEcosystem, uint iFamily )
{
	TFamInfo& famInfo = FamSet[families[iFamily]];
	if ( famInfo.JewelProtIndex == -1 )
	{
		Values[JewelProtection] = (float)PROTECTION_TYPE::None;
	}
	else
	{
		uint protIndex = (uint)famInfo.JewelProtIndex;
		if ( protIndex < PROTECTION_TYPE::Fire )
		{
			Values[JewelProtection] = (statQuality >= Fine) ? (float)(PROTECTION_TYPE::TProtectionType)protIndex : (float)PROTECTION_TYPE::None;
		}
		else if ( protIndex == PROTECTION_TYPE::Fire )
		{
			static const PROTECTION_TYPE::TProtectionType EcosystemToProtectionType[NbEcosystems] = {
				PROTECTION_TYPE::None,			// Common
				PROTECTION_TYPE::Fire,			// Desert (Fyros)
				PROTECTION_TYPE::Poison,		// Forest (Matis)
				PROTECTION_TYPE::Shockwave,		// Lacustre (Tryker)
				PROTECTION_TYPE::Electricity,	// Jungle (Zorai)
				PROTECTION_TYPE::Fear			// Primeroot
			};
			Values[JewelProtection] = (statQuality >= Choice) ? (float)EcosystemToProtectionType[iEcosystem] : (float)PROTECTION_TYPE::None;
		}
		else if ( protIndex >= PROTECTION_TYPE::Madness )
		{
			Values[JewelProtection] = (statQuality >= Choice) ? (float)(PROTECTION_TYPE::TProtectionType)protIndex : (float)PROTECTION_TYPE::None;
		}
	}
}


/*
 * Reject a prop if any of the previous props are in its incompatibility list
 * (assumes the incompatibity lists are filled for every property)
 */
bool passPropSetFilter( const vector<uint32>& iPropertyRelatedProperties, uint32 *iProperties, sint rCurrentProp, uint32 iProp )
{
	for ( sint r=0; r!=rCurrentProp; ++r )
	{
		if ( ! passNegativeFilter( iPropertyRelatedProperties, iProperties[r] ) )
			return false;
	}
	return true;
}


/*
 * Reject a faber element if it is NOT in the compatibility list
 */
bool passFaberElementFilter( const vector<uint32>& iCompatibleFaberElements, uint32 rFaberElement )
{
	return passPositiveFilter( iCompatibleFaberElements, rFaberElement );
}

/*
 * Reject a color if it is in the incompatibility list of a family
 */
/*bool passColorFilter( const vector<uint32>& iFamilyRelatedColors, uint32 iColor )
{
	return passPositiveFilter( iFamilyRelatedColors, iColor );
}*/


/*
 *
 */
void	setCustomizedPropertyValues( UFormElm& node, const CSString& sheetFilename )
{
	CRulesStr2Filter::const_iterator icp = CustomizedPropertiesSheetName.find( sheetFilename );
	if ( icp != CustomizedPropertiesSheetName.end() )
	{
		uint nbCustomProps = 0;
		for ( vs::const_iterator ip=(*icp).second.begin(); ip!=(*icp).second.end(); ++ip )
		{
			CSString propName = *ip;
			++ip;
			if ( ip == (*icp).second.end() )
			{
				nlwarning( "Malformed CustomizedProperties line" );
				break;
			}
			CSString value = *ip;
			node.setValueByName( value.c_str(), propName.c_str() );
			++nbCustomProps;
		}
		nldebug( "%s has %u customized properties", sheetFilename.c_str(), nbCustomProps );
	}
}


/*
 *
 */
void randomizeProperties( uint32 *iProperties, uint32 iFamily, uint32 iEcosystem, uint32 &nbRejectedProperties )
{
	nlerror( "TODO" );
#if 0
	sint rProperty; // property rank
	// Randomize properties
	for ( rProperty=0; rProperty!=NbPropertySlots; ++rProperty )
	{
		uint32 iProp, nbTries = 0;
		
		// Get random value and apply property rules filters
		do
		{
			if ( nbTries != 0 ) // debug display
			{
				//nldebug( "Rejected property %s in family %s with", properties[iProp].c_str(), families[iFamily].c_str() );
				//for ( sint r=0; r!=rProperty; ++r )
				//	DebugLog->displayRaw( "%s ", properties[iProperties[r]].c_str() );
				//DebugLog->displayRawNL( "" );
			}
			iProp = getRandomValue( properties.size() );
			++nbTries;
		}
		while ( ! (passPropFamilyFilter( familyPropertyFilter[iFamily], iProp ) &&
				   passPropSetFilter( propertySetFilter[iProp], iProperties, rProperty, iProp )) );
		nbRejectedProperties += nbTries - 1;

		// Remove redundancy (undefine if already set)
		for ( sint r=0; r!=rProperty; ++r )
		{
			if ( iProp == iProperties[r] )
			{
				iProp = UndefinedProperty;
				break;
			}
		}

		iProperties[rProperty] = iProp;
	}

	// Move undefined properties at the back
	sint lastPropertySlot = NbPropertySlots-1;
	for ( rProperty=lastPropertySlot-1; rProperty>=0; --rProperty )
	{
		if ( iProperties[rProperty] == UndefinedProperty )
		{
			sint lastUsedPropertySlot = getLastUsedPropertySlot( iProperties, lastPropertySlot, UndefinedProperty );
			if ( lastUsedPropertySlot == -1 )
			{
				nlwarning( "No matching properties for %s %s", ecosystems[iEcosystem].c_str(), families[iFamily].c_str() );
			}
			if ( lastUsedPropertySlot > rProperty )
			{
				uint32 itmp = iProperties[lastUsedPropertySlot];
				iProperties[lastUsedPropertySlot] = iProperties[rProperty];
				iProperties[rProperty] = itmp;
			}
		}
	}
#endif
}




/*
 * Returns name in capitals, or an empty string if no match
 */
//string findRawMaterialFromCriteria( uint32 iEcosystem, uint32 iFamily, uint32 iCreature, TFaberInterestLevel actualLevel, char **errorReport )
//{
//	vector<CFaberCombination> *codes = NULL;
//	bool found = false;
//
//	if ( iCreature != ~0 )
//	{
//		// 1. Match ecosystem and creature specialization
//		RawMaterialRepository.getFaberCombinationCodes( iEcosystem, iFamily, iCreature, &codes );
//		if ( codes && (! codes->empty()) )
//			found = true;
//		else
//		{
//			// 2. Match creature specialization in common ecosystem
//			RawMaterialRepository.getFaberCombinationCodes( CommonEcosystem, iFamily, iCreature, &codes );
//			if ( codes && (! codes->empty()) )
//				found = true;
//		}
//	}
//	if ( ! found )
//	{
//		iCreature = ~0;
//
//		// 3. Match ecosystem and common creature
//		RawMaterialRepository.getFaberCombinationCodes( iEcosystem, iFamily, iCreature, &codes );
//		if ( codes && (! codes->empty()) )
//			found = true;
//		else
//		{
//			// 4. Match common creature in common ecosystem
//			RawMaterialRepository.getFaberCombinationCodes( CommonEcosystem, iFamily, iCreature, &codes );
//			if ( codes && (! codes->empty()) )
//				found = true;
//		}
//	}
//	if ( ! found )
//	{
//		*errorReport = "no compatible family";
//		return "";
//	}
//	else
//	{
//		// Remap requested interest level if the material found is from common type
//		if ( iCreature == ~0 )
//		{
//			actualLevel = actualLevel * NbFaberInterestLevelsByEcosystem[CommonEcosystem] / NbFaberInterestLevelsByEcosystem[iEcosystem];
//		}
//
//		// Discard faber codes that are higher than the requested level
//		//nldebug( "Codes 1: %u", codes->size() );
//		vector<CFaberCombination*> matchingCodes;
//		vector<CFaberCombination>::iterator ic;
//		for ( ic=codes->begin(); ic!=codes->end(); ++ic )
//		{
//			if ( hasMatchingFaberLevel( (*ic).FirstLevel, actualLevel ) )
//				matchingCodes.push_back( &(*ic) );
//		}
//		if ( matchingCodes.empty() )
//		{
//			*errorReport = "no compatible level";
//			return "";
//		}
//		//nldebug( "Codes 2: %u", matchingCodes.size() );
//
//		// Select best level matches
//		keepOnlyHighestLevel( matchingCodes );
//		nldebug( "%s has %u matches", families[iFamily].c_str(), matchingCodes.size() );
//
//		// Choose one faber combination
//		uint32 iComb = getRandomValue( matchingCodes.size() );
//
//		// Build string
//		string crStr = (iCreature == ~0) ? CommonCreatureCode : creatureCodes[iCreature];
//		string ilStr = toString( "%02d", matchingCodes[iComb]->FirstLevel + 1 ); // 00 is NAInterestLevel;
//		string faberStr( NB_FABERELEMS_CODE_CHARS, ' ' );
//		memcpy( &faberStr[0], matchingCodes[iComb]->Code.Ch, NB_FABERELEMS_CODE_CHARS );
//		string name = "m" + familyCodes[iFamily] + crStr + ecosystemCodes[iEcosystem] + ilStr + faberStr;
//		strlwr( name );
//		//nlinfo( "%s", name.c_str() );
//		return name;
//	}
//}


/*
 * 1st arg: whichArray
 * 2nd arg: col
 * 3rd arg: iFamily
 */
typedef void (*TDispatchFunc) ( uint32, uint32, uint32 );


/*
 * Build SkgroupToModels and CreatureModels
 */
void	deliverCreatureModels( vs& srcRow )
{
	if ( ! srcRow[0].empty() )
	{
		bool isModelRow = (srcRow.size()>2) && (srcRow[1]=="M");
		bool isNameRow = (srcRow.size()>2) && (srcRow[1]=="N");
		for ( uint32 c=2; c!=srcRow.size(); ++c )
		{
			if ( ! srcRow[c].empty() )
			{
				if ( isModelRow && (!srcRow[c].empty()) )
				{
					// Models (ex: HD)
					SkgroupToModels[srcRow[0]].push_back( srcRow[c] );
					nldebug( "Model: '%s' for %s (%u)", srcRow[c].c_str(), srcRow[0].c_str(), SkgroupToModels[srcRow[0]].size()-1 );
				}
				else if ( isNameRow )
				{
					// Creature names (by model) (assumes names are below filenames)
					vs& modelsOfGroup = SkgroupToModels[srcRow[0]];
					uint32 skIndex = c - 3; // warning: beginning at column D
					if ( skIndex < modelsOfGroup.size() )
					{
						CreatureModels[modelsOfGroup[skIndex]].Name = srcRow[c];
						CSString abbrevName = makeAbbrevName( srcRow[c], CreatureModels );
						CreatureModels[modelsOfGroup[skIndex]].AbbrevName = abbrevName;
						nldebug( "Name: %s '%s' %s", modelsOfGroup[skIndex].c_str(), srcRow[c].c_str(), abbrevName.c_str() );
						
					}
					else
						nlwarning( "Can't set name %s for %s (%u)", srcRow[c].c_str(), srcRow[0].c_str(), skIndex );
				}
			}
		}
	}
}


/*
 *
 */
/*void	deliverSkgroups( vs& srcRow )
{
	if ( ! srcRow[0].empty() )
	{
		for ( uint32 c=1; c!=srcRow.size(); ++c )
		{
			if ( ! srcRow[c].empty() )
			{
				if ( srcRow[c].findNS( ".skel" ) != string::npos )
				{
					// Skeleton filenames
					SkgroupToModels[srcRow[0]].push_back( srcRow[c] );
					nldebug( "Skeleton: '%s' for %s (%u)", srcRow[c].c_str(), srcRow[0].c_str(), SkgroupToModels[srcRow[0]].size()-1 );
				}
				else
				{
					// Creature names (by skeleton) (assumes names are below filenames)
					vs& skeletonsOfGroup = SkgroupToModels[srcRow[0]];
					uint32 skIndex = c - 3; // warning: beginning at column D
					if ( skIndex < skeletonsOfGroup.size() )
					{
						CreatureModels[skeletonsOfGroup[skIndex]].Name = srcRow[c];
						CSString abbrevName = makeAbbrevName( srcRow[c], CreatureModels );
						CreatureModels[skeletonsOfGroup[skIndex]].AbbrevName = abbrevName;
						nldebug( "Name: %s '%s' %s", skeletonsOfGroup[skIndex].c_str(), srcRow[c].c_str(), abbrevName.c_str() );
						
					}
					else
						nlwarning( "Can't set name %s for %s (%u)", srcRow[c].c_str(), srcRow[0].c_str(), skIndex );
				}
			}
		}
	}
}*/

/*
 * Build CreatureToModel (obsolete, used creature_skeletons.csv)
 */
/*void	deliverSkeletons( vs& srcRow )
{
	if ( (srcRow[0] != "FILE") && (srcRow.size()>1) && (!srcRow[1].empty()) )
	{
		// Remove quotes
		string::size_type p = 0;
		while ( p < srcRow[1].size() )
		{
			if ( srcRow[1][p] == '"' )
				srcRow[1].erase( p, 1 );
			else
				++p;
		}
		if ( ! srcRow[1].empty() )
		{
			// Read mapping creature -> skeleton
			CSkeletonMap::iterator ism = CreatureModels.find( srcRow[1] );
			if ( ism == CreatureModels.end() )
				nldebug( "%s", srcRow[1].c_str() );

			if ( (srcRow[0].size() == 6) && (srcRow[0][0] == 'c') )
			{
				TSkeletonInfo& skeInfo = CreatureModels[srcRow[1]];
				CSString creaModel = srcRow[0].left( 5 );
				CreatureFamilyIndices[creaModel] = vu();
				skeInfo.IsUsed = true;
				skeInfo.CreaturesOfSke.push_back( srcRow[0] );
				CreatureToModel[creaModel] = srcRow[1];
				//nlinfo( "Selecting %s (%s)", srcRow[1].c_str(), creaModel.c_str() );
			}
			else
			{
				//nlinfo( "Discarding %s", srcRow[1].c_str() );
				CreatureModels.insert( make_pair( srcRow[1], TSkeletonInfo() ) );
			}
		}
	}
}*/


/*
 *
 */
/*void	dispatchToCreatures( uint32, uint32 col, uint32 iFamily )
{
	nlassert( ! skeletonGroupColumns.empty() ); // missing SkeletonGroup line?
	CSString& skegrp = skeletonGroupColumns[col];
	vs& skeletons = SkgroupToModels[skegrp];
	if ( skeletons.empty() )
		nlwarning( "No skeleton found for group '%s'", skegrp.c_str() );
	for ( vs::const_iterator isk=skeletons.begin(); isk!=skeletons.end(); ++isk )
	{
		const CSString& ske = *isk;
		CreatureModels[ske].SkGroup = skegrp;
		nldebug( "%u: %s -> %s", col, skegrp.c_str(), ske.c_str() );

		vs& creatures = CreatureModels[ske].CreaturesOfSke;
		for ( vs::const_iterator ic=creatures.begin(); ic!=creatures.end(); ++ic )
		{
			const CSString& creaCode = *ic;
			if ( (creaCode.size() == 6) && (creaCode[0] == 'c') && (creaCode[5]>='0') && (creaCode[5]<='5') )
			{
				CSString creaModel = creaCode.rightCrop( 1 ); // don't use local level
				vu& familiesForCreature = CreatureFamilyIndices[creaModel];
				if ( find( familiesForCreature.begin(), familiesForCreature.end(), iFamily ) == familiesForCreature.end() )
				{
					familiesForCreature.push_back( iFamily );
					if ( ! CreatureModels[ske].IsUsed )
						nldebug( "Now %s used", ske.c_str() );
					CreatureModels[ske].IsUsed = true;
					CreatureToModel[creaModel] = ske;
					//nldebug( "%s -> %s %s %s", creaModel.left( 3 ).c_str(), CreatureModels[ske].Name.c_str(), CreatureModels[ske].AbbrevName.c_str(), ske.c_str() );
				}
			}
		}
	}
}*/


/*
 *
 */
/*void    deliverCreatures( vs& srcRow )
{
	if ( srcRow[0] == "SkeletonGroup" )
	{
		srcRow[0] = srcRow[0].strip();
		skeletonGroupColumns.push_back( "" ); // column 0
		for ( uint32 c=1; c!=srcRow.size(); ++c )
		{
			if ( ! srcRow[c].empty() )
			{
				// Set new value in column list
				skeletonGroupColumns.push_back( srcRow[c] );
			}
			else
			{
				// Copy last value in column list
				if ( ! skeletonGroupColumns.empty() )
					skeletonGroupColumns.push_back( skeletonGroupColumns.back() );
			}
		}
		nldebug( "%u skeleton group columns", skeletonGroupColumns.size() );
	}
}*/


/*
 *
 */
void	deliverItemPartParams( vs& srcRow )
{
	const uint itemPartCol = 0; // 1 from base 1
	const uint legendCol = 6;
	const uint firstCharacCol = 7; // 8 from base 1

	if ( srcRow.size() < legendCol+1 )
		return;

	// Read item part index
	uint rItemPart = ~0;
	if ( ! srcRow[itemPartCol].empty() )
		rItemPart = (uint)(srcRow[itemPartCol][0] - 'A');

	// Select item part compatibility or charac bound
	uint characParam = ~0;
	if ( srcRow[legendCol].empty() )
	{
		if ( rItemPart != ~0 )
		{
			if ( srcRow.size() < firstCharacCol+1 )
			{
				nlwarning( "Can't find compatibility for craft characs of item parts %s", srcRow[itemPartCol].c_str() );
				return;
			}

			characParam = 0;
			nldebug( "Loading compability for craft characs of item part %s", srcRow[itemPartCol].c_str() );
			if ( ! CraftParts.isEnabled( rItemPart ) )
				nlinfo( "Item part %s was absent from rm_fam_prop.csv (disabled)", srcRow[itemPartCol].c_str() );
		}
		else
		{
			return;
		}
	}
	else if ( srcRow[legendCol] == "Positive" )
	{
		characParam = 1;
		nldebug( "Loading calc way for craft characs" );
	}
	else if ( srcRow[legendCol] == "Min" )
	{
		characParam = 2;
		nldebug( "Loading min bounds for craft characs" );
	}
	else if ( srcRow[legendCol] == "Max" )
	{
		characParam = 3;
		nldebug( "Loading max bounds for craft characs" );
	}
	else if ( srcRow[legendCol] == "Peak" )
	{
		characParam = 4;
		nldebug( "Loading peak bounds for craft characs" );
	}
	else
	{
		nlwarning( "Unknown legend for item part charac: %s", srcRow[legendCol].c_str() );
		return;
	}

	// Read item part compatibility or charac bounds
	for ( uint c=firstCharacCol; c<srcRow.size(); ++c )
	{
		uint rCharac = c-firstCharacCol;

		switch ( characParam )
		{
		case 0: CharacSlotFilter[rCharac][rItemPart] = (rCharac!=JewelProtection) &&
				((srcRow[c] == "X") || (srcRow[c] == "D") || (srcRow[c] == "C")); break; // don't include JewelProtection, will be processed separately
		case 1: PositiveCharacs[rCharac] = (srcRow[c] == "1"); break;
		case 2: MinCharacValues[rCharac] = (float)atoi( srcRow[c].c_str() ); break;
		case 3: MaxCharacValues[rCharac] = (float)atoi( srcRow[c].c_str() ); break;
		case 4: PeakCharacValues[rCharac] = (float)atoi( srcRow[c].c_str() ); break;
		}
	}
	CharacSlotFilter[CraftCivSpec][rItemPart] = false; // same as JewelProtection
}


/*
 *
 */
/*CSString	getRMName( const CSString& code )
{
	if ( (code.size() != 11) || (code[0] != 'm') )
		return toString( "Invalid code %s", code.c_str() );

	CSString res;
	if ( code[1] == 'c' )
	{
		// Adjective (relative to creature)
		CSString adjCode = code.substr( 8, 1 );
		res += adjectives[adjCode] + " ";

		// Creature skeleton
		CSString creaModel = code.substr( 1, 5 );
		res += CreatureModels[CreatureToModel[creaModel]].Name + " ";
	}
	else if ( code[1] == 'd' )
	{
		// Deposit
		uint32 i;
		for ( i=0; i!=NB_DEPOSITS; ++i )
		{
			if ( tolower(depositTypeNames[i][0]) == tolower(code[2]) )
			{
				res += CSString(depositTypeNames[i]) + " ";
				break;
			}
		}
		if ( i == NB_DEPOSITS )
			res += " ";
	}
	// Ecosystem
	CSString ecoCode = code.substr( 4, 1 );
	uint32 iEcosystem = getIndexFromString( ecoCode, ecosystemCodes );
	if ( iEcosystem != ~0 )
		res += ecosystems[iEcosystem] + " ";

	// Level zone
	res += toString( "zone %c ", code[5] );

	if ( code[1] == 'd' )
	{
		// Adjective (relative to vegetal)
		CSString adjCode = code.substr( 8, 1 );
		res += adjectives[adjCode] + " ";
	}

	// Family
	CSString famCode = code.substr( 6, 2 );
	uint32 iFam = getIndexFromString( famCode, familyCodes );
	if ( iFam != ~0 )
		res += families[iFam] + " ";
	
	// Specialization
	res += code.substr( 9, 2 );

	return res;
}*/


void	capitalize( CSString& s )
{
	if ( ! s.empty() )
	{
		s[0] = toupper( s[0] );
	}
}


bool	endsWith( const CSString& s, const CSString& substring )
{
	return (s.right( substring.size() ) == substring);
}


/*
 * Can have a capital if contains a proper name.
 * Returns "!" in case of error
 */
CSString	getEcosystemLowerName( const CSString& ecoCode )
{
	CSString ecosystemTitle;
	switch ( ecoCode[0] )
	{
	case 'c': return ""; /*"common"*/; break;
	case 'l': return "Lake"; break;
	case 'p': return "Prime Root"; break;
	default:
		{
			uint32 iEcosystem = getIndexFromString( ecoCode, ecosystemCodes );
			if ( iEcosystem == ~0 )
				return "!";
			else
				return ecosystems[iEcosystem];
		}
	}
}

/*
 *
 */
CSString	getRMShortName( const CSString& code, TStatQuality statQuality, bool isMissionRawMaterial )
{
	if ( (code.size() != SIZE_RAW_MATERIAL_SHEET_FILENAME - 6 /*size of ".sitem"*/) || (code[0] != 'm') )
		return toString( "Invalid code %s", code.c_str() );

	CSString res;
	CSString ia = "a";

	// Adjective
	if ( isMissionRawMaterial )
	{
		// Adjective for levelzone (not needed for creatures, already in creature name)
		CSString adjCode = code.substr( RM_INDEX_LEVELZONE_CODE, 1 );
		//res += adjectives[adjCode] + " ";
		uint zoneLevel = (uint)(code[RM_INDEX_LEVELZONE_CODE]-'a');
		switch ( zoneLevel )
		{
		case 0: res += "Plain "; break;
		case 1: res += "Average "; break;
		case 2: res += "Prime "; break;
		case 3: res	+= "Select "; break;
		case 4: res += "Superb "; break;
		case 5: res += "Magnificient "; break;
		}
	}
	else
	{
		switch ( statQuality )
		{
		case Basic:		res += "Basic "; break;
		case Fine:		res += "Fine "; break;
		case Choice:	res	+= "Choice "; break;
		case Excellent:	res += "Excellent "; ia = "an"; /*will be overwritten if there's a prefix*/ break;
		case Supreme:	res += "Supreme "; break;
		}
	}

	// Ecosystem
	CSString eco = getEcosystemLowerName( code.substr( RM_INDEX_ECOSYSTEM_CODE, NB_ECOSYSTEM_CODE_CHARS ) );
	if ( eco == "!" )
		nlerror( "Bad ecosystem in %s", code.c_str() );
	if ( ! eco.empty() )
		eco += " ";
	capitalize( eco );
	res += eco;

	/*if ( code[RM_INDEX_CREATURE_CODE] == 'c' )
	{
		// Creature skeleton (keep capital)
		CSString creaModel = code.substr( RM_INDEX_CREATURE_CODE, NB_CREATURE_CODE_CHARS );
		res += CreatureModels[CreatureToModel[creaModel]].Name + " ";
	}*/

	// Family
	CSString famCode = code.substr( RM_INDEX_FAMILY_CODE, NB_FAMILY_CODE_CHARS );
	uint32 iFam = getIndexFromString( famCode, familyCodes );
	if ( iFam == ~0 )
		nlerror( "Invalid family in %s", code.c_str() );
	CSString familyName = families[iFam];
	res += familyName;

	// Analyse family name (prefix...)
	CSString prefix;
	if ( endsWith( familyName, "Wood" ) )
		prefix = "Bundle";
	else if ( endsWith( familyName, "Bark" ) || endsWith( familyName, "Moss" ) || endsWith( familyName, "Sawdust" ) ||
			  endsWith( familyName, "Straw" ) || endsWith( familyName, "Dust" ) || endsWith( familyName, "Soil" ) ||
			  endsWith( familyName, "Cereal" ) )
		prefix = "Handful";
	else if ( endsWith( familyName, "Resin" ) || endsWith( familyName, "Wax" ) )
		prefix = "Portion";
	else if ( endsWith( familyName, "Whiskers" ) || endsWith( familyName, "Hairs" ) )
		prefix = "Tuft";
	else if ( endsWith( familyName, "Silk" ) )
		prefix = "Ball";
	else if ( endsWith( familyName, "Sap" ) || endsWith( familyName, "Residue" ) || endsWith( familyName, "Honey" ) ||
			  endsWith( familyName, "Blood" ) )
		prefix = "Phial";
	else if ( endsWith( familyName, "Fruit" ) )
		prefix = "Piece";
	else if ( endsWith( familyName, "Flesh" ) )
		prefix = "Morsel";
	else if ( endsWith( familyName, "Saliva" ) )
		prefix = "Sample";
	else if ( endsWith( familyName, "Pollen" ) || endsWith( familyName, "Fiber" ) || endsWith( familyName, "Amber" ) ||
			  endsWith( familyName, "Leather" ) || endsWith( familyName, "Oil" ) )
		ia = "some";
	else if ( endsWith( familyName, "Pelvis" ) || endsWith( familyName, "Eye" ) || endsWith( familyName, "Spine" ) ||
			  endsWith( familyName, "Hoof" ) || endsWith( familyName, "Mandible" ) || endsWith( familyName, "Claw") ||
			  endsWith( familyName, "Tail" ) || endsWith( familyName, "Trunk" ) || endsWith( familyName, "Shell" ) ||
			  endsWith( familyName, "Sting" ) || endsWith( familyName, "Skin" ) || endsWith( familyName, "Beak" ) ||
			  endsWith( familyName, "Wing" ) || endsWith( familyName, "Horn" ) || endsWith( familyName, "Rostrum" ) ||
			  endsWith( familyName, "Skull" ) || endsWith( familyName, "Pistil" ) )
		prefix = "Fragment"; // number-limited creature objects

	// See also Kitin Claw outside this function

	CSString singularWithNoPrefix = res;
	if ( ! prefix.empty() )
	{
		res = prefix + " of " + res;
		ia = "a";
	}
	CSString singular = res;

	// A, The
	res += ";" + ia + ";the;";

	// Plural
	if ( prefix.empty() )
		res += singular + "s";
	else
		res += prefix + "s" + " of " + singularWithNoPrefix;

	res += ";;the;;";

	return res;
}


// _m + 1 char
CSString	getEcoParentName( uint32 iEcosystem )
{
	return CSString("_m") + ecosystemCodes[iEcosystem] + "." + rmSheetType;
}

/*
 *
 */
void	writeParentSheetEco( CForm *form, uint32 iEcosystem )
{
	// Write parent sheet for ecosystem
	clearSheet( form, &form->getRootNode() );
	form->getRootNode().setValueByName( ecosystems[iEcosystem].c_str(), "mp.Ecosystem" );
	form->getRootNode().setValueByName( "raw material (mp)", "basics.family" );
	form->getRootNode().setValueByName( (sint32)99, "basics.stackable" );
	form->getRootNode().setValueByName( "Faber", "mp.Category" );
	CIconInfo& iconInfo = Icons[ecosystems[iEcosystem]];
	if ( ! iconInfo.IconBackground.empty() )
		form->getRootNode().setValueByName( iconInfo.IconBackground.c_str(), "3d.icon background" );
	else
		nlwarning( "Can't find icon background for ecosystem %s", ecosystems[iEcosystem].c_str() );

	CSString ecoParentName = getEcoParentName( iEcosystem );

	setCustomizedPropertyValues( form->getRootNode(), ecoParentName );

	// Don't save in ecosystem directory but in parent dir
	flushSheetToDisk( rawMaterialPath + "_parent/" + ecoParentName, form );
}


// _m + 2 chars
CSString	getFamParentName( uint32 iFamily )
{
	return CSString("_m") + familyCodes[iFamily] + "." + rmSheetType;
}

/*
 *
 */
void	writeParentSheetFam( CForm *form, uint32 iFamily )
{
	const TFamInfo& famInfo = FamSet[families[iFamily]];
	if ( ! famInfo.IsActive )
		return;

	// Family
	clearSheet( form, &form->getRootNode() );
	form->getRootNode().setValueByName( families[iFamily].c_str(), "mp.Family" );

	// Group
	if ( famInfo.Group != ~0 )
		form->getRootNode().setValueByName( groups[famInfo.Group].c_str(), "mp.Group" );

	// Icons
	CIconInfo& iconInfo = Icons[families[iFamily]];
	if ( ! iconInfo.Icon.empty() )
		form->getRootNode().setValueByName( iconInfo.Icon.c_str(), "3d.icon" );
	else
	{
		nlwarning( "Can't find icon for family %s", families[iFamily].c_str() );
		/*CIconInfo& iconInfo2 = Icons["Unknown"];
		if ( ! iconInfo2.Icon.empty() )
			form->getRootNode().setValueByName( iconInfo2.Icon.c_str(), "3d.icon" );
		else
			nlwarning( "Can't find ricon for default background (Unknown)" );*/
	}

	//form->getRootNode().setValueByName( "TODO", "mp.HarvestSkill" );
	CSString famParentName = getFamParentName( iFamily );

	// New: abrevname even for raw materials from deposits
	if ( ! famInfo.IsInCreatures )
	{
		CSString& abbrevName = makeAbbrevName( families[iFamily] );
		if ( ! abbrevName.empty() )
			form->getRootNode().setValueByName( abbrevName.c_str(), "3d.text overlay" );
	}

	// Bulk
	form->getRootNode().setValueByName( "0.5", "basics.Bulk" );
	
	// Sellable?
	form->getRootNode().setValueByName( (!famInfo.IsForMission) ? "true" : "false", "basics.Drop or Sell" );
	/*bool tmpTest; // check bool read/write
	form->getRootNode().getValueByName( tmpTest, "basics.Drop or Sell" );
	nlassert( tmpTest == (!famInfo.IsForMission) );*/

	setCustomizedPropertyValues( form->getRootNode(), famParentName );

	// Don't save in ecosystem directory but in parent dir
	flushSheetToDisk( rawMaterialPath + "_parent/" + famParentName, form );
}


// _m + 3 chars
CSString	getCreaParentName( const CSString& creaMainModel )
{
	return CSString("_m") + creaMainModel + "." + rmSheetType;
}

/*
 *
 */
void	writeParentSheetCreature( CForm *form, const CSString& creaModel )
{
	// Write abbrev name
	//nldebug( "%s %s %s %u", creaModel.c_str(), CreatureToModel[creaModel].c_str(), CreatureModels[CreatureToModel[creaModel]].SkGroup.c_str(), CreatureModels[CreatureToModel[creaModel]].CreaturesOfSke.size() );
	CSString& abbrevName = CreatureModels[creaModel].AbbrevName;
	if ( ! abbrevName.empty() )
		form->getRootNode().setValueByName( abbrevName.c_str(), "3d.text overlay" );
	else
		nlwarning( "Can't find abbrev name for %s", creaModel.c_str() );

	CSString creaParentName = "_mc" + creaModel.toLower() + "." + rmSheetType;

	setCustomizedPropertyValues( form->getRootNode(), creaParentName );

	// Save in base dir
	flushSheetToDisk( rawMaterialPath + "_parent/" + creaParentName, form );
}


/*
 *
 */
bool	CMainStat::updateCraftStatistics( uint32 rFaberElem, uint32 iEcosystem, uint32 iFam, TCiv civ )
{
	++NbRMByFaberElem[rFaberElem];
	++NbRMByFaberElemByEcosystem[iEcosystem][rFaberElem];
	++NbRMByFaberElemByFamilyAndCiv[iFam][civ][rFaberElem];
	return true;
}


/*
 * Returns false if the RM must NOT be generated.
 */
bool	CGenRawMaterial::computeCraftCharacs( uint iVariant, const CSString& sheetName )
{
	/*CSString props;
	for ( uint32 p=0; p!=RMProperties.size(); ++ p )
		props += " " + properties[RMProperties[p]];
	nldebug( "%s %s %s", locFam.c_str(), props.c_str(), (familyGroups[iFam]==~0) ? "-" : groups[familyGroups[iFam]].c_str() );*/

	vector<float> actualEnergies( NbFaberElements, 0.0f );
	vector<float> sapLoads( NbFaberElements, 0.0f );
	TFamInfo& famInfo = FamSet[familyStr()];
	sint masterSumFreq = 0;

	// Compute craft scores
	sint nbFaberElemsFilled = 0;
	for ( uint32 rFaberElem=0; rFaberElem!=NbFaberElements; ++rFaberElem )
	{
		// Exclude if in exclusion list
		if ( hasCraftPart( rFaberElem ) )
		{
			/*sint rarityModifier = 0;
			float baseBoost = 0.0f;

			// For the PrimeRoots ecosystem, increase the stats and rarity
			if ( IEcosystem == PrimeRoots )
			{
				++rarityModifier;
				baseBoost = 10.0f;
			}*/

			/*// For raw materials specialized by civ matching the ecosystem, increase their craft stats
			TCiv civ = getCivSpec( rFaberElem, famInfo );
			if ( civ != AllCiv )
			{
				uint iCivEcosystem = getIndexFromString( string(CivEcosystemCodes[civ]), ecosystemCodes );
				if ( iCivEcosystem == IEcosystem )
				{
					++rarityModifier;
				}
			}*/

			// For rare raw materials, increase their craft stats
			sint sumFreq = 0;
			for ( vu::iterator ivf=famInfo.Freqs.begin(); ivf!=famInfo.Freqs.end(); ++ivf )
				sumFreq += (sint)(*ivf);
			sint avgFreq = sumFreq / (sint)famInfo.Freqs.size();
			/*avgFreq = max( avgFreq - rarityModifier, 0 );
			masterSumFreq += avgFreq;
			float peakOccurRatio;
			switch ( avgFreq )
			{
			case 0:  peakOccurRatio = 0.60f; break; // 60%
			case 1:  peakOccurRatio = 0.40f; break; // 40%
			case 2:  peakOccurRatio = 0.20f; break; // 20%
			default: peakOccurRatio = 0.01f / (float)(avgFreq-2); // 3=1%, 4=0.5%, 5=0.33%...
			}*/
			//float peakOccurRatio = 0.01f;

			// Compute scores (5 levels, width ratio=2 i.e. enlargedSlice=40%)
			CFaberCharacteristics craftSlot;
			craftSlot.initFaberElement( rFaberElem );
			// Now, the stat values are no more random (except special things such as jewels)
			/*if ( ! craftSlot.randomizeValues( (TFaberInterestLevel)(StatQuality), 5, iVariant, 2.0f, peakOccurRatio, baseBoost, (TEcosystem)IEcosystem, avgFreq, IFamily ) )
				return false;*/
			sint remarkableIndicesSetBaseIndex = nbFaberElemsFilled * 3;
			if ( avgFreq == 0 )
				remarkableIndicesSetBaseIndex = -1; // "Kitin Larva" : all at 80
			else
				nlassert( nbFaberElemsFilled < 2 );
			sint32 newStatEnergy = craftSlot.computeValues( StatQuality, famInfo, remarkableIndicesSetBaseIndex, (TEcosystem)IEcosystem, avgFreq, IFamily );
			nlassert( (StatEnergyAvg == 0) || (newStatEnergy == StatEnergyAvg) || (newStatEnergy == -1) );
			if ( newStatEnergy == -1 )
				return false;
			StatEnergyAvg = newStatEnergy;
			RMCraftCharacs.push_back( craftSlot );
			actualEnergies[rFaberElem] = craftSlot.ActualEnergy;
			sapLoads[rFaberElem] = craftSlot.Values[SapLoad];

			++nbFaberElemsFilled;
		}
	}
	// For mission RMs, set statenergy as class
	if ( nbFaberElemsFilled == 0 )
	{
		uint zoneLevel = (uint)(sheetName[RM_INDEX_LEVELZONE_CODE]-'a');
		if ( zoneLevel == 0 )
			StatEnergyAvg = 20;
		else
			StatEnergyAvg = 20 + (zoneLevel-1)*15;
	}

	// Compute rarity
	sint masterAvgFreq;
	if ( nbFaberElemsFilled == 0 )
	{
		// TODO: When the RM is not craftable, the rarity (=> price) depends on the level
		sint sumFreq = 0;
		for ( vu::iterator ivf=famInfo.Freqs.begin(); ivf!=famInfo.Freqs.end(); ++ivf )
			sumFreq += (sint)(*ivf);
		masterAvgFreq = sumFreq / (sint)famInfo.Freqs.size();
	}
	else
	{
		// TODO: depend not only on freq but on ILevelZone or craft characteristics
		masterAvgFreq = masterSumFreq / nbFaberElemsFilled;
	}
	/*switch ( masterAvgFreq )
	{
	case 1 : Rarity = (IEcosystem==PrimeRoots? 80 : 67); break; // only PrimeRoots and one other ecosystem
	case 2 : Rarity = 33; break; // Common
	case 3 : Rarity = 67; break; // only one ecosystem
	case 4 : Rarity = 1; break; // Goo
	default: Rarity = 100;
	}*/

	// Compute property depths
	IPropertyDepths.resize( famInfo.Properties.size() );
	for ( uint p=0; p!=famInfo.CraftPartsByProp.size(); ++p )
	{
		// Calculate a single property depth using the actual energies of faber elements that use the property
		uint32 nbFaberElemsThatUseTheProps = 0;
		float sumActualEnergy = 0.0f;
		for ( uint iCompatibleCP=famInfo.getBeginCraftPartForProp(p); iCompatibleCP!=famInfo.getEndCraftPartForProp(p); ++iCompatibleCP )
		{
			uint rFaberElem = famInfo.getCompatibleCraftPart( iCompatibleCP );
			sumActualEnergy += actualEnergies[rFaberElem];
			++nbFaberElemsThatUseTheProps;
		}
		float avgActualIEnergy = 0.0f;
		if ( nbFaberElemsThatUseTheProps != 0 )
			avgActualIEnergy = sumActualEnergy / (float)nbFaberElemsThatUseTheProps;
		if ( avgActualIEnergy < 0 )
			avgActualIEnergy = 0.0f;
		uint32 iPropertyDepth = (uint32)(avgActualIEnergy*((float)(NbPropertyDepths - 1))) + 1;
		if ( iPropertyDepth >= NbPropertyDepths )
			iPropertyDepth = NbPropertyDepths-1;
		//nldebug( "Depth %s %s-%u: %s (%.2f)", properties[iProperty].c_str(), locFam.c_str(), iLevelZone, PropertyDepths[iPropertyDepth], avgActualInterest );
		nlassert( iPropertyDepth != 0 );
		IPropertyDepths[p] = iPropertyDepth;
	}

	// Compute sap load estimation
	/*float sumSapLoad = 0.0f;
	uint32 nbSapLoads = 0;
	for ( uint32 rFaberElem=0; rFaberElem!=NbFaberElements; ++rFaberElem )
	{
		if ( hasCraftPart( rFaberElem ) )
		{
			if ( sapLoads[rFaberElem] != 0.0f )
			{
				sumSapLoad += sapLoads[rFaberElem];
				++nbSapLoads;
			}
		}
	}
	if ( nbSapLoads != 0 )
	{
		float avgSapLoad = sumSapLoad / nbSapLoads;
		if ( avgSapLoad < 0 )
			avgSapLoad = 0.0f;
		uint32 iSapLoadEst = (uint32)(avgSapLoad * 3.0f / MaxCharacValues[SapLoad]);
		if ( iSapLoadEst > 2 )
			iSapLoadEst = 3; // peak
		//nldebug( "Sap load est: %u", iSapLoadEst );
		SapLoadLevel = iSapLoadEst;
	}*/

	// Compute color
	TColor CommonColors [4] = { Beige, Green, Turquoise, Violet };
	TColor RareColors [2] = { Red, Blue };
	TColor PrimerootColors [2] = { White, Black };
	if ( famInfo.IsForMission )
	{
		Color = Beige;
	}
	else if ( famInfo.ColorIndex == -1 )
	{
		nlwarning( "No color provided for family %s", familyStr().c_str() );
		Color = InvalidColor;
	}
	else
	{
		uint colorIndex = (uint)famInfo.ColorIndex;
		if ( IEcosystem == PrimeRoots )
			Color = PrimerootColors[colorIndex % 2];
		else if ( StatQuality >= Choice )
			Color = RareColors[colorIndex % 2];
		else
			Color = CommonColors[colorIndex % 4];	
	}

	return true;
}


/*
 *
 */
void	CGenRawMaterial::collectStats( TRMItem& item, CMainStat& mainStats )
{
	// Name & title
	item.push( DtName, SheetName );
	item.push( DtTitle, Titles[SheetName] );

	// Properties
	for ( uint32 p=0; p!=/*min( NbPropertySlots,*/ (uint32)RMProperties.size()/*)*/; ++p )
	{
		if ( RMProperties[p] != UndefinedProperty )
		{
			++mainStats.NbRMHavingProperty[ILocation][IEcosystem][RMProperties[p]];
			item.push( DtProp, propertyStr( p ) );
		}
	}

	// Family
	item.push( DtRMFamily, familyStr() );

	// Group
	item.push( DtGroup, groups[Group] );

	// Ecosystem
	item.push( DtEcosystem, ecosystemStr() );

	TFamInfo& famInfo = FamSet[familyStr()];

	// LevelZone (for creatures)
	item.push( DtLevelZone, famInfo.IsForMission ?
		toString( "%c", 'A' + (char)ILevelZone ) :
		"-" );

	// StatQuality
	item.push( DtStatQuality, string( StatQualityStr[StatQuality] ) );

	// Craft stats
	vector<float> actualInterests( NbFaberElements, 0.0f );
	vector<float> sapLoads( NbFaberElements, 0.0f );
	for ( uint p=0; p!=famInfo.CraftPartsByProp.size(); ++p )
	{
		for ( uint iCompatibleCP=famInfo.getBeginCraftPartForProp(p); iCompatibleCP!=famInfo.getEndCraftPartForProp(p); ++iCompatibleCP )
		{
			uint rFaberElem = famInfo.getCompatibleCraftPart( iCompatibleCP );
			CFaberCharacteristics *craftSlot = getCraftSlot( rFaberElem );
			nlassert( craftSlot );

			TCiv civ = getCivSpec( IEcosystem, StatQuality ); ////getCivSpec( rFaberElem, FamSet[familyStr()] );
			CSString civS = CivNames[civ];

			item.push( DtCraftSlotName, getShortFaberElemString( rFaberElem ) );
			item.push( DtCraftCivSpec, civS );
			/*item.push( DtCraftSlotEnergy, toString( "%02u", (uint)(craftSlot->ActualEnergy*100.0f) ), true );
			item.push( DtCraftSlotOriginality, toString( "%03u", (uint)(craftSlot->ActualOriginality*100.0f) ), true );*/

			mainStats.updateCraftStatistics( rFaberElem, IEcosystem, IFamily, famInfo.Civs[p] );
		}
	}

	item.push( DtColor, colors[Color] );
	item.push( DtAverageEnergy, toString( "%d", StatEnergyAvg ) );

	// Rarity
	//item.push( DtRarity, toString( "%2u", Rarity ) );

	// Max Quality
	item.push( DtMaxLevel, toString( "%u", MaxLevel ) );

	// Protection
	CFaberCharacteristics *itemPart = getCraftSlot( ITEM_PART_JEWEL_GEM );
	if ( itemPart )
		item.push( DtJewelProtectionType, PROTECTION_TYPE::toString( (PROTECTION_TYPE::TProtectionType)(uint)itemPart->Values[JewelProtection] ) );

	// Customized properties
	CRulesStr2Filter::const_iterator icp = CustomizedPropertiesSheetName.find( SheetName + "." + rmSheetType );
	if ( icp != CustomizedPropertiesSheetName.end() )
	{
		CSString line;
		for ( vs::const_iterator ip=(*icp).second.begin(); ip!=(*icp).second.end(); ++ip )
		{
			if ( ip != (*icp).second.begin() )
				line += "; ";
			CSString propName = *ip;
			++ip;
			if ( ip == (*icp).second.end() )
				break;
			CSString value = *ip;
			line += propName + "=" + value;
		}
		item.push( DtCustomizedProperties, line );
	}

	if ( GraphFile )
	{
		fprintf( GraphFile, "%s;%s%u;%u;%u;%u;\n", SheetName.c_str(), ecosystems[IEcosystem].c_str(), StatQuality, StatEnergyAvg, (uint)(getOriginalityAvg()*100.0f), (uint)/*(getMainCivSpec( famInfo ))*/getCivSpec( IEcosystem, StatQuality )*4+100 );
	}

	mainStats.updateMainStats( RMCraftCharacs.size() );
}


/*
 *
 */
void	CGenRawMaterial::writeSheet( CForm *form )
{
	// Write properties (obsolete)
	/*if ( RMProperties.size() > NbPropertySlots )
		nlwarning( "More properties than property slots (%s)", familyStr().c_str() );
	for ( uint32 p=0; p!=min( NbPropertySlots, (uint32)RMProperties.size()); ++p )
	{
		if ( RMProperties[p] != UndefinedProperty )
		{
			form->getRootNode().setValueByName( propertyStr( p ).c_str(), toString( "mp.Material property %d", p+1 ).c_str() );
		}
	}*/

	// Write color
	/*vu& famColors = familyColorsFilter[iFam];
	if ( famColors.empty() )
		nlwarning( "No matching color for family %s", families[iFam].c_str() );
	else
	{
		uint32 iiColor = getRandomValue( famColors.size() );
		form->getRootNode().setValueByName( colors[famColors[iiColor]].c_str(), "mp.MpColor" );
	}*/
	if ( Color != InvalidColor )
		form->getRootNode().setValueByName( colors[Color].c_str(), "mp.MpColor" );

	// Write icon overlay (TODO)
	/*CSString& iconTextOver = Icons[locFam.splitTo('.')];
	if ( ! iconTextOver.empty() )
		form->getRootNode().setValueByName( iconTextOver.c_str(), "3d.icon text over" );*/
	
	/*CSString props;
	for ( uint32 p=0; p!=iProperties.size(); ++ p )
		props += " " + properties[iProperties[p]];
	nldebug( "%s %s %s", locFam.c_str(), props.c_str(), (familyGroups[iFam]==~0) ? "-" : groups[familyGroups[iFam]].c_str() );*/

	vector<float> actualInterests( NbFaberElements, 0.0f );
	vector<float> sapLoads( NbFaberElements, 0.0f );

	// Write craft stats
	list<CFaberCharacteristics>::const_iterator ics;
	for ( ics=RMCraftCharacs.begin(); ics!=RMCraftCharacs.end(); ++ics )
	{
		const CFaberCharacteristics& craftSlot = (*ics);
		const uint32& rFaberElem = craftSlot.FaberElement;

		for ( uint r=0; r!=NbCharacs; ++r )
		{
			switch ( r )
			{
			case JewelProtection: // see below
				break;
			case CraftCivSpec:
				{
					CSString civS;
					TCiv civ = getCivSpec( IEcosystem, StatQuality ); //getCivSpec( rFaberElem, FamSet[familyStr()] );
					if ( civ == AllCiv )
						civS = "common"; //"Common_Species";
					else
						civS = CivNames[civ];
					form->getRootNode().setValueByName( civS.toLower().c_str(), (CraftParts[rFaberElem].Path + ".CraftCivSpec").c_str() );
				}
				break;
			default:
				{
					//if ( craftSlot.Values[r] != 0.0f )
					if ( CharacSlotFilter[r][rFaberElem] )
						form->getRootNode().setValueByName( (sint32)(craftSlot.Values[r]), (CraftParts[rFaberElem].Path + "." + string(sCharacs[r])).c_str() );
				}
			}
		}

		// Write CraftEstimatedQuality (formerly known as property depths, I still use old property vector)
		/*for ( uint32 p=0; p!=IPropertyDepths.size(); ++p )
		{
			CSString itemPartAsPropS = propertyStr( p );
			if ( itemPartAsPropS.empty() )
				continue;
			uint itemPartAsProp = itemPartAsPropS[0] - 'A';
			if ( itemPartAsProp == rFaberElem )
			{
				form->getRootNode().setValueByName( propertyDepthStr( p ).c_str(), (CraftParts[rFaberElem].Path + ".CraftEstimatedQuality").c_str() );
				break;
			}
		}*/
	}

	// Jewel protection
	CFaberCharacteristics *itemPart = getCraftSlot( ITEM_PART_JEWEL_GEM );
	if ( itemPart )
		form->getRootNode().setValueByName( PROTECTION_TYPE::toString( (PROTECTION_TYPE::TProtectionType)(uint)itemPart->Values[JewelProtection] ).c_str(), (CraftParts[ITEM_PART_JEWEL_GEM].Path + "." + string(sCharacs[JewelProtection])).c_str() );

	// Write sap load estimation
	/*if ( SapLoadLevel != ~0 )
		form->getRootNode().setValueByName( (uint32)SapLoadLevel, "mp.Sap load est" );*/

	// Write rarity
	//form->getRootNode().setValueByName( (uint32)Rarity, "mp.Rarity" );

	// Write max quality
	form->getRootNode().setValueByName( (uint32)MaxLevel, "mp.MaxQuality" );

	// Write energy
	form->getRootNode().setValueByName( StatEnergyAvg, "mp.StatEnergy" );

	// Write customized properties
	setCustomizedPropertyValues( form->getRootNode(), SheetName + "." + rmSheetType );
}


/*
 *
 */
/*string	getPossibleAdjectivesFromSheetNameForDeposit( const CSString& sheetName )
{
	uint iFamily = getIndexFromString( sheetName.substr( RM_INDEX_FAMILY_CODE, NB_FAMILY_CODE_CHARS ), familyCodes );

	string locFam = packLocFamily( depositTypeLetters[iLoc], IFamily );
	//nlinfo( "%s", locFam.c_str() );
	RAdj = LocFamilyToAdjectives[locFam].find( adj.toUpper().c_str() );
}*/


/*
 *
 */
void	CGenRawMaterial::loadSheet( CForm *form, const std::string& sheetName, bool full )
{
	SheetName = sheetName;
	string value;

	if ( full )
	{
		// Location
		uint32 iLoc;
		if ( SheetName[0] == 'c' )
			iLoc = InCreatures;
		else
		{
			/*switch ( SheetName[2] )
			{
			case 'u': iLoc = Under; break;
			case 'o': iLoc = Over; break;
			case 'f': iLoc = Flora; break;
			default: nlstop;
			}*/
			iLoc = InDeposits;
		}
		ILocation = iLoc;

		// Ecosystem
		form->getRootNode().getValueByName( value, "mp.Ecosystem" );
		if ( ! value.empty() )
			IEcosystem = (TEcosystem)getIndexFromString( SheetName.substr( RM_INDEX_ECOSYSTEM_CODE, NB_ECOSYSTEM_CODE_CHARS ), ecosystemCodes ); // not using value because != label

		// Family
		form->getRootNode().getValueByName( value, "mp.Family" );
		if ( ! value.empty() )
			IFamily = getIndexFromString( SheetName.substr( RM_INDEX_FAMILY_CODE, NB_FAMILY_CODE_CHARS ), familyCodes ); // not using value because != label

		fillPropertiesFromFamily();

		// StatQuality
		setStatQuality( SheetName[RM_INDEX_LEVELZONE_CODE] );
	}
	
	// Color
	uint32 v;
	form->getRootNode().getValueByName( v, "mp.MpColor" );
	Color = (TColor)v;

	// Craft stats
	uint32 nbFaberElemsFilled = 0;
	for ( uint32 rFaberElem=0; rFaberElem!=NbFaberElements; ++rFaberElem )
	{
		sint32 val;
		form->getRootNode().getValueByName( val, (CraftParts[rFaberElem].Path + ".Durability").c_str() );
		if ( val != 0 )
		{
			CFaberCharacteristics craftSlot;
			craftSlot.initFaberElement( rFaberElem );

			for ( uint r=0; r!=NbCharacs; ++r )
			{
				switch ( r )
				{
				case JewelProtection:
					{
					if ( CharacSlotFilter[r][rFaberElem] )
					{
						string proTypeS;
						form->getRootNode().getValueByName( proTypeS, (CraftParts[rFaberElem].Path + "." + string(sCharacs[r])).c_str() );
						craftSlot.Values[r] = (float)(uint)PROTECTION_TYPE::fromString( proTypeS );
					}
					else
						craftSlot.Values[r] = 0.0f;
					}
					break;
				case CraftCivSpec:
					{
					if ( CharacSlotFilter[r][rFaberElem] )
					{
						string civS;
						form->getRootNode().getValueByName( civS, (CraftParts[rFaberElem].Path + "." + string(sCharacs[r])).c_str() );
						TCiv civ = AllCiv;
						if ( civS != "Common_Species" )
							for ( uint c=0; c!=NbCiv; ++c )
								if ( string(CivNames[c]) == civS )
									civ = (TCiv)c;
						craftSlot.Values[r] = (float)(uint)civ;
					}
					}
					break;
				default:
					{
						form->getRootNode().getValueByName( val, (CraftParts[rFaberElem].Path + "." + string(sCharacs[r])).c_str() );
						craftSlot.Values[r] = (float)val;
					}
				}
			}
			
			craftSlot.calcQualitativeValues();
			
			RMCraftCharacs.push_back( craftSlot );

			//nlinfo( "%s %s %s %s", item.Fields[DtName][0].c_str(), faberElems[rFaberElem].c_str(), families[iFam].c_str(), familyGroups[iFam]==~0?"":groups[familyGroups[iFam]].c_str() );
		}
		else
		{
			if ( CheckDurability )
			{
				// Check if Durability==0 is not a mistake
				for ( uint r=0; r!=NbCharacs; ++r )
				{
					if ( r != JewelProtection && r!=CraftCivSpec )
					{
						form->getRootNode().getValueByName( val, (CraftParts[rFaberElem].Path + "." + string(sCharacs[r])).c_str() );
						if ( val != 0 )
						{
							nlwarning( "%s has a null durability for item part %c", sheetName.c_str(), (char)('A' + (char)rFaberElem) );
							break;
						}
					}
				}
			}
		}
	}

	/*// These properties are currently useless when reading (not used for doc)
	// Write property depths
	uint p = 0;
	bool res = true;
	while ( res )
	{
		string valueStr;
		res = form->getRootNode().getValueByName( valueStr, toString( "mp.Prop %u depth", p+1 ).c_str() );
		if ( res )
			IPropertyDepths.push_back( propertyDepthFromString( valueStr ) );
		++p;
	}

	// Sap load est
	form->getRootNode().getValueByName( SapLoadLevel, "mp.Sap load est" );
	*/

	// Rarity
	//form->getRootNode().getValueByName( Rarity, "mp.Rarity" );

	// Max Quality
	form->getRootNode().getValueByName( MaxLevel, "mp.MaxQuality" );

	// Energy
	form->getRootNode().getValueByName( StatEnergyAvg, "mp.StatEnergy" );

	++nbSheetsProcessed;
}


/*
 *
 */
void	writeRMSheetToDisk( CForm *form, const string& sheetName )
{
	if ( ! EraseOldCreatureAssignments )
	{
		flushSheetToDisk( rawMaterialPath + dirbase + sheetName + "." + rmSheetType, form );
		++nbSheetsProcessed;
		if ( inputSheetPathContent.find( sheetName ) == inputSheetPathContent.end() )
			++nbNewSheetsGenerated;
	}
	if ( GetSelectionUntilLevel != ~0 )
	{
		if ( (sheetName[5] - 'a') <= (sint)GetSelectionUntilLevel )
			InfoLog->displayRawNL( "SELECTION: %s", sheetName.c_str() );
	}
}


/*
 * Write sheet, write stat
 */
void	selectRawMaterial( CForm *form, CForm *ecoParentForm, CForm *famParentForm, CGenRawMaterial *rm )
{
	// Clear sheet (otherwise the previous values would remain)
	clearSheet( form, &form->getRootNode() );

	// Link to the parent sheets (the form arguments are always the last parent forms but there aren't used by the func anyway)
	form->insertParent( 0, getEcoParentName( rm->IEcosystem ).c_str(), ecoParentForm );
	form->insertParent( 1, getFamParentName( rm->IFamily ).c_str(), famParentForm );

	// Write stats
	TRMItem item;
	rm->collectStats( item, MainStat );
	SortableData.addItem( item );

	// Write to disk
	rm->writeSheet( form );
	writeRMSheetToDisk( form, rm->SheetName );
}


/*
 *
 */
/*string	findSubstitutionForDepositRM( const string& srcName, char adjCode )
{
	//nldebug( "SRC: %s", srcName.c_str() );
	string substitution = srcName;
	CRMData::CLookup::const_iterator ilk;
	char decl;
	for ( decl='a'; decl!='h'; ++decl ) // up to 7 declinaisons
	{
		substitution[3] = decl;
		substitution[2] = srcName[2];
		substitution[8] = adjCode;
		//nldebug( "TRYING %s", substitution.c_str() );
		ilk = SortableData.lookup( DtName ).find( substitution );
		if ( ilk != SortableData.lookup( DtName ).end() )
			return substitution;

		uint iLoc;
		for ( iLoc=0; iLoc!=NB_DEPOSITS; ++iLoc )
		{
			char locChar = tolower( depositTypeLetters[iLoc][0] );
			if ( locChar == srcName[2] )
				continue;

			substitution[2] = locChar;
			//nldebug( "TRYING %s", substitution.c_str() );
			ilk = SortableData.lookup( DtName ).find( substitution );
			if ( ilk != SortableData.lookup( DtName ).end() )
				return substitution;
		}
	}
	return string();
}*/


/*
 * Result has at least one character
 */
inline CSString getCreatureEcosystemCode( uint itEcosystem )
{
	switch ( itEcosystem )
	{
	case Goo:		return "g";
	case Invasion:	return "i";
	case Raid:		return "r";
	case Event:		return "e";
	case N:			return "n";
	case S:			return "s";
	case T:			return "t";
	case U:			return "u";
	case V:			return "v";
	case W:			return "w";
	case X:			return "x";
	case Y:			return "y";
	case Z:			return "z";
	default:		return ecosystemCodes[itEcosystem];
	}
}


/*
 *
 */
inline CSString getCreatureLZCode( const CSString& creaModel, uint itEcosystem, uint iZone )
{
	nlassert( iZone < 10 );
	return string("c") + creaModel.toLower() + getCreatureEcosystemCode( itEcosystem ) + toString( "%c", (char)('a' + (char)iZone) );
}


/*
 *
 */
void loadConfigFile()
{
	// Load config file
	try
	{
		CConfigFile configFile;
		configFile.load( "raw_material_generation.cfg" );
		CConfigFile::CVar *var = configFile.getVarPtr( "RawMaterialPath" );
		if ( var ) rawMaterialPath = var->asString();
		var = configFile.getVarPtr( "CreatureSubPath" );
		if ( var )
		{
			creaturePath = var->asString();
			if ( creaturePath[creaturePath.size()-1] != '/' )
				creaturePath += "/";
		}
		var = configFile.getVarPtr( "CreatureAssignmentPath" );
		if ( var )
		{
			creatureAssignmentPath = var->asString();
			if ( creatureAssignmentPath[creatureAssignmentPath.size()-1] != '/' )
				creatureAssignmentPath += "/";
		}
		var = configFile.getVarPtr( "DepositSubPath" );
		if ( var )
		{
			depositPath = var->asString();
			if ( depositPath[depositPath.size()-1] != '/' )
				depositPath += "/";
		}

		loadConfigFlag( configFile, "WriteSheetsToDisk", WriteSheetsToDisk );
		loadConfigFlag( configFile, "GenOnlyNewRawMaterials", GenOnlyNewRawMaterials );
		loadConfigFlag( configFile, "SkipRawMaterialsForCreatures", SkipRawMaterialsForCreatures );
		loadConfigFlag( configFile, "AssignRawMaterialsHarvestToCreatureSheets", AssignRawMaterialsHarvestToCreatureSheets );
		loadConfigFlag( configFile, "AssignOnlyToUnassignedCreatures", AssignOnlyToUnassignedCreatures );
		loadConfigFlag( configFile, "SkipRawMaterialsForDeposits", SkipRawMaterialsForDeposits );
		loadConfigFlag( configFile, "GenOnlyMissionRawMaterials", GenOnlyMissionRawMaterials );
		loadConfigFlag( configFile, "AssignRawMaterialsHarvestToDepositSheets", AssignRawMaterialsHarvestToDepositSheets );
		loadConfigFlag( configFile, "BrowseOtherDeposits", BrowseOtherDeposits );
		loadConfigFlag( configFile, "FixDeposits", FixDeposits );
		loadConfigFlag( configFile, "EraseOldCreatureAssignments", EraseOldCreatureAssignments );
		loadConfigFlag( configFile, "TestExistingAssigments", TestExistingAssigments );
		loadConfigFlag( configFile, "OutputNameList", OutputNameList );
		loadConfigFlag( configFile, "DisplayFamAndProp", DisplayFamAndProp );
		loadConfigFlag( configFile, "ProduceDoc", ProduceDoc );
		loadConfigFlag( configFile, "CheckDurability", CheckDurability );
		loadConfigFlag( configFile, "GenerateDepositSystemMpFiles", GenerateDepositSystemMpFiles );

		var = configFile.getVarPtr( "ExistingRMAction" );
		if ( var ) ExistingRMAction = (TExistingRMAction)var->asInt();

		var = configFile.getVarPtr( "GetSelectionUntilLevel" );
		if ( var ) GetSelectionUntilLevel = (uint32)var->asInt();

		var = configFile.getVarPtr( "SortByOriginality" );
		if ( var ) SortByOriginality = ((uint32)var->asInt() == 1);

		var = configFile.getVarPtr( "MaxNbRMByFamilyEZ" );
		if ( var ) MaxNbRMByFamilyEZ = (uint32)var->asInt();

		var = configFile.getVarPtr( "MaxNbRMByCraftSlotE" );
		if ( var ) MaxNbRMByCraftSlotE = (uint32)var->asInt();
		
		//var = configFile.getVarPtr( "OriginalityMinThreshold" );
		//if ( var ) OriginalityMinThreshold = (uint32)var->asInt();
		
		/*var = configFile.getVarPtr( "NbFaberCombinations" );
		if ( var ) NbFaberCombinations = var->asInt();*/

		var = configFile.getVarPtr( "TranslationPath" );
		if ( var ) TranslationPath = var->asString();

		var = configFile.getVarPtr( "SystemMPPath" );
		if ( var ) SystemMPPath = var->asString();

		var = configFile.getVarPtr( "CustomizedProperties" );
		if ( var )
		{
			for ( sint i=0; i!= var->size(); ++i )
			{
				CSString sheetFilename = var->asString( i );
				++i;
				if ( i < var->size() )
				{
					CSString propName = var->asString( i );
					++i;
					if ( i < var->size() )
					{
						CSString value = var->asString( i );
						CustomizedPropertiesSheetName[sheetFilename].push_back( propName );
						CustomizedPropertiesSheetName[sheetFilename].push_back( value );
					}
				}
			}
		}
	}
	catch ( EConfigFile& e )
	{
		nlwarning( "%s - Press a key", e.what() );
		getch();
	}
}


/*
 *
 */
void displayFamNames()
{
	ListLog.displayRawNL( "Family names for en.uxt:" );
	for ( uint i=0; i!=families.size(); ++i )
	{
		if ( ! families[i].empty() )
		{
			uint num = atoi( familyCodes[i].c_str() );
			ListLog.displayRawNL( "mpfam%u\t\t\t[%s]", num, families[i].c_str() );
		}
	}
	ListLog.displayRawNL( "Groups names for en.uxt:" );
	for ( uint i=0; i!=groups.size(); ++i )
	{
		if ( ! groups[i].empty() )
			ListLog.displayRawNL( "mpgroup%u\t\t\t[%s]", i, groups[i].c_str() );
	}
	ListLog.displayRawNL( "Groups by family:" );
	for ( uint i=0; i!=families.size(); ++i )
	{
		if ( ! families[i].empty() )
		{
			TFamInfo& famInfo = FamSet[families[i]];
			ListLog.displayRawNL( "%u\t%s", i, (famInfo.Group!=~0) ? groups[famInfo.Group].c_str() : "" );
		}
	}
	ListLog.displayRawNL( "Families by group:" );
	for ( uint i=0; i!=groups.size(); ++i )
	{
		if ( ! groups[i].empty() )
		{
			ListLog.displayRawNL( "Group:\t\t\t%s", groups[i].c_str() );
			for ( uint j=0; j!=families.size(); ++j )
			{
				if ( FamSet[families[j]].Group == i )
					ListLog.displayRawNL( "Group:\t%u\t%u\t\t%s", i, j, families[j].c_str() );
			}
		}
	}
	ListLog.displayRawNL( "Group icons for _ic_groups.forage_source" );
	ListLog.displayRawNL( "	    <ARRAY Name=\"Icons\">" );
	for ( uint i=0; i!=groups.size(); ++i )
	{
		//if ( ! groups[i].empty() )
			ListLog.displayRawNL( "      <ATOM Value=\"%s\"/>", Icons[groups[i]].Icon.c_str() );
	}
	ListLog.displayRawNL( "    </ARRAY>");
	ListLog.displayRawNL( "Family icons for _ic_families.forage_source" );
	ListLog.displayRawNL( "	    <ARRAY Name=\"Icons\">" );
	for ( uint i=0; i!=families.size(); ++i )
	{
		//if ( ! families[i].empty() )
			ListLog.displayRawNL( "      <ATOM Value=\"%s\"/>", Icons[families[i]].Icon.c_str() );
	}
	ListLog.displayRawNL( "    </ARRAY>");
}


/*
 * For deposit families only
 */
void generateSytemMpFiles()
{
	if ( ! GenerateDepositSystemMpFiles )
		return;

	string dir = SystemMPPath;
	nlinfo( "Processing 'system mp' files for WorldEditor in %s...", dir.c_str() );

	// Build the right list of files
	vector<string> systemMpFilenames;
	systemMpFilenames.resize( families.size() );
	for ( uint i=0; i!=families.size(); ++i )
	{
		if ( ! families[i].empty() )
		{
			if ( FamSet[families[i]].IsInDeposits )
				systemMpFilenames[i] = conventionalDirectory( families[i] ) + toString( "_%u.mp", i );
		}
	}

	// List and delete obsolete files
	vector<string> currentFiles;
	CPath::getPathContent( dir, false, false, true, currentFiles );
	for ( uint i=0; i!=currentFiles.size(); ++i )
	{
		string currentFile = CFile::getFilename( currentFiles[i] );
		if ( currentFile.find( ".mp" ) != string::npos )
		{
			if ( find( systemMpFilenames.begin(), systemMpFilenames.end(), currentFile ) == systemMpFilenames.end() )
			{
				nlinfo( "Deleting obsolete %s (PLEASE COMMIT)", currentFile.c_str() );
				if ( WriteSheetsToDisk )
					CFile::deleteFile( currentFiles[i] );
			}
		}
	}

	// Create new files
	for ( uint i=0; i!=families.size(); ++i )
	{
		if ( (! families[i].empty()) && (! systemMpFilenames[i].empty()) )
		{
			// Check if file present ; if not, create it
			if ( ! CFile::fileExists( dir + systemMpFilenames[i] ) )
			{
				nlinfo( "Creating %s (PLEASE COMMIT)", systemMpFilenames[i].c_str() );
				if ( WriteSheetsToDisk )
				{
					COFile f;
					f.open( dir + systemMpFilenames[i], false, true );
					f.close();
				}
			}
		}
	}
}


/*
 * generateRawMaterials
 */
void generateRawMaterials()
{
	// Load config file
	loadConfigFile();
	
	// Load lists from DFNs
	loadSheetPath();
	UFormLoader *formLoader = UFormLoader::createLoader ();
	loadDFNs( formLoader );

	// TEMP: remove all raw materials for creatures!
	/*string dp = "r:/code/ryzom/data_leveldesign/leveldesign/game_element/sitem/raw_material/";
	string ecos [5] = { "desert", "forest", "jungle", "lacustre", "prime_roots" };
	vector<string> thefiles [5];
	for ( uint i=0; i!=5; ++i )
	{
		CPath::getPathContent( dp + ecos[i], false, false, true, thefiles[i] );
		nlinfo( "%u files in %s", thefiles[i].size(), (dp + ecos[i]).c_str() );
	}
	map<string,string>::iterator it;
	for ( it=inputSheetPathContent.begin(); it!=inputSheetPathContent.end(); ++it )
	{
		const string& s1 = (*it).first, s2 = (*it).second;
		string::size_type p;
		if ( (p = s2.find( "raw_material/_parent/_mc" )) != string::npos )
		{
			string cc = s1.substr( 3, 2 );
			for ( uint i=0; i!=5; ++i )
			{
				for ( vector<string>::iterator its=thefiles[i].begin(); its!=thefiles[i].end(); ++its )
				{
					string& fl = (*its);
					string::size_type q;
					if ( (q = fl.find( string( "c") + cc )) != string::npos )
					{
						if ( (fl[q-1] >= '0') && (fl[q-1] <= '9') )
						{
							nlinfo( "Deleting %s", fl.c_str() );
							CFile::deleteFile( fl );
						}
					}
				}
			}
		}
	}

	return;*/

	// Load families from CSV file
	loadFamAndProp( "rm_fam_prop.csv", DisplayFamAndProp );
	if ( TFamInfo::UseGenerateOnly != 0 )
		nlinfo( "Requested to generate only %u marked RM families", TFamInfo::UseGenerateOnly );
	else
		nlinfo( "Requested to generate all RM families" );

	// Load template sheet for final and parent forms
	NLMISC::CSmartPtr<CForm> form = loadTemplateForm( formLoader, rmSheetType );
	NLMISC::CSmartPtr<CForm> ecoParentForm = loadTemplateForm( formLoader, rmSheetType );
	NLMISC::CSmartPtr<CForm> famParentForm = loadTemplateForm( formLoader, rmSheetType );

	for ( uint r=0; r!=NbCharacs; ++r )
		CharacSlotFilter[r].resize( NbFaberElements );

	// Load other CSV files
	loadCSVFile( "rm_item_parts.csv", deliverItemPartParams );
	loadCSVFile( "creature_models.csv", deliverCreatureModels );
	//nlinfo( "Loaded %u creature categories", SkgroupToModels.size() );
	dispatchFamiliesToLocations();
	nlinfo( "%u creatures models selected", RMFamilyIndicesByCreatureModel.size() );
	nlinfo( "%u creature models used", checkSkeletons() );
	nlinfo( "%u icon lines loaded", Icons.size() );

	// Summary
	nlinfo( "%u families, %u ecosystems, %u properties, %u craft parts, %u colors", families.size(), ecosystems.size(), properties.size(), CraftParts.CraftParts.size(), colors.size() );

	// Generate names(with nomenclature)
	ListLog.addDisplayer( &ListDisplayer );
	familyCodes.resize( families.size() );
	ecosystemCodes.resize( ecosystems.size() );
	for_each( familyCodes.begin(), familyCodes.end(), normalizeFamilyCode );
	displayFamNames();
	generateSytemMpFiles();
	//buildNomenclatureCodes( "New families:", families, familyCodes, NB_FAMILY_CODE_CHARS ); // useless currently
	buildNomenclatureCodes( NULL, ecosystems, ecosystemCodes, NB_ECOSYSTEM_CODE_CHARS );

	// Load Unicode titles (can share the same hash map because codes are distinct)
	loadTitles( "item", "work", "wk", Titles );
	loadTitles( "creature", "translated", "en", Titles );

	createDirectoryStructure();
	MainStat.init();
	FILE *nameOutputFile;
	if ( OutputNameList )
		nameOutputFile = fopen( "rm_names_output.csv", "wt" );
	GraphFile = fopen( "rm_output_graph.csv", "wt" );
	fprintf( GraphFile, "Graph type: Line with markers displayed at each data value, columns B to E. For deposits, sort by column B\n" );
	fprintf( GraphFile, "Ecosystem specialization: 100=Desert, 104=Forest, 108=Lake, 112=Jungle, 116=All\n" );
	fprintf( GraphFile, "RM code;Zone;Energy;Originality;Eco. spec.;\n\n" );
	SortableData.init( ProduceDoc );

	// Generate random values
	//RawMaterialRepository.resize( ecosystems.size(), families.size() );
	RandomGenerator.srand( (unsigned)time( NULL ) );

	/*
	 * New raw materials
	 */
	if ( GenOnlyNewRawMaterials || AssignRawMaterialsHarvestToCreatureSheets || AssignRawMaterialsHarvestToDepositSheets || FixDeposits )
	{
		// Parents
		if ( ExistingRMAction == ModifyRM )
		{
			for ( uint32 iEcosystem=0; iEcosystem!=ecosystems.size(); ++iEcosystem )
			{
				// Parent item for ecosystem
				clearSheet( ecoParentForm, &form->getRootNode() );
				writeParentSheetEco( ecoParentForm, iEcosystem );
			}

			for ( uint32 iFamily=0; iFamily!=families.size(); ++iFamily )
			{
				if ( TFamInfo::mustGenerateFamily( iFamily ) )
				{
					// Parent item for family
					clearSheet( famParentForm, &form->getRootNode() );
					writeParentSheetFam( famParentForm, iFamily );
				}
			}
		}

		uint32 nbAssignedMaterials = 0, nbCreatureSheetsSkipped = 0, nbErasedCreatureAssignments = 0;
		uint32 maxLastAssignedRMSlot = 0;
		NLMISC::CSmartPtr<CForm> creaParentForm;
		NLMISC::CSmartPtr<UForm> creaForm;
		set<CSString, CUnsensitiveSStringLessPred> creatureSheets;

		if ( SkipRawMaterialsForCreatures )
			goto processRawMaterialsForDeposits;

		//
		// Iterate on creatures raw materials
		//
		for ( CRulesStrFilter::iterator icr=RMFamilyIndicesByCreatureModel.begin(); icr!=RMFamilyIndicesByCreatureModel.end(); ++icr )
		{
			// HD
			const CSString& creaModel = (*icr).first;
			vu& familiesForCreatureModel = (*icr).second;

			// Parent item for creature (one per model)
			CSString creaCMainModelCode = string("c") + creaModel.toLower(); // CHD
			static float progress = 0.0f;
			if ( familiesForCreatureModel.empty() )
			{
				nlinfo( "No RM has been defined for non-goo %s (%s)", creaCMainModelCode.c_str(), CreatureModels[creaModel].Name.c_str() );
				CreatureMainModelsWithoutRM.insert( creaCMainModelCode );
			}
			else
				nlinfo( "%u RM for each variant of non-goo %s (%.1f%%)", familiesForCreatureModel.size(), creaModel.c_str(), 100.0f*progress/(float)RMFamilyIndicesByCreatureModel.size() );
			++progress;

			// Write parent item sheet
			if ( ExistingRMAction == ModifyRM )
			{
				creaParentForm = loadTemplateForm( formLoader, rmSheetType );
				clearSheet( creaParentForm, &form->getRootNode() );
				writeParentSheetCreature( creaParentForm, creaModel );
			}

			// Iterate on ecosystems
			for ( uint itEcosystemOfCreature=CommonEcosystem+1; itEcosystemOfCreature!=NbEcosystemsPlusExtensions; ++itEcosystemOfCreature )
			{
				// CHDF

				for ( uint iZone=0; iZone!=10; ++iZone )
				{
					uint iLevelZone = (iZone<=1) ? 0 : iZone-1;

					// CHDFA-CHDFJ
					CSString creaLZCode = getCreatureLZCode( creaModel, itEcosystemOfCreature, iZone );

					for ( uint32 iLocalLevel=1; iLocalLevel!=MAX_NB_LOCAL_LEVELS+1; ++iLocalLevel ) // warning: base 1
					{
						// CHDFA1-CHDFA9 (only if creature sheet exists on disk)
						TStatQuality statQuality = CreatureLocalLevelToStatQuality[iLocalLevel-1];

						CSString creaSheetName = creaLZCode + toString("%u", iLocalLevel);
						map<string,string>::const_iterator ipc = inputSheetPathContent.find( creaSheetName );
						if ( ipc == inputSheetPathContent.end() )
							continue;

						if ( ((itEcosystemOfCreature != Goo) && familiesForCreatureModel.empty()) ||
							 (statQuality == InvalidStatQuality) ) // mission creatures (*6.creature) have invalid StatQuality => no RMs (bugged: no parent created)
						{
							// Generate empty parent creature sheets if not existing yet
							const CSString& creaPathAndFilename = creatureAssignmentPath + string("_") + creaSheetName + "_mp." + crSheetType;
							map<string,string>::const_iterator ipcp = inputSheetPathContent.find( creaSheetName + "_mp" /*+"." + crSheetType*/ );
							if ( ipc == inputSheetPathContent.end() )
							{
								UForm *creaForm = formLoader->loadForm( (string("_empty.") + crSheetType).c_str() );
								flushSheetToDisk( creaPathAndFilename, creaForm );
							}
						}
						else
						{
							// Get the list of RM families for the current processed creature
							vu familiesForThisCreature;
							uint nbInvasionFamilies = 0;
							switch ( itEcosystemOfCreature )
							{
							case Goo:
								familiesForThisCreature = GooCreatureFamilyIndices;
								break;
							case Invasion:
							case Raid:
							case Event:
							case N:
							case S:
							case T:
							case U:
							case V:
							case W:
							case X:
							case Y:
							case Z:
							{
								// Special RMs for all invasion/raid creatures
								vu& familiesForAllIRCreatures = InvasionRaidCreatureFamilyIndices['*'];
								familiesForThisCreature.insert( familiesForThisCreature.end(), familiesForAllIRCreatures.begin(), familiesForAllIRCreatures.end() );
								nbInvasionFamilies += familiesForAllIRCreatures.size();

								// Special RMs for this type of invasion/raid creature
								char cd [2];
								cd[0] = creaLZCode[1];
								cd[1] = '\0';
								::strlwr( cd );
								vu& familiesForThisTypeOfIRCreature = InvasionRaidCreatureFamilyIndices[cd[0]];
								familiesForThisCreature.insert( familiesForThisCreature.end(), familiesForThisTypeOfIRCreature.begin(), familiesForThisTypeOfIRCreature.end() );
								nbInvasionFamilies += familiesForThisTypeOfIRCreature.size();

								// Normal craft RMs if sufficient level or event
								if ( (iLocalLevel >= 5) ||
									 ((itEcosystemOfCreature >= Event) && (itEcosystemOfCreature <= Z)) )
								{
									familiesForThisCreature.insert( familiesForThisCreature.end(), familiesForCreatureModel.begin(), familiesForCreatureModel.end() );
								}
								break;
							}
							default:
								familiesForThisCreature = familiesForCreatureModel;
							}

							// Iterate on corresponding families
							for ( vu::iterator ifc=familiesForThisCreature.begin(); ifc!=familiesForThisCreature.end(); ++ifc )
							{
								uint32 iFam = (*ifc);
								if ( ! TFamInfo::mustGenerateFamily( iFam ) )
									continue;

								//nldebug( "%s - %u adj: %s", families[iFam].c_str(), adjsForFamily.size(), adjsForFamily.c_str() );
								// For creature, the ecosystems are already limited by which creature sheets exist

								bool isMissionRawMaterial = FamSet[families[iFam]].IsForMission;
								if ( GenOnlyMissionRawMaterials && (! isMissionRawMaterial) )
									continue;

								bool isInvasionFamily = ((uint)(ifc-familiesForThisCreature.begin()) < nbInvasionFamilies);
								CSString rmCodeMid;
								uint iEcosystem = ~0;
								switch ( itEcosystemOfCreature )
								{
								case Goo:
									rmCodeMid = toString( "cxxc%c", (char)('a' + (char)iZone) );
									iEcosystem = CommonEcosystem;
									break;
								case Invasion:
								case Raid:
								case Event:
								case N:
								case S:
								case T:
								case U:
								case V:
								case W:
								case X:
								case Y:
								case Z:
									if ( isInvasionFamily || // the special RMs for invasion creature (possibly with several variants)
										 ((iLocalLevel <= 4) && ((itEcosystemOfCreature == Invasion) || (itEcosystemOfCreature == Raid))) ) // invasion/raid creatures have normal RMs only for bosses (all event creatures have normal RMs)
									{
										// Special invasion RM
										rmCodeMid = toString( "ixxcc" );
										iEcosystem = CommonEcosystem;
									}
									else
									{
										// No mission RM for named/bosses/mini-bosses
										if ( isMissionRawMaterial )
											continue;
										
										// Ecosystem is always Common, because the ecosystem of the creature is Invasion or Raid
										iEcosystem = CommonEcosystem;
										CSString creaLZCodeCommon = getCreatureLZCode( creaModel, iEcosystem, iZone );
										rmCodeMid = creaLZCodeCommon.substr( 0, creaLZCode.size() - 1 ) + toString( "%c", (char)((char)'a'+(char)statQuality) );
									}
									break;
								default:
									if ( isMissionRawMaterial )
									{
										// No mission RM for named/bosses/mini-bosses
										if ( iLocalLevel > 4 )
											continue;

										// Ecosystem is always specialized
										iEcosystem = itEcosystemOfCreature;

										// Mission RMs range from quality a (plain, in newbieland only) to f (magnificient)
										rmCodeMid = creaLZCode;
									}
									else
									{
										// Ecosystem-specialized from Choice quality
										if ( (statQuality <= Fine) )
											iEcosystem = CommonEcosystem;
										else
											iEcosystem = itEcosystemOfCreature;

										// Now: for craft RMs, statquality is bound on local level instead of level zone
										CSString creaLZCodeCommon = getCreatureLZCode( creaModel, iEcosystem, iZone );
										rmCodeMid = creaLZCodeCommon.substr( 0, creaLZCode.size() - 1 ) + toString( "%c", (char)((char)'a'+(char)statQuality) );
									}
								}
								dirbase = conventionalDirectory( ecosystems[iEcosystem] ) + "/";

								// Iterate on variants (now: only one variant)
								uint32 nbVariants = 1;
								if ( (itEcosystemOfCreature == Invasion) || (itEcosystemOfCreature == Raid) )
								{
									if ( (iLocalLevel == 6) || (iLocalLevel == 8) )
										nbVariants = 0;
									else if ( iLocalLevel <= 4 )
										nbVariants = 1;
									// for 5 & 7, two variants only for the boss kitin invasion RM (trophy)
									else if ( isInvasionFamily && (creaLZCode[1] == 'k') )
										nbVariants = 2;
								}
								for ( uint32 iVariant=1; iVariant<=nbVariants; ++iVariant )
								{
									CSString sheetName = CSString( "m" )
										+ familyCodes[iFam]
										+ rmCodeMid
										+ toString( "%02u", iVariant );

									if ( GenOnlyNewRawMaterials )
									{
										map<string,string>::const_iterator iprm = inputSheetPathContent.find( sheetName );
										if ( iprm != inputSheetPathContent.end() )
											continue;
									}

									if ( (! IsRMSheetGenerated[sheetName].Done) )
									{
										IsRMSheetGenerated[sheetName].Done = true;
										
										//DebugLog->displayRawNL( "%s %s", sheetName.c_str(), getRMName( sheetName ).c_str() );
										if ( OutputNameList )
										{
											CSString fullName;
											if ( isInvasionFamily && (creaLZCode[1] == 'k') )
											{
												switch ( iVariant )
												{
												case 1: fullName = "Fragment of Kitin Claw;a;the;Fragments of Kitin Claw;;the;;"; break;
												case 2: fullName = "Kitin Trophy;a;the;Kitin Trophies;;the;;"; break;
												default:;
												}
											}
											else
											{
												fullName = getRMShortName( sheetName, statQuality, isMissionRawMaterial );
											}
											fprintf( nameOutputFile, "%s;%s\n", sheetName.c_str(), fullName.c_str() );
										}

										CGenRawMaterial rawMaterial( sheetName );

										rawMaterial.ILocation = InCreatures;
										rawMaterial.IFamily = iFam;
										rawMaterial.Group = FamSet[families[iFam]].Group;
										rawMaterial.IEcosystem = (TEcosystem)iEcosystem;
										rawMaterial.StatQuality = statQuality;
										if ( isMissionRawMaterial )
											rawMaterial.ILevelZone = iZone;
										else
											rawMaterial.ILevelZone = iLevelZone;
										rawMaterial.fillPropertiesFromFamily();

										if ( ExistingRMAction == ModifyRM )
										{
											// Clear sheet (otherwise the previous values would remain)
											clearSheet( form, &form->getRootNode() );

											// Link to the parent sheets (the form arguments are always the last parent forms but there aren't used by the func anyway)
											form->insertParent( 0, getEcoParentName( iEcosystem ).c_str(), ecoParentForm );
											form->insertParent( 1, getFamParentName( iFam ).c_str(), famParentForm );
											if ( ! ((itEcosystemOfCreature == Goo) ||
												    (isInvasionFamily)) )
												form->insertParent( 2, getCreaParentName( creaCMainModelCode ).c_str(), creaParentForm );

											rawMaterial.MaxLevel = 250;
											if ( ! rawMaterial.computeCraftCharacs( iVariant, sheetName ) )
												continue;

											// Write to disk
											rawMaterial.writeSheet( form );
											writeRMSheetToDisk( form, sheetName );
										}
										else
										{
											form = (CForm*)formLoader->loadForm( (rawMaterialPath + dirbase + sheetName + "." + rmSheetType).c_str() );
											if ( ! form )
											{
												nlwarning( "Can't load %s", (rawMaterialPath + dirbase + sheetName + "." + rmSheetType).c_str() );
												continue;
											}
												
											rawMaterial.loadSheet( form, sheetName, false );
										}

										TRMItem item;
										rawMaterial.collectStats( item, MainStat );
										SortableData.addItem( item );
										//Repository.push_back( rawMaterial );
									}

									// Assignment to existing creature sheets (TODO: for Common ecosystem, search Goo creature as well)
									if ( AssignRawMaterialsHarvestToCreatureSheets ) // TEMP
									{
										// For RM of Common ecosystem, add creatures from Goo pseudo-ecosystem
										/*CSString creaModel2 = creaModel;
										if ( iEcosystem == CommonEcosystem )
										{
											creaModel2[3] = 'g'; // replace CommonEcosystem by Goo for creatures
										}*/

										// We don't write into the real creature sheet, but we want it's _*_mp.creature parent sheet
										const CSString& creaPathAndFilename = creatureAssignmentPath + string("_") + creaSheetName + "_mp." + crSheetType;

										// Find free and used slots (when not overwriting)
										WarningLog->addNegativeFilter( "Can't open" ); // because loadForm() will emit a warning if not existing yet (but passing the filter is very slow) :(
										creaForm = formLoader->loadForm( creaPathAndFilename.c_str() );
										WarningLog->removeFilter( "Can't open" );
										if ( ! creaForm )
											creaForm = formLoader->loadForm( (string("_empty.") + crSheetType).c_str() );
										if ( creaForm )
										{
											CSString rmSheetFilename = sheetName + "." + rmSheetType;
											bool alreadyAssigned = false;
											uint32 firstFreeRMSlot = 0, lastAssignedRMSlot = 0;
											for ( uint32 rMP=1; rMP<=NB_RAW_MATERIAL_FAMILIES_PER_CREATURE; ++rMP )
											{
												string value;
												creaForm->getRootNode().getValueByName( value, toString( "Harvest.MP%u.AssociatedItem", rMP ).c_str() );

												if ( (value.size() == SIZE_RAW_MATERIAL_SHEET_FILENAME) && (value[0] == 'm') )
												{
													// Erase all (old & new!) (needs a pass before assignment)
													if ( EraseOldCreatureAssignments )
													{
														creaForm->getRootNode().setValueByName( "", toString( "Harvest.MP%u.AssociatedItem", rMP ).c_str() );
														//nldebug( "Erasing old assignment %s in %s.creature", value.c_str(), sheetName.c_str() );
													}
													else
													{
														if ( value == rmSheetFilename )
														{
															alreadyAssigned = true;
															TRMItem item;
															item.push( DtName, sheetName );
															item.push( DtCreature, creaSheetName );
															item.push( DtCreaTitle, Titles[creaSheetName] );
															SortableData.updateItemAppend( item, DtCreature );
															SortableData.updateItemAppend( item, DtCreaTitle );
														}
														lastAssignedRMSlot = rMP;
														if ( lastAssignedRMSlot > maxLastAssignedRMSlot )
															maxLastAssignedRMSlot = lastAssignedRMSlot;
													}

													// Test validity of existing assignments (in TestExistingAssigments mode)
													if ( TestExistingAssigments )
													{
														string sn = value.substr( 0, 11 );
														if ( inputSheetPathContent.find( sn ) == inputSheetPathContent.end() )
														{
															nlwarning( "RM %s (assigned to %s.creature) not found", sn.c_str(), creaSheetName.c_str() );
														}
													}
												}
												else if ( firstFreeRMSlot == 0 )
													firstFreeRMSlot = rMP;
											}
											
											if ( EraseOldCreatureAssignments )
											{
												if ( creatureSheets.find( creaSheetName ) == creatureSheets.end() )
												{
													flushSheetToDisk( creaPathAndFilename, creaForm );
													++nbErasedCreatureAssignments;
												}
											}
											else if ( AssignOnlyToUnassignedCreatures && (lastAssignedRMSlot != 0) )
											{
												// Skip assignment if already done (in AssignOnlyToUnassignedCreatures mode)
												if ( creatureSheets.find( creaSheetName ) == creatureSheets.end() )
												{
													++nbCreatureSheetsSkipped;
													nldebug( "Skipped %s.creature", creaSheetName.c_str() );
												}
											}
											else
											{
												// Assign current RM to first free slot
												if ( firstFreeRMSlot != 0 )
												{
													if ( ! alreadyAssigned )
													{
														creaForm->getRootNode().setValueByName( rmSheetFilename.c_str(), toString( "Harvest.MP%u.AssociatedItem", firstFreeRMSlot ).c_str() );
														//nlinfo( "%s %s got %s %s", CreatureModels[CreatureToModel[creaModel2]].Name.c_str(), creaSheetName.c_str(), families[iFam].c_str(), sheetName.c_str() );
														flushSheetToDisk( creaPathAndFilename, creaForm );
														++nbAssignedMaterials;

														// These stats won't be correct if WriteSheetsToDisk is disabled && ExistingAction is ModifyRM (infinite slots) 
														TRMItem item;
														item.push( DtName, sheetName );
														item.push( DtCreature, creaSheetName );
														item.push( DtCreaTitle, Titles[creaSheetName] );
														SortableData.updateItemAppend( item, DtCreature );
														SortableData.updateItemAppend( item, DtCreaTitle );
													}
												}
												else if ( (ExistingRMAction == ModifyRM) && (! alreadyAssigned) )
												{
													nlwarning( "No free slot to assign %s %s %s to %s", CreatureModels[CreatureToModel[creaModel/*2*/]].Name.c_str(), families[iFam].c_str(), sheetName.c_str(), creaSheetName.c_str() );
												}
											}
											creatureSheets.insert( creaSheetName );
										}
										else
										{
											nlwarning( "Can't open %s", creaPathAndFilename.c_str() );
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if ( AssignRawMaterialsHarvestToCreatureSheets )
		{
			// Find creature sheets that were not processed (missing model for example)
			for ( map<string,string>::const_iterator ipc = inputSheetPathContent.begin(); ipc!=inputSheetPathContent.end(); ++ipc )
			{
				const CSString& creaSheetName = (*ipc).first;
				const CSString& creaPathAndFilename = (*ipc).second;

				if ( ! ((creaSheetName[0] == 'c') && (creaSheetName.size() == 6)) )
					continue;

				if ( creatureSheets.find( creaSheetName ) == creatureSheets.end() )
				{
					if ( CreatureMainModelsWithoutRM.find( creaSheetName.substr( 0, 3 ) ) != CreatureMainModelsWithoutRM.end() )
					{
						nldebug( "Creature %s (%s) has no MP", creaSheetName.c_str(), CreatureModels[CreatureToModel[creaSheetName.substr( 0, 5 )]].Name.c_str() );
					}
					else if ( TFamInfo::UseGenerateOnly == 0 ) // avoid warnings when generating only a subset of the RM families
					{
						if ( creaSheetName[CR_INDEX_LOCAL_LEVEL_CODE] == '6' )
							nldebug( "Mission creature '6' %s (%s) not processed", creaSheetName.c_str(), CreatureModels[CreatureToModel[creaSheetName.substr( 0, 5 )]].Name.c_str() );
						else if ( creaSheetName[1] == 'd' )
							nldebug( "Degenerated creature 'cd' %s (%s) not processed", creaSheetName.c_str(), CreatureModels[CreatureToModel[creaSheetName.substr( 0, 5 )]].Name.c_str() );
						else
						{
							string reason;
							uint i;
							for ( i=0; i!=NbEcosystemsPlusExtensions; ++i )
							{
								if ( creaSheetName[CR_INDEX_ECOSYSTEM_CODE] == getCreatureEcosystemCode( i )[0] )
									break;
							}
							if ( i == NbEcosystemsPlusExtensions )
								reason = toString( " (invalid ecosystem %c)", creaSheetName[CR_INDEX_ECOSYSTEM_CODE] );
							nlwarning( "Creature %s (%s) not processed%s", creaSheetName.c_str(), CreatureModels[CreatureToModel[creaSheetName.substr( 0, 5 )]].Name.c_str(), reason.c_str() );
						}
					}

					if ( EraseOldCreatureAssignments )
					{
						CSmartPtr<UForm> creaForm = formLoader->loadForm( (creaSheetName + "." + crSheetType).c_str() );
						if ( creaForm )
						{
							for ( uint32 rMP=1; rMP<=4; ++rMP ) // only to 4 (TODO: use the proper number)
							{
								creaForm->getRootNode().setValueByName( "", toString( "Harvest.MP%u.AssociatedItem", rMP ).c_str() );
								//nldebug( "Erasing old assignment %s in %s.creature", value.c_str(), sheetName.c_str() );
							}
							flushSheetToDisk( creaPathAndFilename, creaForm );
							++nbErasedCreatureAssignments;
						}
						else
							nlwarning( "Can't open %s", creaPathAndFilename.c_str() );
					}
				}
			}

			nlinfo( "%u raw materials assigned to %u creature sheets (%u skipped, already assigned; %u erased; biggest last slot: %u)", nbAssignedMaterials, creatureSheets.size(), nbCreatureSheetsSkipped, nbErasedCreatureAssignments, maxLastAssignedRMSlot );
		}

processRawMaterialsForDeposits:
		//
		// Iterate on deposit location
		//

		fprintf( GraphFile, "\n" );
		nlassert( ! DepositFamilyIndices.empty() );
		map< uint32, vs > materialsByZoneForDeposits;
		NLMISC::CSmartPtr<CForm> depParentForm, depForm;
		float avgSumOriginalityAvg = 0.0f;
		uint32 nbDepositsGenerated = 0;

		if ( SkipRawMaterialsForDeposits )
			goto endDeposits;

		if ( AssignRawMaterialsHarvestToDepositSheets || FixDeposits )
		{
			/*uint32*/ nbAssignedMaterials = 0;

			if ( ExistingRMAction == ModifyRM )
			{
				// Write deposit parent sheet
				depParentForm = loadTemplateForm( formLoader, dpSheetType );
				clearSheet( depParentForm, &form->getRootNode() );
				//parentForm->getRootNode().setValueByName( "Harvest", "Skill" );
				for ( uint32 rMP=0; rMP!=NB_RAW_MATERIALS_PER_DEPOSIT; ++rMP )
				{
					string rm = toString( "MP%02u.", rMP+1 );
					uint32 rSeason;
					for ( rSeason=0; rSeason!=seasons.size(); ++rSeason )
					{
						depParentForm->getRootNode().setValueByName( (uint32)0, (rm + seasons[rSeason] + ".MinQuantity").c_str() );
						depParentForm->getRootNode().setValueByName( (uint32)100, (rm + seasons[rSeason] + ".MaxQuantity").c_str() );
						depParentForm->getRootNode().setValueByName( (uint32)10, (rm + seasons[rSeason] + ".RegenRate").c_str() );
						depParentForm->getRootNode().setValueByName( (uint32)50, (rm + seasons[rSeason] + ".AngryLevel").c_str() );
						depParentForm->getRootNode().setValueByName( (uint32)30, (rm + seasons[rSeason] + ".FuryLevel").c_str() );
						depParentForm->getRootNode().setValueByName( (uint32)10, (rm + seasons[rSeason] + ".BlackKamiLevel").c_str() );
					}
					depParentForm->getRootNode().setValueByName( (uint32)1, (rm + "MinGetQuantity").c_str() );
					depParentForm->getRootNode().setValueByName( (uint32)20, (rm + "MaxGetQuantity").c_str() );
					depParentForm->getRootNode().setValueByName( (uint32)1, (rm + "MinGetQuality").c_str() );
					depParentForm->getRootNode().setValueByName( (uint32)25, (rm + "MaxGetQuality").c_str() );
					depParentForm->getRootNode().setValueByName( (uint32)80, (rm + "PresenceProbabilities").c_str() );
				}
				flushSheetToDisk( "_parent.deposit", depParentForm );

				// Setup deposit sheet
				depForm = loadTemplateForm( formLoader, dpSheetType );
				clearSheet( depForm, &form->getRootNode() );
			}
		}

		// Iterate on ecosystem
		for ( uint32 iEcosystem=0; iEcosystem!=NbEcosystems; ++iEcosystem )
		{
			COriginalitySorter OriginalitySorter;	

			uint32 nbPerEcosystem = min( (uint32)((ecosystemCodes[iEcosystem][0] == 'c') ? 7 : 3), MaxNbRMByFamilyEZ );
			nlinfo( "Generating/browsing %u sets for %s...", nbPerEcosystem, ecosystems[iEcosystem].c_str() );
			materialsByZoneForDeposits.clear();

			vu& familiesForDeposit = DepositFamilyIndices;

			// Iterate on variations in ecosystem
			for ( uint32 iv=0; iv!=nbPerEcosystem; ++iv )
			{
				string variationInEcosystem = string( 1, (char)('a' + iv) );

				// Iterate on level zone
				for ( uint32 iZone=0; iZone<6; ++iZone )
				{
					TStatQuality statQuality = (iZone == 0) ? Basic : (TStatQuality)(iZone - 1);

					// Iterate on families
					for ( vu::iterator ifc=familiesForDeposit.begin(); ifc!=familiesForDeposit.end(); ++ifc )
					{
						uint32 iFam = (*ifc);
						if ( ! TFamInfo::mustGenerateFamily( iFam ) )
							continue;

						// Test if the raw material family must be generated for this ecosystem
						const TFamInfo& rmfamily = FamSet[families[iFam]];
						bool isMissionRawMaterial = rmfamily.IsForMission;
						if ( isMissionRawMaterial )
						{
							// All foraged mission RMs are Common (unlike creature RMs)
							if ( iEcosystem != CommonEcosystem )
								continue;

							// Mission quality ranges from A=B to F
						}
						else
						{
							if ( GenOnlyMissionRawMaterials )
								continue;

							// Basic & Fine in Common, better RMs specialized by ecosystem
							if ( ! rmfamily.existsInEcosystem( (TEcosystem)iEcosystem, statQuality ) )
								continue;

							// Craft quality ranges from B (#0) to F (#4) (deposits of newbielands have B)
							if ( iZone == 0 )
								continue;
						}

						// Iterate on variants
						for ( uint32 iVariant=1; iVariant<=1; ++iVariant )
						{
							CSString sheetName = CSString( "m" )
								+ familyCodes[iFam]
								+ CSString( "dx" ) + variationInEcosystem + ecosystemCodes[iEcosystem] + string( 1, (char)('a' + iZone) )
								+ toString( "%02u", iVariant );

							if ( GenOnlyNewRawMaterials )
							{
								map<string,string>::const_iterator iprm = inputSheetPathContent.find( sheetName );
								if ( iprm != inputSheetPathContent.end() )
									continue;
							}

							//DebugLog->displayRawNL( "%s %s", sheetName.c_str(), getRMName( sheetName ).c_str() );
							if ( OutputNameList )
								fprintf( nameOutputFile, "%s;%s\n", sheetName.c_str(), getRMShortName( sheetName, statQuality, isMissionRawMaterial ).c_str() );

							// Set output directory
							dirbase = conventionalDirectory( ecosystems[iEcosystem] ) + "/";

							CGenRawMaterial *rm = new CGenRawMaterial( sheetName );
							CGenRawMaterial& rawMaterial = *rm;
							rawMaterial.ILocation = InDeposits;
							rawMaterial.IFamily = iFam;
							rawMaterial.Group = FamSet[families[iFam]].Group;
							rawMaterial.IEcosystem = (TEcosystem)iEcosystem;
							rawMaterial.StatQuality = statQuality;
							if ( isMissionRawMaterial )
								rawMaterial.ILevelZone = iZone;
							else
								rawMaterial.ILevelZone = ~0;
							rawMaterial.fillPropertiesFromFamily();

							if ( ExistingRMAction == ModifyRM )
							{
								rawMaterial.MaxLevel = (rawMaterial.StatQuality+1) * 250 / (NB_UNIQUE_LEVELZONES_PER_CONTINENT);
								if ( ! rawMaterial.computeCraftCharacs( iVariant, sheetName ) )
								{
									delete rm;
									continue;
								}

								/*if ( (uint)(rawMaterial.getOriginalityMax()*100.0f) >= OriginalityMinThreshold )
								{
									// Write to disk
									rawMaterial.writeSheet( form );
									writeRMSheetToDisk( form, sheetName );
								}*/

								if ( SortByOriginality )
									OriginalitySorter.pushRM( &rawMaterial );
								else
								{
									selectRawMaterial( form, ecoParentForm, famParentForm, &rawMaterial );
									delete rm;
								}
							}
							else
							{
								if ( inputSheetPathContent.find( sheetName ) != inputSheetPathContent.end() )
								{
									form = (CForm*)formLoader->loadForm( (rawMaterialPath + dirbase + sheetName + "." + rmSheetType).c_str() );
									if ( ! form )
									{
										nlwarning( "Can't load %s", (rawMaterialPath + dirbase + sheetName + "." + rmSheetType).c_str() );
									}
									else
									{
										rawMaterial.loadSheet( form, sheetName, false );

										// Write stats
										TRMItem item;
										rm->collectStats( item, MainStat );
										SortableData.addItem( item );
									}
								}
								delete rm;
							}

						}
					}
				}
			}

			if ( ExistingRMAction == ModifyRM )
			{
				if ( SortByOriginality )
				{
					nlerror( "Deprecated" );
					/*set<uint> usedRMFamilies [5];
					uint sumNBRMCraftBySlot = 0, sumOriginalityAvg = 0;
					for ( uint r=0; r!=NbFaberElements; ++r )
					{
						uint iNBRMCraftBySlot = 0, iUniqueNBRMCraftBySlot = 0;
						nlinfo( "%s: Popping at most %u RM from %u (%ux5) compatible", getShortFaberElemString(r).c_str(), MaxNbRMByCraftSlotE, OriginalitySorter.RMByOriginalityByCraftSlot[r].size(), OriginalitySorter.RMByOriginalityByCraftSlot[r].size()/5 );
						COriginalitySorter::CMultiMapByOriginality::const_iterator imo;
						for ( imo= OriginalitySorter.RMByOriginalityByCraftSlot[r].begin();
							  imo!=OriginalitySorter.RMByOriginalityByCraftSlot[r].end();
							  ++imo )
						{
							if ( iNBRMCraftBySlot >= MaxNbRMByCraftSlotE )
								break;

							const uint32& originality = (*imo).first;
							CGenRawMaterial *rm = (*imo).second;

							if ( ! OriginalitySorter.alreadyPopped( rm ) )
							{
								//InfoLog->displayRawNL( "NewOne: %p: %s %s %u", rm, rm->SheetName.c_str(), faberElems[r].c_str(), (*imo).first );
								selectRawMaterial( form, ecoParentForm, famParentForm, rm );

								// Memorize for deposit assignment
								materialsByZoneForDeposits[rm->ILevelZone+1].push_back( rm->SheetName );
								usedRMFamilies[rm->ILevelZone].insert( rm->IFamily );

								sumOriginalityAvg += (uint)(rm->getOriginalityAvg()*100.0f);
								++iUniqueNBRMCraftBySlot;

								OriginalitySorter.popAndDeleteRM( rm );
							}
							else
							{
								//InfoLog->displayRawNL( "Already: %p: %s %u", rm, faberElems[r].c_str(), (*imo).first );
							}
							++iNBRMCraftBySlot; // when already popped, consider as counting for the rms for current craft slot
						}
						nldebug( "%s: %u rm generated (for deposits)", CraftParts[r].Name.c_str(), iNBRMCraftBySlot );
						sumNBRMCraftBySlot += iUniqueNBRMCraftBySlot;
					}
					nlinfo( "Total selected for deposits in %s: %u", ecosystems[iEcosystem].c_str(), sumNBRMCraftBySlot );
					
					// Force at least one rm of each (family, levelzone)
					uint iRM = 0;
					for ( uint32 iLevelZone=0; iLevelZone!=5; ++iLevelZone )
					{
						COriginalitySorter::CRMSet::const_iterator itRm = OriginalitySorter.getRMSetBegin();
						while ( (itRm = OriginalitySorter.getFirstRMNotInFamilyListFromPos( usedRMFamilies[iLevelZone], iLevelZone, itRm )) != OriginalitySorter.getRMSetEnd() )
						{
							CGenRawMaterial *rm = (*itRm);
							selectRawMaterial( form, ecoParentForm, famParentForm, rm );

							// Memorize for deposit assignment
							materialsByZoneForDeposits[rm->ILevelZone+1].push_back( rm->SheetName );
							usedRMFamilies[iLevelZone].insert( rm->IFamily );
							nldebug( "Adding %s (family %s zone %c)", rm->SheetName.c_str(), rm->familyStr().c_str(), 'A' + (char)(rm->ILevelZone+1) );
							++iRM;
							sumOriginalityAvg += (uint)(rm->getOriginalityAvg()*100.0f);
						}
					}
					nlinfo( "Added %u RM (one per family per levelzone)", iRM );
					sumNBRMCraftBySlot += iRM;
					avgSumOriginalityAvg += (float)sumOriginalityAvg / (float)sumNBRMCraftBySlot;
					
					// Delete rm objects
					OriginalitySorter.deleteAllRemainingRM();
					*/
				}
			}

			if ( AssignRawMaterialsHarvestToDepositSheets || FixDeposits )
			{
				nlinfo( "Generating/browsing deposits for %s...", ecosystems[iEcosystem].c_str() );
				
				// Iterate on level zone
				for ( uint32 iZone=0; iZone<=5; ++iZone )
				{
					string depSheetName = string("d") + ecosystemCodes[iEcosystem].c_str() + toString( "%caa", (char)('a' + iZone) );

					if ( ExistingRMAction == ModifyRM )
					{
						clearSheet( depForm, &depForm->getRootNode() );
						depForm->insertParent( 0, "_parent.deposit", depParentForm );

						// Get the list of raw material families and make a random selection of raw material sheets
						set<CSString, CUnsensitiveSStringLessPred> usedFamilies;
						uint32 iz = (iZone==0) ? 1 : iZone; // 'A' gets raw materials of zone 'B'
						vs& compatibleMaterials = materialsByZoneForDeposits[iz];
						for ( uint32 rMP=0; rMP!=NB_RAW_MATERIALS_PER_DEPOSIT; ++rMP )
						{
							bool famOk = false;
							uint32 escapeCounter = 0;
							do
							{
								CSString rmName;

								// Particular cases (forced for LD missions)
								if ( iz == 1 )
								{
									/*switch ( rMP )
									{
									case 0:
										rmName = "mdubcbwof01";
										break;
									case 1:
										rmName = "mduacbwoo01";
										break;
									case 2:
										rmName = "mduacbref01";
										break;
									}*/
								}
								if ( rmName.empty() )
								{
									// Generic case, get a random value
									if ( ! compatibleMaterials.empty() )
									{
										uint32 iMaterial = getRandomValue( compatibleMaterials.size() );
										rmName = compatibleMaterials[iMaterial];
									}
								}

								// No compatible material?
								if ( rmName.empty() )
								{
									escapeCounter = 16;
									break;
								}
								
								// Prevent from having the same family more than once
								CSString extFamily =  rmName.substr( RM_INDEX_FAMILY_CODE, NB_FAMILY_CODE_CHARS );
								if ( usedFamilies.find( extFamily ) == usedFamilies.end() )
								{
									usedFamilies.insert( extFamily );

									// Assign RM to current deposit slot
									depForm->getRootNode().setValueByName( (rmName+"."+rmSheetType).c_str(), toString( "MP%02u.AssociatedItem", rMP+1 ).c_str() );
									nldebug( "%s got %s %s", depSheetName.c_str(), families[getIndexFromString( rmName.substr( 6, 2 ), familyCodes )].c_str(), rmName.c_str() );
									++nbAssignedMaterials;

									// Test validity of existing assignments (in TestExistingAssigments mode)
									if ( TestExistingAssigments )
									{
										if ( inputSheetPathContent.find( rmName ) == inputSheetPathContent.end() )
										{
											nlwarning( "RM %s (assigned to %s.deposit) not found", rmName.c_str(), depSheetName.c_str() );
										}
									}

									TRMItem depItem;
									depItem.push( DtName, rmName );
									//depItem.push( DtDeposit, depSheetName );
									//SortableData.updateItemAppend( depItem, DtDeposit );

									famOk = true;
								}
								else
									++escapeCounter;
							}
							while ( (! famOk) && (escapeCounter < 16) );
							if ( ! famOk )
							{
								nlwarning( "Deposit %s got only %u RM", depSheetName.c_str(), rMP );
								break;
							}
						}

						flushSheetToDisk( depositPath + depSheetName + "." + dpSheetType, depForm );
						++nbDepositsGenerated;
					}
					else if ( ! FixDeposits )
					{
						const string& filename = inputSheetPathContent[depSheetName];
						if ( filename.empty() )
							nlwarning( "%s not found", (depSheetName + "." + dpSheetType).c_str() );
						else
							depForm = (CForm*)formLoader->loadForm( filename.c_str() );

						uint32 nbRMinDep = 0;
						//string rms = depSheetName + ": ";
						for ( uint32 rMP=0; rMP!=NB_RAW_MATERIALS_PER_DEPOSIT; ++rMP )
						{
							CSString value;
							depForm->getRootNode().getValueByName( value, toString( "MP%02u.AssociatedItem", rMP+1 ).c_str() );
							if ( ! value.empty() )
							{
								CSString rmName = value.rightCrop( rmSheetType.size() + 1 );
								TRMItem depItem;
								depItem.push( DtName, rmName );
								//depItem.push( DtDeposit, depSheetName );
								//rms += rmName + ",";
								//SortableData.updateItemAppend( depItem, DtDeposit );
								++nbRMinDep;
							}
						}
						//nlinfo( "%u RM in deposit %s: %s", nbRMinDep, depSheetName.c_str(), rms.c_str() );
					}
				}
			}
		}
		if ( AssignRawMaterialsHarvestToDepositSheets )
		{
			nlinfo( "%u raw materials assigned to %u deposit sheets", nbAssignedMaterials, nbDepositsGenerated );
		}
		nlinfo( "Average originality: %.2f", (float)avgSumOriginalityAvg / (float)ecosystems.size() );
endDeposits:
		nldebug( "End of generation" );
	}

	if ( BrowseOtherDeposits || FixDeposits )
	{
		// Access deposits
		vector<string> depositFiles;
		CPath::getPathContent( depositPath, true, false, true, depositFiles );
		for ( vector<string>::const_iterator idf=depositFiles.begin(); idf!=depositFiles.end(); ++idf )
		{
			const string& filename = (*idf);
			const CSString& depSheetName = CFile::getFilenameWithoutExtension( filename );

			if ( (filename.find( ".deposit" ) != string::npos) && (depSheetName[0] == 'd') && (depSheetName.size() == 5) )
			{
				bool isGenerated = (depSheetName.right( 2 ) == "aa");

				// If not fixing deposits (just browsing), discard generated ones (already browsed)
				if ( (! FixDeposits) && isGenerated )
					continue;

				CSmartPtr<CForm> depForm = (CForm*)formLoader->loadForm( filename.c_str() );
				bool modified = false;

				// Browse or check/substitute all RM assigned to deposit
				for ( uint32 rMP=0; rMP!=NB_RAW_MATERIALS_PER_DEPOSIT; ++rMP )
				{
					CSString value, substitution;
					depForm->getRootNode().getValueByName( value, toString( "MP%02u.AssociatedItem", rMP+1 ).c_str() );
					if ( value.empty() )
						continue;

					if ( ! isGenerated )
					{
						TRMItem depItem;
						depItem.push( DtName, value );
						//depItem.push( DtDeposit, depSheetName );
						//SortableData.updateItemAppend( depItem, DtDeposit );
					}

					if ( FixDeposits )
					{
						nlerror( "TODO" ); // this code is for V2, not suitable for V3
						/*value = value.rightCrop( 6 ); // remove .sitem
						CRMData::CLookup::const_iterator ilk = SortableData.lookup( DtName ).find( value );
						if ( ilk == SortableData.lookup( DtName ).end() )
						{
							// Find a substitution by declinaison/location
							char adjCode = value[8];
							string substitution = findSubstitutionForDepositRM( value, adjCode );
							if ( substitution.empty() )
							{
								// Find a substitution by adjective (emit a warning)
								for ( mss::const_iterator ita=adjectives.begin(); ita!=adjectives.end(); ++ita )
								{
									adjCode = tolower( (*ita).first[0] );
									if ( adjCode == value[8] )
										continue;

									substitution = findSubstitutionForDepositRM( value, adjCode );
									if ( ! substitution.empty() )
									{
										nlinfo( "Replacing adj %s by %s for %s", adjectives[string(1,(char)toupper(value[8]))].c_str(), (*ita).second.c_str(), value.c_str() );
										break;
									}
								}
							}
							if ( substitution.empty() )
							{
								nlwarning( "Could not find a substitution for %s in %s", value.c_str(), depSheetName.c_str() );
							}
							else
							{
								depForm->getRootNode().setValueByName( (substitution+".sitem").c_str(), toString( "MP%02u.AssociatedItem", rMP+1 ).c_str() );
								nlinfo( "%s replaced by %s", value.c_str(), substitution.c_str() );
								modified = true;
							}

						}
						if ( modified )
						{
							flushSheetToDisk( filename, depForm );
						}*/
					}
				}
			}
		}
	}

	if ( OutputNameList )
		fclose( nameOutputFile );
	fclose( GraphFile );

	//nlinfo( "%u RM in repository", Repository.size() );

	// Produce main doc
	CProducedDocHtml			MainDoc;
	CProducedDocCSV				MainCSV;
	MainDoc.open( "rm.html", "Raw materials by generation order", ProduceDoc );
	MainCSV.open( "output_rm_for_craft.csv", ProduceDoc );
	MainDoc.write( "<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n" );
	MainDoc.write( "<tr>" );
	for ( uint32 c=0; c!=DtNbCols; ++c )
	{
		MainDoc.write( "<td><b><a href=\"rm_" + string(DataColStr[c]) + ".html\">" + string(DataColStr[c]) + "</a></b></td>" );
		MainCSV.write( string(DataColStr[c]) + "," );
	}
	MainDoc.write( "</tr>" );
	MainCSV.write( "\n" );
	for ( CRMData::CItems::const_iterator isd=SortableData.items().begin(); isd!=SortableData.items().end(); ++isd )
	{
		MainDoc.write( (*isd).toHTMLRow() );
	}
	MainDoc.write( "</tbody><table>\n" );
	
	// Produce alt docs
	CProducedDocHtml			AltDocs[DtNbCols];
	for ( uint32 c=0; c!=DtNbCols; ++c )
	{
		AltDocs[c].open( "rm_" + string(DataColStr[c]) + ".html", "Raw materials by " + string(DataColStr[c]), ProduceDoc );
		AltDocs[c].write( "<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n" );
		AltDocs[c].write( "<tr>" );
		for ( uint32 cc=0; cc!=DtNbCols; ++cc )
			if ( cc == c )
				AltDocs[c].write( "<td><b>" + string(DataColStr[cc]) + "</b></td>" );
			else
				AltDocs[c].write( "<td><b><a href=\"rm_" + string(DataColStr[cc]) + ".html\">" + string(DataColStr[cc]) + "</a></b></td>" );
		AltDocs[c].write( "</tr>" );
		string previousKey = "[NO PREVIOUS]"; // not a blank string, because it may be a valid value
		string previousName = "";
		for ( CRMData::CLookup::const_iterator isd=SortableData.lookup( c ).begin(); isd!=SortableData.lookup( c ).end(); ++isd )
		{
			const TRMItem& item = SortableData.getRow( (*isd).second );
			AltDocs[c].write( item.toHTMLRow( c, (*isd).first, previousKey, DtName, previousName ) );

			if ( c == DtCraftSlotName )
				MainCSV.write( item.toCSVLine( ',', "", c, (*isd).first, previousKey, DtName, previousName ) );

			previousKey = (*isd).first;
			previousName = item.Fields[DtName][0];
		}
		AltDocs[c].write( "</tbody><table>\n" );
		AltDocs[c].save();
	}

	// Stats
	if ( (nbSheetsProcessed != 0) )
	{
		CProducedDocHtml StatFile;
		StatFile.open( "rm_stats.html", "Raw material statistics", ProduceDoc );
		StatFile.writepln( toString( "<B>Total: %u raw materials (%u new)", nbSheetsProcessed, nbNewSheetsGenerated ) );
		StatFile.writepln( toString( "Faber elements filled: avg %u, min %u, max %u</B>", MainStat.SumNbFaberElemsFilled / nbSheetsProcessed, MainStat.MinNbFaberElemsFilled, MainStat.MaxNbFaberElemsFilled ) );

		// What can be crafted from each raw material family
		StatFile.writeln( "What can be crafted from each raw material family:<br>" );
		StatFile.writeln( "<ul>" );
		for ( uint32 iFam=0; iFam!=families.size(); ++iFam )
		{
			if ( ! FamSet[families[iFam]].IsActive )
				continue;

			string propStr;
			vs& props = FamSet[families[iFam]].Properties;
			for ( vs::const_iterator ip = props.begin(); ip!=props.end(); ++ip )
			{
				if ( ip != props.begin() )
					propStr += ", ";
				propStr += (*ip);
			}
			uint nbLines = 0;
			StatFile.writeln( "<li><p>" + families[iFam] + ": " + propStr + "</p>" );
			StatFile.writeln( "<ul>" );
			for ( uint32 c=0; c!=NbCiv; ++c )
			{
				bool civHasAPlan = false;
				for ( uint32 rFaberElem=0; rFaberElem!=NbFaberElements; ++rFaberElem )
				{
					if ( MainStat.NbRMByFaberElemByFamilyAndCiv[iFam][c][rFaberElem] != 0 )
					{
						civHasAPlan = true;
						break;
					}
				}
				if ( civHasAPlan )
				{
					StatFile.writeln( "<li><p>" + string(CivNames[c]) + " plans</p>" );
					StatFile.writeln( "<ul>" );
					for ( uint32 rFaberElem=0; rFaberElem!=NbFaberElements; ++rFaberElem )
					{
						if ( MainStat.NbRMByFaberElemByFamilyAndCiv[iFam][c][rFaberElem] != 0 )
						{
							StatFile.writebln( CraftParts[rFaberElem].Name + toString(": %u different", MainStat.NbRMByFaberElemByFamilyAndCiv[iFam][c][rFaberElem] ) );
							++nbLines;
						}
					}
					StatFile.writeln( "</ul>" );
				}
			}
			StatFile.writeln( "</ul>" );
			if ( nbLines == 0 )
			{
				if ( ! FamSet[families[iFam]].IsForMission )
				{
					if ( ! ( (SkipRawMaterialsForCreatures && FamSet[families[iFam]].IsInCreatures) ||
						     (SkipRawMaterialsForDeposits && FamSet[families[iFam]].IsInDeposits) ) )
					nlwarning( "%s is not used by any craft plan!", families[iFam].c_str() );
				}
				StatFile.writebln( "<font color=\"red\">No use for this family</font>" );
				// Tip: if RM of a creature, check if the creature exists
			}
		}
		StatFile.writeln( "</ul>" );

		// Number of compatible RM by ecosystem, by craft slot
		vector<uint32> nbFabs( NbEcosystems, 0 );
		for ( uint32 rFaberElem=0; rFaberElem!=NbFaberElements; ++rFaberElem )
		{
			StatFile.writebln( toString( "<B>%s: %u compatible RM</B>", CraftParts[rFaberElem].Name.c_str(), MainStat.NbRMByFaberElem[rFaberElem] ) );
			if ( CraftParts[rFaberElem].Name.c_str(), MainStat.NbRMByFaberElem[rFaberElem] != 0 )
			{
				for ( uint32 iEcosystem=0; iEcosystem!=ecosystems.size(); ++iEcosystem )
				{
					StatFile.writebln( toString( "(%u in %s)", MainStat.NbRMByFaberElemByEcosystem[iEcosystem][rFaberElem], ecosystems[iEcosystem].c_str() ) );
					nbFabs[iEcosystem] += MainStat.NbRMByFaberElemByEcosystem[iEcosystem][rFaberElem];
				}
			}
		}
		for ( uint32 iEcosystem=0; iEcosystem!=ecosystems.size(); ++iEcosystem )
		{
			StatFile.writebln( toString( "<B>(Total %u in %s)</B>", nbFabs[iEcosystem], ecosystems[iEcosystem].c_str() ) );
		}

		// Number of properties by ecosystem
		for ( uint32 iProp=0; iProp!=properties.size(); ++iProp )
		{
			uint32 nbTotal = 0;
			/*for ( uint32 iLoc=0; iLoc!=NB_LOCATIONS; ++iLoc )
			{
				uint32 nbInLoc = 0;
				for ( uint32 iEcosystem=0; iEcosystem!=ecosystems.size(); ++iEcosystem )
				{
					uint32 nb = NbRMHavingProperty[iLoc][iEcosystem][iProp];
					StatFile.writeln( toString( "\t\t%s %s %s: %u RM", locationNames[iLoc], ecosystems[iEcosystem].c_str(), properties[iProp].c_str(), nb ) );
					nbInLoc += nb;
				}
				StatFile.writeln( toString( "\t%s %s: %u RM", locationNames[iLoc], properties[iProp].c_str(), nbInLoc ) );
			}*/
			StatFile.writeln( "<ul>" );
			for ( uint32 iEcosystem=0; iEcosystem!=ecosystems.size(); ++iEcosystem )
			{
				uint32 nbInEco = 0;
				for ( uint32 iLoc=0; iLoc!=NB_LOCATIONS; ++iLoc )
				{
					uint32 nb = MainStat.NbRMHavingProperty[iLoc][iEcosystem][iProp];
					//StatFile.writebln( toString( "\t\t%s %s %s: %u RM", locationNames[iLoc], ecosystems[iEcosystem].c_str(), properties[iProp].c_str(), nb ) );
					nbInEco += nb;
				}
				StatFile.writebln( toString( "%s %s: %u", ecosystems[iEcosystem].c_str(), properties[iProp].c_str(), nbInEco ) );
				nbTotal += nbInEco;
			}
			StatFile.writeln( "</ul>" );
			StatFile.writebln( toString( "%s: %u RM", properties[iProp].c_str(), nbTotal ) );
		}
		StatFile.save();
		MainDoc.writepln( "<a href=\"rm_stats.html\">Go to statistics</a>" );
	}

	MainDoc.save();
	MainCSV.save();

#if 0
		//
		// O L D - V 1
		//
						// Randomize color (POSITIVE filter)
						CRulesFilter& familyColorFilter = familyColorFilters[iEcosystem][iFamily].empty() ? familyColorFilters[CommonEcosystem] : familyColorFilters[iEcosystem];
						if ( familyColorFilter[iFamily].empty() )
						{
							nlwarning( "%s not generated: no possible color in any ecosystem", families[iFamily].c_str() );
							continue;
						}
						else
						{
							iColor = familyColorFilter[iFamily][getRandomValue( familyColorFilter[iFamily].size() )];
						}

						if ( isCrSpecialization )
							form->getRootNode().setValueByName( (creatures[iCreature] + " " + strlwr( static_cast<const string&>(families[iFamily]) )).c_str(), "basics.name" );

	/*
	 * Creatures
	 */
	if ( AssignRawMaterialsHarvestToCreatureSheets )
	{
		uint32 nbCreatures = 0, nbCreaturesInSubPath = 0;
		nlinfo( "Generating assignments to creature sheets..." );
		getTransposedMap( creatureFamilyFilter, familyCreatureFilter );
		for ( CRulesFilter::const_iterator ir=creatureFamilyFilter.begin(); ir!=creatureFamilyFilter.end(); ++ir )
		{
			const char *sCreature = ((*ir).first == ~0) ? "COMMON" : creatures[(*ir).first].c_str();
			nlinfo( "Creature %s -> %u compatible raw materials families", sCreature, (*ir).second.size() );
		}

		// Browse creature sheet repository
		map<string,string>::const_iterator ipc;
		for ( ipc=inputSheetPathContent.begin(); ipc!=inputSheetPathContent.end(); ++ipc )
		{
			const string& sheetName = (*ipc).first;
			const string& filename = (*ipc).second;
			if (CFile::getExtension( filename ) == crSheetType)
			{
				nldebug( "%s", filename.c_str() );
				++nbCreatures;
			}
			if ( (CFile::getExtension( filename ) == crSheetType) &&
				 (strlwr(filename).find( creaturePath ) != string::npos) )
			{
				++nbCreaturesInSubPath;
				nldebug( "OK" );
				// Load creature sheet
				form = (CForm*)formLoader->loadForm( filename.c_str() );
				if ( ! form )
				{
					nlwarning( "Can't load sheet %s", filename.c_str() );
					continue;
				}

				string eco = string(1, sheetName[1+NB_CREATURE_CODE_CHARS]);
				uint32 iEcosystem = getIndexFromString( eco, ecosystemCodes );
				if ( iEcosystem == ~0 )
				{
					nlwarning( "Ecosystem code %s in %s unknown", eco.c_str(), sheetName.c_str() );
					continue;
				}

				string cr = sheetName.substr( 1, NB_CREATURE_CODE_CHARS );
				uint32 iCreature = getIndexFromString( cr, creatureCodes );
				if ( iCreature == ~0 )
				{
					nlwarning( "Creature code %s in %s unknown", cr.c_str(), sheetName.c_str() );
					continue;
				}

				// Transform creature level to faber interest level ((ZL, lastLL) = (ZL+1, FirstLL))
				TFaberInterestLevel mainLevel; // mainLevel in [0.. NbFaberInterestLevelsByEcosystem[iEcosystem][
				sint localLevel = (sint)(sheetName[1+NB_CREATURE_CODE_CHARS+2] - '1');
				uint32 zoneLevel = (sint)(sheetName[1+NB_CREATURE_CODE_CHARS+1] - 'a');
				mainLevel = zoneLevel*NB_UNIQUE_LEVELS_PER_ZONE + localLevel;
				nlassertex( mainLevel >= 0, ("%s ZL=%d LL=%d FL=%d", sheetName.c_str(), zoneLevel, localLevel, mainLevel) );
				if ( mainLevel >= NbFaberInterestLevelsByEcosystem[iEcosystem] )
				{
					nlwarning( "Wrong level %s ZL=%d maxLL=%u LL=%d maxFL=%d FL=%d", sheetName.c_str(), zoneLevel, NB_UNIQUE_LEVELS_PER_ZONE+1, localLevel, NbFaberInterestLevelsByEcosystem[iEcosystem], mainLevel );
					mainLevel = NbFaberInterestLevelsByEcosystem[iEcosystem] - 1;
				}

				for ( uint32 rFamily=0; rFamily!=creatureFamilyFilter[iCreature].size(); ++rFamily )
				{
					if ( rFamily > NB_RAW_MATERIAL_FAMILIES_PER_CREATURE )
					{
						nlwarning( "Too many compatible materials found (%u) for %s (%s, %s)", creatureFamilyFilter[iCreature].size(),
							sheetName.c_str(), (iCreature==~0) ? cr.c_str() : creatures[iCreature].c_str(), families[creatureFamilyFilter[iCreature][rFamily]].c_str() );
						continue;
					}

					sint levelModifier = ((sint)getRandomValue( 3 )) - 1; // {-1, 0, 1}
					TFaberInterestLevel actualLevel = max( 0, min( NbFaberInterestLevelsByEcosystem[iEcosystem]-1, mainLevel + levelModifier ) );
					char *errorReport;
					string rmName = findRawMaterialFromCriteria( iEcosystem, creatureFamilyFilter[iCreature][rFamily], iCreature, actualLevel, &errorReport );
					if ( rmName.empty() )
					{
						nlwarning( "%s %s (%s) has no match (%s)",
							(iCreature==~0) ? cr.c_str() : creatures[iCreature].c_str(), families[creatureFamilyFilter[iCreature][rFamily]].c_str(), sheetName.c_str(), errorReport );
					}
				}

	/*
	 * Deposits
	 */

		// For each ecosystem
		for ( iEcosystem=0; iEcosystem!=ecosystemDepositFilters.size(); ++iEcosystem )
		{
			// For each deposit level
			uint32 rLevel;
			for ( rLevel=0; rLevel!=5; ++rLevel )
			{

			}
		}
	}

	// Test deposit validity
	/*if ( TestExistingAssigments )
	{
		map<string,string>::const_iterator ipc;
		for ( ipc=inputSheetPathContent.begin(); ipc!=inputSheetPathContent.end(); ++ipc )
		{
			const string& sheetName = (*ipc).first;
			const string& filename = (*ipc).second;
			if ( (CFile::getExtension( filename ) == dpSheetType) )
			{
				// Load deposit sheet
				form = (CForm*)formLoader->loadForm( filename.c_str() );
				if ( ! form )
				{
					nlwarning( "Can't load sheet %s", filename.c_str() );
					continue;
				}

				for ( uint32 iRM=0; iRM!=30; ++iRM )
				{
					string value;
					form->getRootNode().getValueByName( value, toString( "MP%02u.AssociatedItem", iRM+1 ).c_str() );
					if ( (value.size() == 19) && (value[0] == 'm') )
					{
						string sn = value.substr( 0, 14 );
						if ( inputSheetPathContent.find( sn ) == inputSheetPathContent.end() )
						{
							nlwarning( "RM %s (assigned to %s.deposit) not found", sn.c_str(), sheetName.c_str() );
						}
					}
				}
			}
		}
	}*/
#endif
}



/*
 *
 */
void	usage(char *argv0, FILE *out)
{
	fprintf(out, "\n");
	fprintf(out, "Syntax: %s [-p <sheet path>]", argv0);
	fprintf(out, "\n");
}


/*
 *
 */
int main(int argc, char* argv[])
{
	// parse command line
	uint32	i;
	for (i=1; (sint)i<argc; i++)
	{
		const char	*arg = argv[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
			case 'p':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <sheet path> after -p option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				inputSheetPath = argv[i];
				break;
			case 'n':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <names csv file> after -n option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				ExtractNamesCsv = argv[i];
				break;
			}
		}
	}

	if ( ! ExtractNamesCsv.empty() )
		extractRawMaterialNames();
	else
		generateRawMaterials();

	return 0;
}


// Impossible to insert game_share/protection_type.h into project because of precompiled headers
namespace PROTECTION_TYPE
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TProtectionType)
		NL_STRING_CONVERSION_TABLE_ENTRY(Cold)
		NL_STRING_CONVERSION_TABLE_ENTRY(Acid)
		NL_STRING_CONVERSION_TABLE_ENTRY(Rot)
		NL_STRING_CONVERSION_TABLE_ENTRY(Fire)
		NL_STRING_CONVERSION_TABLE_ENTRY(Shockwave)
		NL_STRING_CONVERSION_TABLE_ENTRY(Poison)
		NL_STRING_CONVERSION_TABLE_ENTRY(Electricity)
		NL_STRING_CONVERSION_TABLE_ENTRY(Madness)
		NL_STRING_CONVERSION_TABLE_ENTRY(Slow)
		NL_STRING_CONVERSION_TABLE_ENTRY(Snare)
		NL_STRING_CONVERSION_TABLE_ENTRY(Sleep)
		NL_STRING_CONVERSION_TABLE_ENTRY(Stun)
		NL_STRING_CONVERSION_TABLE_ENTRY(Root)
		NL_STRING_CONVERSION_TABLE_ENTRY(Blind)
		NL_STRING_CONVERSION_TABLE_ENTRY(Fear)
		NL_STRING_CONVERSION_TABLE_ENTRY(None)
	NL_END_STRING_CONVERSION_TABLE(TProtectionType, ProtectionTypeConversion, None)
		
	
	//-----------------------------------------------
	// fromString:
	//-----------------------------------------------
	TProtectionType fromString(const std::string &str)
	{
		return ProtectionTypeConversion.fromString(str);
	}


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const std::string& toString(TProtectionType protection_type)
	{
		return ProtectionTypeConversion.toString(protection_type);
	}
}; // PROTECTION_TYPE
