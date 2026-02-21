// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// EGS-specific implementations of egs_sheets methods.
// These depend on EGS globals and are kept separate from the
// egs_sheets library so the library can be used independently
// (e.g. by the sheets_packer_shard tool).

#include "stdpch.h"

#include "egs_sheets/egs_static_game_item.h"
#include "egs_sheets/egs_static_game_sheet.h"
#include "egs_sheets/egs_static_harvestable.h"
#include "egs_sheets/egs_sheets.h"
#include "egs_variables.h"
#include "egs_globals.h"
#include "game_item_manager/weapon_craft_parameters.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
// CStaticItem::getBaseWeight
// ***************************************************************************
float CStaticItem::getBaseWeight() const
{
	switch( Type )
	{
		// melee weapons
	case ITEM_TYPE::DAGGER:
		return CWeaponCraftParameters::DaggerWeight;
	case ITEM_TYPE::SWORD:
		return CWeaponCraftParameters::SwordWeight;
	case ITEM_TYPE::MACE:
		return CWeaponCraftParameters::MaceWeight;
	case ITEM_TYPE::AXE:
		return CWeaponCraftParameters::AxeWeight;
	case ITEM_TYPE::SPEAR:
		return CWeaponCraftParameters::SpearWeight;
	case ITEM_TYPE::STAFF:
		return CWeaponCraftParameters::StaffWeight;
	case ITEM_TYPE::MAGICIAN_STAFF:
		return CWeaponCraftParameters::MagicianStaffWeight;
	case ITEM_TYPE::TWO_HAND_SWORD:
		return CWeaponCraftParameters::TwoHandSwordWeight;
	case ITEM_TYPE::TWO_HAND_AXE:
		return CWeaponCraftParameters::TwoHandAxeWeight;
	case ITEM_TYPE::PIKE:
		return CWeaponCraftParameters::PikeWeight;
	case ITEM_TYPE::TWO_HAND_MACE:
		return CWeaponCraftParameters::TwoHandMaceWeight;
	
	// range weapon
	case ITEM_TYPE::AUTOLAUCH:
		return CWeaponCraftParameters::AutolauchWeight;
	case ITEM_TYPE::BOWRIFLE:
		return CWeaponCraftParameters::BowrifleWeight;
	case ITEM_TYPE::LAUNCHER:
		return CWeaponCraftParameters::LauncherWeight;
	case ITEM_TYPE::PISTOL:
		return CWeaponCraftParameters::PistolWeight;
	case ITEM_TYPE::BOWPISTOL:
		return CWeaponCraftParameters::BowpistolWeight;
	case ITEM_TYPE::RIFLE:
		return CWeaponCraftParameters::RifleWeight;
	
	// ammo
	case ITEM_TYPE::AUTOLAUNCH_AMMO:
		return CWeaponCraftParameters::AutolaunchAmmoWeight;
	case ITEM_TYPE::BOWRIFLE_AMMO:
		return CWeaponCraftParameters::BowrifleAmmoWeight;
	case ITEM_TYPE::LAUNCHER_AMMO:
		return CWeaponCraftParameters::LauncherAmmoWeight;
	case ITEM_TYPE::PISTOL_AMMO:
		return CWeaponCraftParameters::PistolAmmoWeight;
	case ITEM_TYPE::BOWPISTOL_AMMO:
		return CWeaponCraftParameters::BowpistolAmmoWeight;
	case ITEM_TYPE::RIFLE_AMMO:
		return CWeaponCraftParameters::RifleAmmoWeight;
	
	// armor and shield
	case ITEM_TYPE::SHIELD:
		return CWeaponCraftParameters::ShieldWeight;
	case ITEM_TYPE::BUCKLER:
		return CWeaponCraftParameters::BucklerWeight;
	case ITEM_TYPE::LIGHT_BOOTS:
		return CWeaponCraftParameters::LightBootsWeight;
	case ITEM_TYPE::LIGHT_GLOVES:
		return CWeaponCraftParameters::LightGlovesWeight;
	case ITEM_TYPE::LIGHT_PANTS:
		return CWeaponCraftParameters::LightPantsWeight;
	case ITEM_TYPE::LIGHT_SLEEVES:
		return CWeaponCraftParameters::LightSleevesWeight;
	case ITEM_TYPE::LIGHT_VEST:
		return CWeaponCraftParameters::LightVestWeight;
	case ITEM_TYPE::MEDIUM_BOOTS:
		return CWeaponCraftParameters::MediumBootsWeight;
	case ITEM_TYPE::MEDIUM_GLOVES:
		return CWeaponCraftParameters::MediumGlovesWeight;
	case ITEM_TYPE::MEDIUM_PANTS:
		return CWeaponCraftParameters::MediumPantsWeight;
	case ITEM_TYPE::MEDIUM_SLEEVES:
		return CWeaponCraftParameters::MediumSleevesWeight;
	case ITEM_TYPE::MEDIUM_VEST:
		return CWeaponCraftParameters::MediumVestWeight;
	case ITEM_TYPE::HEAVY_BOOTS:
		return CWeaponCraftParameters::HeavyBootsWeight;
	case ITEM_TYPE::HEAVY_GLOVES:
		return CWeaponCraftParameters::HeavyGlovesWeight;
	case ITEM_TYPE::HEAVY_PANTS:
		return CWeaponCraftParameters::HeavyPantsWeight;
	case ITEM_TYPE::HEAVY_SLEEVES:
		return CWeaponCraftParameters::HeavySleevesWeight;
	case ITEM_TYPE::HEAVY_VEST:
		return CWeaponCraftParameters::HeavyVestWeight;
	case ITEM_TYPE::HEAVY_HELMET:
		return CWeaponCraftParameters::HeavyHelmetWeight;
	
	// jewel
	case ITEM_TYPE::ANKLET:
		return CWeaponCraftParameters::AnkletWeight;
	case ITEM_TYPE::BRACELET:
		return CWeaponCraftParameters::BraceletWeight;
	case ITEM_TYPE::DIADEM:
		return CWeaponCraftParameters::DiademWeight;
	case ITEM_TYPE::EARING:
		return CWeaponCraftParameters::EaringWeight;
	case ITEM_TYPE::PENDANT:
		return CWeaponCraftParameters::PendantWeight;
	case ITEM_TYPE::RING:
		return CWeaponCraftParameters::RingWeight;
	default:
		return 0;
	}
}

