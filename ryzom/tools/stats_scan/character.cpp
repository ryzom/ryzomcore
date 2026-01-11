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


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "character.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


#define PERSISTENT_TOKEN_FAMILY RyzomTokenFamily


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// If the following macro is defined then the macros such as PERSISTENT_CLASS, PERSISTENT_DATA, PERSISTENT_PRE_STORE, etc
// are undefined after inclusion of perstent_data_template.h
#define PERSISTENT_MACROS_AUTO_UNDEF


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanMissionHistory
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanMissionHistory

#define PERSISTENT_DATA\
	PROP(bool, Successfull)\
	PROP(TGameCycle, LastSuccessDate)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanDeathPenalties
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanDeathPenalties

#define PERSISTENT_DATA\
	PROP(uint8,_NbDeath)\
	PROP(double,_CurrentDeathXP)\
	PROP(double,_DeathXPToGain)\
	PROP(uint32,_BonusUpdateTime)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanRespawnPoints
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanRespawnPoints

#define PERSISTENT_DATA\
	PROP_VECT(string,RespawnPoints)\


#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanFameContainerEntry
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanFameContainerEntry

#define PERSISTENT_DATA\
	PROP(sint32,Fame)\
	PROP(sint32,FameMemory)\
	PROP(string,LastFameChangeTrend)\
	
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanFameContainer
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanFameContainer

#define PERSISTENT_DATA\
	STRUCT_MAP(CSheetId,CStatsScanFameContainerEntry,_Fame)\
	
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEncyCharThema
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanEncyCharThema

#define PERSISTENT_DATA\
	PROP(uint8,ThemaState)\
	PROP(uint16,RiteTaskStatePacked)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEncyCharAlbum
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanEncyCharAlbum

#define PERSISTENT_DATA\
	PROP(uint8,AlbumState)\
	STRUCT_VECT(Themas)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanCharaterEncyclopedia
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanCharaterEncyclopedia

#define PERSISTENT_DATA\
	STRUCT_VECT(_EncyCharAlbums)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanGameEvent
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanGameEvent

#define PERSISTENT_DATA\
	PROP(uint32,_Date)\
	PROP(string,_EventFaction)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPact
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanPact

#define PERSISTENT_DATA\
	PROP(uint8,PactNature)\
	PROP(uint8,PactType)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPetAnimal
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanPetAnimal

#define PERSISTENT_DATA\
	PROP(CSheetId,TicketPetSheetId)\
	PROP(CSheetId,PetSheetId)\
	PROP(uint32,Price)\
	PROP(CEntityId,OwnerId)\
	PROP(TAIAlias,StableAlias)\
	PROP(sint32,Landscape_X)\
	PROP(sint32,Landscape_Y)\
	PROP(sint32,Landscape_Z)\
	PROP(uint32,DeathTick)\
	PROP(uint16,PetStatus)\
	PROP(sint16,Slot)\
	PROP(bool,IsFollowing)\
	PROP(bool,IsMounted)\
	PROP(bool,IsTpAllowed)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPhysCharacs
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanPhysCharacs

#define PERSISTENT_DATA\
	PROP_MAP(string,sint32,_PhysicalCharacteristics)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPhysScoresEntry
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanPhysScoresEntry

#define PERSISTENT_DATA\
	PROP(sint32,Current)\
	PROP(sint32,Base)\
	PROP(sint32,Max)\
	PROP(float,BaseRegenerateRepos)\
	PROP(float,BaseRegenerateAction)\
	PROP(float,CurrentRegenerate)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPhysScores
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanPhysScores

#define PERSISTENT_DATA\
	PROP(float,BaseWalkSpeed)\
	PROP(float,BaseRunSpeed)\
	PROP(float,CurrentWalkSpeed)\
	PROP(float,CurrentRunSpeed)\
	STRUCT_MAP(string,CStatsScanPhysScoresEntry,PhysicalScores)\

#include "game_share/persistent_data_template.h"

 
//-----------------------------------------------------------------------------
// Persistent data for CStatsScanSkillsEntry
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanSkillsEntry

