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



#ifndef NL_SRG_UTILITIES_H
#define NL_SRG_UTILITIES_H


// Misc
#include <nel/misc/types_nl.h>
#include <nel/misc/sstring.h>
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include <nel/misc/diff_tool.h>
#include <nel/misc/random.h>
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_type.h"
// Georges, bypassing interface
#include "georges/stdgeorges.h"
#include "georges/form.h"
// C
#include <time.h>
#include <conio.h>
// stl
#include <set>
#include <map>
#include <hash_map>

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


typedef sint TFaberInterestLevel;
const TFaberInterestLevel NAInterestLevel = -1;
const uint32 NbNomenclaturedFaberLevel = 5; // excluding N/A
const char *sNomenclaturedInterestLevels [NbNomenclaturedFaberLevel+1] = { "N/A", "Worst", "Bad", "Average", "Good", "Best" }; // from -1 to 5
CRandom RandomGenerator;

typedef CVectorSString vs;

typedef map <uint32, vector<uint32> > CRulesFilter;
typedef map <CSString, vector<uint32>, CUnsensitiveSStringLessPred > CRulesStrFilter;
typedef map <CSString, vs, CUnsensitiveSStringLessPred > CRulesStr2Filter;
typedef map< CSString, CSString, CUnsensitiveSStringLessPred > mss;
typedef vector<uint32> vu;
typedef hash_map< string, string, hash<string> > CTitles; // code -> utf8 title

// Write sheet files, or display to screen only
bool WriteSheetsToDisk = true;

// Overriden by command-line argument after -n
string ExtractNamesCsv;


const uint32 NbFaberElements = 26;

const uint32 RM_INDEX_FAMILY_CODE = 1;
const uint32 NB_FAMILY_CODE_CHARS = 4;

const uint32 RM_INDEX_CREATURE_CODE = 5;
const uint32 NB_CREATURE_CODE_CHARS = 5;

const uint32 RM_INDEX_ECOSYSTEM_CODE = 8;
const uint32 NB_ECOSYSTEM_CODE_CHARS = 1;

const uint32 RM_INDEX_LEVELZONE_CODE = 9;

const uint32 SIZE_RAW_MATERIAL_SHEET_FILENAME = 18;

/*const uint32 RM_INDEX_INTEREST_LEVEL = 6;
const uint32 NB_INTEREST_LEVEL_CHARS = 2;

const uint32 RM_INDEX_FABERELEMS_CODE = 8;
const uint32 NB_FABERELEMS_CODE_CHARS = 6;*/

const uint32 CR_INDEX_ECOSYSTEM_CODE = 3; // in creature code
const uint32 CR_INDEX_LEVELZONE_CODE = 4;
const uint32 CR_INDEX_LOCAL_LEVEL_CODE = 5;

//const uint32 NB_UNIQUE_LEVELS_PER_ZONE = 4; // not counting the bosses, the fifth equals the first one of next 
const uint32 NB_UNIQUE_LEVELZONES_PER_CONTINENT = 5;
const uint32 MAX_NB_LOCAL_LEVELS = 8;
//const uint32 NB_ZONE_LEVELS = 5; // TEMP for forest ecosystem
//const uint32 NbFaberInterestLevels = (NB_ZONE_LEVELS * NB_UNIQUE_LEVELS_PER_ZONE) + 1; // excluding N/A

string				inputSheetPath;
bool				inputSheetPathLoaded = false;
map<string, string>	inputSheetPathContent; // short filename without ext, full filename with path
string				TranslationPath;
string				SystemMPPath;


// These vectors have the same indices : by family
vs families;
vs familyCodes, ecosystemCodes, propertyCodes, creatureCodes;

sint MaxFamilyNum = -1;

enum TColor { Red, Beige, Green, Turquoise, Blue, Violet, White, Black, NbColors, InvalidColor=NbColors };

// By ecosystem, color, property, rm group, creature, season
vs ecosystems, colors, properties, groups, creatures, seasons;
mss adjectives;

// DtName must be the 1st one
enum TDataCol { DtName, DtTitle, DtRMFamily, DtGroup, DtEcosystem, DtLevelZone, DtStatQuality, DtProp, DtCreature, DtCreaTitle, DtCraftSlotName, DtCraftCivSpec, DtColor, DtAverageEnergy, DtMaxLevel, DtJewelProtectionType, DtCustomizedProperties, DtNbCols };
const char *DataColStr [DtNbCols] = { "Code", "Name", "Family", "Group", "Ecosystem", "LevelZone", "Stat Quality", "Properties", "Creature sheets", "Creatures", "Item parts", "Craft civ spec", "Color", "Average energy", "Max level", "Jewel protection type", "Customized" };

const uint32 NbPropertyDepths = 5;
const char *PropertyDepths [NbPropertyDepths] = { "Unknown", "Slightly", "Moderately", "Quite", "Extremely" };

typedef uint32 TGroup; // index in groups, and value in groups .typ

//enum TLocation { Under, Over, Flora, Creatures, NB_LOCATIONS, NB_DEPOSITS=Flora+1 };
enum TLocation { InDeposits, InCreatures, NB_LOCATIONS };
vu DepositFamilyIndices;

enum TCiv { Fyros, Matis, Tryker, Zorai, AllCiv, NbCiv };
const char *CivNames [NbCiv] = { "Fyros", "Matis", "Tryker", "Zorai", "All" };
const char *CivEcosystemCodes [NbCiv] = { "D", "F", "L", "J", "X" };

enum TEcosystem { CommonEcosystem, Desert, Forest, Lacustre, Jungle, PrimeRoots, NbEcosystems, Goo=NbEcosystems, Invasion, Raid, Event, N, S, T, U, V, W, X, Y, Z, NbEcosystemsPlusExtensions };

TCiv EcosystemToCiv[NbEcosystems] = { AllCiv, Fyros, Matis, Tryker, Zorai, AllCiv };

enum TStatQuality { Basic, Fine, Choice, Excellent, Supreme, NbStatQualities, InvalidStatQuality=NbStatQualities };
char *StatQualityStr [NbStatQualities+1] = { "Basic", "Fine", "Choice", "Excellent", "Supreme", "N/A" };

TStatQuality CreatureLocalLevelToStatQuality [MAX_NB_LOCAL_LEVELS] =
{
	Basic,				// 1 (index 0)
	Fine,				// 2 
	Basic,				// 3
	Fine,				// 4
	Excellent,			// 5 (named creatures)
	InvalidStatQuality,	// 6 (mission creatures, their RMs have no craft stats)
	Supreme,			// 7 (bosses)
	Choice				// 8 (mini-bosses)
};


enum TIndexOfRemarkableStatIndex { RBestA, RBest=RBestA, RWorstA1, RWorst1=RWorstA1, RWorstA2, RWorst2=RWorstA2, RBestB, RWorstB1, RWorstB2, NB_REMARKABLE_STAT_INDICES };

struct TSkeletonInfo
{
	TSkeletonInfo() : IsUsed(false) {}

	//vs				CreaturesOfSke;
	CSString			Name;
	CSString			AbbrevName;
	//CSString			SkGroup;
	bool				IsUsed; // true if has RM
};

typedef map<CSString, TSkeletonInfo, CUnsensitiveSStringLessPred> CSkeletonMap;

// By skeleton group, by skeleton, bu creature model
CRulesStr2Filter		SkgroupToModels;
CSkeletonMap			CreatureModels;
mss						CreatureToModel;
//vs						skeletonGroupColumns;
set<CSString, CUnsensitiveSStringLessPred>			CreatureMainModelsWithoutRM;

// By creature model
CRulesStrFilter	RMFamilyIndicesByCreatureModel;
vu GooCreatureFamilyIndices;

typedef map<char, vu> CCreatureTypeToFamilyIndices; // the key is the 2nd char of the creature code (h for herbivore, k for kitin...)
CCreatureTypeToFamilyIndices InvasionRaidCreatureFamilyIndices;

struct TBool
{
	TBool() : Done(false) {}
	bool Done;
};

typedef map< uint, TBool > CDoneMap; // indexed by levelzone
map< string, TBool > IsRMSheetGenerated; // indexed by sheetname

string rawMaterialPath, creaturePath, creatureAssignmentPath, depositPath;
string dirbase;


const string oldRmSheetType = "item";
const string rmSheetType = "sitem";
const string crSheetType = "creature";
const string dpSheetType = "deposit";


//const uint32 NbPropertySlots = 10; // obsolete
uint32 UndefinedProperty = ~0;



/*
 *
 */
struct CIconInfo
{
	CSString	IconBackground, Icon, IconOver, IconOver2;
};

map< CSString, CIconInfo, CUnsensitiveSStringLessPred > Icons;


/*
 *
 */
class CMainStat
{
public:

	///
	CMainStat() : SumNbFaberElemsFilled(0), MaxNbFaberElemsFilled(0), MinNbFaberElemsFilled(~0),
		NbRMByFaberElem( NbFaberElements, 0 ) {}

	/// Call it when families etc. are ready
	void	init()
	{
		NbRMByFaberElemByFamilyAndCiv.resize( families.size() );
		for ( uint32 i=0; i!=families.size(); ++i )
		{
			NbRMByFaberElemByFamilyAndCiv[i].resize( NbCiv );
			for ( uint32 c=0; c!=NbCiv; ++c )
				NbRMByFaberElemByFamilyAndCiv[i][c].resize( NbFaberElements );
		}

		for ( uint32 iEcosystem=0; iEcosystem!=ecosystems.size(); ++iEcosystem )
		{
			for ( uint32 iLoc=0; iLoc!=NB_LOCATIONS; ++iLoc )
			{
				NbRMHavingProperty[iLoc][iEcosystem].resize( properties.size() );
				NbRMByFaberElemByEcosystem[iEcosystem].resize( NbFaberElements );
			}
		}
	}

	///
	bool	updateCraftStatistics( uint32 rFaberElem, uint32 iEcosystem, uint32 iFam, TCiv civ );

	///
	void	updateMainStats( uint32 nbFaberElemsFilled )
	{
		SumNbFaberElemsFilled += nbFaberElemsFilled;
		if ( nbFaberElemsFilled < MinNbFaberElemsFilled )
			MinNbFaberElemsFilled = nbFaberElemsFilled;
		if ( nbFaberElemsFilled> MaxNbFaberElemsFilled )
			MaxNbFaberElemsFilled = nbFaberElemsFilled;
	}

	// By property and by ecosystem and location
	vu NbRMHavingProperty [NB_LOCATIONS][NbEcosystems];

	uint32 SumNbFaberElemsFilled;
	uint32 MaxNbFaberElemsFilled;
	uint32 MinNbFaberElemsFilled;

	vector<uint32> NbRMByFaberElem;;
	vector<uint32> NbRMByFaberElemByEcosystem [NbEcosystems];
	vector< vector < vector< uint32 > > > NbRMByFaberElemByFamilyAndCiv;

};


/*
 * From georges2csv
 */
void	loadSheetPath()
{
	if (inputSheetPathLoaded)
		return;

	NLMISC::createDebug();
	NLMISC::WarningLog->addNegativeFilter( "CPath::insertFileInMap" );

	CPath::addSearchPath(inputSheetPath, true, false); // for Georges to work properly

	vector<string>	files;
	CPath::getPathContent (inputSheetPath, true, false, true, files);

	uint32	i;
	for (i=0; i<files.size(); ++i)
	{
		string	filename = files[i];
		string	filebase = CFile::getFilenameWithoutExtension(filename);
		inputSheetPathContent[filebase] = filename;
	}

	inputSheetPathLoaded = true;
}


/*
 * Get a random value in range [0..nbPossibleValues[
 * Precondition: nbPossibleValues < 65536
 */
