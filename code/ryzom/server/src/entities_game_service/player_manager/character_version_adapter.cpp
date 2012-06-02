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



/////////////
// INCLUDE 
/////////////
#include "stdpch.h"

#include "player_manager/character_version_adapter.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "player_manager/character_encyclopedia.h"
#include "player_manager/player_room.h"

#include "mission_pd.h"
#include "primitives_parser.h"
#include "zone_manager.h"
#include "mission_manager/mission_manager.h"
#include "shop_type/items_for_sale.h"
#include "shop_type/item_for_sale.h"

#include "nel/misc/variable.h"
#include "game_share/visual_slot_manager.h"
#include "game_share/starting_point.h"

#include "egs_sheets/egs_sheets.h"
#include "modules/shard_unifier_client.h"

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

CVariable<sint32> DeltaTickMustBeApplyForWindermeerCommunityMerge("egs","DeltaTickMustBeApplyForWindermeerCommunityMerge", "Delta tick between the 2 server before the merge", 5000000, 0, true );

NL_INSTANCE_COUNTER_IMPL(CCharacterVersionAdapter);

/////////////
// GLOBALS 
/////////////
CCharacterVersionAdapter	*CCharacterVersionAdapter::_Instance = NULL;


//---------------------------------------------------
// currentVersionNumber:
//
//---------------------------------------------------
uint32 CCharacterVersionAdapter::currentVersionNumber() const
{
	////////////////////////////////////
	// VERSION History
	// 0 : 
	// 1 : (04/10/2004) patch bad timers (effects and missions, after a problem with tick save)
	// 2 : (21/10/2004) clear all bricks and phrases and give back skill points
	// 3 : (28/10/2004) clear all missions but the welcome mission
	// 4 : (04/11/2004) give SP for life gift spells (bad dependency)
	// 5 : (18/11/2004) set current title to Homin if player had title Kami_Ally or Karavan_Ally
	// 6 : (25/11/2004) saves must be updated because events mps dont have the right sheet... So we have to replace them
	// 7 : (26/11/2004) saves must be updated because old haircuts must be adapted to new sheets
	// 8 : (29/11/2004) give SP for some mispriced material-specialized prospecting bricks
	// 9 : (10/12/2004) resize pet animal inventories to 256 if necessary

	// 11 : (15/12/2004) adapter to clear player missions and replace them by rites / newbi mission
	// 12 : (25/01/2005) items looted by NPCs were created with 0 HP : patch items with 0 HP
	// 13 : (31/01/2005) Correct the bug where the encyclopedia is not aware of the mission success
	// 14 : (10/03/2005) patch old tools HP, set HP to max HP for items with 0 HP
	// 15 : (29/03/2005) patch tick server for community Windermeer merge to Arispotle community
	// 16 : (17/11/2005) set declared cult to none
	// 17 : (13/12/2005) restore pre order items
	// 18 : (24/02/2006) give full hp to tools
	// 19 : (01/03/2006)
	// 20 : (06/03/2006) give full hp to tools (even in player room and guild inv)
	// 21 : (19/04/2006) convert to position stack & store the mainland in db
	// 22 : (15/05/2006) reset flag pvp for resolve migration timer pb
	////////////////////////////////////
	return 22;
}


