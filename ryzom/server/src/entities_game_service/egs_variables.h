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



#ifndef RY_EGS_VARIABLES_H
#define RY_EGS_VARIABLES_H

// This is the amount to multiply fame values by to get the internally stored values.
//  We store the internal values at a higher multiple so we can have higher precision when
//  adding or removing fame from a player.
#define kFameMultipler 6000

#include "nel/misc/variable.h"

void cbChangeDepositUpdateFrequency( NLMISC::IVariable& v );

extern NLMISC::CVariable<bool>	ClearAttackFlags;

/// variables used to turn off a gameplay system
extern NLMISC::CVariable<bool>					GuildSystemEnabled;
extern NLMISC::CVariable<bool>					MissionSystemEnabled;
extern NLMISC::CVariable<bool>					PackAnimalSystemEnabled;
extern NLMISC::CVariable<bool>					EnchantSystemEnabled;
extern NLMISC::CVariable<bool>					ForageSystemEnabled;
extern NLMISC::CVariable<bool>					CraftSystemEnabled;
extern NLMISC::CVariable<bool>					HarvestSystemEnabled;

/// MISC
extern NLMISC::CVariable<uint32>				DelayBeforeItemTP;
extern NLMISC::CVariable<uint32>				NbTickForRegenCreature;
extern NLMISC::CVariable<uint32>				CompassTargetUpdateFrequency;
extern NLMISC::CVariable<bool>				  	UseAsyncBSPlayerLoading;
//extern NLMISC::CVariable<bool>				  	UseBS;
//extern NLMISC::CVariable<std::string>			BackupServiceIP;
extern NLMISC::CVariable<uint32>				FameMemoryInterpolation;
extern NLMISC::CVariable<uint32>				FameTrendResetDelay;
extern NLMISC::CVariable<std::string>			CreateCharacterStartSkillsValue;
extern NLMISC::CVariable<std::string>			MessageOfTheDay;

extern NLMISC::CVariable<bool>					FightAreaEffectOn;
extern NLMISC::CVariable<bool>					MagicAreaEffectOn;
extern NLMISC::CVariable<bool>					HarvestAreaEffectOn;

extern NLMISC::CVariable<uint32>				MaxAreaTargetCount;
extern NLMISC::CVariable<bool>					CorrectInvalidPlayerPositions;

extern NLMISC::CVariable<uint32>				MountDuration;
extern NLMISC::CVariable<uint32>				UnmountDuration;

extern NLMISC::CVariable<uint32>				AreaEffectClipDistance;
extern NLMISC::CVariable<uint32>				AreaEffectClipVerticalRange;

/// TEMP until PDS is ok : Variables to configure Sadges's persistant data system
extern NLMISC::CVariable<bool>					DataPersistsAsText;
extern NLMISC::CVariable<uint32>				GuildSavingPeriod;
extern NLMISC::CVariable<uint32>				StoreSavePeriod;

// Progression and XP
extern NLMISC::CVariable<float>					MaxDistanceForXpGain;
extern NLMISC::CVariable<float>					XPTeamMemberDivisorValue;
extern NLMISC::CVariable<float>					MaxXPGainPerPlayer;
extern NLMISC::CVariable<sint32>				SkillFightValueLimiter;
extern NLMISC::CVariable<sint32>				SkillMagicValueLimiter;
extern NLMISC::CVariable<sint32>				SkillCraftValueLimiter;
extern NLMISC::CVariable<sint32>				SkillHarvestValueLimiter;

extern NLMISC::CVariable<uint32>				MagicSkillStartValue;

/// death
extern NLMISC::CVariable<double>				DeathXPFactor;
extern NLMISC::CVariable<double>				DeathXPResorptionTime;
extern NLMISC::CVariable<sint32>				DeathPenaltyMinLevel;

// comma duration before character are dead
extern NLMISC::CVariable<uint32>				CommaDelayBeforeDeath;

/// CRAFT
extern NLMISC::CVariable<float>					WearMalusCraftFactor;