inline uint32 getRandomValue( uint32 nbPossibleValues )
{
	/*double	r = (double) rand();
	r/= (double) (RAND_MAX+1); // exclude the ceiling value
	return (uint32)(r * nbPossibleValues);*/
	return RandomGenerator.rand( (uint16)(nbPossibleValues-1) );
}


/*
 *
 */
struct CDfnFieldInfo
{
	CDfnFieldInfo() {}
	CDfnFieldInfo( const vector<string>& values, const vector<string>& labels ) : TypePredefinedValues(values), TypePredefinedLabels(labels) {}

	vector<string>	TypePredefinedValues;
	vector<string>	TypePredefinedLabels;
};


/*
 *
 */
struct TIconMapping
{
	const char *FamilyName;
	const char *IconFilename;
};


/*
 *
 */
sint getNomenclaturedInterestLevel( TFaberInterestLevel level, TFaberInterestLevel nbInterestLevels )
{
	return (level == NAInterestLevel) ? 0 : (level * NbNomenclaturedFaberLevel / nbInterestLevels) + 1;
}


/*
 *
 */
//struct CFaberCode
//{
//	char Ch[NB_FABERELEMS_CODE_CHARS];
//};


/*
 *
 */
//struct CFaberCombination
//{
//	CFaberCombination( TFaberInterestLevel firstLevel, const string& code ) : FirstLevel(firstLevel)
//	{
//		memcpy( Code.Ch, &code[0], NB_FABERELEMS_CODE_CHARS );
//	}
//
//	TFaberInterestLevel	FirstLevel;
//	CFaberCode			Code;
//};


/*
 *
 */
//class CSheetNameRepository
//{
//public:
//
//	///
//	void	resize( uint32 nbEcosystems, uint32 nbFamilies )
//	{
//		_Container.resize( nbEcosystems );
//		for ( uint32 i=0; i!=nbEcosystems; ++i )
//		{
//			_Container[i].resize( nbFamilies );
//		}
//	}
//
//	///
//	void	insert( uint32 iEcosystem, uint32 iFamily, uint32 iCreature, TFaberInterestLevel level, const string& faberCombinationCode )
//	{
//		// nlassert( faberCombinationCode.size() == NB_FABERELEMS_CODE_CHARS );
//		_Container[iEcosystem][iFamily][iCreature].push_back( CFaberCombination( level, faberCombinationCode ) );
//	}
//
//	///
//	void	getFaberCombinationCodes( uint32 iEcosystem, uint32 iFamily, uint32 iCreature, vector<CFaberCombination> **codes )
//	{
//		map < uint32, vector< CFaberCombination > >::iterator im;
//		/*nldebug( "%u %u -> %u cr (searching for %u)", iEcosystem, iFamily, _Container[iEcosystem][iFamily].size(), iCreature );
//		for ( im=_Container[iEcosystem][iFamily].begin(); im!=_Container[iEcosystem][iFamily].end(); ++im )
//			nldebug( "cr %u -- %u combinations", (*im).first, (*im).second.size() );*/
//		im = _Container[iEcosystem][iFamily].find( iCreature );
//		if ( im == _Container[iEcosystem][iFamily].end() )
//			*codes = NULL;
//		else
//			*codes = &((im)->second);
//	}
//
//private:
//
//	/// Indexs: iEcosystem, iFamily, rCreatureSpecialization, rFaberCombination
//	vector< vector < map < uint32, vector< CFaberCombination > > > >	_Container;
//};


//void	CSheetNameRepository::getFaberCombinationCodes( uint32 iEcosystem, uint32 iFamily, uint32 iCreature, vector<CFaberCombination> **codes )


//CSheetNameRepository RawMaterialRepository;

/**
 * Characteristics.
 * When adding a new characteristic, ADD IT INTO "v3_source_tables.xls!Item Parts v3"/"rm_item_parts.csv" and mark compatible item parts
 */
enum	TFaberCharacteristic {
	Durability, Weight, SapLoad, DMG, Speed, Range,
	DodgeModifier, ParryModifier, AdversaryDodgeModifier, AdversaryParryModifier,
	ProtectionFactor, MaxSlashProtect, MaxBluntProtect, MaxPierceProtect,
	ECTF, EPF,
	OACTF, OAPF,
	HCTP, HPF,
	DACTF, DAPF, 
	JewelProtection,
	CraftCivSpec,
	NbCharacs };

const char *sCharacs [NbCharacs] = {
	"Durability", "Weight", "SapLoad", "DMG", "Speed", "Range",
	"DodgeModifier", "ParryModifier", "AdversaryDodgeModifier", "AdversaryParryModifier",
	"ProtectionFactor", "MaxSlashingProtection", "MaxBluntProtection", "MaxPiercingProtection",
	"ElementalCastingTimeFactor", "ElementalPowerFactor",
	"OffensiveAfflictionCastingTimeFactor", "OffensiveAfflictionPowerFactor",
	"HealCastingTimeFactor", "HealPowerFactor",
	"DefensiveAfflictionCastingTimeFactor", "DefensiveAfflictionPowerFactor",
	"JewelProtectionType",
	"CraftCivSpec" };

//const bool PositiveCharacs [NbCharacs] = {	true,	false,	true,	false,	true,	true };
//const float MinCharacValues [NbCharacs] = {	100,	0.1f,	10,		0.2f,	100,	0 };
//const float MaxCharacValues [NbCharacs] = {	500,	1.5f,	400,	2.0f,	500,	60 };
//const float PeakCharacValues [NbCharacs] = {	2000,	2.5f,	800,	5.0f,	2000,	80 };
bool PositiveCharacs [NbCharacs];
float MinCharacValues [NbCharacs];
float MaxCharacValues [NbCharacs];
float PeakCharacValues [NbCharacs];
vector<bool> CharacSlotFilter [NbCharacs]; // it's a positive filter



/*
 *
 */
struct TFamInfo
{
	vs				Properties;				// ex: propForA, propForB, propForC
	CSString		CompatibleCraftParts;	// ex: ABC
	vu				CraftPartsByProp;		// ex: 0, 1, 2 (indices in ompatibleCraftParts)
	vector<TCiv>	Civs;					// ex: Fyros, All, All
	vu				Freqs;					// ex: 1, 2, 2
	TGroup			Group;
	bool			IsActive;					// False if not in rm_fam_prop.csv
	bool			IsInDeposits;
	bool			IsInCreatures;
	CSString			SpecialCreatureTag;
	bool			GenerateOnly;
	bool			IsForMission;
	sint8			RemarkableStatIndex [NB_REMARKABLE_STAT_INDICES];
	sint8			ColorIndex;
	sint8			JewelProtIndex;

	static uint		UseGenerateOnly; // if 0, generate all; otherwise, generate only families that have GenerateOnly set to true

	///
	TFamInfo() : Group(~0), IsInDeposits(false), IsActive(false), IsInCreatures(false), SpecialCreatureTag(), GenerateOnly(false), IsForMission(false), ColorIndex(-1), JewelProtIndex(-1) {}

	///
	uint			getCompatibleCraftPart( uint iCompatibleCP ) const
	{
		return (uint)(CompatibleCraftParts[iCompatibleCP] - 'A');
	}

	/// whichProp: index in Properties
	uint			getBeginCraftPartForProp( uint whichProp ) const
	{
		return CraftPartsByProp[whichProp];
	}

	/// whichProp: index in Properties
	uint			getEndCraftPartForProp( uint whichProp ) const
	{
		if ( whichProp == Properties.size()-1 )
			return CompatibleCraftParts.size();
		else
			return CraftPartsByProp[whichProp+1];
	}

	/// whichProp: index in Properties
	CSString		getCraftPartForProp( uint whichProp ) const
	{
		uint start = getBeginCraftPartForProp( whichProp );
		return CompatibleCraftParts.substr( start, getEndCraftPartForProp( whichProp ) - start );
	}

	/** With the returned index, you can get elt in Property, CraftPartsByProp, Civs and Freqs.
	 * Returns ~0 if not found.
	 */
	uint			getPropIndexByCraftPart( char itemPart ) const
	{
		for ( uint i=0; i!=CraftPartsByProp.size(); ++i )
		{
			char itemPartCode [2];
			itemPartCode[0] = itemPart;
			itemPartCode[1] = '\0';
			if ( getCraftPartForProp( i ).find( itemPartCode ) != string::npos )
				return i;
		}
		return ~0;
	}

	///
	bool			existsInEcosystem( TEcosystem iEcosystem, TStatQuality statQuality ) const;

	///
	static bool		mustGenerateFamily( uint iFamily );
};


const uint ITEM_PART_JEWEL_GEM = 17; // R


/*
 *
 */
struct CFaberCharacteristics
{
	/// Default constructor (for reading)
	CFaberCharacteristics()
		: FaberElement(~0), ActualEnergy(0.0f), ActualOriginality(0.0f)
	{
		for ( uint32 i=0; i!=NbCharacs; ++i )
			Values[i] = 0.0f;
	}

	///
	void	serial( NLMISC::IStream& s )
	{
		s.serial( FaberElement );
		s.serial( ActualEnergy );
		s.serial( ActualOriginality );
		for ( uint32 c=0; c!=NbCharacs; ++c )
			s.serial( Values[c] );
	}

	///
	void	initFaberElement( uint32 rFaberElement ) { FaberElement = rFaberElement; }
	
	/// Returns false if the RM must NOT be generated.
	bool	randomizeValues( TFaberInterestLevel interestLevel, TFaberInterestLevel nbInterestLevels, uint iVariant, float widthRatio, float peakOccurRatio, float baseBoost, TEcosystem iEcosystem, uint iFreq, uint iFamily );

	/// Returns -1 if the RM must NOT be generated, otherwise return the stat average
	sint32	computeValues( TStatQuality statQuality, const TFamInfo& famInfo, sint remarkableIndicesSetBaseIndex,
						   TEcosystem iEcosystem, uint iFreq, uint iFamily );

	///
	void	calcQualitativeValues();

	/// Index of faber element in "MpParam"
	uint32	FaberElement;

	/// Values
	float	Values [NbCharacs];

	/// Average of the actual interest of the random values between 0 and 1
	float	ActualEnergy;

	///
	float	ActualOriginality;

protected:

	// Returns false if the RM must NOT be generated.
	//bool	randomizeJewelProtection( TStatQuality statQuality, TEcosystem iEcosystem, uint iFreq, uint iFamily );

	void	computeJewelProtection( TStatQuality statQuality, TEcosystem iEcosystem, uint iFamily );
};


//
void	CFaberCharacteristics::calcQualitativeValues()
{
	float actualInterests [NbCharacs];

	// Calculate the average of ratio between [0,1]
	float sumActualInterest = 0.0f;
	uint nbCharacsUsed = 0;
	for ( uint r=0; r!=NbCharacs; ++r )
	{
		if ( MaxCharacValues[r] == 0 )
			continue;
		if ( ! CharacSlotFilter[r][FaberElement] )
			continue;

		float interestRatio = (Values[r] / PeakCharacValues[r]); // new: Peak is taken as the max => the Energy is in [0..100]
		if ( ! PositiveCharacs[r] )
			interestRatio = 1.0f - interestRatio; // thus, can be negative

		actualInterests[r] = interestRatio;
		sumActualInterest += interestRatio;
		++nbCharacsUsed;
	}

	if ( nbCharacsUsed == 0 )
		return;

	ActualEnergy = (sumActualInterest / (float)nbCharacsUsed);
	if ( ActualEnergy > 1.0f )
		ActualEnergy = 1.0f;

	// Calculate the standard deviation (SQRT(SUM((Ai-Aavg)^2)/N))
	float varianceSum = 0.0f;
	for ( uint r=0; r!=NbCharacs; ++r )
	{
		if ( MaxCharacValues[r] == 0 )
			continue;
		if ( ! CharacSlotFilter[r][FaberElement] )
			continue;

		varianceSum += sqr( actualInterests[r] - ActualEnergy );
	}

	// Don't normalize standard deviation, otherwise low energy materials will be considered more
	// original (by average) than high energy materials. They wouldn't be comparable.

	//if ( ActualEnergy != 0.0f )
		ActualOriginality = (float)sqrt( (double)(varianceSum / (float)nbCharacsUsed) ); // / ActualEnergy;
	//else
	//	nlinfo( "Null energy for craft slot %u", FaberElement );
}


