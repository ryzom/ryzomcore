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


#ifndef CHARACTER_H
#define CHARACTER_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"
#include "game_share/persistent_data.h"


//-----------------------------------------------------------------------------
// Data types
//-----------------------------------------------------------------------------

typedef uint32 TAIAlias;


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanMissionHistory
//-----------------------------------------------------------------------------

class CStatsScanMissionHistory
{
public:
	DECLARE_PERSISTENCE_METHODS

	bool Successfull;
	NLMISC::TGameCycle LastSuccessDate;

	CStatsScanMissionHistory()
	{
		Successfull=false;
		LastSuccessDate=0;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanDeathPenalties
//-----------------------------------------------------------------------------

class CStatsScanDeathPenalties
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanDeathPenalties()
	{
		_NbDeath=0;
		_CurrentDeathXP=0.0;
		_DeathXPToGain=0.0;
		_BonusUpdateTime=0;
	}

	uint8 _NbDeath;
	double _CurrentDeathXP;
	double _DeathXPToGain;
	uint32 _BonusUpdateTime;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanRespawnPoints
//-----------------------------------------------------------------------------

class CStatsScanRespawnPoints
{
public:
	DECLARE_PERSISTENCE_METHODS

	std::vector<std::string> RespawnPoints;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanFameContainerEntry
//-----------------------------------------------------------------------------

class CStatsScanFameContainerEntry
{
public:
	DECLARE_PERSISTENCE_METHODS

	sint32 Fame;
	sint32 FameMemory;
	std::string LastFameChangeTrend;

	CStatsScanFameContainerEntry()
	{
		Fame=0;
		FameMemory=0;
	}

};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanFameContainer
//-----------------------------------------------------------------------------

class CStatsScanFameContainer
{
public:
	DECLARE_PERSISTENCE_METHODS

	std::map<NLMISC::CSheetId,CStatsScanFameContainerEntry> _Fame;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEncyCharThema
//-----------------------------------------------------------------------------

class CStatsScanEncyCharThema
{
public:
	DECLARE_PERSISTENCE_METHODS

	uint8 ThemaState;
	uint16 RiteTaskStatePacked;

	CStatsScanEncyCharThema()
	{
		ThemaState=0;
		RiteTaskStatePacked=0;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEncyCharAlbum
//-----------------------------------------------------------------------------

class CStatsScanEncyCharAlbum
{
public:
	DECLARE_PERSISTENCE_METHODS

	uint8 AlbumState;
	std::vector<CStatsScanEncyCharThema> Themas;

	CStatsScanEncyCharAlbum()
	{
		AlbumState=0;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanCharaterEncyclopedia
//-----------------------------------------------------------------------------

class CStatsScanCharaterEncyclopedia
{
public:
	DECLARE_PERSISTENCE_METHODS

	std::vector<CStatsScanEncyCharAlbum> _EncyCharAlbums;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanGameEvent
//-----------------------------------------------------------------------------

class CStatsScanGameEvent
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanGameEvent()
	{
		_Date=0;
	}

	uint32 _Date;
	std::string _EventFaction;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPact
//-----------------------------------------------------------------------------

class CStatsScanPact
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanPact()
	{
		PactNature=0;
		PactType=0;
	}

	uint8 PactNature;
	uint8 PactType;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPetAnimal
//-----------------------------------------------------------------------------

class CStatsScanPetAnimal
{
public:
	DECLARE_PERSISTENCE_METHODS

	NLMISC::CSheetId TicketPetSheetId;
	NLMISC::CSheetId PetSheetId;
	uint32 Price;
	NLMISC::CEntityId OwnerId;
	TAIAlias StableAlias;
	sint32 Landscape_X;
	sint32 Landscape_Y;
	sint32 Landscape_Z;
	uint32 DeathTick;
	uint16 PetStatus;
	sint16 Slot;
	bool IsFollowing;
	bool IsMounted;
	bool IsTpAllowed;

	CStatsScanPetAnimal()
	{
		Price=0;
		StableAlias=0;
		Landscape_X=0;
		Landscape_Y=0;
		Landscape_Z=0;
		DeathTick=0;
		PetStatus=0;
		Slot=0;
		IsFollowing=false;
		IsMounted=false;
		IsTpAllowed=false;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPhysCharacs
//-----------------------------------------------------------------------------

class CStatsScanPhysCharacs
{
public:
	DECLARE_PERSISTENCE_METHODS

	std::map<std::string,sint32> _PhysicalCharacteristics;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPhysScoresEntry
//-----------------------------------------------------------------------------

class CStatsScanPhysScoresEntry
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanPhysScoresEntry()
	{
		Current=0;
		Base=0;
		Max=0;
		BaseRegenerateRepos=0.0f;
		BaseRegenerateAction=0.0f;
		CurrentRegenerate=0.0f;
	}
	
	sint32 Current;
	sint32 Base;
	sint32 Max;
	float BaseRegenerateRepos;
	float BaseRegenerateAction;
	float CurrentRegenerate;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPhysScores
//-----------------------------------------------------------------------------

class CStatsScanPhysScores
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanPhysScores()
	{
		BaseWalkSpeed=0.0f;
		BaseRunSpeed=0.0f;
		CurrentWalkSpeed=0.0f;
		CurrentRunSpeed=0.0f;
	}
	
	float BaseWalkSpeed;
	float BaseRunSpeed;
	float CurrentWalkSpeed;
	float CurrentRunSpeed;
	std::map<std::string,CStatsScanPhysScoresEntry> PhysicalScores;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanSkillsEntry
//-----------------------------------------------------------------------------

class CStatsScanSkillsEntry
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanSkillsEntry()
	{
		Base=0;
		Current=0;
		MaxLvlReached=0;
		Xp=0.0;
		XpNextLvl=0.0;
	}
	
	sint32 Base;
	sint32 Current;
	sint32 MaxLvlReached;
	double Xp;
	double XpNextLvl;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanSkills
//-----------------------------------------------------------------------------

class CStatsScanSkills
{
public:
	DECLARE_PERSISTENCE_METHODS

	std::map<std::string,CStatsScanSkillsEntry> Skills;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanItemCraftParameters
//-----------------------------------------------------------------------------

class CStatsScanItemCraftParameters
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanItemCraftParameters()
	{
		Durability=0.0f;
		Weight=0.0f;
		SapLoad=0.0f;
		StatEnergy=0.0f;
		Dmg=0.0f;
		Speed=0.0f;
		Range=0.0f;
		DodgeModifier=0.0f;
		ParryModifier=0.0f;
		AdversaryDodgeModifier=0.0f;
		AdversaryParryModifier=0.0f;
		ProtectionFactor=0.0f;
		MaxSlashingProtection=0.0f;
		MaxBluntProtection=0.0f;
		MaxPiercingProtection=0.0f;
		ElementalCastingTimeFactor=0.0f;
		ElementalPowerFactor=0.0f;
		OffensiveAfflictionCastingTimeFactor=0.0f;
		OffensiveAfflictionPowerFactor=0.0f;
		HealCastingTimeFactor=0.0f;
		HealPowerFactor=0.0f;
		DefensiveAfflictionCastingTimeFactor=0.0f;
		DefensiveAfflictionPowerFactor=0.0f;
		Color=0;
		HpBuff=0;
		SapBuff=0;
		StaBuff=0;
		FocusBuff=0;
	}

	float Durability;
	float Weight;
	float SapLoad;
	float StatEnergy;
	float Dmg;
	float Speed;
	float Range;
	float DodgeModifier;
	float ParryModifier;
	float AdversaryDodgeModifier;
	float AdversaryParryModifier;
	float ProtectionFactor;
	float MaxSlashingProtection;
	float MaxBluntProtection;
	float MaxPiercingProtection;
	float ElementalCastingTimeFactor;
	float ElementalPowerFactor;
	float OffensiveAfflictionCastingTimeFactor;
	float OffensiveAfflictionPowerFactor;
	float HealCastingTimeFactor;
	float HealPowerFactor;
	float DefensiveAfflictionCastingTimeFactor;
	float DefensiveAfflictionPowerFactor;
	uint8 Color;
	sint32 HpBuff;
	sint32 SapBuff;
	sint32 StaBuff;
	sint32 FocusBuff;
	std::string Protection;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanItem
//-----------------------------------------------------------------------------

class CStatsScanItem
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanItem()
	{
		_ClientInventoryPosition=0;
		_HP=0;
		_Recommended=0;
		_LocSlot=0;
		_SlotImage=0;
		_SlotCount=0;
		_SapLoad=0;
		_Dropable=true;
		_Destroyable=true;
		StackSize=0;
	}

	NLMISC::CSheetId _SheetId;
	sint16 _ClientInventoryPosition;
	uint32 _HP;
	uint32 _Recommended;
	NLMISC::CEntityId _CreatorId;
	uint8 _LocSlot;
	std::string _PhraseId;
	uint16 _SlotImage;
	sint16 _SlotCount;
	uint32 _SapLoad;
	bool _Dropable;
	bool _Destroyable;
	uint32 StackSize;
	CStatsScanItemCraftParameters _CraftParameters;
	std::vector<NLMISC::CSheetId> _Enchantment;
	std::vector<CStatsScanItem> Child;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanInventory
//-----------------------------------------------------------------------------

class CStatsScanInventory
{
public:
	DECLARE_PERSISTENCE_METHODS

	std::vector<CStatsScanItem> _Item;
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanPlayerRoom
//-----------------------------------------------------------------------------

class CStatsScanPlayerRoom
{
public:
	DECLARE_PERSISTENCE_METHODS

	TAIAlias Building;
	CStatsScanItem Inventory;
	sint32 Bulk;
	sint32 MaxBulk;

	CStatsScanPlayerRoom()
	{
		Building=0;
		Bulk=0;
		MaxBulk=0;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEntityPosition
//-----------------------------------------------------------------------------

class CStatsScanEntityPosition
{
public:
	DECLARE_PERSISTENCE_METHODS

	sint32 X;
	sint32 Y;
	sint32 Z;
	float Heading;

	CStatsScanEntityPosition()
	{
		X=0;
		Y=0;
		Z=0;
		Heading=0.0f;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanEntityBase
//-----------------------------------------------------------------------------

class CStatsScanEntityBase
{
public:
	DECLARE_PERSISTENCE_METHODS

	CStatsScanEntityPosition _EntityPosition;
	uint32 _SheetId;
	std::string _Name;
	std::string _Race;
	uint8 _Gender;
	uint8 _Size;
	bool _DodgeAsDefense;
	CStatsScanPhysCharacs _PhysCharacs;
	CStatsScanPhysScores _PhysScores;
	CStatsScanSkills _Skills;

	sint32 MeleeAttackModifierOnEnemy;
	sint32 MeleeAttackModifierOnSelf;
	sint32 MagicCastingModifierOnSelf;
	sint32 MagicCastingModifierOnEnemy;
	sint32 RangeAttackModifierOnEnemy;
	sint32 RangeAttackModifierOnSelf;
	sint32 AttackModifierOnSelf;
	sint32 ChanceToFailStrategy;
	sint32 ChanceToFailSpell;
	sint32 ChanceToFailFaber;
	sint32 ChanceToFailHarvest;
	sint32 ChanceToFailTracking;
	sint32 MeleeAttackSlow;
	sint32 MeleeSlashingDamageArmor;
	sint32 MeleeBluntDamageArmor;
	sint32 MeleePiercingDamageArmor;
	sint32 MeleeDamageModifierFactor;
	sint32 RangeDamageModifierFactor;
	sint32 CreatureMeleeTakenDamageFactor;
	sint32 CreatureRangeTakenDamageFactor;
	sint32 CombatBrickLatencyMultiplier;
	sint32 MagicBrickLatencyMultiplier;
	sint32 ArmorQualityModifier;
	sint32 WeaponQualityModifier;
	sint32 ArmorAbsorbtionMultiplier;

	CStatsScanEntityBase()
	{
		_DodgeAsDefense=false;
		_SheetId=0;
		_Gender=0;
		_Size=0;

		MeleeAttackModifierOnEnemy=0;
		MeleeAttackModifierOnSelf=0;
		MagicCastingModifierOnSelf=0;
		MagicCastingModifierOnEnemy=0;
		RangeAttackModifierOnEnemy=0;
		RangeAttackModifierOnSelf=0;
		AttackModifierOnSelf=0;
		ChanceToFailStrategy=0;
		ChanceToFailSpell=0;
		ChanceToFailFaber=0;
		ChanceToFailHarvest=0;
		ChanceToFailTracking=0;
		MeleeAttackSlow=0;
		MeleeSlashingDamageArmor=0;
		MeleeBluntDamageArmor=0;
		MeleePiercingDamageArmor=0;
		MeleeDamageModifierFactor=0;
		RangeDamageModifierFactor=0;
		CreatureMeleeTakenDamageFactor=0;
		CreatureRangeTakenDamageFactor=0;
		CombatBrickLatencyMultiplier=0;
		MagicBrickLatencyMultiplier=0;
		ArmorQualityModifier=0;
		WeaponQualityModifier=0;
		ArmorAbsorbtionMultiplier=0;
	}
};


//-----------------------------------------------------------------------------
// Persistent data for CStatsScanCharacter
//-----------------------------------------------------------------------------

class CStatsScanCharacter
{
public:
	DECLARE_PERSISTENCE_METHODS

	uint32 VERSION;

	uint8 _HairType;
	uint8 _HairColor;
	uint8 _HatColor;
	uint8 _JacketColor;
	uint8 _ArmsColor;
	uint8 _TrousersColor;
	uint8 _FeetColor;
	uint8 _HandsColor;
	uint64 _Money;
	uint32 _GuildId;
	bool _NewCharacter;
	uint8 _CreationPointsRepartition;
	uint32 _ForbidAuraUseEndDate;
	std::string _Title;

	/* Visual Properties */
	uint8 HairType;
	uint8 HairColor;
	uint8 GabaritHeight;
	uint8 GabaritTorsoWidth;
	uint8 GabaritArmsWidth;
	uint8 GabaritLegsWidth;
	uint8 GabaritBreastSize;
	uint8 MorphTarget1;
	uint8 MorphTarget2;
	uint8 MorphTarget3;
	uint8 MorphTarget4;
	uint8 MorphTarget5;
	uint8 MorphTarget6;
	uint8 MorphTarget7;
	uint8 MorphTarget8;
	uint8 EyesColor;
	uint8 Tattoo;
	uint8 NameStringId;

	bool _HairCuteDiscount ;
	float _NextDeathPenaltyFactor;

	CStatsScanDeathPenalties _DeathPenalties;
	CStatsScanPlayerRoom _PlayerRoom;
	CStatsScanEntityBase EntityBase;
	CStatsScanRespawnPoints RespawnPoints;
	CStatsScanFameContainer _Fames;
	CStatsScanCharaterEncyclopedia _EncycloChar;
	CStatsScanGameEvent _GameEvent;

	std::vector<NLMISC::CSheetId> _BoughtPhrases;
	std::vector<NLMISC::CSheetId> _KnownBricks;
	std::vector<NLMISC::CEntityId> _FriendsList;
	std::vector<NLMISC::CEntityId> _IgnoreList;
	std::vector<NLMISC::CEntityId> _IsFriendOf;
	std::vector<NLMISC::CEntityId> _IsIgnoredBy;

	std::vector<CStatsScanPact> _Pact;

	std::map<TAIAlias,CStatsScanMissionHistory> _MissionHistories;
	std::map<std::string,double> SkillPoints;
	std::map<std::string,uint32> SpentSkillPoints;
	std::map<std::string,sint32> ScorePermanentModifiers;
	std::map<std::string,uint8> StartingCharacteristicValues;
	std::map<std::string,CStatsScanInventory> Inventory;
	std::map<uint32,CStatsScanPetAnimal> _PlayerPets;
};


#endif