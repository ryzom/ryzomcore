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



#ifndef RY_CRAFT_PARAMETERS
#define RY_CRAFT_PARAMETERS


// misc
#include "nel/misc/variable.h"

//------------------------------------------------------------------------
// ptr class added by sadge
// inline implementations are located at bottom of this file
// Modified by Saffray :
//		complete with all craft parameters
//		make all parameters is adjustable by variable
class CWeaponCraftParameters
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Common factors
	//////////////////////////////////////////////////////////////////////////
	//Durability
	// melee weapons
	static NLMISC::CVariable<float> DaggerDurability;
	static NLMISC::CVariable<float> SwordDurability;
	static NLMISC::CVariable<float> MaceDurability;
	static NLMISC::CVariable<float> AxeDurability;
	static NLMISC::CVariable<float> SpearDurability;
	static NLMISC::CVariable<float> StaffDurability;
	static NLMISC::CVariable<float> MagicianStaffDurability;
	static NLMISC::CVariable<float> TwoHandSwordDurability;
	static NLMISC::CVariable<float> TwoHandAxeDurability;
	static NLMISC::CVariable<float> PikeDurability;
	static NLMISC::CVariable<float> TwoHandMaceDurability;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchDurability;
	static NLMISC::CVariable<float> BowrifleDurability;
	static NLMISC::CVariable<float> LauncherDurability;
	static NLMISC::CVariable<float> PistolDurability;
	static NLMISC::CVariable<float> BowpistolDurability;
	static NLMISC::CVariable<float> RifleDurability;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoDurability;
	static NLMISC::CVariable<float> BowrifleAmmoDurability;
	static NLMISC::CVariable<float> LauncherAmmoDurability;
	static NLMISC::CVariable<float> PistolAmmoDurability;
	static NLMISC::CVariable<float> BowpistolAmmoDurability;
	static NLMISC::CVariable<float> RifleAmmoDurability;
		
	// armor and shield
	static NLMISC::CVariable<float> ShieldDurability;
	static NLMISC::CVariable<float> BucklerDurability;
	static NLMISC::CVariable<float> LightBootsDurability;
	static NLMISC::CVariable<float> LightGlovesDurability;
	static NLMISC::CVariable<float> LightPantsDurability;
	static NLMISC::CVariable<float> LightSleevesDurability;
	static NLMISC::CVariable<float> LightVestDurability;
	static NLMISC::CVariable<float> MediumBootsDurability;
	static NLMISC::CVariable<float> MediumGlovesDurability;
	static NLMISC::CVariable<float> MediumPantsDurability;
	static NLMISC::CVariable<float> MediumSleevesDurability;
	static NLMISC::CVariable<float> MediumVestDurability;
	static NLMISC::CVariable<float> HeavyBootsDurability;
	static NLMISC::CVariable<float> HeavyGlovesDurability;
	static NLMISC::CVariable<float> HeavyPantsDurability;
	static NLMISC::CVariable<float> HeavySleevesDurability;
	static NLMISC::CVariable<float> HeavyVestDurability;
	static NLMISC::CVariable<float> HeavyHelmetDurability;
	
	// jewel
	static NLMISC::CVariable<float> AnkletDurability;
	static NLMISC::CVariable<float> BraceletDurability;
	static NLMISC::CVariable<float> DiademDurability;
	static NLMISC::CVariable<float> EaringDurability;
	static NLMISC::CVariable<float> PendantDurability;
	static NLMISC::CVariable<float> RingDurability;

	// tool
	static NLMISC::CVariable<float> ForageToolDurability;
	static NLMISC::CVariable<float> AmmoCraftingToolDurability;
	static NLMISC::CVariable<float> ArmorCraftingToolDurability;
	static NLMISC::CVariable<float> JewelryCraftingToolDurability;
	static NLMISC::CVariable<float> RangeWeaponCraftingToolDurability;
	static NLMISC::CVariable<float> MeleeWeaponCraftingToolDurability;
	static NLMISC::CVariable<float> ToolCraftingToolDurability;
		
		
	//Weight
	// melee weapons
	static NLMISC::CVariable<float> DaggerWeight;
	static NLMISC::CVariable<float> SwordWeight;
	static NLMISC::CVariable<float> MaceWeight;
	static NLMISC::CVariable<float> AxeWeight;
	static NLMISC::CVariable<float> SpearWeight;
	static NLMISC::CVariable<float> StaffWeight;
	static NLMISC::CVariable<float> MagicianStaffWeight;
	static NLMISC::CVariable<float> TwoHandSwordWeight;
	static NLMISC::CVariable<float> TwoHandAxeWeight;
	static NLMISC::CVariable<float> PikeWeight;
	static NLMISC::CVariable<float> TwoHandMaceWeight;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchWeight;
	static NLMISC::CVariable<float> BowrifleWeight;
	static NLMISC::CVariable<float> LauncherWeight;
	static NLMISC::CVariable<float> PistolWeight;
	static NLMISC::CVariable<float> BowpistolWeight;
	static NLMISC::CVariable<float> RifleWeight;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoWeight;
	static NLMISC::CVariable<float> BowrifleAmmoWeight;
	static NLMISC::CVariable<float> LauncherAmmoWeight;
	static NLMISC::CVariable<float> PistolAmmoWeight;
	static NLMISC::CVariable<float> BowpistolAmmoWeight;
	static NLMISC::CVariable<float> RifleAmmoWeight;
	
	// armor and shield
	static NLMISC::CVariable<float> ShieldWeight;
	static NLMISC::CVariable<float> BucklerWeight;
	static NLMISC::CVariable<float> LightBootsWeight;
	static NLMISC::CVariable<float> LightGlovesWeight;
	static NLMISC::CVariable<float> LightPantsWeight;
	static NLMISC::CVariable<float> LightSleevesWeight;
	static NLMISC::CVariable<float> LightVestWeight;
	static NLMISC::CVariable<float> MediumBootsWeight;
	static NLMISC::CVariable<float> MediumGlovesWeight;
	static NLMISC::CVariable<float> MediumPantsWeight;
	static NLMISC::CVariable<float> MediumSleevesWeight;
	static NLMISC::CVariable<float> MediumVestWeight;
	static NLMISC::CVariable<float> HeavyBootsWeight;
	static NLMISC::CVariable<float> HeavyGlovesWeight;
	static NLMISC::CVariable<float> HeavyPantsWeight;
	static NLMISC::CVariable<float> HeavySleevesWeight;
	static NLMISC::CVariable<float> HeavyVestWeight;
	static NLMISC::CVariable<float> HeavyHelmetWeight;
	
	// jewel
	static NLMISC::CVariable<float> AnkletWeight;
	static NLMISC::CVariable<float> BraceletWeight;
	static NLMISC::CVariable<float> DiademWeight;
	static NLMISC::CVariable<float> EaringWeight;
	static NLMISC::CVariable<float> PendantWeight;
	static NLMISC::CVariable<float> RingWeight;
	
	//SapLoad Min
	// melee weapons
	static NLMISC::CVariable<float> DaggerSapLoad;
	static NLMISC::CVariable<float> SwordSapLoad;
	static NLMISC::CVariable<float> MaceSapLoad;
	static NLMISC::CVariable<float> AxeSapLoad;
	static NLMISC::CVariable<float> SpearSapLoad;
	static NLMISC::CVariable<float> StaffSapLoad;
	static NLMISC::CVariable<float> MagicianStaffSapLoad;
	static NLMISC::CVariable<float> TwoHandSwordSapLoad;
	static NLMISC::CVariable<float> TwoHandAxeSapLoad;
	static NLMISC::CVariable<float> PikeSapLoad;
	static NLMISC::CVariable<float> TwoHandMaceSapLoad;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchSapLoad;
	static NLMISC::CVariable<float> BowrifleSapLoad;
	static NLMISC::CVariable<float> LauncherSapLoad;
	static NLMISC::CVariable<float> PistolSapLoad;
	static NLMISC::CVariable<float> BowpistolSapLoad;
	static NLMISC::CVariable<float> RifleSapLoad;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoSapLoad;
	static NLMISC::CVariable<float> BowrifleAmmoSapLoad;
	static NLMISC::CVariable<float> LauncherAmmoSapLoad;
	static NLMISC::CVariable<float> PistolAmmoSapLoad;
	static NLMISC::CVariable<float> BowpistolAmmoSapLoad;
	static NLMISC::CVariable<float> RifleAmmoSapLoad;
	
	// armor and shield
	static NLMISC::CVariable<float> ShieldSapLoad;
	static NLMISC::CVariable<float> BucklerSapLoad;
	static NLMISC::CVariable<float> LightBootsSapLoad;
	static NLMISC::CVariable<float> LightGlovesSapLoad;
	static NLMISC::CVariable<float> LightPantsSapLoad;
	static NLMISC::CVariable<float> LightSleevesSapLoad;
	static NLMISC::CVariable<float> LightVestSapLoad;
	static NLMISC::CVariable<float> MediumBootsSapLoad;
	static NLMISC::CVariable<float> MediumGlovesSapLoad;
	static NLMISC::CVariable<float> MediumPantsSapLoad;
	static NLMISC::CVariable<float> MediumSleevesSapLoad;
	static NLMISC::CVariable<float> MediumVestSapLoad;
	static NLMISC::CVariable<float> HeavyBootsSapLoad;
	static NLMISC::CVariable<float> HeavyGlovesSapLoad;
	static NLMISC::CVariable<float> HeavyPantsSapLoad;
	static NLMISC::CVariable<float> HeavySleevesSapLoad;
	static NLMISC::CVariable<float> HeavyVestSapLoad;
	static NLMISC::CVariable<float> HeavyHelmetSapLoad;
	
	// jewel
	static NLMISC::CVariable<float> AnkletSapLoad;
	static NLMISC::CVariable<float> BraceletSapLoad;
	static NLMISC::CVariable<float> DiademSapLoad;
	static NLMISC::CVariable<float> EaringSapLoad;
	static NLMISC::CVariable<float> PendantSapLoad;
	static NLMISC::CVariable<float> RingSapLoad;

	//SapLoad Max
	// melee weapons
	static NLMISC::CVariable<float> DaggerSapLoadMax;
	static NLMISC::CVariable<float> SwordSapLoadMax;
	static NLMISC::CVariable<float> MaceSapLoadMax;
	static NLMISC::CVariable<float> AxeSapLoadMax;
	static NLMISC::CVariable<float> SpearSapLoadMax;
	static NLMISC::CVariable<float> StaffSapLoadMax;
	static NLMISC::CVariable<float> MagicianStaffSapLoadMax;
	static NLMISC::CVariable<float> TwoHandSwordSapLoadMax;
	static NLMISC::CVariable<float> TwoHandAxeSapLoadMax;
	static NLMISC::CVariable<float> PikeSapLoadMax;
	static NLMISC::CVariable<float> TwoHandMaceSapLoadMax;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchSapLoadMax;
	static NLMISC::CVariable<float> BowrifleSapLoadMax;
	static NLMISC::CVariable<float> LauncherSapLoadMax;
	static NLMISC::CVariable<float> PistolSapLoadMax;
	static NLMISC::CVariable<float> BowpistolSapLoadMax;
	static NLMISC::CVariable<float> RifleSapLoadMax;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoSapLoadMax;
	static NLMISC::CVariable<float> BowrifleAmmoSapLoadMax;
	static NLMISC::CVariable<float> LauncherAmmoSapLoadMax;
	static NLMISC::CVariable<float> PistolAmmoSapLoadMax;
	static NLMISC::CVariable<float> BowpistolAmmoSapLoadMax;
	static NLMISC::CVariable<float> RifleAmmoSapLoadMax;
	
	// armor and shield
	static NLMISC::CVariable<float> ShieldSapLoadMax;
	static NLMISC::CVariable<float> BucklerSapLoadMax;
	static NLMISC::CVariable<float> LightBootsSapLoadMax;
	static NLMISC::CVariable<float> LightGlovesSapLoadMax;
	static NLMISC::CVariable<float> LightPantsSapLoadMax;
	static NLMISC::CVariable<float> LightSleevesSapLoadMax;
	static NLMISC::CVariable<float> LightVestSapLoadMax;
	static NLMISC::CVariable<float> MediumBootsSapLoadMax;
	static NLMISC::CVariable<float> MediumGlovesSapLoadMax;
	static NLMISC::CVariable<float> MediumPantsSapLoadMax;
	static NLMISC::CVariable<float> MediumSleevesSapLoadMax;
	static NLMISC::CVariable<float> MediumVestSapLoadMax;
	static NLMISC::CVariable<float> HeavyBootsSapLoadMax;
	static NLMISC::CVariable<float> HeavyGlovesSapLoadMax;
	static NLMISC::CVariable<float> HeavyPantsSapLoadMax;
	static NLMISC::CVariable<float> HeavySleevesSapLoadMax;
	static NLMISC::CVariable<float> HeavyVestSapLoadMax;
	static NLMISC::CVariable<float> HeavyHelmetSapLoadMax;
	
	// jewel
	static NLMISC::CVariable<float> AnkletSapLoadMax;
	static NLMISC::CVariable<float> BraceletSapLoadMax;
	static NLMISC::CVariable<float> DiademSapLoadMax;
	static NLMISC::CVariable<float> EaringSapLoadMax;
	static NLMISC::CVariable<float> PendantSapLoadMax;
	static NLMISC::CVariable<float> RingSapLoadMax;
	
	//////////////////////////////////////////////////////////////////////////
	// Weapons factors
	//////////////////////////////////////////////////////////////////////////
	//Dmg Min for melee weapon, range weapon (modifier), ammo
	// melee weapons
	static NLMISC::CVariable<float> DaggerDmg;
	static NLMISC::CVariable<float> SwordDmg;
	static NLMISC::CVariable<float> MaceDmg;
	static NLMISC::CVariable<float> AxeDmg;
	static NLMISC::CVariable<float> SpearDmg;
	static NLMISC::CVariable<float> StaffDmg;
	static NLMISC::CVariable<float> MagicianStaffDmg;
	static NLMISC::CVariable<float> TwoHandSwordDmg;
	static NLMISC::CVariable<float> TwoHandAxeDmg;
	static NLMISC::CVariable<float> PikeDmg;
	static NLMISC::CVariable<float> TwoHandMaceDmg;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchDmg;
	static NLMISC::CVariable<float> BowrifleDmg;
	static NLMISC::CVariable<float> LauncherDmg;
	static NLMISC::CVariable<float> PistolDmg;
	static NLMISC::CVariable<float> BowpistolDmg;
	static NLMISC::CVariable<float> RifleDmg;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoDmg;
	static NLMISC::CVariable<float> BowrifleAmmoDmg;
	static NLMISC::CVariable<float> LauncherAmmoDmg;
	static NLMISC::CVariable<float> PistolAmmoDmg;
	static NLMISC::CVariable<float> BowpistolAmmoDmg;
	static NLMISC::CVariable<float> RifleAmmoDmg;

	//Dmg Max for melee weapon, range weapon (modifier), ammo
	// melee weapons
	static NLMISC::CVariable<float> DaggerDmgMax;
	static NLMISC::CVariable<float> SwordDmgMax;
	static NLMISC::CVariable<float> MaceDmgMax;
	static NLMISC::CVariable<float> AxeDmgMax;
	static NLMISC::CVariable<float> SpearDmgMax;
	static NLMISC::CVariable<float> StaffDmgMax;
	static NLMISC::CVariable<float> MagicianStaffDmgMax;
	static NLMISC::CVariable<float> TwoHandSwordDmgMax;
	static NLMISC::CVariable<float> TwoHandAxeDmgMax;
	static NLMISC::CVariable<float> PikeDmgMax;
	static NLMISC::CVariable<float> TwoHandMaceDmgMax;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchDmgMax;
	static NLMISC::CVariable<float> BowrifleDmgMax;
	static NLMISC::CVariable<float> LauncherDmgMax;
	static NLMISC::CVariable<float> PistolDmgMax;
	static NLMISC::CVariable<float> BowpistolDmgMax;
	static NLMISC::CVariable<float> RifleDmgMax;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoDmgMax;
	static NLMISC::CVariable<float> BowrifleAmmoDmgMax;
	static NLMISC::CVariable<float> LauncherAmmoDmgMax;
	static NLMISC::CVariable<float> PistolAmmoDmgMax;
	static NLMISC::CVariable<float> BowpistolAmmoDmgMax;
	static NLMISC::CVariable<float> RifleAmmoDmgMax;
	
	
	//HitRate min for ammos (modifier), melee weapon, range weapon
	// melee weapons
	static NLMISC::CVariable<float> DaggerHitRate;
	static NLMISC::CVariable<float> SwordHitRate;
	static NLMISC::CVariable<float> MaceHitRate;
	static NLMISC::CVariable<float> AxeHitRate;
	static NLMISC::CVariable<float> SpearHitRate;
	static NLMISC::CVariable<float> StaffHitRate;
	static NLMISC::CVariable<float> MagicianStaffHitRate;
	static NLMISC::CVariable<float> TwoHandSwordHitRate;
	static NLMISC::CVariable<float> TwoHandAxeHitRate;
	static NLMISC::CVariable<float> PikeHitRate;
	static NLMISC::CVariable<float> TwoHandMaceHitRate;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchHitRate;
	static NLMISC::CVariable<float> BowrifleHitRate;
	static NLMISC::CVariable<float> LauncherHitRate;
	static NLMISC::CVariable<float> PistolHitRate;
	static NLMISC::CVariable<float> BowpistolHitRate;
	static NLMISC::CVariable<float> RifleHitRate;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoHitRate;
	static NLMISC::CVariable<float> BowrifleAmmoHitRate;
	static NLMISC::CVariable<float> LauncherAmmoHitRate;
	static NLMISC::CVariable<float> PistolAmmoHitRate;
	static NLMISC::CVariable<float> BowpistolAmmoHitRate;
	static NLMISC::CVariable<float> RifleAmmoHitRate;

	//HitRate max for ammos (modifier), melee weapon, range weapon
	// melee weapons
	static NLMISC::CVariable<float> DaggerHitRateMax;
	static NLMISC::CVariable<float> SwordHitRateMax;
	static NLMISC::CVariable<float> MaceHitRateMax;
	static NLMISC::CVariable<float> AxeHitRateMax;
	static NLMISC::CVariable<float> SpearHitRateMax;
	static NLMISC::CVariable<float> StaffHitRateMax;
	static NLMISC::CVariable<float> MagicianStaffHitRateMax;
	static NLMISC::CVariable<float> TwoHandSwordHitRateMax;
	static NLMISC::CVariable<float> TwoHandAxeHitRateMax;
	static NLMISC::CVariable<float> PikeHitRateMax;
	static NLMISC::CVariable<float> TwoHandMaceHitRateMax;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchHitRateMax;
	static NLMISC::CVariable<float> BowrifleHitRateMax;
	static NLMISC::CVariable<float> LauncherHitRateMax;
	static NLMISC::CVariable<float> PistolHitRateMax;
	static NLMISC::CVariable<float> BowpistolHitRateMax;
	static NLMISC::CVariable<float> RifleHitRateMax;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoHitRateMax;
	static NLMISC::CVariable<float> BowrifleAmmoHitRateMax;
	static NLMISC::CVariable<float> LauncherAmmoHitRateMax;
	static NLMISC::CVariable<float> PistolAmmoHitRateMax;
	static NLMISC::CVariable<float> BowpistolAmmoHitRateMax;
	static NLMISC::CVariable<float> RifleAmmoHitRateMax;
	
	
	//Range	for ammo, range weapon (modifier)
	// range weapon
	static NLMISC::CVariable<float> AutolauchRange;
	static NLMISC::CVariable<float> BowrifleRange;
	static NLMISC::CVariable<float> LauncherRange;
	static NLMISC::CVariable<float> PistolRange;
	static NLMISC::CVariable<float> BowpistolRange;
	static NLMISC::CVariable<float> RifleRange;
	
	// ammo
	static NLMISC::CVariable<float> AutolaunchAmmoRange;
	static NLMISC::CVariable<float> BowrifleAmmoRange;
	static NLMISC::CVariable<float> LauncherAmmoRange;
	static NLMISC::CVariable<float> PistolAmmoRange;
	static NLMISC::CVariable<float> BowpistolAmmoRange;
	static NLMISC::CVariable<float> RifleAmmoRange;
	
	
	//DodgeModifier not for ammo and jewel, but for armor too
	// melee weapons & armor
	static NLMISC::CVariable<float> DaggerDodgeMinModifier;
	static NLMISC::CVariable<float> DaggerDodgeMaxModifier;
	static NLMISC::CVariable<float> SwordDodgeMinModifier;
	static NLMISC::CVariable<float> SwordDodgeMaxModifier;
	static NLMISC::CVariable<float> MaceDodgeMinModifier;
	static NLMISC::CVariable<float> MaceDodgeMaxModifier;
	static NLMISC::CVariable<float> AxeDodgeMinModifier;
	static NLMISC::CVariable<float> AxeDodgeMaxModifier;
	static NLMISC::CVariable<float> SpearDodgeMinModifier;
	static NLMISC::CVariable<float> SpearDodgeMaxModifier;
	static NLMISC::CVariable<float> StaffDodgeMinModifier;
	static NLMISC::CVariable<float> StaffDodgeMaxModifier;
	static NLMISC::CVariable<float> MagicianStaffDodgeMinModifier;
	static NLMISC::CVariable<float> MagicianStaffDodgeMaxModifier;
	static NLMISC::CVariable<float> TwoHandSwordDodgeMinModifier;
	static NLMISC::CVariable<float> TwoHandSwordDodgeMaxModifier;
	static NLMISC::CVariable<float> TwoHandAxeDodgeMinModifier;
	static NLMISC::CVariable<float> TwoHandAxeDodgeMaxModifier;
	static NLMISC::CVariable<float> PikeDodgeMinModifier;
	static NLMISC::CVariable<float> PikeDodgeMaxModifier;
	static NLMISC::CVariable<float> TwoHandMaceDodgeMinModifier;
	static NLMISC::CVariable<float> TwoHandMaceDodgeMaxModifier;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchDodgeMinModifier;
	static NLMISC::CVariable<float> AutolauchDodgeMaxModifier;
	static NLMISC::CVariable<float> BowrifleDodgeMinModifier;
	static NLMISC::CVariable<float> BowrifleDodgeMaxModifier;
	static NLMISC::CVariable<float> LauncherDodgeMinModifier;
	static NLMISC::CVariable<float> LauncherDodgeMaxModifier;
	static NLMISC::CVariable<float> PistolDodgeMinModifier;
	static NLMISC::CVariable<float> PistolDodgeMaxModifier;
	static NLMISC::CVariable<float> BowpistolDodgeMinModifier;
	static NLMISC::CVariable<float> BowpistolDodgeMaxModifier;
	static NLMISC::CVariable<float> RifleDodgeMinModifier;
	static NLMISC::CVariable<float> RifleDodgeMaxModifier;
	
	// armor and shield
	static NLMISC::CVariable<float> ShieldDodgeMinModifier;
	static NLMISC::CVariable<float> ShieldDodgeMaxModifier;
	static NLMISC::CVariable<float> BucklerDodgeMinModifier;
	static NLMISC::CVariable<float> BucklerDodgeMaxModifier;
	static NLMISC::CVariable<float> LightBootsDodgeMinModifier;
	static NLMISC::CVariable<float> LightBootsDodgeMaxModifier;
	static NLMISC::CVariable<float> LightGlovesDodgeMinModifier;
	static NLMISC::CVariable<float> LightGlovesDodgeMaxModifier;
	static NLMISC::CVariable<float> LightPantsDodgeMinModifier;
	static NLMISC::CVariable<float> LightPantsDodgeMaxModifier;
	static NLMISC::CVariable<float> LightSleevesDodgeMinModifier;
	static NLMISC::CVariable<float> LightSleevesDodgeMaxModifier;
	static NLMISC::CVariable<float> LightVestDodgeMinModifier;
	static NLMISC::CVariable<float> LightVestDodgeMaxModifier;
	static NLMISC::CVariable<float> MediumBootsDodgeMinModifier;
	static NLMISC::CVariable<float> MediumBootsDodgeMaxModifier;
	static NLMISC::CVariable<float> MediumGlovesDodgeMinModifier;
	static NLMISC::CVariable<float> MediumGlovesDodgeMaxModifier;
	static NLMISC::CVariable<float> MediumPantsDodgeMinModifier;
	static NLMISC::CVariable<float> MediumPantsDodgeMaxModifier;
	static NLMISC::CVariable<float> MediumSleevesDodgeMinModifier;
	static NLMISC::CVariable<float> MediumSleevesDodgeMaxModifier;
	static NLMISC::CVariable<float> MediumVestDodgeMinModifier;
	static NLMISC::CVariable<float> MediumVestDodgeMaxModifier;
	static NLMISC::CVariable<float> HeavyBootsDodgeMinModifier;
	static NLMISC::CVariable<float> HeavyBootsDodgeMaxModifier;
	static NLMISC::CVariable<float> HeavyGlovesDodgeMinModifier;
	static NLMISC::CVariable<float> HeavyGlovesDodgeMaxModifier;
	static NLMISC::CVariable<float> HeavyPantsDodgeMinModifier;
	static NLMISC::CVariable<float> HeavyPantsDodgeMaxModifier;
	static NLMISC::CVariable<float> HeavySleevesDodgeMinModifier;
	static NLMISC::CVariable<float> HeavySleevesDodgeMaxModifier;
	static NLMISC::CVariable<float> HeavyVestDodgeMinModifier;
	static NLMISC::CVariable<float> HeavyVestDodgeMaxModifier;
	static NLMISC::CVariable<float> HeavyHelmetDodgeMinModifier;
	static NLMISC::CVariable<float> HeavyHelmetDodgeMaxModifier;
	
	
	//ParryModifier	not for ammo and jewel, but for armor too
	// melee weapons
	static NLMISC::CVariable<float> DaggerParryMinModifier;
	static NLMISC::CVariable<float> DaggerParryMaxModifier;
	static NLMISC::CVariable<float> SwordParryMinModifier;
	static NLMISC::CVariable<float> SwordParryMaxModifier;
	static NLMISC::CVariable<float> MaceParryMinModifier;
	static NLMISC::CVariable<float> MaceParryMaxModifier;
	static NLMISC::CVariable<float> AxeParryMinModifier;
	static NLMISC::CVariable<float> AxeParryMaxModifier;
	static NLMISC::CVariable<float> SpearParryMinModifier;
	static NLMISC::CVariable<float> SpearParryMaxModifier;
	static NLMISC::CVariable<float> StaffParryMinModifier;
	static NLMISC::CVariable<float> StaffParryMaxModifier;
	static NLMISC::CVariable<float> MagicianStaffParryMinModifier;
	static NLMISC::CVariable<float> MagicianStaffParryMaxModifier;
	static NLMISC::CVariable<float> TwoHandSwordParryMinModifier;
	static NLMISC::CVariable<float> TwoHandSwordParryMaxModifier;
	static NLMISC::CVariable<float> TwoHandAxeParryMinModifier;
	static NLMISC::CVariable<float> TwoHandAxeParryMaxModifier;
	static NLMISC::CVariable<float> PikeParryMinModifier;
	static NLMISC::CVariable<float> PikeParryMaxModifier;
	static NLMISC::CVariable<float> TwoHandMaceParryMinModifier;
	static NLMISC::CVariable<float> TwoHandMaceParryMaxModifier;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchParryMinModifier;
	static NLMISC::CVariable<float> AutolauchParryMaxModifier;
	static NLMISC::CVariable<float> BowrifleParryMinModifier;
	static NLMISC::CVariable<float> BowrifleParryMaxModifier;
	static NLMISC::CVariable<float> LauncherParryMinModifier;
	static NLMISC::CVariable<float> LauncherParryMaxModifier;
	static NLMISC::CVariable<float> PistolParryMinModifier;
	static NLMISC::CVariable<float> PistolParryMaxModifier;
	static NLMISC::CVariable<float> BowpistolParryMinModifier;
	static NLMISC::CVariable<float> BowpistolParryMaxModifier;
	static NLMISC::CVariable<float> RifleParryMinModifier;
	static NLMISC::CVariable<float> RifleParryMaxModifier;
	
	// armor and shield
	static NLMISC::CVariable<float> ShieldParryMinModifier;
	static NLMISC::CVariable<float> ShieldParryMaxModifier;
	static NLMISC::CVariable<float> BucklerParryMinModifier;
	static NLMISC::CVariable<float> BucklerParryMaxModifier;
	static NLMISC::CVariable<float> LightBootsParryMinModifier;
	static NLMISC::CVariable<float> LightBootsParryMaxModifier;
	static NLMISC::CVariable<float> LightGlovesParryMinModifier;
	static NLMISC::CVariable<float> LightGlovesParryMaxModifier;
	static NLMISC::CVariable<float> LightPantsParryMinModifier;
	static NLMISC::CVariable<float> LightPantsParryMaxModifier;
	static NLMISC::CVariable<float> LightSleevesParryMinModifier;
	static NLMISC::CVariable<float> LightSleevesParryMaxModifier;
	static NLMISC::CVariable<float> LightVestParryMinModifier;
	static NLMISC::CVariable<float> LightVestParryMaxModifier;
	static NLMISC::CVariable<float> MediumBootsParryMinModifier;
	static NLMISC::CVariable<float> MediumBootsParryMaxModifier;
	static NLMISC::CVariable<float> MediumGlovesParryMinModifier;
	static NLMISC::CVariable<float> MediumGlovesParryMaxModifier;
	static NLMISC::CVariable<float> MediumPantsParryMinModifier;
	static NLMISC::CVariable<float> MediumPantsParryMaxModifier;
	static NLMISC::CVariable<float> MediumSleevesParryMinModifier;
	static NLMISC::CVariable<float> MediumSleevesParryMaxModifier;
	static NLMISC::CVariable<float> MediumVestParryMinModifier;
	static NLMISC::CVariable<float> MediumVestParryMaxModifier;
	static NLMISC::CVariable<float> HeavyBootsParryMinModifier;
	static NLMISC::CVariable<float> HeavyBootsParryMaxModifier;
	static NLMISC::CVariable<float> HeavyGlovesParryMinModifier;
	static NLMISC::CVariable<float> HeavyGlovesParryMaxModifier;
	static NLMISC::CVariable<float> HeavyPantsParryMinModifier;
	static NLMISC::CVariable<float> HeavyPantsParryMaxModifier;
	static NLMISC::CVariable<float> HeavySleevesParryMinModifier;
	static NLMISC::CVariable<float> HeavySleevesParryMaxModifier;
	static NLMISC::CVariable<float> HeavyVestParryMinModifier;
	static NLMISC::CVariable<float> HeavyVestParryMaxModifier;
	static NLMISC::CVariable<float> HeavyHelmetParryMinModifier;
	static NLMISC::CVariable<float> HeavyHelmetParryMaxModifier;
	
	
	//AdversaryDodgeModifier not for ammo, jewel and armor
	// melee weapons
	static NLMISC::CVariable<float> DaggerAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> DaggerAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> SwordAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> SwordAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> MaceAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> MaceAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> AxeAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> AxeAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> SpearAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> SpearAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> StaffAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> StaffAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> MagicianStaffAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> MagicianStaffAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> TwoHandSwordAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> TwoHandSwordAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> TwoHandAxeAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> TwoHandAxeAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> PikeAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> PikeAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> TwoHandMaceAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> TwoHandMaceAdversaryDodgeMaxModifier;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> AutolauchAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> BowrifleAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> BowrifleAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> LauncherAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> LauncherAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> PistolAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> PistolAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> BowpistolAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> BowpistolAdversaryDodgeMaxModifier;
	static NLMISC::CVariable<float> RifleAdversaryDodgeMinModifier;
	static NLMISC::CVariable<float> RifleAdversaryDodgeMaxModifier;
	
	
	//AdversaryParryModifier not for ammo, jewel and armor
	// melee weapons
	static NLMISC::CVariable<float> DaggerAdversaryParryMinModifier;
	static NLMISC::CVariable<float> DaggerAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> SwordAdversaryParryMinModifier;
	static NLMISC::CVariable<float> SwordAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> MaceAdversaryParryMinModifier;
	static NLMISC::CVariable<float> MaceAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> AxeAdversaryParryMinModifier;
	static NLMISC::CVariable<float> AxeAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> SpearAdversaryParryMinModifier;
	static NLMISC::CVariable<float> SpearAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> StaffAdversaryParryMinModifier;
	static NLMISC::CVariable<float> StaffAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> MagicianStaffAdversaryParryMinModifier;
	static NLMISC::CVariable<float> MagicianStaffAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> TwoHandSwordAdversaryParryMinModifier;
	static NLMISC::CVariable<float> TwoHandSwordAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> TwoHandAxeAdversaryParryMinModifier;
	static NLMISC::CVariable<float> TwoHandAxeAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> PikeAdversaryParryMinModifier;
	static NLMISC::CVariable<float> PikeAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> TwoHandMaceAdversaryParryMinModifier;
	static NLMISC::CVariable<float> TwoHandMaceAdversaryParryMaxModifier;
	
	// range weapon
	static NLMISC::CVariable<float> AutolauchAdversaryParryMinModifier;
	static NLMISC::CVariable<float> AutolauchAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> BowrifleAdversaryParryMinModifier;
	static NLMISC::CVariable<float> BowrifleAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> LauncherAdversaryParryMinModifier;
	static NLMISC::CVariable<float> LauncherAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> PistolAdversaryParryMinModifier;
	static NLMISC::CVariable<float> PistolAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> BowpistolAdversaryParryMinModifier;
	static NLMISC::CVariable<float> BowpistolAdversaryParryMaxModifier;
	static NLMISC::CVariable<float> RifleAdversaryParryMinModifier;
	static NLMISC::CVariable<float> RifleAdversaryParryMaxModifier;

	//Elemental casting time factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerElementalCastingTimeFactor;
	static NLMISC::CVariable<float> SwordElementalCastingTimeFactor;
	static NLMISC::CVariable<float> AxeElementalCastingTimeFactor;
	static NLMISC::CVariable<float> MaceElementalCastingTimeFactor;
	static NLMISC::CVariable<float> SpearElementalCastingTimeFactor;
	static NLMISC::CVariable<float> StaffElementalCastingTimeFactor;
	static NLMISC::CVariable<float> MagicianStaffElementalCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandAxeElementalCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandSwordElementalCastingTimeFactor;
	static NLMISC::CVariable<float> PikeElementalCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandMaceElementalCastingTimeFactor;
	// Max
	static NLMISC::CVariable<float> DaggerElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> SwordElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> AxeElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> MaceElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> SpearElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> StaffElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> MagicianStaffElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> PikeElementalCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceElementalCastingTimeFactorMax;

	//Elemental power factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerElementalPowerFactor;
	static NLMISC::CVariable<float> SwordElementalPowerFactor;
	static NLMISC::CVariable<float> AxeElementalPowerFactor;
	static NLMISC::CVariable<float> MaceElementalPowerFactor;
	static NLMISC::CVariable<float> SpearElementalPowerFactor;
	static NLMISC::CVariable<float> StaffElementalPowerFactor;
	static NLMISC::CVariable<float> MagicianStaffElementalPowerFactor;
	static NLMISC::CVariable<float> TwoHandAxeElementalPowerFactor;
	static NLMISC::CVariable<float> TwoHandSwordElementalPowerFactor;
	static NLMISC::CVariable<float> PikeElementalPowerFactor;
	static NLMISC::CVariable<float> TwoHandMaceElementalPowerFactor;
	// Max
	static NLMISC::CVariable<float> DaggerElementalPowerFactorMax;
	static NLMISC::CVariable<float> SwordElementalPowerFactorMax;
	static NLMISC::CVariable<float> AxeElementalPowerFactorMax;
	static NLMISC::CVariable<float> MaceElementalPowerFactorMax;
	static NLMISC::CVariable<float> SpearElementalPowerFactorMax;
	static NLMISC::CVariable<float> StaffElementalPowerFactorMax;
	static NLMISC::CVariable<float> MagicianStaffElementalPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeElementalPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordElementalPowerFactorMax;
	static NLMISC::CVariable<float> PikeElementalPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceElementalPowerFactorMax;

	//OffensiveAffliction casting time factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> SwordOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> AxeOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> MaceOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> SpearOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> StaffOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> MagicianStaffOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandAxeOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandSwordOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> PikeOffensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandMaceOffensiveAfflictionCastingTimeFactor;
	// Max
	static NLMISC::CVariable<float> DaggerOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> SwordOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> AxeOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> MaceOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> SpearOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> StaffOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> MagicianStaffOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> PikeOffensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceOffensiveAfflictionCastingTimeFactorMax;
	
	//OffensiveAffliction power factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> SwordOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> AxeOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> MaceOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> SpearOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> StaffOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> MagicianStaffOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> TwoHandAxeOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> TwoHandSwordOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> PikeOffensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> TwoHandMaceOffensiveAfflictionPowerFactor;
	// Max
	static NLMISC::CVariable<float> DaggerOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> SwordOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> AxeOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> MaceOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> SpearOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> StaffOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> MagicianStaffOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> PikeOffensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceOffensiveAfflictionPowerFactorMax;

	//Heal casting time factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerHealCastingTimeFactor;
	static NLMISC::CVariable<float> SwordHealCastingTimeFactor;
	static NLMISC::CVariable<float> AxeHealCastingTimeFactor;
	static NLMISC::CVariable<float> MaceHealCastingTimeFactor;
	static NLMISC::CVariable<float> SpearHealCastingTimeFactor;
	static NLMISC::CVariable<float> StaffHealCastingTimeFactor;
	static NLMISC::CVariable<float> MagicianStaffHealCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandAxeHealCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandSwordHealCastingTimeFactor;
	static NLMISC::CVariable<float> PikeHealCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandMaceHealCastingTimeFactor;
	// Max
	static NLMISC::CVariable<float> DaggerHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> SwordHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> AxeHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> MaceHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> SpearHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> StaffHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> MagicianStaffHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> PikeHealCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceHealCastingTimeFactorMax;
	
	//Heal power factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerHealPowerFactor;
	static NLMISC::CVariable<float> SwordHealPowerFactor;
	static NLMISC::CVariable<float> AxeHealPowerFactor;
	static NLMISC::CVariable<float> MaceHealPowerFactor;
	static NLMISC::CVariable<float> SpearHealPowerFactor;
	static NLMISC::CVariable<float> StaffHealPowerFactor;
	static NLMISC::CVariable<float> MagicianStaffHealPowerFactor;
	static NLMISC::CVariable<float> TwoHandAxeHealPowerFactor;
	static NLMISC::CVariable<float> TwoHandSwordHealPowerFactor;
	static NLMISC::CVariable<float> PikeHealPowerFactor;
	static NLMISC::CVariable<float> TwoHandMaceHealPowerFactor;
	// Max
	static NLMISC::CVariable<float> DaggerHealPowerFactorMax;
	static NLMISC::CVariable<float> SwordHealPowerFactorMax;
	static NLMISC::CVariable<float> AxeHealPowerFactorMax;
	static NLMISC::CVariable<float> MaceHealPowerFactorMax;
	static NLMISC::CVariable<float> SpearHealPowerFactorMax;
	static NLMISC::CVariable<float> StaffHealPowerFactorMax;
	static NLMISC::CVariable<float> MagicianStaffHealPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeHealPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordHealPowerFactorMax;
	static NLMISC::CVariable<float> PikeHealPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceHealPowerFactorMax;

	//DefensiveAffliction casting time factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> SwordDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> AxeDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> MaceDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> SpearDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> StaffDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> MagicianStaffDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandAxeDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandSwordDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> PikeDefensiveAfflictionCastingTimeFactor;
	static NLMISC::CVariable<float> TwoHandMaceDefensiveAfflictionCastingTimeFactor;
	// Max
	static NLMISC::CVariable<float> DaggerDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> SwordDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> AxeDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> MaceDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> SpearDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> StaffDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> MagicianStaffDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> PikeDefensiveAfflictionCastingTimeFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceDefensiveAfflictionCastingTimeFactorMax;
	
	//DefensiveAffliction power factor (melee weapon only)
	// Min
	static NLMISC::CVariable<float> DaggerDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> SwordDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> AxeDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> MaceDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> SpearDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> StaffDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> MagicianStaffDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> TwoHandAxeDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> TwoHandSwordDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> PikeDefensiveAfflictionPowerFactor;
	static NLMISC::CVariable<float> TwoHandMaceDefensiveAfflictionPowerFactor;
	// Max
	static NLMISC::CVariable<float> DaggerDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> SwordDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> AxeDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> MaceDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> SpearDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> StaffDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> MagicianStaffDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandAxeDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandSwordDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> PikeDefensiveAfflictionPowerFactorMax;
	static NLMISC::CVariable<float> TwoHandMaceDefensiveAfflictionPowerFactorMax;

	//////////////////////////////////////////////////////////////////////////
	// Armor factors
	//////////////////////////////////////////////////////////////////////////
	// ProtectionFactor
	// armor and shield
	// Min
	static NLMISC::CVariable<float> ShieldProtectionFactor;
	static NLMISC::CVariable<float> BucklerProtectionFactor;
	static NLMISC::CVariable<float> LightBootsProtectionFactor;
	static NLMISC::CVariable<float> LightGlovesProtectionFactor;
	static NLMISC::CVariable<float> LightPantsProtectionFactor;
	static NLMISC::CVariable<float> LightSleevesProtectionFactor;
	static NLMISC::CVariable<float> LightVestProtectionFactor;
	static NLMISC::CVariable<float> MediumBootsProtectionFactor;
	static NLMISC::CVariable<float> MediumGlovesProtectionFactor;
	static NLMISC::CVariable<float> MediumPantsProtectionFactor;
	static NLMISC::CVariable<float> MediumSleevesProtectionFactor;
	static NLMISC::CVariable<float> MediumVestProtectionFactor;
	static NLMISC::CVariable<float> HeavyBootsProtectionFactor;
	static NLMISC::CVariable<float> HeavyGlovesProtectionFactor;
	static NLMISC::CVariable<float> HeavyPantsProtectionFactor;
	static NLMISC::CVariable<float> HeavySleevesProtectionFactor;
	static NLMISC::CVariable<float> HeavyVestProtectionFactor;
	static NLMISC::CVariable<float> HeavyHelmetProtectionFactor;
	// Max
	static NLMISC::CVariable<float> ShieldProtectionFactorMax;
	static NLMISC::CVariable<float> BucklerProtectionFactorMax;
	static NLMISC::CVariable<float> LightBootsProtectionFactorMax;
	static NLMISC::CVariable<float> LightGlovesProtectionFactorMax;
	static NLMISC::CVariable<float> LightPantsProtectionFactorMax;
	static NLMISC::CVariable<float> LightSleevesProtectionFactorMax;
	static NLMISC::CVariable<float> LightVestProtectionFactorMax;
	static NLMISC::CVariable<float> MediumBootsProtectionFactorMax;
	static NLMISC::CVariable<float> MediumGlovesProtectionFactorMax;
	static NLMISC::CVariable<float> MediumPantsProtectionFactorMax;
	static NLMISC::CVariable<float> MediumSleevesProtectionFactorMax;
	static NLMISC::CVariable<float> MediumVestProtectionFactorMax;
	static NLMISC::CVariable<float> HeavyBootsProtectionFactorMax;
	static NLMISC::CVariable<float> HeavyGlovesProtectionFactorMax;
	static NLMISC::CVariable<float> HeavyPantsProtectionFactorMax;
	static NLMISC::CVariable<float> HeavySleevesProtectionFactorMax;
	static NLMISC::CVariable<float> HeavyVestProtectionFactorMax;
	static NLMISC::CVariable<float> HeavyHelmetProtectionFactorMax;
	
	// MaxSlashingProtection
	// armor and shield
	static NLMISC::CVariable<float> ShieldMaxSlashingProtection;
	static NLMISC::CVariable<float> BucklerMaxSlashingProtection;
	static NLMISC::CVariable<float> LightBootsMaxSlashingProtection;
	static NLMISC::CVariable<float> LightGlovesMaxSlashingProtection;
	static NLMISC::CVariable<float> LightPantsMaxSlashingProtection;
	static NLMISC::CVariable<float> LightSleevesMaxSlashingProtection;
	static NLMISC::CVariable<float> LightVestMaxSlashingProtection;
	static NLMISC::CVariable<float> MediumBootsMaxSlashingProtection;
	static NLMISC::CVariable<float> MediumGlovesMaxSlashingProtection;
	static NLMISC::CVariable<float> MediumPantsMaxSlashingProtection;
	static NLMISC::CVariable<float> MediumSleevesMaxSlashingProtection;
	static NLMISC::CVariable<float> MediumVestMaxSlashingProtection;
	static NLMISC::CVariable<float> HeavyBootsMaxSlashingProtection;
	static NLMISC::CVariable<float> HeavyGlovesMaxSlashingProtection;
	static NLMISC::CVariable<float> HeavyPantsMaxSlashingProtection;
	static NLMISC::CVariable<float> HeavySleevesMaxSlashingProtection;
	static NLMISC::CVariable<float> HeavyVestMaxSlashingProtection;
	static NLMISC::CVariable<float> HeavyHelmetMaxSlashingProtection;
	
	// MaxBluntProtection
	// armor and shield
	static NLMISC::CVariable<float> ShieldMaxBluntProtection;
	static NLMISC::CVariable<float> BucklerMaxBluntProtection;
	static NLMISC::CVariable<float> LightBootsMaxBluntProtection;
	static NLMISC::CVariable<float> LightGlovesMaxBluntProtection;
	static NLMISC::CVariable<float> LightPantsMaxBluntProtection;
	static NLMISC::CVariable<float> LightSleevesMaxBluntProtection;
	static NLMISC::CVariable<float> LightVestMaxBluntProtection;
	static NLMISC::CVariable<float> MediumBootsMaxBluntProtection;
	static NLMISC::CVariable<float> MediumGlovesMaxBluntProtection;
	static NLMISC::CVariable<float> MediumPantsMaxBluntProtection;
	static NLMISC::CVariable<float> MediumSleevesMaxBluntProtection;
	static NLMISC::CVariable<float> MediumVestMaxBluntProtection;
	static NLMISC::CVariable<float> HeavyBootsMaxBluntProtection;
	static NLMISC::CVariable<float> HeavyGlovesMaxBluntProtection;
	static NLMISC::CVariable<float> HeavyPantsMaxBluntProtection;
	static NLMISC::CVariable<float> HeavySleevesMaxBluntProtection;
	static NLMISC::CVariable<float> HeavyVestMaxBluntProtection;
	static NLMISC::CVariable<float> HeavyHelmetMaxBluntProtection;
	
	// MaxPiercingProtection
	// armor and shield
	static NLMISC::CVariable<float> ShieldMaxPiercingProtection;
	static NLMISC::CVariable<float> BucklerMaxPiercingProtection;
	static NLMISC::CVariable<float> LightBootsMaxPiercingProtection;
	static NLMISC::CVariable<float> LightGlovesMaxPiercingProtection;
	static NLMISC::CVariable<float> LightPantsMaxPiercingProtection;
	static NLMISC::CVariable<float> LightSleevesMaxPiercingProtection;
	static NLMISC::CVariable<float> LightVestMaxPiercingProtection;
	static NLMISC::CVariable<float> MediumBootsMaxPiercingProtection;
	static NLMISC::CVariable<float> MediumGlovesMaxPiercingProtection;
	static NLMISC::CVariable<float> MediumPantsMaxPiercingProtection;
	static NLMISC::CVariable<float> MediumSleevesMaxPiercingProtection;
	static NLMISC::CVariable<float> MediumVestMaxPiercingProtection;
	static NLMISC::CVariable<float> HeavyBootsMaxPiercingProtection;
	static NLMISC::CVariable<float> HeavyGlovesMaxPiercingProtection;
	static NLMISC::CVariable<float> HeavyPantsMaxPiercingProtection;
	static NLMISC::CVariable<float> HeavySleevesMaxPiercingProtection;
	static NLMISC::CVariable<float> HeavyVestMaxPiercingProtection;
	static NLMISC::CVariable<float> HeavyHelmetMaxPiercingProtection;

	// Jewel Magic Protection
	static NLMISC::CVariable<float> AcidJewelProtection;
	static NLMISC::CVariable<float> ColdJewelProtection;
	static NLMISC::CVariable<float> FireJewelProtection;
	static NLMISC::CVariable<float> RotJewelProtection;
	static NLMISC::CVariable<float> ShockWaveJewelProtection;
	static NLMISC::CVariable<float> PoisonJewelProtection;
	static NLMISC::CVariable<float> ElectricityJewelProtection;

	// Jewel Magic Resistance
	static NLMISC::CVariable<float> DesertResistance;
	static NLMISC::CVariable<float> ForestResistance;
	static NLMISC::CVariable<float> LacustreResistance;
	static NLMISC::CVariable<float> JungleResistance;
	static NLMISC::CVariable<float> PrimaryRootResistance;
};

#endif // RY_CRAFT_PARAMETERS

/* End of weapon_craft_parameters.h */