/*
 * Fill childrenToGet if rootNameForGetChildren is not null
 */
void fillFromDFN( UFormLoader *formLoader, map<string, CDfnFieldInfo>& dfnFields, UFormDfn *formDfn, const string& rootName, const string& dfnFilename,
				  const char *rootNameForGetChildren=NULL, vector<string>& childrenToGet=vector<string>() )
{
	uint32 i;
	for ( i=0; i!=formDfn->getNumEntry(); ++i )
	{
		string entryName, rootBase;
		formDfn->getEntryName( i, entryName );
		rootBase = rootName.empty() ? "" : (rootName+".");

		UFormDfn::TEntryType entryType;
		bool array;
		formDfn->getEntryType( i, entryType, array );
		switch ( entryType )
		{
			case UFormDfn::EntryVirtualDfn:
			{
				CSmartPtr<UFormDfn> subFormDfn = formLoader->loadFormDfn( (entryName + ".dfn").c_str() );
				if ( ! subFormDfn )
					nlwarning( "Can't load virtual DFN %s", entryName.c_str() );
				else
					fillFromDFN( formLoader, dfnFields, subFormDfn, rootBase + entryName, entryName + ".dfn", rootNameForGetChildren, childrenToGet ); // recurse
				break;
			}
			case UFormDfn::EntryDfn: // .dfn
			{
				UFormDfn *subFormDfn;
				if ( formDfn->getEntryDfn( i, &subFormDfn) )
				{
					string filename;
					formDfn->getEntryFilename( i, filename );
					fillFromDFN( formLoader, dfnFields, subFormDfn, rootBase + entryName, filename, rootNameForGetChildren, childrenToGet ); // recurse
				}
				if ( rootNameForGetChildren && (rootName == rootNameForGetChildren) )
				{
					childrenToGet.push_back( rootBase + entryName );
				}
				break;
			}
			case UFormDfn::EntryType: // .typ
			{
				vector<string> values, labels;
				UType *subType;
				if ( formDfn->getEntryType( i, &subType ) )
				{
					uint32 listSize = subType->getNumDefinition();
					if ( listSize > 0 )
					{
						string label, value;
						for ( uint32 j=0; j!=listSize; ++j )
						{
							subType->getDefinition( j, label, value );
							if ( (subType->getIncrement() == "1") && (subType->getType() == UType::UnsignedInt || subType->getType() == UType::SignedInt) )
							{
								// Fill blank entry for skipped identifier values (to allow indexing by identifier value)
								sint num = atoi( value.c_str() );
								while ( num - (sint)values.size() > 0 )
								{
									values.push_back( "" );
									labels.push_back( "" );
								}
							}
							values.push_back( value );
							labels.push_back( label );
						}
					}
				}
				dfnFields.insert( make_pair( rootBase + entryName, CDfnFieldInfo(values, labels) ) );
				//nlinfo( "DFN entry: %s (in %s)", (rootBase + entryName).c_str(), dfnFilename.c_str() );
				break;
			}
		}
	}
}


/*
 *
 */
CForm *loadTemplateForm( UFormLoader *formLoader, const string& sheetType )
{
	CForm *form = (CForm*)formLoader->loadForm( (string("_empty.")+sheetType).c_str() );
	if ( ! form )
		nlerror( "Can't load sheet _empty.%s", sheetType.c_str() );
	return form;
}


/*
 *
 */
void eraseCarriageReturns( string& s )
{
	const char CR = '\n';
	string::size_type p = s.find( CR );
	while ( (p=s.find( CR )) != string::npos )
		s.erase( p, 1 );
}


/*
 *
 */
string getNomenclatureCode( const string& longName, set<string>& usedCodes, uint32 nbLetters )
{
	if ( nbLetters > longName.size() )
		nlerror( "Wrong nbLetters for %s", longName.c_str() );

	// Start with beginning of name
	string code = strlwr(longName.substr( 0, nbLetters ));
	uint32 i = nbLetters-1;
	while ( usedCodes.find( code ) != usedCodes.end() )
	{
		++i;
		if ( i < longName.size() )
		{
			// Substitute last code char by a char from the name (except ' ')
			if ( longName[i] != ' ' )
				code[nbLetters-1] = tolower(longName[i]);
			else
				continue;
		}
		else
		{
			// If no char from the name is suitable, increment the last char of the code until suitable
			char c=1;
			while ( usedCodes.find( code ) != usedCodes.end() )
			{
				code[nbLetters-1] = tolower(longName[nbLetters-1]) + c;
				++c;
				if ( code[1] > 'z' )
					nlerror( "Impossible to make code for %s", longName.c_str() );
			}
		}
	}
	strlwr( code );
	usedCodes.insert( code );
	return code;
}


/*
 * Displays mapping if title not null.
 */
void buildNomenclatureCodes( const char *title, const vector<string>& longNames, vector<string>& codes, uint32 nbLetters )
{
	set<string> usedCodeSet;
	uint32 i;
	for ( i=0; i!=longNames.size(); ++i )
	{
		codes[i] = getNomenclatureCode( longNames[i], usedCodeSet, nbLetters );
		if ( title )
			nlinfo( "%s %s -> %s", title, longNames[i].c_str(), codes[i].c_str() );
		//DebugLog->displayRawNL( "%s", longNames[i].c_str() );
	}
}


/*
 * Set the size of a family code to NB_FAMILY_CODE_CHARS
 */
void normalizeFamilyCode( std::string& s )
{
	if ( s.size() > NB_FAMILY_CODE_CHARS )
	{
		nlerror( "Family codes limited to %u chars (%s)", NB_FAMILY_CODE_CHARS, s.c_str() );
	}
	else
	{
		uint p = s.size();
		while ( p < NB_FAMILY_CODE_CHARS )
		{
			s = "0" + s;
			++p;
		}
	}
}


/*
 *
 */
void loadNomenclatureCodes( const char *title, const vector<string>& longNames, vector<string>& codes, const char *filename )
{
	if ( longNames.empty() )
	{
		nlwarning( "No nomenclature codes to load. %s", title ? title : "" );
		return;
	}
	codes.resize( longNames.size() );

	char lineBuffer[2048];
	FILE *rulesFile;
	const char *SEPARATOR = ";";
	vector<string> args;
	vector<string>::iterator iarg;
	vector<string>::const_iterator ivs;

	if ( (rulesFile = fopen( filename, "r" )) == NULL )
	{
		nlwarning( "Can't find file %s", filename );
	}
	else
	{
		while ( ! feof(rulesFile) )
		{
			// Get from file
			fgets( lineBuffer, 2048, rulesFile );
			explode( lineBuffer, SEPARATOR, args );

			// Get rid of carriage returns!
			for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
			{
				eraseCarriageReturns( *iarg );
			}

			// Read
			const uint32 MIN_COLS = 6;
			const uint32 NAME_COL = 4;
			const uint32 C_COL = 2;
			const uint32 R_COL = 3;
			const uint32 NB_CODE_CHARS = 2;
			if ( (args.size()>=MIN_COLS) && (! args[0].empty()) && (args[0].find( "name" )==string::npos) ) // skip blank lines, and lines with blank header or "name" in the first column
			{
				if ( args[NAME_COL].empty() )
					continue;

				ivs = find( longNames.begin(), longNames.end(), args[NAME_COL] );
				if ( ivs == longNames.end() )
					nlwarning( "Name %s is not in the names array", args[NAME_COL].c_str() );
				else
				{
					string code = args[C_COL] + args[R_COL];
					if ( code.size() < NB_CODE_CHARS )
					{
						nlwarning( "Invalid partial code for %s: %s", (*ivs).c_str(), code.c_str() );
						continue;
					}
					else if ( code.size() > NB_CODE_CHARS )
					{
						nlinfo( "Compacting code '%s' for %s", code.c_str(), (*ivs).c_str() );
						string::size_type p;
						while ( (p = code.find( ' ' )) != string::npos )
						{
							code.erase( p, 1 );
						}
					}

					if ( codes[ivs-longNames.begin()].empty() )
					{
						if ( title )
							nlinfo( "%s %s -> %s", title, (*ivs).c_str(), code.c_str() );
						codes[ivs-longNames.begin()] = code;
					}
					else
					{
						if ( code != codes[ivs-longNames.begin()] )
							nlwarning( "Invalid nomenclature: (%s and %s for %s: ", codes[ivs-longNames.begin()].c_str(), code.c_str(), (*ivs).c_str() );
					}
				}
			}
		}

		for ( ivs=codes.begin(); ivs!=codes.end(); ++ivs )
		{
			if ( (*ivs).empty() )
				nlwarning( "No code found for %s", (*(longNames.begin() + (ivs - codes.begin()))).c_str() );
		}
	}
}


/*
 *
 */
inline sint getLastUsedPropertySlot( uint32 *iProperties, sint lastPropertySlot, uint32 undefinedProperty )
{
	for ( sint r=lastPropertySlot; r>=0; --r )
	{
		if ( iProperties[r] != undefinedProperty )
			return r;
	}
	return -1;
}


/*
 *
 */
inline bool passNegativeFilter( const vector<uint32>& incompatibilityList, uint32 iValue )
{
	vector<uint32>::const_iterator ip = find( incompatibilityList.begin(), incompatibilityList.end(), iValue );	
	return (ip == incompatibilityList.end());
}


/*
 *
 */
inline bool passPositiveFilter( const vector<uint32>& compatibilityList, uint32 iValue )
{
	vector<uint32>::const_iterator ip = find( compatibilityList.begin(), compatibilityList.end(), iValue );	
	return (ip != compatibilityList.end());
}


/*
 * Reject a prop if it is in the incompatibility list of a family
 */
bool passPropFamilyFilter( const vector<uint32>& iFamilyRelatedProperties, uint32 iProp )
{
	return passNegativeFilter( iFamilyRelatedProperties, iProp );
}


/*
 * Reject a creature if NOT in the creature list of a family
 */
/*bool passCreatureFilter( const vector<uint32>& iFamilyRelatedCreatures, uint32 iCreature )
{
	//nldebug( "%u related creatures, %s", iFamilyRelatedCreatures.size(), passPositiveFilter( iFamilyRelatedCreatures, iCreature ) ? "TRUE": "FALSE" );
	return passPositiveFilter( iFamilyRelatedCreatures, iCreature );
}*/


/*
 *
 */
class CStrIComparator : public binary_function<string, string, bool>
{
public:
	bool	operator() ( const string& s1, const string& s2 ) const
	{
		return (nlstricmp( s1, s2 ) == 0);
	}
};


/*
 *
 */
void displayList( const vector<string>& v, CLog *log=DebugLog )
{
	vector<string>::const_iterator ist;
	for ( ist=v.begin(); ist!=v.end(); ++ist )
		log->displayRaw( "%s ", (*ist).c_str() );
	log->displayRawNL( "" );
}


/*
 *
 */
uint32 getIndexFromString( const string& s, const vector<string>& v, bool displayWarning=true )
{
	if ( v.empty() )
	{
		if ( displayWarning )
			nlwarning( "Can't find '%s' in empty array", s.c_str() );
		return ~0;
	}
	else
	{
		vector<string>::const_iterator ist = find_if( v.begin(), v.end(), bind2nd(CStrIComparator(), s) );
		if ( ist == v.end() )
		{
			if ( displayWarning )
			{
				nlwarning( "Can't find '%s' in:", s.c_str() );
				displayList( v, WarningLog );
			}
			return ~0;
		}
		else
			return ist - v.begin();
	}
}


