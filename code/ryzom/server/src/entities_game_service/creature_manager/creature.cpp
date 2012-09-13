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

#include "egs_log_filter.h"
#include "nel/misc/command.h"
#include "creature_manager/creature.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "egs_mirror.h"
#include "egs_sheets/egs_sheets.h"
#include "mission_manager/mission_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "phrase_manager/s_effect.h"
#include "team_manager/team_manager.h"
#include "creature_manager/creature_manager.h"
#include "mission_manager/mission_template.h"
#include "egs_variables.h"
#include "building_manager/building_manager.h"
#include "zone_manager.h"
#include "creature_tick_update_timer_event.h"
#include "world_instances.h"
#include "shop_type/merchant.h"
#include "shop_type/shop_type_manager.h"
#include "shop_type/static_items.h"
#include "shop_type/named_items.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"

#include "modules/easter_egg.h"
#include "modules/r2_give_item.h"

//misc
#include "nel/misc/random.h"
#include "nel/misc/config_file.h"
#include "nel/misc/algo.h"

// Game Share
#include "game_share/slot_equipment.h"
#include "game_share/bot_chat_types.h"

//Nel georges
#include "nel/georges/u_form.h"


// Ring
#include "server_share/r2_variables.h"

#include "game_item_manager/game_item_manager.h"
#include "entity_matrix.h"
#include "server_share/log_item_gen.h"
#include "egs_dynamic_sheet_manager.h"

#include <string>

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;

extern CRandom				RandomGenerator;
extern CBSAIDeathReport		BotDeathReport;
extern CCreatureDespawnMsg	CreatureDespawnMsg;

extern CVariable<bool> VerboseQuartering;
extern std::string CurrentCreatureSpawningDebug;

NL_INSTANCE_COUNTER_IMPL(CCreature);

//-----------------------------------------------
// Constructor
// 
//-----------------------------------------------
CCreature::CCreature() : CEntityBase(true)
{
	_TickUpdateTimer.setRemaining( 1, new CCreatureTickUpdateTimerEvent(this),63 );

	_AIGroupAlias = CAIAliasTranslator::Invalid;
	_AIAlias = CAIAliasTranslator::Invalid;
	_LootInventory = 0;
//	harvestable( false );
	_EntityLooter = CEntityId::Unknown;
	_LootSlotCount = 0;
	_LastCycleUpdateSelectors = 0;
	_LastCycleUpdateTradeList = 0;
	_BotChatProgram = 0;

	_Merchant = 0;

	_FightAction = false;
	_MagicAction = false;
	_CraftAction = false;
	_HarvestAction = false;
	_CharacteristicsSeller = false;
	_GuildBuilding = NULL;
	_PlayerBuilding = NULL;
	_OutpostBuilding = NULL;
	_GuildRoleMasterType = EGSPD::CSPType::EndSPType;
	_FilterExplicitActionTradeByPlayerRace= false;
	_ExplicitActionSPType= EGSPD::CSPType::Unknown;
	_FilterExplicitActionTradeByBotRace= true;

	_NbOfPlayersInAggroList = 0;

	_Form = 0;

	_IsAPet = false;

	_DespawnRequested = false;
	_DespawnSentToAI = false;

//#ifdef NL_DEBUG
	// Looking for a 'Zombie' bug (creature dead on EGS, but not on AIS)
	_DeathReportHasBeenPushed = false;
	_DeathReportHasBeenSent = false;
	_DeathDate = 0;

	_MoneyHasBeenLooted = false;
//#endif
	// god mod inactive
	_GodMode = false;

	/// Altar selector
	_TicketClanRestriction = PVP_CLAN::None;
	_TicketForNeutral = false;
	_TicketFameRestriction = CStaticFames::INVALID_FACTION_INDEX;
	_TicketFameRestrictionValue = 0;
	_MaxHitRangeForPC = -1.f;

	_Organization = 0;
//	_MissionIconFlags.IsMissionStepIconDisplayable = true;
//	_MissionIconFlags.IsMissionGiverIconDisplayable = true;
}

//-----------------------------------------------
// Destructor
// 
//-----------------------------------------------
CCreature::~CCreature()
{
	CR2GiveItem::getInstance().onUnspawn( getEntityRowId() );

	removeAllSpells();

	_TickUpdateTimer.reset();

	if( _EntityLooter != NLMISC::CEntityId::Unknown )
	{
		if( _EntityLooter.getType() == RYZOMID::player )
		{
			CCharacter * character = PlayerManager.getChar( _EntityLooter );
			if( character )
			{
				character->pickUpItemClose();
			}
		}
	}

	if( _LootInventory!=NULL )
	{
//		GameItemManager.destroyItem( _LootInventory );
		// this will delete the inventory (if last pointer)
		_LootInventory = NULL;
	}

	if( TheDataset.isAccessible(_RiderEntity()))
	{
		CCharacter *e = PlayerManager.getChar( getRiderEntity() );
		if ( e )
		{
			e->unmount();
		}
	}

	// if beast was in a train disband it
	disbandTrain();

	// remove entity from PhraseManager
	CPhraseManager::getInstance().removeEntity(_EntityRowId, true);

	// Note: Temp-storage properties in CEntityBase need not be manually deleted here
	// because they were mirrorized as soon the object was created

	// remove xp gain structure on this creature and all data related to it
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->removeCreature(_EntityRowId);
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->removeCreature(this);

	CAIAliasTranslator::getInstance()->removeAssociation(_Id);

	if(_Merchant)
	{
		_Merchant->clearCreaturePtr();
	}
}

//-----------------------------------------------
// getCopy:
// 
//-----------------------------------------------
CCreature * CCreature::getCreatureCopy( const NLMISC::CEntityId & entityId, sint32 cellId  )
{
	// allocate a new creature
	CCreature * creature = new CCreature();
	//set its id
	creature->setId(entityId);
	// add it to mirror and init mirror properties
	Mirror.addEntity( false, const_cast<CEntityId&>(entityId) );
	TDataSetRow entityRowId = TheDataset.getDataSetRow( entityId );
	creature->addPropertiesToMirror( entityRowId, true );
	
	// copy properties not copied via the operator
	CMirrorPropValue<TYPE_SHEET> valueDest( TheDataset, entityRowId, DSPropertySHEET_SERVER );
	CMirrorPropValue<TYPE_SHEET> valueSrc( TheDataset, _EntityRowId, DSPropertySHEET_SERVER );
	valueDest = valueSrc.getValue();
	
	CMirrorPropValue<TYPE_SHEET> value2Dest( TheDataset, entityRowId, DSPropertySHEET );
	CMirrorPropValue<TYPE_SHEET> value2Src( TheDataset, _EntityRowId, DSPropertySHEET );
	value2Dest = value2Src.getValue();
	
	CMirrorPropValue<SAltLookProp> value3Dest( TheDataset, entityRowId, DSPropertyVPA );
	CMirrorPropValue<SAltLookProp> value3Src( TheDataset, _EntityRowId, DSPropertyVPA );
	value3Dest = value3Src.getValue();

	CMirrorPropValue<sint32> value4Dest( TheDataset, entityRowId, DSPropertyCURRENT_HIT_POINTS );
	CMirrorPropValue<sint32> value4Src( TheDataset, _EntityRowId, DSPropertyCURRENT_HIT_POINTS );
	value4Dest = value4Src.getValue();

	CMirrorPropValue<sint32> value5Dest( TheDataset, entityRowId, DSPropertyMAX_HIT_POINTS );
	CMirrorPropValue<sint32> value5Src( TheDataset, _EntityRowId, DSPropertyMAX_HIT_POINTS );
	value5Dest = value5Src.getValue();

	CMirrorPropValue<TYPE_SHEET> value6Dest( TheDataset, entityRowId, DSPropertySHEET );
	CMirrorPropValue<TYPE_SHEET> value6Src( TheDataset, _EntityRowId, DSPropertySHEET );
	value5Dest = value5Src.getValue();

	// set the cell (TODO: use message to GPMS)
	CMirrorPropValue<TYPE_CELL> valueCell( TheDataset, entityRowId, DSPropertyCELL );
	valueCell = cellId;
	
	// register bot name in IOS
	CMirrorPropValue<TYPE_NAME_STRING_ID> valueNameId( TheDataset, _EntityRowId, DSPropertyNAME_STRING_ID );
	uint32 newEntityNameId = valueNameId.getValue();
	CMessage msgName("CHARACTER_NAME_ID");
	msgName.serial(const_cast<TDataSetRow&>(entityRowId));
	msgName.serial( newEntityNameId );
	sendMessageViaMirror ("IOS", msgName);
		
	// copy entity base persistant data
	COfflineEntityState offState;
	offState.X = _EntityState.X;
	offState.Y = _EntityState.Y;
	offState.Z = _EntityState.Z;
	offState.Heading = _EntityState.Heading;
	creature->setState( offState );
	
	creature->_UserModelId = _UserModelId;
	creature->_CustomLootTableId = _CustomLootTableId;
	creature->_PrimAlias = _PrimAlias;
	creature->_Name = _Name;
	creature->_Race = _Race;
	
	creature->_Gender = _Gender;
	
	creature->_Size = _Size;

	creature->_ProtectedSlot = _ProtectedSlot;
	
	creature->_DodgeAsDefense = _DodgeAsDefense;
	
	creature->_LootTables.resize( _LootTables.size() );
	for( uint i=0; i<_LootTables.size(); ++i )
	{
		creature->_LootTables[i] = _LootTables[i];
	}
	if ( _LootInventory != NULL )
	{
		creature->_LootInventory = _LootInventory->getInventoryCopy();
		creature->_EntityLooter = CEntityId::Unknown;
	}
	else
		creature->_LootInventory = NULL;
	creature->_AIGroupAlias = _AIGroupAlias;
	creature->_AIAlias = _AIAlias;				
	creature->_MissionsProposed = _MissionsProposed;
	creature->_WebPage = _WebPage;
	creature->_WebPageName = _WebPageName;
	creature->_BotChatCategory = _BotChatCategory;			
	creature->_RmShopSelector = _RmShopSelector;
	creature->_OriginShopSelector = _OriginShopSelector;
	creature->_QualityShopSelector = _QualityShopSelector;
	creature->_LevelShopSelector = _LevelShopSelector;
	creature->_LastCycleUpdateSelectors = _LastCycleUpdateSelectors;
	creature->_LastCycleUpdateTradeList = _LastCycleUpdateTradeList;

	creature->_ExplicitActionTradeList = _ExplicitActionTradeList;
	creature->_FilterExplicitActionTradeByPlayerRace = _FilterExplicitActionTradeByPlayerRace;
	creature->_ExplicitActionSPType = _ExplicitActionSPType;
	creature->_FilterExplicitActionTradeByBotRace = _FilterExplicitActionTradeByBotRace;
	creature->_FightAction = _FightAction;
	creature->_MagicAction = _MagicAction;
	creature->_CraftAction = _CraftAction;
	creature->_HarvestAction = _HarvestAction;
	creature->_CharacteristicsSeller = _CharacteristicsSeller;
	creature->_GuildCreator = _GuildCreator;

	creature->_BotChatOutpost = _BotChatOutpost;
	
	creature->_BotChatProgram = _BotChatProgram;

	creature->loadSheetCreature( CSheetId(_SheetId())  );
	creature->getContextualProperty().directAccessForStructMembers() = getContextualProperty().directAccessForStructMembers();
	
//	Following property must be initialized before
//	_CharacterLeaderIndex


	creature->_WelcomePhrase = _WelcomePhrase;

	creature->_ContextTexts = _ContextTexts;

	creature->_FactionAttackableAbove = _FactionAttackableAbove;
	creature->_FactionAttackableBelow = _FactionAttackableBelow;


	if ( _RightHandItem != NULL )
		creature->_RightHandItem = _RightHandItem->getItemCopy();
	if ( _LeftHandItem != NULL )
		creature->_LeftHandItem = _LeftHandItem->getItemCopy();

	// _LootRight ->no opponent for the copy
	_LootRightDuration = 0;


	////////////////// copy entity base members ////////////////////////
	// _EntityRowId : dont change
	
	creature->_Items = _Items;

	//mode
	CMirrorPropValue<MBEHAV::TMode> modeMirror( TheDataset, entityRowId, DSPropertyMODE );
	MBEHAV::TMode fullmode;
	fullmode.setModeAndPos( MBEHAV::NORMAL, entityRowId );
	modeMirror = fullmode;
	
	creature->_StaticContextualProperty = _StaticContextualProperty;	

	creature->_GuildBuilding = _GuildBuilding;
	creature->_GuildRoleMasterType = _GuildRoleMasterType;
	
	creature->_GodMode = _GodMode;
	creature->_Invulnerable = _Invulnerable;

//	creature->_MissionIconFlags.IsMissionStepIconDisplayable = _MissionIconFlags.IsMissionStepIconDisplayable;
//	creature->_MissionIconFlags.IsMissionGiverIconDisplayable = _MissionIconFlags.IsMissionGiverIconDisplayable;

	creature->mirrorizeEntityState();
	CreatureManager.addCreature( entityId, creature );
	return creature;
}