///  COMBAT
extern NLMISC::CVariable<float>					MinDamage;
extern NLMISC::CVariable<float>					DamageStep;
extern NLMISC::CVariable<float>					ExponentialPower;
extern NLMISC::CVariable<float>					SmoothingFactor;
extern NLMISC::CVariable<float>					HandToHandDamage;
extern NLMISC::CVariable<uint16>				HandToHandLatency;
extern NLMISC::CVariable<float>					GlobalDebugDamageFactor;
extern NLMISC::CVariable<bool>					VerboseWorldInstance;
extern NLMISC::CVariable<bool>					EntitiesNoResist;
extern NLMISC::CVariable<bool>					EntitiesNoActionFailure;
extern NLMISC::CVariable<bool>					EntitiesNoCastBreak;

extern NLMISC::CVariable<uint16>				ReferenceWeaponLatencyForWear;

/// Item wear
extern NLMISC::CVariable<float>					DaggerWearPerAction;
extern NLMISC::CVariable<float>					SwordWearPerAction;
extern NLMISC::CVariable<float>					MaceWearPerAction;
extern NLMISC::CVariable<float>					AxeWearPerAction;
extern NLMISC::CVariable<float>					SpearWearPerAction;
extern NLMISC::CVariable<float>					StaffWearPerAction;
extern NLMISC::CVariable<float>					MagicianStaffWearPerAction;
extern NLMISC::CVariable<float>					TwoHandSwordWearPerAction;
extern NLMISC::CVariable<float>					TwoHandAxeWearPerAction;
extern NLMISC::CVariable<float>					PikeWearPerAction;
extern NLMISC::CVariable<float>					TwoHandMaceWearPerAction;

extern NLMISC::CVariable<float>					AutolauchWearPerAction;
extern NLMISC::CVariable<float>					BowrifleWearPerAction;
extern NLMISC::CVariable<float>					LauncherWearPerAction;
extern NLMISC::CVariable<float>					PistolWearPerAction;
extern NLMISC::CVariable<float>					BowpistolWearPerAction;
extern NLMISC::CVariable<float>					RifleWearPerAction;

extern NLMISC::CVariable<float>					CraftingToolWearPerAction;
extern NLMISC::CVariable<float>					ForageToolWearPerAction;
extern NLMISC::CVariable<float>					ArmorWearPerAction;
extern NLMISC::CVariable<float>					JewelryWearPerAction;
extern NLMISC::CVariable<float>					ShieldWearPerAction;
extern NLMISC::CVariable<float>					WornState1;
extern NLMISC::CVariable<float>					WornState2;
extern NLMISC::CVariable<float>					WornState3;
extern NLMISC::CVariable<float>					WornState4;

/// magic
extern NLMISC::CVariable<uint32>				NoLinkSurvivalAddTime;

extern NLMISC::CVariable<uint32>				NoLinkTimeFear;
extern NLMISC::CVariable<uint32>				NoLinkTimeSleep;
extern NLMISC::CVariable<uint32>				NoLinkTimeStun;
extern NLMISC::CVariable<uint32>				NoLinkTimeRoot;
extern NLMISC::CVariable<uint32>				NoLinkTimeSnare;
extern NLMISC::CVariable<uint32>				NoLinkTimeSlow;
extern NLMISC::CVariable<uint32>				NoLinkTimeBlind;
extern NLMISC::CVariable<uint32>				NoLinkTimeMadness;
extern NLMISC::CVariable<uint32>				NoLinkTimeDot;

extern NLMISC::CVariable<uint32>				UpdatePeriodFear;
extern NLMISC::CVariable<uint32>				UpdatePeriodSleep;
extern NLMISC::CVariable<uint32>				UpdatePeriodStun;
extern NLMISC::CVariable<uint32>				UpdatePeriodRoot;
extern NLMISC::CVariable<uint32>				UpdatePeriodSnare;
extern NLMISC::CVariable<uint32>				UpdatePeriodSlow;
extern NLMISC::CVariable<uint32>				UpdatePeriodBlind;
extern NLMISC::CVariable<uint32>				UpdatePeriodMadness;
extern NLMISC::CVariable<uint32>				UpdatePeriodDot;
extern NLMISC::CVariable<uint32>				DefaultUpdatePeriod;

extern NLMISC::CVariable<uint32>				PostCastLatency;