/*
 *
 */
uint32 getIndexFromString( const string& s, const char **array, uint arraySize, bool displayWarning=true )
{
	if ( arraySize == 0 )
	{
		if ( displayWarning )
			nlwarning( "Can't find '%s' in empty array", s.c_str() );
		return ~0;
	}
	else
	{
		for ( uint i=0; i!=arraySize; ++i )
		{
			if ( strlwr(string(array[i])) == strlwr(s) )
				return i;
		}

		if ( displayWarning )
		{
			nlwarning( "Can't find '%s' in:", s.c_str() );
			//displayList( v, WarningLog );
		}
		return ~0;
	}
}


/*
 * Returns the index of the erased element, ~0 if not found
 */
uint32 removeEntryFromList( vector<string>& v, const string& entry )
{
	vector<string>::iterator ivs;
	ivs = find( v.begin(), v.end(), entry );
	uint32 index;
	if ( ivs != v.end() )
	{
		index = ivs - v.begin();
		v.erase( ivs );
		return index;
	}
	else
		return ~0;
}


/*
 *
 */
bool removeEntryFromListByIndex( vector<string>& v, uint32 index )
{
	if ( index < v.size() )
	{
		v.erase( v.begin() + index );
		return true;
	}
	else
		return false;
}


typedef void (*TMapDeliveryCallback) ( mss& );
typedef void (*TVectorDeliveryCallback) ( vs& );


/*
 *
 */
void	loadCSVFile( const char *filename, TMapDeliveryCallback deliveryCallback, bool firstColWithoutName=false )
{
	char lineBuffer[2048];
	FILE *file;
	const char *SEPARATOR = ";";
	vector<string> args;
	vector<string>::iterator iarg;

	if ( (file = fopen( filename, "r" )) == NULL )
	{
		nlwarning( "Can't find file %s", filename );
	}
	else
	{
		// Read first line as header with column names
		lineBuffer[0] = '\0';
		fgets( lineBuffer, 2048, file );
		explode( lineBuffer, SEPARATOR, args );

		// Store column names (and get rid of carriage returns!)
		vector < string > columnNames;
		mss valuesByName;
		for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
		{
			if ( firstColWithoutName && (iarg == args.begin()) )
			{
				*iarg = "<>"; // override column name for the 1st column
			}
			eraseCarriageReturns( *iarg );
			columnNames.push_back( *iarg );
			valuesByName.insert( make_pair( *iarg, string("") ) );
		}

		while ( ! feof(file) )
		{
			// Get from file
			lineBuffer[0] = '\0';
			fgets( lineBuffer, 2048, file );
			explode( lineBuffer, SEPARATOR, args );

			// Set values (and get rid of carriage returns!)
			for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
			{
				eraseCarriageReturns( *iarg );
				valuesByName[columnNames[iarg-args.begin()]] = *iarg;
			}

			// Deliver the wanted fields
			deliveryCallback( valuesByName );
		}
	}
}


/*
 *
 */
void	loadCSVFile( const char *filename, TVectorDeliveryCallback deliveryCallback )
{
	char lineBuffer[2048];
	FILE *file;
	const char *SEPARATOR = ";";
	vs args;
	vs::iterator iarg;

	if ( (file = fopen( filename, "r" )) == NULL )
	{
		nlwarning( "Can't find file %s", filename );
	}
	else
	{
		while ( ! feof(file) )
		{
			// Get from file
			lineBuffer[0] = '\0';
			fgets( lineBuffer, 2048, file );
			explode( lineBuffer, SEPARATOR, args );

			// Get rid of carriage returns!
			for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
			{
				eraseCarriageReturns( *iarg );
			}

			// Deliver the wanted fields
			deliveryCallback( args );
		}
	}
}


/*
 *
 */
void loadValueFile( const char *filename, const vector<string>& keyStrings,
					vector<sint>& contents, sint defaultValue )
{
	nlassert( keyStrings.size() == contents.size() );
	char lineBuffer[2048];
	FILE *rulesFile;
	const char *SEPARATOR = ";";
	vector<string> args;
	vector<string>::iterator iarg;

	if ( (rulesFile = fopen( filename, "r" )) == NULL )
	{
		nlwarning( "Can't find file %s", filename );
	}
	else
	{
		while ( ! feof(rulesFile) )
		{
			// Get from file
			lineBuffer[0] = '\0';
			fgets( lineBuffer, 2048, rulesFile );
			explode( lineBuffer, SEPARATOR, args );

			// Get rid of carriage returns!
			for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
			{
				eraseCarriageReturns( *iarg );
			}

			// Read
			if ( (! args.empty()) && (! args[0].empty()) ) // skip blank lines, and lines with blank header
			{
				sint value = defaultValue;
				for ( uint32 a=0; a!=args.size()-1; ++a )
				{
					if ( ! args[a+1].empty() ) // skip blank entries
						value = atoi( args[a+1].c_str() );
				}
				uint32 index = getIndexFromString( args[0], keyStrings );
				if ( index != ~0 )
				{
					contents[index] = value;
				} 
			}
		}
		fclose( rulesFile );
	}
}


/*
 *
 */
void loadRulesFile( const char *filename, const vector<string>& keyStrings,
				    const vector<string>& contentStrings, CRulesFilter& filter,
					const string& matchExtKeyAtFirstColumn=string() )
{
	char lineBuffer[2048];
	FILE *rulesFile;
	const char *SEPARATOR = ";";
	uint32 firstColumn = matchExtKeyAtFirstColumn.empty() ? 0 : 1;
	vector<string> args;
	vector<string>::iterator iarg;

	if ( (rulesFile = fopen( filename, "r" )) == NULL )
	{
		nlwarning( "Can't find file %s", filename );
	}
	else
	{
		while ( ! feof(rulesFile) )
		{
			// Get from file
			lineBuffer[0] = '\0';
			fgets( lineBuffer, 2048, rulesFile );
			explode( lineBuffer, SEPARATOR, args );

			// Get rid of carriage returns!
			for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
			{
				eraseCarriageReturns( *iarg );
			}

			// Match with ext key string if set
			if ( (! matchExtKeyAtFirstColumn.empty()) && (args[0]!=matchExtKeyAtFirstColumn) )
				continue;

			// Read
			if ( (! args.empty()) && (! args[firstColumn].empty()) ) // skip blank lines, and lines with blank header
			{
				vector<uint32> contents;
				for ( uint32 a=firstColumn; a!=args.size()-1; ++a )
				{
					if ( ! args[a+1].empty() ) // skip blank entries
						contents.push_back( getIndexFromString( args[a+1], contentStrings ) );
				}
				filter.insert( make_pair( getIndexFromString( args[firstColumn], keyStrings ), contents ) );
			}
		}
		fclose( rulesFile );
	}
}


/*
 * 1st column: extKeyStrings (corresponding to the filters 'vector'); 2nd: keyStrings
 */
void loadRulesFileMulti( const char *filename, const vector<string>& extKeyStrings, const vector<string>& keyStrings, const vector<string>& contentStrings, vector<CRulesFilter>& filters )
{
	filters.resize( extKeyStrings.size() );
	for ( uint32 i=0; i!=filters.size(); ++i )
	{
		loadRulesFile( filename, keyStrings, contentStrings, filters[i], extKeyStrings[i] );
		/*CRulesFilter::const_iterator irf;
		nldebug( "%s", extKeyStrings[i].c_str() );
		for ( irf=filters[i].begin(); irf!=filters[i].end(); ++irf )
		{
			nldebug( "%s", keyStrings[(*irf).first].c_str() );
			vector<uint32>::const_iterator ivi;
			for ( ivi=(*irf).second.begin(); ivi!=(*irf).second.end(); ++ivi )
			{
				nldebug( "%u", *ivi );
			}
		}*/
	}
}


/*
 * Clear the form to reuse it (and all contents below node)
 */
void clearSheet( CForm *form, UFormElm* node )
{
	((CFormElm*)node)->clean();
	form->clean();
}


/*
 * Saves to disk if bool WriteSheetsToDisk is true 
 */
void flushSheetToDisk( const string& fullFilename, UForm *form )
{
	if ( WriteSheetsToDisk )
	{
		COFile output( fullFilename );
		form->write( output, false );
	}
}


/*
 *
 */
string::size_type findCapital( const string& s, string::size_type startPos )
{
	string::size_type p;
	for ( p=startPos; p!=s.size(); ++p )
	{
		if ( (s[p] >= 'A') && (s[p] <= 'Z') )
			return p;
	}
	return string::npos;
}


/*
 * Transform "MyString " into "My string"
 */
void detachValue( string& s )
{
	if ( s.size() < 2 )
		return;

	string::size_type p;
	while ( (p = findCapital( s, 1 )) != string::npos )
	{
		s.insert( p, " " );
		s[p+1] = tolower( s[p+1] );
	}

	// Rip off any blank at the end
	if ( s[s.size()-1] == ' ' )
	{
		s.resize( s.size()-1 );
	}
}


/*
 *
 */
void getTransposedMap( CRulesFilter& dest, const CRulesFilter& src )
{
	CRulesFilter::const_iterator im;
	for ( im=src.begin(); im!=src.end(); ++im )
	{
		vector<uint32>::const_iterator iv;
		for ( iv=(*im).second.begin(); iv!=(*im).second.end(); ++iv )
		{
			dest[*iv].push_back( (*im).first );
		}
	}
}


/*
 *
 */
string makeFaberElementCode( uint32 iFaberElement, TFaberInterestLevel level, TFaberInterestLevel nbInterestLevels )
{
	return toString( "%c%d", 'a' + iFaberElement, getNomenclaturedInterestLevel( level, nbInterestLevels ) );
}


/*
 *
 */
inline bool hasMatchingFaberLevel( TFaberInterestLevel storedLevel, TFaberInterestLevel submittedLevel )
{
	return storedLevel <= submittedLevel;
}


/*
 *
 */
//void keepOnlyHighestLevel( vector<CFaberCombination*>& codes )
//{
//	nlassert( ! codes.empty() );
//	sint maxLevel = -1;
//	uint32 i;
//	for ( i=0; i!=codes.size(); ++i )
//	{
//		if ( codes[i]->FirstLevel > maxLevel )
//		{
//			maxLevel = codes[i]->FirstLevel;
//		}
//	}
//	vector<CFaberCombination*> remainingCodes;
//	for ( i=0; i!=codes.size(); ++i )
//	{
//		if ( codes[i]->FirstLevel == maxLevel )
//			remainingCodes.push_back( codes[i] );
//	}
//
//	//nldebug( "%u codes, highest level = %u with %u occurences", codes.size(), maxLevel, remainingCodes.size() );
//	//nlassert( remainingCodes.size() <= codes.size() );
//	codes = remainingCodes;
//	nlassert( ! codes.empty() );
//}


/*
 *
 */
bool	allIncludedIn( const vu& subset, const vu& bigset )
{
	vu::const_iterator iv;
	for ( iv=subset.begin(); iv!=subset.end(); ++iv )
	{
		if ( find( bigset.begin(), bigset.end(), *iv ) == bigset.end() )
			return false;
	}
	return true;
}


/*
 *
 */
void loadConfigFlag( CConfigFile& configFile, const char *varTitle, bool &flag )
{
	CConfigFile::CVar *var = configFile.getVarPtr( varTitle );
	if ( var )
		flag = (var->asInt() == 1);
}


/*
 *
 */
string::size_type getCapitalFromPos( const string& s, string::size_type startPos )
{
	//nldebug( "%s %u", s.c_str(), startPos );
	string::size_type p;
	for ( p=startPos; p<s.size(); ++p )
	{
		if ( (s[p] >= 'A') && (s[p] <= 'Z') )
			return p;
	}
	return string::npos;
}


