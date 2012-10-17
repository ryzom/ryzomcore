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
#include "egs_variables.h"
#include "entities_game_service.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_manager/fame_manager.h"

using namespace NLMISC;
using namespace std;

CVariable<bool>	ClearAttackFlags("egs","ClearAttackFlags","teampraire !!",true,0,true);

CVariable<bool>	GuildSystemEnabled("egs","GuildSystemEnabled","bool enabling /disabling the GuildSystem",true,0,true);
CVariable<bool>	MissionSystemEnabled("egs","MissionSystemEnabled","bool enabling /disabling the MissionSystem",true,0,true);
CVariable<bool>	PackAnimalSystemEnabled("egs","PackAnimalSystemEnabled","bool enabling /disabling the PackAnimalSystem",true,0,true);
CVariable<bool>	EnchantSystemEnabled("egs","EnchantSystemEnabled","bool enabling /disabling the EnchantSystem",true,0,true);
CVariable<bool>	ForageSystemEnabled("egs","ForageSystemEnabled","bool enabling /disabling the ForageSystem",true,0,true);
CVariable<bool>	CraftSystemEnabled("egs","CraftSystemEnabled","bool enabling /disabling the CraftSystem",true,0,true);
CVariable<bool>	HarvestSystemEnabled("egs","HarvestSystemEnabled","bool enabling /disabling the HarvestSystem",true,0,true);

CVariable<uint32>	DelayBeforeItemTP("egs","DelayBeforeItemTP","delay between item use and teleportation",150,0,true);

// XP/Progression config
CVariable<float>			MaxDistanceForXpGain("egs","MaxDistanceForXpGain", "max distance from combat to gain Xp at creature death (in meters)", 50.0f, 0, true );
CVariable<float>			XPTeamMemberDivisorValue("egs","XPTeamMemberDivisorValue", "for team XP progression, add this value to the xp divisor for each team member above one", 0.9f, 0, true );
CVariable<float>			MaxXPGainPerPlayer("egs","MaxXPGainPerPlayer", "Max XP gain by any one player on any creature (each team member can gain up to this value)", 30.0f, 0, true );

CVariable<uint32>			NbTickForRegenCreature("egs","NbTickForRegenCreature", "nb tick needed for regenerate scores of all creature (total creature / number regenerated per tick)", 64, 0, true );

// variable create Character Start skills value
CVariable<std::string>		CreateCharacterStartSkillsValue("egs","CreateCharacterStartSkillsValue", "create Character Start skills value", std::string(""), 0, true );
// variable for death penalty duration
CVariable<double>			DeathXPFactor("egs","DeathXPFactor", "XP to gain after a death is  DeathXPFactor * max skill /(1 + death index )", 10, 0, true );
// variable for death penalty resorption
CVariable<double>			DeathXPResorptionTime("egs","DeathXPResorptionTime", "XP resorbs totally after DeathXPResorptionTime / NbDeath", 3, 0, true );
// variable for min best skill value to get death penalty
CVariable<sint32>			DeathPenaltyMinLevel("egs","DeathPenaltyMinLevel", "No death penalty if best skill value is lesser than DeathPenaltyMinLevel", 10, 0, true );
// comma duration before character are dead (it's time allowed for resurect player buy a healing spell)
CVariable<uint32>			CommaDelayBeforeDeath( "egs", "CommaDelayBeforeDeath", "Comma time in server tick before a character dead", 3000, 0, true );
// variable define Max skill value can be reached, this not decrease skill value if this limit are reduce and player character have higher skills
CVariable<sint32>			SkillFightValueLimiter("egs","SkillFightValueLimiter", "Skill value limit", 250, 0, true );
CVariable<sint32>			SkillMagicValueLimiter("egs","SkillMagicValueLimiter", "Skill value limit", 250, 0, true );
CVariable<sint32>			SkillCraftValueLimiter("egs","SkillCraftValueLimiter", "Skill value limit", 250, 0, true );
CVariable<sint32>			SkillHarvestValueLimiter("egs","SkillHarvestValueLimiter", "Skill value limit", 250, 0, true );

// magic skill boost for low level chars
CVariable<uint32>			MagicSkillStartValue("egs","MagicSkillStartValue", "Minimum magic skill used for break cast and resist tests", 20, 0, true );

// variable define if areas effects are on or off
CVariable<bool>				FightAreaEffectOn("egs","FightAreaEffectOn", "Are effect for fight are active if variable = true", true, 0, true );
CVariable<bool>				MagicAreaEffectOn("egs","MagicAreaEffectOn", "Are effect for magic are active if variable = true", true, 0, true );
CVariable<bool>				HarvestAreaEffectOn("egs","HarvestAreaEffectOn", "Are effect for harvest (toxic cloud) are active if variable = true", true, 0, true );
CVariable<uint32>			MaxAreaTargetCount("egs","MaxAreaTargetCount", "maximum number of target in an area effect spell/missile", 6, 0, true );

CVariable<bool>				CorrectInvalidPlayerPositions("egs","CorrectInvalidPlayerPositions", "If true, invalid player positions will be corrected if when a player logs in", true, 0, true );

CVariable<uint32>			MountDuration("egs","MountDuration","delay in ticks between mount order and player really mounted on creature",50,0,true);
CVariable<uint32>			UnmountDuration("egs","UnmountDuration","delay in ticks between unmount order and player really unmounted from creature",30,0,true);


/// tod guild : put that to false when ben has debugged the system
CVariable<bool>				DataPersistsAsText("egs","DataPersistsAsText", "If true Persistant data will be stored as text", false, 0, true );
CVariable<uint32>			GuildSavingPeriod("egs","GuildSavingPeriod", "guild saving period in ticks", 10, 0, true );
CVariable<uint32>			StoreSavePeriod("egs","StoreSavePeriod","sell store save period in ticks", 10, 0, true );

// values used by the weapon table
CVariable<float>			MinDamage("egs","MinDamage", "Min damage (when skill 0), used by weapon damage table", 25.0f, 0, true );
CVariable<float>			DamageStep("egs","DamageStep", "Damage step, used by weapon damage table", 1.0f, 0, true );
CVariable<float>			ExponentialPower("egs","ExponentialPower", "ExponentialPower used by weapon damage table", 2.0f, 0, true );
CVariable<float>			SmoothingFactor("egs","SmoothingFactor", "SmoothingFactor used by weapon damage table", 0.8f, 0, true );
// combat config vars
CVariable<float>			HandToHandDamage("egs","HandToHandDamage", "damage factor when fighting without weapons", 0.35f, 0, true );
CVariable<uint16>			HandToHandLatency("egs","HandToHandLatency", "attacks latency *in ticks* when fighting without a weapon", 30, 0, true );
CVariable<uint16>			HandToHandReachValue("egs","HandToHandReachValue", "reach value (~allonge) when fighting without a weapon", 0, 0, true );
CVariable<uint16>			MinTwoWeaponsLatency("egs","MinTwoWeaponsLatency", "Min attack latency (in ticks) when using 2 weapons", 15, 0, true );
CVariable<float>			MaxAngleForRangeCombat("egs","MaxAngleForRangeCombat", "Max angle in Radians between player heading and target position to validate range combat", (float)NLMISC::Pi, 0, true );

CVariable<uint32>			AreaEffectClipDistance("egs","AreaEffectClipDistance", "above this value (in mm) we clip vertical range to 'AreaEffectClipVerticalRange' millimeters", 5000, 0, true );
CVariable<uint32>			AreaEffectClipVerticalRange("egs","AreaEffectClipVerticalRange", "vertical range of area effects when area effect range is below 4-5meters (see 'AreaEffectClipDistance')", 2000, 0, true );

// fame interpolation time (for player fame comming from guild)
CVariable<uint32>			FameMemoryInterpolation("egs","FameMemoryInterpolation", "Guild fame interpolation time in ticks", 4320000, 0, true );
// fame trend reset delay.
CVariable<uint32>			FameTrendResetDelay("egs","FameTrendResetDelay", "Guild fame trend reset delay in ticks", 18000, 0, true );

