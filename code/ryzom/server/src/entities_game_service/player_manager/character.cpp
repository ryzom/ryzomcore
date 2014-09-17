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

/////////////
// INCLUDE //
/////////////
//Misc
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/md5.h"
#include "nel/misc/sha1.h"
#include "nel/misc/vectord.h"
#include "nel/misc/vector_2d.h"
#include "nel/misc/sstring.h"

//Nel georges
#include "nel/georges/u_form.h"

#include "limits.h"


#include "server_share/used_continent.h"
#include "server_share/msg_brick_service.h"
#include "game_share/msg_client_server.h"
#include "game_share/slot_equipment.h"
//#include "game_share/chat_static_database.h"
#include "game_share/inventories.h"
#include "server_share/stats_status.h"
#include "game_share/gender.h"
#include "game_share/chat_group.h"
#include "server_share/npc_description_messages.h"
#include "game_share/roles.h"
#include "game_share/guild_grade.h"
#include "game_share/fame.h"
#include "game_share/send_chat.h"
#include "game_share/animals_orders.h"
#include "game_share/animal_status.h"
#include "game_share/animal_type.h"
#include "game_share/rolemaster_flags.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/visual_fx.h"
#include "game_share/interface_flags.h"
#include "game_share/chat_group.h"
#include "game_share/entity_types.h"
#include "game_share/mount_people.h"
#include "game_share/pvp_mode.h"
#include "game_share/utils.h"
#include "game_share/shard_names.h"
#include "game_share/character_sync_itf.h"
#include "game_share/mainland_summary.h"

#include "game_share/r2_share_itf.h"

#include "server_share/r2_vision.h"

#include "egs_sheets/egs_sheets.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "creature_manager/creature_manager.h"
#include "team_manager/team_manager.h"
#include "phrase_manager/phrase_manager.h"
#include "egs_progress_callback.h"
#include "cdb_check_sum.h"
#include "player_manager/cdb_branch.h"
#include "cdb_struct_banks.h"
#include "egs_mirror.h"
#include "mission_manager/mission_manager.h"
#include "shop_type/shop_type_manager.h"
#include "zone_manager.h"
#include "player_manager/action_distance_checker.h"
#include "phrase_manager/s_phrase.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "phrase_manager/available_phrases.h"
#include "phrase_manager/s_effect.h"
#include "phrase_manager/mod_magic_protection_effet.h"
#include "guild_manager/fame_manager.h"
#include "forage_progress.h"
#include "progression/progression_pve.h"
#include "progression/progression_pvp.h"
#include "player_manager/character_respawn_points.h"
#include "mission_manager/mission_log.h"
#include "entity_matrix.h"
#include "stables/stable.h"
#include "building_manager/building_manager.h"
#include "world_instances.h"
#include "pvp_manager/pvp_manager_2.h"
#include "pvp_manager/pvp_manager.h"
#include "pvp_manager/pvp.h"
#include "guild_manager/guild_manager.h"
#include "guild_manager/guild_member_module.h"
#include "building_manager/room_instance.h"
#include "mission_manager/mission_solo.h"
#include "mission_manager/mission_team.h"
#include "mission_manager/mission_guild.h"
#include "character_tick_update_timer_event.h"
#include "mission_manager/mission_team.h"
#include "mission_pd.h"
#include "shop_type/offline_character_command.h"
#include "player_manager/character_version_adapter.h"
#include "backward_compatibility/places_back_compat.h"
#include "dyn_chat_egs.h"
#include "player_manager/character_encyclopedia.h"
#include "player_manager/character_game_event.h"
#include "shop_type/character_shopping_list.h"
#include "player_manager/player_room.h"
#include "entities_game_service.h"
#include "shop_type/items_for_sale.h"
#include "player_manager/admin_properties.h"
#include "admin.h"
#include "death_penalties.h"
#include "player_manager/gear_latency.h"
#include "primitives_parser.h"
#include "phrase_manager/s_link_effect.h"
#include "item_service_manager.h"
#include "game_item_manager/player_inv_temp.h"
#include "game_item_manager/player_inv_pet.h"
#include "game_item_manager/player_inv_xchg.h"
#include "outpost_manager/outpost_manager.h"
#include "game_event_manager.h"
#include "stat_db.h"
#include "pvp_manager/pvp_faction_reward_manager/pvp_faction_reward_manager.h"
#include "entities_game_service/egs_variables.h"
#include "modules/shard_unifier_client.h"
#include "modules/r2_give_item.h"
#include "modules/char_name_mapper_client.h"
#include "inter_shard_exchange_validator.h"
#include "modules/client_command_forwarder.h"
#include "server_share/log_character_gen.h"
#include "server_share/log_item_gen.h"

///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLGEORGES;
using namespace EGSPD;

NL_INSTANCE_COUNTER_IMPL(CCharacter);
NLMISC::TInstanceCounterData CCharacter::CBotGift::CBotGiftInstanceCounter::_InstanceCounterData("CBotGift");
//NLMISC::TInstanceCounterData CCharacter::CCharacterDbReminder::CCharacterDbReminderInstanceCounter::_InstanceCounterData("CCharacterDbReminder");

/////////////
// DEFINES //
/////////////
#define MAX_PICKED_MISSIONS				8
#define MAX_STEPS_PER_MISSION			16

//#define _DEBUG_SKILL_PROGRESSION

////////////
// EXTERN //
////////////

// utility function to convert a guild id into a human readable string
extern std::string guildIdToString(uint32 guildId);

extern CPlayerManager				PlayerManager;
extern CGenericXmlMsgHeaderManager	GenericMsgManager;
extern sint32						BitPosAfterDatabaseMsgHeader;
extern CTeamManager					TeamManager;
//extern CChatStaticDatabase			ChatDatabase;
extern bool							TTSIsUp;
extern bool							UnlimitedDeathPact;
extern float						CarriedItemsDecayRatio;
extern float						CarriedItemsDecayRate;
extern uint8						TeamMembersStatusMaxValue;
extern CCharacterBotChatBeginEnd	CharacterBotChatBeginEnd;
//*** Removed by Sadge ***
//extern CCreatureAskInformationMsg	CreatureNpcInformation;
//*** ***
extern float						MaxHarvestDistance;
extern float						MaxMountDistance;
extern CRandom						RandomGenerator;
extern CVariable<string>			NeverAggroPriv;
extern SKILLS::ESkills				BarehandCombatSkill;

// Variable to alter the progression factor.
extern float						SkillProgressionFactor;

extern CVariable<string>	TeleportWithMektoubPriv;
extern CVariable<string>	NoActionAllowedPriv;
extern CVariable<string>	NoValueCheckingPriv;
extern CVariable<uint32>	OutpostJoinPvpTimer;
extern CVariable<uint32>	DefaultWeightHands;

extern vector<CMainlandSummary>		Mainlands;

CVariable<uint32>			SpawnedDeadMektoubDelay("egs","SpawnedDeadMektoubDelay", "nb tick before a dead mektoub despawn)", 2592000, 0, true );
CVariable<bool>				ForceQuarteringRight("egs","ForceQuarteringRight", "Allow anyone to quarter a dead creature", false, 0, true );
CVariable<bool>				AllowAnimalInventoryAccessFromAnyStable("egs", "AllowAnimalInventoryAccessFromAnyStable", "If true a player can access to his animal inventory (if the animal is inside a stable) from any stable over the world", true, 0, true );
CVariable<uint32>			FreeTrialSkillLimit("egs", "FreeTrialSkillLimit", "Level limit for characters belonging to free trial accounts", 125,0,true);
CVariable<uint32>			CraftFailureProbaMpLost("egs", "CraftFailureProbaMpLost", "Probability de destruction de chaque MP en cas d'echec du craft", 50,0,true);


// Number of login stats kept for a character
CVariable<uint32> NBLoginStats("egs","NBLoginStats", "Nb logins stats kept (logon time, logoff time", 50, 0, true );

// Max Bonus/malus/consumable effects displayed by client (database corresponding array must have the same size, and client must process the same size)
const uint32 MaxBonusMalusDisplayed = 12;

const string randomStrings = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#";
// We use this constant instead of the _StartingCharacteristicValues because _StartingCharacteristicValues is not working
//TODO
//const uint32 StartCharacteristicsValue = 10;


//CCharacter::CCharacterDbReminder*	CCharacter::_DataIndexReminder = NULL;

#include "cdb_struct_banks.h"
#include "cdb_group.h"

/// get the Right Trail value
uint8 getRightTrailValue(uint16 refValue)
{
	return uint8(15 * refValue / 250.0f);
}
/// get the Left Trail value
uint8 getLeftTrailValue(uint16 refValue)
{
	return uint8(7 * refValue / 250.0f);
}


ICharacter *ICharacter::getInterface(CCharacter *character, bool onlyOnline)
{
	if (character != NULL && onlyOnline && !character->getEnterFlag())
		return NULL;

	return static_cast<ICharacter*>(character);
}

ICharacter *ICharacter::getInterface(uint32 charId, bool onlyOnline)
{
	CCharacter *character = CPlayerManager::getInstance().getChar(charId >> 4, charId & 0xf);

	if (character != NULL && onlyOnline && !character->getEnterFlag() )
		return NULL;

	return character;
}

ICharacter *ICharacter::getInterface(NLMISC::CEntityId charEId, bool onlyOnline)
{
	uint32 charId = uint32(charEId.getShortId());
	CCharacter *character = CPlayerManager::getInstance().getChar(charId >> 4, charId & 0xf);

	if (character != NULL && onlyOnline && !character->getEnterFlag() )
		return NULL;

	return character;
}

ICharacter *ICharacter::getInterfaceFromUser(uint32 userId, bool onlyOnline)
{
	CCharacter *character = CPlayerManager::getInstance().getActiveChar(userId);

	if (character != NULL && onlyOnline && !character->getEnterFlag() )
		return NULL;

	return character;
}
CCharacter *ICharacter::getCharacter()
{
	return static_cast<CCharacter*>(this);
}

CModuleParent & ICharacter::getModuleParentWrap()
{
	return getCharacter()->getModuleParent();
}

CEntityState& ICharacter::getStateWrap()
{
	return getCharacter()->getState();
}

void		ICharacter::setStateWrap( const COfflineEntityState& es )
{
	getCharacter()->setState(es);
}


//-----------------------------------------------
// Constructor
//
//-----------------------------------------------
/*CCharacterFames::CCharacterFames()
{
	Kami= 199;
	Karavan= 199;
}
*/

//-----------------------------------------------
// CCharacterFames::addPropertiesToMirror
// Add fames to mirror
//-----------------------------------------------
/*void CCharacterFames::addPropertiesToMirror( const TDataSetRow& entityIndex )
{
	Kami.tempMirrorize( TheDataset, entityIndex, DSPropertyKAMI_FAME );
	Karavan.tempMirrorize( TheDataset, entityIndex, DSPropertyKARAVAN_FAME );
}
*/

//-----------------------------------------------
// CCharacter :
// Default constructor.
//-----------------------------------------------
CCharacter::CCharacter():	CEntityBase(false),
							_StartupInstance(INVALID_AI_INSTANCE),
							_InventoryUpdater(&_PropertyDatabase),
							_PVPInterface(NULL),
							_EncycloChar(NULL),
							_GameEvent(NULL),
							_RespawnPoints(NULL),
							_PlayerRoom(NULL),
							_ItemsInShopStore(NULL),
							_AdminProperties(NULL),
							_DeathPenalties(NULL),
							_GearLatency(NULL),
							_Missions(NULL),
							_Fames(NULL),
							RingRewardPoints(this),
							_PersistentEffects(this),
							_Invisibility(false),
							_AggroableSave(true),
							_GodModeSave(false)
{
	// todo : uncomment that when sadge item api is plugged
	_AggroCount = 0;
//	_Bulk = 0;
//	_CarriedWeight = 0;
	_GuildId = 0;
	_UseFactionSymbol = false;

	_SavedVersion = 0;
	_Enter = false;

	_OldPosX	= 0;
	_OldPosY	= 0;

	_LastPosXInDB = 0;
	_LastPosYInDB = 0;

	_NbStaticActiveEffects  = 0;
	_StaticActionInProgress = false;

	_NbUserChannels = 0;

	_LoadingFinish = false;

	_PVPFlagAlly = 0;
	_PVPFlagEnemy = 0;

	// init faber plans
//	_KnownFaberPlans.resize(64,(uint64)0); //64*64 bits

	// init of inventories
	initInventories();

	_LootContainer = 0;
	_EntityLoot = NLMISC::CEntityId::Unknown;

//	_NextDecayTickTime = CTickEventHandler::getGameCycle() + uint32(CarriedItemsDecayRate / CTickEventHandler::getGameTimeStep());
	_LastTickSufferGooDamage= CTickEventHandler::getGameCycle();
	_LastTickSaved = CTickEventHandler::getGameCycle();
	_LastTickCompassUpdated = CTickEventHandler::getGameCycle();

	// harvest related
	resetHarvestInfos();
	_HarvestOpened = false;
	_MpSourceSheetId = CSheetId();
	_HarvestDeposit = false;
	_DepositSearchSkill = SKILLS::unknown;

	// forage
	_ForageProgress = NULL;
	_ProspectionLocateDepositEffect = NULL;
	_ForageBonusExtractionTime = 0;

	// bot chat
	_CurrentBotChatListPage = 0;
	_BotGift = NULL;

	//no exchange proposition
	_ExchangeAsker = CEntityId::Unknown;
	_ExchangeId = 0;
	_ExchangeAccepted = false;
	_ExchangeMoney = (uint64)0;

	// pvp for player with privilege inactive
	_PriviledgePvp = false;
	// full pvp
	_FullPvp = false;
	// aggroable undefined
	_Aggroable = true;
	_AggroableOverride = -1;

	// init money to zero
	_Money = 0;

	for (uint i = 0 ; i < (PVP_CLAN::EndClans-PVP_CLAN::BeginClans+1); ++i)
		_FactionPoint[i] = 0;

	_PvpPoint = 0;
	_PVPFlagLastTimeChange = 0;
	_PVPFlagTimeSettedOn = 0;
	_PvPDatabaseCounter = 0;
	_PVPFlag = false;
	_PVPRecentActionTime = 0;

	_Organization = 0;
	_OrganizationStatus = 0;
	_OrganizationPoints = 0;

	// do not start berserk
	_IsBerserk = false;

	// Contextual properties init
	CProperties contprop;
	contprop.selectable( true );
	contprop.talkableTo( false );
	contprop.usable( false );
	contprop.liftable( false );
	contprop.lookableAt( false );
	contprop.givable( false );
	contprop.attackable( false );
	contprop.invitable( true );
	contprop.lootable( false );
	contprop.harvestable( false );
	contprop.canExchangeItem( true );
	contprop.mountable( false );
	_ContextualProperty= contprop;
	_StaticContextualProperty = contprop;

	// init vector size of owned Pets
	_PlayerPets.resize( MAX_INVENTORY_ANIMAL );
	NearPetTpAllowed = false;

//	ensureDbReminderReady();

	TestProgression = false;

	_ActionCounter = 0;
	_InterfaceCounter = 0;
	_CycleCounter = 0;
	_NextCounter = 0;

	_CurrentContinent = CONTINENT::UNKNOWN;
	_CurrentRegion = 0xFFFF;
	_RegionKilledInPvp = 0xFFFF;
	_CurrentStable = 0xFFFF;

	_MeleeCombatIsValid = false;

	_CurrentBotChatType = BOTCHATTYPE::UnknownFlag;

/*
	_HaveMweaTool = false;
	_HaveRweaTool = false;
	_HaveAmmoTool = false;
	_HaveArmorTool = false;
	_HaveJewelTool = false;
	_HaveForageTool = false;
*/
	_DateOfNextAllowedAction = 0;

	_OldHpBarSentToTeam = 0;
	_OldSapBarSentToTeam = 0;
	_OldStaBarSentToTeam = 0;

	_OldHpBarSentToPlayer = 0;
	_OldSapBarSentToPlayer = 0;
	_OldStaBarSentToPlayer = 0;
	_OldFocusBarSentToPlayer = 0;
	_BarSentToPlayerMsgNumber = 0;


	///init teamId
	_TeamId= CTEAM::InvalidTeamId;

	///init LeagueId
	_LeagueId = DYN_CHAT_INVALID_CHAN;

	// init combat flags
	_CombatEventFlagTicks.resize(32);
	for( uint i=0; i<32; ++i )
	{
		_CombatEventFlagTicks[i].StartTick = 0;
		_CombatEventFlagTicks[i].EndTick = 0;
		_CombatEventFlagTicks[i].OldStartTick = 0;
		_CombatEventFlagTicks[i].OldEndTick = 0;
	}

	// init power and aura flags
	_ForbidAuraUseStartDate = 0;
	_ForbidAuraUseEndDate = 0;
	_PowerFlagTicks.resize(32);
	for( uint i=0; i<32; ++i )
	{
		_PowerFlagTicks[i].StartTick = 0;
		_PowerFlagTicks[i].EndTick = 0;
		_PowerFlagTicks[i].OldStartTick = 0;
		_PowerFlagTicks[i].OldEndTick = 0;
	}

	// resize charac vector
	_StartingCharacteristicValues.resize(CHARACTERISTICS::NUM_CHARACTERISTICS);

	_NbAuras = 0;

	_WearEquipmentMalus = 0.0f;

	_DodgeModifier			= 0;
	_ParryModifier			= 0;
	_AdversaryDodgeModifier	= 0;
	_AdversaryParryModifier	= 0;

	_IsInAComa				= false;
	_IntangibleEndDate		= 0;
	uint64 NOT_TELEPORTING_FLAG= (((uint64)0x12345678)<<32)| (uint64)0x87654321;
	_WhoSeesMeBeforeTP		= 	NOT_TELEPORTING_FLAG;

	_PlayerIsInWater		= false;

	_BestSkill				= SKILLS::unknown;

	_ConsumedItemSlot		= -1;
	_ConsumedItemInventory	= INVENTORIES::UNDEFINED;

	_XpCatalyserSlot		= INVENTORIES::INVALID_INVENTORY_SLOT;
	_RingXpCatalyserSlot	= INVENTORIES::INVALID_INVENTORY_SLOT;

	// init _Sp
	for (uint i = 0 ; i < EGSPD::CSPType::EndSPType ; ++i)
	{
		_SpType[i] = 0.0f;
		_SpentSpType[i] = 0;
	}

	// init permanent modifiers on scores
	for (uint i = 0 ; i < SCORES::NUM_SCORES ; ++i)
		_ScorePermanentModifiers[i] = 0;

	// init owner character for CItemsForSale class member
	_ItemsInShopStore = new CItemsForSale;
	_ItemsInShopStore->setOwnerCharacter( this );

	_ShoppingList = 0;
	_RawMaterialItemPartFilter = RM_FABER_TYPE::Unknown;
	_MinQualityFilter = 0;
	_MaxQualityFilter = ~0u;
	_MinPriceFilter = 0;
	_MaxPriceFilter = ~0u;

	_LastAppliedWeightMalus = 0;
	_TpTicketSlot = INVENTORIES::INVALID_INVENTORY_SLOT;

	// setup timer for tickUpdate() calling
	_TickUpdateTimer.setRemaining( 1, new CCharacterTickUpdateTimerEvent( this ), CharacterTickUpdatePeriodGc );
	_DbUpdateTimer.setRemaining( 1, new CCharacterTickUpdateTimerEvent( this ),1 );
	_DeathPenaltyTimer.setRemaining( 1, new CDeathPenaltiesTimerEvent( this ),1 );
	_BarUpdateTimer.setRemaining( 1, new CCharacterBarUpdateTimerEvent( this ),1 );

	_BuildingExitZone = 0xffff;

	_RespawnMainLandInTown = false;

	_CurrentPVPZone = CAIAliasTranslator::Invalid;
	_CurrentOutpostZone = CAIAliasTranslator::Invalid;
	resetNextDeathPenaltyFactor();

	_CurrentDodgeLevel = 1;
	_BaseDodgeLevel = 1;
	_CurrentParryLevel = 1;
	_BaseParryLevel = 1;

	_BaseResistance = 1;

	_SkillUsedForDodge = SKILLS::SF;
	_CurrentParrySkill = BarehandCombatSkill;

	_EncycloChar = new CCharacterEncyclopedia(*this);
	_GameEvent = new CCharacterGameEvent(*this);
	_RespawnPoints = new CCharacterRespawnPoints(*this);
	_PlayerRoom = new CPlayerRoomInterface;
	_PVPInterface = new CPVPInterface(this);
	_AdminProperties = new CAdminProperties;
	_DeathPenalties = new CDeathPenalties;
	_GearLatency = new CGearLatency;

	/** \todo Handle PDS code a MUCH BETTER WAY */
	//_Missions = new EGSPD::CMissionContainerPD;
	//_Fames = new EGSPD::CFameContainerPD;
	_Missions = NULL;
	_Fames = NULL;

	_HairCuteDiscount = false;

	_DPLossDuration = 0.f;

	_EnterCriticalZoneProposalQueueId = 0;

	_NbNonNullClassificationTypesSkillMod = 0;
	for (uint i = 0 ; i < (uint)EGSPD::CClassificationType::EndClassificationType ; ++i)
		_ClassificationTypesSkillModifiers[i] = 0;

	// log stats init
	_FirstConnectedTime = 0;
	_LastConnectedTime = 0;
	_PlayedTime = 0;

	_HaveToUpdateItemsPrerequisit = false;

	_OutpostLeavingTime = 0;
	_OutpostIdBeforeUserValidation = 0;

	_SelectedOutpost = 0;

	_ChannelAdded = false;

	_DuelOpponent = NULL;

	_LastCivPointWriteDB = ~0;
	_LastCultPointWriteDB = ~0;

	_DeclaredCult = PVP_CLAN::Unknown;
	_DeclaredCiv = PVP_CLAN::Unknown;

	_HaveToUpdatePVPMode = false;
	_SessionId = TSessionId(0);
	_CurrentSessionId = _SessionId;
	_PvPSafeZoneActive = false;

	_PVPSafeLastTimeChange = CTickEventHandler::getGameCycle();
	_PVPSafeLastTime = false;
	_PVPInSafeZoneLastTime = false;

	// For client/server contact list communication
	_ContactIdPool= 0;

	_inRoomOfPlayer = CEntityId::Unknown;

	for(uint i = 0; i < BRICK_FAMILIES::NbFamilies; ++i )
		_BrickFamilyBitField[i] = 0;
	_InterfacesFlagsBitField = 0;
	_RingSeason = 0;

	_LastTickNpcControlUpdated = CTickEventHandler::getGameCycle();

	_LastWebCommandIndex = 0;

	_CustomMissionsParams.clear();

	_FriendVisibility = VisibleToAll;

	_LangChannel = "en";
	_NewTitle = "Refugee";

	initDatabase();
} // CCharacter  //


//-----------------------------------------------
// clear :
//
//-----------------------------------------------
void CCharacter::clear()
{
	_HairType=0;
	_HairColor=0;
	_HatColor=0;
	_JacketColor=0;
	_ArmsColor=0;
	_TrousersColor=0;
	_FeetColor=0;
	_HandsColor=0;
	_Money=0;
	_GuildId=0;
	_CreationPointsRepartition=0;
	_ForbidAuraUseStartDate=0;
	_ForbidAuraUseEndDate=0;
	_Title= CHARACTER_TITLE::Refugee;
	_NewTitle = "Refugee";

	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.HatModel,0);
	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.HatColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.CharacterHeight,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.TorsoWidth,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.ArmsWidth,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.LegsWidth,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.BreastSize,0);
	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.JacketColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.TrouserColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.HatColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.ArmColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyB,PropertySubData.HandsColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyB,PropertySubData.FeetColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget1,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget2,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget3,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget4,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget5,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget6,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget7,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.MorphTarget8,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.EyesColor,0);
	SET_STRUCT_MEMBER(_VisualPropertyC,PropertySubData.Tattoo,0);

	_BoughtPhrases.clear();
	_KnownBricks.clear();
	_FriendsList.clear();
	_IgnoreList.clear();
	_IsFriendOf.clear();
	_IsIgnoredBy.clear();

	_MemorizedPhrases.clear();
	_ForbidPowerDates.clear();
	_IneffectiveAuras.clear();
	_ConsumableOverdoseEndDates.clear();
	_ModifiersInDB.clear();
	_DeathPenalties->clear();
	_Missions->clear();
	_PlayerRoom->clear();
	CEntityBase::clear();
	getRespawnPoints().clearRespawnPoints();
	while(_Fames->getEntriesBegin()!=_Fames->getEntriesEnd())	_Fames->deleteFromEntries((*_Fames->getEntriesBegin()).first);

	_Pact.clear();
	_KnownPhrases.clear();
	_MissionHistories.clear();
	uint32 petCount= (uint32)_PlayerPets.size();	_PlayerPets.clear();	_PlayerPets.resize(petCount);

	for(uint32 i=0;i<EGSPD::CSPType::EndSPType;++i)
		_SpType[i]=0.0;

	sint32 startingCharacteristicValuesSize= (sint32)_StartingCharacteristicValues.size();
	_StartingCharacteristicValues.clear();
	_StartingCharacteristicValues.resize(startingCharacteristicValuesSize);

//	for(uint32 i=0;i<INVENTORIES::NUM_INVENTORY;++i)
//		if (_Inventory[i]!=NULL)
//			_Inventory[i].deleteItem();

	for(uint32 i=0;i<INVENTORIES::NUM_INVENTORY;++i)
		_Inventory[i]->clearInventory();

	_SDBPvPPath.clear();
}


/*
 * Init PD Struct
 */
void	CCharacter::initPDStructs()
{
	/**
	 *
	 * Warning!!!
	 * This is a dirty hack to allow CCharacter owning a CMissionContainerPD and a CFameContainer
	 *
	 */
	{
	H_AUTO(InitMissioPD);
	_Missions = EGSPD::CMissionContainerPD::create(_Id);
	}
	{
	H_AUTO(InitFamePD);
	_Fames = EGSPD::CFameContainerPD::create(_Id);
	}
}


//-----------------------------------------------
// updatePVPClanVP
//
//-----------------------------------------------
void CCharacter::updatePVPClanVP() const
{
	TYPE_PVP_CLAN propPvpClanTemp = 0;
	
	if (_LeagueId != DYN_CHAT_INVALID_CHAN)
		propPvpClanTemp = 1+(uint32)(_LeagueId.getShortId());
	else
		propPvpClanTemp = 0;

	CMirrorPropValue<TYPE_PVP_CLAN> propPvpClan( TheDataset, TheDataset.getDataSetRow(_Id), DSPropertyPVP_CLAN );
	if (propPvpClan.getValue() != propPvpClanTemp)
	{
		propPvpClan = propPvpClanTemp;
		propPvpClan.setChanged();
	}
}
/*
TYPE_PVP_CLAN CCharacter::getPVPFamesAllies()
{
	TYPE_PVP_CLAN propPvpClanTemp = 0;
	for (uint8 fameIdx = 0; fameIdx < 7; fameIdx++)
		if (CFameInterface::getInstance().getFameIndexed(_Id, fameIdx) >= PVPFameRequired*6000)
			propPvpClanTemp |= TYPE_PVP_CLAN(1) << TYPE_PVP_CLAN(fameIdx);
	if (getPvPRecentActionFlag())
		return propPvpClanTemp | _PVPFlagAlly;
	return propPvpClanTemp;
}

TYPE_PVP_CLAN CCharacter::getPVPFamesEnemies()
{
	TYPE_PVP_CLAN propPvpClanTemp = 0;
	for (uint8 fameIdx = 0; fameIdx < 7; fameIdx++)
		if (CFameInterface::getInstance().getFameIndexed(_Id, fameIdx) <= -PVPFameRequired*6000)
			propPvpClanTemp |= TYPE_PVP_CLAN(1) << TYPE_PVP_CLAN(fameIdx);
	if (getPvPRecentActionFlag())
		return propPvpClanTemp | _PVPFlagEnemy;
	return propPvpClanTemp;
}
*/

//-----------------------------------------------
// addPropertiesToMirror :
//
//-----------------------------------------------
void CCharacter::addPropertiesToMirror( const TDataSetRow& entityIndex, bool keepSheetId )
{
	nlassert( _Id != CEntityId::Unknown );

	// Add properties of CEntityBase
	CEntityBase::addPropertiesToMirror( entityIndex, keepSheetId );

	// Visual properties for player character
	_VisualPropertyA.tempMirrorize( TheDataset, entityIndex, DSPropertyVPA );
	_VisualPropertyB.tempMirrorize( TheDataset, entityIndex, DSPropertyVPB );
	_VisualPropertyC.tempMirrorize( TheDataset, entityIndex, DSPropertyVPC );

	_ContextualProperty.tempMirrorize( TheDataset, entityIndex, DSPropertyCONTEXTUAL );

	// Size of database impulsion window (TEMP: only 1 FS)
	_AvailImpulseBitsize.init( TheDataset, entityIndex, DSFirstPropertyAvailableImpulseBitSize );

	// current team id
	_TeamId.tempMirrorize( TheDataset, entityIndex, DSPropertyTEAM_ID );

//	_Fames->addPropertiesToMirror( entityIndex );


	updatePVPClanVP();

	updateGuildFlag();

	_OutpostInfos.init( TheDataset, entityIndex, DSPropertyOUTPOST_INFOS );
	_OutpostInfos = 0;

	if( !_OutpostAlias.isReadable() )
	{
		_OutpostAlias.init( TheDataset, entityIndex, DSPropertyIN_OUTPOST_ZONE_ALIAS );
		_OutpostAlias =  0;
	}
	else
	{
		_OutpostAlias.tempMirrorize( TheDataset, entityIndex, DSPropertyIN_OUTPOST_ZONE_ALIAS );
	}
	if( !_OutpostSide.isReadable() )
	{
		_OutpostSide.init( TheDataset, entityIndex, DSPropertyIN_OUTPOST_ZONE_SIDE );
		_OutpostSide = OUTPOSTENUMS::UnknownPVPSide;
	}
	else
	{
		_OutpostSide.tempMirrorize( TheDataset, entityIndex, DSPropertyIN_OUTPOST_ZONE_SIDE );
	}

	if( _OutpostAlias != 0 )
	{
		if( _OutpostLeavingTime < CTickEventHandler::getGameCycle() )
		{
			_OutpostAlias = 0;
			_OutpostSide = OUTPOSTENUMS::UnknownPVPSide;
		}
		else
		{
			setOutpostAlias(_OutpostAlias.getValue());
		}
	}

	CPVPManager2::getInstance()->setPVPModeInMirror(this);

} // addPropertiesToMirror //


//---------------------------------------------------
// setValue :
//
//---------------------------------------------------
bool CCharacter::setValue( string var, string value )
{
//	if( _Behaviour.getValue().Behaviour < MBEHAV::RESURECTED || _Behaviour.getValue().Behaviour > MBEHAV::PERMANENT_DEATH )
	//if (!isDead())
	{
		try
		{
			sint32 &temp = lookupStat(var);
			NLMISC::fromString(value, temp);
		}
		catch(const CCharacter::EInvalidStat &e)
		{
			nlwarning("<CCharacter::setValue> Exception : %s",e.what( var ) );
			return false;
		}
	}
	return true;
} // setValue //

//---------------------------------------------------
// modifyValue :
//
//---------------------------------------------------
bool CCharacter::modifyValue( string var, string value )
{
//	if( _Behaviour.getValue().Behaviour < MBEHAV::RESURECTED || _Behaviour.getValue().Behaviour > MBEHAV::PERMANENT_DEATH )
	if (!isDead())
	{
		try
		{
			sint32 &temp = lookupStat(var);
			sint32 valueInt;
			NLMISC::fromString(value, valueInt);
			temp = temp + valueInt;
		}
		catch(const CCharacter::EInvalidStat &e)
		{
			nlwarning("<CCharacter::modifyValue> Exception : %s",e.what( var ) );
			return false;
		}
	}
	return true;
} // setValue //

//---------------------------------------------------
// getValue :
//
//---------------------------------------------------
bool CCharacter::getValue( string var, string& value )
{
	try
	{
		CEntityBase::getValue( var, value );
	}
	catch(const CCharacter::EInvalidStat &e)
	{
		nlwarning("<CCharacter::getValue> Exception : %s",e.what( var ) );
		return false;
	}
	return true;
} // getValue //

//---------------------------------------------------
// Make all necessary update for player at each ticks
// (manage modifier, manage caracterisitics, derivated scores...)
// This method may not be called every game cycle, but at a lower timer rate
// (ex: every 16 game cycles)
//
//---------------------------------------------------
uint32 CCharacter::tickUpdate()
{
	H_AUTO(CharacterUpdate);
	if ( ! getEnterFlag() ) // wait for the properties to be in the mirror
		return 16;

	// update connexion stats.
	uint32 time = CTime::getSecondsSince1970();
	_PlayedTime += time - _LastConnectedTime;
	_LastConnectedTime = time;

	if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : start tickUpdate !!!") )
		return (uint32)-1;

	// save character if save delay is elapsed
	saveCharacter();

	{
		H_AUTO(CharacterLinkToMatrix);
		EntityMatrix.linkToMatrix(getState().X, getState().Y, _ListLink);

		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after EntityMatrix.linkToMatrix !!!") )
			return (uint32)-1;
	}
/*
	{
		H_AUTO(CharacterAntiBugCheckContinent);
		//ANTIBUG : CHARACTERS MUST BE IN A CONTINENT IF THEY ARE NOT BEING TELEPORTED
		//Note: Characters teleporting on a mount may provoke this warning!
		uint32 in = getInstanceNumber();
		// if the instance is invalid and char on valid coords, we have a problem. We ignore invalid coords because of teleports
		if ( in == ~0 && _EntityState.X > 0 && _EntityState.Y < 0)
		{
			nlwarning("<ANTIBUG>%s IS ON AN INVALID CONTINENT. x= %d, y = %d ",_Id.toString().c_str(),_EntityState.X(), _EntityState.Y() );
			CContinent * cont = CZoneManager::getInstance().getContinent(_EntityState.X,_EntityState.Y);
			if ( cont )
			{
				in = CUsedContinent::instance().getInstanceForContinent( CONTINENT::TContinent(cont->getId()) );
				if ( in == INVALID_AI_INSTANCE )
				{
					nlwarning("%s will arrive in an invalid continent (WE NAME : '%s') (REAL NAME : '%s')  ",_Id.toString().c_str(), NLMISC::strlwr( cont->getName() ).c_str(), CONTINENT::toString(CONTINENT::TContinent(cont->getId())).c_str() );
				}
			}
			else
			{
				nlwarning("<ANTIBUG>%s AT x= %d, y = %d NO VALID CONTINENT FOUND",_Id.toString().c_str(),_EntityState.X() , _EntityState.Y() );
			}
			setInstanceNumber( in );

			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after Antibug check continent !!!") )
				return (uint32)-1;
		}
		// end of ANTIBUG
	}
*/
	{
		H_AUTO(CharacterCheckEnterLeaveZone);
		// check if the player enter/ leaves a zone
		CZoneManager::getInstance().updateCharacterPosition( this );

		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after CZoneManager::updateCharacterPosition !!!") )
			return (uint32)-1;
	}

	// test progression
	static TGameCycle lastTickTestProgressionCalled = 0;
	if( TestProgression && CTickEventHandler::getGameCycle() > ( XpGainRate + lastTickTestProgressionCalled ) )
	{
		lastTickTestProgressionCalled = CTickEventHandler::getGameCycle();
		TReportAction ra;
		ra.ActorRowId = _EntityRowId;
		ra.ActionNature = ACTNATURE::CRAFT;
		ra.DeltaLvl = 0;
		ra.Skill = SKILLS::toSkill( TestProgressSkill );
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->actionReport( ra );
		PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->reportAction(ra);
	}

	{
		H_AUTO(CharacterProcessStaticAction);
		processStaticAction();
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after processStaticAction() !!!") )
			return (uint32)-1;
	}

	// update game event
	_GameEvent->tickUpdate();

	// keep old score values for reference
	sint32 oldHp = _PhysScores._PhysicalScores[ SCORES::hit_points ].Current;
	sint32 oldSta = _PhysScores._PhysicalScores[ SCORES::stamina].Current;
	sint32 oldSap = _PhysScores._PhysicalScores[ SCORES::sap ].Current;
	sint32 oldFocus = _PhysScores._PhysicalScores[ SCORES::focus].Current;

	if( _GodMode || _Invulnerable )
	{
		if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Current <= 0 )
		{
			_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Base;
		}
		if( _PhysScores._PhysicalScores[ SCORES::stamina ].Current < _PhysScores._PhysicalScores[ SCORES::stamina ].Base / 3 )
		{
			_PhysScores._PhysicalScores[ SCORES::stamina ].Current = _PhysScores._PhysicalScores[ SCORES::stamina ].Base;
		}
		if( _PhysScores._PhysicalScores[ SCORES::sap ].Current < _PhysScores._PhysicalScores[ SCORES::sap ].Base / 3 )
		{
			_PhysScores._PhysicalScores[ SCORES::sap ].Current = _PhysScores._PhysicalScores[ SCORES::sap ].Base;
		}
		if( _PhysScores._PhysicalScores[ SCORES::focus ].Current < _PhysScores._PhysicalScores[ SCORES::focus ].Base / 3 )
		{
			_PhysScores._PhysicalScores[ SCORES::focus ].Current = _PhysScores._PhysicalScores[ SCORES::focus ].Base;
		}
	}

	// Check Death of player and manage pact if death occurs
	if( currentHp() <= 0 && !_IsDead && !teleportInProgress() )
	{
		kill();
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after kill() !!!") )
			return (uint32)-1;
	}

	// test again as effects can kill the entity (dots...)
	if (!isDead())
	{
		{
			H_AUTO(CharacterComputeMaxValue);
			// Compute max values after apply modifier
			computeMaxValue();
			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after computeMaxValue() !!!") )
				return (uint32)-1;
		}
		{
			H_AUTO(CharacterRegenScores);
			// now we ready apply regen and clip currents value
			applyRegenAndClipCurrentValue();
			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after applyRegenAndClipCurrentValue() !!!") )
				return (uint32)-1;
		}
		{
			H_AUTO(CharacterGearLatency);
			// update gear latency
			_GearLatency->update(this);
			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after _GearLatency->update() !!!") )
				return (uint32)-1;
		}
	}
	else
	{
		deathOccurs();
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after death !!!") )
			return (uint32)-1;
		_PhysScores.CurrentWalkSpeed = 0;
		_PhysScores.CurrentRunSpeed = 0;
	}

	// reset modifiers now (after comptuing max value and regen, effects are applied by the TimerManager)
	{
		H_AUTO(CharacterModifierReset);
		if ( !isDead() )
		{
			// Set all character stats modifiers to initial state (depend on item etc..)
			resetCharacterModifier();
			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after resetCharacterModifier() !!!") )
				return (uint32)-1;
		}
	}

	{
		H_AUTO(CharacterClearDeadTargetInfo);
		// set TARGET HP, SAP and STAMINA in the database
		CEntityBase * target;
		if ( _Target.isReadable() )
		{
			target = CEntityBaseManager::getEntityBasePtr( getTarget() );
			if( target )
			{
				if (target->isDead())
				{
					// target is a player then do nothing, as dead players (in fact in a coma) can be targeted
					if (target->getId().getType() != RYZOMID::player)
					{
						if	(_Id.getType()==RYZOMID::player)
						{
							setTarget( CEntityId::Unknown );
						}
						// clear DB Bars
//						_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, 0 );
						CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, 0 );
//						_PropertyDatabase.setProp( _DataIndexReminder->TARGET.SAP, 0 );
						CBankAccessor_PLR::getTARGET().getBARS().setSAP(_PropertyDatabase, 0 );
//						_PropertyDatabase.setProp( _DataIndexReminder->TARGET.STA, 0 );
						CBankAccessor_PLR::getTARGET().getBARS().setSTA(_PropertyDatabase, 0 );
//						_PropertyDatabase.setProp( _DataIndexReminder->TARGET.FOCUS, 0 );
						CBankAccessor_PLR::getTARGET().getBARS().setFOCUS(_PropertyDatabase, 0 );
					}
				}
			}
		}
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after update dead target (clear HP/STA/SAP) !!!") )
			return (uint32)-1;
	}

	{
		H_AUTO(CharacterWriteTargetSelectInfos);
		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( _Target() );
		if( target )
		{
			sint8 percent;
			// UID
//			_PropertyDatabase.setProp( _DataIndexReminder->TARGET.UID, target->getEntityRowId().getCompressedIndex() );
			CBankAccessor_PLR::getTARGET().getBARS().setUID(_PropertyDatabase, target->getEntityRowId().getCompressedIndex() );

			// Hp
			if( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max <= 0 )
			{
				percent = 0;
			}
			else
			{
				percent = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
			}
//			_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, percent );
			CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, percent );

			// Sap
			if( target->getPhysScores()._PhysicalScores[ SCORES::sap ].Max <= 0 )
			{
				percent = 0;
			}
			else
			{
				percent = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::sap ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::sap ].Max ) );
			}
//			_PropertyDatabase.setProp( _DataIndexReminder->TARGET.SAP, percent );
			CBankAccessor_PLR::getTARGET().getBARS().setSAP(_PropertyDatabase, percent );

			// Stamina
			if( target->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max <= 0 )
			{
				percent = 0;
			}
			else
			{
				percent = sint8((127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::stamina ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max ) );
			}
//			_PropertyDatabase.setProp( _DataIndexReminder->TARGET.STA, percent );
			CBankAccessor_PLR::getTARGET().getBARS().setSTA(_PropertyDatabase, percent );

			// Focus
			if( target->getPhysScores()._PhysicalScores[ SCORES::focus ].Max <= 0 )
			{
				percent = 0;
			}
			else
			{
				sint8 percentTmp = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::focus ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::focus ].Max ) );
				if( percentTmp < 0 )
					percent = 0;
				else
					percent = percentTmp;
			}
//			_PropertyDatabase.setProp( _DataIndexReminder->TARGET.FOCUS, percent );
			CBankAccessor_PLR::getTARGET().getBARS().setFOCUS(_PropertyDatabase, percent );

			// Set the level of the target (if player character) in database for ForceRegion/ForceLevel deduction
			if ( target->getId().getType() == RYZOMID::player )
			{
				CCharacter *targetChar = safe_cast<CCharacter*>(target);
				sint32 skillBaseValue = targetChar->getSkillBaseValue( targetChar->getBestSkill() );
//				_PropertyDatabase.setProp( _DataIndexReminder->TARGET.PLAYER_LEVEL, skillBaseValue );
				CBankAccessor_PLR::getTARGET().getBARS().setPLAYER_LEVEL(_PropertyDatabase, checkedCast<uint8>(skillBaseValue) );
			}

			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after update target HP/STA/SAP !!!") )
				return (uint32)-1;
		}
	}

	// set life bar
	{
		H_AUTO(CharacterSetBars);
		setBars();
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after setBars() !!!") )
			return (uint32)-1;
	}

	{
		H_AUTO(CharacterWriteTeamInfos);
		// update team bars if needed
		if (_TeamId.getValue() != CTEAM::InvalidTeamId)
		{
			CTeam *team = TeamManager.getRealTeam(_TeamId.getValue());
			if (team)
			{
				uint8 bar = uint8((_StatusBars & (0x000007ff)) >> 3);
				if (_OldHpBarSentToTeam != bar)
				{
					team->updateCharacterScore(this, SCORES::hit_points, bar);
					_OldHpBarSentToTeam = bar;
				}
				bar = uint8( (_StatusBars & (0x0003f800)) >> 11);
				if (_OldStaBarSentToTeam != bar)
				{
					team->updateCharacterScore(this, SCORES::stamina, bar);
					_OldStaBarSentToTeam = bar;
				}
				bar = uint8( (_StatusBars & (0x1fc0000)) >> 18);
				if (_OldSapBarSentToTeam != bar)
				{
					team->updateCharacterScore(this, SCORES::sap, bar);
					_OldSapBarSentToTeam = bar;
				}
				if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after update team bars !!!") )
					return (uint32)-1;
			}
		}
	}

	{
		H_AUTO(CharacterWritePetCoordinateAndDb);
		// Update coordinate of player's pet
		updatePetCoordinateAndDatabase();
		if( !checkCharacterStillValide("Character corrupted : after updatePetCoordinateAndDatabase() !!!") )
			return (uint32)-1;
	}

	{
		if ( _LastTickCompassUpdated + TickFrequencyCompassUpdate.get() <= CTickEventHandler::getGameCycle() )
		{
			_LastTickCompassUpdated = CTickEventHandler::getGameCycle();
			{
				H_AUTO(CharacterUpdateMissionCompass);
				for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
				{
					nlassert((*it).second);
					(*it).second->updateCompass(*this, string());
				}
			}

			{
				H_AUTO(CharacterUpdateTeamCompass);
				CTeam * team = TeamManager.getRealTeam( _TeamId );
				if ( team )
				{
					const uint size = (uint)team->getMissions().size();
					for ( uint i =  0; i < size; i++ )
					{
						nlassert(team->getMissions()[i]);
						team->getMissions()[i]->updateCompass(*this, string(""));
						team->getMissions()[i]->updateCompass(*this, string("GROUP:"));
					}
				}
			}

			// Adding UpdateCompass for guild missions
			{
				H_AUTO(CharacterUpdateGuildCompass);
				CGuild * guild = CGuildManager::getInstance()->getGuildFromId( _GuildId );
				if ( guild )
				{
					const uint size = (uint)guild->getMissions().size();
					for ( uint i =  0; i < size; i++ )
					{
						nlassert(guild->getMissions()[i]);
						guild->getMissions()[i]->updateCompass(*this, string(""));
						guild->getMissions()[i]->updateCompass(*this, string("GROUP:"));
					}
				}
			}

			{
				H_AUTO(CharacterUpdateTargetCoordinatesCompass);
				// update compass coordinates information
				compassDatabaseUpdate();
			}
			if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after compassDatabaseUpdate() !!!") )
				return (uint32)-1;
		}
	}

	if (IsRingShard && _NpcControlEid != CEntityId::Unknown)
	{
		H_AUTO(CharacterUpdateNpcControl);
		if ( _LastTickNpcControlUpdated + TickFrequencyNpcControlUpdate.get() <= CTickEventHandler::getGameCycle() )
		{
			//	_LastTickNpcControlUpdated = CTickEventHandler::getGameCycle(); is done in setNpcControl
			{	// Update npc control
				setNpcControl(_NpcControlEid);
			}
		}
	}


	{
		H_AUTO(CharacterUpdateCombatEventFlags);
		// update combat flags
		updateCombatEventFlags();
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after updateCombatEventFlags() !!!") )
			return (uint32)-1;
	}
	{
		H_AUTO(CharacterUpdatePowerAndAuraFlags);
		// update power flags
		updatePowerAndAuraFlags();
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after updatePowerAndAuraFlags() !!!") )
			return (uint32)-1;
	}

	{
		H_AUTO(CharacterUpdateDisableModifier);
		// update disabled modifiers
		_ModifiersInDB.update(_PropertyDatabase);
		if( !checkCharacterStillValide("<CCharacter::tickUpdate> Character corrupted : after _ModifiersInDB.update() !!!") )
			return (uint32)-1;
	}

	{
		// update items prerequisits
		if( _HaveToUpdateItemsPrerequisit )
		{
			H_AUTO(CharacterUpdateItemsPrerequisit);

			CInventoryPtr bagInv = getInventory( INVENTORIES::bag );
			if( bagInv != NULL )
				bagInv->updateAllItemPrerequisit();

			for( uint i=INVENTORIES::pet_animal; i<INVENTORIES::max_pet_animal; ++i )
			{
				CInventoryPtr petInv = getInventory( (INVENTORIES::TInventory)i );
				if( petInv != NULL )
					petInv->updateAllItemPrerequisit();
			}

			CInventoryPtr roomInv = getInventory( INVENTORIES::player_room );
			if( roomInv != NULL )
				roomInv->updateAllItemPrerequisit();

			_HaveToUpdateItemsPrerequisit = false;
		}
	}

	bool updatePVP = false;
	{
		H_AUTO(CharacterUpdateOutpost);

		TAIAlias outpostAlias = getOutpostAlias();
		if( outpostAlias != 0 )
		{
			// if timer over : reset flag
			if( outpostLeavingDurationElapsed()	)
			{
				stopOutpostLeavingTimer();
				setOutpostAlias(0);
				updatePVP = true;
			}

			CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( outpostAlias );
			if( outpost )
			{
				// if peace : reset flag
				if( outpost->getState() == OUTPOSTENUMS::Peace )
				{
					stopOutpostLeavingTimer();
					setOutpostAlias(0);
					updatePVP = true;
				}
				else
				{
					outpost->fillCharacterOutpostDB(this);
					updatePVP = true;
				}
			}
		}
	}

	{
		H_AUTO(CharacterUpdatePVPMode);

		if (_PVPSafeLastTimeChange + 20 < CTickEventHandler::getGameCycle() || updatePVP)
		{
			_PVPSafeLastTimeChange = CTickEventHandler::getGameCycle();

			if (_PVPSafeLastTime != getSafeInPvPSafeZone())
			{
				_PVPSafeLastTime = !_PVPSafeLastTime;
				updatePVP = true;
			}

			if (_PVPInSafeZoneLastTime != CPVPManager2::getInstance()->inSafeZone(getPosition()))
			{
				_PVPInSafeZoneLastTime = !_PVPInSafeZoneLastTime;
				updatePVP = true;
			}
			
			if (updatePVP)
			{
				CPVPManager2::getInstance()->setPVPModeInMirror(this);
				updatePVPClanVP();
				_HaveToUpdatePVPMode = false;
			}
		}

		if( _HaveToUpdatePVPMode )
		{
			const TGameCycle waitTime = _PVPFlag ? TimeForSetPVPFlag.get() : TimeForPVPFlagOff.get();
			if (_PVPFlagLastTimeChange + waitTime < CTickEventHandler::getGameCycle())
			{
				CPVPManager2::getInstance()->setPVPModeInMirror(this);
				CPVPManager2::getInstance()->updateFactionChannel(this);
				updatePVPClanVP();
				_HaveToUpdatePVPMode = false;
			}
		}

		if( getPvPRecentActionFlag() == false )
		{
			CMirrorPropValue<TYPE_EVENT_FACTION_ID> propPvpMode( TheDataset, TheDataset.getDataSetRow(_Id), DSPropertyEVENT_FACTION_ID );
			if( propPvpMode.getValue()&PVP_MODE::PvpFactionFlagged )
			{
				CPVPManager2::getInstance()->setPVPModeInMirror(this);
				updatePVPClanVP();
			}
		}
	}

	// write faction points in database if needed
	if( CPVPFactionRewardManager::getInstance().getFactionPointPool( _DeclaredCult ) != _LastCultPointWriteDB )
	{
		_LastCultPointWriteDB = CPVPFactionRewardManager::getInstance().getFactionPointPool( _DeclaredCult );
//		_PropertyDatabase.setProp( "PVP_EFFECTS:PVP_FACTION_POINTS:CULT", _DeclaredCult );
		CBankAccessor_PLR::getPVP_EFFECTS().getPVP_FACTION_POINTS().setCULT(_PropertyDatabase, _DeclaredCult );
//		_PropertyDatabase.setProp( "PVP_EFFECTS:PVP_FACTION_POINTS:CULT_POINTS", _LastCultPointWriteDB );
		CBankAccessor_PLR::getPVP_EFFECTS().getPVP_FACTION_POINTS().setCULT_POINTS(_PropertyDatabase, _LastCultPointWriteDB );
	}
	if( CPVPFactionRewardManager::getInstance().getFactionPointPool( _DeclaredCiv ) != _LastCivPointWriteDB )
	{
		_LastCivPointWriteDB = CPVPFactionRewardManager::getInstance().getFactionPointPool( _DeclaredCiv );
//		_PropertyDatabase.setProp( "PVP_EFFECTS:PVP_FACTION_POINTS:CIV", _DeclaredCiv );
		CBankAccessor_PLR::getPVP_EFFECTS().getPVP_FACTION_POINTS().setCIV(_PropertyDatabase, _DeclaredCiv );
//		_PropertyDatabase.setProp( "PVP_EFFECTS:PVP_FACTION_POINTS:CIV_POINTS", _LastCivPointWriteDB );
		CBankAccessor_PLR::getPVP_EFFECTS().getPVP_FACTION_POINTS().setCIV_POINTS(_PropertyDatabase, _LastCivPointWriteDB );
	}

	uint32 nextUpdate = 16;
	if( oldHp != _PhysScores._PhysicalScores[ SCORES::hit_points ].Current ||
		oldSta != _PhysScores._PhysicalScores[ SCORES::stamina].Current ||
		oldSap != _PhysScores._PhysicalScores[ SCORES::sap ].Current ||
		oldFocus != _PhysScores._PhysicalScores[ SCORES::focus].Current )
	{
		nextUpdate = 8;
	}

	return nextUpdate;
} // tickUpdate //

//----------------------------------------------------------------------------
void CCharacter::setEnterFlag( bool b )
{
	if ( b )
	{
		// setup timer for tickUpdate() calling
		_TickUpdateTimer.setRemaining( 1, new CCharacterTickUpdateTimerEvent( this ),16 );
		_DeathPenaltyTimer.setRemaining(1, new CDeathPenaltiesTimerEvent( this ),16);
		_BarUpdateTimer.setRemaining(1, new CCharacterBarUpdateTimerEvent( this ),1);
	}
	else
	{
		_TickUpdateTimer.reset();
		_DbUpdateTimer.reset();
		_DeathPenaltyTimer.reset();
		_BarUpdateTimer.reset();
	}

	_Enter = b;
}

//-----------------------------------------------
// CCharacter::saveCharacter
//-----------------------------------------------
void CCharacter::saveCharacter()
{
	if( ( _LastTickSaved + TickFrequencyPCSave ) <= CTickEventHandler::getGameCycle() )
	{
		_LastTickSaved = CTickEventHandler::getGameCycle();
		PlayerManager.savePlayerActiveChar( PlayerManager.getPlayerId( _Id ) );
	}
}


//-----------------------------------------------
// CCharacter::kill
//-----------------------------------------------
void CCharacter::kill(TDataSetRow killerRowId)
{
	if (_IsDead)
		return;

	// force player to unmount
	if( TheDataset.isAccessible(_EntityMounted()) )
	{
		unmount();
	}

	std::vector< uint16 > usableRespawnPoints;
	getRespawnPoints().getUsableRespawnPoints(getCurrentContinent(), usableRespawnPoints);
	if ( usableRespawnPoints.empty() )
	{
		getRespawnPoints().addDefaultRespawnPoint(getCurrentContinent());
	}

	_IsDead = true;
	_Mode = MBEHAV::DEATH;

	removeAllSpells();
	_ForbidPowerDates.clearConsumable();

	_PhysScores._PhysicalScores[SCORES::hit_points].Current = -_PhysScores._PhysicalScores[SCORES::hit_points].Max / 2;
	setBars();

	_TimeDeath = CTickEventHandler::getGameTime() + 5.0;

	// stop all temp inventory actions
	sendCloseTempInventoryImpulsion();

	// end quartering if in progress
	endHarvest();

	// stop all temp inventory actions
	clearTempInventory();

	// Testing tools report
	if( TTSIsUp )
	{
		CMessage msgout("TTS_REPORT_ACTOR_DEAD");
		msgout.serial( _Id );
		CUnifiedNetwork::getInstance()->send( "TTS", msgout );
	}

	_ContextualProperty.directAccessForStructMembers().talkableTo( false );
	_ContextualProperty.setChanged();

	CPhraseManager::getInstance().removeEntity(_EntityRowId, false);

	if (_TpTicketSlot != INVENTORIES::INVALID_INVENTORY_SLOT)
	{
		unLockItem(INVENTORIES::bag, _TpTicketSlot,1);
		resetTpTicketSlot();
	}

	// Output Stats
	string placeName = string("None");
	string regionName = string("None");
	if ( !_Places.empty() )
	{
		CPlace * p = CZoneManager::getInstance().getPlaceFromId( _Places[0] );
		if( p )
			placeName = p->getName();
	}

	const CRegion * r=NULL;
	CZoneManager::getInstance().getRegion( this, &r);
	if( r )
	{
		regionName = r->getName();
	}

	// if best skill of player is too low then he doesn't get death penalty
	if( getBestChildSkillValue(SKILLS::any) < DeathPenaltyMinLevel )
	{
		setNextDeathPenaltyFactor(0.0f);
	}

	// check killer and do things accordingly
	CEntityId killerId;
	CSheetId killerSheet;
	CEntityBase * e = CEntityBaseManager::getEntityBasePtr(killerRowId);
	if( e )
	{
		// clear XP gain for this player
		PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->clearAllXpForPlayer(_EntityRowId, _TeamId, false);
		if( e->getId().getType() == RYZOMID::player )
		{
			PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerDeath(this, (CCharacter*)e);
		}

		killerId = e->getId();
		killerSheet = e->getType();

		if ( ! (getPVPInterface().isValid() &&
			    getPVPInterface().killedBy( e )) )
		{
			// if killed by a creature, check if death penalty should be applied or not
			if ( killerId.getType() == RYZOMID::creature || killerId.getType() == RYZOMID::npc )
			{
				CCreature *creature = dynamic_cast<CCreature*> (e);
				if (creature && creature->getForm())
				{
					if ( creature->getForm()->getXPGainOnCreature() == 0 )
					{
						setNextDeathPenaltyFactor(0.0f);
					}
				}
			}
		}

		if( killerId.getType() == RYZOMID::player )
		{
			if( getDuelOpponent() )
			{
				if( getDuelOpponent()->getId() != killerId )
				{
					setNextDeathPenaltyFactor(0.0f);
				}
			}
			else
			{
				setNextDeathPenaltyFactor(0.0f);
			}
		}
	}
	//Bsi.append( StatPath, NLMISC::toString("[PJM] %s %s %s %s %s %s", _Id.toString().c_str(), CONTINENT::toString(_CurrentContinent).c_str(), regionName.c_str(), placeName.c_str(), KillerId.toString().c_str(), KillerSheet.toString().c_str()) );
	//EgsStat.displayNL("[PJM] %s %s %s %s %s %s", _Id.toString().c_str(), CONTINENT::toString(_CurrentContinent).c_str(), regionName.c_str(), placeName.c_str(), KillerId.toString().c_str(), KillerSheet.toString().c_str());
//	EGSPD::pCDead(_Id, CONTINENT::toString(_CurrentContinent), regionName, placeName, killerId, killerSheet.toString());
} // kill //


//---------------------------------------------------
// character is dead
//
//---------------------------------------------------
void CCharacter::deathOccurs( void )
{
	H_AUTO(DeathOccursCharacter);

	if( currentHp() > 0 )
	{
		resurrected();
		return;
	}

	if ( getPVPInterface().isValid() )
	{
		// ignore PVP death
		const bool cancelRespawn = getPVPInterface().doCancelRespawn();

		getPVPInterface().leavePVP(IPVP::Death);

		if (cancelRespawn)
			return;
	}

	CPVPManager2::getInstance()->playerDies(this);

	CBuildingManager::getInstance()->removeTriggerRequest(getEntityRowId());

	if( _TimeDeath < CTickEventHandler::getGameTime() )
	{
		if (_Mode.getValue().Mode == MBEHAV::DEATH)
		{
			// Cancel all action during death
			cancelStaticActionInProgress();

			if( _Mode.getValue().Mode == MBEHAV::DEATH && _IsDead == true )
			{
				_TimeDeath = CTickEventHandler::getGameTime() + CommaDelayBeforeDeath;
				CPhraseManager::getInstance().removeEntity(TheDataset.getDataSetRow(_Id), false);
			}
			else
			{
				//todo make necessary for stop vision of character
			}
		}
	}

	// update regen
	if( !_IsInAComa )
	{
		resetCharacterModifier();
		computeMaxValue();

		// negative regen giving healing times for resurrect character
		for( uint32 i = 0; i < SCORES::NUM_SCORES; ++i )
		{
			if( i == SCORES::hit_points )
			{
				float currentRegen = - _PhysScores._PhysicalScores[ i ].Max / (CommaDelayBeforeDeath * 0.2f );
				_PhysScores._PhysicalScores[ i ].CurrentRegenerate = currentRegen;
				_PhysScores._PhysicalScores[ i ].Current = - _PhysScores._PhysicalScores[ i ].Max / 2;
			}
			else
			{
				float currentRegen = - _PhysScores._PhysicalScores[ i ].Current / ( CommaDelayBeforeDeath * 0.1f );
				_PhysScores._PhysicalScores[ i ].CurrentRegenerate = currentRegen;
			}
		}
		_IsInAComa = true;
	}

	for( uint32 i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		sint32 oldCurrent = _PhysScores._PhysicalScores[ i ].Current;
		if( i == SCORES::hit_points )
		{
			if( _PhysScores._PhysicalScores[ i ].Current > - _PhysScores._PhysicalScores[ i ].Max )
			{
				_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal += _PhysScores._PhysicalScores[ i ].CurrentRegenerate * ( CTickEventHandler::getGameCycle() - _PhysScores._PhysicalScores[ i ].RegenerateTickUpdate ) / 10.0f;
				_PhysScores._PhysicalScores[ i ].Current = (sint32) ( _PhysScores._PhysicalScores[ i ].Current + (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal );
				_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal -= (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal;
			}
			if( _PhysScores._PhysicalScores[ i ].Current < -_PhysScores._PhysicalScores[ i ].Max )
			{
				_PhysScores._PhysicalScores[ i ].Current = - _PhysScores._PhysicalScores[ i ].Max;
			}
			_PhysScores._PhysicalScores[ i ].RegenerateTickUpdate = CTickEventHandler::getGameCycle();
		}
		else
		{
			if( _PhysScores._PhysicalScores[ i ].Current > 0 )
			{
				_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal += _PhysScores._PhysicalScores[ i ].CurrentRegenerate * ( CTickEventHandler::getGameCycle() - _PhysScores._PhysicalScores[ i ].RegenerateTickUpdate ) / 10.0f;
				_PhysScores._PhysicalScores[ i ].Current = (sint32) ( _PhysScores._PhysicalScores[ i ].Current + (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal );
				_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal -= (sint32) _PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal;
			}
			if( _PhysScores._PhysicalScores[ i ].Current < 0 )
			{
				_PhysScores._PhysicalScores[ i ].Current = 0;
			}
			_PhysScores._PhysicalScores[ i ].RegenerateTickUpdate = CTickEventHandler::getGameCycle();
		}
	}
	setBars();
}


//---------------------------------------------------
// player choose a re-spawn for his death character
//
//---------------------------------------------------
void CCharacter::respawn( uint16 index )
{
	// ignore message if player isn't dead
	if (!_IsDead)
	{
		return;
	}

	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerRespawn(this);

	sint32 x,y,z;
	float heading;

	if( getRespawnPoints().getRingAdventuresRespawnPoint( x, y ) )
	{
		z = 0;
		heading = 0.0f;
	}
	else
	{
		vector<uint16> points;
		getRespawnPoints().getUsableRespawnPoints(getCurrentContinent(), points);
		if ( index >= points.size() )
		{
			nlwarning("<RESPAWN_POINT> invalid point %u for user %s ( count = %u)",index,_Id.toString().c_str(),points.size());
			index = 0;
			getRespawnPoints().addDefaultRespawnPoint(getCurrentContinent());
			getRespawnPoints().getUsableRespawnPoints(getCurrentContinent(), points);

			if ( index >= points.size() )
			{
				nlwarning("<RESPAWN_POINT> invalid default point %u for user %s. CurrentContinent %d x = %d, y = %d",index,_Id.toString().c_str(), getCurrentContinent(), getState().X(),getState().Y() );
				return;
			}
		}

		// get the tp coords
		const CTpSpawnZone* zone = CZoneManager::getInstance().getTpSpawnZone( points[index] );
		if ( !zone )
		{
			nlwarning("<RESPAWN_POINT> invalid point %u for user %s ( count = %u) ( NULL zone returned )",index,_Id.toString().c_str(),points.size());
			return;
		}
		zone->getRandomPoint(x,y,z,heading);
	}

	// remove character of vision of other PC
	CMessage msgout("ENTITY_TELEPORTATION");
	msgout.serial( _Id );
	if (IsRingShard)
	{
		nlinfo("Asking GPMS to TP character %s to (0,0) for respawn",_Id.toString().c_str());
	}
	sendMessageViaMirror("GPMS", msgout);

	forbidNearPetTp();

	// set player to intangible state
	_IntangibleEndDate = ~0;

	applyRespawnEffects();

	// tpWanted() sends message CAIPlayerRespawnMsg to AIS
	tpWanted( x, y, z, true, heading );

	// give spire effect if needed
	CPVPFactionRewardManager::getInstance().giveTotemsEffects( this );

	_RegionKilledInPvp = 0xffff;
}

//---------------------------------------------------
// apply respawn effects
//
//---------------------------------------------------
void CCharacter::applyRespawnEffects()
{
	if ( _NextDeathPenaltyFactor != 0 )
		_DeathPenalties->addDeath(*this, _NextDeathPenaltyFactor);
	resetNextDeathPenaltyFactor();

	_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Base / 10;
	_PhysScores._PhysicalScores[ SCORES::stamina ].Current = _PhysScores._PhysicalScores[ SCORES::stamina ].Base / 10;
	_PhysScores._PhysicalScores[ SCORES::sap ].Current = _PhysScores._PhysicalScores[ SCORES::sap ].Base / 10;
	_PhysScores._PhysicalScores[ SCORES::focus ].Current = _PhysScores._PhysicalScores[ SCORES::focus ].Base / 10;
	_Mode = MBEHAV::NORMAL;
	_Behaviour = MBEHAV::IDLE;
	_IsDead = false;
	_IsInAComa = false;
}

//---------------------------------------------------
// player accept resurrection by other character
//
//---------------------------------------------------
void CCharacter::resurrected()
{
	_Mode = MBEHAV::NORMAL;
	_Behaviour = MBEHAV::IDLE;
	_IsDead = false;
	_IsInAComa = false;
	resetNextDeathPenaltyFactor();

	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerResurrected(this);

	// give spire effect if needed
	CPVPFactionRewardManager::getInstance().giveTotemsEffects( this );

	_RegionKilledInPvp = 0xffff;
}


//---------------------------------------------------
// revive
// player revives at full health at his location without death penalty
//---------------------------------------------------
void CCharacter::revive()
{
	_Mode = MBEHAV::NORMAL;
	_Behaviour = MBEHAV::IDLE;
	_IsDead = false;
	_IsInAComa = false;
	_RegionKilledInPvp = 0xffff;

	_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Base;
	_PhysScores._PhysicalScores[ SCORES::stamina ].Current = _PhysScores._PhysicalScores[ SCORES::stamina ].Base;
	_PhysScores._PhysicalScores[ SCORES::sap ].Current = _PhysScores._PhysicalScores[ SCORES::sap ].Base;
	_PhysScores._PhysicalScores[ SCORES::focus ].Current = _PhysScores._PhysicalScores[ SCORES::focus ].Base;
}

//---------------------------------------------------
// Buy kami or karavan pact for a respawn point
//
//---------------------------------------------------
void CCharacter::buyPact( const std::string& PactName )
{
	///TODO RESPAWN
}

//---------------------------------------------------
//	incNbAura
//---------------------------------------------------
void CCharacter::incNbAura()
{
	++_NbAuras;
	if (_NbAuras == 1)
	{
		CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
		CVisualFX fx;
		fx.unpack(visualFx.getValue());
		fx.AuraReceipt = true;
		sint64 prop;
		fx.pack(prop);
		visualFx = (sint16)prop;
	}
}

//---------------------------------------------------
//	decNbAura
//---------------------------------------------------
void CCharacter::decNbAura()
{
	if(_NbAuras)
		--_NbAuras;

	if (_NbAuras == 0)
	{
		// set visual fx on the entity
		CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _EntityRowId, DSPropertyVISUAL_FX );
		CVisualFX fx;
		fx.unpack(visualFx.getValue());
		fx.AuraReceipt = false;
		sint64 prop;
		fx.pack(prop);
		visualFx = (sint16)prop;
	}
}

//---------------------------------------------------
// displayPowerFlags
//---------------------------------------------------
void CCharacter::displayPowerFlags()
{
	for (uint i = 0 ; i < 32 ; ++i)
	{
		BRICK_FLAGS::TBrickFlag flag = (BRICK_FLAGS::TBrickFlag) (BRICK_FLAGS::BeginPowerFlags + i);
		nldebug("Power %s, flag = %u startTick = %u endTick = %u", BRICK_FLAGS::toString(flag).c_str(), i, _PowerFlagTicks[i].StartTick, _PowerFlagTicks[i].EndTick);
	}
}

//---------------------------------------------------
// Entity want mount another
//---------------------------------------------------
void CCharacter::mount( TDataSetRow PetRowId )
{
	if ( !R2_VISION::isEntityVisibleToPlayers(getWhoSeesMe()) )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "CANT_MOUNT_WHILE_INVISIBLE" ); // not translated (only for GM)
		return;
	}

	CEntityBase * e = CEntityBaseManager::getEntityBasePtr( PetRowId );
	if( e )
	{
		const CStaticCreatures * form = e->getForm();
		if( form )
		{
			if( form->getProperties().mountable() )
			{
				if( e->getRiderEntity().isNull() )
				{
					setAfkState(false);
					sint32 petIndex = getPlayerPet( PetRowId );
					if( (petIndex!=-1) /*&& (staticActionInProgress() != true)*/ )
					{
						COfflineEntityState state =  e->getState();
						CVector2d destination( state.X, state.Y );
						CVector2d start( _EntityState.X, _EntityState.Y );
						float distance = (float)(start - destination).sqrnorm();
						if( distance <= MaxTalkingDistSquare * 1000 * 1000 )
						{
							// prevent from giving the mount which the player mounts
							abortExchange();

							setEntityMounted( e->getEntityRowId() );

							// set the rider for entity
							e->setRiderEntity( _EntityRowId );

							// remember the mount state
							_PlayerPets[petIndex].IsMounted = true;

//							_PropertyDatabase.setProp( "USER:MOUNT_WALK_SPEED", (sint64)(sint)(e->getPhysScores().CurrentWalkSpeed() * 1000.0f) );
							CBankAccessor_PLR::getUSER().setMOUNT_WALK_SPEED(_PropertyDatabase, checkedCast<uint16>(e->getPhysScores().CurrentWalkSpeed() * 1000.0f) );
//							_PropertyDatabase.setProp( "USER:MOUNT_RUN_SPEED", (sint64)(sint)(e->getPhysScores().CurrentRunSpeed() * 1000.0f) );
							CBankAccessor_PLR::getUSER().setMOUNT_RUN_SPEED(_PropertyDatabase, checkedCast<uint16>(e->getPhysScores().CurrentRunSpeed() * 1000.0f) );

							if( e->getContextualProperty().directAccessForStructMembers().mountable() )
							{
								setMode( MBEHAV::MOUNT_NORMAL, true );

								// egs_chinfo("<CEntityBase::setMode> %d Set Mode to %d for entity %s", CTickEventHandler::getGameCycle(), MBEHAV::EMode(_Mode.getValue().Mode), _Id.toString().c_str() );
								CMessage msgout("ACQUIRE_CONTROL");
								CEntityId mountedEntityId = TheDataset.getEntityId( getEntityMounted() );
								msgout.serial( mountedEntityId );
								msgout.serial( _Id );
								sint32 local = 0;
								msgout.serial( local );
								msgout.serial( local );
								msgout.serial( local );
								sendMessageViaMirror( "GPMS", msgout );

								e->setMode( MBEHAV::MOUNT_NORMAL, true );

								_PhysScores.BaseWalkSpeed = _PhysScores.CurrentWalkSpeed = form->getWalkSpeed();
								_PhysScores.BaseRunSpeed = _PhysScores.CurrentRunSpeed = form->getRunSpeed();

								e->getContextualProperty().directAccessForStructMembers().mountable( false );
								e->getContextualProperty().setChanged();
								setTarget( CEntityId::Unknown );
							}
							else
							{
								CMessage msgout( "IMPULSION_ID" );
								msgout.serial( _Id );
								CBitMemStream bms;
								if ( ! GenericMsgManager.pushNameToStream( "ANIMALS:MOUNT_ABORT", bms) )
								{
									nlwarning("<CEntityBase::tickUpdate> Msg name ANIMALS:MOUNT_ABORT not found");
									return;
								}
								msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
								CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );

								// reset vars
								if (e)
								{
									e->setRiderEntity( TDataSetRow() );
								}
								_EntityMounted = TDataSetRow();
								for ( uint i=0; i!=_PlayerPets.size(); ++i )
								{
									if ( _PlayerPets[i].IsMounted )
									{
										_PlayerPets[i].IsMounted = false;
										break;
									}
								}
							}

							return;
						}
					}
					return;
				}
			}
			else
			{
				nlwarning("<cbAnimalMount> %d Entity %s %s is not moutable !! sheeter or client bug ?", CTickEventHandler::getGameCycle(), e->getId().toString().c_str(), e->getType().toString().c_str() );
			}
		}
		else
		{
			nlwarning("<cbAnimalMount> %d Can't found static form sheet for entity %s %s !!", CTickEventHandler::getGameCycle(), e->getId().toString().c_str(), e->getType().toString().c_str() );
		}
	}

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "ANIMALS:MOUNT_ABORT", bms) )
	{
		nlwarning("<CEntityBase::tickUpdate> Msg name ANIMALS:MOUNT_ABORT not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}


//---------------------------------------------------
// Unmount
// If changeMountedState is false, IsMounted is not changed (useful to remember the state when teleporting).
// If petIndex is ~0, auto-finds the index of the first mount for which IsMounted is true.
//---------------------------------------------------
void CCharacter::unmount( bool changeMountedState, uint petIndex )
{
	setMode( MBEHAV::NORMAL, true );
	//	egs_chinfo("<CEntityBase::unseat> %d Set Mode to %d for entity %s", CTickEventHandler::getGameCycle(), MBEHAV::EMode(_Mode.getValue().Mode) , _Id.toString().c_str() );

	CMessage msgout("LEAVE_CONTROL");
	msgout.serial( _Id );
	sendMessageViaMirror( "GPMS", msgout );

	CEntityBase * e = CEntityBaseManager::getEntityBasePtr( getEntityMounted() );
	if( e )
	{
		e->setMode( MBEHAV::NORMAL, true );
		e->setRiderEntity( TDataSetRow() );
		e->getContextualProperty().directAccessForStructMembers().mountable( true );
		e->getContextualProperty().setChanged();
	}

	{
		_PhysScores.BaseWalkSpeed = _PhysScores.CurrentWalkSpeed = 1.3f;
		_PhysScores.BaseRunSpeed = _PhysScores.CurrentRunSpeed = 6.0f;
	}

	_EntityMounted = TDataSetRow();

	// Set state
	if ( changeMountedState )
	{
		if ( petIndex == ~0 )
		{
			for ( uint i=0; i!=_PlayerPets.size(); ++i )
			{
				if ( _PlayerPets[i].IsMounted )
				{
					petIndex = i;
					break;
				}
			}
			if ( petIndex == ~0 )
			{
				nlwarning( "Mounted pet not found for %s", getId().toString().c_str() );
				return;
			}
		}
		else
		{
			BOMB_IF( petIndex >= _PlayerPets.size(),
					 toString( "Pet index out of range for %s", getId().toString().c_str() ),
					 return );
		}
		_PlayerPets[petIndex].IsMounted = false;
	}
}


/*
 * Return the mount if the player is mounted, otherwise NULL
 */
CEntityBase *CCharacter::getMountEntity()
{
	return CEntityBaseManager::getEntityBasePtr( getEntityMounted() );
}


//---------------------------------------------------
// Set all character stats modifiers to initale states
//---------------------------------------------------
void CCharacter::resetCharacterModifier()
{
	// NEVER RESET MODIFIERS they must be managed by effect themselves
} // resetCharacterModifier //


//---------------------------------------------------
// recompute all Max value
//
//---------------------------------------------------
void CCharacter::computeMaxValue()
{
	int i;
	// Characteristics
	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		_PhysCharacs._PhysicalCharacteristics[ i ].Max = _PhysCharacs._PhysicalCharacteristics[ i ].Modifier + _PhysCharacs._PhysicalCharacteristics[ i ].Base;
		if( _PhysCharacs._PhysicalCharacteristics[ i ].OldMax != _PhysCharacs._PhysicalCharacteristics[ i ].Max )
		{
			// antibug becaus in mirror == 133 and is a bool...
			if ( _PhysCharacs._PhysicalCharacteristics[ i ].Current.testFlagInMirror() > 1 )
			{
				nlwarning("%s NASTY MEMORY BUG in computeMaxValue. current carac %d ",_Id.toString().c_str(), i );
				continue;
			}
			if ( _PhysCharacs._PhysicalCharacteristics[ i ].Max.testFlagInMirror() > 1 )
			{
				nlwarning("%s NASTY MEMORY BUG in computeMaxValue. max carac %d ",_Id.toString().c_str(), i );
				continue;
			}

			// Characteristics should not be negative
			_PhysCharacs._PhysicalCharacteristics[ i ].Current = std::max(sint32(_PhysCharacs._PhysicalCharacteristics[ i ].Max.getValue()), sint32(1));
			if( _PhysCharacs._PhysicalCharacteristics[ i ].OldCurrent != _PhysCharacs._PhysicalCharacteristics[ i ].Current )
			{
//				_PropertyDatabase.setProp( string("CHARACTER_INFO:CHARACTERISTICS:")  + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics)i ) , _PhysCharacs._PhysicalCharacteristics[ i ].Current );
				CBankAccessor_PLR::getCHARACTER_INFO().getCHARACTERISTICS(i).setVALUE(_PropertyDatabase, checkedCast<uint16>(_PhysCharacs._PhysicalCharacteristics[ i ].Current()) );
				_PhysCharacs._PhysicalCharacteristics[ i ].OldCurrent = _PhysCharacs._PhysicalCharacteristics[ i ].Current;
			}
		}
	}

	// Scores
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		// antibug because in mirror == 133 and is a bool...
		if ( _PhysScores._PhysicalScores[ i ].Current.testFlagInMirror() > 1 )
		{
			nlwarning("%s NASTY MEMORY BUG in computeMaxValue. current score %d ",_Id.toString().c_str(), i );
			continue;
		}
		if ( _PhysScores._PhysicalScores[ i ].Max.testFlagInMirror() > 1 )
		{
			nlwarning("%s NASTY MEMORY BUG in computeMaxValue. max score %d ",_Id.toString().c_str(), i );
			continue;
		}
		switch( i )
		{
			case SCORES::hit_points:
				_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::constitution ].Current + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor + _ScorePermanentModifiers[i];
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::metabolism ].Current / RegenDivisor;// + _PhysScores._PhysicalScores[ i ].RegenerateModifier;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			case SCORES::sap:
				_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::intelligence ].Current + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor + _ScorePermanentModifiers[i];
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::wisdom ].Current / RegenDivisor;// + _PhysScores._PhysicalScores[ i ].RegenerateModifier;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			case SCORES::stamina:
				_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::strength ].Current + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor + _ScorePermanentModifiers[i];
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::well_balanced ].Current / RegenDivisor;// + _PhysScores._PhysicalScores[ i ].RegenerateModifier;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			case SCORES::focus:
				_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::dexterity ].Current + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor + _ScorePermanentModifiers[i];
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::will ].Current / RegenDivisor;// - _PhysScores._PhysicalScores[ i ].RegenerateModifier;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			default:;
		}

		// add regen offset
		_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos += RegenOffset;
		_PhysScores._PhysicalScores[ i ].BaseRegenerateAction += RegenOffset;

		//Malkav : done in applyRegenAndClipCurrentValue(), so removed it
//		_PhysScores._PhysicalScores[ i ].CurrentRegenerate = _PhysScores._PhysicalScores[ i ].BaseRegenerateAction;

		_PhysScores._PhysicalScores[ i ].Max = _PhysScores._PhysicalScores[ i ].Base + _PhysScores._PhysicalScores[ i ].Modifier;
		if( _PhysScores._PhysicalScores[ i ].Max < 1 )
		{
			_PhysScores._PhysicalScores[ i ].Max = 1;
		}

		if( _PhysScores._PhysicalScores[ i ].OldMax != _PhysScores._PhysicalScores[ i ].Max )
		{
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.MaxScore[i], _PhysScores._PhysicalScores[ i ].Max );
			CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(i).setMax(_PropertyDatabase, _PhysScores._PhysicalScores[ i ].Max );
			_PhysScores._PhysicalScores[ i ].OldMax = _PhysScores._PhysicalScores[ i ].Max;
		}
	}

	// Skills
	for( i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		_Skills._Skills[ i ].Current = _Skills._Skills[ i ].Modifier + _Skills._Skills[ i ].Base;
	}
} // computeMaxValue //

//---------------------------------------------------
// apply regenerate and clip currents value
//
//---------------------------------------------------
void CCharacter::applyRegenAndClipCurrentValue()
{
	// First compute all current regen
	int i;

	float currentRegen;
	float baseRegen;
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		if( _Mode().Mode == MBEHAV::SIT )
		{
			baseRegen = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos;
		}
		else
		{
			baseRegen = _PhysScores._PhysicalScores[ i ].BaseRegenerateAction;
		}

		currentRegen = baseRegen + _PhysScores._PhysicalScores[ i ].RegenerateModifier;

		if ( currentRegen < 0 )
		{
			currentRegen = 0;
		}
		if( currentRegen != _PhysScores._PhysicalScores[ i ].CurrentRegenerate )
		{
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.BaseRegen[i], (uint32)(baseRegen * 10.0f), true );
			CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(i).setBaseRegen(_PropertyDatabase, (uint32)(baseRegen * 10.0f), true );
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.Regen[i], (uint32)(currentRegen * 10.0f) , true );
			CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(i).setRegen(_PropertyDatabase, (uint32)(currentRegen * 10.0f) , true );
			_PhysScores._PhysicalScores[ i ].CurrentRegenerate = currentRegen;
		}
	}

	sint32 weightMalus = getWeightMalus();

	// Compute current Skills
	for( i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		_Skills._Skills[ i ].Current = _Skills._Skills[ i ].Modifier + _Skills._Skills[ i ].Base;
		if ( _Skills._Skills[ i ].Current < 0 )
			_Skills._Skills[ i ].Current = 0;
		if( _Skills._Skills[ i ].OldCurrent != _Skills._Skills[ i ].Current )
		{
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.Skill[i], _Skills._Skills[ i ].Current );
			CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(i).setSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ i ].Current) );
			_Skills._Skills[ i ].OldCurrent = _Skills._Skills[ i ].Current;
		}
	}

	sint32 oldCurrent;
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		oldCurrent = _PhysScores._PhysicalScores[ i ].Current;
		if( _PhysScores._PhysicalScores[ i ].Current < _PhysScores._PhysicalScores[ i ].Max )
		{
			_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal += _PhysScores._PhysicalScores[ i ].CurrentRegenerate * ( CTickEventHandler::getGameCycle() - _PhysScores._PhysicalScores[ i ].RegenerateTickUpdate ) / 10.0f;
			const sint32 regenWholePart = sint32(_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal);
			_PhysScores._PhysicalScores[ i ].Current = sint32(_PhysScores._PhysicalScores[ i ].Current + regenWholePart);
			_PhysScores._PhysicalScores[ i ].KeepRegenerateDecimal -= regenWholePart;

			if (i == SCORES::hit_points)
			{
				PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->playerRegenHP(this, regenWholePart);
			}
		}
		if( _PhysScores._PhysicalScores[ i ].Current > _PhysScores._PhysicalScores[ i ].Max )
		{
			_PhysScores._PhysicalScores[ i ].Current = _PhysScores._PhysicalScores[ i ].Max;
		}
		else if( _PhysScores._PhysicalScores[ i ].Current < 0 ) _PhysScores._PhysicalScores[ i ].Current = 0;
		_PhysScores._PhysicalScores[ i ].RegenerateTickUpdate = CTickEventHandler::getGameCycle();
	}

	// restore value without weight malus
	_PhysScores.SpeedVariationModifier -= _LastAppliedWeightMalus;

	// compute new value
	_LastAppliedWeightMalus = getWeightMalus();
	_PhysScores.SpeedVariationModifier += _LastAppliedWeightMalus;

	sint16 speedVariationModifier = std::max( (sint)_PhysScores.SpeedVariationModifier, (sint)-100 );

	// Speed
	// while stunned/root/mezzed etc speed is forced to 0
	float oldCurrentSpeed;
	if( _Mode().Mode != MBEHAV::SIT && canEntityMove() && !PlayerManager.isRootedByGM( _Id ) )
	{
		float pcSpeed = 1.0f + speedVariationModifier / 100.0f;
		oldCurrentSpeed = _PhysScores.CurrentWalkSpeed;
		_PhysScores.CurrentWalkSpeed = pcSpeed * _PhysScores.BaseWalkSpeed;
		//if( oldCurrentSpeed != _PhysScores.CurrentWalkSpeed )
		//{
		//	_PropertyDatabase.setProp( "USER:WALK_SPEED", (uint32)(_PhysScores.CurrentWalkSpeed * 1000) );
		//}
		oldCurrentSpeed = _PhysScores.CurrentRunSpeed;
		_PhysScores.CurrentRunSpeed = pcSpeed * _PhysScores.BaseRunSpeed;
		//if( oldCurrentSpeed != _PhysScores.CurrentRunSpeed )
		//{
		//	_PropertyDatabase.setProp( "USER:RUN_SPEED", (uint32)(_PhysScores.CurrentRunSpeed * 1000) );
		//}

//		_PropertyDatabase.setProp( "USER:SPEED_FACTOR", sint64( speedVariationModifier + 100.0f ) );
		CBankAccessor_PLR::getUSER().setSPEED_FACTOR(_PropertyDatabase, checkedCast<uint8>( speedVariationModifier + 100.0f ) );
	}
	else
	{
		oldCurrentSpeed = _PhysScores.CurrentWalkSpeed;
		_PhysScores.CurrentWalkSpeed = 0;
		//if( oldCurrentSpeed != _PhysScores.CurrentWalkSpeed )
		//{
		//	_PropertyDatabase.setProp( "USER:WALK_SPEED", (uint32)(_PhysScores.CurrentWalkSpeed * 1000) );
		//}
		oldCurrentSpeed = _PhysScores.CurrentRunSpeed;
		_PhysScores.CurrentRunSpeed = 0;
		//if( oldCurrentSpeed != _PhysScores.CurrentRunSpeed )
		//{
		//	_PropertyDatabase.setProp( "USER:RUN_SPEED", (uint32)(_PhysScores.CurrentRunSpeed * 1000) );
		//}

//		_PropertyDatabase.setProp( "USER:SPEED_FACTOR", sint64(0) );
		CBankAccessor_PLR::getUSER().setSPEED_FACTOR(_PropertyDatabase, 0 );
	}

} // applyRegenAndClipCurrentValue //


//---------------------------------------------------
// Process static actions (like harvest, faber...)
//
//---------------------------------------------------
void CCharacter::processStaticAction()
{
	static sint countTickNotMoving = 0;

	if (_NbStaticActiveEffects > 0)
	{
		if ( hasMovedDuringStaticAction() )
		{
			cancelStaticEffects();
		}
	}

	// test if the character has moved since last update (if character is in action mode)
	if ( _StaticActionInProgress || _HarvestOpened )
	{
		bool stopAction = false;

		// Casting is canceled by event the smallest move
		switch ( _StaticActionType )
		{
		case STATIC_ACT_TYPES::Casting:
		case STATIC_ACT_TYPES::Teleport:
			stopAction = hasMovedDuringStaticAction();
			break;
		case STATIC_ACT_TYPES::Mount:
			{
				double sqrDistance = CVector2d( _EntityState.X() - _OldPosX, _EntityState.Y() - _OldPosY ).sqrnorm();
				stopAction = ( sqrDistance > ( MaxMountDistance * MaxMountDistance * 1000.0 * 1000.0 ) );
			}
			break;
		case STATIC_ACT_TYPES::BotChat:
			{
				CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( _CurrentInterlocutor );
				if (entity)
				{
					double sqrDistance = CVector2d( _EntityState.X() - entity->getX(), _EntityState.Y() - entity->getY() ).sqrnorm();

					// Special case if it is an outpost building
					CCreature *c = dynamic_cast<CCreature*>(entity);
					if ((c != NULL) && (c->getOutpostBuilding() != NULL))
						stopAction = ( sqrDistance > (MaxTalkingOutpostBuildingDistSquare * 1000.0 * 1000.0 ) );
					else
						stopAction = ( sqrDistance > (MaxTalkingDistSquare * 1000.0 * 1000.0 ) );

					break;
				}
			}
			// else go to default (no break)
		default:
			{
				// other static actions (quartering, botchat etc...) have a spatial tolerance (TEMP : use MaxHarvestDistance as tolerance)
				double sqrDistance = CVector2d( _EntityState.X() - _OldPosX, _EntityState.Y() - _OldPosY ).sqrnorm();
				// get current position
				if ( _StaticActionType == STATIC_ACT_TYPES::Forage )
				{
					stopAction = ( sqrDistance > 200*200 ); // 0.2 m for extraction and prospection (currently hardcoded)
					if ( stopAction )
					{
						// If first run of extraction cycle, do not stop to prevent a bug after auto-move by the client
						const CEntityPhrases *entityPhrases = CPhraseManager::getInstance().getEntityPhrases( _EntityRowId );
						if ( entityPhrases )
						{
							const CSPhrasePtr currentAction = entityPhrases->getCurrentActionConst();
							if ( (currentAction != NULL) && (currentAction->mustOverrideCancelStaticAction()) )
								stopAction = false;
						}
					}
				}
				else
					stopAction = ( sqrDistance > ( MaxHarvestDistance * MaxHarvestDistance * 1000.0 * 1000.0 ) );
			}
			break;
		}

		if (stopAction)
		{
			// cancel action
			cancelStaticActionInProgress();
		}
		else
		{
			if( _HarvestDeposit )
			{
				//processDepositHarvestResult();
			}
			else
			{
				CEntityBase* entity = CEntityBaseManager::getEntityBasePtr( _CurrentInterlocutor );
				if (entity != NULL)
				{
					if (isDead())
					{
						cancelStaticActionInProgress();
					}
				}
			}
		}
	}
	// No static action in progress
	else
	{
		_OldPosX = _EntityState.X();
		_OldPosY = _EntityState.Y();
	}
} // processStaticAction //


//---------------------------------------------------
// update compass coordinates information
//---------------------------------------------------
void CCharacter::compassDatabaseUpdate()
{
	static const TDataSetRow invalidRow;
	static sint32 targetX = 0;
	static sint32 targetY = 0;
	if (TheDataset.isAccessible( _CompassTarget ) )
	{
		CEntityBase * e = CEntityBaseManager::getEntityBasePtr( _CompassTarget );
		if( e == NULL )
		{
			_CompassTarget = invalidRow;
			e = CEntityBaseManager::getEntityBasePtr( _Target );
		}
		if( e )
		{
			if( targetX != e->getState().X || targetY != e->getState().Y )
			{
				targetX = e->getState().X;
				targetY = e->getState().Y;
				uint64 pos = ( ( (uint64)targetX ) << 32 ) + targetY;
//				_PropertyDatabase.setProp( "COMPASS:TARGET", pos );
				CBankAccessor_PLR::getCOMPASS().setTARGET(_PropertyDatabase, pos );
			}
		}
	}
}


//---------------------------------------------------
// serial: reading off-mirror, writing from mirror
//
//---------------------------------------------------
void CCharacter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	nlerror("Serial method no longer exists!");

} // serial //


//---------------------------------------------------
// set player position to default respawn point of continent
//
//---------------------------------------------------
void CCharacter::setPositionToDefaultRespawnPoint()
{
	CContinent * continent = CZoneManager::getInstance().getContinent( getX(), getY() );
	uint16 zoneId = (uint16)~0;
	if( continent == 0 )
	{
		nlwarning("<CHAR_TP_TOWN: Character %s : continent null for pos %d %d", getId().toString().c_str(), getX(), getY());
		return;
	}

	switch( (CONTINENT::TContinent)continent->getId() )
	{
	case CONTINENT::FYROS:
		zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_kami_place_pyr" );
		break;
	case CONTINENT::MATIS:
		zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_karavan_place_yrkanis" );
		break;
	case CONTINENT::TRYKER:
		zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_karavan_place_fairhaven" );
		break;
	case CONTINENT::ZORAI:
		zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_kami_place_zora" );
		break;
	case CONTINENT::FYROS_NEWBIE:
	case CONTINENT::MATIS_NEWBIE:
	case CONTINENT::TRYKER_NEWBIE:
	case CONTINENT::ZORAI_NEWBIE:
		egs_chinfo("<CHAR_TP_TOWN> Character %s pos from %d %d is newbieland, do nothing", getId().toString().c_str(), getX(), getY());
		return;
	default:
		switch( getRace() )
		{
		case EGSPD::CPeople::Fyros:
			zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_kami_place_pyr" );
			break;
		case EGSPD::CPeople::Matis:
			zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_karavan_place_yrkanis" );
			break;
		case EGSPD::CPeople::Tryker:
			zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_karavan_place_fairhaven" );
			break;
		case EGSPD::CPeople::Zorai:
			zoneId = CZoneManager::getInstance().getTpSpawnZoneIdByName( "spawn_kami_place_zora" );
			break;
		default:
			nlwarning("<CHAR_TP_TOWN> Character %s %u have no known race %s", getId().toString().c_str(), continent->getId(), EGSPD::CPeople::toString( getRace() ).c_str() );
			return;
		}
		break;
	}

	if( zoneId != (uint16)~0 )
	{
		// get the tp coords
		const CTpSpawnZone* zone = CZoneManager::getInstance().getTpSpawnZone( zoneId );
		if ( !zone )
		{
			nlwarning("<CHAR_TP_TOWN: Character %s %u race %s : place not contain teleport spawn zone!", getId().toString().c_str(), continent->getId(), EGSPD::CPeople::toString( getRace() ).c_str() );
			return;
		}
		COfflineEntityState position;
		zone->getRandomPoint( position.X, position.Y, position.Z, position.Heading );
		egs_chinfo("<CHAR_TP_TOWN> Character %s pos from %d %d go to %d %d", getId().toString().c_str(), getX(), getY(), position.X, position.Y);
		setState( position );
	}
	else
	{
		nlwarning("<CHAR_TP_TOWN: Character %s %u race %s : can't found town default tp place!", getId().toString().c_str(), continent->getId(), EGSPD::CPeople::toString( getRace() ).c_str() );
	}
}

//---------------------------------------------------
// destructor :
//
//---------------------------------------------------
CCharacter::~CCharacter()
{
	TLogNoContext_Item noLog;
	// remove CSR special properties
	if ( getEnterFlag() )
	{
		CCharacter * user = PlayerManager.getChar( _MonitoringCSR );
		if ( user )
			user->setMonitoringCSR( TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW ) );
		user = PlayerManager.getChar( _AdminProperties->getMissionMonitoredUser() );
		if ( user )
			user->getAdminProperties().setMissionMonitoredUser( TDataSetRow::createFromRawIndex( INVALID_DATASET_ROW ) );
		for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
		{
			CMissionManager::getInstance()->deInstanciateMission( (*it).second );
			/// do not delete the mission : it is pd managed
		}
		PlayerManager.removeSummonedUser( _EntityRowId );
		if ( _BotGift )
			delete _BotGift;

		//	egs_chinfo("<CCharacter::~CCharacter> destructing char %s", _Id.toString().c_str() );
		// Cancel all static action still occurs
		cancelStaticActionInProgress();


		if ( _ExchangeAsker != CEntityId::Unknown )
		{
			CActionDistanceChecker::getInstance()->removePlayer( getEntityRowId() );
		}

		if (_TeamId != CTEAM::InvalidTeamId)
		{
			TeamManager.removeCharacter( _Id );
		}

		if( _LootContainer!=NULL )
		{
			pickUpItemClose();
		}

		// Erase potential forage progress
		if ( _ForageProgress )
			delete _ForageProgress;

		// Do this before we clear the _PlayerPets vector
		for( uint i = 0; i < MAX_INVENTORY_ANIMAL; ++i )
		{
			if( i < _PlayerPets.size() )
			{
				if( TheDataset.isAccessible(_PlayerPets[ i ].SpawnedPets) )
				{
					sendPetCommand( CPetCommandMsg::DESPAWN, i, true );
					_PlayerPets[ i ].SpawnedPets = TDataSetRow();
				}
			}
		}

		// remove recipient for character
		DbGroupGlobal.removeRecipient( _Id );
	}

	// Clear _PlayersPets before releasing inventory because CPetAnimal owns an item reference (ItemPtr)
	_PlayerPets.clear();

	// Release inventories
	releaseInventories();

	// if player was harvesting, cancel it
	if ( _MpSourceId != CEntityId::Unknown )
	{
		CCreature *creature = CreatureManager.getCreature( _MpSourceId );
		if (creature != NULL)
		{
			//creature->harvester( NULL );
			creature->resetHarvesterRowId();
		}
	}

	// if character mount a creature
	if( TheDataset.isAccessible(_EntityMounted()) )
	{
		unmount( false );
	}

	if ( getEnterFlag() ) // avoid players not entered yet
	{
		// remove entity from PhraseManager
		CPhraseManager::getInstance().removeEntity(_EntityRowId, true);
	}

	// free structure eventualy allocated for progression
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->clearAllXpForPlayer( _EntityRowId, _TeamId, false );
	PROGRESSIONPVP::CCharacterProgressionPVP::getInstance()->removePlayer(this);

	CRegion * region = dynamic_cast<CRegion*>( CZoneManager::getInstance().getPlaceFromId( _CurrentRegion ) );
	if ( region )
	{
		region->removePlayer(_Id);
	}

	// reset timer
	_TickUpdateTimer.reset();
	_DbUpdateTimer.reset();
	_DeathPenaltyTimer.reset();
	_BarUpdateTimer.reset();

	delete _EncycloChar;
	delete _GameEvent;
	delete _RespawnPoints;
	delete _PlayerRoom;
	delete _ItemsInShopStore;
	delete _PVPInterface;
	delete _AdminProperties;
	delete _DeathPenalties;
	delete _GearLatency;

	EGSPD::CMissionContainerPD::unload(_Id);

	EGSPD::CFameContainerPD::unload(_Id);

//	NLMEMORY::StatisticsReport( "egs_memory_report.csv", false );
} // destructor //


//---------------------------------------------------
// prepareToLoad: method called before applying a pdr save record
//
//---------------------------------------------------
void CCharacter::prepareToLoad()
{
	initPDStructs();
}


//---------------------------------------------------
// postLoadTreatment: method called after applying a pdr save record for all generic processing work
//
//---------------------------------------------------
void CCharacter::postLoadTreatment()
{
	H_AUTO(CCharacterPostLoadTreatment);

	// Check if the name is correct, otherwise set name to default (in case of corrupted name)
	uint8 charIndex = PlayerManager.getCharIndex( getId() );
	uint32 userId = PlayerManager.getPlayerId( getId() );

	// enter the name in the eid translator
	BOMB_IF(PlayerManager.getPlayer(userId) == NULL, "postLoadTreatment : can't find CPlayer instance for user "<<userId, return);
	if (!CEntityIdTranslator::getInstance()->isEntityRegistered(getId()))
	{
		std::string fullName = CShardNames::getInstance().makeFullNameFromRelative(getHomeMainlandSessionId(), getName().toUtf8());
		CEntityIdTranslator::getInstance()->registerEntity( getId(), capitalize( fullName ), charIndex, userId, PlayerManager.getPlayer(userId)->getUserName(), IService::getInstance()->getShardId() );
		ICharNameMapperClient::getInstance()->mapCharacterName(getId(), fullName);
	}

	// force the char into a guild, according to guild member list
	TGuildId gid = CGuildManager::getInstance()->getCharGuildAssoc(_Id);

	if (gid != _GuildId)
	{
		nlwarning("Player %s think he is in guild %u, he really is in guild %u, changing",
			_Id.toString().c_str(),
			_GuildId,
			guildIdToString(gid).c_str());

		_GuildId = gid;
	}


	/* make sure the player is in the guild that he thinks he's in */
	if ( _GuildId && !CGuildManager::getInstance()->checkGuildMemberShip( _Id,_GuildId ) )
	{
		nlwarning("<GUILD>%s is not in its guild %u!!!", _Id.toString().c_str(), _GuildId );
		_GuildId = 0;
	}

	_NbSurvivePact = (uint8)_Pact.size();

	{
	H_AUTO(FixPetTicket);
	/* fix the pets to their tickets */
	vector< bool > tickets;
	tickets.resize( INVENTORIES::NbPackerSlots, false );
	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		if( _PlayerPets[ i ].PetStatus != CPetAnimal::not_present )
		{
			if( _PlayerPets[ i ].Slot >= 0 && _PlayerPets[ i ].Slot < INVENTORIES::NbPackerSlots )
			{
				if( tickets[ _PlayerPets[ i ].Slot ] )
				{
					_PlayerPets[ i ].Slot = INVENTORIES::INVALID_INVENTORY_SLOT;
				}
				else
				{
					tickets[ _PlayerPets[ i ].Slot ] = true;
				}
			}
			else
			{
				_PlayerPets[ i ].Slot = INVENTORIES::INVALID_INVENTORY_SLOT;
			}

			sendPetCustomNameToClient(i);
			uint32 slot = _PlayerPets[ i ].initLinkAnimalToTicket( this, i );
			if( slot < INVENTORIES::NbPackerSlots )
			{
				tickets[ slot ] =  true;

				initPetInventory(i);
			}
		}
	}
	}

	{
	H_AUTO(PetPostLoadTrait);
	/* pet following will be done when receiving Pet Spawn acknowledge (AnimalSpawned) */
	CVector2f playerPos( (float)_EntityState.X, (float)_EntityState.Y );
	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		CPetAnimal& animal = _PlayerPets[i];
		if( animal.PetStatus != CPetAnimal::not_present )
		{
			// Disable following if the distance is too far (e.g. AIS shutdown before EGS received the actual pos and EGS saved an old pos)
			CVector2f petPos( (float)animal.Landscape_X, (float)animal.Landscape_Y );
			float sqDistance = (float)(playerPos - petPos).sqrnorm();
			if ( sqDistance > (float)MaxAnimalCommandDistSquare * (4.0f * 1000.0f * 1000.0f) ) // allow twice the MaxAnimalCommandDistSquare
			{
				animal.IsFollowing = false;
				animal.IsMounted = false;
			}

			// Init satiety if value not in the saved file
			if ( animal.Satiety == SatietyNotInit )
			{
				const CStaticItem* ticketPetForm = CSheets::getForm( animal.TicketPetSheetId );
				animal.MaxSatiety = ticketPetForm ? ticketPetForm->PetHungerCount : 0;\
				animal.Satiety = animal.MaxSatiety;
			}
		}
	}
	}

	{
		H_AUTO(LockTicketInInventory);
		lockTicketInInventory();
	}

	// if EId translator has been initialized by SU, check contact list
	{
		H_AUTO(ValidateContactList);
		validateContactList();
	}

	{
		H_AUTO(UpdateFlagForActiveEffect);
		/* update flags for active effects */
		if( _ForbidAuraUseEndDate>0 && _ForbidAuraUseStartDate==0 )
		{
			// thus timer won't look like infinte on client(can happen due to old save file where startDate didn't exist)
			_ForbidAuraUseStartDate = CTickEventHandler::getGameCycle();
		}
		setPowerFlagDates();
		setAuraFlagDates();
		updateBrickFlagsDBEntry();
		_ModifiersInDB.writeInDatabase(_PropertyDatabase);
	}

	{
		H_AUTO(AddCreationBricks);
		/* add starting bricks if any modification has been done */
		addCreationBricks();
	}

	{
		H_AUTO(CheckCharacterAndScoresValues);
		/* check character and scores are as they are supposed to be according to bricks possessed */
		checkCharacAndScoresValues();
	}

	{
		H_AUTO(ComputeBestSkill);
		/* compute player best skill */
		computeBestSkill();
	}

	{
		H_AUTO(ComputeSkillUsedForDodge);
		/* compute player best skill to use for dodge */
		computeSkillUsedForDodge();
	}

	{
		H_AUTO(UpdateMagicProtectionAndResistance);
		/* compute resists scores*/
		updateMagicProtectionAndResistance();
	}

	{
		H_AUTO(LoadedMissionPostLoad);
		/* Call the postLoad methods for the loaded missions */
		for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
		{
			BOMB_IF( (*it).second == NULL, "Mission is NULL after load", continue );
			(*it).second->onLoad();
		}
	}

	/* setup the implicit visual property fields */
	SET_STRUCT_MEMBER(_VisualPropertyA,PropertySubData.Sex,getGender());

	{
		H_AUTO(ComputeForageBonus);
		/* compute the forage bonuses */
		computeForageBonus();
	}

	{
		H_AUTO(ComputeMiscBonus);
		/* compute misc bonuses */
		computeMiscBonus();
	}

	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
	if (!p->isTrialPlayer())
	{
		CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setLevel(_PropertyDatabase, 250);
		CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setCount(_PropertyDatabase, 999);
	}

	{
		H_AUTO(CItemServiceManager);
		CItemServiceManager::getInstance()->initPersistentServices(this);
	}

	{
		H_AUTO(CheckPvPTagValidity);

		_HaveToUpdatePVPMode = true;
	}

}

//---------------------------------------------------
// lock all tickets if IsRingShard is true
//
//---------------------------------------------------
void CCharacter::lockTicketInInventory()
{
	if(IsRingShard)
	{
		CInventoryPtr inv = getInventory(INVENTORIES::bag);
		if(inv)
		{
			for(uint i = 0; i < INVENTORIES::NbBagSlots; ++i)
			{
				// check if source slot is not empty
				CGameItemPtr item = inv->getItem(i);
				if (item != NULL && item->getStaticForm() != NULL )
				{
					if(item->getStaticForm()->Family == ITEMFAMILY::PET_ANIMAL_TICKET
					|| item->getStaticForm()->Family == ITEMFAMILY::TELEPORT)
					{
						item->setLockCount(item->getStackSize());
					}
				}
			}
		}
	}
}

void CCharacter::validateContactList()
{
	if (IShardUnifierEvent::getInstance() == NULL)
		return;

	// if EId translator has been initialized by SU, check contact list
	if (IShardUnifierEvent::getInstance()->isEidTranslatorInitilazed())
	{
		CEntityIdTranslator *eidt = CEntityIdTranslator::getInstance();
		// any friend must be in the Eid translator
		for (uint i=0; i<_FriendsList.size(); ++i)
		{
			if (!eidt->isEntityRegistered(_FriendsList[i].EntityId))
			{
				_FriendsList.erase(_FriendsList.begin()+i);
				--i;
			}
		}
		for (uint i=0; i<_IgnoreList.size(); ++i)
		{
			if (!eidt->isEntityRegistered(_IgnoreList[i].EntityId))
			{
				_IgnoreList.erase(_IgnoreList.begin()+i);
				--i;
			}
		}

		// any entity that referenced this must exist in EID translator
		for (uint i=0; i<_IsFriendOf.size(); ++i)
		{
			if (!eidt->isEntityRegistered(_IsFriendOf[i]))
			{
				_IsFriendOf.erase(_IsFriendOf.begin()+i);
				--i;
			}
		}
		for (uint i=0; i<_IsIgnoredBy.size(); ++i)
		{
			if (!eidt->isEntityRegistered(_IsIgnoredBy[i]))
			{
				_IsIgnoredBy.erase(_IsIgnoredBy.begin()+i);
				--i;
			}
		}
	}
}

//---------------------------------------------------
// computeBestSkill: compute the best skill and set _BestSkill
//
//---------------------------------------------------
void CCharacter::computeBestSkill()
{
	sint32 bestBase = 0;
	for (int i = 0; i < SKILLS::NUM_SKILLS; i++)
	{
		const sint32 base = _Skills._Skills[i].Base;
		if (base > bestBase)
		{
			bestBase = base;
			_BestSkill = (SKILLS::ESkills) i;
		}
	}
}

//---------------------------------------------------
// computeSkillUsedForDodge: compute the best skill to use a dodge skill
//
//---------------------------------------------------
void CCharacter::computeSkillUsedForDodge()
{
	for (int i = 0; i < SKILLS::NUM_SKILLS; i++)
	{
		const sint32 val = getSkillEquivalentDodgeValue( SKILLS::ESkills(i) );
		if (val > _BaseDodgeLevel)
		{
			_SkillUsedForDodge = (SKILLS::ESkills) i;
			_BaseDodgeLevel = val;
			_CurrentDodgeLevel = max(sint32(0), val + _DodgeModifier);
		}
	}
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeBase, _BaseDodgeLevel);
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseDodgeLevel));
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeCurrent, _CurrentDodgeLevel);
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentDodgeLevel));
}


//---------------------------------------------------
// computeForageBonus
//---------------------------------------------------
void CCharacter::computeForageBonus()
{
	// Browse the known bricks
	CBrickProperties forageBonusBricks;
	getPropertiesFromKnownBricks( BRICK_FAMILIES::BPBHFEA, forageBonusBricks );

	// Calculate the sum of the bonuses for extraction, currently only extraction time bonus
	float totalAdditionalTimeGC = 0;
	const CBrickPropertyValues& extractionTimeBonuses = forageBonusBricks[TBrickParam::BONUS_FG_EXTRACTION_TIME_GC];
	for ( CBrickPropertyValues::const_iterator ibv=extractionTimeBonuses.begin(), endOfMap=extractionTimeBonuses.end(); ibv!=endOfMap; ++ibv )
	{
		CSBrickParamBonusFgExtractionTimeGC *param = (CSBrickParamBonusFgExtractionTimeGC*)(TBrickParam::IId*)(*ibv);
		totalAdditionalTimeGC += param->AdditionalTimeGC;
	}
	_ForageBonusExtractionTime = (NLMISC::TGameCycle)totalAdditionalTimeGC;
}


//---------------------------------------------------
// process a new received bonus brick
//---------------------------------------------------
void CCharacter::processForageBonusBrick( const CStaticBrick *brick )
{
	if ( !brick )
		return;

	for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick->Params.begin(); ip!=brick->Params.end(); ++ip )
	{
		CSBrickParamBonusFgExtractionTimeGC *param = (CSBrickParamBonusFgExtractionTimeGC*)(TBrickParam::IId*)(*ip);

		_ForageBonusExtractionTime += (NLMISC::TGameCycle)param->AdditionalTimeGC;
	}
}


//---------------------------------------------------
// unprocess a new received bonus brick
//---------------------------------------------------
void CCharacter::unprocessForageBonusBrick( const CStaticBrick *brick )
{
	if ( !brick )
		return;

	for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick->Params.begin(); ip!=brick->Params.end(); ++ip )
	{
		CSBrickParamBonusFgExtractionTimeGC *param = (CSBrickParamBonusFgExtractionTimeGC*)(TBrickParam::IId*)(*ip);

		_ForageBonusExtractionTime -= (NLMISC::TGameCycle)param->AdditionalTimeGC;
	}
}


//---------------------------------------------------
// computeMiscBonus
//---------------------------------------------------
void CCharacter::computeMiscBonus()
{
	// Browse the known bricks
	CBrickProperties landmarkBonusBricks;
	getPropertiesFromKnownBricks( BRICK_FAMILIES::BPBGLA, landmarkBonusBricks );

	// Calculate the sum of the bonuses for landmark number
	float sumOfBonusLandmarkNumber = 0;
	const CBrickPropertyValues& landmarkBonuses = landmarkBonusBricks[TBrickParam::BONUS_LANDMARK_NUMBER];
	for ( CBrickPropertyValues::const_iterator ibv=landmarkBonuses.begin(), endOfMap=landmarkBonuses.end(); ibv!=endOfMap; ++ibv )
	{
		CSBrickParamBonusLandmarkNumber *param = (CSBrickParamBonusLandmarkNumber*)(TBrickParam::IId*)(*ibv);
		sumOfBonusLandmarkNumber += param->Nb;
	}

	// Set in db
//	_PropertyDatabase.setProp( "INTERFACES:NB_BONUS_LANDMARKS", (sint64)(sint)sumOfBonusLandmarkNumber );
	CBankAccessor_PLR::getINTERFACES().setNB_BONUS_LANDMARKS(_PropertyDatabase, checkedCast<uint16>(sumOfBonusLandmarkNumber) );
}


//---------------------------------------------------
// process a new received bonus brick
//---------------------------------------------------
void CCharacter::processMiscBonusBrick( const CStaticBrick *brick )
{
	if ( !brick )
		return;

	BOMB_IF( (brick->Family != BRICK_FAMILIES::BPBGLA), "Invalid bonus brick", return ); // currently, only 1 misc bonus brick

	float sumOfBonusLandmarkNumber = 0;
	for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick->Params.begin(); ip!=brick->Params.end(); ++ip )
	{
		CSBrickParamBonusLandmarkNumber *param = (CSBrickParamBonusLandmarkNumber*)(TBrickParam::IId*)(*ip);

		sumOfBonusLandmarkNumber += (NLMISC::TGameCycle)param->Nb;
	}

//	ICDBStructNode *node = _PropertyDatabase.getICDBStructNodeFromName( "INTERFACES:NB_BONUS_LANDMARKS" );
//	BOMB_IF( !node, "Node NB_BONUS_LANDMARKS not found", return );
//	_PropertyDatabase.setProp( node, _PropertyDatabase.getProp( node ) + (sint64)(sint)sumOfBonusLandmarkNumber );
	CBankAccessor_PLR::getINTERFACES().setNB_BONUS_LANDMARKS(_PropertyDatabase, checkedCast<uint16>(CBankAccessor_PLR::getINTERFACES().getNB_BONUS_LANDMARKS(_PropertyDatabase) + sumOfBonusLandmarkNumber) );
}


//---------------------------------------------------
// unprocess a new received bonus brick
//---------------------------------------------------
void CCharacter::unprocessMiscBonusBrick( const CStaticBrick *brick )
{
	if ( !brick )
		return;

	BOMB_IF( (brick->Family != BRICK_FAMILIES::BPBGLA), "Invalid bonus brick", return ); // currently, only 1 misc bonus brick

	float sumOfBonusLandmarkNumber = 0;
	for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick->Params.begin(); ip!=brick->Params.end(); ++ip )
	{
		CSBrickParamBonusLandmarkNumber *param = (CSBrickParamBonusLandmarkNumber*)(TBrickParam::IId*)(*ip);

		sumOfBonusLandmarkNumber += (NLMISC::TGameCycle)param->Nb;
	}

//	ICDBStructNode *node = _PropertyDatabase.getICDBStructNodeFromName( "INTERFACES:NB_BONUS_LANDMARKS" );
//	BOMB_IF( !node, "Node NB_BONUS_LANDMARKS not found", return );
//	_PropertyDatabase.setProp( node, _PropertyDatabase.getProp( node ) + (sint64)(sint)sumOfBonusLandmarkNumber );
	CBankAccessor_PLR::getINTERFACES().setNB_BONUS_LANDMARKS(_PropertyDatabase, checkedCast<uint16>(CBankAccessor_PLR::getINTERFACES().getNB_BONUS_LANDMARKS(_PropertyDatabase) - sumOfBonusLandmarkNumber) );
}


//---------------------------------------------------
//   :
//---------------------------------------------------
void CCharacter::setTarget( const CEntityId &targetId, bool sendMessage )
{
	// If target is the same, do not send message
	if ( _Target() == TheDataset.getDataSetRow( targetId ) && !IsRingShard )
		return;

	CR2GiveItem::getInstance().onUntarget( this, _Target() );

	removeTargetingChar( _Target() );

	CEntityBase * target = CEntityBaseManager::getEntityBasePtr( targetId );
	if( target )
	{
		if( !R2_VISION::isEntityVisibleToPlayers(target->getWhoSeesMe()) )
			return;
	}

	//check entity exists and is targetable
	if (targetId != CEntityId::Unknown)
	{
		// get data set row
		TDataSetRow rowId = TheDataset.getDataSetRow(targetId);
		if ( TheDataset.isAccessible( rowId ) )
		{
			addTargetingChar(rowId);
			// get contextual properties and check targetable
			const CMirrorPropValue<TYPE_CONTEXTUAL> contextualProperties(TheDataset, rowId, DSPropertyCONTEXTUAL );
			const CProperties prop(contextualProperties.getValue());
			if (!prop.selectable())
			{
				return;
			}
		}
	}

	//if targeting an entity which is already targeted, set the mirror value to "invalid" before, so that
	// the onTarget callback is sent.
	// On a RingShard,We want that a targeted entity can be targeted again without selecting another entity.
	if (IsRingShard)
	{
		CEntityId invalidTarget = CEntityId::Unknown;
		CEntityBase::setTarget( invalidTarget );
	}

	CEntityBase::setTarget( targetId );

	// Forage source are not handled by CEntityBaseManager
	if ( targetId.getType() == RYZOMID::forageSource )
	{
		CEntityBase::setTarget( targetId );
		// TODO: mission event?
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.UID, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
		CBankAccessor_PLR::getTARGET().getBARS().setUID(_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.SAP, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setSAP(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.STA, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setSTA(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.FOCUS, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setFOCUS(_PropertyDatabase, 0 );

		//_PropertyDatabase.setProp( "TARGET:AGGRESSIVE", 0 );
//		_PropertyDatabase.setProp( "TARGET:FORCE_RATIO", 0 );
		CBankAccessor_PLR::getTARGET().setFORCE_RATIO(_PropertyDatabase, 0 );
		return;
	}

	//uint agressiveness = 0;
	uint rangeLevel = 0;

	// reset combat event flags
	resetCombatEventFlags();

	// Get target, his Hp, and set TARGET HP in the database
	target = CEntityBaseManager::getEntityBasePtr( _Target() );
	if( target )
	{
		CCreature * creature = dynamic_cast< CCreature *>(target);
		if( creature )
		{
			const CStaticCreatures * form = creature->getForm();
			if( form )
			{
				if (form->getLevel() == 0)
					rangeLevel = 0;
				else
				{
					rangeLevel = ( ((form->getLevel() - 1) / 5) << 1) + ( ((form->getLevel()-1) % 5) >= 2 ? 2 : 1 );

					if (rangeLevel > 11)
						rangeLevel = 11;
				}
			}
		}

		if ( sendMessage )
		{
			if (targetId == _Id)
			{
				PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "TARGET_SELF");
			}
			else if( targetId != CEntityId::Unknown )
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
				params[0].setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId) );
				PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "TARGET_NEW", params);
			}
		}

		// Process mission event "target" until all steps "target" of all missions have been processed
		CMissionEventTarget event( target->getEntityRowId() );
		processMissionMultipleEvent( event );

		// set botchat programm and enable filter is needed
		setTargetBotchatProgramm( target, targetId );

		// UID
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.UID, target->getEntityRowId().getCompressedIndex() );
		CBankAccessor_PLR::getTARGET().getBARS().setUID(_PropertyDatabase, target->getEntityRowId().getCompressedIndex() );

		sint8 percent;
		// Hp
		if( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max == 0 )
		{
			percent = 0;
		}
		else
		{
			percent = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
		}
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, percent );
		CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, percent );

		// Sap
		if( target->getPhysScores()._PhysicalScores[ SCORES::sap ].Max == 0 )
		{
			percent = 0;
		}
		else
		{
			sint8 percentTmp = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::sap ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::sap ].Max ) );
			if( percentTmp < 0 )
				percent = 0;
			else
				percent = percentTmp;
		}
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.SAP, percent );
		CBankAccessor_PLR::getTARGET().getBARS().setSAP(_PropertyDatabase, percent );

		// Stamina
		if( target->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max == 0 )
		{
			percent = 0;
		}
		else
		{
			sint8 percentTmp = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::stamina ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::stamina ].Max ) );
			if( percentTmp < 0 )
				percent = 0;
			else
				percent = percentTmp;
		}
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.STA, percent );
		CBankAccessor_PLR::getTARGET().getBARS().setSTA(_PropertyDatabase, percent );

		// Focus
		if( target->getPhysScores()._PhysicalScores[ SCORES::focus].Max == 0 )
		{
			percent = 0;
		}
		else
		{
			sint8 percentTmp = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::focus ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::focus ].Max ) );
			if( percentTmp < 0 )
				percent = 0;
			else
				percent = percentTmp;
		}
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.FOCUS, percent );
		CBankAccessor_PLR::getTARGET().getBARS().setFOCUS(_PropertyDatabase, percent );

		// Validate properties of target
		CProperties prop;
		// set all flags to true as this bifield is used as a AND mask on the client side
		prop.setAllFlags();
		// set the mountable property
		if( target->getContextualProperty().directAccessForStructMembers().mountable() )
		{
			if( (getPlayerPet( target->getEntityRowId() ) == -1) || TheDataset.getEntityId( _EntityMounted() ) == target->getId() )
			{
				prop.mountable( false );
			}
		}
		if ( target->getId().getType() == RYZOMID::player )
		{
			CCharacter * c = dynamic_cast<CCharacter *>(target);
			if (c)
			{
				// Set the invitable property
				if ( ! TeamManager.isInvitableBy(c, this) )
					prop.invitable( false );
				// if any of the character is god, don't allow to team
				if (c->godMode() || godMode())
					prop.invitable( false );

				// Set the level in database for ForceRegion/ForceLevel deduction
				sint32 skillBaseValue = c->getSkillBaseValue( c->getBestSkill() );
//				_PropertyDatabase.setProp( _DataIndexReminder->TARGET.PLAYER_LEVEL, skillBaseValue );
				CBankAccessor_PLR::getTARGET().getBARS().setPLAYER_LEVEL(_PropertyDatabase, checkedCast<uint8>(skillBaseValue) );
			}
			else
			{
				nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", target->getId().toString().c_str());
			}
		}

		if ( CPVPFactionRewardManager::getInstance().isAttackable( this, target ) )
		{
			prop.attackable( true );
		}

//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.CONTEXT_VAL, (uint16) prop );
		CBankAccessor_PLR::getTARGET().setCONTEXT_VAL(_PropertyDatabase, prop );

//*** Removed by Sadge ***
//		// Ask information about target to AI service
//		CreatureNpcInformation.Character.push_back( _EntityRowId );
//		CreatureNpcInformation.Creature.push_back( target->getEntityRowId() );
//*** ***
	}
	else // target == NULL
	{
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.UID, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
		CBankAccessor_PLR::getTARGET().getBARS().setUID(_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.SAP, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setSAP(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.STA, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setSTA(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( _DataIndexReminder->TARGET.FOCUS, 0 );
		CBankAccessor_PLR::getTARGET().getBARS().setFOCUS(_PropertyDatabase, 0 );

//		PHRASE_UTILITIES::sendDynamicSystemMessage( _Id, "TARGET_NONE");
	}
	//_PropertyDatabase.setProp( "TARGET:AGGRESSIVE", agressiveness );
//	_PropertyDatabase.setProp( "TARGET:FORCE_RATIO", rangeLevel );
	CBankAccessor_PLR::getTARGET().setFORCE_RATIO(_PropertyDatabase, rangeLevel );
} // setTarget //


//---------------------------------------------------
// setTargetBotchatProgramm:
//---------------------------------------------------
void CCharacter::setTargetBotchatProgramm( CEntityBase * target, const CEntityId& targetId )
{
	uint32 programm = 0;
	// set bot chat programms and npcs special options
	CCreature * c = NULL;
	if (targetId.getType() == RYZOMID::npc)
	{
		c = dynamic_cast<CCreature *>(target);
		if (c == NULL)
		{
			nlwarning("This dynamic_cast should not return NULL");
		}
	}
	if (c)
	{
		programm = c->getBotChatProgram();
		if( programm & ( (uint32)1 << uint32(BOTCHATTYPE::TradeItemFlag) ) )
		{
			enableAppropriateFiltersForSeller( c );
		}

		// guild special features
		if (_GuildId != 0)
		{
			programm &= ~( (uint32)1 << uint32(BOTCHATTYPE::CreateGuildFlag) );
		}
		if (c->getOutpostBuilding() != NULL)
		{
			bool hasRightsToTradeOutpostBuilding = false;
			// if the target is an outpost building check we have rights to build/construct
			CGuild *pGuild = CGuildManager::getInstance()->getGuildFromId(_GuildId);
			if (pGuild != NULL)
			{
				CGuildMember *pMember = pGuild->getMemberFromEId(_Id);
				if (pMember != NULL)
				{
					if ((pMember->getGrade() == EGSPD::CGuildGrade::Leader) ||
						(pMember->getGrade() == EGSPD::CGuildGrade::HighOfficer))
					{
						// Ok the user is a leader or a high officer
						// check that the outpost belongs to its guild
						const COutpost *pO = c->getOutpostBuilding()->getParent();
						if (pO != NULL)
							if ((pO->isBelongingToAGuild()) &&
								(pO->getOwnerGuild() == _GuildId))
								hasRightsToTradeOutpostBuilding = true;
					}
				}
			}
			if (!hasRightsToTradeOutpostBuilding)
				programm &= ~( (uint32)1 << uint32(BOTCHATTYPE::TradeOutpostBuilding) );
		}
		// solo mission
		uint i  = 0;
		for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
		{
			vector< pair< bool, uint32 > > texts;
			(*it).second->sendContextTexts( _EntityRowId, c->getEntityRowId(),texts );
			for ( uint k = 0; k < texts.size(); k++)
			{
				if(i >= NB_CONTEXT_DYN_TEXTS) break; // no more room in the context menu, don't fill more or it'll assert
//				_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:TITLE",i) , texts[k].second );
				CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setTITLE(_PropertyDatabase, texts[k].second );
//				_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PLAYER_GIFT_NEEDED",i),texts[k].first  );
				CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPLAYER_GIFT_NEEDED(_PropertyDatabase, texts[k].first );
//				_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PRIORITY",i), 3  );
				CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPRIORITY(_PropertyDatabase, 3 );
				i++;
			}
		}

		// group mission
		CTeam * team = TeamManager.getRealTeam(_TeamId);
		if ( team )
		{
			for (uint j = 0 ; j < team->getMissions().size(); j++ )
			{
				vector< pair< bool, uint32 > > texts;
				team->getMissions()[j]->sendContextTexts( _EntityRowId, c->getEntityRowId(),texts );
				for ( uint k = 0; k < texts.size(); k++)
				{
					if(i >= NB_CONTEXT_DYN_TEXTS) break; // no more room in the context menu, don't fill more or it'll assert
//					_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:TITLE",i) , texts[k].second );
					CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setTITLE(_PropertyDatabase, texts[k].second );
//					_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PLAYER_GIFT_NEEDED",i),texts[k].first  );
					CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPLAYER_GIFT_NEEDED(_PropertyDatabase, texts[k].first );
//					_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PRIORITY",i), 3  );
					CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPRIORITY(_PropertyDatabase, 3 );
					i++;
				}
			}
		}

		//send special contextual texts
		if ( !c->getContextTexts().empty() )
		{
			TVectorParamCheck vect;
			STRING_MANAGER::TParam param;

			param.Type = STRING_MANAGER::player;
			param.setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias( _Id) );
			vect.push_back( param );

			param.Type = STRING_MANAGER::bot;
			param.setEIdAIAlias( targetId, CAIAliasTranslator::getInstance()->getAIAlias( targetId) );

			vect.push_back( param );

			for ( uint j = 0; j < c->getContextTexts().size(); j++ )
			{
				if(i >= NB_CONTEXT_DYN_TEXTS) break; // no more room in the context menu, don't fill more or it'll assert
				uint32 text = STRING_MANAGER::sendStringToClient(_EntityRowId, c->getContextTexts()[j].first.c_str(),vect );
//				_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:TITLE",i) , text );
				CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setTITLE(_PropertyDatabase, text);
//				_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PLAYER_GIFT_NEEDED",i) , 0 );
				CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPLAYER_GIFT_NEEDED(_PropertyDatabase, 0 );
//				_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PRIORITY",i), 0  );
				CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPRIORITY(_PropertyDatabase, 2 );
				i++;
			}
		}

		// send auto missions
		for ( uint j = 0; j < c->getMissionVector().size(); j++ )
		{
			const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( c->getMissionVector()[j] );
			if ( (templ != NULL) && !templ->AutoText.empty() )
			{
				if (templ->testPrerequisits(this, false) == MISSION_DESC::PreReqSuccess)
				{
					if(i >= NB_CONTEXT_DYN_TEXTS) break; // no more room in the context menu, don't fill more or it'll assert
					uint32 text = templ->sendAutoText(_EntityRowId,_CurrentInterlocutor);
//					_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:TITLE",i) , text );
					CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setTITLE(_PropertyDatabase, text );
//					_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PLAYER_GIFT_NEEDED",i) , 0 );
					CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPLAYER_GIFT_NEEDED(_PropertyDatabase, 0 );
//					_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PRIORITY",i), 3 );
					CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPRIORITY(_PropertyDatabase, 3 );
					i++;
				}
			}
		}

		for (; i < NB_CONTEXT_DYN_TEXTS; i++ )
		{
//			_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:TITLE",i) , 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setTITLE(_PropertyDatabase, 0 );
//			_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PLAYER_GIFT_NEEDED",i) , 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPLAYER_GIFT_NEEDED(_PropertyDatabase, 0 );
//			_PropertyDatabase.setProp( toString("TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:%u:PRIORITY",i), 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(i).setPRIORITY(_PropertyDatabase, 0 );
		}

		// WebPage Title
		if( programm & (1<<BOTCHATTYPE::WebPageFlag) )
		{
			// send the web page title
			uint32 text;
			if(c->getWebPageName().find("MENU_") == 0)
			{
				text = STRING_MANAGER::sendStringToClient(_EntityRowId, c->getWebPageName(), TVectorParamCheck() );
			}
			else
			{
				SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
				params[0].Literal= c->getWebPageName();
				text = STRING_MANAGER::sendStringToClient(_EntityRowId, "LITERAL", params );
			}
//          _PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:WEB_PAGE_TITLE" , text );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setWEB_PAGE_TITLE(_PropertyDatabase, text );

			// send the web page url
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
			params[0].Literal= c->getWebPage();
			text = STRING_MANAGER::sendStringToClient(_EntityRowId, "LITERAL", params );
//			_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:WEB_PAGE_URL" , text );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setWEB_PAGE_URL(_PropertyDatabase, text );
		}
		else
		{
//			_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:WEB_PAGE_TITLE" , 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setWEB_PAGE_TITLE(_PropertyDatabase, 0 );
//			_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:WEB_PAGE_URL" , 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setWEB_PAGE_URL(_PropertyDatabase, 0 );
		}
		// Outpost
		if( programm & (1<<BOTCHATTYPE::OutpostFlag) )
		{
//			_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:OUTPOST" , c->getBotChatOutpost().asInt() );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setOUTPOST(_PropertyDatabase, c->getBotChatOutpost() );
		}
		else
		{
//			_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:OUTPOST" , 0 );
			CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setOUTPOST(_PropertyDatabase, CSheetId::Unknown );
		}
	}
	else
	{
		if ( target->getId().getType() == RYZOMID::player )
		{
			CCharacter * c = dynamic_cast<CCharacter *>(target);
			if (c)
			{
				CGuildMemberModule * module;
				if ( _ModulesCont->getModule( module ) )
				{
					if ( c->getGuildId() == 0 && module->canInvite() )
						programm |= 1 << BOTCHATTYPE::GuildInviteFlag;
				}
			}
			else
			{
				nlwarning("Entity %s type is player but dynamic_cast in CCharacter * returns NULL ?!", target->getId().toString().c_str());
			}
		}
//		_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:PROGRAMMES", programm );
		CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setPROGRAMMES(_PropertyDatabase, programm );
//		_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:0:TITLE", 0 );
		CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(0).setTITLE(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:0:PLAYER_GIFT_NEEDED", 0 );
		CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(0).setPLAYER_GIFT_NEEDED(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:MISSIONS_OPTIONS:0:PRIORITY", 0 );
		CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().getMISSIONS_OPTIONS().getArray(0).setPRIORITY(_PropertyDatabase, 0 );
	}

	// Can attack another player or a npc/creature helping players?
	bool invulnerable = target->getContextualProperty().directAccessForStructMembers().invulnerable();
	if( CPVPManager2::getInstance()->isOffensiveActionValid( this, target, true ) && !invulnerable )
	{
		programm |= 1 << BOTCHATTYPE::Attackable;
	}
	// otherwise if target is a creature/npc, check fame attackable
	else if( c && c->checkFactionAttackable(_Id) )
	{
		programm |= 1 << BOTCHATTYPE::Attackable;
	}
	else if( isEntityAnOutpostEnemy(targetId) && !invulnerable )
	{
		programm |= 1 << BOTCHATTYPE::Attackable;
	}

	if ( CPVPFactionRewardManager::getInstance().isAttackable( this, target ) )
		programm |= 1 << BOTCHATTYPE::Attackable;

//	_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:PROGRAMMES", programm, true );
	CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setPROGRAMMES(_PropertyDatabase, programm, true );
}

//-----------------------------------------------------------------------------
void CCharacter::enableAppropriateFiltersForSeller( CCreature * c )
{
	nlassert( c != 0 );

	const std::vector< uint32 >& botChatCategory = c->getBotChatCategory();

	bool rawMaterialSeller = false;
	uint64 itemTypeSelled64 = 0;
	uint64 itemTypeSelled128 = 0;
	for( uint32 i = 0; i < botChatCategory.size(); ++i )
	{
		if( botChatCategory[ i ] < CShopTypeManager::getCategoryName().size() )
		{
			if( botChatCategory[ i ] > CShopTypeManager::getRmStart() && botChatCategory[ i ] < CShopTypeManager::getRmEnd() )
			{
				rawMaterialSeller = true;
			}

			if( botChatCategory[ i ] > CShopTypeManager::getItemStart() && botChatCategory[ i ] < CShopTypeManager::getItemEnd() )
			{
				ITEM_TYPE::TItemType itemType = ITEM_TYPE::stringToItemType( CShopTypeManager::getCategoryName()[ botChatCategory[ i ] ] );
				if( itemType != ITEM_TYPE::UNDEFINED )
				{
					if( itemType < ITEM_TYPE::LIMIT_64 )
					{
						itemTypeSelled64 |= ((uint64)1) << itemType;
					}
					else
					{
						itemTypeSelled128 |= ((uint64)1) << (itemType - ITEM_TYPE::LIMIT_64);
					}
				}
			}
		}
	}
//	_PropertyDatabase.setProp( "TRADING:RAW_MATERIAL_SELLER", rawMaterialSeller );
	CBankAccessor_PLR::getTRADING().setRAW_MATERIAL_SELLER(_PropertyDatabase, rawMaterialSeller );
//	_PropertyDatabase.setProp( "TRADING:ITEM_TYPE_SELLER_BITFILED_0_63", itemTypeSelled64 );
	CBankAccessor_PLR::getTRADING().setITEM_TYPE_SELLER_BITFILED_0_63(_PropertyDatabase, itemTypeSelled64);
//	_PropertyDatabase.setProp( "TRADING:ITEM_TYPE_SELLER_BITFILED_64_127", itemTypeSelled128 );
	CBankAccessor_PLR::getTRADING().setITEM_TYPE_SELLER_BITFILED_64_127(_PropertyDatabase, itemTypeSelled128);
}

//-----------------------------------------------------------------------------
void CCharacter::updateTargetingChars()
{
	const uint size = (uint)_TargetingChars.size();
	for ( uint i = 0; i < size; i++ )
	{
		CCharacter* user = PlayerManager.getChar( _TargetingChars[i] );
		if ( user )
		{
			user->updateTarget();
		}
	}
}

CRingRewardPoints &CCharacter::getRingRewardPoints()
{
	return RingRewardPoints;
}


//---------------------------------------------------
// sendBetaTesterStatus :
//---------------------------------------------------
void CCharacter::sendBetaTesterStatus()
{
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
	if (p == NULL)
		return;

	sendReservedTitleStatus( CHARACTER_TITLE::FBT, p->isBetaTester() );

	if (!p->isBetaTester() && _NewTitle == "FBT")
	{
		_NewTitle = "Refugee";
		registerName();
	}
}

//---------------------------------------------------
// send windermeer old community status
//---------------------------------------------------
void CCharacter::sendWindermeerStatus()
{
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
	if (p == NULL)
		return;

	sendReservedTitleStatus( CHARACTER_TITLE::WIND, p->isWindermeerCommunity() );

	if ( !p->isWindermeerCommunity() && _NewTitle == "WIND")
	{
		_NewTitle = "Refugee";
		registerName();
	}
}

//---------------------------------------------------
// sendReservedTitleStatus :
//---------------------------------------------------
void CCharacter::sendReservedTitleStatus(CHARACTER_TITLE::ECharacterTitle title, bool available) const
{
	// send message to client
	CMessage msgout( "IMPULSION_ID" );
	CEntityId id = _Id;
	msgout.serial( id );

	CBitMemStream bms;
	if ( !GenericMsgManager.pushNameToStream("GUILD:UPDATE_PLAYER_TITLE", bms) )
	{
		nlwarning("<CCharacter::sendReservedTitleStatus> Msg name GUILD:UPDATE_PLAYER_TITLE not found");
		return;
	}

	vector<uint16> titles;
	titles.push_back( (uint16)title );

	bms.serial( available );
	bms.serialCont( titles );

	msgout.serialBufferWithSize( (uint8*)bms.buffer(), bms.length() );
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}

/*
 * Constructor
 */
//CCharacter::CCharacterDbReminder::CCharacterDbReminder()
//{
//	CCDBStructNodeBranch *rootNode = CCDBStructBanks::instance()->getStructRoot( CDBPlayer );
//	nlassert( rootNode );
//
//	nlverify( DATABASE_PING.PING = rootNode->getICDBStructNodeFromNameFromRoot( "DEBUG:Ping" ) );
//
//	// PACK_ANIMAL
//	{
//		// BEAST[MAX_INVENTORY_ANIMAL]
//		for (int i=0; i<4; ++i)
//		{
//			nlverify( PACK_ANIMAL.BEAST[i].TYPE		= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:TYPE", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].UID		= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:UID", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].STATUS	= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:STATUS", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].HP		= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:HP", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].BULK_MAX	= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:BULK_MAX", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].POS		= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:POS", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].HUNGER	= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:HUNGER", i ) ) );
//			nlverify( PACK_ANIMAL.BEAST[i].DESPAWN	= rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("PACK_ANIMAL:BEAST%d:DESPAWN", i ) ) );
//		}
//	}
//	// CHARACTER_INFO
//	{
//		// SCORES
//		{
//			for (int i=0; i<SCORES::NUM_SCORES; ++i)
//			{
//				nlverify( CHARACTER_INFO.SCORES.BaseRegen[i] = rootNode->getICDBStructNodeFromNameFromRoot( std::string("CHARACTER_INFO:SCORES")+toString(i)+":BaseRegen" );
//				nlverify( CHARACTER_INFO.SCORES.Regen[i] = rootNode->getICDBStructNodeFromNameFromRoot( std::string("CHARACTER_INFO:SCORES")+toString(i)+":Regen"));
//				nlverify( CHARACTER_INFO.SCORES.BaseScore[i] = rootNode->getICDBStructNodeFromNameFromRoot( std::string("CHARACTER_INFO:SCORES")+toString(i)+":Base" );
//				nlverify( CHARACTER_INFO.SCORES.MaxScore[i] = rootNode->getICDBStructNodeFromNameFromRoot( std::string("CHARACTER_INFO:SCORES")+toString(i)+":Max"));
//			}
//
//			for (int i=0; i<SKILLS::NUM_SKILLS; ++i)
//			{
//				nlverify( CHARACTER_INFO.SKILLS.BaseSkill[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("CHARACTER_INFO:SKILLS:%d:BaseSKILL", i) ) );
//				nlverify( CHARACTER_INFO.SKILLS.Skill[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("CHARACTER_INFO:SKILLS:%d:SKILL", i) ) );
//				nlverify( CHARACTER_INFO.SKILLS.ProgressBar[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("CHARACTER_INFO:SKILLS:%d:PROGRESS_BAR", i) ) );
//			}
//		}
//		// Dodge / Parry modifier
//		nlverify( CHARACTER_INFO.DodgeBase = rootNode->getICDBStructNodeFromNameFromRoot("CHARACTER_INFO:DODGE:Base") );
//		nlverify( CHARACTER_INFO.DodgeCurrent = rootNode->getICDBStructNodeFromNameFromRoot("CHARACTER_INFO:DODGE:Current") );
//		nlverify( CHARACTER_INFO.ParryBase = rootNode->getICDBStructNodeFromNameFromRoot("CHARACTER_INFO:PARRY:Base") );
//		nlverify( CHARACTER_INFO.ParryCurrent = rootNode->getICDBStructNodeFromNameFromRoot("CHARACTER_INFO:PARRY:Current") );
//	}
//	// TARGET
//	{
//		nlverify( TARGET.UID   = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:BARS:UID" ) );
//		nlverify( TARGET.HP    = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:BARS:HP" ) );
//		nlverify( TARGET.SAP   = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:BARS:SAP" ) );
//		nlverify( TARGET.STA   = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:BARS:STA" ) );
//		nlverify( TARGET.FOCUS = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:BARS:FOCUS" ) );
//		nlverify( TARGET.PLAYER_LEVEL = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:BARS:PLAYER_LEVEL" ) );
//		nlverify( TARGET.CONTEXT_VAL = rootNode->getICDBStructNodeFromNameFromRoot( "TARGET:CONTEXT_VAL" ) );
//	}
//	// Known Bricks Families
//	{
//		for (int i=0; i<BRICK_FAMILIES::NbFamilies; ++i)
//		{
//			nlverify( KnownBricksFamilies[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("BRICK_FAMILY:%d:BRICKS", i) ) );
//		}
//	}
//
//	// Modifier
//	{
//		for (int i=0; i<MaxBonusMalusDisplayed; ++i)
//		{
//			nlverify( Modifiers.Bonus.Sheet[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("MODIFIERS:BONUS:%d:SHEET", i) ) );
//			nlverify( Modifiers.Bonus.Disable[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("MODIFIERS:BONUS:%d:DISABLED", i) ) );
//			nlverify( Modifiers.Bonus.DisableTime[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("MODIFIERS:BONUS:%d:DISABLED_TIME", i) ) );
//			nlverify( Modifiers.Malus.Sheet[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("MODIFIERS:MALUS:%d:SHEET", i) ) );
//			nlverify( Modifiers.Malus.Disable[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("MODIFIERS:MALUS:%d:DISABLED", i) ) );
//			nlverify( Modifiers.Malus.DisableTime[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("MODIFIERS:MALUS:%d:DISABLED_TIME", i) ) );
//		}
//		nlverify( Modifiers.TotalMalusEquip = rootNode->getICDBStructNodeFromNameFromRoot( "MODIFIERS:TOTAL_MALUS_EQUIP" ) );
//	}
//
//	// Disable consumable family (overdose)
//	{
//		for(int i=0; i<MaxBonusMalusDisplayed; ++i)
//		{
//			nlverify( DisableConsumable.Family[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("DISABLE_CONSUMABLE:%d:FAMILY", i) ) );
//			nlverify( DisableConsumable.DisableTime[i] = rootNode->getICDBStructNodeFromNameFromRoot( NLMISC::toString("DISABLE_CONSUMABLE:%d:DISABLE_TIME", i) ) );
//		}
//	}
//}


/*
 * Fill version data for handshake. Please update checkHandshake() in client:net_manager.cpp accordingly.
 */
void CCharacter::fillHandshake( CBitMemStream& bms )
{
	uint16 handshakeVersion = 0;
	uint16 itemSlotVersion = INVENTORIES::CItemSlot::getVersion();
	bms.serial( handshakeVersion );
	bms.serial( itemSlotVersion );

	// Here we could fill with the SHA1 hash code of database.xml, msg.xml (but this one could bug because
	// the handshake uses it already)...
}


//---------------------------------------------------
// initDatabase :
//---------------------------------------------------
void CCharacter::initDatabase()
{
	// Load the database, and prepare database outbox
//	_PropertyDatabase.init( CDBPlayer );
	_PropertyDatabase.init( );

	// Target
//	_PropertyDatabase.setProp( _DataIndexReminder->TARGET.UID, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
	CBankAccessor_PLR::getTARGET().getBARS().setUID(_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );


	// set player max bulk
//	_PropertyDatabase.setProp( "STATIC_DATA:BAG_BULK_MAX", MaxPlayerBulk / 1000 );
	CBankAccessor_PLR::getSTATIC_DATA().setBAG_BULK_MAX(_PropertyDatabase, MaxPlayerBulk / 1000 );

	// set player room max bulk
//	_PropertyDatabase.setProp( "INVENTORY:ROOM:BULK_MAX", BasePlayerRoomBulk / 1000 );
	CBankAccessor_PLR::getINVENTORY().getROOM().setBULK_MAX(_PropertyDatabase, BasePlayerRoomBulk / 1000 );

	// GROUP is empty
	for (uint i = 0 ; i < 8 ; ++i)
	{
//		_PropertyDatabase.setProp( NLMISC::toString("GROUP:%d:PRESENT",i), 0 );
		CBankAccessor_PLR::getGROUP().getArray(i).setPRESENT(_PropertyDatabase, false );
//		_PropertyDatabase.setProp( NLMISC::toString("GROUP:%d:NAME",i), 0 );
		CBankAccessor_PLR::getGROUP().getArray(i).setNAME(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp( NLMISC::toString("GROUP:%d:UID",i), CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
		CBankAccessor_PLR::getGROUP().getArray(i).setUID(_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
	}

	// PACK_ANIMAL is empty
	for (uint i = 0 ; i < MAX_INVENTORY_ANIMAL; ++i)
	{
//		_PropertyDatabase.setProp( NLMISC::toString("PACK_ANIMAL:BEAST%d:UID",i), CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setUID(_PropertyDatabase, CLFECOMMON::INVALID_CLIENT_DATASET_INDEX );
	}

//	_PropertyDatabase.setPropButDontSend( "BUILDING_SENTENCE:COUNTER", 0 );

	// modifiers
	for (uint i = 0 ; i < MaxBonusMalusDisplayed ; ++i)
	{
		CBankAccessor_PLR::TMODIFIERS::TMALUS::TArray &malusElem = CBankAccessor_PLR::getMODIFIERS().getMALUS().getArray(i);
		CBankAccessor_PLR::TMODIFIERS::TBONUS::TArray &bonusElem = CBankAccessor_PLR::getMODIFIERS().getBONUS().getArray(i);
//		_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.Malus.Sheet[i], 0);
		malusElem.setSHEET(_PropertyDatabase, CSheetId::Unknown);
//		_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.Malus.Disable[i], 0);
		malusElem.setDISABLED(_PropertyDatabase, false);
//		_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.Malus.DisableTime[i], 0);
		malusElem.setDISABLED_TIME(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.Bonus.Sheet[i], 0);
		bonusElem.setSHEET(_PropertyDatabase, CSheetId::Unknown);
//		_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.Bonus.Disable[i], 0);
		bonusElem.setDISABLED(_PropertyDatabase, false);
//		_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.Bonus.DisableTime[i], 0);
		bonusElem.setDISABLED_TIME(_PropertyDatabase, 0);
	}

	//money
//	_PropertyDatabase.setProp( "INVENTORY:MONEY", _Money );
	CBankAccessor_PLR::getINVENTORY().setMONEY( _PropertyDatabase, _Money );

	//Temporary until managed by AI
//	_PropertyDatabase.setProp( _DataIndexReminder->TARGET.CONTEXT_VAL, 0xffff );
	CBankAccessor_PLR::getTARGET().setCONTEXT_VAL(_PropertyDatabase, 0xffff );

	// interfaces flags
//	_PropertyDatabase.setProp( "INTERFACES:FLAGS", 0);
	CBankAccessor_PLR::getINTERFACES().setFLAGS(_PropertyDatabase, 0);

	// combat flags
	//_ForbidPowerDates.writeUsablePowerFlags(_UsablePowerFlags);
	setPowerFlagDates();
	setAuraFlagDates();
	updateBrickFlagsDBEntry();

	// defense interface
	for (uint i = 0 ; i < 6 ; ++i)
	{
//		_PropertyDatabase.setProp( NLMISC::toString("DEFENSE:SLOTS:%d:MODIFIER",i), 0 );
		CBankAccessor_PLR::getDEFENSE().getSLOTS().getArray(i).setMODIFIER(_PropertyDatabase, 0 );
	}

	// init death malus
//	_PropertyDatabase.setProp( "USER:DEATH_XP_MALUS", 255 );
	CBankAccessor_PLR::getUSER().setDEATH_XP_MALUS(_PropertyDatabase, 255 );

	// dodge and parry levels
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeBase, _BaseDodgeLevel);
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseDodgeLevel));
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeCurrent, _CurrentDodgeLevel);
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentDodgeLevel));
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryBase, _BaseParryLevel);
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseParryLevel));
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel);
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel));

//	_PropertyDatabase.setProp( "USER:SPEED_FACTOR", sint64(100) );
	CBankAccessor_PLR::getUSER().setSPEED_FACTOR(_PropertyDatabase, 100 );

	// consumable modifiers
	parrySuccessModifier(0);
	dodgeSuccessModifier(0);
	craftSuccessModifier(0);
	meleeSuccessModifier(0);
	rangeSuccessModifier(0);
	magicSuccessModifier(0);
	_ForageSuccessModifiers.resize( ECOSYSTEM::NUM_ECOSYSTEM );
	for(uint8 i = 0; i < (uint8)ECOSYSTEM::NUM_ECOSYSTEM; ++i )
		forageSuccessModifier((ECOSYSTEM::EECosystem)i,0);

//	_PropertyDatabase.setProp("USER:DEFAULT_WEIGHT_HANDS", DefaultWeightHands);
	CBankAccessor_PLR::getUSER().setDEFAULT_WEIGHT_HANDS(_PropertyDatabase, DefaultWeightHands);
} // initDatabase //


extern CBitMemStream DBOutput; // global to avoid reallocation

//---------------------------------------------------
// databaseUpdate :
// Send	a delta message to the client for changed properties
//---------------------------------------------------
void CCharacter::databaseUpdate()
{
	// Write the inventory updates
	_InventoryUpdater.sendAllUpdates( _Id ); // must be before the sending of _PropertyDatabase, because it tests _PropertyDatabase.notSentYet()

	// Write the character's database delta (for comment numbers, see tutorial in cdb_group.h)
	if ( _PropertyDatabase.getChangedPropertyCount() != 0 ) // ensures writeDelta() will return true
	{
		DBOutput.resetBufPos();
		bool hasContentToSend = true;
		if ( _PropertyDatabase.notSentYet() )
		{
			// The first message has a different name, because the client must know that it's the
			// first message to inhibit its oberver callbacks, although it is not garanteed that
			// it's the first message to arrive on the client (see impulsion channels on the FS).
			GenericMsgManager.pushNameToStream( "DB_INIT:PLR", DBOutput );
			// write the server tick, to ensure old DB update are not applied after newer
			TGameCycle	serverTick= CTickEventHandler::getGameCycle();
			DBOutput.serial(serverTick);
			// write the delta DB
			_PropertyDatabase.writeDelta( DBOutput, ~0 ); // no size limit for first sending
			//egs_chinfo( "Sending 1st database packet" );
		}
		else
		{
			uint16 databaseImpulseWindowBitSize = _AvailImpulseBitsize.isReadable() ? _AvailImpulseBitsize() : 91*8;
			sint32 characterBankMaxBitSize = ((sint32)databaseImpulseWindowBitSize);
			if ( characterBankMaxBitSize > 0 )
			{
				// Write using bandwith limit
				GenericMsgManager.pushNameToStream( "DB_UPD_PLR", DBOutput );
				// write the server tick, to ensure old DB update are not applied after newer
				TGameCycle	serverTick= CTickEventHandler::getGameCycle();
				DBOutput.serial(serverTick);
				// write the delta DB
				_PropertyDatabase.writeDelta( DBOutput, (uint32)characterBankMaxBitSize );
			}
			else
				hasContentToSend = false;
		}

		// "Client only" property changes
		/*bool hasCOPropChanges = _PropertyDatabase.hasClientonlyPropertyChanges();
		DBOutput.serialBit( hasCOPropChanges );
		if ( hasCOPropChanges )
			_PropertyDatabase.writeClientonlyPropertyChanges( DBOutput );*/

		// Send impulsion to front-end service
		if ( hasContentToSend )
		{
			CMessage msgout( "CDB_IMPULSION" );
			msgout.serial( _Id );
			msgout.serialBufferWithSize( (uint8*)DBOutput.buffer(), DBOutput.length() );
			CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
		}
	}

} // databaseUpdate //


/** Fill the TCharInfo struct used to send info to SU
 */
void CCharacter::fillCharInfo(CHARSYNC::TCharInfo &charInfo) const
{
	charInfo.setCharEId(getId());
	charInfo.setCharName(getName().toUtf8());
	charInfo.setHomeSessionId(getHomeMainlandSessionId());
	charInfo.setBestCombatLevel(max(getBestChildSkillValue(SKILLS::SF), getBestChildSkillValue(SKILLS::SMO)));
	charInfo.setGuildId(getGuildId());
	charInfo.setRace((CHARSYNC::TRace::TValues)getRace());
	std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance = getAllegiance();
	std::pair<CHARSYNC::TCult, CHARSYNC::TCivilisation> charSyncAll = IShardUnifierEvent::convertAllegiance(allegiance);
	charInfo.setCult(charSyncAll.first);
	charInfo.setCivilisation(charSyncAll.second);
	charInfo.setRespawnPoints(getRespawnPoints().buildRingPoints());
	charInfo.setNewcomer(isNewbie());
}

//---------------------------------------------------
// setId :
//---------------------------------------------------
void CCharacter::setId( const CEntityId& id )
{
	_Id = id;

	// set the Team
	//const uint16 teamId = (uint16) _UserId / 10;
	//TeamManager.addCharacterToTeam( this, teamId );
} // setId //


void CCharacter::setName(const ucstring &name)
{
	// update only if different
	if(_Name!=name)
	{
		CEntityBase::setName(name);

		// update the eid translator
		CPlayer * player = PlayerManager.getPlayer(PlayerManager.getPlayerId(getId()));
		if(player)
		{
			CEntityIdTranslator::getInstance()->updateEntity( getId(), capitalize( getName().toUtf8() ), sint8(PlayerManager.getCharIndex(getId())), PlayerManager.getPlayerId(getId()), player->getUserName(), getHomeMainlandSessionId().asInt() );
			ICharNameMapperClient::getInstance()->mapCharacterName(getId(), getName());
		}
	}
}

//-----------------------------------------------
// CCharacter::haveBrick check if player have the brick
//-----------------------------------------------
bool CCharacter::haveBrick( const CSheetId& brickId )
{
	const CStaticBrick* brickForm = CSheets::getSBrickForm( brickId );
	if( brickForm )
	{
		// if brick already known, just return
		if ( _KnownBricks.find( brickId ) != _KnownBricks.end())
			return true;
	}
	return false;
}

//-----------------------------------------------
// CCharacter::addKnownBrick add a know brick
//-----------------------------------------------
void CCharacter::addKnownBrick( const CSheetId& brickId )
{
//	egs_chinfo("<CCharacter::addKnownBrick> adding new known brick idSheet (%s)", brickId.toString().c_str() );
	const CStaticBrick* brickForm = CSheets::getSBrickForm( brickId );
	if( brickForm )
	{
		// if brick already known, just return
		if ( _KnownBricks.find( brickId ) != _KnownBricks.end())
			return;

		log_Character_AddKnownBrick(brickId);
		_KnownBricks.insert( brickId );

		// if the brick is a training brick, then apply charac increase
		if ( BRICK_FAMILIES::brickType(brickForm->Family) == BRICK_TYPE::TRAINING)
		{
			processTrainingBrick(brickForm);
		}

		// if the brick is a bonus that needs to be taken into account now, do it
		switch ( brickForm->Family )
		{
		case BRICK_FAMILIES::BPBHFEA:
			processForageBonusBrick(brickForm);
			break;
		case BRICK_FAMILIES::BPBGLA:
			processMiscBonusBrick(brickForm);
			break;
		default:;
		}

		// update the database
		uint8 pos = (uint8)brickForm->IndexInFamily;
		if ( INVALID_POSITION_ID < pos)
		{
			--pos;
		}
		else if ( INVALID_POSITION_ID == pos)
		{
			nlwarning("brick id %s is invalid (index in family == INVALID_POSITION_ID", brickId.toString().c_str());
			return;
		}

		_BrickFamilyBitField[brickForm->Family] |= ( (sint64)1 << (sint64)pos);
//		_PropertyDatabase.setProp( _DataIndexReminder->KnownBricksFamilies[brickForm->Family], _BrickFamilyBitField[brickForm->Family]);
		CBankAccessor_PLR::getBRICK_FAMILY().getArray(brickForm->Family).setBRICKS(_PropertyDatabase, _BrickFamilyBitField[brickForm->Family]);

		// unlock the relative interface if it's the first brick of this type
		INTERFACE_FLAGS::TInterfaceFlag flag = INTERFACE_FLAGS::Unknown;
		switch( BRICK_FAMILIES::brickType(brickForm->Family))
		{
		case BRICK_TYPE::COMBAT:
			flag = INTERFACE_FLAGS::Combat;
			break;
		case BRICK_TYPE::MAGIC:
			flag = INTERFACE_FLAGS::Magic;
			break;
		case BRICK_TYPE::COMMERCE:
			flag = INTERFACE_FLAGS::Commerce;
			break;
		default:
			flag = INTERFACE_FLAGS::Special;
			break;
		};

		_InterfacesFlagsBitField |= SINT64_CONSTANT(1) << uint8(flag);
//		_PropertyDatabase.setProp( "INTERFACES:FLAGS", _InterfacesFlagsBitField);
		CBankAccessor_PLR::getINTERFACES().setFLAGS(_PropertyDatabase, _InterfacesFlagsBitField);
	}
	else
	{
		nlwarning("<CCharacter::addKnownBrick> Can't add new known brick cause static form of idSheet (%s) missing", brickId.toString().c_str() );
	}
} //addKnownBrick//

//-----------------------------------------------
// CCharacter::removeKnownBrick remove a known brick
//-----------------------------------------------
void CCharacter::removeKnownBrick( const CSheetId& brickId )
{
//	egs_chinfo("<CCharacter::removeKnownBrick> removing a known brick idSheet (%s)", brickId.toString().c_str() );
	const CStaticBrick* brickForm = CSheets::getSBrickForm( brickId );
	if( brickForm )
	{
		// if brick is not known, just return
		if ( _KnownBricks.find( brickId ) == _KnownBricks.end())
			return;

		log_Character_RemoveKnownBrick(brickId);
		_KnownBricks.erase( brickId );

		// if the brick is a training brick, then apply charac increase
		if ( BRICK_FAMILIES::brickType(brickForm->Family) == BRICK_TYPE::TRAINING)
		{
			unprocessTrainingBrick(brickForm, true);
		}

		// if the brick is a bonus that needs to be taken into account now, do it
		switch ( brickForm->Family )
		{
		case BRICK_FAMILIES::BPBHFEA:
			unprocessForageBonusBrick(brickForm);
			break;
		case BRICK_FAMILIES::BPBGLA:
			unprocessMiscBonusBrick(brickForm);
			break;
		default:;
		}

		// update the database
		uint8 pos = (uint8)brickForm->IndexInFamily;
		if ( INVALID_POSITION_ID < pos)
		{
			--pos;
		}
		else if ( INVALID_POSITION_ID == pos)
		{
			nlwarning("brick id %s is invalid (index in family == INVALID_POSITION_ID", brickId.toString().c_str());
			return;
		}

		_BrickFamilyBitField[brickForm->Family] |= ( (sint64)1 << (sint64)pos);
//		_PropertyDatabase.setProp( _DataIndexReminder->KnownBricksFamilies[brickForm->Family], _BrickFamilyBitField[brickForm->Family]);
		CBankAccessor_PLR::getBRICK_FAMILY().getArray(brickForm->Family).setBRICKS(_PropertyDatabase, _BrickFamilyBitField[brickForm->Family]);

		// unlock the relative interface if it's the first brick of this type
		INTERFACE_FLAGS::TInterfaceFlag flag = INTERFACE_FLAGS::Unknown;
		switch( BRICK_FAMILIES::brickType(brickForm->Family))
		{
		case BRICK_TYPE::COMBAT:
			flag = INTERFACE_FLAGS::Combat;
			break;
		case BRICK_TYPE::MAGIC:
			flag = INTERFACE_FLAGS::Magic;
			break;
		case BRICK_TYPE::COMMERCE:
			flag = INTERFACE_FLAGS::Commerce;
			break;
		default:
			flag = INTERFACE_FLAGS::Special;
			break;
		};

		_InterfacesFlagsBitField |= SINT64_CONSTANT(1) << uint8(flag);
//		_PropertyDatabase.setProp( "INTERFACES:FLAGS", _InterfacesFlagsBitField);
		CBankAccessor_PLR::getINTERFACES().setFLAGS(_PropertyDatabase, _InterfacesFlagsBitField);
	}
	else
	{
		nlwarning("<CCharacter::removeKnownBrick> Can't remove known brick cause static form of idSheet (%s) missing", brickId.toString().c_str() );
	}
} //removeKnownBrick//

//-----------------------------------------------
// CCharacter::processTrainingBrick
//-----------------------------------------------
void CCharacter::processTrainingBrick( const CStaticBrick *brick, bool sendChatMessage )
{
	if (!brick)
		return;

	const uint nbParams = (uint)brick->Params.size();
	for ( uint i = 0 ; i < nbParams ; ++i )
	{
		const TBrickParam::IId* param = brick->Params[i];
		switch(param->id())
		{
		case  TBrickParam::CHARAC_UPGRADE:
			{
				CHARACTERISTICS::TCharacteristics charac = CHARACTERISTICS::toCharacteristic((((CSBrickParamCharacUpgrade *)param)->Characteristic));
				if (charac == CHARACTERISTICS::Unknown)
				{
					nlwarning("Training brick %s has bad charc parameter %s", brick->SheetId.toString().c_str(), ((CSBrickParamCharacUpgrade *)param)->Characteristic.c_str());
				}
				else
				{
					const sint16 modifier = (sint16) ((CSBrickParamCharacUpgrade *)param)->Modifier;
					changeCharacteristic(charac, modifier);

					if (sendChatMessage)
					{
						SM_STATIC_PARAMS_3(params, STRING_MANAGER::characteristic, STRING_MANAGER::integer, STRING_MANAGER::integer);
						params[0].Enum = charac;
						params[1].Int = _PhysCharacs._PhysicalCharacteristics[charac].Base;
						params[2].Int = brick->SkillPointPrice;
						PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "PHRASE_CHARAC_BUY", params);
					}
				}
			}
			break;

		case TBrickParam::SCORE_UPGRADE:
			{
				const SCORES::TScores score = SCORES::toScore((((CSBrickParamScoreUpgrade *)param)->Score));
				if ( score < 0 || score >= SCORES::NUM_SCORES)
				{
					nlwarning("Training brick %s has bad score parameter %s", brick->SheetId.toString().c_str(), ((CSBrickParamScoreUpgrade *)param)->Score.c_str());
				}
				else
				{
					const sint32 modifier = ((CSBrickParamScoreUpgrade *)param)->Modifier;
					// change Score
					_ScorePermanentModifiers[score] += modifier;
					_PhysScores._PhysicalScores[ score ].Base += modifier;
				}
			}
			break;

		default:
			;
		};
	}
}


//-----------------------------------------------
// CCharacter::processTrainingBrick
//-----------------------------------------------
void CCharacter::unprocessTrainingBrick( const CStaticBrick *brick, bool sendChatMessage )
{
	if (!brick)
		return;

	const uint nbParams = (uint)brick->Params.size();
	for ( uint i = 0 ; i < nbParams ; ++i )
	{
		const TBrickParam::IId* param = brick->Params[i];
		switch(param->id())
		{
		case  TBrickParam::CHARAC_UPGRADE:
			{
				CHARACTERISTICS::TCharacteristics charac = CHARACTERISTICS::toCharacteristic((((CSBrickParamCharacUpgrade *)param)->Characteristic));
				if (charac == CHARACTERISTICS::Unknown)
				{
					nlwarning("Training brick %s has bad charc parameter %s", brick->SheetId.toString().c_str(), ((CSBrickParamCharacUpgrade *)param)->Characteristic.c_str());
				}
				else
				{
					const sint16 modifier = (sint16) ((CSBrickParamCharacUpgrade *)param)->Modifier;
					changeCharacteristic(charac, -modifier);

					if (sendChatMessage)
					{
						SM_STATIC_PARAMS_3(params, STRING_MANAGER::characteristic, STRING_MANAGER::integer, STRING_MANAGER::integer);
						params[0].Enum = charac;
						params[1].Int = _PhysCharacs._PhysicalCharacteristics[charac].Base;
						params[2].Int = brick->SkillPointPrice;
						PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "PHRASE_CHARAC_BUY", params);
					}
				}
			}
			break;

		case TBrickParam::SCORE_UPGRADE:
			{
				const SCORES::TScores score = SCORES::toScore((((CSBrickParamScoreUpgrade *)param)->Score));
				if ( score < 0 || score >= SCORES::NUM_SCORES)
				{
					nlwarning("Training brick %s has bad score parameter %s", brick->SheetId.toString().c_str(), ((CSBrickParamScoreUpgrade *)param)->Score.c_str());
				}
				else
				{
					const sint32 modifier = ((CSBrickParamScoreUpgrade *)param)->Modifier;
					// change Score
					_ScorePermanentModifiers[score] -= modifier;
					_PhysScores._PhysicalScores[ score ].Base -= modifier;
				}
			}
			break;

		default:
			;
		};
	}
}


//-----------------------------------------------
// Return the values of properties of the known bricks of the specified family (useful for bonus values)
//-----------------------------------------------
void CCharacter::getPropertiesFromKnownBricks(
	BRICK_FAMILIES::TBrickFamily brickFamily,
	CBrickProperties& results )
{
	results.clear();

	// Browse the known bricks
	const std::set<NLMISC::CSheetId>& knownBricks = getKnownBricks();
	std::set<NLMISC::CSheetId>::const_iterator it, end = knownBricks.end();
	for ( it=knownBricks.begin(); it!=end; ++it )
	{
		const NLMISC::CSheetId& brickSheetId = *it;
		const CStaticBrick* brick = CSheets::getSBrickForm( brickSheetId );
		if ( brick )
		{
			if ( brick->Family == brickFamily )
			{
				// Browse the params of the brick matching the family
				for ( std::vector<TBrickParam::IIdPtr>::const_iterator ip=brick->Params.begin(); ip!=brick->Params.end(); ++ip )
				{
					TBrickParam::IIdPtr param = (*ip);

					// Add the value by property
					results[ param->id() ].push_back( param );
				}
			}
		}
	}
}


//-----------------------------------------------
// fillFaberMaterialArray
//-----------------------------------------------
bool CCharacter::fillFaberMaterialArray( vector<CFaberMsgItem>& materialsSelectedForFaber, vector< const CStaticItem * >& materials, uint16& lowerMaterialQuality )
{
	bool ok = true;
	lowerMaterialQuality = USHRT_MAX;
	for( uint s = 0; s < materialsSelectedForFaber.size(); ++s )
	{
		bool bOk = false;
		CInventoryPtr pInv = NULL;
		if( materialsSelectedForFaber[ s ].getInvId() >= INVENTORIES::NUM_INVENTORY )
		{
			// Also allow crafting from player's room
			if (materialsSelectedForFaber[ s ].getInvId() == INVENTORIES::player_room)
			{
				if (getRoomInterface().isValid() && getRoomInterface().canUseInventory(this, this))
				{
					pInv = getRoomInterface().getInventory();
					bOk = true;
				}
			}

			if ( ! bOk) 
			{
				nlwarning("<CCharacter::fillFaberMaterialArray> CFaberMsgItem[%d] sended by client contains an invalid inventory index %d", s, materialsSelectedForFaber[ s ].getInvId() );
				return false;
			}
		}
		else
		{
			pInv = _Inventory[ materialsSelectedForFaber[ s ].getInvId() ];
		}

		if( materialsSelectedForFaber[ s ].IndexInInv >= pInv->getSlotCount() )
		{
			nlwarning("<CCharacter::fillFaberMaterialArray> CFaberMsgItem[%d] sended by client contains an invalid index %d for inventory %d", s, materialsSelectedForFaber[ s ].IndexInInv, materialsSelectedForFaber[ s ].getInvId() );
			return false;
		}

		uint quantityLeft = materialsSelectedForFaber[ s ].Quantity;
		for( uint i = 0; i < quantityLeft; ++i )
		{
			CGameItemPtr material = pInv->getItem( materialsSelectedForFaber[ s ].IndexInInv );
			if (material != 0)
			{
				if( quantityLeft > material->getStackSize() )
				{
					nlwarning("<CCharacter::fillFaberMaterialArray> CFaberMsgItem[%d] sended by client contains an invalid quantity %d asked because stack is smaller (size =  %d)", s, materialsSelectedForFaber[ s ].Quantity, pInv->getItem( materialsSelectedForFaber[ s ].IndexInInv )->getStackSize() );
					return false;
				}
			}

			if( material != 0 )
			{
				const CStaticItem * si = CSheets::getForm( material->getSheetId() );
				if( si )
				{
					if( si->Family == ITEMFAMILY::RAW_MATERIAL )
					{
						if( material->quality() < lowerMaterialQuality ) lowerMaterialQuality = material->quality();
						materials.push_back( si );
					}
					else
					{
						nlwarning("<CCharacter::fillFaberMaterialArray> Static Form %s is a item of family %s, we must have RAW_MATERIAL item", material->getSheetId().toString().c_str(), ITEMFAMILY::toString(si->Family).c_str() );
						ok = false;
					}
				}
				else
				{
					nlwarning("<CCharacter::fillFaberMaterialArray> Static Form %s not found for raw material select index %d", material->getSheetId().toString().c_str(), i );
					ok = false;
				}
			}
			else
			{
				nlwarning("<CCharacter::fillFaberMaterialArray> Bag or stack contains no item in bag for slot %d, inventory %d", materialsSelectedForFaber[ s ].IndexInInv, materialsSelectedForFaber[ s ].getInvId() );
				ok = false;
			}
		}
	}
	return ok;

} // fillFaberMaterialArray //



//-----------------------------------------------
// CCharacter::getFillFaberRms: fill vector of const GameItem pointer with Raw material used for faber
//-----------------------------------------------
bool CCharacter::getFillFaberRms( std::vector< const CStaticItem * >& rms, std::vector< const CStaticItem * >& rmsFormula, uint16& lowerQuality )
{
	//_TempInventoryMode = TEMP_INV_MODE::None;
	leaveTempInventoryMode();

	uint16 lowerRmQuality;
	uint16 lowerRmFormulaQuality;

	if( fillFaberMaterialArray(_RmSelectedForFaber, rms, lowerRmQuality) &&
		fillFaberMaterialArray(_RmFormulaSelectedForFaber, rmsFormula, lowerRmFormulaQuality) )
	{
		lowerQuality = ( lowerRmQuality < lowerRmFormulaQuality) ? lowerRmQuality : lowerRmFormulaQuality;
		return true;
	}
	else
	{
		return false;
	}

} // getFillFaberRms //


//-----------------------------------------------
// CCharacter::lockFaberRms: lock Faber raw material, return true if lock success
//-----------------------------------------------
bool CCharacter::lockFaberRms()
{
	uint size = (uint)_RmSelectedForFaber.size();

	for( uint i = 0; i < size; ++i )
	{
		if( (INVENTORIES::TInventory)_RmSelectedForFaber[ i ].getInvId() >= INVENTORIES::pet_animal && (INVENTORIES::TInventory)_RmSelectedForFaber[ i ].getInvId() < INVENTORIES::max_pet_animal )
		{
			uint32 beastindex = _RmSelectedForFaber[ i ].getInvId() - INVENTORIES::pet_animal;
			if( (_PlayerPets[ beastindex ].AnimalStatus & ANIMAL_STATUS::InventoryAvailableFlag) == 0 )
			{
				return false;
			}
		}

		if( lockItem( (INVENTORIES::TInventory)_RmSelectedForFaber[ i ].getInvId(), _RmSelectedForFaber[ i ].IndexInInv, _RmSelectedForFaber[ i ].Quantity ) == false )
		{
			return false;
		}
	}

	size = (uint)_RmFormulaSelectedForFaber.size();

	for( uint i = 0; i < size; ++i )
	{
		if( (INVENTORIES::TInventory)_RmFormulaSelectedForFaber[ i ].getInvId() >= INVENTORIES::pet_animal && (INVENTORIES::TInventory)_RmFormulaSelectedForFaber[ i ].getInvId() < INVENTORIES::max_pet_animal )
		{
			uint32 beastindex = _RmFormulaSelectedForFaber[ i ].getInvId() - INVENTORIES::pet_animal;
			if( (_PlayerPets[ beastindex ].AnimalStatus & ANIMAL_STATUS::InventoryAvailableFlag) == 0 )
			{
				return false;
			}
		}

		if( lockItem( (INVENTORIES::TInventory)_RmFormulaSelectedForFaber[ i ].getInvId(), _RmFormulaSelectedForFaber[ i ].IndexInInv, _RmFormulaSelectedForFaber[ i ].Quantity ) == false )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------
// CCharacter::unlockFaberRms: unlock Faber raw material
//-----------------------------------------------
void CCharacter::unlockFaberRms()
{
	uint size = (uint)_RmSelectedForFaber.size();

	for( uint i = 0; i < size; ++i )
	{
		unLockItem( (INVENTORIES::TInventory)_RmSelectedForFaber[ i ].getInvId(), _RmSelectedForFaber[ i ].IndexInInv, _RmSelectedForFaber[ i ].Quantity );
	}

	size = (uint)_RmFormulaSelectedForFaber.size();

	for( uint i = 0; i < size; ++i )
	{
		unLockItem( (INVENTORIES::TInventory)_RmFormulaSelectedForFaber[ i ].getInvId(), _RmFormulaSelectedForFaber[ i ].IndexInInv, _RmFormulaSelectedForFaber[ i ].Quantity );
	}
}


//-----------------------------------------------
// CCharacter::consumeFaberRms: consume Faber raw material
//-----------------------------------------------
void CCharacter::consumeFaberRms(bool failed)
{
	TLogContext_Item_ConsumeFaberMp logContext(_Id);
	unlockFaberRms();
	uint size = (uint)_RmSelectedForFaber.size();

	for( uint i = 0; i < size; ++i )
	{
		if( failed == false || uint32((RandomGenerator.rand(99)+1)) <= CraftFailureProbaMpLost)
			destroyItem( _RmSelectedForFaber[ i ].getInvId(), _RmSelectedForFaber[ i ].IndexInInv, _RmSelectedForFaber[ i ].Quantity );
	}

	size = (uint)_RmFormulaSelectedForFaber.size();

	for( uint i = 0; i < size; ++i )
	{
		if( failed == false || uint32((RandomGenerator.rand(99)+1)) <= CraftFailureProbaMpLost)
			destroyItem( _RmFormulaSelectedForFaber[ i ].getInvId(), _RmFormulaSelectedForFaber[ i ].IndexInInv, _RmFormulaSelectedForFaber[ i ].Quantity );
	}
}


//-----------------------------------------------
// CCharacter::updateVisualInformation : update visual information before inventory manipulation
//-----------------------------------------------
void CCharacter::updateVisualInformation( uint16 InventoryEmpty, uint16 SlotEmpty, uint16 InventoryFull, uint16 SlotFull, const CSheetId& IdSheetItem, CGameItemPtr Item )
{
	if( InventoryEmpty == INVENTORIES::handling )
	{
		if( SlotEmpty == INVENTORIES::right )
		{
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.WeaponRightHand, 0 );
			SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.RTrail, 0 );
			_Items.Sheath[ 0 ].HandR = SMirrorEquipment( CSheetId(), 0 );
		}
		else if( SlotEmpty == INVENTORIES::left )
		{
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.WeaponLeftHand, 0 );
			SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.LTrail, 0 );
			_Items.Sheath[ 0 ].HandL = SMirrorEquipment( CSheetId(), 0 );
		}
		else
		{
			_Items.Sheath[ 0 ].Ammo = SMirrorEquipment( CSheetId(), 0 );
		}
	}
	else if( InventoryEmpty == INVENTORIES::equipment )
	{
		const CStaticRaceStats * raceForm = CSheets::getRaceStats( CSheetId(_SheetId) );
		const CStaticItem* srcForm = 0;
		if( raceForm )
		{
			switch( SlotEmpty )
			{
			case SLOT_EQUIPMENT::HEAD:
				// keep a NULL form and update visual information with custom properties
				SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.HatModel, _HairType );
				SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.HatColor, _HairColor );
				break;
			case SLOT_EQUIPMENT::CHEST:
				switch( _Gender )
				{
					case GSGENDER::male :
						srcForm = CSheets::getForm( CSheetId( raceForm->MaleDefaultEquipment.DefaultChest ) );
						break;
					case GSGENDER::female :
						srcForm = CSheets::getForm( CSheetId( raceForm->FemaleDefaultEquipment.DefaultChest ) );
						break;
				}
				break;
			case SLOT_EQUIPMENT::ARMS:
				switch( _Gender )
				{
					case GSGENDER::male :
						srcForm = CSheets::getForm( CSheetId( raceForm->MaleDefaultEquipment.DefaultArms ) );
						break;
					case GSGENDER::female :
						srcForm = CSheets::getForm( CSheetId( raceForm->FemaleDefaultEquipment.DefaultArms ) );
						break;
				}
				break;
			case SLOT_EQUIPMENT::HANDS:
				switch( _Gender )
				{
					case GSGENDER::male :
						srcForm = CSheets::getForm( CSheetId( raceForm->MaleDefaultEquipment.DefaultHands ) );
						break;
					case GSGENDER::female :
						srcForm = CSheets::getForm( CSheetId( raceForm->FemaleDefaultEquipment.DefaultHands ) );
						break;
				}
				break;
			case SLOT_EQUIPMENT::LEGS:
				switch( _Gender )
				{
					case GSGENDER::male :
						srcForm = CSheets::getForm( CSheetId( raceForm->MaleDefaultEquipment.DefaultLegs ) );
						break;
					case GSGENDER::female :
						srcForm = CSheets::getForm( CSheetId( raceForm->FemaleDefaultEquipment.DefaultLegs ) );
						break;
				}
				break;
			case SLOT_EQUIPMENT::FEET:
				switch( _Gender )
				{
					case GSGENDER::male :
						srcForm = CSheets::getForm( CSheetId( raceForm->MaleDefaultEquipment.DefaultFeet ) );
						break;
					case GSGENDER::female :
						srcForm = CSheets::getForm( CSheetId( raceForm->FemaleDefaultEquipment.DefaultFeet ) );
						break;
				}
				break;
			default:;
			}
			// update visual property only if srcForm is valid
			if(srcForm)
				setVisualPropertyForEquipment( SlotEmpty, srcForm, 0 , 254 );
		}
	}

	if( InventoryFull == INVENTORIES::handling )
	{
		const CStaticItem* srcForm = CSheets::getForm( IdSheetItem );
		if( srcForm && Item != 0 )
		{
			// get ref skill level
			const uint16 refSkillLevel = min( (uint16)getSkillValue(srcForm->Skill), Item->quality() );
			if( SlotFull == INVENTORIES::right )
			{
				SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.WeaponRightHand, srcForm->ItemIdSheetToModelNumber );
				SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.RTrail, getRightTrailValue(refSkillLevel) );
				_Items.Sheath[ 0 ].HandR = SMirrorEquipment( IdSheetItem, Item->quality() );
			}
			else if( SlotFull == INVENTORIES::left )
			{
				SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.WeaponLeftHand, srcForm->ItemIdSheetToModelNumberLeftHands );
				SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.LTrail, getLeftTrailValue(refSkillLevel) );
				_Items.Sheath[ 0 ].HandL = SMirrorEquipment( IdSheetItem, Item->quality() );
			}
			else
			{
				_Items.Sheath[ 0 ].Ammo = SMirrorEquipment( IdSheetItem, Item->quality() );
			}
		}
	}
	else if( InventoryFull == INVENTORIES::equipment )
	{
		const CStaticItem* srcForm = CSheets::getForm( IdSheetItem );
		if( ( srcForm != 0 ) && ( Item != NULL ) )
		{
			setVisualPropertyForEquipment( SlotFull, srcForm, Item->quality(), Item->color() );
		}
	}
} // updateVisualInformation //

//-----------------------------------------------
// CCharacter::setVisualPropertyForEquipment : update visual properties after equipment inventory manipulation
//-----------------------------------------------
void CCharacter::setVisualPropertyForEquipment( uint16 slot, const CStaticItem* srcForm, uint16 quality, uint8 color )
{
	if( srcForm == NULL )
	{
		nlwarning("<CCharacter::setVisualPropertyForEquipment> Character %s Try to set visual properties with null form");
		return;
	}
	switch( slot )
	{
		case SLOT_EQUIPMENT::HEADDRESS:
			_Items.Headdress = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::HEAD:
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.HatModel, srcForm->ItemIdSheetToModelNumber );
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.HatColor, color );
			_Items.Head = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::EARL:
			_Items.EarL = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::EARR:
			_Items.EarR = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::NECKLACE:
			_Items.Neck = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::CHEST:
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.JacketModel, srcForm->ItemIdSheetToModelNumber );
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.JacketColor, color );
			_Items.Chest = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::ARMS:
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.ArmModel, srcForm->ItemIdSheetToModelNumber );
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.ArmColor, color );
			_Items.Arms = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::WRISTL:
			_Items.WristL = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::WRISTR:
			_Items.WristR = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::HANDS:
			SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.HandsModel, srcForm->ItemIdSheetToModelNumber );
			SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.HandsColor, color );
			_Items.Hands = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::FINGERL:
			_Items.FingerL = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::FINGERR:
			_Items.FingerR = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::LEGS:
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.TrouserModel, srcForm->ItemIdSheetToModelNumber );
			SET_STRUCT_MEMBER( _VisualPropertyA, PropertySubData.TrouserColor, color );
			_Items.Legs = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::ANKLEL:
			_Items.AnkleL = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::ANKLER:
			_Items.AnkleR = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		case SLOT_EQUIPMENT::FEET:
			SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.FeetModel, srcForm->ItemIdSheetToModelNumber );
			SET_STRUCT_MEMBER( _VisualPropertyB, PropertySubData.FeetColor, color );
			_Items.Feet = SMirrorEquipment( srcForm->SheetId, quality );
			break;
		default:;
	}
} // setVisualPropertyForEquipment //


//-----------------------------------------------
// CCharacter::teleportCharacter tp wanted, check if tp is regular and send a server tp command
// allowNearPetTp() or forbidNearPetTp() must have been called
//-----------------------------------------------
void CCharacter::teleportCharacter( sint32 x, sint32 y, sint32 z, bool teleportWithMount, bool useHeading , float heading , uint8 continent , sint32 cell, uint8 season, const  R2::TR2TpInfos& tpInfos)
{
	if ( ! getEnterFlag() ) // wait for the properties to be in the mirror
		return;

	// Check if player is dying
	if( currentHp() <= 0 )
	{
		return;
	}

	if (IsRingShard)
	{
		_RingSeason = season;
	}


	// force player to leave PVP if engaged
	if ( getPVPInterface().isValid() )
		getPVPInterface().leavePVP(IPVP::Teleport);

	// take all items from temp inventory
	sendCloseTempInventoryImpulsion();

	// manage teleport in pvp
	CPVPManager2::getInstance()->playerTeleports(this);

	// force player to unmount
	if( TheDataset.isAccessible(_EntityMounted()) )
	{
		TDataSetRow creatureId = _EntityMounted;

		unmount( false );

		if (teleportWithMount)
		{
			// if player is a GM or DEV or better, also TP mounted creature
			CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(_Id) );
			if (player != NULL)
			{
				if ( player->havePriv(TeleportWithMektoubPriv) )
				{
					CCreature *creature = CreatureManager.getCreature(creatureId);
					if (creature)
					{
						creature->tpWanted(x,y,z,useHeading,heading,continent,cell);
					}
				}
			}
		}
	}

	CBuildingManager::getInstance()->removePlayerFromRoom( this );

	// despawn pets or stop it
	for ( uint i =  0; i < _PlayerPets.size(); i++ )
	{
		if( _PlayerPets[ i ].PetStatus == CPetAnimal::landscape )
		{
			if( TheDataset.isAccessible(_PlayerPets[ i ].SpawnedPets) )
			{
				CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ i ].SpawnedPets ) );
				if( c )
				{
					CVector2f animalPos;
					CVector2f characterPos;
					c->getState().getVector2f(animalPos);
					getState().getVector2f(characterPos);
					float squareDistance = (animalPos - characterPos).sqrnorm();

					// Teleport as well pets that are following or mounted and in the neighbourhood
					if( ( isNearPetTpIsAllowed() && (_PlayerPets[ i ].IsFollowing || _PlayerPets[ i ].IsMounted) && squareDistance <= 50.0f * 50.0f ) )
					{
						// despawn it
						sendPetCommand( CPetCommandMsg::DESPAWN, i, true );
						_PlayerPets[ i ].SpawnedPets = TDataSetRow();
						_PlayerPets[ i ].PetStatus = CPetAnimal::tp_continent;
					}
					else
					{
						// stop it
						sendPetCommand( CPetCommandMsg::STAND, i, true );
					}
				}
			}
		}
	}

	_TpCoordinate.X = x;
	_TpCoordinate.Y = y;
	_TpCoordinate.Z = z;
	//outdoor teleport so cell is 0
	_TpCoordinate.Heading = heading;
	_TpCoordinate.Cell = cell;
	_TpCoordinate.Continent = continent;

	CPhraseManager::getInstance().removeEntity(TheDataset.getDataSetRow(_Id), false);
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;
	GenericMsgManager.pushNameToStream(season == 0xff ?  "TP:DEST" : "TP:DEST_WITH_SEASON", bms);
	bms.serial( x );
	bms.serial( y );
	bms.serial( z );
	bms.serialBit( useHeading );
	if ( useHeading )
		bms.serial( heading );
	if (season != 0xff)
	{
		bms.serial(season);
	}

	bms.serial(const_cast<R2::TR2TpInfos&>(tpInfos));
	/*
	R2::TTeleportContext tpContext = R2::TPContext_Unknown;
	std::string tpReasonId;
	std::string tpCancelTextId;
	impulse.serial(tpReasonId);
	impulse.serial(tpCancelTextId);
	impulse.serialEnum(tpContext);
*/

	msgout.serialBufferWithSize ( (uint8*)bms.buffer(), bms.length() );
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout);

	CMessage msgout2("ENTITY_TELEPORTATION");
	msgout2.serial( _Id );
	if (IsRingShard)
	{
		nlinfo("Asking GPMS to TP character %s to (0,0)",_Id.toString().c_str());
	}
	sendMessageViaMirror("GPMS", msgout2);

	// send message to AIS to indicate this player has respawned/teleport and must be removed from aggro lists
	CAIPlayerRespawnMsg respawnMsg;
	respawnMsg.PlayerRowId = _EntityRowId;
	CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), respawnMsg);
//	respawnMsg.send("AIS");

	// backup the who sees me property and set it to 0
	CMirrorPropValue<TYPE_WHO_SEES_ME> whoSeesMe(TheDataset, _EntityRowId, DSPropertyWHO_SEES_ME );


	/*
	FOR AIS the change of property value AIInstance is handled before the sendAggro message send by setWhoSeesMe function.
	So we first change the AI Instance to prevent the setWhosSeesMe to send message to ais about and npc that is previously
	removed of the aiInstance after the setInstanceNumber(INVALID_AI_INSTANCE).
	*/
	// reset the continent instance during tp
	if (IsRingShard)
		setRingShardInstanceNumber(INVALID_AI_INSTANCE);
	else
		setInstanceNumber(INVALID_AI_INSTANCE);


	//////////////////////////////////////////////////////////////////////////
	// Looking for invisibility bug
	uint64 NOT_TELEPORTING_FLAG= (((uint64)0x12345678)<<32)| (uint64)0x87654321;
	if ( _WhoSeesMeBeforeTP != NOT_TELEPORTING_FLAG )
	{
		nlwarning("INVISIBILITY (teleportCharacter) Player %s already being Tp as _WhoSeesMeBeforeTp is = %"NL_I64"u and WhoSeesMe = %"NL_I64"u", _Id.toString().c_str(), _WhoSeesMeBeforeTP, whoSeesMe.getValue() );
		nlwarning("INVISIBILITY (teleportCharacter) Current Coordinates :(%d, %d, %d) TP coordinates : (%d, %d, %d)", x,y,z);
	}
	else
	{
		_WhoSeesMeBeforeTP = whoSeesMe.getValue();
		setWhoSeesMe( IsRingShard? R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_INVISIBLE_PLAYER,false): uint64(0));
	}
	//////////////////////////////////////////////////////////////////////////


	if (IsRingShard)
	{
		_RingSeason = season;
	}

}


//-----------------------------------------------
void CCharacter::setCurrentContinent (CONTINENT::TContinent continent)
{
	_CurrentContinent = continent;
}

//-----------------------------------------------
// CCharacter::addCharacterAnimal buy a creature
//-----------------------------------------------
bool CCharacter::addCharacterAnimal( const CSheetId& PetTicket, uint32 Price, CGameItemPtr ptr )
{
	if ( !PackAnimalSystemEnabled )
		return false;

	CPetAnimal pet;
	pet.PetStatus = CPetAnimal::waiting_spawn;
	pet.TicketPetSheetId = PetTicket;
	pet.Price = Price;
	pet.ItemPtr = ptr;
	pet.OwnerId = _Id;

	if( checkAnimalCount( PetTicket, true, 1 ) )
	{
		const CStaticItem* form = CSheets::getForm( PetTicket );
		pet.PetSheetId = form->PetSheet;
		pet.Satiety = form->PetHungerCount;
		pet.MaxSatiety = form->PetHungerCount;

		sint32 i = getFreePetSlot();
		if( i >= 0 )
		{
			_PlayerPets[ i ] = pet;

			pet.ItemPtr->setPetIndex(i);
			pet.Slot = ptr->getInventorySlot();

			// init pet inventory
			if ( ! initPetInventory( i ))
			{
				return false;
			}

			return spawnCharacterAnimal( i );
		}
	}
	return false;
}


//-----------------------------------------------
// CCharacter::getFreePetSlot return free slot for pet spawn or -1 if there are no free slot
//-----------------------------------------------
sint32 CCharacter::getFreePetSlot()
{
	for( sint32 i = 0; i < (sint32)_PlayerPets.size(); ++i )
	{
		if( _PlayerPets[ i ].TicketPetSheetId == CSheetId::Unknown )
		{
			return i;
		}
	}
	return -1;
}


//-----------------------------------------------
// CCharacter::checkAnimalCount return true if can add 'delta' pets to current player pets
//-----------------------------------------------
bool CCharacter::checkAnimalCount( const CSheetId& PetTicket, bool sendMessage, sint32 delta )
{
	if ( !PackAnimalSystemEnabled )
		return false;

	const CStaticItem* form = CSheets::getForm( PetTicket );
	if( form == 0 )
	{
		if( sendMessage )
		{
			nlwarning("<CCharacter::addCharacterAnimal> Can't find static sheet for item %s (Character %s buy this pet )", PetTicket.toString().c_str(),  _Id.toString().c_str() );
		}
		return false;
	}

	if( form->Family != ITEMFAMILY::PET_ANIMAL_TICKET )
	{
		if( sendMessage )
		{
			nlwarning("<CCharacter::addCharacterAnimal> Sheet %s have not right item family (%s) (Character %s buy this pet )", PetTicket.toString().c_str(), ITEMFAMILY::toString( form->Family ).c_str(), _Id.toString().c_str() );
		}
		return false;
	}

	if( form->PetSheet == CSheetId::Unknown )
	{
		if( sendMessage )
		{
			nlwarning("<CCharacter::addCharacterAnimal> Cant found sheet name of creature found in ticket %s", PetTicket.toString().c_str() );
		}
		return false;
	}

	if( form->Type == ITEM_TYPE::MEKTOUB_MOUNT_TICKET )
	{
		uint32 nbMektoubMount = 0;
		for( vector< CPetAnimal >::const_iterator it = _PlayerPets.begin(); it != _PlayerPets.end(); ++it )
		{
			// check sheet is asigned (prevent an useless warning)
			if ((*it).PetSheetId != CSheetId::Unknown)
			{
				const CStaticCreatures * form = CSheets::getCreaturesForm( (*it).PetSheetId );
				if( form )
				{
					if( form->getRace() == EGSPD::CPeople::MektoubMount )
					{
						++nbMektoubMount;
					}
				}
			}
		}
		// check we can add delta mount
		if( (nbMektoubMount + delta) > MAX_MEKTOUB_MOUNT )
		{
			if( sendMessage )
			{
				sendDynamicSystemMessage( _Id, "EGS_ALREADY_HAVE_MOUNT" );
			}
			return false;
		}
	}
	else if( form->Type == ITEM_TYPE::MEKTOUB_PACKER_TICKET )
	{
		uint32 nbMektoubPacker = 0;
		for( vector< CPetAnimal >::const_iterator it = _PlayerPets.begin(); it != _PlayerPets.end(); ++it )
		{
			// check sheet is asigned (prevent an useless warning)
			if ((*it).PetSheetId != CSheetId::Unknown)
			{
				const CStaticCreatures * form = CSheets::getCreaturesForm( (*it).PetSheetId );
				if( form )
				{
					if( form->getRace() == EGSPD::CPeople::MektoubPacker )
					{
						++nbMektoubPacker;
					}
				}
			}
		}
		// check we can add delta packer
		if( (nbMektoubPacker + delta) > MAX_PACK_ANIMAL )
		{
			if( sendMessage )
			{
				sendDynamicSystemMessage( _Id, "EGS_CANT_BUY_ANOTHER_PACKER" );
			}
			return false;
		}

		CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
		BOMB_IF(p == NULL,"Failed to find player record for character: "<<getId().toString(),return 0.0);
		if ( p->isTrialPlayer() )
		{
			if( sendMessage )
			{
				sendDynamicSystemMessage( _Id, "EGS_CANT_BUY_PACKER_IS_TRIAL_PLAYER" );
			}
			return false;
		}
	}
	else
	{
		if( sendMessage )
		{
			nlwarning("<CCharacter::addCharacterAnimal> PetAnimal %s not yet implemented", PetTicket.toString().c_str() );
		}
		return false;
	}
	return true;
}


//-----------------------------------------------
// CCharacter::respawnPet respawn all pet of player
//-----------------------------------------------
void CCharacter::respawnPet()
{
	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		if( _PlayerPets[ i ].PetStatus != CPetAnimal::not_present && ( _PlayerPets[ i ].PetStatus != CPetAnimal::stable || _RespawnMainLandInTown ) )
		{
			if( _PlayerPets[ i ].TicketPetSheetId != CSheetId::Unknown )
			{
				spawnCharacterAnimal( i );
			}
		}
		else if( _PlayerPets[ i ].PetStatus == CPetAnimal::stable )
		{
			// check if stable exist
			CStable::TStableData stableData;
			if( ! CStable::getInstance()->getStableData( (uint16)_PlayerPets[ i ].StableId, stableData ) )
			{
				_PlayerPets[ i ].PetStatus = CPetAnimal::waiting_spawn;
				spawnCharacterAnimal( i );
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CCharacter::respawnPetForInstance( uint32 InstanceNumber, const std::string& InstanceContinent )
{
	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		if( _PlayerPets[ i ].PetStatus != CPetAnimal::not_present && _PlayerPets[ i ].PetStatus != CPetAnimal::stable )
		{
			if( _PlayerPets[ i ].TicketPetSheetId != CSheetId::Unknown )
			{
				uint32 instance = INVALID_AI_INSTANCE;
				CContinent * cont = CZoneManager::getInstance().getContinent( _PlayerPets[ i ].Landscape_X, _PlayerPets[ i ].Landscape_Y );
				if ( cont )
				{
					instance = CUsedContinent::instance().getInstanceForContinent( CONTINENT::TContinent(cont->getId()) );
					if ( instance != INVALID_AI_INSTANCE )
					{
						if( InstanceNumber == instance )
						{
							spawnCharacterAnimal( i );
							continue;
						}
					}
				}

				if( instance == INVALID_AI_INSTANCE )
				{
					nlwarning("<CCharacter::respawnPetForInstance> Pet %d are not in a continent, re-spawn it near player...", i );
					_PlayerPets[ i ].PetStatus = CPetAnimal::waiting_spawn;
					spawnCharacterAnimal( i );
				}
			}
		}
		else if( _PlayerPets[ i ].PetStatus == CPetAnimal::stable )
		{
			// check if stable exist
			CStable::TStableData stableData;
			if( ! CStable::getInstance()->getStableData( (uint16)_PlayerPets[ i ].StableId, stableData ) )
			{
				_PlayerPets[ i ].PetStatus = CPetAnimal::waiting_spawn;
				spawnCharacterAnimal( i );
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CCharacter::respawnPetAfterTp( const SGameCoordinate& destination, uint32 destAIInstance )
{
	if (destination.Continent == CONTINENT::UNKNOWN)
		return;

	for ( uint i =  0; i < _PlayerPets.size(); ++i )
	{
		if ( _PlayerPets[ i ].PetStatus == CPetAnimal::tp_continent )
		{
			_PlayerPets[ i ].PetStatus = CPetAnimal::waiting_spawn;
			spawnWaitingCharacterAnimalNear( i, destination, destAIInstance );
		}
	}
}


//-----------------------------------------------
// Specialized version of spawnCharacterAnimal() for respawnPetAfterTp()
//-----------------------------------------------
bool CCharacter::spawnWaitingCharacterAnimalNear( uint index, const SGameCoordinate& destination, uint32 destAIInstance )
{
	if ( index >= _PlayerPets.size() )
		return false;

	if ( TheDataset.isAccessible(_PlayerPets[ index ].SpawnedPets) )
	{
		nlwarning("Pets index %d already spawned has creature row %d for player %s", index, _PlayerPets[ index ].SpawnedPets.getIndex(), _Id.toString().c_str() );
		return false;
	}

	const float distFromPlayer = 2000.f * (index + 1);

	CPetSpawnMsg msg;
	msg.SpawnMode = CPetSpawnMsg::NEAR_POINT; // if NEAR_PLAYER, the AIS would take the pos of the player (0,0,0 at this time)
	msg.Coordinate_X = destination.X - sint32(cos(getHeading()) * distFromPlayer);
	msg.Coordinate_Y = destination.Y - sint32(sin(getHeading()) * distFromPlayer);
	msg.Coordinate_H = destination.Z;
	msg.Heading = getHeading();
	msg.CharacterMirrorRow = _EntityRowId;
	msg.PetSheetId = _PlayerPets[ index ].PetSheetId;
	msg.PetIdx = index;
	msg.CustomName = _PlayerPets[ index ].CustomName;
	msg.AIInstanceId = (uint16)destAIInstance;
	CWorldInstances::instance().msgToAIInstance( msg.AIInstanceId, msg);
	// The row will be received in AnimalSpawned()

	return true;
}


//-----------------------------------------------
// CCharacter::spawnCharacterAnimal want spawn one of own creature
//-----------------------------------------------
bool CCharacter::spawnCharacterAnimal(uint index )
{
	if( index == 255 )
	{
		bool returnValue = true;
		for( uint i = 0; i < _PlayerPets.size(); ++i )
		{
			if( _PlayerPets[ i ].PetStatus != CPetAnimal::not_present )
			{
				if( !TheDataset.isAccessible( _PlayerPets[ i ].SpawnedPets ) )
				{
					returnValue &= spawnCharacterAnimal( i );
				}
			}
		}
		return returnValue;
	}

	if( index < _PlayerPets.size() )
	{
		if( !TheDataset.isAccessible(_PlayerPets[ index ].SpawnedPets) )
		{
			CPetSpawnMsg msg;
			switch( _PlayerPets[ index ].PetStatus )
			{
				case CPetAnimal::waiting_spawn: // see also spawnWaitingCharacterAnimalNear()
					msg.SpawnMode = CPetSpawnMsg::NEAR_PLAYER;
					msg.Coordinate_X = getX();
					msg.Coordinate_Y = getY();
					msg.Coordinate_H = getZ();
					msg.Heading = 0.0f;
					break;
				case CPetAnimal::death:
					if( ( _PlayerPets[ index ].DeathTick + 3 * 24 * 36000 < CTickEventHandler::getGameCycle() ) )
					{
						removePetCharacterAfterDeath( index );
						return false;
					}
					else
					{
						updateAnimalDespawnDb( index );
					}
					// same as landscape if come here, so it's normal we haven't break here !
				case CPetAnimal::landscape:
					msg.SpawnMode = CPetSpawnMsg::NEAR_POINT;
					msg.Coordinate_X = _PlayerPets[ index ].Landscape_X;
					msg.Coordinate_Y = _PlayerPets[ index ].Landscape_Y;
					msg.Coordinate_H = _PlayerPets[ index ].Landscape_Z;
					msg.Heading = 0.0f;
					break;
				case CPetAnimal::stable:

					if (_PlayerPets[index].StableId != _CurrentStable)
					{
						nlwarning("<CCharacter::spawnCharacterAnimal> Received spawn message for animal in stable (index:%d) but player is not near the animal stable", index);
						return false;
					}

					msg.SpawnMode = CPetSpawnMsg::NEAR_POINT;
					{
						CStable::TStableData stable;
						if( CStable::getInstance()->getStableData( _CurrentStable, stable ) )
						{
							msg.Coordinate_X = stable.StableExitX;
							msg.Coordinate_Y = stable.StableExitY;
							msg.Coordinate_H = stable.StableExitZ;
							msg.Heading = stable.Theta;
						}
						else
						{
							nlwarning("<CCharacter::spawnCharacterAnimal> Received spawn message for a animal in stable but player is not near a stable");
							msg.SpawnMode = CPetSpawnMsg::NEAR_PLAYER;
							msg.Coordinate_X = getX();
							msg.Coordinate_Y = getY();
							msg.Coordinate_H = getZ();
							msg.Heading = 0.0f;
						}
					}
					break;
				default:
					nlwarning("<CCharacter::spawnCharacterAnimal> Unknown pet status %u for pet number %d", _PlayerPets[ index ].PetStatus, index );
					return false;
			}

			// set the race of owner
			setAnimalPeople( index );

			// respawn pet near player of character are loaded with old serial save file format
			if( _RespawnMainLandInTown )
			{
				msg.SpawnMode = CPetSpawnMsg::NEAR_PLAYER;
				msg.Coordinate_X = getX();
				msg.Coordinate_Y = getY();
				msg.Coordinate_H = getZ();
				msg.Heading = 0.0f;
			}

			msg.CharacterMirrorRow = _EntityRowId;
			msg.PetSheetId = _PlayerPets[ index ].PetSheetId;
			msg.PetIdx = index;
			msg.CustomName = _PlayerPets[ index ].CustomName;

			CVector pos;
			pos.x = msg.Coordinate_X * 0.001f;
			pos.y = msg.Coordinate_Y * 0.001f;
			pos.z = msg.Coordinate_H * 0.001f;
			uint32 continentId = _CurrentContinent;
			CContinent * continent = CZoneManager::getInstance().getContinent( pos );
			if( continent )
			{
				continentId = continent->getId();
			}
			msg.AIInstanceId = CUsedContinent::instance().getInstanceForContinent( (CONTINENT::TContinent) continentId );
			CWorldInstances::instance().msgToAIInstance( msg.AIInstanceId, msg);
			return true;
		}
		else
		{
			nlwarning("<CCharacter::spawnCharacterAnimal> Pets index %d already spawned has creature row %d for player %s", index, _PlayerPets[ index ].SpawnedPets.getIndex(), _Id.toString().c_str() );
		}
	}
	else
	{
		nlwarning("<CCharacter::spawnCharacterAnimal> Can't spawn pet index %d, character %s can have only %d pets", index, _Id.toString().c_str(), MAX_INVENTORY_ANIMAL );
	}
	return false;
}


//-----------------------------------------------
// CCharacter::AnimalSpawned character buy a creature
//-----------------------------------------------
void CCharacter::onAnimalSpawned( CPetSpawnConfirmationMsg::TSpawnError SpawnStatus, uint32 PetIdx, const TDataSetRow& PetMirrorRow )
{
	CPetAnimal& animal = _PlayerPets[PetIdx];
	if( SpawnStatus == CPetSpawnConfirmationMsg::NO_ERROR_SPAWN )
	{
		if( PetIdx < MAX_INVENTORY_ANIMAL )
		{
			if( !TheDataset.isAccessible(animal.SpawnedPets) )
			{
				CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( PetMirrorRow ) );
				if( c )
					c->setIsAPet(true);

				bool isInit = animal.SpawnedPets.isNull();
				animal.SpawnedPets = PetMirrorRow;

				if ( isInit )
				{
					// Export animal hunger properties
					setAnimalSatiety( PetIdx, animal.Satiety, c );

					// set the race of owner
					setAnimalPeople( PetIdx );
				}

				if( animal.PetStatus != CPetAnimal::death )
				{
					animal.PetStatus = CPetAnimal::landscape;

					// Store the new position (otherwise a teleport would decrease the satiety because of a great delta)
					animal.Landscape_X = c->getState().X(); // the pos is ready in mirror
					animal.Landscape_Y = c->getState().Y(); // thanks to the use of a
					animal.Landscape_Z = c->getState().Z(); // CMirrorTransportClass

					if( animal.IsFollowing )
					{
						// Resend the FOLLOW command (note: at this time, the player may still be in (0,0))
						sendPetCommand( CPetCommandMsg::FOLLOW, PetIdx, true );
					}
					if ( animal.IsMounted )
					{
						// Remount it!
						mount( PetMirrorRow );
					}
				}
				else
				{
					CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( animal.SpawnedPets ) );
					if( c )
					{
						c->setBehaviour( MBEHAV::DEATH, true );
						//c->setMode( MBEHAV::DEATH ); => send Message to AIS
						c->getScores()._PhysicalScores[SCORES::hit_points].Current = 0;
					}
				}
			}
			else
			{
				//TODO send message to client about technical failure on spawn animal
				nlwarning("<CCharacter::AnimalSpawned> PetIdx %d returned by AIS is already spawned (creature %d) for player %s", PetIdx, animal.SpawnedPets.getIndex(), _Id.toString().c_str() );
			}
		}
		else
		{
			//TODO send message to client about technical failure on spawn animal
			nlwarning("<CCharacter::AnimalSpawned> PetIdx %d returned by AIS is not in permited range for player %s", PetIdx, _Id.toString().c_str() );
		}
	}
	else
	{
		//TODO send message to client about technical failure on spawn animal
		nlwarning("<CCharacter::AnimalSpawned> AIS return error %d for spawning pet for player %s", SpawnStatus, _Id.toString().c_str() );
		spawnCharacterAnimal( PetIdx );
	}
}


/*
 * Init database value
 * Preconditions:
 * - petIndex<_PlayerPets.size())
 * - The current pet type has already been set in the database
 */
void CCharacter::updateAnimalHungerDb( uint petIndex )
{
	// Compute the hunger bar
	CPetAnimal& animal = _PlayerPets[petIndex];
	sint hunger;
	if ( (animal.Satiety <= 0) || (animal.MaxSatiety == 0) )
	{
		hunger = ANIMAL_TYPE::DbHungryValue; // only used when hunger is really 0
	}
	else
	{
		hunger = std::max( (uint32)(animal.Satiety / animal.MaxSatiety * ((float)ANIMAL_TYPE::MaxDbSatiety)), (uint32)(ANIMAL_TYPE::DbHungryValue+1) );
	}

	// Update the hunger bar (will not emit if the bar value is unchanged)
//	_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[petIndex].HUNGER, (sint64)hunger );
	CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(petIndex).setHUNGER(_PropertyDatabase, hunger );
}

/*
 * Init hunger database for all animals: needed for animals that are not spawned
 */
void CCharacter::initAnimalHungerDb()
{
	for (uint i = 0; i < _PlayerPets.size(); i++)
	{
		CPetAnimal& animal = _PlayerPets[i];
		if (animal.PetStatus == CPetAnimal::not_present)
			continue;

		const CStaticCreatures * form = CSheets::getCreaturesForm(animal.PetSheetId);
		if ( form )
		{
			// The pet type has to be up to date when the client receives the hunger value
			uint petType = EGSPD::getPetType( form->getRace() );
//			_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].TYPE, petType );
			CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setTYPE(_PropertyDatabase, petType );
		}
		updateAnimalHungerDb(i);
	}
}


/*
 * React to the hunger of the specified animal (precondition: petIndex<_PlayerPets.size())
 */
bool CCharacter::onAnimalHungry( uint petIndex, bool justBecameHungry )
{
	H_AUTO(CCharacter_onAnimalHungry);

	TLogContext_Item_Consume logContext(_Id);

	// Message for new hungry state
	if ( justBecameHungry )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "ANIMAL_HUNGRY" );
	}

	// Look for food in animal's inventory and consume part of it
	bool mustUpdateHungerDb = false;
	CPetAnimal& animal = _PlayerPets[petIndex];
	const sint MaxFoodItemsPerMeal = 99999;
	sint nbItemsLeftToConsume = MaxFoodItemsPerMeal;
	while ( nbItemsLeftToConsume > 0 )
	{
		uint32 itemSlot;
		CGameItemPtr foodItem = getItemByFamily( (INVENTORIES::TInventory)(INVENTORIES::pet_animal+petIndex), ITEMFAMILY::FOOD, itemSlot );
		if ( foodItem != NULL )
		{
			// Food found
			float caloriesPerUnit;
			sint nbUnits;
			caloriesPerUnit = foodItem->getStaticForm()->Calories; // see garanties of getItemByFamily()
			nbUnits = 1;

			// Increase satiety (not exceeding MaxSatiety)
			if ( nbUnits > nbItemsLeftToConsume )
				nbUnits = nbItemsLeftToConsume;
			float caloriesAvailable = caloriesPerUnit * (float)nbUnits;
			float caloriesNeeded = animal.MaxSatiety - animal.Satiety;
			if ( caloriesAvailable < caloriesNeeded )
			{
				// Consume all available in this item
				animal.Satiety += caloriesAvailable;
			}
			else
			{
				// Consume to full satiety (last useful unit is entirely consumed)
				animal.Satiety = animal.MaxSatiety;
				nbUnits = (sint)ceil(caloriesNeeded / caloriesPerUnit);
			}
			nbItemsLeftToConsume -= nbUnits;

			// Destroy items
			if (nbUnits >= (sint)foodItem->getStackSize())
				foodItem.deleteItem();
			else
				foodItem->setStackSize(foodItem->getStackSize() - nbUnits);

			// Satieted
			if ( ! (animal.Satiety < animal.MaxSatiety) )
				break;
		}
		else
		{
			// No more food
			break;
		}
	}
	if ( nbItemsLeftToConsume != MaxFoodItemsPerMeal )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "ANIMAL_HAS_EATEN" );
		mustUpdateHungerDb = true;
	}

	// Let the animal be slowed down (hungry) or unslowed down (fed) (not reemitted if MIRROR_ASSIGN_ONLY_IF_CHANGED)
	CMirrorPropValue<TYPE_FUEL> freeSpeedMode( TheDataset, animal.SpawnedPets, DSPropertyFUEL );
	freeSpeedMode = (animal.Satiety > 0);

	return mustUpdateHungerDb;
}


/*
 * Stop the specified animal if following the character and too far from him
 */
void CCharacter::checkAnimalInRange( uint petIndex )
{
	// Stop the animal while in follow mode and target too far
	CPetAnimal& animal = _PlayerPets[petIndex];
	if ( animal.IsFollowing )
	{
		// Uses animal's previous position (not a problem because client vision uses lagged position as well)
		float dxm = ((float)(animal.Landscape_X - getState().X)) * 0.001f;
		float dym = ((float)(animal.Landscape_Y - getState().Y)) * 0.001f;
		float sqrDistToTarget = dxm*dxm + dym*dym;
		if ( sqrDistToTarget > AnimalStopFollowingDistance.get() * AnimalStopFollowingDistance.get() )
		{
			sendPetCommand( CPetCommandMsg::STAND, petIndex, true ); // resets IsFollowing
			PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "ANIMAL_FOLLOW_TOO_FAR" );
		}
	}
}

/*
 * Set satiety directly (if corresponding petCreature is NULL, will do a lookup)
 */
void CCharacter::setAnimalSatiety( uint petIndex, float value, CCreature *petCreature )
{
	if ( (petIndex >= _PlayerPets.size()) ||
		 (_PlayerPets[petIndex].PetStatus == CPetAnimal::not_present) )
	{
		nlwarning( "Invalid pet index %u for setAnimalSatiety", petIndex );
		return;
	}

	H_AUTO(CCharacter_setAnimalSatiety);

	// Set value
	CPetAnimal& animal = _PlayerPets[petIndex];
	animal.Satiety = std::max( 0.0f, min( animal.MaxSatiety, value ) );

	// Export to mirror
	if (TheDataset.isAccessible(animal.SpawnedPets))
	{
		CMirrorPropValue<TYPE_FUEL> freeSpeedMode( TheDataset, animal.SpawnedPets, DSPropertyFUEL );
		freeSpeedMode = (animal.Satiety > 0);
	}

	// Export to database
	const CStaticCreatures * form;
	if ( petCreature )
		form = petCreature->getForm();
	else
		form = CSheets::getCreaturesForm( animal.PetSheetId );

	if ( form )
	{
		// The pet type has to be up to date when the client receives the hunger value
		uint petType = EGSPD::getPetType( form->getRace() );
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[petIndex].TYPE, petType );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(petIndex).setTYPE(_PropertyDatabase, petType );
	}
	updateAnimalHungerDb( petIndex );
}

/*
 * Set satiety to the max (animal won't be hungry anymore)
 */
void CCharacter::setAnimalSatietyToMax(uint petIndex)
{
	if (petIndex >= _PlayerPets.size() || _PlayerPets[petIndex].PetStatus == CPetAnimal::not_present)
	{
		nlwarning( "Invalid pet index %u for setAnimalSatietyToMax", petIndex );
		return;
	}

	setAnimalSatiety(petIndex, _PlayerPets[petIndex].MaxSatiety);
}


/*
 * Get the current and max satiety of the specified pet, or return false if the pet is not found
 */
bool CCharacter::getAnimalSatiety( uint petIndex, float& currentSatiety, float& maxSatiety )
{
	if ( (petIndex >= _PlayerPets.size()) ||
		 (_PlayerPets[petIndex].PetStatus == CPetAnimal::not_present) )
		return false;

	CPetAnimal& animal = _PlayerPets[petIndex];
	currentSatiety = animal.Satiety;
	maxSatiety = animal.MaxSatiety;
	return true;
}


/*
 * Returns true if animal is in the given stable
 */
bool CCharacter::isAnimalInStable(uint petIndex, uint16 stableId) const
{
	if (petIndex < _PlayerPets.size() && _PlayerPets[petIndex].PetStatus == CPetAnimal::stable)
	{
		if (uint16(_PlayerPets[petIndex].StableId) == stableId)
			return true;
	}
	return false;
}


//-----------------------------------------------
//	setAnimalPeople
//
// Set the race of the pet's owner (used to set size in the client)
//-----------------------------------------------
void CCharacter::setAnimalPeople( uint petIndex )
{
	CPetAnimal& animal = _PlayerPets[petIndex];

	CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId(animal.SpawnedPets) );
	if( c )
	{
		switch( _Race )
		{
			case EGSPD::CPeople::Fyros:
				c->setOwnerPeople( MOUNT_PEOPLE::Fyros );
				break;
			case EGSPD::CPeople::Matis:
				c->setOwnerPeople( MOUNT_PEOPLE::Matis );
				break;
			case EGSPD::CPeople::Tryker:
				c->setOwnerPeople( MOUNT_PEOPLE::Tryker );
				break;
			case EGSPD::CPeople::Zorai:
				c->setOwnerPeople( MOUNT_PEOPLE::Zorai );
				break;
			default:
				c->setOwnerPeople( MOUNT_PEOPLE::Unknown );
				break;
		}
	}
}


// Remove all spwanedpet (AIS shut down)
void CCharacter::removeSpawnedPet(NLNET::TServiceId aiServiceId)
{
	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		CPetAnimal &pa = _PlayerPets[ i ];

		// determine pet continent
		CVector pos;
		pos.x = pa.Landscape_X * 0.001f;
		pos.y = pa.Landscape_Y * 0.001f;
		pos.z = pa.Landscape_Z * 0.001f;
		uint32 continentId = _CurrentContinent;
		CContinent * continent = CZoneManager::getInstance().getContinent( pos );
		if( continent )
		{
			continentId = continent->getId();
		}
		uint32 aiInstance = CUsedContinent::instance().getInstanceForContinent( strlwr( CONTINENT::toString( (CONTINENT::TContinent) continentId ) ) );

		// retrieve the AI service ID for this continent
		NLNET::TServiceId aisId;
		CWorldInstances::instance().getAIServiceIdForInstance(aiInstance, aisId);

		if (aisId == aiServiceId)
		{
			// ok, this pet is on the AIS that juste stoped !, remove it
			_PlayerPets[ i ].SpawnedPets = TDataSetRow();
		}
	}
	updatePetDatabase();
}


/*
 * Return the index of the animal corresponding to the specified ticket item, or ~0 if not found
 */
uint CCharacter::getAnimalByTicket( CGameItemPtr item )
{
	uint32 PlayerPetsSize = (uint32)_PlayerPets.size();
	for( uint i=0; i!=PlayerPetsSize; ++i )
	{
		if( _PlayerPets[ i ].TicketPetSheetId != CSheetId::Unknown )
		{
			if( _PlayerPets[ i ].ItemPtr == item )
				return i;
		}
	}
	return ~0;
}


//-----------------------------------------------
// return true if corresponding pack animal is empty (return false for invalid index)
//-----------------------------------------------
bool CCharacter::checkPackAnimalEmptyInventory( uint32 petIndex )
{
	if ( !PackAnimalSystemEnabled )
		return false;

	// we check MAX_INVENTORY_ANIMAL and not MAX_PACK_ANIMAL as the inventory is created even for mount
	// if this change, this code need to be updated
	if ( ! ((petIndex < _PlayerPets.size()) && (petIndex < MAX_INVENTORY_ANIMAL)) )
	{
		nlwarning( "Invalid pack animal index" );
		return false;
	}

	uint32 packInv = INVENTORIES::pet_animal + petIndex;
	return _Inventory[ packInv ]->getUsedSlotCount() == 0;
}


//-----------------------------------------------------------------------------
bool CCharacter::petCommandDistance( uint32 beastIndex )
{
	if( beastIndex <= _PlayerPets.size() )
	{
		if( _PlayerPets[ beastIndex ].PetStatus == CPetAnimal::landscape || _PlayerPets[ beastIndex ].PetStatus == CPetAnimal::death )
		{
			if( TheDataset.isAccessible(_PlayerPets[ beastIndex ].SpawnedPets) )
			{
				CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ beastIndex ].SpawnedPets ) );
				if( c )
				{
					CVector2f animalPos;
					CVector2f characterPos;
					c->getState().getVector2f(animalPos);
					_EntityState.getVector2f(characterPos);
					float squareDistance = (animalPos - characterPos).sqrnorm();
					if( squareDistance <= 100.0f * 100.0f )
					{
						return true;
					}
				}
			}
		}
		else if( _PlayerPets[ beastIndex ].PetStatus == CPetAnimal::stable )
		{
			if( _CurrentStable == _PlayerPets[ beastIndex ].StableId )
				return true;
		}
	}
	return false;
}


//-----------------------------------------------------------------------------
bool CCharacter::petInventoryDistance( uint32 beastIndex )
{
	if ( beastIndex >= _PlayerPets.size() )
		return false;

	switch (_PlayerPets[ beastIndex ].PetStatus)
	{
	case CPetAnimal::landscape:
	case CPetAnimal::death:
		if( TheDataset.isAccessible(_PlayerPets[ beastIndex ].SpawnedPets) )
		{
			CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ beastIndex ].SpawnedPets ) );
			if( c )
			{
				CVector2f animalPos;
				CVector2f characterPos;
				c->getState().getVector2f(animalPos);
				_EntityState.getVector2f(characterPos);
				float squareDistance = (animalPos - characterPos).sqrnorm();
				if( squareDistance <= 30.0f * 30.0f )
				{
					return true;
				}
			}
		}
		break;

	case CPetAnimal::stable:
		{
			CStable::TStableData stable;
			if( CStable::getInstance()->getStableData( uint16(_PlayerPets[ beastIndex ].StableId), stable ) )
			{
				CVector2f animalPos( stable.StableExitX * 0.001f, stable.StableExitY * 0.001f );
				CVector2f characterPos;
				_EntityState.getVector2f(characterPos);
				float squareDistance = (animalPos - characterPos).sqrnorm();
				if( squareDistance <= 30.0f * 30.0f )
				{
					return true;
				}
			}
		}
		break;
	}
	return false;
}


//-----------------------------------------------
// remove pet from player corresponding to item and despawn it
//-----------------------------------------------
void CCharacter::removeAnimal( CGameItemPtr item, CPetCommandMsg::TCommand mode )
{
	uint32 PlayerPetsSize = (uint32)_PlayerPets.size();
	sint32 PackAnimalIndex = -1;
	for( uint i = 0; i < PlayerPetsSize; ++i )
	{
		if( _PlayerPets[ i ].ItemPtr == item )
		{
			removeAnimalIndex( i, mode );
		}
	}
}


//-----------------------------------------------
// remove pet from player corresponding to index and despawn it
//-----------------------------------------------
void CCharacter::removeAnimalIndex( uint32 beastIndex, CPetCommandMsg::TCommand commande )
{
	// make sure the index is in range
	BOMB_IF( beastIndex >= _PlayerPets.size(), "Ignoring removeAnimalIndex for animal beyond _PlayerPets.size(): "<<_Id.toString(), return );

	// if the animal is dead then just get rid of it
	if( _PlayerPets[ beastIndex ].PetStatus == CPetAnimal::death )
	{
		removePetCharacterAfterDeath( beastIndex );
		return;
	}

	CPetCommandMsg msg;
	msg.Command = commande;
	msg.CharacterMirrorRow = _EntityRowId;
	msg.PetMirrorRow = _PlayerPets[ beastIndex ].SpawnedPets;

	CCreature *c = NULL;

	switch( commande )
	{
		case CPetCommandMsg::GOTO_POINT_DESPAWN:
			{
				if( _CurrentStable == 0xFFFF )
				{
					return;
				}

				CStable::TStableData stable;
				BOMB_IF( !CStable::getInstance()->getStableData( _CurrentStable, stable ), NLMISC::toString("<CCharacter::removeAnimalIndex> Player is in a current stable %u but can't found it in CStable....", _CurrentStable), return );
				// do nothing if pet already in stable
				if ( _PlayerPets[ beastIndex ].PetStatus == CPetAnimal::stable )
					return;

				if( TheDataset.isAccessible(_PlayerPets[ beastIndex ].SpawnedPets) )
					c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ beastIndex ].SpawnedPets ) );
				if ( c )
				{
					// Ignore 'Enter Stable' commands if the distance is too high
					COfflineEntityState state =  c->getState();
					CVector2d destination( state.X, state.Y );
					CVector2d start( _EntityState.X, _EntityState.Y );
					float distance = (float)(start - destination).sqrnorm();
					if( distance > MaxAnimalCommandDistSquare * 1000 * 1000 )
						return;
				}

				_PlayerPets[ beastIndex ].PetStatus = CPetAnimal::stable;
				_PlayerPets[ beastIndex ].StableId = _CurrentStable;

				msg.Command = CPetCommandMsg::GOTO_POINT_DESPAWN;
				msg.Coordinate_X = stable.StableExitX;
				msg.Coordinate_Y = stable.StableExitY;
				msg.Coordinate_H = stable.StableExitZ;
				msg.Heading = stable.Theta;
			}
			break;

		case CPetCommandMsg::LIBERATE:
			removePetCharacterAfterDeath( (uint16)beastIndex );
			break;

		default:
			break;
	}

	uint32 AIInstanceId = getInstanceNumber();
	if( !c )
	{
		if( TheDataset.isAccessible(_PlayerPets[ beastIndex ].SpawnedPets) )
			c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ beastIndex ].SpawnedPets ) );
	}
	if( c )
	{
		CVector animalPos;
		c->getState().getVector(animalPos);
		uint32 continentId = _CurrentContinent;
		CContinent * continent = CZoneManager::getInstance().getContinent( animalPos );
		if( continent )
		{
			continentId = continent->getId();
		}
		AIInstanceId = CUsedContinent::instance().getInstanceForContinent( (CONTINENT::TContinent) continentId );
	}
	CWorldInstances::instance().msgToAIInstance(AIInstanceId, msg);

	_PlayerPets[ beastIndex ].SpawnedPets = TDataSetRow();
}


//-----------------------------------------------
// CCharacter::updatePetCoordinate Update coordinate for spawned pets
//-----------------------------------------------
void CCharacter::updatePetCoordinateAndDatabase()
{
	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		bool mustUpdateHungerDb = false;
		CPetAnimal& animal = _PlayerPets[ i ];
		if( animal.PetStatus != CPetAnimal::not_present )
		{
			if( animal.PetStatus == CPetAnimal::landscape )
			{
				if( TheDataset.isAccessible(animal.SpawnedPets) )
				{
					CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( animal.SpawnedPets ) );
					if( c )
					{
						// Update position evenly (not every game cycle)
						// updatePetCoordinateAndDatabase() is in the character tick update, which is alredy called every 16 game cycles
						// Compute hunger
						if ( animal.Satiety > 0 )
						{
							// If the animal moves, its speed may decrease its satiety
							sint32 deltaX = abs( c->getState().X() - animal.Landscape_X );
							sint32 deltaY = abs( c->getState().Y() - animal.Landscape_Y );
							if ( (deltaX > 10) && (deltaY > 10) )
							{
								if ( ! ((animal.Landscape_X == 0) && (animal.Landscape_Y == 0)) ) // avoid initialization of coordinates
								{
									H_AUTO(CCharacter_processPetCoordinateForHunger);
									float actualSpeedMM = ((float)sqrt((float)(deltaX*deltaX + deltaY*deltaY))) / (((float)CTickEventHandler::getGameTimeStep()) * ((float)CharacterTickUpdatePeriodGc));
									//nldebug( "actualSpeed = %.1f m/s", actualSpeed/1000.0f );
									float deltaSpeed = actualSpeedMM * 0.001f - c->getPhysScores().CurrentWalkSpeed;
									if ( deltaSpeed > 0 )
									{
										// Increase hunger
										TSatiety deltaSatiety = deltaSpeed * AnimalHungerFactor.get() * (float)CharacterTickUpdatePeriodGc;
										animal.Satiety -= deltaSatiety;
										mustUpdateHungerDb = true;

										// React to "hungry" state
										if ( animal.Satiety <= 0 )
										{
											animal.Satiety = 0;
											onAnimalHungry( i, true );
										}
									}

									// Stop if following and too far
									checkAnimalInRange( i );
								}
							}
						}
						else
						{
							// Check if the animal can eat some food
							mustUpdateHungerDb = onAnimalHungry( i, false );

							// Stop if following and too far
							checkAnimalInRange( i );
						}

						// Update position
						animal.Landscape_X = c->getState().X();
						animal.Landscape_Y = c->getState().Y();
						animal.Landscape_Z = c->getState().Z();

						// Check if pet is dead
						if( c->isDead() )
						{
							_PlayerPets[ i ].PetStatus = CPetAnimal::death;
							_PlayerPets[ i ].DeathTick = CTickEventHandler::getGameCycle();

							updateAnimalDespawnDb(i);

							SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
							params[0].Int = i + 1;
							sendDynamicSystemMessage( _Id, "ANIMAL_DEAD",params );
						}
					}
				}
			}
			else if( animal.PetStatus == CPetAnimal::death )
			{
				updateAnimalDespawnDb(i);

				if( ( _PlayerPets[ i ].DeathTick + SpawnedDeadMektoubDelay ) < CTickEventHandler::getGameCycle() )
				{
					removePetCharacterAfterDeath( i );

					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = i + 1;
					sendDynamicSystemMessage( _Id, "ANIMAL_DISEAPER",params );

				}
			}
			else if( animal.PetStatus == CPetAnimal::waiting_spawn && animal.spawnFlag )
			{
				if( !TheDataset.isAccessible(animal.SpawnedPets) )
				{
					animal.spawnFlag = false;
					spawnCharacterAnimal( i );
				}
			}
		}

		updateOnePetDatabase( i, mustUpdateHungerDb );
	}
}


//-----------------------------------------------
// CCharacter::updateAnimalDespawnDb
//-----------------------------------------------
void CCharacter::updateAnimalDespawnDb( uint petIndex )
{
	if( CTickEventHandler::getGameCycle() <= _PlayerPets[ petIndex ].DeathTick + 3 * 24 * 36000 )
	{
		uint32 timeBeforeDespawn = (_PlayerPets[ petIndex ].DeathTick + 3 * 24 * 36000) - CTickEventHandler::getGameCycle();
		uint32 timeBeforeDespawnDB = (uint32)( timeBeforeDespawn * (float)ANIMAL_TYPE::MaxDbTimeBeforeDespawn / (3 * 24 * 36000) );

		// Update the despawn bar
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[petIndex].DESPAWN, (sint64)timeBeforeDespawnDB );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(petIndex).setDESPAWN(_PropertyDatabase, checkedCast<uint8>(timeBeforeDespawnDB) );
	}
}


//-----------------------------------------------
// CCharacter::removePetCharacter Update database for spawned pets
//-----------------------------------------------
void CCharacter::removePetCharacterAfterDeath( uint32 index )
{
	TLogContext_Item_PetDespawn logContext(_Id);
	// pet founded
	if( index < MAX_INVENTORY_ANIMAL )
	{
		// reset despawn timer
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[index].DESPAWN, 0 );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(index).setDESPAWN(_PropertyDatabase, 0 );

		uint32 packInv = INVENTORIES::pet_animal + index;
		if( packInv < INVENTORIES::max_pet_animal )
		{
			CInventoryPtr petInv = _Inventory[packInv];
			if (petInv != NULL)
				petInv->clearInventory();
		}
	}
	else
	{
		nlwarning("<CCharacter::removePetCharacterAfterDeath> Invalid pack animal index, abort !");
	}

	// Send despawn pet command
	sendPetCommand( CPetCommandMsg::DESPAWN, index, true );

	// forget it
	if( _PlayerPets[ index ].ItemPtr != 0 )
	{
		uint32 slot = _PlayerPets[ index ].ItemPtr->getInventorySlot();

		// release our ref before we destroy the item
		_PlayerPets[ index ].ItemPtr = 0;

		destroyItem( INVENTORIES::bag, slot, 1 );
	}
	_PlayerPets[ index ].PetStatus = CPetAnimal::not_present;
	_PlayerPets[ index ].OwnerId = CEntityId::Unknown;
	_PlayerPets[ index ].PetSheetId = CSheetId::Unknown;
	_PlayerPets[ index ].TicketPetSheetId = CSheetId::Unknown;
	_PlayerPets[ index ].SpawnedPets = TDataSetRow();
}


//-----------------------------------------------
// CCharacter::updatePetDatabase Update database for spawned pets
//-----------------------------------------------
void CCharacter::updatePetDatabase()
{
	for( uint i = 0; i != MAX_INVENTORY_ANIMAL; ++i )
	{
		updateOnePetDatabase( i, false );
	}
}


//-----------------------------------------------
// CCharacter::updateOnePetDatabase Update database for a spawned pet
//-----------------------------------------------
void CCharacter::updateOnePetDatabase( uint petIndex, bool mustUpdateHungerDb )
{
	uint& i = petIndex;
	uint petType = 0;
	sint8 pcHp = 0;
	uint32 uid = CLFECOMMON::INVALID_CLIENT_DATASET_INDEX;
	uint64 pos = 0;
	uint32 bulkMax = 0;
	uint32 weightMax = 0;
	_PlayerPets[ i ].AnimalStatus = 0;

	if( _PlayerPets[ i ].PetStatus != CPetAnimal::not_present )
	{
		_PlayerPets[ i ].AnimalStatus = ANIMAL_STATUS::AliveFlag;

		const CStaticCreatures * form = CSheets::getCreaturesForm( _PlayerPets[ i ].PetSheetId );
		if( form )
		{
			petType = EGSPD::getPetType( form->getRace() );

			CSheetId creatureBag = CSheetId( form->getBagInventorySheet() );
			const CStaticItem * form2 = CSheets::getForm( creatureBag );
			if( form2 )
			{
				bulkMax = form2->BulkMax;
				weightMax = form2->WeightMax;
			}
		}

		switch( _PlayerPets[ i ].PetStatus )
		{
		case CPetAnimal::waiting_spawn:
			_PlayerPets[ i ].AnimalStatus = 0;
			pos = (((uint64)(_PlayerPets[ i ].Landscape_X)) << 32) + _PlayerPets[ i ].Landscape_Y;
			break;
		case CPetAnimal::stable:
			{
				CStable::TStableData stableData;
				if( CStable::getInstance()->getStableData( (uint16)_PlayerPets[ i ].StableId, stableData ) )
				{
					// NB : coordinate are first converted to uint32 prior to uint64 to prevent sign bit extension over 64 bits
					pos = (uint64(uint32(stableData.StableExitX)) << 32) + uint32(stableData.StableExitY);
					if( _PlayerPets[ i ].StableId == _CurrentStable )
					{
						_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::CanEnterLeaveStableFlag | ANIMAL_STATUS::InventoryAvailableFlag;
					}
					else if (AllowAnimalInventoryAccessFromAnyStable.get())
					{
						if (_PlaceOfCurrentStable != 0xFFFF)
							_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::InventoryAvailableFlag;
					}
					else
					{
						// from a stable, a player can only access inventory of animals which are in a stable of the same main place (same town)
						CPlace * place = CZoneManager::getInstance().getPlaceFromId(_PlaceOfCurrentStable);
						CVector stablePos(stableData.StableExitX/1000.f, stableData.StableExitY/1000.f, stableData.StableExitZ/1000.f);
						if (place != NULL && place->contains(stablePos))
						{
							_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::InventoryAvailableFlag;
						}
					}
				}
			}
			break;
		case CPetAnimal::landscape:
			_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::InLandscapeFlag;
			if( _CurrentStable != 0xFFFF )
			{
				_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::CanEnterLeaveStableFlag;
			}
			break;
		case CPetAnimal::death:
			_PlayerPets[ i ].AnimalStatus &= ~ANIMAL_STATUS::AliveFlag;
			_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::InLandscapeFlag;
			break;
		}

		if( TheDataset.isAccessible(_PlayerPets[ i ].SpawnedPets) )
		{
			CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ i ].SpawnedPets ) );
			if( c )
			{
				pcHp = (sint8) ( ( 127.0 * c->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) / c->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max );
				if( pcHp < 0 ) pcHp = 0;
				uid = c->getEntityRowId().getCompressedIndex();
				pos = (((uint64)c->getState().X) << 32) + c->getState().Y;
				CVector2f animalPos;
				CVector2f characterPos;
				c->getState().getVector2f(animalPos);
				_EntityState.getVector2f(characterPos);
				float squareDistance = (animalPos - characterPos).sqrnorm();
				if( squareDistance <= 30.0f * 30.0f )
				{
					_PlayerPets[ i ].AnimalStatus |= ANIMAL_STATUS::InventoryAvailableFlag;
				}
			}
		}
	}
	if (   _PlayerPets[ i ].PetStatus   != CPetAnimal::not_present /* Need to update */
		|| _PlayerPets[ i ].DbPetStatus != _PlayerPets[ i ].PetStatus /* db expects update */ )
	{
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].TYPE,     petType );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setTYPE(_PropertyDatabase, petType );
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].UID,      uid );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setUID(_PropertyDatabase, uid );
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].STATUS,   _PlayerPets[ i ].AnimalStatus );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setSTATUS(_PropertyDatabase, checkedCast<uint8>(_PlayerPets[ i ].AnimalStatus) );
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].HP,       pcHp );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setHP(_PropertyDatabase, pcHp );
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].BULK_MAX, bulkMax / 1000 );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setBULK_MAX(_PropertyDatabase, bulkMax / 1000 );
//		_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].POS,      pos );
		CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setPOS(_PropertyDatabase, pos );

		if (_PlayerPets[ i ].PetStatus == CPetAnimal::not_present)
//			_PropertyDatabase.setProp( _DataIndexReminder->PACK_ANIMAL.BEAST[i].DESPAWN, 0 );
			CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(i).setDESPAWN(_PropertyDatabase, 0 );

		if ( mustUpdateHungerDb )
		{
			updateAnimalHungerDb( i ); // petType must always been set before
		}
		_PlayerPets[ i ].DbPetStatus = _PlayerPets[ i ].PetStatus;
	}
}


//-----------------------------------------------
// CCharacter::getPlayerPet return the index of a player pet, or -1 if not found
//-----------------------------------------------
sint32 CCharacter::getPlayerPet( const TDataSetRow& petRowId ) const
{
	if ( ! TheDataset.isAccessible( petRowId ) )
		return -1;

	for( uint i = 0; i < _PlayerPets.size(); ++i )
	{
		if( _PlayerPets[ i ].PetStatus == CPetAnimal::landscape )
		{
			if( _PlayerPets[ i ].SpawnedPets == petRowId )
			{
				return (sint32)i;
			}
		}
	}
	return -1;
}


//-----------------------------------------------
// CCharacter::sendPetCommand character want send a command to his pet
//-----------------------------------------------
void CCharacter::sendPetCommand( CPetCommandMsg::TCommand command, uint PetIndex, bool bypassDistanceCheck )
{
	if( PetIndex < _PlayerPets.size() )
	{
		if( _PlayerPets[ PetIndex ].PetStatus == CPetAnimal::landscape || _PlayerPets[ PetIndex ].PetStatus == CPetAnimal::death )
		{
			if( TheDataset.isAccessible(_PlayerPets[ PetIndex ].SpawnedPets) )
			{
				CPetCommandMsg msg;
				msg.Command = command;
				msg.CharacterMirrorRow = _EntityRowId;
				msg.PetMirrorRow = _PlayerPets[ PetIndex ].SpawnedPets;
				msg.Coordinate_X = _EntityState.X; // not used
				msg.Coordinate_Y = _EntityState.Y; // by a
				msg.Coordinate_H = _EntityState.Z; // FOLLOW command

				uint32 AIInstanceId = getInstanceNumber();
				CCreature * c = CreatureManager.getCreature( TheDataset.getEntityId( _PlayerPets[ PetIndex ].SpawnedPets ) );
				if( c )
				{
					if ( ! bypassDistanceCheck )
					{
						// Ignore certain commands if the distance is too high
						switch ( command )
						{
						case CPetCommandMsg::STAND:
						case CPetCommandMsg::FOLLOW:
							{
								COfflineEntityState state =  c->getState();
								CVector2d destination( state.X, state.Y );
								CVector2d start( _EntityState.X, _EntityState.Y );
								float distance = (float)(start - destination).sqrnorm();
								if( distance > MaxAnimalCommandDistSquare * 1000 * 1000 )
									return;
							}
							break;
						default:;
						}
					}

					CVector animalPos;
					c->getState().getVector(animalPos);
					uint32 continentId = _CurrentContinent;
					CContinent * continent = CZoneManager::getInstance().getContinent( animalPos );
					if( continent )
					{
						continentId = continent->getId();
					}
					AIInstanceId = CUsedContinent::instance().getInstanceForContinent( (CONTINENT::TContinent) continentId );
				}
				CWorldInstances::instance().msgToAIInstance(AIInstanceId, msg);

				switch ( command )
				{
				case CPetCommandMsg::FOLLOW: // set following state
					_PlayerPets[ PetIndex ].IsFollowing = true;
					break;
				case CPetCommandMsg::DESPAWN: // don't change following state (e.g. while teleporting)
					break;
				default: // unset following state
					_PlayerPets[ PetIndex ].IsFollowing = false;
				}
			}
		}
	}
}


//-----------------------------------------------
// CCharacter::sendPackAnimalCommand send pack animal command
//-----------------------------------------------
void CCharacter::sendAnimalCommand( uint8 petIndexCode, uint8 command )
{
	DROP_IF( !PackAnimalSystemEnabled, "Ignoring action on pack animals because pack animal suystem is not enabled: "<<_Id.toString(), return );

	// calculate the size of the pets container for future use
	uint32 numPets= (uint32)_PlayerPets.size();

	// make sure the petIndexCode is valid (should be >=0 and <= _PlayerPets.size())
	BOMB_IF(petIndexCode>numPets,"Ignoring action by player "<<_Id.toString()<<" on pet with invalid index",return);

	// decode the pet index info
	bool applyToAllPets= (petIndexCode==0);
	uint32 firstPetIndex= (!applyToAllPets)? petIndexCode-1: 0;
	uint32 lastPetIndex=  (!applyToAllPets)? petIndexCode-1: numPets-1;


	// mounting and dismounting are special cases so we manage them first
	switch( (ANIMALS_ORDERS::EBeastOrder) command )
	{
		case ANIMALS_ORDERS::MOUNT:
			{
				BOMB_IF( applyToAllPets, "Ignoring attempt to mount all of my pets at once! : "<<_Id.toString(), return );
				BOMB_IF( _PlayerPets[ firstPetIndex ].PetStatus != CPetAnimal::landscape, "Ignoring attempt to mount pet because landscape test failed : "<<_Id.toString(), return );
				DROP_IF( petCommandDistance( firstPetIndex ) == false, "Ignoring attempt to mount pet because distance check failed : "<<_Id.toString(), return );

				// test player isn't using a TP
				if (getTpTicketSlot() != INVENTORIES::INVALID_INVENTORY_SLOT)
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "MOUNT_CANT_WHILE_TP" );
				}
				else
				{
					// add mount phrase in manager
					static CSheetId mountBrick("bapa03.sbrick");
					vector<CSheetId> bricks;
					bricks.push_back(mountBrick);
					CPhraseManager::getInstance().executePhrase(_EntityRowId, _PlayerPets[ firstPetIndex ].SpawnedPets, bricks);
				}
			}
			return;

		case ANIMALS_ORDERS::UNMOUNT:
			{
				static CSheetId unmountBrick("bapa04.sbrick");
				vector<CSheetId> bricks;
				bricks.push_back(unmountBrick);
				CPhraseManager::getInstance().executePhrase(_EntityRowId, _EntityRowId, bricks);
			}
			return; // bypass command to the AIS
	}

	for (uint32 petIndex= firstPetIndex; petIndex<= lastPetIndex; ++petIndex)
	{
		// if the player doesn't have a pet at this index then just continue
		if ( _PlayerPets[ petIndex ].PetStatus == CPetAnimal::not_present )
		{
			continue;
		}

		// make sure that the player is close enough to the pet to perform the requested action
		if( petCommandDistance( petIndex ) == false && ((ANIMALS_ORDERS::EBeastOrder)command) != ANIMALS_ORDERS::FREE )
		{
			continue;
		}

		CPetCommandMsg::TCommand petCommand;

		switch( (ANIMALS_ORDERS::EBeastOrder) command )
		{
			case ANIMALS_ORDERS::ENTER_STABLE:
				if( _PlayerPets[ petIndex ].IsMounted  )
					continue;

				petCommand = CPetCommandMsg::GOTO_POINT_DESPAWN;
				break;

			case ANIMALS_ORDERS::FREE:
				if( _PlayerPets[ petIndex ].IsMounted  )
					continue;

				petCommand= CPetCommandMsg::LIBERATE;
				break;

			case ANIMALS_ORDERS::LEAVE_STABLE:
				spawnCharacterAnimal( petIndex );
				// no petCommand setup here so continue instead of break
				continue;

			case ANIMALS_ORDERS::STOP:
				petCommand = CPetCommandMsg::STAND;
				break;

			case ANIMALS_ORDERS::FOLLOW:
				petCommand = CPetCommandMsg::FOLLOW;
				break;

			default:
				BOMB( NLMISC::toString("<CCharacter::sendPackAnimalCommand> Command %u unknown", command), return);
		}

		switch( petCommand )
		{
			case CPetCommandMsg::GOTO_POINT_DESPAWN: // here we add a test on death status
					if( _PlayerPets[ petIndex ].PetStatus != CPetAnimal::death )
					{
						removeAnimalIndex( petIndex, petCommand );
					}
				break;

			case CPetCommandMsg::DESPAWN:
			case CPetCommandMsg::LIBERATE:
					removeAnimalIndex( petIndex, petCommand );
				break;

			default:
				sendPetCommand( petCommand, petIndex );
		}
	}
}

void CCharacter::setAnimalName( uint8 petIndex, ucstring customName )
{
	if (petIndex < 0 || petIndex >= MAX_INVENTORY_ANIMAL)
	{
		nlwarning("<CCharacter::setAnimalName> Incorect animal index '%d'.", petIndex);
		return;
	}
	CPetAnimal& animal = _PlayerPets[petIndex];

	animal.setCustomName(customName);
	sendPetCustomNameToClient(petIndex);

	if ( ! customName.empty())
	{
		TDataSetRow row = animal.SpawnedPets;
		NLNET::CMessage	msgout("CHARACTER_NAME");
		msgout.serial(row);
		msgout.serial(customName);
		sendMessageViaMirror("IOS", msgout);
	}
}

//-----------------------------------------------------------------------------
void CCharacter::sendPetCustomNameToClient(uint8 petIndex)
{
	uint32 textId = 0;
	if (_PlayerPets[petIndex].CustomName.length() > 0)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
		params[0].Literal= _PlayerPets[petIndex].CustomName;
		uint32 userId = PlayerManager.getPlayerId(_Id);
		textId = STRING_MANAGER::sendStringToUser(userId, "LITERAL", params);
	}
	CBankAccessor_PLR::getPACK_ANIMAL().getBEAST(petIndex).setNAME(_PropertyDatabase, textId);
}


//-----------------------------------------------
//		addCatalyserXpBonus
//-----------------------------------------------
bool CCharacter::addCatalyserXpBonus( uint32& slot, SSkill * skill, double xpGain, double& xpBonus, uint32& stackSizeToRemove,
									 uint32& catalyserLvl, uint32& catalyserCount )
{
	CGameItemPtr item = _Inventory[INVENTORIES::bag]->getItem( slot );
	if ( item != NULL )
	{
		const CStaticItem * form = item->getStaticForm();
		if ( form != NULL )
		{
			catalyserLvl = item->quality();
			if( catalyserLvl > (uint32)skill->Base )
			{
				if( form->XpCatalyser )
				{
					uint32 stackSizeFromSheet = (uint32)((xpGain*100)/form->XpCatalyser->XpBonus); // 100 because of display scale(see txt progress normal gain)
					uint32 stackSizeFromInventory = item->getStackSize();
					stackSizeToRemove = min(stackSizeFromSheet,stackSizeFromInventory);
					CInventoryBase::TInventoryOpResult result;
					_Inventory[INVENTORIES::bag]->removeItem(slot, stackSizeToRemove, &result);
					if( result == CInventoryBase::ior_ok )
					{
						xpBonus = (stackSizeToRemove * form->XpCatalyser->XpBonus) / 100.;

						if( stackSizeToRemove == stackSizeFromInventory )
						{
							slot = INVENTORIES::INVALID_INVENTORY_SLOT;
							PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "XP_CATALYSER_NO_MORE_ACTIVE");
						}
						catalyserCount = stackSizeFromInventory-stackSizeToRemove;
						return true;
					}
					else
					{
						nlwarning("<CCharacter::addCatalyserXpBonus>%s can't remove %d catalysers in slot %d",_Id.toString().c_str(),stackSizeToRemove,_XpCatalyserSlot);
					}
				}
				else
				{
					nlwarning("<CCharacter::addCatalyserXpBonus> Trying to consume an xp catalyser but it's not an xp catalyser !!");
				}
			}
		}
		else
		{
			nlwarning("<CCharacter::addCatalyserXpBonus>%s item in slot %u has a NULL form count = %u",_Id.toString().c_str(),slot,_Inventory[INVENTORIES::bag]->getSlotCount());
		}
	}
	else
	{
		nlwarning("<CCharacter::addCatalyserXpBonus>%s NULL item in slot %u count = %u",_Id.toString().c_str(),slot,_Inventory[INVENTORIES::bag]->getSlotCount());
	}

	return false;

} // addCatalyserXpBonus



//---------------------------------------------------
// addXpToSkillInternal : Add amount of xp gain to a skill
//---------------------------------------------------
double CCharacter::addXpToSkillInternal( double XpGain, const std::string& ContSkill, TAddXpToSkillMode addXpMode, std::map<SKILLS::ESkills,CXpProgressInfos> &gainBySkill )
{
	H_AUTO(CCharacter_addXpToSkill);

	// if no remaining XPGain, return
	if (XpGain == 0.0f)
		return 0.0;

	// get pointer to static skills tree definition
	CSheetId sheet("skills.skill_tree");
	const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );
	nlassert( SkillsTree );

	// convert the skill name to enum entry
	string skillName = ContSkill;
	SKILLS::ESkills skillEnum = SKILLS::toSkill( skillName );
	BOMB_IF( skillEnum == SKILLS::unknown, "CCharacter::addXpToSkill() FAILED for unknown skill "<<ContSkill, return 0.0 ); //Normaly not happens, unless skills tree not ready

	// Find compatible unlocked skill
	while( _Skills.getSkillStruct( skillName )->Base == 0 && SkillsTree->SkillsTree[ skillEnum ].ParentSkill != SKILLS::unknown )
	{
		skillEnum = SkillsTree->SkillsTree[ skillEnum ].ParentSkill;
		skillName = SKILLS::toString( skillEnum );
	}

	// get a pointer to skill structure
	SSkill * skill = _Skills.getSkillStruct( skillName );
	nlassert( skill );
	nlassert( skillEnum != SKILLS::unknown );

	// treat ring scenarios as a special case...
	if(IsRingShard)
	{
		// don't gain reward points for crafting in the ring
		DROP_IF(reinterpret_cast<CSString*>(&skillName)->left(2).toUpper()=="SC","No XP gain for using crafting skill "<<skillName<<"("<<ContSkill<<") for character "<<getId().toString(),return 0.0);

		// skills are built up as strings with 1 letter per skill level following the same pattern as the xpLevel system so SM would be LEVEL_A, SMOEAEM would be LEVEL_6
		R2::TSessionLevel xpLevel;
		xpLevel.fromSkillName(skillName);

		// deal with the xp gain in the ring reward points system and drop out
		RingRewardPoints.addXp(xpLevel,(uint32)(XpGain*100.0));
		return 0.0;
	}

	// prevent SP Gain on skills already maxed
	// if a skill already has unlocked children skills, return doing nothing
	if (skill->MaxLvlReached > skill->Base)
	{
		return 0.0;
	}

	// check whether this character is on a free trial account
	bool bFreeTrialLimitReached = false;
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
	BOMB_IF(p == NULL,"Failed to find player record for character: "<<getId().toString(),return 0.0);
	if (p->isTrialPlayer())
	{
		// prevent free trial players from gaining XP beyond level FreeTrialSkillLimit
		if (skill->Base >= (sint32)(uint32)FreeTrialSkillLimit)
		{
			// send a 'You gain 0 XP. In order to progress further in this skill you must upgrade your free trial account' message to the client
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::skill);
			params[0].Enum = skillEnum;
			PHRASE_UTILITIES::sendDynamicSystemMessage(getEntityRowId(), "PROGRESS_FREE_TRIAL_LIMIT", params);
			bFreeTrialLimitReached = true;
		}
	}


	double xpBonus = 0;
	uint32 stackSizeToRemove = 0;
	uint32 catalyserLvl = 0;
	uint32 catalyserCount = 0;

	double ringXpBonus = 0;
	uint32 ringStackSizeToRemove = 0;
	uint32 ringCatalyserLvl = 0;
	uint32 ringCatalyserCount = 0;

	// Don't take away cats if free trial limit reached and there is no DP.
	bool bConsumeCats = ! ( bFreeTrialLimitReached && _DeathPenalties->isNull() );

	if( bConsumeCats && (addXpMode != AddXpToSkillBranch) )
	{
		if( _XpCatalyserSlot != INVENTORIES::INVALID_INVENTORY_SLOT )
		{
			if( addCatalyserXpBonus( _XpCatalyserSlot, skill, XpGain, xpBonus, stackSizeToRemove, catalyserLvl, catalyserCount ) )
			{
//				_PropertyDatabase.setProp( "CHARACTER_INFO:XP_CATALYSER:Count", catalyserCount );
				CBankAccessor_PLR::getCHARACTER_INFO().getXP_CATALYSER().setCount(_PropertyDatabase, checkedCast<uint16>(catalyserCount) );
			}
		}

		if( _RingXpCatalyserSlot != INVENTORIES::INVALID_INVENTORY_SLOT )
		{
			if( addCatalyserXpBonus( _RingXpCatalyserSlot, skill, XpGain, ringXpBonus, ringStackSizeToRemove, ringCatalyserLvl, ringCatalyserCount ) )
			{
//				_PropertyDatabase.setProp( "CHARACTER_INFO:RING_XP_CATALYSER:Count", ringCatalyserCount );
				CBankAccessor_PLR::getCHARACTER_INFO().getRING_XP_CATALYSER().setCount(_PropertyDatabase, checkedCast<uint16>(ringCatalyserCount) );
			}
		}

		if (!p->isTrialPlayer())
		{
			xpBonus = XpGain;
		}
	}

	XpGain += xpBonus + ringXpBonus;

	// update death penalty
	_DeathPenalties->addXP( *this, skillEnum, XpGain);

	if (bFreeTrialLimitReached)
	{
		return 0.0;
	}

	// if no remaining XPGain, return
	if (XpGain == 0.0f)
		return 0.0;

	// get pointer on static xp tables definition
	sheet = CSheetId("xptable.xp_table");
	const CStaticXpStagesTable* XpStageTable = CSheets::getXpStageTableForm( sheet );
	nlassert( XpStageTable );

	// get pointer on xp table stage part
	const CStaticXpStagesTable::SXpStage * stage = XpStageTable->getXpStage( skill->Base, SkillsTree->SkillsTree[ skillEnum ].StageType );
	// If stage is zero, they are no more stage, max lvl reached
	if( stage == 0 ) return 0.0;

#ifdef _DEBUG_SKILL_PROGRESSION
	egs_chinfo("<CCharacter::addXpToSkill> Skill %s gain %f xp", skillName.c_str(), XpGain );
#endif

	// check if skills limiters is reached
	string skillTypeName = SKILLS::toString( skillEnum ).substr(1,1);
	nlassert( !skillTypeName.empty() );
	switch( skillTypeName[0] )
	{
	case 'F':
		if( skill->Base >= (sint32)SkillFightValueLimiter )
			return 0.0;
		break;
	case 'M':
		if( skill->Base >= (sint32)SkillMagicValueLimiter )
			return 0.0;
		break;
	case 'C':
		if( skill->Base >= (sint32)SkillCraftValueLimiter )
			return 0.0;
		break;
	case 'H':
		if( skill->Base >= (sint32)SkillHarvestValueLimiter )
			return 0.0;
		break;
	default:
		nlstop;
	}

	// Increment XP
	skill->Xp += XpGain;

	// For AddXpToSkillBranch mode
	double	XpGainTruncated= XpGain;
	double	XpGainRemainder= 0.0;
	if( addXpMode==AddXpToSkillBranch && skill->Xp > stage->XpForPointSkill )
	{
		XpGainTruncated= stage->XpForPointSkill - (skill->Xp - XpGain);
		XpGainRemainder= XpGain - XpGainTruncated;
		// Yoyo: avoid any precision problems and infinite loop in addXpToSkillBranch, truncate under some value
		XpGainTruncated= max(0.0, XpGainTruncated);
		if(XpGainRemainder < 0.01)
			XpGainRemainder= 0.0;
	}

	// output stats
	//Bsi.append( StatPath, NLMISC::toString("[XP] %s %.3f %.3f %s %d %d", _Id.toString().c_str(), XpGain, skill->Xp, Skill.c_str(), skill->Base, stage->XpForPointSkill) );
	//EgsStat.displayNL("[XP] %s %.3f %.3f %s %d %d", _Id.toString().c_str(), XpGain, skill->Xp, Skill.c_str(), skill->Base, stage->XpForPointSkill);
//	EGSPD::xPGained(_Id, float(XpGain), float(skill->Xp), skillName, skill->Base, stage->XpForPointSkill);

//	log_Character_XPGain(_Id, skillName, skill->Base, float(XpGain));

	// bufferize xp gain, store it in paramet map
	if (addXpMode==AddXpToSkillBuffer)
	{
		map<SKILLS::ESkills,CXpProgressInfos>::iterator itGainSkill = gainBySkill.find( skillEnum );
		if( itGainSkill != gainBySkill.end() )
		{
			(*itGainSkill).second.TotalXpGain += XpGain;
			(*itGainSkill).second.XpBonus += xpBonus;
			(*itGainSkill).second.CatalyserCount += stackSizeToRemove;
			(*itGainSkill).second.CatalyserLvl = catalyserLvl;
			(*itGainSkill).second.RingXpBonus += ringXpBonus;
			(*itGainSkill).second.RingCatalyserCount += ringStackSizeToRemove;
			(*itGainSkill).second.RingCatalyserLvl = ringCatalyserLvl;
		}
		else
		{
			CXpProgressInfos xpInf;
			xpInf.TotalXpGain = XpGain;
			xpInf.XpBonus = xpBonus;
			xpInf.CatalyserCount = stackSizeToRemove;
			xpInf.CatalyserLvl = catalyserLvl;
			xpInf.RingXpBonus = ringXpBonus;
			xpInf.RingCatalyserCount = ringStackSizeToRemove;
			xpInf.RingCatalyserLvl = ringCatalyserLvl;
			gainBySkill.insert( make_pair(skillEnum,xpInf) );
		}
	}
	// report to client xp gain
	else
	{
		if( (xpBonus+ringXpBonus) > 0 )
		{
			SM_STATIC_PARAMS_3(paramsP, STRING_MANAGER::skill, STRING_MANAGER::integer, STRING_MANAGER::integer);
			paramsP[0].Enum = skillEnum;
			paramsP[1].Int = max((sint32)1, sint32(100*XpGain) );
			paramsP[2].Int = max((sint32)1, sint32(100*(XpGain - xpBonus - ringXpBonus)));
			PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "XP_CATALYSER_PROGRESS_NORMAL_GAIN", paramsP);

			if( xpBonus > 0 )
			{
				SM_STATIC_PARAMS_2(paramsC, STRING_MANAGER::integer, STRING_MANAGER::integer);
				paramsC[0].Int = stackSizeToRemove;
				paramsC[1].Int = catalyserLvl;
				PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "XP_CATALYSER_CONSUME", paramsC);
			}

			if( ringXpBonus > 0 )
			{
				SM_STATIC_PARAMS_2(paramsC, STRING_MANAGER::integer, STRING_MANAGER::integer);
				paramsC[0].Int = ringStackSizeToRemove;
				paramsC[1].Int = ringCatalyserLvl;
				PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "XP_CATALYSER_CONSUME", paramsC);
			}
		}
		else
		{
			SM_STATIC_PARAMS_2(params, STRING_MANAGER::skill, STRING_MANAGER::integer);
			params[0].Enum = skillEnum;
			// Use XpGainTruncated to make it works correctly in AddXpToSkillBranch mode
			// NB: don't need to use XpGainTruncated for Buffer and simple mode (with XpCat) since the Xp gain is stopped on the current skill
			params[1].Int = max((sint32)1, sint32(100*XpGainTruncated) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "PROGRESS_NORMAL_GAIN", params);
		}
	}

	// check if xp gain reach amount xp needed for skill progress
	if( skill->Xp >= stage->XpForPointSkill )
	{
#ifdef _DEBUG_SKILL_PROGRESSION
		egs_chinfo("<CCharacter::addXpToSkill> Skill %s Reach needed amount xp ( xp %f / xp Needed %d ) for progress", skillName.c_str(), skill->Xp, stage->XpForPointSkill );
#endif
		// get skill type
		// TSPType enumerates the different types of skill points
		EGSPD::CSPType::TSPType SPType = EGSPD::CSPType::EndSPType; // initialize to the last enum in the skill type list
		string testStr = ContSkill.substr(0,2);						// do a test on the first two letters of the skill string
		if (nlstricmp(testStr,"SF") == 0)							// derive the category of skill
			SPType = EGSPD::CSPType::Fight;
		else if (nlstricmp(testStr,"SM") == 0)
			SPType = EGSPD::CSPType::Magic;
		else if (nlstricmp(testStr,"SC") == 0)
			SPType = EGSPD::CSPType::Craft;
		else if (nlstricmp(testStr,"SH") == 0)
			SPType = EGSPD::CSPType::Harvest;

		if (SPType != EGSPD::CSPType::EndSPType)
		{
			// add Sp gain
			addSP( stage->SpPointMultiplier, SPType);
		}

		_HaveToUpdateItemsPrerequisit = true;

		// check if skill reach is max value
		if( skill->Base < SkillsTree->SkillsTree[ skillEnum ].MaxSkillValue )
		{
			skill->Base += 1;
#ifdef _DEBUG_SKILL_PROGRESSION
			egs_chinfo("<CCharacter::addXpToSkill> Skill %s progress to value %d ( MaxValue %d)", skillName.c_str(), skill->Base, SkillsTree->SkillsTree[ skillEnum ].MaxSkillValue );
#endif
			skill->Current = skill->Base + skill->Modifier;
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.BaseSkill[skillEnum], skill->Base );
			CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(skillEnum).setBaseSKILL(_PropertyDatabase, checkedCast<uint16>(skill->Base) );

			//skill->Xp -= stage->XpForPointSkill;
			skill->Xp = 0.0f;

			// update skill level information
			if( skill->MaxLvlReached < skill->Base )
			{
				skill->MaxLvlReached = skill->Base;
			}

			// update all parent skill with new max children
			SKILLS::ESkills skillUpdated = skillEnum;
			while( SkillsTree->SkillsTree[ skillUpdated ].ParentSkill != SKILLS::unknown )
			{
				if( _Skills._Skills[ skillUpdated ].MaxLvlReached > _Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached )
				{
					_Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached = _Skills._Skills[ skillUpdated ].MaxLvlReached;
					skillUpdated = SkillsTree->SkillsTree[ skillUpdated ].ParentSkill;
				}
				else
				{
					break;
				}
			}

			//update stage pointer (for case of skill have changed of stage segment)
			const CStaticXpStagesTable::SXpStage * stageTmp = XpStageTable->getXpStage( skill->Base , SkillsTree->SkillsTree[ skillEnum ].StageType );
			if( stageTmp != 0 ) stage = stageTmp;
			skill->XpNextLvl = stage->XpForPointSkill;

			// update magic resist level
			updateMagicProtectionAndResistance();

			// update the best skill if needed
			if ( (_BestSkill == SKILLS::unknown) || (skill->Base > _Skills._Skills[_BestSkill].Base) )
			{
				_BestSkill = skillEnum;

				// need to re-compute resist modifiers as best skill has changed
				updateMagicProtectionAndResistance();
			}
			else if (skillEnum == _BestSkill)
			{
				// need to re-compute resist modifiers as best skill level has changed
				updateMagicProtectionAndResistance();
			}

			// update best skill for dodge if needed
			const sint32 dodgeVal = getSkillEquivalentDodgeValue(skillEnum);
			if ( dodgeVal > _BaseDodgeLevel )
			{
				_SkillUsedForDodge = skillEnum;
				_BaseDodgeLevel = dodgeVal;
				_CurrentDodgeLevel = max(sint32(0), dodgeVal + _DodgeModifier);
//				_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeBase, _BaseDodgeLevel);
				CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseDodgeLevel));
//				_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeCurrent, _CurrentDodgeLevel);
				CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentDodgeLevel));
			}

			// update parry level if needed
			// check if skill is a parent of parry skill
			const string parrySkillStr = SKILLS::toString(_CurrentParrySkill);
			bool parry = false;
			if (parrySkillStr.size() >= ContSkill.size())
			{
				string testStr = parrySkillStr.substr(0,ContSkill.size());
				if (nlstricmp(testStr,ContSkill) == 0)
					parry = true;
			}
			else
			{
				string testStr = ContSkill.substr(0,parrySkillStr.size());
				if (nlstricmp(testStr,parrySkillStr) == 0)
					parry = true;
			}
			if (parry)
			{
				_BaseParryLevel = skill->Base;
				_CurrentParryLevel = max( sint32(0), _BaseParryLevel + _ParryModifier);

//				_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryBase, _BaseParryLevel);
				CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseParryLevel));
//				_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel );
				CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel) );
			}

			SM_STATIC_PARAMS_2(params, STRING_MANAGER::skill, STRING_MANAGER::integer);
			params[0].Enum = skillEnum;
			params[1].Int = skill->Base;
			PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "PROGRESS_SKILL", params);

			// inform mission system.
			sint32 skillMission = getBestChildSkillValue(skillEnum);
			CMissionEventSkillProgress event( skillEnum, skillMission );
			processMissionEvent(event);

			// output stats
			//Bsi.append( StatPath, NLMISC::toString("[SKILLP] %s %s %d", _Id.toString().c_str(), skillName.c_str(), skill->Base) );
			//EgsStat.displayNL("[SKILLP] %s %s %d", _Id.toString().c_str(), skillName.c_str(), skill->Base);
//			EGSPD::skillProgessed(_Id, skillName, skill->Base);
			log_Character_LevelUp(_Id, skillName, skill->Base);
			// resplenish the enrgies as player gets a new level
			_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Max;
			_PhysScores._PhysicalScores[ SCORES::stamina ].Current = _PhysScores._PhysicalScores[ SCORES::stamina ].Max;
			_PhysScores._PhysicalScores[ SCORES::sap ].Current = _PhysScores._PhysicalScores[ SCORES::sap ].Max;
			_PhysScores._PhysicalScores[ SCORES::focus ].Current = _PhysScores._PhysicalScores[ SCORES::focus ].Max;
		}
		else
		{
			//check if skill have children skill
			if ( ! SkillsTree->SkillsTree[ skillEnum ].ChildSkills.empty() )
			{
				for( vector<SKILLS::ESkills>::const_iterator it = SkillsTree->SkillsTree[ skillEnum ].ChildSkills.begin(); it != SkillsTree->SkillsTree[ skillEnum ].ChildSkills.end(); ++it )
				{
					if( _Skills._Skills[ *it ].Base == 0 )
					{
						_Skills._Skills[ *it ].Base = skill->Base + 1;
						if(_Skills._Skills[ *it ].MaxLvlReached < (skill->Base + 1))
							_Skills._Skills[ *it ].MaxLvlReached = skill->Base + 1;

						_Skills._Skills[ *it ].Current = _Skills._Skills[ *it ].Base + _Skills._Skills[ *it ].Modifier;
						const CStaticXpStagesTable::SXpStage * stageTmp = XpStageTable->getXpStage( (skill->Base + 1), SkillsTree->SkillsTree[ *it ].StageType );
						if( stageTmp != 0 ) stage = stageTmp;
						_Skills._Skills[ *it ].Xp = 0;
						_Skills._Skills[ *it ].XpNextLvl = stage->XpForPointSkill;
//						_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.Skill[*it], _Skills._Skills[ *it ].Current );
						CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(*it).setSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ *it ].Current) );
//						_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.BaseSkill[*it], _Skills._Skills[ *it ].Base );
						CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(*it).setBaseSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ *it ].Base) );
						if( _Skills._Skills[ *it ].xpNextLvl > 0 )
						{
//							_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.ProgressBar[*it], (uint8) ( _Skills._Skills[ *it ].Xp * 255 / _Skills._Skills[ *it ].XpNextLvl ) );
							CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(*it).setPROGRESS_BAR(_PropertyDatabase, checkedCast<uint8>( _Skills._Skills[ *it ].Xp * 255 / _Skills._Skills[ *it ].XpNextLvl ) );
						}
						else
						{
//							_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.ProgressBar[*it], 0 );
							CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(*it).setPROGRESS_BAR(_PropertyDatabase, 0 );
						}

						// update all parent skill with new max children
						SKILLS::ESkills skillUpdated = (*it);
						while( SkillsTree->SkillsTree[ skillUpdated ].ParentSkill != SKILLS::unknown )
						{
							if( _Skills._Skills[ skillUpdated ].MaxLvlReached > _Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached )
							{
								_Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached = _Skills._Skills[ skillUpdated ].MaxLvlReached;
								skillUpdated = SkillsTree->SkillsTree[ skillUpdated ].ParentSkill;
							}
							else
							{
								break;
							}
						}

						SM_STATIC_PARAMS_2(params, STRING_MANAGER::skill, STRING_MANAGER::integer);
						params[0].Enum = (*it);
						params[1].Int = _Skills._Skills[ *it ].Base;
						PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "PROGRESS_UNLOCK_SKILL", params);

						CMissionEventSkillProgress event( (*it), _Skills._Skills[ *it ].Base );
						processMissionEvent(event);
					}
				}
				skill->Xp = stage->XpForPointSkill;
			}
			else
			{
				// MALKAV : changed this for infinite SP bug when player reaches max skill level
				skill->Xp = 0;
			}
		}
	}
	// Update database for skill and current skill value
//	_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.ProgressBar[skillEnum], (uint8) ( skill->Xp * 255 / stage->XpForPointSkill ) );
	CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(skillEnum).setPROGRESS_BAR(_PropertyDatabase, checkedCast<uint8>(skill->Xp * 255 / stage->XpForPointSkill));

	return XpGainRemainder;
}

//-----------------------------------------------
// CCharacter::setSkillTreeToMaxValue Set skill tree of character to max value of each skill
//-----------------------------------------------
void CCharacter::setSkillsToMaxValue()
{
	setSkillsToValue(-1);
}

//-----------------------------------------------
// CCharacter::setSkillTreeToMaxValue Set skill tree of character to max value of each skill
//-----------------------------------------------
void CCharacter::setSkillsToValue(const sint32& value)
{
	// get pointer on static skills tree definition
	CSheetId sheet("skills.skill_tree");
	const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );
	nlassert( SkillsTree );

	for( uint i = 0; i < SKILLS::NUM_SKILLS; ++i )
	{
		_Skills._Skills[ i ].Base = (value < 0) ? SkillsTree->SkillsTree[ i ].MaxSkillValue : min( value, (sint32)SkillsTree->SkillsTree[ i ].MaxSkillValue );
		_Skills._Skills[ i ].Current = SkillsTree->SkillsTree[ i ].MaxSkillValue + _Skills._Skills[ i ].Modifier;
		_Skills._Skills[ i ].MaxLvlReached = _Skills._Skills[ i ].Current;

		CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(i).setBaseSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ i ].Base) );
		CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(i).setSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ i ].Current) );

		// update all parent skill with new max children
		SKILLS::ESkills skillUpdated = (SKILLS::ESkills)i;
		while( SkillsTree->SkillsTree[ skillUpdated ].ParentSkill != SKILLS::unknown )
		{
			if( _Skills._Skills[ i ].Base > _Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached )
			{
				_Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached = _Skills._Skills[ i ].Base;
				_Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].Base = min( _Skills._Skills[ skillUpdated ].Base, (sint32)SkillsTree->SkillsTree[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxSkillValue );
				skillUpdated = SkillsTree->SkillsTree[ skillUpdated ].ParentSkill;
			}
			else
			{
				break;
			}
		}
	}
}

//-----------------------------------------------
// CCharacter::sendDynamicSystemMessage
//-----------------------------------------------
void CCharacter::sendDynamicSystemMessage(const TDataSetRow &playerRowId, const std::string &msgName, const TVectorParamCheck & params )
{
	const CEntityId eid = TheDataset.getEntityId(playerRowId);
	if (eid.getType() != RYZOMID::player)
		return;

	uint32 stringId = STRING_MANAGER::sendStringToClient(playerRowId, msgName, params );

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (eid) );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "STRING:DYN_STRING", bms) )
	{
		nlwarning("<sendDynamicSystemMessage> Msg name STRING:DYN_STRING not found");
	}
	else
	{
		bms.serial( stringId );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(eid.getDynamicId()), msgout );
	}
}

//-----------------------------------------------
// CCharacter::sendDynamicSystemMessage
//-----------------------------------------------
void CCharacter::sendDynamicSystemMessage(const NLMISC::CEntityId &eId, const std::string &msgName, const TVectorParamCheck & params )
{
	if (eId.getType() != RYZOMID::player)
		return;

	TDataSetRow playerRowId = TheDataset.getDataSetRow( eId );
	if ( !TheDataset.isAccessible(playerRowId) )
		return;

	sendDynamicSystemMessage(playerRowId, msgName, params );
}

//-----------------------------------------------
// CCharacter::sendDynamicSystemMessageToChatGroup
//-----------------------------------------------
void CCharacter::sendDynamicMessageToChatGroup(const TDataSetRow &playerRowId, const std::string &msgName, CChatGroup::TGroupType type, const TVectorParamCheck & params )
{
	const CEntityId eId = TheDataset.getEntityId(playerRowId);
	if (eId.getType() != RYZOMID::player)
		return;

	uint32 stringId = STRING_MANAGER::sendStringToClient(playerRowId, msgName, params );

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( const_cast<CEntityId&> (eId) );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "STRING:DYN_STRING_GROUP", bms) )
	{
		nlwarning("<sendDynamicSystemMessage> Msg name STRING:DYN_STRING_GROUP not found");
	}
	else
	{
		bms.serialEnum(type);
		bms.serial( stringId );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(eId.getDynamicId()), msgout );
	}
}

//-----------------------------------------------
// CCharacter::sendDynamicSystemMessageToChatGroup
//-----------------------------------------------
void CCharacter::sendDynamicMessageToChatGroup(const NLMISC::CEntityId &eId, const std::string &msgName, CChatGroup::TGroupType type, const TVectorParamCheck & params )
{
	if (eId.getType() != RYZOMID::player)
		return;

	TDataSetRow playerRowId = TheDataset.getDataSetRow( eId );
	if ( !TheDataset.isAccessible(playerRowId) )
		return;

	sendDynamicMessageToChatGroup(playerRowId, msgName, type, params );
}


//----------------------------------------------------------------------------
void CCharacter::sendMessageToClient( uint32 userId, NLNET::CMessage& msgout )
{
	CUnifiedNetwork::getInstance()->send( PlayerManager.getPlayerFrontEndId( userId ), msgout );
}


//----------------------------------------------------------------------------
void CCharacter::sendUserChar( uint32 userId, uint8 scenarioSeason, const R2::TUserRole& userRole )
{
	CBitMemStream bms1;
	GenericMsgManager.pushNameToStream( "CONNECTION:USER_CHAR", bms1 );

	// Tell the client the session id of the highest mainland in its position stack (especially for the ticket system)
	TSessionId highestMainlandSessionId = 0;
	if (!PositionStack.empty())
	{
		for (sint i=(sint)(PositionStack.size()-1); i>=0; --i)
		{
			TSessionId sessionId = PositionStack[i].SessionId;
			if (CShardNames::getInstance().getShardIndex(sessionId) != 0xffffffff) // uses HomeMainlandNames (includes CSR shard) instead of Mainlands
			{
				highestMainlandSessionId = sessionId;
				break;
			}
		}
	}
	CUserCharMsg::write( bms1, getState(), scenarioSeason, userRole.getValue(), IsRingShard.get(), highestMainlandSessionId, _FirstConnectedTime, _PlayedTime );
	CMessage msgout1( "IMPULSION_ID" ); // new: now uses Id instead of Uid (CL_ID must have been sent before to the FS)
	msgout1.serial( _Id );
	msgout1.serialBufferWithSize((uint8*)bms1.buffer(), bms1.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout1 );

	if (IsRingShard)
	{
		_RingSeason = scenarioSeason;
	}
}

//-----------------------------------------------
// Return the home mainland session id for a character
TSessionId CCharacter::getHomeMainlandSessionId() const
{
	if (PositionStack.empty())
		return TSessionId(0);
	else
		return PositionStack[0].SessionId;
}

// Set the home mainland session id
void CCharacter::setHomeMainlandSessionId(TSessionId homeSessionId)
{
	if ( PositionStack.empty() )
		pushCurrentPosition();

	CFarPosition pos = PositionStack[0];
	pos.SessionId = homeSessionId;

	PositionStack.substFarPosition(0, pos);
}

// check if the character is a newbie by checking the base of the position stack
bool CCharacter::isNewbie() const
{
	// default to newbie in case of position stack error
	bool newbie = true;
	if (!PositionStack.empty())
	{
		const COfflineEntityState& state = (PositionStack[0].SessionId == sessionId()) ? getState() : PositionStack[0].PosState;
		CContinent *homeCont= CZoneManager::getInstance().getContinent(state.X, state.Y);
		if (homeCont != NULL)
			newbie = (((CONTINENT::TContinent)homeCont->getId()) == CONTINENT::NEWBIELAND) ? 1 : 0;
	}

	return newbie;
}

//-----------------------------------------------
void CCharacter::applyTopOfPositionStack()
{
//	nlassert( !_Enter );
	nlassert( ! PositionStack.empty() );
	WARN_IF( sessionUserRole() == R2::TUserRole::ur_editor, NLMISC::toString("Applying stack pos in edition mode for %u", (uint)_Id.getShortId()) );
	setState( PositionStack.top().PosState );
	setSessionId( PositionStack.top().SessionId );
	// Note: the top will be replaced by the current position when saving
}

//-----------------------------------------------
void CCharacter::applyAndPushNewPosition( const CFarPosition& farPos )
{
//	nlassert( !_Enter );
	WARN_IF( sessionUserRole() == R2::TUserRole::ur_editor, NLMISC::toString("Pushing stack pos in edition mode for %u", (uint)_Id.getShortId()) );
	setState( farPos.PosState ); // goes to temp storage (overwriting loaded pos), character is not in mirror yet
	setSessionId( farPos.SessionId );
	if (PositionStack.empty() || PositionStack.top().SessionId != farPos.SessionId)
	{
		PositionStack.push( farPos );
	}
	else
	{
		PositionStack.topToModify() = farPos;
	}

	// Note: the top will be replaced by the current position when saving
}

//-----------------------------------------------
void CCharacter::applyEditorPosition( const CFarPosition& farPos )
{
	//nlassert( !_Enter );
	setState( farPos.PosState ); // goes to temp storage (overwriting loaded pos), character is not in mirror yet
	setSessionId( SessionLockPositionStack );
	setCurrentSessionId(farPos.SessionId);
}

//-----------------------------------------------
void CCharacter::pushCurrentPosition()
{
	CFarPosition farPos;
	farPos.SessionId = sessionId();
	farPos.PosState = getState();
	PositionStack.push( farPos );
}

//-----------------------------------------------
void CCharacter::popAndApplyPosition()
{
//	nlassert( !_Enter );
	nlassert( PositionStack.size() > 1 );
	PositionStack.pop();
	applyTopOfPositionStack();
}

//-----------------------------------------------
void CCharacter::leavePreviousSession(TSessionId sessionId)
{
	if (PositionStack.size() == 0)  { return; }
	if (PositionStack.top().SessionId == sessionId)
	{
		PositionStack.pop();
	}

	applyTopOfPositionStack();
}


//-----------------------------------------------
void CCharacter::requestFarTP( TSessionId destSessionId, bool allowRetToMainlandIfFails, bool sendViaUid )
{
	CMessage msgout1;
	NLNET::TServiceId destFrontendId;
	if ( sendViaUid )
	{
		msgout1.setType( "IMPULSION_UID" );
		uint32 userId = PlayerManager.getPlayerId( _Id );
		msgout1.serial( userId );
		destFrontendId = PlayerManager.getPlayerFrontEndId( userId );
	}
	else
	{
		msgout1.setType( "IMPULSION_ID" );
		msgout1.serial( _Id );
		destFrontendId = NLNET::TServiceId(_Id.getDynamicId());
	}
	CBitMemStream bms1;
	GenericMsgManager.pushNameToStream( "CONNECTION:FAR_TP", bms1 );
	bms1.serial( destSessionId ); // TODO: Send (character, destSessionId) to SU (anti-hacking)
	bool bailOutIfSessionVanished = allowRetToMainlandIfFails && true; // TODO: Only if destSession does not belong to the mainland shards
	bms1.serial( bailOutIfSessionVanished );
	msgout1.serialBufferWithSize((uint8*)bms1.buffer(), bms1.length());
	CUnifiedNetwork::getInstance()->send( destFrontendId, msgout1 );
	nldebug( "User %u: Far TP to session %u (%u %u)", PlayerManager.getPlayerId( _Id ), destSessionId.asInt(), (uint)allowRetToMainlandIfFails, (uint)sendViaUid );
	// Note: Disconnect to prevent cheating if the client could sent actions now?
}

//-----------------------------------------------
void CCharacter::returnToPreviousSession( uint32 userId, sint32 charIndex, TSessionId rejectedSessionId )
{
	if ( PositionStack.empty() )
	{
		nlwarning( "Can't return to mainland session %u, no position in stack for char %u", _SessionId.asInt(), getId().getShortId() );
		return;
	}

	// Pop the top position from the position stack.
	// Except: If there is only one position in stack, keep it.
	// Except: The client may be in edition mode (stack locked without the edition position in stack).
	// Force popping if the client reports an error when trying to join rejectedSessionId.
	if ( PositionStack.size() > 1 )
	{
		if ( sessionId() != SessionLockPositionStack )
		{
			PositionStack.pop();
			nldebug( "User %u: Returning to previous session %u (unlocked)", (uint)_Id.getShortId() >> 4, PositionStack.top().SessionId.asInt() );
		}
		else if ( PositionStack.top().SessionId == rejectedSessionId )
		{
			PositionStack.pop();
			nldebug( "User %u: Forcing return to session %u (locked but %u rejected)", (uint)_Id.getShortId() >> 4, PositionStack.top().SessionId.asInt(), rejectedSessionId.asInt() );
		}
		else
			nldebug( "User %u: Returning to top session %u (locked)", (uint)_Id.getShortId() >> 4, PositionStack.top().SessionId.asInt() );
	}
	else
		nldebug( "User %u: Keeping single session %u", (uint)_Id.getShortId() >> 4, PositionStack.top().SessionId.asInt() );

	// We can't modify the current position (setState()) now (managed by the GPMS),
	// so let's lock the top of the stack to prevent from overwriting when saving.
	setSessionId( SessionLockPositionStack );

	// Tell the client to make a Far TP to the new top session
	forbidNearPetTp();
	requestFarTP( PositionStack.top().SessionId, (PositionStack.size() > 1), (userId!=~0) );

	if ( ! getEnterFlag() )
	{
		// Force saving now because it will be skipped when receiving 'quit request' and when disconnecting if not 'entered'
		if ( userId == ~0 )
			PlayerManager.savePlayerActiveChar( PlayerManager.getPlayerId( getId() ) );
		else
			PlayerManager.savePlayerChar( userId, charIndex );
	}
}

//-----------------------------------------------
// CCharacter::harvestAsked
//-----------------------------------------------
void CCharacter::harvestAsked( uint16 mpIndex , uint16 quantity )
{
	// harvest
	if ( harvestedMpIndex() != 0xff )
	{
		// player already harvesting, cancel
		nlwarning("<cbHarvest> player Id %s already harvesting", _Id.toString().c_str() );


		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( harvestedEntity(), CAIAliasTranslator::getInstance()->getAIAlias(harvestedEntity()) );
		sendDynamicSystemMessage(_Id, "OPS_HARVEST_MP_IN_PROGRESS_E", params);

		return;
	}

	if ( _MpSourceId != CEntityId::Unknown )
	{
		// get harvested creature
		CCreature *creature = CreatureManager.getCreature( _MpSourceId );
		if (creature != NULL)
		{
			// Check if PJ have loot right
			if ( ForceQuarteringRight.get() || (std::find( creature->getLootRight().begin(),creature->getLootRight().end(),_EntityRowId ) != creature->getLootRight().end()) )
			{
				if (!enterTempInventoryMode(TEMP_INV_MODE::Quarter))
				{
					//sendCloseTempInventoryImpulsion();
					endHarvest();
					return;
				}

				const vector<CCreatureRawMaterial> &mps = creature->getMps();
				if (mpIndex < mps.size())
				{
					if (quantity > mps[mpIndex].Quantity)
						quantity = mps[mpIndex].Quantity;

					harvest( (uint8)mpIndex, quantity );
				}
			}
			else
			{
				sendCloseTempInventoryImpulsion();
				endHarvest();
				CCharacter::sendDynamicSystemMessage(_EntityRowId, "YOU_NOT_HAVE_QUARTER_RIGHT");
				return;
			}
		}
	}
	_HarvestedQuantity = quantity;
}


//-----------------------------------------------
// harvest
//-----------------------------------------------
void CCharacter::harvest( uint8 mpIndex, uint16 quantity )
{
	if (quantity == 0)
	{
		nlwarning("<CCharacter::harvest> Player %s tried to harvest 0 element, do nothing",  _Id.toString().c_str());
		//sendMessageToClient( _Id, "OPS_HARVEST_NULL_QTY");
		return;
	}

	if ( _MpIndex != 0xff )
	{
		nlwarning("<CCharacter::harvest> player %s already being harvesting mp index %u, cancel", _Id.toString().c_str(), _MpIndex );
		return;
	}

	// get the harvested entity
	CCreature * creature = CreatureManager.getCreature( _MpSourceId );
	if (creature != NULL)
	{
		// check the quantity is good
		if ( quantity > creature->getQuantity( mpIndex ) )
		{
			quantity = creature->getQuantity( mpIndex );
		}
	}
	else
	{
		// ERROR
		nlwarning("<CCharacter::harvest>Player %s harvest on UNKNOWN entity sheet %s", _Id.toString().c_str(), _MpSourceSheetId.toString().c_str() );
		return;
	}

	// get the mp wanted
	const CRawMaterial *mp = creature->getRawMaterial( mpIndex );
	if (mp == NULL)
	{
		nlwarning("<CCharacter::harvest> Invalid MP index %u, cancel", mpIndex);
		return;
	}

	CSheetId itemId(mp->AssociatedItemName.c_str());

	_MpIndex = mpIndex;
	_HarvestedQuantity = quantity;

	// begin the harvest
//	egs_chinfo("< CCharacter::harvest>Player %s harvest on entity sheet %s", _Id.toString().c_str(), _MpSourceSheetId.toString().c_str() );

	if (!CPhraseManager::getInstance().harvestDefault( _EntityRowId, itemId, mp->MinQuality, mp->MaxQuality, quantity))
		resetHarvestInfos();
} // harvest //


//-----------------------------------------------
// endHarvest
//-----------------------------------------------
void CCharacter::endHarvest(bool sendCloseTempImpulsion)
{
	pickUpItemClose();

	_HarvestDeposit = false;
	_HarvestOpened = false;
	_DepositSearchSkill = SKILLS::unknown;
	_MpIndex = 0xff;
	_DepositHarvestInformation.DepositIndex = 0xffffffff;

	if ( _DepositHarvestInformation.Sheet != CSheetId::Unknown/*_DepositHarvestInformation.EndCherchingTime != 0xffffffff && _DepositHarvestInformation.EndCherchingTime > CTickEventHandler::getGameCycle()*/ )
	{
		if ( sendCloseTempImpulsion )
		{
			sendCloseTempInventoryImpulsion();
		}
	}

	if ( _MpSourceId != CEntityId::Unknown || _MpSourceSheetId != CSheetId::Unknown )
	{
		//get harvested creature if any
		CCreature *creature = CreatureManager.getCreature( _MpSourceId );
		if (creature != NULL)
		{
			creature->resetHarvesterRowId();
			// only send interupt message if some rm remains on the corpse
			if ( sendCloseTempImpulsion )
				sendCloseTempInventoryImpulsion();
		}

		// send end loot behaviour
		PHRASE_UTILITIES::sendUpdateBehaviour( _EntityRowId, MBEHAV::LOOT_END );

		resetHarvestInfos();
		harvestedEntity( CEntityId::Unknown );
		_MpSourceSheetId = CSheetId();
	}

	clearHarvestDB();

} // endHarvest //

//-----------------------------------------------
// clearHarvestDB
//------------------------------------------------
void CCharacter::clearHarvestDB()
{
	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
	nlassert(invTemp != NULL);
	// init the database
	for (uint i = 0 ; i < invTemp->getSlotCount(); ++i)
	{
		invTemp->clearDisp(i);
	}
} // clearHarvestDB //

//-----------------------------------------------
// checkCreateParams : Check create parameters return false if error and set createCharErrorMsg
//-----------------------------------------------
bool CCharacter::checkCreateParams( const CCreateCharMsg& createCharMsg, CCreateCharErrorMsg& createCharErrorMsg, uint32 userId )
{
	bool returnValue = true;
	CSheetId people_sheet/*, role_sheet*/;
	const CStaticRaceStats* staticRaceStats = 0;

	CPlayer *player = PlayerManager.getPlayer(userId);
	if (player == NULL)
	{
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Can't find the player for user %u", userId);
		return false;
	}

	// check if the slot is correct
	if (createCharMsg.Slot >= player->getCharacterReference().size())
	{
		createCharErrorMsg.Slot = false;
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Slot %d is refused because out of bound", createCharMsg.Slot );
	}

	// check if slot is free
	if( PlayerManager.getChar( userId, createCharMsg.Slot ) != NULL )
	{
		createCharErrorMsg.Slot = false;
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Slot %d is refused because character already exist in this slot", createCharMsg.Slot );
	}

	//check if name is ok ( for now ok means not empty and with no space )
	if ( createCharMsg.Name.empty() || createCharMsg.Name.find(' ') != string::npos )
	{
		createCharErrorMsg.Name = false;
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Name %s is refused because it's contains spaces", createCharMsg.Name.toString().c_str() );
	}

	if( CEntityIdTranslator::getInstance()->entityNameExists( createCharMsg.Name ) )
	{
		createCharErrorMsg.Name = false;
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Name %s is refused because it's already registered", createCharMsg.Name.toString().c_str());
	}

	for( map< CSheetId, CStaticRaceStats >::const_iterator it = CSheets::getRaceStatsContainer().begin(); it != CSheets::getRaceStatsContainer().end(); ++it )
	{
		if( (*it).second.Race == createCharMsg.People )
		{
			people_sheet = (*it).first;
			break;
		}
	}

	// Summ role point distribution
	uint8 sum = createCharMsg.NbPointFighter + createCharMsg.NbPointCaster + createCharMsg.NbPointCrafter + createCharMsg.NbPointHarvester;
	if( sum > 5 )
	{
		createCharErrorMsg.Role = false;
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Role point distribution refused: FighterPoint: %hu, CasterPoint: %hu, CrafterPoint: %hu, HarvesterPoint: %hu", createCharMsg.NbPointFighter, createCharMsg.NbPointCaster, createCharMsg.NbPointCrafter, createCharMsg.NbPointHarvester);
	}

	if( createCharMsg.Sex != GSGENDER::male && createCharMsg.Sex != GSGENDER::female )
	{
		createCharErrorMsg.Sex = false;
		returnValue = false;
		egs_chinfo("<CCharacter::checkCreateParams> Sex %d refused (only male and female managed...)", createCharMsg.Sex );
	}

#ifdef NL_DEBUG
	egs_chinfo("<CCharacter::checkCreateParams> CCreateCharMsg Content:");
	egs_chinfo("-------		Mainland         %d", createCharMsg.Mainland.asInt());
	egs_chinfo("-------		Name             %s", createCharMsg.Name.toString().c_str());
	egs_chinfo("-------		People           %s", EGSPD::CPeople::toString((EGSPD::CPeople::TPeople)createCharMsg.People).c_str());
	egs_chinfo("-------		Sex              %s", GSGENDER::toString((GSGENDER::EGender)createCharMsg.Sex).c_str());
	egs_chinfo("-------		NBPointFighter   %d", createCharMsg.NbPointFighter);
	egs_chinfo("-------		NBPointCaster    %d", createCharMsg.NbPointCaster);
	egs_chinfo("-------		NBPointCrafter   %d", createCharMsg.NbPointCrafter);
	egs_chinfo("-------		NBPointHarvester %d", createCharMsg.NbPointHarvester);
	egs_chinfo("-------		StartingPoint    %s", RYZOM_STARTING_POINT::toString(createCharMsg.StartPoint).c_str());
#endif
	return returnValue;
}

//-----------------------------------------------
// Statistics : Set start statistics and other params on character
//-----------------------------------------------
void CCharacter::setStartStatistics( const CCreateCharMsg& createCharMsg )
{
	_SavedVersion = getCurrentVersion();
	CSheetId people_sheet;
	const CStaticRaceStats* staticRaceStats = 0;

	for( map< CSheetId, CStaticRaceStats >::const_iterator it = CSheets::getRaceStatsContainer().begin(); it != CSheets::getRaceStatsContainer().end(); ++it )
	{
		if( (*it).second.Race == createCharMsg.People )
		{
			people_sheet = (*it).first;
			break;
		}
	}
	nlassert( people_sheet != CSheetId::Unknown );
	staticRaceStats = CSheets::getRaceStats( people_sheet );
	nlassert( staticRaceStats );

	_SheetId= people_sheet.asInt();

	_Name				= createCharMsg.Name;
	_Race				= (EGSPD::CPeople::TPeople) createCharMsg.People;
	_Gender				= createCharMsg.Sex;
	_Title				= CHARACTER_TITLE::Refugee;
	_NewTitle			= "Refugee";

	// fame information
	// Players start out as Neutral in their declared clans
	_DeclaredCult = PVP_CLAN::Neutral;
	_DeclaredCiv = PVP_CLAN::Neutral;

	// output stats
	//Bsi.append( StatPath, NLMISC::toString("[CPJ] %s %s %s %d %d %d %d %s", _Id.toString().c_str(), EGSPD::CPeople::toString(_Race).c_str(), GSGENDER::toString((GSGENDER::EGender)_Gender).c_str(), createCharMsg.NbPointFighter, createCharMsg.NbPointCaster, createCharMsg.NbPointCrafter, createCharMsg.NbPointHarvester, RYZOM_STARTING_POINT::toString(createCharMsg.StartPoint).c_str()) );
	//EgsStat.displayNL("[CPJ] %s %s %s %d %d %d %d %s", _Id.toString().c_str(), EGSPD::CPeople::toString(_Race).c_str(), GSGENDER::toString((GSGENDER::EGender)_Gender).c_str(), createCharMsg.NbPointFighter, createCharMsg.NbPointCaster, createCharMsg.NbPointCrafter, createCharMsg.NbPointHarvester, RYZOM_STARTING_POINT::toString(createCharMsg.StartPoint).c_str());
//	EGSPD::createPlayerCharacter(_Id, _Race, (uint8)_Gender, createCharMsg.NbPointFighter, createCharMsg.NbPointCaster, createCharMsg.NbPointCrafter, createCharMsg.NbPointHarvester, RYZOM_STARTING_POINT::toString(createCharMsg.StartPoint));
	log_Character_Create(uint32(_Id.getShortId()>>4), _Id, _Name.toUtf8());

	// Set Characteristics
	int i;
	for( i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		// at level 1 for formula dependency with regenerate value
		_PhysCharacs._PhysicalCharacteristics[ i ].Base = 10;
		//TODO _PhysCharacs._PhysicalCharacteristics[ i ].Base = StartCharacteristicsValue;
		_PhysCharacs._PhysicalCharacteristics[ i ].Modifier = 0;

		_PhysCharacs._PhysicalCharacteristics[ i ].RegenerateModifier = 0;

		_PhysCharacs._PhysicalCharacteristics[ i ].Max = _PhysCharacs._PhysicalCharacteristics[ i ].Base;
		_PhysCharacs._PhysicalCharacteristics[ i ].Current = _PhysCharacs._PhysicalCharacteristics[ i ].Base;

		// Keep starting value
		_StartingCharacteristicValues[i] = (uint8)_PhysCharacs._PhysicalCharacteristics[ i ].Base;
	}

	// set root skills of skill tree to 1
	CSheetId sheet("skills.skill_tree");
	const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );
	sheet = string("xptable.xp_table");
	const CStaticXpStagesTable* XpStageTable = CSheets::getXpStageTableForm( sheet );

	for( vector< CStaticSkillsTree::SSkillData >::const_iterator it = SkillsTree->SkillsTree.begin(); it != SkillsTree->SkillsTree.end(); ++it )
	{
		if( (*it).ParentSkill == SKILLS::unknown )
		{
			_Skills._Skills[ (*it).Skill ].Base = 1;
			_Skills._Skills[ (*it).Skill ].Current = 1;

			const CStaticXpStagesTable::SXpStage * stage = XpStageTable->getXpStage( _Skills._Skills[ (*it).Skill ].Base, (*it).StageType );
			if( stage )
			{
				_Skills._Skills[ (*it).Skill ].XpNextLvl =  stage->XpForPointSkill;
			}
		}
	}

	// create character start skills, skill point and money
	string s = CreateCharacterStartSkillsValue;
	if( s.size() > 0 )
	{
		CSString skillValue = s;

		string skillString = skillValue.strtok(":");
		string valueString;
		while( skillString.size() > 0 )
		{
			valueString = skillValue.strtok(":");
			if( valueString.size() > 0 )
			{
				SKILLS::ESkills skillEnum = SKILLS::toSkill(skillString);
				sint32 skillPoint;
				NLMISC::fromString(valueString, skillPoint);
				if( skillEnum != SKILLS::unknown )
				{
					_Skills._Skills[ skillEnum ].Base = min( skillPoint, (sint32)SkillsTree->SkillsTree[ skillEnum ].MaxSkillValue );
					_Skills._Skills[ skillEnum ].Current = _Skills._Skills[ skillEnum ].Base;
					_Skills._Skills[ skillEnum ].MaxLvlReached = _Skills._Skills[ skillEnum ].Base;

					// update all parent skill with new max children
					SKILLS::ESkills skillUpdated = skillEnum;
					while( SkillsTree->SkillsTree[ skillUpdated ].ParentSkill != SKILLS::unknown )
					{
						if( _Skills._Skills[ skillEnum ].Base > _Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached )
						{
							_Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxLvlReached = _Skills._Skills[ skillEnum ].Base;
							_Skills._Skills[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].Base = min( _Skills._Skills[ skillUpdated ].Base, (sint32)SkillsTree->SkillsTree[ SkillsTree->SkillsTree[ skillUpdated ].ParentSkill ].MaxSkillValue );
							skillUpdated = SkillsTree->SkillsTree[ skillUpdated ].ParentSkill;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					if( skillString == string("SKILL_POINTS") )
					{
						nlwarning("CCharacter::setStartStatistics : SKILL_POINTS deprecated, should specify type");
						setSP( skillPoint, (EGSPD::CSPType::TSPType) 0 );
						setSP( skillPoint, (EGSPD::CSPType::TSPType) 1 );
						setSP( skillPoint, (EGSPD::CSPType::TSPType) 2 );
						setSP( skillPoint, (EGSPD::CSPType::TSPType) 3 );
					}
					else if(skillString == string("MONEY") )
					{
						setMoney( skillPoint );
					}
				}
				skillString = skillValue.strtok(":");
			}
			else
			{
				skillString = string("");
			}
		}
	}

	// Compute Scores
	for( i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		switch( i )
		{
		case SCORES::hit_points:
			_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::constitution ].Base + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor;
			break;
		case SCORES::sap:
			_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::intelligence ].Base + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor;
			break;
		case SCORES::stamina:
			_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::strength ].Base + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor;
			break;
		case SCORES::focus:
			_PhysScores._PhysicalScores[ i ].Base = (_PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::dexterity ].Base + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor;
			break;
		default:;
		}
		updateRegen();
		_PhysScores._PhysicalScores[ i ].Current = _PhysScores._PhysicalScores[ i ].Base;
		_PhysScores._PhysicalScores[ i ].Modifier = 0;
		_PhysScores._PhysicalScores[ i ].CurrentRegenerate = _PhysScores._PhysicalScores[ i ].BaseRegenerateAction;
		_PhysScores._PhysicalScores[ i ].RegenerateModifier = 0;
	}

	///////////////////////////////////////////////////////
	// Init Pacts
	///////////////////////////////////////////////////////
	uint8 pactNature = 0;
	bool magicien = false;

	//TODO rules about pact (magicien or fighter ?)
	if( magicien )
	{
		pactNature = GSPACT::Kamique;
	}
	else
	{
		switch( _Race )
		{
			case EGSPD::CPeople::Fyros:
			case EGSPD::CPeople::Matis:
				pactNature = GSPACT::Caravane;
				break;

			case EGSPD::CPeople::Tryker:
			case EGSPD::CPeople::Zorai:
				pactNature = GSPACT::Kamique;
				break;

			default:
				nlwarning("<CCharacter::loadSheetCharacter> Unknown Race %d", _Race );
				nlstop;
		}
	}
	_Pact.push_back( CPact( pactNature, GSPACT::Type1 ) );
//	_Pact.push_back( CPact( pactNature, GSPACT::Type2 ) );
	_Pact.push_back( CPact( pactNature, GSPACT::Type2 ) );
//	_Pact.push_back( CPact( pactNature, GSPACT::Type3 ) );
	_Pact.push_back( CPact( pactNature, GSPACT::Type3 ) );
	_NbSurvivePact = 3;

	// Visual properties
	SPropVisualA vpa;
	SPropVisualB vpb;
	SPropVisualC vpc;

	// Facial detail
	vpc.PropertySubData.MorphTarget1 = createCharMsg.MorphTarget1;
	vpc.PropertySubData.MorphTarget2 = createCharMsg.MorphTarget2;
	vpc.PropertySubData.MorphTarget3 = createCharMsg.MorphTarget3;
	vpc.PropertySubData.MorphTarget4 = createCharMsg.MorphTarget4;
	vpc.PropertySubData.MorphTarget5 = createCharMsg.MorphTarget5;
	vpc.PropertySubData.MorphTarget6 = createCharMsg.MorphTarget6;
	vpc.PropertySubData.MorphTarget7 = createCharMsg.MorphTarget7;
	vpc.PropertySubData.MorphTarget8 = createCharMsg.MorphTarget8;

	// Eyes color
	vpc.PropertySubData.EyesColor = createCharMsg.EyesColor;

	// Tattoo
	vpc.PropertySubData.Tattoo = createCharMsg.Tattoo;

	// Gabarits
	vpc.PropertySubData.CharacterHeight = createCharMsg.GabaritHeight;
	vpc.PropertySubData.TorsoWidth = createCharMsg.GabaritTorsoWidth;
	vpc.PropertySubData.ArmsWidth = createCharMsg.GabaritArmsWidth;
	vpc.PropertySubData.LegsWidth = createCharMsg.GabaritLegsWidth;
	vpc.PropertySubData.BreastSize = createCharMsg.GabaritBreastSize;


	// Hair type and colors
	vpa.PropertySubData.HatModel = createCharMsg.HairType;
	vpa.PropertySubData.HatColor = createCharMsg.HairColor;
	setHair( createCharMsg.HairType );
	setHairColor( createCharMsg.HairColor );
	_HatColor = createCharMsg.HairColor;

	_JacketColor = createCharMsg.JacketColor;
	_ArmsColor = createCharMsg.ArmsColor;
	_TrousersColor = createCharMsg.TrousersColor;
	_FeetColor = createCharMsg.FeetColor;
	_HandsColor = createCharMsg.HandsColor;

	// Custom color
	vpa.PropertySubData.JacketColor = createCharMsg.JacketColor;
	vpa.PropertySubData.ArmColor = createCharMsg.ArmsColor;
	vpa.PropertySubData.TrouserColor = createCharMsg.TrousersColor;
	vpb.PropertySubData.FeetColor = createCharMsg.FeetColor;
	vpb.PropertySubData.HandsColor = createCharMsg.HandsColor;

	// Sex
	vpa.PropertySubData.Sex = createCharMsg.Sex;

	// Default equipment
	const CStaticRaceStats::SDefaultEquipment * pDefaultEquipment = NULL;
	switch( _Gender )
	{
		case GSGENDER::male:
			pDefaultEquipment = &staticRaceStats->MaleDefaultEquipment;
			break;
		case GSGENDER::female:
			pDefaultEquipment = &staticRaceStats->FemaleDefaultEquipment;
			break;
		default:
			nlstop;
	}

	const CStaticItem * pStaticItem = CSheets::getForm( CSheetId(pDefaultEquipment->DefaultChest) );
	uint16 model = 0;
	if( pStaticItem )
	{
		model = pStaticItem->ItemIdSheetToModelNumber;
		vpa.PropertySubData.JacketColor = pStaticItem->Color;
	}
	vpa.PropertySubData.JacketModel = model;

	pStaticItem = CSheets::getForm( CSheetId(pDefaultEquipment->DefaultLegs) );
	model = 0;
	if( pStaticItem )
	{
		model = pStaticItem->ItemIdSheetToModelNumber;
		vpa.PropertySubData.TrouserColor = pStaticItem->Color;
	}
	vpa.PropertySubData.TrouserModel = model;

	pStaticItem = CSheets::getForm( CSheetId(pDefaultEquipment->DefaultArms) );
	model = 0;
	if( pStaticItem )
	{
		model = pStaticItem->ItemIdSheetToModelNumber;
		vpa.PropertySubData.ArmColor = pStaticItem->Color;
	}
	vpa.PropertySubData.ArmModel = model;

	pStaticItem = CSheets::getForm( CSheetId(pDefaultEquipment->DefaultHands) );
	model = 0;
	if( pStaticItem )
	{
		model = pStaticItem->ItemIdSheetToModelNumber;
		vpb.PropertySubData.HandsColor = pStaticItem->Color;
	}
	vpb.PropertySubData.HandsModel = model;

	pStaticItem = CSheets::getForm( CSheetId(pDefaultEquipment->DefaultFeet) );
	model = 0;
	if( pStaticItem )
	{
		model = pStaticItem->ItemIdSheetToModelNumber;
		vpb.PropertySubData.FeetColor = pStaticItem->Color;
	}
	vpb.PropertySubData.FeetModel = model;

	_VisualPropertyA= vpa;
	_VisualPropertyB= vpb;
	_VisualPropertyC= vpc;

	RYZOM_STARTING_POINT::TStartPoint sp = createCharMsg.StartPoint;
	if( sp < RYZOM_STARTING_POINT::stalli || sp >= RYZOM_STARTING_POINT::NB_START_POINTS )
	{
		if(UseNewNewbieLandStartingPoint)
			sp = RYZOM_STARTING_POINT::starting_city;
		else
//			sp = RYZOM_STARTING_POINT::stalli;
			sp = RYZOM_STARTING_POINT::starting_city;
		nlwarning( "Invalid start point %d", sp );
	}

	// set the character initial state
	TAIAlias bot,mission;
	const CTpSpawnZone * zone = CZoneManager::getInstance().getStartPoint( (uint16)sp,bot,mission );
	if ( !zone )
	{
		nlwarning( "Invalid start point %d", sp );
	}
	else
	{
		setWelcomeMissionDesc(mission, bot);
		COfflineEntityState state;
		zone->getRandomPoint( state.X,state.Y,state.Z,state.Heading );
		setState( state );
	}

	// set the selected mainland shard
	setSessionId( createCharMsg.Mainland );

	// save the resulint "far position"
	pushCurrentPosition();

	// give item and action corresponding to role chosen at character create
	searchCreateRoleSheet( _Race, ROLES::crafter, createCharMsg.NbPointCrafter );
	searchCreateRoleSheet( _Race, ROLES::harvester, createCharMsg.NbPointHarvester );
	searchCreateRoleSheet( _Race, ROLES::caster, createCharMsg.NbPointCaster );
	searchCreateRoleSheet( _Race, ROLES::fighter, createCharMsg.NbPointFighter );

	// give item reserved for pre-order players
	bool preOrder = false;
	CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
	if (p != NULL)
		preOrder = p->isPreOrder();

	if (preOrder)
	{
		CSheetId sheetId("pre_order.sitem");
		if (sheetId != CSheetId::Unknown)
		{
			createItemInInventory(INVENTORIES::bag, 10, 1, sheetId, _Id);
		}
	}
	_CreationPointsRepartition = (createCharMsg.NbPointCaster<<6) + (createCharMsg.NbPointFighter<<4) + (createCharMsg.NbPointCrafter<<2) + createCharMsg.NbPointHarvester;

	// log stat
	_FirstConnectedTime = CTime::getSecondsSince1970();
}


//-----------------------------------------------
// search role sheet and call start equipment and memorize actions at create character
//-----------------------------------------------
void CCharacter::searchCreateRoleSheet( EGSPD::CPeople::TPeople people, ROLES::ERole role, uint8 nbPoints, bool onlyMemorizeActions )
{
	// give item depending of roles choice
	const CStaticRole * staticRole = 0;
	if( nbPoints > 0 )
	{
		for( map< CSheetId, CStaticRole >::const_iterator itr = CSheets::getRoleContainer().begin(); itr != CSheets::getRoleContainer().end(); ++itr )
		{
			if( (*itr).second.Race == people )
			{
				if( (*itr).second.Role == role )
				{
					staticRole = &(*itr).second;
#ifdef NL_DEBUG
					egs_chinfo("<CCharacter::searchCreateRoleSheet> Selected sheet: %s", (*itr).first.toString().c_str());
#endif
					break;
				}
			}
		}
	}
	if( staticRole != 0 )
	{
		switch( nbPoints )
		{
			case 1:
				memorizeStartAction( staticRole->MemorizedSentences1 );
				if (!onlyMemorizeActions)
				{
					if( role == ROLES::crafter )
					{
						createItemInInventory( INVENTORIES::bag, 10, 50, CSheetId("system_mp.sitem") );
					}
				}
				break;
			case 2:
				memorizeStartAction( staticRole->MemorizedSentences2 );
				if (!onlyMemorizeActions)
				{
					if( role == ROLES::crafter )
					{
						createItemInInventory( INVENTORIES::bag, 10, 100, CSheetId("system_mp.sitem") );
					}
				}
				break;
			case 3:
				memorizeStartAction( staticRole->MemorizedSentences3 );
				if (!onlyMemorizeActions)
				{
					if( role == ROLES::crafter )
					{
						createItemInInventory( INVENTORIES::bag, 10, 100, CSheetId("system_mp.sitem") );
						createItemInInventory( INVENTORIES::bag, 10, 100, CSheetId("system_mp.sitem") );
					}
				}
				break;
			default:;
		}

		if (!onlyMemorizeActions)
		{
			switch( nbPoints )
			{
				case 3:
					setStartEquipment( staticRole->Items3 );
				case 2:
					setStartEquipment( staticRole->Items2 );
				case 1:
					setStartEquipment( staticRole->Items1 );
				default:;
			}
		}
	}
}


//-----------------------------------------------
// setStartEquipment : Set start equipment at create character
//-----------------------------------------------
void CCharacter::setStartEquipment( const SMirrorEquipment* Items )
{
	//////////////////////////////////////////////////////
	// Equipment
	///////////////////////////////////////////////////////
	// Headdress
	if( _Items.Headdress != Items[ SLOT_EQUIPMENT::HEADDRESS ] )
	{
		if (Items[SLOT_EQUIPMENT::HEADDRESS].IdSheet != CSheetId::Unknown)
		{
			_Items.Headdress = Items[ SLOT_EQUIPMENT::HEADDRESS ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Headdress.Quality, 1, _Items.Headdress.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::HEADDRESS, item->getInventorySlot() );
			}
		}
	}
	// Chest
	if( _Items.Chest != Items[ SLOT_EQUIPMENT::CHEST ] )
	{
		if (Items[SLOT_EQUIPMENT::CHEST].IdSheet != CSheetId::Unknown)
		{
			_Items.Chest = Items[ SLOT_EQUIPMENT::CHEST ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Chest.Quality, 1, _Items.Chest.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::CHEST, item->getInventorySlot() );
			}
		}
	}
	// Legs
	if( _Items.Legs != Items[ SLOT_EQUIPMENT::LEGS ] )
	{
		if (Items[SLOT_EQUIPMENT::LEGS].IdSheet != CSheetId::Unknown)
		{
			_Items.Legs = Items[ SLOT_EQUIPMENT::LEGS ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Legs.Quality, 1, _Items.Legs.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::LEGS, item->getInventorySlot() );
			}
		}
	}
	// Arms
	if( _Items.Arms != Items[ SLOT_EQUIPMENT::ARMS ] )
	{
		if (Items[SLOT_EQUIPMENT::ARMS].IdSheet != CSheetId::Unknown)
		{
			_Items.Arms = Items[ SLOT_EQUIPMENT::ARMS ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Arms.Quality, 1, _Items.Arms.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::ARMS, item->getInventorySlot() );
			}
		}
	}
	// Hands
	if( _Items.Hands != Items[ SLOT_EQUIPMENT::HANDS ] )
	{
		if (Items[SLOT_EQUIPMENT::HANDS].IdSheet != CSheetId::Unknown)
		{
			_Items.Hands = Items[ SLOT_EQUIPMENT::HANDS ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Hands.Quality, 1, _Items.Hands.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::HANDS, item->getInventorySlot() );
			}
		}
	}
	// HandL
	if( _Items.Sheath[ 0 ].HandL != Items[ SLOT_EQUIPMENT::HANDL ] )
	{
		if (Items[SLOT_EQUIPMENT::HANDL].IdSheet != CSheetId::Unknown)
		{
			_Items.Sheath[ 0 ].HandL = Items[ SLOT_EQUIPMENT::HANDL ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Sheath[ 0 ].HandL.Quality, 1, _Items.Sheath[ 0 ].HandL.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::handling, INVENTORIES::left, item->getInventorySlot() );
			}
		}
	}
	// HandR
	if( _Items.Sheath[ 0 ].HandR != Items[ SLOT_EQUIPMENT::HANDR ] )
	{
		if (Items[SLOT_EQUIPMENT::HANDR].IdSheet != CSheetId::Unknown)
		{
			_Items.Sheath[ 0 ].HandR = Items[ SLOT_EQUIPMENT::HANDR ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Sheath[ 0 ].HandR.Quality, 1, _Items.Sheath[ 0 ].HandR.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::handling, INVENTORIES::right, item->getInventorySlot() );
			}
		}
	}
	// Feet
	if( _Items.Feet != Items[ SLOT_EQUIPMENT::FEET ] )
	{
		if (Items[SLOT_EQUIPMENT::FEET].IdSheet != CSheetId::Unknown)
		{
			_Items.Feet = Items[ SLOT_EQUIPMENT::FEET ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Feet.Quality, 1, _Items.Feet.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::FEET, item->getInventorySlot() );
			}
		}
	}
	// Head
	if( _Items.Head != Items[ SLOT_EQUIPMENT::HEAD ] )
	{
		if (Items[SLOT_EQUIPMENT::HEAD].IdSheet != CSheetId::Unknown)
		{
			_Items.Head = Items[ SLOT_EQUIPMENT::HEAD ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Head.Quality, 1, _Items.Head.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::HEAD, item->getInventorySlot() );
			}
		}
	}
	// Earl
	if( _Items.EarL != Items[ SLOT_EQUIPMENT::EARL ] )
	{
		if (Items[SLOT_EQUIPMENT::EARL].IdSheet != CSheetId::Unknown)
		{
			_Items.EarL = Items[ SLOT_EQUIPMENT::EARL ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.EarL.Quality, 1, _Items.EarL.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::EARL, item->getInventorySlot() );
			}
		}
	}
	// EarR
	if( _Items.EarR != Items[ SLOT_EQUIPMENT::EARR ] )
	{
		if (Items[SLOT_EQUIPMENT::EARR].IdSheet != CSheetId::Unknown)
		{
			_Items.EarR = Items[ SLOT_EQUIPMENT::EARR ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.EarR.Quality, 1, _Items.EarR.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::EARR, item->getInventorySlot() );
			}
		}
	}
	// Neck
	if( _Items.Neck != Items[ SLOT_EQUIPMENT::NECKLACE ] )
	{
		if (Items[SLOT_EQUIPMENT::NECKLACE].IdSheet != CSheetId::Unknown)
		{
			_Items.Neck = Items[ SLOT_EQUIPMENT::NECKLACE ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.Neck.Quality, 1, _Items.Neck.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::NECKLACE, item->getInventorySlot() );
			}
		}
	}
	// WristL
	if( _Items.WristL != Items[ SLOT_EQUIPMENT::WRISTL ] )
	{
		if (Items[SLOT_EQUIPMENT::WRISTL].IdSheet != CSheetId::Unknown)
		{
			_Items.WristL = Items[ SLOT_EQUIPMENT::WRISTL ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.WristL.Quality, 1, _Items.WristL.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::WRISTL, item->getInventorySlot() );
			}
		}
	}
	// WristR
	if( _Items.WristR != Items[ SLOT_EQUIPMENT::WRISTR ] )
	{
		if (Items[SLOT_EQUIPMENT::WRISTR].IdSheet != CSheetId::Unknown)
		{
			_Items.WristR = Items[ SLOT_EQUIPMENT::WRISTR ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.WristR.Quality, 1, _Items.WristR.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::WRISTR, item->getInventorySlot() );
			}
		}
	}
	// FingerL
	if( _Items.FingerL != Items[ SLOT_EQUIPMENT::FINGERL ] )
	{
		if (Items[SLOT_EQUIPMENT::FINGERL].IdSheet != CSheetId::Unknown)
		{
			_Items.FingerL = Items[ SLOT_EQUIPMENT::FINGERL ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.FingerL.Quality, 1, _Items.FingerL.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::FINGERL, item->getInventorySlot() );
			}
		}
	}
	// FingerR
	if( _Items.FingerR != Items[ SLOT_EQUIPMENT::FINGERR ] )
	{
		if (Items[SLOT_EQUIPMENT::FINGERR].IdSheet != CSheetId::Unknown)
		{
			_Items.FingerR = Items[ SLOT_EQUIPMENT::FINGERR ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.FingerR.Quality, 1, _Items.FingerR.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::FINGERR, item->getInventorySlot() );
			}
		}
	}
	// AnkleL
	if( _Items.AnkleL != Items[ SLOT_EQUIPMENT::ANKLEL ] )
	{
		if (Items[SLOT_EQUIPMENT::ANKLEL].IdSheet != CSheetId::Unknown)
		{
			_Items.AnkleL = Items[ SLOT_EQUIPMENT::ANKLEL ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.AnkleL.Quality, 1, _Items.AnkleL.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::ANKLEL, item->getInventorySlot() );
			}
		}
	}
	// AnkleR
	if( _Items.AnkleR != Items[ SLOT_EQUIPMENT::ANKLER ] )
	{
		if (Items[SLOT_EQUIPMENT::ANKLER].IdSheet != CSheetId::Unknown)
		{
			_Items.AnkleR = Items[ SLOT_EQUIPMENT::ANKLER ];
			CGameItemPtr item = createItemInInventoryFreeSlot( INVENTORIES::bag, _Items.AnkleR.Quality, 1, _Items.AnkleR.IdSheet );
			if( item != 0 )
			{
				equipCharacter( INVENTORIES::equipment, SLOT_EQUIPMENT::ANKLER, item->getInventorySlot() );
			}
		}
	}
}

//-----------------------------------------------
// memorizeStartAction : memorize start action at create character
//-----------------------------------------------
void CCharacter::memorizeStartAction( const std::vector< CStaticRole::TMemorizedSentence >& MemorizedSentences )
{
	uint index = getFirstFreeSlotInKnownPhrase();

	for( vector< CStaticRole::TMemorizedSentence >::const_iterator it = MemorizedSentences.begin(); it != MemorizedSentences.end(); ++it )
	{
		const CStaticRolemasterPhrase * phrase = CSheets::getSRolemasterPhrase( (*it).sentence );
		if( phrase == 0 )
		{
			index = getFirstFreeSlotInKnownPhrase();
			continue;
		}

		bool mustMemorize = learnPrebuiltPhrase( (*it).sentence, index, true, !phrase->IsValidPhrase );
		if( !mustMemorize || !phrase->IsValidPhrase )
			continue;

		if ( !phrase->Bricks.empty() )
		{
			const CStaticBrick * brick = CSheets::getSBrickForm( phrase->Bricks[0] );
			if( brick )
			{
				BRICK_TYPE::EBrickType phraseType = BRICK_FAMILIES::brickType(brick->Family);
				if ((phraseType == BRICK_TYPE::FORAGE_PROSPECTION) || (phraseType == BRICK_TYPE::FORAGE_EXTRACTION) )
				{
					mustMemorize = false;
					_MemorizedPhrases.memorizeStarterPhrase( phrase->Bricks, index );
				}
			}
		}

		if( mustMemorize )
		{
			_MemorizedPhrases.memorizeStarterPhrase( phrase->Bricks, index );
		}

		index = getFirstFreeSlotInKnownPhrase();
	}
}


//
// Same but nearly empty
//
void CCharacter::setDummyStartCharacteristics()
{
}


//-----------------------------------------------
// updateRegen : update regen of character (new char or regen params change in cfg file)
//-----------------------------------------------
void CCharacter::updateRegen()
{
	for( uint i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		if( _IsInAComa || _IsDead )
		{
			if( i == SCORES::hit_points )
			{
				float currentRegen = - _PhysScores._PhysicalScores[ i ].Max / 240.0f; //TODO make time to variable
				_PhysScores._PhysicalScores[ i ].CurrentRegenerate = currentRegen;
			}
			else
			{
				float currentRegen = - _PhysScores._PhysicalScores[ i ].Current / 120.0f; //TODO make time to variable
				_PhysScores._PhysicalScores[ i ].CurrentRegenerate = currentRegen;
			}
		}
		else
		{
			// normal regen
			switch( i )
			{
			case SCORES::hit_points:
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::metabolism ].Base / RegenDivisor;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			case SCORES::sap:
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::wisdom ].Base / RegenDivisor;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			case SCORES::stamina:
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::well_balanced ].Base / RegenDivisor;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			case SCORES::focus:
				_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::will ].Base / RegenDivisor;
				_PhysScores._PhysicalScores[ i ].BaseRegenerateAction = _PhysScores._PhysicalScores[ i ].BaseRegenerateRepos / RegenReposFactor;
				break;
			default:;
			}

			// add regen offset
			_PhysScores._PhysicalScores[ i ].BaseRegenerateRepos += RegenOffset;
			_PhysScores._PhysicalScores[ i ].BaseRegenerateAction += RegenOffset;

			_PhysScores._PhysicalScores[ i ].CurrentRegenerate = _PhysScores._PhysicalScores[ i ].BaseRegenerateAction;
		}
	}
}


//-----------------------------------------------
// updateScoresInDatabase
//-----------------------------------------------
void CCharacter::updateScoresInDatabase()
{
	for( int s = 0; s < SCORES::NUM_SCORES; ++s )
	{
//		_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.BaseScore[s], _PhysScores._PhysicalScores[ s ].Base, true );
		CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(s).setBase(_PropertyDatabase, _PhysScores._PhysicalScores[ s ].Base, true );
//		_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.MaxScore[s], _PhysScores._PhysicalScores[ s ].Max, true );
		CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(s).setMax(_PropertyDatabase, _PhysScores._PhysicalScores[ s ].Max, true );
//		_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.BaseRegen[s], (uint32)(_PhysScores._PhysicalScores[ s ].BaseRegenerateAction * 10.0f), true );
		CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(s).setBaseRegen(_PropertyDatabase, uint32(_PhysScores._PhysicalScores[ s ].BaseRegenerateAction * 10.0f), true );
//		_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.Regen[s], (uint32)(_PhysScores._PhysicalScores[ s ].CurrentRegenerate * 10.0f), true );
		CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(s).setRegen(_PropertyDatabase, uint32(_PhysScores._PhysicalScores[ s ].CurrentRegenerate * 10.0f), true );
	}
}


//-----------------------------------------------
// setDatabase : Set all client database
//-----------------------------------------------
void CCharacter::setDatabase()
{
	_DbUpdateTimer.setRemaining( 1, new CCharacterDbUpdateTimerEvent( this ),1 );

	// Update characteristics database part
	for( int c = 0; c < CHARACTERISTICS::NUM_CHARACTERISTICS; ++c )
	{
//		_PropertyDatabase.setProp( string("CHARACTER_INFO:CHARACTERISTICS:") + CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics) c ), _PhysCharacs._PhysicalCharacteristics[ c ].Current, true );
		CBankAccessor_PLR::getCHARACTER_INFO().getCHARACTERISTICS(c).setVALUE(_PropertyDatabase, checkedCast<uint16>(_PhysCharacs._PhysicalCharacteristics[ c ].Current()), true );
//		egs_chinfo("<CCharacter::setDatabase> CHARACTER_INFO:CHARACTERISTICS:%s %d", CHARACTERISTICS::toString( (CHARACTERISTICS::TCharacteristics) c ).c_str(), _PhysCharacs._PhysicalCharacteristics[ c ].Current() );
	}

	updateRegen();

	updateScoresInDatabase();

	for( int skill = 0; skill < SKILLS::NUM_SKILLS; ++skill )
	{
//		_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.BaseSkill[skill], _Skills._Skills[ skill ].Base );
		CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(skill).setBaseSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ skill ].Base) );
//		_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.Skill[skill], _Skills._Skills[ skill ].Current );
		CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(skill).setSKILL(_PropertyDatabase, checkedCast<uint16>(_Skills._Skills[ skill ].Current) );
		if( _Skills._Skills[ skill ].XpNextLvl )
		{
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SKILLS.ProgressBar[skill], (uint8) ( _Skills._Skills[ skill ].Xp * 255 / _Skills._Skills[ skill ].XpNextLvl ) );
			CBankAccessor_PLR::getCHARACTER_INFO().getSKILLS().getArray(skill).setPROGRESS_BAR(_PropertyDatabase, checkedCast<uint8>(std::min(255.,_Skills._Skills[ skill ].Xp * 255 / _Skills._Skills[ skill ].XpNextLvl)) );
		}
	}

	// update money part of database
//	_PropertyDatabase.setProp( "INVENTORY:MONEY", _Money );
	CBankAccessor_PLR::getINVENTORY().setMONEY(_PropertyDatabase, _Money );

	// update known brick part of database
	// Known Bricks
	vector<CSheetId> brickToRemove;
	//for ( map< CSheetId, CKnownBrickInfo >::iterator itb = _KnownBricks.begin() ; itb != _KnownBricks.end() ; ++itb )
	for ( set<CSheetId>::iterator itb = _KnownBricks.begin() ; itb != _KnownBricks.end() ; ++itb )
	{
		const CStaticBrick* brickForm = CSheets::getSBrickForm( (*itb) );
		if( brickForm )
		{
			// update the database
			sint64 pos = (sint64)brickForm->IndexInFamily;
			if ( INVALID_POSITION_ID < pos)
				pos -=1;
			else if ( INVALID_POSITION_ID == pos)
				continue;

			_BrickFamilyBitField[brickForm->Family] |= ( (sint64)1 << (sint64)pos);

			// unlock the relative interface if it's the first brick of this type
			INTERFACE_FLAGS::TInterfaceFlag flag = INTERFACE_FLAGS::Unknown;
			switch( BRICK_FAMILIES::brickType(brickForm->Family) )
			{
			case BRICK_TYPE::COMBAT:
				flag = INTERFACE_FLAGS::Combat;
				break;
			case BRICK_TYPE::MAGIC:
				flag = INTERFACE_FLAGS::Magic;
				break;
			case BRICK_TYPE::COMMERCE:
				flag = INTERFACE_FLAGS::Commerce;
				break;
			default:
				flag = INTERFACE_FLAGS::Special;
				break;
			};

			_InterfacesFlagsBitField |= SINT64_CONSTANT(1) << uint8(flag);
		}
		else
		{
			nlwarning("<CCharacter::setDatabase> ERROR static form of idSheet (%s) missing", (*itb).toString().c_str() );
			brickToRemove.push_back( *itb );
		}
	}

	for(uint i = 0; i < BRICK_FAMILIES::NbFamilies; ++i )
//		_PropertyDatabase.setProp(_DataIndexReminder->KnownBricksFamilies[i], _BrickFamilyBitField[i], true);
		CBankAccessor_PLR::getBRICK_FAMILY().getArray(i).setBRICKS(_PropertyDatabase, _BrickFamilyBitField[i], true);

//	_PropertyDatabase.setProp( "INTERFACES:FLAGS", _InterfacesFlagsBitField, true );
	CBankAccessor_PLR::getINTERFACES().setFLAGS(_PropertyDatabase, _InterfacesFlagsBitField, true );

	for (uint j = 0 ; j < brickToRemove.size() ; ++j)
		_KnownBricks.erase( brickToRemove[j] );

	// set skill points
//	_PropertyDatabase.setProp("USER:SKILL_POINTS", (uint32)(_Skills._Sp));
	CBankAccessor_PLR::getUSER().setSKILL_POINTS(_PropertyDatabase, (uint32)(_Skills._Sp));
	for (uint i = 0 ; i < EGSPD::CSPType::EndSPType ; ++i)
//		_PropertyDatabase.setProp( std::string("USER:SKILL_POINTS_" ) + NLMISC::toString(i) +std::string(":VALUE"), (uint32)(_SpType[i]));
		CBankAccessor_PLR::getUSER().getSKILL_POINTS_(i).setVALUE(_PropertyDatabase, (uint32)(_SpType[i]));

	// setup the inventory updater for maximum performance
	_InventoryUpdater.reserve( INVENTORIES::CInventoryCategoryForCharacter::Bag, INVENTORIES::NbBagSlots*2 );
	const vector<CPetAnimal>& pets = getPlayerPets(); // the indices in this vector must match the indices of the pet_animal inventories
	for ( uint i=0; i!=pets.size(); ++i )
	{
		if ( pets[i].PetStatus != CPetAnimal::not_present )
			_InventoryUpdater.reserve( (INVENTORIES::CInventoryCategoryForCharacter::TInventoryId)(INVENTORIES::CInventoryCategoryForCharacter::Packers+i), INVENTORIES::NbPackerSlots*2 );
	}
	if ( getRoomInterface().isValid() )
		_InventoryUpdater.reserve( INVENTORIES::CInventoryCategoryForCharacter::Room, INVENTORIES::NbRoomSlots*2 );

	// fill the inventory updater & classic database with initial state
	initInventoriesDb();

//	_PropertyDatabase.setProp("DEFENSE:DEFENSE_MODE", !_DodgeAsDefense);
	CBankAccessor_PLR::getDEFENSE().setDEFENSE_MODE(_PropertyDatabase, !_DodgeAsDefense);
	protectedSlot(_ProtectedSlot);

	updateMagicProtectionAndResistance(); // update magic protections and resistances

	_DeathPenalties->updataDb(*this);

//	_PropertyDatabase.setProp( "USER:HAIR_COLOR",_HairColor );
	CBankAccessor_PLR::getUSER().setHAIR_COLOR(_PropertyDatabase, _HairColor );
//	_PropertyDatabase.setProp( "USER:HAIR_TYPE",_HairType );
	CBankAccessor_PLR::getUSER().setHAIR_TYPE(_PropertyDatabase,_HairType );

	setPVPFlagDatabase();

	// write new allegiance in database
//	_PropertyDatabase.setProp("FAME:CULT_ALLEGIANCE", _DeclaredCult);
	CBankAccessor_PLR::getFAME().setCULT_ALLEGIANCE(_PropertyDatabase, _DeclaredCult);
	// write new allegiance in database
//	_PropertyDatabase.setProp("FAME:CIV_ALLEGIANCE", _DeclaredCiv);
	CBankAccessor_PLR::getFAME().setCIV_ALLEGIANCE(_PropertyDatabase, _DeclaredCiv);
	// activate effects active on character
	_PersistentEffects.activate();
	// activate forbid power end date, infective aura end date and consumable overdose timer
	_ForbidPowerDates.activate();
	_IneffectiveAuras.activate();
	_ConsumableOverdoseEndDates.activate();
	// init the RRPs
	//RingRewardPoints.initDb();

}// setDatabase //


//-----------------------------------------------
// Eval Specialization for return Characteristics value
//-----------------------------------------------
CEvalNumExpr::TReturnState CCharacter::evalValue (const char *value, double &result, uint32 userData)
{
	string expression( value );
	if( expression.find( "Characteristics." ) != string::npos )
	{
		string characteristic = expression.substr( expression.find( "." ) + 1 );
		characteristic = characteristic.substr( 0, characteristic.find( "\"") );
		CHARACTERISTICS::TCharacteristics num_characteristic = CHARACTERISTICS::toCharacteristic( characteristic );
		if( num_characteristic != CHARACTERISTICS::Unknown )
		{
			result = (double) _PhysCharacs._PhysicalCharacteristics[ num_characteristic ].Base;
			return CEvalNumExpr::NoError;
		}
	}
	else if( expression.find( "Scores." ) != string::npos )
	{
		string score = expression.substr( expression.find( "." ) + 1 );
		score = score.substr( 0, score.find( "\"") );
		uint16 num_score = SCORES::toScore( score );
		if( num_score != SCORES::unknown )
		{
			result = (double) _PhysScores._PhysicalScores[ num_score ].Base;
			return CEvalNumExpr::NoError;
		}
	}
	return CEvalNumExpr::UnknownValue;
}

//-----------------------------------------------------------------------------
void CCharacter::startTradeItemSession( uint16 session )
{

	// Start a new trade sesssion
	_CurrentTradeSession = session;

	// Delete any current TradeItem shopping list
	if( _ShoppingList != 0 ) delete _ShoppingList;
	_ShoppingList = 0;

	// get the bot trade list
	CCreature * bot = CreatureManager.getCreature( _CurrentInterlocutor );
	if ( bot == NULL )
	{
		nlwarning( "<CCharacter startTradeItemSession> Invalid bot %s",_CurrentInterlocutor.toString().c_str() );
		return;
	}

	// get player fame with bot
	sint32 fame = CFameInterface::getInstance().getFameIndexed( _Id, bot->getForm()->getFaction() );
	if ( fame == NO_FAME )
	{
		nlwarning("fame %u is INVALID",(uint)bot->getRace() );
		fame = MinFameToTrade;
	}
	
	if ( (bot->getOrganization() == 0 && fame < MinFameToTrade) || (bot->getOrganization() != 0 && bot->getOrganization() != getOrganization()) )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::bot);
		params[0].setEIdAIAlias( _CurrentInterlocutor, CAIAliasTranslator::getInstance()->getAIAlias(_CurrentInterlocutor) );
		uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId,"EGS_TRADE_BAD_FAME",params );
		npcTellToPlayerEx( bot->getEntityRowId(),_EntityRowId,txt );
		return;
	}
	else if (bot->getOrganization() != 0 && bot->getOrganization() == getOrganization())
		fame = 0;


	float fameFactor = 1.0f;
	if(bot->getForm()->getFaction() != CStaticFames::INVALID_FACTION_INDEX)
	{
		fameFactor += CShopTypeManager::getFamePriceFactor( fame );
	}

	_ShoppingList = new CCharacterShoppingList( bot->getMerchantPtr(), *this, fameFactor );

	_ShoppingList->mountShoppingList( _CurrentContinent );
	_ShoppingList->initPageToUpdate( NB_SLOT_PER_PAGE );
	_ShoppingList->fillTradePage( _CurrentTradeSession );

	// set sell item price
	// invert fame price factor as bot buy price increase when fame increase
	fameFactor = 2.0f - fameFactor;
//	_PropertyDatabase.setProp( "TRADING:FAME_PRICE_FACTOR", uint32(fameFactor * 10000) );
	CBankAccessor_PLR::getTRADING().setFAME_PRICE_FACTOR(_PropertyDatabase, uint16(fameFactor * 10000) );
}

//-----------------------------------------------
// set filter and refresh trade list if trade occurs
//-----------------------------------------------
void CCharacter::setFilters( uint32 minQuality, uint32 maxQuality, uint32 minPrice, uint32 maxPrice, RM_CLASS_TYPE::TRMClassType minClass, RM_CLASS_TYPE::TRMClassType maxClass, RM_FABER_TYPE::TRMFType itemPartFilter, ITEM_TYPE::TItemType itemTypeFilter )
{
	if( itemPartFilter < RM_FABER_TYPE::NUM_FABER_TYPE )
		_RawMaterialItemPartFilter = itemPartFilter;
	else
		_RawMaterialItemPartFilter = RM_FABER_TYPE::Unknown;

	if( itemTypeFilter < ITEM_TYPE::NB_ITEM_TYPE )
		_ItemTypeFilter = itemTypeFilter;
	else
		_ItemTypeFilter = ITEM_TYPE::UNDEFINED;

	_MinClass = minClass;

	if( maxClass == RM_CLASS_TYPE::Supreme )
		_MaxClass = RM_CLASS_TYPE::Unknown;
	else
		_MaxClass = maxClass;

	if( _MinClass > _MaxClass )
	{
		_MinClass = RM_CLASS_TYPE::Unknown;
		_MaxClass = RM_CLASS_TYPE::Unknown;
	}

	_MinQualityFilter = minQuality;
	_MaxQualityFilter = maxQuality;
	_MinPriceFilter = minPrice;
	_MaxPriceFilter = maxPrice;

	if( _MinQualityFilter > _MaxQualityFilter )
	{
		_MinQualityFilter = 0;
		_MaxQualityFilter = ~0u;
	}

	if( _MinPriceFilter > _MaxPriceFilter )
	{
		_MaxPriceFilter = 0;
		_MaxPriceFilter = ~0u;
	}

	if( _CurrentInterlocutor != CEntityId::Unknown )
		refreshTradeList();
}

//-----------------------------------------------
// start a trade action
//-----------------------------------------------
void CCharacter::startTradePhrases(uint16 session)
{
	H_AUTO(CCharacterStartTradePhrases);

	// Start a new trade session
	_CurrentTradeSession = session;

	// Delete any current TradeItem shopping list
	if( _ShoppingList != 0 ) delete _ShoppingList;
	_ShoppingList = 0;

	// get the bot creature
	CCreature * bot = CreatureManager.getCreature( _CurrentInterlocutor );
	if ( bot == NULL )
	{
		nlwarning( "<CCharacter startTradePhrases> Invalid bot %s",_CurrentInterlocutor.toString().c_str() );
		return;
	}

	// *** Check the player has sufficient fame to Trade with Bot.
	sint32 fame = CFameInterface::getInstance().getFameIndexed( _Id, bot->getForm()->getFaction() );
	if ( fame == NO_FAME )
	{
		nlwarning("fame %u is INVALID",(uint)bot->getRace() );
	}
	
	if ( (bot->getOrganization() == 0 && fame < MinFameToTrade) || (bot->getOrganization() != 0 && bot->getOrganization() != getOrganization()) )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::bot);
		params[0].setEIdAIAlias( _CurrentInterlocutor, CAIAliasTranslator::getInstance()->getAIAlias(_CurrentInterlocutor) );
		uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId,"EGS_TRADE_BAD_FAME",params );
		npcTellToPlayerEx( bot->getEntityRowId(),_EntityRowId,txt );
		return;
	}

	// *** Set right rolemaster flags and race in Database
	uint8 flags = 0;
	if( bot->isSellingCharacteristics() )
		flags |= (1<<ROLEMASTER_FLAGS::CaracteristicUpgrades);
	if( bot->isSellingFightAction() )
	{
		flags |= (1<<ROLEMASTER_FLAGS::FightActions);
		setRolemasterType(EGSPD::CSPType::Fight);
	}
	if( bot->isSellingMagicAction() )
	{
		flags |= (1<<ROLEMASTER_FLAGS::MagicActions);
		setRolemasterType(EGSPD::CSPType::Magic);
	}
	if( bot->isSellingHarvestAction() )
	{
		flags |= (1<<ROLEMASTER_FLAGS::ForageActions);
		setRolemasterType(EGSPD::CSPType::Harvest);
	}
	if( bot->isSellingCraftAction() )
	{
		flags |= (1<<ROLEMASTER_FLAGS::CraftActions);
		setRolemasterType(EGSPD::CSPType::Craft);
	}

	if (flags != 0)
		flags |= (1<<ROLEMASTER_FLAGS::SpecialPowers);

//	_PropertyDatabase.setProp( "TRADING:ROLEMASTER_FLAGS", flags );
	CBankAccessor_PLR::getTRADING().setROLEMASTER_FLAGS(_PropertyDatabase, flags );

	// get the phrase trade filter
	for( vector< uint32 >::const_iterator it = bot->getOriginShopSelector().begin(); it != bot->getOriginShopSelector().end(); ++it )
	{
		EGSPD::CPeople::TPeople people = ITEM_ORIGIN::itemOriginStringToPeopleEnum( CShopTypeManager::getCategoryName()[ *it ] );
		if (people != EGSPD::CPeople::Common)
		{
//			_PropertyDatabase.setProp( "TRADING:ROLEMASTER_RACE", people);
			CBankAccessor_PLR::getTRADING().setROLEMASTER_RACE(_PropertyDatabase, people);
			break;
		}
	}


	// *** Specific Phrase List?
	if (flags == 0)
	{
		// get the phrases this bot can specifically sell
		getPhrasesOfferedByBot(*bot, _CurrentPhrasesTradeList);

		// Start to fill the pages for trade
		if( ! _CurrentPhrasesTradeList.empty() )
		{
			// get nb of pages for trade list
			nlassert(NB_SLOT_PER_PAGE > 0);
			const uint totalNbElts = (uint)_CurrentPhrasesTradeList.size();

			const uint nbPagesItems = 0;
			const uint nbPagesTotal = (uint16)ceil( double(totalNbElts) / NB_SLOT_PER_PAGE );

			for (uint i = nbPagesItems ; i < nbPagesTotal ; ++i)
				_TradePagesToUpdate.push_back(i);

			fillTradePage(_CurrentTradeSession);
		}
		else
		{
			sendDynamicSystemMessage( _Id, "EGS_CANT_SELL_ANYTHING" );
		}
	}
	// *** Else start Generic Phrase Sell
	else
	{
		_CurrentPhrasesTradeList.clear();
//		_PropertyDatabase.setProp( "TRADING:SESSION", session );
		CBankAccessor_PLR::getTRADING().setSESSION(_PropertyDatabase, session );
//		_PropertyDatabase.setProp( "TRADING:PAGE_ID", 0 );
		CBankAccessor_PLR::getTRADING().setPAGE_ID(_PropertyDatabase, 0 );
	}

} // startTradePhrases //


//-----------------------------------------------
// getPhrasesOfferedByBot :
//-----------------------------------------------
void CCharacter::getPhrasesOfferedByBot(const CCreature &bot, vector<CTradePhrase> &phrases)
{
	// clear the result
	phrases.clear();

	// the trade list
	const std::vector<NLMISC::CSheetId>	&tradeList= bot.getExplicitActionTradeList();
	phrases.reserve( tradeList.size() );

	// run all phrases, take only the one the player can buy now
	EGSPD::CSPType::TSPType		newRoleMasterType= EGSPD::CSPType::Unknown;
	for(uint i=0;i<tradeList.size();i++)
	{
		NLMISC::CSheetId	phraseSheetId= tradeList[i];

		// Exclude the phrase if the player already knows it
		if ( _BoughtPhrases.find( phraseSheetId ) == _BoughtPhrases.end() )
		{
			const CStaticRolemasterPhrase	*phrase= CSheets::getSRolemasterPhrase( phraseSheetId );
			if(phrase)
			{
				bool	raceOk= true;
				// Check race, and Find the type of the phrase to deduce type of rolemaster
				// NB: this is ugly because will take the last phrase to deduce the rolemaster type
				for(uint j=0;j<phrase->Bricks.size();j++)
				{
					const CStaticBrick *staticBrick = CSheets::getSBrickForm( phrase->Bricks[j] );
					if ( staticBrick )
					{
						BRICK_TYPE::EBrickType bt= BRICK_FAMILIES::brickType(staticBrick->Family);
						if(bt==BRICK_TYPE::COMBAT)
							newRoleMasterType= EGSPD::CSPType::Fight;
						else if(bt==BRICK_TYPE::MAGIC)
							newRoleMasterType= EGSPD::CSPType::Magic;
						else if(bt==BRICK_TYPE::FABER)
							newRoleMasterType= EGSPD::CSPType::Craft;
						else if(bt==BRICK_TYPE::HARVEST ||
								bt==BRICK_TYPE::FORAGE_PROSPECTION ||
								bt==BRICK_TYPE::FORAGE_EXTRACTION )
							newRoleMasterType= EGSPD::CSPType::Harvest;

						// If filter bot race, and if not same race than the stanza requires
						if( bot.getFilterExplicitActionTradeByBotRace() &&
							staticBrick->CivRestriction!=EGSPD::CPeople::Common &&
							staticBrick->CivRestriction!=bot.getRace() )
						{
							raceOk= false;
							break;
						}

						// If filter player race, and if not same race than the stanza requires
						if( bot.getFilterExplicitActionTradeByPlayerRace() &&
							staticBrick->CivRestriction!=EGSPD::CPeople::Common &&
							staticBrick->CivRestriction!=getRace() )
						{
							raceOk= false;
							break;
						}
					}
				}

				// Exclude the phrase if it contains bricks that require a skill level or a brick that the player doesn't have
				if ( raceOk && isPlayerAllowedToGetAllBricksFromPhrase( _Id, *phrase, _Skills._Skills, _KnownBricks, 250, bot.getRace(), false ) )
				{
					// add the phrase to the one he can sell to the player
					phrases.push_back(CTradePhrase(phraseSheetId));
				}
			}
		}
	}

	// if none found, fallback to Fight (hack)
	if(newRoleMasterType==EGSPD::CSPType::Unknown)
		newRoleMasterType= EGSPD::CSPType::Fight;

	// if some defined in the bot, force to use this one (good way to do)
	if(bot.getExplicitActionSPType()!=EGSPD::CSPType::Unknown)
		newRoleMasterType= bot.getExplicitActionSPType();

	// and set this new type
	setRolemasterType(newRoleMasterType);

} // getPhrasesOfferedByBot //

//-----------------------------------------------
// removeItemFromShop
//-----------------------------------------------
void CCharacter::removeItemFromShop( uint32 identifier, uint32 quantity )
{
	_ItemsInShopStore->removeItem( identifier, quantity, false );
}

//-----------------------------------------------
// fillTradePage : fill a trade page
//
//-----------------------------------------------
void CCharacter::fillTradePage(uint16 session, bool enableBuildingLossWarning)
{
	// If the request session is not the same as the current one, discard this request
	if(session != _CurrentTradeSession)
		return;

	// Trade item?
	if( _ShoppingList != 0 )
	{
		_ShoppingList->fillTradePage( session );
		return;
	}

	// get page to update if any
	if (_TradePagesToUpdate.empty())
		return;

	uint16 page = (*_TradePagesToUpdate.begin());
	_TradePagesToUpdate.pop_front();
	const uint begin = page * NB_SLOT_PER_PAGE;

	uint end;
	bool hasNext;

	const uint totalNbElts = (uint)_CurrentPhrasesTradeList.size();

	if ( begin + NB_SLOT_PER_PAGE < totalNbElts )
	{
		end = begin + NB_SLOT_PER_PAGE;
		hasNext = true;
	}
	else
	{
		end = totalNbElts;
		hasNext = true;
	}

	// setup db
//	_PropertyDatabase.setProp( "TRADING:SESSION", session );
	CBankAccessor_PLR::getTRADING().setSESSION(_PropertyDatabase, session );
//	_PropertyDatabase.setProp( "TRADING:PAGE_ID", page );
	CBankAccessor_PLR::getTRADING().setPAGE_ID(_PropertyDatabase, checkedCast<uint8>(page) );
//	_PropertyDatabase.setProp( "TRADING:HAS_NEXT", hasNext );
	CBankAccessor_PLR::getTRADING().setHAS_NEXT(_PropertyDatabase, hasNext );
//	_PropertyDatabase.setProp( "TRADING:ROLEMASTER_FLAGS", 0 );
	CBankAccessor_PLR::getTRADING().setROLEMASTER_FLAGS(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "TRADING:ROLEMASTER_RACE", 0 );
	CBankAccessor_PLR::getTRADING().setROLEMASTER_RACE(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "TRADING:BUILDING_LOSS_WARNING", enableBuildingLossWarning );
	CBankAccessor_PLR::getTRADING().setBUILDING_LOSS_WARNING(_PropertyDatabase, enableBuildingLossWarning );

	// fill first part with the phrases to sell
	uint index = 0;
	for ( uint i = begin ; i < end; ++i )
	{
		{
			CTradePhrase & trade = _CurrentPhrasesTradeList[ i ];

			CBankAccessor_PLR::TTRADING::TArray itemElem = CBankAccessor_PLR::getTRADING().getArray(index);

//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SHEET",index ),  trade.SheetId == CSheetId::Unknown ? 0 : trade.SheetId.asInt() );
			itemElem.setSHEET(_PropertyDatabase, trade.SheetId);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:QUALITY",index  ),  0 );
			itemElem.setQUALITY(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:USER_COLOR",index  ),  1 );
			itemElem.setUSER_COLOR(_PropertyDatabase, 1);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:WEIGHT",index  ),  0 );
			itemElem.setWEIGHT(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:INFO_VERSION",index  ),  0 );
			itemElem.setINFO_VERSION(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE",index ),  0 );
			itemElem.setPRICE(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:CURRENCY",index ),  RYMSG::TTradeCurrency::invalid_val );
			itemElem.setCURRENCY(_PropertyDatabase, RYMSG::TTradeCurrency::invalid_val);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:MONEY_SHEET",index ),  CSheetId::Unknown.asInt() );
			itemElem.setMONEY_SHEET(_PropertyDatabase, CSheetId::Unknown);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RRP_LEVEL",index ),  0 );
			itemElem.setRRP_LEVEL(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:BASE_SKILL",index ),  0 );
			itemElem.setBASE_SKILL(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:FACTION_TYPE",index ),  0 );
			itemElem.setFACTION_TYPE(_PropertyDatabase, 0);

//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SLOT_TYPE",index  ),  0 );
			itemElem.setSLOT_TYPE(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:ENCHANT",index  ), 0 );
			itemElem.setENCHANT(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_CLASS_TYPE",index) , 0 );
			itemElem.setRM_CLASS_TYPE(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_FABER_STAT_TYPE",index) , 0 );
			itemElem.setRM_FABER_STAT_TYPE(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PREREQUISIT_VALID",index) , trade.SheetId == CSheetId::Unknown ? 0 : true );
			itemElem.setPREREQUISIT_VALID(_PropertyDatabase, trade.SheetId != CSheetId::Unknown);
			++index;
		}
	}

	// reset any remaining empty slots
	for ( ; index <  NB_SLOT_PER_PAGE; ++index )
	{
		CBankAccessor_PLR::TTRADING::TArray itemElem = CBankAccessor_PLR::getTRADING().getArray(index);

//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SHEET",index ),  0 );
		itemElem.setSHEET(_PropertyDatabase, CSheetId::Unknown);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:QUALITY",index  ), 0);
		itemElem.setQUALITY(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:USER_COLOR",index  ),  1 );
		itemElem.setUSER_COLOR(_PropertyDatabase, 1);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:WEIGHT",index  ),  0 );
		itemElem.setWEIGHT(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:INFO_VERSION",index  ),  0 );
		itemElem.setINFO_VERSION(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:CURRENCY",index ),  RYMSG::TTradeCurrency::invalid_val );
		itemElem.setCURRENCY(_PropertyDatabase, RYMSG::TTradeCurrency::invalid_val);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PRICE",index  ),  0 );
		itemElem.setPRICE(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RRP_LEVEL",index ),  0 );
		itemElem.setRRP_LEVEL(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:MONEY_SHEET",index ),  CSheetId::Unknown.asInt() );
		itemElem.setMONEY_SHEET(_PropertyDatabase, CSheetId::Unknown);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:BASE_SKILL",index ),  0 );
		itemElem.setBASE_SKILL(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:FACTION_TYPE",index ),  0 );
		itemElem.setFACTION_TYPE(_PropertyDatabase, 0);

//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:SLOT_TYPE",index  ),  0 );
		itemElem.setSLOT_TYPE(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:ENCHANT",index  ), 0 );
		itemElem.setENCHANT(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_CLASS_TYPE",index) , 0 );
		itemElem.setRM_CLASS_TYPE(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:RM_FABER_STAT_TYPE",index) , 0 );
		itemElem.setRM_FABER_STAT_TYPE(_PropertyDatabase, 0);
//		_PropertyDatabase.setProp( NLMISC::toString("TRADING:%u:PREREQUISIT_VALID",index) , 0 );
		itemElem.setPREREQUISIT_VALID(_PropertyDatabase, 0);
	}
}

//-----------------------------------------------
// buyItem : Buy an item in current selected page
//
//-----------------------------------------------
void CCharacter::buyItem( uint16 itemNumber, uint16 quantity )
{
	if( _ShoppingList == 0 )
	{
		nlwarning( "<buyItem> Character %s Receive a BOTCHAT:BUY message but _ShoppingList of character are NULL", _Id.toString().c_str() );
		return;
	}
	_ShoppingList->buyItem( itemNumber, quantity );
}

//-----------------------------------------------
// destroyItem : destroy an item in current selected page
//
//-----------------------------------------------
void CCharacter::destroySaleItem( uint16 itemNumber, uint16 quantity )
{
	TLogContext_Item_DestroySaleStore logContext(_Id);
	if( _ShoppingList == 0 )
	{
		nlwarning( "<destroyItem> Character %s Receive a BOTCHAT:DESTROY message but _ShoppingList of character are NULL", _Id.toString().c_str() );
		return;
	}
	_ShoppingList->destroyItem( itemNumber, quantity );
}

//-----------------------------------------------
// buyRolemasterPhrase
//-----------------------------------------------
bool CCharacter::buyRolemasterPhrase( const NLMISC::CSheetId &phraseId, uint16 knownPhraseIndex, bool testRestrictions )
{
	TLogContext_Character_BuyRolemasterPhrase logContext(_Id);

	bool ok = false;
	const CStaticRolemasterPhrase *staticPhrase = CSheets::getSRolemasterPhrase(phraseId);
	if (!staticPhrase)
	{
		nlwarning("Cannot find the static rolemaster phrase for sheet %s", phraseId.toString().c_str() );
		return false;
	}

	// compute phrase SP price
	uint32 spPrice = 0;

	for (uint j = 0 ; j < staticPhrase->Bricks.size() ; ++j)
	{
		if ( _KnownBricks.find(staticPhrase->Bricks[j]) == _KnownBricks.end() )
		{
			const CStaticBrick *brick = CSheets::getSBrickForm(staticPhrase->Bricks[j]);
			if (brick)
				spPrice += brick->SkillPointPrice;
		}
	}

	// check price can be paid
	if ( getSP(_RolemasterType) < spPrice )
	{
		// return information 'not enough skill points'
		sendDynamicSystemMessage( _Id, "EGS_NOT_ENOUGHT_SP" );
		return false;
	}
	//Bsi.append( StatPath, NLMISC::toString("[AA] %s %s %d", _Id.toString().c_str(), phraseId.toString().c_str(), spPrice) );
	//EgsStat.displayNL("[AA] %s %s %d", _Id.toString().c_str(), phraseId.toString().c_str(), spPrice);
//	EGSPD::buyAction(_Id, phraseId.toString(), spPrice);

	// check phrase requirements
	if (testRestrictions)
	{
		CCreature * bot = CreatureManager.getCreature( _CurrentInterlocutor );
		if ( bot == NULL )
		{
			nlwarning( "Invalid bot %s",_CurrentInterlocutor.toString().c_str() );
			return false;
		}

		if ( bot->getOriginShopSelector().empty() )
			return false;

		EGSPD::CPeople::TPeople people = EGSPD::CPeople::Common;
		for( vector< uint32 >::const_iterator it = bot->getOriginShopSelector().begin(); it != bot->getOriginShopSelector().end(); ++it )
		{
			people = ITEM_ORIGIN::itemOriginStringToPeopleEnum( CShopTypeManager::getCategoryName()[ *it ] );
			if (people != EGSPD::CPeople::Common)
				break;
		}

		if( ! isPlayerAllowedToGetAllBricksFromPhrase(_Id, *staticPhrase, _Skills._Skills, _KnownBricks, 250, people, false) )
		{
			nlwarning("Server refused buy sheet action for player %s, sheet %s, rolemaster race %s",
						_Id.toString().c_str(), phraseId.toString().c_str(), EGSPD::CPeople::toString(people).c_str() );
			return false;
		}
	}

	if ( ! learnPrebuiltPhrase(phraseId, knownPhraseIndex, false, (knownPhraseIndex==0) ) )
		return false;

	spendSP( (double)spPrice, _RolemasterType );

	BRICK_TYPE::EBrickType phraseType = BRICK_TYPE::UNKNOWN;
	if ( !staticPhrase->Bricks.empty() )
	{
		const CStaticBrick * brick = CSheets::getSBrickForm( staticPhrase->Bricks[0] );
		if( brick )
		{
			phraseType = BRICK_FAMILIES::brickType(brick->Family);
		}
	}

	string msgName;
	switch(phraseType)
	{
	case BRICK_TYPE::COMBAT:
		msgName = "PHRASE_BUY_COMBAT";
		break;
	case BRICK_TYPE::MAGIC:
		msgName = "PHRASE_BUY_MAGIC";
		break;
	case BRICK_TYPE::FABER:
		msgName = "PHRASE_BUY_FABER";
		break;
	default:
		break;
	};

	if (!msgName.empty())
	{
		// send message
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::item, STRING_MANAGER::integer);
		params[0].SheetId = phraseId;
		params[1].Int = spPrice;
		PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, msgName, params);
	}

	return true;
}


//-----------------------------------------------
// buyPhraseBySheet
//-----------------------------------------------
void CCharacter::buyPhraseBySheet( const NLMISC::CSheetId &phraseId, uint16 knownPhraseIndex )
{
	// buy the phrase
	bool ok = buyRolemasterPhrase(phraseId, knownPhraseIndex, true);

	// send the result to client if he tries to learn it in the ActionBook
	if (knownPhraseIndex != 0)
	{
		// CONFIRM_BUY uint16 bool
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( _Id );
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "PHRASE:CONFIRM_BUY", bms) )
		{
			nlwarning("Msg name PHRASE:CONFIRM_BUY not found");
			return;
		}
		bms.serial( knownPhraseIndex );
		bms.serial( ok );

		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
	}
}

//-----------------------------------------------
// buyPhraseByIndex
//-----------------------------------------------
void CCharacter::buyPhraseByIndex( uint8 botChatIndex, uint16 knownPhraseIndex )
{
	// Check the Buy
	bool ok = true;
	if (botChatIndex >= uint8(_CurrentPhrasesTradeList.size() ) )
		ok = false;

	// Old feature if the user is buying a guild rolemaster research option, treat this case separatly
	if ( _CurrentBotChatType == BOTCHATTYPE::GuildRoleMaster )
		ok = false;	// todo

	// buy the phrase
	CTradePhrase &phrase = _CurrentPhrasesTradeList[ botChatIndex ];
	if(ok)
		ok = buyRolemasterPhrase(phrase.SheetId, knownPhraseIndex, false);

	// send the result to client if he tries to learn it in the ActionBook
	if (knownPhraseIndex != 0)
	{
		// CONFIRM_BUY uint16 bool
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( _Id );
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "PHRASE:CONFIRM_BUY", bms) )
		{
			nlwarning("Msg name PHRASE:CONFIRM_BUY not found");
			return;
		}
		bms.serial( knownPhraseIndex );
		bms.serial( ok );

		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
	}

	// if ok then must refresh list and append new phrases
	if (ok)
	{
		// mark phrase as deleted (NB: already no more appear in the trade list, since hidden client-side)
		phrase.SheetId = CSheetId::Unknown;

		CCreature * bot = CreatureManager.getCreature( _CurrentInterlocutor );
		if ( bot == NULL )
		{
			nlwarning( "<CCharacter buyPhraseByIndex> Invalid bot %s",_CurrentInterlocutor.toString().c_str() );
			return;
		}

		// get new phrases offered by bot
		vector<CTradePhrase> newPhrases;
		getPhrasesOfferedByBot(*bot, newPhrases);

		// sort vectors for easier comparison
		vector<CTradePhrase> oldPhrases = _CurrentPhrasesTradeList;
		std::sort(oldPhrases.begin(), oldPhrases.end());
		std::sort(newPhrases.begin(), newPhrases.end());
		vector<CTradePhrase> phraseRemoved;

		// get new phrases if any
		const uint newSize = (uint)newPhrases.size();
		const uint oldSize = (uint)oldPhrases.size();
		uint oldI = 0;
		for (uint newI = 0 ; newI < newSize ; ++newI)
		{
			if (oldI < oldSize)
			{
				while( (oldI < oldSize) && (oldPhrases[oldI].SheetId < newPhrases[newI].SheetId))
				{
					if(oldPhrases[oldI].SheetId != CSheetId::Unknown)
						phraseRemoved.push_back(oldPhrases[oldI]);
					++oldI;
				}

				if (oldI < oldSize && oldPhrases[oldI].SheetId == newPhrases[newI].SheetId )
				{
					++oldI;
					continue;
				}
			}

			// this is a new phrase, add it
			_CurrentPhrasesTradeList.push_back(newPhrases[newI]);
		}

		// if some new phrases added or some phrase removed
		if(_CurrentPhrasesTradeList.size()>oldSize || phraseRemoved.size()>0)
		{
			// must start the send of pages, only if no one are currently on the way
			bool mustFillTradePage = _TradePagesToUpdate.empty();

			// clear entry in trade list for page removed and add corresponding page for update
			for(uint removed = 0; removed < phraseRemoved.size(); ++removed)
			{
				for(uint oldI = 0; oldI < _CurrentPhrasesTradeList.size(); ++oldI)
				{
					if(phraseRemoved[removed].SheetId == _CurrentPhrasesTradeList[oldI].SheetId)
					{
						_CurrentPhrasesTradeList[oldI].SheetId = CSheetId::Unknown;
						uint16 pageToUpdate = oldI / NB_SLOT_PER_PAGE;
						if(_TradePagesToUpdate.empty() || _TradePagesToUpdate.back() != pageToUpdate)
							_TradePagesToUpdate.push_back(pageToUpdate);
						break;
					}
				}
			}

			const uint16 oldNbPages = (uint16)ceil( double(oldSize) / NB_SLOT_PER_PAGE );

			if (!mustFillTradePage)
			{
				for (list<uint16>::iterator it = _TradePagesToUpdate.begin(); it != _TradePagesToUpdate.end(); ++it)
					nldebug("wait update for page %u/%u",*it,oldNbPages);
			}

			// update last trade page if already sent as a partial page
			if (oldSize && (oldSize % NB_SLOT_PER_PAGE))
			{
				if( (!_TradePagesToUpdate.empty() && _TradePagesToUpdate.back() != (oldNbPages - 1)) || _TradePagesToUpdate.empty() )
					_TradePagesToUpdate.push_back(oldNbPages - 1);
			}

			// add all new pages to vector
			const uint16 nbPages = (uint16)ceil( double(_CurrentPhrasesTradeList.size()) / NB_SLOT_PER_PAGE );
			for (uint16 i = oldNbPages ; i < nbPages ; ++i)
			{
				_TradePagesToUpdate.push_back(i);
			}

			// start to send pages if needed
			if (mustFillTradePage)
				fillTradePage(_CurrentTradeSession);
		}
	}
} // buyPhraseByIndex //


//-----------------------------------------------
// changeCharacteristic
//-----------------------------------------------
void CCharacter::changeCharacteristic( CHARACTERISTICS::TCharacteristics charac, sint16 mod )
{
	if (charac < 0 || charac >= CHARACTERISTICS::NUM_CHARACTERISTICS)
		return;

	_PhysCharacs._PhysicalCharacteristics[charac].Base += mod;
	_PhysCharacs._PhysicalCharacteristics[charac].Current = _PhysCharacs._PhysicalCharacteristics[charac].Current + mod;

	if( _PhysCharacs._PhysicalCharacteristics[charac].Base > MaxCharacteristicValue)
	{
		mod -= _PhysCharacs._PhysicalCharacteristics[charac].Base - MaxCharacteristicValue;
		_PhysCharacs._PhysicalCharacteristics[charac].Base = MaxCharacteristicValue;
	}

	if( _PhysCharacs._PhysicalCharacteristics[charac].Current > MaxCharacteristicValue)
		_PhysCharacs._PhysicalCharacteristics[charac].Current = MaxCharacteristicValue;

	switch( charac )
	{
		// NB : do not need to update the client database, it will be done in the next call to applyRegenAndClipCurrentValue()
		//		as we change base but not current (and so current will change and database will be updated)
	case CHARACTERISTICS::constitution:
		_PhysScores._PhysicalScores[ SCORES::hit_points ].Base += PhysicalCharacteristicsFactor * mod;
		break;
	case CHARACTERISTICS::metabolism:
		_PhysScores._PhysicalScores[ SCORES::hit_points ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[charac].Base / RegenDivisor;
		_PhysScores._PhysicalScores[ SCORES::hit_points ].BaseRegenerateAction = _PhysScores._PhysicalScores[ SCORES::hit_points ].BaseRegenerateRepos / RegenReposFactor;
		// add regen offset
		_PhysScores._PhysicalScores[ SCORES::hit_points ].BaseRegenerateRepos += RegenOffset;
		_PhysScores._PhysicalScores[ SCORES::hit_points ].BaseRegenerateAction += RegenOffset;
		break;
	case CHARACTERISTICS::intelligence:
		_PhysScores._PhysicalScores[ SCORES::sap ].Base += PhysicalCharacteristicsFactor * mod;
		break;
	case CHARACTERISTICS::wisdom:
		_PhysScores._PhysicalScores[ SCORES::sap].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[charac].Base / RegenDivisor;
		_PhysScores._PhysicalScores[ SCORES::sap].BaseRegenerateAction = _PhysScores._PhysicalScores[ SCORES::sap].BaseRegenerateRepos / RegenReposFactor;
		// add regen offset
		_PhysScores._PhysicalScores[ SCORES::sap ].BaseRegenerateRepos += RegenOffset;
		_PhysScores._PhysicalScores[ SCORES::sap ].BaseRegenerateAction += RegenOffset;
		break;
	case CHARACTERISTICS::strength:
		_PhysScores._PhysicalScores[ SCORES::stamina ].Base += PhysicalCharacteristicsFactor * mod;
		break;
	case CHARACTERISTICS::well_balanced:
		_PhysScores._PhysicalScores[ SCORES::stamina ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[charac].Base / RegenDivisor;
		_PhysScores._PhysicalScores[ SCORES::stamina ].BaseRegenerateAction = _PhysScores._PhysicalScores[ SCORES::stamina ].BaseRegenerateRepos/ RegenReposFactor;
		// add regen offset
		_PhysScores._PhysicalScores[ SCORES::stamina ].BaseRegenerateRepos += RegenOffset;
		_PhysScores._PhysicalScores[ SCORES::stamina ].BaseRegenerateAction += RegenOffset;
		break;
	case CHARACTERISTICS::dexterity:
		_PhysScores._PhysicalScores[ SCORES::focus ].Base += PhysicalCharacteristicsFactor * mod;
		break;
	case CHARACTERISTICS::will:
		_PhysScores._PhysicalScores[ SCORES::focus ].BaseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[charac].Base / RegenDivisor;
		_PhysScores._PhysicalScores[ SCORES::focus ].BaseRegenerateAction = _PhysScores._PhysicalScores[ SCORES::focus ].BaseRegenerateRepos / RegenReposFactor;
		// add regen offset
		_PhysScores._PhysicalScores[ SCORES::focus ].BaseRegenerateRepos += RegenOffset;
		_PhysScores._PhysicalScores[ SCORES::focus ].BaseRegenerateAction += RegenOffset;
		break;
	default:
		break;
	}

	// update scores in database
	updateScoresInDatabase();

	_HaveToUpdateItemsPrerequisit = true;
}

//-----------------------------------------------
// queryItemPrice
//
//-----------------------------------------------
bool CCharacter::queryItemPrice( const CGameItemPtr item, uint32& price )
{
	CSheetId sheet;
	uint16 quality;
	float wornFactor = 1.0;

	CGameItemPtr theItem;
	theItem = item;

	sheet = theItem->getSheetId();
	quality = theItem->quality();
	if ( theItem->maxDurability() )
		wornFactor = float(theItem->durability()) / float(theItem->maxDurability());
	price = (uint32) ( CShopTypeManager::computeBasePrice( theItem, quality ) * wornFactor * 0.02 );
	return true;
}

//-----------------------------------------------
// sellItem
//
//-----------------------------------------------
void CCharacter::sellItem( INVENTORIES::TInventory inv, uint32 slot, uint32 quantity, uint32 sellPrice )
{
	TLogContext_Item_Sell contextLog(_Id);
	CCreature* bot = CreatureManager.getCreature( _CurrentInterlocutor );
	if ( !bot || ((bot->getBotChatProgram() & (1<<BOTCHATTYPE::TradeItemFlag)) == 0) )
	{
		return;
	}

	if( inv >= INVENTORIES::NUM_INVENTORY)
	{
		nlwarning("<CCharacter:-:sellItem> bad inventory index sended by client");
		return;
	}

	if( inv >= INVENTORIES::pet_animal1 && inv <= INVENTORIES::pet_animal4 )
	{
		if( (_PlayerPets[ inv - INVENTORIES::pet_animal1 ].AnimalStatus & ANIMAL_STATUS::InventoryAvailableFlag) == false )
		{
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = inv - INVENTORIES::pet_animal1;
			sendDynamicSystemMessage( _Id, "ANIMAL_INVENTORY_INACCESSIBLE", params );
			return;
		}
	}

	// get player fame with bot
	sint32 fame = CFameInterface::getInstance().getFameIndexed( _Id, bot->getForm()->getFaction() );
	if ( fame == NO_FAME )
	{
		nlwarning("fame %u is INVALID",(uint)bot->getRace() );
		fame = MinFameToTrade;
	}

	if ( (bot->getOrganization() == 0 && fame < MinFameToTrade) || (bot->getOrganization() != 0 && bot->getOrganization() != getOrganization()) )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::bot);
		params[0].setEIdAIAlias( _CurrentInterlocutor, CAIAliasTranslator::getInstance()->getAIAlias(_CurrentInterlocutor) );

		uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId,"EGS_TRADE_BAD_FAME",params );
		npcTellToPlayerEx( bot->getEntityRowId(),_EntityRowId,txt );

		sendDynamicSystemMessage( _Id, "TRADE_FAME_TOO_LOW", params );

		return;
	}
	else if (bot->getOrganization() != 0 && bot->getOrganization() == getOrganization())
		fame = 0;

	CInventoryPtr child = _Inventory[ inv ];
	if( child->getSlotCount() > slot && child->getItem( slot ) != NULL )
	{
		CSheetId sheet;
		uint16 quality;
		CGameItemPtr item;
		float wornFactor = 1.0f;

		item = child->getItem(slot);
		if (item == NULL)
			return;
		if ( item->getLockCount() )
			return;
		sheet = item->getSheetId();
		quality = item->quality();
		quantity = min(quantity, item->getStackSize());
		if ( item->maxDurability() )
			wornFactor = float(item->durability()) / float(item->maxDurability());

		const CStaticItem* itemForm = CSheets::getForm( sheet );
		if ( !itemForm )
		{
			nlwarning("<CCharacter sellItem> character %s Invalid item sheet %s : the sheet is invalid",_Id.toString().c_str(),sheet.toString().c_str());
			return;
		}
		if ( !itemForm->DropOrSell )
		{
			nlwarning("<CCharacter sellItem> character %s try to sell item slot %u  sheet %s",
						_Id.toString().c_str(),
						slot,
						sheet.toString().c_str() );
			return;
		}

		// You cannot exchange genesis named items
		if (item->getPhraseId().find("genesis_") == 0)
		{
			nlwarning("Character %s tries to sell '%s'", _Id.toString().c_str(), item->getPhraseId().c_str() );
			return;
		}

		if( ! ITEMFAMILY::isSellableByPlayer( itemForm->Family ) )
		{
			nlwarning("<CCharacter sellItem> character %s try to sell an unsealable item %s, must not permited by client", _Id.toString().c_str(), sheet.toString().c_str() );
			return;
		}

		if( wornFactor != 1.0f && sellPrice != 0 )
		{
			nlwarning("<CCharacter sellItem> character %s try to sell an worn item %s (inv %d, slot %d), must that permited by client", _Id.toString().c_str(), sheet.toString().c_str(), inv, slot );
			return;
		}

		// compute bot item buy price
		uint32 basePrice;
		queryItemPrice( item, basePrice );

		float famePriceFactor = 1.0f - CShopTypeManager::getFamePriceFactor( fame );
		basePrice = uint32(basePrice * famePriceFactor);

		if( sellPrice < basePrice && sellPrice != 0 )
		{
			nlwarning("<CCharacter sellItem> character %s try to sell an item %s below it's base price (base price %d, sell price %d), must not permited by client", _Id.toString().c_str(), sheet.toString().c_str(), basePrice, sellPrice );
			return;
		}

		if (item->getRefInventory() == _Inventory[INVENTORIES::equipment])
		{
			nlwarning("<CCharacter sellItem> character %s try to sell an equipped item %s, must not permited by client", _Id.toString().c_str(), sheet.toString().c_str() );
			return;
		}

		if( quantity > 0 )
		{

			CGameItemPtr tempItem = removeItemFromInventory(inv, slot, quantity);
			if (tempItem == NULL)
			{
				nlwarning("Failed to remove %u item from inventory %u", quantity, inv);
				return;
			}
			item = tempItem;
			tempItem = 0;

			// if return false, addItemForSale delete item by IITemTrade destructor!
			if( sellPrice == 0 || _ItemsInShopStore->addItemForSale( sellPrice, basePrice, item, quantity ) )
			{
				giveMoney( basePrice * quantity );

				//output stats
				//Bsi.append( StatPath, NLMISC::toString("[VI] %s %s %u %u %u %u", _Id.toString().c_str(), sheet.toString().c_str(), quality, quantity, basePrice, sellPrice) );
				//EgsStat.displayNL("[VI] %s %s %u %u %u %u", _Id.toString().c_str(), sheet.toString().c_str(), quality, quantity, basePrice, sellPrice);
//				EGSPD::sellItem(_Id, sheet.toString(), quality, quantity, basePrice, sellPrice);
				log_Item_PutInSaleStore(item->getItemId());

				CMissionEventSellItem event(bot->getEntityRowId(),sheet,quantity,quality);
				processMissionEvent(event);

				// delete item if it's solded to market (= to NPC)
				if( sellPrice == 0 )
				{
					SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
					params[0].SheetId = item->getSheetId();
					params[1].Int = quantity;
					params[2].Int = basePrice * quantity;
					sendDynamicSystemMessage( _Id, "ITEM_SOLDED_TO_NPC", params );
					item.deleteItem();
				}
				else
				{
					SM_STATIC_PARAMS_4(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
					params[0].SheetId = item->getSheetId();
					params[1].Int = quantity;
					params[2].Int = basePrice * quantity;
					params[3].Int = sellPrice * quantity - basePrice * quantity;
					sendDynamicSystemMessage( _Id, "ITEM_PUT_IN_RESELLER", params );
				}
			}
			else
			{
				CGameItemPtr tmp = item->getItemCopy();
				if ( !addItemToInventory(inv, tmp) )
				{
					addItemToInventory(INVENTORIES::temporary, tmp);
				}

				if( _ItemsInShopStore->getNbItemInShopStore() >= NBMaxItemYoursSellDisplay )
				{
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
					params[0].Int = NBMaxItemYoursSellDisplay;
					sendDynamicSystemMessage( _Id, "ITEM_SHOP_LIMIT_REACHED", params );
				}
				else
				{
					SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
					params[0].SheetId = item->getSheetId();
					params[1].Int = quantity;
					params[2].Int = quality;
					sendDynamicSystemMessage( _Id, "ITEM_CANT_BE_SOLD", params );
				}
				item.deleteItem();
			}
		}
	}
}


//-----------------------------------------------------------------------------
void CCharacter::itemSolded( uint32 identifier, uint32 quantity, uint32 sellPrice, uint32 basePrice, const CEntityId& buyer, bool sellOffline )
{
	CSmartPtr< IItemTrade > itemTrade = _ItemsInShopStore->removeItem( identifier, quantity, sellOffline );
	if( itemTrade != 0 )
	{
		if( buyer != getId() )
		{
			if( sellPrice < basePrice )
			{
				nlwarning("sell price is smaller than base price, setting sell price = base price !");
				sellPrice = basePrice;
			}
			uint64 winMoney = (sellPrice - basePrice) * quantity;
			giveMoney( winMoney );

			SM_STATIC_PARAMS_6(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::entity, STRING_MANAGER::integer, STRING_MANAGER::integer, STRING_MANAGER::integer);
			params[0].SheetId = itemTrade->getSheetId();
			params[1].Int = quantity;
			params[2].setEIdAIAlias( buyer, CAIAliasTranslator::getInstance()->getAIAlias(buyer) );
			params[3].Int = (uint32)sellPrice * quantity;
			params[4].Int = (uint32)winMoney;
			params[5].Int = (uint32)basePrice * quantity;
			sendDynamicSystemMessage( _Id, "ITEM_SOLD", params );

//			PlayerManager.savePlayerActiveChar( PlayerManager.getPlayerId( _Id ) );
		}
	}
}

//-----------------------------------------------------------------------------
void CCharacter::itemReachMaximumSellStoreTime( uint32 identifier, uint32 quantity, bool sellOffline )
{
	TLogContext_Item_SaleStoreTimeout logContext(_Id);
	CSmartPtr<IItemTrade> itemTrade = _ItemsInShopStore->removeItem( identifier, quantity, sellOffline );
	if( itemTrade != 0 )
	{
		SM_STATIC_PARAMS_2(params, STRING_MANAGER::item, STRING_MANAGER::integer);
		params[0].SheetId = itemTrade->getSheetId();
		params[1].Int = quantity;
		sendDynamicSystemMessage( _Id, "ITEM_SOLD_TO_MARKET", params );

//		PlayerManager.savePlayerActiveChar( PlayerManager.getPlayerId( _Id ) );
	}
}

//-----------------------------------------------------------------------------
void CCharacter::checkSellStore()
{
	_ItemsInShopStore->checkSellStore( getId() );
}

//-----------------------------------------------------------------------------
void CCharacter::clearMissionHistories()
{
	_MissionHistories.clear();
}

//-----------------------------------------------
// addPact : Add a surviving pact (Kami or Karavan)
//
//-----------------------------------------------
void CCharacter::addPact( uint8 PactNature, uint8 PactType )
{
	if( PactNature <= GSPACT::Caravane && PactType <= GSPACT::Type5 )
	{
		CPact NewPact( PactNature, PactType );
		_Pact.push_back( NewPact );
		_NbSurvivePact = (uint8)_Pact.size();
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = PactType+1;
		sendDynamicSystemMessage(_Id, "OPS_PACT_GAIN_U", params);
//		sendMessageToClient( _Id, "OPS_PACT_GAIN_U", PactType + 1 );

		// Update pact nature and type part of database
//		uint8 PactNumber = 0;
//		char buffer[256];
//		for( vector< CPact >::const_iterator it = _Pact.begin(); it != _Pact.end(); ++it )
//		{
//			sprintf( buffer, "CHARACTER_INFO:PACTS%d:PACT_NATURE", PactNumber );
//			_PropertyDatabase.setProp( buffer, (*it).PactNature );
//			sprintf( buffer, "CHARACTER_INFO:PACTS%d:PACT_TYPE", PactNumber );
//			_PropertyDatabase.setProp( buffer, (*it).PactType );
//			++PactNumber;
//		}
	}
	else
		nlwarning("cannot add pact, nature = %d, type = %d",PactNature,PactType);
} // addPact //


//-----------------------------------------------
// spend : Spend money
//
//-----------------------------------------------
void CCharacter::spendMoney( const uint64 & price )
{
	if ( _Money < price  )
		setMoney(0);
	else
		setMoney(_Money - price);
//	_PropertyDatabase.setProp( "INVENTORY:MONEY", _Money );
} // spend //


//-----------------------------------------------
// giveMoney : Character gain money
//
//-----------------------------------------------
void CCharacter::giveMoney( const uint64 & money )
{
	setMoney(_Money + money);
//	_PropertyDatabase.setProp( "INVENTORY:MONEY", _Money );
}

//-----------------------------------------------
// setMoney : set money value
//
//-----------------------------------------------
void CCharacter::setMoney( const uint64 & money )
{
	if (money != _Money)
	{
		log_Item_Money(_Money, money);
		_Money = money;
//		_PropertyDatabase.setProp( "INVENTORY:MONEY", _Money );
		CBankAccessor_PLR::getINVENTORY().setMONEY(_PropertyDatabase, _Money );
	}
}

//-----------------------------------------------
// setFactionPoint : set the number of faction point dependant of the faction
//
//-----------------------------------------------
void CCharacter::setFactionPoint(PVP_CLAN::TPVPClan clan, uint32 nbPt, bool factionPVP)
{
	if (clan < PVP_CLAN::BeginClans)
		return;

	uint8 index = clan - PVP_CLAN::BeginClans;

	if (index > (PVP_CLAN::EndClans-PVP_CLAN::BeginClans))
		return;

	sint32 delta = sint32(nbPt) - sint32(_FactionPoint[index]);
	if (VerboseFactionPoint.get())
	{
		nlinfo("FactionPoint: player %s '%s' has now %u points for faction '%s' (delta=%+d)",
			_Id.toString().c_str(),
			_Name.toUtf8().c_str(),
			nbPt,
			PVP_CLAN::toString(clan).c_str(),
			delta
			);
	}

	_FactionPoint[index] = nbPt;
//	_PropertyDatabase.setProp( toString("USER:FACTION_POINTS_%d:VALUE", index), nbPt );
	CBankAccessor_PLR::getUSER().getFACTION_POINTS_(index).setVALUE(_PropertyDatabase, nbPt );

	// update spire effects & faction points for build spire
	if( factionPVP )
	{
//		CPVPFactionRewardManager::getInstance().removeTotemsEffects( this );
//		CPVPFactionRewardManager::getInstance().giveTotemsEffects( this );
		CPVPFactionRewardManager::getInstance().updateFactionPointPool( clan, delta );
	}
}

//-----------------------------------------------
// getFactionPoint : get the number of faction point given a faction
//
//-----------------------------------------------
uint32 CCharacter::getFactionPoint(PVP_CLAN::TPVPClan clan)
{
	if (clan < PVP_CLAN::BeginClans)
		return 0;

	uint8 index = clan - PVP_CLAN::BeginClans;

	if (index > (PVP_CLAN::EndClans-PVP_CLAN::BeginClans))
		return 0;

	return _FactionPoint[index];
}

//-----------------------------------------------------------------------------
void CCharacter::initFactionPointDb()
{
	for (uint i = 0 ; i < (PVP_CLAN::EndClans-PVP_CLAN::BeginClans+1); i++)
	{
//		_PropertyDatabase.setProp( toString("USER:FACTION_POINTS_%d:VALUE", i), _FactionPoint[i] );
		CBankAccessor_PLR::getUSER().getFACTION_POINTS_(i).setVALUE(_PropertyDatabase, _FactionPoint[i] );
	}
}


//-----------------------------------------------
// setPvpPoint : set the number of pvp point
//
//-----------------------------------------------
void CCharacter::setPvpPoint(uint32 nbPt)
{
	_PvpPoint = nbPt;
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(0).setVALUE(_PropertyDatabase, nbPt );

}

//-----------------------------------------------
// getPvpPoint : get the number of pvp point
//
//-----------------------------------------------
uint32 CCharacter::getPvpPoint()
{
	return _PvpPoint;
}

//-----------------------------------------------------------------------------
void CCharacter::initPvpPointDb()
{
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(0).setVALUE(_PropertyDatabase, _PvpPoint );
}

//-----------------------------------------------------------------------------
void CCharacter::setLangChannel(const string &lang) {
	_LangChannel = lang;
}

//-----------------------------------------------------------------------------
void CCharacter::setNewTitle(const string &title) {
	_NewTitle = title;
}

//-----------------------------------------------------------------------------
void CCharacter::setTagPvPA(const string &tag) {
	_TagPvPA = tag;
}

//-----------------------------------------------------------------------------
void CCharacter::setTagPvPB(const string &tag) {
	_TagPvPB = tag;
}

//-----------------------------------------------------------------------------
void CCharacter::setTagA(const string &tag) {
	_TagA = tag;
}

//-----------------------------------------------------------------------------
void CCharacter::setTagB(const string &tag) {
	_TagB = tag;
}

//-----------------------------------------------------------------------------
void CCharacter::setOrganization(uint32 org)
{
	if (org == _Organization)
		return;
	_Organization = org;
	_OrganizationStatus = 0;
	_OrganizationPoints = 0;
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(1).setVALUE(_PropertyDatabase, _Organization );
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(2).setVALUE(_PropertyDatabase, _OrganizationStatus );
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(3).setVALUE(_PropertyDatabase, _OrganizationPoints );
}

//-----------------------------------------------------------------------------
void CCharacter::setOrganizationStatus(uint32 status)
{
	_OrganizationStatus = status;
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(2).setVALUE(_PropertyDatabase, _OrganizationStatus );
}

//-----------------------------------------------------------------------------
void CCharacter::changeOrganizationStatus(sint32 status)
{
	if (status < 0 && abs(status) > (sint32)_OrganizationStatus)
		_OrganizationStatus = 0;
	else
		_OrganizationStatus += status;
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(2).setVALUE(_PropertyDatabase, _OrganizationStatus );
}

//-----------------------------------------------------------------------------
void CCharacter::changeOrganizationPoints(sint32 points)
{
	if (points < 0 && abs(points) > (sint32)_OrganizationPoints)
		_OrganizationPoints = 0;
	else
		_OrganizationPoints += points;
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(3).setVALUE(_PropertyDatabase, _OrganizationPoints );
}


//-----------------------------------------------------------------------------
void CCharacter::initOrganizationInfos()
{
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(1).setVALUE(_PropertyDatabase, _Organization );
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(2).setVALUE(_PropertyDatabase, _OrganizationStatus );
	CBankAccessor_PLR::getUSER().getRRPS_LEVELS(3).setVALUE(_PropertyDatabase, _OrganizationPoints );
}


//-----------------------------------------------------------------------------
void CCharacter::sendFactionPointGainMessage(PVP_CLAN::TPVPClan clan, uint32 fpGain)
{
	BOMB_IF(clan < PVP_CLAN::BeginClans || clan > PVP_CLAN::EndClans, "invalid pvp clan!", return);

	SM_STATIC_PARAMS_2(fpMsgParams, STRING_MANAGER::faction, STRING_MANAGER::integer);
	fpMsgParams[0].Enum = PVP_CLAN::getFactionIndex(clan);
	fpMsgParams[1].Int = sint32(fpGain);

	sendDynamicSystemMessage(_Id, "FACTION_POINT_GAIN", fpMsgParams);
}

//-----------------------------------------------------------------------------
void CCharacter::sendFactionPointGainKillMessage(PVP_CLAN::TPVPClan clan, uint32 fpGain, const NLMISC::CEntityId & victimId)
{
	BOMB_IF(clan < PVP_CLAN::BeginClans || clan > PVP_CLAN::EndClans, "invalid pvp clan!", return);

	SM_STATIC_PARAMS_3(fpMsgParams, STRING_MANAGER::faction, STRING_MANAGER::integer, STRING_MANAGER::player);
	fpMsgParams[0].Enum = PVP_CLAN::getFactionIndex(clan);
	fpMsgParams[1].Int = sint32(fpGain);
	fpMsgParams[2].setEId(victimId);

	sendDynamicSystemMessage(_Id, "FACTION_POINT_GAIN_KILL", fpMsgParams);
}

//-----------------------------------------------------------------------------
void CCharacter::sendFactionPointLoseMessage(PVP_CLAN::TPVPClan clan, uint32 fpLose)
{
	BOMB_IF(clan < PVP_CLAN::BeginClans || clan > PVP_CLAN::EndClans, "invalid pvp clan!", return);

	SM_STATIC_PARAMS_2(fpMsgParams, STRING_MANAGER::faction, STRING_MANAGER::integer);
	fpMsgParams[0].Enum = PVP_CLAN::getFactionIndex(clan);
	fpMsgParams[1].Int = sint32(fpLose);

	sendDynamicSystemMessage(_Id, "FACTION_POINT_LOSE", fpMsgParams);
}

//-----------------------------------------------------------------------------
void CCharacter::sendFactionPointCannotGainYetMessage(const NLMISC::CEntityId & victimId, uint32 remainingSeconds)
{
	uint32 minutes = remainingSeconds / 60;
	uint32 seconds = remainingSeconds % 60;

	SM_STATIC_PARAMS_3(fpMsgParams, STRING_MANAGER::player, STRING_MANAGER::integer, STRING_MANAGER::integer);
	fpMsgParams[0].setEId(victimId);
	fpMsgParams[1].Int = sint32(minutes);
	fpMsgParams[2].Int = sint32(seconds);

	sendDynamicSystemMessage(_Id, "FACTION_POINT_CANNOT_GAIN_YET", fpMsgParams);
}

//-----------------------------------------------
// exchangeProposal : invite an entity to exchange items
//
//-----------------------------------------------
void CCharacter::exchangeProposal()
{
	if (_CurrentInterlocutor != CEntityId::Unknown)
	{
		//one cannot exchange if you are already talking
		PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_ALREADY_TRADING");
		return;
	}
	CCharacter* c = PlayerManager.getChar(_Target());
	if (c)
	{
		TVectorParamCheck params;

		//test distance
		const double distanceSquare = pow(float( c->getState().X - _EntityState.X )/1000.0f,2) + pow(float( c->getState().Y - _EntityState.Y )/1000.0f,2);
		if( distanceSquare > MaxTalkingDistSquare )
		{
			params.resize(1);
			params[0].Type = STRING_MANAGER::player;
			params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_TOO_FAR", params);
			return;
		}

		//cannot propose a trade to someone busy
		if ( c->_CurrentInterlocutor != CEntityId::Unknown || c->_ExchangeAsker != CEntityId::Unknown )
		{
			params.resize(1);
			params[0].Type = STRING_MANAGER::player;
			params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
			PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_BUSY", params);
			return;
		}

		// If not a privileged player and in ignorelist, don't trade
		if ( !haveAnyPrivilege() && c->hasInIgnoreList( getId() ) )
		{
			params.resize(1);
			params[0].Type = STRING_MANAGER::player;
			params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
			CCharacter::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_DECLINE", params);
			return;
		}

		//set the target's exchange asker, and inform the target
		c->_ExchangeAsker = _Id;

		//send the appropriate string to the client
		TVectorParamCheck vect;
		STRING_MANAGER::TParam param;
		param.Type = STRING_MANAGER::player;
		param.setEId (_Id );
		vect.push_back(param);
		uint32 strId  =STRING_MANAGER::sendStringToClient(_Target(),"EXCHANGE_INVITATION",vect);

		//send the invitation msg to the client, with the id of the built string
		CMessage msgout( "IMPULSION_ID" );
		CBitMemStream bms;
		CEntityId targetId = c->getId();
		msgout.serial( targetId );
		if ( ! GenericMsgManager.pushNameToStream( "EXCHANGE:INVITATION", bms) )
		{
			nlwarning("<CCharacter exchangeProposal> Msg name EXCHANGE:INVITATION not found");
			return;
		}
		bms.serial(strId);
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(c->getId().getDynamicId()), msgout );
		CActionDistanceChecker::getInstance()->addPlayer( c->getEntityRowId(), getEntityRowId() );

		params.resize(1);
		params[0].Type = STRING_MANAGER::player;
		params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
		PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_SEND_PROPOSAL", params);
	}
	else
		nlwarning("invalid character DatasetRow %d", _Target().getIndex() );
} // exchangeProposal //

//-----------------------------------------------
// acceptExchangeInvitation : accept an exchange invitation
//
//-----------------------------------------------
void CCharacter::acceptExchangeInvitation()
{
	if (_CurrentInterlocutor != CEntityId::Unknown)
	{
		//one cannot exchange if you are already talking
		sendDynamicSystemMessage( _Id, "OPS_EXCHANGE_IMPOSSIBLE");
		return;
	}
	CCharacter* c = PlayerManager.getChar(_ExchangeAsker);
	if ( !c )
	{
		nlwarning("CCharacter::acceptExchangeInvitation : unknown character %s",_ExchangeAsker.toString().c_str());
		return;
	}

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);

	if (c->_CurrentInterlocutor != CEntityId::Unknown )
	{
		params[0].setEIdAIAlias( _ExchangeAsker, CAIAliasTranslator::getInstance()->getAIAlias(_ExchangeAsker) );
		PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_BUSY", params);
		//sendMessageToClient( _Id, "OPS_EXCHANGE_BUSY",_CurrentInterlocutor);
		return;
	}

	//test distance
	const double distanceSquare = pow(float( c->getState().X - _EntityState.X )/1000.0f,2) + pow(float( c->getState().Y - _EntityState.Y )/1000.0f,2);
	if( distanceSquare > MaxTalkingDistSquare )
	{
		params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
		CCharacter::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_TOO_FAR", params);
		params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
		CCharacter::sendDynamicSystemMessage(_Target(), "EXCHANGE_ACCEPT_TOO_FAR", params);
		return;
	}

	params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
	uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId, "EXCHANGE_TITLE_PLAYER", params );
//	_PropertyDatabase.setProp( "EXCHANGE:TEXT", txt );
	CBankAccessor_PLR::getEXCHANGE().setTEXT(_PropertyDatabase, txt );

	params[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
	txt = STRING_MANAGER::sendStringToClient( c->getEntityRowId(), "EXCHANGE_TITLE_PLAYER", params );
//	c->_PropertyDatabase.setProp( "EXCHANGE:TEXT", txt );
	CBankAccessor_PLR::getEXCHANGE().setTEXT(c->_PropertyDatabase, txt );


//	_PropertyDatabase.setProp( "EXCHANGE:BEGUN", 1 );
	CBankAccessor_PLR::getEXCHANGE().setBEGUN(_PropertyDatabase, true );
//	c->_PropertyDatabase.setProp( "EXCHANGE:BEGUN", 1 );
	CBankAccessor_PLR::getEXCHANGE().setBEGUN(c->_PropertyDatabase, true );

	//reset the trade inventories
//	_PropertyDatabase.setProp( "EXCHANGE:ACCEPTED", 0 );
	CBankAccessor_PLR::getEXCHANGE().setACCEPTED(_PropertyDatabase, false );
//	c->_PropertyDatabase.setProp( "EXCHANGE:ACCEPTED", 0 );
	CBankAccessor_PLR::getEXCHANGE().setACCEPTED(c->_PropertyDatabase, false );
//	_PropertyDatabase.setProp( "EXCHANGE:MONEY", 0 );
	CBankAccessor_PLR::getEXCHANGE().setMONEY(_PropertyDatabase, 0 );
//	c->_PropertyDatabase.setProp( "EXCHANGE:MONEY", 0 );
	CBankAccessor_PLR::getEXCHANGE().setMONEY(c->_PropertyDatabase, 0 );

//	_PropertyDatabase.setProp( "EXCHANGE:ID", 0 );
	CBankAccessor_PLR::getEXCHANGE().setID(_PropertyDatabase, 0 );
//	c->_PropertyDatabase.setProp( "EXCHANGE:ID", 0 );
	CBankAccessor_PLR::getEXCHANGE().setID(c->_PropertyDatabase, 0 );

	CActionDistanceChecker::getInstance()->removePlayer( getEntityRowId() );
	staticActionInProgress(true);

	_CurrentInterlocutor = _ExchangeAsker;
	_ExchangeAsker = CEntityId::Unknown;
	_ExchangeId = 0;
	_ExchangeMoney = (uint64) 0;

	c->_ExchangeMoney = (uint64) 0;
	c->_ExchangeId = 0;
	c->_CurrentInterlocutor = _Id;


	// init exchange views
	_ExchangeView = new CExchangeView;
	_ExchangeView->setCharacter(this);
	_ExchangeView->bindToInventory(_Inventory[INVENTORIES::bag]);

	c->_ExchangeView = new CExchangeView;
	c->_ExchangeView->setCharacter(c);
	c->_ExchangeView->bindToInventory(c->_Inventory[INVENTORIES::bag]);

	_ExchangeView->setInterlocutorView(c->_ExchangeView);
	c->_ExchangeView->setInterlocutorView(_ExchangeView);

} // acceptExchangeInvitation //

//-----------------------------------------------
// declineExchangeInvitation : decline an exchange invitation
//
//-----------------------------------------------
void CCharacter::declineExchangeInvitation()
{
	//the target decline the proposition, reset everything and inform the asker
	SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
	params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
	PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(_ExchangeAsker), "EXCHANGE_DECLINE", params);

	_ExchangeAsker = CEntityId::Unknown;
	CActionDistanceChecker::getInstance()->removePlayer( getEntityRowId() );
} // declineExchangeInvitation //


//-----------------------------------------------
// cancelExchangeInvitation : cancel an exchange invitation
//
//-----------------------------------------------
void CCharacter::cancelExchangeInvitation()
{
	_ExchangeAsker = CEntityId::Unknown;
	CMessage msgout( "IMPULSION_ID" );
	CBitMemStream bms;
	msgout.serial( _Id );
	if ( ! GenericMsgManager.pushNameToStream( "EXCHANGE:CLOSE_INVITATION", bms) )
	{
		nlwarning("<CCharacter cancelExchangeInvitation> Msg name EXCHANGE:CLOSE_INVITATION not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}// cancelExchangeInvitation //

//-----------------------------------------------
// abortExchange
//
//-----------------------------------------------
void CCharacter::abortExchange()
{
	CCharacter * c = PlayerManager.getChar(_CurrentInterlocutor);

	if (isExchanging() && c != NULL)
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
		params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );

		PHRASE_UTILITIES::sendDynamicSystemMessage(c->_EntityRowId, "EXCHANGE_END", params);
	}

	resetExchange();

} // abortExchange //

//-----------------------------------------------
// resetExchange
//
//-----------------------------------------------
void CCharacter::resetExchange()
{
	if (_ExchangeView == NULL)
		return;

	// interlocutor player
	CCharacter * c = NULL;
	if (_ExchangeView->getInterlocutorView() != NULL)
		c = _ExchangeView->getInterlocutorView()->getCharacter();


	_ExchangeView->clearExchangeView();
	if (c != NULL && c->_ExchangeView != NULL)
		c->_ExchangeView->clearExchangeView();

	_ExchangeView->unbindFromInventory();
	_ExchangeView->setInterlocutorView(NULL);
	if (c != NULL && c->_ExchangeView != NULL)
	{
		c->_ExchangeView->unbindFromInventory();
		c->_ExchangeView->setInterlocutorView(NULL);
	}

	// this should be last reference to the exchange views
	// they are deleted here
	_ExchangeView = NULL;
	if (c != NULL && c->_ExchangeView != NULL)
		c->_ExchangeView = NULL;

//	_PropertyDatabase.setProp("EXCHANGE:BEGUN", 0);
	CBankAccessor_PLR::getEXCHANGE().setBEGUN(_PropertyDatabase, false);
//	_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", 0);
	CBankAccessor_PLR::getEXCHANGE().setACCEPTED(_PropertyDatabase, false);
	_ExchangeAsker = CEntityId::Unknown;
	_ExchangeAccepted = false;
	_ExchangeMoney = (uint64) 0;

	// if interlocutor is a bot, leave it to endBotChat() method
	if (_CurrentInterlocutor.getType() == RYZOMID::player)
		_CurrentInterlocutor = CEntityId::Unknown;

	if (c != NULL)
	{
//		c->_PropertyDatabase.setProp("EXCHANGE:BEGUN", 0);
		CBankAccessor_PLR::getEXCHANGE().setBEGUN(c->_PropertyDatabase, 0);
//		c->_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", 0);
		CBankAccessor_PLR::getEXCHANGE().setACCEPTED(c->_PropertyDatabase, 0);
		c->_ExchangeAsker = CEntityId::Unknown;
		c->_CurrentInterlocutor = CEntityId::Unknown;
		c->_ExchangeAccepted = false;
		c->_ExchangeMoney = (uint64) 0;
	}
}

//-----------------------------------------------
// startBotChat
//-----------------------------------------------
CCreature *  CCharacter::startBotChat(BOTCHATTYPE::TBotChatFlags chatType)
{
	/// set this chat as the new static action and cancel others
	staticActionInProgress(true,STATIC_ACT_TYPES::BotChat);
	CCreature * bot = CreatureManager.getCreature(_Target());
	if (!bot)
	{
		nlwarning("<CCharacter startBotChat> invalid bot %s", _Target().toString().c_str());
		return NULL;
	}
	// Special case if it is an outpost building
	if (bot->getOutpostBuilding() != NULL)
	{
		const double distanceSquare = pow(float( bot->getState().X - _EntityState.X )/1000.0f,2) + pow(float( bot->getState().Y - _EntityState.Y )/1000,2);
		if( distanceSquare > MaxTalkingOutpostBuildingDistSquare )
		{
			return NULL;
		}
	}
	else
	{
		const double distanceSquare = pow(float( bot->getState().X - _EntityState.X )/1000.0f,2) + pow(float( bot->getState().Y - _EntityState.Y )/1000,2);
		if( distanceSquare > MaxTalkingDistSquare )
		{
			return NULL;
		}
	}
	_CurrentBotChatType = (uint8)chatType;

	// set the bot as the new interlocutor
	_CurrentInterlocutor = bot->getId();
	CharacterBotChatBeginEnd.BotChatStart.push_back( TheDataset.getDataSetRow(_Id) );
	CharacterBotChatBeginEnd.BotChatStart.push_back( TheDataset.getDataSetRow(_CurrentInterlocutor) );

	return bot;
}

//-----------------------------------------------
// endBotChat
//
//-----------------------------------------------
void CCharacter::endBotChat(bool newBotChat, bool closeDynChat)
{
	_TradePagesToUpdate.clear();

	if( _ShoppingList != 0 )
	{
		delete _ShoppingList;
		_ShoppingList = 0;
	}

	if ( _BotGift != NULL )
		clearBotGift();

	if ( (!_CurrentInterlocutor.isUnknownId()) && _CurrentInterlocutor.getType() != RYZOMID::player)
	{
		NLMISC::CEntityId botChatInterlocutor = _CurrentInterlocutor; // save interlocutore for AI botchat end
		_CurrentInterlocutor = CEntityId::Unknown; // must be done before removeAllUserDynChat() because it can open dyn chat back when processing events

		if ( !newBotChat)
		{
			//force the end of the bot chat on the client side
			CMessage msgout( "IMPULSION_ID" );
			CBitMemStream bms;
			msgout.serial( _Id );
			if ( ! GenericMsgManager.pushNameToStream( "BOTCHAT:FORCE_END", bms) )
			{
				nlwarning("<CCharacter endBotChat> Msg name BOTCHAT:FORCE_END not found");
				return;
			}
			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			sendMessageViaMirror( NLNET::TServiceId(_Id.getDynamicId()), msgout );

			if ( closeDynChat )
				CMissionManager::getInstance()->removeAllUserDynChat( this );
		}
		//Inform AI about bot chat end

		CharacterBotChatBeginEnd.BotChatEnd.push_back( TheDataset.getDataSetRow(_Id) );
		CharacterBotChatBeginEnd.BotChatEnd.push_back( TheDataset.getDataSetRow(botChatInterlocutor) );

		_CurrentBotChatListPage	= 0;

		// delete data specific to the current bot chat
		switch( _CurrentBotChatType )
		{
		case (uint8)BOTCHATTYPE::ChooseMissionFlag :
			_CurrentMissionList.clear();
			break;
		case (uint8)BOTCHATTYPE::TradeItemFlag :
		case (uint8)BOTCHATTYPE::TradePhraseFlag :
		case (uint8)BOTCHATTYPE::TradeTeleportFlag :
		case (uint8)BOTCHATTYPE::TradeBuildingOptions :
		case (uint8)BOTCHATTYPE::TradeOutpostBuilding :
			_CurrentPhrasesTradeList.clear();
			break;
		case (uint8)BOTCHATTYPE::CreateGuildFlag:
			{
				if ( !newBotChat)
				{
					NLNET::CMessage msgout( "IMPULSION_ID" );
					msgout.serial( _Id );
					CBitMemStream bms;
					if ( ! GenericMsgManager.pushNameToStream( "GUILD:ABORT_CREATION", bms) )
					{
						nlwarning("<CBuildingManager::addLiftRequest> Msg name GUILD:ASCENSOR not found");
						return;
					}
					msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
					sendMessageViaMirror( NLNET::TServiceId(_Id.getDynamicId()), msgout );
					break;
				}
			}
		default:
			// do nothing
			break;
		}
		_CurrentBotChatType = (uint8)BOTCHATTYPE::UnknownFlag;


		///\todo: we have to check the open bot chat flag and chck what we have to clear
	}
}

//-----------------------------------------------
// checkBotGift
//
//-----------------------------------------------
void CCharacter::checkBotGift()
{
	if (_BotGift == NULL)
	{
		nlwarning("Player %s has no bot gift!", _Id.toString().c_str());
		DEBUG_STOP;
		return;
	}

	CMission * mission = NULL;
	if (_BotGift->Type == MISSION_DESC::Solo)
	{
		mission = _Missions->getMissions( _BotGift->MissionAlias );
	}
	else if (_BotGift->Type == MISSION_DESC::Group)
	{
		CTeam * team = TeamManager.getRealTeam( _TeamId );
		if (!team)
		{
			nlwarning("Player %s has no team!", _Id.toString().c_str());
			return;
		}
		mission = team->getMissionByAlias( _BotGift->MissionAlias );
	}

	if (mission)
	{
		const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
		nlassert( templ );

		set<uint32>::iterator itSet;
		bool giftOk = false;
		for( itSet=_BotGift->StepsIndex.begin(); itSet!=_BotGift->StepsIndex.end(); ++itSet )
		{
			if ( templ->Steps[*itSet-1]->checkPlayerGift(mission,this) )
			{
				giftOk = true;
				break;
			}
		}
//		_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", giftOk);
		CBankAccessor_PLR::getEXCHANGE().setACCEPTED(_PropertyDatabase, giftOk);

	}
	else
//		_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", false);
CBankAccessor_PLR::getEXCHANGE().setACCEPTED(_PropertyDatabase, false);
}

//-----------------------------------------------
// clearBotGift
//
//-----------------------------------------------
void CCharacter::clearBotGift()
{
	// clear botgift/exchange state
	if ( _BotGift )
	{
		delete _BotGift;
		_BotGift = NULL;
	}

	// reset exchange
	resetExchange();
}

//-----------------------------------------------
// acceptExchange
//
//-----------------------------------------------
void CCharacter::acceptExchange(uint8 exchangeId)
{
	TLogContext_Item_Exchange logContext(_Id, _CurrentInterlocutor);
	if (_CurrentInterlocutor.getType() == RYZOMID::player)
	{
		CCharacter * c = PlayerManager.getChar(_CurrentInterlocutor);
		if (c == NULL)
		{
			nlwarning("CCharacter::acceptExchange : unknown character %s",_CurrentInterlocutor.toString().c_str());
			abortExchange();
			return;
		}

		//if the client had the good trade information
		if (isExchanging() && c->isExchanging() && exchangeId == c->_ExchangeId)
		{
			if (!tempInventoryEmpty())
			{
				PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_TEMP_INVENTORY_MUST_EMPTY");
				abortExchange();
				return;
			}

			if (c->_ExchangeAccepted)
			{
				log_Item_ExchangeWithChar();

				if (validateExchange())
				{
					if (_ExchangeMoney > _Money || c->_ExchangeMoney > c->_Money)
					{
						sendDynamicSystemMessage(TheDataset.getDataSetRow(_CurrentInterlocutor), "EXCHANGE_MONEY_ERROR");
						sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_MONEY_ERROR");
						abortExchange();
						return;
					}

					// both accepted the trade, inform them and reset their exchange status
					SM_STATIC_PARAMS_1(params, STRING_MANAGER::player);
					params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(TheDataset.getDataSetRow(_CurrentInterlocutor), "EXCHANGE_ACCEPTED", params);
					params[0].setEIdAIAlias( _CurrentInterlocutor, CAIAliasTranslator::getInstance()->getAIAlias(_CurrentInterlocutor) );
					PHRASE_UTILITIES::sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_ACCEPTED", params);

					// remove the items traded
					vector<CGameItemPtr> items1, items2;
					vector<CPetAnimal> exchangePlayerPets1, exchangePlayerPets2;

					{
						TLogContext_Item_Swap logContext(_Id);
						removeExchangeItems(items1, exchangePlayerPets1);
					}
					{
						TLogContext_Item_Swap logContext(c->_Id);
						c->removeExchangeItems(items2, exchangePlayerPets2);
					}

					// do the exchange
					{
						TLogContext_Item_Swap logContext(_Id);
						addExchangeItems(c, items2, exchangePlayerPets2);
					}
					{
						TLogContext_Item_Swap logContext(c->_Id);
						c->addExchangeItems(this, items1, exchangePlayerPets1);
					}

					// reset exchange
					resetExchange();

					/* Since the exchange succeeds, the players are now somewhat "linked".
						We must be sure that saving player A will save also player B, else if a crash happens,
						we may have duplication or loss of items
					*/
					addEntityToSaveWithMe(c->getId());
					c->addEntityToSaveWithMe(getId());
				}
			}
			else
			{
				// this player is the first to accept
				_ExchangeAccepted = true;
//				c->_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", 1);
				CBankAccessor_PLR::getEXCHANGE().setACCEPTED(c->_PropertyDatabase, true);
			}
		}
	}
	else if (_CurrentInterlocutor.getType() == RYZOMID::npc)
	{
		log_Item_ExchangeWithNPC();

		TLogContext_Item_Swap logContext(_Id);


		if (_ExchangeMoney > _Money)
		{
			sendDynamicSystemMessage(_EntityRowId, "EXCHANGE_MONEY_ERROR");
			abortExchange();
			return;
		}

		CTeam * team = TeamManager.getRealTeam( _TeamId );
		CGuild* guild = CGuildManager::getInstance()->getGuildFromId( _GuildId );
		if (_BotGift == NULL)
		{
			nlwarning("Player %s has no bot gift", _Id.toString().c_str());
			return;
		}

		MISSION_DESC::TMissionType type = _BotGift->Type;
		TAIAlias	missionAlias = _BotGift->MissionAlias;

		CMission * mission = NULL;
		if (type == MISSION_DESC::Solo)
		{
			mission = _Missions->getMissions( missionAlias );
		}
		else if (type == MISSION_DESC::Group)
		{
			if (team == NULL)
			{
				nlwarning("CCharacter::acceptExchange : character %s ->  no team",_Id.toString().c_str() );
				return;
			}
			mission = team->getMissionByAlias( missionAlias );
		}
		else if (type == MISSION_DESC::Guild)
		{
			if (guild == NULL)
			{
				nlwarning("CCharacter::acceptExchange : character %s ->  no guild",_Id.toString().c_str() );
				return;
			}
			mission = guild->getMissionByAlias( missionAlias );
		}

		vector<CGameItemPtr> vect;
		vector<CPetAnimal> exchangePlayerPets;

		removeExchangeItems(vect, exchangePlayerPets);

		if (mission)
		{
			// check if gift is ok for the step or for one of the "step any" steps
			uint32					stepIndex;
			vector<uint32>			quantities;
			bool					giftDone = false;
			set<uint32>::iterator	itSet;
			for( itSet=_BotGift->StepsIndex.begin(); itSet!=_BotGift->StepsIndex.end(); ++itSet )
			{
				stepIndex = *itSet;
				quantities.clear();
				giftDone = mission->itemGiftDone( this ,vect, stepIndex, quantities );
				if( giftDone )
					break;
			}
			if( !giftDone )
			{
				stepIndex = *_BotGift->StepsIndex.begin();

				std::list< CMissionEvent* > eventList;
				CMissionEventGiveMoney eventMoney( (uint32) _ExchangeMoney );
				// no item accepted : this is a give money
				eventList.push_back( &eventMoney );
				if ( type == MISSION_DESC::Solo )
					processMissionStepUserEvent( eventList,missionAlias,stepIndex );
				else if ( type == MISSION_DESC::Group )
					team->processTeamMissionStepEvent( eventList,missionAlias,stepIndex );
				else if ( type == MISSION_DESC::Guild )
					guild->processGuildMissionStepEvent( eventList,missionAlias,stepIndex );
				eventList.pop_front();
				for ( std::list< CMissionEvent* >::iterator it = eventList.begin(); it != eventList.end(); ++it  )
					processMissionEvent(*(*it));
			}
			else
			{
				for ( uint i = 0; i < quantities.size(); i++ )
				{
					if ( quantities[i] )
					{
						std::list< CMissionEvent* > eventList;
						CMissionEventGiveItem eventItem;
						eventItem.Quantity = quantities[i];
						eventItem.StepIndex = i;
						eventList.push_back( &eventItem );
						if ( type == MISSION_DESC::Solo )
							processMissionStepUserEvent( eventList,missionAlias,stepIndex );
						else if ( type == MISSION_DESC::Group )
							team->processTeamMissionStepEvent( eventList,missionAlias,stepIndex );
						else if ( type == MISSION_DESC::Guild )
							guild->processGuildMissionStepEvent( eventList,missionAlias,stepIndex );
						eventList.pop_front();
						for ( std::list< CMissionEvent* >::iterator it = eventList.begin(); it != eventList.end(); ++it  )
							processMissionEvent(*(*it));
					}
				}
			}
		}

		// reset exchange
		resetExchange();

		for (uint i = 0; i < vect.size(); i++ )
		{
			GameItemManager.destroyItem(vect[i]);
		}

		// remove given pet
		for( uint i = 0; i < exchangePlayerPets.size(); i++)
		{
			removeAnimal(exchangePlayerPets[i].ItemPtr, CPetCommandMsg::DESPAWN);
		}
	}
} // acceptExchange //

//-----------------------------------------------
// validateExchange: check if each character have enough room in bag for exchange
//
//-----------------------------------------------
bool CCharacter::validateExchange()
{
	CCharacter* c = PlayerManager.getChar(_CurrentInterlocutor);
	if (!c)
	{
		nlwarning("CCharacter::validateExchange : unknown character %s",_CurrentInterlocutor.toString().c_str());
		abortExchange();
		return false;
	}

	uint32 userGiveBulk = 0;
	uint32 userGiveWeight = 0;
	uint32 userGiveSlots = 0;
	uint32 userReceiveBulk = 0;
	uint32 userReceiveWeight = 0;
	uint32 userReceiveSlots = 0;
	bool   bExchangePacker = false;
	bool   bExchangeMount = false;
	CSheetId packerSheet;
	CSheetId mountSheet;

	// in the ring check for exploit attempts
	if (IsRingShard)
	{
		if (_ExchangeMoney > 0 || c->_ExchangeMoney > 0)
		{
			// see if the players are from the same home mainland
			if (!IInterShardExchangeValidator::getInstance()->isMoneyExchangeAllowed(getHomeMainlandSessionId(),c->getHomeMainlandSessionId())
				// && getHomeMainlandSessionId() != CSR_SHARD_ID	// allow CSR players to exchange items with normal players
				// && c->getHomeMainlandSessionId() != CSR_SHARD_ID	// allow CSR players to exchange items with normal players
				)
			{
				// not from the same home mainland
				sendDynamicSystemMessage( getId(), "INVALID_EXCHANGE_IN_RING" );
				sendDynamicSystemMessage( c->getId(), "INVALID_EXCHANGE_IN_RING" );

				invalidateExchange();
				return false;
			}

		}
	}

	for (uint i = 0; i < CExchangeView::NbExchangeSlots; i++)
	{
		CGameItemPtr item;
		uint32 exchangeQuantity;

		// compute bulk/weight/slots that user will give
		item = _ExchangeView->getExchangeItem(i, &exchangeQuantity);
		if (item != NULL)
		{
			userGiveBulk += item->getStackBulk(exchangeQuantity);
			userGiveWeight += item->getStackWeight(exchangeQuantity);
			userGiveSlots++;

			// in the ring check for exploit attempts
			if ( IsRingShard )
			{
				if (!IInterShardExchangeValidator::getInstance()->isExchangeAllowed(item,getHomeMainlandSessionId(), c->getHomeMainlandSessionId())
					// && getHomeMainlandSessionId() != CSR_SHARD_ID	// allow CSR players to exchange items with normal players
					// && c->getHomeMainlandSessionId() != CSR_SHARD_ID	// allow CSR players to exchange items with normal players
					)
				{
					// not from the same home mainland
					sendDynamicSystemMessage( getId(), "INVALID_EXCHANGE_IN_RING" );
					sendDynamicSystemMessage( c->getId(), "INVALID_EXCHANGE_IN_RING" );

					invalidateExchange();
					return false;
				}
			}

			if( item->getStaticForm()->Type == ITEM_TYPE::MEKTOUB_PACKER_TICKET )
			{
				bExchangePacker = true;
				packerSheet = item->getSheetId();
			}
			if( item->getStaticForm()->Type == ITEM_TYPE::MEKTOUB_MOUNT_TICKET )
			{
				bExchangeMount = true;
				mountSheet = item->getSheetId();
			}
		}

		// compute bulk/weight/slots that user will receive
		item = c->_ExchangeView->getExchangeItem(i, &exchangeQuantity);
		if (item != NULL)
		{
			userReceiveBulk += item->getStackBulk(exchangeQuantity);
			userReceiveWeight += item->getStackWeight(exchangeQuantity);
			userReceiveSlots++;

			// in the ring check for exploit attempts
			if ( IsRingShard )
			{
				if (!IInterShardExchangeValidator::getInstance()->isExchangeAllowed(item,getHomeMainlandSessionId(), c->getHomeMainlandSessionId())
						// && getHomeMainlandSessionId() != CSR_SHARD_ID	// allow CSR players to exchange items with normal players
						// && c->getHomeMainlandSessionId() != CSR_SHARD_ID	// allow CSR players to exchange items with normal players
						)
				{
					// not from the same home mainland
					sendDynamicSystemMessage( getId(), "INVALID_EXCHANGE_IN_RING" );
					sendDynamicSystemMessage( c->getId(), "INVALID_EXCHANGE_IN_RING" );

					invalidateExchange();
					return false;
				}
			}

			if( item->getStaticForm()->Type == ITEM_TYPE::MEKTOUB_PACKER_TICKET )
			{
				bExchangePacker = true;
				packerSheet = item->getSheetId();
			}
			if( item->getStaticForm()->Type == ITEM_TYPE::MEKTOUB_MOUNT_TICKET )
			{
				bExchangeMount = true;
				mountSheet = item->getSheetId();
			}
		}
	}

	// NOTE: to check that players have enough slots in their bag, we test the worse case

	const CInventoryPtr & userBag = _Inventory[INVENTORIES::bag];
	if (	userBag->getInventoryBulk() - userGiveBulk + userReceiveBulk > userBag->getMaxBulk()
		||	userBag->getInventoryWeight() - userGiveWeight + userReceiveWeight > userBag->getMaxWeight()
		||	userBag->getUsedSlotCount() + userReceiveSlots > userBag->getSlotCount())
	{
		// user is too encumbered
		sendDynamicSystemMessage( getId(), "TOO_ENCUMBERED_FOR_EXCHANGE" );

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
		sendDynamicSystemMessage( c->getId(), "INTERLOCUTOR_TOO_ENCUMBERED_FOR_EXCHANGE", params );

		invalidateExchange();
		return false;
	}

	const CInventoryPtr & interlocutorBag = c->_Inventory[INVENTORIES::bag];
	if (	interlocutorBag->getInventoryBulk() - userReceiveBulk + userGiveBulk > interlocutorBag->getMaxBulk()
		||	interlocutorBag->getInventoryWeight() - userReceiveWeight + userGiveWeight > interlocutorBag->getMaxWeight()
		||	interlocutorBag->getUsedSlotCount() + userGiveSlots > interlocutorBag->getSlotCount())
	{
		// interlocutor is too encumbered
		sendDynamicSystemMessage( c->getId(), "TOO_ENCUMBERED_FOR_EXCHANGE" );

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::entity);
		params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
		sendDynamicSystemMessage( getId(), "INTERLOCUTOR_TOO_ENCUMBERED_FOR_EXCHANGE", params );

		invalidateExchange();
		return false;
	}

	// Validate mektoub count
	if( bExchangePacker )
	{
		sint32 ticketDelta = c->_ExchangeView->getPetTicketExchanged( ITEM_TYPE::MEKTOUB_PACKER_TICKET ) - _ExchangeView->getPetTicketExchanged( ITEM_TYPE::MEKTOUB_PACKER_TICKET );
		if( !checkAnimalCount( packerSheet, false, ticketDelta ) )
		{
			sendDynamicSystemMessage(getId(), "ANIMAL_PLAYER_HAVE_MAX");
			c->sendDynamicSystemMessage(c->getId(), "ANIMAL_INTERLOCUTOR_HAVE_MAX");
			invalidateExchange();
			return false;
		}
		if( !c->checkAnimalCount( packerSheet, false, -ticketDelta ) )
		{
			c->sendDynamicSystemMessage(getId(), "ANIMAL_PLAYER_HAVE_MAX");
			sendDynamicSystemMessage(c->getId(), "ANIMAL_INTERLOCUTOR_HAVE_MAX");
			invalidateExchange();
			return false;
		}
	}

	if( bExchangeMount )
	{
		sint32 ticketDelta = c->_ExchangeView->getPetTicketExchanged(ITEM_TYPE::MEKTOUB_MOUNT_TICKET) - _ExchangeView->getPetTicketExchanged(ITEM_TYPE::MEKTOUB_MOUNT_TICKET);
		if( !checkAnimalCount( mountSheet, false, ticketDelta ) )
		{
			sendDynamicSystemMessage(getId(), "ANIMAL_PLAYER_HAVE_MAX");
			c->sendDynamicSystemMessage(c->getId(), "ANIMAL_INTERLOCUTOR_HAVE_MAX");
			invalidateExchange();
			return false;
		}
		if( !c->checkAnimalCount( mountSheet, false, -ticketDelta ) )
		{
			c->sendDynamicSystemMessage(getId(), "ANIMAL_PLAYER_HAVE_MAX");
			sendDynamicSystemMessage(c->getId(), "ANIMAL_INTERLOCUTOR_HAVE_MAX");
			invalidateExchange();
			return false;
		}
	}
	return true;
}

//-----------------------------------------------
// invalidateExchange
//
//-----------------------------------------------
void CCharacter::invalidateExchange()
{
	// trader can be NULL if exchange is done with a bot
	CCharacter * trader = PlayerManager.getChar( _CurrentInterlocutor );

	if (!isExchanging())
	{
		nlwarning("<CCharacter invalidateExchange> player %s is not exchanging! Interlocutor is %s", _Id.toString().c_str(), _CurrentInterlocutor.toString().c_str());
		return;
	}

	_ExchangeAccepted = false;
//	_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", 0);
	CBankAccessor_PLR::getEXCHANGE().setACCEPTED(_PropertyDatabase, 0);
//	_PropertyDatabase.setProp("EXCHANGE:FORCE_REFUSE", _PropertyDatabase.getProp("EXCHANGE:FORCE_REFUSE") + 1);
	CBankAccessor_PLR::getEXCHANGE().setFORCE_REFUSE(_PropertyDatabase, (CBankAccessor_PLR::getEXCHANGE().getFORCE_REFUSE(_PropertyDatabase) + 1) & 0xf);

	if (trader)
	{
		if (!trader->isExchanging())
		{
			nlwarning("<CCharacter invalidateExchange> interlocutor %s is not exchanging! Player is %s", _CurrentInterlocutor.toString().c_str(), _Id.toString().c_str());
			return;
		}
		trader->_ExchangeAccepted = false;
//		trader->_PropertyDatabase.setProp("EXCHANGE:ACCEPTED", 0);
		CBankAccessor_PLR::getEXCHANGE().setACCEPTED(trader->_PropertyDatabase, 0);
//		trader->_PropertyDatabase.setProp("EXCHANGE:FORCE_REFUSE", _PropertyDatabase.getProp("EXCHANGE:FORCE_REFUSE") + 1);
		CBankAccessor_PLR::getEXCHANGE().setFORCE_REFUSE(trader->_PropertyDatabase, (CBankAccessor_PLR::getEXCHANGE().getFORCE_REFUSE(trader->_PropertyDatabase) + 1) & 0xf);
	}
}

//-----------------------------------------------
// removeExchangeItems
//
//-----------------------------------------------
void CCharacter::removeExchangeItems(vector<CGameItemPtr >& itemRemoved, vector< CPetAnimal >& playerPetsRemoved)
{
	bool needUpdatePetDatabase = false;

	// get exchange items
	nlassert(_ExchangeView != NULL);
	_ExchangeView->validateExchange(&itemRemoved);

	// catch pet animal tickets
	for (uint i = 0; i < itemRemoved.size(); i++)
	{
		CGameItemPtr item = itemRemoved[i];
		if (item == NULL)
			continue;

		const CStaticItem * form = CSheets::getForm( item->getSheetId() );
		if (form && form->Family == ITEMFAMILY::PET_ANIMAL_TICKET)
		{
			for (uint p = 0; p < _PlayerPets.size(); p++)
			{
				if (_PlayerPets[p].ItemPtr == item)
				{
					playerPetsRemoved.push_back( _PlayerPets[p] );

					// reset player pet
					_PlayerPets[p].PetStatus = CPetAnimal::not_present;
					_PlayerPets[p].ItemPtr = 0;
					_PlayerPets[p].OwnerId = CEntityId::Unknown;
					_PlayerPets[p].PetSheetId = CSheetId::Unknown;
					_PlayerPets[p].TicketPetSheetId = CSheetId::Unknown;

					needUpdatePetDatabase = true;
					break;
				}
			}
		}
	}

	if (needUpdatePetDatabase)
		updatePetDatabase();

	spendMoney(_ExchangeMoney);

} // removeExchangeItems //

//-----------------------------------------------
// addExchangeItems
//
//-----------------------------------------------
void CCharacter::addExchangeItems(CCharacter* trader,vector<CGameItemPtr >& itemToAdd, vector< CPetAnimal >& playerPetsAdded)
{
	// inform AI
	CPetSetOwner msgAI;

	bool updatePetDataBase = false;

	for( uint32 p = 0; p < playerPetsAdded.size(); ++p )
	{
		sint32 i = getFreePetSlot();
		if( i >= 0 )
		{
			_PlayerPets[ i ] = playerPetsAdded[ p ];
			_PlayerPets[ i ].OwnerId = _Id;

			initPetInventory(i);

			if( _PlayerPets[ i ].PetStatus == CPetAnimal::waiting_spawn )
			{
				spawnCharacterAnimal( i );
			}
			else
			{
				setAnimalPeople( i );
			}

			updatePetDataBase = true;
			msgAI.OwnerMirrorRow = _EntityRowId;
			msgAI.PetMirrorRow = _PlayerPets[ i ].SpawnedPets;
			CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), msgAI);
		}
	}

	if( updatePetDataBase )
	{
		updatePetDatabase();
	}

	// update money
	giveMoney( trader->_ExchangeMoney );

	// add the items to the character bag
	for (uint i = 0; i < itemToAdd.size(); ++i)
	{
		if ( !addItemToInventory(INVENTORIES::bag, itemToAdd[i]) &&	!addItemToInventory(INVENTORIES::temporary, itemToAdd[i]) )
		{
			nlwarning("<CCharacter::addExchangeItems> Both Temp and bag are full for user %s. item index :'%u'. '%u'",_Id.toString().c_str(),i,itemToAdd.size() );
			itemToAdd[i].deleteItem();
		}
	}
}

//-----------------------------------------------
// exchangeMoney
//
//-----------------------------------------------
void CCharacter::exchangeMoney(const uint64 &money)
{
	bool exchangeWithBot;
	if (!checkExchangeActors(&exchangeWithBot))
	{
		DEBUG_STOP;
		return;
	}

	uint64 quantity = money;
	if (quantity > _Money)
		quantity = _Money;

	_ExchangeMoney = quantity;

	if (!exchangeWithBot)
	{
		CCharacter * c = PlayerManager.getChar( _CurrentInterlocutor );
		if (c != NULL)
		{
//			c->_PropertyDatabase.setProp( "EXCHANGE:MONEY", quantity );
			CBankAccessor_PLR::getEXCHANGE().setMONEY(c->_PropertyDatabase, quantity );
//			c->_PropertyDatabase.setProp( "EXCHANGE:ID", ++_ExchangeId );
			CBankAccessor_PLR::getEXCHANGE().setID(c->_PropertyDatabase, ++_ExchangeId );
		}

		invalidateExchange();
	}
	else
	{
		checkBotGift();
	}
} // exchangeMoney //


//-----------------------------------------------
// removeFromExchange:
//	remove the entity from exchange
//-----------------------------------------------
void CCharacter::removeFromExchange()
{
	if (isExchanging())
	{
		// reset exchange
		resetExchange();
	}
} // removeFromExchange //


//-----------------------------------------------
// setBerserkFlag:
//-----------------------------------------------
void CCharacter::setBerserkFlag(bool isBerserk)
{
	if ( _IsBerserk != isBerserk )
	{
		_IsBerserk = isBerserk;
		if ( _IsBerserk )
		{
			// YOYO: Berserk Flag since to be no more used. There is no more CHARACTER_INFO:SCORES:HitPoints database
			// hence must do it another way _PropertyDatabase.setProp( "CHARACTER_INFO:SCORES:HitPoints", 0);
//			_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, 0 );
			CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, 0 );
		}
		else
		{
			//sint32 temp = lookupStat("CurrentHitPoints");
			// YOYO: Berserk Flag since to be no more used. There is no more CHARACTER_INFO:SCORES:HitPoints database
			// hence must do it another way: _PropertyDatabase.setProp( "CHARACTER_INFO:SCORES:HitPoints", temp );
			sint32 temp = lookupStat("MaxHitPoints");
//			_PropertyDatabase.setProp( _DataIndexReminder->CHARACTER_INFO.SCORES.MaxScore[SCORES::hit_points], temp );
			CBankAccessor_PLR::getCHARACTER_INFO().getSCORES(SCORES::hit_points).setMax(_PropertyDatabase, temp );

			CEntityBase*	target = CEntityBaseManager::getEntityBasePtr( _Target() );
			if( target )
			{
				sint8 percent;
				// Hp
				if( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max <= 0 )
				{
					percent = 0;
				}
				else
				{
					sint8 percentTmp = sint8( (127.0 * ( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Current ) ) / ( target->getPhysScores()._PhysicalScores[ SCORES::hit_points ].Max ) );
					if( percentTmp < 0 )
						percent = 0;
					else
						percent = (uint8)percentTmp;
				}
//				_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, percent );
				CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, percent );
			}
			else
//				_PropertyDatabase.setProp( _DataIndexReminder->TARGET.HP, 0 );
				CBankAccessor_PLR::getTARGET().getBARS().setHP(_PropertyDatabase, 0 );
		}
//		_PropertyDatabase.setProp( "USER:BERSERK", _IsBerserk );
		CBankAccessor_PLR::getUSER().setBERSERK(_PropertyDatabase, _IsBerserk );
	}
} // setBerserkFlag //


//-----------------------------------------------
// cancelStaticActionInProgress:
//-----------------------------------------------
void CCharacter::cancelStaticActionInProgress(STATIC_ACT_TYPES::TStaticActTypes type, bool cancelBotChat, bool cancelLoot)
{
	setAfkState( false );
	// changed : always stop links (so one can only have one link at a time... as casting a new one will cancel the previous one)
	stopAllLinks();

	CPhraseManager::getInstance().cancelTopPhrase(_EntityRowId, true);

	bool noMoreActions = true;
	if (_HarvestOpened)
	{
		endHarvest();
	}

	if ( cancelLoot )
		pickUpItemClose();
	else if ( isExchanging() )
		noMoreActions = false;
	abortExchange();

	if ( (! cancelBotChat) &&
		 (_CurrentInterlocutor != CEntityId::Unknown && _CurrentInterlocutor.getType() != RYZOMID::player) )
		noMoreActions = false;

	if ( noMoreActions && _SEffectLinks.empty() )
	{
		_StaticActionInProgress = false;
	}

	clearCurrentAction();

	if ( cancelBotChat )
	{
		// Prevent cancelling a bot chat when a bot chat is starting. Warning: this can call processEvent()
		endBotChat( type == STATIC_ACT_TYPES::BotChat,true );
	}

} // cancelStaticActionInProgress //

//---------------------------------------------------
// cancelStaticEffects
//---------------------------------------------------
void CCharacter::cancelStaticEffects()
{
} // cancelStaticEffects //


//-----------------------------------------------
// staticActionInProgress:
//-----------------------------------------------
void CCharacter::staticActionInProgress(bool flag,STATIC_ACT_TYPES::TStaticActTypes type)
{
	if (flag == true)
	{
		if (_StaticActionInProgress == true)
			cancelStaticActionInProgress(type);
		if ( _NbStaticActiveEffects )
			cancelStaticEffects();

		CPhraseManager::getInstance().breakLaunchingLinks(this);

		// get current position and init pos
		CMirrorPropValue<sint32> propertyX( TheDataset, _Id, "X" );
		CMirrorPropValue<sint32> propertyY( TheDataset, _Id, "Y" );

		_OldPosX = propertyX;
		_OldPosY = propertyY;
		_StaticActionInProgress = true;
		_StaticActionType = type;
	}
	else
	{
		if ( _SEffectLinks.empty() )
		{
			_StaticActionInProgress = false;
		}
		_StaticActionType = STATIC_ACT_TYPES::Unknown;
	}
} // staticActionInProgress //

//-----------------------------------------------
// buildMissionList
//-----------------------------------------------
void CCharacter::buildMissionList( CCreature * creature,uint16 sessionId )
{
	// check must be done done before
	nlassert(creature);

	// re-init mission interface page
	_CurrentBotChatListPage = 0;
	// for each mission proposed by the creature, check if the player can take the mission, if yes, add the mission to the proposed mission list
	string noMissionMsg;
	for ( uint i =  0; i < creature->getMissionVector().size(); i++ )
	{
		// get the template
		const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( creature->getMissionVector()[i] );
		if ( !templ )
		{
			MISDBG("user:%s buildMissionList : ERROR : Invalid mission template %u", getId().toString().c_str(), creature->getMissionVector()[i]);
			continue;
		}
		if ( templ->AutoText.empty() && (templ->Tags.NotProposed == false) )
		{
			SBotChatMission m;
			// test the mission prerequisits with this player
			m.PreReqState = (MISSION_DESC::TPreReqState)templ->testPrerequisits(this, true);
			m.Mission = templ->Alias;
			_CurrentMissionList.push_back(m);
		}
	}
//	if ( !noMissionMsg.empty() && _CurrentMissionList.empty() )
//	{
//		TVectorParamCheck params;
//		uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId,noMissionMsg,params );
//		_PropertyDatabase.setProp( "CHOOSE_MISSIONS:NO_MISSION_MSG", txt );
//	}
	// fill the first mission page
	fillMissionPage(sessionId);
}// buildMissionList

//-----------------------------------------------
// sendMissionPrerequisitInfos
//-----------------------------------------------
void CCharacter::sendMissionPrerequisitInfos( uint16 missionIndex )
{
	if (missionIndex >= _CurrentMissionList.size())
	{
		nlwarning("Received mission index %u, but there is only %u missions in _CurrentMissionList", missionIndex, _CurrentMissionList.size());
		return;
	}

	const SBotChatMission &mission = _CurrentMissionList[missionIndex];

	// get the template
	const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission.Mission );
	if ( !templ )
	{
		MISDBG("user:%s sendMissionPrerequisitInfos : ERROR : Invalid mission template %u", getId().toString().c_str(), mission.Mission);
		return;
	}

	CPrerequisitInfos infos;
	templ->testPrerequisits(this, infos, true);

	uint8 index = (uint8)missionIndex;

	CMessage msgout( "IMPULSION_ID" );
	CBitMemStream bms;
	msgout.serial( _Id );
	if ( ! GenericMsgManager.pushNameToStream( "MISSION_PREREQ:SET", bms) )
	{
		nlwarning("Msg name MISSION_PREREQ:SET not found");
		return;
	}
	bms.serial(infos);
	bms.serial(index);
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	sendMessageViaMirror( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}

//-----------------------------------------------
// fillMissionPage
//-----------------------------------------------
void CCharacter::fillMissionPage(uint16 sessionId)
{
	// compute the bounds of the indexes of the mission to be put in the page
	const uint begin = _CurrentBotChatListPage * NB_SLOT_PER_PAGE;
	uint end;
	bool hasNext;
	if ( begin + NB_SLOT_PER_PAGE < _CurrentMissionList.size() )
	{
		end = begin + NB_SLOT_PER_PAGE;
		hasNext = true;
	}
	else
	{
		end = (uint)_CurrentMissionList.size();
		hasNext = false;
	}
//	_PropertyDatabase.setProp( "CHOOSE_MISSIONS:SESSION", sessionId );
	CBankAccessor_PLR::getCHOOSE_MISSIONS().setSESSION(_PropertyDatabase, sessionId );
//	_PropertyDatabase.setProp( "CHOOSE_MISSIONS:PAGE_ID", _CurrentBotChatListPage );
	CBankAccessor_PLR::getCHOOSE_MISSIONS().setPAGE_ID(_PropertyDatabase, _CurrentBotChatListPage );
//	_PropertyDatabase.setProp( "CHOOSE_MISSIONS:HAS_NEXT", hasNext );
	CBankAccessor_PLR::getCHOOSE_MISSIONS().setHAS_NEXT(_PropertyDatabase, hasNext);

	const TDataSetRow & botRow = TheDataset.getDataSetRow( _CurrentInterlocutor );

	uint index = 0;
	for ( uint i = begin; i < end;i++ )
	{
		const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate(_CurrentMissionList[i].Mission);
		if ( templ )
		{
//			_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:ICON",index),  templ->Icon );
			CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setICON(_PropertyDatabase,  templ->Icon );
//			_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:TEXT",index),  templ->sendTitleText(_EntityRowId,botRow) );
			CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setTEXT(_PropertyDatabase,  templ->sendTitleText(_EntityRowId,botRow) );
			// send the description without any overriden text
//			_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:DETAIL_TEXT",index),  templ->sendDescText(_EntityRowId, botRow ) );
			CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setDETAIL_TEXT(_PropertyDatabase,  templ->sendDescText(_EntityRowId, botRow ) );
//			_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:PREREQ_STATE",index),  _CurrentMissionList[i].PreReqState );
			CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setPREREQ_STATE(_PropertyDatabase,  _CurrentMissionList[i].PreReqState );
			index++;
		}
		else
		{
			nlwarning("<CCharacter fillMissionPage> Invalid mission template %u for user %s",
				_CurrentMissionList[i].Mission, _Id.toString().c_str());
		}
	}
	for ( ; index < NB_SLOT_PER_PAGE; index++ )
	{
//		_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:ICON",index),  0 );
		CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setICON(_PropertyDatabase,  CSheetId::Unknown );
//		_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:TEXT",index),  0 );
		CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setTEXT(_PropertyDatabase,  0);
//		_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:DETAIL_TEXT",index),  0 );
		CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setDETAIL_TEXT(_PropertyDatabase,  0);
//		_PropertyDatabase.setProp( NLMISC::toString("CHOOSE_MISSIONS:%u:PREREQ_STATE",index),  0 );
		CBankAccessor_PLR::getCHOOSE_MISSIONS().getArray(index).setPREREQ_STATE(_PropertyDatabase,  0);
	}
	_CurrentBotChatListPage++;
}// fillMissionPage


void CCharacter::addMission(CMissionSolo * mission)
{
	_Missions->setMissions( mission );
	TMissionHistory &mh = _MissionHistories[mission->getTemplateId()];
	// store the date.
	mission->updateUsersJournalEntry();
}


//-----------------------------------------------
// removeMission
//-----------------------------------------------
void CCharacter::removeMission(TAIAlias alias, /*TMissionResult*/ uint32 result)
{
	CMission * mission = _Missions->getMissions(alias);
	if ( mission == NULL )
	{
		nlwarning("<CCharacter removeMission> In char %s : mission not found", _Id.toString().c_str());
		return;
	}
	if ( mission->getFinished() )
	{
		if (mission->getMissionSuccess())
			result = mr_success;
		else
			result = mr_fail;
	}

	CMissionTemplate * tpl = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );


	updateMissionHistories(alias, result);

	if ( tpl && !tpl->Tags.NoList )
	{
//		Bsi.append( StatPath, NLMISC::toString("[MI%s] %s %s",
//			MissionResultStatLogTag[result],
//			_Id.toString().c_str(),
//			tpl->getMissionName().c_str()) );
//
//		EgsStat.displayNL("[MI%s] %s %s",
//			MissionResultStatLogTag[result],
//			_Id.toString().c_str(),
//			tpl->getMissionName().c_str());

//		EGSPD::missionLog(MissionResultStatLogTag[result], _Id, tpl->getMissionName());

		mission->clearUsersJournalEntry();
	}

	CMissionManager::getInstance()->deInstanciateMission(mission);
	_Missions->deleteFromMissions( alias );
	updateTarget();
}

//-----------------------------------------------
// abandonMission
//-----------------------------------------------
void CCharacter::abandonMission(uint8 indexClient)
{
	CMission* mission = NULL;
	CMissionTemplate * templ = NULL;
	for ( map<TAIAlias, CMission*>::iterator it =  getMissionsBegin(); it != getMissionsEnd(); ++it )
	{
		if ( (*it).second->getClientIndex() == indexClient )
		{
			mission = (*it).second;
			if ( mission )
			{
				templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
				if ( templ->Tags.NoList ) // skip invisible missions
					continue;
			}
			break;
		}
	}
	if ( !mission )
	{
		MISDBG( "user:%s abandonMission : ERROR : invalid mission index %u sent ",_Id.toString().c_str(), indexClient );
		return;
	}
	if ( mission->getFinished() == false )
	{
		if ( !templ )
		{
			MISDBG( "user:%s abandonMission : ERROR : invalid mission alias %u", _Id.toString().c_str(), mission->getTemplateId() );
			return;
		}
		if ( templ->Tags.NonAbandonnable )
		{
			MISDBG( "user:%s abandonMission : ERROR : mission alias %u is not abandonnable but user tries to abandon it", _Id.toString().c_str(), mission->getTemplateId() );
			return;
		}
		MISDBG("user:%s abandonMission", getId().toString().c_str());
		mission->onFailure(true, false);
		CCharacter::sendDynamicSystemMessage( _EntityRowId, "ABANDON_MISSION" );
	}
	removeMission( mission->getTemplateId(), mr_abandon );
}

void CCharacter::clearSuccessfullMission()
{
	_MissionHistories.clear();
}

void CCharacter::updateMissionHistories(TAIAlias missionAlias, /*TMissionResult*/ uint32 result)
{
	TMissionHistory &mh = _MissionHistories[missionAlias];

	switch(result)
	{
	case mr_success:
	case mr_forced:
		mh.Successfull = true;
		// validate last try date
		_MissionHistories[missionAlias].LastSuccessDate = CTickEventHandler::getGameCycle();
		break;
	}
}

bool CCharacter::processMissionEventList( std::list< CMissionEvent* > & eventList,bool deleteEvent, TAIAlias alias)
{
	H_AUTO(CCharacter_processMissionEventList);

	if ( eventList.empty() )
		return false;
	// we process all the event of our list. Events can be added in the list during the processing
	CTeam * team = TeamManager.getRealTeam(_TeamId);
	bool processed = false;

	bool firstEvent = true;
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId( _GuildId );
	while ( !eventList.empty() )
	{
		bool eventProcessed = false;
		CMissionEvent & event = *(*eventList.begin());

		if ( event.Type == CMissionEvent::AddMission )
		{
			CMissionEventAddMission& eventSpe = (CMissionEventAddMission&) event;
			TAIAlias mission = eventSpe.Mission;
			TAIAlias giver = eventSpe.Giver;
			TAIAlias mainMission = eventSpe.MainMission;
			bool missionForGuild = eventSpe.Guild;

			// add mission event are always allocated on heap
			delete ( CMissionEvent *) ( eventList.front() );
			eventList.pop_front();

			// If the mission is not for guild members we just instanciate it
			if (!missionForGuild)
				CMissionManager::getInstance()->instanciateMission(this, mission, giver, eventList, mainMission);
			else
			{
				// We find the guild and each guild members and we instanciate the mission for them
				if (guild)
				{
					for ( std::map<EGSPD::TCharacterId, EGSPD::CGuildMemberPD*>::iterator it = guild->getMembersBegin();
						it != guild->getMembersEnd();++it )
					{
						CCharacter * guildUser = PlayerManager.getChar( it->first );
						if ( !guildUser )
						{
							nlwarning( "<MISSIONS>cant find user %s", it->first.toString().c_str() );
							continue;
						}
						CMissionManager::getInstance()->instanciateMission(guildUser, mission, giver, eventList, mainMission);
					}
				}
			}
		}
		// event may have been processed during instanciateMission
		if ( eventList.empty() )
			return true;

		// Process event

		/// FIRST - Check with character missions
		if (event.Restriction != CMissionEvent::NoSolo)
		{
			eventProcessed = processMissionUserEvent(eventList, alias);
		}

		// SECOND - Check with team missions (if event not already processed and char belongs to a team)
		if (!eventProcessed && (event.Restriction != CMissionEvent::NoGroup))
		{
			if (team != NULL)
				eventProcessed = team->processTeamMissionEvent(eventList, alias);
		}

		// THIRD - Check with guild missions (if event not already processed and char belongs to a guild)
		if (!eventProcessed)// && (event.Restriction != CMissionEvent::NoGroup))
		{
			if (guild != NULL)
				eventProcessed = guild->processGuildMissionEvent(eventList, alias);
		}

		processed |= eventProcessed;

		// the first event of the list was processed, so we remove it.
		// the initial event was built on stack, the other on heap.
		if ( deleteEvent || !firstEvent )
			delete ( CMissionEvent *) ( eventList.front() );
		eventList.pop_front();
		firstEvent = false;
	}
	return processed;
}

//-----------------------------------------------
// processMissionEvent
//-----------------------------------------------
bool CCharacter::processMissionEvent( CMissionEvent & event, TAIAlias alias )
{
	H_AUTO(CCharacter_processMissionEvent)
	if ( !MissionSystemEnabled )
		return false;

	std::list< CMissionEvent * > eventList;
	eventList.push_back( &event );
	return processMissionEventList( eventList, false, alias );
}

//-----------------------------------------------
// processMissionEvent
//-----------------------------------------------
bool CCharacter::processMissionEventWithTeamMate( CMissionEvent & event, TAIAlias alias )
{
	bool bProcessed = false;
	bProcessed = processMissionEvent(event, alias);

	// If the event do not serve for the character missions so it could serve for the teammate missions
	if (!bProcessed)
	{
		CTeam *pTeam = TeamManager.getRealTeam(_TeamId);
		if (pTeam != NULL)
		{
			// Try with all other members (randomly)
			if (!bProcessed)
			{
				// Construct a vector of members
				vector<CEntityId> vMembers;
				for (list<CEntityId>::const_iterator it = pTeam->getTeamMembers().begin();
						it != pTeam->getTeamMembers().end(); ++it)
					vMembers.push_back(*it);

				while (!vMembers.empty())
				{
					// Pick up a member (in a random order)
					uint idx = RandomGenerator.rand((uint16)vMembers.size() - 1);
					CCharacter * c = PlayerManager.getChar( vMembers[idx] );
					if ((c != NULL) && c->getEnterFlag() && (c != this))
					{
						bProcessed = c->processMissionEvent(event, alias);
						if (bProcessed)
							break;
					}
					vMembers[idx] = vMembers.back();
					vMembers.pop_back();
				}
			}
		}
	}
	return bProcessed;
}

//-----------------------------------------------
// process a mission event multiple times, until all matching steps are done. Return the number of matching steps for which processMissionEvent() returned true.
//-----------------------------------------------
sint CCharacter::processMissionMultipleEvent( CMissionEvent & event, TAIAlias alias )
{
	H_AUTO(CCharacter_processMissionMultipleEvent)
	if ( !MissionSystemEnabled )
		return false;

	sint nbProcessed = -1;
	bool result;
	do
	{
		++nbProcessed;
		std::list< CMissionEvent * > eventList;
		eventList.push_back( &event );
		result = processMissionEventList( eventList, false, alias );
	}
	while ( result );
	return nbProcessed;
}

//-----------------------------------------------
// processMissionUserEvent
//-----------------------------------------------
bool CCharacter::processMissionUserEvent(std::list< CMissionEvent* > & eventList,TAIAlias alias)
{
	if ( alias != CAIAliasTranslator::Invalid )
		return processMissionStepUserEvent( eventList, alias, 0xFFFFFFFF );
	else
	{
		for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
		{
			if ( processMissionStepUserEvent( eventList,(*it).second->getTemplateId() ,0xFFFFFFFF) )
				return true;
		}
	}
	return false;
}// processMissionUserEvent

//-----------------------------------------------
// processMissionStepUserEvent
//-----------------------------------------------
bool CCharacter::processMissionStepUserEvent(std::list< CMissionEvent* > & eventList,uint missionAlias,uint32 stepIndex)
{
	H_AUTO(CCharacter_processMissionStepUserEvent);

	CMission * mission = _Missions->getMissions( missionAlias );
	if (!mission )
	{
		const string& mName = CAIAliasTranslator::getInstance()->getMissionNameFromUniqueId(missionAlias);
		nlwarning("<CCharacter::processMissionStepUserEvent> Mission %d(%s) is not active for player %s at the moment, can't process step user event", missionAlias, mName.c_str(), _Id.toString().c_str());

		// display some more infos about the mission
		CMissionTemplate * mTemplate = CMissionManager::getInstance()->getTemplate(missionAlias);
		if( mTemplate)
		{
			nlwarning("<CCharacter::processMissionStepUserEvent> About this mission: TitleText= %s, DescText= %s, AutoText= %s",mTemplate->TitleText.c_str(),mTemplate->DescText.c_str(), mTemplate->AutoText.c_str());
		}

		return false;
	}
	CMissionEvent::TResult result = mission->processEvent( _EntityRowId,eventList,stepIndex );
	if ( result == CMissionEvent::Nothing )
		return false;
	else if ( result == CMissionEvent::MissionFailed )
		return true;

	CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
	nlassert( templ );
	if ( result == CMissionEvent::MissionEnds )
	{
		CMissionEventMissionDone * event = new CMissionEventMissionDone(templ->Alias);
		eventList.push_back(event);
		addSuccessfulMissions( *templ );

		bool bChained = false;
		CMissionGuild *pMG = dynamic_cast<CMissionGuild*>(mission);
		if (pMG != NULL) bChained = pMG->isChained();
		CMissionTeam *pMT = dynamic_cast<CMissionTeam*>(mission);
		if (pMT != NULL) bChained = pMT->isChained();
		CMissionSolo *pMS = dynamic_cast<CMissionSolo*>(mission);
		if (pMS != NULL) bChained = pMS->isChained();

		if ( !templ->Tags.NoList && !templ->Tags.AutoRemove )
			sendDynamicSystemMessage( _Id,bChained?"EGS_MISSION_STEP_SUCCESS":"EGS_MISSION_SUCCESS");
		CMissionManager::getInstance()->missionDoneOnce(templ);
		mission->stopChildren();

		// only remove no list missions, other must be manually removed by user
		if ( templ->Tags.NoList || bChained || templ->Tags.AutoRemove )
		{
			mission->updateEncyclopedia();
			removeMission(missionAlias, mr_success);
		}
		else
		{
			mission->setSuccessFlag();
			mission->updateUsersJournalEntry();
		}
		return true;
	}
	else if ( result == CMissionEvent::StepEnds )
	{
		if ( !templ->Tags.NoList )
			sendDynamicSystemMessage( _Id,"EGS_MISSION_STEP_SUCCESS");
	}
	mission->updateUsersJournalEntry();
	return true;

}// processMissionStepUserEvent


//-----------------------------------------------
// botChatMissionAdvance
//-----------------------------------------------
void CCharacter::botChatMissionAdvance( uint8 index )
{
	CCreature * bot = CreatureManager.getCreature( _CurrentInterlocutor );
	// no mission found : check in the special contexts of the bot
	if ( !bot )
	{
		nlwarning("<CCharacter botChatMissionAdvance> invalid bot %s", _CurrentInterlocutor.toString().c_str() );
		return;
	}

	uint idx = 0;
	for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
	{
		std::vector<CMission::CBotChat> botchats;
		(*it).second->getBotChatOptions( TheDataset.getDataSetRow( _CurrentInterlocutor ),botchats);
		for ( uint j = 0; j < botchats.size(); )
		{
			if ( idx == index )
			{
				if ( !botchats[j].Gift )
				{
					// build a talk event
					std::list<CMissionEvent*> eventList;
					CMissionEventTalk event;
					eventList.push_back( &event );
					// process the event
					processMissionStepUserEvent( eventList,(*it).second->getTemplateId(),botchats[j].StepIndex );
					// remove the initial event, as it was processed
					eventList.pop_front();
					// process all the event triggered by the processing of the initial event
					for ( std::list< CMissionEvent* >::iterator it = eventList.begin(); it != eventList.end(); ++it  )
						processMissionEvent(*(*it));
					j++;
				}
				else
				{
					// store infos on the current bot gift
					if ( _BotGift )
						delete _BotGift;
					_BotGift = new CBotGift;
					_BotGift->Type = MISSION_DESC::Solo;
					_BotGift->MissionAlias = (*it).second->getTemplateId();
					_BotGift->StepsIndex.insert( botchats[j].StepIndex );

					const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _BotGift->MissionAlias );
					nlassert( templ );
					// if step Any, add all following steps which are part of this step any
					if( templ->Steps[botchats[j].StepIndex - 1]->isAny() )
					{
						j++;
						while(j<botchats.size() && botchats[j].Gift && templ->Steps[botchats[j].StepIndex - 1]->isAny())
						{
							_BotGift->StepsIndex.insert( botchats[j].StepIndex );
							j++;
						}
					}
					else
						j++;

					// init exchange view
					_ExchangeView = new CExchangeView;
					_ExchangeView->setCharacter(this);
					_ExchangeView->bindToInventory(_Inventory[INVENTORIES::bag]);

					SM_STATIC_PARAMS_1(params, STRING_MANAGER::bot);
					params[0].setEIdAIAlias( _CurrentInterlocutor, CAIAliasTranslator::getInstance()->getAIAlias(_CurrentInterlocutor) );
					uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId, "EXCHANGE_TITLE_BOT", params );
//					_PropertyDatabase.setProp( "EXCHANGE:TEXT", txt );
					CBankAccessor_PLR::getEXCHANGE().setTEXT(_PropertyDatabase, txt );

					// try to auto fill exchange view
					autoFillExchangeView();
				}
				return;
			}
			idx++;
		}
		uint j = 0;
		for ( map<uint32,EGSPD::CMissionTeleportPD>::iterator itTp = (*it).second->getTeleportsBegin(); itTp != (*it).second->getTeleportsEnd(); ++itTp )
		{
			if ( (*it).second->getTeleportBot( (*itTp).second.getIndex() ) == bot->getAlias() )
			{
				if ( idx == index )
				{
					(*it).second->teleport( this, (*itTp).second.getIndex() );
					return;
				}
				idx++;
			}
		}
	}

	// team missions
	CTeam * team = TeamManager.getRealTeam( _TeamId );
	if ( team )
	{
		for ( uint i  = 0 ; i < team->getMissions().size(); i++ )
		{
			nlassert( team->getMissions()[i] );
			std::vector<CMission::CBotChat> botchats;
			team->getMissions()[i]->getBotChatOptions( TheDataset.getDataSetRow( _CurrentInterlocutor ),botchats);
			for ( uint j = 0; j < botchats.size(); )
			{
				if ( idx == index )
				{
					if ( !botchats[j].Gift )
					{
						// build a talk event
						std::list<CMissionEvent*> eventList;
						CMissionEventTalk event;
						eventList.push_back( &event );
						// process the event
						team->processTeamMissionStepEvent( eventList,team->getMissions()[i]->getTemplateId(),botchats[j].StepIndex );
						// remove the initial event, as it was processed
						eventList.pop_front();
						// process all the event triggered by the processing of the initial event
						for ( std::list< CMissionEvent* >::iterator it = eventList.begin(); it != eventList.end(); ++it  )
							processMissionEvent(*(*it));
						j++;
					}
					else
					{
						// store infos on the current bot gift
						_BotGift = new CBotGift;
						_BotGift->Type = MISSION_DESC::Group;
						_BotGift->MissionAlias = team->getMissions()[i]->getTemplateId();
						_BotGift->StepsIndex.insert( botchats[j].StepIndex );

						const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( _BotGift->MissionAlias );
						nlassert( templ );
						// if step Any, add all following steps which are part of this step any
						if( templ->Steps[botchats[j].StepIndex - 1]->isAny() )
						{
							j++;
							while(j<botchats.size() && botchats[j].Gift && templ->Steps[botchats[j].StepIndex - 1]->isAny())
							{
								_BotGift->StepsIndex.insert( botchats[j].StepIndex );
								j++;
							}
						}
						else
							j++;

						// init exchange view
						_ExchangeView = new CExchangeView;
						_ExchangeView->setCharacter(this);
						_ExchangeView->bindToInventory(_Inventory[INVENTORIES::bag]);
					}
					return;
				}
				idx++;
			}
			for ( map<uint32,EGSPD::CMissionTeleportPD>::iterator it = team->getMissions()[i]->getTeleportsBegin(); it != team->getMissions()[i]->getTeleportsEnd(); ++it )
			{
				if ( team->getMissions()[i]->getTeleportBot( (*it).second.getIndex() ) == bot->getAlias() )
				{
					if ( idx == index )
					{
						team->getMissions()[i]->teleport( this, (*it).second.getIndex() );
						return;
					}
					idx++;
				}
			}
		}
	}

	for ( uint i =  0; i < bot->getContextTexts().size(); i++ )
	{
		if ( idx == index )
		{
			TVectorParamCheck vect;
			STRING_MANAGER::TParam param;

			param.Type = STRING_MANAGER::player;
			param.setEId( _Id );
			vect.push_back( param );

			param.Type = STRING_MANAGER::bot;
			param.setEIdAIAlias( _CurrentInterlocutor, CAIAliasTranslator::getInstance()->getAIAlias( _CurrentInterlocutor) );
			vect.push_back( param );

			uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId, bot->getContextTexts()[i].second,vect );
			npcTellToPlayerEx( TheDataset.getDataSetRow(_CurrentInterlocutor),_EntityRowId,txt );
			return;
		}
		idx++;
	}

	// auto missions
	// send auto missions
	for ( uint i = 0; i < bot->getMissionVector().size(); i++ )
	{
		const CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( bot->getMissionVector()[i] );
		if ( (templ != NULL) && !templ->AutoText.empty() )
		if (templ->testPrerequisits(this, false) == MISSION_DESC::PreReqSuccess)
		{
			if ( idx == index )
			{
				std::list<CMissionEvent*> eventList;
				CMissionManager::getInstance()->instanciateMission(this,templ->Alias,bot->getAlias(),eventList);
				processMissionEventList( eventList,true, CAIAliasTranslator::Invalid );
				setTargetBotchatProgramm(bot, bot->getId());
				return;
			}
			idx++;
		}
	}
	nlwarning("<CCharacter botChatMissionAdvance> invalid index %u", index );
}// botChatMissionAdvance



// Briancode: complete, needs testing
// helper function, will probably put in "checkBotGift", which is where I got the code from.
//-----------------------------------------------
CMission * CCharacter::getMissionFromBotGift()
{
	nlassert(_BotGift);

	if (_BotGift->Type == MISSION_DESC::Solo)
	{
		return _Missions->getMissions( _BotGift->MissionAlias );
	}
	else if (_BotGift->Type == MISSION_DESC::Group)
	{
		CTeam * team = TeamManager.getRealTeam( _TeamId );
		if (!team)
		{
			nlwarning("Player %s has no team!", _Id.toString().c_str());
			return NULL;
		}
		return team->getMissionByAlias( _BotGift->MissionAlias );
	}
	else
		return NULL;
}


//-----------------------------------------------
// Briancode: complete, needs testing

// bool CExchangeView::putItemInExchange(uint32 bagSlot, uint32 exchangeSlot, uint32 quantity)
// from mission_step_talk.cpp
//	bool checkPlayerGift( CMission * instance, CCharacter * user )

// _ExchangeView already invoked by calling class

// only need items for the NPC being *actively addressed*

// don't need to search all missions - the mission in question is already found, and stored in _BotGift->MissionAlias

// for missions: the *instance* is the base singleton original primative. the *template* is the personal copy each player has.

// have mission template ID in _BotGift->MissionAlias - get mission from that just pass mission in?
// have botchat step index in _BotGift->StepIndex
// basing off of mission_step_talk::checkPlayerGift

// a missions step's "State" is not a "State" in the formal / state machine sense.
// it is, at least in the case of gift giving and loot getting missions, a list
// of objects containing the sheet ids and quantities required for step completion.
// when items are given / looted towards the mission's completeion, the quantity
// is decremented. when it reaches zero, that "state" is considered completed.
// Remember! A mission's *template* is an instanciated private version for this user / group

// This code examinses the current step of the current mission. if there are multiple "turn in" steps
// or turn in messages for this NPC that the player can satisfy at once, they will still have to do
// this in multiple stages.

// New functionality: tests sheetIDs against masks as well as whole values. For example,
// if validateSteps[stepCounter].Sheet equals "ic?m1ss*", then all 1 handed swords will be accepted.

//-----------------------------------------------
//-----------------------------------------------
bool CCharacter::autoFillExchangeView()
{
	CInventoryPtr playerBagInvPointer;
	CMission * currentMission = NULL;
	CGameItemPtr invItem;
	uint stepCounter, candidateCounter, totalItemsInBag, itemsSeenCount;
	CActiveStepPD activeStep;
	CMissionTemplate * missionTemplate;
	std::map<uint32, CActiveStepPD>::iterator stepIterator;
	std::vector<IMissionStepTemplate::CSubStep> validateSteps;
	uint32 inventoryIndex, lowestQualityIndex;
	uint16 lowestQuality;
	bool exchangeWorked;

	nlassert(getExchangeView() != NULL);

	playerBagInvPointer = _Inventory[ INVENTORIES::bag ];
	nlassert(playerBagInvPointer != NULL);

	currentMission = getMissionFromBotGift();
	if(!currentMission)
		return false;

	missionTemplate = CMissionManager::getInstance()->getTemplate( currentMission->getTemplateId() );
	nlassert( missionTemplate );


	if (currentMission->getStepsBegin() == currentMission->getStepsEnd())
		return false;

	// Assuming there will be less missions than inventory, and that while we
	// have to search every mission we don't have to search every inventory slot (just as many as
	// it takes to satisfy mission requirements...)
	totalItemsInBag = playerBagInvPointer->getUsedSlotCount();

	// used to know if some item have already been put into exchange inventory
	bool exchangeEmpty = true;

	set<uint32>::iterator itSet;
	for( itSet=_BotGift->StepsIndex.begin(); itSet!=_BotGift->StepsIndex.end(); ++itSet )
	{
		if( !exchangeEmpty )
			break;

		validateSteps = missionTemplate->Steps[*itSet-1]->getSubSteps();

		// the exchange temp inventory thingy has only 8 slots, so very benign failures to put items into it
		// are possible. Hence merely breaking (doing no further work) as opposed to aborting work done,
		// and still returning "true". (exchangeWorked == false does not necessarily represent a failure of the whole operation)
		exchangeWorked = true;

		// reset step counter, and iterate through all items in active step...
		for (stepCounter = 0; stepCounter < validateSteps.size(); stepCounter++)
		{
			std::vector<uint32> candidateIndexes;

			// factors out already satisfied requirements.
			if (validateSteps[stepCounter].Quantity == 0)
				continue;

			itemsSeenCount = 0;

			for (inventoryIndex = 0; inventoryIndex < playerBagInvPointer->getSlotCount(); inventoryIndex++)
			{
				invItem = playerBagInvPointer->getItem(inventoryIndex);
				if (invItem == NULL)
					continue;

				if (invItem->getLockedByOwner())
					continue;

				if (invItem->getRefInventory() != NULL)
					continue;

				itemsSeenCount++;
				// Changed to support comparisons on sheetID masks
				if (invItem->getSheetId() == validateSteps[stepCounter].Sheet)
				{
					// invItem->recommended() is the "recommended" skill to use an item! ah hah!
					// if the quality is equal, it's safe to put it up
					if (invItem->recommended() == validateSteps[stepCounter].Quality)
					{
						if (invItem->getStackSize() >= validateSteps[stepCounter].Quantity)
						{
							// if user has put more that this sub step quantity, just put that many in exchange
							exchangeWorked = _ExchangeView->putItemInFirstEmptyExchangeSlot(inventoryIndex, validateSteps[stepCounter].Quantity);
							if (exchangeWorked)
							{
								validateSteps[stepCounter].Quantity = 0;
								exchangeEmpty = false;
							}
							else
								break;
						}
						else
						{
							// place all of the quantity that the player has in the exchange inventory
							exchangeWorked = _ExchangeView->putItemInFirstEmptyExchangeSlot(inventoryIndex, invItem->getStackSize());
							if (exchangeWorked)
							{
								validateSteps[stepCounter].Quantity -= invItem->getStackSize();
								exchangeEmpty = false;
							}
							else
								break;
						}

					}
					// if the item is of superior quality, we don't necessarily want to select it, unless it's the only option.
					else if (invItem->recommended() > validateSteps[stepCounter].Quality)
					{
						candidateIndexes.push_back(inventoryIndex);
					}
				}

				// it is possible that desired items might be in more than one stack. This function
				// will loop through objects on this mission step until Quantity is zero, at which time
				// all appropriate items have been found.
				if (validateSteps[stepCounter].Quantity == 0)
					break;

				// the complete inventory doesn't seem to have a garauntee of being collapsed (i.e. no empty slots
				// between used slots). However, it does count the used slots. So, once I've seen as many used slots
				// as I know the bag has, abort.
				if (itemsSeenCount >= totalItemsInBag) break;
			}

			if (exchangeWorked == false)
				break;

			// if there are any gifts that have not been completely filled by items of exactly the quality desired,
			// and there were found appropriate items that were of higher than minimum quality, attempt to fill
			// the remaining needs through these super-optimal items, starting with the least valuable first.
			lowestQuality = 0;
			if ((validateSteps[stepCounter].Quantity > 0) && (candidateIndexes.empty() == 0))
			{
				// implicit in (lowestQuality < 999) is (candidateIndexes.empty() == 0)
				while ( (lowestQuality < 999) && (validateSteps[stepCounter].Quantity > 0) )
				{
					if (exchangeWorked == false)
						break;

					lowestQuality = 999;
					for (candidateCounter = 0; candidateCounter < candidateIndexes.size(); candidateCounter++)
					{
						invItem = playerBagInvPointer->getItem(candidateIndexes[candidateCounter]);
						if ( (invItem != NULL)
							&& (invItem->recommended() < lowestQuality)
							&& (invItem->getNonLockedStackSize() > 0) )
						{
							lowestQualityIndex = candidateCounter;
							lowestQuality = (uint16)invItem->recommended();

						}
					}
					if (lowestQuality < 999)
					{
						invItem = playerBagInvPointer->getItem(candidateIndexes[lowestQualityIndex]);
						if (invItem->getStackSize() >= validateSteps[stepCounter].Quantity)
						{
							// if user has put more that this sub step quantity, just put that many in exchange
							exchangeWorked = _ExchangeView->putItemInFirstEmptyExchangeSlot(candidateIndexes[lowestQualityIndex], validateSteps[stepCounter].Quantity);
							if (exchangeWorked)
							{
								validateSteps[stepCounter].Quantity = 0;
								exchangeEmpty = false;
							}
							else
								break;
						}
						else
						{
							// place all of the quantity that the player has in the exchange inventory
							// candidateIndexs.erase not necessary due to the stack locking, but probably good form.
							exchangeWorked = _ExchangeView->putItemInFirstEmptyExchangeSlot(candidateIndexes[lowestQualityIndex], invItem->getStackSize());
							if (exchangeWorked)
							{
								validateSteps[stepCounter].Quantity -= invItem->getStackSize();
								candidateIndexes.erase(candidateIndexes.begin() + lowestQualityIndex);
								exchangeEmpty = false;
							}
							else
								break;
						}
					}
				}
			}

			if (exchangeWorked == false)
				break;
		}
	}
	return true;
}

//-----------------------------------------------
// updateSavedMissions
//-----------------------------------------------
void CCharacter::updateSavedMissions()
{
	// remove all mission failed when offline
	vector<CMissionSolo*> failed;
	for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); ++it )
	{
		CMissionSolo* mission = EGS_PD_CAST<CMissionSolo*>( (*it).second );
		EGS_PD_AST( mission );
		CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( mission->getTemplateId() );
		if ( !templ )
		{
			failed.push_back( mission );
			continue;
		}
		if ( !mission->checkConsistencyWithTemplate() ||
			templ->Tags.DoneOnce && templ->AlreadyDone ||
			 templ->MonoTimer && !templ->Instances.empty()  )
		{
			failed.push_back( mission );
		}
		// check critical part timer if any
		else if (mission->getCriticalPartEndDate() != 0 && mission->getCriticalPartEndDate() < CTickEventHandler::getGameCycle())
		{
			failed.push_back( mission );
		}
	}
	for ( uint i  = 0; i < failed.size(); i++ )
		removeMission( failed[i]->getTemplateId() , mr_fail );
	uint idx = 0;

	bool bLaunchCrashHandler = true;
	if (_SaveDate > IService::getInstance()->getLaunchingDate())
		bLaunchCrashHandler = false; // Launch the player_reconnect handler

	for ( map<TAIAlias, CMission*>::iterator it = getMissionsBegin(); it != getMissionsEnd(); )
	{
		CMissionTemplate * templ = CMissionManager::getInstance()->getTemplate( (*it).second->getTemplateId() );
		EGS_PD_AST ( templ );

		CMissionSolo* mission = EGS_PD_CAST<CMissionSolo*>( (*it).second );
		EGS_PD_AST( mission );
		mission->setTaker(_EntityRowId);
		if ( !templ->Tags.NoList )
		{
			mission->setClientIndex( idx );
			mission->putInGame();
			mission->updateUsersJournalEntry();
			list<CMissionEvent*> eventList;
			for ( map<uint32,EGSPD::CActiveStepPD>::iterator itStep = mission->getStepsBegin(); itStep != mission->getStepsEnd(); ++itStep )
			{
				uint32 indexInTempl = (*itStep).second.getIndexInTemplate() - 1;
				if ( indexInTempl < templ->Steps.size() )
					templ->Steps[indexInTempl]->onActivation( mission, (*itStep).second.getIndexInTemplate() - 1, eventList );
			}
			while ( !eventList.empty() )
			{
				nlassert(*eventList.begin());
				processMissionEvent(**eventList.begin());
				delete *eventList.begin();
				eventList.pop_front();
			}
			// we reload the missions : there was a crash before
			map<TAIAlias, CMission*>::iterator itBack = it;
			++itBack;
			if (bLaunchCrashHandler)
				mission->applyCrashHandler(true, std::string());
			else
				mission->applyPlayerReconnectHandler();
			it = itBack;
			++idx;
		}
		else
		{
			mission->setClientIndex( 0xFF );
			mission->setTaker(_EntityRowId);
			mission->putInGame();

			for ( map<uint32,EGSPD::CMissionCompassPD>::iterator itCompass = (*it).second->getCompassBegin(); itCompass != (*it).second->getCompassEnd(); ++itCompass )
			{
				TVectorParamCheck params(1);
				sint32 x = 0;
				sint32 y = 0;
				string msg;
				CCreature * c = CreatureManager.getCreature( CAIAliasTranslator::getInstance()->getEntityId( (*itCompass).second.getBotId() ) );
				if ( c )
				{
					x = c->getState().X();
					y = c->getState().Y();

					// Send the bot name to the client if not already done (or if the name has changed)
					//CMirrorPropValueRO<TYPE_NAME_STRING_ID> botNameId( TheDataset, c->getEntityRowId(), DSPropertyNAME_STRING_ID );
					params[0].Type = STRING_MANAGER::bot;
					params[0].setEIdAIAlias( c->getId(), CAIAliasTranslator::getInstance()->getAIAlias(c->getId()) );
					msg = "COMPASS_BOT";
					uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId,msg,params );
					PlayerManager.sendImpulseToClient( _Id, "JOURNAL:ADD_COMPASS_BOT", x,y,txt, c->getEntityRowId().getCompressedIndex() );

				}
				else
				{
					CPlace * place = CZoneManager::getInstance().getPlaceFromId( (uint16)(*itCompass).second.getPlace() );
					if ( place )
					{
						x = place->getCenterX();
						y = place->getCenterY();

						params[0].Identifier = place->getName();
						params[0].Type = STRING_MANAGER::place;
						msg = "COMPASS_PLACE";
						uint32 txt = STRING_MANAGER::sendStringToClient( _EntityRowId,msg,params );
						PlayerManager.sendImpulseToClient( _Id, "JOURNAL:ADD_COMPASS", x,y,txt );
					}
				}
			}
			++it;
		}
	}
}

//-----------------------------------------------
// addHandledAIGroup
// Note : we send the message to all AIS because we can't know which AiInstance the bot belongs to.
// There are no info on the mirror, and we can't rely on primitive name.
//-----------------------------------------------
void CCharacter::addHandledAIGroup(CMission *m, TAIAlias nGroupAlias, uint32 nDespawnTime)
{
	TAIAlias nMissionAlias = m->getTemplateId(); // Get mission alias
	// Send message to AIS
	CAddHandledAIGroupMsg msg;
	msg.PlayerRowId = getEntityRowId();
	msg.GroupAlias = nGroupAlias;
	msg.MissionAlias = nMissionAlias;
	msg.DespawnTimeInTick = nDespawnTime;
	msg.send("AIS"); // send the message to all AIS

	string sDebugString = string("user:") + getId().toString();
	sDebugString += " miss:" + CPrimitivesParser::aliasToString(nMissionAlias);

	// Store the handle into the mission
	CMission *pMission = _Missions->getMissions(nMissionAlias);
	if (pMission == NULL)
	{
		sDebugString += " ERROR cant get mission in character from mission alias";
		MISLOG(sDebugString.c_str());
		return;
	}
	EGSPD::CHandledAIGroupPD *pGrp = pMission->getHandledAIGroups(nGroupAlias);
	if (pGrp == NULL)
	{
		pGrp = pMission->addToHandledAIGroups(nGroupAlias);
		pGrp->setDespawnTime(nDespawnTime);
	}
	else
	{
		sDebugString += " ERROR cant put the handle on group '"
						+ CPrimitivesParser::aliasToString(nGroupAlias)
						+ "'. The handle already exists.";
		MISLOG(sDebugString.c_str());
		return;
	}
}

//-----------------------------------------------
// delHandledAIGroup
//-----------------------------------------------
void CCharacter::delHandledAIGroup(CMission *m, TAIAlias nGroupAlias)
{
	TAIAlias nMissionAlias = m->getTemplateId(); // Get mission alias
	// Send message to AIS
	CDelHandledAIGroupMsg msg;
	msg.PlayerRowId = getEntityRowId();
	msg.GroupAlias = nGroupAlias;
	msg.MissionAlias = nMissionAlias;
	msg.send("AIS"); // send the message to all AIS

	string sDebugString = "user:" + getId().toString();
	sDebugString += " miss:" + CPrimitivesParser::aliasToString(nMissionAlias);

	// Remove the handle from the mission
	CMission *pMission = _Missions->getMissions(nMissionAlias);
	if (pMission == NULL)
	{
		sDebugString += " ERROR cant get mission in character from mission alias";
		MISLOG(sDebugString.c_str());
		return;
	}
	EGSPD::CHandledAIGroupPD *pGrp = pMission->getHandledAIGroups(nGroupAlias);
	if (pGrp != NULL)
	{
		pMission->deleteFromHandledAIGroups(nGroupAlias);
	}
	else
	{
		sDebugString += " ERROR cant find the handle on group '"
			+ CPrimitivesParser::aliasToString(nGroupAlias);
		MISLOG(sDebugString.c_str());
	}
}

//-----------------------------------------------
// delAllHandledAIGroup
//-----------------------------------------------
void CCharacter::delAllHandledAIGroup(CMission *m)
{
	TAIAlias nMissionAlias = m->getTemplateId(); // Get mission alias

	CMission *pMiss = _Missions->getMissions(nMissionAlias);
	if (pMiss != NULL)
	{
		// Remove all handledAIGroups in a mission
		map<uint32, EGSPD::CHandledAIGroupPD>::iterator itHAIG = pMiss->getHandledAIGroupsBegin();
		while (itHAIG != pMiss->getHandledAIGroupsEnd())
		{
			uint32 nGroupAlias = itHAIG->first;
			// Send del message to the AIS
			CDelHandledAIGroupMsg msg;
			msg.PlayerRowId = getEntityRowId();
			msg.GroupAlias = nGroupAlias;
			msg.MissionAlias = nMissionAlias;
			msg.send("AIS"); // send the message to all AIS

			++itHAIG;
		}

		// Remove all handles effectively
		itHAIG = pMiss->getHandledAIGroupsBegin();
		while (itHAIG != pMiss->getHandledAIGroupsEnd())
		{
			pMiss->deleteFromHandledAIGroups(itHAIG->first);
			itHAIG = pMiss->getHandledAIGroupsBegin();
		}
	}
}


//-----------------------------------------------
// spawnAllHandledAIGroup
//-----------------------------------------------
void CCharacter::spawnAllHandledAIGroup()
{
	// For all missions of the player
	map<TAIAlias, CMission*>::iterator it = getMissionsBegin();
	while (it != getMissionsEnd())
	{
		TAIAlias nMissionAlias = it->first;
		CMission *m = it->second;

		// Add all handledAIGroups present in mission
		map<uint32, EGSPD::CHandledAIGroupPD>::iterator itHAIG = m->getHandledAIGroupsBegin();
		while (itHAIG != m->getHandledAIGroupsEnd())
		{
			uint32 nGroupAlias = itHAIG->first;
			uint32 nDespawnTime = itHAIG->second.getDespawnTime();
			// Send add message to the AIS
			CAddHandledAIGroupMsg msg;
			msg.PlayerRowId = getEntityRowId();
			msg.GroupAlias = nGroupAlias;
			msg.MissionAlias = nMissionAlias;
			msg.DespawnTimeInTick = nDespawnTime;
			msg.send("AIS"); // send the message to all AIS

			++itHAIG;
		}

		++it;
	}
}


//-----------------------------------------------
// spawnAllHandledAIGroup
//-----------------------------------------------
void CCharacter::despawnAllHandledAIGroup()
{
	// For all missions of the player
	map<TAIAlias, CMission*>::iterator it = getMissionsBegin();
	while (it != getMissionsEnd())
	{
		TAIAlias nMissionAlias = it->first;
		CMission *m = it->second;

		// Remove all handledAIGroups in a mission
		map<uint32, EGSPD::CHandledAIGroupPD>::iterator itHAIG = m->getHandledAIGroupsBegin();
		while (itHAIG != m->getHandledAIGroupsEnd())
		{
			uint32 nGroupAlias = itHAIG->first;
			// Send del message to the AIS
			CDelHandledAIGroupMsg msg;
			msg.PlayerRowId = getEntityRowId();
			msg.GroupAlias = nGroupAlias;
			msg.MissionAlias = nMissionAlias;
			msg.send("AIS"); // send the message to all AIS

			++itHAIG;
		}

		++it;
	}
}


//-----------------------------------------------
// processItemsDecay
//-----------------------------------------------
void CCharacter::processItemsDecay()
{
} // processItemsDecay //


//-----------------------------------------------
// getCreatorNameId
//-----------------------------------------------
uint32 CCharacter::getCreatorNameId( const CEntityId &creatorId)
{
	SM_STATIC_PARAMS_2(params, STRING_MANAGER::player, STRING_MANAGER::integer);

	params[0].setEIdAIAlias( _Id, CAIAliasTranslator::getInstance()->getAIAlias(_Id) );
	if (creatorId != CEntityId::Unknown)
	{
		TDataSetRow row = TheDataset.getDataSetRow( creatorId );
		if ( TheDataset.isAccessible(row))
			params[1].Int = 0; // online creator
		else
			params[1].Int = 1; // offline creator
	}
	else
	{
		params[1].Int = 2; // unknown creator
	}
	return STRING_MANAGER::sendStringToClient( _EntityRowId, "ITEM_CREATOR", params);
} // getCreatorNameId //


//-----------------------------------------------
// Register character name in IOS
//-----------------------------------------------
void CCharacter::registerName(const ucstring &newName)
{
	CMessage msgName("CHARACTER_NAME_LANG");
	msgName.serial(_EntityRowId);

	string sTitle = getFullTitle();
	ucstring RegisteredName;
	if (newName.empty())
		RegisteredName = getName() + string("$") + sTitle + string("$");
	else
		RegisteredName = newName + string("$") + sTitle + string("$");
	msgName.serial( RegisteredName );
	// added 27/04/2006 : now for domain unification, we transmit home mainland session to IOS
	nlWrite(msgName, serial, getHomeMainlandSessionId());

	CPlayer *player = PlayerManager.getPlayer(PlayerManager.getPlayerId(_Id));
	if (player == NULL)
	{
		string lang("en");
		msgName.serial(lang);
	}
	else
	{
		std::string lang = player->getUserLanguage();
		msgName.serial(lang);
	}
	bool playerWithPrivilege = false;
	if( player != NULL )
	{
		if (!player->getUserPriv().empty() && !player->havePriv(":DEV:"))
		{
			playerWithPrivilege = true;
		}
	}

	// send all entities that are currently ignored
	// convert to vector<CEntityId> format
	vector<CEntityId>	ignores;
	ignores.resize(_IgnoreList.size());
	for(uint i=0;i<ignores.size();i++)
		ignores[i]= _IgnoreList[i].EntityId;

	// send to IOS
	msgName.serialCont(ignores);
	msgName.serial( playerWithPrivilege );
	sendMessageViaMirror ("IOS", msgName);
}


//-----------------------------------------------
// clearBeastTrain
//-----------------------------------------------
void CCharacter::clearBeastTrain()
{
	CPetCommandMsg commandMsg;
	commandMsg.Command = (uint16) CPetCommandMsg::FOLLOW;
	commandMsg.CharacterMirrorRow = TheDataset.getDataSetRow(_Id);

	const uint size = (uint)_BeastTrain.size();
	const uint nbPets = (uint)_PlayerPets.size();
	for ( uint i = 0 ; i < size ; ++i )
	{
		/// \todo Malkav: optimize this !!!!!
		// find pet index
		for (uint j = 0 ; j < nbPets ; ++j)
		{
			if( _PlayerPets[ j ].PetStatus == CPetAnimal::landscape )
			{
				if (_PlayerPets[ j ].SpawnedPets == _BeastTrain[i] )
				{
					commandMsg.PetMirrorRow = _PlayerPets[ j ].SpawnedPets;
					_PlayerPets[ j ].IsFollowing = true;
					break;
				}
			}
		}

		// update the beast
		CCreature *creature = CreatureManager.getCreature( TheDataset.getEntityId(_BeastTrain[i]) );
		if (creature)
		{
			creature->resetCharacterLeaderIndex();
		}
	}
	CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), commandMsg);

	_BeastTrain.clear();
} // clearBeastTrain //


//-----------------------------------------------
// addBeast
//-----------------------------------------------
void CCharacter::addBeast( uint16 petIndex )
{
	// get pet row id
	if (petIndex >= _PlayerPets.size() )
	{
		nlwarning("<CCharacter::addBeast> For entity %s, tried to access pet index %d but there is only %d pets", _Id.toString().c_str(), petIndex, _PlayerPets.size() );
		return;
	}

	if( _PlayerPets[ petIndex ].PetStatus != CPetAnimal::landscape )
	{
		nlwarning("<CCharacter::addBeast> For entity %s, tried to access pet index %d but it's not a spawned pet", _Id.toString().c_str(), petIndex );
		return;
	}

	const uint size = (uint)_BeastTrain.size();

	// check there is room in the train
	if (size == _TrainMaxSize)
	{
		nlwarning("<CCharacter::addBeast> Entity %s, tried to insert a beast in the train but there is no room in the train (size %d)", _Id.toString().c_str(), size );
		return;
	}

	_BeastTrain.push_back( _PlayerPets[petIndex].SpawnedPets );

	// send message to AI to add this beast to the train
	CPetCommandMsg commandMsg;
	commandMsg.Command = (uint16) CPetCommandMsg::FOLLOW;
	commandMsg.CharacterMirrorRow = _EntityRowId;
	commandMsg.PetMirrorRow = _PlayerPets[ petIndex ].SpawnedPets;
	CWorldInstances::instance().msgToAIInstance(getInstanceNumber(), commandMsg);

	_PlayerPets[ petIndex ].IsFollowing = true;

	// update the beast
	CCreature *creature = CreatureManager.getCreature( TheDataset.getEntityId(_PlayerPets[petIndex].SpawnedPets) );
	if (creature)
	{
		creature->setCharacterLeaderIndex(commandMsg.CharacterMirrorRow);
	}
	else
	{
		nlwarning("<CCharacter::addBeast> For player %s, unable to find creature datasetRow %d, (petIndex %d)",_Id.toString().c_str(), (uint32)_PlayerPets[petIndex].SpawnedPets.getIndex(), petIndex );
	}
} // addBeast //

//-----------------------------------------------
//	havePriv
//-----------------------------------------------
bool CCharacter::havePriv (const std::string &priv) const
{
	return PlayerManager.havePriv(getId(), priv);
}

//-----------------------------------------------
//	have any privilege
//-----------------------------------------------
bool CCharacter::haveAnyPrivilege() const
{
	return PlayerManager.haveAnyPriv(getId());
}

//-----------------------------------------------
//	setCurrentRegion
//-----------------------------------------------
void CCharacter::setCurrentRegion(uint16 region)
{
	_CurrentRegion = region;
	updateMagicProtectionAndResistance();
}

//-----------------------------------------------
//	setPlaces
//-----------------------------------------------
void CCharacter::setPlaces(const std::vector<const CPlace*> & places)
{
	nldebug("CCharacter_setPlaces(%u)",places.size());
	const uint size = (uint)places.size();
	_Places.resize(places.size());
	for ( uint i = 0; i < size; i++ )
	{
		_Places[i] = places[i]->getId();
	}
}

//-----------------------------------------------
// memorize
//-----------------------------------------------
void CCharacter::memorize(uint8 memorizationSet, uint8 index, uint16 phraseId, const vector<CSheetId> &bricks)
{
	// check all used bricks are known
	const uint size = (uint)bricks.size();
	for ( uint i = 0 ; i < size ; ++i)
	{
		if (_KnownBricks.find(bricks[i]) == _KnownBricks.end())
		{
			// brick isn't known by player
			nlwarning("Player %s tried to memorize a phrase with brick %s, but doesn't known it.", _Id.toString().c_str(), bricks[i].toString().c_str());
			return;
		}
	}

	// check phrase is valid
	if ( ! CPhraseManager::getInstance().checkPhraseValidity(bricks) )
	{
		nlwarning("Player %s tried to memorize a phrase which failed to be built by phrase manager, cancel", _Id.toString().c_str());
		return;
	}

	_MemorizedPhrases.memorize(memorizationSet, index, bricks, phraseId, _EntityRowId);
} // memorize //

//-----------------------------------------------
// addLink
//-----------------------------------------------
void CCharacter::addLink( CSLinkEffect * effect)
{
	if (!effect)
		return;

	// set the static action flag
	staticActionInProgress(true,STATIC_ACT_TYPES::Casting);

	const uint8 size = (uint8)_SEffectLinks.size();

	CEntityBase::addLink(effect);

	if (_SEffectLinks.size() > size)
	{
		// set new link in DB
//		ICDBStructNode *node = _PropertyDatabase.getICDBStructNodeFromName( NLMISC::toString( "EXECUTE_PHRASE:LINK:%u", size ) );
		CBankAccessor_PLR::TEXECUTE_PHRASE::TLINK::TArray &linkElem = CBankAccessor_PLR::getEXECUTE_PHRASE().getLINK().getArray(size);
//		if ( node )
//		{
//			_PropertyDatabase.setProp( node, "COUNTER", _PropertyDatabase.getProp("EXECUTE_PHRASE:LINK:0:COUNTER") + 1 );
			linkElem.setCOUNTER(_PropertyDatabase, (CBankAccessor_PLR::getEXECUTE_PHRASE().getLINK().getArray(0).getCOUNTER(_PropertyDatabase) + 1) &0xf );
//			_PropertyDatabase.setProp( node, "PHRASE", effect->getPhraseBookIndex() );
			linkElem.setPHRASE(_PropertyDatabase, effect->getPhraseBookIndex());
//			_PropertyDatabase.setProp( node, "SAP_COST",  (sint16)effect->costPerUpdate() );
			linkElem.setSAP_COST(_PropertyDatabase,  checkedCast<uint16>(effect->costPerUpdate()) );
//		}
	}

	// in case the effect cannot be added, set the flag to false again
	if ( _SEffectLinks.empty() )
		staticActionInProgress(false);
}// addLink

//-----------------------------------------------
// removeLink
//-----------------------------------------------
void CCharacter::removeLink( CSLinkEffect * effect, float factorOnSurvivalTime)
{
	CEntityBase::removeLink(effect, factorOnSurvivalTime);

	// set new link in DB
//	ICDBStructNode *node = _PropertyDatabase.getICDBStructNodeFromName( NLMISC::toString( "EXECUTE_PHRASE:LINK:%u", 0) );
//	if ( node )
//	{
//		_PropertyDatabase.setProp( node, "PHRASE", 0 );
	CBankAccessor_PLR::getEXECUTE_PHRASE().getLINK().getArray(0).setPHRASE(_PropertyDatabase, 0 );
//	}

	if ( _SEffectLinks.empty() )
		_StaticActionInProgress = false;
}// removeLink

//-----------------------------------------------
// stopAllLinks
//-----------------------------------------------
void CCharacter::stopAllLinks(float factorOnSurvivalTime)
{
	CEntityBase::stopAllLinks(factorOnSurvivalTime);

	for (uint i = 0; i < 10 ; ++i)
	{
//		ICDBStructNode *node = _PropertyDatabase.getICDBStructNodeFromName( NLMISC::toString( "EXECUTE_PHRASE:LINK:%u", i ) );
//		if ( node )
//		{
//			_PropertyDatabase.setProp( node, "PHRASE", 0);
			CBankAccessor_PLR::getEXECUTE_PHRASE().getLINK().getArray(i).setPHRASE(_PropertyDatabase, 0);
//		}
	}
}

//-----------------------------------------------
// applyDamageOnArmor
//-----------------------------------------------
inline sint32 CCharacter::applyDamageOnArmor( DMGTYPE::EDamageType dmgType, sint32 damage, SLOT_EQUIPMENT::TSlotEquipment forcedSlot)
{
	uint32 protection = 0;

	// only used by magic
	switch(dmgType)
	{
	case DMGTYPE::ROT:
		protection = getMagicProtection(PROTECTION_TYPE::Rot);
		break;
	case DMGTYPE::ACID:
		protection = getMagicProtection(PROTECTION_TYPE::Acid);
		break;
	case DMGTYPE::COLD:
		protection = getMagicProtection(PROTECTION_TYPE::Cold);
		break;
	case DMGTYPE::FIRE:
		protection = getMagicProtection(PROTECTION_TYPE::Fire);
		break;
	case DMGTYPE::POISON:
		protection = getMagicProtection(PROTECTION_TYPE::Poison);
		break;
	case DMGTYPE::ELECTRICITY:
		protection = getMagicProtection(PROTECTION_TYPE::Electricity);
		break;
	case DMGTYPE::SHOCK:
		protection = getMagicProtection(PROTECTION_TYPE::Shockwave);
		break;
	default:
		return damage;
	}

	if( protection > 0 )
	{
		uint32 absorbedDamage = (damage * protection) / 100;
		clamp( absorbedDamage, (uint32)0, _MaxAbsorption );
		damage -= absorbedDamage;
	}
	return damage;
}// applyDamageOnArmor


//-----------------------------------------------
// getArmorCastingMalus
//-----------------------------------------------
float CCharacter::getArmorCastingMalus()
{
	float malus = 0.0f;
	return malus;
}// getArmorCastingMalus


/*
 * Return the armor factor for a explosion (e.g. forage source explosion)
 */
float CCharacter::getActualDamageFromExplosionWithArmor( float dmg ) const
{
	const uint NbArmorSlots = 6;
	const float NbArmorSlotsF = 6.0f;
	CGameItemPtr armorPtr[NbArmorSlots];
	armorPtr[0] = getItem( INVENTORIES::equipment, SLOT_EQUIPMENT::HEAD );
	armorPtr[1] = getItem( INVENTORIES::equipment, SLOT_EQUIPMENT::CHEST );
	armorPtr[2] = getItem( INVENTORIES::equipment, SLOT_EQUIPMENT::ARMS );
	armorPtr[3] = getItem( INVENTORIES::equipment, SLOT_EQUIPMENT::HANDS );
	armorPtr[4] = getItem( INVENTORIES::equipment, SLOT_EQUIPMENT::LEGS );
	armorPtr[5] = getItem( INVENTORIES::equipment, SLOT_EQUIPMENT::FEET );
	float slotDdmg = dmg / NbArmorSlotsF;
	float bestArmorSkill = (float)getSkillBaseValue( getBestSkill() );
	float localizedDmgAbsorptionSum = 0;
	for ( uint i=0; i!=NbArmorSlots; ++i )
	{
		if ( armorPtr[i] == NULL )
			continue;

		float maxProtection = ((float)(armorPtr[i]->maxBluntProtection() + armorPtr[i]->maxPiercingProtection() + armorPtr[i]->maxSlashingProtection())) / 3.0f;
		if ( bestArmorSkill != 0 )
		{
			if ( armorPtr[i]->recommended() < bestArmorSkill )
				maxProtection *= ((float)armorPtr[i]->recommended()) / bestArmorSkill;
		}
		else
		{
			maxProtection = 0;
		}
		localizedDmgAbsorptionSum += min( slotDdmg * (float)armorPtr[i]->protectionFactor(), maxProtection / NbArmorSlotsF );
	}
	return dmg - localizedDmgAbsorptionSum;
}


//-----------------------------------------------
// checkPhrases :
// Check known and memorised phrases in cases some bricks would have been removed.
//-----------------------------------------------
void CCharacter::checkPhrases()
{
	// Re-build pre-built phrases (in case bricks changed) and remove phrases with still unknown bricks after.
	const uint nbKnownPhrases = (uint)_KnownPhrases.size();
	for(uint phraseIndex=0; phraseIndex<nbKnownPhrases; ++phraseIndex)
	{
		// if it's a preset phrase Re-build the phrase from sheet in case bricks changed.
		if (_KnownPhrases[phraseIndex].PhraseSheetId != CSheetId::Unknown)
		{
			learnPrebuiltPhrase(_KnownPhrases[phraseIndex].PhraseSheetId, phraseIndex, true);
		}

		// Get a reference on the current phrase (just easier to code).
		CKnownPhrase &phrase = _KnownPhrases[phraseIndex];

		// Parse all bricks in the current phrase.
		const uint nbBricks = (uint)phrase.PhraseDesc.Bricks.size();
		for(uint brickIndex=0; brickIndex<nbBricks; ++brickIndex)
		{
			// Get a reference on the current phrase (just easier to code).
			const CSheetId &brick = phrase.PhraseDesc.Bricks[brickIndex];
			// If the sheet does not exist,
			if(CSheets::getSBrickForm(brick) == 0)
			{
				// Log infos.
				nlwarning("CCharacter:checkPhrases: remove phrase with index '%d'(sheet '%u(%s)'), because the brick with index '%d' (sheet '%d(%s)') is invalid.",
					phraseIndex, phrase.PhraseSheetId.asInt(), phrase.PhraseSheetId.toString().c_str(),
					brickIndex, brick.asInt(), brick.toString().c_str());
				// Remove the phrase with this invalid brick.
				deleteKnownPhrase(phraseIndex);
				// \warning if one day, deleteKnown phrase whould erase the index, this code will need to be re-done.
				break;
			}
		}
	}
	// Re-memorise phrases to reconstruct them in case bricks inside had changed.
	_MemorizedPhrases.fixPhrases(_KnownPhrases, getEntityRowId());
}// checkPhrases //


//-----------------------------------------------
// sendPhrasesToClient
//-----------------------------------------------
void CCharacter::sendPhrasesToClient()
{
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "PHRASE:DOWNLOAD", bms) )
	{
		nlwarning("<CCharacter::sendPhrasesToClient> Msg name PHRASE:DOWNLOAD not found");
		return;
	}

	// Verify that known phrase are connected to slots !
	// slot 0 and 1 are reserved

	vector<sint32> vConvertTable;

	// Detect referenced phrase in the action bar and set the convert table to 0 if the
	// phrase should exists in the final known phrase vector

	vConvertTable.resize(_KnownPhrases.size(), -1); // init with not present in action bar
	{
		const std::vector<CMemorizationSet*> &sets = _MemorizedPhrases.getMemorizationSets();
		for (uint ii = 0 ; ii < sets.size() ; ++ii)
		{
			if ( sets[ii] != NULL)
			{
				const std::vector<CMemorizedPhrase*> &phrases = sets[ii]->getMemorizedPhrases();
				for (uint j = 0; j < phrases.size() ; ++j)
					if (phrases[j] != NULL)
					{
						if ((phrases[j]->PhraseId == 0) || (phrases[j]->PhraseId == 1))
							nlwarning("PhraseId == 0 ou PhraseId == 1"); // ERREUR !!!!!!!!!

						if (phrases[j]->PhraseId >= vConvertTable.size())
							nlwarning("PhraseId >= _KnownPhrases.size()"); // ERREUR !!!!!!!!!
						else
							vConvertTable[phrases[j]->PhraseId] = 0;
					}
			}
		}
	}
	vConvertTable[0] = 0; // reserved
	vConvertTable[1] = 0;

	// Initialize the convert table with contigous indexes : new phrase position is vConvertTable[oldPos]
	uint32 i, j = 0;
	for (i = 0; i < vConvertTable.size(); ++i)
	{
		if (vConvertTable[i] != -1)
		{
			vConvertTable[i] = j;
			j++;
		}
	}

	// Make the new Known Phrase Vector (eliminate all phrase not present in action bar)
	vector<CKnownPhrase> vNewPhrase;
	vNewPhrase.resize(j);
	j = 0;
	for (i = 0; i < vConvertTable.size(); ++i)
	{
		if (vConvertTable[i] != -1) // is present in action bar
		{
			vNewPhrase[j] = _KnownPhrases[i];
			j++;
		}
	}
	_KnownPhrases = vNewPhrase;

	// remap the action bar phrase positions with the vConvertTable help
	{
		const std::vector<CMemorizationSet*> &sets = _MemorizedPhrases.getMemorizationSets();
		for (uint ii = 0 ; ii < sets.size() ; ++ii)
		{
			if ( sets[ii] != NULL)
			{
				const std::vector<CMemorizedPhrase*> &phrases = sets[ii]->getMemorizedPhrases();
				for (uint j = 0; j < phrases.size() ; ++j)
					if (phrases[j] != NULL)
					{
						phrases[j]->PhraseId = (uint16)vConvertTable[phrases[j]->PhraseId];
					}
			}
		}
	}

	// End of cleaning unused known phrase (phrase not present in action bar)

	// known phrases
	vector<CSPhraseSlot> knownPhrases;
	CSPhraseSlot phrase;
	knownPhrases.reserve(_KnownPhrases.size());
	for (uint i = 0 ; i < _KnownPhrases.size() ; ++i)
	{
		if ( !_KnownPhrases[i].empty() )
		{
			phrase.Phrase			= _KnownPhrases[i].PhraseDesc;
			phrase.PhraseSheetId	= _KnownPhrases[i].PhraseSheetId;
			phrase.KnownSlot = i;
			knownPhrases.push_back(phrase);
		}
	}

	bms.serialCont(knownPhrases);

	// memorized phrases
	vector<CSPhraseMemorySlot> vect;

	CSPhraseMemorySlot p;
	const std::vector<CMemorizationSet*> &sets = _MemorizedPhrases.getMemorizationSets();
	for (uint i = 0 ; i < sets.size() ; ++i)
	{
		if ( sets[i] != 0)
		{
			const std::vector<CMemorizedPhrase*> &phrases = sets[i]->getMemorizedPhrases();
			for (uint j = 0; j < phrases.size() ; ++j)
			{
				if (phrases[j] != 0)
				{
					p.MemoryLineId = i;
					p.MemorySlotId = j;
					p.PhraseId = phrases[j]->PhraseId;
					vect.push_back(p);
				}
			}
		}
	}

	bms.serialCont(vect);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );

} // sendPhrasesToClient //


//-----------------------------------------------
// learnPhrase
//-----------------------------------------------
void CCharacter::learnPhrase(const vector<CSheetId> &bricks, uint16 phraseId, const ucstring &name)
{
	if (phraseId >= _KnownPhrases.size())
		_KnownPhrases.resize(phraseId + 1);

	_KnownPhrases[phraseId].clear();

	const uint size = (uint)bricks.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if (bricks[i] != NLMISC::CSheetId::Unknown)
			_KnownPhrases[phraseId].PhraseDesc.Bricks.push_back(bricks[i]);
	}
	_KnownPhrases[phraseId].PhraseDesc.Name = name;
} // learnPhrase //

//-----------------------------------------------
// deleteKnownPhrase
//-----------------------------------------------
void CCharacter::deleteKnownPhrase(uint16 phraseId)
{
	// for the moment do not change the vector size, an "hole" is no problem
	// if this has to change then don't forget to change the serial method and check any method calling this

	if (phraseId >= _KnownPhrases.size())
		return;

	_KnownPhrases[phraseId].clear();

	// delete all memorized phrase pointing on this one
	const std::vector<CMemorizationSet*> &sets = _MemorizedPhrases.getMemorizationSets();
	for (uint i = 0; i < sets.size(); ++i)
	{
		if (!sets[i]) continue;
		const std::vector<CMemorizedPhrase*> &phrases = sets[i]->getMemorizedPhrases();
		for (uint j = 0 ; j < phrases.size() ; ++j)
		{
			if (phrases[j] && phrases[j]->PhraseId == phraseId)
			{
				_MemorizedPhrases.forget(i,j);
			}
		}
	}
} // deleteKnownPhrase //

//-----------------------------------------------
// getFirstFreePhraseSlot
//-----------------------------------------------
uint16 CCharacter::getFirstFreePhraseSlot()
{
	for ( uint16 i = 0; i < _KnownPhrases.size(); i++ )
	{
		if (_KnownPhrases[i].empty() )
			return i;
	}
	return 0xFFFF;
} // getFirstFreePhraseSlot //

//-----------------------------------------------
// learnPrebuiltPhrase
//-----------------------------------------------
bool CCharacter::learnPrebuiltPhrase( const CSheetId &phraseId, uint16 knownPhraseIndex, bool replace, bool onlyLearnBricks)
{
	// if phrase already learnt and not in replace mode, return false
	if (_BoughtPhrases.find(phraseId) != _BoughtPhrases.end())
		return false;

	// Try to get the phrase sheet from the ID.
	const CStaticRolemasterPhrase *phrase = CSheets::getSRolemasterPhrase(phraseId);
	if(phrase == 0)
	{
		return false;
	}

	if ( !onlyLearnBricks )
	{
		//check if phrase is already known
		for( uint p = 0; p < _KnownPhrases.size(); ++p )
		{
			if( _KnownPhrases[ p ].PhraseSheetId == phraseId )
			{
				return false;
			}
		}

		// Increase the vector of known phrases.
		// \todo ... : set a maximum to have no pb with hackers that could force a really too big value.
		if ( knownPhraseIndex >= _KnownPhrases.size() )
		{
			_KnownPhrases.resize(knownPhraseIndex + 1);
		}
		// Check there is not already a phrase in the given index (if replace = false).
		else if( (_KnownPhrases[knownPhraseIndex].empty()==false) && (replace==false) )
		{
			// if player already knowns a phrase in knownPhraseIndex, cancel
			nlwarning("Player %s tried to learn a phrase in slot %u, but slot is already full", _Id.toString().c_str(), knownPhraseIndex);
			return false;
		}

		log_Character_LearnPhrase(phraseId);
		// Set the bricks in the phrase.
		_KnownPhrases[knownPhraseIndex].PhraseDesc.Bricks = phrase->Bricks;
		// Set the phrase Id.
		_KnownPhrases[knownPhraseIndex].PhraseSheetId = phraseId;
	}

	// Set the phrase as bought.
	_BoughtPhrases.insert( phraseId );

	// Set bricks in the phrase as known bricks.
	const uint size = (uint)phrase->Bricks.size();
	for ( uint i = 0; i < size ; ++i)
	{
		if(phrase->Bricks[i] != CSheetId::Unknown)
		{
			addKnownBrick( phrase->Bricks[i] );
		}
		else
		{
			nlwarning("<CCharacter::learnPrebuiltPhrase> Phrase %s contain unknown bricks", phraseId.toString().c_str());
		}
	}

	// Well Done
	return true;
} // learnPrebuiltPhrase //


//-----------------------------------------------
// getFirstFreeSlotInKnownPhrase
//-----------------------------------------------
uint16 CCharacter::getFirstFreeSlotInKnownPhrase()
{
	const uint size = (uint)_KnownPhrases.size();
	for (uint i = 2 ; i < size ; ++i)
	{
		if (_KnownPhrases[i].empty())
			return i;
	}

	// vector is full, increase size
	if (size < 2)
		_KnownPhrases.resize(3);
	else
		_KnownPhrases.resize(size+1);
	return (uint16)_KnownPhrases.size()-1;
} // getFirstFreeSlotInKnownPhrase //


void CCharacter::sendUrl(const string &url, const string &salt)
{
	string control;
	if (!salt.empty())
	{
		control = "&hmac="+getHMacSHA1((uint8*)&url[0], (uint32)url.size(), (uint8*)&salt[0], (uint32)salt.size()).toString();
	}
	else
	{
		string defaultSalt = toString(getLastConnectedDate());
		control = "&hmac="+getHMacSHA1((uint8*)&url[0], (uint32)url.size(), (uint8*)&defaultSalt[0], (uint32)defaultSalt.size()).toString();
	}

	nlinfo(url.c_str());
	TVectorParamCheck titleParams;
	TVectorParamCheck textParams;
	uint32 userId = PlayerManager.getPlayerId(getId());
	std::string name = "CUSTOM_URL_"+toString(userId);
	ucstring phrase = ucstring(name+"(){[WEB : "+url+control+"]}");
	NLNET::CMessage	msgout("SET_PHRASE");
	msgout.serial(name);
	msgout.serial(phrase);
	sendMessageViaMirror("IOS", msgout);

	uint32 titleId = STRING_MANAGER::sendStringToUser(userId, "web_transactions", titleParams);
	uint32 textId = STRING_MANAGER::sendStringToUser(userId, name, textParams);
	PlayerManager.sendImpulseToClient(getId(), "USER:POPUP", titleId, textId);
}

void CCharacter::validateDynamicMissionStep(const string &url)
{
	sendUrl(url+"&player_eid="+getId().toString()+"&event=mission_step_finished", getSalt());
}

/// set custom mission param
void CCharacter::setCustomMissionParams(const string &missionName, const string &params)
{
	_CustomMissionsParams[missionName] = params;
}

/// add custom mission param
void CCharacter::addCustomMissionParam(const string &missionName, const string &param)
{
	if (!_CustomMissionsParams.empty() && _CustomMissionsParams.find(missionName) != _CustomMissionsParams.end())
		_CustomMissionsParams[missionName] += ","+param;
	else
		_CustomMissionsParams[missionName] = param;
}

/// get custom mission params 
vector<string> CCharacter::getCustomMissionParams(const string &missionName)
{
	vector<string> params;
	if (_CustomMissionsParams.empty()) 
	{
		return params;
	}
	
	if (!_CustomMissionsParams.empty() && _CustomMissionsParams.find(missionName) != _CustomMissionsParams.end())
	{
		if (!_CustomMissionsParams[missionName].empty())
			NLMISC::splitString(_CustomMissionsParams[missionName], ",", params);
	}
	return params;
}


// !!! Deprecated !!!
void CCharacter::addWebCommandCheck(const string &url, const string &data, const string &salt)
{
	uint webCommand = getWebCommandCheck(url);
	string randomString;

	for (uint8 i = 0; i < 8; i++)
	{
		uint32 r = (uint32)((double)rand()/((double)RAND_MAX)*62);
		randomString += randomStrings[r];
	}
 
	if (webCommand == INVENTORIES::NbBagSlots)
	{
		CGameItemPtr item = createItemInInventoryFreeSlot(INVENTORIES::bag, 1, 1, CSheetId("web_transaction.sitem"));
		if (item != 0)
		{
			if (data.empty())
			{
				item->setCustomText(ucstring(url));
				vector<string> infos;
				NLMISC::splitString(url, "\n", infos);
				sendUrl(infos[0]+"&player_eid="+getId().toString()+"&event=command_added", salt);
			}
			else
			{
				vector<string> infos;
				NLMISC::splitString(data, ",", infos);
				string finalData = randomString+infos[0];
				for (uint i = 1; i < infos.size(); i++)
				{
					finalData += ","+randomString+infos[i];
				}
				item->setCustomText(ucstring(url+"\n"+finalData));
				sendUrl(url+"&player_eid="+getId().toString()+"&event=command_added&transaction_id="+randomString, salt);
			}
		}
	}
	else
	{
		CInventoryPtr inv = getInventory(INVENTORIES::bag);
		if(inv)
		{
			CGameItemPtr item = inv->getItem(webCommand);
			if (item != NULL && item->getStaticForm() != NULL )
			{
				if(item->getStaticForm()->Name == "Web Transaction"
					|| item->getStaticForm()->Family == ITEMFAMILY::SCROLL)
				{
					string cText = item->getCustomText().toString();
					if (!cText.empty())
					{
						vector<string> infos;
						NLMISC::splitString(cText, "\n", infos);
						vector<string> datas;
						NLMISC::splitString(infos[1], ",", datas);
						sendUrl(infos[0]+"&player_eid="+getId().toString()+"&event=command_added&transaction_id="+datas[0].substr(0, 8), salt);
					}
				}
			}
		}
	}
}

// !!! Deprecated !!!
uint CCharacter::getWebCommandCheck(const string &url)
{
	CInventoryPtr inv = getInventory(INVENTORIES::bag);
	if(inv)
	{
		for(uint i = 0; i < INVENTORIES::NbBagSlots; ++i)
		{
			CGameItemPtr item = inv->getItem(i);
			if (item != NULL && item->getStaticForm() != NULL )
			{
				if(item->getStaticForm()->Name == "Web Transaction"
					|| item->getStaticForm()->Family == ITEMFAMILY::SCROLL)
				{
					string cText = item->getCustomText().toString();
					if (!cText.empty())
					{
						vector<string> infos;
						NLMISC::splitString(cText, "\n", infos);
						if (infos.size() == 2)
						{
							if (infos[0] == url)
							{
								return i;
							}
						}
					}
				}
			}
		}
	}

	return INVENTORIES::NbBagSlots;
}

// !!! Deprecated !!!
uint CCharacter::checkWebCommand(const string &url, const string &data, const string &hmac, const string &salt)
{
	if (salt.empty())
		return INVENTORIES::NbBagSlots;
	uint slot = getWebCommandCheck(url);
	if (slot == INVENTORIES::NbBagSlots)
		return slot;
	string checksum = url + data + getId().toString();
	string realhmac = getHMacSHA1((uint8*)&checksum[0], (uint32)checksum.size(), (uint8*)&salt[0], (uint32)salt.size()).toString();
	if (realhmac == hmac)
		return slot;
	return INVENTORIES::NbBagSlots;
}


//-----------------------------------------------
// getAvailablePhrasesList
//-----------------------------------------------
void CCharacter::getAvailablePhrasesList( const string &brickFilter, vector<CSheetId> &selectedPhrases, EGSPD::CPeople::TPeople people, bool bypassBrickRequirements, bool includeNonRolemasterBricks )
{
	H_AUTO(CCharacterGetAvailablePhrasesList);

	const uint nbSkills = (uint)_Skills._Skills.size();

	buildAvailablePhrasesList( _Id, brickFilter, _KnownBricks, _BoughtPhrases, _Skills._Skills, selectedPhrases, 250, people, bypassBrickRequirements, includeNonRolemasterBricks );
} // getAvailablePhrasesList //


//-----------------------------------------------
// harvestCorpseResult
//-----------------------------------------------
void CCharacter::harvestCorpseResult( const vector<uint16> &qualities )
{
	H_AUTO(CCharacter_harvestCorpseResult);
	// get creature being harvested
	CCreature *creature = CreatureManager.getCreature( _MpSourceId );
	if (creature == NULL)
		return;
	if (creature->harvesterRowId() != _EntityRowId)
		return;
	const CStaticCreatures * form = creature->getForm();
	if (!form)
		return;

	if( creature->getContextualProperty().directAccessForStructMembers().lootable() )
	{
		pickUpItem( creature->getId() );
	}

	creature->writeMpInfos();
	sendDynamicSystemMessage(_Id, "WOS_HARVEST_FOUND_MP");

	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
	nlassert(invTemp != NULL);
	uint32 usedSlot = creature->getLootSlotCount();

	const uint8 size = (uint8)creature->getMps().size();
	for (uint i = 0; i < size ; ++i)
	{
		const CCreatureRawMaterial *mp = creature->getCreatureRawMaterial(i);
		if (mp != NULL)
		{
			if( usedSlot+i < INVENTORIES::NbTempInvSlots )
			{
				invTemp->setDispQuality(usedSlot+i, form->getXPLevel());
			}
			else
			{
				nlwarning("<CCharacter::harvestCorpseResult> creature %s has too many RM/Loot for temp inventory : %d",creature->getType().toString().c_str(),usedSlot+size);
				break;
			}
		}
	}
	for (uint k = usedSlot+size; k < INVENTORIES::NbTempInvSlots; ++k)
	{
		invTemp->setDispSheetId(k, CSheetId::Unknown);
		invTemp->setDispQuantity(k, 0);
	}

} // harvestCorpseResult //


/*
 * Get the indices in the 'materials' vector of the items that have are currently required by a mission.
 * If all items that match have a level tool low compared to the provided itemLevel, a message is sent to the player.
 */
void	CCharacter::getMatchingMissionLootRequirements( uint16 itemLevel, const std::vector<NLMISC::CSheetId>& materials, std::vector<CAutoQuarterItemDescription>& matchingItems )
{
	bool foundMatchingOK = false;
	bool foundMatchingExceptLevel = false;

	// Browse active missions
	for ( std::map<TAIAlias, CMission*>::iterator itMission=getMissionsBegin(); itMission!=getMissionsEnd(); ++itMission )
	{
		CMission *mission = (*itMission).second;
		//uint32 missionTemplateId = mission->getTemplateId();
		CMissionTemplate *missionTemplate = CMissionManager::getInstance()->getTemplate( (*itMission).first );
		// Browse active steps
		for ( std::map<uint32, EGSPD::CActiveStepPD>::iterator itStep=mission->getStepsBegin(); itStep!=mission->getStepsEnd(); ++itStep )
		{
			EGSPD::CActiveStepPD& step = (*itStep).second;
			uint32 indexInTemplate = step.getIndexInTemplate(); // or (*itStep).first?

			// Access the step if it's a 'loot' one
			if ( (! missionTemplate) || (indexInTemplate > missionTemplate->Steps.size()) )
			{
				nlwarning( "Invalid active mission step %u/%hu", (*itMission).first, (uint16)indexInTemplate );
				continue;
			}
			IMissionStepTemplate *missionStepTemplate = missionTemplate->Steps[indexInTemplate-1];
			CMissionStepLootRm *missionStepLootTemplate = dynamic_cast<CMissionStepLootRm*>(missionStepTemplate);
			if ( ! missionStepLootTemplate )
				continue;

			// Get the states of the mission steps
			uint stateIndex = 0;
			for ( map<uint32,EGSPD::CActiveStepStatePD>::iterator itState=step.getStatesBegin(); itState!=step.getStatesEnd(); ++itState )
			{
				// We only test uncomplete steps
				uint32 currentState = (*itState).second.getState();
				if ( currentState != 0 )
				{
					// Find the substep of the template corresponding to this state
					const vector<IMissionStepItem::CSubStep>& requestedItems = missionStepLootTemplate->getRequestedItems();
					BOMB_IF( stateIndex >= requestedItems.size(), "State index out of substep range", ++stateIndex; continue );
					const IMissionStepItem::CSubStep& itemRequirement = requestedItems[stateIndex];

					// Browse the creature materials to see which ones match
					for ( vector<NLMISC::CSheetId>::const_iterator icm=materials.begin(); icm!=materials.end(); ++icm )
					{
						if ( itemRequirement.Sheet == (*icm) )
						{
							if ( itemLevel < itemRequirement.Quality )
							{
								// The level is too low
								foundMatchingExceptLevel = true;
							}
							else
							{
								// Matching OK
								CAutoQuarterItemDescription desc;
								uint itemIndex = (uint)(icm - materials.begin());
#ifdef NL_DEBUG
								nlassert( itemIndex <= (uint16)~0 );
#endif
								desc.ItemIndex = (uint16)itemIndex;
								desc.QuantityNeeded = (uint16)currentState; // the number of remaining items to get, not the initial one
								desc.PlayerNeedingTheItem = this;
								matchingItems.push_back( desc );
								foundMatchingOK = true;
							}
						}
					}
				}
				++stateIndex;
			}
		}
	}

	// Tell the player if he can't get any material because of the level
	// (note: If he can get, it doesn't mean it will get)
	if ( (! foundMatchingOK) && foundMatchingExceptLevel )
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( getEntityRowId(), "AUTO_LOOT_LEVEL_TOO_LOW" );
	}
}


//-----------------------------------------------
// pickUpRawMaterial. Return false if we are too encumbered to pick up
//-----------------------------------------------
bool CCharacter::pickUpRawMaterial( uint32 indexInTempInv, bool * lastMaterial )
{
	H_AUTO(CCharacter_pickUpRawMaterial);

	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
	nlassert(invTemp != NULL);

	if (lastMaterial)
		*lastMaterial = false;

	if ( _ForageProgress )
	{
		if (lastMaterial)
			*lastMaterial = true;

		if ( _ForageProgress->resultCanBeTaken() )
		{
			// Forage
			uint16 quality = 0;
			quality = invTemp->getDispQuality(indexInTempInv);

			if (quality != 0)
			{
				// Create and stack the item in the bag
				CGameItemPtr item = createItem(	_ForageProgress->quality(),
												_ForageProgress->amount(),
												_ForageProgress->material() );
				if (item == NULL)
					return false;
				if (!addItemToInventory(INVENTORIES::bag, item)) // Autostack the item in the bag
					return false;

				// Send event to the mission : we have harvested some item !
				const CStaticItem *itemForm = CSheets::getForm( _ForageProgress->material() );
				if (itemForm != NULL)
				{
					CMissionEventForage event( _ForageProgress->material(), _ForageProgress->amount(), _ForageProgress->quality() );
					processMissionEventWithTeamMate( event );

					SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
					params[0].Int = (sint32)_ForageProgress->amount();
					params[1].SheetId = _ForageProgress->material();
					params[2].Int = (sint32)_ForageProgress->quality();

					PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "HARVEST_SUCCESS", params );
				}

				clearHarvestDB();
				//CZoneManager::getInstance().removeRmFromDeposit( this, _DepositHarvestInformation.DepositIndex, _DepositHarvestInformation.DepositIndexContent,_HarvestedQuantity);
			}
		}
	}
	else
	{
		// Quartering
		CCreature *creature = CreatureManager.getCreature( _MpSourceId );
		if (creature == NULL || creature->harvesterRowId() != _EntityRowId)
		{
			resetHarvestInfos();
			return true;
		}

		if( creature->getLootSlotCount() > indexInTempInv )
		{
			return false; // don't try to quarter an item for looting
		}

		// first slots are filled with loot items, quarter items are not in temp inv but only info in DB
		uint32 rawMaterialIndex = indexInTempInv - creature->getLootSlotCount();
		const CCreatureRawMaterial * mp = creature->getCreatureRawMaterial( rawMaterialIndex );
		if (mp == NULL)
		{
			resetHarvestInfos();
			return true;
		}

		// get Mp Info
		uint16 quality = 0;
		quality = invTemp->getDispQuality(indexInTempInv);

		if ( quality != 0 )
		{
			// Create and stack the item in the bag
			CGameItemPtr item = createItem(quality, mp->Quantity, mp->ItemId);
			if (item == NULL)
				return false;
			if (!addItemToInventory(INVENTORIES::bag, item)) // Autostack the item in the bag
				// Cannot create the item (too encumbered), the user has to make room in his inventory
				return false;

			const CStaticItem *itemForm = CSheets::getForm(mp->ItemId);
			if (itemForm != NULL)
			{
				CMissionEventLootRm event(mp->ItemId, mp->Quantity, quality);
				processMissionEvent( event );

				SM_STATIC_PARAMS_3(params, STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
				params[0].Int = (sint32)mp->Quantity;
				params[1].SheetId = mp->ItemId;
				params[2].Int = (sint32)quality;

				PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "HARVEST_SUCCESS", params );

				// send loot txt to every team members (except looter ..)
				CTeam * team = TeamManager.getTeam(_TeamId);
				if (team)
				{
					SM_STATIC_PARAMS_4(paramsOther, STRING_MANAGER::player,STRING_MANAGER::integer, STRING_MANAGER::item, STRING_MANAGER::integer);
					paramsOther[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
					paramsOther[1].Int = (sint32)mp->Quantity;
					if(paramsOther[1].Int == 0)
						paramsOther[1].Int = 1;
					paramsOther[2].SheetId = mp->ItemId;
					paramsOther[3].Int = (sint32)quality;

					const list<CEntityId> &ids = team->getTeamMembers();

					list<CEntityId>::const_iterator itEnd = ids.end();
					for (list<CEntityId>::const_iterator it = ids.begin() ; it != itEnd ; ++it)
					{
						TDataSetRow entityRowId = TheDataset.getDataSetRow(*it);
						if( entityRowId != _EntityRowId )
						{
							sendDynamicSystemMessage( entityRowId, "LOOT_SUCCESS_OTHER", paramsOther );
						}
					}
				}
			}
		}

		// remove the quantity of mp harvested from the ressource
		creature->removeMp( rawMaterialIndex, mp->Quantity );

		// if there is no more mp on creature, call endHarvest
		const vector<CCreatureRawMaterial> &rm = creature->getMps();
		uint i;
		for (i = 0 ;i < rm.size() ; ++i)
		{
			if (rm[i].Quantity > 0)
			{
				break;
			}
		}
		if (i == rm.size())
		{
			if (lastMaterial)
				*lastMaterial = true;
		}
	}

	// reset harvest info
	resetHarvestInfos();

	// clear slot
	invTemp->clearDisp(indexInTempInv);
	return true;
}

//-----------------------------------------------
// sendCloseTempInventoryImpulsion
//-----------------------------------------------
void CCharacter::sendCloseTempInventoryImpulsion()
{
	// Sage: May 9 2007
	// The live servers are crashing due to an infinite indirect recursion loop
	// itemTempInventoryToBag() calls endHarvest() calls sendCloseTempInventoryImpulsion() calls getAllTempInventoryItems() calls itemTempInventoryToBag() ...
	// the following anti bug is a temporaray fix to prevent inifinte recursion and stack overflow
	static bool isRecursing= false;																	// **** Temp Fix 1/4 **** //
	BOMB_IF(isRecursing,"CCharacter::sendCloseTempInventoryImpulsion is recursing!",return);		// **** Temp Fix 2/4 **** //
	isRecursing= true;																				// **** Temp Fix 3/4 **** //

	getAllTempInventoryItems(false);

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "HARVEST:CLOSE_TEMP_INVENTORY", bms) )
	{
		nlwarning("<CCharacter::sendCloseTempInventoryImpulsion> Msg name HARVEST:CLOSE_TEMP_INVENTORY not found");
		return;
	}
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );

	isRecursing= false;																				// **** Temp Fix 4/4 **** //
} // sendCloseTempInventoryImpulsion //

//-----------------------------------------------
// setFameValuesPlayer
//-----------------------------------------------
void CCharacter::setFameValuePlayer(uint32 factionIndex, sint32 playerFame, sint32 fameMax, uint16 fameTrend)
{
	uint32 firstTribeFameIndex = CStaticFames::getInstance().getFirstTribeFameIndex();
	uint32 firstTribeDbIndex = CStaticFames::getInstance().getDatabaseIndex( firstTribeFameIndex );
	uint32 fameIndexInDatabase = CStaticFames::getInstance().getDatabaseIndex( factionIndex );

	if( factionIndex >= firstTribeFameIndex )
	{
		if (playerFame != NO_FAME)
		{
//			_PropertyDatabase.setProp( toString("FAME:TRIBE%d:VALUE", fameIndexInDatabase - firstTribeDbIndex), sint64(float(playerFame)/FameAbsoluteMax*100) );
			CBankAccessor_PLR::getFAME().getTRIBE(fameIndexInDatabase - firstTribeDbIndex).setVALUE(_PropertyDatabase, checkedCast<sint8>(float(playerFame)/FameAbsoluteMax*100) );
//			_PropertyDatabase.setProp( toString("FAME:TRIBE%d:TREND", fameIndexInDatabase - firstTribeDbIndex), fameTrend );
			CBankAccessor_PLR::getFAME().getTRIBE(fameIndexInDatabase - firstTribeDbIndex).setTREND(_PropertyDatabase, checkedCast<uint8>(fameTrend) );
		}
		else
		{
//			_PropertyDatabase.setProp( toString("FAME:TRIBE%d:VALUE", fameIndexInDatabase - firstTribeDbIndex), 0);
			CBankAccessor_PLR::getFAME().getTRIBE(fameIndexInDatabase - firstTribeDbIndex).setVALUE(_PropertyDatabase, 0 );
//			_PropertyDatabase.setProp( toString("FAME:TRIBE%d:TREND", fameIndexInDatabase - firstTribeDbIndex), 0);
			CBankAccessor_PLR::getFAME().getTRIBE(fameIndexInDatabase - firstTribeDbIndex).setTREND(_PropertyDatabase, 0 );
		}
//		_PropertyDatabase.setProp( toString("FAME:TRIBE%d:THRESHOLD", fameIndexInDatabase - firstTribeDbIndex), sint64(float(fameMax)/FameAbsoluteMax*100) );
		CBankAccessor_PLR::getFAME().getTRIBE(fameIndexInDatabase - firstTribeDbIndex).setTHRESHOLD(_PropertyDatabase, checkedCast<sint8>(float(fameMax)/FameAbsoluteMax*100)  );
	}
	// they are some unused fame between civ/cult fame and tribe fame...
	else if( fameIndexInDatabase < PVP_CLAN::NbClans - PVP_CLAN::BeginClans )
	{
		if (playerFame != NO_FAME)
		{
//			_PropertyDatabase.setProp( toString("FAME:PLAYER%d:VALUE", fameIndexInDatabase), sint64(float(playerFame)/FameAbsoluteMax*100) );
			CBankAccessor_PLR::getFAME().getPLAYER(fameIndexInDatabase).setVALUE(_PropertyDatabase, checkedCast<sint8>(float(playerFame)/FameAbsoluteMax*100) );
//			_PropertyDatabase.setProp( toString("FAME:PLAYER%d:TREND", fameIndexInDatabase), fameTrend );
			CBankAccessor_PLR::getFAME().getPLAYER(fameIndexInDatabase).setTREND(_PropertyDatabase, checkedCast<uint8>(fameTrend) );
		}
		else
		{
//			_PropertyDatabase.setProp( toString("FAME:PLAYER%d:VALUE", fameIndexInDatabase), 0);
			CBankAccessor_PLR::getFAME().getPLAYER(fameIndexInDatabase).setVALUE(_PropertyDatabase, 0 );
//			_PropertyDatabase.setProp( toString("FAME:PLAYER%d:TREND", fameIndexInDatabase), 0);
			CBankAccessor_PLR::getFAME().getPLAYER(fameIndexInDatabase).setTREND(_PropertyDatabase, 0 );
		}
//		_PropertyDatabase.setProp( toString("FAME:PLAYER%d:THRESHOLD", fameIndexInDatabase), sint64(float(fameMax)/FameAbsoluteMax*100) );
		CBankAccessor_PLR::getFAME().getPLAYER(fameIndexInDatabase).setTHRESHOLD(_PropertyDatabase, checkedCast<sint8>(float(fameMax)/FameAbsoluteMax*100) );
	}

	bool canPvp = false;
	for (uint8 fameIdx = 0; fameIdx < 7; fameIdx++)
	{
		sint32 fame = CFameInterface::getInstance().getFameIndexed(_Id, fameIdx);

		if (fame >= PVPFameRequired*6000)
		{
			canPvp = true;
		}
		else if (fame <= -PVPFameRequired*6000)
		{
			canPvp = true;
		}
	}

	if (_LoadingFinish)
	{
		if(!canPvp && (_PVPFlag || getPvPRecentActionFlag()) )
		{
			_PVPFlag = false;
			_PVPFlagLastTimeChange = 0;
			_PVPFlagTimeSettedOn = 0;
			_PVPRecentActionTime = 0;
			_HaveToUpdatePVPMode = true;
		}
		updatePVPClanVP();
		setPVPFlagDatabase();
		
		// handle with faction channel
		CPVPManager2::getInstance()->updateFactionChannel(this);
	}

}

//-----------------------------------------------
// setFameBoundaries
//-----------------------------------------------
void CCharacter::setFameBoundaries()
{
	// Send all possible changed variables, just to make sure.
//	_PropertyDatabase.setProp("FAME:THRESHOLD_TRADE", FameMinToTrade/kFameMultipler );
	CBankAccessor_PLR::getFAME().setTHRESHOLD_TRADE(_PropertyDatabase, FameMinToTrade/kFameMultipler );
//	_PropertyDatabase.setProp("FAME:THRESHOLD_KOS", FameMinToKOS/kFameMultipler );
	CBankAccessor_PLR::getFAME().setTHRESHOLD_KOS(_PropertyDatabase, FameMinToKOS/kFameMultipler );
}

//-----------------------------------------------
// setFameValues
//-----------------------------------------------
void CCharacter::resetFameDatabase()
{
	CFameInterface &fi = CFameInterface::getInstance();

	for (uint i=0; i<CStaticFames::getInstance().getNbFame(); ++i)
	{
		// update player fame info
		sint32 fame = fi.getFameIndexed(_Id, i, false, true);
		sint32 maxFame = CFameManager::getInstance().getMaxFameByFactionIndex(getAllegiance(), i);
		setFameValuePlayer(i, fame, maxFame, 0);
	}
}

//-----------------------------------------------
// protectedSlot
//-----------------------------------------------
void CCharacter::protectedSlot( SLOT_EQUIPMENT::TSlotEquipment slot)
{
	_ProtectedSlot = slot;

	uint8 index;
	switch(slot)
	{
	case SLOT_EQUIPMENT::HEAD:
		index = 1;
		break;
	case SLOT_EQUIPMENT::CHEST:
		index = 2;
		break;
	case SLOT_EQUIPMENT::ARMS:
		index = 3;
		break;
	case SLOT_EQUIPMENT::HANDS:
		index = 4;
		break;
	case SLOT_EQUIPMENT::LEGS:
		index = 5;
		break;
	case SLOT_EQUIPMENT::FEET:
		index = 6;
		break;
	default:
		index = 0;
		break;
	};

//	_PropertyDatabase.setProp("DEFENSE:PROTECTED_SLOT", index);
	CBankAccessor_PLR::getDEFENSE().setPROTECTED_SLOT(_PropertyDatabase, index);

	const std::vector<sint8> &mod = PHRASE_UTILITIES::getDefenseLocalisationModifiers(index-1);
	for (uint i = 0 ; i < mod.size() ; ++i)
	{
//		_PropertyDatabase.setProp( NLMISC::toString("DEFENSE:SLOTS:%u:MODIFIER", i), mod[i]);
		CBankAccessor_PLR::getDEFENSE().getSLOTS().getArray(i).setMODIFIER(_PropertyDatabase, mod[i]);
	}
} // protectedSlot //

//-----------------------------------------------
// updateCombatEventFlags
//-----------------------------------------------
void CCharacter::updateCombatEventFlags()
{
	const TGameCycle date = CTickEventHandler::getGameCycle();
	for (uint i = 0 ; i < BRICK_FLAGS::NbCombatFlags ; ++i)
	{
		if( _CombatEventFlagTicks[i].EndTick <= date )
		{
			_CombatEventFlagTicks[i].StartTick = 0;
			_CombatEventFlagTicks[i].EndTick = 0;
		}
	}

	// update database if flags have changed
	bool updateDB = false;
	for (uint i = 0 ; i < BRICK_FLAGS::NbCombatFlags ; ++i )
	{
		if( _CombatEventFlagTicks[i].OldEndTick != _CombatEventFlagTicks[i].EndTick )
		{
			updateDB = true;
			_CombatEventFlagTicks[i].OldStartTick = _CombatEventFlagTicks[i].StartTick;
			_CombatEventFlagTicks[i].OldEndTick = _CombatEventFlagTicks[i].EndTick;
		}
	}
	if( updateDB )
	{
		updateBrickFlagsDBEntry();
	}
} // updateCombatEventFlags //


//-----------------------------------------------
// setPowerFlagDates
//-----------------------------------------------
void CCharacter::setPowerFlagDates()
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	// powers
	std::vector <CPowerActivationDate>::iterator it = _ForbidPowerDates.PowerActivationDates.begin();
	while (it != _ForbidPowerDates.PowerActivationDates.end())
	{
		uint32 flag = BRICK_FLAGS::powerTypeToFlag((*it).PowerType) - BRICK_FLAGS::BeginPowerFlags;

		if ( (*it).ActivationDate <= time && _ForbidPowerDates.doNotClear == false )
		{
			// erase returns an iterator that designates the first element remaining beyond any elements removed, or end() if no such element exists.
			it = _ForbidPowerDates.PowerActivationDates.erase(it);
			_PowerFlagTicks[flag].StartTick = 0;
			_PowerFlagTicks[flag].EndTick = 0;
		}
		else
		{
			_PowerFlagTicks[flag].StartTick = (*it).DeactivationDate;
			_PowerFlagTicks[flag].EndTick = (*it).ActivationDate;
			++it;
		}
	}

} // setPowerFlagDates //


//-----------------------------------------------
// setAuraFlagDates
//-----------------------------------------------
void CCharacter::setAuraFlagDates()
{
	const NLMISC::TGameCycle time = CTickEventHandler::getGameCycle();

	uint32 flag = BRICK_FLAGS::Aura - BRICK_FLAGS::BeginPowerFlags;
	if ( (_ForbidAuraUseEndDate < time) || (_ForbidAuraUseEndDate - time > 72000) )
 	{
		_ForbidAuraUseStartDate = 0;
		_ForbidAuraUseEndDate = 0;
	}
	_PowerFlagTicks[flag].StartTick = _ForbidAuraUseStartDate;
	_PowerFlagTicks[flag].EndTick = _ForbidAuraUseEndDate;

} // setAuraFlagDates //


//-----------------------------------------------
// updatePowerAndAuraFlags
//-----------------------------------------------
void CCharacter::updatePowerAndAuraFlags()
{
	// set powers flags
	setPowerFlagDates();

	// set aura flag
	setAuraFlagDates();

	// update database if flags have changed
	bool updateDB = false;
	for (uint i = 0 ; i < 32 ; ++i )
	{
		if( _PowerFlagTicks[i].EndTick != _PowerFlagTicks[i].OldEndTick )
		{
			updateDB = true;
			_PowerFlagTicks[i].OldStartTick = _PowerFlagTicks[i].StartTick;
			_PowerFlagTicks[i].OldEndTick = _PowerFlagTicks[i].EndTick;
		}
	}
	if( updateDB )
	{
		updateBrickFlagsDBEntry();
	}

} // updatePowerAndAuraFlags //


/*
 * Start a forage extraction if not already started.
 * - If the player was not foraging, a forage progress is created.
 * - If the player was foraging the same source, the forage progress is resumed.
 * - If the player was foraging another source, the forage progress is reset (and the previous contents lost).
 */
void CCharacter::beginOrResumeForageSession( const NLMISC::CSheetId& materialSheetId, const TDataSetRow& sourceRowId, SKILLS::ESkills usedSkill, bool isTheExtractor /*, CGameItemPtr forageToolUsed*/ )
{
	bool resetProgress = false;
	if ( ! _ForageProgress )
	{
		if (!enterTempInventoryMode(TEMP_INV_MODE::Forage))
			return;

		// Create progress structure (note: currently, the only forage tool stored is the one here, when creating the progress structure. Idem for usedSkill)
		SCharacteristicsAndScores& focus = getScores()._PhysicalScores[SCORES::focus];
		_ForageProgress = new CForageProgress( materialSheetId, sourceRowId, usedSkill, focus.Current /*, forageToolUsed*/ );
		resetProgress = true;
	}
	else
	{
		if ( sourceRowId != _ForageProgress->sourceRowId() )
		{
			// Reset progress
			SCharacteristicsAndScores& focus = getScores()._PhysicalScores[SCORES::focus];
			_ForageProgress->reset( materialSheetId, sourceRowId, focus.Current );
			resetProgress = true;
		}
	}

	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
	nlassert(invTemp != NULL);
	if ( resetProgress && isTheExtractor ) // if care action and player is an extractor, send because the player may reswitch to an extraction
	{
		invTemp->clearDisp(0);
		invTemp->setDispSheetId(0, materialSheetId);

	}
	invTemp->enableTakeDisp(false);
}

void CCharacter::setExtractionProgressCounters( uint16 amountX10, uint16 qualityX10 )
{
	CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
	nlassert(invTemp != NULL);
	invTemp->setDispQuality(0, qualityX10);
	invTemp->setDispQuantity(0, amountX10);
}

/*
 * End a forage extraction session, but do not close the temp inventory if something can be taken.
 */
void CCharacter::giveForageSessionResult( const CHarvestSource *source )
{
	if ( _ForageProgress )
	{
		// Give XP, raw material, etc. (if possible)
		if ( _ForageProgress->giveForageResult( this, source ) ) // if successful, any current extraction will stop
		{
			CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
			nlassert(invTemp != NULL);
			invTemp->enableTakeDisp(true);
		}
		else
		{
			PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "FORAGE_FAIL" );
			endForageSession();
		}
	}
}


/*
 * If a forage session is active, reset the temp inventory
 * and delete the forage progress structure created by beginOrResumeForageSession().
 * Otherwise do nothing.
 */
void CCharacter::endForageSession()
{
	if ( _ForageProgress )
	{
		CTempInventory *invTemp = (CTempInventory*)(CInventoryBase*)getInventory(INVENTORIES::temporary);
		nlassert(invTemp != NULL);
		invTemp->setDispSheetId(0, CSheetId::Unknown);

		leaveTempInventoryMode();

		// Delete the forage progress structure. Now any current extraction will stop.
		delete _ForageProgress;
		_ForageProgress = NULL;
	}
}

//-----------------------------------------------
// CCharacter::canEntityUseAction
//-----------------------------------------------
bool CCharacter::canEntityUseAction(CBypassCheckFlags bypassCheckFlags, bool sendMessage) const
{
	if ( !CEntityBase::canEntityUseAction(bypassCheckFlags, sendMessage) )
		return false;

	// observer cannot use actions
	if (havePriv(NoActionAllowedPriv))
	{
		return false;
	}

	if (isInWater() && !bypassCheckFlags.Flags.InWater)
	{
		if (sendMessage)
			CCharacter::sendDynamicSystemMessage( _EntityRowId, "NO_ACTION_WHILE_SWIMMING" );

		return false;
	}

	if( !bypassCheckFlags.Flags.WhileSitting )
	{
		if (isSitting())
		{
			if (sendMessage)
				CCharacter::sendDynamicSystemMessage( _EntityRowId, "NO_ACTION_WHEN_SITTING" );

			return false;
		}
	}

	if ( _GearLatency->isLatent()  )
	{
		if (sendMessage)
			CCharacter::sendDynamicSystemMessage( _EntityRowId, "NO_ACTION_WHILE_EQUIPPING" );

		return false;
	}

	if ( _IntangibleEndDate > CTickEventHandler::getGameCycle() )
	{
		if (sendMessage)
			CCharacter::sendDynamicSystemMessage( _EntityRowId, "NO_ACTION_WHILE_INTANGIBLE" );

		return false;
	}

	if ( ! bypassCheckFlags.Flags.OnMount )
	{
		if( TheDataset.isAccessible( getEntityMounted() ) )
		{
			if (sendMessage)
				CCharacter::sendDynamicSystemMessage( _EntityRowId, "NO_ACTION_WHILE_MOUNTED" );
			return false;
		}
	}

	return true;
} // canEntityUseAction //


//-----------------------------------------------
// CCharacter::canEntityDefend
//-----------------------------------------------
bool CCharacter::canEntityDefend()
{
	if (_StaticActionInProgress)
		return false;

	if (isInWater())
		return false;

	// cant defend while equipping
	if ( getGearLatency().isLatent() )
		return false;

	return CEntityBase::canEntityDefend();
} // canEntityDefend //

//-----------------------------------------------
// CCharacter::addSabrinaEffect
//-----------------------------------------------
bool CCharacter::addSabrinaEffect( CSEffect *effect )
{
	if (!effect)
	{
		nlwarning("<CCharacter::addSabrinaEffect> tried to add a NULL effect for entity %s", _Id.toString().c_str());
		return false;
	}

	const bool addSabrinaEffect = CEntityBase::addSabrinaEffect(effect);

	if (addSabrinaEffect)
	{
		// add effect to active modifiers if sheetid is set
		CSheetId sheet = effect->getAssociatedSheetId();
		if (sheet != CSheetId::Unknown)
		{
			effect->setEffectIndexInDB( addEffectInDB(sheet, EFFECT_FAMILIES::isEffectABonus(effect->getFamily())) );
		}
		return true;
	}
	else
	{
		return false;
	}
} // addSabrinaEffect //

//-----------------------------------------------
// CCharacter::removeSabrinaEffect
//-----------------------------------------------
bool CCharacter::removeSabrinaEffect( CSEffect *effect, bool activateSleepingEffect )
{
	if (!effect)
	{
		nlwarning("<CCharacter::removeSabrinaEffect> tried to remove a NULL effect for entity %s", _Id.toString().c_str());
		return false;
	}

	const bool removeSabrinaEffect = CEntityBase::removeSabrinaEffect(effect, activateSleepingEffect);

	if (removeSabrinaEffect)
	{
		// remove effect from DB unless effect have been disabled
		const sint8 index = effect->getEffectIndexInDB();
		if (index >= 0)
		{
			const bool bonus = EFFECT_FAMILIES::isEffectABonus(effect->getFamily());
			if (bonus && index < (sint)_ModifiersInDB.Bonus.size())
			{
				if (_ModifiersInDB.Bonus[effect->getEffectIndexInDB()].Disabled == false )
				{
					removeEffectInDB((uint8)effect->getEffectIndexInDB(), true);
				}
			}
			else if (index < (sint)_ModifiersInDB.Malus.size())
			{
				if (_ModifiersInDB.Malus[effect->getEffectIndexInDB()].Disabled == false )
				{
					removeEffectInDB((uint8)effect->getEffectIndexInDB(), false);
				}
			}
		}
	}

	return false;
} // removeSabrinaEffect //


//--------------------------------------------------------------
//	CCharacter::getCarreidWeight()
//--------------------------------------------------------------
uint32 CCharacter::getCarriedWeight()
{
	return _Inventory[INVENTORIES::bag]->getInventoryWeight();
}

//--------------------------------------------------------------
//	CCharacter::getMagicResistance()
//--------------------------------------------------------------
uint32 CCharacter::getMagicResistance(RESISTANCE_TYPE::TResistanceType magicResistanceType) const
{
	uint32 val = getUnclampedMagicResistance(magicResistanceType);
	NLMISC::clamp( val, (uint32)0, (uint32)((_BaseResistance + MaxMagicResistanceBonus) * 100) );
	return val;
}

//--------------------------------------------------------------
//	CCharacter::getMagicResistance()
//--------------------------------------------------------------
uint32 CCharacter::getMagicResistance(EFFECT_FAMILIES::TEffectFamily effectFamily)
{
	RESISTANCE_TYPE::TResistanceType resistanceType = EFFECT_FAMILIES::getAssociatedResistanceType(effectFamily);
	return getMagicResistance(resistanceType);
}

//--------------------------------------------------------------
//	CCharacter::getMagicResistance()
//--------------------------------------------------------------
uint32 CCharacter::getMagicResistance(DMGTYPE::EDamageType dmgType)
{
	RESISTANCE_TYPE::TResistanceType resistanceType = DMGTYPE::getAssociatedResistanceType(dmgType);
	return getMagicResistance(resistanceType);
}

//--------------------------------------------------------------
// addPlayerToFriendList
//--------------------------------------------------------------
void CCharacter::addPlayerToFriendList(const ucstring &name)
{
	std::string fullName = CShardNames::getInstance().makeFullNameFromRelative(getHomeMainlandSessionId(), name.toUtf8());
	addPlayerToFriendList(NLMISC::CEntityIdTranslator::getInstance()->getByEntity(ucstring::makeFromUtf8(fullName)));
}

//--------------------------------------------------------------
// addPlayerToIgnoreList
//--------------------------------------------------------------
void CCharacter::addPlayerToIgnoreList(const ucstring &name)
{
	std::string fullName = CShardNames::getInstance().makeFullNameFromRelative(getHomeMainlandSessionId(), name.toUtf8());
	addPlayerToIgnoreList( NLMISC::CEntityIdTranslator::getInstance()->getByEntity(ucstring::makeFromUtf8(fullName)));
}

/// Compute the 'visual' online state of a friend character
TCharConnectionState CCharacter::isFriendCharVisualyOnline(const NLMISC::CEntityId &friendId)
{
	TCharConnectionState ret = ccs_offline;

	if (CEntityIdTranslator::getInstance()->isEntityOnline(friendId))
	{
		if ( PlayerManager.hasBetterCSRGrade(friendId, _Id, true))
		{
			// better CSR grade return always 'offline' status
			return ccs_offline;
		}

		ret = ccs_online;
	}

	// Handle friend preference setting
	CCharacter *friendChar = PlayerManager.getChar(friendId);
	if (friendChar != NULL)
	{
		volatile TFriendVisibility friendMode = friendChar->getFriendVisibility();
		switch (friendMode)
		{
			case VisibleToGuildOnly:
				{
					uint32 fgid = friendChar->getGuildId();
					uint32 mgid = this->getGuildId();
					bool inSameGuild = (mgid != 0) && (fgid == mgid);
					if ( ! inSameGuild)
					{
						return ccs_offline;
					}
				}
				break;
			case VisibleToGuildAndFriends:
				if (this->isIgnoredBy(friendId))
				{
					return ccs_offline;
				}
				break;
			case VisibleToAll: // fallthrough
			default:
				break; // no-op
		}
	}

	// Additional online check for ring shard :
	//   - a contact is online only if it is in the same ring session
	if (ret == ccs_online && IsRingShard)
	{
		if (friendChar == NULL)	// not found ! set offline
			ret = ccs_offline;
		else
		{
			if (sessionId() == friendChar->sessionId())
				ret = ccs_offline;
		}
	}
	if (ret == ccs_offline)
	{
		// check for 'far online'
		if (IShardUnifierEvent::getInstance() && IShardUnifierEvent::getInstance()->isCharacterOnlineAbroad(friendId))
			ret = ccs_online_abroad;
	}

	return ret;
}

//--------------------------------------------------------------
//	CCharacter::sendContactListInit()
//--------------------------------------------------------------
void CCharacter::sendContactListInit()
{
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;

	if ( ! GenericMsgManager.pushNameToStream( "TEAM:CONTACT_INIT", bms) )
	{
		nlwarning("<CEntityBase::sendContactListInit> Msg name TEAM:CONTACT_INIT not found");
		return;
	}

	// rebuild the contact ids (client will do the same code)
	_ContactIdPool=0;

	// build friend list with names
	vector< uint32 > friendStringIds;
	vector< TCharConnectionState > friendOnlineStatus;
	friendStringIds.resize(_FriendsList.size());
	friendOnlineStatus.resize(_FriendsList.size());
	for (uint i = 0 ; i < _FriendsList.size() ; ++i)
	{
		// associate a unique id
		_FriendsList[i].ContactId= _ContactIdPool++;
		// fill array
		friendStringIds[i] = CEntityIdTranslator::getInstance()->getEntityNameStringId(_FriendsList[i].EntityId);
		friendOnlineStatus[i] = isFriendCharVisualyOnline(_FriendsList[i].EntityId);
	}

	// build ignore list
	vector<ucstring> ignoreList;
	ignoreList.resize(_IgnoreList.size());
	for (uint i = 0 ; i < _IgnoreList.size() ; ++i)
	{
		// associate a unique id
		_IgnoreList[i].ContactId= _ContactIdPool++;
		// fill array
		ignoreList[i] = CEntityIdTranslator::getInstance()->getByEntity(_IgnoreList[i].EntityId);
	}

	// send to client
	bms.serialCont(friendStringIds);
	uint32 nbState = (uint32)friendOnlineStatus.size();
	bms.serial(nbState);
	for (uint i=0; i<nbState; ++i)
	{
		bms.serialShortEnum(friendOnlineStatus[i]);
	}
//	bms.serialCont(friendOnlineStatus);
	bms.serialCont(ignoreList);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
} // sendContactListInit //

//--------------------------------------------------------------
// CCharacter::syncContactListWithCharNameChanges
//--------------------------------------------------------------
void CCharacter::syncContactListWithCharNameChanges(const std::vector<NLMISC::CEntityId> &charNameChanges)
{
	// NB: the algo is in O(N*M) (M==charNameChanges.size()) but don't care since M should be very small
	bool	mustUpdate= false;


	// For all friends/ignored, if the name has changed, must update
	for(uint i=0;i<_FriendsList.size();i++)
	{
		if(mustUpdate)
			break;
		for(uint j=0;j<charNameChanges.size();j++)
		{
			if(_FriendsList[i].EntityId==charNameChanges[j])
			{
				mustUpdate= true;
				break;
			}
		}
	}
	for(uint i=0;i<_IgnoreList.size();i++)
	{
		if(mustUpdate)
			break;
		for(uint j=0;j<charNameChanges.size();j++)
		{
			if(_IgnoreList[i].EntityId==charNameChanges[j])
			{
				mustUpdate= true;
				break;
			}
		}
	}


	// If must update, then just resend the whole contact list.
	// NB: not optimal, but sufficient because this case is very rare (a char name is changed by a GM)
	if(mustUpdate)
		sendContactListInit();
}


void CCharacter::setInRoomOfPlayer(const NLMISC::CEntityId &id)
{
	_inRoomOfPlayer = id;
}

const NLMISC::CEntityId& CCharacter::getInRoomOfPlayer()
{
	return _inRoomOfPlayer;
}

//--------------------------------------------------------------
// CCharacter::havePlayerRoomAccess
//--------------------------------------------------------------

bool CCharacter::playerHaveRoomAccess(const NLMISC::CEntityId &id)
{	
	const uint size = (uint)_RoomersList.size();
	for ( uint i =0 ; i < size ; ++i)
	{
		if ( _RoomersList[i].getShortId() == id.getShortId())
			return true;
	}
	return false;
}

//--------------------------------------------------------------
// CCharacter::addRoomAccessToPlayer
//--------------------------------------------------------------

void CCharacter::addRoomAccessToPlayer(const NLMISC::CEntityId &id)
{
	// if player not found
	if (id == CEntityId::Unknown || PlayerManager.getChar(id)==NULL)
	{
		if ( ! (IShardUnifierEvent::getInstance() && IShardUnifierEvent::getInstance()->isCharacterOnlineAbroad(id)))
		{
			// player not found => message
			PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "OPERATION_OFFLINE");
			return;
		}
	}

	// check not already in list
	const uint size = (uint)_RoomersList.size();
	for ( uint i =0 ; i < size ; ++i)
	{
		if ( _RoomersList[i].getShortId() == id.getShortId())
		{
			return;
		}
	}

	uint32 playerId = PlayerManager.getPlayerId(id);
	_RoomersList.push_back(id);
}

//--------------------------------------------------------------
//	CCharacter::addPlayerToFriendList()
//--------------------------------------------------------------
void CCharacter::addPlayerToFriendList(const NLMISC::CEntityId &id)
{
	// if player not found
	if (id == CEntityId::Unknown)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "OPERATION_NOTEXIST");
		return;
	}

	// check not already in list
	const uint size = (uint)_FriendsList.size();
	for ( uint i =0 ; i < size ; ++i)
	{
		if ( _FriendsList[i].EntityId.getShortId() == id.getShortId())
		{
			return;
		}
	}

	if(haveAnyPrivilege() == false && PlayerManager.haveAnyPriv(id))
		return; // a character without privilege can't add one with privilege.

	uint32 playerId = PlayerManager.getPlayerId(id);

	// check the two char aren't from the same account
	if (playerId == PlayerManager.getPlayerId(_Id))
	{
		egs_chinfo("Char %s tried to add %s in his friend list but they are from the same account->return", _Id.toString().c_str(), id.toString().c_str());
		return;
	}

	// get online status
	TCharConnectionState onlineStatus = isFriendCharVisualyOnline(id);

	// reference count
	contactListRefChange( id, AddedAsFriend);

	// add the char to friends
	CContactId	contactId;
	contactId.EntityId= id;
	contactId.ContactId= _ContactIdPool++;	// create a new Id for client/server communication
	_FriendsList.push_back(contactId);


	// send create message to client
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;

	if ( ! GenericMsgManager.pushNameToStream( "TEAM:CONTACT_CREATE", bms) )
	{
		nlwarning("<CEntityBase::addPlayerToFriendList> Msg name TEAM:CONTACT_CREATE not found");
		return;
	}

	uint32	nameId = CEntityIdTranslator::getInstance()->getEntityNameStringId(id);
	uint8	listIndex = 0;

	bms.serial(contactId.ContactId);
	bms.serial(nameId);
	bms.serialShortEnum(onlineStatus);
	bms.serial(listIndex);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );

}


//--------------------------------------------------------------
//	CCharacter::addPlayerToLeagueList() // unused, need more tests
//--------------------------------------------------------------
void CCharacter::addPlayerToLeagueList(const NLMISC::CEntityId &id)
{
	// if player not found
	if (id == CEntityId::Unknown)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "OPERATION_NOTEXIST");
		return;
	}

	// check not already in list
	const uint size = (uint)_LeagueList.size();
	for ( uint i =0 ; i < size ; ++i)
	{
		if ( _LeagueList[i].EntityId.getShortId() == id.getShortId())
		{
			return;
		}
	}

	if(haveAnyPrivilege() == false && PlayerManager.haveAnyPriv(id))
		return; // a character without privilege can't add one with privilege.

	uint32 playerId = PlayerManager.getPlayerId(id);

	// check the two char aren't from the same account
	if (playerId == PlayerManager.getPlayerId(_Id))
	{
		egs_chinfo("Char %s tried to add %s in his friend list but they are from the same account->return", _Id.toString().c_str(), id.toString().c_str());
		return;
	}

	// reference count
	contactListRefChange( id, AddedAsLeague);

	// add the char to friends
	CContactId	contactId;
	contactId.EntityId= id;
	contactId.ContactId= _ContactIdPool++;	// create a new Id for client/server communication
	_LeagueList.push_back(contactId);


	// send create message to client
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;

	if ( ! GenericMsgManager.pushNameToStream( "TEAM:CONTACT_CREATE", bms) )
	{
		nlwarning("<CEntityBase::addPlayerToLeagueList> Msg name TEAM:CONTACT_CREATE not found");
		return;
	}
	
	TCharConnectionState onlineStatus = ccs_online;

	uint32	nameId = CEntityIdTranslator::getInstance()->getEntityNameStringId(id);
	uint8	listIndex = 2;

	bms.serial(contactId.ContactId);
	bms.serial(nameId);
	bms.serialShortEnum(onlineStatus);
	bms.serial(listIndex);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );

}



//--------------------------------------------------------------
//	CCharacter::addPlayerToIgnoreList()
//--------------------------------------------------------------
void CCharacter::addPlayerToIgnoreList(const NLMISC::CEntityId &id)
{
	if (id == CEntityId::Unknown)
	{
		// player not found => message
		PHRASE_UTILITIES::sendDynamicSystemMessage( _EntityRowId, "OPERATION_NOTEXIST");
		return;
	}

	// check not already ignored
	const uint size = (uint)_IgnoreList.size();
	for ( uint i =0 ; i < size ; ++i)
	{
		if ( _IgnoreList[i].EntityId.getShortId() == id.getShortId())
		{
			return;
		}
	}

	if(haveAnyPrivilege() == false && PlayerManager.haveAnyPriv(id))
		return; // a character without privilege can't add one with privilege.

	// check the two char aren't from the same account
	if (id.getShortId() >> 4 == _Id.getShortId() >> 4)
	{
		egs_chinfo("Char %s tried to add %s in his ignore list but they are from the same account->return", _Id.toString().c_str(), id.toString().c_str());
		return;
	}

	// reference count
	contactListRefChange( id, AddToIgnored);

	// add the char to ignore list
	CContactId	contactId;
	contactId.EntityId= id;
	contactId.ContactId= _ContactIdPool++;	// create a new Id for client/server communication
	_IgnoreList.push_back(contactId);


	// send add msg to client
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;

	if ( ! GenericMsgManager.pushNameToStream( "TEAM:CONTACT_CREATE", bms) )
	{
		nlwarning("<CEntityBase::addPlayerToIgnoreList> Msg name TEAM:CONTACT_CREATE not found");
		return;
	}

	uint32	nameId = CEntityIdTranslator::getInstance()->getEntityNameStringId(id);
	// get online status
	TCharConnectionState onlineStatus = isFriendCharVisualyOnline(id);
	uint8	listIndex = 1;

	bms.serial(contactId.ContactId);
	bms.serial(nameId);
	bms.serialShortEnum(onlineStatus);
	bms.serial(listIndex);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );


	// update ios state
	uint32 playerId = PlayerManager.getPlayerId(id);
	CPlayer *player = PlayerManager.getPlayer( playerId );
	if ( (!player) || (!player->havePriv( ":SGM:GM:VG:SG:G:EM:EG:" )) ) // if online, messages from CSRs can't be ignored
	{
		CEntityId senderId = getId();
		CEntityId ignoredId = id;
		CMessage msgName("IGNORE");
		msgName.serial(senderId);
		msgName.serial(ignoredId);
		sendMessageViaMirror ("IOS", msgName);
	}

}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromFriendListByIndex()
//--------------------------------------------------------------
void CCharacter::removePlayerFromFriendListByIndex(uint16 index)
{
	if (index >= _FriendsList.size())
		return;

	const	CEntityId id = _FriendsList[index].EntityId;
	uint32	contactId= _FriendsList[index].ContactId;

	// remove entry
	_FriendsList.erase(_FriendsList.begin() + index);
	sendRemoveContactMessage(contactId, 0);
	contactListRefChange( id, RemovedFromFriends);
}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromFriendListByIndex() // unused, need more tests
//--------------------------------------------------------------
void CCharacter::removePlayerFromLeagueListByIndex(uint16 index)
{
	if (index >= _LeagueList.size())
		return;

	const	CEntityId id = _LeagueList[index].EntityId;
	uint32	contactId= _LeagueList[index].ContactId;

	// remove entry
	_LeagueList.erase(_LeagueList.begin() + index);
	sendRemoveContactMessage(contactId, 2);
	contactListRefChange( id, RemovedFromLeague);
}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromIgnoreListByIndex()
//--------------------------------------------------------------
void CCharacter::removePlayerFromIgnoreListByIndex(uint16 index)
{
	if (index >= _IgnoreList.size())
		return;

	const	CEntityId id = _IgnoreList[index].EntityId;
	uint32	contactId= _IgnoreList[index].ContactId;

	// remove entry
	_IgnoreList.erase(_IgnoreList.begin() + index);
	sendRemoveContactMessage(contactId, 1);
	contactListRefChange( id, RemovedFromIgnored);

	// update ios state
	CEntityId senderId = getId();
	CEntityId ignoredId = id;
	CMessage msgName("UNIGNORE");
	msgName.serial(senderId);
	msgName.serial(ignoredId);
	sendMessageViaMirror ("IOS", msgName);
}

//--------------------------------------------------------------
//	CCharacter::removeRoomAccesToPlayer()
//--------------------------------------------------------------
void CCharacter::removeRoomAccesToPlayer(const NLMISC::CEntityId &id, bool kick)
{
	if (id == NLMISC::CEntityId::Unknown)
		return;

	CCharacter *target = PlayerManager.getChar(id);

	for ( uint i = 0 ; i < _RoomersList.size() ; ++i)
	{
		if ( _RoomersList[i].getShortId() == id.getShortId() )
		{
			_RoomersList.erase(_RoomersList.begin() + i);
			return;
		}
	}

	if (kick & (target->getInRoomOfPlayer().getShortId() == getId().getShortId()))
	{
		target->setInRoomOfPlayer(CEntityId::Unknown);
		if (!TheDataset.isAccessible(getEntityRowId()))
			return;

		const CTpSpawnZone * zone = CZoneManager::getInstance().getTpSpawnZone(target->getBuildingExitZone());
		if (zone)
		{
			sint32 x,y,z;
			float heading;
			zone->getRandomPoint(x,y,z,heading);
			target->tpWanted(x,y,z,true,heading);
		}
	}
}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromFriendListByEntityId()
//--------------------------------------------------------------
void CCharacter::removePlayerFromFriendListByEntityId(const NLMISC::CEntityId &id)
{
	if (id == NLMISC::CEntityId::Unknown)
		return;

	for ( uint i = 0 ; i < _FriendsList.size() ; ++i)
	{
		if ( _FriendsList[i].EntityId.getShortId() == id.getShortId() )
		{
			removePlayerFromFriendListByIndex(i);
			break;
		}
	}
}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromLeagueListByEntityId() // unused, need more tests
//--------------------------------------------------------------
void CCharacter::removePlayerFromLeagueListByEntityId(const NLMISC::CEntityId &id)
{
	if (id == NLMISC::CEntityId::Unknown)
		return;

	for ( uint i = 0 ; i < _LeagueList.size() ; ++i)
	{
		if ( _LeagueList[i].EntityId.getShortId() == id.getShortId() )
		{
			removePlayerFromLeagueListByIndex(i);
			break;
		}
	}
}


//--------------------------------------------------------------
//	CCharacter::removePlayerFromIgnoreListByEntityId()
//--------------------------------------------------------------
void CCharacter::removePlayerFromIgnoreListByEntityId(const NLMISC::CEntityId &id)
{
	if (id == NLMISC::CEntityId::Unknown)
		return;

	for ( uint i = 0 ; i < _IgnoreList.size() ; ++i)
	{
		if ( _IgnoreList[i].EntityId.getShortId() == id.getShortId() )
		{
			removePlayerFromIgnoreListByIndex(i);
			break;
		}
	}
}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromFriendListByContactId()
//--------------------------------------------------------------
void CCharacter::removePlayerFromFriendListByContactId(uint32 contactId)
{
	for ( uint i = 0 ; i < _FriendsList.size() ; ++i)
	{
		if ( _FriendsList[i].ContactId == contactId )
		{
			removePlayerFromFriendListByIndex(i);
			break;
		}
	}
}

//--------------------------------------------------------------
//	CCharacter::removePlayerFromFriendListByContactId() unused, need more tests
//--------------------------------------------------------------
void CCharacter::removePlayerFromLeagueListByContactId(uint32 contactId)
{
	for ( uint i = 0 ; i < _LeagueList.size() ; ++i)
	{
		if ( _LeagueList[i].ContactId == contactId )
		{
			removePlayerFromLeagueListByIndex(i);
			break;
		}
	}
}


//--------------------------------------------------------------
//	CCharacter::removePlayerFromIgnoreListByContactId()
//--------------------------------------------------------------
void CCharacter::removePlayerFromIgnoreListByContactId(uint32 contactId)
{
	for ( uint i = 0 ; i < _IgnoreList.size() ; ++i)
	{
		if ( _IgnoreList[i].ContactId == contactId )
		{
			removePlayerFromIgnoreListByIndex(i);
			break;
		}
	}
}

//--------------------------------------------------------------
// getFriendByContactId(uint32 contactId)
//--------------------------------------------------------------
const NLMISC::CEntityId	&CCharacter::getFriendByContactId(uint32 contactId)
{
	for ( uint i = 0 ; i < _FriendsList.size() ; ++i)
	{
		if ( _FriendsList[i].ContactId == contactId )
		{
			return _FriendsList[i].EntityId;
		}
	}
	return CEntityId::Unknown;
}

//--------------------------------------------------------------
// getIgnoreByContactId(uint32 contactId)
//--------------------------------------------------------------
const NLMISC::CEntityId	&CCharacter::getIgnoreByContactId(uint32 contactId)
{
	for ( uint i = 0 ; i < _IgnoreList.size() ; ++i)
	{
		if ( _IgnoreList[i].ContactId == contactId )
		{
			return _IgnoreList[i].EntityId;
		}
	}
	return CEntityId::Unknown;
}

//--------------------------------------------------------------
//	CCharacter::hasInFriendList()
//--------------------------------------------------------------
bool CCharacter::hasInFriendList(const NLMISC::CEntityId &player) const
{
	for ( uint i = 0 ; i < _FriendsList.size() ; ++i)
	{
		if ( _FriendsList[i].EntityId == player )
		{
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
//	CCharacter::hasInIgnoreList()
//--------------------------------------------------------------
bool CCharacter::hasInIgnoreList(const NLMISC::CEntityId &player) const
{
	for ( uint i = 0 ; i < _IgnoreList.size() ; ++i)
	{
		if ( _IgnoreList[i].EntityId == player )
		{
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
//	for CCharacter::contactListRefChange()
//--------------------------------------------------------------
const NLMISC::CStringConversion<CCharacter::TConctactListAction>::CPair CCharacter::ContactListActionConversionTable[] =
{
	{"AddedAsFriend",CCharacter::AddedAsFriend},
	{"RemovedFromFriends",CCharacter::RemovedFromFriends},
	{"AddToIgnored",CCharacter::AddToIgnored},
	{"RemovedFromIgnored",CCharacter::RemovedFromIgnored},
	{"RemoveFriend",CCharacter::RemoveFriend},
	{"RemoveIgnored",CCharacter::RemoveIgnored},
};
NLMISC::CStringConversion<CCharacter::TConctactListAction>
	CCharacter::ContactListActionConversion(CCharacter::ContactListActionConversionTable,
	sizeof(CCharacter::ContactListActionConversionTable) / sizeof(CCharacter::ContactListActionConversionTable[0]),
	UnknownContactListAction);

CCharacter::TConctactListAction	CCharacter::toContactListAction(const std::string &str)
{
	return ContactListActionConversion.fromString(str);
}
const std::string			&CCharacter::contactListActionToString(TConctactListAction e)
{
	return ContactListActionConversion.toString(e);
}

//--------------------------------------------------------------
//	CCharacter::contactListRefChangeFromCommand()
//--------------------------------------------------------------
void CCharacter::contactListRefChangeFromCommand(const NLMISC::CEntityId &id, const std::string &operation)
{
	// NB: pass operation as string for forward/backward compatibility in player save
	TConctactListAction	actionType= toContactListAction(operation);

	// according to type, do the action
	switch(actionType)
	{
	case AddedAsFriend:
		referencedAsFriendBy(id);
		break;
	case RemovedFromFriends:
		unreferencedAsFriendBy(id);
		break;
	case AddToIgnored:
		addedInIgnoreListBy(id);
		break;
	case RemovedFromIgnored:
		removedFromIgnoreListBy(id);
		break;
	case RemoveFriend:
		removePlayerFromFriendListByEntityId(id);
		break;
	case RemoveIgnored:
		removePlayerFromIgnoreListByEntityId(id);
		break;
	default:
		break;
	};
}

//--------------------------------------------------------------
//	CCharacter::contactListRefChange()
//--------------------------------------------------------------
void CCharacter::contactListRefChange(const NLMISC::CEntityId &id, TConctactListAction actionType)
{
	// Use an offline command for this because this must done even if dest id is disconnected
	std::string	command;
	CModifyContactCommand::makeStringCommande(command, id, contactListActionToString(actionType), _Id);
	// NB: execute the command now if the player is connected, else backup for next connection
	COfflineCharacterCommand::getInstance()->addOfflineCommand( command );

	/* Yoyo: If player not found, old system tried to load it, change its ContactList data and resave it
		AlainS the 11/03/2004 changed it surely because was too slow!!

		I then changed so it works in the following case even if dest is offline (following examples are with ignoreList, but same with friendList) :
		* If A ignore B, A connected, B not connected, and A unignore B, then B->_IgnoredBy will still be updated at next connection
		* If A ignore B, A not connected, and B is destroyed, then A->_IgnoreList will still be updated at next connection
			=> A won't contain invalid contacts

		Please note that the scenario still will fail:
			A adds B to his friend list (B is connected else it wouldn't work, see callers)
			A is saved
			EGS crashes before B is saved (=> B and B->_IsFriendOf list not saved)
			=> when EGS reco, then A, then B, A won't know that B connected is because of the crash...
			Moreover, if B deletes his character, A won't be notifyed => Invalid contact in his list.
		NB: a solution would be to use CCharacter::addEntityToSaveWithMe() (used for exchange), but I'm afraid of
		saving performance of big player graphes....
	*/

}

//--------------------------------------------------------------
//	CCharacter::isIgnoredBy()
//--------------------------------------------------------------
bool CCharacter::isIgnoredBy(const NLMISC::CEntityId &id)
{
	const uint size = (uint)_IsIgnoredBy.size();
	for (uint i = 0; i < size; ++i)
	{
		if (_IsIgnoredBy[i].getShortId() == id.getShortId())
		{
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
//	CCharacter::isFriendOf()
//--------------------------------------------------------------
bool CCharacter::isFriendOf(const NLMISC::CEntityId &id)
{
	const uint size = (uint)_IsFriendOf.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		if (_IsFriendOf[i].getShortId() == id.getShortId())
		{
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
//	CCharacter::referencedAsFriendBy()
//--------------------------------------------------------------
void CCharacter::referencedAsFriendBy( const NLMISC::CEntityId &id)
{
	if (isFriendOf(id))
	{
		return;
	}

	// not found -> add it
	_IsFriendOf.push_back(id);
}

//--------------------------------------------------------------
//	CCharacter::unreferencedAsFriendBy()
//--------------------------------------------------------------
void CCharacter::unreferencedAsFriendBy( const NLMISC::CEntityId &id)
{
	vector<CEntityId>::iterator it;
	for (it = _IsFriendOf.begin() ; it != _IsFriendOf.end() ; ++it)
	{
		if ( (*it).getShortId() == id.getShortId())
		{
			_IsFriendOf.erase(it);
			return;
		}
	}
}

//--------------------------------------------------------------
//	CCharacter::addedInIgnoreListBy()
//--------------------------------------------------------------
void CCharacter::addedInIgnoreListBy( const NLMISC::CEntityId &id)
{
	// check this entity isn't already in the list
	const uint size = (uint)_IsIgnoredBy.size();
	for ( uint i =0 ; i < size ; ++i)
	{
		if ( _IsIgnoredBy[i].getShortId() == id.getShortId())
		{
			return;
		}
	}

	// not found -> add it
	_IsIgnoredBy.push_back(id);
}

//--------------------------------------------------------------
//	CCharacter::removedFromIgnoreListBy()
//--------------------------------------------------------------
void CCharacter::removedFromIgnoreListBy( const NLMISC::CEntityId &id)
{
	vector<CEntityId>::iterator it;
	for (it = _IsIgnoredBy.begin() ; it != _IsIgnoredBy.end() ; ++it)
	{
		if ( (*it).getShortId() == id.getShortId())
		{
			_IsIgnoredBy.erase(it);
			return;
		}
	}
}

//--------------------------------------------------------------
//	CCharacter::setContactOnlineStatus()
//--------------------------------------------------------------
void CCharacter::setContactOnlineStatus( const NLMISC::CEntityId &id, bool online )
{
	// find entity is list
	uint16 index;
	for ( index = 0 ; index < _FriendsList.size() ; ++index)
	{
		if ( _FriendsList[index].EntityId.getShortId() == id.getShortId())
			break;
	}

	if (index == _FriendsList.size())
		return;

	// we not allow to have power character in any contact list of normal character
	if(!haveAnyPrivilege() && PlayerManager.haveAnyPriv(id))
	{
		removePlayerFromFriendListByEntityId(id);
		removePlayerFromIgnoreListByEntityId(id);
		return;
	}


	TCharConnectionState onlineState;
	if (online)
		onlineState = isFriendCharVisualyOnline(id);
	else
		onlineState = ccs_offline;

//	// do not set online for higher grade CSR (except devs)
//	if ( PlayerManager.hasBetterCSRGrade( id, _Id, true ) )
//		return;

	// send update message to client
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;

	if ( ! GenericMsgManager.pushNameToStream( "TEAM:CONTACT_STATUS", bms) )
	{
		nlwarning("<CEntityBase::setContactOnlineStatus> Msg name TEAM:CONTACT_STATUS not found");
		return;
	}

	bms.serial(_FriendsList[index].ContactId);
	bms.serialShortEnum(onlineState);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}

//--------------------------------------------------------------
//	CCharacter::clearFriendList()
//--------------------------------------------------------------
void CCharacter::clearFriendList()
{
	const uint size = (uint)_FriendsList.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		removePlayerFromFriendListByIndex(0);
	}
} // clearFriendList //

//--------------------------------------------------------------
//	CCharacter::clearIgnoreList()
//--------------------------------------------------------------
void CCharacter::clearIgnoreList()
{
	const uint size = (uint)_IgnoreList.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		removePlayerFromIgnoreListByIndex(0);
	}
} // clearIgnoreList //

//--------------------------------------------------------------
//	CCharacter::online()
//--------------------------------------------------------------
void CCharacter::online(bool onlineStatus)
{
	// send contact list init if goind online
	if (onlineStatus)
		sendContactListInit();

	// tell other players i'm now online or offline
	// setContactOnlineStatus may change _IsFriendOf container, so we
	// take of copy before iterating
	vector<CEntityId> friendOf(_IsFriendOf);
	vector<CEntityId>::iterator it;
	for (it = friendOf.begin() ; it != friendOf.end() ; ++it)
	{
		// notify active character matching the id (ignoring the dynamic part because it may have changed)
		CEntityId &id = (*it);
		uint32 playerId = PlayerManager.getPlayerId(id);
		CCharacter *character = PlayerManager.getActiveChar(playerId);
		if (character && character->getId().getShortId() == id.getShortId())
		{
			character->setContactOnlineStatus(_Id, onlineStatus);
		}
	}

	// add / remove from dyn chat
	if (onlineStatus)
	{
		bool res = DynChatEGS.addClient(getEntityRowId());
		if (!res)
		{
			nlwarning("Couldn't add player %s in dynamic chat", getId().toString().c_str());
		}
	}
	else
	{
		bool res = DynChatEGS.removeClient(getEntityRowId());
		if (!res)
		{
			nlwarning("Couldn't remove player %s from dynamic chat", getId().toString().c_str());
		}
	}


	// if the character has a CSR grade, remove from all ignore lists
	if ( onlineStatus && (! _IsIgnoredBy.empty()) && havePriv( ":SGM:GM:VG:SG:G:EM:EG:" ) )
	{
		CMessage msgout( "UNIGNORE_ALL" );
		msgout.serial( _Id );
		msgout.serialCont( _IsIgnoredBy );
		sendMessageViaMirror( "IOS", msgout );
	}


} // online //

//--------------------------------------------------------------
//	CCharacter::sendRemoveContactMessage()
//--------------------------------------------------------------
void CCharacter::sendRemoveContactMessage(uint32 contactId, uint8 listNumber)
{
	// send update message to client
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	CBitMemStream bms;

	if ( ! GenericMsgManager.pushNameToStream( "TEAM:CONTACT_REMOVE", bms) )
	{
		nlwarning("<CEntityBase::sendRemoveContactMessage> Msg name TEAM:CONTACT_REMOVE not found");
		return;
	}

	bms.serial(contactId);
	bms.serial(listNumber);

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
} // sendRemoveContactMessage //

//--------------------------------------------------------------
//	CCharacter::setLastConnectionDate()
//--------------------------------------------------------------
void CCharacter::setLastConnectionDate(uint32 date)
{
	_LastConnectedDate = date;	
}


//--------------------------------------------------------------
//	CCharacter::destroyCharacter()
//--------------------------------------------------------------
void CCharacter::destroyCharacter()
{
	// Update the statistical database
	CStatDB::getInstance()->removePlayer(_Id);
	// remove the character from its guild, if any
	CGuildManager::getInstance()->characterDeleted( *this );

	clearFriendList();
	clearIgnoreList();

	// tell all players referencing this char he doesn't exist anymore
	uint size = (uint)_IsFriendOf.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		contactListRefChange( _IsFriendOf[i], RemoveFriend);
	}

	size = (uint)_IsIgnoredBy.size();
	for (uint i = 0 ; i < size ; ++i)
	{
		contactListRefChange( _IsIgnoredBy[i], RemoveIgnored);
	}

	_IsFriendOf.clear();
	_IsIgnoredBy.clear();

} // destroyCharacter //

//---------------------------------------------------
// CCharacter::setMode
//----------------------------------------------------
void CCharacter::setMode( MBEHAV::EMode mode, bool forceUpdate, bool disengage )
{
	H_AUTO(CEntityBaseSetMode1);

	// the character can't switch to combat mode if he's swimming
	if( _Mode.getValue().Mode == MBEHAV::NORMAL )
	{
		if( isInWater() )
		{
			if( mode == MBEHAV::COMBAT || mode == MBEHAV::COMBAT_FLOAT)
			{
				return;
			}
		}
	}

	if	(	_Mode.getValue().Mode != MBEHAV::DEATH
		||	forceUpdate )
	{
		if( ( _Mode().Mode == MBEHAV::COMBAT || _Mode().Mode == MBEHAV::COMBAT_FLOAT ) && ( mode != MBEHAV::COMBAT && mode != MBEHAV::COMBAT_FLOAT ) )
		{
			_ContextualProperty = _StaticContextualProperty;
		}

		MBEHAV::TMode tmpmode;
		if( mode == MBEHAV::COMBAT_FLOAT )
		{
			tmpmode.setModeAndTheta( mode, _EntityState.Heading );
		}
		else
		{
			tmpmode.setModeAndPos( mode, _EntityRowId );
		}

		_Mode = tmpmode;

		if( mode == MBEHAV::COMBAT || mode == MBEHAV::COMBAT_FLOAT )
		{
			_ContextualProperty.directAccessForStructMembers().talkableTo( false );
			_ContextualProperty.setChanged();
		}
		else if( _Id.getType() == RYZOMID::player )
		{
			if (disengage)
			{
				// disengage from combat if any
				CPhraseManager::getInstance().disengage( _EntityRowId, true );
			}
		}
		//egs_chinfo("<CEntityBase::setMode> %d Set Mode to %d for entity %s", CTickEventHandler::getGameCycle(), mode, _Id.toString().c_str() );
	}
	else
	{
		//egs_chinfo("<CEntityBase::setMode> %d Set Mode refused because entity %s is dead", CTickEventHandler::getGameCycle(), _Id.toString().c_str() );
	}
} // setMode //


//---------------------------------------------------
// CCharacter::setMode
//----------------------------------------------------
void CCharacter::setMode( MBEHAV::TMode mode )
{
	H_AUTO(CEntityBaseSetMode2);

	// the character can't switch to combat mode if he's swimming
	if( _Mode.getValue().Mode == MBEHAV::NORMAL )
	{
		if( isInWater() )
		{
			if( mode.Mode == MBEHAV::COMBAT || mode.Mode == MBEHAV::COMBAT_FLOAT)
			{
				return;
			}
		}
	}

	if	(_Mode.getValue().Mode != MBEHAV::DEATH)
	{
		if( ( _Mode().Mode == MBEHAV::COMBAT || _Mode().Mode == MBEHAV::COMBAT_FLOAT ) && ( mode.Mode != MBEHAV::COMBAT && mode.Mode != MBEHAV::COMBAT_FLOAT ) )
		{
			_ContextualProperty = _StaticContextualProperty;
		}

		_Mode = mode;

		if( mode.Mode == MBEHAV::COMBAT || mode.Mode == MBEHAV::COMBAT_FLOAT )
		{
			_ContextualProperty.directAccessForStructMembers().talkableTo( false );
			_ContextualProperty.setChanged();
		}
	}
} // setMode //

//--------------------------------------------------------------
//	send message of the day to new connected players
//--------------------------------------------------------------
void CCharacter::sendMessageOfTheDay()
{
	if (MessageOfTheDay.toString().empty() || IsRingShard )
		return;

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);
	params[0].Literal.fromUtf8(MessageOfTheDay);

	sendDynamicSystemMessage(_Id, "MOTD", params);
}

//--------------------------------------------------------------
void CCharacter::onConnection()
{
	// Add all handledAIGroups for all missions of the player
	spawnAllHandledAIGroup();

	// update for the unified entity locator
	if (IShardUnifierEvent::getInstance() != NULL)
	{
		IShardUnifierEvent::getInstance()->charConnected(_Id, getLastDisconnectionDate());
	}

	CPVPManager2::getInstance()->playerConnects(this);
}

//--------------------------------------------------------------
void CCharacter::onDisconnection(bool bCrashed)
{
	// indicate to the char that it's going offline
	online(false);
	// remove dyn chats before saving
	CMissionManager::getInstance()->removeAllUserDynChat(this);

	// get all temp inventory content to bag, destroy items can't be put in bag
	sendCloseTempInventoryImpulsion();
	clearTempInventory();

	cancelStaticActionInProgress();
//	removeAllSpells();

	CBuildingManager::getInstance()->removePlayerFromRoom(this);
	// TODO : check if it's not an exploit to disconnect the player from PVP now (not waiting the disconnection time)
	CPVPManager2::getInstance()->playerDisconnects(this);

	// Remove handledAIGroups for all missions of the player
	despawnAllHandledAIGroup();

	// update for the unified entity locator
	if (IShardUnifierEvent::getInstance() != NULL
		&& _Enter)
	{
		IShardUnifierEvent::getInstance()->charDisconnected(_Id);
	}

}

//----------------------------------------------------------------------------
void CCharacter::setAfkState( bool isAfk )
{
	if (_ContextualProperty.directAccessForStructMembers().afk() != isAfk)
	{
		_ContextualProperty.directAccessForStructMembers().afk(isAfk);
		_ContextualProperty.setChanged();

		if( !isInWater() && !isSitting() )
		{
			if (isAfk)
			{
				setMode(MBEHAV::REST);
			}
			else
			{
				setMode(MBEHAV::NORMAL);
			}
		}
	}
}

//----------------------------------------------------------------------------
sint32 CCharacter::getSkillValue(SKILLS::ESkills skill) const
{
	if (skill < 0 || skill >= SKILLS::unknown)
		return 0;

	// quick test if skill is unlocked
	if (_Skills._Skills[skill].Base != 0)
		return _Skills._Skills[skill].Current;

	skill = getFirstUnlockedParentSkill(skill);
	if (skill == SKILLS::unknown)
		return 0;
	else
		return _Skills._Skills[skill].Current;
}
//----------------------------------------------------------------------------
sint32 CCharacter::getSkillBaseValue(SKILLS::ESkills skill) const
{
	if (skill < 0 || skill >= SKILLS::unknown)
		return 0;

	// quick test if skill is unlocked
	if (_Skills._Skills[skill].Base != 0)
		return _Skills._Skills[skill].Base;

	skill = getFirstUnlockedParentSkill(skill);
	if (skill == SKILLS::unknown)
		return 0;
	else
		return _Skills._Skills[skill].Base;
}
//----------------------------------------------------------------------------
sint32 CCharacter::getBestChildSkillValue(SKILLS::ESkills skill) const
{
	// BRIANCODE - changed to work with the skill enum "any". Set value to be equal to
	//				the target value. should be safe because it gets reset regardless.


	sint32 value;
	if( skill < SKILLS::NUM_SKILLS)
		value = _Skills._Skills[skill].MaxLvlReached;
	else if (skill == SKILLS::any)
	{
		value = 0;

		// Lookup the 4 root skills only (faster than checking all)
		value = max(value, _Skills._Skills[SKILLS::SF].MaxLvlReached);
		value = max(value, _Skills._Skills[SKILLS::SM].MaxLvlReached);
		value = max(value, _Skills._Skills[SKILLS::SH].MaxLvlReached);
		value = max(value, _Skills._Skills[SKILLS::SC].MaxLvlReached);
	}
	else
		value = 0;

	return value;
}

//----------------------------------------------------------------------------
sint32 CCharacter::getBestSkillValue(SKILLS::ESkills skill)
{
	skill = getFirstUnlockedParentSkill( skill );
	return getBestChildSkillValue(skill);
}

//----------------------------------------------------------------------------
SKILLS::ESkills CCharacter::getFirstUnlockedParentSkill(SKILLS::ESkills skill) const
{
	// store the original value of the skill in order to have a valid return value in BOMB_IF
	SKILLS::ESkills originalSkill=skill;

	// get pointer on static skills tree definition
	static const NLMISC::CSheetId sheet("skills.skill_tree");
	static const CStaticSkillsTree * skillsTree = CSheets::getSkillsTreeForm( sheet );
	BOMB_IF(skill == SKILLS::unknown,"getFirstUnlockedParentSkill() called for skill: SKILLS::unknown",return skill);
#ifdef NL_DEBUG
	nlassert( skillsTree );
#endif

	std::string skillStr = SKILLS::toString(skill);
	// Found compatible unlocked skill
	while( _Skills.getSkillStruct( skillStr )->Base == 0 )
	{
		skill = skillsTree->SkillsTree[ skill ].ParentSkill;
		skillStr = SKILLS::toString( skill );
		BOMB_IF( skill == SKILLS::unknown,NLMISC::toString("Skill found with value 0 (%s)",SKILLS::toString(originalSkill).c_str()).c_str(), return originalSkill );
	}

	return skill;
}

bool CCharacter::changeCurrentHp(sint32 deltaValue, TDataSetRow responsibleEntity)
{
	H_AUTO(CCharacter_changeCurrentHp);
	// test entity isn't dead already (unless it's a player in coma)
	if	(isDead())
	{
		if ( deltaValue > 0 && _PhysScores._PhysicalScores[SCORES::hit_points].Current > (-_PhysScores._PhysicalScores[SCORES::hit_points].Max) )
		{
			// possible as a player in a 'coma' can still be healed (but not lose hp)
		}
		else
		{
			return false;
		}
	}
	// test player isn't intangible
	if ( _IntangibleEndDate > CTickEventHandler::getGameCycle())
		return false;

	// if a reverse damage effect is on this entity, inverse damage
	if (deltaValue < 0)
	{
		const CSEffect *effect = lookForActiveEffect( EFFECT_FAMILIES::ReverseDamage);
		if (effect)
		{
			deltaValue = -deltaValue;
		}
	}

	_PhysScores._PhysicalScores[SCORES::hit_points].Current = _PhysScores._PhysicalScores[SCORES::hit_points].Current + deltaValue;

	// if entity is mezzed and delta is != 0 unmezz it
	if (_MezzCount && deltaValue != 0)
	{
		unmezz();
	}

	if (_PhysScores._PhysicalScores[SCORES::hit_points].Current <= 0)
	{
		// for god mode
		if (!_GodMode && !_Invulnerable)
		{
			kill(responsibleEntity);
			return true;
		}
		else
		{
			_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Base;
			setHpBar( 1023 );
			return false;
		}
	}
	else
	{
		if( _PhysScores._PhysicalScores[SCORES::hit_points].Max > 0)
			setHpBar( (uint32) ( (1023 * _PhysScores._PhysicalScores[SCORES::hit_points].Current) / _PhysScores._PhysicalScores[SCORES::hit_points].Max) );
		else
			setHpBar(0);
		return false;
	}
}


//--------------------------------------------------------------
//	apply goo damage if character is too close than a goo path
//--------------------------------------------------------------
void CCharacter::applyGooDamage( float gooDistance )
{
	uint32 tempTickForGooDamageRate = NBTickForGooDamageRate;
	if (_CurrentContinent == CONTINENT::NEWBIELAND)
	{
		tempTickForGooDamageRate = NBTickForNewbieGooDamageRate;
	}

	if(	(CTickEventHandler::getGameCycle() - _LastTickSufferGooDamage) > tempTickForGooDamageRate )
	{
		if( gooDistance < MaxDistanceGooDamage )
		{
			// check if player is invulnerable, if so do not apply goo damage
			bool invulnerable = false;
			CSEffect *effect = lookForActiveEffect( EFFECT_FAMILIES::PowerInvulnerability );
			if (effect)
				invulnerable = true;
			else
			{
				effect = lookForActiveEffect( EFFECT_FAMILIES::Invincibility);
				if (effect)
					invulnerable = true;
			}

			if ( _IntangibleEndDate > CTickEventHandler::getGameCycle() )
			{
				invulnerable = true;
			}

			if (!invulnerable)
			{//
				float damageRatio = ( MaxDistanceGooDamage - ( gooDistance - DeathGooDistance ) ) / MaxDistanceGooDamage;

				if (_CurrentContinent == CONTINENT::NEWBIELAND)
				{
					damageRatio = damageRatio * NewbieGooDamageFactor;
				}

				if( damageRatio > 0.0f )
				{
					_LastTickSufferGooDamage = CTickEventHandler::getGameCycle();

					// Apply damage corresponding to distance from goo if not dead
					if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Current > 0 )
					{
						sint32 hpLost = (sint32)(_PhysScores._PhysicalScores[ SCORES::hit_points ].Base * damageRatio * MaxGooDamageRatio);
						if (hpLost < 1) hpLost = 1;
						if( hpLost > _PhysScores._PhysicalScores[ SCORES::hit_points ].Current )
						{
							_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = 0;	
							
							// send message to player for inform is dead by goo or other
							if (_CurrentContinent == CONTINENT::FYROS)
								sendDynamicSystemMessage(_EntityRowId, "KILLED_BY_FIRE");
							else if (_CurrentContinent == CONTINENT::TRYKER)
								sendDynamicSystemMessage(_EntityRowId, "KILLED_BY_STEAM");
							else if (_CurrentContinent == CONTINENT::MATIS)
								sendDynamicSystemMessage(_EntityRowId, "KILLED_BY_POISON");
							else
								sendDynamicSystemMessage(_EntityRowId, "KILLED_BY_GOO");
						}
						else
						{
							_PhysScores._PhysicalScores[ SCORES::hit_points ].Current = _PhysScores._PhysicalScores[ SCORES::hit_points ].Current - hpLost;
							// send message to player for inform is suffer goo damage
							if (_CurrentContinent == CONTINENT::FYROS)
								sendDynamicSystemMessage(_EntityRowId, "SUFFER_FIRE_DAMAGE");
							else if (_CurrentContinent == CONTINENT::TRYKER)
								sendDynamicSystemMessage(_EntityRowId, "SUFFER_STEAM_DAMAGE");
							else if (_CurrentContinent == CONTINENT::MATIS)
								sendDynamicSystemMessage(_EntityRowId, "SUFFER_POISON_DAMAGE");
							else
								sendDynamicSystemMessage(_EntityRowId, "SUFFER_GOO_DAMAGE");
						}
					}
				}

			}
		}
	}
}


//--------------------------------------------------------------
//	addCreationBricks
//--------------------------------------------------------------
void CCharacter::addCreationBricks()
{
	//_CreationPointsRepartition
	uint8 nbPointsCaster = ( (_CreationPointsRepartition>>6)&3 );
	uint8 nbPointsFighter = ( (_CreationPointsRepartition>>4)&3 );
	uint8 nbPointsCrafter = ( (_CreationPointsRepartition>>2)&3 );
	uint8 nbPointsHarvester = ( _CreationPointsRepartition & 3 );

	// add default actions without creating items
	searchCreateRoleSheet( _Race, ROLES::crafter, nbPointsCrafter, true );
	searchCreateRoleSheet( _Race, ROLES::harvester, nbPointsHarvester, true );
	searchCreateRoleSheet( _Race, ROLES::caster, nbPointsCaster, true );
	searchCreateRoleSheet( _Race, ROLES::fighter, nbPointsFighter, true );
}

//
// sendEmote
//
void CCharacter::sendEmote( const NLMISC::CEntityId& id, MBEHAV::EBehaviour behaviour, uint16 emoteTextId, bool checkPrivilege )
{
	const CStaticTextEmotes & emotes = CSheets::getTextEmoteList();
	const CStaticTextEmotes::CTextEmotePhrases * phraseCont = emotes.getPhrase( emoteTextId );
	if ( phraseCont == NULL )
	{
		nlwarning("<EMOTES> client %s sent invalid emote %u",getId().toString().c_str(),emoteTextId );
		return;
	}

	// kxu: TODO? check that behaviour and text id match

	// check emote reserved for FBT
	// it can be hacked because behaviour and text id are both sent by the client:
	// if the client sends a good text id and a cheated behaviour, the player could use the FBT emote without having the right
	// (only sys info would be wrong)
	if (phraseCont->OnlyForFBT && checkPrivilege)
	{
		CPlayer * p = PlayerManager.getPlayer(PlayerManager.getPlayerId( getId() ));
		if (p == NULL || !p->isBetaTester())
			return;
	}

	CEntityId targetId = getTarget();
	if (targetId.getType() > RYZOMID::creature_end)
		targetId = CEntityId::Unknown;

	TDataSetRow targetRow = TheDataset.getDataSetRow( targetId );

	// set behaviour
	setEmote( behaviour );
	setAfkState(false);

	// emote is set for the user. We now have to send the text message
	// get the emote phrases
	const string * crowd = NULL;
	const string * self = NULL;
	TVectorParamCheck params;

	vector<TDataSetRow> excludedFromCrowd;
	excludedFromCrowd.push_back( getEntityRowId() );

	// if the user target himself
	if ( targetId == getId() )
	{
		crowd = &phraseCont->SelfCrowd;
		self = &phraseCont->SelfActor;

		params.resize(1);
		params[0].Type = STRING_MANAGER::entity;
		params[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
	}
	// if there is no valid target
	else if ( targetId == CEntityId::Unknown  )
	{
		crowd = &phraseCont->NoTargetCrowd;
		self = &phraseCont->NoTargetActor;

		params.resize(1);
		params[0].Type = STRING_MANAGER::entity;
		params[0].setEIdAIAlias( getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()) );
	}
	// if there is a target != from the player
	else
	{
		crowd = &phraseCont->TargetCrowd;
		self = &phraseCont->TargetActor;

		params.push_back( STRING_MANAGER::TParam(STRING_MANAGER::entity) );
		params.back().setEIdAIAlias(getId(), CAIAliasTranslator::getInstance()->getAIAlias(getId()));
		params.push_back( STRING_MANAGER::TParam(STRING_MANAGER::entity) );
		params.back().setEIdAIAlias(targetId, CAIAliasTranslator::getInstance()->getAIAlias(targetId));

		// send string to target
		if ( targetId.getType() == RYZOMID::player )
		{
			excludedFromCrowd.push_back( targetRow );

			uint32 txtId = STRING_MANAGER::sendStringToClient( targetRow, phraseCont->TargetTarget, params );
			// send emote message to IOS
			NLNET::CMessage	msgout("EMOTE_PLAYER");
			msgout.serial( const_cast<TDataSetRow&>( getEntityRowId() ) );
			msgout.serial( targetRow );
			msgout.serial(txtId);
			sendMessageViaMirror("IOS", msgout);
		}
	}

	// send string to player
	uint32 txtId = STRING_MANAGER::sendStringToClient(getEntityRowId(), *self, params );
	// send emote message to IOS
	NLNET::CMessage	msgout("EMOTE_PLAYER");
	msgout.serial( const_cast<TDataSetRow&>( getEntityRowId() ) ); // sender
	msgout.serial( const_cast<TDataSetRow&>( getEntityRowId() ) ); // receiver
	msgout.serial(txtId);
	sendMessageViaMirror("IOS", msgout);


	// send crowd emote message to IOS
	NLNET::CMessage	msgout2("EMOTE_CROWD");
	msgout2.serial( const_cast<TDataSetRow&>(getEntityRowId()) );
	msgout2.serial(const_cast<string&>(*crowd));

	uint32 size = (uint32)params.size();
	msgout2.serial(size);
	for ( uint i = 0; i < size; i++ )
	{
		uint8 type8 = (uint8)params[i].Type;
		msgout2.serial(type8);
		params[i].serialParam( false, msgout2, params[i].Type );
	}
	msgout2.serialCont(excludedFromCrowd);
	sendMessageViaMirror("IOS", msgout2);
}


//-----------------------------------------------
//		sendCustomEmote
//-----------------------------------------------
void CCharacter::sendCustomEmote( const NLMISC::CEntityId& id, MBEHAV::EBehaviour behaviour, ucstring& emoteCustomText )
{
	// set behaviour
	if( behaviour != MBEHAV::IDLE )
	{
		setEmote( behaviour );
	}

	string sEmoteCustomText = emoteCustomText.toUtf8();

	if ((behaviour != MBEHAV::IDLE) || (sEmoteCustomText == "none"))
	{
		setAfkState(false);
	}

	if( sEmoteCustomText == "none" )
	{
		return;
	}

	// send emote message to IOS
	NLNET::CMessage	msgout("CUSTOM_EMOTE");
	msgout.serial( const_cast<TDataSetRow&>( getEntityRowId() ) );
	msgout.serial(emoteCustomText);
	sendMessageViaMirror("IOS", msgout);

} // sendCustomEmote //


//--------------------------------------------------------------
//	getActionsSPValue
//--------------------------------------------------------------
uint32 CCharacter::getActionsSPValue() const
{
	uint32 totalSp = 0;

	set<CSheetId>::const_iterator it = _KnownBricks.begin();

	set<CSheetId> alreadyParsedBricks;

	for ( ; it != _KnownBricks.end() ; ++it)
	{
		const CStaticBrick *brickForm = CSheets::getSBrickForm(*it);
		if (brickForm && alreadyParsedBricks.find(*it) == alreadyParsedBricks.end() )
		{
			alreadyParsedBricks.insert(*it);
			totalSp += brickForm->SkillPointPrice;
//			nldebug("Known Brick %s, SP %d", (*it).toString().c_str(), brickForm->SkillPointPrice);
		}
	}

	return totalSp;
}

//--------------------------------------------------------------
//	getStartActionsSPValue
//--------------------------------------------------------------
uint32 CCharacter::getStartActionsSPValue() const
{
	//_CreationPointsRepartition
	uint8 nbPointsCaster = ( (_CreationPointsRepartition>>6)&3 );
	uint8 nbPointsFighter = ( (_CreationPointsRepartition>>4)&3 );
	uint8 nbPointsCrafter = ( (_CreationPointsRepartition>>2)&3 );
	uint8 nbPointsHarvester = ( _CreationPointsRepartition & 3 );

	// get phrases
	vector<CSheetId> phrases;
	vector<CSheetId> temp;
	getRoleStartActions( _Race, ROLES::caster, nbPointsCaster, phrases);

	temp.clear();
	getRoleStartActions( _Race, ROLES::fighter, nbPointsCaster, temp);
	phrases.insert(phrases.end(), temp.begin(), temp.end());

	temp.clear();
	getRoleStartActions( _Race, ROLES::crafter, nbPointsCrafter, temp);
	phrases.insert(phrases.end(), temp.begin(), temp.end());

	temp.clear();
	getRoleStartActions( _Race, ROLES::harvester, nbPointsHarvester, temp);
	phrases.insert(phrases.end(), temp.begin(), temp.end());

	set<CSheetId> alreadyParsedBricks;

	uint32 totalSp = 0;

	for ( uint i = 0 ; i < phrases.size() ; ++i )
	{
		const CStaticRolemasterPhrase *staticPhrase = CSheets::getSRolemasterPhrase(phrases[i]);
		if (staticPhrase)
		{
			// add all sp costs
			for (uint i = 0; i < staticPhrase->Bricks.size() ; ++i)
			{
				const CStaticBrick *brickForm = CSheets::getSBrickForm(staticPhrase->Bricks[i]);
				if (brickForm && alreadyParsedBricks.find(staticPhrase->Bricks[i]) == alreadyParsedBricks.end() )
				{
					alreadyParsedBricks.insert(staticPhrase->Bricks[i]);
					totalSp += brickForm->SkillPointPrice;
//					egs_chinfo("Brick %s, SP %d", staticPhrase->Bricks[i].toString().c_str(), brickForm->SkillPointPrice);
				}
			}
		}
	}


	return totalSp;
}


//--------------------------------------------------------------
//	getStartActionsSPValue
//--------------------------------------------------------------
void CCharacter::getRoleStartActions(EGSPD::CPeople::TPeople people, ROLES::ERole role, uint8 nbPoints, vector<CSheetId> &phrases) const
{
	if (nbPoints == 0)
		return;

	// get role
	const CStaticRole * staticRole = 0;
	for( map< CSheetId, CStaticRole >::const_iterator itr = CSheets::getRoleContainer().begin(); itr != CSheets::getRoleContainer().end(); ++itr )
	{
		if( (*itr).second.Race == people )
		{
			if( (*itr).second.Role == role )
			{
				staticRole = &(*itr).second;
//				egs_chinfo("<CCharacter::getRoleStartActions> Selected sheet: %s", (*itr).first.toString().c_str());
				break;
			}
		}
	}

	if (!staticRole)
		return;

	vector<CStaticRole::TMemorizedSentence> sentences;

	// get phrases
	switch( nbPoints )
	{
	case 1:
		sentences = staticRole->MemorizedSentences1;
		break;
	case 2:
		sentences.insert(sentences.end(), staticRole->MemorizedSentences2.begin(), staticRole->MemorizedSentences2.end() );
		break;
	case 3:
		sentences.insert(sentences.end(), staticRole->MemorizedSentences3.begin(), staticRole->MemorizedSentences3.end() );
		break;
	default:;
	}

	phrases.clear();
	for (uint i = 0 ; i < sentences.size() ; ++i)
	{
		phrases.push_back(sentences[i].sentence);
	}
}

//--------------------------------------------------------------
//	getStartActionsSPValue
//--------------------------------------------------------------
uint32 CCharacter::getTotalEarnedSP() const
{
	static const NLMISC::CSheetId sheet("skills.skill_tree");
	static const CStaticSkillsTree * skillsTree = CSheets::getSkillsTreeForm( sheet );

	uint32 sp = 10 * skillsTree->getPlayerSkillPointsUnderSkill(&_Skills, SKILLS::SC);
	sp += 10 * skillsTree->getPlayerSkillPointsUnderSkill(&_Skills, SKILLS::SF);
	sp += 10 * skillsTree->getPlayerSkillPointsUnderSkill(&_Skills, SKILLS::SM);
	sp += 10 * skillsTree->getPlayerSkillPointsUnderSkill(&_Skills, SKILLS::SH);

	return sp;
}

//--------------------------------------------------------------
// setCurrentAction
//--------------------------------------------------------------
void CCharacter::setCurrentAction(CLIENT_ACTION_TYPE::TClientActionType actionType,NLMISC::TGameCycle actionEndCycle )
{
	sint64 tmp = (sint64) CTickEventHandler::getGameCycle();
//	_PropertyDatabase.setProp( "USER:ACT_TSTART", tmp );
	CBankAccessor_PLR::getUSER().setACT_TSTART(_PropertyDatabase, uint32(tmp) );
	tmp = (sint64)actionEndCycle;
//	_PropertyDatabase.setProp( "USER:ACT_TEND", tmp );
	CBankAccessor_PLR::getUSER().setACT_TEND(_PropertyDatabase, uint32(tmp) );
	tmp = (sint64)actionType;
//	_PropertyDatabase.setProp( "USER:ACT_TYPE", tmp );
	CBankAccessor_PLR::getUSER().setACT_TYPE(_PropertyDatabase, checkedCast<uint8>(tmp) );
	tmp = (sint64)_ActionCounter;
//	_PropertyDatabase.setProp( "USER:ACT_NUMBER", tmp );
	CBankAccessor_PLR::getUSER().setACT_NUMBER(_PropertyDatabase, checkedCast<uint8>(tmp) );
}

//--------------------------------------------------------------
// clearCurrentAction
//--------------------------------------------------------------
void CCharacter::clearCurrentAction( )
{
//	_PropertyDatabase.setProp( "USER:ACT_TSTART", 0 );
	CBankAccessor_PLR::getUSER().setACT_TSTART(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "USER:ACT_TEND", 0 );
	CBankAccessor_PLR::getUSER().setACT_TEND(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "USER:ACT_TYPE", 0 );
	CBankAccessor_PLR::getUSER().setACT_TYPE(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", 0 );
	CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "EXECUTE_PHRASE:SHEET", 0 );
	CBankAccessor_PLR::getEXECUTE_PHRASE().setSHEET(_PropertyDatabase, CSheetId::Unknown );
}

//--------------------------------------------------------------
void CCharacter::setWelcomeMissionDesc(TAIAlias missionAlias, TAIAlias botAlias)
{
	if (missionAlias == CAIAliasTranslator::Invalid)
	{
		nlwarning("invalid mission alias");
		return;
	}

	if (botAlias == CAIAliasTranslator::Invalid)
	{
		nlwarning("invalid bot alias");
		return;
	}

	_WelcomeMissionDesc.MissionAlias = missionAlias;
	_WelcomeMissionDesc.BotAlias = botAlias;
	nlassert(_WelcomeMissionDesc.isValid());
}

//--------------------------------------------------------------
void CCharacter::assignWelcomeMission()
{
	if (_WelcomeMissionDesc.isValid())
	{
		std::list<CMissionEvent*> eventList;
		CMissionManager::getInstance()->instanciateMission(this, _WelcomeMissionDesc.MissionAlias, _WelcomeMissionDesc.BotAlias, eventList);
		processMissionEventList(eventList, true, CAIAliasTranslator::Invalid);

		_WelcomeMissionDesc.clear();
		nlassert(!_WelcomeMissionDesc.isValid());
	}
}

//--------------------------------------------------------------
//--------------------------------------------------------------
uint CCharacter::getMissionsCount()
{
	uint ret = 0;
	for( std::map<TAIAlias, CMission*>::iterator it = getMissionsBegin() = _Missions->getMissionsBegin(); it != getMissionsEnd(); ++it )
		++ret;
	return ret;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
map<TAIAlias, CMission*>::iterator CCharacter::getMissionsBegin()
{
	return _Missions->getMissionsBegin();
}

//--------------------------------------------------------------
//--------------------------------------------------------------
map<TAIAlias, CMission*>::iterator CCharacter::getMissionsEnd()
{
	return _Missions->getMissionsEnd();
}

//--------------------------------------------------------------
//--------------------------------------------------------------
const CMission* CCharacter::getMission( TAIAlias missionId ) const
{
	return _Missions->getMissions( missionId );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
CMission* CCharacter::getMission( TAIAlias missionId )
{
	return _Missions->getMissions( missionId );
}

//--------------------------------------------------------------
//	addSuccessfulMissions
//--------------------------------------------------------------
void CCharacter::addSuccessfulMissions( const CMissionTemplate & templ)
{
	TMissionHistory &mh = _MissionHistories[templ.Alias];
	mh.Successfull = true;

	// stat log the event
//	Bsi.append( StatPath, NLMISC::toString("[MIADDSUC] %s %s",
//		_Id.toString().c_str(),
//		templ.getMissionName().c_str()) );
//
//	EgsStat.displayNL("[MIADDSUC] %s %s",
//	_Id.toString().c_str(),
//	templ.getMissionName().c_str());
//
//	EGSPD::missionLog("ADDSUC", _Id, templ.getMissionName());
}
bool CCharacter::isMissionSuccessfull(const CMissionTemplate & templ)
{
	std::map< TAIAlias, TMissionHistory >::iterator it(_MissionHistories.find(templ.Alias));
	if (it != _MissionHistories.end())
		return it->second.Successfull;
	return false;
}

/// check the last date of trying for a mission (0 if never tryied)
NLMISC::TGameCycle CCharacter::getMissionLastSuccess(const CMissionTemplate & templ)
{
	std::map< TAIAlias, TMissionHistory >::iterator it(_MissionHistories.find(templ.Alias));
	if (it != _MissionHistories.end())
	{
		TGameCycle lastSuccessDate =  it->second.LastSuccessDate;

		if (lastSuccessDate > CTickEventHandler::getGameCycle())
			return 0;

		return it->second.LastSuccessDate;
	}
	return 0;

}

//--------------------------------------------------------------
//	sendCyclePhraseExecAck
//--------------------------------------------------------------
void CCharacter::sendPhraseExecAck(bool cyclic, uint8 counter, bool execOk)
{
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );

	CBitMemStream bms;
	std::string msgName;
	if (cyclic)
		msgName = "PHRASE:EXEC_CYCLIC_ACK";
	else
		msgName = "PHRASE:EXEC_NEXT_ACK";

	if ( ! GenericMsgManager.pushNameToStream( msgName, bms) )
	{
		nlwarning("<CCharacter::sendPhraseExecAck> Msg name %s not found", msgName.c_str());
		return;
	}
	bms.serial( execOk );
	bms.serial( counter );

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}

//-----------------------------------------------------------------------------
bool CCharacter::checkCharacterStillValide( const char * msgError)
{
	bool mirrorBoolCrushed = false;

	for( sint32 i = 0; i < CHARACTERISTICS::NUM_CHARACTERISTICS; ++i )
	{
		if( _PhysCharacs._PhysicalCharacteristics[i].Current.testFlagInMirror() != 0 )
		{
			nlwarning("BUG: %s NASTY MEMORY BUG. current characteristic %s is in mirror !", _Id.toString().c_str(), CHARACTERISTICS::toString(i).c_str() );
			mirrorBoolCrushed = true;
		}
		else if( _PhysCharacs._PhysicalCharacteristics[i].Current.location().dataSet() != 0 )
		{
			nlwarning("BUG: %s NASTY MEMORY BUG. current characteristic %s have a dataset !", _Id.toString().c_str(), CHARACTERISTICS::toString(i).c_str() );
			mirrorBoolCrushed = true;
		}

		if( _PhysCharacs._PhysicalCharacteristics[i].Max.testFlagInMirror() != 0 )
		{
			nlwarning("BUG: %s NASTY MEMORY BUG. max characteristic %s is in mirror !", _Id.toString().c_str(), CHARACTERISTICS::toString(i).c_str() );
			mirrorBoolCrushed = true;
		}
		else if( _PhysCharacs._PhysicalCharacteristics[i].Max.location().dataSet() != 0 )
		{
			nlwarning("BUG: %s NASTY MEMORY BUG. max characteristic %s have a dataset !", _Id.toString().c_str(), CHARACTERISTICS::toString(i).c_str() );
			mirrorBoolCrushed = true;
		}
	}

	for( sint32 i = 0; i < SCORES::NUM_SCORES; ++i )
	{
		if( i == 0 )
		{
			if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Current.testFlagInMirror() != 1 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. current score %s ", _Id.toString().c_str(), SCORES::toString(SCORES::hit_points).c_str() );
				mirrorBoolCrushed = true;
			}
			else if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Current.location().dataSet() == 0 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. current score %s don't have a dataset !", _Id.toString().c_str(), SCORES::toString(SCORES::hit_points).c_str() );
				mirrorBoolCrushed = true;
			}

			if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Max.testFlagInMirror() != 1 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. max score %s ", _Id.toString().c_str(), SCORES::toString(SCORES::hit_points).c_str() );
				mirrorBoolCrushed = true;
			}
			else if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Max.location().dataSet() == 0 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. max score %s don't have a dataset !", _Id.toString().c_str(), SCORES::toString(SCORES::hit_points).c_str() );
				mirrorBoolCrushed = true;
			}
		}
		else
		{
			if( _PhysScores._PhysicalScores[i].Current.testFlagInMirror() != 0 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. current score %s is in mirror !", _Id.toString().c_str(), SCORES::toString(i).c_str() );
				mirrorBoolCrushed = true;
			}
			else if( _PhysScores._PhysicalScores[i].Current.location().dataSet() != 0 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. current score %s have a dataset !", _Id.toString().c_str(), SCORES::toString(i).c_str() );
				mirrorBoolCrushed = true;
			}

			if( _PhysScores._PhysicalScores[i].Max.testFlagInMirror() != 0 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. max score %s is in mirror !", _Id.toString().c_str(), SCORES::toString(i).c_str() );
				mirrorBoolCrushed = true;
			}
			else if( _PhysScores._PhysicalScores[i].Max.location().dataSet() != 0 )
			{
				nlwarning("BUG: %s NASTY MEMORY BUG. max score %s have a dataset !", _Id.toString().c_str(), SCORES::toString(i).c_str() );
				mirrorBoolCrushed = true;
			}
		}
	}

	if( ! TheDataset.isAccessible( _EntityRowId ) || ((int)_Enter) != 1 || mirrorBoolCrushed )
	{
		if( msgError != 0 )
			nlwarning("BUG: Tick %d Character %s : %s", CTickEventHandler::getGameCycle(), _Id.toString().c_str(),  msgError);
		else
			nlwarning("BUG: Tick %d Character %s : %s", CTickEventHandler::getGameCycle(), _Id.toString().c_str(), "checkCharacterStillValide with no error message");

		PlayerManager.addPlayerMustBeDisconnected( PlayerManager.getPlayerId( _Id ) );
		return false;
	}
	return true;
}


//-----------------------------------------------------------------------------
void CCharacter::checkScoresValues( SCORES::TScores score, CHARACTERISTICS::TCharacteristics charac )
{
	// check energy qty
	sint32 base = (_PhysCharacs._PhysicalCharacteristics[ charac ].Base + PhysicalCharacteristicsBaseValue) * PhysicalCharacteristicsFactor + _ScorePermanentModifiers[ score ];
	if(	_PhysScores._PhysicalScores[ score ].Base != base )
	{
		nlwarning("BADCHECK For player %s, for %s, player should have %u and he has %u !", _Id.toString().c_str(), SCORES::toString(score).c_str(), base, _PhysScores._PhysicalScores[ score ].Base);
//vl		_PhysScores._PhysicalScores[ score ].Base = base;
	}

	// check regen
	float baseRegenerateRepos = _PhysCharacs._PhysicalCharacteristics[ charac + 1 ].Current / RegenDivisor;
	float baseRegenerateAction = _PhysScores._PhysicalScores[ score ].BaseRegenerateRepos / RegenReposFactor;
	baseRegenerateRepos += RegenOffset;
	baseRegenerateAction += RegenOffset;
	if(	fabs((_PhysScores._PhysicalScores[ score ].BaseRegenerateRepos * 100.0f) - (100.0f * baseRegenerateRepos)) > 0.001)
	{
		nlwarning("BADCHECK For player %s, for %s regen, player should have %f and he has %f !", _Id.toString().c_str(), SCORES::toString(score).c_str(), baseRegenerateRepos, _PhysScores._PhysicalScores[ score ].BaseRegenerateRepos);
//vl		_PhysScores._PhysicalScores[ score ].BaseRegenerateRepos = baseRegenerateRepos;
//vl		_PhysScores._PhysicalScores[ score ].BaseRegenerateAction = baseRegenerateAction;
	}
}



//-----------------------------------------------------------------------------
void CCharacter::checkCharacAndScoresValues()
{
	// do not check values for GM and DEV
	CPlayer * player = PlayerManager.getPlayer( PlayerManager.getPlayerId(_Id) );
	if (player != NULL)
	{
		if ( player->havePriv(NoValueCheckingPriv) )
			return;
	}

	uint8 maxPhraseLvlValue[CHARACTERISTICS::NUM_CHARACTERISTICS];

	for ( sint charac = 0 ; charac < (sint)CHARACTERISTICS::NUM_CHARACTERISTICS ; ++charac)
		maxPhraseLvlValue[charac] = 0;

	{
	H_AUTO(GetHigestLevelUpgradePhrase);
	// get all charac highest level upgrade phrase
	std::string phraseStr, code, txt;
	uint8 lvl;
	for (set<CSheetId>::const_iterator it = _BoughtPhrases.begin() ;  it != _BoughtPhrases.end() ; ++it)
	{
		// test bricks is a charac upgrade
		phraseStr = (*it).toString();
		if (phraseStr.find("abpp") != string::npos)
		{
			// phrase = abppXZZ.sphrase with X = characteristic code and ZZ = brick level (CharacteristicBrickStep*ZZ)
			code = phraseStr.substr(4,1); //string( text[4] );
			txt = phraseStr.substr(5,2);
			NLMISC::fromString(txt, lvl);

			CHARACTERISTICS::TCharacteristics charac = CHARACTERISTICS::getCharacteristicFromCode(code);
			if (charac < CHARACTERISTICS::NUM_CHARACTERISTICS)
			{
				if (maxPhraseLvlValue[charac] < lvl)
				{
					maxPhraseLvlValue[charac] = lvl;
				}
			}
		}
	}
	}

	{
	H_AUTO(CheckCharacteristics);
	// check caracs
	sint32 tvalue;
	for ( sint charac = 0 ; charac < (sint)CHARACTERISTICS::NUM_CHARACTERISTICS ; ++charac)
	{
		// compute theoretical value
		tvalue = _StartingCharacteristicValues[charac] + maxPhraseLvlValue[charac] * (sint32)CharacteristicBrickStep;
		//TODO tvalue = StartCharacteristicsValue + maxPhraseLvlValue[charac] * (sint32)CharacteristicBrickStep;

		// compare
		if (_PhysCharacs._PhysicalCharacteristics[charac].Base != tvalue)
		{
			nlwarning("BADCHECK For player %s, for charac %s, player should have %u and he has %u !", _Id.toString().c_str(), CHARACTERISTICS::toString(charac).c_str(), tvalue,_PhysCharacs._PhysicalCharacteristics[charac].Base);

//vl			_PhysCharacs._PhysicalCharacteristics[charac].Base = tvalue;
//vl			_PhysCharacs._PhysicalCharacteristics[charac].Current = tvalue;
		}
	}
	}


	{
		H_AUTO(CheckScores);
		// Check Scores
		checkScoresValues( SCORES::hit_points,	CHARACTERISTICS::constitution );
		checkScoresValues( SCORES::sap,			CHARACTERISTICS::intelligence );
		checkScoresValues( SCORES::stamina,		CHARACTERISTICS::strength );
		checkScoresValues( SCORES::focus,		CHARACTERISTICS::dexterity );
	}
}

//-----------------------------------------------------------------------------
void CCharacter::entersWater() const
{
	// tranfert damage for player
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->transferDamageOnFictitiousCreature(_EntityRowId, _TeamId);
	// clear Xp for player
	PROGRESSIONPVE::CCharacterProgressionPVE::getInstance()->clearAllXpForPlayer(_EntityRowId, _TeamId, true);
	// disengage player if engaged in combat
	CPhraseManager::getInstance().disengage(_EntityRowId, true);
}

//-----------------------------------------------------------------------------
void CCharacter::consumeItem( INVENTORIES::TInventory inventory, uint32 slot )
{
	TLogContext_Item_Consume logContext(_Id);
	//if player is already consumming something, cancel action
	if (_ConsumedItemInventory != INVENTORIES::UNDEFINED)
	{
		// player already consuming an item, cancel action
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		return;
	}

	// get item in inventory
	// check if inventory exist
	if ( inventory > INVENTORIES::NUM_INVENTORY )
	{
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		nlwarning("<CCharacter::consumeItem> Got inventory Id %u but there is only %u inventories", inventory, INVENTORIES::NUM_INVENTORY );
		return;
	}

	CInventoryPtr childSrc = _Inventory[ inventory ];
	// check if slots is in range of inventories
	if( childSrc->getSlotCount() <= slot )
	{
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		nlwarning("<CCharacter::consumeItem> Specified slot %u but there is only slot in inventory %hu", slot,  childSrc->getSlotCount(), inventory );
		return;
	}
	CGameItemPtr item = childSrc->getItem( slot );
	if ( item == NULL || item->getNonLockedStackSize() == 0 )
	{
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		return;
	}
	const CStaticItem * form = item->getStaticForm();
	if ( !form )
	{
		nlwarning("<consumeItem>%s item in slot %u has a NULL form. NbSlot = %u",_Id.toString().c_str(), slot, childSrc->getSlotCount());
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		return;
	}
	if (form->ConsumableItem == NULL)
	{
		if( form->Family == ITEMFAMILY::COMMAND_TICKET && form->CommandTicket != NULL )
		{
			launchCommandTicket(form);
			// Quality is used as Nb run counter here, if 0 mean infinite run
			if( item->quality() != 0 )
			{
				item->quality(item->quality() - 1);
				if(item->quality() == 0)
				{
					_ConsumedItemSlot = slot;
					_ConsumedItemInventory = inventory;
					lockItem( inventory, slot, 1);
					destroyConsumedItem();
				}
			}
			return;
		}
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		return;
	}

	// is effect disabled ?
	NLMISC::TGameCycle endDate;
	if ( !canUseConsumableFamily(form->ConsumableItem->Family, endDate))
	{
		uint16 secondes = uint16((endDate - CTickEventHandler::getGameCycle()) * CTickEventHandler::getGameTimeStep());
		const uint16 minutes = secondes / 60;
		secondes = secondes%60;

		// send error message
		SM_STATIC_PARAMS_3(params, STRING_MANAGER::item, STRING_MANAGER::integer, STRING_MANAGER::integer);
		params[0].SheetId = form->SheetId;
		params[1].Int = minutes;
		params[2].Int = secondes;
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_OVERDOSE_TIMER", params);

		return;
	}

	_ConsumedItemSlot = slot;
	_ConsumedItemInventory = inventory;

	// execute phrase
	static CSheetId consumeBrick("bapa05.sbrick");
	vector<CSheetId> bricks;
	bricks.push_back(consumeBrick);
	if ( !CPhraseManager::getInstance().executePhrase(_EntityRowId, _EntityRowId, bricks) )
	{
		// send error message
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CANCEL");
		resetConsumedItem();
		return;
	}

	// send chat message
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
		params[0].SheetId = form->SheetId;
		CCharacter::sendDynamicSystemMessage(_Id,"CONSUMABLE_CONSUME_ITEM_BEGIN", params);
	}

	// lock item and keep item slot and inventory
	lockItem( inventory, slot, 1);
}

//-----------------------------------------------------------------------------
void CCharacter::destroyConsumedItem()
{
	if (_ConsumedItemSlot == -1)
	{
		nlwarning("Player %s, No item currently consummed", _Id.toString().c_str());
		return;
	}

	unLockItem(_ConsumedItemInventory, (uint32)_ConsumedItemSlot, 1);
	destroyItem(_ConsumedItemInventory, (uint32)_ConsumedItemSlot,1,false);
	resetConsumedItem();
}

//-----------------------------------------------------------------------------
void CCharacter::launchCommandTicket(const CStaticItem * form)
{
	CSString command = form->CommandTicket->Command;
	string priv = form->CommandTicket->Priviledge;
	CSString cmdName = command.firstWord(true);

	if(command.empty())
		return;

	if(priv.empty() == false )
	{
		if( havePriv(priv) == false )
		{
			nlwarning ("Character %s doesn't have privilege needed by ticket to execute the command '%s' in ticket '%s' ", _Id.toString().c_str(), cmdName.c_str(), form->SheetId.toString().c_str());
			chatToPlayer (_Id, "You don't have privilege needed by ticket to execute this command");
			return;
		}
	}

	// find if the command is available
	const CAdminCommand * cmd = findAdminCommand(cmdName);
	if (!cmd)
	{
		nlwarning ("Character %s tried to execute a no valid command '%s' with command ticket '%s'", _Id.toString().c_str(), cmdName.c_str(), form->SheetId.toString().c_str());
		chatToPlayer (_Id, "Your command ticket have an invalide command can't be executed");
		return;
	}

	if (!havePriv(cmd->Priv))
	{
		nlwarning ("Character %s doesn't have privilege to execute the command '%s' in ticket '%s' ", _Id.toString().c_str(), cmdName.c_str(), form->SheetId.toString().c_str());
		chatToPlayer (_Id, "You don't have privilege to execute this command");
		return;
	}

	if (!cmd->ForwardToservice.empty())
	{
		// we need to forward the command to another service
		if (IClientCommandForwader::getInstance())
		{
			IClientCommandForwader::getInstance()->sendCommand(cmd->ForwardToservice, cmdName, _Id, false, CEntityId::Unknown, command);
		}
	}
	else
	{
		// execute locally
		// create the command line
		string res = cmdName;

		// add the eid of the player or target if necessary
		if (cmd->AddEId)
		{
			res += " ";
			res += _Id.toString();
		}

		res += command;

		nlinfo ("Character %s will execute command '%s' from command ticket '%s'", _Id
			.toString().c_str(), res.c_str(), form->SheetId.toString().c_str());
		NLMISC::ICommand::execute(res, *InfoLog);
	}
}

//-----------------------------------------------------------------------------
void CCharacter::executeMemorizedPhrase(uint8 memSet, uint8 index, bool cyclic, bool enchant )
{
	if (cyclic)
		++_CycleCounter;
	else
		++_NextCounter;

	_MemorizedPhrases.executePhrase(memSet, index, this, _Target, cyclic,enchant);
}

//-----------------------------------------------------------------------------
void CCharacter::forgetPhrase(uint8 memSet, uint8 i)
{
	_MemorizedPhrases.forget(memSet,i);
}

//-----------------------------------------------------------------------------
sint32 CCharacter::getSkillEquivalentDodgeValue(SKILLS::ESkills skill) const
{
	const string &str = SKILLS::toString(skill);

	const string testStr = str.substr(0,2);
	if (testStr == "SF")
	{
		return getSkillBaseValue(skill);
	}
	else if (testStr == "SM")
	{
		return sint32( getSkillBaseValue(skill) * DodgeFactorForMagicSkills );
	}
	else if (testStr == "SC")
	{
		return 0;
	}
	else if (testStr == "SH")
	{
		return sint32(getSkillBaseValue(skill) * DodgeFactorForForageSkills);
	}

	return 0;
}

//-----------------------------------------------------------------------------
void CCharacter::logAndClearTempInventory()
{
	CInventoryPtr tempInv = _Inventory[ INVENTORIES::temporary ];
	for (uint i = 0 ; i < tempInv->getSlotCount(); ++i )
	{
		CGameItemPtr itemPtr = tempInv->getItem(i);
		if (itemPtr != NULL )
		{
			CItemCraftParameters tmp;
			const CItemCraftParameters *craftParams = &tmp;
			if( itemPtr->getCraftParameters() != 0 )
			{
				craftParams = itemPtr->getCraftParameters();
			}

			// log item stats
			egs_chinfo("TEMP_INVENTORY_BUG : player %s (%s), Item sheet = %s", _Id.toString().c_str(), _Name.toString().c_str(), itemPtr->getSheetId().toString().c_str() );
			egs_chinfo("TEMP_INVENTORY_BUG : Quality = %u, HP = %d/%d", itemPtr->quality(), itemPtr->durability(), itemPtr->maxDurability());
			egs_chinfo("TEMP_INVENTORY_BUG : Craft Params ");
			egs_chinfo("TEMP_INVENTORY_BUG : Durability = %f Weight = %f SapLoad = %f ", craftParams->Durability, craftParams->Weight, craftParams->SapLoad);

			// weapons factor
			egs_chinfo("TEMP_INVENTORY_BUG : Dmg = %f Speed = %f Range = %f", craftParams->Dmg, craftParams->Speed, craftParams->Range);
			egs_chinfo("TEMP_INVENTORY_BUG : DodgeModifier = %f ParryModifier = %f ", craftParams->DodgeModifier, craftParams->ParryModifier, craftParams->AdversaryDodgeModifier);
			egs_chinfo("TEMP_INVENTORY_BUG : AdversaryDodgeModifier = %f AdversaryParryModifier %f", craftParams->AdversaryDodgeModifier, craftParams->AdversaryParryModifier);

			// magic focus factor
			egs_chinfo("TEMP_INVENTORY_BUG : ElementalCastingTimeFactor = %f ElementalPowerFactor = %f OffensiveAfflictionCastingTimeFactor = %f", craftParams->ElementalCastingTimeFactor, craftParams->ElementalPowerFactor, craftParams->OffensiveAfflictionCastingTimeFactor);
			egs_chinfo("TEMP_INVENTORY_BUG : OffensiveAfflictionPowerFactor = %f HealCastingTimeFactor = %f HealPowerFactor = %f", craftParams->OffensiveAfflictionPowerFactor, craftParams->HealCastingTimeFactor, craftParams->HealPowerFactor);
			egs_chinfo("TEMP_INVENTORY_BUG : DefensiveAfflictionCastingTimeFactor = %f DefensiveAfflictionPowerFactor = %f ProtectionFactor = %f", craftParams->DefensiveAfflictionCastingTimeFactor, craftParams->DefensiveAfflictionPowerFactor, craftParams->ProtectionFactor);
			egs_chinfo("TEMP_INVENTORY_BUG : MaxSlashingProtection = %f MaxBluntProtection = %f MaxPiercingProtection = %f", craftParams->MaxSlashingProtection, craftParams->MaxBluntProtection, craftParams->MaxPiercingProtection);

			// jewel protection
			egs_chinfo("TEMP_INVENTORY_BUG : Color = %u, Protection : %s", craftParams->Color, BACK_COMPAT::OLD_PROTECTION_TYPE::toString(craftParams->Protection).c_str() );

			// armor and jewel buff
			egs_chinfo("TEMP_INVENTORY_BUG : HpBuff = %u, SapBuff = %u, StaBuff = %u, FocusBuff = %u", craftParams->HpBuff, craftParams->SapBuff, craftParams->StaBuff, craftParams->FocusBuff);

			// destroy item
			tempInv->removeItem(i);
			itemPtr.deleteItem();
		}
	}
}



//------------------------------------------------------------------------------
void CCharacter::checkSkillTreeForLockedSkill()
{
	// set root skills of skill tree to 1
	CSheetId sheet("skills.skill_tree");
	const CStaticSkillsTree * SkillsTree = CSheets::getSkillsTreeForm( sheet );

	const vector< CStaticSkillsTree::SSkillData >& skillsTree = SkillsTree->SkillsTree;

	for( uint32 i = 0; i < skillsTree.size(); ++i )
	{
		if( skillsTree[ i ].ParentSkill == SKILLS::unknown )
		{
			// skill is a root skill, insure is unlocked
			if( _Skills._Skills[ skillsTree[ i ].Skill ].Base == 0 )
			{
				_Skills._Skills[ skillsTree[ i ].Skill ].Base = 1;
				_Skills._Skills[ skillsTree[ i ].Skill ].Current = 1;
			}
		}
		// skill is not a root skill, if they are locked
		else if( _Skills._Skills[ skillsTree[ i ].Skill ].Base == 0 )
		{
			// check if one childrens of parents skill are unlocked ( other skills with same parent )
			for( uint32 child = 0; child < skillsTree[ skillsTree[ i ].ParentSkill ].ChildSkills.size(); ++child )
			{
				if( _Skills._Skills[ skillsTree[ skillsTree[ i ].ParentSkill ].ChildSkills[ child ] ].Base != 0 )
				{
					// one child skill of same parent are unlocked, unlock current skill
					_Skills._Skills[ skillsTree[ i ].Skill ].Base = skillsTree[ skillsTree[ i ].ParentSkill ].MaxSkillValue + 1;
					_Skills._Skills[ skillsTree[ i ].Skill ].Current = skillsTree[ skillsTree[ i ].ParentSkill ].MaxSkillValue + 1;
					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
bool CCharacter::setHairColor(uint32 colorValue)
{
	if ( colorValue == _HairColor )
	{
		CCharacter::sendDynamicSystemMessage(_Id,"EGS_COSMETIC_SAME_HAIR_COLOR");
		return false;
	}
	// if the user has a naked head, update visal property
	if ( _HairColor == _VisualPropertyA.directAccessForStructMembers().PropertySubData.HatColor )
	{
		_VisualPropertyA.directAccessForStructMembers().PropertySubData.HatColor = (uint8)colorValue;
		_VisualPropertyA.setChanged();
	}
	_HairColor = (uint8)colorValue;
//	_PropertyDatabase.setProp( "USER:HAIR_COLOR",_HairColor );
	CBankAccessor_PLR::getUSER().setHAIR_COLOR(_PropertyDatabase, _HairColor );
	return true;
}

//------------------------------------------------------------------------------
bool CCharacter::setHair(uint32 hairValue)
{
	if ( hairValue == _HairType )
	{
		CCharacter::sendDynamicSystemMessage(_Id,"EGS_COSMETIC_SAME_HAIR");
		return false;
	}
	// if the user has a naked head, update visal property
	if ( _HairType == _VisualPropertyA.directAccessForStructMembers().PropertySubData.HatModel )
	{
		_VisualPropertyA.directAccessForStructMembers().PropertySubData.HatModel = (uint8)hairValue;
		_VisualPropertyA.setChanged();
	}
	_HairType = (uint8)hairValue;
//	_PropertyDatabase.setProp( "USER:HAIR_TYPE",_HairType );
	CBankAccessor_PLR::getUSER().setHAIR_TYPE(_PropertyDatabase, _HairType );
	return true;
}

//------------------------------------------------------------------------------
bool CCharacter::setTatoo(uint32 tatooValue)
{
	if ( tatooValue == _VisualPropertyC.directAccessForStructMembers().PropertySubData.Tattoo )
	{
		CCharacter::sendDynamicSystemMessage(_Id,"EGS_COSMETIC_SAME_TATOO");
		return false;
	}
	_VisualPropertyC.directAccessForStructMembers().PropertySubData.Tattoo = (uint8)tatooValue;
	_VisualPropertyC.setChanged();
	return true;
}

//--------------------------------------------------------------
//	player enters in a PVP zone, send appropriate client message
//--------------------------------------------------------------
void CCharacter::enterPVPZone(uint32 pvpZoneType) const
{
	switch ( PVP_ZONE_TYPE::TPVPZoneType(pvpZoneType) )
	{
	case PVP_ZONE_TYPE::FreeZone:
		CCharacter::sendDynamicSystemMessage( _EntityRowId, "PVP_FREE_ZONE_ENTER" );
		break;

	case PVP_ZONE_TYPE::VersusZone:
		CCharacter::sendDynamicSystemMessage( _EntityRowId, "PVP_VERSUS_ZONE_ENTER" );
		break;

	case PVP_ZONE_TYPE::GuildZone:
		CCharacter::sendDynamicSystemMessage( _EntityRowId, "PVP_GUILD_ZONE_ENTER" );
		break;
	}
}

//--------------------------------------------------------------
//	character enter in versus pvp zone, player must choose a clan
//--------------------------------------------------------------
void CCharacter::openPVPVersusDialog() const
{
	PVP_CLAN::TPVPClan clan1, clan2;
	if( getPVPInterface().isValid() && getPVPInterface().getPvpClan( clan1, clan2 ) )
	{
		EGSPD::CPeople::TPeople clan1People, clan2People;

		switch( clan1 )
		{
			case PVP_CLAN::Kami:
				clan1People = EGSPD::CPeople::Kami;
				break;
			case PVP_CLAN::Karavan:
				clan1People = EGSPD::CPeople::Karavan;
				break;
			case PVP_CLAN::Fyros:
				clan1People = EGSPD::CPeople::Fyros;
				break;
			case PVP_CLAN::Matis:
				clan1People = EGSPD::CPeople::Matis;
				break;
			case PVP_CLAN::Tryker:
				clan1People = EGSPD::CPeople::Tryker;
				break;
			case PVP_CLAN::Zorai:
				clan1People = EGSPD::CPeople::Zorai;
				break;
			default:
				nlwarning("unknown pvp versus clan1 for character %s", getId().toString().c_str());
				return;
		}

		switch( clan2 )
		{
		case PVP_CLAN::Kami:
			clan2People = EGSPD::CPeople::Kami;
			break;
		case PVP_CLAN::Karavan:
			clan2People = EGSPD::CPeople::Karavan;
			break;
		case PVP_CLAN::Fyros:
			clan2People = EGSPD::CPeople::Fyros;
			break;
		case PVP_CLAN::Matis:
			clan2People = EGSPD::CPeople::Matis;
			break;
		case PVP_CLAN::Tryker:
			clan2People = EGSPD::CPeople::Tryker;
			break;
		case PVP_CLAN::Zorai:
			clan2People = EGSPD::CPeople::Zorai;
			break;
		default:
			nlwarning("unknown pvp versus clan2 for character %s", getId().toString().c_str());
			return;
		}

		CMessage msgout( "IMPULSION_ID" );
		CEntityId eid(_Id);
		msgout.serial( eid );
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "PVP_VERSUS:CHOOSE_CLAN", bms) )
		{
			nlwarning("<CEntityBase::tickUpdate> Msg name PVP_VERSUS:CHOOSE_CLAN not found");
			return;
		}
		bms.serialEnum( clan1People );
		bms.serialEnum( clan2People );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
	}
}

//-----------------------------------------------------------------------------
bool CCharacter::getPVPFlag( bool updatePVPModeInMirror ) const
{
	bool returnVal;
	if( _PVPFlag == true )
	{
		returnVal = (_PVPFlagLastTimeChange + TimeForSetPVPFlag) < CTickEventHandler::getGameCycle();
	}
	else
	{
		returnVal = (_PVPFlagLastTimeChange + TimeForPVPFlagOff) >= CTickEventHandler::getGameCycle();
	}

	if( updatePVPModeInMirror )
	{
		if (TheDataset.isAccessible(_EntityRowId))
		{
			CPVPManager2::getInstance()->setPVPModeInMirror(this);

			if (returnVal)
			{
				// Update PvP clan in mirror for faction PvP
				updatePVPClanVP();
				updateGuildFlag();
			}
		}
	}
	return returnVal;
}

//-----------------------------------------------------------------------------
void CCharacter::setPVPFlag( bool pvpFlag )
{
	bool actualPvpFlag = getPVPFlag();

	if( pvpFlag == true )
	{


// NEW PVP : Check fames
		bool havePvpFame = false;
		for (uint8 fameIdx = 0; fameIdx < 7; fameIdx++)
		{
			sint32 fame = CFameInterface::getInstance().getFameIndexed(_Id, fameIdx);
			if ((fame >= PVPFameRequired*6000) || (fame <= -PVPFameRequired*6000))
				havePvpFame = true;
		}


//-		if( (_DeclaredCult == PVP_CLAN::Neutral || _DeclaredCult == PVP_CLAN::None ) && (_DeclaredCiv == PVP_CLAN::Neutral || _DeclaredCiv == PVP_CLAN::None ) )

		if (!havePvpFame)
		{
			// character can set it's tag pvp on if he have no allegiance.
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			sendDynamicSystemMessage(_EntityRowId, "PVP_TAG_PVP_NEED_ALLEGIANCE");
//			_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:COUNTER", ++_PvPDatabaseCounter );
			CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setCOUNTER(_PropertyDatabase, uint8(++_PvPDatabaseCounter));
			return;
		}

// OLD PVP
/*		if( CPVPManager2::getInstance()->isFactionInWar( _DeclaredCult ) == false &&
			CPVPManager2::getInstance()->isFactionInWar( _DeclaredCiv ) == false)
		{
			// character can set it's tag pvp on if none of his clan is in war
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			sendDynamicSystemMessage(_EntityRowId, "PVP_TAG_PVP_NEED_ALLEGIANCE");
//			_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:COUNTER", ++_PvPDatabaseCounter );
			CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setCOUNTER(_PropertyDatabase, uint8(++_PvPDatabaseCounter));
			return;
		}
		*/	
	}


	// if player changed it's decision before timer if finished
	if( pvpFlag != _PVPFlag && actualPvpFlag == pvpFlag )
	{
		if( _PVPFlag )
		{
			_PVPFlagLastTimeChange = CTickEventHandler::getGameCycle() - TimeForSetPVPFlag;
			_PVPFlagTimeSettedOn = CTickEventHandler::getGameCycle() - TimeForSetPVPFlag;
		}
		else
			_PVPFlagLastTimeChange = CTickEventHandler::getGameCycle() - TimeForResetPVPFlag;
		_PVPFlag = pvpFlag;

		// database update
		setPVPFlagDatabase();
	}
	else if( pvpFlag == _PVPFlag)
	{
		if( actualPvpFlag != _PVPFlag )
		{
			// pvp flag already set, wait until timer delay for become effective
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = _PVPFlag;
			sendDynamicSystemMessage(_EntityRowId, "PVP_FLAG_WAIT_FOR_TIMER", params);
		}
		else
		{
			// this pvp mode is already set
			SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
			params[0].Int = _PVPFlag;
			sendDynamicSystemMessage(_EntityRowId, "PVP_FLAG_ALREADY_IN_THIS_MODE", params);
		}
//		_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:COUNTER", ++_PvPDatabaseCounter );
		CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setCOUNTER(_PropertyDatabase, uint8(++_PvPDatabaseCounter) );
	}
	else
	{
		//  set the new pvp flag and time last change for apply timer delay before this change become effective
		if( pvpFlag == false )
		{
			if (_PVPFlagTimeSettedOn > CTickEventHandler::getGameCycle())
				_PVPFlagTimeSettedOn = CTickEventHandler::getGameCycle() - TimeForResetPVPFlag;

			if( _PVPFlagTimeSettedOn + TimeForResetPVPFlag > CTickEventHandler::getGameCycle() )
			{
				// we need wait a minimal of time before reset your pvp tag
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::time, STRING_MANAGER::time);
				params[0].Time = TimeForResetPVPFlag;
				params[1].Time = _PVPFlagTimeSettedOn + TimeForResetPVPFlag - CTickEventHandler::getGameCycle();
				sendDynamicSystemMessage(_EntityRowId, "PVP_FLAG_RESET_TAG_MINIMAL_TIME", params);
//				_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:COUNTER", ++_PvPDatabaseCounter );
				CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setCOUNTER(_PropertyDatabase, uint8(++_PvPDatabaseCounter) );
				return;
			}

			if( getPvPRecentActionFlag() )
			{
				// reset pvp tag is forbided when recent pvp action are made
				SM_STATIC_PARAMS_2(params, STRING_MANAGER::time, STRING_MANAGER::time);
				params[0].Time = PVPActionTimer;
				params[1].Time = _PVPRecentActionTime + PVPActionTimer - CTickEventHandler::getGameCycle();
				sendDynamicSystemMessage(_EntityRowId, "PVP_FLAG_RESET_NEED_NO_RECENT_PVP_ACTION", params);
//				_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:COUNTER", ++_PvPDatabaseCounter );
				CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setCOUNTER(_PropertyDatabase, uint8(++_PvPDatabaseCounter) );
				return;
			}
		}
		_PVPFlag = pvpFlag;
		_PVPFlagLastTimeChange = CTickEventHandler::getGameCycle();
		if( pvpFlag )
			_PVPFlagTimeSettedOn = CTickEventHandler::getGameCycle() + TimeForSetPVPFlag;

		// database update
		setPVPFlagDatabase();

		_HaveToUpdatePVPMode = true;
	}
}


//-----------------------------------------------------------------------------
void CCharacter::resetPVPTimers()
{
	_PVPFlag = false;
	_PVPFlagLastTimeChange = 0;
	_PVPRecentActionTime = 0;
	_PVPFlagTimeSettedOn = 0;
	_HaveToUpdatePVPMode = true;
	setPVPFlagDatabase();
}


//-----------------------------------------------------------------------------
void CCharacter::pvpActionMade()
{
	if( getPVPFlag() )
	{
		if( _PVPFlag == false )
		{
			if( (_PVPFlagLastTimeChange + TimeForPVPFlagOff) > CTickEventHandler::getGameCycle() )
			{
				_PVPFlagLastTimeChange = CTickEventHandler::getGameCycle() - TimeForResetPVPFlag;
				_PVPFlag = true;
				// database update
				setPVPFlagDatabase();
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CCharacter::setPVPFlagDatabase()
{
	// Fix for when negative ticks were saved
	if (   (_PVPRecentActionTime > CTickEventHandler::getGameCycle())
		|| (_PVPFlagLastTimeChange > CTickEventHandler::getGameCycle())
		|| (_PVPFlagTimeSettedOn > CTickEventHandler::getGameCycle() + TimeForSetPVPFlag)  )
	{
		_PVPRecentActionTime   = 0;
		_PVPFlagLastTimeChange = 0;
		_PVPFlagTimeSettedOn   = 0;
		_PVPSafeLastTimeChange = 0;
		_PVPFlag = false;
	}

	uint32 activationTime;
	if( _PVPFlag == true )
		activationTime = _PVPFlagLastTimeChange + TimeForSetPVPFlag;
	else
		activationTime = _PVPFlagLastTimeChange + TimeForPVPFlagOff;

//	_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:TAG_PVP", _PVPFlag );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setTAG_PVP(_PropertyDatabase, _PVPFlag);
//	_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:ACTIVATION_TIME", activationTime );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setACTIVATION_TIME(_PropertyDatabase, activationTime );
//	_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:COUNTER", ++_PvPDatabaseCounter );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setCOUNTER(_PropertyDatabase, uint8(++_PvPDatabaseCounter) );

	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setFLAG_PVP_TIME_LEFT(_PropertyDatabase, _PVPRecentActionTime + PVPActionTimer );
}

//-----------------------------------------------------------------------------
void CCharacter::setPVPRecentActionFlag(CCharacter *target)
{
	if (!getPvPRecentActionFlag())
	{
		_PVPFlagAlly = 0;
		_PVPFlagEnemy = 0;
	}

	_PVPRecentActionTime = CTickEventHandler::getGameCycle();

//	_PropertyDatabase.setProp("CHARACTER_INFO:PVP_FACTION_TAG:FLAG_PVP_TIME_LEFT", _PVPRecentActionTime + PVPActionTimer );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_FACTION_TAG().setFLAG_PVP_TIME_LEFT(_PropertyDatabase, _PVPRecentActionTime + PVPActionTimer );

	CPVPManager2::getInstance()->setPVPModeInMirror(this);
}

//-----------------------------------------------------------------------------
std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> CCharacter::getAllegiance() const
{
	return std::make_pair( _DeclaredCult, _DeclaredCiv );
}

//-----------------------------------------------------------------------------
bool CCharacter::setDeclaredCult(PVP_CLAN::TPVPClan newClan)
{
	// Check to make sure fame is a Cult, and we meet minimum fame to declare.
	if (newClan == PVP_CLAN::None || newClan == PVP_CLAN::Neutral
		|| ((newClan >= PVP_CLAN::BeginCults && newClan <= PVP_CLAN::EndCults)
		    && (CFameInterface::getInstance().getFameIndexed(_Id,PVP_CLAN::getFactionIndex(newClan)) >= FameMinToDeclare) ))
	{
		PVP_CLAN::TPVPClan oldCult = _DeclaredCult;
		_DeclaredCult = newClan;
		// Check guild allegiances.
		if (canBelongToGuild(_GuildId))
		{
			// No problems, let the change happen.
			// Make sure fame values are properly capped.
			CFameManager::getInstance().enforceFameCaps(this->getId(), this->getAllegiance());

			// set tribe fame threshold and clamp fame if necessary
			CFameManager::getInstance().setAndEnforceTribeFameCap(this->getId(), this->getAllegiance());

			// handle with faction channel
			CPVPManager2::getInstance()->updateFactionChannel(this);

			// write new allegiance in database
//			_PropertyDatabase.setProp("FAME:CULT_ALLEGIANCE", newClan);
			CBankAccessor_PLR::getFAME().setCULT_ALLEGIANCE(_PropertyDatabase, newClan);

			_LastCultPointWriteDB = ~0;

//			if( _DeclaredCult == PVP_CLAN::Neutral || _DeclaredCult == PVP_CLAN::None )
//			{
				if( _PVPFlag || getPvPRecentActionFlag() )
				{
					// if no cult declared and civ is not in war we remove pvp tag
//					if( CPVPManager2::getInstance()->isFactionInWar( _DeclaredCiv ) == false )
//					{
						_PVPFlag = false;
						_PVPFlagLastTimeChange = 0;
						_PVPFlagTimeSettedOn = 0;
						_PVPRecentActionTime = 0;
						setPVPFlagDatabase();
						_HaveToUpdatePVPMode = true;
//					}
				}
//			}

			// Update PvP clan in mirror for faction PvP
			updatePVPClanVP();

			// update ring database
			IShardUnifierEvent::getInstance()->onUpdateCharAllegiance(_Id, _DeclaredCult, _DeclaredCiv);

			return true;
		}
		// Revert the change.
		_DeclaredCult = oldCult;
		// Send a message.
		PHRASE_UTILITIES::sendDynamicSystemMessage(this->getEntityRowId(),"FAME_CULT_NOT_GOOD_FOR_GUILD");
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CCharacter::setDeclaredCiv(PVP_CLAN::TPVPClan newClan)
{
	// Check to make sure fame is within range.
	if (newClan == PVP_CLAN::None || newClan == PVP_CLAN::Neutral
		|| ((newClan >= PVP_CLAN::BeginCivs && newClan <= PVP_CLAN::EndCivs)
		    && (CFameInterface::getInstance().getFameIndexed(_Id,PVP_CLAN::getFactionIndex(newClan)) >= FameMinToDeclare) ))
	{
		PVP_CLAN::TPVPClan oldCiv = _DeclaredCiv;
		_DeclaredCiv = newClan;
		// Check guild allegiances.
		if (canBelongToGuild(_GuildId))
		{
			// No problems, let the change happen.
			// Make sure fame values are properly capped.
			CFameManager::getInstance().enforceFameCaps(this->getId(), this->getAllegiance());

			// set tribe fame threshold and clamp fame if necessary
			CFameManager::getInstance().setAndEnforceTribeFameCap(this->getId(), this->getAllegiance());

			// handle with faction channel
			CPVPManager2::getInstance()->updateFactionChannel(this);

			// write new allegiance in database
//			_PropertyDatabase.setProp("FAME:CIV_ALLEGIANCE", newClan);
			CBankAccessor_PLR::getFAME().setCIV_ALLEGIANCE(_PropertyDatabase, newClan);

			_LastCivPointWriteDB = ~0;

//			if( _DeclaredCiv == PVP_CLAN::Neutral || _DeclaredCiv == PVP_CLAN::None )
//			{
				if( _PVPFlag || getPvPRecentActionFlag() )
				{
					// if no civ declared and cult is not in war we remove pvp tag
//					if( CPVPManager2::getInstance()->isFactionInWar( _DeclaredCult ) == false )
//					{
						_PVPFlag = false;
						_PVPFlagLastTimeChange = 0;
						_PVPFlagTimeSettedOn = 0;
						_PVPRecentActionTime = 0;
						setPVPFlagDatabase();
						_HaveToUpdatePVPMode = true;
//					}
				}
//			}

			// Update PvP clan in mirror for faction PvP
			updatePVPClanVP();

			// update ring database
			IShardUnifierEvent::getInstance()->onUpdateCharAllegiance(_Id, _DeclaredCult, _DeclaredCiv);

			return true;
		}
		// Revert the change.
		_DeclaredCiv = oldCiv;
		// Send a message.
		PHRASE_UTILITIES::sendDynamicSystemMessage(this->getEntityRowId(),"FAME_CIV_NOT_GOOD_FOR_GUILD");
	}
	return false;
}

//-----------------------------------------------------------------------------
void CCharacter::setAllegianceFromIndeterminedStatus(PVP_CLAN::TPVPClan allegiance)
{
	switch(allegiance)
	{
	case PVP_CLAN::Kami:
	case PVP_CLAN::Karavan:
		if(_DeclaredCult == PVP_CLAN::None)
		{
			setDeclaredCult(PVP_CLAN::Neutral);
			return;
		}
		nlwarning("Only character with indefinined status in there cult allegiance can do that for become neutral, check the client code !");
		return;
	case PVP_CLAN::Fyros:
	case PVP_CLAN::Matis:
	case PVP_CLAN::Tryker:
	case PVP_CLAN::Zorai:
		if(_DeclaredCiv == PVP_CLAN::None)
		{
			setDeclaredCiv(PVP_CLAN::Neutral);
			return;
		}
		nlwarning("Only character with indefinined status in there civ allegiance can do that for become neutral, check the client code !");
		return;

	default:
		nlwarning("Received wrong allegiance '%s' from client '%s'", PVP_CLAN::toString(allegiance).c_str(), _Id.toString().c_str());
		return;
	}
}

//-----------------------------------------------------------------------------
bool CCharacter::verifyClanAllegiance(PVP_CLAN::TPVPClan theClan, sint32 newFameValue)
{
	// If the value is too low...
	if (newFameValue < FameMinToRemain)
	{
		// Check to see if cult or civilization matches.
		if (theClan == _DeclaredCult)
		{
			// Send message.
			SM_STATIC_PARAMS_1(theCult, STRING_MANAGER::faction);
			theCult[0].Enum = PVP_CLAN::getFactionIndex(_DeclaredCult);
			PHRASE_UTILITIES::sendDynamicSystemMessage(this->getEntityRowId(),"FAME_TOO_LOW_FOR_ALLEGIANCE", theCult);
			// Boot 'em!
			setDeclaredCult(PVP_CLAN::Neutral);

			return false;
		}
		else
		{
			if (theClan == _DeclaredCiv)
			{
				// Send message.
				SM_STATIC_PARAMS_1(theCiv, STRING_MANAGER::faction);
				theCiv[0].Enum = PVP_CLAN::getFactionIndex(_DeclaredCiv);
				PHRASE_UTILITIES::sendDynamicSystemMessage(this->getEntityRowId(),"FAME_TOO_LOW_FOR_ALLEGIANCE", theCiv);
				// Boot 'em!
				setDeclaredCiv(PVP_CLAN::Neutral);

				return false;
			}
		}
	}
	else
	{
		// If the fame is below the warning level, but not below the minimum...
		if (newFameValue < FameWarningLevel)
		{
			// Check to see if cult or civilization matches.
			if (theClan == _DeclaredCult)
			{
				// Send message.
				SM_STATIC_PARAMS_1(theCult, STRING_MANAGER::faction);
				theCult[0].Enum = PVP_CLAN::getFactionIndex(_DeclaredCult);
				PHRASE_UTILITIES::sendDynamicSystemMessage(this->getEntityRowId(),"FAME_LOW_WARNING_FOR_ALLIANCE", theCult);
			}
			else
			{
				if (theClan == _DeclaredCiv)
				{
					// Send message.
					SM_STATIC_PARAMS_1(theCiv, STRING_MANAGER::faction);
					theCiv[0].Enum = PVP_CLAN::getFactionIndex(_DeclaredCiv);
					PHRASE_UTILITIES::sendDynamicSystemMessage(this->getEntityRowId(),"FAME_LOW_WARNING_FOR_ALLIANCE", theCiv);
				}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void CCharacter::setPvPSafeZoneActive()
{
	_PvPSafeZoneActive = true;

	if( getPVPFlag() )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		sendDynamicSystemMessage(_EntityRowId, "PVP_SAFE_ZONE");
	}
}

//-----------------------------------------------------------------------------
void CCharacter::setOutpostAlias( uint32 id )
{
	// remove player from old outpost player list and reset interface
	if( _OutpostAlias != 0 )
	{
		if( getPVPInterface().isValid() )
		{
			getPVPInterface().leavePVP( IPVP::LeavePVPZone );
		}
	}

	bool hasRightToBanish = false;
	// add player in new outpost player list
	if( id != 0 )
	{
		// init pvp interface
		IPVPZone * zone = CPVPManager::getInstance()->getPVPZone( id );
		if ( !zone )
		{
			nlwarning("<CCharacter::setOutpostAlias> player %s invalid current PVP zone with alias %s", getId().toString().c_str(), CPrimitivesParser::aliasToString(id).c_str() );
			return;
		}
		getPVPInterface().init( zone );

		// add player to zone
		zone->addPlayer( this );

		sendDynamicSystemMessage( _EntityRowId, "OUTPOST_IN_CONFLICT");

		// joining an outpost conflict reset leaving timer
		stopOutpostLeavingTimer();

		// see if he has right to banish
		bool guildIsAttacker;
		if( isGuildInConflictWithOutpost(id,guildIsAttacker) )
		{
			CGuildMember * member = CGuildManager::getInstance()->getGuildFromId(_GuildId)->getMemberFromEId(_Id);
			if (member != NULL)
			{
				if (member->getGrade() < EGSPD::CGuildGrade::Member)
					hasRightToBanish = true;
			}
		}
	}
	else
	{
		if( _OutpostAlias != 0 )
		{
			sendDynamicSystemMessage( _Id, "OUTPOST_NO_MORE_IN_CONFLICT");
		}
//		_PropertyDatabase.setProp("CHARACTER_INFO:PVP_OUTPOST:ROUND_LVL_CUR", 0 );
		CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setROUND_LVL_CUR(_PropertyDatabase, 0 );
//		_PropertyDatabase.setProp("CHARACTER_INFO:PVP_OUTPOST:ROUND_END_DATE", 0 );
		CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setROUND_END_DATE(_PropertyDatabase, 0 );
	}

	CEntityBase::setOutpostAlias( id );
//	_PropertyDatabase.setProp( "CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP", (id != 0) );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setFLAG_PVP(_PropertyDatabase, (id != 0) );
//	_PropertyDatabase.setProp( "CHARACTER_INFO:PVP_OUTPOST:RIGHT_TO_BANISH", hasRightToBanish );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setRIGHT_TO_BANISH(_PropertyDatabase, hasRightToBanish );

	CPVPManager2::getInstance()->setPVPModeInMirror(this);
	updatePVPClanVP();	
}

//-----------------------------------------------------------------------------
void CCharacter::startOutpostLeavingTimer()
{
	if( _OutpostLeavingTime==0 || (_OutpostLeavingTime < CTickEventHandler::getGameCycle()) )
	{
		_OutpostLeavingTime = CTickEventHandler::getGameCycle() + OutpostLeavePeriod*10;
//		_PropertyDatabase.setProp( "CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP_TIME_END", _OutpostLeavingTime );
		CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setFLAG_PVP_TIME_END(_PropertyDatabase, _OutpostLeavingTime );
	}

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
	params[0].Int = (_OutpostLeavingTime - CTickEventHandler::getGameCycle())/10;
	sendDynamicSystemMessage( _Id, "PVP_ZONE_LEAVE_TIME", params );
}

//-----------------------------------------------------------------------------
void CCharacter::refreshOutpostLeavingTimer()
{
	if( _OutpostLeavingTime != 0 )
	{
		_OutpostLeavingTime = CTickEventHandler::getGameCycle() + OutpostLeavePeriod*10;
//		_PropertyDatabase.setProp( "CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP_TIME_END", _OutpostLeavingTime );
		CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setFLAG_PVP_TIME_END(_PropertyDatabase, _OutpostLeavingTime );
	}
}

//-----------------------------------------------------------------------------
void CCharacter::stopOutpostLeavingTimer()
{
	_OutpostLeavingTime = 0;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:PVP_OUTPOST:FLAG_PVP_TIME_END", 0 );
	CBankAccessor_PLR::getCHARACTER_INFO().getPVP_OUTPOST().setFLAG_PVP_TIME_END(_PropertyDatabase, 0 );
}

//-----------------------------------------------------------------------------
bool CCharacter::outpostLeavingDurationElapsed()
{
	if( _OutpostLeavingTime != 0 )
	{
		return (_OutpostLeavingTime < CTickEventHandler::getGameCycle());
	}

	return false;
}


//-----------------------------------------------------------------------------
bool CCharacter::isGuildInConflictWithOutpost( TAIAlias outpostId, bool &guildIsAttacker )
{
	// Default: the player guild( if any) is not the attacker
	guildIsAttacker= false;
	CGuild * guild = CGuildManager::getInstance()->getGuildFromId(_GuildId);
	if (guild != NULL)
	{
		if( _GuildId == COutpostManager::getInstance().getOutpostFromAlias( outpostId )->getAttackerGuild() )
			guildIsAttacker= true;
		if( _GuildId == COutpostManager::getInstance().getOutpostFromAlias( outpostId )->getOwnerGuild() ||
			guildIsAttacker )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------
void CCharacter::outpostOpenChooseSideDialog( TAIAlias outpostId )
{
	if( _OutpostAlias == outpostId )
	{
		return;
	}

	CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( outpostId );
	if( !outpost )
	{
		nlwarning("<CCharacter::outpostOpenChooseSideDialog> can't find outpost %s",CPrimitivesParser::aliasToString(outpostId).c_str());
		return;
	}

	bool playerGuildIsAttacker= false;
	bool playerGuildInConflict = isGuildInConflictWithOutpost( outpostId, playerGuildIsAttacker );

	if(outpost->isPlayerBanishedForAttack(_Id))
	{
		if(playerGuildIsAttacker)
		{
			sendDynamicSystemMessage( _EntityRowId, "OUTPOST_CANT_PARTICIPATE");
			return;
		}
		else
		{
			sendDynamicSystemMessage( _EntityRowId, "OUTPOST_CANT_PARTICIPATE_ATTACK");
			playerGuildInConflict = true; //Forcing defense mode
		}
	}

	if(outpost->isPlayerBanishedForDefense(_Id))
	{
		if(playerGuildInConflict && !playerGuildIsAttacker)
		{
			sendDynamicSystemMessage( _EntityRowId, "OUTPOST_CANT_PARTICIPATE");
			return;
		}
		else
		{
			sendDynamicSystemMessage( _EntityRowId, "OUTPOST_CANT_PARTICIPATE_DEFENSE");
			playerGuildIsAttacker = true; // Forcing attack mode
			playerGuildInConflict = true; //
		}
	}

	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );

	CBitMemStream bms;
	if ( ! GenericMsgManager.pushNameToStream( "OUTPOST:CHOOSE_SIDE", bms) )
	{
		nlwarning("<CCharacter::outpostOpenChooseSideDialog> Msg name OUTPOST:CHOOSE_SIDE not found");
		return;
	}

	bms.serial( playerGuildInConflict );
	bms.serial( playerGuildIsAttacker );

	// always Display the guild names in the interface
	{
		uint32 ownerGuildNameId = 0;
		uint32 attackerGuildNameId = 0;

		// retrieve the owner guild
		EGSPD::TGuildId ownerGuildId = outpost->getOwnerGuild();
		// If the outpost is owned by the marauders, just send 0
		if(ownerGuildId == 0)
		{
			ownerGuildNameId= 0;
		}
		else
		{
			CGuild * ownerGuild = CGuildManager::getInstance()->getGuildFromId( ownerGuildId );
			if( ownerGuild )
			{
				ownerGuildNameId = ownerGuild->getNameId();
			}
			else
			{
				nlwarning("<CCharacter::outpostOpenChooseSideDialog> can't find owner guild of outpost %s",CPrimitivesParser::aliasToString(outpostId).c_str());
				return;
			}
		}

		// retrieve the attacker guild
		EGSPD::TGuildId attackerGuildId = COutpostManager::getInstance().getOutpostFromAlias( outpostId )->getAttackerGuild();
		CGuild * attackerGuild = CGuildManager::getInstance()->getGuildFromId( attackerGuildId );
		if( attackerGuild )
		{
			attackerGuildNameId = attackerGuild->getNameId();
		}
		else
		{
			nlwarning("<CCharacter::outpostOpenChooseSideDialog> can't find attacker guild of outpost %s",CPrimitivesParser::aliasToString(outpostId).c_str());
			return;
		}
		bms.serial( ownerGuildNameId );
		bms.serial( attackerGuildNameId );

		// max time to answer (in ticks)
		uint32	timerDecl= OutpostJoinPvpTimer * 10;
		bms.serial(timerDecl);
	}

	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}


//-----------------------------------------------------------------------------
void CCharacter::outpostSideChosen( bool neutral, OUTPOSTENUMS::TPVPSide side )
{
	// test if he can choose his side
	if( _OutpostIdBeforeUserValidation == 0 )
	{
		nlwarning("<CCharacter::outpostSideChosen> player %s choosed an outpost side but didn't enter the outpost zone before",getId().toString().c_str());
		return;
	}

	if( !neutral )
	{
		// validate outpost alias
		setOutpostAlias( _OutpostIdBeforeUserValidation );

		CGuild * guild = CGuildManager::getInstance()->getGuildFromId(_GuildId);
		if (guild != NULL)
		{
			// he his guild owns the outpost he can only help his guild
			if( _GuildId == COutpostManager::getInstance().getOutpostFromAlias( _OutpostIdBeforeUserValidation )->getOwnerGuild() )
			{
				setOutpostSide( OUTPOSTENUMS::OutpostOwner );
				_OutpostIdBeforeUserValidation = 0;
				return;
			}
			// he his guild attacks the outpost he can only help his guild
			if( _GuildId == COutpostManager::getInstance().getOutpostFromAlias( _OutpostIdBeforeUserValidation )->getAttackerGuild() )
			{
				setOutpostSide( OUTPOSTENUMS::OutpostAttacker );
				_OutpostIdBeforeUserValidation = 0;
				return;
			}
		}
		// check : if outpost belongs to a tribe the choice can only be attacker
		CSmartPtr<COutpost> outpost = COutpostManager::getInstance().getOutpostFromAlias( _OutpostIdBeforeUserValidation );
		if( outpost )
		{
			if (outpost->isBelongingToAGuild() == false)
			{
				if( side != OUTPOSTENUMS::OutpostAttacker )
				{
					nlwarning("<CCharacter::outpostSideChosen> Outpost %s belongs to a tribe but entity %s wants to help tribe, hack ?",CPrimitivesParser::aliasToString(_OutpostIdBeforeUserValidation).c_str(),_Id.toString().c_str());
					side = OUTPOSTENUMS::OutpostAttacker;
				}
			}
		}
		// his guild doesn't participate in outpost conflict so he can choose the side he wants
		setOutpostSide( side );
	}

	_OutpostIdBeforeUserValidation = 0;
}

//-----------------------------------------------------------------------------
uint32 CCharacter::getLastDisconnectionDate()
{
	if (_LastLogStats.size() < 2)
	{
		// this is the first connection, return 0 as last log time
		return 0;
	}
	else
	{
		// return the last disconnection time (ie, the date in the second log record)
		std::list<TCharacterLogTime>::iterator it(_LastLogStats.begin());
		++it;

		return it->LogoffTime;
	}
}


//-----------------------------------------------------------------------------
void CCharacter::setDynChatChan(TChanID id, uint32 name, bool writeRight)
{
	// find entry for chan
	uint entry;
//	std::string basePath;
	CBankAccessor_PLR::TDYN_CHAT::TCHANNEL channel;
	// see if already exists
	for (entry = 0; entry < CChatGroup::MaxDynChanPerPlayer; ++entry)
	{
//		basePath = toString("DYN_CHAT:CHANNEL%d", (int) entry);
//		CEntityId currId = CEntityId(uint64(_PropertyDatabase.getProp(basePath + ":ID")));
		channel = CBankAccessor_PLR::getDYN_CHAT().getCHANNEL(entry);
		CEntityId currId( channel.getID(_PropertyDatabase) );
		if (currId == id)
			break;
	}
	if (entry == CChatGroup::MaxDynChanPerPlayer)
	{
		// search empty place
		for (entry = 0; entry < CChatGroup::MaxDynChanPerPlayer; ++entry)
		{
//			basePath = toString("DYN_CHAT:CHANNEL%d", (int) entry);
//			uint32 currName = (uint32) _PropertyDatabase.getProp(basePath + ":NAME");
			channel = CBankAccessor_PLR::getDYN_CHAT().getCHANNEL(entry);
			uint32 currName =  channel.getNAME(_PropertyDatabase);
			if (currName == 0) break;
		}
		if (entry == CChatGroup::MaxDynChanPerPlayer)
		{
			nlwarning("Too many channels");
			return;
		}
	}
//	_PropertyDatabase.setProp(basePath + ":ID", sint64(id.getRawId()));
	channel.setID(_PropertyDatabase, id.getRawId());
	channel.setNAME(_PropertyDatabase, name);
	channel.setWRITE_RIGHT(_PropertyDatabase, writeRight);
}

//-----------------------------------------------------------------------------
void CCharacter::removeDynChatChan(TChanID id)
{
	uint entry;
//	std::string basePath;
	CBankAccessor_PLR::TDYN_CHAT::TCHANNEL channel;
	// see if already exists
	for (entry = 0; entry < CChatGroup::MaxDynChanPerPlayer; ++entry)
	{
//		basePath = toString("DYN_CHAT:CHANNEL%d", (int) entry);
//		CEntityId currId = CEntityId(uint64(_PropertyDatabase.getProp(basePath + ":ID")));
		channel = CBankAccessor_PLR::getDYN_CHAT().getCHANNEL(entry);
		CEntityId currId(channel.getID(_PropertyDatabase));
		if (currId == id) break;
	}
	if (entry == CChatGroup::MaxDynChanPerPlayer)
	{
		nlwarning("Channel not found");
		return;
	}
//	_PropertyDatabase.setProp(basePath + ":ID", 0);
//	_PropertyDatabase.setProp(basePath + ":NAME", 0);
//	_PropertyDatabase.setProp(basePath + ":WRITE_RIGHT", 0);
	channel.setID(_PropertyDatabase, 0);
	channel.setNAME(_PropertyDatabase, 0);
	channel.setWRITE_RIGHT(_PropertyDatabase, false);

}

//-----------------------------------------------------------------------------
void CCharacter::setWeatherValue(uint16 value)
{
//	_PropertyDatabase.setProp("WEATHER:VALUE", value);
	CBankAccessor_PLR::getWEATHER().setVALUE(_PropertyDatabase, value);
}

//--------------------------------------------------------------
//	CPetAnimal
//--------------------------------------------------------------
CPetAnimal::CPetAnimal()
{
	clear();
}

//-----------------------------------------------------------------------------
void CPetAnimal::clear()
{
	PetStatus = not_present;
	DbPetStatus = db_unknown;
	TicketPetSheetId = NLMISC::CSheetId::Unknown;
	PetSheetId = NLMISC::CSheetId::Unknown;
	Price = 0;
	ItemPtr = NULL;
	OwnerId = NLMISC::CEntityId::Unknown;
	SpawnedPets = TDataSetRow();
	StableId = 0;
	Landscape_X = 0;
	Landscape_Y = 0;
	Landscape_Z = 0;
	DeathTick = 0;
	Slot = INVENTORIES::INVALID_INVENTORY_SLOT;
	Satiety = SatietyNotInit;
	MaxSatiety = SatietyNotInit;
	AnimalStatus = 0;
	IsFollowing = false;
	IsMounted = false;
	IsTpAllowed = false;
	spawnFlag = false;
}

//-----------------------------------------------------------------------------
void CPetAnimal::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// ensure we won't save in this format anymore
	nlassertex( f.isReading(), ("<CPetAnimal::serial> you should not save in old format anymore!!!") );

	f.serialEnum( PetStatus );
	f.serial( TicketPetSheetId );
	f.serial( PetSheetId );
	f.serial( Price );

	if( f.isReading() )
	{
		// do not change !!! backward compatibility stuff !!!
		sint16 oldSlot;
		f.serial( oldSlot );

		if (oldSlot < 0)
			Slot = INVENTORIES::INVALID_INVENTORY_SLOT;
		else
			Slot = uint32(oldSlot);
	}
	else
	{
		// this serial is only here for loading old save game!!!
		nlstop;

		if( ItemPtr != 0 )
		{
			Slot = ItemPtr->getInventorySlot();
		}
		sint16 oldSlot;
		if (Slot == INVENTORIES::INVALID_INVENTORY_SLOT)
			oldSlot = -1;
		else
			oldSlot = sint16(Slot);
		f.serial( oldSlot );
	}

	f.serial( OwnerId );
	f.serial( StableId );
	if ( f.isReading() && PetStatus == CPetAnimal::stable)
	{
		StableId = BACK_COMPAT::oldToNewPlaceId( uint16(StableId) );
	}

	f.serial( Landscape_X );
	f.serial( Landscape_Y );
	f.serial( Landscape_Z );

	if (f.isReading())
	{
		// restore the death date to now
		DeathTick = CTickEventHandler::getGameCycle();
	}
}

//-----------------------------------------------------------------------------
uint32 CPetAnimal::initLinkAnimalToTicket( CCharacter * c, uint8 index )
{
	if( c )
	{
		ItemPtr = c->getItem( INVENTORIES::bag, Slot );
		if( ( ItemPtr != 0 ) && ( ItemPtr->getStaticForm() != NULL ) && ( ItemPtr->getStaticForm()->Family == ITEMFAMILY::PET_ANIMAL_TICKET ) )
		{
//			Slot = ItemPtr->getLocSlot();
			ItemPtr->setPetIndex(index);
			ItemPtr->setCustomName(CustomName);
			Slot = ItemPtr->getInventorySlot();
			return Slot;
		}
		else
		{
			nlwarning("<initLinkAnimalToTicket> Invalid Pet ticket for user '%s'", c->getId().toString().c_str() );
			// create another ticket
			ItemPtr = c->createItemInInventoryFreeSlot( INVENTORIES::bag, 1, 1, TicketPetSheetId, c->getId() );
			if( ItemPtr != 0 )
			{
//				Slot = ItemPtr->getLocSlot();
				Slot = ItemPtr->getInventorySlot();
				ItemPtr->setPetIndex(index);
				ItemPtr->setCustomName(CustomName);
				return Slot;
			}
			else
			{
				// can't re-create pet ticket
				nlwarning("<CPetAnimal::initLinkAnimalToTicket> Can't re-create pet ticket %s for character %s, sheet not exist or bag full ?. REMOVING PET !!", TicketPetSheetId.toString().c_str(), c->getId().toString().c_str() );
				PetStatus = not_present;
				ItemPtr = 0;
				Slot = INVENTORIES::INVALID_INVENTORY_SLOT;
				return INVENTORIES::INVALID_INVENTORY_SLOT;
			}
		}
	}
	return INVENTORIES::INVALID_INVENTORY_SLOT;
}

//-----------------------------------------------
// getAnimalMaxBulk
//-----------------------------------------------
uint32 CPetAnimal::getAnimalMaxBulk()
{
	const CStaticCreatures * form = CSheets::getCreaturesForm( PetSheetId );
	if( form )
	{
		CSheetId creatureBagSheet;

		// work around because the forms contain "pack_animal_inventory.sitem" for all animal inventories
		if ( form->getRace() == EGSPD::CPeople::MektoubMount)
		{
			creatureBagSheet = CSheetId( "steed_inventory.sitem" );
		}
		else
		{
			creatureBagSheet = CSheetId( form->getBagInventorySheet() );
		}

		const CStaticItem * formBag = CSheets::getForm( creatureBagSheet );
		if( formBag )
		{
			return formBag->BulkMax;
		}
	}

	return 0;
} // getAnimalMaxBulk //

//----------------------------------------------------------------------------
// aggroable status
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void CCharacter::setAggroable(bool aggroable, bool forceUpdate)
{
//	if (aggroable!=_Aggroable || forceUpdate)
	{
		_Aggroable = aggroable;

		// Test override to avoid unnecessary messages (include privileges)
//		if (!isAggroableOverridden())
			if ( getInstanceNumber() != INVALID_AI_INSTANCE)
			{
					sendAggroable();
			}


		// update the high dword of whoSeesMe field to keep aggro and mob vision in sync
		_WhoSeesMe= R2_VISION::buildWhoSeesMe((R2_VISION::TInvisibilityLevel)(uint32)_WhoSeesMe,aggroable);
	}
}

//----------------------------------------------------------------------------
bool CCharacter::isAggroable()
{
	bool defined = false;
	if (_AggroableOverride >= 0)
	{
		return (_AggroableOverride > 0);
	}
	else
	{
		// no aggro for GM, SGM, G, VG, SG and OBSERVER
		CPlayer* p = PlayerManager.getPlayer(PlayerManager.getPlayerId(_Id));
		if (p != NULL)
		{
			if (p->havePriv(NeverAggroPriv))
				return false;
		}
		return _Aggroable;
	}
}

//----------------------------------------------------------------------------
void CCharacter::setAggroableOverride(sint8 aggroableOverride)
{
	_AggroableOverride = aggroableOverride;

	// notify the AIS each time (not only when the value changes)
	sendAggroable();

	// update the high dword of whoSeesMe field to keep aggro and mob vision in sync
	switch(aggroableOverride)
	{
	case -1:
		_WhoSeesMe= R2_VISION::buildWhoSeesMe((R2_VISION::TInvisibilityLevel)(uint32)_WhoSeesMe,_Aggroable);
		break;

	case 0:
		_WhoSeesMe= R2_VISION::buildWhoSeesMe((R2_VISION::TInvisibilityLevel)(uint32)_WhoSeesMe,false);
		break;

	case 1:
		_WhoSeesMe= R2_VISION::buildWhoSeesMe((R2_VISION::TInvisibilityLevel)(uint32)_WhoSeesMe,true);
		break;

	default:
		STOP("Attempted to set aggroableOverride to invalid value: "<<aggroableOverride);
	}
}

//----------------------------------------------------------------------------
sint8 CCharacter::getAggroableOverride() const
{
	return _AggroableOverride;
}

//----------------------------------------------------------------------------
bool CCharacter::isAggroableOverridden()
{
	if (_AggroableOverride >= 0)
		return true;
	CPlayer* p = PlayerManager.getPlayer(PlayerManager.getPlayerId(_Id));
	if (p != NULL)
	{
		if (p->havePriv(NeverAggroPriv))
			return true;
	}
	return false;
}

//----------------------------------------------------------------------------
void CCharacter::sendAggroable()
{
	CEnableAggroOnPlayerMsg msg;
	msg.EntityRowId = getEntityRowId();
	msg.EnableAggro = isAggroable();
	// inform AI of new aggro status
	CMirrorPropValue<uint32> instanceNumber(TheDataset, getEntityRowId(), DSPropertyAI_INSTANCE );
	if (IsRingShard)
	{

		if ( CWorldInstances::instance().getAISId(static_cast<uint32>(instanceNumber)) == NLNET::TServiceId(0))
		{
			// The Ai Instance can be temporary removed  when we do a stop test and go to edit session so no message can message can be send
			return;
		}
	}


	if (instanceNumber() != INVALID_AI_INSTANCE)
		CWorldInstances::instance().msgToAIInstance(instanceNumber, msg);
	else
	{
		nldebug("CEntityBase::sendAggroable : CAN'T SET AGGRO, NO AI INSTANCE FOR CHAR %s", getId().toString().c_str());
	}
}

//----------------------------------------------------------------------------
uint32 CCharacter::updateDeathPenaltyResorption()
{
	return _DeathPenalties->updateResorption(*this);
}

//----------------------------------------------------------------------------
float CCharacter::getDPLossDuration() const
{
	if (_DPLossDuration <= 0.f)
		return float(DeathXPResorptionTime.get());

	return _DPLossDuration;
}

//----------------------------------------------------------------------------
void	CCharacter::barUpdate()
{
	H_AUTO(CharacterBarUpdate);
	if ( ! getEnterFlag() ) // wait for the properties to be in the mirror
		return;

	/* This message in extreme case can be send very often (if eg the player is damaged from lot of creatures)
		If for instance the player is temporary disconnected, then stop to send this message
	*/
	sint32	impulseWindowBitSize = (sint32)(_AvailImpulseBitsize.isReadable() ? _AvailImpulseBitsize() : 0);
	if(impulseWindowBitSize<=0)
		return;

	// if different from what sent to player
	if( _PhysScores._PhysicalScores[ SCORES::hit_points ].Current() != _OldHpBarSentToPlayer ||
		_PhysScores._PhysicalScores[ SCORES::sap].Current() != _OldSapBarSentToPlayer ||
		_PhysScores._PhysicalScores[ SCORES::stamina].Current() != _OldStaBarSentToPlayer ||
		_PhysScores._PhysicalScores[ SCORES::focus].Current() != _OldFocusBarSentToPlayer )
	{
		// bkup cache
		_OldHpBarSentToPlayer= _PhysScores._PhysicalScores[ SCORES::hit_points ].Current();
		_OldSapBarSentToPlayer= _PhysScores._PhysicalScores[ SCORES::sap].Current();
		_OldStaBarSentToPlayer= _PhysScores._PhysicalScores[ SCORES::stamina].Current();
		_OldFocusBarSentToPlayer= _PhysScores._PhysicalScores[ SCORES::focus].Current();

		// Since client must listen only the last message (no delta like DB here...), use a small counter
		_BarSentToPlayerMsgNumber++;

		// message to player
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( _Id );
		CBitMemStream bms;
		if ( ! GenericMsgManager.pushNameToStream( "USER:BARS", bms) )
		{
			nlwarning("<barUpdate> Msg name USER:BARS not found");
		}
		else
		{
			// write the message
			bms.serial( _BarSentToPlayerMsgNumber );
			bms.serial( _OldHpBarSentToPlayer );
			bms.serial( _OldSapBarSentToPlayer );
			bms.serial( _OldStaBarSentToPlayer );
			bms.serial( _OldFocusBarSentToPlayer );
			// send
			msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
			CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
		}
	}
}

//----------------------------------------------------------------------------
void CCharacter::resetTpTicketSlot()
{
	_TpTicketSlot = INVENTORIES::INVALID_INVENTORY_SLOT;
}

//----------------------------------------------------------------------------
void CCharacter::updateParry(ITEMFAMILY::EItemFamily family, SKILLS::ESkills skill)
{
	if ((family == ITEMFAMILY::MELEE_WEAPON || family == ITEMFAMILY::RANGE_WEAPON)  && skill < SKILLS::NUM_SKILLS)
		_CurrentParrySkill = skill;
	else
		_CurrentParrySkill = BarehandCombatSkill;

	_BaseParryLevel = getSkillBaseValue(_CurrentParrySkill);
	_CurrentParryLevel = max( sint32(0), _BaseParryLevel + _ParryModifier );

//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryBase, _BaseParryLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setBase(_PropertyDatabase, checkedCast<uint16>(_BaseParryLevel) );
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent, _CurrentParryLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel) );
}

//----------------------------------------------------------------------------
void CCharacter::updateMagicProtectionAndResistance()
{
	// update magic protection values & magic resistance value

	CInventoryPtr invPtr = getInventory(INVENTORIES::equipment);
	BOMB_IF( invPtr == NULL, "Equipement Inventory Ptr is NULL !!!", return );

	// magic protections
	for( uint32 i = 0; i < PROTECTION_TYPE::NB_PROTECTION_TYPE; ++i)
	{
		_MagicProtection[i] = 0;
		switch(i)
		{
		case PROTECTION_TYPE::Acid:
		case PROTECTION_TYPE::Cold:
		case PROTECTION_TYPE::Rot:
			_MagicProtection[i] = HominBaseProtection;
			break;
		case PROTECTION_TYPE::Fire:
			if( _Race == EGSPD::CPeople::Fyros )
			{
				_MagicProtection[i] = HominRacialProtection;
			}
			break;
		case PROTECTION_TYPE::Poison:
			if( _Race == EGSPD::CPeople::Matis )
			{
				_MagicProtection[i] = HominRacialProtection;
			}
			break;
		case PROTECTION_TYPE::Shockwave:
			if( _Race == EGSPD::CPeople::Tryker )
			{
				_MagicProtection[i] = HominRacialProtection;
			}
			break;
		case PROTECTION_TYPE::Electricity:
			if( _Race == EGSPD::CPeople::Zorai )
			{
				_MagicProtection[i] = HominRacialProtection;
			}
			break;
		default:
			break;
		}
	}

	_MaxAbsorption = (getSkillBaseValue(getBestSkill()) * MaxAbsorptionFactor) / 100;

	// magic resistance
	_BaseResistance = (sint32)(_Skills._Skills[SKILLS::SF].MaxLvlReached * MagicResistFactorForCombatSkills) + MagicResistSkillDelta;
	
	sint32 magicResist = ((sint32)(_Skills._Skills[SKILLS::SM].MaxLvlReached * MagicResistFactorForMagicSkills) + MagicResistSkillDelta);
	_BaseResistance = max(_BaseResistance, magicResist);
	
	sint32 forageResist = ((sint32)(_Skills._Skills[SKILLS::SH].MaxLvlReached * MagicResistFactorForForageSkills) + MagicResistSkillDelta);
	_BaseResistance = max(_BaseResistance, forageResist);

	clamp(_BaseResistance, 0, 225);

	// set up base
	for( uint32 i = 0; i < RESISTANCE_TYPE::NB_RESISTANCE_TYPE; ++i )
	{
		_MagicResistance[i]= (uint32)_BaseResistance * 100;
	}

	// correct for race
	switch ( _Race)
	{
		case EGSPD::CPeople::Fyros:
			_MagicResistance[RESISTANCE_TYPE::Desert] += HominRacialResistance * 100;
			break;

		case EGSPD::CPeople::Matis:
			_MagicResistance[RESISTANCE_TYPE::Forest] += HominRacialResistance * 100;
			break;

		case EGSPD::CPeople::Tryker:
			_MagicResistance[RESISTANCE_TYPE::Lacustre] += HominRacialResistance * 100;
			break;

		case EGSPD::CPeople::Zorai:
			_MagicResistance[RESISTANCE_TYPE::Jungle] += HominRacialResistance * 100;
			break;

		default:
			break;
	}

	// correct for current region
	for( uint32 i = 0; i < RESISTANCE_TYPE::NB_RESISTANCE_TYPE; ++i )
	{
		_MagicResistance[i] = (uint32)((sint32)max( (sint32)0, ((sint32)_MagicResistance[i]) + getRegionResistanceModifier((RESISTANCE_TYPE::TResistanceType)i) * (sint32)100));
	}

	// protection
	PROTECTION_TYPE::TProtectionType protectionType;
	uint32 protectionValue;

	for( uint32 slot = 0; slot < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++slot)
	{
		CGameItemPtr item = invPtr->getItem(slot);
		if (item != NULL)
		{
			if (item->getStaticForm() != NULL && item->getStaticForm()->Family == ITEMFAMILY::JEWELRY)
			{
				for( uint32 protectionNumber = 1; protectionNumber <= 3; ++protectionNumber )
				{
					item->magicProtection(protectionNumber, protectionType, protectionValue);

					if (protectionType != PROTECTION_TYPE::None)
					{
						_MagicProtection[ protectionType ] += protectionValue;
					}
				}

				_MaxAbsorption += (item->recommended() * MaxAbsorptionFactor) / 100;

				// resistance
				for( uint32 res = 0; res < RESISTANCE_TYPE::NB_RESISTANCE_TYPE; ++res )
				{
					_MagicResistance[res]+= item->magicResistance((RESISTANCE_TYPE::TResistanceType)res);
				}
			}
		}
	}

	// update current magic protection effect
	const std::vector< CSEffectPtr >& activeEffects = getAllActiveEffects();
	for( uint i = 0; i < activeEffects.size(); ++i )
	{
		if( activeEffects[i]->getFamily() == EFFECT_FAMILIES::PowerModMagicProtection )
		{
			CModMagicProtectionEffect * effect = dynamic_cast<CModMagicProtectionEffect *>(&(*activeEffects[i]));
			if(effect)
			{
				_MagicProtection[effect->getAffectedProtection()]+= effect->getEffectValue();
			}
		}
	}

	for( uint32 res = 0; res < RESISTANCE_TYPE::NB_RESISTANCE_TYPE; ++res )
	{
		_MagicResistance[res]= (uint32)(_MagicResistance[res] * 0.01f);

		// add spire effect ( magic resistance )
		const CSEffect* pEffect = lookForActiveEffect( EFFECT_FAMILIES::TotemCombatRes );
		if ( pEffect != NULL )
		{
			_MagicResistance[res] += ( (sint32)_MagicResistance[res] * pEffect->getParamValue() / 100 );
		}
	}

//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:MaxProtectionClampValue", MaxMagicProtection);
	CBankAccessor_PLR::getCHARACTER_INFO().getMAGIC_PROTECTION().setMaxProtectionClampValue(_PropertyDatabase, checkedCast<uint16>(MaxMagicProtection.get()));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:MaxAbsorptionFactor", MaxAbsorptionFactor);
	CBankAccessor_PLR::getCHARACTER_INFO().getMAGIC_PROTECTION().setMaxAbsorptionFactor(_PropertyDatabase, checkedCast<uint16>(MaxAbsorptionFactor.get()));

	for (uint i=0; i<PROTECTION_TYPE::NB_PROTECTION_TYPE; ++i)
	{
		CBankAccessor_PLR::getCHARACTER_INFO().getMAGIC_PROTECTION().getArray(i).setVALUE(_PropertyDatabase, checkedCast<uint16>(getUnclampedMagicProtection((PROTECTION_TYPE::TProtectionType)i)));
	}
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Cold", getUnclampedMagicProtection(PROTECTION_TYPE::Cold));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Acid", getUnclampedMagicProtection(PROTECTION_TYPE::Acid));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Rot", getUnclampedMagicProtection(PROTECTION_TYPE::Rot));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Fire", getUnclampedMagicProtection(PROTECTION_TYPE::Fire));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Shockwave", getUnclampedMagicProtection(PROTECTION_TYPE::Shockwave));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Poison", getUnclampedMagicProtection(PROTECTION_TYPE::Poison));
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:Electricity", getUnclampedMagicProtection(PROTECTION_TYPE::Electricity));

//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_RESISTANCE:MaxResistanceBonus", MaxMagicResistanceBonus);
	CBankAccessor_PLR::getCHARACTER_INFO().getMAGIC_RESISTANCE().setMaxResistanceBonus(_PropertyDatabase, checkedCast<uint16>(MaxMagicResistanceBonus.get()));

	for (uint i=0; i<RESISTANCE_TYPE::NB_RESISTANCE_TYPE; ++i)
	{
		CBankAccessor_PLR::getCHARACTER_INFO().getMAGIC_RESISTANCE().getArray(i).setVALUE(_PropertyDatabase, checkedCast<uint16>(getUnclampedMagicResistance((RESISTANCE_TYPE::TResistanceType)i)));
	}
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_RESISTANCE:Desert", _MagicResistance[RESISTANCE_TYPE::Desert]);
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_RESISTANCE:Forest", _MagicResistance[RESISTANCE_TYPE::Forest]);
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_RESISTANCE:Lacustre", _MagicResistance[RESISTANCE_TYPE::Lacustre]);
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_RESISTANCE:Jungle", _MagicResistance[RESISTANCE_TYPE::Jungle]);
//	_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_RESISTANCE:PrimaryRoot", _MagicResistance[RESISTANCE_TYPE::PrimaryRoot]);
}

//----------------------------------------------------------------------------
sint32 CCharacter::getRegionResistanceModifier( RESISTANCE_TYPE::TResistanceType resistanceType )
{
	CVector pos;
	_EntityState.getVector( pos );
	ECOSYSTEM::EECosystem ecosystem = CZoneManager::getInstance().getEcotype( pos );

	switch( ecosystem )
	{
	case ECOSYSTEM::desert:
		if( resistanceType == RESISTANCE_TYPE::Desert )
			return -(sint32)EcosystemResistancePenalty;
		break;
	case ECOSYSTEM::forest:
		if( resistanceType == RESISTANCE_TYPE::Forest )
			return -(sint32)EcosystemResistancePenalty;
		break;
	case ECOSYSTEM::lacustre:
		if( resistanceType == RESISTANCE_TYPE::Lacustre )
			return -(sint32)EcosystemResistancePenalty;
		break;
	case ECOSYSTEM::jungle:
		if( resistanceType == RESISTANCE_TYPE::Jungle )
			return -(sint32)EcosystemResistancePenalty;
		break;
	case ECOSYSTEM::primary_root:
		if( resistanceType == RESISTANCE_TYPE::PrimaryRoot )
			return -(sint32)EcosystemResistancePenalty;
		break;
	default:
		break;
	}
	return 0;
}

//----------------------------------------------------------------------------
void CCharacter::addEntityToSaveWithMe(const NLMISC::CEntityId &entityId)
{
	// insert in the set (hence no op if already here)
	_EntitiesToSaveWithMe.insert(entityId);
}

//-----------------------------------------------------------------------------
void	CCharacter::clearEntitiesToSaveWithMe()
{
	_EntitiesToSaveWithMe.clear();
}

//-----------------------------------------------------------------------------
const std::set<NLMISC::CEntityId>& CCharacter::getEntitiesToSaveWithMe() const
{
	return _EntitiesToSaveWithMe;
}

//-----------------------------------------------------------------------------
sint32 CCharacter::getSkillModifierForRace(EGSPD::CPeople::TPeople people) const
{
	// if the array of skill modifiers has no value != 0, just returns
	if (_NbNonNullClassificationTypesSkillMod == 0)
		return 0;

	if (people < 0 || people >= EGSPD::CPeople::Unknown)
		return 0;

	vector<CClassificationType::TClassificationType> types;
	EGSPD::getMatchingClassificationType(people, types);

	sint32 max = 0;
	const uint nbTypes = (uint)types.size();
	for (uint i = 0 ; i < nbTypes ; ++i)
	{
		if (types[i] >=0  && types[i] < EGSPD::CClassificationType::EndClassificationType)
		{
			const sint32 mod = _ClassificationTypesSkillModifiers[(uint)types[i]];
			if (mod > max)
				max = mod;
		}
	}

	return max;
}

//-----------------------------------------------------------------------------
void CCharacter::updateConnexionStat()
{
	_LastConnectedTime = CTime::getSecondsSince1970();
	_LastLogStats.push_front( TCharacterLogTime() );
	_LastLogStats.begin()->LoginTime = _LastConnectedTime;
	_LastLogStats.begin()->LogoffTime = 0;
	_LastLogStats.begin()->Duration = 0;

	uint32 size = (uint32)_LastLogStats.size();
	if( size > NBLoginStats )
	{
		for( uint i = 0; i < size - NBLoginStats; ++i )
		{
			_LastLogStats.pop_back();
		}
	}
}

//-----------------------------------------------------------------------------
void CCharacter::setDisconnexionTime()
{
	uint32 time = CTime::getSecondsSince1970();
	_PlayedTime += time - _LastConnectedTime;
	_LastConnectedTime = time;
	_LastLogStats.begin()->LogoffTime = _LastConnectedTime;
	_LastLogStats.begin()->Duration = time - _LastLogStats.begin()->LoginTime;
}

//-----------------------------------------------------------------------------
void CCharacter::updateOutpostAdminFlagInDB()
{
	bool isOutpostAdmin = false;
	CGuildMemberModule * memberModule = NULL;
	if ( _GuildId != 0 && getModuleParent().getModule( memberModule ) )
	{
		isOutpostAdmin = memberModule->isOutpostAdmin();
	}
//	_PropertyDatabase.setProp("USER:OUTPOST_ADMIN", isOutpostAdmin);
	CBankAccessor_PLR::getUSER().setOUTPOST_ADMIN(_PropertyDatabase, isOutpostAdmin);
}

//-----------------------------------------------------------------------------
std::vector<SItemSpecialEffect> CCharacter::lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::TItemSpecialEffect effectType) const
{
	CGameItemPtr usedItem;
	std::vector<SItemSpecialEffect> effects;
	usedItem = getRightHandItem();
	if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
	{
		const std::vector<SItemSpecialEffect>& effects2 = usedItem->getStaticForm()->lookForEffects(effectType);
		effects.insert(effects.end(), effects2.begin(), effects2.end());
	}
	usedItem = getLeftHandItem();
	if (usedItem!=NULL && usedItem->getStaticForm() && usedItem->getStaticForm()->ItemSpecialEffects && !usedItem->getStaticForm()->ItemSpecialEffects->Effects.empty())
	{
		const std::vector<SItemSpecialEffect>& effects2 = usedItem->getStaticForm()->lookForEffects(effectType);
		effects.insert(effects.end(), effects2.begin(), effects2.end());
	}
	return effects;
}

//-----------------------------------------------------------------------------
bool CCharacter::canBelongToGuild( uint32 guildId, bool setToNone /*= false*/ )
{
	// Allow changes to guild 0 (no guild)
	if (guildId == 0)
	{
		return true;
	}

	// The guild's Cult and Civ allegiance must he same as the character's allegiance,
	//  or the character must be "unspecified" (None) or Neutral, else they can't be in the guild.
	CGuild* theGuild = CGuildManager::getInstance()->getGuildFromId(guildId);
	if (theGuild)
	{
		// guildAllegiance is a pair; first element = cult, second element = civilization.
		std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> guildAllegiance = theGuild->getAllegiance();

		if (guildAllegiance.first != _DeclaredCult
			&& _DeclaredCult != PVP_CLAN::None && _DeclaredCult != PVP_CLAN::Neutral)
		{
			// Check parameter.  Used to correct players just logging on if guild changed
			//   allegiance.  If we change it, we should return true.
			if (setToNone)
			{
				_DeclaredCult = PVP_CLAN::None;
			}
			else
			{
				return false;
			}
		}

		if (guildAllegiance.second != _DeclaredCiv
		    && _DeclaredCiv != PVP_CLAN::None && _DeclaredCiv != PVP_CLAN::Neutral)
		{
			// Check parameter.  Used to correct players just logging on if guild changed
			//   allegiance.  If we change it, we should return true.
			if (setToNone)
			{
				_DeclaredCiv = PVP_CLAN::None;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool CCharacter::setGuildId( uint32 guildId )
{
	// Check guild allegiance
	if (canBelongToGuild(guildId))
	{
		// warn the SU to update the ring database
		if (IShardUnifierEvent::getInstance()
			&& _GuildId != guildId)
			IShardUnifierEvent::getInstance()->onUpdateCharGuild(_Id, guildId);

		_GuildId = guildId;

		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------
void CCharacter::updateGuildFlag() const
{
	CMirrorPropValue<TYPE_GUILD_SYMBOL> guildFlag( TheDataset, this->getEntityRowId(), DSPropertyGUILD_SYMBOL );
	CMirrorPropValue<TYPE_PVP_CLAN> propPvpClan( TheDataset, this->getEntityRowId(), DSPropertyPVP_CLAN );
	// If we should display faction symbol
	if (_UseFactionSymbol && (PVP_CLAN::TPVPClan)propPvpClan.getValue()!=PVP_CLAN::None)
	{
		NLMISC::CSheetId faction = PVP_CLAN::getFactionSheetId((PVP_CLAN::TPVPClan)propPvpClan.getValue());
		guildFlag = sint64(faction.asInt()) | sint64(1)<<59;
	}
	else
	{
		CGuild* guild = CGuildManager::getInstance()->getGuildFromId(this->getGuildId());
		if (guild)
			guildFlag = guild->getIcon();
		else
			guildFlag = 0;
	}
}


//-----------------------------------------------
//	isAnActiveXpCatalyser
//-----------------------------------------------
bool CCharacter::isAnActiveXpCatalyser( CGameItemPtr item )
{
	if( item != NULL )
	{
		const CStaticItem * form = item->getStaticForm();
		if (form != NULL)
		{
			if( form->Family == ITEMFAMILY::XP_CATALYSER )
			{
				if( item->getInventory() != NULL )
				{
					if( item->getInventory()->getInventoryId()==INVENTORIES::bag && (item->getInventorySlot()==_XpCatalyserSlot || item->getInventorySlot()==_RingXpCatalyserSlot) )
					{
						return true;
					}
				}
			}
		}
	}
	return false;

} // isAnActiveXpCatalyser //


//------------------------------------------------------------------------------
// methods converted from inlines in character.h (Sadge)
//------------------------------------------------------------------------------

void CCharacter::setStartupInstance(uint32 instanceId)
{
	_StartupInstance = instanceId;
}


//------------------------------------------------------------------------------

void CCharacter::setTitle( CHARACTER_TITLE::ECharacterTitle title )
{
	setNewTitle(CHARACTER_TITLE::toString(title));
}


//------------------------------------------------------------------------------

void CCharacter::setUpdateNextTick()
{
	_TickUpdateTimer.setRemaining( 1, _TickUpdateTimer.getEvent() );
}


//------------------------------------------------------------------------------

void CCharacter::setState( const COfflineEntityState& es )
{
	CEntityBase::setState(es);
}


//------------------------------------------------------------------------------

void CCharacter::addTargetingChar( const TDataSetRow & row )
{
	if ( std::find( _TargetingChars.begin(), _TargetingChars.end(), row ) !=  _TargetingChars.end() )
		_TargetingChars.push_back( row );
}


//------------------------------------------------------------------------------

void CCharacter::removeTargetingChar( const TDataSetRow & row )
{
	std::vector<TDataSetRow>::iterator it = std::find( _TargetingChars.begin(), _TargetingChars.end(), row );
	if ( it != _TargetingChars.end() )
	{
		_TargetingChars.back() == *it;
		_TargetingChars.pop_back();
	}
}


//------------------------------------------------------------------------------

void CCharacter::updateTarget()
{
	if( TheDataset.isAccessible( _Target() ) )
	{
		NLMISC::CEntityId target = TheDataset.getEntityId(_Target());
		setTarget(NLMISC::CEntityId::Unknown, false);
		setTarget(target, false);
	}
}


//------------------------------------------------------------------------------

void CCharacter::tpWanted( sint32 x, sint32 y, sint32 z , bool useHeading, float heading, uint8 continent, sint32 cell)
{
	teleportCharacter(x,y,z,false,useHeading,heading,continent,cell);
}


//------------------------------------------------------------------------------

void CCharacter::teleportCharacter( sint32 x, sint32 y)
{
	teleportCharacter( x, y, 0, false );
}


//------------------------------------------------------------------------------

void CCharacter::setTeamId(uint16 id)
{
	_TeamId = id;
	updatePVPClanVP();
}

void CCharacter::setLeagueId(TChanID id, bool removeIfEmpty)
{

	ucstring name = CEntityIdTranslator::getInstance()->getByEntity(getId());
	CEntityIdTranslator::removeShardFromName(name);

	// Remove old dynamic channel
	if (_LeagueId != DYN_CHAT_INVALID_CHAN)
	{
		CPVPManager2::getInstance()->broadcastMessage(_LeagueId, string("<INFO>"), name+" -->[]");
		PHRASE_UTILITIES::sendDynamicSystemMessage(getEntityRowId(), "TEAM_QUIT_LEAGUE");
		DynChatEGS.removeSession(_LeagueId, getEntityRowId());		
		
		vector<CEntityId> players;
		bool isEmpty = DynChatEGS.getPlayersInChan(_LeagueId, players);
				
		if (isEmpty)
		{
			if (removeIfEmpty)
				DynChatEGS.removeChan(_LeagueId);
		}
	}

	if (id != DYN_CHAT_INVALID_CHAN)
	{
		PHRASE_UTILITIES::sendDynamicSystemMessage(getEntityRowId(), "TEAM_JOIN_LEAGUE");
		DynChatEGS.addSession(id, getEntityRowId(), true);
		CPVPManager2::getInstance()->broadcastMessage(id, string("<INFO>"), "<-- "+name);
	}

	_LeagueId = id;
	updatePVPClanVP();
}

//------------------------------------------------------------------------------

void CCharacter::setTeamInvitor(const NLMISC::CEntityId & invitorId)
{
	_TeamInvitor = invitorId;
}

//------------------------------------------------------------------------------

void CCharacter::setLeagueInvitor(const NLMISC::CEntityId & invitorId)
{
	_LeagueInvitor = invitorId;
}

//------------------------------------------------------------------------------

void CCharacter::setFightingTarget( const NLMISC::CEntityId& targetId )
{
	CEntityBase::setTarget( targetId );
}


//------------------------------------------------------------------------------

void CCharacter::harvestedEntity( const NLMISC::CEntityId &id)
{
	_MpSourceId = id;
}


//------------------------------------------------------------------------------

void CCharacter::harvestedEntitySheetId( const NLMISC::CSheetId &sheet)
{
	_MpSourceSheetId = sheet;
}


//------------------------------------------------------------------------------

void CCharacter::harvestedMpQuantity(uint16 qty)
{
	_HarvestedQuantity = qty;
}


//------------------------------------------------------------------------------

void CCharacter::resetHarvestInfos()
{
	_MpIndex = 0xff; _HarvestedQuantity = 0;
}


//------------------------------------------------------------------------------

void CCharacter::openHarvest()
{
	_HarvestOpened = true;
}


//------------------------------------------------------------------------------

void CCharacter::setProspectionLocateDepositEffect( CSEffectPtr effect )
{
	_ProspectionLocateDepositEffect = effect;
}


//------------------------------------------------------------------------------

void CCharacter::addXpToSkill( double XpGain, const std::string& Skill)
{
	std::map<SKILLS::ESkills,CXpProgressInfos> dummy;
	addXpToSkillInternal(XpGain, Skill, AddXpToSkillSingle, dummy);
}


//------------------------------------------------------------------------------

void CCharacter::addXpToSkillAndBuffer( double XpGain, const std::string& Skill, std::map<SKILLS::ESkills,CXpProgressInfos> &gainBySkill)
{
	addXpToSkillInternal(XpGain, Skill, AddXpToSkillBuffer, gainBySkill);
}


//------------------------------------------------------------------------------

void CCharacter::addXpToSkillBranch( double XpGain, const std::string& Skill)
{
	std::map<SKILLS::ESkills,CXpProgressInfos> dummy;
	// Warning: disabling XpCat is important here, else could infinite loop (returned XpRemain can already have the XpCat bonus)
	do
	{
		XpGain= addXpToSkillInternal(XpGain, Skill, AddXpToSkillBranch, dummy);
	}
	while( XpGain>0 );
}


//------------------------------------------------------------------------------

void CCharacter::setTimeOfDeath( NLMISC::TGameTime t)
{
	_TimeDeath = t;
}


//------------------------------------------------------------------------------

uint CCharacter::getAnimalIndex( const TDataSetRow& animalRow )
{
	for ( uint i=0; i!=_PlayerPets.size(); ++i )
	{
		if ( _PlayerPets[i].SpawnedPets == animalRow )
			return i;
	}
	return ~0;
}


//------------------------------------------------------------------------------

void CCharacter::allowNearPetTp()
{
	NearPetTpAllowed = true;
}


//------------------------------------------------------------------------------

void CCharacter::forbidNearPetTp()
{
	NearPetTpAllowed = false;
}


//------------------------------------------------------------------------------

void CCharacter::incCurrentBotChatListPage()
{
	_CurrentBotChatListPage++;
}


//------------------------------------------------------------------------------

void CCharacter::resetTpCoordinate()
{
	_TpCoordinate.reset();
}


//------------------------------------------------------------------------------

bool CCharacter::teleportInProgress() const
{
	return (_TpCoordinate.X != 0 || _TpCoordinate.Y != 0 || _TpCoordinate.Z != 0);
}


//------------------------------------------------------------------------------

void CCharacter::setSDBPvPPath(const std::string & sdbPvPPath)
{
	_SDBPvPPath = sdbPvPPath;
}


//------------------------------------------------------------------------------

bool CCharacter::getSDBPvPPath(std::string & sdbPvPPath)
{
	if (_SDBPvPPath.empty())
		return false;
	sdbPvPPath = _SDBPvPPath;
	return true;
}


//------------------------------------------------------------------------------

void CCharacter::setCurrentInterlocutor(const NLMISC::CEntityId & interlocutor)
{
	_CurrentInterlocutor = interlocutor;
}


//------------------------------------------------------------------------------

void CCharacter::setExchangeMoney( uint64 amount )
{
	_ExchangeMoney = amount;
}


//------------------------------------------------------------------------------

void CCharacter::setLootContainer(CInventoryPtr lootSac)
{
	_LootContainer = lootSac;
}


//------------------------------------------------------------------------------

void CCharacter::setEmote( MBEHAV::EBehaviour emote )
{
	setBehaviour( MBEHAV::CBehaviour( emote, (uint16) CTickEventHandler::getGameCycle() ) );
}


//------------------------------------------------------------------------------

void CCharacter::setSaveDate(uint32 nTimeStamp)
{
	_SaveDate = nTimeStamp;
}


//------------------------------------------------------------------------------

void CCharacter::addTradePageToUpdate(uint16 idx)
{
	_TradePagesToUpdate.push_back( idx );
}


//------------------------------------------------------------------------------

const CItemsForSale &CCharacter::getItemInShop()
{
	nlassert(_ItemsInShopStore != NULL); return *_ItemsInShopStore;
}


//------------------------------------------------------------------------------

void CCharacter::resetRawMaterialItemPartFilter()
{
	_RawMaterialItemPartFilter = RM_FABER_TYPE::Unknown;
}


//------------------------------------------------------------------------------

void CCharacter::resetItemTypeFilter()
{
	_ItemTypeFilter = ITEM_TYPE::UNDEFINED;
}


//------------------------------------------------------------------------------

void CCharacter::refreshTradeList()
{
	startTradeItemSession( _CurrentTradeSession + 1 );
}


//------------------------------------------------------------------------------

void	CCharacter::setCurrentTradeSession(uint16 session)
{
	_CurrentTradeSession= session;
}


//------------------------------------------------------------------------------

void CCharacter::createTrain( const TDataSetRow& beastLeaderRow, uint8 nbBeasts )
{
	if ( nbBeasts == 0)
		return;

	if ( !_BeastTrain.empty() )
	{
		nlwarning("<CCharacter::createTrain> Entity %s, a beast train already exists",_Id.toString().c_str() );
		return;
	}

	_BeastTrain.push_back(beastLeaderRow);
	_TrainMaxSize = nbBeasts;
}


//------------------------------------------------------------------------------

void CCharacter::incActionCounter()
{
	++_ActionCounter;
}


//------------------------------------------------------------------------------

void CCharacter::incInterfaceCounter()
{
	++_InterfaceCounter;
//	_PropertyDatabase.setProp( "INVENTORY:COUNTER", _InterfaceCounter );
	CBankAccessor_PLR::getINVENTORY().setCOUNTER(_PropertyDatabase, checkedCast<uint8>(_InterfaceCounter&0xf) );
//	_PropertyDatabase.setProp( "TARGET:CONTEXT_MENU:COUNTER", _InterfaceCounter );
	CBankAccessor_PLR::getTARGET().getCONTEXT_MENU().setCOUNTER(_PropertyDatabase, checkedCast<uint8>(_InterfaceCounter&0xf) );
//	_PropertyDatabase.setProp( "EXCHANGE:COUNTER", _InterfaceCounter );
	CBankAccessor_PLR::getEXCHANGE().setCOUNTER(_PropertyDatabase, checkedCast<uint8>(_InterfaceCounter&0xf));
//	_PropertyDatabase.setProp( "USER:COUNTER", _InterfaceCounter );
	CBankAccessor_PLR::getUSER().setCOUNTER(_PropertyDatabase, checkedCast<uint8>(_InterfaceCounter&0xf) );
}


//------------------------------------------------------------------------------

bool CCharacter::isInPlace( uint16 place )
{
	if ( place == 0xFFFF )
		return false;
	if (_CurrentRegion == place )
		return true;
	if ( std::find( _Places.begin(), _Places.end(), place ) != _Places.end() )
		return true;
	return false;
}


//------------------------------------------------------------------------------

void CCharacter::setCurrentStable (uint16 stable, uint16 placeId)
{
	_CurrentStable = stable;
	_PlaceOfCurrentStable = placeId;
	_RegionOfCurrentStable = _CurrentRegion;
}


//------------------------------------------------------------------------------

void CCharacter::validateMeleeCombat(bool flag)
{
	_MeleeCombatIsValid = flag;
}


//------------------------------------------------------------------------------

void CCharacter::setCraftPlan( NLMISC::CSheetId sheet )
{
	_CraftPlan = sheet;
}


//------------------------------------------------------------------------------

void CCharacter::clearFaberRms()
{
	if( _RmSelectedForFaber.size() > 0 || _RmFormulaSelectedForFaber.size() > 0 )
	{
		unlockFaberRms();
		_RmSelectedForFaber.clear();
		_RmFormulaSelectedForFaber.clear();
	}
}


//------------------------------------------------------------------------------

void CCharacter::setCompassTarget( TDataSetRow rowId)
{
	_CompassTarget = rowId;
}


//------------------------------------------------------------------------------

void CCharacter::writeCycleCounterInDB()
{
//	_PropertyDatabase.setProp( "EXECUTE_PHRASE:CYCLE_COUNTER", _CycleCounter );
	CBankAccessor_PLR::getEXECUTE_PHRASE().setCYCLE_COUNTER(_PropertyDatabase, _CycleCounter );
}


//------------------------------------------------------------------------------

void CCharacter::writeExecPhraseInDB(sint16 id)
{
//	_PropertyDatabase.setProp( "EXECUTE_PHRASE:PHRASE", id );
	CBankAccessor_PLR::getEXECUTE_PHRASE().setPHRASE(_PropertyDatabase, id );

}


//------------------------------------------------------------------------------

void CCharacter::writeNextPhraseInDB(uint8 counter)
{
//	_PropertyDatabase.setProp( "EXECUTE_PHRASE:NEXT_COUNTER", counter );
	CBankAccessor_PLR::getEXECUTE_PHRASE().setNEXT_COUNTER(_PropertyDatabase, counter );
}


//------------------------------------------------------------------------------

CGameItemPtr CCharacter::getRightHandItem() const
{
	return getItem(INVENTORIES::handling, INVENTORIES::right);
}


//------------------------------------------------------------------------------

CGameItemPtr CCharacter::getLeftHandItem() const
{
	return getItem(INVENTORIES::handling, INVENTORIES::left);
}


//------------------------------------------------------------------------------

CGameItemPtr CCharacter::getAmmoItem() const
{
	return getLeftHandItem();
}


//------------------------------------------------------------------------------

void CCharacter::spendSP(double sp, EGSPD::CSPType::TSPType type)
{
	if( type >= 0 && type < EGSPD::CSPType::EndSPType)
	{
		double spOld = _SpType[type];
		if (sp < _SpType[type])
		{
			_SpType[type] -= sp;
			_SpentSpType[type] += (uint32)sp;
		}
		else
		{
			_SpentSpType[type] += (uint32)_SpType[type];
			_SpType[type] = 0.0;
		}

//		_PropertyDatabase.setProp( std::string("USER:SKILL_POINTS_" ) + NLMISC::toString((uint)type) +std::string(":VALUE"), (uint32)(_SpType[type]));
		CBankAccessor_PLR::getUSER().getSKILL_POINTS_(type).setVALUE(_PropertyDatabase, uint32(_SpType[type]));
		log_Character_UpdateSP(EGSPD::CSPType::toString(type), (float)spOld, (float)_SpType[type]);
	}
}


//------------------------------------------------------------------------------

void CCharacter::addSP(double sp, EGSPD::CSPType::TSPType type)
{
	if( type >= 0 && type < EGSPD::CSPType::EndSPType)
	{
		double spOld = _SpType[type];
		_SpType[type] += sp;
//		_PropertyDatabase.setProp( std::string("USER:SKILL_POINTS_" ) + NLMISC::toString((uint)type) +std::string(":VALUE"), (uint32)(_SpType[type]));
		CBankAccessor_PLR::getUSER().getSKILL_POINTS_(type).setVALUE(_PropertyDatabase, uint32(_SpType[type]));
		log_Character_UpdateSP(EGSPD::CSPType::toString(type), (float)spOld, (float)_SpType[type]);
	}
}


//------------------------------------------------------------------------------

double CCharacter::getSP(EGSPD::CSPType::TSPType type)
{
	if( type >= 0 && type < EGSPD::CSPType::EndSPType)
	{
		return _SpType[type];
	}
	return 0;
}


//------------------------------------------------------------------------------

void CCharacter::setSP(double d, EGSPD::CSPType::TSPType type )
{
	if( type >= 0 && type < EGSPD::CSPType::EndSPType)
	{
		double spOld = _SpType[type];
		_SpType[type] = d;
//		_PropertyDatabase.setProp( std::string("USER:SKILL_POINTS_" ) + NLMISC::toString((uint)type) +std::string(":VALUE"), (uint32)(_SpType[type]));
		CBankAccessor_PLR::getUSER().getSKILL_POINTS_(type).setVALUE(_PropertyDatabase, uint32(_SpType[type]));
		log_Character_UpdateSP(EGSPD::CSPType::toString(type), (float)spOld, (float)_SpType[type]);
	}
}


//------------------------------------------------------------------------------

void CCharacter::dodgeAsDefense( bool b)
{
	_DodgeAsDefense = b;
//	_PropertyDatabase.setProp("DEFENSE:DEFENSE_MODE", !b);
	CBankAccessor_PLR::getDEFENSE().setDEFENSE_MODE(_PropertyDatabase, !b);
}

//------------------------------------------------------------------------------

void CCharacter::parrySuccessModifier( sint32 mod )
{
	_ParrySuccessModifier = mod;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:SUCCESS_MODIFIER:PARRY", mod );
	CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().setPARRY(_PropertyDatabase, (sint16)mod );
}

//------------------------------------------------------------------------------

void CCharacter::dodgeSuccessModifier( sint32 mod )
{
	_DodgeSuccessModifier = mod;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:SUCCESS_MODIFIER:DODGE", mod );
	CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().setDODGE(_PropertyDatabase, (sint16)mod );
}

//------------------------------------------------------------------------------

void CCharacter::dateOfNextAllowedAction(NLMISC::TGameCycle date)
{
	_DateOfNextAllowedAction = date;
}


//------------------------------------------------------------------------------

void CCharacter::setForbidAuraUseDates(NLMISC::TGameCycle startDate, NLMISC::TGameCycle endDate)
{
	_ForbidAuraUseStartDate = startDate;
	_ForbidAuraUseEndDate = endDate;

	uint32 flag = BRICK_FLAGS::Aura - BRICK_FLAGS::BeginPowerFlags;
	_PowerFlagTicks[flag].StartTick = startDate;
	_PowerFlagTicks[flag].EndTick = endDate;

	updateBrickFlagsDBEntry();
}


//------------------------------------------------------------------------------

void CCharacter::useAura(POWERS::TPowerType auraType, NLMISC::TGameCycle startDate, NLMISC::TGameCycle endDate, const NLMISC::CEntityId &userId)
{
	_IneffectiveAuras.disableAura( auraType, startDate, endDate, userId );
}


//------------------------------------------------------------------------------

bool CCharacter::isAuraEffective(POWERS::TPowerType auraType, NLMISC::TGameCycle &endDate, const NLMISC::CEntityId &userId)
{
	return _IneffectiveAuras.isAuraEffective(auraType, userId, endDate);
}


//------------------------------------------------------------------------------

bool CCharacter::canUsePower(POWERS::TPowerType type, uint16 consumableFamilyId, NLMISC::TGameCycle &endDate)
{
	return _ForbidPowerDates.isPowerAllowed(type, consumableFamilyId, endDate);
}


//------------------------------------------------------------------------------

void CCharacter::forbidPower(POWERS::TPowerType type, uint16 consumableFamilyId, NLMISC::TGameCycle endDate)
{
	TGameCycle startDate = CTickEventHandler::getGameCycle();

	if (endDate <= startDate)
		return;
#if !FINAL_VERSION
	nlassert(type >=0 && type < POWERS::NbTypes);
#endif
	_ForbidPowerDates.PowerActivationDates.push_back( CPowerActivationDate(type, consumableFamilyId, startDate, endDate) );

	uint flag = BRICK_FLAGS::powerTypeToFlag(type) - BRICK_FLAGS::BeginPowerFlags;
	_PowerFlagTicks[flag].StartTick = startDate;
	_PowerFlagTicks[flag].EndTick = endDate;
	updateBrickFlagsDBEntry();
}


//------------------------------------------------------------------------------

void CCharacter::resetPowerFlags()
{
	_ForbidPowerDates.clear();
	_IneffectiveAuras.clear();
	_ForbidAuraUseStartDate = 0;
	_ForbidAuraUseEndDate = 0;
	for( uint i=0; i<32; ++i )
	{
		_PowerFlagTicks[i].StartTick = 0;
		_PowerFlagTicks[i].EndTick = 0;
	}
	_ConsumableOverdoseEndDates.clear();
	updateBrickFlagsDBEntry();
}


//------------------------------------------------------------------------------

void CCharacter::resetCombatEventFlag(BRICK_FLAGS::TBrickFlag flag)
{
	BOMB_IF( (BRICK_FLAGS::NbCombatFlags > 32), "BRICK_FLAGS::NbCombatFlags should be less than 32", return );
	BOMB_IF( (flag < 0 || flag >= BRICK_FLAGS::NbCombatFlags), "Invalid brick flag!", return );

	_CombatEventFlagTicks[flag].StartTick = 0;
	_CombatEventFlagTicks[flag].EndTick = 0;
//	_CombatEventResetDate[uint(flag)] = 0;
}


//------------------------------------------------------------------------------

void CCharacter::setCombatEventFlag(BRICK_FLAGS::TBrickFlag flag)
{
	BOMB_IF( (BRICK_FLAGS::NbCombatFlags > 32), "BRICK_FLAGS::NbCombatFlags should be less than 32", return );
	BOMB_IF( (flag < 0 || flag >= BRICK_FLAGS::NbCombatFlags), "Invalid brick flag!", return );

//	_CombatEventResetDate[uint(flag)] = CTickEventHandler::getGameCycle() + CombatFlagLifetime;

	_CombatEventFlagTicks[flag].StartTick = CTickEventHandler::getGameCycle();
	_CombatEventFlagTicks[flag].EndTick = _CombatEventFlagTicks[flag].StartTick + CombatFlagLifetime;
}


//------------------------------------------------------------------------------

void CCharacter::resetCombatEventFlags()
{
	// reset ticks for each combat flags then update DB
	for( uint i=0; i<BRICK_FLAGS::NbCombatFlags; ++i )
	{
		_CombatEventFlagTicks[i].StartTick = 0;
		_CombatEventFlagTicks[i].EndTick = 0;
	}
	updateBrickFlagsDBEntry();
}


//------------------------------------------------------------------------------

void CCharacter::updateBrickFlagsDBEntry()
{
	// combat event flags
	for( uint i=0; i<32; ++i )
	{
		CBankAccessor_PLR::getFLAGS().getBRICK_TICK_RANGE().getArray(i).setTICK_RANGE(_PropertyDatabase, (uint64(_CombatEventFlagTicks[i].EndTick)<<32) + _CombatEventFlagTicks[i].StartTick);
	}

	// powers + aura flags
	for( uint i=0; i<32; ++i )
	{
		CBankAccessor_PLR::getFLAGS().getBRICK_TICK_RANGE().getArray(i+32).setTICK_RANGE(_PropertyDatabase, (uint64(_PowerFlagTicks[i].EndTick)<<32) + _PowerFlagTicks[i].StartTick);
	}
}


//------------------------------------------------------------------------------

void CCharacter::addWearMalus( float m )
{
	_WearEquipmentMalus += m;
//	_PropertyDatabase.setProp( _DataIndexReminder->Modifiers.TotalMalusEquip, (uint32)(_WearEquipmentMalus * 50) );
	CBankAccessor_PLR::getMODIFIERS().setTOTAL_MALUS_EQUIP(_PropertyDatabase, checkedCast<uint8>(_WearEquipmentMalus * 50) );
}


//------------------------------------------------------------------------------

void CCharacter::incDodgeModifier(sint32 inc)
{
	_DodgeModifier += inc;
	_CurrentDodgeLevel = std::max(sint32(0), _BaseDodgeLevel + _DodgeModifier);
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.DodgeCurrent, _CurrentDodgeLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getDODGE().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentDodgeLevel) );
}


//------------------------------------------------------------------------------

void CCharacter::incParryModifier(sint32 inc)
{
	_ParryModifier += inc;
	_CurrentParryLevel = std::max(sint32(0), _BaseParryLevel + _ParryModifier);
//	_PropertyDatabase.setProp(_DataIndexReminder->CHARACTER_INFO.ParryCurrent,_CurrentParryLevel );
	CBankAccessor_PLR::getCHARACTER_INFO().getPARRY().setCurrent(_PropertyDatabase, checkedCast<uint16>(_CurrentParryLevel) );
}


//------------------------------------------------------------------------------

sint8 CCharacter::addEffectInDB(const NLMISC::CSheetId &sheetId, bool bonus)
{
	return _ModifiersInDB.addEffect(sheetId, bonus, _PropertyDatabase);
}


//------------------------------------------------------------------------------

void CCharacter::removeEffectInDB(uint8 index, bool bonus)
{
	_ModifiersInDB.removeEffect(index, bonus, _PropertyDatabase);
}


//------------------------------------------------------------------------------

void CCharacter::disableEffectInDB(uint8 index, bool bonus, NLMISC::TGameCycle activationDate)
{
	_ModifiersInDB.disableEffect(index, bonus, activationDate, _PropertyDatabase);
}


//------------------------------------------------------------------------------

sint32 CCharacter::getWeightMalus()
{
	sint32 maxWeight = BaseMaxCarriedWeight +  1000 * _PhysCharacs._PhysicalCharacteristics[ CHARACTERISTICS::strength ].Current;
	sint32 weightDiff = ( maxWeight - sint32(getCarriedWeight()) );
	sint32 weightMalus = weightDiff / 1000;
	if ( weightDiff % 1000 )
		weightMalus--;
	if ( weightMalus > 0)
		weightMalus = 0;
	return weightMalus;
}


//------------------------------------------------------------------------------

const CGearLatency & CCharacter::getGearLatency()
{
	nlassert(_GearLatency != NULL); return *_GearLatency;
}


//------------------------------------------------------------------------------

void CCharacter::setLastPosXInDB(sint32 x)
{
	_LastPosXInDB = x;
}


//------------------------------------------------------------------------------

void CCharacter::setLastPosYInDB(sint32 y)
{
	_LastPosYInDB = y;
}


//------------------------------------------------------------------------------

void CCharacter::clearFriendOfList()
{
	_IsFriendOf.clear();
}


//------------------------------------------------------------------------------

const NLMISC::CEntityId &CCharacter::getFriend(uint16 index) const
{
	if (index< _FriendsList.size())
		return _FriendsList[index].EntityId;
	else
		return NLMISC::CEntityId::Unknown;
}


//------------------------------------------------------------------------------

uint	CCharacter::getNumIgnored() const
{
	return (uint)_IgnoreList.size();
}


//------------------------------------------------------------------------------

const NLMISC::CEntityId &CCharacter::getIgnoredPlayer(uint16 index) const
{
	if (index< _IgnoreList.size())
		return _IgnoreList[index].EntityId;
	else
		return NLMISC::CEntityId::Unknown;
}


//------------------------------------------------------------------------------

void CCharacter::setRolemasterType(EGSPD::CSPType::TSPType type)
{
	_RolemasterType = type;
//	_PropertyDatabase.setProp("BOTCHAT:ROLEMASTER_TYPE", (sint64)type );
	CBankAccessor_PLR::getBOTCHAT().setROLEMASTER_TYPE(_PropertyDatabase, type );
}


//------------------------------------------------------------------------------

void CCharacter::setIntangibleEndDate(NLMISC::TGameCycle date)
{
	_IntangibleEndDate = date;
}


//------------------------------------------------------------------------------

void CCharacter::setWhoSeesMeBeforeTP(const uint64 &whoSeesMe)
{
	uint64 NOT_TELEPORTING_FLAG= (((uint64)0x12345678)<<32)| (uint64)0x87654321;
	BOMB_IF(_WhoSeesMeBeforeTP != NOT_TELEPORTING_FLAG && whoSeesMe != NOT_TELEPORTING_FLAG , NLMISC::toString("Failing to set _WhoSeesMeBeforeTP old value=%"NL_I64"x  new value =%"NL_I64"x", _WhoSeesMeBeforeTP, whoSeesMe), return  );
	_WhoSeesMeBeforeTP= whoSeesMe;
}


//------------------------------------------------------------------------------

void CCharacter::resetWhoSeesMeBeforeTP()
{
	uint64 NOT_TELEPORTING_FLAG= (((uint64)0x12345678)<<32)| (uint64)0x87654321;
	_WhoSeesMeBeforeTP = NOT_TELEPORTING_FLAG;
}


//------------------------------------------------------------------------------

CAdminProperties & CCharacter::getAdminProperties()
{
	nlassert(_AdminProperties != NULL); return *_AdminProperties;
}


//------------------------------------------------------------------------------

void CCharacter::setMonitoringCSR(const TDataSetRow& csr)
{
	_MonitoringCSR = csr;
}


//------------------------------------------------------------------------------

const CDeathPenalties & CCharacter::getDeathPenalties() const
{
	return *_DeathPenalties;
}


//------------------------------------------------------------------------------

void CCharacter::resetNextDeathPenaltyFactor()
{
	_NextDeathPenaltyFactor = 1.0f;
}


//------------------------------------------------------------------------------

void CCharacter::setNextDeathPenaltyFactor(float factor)
{
	_NextDeathPenaltyFactor = factor;
}


//------------------------------------------------------------------------------

void CCharacter::setDPLossDuration(float duration)
{
	_DPLossDuration = duration;
}


//------------------------------------------------------------------------------

CPlayerRoomInterface	&CCharacter::getRoomInterface()
{
	nlassert(_PlayerRoom != NULL); return *_PlayerRoom;
}


//------------------------------------------------------------------------------

void CCharacter::logXpGain(bool b)
{
	_LogXpGain = b;
}


//------------------------------------------------------------------------------

void CCharacter::decAggroCount()
{
	if ( _AggroCount != 0 )
		--_AggroCount;
}


//------------------------------------------------------------------------------

void CCharacter::incAggroCount()
{
	if ( _AggroCount != 255 )
		++_AggroCount;
}


//------------------------------------------------------------------------------

bool CCharacter::isInWater() const
{
	if (!_PlayerIsInWater && (_ActionFlags.getValue() & RYZOMACTIONFLAGS::InWater))
	{
		entersWater();
		_PlayerIsInWater = true;
	}

	return (_PlayerIsInWater || (_ActionFlags.getValue() & RYZOMACTIONFLAGS::InWater) );
}


//------------------------------------------------------------------------------

void CCharacter::updateIsInWater()
{
	// water flag use bit 2 of Z value
	_PlayerIsInWater = ( (_EntityState.Z.getValue() & 4) != 0);
}


//------------------------------------------------------------------------------

bool CCharacter::isSitting() const
{
	return ( _Mode.getValue().Mode==MBEHAV::SIT );
}


//------------------------------------------------------------------------------

void CCharacter::setBuildingExitZone(uint16 zoneIdx)
{
	_BuildingExitZone = zoneIdx;
}


//------------------------------------------------------------------------------

void CCharacter::setPetStatus( uint32 index, CPetAnimal::TStatus status )
{
	if( index < _PlayerPets.size() ) _PlayerPets[index].PetStatus = status;
}


//------------------------------------------------------------------------------

void CCharacter::petTpAllowed( uint32 index, bool allowed )
{
	if( index < _PlayerPets.size() ) _PlayerPets[index].IsTpAllowed = allowed;
}


//------------------------------------------------------------------------------

void CCharacter::setSpawnPetFlag( uint32 index )
{
	if( index < _PlayerPets.size() ) _PlayerPets[index].spawnFlag = true;
}


//------------------------------------------------------------------------------

void CCharacter::resetHairCutDiscount()
{
	_HairCuteDiscount = false;
}


//------------------------------------------------------------------------------

CCharacterEncyclopedia &CCharacter::getEncyclopedia()
{
	nlassert(_EncycloChar != NULL); return *_EncycloChar;
}


//------------------------------------------------------------------------------

CCharacterGameEvent &CCharacter::getGameEvent()
{
	nlassert(_GameEvent != NULL); return *_GameEvent;
}


//------------------------------------------------------------------------------

CCharacterRespawnPoints &CCharacter::getRespawnPoints()
{
	nlassert(_RespawnPoints != NULL); return *_RespawnPoints;
}


//------------------------------------------------------------------------------

const CCharacterRespawnPoints &CCharacter::getRespawnPoints() const
{
	nlassert(_RespawnPoints != NULL); return *_RespawnPoints;
}


//------------------------------------------------------------------------------

void CCharacter::addInQueue(uint32 id)
{
	removeFromQueue(id);
	_MissionsQueues.push_back(id);
}


//------------------------------------------------------------------------------

void CCharacter::removeFromQueue(uint32 id)
{
	const uint size = (uint)_MissionsQueues.size();
	for ( uint i = 0 ; i < size ; ++i )
	{
		if (_MissionsQueues[i] == id)
		{
			_MissionsQueues[i] = _MissionsQueues.back();
			_MissionsQueues.pop_back();
			break;
		}
	}
}


//------------------------------------------------------------------------------

void CCharacter::setEnterCriticalZoneProposalQueueId(uint32 queueId)
{
	_EnterCriticalZoneProposalQueueId = queueId;
}


//------------------------------------------------------------------------------

uint32 CCharacter::getMagicProtection( PROTECTION_TYPE::TProtectionType magicProtectionType ) const
{
	uint32 val = getUnclampedMagicProtection(magicProtectionType);
	NLMISC::clamp( val, (uint32)0, MaxMagicProtection );
	return val;
}


//------------------------------------------------------------------------------

uint32 CCharacter::getUnclampedMagicProtection( PROTECTION_TYPE::TProtectionType magicProtectionType ) const
{
	return magicProtectionType < PROTECTION_TYPE::NB_PROTECTION_TYPE ? _MagicProtection[ magicProtectionType ] : 0;
}


//------------------------------------------------------------------------------

void CCharacter::setUnclampedMagicProtection( PROTECTION_TYPE::TProtectionType magicProtectionType, sint32 value )
{
	if( magicProtectionType < PROTECTION_TYPE::NB_PROTECTION_TYPE )
	{
		_MagicProtection[ magicProtectionType ]= value;
//		_PropertyDatabase.setProp("CHARACTER_INFO:MAGIC_PROTECTION:"+PROTECTION_TYPE::toString(magicProtectionType), _MagicProtection[ magicProtectionType ]);
		CBankAccessor_PLR::getCHARACTER_INFO().getMAGIC_PROTECTION().getArray(magicProtectionType).setVALUE(_PropertyDatabase, checkedCast<uint16>(_MagicProtection[ magicProtectionType ]));
	}
	else
	{
		nlwarning("setUnclampedMagicProtection unknown protection type %d", magicProtectionType);
	}
}


//------------------------------------------------------------------------------

uint32 CCharacter::getUnclampedMagicResistance( RESISTANCE_TYPE::TResistanceType magicResistanceType ) const
{
	return magicResistanceType < RESISTANCE_TYPE::NB_RESISTANCE_TYPE ? _MagicResistance[ magicResistanceType ] : 0;
}


//------------------------------------------------------------------------------

CGameItemPtr CCharacter::getConsumedItem() const
{
	if (_ConsumedItemInventory != INVENTORIES::UNDEFINED)
		return getItem(_ConsumedItemInventory, _ConsumedItemSlot);
	else
		return NULL;
}


//------------------------------------------------------------------------------

void CCharacter::resetConsumedItem(bool unlock)
{
	if (unlock)
		unLockItem(_ConsumedItemInventory, (uint32)_ConsumedItemSlot, 1);

	_ConsumedItemInventory = INVENTORIES::UNDEFINED;
	_ConsumedItemSlot = -1;
}


//------------------------------------------------------------------------------

bool CCharacter::canUseConsumableFamily(uint16 family, NLMISC::TGameCycle &endDate)
{
	updateConsumableFamily();
	bool ret = _ConsumableOverdoseEndDates.canConsume(family, endDate);
	if(ret)
	{
		return canUsePower(POWERS::EndPower, family, endDate);
	}
	return false;
}


//------------------------------------------------------------------------------

void CCharacter::disableConsumableFamily(uint16 family, NLMISC::TGameCycle date)
{
	updateConsumableFamily();
	_ConsumableOverdoseEndDates.Dates.push_back( CConsumableOverdoseTimer(family, date) );
	if( _ConsumableOverdoseEndDates.Dates.size() <= MaxBonusMalusDisplayed )
	{
//		_PropertyDatabase.setProp( _DataIndexReminder->DisableConsumable.Family[_ConsumableOverdoseEndDates.Dates.size()-1], family );
		CBankAccessor_PLR::getDISABLE_CONSUMABLE().getArray((uint32)_ConsumableOverdoseEndDates.Dates.size()-1).setFAMILY(_PropertyDatabase, family );
//		_PropertyDatabase.setProp( _DataIndexReminder->DisableConsumable.DisableTime[_ConsumableOverdoseEndDates.Dates.size()-1], date );
		CBankAccessor_PLR::getDISABLE_CONSUMABLE().getArray((uint32)_ConsumableOverdoseEndDates.Dates.size()-1).setDISABLE_TIME(_PropertyDatabase, date );
	}
}


//------------------------------------------------------------------------------

void CCharacter::updateConsumableFamily()
{
	if( _ConsumableOverdoseEndDates.cleanVector() )
	{
		uint i;
		for(i=0; i < min((uint)MaxBonusMalusDisplayed,(uint)_ConsumableOverdoseEndDates.Dates.size()); ++i)
		{
//			_PropertyDatabase.setProp( _DataIndexReminder->DisableConsumable.Family[i], _ConsumableOverdoseEndDates.Dates[i].Family );
			CBankAccessor_PLR::getDISABLE_CONSUMABLE().getArray(i).setFAMILY(_PropertyDatabase, _ConsumableOverdoseEndDates.Dates[i].Family );
//			_PropertyDatabase.setProp( _DataIndexReminder->DisableConsumable.DisableTime[i], _ConsumableOverdoseEndDates.Dates[i].ActivationDate );
			CBankAccessor_PLR::getDISABLE_CONSUMABLE().getArray(i).setDISABLE_TIME(_PropertyDatabase, _ConsumableOverdoseEndDates.Dates[i].ActivationDate );
		}
		for(uint j=i; j < MaxBonusMalusDisplayed; ++j)
		{
//			_PropertyDatabase.setProp( _DataIndexReminder->DisableConsumable.Family[j], 0 );
			CBankAccessor_PLR::getDISABLE_CONSUMABLE().getArray(j).setFAMILY(_PropertyDatabase, 0);
//			_PropertyDatabase.setProp( _DataIndexReminder->DisableConsumable.DisableTime[j], 0 );
			CBankAccessor_PLR::getDISABLE_CONSUMABLE().getArray(j).setDISABLE_TIME(_PropertyDatabase, 0);
		}
	}
}


//------------------------------------------------------------------------------

void CCharacter::setPriviledgePVP( bool b )
{
	_PriviledgePvp = b;
}

//------------------------------------------------------------------------------

void CCharacter::setFullPVP( bool b )
{
	_FullPvp = b;
}


//------------------------------------------------------------------------------

void CCharacter::setCurrentPVPZone(TAIAlias alias)
{
	_CurrentPVPZone = alias;
}


//------------------------------------------------------------------------------

void CCharacter::setCurrentOutpostZone(TAIAlias alias)
{
	_CurrentOutpostZone = alias;
}


//------------------------------------------------------------------------------

bool CCharacter::getPvPRecentActionFlag() const
{
	return ((_PVPRecentActionTime + PVPActionTimer) > CTickEventHandler::getGameCycle());
}


//------------------------------------------------------------------------------

void CCharacter::killedInPVP()
{
	_RegionKilledInPvp = _CurrentRegion;
}


//------------------------------------------------------------------------------

void CCharacter::clearSafeInPvPSafeZone()
{
	_PvPSafeZoneActive = false;
}


//------------------------------------------------------------------------------

void CCharacter::setOutpostAliasBeforeUserValidation( TAIAlias outpostId )
{
	_OutpostIdBeforeUserValidation = outpostId;
}


//------------------------------------------------------------------------------

void CCharacter::setDuelOpponent( CCharacter * user )
{
	_DuelOpponent = user;
}


//------------------------------------------------------------------------------

void CCharacter::haveToUpdateItemsPrerequisit( bool b )
{
	_HaveToUpdateItemsPrerequisit = b;
}


//------------------------------------------------------------------------------

void CCharacter::channelAdded( bool b )
{
	_ChannelAdded = b;
}


//------------------------------------------------------------------------------

void CCharacter::setShowFactionChannelsMode(TChanID channel, bool s)
{
	_FactionChannelsMode[channel] = s;
	CPVPManager2::getInstance()->addRemoveFactionChannelToUserWithPriviledge(channel, this, s);
}


//------------------------------------------------------------------------------

void CCharacter::setEntityRowId( const TDataSetRow& r )
{
	_EntityRowId = r;
}


//------------------------------------------------------------------------------

void CCharacter::resetPvPFlag()
{
	_PVPFlag = false;
}


//------------------------------------------------------------------------------

void CCharacter::setSessionUserRole( R2::TUserRole mode )
{
	_SessionUserRole = mode;
}


//------------------------------------------------------------------------------

void CCharacter::setSessionId( TSessionId sessionId )
{
	_SessionId = sessionId; _CurrentSessionId = sessionId;
}


//------------------------------------------------------------------------------

void CCharacter::setCurrentSessionId( TSessionId sessionId )
{
	_CurrentSessionId = sessionId;
}


//------------------------------------------------------------------------------

//void CCharacter::ensureDbReminderReady()
//{
//	if ( ! _DataIndexReminder )
//		_DataIndexReminder = new CCharacterDbReminder();
//}


//------------------------------------------------------------------------------

void CCharacter::setSelectedOutpost(TAIAlias alias)
{
	_SelectedOutpost = alias;
}


//------------------------------------------------------------------------------

void CCharacter::setNpcControl(const NLMISC::CEntityId& eid)
{
	_LastTickNpcControlUpdated = CTickEventHandler::getGameCycle();
	CCreature * creature = CreatureManager.getCreature( eid  );
	if(  !creature ||  !TheDataset.isAccessible(creature->getEntityRowId()))
	{
		setStopNpcControl();
		return;
	}

	NLMISC::CSheetId sheetId = creature->getType();
	CPhysicalScores& score = creature->getPhysScores();

//	_PropertyDatabase.setProp( "USER:NPC_CONTROL:SHEET", sheetId.asInt() );
	CBankAccessor_PLR::getUSER().getNPC_CONTROL().setSHEET(_PropertyDatabase, sheetId );
//	_PropertyDatabase.setProp( "USER:NPC_CONTROL:WALK", static_cast<sint32>(score.CurrentWalkSpeed*1000) );
	CBankAccessor_PLR::getUSER().getNPC_CONTROL().setWALK(_PropertyDatabase, uint32(score.CurrentWalkSpeed*1000) );
//	_PropertyDatabase.setProp( "USER:NPC_CONTROL:RUN", static_cast<sint32>(score.CurrentRunSpeed*1000) );
	CBankAccessor_PLR::getUSER().getNPC_CONTROL().setRUN(_PropertyDatabase, uint32(score.CurrentRunSpeed*1000) );
	_NpcControlEid = eid;
}

void CCharacter::setStopNpcControl()
{
	_LastTickNpcControlUpdated = CTickEventHandler::getGameCycle();
//	_PropertyDatabase.setProp( "USER:NPC_CONTROL:SHEET", NLMISC::CSheetId::Unknown.asInt() );
	CBankAccessor_PLR::getUSER().getNPC_CONTROL().setSHEET(_PropertyDatabase, CSheetId::Unknown );
//	_PropertyDatabase.setProp( "USER:NPC_CONTROL:WALK", static_cast<sint32>(0) );
	CBankAccessor_PLR::getUSER().getNPC_CONTROL().setWALK(_PropertyDatabase, 0 );
//	_PropertyDatabase.setProp( "USER:NPC_CONTROL:RUN", static_cast<sint32>(0) );
	CBankAccessor_PLR::getUSER().getNPC_CONTROL().setRUN(_PropertyDatabase, 0 );
	_NpcControlEid = NLMISC::CEntityId::Unknown;
}

void CCharacter::craftSuccessModifier( sint32 mod )
{
	_CraftSuccessModifier = mod;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:SUCCESS_MODIFIER:CRAFT", mod );
	CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().setCRAFT(_PropertyDatabase, (sint16)mod );
}


//------------------------------------------------------------------------------

void CCharacter::meleeSuccessModifier( sint32 mod )
{
	_MeleeSuccessModifier = mod;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:SUCCESS_MODIFIER:MELEE", mod );
	CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().setMELEE(_PropertyDatabase, (sint16)mod );
}


//------------------------------------------------------------------------------

void CCharacter::rangeSuccessModifier( sint32 mod )
{
	_RangeSuccessModifier = mod;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:SUCCESS_MODIFIER:RANGE", mod );
	CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().setRANGE(_PropertyDatabase, (sint16)mod );
}


//------------------------------------------------------------------------------

void CCharacter::magicSuccessModifier( sint32 mod )
{
	_MagicSuccessModifier = mod;
//	_PropertyDatabase.setProp( "CHARACTER_INFO:SUCCESS_MODIFIER:MAGIC", mod );
	CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().setMAGIC(_PropertyDatabase, (sint16)mod );
}


//------------------------------------------------------------------------------

void CCharacter::forageSuccessModifier( ECOSYSTEM::EECosystem eco, sint32 mod )
{
	if ( eco>=0 && eco < ECOSYSTEM::unknown )
	{
		_ForageSuccessModifiers[(uint8)eco] = mod;
//		_PropertyDatabase.setProp( NLMISC::toString("CHARACTER_INFO:SUCCESS_MODIFIER:ECO:%u:FORAGE",(uint8)eco), mod );
		CBankAccessor_PLR::getCHARACTER_INFO().getSUCCESS_MODIFIER().getECO().getArray(eco).setFORAGE(_PropertyDatabase, (sint16)mod );
	}
}


// Along with Uptime, helps knowing the average frequency of queries
CVariable<uint32> NbNpcMissionGiverDescQueriesAll("egs", "NbNpcMissionGiverDescQueriesAll", "Counted for all requested NPCs", 0);
CVariable<uint32> NbNpcMissionGiverDescQueriesHavingMissions("egs", "NbNpcMissionGiverDescQueriesHavingMissions", "Counted for NPCs having missions only", 0);

void CCharacter::sendNpcMissionGiverIconDesc( const std::vector<uint32>& npcKeys )
{
	//H_AUTO(SendNpcMissionGiverIconDesc); see USRCB_CLIENT:NPC_ICON:GET_DESC
	CBitMemStream bms;
	GenericMsgManager.pushNameToStream("NPC_ICON:SET_DESC", bms);
	uint8 nb8 = (uint8)npcKeys.size();
	bms.serial( nb8 );

	for ( std::vector<uint32>::const_iterator ink=npcKeys.begin(); ink!=npcKeys.end(); ++ink )
	{
		NbNpcMissionGiverDescQueriesAll = NbNpcMissionGiverDescQueriesAll.get() + 1;

		const TAIAlias& npcAlias = *ink;
		CEntityId npcEntityId = CAIAliasTranslator::getInstance()->getEntityId( npcAlias );
		bms.serial( (uint32&)npcAlias );
		uint32 state;
		if ( npcEntityId.isUnknownId() ) // no more valid
		{
			state = NPC_ICON::NotAMissionGiver;
			bms.serial( state );
			continue;
		}

		// Check if the NPC qualifies for mission availability
		CCreature *creature = CreatureManager.getCreature( npcEntityId );
		if ( creature->getMissionVector().empty() /*||(!creature->isMissionGiverIconDisplayable())*/ )
		{
			state = NPC_ICON::NotAMissionGiver;
			bms.serial( state );
			continue;
		}

		H_AUTO(SendNpcMissionGiverIconDesc_HasMissions);
		NbNpcMissionGiverDescQueriesHavingMissions = NbNpcMissionGiverDescQueriesHavingMissions.get() + 1;

		// Report if the character can take at least one mission from the NPC, or has already taken all of them, or cannot take anyone yet
		bool hasAvailableAutoMission = false, hasAvailableListMission = false,
			 hasAutoMission = false, hasListMission = false,
			 hasAllTakenListMission = true;
		for ( uint i=0; i!=creature->getMissionVector().size(); ++i )
		{
			// Get the template
			const CMissionTemplate *templ = CMissionManager::getInstance()->getTemplate( creature->getMissionVector()[i] );
			if ( (!templ) || templ->Tags.NotProposed || templ->Tags.HideIconOnGiverNPC )
				continue;
			if ( !templ->AutoText.empty() )
			{
				hasAutoMission = true;

				// Test the mission prerequisits with this player
				H_AUTO(MissionGiverTestPrerequisitsAuto);
				uint32 res = templ->testPrerequisits(this, true);
				if ( res == MISSION_DESC::PreReqSuccess )
				{
					hasAvailableAutoMission = true;
					break; // short-cut, no need to test remaining missions
				}
				// MISSION_DESC::PreReqFailAlreadyDone:
			}
			else
			{
				hasListMission = true;

				// Test the mission prerequisits with this player
				H_AUTO(MissionGiverTestPrerequisitsList);
				uint32 res = templ->testPrerequisits(this, true);
				switch ( res )
				{
				case MISSION_DESC::PreReqSuccess:
					hasAvailableListMission = true; // still test remaining missions in case they are Auto missions
					break;
				case MISSION_DESC::PreReqFail:
					hasAllTakenListMission = false;
					break;
				default:;
				}
			}
		}
		if ( hasAvailableAutoMission )
		{
			state = NPC_ICON::AutoHasAvailableMission;
		}
		else if ( hasListMission )
		{
			if ( hasAvailableListMission ) // at least one PreReqSuccess mission
				state = NPC_ICON::ListHasAvailableMission;
			else if ( hasAllTakenListMission ) // (no PreReqSucess) and no PreReqFail mission (only
				state = NPC_ICON::ListHasAlreadyTakenMissions; // PreReqFailAlreadyDone or PreReqFailRunning)
			else // (no PreReqSuccess) and at least one PreReqFail
				state = NPC_ICON::ListHasOutOfReachMissions;
		}
		else if ( hasAutoMission ) // has neither Auto nor List available missions
		{
			state = NPC_ICON::AutoHasUnavailableMissions;
		}
		else // none of the missions need an icon
		{
			state = NPC_ICON::NotAMissionGiver;
		}
		bms.serial( state );
	}
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}


void CCharacter::sendEventForMissionAvailabilityCheck()
{
	CBitMemStream bms;
	GenericMsgManager.pushNameToStream("NPC_ICON:SVR_EVENT_MIS_AVL", bms);
	CMessage msgout( "IMPULSION_ID" );
	msgout.serial( _Id );
	msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
	CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
}


void CCharacter::sendNpcMissionGiverTimer(bool force)
{
	NLMISC::TGameCycle delayVar = (NLMISC::TGameCycle)ClientNPCIconRefreshTimerDelay.get();
	if (force || (delayVar != NPC_ICON::DefaultClientNPCIconRefreshTimerDelayGC))
	{
		CBitMemStream bms;
		GenericMsgManager.pushNameToStream("NPC_ICON:SET_TIMER", bms);
		bms.serial( delayVar );
		CMessage msgout( "IMPULSION_ID" );
		msgout.serial( _Id );
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length());
		CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(_Id.getDynamicId()), msgout );
	}
}

//------------------------------------------------------------------------------

bool CCharacter::initPetInventory(uint8 index)
{
	// init pet inventory
	const uint32 petMaxWeight = 0xFFFFFFFF; // no weight limit
	const uint32 petMaxBulk = _PlayerPets[ index ].getAnimalMaxBulk();

	const INVENTORIES::TInventory petInvId = (INVENTORIES::TInventory)(index + INVENTORIES::pet_animal);
	CPetInventory *petInventory = dynamic_cast<CPetInventory*> ((CInventoryBase*)_Inventory[petInvId]);
	if (petInventory)
	{
		petInventory->initPetInventory( index, petMaxWeight, petMaxBulk );
		return true;
	}
	return false;
}