//-----------------------------------------------
// addPropertiesToMirror :
// 
//-----------------------------------------------
void CCreature::addPropertiesToMirror( const TDataSetRow& entityIndex, bool keepSheetId )
{
	// Add properties of CEntityBase
	// addPropertiesToMirror() sets _SheetId to point to the SHEET property, but it will be changed by setServerSheet()
	CEntityBase::addPropertiesToMirror( entityIndex, keepSheetId );

	// the values are set by the AIS(cf outpost squad) so we don't overwrite them
	_OutpostInfos.init( TheDataset, entityIndex, DSPropertyOUTPOST_INFOS );
	_OutpostAlias.init( TheDataset, entityIndex, DSPropertyIN_OUTPOST_ZONE_ALIAS );
	_OutpostSide.init( TheDataset, entityIndex, DSPropertyIN_OUTPOST_ZONE_SIDE );

	updateOutpostAliasClientVP();
	updateOutpostSideClientVP();
	
	_ContextualProperty.init( TheDataset, entityIndex, DSPropertyCONTEXTUAL );
}


//-----------------------------------------------
// setServerSheet : Reassign _SheetId to server sheet
// 
//-----------------------------------------------
void CCreature::setServerSheet()
{
	// Now _SheetId will point to the SHEET_SERVER property, instead of SHEET (because it's a creature)
	_SheetId.changeLocation( TheDataset, _EntityRowId, DSPropertySHEET_SERVER );
}

void CCreature::setUserModelId(const std::string &id)
{
	_UserModelId = id;
}

void CCreature::setCustomLootTableId(const std::string &id)
{
	_CustomLootTableId = id;
}

void CCreature::setPrimAlias(uint32 alias)
{
	_PrimAlias = alias;
}

//init form pointer : if a user model is defined for the spawned npc, then retrieve data from 
//the modified sheet stored in the UserModelManager
void CCreature::initFormPointer(NLMISC::CSheetId sheetId)
{
	if (_UserModelId.empty() || _PrimAlias == 0)
	{
		_Form = CSheets::getCreaturesForm( sheetId );
		_SheetName = sheetId.toString();
	}
	else
	{
		_Form = CDynamicSheetManager::getInstance()->getDynamicSheet(_PrimAlias, _UserModelId);
	}
}

//method called by NLMISC_COMMAND displayModifiedEntity to dump all attributes which can be potentially modified by
//a userModel script
void CCreature::displayModifiedAttributes(CEntityId id, NLMISC::CLog &log)
{	
	const CStaticCreatures *baseForm = CSheets::getCreaturesForm(CSheetId(_SheetId));
	if(baseForm)
	{
		log.displayNL("<CCreature::displayModifiedAttributes> Entity %s modified by script %s", id.toString().c_str(), _UserModelId.c_str());
		log.displayNL("Race : %s [%s]", EGSPD::CPeople::toString(_Race).c_str(), EGSPD::CPeople::toString(baseForm->getRace()).c_str());
		log.displayNL("Size : %i [%i]", _Size, baseForm->getSize());
		log.displayNL("Faction : %d [%d]", _Faction, baseForm->getFaction());
		log.displayNL("FameByKill : %d [%d]", _FameByKill, baseForm->getFameByKill());
		log.displayNL("HP : %d [%d]", _PhysScores._PhysicalScores[SCORES::hit_points].Base, baseForm->getScores(SCORES::hit_points));
		//attributes only present in sheets, are they really used?
		log.displayNL("Ecosystem : %s [%s]", ECOSYSTEM::toString(_Form->getEcosystem()).c_str(), ECOSYSTEM::toString(baseForm->getEcosystem()).c_str());
		/*log.displayNL("NbPlayers : %i [%i]", _NbPlayers, ???);
		log.displayNL("PlayerHpLevel: %d [%d]", _);
		log.displayNL("NbHitToKillPlayer: %d [%d]", _);
		log.displayNL("AttackLevel : %i [%i]", _AttackLevel, baseForm->getAttackLevel());
		log.displayNL("AttackSpeed : %d [%d]", _AttackLatency, baseForm->getAttackLatency());
		log.displayNL("DefenseLevel");
		log.displayNL("XPLevel : %i [%i]", _XPLevel, baseForm->getXPLevel());
		log.displayNL("TauntLevel : %i [%i]", _TauntLevel, baseForm->getTauntLevel());
		log.displayNL("XPGainOnCreature : %f [%f]", _XPGainOnCreature, baseForm->getXPGainOnCreature());*/
		log.displayNL("Regen: %f [%f]",_PhysScores._PhysicalScores[SCORES::hit_points].CurrentRegenerate, baseForm->getRegen(SCORES::hit_points));
		log.displayNL("DodgeAsDefense : %d [%d]", _DodgeAsDefense, baseForm->getDodgeAsDefense());
	}
}
//-----------------------------------------------
// loadSheetCreature: Load George Sheet
// 
//-----------------------------------------------
void CCreature::loadSheetCreature( const TDataSetRow& entityIndex )
{
	loadSheetCreature(CSheetId(_SheetId) );
}

//-----------------------------------------------
// loadSheetCreature: Load George Sheet
// 
//-----------------------------------------------
void CCreature::loadSheetCreature( NLMISC::CSheetId sheetId )
{
	// CEntityBase::loadSheetEntity( sheetId ); // Removed because the sheet is already in mirror, replaced by:
	//_Form = CSheets::getCreaturesForm( sheetId );
	initFormPointer(sheetId);
	if ( _Form == 0  )
	{
		nlwarning("<CCreature::loadSheetCreature> Creature %s has spawned with unknown static form for Sheet %d ", _Id.toString().c_str(), sheetId.asInt());
		return;
	}
//	nldebug("<CCreature::loadSheetCreature> Creature '%s' has spawned with custom loot table '%s'", _Id.toString().c_str(), _CustomLootTableId.c_str());
	///////////////////////////////////////////////////////
	// Race
	///////////////////////////////////////////////////////
	_Race = _Form->getRace();

	///////////////////////////////////////////////////////
	// Gender
	///////////////////////////////////////////////////////
	_Gender = _Form->getGender();

	///////////////////////////////////////////////////////
	// Creature Size
	///////////////////////////////////////////////////////
	_Size = _Form->getSize();

	///////////////////////////////////////////////////////
	// Characteristics
	///////////////////////////////////////////////////////
	uint i;
	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		_PhysCharacs._PhysicalCharacteristics[ i ].Base = _Form->getCharacteristics( i );
		_PhysCharacs._PhysicalCharacteristics[ i ].Current = _Form->getCharacteristics( i );
		_PhysCharacs._PhysicalCharacteristics[ i ].Max = _Form->getCharacteristics( i );
		
		_PhysCharacs._PhysicalCharacteristics[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ i ].BaseRegenerateAction = 0;
	}

	///////////////////////////////////////////////////////
	// Derivated Scores
	///////////////////////////////////////////////////////
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		_PhysScores._PhysicalScores[ i ].Base = _Form->getScores( i );
		_PhysScores._PhysicalScores[ i ].Current = _Form->getScores( i );
		_PhysScores._PhysicalScores[ i ].Max = _Form->getScores( i );
		
		_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _Form->getRegen( i );
		_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _Form->getRegen( i );
	}

	if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Base == 0 || _PhysScores._PhysicalScores[ SCORES::hit_points ].Current == 0)
	{
		const sint32 score = (sint32) ( (2*_Form->getDefenseLevel()*_Form->getNbPlayers() + 0.5)*(MinDamage + DamageStep * _Form->getDefenseLevel())*0.65 );
		nlwarning("CREATURE : spawning a creature with 0 Hit Points ! (SHEET %s), set the hp to %d", sheetId.toString().c_str(), score);
		_PhysScores._PhysicalScores[ SCORES::hit_points ].Base = max(score,sint32(1));
		_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = max(score,sint32(1));
		_PhysScores._PhysicalScores[ SCORES::hit_points ].Max = max(score,sint32(1));
	}
	
	///////////////////////////////////////////////////////
	// Items
	///////////////////////////////////////////////////////
	// Headdress
	_Items.Headdress = _Form->getItems( SLOT_EQUIPMENT::HEADDRESS );
	// Body
	_Items.Chest = _Form->getItems( SLOT_EQUIPMENT::CHEST );
	// Legs
	_Items.Legs = _Form->getItems( SLOT_EQUIPMENT::LEGS );
	// Arms
	_Items.Arms = _Form->getItems( SLOT_EQUIPMENT::ARMS );
	// Hands
	_Items.Hands = _Form->getItems( SLOT_EQUIPMENT::HANDS );
	// Feet
	_Items.Feet = _Form->getItems( SLOT_EQUIPMENT::FEET );
	// Head
	_Items.Head = _Form->getItems( SLOT_EQUIPMENT::HEAD );
	// Earl
	_Items.EarL = _Form->getItems( SLOT_EQUIPMENT::EARL );
	// EarR
	_Items.EarR = _Form->getItems( SLOT_EQUIPMENT::EARR );
	// Neck
	_Items.Neck = _Form->getItems( SLOT_EQUIPMENT::NECKLACE );
	// WristL
	_Items.WristL = _Form->getItems( SLOT_EQUIPMENT::WRISTL );
	// WristR
	_Items.WristR = _Form->getItems( SLOT_EQUIPMENT::WRISTR );
	// FingerL
	_Items.FingerL = _Form->getItems( SLOT_EQUIPMENT::FINGERL );
	// FingerR
	_Items.FingerR = _Form->getItems( SLOT_EQUIPMENT::FINGERR );
	// AnkleL
	_Items.AnkleL = _Form->getItems( SLOT_EQUIPMENT::ANKLEL );
	// AnkleR
	_Items.AnkleR = _Form->getItems( SLOT_EQUIPMENT::ANKLER );


	///////////////////////////////////////////////////////
	// Loot Table
	///////////////////////////////////////////////////////
	_LootTables.resize( _Form->getLootTableCount() );
	for( i=0; i<_Form->getLootTableCount(); ++i )
	{
		if( !_Form->getLootTable(i).empty() )
		{
			_LootTables[i] = CSheetId( _Form->getLootTable(i) );
		}
	}
	
	///////////////////////////////////////////////////////
	// Harvest
	///////////////////////////////////////////////////////
	if (VerboseQuartering)
		CurrentCreatureSpawningDebug = sheetId.toString().c_str();
	setMps( _Form->getMps() );