// ***************************************************************************
// CStaticLootTable::selectRandomLootSet
// ***************************************************************************
CSheetId CStaticLootTable::selectRandomLootSet() const
{
	if( LootSets.empty() )
		return CSheetId::Unknown;

	// compute the probability sum
	uint16 probabilitySum = 0;
	map<CSheetId,uint16>::const_iterator itconst;
	for( itconst = LootSets.begin(); itconst != LootSets.end(); ++itconst )
	{
		probabilitySum += (*itconst).second;
	}

	// choose a random number between and probabilitySum
	uint32 randWeight;
	if( probabilitySum == 0 )
		randWeight = 0;
	else
		randWeight = RandomGenerator.rand(probabilitySum-1) + 1;

	// "concatenate" weights of each index, when the random value is reached we'll have the index to use
	uint16 w = 0;
	for( itconst = LootSets.begin(); itconst != LootSets.end(); ++itconst )
	{
		w += (*itconst).second;
		if( randWeight <= w )
		{
			break;
		}
	}
	if( itconst != LootSets.end() )
	{
		return (*itconst).first;
	}

	nlwarning("<CStaticLootTable::selectRandomLootSet> can't find any lootset rand=%d probabilitySum=%d weightCount=%d",randWeight,probabilitySum,LootSets.size());
	return CSheetId::Unknown;
}

// ***************************************************************************
// CStaticLootTable::selectRandomCustomLootSet
// ***************************************************************************
const CStaticLootSet *CStaticLootTable::selectRandomCustomLootSet() const
{
	if( CustomLootSets.empty() )
		return 0;

	// compute the probability sum
	uint16 probabilitySum = 0;
	multimap<uint16, CStaticLootSet>::const_iterator it = CustomLootSets.begin();
	for( ; it != CustomLootSets.end(); ++it )
	{
		probabilitySum += (*it).first;
	}
	
	// choose a random number between and probabilitySum
	uint32 randWeight;
	if( probabilitySum == 0 )
		randWeight = 0;
	else
		randWeight = RandomGenerator.rand(probabilitySum-1) + 1;
	
	// "concatenate" weights of each index, when the random value is reached we'll have the index to use
	uint16 w = 0;
	for (it = CustomLootSets.begin(); it != CustomLootSets.end(); ++it )
	{
		w += (*it).first;
		if( randWeight <= w )
		{
			break;
		}
	}

	if( it != CustomLootSets.end() )
	{
		return &(it->second);
	}

	nlwarning("Can't find any lootset rand=%d probabilitySum=%d weightCount=%d",randWeight,probabilitySum,CustomLootSets.size());
	return 0;
}

// ***************************************************************************
// QuarteringQuantityByVariable - EGS-specific initialization
// ***************************************************************************
static const float QuarteringForcedQuantities [6] = { 0, 1.0f, 2.0f, 3.0f, 4.0f, 0.5f };

const float *QuarteringQuantityByVariable [NBRMQuantityVariables] =
{
	&QuarteringQuantityAverageForCraftHerbivore.get(),
	&QuarteringQuantityAverageForCraftCarnivore.get(),
	&QuarteringQuantityAverageForBoss5.get(),
	&QuarteringQuantityAverageForBoss7.get(),
	&QuarteringQuantityForInvasion5.get(),
	&QuarteringQuantityForInvasion7.get(),
	&QuarteringForcedQuantities[0],
	&QuarteringForcedQuantities[1],
	&QuarteringForcedQuantities[2],
	&QuarteringForcedQuantities[3],
	&QuarteringForcedQuantities[4],
	&QuarteringForcedQuantities[5]
};