/*
 * Also used to make system_mp filenames.
 * Converts "My Identifier" or "MyIdentifier" to "my_identifier" ("My identifier" to "Myidentifier")
 */
string conventionalDirectory( const string& dirname )
{
	if ( dirname.empty() )
		return "";

	string result = dirname;

	// Remove blanks
	string::size_type p = 0;
	while ( (p = result.find( ' ' )) != string::npos )
	{
		result.erase( p, 1 );
	}

	// Convert capitals to underscores
	result[0] = tolower( result[0] );
	p = 1;
	while ( (p = getCapitalFromPos( result, p )) != string::npos )
	{
		result.insert( p, "_" );
		++p;
		result[p] = tolower( result[p] );
	}
	return result;
}


mss	UniqueRMNamesAndSheetCodeHead;


/*
 *
 */
void	readRMNames( mss& values )
{
	string& name = values["basics.name"];
	if ( ! name.empty() )
	{
		string radix = values["FILE"].substr( 0, 5 );
		UniqueRMNamesAndSheetCodeHead.insert( make_pair( name, radix ) );
	}
}


/*
 *
 */
void	loadTitles( const string& sourceWords, const string& sourceBase, const string& languageCode, CTitles& dest )
{
	STRING_MANAGER::TWorksheet worksheet;
	STRING_MANAGER::loadExcelSheet( TranslationPath + sourceBase + "/" + sourceWords + "_words_" + languageCode + ".txt", worksheet );
	uint cp, cn, nbTitles = 0;
	if ( worksheet.findCol( ucstring(sourceWords + " ID"), cp ) && worksheet.findCol( ucstring("name"), cn ) )
	{
		for ( std::vector<STRING_MANAGER::TWorksheet::TRow>::iterator ip = worksheet.begin(); ip!=worksheet.end(); ++ip )
		{
			if ( ip == worksheet.begin() ) // skip first row
				continue;
			STRING_MANAGER::TWorksheet::TRow& row = *ip;
			dest.insert( make_pair( row[cp].toString(), row[cn].toUtf8() ) );
			++nbTitles;
		}
	}
	else
		nlwarning( "%s ID or name not found", sourceWords.c_str() );

	nlinfo( "Loaded %u %s titles", nbTitles, sourceWords.c_str() );
}


/*
 *
 */
void	extractRawMaterialNames()
{
	loadCSVFile( ExtractNamesCsv.c_str(), readRMNames );
	FILE *output = fopen( (CFile::getFilenameWithoutExtension( ExtractNamesCsv ) + "_output.csv").c_str(), "wt" );
	fprintf( output, "Code;Name\n" );
	for ( mss::const_iterator iun=UniqueRMNamesAndSheetCodeHead.begin(); iun!=UniqueRMNamesAndSheetCodeHead.end(); ++iun )
	{
		const string& codeRadix = (*iun).second;
		const string& name = (*iun).first;
		fprintf( output, "%s;%s\n", codeRadix.c_str(), name.c_str() );
	}
}


/*
 *
 */
void	cleanExteriorWhitespace( vs& line )
{
	for ( vs::iterator it=line.begin(); it!=line.end(); ++it )
	{
		CSString& s = (*it);
		string::size_type p;
		for ( p=0; p!=s.size(); ++p )
		{
			if ( s[p] != ' ' )
				break;
		}
		if ( (p != 0) && (p != s.size()) )
			s = s.substr( p );

		for ( p=0; p!=s.size(); ++p )
		{
			if ( s[s.size()-1-p] != ' ' )
				break;
		}
		if ( (p != 0) && (p != s.size()) )
			s = s.rightCrop( p );
	}
}


uint TFamInfo::UseGenerateOnly = 0;

// Only used for deposits; for creature, works with the creature sheets found
/*bool TFamInfo::existsInEcosystem( TEcosystem iEcosystem, TStatQuality statQuality ) const
{
	switch ( iEcosystem )
	{
	case CommonEcosystem: // The Common rm exists if the rm family has a freq=2 (or 0 but only in Supreme)
		return ( (find( Freqs.begin(), Freqs.end(), 2 ) != Freqs.end())
			  || ((find( Freqs.begin(), Freqs.end(), 0 ) != Freqs.end()) && (statQuality == Supreme)) );
			  // was: find_if ... bind2nd( equals<uint>(), 1 )
		break;
	case PrimeRoots: // Only freq 1 families exist if the PrimeRoots
		return find( Freqs.begin(), Freqs.end(), 1 ) != Freqs.end();
		break;
	default: // A rm family exists in the ecosystem matching a civ if the corresponding freq is 1 or 3
		{
		uint iCiv = getIndexFromString( ecosystemCodes[iEcosystem], CivEcosystemCodes, NbCiv, false );
		vector<TCiv>::const_iterator it = find( Civs.begin(), Civs.end(), (TCiv)iCiv );
		if ( it != Civs.end() )
			return (Freqs[it-Civs.begin()] == 1) || (Freqs[it-Civs.begin()] == 3);
		else
			return false;
		}
	}
}*/

// Only used for deposits;
bool TFamInfo::existsInEcosystem( TEcosystem iEcosystem, TStatQuality statQuality ) const
{
	if ( find( Freqs.begin(), Freqs.end(), 0 ) != Freqs.end() )
	{
		// Freq 0 => only Common/Supreme
		return (statQuality == Supreme) && (iEcosystem == CommonEcosystem);
	}
	else if ( statQuality <= Fine )
	{
		// Basic, Fine => Common
		return (iEcosystem == CommonEcosystem);
	}
	else
	{
		// Choice to Supreme => One per ecosystem
		return (iEcosystem != CommonEcosystem) && (iEcosystem < NbEcosystems);
	}
}


/*
 *
 */
struct TCraftPartInfo
{
	CSString		Name;
	CSString		Path;
	uint8			PartIndex;
	bool			Enabled;
};


/*
 *
 */
class CCraftParts
{
public:

	///
	CCraftParts() : CraftParts( NbFaberElements )
	{
		for ( uint i=0; i!=NbFaberElements; ++i )
		{
			CraftParts[i].PartIndex = i;
			CraftParts[i].Enabled = false;
		}
	}

	///
	void	registerPartChars( const CSString& parts )
	{
		for ( string::size_type p=0; p!=parts.size(); ++p )
		{
			uint index = (uint)(parts[p] - 'A');
			CraftParts[index].Enabled = true;
		}
	}

	///
	bool	isEnabled( uint index ) const
	{
		return CraftParts[index].Enabled;
	}

	///
	void	getNamesAndPaths( const vector<string>& paths )
	{
		uint i = 0;
		vector<string>::const_iterator ip;
		for ( ip=paths.begin(); ip!=paths.end(); ++ip )
		{
			if ( i >= CraftParts.size() )
				nlerror( "Mismatch between sitem DFN and constant (nb of craft parts)" );

			CraftParts[i].Path = (*ip);
			string::size_type p = (*ip).rfind( '.' ) + 1; // string::npos+1 gives 0
			CraftParts[i].Name = (*ip).substr( p );
			nldebug( "%u: %s", ip-paths.begin(), CraftParts[i].Name.c_str() );
			++i;
		}
	}

	///
	TCraftPartInfo&	operator[] ( uint index ) { return CraftParts[index]; }

	vector< TCraftPartInfo > CraftParts;

};

typedef map<CSString, TFamInfo, CUnsensitiveSStringLessPred > CFamMap;
CFamMap		FamSet;
CCraftParts CraftParts;

enum TFamAndPropLine {
	LFam, LGroup, LCraftParts, LCiv, LFreq, LLoc,
	LIconMain, LIconBk, LIconOv1, LIconOv2, LIconSpecial,
	LCraftPlans, LGenerateOnly,
	LBaseOfRemarkableStatIndices,
	LColorIndex = LBaseOfRemarkableStatIndices + NB_REMARKABLE_STAT_INDICES,
	LJewelProtIndex,
	NbFamAndPropCols };


// static
bool		TFamInfo::mustGenerateFamily( uint iFamily )
{
	if ( families[iFamily].empty() )
		return false;
	else if ( ! TFamInfo::UseGenerateOnly )
		return true;
	else
	{
		TFamInfo& famInfo = FamSet[families[iFamily]];
		return ( famInfo.GenerateOnly );
	}
}


/*
 *
 */
CSString	getShortFaberElemString( uint rFaberElem )
{
	string& longString = CraftParts[rFaberElem].Name;
	return reinterpret_cast<CSString&>(longString.substr( longString.find( "(" ) + 1 )).rightCrop( 1 );
}


/*
 *
 */
TCiv getCivFromStr( const CSString& civStr )
{
	if ( civStr.empty() )
		return AllCiv;
	else
	{
		for ( uint i=0; i!=NbCiv; ++i )
		{
			if ( civStr == CSString(CivNames[i]) )
				return (TCiv)i;
		}
		nlwarning( "Unknown civ '%s'", civStr.c_str() );
		return AllCiv;
	}
}


/*
 *
 */
uint getFreqFromStr( const CSString& freqStr )
{
	uint f = atoi( freqStr.c_str() );
	if ( (f < 1) && (f > 5) )
		nlwarning( "Unknown freq '%s'", freqStr.c_str() );
	return f;
}


/*
 * Returns ~0 if s is empty
 */
TGroup getNewOrExistingGroupFromStr( const CSString& s )
{
	uint i = getIndexFromString( s, groups, false );
	if ( i == ~0 )
	{
		if ( s.empty() )
			return ~0;
		else
		{
			i = groups.size();
			groups.push_back( s );
			nlinfo( "New group: %s (%u)", s.c_str(), i );
		}
	}
	return i;
}


/*
 *
 */