//	_Harvester = NULL;

	///////////////////////////////////////////////////////
	// Movement Speed
	///////////////////////////////////////////////////////
	_PhysScores.BaseWalkSpeed = _Form->getWalkSpeed();
	_PhysScores.BaseRunSpeed = _Form->getRunSpeed();
	_PhysScores.CurrentWalkSpeed = _PhysScores.BaseWalkSpeed;
	_PhysScores.CurrentRunSpeed = _PhysScores.BaseRunSpeed;

	///////////////////////////////////////////////////////
	// Contextual properties
	///////////////////////////////////////////////////////
	_StaticContextualProperty = _ContextualProperty = _Form->getProperties();

	///////////////////////////////////////////////////////
	// Damage Shield
	///////////////////////////////////////////////////////
	_DamageShieldDamage = _Form->getDamageShieldDamage();
	_DamageShieldHpDrain= _Form->getDamageShieldHpDrain();

	///////////////////////////////////////////////////////
	// defense mode
	///////////////////////////////////////////////////////
	_DodgeAsDefense = _Form->getDodgeAsDefense();
}



//---------------------------------------------------
// fillLootInventory
//
//---------------------------------------------------
void CCreature::fillLootInventory( CSheetId& lootTable, const CStaticItem * bagForm, bool& haveLoot )
{
	H_AUTO(fillLootInventory);

	nlassert(bagForm);
	bool isCustom = !_CustomLootTableId.empty();
	
	const CStaticLootTable* loot;
	// Get form of loot table info
	if (isCustom == true)
	{
		//if the loot table is a custom loot table 
		loot = CDynamicSheetManager::getInstance()->getLootTable(_PrimAlias, _CustomLootTableId);
		nldebug("<CCreature::fillLootInventory> Retrieving custom loot table with id '%s' and primAlias '%u'", _CustomLootTableId.c_str(), _PrimAlias);
	}
	else
	{
		loot = CSheets::getLootTableForm( lootTable );
	}
	if( loot == NULL )
	{
		return;
	}

	const CStaticLootSet * lootSet = NULL;
	// select a loot set from loot table
	if (isCustom == true)
	{
		lootSet = loot->selectRandomCustomLootSet();
		if( lootSet==NULL )
		{
			return;
		}
	}
	else
	{
		CSheetId lootSetId = loot->selectRandomLootSet();
		if( lootSetId == CSheetId::Unknown ) // ghost loot set means no items
		{
			return;
		}
		lootSet = CSheets::getLootSetForm( lootSetId );
	}

	if( lootSet==NULL )
	{
		nlwarning("<CCreature::fillLootInventory> Loot table '%s' assigned to creature '%s' not found !", lootTable.toString().c_str(), _SheetId.toString().c_str() );
		return;
	}

	// create the objects from the loot set
	sint16 i = 0;
	for( vector< CStaticLootSet::SItemLoot >::const_iterator itemLoot = lootSet->ItemLoot.begin(); itemLoot != lootSet->ItemLoot.end(); ++itemLoot )
	{
		if( i >= bagForm->SlotCount )
		{
			nlwarning("<CCreature::fillLootInventory> Too many items in loot set, max is %d",bagForm->SlotCount);
			break;
		}
		i++;

		CSheetId obtainedItem;
		if ( itemLoot->Item.find(".sitem") != string::npos )
			obtainedItem = CSheetId(itemLoot->Item);
		if( obtainedItem != CSheetId::Unknown )
		{
			haveLoot = true;
					
			const CStaticItem* form = CSheets::getForm( obtainedItem );
			if (!form)
			{
				nlwarning("<CCreature::fillLootInventory> Cannot find form of item %s", obtainedItem.toString().c_str());
				return;
			}

			CGameItemPtr sellingItem = NULL;
			// if item can be sold, get it in the sold items list
			if ( form->Family != ITEMFAMILY::RAW_MATERIAL 
				&& form->Family != ITEMFAMILY::HARVEST_TOOL
				&& form->Family != ITEMFAMILY::CRAFTING_TOOL
				&& form->Family != ITEMFAMILY::CRYSTALLIZED_SPELL
				&& form->Family != ITEMFAMILY::ITEM_SAP_RECHARGE
				&& form->Family != ITEMFAMILY::GENERIC_ITEM
			)
			{
				vector< CGameItemPtr >::const_iterator it;
				const vector< CGameItemPtr >::const_iterator itEnd = CStaticItems::getStaticItems().end();
				for( it = CStaticItems::getStaticItems().begin(); it != itEnd; ++it )
				{
					if( (*it)->getSheetId() == obtainedItem )
					{
						sellingItem = *it;
						break;
					}
				}
			}

			// create the item in inventory with stack if possible
			uint32 qtToLoot = itemLoot->Quantity;
			while (qtToLoot > 0)
			{
				uint16 quality;
				if( itemLoot->Level == 0 )
					quality = _Form->getXPLevel();
				else
					quality = itemLoot->Level;

				CGameItemPtr item = GameItemManager.createItem(obtainedItem, quality, true, true);
				item->setStackSize(min(item->getMaxStackSize(), qtToLoot));
				qtToLoot -= item->getStackSize();

				// inventory is not made to grow up automatically
				// TODO: should implement a better solution in terms of performance
				if (_LootInventory->getFreeSlotCount() == 0)
				{
					_LootInventory->setSlotCount(_LootInventory->getSlotCount()+1);
				}
				_LootInventory->insertItem(item);
			}
		}
		else
		{
			CGameItemPtr itemTmp = CNamedItems::getInstance().createNamedItem( itemLoot->Item, itemLoot->Quantity );
			if ( itemTmp != NULL )
			{
				// inventory is not made to grow up automatically
				// TODO: should implement a better solution in terms of performance
				if (_LootInventory->getFreeSlotCount() == 0)
				{
					_LootInventory->setSlotCount(_LootInventory->getSlotCount()+1);
				}
				_LootInventory->insertItem(itemTmp);
			}
		}
	}
}// fillLootInventory //


//---------------------------------------------------
// entity is dead :
//
//---------------------------------------------------
void CCreature::deathOccurs()
{
	H_AUTO(deathOccursCreature);

	TLogNoContext_Item noLog;

//	static const CSheetId idSheetStack("stack.sitem");

	if( _LootInventory == 0 )
	{
		// give xp gained on this creature
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->creatureDeath( _EntityRowId );
		disbandTrain();

		CSheetId idSheet("bag.sitem");
		const CStaticItem* bagForm = CSheets::getForm( idSheet );
		if (!bagForm)
		{
			nlwarning("<CCreature::deathOccurs> Cannot find form of item %s", idSheet.toString().c_str());
			return;
		}

		if( CR2EasterEgg::getInstance().isEasterEgg(_Id) )
		{
			requestDespawn(0);
			return;
		}

		bool haveLoot = false;

		_LootInventory = new CInventoryBase;

		if( _LootInventory != NULL )
		{
			if( _LootRight.size() > 0 && !IsRingShard ) // some PJ have loot right
			{
				if (!_CustomLootTableId.empty())
				{
					//CSheetId fake = CSheetId::Unknown;
					fillLootInventory(const_cast<CSheetId&>(CSheetId::Unknown), bagForm, haveLoot);
				}
				else
				{		
					for( uint i=0; i<_LootTables.size(); ++i )
					{
						if( _LootTables[i] != CSheetId::Unknown )
						{
							fillLootInventory( _LootTables[i], bagForm, haveLoot );
						}
					}
				}
				_LootSlotCount = _LootInventory->getUsedSlotCount();
			
				// clear loot right for gibe it to all after loot right duration elapsed 
				if( _LootRightDuration < CTickEventHandler::getGameCycle() )
				{
					_LootRight.clear();
				}
				
				if( !haveLoot && _Mps.size()==0 && _Id.getType() != RYZOMID::npc )
				{
					const double waitTime = 10.0; // in seconds
					const TGameCycle waitCycles = TGameCycle(waitTime / CTickEventHandler::getGameTimeStep());
					requestDespawn(waitCycles);
				}
				else
				{
					const double waitTime = 5 * 60.0; // in seconds
					const TGameCycle waitCycles = TGameCycle(waitTime / CTickEventHandler::getGameTimeStep());
					requestDespawn(waitCycles);
				}
			}
		}
		else
		{
			nlwarning("<CCreature::deathOccurs> Can't create loot inventory item sheet:%s for creature %s", idSheet.toString().c_str(), _Id.toString().c_str() );
			requestDespawn(0);
			return;				
		}
	}
}


