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

#ifndef MOULINETTE_H_
#define MOULINETTE_H_


//////////////////////////////////
//       INCLUDE FILES          //
//////////////////////////////////
#include <nel/misc/types_nl.h>
#include <nel/misc/sstring.h>
#include "nel/misc/path.h"
#include <nel/misc/file.h>

//////////////////////////////////
//      USED NAMESPACES         //
//////////////////////////////////
using namespace NLMISC;
using namespace std;


//////////////////////////////////
//     DATA STRUCTURES          //
//////////////////////////////////

enum
{
	NumMPStats= 34,
	NumMPCraftParts= 26,		// Warning: changing this value won't be sufficient. 
								// You'll have to look deeper into code
};

// Contains Craft Part description and different stats related
struct CraftPart
{
	CSString Desc;
	bool Carac[NumMPStats];
};

// Raw mats stats for craft
struct MPCraftStats
{
	CSString Craft;
	bool	UsedAsCraftRequirement;
	int bestStatA;
	int worstStatA1;
	int worstStatA2;
	int bestStatB;
	int worstStatB1;
	int worstStatB2;
	int color;

	MPCraftStats()
	{
		UsedAsCraftRequirement= false;
	}
};

// Allows to sort strings
struct ItemSort : public std::less<NLMISC::CSString> 
{ 
	bool operator()( const NLMISC::CSString& x, const NLMISC::CSString& y ) const 
	{ 
		return x.icompare( y ); 
	} 
};
typedef set<CSString, ItemSort> CSortedStringSet;

// Describe a item generated for a creature raw mat
struct CreatureMPItem
{
	char eco;
	int itemLevel;
	int creatureLevel;
	CSString codeCreature;
	CSortedStringSet generatedItems;
	CSString creatureFileName;
};
typedef list<CreatureMPItem*> ListeCreatureMP;

// Misc Info on a MP family
class CExtraInfo
{
public:
	bool	DropOrSell;

	CExtraInfo()
	{
		DropOrSell= false;
	}
};

//////////////////////////////////
//      GLOBAL VARIABLES        //
//////////////////////////////////

// Craft parts list
CraftPart craftParts[NumMPCraftParts];

// .creature files list
map<string, string>	creatureFiles;

// The raw material assignment files that exist (_<creature>_mp.creature), by file name without extension
map<string, string>	assignmentFiles;

// The raw materials assigned to each creature in this run, in order, by creature file name without extension
map<string, vector<CSString> >	creatureAssignments;

// generated names for each item
CSortedStringSet itemNames;

// Items list to generate for each creature code
map<CSString, ListeCreatureMP> itemsAGenerer;

// The creature model name of each creature code of creature_models.csv, by code in lower case
map<CSString, CSString> creatureModelNames;

// The creature codes ("chd") whose raw materials were generated, each needs its parent sheet (_mchd.sitem)
set<CSString> generatedCreatureCodes;

// MP Family list
class CMPFamily
{
public:
	// Name of the family
	CSString	Name;
	// Icon
	CSString	Icon;
};
std::vector<CMPFamily>			MPFamilies;


// directories pathes
CSString LEVEL_DESIGN_PATH;
CSString TRANSLATION_PATH;
	
CSString ITEM_MP_FAMILY_TYP;
CSString ITEM_MP_GROUPE_TYP;
CSString ITEM_MP_PARAM_DFN;
CSString MP_DIRECTORY;
CSString DEPOSIT_MPS;
CSString CREATURE_PATH;
CSString RAW_MATERIAL_ASSIGN;
CSString IC_FAMILIES_FORAGE_SOURCE;

CSString WK_UXT;
CSString ITEM_WORDS_WK;

#endif