extern NLMISC::CVariable<uint16>				ResistIncreaseFear;
extern NLMISC::CVariable<uint16>				ResistIncreaseSleep;
extern NLMISC::CVariable<uint16>				ResistIncreaseStun;
extern NLMISC::CVariable<uint16>				ResistIncreaseRoot;
extern NLMISC::CVariable<uint16>				ResistIncreaseSnare;
extern NLMISC::CVariable<uint16>				ResistIncreaseSlow;
extern NLMISC::CVariable<uint16>				ResistIncreaseBlind;
extern NLMISC::CVariable<uint16>				ResistIncreaseMadness;
extern NLMISC::CVariable<uint16>				ResistIncreaseAcid;
extern NLMISC::CVariable<uint16>				ResistIncreaseCold;
extern NLMISC::CVariable<uint16>				ResistIncreaseElectricity;
extern NLMISC::CVariable<uint16>				ResistIncreaseFire;
extern NLMISC::CVariable<uint16>				ResistIncreasePoison;
extern NLMISC::CVariable<uint16>				ResistIncreaseRot;
extern NLMISC::CVariable<uint16>				ResistIncreaseShockwave;

extern NLMISC::CVariable<uint16>				IntangibleTimeAfterTP;
extern NLMISC::CVariable<uint32>				AurasUpdateFrequency;
extern NLMISC::CVariable<uint32>				ForageLocateDepositUpdateFrequency;
extern NLMISC::CVariable<uint32>				CharacteristicBrickStep;
extern NLMISC::CVariable<float>					RegenDivisor;
extern NLMISC::CVariable<float>					RegenReposFactor;
extern NLMISC::CVariable<float>					RegenOffset;
extern NLMISC::CVariable<uint16>				MaxCharacteristicValue;
extern NLMISC::CVariable<float>					BotDamageFactor;
extern NLMISC::CVariable<float>					HitChestStaLossFactor;
extern NLMISC::CVariable<float>					HitHeadStunDuration;
extern NLMISC::CVariable<float>					HitArmsSlowDuration;
extern NLMISC::CVariable<sint16>				HitArmsSlowFactor;
extern NLMISC::CVariable<float>					HitLegsSlowDuration;
extern NLMISC::CVariable<sint16>				HitLegsSlowFactor;
extern NLMISC::CVariable<float>					HitHandsDebuffDuration;
extern NLMISC::CVariable<sint32>				HitHandsDebuffValue;
extern NLMISC::CVariable<float>					HitFeetDebuffDuration;
extern NLMISC::CVariable<sint32>				HitFeetDebuffValue;
extern NLMISC::CVariable<uint32>				NbOpponentsBeforeMalus;
extern NLMISC::CVariable<sint32>				ModPerSupernumeraryOpponent;
extern NLMISC::CVariable<float>					ShieldingRadius;
extern NLMISC::CVariable<uint32>				CombatFlagLifetime;
extern NLMISC::CVariable<uint16>				CriticalHitChances;
extern NLMISC::CVariable<float>					DodgeFactorForMagicSkills;
extern NLMISC::CVariable<float>					DodgeFactorForForageSkills;
extern NLMISC::CVariable<float>					MaxAngleForRangeCombat;

extern NLMISC::CVariable<float>					MagicResistFactorForCombatSkills;
extern NLMISC::CVariable<float>					MagicResistFactorForMagicSkills;
extern NLMISC::CVariable<float>					MagicResistFactorForForageSkills;
extern NLMISC::CVariable<sint32>				MagicResistSkillDelta;

extern NLMISC::CVariable<uint32>				MaxMagicProtection;
extern NLMISC::CVariable<uint32>				MaxAbsorptionFactor;
extern NLMISC::CVariable<uint32>				HominBaseProtection;
extern NLMISC::CVariable<uint32>				HominRacialProtection;
extern NLMISC::CVariable<uint32>				HominRacialResistance;
extern NLMISC::CVariable<uint32>				MaxMagicResistanceBonus;
extern NLMISC::CVariable<uint32>				EcosystemResistancePenalty;