//---------------------------------------------------
// set looter entity
//
//---------------------------------------------------
bool CCreature::setLooter( const CEntityId& entity, bool forceUpdate ) 
{
	if( _EntityLooter == CEntityId::Unknown || forceUpdate )
	{
		_EntityLooter = entity;
		return true;
	}
	else
	{
		return false;
	}
}


//---------------------------------------------------
// isShopStaticFamily
// true if the item does not exist in the StaticItems list, but must be created instead
//---------------------------------------------------
static bool isShopStaticItemFamily(ITEMFAMILY::EItemFamily fam)
{
	return  fam== ITEMFAMILY::RAW_MATERIAL ||
			fam == ITEMFAMILY::XP_CATALYSER || 
			fam == ITEMFAMILY::CRAFTING_TOOL || 
			fam == ITEMFAMILY::HARVEST_TOOL ||
			fam == ITEMFAMILY::GENERIC_ITEM;
}

//---------------------------------------------------
// setBotDescription Set all params describe bot relative to bot chat (what selling, what mission proposed etc...)
//
//---------------------------------------------------
void CCreature::setBotDescription( const CGenNpcDescMsgImp& description )
{
	_BotChatProgram = 0;
	_LastCycleUpdateSelectors = CTickEventHandler::getGameCycle();

	_MissionsProposed.clear();
	_WebPage.clear();
	_WebPageName.clear();
	_BotChatCategory.clear();
	_FactionAttackableBelow.clear();
	_FactionAttackableAbove.clear();
	_ExplicitActionTradeList.clear();
	_FilterExplicitActionTradeByPlayerRace= false;
	_ExplicitActionSPType= EGSPD::CSPType::Unknown;
	_FilterExplicitActionTradeByBotRace= true;

	_RmShopSelector.clear();
	_OriginShopSelector.clear();
	_QualityShopSelector.clear();
	_LevelShopSelector.clear();
	_ShopTypeSelector.clear();

	_GuildBuilding = NULL;
	_PlayerBuilding = NULL;
	_GuildRoleMasterType = EGSPD::CSPType::EndSPType;
	
	_TicketClanRestriction = PVP_CLAN::None;
	_TicketForNeutral = false;
	_TicketFameRestriction = CStaticFames::INVALID_FACTION_INDEX;
	_TicketFameRestrictionValue = 0;

	getMerchantPtr()->clearMerchantTradeList();
	_SpecialTradeList.removeAllContent();

	_BotChatProgram = 0;

	_FightAction = false;
	_MagicAction = false;
	_CraftAction = false;
	_HarvestAction = false;
	_CharacteristicsSeller = false;
	_GuildBuilding = NULL;
	_PlayerBuilding = NULL;
	_OutpostBuilding = NULL;
	_GuildRoleMasterType = EGSPD::CSPType::EndSPType;
	
	_GuildCreator = false;

	_MissionsProposed = description.getMissionIds();
	_WebPage = description.getWebPage();
	_WebPageName = description.getWebPageName();

	_BotChatOutpost = description.getOutpost();
	
	_Organization = description.getOrganization();

	bool cosmeticCategory = false;
	bool tradeCategory = false;
	bool tpCategory = false;
	bool factionCategory = false;

	// Set Explicit Action List
	_FilterExplicitActionTradeByPlayerRace= description.getFilterExplicitActionTradeByPlayerRace();
	_ExplicitActionSPType= description.getExplicitActionSPType();
	_FilterExplicitActionTradeByBotRace= description.getFilterExplicitActionTradeByBotRace();
	
	// Encyclopedia NPC setup
	for (uint32 nMis = 0; nMis < _MissionsProposed.size(); ++nMis)
	{
		CAIAliasTranslator *pAIAT = CAIAliasTranslator::getInstance();
		CMissionManager *pMM = CMissionManager::getInstance();

		CMissionTemplate *pMT;
		pMT = pMM->getTemplate( _MissionsProposed[nMis] );
		if (pMT != NULL)
		{
			if (pMT->EncycloAlbum != -1)
				pMT->EncycloNPC = description.getAlias();
		}
	}

	_WelcomePhrase = description.getWelcomePhrase();

	//_IsDead = false; // commented out because the AIS can resend the message whenever the creature is dead (ex: bodokin 'no drop' bug)

	///  set follow flag
	if ( description.getDontFollow() )
		_BotChatProgram |= ( 1 << BOTCHATTYPE::DontFollow );

	// set botchat description
	for( vector< uint32 >::const_iterator it = description.getShopCategories().begin(); it != description.getShopCategories().end(); ++it )
	{
//		nldebug(" for bot %s shop category %s", _Id.toString().c_str(), CShopTypeManager::getCategoryName()[ *it ].c_str() );
		if( (*it) > CShopTypeManager::getRmEcosystemStart() && (*it) < CShopTypeManager::getRmEcosystemEnd() )
		{
			if( ECOSYSTEM::stringToEcosystem( CShopTypeManager::getCategoryName()[ *it ] ) != ECOSYSTEM::unknown )
			{
				_RmShopSelector.push_back( *it );
			}
		}
		else if( (*it) > CShopTypeManager::getOriginStart() && (*it) < CShopTypeManager::getOriginEnd() )
		{
			if( ITEM_ORIGIN::stringToEnum( CShopTypeManager::getCategoryName()[ *it ] ) < ITEM_ORIGIN::NUM_ITEM_ORIGIN )
			{
				_OriginShopSelector.push_back( *it );
			}
		}
		else if( (*it) > CShopTypeManager::getQualityStart() && (*it) < CShopTypeManager::getQualityEnd() )
		{
			if( ( (*it) - CShopTypeManager::getQualityStart() - 1 ) < min((uint)NUM_QUALITY, (uint)(CShopTypeManager::getQualityEnd() - CShopTypeManager::getQualityStart() - 1)) )
			{
				_QualityShopSelector.push_back( *it );
			}
		}
		else if( (*it) > CShopTypeManager::getLevelStart() && (*it) < CShopTypeManager::getLevelEnd() )
		{
			if( ( (*it) - CShopTypeManager::getLevelStart() - 1 ) < min((uint)NUM_LEVEL, (uint)(CShopTypeManager::getLevelEnd() - CShopTypeManager::getLevelStart() - 1)) )
			{
				_LevelShopSelector.push_back( *it );
			}
		}
		else if( (*it) > CShopTypeManager::getShopTypeStart() && (*it) < CShopTypeManager::getShopTypeEnd() )
		{
			_ShopTypeSelector.push_back( *it );
		}

		else if( (*it) > CShopTypeManager::getItemStart() && (*it) < CShopTypeManager::getItemEnd() )
		{
			if ( CShopTypeManager::isCosmeticItem( *it ) )
				cosmeticCategory = true;
			else
				tradeCategory = true;
		}
		else if( (*it) > CShopTypeManager::getRmStart() && (*it) < CShopTypeManager::getRmEnd() )
		{
			tradeCategory = true;
		}
		else if( CShopTypeManager::getCategoryName()[*it] == string("fight_action") )
		{
			_FightAction = true;
		}
		else if( CShopTypeManager::getCategoryName()[*it] == string("magic_action") )
		{
			_MagicAction = true;
		}
		else if( CShopTypeManager::getCategoryName()[*it] == string("craft_action") )
		{
			_CraftAction = true;
		}
		else if( CShopTypeManager::getCategoryName()[*it] == string("harvest_action") )
		{
			_HarvestAction = true;
		}
		else if( CShopTypeManager::getCategoryName()[*it] == string("characteristics_seller") )
		{
			_CharacteristicsSeller = true;
		}
		else if( CShopTypeManager::getCategoryName()[*it] == string("guild_creator") )
		{
			_GuildCreator = true;
		}
		_BotChatCategory.push_back( *it ); //add shop type
	}
	
	// remove duplicate bot chat category
	sort(_BotChatCategory.begin(), _BotChatCategory.end());
	_BotChatCategory.erase(unique(_BotChatCategory.begin(), _BotChatCategory.end()), _BotChatCategory.end());

	_AIAlias = description.getAlias();

	// add a level selector for cosmetics
	if ( cosmeticCategory && _LevelShopSelector.empty() )
	{
		_LevelShopSelector.push_back( CShopTypeManager::getLevelStart() + 1 );
	}


	// set explicite trade list
	for (uint i=0; i<description.getExplicitSales().size(); ++i)
	{
		const RYMSG::TExplicitSale &sale = description.getExplicitSales()[i];

		switch (sale.getSaleType().getValue())
		{
		case RYMSG::TExplicitSaleType::est_item:
			{
				const CStaticItem * itemForm = CSheets::getForm( sale.getSheetId() );
				if( itemForm )
				{
					if (	itemForm->Family == ITEMFAMILY::TELEPORT
						|| (itemForm->Family == ITEMFAMILY::SERVICE && itemForm->SheetId.toString().substr(0,3) == "isa") // if it is an Item Service Altar
						)
					{
		//				nlinfo("Explicit Teleport %s selling by bot %s id %s", (*it).toString().c_str(), _SheetId.toString().c_str(), _Id.toString().c_str() );
						
						CSmartPtr< IItemTrade> itemTrade = new CTradeBase;
						itemTrade->setSheetId( sale.getSheetId() );
						RYMSG::TPriceInfo priceInfo;
						priceInfo.setCurrency(RYMSG::TTradeCurrency::tc_dappers);
						priceInfo.setAmount(itemForm->ItemPrice);
						itemTrade->setPriceInfo(priceInfo);
						itemTrade->setLevel( 1 );
						getMerchantPtr()->addExpliciteSellingItem( itemTrade );				
						tpCategory = true;
					}
					else if( itemForm->Family == ITEMFAMILY::AMMO || itemForm->Family == ITEMFAMILY::ARMOR || itemForm->Family == ITEMFAMILY::MELEE_WEAPON
						|| itemForm->Family == ITEMFAMILY::RANGE_WEAPON || itemForm->Family == ITEMFAMILY::SHIELD || 
						itemForm->Family == ITEMFAMILY::JEWELRY || isShopStaticItemFamily(itemForm->Family) )
					{
						CSmartPtr< IItemTrade> itemTrade = new CTradeBase;
						itemTrade->setSheetId( sale.getSheetId() );

						for( std::vector< CGameItemPtr >::const_iterator it = CStaticItems::getStaticItems().begin(); it != CStaticItems::getStaticItems().end(); ++it )
						{
							if( isShopStaticItemFamily(itemForm->Family) || (*it)->getSheetId() == itemTrade->getSheetId() )
							{
								itemTrade->setPriceInfo(sale.getPrice());
								// if price is <0, means not defined, thus compute std price
								if(sale.getPrice().getAmount()<0)
									itemTrade->getPriceInfo().setAmount(CShopTypeManager::computeBasePrice( itemTrade->getSheetId(), (uint16)sale.getQuality() ));
								
								// setup level and stat energy
								itemTrade->setLevel( sale.getQuality() );
								itemTrade->setQuality( 20 ); //temporary until AI & leveldesign enter quality data for explicite item
								if( isShopStaticItemFamily(itemForm->Family) )
								{
									itemTrade->setItemPtr( 0 );
								}
								else
								{
									itemTrade->setItemPtr( *it );
								}

//								// some faction point cost?
//								if (sale.getPrice().getCurrency() == RYMSG::TTradeCurrency::tc_faction_points)
//								{
//									itemTrade->setFactionType(sale.getPrice().getFaction());
//									itemTrade->setFactionPointPrice(sale.getPrice().getAmount());
//								}

								// add
								getMerchantPtr()->addExpliciteSellingItem( itemTrade );
								if (sale.getPrice().getCurrency() == RYMSG::TTradeCurrency::tc_faction_points)
									factionCategory = true;
								else
									tradeCategory = true;
								break;
								//nlinfo("CCreature::setBotDescription: explicite item for trade %s quality %d", description.ItemTypesForSale[ i ].toString().c_str(), description.ItemQualitiesForSale[ i ] );
							}
						}
					}
				}
			}
			break;
		case RYMSG::TExplicitSaleType::est_named_item:
			{
				CNamedItems &rNI = CNamedItems::getInstance();
				CGameItemPtr itemRef = rNI.getNamedItemRef(sale.getNamed());
				if (itemRef != NULL)
				{
					CSmartPtr<IItemTrade> itemTrade = new CTradeBase;
					itemTrade->setItemPtr(itemRef);
					itemTrade->setSheetId(itemRef->getSheetId());
					itemTrade->setPriceInfo(sale.getPrice());
					if (sale.getPrice().getCurrency() == RYMSG::TTradeCurrency::tc_dappers)
					{
//						itemTrade->setPrice(sale.getPrice().getAmount());
						tradeCategory = true;
					}
					else if (sale.getPrice().getCurrency() == RYMSG::TTradeCurrency::tc_faction_points)
					{
//						itemTrade->setFactionType(sale.getPrice().getFaction());
//						itemTrade->setFactionPointPrice(sale.getPrice().getAmount());
						factionCategory = true;
					}
					itemTrade->setLevel(itemRef->recommended());
					itemTrade->setQuality((uint32)(100.0f * itemRef->getStatEnergy()));
					// special for named item: allow duplication of them (because can have same level/sheet...)
					((CTradeBase*)(IItemTrade*)itemTrade)->setAllowSameItemInShopList(true);
					getMerchantPtr()->addExpliciteSellingItem(itemTrade);
				}
			}
			break;
		case RYMSG::TExplicitSaleType::est_phrase:
			{
				const CStaticRolemasterPhrase	*phrase= CSheets::getSRolemasterPhrase( sale.getSheetId() );
				BOMB_IF(phrase == NULL, "Invalid phrase sheet '"<<sale.getSheetId().toString(), continue);
				_ExplicitActionTradeList.push_back(sale.getSheetId());
			}
			break;
		default:
			STOP("Unsupported sale type "<<sale.getSaleType().toString());
		}

	}

	// Yoyo: decide that cannot have both faction and trade
	if(factionCategory)
		tradeCategory= false;

	if ( cosmeticCategory )
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::TradeCosmeticFlag;
	}
	if( tpCategory )
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::TradeTeleportFlag;
	}
	if( tradeCategory )
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::TradeItemFlag;
	}
	if( factionCategory )
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::TradeFactionFlag;
	}
	// Trade phrase if some trade action filter or if explicit action setuped
	if( _FightAction || _MagicAction || _CraftAction || _HarvestAction || !_ExplicitActionTradeList.empty())
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::TradePhraseFlag;
	}

	// update contextual property
	if ( _StaticContextualProperty.talkableTo() && _BotChatCategory.size() != 0 && _BotChatCategory[0] == 0 )
	{
		_ContextualProperty.directAccessForStructMembers().talkableTo( false );
	}

	_ContextualProperty.directAccessForStructMembers().attackable( description.getPlayerAttackable() );

	// In Ring shard  BotAttackable means that a bot is attackable by a bot not that he is vulnerable
	if (!IsRingShard)
	{
		_ContextualProperty.directAccessForStructMembers().invulnerable( !description.getBotAttackable());
	}
	else
	{
		_ContextualProperty.directAccessForStructMembers().invulnerable( !description.getBotAttackable() && !description.getPlayerAttackable() );
	}
	

	_ContextualProperty.setChanged();
	if ( _GuildCreator )
		_BotChatProgram |= 1 << BOTCHATTYPE::CreateGuildFlag;

	// bot has the mission interface only if it has a non auto mission
	if ( !_MissionsProposed.empty() )
	{
		for ( uint i = 0; i < _MissionsProposed.size(); i++ )
		{
			const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _MissionsProposed[i] );
			if ( templ && templ->AutoText.empty() && !templ->Tags.NotProposed)
			{
				_BotChatProgram |= 1 << BOTCHATTYPE::ChooseMissionFlag;
				// no need to read more
				break;
			}
		}
	}
	
	// bot has the WebPage interface?
	if(!_WebPage.empty())
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::WebPageFlag;
	}

	// bot has the outpost interface?
	if(_BotChatOutpost != NLMISC::CSheetId::Unknown)
	{
		_BotChatProgram |= 1 << BOTCHATTYPE::OutpostFlag;
	}
	
	bool ItemRmShopProgram = false;
	
	// update bot chat program validation
	for( vector< uint32 >::const_iterator itB = _BotChatCategory.begin(); itB != _BotChatCategory.end(); ++itB )
	{
		if( CShopTypeManager::getCategoryNumberToShopBaseIndex().size() > *itB )
		{
			if( CShopTypeManager::getCategoryNumberToShopBaseIndex()[ *itB ] < INVALID_SHOP_INDEX )
			{
				ItemRmShopProgram = true;
			}
		}
	}

	// set npc items
	setItems( description.getRightHandItem(), description.getRightHandItemQuality(), description.getLeftHandItem(), description.getLeftHandItemQuality() );

	_ContextTexts.resize(  description.getContextOptions().size() );
	for (uint i = 0; i < _ContextTexts.size(); i++ )
	{
		_ContextTexts[i].first = description.getContextOptions()[i].getTitle();
		_ContextTexts[i].second = description.getContextOptions()[i].getDetail();
	}

	// let's parse optional properties
	for ( uint i = 0; i < description.getOptionalProperties().size(); i++ )
	{
		std::vector< std::string > result;

		NLMISC::splitString( description.getOptionalProperties()[i],":", result ); 
		if ( result.empty() )
			continue;

		CMissionParser::removeBlanks( result[0] );
		
		if ( result[0] == "guild_caretaker" )
		{
			if ( result.size() != 2 )
			{
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
				continue;
			}
			if ( _GuildBuilding != NULL )
			{
				nlwarning( "parseBotOption -> in bot %u : building '%s' : more than one guild building!",_AIAlias, result[1].c_str() );
				continue;
			}
			CMissionParser::removeBlanks( result[1] );
			_GuildBuilding = CBuildingManager::getInstance()->parseGuildCaretaker( result[1] );
			if ( _GuildBuilding == NULL )
			{
				nlwarning( "parseBotOption -> in bot %u : caretaker building '%s' error in parsing",_AIAlias, result[1].c_str() );
				continue;
			}
			_BotChatProgram |= ( 1 << BOTCHATTYPE::TradeBuildingOptions );
		}
		else if ( result[0] == "player_caretaker" )
		{
			if ( result.size() != 2 )
			{
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
				continue;
			}
			if ( _PlayerBuilding != NULL )
			{
				nlwarning( "parseBotOption -> in bot %u : building '%s' : more than one player building!",_AIAlias, result[1].c_str() );
				continue;
			}
			CMissionParser::removeBlanks( result[1] );
			_PlayerBuilding = CBuildingManager::getInstance()->parsePlayerCaretaker( result[1] );
			if ( _PlayerBuilding == NULL )
			{
				nlwarning( "parseBotOption -> in bot %u : caretaker building '%s' error in parsing",_AIAlias, result[1].c_str() );
				continue;
			}
			_BotChatProgram |= ( 1 << BOTCHATTYPE::TradeBuildingOptions );
		}
		else if ( result[0] == "guild_rolemaster" )
		{
			if ( result.size() != 2 )
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
			else
			{
				CMissionParser::removeBlanks( result[1] );
				_GuildRoleMasterType = EGSPD::CSPType::fromString( result[1] );
				if ( _GuildRoleMasterType== EGSPD::CSPType::EndSPType )
					nlwarning( "bot %u : invalid dp type %s", _AIAlias, result[1].c_str() );
				_BotChatProgram |= (1 << BOTCHATTYPE::TradePhraseFlag);
				_BotChatProgram |= ( 1 << BOTCHATTYPE::GuildRoleMaster );
			}
		}
		else if ( result[0] == "buy" )
		{
			if ( result.size() != 3 )
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
			else
			{
				CMissionParser::removeBlanks( result[1] );
				CSheetId sheet( result[1] + ".sitem" );
				if ( sheet == CSheetId::Unknown )
					nlwarning("parseBotOption -> invalid sheet '%s' for bot %u", result[1].c_str(), _AIAlias );
				else
				{
					float factor = (float) atof( result[2].c_str() );
					_SpecialTradeList.addBoughtItem( sheet,factor );
				}
			}
		}
		else if ( result[0] == "PriceFactor" )
		{
			if ( result.size() != 2 )
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
			else
			{
				CMissionParser::removeBlanks( result[1] );
				float factor = (float) atof( result[1].c_str() );
				getMerchantPtr()->setPriceFactor( factor );
			}
		}
		else if ( result[0] == "FactionAttackableAbove" )
		{
			if (result.size() != 3)
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
			else
			{
				const string &factionName = result[1];
				sint32 fameLevel;
				NLMISC::fromString(result[2], fameLevel);
				// get faction index
				const uint32 index = CStaticFames::getInstance().getFactionIndex(factionName);
				if (index == CStaticFames::INVALID_FACTION_INDEX)
					nlwarning("Invalid faction %s",factionName.c_str());
				else
					_FactionAttackableAbove.push_back( make_pair(index, fameLevel) );
			}
		}
		else if ( result[0] == "FactionAttackableBelow" )
		{
			if (result.size() != 3)
				nlwarning("parseBotOption -> invalid number of params in command '%s' for bot %u", result[0].c_str(), _AIAlias );
			else
			{
				const string &factionName = result[1];
				sint32 fameLevel;
				NLMISC::fromString(result[2], fameLevel);
				// get faction index
				const uint32 index = CStaticFames::getInstance().getFactionIndex(factionName);
				if (index == CStaticFames::INVALID_FACTION_INDEX)
					nlwarning("Invalid faction %s",factionName.c_str());
				else
					_FactionAttackableBelow.push_back( make_pair(index, fameLevel) );
			}
		}
		else if ( NLMISC::strlwr(result[0]) == "spire" )
		{
			_BotChatProgram |= (1 << BOTCHATTYPE::Totem);
		}
		else if ( NLMISC::strlwr(result[0]) == "altar" )
		{
//			nlinfo("OPString: %s", optionalPropertiesString.c_str());
			uint32 nbAltarParams = (uint32)result.size();
			for( uint32 i = 1; i < nbAltarParams; ++i )
			{
//				nlinfo("OPSResult: %s", result[i].c_str());
				if( NLMISC::strlwr(result[i]) == "kami" && _TicketClanRestriction != PVP_CLAN::Kami )
				{
					_TicketClanRestriction = PVP_CLAN::Kami;
				}
				else if( NLMISC::strlwr(result[i]) == "karavan" && _TicketClanRestriction != PVP_CLAN::Karavan )
				{
					_TicketClanRestriction = PVP_CLAN::Karavan;
				}
				else if( NLMISC::strlwr(result[i]) == "neutral" )
				{
					_TicketForNeutral = true;
				}
				else if( CStaticFames::getInstance().getFactionIndex(NLMISC::strlwr(result[i])) != CStaticFames::INVALID_FACTION_INDEX )
				{
					string res = NLMISC::strlwr(result[i]);
					_TicketFameRestriction = CStaticFames::getInstance().getFactionIndex(NLMISC::strlwr(result[i]));
				}
				else
				{
					uint32 fame;
					NLMISC::fromString(result[i], fame);
					if( ( fame * FameAbsoluteMax / 100 ) != 0 )
					{
						_TicketFameRestrictionValue = fame * FameAbsoluteMax / 100;
						if( _TicketFameRestriction == CStaticFames::INVALID_FACTION_INDEX )
						{
							if( _TicketClanRestriction != PVP_CLAN::None && _TicketFameRestriction == CStaticFames::INVALID_FACTION_INDEX )
							{
								_TicketFameRestriction = PVP_CLAN::getFactionIndex(_TicketClanRestriction);
								_TicketClanRestriction = PVP_CLAN::None;
							}
						}
					}
					else
						nlwarning("parseBotOption -> invalid parameter '%s' for 'altar' command in bot %u", result[i].c_str(), _AIAlias );
				}
			}
		}
		else
			nlwarning("parseBotOption -> invalid command '%s' in bot %u", result[0].c_str(), _AIAlias );
	}
	// if the bot has a special trade list, it can trade
	if ( !_SpecialTradeList.empty() )
		_BotChatProgram |= 1 << BOTCHATTYPE::TradeItemFlag;

	_MaxHitRangeForPC = description.getMaxHitRangeForPC();