void	deliverFamAndProp( vs& line )
{
	if ( line.size() < NbFamAndPropCols )
		line.resize( NbFamAndPropCols );

	cleanExteriorWhitespace( line );

	if ( line[LFam].empty() )
	{
		// Load special icons
		if ( (line.size() >= LIconSpecial+1) && (! line[LIconSpecial].empty()) && (line[LIconMain].empty()) )
		{
			/*if ( line.size() >= LIconMain+1 )
				Icons[line[LIconSpecial]].Icon = line[LIconMain];*/
			if ( line.size() >= LIconBk+1 )
				Icons[line[LIconSpecial]].IconBackground = line[LIconBk];
			if ( line.size() >= LIconOv1+1 )
				Icons[line[LIconSpecial]].IconOver = line[LIconOv1];
		}
		return;
	}

	// Load icons of families
	if ( line.size() >= LIconMain+1 )
	{
		Icons[line[LFam]].Icon = line[LIconMain];
		if ( ! line[LGroup].empty() )
		{
			// For group, set icon of first family of group found! (for forage source knowledge)
			if ( Icons.find( line[LGroup] ) == Icons.end() )
			{
				Icons[line[LGroup]].Icon = line[LIconMain];
			}
		}
	}
	if ( line.size() >= LIconBk+1 )
		Icons[line[LFam]].IconBackground = line[LIconBk];
	if ( line.size() >= LIconOv1+1 )
		Icons[line[LFam]].IconOver = line[LIconOv1];

	TFamInfo& famInfo = FamSet[line[LFam]];
	famInfo.IsActive = true;
	/*if ( ! line[LCraftParts].empty() )
	{
		// Store by property (line[LProp])
		famInfo.Properties.push_back( line[LProp] );
		famInfo.CraftPartsByProp.push_back( famInfo.CompatibleCraftParts.size() ); // beginning of craft parts chars
		famInfo.CompatibleCraftParts += line[LCraftParts];
		CraftParts.registerPartChars( line[LCraftParts] );
		famInfo.Civs.push_back( getCivFromStr( line[LCiv] ) );
		famInfo.Freqs.push_back( getFreqFromStr( line[LFreq] ) );
		famInfo.IsInDeposits = line[LLoc].contains( "D" );
		famInfo.IsInCreatures = line[LLoc].contains( "C" );
		if ( ! (famInfo.IsInDeposits || famInfo.IsInCreatures) )
			nlwarning( "Unknown loc for %s", line[LFam].c_str() );
	}*/

	for ( string::size_type p=0; p!=line[LCraftParts].size(); ++p )
	{
		// Store by property = craft part (each char of line[LCraftParts])
		CSString craftPart = string( 1, line[LCraftParts][p]);
		famInfo.Properties.push_back( craftPart );
		famInfo.CraftPartsByProp.push_back( famInfo.CompatibleCraftParts.size() );
		famInfo.CompatibleCraftParts += craftPart;
		CraftParts.registerPartChars( craftPart );
		famInfo.Civs.push_back( getCivFromStr( line[LCiv] ) );
		famInfo.Freqs.push_back( getFreqFromStr( line[LFreq] ) );
	}
	if ( line[LCraftParts].empty() )
	{
		famInfo.Freqs.push_back( getFreqFromStr( line[LFreq] ) ); // freq needed for Rarity computation
	}
	famInfo.Group = getNewOrExistingGroupFromStr( line[LGroup] );
	famInfo.IsInDeposits = line[LLoc].contains( "D" );
	famInfo.IsInCreatures = line[LLoc].contains( "C" );
	if ( ! (famInfo.IsInDeposits || famInfo.IsInCreatures) )
	{
		famInfo.SpecialCreatureTag = line[LLoc];
		if ( (famInfo.SpecialCreatureTag[0] != 'G') && (famInfo.SpecialCreatureTag[0] != 'I') )
			nlwarning( "Unknown loc %s for %s", line[LLoc].c_str(), line[LFam].c_str() );
	}
	famInfo.IsForMission = line[LCraftParts].empty();
	for ( uint i=0; i!=NB_REMARKABLE_STAT_INDICES; ++i )
	{
		if ( line[LBaseOfRemarkableStatIndices+i].empty() && (! line[LCraftParts].empty()) && (line[LFreq] != "0") )
			nlerror( "%s has empty stat index %u", line[LFam].c_str(), i );
		famInfo.RemarkableStatIndex[i] = atoi( line[LBaseOfRemarkableStatIndices+i].c_str() );
	}
	if ( ! line[LColorIndex].empty() )
		famInfo.ColorIndex = atoi( line[LColorIndex].c_str() );
	if ( ! line[LJewelProtIndex].empty() )
		famInfo.JewelProtIndex = atoi( line[LJewelProtIndex].c_str() );
	bool markedForGeneration = (line[LGenerateOnly] == "X");
	if ( (!markedForGeneration) && famInfo.GenerateOnly )
	{
		nlwarning( "Found duplicate family line with different GenerateOnly setting" );
	}
	else
	{
		famInfo.GenerateOnly = markedForGeneration;
	}
	if ( famInfo.GenerateOnly )
		++TFamInfo::UseGenerateOnly;
}


typedef map< TGroup, set<uint32> > CGroupMap;


/*
 *
 */
void	loadFamAndProp( const string& filename, bool displayAll )
{
	loadCSVFile( filename.c_str(), deliverFamAndProp );

	if ( displayAll )
	{
		set<CSString, CUnsensitiveSStringLessPred> propSet;
		CGroupMap groupMap;

		/// Generate contents of item_mp_family.typ (and fill group map)
		nlinfo( "item_mp_family.typ:" );
		InfoLog->displayRawNL( "<DEFINITION Label=\"Undefined\" Value=\"0\"/>" );
		uint i = 1;
		for ( CFamMap::iterator iss=FamSet.begin(); iss!=FamSet.end(); ++iss )
		{
			const CSString& famStr = (*iss).first;
			TFamInfo& famInfo = (*iss).second;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", famStr.c_str(), i );

			// Get info about props and group
			for ( vs::iterator ip=famInfo.Properties.begin(); ip!=famInfo.Properties.end(); ++ip )
			{
				propSet.insert( *ip );
			}
			groupMap[ famInfo.Group ].insert( i ); // ~0 is for "no group" (creature's RMs only)
			++i;
		}

		/*
		/// Generate family-specialized forage search bricks (TODO)
		nlinfo( "Family-specialized forage search bricks:");
		i = 1;
		for ( CGroupMap::iterator igm=groupMap.begin(); igm!=groupMap.end(); ++igm )
		{
			CSString skill = toString( "SHFM%u", i );
			CSString rmgrpBrickCode = toString( "BHFPMB%02u", i );
			CSString rmfamBrickFamCode = "BHFPMI" + string( 1, (char)'A' + ((char)(i-1)) );
			uint j = 1;
			for ( set<uint32>:::iterator ifs=(*igm).begin(); ifs!=(*igm).end(); ++ifs )
			{
				// TODO: modifier of modifier
				CSString brickCode = rmfamBrickFamCode + toString( "%02u", j );
				InfoLog->displayRawNL( "%s\t80\t%s\t%u\t\t%s\t\tFG_RMFAM_FILT: %u\t", brickCode.c_str(), rmgrpBrickCode.c_str(), j, skill.c_str(), (*ifs) );
				++j;
			}
		}

		/// Generate family-specialized forage search phrases (TODO)
		nlinfo( "Family-specialized forage search phrases:");
		i = 1;
		for ( CFamMap::iterator iss=FamSet.begin(); iss!=FamSet.end(); ++iss )
		{
			const CSString& famStr = (*iss).first;
			TFamInfo& famInfo = (*iss).second;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", famStr.c_str(), i );
		}

		/// Generate family-specialized forage extraction bricks (TODO)
		nlinfo( "Family-specialized forage extraction bricks:");
		i = 1;
		for ( CFamMap::iterator iss=FamSet.begin(); iss!=FamSet.end(); ++iss )
		{
			const CSString& famStr = (*iss).first;
			TFamInfo& famInfo = (*iss).second;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", famStr.c_str(), i );
			++i;
		}

		/// Generate family-specialized forage extraction phrases (TODO)
		nlinfo( "Family-specialized forage extraction phrases:");
		i = 1;
		for ( CFamMap::iterator iss=FamSet.begin(); iss!=FamSet.end(); ++iss )
		{
			const CSString& famStr = (*iss).first;
			TFamInfo& famInfo = (*iss).second;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", famStr.c_str(), i );
			++i;
		}*/

		/// Generate item_mp_property.typ
		nlinfo( "Item parts as props:" );
		InfoLog->displayRawNL( "<DEFINITION Label=\"Undefined\" Value=\"0\"/>" );
		i = 1;
		for ( set<CSString, CUnsensitiveSStringLessPred>::iterator iss=propSet.begin(); iss!=propSet.end(); ++iss )
		{
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", (*iss).c_str(), i );
			++i;
		}

		/// Generate item_mp_group.typ
		nlinfo( "Groups:" );
		InfoLog->displayRawNL( "<DEFINITION Label=\"Undefined\" Value=\"0\"/>" );
		i = 1;
		for ( CGroupMap::iterator igm=groupMap.begin(); igm!=groupMap.end(); ++igm )
		{
			if ( (*igm).first != ~0 )
			{
				InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", (groups[(*igm).first]).c_str(), i );
				++i;
			}
		}

		/*
		/// Generate group-specialized forage search bricks (TODO)
		nlinfo( "Group-specialized forage search bricks:");
		i = 1;
		for ( CGroupMap::iterator igm=groupMap.begin(); igm!=groupMap.end(); ++igm )
		{
			CSString skill = toString( "SHFM%u", i );
			CSString rmgrpBrickCode = toString( "BHFPMB%02u", i );
			++i;
		}

		/// Generate group-specialized forage search phrases
		nlinfo( "Group-specialized forage search phrases:");
		i = 1;
		for ( set<CSString, CUnsensitiveSStringLessPred>::iterator iss=groupSet.begin(); iss!=groupSet.end(); ++iss )
		{
			if ( (*iss).empty() )
				continue;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", (*iss).c_str(), i );
			++i;
		}

		/// Generate group-specialized forage extraction bricks
		nlinfo( "Group-specialized forage extraction bricks:");
		i = 1;
		for ( set<CSString, CUnsensitiveSStringLessPred>::iterator iss=groupSet.begin(); iss!=groupSet.end(); ++iss )
		{
			if ( (*iss).empty() )
				continue;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", (*iss).c_str(), i );
			++i;
		}

		/// Generate group-specialized forage extraction phrases
		nlinfo( "Group-specialized forage extraction phrases:");
		i = 1;
		for ( set<CSString, CUnsensitiveSStringLessPred>::iterator iss=groupSet.begin(); iss!=groupSet.end(); ++iss )
		{
			if ( (*iss).empty() )
				continue;
			InfoLog->displayRawNL( "<DEFINITION Label=\"%s\" Value=\"%u\"/>", (*iss).c_str(), i );
			++i;
		}*/

		nlinfo( "TODO: Keep old values when adding new entries" );
		nlinfo( "Don't forget to regen craft plans and to map localized texts" );
	}
}


/*
 * Multi-indexed array.
 * NC is the number of columns.
 */
template <uint32 NC>
class CSortableData
{
public:

	/// A row is made of fields, usually 1 per column but there may be more than one (each one is a key)
	struct TSortableItem
	{
		std::vector<std::string>		Fields [NC];

		///
		void			push( uint32 column, const std::string& f, bool allowDuplicates=false )
		{
			if ( (allowDuplicates) || (find( Fields[column].begin(), Fields[column].end(), f ) == Fields[column].end()) )
				Fields[column].push_back( f );
		}

		/**
		 * Display the item as a row of a HTML table.
		 * If (key!=previousKey) and (name==previousName), the row will not be displayed entirely to save space
		 *
		 * \param keyColumn If not ~0, column used for sorting => this column displays only the field matching the key
		 * \param key The key used for sorting (see keyColumn)
		 * \param previousKey Previous key
		 * \param nameColumn If not ~0, column used for the unique name (column must have exaclty one element)
		 * \param previousName Previous name
		 */
		std::string		toHTMLRow( uint32 keyColumn=~0, const string& key=string(), const string& previousKey=string(),
								   uint32 nameColumn=~0, const string& previousName=string() ) const
		{
			std::string s = "<tr>";
			bool lightMode = (nameColumn == ~0) ? false : ((key != previousKey) && (Fields[nameColumn][0] == previousName));
			for ( uint32 c=0; c!=NC; ++c )
			{
				s += "<td>";
				if ( c == keyColumn )
					s += key; // key should be a substr of toString( c )
				else
				{
					if ( lightMode )
						s += "\"";
					else
						s += columnToString( c );
				}
				s += "</td>";
			}
			s += "</tr>\n";
			return s;
		}


		///
		std::string		toCSVLine( char columnSeparator=',', string internalSeparator=" - ", uint32 keyColumn=~0, const string& key=string(), const string& previousKey=string(),
								   uint32 nameColumn=~0, const string& previousName=string()  ) const
		{
			std::string s;
			bool lightMode = (nameColumn == ~0) ? false : ((key != previousKey) && (Fields[nameColumn][0] == previousName));
			for ( uint32 c=0; c!=NC; ++c )
			{
				if ( c == keyColumn )
					s += key; // key should be a substr of columnToString( c )
				else
				{
					if ( lightMode )
						s += "\"";
					else
						s += columnToString( c, internalSeparator );
				}
				s += columnSeparator;
			}
			s += "\n";
			return s;
		}

		///
		std::string		columnToString( uint32 column, const std::string& internalSeparator=", " ) const
		{
			std::string s;
			std::vector<std::string>::const_iterator ivs;
			for ( ivs=Fields[column].begin(); ivs!=Fields[column].end(); ++ivs )
			{
				if ( ivs!=Fields[column].begin() )
					s += internalSeparator;
				s += (*ivs);
			}
			return s;
		}
	};