/// MISSIONS
extern NLMISC::CVariable<float>					KillAttribMinFactor;
extern NLMISC::CVariable<NLMISC::TGameCycle>	MonoMissionTimout;
extern NLMISC::CVariable<bool>					VerboseMissions;
extern NLMISC::CVariable<uint32>				TickFrequencyCompassUpdate;

extern NLMISC::CVariable<sint32>				FameByKill;

/// FORAGE
extern NLMISC::CVariable<float>					ToxicCloudDamage;
extern NLMISC::CVariable<float>					ForageExplosionDamage;
extern NLMISC::CVariable<uint32>				AutoSpawnForageSourcePeriodOverride;
extern NLMISC::CVariable<float>					ForageKamiAngerDecreasePerHour;
extern NLMISC::CVariable<float>					ForageKamiAngerOverride;
extern NLMISC::CVariable<float>					ForageKamiAngerThreshold1;
extern NLMISC::CVariable<float>					ForageKamiAngerThreshold2;
extern NLMISC::CVariable<sint32>				ForageKamiAngerPunishDamage;
extern NLMISC::CVariable<uint16>				ForageSiteStock;
extern NLMISC::CVariable<uint16>				ForageSiteNbUpdatesToLive;
extern NLMISC::CVariable<float>					ForageSiteRadius;
extern NLMISC::CVariable<uint32>				ToxicCloudUpdateFrequency;
extern NLMISC::CVariable<uint32>				DepositUpdateFrequency;
extern NLMISC::CVariable<float>					ForageQuantityBaseRate;
extern NLMISC::CVariable<float>					ForageQuantitySlowFactor;
extern NLMISC::CVariable<float>					ForageQualitySlowFactorQualityLevelRatio;
extern NLMISC::CVariable<float>					ForageQualitySlowFactorDeltaLevelRatio;
extern NLMISC::CVariable<float>					ForageQualitySlowFactorMatSpecRatio;
extern NLMISC::CVariable<float>					ForageQualitySlowFactor;
extern NLMISC::CVariable<float>					ForageQualityCeilingFactor;
extern NLMISC::CVariable<bool>					ForageQualityCeilingClamp;
extern NLMISC::CVariable<float>					ForageQuantityImpactFactor;
extern NLMISC::CVariable<float>					ForageQualityImpactFactor;
extern NLMISC::CVariable<float>					ForageExtractionAbsorptionMatSpecFactor;
extern NLMISC::CVariable<float>					ForageExtractionAbsorptionMatSpecMax;
extern NLMISC::CVariable<float>					ForageExtractionCareMatSpecFactor;
extern NLMISC::CVariable<float>					ForageExtractionAbsorptionEcoSpecFactor;
extern NLMISC::CVariable<float>					ForageExtractionAbsorptionEcoSpecMax;
extern NLMISC::CVariable<float>					ForageExtractionCareEcoSpecFactor;
extern NLMISC::CVariable<float>					ForageExtractionNaturalDDeltaPerTick;
extern NLMISC::CVariable<float>					ForageExtractionNaturalEDeltaPerTick;
extern NLMISC::CVariable<float>					ForageHPRatioPerSourceLifeImpact;
extern NLMISC::CVariable<float>					ForageCareFactor;
extern NLMISC::CVariable<float>					ForageCareSpeed;
extern NLMISC::CVariable<float>					ForageCareBeginZone;
extern NLMISC::CVariable<float>					ForageProspectionXPBonusRatio;
extern NLMISC::CVariable<float>					ForageExtractionXPFactor;
extern NLMISC::CVariable<float>					ForageExtractionNbParticipantsXPBonusRatio;
extern NLMISC::CVariable<float>					ForageExtractionNastyEventXPMalusRatio;
extern NLMISC::CVariable<float>					ForageKamiOfferingSpeed;
extern NLMISC::CVariable<uint32>				ForageReduceDamageTimeWindow;
extern NLMISC::CVariable<uint32>				ForageDebug;
extern NLMISC::CVariable<uint32>				ForageSourceSpawnDelay;
extern NLMISC::CVariable<bool>					ForageValidateSourcesSpawnPos;
extern NLMISC::CVariable<uint8>					ForageRange;
extern NLMISC::CVariable<uint8>					ForageAngle;
extern NLMISC::CVariable<uint8>					ForageLevel;
extern NLMISC::CVariable<sint32>				ForageFocusRatioOfLocateDeposit;
extern NLMISC::CVariable<float>					ForageQuantityXPDeltaLevelBonusRate;
extern NLMISC::CVariable<float>					ForageExtractionTimeMinGC;
extern NLMISC::CVariable<float>					ForageExtractionTimeSlopeGC;