//	_MissionIconFlags.IsMissionStepIconDisplayable = description.getIsMissionStepIconDisplayable();
//	_MissionIconFlags.IsMissionGiverIconDisplayable = description.getIsMissionGiverIconDisplayable();

}

//-----------------------------------------------------------------------------
CSmartPtr<CMerchant>& CCreature::getMerchantPtr()
{
	if( _Merchant == 0 )
	{
		_Merchant = new CMerchant( *this );
		nlassert( _Merchant != 0 );
	}
	return _Merchant;
}


//---------------------------------------------------
// Update merchant trade list
//---------------------------------------------------
void CCreature::updateMerchantTradeList()
{
//	if( _LastCycleUpdateSelectors != _LastCycleUpdateTradeList || ( CTickEventHandler::getGameCycle() - _LastCycleUpdateTradeList > 100 ) )
	{
		// trade list is not up to date
		_LastCycleUpdateTradeList = _LastCycleUpdateSelectors = CTickEventHandler::getGameCycle();

		getMerchantPtr()->clearMerchantTradeList();

		// update merchant trade list
//		nlinfo("<CCreature::updateTradeList> Shop category for creature %s:", _Id.toString().c_str() );
		for( std::vector< uint32 >::const_iterator it = _BotChatCategory.begin(); it != _BotChatCategory.end(); ++it )
		{
			if( *it < CShopTypeManager::getCategoryName().size() )
			{				
				egs_shinfo("             shop %d - %s:", *it, CShopTypeManager::getCategoryName()[ *it ].c_str() );				
				CShopTypeManager::addShopBase( (*it), *getMerchantPtr(), _RmShopSelector, _OriginShopSelector, _QualityShopSelector, _LevelShopSelector, _ShopTypeSelector );
			}
		}
	}
}