#define PERSISTENT_DATA\
	PROP(sint32,Base)\
	PROP(sint32,Current)\
	PROP(sint32,MaxLvlReached)\
	PROP(double,Xp)\
	PROP(double,XpNextLvl)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanSkills
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanSkills

#define PERSISTENT_DATA\
	STRUCT_MAP(string,CStatsScanSkillsEntry,Skills)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanItemCraftParameters
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanItemCraftParameters

#define PERSISTENT_DATA\
	PROP(float,Durability)\
	PROP(float,Weight)\
	PROP(float,SapLoad)\
	PROP(float,StatEnergy)\
	PROP(float,Dmg)\
	PROP(float,Speed)\
	PROP(float,Range)\
	PROP(float,DodgeModifier)\
	PROP(float,ParryModifier)\
	PROP(float,AdversaryDodgeModifier)\
	PROP(float,AdversaryParryModifier)\
	PROP(float,ProtectionFactor)\
	PROP(float,MaxSlashingProtection)\
	PROP(float,MaxBluntProtection)\
	PROP(float,MaxPiercingProtection)\
	PROP(float,ElementalCastingTimeFactor)\
	PROP(float,ElementalPowerFactor)\
	PROP(float,OffensiveAfflictionCastingTimeFactor)\
	PROP(float,OffensiveAfflictionPowerFactor)\
	PROP(float,HealCastingTimeFactor)\
	PROP(float,HealPowerFactor)\
	PROP(float,DefensiveAfflictionCastingTimeFactor)\
	PROP(float,DefensiveAfflictionPowerFactor)\
	PROP(uint8,Color)\
	PROP(sint32,HpBuff)\
	PROP(sint32,SapBuff)\
	PROP(sint32,StaBuff)\
	PROP(sint32,FocusBuff)\
	PROP(string,Protection)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanItem
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanItem

#define PERSISTENT_DATA\
	PROP(CSheetId,_SheetId)\
	PROP(sint16,_ClientInventoryPosition)\
	PROP(uint32,_HP)\
	PROP(uint32,_Recommended)\
	PROP(CEntityId,_CreatorId)\
	PROP(uint8,_LocSlot)\
	PROP(string,_PhraseId)\
	PROP(uint16,_SlotImage)\
	PROP(sint16,_SlotCount)\
	PROP(uint32,_SapLoad)\
	PROP(bool,_Dropable)\
	PROP(bool,_Destroyable)\
	PROP(uint32,StackSize)\
	STRUCT(_CraftParameters)\
	PROP_VECT(CSheetId,_Enchantment)\
	STRUCT_VECT(Child)\
			 
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanInventory
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanInventory

#define PERSISTENT_DATA\
	STRUCT_VECT(_Item)\

		
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPlayerRoom
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanPlayerRoom

#define PERSISTENT_DATA\
	PROP(TAIAlias,Building)\
	STRUCT(Inventory)\
	PROP(sint32,Bulk)\
	PROP(sint32,MaxBulk)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEntityPosition
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanEntityPosition

#define PERSISTENT_DATA\
	PROP(sint32,X)\
	PROP(sint32,Y)\
	PROP(sint32,Z)\
	PROP(float,Heading)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEntityBase
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanEntityBase

#define PERSISTENT_DATA\
	STRUCT(_EntityPosition)\
	PROP(uint32,_SheetId)\
	PROP(string,_Name)\
	PROP(string,_Race)\
	PROP(uint8,_Gender)\
	PROP(uint8,_Size)\
	PROP(bool,_DodgeAsDefense)\
	STRUCT(_PhysCharacs)\
	STRUCT(_PhysScores)\
	STRUCT(_Skills)\
	\
	PROP(sint32,MeleeAttackModifierOnEnemy)\
	PROP(sint32,MeleeAttackModifierOnSelf)\
	PROP(sint32,MagicCastingModifierOnSelf)\
	PROP(sint32,MagicCastingModifierOnEnemy)\
	PROP(sint32,RangeAttackModifierOnEnemy)\
	PROP(sint32,RangeAttackModifierOnSelf)\
	PROP(sint32,AttackModifierOnSelf)\
	PROP(sint32,ChanceToFailStrategy)\
	PROP(sint32,ChanceToFailSpell)\
	PROP(sint32,ChanceToFailFaber)\
	PROP(sint32,ChanceToFailHarvest)\
	PROP(sint32,ChanceToFailTracking)\
	PROP(sint32,MeleeAttackSlow)\
	PROP(sint32,MeleeSlashingDamageArmor)\
	PROP(sint32,MeleeBluntDamageArmor)\
	PROP(sint32,MeleePiercingDamageArmor)\
	PROP(sint32,MeleeDamageModifierFactor)\
	PROP(sint32,RangeDamageModifierFactor)\
	PROP(sint32,CreatureMeleeTakenDamageFactor)\
	PROP(sint32,CreatureRangeTakenDamageFactor)\
	PROP(sint32,CombatBrickLatencyMultiplier)\
	PROP(sint32,MagicBrickLatencyMultiplier)\
	PROP(sint32,ArmorQualityModifier)\
	PROP(sint32,WeaponQualityModifier)\
	PROP(sint32,ArmorAbsorbtionMultiplier)\