// QUARTERING
extern NLMISC::CVariable<float>					QuarteringQuantityAverageForCraftHerbivore;
extern NLMISC::CVariable<float>					QuarteringQuantityAverageForCraftCarnivore;
extern NLMISC::CVariable<float>					QuarteringQuantityAverageForMissions;
extern NLMISC::CVariable<float>					QuarteringQuantityAverageForBoss5;
extern NLMISC::CVariable<float>					QuarteringQuantityAverageForBoss7;
extern NLMISC::CVariable<float>					QuarteringQuantityForInvasion5;
extern NLMISC::CVariable<float>					QuarteringQuantityForInvasion7;

// LOOT
extern NLMISC::CVariable<float>					LootMoneyAmountPerXPLevel;

/// GUILDS
extern NLMISC::CVariable<sint32>				BaseGuildBulk;
extern NLMISC::CVariable<sint16>				MinFameToBuyGuildBuilding;
extern NLMISC::CVariable<sint16>				MinFameToBuyPlayerBuilding;
extern NLMISC::CVariable<uint32>				GuildCreationCost;
extern NLMISC::CVariable<uint32>				GuildMaxMemberCount;
extern NLMISC::CVariable<NLMISC::TGameCycle>	TriggerRequestTimout;


/// OUTPOSTS (old)
extern NLMISC::CVariable<uint32>				GuildChargeSavePeriod;
extern NLMISC::CVariable<uint32>				MaxAppliedChargeCount;
extern NLMISC::CVariable<float>					OupostPowerRadius;
extern NLMISC::CVariable<uint32>				OutpostPowerDuration;

/// RANGE COMBAT
extern NLMISC::CVariable<bool>					DumpRangeAnalysis;

/// ENCHANTEMENT
extern NLMISC::CVariable<float>					RechargeMoneyFactor;
extern NLMISC::CVariable<float>					RechargeMoneyFactor;
extern NLMISC::CVariable<float>					CristalMoneyFactor;

/// PVP
extern NLMISC::CVariable<bool>					AllowPVP;
extern NLMISC::CVariable<sint32>				PVPFameRequired;
extern NLMISC::CVariable<NLMISC::TGameCycle>	DuelQueryDuration ;
extern NLMISC::CVariable<NLMISC::TGameCycle>	PVPZoneEnterBufferTime;
extern NLMISC::CVariable<NLMISC::TGameCycle>	PVPZoneLeaveBufferTime;
extern NLMISC::CVariable<NLMISC::TGameCycle>	PVPZoneWarningRepeatTime;
extern NLMISC::CVariable<NLMISC::TGameCycle>	PVPZoneWarningRepeatTimeL;
extern NLMISC::CVariable<bool>					PVPZoneWithDeathPenalty;

extern NLMISC::CVariable<float>					PVPMeleeCombatDamageFactor;
extern NLMISC::CVariable<float>					PVPRangeCombatDamageFactor;
extern NLMISC::CVariable<float>					PVPMagicDamageFactor;
//extern NLMISC::CVariable<float>					PVPMagicHealFactor;

extern NLMISC::CVariable<NLMISC::TGameCycle>	TimeForSetPVPFlag;
extern NLMISC::CVariable<NLMISC::TGameCycle>	TimeForResetPVPFlag;
extern NLMISC::CVariable<NLMISC::TGameCycle>	TimeForPVPFlagOff;
extern NLMISC::CVariable<NLMISC::TGameCycle>	PVPActionTimer;

/// BULK / WEIGHT
extern NLMISC::CVariable<uint32>				MaxPlayerBulk;
extern NLMISC::CVariable<uint32>				BaseMaxCarriedWeight;
extern NLMISC::CVariable<uint32>				BasePlayerRoomBulk;