//---------------------------------------------------
void CCharacterVersionAdapter::adaptCharacterFromVersion( CCharacter &character, uint32 version ) const
{
	// Do NOT break between case labels
	switch (version)
	{
	case 0: adaptToVersion1(character);
	case 1: adaptToVersion2(character);
    case 2: adaptToVersion3(character);
	case 3: adaptToVersion4(character);
	case 4: adaptToVersion5(character);
	case 5: adaptToVersion6(character);
	case 6: adaptToVersion7(character);
	case 7: adaptToVersion8(character);
	case 8: adaptToVersion9(character);
	case 9: adaptToVersion10(character);
	case 10: adaptToVersion11(character);
	case 11: adaptToVersion12(character);
	case 12: adaptToVersion13(character);
	case 13: adaptToVersion14(character);
	case 14: adaptToVersion15(character);
	case 15: adaptToVersion16(character);
	case 16: adaptToVersion17(character);
	case 17: adaptToVersion18(character);
	case 18: adaptToVersion19(character);
	case 19: adaptToVersion20(character);
	case 20: adaptToVersion21(character);
	case 21: adaptToVersion22(character);
	default:;
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion1(CCharacter &character) const
{
	// clear all timers, for effects
	character._ModifiersInDB.clear();
	character.resetPowerFlags();

	// for missions
	std::map< TAIAlias, TMissionHistory >::iterator itH = character._MissionHistories.begin();
	for ( ; itH != character._MissionHistories.end() ; ++itH)
	{
		TMissionHistory &history = (*itH).second;
		history.LastSuccessDate = 0;		
	}

	const TGameCycle time = CTickEventHandler::getGameCycle();

	std::map<uint32, EGSPD::CMissionPD*>::iterator itM = character._Missions->getMissionsBegin();
	for ( ; itM != character._Missions->getMissionsEnd() ; ++itM)
	{
		EGSPD::CMissionPD * missionPD = (*itM).second;
		if (!missionPD)
			continue;

		if (missionPD->getBeginDate() > time)
		{
			missionPD->setEndDate(time);
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion2(CCharacter &character) const
{
	/****************************************************************/
	/* reset all bricks and actions, set charcateristics to their 
	/* starting values and recompute scores
	/****************************************************************/

	static const NLMISC::CSheetId sheet("skills.skill_tree");
	static const CStaticSkillsTree * skillsTree = CSheets::getSkillsTreeForm( sheet );

	character._SpType[EGSPD::CSPType::Fight] = 10 * skillsTree->getPlayerSkillPointsUnderSkill(&character._Skills, SKILLS::SF);
	character._SpType[EGSPD::CSPType::Magic] = 10 * skillsTree->getPlayerSkillPointsUnderSkill(&character._Skills, SKILLS::SM);
	character._SpType[EGSPD::CSPType::Craft] = 10 * skillsTree->getPlayerSkillPointsUnderSkill(&character._Skills, SKILLS::SC);
	character._SpType[EGSPD::CSPType::Harvest] = 10 * skillsTree->getPlayerSkillPointsUnderSkill(&character._Skills, SKILLS::SH);

	character._Skills._Sp = 0;
	
	// clear all memories... 
	character._MemorizedPhrases.forgetAll();

	// clear bricks and phrases
	character._KnownPhrases.clear();
	character._BoughtPhrases.clear();
	character._KnownBricks.clear();

	for( uint i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		// at level 1 for formula dependency with regenerate value
		character._PhysCharacs._PhysicalCharacteristics[ i ].Base = 10; 
	
		character._PhysCharacs._PhysicalCharacteristics[ i ].Max = character._PhysCharacs._PhysicalCharacteristics[ i ].Base + character._PhysCharacs._PhysicalCharacteristics[ i ].Modifier;
		character._PhysCharacs._PhysicalCharacteristics[ i ].Current = character._PhysCharacs._PhysicalCharacteristics[ i ].Max;
	}
	// Compute Scores
	// !!! DOT NOT change modifiers as they are changed by equipement !!!
	for(uint i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		switch( i )
		{
		case SCORES::hit_points:
			character._PhysScores._PhysicalScores[ i ].Base = character._PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::constitution ].Base * 10;
			break;
		case SCORES::sap:
			character._PhysScores._PhysicalScores[ i ].Base = character._PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::intelligence ].Base * 10;
			break;
		case SCORES::stamina:
			character._PhysScores._PhysicalScores[ i ].Base = character._PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::strength ].Base * 10;
			break;
		case SCORES::focus:
			character._PhysScores._PhysicalScores[ i ].Base = character._PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::dexterity ].Base * 10;
			break;
		default:;
		}
		character.updateRegen();
		character._PhysScores._PhysicalScores[ i ].Max = character._PhysScores._PhysicalScores[ i ].Base + character._PhysScores._PhysicalScores[ i ].Modifier;
		character._PhysScores._PhysicalScores[ i ].Current = character._PhysScores._PhysicalScores[ i ].Max;
		character._PhysScores._PhysicalScores[ i ].CurrentRegenerate = character._PhysScores._PhysicalScores[ i ].BaseRegenerateAction + character._PhysScores._PhysicalScores[ i ].RegenerateModifier;
	}

	character.addCreationBricks();
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion3(CCharacter &character) const
{
	static map<TAIAlias,TAIAlias> welcomeMissionToBot;
	if (welcomeMissionToBot.empty())
	{
		// init our welcome missions map
		for (uint16 i = 0; i < RYZOM_STARTING_POINT::NB_START_POINTS; i++)
		{
			vector<CZoneManager::CStartPoint> startPoints = CZoneManager::getInstance().getStartPointVector( i );
			for (uint j = 0; j < startPoints.size(); j++)
			{
				TAIAlias mission = startPoints[j].Mission;
				TAIAlias bot = startPoints[j].Welcomer;
				if (mission != CAIAliasTranslator::Invalid)
				{
					nlinfo("<WELCOME_MISSIONS> adding welcome mission %s, alias: %s",
						CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId( mission ).c_str(),
						CPrimitivesParser::aliasToString( mission ).c_str()
						);
					welcomeMissionToBot.insert( make_pair(mission, bot) );
				}
			}
		}
	}

	TAIAlias welcomeMission = CAIAliasTranslator::Invalid;
	TAIAlias welcomeBot = CAIAliasTranslator::Invalid;

	// clear all missions
	std::map<uint32,EGSPD::CMissionPD *>::iterator it;
	while (true)
	{
		it = character._Missions->getMissionsBegin();
		if (it == character._Missions->getMissionsEnd())
			break;

		TAIAlias mission = (*it).first;
		map<TAIAlias,TAIAlias>::const_iterator itWel = welcomeMissionToBot.find( mission );
		if (itWel != welcomeMissionToBot.end())
		{
			welcomeMission = mission;
			welcomeBot = (*itWel).second;
		}

/*		nlinfo("<DELETE_MISSIONS> deleting mission %s, alias: %s",
			CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId( mission ).c_str(),
			CPrimitivesParser::aliasToString( mission ).c_str()
			);
*/		character._Missions->deleteFromMissions( mission );
	}

	// give back welcome mission if player had it
	if (welcomeMission != CAIAliasTranslator::Invalid && welcomeBot != CAIAliasTranslator::Invalid)
	{
		character.setWelcomeMissionDesc(welcomeMission, welcomeBot);
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion4(CCharacter &character) const
{
/*	Removed because players are not complaining
//	 give back SP for player who bought life gift when dependencies were bad

	uint16 sp = 0;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00010.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00020.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00030.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00050.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00080.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00120.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00160.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00200.sbrick") ) != character._KnownBricks.end() )
		sp += 10;
	if (character._KnownBricks.find( CSheetId("bmdhtmp00250.sbrick") ) != character._KnownBricks.end() )
		sp += 10;

	character._SpType[EGSPD::CSPType::Magic] += sp;
*/
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion5(CCharacter &character) const
{
	// set current title to Homin if player had title Kami_Ally or Karavan_Ally
	if (character.getTitle() == CHARACTER_TITLE::Kami_Ally || character.getTitle() == CHARACTER_TITLE::Karavan_Ally)
	{
		character.setTitle(CHARACTER_TITLE::Homin);
	}
}


//---------------------------------------------------
void CCharacterVersionAdapter::updateInventoryToVersion6 ( CInventoryBase *inventory, INVENTORIES::TInventory inventoryType , CCharacter * character)const
{
	static const CSheetId bad1("m0308cxxcc01.sitem");
	static const CSheetId bad2("m0308cxxcd01.sitem");
	static const CSheetId bad3("m0308cxxce01.sitem");
	static const CSheetId bad4("m0308cxxcf01.sitem");
	static const CSheetId good("m0308cxxcb01.sitem");

	if ( inventory != NULL )
	{
		const uint size2 = inventory->getSlotCount();
		for ( uint j = 0; j < size2; j++ )
		{
			if ( inventory->getItem(j) != NULL )
			{
				uint quantity = inventory->getItem(j)->getStackSize();
				CSheetId sheet( inventory->getItem(j)->getSheetId() ); 
				if ( sheet == bad1 || sheet == bad2 || sheet == bad3 || sheet == bad4 )
				{
					inventory->deleteItem(j);
					if ( inventoryType == INVENTORIES::guild )
					{
						CGameItemPtr item = GameItemManager.createInGameItem( 200,quantity,good, CEntityId::Unknown, NULL );
						inventory->insertItem(item);
					}
					else if ( character )
						character->createItemInInventory( inventoryType, 200, quantity, good, CEntityId::Unknown );
				}
			}
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion6(CCharacter &character) const
{
	
//	const uint size1 = character._Inventory.size();
	const uint size1 = INVENTORIES::NUM_INVENTORY;
	for ( uint i = 0; i < size1; ++i )
//		updateInventoryToVersion6 ( character._Inventory[i], INVENTORIES::EInventory (i), &character );
		updateInventoryToVersion6 ( character._Inventory[i], INVENTORIES::TInventory (i), &character );
	if ( character.getRoomInterface().isValid() && character.getRoomInterface().getInventory() != NULL )
		updateInventoryToVersion6 ( character.getRoomInterface().getInventory(), INVENTORIES::player_room, &character );

}

//---------------------------------------------------
void initCharacterAdapterToVersion7(std::map<uint,std::string> & assocMale, std::map<uint,std::string> & assocFemale )
{
		assocMale.insert( make_pair( 1 , string("fy_hom_hair_style03.sitem") ) );
		assocMale.insert( make_pair( 2 , string("fy_hom_hair_style02.sitem") ) );
		assocMale.insert( make_pair( 3 , string("fy_hom_hair_basic01.sitem") ) );
		assocMale.insert( make_pair( 4 , string("fy_hom_hair_basic02.sitem") ) );
		assocMale.insert( make_pair( 30 , string("fy_hom_hair_style01.sitem") ) );
		assocMale.insert( make_pair( 31 , string("fy_hom_hair_artistic01.sitem") ) );
		assocMale.insert( make_pair( 32 , string("fy_hom_hair_basic03.sitem") ) );
		assocMale.insert( make_pair( 16 , string("tr_hom_hair_artistic01.sitem") ) );
		assocMale.insert( make_pair( 17 , string("tr_hom_hair_basic03.sitem") ) );
		assocMale.insert( make_pair( 18 , string("tr_hom_hair_basic01.sitem") ) );
		assocMale.insert( make_pair( 19 , string("tr_hom_hair_style01.sitem") ) );
		assocMale.insert( make_pair( 36 , string("tr_hom_hair_style02.sitem") ) );
		assocMale.insert( make_pair( 37 , string("tr_hom_hair_basic02.sitem") ) );
		assocMale.insert( make_pair( 38 , string("tr_hom_hair_artistic02.sitem") ) );
		assocMale.insert( make_pair( 10 , string("ma_hom_hair_style04.sitem") ) );
		assocMale.insert( make_pair( 11 , string("ma_hom_hair_basic02.sitem") ) );
		assocMale.insert( make_pair( 12 , string("ma_hom_hair_basic01.sitem") ) );
		assocMale.insert( make_pair( 13 , string("ma_hom_hair_basic01.sitem") ) );
		assocMale.insert( make_pair( 14 , string("ma_hom_hair_style01.sitem") ) );
		assocMale.insert( make_pair( 33 , string("ma_hom_hair_artistic01.sitem") ) );
		assocMale.insert( make_pair( 34 , string("ma_hom_hair_style02.sitem") ) );
		assocMale.insert( make_pair( 35 , string("ma_hom_hair_style03.sitem") ) );
		assocMale.insert( make_pair( 21 , string("zo_hom_hair_basic02.sitem") ) );
		assocMale.insert( make_pair( 22 , string("zo_hom_hair_style01.sitem") ) );
		assocMale.insert( make_pair( 23 , string("zo_hom_hair_basic01.sitem") ) );
		assocMale.insert( make_pair( 24 , string("zo_hom_hair_basic03.sitem") ) );
		assocMale.insert( make_pair( 39 , string("zo_hom_hair_basic04.sitem") ) );
		assocMale.insert( make_pair( 40 , string("zo_hom_hair_style02.sitem") ) );
		assocMale.insert( make_pair( 41 , string("zo_hom_hair_style03.sitem") ) );


		assocFemale.insert( make_pair( 1 , string("fy_hof_hair_style03.sitem") ) );
		assocFemale.insert( make_pair( 2 , string("fy_hof_hair_basic02.sitem") ) );
		assocFemale.insert( make_pair( 3 , string("fy_hof_hair_basic01.sitem") ) );
		assocFemale.insert( make_pair( 4 , string("fy_hof_hair_style01.sitem") ) );
		assocFemale.insert( make_pair( 30 , string("fy_hof_hair_style02.sitem") ) );
		assocFemale.insert( make_pair( 31 , string("fy_hof_hair_artistic01.sitem") ) );
		assocFemale.insert( make_pair( 32 , string("fy_hof_hair_basic03.sitem") ) );
		assocFemale.insert( make_pair( 16 , string("tr_hof_hair_style01.sitem") ) );
		assocFemale.insert( make_pair( 17 , string("tr_hof_hair_style02.sitem") ) );
		assocFemale.insert( make_pair( 18 , string("tr_hof_hair_basic01.sitem") ) );
		assocFemale.insert( make_pair( 19 , string("tr_hof_hair_artistic03.sitem") ) );
		assocFemale.insert( make_pair( 36 , string("tr_hof_hair_artistic04.sitem") ) );
		assocFemale.insert( make_pair( 37 , string("tr_hof_hair_artistic01.sitem") ) );
		assocFemale.insert( make_pair( 38 , string("tr_hof_hair_artistic02.sitem") ) );
		assocFemale.insert( make_pair( 10 , string("ma_hof_hair_style01.sitem") ) );
		assocFemale.insert( make_pair( 11 , string("ma_hof_hair_artistic01.sitem") ) );
		assocFemale.insert( make_pair( 12 , string("ma_hof_hair_basic01.sitem") ) );
		assocFemale.insert( make_pair( 13 , string("ma_hof_hair_basic01.sitem") ) );
		assocFemale.insert( make_pair( 14 , string("ma_hof_hair_basic02.sitem") ) );
		assocFemale.insert( make_pair( 33 , string("ma_hof_hair_style02.sitem") ) );
		assocFemale.insert( make_pair( 34 , string("ma_hof_hair_artistic02.sitem") ) );
		assocFemale.insert( make_pair( 35 , string("ma_hof_hair_style03.sitem") ) );
		assocFemale.insert( make_pair( 21 , string("zo_hof_hair_style04.sitem") ) );
		assocFemale.insert( make_pair( 22 , string("zo_hof_hair_style02.sitem") ) );
		assocFemale.insert( make_pair( 23 , string("zo_hof_hair_basic01.sitem") ) );
		assocFemale.insert( make_pair( 24 , string("zo_hof_hair_basic02.sitem") ) );
		assocFemale.insert( make_pair( 39 , string("zo_hof_hair_basic03.sitem") ) );
		assocFemale.insert( make_pair( 40 , string("zo_hof_hair_style01.sitem") ) );
		assocFemale.insert( make_pair( 41 , string("zo_hof_hair_style03.sitem") ) );
}


			
//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion7(CCharacter &character) const
{
	static map<uint,string> maleSheets;
	static map<uint,string> femaleSheets;
	if ( maleSheets.empty() )
		initCharacterAdapterToVersion7( maleSheets, femaleSheets );


	map<uint,string>::iterator it;
	if ( character.getGender() == GSGENDER::female )
	{		
		it = femaleSheets.find( character._HairType );
		if ( it == femaleSheets.end() )
		{
			nlwarning( "<adaptToVersion7> invalid hair type %u in char %s (female)",character._HairType, character.getId().toString().c_str() );
			return;
		}
	}
	else
	{		
		it = maleSheets.find( character._HairType );
		if ( it == maleSheets.end() )
		{
			nlwarning( "<adaptToVersion7> invalid hair type %u in char %s (male)",character._HairType, character.getId().toString().c_str() );
			return;
		}
	}
	CSheetId sheet( (*it).second );
	if ( sheet == CSheetId::Unknown )
	{
		nlwarning( "<adaptToVersion7> char %s, sheet '%s' is invalid",character.getId().toString().c_str(), (*it).second.c_str() );
		return;
	}
	uint idx = CVisualSlotManager::getInstance()->sheet2Index( sheet,SLOTTYPE::HEAD_SLOT );
	if ( idx == 0 )
	{
		nlwarning( "<adaptToVersion7> char %s, sheet '%s' has no valid slot index",character.getId().toString().c_str(), (*it).second.c_str() );
		return;
	}
	character._HairType = idx;
	
		/*
		 Below is the association between former visual slot id / old sheet / new shhet

	male

		1 : fy_cheveux_long01.sitem : fy_hom_hair_style03.sitem
		2 : fy_cheveux_medium01.sitem : fy_hom_hair_style02.sitem
		3 : fy_cheveux_shave01.sitem : fy_hom_hair_basic01.sitem
		4 : fy_cheveux_short01.sitem : fy_hom_hair_basic02.sitem
		30 : fy_cheveux_medium02.sitem : fy_hom_hair_style01.sitem
		31 : fy_cheveux_medium03.sitem : fy_hom_hair_artistic01.sitem
		32 : fy_cheveux_short02.sitem : fy_hom_hair_basic03.sitem

		16 : tr_cheveux_long01.sitem : tr_hom_hair_artistic01.sitem
		17 : tr_cheveux_medium01.sitem : tr_hom_hair_basic03.sitem
		18 : tr_cheveux_shave01.sitem : tr_hom_hair_basic01.sitem
		19 : tr_cheveux_short01.sitem : tr_hom_hair_style01.sitem
		36 : tr_cheveux_medium02.sitem : tr_hom_hair_style02.sitem
		37 : tr_cheveux_short02.sitem : tr_hom_hair_basic02.sitem
		38 : tr_cheveux_short03.sitem : tr_hom_hair_artistic02.sitem
		
		
		10 : ma_cheveux_long01.sitem : ma_hom_hair_style04.sitem
		11 : ma_cheveux_medium01.sitem : ma_hom_hair_basic02.sitem
		12 : ma_cheveux_shave01.sitem : ma_hom_hair_basic01.sitem
		13 : ma_cheveux_shave_01.sitem : ma_hom_hair_basic01.sitem
		14 : ma_cheveux_short01.sitem : ma_hom_hair_style01.sitem
		33 : ma_cheveux_long02.sitem : ma_hom_hair_artistic01.sitem
		34 : ma_cheveux_medium02.sitem : ma_hom_hair_style02.sitem
		35 : ma_cheveux_short02.sitem : ma_hom_hair_style03.sitem
		
		
		
		21 : zo_cheveux_long01.sitem : zo_hom_hair_basic02.sitem
		22 : zo_cheveux_medium01.sitem : zo_hom_hair_style01.sitem
		23 : zo_cheveux_shave01.sitem : zo_hom_hair_basic01.sitem
		24 : zo_cheveux_short01.sitem : zo_hom_hair_basic03.sitem
		39 : zo_cheveux_long02.sitem : zo_hom_hair_basic04.sitem
		40 : zo_cheveux_medium02.sitem : zo_hom_hair_style02.sitem
		41 : zo_cheveux_medium03.sitem : zo_hom_hair_style03.sitem






  female


		1 : fy_cheveux_long01.sitem : fy_hof_hair_style03.sitem
		2 : fy_cheveux_medium01.sitem : fy_hof_hair_basic02.sitem
		3 : fy_cheveux_shave01.sitem : fy_hof_hair_basic01.sitem
		4 : fy_cheveux_short01.sitem : fy_hof_hair_style01.sitem
		30 : fy_cheveux_medium02.sitem : fy_hof_hair_style02.sitem
		31 : fy_cheveux_medium03.sitem : fy_hof_hair_artistic01.sitem
		32 : fy_cheveux_short02.sitem : fy_hof_hair_basic03.sitem
		
		16 : tr_cheveux_long01.sitem : tr_hof_hair_style01.sitem  
		17 : tr_cheveux_medium01.sitem : tr_hof_hair_style02.sitem  
		18 : tr_cheveux_shave01.sitem : tr_hof_hair_basic01.sitem
		19 : tr_cheveux_short01.sitem : tr_hof_hair_artistic03.sitem
		36 : tr_cheveux_medium02.sitem : tr_hof_hair_artistic04.sitem
		37 : tr_cheveux_short02.sitem : tr_hof_hair_artistic01.sitem
		38 : tr_cheveux_short03.sitem : tr_hof_hair_artistic02.sitem
		  
			
		10 : ma_cheveux_long01.sitem : ma_hof_hair_style01.sitem
		11 : ma_cheveux_medium01.sitem : ma_hof_hair_artistic_01.sitem
		12 : ma_cheveux_shave01.sitem : ma_hof_hair_basic_01.sitem
		13 : ma_cheveux_shave_01.sitem : ma_hof_hair_basic_01.sitem
		14 : ma_cheveux_short01.sitem : ma_hof_hair_basic_02.sitem
		33 : ma_cheveux_long02.sitem : ma_hof_hair_style02.sitem
		34 : ma_cheveux_medium02.sitem : ma_hof_hair_artistic_02.sitem
		35 : ma_cheveux_short02.sitem : ma_hof_hair_style03.sitem
			  
				
				  
		21 : zo_cheveux_long01.sitem : zo_hof_hair_style_04.sitem
		22 : zo_cheveux_medium01.sitem : zo_hof_hair_style_02.sitem
		23 : zo_cheveux_shave01.sitem : zo_hof_hair_basic_01.sitem
		24 : zo_cheveux_short01.sitem : zo_hof_hair_basic_02.sitem
		39 : zo_cheveux_long02.sitem : zo_hof_hair_basic_03.sitem
		40 : zo_cheveux_medium02.sitem : zo_hof_hair_style_01.sitem
		41 : zo_cheveux_medium03.sitem : zo_hof_hair_style_03.sitem

  */
		
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion8(CCharacter &character) const
{
	const uint NB_MISPRICED_BRICKS = 7;
	const char *mispricedBricks [NB_MISPRICED_BRICKS] =
	{ "bhfprmfma01.sbrick", "bhfprmfmb01.sbrick", "bhfprmfmd01.sbrick",
	  "bhfprmfmf01.sbrick", "bhfprmfmg01.sbrick", "bhfprmfmi01.sbrick", "bhfprmfmj01.sbrick" };
	uint16 sp = 0;
	for ( uint i=0; i!=NB_MISPRICED_BRICKS; ++i )
	{
		if ( character._KnownBricks.find( CSheetId(mispricedBricks[i]) ) != character._KnownBricks.end() )
			sp += 10;
	}
	character._SpType[EGSPD::CSPType::Harvest] += sp;
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion9(CCharacter &character) const
{
//	for (int i = 0; i < MAX_INVENTORY_ANIMAL; i++)
	for (int i = INVENTORIES::pet_animal; i < INVENTORIES::max_pet_animal; i++)
	{
//		CGameItem * petInv = *character._Inventory[INVENTORIES::pet_animal + i];
		CInventoryPtr petInv = character._Inventory[INVENTORIES::TInventory(i)];
		if (!petInv)
			continue;
//		if (petInv->getChildren().size() < 256)
		// TODO : still needed ?
		if (petInv->getSlotCount() < 256)
		{
//			petInv->getChildren().resize(256, NULL);
			petInv->setSlotCount(256);
//			petInv->SlotCount = 256;
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion10(CCharacter &character) const
{
	character._HairCuteDiscount = true;
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion11(CCharacter &character) const
{
	// delete all missions
	std::map<uint32,EGSPD::CMissionPD *>::iterator it;
	while (true)
	{
		it = character._Missions->getMissionsBegin();
		if (it == character._Missions->getMissionsEnd())
			break;
		character._Missions->deleteFromMissions( (*it).first );
	}
	
	// get the region where the user is
	CRegion * region = dynamic_cast<CRegion*> (	CZoneManager::getInstance().getPlaceFromId( character.getCurrentRegion() ) );
	if ( !region )
	{
		nlwarning("<adaptToVersion11> user%s is on invalid region %u",character.getId().toString().c_str(), character.getCurrentRegion()  );
		return;
	}
	
	vector<TAIAlias> bots;
	TAIAlias mission = CAIAliasTranslator::Invalid;
	// if the user is in newbie land, give him the appropriate newbie mission
	if ( region->isNewbieRegion() )
	{
		switch( character.getRace() )
		{
		case EGSPD::CPeople::Fyros :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "FYROS_NEWB_WELCOME_KAEMON_1" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("welcomer_kaemon_1", bots);
			break;
		case EGSPD::CPeople::Matis :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "MATIS_NEWB_WELCOME_BOREA_1" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("welcomer_borea_1", bots);
			break;
		case EGSPD::CPeople::Tryker :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "TRYKER_NEWB_WELCOME_BARKDELL_1" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("welcomer_barkdell_1", bots);
			break;
		case EGSPD::CPeople::Zorai :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "ZORAI_NEWB_WELCOME_SHENG_WO_1" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("welcomer_sheng_wo_1", bots);
			break;	
		}
	}
	// other give him a rite intro mission
	else
	{
		switch( character.getRace() )
		{
		case EGSPD::CPeople::Fyros :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "FYROS_ENCYCLO_TUTORIAL" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("pyr_barman", bots);
			break;
		case EGSPD::CPeople::Matis :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "MATIS_ENCYCLO_TUTORIAL" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("yrkanis_barman", bots);
			break;
		case EGSPD::CPeople::Tryker :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "TRYKER_ENCYCLO_TUTORIAL" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("fairhaven_barman_1", bots);
			break;
		case EGSPD::CPeople::Zorai :
			mission = CAIAliasTranslator::getInstance()->getMissionUniqueIdFromName( "ZORAI_ENCYCLO_TUTORIAL" );
			CAIAliasTranslator::getInstance()->getNPCAliasesFromName("zora_barman", bots);
			break;	
		}
	}
	if ( mission == CAIAliasTranslator::Invalid || bots.empty() )
	{
		nlwarning("<adaptToVersion11> %s cant have newbie/rite mission set. race is '%s' bot vector size is %u mission is %u newbie mission is %u",
			character.getId().toString().c_str(),
			EGSPD::CPeople::toString( character.getRace() ).c_str(),
			bots.size(),
			mission,
			region->isNewbieRegion());
		return;
	}

	nlassert(!bots.empty());
	character.setWelcomeMissionDesc(mission, bots[0]);
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion12(CCharacter &character) const
{
	// parse all inventories and set items to max Hp when current Hp == 0
	const uint sizeInv = INVENTORIES::NUM_INVENTORY;
	for ( uint i = 0; i < sizeInv ; ++i )
	if (character._Inventory[i] != NULL)
	{
		CInventoryPtr childSrc = character._Inventory[i];
		for ( uint j = 0; j < childSrc->getSlotCount(); j++ )
		{
			CGameItemPtr item = childSrc->getItem(j);
			if (item != NULL)
			{
				if (item->getSheetId() != CSheetId("stack.sitem") )
				{
					if (item->durability() == 0 && item->maxDurability() > 0)
					{
						nlinfo("player %s, patching item %s HP", character.getId().toString().c_str(), item->getSheetId().toString().c_str());
						item->addHp(item->maxDurability());
					}
				}
			}
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion13(CCharacter &c) const
{
	CMissionManager *pMM = CMissionManager::getInstance();
	CAIAliasTranslator *pAIAT = CAIAliasTranslator::getInstance();

	std::map<TAIAlias, TMissionHistory>::iterator it = c._MissionHistories.begin();
	while (it != c._MissionHistories.end())
	{
		TMissionHistory &mi = it->second;
		if (mi.Successfull)
		{
			CMissionTemplate *pMT = pMM->getTemplate(it->first);
			if (pMT != NULL)
			{
				string sName = pMT->getMissionName();
				if ((sName == "fyros_encyclo_tutorial") ||
					(sName == "matis_encyclo_tutorial") ||
					(sName == "tryker_encyclo_tutorial") ||
					(sName == "zorai_encyclo_tutorial"))
				{
					TAIAlias aRite01 = pAIAT->getMissionUniqueIdFromName("R_00_01");
					CMissionTemplate *pMTRite01 = pMM->getTemplate(aRite01);
					if (pMTRite01 != NULL)
					{
						TMissionHistory mh;
						mh.Successfull = true;
						mh.LastSuccessDate = CTickEventHandler::getGameCycle();
						c._MissionHistories.insert(pair<TAIAlias, TMissionHistory>(aRite01, mh));
						if (pMTRite01->EncycloAlbum != -1)
						{
							c._EncycloChar->updateTask(	pMTRite01->EncycloAlbum, 
														pMTRite01->EncycloThema, 
														pMTRite01->EncycloTask, 
														2, 
														false);
						}
					}
				}
			}
		}
		++it;
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion14(CCharacter &character) const
{
	// parse all inventories and set items to max Hp when current Hp == 0
	const uint sizeInv = INVENTORIES::NUM_INVENTORY;
	for ( uint i = 0; i < sizeInv ; ++i )
	if (character._Inventory[i] != NULL)
	{
		CInventoryPtr childSrc = character._Inventory[i];
		for ( uint j = 0; j < childSrc->getSlotCount(); j++ )
		{
			CGameItemPtr item = childSrc->getItem(j);
			if (item != NULL)
			{
				if (item->getSheetId() != CSheetId("stack.sitem") )
				{
					if (item->durability() == 0 && item->maxDurability() > 0)
					{
						nlinfo("player %s, patching item %s HP, new value= %u", 
							character.getId().toString().c_str(), item->getSheetId().toString().c_str(), item->maxDurability());
						item->addHp(item->maxDurability());
					}
				}
			}
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion15(CCharacter &character) const
{
	CPlayer * p = PlayerManager.getPlayer( PlayerManager.getPlayerId( character.getId() ) );
	if( p && DeltaTickMustBeApplyForWindermeerCommunityMerge != 0 )
	{
		// update dead mektoubs DeathTick value
		for( uint32 i = 0; i < character.getPlayerPets().size(); ++i )
		{
			if( character.getPlayerPets()[ i ].PetStatus == CPetAnimal::death )
			{
				const_cast<NLMISC::TGameCycle&> (character.getPlayerPets()[ i ].DeathTick) += DeltaTickMustBeApplyForWindermeerCommunityMerge;
			}
		}
		
		// update start sale cycle for item in sale store
		for( uint32 i = 0; i < character.getItemInShop().getContent().size(); ++i )
		{
			TItemTradePtr itmPtr = const_cast<TItemTradePtr&>(character.getItemInShop().getContent()[ i ]);
			itmPtr->setStartSaleCycle( itmPtr->getStartSaleCycle() + DeltaTickMustBeApplyForWindermeerCommunityMerge );
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion16(CCharacter &character) const
{
	// Set faction allegiance to PVP_CLAN::None.  This is a "limbo" status for existing characters.
	character._DeclaredCult = PVP_CLAN::None;
	character._DeclaredCiv = PVP_CLAN::None;
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion17(CCharacter &character) const
{
	// only for pre-order players
	CPlayer * p = PlayerManager.getPlayer( PlayerManager.getPlayerId( character.getId() ) );
	if (p != NULL && p->isPreOrder())
	{
		INVENTORIES::TInventory inventories[] =
		{
			INVENTORIES::bag,
			INVENTORIES::pet_animal1,
			INVENTORIES::pet_animal2,
			INVENTORIES::pet_animal3,
			INVENTORIES::pet_animal4,
			INVENTORIES::player_room
		};

		CSheetId preOrderSheet("pre_order.sitem");
		BOMB_IF(preOrderSheet == CSheetId::Unknown, "cannot find pre_order.sitem!", return);

		// restore pre-order item hp to the max if any
		bool foundPreOrderItem = false;
		for (uint i = 0; i < sizeof(inventories)/sizeof(inventories[0]); ++i)
		{
			CInventoryPtr inv = character.getInventory(inventories[i]);
			if (inv == NULL)
				continue;

			for (uint slot = 0; slot < inv->getSlotCount(); ++slot)
			{
				CGameItemPtr item = inv->getItem(slot);
				if (item == NULL)
					continue;

				if (item->getSheetId() == preOrderSheet)
				{
					foundPreOrderItem = true;
					item->addHp(item->maxDurability());
				}
			}
		}

		// give a new pre-order item to players who lost it
		if (!foundPreOrderItem)
		{
			CGameItemPtr item = character.createItem(10, 1, preOrderSheet);
			BOMB_IF(item == NULL, "cannot create pre_order.sitem!", return);

			if (!character.addItemToInventory(INVENTORIES::bag, item))
				if (!character.addItemToInventory(INVENTORIES::temporary, item))
					item.deleteItem();
		}
	}
}


//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion18(CCharacter &character) const
{
	// parse all inventories and set tools to max Hp
	const uint sizeInv = INVENTORIES::NUM_INVENTORY;
	for ( uint i = 0; i < sizeInv ; ++i )
	{
		if (character._Inventory[i] != NULL)
		{
			CInventoryPtr childSrc = character._Inventory[i];
			for ( uint j = 0; j < childSrc->getSlotCount(); j++ )
			{
				CGameItemPtr item = childSrc->getItem(j);
				if (item != NULL)
				{
					const CStaticItem * form = CSheets::getForm( item->getSheetId() );
					if( form )
					{
						if( form->Family == ITEMFAMILY::CRAFTING_TOOL || form->Family == ITEMFAMILY::HARVEST_TOOL )
						{
							if (item->maxDurability() > 0)
							{
								nlinfo("player %s, patching tool %s HP, new value= %u", 
									character.getId().toString().c_str(), item->getSheetId().toString().c_str(), item->maxDurability());
								item->addHp(item->maxDurability());
							}
						}
					}
				}
			}
		}
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion19(CCharacter &character) const
{
	if( character.getPVPFlag() == true )
	{
		if( character.getAllegiance().first == PVP_CLAN::None || character.getAllegiance().second == PVP_CLAN::None )
		{
			character.resetPvPFlag();
		}
	}
}


//---------------------------------------------------
void CCharacterVersionAdapter::setToolsToMaxHP(CInventoryBase * pInv) const
{
	if( pInv != NULL )
	{
		for ( uint j = 0; j < pInv->getSlotCount(); j++ )
		{
			CGameItemPtr item = pInv->getItem(j);
			if (item != NULL)
			{
				const CStaticItem * form = CSheets::getForm( item->getSheetId() );
				if( form )
				{
					if( form->Family == ITEMFAMILY::CRAFTING_TOOL || form->Family == ITEMFAMILY::HARVEST_TOOL )
					{
						if (item->maxDurability() > 0)
						{
							nlinfo("patching tool %s HP, new value= %u", item->getSheetId().toString().c_str(), item->maxDurability());
							item->addHp(item->maxDurability());
						}
					}
				}
			}
		}
	}
}


//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion20(CCharacter &character) const
{
	nlinfo("patching tools of player %s", character.getId().toString().c_str());

	// player inventories
	const uint sizeInv = INVENTORIES::NUM_INVENTORY;
	for ( uint i = 0; i < sizeInv ; ++i )
	{
		setToolsToMaxHP( character._Inventory[i] );
	}

	// player room
	if( character._PlayerRoom )
	{
		setToolsToMaxHP( character._PlayerRoom->getInventory() );
	}
	
}

//---------------------------------------------------
extern NLMISC::CVariable<uint32> FixedSessionId;

void CCharacterVersionAdapter::adaptToVersion21(CCharacter &character) const
{
	// If loading an old file with no normal positions, "regularize" the position stack
	if ( character.PositionStack.empty() )
	{
		if ( IsRingShard )
		{
			nlwarning( "This conversion must be done on the mainland shard holding the character file (%s)", character.getId().toString().c_str() );
			return;
		}
		TSessionId homeMainlandForThisChar = TSessionId(FixedSessionId.get());
		if ( homeMainlandForThisChar == TSessionId(0) )
			nlerror( "This conversion must be done on the mainland shard holding the character file (%s)", character.getId().toString().c_str() );

		// Set the session id and push the current state
		// THIS *MUST* BE DONE ON THE MAINLAND SHARD HOLDING THE CHARACTER FILE
		character.setSessionId( homeMainlandForThisChar );
		character.pushCurrentPosition();

//		IShardUnifierEvent::getInstance()->onUpdateCharHomeMainland( character.getId(), homeMainlandForThisChar );
	}
}

//---------------------------------------------------
void CCharacterVersionAdapter::adaptToVersion22(CCharacter &character) const
{
	character.resetPVPTimers();
}
