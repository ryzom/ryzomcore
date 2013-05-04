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



#ifndef RY_EGS_SHEETS_H
#define RY_EGS_SHEETS_H

#include "egs_static_deposit.h"
#include "egs_static_game_item.h"
#include "egs_static_game_sheet.h"
#include "egs_static_harvestable.h"
#include "egs_static_raw_material.h"
#include "egs_static_brick.h"
#include "egs_static_rolemaster_phrase.h"
#include "egs_static_ai_action.h"
#include "egs_static_success_table.h"
#include "egs_static_xp_factor_table.h"
#include "egs_static_world.h"
#include "egs_static_text_emotes.h"
#include "egs_static_emot.h"
#include "egs_static_outpost.h"
#include "game_share/time_weather_season/weather_setup_sheet_base.h"
#include "game_share/time_weather_season/weather_function_params_sheet_base.h"

// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_loader.h"

typedef CHashMap< NLMISC::CSheetId, CStaticItem, NLMISC::CSheetIdHashMapTraits> CAllStaticItems;
//typedef std::map< NLMISC::CSheetId, CStaticItem > CAllStaticItems;
typedef CHashMap< NLMISC::CSheetId, CStaticGameBrick, NLMISC::CSheetIdHashMapTraits> CAllBrickSheets;
typedef CHashMap< NLMISC::CSheetId, CStaticBrick, NLMISC::CSheetIdHashMapTraits> CAllStaticBricks;
typedef CHashMap< NLMISC::CSheetId, CStaticRolemasterPhrase, NLMISC::CSheetIdHashMapTraits> CAllRolemasterPhrases;
typedef CHashMap< NLMISC::CSheetId, CStaticCreatures, NLMISC::CSheetIdHashMapTraits> CAllStaticCreatures;


// For fast linear browsing (excluding .saiphrase). The string is the sheet name without extension.
typedef std::vector< std::pair< CStaticRolemasterPhrase*, std::pair< NLMISC::CSheetId, std::string > > > CAllRolemasterPhrasesLinear;

/*
 *	
 */

class CStaticEncyclo;

/**
 * CSheets
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CSheets
{
public :

	/// decay delay of an item on the ground (tick count)
	NLMISC::TGameCycle DecayDelay;

	/// exception thrown when a sheet is unknown
	struct ESheet : public NLMISC::Exception
	{
		ESheet( const NLMISC::CSheetId& sheetId ) : Exception ("The sheet "+sheetId.toString()+" is unknown") {}
	};

public:

	/**
	 * Init the manager (register callbacks)
	 */
	static void init();

	/**
	 * Get the form from its id
	 * \param sheet id
	 * \return form
	 */
	// Get item form
	static const CStaticItem* getForm( const NLMISC::CSheetId& sheetId );

	// Get item form map reference
	static inline const CAllStaticItems& getItemMapForm() { return _StaticSheets._StaticItems; }
	static inline CAllStaticItems& getItemMapFormNoConst() { return _StaticSheets._StaticItems; }
	
	// Get form for brick
//	static const CStaticGameBrick* getBrickForm( const NLMISC::CSheetId& sheetId );
	// get sabrina brick
	static const CStaticBrick* getSBrickForm( const NLMISC::CSheetId& sheetId );
	
	// Get brick form map reference
//	static inline const CAllBrickSheets& getBrickMapForm() { return _StaticSheets._BrickSheets; }

	/// Get sabrina phrase
	static const CStaticRolemasterPhrase* getSRolemasterPhrase( const NLMISC::CSheetId& sheetId );

	/// Get sabrina phrase form map reference
	static inline const CAllRolemasterPhrases& getSRolemasterPhrasesMap() { return _StaticSheets._SPhraseSheets; }

	/// Get sabrina phrase form vector (for linear browsing) (filled only at startup, excluding .saiphrase)
	static inline const CAllRolemasterPhrasesLinear& getSRolemasterPhrasesVector() { return _StaticSheets._SPhraseSheetVector; }

	// Get form of XpStageTable
	static const CStaticXpStagesTable* getXpStageTableForm( const NLMISC::CSheetId& sheetId );

	// Get form of Skill to Stage type table association
	static const CStaticStagesTypeSkillTable* getStageTypeSkill( const NLMISC::CSheetId& sheetId );

	// Get form of pacts info
	static const CStaticPacts* getPactsForm( const NLMISC::CSheetId& sheetId );

	// Get form of creatures info
	static const CStaticCreatures* getCreaturesForm( const NLMISC::CSheetId& sheetId );

	// Get form of loot set info
	static const CStaticLootSet* getLootSetForm( const NLMISC::CSheetId& sheetId );

	// Get form of loot table info
	static const CStaticLootTable* getLootTableForm( const NLMISC::CSheetId& sheetId );

	// Get form of Stats by race (people) for character create
	static const CStaticRaceStats* getRaceStats( const NLMISC::CSheetId& sheetId );

	// Get form of starting equipment and actions by race/role (people) for character create
	static const CStaticRole* getStartRole( const NLMISC::CSheetId& sheetId );
	static inline std::map< NLMISC::CSheetId,CStaticRole >& getRoleContainer() { return _StaticSheets._StaticRole; }

	// Return reference on map contained all Race stats sheet for character create
	static const std::map< NLMISC::CSheetId, CStaticRaceStats >& getRaceStatsContainer() { return _StaticSheets._StaticRaceStats; }
		
	// Get form of skill tree
	static const CStaticSkillsTree * getSkillsTreeForm( const NLMISC::CSheetId& sheetId );

	// Get form of xp factor table
	static const CStaticXpFactorTable * getXpFactorTableForm( const NLMISC::CSheetId& sheetId );

	// Get form of deposit
	//static const CStaticDeposit * getDepositForm( const NLMISC::CSheetId& sheetId );

	// get form of Ai Action
	static const CStaticAiAction * getAiActionForm( const NLMISC::CSheetId& sheetId );

	// get form of a success table
	static const CStaticSuccessTable * getSuccessTableForm( const NLMISC::CSheetId& sheetId );

	// get form of the world
	static const CStaticWorld * getWorldForm( const NLMISC::CSheetId& sheetId );

	// get form of a continent
	static const CStaticContinent * getContinentForm( const NLMISC::CSheetId& sheetId );

	// get form of an outpost
	static const CStaticOutpost * getOutpostForm( const NLMISC::CSheetId& sheetId );

	// get form of an outpost building
	static const CStaticOutpostBuilding * getOutpostBuildingForm( const NLMISC::CSheetId& sheetId );

	// get form of an outpost squad
	static const CStaticOutpostSquad * getOutpostSquadForm( const NLMISC::CSheetId& sheetId );

	// Get weather sheets
	static const CWeatherSetupSheetBase * getWeatherSetupSheet( const NLMISC::CSheetId& sheetId );
	static const std::map< NLMISC::CSheetId, CWeatherFunctionParamsSheetBase >& getWeatherFunctionParamsSheets() { return _StaticSheets._WeatherFunctionParams; }

	// get text emotes
	static const CStaticTextEmotes & getTextEmoteList()
	{
		return _StaticSheets._TextEmotes;
	}
	
	// get emote anims
	static const CStaticEmot & getEmoteList()
	{
		return _StaticSheets._Emots;
	}
	
	// get encyclopedia part
	static const CStaticEncyclo & getEncyclopedia()
	{
		return *_StaticSheets._Encyclopedia;
	}

	/// \name reloading
	// @{
	static void		reloadCreature(const std::string &wildcardFilter);
	static void		reloadSItem(const std::string &wildcardFilter);
	static void		reloadSBrick(const std::string &wildcardFilter);
	static void		reloadSPhrase(const std::string &wildcardFilter);
	static void		reloadSuccessChancesTable(const std::string &wildcardFilter);
	static void		reloadXPTable(const std::string &wildcardFilter);
	// @}