	typedef std::multimap< std::string, uint32 > CLookup; // key to index (not pt because reallocation invalidates pointers)
	typedef std::vector< TSortableItem > CItems;

	/// Init
	void	init( bool enabled )
	{
		_Enabled = enabled;
	}

	/// Add a row
	void	addItem( const TSortableItem& item )
	{
		if ( ! _Enabled )
			return;

		_Items.push_back( item );
		for ( uint32 c=0; c!=NC; ++c )
		{
			for ( std::vector<std::string>::const_iterator ik=item.Fields[c].begin(); ik!=item.Fields[c].end(); ++ik )
			{
				_Indices[c].insert( make_pair( *ik, _Items.size()-1 ) );
			}
		}
	}

	/**
	 * Update a row (found by the first column, which must have exactly one element).
	 * Returns true if it existed before, false if it's being created.
	 * If it existed before:
	 * - Does not remove elements that already exist and are not in the new item
	 * - Adds the new elements found in the new item at the specified columns, and updates lookup map
	 */
	bool	updateItemAppend( const TSortableItem& item, uint32 column )
	{
		if ( ! _Enabled )
			return true; // quiet

		uint32 nameColumn = 0;
		CLookup::iterator ilk = _Indices[nameColumn].find( item.Fields[nameColumn][0] );
		if ( ilk != _Indices[nameColumn].end() )
		{
			uint32& index = (*ilk).second;

			// Update map for the specified column
			// and update item column
			for ( std::vector<std::string>::const_iterator ivs=item.Fields[column].begin(); ivs!=item.Fields[column].end(); ++ivs )
			{
				ilk = _Indices[column].find( *ivs );
				if ( (ilk == _Indices[column].end()) || (  getRow( (*ilk).second ).Fields[nameColumn][0] != item.Fields[nameColumn][0]) )
				{
					_Indices[column].insert( make_pair( *ivs, index ) );
					_Items[index].Fields[column].push_back( *ivs );
				}
			}

			return true;
		}
		else
		{
			addItem( item );
			return false;
		}
	}

	/**
	 * Update a row (found by the first column, which must have exactly one element).
	 * Returns true if it existed before, false if it's being created.
	 * If it existed before:
	 * - Does not update lookup maps or item for columns that were already present.
	 * - Adds entries in lookup maps and updates item for new columns (fields that were empty).
	 */
	/*bool	updateItemAppend( const TSortableItem& item )
	{
		if ( ! _Enabled )
			return true; // quiet

		CLookup::iterator ilk = _Indices[0].find( item.Fields[0][0] );
		if ( ilk != _Indices[0].end() )
		{
			uint32& index = (*ilk).second;

			for ( uint32 c=1; c!=NC; ++c )
			{
				// Update maps for previously empty columns
				if ( _Items[index].Fields[c].empty() )
				{
					for ( std::vector<std::string>::iterator ivs=item.Fields[c].begin(); ivs!=item.Fields[c].end(); ++ivs )
						_Indices[c].insert( make_pair( *ivs, index ) );
				}

				// Update item column
				_Items[index].Fields[c] = item.Fields[c];
			}

			return true;
		}
		else
		{
			addItem( item );
			return false;
		}
	}*/

	/// Find or browse by key
	CLookup& lookup( uint32 column )
	{
		return _Indices[column];
	}

	/// Browse by adding order
	CItems& items()
	{
		return _Items;
	}

	/// Get a row by index
	TSortableItem&	getRow( uint32 index )
	{
		return _Items[index];
	}

private:
	
	CLookup							_Indices [NC];

	CItems							_Items;

	bool							_Enabled;
};


typedef CSortableData<DtNbCols> CRMData;
typedef CRMData::TSortableItem TRMItem;


/**
 *
 */
class CProducedDocHtml
{
public:

	///
	CProducedDocHtml() : _File(NULL), _Enabled(false) {}

	///
	void	open( const std::string& filename, const std::string& title, bool enableFlag )
	{
		_Enabled = enableFlag;
		if ( ! _Enabled )
			return;

		_File = fopen( filename.c_str(), "wt" );
		fprintf( _File, ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>" + title + "</title>\n</head><body>\n").c_str() );
	}

	///
	void	write( const std::string& htmlCode )
	{
		if ( ! _Enabled )
			return;

		fprintf( _File, htmlCode.c_str() );
	}

	///
	void	writeln( const std::string& htmlCode )
	{
		write( htmlCode + "\n" );
	}

	///
	void	writebln( const std::string& htmlCode )
	{
		write( htmlCode + "<br>\n" );
	}

	///
	void	writepln( const std::string& htmlCode )
	{
		write( "<p>" + htmlCode + "</p>\n" );
	}

	///
	void	save()
	{
		if ( ! _Enabled )
			return;

		fprintf( _File, "</body></html>\n" );
		fclose( _File );
	}

private:

	FILE	*_File;
	bool	_Enabled;
};


/**
 *
 */
class CProducedDocCSV
{
public:

	///
	CProducedDocCSV() : _File(NULL), _Enabled(false) {}

	///
	void	open( const std::string& filename, bool enableFlag )
	{
		_Enabled = enableFlag;
		if ( ! _Enabled )
			return;

		_File = fopen( filename.c_str(), "wt" );
	}

	///
	void	write( const std::string& data )
	{
		if ( ! _Enabled )
			return;

		fprintf( _File, data.c_str() );
	}

	///
	void	writeln( const std::string& data )
	{
		write( data + "\n" );
	}

	///
	void	save()
	{
		if ( ! _Enabled )
			return;

		fclose( _File );
	}

private:

	FILE	*_File;
	bool	_Enabled;
};



/**
 *
 */
class CGenRawMaterial
{
public:

	/// Constructor
	CGenRawMaterial( const std::string& sheetName = std::string() ) : SheetName(sheetName), ILocation(~0), IFamily(~0), IEcosystem(NbEcosystems), StatQuality(InvalidStatQuality), Color(InvalidColor), StatEnergyAvg(0)
	{}

	/// Serial
	void		serial( NLMISC::IStream& s )
	{
		s.serial( SheetName );
		s.serial( (uint32&)ILocation );
		s.serial( (uint32&)IFamily );
		s.serial( (uint32&)Group );
		s.serial( (uint32&)IEcosystem );
		s.serial( (uint32&)StatQuality );
		s.serial( (sint32&)Color );
		//s.serial( (uint32&)SapLoadLevel );
		//s.serial( (uint32&)Rarity );
		s.serial( (sint32&)StatEnergyAvg );
		s.serial( (uint32&)MaxLevel );
		s.serialCont( RMProperties );
		s.serialCont( IPropertyDepths );
		s.serialCont( RMCraftCharacs );
	}

	/// Computes randomly RMCraftCharacs, IPropertyDepths... Returns false if the RM must NOT be generated.
	bool	computeCraftCharacs( uint iVariant, const CSString& sheetName );

	///
	void	writeSheet( CForm *form );

	///
	void	loadSheet( CForm *form, const std::string& sheetName, bool full );

	///
	void	collectStats( TRMItem& item, CMainStat& mainStats );

	/// Return average of energies (including max quality as half of the balance)
	/*float	getEnergyAvg() const
	{
		if ( RMCraftCharacs.empty() )
			return 0.0f;
		else
		{
			float sum = 0.0f;
			for ( list<CFaberCharacteristics>::const_iterator ics=RMCraftCharacs.begin(); ics!=RMCraftCharacs.end(); ++ics )
			{
				sum += (*ics).ActualEnergy;
			}
			//return (sum + (float)MaxQuality / 250.0f) / ((float)RMCraftCharacs.size() + 1);
			return (sum / (float)RMCraftCharacs.size()); // now, MaxQuality is not part of the average
		}
	}*/

	///
	float	getOriginalityAvg() const
	{
		float sum = 0.0f;
		for ( list<CFaberCharacteristics>::const_iterator ics=RMCraftCharacs.begin(); ics!=RMCraftCharacs.end(); ++ics )
		{
			sum += (*ics).ActualOriginality;
		}
		return sum / (float)RMCraftCharacs.size();
	}

	///
	float	getOriginalityMax() const
	{
		float maxOriginality = 0.0f;
		for ( list<CFaberCharacteristics>::const_iterator ics=RMCraftCharacs.begin(); ics!=RMCraftCharacs.end(); ++ics )
		{
			if ( (*ics).ActualOriginality > maxOriginality )
				maxOriginality = (*ics).ActualOriginality;
		}
		return maxOriginality;
	}

	///
	void	fillPropertiesFromFamily()
	{
		vs& props = FamSet[familyStr()].Properties;
		RMProperties.resize( props.size() );
		for ( vs::iterator ip=props.begin(); ip!=props.end(); ++ip )
		{
			RMProperties[ip-props.begin()] = getIndexFromString( *ip, properties );
		}
	}

	///
	bool	hasCraftPart( uint craftPartIndex )
	{
		return CraftParts.isEnabled( craftPartIndex ) && (FamSet[familyStr()].CompatibleCraftParts.find( string( 1, (char)'A' + craftPartIndex ).c_str() ) != string::npos);
	}

	///
	/*TCiv	getCivSpec( uint craftPartIndex, const TFamInfo& famInfo )
	{
		TCiv civ = NbCiv;	
		for ( vector<TCiv>::const_iterator ivc=famInfo.Civs.begin(); ivc!=famInfo.Civs.end(); ++ivc )
		{
			// Skip those not matching the current rFaberElem
			if ( famInfo.getCraftPartForProp( ivc-famInfo.Civs.begin() ).find( string( 1, (char)('A' + craftPartIndex) ).c_str() ) != string::npos )
			{
				if ( (civ != NbCiv) && ((*ivc) != civ) )
				{
					nlwarning( "Different civ specializations for %s, %s (%s and %s)", familyStr().c_str(), getShortFaberElemString( craftPartIndex ).c_str(), CivNames[civ], CivNames[*ivc] );
					return AllCiv;
				}
				civ = (*ivc);
			}
		}
		if ( civ == NbCiv )
			return AllCiv;
		else if ( civ = 
			return civ;
	}*/

	///
	TCiv	getCivSpec( TEcosystem iEcosystem, TStatQuality statQuality )
	{
		if ( (statQuality <= Fine) || (iEcosystem >= NbEcosystems) )
			return AllCiv;
		else
			return EcosystemToCiv[iEcosystem];
	}

	///
	/*TCiv	getMainCivSpec( const TFamInfo& famInfo )
	{
		TCiv civ = NbCiv;
		for ( list<CFaberCharacteristics>::const_iterator ics=RMCraftCharacs.begin(); ics!=RMCraftCharacs.end(); ++ics )
		{
			TCiv civOfCraftPart = getCivSpec( (*ics).FaberElement, famInfo );
			if ( (civ != NbCiv) && (civOfCraftPart != civ) )
			{
				return AllCiv;
			}
			civ = civOfCraftPart;
		}
		if ( civ == NbCiv )
			return AllCiv;
		else
			return civ;
	}*/

	///
	/*const char *	getMainEcosystemSpec( const TFamInfo& famInfo )
	{
		return CivEcosystemCodes[getMainCivSpec( famInfo )];
	}*/

	/// Code
	CSString					SheetName;

	/// Index in locations
	uint32						ILocation;

	/// Index in families
	uint32						IFamily;

	///
	CSString					familyStr() const { return families[IFamily]; }

	/// Group number
	TGroup						Group;

	///
	CSString					groupStr() const { return Group==~0 ? "" : groups[Group]; }

	/// Index in ecosystems
	TEcosystem					IEcosystem;

	///
	CSString					ecosystemStr() const { return ecosystems[IEcosystem]; }

	/// From Basic) to Supreme)
	TStatQuality				StatQuality;