//---------------------------------------------------
// get a reference on  shop unit of bot
//---------------------------------------------------
const std::vector< const IShopUnit * >& CCreature::getMerchantTradeList() 
{ 
	return getMerchantPtr()->getMerchantTradeList(); 
}


//---------------------------------------------------
// setItemsInSheath
//---------------------------------------------------
void CCreature::setItemsInSheath( uint8 sheath, const NLMISC::CSheetId &rightHandItem, uint8 qualityR, const NLMISC::CSheetId &leftHandItem, uint8 qualityL, const NLMISC::CSheetId &ammoItem, uint8 qualityA)
{
	if ( sheath >= NB_SHEATH )
	{
		nlwarning("<CCreature::setItemsInSheath> entity %s, Try to set sheath %d, while there is %d sheath ", _Id.toString().c_str(), sheath , NB_SHEATH);
		return;
	}

	_Items.Sheath[ sheath ].HandR.IdSheet	= rightHandItem;
	_Items.Sheath[ sheath ].HandL.IdSheet	= leftHandItem;
	_Items.Sheath[ sheath ].Ammo.IdSheet	= ammoItem;

	_Items.Sheath[ sheath ].HandR.Quality	= qualityR;
	_Items.Sheath[ sheath ].HandL.Quality	= qualityL;
	_Items.Sheath[ sheath ].Ammo.Quality	= qualityA;
} // setItemsInSheath //


//---------------------------------------------------
// disbandTrain
//---------------------------------------------------
void CCreature::disbandTrain()
{ 	
	// if beast was in a train, disband it
	if ( TheDataset.isAccessible(_CharacterLeaderIndex) )
	{
		CCharacter *player = PlayerManager.getChar( _CharacterLeaderIndex );
		if( player != NULL )
		{
			player->clearBeastTrain();
		}
		else
		{
			nlwarning("<CCreature::disbandTrain> Creature %s, unable to find leader CCharacter object related to the datasetRow %d",_Id.toString().c_str(), (uint32)_CharacterLeaderIndex.getIndex() );
		}

		_CharacterLeaderIndex = TDataSetRow();
	}
} // disbandTrain //

//---------------------------------------------------
// applyDamageOnArmor
//---------------------------------------------------
inline sint32 CCreature::applyDamageOnArmor( DMGTYPE::EDamageType dmgType, sint32 damage, SLOT_EQUIPMENT::TSlotEquipment forcedSlot)
{
	///\todo localisation for NPCS
	if ( dmgType <0 || uint(dmgType) >= /*form->Protections.size()*/DMGTYPE::NBTYPES )
	{
		nlwarning( "<CCreature getProtection> invalid damage type %d in entity %s", (sint)dmgType, _Id.toString().c_str() );
		return damage;
	}
	else
	{
		float armorReducFactor = 1.0f;
		const CSEffect* effect = lookForActiveEffect( EFFECT_FAMILIES::getDebuffResistEffect( dmgType) );
		if ( effect )
		{
			armorReducFactor = float(effect->getParamValue()) /100.0f;
		}
		
		uint16 max = _Form->getProtections()[dmgType].Max;
		float factor = _Form->getProtections()[dmgType].Factor;

		sint32 dmgAbsorbed = sint32(damage * factor);
		if ( dmgAbsorbed > max )
		{
			dmgAbsorbed = max;
		}
		dmgAbsorbed = sint32( dmgAbsorbed * armorReducFactor );
		damage -= dmgAbsorbed;
		if ( damage < 0 )
			return 0;
		return damage;
	}
}// applyDamageOnArmor


/*
 * Return the armor factor for a explosion (e.g. forage source explosion)
 */
float CCreature::getActualDamageFromExplosionWithArmor( float dmg ) const
{
	return dmg;
	/*if ( ! _Form )
		return dmg;
	..._CreatureForm->Protections[i]
	TODO
	*/
}


//---------------------------------------------------
// getNpcItem
//---------------------------------------------------
CGameItemPtr CCreature::getNpcItem( const NLMISC::CSheetId &sheet, uint16 quality)
{
	if (sheet == CSheetId::Unknown)
		return CGameItemPtr(NULL);
	
	// if item is craftable search in craftables items
	if( sheet.toString().substr(0,2) == string("ic") )
	{
		const std::vector< CGameItemPtr > & item = CStaticItems::getStaticItems();
		
		const uint nbItems = (uint)item.size();
		for (uint i = 0 ; i < nbItems ; ++i)
		{
			if ( item[i] != NULL && item[i]->getSheetId() == sheet && item[i]->quality() == quality )
			{
				return item[i];
			}
		}		
	}
	// else search in non craftable/non sellable ones
	else
	{
		const std::vector< CGameItemPtr > & items = CGameItemManager::getNpcSpecificItems();
		
		const uint nbItems = (uint)items.size();
		for (uint i = 0 ; i < nbItems ; ++i)
		{
			if ( items[i] != NULL && items[i]->getSheetId() == sheet )
			{
				return items[i];
			}
		}
	}
	
	nlwarning("Failed to create NPC item %s with quality %d for entity %s (%s)", sheet.toString().c_str(), quality, _Id.toString().c_str(), getType().toString().c_str() );

	return CGameItemPtr(NULL);
} // getNpcItem //