public:
	/**
	 * Constructor (should be private because this is a singleton)
	 */
	CSheets()	{nlassert(!_Initialised); _Initialised=true;}

	/**
	 * Destructor (should be private because this is a singleton)
	 */
	~CSheets()	{nlassert(_Initialised); nlassert(!_Destroyed); _Destroyed=true;}

	/**
	 * init the static items plan index in the database bitfield
	 */
	static void initBitfieldIndex();

private :
	/// item static infos
	CAllStaticItems _StaticItems;

	/// brick static infos
//	CAllBrickSheets	_BrickSheets;
	CAllStaticBricks _SBrickSheets;

	/// XpStageTable static infos
	std::map< NLMISC::CSheetId, CStaticXpStagesTable > _StaticXpStageTable;

	/// Pacts static infos
	std::map< NLMISC::CSheetId, CStaticPacts > _StaticPacts;

	/// Creatures static infos
	CAllStaticCreatures _StaticCreatures;

	/// Loot set static infos
	std::map< NLMISC::CSheetId, CStaticLootSet > _StaticLootSet;

	/// Loot table static infos
	std::map< NLMISC::CSheetId, CStaticLootTable > _StaticLootTable;

	/// Stats by race (people) for character create
	std::map< NLMISC::CSheetId, CStaticRaceStats > _StaticRaceStats;

	/// Stats by race (people) for character create
	std::map< NLMISC::CSheetId, CStaticRole > _StaticRole;

	/// skill tree
	std::map< NLMISC::CSheetId, CStaticSkillsTree > _SkillsTree;

	/// ai actions
	std::map< NLMISC::CSheetId, CStaticAiAction > _AiActions;

	// success tables
	std::map< NLMISC::CSheetId, CStaticSuccessTable > _SuccessTables;

	/// xp factor tables
	std::map< NLMISC::CSheetId, CStaticXpFactorTable > _StaticXpFactorTables;
	/// continents
	std::map< NLMISC::CSheetId, CStaticContinent > _StaticContinents;

	///  outposts
	std::map< NLMISC::CSheetId, CStaticOutpost > _StaticOutposts;

	///  outposts squads
	std::map< NLMISC::CSheetId, CStaticOutpostSquad > _StaticOutpostSquads;

	/// outposts buildings
	std::map< NLMISC::CSheetId, CStaticOutpostBuilding > _StaticOutpostBuilding;

	/// worlds
	std::map< NLMISC::CSheetId, CStaticWorld > _StaticWorld;

	// For weather
	std::map< NLMISC::CSheetId, CWeatherSetupSheetBase > _WeatherSetups;
	std::map< NLMISC::CSheetId, CWeatherFunctionParamsSheetBase > _WeatherFunctionParams;
	
	/// Rolemaster sphrases
	CAllRolemasterPhrases _SPhraseSheets;

	/// Linear access to rolemaster sphrases (excluding .saiphrase)
	CAllRolemasterPhrasesLinear _SPhraseSheetVector;

	// text emotes list
	CStaticTextEmotes		_TextEmotes;
	// emote anim list
	CStaticEmot				_Emots;
	
	// Static Encyclopedia part
	CStaticEncyclo			*_Encyclopedia;
	
private:
	static CSheets _StaticSheets;	//the singleton instance
	static bool _Initialised;		//default =false - set true by constructor
	static bool _Destroyed;			//default =false - set true by destructor
};

#endif // SHEETS_H

/* End of sheets.h */

