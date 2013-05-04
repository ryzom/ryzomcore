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



// game share pre-compiled headers
#include "stdpch.h"

// misc
//#include "nel/misc/path.h"
//#include "nel/misc/file.h"
//#include "nel/misc/command.h"

// net
#include "nel/net/service.h"

// game share
#include "egs_sheets/egs_sheets.h"
#include "game_share/roles.h"
#include "nel/misc/algo.h"

//#include "game_item_manager/game_item_manager.h"
//#include "player_manager/player_manager.h"
//#include "egs_mirror.h"

#include "egs_static_encyclo.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;
using namespace NLGEORGES;

//extern CPlayerManager PlayerManager;
extern CVariable<bool> LoadOutposts;
extern CVariable<bool> EGSLight;


//------------------------------------------------------------------------------------------
// Singleton data instantiation
//------------------------------------------------------------------------------------------

CSheets CSheets::_StaticSheets;		// the singleton instance
bool CSheets::_Initialised=false;	// - set true by constructor
bool CSheets::_Destroyed=false;		// - set true by destructor


//---------------------------------------------------
// scanDirectoriesForFiles : utility routine for init()
//
// /TODO: This routine should be static by rights but we've left it
// accessible to the brick manager who generates own packed sheets
//---------------------------------------------------
void scanGeorgePaths(bool forceFullRescan=false)
{
	// a handy static local flag used to make sure this routine isn't
	// rerun in full after a first run unless the 'force' flag is set true
	static bool GeorgePathsAdded=false;

	// a handy object for reading config file variables...
	CConfigFile::CVar *var;

	// if we've already added the george directories then exit
	if (GeorgePathsAdded && !forceFullRescan)
		return;

	if (forceFullRescan)
	{
		// clear out the search paths 'cos we're performing a complete rescan
		NLMISC::CPath::clearMap();

		// rescan 'Paths' directories
		if ((var = IService::getInstance()->ConfigFile.getVarPtr ("Paths")) != NULL)
		{
			for (uint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), true, false);
			}
		}

		// rescan 'PathsNoRecurse' directories
		if ((var = IService::getInstance()->ConfigFile.getVarPtr ("PathsNoRecurse")) != NULL)
		{
			for (uint i = 0; i < var->size(); i++)
			{
				CPath::addSearchPath (var->asString(i), false, false);
			}
		}
	}

	// add any paths listed in the 'GeorgeFiles' config file variable
	if ((var = IService::getInstance()->ConfigFile.getVarPtr ("GeorgePaths")) != NULL)
	{
		for (uint i = 0; i < var->size(); i++)
		{
			CPath::addSearchPath (var->asString(i), true, false);
		}
	}

	// setup our global boolean for next time round
	GeorgePathsAdded=true;
}

//---------------------------------------------------
// loadSheetSet : utility routine for init()
//
//---------------------------------------------------
template <class C> void loadSheetSet(const char *fileType,const char *sheetFile, C &sheetMap)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetMap.clear();

	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL)
	{
		loadForm( fileType, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, false, false);
	}

	// if we haven't succeeded in minimal scan (or 'GeorgePaths' wasn't found in config file) then perform standard scan
	if (sheetMap.empty())
	{
		// if the 'GeorgePaths' variable exists and hasn't already been treated then add new paths to CPath singleton
		scanGeorgePaths();
		loadForm( fileType, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, true);
	}
}

//---------------------------------------------------
// loadSheetSet : utility routine for init()
//
//---------------------------------------------------
// variant with smart pointers, maintain with function above
template <class C> void loadSheetSet2(const char *fileType,const char *sheetFile, C &sheetMap)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetMap.clear();
	
	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL)
	{
		loadForm2( fileType, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, false, false);
	}
	
	// if we haven't succeeded in minimal scan (or 'GeorgePaths' wasn't found in config file) then perform standard scan
	if (sheetMap.empty())
	{
		// if the 'GeorgePaths' variable exists and hasn't already been treated then add new paths to CPath singleton
		scanGeorgePaths();
		loadForm2( fileType, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, true);
	}
}