#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// Persistent data for CCharacter
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CStatsScanCharacter

#define PERSISTENT_DATA\
	PROP(uint32,VERSION)\
	\
	PROP(uint8,	_HairType)\
	PROP(uint8,	_HairColor)\
	PROP(uint8,	_HatColor)\
	PROP(uint8,	_JacketColor)\
	PROP(uint8,	_ArmsColor)\
	PROP(uint8,	_TrousersColor)\
	PROP(uint8,	_FeetColor)\
	PROP(uint8,	_HandsColor)\
	PROP(uint64,_Money)\
	PROP(uint32,_GuildId)\
	PROP(bool,	_NewCharacter)\
	PROP(uint8, _CreationPointsRepartition)\
	PROP(uint32,_ForbidAuraUseEndDate)\
	PROP(string,_Title)\
\
	/* Visual Properties */\
	PROP(uint8,HairType)\
	PROP(uint8,HairColor)\
	PROP(uint8,GabaritHeight)\
	PROP(uint8,GabaritTorsoWidth)\
	PROP(uint8,GabaritArmsWidth)\
	PROP(uint8,GabaritLegsWidth)\
	PROP(uint8,GabaritBreastSize)\
	PROP(uint8,MorphTarget1)\
	PROP(uint8,MorphTarget2)\
	PROP(uint8,MorphTarget3)\
	PROP(uint8,MorphTarget4)\
	PROP(uint8,MorphTarget5)\
	PROP(uint8,MorphTarget6)\
	PROP(uint8,MorphTarget7)\
	PROP(uint8,MorphTarget8)\
	PROP(uint8,EyesColor)\
	PROP(uint8,Tattoo)\
	PROP(uint8,NameStringId)\
\
	PROP(bool, _HairCuteDiscount )\
	PROP(float,_NextDeathPenaltyFactor)\
\
	STRUCT(_DeathPenalties)\
	STRUCT(_PlayerRoom)\
	STRUCT(EntityBase)\
	STRUCT(RespawnPoints)\
	STRUCT(_Fames)\
	STRUCT(_EncycloChar)\
	STRUCT(_GameEvent)\
\
	PROP_VECT(CSheetId,_BoughtPhrases)\
	PROP_VECT(CSheetId,_KnownBricks)\
	PROP_VECT(CEntityId,_FriendsList)\
	PROP_VECT(CEntityId,_IgnoreList)\
	PROP_VECT(CEntityId,_IsFriendOf)\
	PROP_VECT(CEntityId,_IsIgnoredBy)\
\
	STRUCT_VECT(_Pact)\
\
	PROP_MAP(string,double,SkillPoints)\
	PROP_MAP(string,uint32,SpentSkillPoints)\
	PROP_MAP(string,sint32,ScorePermanentModifiers)\
	PROP_MAP(string,uint8,StartingCharacteristicValues)\
\
	STRUCT_MAP(TAIAlias,CStatsScanMissionHistory,_MissionHistories)\
	STRUCT_MAP(uint32,CStatsScanPetAnimal,_PlayerPets)\
	STRUCT_MAP(string,CStatsScanInventory,Inventory)\


#include "game_share/persistent_data_template.h"