/// GOO
extern NLMISC::CVariable<float>					MaxDistanceGooDamage;
extern NLMISC::CVariable<float>					DeathGooDistance;
extern NLMISC::CVariable<float>					MaxGooDamageRatio;
extern NLMISC::CVariable<uint32>				NBTickForGooDamageRate;
extern NLMISC::CVariable<uint32>				NBTickForNewbieGooDamageRate;
extern NLMISC::CVariable<float>					NewbieGooDamageFactor;

/// SAVE
extern NLMISC::CVariable<uint32>				TickFrequencyPCSave;
extern NLMISC::CVariable<uint32>				MinPlayerSavePeriod;
extern NLMISC::CVariable<bool>					XMLSave;
extern NLMISC::CVariable<bool>					PDRSave;
extern NLMISC::CVariable<bool>					PDRLoad;
extern NLMISC::CVariable<bool>					SerialSave;

/// TRADE 
extern NLMISC::CVariable<float>					ItemPriceCoeff0;
extern NLMISC::CVariable<float>					ItemPriceCoeff1;
extern NLMISC::CVariable<float>					ItemPriceCoeff2;
extern NLMISC::CVariable<float>					ItemPriceFactor;
extern NLMISC::CVariable<float>					AnimalSellFactor;
extern NLMISC::CVariable<float>					TeleportSellFactor;

extern NLMISC::CVariable<float>					MaxFamePriceVariation;
extern NLMISC::CVariable<sint32>				MaxFameToTrade;
extern NLMISC::CVariable<sint32>				MinFameToTrade;

extern NLMISC::CVariable<NLMISC::TGameCycle>	MaxGameCycleSaleStore;

extern NLMISC::CVariable<sint32>				MaxLevelNpcItemInStore;

extern NLMISC::CVariable<uint32>				NBMaxItemPlayerSellDisplay;
extern NLMISC::CVariable<uint32>				NBMaxItemNpcSellDisplay;
extern NLMISC::CVariable<uint32>				NBMaxItemYoursSellDisplay;

/// disconnection delay
extern NLMISC::CVariable<uint32>				TimeBeforeDisconnection;

extern NLMISC::CVariable<bool>					VerboseShopParsing;

extern NLMISC::CVariable<float>					AnimalHungerFactor;
extern NLMISC::CVariable<float>					AnimalStopFollowingDistance;

// events
extern NLMISC::CVariable<uint32>				EventChannelHistoricSize;

//outposts
extern NLMISC::CVariable<NLMISC::TGameCycle>	OutpostSavingPeriod;
extern NLMISC::CVariable<NLMISC::TGameCycle>	OutpostUpdatePeriod;
extern NLMISC::CVariable<NLMISC::TGameCycle>	OutpostLeavePeriod;

extern NLMISC::CVariable<bool>					VerboseFactionPoint;

// Newbieland
extern NLMISC::CVariable<bool>					UseNewNewbieLandStartingPoint;


// New fame system
// - Absolutes
extern NLMISC::CVariable<sint32>				FameMinToDeclare;
extern NLMISC::CVariable<sint32>				FameWarningLevel;
extern NLMISC::CVariable<sint32>				FameMinToRemain;
extern NLMISC::CVariable<sint32>				FameMinToTrade;
extern NLMISC::CVariable<sint32>				FameMinToKOS;
extern NLMISC::CVariable<sint32>				FameAbsoluteMin;
extern NLMISC::CVariable<sint32>				FameAbsoluteMax;
extern NLMISC::CVariable<sint32>				FameMaxDefault;

