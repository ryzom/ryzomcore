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
															 
#include "game_item_manager/weapon_craft_parameters.h"


	//////////////////////////////////////////////////////////////////////////
	// Common factors
	//////////////////////////////////////////////////////////////////////////
	//Durability
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDurability("egs_craft", "DaggerDurability", "Variable DaggerDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDurability("egs_craft", "SwordDurability", "Variable SwordDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDurability("egs_craft", "MaceDurability", "Variable MaceDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDurability("egs_craft", "AxeDurability", "Variable AxeDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDurability("egs_craft", "SpearDurability", "Variable SpearDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDurability("egs_craft", "StaffDurability", "Variable StaffDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDurability("egs_craft", "MagicianStaffDurability", "Variable MagicianStaffDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDurability("egs_craft", "TwoHandSwordDurability", "Variable TwoHandSwordDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDurability("egs_craft", "TwoHandAxeDurability", "Variable TwoHandAxeDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDurability("egs_craft", "PikeDurability", "Variable PikeDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDurability("egs_craft", "TwoHandMaceDurability", "Variable TwoHandMaceDurability", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchDurability("egs_craft", "AutolauchDurability", "Variable AutolauchDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleDurability("egs_craft", "BowrifleDurability", "Variable BowrifleDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherDurability("egs_craft", "LauncherDurability", "Variable LauncherDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolDurability("egs_craft", "PistolDurability", "Variable PistolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolDurability("egs_craft", "BowpistolDurability", "Variable BowpistolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleDurability("egs_craft", "RifleDurability", "Variable RifleDurability", 0.0f, 0, true);
	
	// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoDurability("egs_craft", "AutolaunchAmmoDurability", "Variable AutolaunchAmmoDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoDurability("egs_craft", "BowrifleAmmoDurability", "Variable BowrifleAmmoDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoDurability("egs_craft", "LauncherAmmoDurability", "Variable LauncherAmmoDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoDurability("egs_craft", "PistolAmmoDurability", "Variable PistolAmmoDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoDurability("egs_craft", "BowpistolAmmoDurability", "Variable BowpistolAmmoDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoDurability("egs_craft", "RifleAmmoDurability", "Variable RifleAmmoDurability", 0.0f, 0, true);
	
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldDurability("egs_craft", "ShieldDurability", "Variable ShieldDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerDurability("egs_craft", "BucklerDurability", "Variable BucklerDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsDurability("egs_craft", "LightBootsDurability", "Variable LightBootsDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesDurability("egs_craft", "LightGlovesDurability", "Variable LightGlovesDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsDurability("egs_craft", "LightPantsDurability", "Variable LightPantsDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesDurability("egs_craft", "LightSleevesDurability", "Variable LightSleevesDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestDurability("egs_craft", "LightVestDurability", "Variable LightVestDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsDurability("egs_craft", "MediumBootsDurability", "Variable MediumBootsDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesDurability("egs_craft", "MediumGlovesDurability", "Variable MediumGlovesDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsDurability("egs_craft", "MediumPantsDurability", "Variable MediumPantsDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesDurability("egs_craft", "MediumSleevesDurability", "Variable MediumSleevesDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestDurability("egs_craft", "MediumVestDurability", "Variable MediumVestDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsDurability("egs_craft", "HeavyBootsDurability", "Variable HeavyBootsDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesDurability("egs_craft", "HeavyGlovesDurability", "Variable HeavyGlovesDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsDurability("egs_craft", "HeavyPantsDurability", "Variable HeavyPantsDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesDurability("egs_craft", "HeavySleevesDurability", "Variable HeavySleevesDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestDurability("egs_craft", "HeavyVestDurability", "Variable HeavyVestDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetDurability("egs_craft", "HeavyHelmetDurability", "Variable HeavyHelmetDurability", 0.0f, 0, true);
	
	// jewel
NLMISC::CVariable<float> CWeaponCraftParameters::AnkletDurability("egs_craft", "AnkletDurability", "Variable AnkletDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BraceletDurability("egs_craft", "BraceletDurability", "Variable BraceletDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DiademDurability("egs_craft", "DiademDurability", "Variable DiademDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::EaringDurability("egs_craft", "EaringDurability", "Variable EaringDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PendantDurability("egs_craft", "PendantDurability", "Variable PendantDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RingDurability("egs_craft", "RingDurability", "Variable RingDurability", 0.0f, 0, true);

	// tool
NLMISC::CVariable<float> CWeaponCraftParameters::ForageToolDurability("egs_craft", "ForageToolDurability", "Variable ForageToolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AmmoCraftingToolDurability("egs_craft", "AmmoCraftingToolDurability", "Variable AmmoCraftingToolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ArmorCraftingToolDurability("egs_craft", "ArmorCraftingToolDurability", "Variable ArmorCraftingToolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::JewelryCraftingToolDurability("egs_craft", "JewelryCraftingToolDurability", "Variable JewelryCraftingToolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RangeWeaponCraftingToolDurability("egs_craft", "RangeWeaponCraftingToolDurability", "Variable RangeWeaponCraftingToolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MeleeWeaponCraftingToolDurability("egs_craft", "MeleeWeaponCraftingToolDurability", "Variable MeleeWeaponCraftingToolDurability", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ToolCraftingToolDurability("egs_craft", "ToolCraftingToolDurability", "Variable ToolCraftingToolDurability", 0.0f, 0, true);
		
		
	//Weight
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerWeight("egs_craft", "DaggerWeight", "Variable DaggerWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordWeight("egs_craft", "SwordWeight", "Variable SwordWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceWeight("egs_craft", "MaceWeight", "Variable MaceWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeWeight("egs_craft", "AxeWeight", "Variable AxeWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearWeight("egs_craft", "SpearWeight", "Variable SpearWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffWeight("egs_craft", "StaffWeight", "Variable StaffWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffWeight("egs_craft", "MagicianStaffWeight", "Variable MagicianStaffWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordWeight("egs_craft", "TwoHandSwordWeight", "Variable TwoHandSwordWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeWeight("egs_craft", "TwoHandAxeWeight", "Variable TwoHandAxeWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeWeight("egs_craft", "PikeWeight", "Variable PikeWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceWeight("egs_craft", "TwoHandMaceWeight", "Variable TwoHandMaceWeight", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchWeight("egs_craft", "AutolauchWeight", "Variable AutolauchWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleWeight("egs_craft", "BowrifleWeight", "Variable BowrifleWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherWeight("egs_craft", "LauncherWeight", "Variable LauncherWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolWeight("egs_craft", "PistolWeight", "Variable PistolWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolWeight("egs_craft", "BowpistolWeight", "Variable BowpistolWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleWeight("egs_craft", "RifleWeight", "Variable RifleWeight", 0.0f, 0, true);
	
	// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoWeight("egs_craft", "AutolaunchAmmoWeight", "Variable AutolaunchAmmoWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoWeight("egs_craft", "BowrifleAmmoWeight", "Variable BowrifleAmmoWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoWeight("egs_craft", "LauncherAmmoWeight", "Variable LauncherAmmoWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoWeight("egs_craft", "PistolAmmoWeight", "Variable PistolAmmoWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoWeight("egs_craft", "BowpistolAmmoWeight", "Variable BowpistolAmmoWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoWeight("egs_craft", "RifleAmmoWeight", "Variable RifleAmmoWeight", 0.0f, 0, true);
	
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldWeight("egs_craft", "ShieldWeight", "Variable ShieldWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerWeight("egs_craft", "BucklerWeight", "Variable BucklerWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsWeight("egs_craft", "LightBootsWeight", "Variable LightBootsWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesWeight("egs_craft", "LightGlovesWeight", "Variable LightGlovesWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsWeight("egs_craft", "LightPantsWeight", "Variable LightPantsWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesWeight("egs_craft", "LightSleevesWeight", "Variable LightSleevesWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestWeight("egs_craft", "LightVestWeight", "Variable LightVestWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsWeight("egs_craft", "MediumBootsWeight", "Variable MediumBootsWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesWeight("egs_craft", "MediumGlovesWeight", "Variable MediumGlovesWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsWeight("egs_craft", "MediumPantsWeight", "Variable MediumPantsWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesWeight("egs_craft", "MediumSleevesWeight", "Variable MediumSleevesWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestWeight("egs_craft", "MediumVestWeight", "Variable MediumVestWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsWeight("egs_craft", "HeavyBootsWeight", "Variable HeavyBootsWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesWeight("egs_craft", "HeavyGlovesWeight", "Variable HeavyGlovesWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsWeight("egs_craft", "HeavyPantsWeight", "Variable HeavyPantsWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesWeight("egs_craft", "HeavySleevesWeight", "Variable HeavySleevesWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestWeight("egs_craft", "HeavyVestWeight", "Variable HeavyVestWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetWeight("egs_craft", "HeavyHelmetWeight", "Variable HeavyHelmetWeight", 0.0f, 0, true);
	
	// jewel
NLMISC::CVariable<float> CWeaponCraftParameters::AnkletWeight("egs_craft", "AnkletWeight", "Variable AnkletWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BraceletWeight("egs_craft", "BraceletWeight", "Variable BraceletWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DiademWeight("egs_craft", "DiademWeight", "Variable DiademWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::EaringWeight("egs_craft", "EaringWeight", "Variable EaringWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PendantWeight("egs_craft", "PendantWeight", "Variable PendantWeight", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RingWeight("egs_craft", "RingWeight", "Variable RingWeight", 0.0f, 0, true);
	
	//SapLoad Min
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerSapLoad("egs_craft", "DaggerSapLoad", "Variable DaggerSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordSapLoad("egs_craft", "SwordSapLoad", "Variable SwordSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceSapLoad("egs_craft", "MaceSapLoad", "Variable MaceSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeSapLoad("egs_craft", "AxeSapLoad", "Variable AxeSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearSapLoad("egs_craft", "SpearSapLoad", "Variable SpearSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffSapLoad("egs_craft", "StaffSapLoad", "Variable StaffSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffSapLoad("egs_craft", "MagicianStaffSapLoad", "Variable MagicianStaffSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordSapLoad("egs_craft", "TwoHandSwordSapLoad", "Variable TwoHandSwordSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeSapLoad("egs_craft", "TwoHandAxeSapLoad", "Variable TwoHandAxeSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeSapLoad("egs_craft", "PikeSapLoad", "Variable PikeSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceSapLoad("egs_craft", "TwoHandMaceSapLoad", "Variable TwoHandMaceSapLoad", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchSapLoad("egs_craft", "AutolauchSapLoad", "Variable AutolauchSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleSapLoad("egs_craft", "BowrifleSapLoad", "Variable BowrifleSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherSapLoad("egs_craft", "LauncherSapLoad", "Variable LauncherSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolSapLoad("egs_craft", "PistolSapLoad", "Variable PistolSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolSapLoad("egs_craft", "BowpistolSapLoad", "Variable BowpistolSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleSapLoad("egs_craft", "RifleSapLoad", "Variable RifleSapLoad", 0.0f, 0, true);
	
	// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoSapLoad("egs_craft", "AutolaunchAmmoSapLoad", "Variable AutolaunchAmmoSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoSapLoad("egs_craft", "BowrifleAmmoSapLoad", "Variable BowrifleAmmoSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoSapLoad("egs_craft", "LauncherAmmoSapLoad", "Variable LauncherAmmoSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoSapLoad("egs_craft", "PistolAmmoSapLoad", "Variable PistolAmmoSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoSapLoad("egs_craft", "BowpistolAmmoSapLoad", "Variable BowpistolAmmoSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoSapLoad("egs_craft", "RifleAmmoSapLoad", "Variable RifleAmmoSapLoad", 0.0f, 0, true);
	
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldSapLoad("egs_craft", "ShieldSapLoad", "Variable ShieldSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerSapLoad("egs_craft", "BucklerSapLoad", "Variable BucklerSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsSapLoad("egs_craft", "LightBootsSapLoad", "Variable LightBootsSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesSapLoad("egs_craft", "LightGlovesSapLoad", "Variable LightGlovesSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsSapLoad("egs_craft", "LightPantsSapLoad", "Variable LightPantsSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesSapLoad("egs_craft", "LightSleevesSapLoad", "Variable LightSleevesSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestSapLoad("egs_craft", "LightVestSapLoad", "Variable LightVestSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsSapLoad("egs_craft", "MediumBootsSapLoad", "Variable MediumBootsSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesSapLoad("egs_craft", "MediumGlovesSapLoad", "Variable MediumGlovesSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsSapLoad("egs_craft", "MediumPantsSapLoad", "Variable MediumPantsSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesSapLoad("egs_craft", "MediumSleevesSapLoad", "Variable MediumSleevesSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestSapLoad("egs_craft", "MediumVestSapLoad", "Variable MediumVestSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsSapLoad("egs_craft", "HeavyBootsSapLoad", "Variable HeavyBootsSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesSapLoad("egs_craft", "HeavyGlovesSapLoad", "Variable HeavyGlovesSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsSapLoad("egs_craft", "HeavyPantsSapLoad", "Variable HeavyPantsSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesSapLoad("egs_craft", "HeavySleevesSapLoad", "Variable HeavySleevesSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestSapLoad("egs_craft", "HeavyVestSapLoad", "Variable HeavyVestSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetSapLoad("egs_craft", "HeavyHelmetSapLoad", "Variable HeavyHelmetSapLoad", 0.0f, 0, true);
	
	// jewel
NLMISC::CVariable<float> CWeaponCraftParameters::AnkletSapLoad("egs_craft", "AnkletSapLoad", "Variable AnkletSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BraceletSapLoad("egs_craft", "BraceletSapLoad", "Variable BraceletSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DiademSapLoad("egs_craft", "DiademSapLoad", "Variable DiademSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::EaringSapLoad("egs_craft", "EaringSapLoad", "Variable EaringSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PendantSapLoad("egs_craft", "PendantSapLoad", "Variable PendantSapLoad", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RingSapLoad("egs_craft", "RingSapLoad", "Variable RingSapLoad", 0.0f, 0, true);

//SapLoad Max
// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerSapLoadMax("egs_craft", "DaggerSapLoadMax", "Variable DaggerSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordSapLoadMax("egs_craft", "SwordSapLoadMax", "Variable SwordSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceSapLoadMax("egs_craft", "MaceSapLoadMax", "Variable MaceSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeSapLoadMax("egs_craft", "AxeSapLoadMax", "Variable AxeSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearSapLoadMax("egs_craft", "SpearSapLoadMax", "Variable SpearSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffSapLoadMax("egs_craft", "StaffSapLoadMax", "Variable StaffSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffSapLoadMax("egs_craft", "MagicianStaffSapLoadMax", "Variable MagicianStaffSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordSapLoadMax("egs_craft", "TwoHandSwordSapLoadMax", "Variable TwoHandSwordSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeSapLoadMax("egs_craft", "TwoHandAxeSapLoadMax", "Variable TwoHandAxeSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeSapLoadMax("egs_craft", "PikeSapLoadMax", "Variable PikeSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceSapLoadMax("egs_craft", "TwoHandMaceSapLoadMax", "Variable TwoHandMaceSapLoad Max", 0.0f, 0, true);

// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchSapLoadMax("egs_craft", "AutolauchSapLoadMax", "Variable AutolauchSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleSapLoadMax("egs_craft", "BowrifleSapLoadMax", "Variable BowrifleSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherSapLoadMax("egs_craft", "LauncherSapLoadMax", "Variable LauncherSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolSapLoadMax("egs_craft", "PistolSapLoadMax", "Variable PistolSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolSapLoadMax("egs_craft", "BowpistolSapLoadMax", "Variable BowpistolSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleSapLoadMax("egs_craft", "RifleSapLoadMax", "Variable RifleSapLoad Max", 0.0f, 0, true);

// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoSapLoadMax("egs_craft", "AutolaunchAmmoSapLoadMax", "Variable AutolaunchAmmoSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoSapLoadMax("egs_craft", "BowrifleAmmoSapLoadMax", "Variable BowrifleAmmoSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoSapLoadMax("egs_craft", "LauncherAmmoSapLoadMax", "Variable LauncherAmmoSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoSapLoadMax("egs_craft", "PistolAmmoSapLoadMax", "Variable PistolAmmoSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoSapLoadMax("egs_craft", "BowpistolAmmoSapLoadMax", "Variable BowpistolAmmoSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoSapLoadMax("egs_craft", "RifleAmmoSapLoadMax", "Variable RifleAmmoSapLoad Max", 0.0f, 0, true);

// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldSapLoadMax("egs_craft", "ShieldSapLoadMax", "Variable ShieldSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerSapLoadMax("egs_craft", "BucklerSapLoadMax", "Variable BucklerSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsSapLoadMax("egs_craft", "LightBootsSapLoadMax", "Variable LightBootsSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesSapLoadMax("egs_craft", "LightGlovesSapLoadMax", "Variable LightGlovesSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsSapLoadMax("egs_craft", "LightPantsSapLoadMax", "Variable LightPantsSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesSapLoadMax("egs_craft", "LightSleevesSapLoadMax", "Variable LightSleevesSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestSapLoadMax("egs_craft", "LightVestSapLoadMax", "Variable LightVestSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsSapLoadMax("egs_craft", "MediumBootsSapLoadMax", "Variable MediumBootsSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesSapLoadMax("egs_craft", "MediumGlovesSapLoadMax", "Variable MediumGlovesSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsSapLoadMax("egs_craft", "MediumPantsSapLoadMax", "Variable MediumPantsSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesSapLoadMax("egs_craft", "MediumSleevesSapLoadMax", "Variable MediumSleevesSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestSapLoadMax("egs_craft", "MediumVestSapLoadMax", "Variable MediumVestSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsSapLoadMax("egs_craft", "HeavyBootsSapLoadMax", "Variable HeavyBootsSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesSapLoadMax("egs_craft", "HeavyGlovesSapLoadMax", "Variable HeavyGlovesSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsSapLoadMax("egs_craft", "HeavyPantsSapLoadMax", "Variable HeavyPantsSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesSapLoadMax("egs_craft", "HeavySleevesSapLoadMax", "Variable HeavySleevesSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestSapLoadMax("egs_craft", "HeavyVestSapLoadMax", "Variable HeavyVestSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetSapLoadMax("egs_craft", "HeavyHelmetSapLoadMax", "Variable HeavyHelmetSapLoad Max", 0.0f, 0, true);

// jewel
NLMISC::CVariable<float> CWeaponCraftParameters::AnkletSapLoadMax("egs_craft", "AnkletSapLoadMax", "Variable AnkletSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BraceletSapLoadMax("egs_craft", "BraceletSapLoadMax", "Variable BraceletSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DiademSapLoadMax("egs_craft", "DiademSapLoadMax", "Variable DiademSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::EaringSapLoadMax("egs_craft", "EaringSapLoadMax", "Variable EaringSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PendantSapLoadMax("egs_craft", "PendantSapLoadMax", "Variable PendantSapLoad Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RingSapLoadMax("egs_craft", "RingSapLoadMax", "Variable RingSapLoad Max", 0.0f, 0, true);

	//////////////////////////////////////////////////////////////////////////
	// Weapons factors
	//////////////////////////////////////////////////////////////////////////
	//Dmg Min for melee weapon, range weapon (modifier), ammo
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDmg("egs_craft", "DaggerDmg", "Variable DaggerDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDmg("egs_craft", "SwordDmg", "Variable SwordDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDmg("egs_craft", "MaceDmg", "Variable MaceDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDmg("egs_craft", "AxeDmg", "Variable AxeDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDmg("egs_craft", "SpearDmg", "Variable SpearDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDmg("egs_craft", "StaffDmg", "Variable StaffDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDmg("egs_craft", "MagicianStaffDmg", "Variable MagicianStaffDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDmg("egs_craft", "TwoHandSwordDmg", "Variable TwoHandSwordDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDmg("egs_craft", "TwoHandAxeDmg", "Variable TwoHandAxeDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDmg("egs_craft", "PikeDmg", "Variable PikeDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDmg("egs_craft", "TwoHandMaceDmg", "Variable TwoHandMaceDmg", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchDmg("egs_craft", "AutolauchDmg", "Variable AutolauchDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleDmg("egs_craft", "BowrifleDmg", "Variable BowrifleDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherDmg("egs_craft", "LauncherDmg", "Variable LauncherDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolDmg("egs_craft", "PistolDmg", "Variable PistolDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolDmg("egs_craft", "BowpistolDmg", "Variable BowpistolDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleDmg("egs_craft", "RifleDmg", "Variable RifleDmg", 0.0f, 0, true);
	
	// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoDmg("egs_craft", "AutolaunchAmmoDmg", "Variable AutolaunchAmmoDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoDmg("egs_craft", "BowrifleAmmoDmg", "Variable BowrifleAmmoDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoDmg("egs_craft", "LauncherAmmoDmg", "Variable LauncherAmmoDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoDmg("egs_craft", "PistolAmmoDmg", "Variable PistolAmmoDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoDmg("egs_craft", "BowpistolAmmoDmg", "Variable BowpistolAmmoDmg", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoDmg("egs_craft", "RifleAmmoDmg", "Variable RifleAmmoDmg", 0.0f, 0, true);

//Dmg Max for melee weapon, range weapon (modifier), ammo
// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDmgMax("egs_craft", "DaggerDmgMax", "Variable DaggerDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDmgMax("egs_craft", "SwordDmgMax", "Variable SwordDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDmgMax("egs_craft", "MaceDmgMax", "Variable MaceDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDmgMax("egs_craft", "AxeDmgMax", "Variable AxeDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDmgMax("egs_craft", "SpearDmgMax", "Variable SpearDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDmgMax("egs_craft", "StaffDmgMax", "Variable StaffDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDmgMax("egs_craft", "MagicianStaffDmgMax", "Variable MagicianStaffDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDmgMax("egs_craft", "TwoHandSwordDmgMax", "Variable TwoHandSwordDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDmgMax("egs_craft", "TwoHandAxeDmgMax", "Variable TwoHandAxeDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDmgMax("egs_craft", "PikeDmgMax", "Variable PikeDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDmgMax("egs_craft", "TwoHandMaceDmgMax", "Variable TwoHandMaceDmg Max", 0.0f, 0, true);

// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchDmgMax("egs_craft", "AutolauchDmgMax", "Variable AutolauchDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleDmgMax("egs_craft", "BowrifleDmgMax", "Variable BowrifleDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherDmgMax("egs_craft", "LauncherDmgMax", "Variable LauncherDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolDmgMax("egs_craft", "PistolDmgMax", "Variable PistolDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolDmgMax("egs_craft", "BowpistolDmgMax", "Variable BowpistolDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleDmgMax("egs_craft", "RifleDmgMax", "Variable RifleDmg Max", 0.0f, 0, true);

// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoDmgMax("egs_craft", "AutolaunchAmmoDmgMax", "Variable AutolaunchAmmoDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoDmgMax("egs_craft", "BowrifleAmmoDmgMax", "Variable BowrifleAmmoDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoDmgMax("egs_craft", "LauncherAmmoDmgMax", "Variable LauncherAmmoDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoDmgMax("egs_craft", "PistolAmmoDmgMax", "Variable PistolAmmoDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoDmgMax("egs_craft", "BowpistolAmmoDmgMax", "Variable BowpistolAmmoDmg Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoDmgMax("egs_craft", "RifleAmmoDmgMax", "Variable RifleAmmoDmg Max", 0.0f, 0, true);

	
//HitRate min for ammos (modifier), melee weapon, range weapon
// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerHitRate("egs_craft", "DaggerHitRate", "Variable DaggerHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordHitRate("egs_craft", "SwordHitRate", "Variable SwordHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceHitRate("egs_craft", "MaceHitRate", "Variable MaceHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeHitRate("egs_craft", "AxeHitRate", "Variable AxeHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearHitRate("egs_craft", "SpearHitRate", "Variable SpearHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffHitRate("egs_craft", "StaffHitRate", "Variable StaffHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffHitRate("egs_craft", "MagicianStaffHitRate", "Variable MagicianStaffHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordHitRate("egs_craft", "TwoHandSwordHitRate", "Variable TwoHandSwordHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeHitRate("egs_craft", "TwoHandAxeHitRate", "Variable TwoHandAxeHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeHitRate("egs_craft", "PikeHitRate", "Variable PikeHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceHitRate("egs_craft", "TwoHandMaceHitRate", "Variable TwoHandMaceHitRate", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchHitRate("egs_craft", "AutolauchHitRate", "Variable AutolauchHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleHitRate("egs_craft", "BowrifleHitRate", "Variable BowrifleHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherHitRate("egs_craft", "LauncherHitRate", "Variable LauncherHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolHitRate("egs_craft", "PistolHitRate", "Variable PistolHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolHitRate("egs_craft", "BowpistolHitRate", "Variable BowpistolHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleHitRate("egs_craft", "RifleHitRate", "Variable RifleHitRate", 0.0f, 0, true);

	// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoHitRate("egs_craft", "AutolaunchAmmoHitRate", "Variable AutolaunchAmmoHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoHitRate("egs_craft", "BowrifleAmmoHitRate", "Variable BowrifleAmmoHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoHitRate("egs_craft", "LauncherAmmoHitRate", "Variable LauncherAmmoHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoHitRate("egs_craft", "PistolAmmoHitRate", "Variable PistolAmmoHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoHitRate("egs_craft", "BowpistolAmmoHitRate", "Variable BowpistolAmmoHitRate", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoHitRate("egs_craft", "RifleAmmoHitRate", "Variable RifleAmmoHitRate", 0.0f, 0, true);

//HitRate max for ammos (modifier), melee weapon, range weapon
// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerHitRateMax("egs_craft", "DaggerHitRateMax", "Variable DaggerHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordHitRateMax("egs_craft", "SwordHitRateMax", "Variable SwordHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceHitRateMax("egs_craft", "MaceHitRateMax", "Variable MaceHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeHitRateMax("egs_craft", "AxeHitRateMax", "Variable AxeHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearHitRateMax("egs_craft", "SpearHitRateMax", "Variable SpearHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffHitRateMax("egs_craft", "StaffHitRateMax", "Variable StaffHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffHitRateMax("egs_craft", "MagicianStaffHitRateMax", "Variable MagicianStaffHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordHitRateMax("egs_craft", "TwoHandSwordHitRateMax", "Variable TwoHandSwordHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeHitRateMax("egs_craft", "TwoHandAxeHitRateMax", "Variable TwoHandAxeHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeHitRateMax("egs_craft", "PikeHitRateMax", "Variable PikeHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceHitRateMax("egs_craft", "TwoHandMaceHitRateMax", "Variable TwoHandMaceHitRate max", 0.0f, 0, true);

// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchHitRateMax("egs_craft", "AutolauchHitRateMax", "Variable AutolauchHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleHitRateMax("egs_craft", "BowrifleHitRateMax", "Variable BowrifleHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherHitRateMax("egs_craft", "LauncherHitRateMax", "Variable LauncherHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolHitRateMax("egs_craft", "PistolHitRateMax", "Variable PistolHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolHitRateMax("egs_craft", "BowpistolHitRateMax", "Variable BowpistolHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleHitRateMax("egs_craft", "RifleHitRateMax", "Variable RifleHitRate max", 0.0f, 0, true);

// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoHitRateMax("egs_craft", "AutolaunchAmmoHitRateMax", "Variable AutolaunchAmmoHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoHitRateMax("egs_craft", "BowrifleAmmoHitRateMax", "Variable BowrifleAmmoHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoHitRateMax("egs_craft", "LauncherAmmoHitRateMax", "Variable LauncherAmmoHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoHitRateMax("egs_craft", "PistolAmmoHitRateMax", "Variable PistolAmmoHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoHitRateMax("egs_craft", "BowpistolAmmoHitRateMax", "Variable BowpistolAmmoHitRate max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoHitRateMax("egs_craft", "RifleAmmoHitRateMax", "Variable RifleAmmoHitRate max", 0.0f, 0, true);
	
	
	//Range	for ammo, range weapon (modifier)
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchRange("egs_craft", "AutolauchRange", "Variable AutolauchRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleRange("egs_craft", "BowrifleRange", "Variable BowrifleRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherRange("egs_craft", "LauncherRange", "Variable LauncherRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolRange("egs_craft", "PistolRange", "Variable PistolRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolRange("egs_craft", "BowpistolRange", "Variable BowpistolRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleRange("egs_craft", "RifleRange", "Variable RifleRange", 0.0f, 0, true);
	
	// ammo
NLMISC::CVariable<float> CWeaponCraftParameters::AutolaunchAmmoRange("egs_craft", "AutolaunchAmmoRange", "Variable AutolaunchAmmoRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAmmoRange("egs_craft", "BowrifleAmmoRange", "Variable BowrifleAmmoRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAmmoRange("egs_craft", "LauncherAmmoRange", "Variable LauncherAmmoRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAmmoRange("egs_craft", "PistolAmmoRange", "Variable PistolAmmoRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAmmoRange("egs_craft", "BowpistolAmmoRange", "Variable BowpistolAmmoRange", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAmmoRange("egs_craft", "RifleAmmoRange", "Variable RifleAmmoRange", 0.0f, 0, true);
	
	
	//DodgeModifier not for ammo and jewel, but for armor too
	// melee weapons & armor
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDodgeMinModifier("egs_craft", "DaggerDodgeMinModifier", "Variable DaggerDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDodgeMaxModifier("egs_craft", "DaggerDodgeMaxModifier", "Variable DaggerDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDodgeMinModifier("egs_craft", "SwordDodgeMinModifier", "Variable SwordDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDodgeMaxModifier("egs_craft", "SwordDodgeMaxModifier", "Variable SwordDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDodgeMinModifier("egs_craft", "MaceDodgeMinModifier", "Variable MaceDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDodgeMaxModifier("egs_craft", "MaceDodgeMaxModifier", "Variable MaceDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDodgeMinModifier("egs_craft", "AxeDodgeMinModifier", "Variable AxeDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDodgeMaxModifier("egs_craft", "AxeDodgeMaxModifier", "Variable AxeDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDodgeMinModifier("egs_craft", "SpearDodgeMinModifier", "Variable SpearDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDodgeMaxModifier("egs_craft", "SpearDodgeMaxModifier", "Variable SpearDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDodgeMinModifier("egs_craft", "StaffDodgeMinModifier", "Variable StaffDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDodgeMaxModifier("egs_craft", "StaffDodgeMaxModifier", "Variable StaffDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDodgeMinModifier("egs_craft", "MagicianStaffDodgeMinModifier", "Variable MagicianStaffDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDodgeMaxModifier("egs_craft", "MagicianStaffDodgeMaxModifier", "Variable MagicianStaffDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDodgeMinModifier("egs_craft", "TwoHandSwordDodgeMinModifier", "Variable TwoHandSwordDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDodgeMaxModifier("egs_craft", "TwoHandSwordDodgeMaxModifier", "Variable TwoHandSwordDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDodgeMinModifier("egs_craft", "TwoHandAxeDodgeMinModifier", "Variable TwoHandAxeDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDodgeMaxModifier("egs_craft", "TwoHandAxeDodgeMaxModifier", "Variable TwoHandAxeDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDodgeMinModifier("egs_craft", "PikeDodgeMinModifier", "Variable PikeDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDodgeMaxModifier("egs_craft", "PikeDodgeMaxModifier", "Variable PikeDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDodgeMinModifier("egs_craft", "TwoHandMaceDodgeMinModifier", "Variable TwoHandMaceDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDodgeMaxModifier("egs_craft", "TwoHandMaceDodgeMaxModifier", "Variable TwoHandMaceDodgeMaxModifier", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchDodgeMinModifier("egs_craft", "AutolauchDodgeMinModifier", "Variable AutolauchDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchDodgeMaxModifier("egs_craft", "AutolauchDodgeMaxModifier", "Variable AutolauchDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleDodgeMinModifier("egs_craft", "BowrifleDodgeMinModifier", "Variable BowrifleDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleDodgeMaxModifier("egs_craft", "BowrifleDodgeMaxModifier", "Variable BowrifleDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherDodgeMinModifier("egs_craft", "LauncherDodgeMinModifier", "Variable LauncherDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherDodgeMaxModifier("egs_craft", "LauncherDodgeMaxModifier", "Variable LauncherDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolDodgeMinModifier("egs_craft", "PistolDodgeMinModifier", "Variable PistolDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolDodgeMaxModifier("egs_craft", "PistolDodgeMaxModifier", "Variable PistolDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolDodgeMinModifier("egs_craft", "BowpistolDodgeMinModifier", "Variable BowpistolDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolDodgeMaxModifier("egs_craft", "BowpistolDodgeMaxModifier", "Variable BowpistolDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleDodgeMinModifier("egs_craft", "RifleDodgeMinModifier", "Variable RifleDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleDodgeMaxModifier("egs_craft", "RifleDodgeMaxModifier", "Variable RifleDodgeMaxModifier", 0.0f, 0, true);
	
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldDodgeMinModifier("egs_craft", "ShieldDodgeMinModifier", "Variable ShieldDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldDodgeMaxModifier("egs_craft", "ShieldDodgeMaxModifier", "Variable ShieldDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerDodgeMinModifier("egs_craft", "BucklerDodgeMinModifier", "Variable BucklerDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerDodgeMaxModifier("egs_craft", "BucklerDodgeMaxModifier", "Variable BucklerDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsDodgeMinModifier("egs_craft", "LightBootsDodgeMinModifier", "Variable LightBootsDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsDodgeMaxModifier("egs_craft", "LightBootsDodgeMaxModifier", "Variable LightBootsDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesDodgeMinModifier("egs_craft", "LightGlovesDodgeMinModifier", "Variable LightGlovesDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesDodgeMaxModifier("egs_craft", "LightGlovesDodgeMaxModifier", "Variable LightGlovesDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsDodgeMinModifier("egs_craft", "LightPantsDodgeMinModifier", "Variable LightPantsDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsDodgeMaxModifier("egs_craft", "LightPantsDodgeMaxModifier", "Variable LightPantsDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesDodgeMinModifier("egs_craft", "LightSleevesDodgeMinModifier", "Variable LightSleevesDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesDodgeMaxModifier("egs_craft", "LightSleevesDodgeMaxModifier", "Variable LightSleevesDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestDodgeMinModifier("egs_craft", "LightVestDodgeMinModifier", "Variable LightVestDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestDodgeMaxModifier("egs_craft", "LightVestDodgeMaxModifier", "Variable LightVestDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsDodgeMinModifier("egs_craft", "MediumBootsDodgeMinModifier", "Variable MediumBootsDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsDodgeMaxModifier("egs_craft", "MediumBootsDodgeMaxModifier", "Variable MediumBootsDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesDodgeMinModifier("egs_craft", "MediumGlovesDodgeMinModifier", "Variable MediumGlovesDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesDodgeMaxModifier("egs_craft", "MediumGlovesDodgeMaxModifier", "Variable MediumGlovesDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsDodgeMinModifier("egs_craft", "MediumPantsDodgeMinModifier", "Variable MediumPantsDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsDodgeMaxModifier("egs_craft", "MediumPantsDodgeMaxModifier", "Variable MediumPantsDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesDodgeMinModifier("egs_craft", "MediumSleevesDodgeMinModifier", "Variable MediumSleevesDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesDodgeMaxModifier("egs_craft", "MediumSleevesDodgeMaxModifier", "Variable MediumSleevesDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestDodgeMinModifier("egs_craft", "MediumVestDodgeMinModifier", "Variable MediumVestDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestDodgeMaxModifier("egs_craft", "MediumVestDodgeMaxModifier", "Variable MediumVestDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsDodgeMinModifier("egs_craft", "HeavyBootsDodgeMinModifier", "Variable HeavyBootsDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsDodgeMaxModifier("egs_craft", "HeavyBootsDodgeMaxModifier", "Variable HeavyBootsDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesDodgeMinModifier("egs_craft", "HeavyGlovesDodgeMinModifier", "Variable HeavyGlovesDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesDodgeMaxModifier("egs_craft", "HeavyGlovesDodgeMaxModifier", "Variable HeavyGlovesDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsDodgeMinModifier("egs_craft", "HeavyPantsDodgeMinModifier", "Variable HeavyPantsDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsDodgeMaxModifier("egs_craft", "HeavyPantsDodgeMaxModifier", "Variable HeavyPantsDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesDodgeMinModifier("egs_craft", "HeavySleevesDodgeMinModifier", "Variable HeavySleevesDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesDodgeMaxModifier("egs_craft", "HeavySleevesDodgeMaxModifier", "Variable HeavySleevesDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestDodgeMinModifier("egs_craft", "HeavyVestDodgeMinModifier", "Variable HeavyVestDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestDodgeMaxModifier("egs_craft", "HeavyVestDodgeMaxModifier", "Variable HeavyVestDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetDodgeMinModifier("egs_craft", "HeavyHelmetDodgeMinModifier", "Variable HeavyHelmetDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetDodgeMaxModifier("egs_craft", "HeavyHelmetDodgeMaxModifier", "Variable HeavyHelmetDodgeMaxModifier", 0.0f, 0, true);
	
	
	//ParryModifier	not for ammo and jewel, but for armor too
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerParryMinModifier("egs_craft", "DaggerParryMinModifier", "Variable DaggerParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerParryMaxModifier("egs_craft", "DaggerParryMaxModifier", "Variable DaggerParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordParryMinModifier("egs_craft", "SwordParryMinModifier", "Variable SwordParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordParryMaxModifier("egs_craft", "SwordParryMaxModifier", "Variable SwordParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceParryMinModifier("egs_craft", "MaceParryMinModifier", "Variable MaceParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceParryMaxModifier("egs_craft", "MaceParryMaxModifier", "Variable MaceParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeParryMinModifier("egs_craft", "AxeParryMinModifier", "Variable AxeParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeParryMaxModifier("egs_craft", "AxeParryMaxModifier", "Variable AxeParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearParryMinModifier("egs_craft", "SpearParryMinModifier", "Variable SpearParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearParryMaxModifier("egs_craft", "SpearParryMaxModifier", "Variable SpearParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffParryMinModifier("egs_craft", "StaffParryMinModifier", "Variable StaffParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffParryMaxModifier("egs_craft", "StaffParryMaxModifier", "Variable StaffParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffParryMinModifier("egs_craft", "MagicianStaffParryMinModifier", "Variable MagicianStaffParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffParryMaxModifier("egs_craft", "MagicianStaffParryMaxModifier", "Variable MagicianStaffParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordParryMinModifier("egs_craft", "TwoHandSwordParryMinModifier", "Variable TwoHandSwordParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordParryMaxModifier("egs_craft", "TwoHandSwordParryMaxModifier", "Variable TwoHandSwordParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeParryMinModifier("egs_craft", "TwoHandAxeParryMinModifier", "Variable TwoHandAxeParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeParryMaxModifier("egs_craft", "TwoHandAxeParryMaxModifier", "Variable TwoHandAxeParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeParryMinModifier("egs_craft", "PikeParryMinModifier", "Variable PikeParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeParryMaxModifier("egs_craft", "PikeParryMaxModifier", "Variable PikeParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceParryMinModifier("egs_craft", "TwoHandMaceParryMinModifier", "Variable TwoHandMaceParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceParryMaxModifier("egs_craft", "TwoHandMaceParryMaxModifier", "Variable TwoHandMaceParryMaxModifier", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchParryMinModifier("egs_craft", "AutolauchParryMinModifier", "Variable AutolauchParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchParryMaxModifier("egs_craft", "AutolauchParryMaxModifier", "Variable AutolauchParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleParryMinModifier("egs_craft", "BowrifleParryMinModifier", "Variable BowrifleParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleParryMaxModifier("egs_craft", "BowrifleParryMaxModifier", "Variable BowrifleParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherParryMinModifier("egs_craft", "LauncherParryMinModifier", "Variable LauncherParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherParryMaxModifier("egs_craft", "LauncherParryMaxModifier", "Variable LauncherParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolParryMinModifier("egs_craft", "PistolParryMinModifier", "Variable PistolParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolParryMaxModifier("egs_craft", "PistolParryMaxModifier", "Variable PistolParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolParryMinModifier("egs_craft", "BowpistolParryMinModifier", "Variable BowpistolParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolParryMaxModifier("egs_craft", "BowpistolParryMaxModifier", "Variable BowpistolParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleParryMinModifier("egs_craft", "RifleParryMinModifier", "Variable RifleParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleParryMaxModifier("egs_craft", "RifleParryMaxModifier", "Variable RifleParryMaxModifier", 0.0f, 0, true);
	
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldParryMinModifier("egs_craft", "ShieldParryMinModifier", "Variable ShieldParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldParryMaxModifier("egs_craft", "ShieldParryMaxModifier", "Variable ShieldParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerParryMinModifier("egs_craft", "BucklerParryMinModifier", "Variable BucklerParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerParryMaxModifier("egs_craft", "BucklerParryMaxModifier", "Variable BucklerParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsParryMinModifier("egs_craft", "LightBootsParryMinModifier", "Variable LightBootsParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsParryMaxModifier("egs_craft", "LightBootsParryMaxModifier", "Variable LightBootsParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesParryMinModifier("egs_craft", "LightGlovesParryMinModifier", "Variable LightGlovesParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesParryMaxModifier("egs_craft", "LightGlovesParryMaxModifier", "Variable LightGlovesParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsParryMinModifier("egs_craft", "LightPantsParryMinModifier", "Variable LightPantsParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsParryMaxModifier("egs_craft", "LightPantsParryMaxModifier", "Variable LightPantsParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesParryMinModifier("egs_craft", "LightSleevesParryMinModifier", "Variable LightSleevesParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesParryMaxModifier("egs_craft", "LightSleevesParryMaxModifier", "Variable LightSleevesParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestParryMinModifier("egs_craft", "LightVestParryMinModifier", "Variable LightVestParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestParryMaxModifier("egs_craft", "LightVestParryMaxModifier", "Variable LightVestParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsParryMinModifier("egs_craft", "MediumBootsParryMinModifier", "Variable MediumBootsParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsParryMaxModifier("egs_craft", "MediumBootsParryMaxModifier", "Variable MediumBootsParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesParryMinModifier("egs_craft", "MediumGlovesParryMinModifier", "Variable MediumGlovesParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesParryMaxModifier("egs_craft", "MediumGlovesParryMaxModifier", "Variable MediumGlovesParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsParryMinModifier("egs_craft", "MediumPantsParryMinModifier", "Variable MediumPantsParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsParryMaxModifier("egs_craft", "MediumPantsParryMaxModifier", "Variable MediumPantsParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesParryMinModifier("egs_craft", "MediumSleevesParryMinModifier", "Variable MediumSleevesParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesParryMaxModifier("egs_craft", "MediumSleevesParryMaxModifier", "Variable MediumSleevesParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestParryMinModifier("egs_craft", "MediumVestParryMinModifier", "Variable MediumVestParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestParryMaxModifier("egs_craft", "MediumVestParryMaxModifier", "Variable MediumVestParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsParryMinModifier("egs_craft", "HeavyBootsParryMinModifier", "Variable HeavyBootsParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsParryMaxModifier("egs_craft", "HeavyBootsParryMaxModifier", "Variable HeavyBootsParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesParryMinModifier("egs_craft", "HeavyGlovesParryMinModifier", "Variable HeavyGlovesParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesParryMaxModifier("egs_craft", "HeavyGlovesParryMaxModifier", "Variable HeavyGlovesParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsParryMinModifier("egs_craft", "HeavyPantsParryMinModifier", "Variable HeavyPantsParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsParryMaxModifier("egs_craft", "HeavyPantsParryMaxModifier", "Variable HeavyPantsParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesParryMinModifier("egs_craft", "HeavySleevesParryMinModifier", "Variable HeavySleevesParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesParryMaxModifier("egs_craft", "HeavySleevesParryMaxModifier", "Variable HeavySleevesParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestParryMinModifier("egs_craft", "HeavyVestParryMinModifier", "Variable HeavyVestParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestParryMaxModifier("egs_craft", "HeavyVestParryMaxModifier", "Variable HeavyVestParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetParryMinModifier("egs_craft", "HeavyHelmetParryMinModifier", "Variable HeavyHelmetParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetParryMaxModifier("egs_craft", "HeavyHelmetParryMaxModifier", "Variable HeavyHelmetParryMaxModifier", 0.0f, 0, true);
	
	
	//AdversaryDodgeModifier not for ammo, jewel and armor
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerAdversaryDodgeMinModifier("egs_craft", "DaggerAdversaryDodgeMinModifier", "Variable DaggerAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerAdversaryDodgeMaxModifier("egs_craft", "DaggerAdversaryDodgeMaxModifier", "Variable DaggerAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordAdversaryDodgeMinModifier("egs_craft", "SwordAdversaryDodgeMinModifier", "Variable SwordAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordAdversaryDodgeMaxModifier("egs_craft", "SwordAdversaryDodgeMaxModifier", "Variable SwordAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceAdversaryDodgeMinModifier("egs_craft", "MaceAdversaryDodgeMinModifier", "Variable MaceAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceAdversaryDodgeMaxModifier("egs_craft", "MaceAdversaryDodgeMaxModifier", "Variable MaceAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeAdversaryDodgeMinModifier("egs_craft", "AxeAdversaryDodgeMinModifier", "Variable AxeAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeAdversaryDodgeMaxModifier("egs_craft", "AxeAdversaryDodgeMaxModifier", "Variable AxeAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearAdversaryDodgeMinModifier("egs_craft", "SpearAdversaryDodgeMinModifier", "Variable SpearAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearAdversaryDodgeMaxModifier("egs_craft", "SpearAdversaryDodgeMaxModifier", "Variable SpearAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffAdversaryDodgeMinModifier("egs_craft", "StaffAdversaryDodgeMinModifier", "Variable StaffAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffAdversaryDodgeMaxModifier("egs_craft", "StaffAdversaryDodgeMaxModifier", "Variable StaffAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffAdversaryDodgeMinModifier("egs_craft", "MagicianStaffAdversaryDodgeMinModifier", "Variable MagicianStaffAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffAdversaryDodgeMaxModifier("egs_craft", "MagicianStaffAdversaryDodgeMaxModifier", "Variable MagicianStaffAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordAdversaryDodgeMinModifier("egs_craft", "TwoHandSwordAdversaryDodgeMinModifier", "Variable TwoHandSwordAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordAdversaryDodgeMaxModifier("egs_craft", "TwoHandSwordAdversaryDodgeMaxModifier", "Variable TwoHandSwordAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeAdversaryDodgeMinModifier("egs_craft", "TwoHandAxeAdversaryDodgeMinModifier", "Variable TwoHandAxeAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeAdversaryDodgeMaxModifier("egs_craft", "TwoHandAxeAdversaryDodgeMaxModifier", "Variable TwoHandAxeAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeAdversaryDodgeMinModifier("egs_craft", "PikeAdversaryDodgeMinModifier", "Variable PikeAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeAdversaryDodgeMaxModifier("egs_craft", "PikeAdversaryDodgeMaxModifier", "Variable PikeAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceAdversaryDodgeMinModifier("egs_craft", "TwoHandMaceAdversaryDodgeMinModifier", "Variable TwoHandMaceAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceAdversaryDodgeMaxModifier("egs_craft", "TwoHandMaceAdversaryDodgeMaxModifier", "Variable TwoHandMaceAdversaryDodgeMaxModifier", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchAdversaryDodgeMinModifier("egs_craft", "AutolauchAdversaryDodgeMinModifier", "Variable AutolauchAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchAdversaryDodgeMaxModifier("egs_craft", "AutolauchAdversaryDodgeMaxModifier", "Variable AutolauchAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAdversaryDodgeMinModifier("egs_craft", "BowrifleAdversaryDodgeMinModifier", "Variable BowrifleAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAdversaryDodgeMaxModifier("egs_craft", "BowrifleAdversaryDodgeMaxModifier", "Variable BowrifleAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAdversaryDodgeMinModifier("egs_craft", "LauncherAdversaryDodgeMinModifier", "Variable LauncherAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAdversaryDodgeMaxModifier("egs_craft", "LauncherAdversaryDodgeMaxModifier", "Variable LauncherAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAdversaryDodgeMinModifier("egs_craft", "PistolAdversaryDodgeMinModifier", "Variable PistolAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAdversaryDodgeMaxModifier("egs_craft", "PistolAdversaryDodgeMaxModifier", "Variable PistolAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAdversaryDodgeMinModifier("egs_craft", "BowpistolAdversaryDodgeMinModifier", "Variable BowpistolAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAdversaryDodgeMaxModifier("egs_craft", "BowpistolAdversaryDodgeMaxModifier", "Variable BowpistolAdversaryDodgeMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAdversaryDodgeMinModifier("egs_craft", "RifleAdversaryDodgeMinModifier", "Variable RifleAdversaryDodgeMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAdversaryDodgeMaxModifier("egs_craft", "RifleAdversaryDodgeMaxModifier", "Variable RifleAdversaryDodgeMaxModifier", 0.0f, 0, true);
	
	
	//AdversaryParryModifier not for ammo, jewel and armor
	// melee weapons
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerAdversaryParryMinModifier("egs_craft", "DaggerAdversaryParryMinModifier", "Variable DaggerAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerAdversaryParryMaxModifier("egs_craft", "DaggerAdversaryParryMaxModifier", "Variable DaggerAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordAdversaryParryMinModifier("egs_craft", "SwordAdversaryParryMinModifier", "Variable SwordAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordAdversaryParryMaxModifier("egs_craft", "SwordAdversaryParryMaxModifier", "Variable SwordAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceAdversaryParryMinModifier("egs_craft", "MaceAdversaryParryMinModifier", "Variable MaceAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceAdversaryParryMaxModifier("egs_craft", "MaceAdversaryParryMaxModifier", "Variable MaceAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeAdversaryParryMinModifier("egs_craft", "AxeAdversaryParryMinModifier", "Variable AxeAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeAdversaryParryMaxModifier("egs_craft", "AxeAdversaryParryMaxModifier", "Variable AxeAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearAdversaryParryMinModifier("egs_craft", "SpearAdversaryParryMinModifier", "Variable SpearAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearAdversaryParryMaxModifier("egs_craft", "SpearAdversaryParryMaxModifier", "Variable SpearAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffAdversaryParryMinModifier("egs_craft", "StaffAdversaryParryMinModifier", "Variable StaffAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffAdversaryParryMaxModifier("egs_craft", "StaffAdversaryParryMaxModifier", "Variable StaffAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffAdversaryParryMinModifier("egs_craft", "MagicianStaffAdversaryParryMinModifier", "Variable MagicianStaffAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffAdversaryParryMaxModifier("egs_craft", "MagicianStaffAdversaryParryMaxModifier", "Variable MagicianStaffAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordAdversaryParryMinModifier("egs_craft", "TwoHandSwordAdversaryParryMinModifier", "Variable TwoHandSwordAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordAdversaryParryMaxModifier("egs_craft", "TwoHandSwordAdversaryParryMaxModifier", "Variable TwoHandSwordAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeAdversaryParryMinModifier("egs_craft", "TwoHandAxeAdversaryParryMinModifier", "Variable TwoHandAxeAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeAdversaryParryMaxModifier("egs_craft", "TwoHandAxeAdversaryParryMaxModifier", "Variable TwoHandAxeAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeAdversaryParryMinModifier("egs_craft", "PikeAdversaryParryMinModifier", "Variable PikeAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeAdversaryParryMaxModifier("egs_craft", "PikeAdversaryParryMaxModifier", "Variable PikeAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceAdversaryParryMinModifier("egs_craft", "TwoHandMaceAdversaryParryMinModifier", "Variable TwoHandMaceAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceAdversaryParryMaxModifier("egs_craft", "TwoHandMaceAdversaryParryMaxModifier", "Variable TwoHandMaceAdversaryParryMaxModifier", 0.0f, 0, true);
	
	// range weapon
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchAdversaryParryMinModifier("egs_craft", "AutolauchAdversaryParryMinModifier", "Variable AutolauchAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AutolauchAdversaryParryMaxModifier("egs_craft", "AutolauchAdversaryParryMaxModifier", "Variable AutolauchAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAdversaryParryMinModifier("egs_craft", "BowrifleAdversaryParryMinModifier", "Variable BowrifleAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowrifleAdversaryParryMaxModifier("egs_craft", "BowrifleAdversaryParryMaxModifier", "Variable BowrifleAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAdversaryParryMinModifier("egs_craft", "LauncherAdversaryParryMinModifier", "Variable LauncherAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LauncherAdversaryParryMaxModifier("egs_craft", "LauncherAdversaryParryMaxModifier", "Variable LauncherAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAdversaryParryMinModifier("egs_craft", "PistolAdversaryParryMinModifier", "Variable PistolAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PistolAdversaryParryMaxModifier("egs_craft", "PistolAdversaryParryMaxModifier", "Variable PistolAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAdversaryParryMinModifier("egs_craft", "BowpistolAdversaryParryMinModifier", "Variable BowpistolAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BowpistolAdversaryParryMaxModifier("egs_craft", "BowpistolAdversaryParryMaxModifier", "Variable BowpistolAdversaryParryMaxModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAdversaryParryMinModifier("egs_craft", "RifleAdversaryParryMinModifier", "Variable RifleAdversaryParryMinModifier", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RifleAdversaryParryMaxModifier("egs_craft", "RifleAdversaryParryMaxModifier", "Variable RifleAdversaryParryMaxModifier", 0.0f, 0, true);


//Elemental casting time factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerElementalCastingTimeFactor("egs_craft","DaggerElementalCastingTimeFactor", "Variable.DaggerElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordElementalCastingTimeFactor("egs_craft","SwordElementalCastingTimeFactor", "Variable.SwordElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeElementalCastingTimeFactor("egs_craft","AxeElementalCastingTimeFactor", "Variable.AxeElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceElementalCastingTimeFactor("egs_craft","MaceElementalCastingTimeFactor", "Variable.MaceElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearElementalCastingTimeFactor("egs_craft","SpearElementalCastingTimeFactor", "Variable.SpearElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffElementalCastingTimeFactor("egs_craft","StaffElementalCastingTimeFactor", "Variable.StaffElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffElementalCastingTimeFactor("egs_craft","MagicianStaffElementalCastingTimeFactor", "Variable.MagicianStaffElementalCastingTimeFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeElementalCastingTimeFactor("egs_craft","TwoHandAxeElementalCastingTimeFactor", "Variable.TwoHandAxeElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordElementalCastingTimeFactor("egs_craft","TwoHandSwordElementalCastingTimeFactor", "Variable.TwoHandSwordElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeElementalCastingTimeFactor("egs_craft","PikeElementalCastingTimeFactor", "Variable.PikeElementalCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceElementalCastingTimeFactor("egs_craft","TwoHandMaceElementalCastingTimeFactor", "Variable.TwoHandMaceElementalCastingTimeFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerElementalCastingTimeFactorMax("egs_craft","DaggerElementalCastingTimeFactorMax", "Variable.DaggerElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordElementalCastingTimeFactorMax("egs_craft","SwordElementalCastingTimeFactorMax", "Variable.SwordElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeElementalCastingTimeFactorMax("egs_craft","AxeElementalCastingTimeFactorMax", "Variable.AxeElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceElementalCastingTimeFactorMax("egs_craft","MaceElementalCastingTimeFactorMax", "Variable.MaceElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearElementalCastingTimeFactorMax("egs_craft","SpearElementalCastingTimeFactorMax", "Variable.SpearElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffElementalCastingTimeFactorMax("egs_craft","StaffElementalCastingTimeFactorMax", "Variable.StaffElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffElementalCastingTimeFactorMax("egs_craft","MagicianStaffElementalCastingTimeFactorMax", "Variable.MagicianStaffElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeElementalCastingTimeFactorMax("egs_craft","TwoHandAxeElementalCastingTimeFactorMax", "Variable.TwoHandAxeElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordElementalCastingTimeFactorMax("egs_craft","TwoHandSwordElementalCastingTimeFactorMax", "Variable.TwoHandSwordElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeElementalCastingTimeFactorMax("egs_craft","PikeElementalCastingTimeFactorMax", "Variable.PikeElementalCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceElementalCastingTimeFactorMax("egs_craft","TwoHandMaceElementalCastingTimeFactorMax", "Variable.TwoHandMaceElementalCastingTimeFactorMax", 1.0f, 0, true);

//Elemental power factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerElementalPowerFactor("egs_craft","DaggerElementalPowerFactor", "Variable.DaggerElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordElementalPowerFactor("egs_craft","SwordElementalPowerFactor", "Variable.SwordElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeElementalPowerFactor("egs_craft","AxeElementalPowerFactor", "Variable.AxeElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceElementalPowerFactor("egs_craft","MaceElementalPowerFactor", "Variable.MaceElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearElementalPowerFactor("egs_craft","SpearElementalPowerFactor", "Variable.SpearElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffElementalPowerFactor("egs_craft","StaffElementalPowerFactor", "Variable.StaffElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffElementalPowerFactor("egs_craft","MagicianStaffElementalPowerFactor", "Variable.MagicianStaffElementalPowerFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeElementalPowerFactor("egs_craft","TwoHandAxeElementalPowerFactor", "Variable.TwoHandAxeElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordElementalPowerFactor("egs_craft","TwoHandSwordElementalPowerFactor", "Variable.TwoHandSwordElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeElementalPowerFactor("egs_craft","PikeElementalPowerFactor", "Variable.PikeElementalPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceElementalPowerFactor("egs_craft","TwoHandMaceElementalPowerFactor", "Variable.TwoHandMaceElementalPowerFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerElementalPowerFactorMax("egs_craft","DaggerElementalPowerFactorMax", "Variable.DaggerElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordElementalPowerFactorMax("egs_craft","SwordElementalPowerFactorMax", "Variable.SwordElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeElementalPowerFactorMax("egs_craft","AxeElementalPowerFactorMax", "Variable.AxeElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceElementalPowerFactorMax("egs_craft","MaceElementalPowerFactorMax", "Variable.MaceElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearElementalPowerFactorMax("egs_craft","SpearElementalPowerFactorMax", "Variable.SpearElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffElementalPowerFactorMax("egs_craft","StaffElementalPowerFactorMax", "Variable.StaffElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffElementalPowerFactorMax("egs_craft","MagicianStaffElementalPowerFactorMax", "Variable.MagicianStaffElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeElementalPowerFactorMax("egs_craft","TwoHandAxeElementalPowerFactorMax", "Variable.TwoHandAxeElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordElementalPowerFactorMax("egs_craft","TwoHandSwordElementalPowerFactorMax", "Variable.TwoHandSwordElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeElementalPowerFactorMax("egs_craft","PikeElementalPowerFactorMax", "Variable.PikeElementalPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceElementalPowerFactorMax("egs_craft","TwoHandMaceElementalPowerFactorMax", "Variable.TwoHandMaceElementalPowerFactorMax", 1.0f, 0, true);

//OffensiveAffliction casting time factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerOffensiveAfflictionCastingTimeFactor("egs_craft","DaggerOffensiveAfflictionCastingTimeFactor", "Variable.DaggerOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordOffensiveAfflictionCastingTimeFactor("egs_craft","SwordOffensiveAfflictionCastingTimeFactor", "Variable.SwordOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeOffensiveAfflictionCastingTimeFactor("egs_craft","AxeOffensiveAfflictionCastingTimeFactor", "Variable.AxeOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceOffensiveAfflictionCastingTimeFactor("egs_craft","MaceOffensiveAfflictionCastingTimeFactor", "Variable.MaceOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearOffensiveAfflictionCastingTimeFactor("egs_craft","SpearOffensiveAfflictionCastingTimeFactor", "Variable.SpearOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffOffensiveAfflictionCastingTimeFactor("egs_craft","StaffOffensiveAfflictionCastingTimeFactor", "Variable.StaffOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffOffensiveAfflictionCastingTimeFactor("egs_craft","MagicianStaffOffensiveAfflictionCastingTimeFactor", "Variable.MagicianStaffOffensiveAfflictionCastingTimeFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionCastingTimeFactor("egs_craft","TwoHandAxeOffensiveAfflictionCastingTimeFactor", "Variable.TwoHandAxeOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionCastingTimeFactor("egs_craft","TwoHandSwordOffensiveAfflictionCastingTimeFactor", "Variable.TwoHandSwordOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeOffensiveAfflictionCastingTimeFactor("egs_craft","PikeOffensiveAfflictionCastingTimeFactor", "Variable.PikeOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionCastingTimeFactor("egs_craft","TwoHandMaceOffensiveAfflictionCastingTimeFactor", "Variable.TwoHandMaceOffensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerOffensiveAfflictionCastingTimeFactorMax("egs_craft","DaggerOffensiveAfflictionCastingTimeFactorMax", "Variable.DaggerOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordOffensiveAfflictionCastingTimeFactorMax("egs_craft","SwordOffensiveAfflictionCastingTimeFactorMax", "Variable.SwordOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeOffensiveAfflictionCastingTimeFactorMax("egs_craft","AxeOffensiveAfflictionCastingTimeFactorMax", "Variable.AxeOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceOffensiveAfflictionCastingTimeFactorMax("egs_craft","MaceOffensiveAfflictionCastingTimeFactorMax", "Variable.MaceOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearOffensiveAfflictionCastingTimeFactorMax("egs_craft","SpearOffensiveAfflictionCastingTimeFactorMax", "Variable.SpearOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffOffensiveAfflictionCastingTimeFactorMax("egs_craft","StaffOffensiveAfflictionCastingTimeFactorMax", "Variable.StaffOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffOffensiveAfflictionCastingTimeFactorMax("egs_craft","MagicianStaffOffensiveAfflictionCastingTimeFactorMax", "Variable.MagicianStaffOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionCastingTimeFactorMax("egs_craft","TwoHandAxeOffensiveAfflictionCastingTimeFactorMax", "Variable.TwoHandAxeOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionCastingTimeFactorMax("egs_craft","TwoHandSwordOffensiveAfflictionCastingTimeFactorMax", "Variable.TwoHandSwordOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeOffensiveAfflictionCastingTimeFactorMax("egs_craft","PikeOffensiveAfflictionCastingTimeFactorMax", "Variable.PikeOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionCastingTimeFactorMax("egs_craft","TwoHandMaceOffensiveAfflictionCastingTimeFactorMax", "Variable.TwoHandMaceOffensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);

//OffensiveAffliction power factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerOffensiveAfflictionPowerFactor("egs_craft","DaggerOffensiveAfflictionPowerFactor", "Variable.DaggerOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordOffensiveAfflictionPowerFactor("egs_craft","SwordOffensiveAfflictionPowerFactor", "Variable.SwordOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeOffensiveAfflictionPowerFactor("egs_craft","AxeOffensiveAfflictionPowerFactor", "Variable.AxeOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceOffensiveAfflictionPowerFactor("egs_craft","MaceOffensiveAfflictionPowerFactor", "Variable.MaceOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearOffensiveAfflictionPowerFactor("egs_craft","SpearOffensiveAfflictionPowerFactor", "Variable.SpearOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffOffensiveAfflictionPowerFactor("egs_craft","StaffOffensiveAfflictionPowerFactor", "Variable.StaffOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffOffensiveAfflictionPowerFactor("egs_craft","MagicianStaffOffensiveAfflictionPowerFactor", "Variable.MagicianStaffOffensiveAfflictionPowerFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionPowerFactor("egs_craft","TwoHandAxeOffensiveAfflictionPowerFactor", "Variable.TwoHandAxeOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionPowerFactor("egs_craft","TwoHandSwordOffensiveAfflictionPowerFactor", "Variable.TwoHandSwordOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeOffensiveAfflictionPowerFactor("egs_craft","PikeOffensiveAfflictionPowerFactor", "Variable.PikeOffensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionPowerFactor("egs_craft","TwoHandMaceOffensiveAfflictionPowerFactor", "Variable.TwoHandMaceOffensiveAfflictionPowerFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerOffensiveAfflictionPowerFactorMax("egs_craft","DaggerOffensiveAfflictionPowerFactorMax", "Variable.DaggerOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordOffensiveAfflictionPowerFactorMax("egs_craft","SwordOffensiveAfflictionPowerFactorMax", "Variable.SwordOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeOffensiveAfflictionPowerFactorMax("egs_craft","AxeOffensiveAfflictionPowerFactorMax", "Variable.AxeOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceOffensiveAfflictionPowerFactorMax("egs_craft","MaceOffensiveAfflictionPowerFactorMax", "Variable.MaceOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearOffensiveAfflictionPowerFactorMax("egs_craft","SpearOffensiveAfflictionPowerFactorMax", "Variable.SpearOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffOffensiveAfflictionPowerFactorMax("egs_craft","StaffOffensiveAfflictionPowerFactorMax", "Variable.StaffOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffOffensiveAfflictionPowerFactorMax("egs_craft","MagicianStaffOffensiveAfflictionPowerFactorMax", "Variable.MagicianStaffOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionPowerFactorMax("egs_craft","TwoHandAxeOffensiveAfflictionPowerFactorMax", "Variable.TwoHandAxeOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionPowerFactorMax("egs_craft","TwoHandSwordOffensiveAfflictionPowerFactorMax", "Variable.TwoHandSwordOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeOffensiveAfflictionPowerFactorMax("egs_craft","PikeOffensiveAfflictionPowerFactorMax", "Variable.PikeOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionPowerFactorMax("egs_craft","TwoHandMaceOffensiveAfflictionPowerFactorMax", "Variable.TwoHandMaceOffensiveAfflictionPowerFactorMax", 1.0f, 0, true);

//Heal casting time factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerHealCastingTimeFactor("egs_craft","DaggerHealCastingTimeFactor", "Variable.DaggerHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordHealCastingTimeFactor("egs_craft","SwordHealCastingTimeFactor", "Variable.SwordHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeHealCastingTimeFactor("egs_craft","AxeHealCastingTimeFactor", "Variable.AxeHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceHealCastingTimeFactor("egs_craft","MaceHealCastingTimeFactor", "Variable.MaceHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearHealCastingTimeFactor("egs_craft","SpearHealCastingTimeFactor", "Variable.SpearHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffHealCastingTimeFactor("egs_craft","StaffHealCastingTimeFactor", "Variable.StaffHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffHealCastingTimeFactor("egs_craft","MagicianStaffHealCastingTimeFactor", "Variable.MagicianStaffHealCastingTimeFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeHealCastingTimeFactor("egs_craft","TwoHandAxeHealCastingTimeFactor", "Variable.TwoHandAxeHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordHealCastingTimeFactor("egs_craft","TwoHandSwordHealCastingTimeFactor", "Variable.TwoHandSwordHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeHealCastingTimeFactor("egs_craft","PikeHealCastingTimeFactor", "Variable.PikeHealCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceHealCastingTimeFactor("egs_craft","TwoHandMaceHealCastingTimeFactor", "Variable.TwoHandMaceHealCastingTimeFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerHealCastingTimeFactorMax("egs_craft","DaggerHealCastingTimeFactorMax", "Variable.DaggerHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordHealCastingTimeFactorMax("egs_craft","SwordHealCastingTimeFactorMax", "Variable.SwordHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeHealCastingTimeFactorMax("egs_craft","AxeHealCastingTimeFactorMax", "Variable.AxeHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceHealCastingTimeFactorMax("egs_craft","MaceHealCastingTimeFactorMax", "Variable.MaceHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearHealCastingTimeFactorMax("egs_craft","SpearHealCastingTimeFactorMax", "Variable.SpearHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffHealCastingTimeFactorMax("egs_craft","StaffHealCastingTimeFactorMax", "Variable.StaffHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffHealCastingTimeFactorMax("egs_craft","MagicianStaffHealCastingTimeFactorMax", "Variable.MagicianStaffHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeHealCastingTimeFactorMax("egs_craft","TwoHandAxeHealCastingTimeFactorMax", "Variable.TwoHandAxeHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordHealCastingTimeFactorMax("egs_craft","TwoHandSwordHealCastingTimeFactorMax", "Variable.TwoHandSwordHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeHealCastingTimeFactorMax("egs_craft","PikeHealCastingTimeFactorMax", "Variable.PikeHealCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceHealCastingTimeFactorMax("egs_craft","TwoHandMaceHealCastingTimeFactorMax", "Variable.TwoHandMaceHealCastingTimeFactorMax", 1.0f, 0, true);

//Heal power factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerHealPowerFactor("egs_craft","DaggerHealPowerFactor", "Variable.DaggerHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordHealPowerFactor("egs_craft","SwordHealPowerFactor", "Variable.SwordHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeHealPowerFactor("egs_craft","AxeHealPowerFactor", "Variable.AxeHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceHealPowerFactor("egs_craft","MaceHealPowerFactor", "Variable.MaceHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearHealPowerFactor("egs_craft","SpearHealPowerFactor", "Variable.SpearHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffHealPowerFactor("egs_craft","StaffHealPowerFactor", "Variable.StaffHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffHealPowerFactor("egs_craft","MagicianStaffHealPowerFactor", "Variable.MagicianStaffHealPowerFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeHealPowerFactor("egs_craft","TwoHandAxeHealPowerFactor", "Variable.TwoHandAxeHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordHealPowerFactor("egs_craft","TwoHandSwordHealPowerFactor", "Variable.TwoHandSwordHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeHealPowerFactor("egs_craft","PikeHealPowerFactor", "Variable.PikeHealPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceHealPowerFactor("egs_craft","TwoHandMaceHealPowerFactor", "Variable.TwoHandMaceHealPowerFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerHealPowerFactorMax("egs_craft","DaggerHealPowerFactorMax", "Variable.DaggerHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordHealPowerFactorMax("egs_craft","SwordHealPowerFactorMax", "Variable.SwordHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeHealPowerFactorMax("egs_craft","AxeHealPowerFactorMax", "Variable.AxeHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceHealPowerFactorMax("egs_craft","MaceHealPowerFactorMax", "Variable.MaceHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearHealPowerFactorMax("egs_craft","SpearHealPowerFactorMax", "Variable.SpearHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffHealPowerFactorMax("egs_craft","StaffHealPowerFactorMax", "Variable.StaffHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffHealPowerFactorMax("egs_craft","MagicianStaffHealPowerFactorMax", "Variable.MagicianStaffHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeHealPowerFactorMax("egs_craft","TwoHandAxeHealPowerFactorMax", "Variable.TwoHandAxeHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordHealPowerFactorMax("egs_craft","TwoHandSwordHealPowerFactorMax", "Variable.TwoHandSwordHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeHealPowerFactorMax("egs_craft","PikeHealPowerFactorMax", "Variable.PikeHealPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceHealPowerFactorMax("egs_craft","TwoHandMaceHealPowerFactorMax", "Variable.TwoHandMaceHealPowerFactorMax", 1.0f, 0, true);

//DefensiveAffliction casting time factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDefensiveAfflictionCastingTimeFactor("egs_craft","DaggerDefensiveAfflictionCastingTimeFactor", "Variable.DaggerDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDefensiveAfflictionCastingTimeFactor("egs_craft","SwordDefensiveAfflictionCastingTimeFactor", "Variable.SwordDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDefensiveAfflictionCastingTimeFactor("egs_craft","AxeDefensiveAfflictionCastingTimeFactor", "Variable.AxeDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDefensiveAfflictionCastingTimeFactor("egs_craft","MaceDefensiveAfflictionCastingTimeFactor", "Variable.MaceDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDefensiveAfflictionCastingTimeFactor("egs_craft","SpearDefensiveAfflictionCastingTimeFactor", "Variable.SpearDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDefensiveAfflictionCastingTimeFactor("egs_craft","StaffDefensiveAfflictionCastingTimeFactor", "Variable.StaffDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDefensiveAfflictionCastingTimeFactor("egs_craft","MagicianStaffDefensiveAfflictionCastingTimeFactor", "Variable.MagicianStaffDefensiveAfflictionCastingTimeFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionCastingTimeFactor("egs_craft","TwoHandAxeDefensiveAfflictionCastingTimeFactor", "Variable.TwoHandAxeDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionCastingTimeFactor("egs_craft","TwoHandSwordDefensiveAfflictionCastingTimeFactor", "Variable.TwoHandSwordDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDefensiveAfflictionCastingTimeFactor("egs_craft","PikeDefensiveAfflictionCastingTimeFactor", "Variable.PikeDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionCastingTimeFactor("egs_craft","TwoHandMaceDefensiveAfflictionCastingTimeFactor", "Variable.TwoHandMaceDefensiveAfflictionCastingTimeFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDefensiveAfflictionCastingTimeFactorMax("egs_craft","DaggerDefensiveAfflictionCastingTimeFactorMax", "Variable.DaggerDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDefensiveAfflictionCastingTimeFactorMax("egs_craft","SwordDefensiveAfflictionCastingTimeFactorMax", "Variable.SwordDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDefensiveAfflictionCastingTimeFactorMax("egs_craft","AxeDefensiveAfflictionCastingTimeFactorMax", "Variable.AxeDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDefensiveAfflictionCastingTimeFactorMax("egs_craft","MaceDefensiveAfflictionCastingTimeFactorMax", "Variable.MaceDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDefensiveAfflictionCastingTimeFactorMax("egs_craft","SpearDefensiveAfflictionCastingTimeFactorMax", "Variable.SpearDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDefensiveAfflictionCastingTimeFactorMax("egs_craft","StaffDefensiveAfflictionCastingTimeFactorMax", "Variable.StaffDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDefensiveAfflictionCastingTimeFactorMax("egs_craft","MagicianStaffDefensiveAfflictionCastingTimeFactorMax", "Variable.MagicianStaffDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionCastingTimeFactorMax("egs_craft","TwoHandAxeDefensiveAfflictionCastingTimeFactorMax", "Variable.TwoHandAxeDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionCastingTimeFactorMax("egs_craft","TwoHandSwordDefensiveAfflictionCastingTimeFactorMax", "Variable.TwoHandSwordDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDefensiveAfflictionCastingTimeFactorMax("egs_craft","PikeDefensiveAfflictionCastingTimeFactorMax", "Variable.PikeDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionCastingTimeFactorMax("egs_craft","TwoHandMaceDefensiveAfflictionCastingTimeFactorMax", "Variable.TwoHandMaceDefensiveAfflictionCastingTimeFactorMax", 1.0f, 0, true);

//DefensiveAffliction power factor (melee weapon only)
// Min
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDefensiveAfflictionPowerFactor("egs_craft","DaggerDefensiveAfflictionPowerFactor", "Variable.DaggerDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDefensiveAfflictionPowerFactor("egs_craft","SwordDefensiveAfflictionPowerFactor", "Variable.SwordDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDefensiveAfflictionPowerFactor("egs_craft","AxeDefensiveAfflictionPowerFactor", "Variable.AxeDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDefensiveAfflictionPowerFactor("egs_craft","MaceDefensiveAfflictionPowerFactor", "Variable.MaceDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDefensiveAfflictionPowerFactor("egs_craft","SpearDefensiveAfflictionPowerFactor", "Variable.SpearDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDefensiveAfflictionPowerFactor("egs_craft","StaffDefensiveAfflictionPowerFactor", "Variable.StaffDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDefensiveAfflictionPowerFactor("egs_craft","MagicianStaffDefensiveAfflictionPowerFactor", "Variable.MagicianStaffDefensiveAfflictionPowerFactor", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionPowerFactor("egs_craft","TwoHandAxeDefensiveAfflictionPowerFactor", "Variable.TwoHandAxeDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionPowerFactor("egs_craft","TwoHandSwordDefensiveAfflictionPowerFactor", "Variable.TwoHandSwordDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDefensiveAfflictionPowerFactor("egs_craft","PikeDefensiveAfflictionPowerFactor", "Variable.PikeDefensiveAfflictionPowerFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionPowerFactor("egs_craft","TwoHandMaceDefensiveAfflictionPowerFactor", "Variable.TwoHandMaceDefensiveAfflictionPowerFactor", 0.0f, 0, true);
// Max
NLMISC::CVariable<float> CWeaponCraftParameters::DaggerDefensiveAfflictionPowerFactorMax("egs_craft","DaggerDefensiveAfflictionPowerFactorMax", "Variable.DaggerDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SwordDefensiveAfflictionPowerFactorMax("egs_craft","SwordDefensiveAfflictionPowerFactorMax", "Variable.SwordDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::AxeDefensiveAfflictionPowerFactorMax("egs_craft","AxeDefensiveAfflictionPowerFactorMax", "Variable.AxeDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MaceDefensiveAfflictionPowerFactorMax("egs_craft","MaceDefensiveAfflictionPowerFactorMax", "Variable.MaceDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::SpearDefensiveAfflictionPowerFactorMax("egs_craft","SpearDefensiveAfflictionPowerFactorMax", "Variable.SpearDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::StaffDefensiveAfflictionPowerFactorMax("egs_craft","StaffDefensiveAfflictionPowerFactorMax", "Variable.StaffDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MagicianStaffDefensiveAfflictionPowerFactorMax("egs_craft","MagicianStaffDefensiveAfflictionPowerFactorMax", "Variable.MagicianStaffDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionPowerFactorMax("egs_craft","TwoHandAxeDefensiveAfflictionPowerFactorMax", "Variable.TwoHandAxeDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionPowerFactorMax("egs_craft","TwoHandSwordDefensiveAfflictionPowerFactorMax", "Variable.TwoHandSwordDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PikeDefensiveAfflictionPowerFactorMax("egs_craft","PikeDefensiveAfflictionPowerFactorMax", "Variable.PikeDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionPowerFactorMax("egs_craft","TwoHandMaceDefensiveAfflictionPowerFactorMax", "Variable.TwoHandMaceDefensiveAfflictionPowerFactorMax", 1.0f, 0, true);

	//////////////////////////////////////////////////////////////////////////
	// Armor factors
	//////////////////////////////////////////////////////////////////////////
	// ProtectionFactor
	// armor and shield
//Min
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldProtectionFactor("egs_craft", "ShieldProtectionFactor", "Variable ShieldProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerProtectionFactor("egs_craft", "BucklerProtectionFactor", "Variable BucklerProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsProtectionFactor("egs_craft", "LightBootsProtectionFactor", "Variable LightBootsProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesProtectionFactor("egs_craft", "LightGlovesProtectionFactor", "Variable LightGlovesProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsProtectionFactor("egs_craft", "LightPantsProtectionFactor", "Variable LightPantsProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesProtectionFactor("egs_craft", "LightSleevesProtectionFactor", "Variable LightSleevesProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestProtectionFactor("egs_craft", "LightVestProtectionFactor", "Variable LightVestProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsProtectionFactor("egs_craft", "MediumBootsProtectionFactor", "Variable MediumBootsProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesProtectionFactor("egs_craft", "MediumGlovesProtectionFactor", "Variable MediumGlovesProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsProtectionFactor("egs_craft", "MediumPantsProtectionFactor", "Variable MediumPantsProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesProtectionFactor("egs_craft", "MediumSleevesProtectionFactor", "Variable MediumSleevesProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestProtectionFactor("egs_craft", "MediumVestProtectionFactor", "Variable MediumVestProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsProtectionFactor("egs_craft", "HeavyBootsProtectionFactor", "Variable HeavyBootsProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesProtectionFactor("egs_craft", "HeavyGlovesProtectionFactor", "Variable HeavyGlovesProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsProtectionFactor("egs_craft", "HeavyPantsProtectionFactor", "Variable HeavyPantsProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesProtectionFactor("egs_craft", "HeavySleevesProtectionFactor", "Variable HeavySleevesProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestProtectionFactor("egs_craft", "HeavyVestProtectionFactor", "Variable HeavyVestProtectionFactor", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetProtectionFactor("egs_craft", "HeavyHelmetProtectionFactor", "Variable HeavyHelmetProtectionFactor", 0.0f, 0, true);
//Max
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldProtectionFactorMax("egs_craft", "ShieldProtectionFactorMax", "Variable ShieldProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerProtectionFactorMax("egs_craft", "BucklerProtectionFactorMax", "Variable BucklerProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsProtectionFactorMax("egs_craft", "LightBootsProtectionFactorMax", "Variable LightBootsProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesProtectionFactorMax("egs_craft", "LightGlovesProtectionFactorMax", "Variable LightGlovesProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsProtectionFactorMax("egs_craft", "LightPantsProtectionFactorMax", "Variable LightPantsProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesProtectionFactorMax("egs_craft", "LightSleevesProtectionFactorMax", "Variable LightSleevesProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestProtectionFactorMax("egs_craft", "LightVestProtectionFactorMax", "Variable LightVestProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsProtectionFactorMax("egs_craft", "MediumBootsProtectionFactorMax", "Variable MediumBootsProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesProtectionFactorMax("egs_craft", "MediumGlovesProtectionFactorMax", "Variable MediumGlovesProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsProtectionFactorMax("egs_craft", "MediumPantsProtectionFactorMax", "Variable MediumPantsProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesProtectionFactorMax("egs_craft", "MediumSleevesProtectionFactorMax", "Variable MediumSleevesProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestProtectionFactorMax("egs_craft", "MediumVestProtectionFactorMax", "Variable MediumVestProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsProtectionFactorMax("egs_craft", "HeavyBootsProtectionFactorMax", "Variable HeavyBootsProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesProtectionFactorMax("egs_craft", "HeavyGlovesProtectionFactorMax", "Variable HeavyGlovesProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsProtectionFactorMax("egs_craft", "HeavyPantsProtectionFactorMax", "Variable HeavyPantsProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesProtectionFactorMax("egs_craft", "HeavySleevesProtectionFactorMax", "Variable HeavySleevesProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestProtectionFactorMax("egs_craft", "HeavyVestProtectionFactorMax", "Variable HeavyVestProtectionFactor Max", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetProtectionFactorMax("egs_craft", "HeavyHelmetProtectionFactorMax", "Variable HeavyHelmetProtectionFactor Max", 0.0f, 0, true);

	// MaxSlashingProtection
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldMaxSlashingProtection("egs_craft", "ShieldMaxSlashingProtection", "Variable ShieldMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerMaxSlashingProtection("egs_craft", "BucklerMaxSlashingProtection", "Variable BucklerMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsMaxSlashingProtection("egs_craft", "LightBootsMaxSlashingProtection", "Variable LightBootsMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesMaxSlashingProtection("egs_craft", "LightGlovesMaxSlashingProtection", "Variable LightGlovesMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsMaxSlashingProtection("egs_craft", "LightPantsMaxSlashingProtection", "Variable LightPantsMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesMaxSlashingProtection("egs_craft", "LightSleevesMaxSlashingProtection", "Variable LightSleevesMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestMaxSlashingProtection("egs_craft", "LightVestMaxSlashingProtection", "Variable LightVestMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsMaxSlashingProtection("egs_craft", "MediumBootsMaxSlashingProtection", "Variable MediumBootsMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesMaxSlashingProtection("egs_craft", "MediumGlovesMaxSlashingProtection", "Variable MediumGlovesMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsMaxSlashingProtection("egs_craft", "MediumPantsMaxSlashingProtection", "Variable MediumPantsMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesMaxSlashingProtection("egs_craft", "MediumSleevesMaxSlashingProtection", "Variable MediumSleevesMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestMaxSlashingProtection("egs_craft", "MediumVestMaxSlashingProtection", "Variable MediumVestMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsMaxSlashingProtection("egs_craft", "HeavyBootsMaxSlashingProtection", "Variable HeavyBootsMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesMaxSlashingProtection("egs_craft", "HeavyGlovesMaxSlashingProtection", "Variable HeavyGlovesMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsMaxSlashingProtection("egs_craft", "HeavyPantsMaxSlashingProtection", "Variable HeavyPantsMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesMaxSlashingProtection("egs_craft", "HeavySleevesMaxSlashingProtection", "Variable HeavySleevesMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestMaxSlashingProtection("egs_craft", "HeavyVestMaxSlashingProtection", "Variable HeavyVestMaxSlashingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetMaxSlashingProtection("egs_craft", "HeavyHelmetMaxSlashingProtection", "Variable HeavyHelmetMaxSlashingProtection", 0.0f, 0, true);
	
	// MaxBluntProtection
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldMaxBluntProtection("egs_craft", "ShieldMaxBluntProtection", "Variable ShieldMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerMaxBluntProtection("egs_craft", "BucklerMaxBluntProtection", "Variable BucklerMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsMaxBluntProtection("egs_craft", "LightBootsMaxBluntProtection", "Variable LightBootsMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesMaxBluntProtection("egs_craft", "LightGlovesMaxBluntProtection", "Variable LightGlovesMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsMaxBluntProtection("egs_craft", "LightPantsMaxBluntProtection", "Variable LightPantsMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesMaxBluntProtection("egs_craft", "LightSleevesMaxBluntProtection", "Variable LightSleevesMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestMaxBluntProtection("egs_craft", "LightVestMaxBluntProtection", "Variable LightVestMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsMaxBluntProtection("egs_craft", "MediumBootsMaxBluntProtection", "Variable MediumBootsMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesMaxBluntProtection("egs_craft", "MediumGlovesMaxBluntProtection", "Variable MediumGlovesMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsMaxBluntProtection("egs_craft", "MediumPantsMaxBluntProtection", "Variable MediumPantsMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesMaxBluntProtection("egs_craft", "MediumSleevesMaxBluntProtection", "Variable MediumSleevesMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestMaxBluntProtection("egs_craft", "MediumVestMaxBluntProtection", "Variable MediumVestMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsMaxBluntProtection("egs_craft", "HeavyBootsMaxBluntProtection", "Variable HeavyBootsMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesMaxBluntProtection("egs_craft", "HeavyGlovesMaxBluntProtection", "Variable HeavyGlovesMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsMaxBluntProtection("egs_craft", "HeavyPantsMaxBluntProtection", "Variable HeavyPantsMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesMaxBluntProtection("egs_craft", "HeavySleevesMaxBluntProtection", "Variable HeavySleevesMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestMaxBluntProtection("egs_craft", "HeavyVestMaxBluntProtection", "Variable HeavyVestMaxBluntProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetMaxBluntProtection("egs_craft", "HeavyHelmetMaxBluntProtection", "Variable HeavyHelmetMaxBluntProtection", 0.0f, 0, true);
	
	// MaxPiercingProtection
	// armor and shield
NLMISC::CVariable<float> CWeaponCraftParameters::ShieldMaxPiercingProtection("egs_craft", "ShieldMaxPiercingProtection", "Variable ShieldMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::BucklerMaxPiercingProtection("egs_craft", "BucklerMaxPiercingProtection", "Variable BucklerMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightBootsMaxPiercingProtection("egs_craft", "LightBootsMaxPiercingProtection", "Variable LightBootsMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightGlovesMaxPiercingProtection("egs_craft", "LightGlovesMaxPiercingProtection", "Variable LightGlovesMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightPantsMaxPiercingProtection("egs_craft", "LightPantsMaxPiercingProtection", "Variable LightPantsMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightSleevesMaxPiercingProtection("egs_craft", "LightSleevesMaxPiercingProtection", "Variable LightSleevesMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LightVestMaxPiercingProtection("egs_craft", "LightVestMaxPiercingProtection", "Variable LightVestMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumBootsMaxPiercingProtection("egs_craft", "MediumBootsMaxPiercingProtection", "Variable MediumBootsMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumGlovesMaxPiercingProtection("egs_craft", "MediumGlovesMaxPiercingProtection", "Variable MediumGlovesMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumPantsMaxPiercingProtection("egs_craft", "MediumPantsMaxPiercingProtection", "Variable MediumPantsMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumSleevesMaxPiercingProtection("egs_craft", "MediumSleevesMaxPiercingProtection", "Variable MediumSleevesMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::MediumVestMaxPiercingProtection("egs_craft", "MediumVestMaxPiercingProtection", "Variable MediumVestMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyBootsMaxPiercingProtection("egs_craft", "HeavyBootsMaxPiercingProtection", "Variable HeavyBootsMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyGlovesMaxPiercingProtection("egs_craft", "HeavyGlovesMaxPiercingProtection", "Variable HeavyGlovesMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyPantsMaxPiercingProtection("egs_craft", "HeavyPantsMaxPiercingProtection", "Variable HeavyPantsMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavySleevesMaxPiercingProtection("egs_craft", "HeavySleevesMaxPiercingProtection", "Variable HeavySleevesMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyVestMaxPiercingProtection("egs_craft", "HeavyVestMaxPiercingProtection", "Variable HeavyVestMaxPiercingProtection", 0.0f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::HeavyHelmetMaxPiercingProtection("egs_craft", "HeavyHelmetMaxPiercingProtection", "Variable HeavyHelmetMaxPiercingProtection", 0.0f, 0, true);

// Jewel Magic Protection
NLMISC::CVariable<float> CWeaponCraftParameters::AcidJewelProtection("egs_craft", "AcidJewelProtection", "Variable AcidJewelProtection", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ColdJewelProtection("egs_craft", "ColdJewelProtection", "Variable ColdJewelProtection", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::FireJewelProtection("egs_craft", "FireJewelProtection", "Variable FireJewelProtection", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::RotJewelProtection("egs_craft", "RotJewelProtection", "Variable RotJewelProtection", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ShockWaveJewelProtection("egs_craft", "ShockWaveJewelProtection", "Variable ShockWaveJewelProtection", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PoisonJewelProtection("egs_craft", "PoisonJewelProtection", "Variable PoisonJewelProtection", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ElectricityJewelProtection("egs_craft", "ElectricityJewelProtection", "Variable ElectricityJewelProtection", 0.1f, 0, true);

// Jewel Magic Resistance
NLMISC::CVariable<float> CWeaponCraftParameters::DesertResistance("egs_craft", "DesertResistance", "Variable DesertResistance", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::ForestResistance("egs_craft", "ForestResistance", "Variable ForestResistance", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::LacustreResistance("egs_craft", "LacustreResistance", "Variable LacustreResistance", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::JungleResistance("egs_craft", "JungleResistance", "Variable JungleResistance", 0.1f, 0, true);
NLMISC::CVariable<float> CWeaponCraftParameters::PrimaryRootResistance("egs_craft", "PrimaryRootResistance", "Variable PrimaryRootResistance", 0.1f, 0, true);