//---------------------------------------------------
// loadSheetSet : utility routine for init()
//
//---------------------------------------------------
template <class T> void loadSheetSet(const vector<string> &fileTypes,const char *sheetFile, T &sheetMap)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetMap.clear();

	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL)
	{
		loadForm( fileTypes, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, false, false);
	}

	// if we haven't succeeded in minimal scan (or 'GeorgePaths' wasn't found in config file) then perform standard scan
	if (sheetMap.empty())
	{
		// if the 'GeorgePaths' variable exists and hasn't already been treated then add new paths to CPath singleton
		scanGeorgePaths();
		loadForm( fileTypes, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, true);
	}
}


template <class T>
void loadSheetSetForHashMap(const vector<string> &fileTypes,const char *sheetFile, CHashMap<CSheetId,T,CSheetIdHashMapTraits> &sheetHashMap)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetHashMap.clear();

	map<CSheetId,T> sheetMap;

	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL)
	{
		loadForm( fileTypes, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, false, false);
	}

	// if we haven't succeeded in minimal scan (or 'GeorgePaths' wasn't found in config file) then perform standard scan
	if (sheetMap.empty())
	{
		// if the 'GeorgePaths' variable exists and hasn't already been treated then add new paths to CPath singleton
		scanGeorgePaths();
		loadForm( fileTypes, IService::getInstance()->WriteFilesDirectory.toString()+sheetFile, sheetMap, true);
	}

	// Convert map to hash_map
	for ( typename map<CSheetId,T>::const_iterator im=sheetMap.begin(); im!=sheetMap.end(); ++im )
	{
		sheetHashMap.insert( *im );
	}
}



//---------------------------------------------------
// init :
//
//---------------------------------------------------
void CSheets::init()
{
	CMP::loadGroups( "item_mp_group.typ" );
	vector<string> fileTypes;

	loadSheetSet( "succes_chances_table",	"egs_success_chance_tables.packed_sheets",	_StaticSheets._SuccessTables); // note: need to be loaded before creatures...
	fileTypes.clear();
	fileTypes.push_back("sitem");
	loadSheetSetForHashMap( fileTypes,		"egs_items.packed_sheets",					_StaticSheets._StaticItems); // note: need to be loaded before creatures...
//	loadSheetSet( "sitem",					"egs_items.packed_sheets",					_StaticSheets._StaticItems); // note: need to be loaded before creatures...
	loadSheetSet( "xp_table",				"egs_xptables.packed_sheets",				_StaticSheets._StaticXpStageTable);
	loadSheetSet( "action_xp_factor",		"egs_action_xp_factor.packed_sheets",		_StaticSheets._StaticXpFactorTables);
	loadSheetSet( "death_impact",			"egs_death_impact.packed_sheets",			_StaticSheets._StaticPacts);
	fileTypes.clear();
	fileTypes.push_back("creature");
	loadSheetSetForHashMap(fileTypes,		"egs_creatures.packed_sheets",				_StaticSheets._StaticCreatures);
	loadSheetSet( "loot_set",				"egs_loot_set.packed_sheets",				_StaticSheets._StaticLootSet);
	loadSheetSet( "loot_table",				"egs_loot_table.packed_sheets",				_StaticSheets._StaticLootTable);
	loadSheetSet( "race_stats",				"egs_race_stats.packed_sheets",				_StaticSheets._StaticRaceStats);
	loadSheetSet( "starting_role",			"egs_starting_role.packed_sheets",			_StaticSheets._StaticRole);
	loadSheetSet( "skill_tree",				"egs_skill_tree.packed_sheets",				_StaticSheets._SkillsTree);
	loadSheetSet( "aiaction",				"egs_aiactions.packed_sheets",				_StaticSheets._AiActions);
	loadSheetSet( "weather_setup",			"egs_weather_setup.packed_sheets",			_StaticSheets._WeatherSetups);
	loadSheetSet( "weather_function_params","egs_weather_function_params.packed_sheets",_StaticSheets._WeatherFunctionParams);	
	loadSheetSet( "world",					"egs_world.packed_sheets",					_StaticSheets._StaticWorld);
	loadSheetSet( "continent",				"egs_continents.packed_sheets",				_StaticSheets._StaticContinents);

	// only load outpost sheets if outpost system is activated
	if (LoadOutposts.get())
	{
		loadSheetSet( "outpost_building",		"egs_outpost_building.packed_sheets",		_StaticSheets._StaticOutpostBuilding);
		loadSheetSet( "outpost_squad",			"egs_outpost_squads.packed_sheets",			_StaticSheets._StaticOutpostSquads);
		loadSheetSet( "outpost",				"egs_outposts.packed_sheets",				_StaticSheets._StaticOutposts);
	}
	
	fileTypes.clear();
	fileTypes.push_back("sbrick");
	fileTypes.push_back("saibrick");
	loadSheetSetForHashMap( fileTypes,		"egs_sbricks.packed_sheets",				_StaticSheets._SBrickSheets);

	fileTypes.clear();
	fileTypes.push_back("sphrase");
	fileTypes.push_back("saiphrase");
	loadSheetSetForHashMap( fileTypes,		"egs_sphrases.packed_sheets",				_StaticSheets._SPhraseSheets);

	// Make fast-browsing vector
	for ( CAllRolemasterPhrases::iterator ihm=_StaticSheets._SPhraseSheets.begin(); ihm!=_StaticSheets._SPhraseSheets.end(); ++ihm )
	{
		if ( ((*ihm).first.toString().find( "saiphrase" ) == string::npos) && (*ihm).second.IsRolemasterPhrase )
		{
			string phraseName = (*ihm).first.toString();
			phraseName = phraseName.substr( 0, phraseName.find( "." ) );
			_StaticSheets._SPhraseSheetVector.push_back( make_pair( &(*ihm).second, make_pair( (*ihm).first, phraseName ) ) );
		}
	}

	if (!EGSLight)
	{
		// Emotes
		std::map< NLMISC::CSheetId, CStaticTextEmotes > emoteTexts;
		loadSheetSet( "text_emotes", "egs_text_emotes.packed_sheets", emoteTexts);
		if ( emoteTexts.size() != 1 )
			nlerror("there must be one '.text_emotes' count = %u",emoteTexts.size() );
		else
			_StaticSheets._TextEmotes = (*emoteTexts.begin()).second;
		// Emote anims
		std::map< NLMISC::CSheetId, CStaticEmot > emots;
		loadSheetSet( "emot", "egs_emot.packed_sheets", emots);
		if ( emoteTexts.size() != 1 )
			nlerror("there must be one '.emot' count = %u",emots.size() );
		else
			_StaticSheets._Emots = (*emots.begin()).second;
	}

	nlinfo("<CSheets::loadSheets> SHEETS LOADED");

	initBitfieldIndex();

	// Encyclopedia
	_StaticSheets._Encyclopedia = new CStaticEncyclo;
	if (!EGSLight)
	{
		loadSheetSet( "encyclo_album",	"egs_encyclo_album.packed_sheets",	_StaticSheets._Encyclopedia->_AlbumsFromSheet);
		loadSheetSet( "encyclo_thema",	"egs_encyclo_thema.packed_sheets",	_StaticSheets._Encyclopedia->_ThemasFromSheet);
	}
	_StaticSheets._Encyclopedia->init();
} // init //