// - Starting values, Civilizations
extern NLMISC::CVariable<sint32>				FameStartFyrosvFyros;
extern NLMISC::CVariable<sint32>				FameStartFyrosvMatis;
extern NLMISC::CVariable<sint32>				FameStartFyrosvTryker;
extern NLMISC::CVariable<sint32>				FameStartFyrosvZorai;
extern NLMISC::CVariable<sint32>				FameStartMatisvFyros;
extern NLMISC::CVariable<sint32>				FameStartMatisvMatis;
extern NLMISC::CVariable<sint32>				FameStartMatisvTryker;
extern NLMISC::CVariable<sint32>				FameStartMatisvZorai;
extern NLMISC::CVariable<sint32>				FameStartTrykervFyros;
extern NLMISC::CVariable<sint32>				FameStartTrykervMatis;
extern NLMISC::CVariable<sint32>				FameStartTrykervTryker;
extern NLMISC::CVariable<sint32>				FameStartTrykervZorai;
extern NLMISC::CVariable<sint32>				FameStartZoraivFyros;
extern NLMISC::CVariable<sint32>				FameStartZoraivMatis;
extern NLMISC::CVariable<sint32>				FameStartZoraivTryker;
extern NLMISC::CVariable<sint32>				FameStartZoraivZorai;

// - Starting values, Cults
extern NLMISC::CVariable<sint32>				FameStartFyrosvKami;
extern NLMISC::CVariable<sint32>				FameStartFyrosvKaravan;
extern NLMISC::CVariable<sint32>				FameStartMatisvKami;
extern NLMISC::CVariable<sint32>				FameStartMatisvKaravan;
extern NLMISC::CVariable<sint32>				FameStartTrykervKami;
extern NLMISC::CVariable<sint32>				FameStartTrykervKaravan;
extern NLMISC::CVariable<sint32>				FameStartZoraivKami;
extern NLMISC::CVariable<sint32>				FameStartZoraivKaravan;

// - Max Values when declared, Civilizations
extern NLMISC::CVariable<sint32>				FameMaxNeutralvFyros;
extern NLMISC::CVariable<sint32>				FameMaxNeutralvMatis;
extern NLMISC::CVariable<sint32>				FameMaxNeutralvTryker;
extern NLMISC::CVariable<sint32>				FameMaxNeutralvZorai;
extern NLMISC::CVariable<sint32>				FameMaxFyrosvFyros;
extern NLMISC::CVariable<sint32>				FameMaxFyrosvMatis;
extern NLMISC::CVariable<sint32>				FameMaxFyrosvTryker;
extern NLMISC::CVariable<sint32>				FameMaxFyrosvZorai;
extern NLMISC::CVariable<sint32>				FameMaxMatisvFyros;
extern NLMISC::CVariable<sint32>				FameMaxMatisvMatis;
extern NLMISC::CVariable<sint32>				FameMaxMatisvTryker;
extern NLMISC::CVariable<sint32>				FameMaxMatisvZorai;
extern NLMISC::CVariable<sint32>				FameMaxTrykervFyros;
extern NLMISC::CVariable<sint32>				FameMaxTrykervMatis;
extern NLMISC::CVariable<sint32>				FameMaxTrykervTryker;
extern NLMISC::CVariable<sint32>				FameMaxTrykervZorai;
extern NLMISC::CVariable<sint32>				FameMaxZoraivFyros;
extern NLMISC::CVariable<sint32>				FameMaxZoraivMatis;
extern NLMISC::CVariable<sint32>				FameMaxZoraivTryker;
extern NLMISC::CVariable<sint32>				FameMaxZoraivZorai;

// - Max Values when declared, Cults
extern NLMISC::CVariable<sint32>				FameMaxNeutralvKami;
extern NLMISC::CVariable<sint32>				FameMaxNeutralvKaravan;
extern NLMISC::CVariable<sint32>				FameMaxKamivKami;
extern NLMISC::CVariable<sint32>				FameMaxKamivKaravan;
extern NLMISC::CVariable<sint32>				FameMaxKaravanvKami;
extern NLMISC::CVariable<sint32>				FameMaxKaravanvKaravan;

extern NLMISC::CVariable<bool>					UseFemaleTitles;

// - NPC Icons
extern NLMISC::CVariable<uint32>				ClientNPCIconRefreshTimerDelay;

// - Ring 
extern NLMISC::CVariable<uint32>				TickFrequencyNpcControlUpdate;

// scores
extern NLMISC::CVariable<sint32>				PhysicalCharacteristicsBaseValue;
extern NLMISC::CVariable<sint32>				PhysicalCharacteristicsFactor;

#endif // RY_EGS_VARIABLES_H

/* End of egs_variables.h */