CVariable<uint32> AutoSpawnForageSourcePeriodOverride(
    "egs",
	"AutoSpawnForageSourcePeriodOverride",
	"Average time in ticks between two auto-spawns of forage source in a deposit with auto-spawn enabled (set to 0 to use deposit settings instead)",
	0, // using deposit settings instead
	0, true );

CVariable<float> ForageKamiAngerDecreasePerHour(
	"egs",
	"ForageKamiAngerDecreasePerHour",
	"Level of kami anger automatically decreased per hour (10x3600 gamecycles)",
	83.0f, // decrease by 1000 in about 12h
	0, true );

CVariable<float> ForageKamiAngerOverride(
	"egs",
	"ForageKamiAngerOverride",
	"Force all deposits to the specified kami anger level (debug option)",
	0,
	0, false );

CVariable<float> ForageKamiAngerThreshold1(
	"egs",
	"ForageKamiAngerThreshold1", "Threshold 1 of kami anger level (<ForageKamiAngerThreshold2)",
	950.0f,
	0, true );

CVariable<float> ForageKamiAngerThreshold2(
	"egs",
	"ForageKamiAngerThreshold2", "Threshold 1 of kami anger level (>ForageKamiAngerThreshold1)",
	1000.0f,
	0, true );

CVariable<sint32> ForageKamiAngerPunishDamage(
	"egs",
	"ForageKamiPunishDamage", "Max HP hit once by an angry (invisible) kami",
	6000,
	0, true );

/*CVariable<uint> ForageKamiAngerPunishFX(
	"KamiAngerPunishFX", "0 = Piercing; 1 = Blunt; 2 = Slashing",
	1,
	0, true );*/

CVariable<uint16> ForageSiteStock(
	"egs",
	"ForageSiteStock", "Number of extractions before a forage site is empty",
	100,
	0, true );

CVariable<uint16> ForageSiteNbUpdatesToLive(
	"egs",
	"ForageSiteNbUpdatesToLive", "Number of deposit updates before a forage site stock is reset",
	10,
	0, true );

CVariable<float> ForageSiteRadius(
	"egs",
	"ForageSiteRadius", "Radius of foage site area",
	9.0f, // diameter 18 m (as wide as three source ranges)
	0, true );

CVariable<bool>				  UseAsyncBSPlayerLoading("egs","UseAsyncBSPlayerLoading", "Use BS for Asynchrone player loading", true, 0, true );
//CVariable<string>			BackupServiceIP("egs","BSHost", "Host address and port of backup service (ip:port)", "localhost", 0, true );
//CVariable<bool>         	UseBS("egs","UseBS", "if 1, use the backup service or use local save", false, 0, true);


//#ifdef NL_DEBUG
CVariable<float>			GlobalDebugDamageFactor("egs","GlobalDebugDamageFactor", "global damage factor (debug only)", 1.0f, 0, true );
//#endif

CVariable<bool>				VerboseWorldInstance("egs","VerboseWorldInstance", "world instance activity verbose", false, 0, true );


// FOR TESTING ONLY
CVariable<bool>				EntitiesNoResist("egs","EntitiesNoResist", "change or just display the NoResist flag - true means no target resist", false ,0 , true);
CVariable<bool>				EntitiesNoActionFailure("egs","EntitiesNoActionFailure", "hange or just display the NoActionFailure flag - true means no failure", false ,0 , true);
CVariable<bool>				EntitiesNoCastBreak("egs","EntitiesNoCastBreak","change or just display the NoCastBreak flag - true means no cast break when hit", false ,0 , true);

// ITEM DECAY vars
CVariable<uint16>			ReferenceWeaponLatencyForWear("egs","ReferenceLatencyForWear","latency used as reference for weapon wear, a weapon with this latency will have a wear factor of 1.0", 20,0,true);;