//-----------------------------------------------
// CCreature::kill
//-----------------------------------------------
void CCreature::kill(TDataSetRow killerRowId)
{
	if (_IsDead)
	{
//#ifdef NL_DEBUG
		if(!_DeathReportHasBeenPushed)
		{
			nlwarning("ZOMBIE : Dead Creature but report haven't been sent to AIS !!!! (_DeathReportHasBeenPushed = false)");
			BotDeathReport.Bots.push_back(_EntityRowId);
			TDataSetRow emptyRow;
			BotDeathReport.Killers.push_back(emptyRow);
			BotDeathReport.Zombies.push_back(true);
			_DeathReportHasBeenPushed = true;
		}
//#endif
		return;
	}

	// force rider to unmount
	if( TheDataset.isAccessible(_RiderEntity()))
	{
		CCharacter *e = PlayerManager.getChar( getRiderEntity() );
		if ( e )
		{
			e->unmount();
		}
	}
		
	// before marking the entity as dead, execute it's death action if any (MUST be an instant action)
	if (_Form && _Form->getActionOnDeath() != CSheetId::Unknown)
	{
		CPhraseManager::getInstance().executeAiAction(_EntityRowId, _EntityRowId, _Form->getActionOnDeath() );
	}
		
	_IsDead = true;
	_DeathDate = CTickEventHandler::getGameCycle();
	
	// if creature not killed by PC, no xp gain (same has creature despawn before death) unless creature killed itself
	CEntityBase * killer = CEntityBaseManager::getEntityBasePtr( killerRowId );
	if( killer )
	{
		if( killer != this && killer->getId().getType() != RYZOMID::player )
		{
			if (_SheetName.size() != 15 || (_SheetName.size() == 15 && _SheetName[5] != '5' && _SheetName[5] != '6' && _SheetName[5] != '7')) 
				PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->removeXpCreature(_EntityRowId);
			else
				PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->removeCreature(_EntityRowId);
		}
		else if( killer != this )
		{
			if( isSpire() )
				PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->spireDestroyed(this, (CCharacter*)killer);
		}
	}

	// creature is dead
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->removeCreature(this);
	
	removeAllSpells();
	
	_PhysScores._PhysicalScores[SCORES::hit_points].Current = 0;
	setBars(0);	
	
	BotDeathReport.Bots.push_back(_EntityRowId);
	BotDeathReport.Killers.push_back(killerRowId);
	BotDeathReport.Zombies.push_back(false);

//#ifdef NL_DEBUG
	// Looking for a 'Zombie' bug (creature dead on EGS, but not on AIS)
	_DeathReportHasBeenPushed = true;
//#endif
	
	_ContextualProperty.directAccessForStructMembers().talkableTo( false );
	_ContextualProperty.setChanged();
	
	CPhraseManager::getInstance().removeEntity(_EntityRowId, false);

	// Output Stats
	string continentName = string("None");
	string regionName = string("None");
	string placeName = string("None");
	
	const CPlace * stable;
	vector<const CPlace *> places;
	const CRegion * region ;
	const CContinent * continent;
	float gooDistance;
	CZoneManager::getInstance().getPlace( this, gooDistance, &stable,places, &region, &continent);
	if( continent ) { continentName = continent->getName(); }
	if( region ) { regionName = region->getName(); }
	if( !places.empty() ) { placeName = places[0]->getName(); }
	
	CEntityId KillerId;
	CSheetId KillerSheet;
	CEntityBase * e = CEntityBaseManager::getEntityBasePtr(killerRowId);
	if( e )
	{
		KillerId = e->getId();
		KillerSheet = e->getType();
	}

	// clear aggro list
	for ( set<TDataSetRow>::iterator it = _Agressiveness.begin(); it != _Agressiveness.end(); ++it )
	{
		CCharacter * pChar = PlayerManager.getChar( *it );
		if ( pChar )
			pChar->decAggroCount();
	}
	_Agressiveness.clear();

	if (_Id.getType() == RYZOMID::npc && _AIGroupAlias != CAIAliasTranslator::Invalid)
	{
		CreatureManager.removeNpcFromGroup(_AIGroupAlias, _AIAlias);
	}

	if ( CPVPFactionRewardManager::getInstance().isATotem( this ) && region )
	{
		CPVPFactionRewardManager::getInstance().destroyTotem( region->getId(), killerRowId );
	}

	if( CR2EasterEgg::getInstance().isEasterEgg( _Id ) )
	{	
		CR2EasterEgg::getInstance().characterLootEasterEgg( KillerId, _Id );
	}

	//Bsi.append( StatPath, NLMISC::toString("[PNJM] %s %s %s %s %s %s %s", _Id.toString().c_str(), _SheetId.toString().c_str(), continentName.c_str(), regionName.c_str(), placeName.c_str(), KillerId.toString().c_str(), KillerSheet.toString().c_str()) );
	//EgsStat.displayNL("[PNJM] %s %s %s %s %s %s %s", _Id.toString().c_str(), _SheetId.toString().c_str(), continentName.c_str(), regionName.c_str(), placeName.c_str(), KillerId.toString().c_str(), KillerSheet.toString().c_str());
} // kill //


//-----------------------------------------------
// CCreature::::tpWanted a tp wanted, check if tp is regular and send a server tp command
//-----------------------------------------------
void CCreature::tpWanted( sint32 x, sint32 y, sint32 z , bool useHeading , float heading , uint8 continent , sint32 cell )
{ 
	CPhraseManager::getInstance().removeEntity(TheDataset.getDataSetRow(_Id), false);

	// remove character of vision of other PC
	CMessage msgout("ENTITY_TELEPORTATION");
	msgout.serial( _Id );
	sendMessageViaMirror("GPMS", msgout);
	NLMISC::TGameCycle tick = CTickEventHandler::getGameCycle() + 1;
	
	CMessage msgout2("ENTITY_TELEPORTATION");
	msgout2.serial( _Id );
	msgout2.serial( x );
	msgout2.serial( y );
	msgout2.serial( z );
	msgout2.serial( heading );
	msgout2.serial( tick );
	
	sendMessageViaMirror("GPMS", msgout2);
}

//--------------------------------------------------------------
//					getResistScore() 
//--------------------------------------------------------------
uint32 CCreature::getMagicResistance(EFFECT_FAMILIES::TEffectFamily effectFamily)
{
	nlassert(_Form);

	switch(effectFamily)
	{
	case EFFECT_FAMILIES::Root:
		return _Form->getResists().Root + _ResistModifiers.Root;
	case EFFECT_FAMILIES::Mezz:
		return _Form->getResists().Sleep + _ResistModifiers.Sleep;
	case EFFECT_FAMILIES::Stun:
		return _Form->getResists().Stun + _ResistModifiers.Stun;
	case EFFECT_FAMILIES::Blind:
		return _Form->getResists().Blind + _ResistModifiers.Blind;
	case EFFECT_FAMILIES::SlowMove:
		return _Form->getResists().Snare + _ResistModifiers.Snare;
	case EFFECT_FAMILIES::SlowMelee:
	case EFFECT_FAMILIES::SlowRange:
	case EFFECT_FAMILIES::SlowMagic:
	case EFFECT_FAMILIES::SlowAttack:
		return _Form->getResists().Slow + _ResistModifiers.Slow;
	case EFFECT_FAMILIES::Fear:
		return _Form->getResists().Fear + _ResistModifiers.Fear;
	case EFFECT_FAMILIES::MadnessMelee:
	case EFFECT_FAMILIES::MadnessMagic:
	case EFFECT_FAMILIES::MadnessRange:
	case EFFECT_FAMILIES::Madness:
		return _Form->getResists().Madness + _ResistModifiers.Madness;
	default:
		return 0;
	};
} // getResistScore //

//--------------------------------------------------------------
//					getResistScore() 
//--------------------------------------------------------------
uint32 CCreature::getMagicResistance(DMGTYPE::EDamageType dmgType)
{
	nlassert(_Form);

	switch(dmgType) 
	{
	case DMGTYPE::ACID:
		return _Form->getResists().Acid + _ResistModifiers.Acid;
	case DMGTYPE::COLD:
		return _Form->getResists().Cold + _ResistModifiers.Cold;
	case DMGTYPE::ELECTRICITY:
		return _Form->getResists().Electricity + _ResistModifiers.Electricity;
	case DMGTYPE::FIRE:
		return _Form->getResists().Fire + _ResistModifiers.Fire;
	case DMGTYPE::POISON:
		return _Form->getResists().Poison + _ResistModifiers.Poison;
	case DMGTYPE::ROT:
		return _Form->getResists().Rot + _ResistModifiers.Rot;
	case DMGTYPE::SHOCK:
		return _Form->getResists().Shockwave + _ResistModifiers.Shockwave;
	default:
		return 0;
	};
} // getResistScore //


//--------------------------------------------------------------
//	incResistModifier
//--------------------------------------------------------------
void CCreature::incResistModifier(EFFECT_FAMILIES::TEffectFamily effectFamily, float factor)
{
	// only inc aggro if a player is in the aggro list
	if (!_NbOfPlayersInAggroList)
		return;

	switch(effectFamily)
	{
	case EFFECT_FAMILIES::Root:
		_ResistModifiers.Root += sint16(ResistIncreaseRoot*factor);
		break;
	case EFFECT_FAMILIES::Mezz:
		_ResistModifiers.Sleep += sint16(ResistIncreaseSleep*factor);
		break;
	case EFFECT_FAMILIES::Stun:
		_ResistModifiers.Stun += sint16(ResistIncreaseStun*factor);
		break;
	case EFFECT_FAMILIES::Blind:
		_ResistModifiers.Blind += sint16(ResistIncreaseBlind*factor);
		break;
	case EFFECT_FAMILIES::SlowMove:
		_ResistModifiers.Snare += sint16(ResistIncreaseSnare*factor);
		break;
	case EFFECT_FAMILIES::SlowMelee:
	case EFFECT_FAMILIES::SlowRange:
	case EFFECT_FAMILIES::SlowMagic:
	case EFFECT_FAMILIES::SlowAttack:
		_ResistModifiers.Slow += sint16(ResistIncreaseSlow*factor);
		break;
	case EFFECT_FAMILIES::Fear:
		_ResistModifiers.Fear += sint16(ResistIncreaseFear*factor);
		break;
	case EFFECT_FAMILIES::MadnessMelee:
	case EFFECT_FAMILIES::MadnessMagic:
	case EFFECT_FAMILIES::MadnessRange:
	case EFFECT_FAMILIES::Madness:
		_ResistModifiers.Madness += sint16(ResistIncreaseMadness*factor);
		break;
	default:
		;
		//nlwarning("<incResistModifier> unknown effect family %d (%s)", (sint16)effectFamily, EFFECT_FAMILIES::toString(effectFamily).c_str());
	};
} // incResistModifier //

//--------------------------------------------------------------
//	incResistModifier
//--------------------------------------------------------------
void CCreature::incResistModifier(DMGTYPE::EDamageType dmgType, float factor)
{
	// only inc aggro if a player is in the aggro list
	if (!_NbOfPlayersInAggroList)
		return;

	switch(dmgType) 
	{
	case DMGTYPE::ACID:
		_ResistModifiers.Acid += sint16(ResistIncreaseAcid*factor);
		break;
	case DMGTYPE::COLD:
		_ResistModifiers.Cold += sint16(ResistIncreaseCold*factor);
		break;
	case DMGTYPE::ELECTRICITY:
		_ResistModifiers.Electricity += sint16(ResistIncreaseElectricity*factor);
		break;
	case DMGTYPE::FIRE:
		_ResistModifiers.Fire += sint16(ResistIncreaseFire*factor);
		break;
	case DMGTYPE::POISON:
		_ResistModifiers.Poison += sint16(ResistIncreasePoison*factor);
		break;
	case DMGTYPE::ROT:
		_ResistModifiers.Rot += sint16(ResistIncreaseRot*factor);
		break;
	case DMGTYPE::SHOCK:
		_ResistModifiers.Shockwave += sint16(ResistIncreaseShockwave*factor);
		break;
	default:
		//nlwarning("<incResistModifier> unknown damage type %d", (sint16)dmgType);
		// do nothing
		;
	};
} // incResistModifier //

