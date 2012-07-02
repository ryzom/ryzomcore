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

// Contient la description d'une Craft Part
// ainsi que les différentes caracteristiques
// qu'elle concerne
struct CraftPart
{
	CSString Desc;
	bool Carac[NumMPStats];
};

// Caractéristiques d'une MP relatives au craft
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

// Permet de trier des chaines de caractères
struct ItemSort : public std::less<NLMISC::CSString> 
{ 
	bool operator()( const NLMISC::CSString& x, const NLMISC::CSString& y ) const 
	{ 
		return x.icompare( y ); 
	} 
};
typedef set<CSString, ItemSort> CSortedStringSet;

// Decrit un des items générés 
// pour une MP de créature
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
//      VARIABLES GLOBALES      //
//////////////////////////////////

// Liste des différentes Craft parts
CraftPart craftParts[NumMPCraftParts];

// Liste des fichiers .creature
map<string, string>	creatureFiles;

// noms générés pour chaque item
CSortedStringSet itemNames;

// Liste des items à generer 
// pour chaque code de créature
map<CSString, ListeCreatureMP> itemsAGenerer;

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


// chemin des différents repertoires
CSString LEVEL_DESIGN_PATH;
CSString TRANSLATION_PATH;
	
CSString ITEM_MP_FAMILY_TYP;
CSString ITEM_MP_GROUPE_TYP;
CSString ITEM_MP_PARAM_DFN;
CSString MP_DIRECTORY;
CSString DEPOSIT_MPS; 
CSString RAW_MATERIAL_ASSIGN;
CSString IC_FAMILIES_FORAGE_SOURCE;

CSString WK_UXT;
CSString ITEM_WORDS_WK;

#endif