CVariable<float>			DaggerWearPerAction("egs","DaggerWearPerAction","define the 'wear per action' for Dagger (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			SwordWearPerAction("egs","SwordWearPerAction","define the 'wear per action' for Sword (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			MaceWearPerAction("egs","MaceWearPerAction","define the 'wear per action' for Mace (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			AxeWearPerAction("egs","AxeWearPerAction","define the 'wear per action' for Axe (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			SpearWearPerAction("egs","SpearWearPerAction","define the 'wear per action' for Spear (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			StaffWearPerAction("egs","StaffWearPerAction","define the 'wear per action' for Staff (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			MagicianStaffWearPerAction("egs","MagicianStaffWearPerAction","define the 'wear per action' for MagicianStaff (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			TwoHandSwordWearPerAction("egs","TwoHandSwordWearPerAction","define the 'wear per action' for TwoHandSword (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			TwoHandAxeWearPerAction("egs","TwoHandAxeWearPerAction","define the 'wear per action' for TwoHandAxe (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			PikeWearPerAction("egs","PikeWearPerAction","define the 'wear per action' for Pike (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			TwoHandMaceWearPerAction("egs","TwoHandMaceWearPerAction","define the 'wear per action' for TwoHandMace (1.0 = -1 HP for each action)", 0.01f,0,true);

CVariable<float>			AutolauchWearPerAction("egs","AutolauchWearPerAction","define the 'wear per action' for Autolauch (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			BowrifleWearPerAction("egs","BowrifleWearPerAction","define the 'wear per action' for Bowrifle (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			LauncherWearPerAction("egs","LauncherWearPerAction","define the 'wear per action' for Launcher (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			PistolWearPerAction("egs","PistolWearPerAction","define the 'wear per action' for Pistol (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			BowpistolWearPerAction("egs","BowpistolWearPerAction","define the 'wear per action' for Bowpistol (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			RifleWearPerAction("egs","RifleWearPerAction","define the 'wear per action' for Rifle (1.0 = -1 HP for each action)", 0.01f,0,true);

CVariable<float>			CraftingToolWearPerAction("egs","CraftingToolWearPerAction","define the 'wear per action' for CraftingTool (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			ForageToolWearPerAction("egs","ForageToolWearPerAction","define the 'wear per action' for ForageTool (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			ArmorWearPerAction("egs","ArmorWearPerAction","define the 'wear per action' for Armor (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			JewelryWearPerAction("egs","JewelryWearPerAction","define the 'wear per action' for Jewels (1.0 = -1 HP for each action)", 0.01f,0,true);
CVariable<float>			ShieldWearPerAction("egs","ShieldWearPerAction","define the 'wear per action' for Shield (1.0 = -1 HP for each action)", 0.01f,0,true);

CVariable<float>			WornState1("egs","WornState1","define the 'WornState1' for items (0.2 = 20% of hp)", 0.25f,0,true);
CVariable<float>			WornState2("egs","WornState2","define the 'WornState2' for items (0.2 = 20% of hp)", 0.10f,0,true);
CVariable<float>			WornState3("egs","WornState3","define the 'WornState3' for items (0.2 = 20% of hp)", 0.05f,0,true);
CVariable<float>			WornState4("egs","WornState4","define the 'WornState4' for items (0.2 = 20% of hp)", 0.01f,0,true);

// MAGIC config
// add some time to dead links
CVariable<uint32>			NoLinkSurvivalAddTime("egs","NoLinkSurvivalAddTime","add some time to dead links", 0,0,true);
CVariable<uint32>			NoLinkTimeFear("egs","NoLinkTimeFear","add some time to dead Fear links", 0,0,true);
CVariable<uint32>			NoLinkTimeSleep("egs","NoLinkTimeSleep","add some time to dead Sleep links", 0,0,true);
CVariable<uint32>			NoLinkTimeStun("egs","NoLinkTimeStun","add some time to dead Stun links", 0,0,true);
CVariable<uint32>			NoLinkTimeRoot("egs","NoLinkTimeRoot","add some time to dead Root links", 0,0,true);
CVariable<uint32>			NoLinkTimeSnare("egs","NoLinkTimeSnare","add some time to dead Snare links", 0,0,true);
CVariable<uint32>			NoLinkTimeSlow("egs","NoLinkTimeSlow","add some time to dead Slow links", 0,0,true);
CVariable<uint32>			NoLinkTimeBlind("egs","NoLinkTimeBlind","add some time to dead Blind links", 0,0,true);
CVariable<uint32>			NoLinkTimeMadness("egs","NoLinkTimeMadness","add some time to dead Madness links", 0,0,true);
CVariable<uint32>			NoLinkTimeDot("egs","NoLinkTimeDot","add some time to dead Dot links", 0,0,true);

CVariable<uint32>			UpdatePeriodFear("egs","UpdatePeriodFear","update period of Fear link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodSleep("egs","UpdatePeriodSleep","update period of Sleep link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodStun("egs","UpdatePeriodStun","update period of Stun link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodRoot("egs","UpdatePeriodRoot","update period of Root link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodSnare("egs","UpdatePeriodSnare","update period of Snare link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodSlow("egs","UpdatePeriodSlow","update period of Slow link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodBlind("egs","UpdatePeriodBlind","update period of Blind link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodMadness("egs","UpdatePeriodMadness","update period of Madness link spells", 40,0,true);
CVariable<uint32>			UpdatePeriodDot("egs","UpdatePeriodDot","update period of Dot link spells", 40,0,true);
CVariable<uint32>			DefaultUpdatePeriod("egs","DefaultUpdatePeriod","default update period of link spells", 40,0,true);

// add some post cast time to magic (latency)
CVariable<uint32>			PostCastLatency("egs","PostCastLatency","add some post cast time to magic (latency in ticks)", 5,0,true);

// value added to resist value each time a spell is cast on an entity
CVariable<uint16>			ResistIncreaseFear("egs","ResistIncreaseFear","value added to Fear resist each time a Fear spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseSleep("egs","ResistIncreaseSleep","value added to Sleep resist each time a Sleep spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseStun("egs","ResistIncreaseStun","value added to Stun resist each time a Stun spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseRoot("egs","ResistIncreaseRoot","value added to Root resist each time a Root spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseSnare("egs","ResistIncreaseSnare","value added to Snare resist each time a Snare spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseSlow("egs","ResistIncreaseSlow","value added to Slow resist each time a Slow spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseBlind("egs","ResistIncreaseBlind","value added to Blind resist each time a Blind spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseMadness("egs","ResistIncreaseMadness","value added to Madness resist each time a Madness spell is cast on an entity", 5,0,true);

CVariable<uint16>			ResistIncreaseAcid("egs","ResistIncreaseAcid","value added to Acid resist each time a Acid spell is cast on an entity", 3,0,true);
CVariable<uint16>			ResistIncreaseCold("egs","ResistIncreaseCold","value added to Cold resist each time a Cold spell is cast on an entity", 3,0,true);
CVariable<uint16>			ResistIncreaseElectricity("egs","ResistIncreaseElectricity","value added to Electricity resist each time a Electricity spell is cast on an entity", 5,0,true);
CVariable<uint16>			ResistIncreaseFire("egs","ResistIncreaseFire","value added to Fire resist each time a Fire spell is cast on an entity", 3,0,true);
CVariable<uint16>			ResistIncreasePoison("egs","ResistIncreasePoison","value added to Poison resist each time a Poison spell is cast on an entity", 3,0,true);
CVariable<uint16>			ResistIncreaseRot("egs","ResistIncreaseRot","value added to Rot resist each time a Rot spell is cast on an entity", 3,0,true);
CVariable<uint16>			ResistIncreaseShockwave("egs","ResistIncreaseShockwave","value added to Shockwave resist each time a Shockwave spell is cast on an entity", 3,0,true);

// values used to determine player magic resist level
CVariable<float>			MagicResistFactorForCombatSkills("egs","MagicResistFactorForCombatSkills","factor on combat skill level to get equivalent magic resist level", 1.0f,0,true);
CVariable<float>			MagicResistFactorForMagicSkills("egs","MagicResistFactorForMagicSkills","factor on magic skill level to get equivalent magic resist level", 1.0f,0,true);
CVariable<float>			MagicResistFactorForForageSkills("egs","MagicResistFactorForForageSkills","factor on forage skill level to get equivalent magic resist level", 0.5f,0,true);
CVariable<sint32>			MagicResistSkillDelta("egs","MagicResistSkillDelta","value always added to player base magic resist", -25,0,true);

CVariable<uint32>			MaxMagicProtection("egs","MaxMagicProtection","Maximum protection a character can have against a magic damage",80,0,true);
CVariable<uint32>			MaxAbsorptionFactor("egs","MaxAbsorptionFactor","Maximum magic damage absorption factor, 100 = factor 1.0f, base absorption is sum of jewel level", 100,0,true);
CVariable<uint32>			HominBaseProtection("egs","HominBaseProtection","Homin base protection against magic common damage (cold, acid, rot)", 10,0,true);
CVariable<uint32>			HominRacialProtection("egs","HominRacialProtection","Homin base protection against magic racial damage (Fire, Poison, Shockwave, Electricity)", 20,0,true);
CVariable<uint32>			HominRacialResistance("egs","HominRacialResistance","Homin base resistance against magic racial spell type", 10,0,true);
CVariable<uint32>			MaxMagicResistanceBonus("egs","MaxMagicResistanceBonus","Max bonus resistance against magic spell type", 50,0,true);
CVariable<uint32>			EcosystemResistancePenalty("egs","EcosystemResistancePenalty","Resistance Penalty gived by each ecosystem", 10,0,true);

// intangible time after a teleport or a respawn
CVariable<uint16>			IntangibleTimeAfterTP("egs","IntangibleTimeAfterTP","intangible time after a teleport or a respawn (in ticks)", 30,0,true);

// update frequency of the compass target position (in ticks)
CVariable<uint32>			CompassTargetUpdateFrequency("egs","CompassTargetUpdateFrequency", " update frequency of the compass target position (in ticks)", 20, 0, true);

// update period of toxic cloud damaging (in ticks)
CVariable<uint32>			ToxicCloudUpdateFrequency("egs","ToxicCloudUpdateFrequency", "update frequency of toxic cloud damaging (in ticks)", 20, 0, true);

// update period of deposits (in ticks) (should not be frequent; 0 is forbidden)
CVariable<uint32>			DepositUpdateFrequency("egs","DepositUpdateFrequency", "update frequency of deposits (in ticks)", 300, 0, true, cbChangeDepositUpdateFrequency); // default: 30 s

// update period of auras effects (to determine chich entities are affected for instance) (in ticks)
CVariable<uint32>			AurasUpdateFrequency("egs","AurasUpdateFrequency", "update frequency of auras effects (to determine which entities are affected for instance) (in ticks)", 20, 0, true);

// update frequency of locate deposit effect
CVariable<uint32>			ForageLocateDepositUpdateFrequency("egs","ForageLocateDepositUpdateFrequency", "update frequency of locate deposit effect", 25, 0, true );

// characteristic brick progression step
CVariable<uint32>			CharacteristicBrickStep("egs","CharacteristicBrickStep", "characteristic brick progression step(brick xxxx05 match a charac value of characteristic 5*CharacteristicBrickStep)", 5, 0, true);

// Variable regen divisor & factor
CVariable<float>			RegenDivisor("egs","RegenDivisor", "Divisor used for compute regenerate per seconde in repos (divide characteristics per divisor)", 16.0f, 0, true, CPlayerService::updateRegen );
CVariable<float>			RegenReposFactor("egs","RegenReposFactor", "Divisor used for compute regenerate per seconde in action (regen repos / factor)", 2.0f, 0, true, CPlayerService::updateRegen );
CVariable<float>			RegenOffset("egs","RegenOffset", "Regen offset", 1.6f, 0, true, CPlayerService::updateRegen );

// Max value for player's characteristics
CVariable<uint16>			MaxCharacteristicValue("egs","MaxCharacteristicValue", "Max value for player's characteristics", 250, 0, true);

// COMBAT config
// factor on creature and npc damage
CVariable<float>			BotDamageFactor("egs","BotDamageFactor", "Factor applied on all Bots (creature and npcs) damage", 1.0f, 0, true );

CVariable<float>			HitChestStaLossFactor("egs","HitChestStaLossFactor", "factor of damage also lost in sta when hit to chest", 0.5f, 0, true );
CVariable<float>			HitHeadStunDuration("egs","HitHeadStunDuration", "duration (in seconds) of a stun when hit to head", 2.5f, 0, true );
CVariable<float>			HitArmsSlowDuration("egs","HitArmsSlowDuration", "duration (in seconds) of a slow attack when hit to arms", 5.0f, 0, true );
CVariable<sint16>			HitArmsSlowFactor("egs","HitArmsSlowFactor", "slowing factor when hit to arms (+20 = +20% to attack latency)", 50, 0, true );
CVariable<float>			HitLegsSlowDuration("egs","HitLegsSlowDuration", "duration (in seconds) of a slow move when hit to legs", 5.0f, 0, true );
CVariable<sint16>			HitLegsSlowFactor("egs","HitLegsSlowFactor", "slowing factor when hit to legs (-20 = -20% to move speed)", -20, 0, true );
CVariable<float>			HitHandsDebuffDuration("egs","HitHandsDebuffDuration", "duration in seconds of skills debuff when hit to hands", 5.0f, 0, true );
CVariable<sint32>			HitHandsDebuffValue("egs","HitHandsDebuffValue", "value of skills debuff when hit to hands ", -20, 0, true );
CVariable<float>			HitFeetDebuffDuration("egs","HitFeetDebuffDuration", "duration in seconds of dodge debuff when hit to feet", 5.0f, 0, true );
CVariable<sint32>			HitFeetDebuffValue("egs","HitFeetDebuffValue", "value of skills dodge when hit to hands ", -20, 0, true );

CVariable<uint32>			NbOpponentsBeforeMalus("egs","NbOpponentsBeforeMalus", "number of opponent one can handle without defense malus", 1, 0, true );
CVariable<sint32>			ModPerSupernumeraryOpponent("egs","ModPerSupernumeraryOpponent", "modifier on defense (parry/dodge) per opponent after 'n'", -5, 0, true );
CVariable<float>			ShieldingRadius("egs","ShieldingRadius", "effective radius of the 'shielding' power", 5.0f, 0, true );

CVariable<uint32>			CombatFlagLifetime("egs","CombatFlagLifetime", "time *in ticks* during which a combat flag is kept 'active'", 50, 0, true );

CVariable<uint16>			CriticalHitChances("egs","CriticalHitChances", "chances (in%) to make a critical hit during a combat action (5 means 5%)", 5, 0, true );

CVariable<float>			DodgeFactorForMagicSkills("egs","DodgeFactorForMagicSkills", "Factor aplied on magic skill values to determine the equivalent for dodge", 1.0f, 0, true );
CVariable<float>			DodgeFactorForForageSkills("egs","DodgeFactorForForageSkills", "Factor aplied on forage skill values to determine the equivalent for dodge", 0.5f, 0, true );

CVariable<float> ForageExtractionTimeMinGC( "egs", "ForageExtractionTimeMinGC", "Minimum time of extraction in ticks", 230.0f, 0, true );
CVariable<float> ForageExtractionTimeSlopeGC( "egs", "ForageExtractionTimeSlopeGC", "Slope of base extraction time curve", 2.0f, 0, true );


CVariable<float> ForageQuantityBaseRate( "egs", "ForageQuantityBaseRate", "Base of extraction rate", 0.23f, 0, true ); // 0.23 doubles the previous minimum setting

// ForageQuantitySlowFactor=0.5 => Results: 100% of rate at extraction #4
CVariable<float> ForageQuantitySlowFactor( "egs","ForageQuantitySlowFactor",
										  "Factor for slowness of quantity growth", 0.5f,
										  0, true );

// ForageQualitySlowFactor=1.69 and ForageQualityCeilingFactor=1.1
CVariable<float> ForageQualitySlowFactor( "egs","ForageQualitySlowFactor",
										 "Factor for slowness of quality growth", 1.69f,
										 0, true );
CVariable<float> ForageQualitySlowFactorQualityLevelRatio( "egs", "ForageQualitySlowFactorQualityLevelRatio",
											"Each quality level in extraction action slows the quality raising from this amount", 0.01f,
											0, true );
CVariable<float> ForageQualitySlowFactorDeltaLevelRatio( "egs", "ForageQualitySlowFactorDeltaLevelRatio",
											"Each delta between quality level in extraction action and player's level slows the quality raising from this amount", 0.08f,
											0, true );
CVariable<float> ForageQualitySlowFactorMatSpecRatio( "egs", "ForageQualitySlowFactorMatSpecRatio",
											"Material specialization in extraction speeds up the quality raising from this amount", 0.80f,
											0, true );
CVariable<float> ForageQualityCeilingFactor( "egs","ForageQualityCeilingFactor",
											"Factor for ceiling of quality growth", 1.1f,
											0, true );
CVariable<bool> ForageQualityCeilingClamp( "egs","ForageQualityCeilingClamp",
										  "Prevent to go past the ceiling", true,
										  0, true );

// Impact on negative values
CVariable<float> ForageQuantityImpactFactor( "egs","ForageQuantityImpactFactor",
											"Factor for extraction impact", 20.0f,
											0, true );
CVariable<float> ForageQualityImpactFactor( "egs","ForageQualityImpactFactor",
										   "Factor for extraction impact", 1.5f,
										   0, true );

CVariable<float> ForageExtractionAbsorptionMatSpecFactor( "egs", "ForageExtractionAbsorptionMatSpecFactor", "Material specialization reduces harmfulness", 4.0f, 0, true );
CVariable<float> ForageExtractionAbsorptionMatSpecMax( "egs", "ForageExtractionAbsorptionMatSpecMax", "Material specialization reduces harmfulness", 0.8f, 0, true );
CVariable<float> ForageExtractionCareMatSpecFactor( "egs", "ForageExtractionCareMatSpecFactor", "Material specialization raises care", 1.2f, 0, true );

CVariable<float> ForageExtractionAbsorptionEcoSpecFactor( "egs", "ForageExtractionAbsorptionEcoSpecFactor", "Ecosystem specialization reduces harmfulness", 3.0f, 0, true );
CVariable<float> ForageExtractionAbsorptionEcoSpecMax( "egs", "ForageExtractionAbsorptionEcoSpecMax", "Ecosystem specialization reduces harmfulness", 0.8f, 0, true );
CVariable<float> ForageExtractionCareEcoSpecFactor( "egs", "ForageExtractionCareEcoSpecFactor", "Ecosystem specialization raises care", 1.1f, 0, true );

CVariable<float> ForageExtractionNaturalDDeltaPerTick( "egs", "ForageExtractionNaturalDDeltaPerTick", "D increase per tick", 0.1f, 0, true );
CVariable<float> ForageExtractionNaturalEDeltaPerTick( "egs", "ForageExtractionNaturalEDeltaPerTick", "E increase per tick", 0.1f, 0, true );
CVariable<float> ForageHPRatioPerSourceLifeImpact( "egs", "HPRatioPerSourceLifeImpact", "Ratio of HP damaged per source life (D) impact value", 0.003937f, 0, true ); // 1/127 / 2
CVariable<float> ForageCareFactor( "egs","ForageCareFactor","Factor for care delta", 4.0f,0, true );

CVariable<float> ForageCareBeginZone( "egs", "ForageCareBeginZone", "", 5.0f, 0, true );
CVariable<float> ForageProspectionXPBonusRatio( "egs", "ForageProspectionXPBonusRatio", "", 0.5f, 0, true );
CVariable<float> ForageExtractionXPFactor( "egs", "ForageExtractionXPFactor", "", 5.0f, 0, true );
CVariable<float> ForageExtractionNbParticipantsXPBonusRatio( "egs", "ForageExtractionNbParticipantsXPBonusRatio", "", 0.1f, 0, true );
CVariable<float> ForageExtractionNastyEventXPMalusRatio( "egs", "ForageExtractionNastyEventXPMalusRatio", "", 0.1f, 0, true );

CVariable<float> ToxicCloudDamage( "egs","ToxicCloudDamage", "Max HP hit by a toxic cloud at ToxicCloudUpdateFrequency", 800.0f, true );
CVariable<float> ForageExplosionDamage( "egs","ForageExplosionDamage", "Max HP hit once by a forage explosion", 4000.0f, true );

CVariable<sint32> FameByKill( "egs","FameByKill", "Number of fame point lost for a kill", -5000, true );

CVariable<sint32> PVPFameRequired ("egs","PVPFameRequired", "Minimum of positive or negative fame for PVP", 25, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle>DuelQueryDuration ("egs","DuelQueryDuration", "duration in ticks of a duel requests", 600, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle> PVPZoneEnterBufferTime ("egs","PVPZoneEnterBufferTime", "duration in ticks of the time buffer triggered when someone enters a PVP zone", 300, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle> PVPZoneLeaveBufferTime ("egs","PVPZoneLeaveBufferTime", "duration in ticks of the time buffer triggered when someone leaves a PVP zone", 9000, 0, true );

NLMISC::CVariable<NLMISC::TGameCycle> PVPZoneWarningRepeatTime ("egs","PVPZoneWarningRepeatTime", "duration in ticks used to repeat warnings of PVP zones", 50, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle> PVPZoneWarningRepeatTimeL ("egs","PVPZoneWarningRepeatTimeL", "duration in ticks used to repeat warnings of PVP zones for time left is more than one minute", 50, 0, true );
NLMISC::CVariable<bool> PVPZoneWithDeathPenalty("egs","PVPZoneWithDeathPenalty", "if true a player killed by another player in a PVP zone will have a death penalty", false, 0, true );

NLMISC::CVariable<float> PVPMeleeCombatDamageFactor("egs","PVPMeleeCombatDamageFactor", "Factor applied on Melee Damage in PVP", 0.8f, 0, true );
NLMISC::CVariable<float> PVPRangeCombatDamageFactor("egs","PVPRangeCombatDamageFactor", "Factor applied on Range Damage in PVP", 0.8f, 0, true );
NLMISC::CVariable<float> PVPMagicDamageFactor("egs","PVPMagicDamageFactor", "Factor applied on Magic Damage in PVP", 0.8f, 0, true );
//NLMISC::CVariable<float> PVPMagicHealFactor("egs","PVPMagicHealFactor", "Factor applied on heals in PVP", 0.8f, 0, true );

NLMISC::CVariable<NLMISC::TGameCycle> TimeForSetPVPFlag("egs","TimeForSetPVPFlag", "time before set pvp flag become effective (in game cycle)", 1200, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle> TimeForResetPVPFlag("egs","TimeForResetPVPFlag", "minimal time before reset pvp flag (in game cycle)", 18000, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle> TimeForPVPFlagOff("egs","TimeForPVPFlagOff", "time before pvp flag return to off become effective (in game cycle)", 300, 0, true );
NLMISC::CVariable<NLMISC::TGameCycle> PVPActionTimer("egs","PVPActionTimer", "time duration of pvp action flag, each time a character made a pvp action, corresponding flag is true during this timer", 6000, 0, true );


NLMISC::CVariable<float> KillAttribMinFactor("egs","KillAttribMinFactor", "min fraction of the total damage done on a creature that a group/player must do to be attributed a kill", 0.3f, 0, true);


CVariable<sint32> BaseGuildBulk("egs","BaseGuildBulk", "base bulk for a new guild building", 2000000, 0, true);
CVariable<sint16> MinFameToBuyGuildBuilding("egs","MinFameToBuyGuildBuilding", "Minimum Fame To Buy a Guild Building", 0, 0, true);
CVariable<sint16> MinFameToBuyPlayerBuilding("egs","MinFameToBuyPlayerBuilding", "Minimum Fame To Buy a Player Building", 0, 0, true);
CVariable<uint32> GuildCreationCost("egs","GuildCreationCost", "", 100, 0, true);
CVariable<uint32> GuildMaxMemberCount("egs","GuildMaxMemberCount", "", 256, 0, true);


CVariable<NLMISC::TGameCycle> TriggerRequestTimout("egs","TriggerRequestTimout", "", 300, 0, true);

CVariable<uint32> GuildSavePeriod("egs","GuildSavePeriod", "", 100, 0, true);



CVariable<TGameCycle> MonoMissionTimout("egs","MonoMissionTimout", "default timout of mono mission", 144000, 0, true);
CVariable<bool>	VerboseMissions("egs","VerboseMissions", "Logging of mission parsing/event (0/1)", false, 0, true);
CVariable<bool>	MissionPrerequisitsEnabled("egs","MissionPrerequisitsEnabled", "Activate the checking of pre requisits for mission", true, 0, true);

CVariable<uint32> TickFrequencyCompassUpdate("egs","TickFrequencyCompassUpdate","Min time in tick between 2 updates of mission compass of a character", 32, 0, true);
CVariable<float> WearMalusCraftFactor("egs","WearMalusCraftFactor", "Factor for apply wear penalty to craft", 0.1f, 0, true );

CVariable<float> ForageCareSpeed( "egs","ForageCareSpeed",
								 "Speed of care action in action/tick", 0.05f, // 2 s
								 0, true );
CVariable<float> ForageKamiOfferingSpeed( "egs","ForageKamiOfferingSpeed",
										 "Speed of offering action in action/tick", 0.02f, // 5 s
									 	0, true );
CVariable<uint32> ForageReduceDamageTimeWindow( "egs", "ForageReduceDamageTimeWindow", "Time in tick before a blowing up when Reduce Damage actions are taken into account", 30, 0, true ); // 3 s

CVariable<uint32> ForageDebug( "egs","ForageDebug",
							  "Debug mode for forage phrases (0=normal 2=focusOnly 5=focus+skill 7=prospectionWithVariables 10=fullCheat)", 0,
							  0, true );
CVariable<uint32> ForageSourceSpawnDelay( "egs","ForageSourceSpawnDelay",
										 "Time (ticks) between end of prospection & spawning the source(s)",
										 50, 0, true );
CVariable<bool> ForageValidateSourcesSpawnPos( "egs", "ForageValidateSourcesSpawnPos",
										 "If true, the AIS checks all source spawn positions",
										 true, 0, true );

CVariable<uint8> ForageRange( "egs","ForageRange", "", 0, 0, false );
CVariable<uint8> ForageAngle( "egs","ForageAngle", "", 0, 0, false );
CVariable<uint8> ForageLevel( "egs","ForageLevel", "", 0, 0, false );
CVariable<sint32> ForageFocusRatioOfLocateDeposit( "egs", "ForageFocusRatioOfLocateDeposit", "", 10, 0, true );
CVariable<float> ForageQuantityXPDeltaLevelBonusRate( "egs", "ForageQuantityXPDeltaLevelBonusRate", "Delta level increase per qtty unit (except 1st one)", 2.0f, 0, true );

CVariable<float> QuarteringQuantityAverageForCraftHerbivore( "egs", "QuarteringQuantityAverageForCraftHerbivore", "Control the global quantity in quartering for craft RMs of herbivores (H/B/P)", 1.0f, 0, true );
CVariable<float> QuarteringQuantityAverageForCraftCarnivore( "egs", "QuarteringQuantityAverageForCraftCarnivore", "Control the global quantity in quartering for craft RMs of carnivores", 5.0f, 0, true );
CVariable<float> QuarteringQuantityAverageForMissions( "egs", "QuarteringQuantityAverageForMissions", "Control the global quantity in quartering for mission RMs", 5.0f, 0, true );
CVariable<float> QuarteringQuantityAverageForBoss5( "egs", "QuarteringQuantityAverageForBoss5", "Control the quantity of quartered material of normal creatures '5' and '8'", 10, 0, true );
CVariable<float> QuarteringQuantityAverageForBoss7( "egs", "QuarteringQuantityAverageForBoss7", "Control the quantity of quartered material of normal creatures '7'", 40, 0, true );
CVariable<float> QuarteringQuantityForInvasion5( "egs", "QuarteringQuantityForInvasion5", "Control the quantity of quartered invasion material of creatures '5'", 40, 0, true );
CVariable<float> QuarteringQuantityForInvasion7( "egs", "QuarteringQuantityForInvasion7", "Control the quantity of quartered invasion material of creatures '7'", 80, 0, true );

CVariable<float> LootMoneyAmountPerXPLevel( "egs", "LootMoneyAmountPerXPLevel", "Amount of money to earn when looting, per XPLevel of the looted NPC", 10.0f, 0, true );


CVariable<uint32> GuildChargeSavePeriod("egs","GuildChargeSavePeriod", "", 100, 0, true);
CVariable<uint32> MaxAppliedChargeCount("egs","AppliedChargeMaxCount", "max number of charge a guild can apply for", 1, 0, true);
CVariable<float> OupostPowerRadius("egs","OupostPowerRadius", "Radius of an outpost power in meters", 100, 0, true);
CVariable<uint32> OutpostPowerDuration("egs","OutpostPowerDuration", "duration of outpost powers in ticks", 54000, 0, true);



CVariable<bool> DumpRangeAnalysis("egs","DumpRangeAnalysis","debug dump for range combat",false,0,true);
CVariable<float> RechargeMoneyFactor("egs","RechargeMoneyFactor", "factor to apply on a recharge spell sabrina cost to get the cost in money", 2.0f, 0, true );
CVariable<float> CristalMoneyFactor("egs","CristalMoneyFactor", "factor to apply on a cristallisation spell sabrina cost to get the cost in money", 2.0f, 0, true );

CVariable<bool> AllowPVP("egs","AllowPVP", "AllowPVP", false ); // free PVP everywhere!


NLMISC::CVariable<uint32> BasePlayerRoomBulk("egs","BasePlayerRoomBulk", "Maximum bulk a player can have in his room", 500000, 0, true);
NLMISC::CVariable<uint32> MaxPlayerBulk("egs","MaxPlayerBulk", "Maximum bulk a player can have on him", 100000, 0, true);
NLMISC::CVariable<uint32> BaseMaxCarriedWeight("egs","BaseMaxCarriedWeight", "max weight in grammes a player can have on him if his strength is 0", 100000, 0, true);
NLMISC::CVariable<std::string> MessageOfTheDay("egs","MessageOfTheDay", "Message of the day sended at new connected player", string(""), 0, true, broadcastMessageOfTheDay);

NLMISC::CVariable<float> MaxDistanceGooDamage("egs","MaxDistanceGooDamage", "the maximum distance when character take damage from goo path", 100.0f, 0, true );
NLMISC::CVariable<float> DeathGooDistance("egs","DeathGooDistance", "the distance when character take 100% damage from goo path", 1.0f, 0, true );
NLMISC::CVariable<float> MaxGooDamageRatio("egs","MaxGooDamageRatio", "the factor applied goo damage for calculate effective goo damage (distance = 0 - 100% * ratio * base hp = damages)", 0.5f, 0, true );
NLMISC::CVariable<uint32> NBTickForGooDamageRate("egs","NBTickForGooDamageRate", "nb tick needed for apply goo damage again", 5, 0, true );
NLMISC::CVariable<uint32> NBTickForNewbieGooDamageRate("egs","NBTickForNewbieGooDamageRate", "nb tick needed for apply goo damage again in newbieland", 10, 0, true );
NLMISC::CVariable<float> NewbieGooDamageFactor("egs","NewbieGooDamageFactor", "percentage of life lost at maximum from goo in newbieland", .2f, 0, true );


NLMISC::CVariable<uint32> TickFrequencyPCSave("loadSave","TickFrequencyPCSave", "A PC is saved in each nb tick for prevent overrun backup system", 100, 0, true );
NLMISC::CVariable<uint32> MinPlayerSavePeriod("loadSave","MinPlayerSavePeriod", "The same PC cant be saved more than once during MinPlayerSavePeriod ticks", 600, 0, true );
NLMISC::CVariable<bool> XMLSave("loadSave","XMLSave", "boolean : if true players are saved in XML format", true, 0, true );
NLMISC::CVariable<bool> PDRSave("loadSave","PDRSave", "boolean : if true players are saved in PDR format", true, 0, true );
NLMISC::CVariable<bool> PDRLoad("loadSave","PDRLoad", "boolean : if true players are loaded from PDR format", false, 0, true );
NLMISC::CVariable<bool> SerialSave("loadSave","SerialSave", "boolean : if true players are saved in serial format", false, 0, true );

CVariable<float> ItemPriceCoeff0("egs","ItemPriceCoeff0", "polynom coeff of degree 0 in the price formula", 1.0f, 0, true );
CVariable<float> ItemPriceCoeff1("egs","ItemPriceCoeff1", "polynom coeff of degree 1 in the price formula", 1.0f, 0, true );
CVariable<float> ItemPriceCoeff2("egs","ItemPriceCoeff2", "polynom coeff of degree 2 in the price formula", 1.0f, 0, true );
CVariable<float> ItemPriceFactor("egs","ItemPriceFactor", "factor to apply on non raw material items to compute their price", 1.0f, 0, true );

CVariable<float> AnimalSellFactor("egs","AnimalSellFactor", "factor to apply on animal price to get the price a user can buy them", 1.0f, 0, true );
CVariable<float> TeleportSellFactor("egs","TeleportSellFactor", "factor to apply on teleport price to get the price a user can buy them", 1.0f, 0, true );

CVariable<float> MaxFamePriceVariation("egs","MaxFamePriceVariation", "maximum price variation ( in absolute value ) that can be due to fame", 0.3f, 0, true );
CVariable<sint32> MaxFameToTrade("egs","MaxFameToTrade", "maximum fame value taken in account in trade", 600000, 0, true );
CVariable<sint32> MinFameToTrade("egs","MinFameToTrade", "minimum fame value taken in account in trade", -200000, 0, true );

CVariable<NLMISC::TGameCycle> MaxGameCycleSaleStore("egs", "MaxGameCycleSaleStore", "maximum time game cycle an item stay in sale store before been destroyed", 7*24*60*60*10, 0, true );

CVariable<sint32>	MaxLevelNpcItemInStore("egs","MaxLevelNpcItemInStore", "Maximum level for item solded by NPC", 250, 0, true );

CVariable<bool>		VerboseShopParsing("egs","VerboseShopParsing", "activate verbose mode for shop category parsing", false, 0, true );

CVariable<uint32>	NBMaxItemPlayerSellDisplay("egs","NBMaxItemPlayerSellDisplay","NB max item can be displayed for player item list selled",128, 0, true);
CVariable<uint32>	NBMaxItemNpcSellDisplay("egs","NBMaxItemNpcSellDisplay","NB max item can be displayed for npc item list selled",128, 0, true);
CVariable<uint32>	NBMaxItemYoursSellDisplay("egs","NBMaxItemYoursSellDisplay","NB max item can be displayed for your item list selled",128, 0, true);

/// disconnection delay
CVariable<uint32>	TimeBeforeDisconnection("egs","TimeBeforeDisconnection", "delai during character stay in game after a disconnection for prevent logoff exploit", 300, 0, true );


// Privilege needed for banner
CVariable<string>	BannerPriv("privilege", "BannerPriv", "Privilege needed for banner", string(":G:SG:VG:GM:SGM:"), 0, true);
// Privilege that never aggro the bots
CVariable<string>	NeverAggroPriv("privilege", "NeverAggroPriv", "Privilege that never aggro the bots", string(":OBSERVER:G:SG:VG:GM:SGM:"), 0, true);
// Privilege always invisible
CVariable<string>	AlwaysInvisiblePriv("privilege", "AlwaysInvisiblePriv", "Privilege always invisible", string(":OBSERVER:"), 0, true);
// Privilege to teleport with a mektoub
CVariable<string>	TeleportWithMektoubPriv("privilege", "TeleportWithMektoubPriv", "Privilege to teleport with a mektoub", string(":GM:SGM:DEV:"), 0, true);
// Privilege that forbid action execution
CVariable<string>	NoActionAllowedPriv("privilege", "NoActionAllowedPriv", "Privilege that forbid action execution", string(":OBSERVER:"), 0, true);
// Privilege that bypass value and score checking
CVariable<string>	NoValueCheckingPriv("privilege", "NoValueCheckingPriv", "Privilege that bypass value and score checking", string(":GM:SGM:DEV:"), 0, true);
// Privilege that prevent being disconnected in case of shard closing for technical problem
CVariable<string>	NoForceDisconnectPriv("privilege", "NoForceDisconnectPriv", "Privilege that prevent being disconnected in case of shard closing for technical problem", string(":GM:SGM:DEV:"), 0, true);

/**
 * AnimalHungerFactor = MaxSatiety * RunSpeed / ( Range * (RunSpeed - WalkSpeed) * NbSecondPerTick)
 *           Example:   1000       * 9        / ( 7680  * (9        - 4.5      ) * 0.1)
 */
CVariable<float> AnimalHungerFactor( "egs", "AnimalHungerFactor",
	"Number of satiety points decreased per tick for each m/tick above the walk speed",
	1000.0f*9.0f / (7680.0f * (9.0f-4.5f) * 0.1f), // maxsatiety=1000, range=7680 m, runspeed=9m/s, walkspeed=4.5m/s
	0, true	);

CVariable<float> AnimalStopFollowingDistance( "egs", "AnimalStopFollowingDistance",
	"When a follower mektoub get further than this distance (in m) from his target, he stops", 100.0f, 0, true );

// events
CVariable<uint32> EventChannelHistoricSize( "egs", "EventChannelHistoricSize", "Historic size of event channel", 512, 0, true );

// outposts
CVariable<NLMISC::TGameCycle> OutpostSavingPeriod( "egs", "OutpostSavingPeriod", "Save period of outposts in ticks (1 outpost saved at a time)", 10, 0, true );
CVariable<NLMISC::TGameCycle> OutpostUpdatePeriod( "egs", "OutpostUpdatePeriod", "Period in ticks between 2 updates of the same outpost", 10, 0, true );
CVariable<NLMISC::TGameCycle> OutpostLeavePeriod("egs","OutpostLeavePeriod", "time duration before a user who has left outpost zone looses his pvp flag", 60*10, 0, true ); // 10 min as seconds

CVariable<bool>	VerboseFactionPoint("egs", "VerboseFactionPoint", "set if faction point system is verbose or not", false, 0, true);

CVariable<bool> UseNewNewbieLandStartingPoint("egs", "UseNewNewbieLandStartingPoint", "set if create new character start at new noobland or old", false, 0, true);

// Fame
// - Limits
CVariable<sint32> FameMinToDeclare( "egs", "FameMinToDeclare", "Minimum level of fame needed to declare membership to a clan", 30*kFameMultipler, 0, true );
CVariable<sint32> FameWarningLevel( "egs", "FameWarningLevel", "Level at which a player is notified that their fame is getting too low", 5*kFameMultipler, 0, true );
CVariable<sint32> FameMinToRemain( "egs", "FameMinToRemain", "Minimum fame needed to remain as a member of a clan", 0*kFameMultipler, 0, true );
CVariable<sint32> FameMinToTrade( "egs", "FameMinToTrade", "Minimum fame needed to trade with a clan", -30*kFameMultipler, 0, true, CFameManager::thresholdChanged );
CVariable<sint32> FameMinToKOS( "egs", "FameMinToKOS", "Minimum fame needed to not be KOS to a clan", -50*kFameMultipler, 0, true, CFameManager::thresholdChanged );
CVariable<sint32> FameMaxDefault( "egs", "FameMaxDefault", "Default maximum amount of fame (same as neutral max)", 50*kFameMultipler, 0, true );
CVariable<sint32> FameAbsoluteMin( "egs", "FameAbsoluteMin", "Absolute fame minimum a player can have", -100*kFameMultipler, 0, true );
CVariable<sint32> FameAbsoluteMax( "egs", "FameAbsoluteMax", "Absolute fame maximum a player can have", 100*kFameMultipler, 0, true );

/* Fame variables, naming:
	The fame variables begin with the name "Fame". The next part is if the variable is a 
	starting value, "Start", or maximum value, "Max".  The last elements indicate the groups 
	compared.  The first group is the character's group, then a small 'v' to separate, and 
	then the target group of the fame desired. For example: FameStartTrykervFyros is a Tryker 
	character's starting Fyros fame.
*/

// - Start values, Civilizations
CVariable<sint32> FameStartFyrosvFyros( "egs", "FameStartFyrosvFyros", "Starting fame of a Fyros to their own civ", 20*kFameMultipler, 0, true );
CVariable<sint32> FameStartFyrosvMatis( "egs", "FameStartFyrosvMatis", "Starting fame of a Fyros to the Matis civ", -20*kFameMultipler, 0, true );
CVariable<sint32> FameStartFyrosvTryker( "egs", "FameStartFyrosvTryker", "Starting fame of a Fyros to the Tryker civ", -10*kFameMultipler, 0, true );
CVariable<sint32> FameStartFyrosvZorai( "egs", "FameStartFyrosvZorai", "Starting fame of a Fyros to the Zorai civ", 10*kFameMultipler, 0, true );
CVariable<sint32> FameStartMatisvFyros( "egs", "FameStartMatisvFyros", "Starting fame of a Matis to the Fyros civ", -20*kFameMultipler, 0, true );
CVariable<sint32> FameStartMatisvMatis( "egs", "FameStartMatisvMatis", "Starting fame of a Matis to their own civ", 20*kFameMultipler, 0, true );
CVariable<sint32> FameStartMatisvTryker( "egs", "FameStartMatisvTryker", "Starting fame of a Matis to the Tryker civ", 10*kFameMultipler, 0, true );
CVariable<sint32> FameStartMatisvZorai( "egs", "FameStartMatisvZorai", "Starting fame of a Matis to the Zorai civ", -10*kFameMultipler, 0, true );
CVariable<sint32> FameStartTrykervFyros( "egs", "FameStartTrykervFyros", "Starting fame of a Tryker to the Fyros civ", -10*kFameMultipler, 0, true );
CVariable<sint32> FameStartTrykervMatis( "egs", "FameStartTrykervMatis", "Starting fame of a Tryker to the Matis civ", 10*kFameMultipler, 0, true );
CVariable<sint32> FameStartTrykervTryker( "egs", "FameStartTrykervTryker", "Starting fame of a Tryker to their own civ", 20*kFameMultipler, 0, true );
CVariable<sint32> FameStartTrykervZorai( "egs", "FameStartTrykervZorai", "Starting fame of a Tryker to the Zorai civ", -20*kFameMultipler, 0, true );
CVariable<sint32> FameStartZoraivFyros( "egs", "FameStartZoraivFyros", "Starting fame of a Zorai to the Fyros civ", 10*kFameMultipler, 0, true );
CVariable<sint32> FameStartZoraivMatis( "egs", "FameStartZoraivMatis", "Starting fame of a Zorai to the Matis civ", -10*kFameMultipler, 0, true );
CVariable<sint32> FameStartZoraivTryker( "egs", "FameStartZoraivTryker", "Starting fame of a Zorai to the Tryker civ", -20*kFameMultipler, 0, true );
CVariable<sint32> FameStartZoraivZorai( "egs", "FameStartZoraivZorai", "Starting fame of a Zorai to their own civ", 20*kFameMultipler, 0, true );

// - Start values, Cults
CVariable<sint32> FameStartFyrosvKami( "egs", "FameStartFyrosvKami", "Starting fame of a Fyros to the Kami cult", 10*kFameMultipler, 0, true );
CVariable<sint32> FameStartFyrosvKaravan( "egs", "FameStartFyrosvKaravan", "Starting fame of a Fyros to the Karavan cult", -10*kFameMultipler, 0, true );
CVariable<sint32> FameStartMatisvKami( "egs", "FameStartMatisvKami", "Starting fame of a Matis to the Kami cult", -20*kFameMultipler, 0, true );
CVariable<sint32> FameStartMatisvKaravan( "egs", "FameStartMatisvKaravan", "Starting fame of a Matis to the Karavan cult", 20*kFameMultipler, 0, true );
CVariable<sint32> FameStartTrykervKami( "egs", "FameStartTrykervKami", "Starting fame of a Tryker to the Kami cult", -10*kFameMultipler, 0, true );
CVariable<sint32> FameStartTrykervKaravan( "egs", "FameStartTrykervKaravan", "Starting fame of a Tryker to the Karavan cult", 10*kFameMultipler, 0, true );
CVariable<sint32> FameStartZoraivKami( "egs", "FameStartZoraivKami", "Starting fame of a Zorai to the Kami cult", 20*kFameMultipler, 0, true );
CVariable<sint32> FameStartZoraivKaravan( "egs", "FameStartZoraivKaravan", "Starting fame of a Zorai to the Karavan cult", -20*kFameMultipler, 0, true );

// - Max Values when declared, Civilizations
CVariable<sint32> FameMaxNeutralvFyros( "egs", "FameMaxNeutralvFyros", "Maximum Fame for a neutral to the Fyros civ", 50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxNeutralvMatis( "egs", "FameMaxNeutralvMatis", "Maximum Fame for a neutral to the Matis civ", 50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxNeutralvTryker( "egs", "FameMaxNeutralvTryker", "Maximum Fame for a neutral to the Tryker civ", 50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxNeutralvZorai( "egs", "FameMaxNeutralvZorai", "Maximum Fame for a neutral to the Zorai civ", 50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxFyrosvFyros( "egs", "FameMaxFyrosvFyros", "Maximum Fame for a Fyros to their own civ", 100*kFameMultipler, 0, true );
CVariable<sint32> FameMaxFyrosvMatis( "egs", "FameMaxFyrosvMatis", "Maximum Fame for a Fyros to the Matis civ", 0*kFameMultipler, 0, true );
CVariable<sint32> FameMaxFyrosvTryker( "egs", "FameMaxFyrosvTryker", "Maximum Fame for a Fyros to the Tryker civ", 25*kFameMultipler, 0, true );
CVariable<sint32> FameMaxFyrosvZorai( "egs", "FameMaxFyrosvZorai", "Maximum Fame for a Fyros to the Zorai civ", 75*kFameMultipler, 0, true );
CVariable<sint32> FameMaxMatisvFyros( "egs", "FameMaxMatisvFyros", "Maximum Fame for a Matis to the Fyros civ", 0*kFameMultipler, 0, true );
CVariable<sint32> FameMaxMatisvMatis( "egs", "FameMaxMatisvMatis", "Maximum Fame for a Matis to their own civ", 100*kFameMultipler, 0, true );
CVariable<sint32> FameMaxMatisvTryker( "egs", "FameMaxMatisvTryker", "Maximum Fame for a Matis to the Tryker civ", 75*kFameMultipler, 0, true );
CVariable<sint32> FameMaxMatisvZorai( "egs", "FameMaxMatisvZorai", "Maximum Fame for a Matis to the Zorai civ", 25*kFameMultipler, 0, true );
CVariable<sint32> FameMaxTrykervFyros( "egs", "FameMaxTrykervFyros", "Maximum Fame for a Tryker to the Fyros civ", 25*kFameMultipler, 0, true );
CVariable<sint32> FameMaxTrykervMatis( "egs", "FameMaxTrykervMatis", "Maximum Fame for a Tryker to the Matis civ", 75*kFameMultipler, 0, true );
CVariable<sint32> FameMaxTrykervTryker( "egs", "FameMaxTrykervTryker", "Maximum Fame for a Tryker to their own civ", 100*kFameMultipler, 0, true );
CVariable<sint32> FameMaxTrykervZorai( "egs", "FameMaxTrykervZorai", "Maximum Fame for a Tryker to the Zorai civ", 0*kFameMultipler, 0, true );
CVariable<sint32> FameMaxZoraivFyros( "egs", "FameMaxZoraivFyros", "Maximum Fame for a Zorai to the Fyros civ", 75*kFameMultipler, 0, true );
CVariable<sint32> FameMaxZoraivMatis( "egs", "FameMaxZoraivMatis", "Maximum Fame for a Zorai to the Matis civ", 25*kFameMultipler, 0, true );
CVariable<sint32> FameMaxZoraivTryker( "egs", "FameMaxZoraivTryker", "Maximum Fame for a Zorai to the Tryker civ", 0*kFameMultipler, 0, true );
CVariable<sint32> FameMaxZoraivZorai( "egs", "FameMaxZoraivZorai", "Maximum Fame for a Zorai to their own civ", 100*kFameMultipler, 0, true );

// - Max Values when declared, Civilizations
CVariable<sint32> FameMaxNeutralvKami( "egs", "FameMaxNeutralvKami", "Max fame for a neutral to the Kami cult", 50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxNeutralvKaravan( "egs", "FameMaxNeutralvKaravan", "Max fame for a neutral to the Karavan cult", 50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxKamivKami( "egs", "FameMaxKamivKami", "Max fame for a Kami to the Kami cult", 100*kFameMultipler, 0, true );
CVariable<sint32> FameMaxKamivKaravan( "egs", "FameMaxKamivKaravan", "Max fame for a Kami to the Karavan cult", -50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxKaravanvKami( "egs", "FameMaxKaravanvKami", "Max fame for a Karavan to the Kami cult", -50*kFameMultipler, 0, true );
CVariable<sint32> FameMaxKaravanvKaravan( "egs", "FameMaxKaravanvKaravan", "Max fame for a Karavan to the Karavan cult", 100*kFameMultipler, 0, true );

// female titles
CVariable<bool> UseFemaleTitles("egs","UseFemaleTitles", "Use female titles when character is a female character", true, 0, true);

// - NPC Icons
CVariable<uint32> ClientNPCIconRefreshTimerDelay("egs", "ClientNPCIconRefreshTimerDelay", "Max number of gamecycles between 2 refreshes of a single mission giver NPC", 60*10, 0, true, CPlayerManager::onNPCIconTimerChanged);

// - Ring 
CVariable<uint32> TickFrequencyNpcControlUpdate("egs","TickFrequencyNpcControlUpdate","Min time in tick between 2 updates of udate of npc control", 6, 0, true);

// Scores
CVariable<sint32> PhysicalCharacteristicsBaseValue( "egs", "PhysicalCharacteristicsBaseValue", "Physical characteristic base value used to compute base score value", 10, 0, true );
CVariable<sint32> PhysicalCharacteristicsFactor( "egs", "PhysicalCharacteristicsFactor", "Factor used to compute base score value from characteristic value", 10, 0, true );