	/// From 'b' (Basic) to 'f' (Supreme)
	char						levelZoneLChar() const { return 'b' + (char)StatQuality; }

	/// Same
	void						setStatQuality( char levelZoneChar ) { StatQuality = (TStatQuality)(levelZoneChar - 'b'); }

	/// For creatures
	uint32						ILevelZone;

	/// Index in colors
	TColor						Color;

	///
	CSString					colorStr() const { return colors[Color]; }

	/// Sap load level
	//uint32						SapLoadLevel;

	/// Rarity
	//uint32						Rarity;

	sint32						StatEnergyAvg;

	/// Max quality
	uint32						MaxLevel;

	/// Indices in properties
	vu							RMProperties;

	///
	CSString					propertyStr( uint32 p ) const { return properties[RMProperties[p]]; }

	///
	vu							IPropertyDepths;

	///
	CSString					propertyDepthStr( uint32 p ) const { return PropertyDepths[IPropertyDepths[p]]; }

	///
	CFaberCharacteristics		*getCraftSlot( uint rFaberElem )
	{
		std::list< CFaberCharacteristics >::iterator icl;
		for ( icl=RMCraftCharacs.begin(); icl!=RMCraftCharacs.end(); ++icl )
		{
			if ( (*icl).FaberElement == rFaberElem )
				return &(*icl);
		}
		return NULL;
	}

	/// Randomly generated characs
	std::list< CFaberCharacteristics >	RMCraftCharacs;
};


/**
 *
 */
class COriginalitySorter
{
public:

	typedef std::set< CGenRawMaterial* > CRMSet;
	typedef std::multimap< uint32, CGenRawMaterial*, std::greater<uint32> > CMultiMapByOriginality;

	///
	COriginalitySorter() : RMByOriginalityByCraftSlot( NbFaberElements ) {}

	///
	void	pushRM( CGenRawMaterial *rawMaterial )
	{
		RawMaterials.insert( rawMaterial );

		//InfoLog->displayRawNL( "Inserting RM" );
		std::list< CFaberCharacteristics >::const_iterator ilc;
		for ( ilc=rawMaterial->RMCraftCharacs.begin(); ilc!=rawMaterial->RMCraftCharacs.end(); ++ilc )
		{
			//InfoLog->displayRawNL( "   %u: %s orig=%u", (*ilc).FaberElement, rawMaterial->SheetName.c_str(), (uint32)((*ilc).ActualOriginality*100.0f) );
			RMByOriginalityByCraftSlot[(*ilc).FaberElement].insert( make_pair( (uint32)((*ilc).ActualOriginality*100.0f), rawMaterial ) );
		}
	}

	///
	void	popAndDeleteRM( CGenRawMaterial *rawMaterial )
	{
		delete rawMaterial;
		RawMaterials.erase( rawMaterial );
	}

	///
	bool	alreadyPopped( CGenRawMaterial *rawMaterial ) const
	{
		return RawMaterials.find( rawMaterial ) == RawMaterials.end();
	}

	/// fromPos and the returned iterator are the pos internal to the COriginalitySorter RM set
	/*CRMSet::const_iterator	getFirstRMNotInFamilyListFromPos( const set<uint>& familyList, TStatQuality statQuality, CRMSet::const_iterator fromPos ) const
	{
		CRMSet::const_iterator irm;
		for ( irm=fromPos; irm!=RawMaterials.end(); ++irm )
		{
			if ( ((*irm)->StatQuality == statQuality) &&
				 (familyList.find( (*irm)->IFamily ) == familyList.end()) )
				return irm;
		}
		return RawMaterials.end();
	}*/

	///
	CRMSet::iterator	getRMSetBegin() const { return RawMaterials.begin(); }

	///
	CRMSet::iterator	getRMSetEnd() const { return RawMaterials.end(); }

	///
	void	deleteAllRemainingRM()
	{
		CRMSet::iterator irm;
		for ( irm=RawMaterials.begin(); irm!=RawMaterials.end(); ++irm )
		{
			delete (*irm);
		}
		RawMaterials.clear();
		// Does not clear the maps by originality
	}

	std::vector< CMultiMapByOriginality >	RMByOriginalityByCraftSlot;

private:

	CRMSet									RawMaterials;
};


#define checkColor( c ) nlassert( colors[c] == #c );


/*
 *
 */
void	loadDFNs( UFormLoader *formLoader )
{
	map<string, CDfnFieldInfo> dfnFields;
	NLMISC::CSmartPtr<UFormDfn> formDfn;
	formDfn = formLoader->loadFormDfn( (rmSheetType + ".dfn").c_str() );
	if ( ! formDfn )
		nlerror( "Can't find DFN for %s", rmSheetType.c_str() );
	vector<string> craftPartsPaths;
	fillFromDFN( formLoader, dfnFields, formDfn, "", rmSheetType, "mp.MpParam", craftPartsPaths );

	// Get craft parts
	CraftParts.getNamesAndPaths( craftPartsPaths );
	
	formDfn = formLoader->loadFormDfn( (crSheetType + ".dfn").c_str() );
	if ( ! formDfn )
		nlerror( "Can't find DFN for %s", crSheetType.c_str() );
	fillFromDFN( formLoader, dfnFields, formDfn, "", crSheetType );

	// Get lists of predefined values from sitem and creature DFN
	families = dfnFields["mp.Family"].TypePredefinedLabels;
	familyCodes = dfnFields["mp.Family"].TypePredefinedValues;
	groups = dfnFields["mp.Group"].TypePredefinedLabels;

	//properties = dfnFields["mp.Material property 1"].TypePredefinedLabels;
	//nlverify( removeEntryFromList( properties, "Undefined" ) != ~0 );
	properties.resize( craftPartsPaths.size() );
	for ( uint i=0; i!=properties.size(); ++i ) // now, use properties as item part list
		properties[i] = string( 1, (char)('A' + i) );

	ecosystems = dfnFields["mp.Ecosystem"].TypePredefinedLabels,
	colors = dfnFields["mp.MpColor"].TypePredefinedLabels,
	creatures = dfnFields["Basics.Race"].TypePredefinedLabels;
	seasons.push_back( "Winter" );
	seasons.push_back( "Spring" );
	seasons.push_back( "Summer" );
	seasons.push_back( "Autumn" );
	//removeEntryFromList( families, "Undefined" );
	//removeEntryFromList( familyCodes, "0" );
	nlverify( removeEntryFromList( ecosystems, "unknown" ) != ~0 );
	nlverify( removeEntryFromList( ecosystems, "Goo" ) != ~0 );
	nlassert( ecosystems[0] == "Common" );
	nlassert( ecosystems[1] == "Desert" ); // ensure we match with enum TEcosystem!
	nlassert( ecosystems[2] == "Forest" );
	nlassert( ecosystems[3] == "Lacustre" );
	nlassert( ecosystems[4] == "Jungle" );
	nlassert( ecosystems[5] == "PrimeRoots" );
	//removeEntryFromList( ecosystems, "Common" ); // TODO
	nlassert( NbEcosystems == ecosystems.size() );
	nlverify( removeEntryFromList( colors, "None" ) != ~0 );
	nlverify( removeEntryFromList( colors, "UserColor") != ~0 );
	nlassert( colors.size() == NbColors );
	checkColor( Red );
	checkColor( Beige );
	checkColor( Green );
	checkColor( Turquoise );
	checkColor( Blue );
	checkColor( Violet );
	checkColor( White );
	checkColor( Black );

	/*UndefinedProperty = getIndexFromString( "Undefined", properties );
	nlassert( UndefinedProperty != ~0 );*/
}


/*
 * Build RMFamilyIndicesByCreatureModel and DepositFamilyIndices
 */
void	dispatchFamiliesToLocations()
{
	for ( CFamMap::iterator iss=FamSet.begin(); iss!=FamSet.end(); ++iss )
	{
		const CSString& famStr = (*iss).first;
		TFamInfo& famInfo = (*iss).second;
		uint iFam = getIndexFromString( famStr, families );
		if ( famInfo.IsInDeposits )
		{
			// Deposits
			nlassert( iFam != ~0 );
			DepositFamilyIndices.push_back( iFam );
		}
		if ( famInfo.IsInCreatures )
		{
			// Extract creature name from left of family name (ASSUMES there's no blank in creature name)
			CSString creaNameForRMFamily = famStr.splitTo( ' ' );

			// Dispatch 
			for ( CSkeletonMap::iterator icm=CreatureModels.begin(); icm!=CreatureModels.end(); ++icm )
			{
				const CSString& creaModel = (*icm).first;
				TSkeletonInfo& modelInfo = (*icm).second;

				if ( modelInfo.Name == creaNameForRMFamily )
				{
					RMFamilyIndicesByCreatureModel[creaModel].push_back( iFam );
					//nlinfo( "+ %s for %s (now %u models registered)", famStr.c_str(), creaModel.c_str(), RMFamilyIndicesByCreatureModel.size() );
					modelInfo.IsUsed = true; // Name and AbbrevName are set by deliverCreatureModels()
				}
			}
		}
		else switch ( famInfo.SpecialCreatureTag[0] )
		{
			// Goo & invasion/raid creatures
			case 'G':
			{
				GooCreatureFamilyIndices.push_back( iFam );
				nldebug( "Family %s selected for goo creatures", famStr.c_str() );
				break;
			}
			case 'I':
			{
				if ( famInfo.SpecialCreatureTag.size() == 1 )
				{
					InvasionRaidCreatureFamilyIndices['*'].push_back( iFam );
					nldebug( "Family %s selected for all invasion creatures", famStr.c_str() );
				}
				else
				{
					for ( uint c=1; c!=famInfo.SpecialCreatureTag.size(); ++c )
					{
						InvasionRaidCreatureFamilyIndices[famInfo.SpecialCreatureTag[c]].push_back( iFam );
						nldebug( "Family %s selected for invasion creature of type %c", famStr.c_str(), famInfo.SpecialCreatureTag[c] );
					}
				}
				break;
			}
		}
	}
}


/*
 * Returns the number of models used
 */
uint	checkSkeletons()
{
	uint32 nbSkeUsed = 0;
	for ( CSkeletonMap::const_iterator isc=CreatureModels.begin(); isc!=CreatureModels.end(); ++isc )
	{
		const string& skeFilename = (*isc).first;
		const TSkeletonInfo& ske = (*isc).second;
		const bool& used = (*isc).second.IsUsed;
		if ( used )
		{
			nldebug( "Model %s (%s) %s", skeFilename.c_str(), ske.AbbrevName.c_str(), used?"used":"NOT USED" );
			++nbSkeUsed;
		}
		else
			nlwarning( "Model %s %s", skeFilename.c_str(), used?"":"NOT USED" );
	}
	return nbSkeUsed;
}


/*
 *
 */
void	createDirectoryStructure()
{
	// Create the directory structure
	if ( WriteSheetsToDisk )
	{
		for ( uint32 i=0; i!=ecosystems.size(); ++i )
		{
			string dirname = conventionalDirectory( ecosystems[i] );
			if ( ! CFile::isExists( rawMaterialPath + dirname ) )
			{
				CFile::createDirectory( rawMaterialPath + dirname );
			}
			else
			{
				if ( ! CFile::isDirectory( rawMaterialPath + dirname ) )
				{
					nlwarning( "%s already existing but not a directory!", (rawMaterialPath + dirname).c_str() );
				}
			}
		}
		string dirname = "_parent";
		if ( ! CFile::isExists( rawMaterialPath + dirname ) )
		{
			CFile::createDirectory( rawMaterialPath + dirname );
		}
		else
		{
			if ( ! CFile::isDirectory( rawMaterialPath + dirname ) )
			{
				nlwarning( "%s already existing but not a directory!", (rawMaterialPath + dirname).c_str() );
			}
		}
	}
}


#endif // NL_SRG_UTILITIES_H

/* End of srg_utilities.h */