//--------------------------------------------------------------
//--------------------------------------------------------------
void CCreature::setOutpostBuilding(COutpostBuilding *pOB)
{
	_ContextualProperty.directAccessForStructMembers().selectable(true);
	_ContextualProperty.directAccessForStructMembers().talkableTo(pOB != NULL);
	_ContextualProperty.setChanged();

	if (pOB == NULL)
	{
		_OutpostBuilding = pOB;
		_BotChatProgram = 0;
	}
	else
	{
		_OutpostBuilding = pOB;
		_BotChatProgram = ( 1 << BOTCHATTYPE::TradeOutpostBuilding );
	}
}

//--------------------------------------------------------------
//	keep aggressiveness	of a creature against player character
//--------------------------------------------------------------
void CCreature::addAggressivenessAgainstPlayerCharacter( TDataSetRow PlayerRowId )
{
	if( _Agressiveness.find( PlayerRowId ) == _Agressiveness.end() )
	{
		_Agressiveness.insert( PlayerRowId );

		// if it's a player, inc _NbOfPlayersInAggroList
		CCharacter * pChar = PlayerManager.getChar(PlayerRowId);
		if ( pChar != NULL )
		{
			++_NbOfPlayersInAggroList;
			pChar->incAggroCount();
		}
	}
}

//--------------------------------------------------------------
//	remove aggressiveness of a creature against player character
//--------------------------------------------------------------
void CCreature::removeAggressivenessAgainstPlayerCharacter( TDataSetRow PlayerRowId )
{
	set< TDataSetRow >::iterator it = _Agressiveness.find( PlayerRowId );
	if( it != _Agressiveness.end() )
	{
		_Agressiveness.erase( it );
		// if it's a player, dec _NbOfPlayersInAggroList
		CCharacter * pChar = PlayerManager.getChar(PlayerRowId);
		if ( pChar != NULL )
		{
			--_NbOfPlayersInAggroList;
			pChar->decAggroCount();

			// is it was the last one, clear resist modifiers
			if (!_NbOfPlayersInAggroList)
				_ResistModifiers.clearModifiers();
		}
	}
}

//---------------------------------------------------
// Make all necessary update for entity at each ticks 
// (manage modifier, manage caracteristics, derivated scores...)
//
//---------------------------------------------------
uint32 CCreature::tickUpdate()
{	
	H_AUTO(CreatureUpdate);

	EntityMatrix.linkToMatrix(getState().X, getState().Y, _ListLink);

	if( _GodMode || _Invulnerable )
	{
		if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Current <= 0 )
			_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Base;
	}
	
	// Check if death of entity occurs
	if (_PhysScores._PhysicalScores[ SCORES::hit_points ].Current <= 0 &&	_Mode.getValue().Mode != MBEHAV::DEATH )
	{
		kill();
	}

	if (!isDead())
	{
		applyRegenAndClipCurrentValue();
		setBars();
	}

	// test again as effects can kill the entity (dots...)
	if (isDead())
	{
		deathOccurs();
	}

	// check if the creature must despawn
	if (_DespawnRequested && !_DespawnSentToAI)
	{
		const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();
		if (_DespawnDate <= time)
		{
			CreatureDespawnMsg.Entities.push_back(_EntityRowId);
			_DespawnSentToAI = true;
		}
	}

	if (currentHp()!=maxHp() && !isDead()) return 12;
	return 24;		
} // tickUpdate //

//---------------------------------------------------
// enable loot rights for team
//---------------------------------------------------
void CCreature::enableLootRights(uint16 teamId)
{
	_LootRight.clear();

	CTeam *team = TeamManager.getTeam(teamId);
	if (!team)
	{
		nlwarning("Failed to find team id %u", teamId);
		return;
	}

	const list<CEntityId> &ids = team->getTeamMembers();

	list<CEntityId>::const_iterator itEnd = ids.end();
	for (list<CEntityId>::const_iterator it = ids.begin() ; it != itEnd ; ++it)
	{
		TDataSetRow entityRowId = TheDataset.getDataSetRow(*it);
		_LootRight.push_back(entityRowId);
	}

	_LootRightDuration = CTickEventHandler::getGameCycle() + 900;
}

//---------------------------------------------------
// enable loot rights for team
//---------------------------------------------------
void CCreature::setMode( MBEHAV::EMode mode, bool, bool)
{
	if (_Mode.getValue().Mode == mode)
		return;
	
	CChangeCreatureModeMsg msgOut;
	msgOut.CreatureId = _EntityRowId;
	msgOut.NewMode = mode;
	
	CMirrorPropValueRO<uint32> instanceNumber(TheDataset, _EntityRowId, DSPropertyAI_INSTANCE );
	CWorldInstances::instance().msgToAIInstance(instanceNumber, msgOut);
	
/*		nlwarning("setting mode for a creature !!! Forbidden");
#ifdef NL_DEBUG
	nlstop("set mode for creature");
#endif
	*/
}

//---------------------------------------------------
// request despawn
//---------------------------------------------------
void CCreature::requestDespawn(NLMISC::TGameCycle waitCycles)
{
	WARN_IF(!isDead(), "<CCreature::requestDespawn> trying to despawn a living creature.");

	if (_DespawnSentToAI)
		return;
	
	// don't despawn mektoub
	if (getIsAPet())
		return;


	if (waitCycles == 0)
	{
		CreatureDespawnMsg.Entities.push_back(_EntityRowId);
		_DespawnSentToAI = true;
		return;
	}

	_DespawnRequested = true;
	_DespawnDate = CTickEventHandler::getGameCycle() + waitCycles;

	_TickUpdateTimer.setRemaining(waitCycles, new CCreatureTickUpdateTimerEvent(this), 1);
}

//---------------------------------------------------
// abort despawn
//---------------------------------------------------
bool CCreature::abortDespawn()
{
	// too late to abort
	if (_DespawnSentToAI)
		return false;

	_DespawnRequested = false;

	return true;
}

//-----------------------------------------------------------------------------
bool CCreature::sellPlayerItem()
{
	for( uint32 i = 0; i < _ShopTypeSelector.size(); ++i )
	{
		if( CShopTypeManager::getCategoryName()[ _ShopTypeSelector[ i ] ] == string("DYNAMIC_SHOP") )
		{
			return true;
		}
		else if( CShopTypeManager::getCategoryName()[ _ShopTypeSelector[ i ] ] == string("STATIC_DYNAMIC_SHOP") )
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------
// Display shop selectors
//------------------------------------------------------------------------------
void CCreature::displayShopSelectors( NLMISC::CLog& log )
{
	log.displayNL("Display shop selectors of NPC:");
	
	log.displayNL(" Shop type selector:");
	for( uint32 i = 0; i < _ShopTypeSelector.size(); ++i )
	{
		log.displayNL("  %s", CShopTypeManager::getCategoryName()[ _ShopTypeSelector[i] ].c_str() );
	}

	log.displayNL(" Bot Chat Category:");
	for( uint32 i = 0; i < _BotChatCategory.size(); ++i )
	{
		log.displayNL("  %s", CShopTypeManager::getCategoryName()[ _BotChatCategory[i] ].c_str() );
	}
	
	log.displayNL(" Raw Material Selector:");
	for( uint32 i = 0; i < _RmShopSelector.size(); ++i )
	{
		log.displayNL("  %s", CShopTypeManager::getCategoryName()[ _RmShopSelector[i] ].c_str() );
	}
	
	log.displayNL(" Origin Shop Selector:");
	for( uint32 i = 0; i < _OriginShopSelector.size(); ++i )
	{
		log.displayNL("  %s", CShopTypeManager::getCategoryName()[ _OriginShopSelector[i] ].c_str() );
	}
	
	log.displayNL(" Quality Shop Selector:");
	for( uint32 i = 0; i < _QualityShopSelector.size(); ++i )
	{
		log.displayNL("  %s", CShopTypeManager::getCategoryName()[ _QualityShopSelector[i] ].c_str() );
	}
	
	log.displayNL(" Level Shop Selector:");
	for( uint32 i = 0; i < _LevelShopSelector.size(); ++i )
	{
		log.displayNL("  %s", CShopTypeManager::getCategoryName()[ _LevelShopSelector[i] ].c_str() );
	}
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
bool CCreature::checkFactionAttackable(const CEntityId &playerId) const
{
	const uint size = (uint)_FactionAttackableAbove.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		// if player has one of specified fame above 0 he can attack this creature
		sint32 fame = CFameInterface::getInstance().getFameIndexed(playerId, _FactionAttackableAbove[i].first);
		if (fame > _FactionAttackableAbove[i].second)
			return true;
	}

	const uint size2 = (uint)_FactionAttackableBelow.size();
	for (uint i = 0 ; i < size2 ; ++i)
	{
		// if player has one of specified fame above 0 he can attack this creature
		sint32 fame = CFameInterface::getInstance().getFameIndexed(playerId, _FactionAttackableBelow[i].first);
		if (fame < _FactionAttackableBelow[i].second)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
// CCreatureSheet                                                           //
//////////////////////////////////////////////////////////////////////////////

CCreatureSheet::CCreatureSheet(IStaticCreaturesCPtr sheet)
: CStaticCreaturesProxy(sheet)
, _Faction(CStaticFames::INVALID_FACTION_INDEX)
, _FameByKillValid(false)
, _FameByKill(0)
{
	reset();
}


//----------------------------------------------------------------------------
NLMISC_COMMAND(checkNpcItem, "debug command for check getNpcItem() methode", "<bot eid> <item sheet Id>")
{
	if (args.size() < 2) return false;
	
	CEntityId eid(args[0]);
	CCreature* creature = CreatureManager.getCreature(eid);
	if (creature)
	{
		TLogNoContext_Item no_context;
		CSheetId sheet(args[1]);
		CGameItemPtr item = creature->getNpcItem(sheet, 1);
		if(item != 0)
		{
			nldebug("<checkNpcItem> getNpcItem(%s, 1) success", sheet.toString().c_str());
			//item.deleteItem();
		}
		else
		{
			nldebug("<checkNpcItem> getNpcItem(%s, 1) faild", sheet.toString().c_str());
		}
	}
	return true;
}