//---------------------------------------------------
// initBitfieldIndex :
//---------------------------------------------------
void CSheets::initBitfieldIndex()
{
/*
	//std::map< NLMISC::CSheetId,CStaticItem > _StaticItems;
	map< CSheetId,CStaticItem >::iterator it;
	const map< CSheetId,CStaticItem >::iterator itEnd = _StaticSheets._StaticItems.end();

	uint16 bitIndex = 0;
	for ( it = _StaticSheets._StaticItems.begin() ; it != itEnd ; ++it)
	{
		// \todo Malkav : check item can be built via faber (check family)
		if ( (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::ARMOR
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::MELEE_WEAPON
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::RANGE_WEAPON
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::AMMO
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::SHIELD
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::CRAFTING_TOOL
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::HARVEST_TOOL
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::TAMING_TOOL
			|| (ITEMFAMILY::EItemFamily)(*it).second.Family == ITEMFAMILY::JEWELRY
			)
		{
			// TempYoyo
			//(*it).second.FaberPlanBitIndex = bitIndex;
			++bitIndex;
		}
	}
*/
} // initBitfieldIndex //


//---------------------------------------------------
// getForm :
//
//---------------------------------------------------
const CStaticItem* CSheets::getForm( const CSheetId& sheetId )
{
	CAllStaticItems::const_iterator itForm = _StaticSheets._StaticItems.find( sheetId );
	if( itForm == _StaticSheets._StaticItems.end() )
	{
		//nlwarning( "<CSheets::getForm> The static form for sheet %s (%08x) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
} // getForm //


//---------------------------------------------------
// getSBrickForm : return form for sabrina brick
//
//---------------------------------------------------
const CStaticBrick* CSheets::getSBrickForm( const CSheetId& sheetId )
{
	CAllStaticBricks::const_iterator itForm = _StaticSheets._SBrickSheets.find( sheetId );
	if( itForm == _StaticSheets._SBrickSheets.end() )
	{
		return 0;
	}
	return &(*itForm).second;
} // getSBrickForm //


//---------------------------------------------------
// Get sabrina phrase
//
//---------------------------------------------------
const CStaticRolemasterPhrase* CSheets::getSRolemasterPhrase( const NLMISC::CSheetId& sheetId )
{
	CAllRolemasterPhrases::const_iterator ip = _StaticSheets._SPhraseSheets.find( sheetId );
	if ( ip == _StaticSheets._SPhraseSheets.end() )
	{
		nlwarning( "<CSheets::getSRolemasterPhrase> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*ip).second;
} // getSRolemasterPhrase //



//---------------------------------------------------
// getBrickForm : return form for brick
//
//---------------------------------------------------
//const CStaticGameBrick* CSheets::getBrickForm( const CSheetId& sheetId )
//{
//	map<CSheetId, CStaticGameBrick>::const_iterator itForm = _StaticSheets._BrickSheets.find( sheetId );
//	if( itForm == _StaticSheets._BrickSheets.end() )
//	{
//		nlwarning( "<CSheets::getBrickForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
//		return 0;
//	}
//	return &(*itForm).second;
//} // getForm //


//---------------------------------------------------
// getXpStageTableForm : Get form of XpStageTable
//
//---------------------------------------------------
const CStaticXpStagesTable* CSheets::getXpStageTableForm( const NLMISC::CSheetId& sheetId )
{
	map<CSheetId, CStaticXpStagesTable>::const_iterator itForm = _StaticSheets._StaticXpStageTable.find( sheetId );
	if( itForm == _StaticSheets._StaticXpStageTable.end() )
	{
		nlwarning( "<CSheets::getXpStageTableForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}


//---------------------------------------------------
// getPactsForm : Get form of pacts info
//
//---------------------------------------------------
const CStaticPacts* CSheets::getPactsForm( const NLMISC::CSheetId& sheetId )
{
	map< CSheetId, CStaticPacts >::const_iterator itForm = _StaticSheets._StaticPacts.find( sheetId );
	if( itForm == _StaticSheets._StaticPacts.end() )
	{
		nlwarning( "<CSheets::getPactsForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getCreaturesForm : Get form of creatures info
//
//---------------------------------------------------
const CStaticCreatures* CSheets::getCreaturesForm( const NLMISC::CSheetId& sheetId )
{
	CAllStaticCreatures::const_iterator itForm = _StaticSheets._StaticCreatures.find( sheetId );
	if( itForm == _StaticSheets._StaticCreatures.end() )
	{
		nlwarning( "<CSheets::getCreaturesForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}


//---------------------------------------------------
// getLootSetForm : Get form of loot set info
//
//---------------------------------------------------
const CStaticLootSet * CSheets::getLootSetForm( const NLMISC::CSheetId& sheetId )
{
	map< CSheetId, CStaticLootSet >::const_iterator itForm = _StaticSheets._StaticLootSet.find( sheetId );
	if( itForm == _StaticSheets._StaticLootSet.end() )
	{
		nlwarning( "<CSheets::getLootSetForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}


//---------------------------------------------------
// getLootTableForm : Get form of loot table info
//
//---------------------------------------------------
const CStaticLootTable* CSheets::getLootTableForm( const NLMISC::CSheetId& sheetId )
{
	if( sheetId == CSheetId::Unknown )
	{
		return NULL;
	}
	map< CSheetId, CStaticLootTable >::const_iterator itForm = _StaticSheets._StaticLootTable.find( sheetId );
	if( itForm == _StaticSheets._StaticLootTable.end() )
	{
		nlwarning( "<CSheets::getLootTableForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getRaceStats : Get form of Stats by race (people) for character create
//
//---------------------------------------------------
const CStaticRaceStats* CSheets::getRaceStats( const NLMISC::CSheetId& sheetId )
{
	map< CSheetId, CStaticRaceStats >::const_iterator itForm = _StaticSheets._StaticRaceStats.find( sheetId );
	if( itForm == _StaticSheets._StaticRaceStats.end() )
	{
		//nlwarning( "<CSheets::getRaceStats> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getStartRole : Get form of starting equipment and actions by race/role (people) for character create
//
//---------------------------------------------------
const CStaticRole* CSheets::getStartRole( const NLMISC::CSheetId& sheetId )
{
	map< CSheetId, CStaticRole >::const_iterator itForm = _StaticSheets._StaticRole.find( sheetId );
	if( itForm == _StaticSheets._StaticRole.end() )
	{
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getRaceStats : Get form of Stats by race (people) for character create
//
//---------------------------------------------------
//Deprecated
/*const CStaticRoleSkill* CSheets::getRoleSkill( const NLMISC::CSheetId& sheetId )
{
	map< CSheetId, CStaticRoleSkill >::const_iterator itForm = _StaticSheets._StaticRoleSkill.find( sheetId );
	if( itForm == _StaticSheets._StaticRoleSkill.end() )
	{
		nlwarning( "<CSheets::getRoleSkill> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}
*/


//---------------------------------------------------
// getSkillsTreeForm : Get form of Skills tree
//
//---------------------------------------------------
const CStaticSkillsTree * CSheets::getSkillsTreeForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticSkillsTree >::const_iterator itForm = _StaticSheets._SkillsTree.find( sheetId );
	if (itForm == _StaticSheets._SkillsTree.end() )
	{
		nlwarning("<CSheets::getSkillsTreeForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getXpFactorTableForm
//
//---------------------------------------------------
const CStaticXpFactorTable * CSheets::getXpFactorTableForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticXpFactorTable >::const_iterator itForm = _StaticSheets._StaticXpFactorTables.find( sheetId );
	if (itForm == _StaticSheets._StaticXpFactorTables.end() )
	{
		nlwarning("<CSheets::getXpFactorTableForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getDepositForm : Get form of deposit
//---------------------------------------------------
/*const CStaticDeposit * CSheets::getDepositForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticDeposit >::const_iterator itForm = _StaticSheets._Deposit.find( sheetId );
	if (itForm == _StaticSheets._Deposit.end() )
	{
		nlwarning("<CSheets::getDepositForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}*/

//---------------------------------------------------
// getSkilgetAiActionFormlsTreeForm : Get form of ai action
//---------------------------------------------------
const CStaticAiAction * CSheets::getAiActionForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticAiAction >::const_iterator itForm = _StaticSheets._AiActions.find( sheetId );
	if (itForm == _StaticSheets._AiActions.end() )
	{
		nlwarning("<CSheets::getAiActionForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getSuccessTableForm : Get form of success table
//---------------------------------------------------
const CStaticSuccessTable * CSheets::getSuccessTableForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticSuccessTable >::const_iterator itForm = _StaticSheets._SuccessTables.find( sheetId );
	if (itForm == _StaticSheets._SuccessTables.end() )
	{
		nlwarning("<CSheets::getSuccessTableForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}	

//---------------------------------------------------
// getWeatherSetupSheet
//---------------------------------------------------
const CWeatherSetupSheetBase * CSheets::getWeatherSetupSheet( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CWeatherSetupSheetBase >::const_iterator itForm = _StaticSheets._WeatherSetups.find( sheetId );
	if ( itForm == _StaticSheets._WeatherSetups.end() )
	{
		// No warning, because as we list weather setups found in sheet_id.bin, some may have been removed
		nlinfo("<CSheets::getWeatherSetupSheet> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}


//---------------------------------------------------
// getWorldForm
//---------------------------------------------------
const CStaticWorld * CSheets::getWorldForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticWorld >::const_iterator itForm = _StaticSheets._StaticWorld.find( sheetId );
	if ( itForm == _StaticSheets._StaticWorld.end() )
	{
		nlwarning("<getWorldForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getContinentForm
//---------------------------------------------------
const CStaticContinent * CSheets::getContinentForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticContinent >::const_iterator itForm = _StaticSheets._StaticContinents.find( sheetId );
	if ( itForm == _StaticSheets._StaticContinents.end() )
	{
		nlwarning("<CSheets::getContinentForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getOutpostForm
//---------------------------------------------------
const CStaticOutpost * CSheets::getOutpostForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticOutpost >::const_iterator itForm = _StaticSheets._StaticOutposts.find( sheetId );
	if ( itForm == _StaticSheets._StaticOutposts.end() )
	{
		nlwarning("<CSheets::getOutpostForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getOutpostBuildingForm
//---------------------------------------------------
const CStaticOutpostBuilding * CSheets::getOutpostBuildingForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticOutpostBuilding >::const_iterator itForm = _StaticSheets._StaticOutpostBuilding.find( sheetId );
	if ( itForm == _StaticSheets._StaticOutpostBuilding.end() )
	{
		nlwarning("<CSheets::getOutpostBuildingForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

//---------------------------------------------------
// getOutpostSquadForm
//---------------------------------------------------
const CStaticOutpostSquad * CSheets::getOutpostSquadForm( const NLMISC::CSheetId& sheetId )
{
	std::map< NLMISC::CSheetId, CStaticOutpostSquad >::const_iterator itForm = _StaticSheets._StaticOutpostSquads.find( sheetId );
	if ( itForm == _StaticSheets._StaticOutpostSquads.end() )
	{
		nlwarning("<CSheets::getOutpostSquadForm> The static form sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}


// ***************************************************************************
template <class T> void reloadSheetSet(const vector<string> &fileTypes, T &sheetMap, const string &wildcardFilter)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetMap.clear();
	
	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL)
	{
		scanGeorgePaths();
		loadFormNoPackedSheet( fileTypes, sheetMap, wildcardFilter);
	}
}

// variant with smart pointers, maintain with function above
template <class T> void reloadSheetSet2(const vector<string> &fileTypes, T &sheetMap, const string &wildcardFilter)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetMap.clear();
	
	// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
	if (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL)
	{
		scanGeorgePaths();
		loadFormNoPackedSheet2( fileTypes, sheetMap, wildcardFilter);
	}
}

template <class T> void reloadSheetSet(const string &fileType, T &sheetMap, const string &wildcardFilter)
{
	vector<string>	fileTypes;
	fileTypes.push_back(fileType);
	reloadSheetSet(fileTypes, sheetMap, wildcardFilter);
}

// variant with smart pointers, maintain with function above
template <class T> void reloadSheetSet2(const string &fileType, T &sheetMap, const string &wildcardFilter)
{
	vector<string>	fileTypes;
	fileTypes.push_back(fileType);
	reloadSheetSet2(fileTypes, sheetMap, wildcardFilter);
}

template <class T>
void reloadSheetSetForHashMap(const vector<string> &fileTypes, CHashMap<CSheetId,T,CSheetIdHashMapTraits> &sheetHashMap, const string &wildcardFilter)
{
	// clear out the previous sheet set	(just in case this is a re-read)
	sheetHashMap.clear();
	
	map<CSheetId,T> sheetMap;
	reloadSheetSet(fileTypes, sheetMap, wildcardFilter);
	
	// Convert map to hash_map
	for ( typename map<CSheetId,T>::const_iterator im=sheetMap.begin(); im!=sheetMap.end(); ++im )
	{
		sheetHashMap.insert( *im );
	}
}


// ***************************************************************************
template<class T> void reloadExistingSheets(T &src, T &dst)
{
	typename T::const_iterator	itSrc;
	for(itSrc=src.begin();itSrc!=src.end();++itSrc)
	{
		typename T::iterator		itDst= dst.find(itSrc->first);
		if(itDst!=dst.end())
		{
			itDst->second.reloadSheet(itSrc->second);
		}
	}
}

// ***************************************************************************
// variant with smart pointers, maintain with function above
template<class T> void reloadExistingSheets2(T &src, T &dst)
{
	typename T::const_iterator	itSrc;
	for(itSrc=src.begin();itSrc!=src.end();++itSrc)
	{
		typename T::iterator		itDst= dst.find(itSrc->first);
		if(itDst!=dst.end())
		{
			itDst->second->reloadSheet(*itSrc->second);
		}
	}
}

// ***************************************************************************
template<class T> void reloadCommonSheets(T &dst, const char *type, const string &wildcardFilter)
{
	// Load sheets in a temp container
	T	tmpContainer;
	reloadSheetSet( type, tmpContainer, wildcardFilter);
	
	// copy entries to existing sheets
	reloadExistingSheets(tmpContainer, dst);
	
}
	
// ****************************************************************************
// variant with smart pointers, maintain with function above
template<class T> void reloadCommonSheets2(T &dst, const char *type, const string &wildcardFilter)
{
	// Load sheets in a temp container
	T	tmpContainer;
	reloadSheetSet2( type, tmpContainer, wildcardFilter);
	
	// copy entries to existing sheets
	reloadExistingSheets2(tmpContainer, dst);
	
}

// ***************************************************************************
void		CSheets::reloadCreature(const string &wildcardFilter)
{
	vector<string> fileTypes;
	fileTypes.push_back("creature");
	CAllStaticCreatures creatures;
	reloadSheetSetForHashMap(fileTypes, creatures, wildcardFilter);
	
	// copy entries to existing sheets
	reloadExistingSheets(creatures, _StaticSheets._StaticCreatures	);
}

// ***************************************************************************
void		CSheets::reloadSItem(const string &wildcardFilter)
{
	// note: need to be loaded before creatures... \todo what can be the problem on reload?
	
	vector<string> fileTypes;
	fileTypes.push_back("sitem");
	CAllStaticItems items;
	reloadSheetSetForHashMap(fileTypes, items, wildcardFilter);

	// copy entries to existing sheets
	reloadExistingSheets(items, _StaticSheets._StaticItems	);

//	reloadCommonSheets(_StaticSheets._StaticItems, "sitem", wildcardFilter);
}

// ***************************************************************************
void		CSheets::reloadSBrick(const string &wildcardFilter)
{
	// Load sheets in a temp container
	vector<string> fileTypes;
	fileTypes.push_back("sbrick");
	fileTypes.push_back("saibrick");
	CAllStaticBricks		bricks;
	reloadSheetSetForHashMap( fileTypes, bricks, wildcardFilter);

	// copy entries to existing sheets
	reloadExistingSheets(bricks, _StaticSheets._SBrickSheets	);
}

// ***************************************************************************
void		CSheets::reloadSPhrase(const string &wildcardFilter)
{
	// Load sheets in a temp container
	vector<string> fileTypes;
	fileTypes.push_back("sphrase");
	fileTypes.push_back("saiphrase");
	CAllRolemasterPhrases	phrases;
	reloadSheetSetForHashMap( fileTypes, phrases, wildcardFilter);

	// copy entries to existing sheets
	reloadExistingSheets(phrases, _StaticSheets._SPhraseSheets	);
}

// ***************************************************************************
void		CSheets::reloadSuccessChancesTable(const std::string &wildcardFilter)
{
	reloadCommonSheets(_StaticSheets._SuccessTables, "succes_chances_table", wildcardFilter);

	// in case of dodge parry success chance table reloaded, recompile some params of creatures
	CAllStaticCreatures::iterator	it= _StaticSheets._StaticCreatures.begin();
	for(;it!=_StaticSheets._StaticCreatures.end();++it)
	{
		it->second.compileCreatureDamagePerHit();
	}
}

// ***************************************************************************
void		CSheets::reloadXPTable(const std::string &wildcardFilter)
{
	reloadCommonSheets(_StaticSheets._StaticXpStageTable, "xp_table", wildcardFilter);
}

// ***************************************************************************
std::string	extendWildcard(const std::string &in)
{
	string	out;
	// append * at begin if not present (or if enp
	if(in.empty() || in[0]!='*')
		out= '*';
	out+= in;
	// append .* at end if no . found
	if(in.find('.')==string::npos)
		out+= ".*";
	return out;
}

// ***************************************************************************
#define CMD_RELOAD_SHEET(_func)			\
NLMISC_COMMAND(_func, #_func, "")		\
{										\
	if (args.size()>1) return false;	\
	string		wildcardFilter;			\
	if (args.size()>=1)					\
		wildcardFilter= extendWildcard(args[0]);		\
	CSheets::_func (wildcardFilter);	\
	return true;						\
}
CMD_RELOAD_SHEET(reloadCreature)
CMD_RELOAD_SHEET(reloadSItem)
CMD_RELOAD_SHEET(reloadSBrick)
CMD_RELOAD_SHEET(reloadSPhrase)
CMD_RELOAD_SHEET(reloadSuccessChancesTable)
CMD_RELOAD_SHEET(reloadXPTable)













