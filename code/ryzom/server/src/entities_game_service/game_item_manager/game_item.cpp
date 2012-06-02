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

#include "game_item_manager/game_item.h"
#include "game_item_manager/game_item_manager.h"
#include "game_item_manager/weapon_craft_parameters.h"
#include "game_share/string_manager_sender.h"

#include "game_share/scenario.h"

#include "modules/character_control.h"
#include "modules/r2_mission_item.h"
#include "modules/character_control.h"

#include "weapon_damage_table.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "egs_sheets/egs_sheets.h"
#include "server_share/log_item_gen.h"

using namespace NLMISC;
using namespace std;
using namespace NLNET;

extern CPlayerManager PlayerManager;

NL_INSTANCE_COUNTER_IMPL(CGameItem);

//--------------------------------------------------------------------
// singleton data

CGameItemVector CGameItem::_Items;
uint32 CGameItem::_FirstFreeItem;
uint32 CGameItem::_BugTestCounter;

#if defined(ITEM_DEBUG) || defined(SAFE_ITEMS)
	sint32 CGameItem::_NextAllocatorDataValue;
#endif


// ****************************************************************************
// CCraftParameters
// ****************************************************************************

CCraftParameters::CCraftParameters()
{
	Durability = 0.0f;
	nbDurability = 0;
	Weight = 0.0f;
	nbWeight = 0;
	StatEnergy = 0.0f;
	nbStatEnergy = 0;
	
	Dmg = 0.0f;
	nbDmg = 0;
	Speed = 0.0f;
	nbSpeed = 0;
	SapLoad = 0.0f;
	nbSapLoad = 0;
	Range = 0.0f;
	nbRange = 0;
	DodgeModifier = 0.0f;
	nbDodgeModifier = 0;
	ParryModifier = 0.0f;
	nbParryModifier = 0;
	AdversaryDodgeModifier = 0.0f;
	nbAdversaryDodgeModifier = 0;
	AdversaryParryModifier = 0.0f;
	nbAdversaryParryModifier = 0;
	
	ProtectionFactor = 0.0f;
	nbProtectionFactor = 0;
	MaxSlashingProtection = 0.0f;
	nbMaxSlashingProtection = 0;
	MaxBluntProtection = 0.0f;
	nbMaxBluntProtection = 0;
	MaxPiercingProtection = 0.0f;
	nbMaxPiercingProtection = 0;
	Color.resize(8, 0); //need replace 8 by an enumerate type of color
	
	AcidProtectionFactor = 0.0f;
	nbAcidProtectionFactor = 0;
	ColdProtectionFactor = 0.0f;
	nbColdProtectionFactor = 0;
	FireProtectionFactor = 0.0f;
	nbFireProtectionFactor = 0;
	RotProtectionFactor = 0.0f;
	nbRotProtectionFactor = 0;
	ShockWaveProtectionFactor = 0.0f;
	nbShockWaveProtectionFactor = 0;
	PoisonProtectionFactor = 0.0f;
	nbPoisonProtectionFactor = 0;
	ElectricityProtectionFactor = 0.0f;
	nbElectricityProtectionFactor = 0;

	DesertResistanceFactor = 0.0f;
	nbDesertResistanceFactor = 0;
	ForestResistanceFactor = 0.0f;
	nbForestResistanceFactor = 0;
	LacustreResistanceFactor = 0.0f;
	nbLacustreResistanceFactor =0;
	JungleResistanceFactor = 0.0f;
	nbJungleResistanceFactor = 0;
	PrimaryRootResistanceFactor = 0.0f;
	nbPrimaryRootResistanceFactor = 0;

	ElementalCastingTimeFactor = 0.0f;
	nbElementalCastingTimeFactor = 0;
	ElementalPowerFactor = 0.0f;
	nbElementalPowerFactor = 0;
	OffensiveAfflictionCastingTimeFactor = 0.0f;
	nbOffensiveAfflictionCastingTimeFactor = 0;
	OffensiveAfflictionPowerFactor = 0.0f;
	nbOffensiveAfflictionPowerFactor = 0;
	HealCastingTimeFactor = 0.0f;
	nbHealCastingTimeFactor = 0;
	HealPowerFactor = 0.0f;
	nbHealPowerFactor = 0;
	DefensiveAfflictionCastingTimeFactor = 0.0f;
	nbDefensiveAfflictionCastingTimeFactor = 0;
	DefensiveAfflictionPowerFactor = 0.0f;
	nbDefensiveAfflictionPowerFactor = 0;
	
	HpBuff = 0;
	SapBuff = 0;
	StaBuff = 0;
	FocusBuff = 0;
}

// ****************************************************************************
// CItemCraftParameters
// ****************************************************************************

CItemCraftParameters::CItemCraftParameters()
{
	clear();
}

void CItemCraftParameters::clear()
{
	Durability = 0.0f;
	Weight = 0.0f;
	StatEnergy = 0.0f;
	
	Dmg = 0.0f;
	Speed = 0.0f;
	SapLoad = 0.0f;
	Range = 0.0f;
	DodgeModifier = 0.0f;
	ParryModifier = 0.0f;
	AdversaryDodgeModifier = 0.0f;
	AdversaryParryModifier = 0.0f;
	
	ProtectionFactor = 0.0f;
	MaxSlashingProtection = 0.0f;
	MaxBluntProtection = 0.0f;
	MaxPiercingProtection = 0.0f;
	Color = 1;
	
	Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
	Protection1 = PROTECTION_TYPE::None;
	Protection1Factor = 0.0f;
	Protection2 = PROTECTION_TYPE::None;
	Protection2Factor = 0.0f;
	Protection3 = PROTECTION_TYPE::None;
	Protection3Factor = 0.0f;
	
	DesertResistanceFactor = 0.0f;
	ForestResistanceFactor = 0.0f;
	LacustreResistanceFactor = 0.0f;
	JungleResistanceFactor = 0.0f;
	PrimaryRootResistanceFactor = 0.0f;
	
	ElementalCastingTimeFactor = 0.0f;
	ElementalPowerFactor = 0.0f;
	OffensiveAfflictionCastingTimeFactor = 0.0f;
	OffensiveAfflictionPowerFactor = 0.0f;
	HealCastingTimeFactor = 0.0f;
	HealPowerFactor = 0.0f;
	DefensiveAfflictionCastingTimeFactor = 0.0f;
	DefensiveAfflictionPowerFactor = 0.0f;
	
	HpBuff = 0;
	SapBuff = 0;
	StaBuff = 0;
	FocusBuff = 0;
}

float CItemCraftParameters::getCraftParameterValue( RM_FABER_STAT_TYPE::TRMStatType statType ) const
{
	switch( statType )
	{
		case RM_FABER_STAT_TYPE::Durability:
			return Durability;
		case RM_FABER_STAT_TYPE::Weight:
			return Weight;
		case RM_FABER_STAT_TYPE::SapLoad:
			return SapLoad;
		case RM_FABER_STAT_TYPE::DMG:
			return Dmg;
		case RM_FABER_STAT_TYPE::Speed:
			return Speed;
		case RM_FABER_STAT_TYPE::Range:
			return Range;
		case RM_FABER_STAT_TYPE::DodgeModifier:
			return DodgeModifier;
		case RM_FABER_STAT_TYPE::ParryModifier:
			return ParryModifier;
		case RM_FABER_STAT_TYPE::AdversaryDodgeModifier:
			return AdversaryDodgeModifier;
		case RM_FABER_STAT_TYPE::AdversaryParryModifier:
			return AdversaryParryModifier;
		case RM_FABER_STAT_TYPE::ProtectionFactor:
			return ProtectionFactor;
		case RM_FABER_STAT_TYPE::MaxSlashingProtection:
			return MaxSlashingProtection;
		case RM_FABER_STAT_TYPE::MaxBluntProtection:
			return MaxBluntProtection;
		case RM_FABER_STAT_TYPE::MaxPiercingProtection:
			return MaxPiercingProtection;
		case RM_FABER_STAT_TYPE::AcidProtection:
		case RM_FABER_STAT_TYPE::ColdProtection:
		case RM_FABER_STAT_TYPE::FireProtection:
		case RM_FABER_STAT_TYPE::RotProtection:
		case RM_FABER_STAT_TYPE::ShockWaveProtection:
		case RM_FABER_STAT_TYPE::PoisonProtection:
		case RM_FABER_STAT_TYPE::ElectricityProtection:
			if (Protection1 == statType)
				return Protection1Factor;
			else if (Protection2 == statType)
				return Protection2Factor;
			else if (Protection3 == statType)
				return Protection3Factor;
			else return 0.0f;
		case RM_FABER_STAT_TYPE::DesertResistance:
			return DesertResistanceFactor;
		case RM_FABER_STAT_TYPE::ForestResistance:
			return ForestResistanceFactor;
		case RM_FABER_STAT_TYPE::LacustreResistance:
			return LacustreResistanceFactor;
		case RM_FABER_STAT_TYPE::JungleResistance:
			return JungleResistanceFactor;
		case RM_FABER_STAT_TYPE::PrimaryRootResistance:
			return PrimaryRootResistanceFactor;
		case RM_FABER_STAT_TYPE::ElementalCastingTimeFactor:
			return ElementalCastingTimeFactor;
		case RM_FABER_STAT_TYPE::ElementalPowerFactor:
			return ElementalPowerFactor;
		case RM_FABER_STAT_TYPE::OffensiveAfflictionCastingTimeFactor:
			return OffensiveAfflictionCastingTimeFactor;
		case RM_FABER_STAT_TYPE::OffensiveAfflictionPowerFactor:
			return OffensiveAfflictionPowerFactor;
		case RM_FABER_STAT_TYPE::DefensiveAfflictionCastingTimeFactor:
			return DefensiveAfflictionCastingTimeFactor;
		case RM_FABER_STAT_TYPE::DefensiveAfflictionPowerFactor:
			return DefensiveAfflictionPowerFactor;
		case RM_FABER_STAT_TYPE::HealCastingTimeFactor:
			return HealCastingTimeFactor;
		case RM_FABER_STAT_TYPE::HealPowerFactor:
			return HealPowerFactor;
		default:
			return 0.0f;
	}
}

RM_FABER_STAT_TYPE::TRMStatType CItemCraftParameters::getBestItemStat() const
{
	float bestStatValue =-1.0f;
	RM_FABER_STAT_TYPE::TRMStatType bestStat = RM_FABER_STAT_TYPE::NumRMStatType;

	for( uint32 i = 0; i < RM_FABER_STAT_TYPE::NumRMStatType; ++i )
	{
		float value = getCraftParameterValue( (RM_FABER_STAT_TYPE::TRMStatType)i );
		if( value > bestStatValue )
		{
			bestStatValue = value;
			bestStat = (RM_FABER_STAT_TYPE::TRMStatType)i;
		}
	}
	return bestStat;
}

	/// serial validated point for a character
void CItemCraftParameters::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	uint32 version = CItemCraftParameters::getCurrentVersion();
	f.serial( version );

	f.serial( Durability );
	f.serial( Weight );
	if (version >= 3)
	{
		f.serial( StatEnergy );
	}

	f.serial( Dmg );
	f.serial( Speed );
	f.serial( SapLoad );
	f.serial( Range );
	f.serial( DodgeModifier );
	f.serial( ParryModifier );
	f.serial( AdversaryDodgeModifier );
	f.serial( AdversaryParryModifier );
	
	f.serial( ProtectionFactor );
	f.serial( MaxSlashingProtection );
	f.serial( MaxBluntProtection );
	f.serial( MaxPiercingProtection );
	f.serial( Color );
	
	f.serialEnum( Protection );
	if (version >= 4)
	{
		f.serialEnum( Protection1 );
		f.serial( Protection1Factor );
		f.serialEnum( Protection2 );
		f.serial( Protection2Factor );
		f.serialEnum( Protection3 );
		f.serial( Protection3Factor );
	}
	
	if( version >= 5)
	{
		f.serial( DesertResistanceFactor );
		f.serial( ForestResistanceFactor );
		f.serial( LacustreResistanceFactor );
		f.serial( JungleResistanceFactor );
		f.serial( PrimaryRootResistanceFactor );
	}

	if (version >= 2)
	{
		f.serial( ElementalCastingTimeFactor );
		f.serial( ElementalPowerFactor );
		f.serial( OffensiveAfflictionCastingTimeFactor );
		f.serial( OffensiveAfflictionPowerFactor );
		f.serial( HealCastingTimeFactor );
		f.serial( HealPowerFactor );
		f.serial( DefensiveAfflictionCastingTimeFactor );
		f.serial( DefensiveAfflictionPowerFactor );
	}
	
	f.serial( HpBuff );
	f.serial( SapBuff );
	f.serial( StaBuff );
	f.serial( FocusBuff );
}

// operator !=
bool CItemCraftParameters::operator!=(const CItemCraftParameters &p) const
{
	return !(*this==p);
}

// operator ==
bool CItemCraftParameters::operator==(const CItemCraftParameters &p) const
{
	if ( Durability != p.Durability )
		return false;
	if ( Weight != p.Weight )
		return false;
	if ( SapLoad != p.SapLoad )
		return false;
	if ( Dmg != p.Dmg )
		return false;
	if ( Speed != p.Speed )
		return false;
	if ( Range != p.Range )
		return false;
	if ( DodgeModifier != p.DodgeModifier )
		return false;
	if ( ParryModifier != p.ParryModifier )
		return false;
	if ( AdversaryDodgeModifier != p.AdversaryDodgeModifier )
		return false;
	if ( AdversaryParryModifier != p.AdversaryParryModifier )
		return false;
	if ( ProtectionFactor != p.ProtectionFactor )
		return false;
	if ( MaxSlashingProtection != p.MaxSlashingProtection )
		return false;
	if ( MaxBluntProtection != p.MaxBluntProtection )
		return false;
	if ( MaxPiercingProtection != p.MaxPiercingProtection )
		return false;
	if ( Protection != p.Protection )
		return false;
	if( !checkProtectionEgality( p ) )
		return false;
	if( DesertResistanceFactor != p.DesertResistanceFactor )
		return false;
	if( ForestResistanceFactor != p.ForestResistanceFactor )
		return false;
	if( LacustreResistanceFactor != p.LacustreResistanceFactor )
		return false;
	if( JungleResistanceFactor != p.JungleResistanceFactor )
		return false;
	if( PrimaryRootResistanceFactor != p.PrimaryRootResistanceFactor )
		return false;
	if ( Color != p.Color )
		return false;
	if ( ElementalCastingTimeFactor != p.ElementalCastingTimeFactor )
		return false;
	if ( ElementalPowerFactor != p.ElementalPowerFactor )
		return false;
	if ( OffensiveAfflictionCastingTimeFactor != p.OffensiveAfflictionCastingTimeFactor )
		return false;
	if ( OffensiveAfflictionPowerFactor != p.OffensiveAfflictionPowerFactor )
		return false;
	if ( HealCastingTimeFactor != p.HealCastingTimeFactor )
		return false;
	if ( HealPowerFactor != p.HealPowerFactor )
		return false;
	if ( DefensiveAfflictionCastingTimeFactor != p.DefensiveAfflictionCastingTimeFactor )
		return false;
	if ( DefensiveAfflictionPowerFactor != p.DefensiveAfflictionPowerFactor )
		return false;
	if ( HpBuff != p.HpBuff )
		return false;
	if ( SapBuff != p.SapBuff )
		return false;
	if ( StaBuff != p.StaBuff )
		return false;
	if ( FocusBuff != p.FocusBuff )
		return false;

	return true;
}
// operator =
const CItemCraftParameters& CItemCraftParameters::operator = ( const CCraftParameters& p )
{
	Durability = p.Durability;
	Weight = p.Weight;
	
	Dmg = p.Dmg;
	Speed = p.Speed;
	SapLoad = p.SapLoad;
	StatEnergy = p.StatEnergy;
	Range = p.Range;
	DodgeModifier = p.DodgeModifier;
	ParryModifier = p.ParryModifier;
	AdversaryDodgeModifier = p.AdversaryDodgeModifier;
	AdversaryParryModifier = p.AdversaryParryModifier;
	
	ProtectionFactor = p.ProtectionFactor;
	MaxSlashingProtection = p.MaxSlashingProtection;
	MaxBluntProtection = p.MaxBluntProtection;
	MaxPiercingProtection = p.MaxPiercingProtection;

	keepTheThreeBestProtection( p );

	keepTheThreeBestResistance( p );

	ElementalCastingTimeFactor	= p.ElementalCastingTimeFactor;
	ElementalPowerFactor		= p.ElementalPowerFactor;
	OffensiveAfflictionCastingTimeFactor= p.OffensiveAfflictionCastingTimeFactor;
	OffensiveAfflictionPowerFactor		= p.OffensiveAfflictionPowerFactor;
	HealCastingTimeFactor		= p.HealCastingTimeFactor;
	HealPowerFactor				= p.HealPowerFactor;
	DefensiveAfflictionCastingTimeFactor= p.DefensiveAfflictionCastingTimeFactor;
	DefensiveAfflictionPowerFactor		= p.DefensiveAfflictionPowerFactor;

	uint32 MaxNumColor = 0;
	uint8 DominanteColor = (uint8)1;
	for( uint32 i = 0; i < p.Color.size(); ++i )
	{
		if( p.Color[i] > MaxNumColor )
		{
			MaxNumColor = p.Color[i];
			DominanteColor = (uint8)i;
		}
	}
	Color = DominanteColor;
	
	HpBuff = p.HpBuff;
	SapBuff = p.SapBuff;
	StaBuff = p.StaBuff;
	FocusBuff = p.FocusBuff;

	return *this;
}

void CItemCraftParameters::keepTheThreeBestProtection( const CCraftParameters& p )
{
	for( uint i = 0; i < PROTECTION_TYPE::NB_PROTECTION_TYPE; ++i )
	{
		PROTECTION_TYPE::TProtectionType protection;
		float protectionFactor;

		protection = (PROTECTION_TYPE::TProtectionType) i;
		
		switch( (PROTECTION_TYPE::TProtectionType) i )
		{
		case PROTECTION_TYPE::Acid:
			protectionFactor = p.AcidProtectionFactor;
			break;
		case PROTECTION_TYPE::Cold:
			protectionFactor = p.ColdProtectionFactor;
			break;
		case PROTECTION_TYPE::Fire:
			protectionFactor = p.FireProtectionFactor;
			break;
		case PROTECTION_TYPE::Rot:
			protectionFactor = p.RotProtectionFactor;
			break;
		case PROTECTION_TYPE::Shockwave:
			protectionFactor = p.ShockWaveProtectionFactor;
			break;
		case PROTECTION_TYPE::Poison:
			protectionFactor = p.PoisonProtectionFactor;
			break;
		case PROTECTION_TYPE::Electricity:
			protectionFactor = p.ElectricityProtectionFactor;
			break;
		default: continue;
		}

		if (protectionFactor > Protection1Factor)
		{
			Protection3 = Protection2;
			Protection3Factor = Protection2Factor;
			Protection2 = Protection1;
			Protection2Factor = Protection1Factor;
			Protection1 = protection;
			Protection1Factor = protectionFactor;
		}
		else if (protectionFactor > Protection2Factor)
		{
			Protection3 = Protection2;
			Protection3Factor = Protection2Factor;
			Protection2 = protection;
			Protection2Factor = protectionFactor;
		}
		else if (protectionFactor > Protection3Factor)
		{
			Protection3 = protection;
			Protection3Factor = protectionFactor;
		}
	}
}

bool CItemCraftParameters::checkProtectionEgality(const CItemCraftParameters &p) const
{
	bool result = true;

	if (Protection1 == p.Protection1)
		result &= (Protection1Factor == p.Protection1Factor);
	else if (Protection1 == p.Protection2)
		result &= (Protection1Factor == p.Protection2Factor);
	else if (Protection1 == p.Protection3)
		result &= (Protection1Factor == p.Protection3Factor);
	else return false;

	if (Protection2 == p.Protection1)
		result &= (Protection2Factor == p.Protection1Factor);
	else if (Protection2 == p.Protection2)
		result &= (Protection2Factor == p.Protection2Factor);
	else if (Protection2 == p.Protection3)
		result &= (Protection2Factor == p.Protection3Factor);
	else return false;

	if (Protection3 == p.Protection1)
		result &= (Protection3Factor == p.Protection1Factor);
	else if (Protection3 == p.Protection2)
		result &= (Protection3Factor == p.Protection2Factor);
	else if (Protection3 == p.Protection3)
		result &= (Protection3Factor == p.Protection3Factor);
	else return false;

	return result;
}

void CItemCraftParameters::keepTheThreeBestResistance( const CCraftParameters& p )
{
	class CBestResistance
	{
	public:
#define NbResistanceKeeped 3

		struct TBestResistance
		{
			RESISTANCE_TYPE::TResistanceType BestResistance;
			float	BestResistanceValue;

			TBestResistance()
			{
				BestResistance = RESISTANCE_TYPE::None;
				BestResistanceValue = 0.0f;
			}
		};

		CBestResistance() {}

		void keepResistanceIfBest( float value, RESISTANCE_TYPE::TResistanceType resistanceType )
		{
			for( uint i = 0; i < NbResistanceKeeped; ++i )
			{
				if( value > _BestResistance[ i ].BestResistanceValue )
				{
					if( NbResistanceKeeped > 1 )
					{
						for( sint j = NbResistanceKeeped - 2; j >= (sint32)i; --j )
						{
							_BestResistance[ j + 1 ] = _BestResistance[ j ];
						}
					}

					_BestResistance[ i ].BestResistanceValue = value;
					_BestResistance[ i ].BestResistance = resistanceType;

					break;
				}
			}
		}

		float getResistanceValueIfBest( RESISTANCE_TYPE::TResistanceType resistanceType ) const
		{
			for( uint i = 0; i < NbResistanceKeeped; ++i )
			{
				if( _BestResistance[ i ].BestResistance == resistanceType )
				{
					return _BestResistance[ i ].BestResistanceValue;
				}
			}
			return 0.0f;
		}

	private:
		TBestResistance _BestResistance[NbResistanceKeeped];
	};

	CBestResistance bestResistance;

	bestResistance.keepResistanceIfBest( p.DesertResistanceFactor, RESISTANCE_TYPE::Desert );
	bestResistance.keepResistanceIfBest( p.ForestResistanceFactor, RESISTANCE_TYPE::Forest );
	bestResistance.keepResistanceIfBest( p.LacustreResistanceFactor, RESISTANCE_TYPE::Lacustre );
	bestResistance.keepResistanceIfBest( p.JungleResistanceFactor, RESISTANCE_TYPE::Jungle );
	bestResistance.keepResistanceIfBest( p.PrimaryRootResistanceFactor, RESISTANCE_TYPE::PrimaryRoot );
	
	DesertResistanceFactor = bestResistance.getResistanceValueIfBest(RESISTANCE_TYPE::Desert);
	ForestResistanceFactor = bestResistance.getResistanceValueIfBest(RESISTANCE_TYPE::Forest);
	LacustreResistanceFactor = bestResistance.getResistanceValueIfBest(RESISTANCE_TYPE::Lacustre);
	JungleResistanceFactor = bestResistance.getResistanceValueIfBest(RESISTANCE_TYPE::Jungle);
	PrimaryRootResistanceFactor = bestResistance.getResistanceValueIfBest(RESISTANCE_TYPE::PrimaryRoot);
}

// ****************************************************************************
// CGameItem
// ****************************************************************************

//-----------------------------------------------
// areStackable
//-----------------------------------------------
bool CGameItem::areStackable(const CGameItemPtr item1, const CGameItemPtr item2)
{
	if (item1 == NULL || item2 == NULL)
		return false;
	
	// test sheet
	if (item1->_SheetId != item2->_SheetId)
		return false;
	
	// test quality/recommanded level
	if (item1->_Recommended != item2->_Recommended)
		return false;
	
	// for craftable items test craft params
	if (item1->_Form != NULL && item1->_Form->CraftPlan != NLMISC::CSheetId::Unknown)
	{
		if( item1->_CraftParameters != 0 )
		{
			if( item2->_CraftParameters != 0 )
			{
				if ( *item1->_CraftParameters != *item2->_CraftParameters )
					return false;
			}
			else 
				return false;
		}
		else
			if( item2->_CraftParameters != 0 )
				return false;
	}
	
	return true;
}

//-----------------------------------------------
// CInventoryItem:
//
//-----------------------------------------------

void CGameItem::callStackSizeChanged(uint32 previousStackSize)
{
	if (_Inventory != NULL)
		_Inventory->onItemStackSizeChanged(getInventorySlot(), previousStackSize);
	if (_RefInventory != NULL)
		_RefInventory->onItemStackSizeChanged(getRefInventorySlot(), previousStackSize);
}

void CGameItem::callItemChanged(INVENTORIES::TItemChangeFlags changeFlags)
{
	if (_Inventory != NULL)
		_Inventory->onItemChanged(getInventorySlot(), changeFlags);
	if (_RefInventory != NULL)
		_RefInventory->onItemChanged(getRefInventorySlot(), changeFlags);
}

//-----------------------------------------------
// CGameItemPtr:
//
//-----------------------------------------------

void CGameItemPtr::deleteItem()
{
	CGameItem *item=**this;

	BOMB_IF( item == NULL, "Attempt to delete an item that is not allocated or has been freed", return );

	if (item->_Inventory != NULL)
		item->_Inventory->removeItem(item->getInventorySlot());

	// only unlink if the pointer 'this' is not the same as the inventory ptr for the item
	// after the unlink this == NULL
//	if (! ( item->_Parent!=NULL && 
//			item->Loc.Slot<item->_Parent->getChildren().size() && 
//			&(item->_Parent->getChildren()[item->Loc.Slot])==this ) )
	unlinkFromItem();

	// call dtor now to unlink all children
	item->dtor();
	// check no one else is referencing us
	nlassert(item->_Ptrs.empty());

	CGameItem::deleteItem(item);
	item->_BugTestUpdate=CGameItem::_BugTestCounter;
}

//-----------------------------------------------
// CGameItem
//
//-----------------------------------------------


//-----------------------------------------------
// ctor :
//
//-----------------------------------------------
void CGameItem::ctor()
{
	// if we have children already we need to get rid of them cleanly!
//	if (!_Children.empty())
//		dtor();

	// Generate a new Item id
	_ItemId = INVENTORIES::TItemId();

	clear();
};

//-----------------------------------------------
// ctor :
//
//-----------------------------------------------
//void CGameItem::ctor( const CEntityId& id, const CSheetId& sheetId, uint32 recommended, sint16 slotCount, bool destroyable, bool dropable )
void CGameItem::ctor( const CSheetId& sheetId, uint32 recommended, bool destroyable, bool dropable )
{
	_CraftParameters = 0;
	ctor();
//	_ItemId = id;
	_SheetId = sheetId;
	_Destroyable = destroyable;
	_Dropable = dropable;
//	SlotCount = slotCount;
	_Recommended = recommended;
	_TotalSaleCycle = 0;
	_PetIndex = MAX_INVENTORY_ANIMAL;

	_Form = CSheets::getForm( sheetId );
	if (_Form)
	{
//		if( SlotCount > 0 )
//		{
//			_Children.reserve( SlotCount );
//			if( _Form->Family == ITEMFAMILY::BAG )
//			{
//				_Children.resize( SlotCount, NULL);
//			}
//			else if( _Form->Family == ITEMFAMILY::STACK )
//			{
//				_Children.resize( 0 );
//			}
//		}
//		else
//		{
//			if( _Form->Family == ITEMFAMILY::CRAFTING_TOOL || _Form->Family == ITEMFAMILY::HARVEST_TOOL )
			{
				_HP = maxDurability();
			}
//		}

		// compute default requirement from _Form
		computeRequirementFromForm();

		// Form TypeSkillMods
		_TypeSkillMods = _Form->TypeSkillMods;
	}
}

//-----------------------------------------------
// Compute required level (skills and charac) for wearing item
//-----------------------------------------------
void CGameItem::computeRequiredLevel()
{
	if( _Form )
	{
		// new system? compute from from
		if(_UseNewSystemRequirement)
		{
			sint32 value = 0;
			value = (sint32) _Form->RequiredSkillQualityFactor * _Recommended + _Form->RequiredSkillQualityOffset;
			if( value < (sint32)_Form->MinRequiredSkillLevel || value < 0 )
			{
				value = _Form->MinRequiredSkillLevel;
			}
			_RequiredSkillLevel = (uint16) value;
			
			value = (sint32) (_Form->RequiredSkillQualityFactor2 * _Recommended + _Form->RequiredSkillQualityOffset2);
			if( value < (sint32)_Form->MinRequiredSkillLevel2 || value < 0 )
			{
				value = _Form->MinRequiredSkillLevel2;
			}
			_RequiredSkillLevel2 = (uint16) value;
			
			value = (sint32) (_Form->RequiredCharacQualityFactor * _Recommended + _Form->RequiredCharacQualityOffset);
			if( value < (sint32)_Form->MinRequiredCharacLevel || value < 0 )
			{
				value = _Form->MinRequiredCharacLevel;
			}
			_RequiredCharacLevel = (uint16) value;
		}
		else
		{
		// Old System: it used only Charac query, according to item type
			_RequiredSkillLevel= 0;
			_RequiredSkillLevel2= 0;
			_RequiredCharacLevel = 0;
			switch( _Form->Family )
			{
			// **** ARMORS / BUCKLERS
			case ITEMFAMILY::ARMOR:
			case ITEMFAMILY::SHIELD:
				switch( _Form->Type )
				{
				case ITEM_TYPE::MEDIUM_BOOTS:
				case ITEM_TYPE::MEDIUM_GLOVES:
				case ITEM_TYPE::MEDIUM_PANTS:
				case ITEM_TYPE::MEDIUM_SLEEVES:
				case ITEM_TYPE::MEDIUM_VEST:
				case ITEM_TYPE::BUCKLER:
					// Constitution requirement
					_RequiredCharacLevel = (uint16)floor(_Recommended / 1.5f);
					break;
				case ITEM_TYPE::HEAVY_BOOTS:
				case ITEM_TYPE::HEAVY_GLOVES:
				case ITEM_TYPE::HEAVY_PANTS:
				case ITEM_TYPE::HEAVY_SLEEVES:
				case ITEM_TYPE::HEAVY_VEST:
				case ITEM_TYPE::HEAVY_HELMET:
				case ITEM_TYPE::SHIELD:
					// Constitution requirement
					_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
					break;
				default:
					// No carac requirement
					break;
				}
				break;
				
			// **** MELEE_WEAPONS
			case ITEMFAMILY::MELEE_WEAPON:
				switch( _Form->Type )
				{
				case ITEM_TYPE::MAGICIAN_STAFF:
					// Intelligence requirement
					_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
					break;
				default:
					// Strength requirement
					_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
					break;
				}
				break;
				
			// **** RANGE_WEAPON
			case ITEMFAMILY::RANGE_WEAPON:
				_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
				break;
				
				// No carac requirement
			default:
				break;
			};
		}
	}		

	computeHasPrerequisit();
}


//-----------------------------------------------
// Compute whether item has a skill/charac prerequisit
//-----------------------------------------------
void CGameItem::computeHasPrerequisit()
{
	_HasPrerequisit = _RequiredSkillLevel>0 || _RequiredSkillLevel2>0 || _RequiredCharacLevel>0;
}


//-----------------------------------------------
// computeRequirementFromForm :
//-----------------------------------------------
void CGameItem::computeRequirementFromForm()
{
	if(_Form)
	{
		_UseNewSystemRequirement = true;
		_RequiredSkill = _Form->RequiredSkill;
		_RequiredSkill2 = _Form->RequiredSkill2;
		_RequiredCharac = _Form->RequiredCharac;
		computeRequiredLevel();
	}
}

//-----------------------------------------------
// computeRequirementFromOldSystem : for backward compatibility
//-----------------------------------------------
void CGameItem::computeRequirementFromOldSystem()
{
	if(_Form)
	{
		// NB: this item CAN'T BE muted to the new system (because of setRecommended and computeRequiredLevel())
		_UseNewSystemRequirement = false;
		_RequiredSkill = SKILLS::unknown;
		_RequiredSkillLevel = 0;
		_RequiredSkill2 = SKILLS::unknown;
		_RequiredSkillLevel2 = 0;
		_RequiredCharac = CHARACTERISTICS::Unknown;
		_RequiredCharacLevel = 0;

		// Old System: it used only Charac query, according to item type
		switch( _Form->Family )
		{
		// **** ARMORS / BUCKLERS
		case ITEMFAMILY::ARMOR:
		case ITEMFAMILY::SHIELD:
			switch( _Form->Type )
			{
			case ITEM_TYPE::MEDIUM_BOOTS:
			case ITEM_TYPE::MEDIUM_GLOVES:
			case ITEM_TYPE::MEDIUM_PANTS:
			case ITEM_TYPE::MEDIUM_SLEEVES:
			case ITEM_TYPE::MEDIUM_VEST:
			case ITEM_TYPE::BUCKLER:
				// Constitution requirement
				_RequiredCharac = CHARACTERISTICS::constitution;
				_RequiredCharacLevel = (uint16)floor(_Recommended / 1.5f);
				break;
			case ITEM_TYPE::HEAVY_BOOTS:
			case ITEM_TYPE::HEAVY_GLOVES:
			case ITEM_TYPE::HEAVY_PANTS:
			case ITEM_TYPE::HEAVY_SLEEVES:
			case ITEM_TYPE::HEAVY_VEST:
			case ITEM_TYPE::HEAVY_HELMET:
			case ITEM_TYPE::SHIELD:
				// Constitution requirement
				_RequiredCharac = CHARACTERISTICS::constitution;
				_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
				break;
			default:
				// No carac requirement
				break;
			}
			break;
			
		// **** MELEE_WEAPONS
		case ITEMFAMILY::MELEE_WEAPON:
			switch( _Form->Type )
			{
			case ITEM_TYPE::MAGICIAN_STAFF:
				// Intelligence requirement
				_RequiredCharac = CHARACTERISTICS::intelligence;
				_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
				break;
			default:
				// Strength requirement
				_RequiredCharac = CHARACTERISTICS::strength;
				_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
				break;
			}
			break;
				
		// **** RANGE_WEAPON
		case ITEMFAMILY::RANGE_WEAPON:
			_RequiredCharac = CHARACTERISTICS::well_balanced;
			_RequiredCharacLevel = (uint16)(max(0, ((sint)_Recommended-10)));
			break;

		// No carac requirement
		default:
			break;
		};

		// And compute required level from it (if any)
		computeRequiredLevel();
	}
}

//-----------------------------------------------
// reloadSapLoad :
//-----------------------------------------------
void CGameItem::reloadSapLoad( uint32 sapAdded )
{
	_SapLoad = std::min( maxSapLoad(), _SapLoad + sapAdded );

	if (getInventory() != NULL)
		getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_enchant));
}

//-----------------------------------------------
// consumeSapLoad :
//-----------------------------------------------
void CGameItem::consumeSapLoad( uint32 sapConsumed )
{
	_SapLoad = (uint32) ( (sint32) ( std::max( (sint32)0, ((sint32)(_SapLoad)) - ((sint32)sapConsumed) ) ) ); 

	if (getInventory() != NULL)
		getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_enchant));
}

//-----------------------------------------------
// setSapLoad :
//-----------------------------------------------
void CGameItem::setSapLoad( uint32 sap )
{
	if ((_SheetId == NLMISC::CSheetId("item_sap_recharge.sitem")) || (_SheetId == NLMISC::CSheetId("light_sap_recharge.sitem")))
	{
		_SapLoad = sap;
	} 
}

//-----------------------------------------------
// applyEnchantment :
//-----------------------------------------------
void CGameItem::applyEnchantment( const vector< CSheetId >& action ) 
{ 
	_Enchantment = action; 

	if (getInventory() != NULL)
		getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_enchant));
}

//-----------------------------------------------
// resetEnchantment :
//-----------------------------------------------
void CGameItem::resetEnchantment()
{
	_SapLoad = 0;
	_Enchantment.clear();
	contReset( _Enchantment );
	_LatencyEndDate = 0;

	if (getInventory() != NULL)
		getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_enchant));
}

//-----------------------------------------------
// computeItemWornState :
//-----------------------------------------------
void CGameItem::computeItemWornState()
{
	const uint32 maxHp = maxDurability();
	
	// Do not evaluate the worn state if the item max durability is zero (mp case)
	if (maxHp == 0)
		return;
	
	if (_HP > 1 && _HP > uint32(WornState1*maxHp) )
	{
		_CurrentWornState = ITEM_WORN_STATE::Unspoiled;
	}
	else if ( _HP > 1 && _HP > uint32(WornState2*maxHp) )
	{
		_CurrentWornState = ITEM_WORN_STATE::WornState1;
	}
	else if ( _HP > 1 && _HP > uint32(WornState3*maxHp) )
	{
		_CurrentWornState = ITEM_WORN_STATE::WornState2;
	}
	else if (_HP > 1 && _HP > uint32(WornState4*maxHp) )
	{
		_CurrentWornState = ITEM_WORN_STATE::WornState3;
	}
	else if (_HP > 1 )
	{
		_CurrentWornState = ITEM_WORN_STATE::WornState4;
	}
	else
	{
		_CurrentWornState = ITEM_WORN_STATE::Worned;
		if (getInventory() != NULL)
			getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_worned));
	}
}

void CGameItem::setLockCount(uint32 lockCount) 
{ 
	nlassert(lockCount <= _StackSize);
	_LockCount = lockCount;

	callItemChanged(INVENTORIES::TItemChangeFlags(INVENTORIES::itc_lock_state));
}


uint32	CGameItem::getNonLockedStackSize()
{
	if (_LockCount >= _StackSize)
		return 0;
	else
		return _StackSize - _LockCount;
}

void CGameItem::setLockedByOwner(bool value)
{
	if (value != _LockedByOwner)
	{
		_LockedByOwner = value;
		callItemChanged(INVENTORIES::TItemChangeFlags(INVENTORIES::itc_lock_state));
	}
}

//-----------------------------------------------
// getCopy :
//-----------------------------------------------
CGameItemPtr CGameItem::getItemCopy()
{
	// create a new item
	CGameItemPtr ret;
	ret.newItem(true);

	// backup id and allocation data
//	CEntityId itemId(RYZOMID::object, GameItemManager.getFreeItemIndice() );
	sint32 alloc = ret->_AllocatorData;

	// use the default copy ctor to init it
	CGameItem* item = *ret;
	CGameItemPtrArray old;
	old= *item;
	*item = *this;
	if( this->_CraftParameters != 0 )
	{
		item->_CraftParameters = new CItemCraftParameters(*this->_CraftParameters);
	}
	*(CGameItemPtrArray*)item=old;

	// generate a new item id
	item->_ItemId = INVENTORIES::TItemId();
	// reset allocator
	item->_AllocatorData = alloc;
	// reset dynamic elements. ( we do it this way because people who add gameplay data properties will probably forget to update this method. So it is safe this way
	item->_StackSize = _StackSize;
//	item->_IsOnTheGround	= false;
	item->_Looter		= CEntityId::Unknown;
//	item->_Id			= itemId;
	item->_AllocatorData = alloc;
//	item->_IsOnTheGround = false;
//	item->_Parent = NULL;
	item->_Inventory = NULL;
	item->_InventorySlot = INVENTORIES::INVALID_INVENTORY_SLOT;
	item->_RefInventory = NULL;
	item->_RefInventorySlot = INVENTORIES::INVALID_INVENTORY_SLOT;
	item->_CreatorId = _CreatorId;
	item->_LockCount = 0;
	item->_HP = item->maxDurability();

	item->_LatencyEndDate = _LatencyEndDate;
	item->_Enchantment = _Enchantment;
	item->_SapLoad = _SapLoad;
	item->_LostHPremains = 0.0f;
	item->_PhraseId = _PhraseId;
	item->computeItemWornState();

	log_Item_Create(item->getItemId(), item->getSheetId(), item->getStackSize(), item->quality());

	return ret;
}
//-----------------------------------------------
// sendNameId :
//-----------------------------------------------
uint32 CGameItem::sendNameId(CCharacter * user)
{
	nlassert( _Form != 0 );
	if( _Form->Family != ITEMFAMILY::SCROLL_R2 )
	{
		if ( ! _PhraseId.empty() )
			return STRING_MANAGER::sendStringToClient( user->getEntityRowId(), _PhraseId, TVectorParamCheck() );
	}
	else
	{
		// sessionId() don't works with edition session
		TSessionId scenarioId = ICharacterControl::getInstance()->getSessionId( user->currentSessionId());
		const R2::TMissionItem * itemDesc = CR2MissionItem::getInstance().getR2ItemDefinition( scenarioId, _SheetId );
		if( itemDesc != 0 )
		{
			SM_STATIC_PARAMS_1(params,STRING_MANAGER::literal);
			params[0].Literal = itemDesc->Name;
			return STRING_MANAGER::sendStringToClient( user->getEntityRowId(),"LITERAL", params );
		}
	}
	return 0;
}

//-----------------------------------------------
// ~CGameItem :
//-----------------------------------------------
void CGameItem::dtor()
{
	// detach from parent
//	detachFromParent();
//	detachFromInventory();

	if (getInventory() != NULL)
	{
		getInventory()->removeItem(getInventorySlot());
	}

	// destroy children. As they call detachFromParent() when destroyed and _Children vector may be resized 
	// we must make a copy of _children ptrs before
	// so we now work on a safe copy of _Children vector
//	vector<CGameItemPtr> children = _Children;
//	const sint size = children.size();
//	for (sint i = size-1 ; i >= 0 ; --i)
//	{
//		if(children[i] != NULL && children[i] != this)
//		{
//			// debug level 2 loop
//			for(uint j = 0; j  < children[i]->_Children.size(); j++) 
//				if(children[i]->_Children[j] == this) goto failed;
//				
//				detachChild(i);
//				children[i].deleteItem();
//		}
//	}
//	if( _CraftParameters != NULL )
//	{
//		delete _CraftParameters;
//		_CraftParameters = 0;
//	}
//	failed:
//	_Children.clear();
//	GameItemManager.removeFromMap( _Id );

//	CGameItemPtr item(this);
//	GameItemManager.removeFromMap(item);
}

//void CGameItem::copyItem(const CGameItemPtr &model)
//{
//	// Copy all parameter for the item
//	// Inventory info, lock state and looter are not copied
//	_StackSize			= model->_StackSize;
//	//_Looter
//	_SheetId			= model->_SheetId;
//	_Destroyable		= model->_Destroyable;
//	_Dropable			= model->_Dropable;
//	_Recommended		= model->_Recommended;
//	_HP					= model->_HP;
//	_LostHPremains		= model->_LostHPremains;
//	_CurrentWornState	= model->_CurrentWornState;
//	_SapLoad			= model->_SapLoad;
//	_CraftParameters	= model->_CraftParameters;
//	_CreatorId			= model->_CreatorId;
//	//_LockCount
//	_Enchantment		= model->_Enchantment;
//	_Form				= model->_Form;
//	_PhraseId			= model->_PhraseId;
//	_LatencyEndDate		= model->_LatencyEndDate;
//	_TotalSaleCycle		= model->_TotalSaleCycle;
//
//	computeRequirementFromForm();
//
//	_TypeSkillMods		= model->_TypeSkillMods;
//}
//

//-----------------------------------------------
// clear :
//
//-----------------------------------------------
void CGameItem::clear()
{
	// remove all of the item's children
//	for (uint32 i=0;i<_Children.size();++i)
//		if (_Children[i]!=NULL)
//			_Children[i].deleteItem();

	// if the item was a stack then resize the children vector back down to 0
//	if (_SheetId==NLMISC::CSheetId("stack.sitem"))
//	{
//		// TODO : remove when no more trace of stack.sitem
//		nlassert(false);
//		_Children.clear();
//	}

	// clear the enchantment
	_Enchantment.clear();
	contReset( _Enchantment );

	if( _CraftParameters )
	{
		_CraftParameters->clear();
		delete _CraftParameters;
		_CraftParameters = 0;
	}

	_StackSize = 1;
//	_IsOnTheGround = false;
	_LockCount = 0;

	_CreatorId = NLMISC::CEntityId::Unknown;
//	TimeOnTheGround = 0;
//	SlotCount = 0;
	_Destroyable = true;
	_Dropable = true;
//	_SlotImage = 0xFFFF;
	_LatencyEndDate = 0;
//	_Parent = NULL;
	_Inventory = NULL;
	_InventorySlot = INVENTORIES::INVALID_INVENTORY_SLOT;
	_Form = NULL;
	_ClientInventoryPosition = -1;
	_HP = 0;
	_CurrentWornState = ITEM_WORN_STATE::Unspoiled;
	_LostHPremains = 0.0f;
	_SapLoad = 0;
	_TotalSaleCycle = 0;

	_UseNewSystemRequirement = false;
	_RequiredSkill = SKILLS::unknown;
	_RequiredSkillLevel = 0;
	_RequiredSkill2 = SKILLS::unknown;
	_RequiredSkillLevel2 = 0;
	_RequiredCharac = CHARACTERISTICS::Unknown;
	_RequiredCharacLevel = 0;
	_HasPrerequisit= false;

	_LockedByOwner = false;

	_TypeSkillMods.clear();
	_PhraseId.clear();
	_CustomText.clear();
}

void CGameItem::setStackSize(uint32 size)
{
	nlassert(size!= 0);
#ifdef NL_DEBUG
	if (_Form != NULL)
		nlassert(size <= getMaxStackSize());
#endif

	if (_StackSize == size)
		return;


	uint32 prevSize = _StackSize;
	_StackSize = size;

	log_Item_UpdateQuantity(_ItemId, _StackSize, prevSize);
	
	if (_LockCount > _StackSize)
		_LockCount = _StackSize;

	// Callback inventories
	if (_Inventory)
		_Inventory->onItemStackSizeChanged(getInventorySlot(), prevSize);

	if (_RefInventory)
		_RefInventory->onItemStackSizeChanged(getRefInventorySlot(), prevSize);
}

uint32 CGameItem::fillStack(uint32 addQt)
{
	uint32 maxStack = getMaxStackSize();
	uint32 allowedQt = min(getStackSize()+addQt, maxStack);
	uint32 ret = addQt - (allowedQt - getStackSize());

	nlassert(ret <= addQt);

	setStackSize(allowedQt);

	return ret;
}

uint32 CGameItem::getMaxStackSize() const
{
	nlassert(_Form != NULL);
	return _Form->getMaxStackSize();
}


//-----------------------------------------------
// getFreeSlotCount :
//
//-----------------------------------------------
//sint16 CGameItem::getFreeSlotCount() const
//{
//	if( SlotCount == 0 )
//	{
//		return 0;
//	}
//
//	sint16 usedSlots = 0;
//	vector<CGameItemPtr>::const_iterator it;
//	for( it = _Children.begin(); it != _Children.end(); ++it )
//	{
//		if( *it != NULL )
//		{
//			++usedSlots;
//		}
//	}
//	return _Children.size() - usedSlots;
//
//} // getFreeSlotCount //

//-----------------------------------------------
// getUsedSlotCount :
//
//-----------------------------------------------
//sint16 CGameItem::getUsedSlotCount() const
//{
//	sint16 usedSlots = 0;
//	vector<CGameItemPtr >::const_iterator it;
//	for( it = _Children.begin(); it != _Children.end(); ++it )
//	{
//		if( *it != NULL )
//		{
//			++usedSlots;
//		}
//	}
//	return usedSlots;
//
//} // getUsedSlotCount //

//-----------------------------------------------
// addChildItem :
//
//-----------------------------------------------
//bool CGameItem::addChild( CGameItemPtr  item, sint16 slot)
//{
//	// check item is valid
//	if (item == NULL)
//	{
//		nlwarning("<CGameItem::addChild> Attempt of adding a NULL item");
//		return false;
//	}
//	// check if slot index is not too high
//	if( slot >= SlotCount )
//	{
//		nlwarning("<CGameItem::addChild> Attempt of adding an item in slot %d but slot count is %d",slot,SlotCount);
//		return false;
//	}
//	// if autoequip
//	if( slot == -1 )
//	{
//		slot = 0;
//		vector<CGameItemPtr >::iterator it = _Children.begin();
//		while( it != _Children.end() && *it != NULL  )
//		{
//			++it;
//			++slot;
//		}
//		if( it == _Children.end() )
//		{
//			//(*item).Loc.Slot = (uint8)slot;
//			_Children.push_back( item );
//		}
//		else
//		{
//			//(*item).Loc.Slot = (uint8)slot;
//			_Children[slot] = item;
//		}
////		nldebug("<CGameItem::addChild> adding item %s to slot %d of the item %s",(*item).getId().toString().c_str(),slot,_Id.toString().c_str());
//	}
//	else
//	{
//		if( slot < 0 )
//		{
//			nlwarning("<CGameItem::addChild> Attempt of adding an item in a negative slot : %d",slot);
//			return false;
//		}
//		if( slot >= (sint16)_Children.size() )
//		{
//			_Children.resize( slot + 1, NULL );
//		}
//		if( _Children[slot] != NULL )
//		{
//			nlwarning("<CGameItem::addChild> slot %d is not empty",slot);
//			return false;
//		}
//		_Children[slot] = item;
//		//(*item).Loc.Slot = (uint8)slot;
//		//(*item).attachTo( this, (uint8)slot );
//	}
//	
//	item->_Parent = this;
//	item->Loc.Slot = (uint8) slot;
//	item->Owner = getId();
//	return true;
//} // addChild //



//-----------------------------------------------
// detachChild :
//
//-----------------------------------------------
//CGameItemPtr  CGameItem::detachChild( sint16 slot )
//{
//	sint childrenSize = _Children.size();
//	if( slot == -1 )
//	{
//		slot = childrenSize - 1;
//	}
//
//	if( slot < 0 || slot >= childrenSize )
//	{
//		nlwarning("<CGameItem::detachChild> can't detach item in slot %d, only %d slots allocated",slot,childrenSize);
//		return NULL;
//	}
//	
//	CGameItemPtr  detachedItem = _Children[slot];
//	if ( detachedItem == NULL)
//	{
//		nlwarning("<CGameItem::detachChild> Found child NULL in slot %u for item %s", slot, _SheetId.toString().c_str() );
//	}
//	else
//	{
////#if !FINAL_VERSION
////		nlassert( slot == (*detachedItem).Loc.Slot);
////#else
//		if( slot != (*detachedItem)->Loc.Slot)
//		{
//			nlwarning("Incoherency found in items Loc.Slot");
//			reorganizeChildren();
//			detachedItem = _Children[slot];
//		}
////#endif
//
//		(*detachedItem)->Owner = CEntityId::Unknown;
//		(*detachedItem)->setParent(NULL);
//		(*detachedItem)->Loc.Slot = 0;
//		//detachedItem->detachFromParent();
//		_Children[slot] = NULL;
//	}
//
//	// do not allow 'holes' in stack, so move item and resize stack size
//	if( _SheetId == CSheetId("stack.sitem") )
//	{
//		// get last item if any
//		CGameItemPtr lastItem = _Children.back();
//		if (lastItem != NULL)
//		{
//			// place last item in stack in newly created hole
//			lastItem->Loc.Slot = (uint8) slot;
//			_Children[ slot ] = lastItem;
//		}
//
//		// remove last item in stack
//		_Children.pop_back();
//	}
//	return detachedItem;
//} // detachChild //



//-----------------------------------------------
// detachChildren :
// BUGGED !!
//-----------------------------------------------
/*void CGameItem::detachChildren()
{
	vector<CGameItemPtr >::iterator it;
	for( it = _Children.begin(); it != _Children.end(); ++it )
	{
		if ( (*it) != NULL )
		{
			(**it).detachFromParent(); 
			(*it) = NULL;
		}
	}
} // detachChildren //
*/

//-----------------------------------------------
// moveChild :
//
//-----------------------------------------------
//void CGameItem::moveChild( uint8 from, uint8 to )
//{
//	if( from >= _Children.size() )
//	{
//		nlwarning("<CGameItem::moveChild> the source slot is not valid : %d, there are %d slot allocated",from,_Children.size());
//		return;
//	}
//	if( to >= SlotCount )
//	{
//		nlwarning("<CGameItem::moveChild> Attempt of moving an item in slot %d but slot count is %d",to,SlotCount);
//		return;
//	}
//
//	if( to >= _Children.size() )
//	{
//		_Children.resize( to + 1 );
//	}
//
//	_Children[to] = _Children[from];
//	_Children[from] = NULL;
//	(*_Children[to])->Loc.Slot = to;
//
//} // moveChild //


//-----------------------------------------------
// reorganizeChildren :
//
//-----------------------------------------------
//void CGameItem::reorganizeChildren()
//{
//	static CSheetId stackSheet("stack.sitem");
//
//	// only allow this for stacks
//	if ( _SheetId != stackSheet)
//		return;
//
//	nlassert(false);
//
//	uint nullItemsCount = 0;
//	const uint nbItems = _Children.size();
//
//	for ( uint i = 0 ; i + nullItemsCount < nbItems ; )
//	{
//		if ( _Children[i] == NULL)
//		{
//			++nullItemsCount;
//			if ( i + nullItemsCount == nbItems)
//				break;
//			else
//			{
//				_Children[i] = _Children[nbItems - nullItemsCount];
//				if (_Children[i] != NULL)
//					_Children[i]->Loc.Slot = i;
//			}
//		}
//		else
//		{
//			_Children[i]->Loc.Slot = i;
//			++i;
//		}
//	}
//
//	// remove the NULL elements which are now all at the end of the vector
//	if ( nullItemsCount > 0)
//		_Children.resize( nbItems - nullItemsCount);
//} // reorganizeChildren //


//-----------------------------------------------
// detachFromParent :
//
//-----------------------------------------------
//void CGameItem::detachFromParent() 
//{
//	if (_Parent == NULL) return;
//
//	nlassert( _Parent->getChildren()[Loc.Slot] == this);
//
//	(*_Parent)->detachChild(Loc.Slot);
//	_Parent = NULL; 
//	Loc.Slot = 0; 
//}

//-----------------------------------------------
// attachTo :
//
//-----------------------------------------------
//void CGameItem::attachTo(CGameItemPtr item, uint8 slot) 
//{
//	// if the item is still attached, detach it from its parent
//	if ( _Parent != NULL )
//	{
//		if ( item != NULL )
//			nlwarning("<attachTo> ATTACHING AN ITEM (%s) STILL ATTACHED TO ANOTHER PARENT (%s) (new parent %s)",_SheetId.toString().c_str(), (*_Parent)->getSheetId().toString().c_str(), (*item)->getSheetId().toString().c_str() );
//		(*_Parent)->detachChild( Loc.Slot );
//	}
//
//	if ( item != NULL )
//	{
//		item->addChild( this,slot );
//	}
//}

//-----------------------------------------------
// getSlotImage :
//
//-----------------------------------------------
//bool CGameItem::getSlotImage(bool & inHand, uint16 & slot) const
//{
//	if ( _SlotImage == 0xFFFF )
//		return false;
//	if ( _SlotImage < 2 )
//	{
//		inHand = true;
//		slot = _SlotImage;
//		return  true;
//	}
//	inHand = false;
//	slot = _SlotImage - 2;
//	return true;
//}

//-----------------------------------------------
// setSlotImage :
//
//-----------------------------------------------
//void CGameItem::setSlotImage(bool inHand, uint16 slot)
//{
//	if ( inHand )
//		_SlotImage = slot;
//	else
//		_SlotImage = slot + 2;
//}

//-----------------------------------------------
// resetSlotImage :
//
//-----------------------------------------------
//void CGameItem::resetSlotImage()
//{
//	_SlotImage = 0xFFFF;
//}

//-----------------------------------------------
// getLocSlot :
//
//-----------------------------------------------
//uint8 CGameItem::getLocSlot()
//{
//	if (_Parent!=NULL)
//	{
////		nlassert(_Parent->getChildren().size()>Loc.Slot);
//		nlassert(_Parent->_Children.size() > Loc.Slot);
////		nlassert(_Parent->getChildren()[Loc.Slot]==this);
//		nlassert(_Parent->_Children[Loc.Slot] == this);
//	}
//	return Loc.Slot;
//}


//uint32 CGameItem::getInventorySlot()
//{
//	if (_Inventory == NULL)
//		return CInventory::INVALID_INVENTORY_SLOT;
//
//	return Loc.Slot;
//}

//-----------------------------------------------
// getItem :
//
//-----------------------------------------------
CGameItem * CGameItem::getItem(uint idx)
{	
	BOMB_IF( idx>=_Items.size(), "Attempt to access an item beyond end of item vector", return 0 );
	BOMB_IF( _Items[idx]._AllocatorData>=0, NLMISC::toString("Attempt to access an item that is not allocated or has been freed (idx: %d)",idx), return 0 );
	return &_Items[idx];
}

//-----------------------------------------------
// newItem :
//
//-----------------------------------------------
CGameItem *CGameItem::newItem()
{
	// NOTE
	// the following assert is very important as the rest of this algorithm depends on this condition being met
	nlassert(_FirstFreeItem<=_Items.size());

	// see whether there are any deleted items awaiting re-allocation
	if (_FirstFreeItem==_Items.size())
	{
		// no unallocated items found so allocate a bit more RAM
//			uint sizeIncrement=65536; //16; //65536;	// use 16 for testing - likely 64k for exploitation
//			egs_giinfo("Increased item vector size to %d items",(_Items.size()+1)*sizeIncrement);
//			for (uint i=0;i<sizeIncrement;++i)
//				_Items[_FirstFreeItem+i]._AllocatorData=_FirstFreeItem+i+1;
		_Items.extend();
	}

	// get the first free item
	uint32 idx=_FirstFreeItem;
	_FirstFreeItem=_Items[idx]._AllocatorData;

	#ifdef ITEM_DEBUG
		// make sure the new _FirstFreeItem is valid
		nlassert(_FirstFreeItem<=_Items.size());
		// make sure this record is not already allocated
		nlassert(_Items[idx]._AllocatorData>=0);
		// overwrite the _AllocatorData with a unique(ish) negative value
		if (--_NextAllocatorDataValue>=0)
			_NextAllocatorDataValue=-1;
		_Items[idx]._AllocatorData=_NextAllocatorDataValue;

		#ifdef NL_DEBUG
			// make sure the linked list of free blocks is intact
			uint i,j,k;
			for(i=0, j=0; i<_Items.size(); ++i)
				if (_Items[i]._AllocatorData>=0)
					++j;
			for(i=_FirstFreeItem, k=0; i<_Items.size(); i=_Items[i]._AllocatorData)
				++k;
			nlassert(j==k);
		#endif

		// TODO: replace this egs_giinfo() with a LOG()
//			egs_giinfo("newItem(): %5d (free: %d/%d)",idx,j,_Items.size());
	#endif

	return &_Items[idx];
}

//-----------------------------------------------
// deleteItem :
//
//-----------------------------------------------
void CGameItem::deleteItem(CGameItem *item)
{

	uint32 idx=_Items.getUniqueIndex(*item);
	BOMB_IF( idx == ~0u, "Delete item failed because getIndex failed", return);
	BOMB_IF( item->_Inventory != NULL, "Item still in an inventory while deleting", item->_Inventory->removeItem(item->getInventorySlot()););
	BOMB_IF( item->_RefInventory != NULL, "Item still in a ref inventory while deleting", item->_RefInventory->removeItem(item->getRefInventorySlot()););

	log_Item_Delete(item->getItemId(), item->getSheetId(), item->getStackSize(), item->quality());

	// this test would only have value if the implementation of std::vector<> doesn't guarantee a 
	// continuous memory address space for the vector's data
	// It's not in the #IFDEF DEBUG because it may only be triggered in very obscure conditions
	nlassert(&_Items[idx]==item);

	#ifdef ITEM_DEBUG
		// make sure idx is within the _Items vector
		nlassert(idx<_Items.size());
		// make sure the item hasn't already been liberated

		nlassert(_Items[idx]._AllocatorData<0);

		#ifdef NL_DEBUG
			// make sure the linked list of free blocks is intact
			uint i,j,k;
			for(i=0, j=0; i<_Items.size(); ++i)
				if (_Items[i]._AllocatorData>=0)
					++j;
			for(i=_FirstFreeItem, k=0; i<_Items.size() && k<_Items.size(); i=_Items[i]._AllocatorData)
				++k;
			nlassert(j==k);
		#endif
			
		// TODO: replace this egs_giinfo() with a LOG()
//			egs_giinfo("deleteItem(): %5d (free: %d/%d)",idx,j,_Items.size());
	#endif

	_Items[idx]._AllocatorData=_FirstFreeItem;
	_FirstFreeItem=idx;
}

//void CGameItem::save( NLMISC::IStream &f )
//{
//	//NLMEMORY::CheckHeap (true);
//	
//	if ( _IsOnTheGround )
//			nlwarning("<CGameItem load> saving an object on the ground should never happen sheet: %s, id: %s ",_SheetId.toString().c_str(), _Id.toString().c_str() );
//	// save item infos
//	f.serial( _SlotImage );
//	f.serial( _SheetId );
//	f.serial( Owner );
//	if( Owner != CEntityId::Unknown )
//	{
//		f.serial(Loc.Slot);
//	}
//	else
//	{
//		f.serial( Loc.Pos.X );
//		f.serial( Loc.Pos.Y );
//		f.serial( Loc.Pos.Z );
//	}
//
//	//NLMEMORY::CheckHeap (true);
//
//	f.serial( TimeOnTheGround );
//	f.serial( SlotCount );
//	f.serial( _CarrionSheetId );
//	f.serial( _Destroyable );
//	f.serial( _Dropable);
//
//	f.serial( _CreatorId );
//
//	f.serial( _HP );
//	f.serial( _Recommended );
//	f.serialPtr( _CraftParameters );
//	f.serial( _LostHPremains );
//
//	//NLMEMORY::CheckHeap (true);
//	
//	// serial rm used for craft item
////	f.serialCont( _RmUsedForCraft );
//	std::vector<CSheetId> RmUsedCraft;
//	f.serialCont( RmUsedCraft );
//
//	//NLMEMORY::CheckHeap (true);
//	
//	// serial position in bag inventory
//	f.serial(_ClientInventoryPosition);
//
//	
//	f.serial( _SapLoad );
//	f.serialCont( _Enchantment );
//
//	//NLMEMORY::CheckHeap (true);
//	
//	// save children count
//	uint32 childrenCount = _Children.size(); 
//	f.serial( childrenCount );
//
//	// save non null children count
//	uint32 nonNullChildrenCount = 0;
//	uint32 i;
//	for( i = 0; i < childrenCount; ++i )
//	{
//		if( _Children[i]!=NULL && _Children[i] != this ) 
//			++nonNullChildrenCount;
//	}
//	f.serial( nonNullChildrenCount );
//
//	//NLMEMORY::CheckHeap (true);
//
//	string sheetName = _SheetId.toString();
//	
//	// for each non null child
//	for( i = 0; i < childrenCount; ++i )
//	{
//		if( _Children[i]!=NULL && _Children[i] != this )
//		{
//			// save child index
//			f.serial( i );
//
//			// save child
//			(*_Children[i])->save( f );
//
//			if (getSheetId() == CSheetId("stack.sitem"))
//				break;
//
//		}
//	}
//	//NLMEMORY::CheckHeap (true);
//} // save //


//-----------------------------------------------
// load :
//
//-----------------------------------------------
//void CGameItem::legacyLoad( NLMISC::IStream &f, uint16 characterSerialVersion, CCharacter *ownerPtr )
//{
//	_LatencyEndDate = 0;
//	uint16 slotImage;
//	NLMISC::CEntityId owner;
//	uint8 slot = 0;
//	sint32 coord = 0;
//	NLMISC::TGameCycle timeOnGround = 0;
//	sint16 slotCount = 0;
//	uint32 carrionSheetId = 0;
//
//	//NLMEMORY::CheckHeap (true);
////	try
////	{
//		if ( characterSerialVersion >= 32 )
//		{
////			f.serial(_SlotImage);
//			f.serial(slotImage);
//		}
//
//		if( characterSerialVersion >= 17 )
//		{
//			// load item infos
//			f.serial( _SheetId );
//
//
///***/		string sheetString = _SheetId.toString();
//
////			f.serial( Owner );
//			f.serial(owner);
////			if( Owner != CEntityId::Unknown )
//			if( owner != CEntityId::Unknown )
//			{
////				f.serial(Loc.Slot);
//				f.serial(slot);
//			}
//			else
//			{
////				f.serial( Loc.Pos.X );
////				f.serial( Loc.Pos.Y );
////				f.serial( Loc.Pos.Z );
//				f.serial(coord);
//				f.serial(coord);
//				f.serial(coord);
//			}
//			
//			//NLMEMORY::CheckHeap (true);
//
////			f.serial( TimeOnTheGround );
//			f.serial(timeOnGround);
////			f.serial( SlotCount );
//			f.serial(slotCount);
////			f.serial( _CarrionSheetId );
//			f.serial(carrionSheetId);
//
//			f.serial( _Destroyable );
//			if ( characterSerialVersion >= 19 )
//				f.serial(_Dropable);
//			
//			f.serial( _CreatorId );
//			
//			f.serial( _HP );
//			f.serial( _Recommended );
//
//			if (_CraftParameters == NULL)
//				_CraftParameters = new CItemCraftParameters();
//			f.serial( *_CraftParameters );
//
//			if ( characterSerialVersion >= 55 )
//				f.serial(_LostHPremains);
//
//			//NLMEMORY::CheckHeap (true);
//			
//			// serial rm used for craft item
////			f.serialCont( _RmUsedForCraft );
//			std::vector<CSheetId> RmUsedCraft;
//			f.serialCont( RmUsedCraft );
//
//			// ANTIBUG : huge rm vector serialized !!!!
////			if ( f.isReading() && _RmUsedForCraft.size() >= 1000 )
////				_RmUsedForCraft.clear();
//			
//			//NLMEMORY::CheckHeap (true);
//
//			f.serial(_ClientInventoryPosition);
//
//			if( characterSerialVersion >= 20 )
//			{
//				f.serial( _SapLoad );
//				f.serialCont( _Enchantment );
//			}
//
//			// load children count
//			uint32 childrenCount = 0; 
//			f.serial( childrenCount );
////			
////			// load non null children count
//			uint32 nonNullChildrenCount = 0;
//			f.serial( nonNullChildrenCount );
////
////			//NLMEMORY::CheckHeap (true);
////			
//			bool isStack = false;
//			if ( _SheetId == CSheetId("stack.sitem") )
//			{
//				// stack do not have NULL items
//				isStack = true;
//				if ( nonNullChildrenCount != childrenCount )
//				{
////					nlwarning("<CGameItem::load>char %s a stack contains NULL items! children: '%d' non null children: '%d'", Idc.toString().c_str(),childrenCount,nonNullChildrenCount);
//					nlwarning("<CGameItem::load> A stack contains NULL items! children: '%d' non null children: '%d'", 
//						childrenCount,
//						nonNullChildrenCount);
//				}
//			}
//			else
//			{
//				nlassertex(childrenCount == 0, ("Can't read old inventory item as item, must be read as inventory"));
////				_Children.resize( childrenCount, NULL );
//			}
////			
////			//NLMEMORY::CheckHeap (true);
////
//			uint32 i;
//			for( i = 0; i < nonNullChildrenCount; ++i )
//			{
////				// load child index
////				//NLMEMORY::CheckHeap (true);
////
//				uint32 index;
//				f.serial( index );
////				
////				//NLMEMORY::CheckHeap (true);
////
////				// load child
//				CGameItemPtr  item;
//				item.newItem();
////
////				//NLMEMORY::CheckHeap (true);
////				CEntityId id( RYZOMID::object, GameItemManager.getFreeItemIndice() );
////				item->_Id = id;
////				//NLMEMORY::CheckHeap (true);
//				if( item!=NULL )
//				{
////					//NLMEMORY::CheckHeap (true);
////					(*item)->load(f, Idc, characterSerialVersion);
//					item->legacyLoad(f, characterSerialVersion, ownerPtr);
////					//NLMEMORY::CheckHeap (true);
//				}
//				else
//				{
////					nlwarning("<CGameItem::load>char %s Can't allocate child item %d",Idc.toString().c_str(),i);
//					nlwarning("<CGameItem::load>Can't allocate child item %d", i);
//					return;
//				}
//
//				if (i == 0)
//				{
//					// replace the current item with the first stacked item
//					copyItem(item);
//				}
////				
//				// delete this garbage item
//				item.deleteItem();
//
////				//NLMEMORY::CheckHeap (true);
////
////				// add child in children
////				if (isStack)
////					_Children.push_back(item);
////				else
////				{
////					if( index == ((uint32)-1) || (index >= _Children.size()) )
////					{
////						nlwarning("<CGameItem::load> load children item with bad index = %d (item number slots %d ), skip it...", index, _Children.size() );
////						
////						for( index = 0; index < _Children.size(); ++index )
////						{
////							if(_Children[index] == 0 ) break;
////						}
////					}
////					if( index < _Children.size() )
////					{
////						_Children[index] = item;
////					}
////					else
////					{
////						nlwarning("<CGameItem::load> load children item with bad index = %d (item number slots %d ), skip it...", index, _Children.size() );
////					}
////				}
////				
////				//NLMEMORY::CheckHeap (true);
////
////				// set the parent
////				(*item)->setParent( this );
////				
////				//NLMEMORY::CheckHeap (true);
////
////				// add item in manager
////				GameItemManager.insertItem( item );
////
////
////				if (getSheetId() == CSheetId("stack.sitem") && characterSerialVersion>=43 )
////				{
////					for (uint32 j=1;j<nonNullChildrenCount;++j)
////					{
////						CGameItemPtr copy =	_Children[0]->getCopy();
////						if ( copy != NULL )
////						{
////							//_Children.push_back( copy );
////							addChild(copy);
////						}
////					}
////					break;
////				}
////
////				//NLMEMORY::CheckHeap (true);
//			}
//
//			// Set the stack size
//			if (nonNullChildrenCount == 0)
//				nonNullChildrenCount = 1; // ANTI-BUG: repair empty stacks... is it a good solution?
//
//			setStackSize(nonNullChildrenCount);
//
///*			// not sure about version number
//			if( characterSerialVersion < 17 && characterSerialVersion > 13 )
//			{
//				f.serial(_ClientInventoryPosition);
//			}
//			//NLMEMORY::CheckHeap (true);
//*/		}
//		else // load old item format
//		{
//			//NLMEMORY::CheckHeap (true);
//
//			float dummy_f;
//			uint16 dummy_uint16;
//			uint32 dummy_uint32;
//			bool dummy_bool;
//
//			//NLMEMORY::CheckHeap (true);
//
//			// load item infos
//			f.serial( _SheetId );
////			f.serial( Owner );
//			f.serial( owner );
////			if( Owner != CEntityId::Unknown )
//			if( owner != CEntityId::Unknown )
//			{
////				f.serial(Loc.Slot);
//				f.serial(slot);
//			}
//			else
//			{
////				f.serial( Loc.Pos.X );
////				f.serial( Loc.Pos.Y );
////				f.serial( Loc.Pos.Z );
//				f.serial(coord);
//				f.serial(coord);
//				f.serial(coord);
//			}
//
//			//NLMEMORY::CheckHeap (true);
//
//			if( characterSerialVersion >= 8 )
//			{
//				f.serial( dummy_f );
//				f.serial( dummy_f );
//				f.serial( dummy_f );
//			}
//
//			//NLMEMORY::CheckHeap (true);
//
//			f.serial( dummy_uint16 ); //uint16
//			_Recommended = dummy_uint16;
//
//			f.serial( _HP ); //uint32
//		
//			if( characterSerialVersion < 7 )
//			{
//				f.serial( dummy_uint16 );
//			}
//			else if( characterSerialVersion == 7 )
//			{
//				f.serial( dummy_f );
//			}
//			
//			//NLMEMORY::CheckHeap (true);
//
//			if( characterSerialVersion < 8 )
//			{
//				f.serial( dummy_f );
//			}
//			f.serial( dummy_f );
//
//			//NLMEMORY::CheckHeap (true);
//
//			f.serial( dummy_uint16 );
//			f.serial( dummy_uint16 );
//			
//			//NLMEMORY::CheckHeap (true);
//
//			if( characterSerialVersion < 8 )
//			{
//				f.serial( dummy_f );
//			}
//			f.serial( dummy_uint16 );
//			
//			f.serial( dummy_uint32 );
//			f.serial( dummy_uint16 );
//			f.serial( dummy_uint32 );
//			f.serial( dummy_uint16 );
//			f.serial( dummy_uint16 );
//			f.serial( dummy_bool );
//
//			//NLMEMORY::CheckHeap (true);
//
//			vector< SProtection > dummy_protection;
//			f.serialCont(dummy_protection);
//			
//			//NLMEMORY::CheckHeap (true);
//
//			f.serial( timeOnGround );
//			f.serial( slotCount );
//			uint8 col;
//			f.serial( col );
//			if( _CraftParameters )
//				_CraftParameters->Color = col;
//
//			f.serial( carrionSheetId );
//
//			f.serial( _Destroyable );
//
//			f.serial( _CreatorId );
//
//			// load children count
//			uint32 childrenCount = 0; 
//			f.serial( childrenCount );
//
//			// load non null children count
//			uint32 nonNullChildrenCount = 0;
//			f.serial( nonNullChildrenCount );
//
//			//NLMEMORY::CheckHeap (true);
//
//			bool isStack = false;
//			if ( _SheetId == CSheetId("stack.sitem") )
//			{
////				// stack do not have NULL items
//				isStack = true;
//				if ( nonNullChildrenCount != childrenCount )
//				{
////					nlwarning("<CGameItem::load>char %s a stack contains NULL items! children: '%d' non null children: '%d'",Idc.toString().c_str(),childrenCount,nonNullChildrenCount);
//					nlwarning("<CGameItem::load> A stack contains NULL items! children: '%d' non null children: '%d'",
//						childrenCount,
//						nonNullChildrenCount);
//				}
//			}
//			else
//			{
//				nlassertex(childrenCount == 0, ("Can't read old inventory item as item, must be read as inventory"));
////				_Children.resize( childrenCount, NULL );
//			}
//
//			//NLMEMORY::CheckHeap (true);
//
//			uint32 i;
//			for( i = 0; i < nonNullChildrenCount; ++i )
//			{
//				// load child index
//				uint32 index;
//				f.serial( index );
////
////				//NLMEMORY::CheckHeap (true);
////
//				// load child
//				CGameItemPtr  item;
////
////				//NLMEMORY::CheckHeap (true);
////
//				item.newItem();
////				
////				//NLMEMORY::CheckHeap (true);
////				
////				CEntityId id( RYZOMID::object, GameItemManager.getFreeItemIndice() );
////				
////				item->_Id = id;
////				
////				//NLMEMORY::CheckHeap (true);
//				if( item!=NULL )
//				{
//					//NLMEMORY::CheckHeap (true);
////					item->load(f, Idc, characterSerialVersion);
//					item->legacyLoad(f, characterSerialVersion, ownerPtr);
////					//NLMEMORY::CheckHeap (true);
//				}
//				else
//				{
////					nlwarning("<CGameItem::load>char %s Can't allocate child item %d",Idc.toString().c_str(),i);
//					nlwarning("<CGameItem::load> Can't allocate child item %d", i);
//					return;
//				}
////
////				//NLMEMORY::CheckHeap (true);
////
//				if (i==0)
//				{
//					// copy first item into current item
//					copyItem(item);
//				}
//
//				item.deleteItem();
////				// add child in children
////				if (isStack)
////					_Children.push_back(item);
////				else
////					_Children[index] = item;
////
////				//NLMEMORY::CheckHeap (true);
////
////				// set the parent
////				(*item)->setParent( this );
////
////				//NLMEMORY::CheckHeap (true);
////
////				// add item in manager
////				GameItemManager.insertItem( item );
////
////				if (getSheetId() == CSheetId("stack.sitem") && characterSerialVersion>=43 )
////				{
////					for (uint32 j=1;j<nonNullChildrenCount;++j)
////					{
////						CGameItemPtr copy =	_Children[0]->getCopy();
////						if ( copy != NULL )
////							_Children.push_back( copy );
////					}
////					break;
////				}
////				
////			
////				//NLMEMORY::CheckHeap (true);
//			}
//
//			if (nonNullChildrenCount == 0)
//				nonNullChildrenCount = 1; // ANTI-BUG: repair empty stacks... is it a good solution?
//
//			setStackSize(nonNullChildrenCount);
//
//			// the client position has been added with Character version 4, for older item, just init it wiht -1
//			// we do this to keep compatibility with previous character backup version
//			if (characterSerialVersion >= 4)
//				f.serial(_ClientInventoryPosition);
//			else
//				_ClientInventoryPosition = -1;
//
//			//NLMEMORY::CheckHeap (true);
//
//			// serialize raw material used for craft item
//			if( characterSerialVersion >= 10 )
//			{
////				f.serialCont( _RmUsedForCraft );
//				std::vector<CSheetId> RmUsedCraft;
//				f.serialCont( RmUsedCraft );
//
////				if ( f.isReading() && _RmUsedForCraft.size() >= 20 )
////					_RmUsedForCraft.clear();
//			}
//			//NLMEMORY::CheckHeap (true);
//		}
////	}
////	catch(const Exception &e)
////	{
////		nlwarning("<CGameItem::load> %s",e.what());
////	}
//
//	// init form
//	_Form = CSheets::getForm( _SheetId );
//
//	// patch tool HP for old saves
//	if ( characterSerialVersion < 68 && _Form)
//	{
//		if ( _Form->Family == ITEMFAMILY::CRAFTING_TOOL	|| _Form->Family == ITEMFAMILY::HARVEST_TOOL )
//			_HP = maxDurability();
//	}
//
//
//	// memory optimization for non craftable items
//	if (_Form != NULL)
//	{
//		switch(_Form->Family) 
//		{
//			//craftable families, do nothing
//		case ITEMFAMILY::ARMOR:
//		case ITEMFAMILY::MELEE_WEAPON:
//		case ITEMFAMILY::RANGE_WEAPON:
//		case ITEMFAMILY::AMMO:
//		case ITEMFAMILY::SHIELD:
//		case ITEMFAMILY::JEWELRY:
//		case ITEMFAMILY::CRAFTING_TOOL:
//		case ITEMFAMILY::HARVEST_TOOL:
//			break;
//			// non craftable-> release craft parameters structure
//		default:
//			if (_CraftParameters != NULL)
//			{
//				delete _CraftParameters;
//				_CraftParameters = NULL;
//			}
//		}
//	}
//
//	// compute item worn state
//	computeItemWornState();
//
//	// adapt the old slotimage method to new ref method and auto equip item if needed
//	if ( f.isReading() && slotImage != 0xFFFF && ownerPtr != NULL)
//	{
//		INVENTORIES::TInventory refInventoryId;
//		if ( slotImage < 2 )
//		{
//			refInventoryId = INVENTORIES::handling;
//			_RefInventorySlot = slotImage;
//			nlinfo("Convert item %s, had slot image %u, is now in HAND, slot %u", _SheetId.toString().c_str(),slotImage,_RefInventorySlot);
//		}
//		else
//		{
//			refInventoryId = INVENTORIES::equipment;
//			_RefInventorySlot = slotImage - 2;
//			nlinfo("Convert item %s, had slot image %u, is now in EQUIPMENT, slot %u", _SheetId.toString().c_str(),slotImage,_RefInventorySlot);
//		}
//
//		const uint32 slot = _RefInventorySlot;
//		_RefInventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//		const CInventoryPtr inv = ownerPtr->getInventory(refInventoryId);
//		if (inv)
//		{
//			CGameItemPtr itemPtr(this);
//			inv->insertItem(itemPtr, slot);
//		}
//		else
//			_RefInventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//			
//		//ownerPtr->equipCharacter(refInventoryId, slot, getInventorySlot());
//	}
//	else
//		_RefInventorySlot = CInventoryBase::INVALID_INVENTORY_SLOT;
//
////	if ( _IsOnTheGround )
////		nlwarning("<CGameItem load> loading an object on the ground should never happen sheet: %s, id: %s ",_SheetId.toString().c_str(), _ItemId.toString().c_str() );
//
//	//NLMEMORY::CheckHeap (true);
//} // load //


//-----------------------------------------------
// dumpGameItemStats :
//
//-----------------------------------------------
void CGameItem::dumpGameItemStats( const string& fileName )
{
	if( !fileName.empty() )
	{
		FILE * f;
		f = fopen(fileName.c_str(),"w");
		
		if(f)
		{
//			fprintf(f,"ItemID   : %s\n", _ItemId.toString().c_str());
			fprintf(f,"SheetId : %d\n", _SheetId.asInt());
			fprintf(f,"Loc  : TODO\n");
			fprintf(f,"Recommended  : %d\n", _Recommended);
			fprintf(f,"Hp  : %d\n", _HP);

			if( _CraftParameters )
			{
				fprintf(f,"DamageFactor: %f", _CraftParameters->Dmg );
				fprintf(f,"SpeedFactor: %f", _CraftParameters->Speed );
				fprintf(f,"RangeFactor: %f", _CraftParameters->Range );
			}

			fclose(f);
		}
		else
		{
			nlwarning("(EGS)<CGameItem::dumpGameItemStats> Can't open the file %s",fileName.c_str());
		}
	}
	else
	{

	}	
} // dumpGameItemStats //


//-----------------------------------------------
// removeHp :
//-----------------------------------------------
uint32 CGameItem::removeHp( double hpLost ) 
{
	if (_Form != NULL && _Form->Shield != NULL && _Form->Shield->Unbreakable)
		return 0;

	uint32 hp = (uint32)fabs(hpLost);
	_LostHPremains += (float)hpLost - float(hp);
	if (_LostHPremains >= 1.0f)
	{
		--_LostHPremains;
		++hp;
	}

	if (!hp)
		return 0;

	if( hp >= _HP)
	{
		_HP = 1;
	
//		// if states differs send a message if owner is a player
//		if (_CurrentWornState != ITEM_WORN_STATE::Worned)
//		{
//			_CurrentWornState = ITEM_WORN_STATE::Worned;
//
//			// only send message for non armor and non shield items
//			if (_Form && !ITEMFAMILY::destroyedWhenWorned(_Form->Family) )
//			{
//				const CEntityId itemOwner = getOwnerPlayer();
//				if (itemOwner != CEntityId::Unknown)
//				{
//					SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
//					params[0].SheetId = _SheetId;
//					PHRASE_UTILITIES::sendDynamicSystemMessage( TheDataset.getDataSetRow(itemOwner), ITEM_WORN_STATE::getMessageForState(ITEM_WORN_STATE::Worned), params);
//				}
//			}
// 		}

		getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_hp));

		return (hp-_HP);
	}

	_HP -= hp;

	// get new worn state
//	ITEM_WORN_STATE::TItemWornState wornState = _CurrentWornState;
//	computeItemWornState();

//	// if states differs send a message if owner is a player
//	if (wornState != _CurrentWornState)
//	{
//		const CEntityId itemOwner = getOwnerPlayer();
//		if (itemOwner != CEntityId::Unknown)
//		{
//			string msgName = ITEM_WORN_STATE::getMessageForState(_CurrentWornState);
//			if ( !msgName.empty())
//			{
//				SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
//				params[0].SheetId = _SheetId;
//				PHRASE_UTILITIES::sendDynamicSystemMessage( TheDataset.getDataSetRow(itemOwner), msgName, params);
//			}
//		}
//	}

	getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_hp));

	return hp;
} // removeHp //

//-----------------------------------------------
// add hp to item
//-----------------------------------------------
void CGameItem::addHp( double hpGain )
{
	uint32 hp = (uint32)fabs(hpGain);
	_LostHPremains -= (float)hpGain - float(hp);

	_HP += hp;
	const uint32 max = maxDurability();
	if ( _HP >= max )
		_HP = max;

	// get new worn state
	ITEM_WORN_STATE::TItemWornState wornState = _CurrentWornState;
	computeItemWornState();
	
//	// if states differs send a message if owner is a player
//	if (wornState != _CurrentWornState)
//	{
//		const CEntityId itemOwner = getOwnerPlayer();
//		if (itemOwner != CEntityId::Unknown)
//		{
//			string msgName = ITEM_WORN_STATE::getMessageForState(_CurrentWornState);
//			if ( !msgName.empty())
//			{
//				SM_STATIC_PARAMS_1(params, STRING_MANAGER::item);
//				params[0].SheetId = _SheetId;
//				PHRASE_UTILITIES::sendDynamicSystemMessage( TheDataset.getDataSetRow(itemOwner), msgName, params);
//			}
//		}
//	}
	
	getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_hp));

} // addHp //

//-----------------------------------------------
// changes the custom text of an item
//-----------------------------------------------
void CGameItem::setCustomText(const ucstring &val)
{
	_CustomText = val;
//	getInventory()->onItemChanged(getInventorySlot(), INVENTORIES::TItemChangeFlags(INVENTORIES::itc_custom_text));
}

void CGameItem::setInventory(const CInventoryPtr &inv, uint32 slot)
{
	nlassert((_Inventory == NULL && inv != NULL) 
			|| (_Inventory != NULL && inv == NULL));
	nlassert((_InventorySlot == INVENTORIES::INVALID_INVENTORY_SLOT && _Inventory == NULL) 
			||(_InventorySlot != INVENTORIES::INVALID_INVENTORY_SLOT && _Inventory != NULL));
	nlassert((slot == INVENTORIES::INVALID_INVENTORY_SLOT && inv == NULL) 
			||(slot != INVENTORIES::INVALID_INVENTORY_SLOT && inv != NULL));
	nlassert(inv == NULL || (slot < inv->getSlotCount()));

	_Inventory = inv;
	_InventorySlot = slot;
}

/// set link information between item and reference inventory (used by CRefInventory)
void CGameItem::setRefInventory(const CInventoryPtr &inv, uint32 slot)
{
	nlassert((_RefInventory == NULL && inv != NULL) 
		|| (_RefInventory != NULL && inv == NULL));
	nlassert((_RefInventorySlot == INVENTORIES::INVALID_INVENTORY_SLOT && _RefInventory == NULL) 
		||(_RefInventorySlot != INVENTORIES::INVALID_INVENTORY_SLOT && _RefInventory != NULL));
	nlassert((slot == INVENTORIES::INVALID_INVENTORY_SLOT && inv == NULL) 
		||(slot != INVENTORIES::INVALID_INVENTORY_SLOT && inv != NULL));
	nlassert(inv == NULL || (slot < inv->getSlotCount()));
	
	_RefInventory = inv;
	_RefInventorySlot = slot;
}


//-----------------------------------------------
// get stat energy of item
//-----------------------------------------------
float CGameItem::getStatEnergy() 
{ 
	if (_Form == NULL)
		return 0.0f;

	switch (_Form->Family)
	{
		case ITEMFAMILY::ARMOR:
		case ITEMFAMILY::MELEE_WEAPON:
		case ITEMFAMILY::RANGE_WEAPON:
		case ITEMFAMILY::AMMO:
		case ITEMFAMILY::SHIELD:
		case ITEMFAMILY::JEWELRY:
			break;
		case ITEMFAMILY::RAW_MATERIAL:
			return _Form->Mp->StatEnergy;
		default:
			return 0.0f;
	}
	if( _CraftParameters == 0 )
	{
		return 0.0f;
	}
	else
	{
		if( _CraftParameters->StatEnergy == 0.0f ) 
			return estimateStatEnergy(); 
		else 
			return _CraftParameters->StatEnergy; 
	}
}


//-----------------------------------------------
// get class of item
//-----------------------------------------------
RM_CLASS_TYPE::TRMClassType CGameItem::getItemClass()
{
	return RM_CLASS_TYPE::getItemClass((uint32)(100.0f * getStatEnergy()));
}

//-----------------------------------------------
// estimate stat energy of an item ( used for item crafted before stat energy are used)
//-----------------------------------------------
float CGameItem::estimateStatEnergy()
{
	float statEnergy = 0.0f;
	
	if( _CraftParameters )
	{
		statEnergy += _CraftParameters->Durability;
		statEnergy += _CraftParameters->Weight;
		statEnergy += _CraftParameters->SapLoad;
		statEnergy += _CraftParameters->Dmg;
		statEnergy += _CraftParameters->Speed;
		statEnergy += _CraftParameters->Range;
		statEnergy += _CraftParameters->DodgeModifier;
		statEnergy += _CraftParameters->ParryModifier;
		statEnergy += _CraftParameters->AdversaryDodgeModifier;
		statEnergy += _CraftParameters->AdversaryParryModifier;
		statEnergy += _CraftParameters->ProtectionFactor;
		statEnergy += _CraftParameters->MaxSlashingProtection;
		statEnergy += _CraftParameters->MaxBluntProtection;
		statEnergy += _CraftParameters->MaxPiercingProtection;
		statEnergy += _CraftParameters->ElementalCastingTimeFactor;
		statEnergy += _CraftParameters->ElementalPowerFactor;
		statEnergy += _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		statEnergy += _CraftParameters->OffensiveAfflictionPowerFactor;
		statEnergy += _CraftParameters->HealCastingTimeFactor;
		statEnergy += _CraftParameters->HealPowerFactor;
		statEnergy += _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		statEnergy += _CraftParameters->DefensiveAfflictionPowerFactor;
		return ( _CraftParameters->StatEnergy = statEnergy / 22.0f );
	}
	return 0.0f;
}


//-----------------------------------------------
// return max durability (= max hp) of item
//-----------------------------------------------
uint32 CGameItem::maxDurability() const
{
	if (_Form == NULL)
		return 0;

	// craftable items
//	if (_CraftParameters != NULL)
	{
		float d = 0.0f;
		switch( _Form->Type )
		{
			// melee weapons
			case ITEM_TYPE::DAGGER:			d = CWeaponCraftParameters::DaggerDurability;		break;				
			case ITEM_TYPE::SWORD:			d = CWeaponCraftParameters::SwordDurability;		break;
			case ITEM_TYPE::MACE:			d = CWeaponCraftParameters::MaceDurability;			break;
			case ITEM_TYPE::AXE:			d = CWeaponCraftParameters::AxeDurability;			break;
			case ITEM_TYPE::SPEAR:			d = CWeaponCraftParameters::SpearDurability;		break;
			case ITEM_TYPE::STAFF:			d = CWeaponCraftParameters::StaffDurability;		break;
			case ITEM_TYPE::MAGICIAN_STAFF: d = CWeaponCraftParameters::MagicianStaffDurability;break;
			case ITEM_TYPE::TWO_HAND_SWORD: d = CWeaponCraftParameters::TwoHandSwordDurability; break;
			case ITEM_TYPE::TWO_HAND_AXE:	d = CWeaponCraftParameters::TwoHandAxeDurability;	break;
			case ITEM_TYPE::PIKE:			d = CWeaponCraftParameters::PikeDurability;			break;
			case ITEM_TYPE::TWO_HAND_MACE:	d = CWeaponCraftParameters::TwoHandMaceDurability;	break;
		
			// range weapon
			case ITEM_TYPE::AUTOLAUCH:	d = CWeaponCraftParameters::AutolauchDurability;	break;
			case ITEM_TYPE::BOWRIFLE:	d = CWeaponCraftParameters::BowrifleDurability;		break;
			case ITEM_TYPE::LAUNCHER:	d = CWeaponCraftParameters::LauncherDurability;		break;
			case ITEM_TYPE::PISTOL:		d = CWeaponCraftParameters::PistolDurability;		break;
			case ITEM_TYPE::BOWPISTOL:	d = CWeaponCraftParameters::BowpistolDurability;	break;
			case ITEM_TYPE::RIFLE:		d = CWeaponCraftParameters::RifleDurability;		break;
		
			// ammo
			case ITEM_TYPE::AUTOLAUNCH_AMMO:d = CWeaponCraftParameters::AutolaunchAmmoDurability;	break;
			case ITEM_TYPE::BOWRIFLE_AMMO:	d = CWeaponCraftParameters::BowrifleAmmoDurability;		break;
			case ITEM_TYPE::LAUNCHER_AMMO:	d = CWeaponCraftParameters::LauncherAmmoDurability;		break;
			case ITEM_TYPE::PISTOL_AMMO:	d = CWeaponCraftParameters::PistolAmmoDurability;		break;
			case ITEM_TYPE::BOWPISTOL_AMMO: d = CWeaponCraftParameters::BowpistolAmmoDurability;	break;
			case ITEM_TYPE::RIFLE_AMMO:		d = CWeaponCraftParameters::RifleAmmoDurability;		break;
		
			// armor and shield
			case ITEM_TYPE::SHIELD:			d = CWeaponCraftParameters::ShieldDurability;		break;
			case ITEM_TYPE::BUCKLER:		d = CWeaponCraftParameters::BucklerDurability;		break;
			case ITEM_TYPE::LIGHT_BOOTS:	d = CWeaponCraftParameters::LightBootsDurability;	break;
			case ITEM_TYPE::LIGHT_GLOVES:	d = CWeaponCraftParameters::LightGlovesDurability;	break;
			case ITEM_TYPE::LIGHT_PANTS:	d = CWeaponCraftParameters::LightPantsDurability;	break;
			case ITEM_TYPE::LIGHT_SLEEVES:	d = CWeaponCraftParameters::LightSleevesDurability; break;
			case ITEM_TYPE::LIGHT_VEST:		d = CWeaponCraftParameters::LightVestDurability;	break;
			case ITEM_TYPE::MEDIUM_BOOTS:	d = CWeaponCraftParameters::MediumBootsDurability;	break;
			case ITEM_TYPE::MEDIUM_GLOVES:	d = CWeaponCraftParameters::MediumGlovesDurability; break;
			case ITEM_TYPE::MEDIUM_PANTS:	d = CWeaponCraftParameters::MediumPantsDurability;	break;
			case ITEM_TYPE::MEDIUM_SLEEVES:	d = CWeaponCraftParameters::MediumSleevesDurability;break;
			case ITEM_TYPE::MEDIUM_VEST:	d = CWeaponCraftParameters::MediumVestDurability;	break;
			case ITEM_TYPE::HEAVY_BOOTS:	d = CWeaponCraftParameters::HeavyBootsDurability;	break;
			case ITEM_TYPE::HEAVY_GLOVES:	d = CWeaponCraftParameters::HeavyGlovesDurability;	break;
			case ITEM_TYPE::HEAVY_PANTS:	d = CWeaponCraftParameters::HeavyPantsDurability;	break;
			case ITEM_TYPE::HEAVY_SLEEVES:	d = CWeaponCraftParameters::HeavySleevesDurability; break;
			case ITEM_TYPE::HEAVY_VEST:		d = CWeaponCraftParameters::HeavyVestDurability;	break;
			case ITEM_TYPE::HEAVY_HELMET:	d = CWeaponCraftParameters::HeavyHelmetDurability;	break;
		
			// jewel
			case ITEM_TYPE::ANKLET:		d = CWeaponCraftParameters::AnkletDurability;	break;
			case ITEM_TYPE::BRACELET:	d = CWeaponCraftParameters::BraceletDurability; break;
			case ITEM_TYPE::DIADEM:		d = CWeaponCraftParameters::DiademDurability;	break;
			case ITEM_TYPE::EARING:		d = CWeaponCraftParameters::EaringDurability;	break;
			case ITEM_TYPE::PENDANT:	d = CWeaponCraftParameters::PendantDurability;	break;
			case ITEM_TYPE::RING:		d = CWeaponCraftParameters::RingDurability;		break;

			// tools
			// SHEARS = pick for forage
			case ITEM_TYPE::SHEARS:			return (uint32)CWeaponCraftParameters::ForageToolDurability;
			case ITEM_TYPE::AmmoTool:		return (uint32)CWeaponCraftParameters::AmmoCraftingToolDurability;
			case ITEM_TYPE::ArmorTool:		return (uint32)CWeaponCraftParameters::ArmorCraftingToolDurability;
			case ITEM_TYPE::JewelryTool:	return (uint32)CWeaponCraftParameters::JewelryCraftingToolDurability;
			case ITEM_TYPE::MeleeWeaponTool:return (uint32)CWeaponCraftParameters::MeleeWeaponCraftingToolDurability;
			case ITEM_TYPE::RangeWeaponTool:return (uint32)CWeaponCraftParameters::RangeWeaponCraftingToolDurability;
			case ITEM_TYPE::ToolMaker:		return (uint32)CWeaponCraftParameters::ToolCraftingToolDurability;

			default:
				return 0;
		}
		
		if (_CraftParameters != NULL)
			return (uint32) ( d + d * _CraftParameters->Durability );
		else
			return (uint32) d;

	}
	/*else // non craftable items -> tools
	{
		// tool
		switch( _Form->Type )
		{
			// SHEARS = pick for forage
			case ITEM_TYPE::SHEARS:			return (uint32)CWeaponCraftParameters::ForageToolDurability;
			case ITEM_TYPE::AmmoTool:		return (uint32)CWeaponCraftParameters::AmmoCraftingToolDurability;
			case ITEM_TYPE::ArmorTool:		return (uint32)CWeaponCraftParameters::ArmorCraftingToolDurability;
			case ITEM_TYPE::JewelryTool:	return (uint32)CWeaponCraftParameters::JewelryCraftingToolDurability;
			case ITEM_TYPE::MeleeWeaponTool:return (uint32)CWeaponCraftParameters::MeleeWeaponCraftingToolDurability;
			case ITEM_TYPE::RangeWeaponTool:return (uint32)CWeaponCraftParameters::RangeWeaponCraftingToolDurability;
			
			default:
				return 0;
		}
	}
	*/
//	return 0;
}


//-----------------------------------------------
// return weight
//-----------------------------------------------
uint32 CGameItem::weight() const
{
	float weight = 0.0f;
	if( _Form == 0 )
		return 0;

	if( _CraftParameters != NULL )
	{
		if ( _Form->Family == ITEMFAMILY::MISSION_ITEM )
		{
			// Crafted missions items have no crafted weight but still a _CraftParameters struct
			return  _Form->Weight;
		}
		else
		{
			weight = _Form->getBaseWeight();
			weight = 2.0f * weight - weight * _CraftParameters->Weight;
			return uint32(max(0.f, weight * 1000.f));
		}
	}
	else if( _Form != NULL )
	{
		switch( _Form->Family )
		{
			case ITEMFAMILY::RAW_MATERIAL:
			case ITEMFAMILY::CRAFTING_TOOL:
			case ITEMFAMILY::HARVEST_TOOL:
			case ITEMFAMILY::MISSION_ITEM:
			case ITEMFAMILY::GENERIC_ITEM:
			case ITEMFAMILY::ITEM_SAP_RECHARGE:
				return _Form->Weight;
			default:
				return 0;
		}
	}
	else
		return 0;
}

//-----------------------------------------------
// return Weight (Total for stack) of item * quantity
//-----------------------------------------------
uint32 CGameItem::getStackWeight(uint32 limitQuantity) const
{
	return weight() * min(_StackSize, limitQuantity);
}

//-----------------------------------------------
// return Bulk of item * quantity
//-----------------------------------------------
uint32 CGameItem::getStackBulk(uint32 limitQuantity) const
{
	if (_Form)
		return _Form->Bulk * min(_StackSize, limitQuantity);

	return 0;
}

//-----------------------------------------------
// return maxSapLoad
//-----------------------------------------------
uint32 CGameItem::maxSapLoad() const
{
	if( _Form == 0)
		return 0;

	if( _CraftParameters != 0 )
	{
		float s = 0.0f;
		float m = 0.0f;
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:		s = CWeaponCraftParameters::DaggerSapLoad;
									m = CWeaponCraftParameters::DaggerSapLoadMax; break;
		case ITEM_TYPE::SWORD:	s = CWeaponCraftParameters::SwordSapLoad;
								m = CWeaponCraftParameters::SwordSapLoadMax; break;
		case ITEM_TYPE::MACE:		s = CWeaponCraftParameters::MaceSapLoad;
									m = CWeaponCraftParameters::MaceSapLoadMax; break;
		case ITEM_TYPE::AXE:	s = CWeaponCraftParameters::AxeSapLoad;
								m = CWeaponCraftParameters::AxeSapLoadMax; break;
		case ITEM_TYPE::SPEAR:		s = CWeaponCraftParameters::SpearSapLoad;
									m = CWeaponCraftParameters::SpearSapLoadMax; break;
		case ITEM_TYPE::STAFF:	s = CWeaponCraftParameters::StaffSapLoad;
								m = CWeaponCraftParameters::StaffSapLoadMax; break;
		case ITEM_TYPE::MAGICIAN_STAFF: s = CWeaponCraftParameters::MagicianStaffSapLoad;
										m = CWeaponCraftParameters::MagicianStaffSapLoadMax; break;
		case ITEM_TYPE::TWO_HAND_SWORD:		s = CWeaponCraftParameters::TwoHandSwordSapLoad;
											m = CWeaponCraftParameters::TwoHandSwordSapLoadMax; break;
		case ITEM_TYPE::TWO_HAND_AXE:	s = CWeaponCraftParameters::TwoHandAxeSapLoad;
										m = CWeaponCraftParameters::TwoHandAxeSapLoadMax; break;
		case ITEM_TYPE::PIKE:	s = CWeaponCraftParameters::PikeSapLoad;
								m = CWeaponCraftParameters::PikeSapLoadMax; break;
		case ITEM_TYPE::TWO_HAND_MACE:	s = CWeaponCraftParameters::TwoHandMaceSapLoad;
										m = CWeaponCraftParameters::TwoHandMaceSapLoadMax; break;
		
		// range weapon
		case ITEM_TYPE::AUTOLAUCH:	s = CWeaponCraftParameters::AutolauchSapLoad;
									m = CWeaponCraftParameters::AutolauchSapLoadMax; break;
		case ITEM_TYPE::BOWRIFLE:		s = CWeaponCraftParameters::BowrifleSapLoad;
										m = CWeaponCraftParameters::BowrifleSapLoadMax; break;
		case ITEM_TYPE::LAUNCHER:	s = CWeaponCraftParameters::LauncherSapLoad;
									m = CWeaponCraftParameters::LauncherSapLoadMax; break;
		case ITEM_TYPE::PISTOL: s = CWeaponCraftParameters::PistolSapLoad;
								m = CWeaponCraftParameters::PistolSapLoadMax; break;
		case ITEM_TYPE::BOWPISTOL:	s = CWeaponCraftParameters::BowpistolSapLoad;
									m = CWeaponCraftParameters::BowpistolSapLoadMax; break;
		case ITEM_TYPE::RIFLE:	s = CWeaponCraftParameters::RifleSapLoad;
								m = CWeaponCraftParameters::RifleSapLoadMax; break;
		
		// ammo
		case ITEM_TYPE::AUTOLAUNCH_AMMO:	s = CWeaponCraftParameters::AutolaunchAmmoSapLoad;
											m = CWeaponCraftParameters::AutolaunchAmmoSapLoadMax; break;
		case ITEM_TYPE::BOWRIFLE_AMMO:	s = CWeaponCraftParameters::BowrifleAmmoSapLoad;
										m = CWeaponCraftParameters::BowrifleAmmoSapLoadMax; break;
		case ITEM_TYPE::LAUNCHER_AMMO:		s = CWeaponCraftParameters::LauncherAmmoSapLoad;
											m = CWeaponCraftParameters::LauncherAmmoSapLoadMax; break;
		case ITEM_TYPE::PISTOL_AMMO:	s = CWeaponCraftParameters::PistolAmmoSapLoad;
										m = CWeaponCraftParameters::PistolAmmoSapLoadMax; break;
		case ITEM_TYPE::BOWPISTOL_AMMO:		s = CWeaponCraftParameters::BowpistolAmmoSapLoad;
											m = CWeaponCraftParameters::BowpistolAmmoSapLoadMax; break;
		case ITEM_TYPE::RIFLE_AMMO:	s = CWeaponCraftParameters::RifleAmmoSapLoad;
									m = CWeaponCraftParameters::RifleAmmoSapLoadMax; break;
		
		// armor and shield
		case ITEM_TYPE::SHIELD:	s = CWeaponCraftParameters::ShieldSapLoad;
								m = CWeaponCraftParameters::ShieldSapLoadMax; break;
		case ITEM_TYPE::BUCKLER:	s = CWeaponCraftParameters::BucklerSapLoad;
									m = CWeaponCraftParameters::BucklerSapLoadMax; break;
		case ITEM_TYPE::LIGHT_BOOTS:	s = CWeaponCraftParameters::LightBootsSapLoad;
										m = CWeaponCraftParameters::LightBootsSapLoadMax; break;
		case ITEM_TYPE::LIGHT_GLOVES:		s = CWeaponCraftParameters::LightGlovesSapLoad;
											m = CWeaponCraftParameters::LightGlovesSapLoadMax; break;
		case ITEM_TYPE::LIGHT_PANTS:	s = CWeaponCraftParameters::LightPantsSapLoad;
										m = CWeaponCraftParameters::LightPantsSapLoadMax; break;
		case ITEM_TYPE::LIGHT_SLEEVES:		s = CWeaponCraftParameters::LightSleevesSapLoad;
											m = CWeaponCraftParameters::LightSleevesSapLoadMax; break;
		case ITEM_TYPE::LIGHT_VEST:		s = CWeaponCraftParameters::LightVestSapLoad;
										m = CWeaponCraftParameters::LightVestSapLoadMax; break;
		case ITEM_TYPE::MEDIUM_BOOTS:		s = CWeaponCraftParameters::MediumBootsSapLoad;
											m = CWeaponCraftParameters::MediumBootsSapLoadMax; break;
		case ITEM_TYPE::MEDIUM_GLOVES:	s = CWeaponCraftParameters::MediumGlovesSapLoad;
										m = CWeaponCraftParameters::MediumGlovesSapLoadMax; break;
		case ITEM_TYPE::MEDIUM_PANTS:		s = CWeaponCraftParameters::MediumPantsSapLoad;
											m = CWeaponCraftParameters::MediumPantsSapLoadMax; break;
		case ITEM_TYPE::MEDIUM_SLEEVES: s = CWeaponCraftParameters::MediumSleevesSapLoad;
										m = CWeaponCraftParameters::MediumSleevesSapLoadMax; break;
		case ITEM_TYPE::MEDIUM_VEST:		s = CWeaponCraftParameters::MediumVestSapLoad;
											m = CWeaponCraftParameters::MediumVestSapLoadMax; break;
		case ITEM_TYPE::HEAVY_BOOTS:	s = CWeaponCraftParameters::HeavyBootsSapLoad;
										m = CWeaponCraftParameters::HeavyBootsSapLoadMax; break;
		case ITEM_TYPE::HEAVY_GLOVES:		s = CWeaponCraftParameters::HeavyGlovesSapLoad;
											m = CWeaponCraftParameters::HeavyGlovesSapLoadMax; break;
		case ITEM_TYPE::HEAVY_PANTS:	s = CWeaponCraftParameters::HeavyPantsSapLoad;
										m = CWeaponCraftParameters::HeavyPantsSapLoadMax; break;
		case ITEM_TYPE::HEAVY_SLEEVES:		s = CWeaponCraftParameters::HeavySleevesSapLoad;
											m = CWeaponCraftParameters::HeavySleevesSapLoadMax; break;
		case ITEM_TYPE::HEAVY_VEST:		s = CWeaponCraftParameters::HeavyVestSapLoad;
										m = CWeaponCraftParameters::HeavyVestSapLoadMax; break;
		case ITEM_TYPE::HEAVY_HELMET:		s = CWeaponCraftParameters::HeavyHelmetSapLoad;
											m = CWeaponCraftParameters::HeavyHelmetSapLoadMax; break;
		
		// jewel
		case ITEM_TYPE::ANKLET:		s = CWeaponCraftParameters::AnkletSapLoad;
									m = CWeaponCraftParameters::AnkletSapLoadMax; break;
		case ITEM_TYPE::BRACELET:		s = CWeaponCraftParameters::BraceletSapLoad;
										m = CWeaponCraftParameters::BraceletSapLoadMax; break;
		case ITEM_TYPE::DIADEM:		s = CWeaponCraftParameters::DiademSapLoad;
									m = CWeaponCraftParameters::DiademSapLoadMax; break;
		case ITEM_TYPE::EARING: s = CWeaponCraftParameters::EaringSapLoad;
								m = CWeaponCraftParameters::EaringSapLoadMax; break;
		case ITEM_TYPE::PENDANT:	s = CWeaponCraftParameters::PendantSapLoad;
									m = CWeaponCraftParameters::PendantSapLoadMax; break;
		case ITEM_TYPE::RING:	s = CWeaponCraftParameters::RingSapLoad;
								m = CWeaponCraftParameters::RingSapLoadMax; break;
			default:
				return 0;
		}
		return (uint32) ( s + (m - s) * _CraftParameters->SapLoad );
	}
	return 0;
}


//-----------------------------------------------
// return damage factor
//-----------------------------------------------
float CGameItem::damageFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerDmg + (CWeaponCraftParameters::DaggerDmgMax - CWeaponCraftParameters::DaggerDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordDmg + (CWeaponCraftParameters::SwordDmgMax - CWeaponCraftParameters::SwordDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceDmg + (CWeaponCraftParameters::MaceDmgMax - CWeaponCraftParameters::MaceDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeDmg + (CWeaponCraftParameters::AxeDmgMax - CWeaponCraftParameters::AxeDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearDmg + (CWeaponCraftParameters::SpearDmgMax - CWeaponCraftParameters::SpearDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffDmg + (CWeaponCraftParameters::StaffDmgMax - CWeaponCraftParameters::StaffDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffDmg + (CWeaponCraftParameters::MagicianStaffDmgMax - CWeaponCraftParameters::MagicianStaffDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordDmg + (CWeaponCraftParameters::TwoHandSwordDmgMax - CWeaponCraftParameters::TwoHandSwordDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeDmg + (CWeaponCraftParameters::TwoHandAxeDmgMax - CWeaponCraftParameters::TwoHandAxeDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeDmg + (CWeaponCraftParameters::PikeDmgMax - CWeaponCraftParameters::PikeDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceDmg + (CWeaponCraftParameters::TwoHandMaceDmgMax - CWeaponCraftParameters::TwoHandMaceDmg) * _CraftParameters->Dmg;
			
			// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return CWeaponCraftParameters::AutolauchDmg + (CWeaponCraftParameters::AutolauchDmgMax - CWeaponCraftParameters::AutolauchDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::BOWRIFLE:
			return CWeaponCraftParameters::BowrifleDmg + (CWeaponCraftParameters::BowrifleDmgMax - CWeaponCraftParameters::BowrifleDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::LAUNCHER:
			return CWeaponCraftParameters::LauncherDmg + (CWeaponCraftParameters::LauncherDmgMax - CWeaponCraftParameters::LauncherDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::PISTOL:
			return CWeaponCraftParameters::PistolDmg + (CWeaponCraftParameters::PistolDmgMax - CWeaponCraftParameters::PistolDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::BOWPISTOL:
			return CWeaponCraftParameters::BowpistolDmg + (CWeaponCraftParameters::BowpistolDmgMax - CWeaponCraftParameters::BowpistolDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::RIFLE:
			return CWeaponCraftParameters::RifleDmg + (CWeaponCraftParameters::RifleDmgMax - CWeaponCraftParameters::RifleDmg) * _CraftParameters->Dmg;
			
			// ammo
		case ITEM_TYPE::AUTOLAUNCH_AMMO:
			return CWeaponCraftParameters::AutolaunchAmmoDmg + (CWeaponCraftParameters::AutolaunchAmmoDmgMax - CWeaponCraftParameters::AutolaunchAmmoDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::BOWRIFLE_AMMO:
			return CWeaponCraftParameters::BowrifleAmmoDmg + (CWeaponCraftParameters::BowrifleAmmoDmgMax - CWeaponCraftParameters::BowrifleAmmoDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::LAUNCHER_AMMO:
			return CWeaponCraftParameters::LauncherAmmoDmg + (CWeaponCraftParameters::LauncherAmmoDmgMax - CWeaponCraftParameters::LauncherAmmoDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::PISTOL_AMMO:
			return CWeaponCraftParameters::PistolAmmoDmg + (CWeaponCraftParameters::PistolAmmoDmgMax - CWeaponCraftParameters::PistolAmmoDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::BOWPISTOL_AMMO:
			return CWeaponCraftParameters::BowpistolAmmoDmg + (CWeaponCraftParameters::BowpistolAmmoDmgMax - CWeaponCraftParameters::BowpistolAmmoDmg) * _CraftParameters->Dmg;
		case ITEM_TYPE::RIFLE_AMMO:
			return CWeaponCraftParameters::RifleAmmoDmg + (CWeaponCraftParameters::RifleAmmoDmgMax - CWeaponCraftParameters::RifleAmmoDmg) * _CraftParameters->Dmg;
		default:
			;// dont warn : can be called for all weapons
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return damage/max damage
//-----------------------------------------------
void CGameItem::damage( sint32 skill, uint32& currentDamage, uint32& maxDamage ) const
{
	float dmgFactor;

	currentDamage = 0;
	maxDamage = 0;

	dmgFactor = damageFactor();
	if( dmgFactor != 0.0f )
	{
		currentDamage = (uint32) ( ( CWeaponDamageTable::getInstance().getRefenceDamage( _Recommended, skill ) * dmgFactor ) ); 
		maxDamage = (uint32) ( ( CWeaponDamageTable::getInstance().getRefenceDamage( _Recommended, _Recommended ) * dmgFactor ) );
	}
}


//-----------------------------------------------
// return speed
//-----------------------------------------------
float CGameItem::hitRate() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerHitRate + (CWeaponCraftParameters::DaggerHitRateMax - CWeaponCraftParameters::DaggerHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordHitRate + (CWeaponCraftParameters::SwordHitRateMax - CWeaponCraftParameters::SwordHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceHitRate + (CWeaponCraftParameters::MaceHitRateMax - CWeaponCraftParameters::MaceHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeHitRate + (CWeaponCraftParameters::AxeHitRateMax - CWeaponCraftParameters::AxeHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearHitRate + (CWeaponCraftParameters::SpearHitRateMax - CWeaponCraftParameters::SpearHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffHitRate + (CWeaponCraftParameters::StaffHitRateMax - CWeaponCraftParameters::StaffHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffHitRate + (CWeaponCraftParameters::MagicianStaffHitRateMax - CWeaponCraftParameters::MagicianStaffHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordHitRate + (CWeaponCraftParameters::TwoHandSwordHitRateMax - CWeaponCraftParameters::TwoHandSwordHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeHitRate + (CWeaponCraftParameters::TwoHandAxeHitRateMax - CWeaponCraftParameters::TwoHandAxeHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeHitRate + (CWeaponCraftParameters::PikeHitRateMax - CWeaponCraftParameters::PikeHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceHitRate + (CWeaponCraftParameters::TwoHandMaceHitRateMax - CWeaponCraftParameters::TwoHandMaceHitRate) * _CraftParameters->Speed;

		// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return CWeaponCraftParameters::AutolauchHitRate + (CWeaponCraftParameters::AutolauchHitRateMax - CWeaponCraftParameters::AutolauchHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::BOWRIFLE:
			return CWeaponCraftParameters::BowrifleHitRate + (CWeaponCraftParameters::BowrifleHitRateMax - CWeaponCraftParameters::BowrifleHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::LAUNCHER:
			return CWeaponCraftParameters::LauncherHitRate + (CWeaponCraftParameters::LauncherHitRateMax - CWeaponCraftParameters::LauncherHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::PISTOL:
			return CWeaponCraftParameters::PistolHitRate + (CWeaponCraftParameters::PistolHitRateMax - CWeaponCraftParameters::PistolHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::BOWPISTOL:
			return CWeaponCraftParameters::BowpistolHitRate + (CWeaponCraftParameters::BowpistolHitRateMax - CWeaponCraftParameters::BowpistolHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::RIFLE:
			return CWeaponCraftParameters::RifleHitRate + (CWeaponCraftParameters::RifleHitRateMax - CWeaponCraftParameters::RifleHitRate) * _CraftParameters->Speed;
		
		// ammo
		case ITEM_TYPE::AUTOLAUNCH_AMMO:
			return CWeaponCraftParameters::AutolaunchAmmoHitRate + (CWeaponCraftParameters::AutolaunchAmmoHitRateMax - CWeaponCraftParameters::AutolaunchAmmoHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::BOWRIFLE_AMMO:
			return CWeaponCraftParameters::BowrifleAmmoHitRate + (CWeaponCraftParameters::BowrifleAmmoHitRateMax - CWeaponCraftParameters::BowrifleAmmoHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::LAUNCHER_AMMO:
			return CWeaponCraftParameters::LauncherAmmoHitRate + (CWeaponCraftParameters::LauncherAmmoHitRateMax - CWeaponCraftParameters::LauncherAmmoHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::PISTOL_AMMO:
			return CWeaponCraftParameters::PistolAmmoHitRate + (CWeaponCraftParameters::PistolAmmoHitRateMax - CWeaponCraftParameters::PistolAmmoHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::BOWPISTOL_AMMO:
			return CWeaponCraftParameters::BowpistolAmmoHitRate + (CWeaponCraftParameters::BowpistolAmmoHitRateMax - CWeaponCraftParameters::BowpistolAmmoHitRate) * _CraftParameters->Speed;
		case ITEM_TYPE::RIFLE_AMMO:
			return CWeaponCraftParameters::RifleAmmoHitRate + (CWeaponCraftParameters::RifleAmmoHitRateMax - CWeaponCraftParameters::RifleAmmoHitRate) * _CraftParameters->Speed;
		default:;
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return range in meters
//-----------------------------------------------
float CGameItem::range() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
		// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return CWeaponCraftParameters::AutolauchRange + CWeaponCraftParameters::AutolauchRange * _CraftParameters->Range;
		case ITEM_TYPE::BOWRIFLE:
			return CWeaponCraftParameters::BowrifleRange + CWeaponCraftParameters::BowrifleRange * _CraftParameters->Range;
		case ITEM_TYPE::LAUNCHER:
			return CWeaponCraftParameters::LauncherRange + CWeaponCraftParameters::LauncherRange * _CraftParameters->Range;
		case ITEM_TYPE::PISTOL:
			return CWeaponCraftParameters::PistolRange + CWeaponCraftParameters::PistolRange * _CraftParameters->Range;
		case ITEM_TYPE::BOWPISTOL:
			return CWeaponCraftParameters::BowpistolRange + CWeaponCraftParameters::BowpistolRange * _CraftParameters->Range;
		case ITEM_TYPE::RIFLE:
			return CWeaponCraftParameters::RifleRange + CWeaponCraftParameters::RifleRange * _CraftParameters->Range;
		
		// ammo
		case ITEM_TYPE::AUTOLAUNCH_AMMO:
			return CWeaponCraftParameters::AutolaunchAmmoRange + CWeaponCraftParameters::AutolaunchAmmoRange * _CraftParameters->Range;
		case ITEM_TYPE::BOWRIFLE_AMMO:
			return CWeaponCraftParameters::BowrifleAmmoRange + CWeaponCraftParameters::BowrifleAmmoRange * _CraftParameters->Range;
		case ITEM_TYPE::LAUNCHER_AMMO:
			return CWeaponCraftParameters::LauncherAmmoRange + CWeaponCraftParameters::LauncherAmmoRange * _CraftParameters->Range;
		case ITEM_TYPE::PISTOL_AMMO:
			return CWeaponCraftParameters::PistolAmmoRange + CWeaponCraftParameters::PistolAmmoRange * _CraftParameters->Range;
		case ITEM_TYPE::BOWPISTOL_AMMO:
			return CWeaponCraftParameters::BowpistolAmmoRange + CWeaponCraftParameters::BowpistolAmmoRange * _CraftParameters->Range;
		case ITEM_TYPE::RIFLE_AMMO:
			return CWeaponCraftParameters::RifleAmmoRange + CWeaponCraftParameters::RifleAmmoRange * _CraftParameters->Range;
		default:;
		}
	}
	return 0.0f;

}


//-----------------------------------------------
// return dodge modifier
//-----------------------------------------------
sint32 CGameItem::dodgeModifier() const
{
	if( _Form == 0 )
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return (sint32) ( CWeaponCraftParameters::DaggerDodgeMinModifier + (CWeaponCraftParameters::DaggerDodgeMaxModifier - CWeaponCraftParameters::DaggerDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::SWORD:
			return (sint32) ( CWeaponCraftParameters::SwordDodgeMinModifier + (CWeaponCraftParameters::SwordDodgeMaxModifier - CWeaponCraftParameters::SwordDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MACE:
			return (sint32) ( CWeaponCraftParameters::MaceDodgeMinModifier + (CWeaponCraftParameters::MaceDodgeMaxModifier - CWeaponCraftParameters::MaceDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::AXE:
			return (sint32) ( CWeaponCraftParameters::AxeDodgeMinModifier + (CWeaponCraftParameters::AxeDodgeMaxModifier - CWeaponCraftParameters::AxeDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::SPEAR:
			return (sint32) ( CWeaponCraftParameters::SpearDodgeMinModifier + (CWeaponCraftParameters::SpearDodgeMaxModifier - CWeaponCraftParameters::SpearDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::STAFF:
			return (sint32) ( CWeaponCraftParameters::StaffDodgeMinModifier + (CWeaponCraftParameters::StaffDodgeMaxModifier - CWeaponCraftParameters::StaffDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MAGICIAN_STAFF:
			return (sint32) ( CWeaponCraftParameters::MagicianStaffDodgeMinModifier + (CWeaponCraftParameters::MagicianStaffDodgeMaxModifier - CWeaponCraftParameters::MagicianStaffDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::TWO_HAND_SWORD:
			return (sint32) ( CWeaponCraftParameters::TwoHandSwordDodgeMinModifier + (CWeaponCraftParameters::TwoHandSwordDodgeMaxModifier - CWeaponCraftParameters::TwoHandSwordDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::TWO_HAND_AXE:
			return (sint32) ( CWeaponCraftParameters::TwoHandAxeDodgeMinModifier + (CWeaponCraftParameters::TwoHandAxeDodgeMaxModifier - CWeaponCraftParameters::TwoHandAxeDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::PIKE:
			return (sint32) ( CWeaponCraftParameters::PikeDodgeMinModifier + (CWeaponCraftParameters::PikeDodgeMaxModifier - CWeaponCraftParameters::PikeDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::TWO_HAND_MACE:
			return (sint32) ( CWeaponCraftParameters::TwoHandMaceDodgeMinModifier + (CWeaponCraftParameters::TwoHandMaceDodgeMaxModifier - CWeaponCraftParameters::TwoHandMaceDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		
		// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return (sint32) ( CWeaponCraftParameters::AutolauchDodgeMinModifier + (CWeaponCraftParameters::AutolauchDodgeMaxModifier - CWeaponCraftParameters::AutolauchDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::BOWRIFLE:
			return (sint32) ( CWeaponCraftParameters::BowrifleDodgeMinModifier + (CWeaponCraftParameters::BowrifleDodgeMaxModifier - CWeaponCraftParameters::BowrifleDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::LAUNCHER:
			return (sint32) ( CWeaponCraftParameters::LauncherDodgeMinModifier + (CWeaponCraftParameters::LauncherDodgeMaxModifier - CWeaponCraftParameters::LauncherDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::PISTOL:
			return (sint32) ( CWeaponCraftParameters::PistolDodgeMinModifier + (CWeaponCraftParameters::PistolDodgeMaxModifier - CWeaponCraftParameters::PistolDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::BOWPISTOL:
			return (sint32) ( CWeaponCraftParameters::BowpistolDodgeMinModifier + (CWeaponCraftParameters::BowpistolDodgeMaxModifier - CWeaponCraftParameters::BowpistolDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::RIFLE:
			return (sint32) ( CWeaponCraftParameters::RifleDodgeMinModifier + (CWeaponCraftParameters::RifleDodgeMaxModifier - CWeaponCraftParameters::RifleDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		
		// armor and shield
		case ITEM_TYPE::SHIELD:
			return (sint32) ( CWeaponCraftParameters::ShieldDodgeMinModifier + (CWeaponCraftParameters::ShieldDodgeMaxModifier - CWeaponCraftParameters::ShieldDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::BUCKLER:
			return (sint32) ( CWeaponCraftParameters::BucklerDodgeMinModifier + (CWeaponCraftParameters::BucklerDodgeMaxModifier - CWeaponCraftParameters::BucklerDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::LIGHT_BOOTS:
			return (sint32) ( CWeaponCraftParameters::LightBootsDodgeMinModifier + (CWeaponCraftParameters::LightBootsDodgeMaxModifier - CWeaponCraftParameters::LightBootsDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::LIGHT_GLOVES:
			return (sint32) ( CWeaponCraftParameters::LightGlovesDodgeMinModifier + (CWeaponCraftParameters::LightGlovesDodgeMaxModifier - CWeaponCraftParameters::LightGlovesDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::LIGHT_PANTS:
			return (sint32) ( CWeaponCraftParameters::LightPantsDodgeMinModifier + (CWeaponCraftParameters::LightPantsDodgeMaxModifier - CWeaponCraftParameters::LightPantsDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::LIGHT_SLEEVES:
			return (sint32) ( CWeaponCraftParameters::LightSleevesDodgeMinModifier + (CWeaponCraftParameters::LightSleevesDodgeMaxModifier - CWeaponCraftParameters::LightSleevesDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::LIGHT_VEST:
			return (sint32) ( CWeaponCraftParameters::LightVestDodgeMinModifier + (CWeaponCraftParameters::LightVestDodgeMaxModifier - CWeaponCraftParameters::LightVestDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MEDIUM_BOOTS:
			return (sint32) ( CWeaponCraftParameters::MediumBootsDodgeMinModifier + (CWeaponCraftParameters::MediumBootsDodgeMaxModifier - CWeaponCraftParameters::MediumBootsDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MEDIUM_GLOVES:
			return (sint32) ( CWeaponCraftParameters::MediumGlovesDodgeMinModifier + (CWeaponCraftParameters::MediumGlovesDodgeMaxModifier - CWeaponCraftParameters::MediumGlovesDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MEDIUM_PANTS:
			return (sint32) ( CWeaponCraftParameters::MediumPantsDodgeMinModifier + (CWeaponCraftParameters::MediumPantsDodgeMaxModifier - CWeaponCraftParameters::MediumPantsDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MEDIUM_SLEEVES:
			return (sint32) ( CWeaponCraftParameters::MediumSleevesDodgeMinModifier + (CWeaponCraftParameters::MediumSleevesDodgeMaxModifier - CWeaponCraftParameters::MediumSleevesDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::MEDIUM_VEST:
			return (sint32) ( CWeaponCraftParameters::MediumVestDodgeMinModifier + (CWeaponCraftParameters::MediumVestDodgeMaxModifier - CWeaponCraftParameters::MediumVestDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::HEAVY_BOOTS:
			return (sint32) ( CWeaponCraftParameters::HeavyBootsDodgeMinModifier + (CWeaponCraftParameters::HeavyBootsDodgeMaxModifier - CWeaponCraftParameters::HeavyBootsDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::HEAVY_GLOVES:
			return (sint32) ( CWeaponCraftParameters::HeavyGlovesDodgeMinModifier + (CWeaponCraftParameters::HeavyGlovesDodgeMaxModifier - CWeaponCraftParameters::HeavyGlovesDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::HEAVY_PANTS:
			return (sint32) ( CWeaponCraftParameters::HeavyPantsDodgeMinModifier + (CWeaponCraftParameters::HeavyPantsDodgeMaxModifier - CWeaponCraftParameters::HeavyPantsDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::HEAVY_SLEEVES:
			return (sint32) ( CWeaponCraftParameters::HeavySleevesDodgeMinModifier + (CWeaponCraftParameters::HeavySleevesDodgeMaxModifier - CWeaponCraftParameters::HeavySleevesDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::HEAVY_VEST:
			return (sint32) ( CWeaponCraftParameters::HeavyVestDodgeMinModifier + (CWeaponCraftParameters::HeavyVestDodgeMaxModifier - CWeaponCraftParameters::HeavyVestDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		case ITEM_TYPE::HEAVY_HELMET:
			return (sint32) ( CWeaponCraftParameters::HeavyHelmetDodgeMinModifier + (CWeaponCraftParameters::HeavyHelmetDodgeMaxModifier - CWeaponCraftParameters::HeavyHelmetDodgeMinModifier ) * _CraftParameters->DodgeModifier );
		default:;
		}
	}
	return 0;
}


//-----------------------------------------------
// return parry modifier
//-----------------------------------------------
sint32 CGameItem::parryModifier() const
{
	if( _Form == 0 )
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return (sint32) ( CWeaponCraftParameters::DaggerParryMinModifier + (CWeaponCraftParameters::DaggerParryMaxModifier - CWeaponCraftParameters::DaggerParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::SWORD:
			return (sint32) ( CWeaponCraftParameters::SwordParryMinModifier + (CWeaponCraftParameters::SwordParryMaxModifier - CWeaponCraftParameters::SwordParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MACE:
			return (sint32) ( CWeaponCraftParameters::MaceParryMinModifier + (CWeaponCraftParameters::MaceParryMaxModifier - CWeaponCraftParameters::MaceParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::AXE:
			return (sint32) ( CWeaponCraftParameters::AxeParryMinModifier + (CWeaponCraftParameters::AxeParryMaxModifier - CWeaponCraftParameters::AxeParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::SPEAR:
			return (sint32) ( CWeaponCraftParameters::SpearParryMinModifier + (CWeaponCraftParameters::SpearParryMaxModifier - CWeaponCraftParameters::SpearParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::STAFF:
			return (sint32) ( CWeaponCraftParameters::StaffParryMinModifier + (CWeaponCraftParameters::StaffParryMaxModifier - CWeaponCraftParameters::StaffParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MAGICIAN_STAFF:
			return (sint32) ( CWeaponCraftParameters::MagicianStaffParryMinModifier + (CWeaponCraftParameters::MagicianStaffParryMaxModifier - CWeaponCraftParameters::MagicianStaffParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::TWO_HAND_SWORD:
			return (sint32) ( CWeaponCraftParameters::TwoHandSwordParryMinModifier + (CWeaponCraftParameters::TwoHandSwordParryMaxModifier - CWeaponCraftParameters::TwoHandSwordParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::TWO_HAND_AXE:
			return (sint32) ( CWeaponCraftParameters::TwoHandAxeParryMinModifier + (CWeaponCraftParameters::TwoHandAxeParryMaxModifier - CWeaponCraftParameters::TwoHandAxeParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::PIKE:
			return (sint32) ( CWeaponCraftParameters::PikeParryMinModifier + (CWeaponCraftParameters::PikeParryMaxModifier - CWeaponCraftParameters::PikeParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::TWO_HAND_MACE:
			return (sint32) ( CWeaponCraftParameters::TwoHandMaceParryMinModifier + (CWeaponCraftParameters::TwoHandMaceParryMaxModifier - CWeaponCraftParameters::TwoHandMaceParryMinModifier ) * _CraftParameters->ParryModifier );
			
			// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return (sint32) ( CWeaponCraftParameters::AutolauchParryMinModifier + (CWeaponCraftParameters::AutolauchParryMaxModifier - CWeaponCraftParameters::AutolauchParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::BOWRIFLE:
			return (sint32) ( CWeaponCraftParameters::BowrifleParryMinModifier + (CWeaponCraftParameters::BowrifleParryMaxModifier - CWeaponCraftParameters::BowrifleParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::LAUNCHER:
			return (sint32) ( CWeaponCraftParameters::LauncherParryMinModifier + (CWeaponCraftParameters::LauncherParryMaxModifier - CWeaponCraftParameters::LauncherParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::PISTOL:
			return (sint32) ( CWeaponCraftParameters::PistolParryMinModifier + (CWeaponCraftParameters::PistolParryMaxModifier - CWeaponCraftParameters::PistolParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::BOWPISTOL:
			return (sint32) ( CWeaponCraftParameters::BowpistolParryMinModifier + (CWeaponCraftParameters::BowpistolParryMaxModifier - CWeaponCraftParameters::BowpistolParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::RIFLE:
			return (sint32) ( CWeaponCraftParameters::RifleParryMinModifier + (CWeaponCraftParameters::RifleParryMaxModifier - CWeaponCraftParameters::RifleParryMinModifier ) * _CraftParameters->ParryModifier );
			
			// armor and shield
		case ITEM_TYPE::SHIELD:
			return (sint32) ( CWeaponCraftParameters::ShieldParryMinModifier + (CWeaponCraftParameters::ShieldParryMaxModifier - CWeaponCraftParameters::ShieldParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::BUCKLER:
			return (sint32) ( CWeaponCraftParameters::BucklerParryMinModifier + (CWeaponCraftParameters::BucklerParryMaxModifier - CWeaponCraftParameters::BucklerParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::LIGHT_BOOTS:
			return (sint32) ( CWeaponCraftParameters::LightBootsParryMinModifier + (CWeaponCraftParameters::LightBootsParryMaxModifier - CWeaponCraftParameters::LightBootsParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::LIGHT_GLOVES:
			return (sint32) ( CWeaponCraftParameters::LightGlovesParryMinModifier + (CWeaponCraftParameters::LightGlovesParryMaxModifier - CWeaponCraftParameters::LightGlovesParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::LIGHT_PANTS:
			return (sint32) ( CWeaponCraftParameters::LightPantsParryMinModifier + (CWeaponCraftParameters::LightPantsParryMaxModifier - CWeaponCraftParameters::LightPantsParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::LIGHT_SLEEVES:
			return (sint32) ( CWeaponCraftParameters::LightSleevesParryMinModifier + (CWeaponCraftParameters::LightSleevesParryMaxModifier - CWeaponCraftParameters::LightSleevesParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::LIGHT_VEST:
			return (sint32) ( CWeaponCraftParameters::LightVestParryMinModifier + (CWeaponCraftParameters::LightVestParryMaxModifier - CWeaponCraftParameters::LightVestParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MEDIUM_BOOTS:
			return (sint32) ( CWeaponCraftParameters::MediumBootsParryMinModifier + (CWeaponCraftParameters::MediumBootsParryMaxModifier - CWeaponCraftParameters::MediumBootsParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MEDIUM_GLOVES:
			return (sint32) ( CWeaponCraftParameters::MediumGlovesParryMinModifier + (CWeaponCraftParameters::MediumGlovesParryMaxModifier - CWeaponCraftParameters::MediumGlovesParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MEDIUM_PANTS:
			return (sint32) ( CWeaponCraftParameters::MediumPantsParryMinModifier + (CWeaponCraftParameters::MediumPantsParryMaxModifier - CWeaponCraftParameters::MediumPantsParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MEDIUM_SLEEVES:
			return (sint32) ( CWeaponCraftParameters::MediumSleevesParryMinModifier + (CWeaponCraftParameters::MediumSleevesParryMaxModifier - CWeaponCraftParameters::MediumSleevesParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::MEDIUM_VEST:
			return (sint32) ( CWeaponCraftParameters::MediumVestParryMinModifier + (CWeaponCraftParameters::MediumVestParryMaxModifier - CWeaponCraftParameters::MediumVestParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::HEAVY_BOOTS:
			return (sint32) ( CWeaponCraftParameters::HeavyBootsParryMinModifier + (CWeaponCraftParameters::HeavyBootsParryMaxModifier - CWeaponCraftParameters::HeavyBootsParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::HEAVY_GLOVES:
			return (sint32) ( CWeaponCraftParameters::HeavyGlovesParryMinModifier + (CWeaponCraftParameters::HeavyGlovesParryMaxModifier - CWeaponCraftParameters::HeavyGlovesParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::HEAVY_PANTS:
			return (sint32) ( CWeaponCraftParameters::HeavyPantsParryMinModifier + (CWeaponCraftParameters::HeavyPantsParryMaxModifier - CWeaponCraftParameters::HeavyPantsParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::HEAVY_SLEEVES:
			return (sint32) ( CWeaponCraftParameters::HeavySleevesParryMinModifier + (CWeaponCraftParameters::HeavySleevesParryMaxModifier - CWeaponCraftParameters::HeavySleevesParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::HEAVY_VEST:
			return (sint32) ( CWeaponCraftParameters::HeavyVestParryMinModifier + (CWeaponCraftParameters::HeavyVestParryMaxModifier - CWeaponCraftParameters::HeavyVestParryMinModifier ) * _CraftParameters->ParryModifier );
		case ITEM_TYPE::HEAVY_HELMET:
			return (sint32) ( CWeaponCraftParameters::HeavyHelmetParryMinModifier + (CWeaponCraftParameters::HeavyHelmetParryMaxModifier - CWeaponCraftParameters::HeavyHelmetParryMinModifier ) * _CraftParameters->ParryModifier );
		default:;
		}
	}
	return 0;
}

//-----------------------------------------------
// return adversary dodge modifier
//-----------------------------------------------
sint32 CGameItem::adversaryDodgeModifier() const
{
	if( _Form == 0 )
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return (sint32) ( CWeaponCraftParameters::DaggerAdversaryDodgeMinModifier + (CWeaponCraftParameters::DaggerAdversaryDodgeMaxModifier - CWeaponCraftParameters::DaggerAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::SWORD:
			return (sint32) ( CWeaponCraftParameters::SwordAdversaryDodgeMinModifier + (CWeaponCraftParameters::SwordAdversaryDodgeMaxModifier - CWeaponCraftParameters::SwordAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::MACE:
			return (sint32) ( CWeaponCraftParameters::MaceAdversaryDodgeMinModifier + (CWeaponCraftParameters::MaceAdversaryDodgeMaxModifier - CWeaponCraftParameters::MaceAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::AXE:
			return (sint32) ( CWeaponCraftParameters::AxeAdversaryDodgeMinModifier + (CWeaponCraftParameters::AxeAdversaryDodgeMaxModifier - CWeaponCraftParameters::AxeAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::SPEAR:
			return (sint32) ( CWeaponCraftParameters::SpearAdversaryDodgeMinModifier + (CWeaponCraftParameters::SpearAdversaryDodgeMaxModifier - CWeaponCraftParameters::SpearAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::STAFF:
			return (sint32) ( CWeaponCraftParameters::StaffAdversaryDodgeMinModifier + (CWeaponCraftParameters::StaffAdversaryDodgeMaxModifier - CWeaponCraftParameters::StaffAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::MAGICIAN_STAFF:
			return (sint32) ( CWeaponCraftParameters::MagicianStaffAdversaryDodgeMinModifier + (CWeaponCraftParameters::MagicianStaffAdversaryDodgeMaxModifier - CWeaponCraftParameters::MagicianStaffAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::TWO_HAND_SWORD:
			return (sint32) ( CWeaponCraftParameters::TwoHandSwordAdversaryDodgeMinModifier + (CWeaponCraftParameters::TwoHandSwordAdversaryDodgeMaxModifier - CWeaponCraftParameters::TwoHandSwordAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::TWO_HAND_AXE:
			return (sint32) ( CWeaponCraftParameters::TwoHandAxeAdversaryDodgeMinModifier + (CWeaponCraftParameters::TwoHandAxeAdversaryDodgeMaxModifier - CWeaponCraftParameters::TwoHandAxeAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::PIKE:
			return (sint32) ( CWeaponCraftParameters::PikeAdversaryDodgeMinModifier + (CWeaponCraftParameters::PikeAdversaryDodgeMaxModifier - CWeaponCraftParameters::PikeAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::TWO_HAND_MACE:
			return (sint32) ( CWeaponCraftParameters::TwoHandMaceAdversaryDodgeMinModifier + (CWeaponCraftParameters::TwoHandMaceAdversaryDodgeMaxModifier - CWeaponCraftParameters::TwoHandMaceAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
			
			// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return (sint32) ( CWeaponCraftParameters::AutolauchAdversaryDodgeMinModifier + (CWeaponCraftParameters::AutolauchAdversaryDodgeMaxModifier - CWeaponCraftParameters::AutolauchAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::BOWRIFLE:
			return (sint32) ( CWeaponCraftParameters::BowrifleAdversaryDodgeMinModifier + (CWeaponCraftParameters::BowrifleAdversaryDodgeMaxModifier - CWeaponCraftParameters::BowrifleAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::LAUNCHER:
			return (sint32) ( CWeaponCraftParameters::LauncherAdversaryDodgeMinModifier + (CWeaponCraftParameters::LauncherAdversaryDodgeMaxModifier - CWeaponCraftParameters::LauncherAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::PISTOL:
			return (sint32) ( CWeaponCraftParameters::PistolAdversaryDodgeMinModifier + (CWeaponCraftParameters::PistolAdversaryDodgeMaxModifier - CWeaponCraftParameters::PistolAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::BOWPISTOL:
			return (sint32) ( CWeaponCraftParameters::BowpistolAdversaryDodgeMinModifier + (CWeaponCraftParameters::BowpistolAdversaryDodgeMaxModifier - CWeaponCraftParameters::BowpistolAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		case ITEM_TYPE::RIFLE:
			return (sint32) ( CWeaponCraftParameters::RifleAdversaryDodgeMinModifier + (CWeaponCraftParameters::RifleAdversaryDodgeMaxModifier - CWeaponCraftParameters::RifleAdversaryDodgeMinModifier ) * _CraftParameters->AdversaryDodgeModifier );
		default:;
		}
	}
	return 0;
}

//-----------------------------------------------
// return adversary parry modifier
//-----------------------------------------------
sint32 CGameItem::adversaryParryModifier() const
{
	if( _Form == 0 )
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return (sint32) ( CWeaponCraftParameters::DaggerAdversaryParryMinModifier + (CWeaponCraftParameters::DaggerAdversaryParryMaxModifier - CWeaponCraftParameters::DaggerAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::SWORD:
			return (sint32) ( CWeaponCraftParameters::SwordAdversaryParryMinModifier + (CWeaponCraftParameters::SwordAdversaryParryMaxModifier - CWeaponCraftParameters::SwordAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::MACE:
			return (sint32) ( CWeaponCraftParameters::MaceAdversaryParryMinModifier + (CWeaponCraftParameters::MaceAdversaryParryMaxModifier - CWeaponCraftParameters::MaceAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::AXE:
			return (sint32) ( CWeaponCraftParameters::AxeAdversaryParryMinModifier + (CWeaponCraftParameters::AxeAdversaryParryMaxModifier - CWeaponCraftParameters::AxeAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::SPEAR:
			return (sint32) ( CWeaponCraftParameters::SpearAdversaryParryMinModifier + (CWeaponCraftParameters::SpearAdversaryParryMaxModifier - CWeaponCraftParameters::SpearAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::STAFF:
			return (sint32) ( CWeaponCraftParameters::StaffAdversaryParryMinModifier + (CWeaponCraftParameters::StaffAdversaryParryMaxModifier - CWeaponCraftParameters::StaffAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::MAGICIAN_STAFF:
			return (sint32) ( CWeaponCraftParameters::MagicianStaffAdversaryParryMinModifier + (CWeaponCraftParameters::MagicianStaffAdversaryParryMaxModifier - CWeaponCraftParameters::MagicianStaffAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::TWO_HAND_SWORD:
			return (sint32) ( CWeaponCraftParameters::TwoHandSwordAdversaryParryMinModifier + (CWeaponCraftParameters::TwoHandSwordAdversaryParryMaxModifier - CWeaponCraftParameters::TwoHandSwordAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::TWO_HAND_AXE:
			return (sint32) ( CWeaponCraftParameters::TwoHandAxeAdversaryParryMinModifier + (CWeaponCraftParameters::TwoHandAxeAdversaryParryMaxModifier - CWeaponCraftParameters::TwoHandAxeAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::PIKE:
			return (sint32) ( CWeaponCraftParameters::PikeAdversaryParryMinModifier + (CWeaponCraftParameters::PikeAdversaryParryMaxModifier - CWeaponCraftParameters::PikeAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::TWO_HAND_MACE:
			return (sint32) ( CWeaponCraftParameters::TwoHandMaceAdversaryParryMinModifier + (CWeaponCraftParameters::TwoHandMaceAdversaryParryMaxModifier - CWeaponCraftParameters::TwoHandMaceAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
			
			// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return (sint32) ( CWeaponCraftParameters::AutolauchAdversaryParryMinModifier + (CWeaponCraftParameters::AutolauchAdversaryParryMaxModifier - CWeaponCraftParameters::AutolauchAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::BOWRIFLE:
			return (sint32) ( CWeaponCraftParameters::BowrifleAdversaryParryMinModifier + (CWeaponCraftParameters::BowrifleAdversaryParryMaxModifier - CWeaponCraftParameters::BowrifleAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::LAUNCHER:
			return (sint32) ( CWeaponCraftParameters::LauncherAdversaryParryMinModifier + (CWeaponCraftParameters::LauncherAdversaryParryMaxModifier - CWeaponCraftParameters::LauncherAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::PISTOL:
			return (sint32) ( CWeaponCraftParameters::PistolAdversaryParryMinModifier + (CWeaponCraftParameters::PistolAdversaryParryMaxModifier - CWeaponCraftParameters::PistolAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::BOWPISTOL:
			return (sint32) ( CWeaponCraftParameters::BowpistolAdversaryParryMinModifier + (CWeaponCraftParameters::BowpistolAdversaryParryMaxModifier - CWeaponCraftParameters::BowpistolAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		case ITEM_TYPE::RIFLE:
			return (sint32) ( CWeaponCraftParameters::RifleAdversaryParryMinModifier + (CWeaponCraftParameters::RifleAdversaryParryMaxModifier - CWeaponCraftParameters::RifleAdversaryParryMinModifier ) * _CraftParameters->AdversaryParryModifier );
		default:;
		}
	}
	return 0;
}

//-----------------------------------------------
// return protection factor
//-----------------------------------------------
float CGameItem::protectionFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// armor and shield
		case ITEM_TYPE::SHIELD:
			return CWeaponCraftParameters::ShieldProtectionFactor + (CWeaponCraftParameters::ShieldProtectionFactorMax - CWeaponCraftParameters::ShieldProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::BUCKLER:
			return CWeaponCraftParameters::BucklerProtectionFactor + (CWeaponCraftParameters::BucklerProtectionFactorMax - CWeaponCraftParameters::BucklerProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::LIGHT_BOOTS:
			return CWeaponCraftParameters::LightBootsProtectionFactor + (CWeaponCraftParameters::LightBootsProtectionFactorMax - CWeaponCraftParameters::LightBootsProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::LIGHT_GLOVES:
			return CWeaponCraftParameters::LightGlovesProtectionFactor + (CWeaponCraftParameters::LightGlovesProtectionFactorMax - CWeaponCraftParameters::LightGlovesProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::LIGHT_PANTS:
			return CWeaponCraftParameters::LightPantsProtectionFactor + (CWeaponCraftParameters::LightPantsProtectionFactorMax - CWeaponCraftParameters::LightPantsProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::LIGHT_SLEEVES:
			return CWeaponCraftParameters::LightSleevesProtectionFactor + (CWeaponCraftParameters::LightSleevesProtectionFactorMax - CWeaponCraftParameters::LightSleevesProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::LIGHT_VEST:
			return CWeaponCraftParameters::LightVestProtectionFactor + (CWeaponCraftParameters::LightVestProtectionFactorMax - CWeaponCraftParameters::LightVestProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::MEDIUM_BOOTS:
			return CWeaponCraftParameters::MediumBootsProtectionFactor + (CWeaponCraftParameters::MediumBootsProtectionFactorMax - CWeaponCraftParameters::MediumBootsProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::MEDIUM_GLOVES:
			return CWeaponCraftParameters::MediumGlovesProtectionFactor + (CWeaponCraftParameters::MediumGlovesProtectionFactorMax - CWeaponCraftParameters::MediumGlovesProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::MEDIUM_PANTS:
			return CWeaponCraftParameters::MediumPantsProtectionFactor + (CWeaponCraftParameters::MediumPantsProtectionFactorMax - CWeaponCraftParameters::MediumPantsProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::MEDIUM_SLEEVES:
			return CWeaponCraftParameters::MediumSleevesProtectionFactor + (CWeaponCraftParameters::MediumSleevesProtectionFactorMax - CWeaponCraftParameters::MediumSleevesProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::MEDIUM_VEST:
			return CWeaponCraftParameters::MediumVestProtectionFactor + (CWeaponCraftParameters::MediumVestProtectionFactorMax - CWeaponCraftParameters::MediumVestProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::HEAVY_BOOTS:
			return CWeaponCraftParameters::HeavyBootsProtectionFactor + (CWeaponCraftParameters::HeavyBootsProtectionFactorMax - CWeaponCraftParameters::HeavyBootsProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::HEAVY_GLOVES:
			return CWeaponCraftParameters::HeavyGlovesProtectionFactor + (CWeaponCraftParameters::HeavyGlovesProtectionFactorMax - CWeaponCraftParameters::HeavyGlovesProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::HEAVY_PANTS:
			return CWeaponCraftParameters::HeavyPantsProtectionFactor + (CWeaponCraftParameters::HeavyPantsProtectionFactorMax - CWeaponCraftParameters::HeavyPantsProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::HEAVY_SLEEVES:
			return CWeaponCraftParameters::HeavySleevesProtectionFactor + (CWeaponCraftParameters::HeavySleevesProtectionFactorMax - CWeaponCraftParameters::HeavySleevesProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::HEAVY_VEST:
			return CWeaponCraftParameters::HeavyVestProtectionFactor + (CWeaponCraftParameters::HeavyVestProtectionFactorMax - CWeaponCraftParameters::HeavyVestProtectionFactor) * _CraftParameters->ProtectionFactor;
		case ITEM_TYPE::HEAVY_HELMET:
			return CWeaponCraftParameters::HeavyHelmetProtectionFactor + (CWeaponCraftParameters::HeavyHelmetProtectionFactorMax - CWeaponCraftParameters::HeavyHelmetProtectionFactor) * _CraftParameters->ProtectionFactor;
		default:;
		}
	}
	return 0.0f;
}

//-----------------------------------------------
// return maximum slashing protection
//-----------------------------------------------
uint32 CGameItem::maxSlashingProtection() const
{
	if( _Form == 0)
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// armor and shield
		case ITEM_TYPE::SHIELD:
			return (uint32) ( ( CWeaponCraftParameters::ShieldMaxSlashingProtection + CWeaponCraftParameters::ShieldMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::BUCKLER:
			return (uint32) ( ( CWeaponCraftParameters::BucklerMaxSlashingProtection + CWeaponCraftParameters::BucklerMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::LightBootsMaxSlashingProtection + CWeaponCraftParameters::LightBootsMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::LightGlovesMaxSlashingProtection + CWeaponCraftParameters::LightGlovesMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::LightPantsMaxSlashingProtection + CWeaponCraftParameters::LightPantsMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::LightSleevesMaxSlashingProtection + CWeaponCraftParameters::LightSleevesMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_VEST:
			return (uint32) ( ( CWeaponCraftParameters::LightVestMaxSlashingProtection + CWeaponCraftParameters::LightVestMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::MediumBootsMaxSlashingProtection + CWeaponCraftParameters::MediumBootsMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::MediumGlovesMaxSlashingProtection + CWeaponCraftParameters::MediumGlovesMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::MediumPantsMaxSlashingProtection + CWeaponCraftParameters::MediumPantsMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::MediumSleevesMaxSlashingProtection + CWeaponCraftParameters::MediumSleevesMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_VEST:
			return (uint32) ( ( CWeaponCraftParameters::MediumVestMaxSlashingProtection + CWeaponCraftParameters::MediumVestMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::HeavyBootsMaxSlashingProtection + CWeaponCraftParameters::HeavyBootsMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::HeavyGlovesMaxSlashingProtection + CWeaponCraftParameters::HeavyGlovesMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::HeavyPantsMaxSlashingProtection + CWeaponCraftParameters::HeavyPantsMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::HeavySleevesMaxSlashingProtection + CWeaponCraftParameters::HeavySleevesMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_VEST:
			return (uint32) ( ( CWeaponCraftParameters::HeavyVestMaxSlashingProtection + CWeaponCraftParameters::HeavyVestMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_HELMET:
			return (uint32) ( ( CWeaponCraftParameters::HeavyHelmetMaxSlashingProtection + CWeaponCraftParameters::HeavyHelmetMaxSlashingProtection * _CraftParameters->MaxSlashingProtection ) * _Recommended );
		default:;
		}
	}
	return 0;
}

//-----------------------------------------------
// return maximum blunt protection
//-----------------------------------------------
uint32 CGameItem::maxBluntProtection() const
{
	if( _Form == 0 )
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// armor and shield
		case ITEM_TYPE::SHIELD:
			return (uint32) ( ( CWeaponCraftParameters::ShieldMaxBluntProtection + CWeaponCraftParameters::ShieldMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::BUCKLER:
			return (uint32) ( ( CWeaponCraftParameters::BucklerMaxBluntProtection + CWeaponCraftParameters::BucklerMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::LightBootsMaxBluntProtection + CWeaponCraftParameters::LightBootsMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::LightGlovesMaxBluntProtection + CWeaponCraftParameters::LightGlovesMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::LightPantsMaxBluntProtection + CWeaponCraftParameters::LightPantsMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::LightSleevesMaxBluntProtection + CWeaponCraftParameters::LightSleevesMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_VEST:
			return (uint32) ( ( CWeaponCraftParameters::LightVestMaxBluntProtection + CWeaponCraftParameters::LightVestMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::MediumBootsMaxBluntProtection + CWeaponCraftParameters::MediumBootsMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::MediumGlovesMaxBluntProtection + CWeaponCraftParameters::MediumGlovesMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::MediumPantsMaxBluntProtection + CWeaponCraftParameters::MediumPantsMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::MediumSleevesMaxBluntProtection + CWeaponCraftParameters::MediumSleevesMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_VEST:
			return (uint32) ( ( CWeaponCraftParameters::MediumVestMaxBluntProtection + CWeaponCraftParameters::MediumVestMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::HeavyBootsMaxBluntProtection + CWeaponCraftParameters::HeavyBootsMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::HeavyGlovesMaxBluntProtection + CWeaponCraftParameters::HeavyGlovesMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::HeavyPantsMaxBluntProtection + CWeaponCraftParameters::HeavyPantsMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended);
		case ITEM_TYPE::HEAVY_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::HeavySleevesMaxBluntProtection + CWeaponCraftParameters::HeavySleevesMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_VEST:
			return (uint32) ( ( CWeaponCraftParameters::HeavyVestMaxBluntProtection + CWeaponCraftParameters::HeavyVestMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_HELMET:
			return (uint32) ( ( CWeaponCraftParameters::HeavyHelmetMaxBluntProtection + CWeaponCraftParameters::HeavyHelmetMaxBluntProtection * _CraftParameters->MaxBluntProtection ) * _Recommended );
		default:;
		}
	}
	return 0;
}

//-----------------------------------------------
// return maximum piercing protection
//-----------------------------------------------
uint32 CGameItem::maxPiercingProtection() const
{
	if( _Form == 0 )
		return 0;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// armor and shield
		case ITEM_TYPE::SHIELD:
			return (uint32) ( ( CWeaponCraftParameters::ShieldMaxPiercingProtection + CWeaponCraftParameters::ShieldMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::BUCKLER:
			return (uint32) ( ( CWeaponCraftParameters::BucklerMaxPiercingProtection + CWeaponCraftParameters::BucklerMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::LightBootsMaxPiercingProtection + CWeaponCraftParameters::LightBootsMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::LightGlovesMaxPiercingProtection + CWeaponCraftParameters::LightGlovesMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::LightPantsMaxPiercingProtection + CWeaponCraftParameters::LightPantsMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::LightSleevesMaxPiercingProtection + CWeaponCraftParameters::LightSleevesMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::LIGHT_VEST:
			return (uint32) ( ( CWeaponCraftParameters::LightVestMaxPiercingProtection + CWeaponCraftParameters::LightVestMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::MediumBootsMaxPiercingProtection + CWeaponCraftParameters::MediumBootsMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::MediumGlovesMaxPiercingProtection + CWeaponCraftParameters::MediumGlovesMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::MediumPantsMaxPiercingProtection + CWeaponCraftParameters::MediumPantsMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::MediumSleevesMaxPiercingProtection + CWeaponCraftParameters::MediumSleevesMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::MEDIUM_VEST:
			return (uint32) ( ( CWeaponCraftParameters::MediumVestMaxPiercingProtection + CWeaponCraftParameters::MediumVestMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_BOOTS:
			return (uint32) ( ( CWeaponCraftParameters::HeavyBootsMaxPiercingProtection + CWeaponCraftParameters::HeavyBootsMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_GLOVES:
			return (uint32) ( ( CWeaponCraftParameters::HeavyGlovesMaxPiercingProtection + CWeaponCraftParameters::HeavyGlovesMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_PANTS:
			return (uint32) ( ( CWeaponCraftParameters::HeavyPantsMaxPiercingProtection + CWeaponCraftParameters::HeavyPantsMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_SLEEVES:
			return (uint32) ( ( CWeaponCraftParameters::HeavySleevesMaxPiercingProtection + CWeaponCraftParameters::HeavySleevesMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_VEST:
			return (uint32) ( ( CWeaponCraftParameters::HeavyVestMaxPiercingProtection + CWeaponCraftParameters::HeavyVestMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		case ITEM_TYPE::HEAVY_HELMET:
			return (uint32) ( ( CWeaponCraftParameters::HeavyHelmetMaxPiercingProtection + CWeaponCraftParameters::HeavyHelmetMaxPiercingProtection * _CraftParameters->MaxPiercingProtection ) * _Recommended );
		default:;
		}
	}
	return 0;
}


//-----------------------------------------------
// get one of the three possible protection
//-----------------------------------------------
void CGameItem::magicProtection(uint32 protectionNumber, PROTECTION_TYPE::TProtectionType& protectionType, uint32& protectionValue) const
{
	protectionType = PROTECTION_TYPE::None;
	protectionValue = 0;

	if (_CraftParameters == 0)
		return;

	switch (protectionNumber)
	{
	case 1:
		protectionType = _CraftParameters->Protection1;
		protectionValue = (uint32)(_CraftParameters->Protection1Factor * getMagicProtectionCraftParateters( protectionType ) * 100.0f);
		break;
	case 2:
		protectionType = _CraftParameters->Protection2;
		protectionValue = (uint32)(_CraftParameters->Protection2Factor * getMagicProtectionCraftParateters( protectionType ) * 100.0f);
		break;
	case 3:
		protectionType = _CraftParameters->Protection3;
		protectionValue = (uint32)(_CraftParameters->Protection3Factor * getMagicProtectionCraftParateters( protectionType ) * 100.0f);
		break;
	default:
		BOMB("CGameItem::protection called with invalid protectionNumber(uint32) parameter", return);
		break;
	}
}


//-----------------------------------------------
// return the CWeaponCraftParameters variable corresponding to protection type 
//-----------------------------------------------
float CGameItem::getMagicProtectionCraftParateters( PROTECTION_TYPE::TProtectionType protection ) const
{
	switch( protection )
	{
	case PROTECTION_TYPE::Cold:
		return CWeaponCraftParameters::ColdJewelProtection;
	case PROTECTION_TYPE::Acid:
		return CWeaponCraftParameters::AcidJewelProtection;
	case PROTECTION_TYPE::Rot:
		return CWeaponCraftParameters::RotJewelProtection;
	case PROTECTION_TYPE::Fire:
		return CWeaponCraftParameters::FireJewelProtection;
	case PROTECTION_TYPE::Shockwave:
		return CWeaponCraftParameters::ShockWaveJewelProtection;
	case PROTECTION_TYPE::Poison:
		return CWeaponCraftParameters::PoisonJewelProtection;
	case PROTECTION_TYPE::Electricity:
		return CWeaponCraftParameters::ElectricityJewelProtection;
	default:
		return 0.0f;
	}
}

//-----------------------------------------------
// return protection gived by item for a protection type 
//-----------------------------------------------
uint32 CGameItem::magicProtection(PROTECTION_TYPE::TProtectionType protectionType) const
{
	if (_CraftParameters == 0 || protectionType == PROTECTION_TYPE::None)
		return 0;

	if (protectionType == _CraftParameters->Protection1)
	{
		return (uint32)(_CraftParameters->Protection1Factor * getMagicProtectionCraftParateters( protectionType ) * 100.0f);
	}
	else if (protectionType == _CraftParameters->Protection2)
	{
		return (uint32)(_CraftParameters->Protection2Factor * getMagicProtectionCraftParateters( protectionType ) * 100.0f);
	}
	else if (protectionType == _CraftParameters->Protection3)
	{
		return (uint32)(_CraftParameters->Protection3Factor * getMagicProtectionCraftParateters( protectionType ) * 100.0f);
	}
	return 0;
}


//-----------------------------------------------
// return resistance gived by item for a resistance type
//-----------------------------------------------
uint32 CGameItem::magicResistance(RESISTANCE_TYPE::TResistanceType resistanceType) const
{
	if( _CraftParameters == 0 || resistanceType == RESISTANCE_TYPE::None )
		return 0;

	switch(resistanceType)
	{
		case RESISTANCE_TYPE::Desert:
			return (uint32)(_CraftParameters->DesertResistanceFactor * CWeaponCraftParameters::DesertResistance * 100.0f);
		case RESISTANCE_TYPE::Forest:
			return (uint32)(_CraftParameters->ForestResistanceFactor * CWeaponCraftParameters::ForestResistance * 100.0f);
		case RESISTANCE_TYPE::Lacustre:
			return (uint32)(_CraftParameters->LacustreResistanceFactor * CWeaponCraftParameters::LacustreResistance * 100.0f);
		case RESISTANCE_TYPE::Jungle:
			return (uint32)(_CraftParameters->JungleResistanceFactor * CWeaponCraftParameters::JungleResistance * 100.0f);
		case RESISTANCE_TYPE::PrimaryRoot:
			return (uint32)(_CraftParameters->PrimaryRootResistanceFactor * CWeaponCraftParameters::PrimaryRootResistance * 100.0f);
		default:
			return 0;
	}
}

//-----------------------------------------------
// armorHpBuff
//-----------------------------------------------
sint32 CGameItem::armorHpBuff() const
{
	if (_Form == NULL || _Form->Family != ITEMFAMILY::ARMOR)
		return 0;

	nlassert(_Form->Armor != NULL);

	sint32 hpBuff;
	switch (_Form->Armor->ArmorType)
	{
	case ARMORTYPE::MEDIUM:
		hpBuff = quality() / 2;
		break;

	case ARMORTYPE::HEAVY:
		hpBuff = quality();
		break;

	default:
		hpBuff = 0;
		break;
	}

	return hpBuff;
}

//-----------------------------------------------
// return ElementalCastingTimeFactor 
//-----------------------------------------------
float CGameItem::getElementalCastingTimeFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerElementalCastingTimeFactor + (CWeaponCraftParameters::DaggerElementalCastingTimeFactorMax - CWeaponCraftParameters::DaggerElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordElementalCastingTimeFactor + (CWeaponCraftParameters::SwordElementalCastingTimeFactorMax - CWeaponCraftParameters::SwordElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceElementalCastingTimeFactor + (CWeaponCraftParameters::MaceElementalCastingTimeFactorMax - CWeaponCraftParameters::MaceElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeElementalCastingTimeFactor + (CWeaponCraftParameters::AxeElementalCastingTimeFactorMax - CWeaponCraftParameters::AxeElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearElementalCastingTimeFactor + (CWeaponCraftParameters::SpearElementalCastingTimeFactorMax - CWeaponCraftParameters::SpearElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffElementalCastingTimeFactor + (CWeaponCraftParameters::StaffElementalCastingTimeFactorMax - CWeaponCraftParameters::StaffElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffElementalCastingTimeFactor + (CWeaponCraftParameters::MagicianStaffElementalCastingTimeFactorMax - CWeaponCraftParameters::MagicianStaffElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordElementalCastingTimeFactor + (CWeaponCraftParameters::TwoHandSwordElementalCastingTimeFactorMax - CWeaponCraftParameters::TwoHandSwordElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeElementalCastingTimeFactor + (CWeaponCraftParameters::TwoHandAxeElementalCastingTimeFactorMax - CWeaponCraftParameters::TwoHandAxeElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeElementalCastingTimeFactor + (CWeaponCraftParameters::PikeElementalCastingTimeFactorMax - CWeaponCraftParameters::PikeElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceElementalCastingTimeFactor + (CWeaponCraftParameters::TwoHandMaceElementalCastingTimeFactorMax - CWeaponCraftParameters::TwoHandMaceElementalCastingTimeFactor) * _CraftParameters->ElementalCastingTimeFactor;
		default:;
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return ElementalPowerFactor
//-----------------------------------------------
float CGameItem::getElementalPowerFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerElementalPowerFactor + (CWeaponCraftParameters::DaggerElementalPowerFactorMax - CWeaponCraftParameters::DaggerElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordElementalPowerFactor + (CWeaponCraftParameters::SwordElementalPowerFactorMax - CWeaponCraftParameters::SwordElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceElementalPowerFactor + (CWeaponCraftParameters::MaceElementalPowerFactorMax - CWeaponCraftParameters::MaceElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeElementalPowerFactor + (CWeaponCraftParameters::AxeElementalPowerFactorMax - CWeaponCraftParameters::AxeElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearElementalPowerFactor + (CWeaponCraftParameters::SpearElementalPowerFactorMax - CWeaponCraftParameters::SpearElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffElementalPowerFactor + (CWeaponCraftParameters::StaffElementalPowerFactorMax - CWeaponCraftParameters::StaffElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffElementalPowerFactor + (CWeaponCraftParameters::MagicianStaffElementalPowerFactorMax - CWeaponCraftParameters::MagicianStaffElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordElementalPowerFactor + (CWeaponCraftParameters::TwoHandSwordElementalPowerFactorMax - CWeaponCraftParameters::TwoHandSwordElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeElementalPowerFactor + (CWeaponCraftParameters::TwoHandAxeElementalPowerFactorMax - CWeaponCraftParameters::TwoHandAxeElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeElementalPowerFactor + (CWeaponCraftParameters::PikeElementalPowerFactorMax - CWeaponCraftParameters::PikeElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceElementalPowerFactor + (CWeaponCraftParameters::TwoHandMaceElementalPowerFactorMax - CWeaponCraftParameters::TwoHandMaceElementalPowerFactor) * _CraftParameters->ElementalPowerFactor;
		default:;
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return OffensiveAfflictionCastingTimeFactor
//-----------------------------------------------
float CGameItem::getOffensiveAfflictionCastingTimeFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::DaggerOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::DaggerOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::SwordOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::SwordOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::MaceOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::MaceOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::AxeOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::AxeOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::SpearOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::SpearOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::StaffOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::StaffOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::MagicianStaffOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::MagicianStaffOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::PikeOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::PikeOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionCastingTimeFactor) * _CraftParameters->OffensiveAfflictionCastingTimeFactor;
		default:;
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return OffensiveAfflictionPowerFactor
//-----------------------------------------------
float CGameItem::getOffensiveAfflictionPowerFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::DaggerOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::DaggerOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::SwordOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::SwordOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::MaceOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::MaceOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::AxeOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::AxeOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::SpearOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::SpearOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::StaffOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::StaffOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::MagicianStaffOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::MagicianStaffOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::TwoHandSwordOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::TwoHandAxeOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::PikeOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::PikeOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionPowerFactor + (CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionPowerFactorMax - CWeaponCraftParameters::TwoHandMaceOffensiveAfflictionPowerFactor) * _CraftParameters->OffensiveAfflictionPowerFactor;
		default:;
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return getHealCastingTimeFactor 
//-----------------------------------------------
float CGameItem::getHealCastingTimeFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerHealCastingTimeFactor + (CWeaponCraftParameters::DaggerHealCastingTimeFactorMax - CWeaponCraftParameters::DaggerHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordHealCastingTimeFactor + (CWeaponCraftParameters::SwordHealCastingTimeFactorMax - CWeaponCraftParameters::SwordHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceHealCastingTimeFactor + (CWeaponCraftParameters::MaceHealCastingTimeFactorMax - CWeaponCraftParameters::MaceHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeHealCastingTimeFactor + (CWeaponCraftParameters::AxeHealCastingTimeFactorMax - CWeaponCraftParameters::AxeHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearHealCastingTimeFactor + (CWeaponCraftParameters::SpearHealCastingTimeFactorMax - CWeaponCraftParameters::SpearHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffHealCastingTimeFactor + (CWeaponCraftParameters::StaffHealCastingTimeFactorMax - CWeaponCraftParameters::StaffHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffHealCastingTimeFactor + (CWeaponCraftParameters::MagicianStaffHealCastingTimeFactorMax - CWeaponCraftParameters::MagicianStaffHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordHealCastingTimeFactor + (CWeaponCraftParameters::TwoHandSwordHealCastingTimeFactorMax - CWeaponCraftParameters::TwoHandSwordHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeHealCastingTimeFactor + (CWeaponCraftParameters::TwoHandAxeHealCastingTimeFactorMax - CWeaponCraftParameters::TwoHandAxeHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeHealCastingTimeFactor + (CWeaponCraftParameters::PikeHealCastingTimeFactorMax - CWeaponCraftParameters::PikeHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceHealCastingTimeFactor + (CWeaponCraftParameters::TwoHandMaceHealCastingTimeFactorMax - CWeaponCraftParameters::TwoHandMaceHealCastingTimeFactor) * _CraftParameters->HealCastingTimeFactor;
		default:;
		}
	}
	return 0.0f;
}


//-----------------------------------------------
// return HealPowerFactor
//-----------------------------------------------
float CGameItem::getHealPowerFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerHealPowerFactor + (CWeaponCraftParameters::DaggerHealPowerFactorMax - CWeaponCraftParameters::DaggerHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordHealPowerFactor + (CWeaponCraftParameters::SwordHealPowerFactorMax - CWeaponCraftParameters::SwordHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceHealPowerFactor + (CWeaponCraftParameters::MaceHealPowerFactorMax - CWeaponCraftParameters::MaceHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeHealPowerFactor + (CWeaponCraftParameters::AxeHealPowerFactorMax - CWeaponCraftParameters::AxeHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearHealPowerFactor + (CWeaponCraftParameters::SpearHealPowerFactorMax - CWeaponCraftParameters::SpearHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffHealPowerFactor + (CWeaponCraftParameters::StaffHealPowerFactorMax - CWeaponCraftParameters::StaffHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffHealPowerFactor + (CWeaponCraftParameters::MagicianStaffHealPowerFactorMax - CWeaponCraftParameters::MagicianStaffHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordHealPowerFactor + (CWeaponCraftParameters::TwoHandSwordHealPowerFactorMax - CWeaponCraftParameters::TwoHandSwordHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeHealPowerFactor + (CWeaponCraftParameters::TwoHandAxeHealPowerFactorMax - CWeaponCraftParameters::TwoHandAxeHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeHealPowerFactor + (CWeaponCraftParameters::PikeHealPowerFactorMax - CWeaponCraftParameters::PikeHealPowerFactor) * _CraftParameters->HealPowerFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceHealPowerFactor + (CWeaponCraftParameters::TwoHandMaceHealPowerFactorMax - CWeaponCraftParameters::TwoHandMaceHealPowerFactor) * _CraftParameters->HealPowerFactor;
		default:;
		}
	}
	return 0.0f;
}

//-----------------------------------------------
// return DefensiveAfflictionCastingTimeFactor 
//-----------------------------------------------
float CGameItem::getDefensiveAfflictionCastingTimeFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::DaggerDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::DaggerDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::SwordDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::SwordDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::MaceDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::MaceDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::AxeDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::AxeDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::SpearDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::SpearDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::StaffDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::StaffDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::MagicianStaffDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::MagicianStaffDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::PikeDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::PikeDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionCastingTimeFactor + (CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionCastingTimeFactorMax - CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionCastingTimeFactor) * _CraftParameters->DefensiveAfflictionCastingTimeFactor;
		default:;
		}
	}
	return 0.0f;
}

//-----------------------------------------------
// return DefensiveAfflictionPowerFactor
//-----------------------------------------------
float CGameItem::getDefensiveAfflictionPowerFactor() const
{
	if( _Form == 0 )
		return 0.0f;
	if( _CraftParameters != 0 )
	{
		switch( _Form->Type )
		{
			// melee weapons
		case ITEM_TYPE::DAGGER:
			return CWeaponCraftParameters::DaggerDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::DaggerDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::DaggerDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::SWORD:
			return CWeaponCraftParameters::SwordDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::SwordDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::SwordDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::MACE:
			return CWeaponCraftParameters::MaceDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::MaceDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::MaceDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::AXE:
			return CWeaponCraftParameters::AxeDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::AxeDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::AxeDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::SPEAR:
			return CWeaponCraftParameters::SpearDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::SpearDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::SpearDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::STAFF:
			return CWeaponCraftParameters::StaffDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::StaffDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::StaffDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return CWeaponCraftParameters::MagicianStaffDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::MagicianStaffDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::MagicianStaffDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::TwoHandSwordDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::TWO_HAND_AXE:
			return CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::TwoHandAxeDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::PIKE:
			return CWeaponCraftParameters::PikeDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::PikeDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::PikeDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		case ITEM_TYPE::TWO_HAND_MACE:
			return CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionPowerFactor + (CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionPowerFactorMax - CWeaponCraftParameters::TwoHandMaceDefensiveAfflictionPowerFactor) * _CraftParameters->DefensiveAfflictionPowerFactor;
		default:;
		}
	}
	return 0.0f;
}

//-----------------------------------------------
// Set item stats with craft parameters
//-----------------------------------------------
void CGameItem::setCraftParameters( const CCraftParameters& param )
{
	if( _CraftParameters == 0 )
		_CraftParameters = new CItemCraftParameters();
	*_CraftParameters = param;
	_HP = maxDurability();
}


//-----------------------------------------------
// displayInLog
//-----------------------------------------------
void CGameItem::displayInLog( CLog &log )
{
	log.displayRawNL( "%s QT=%u QL=%hu Class=%u StatEnergy=%.1f",
		_SheetId.toString().c_str(), _StackSize, quality(), (uint)getItemClass(), getStatEnergy() );
	log.displayRawNL( "\tHP=%u/%u SapLoad=%u/%u creator=%s ",
		durability(), maxDurability(), sapLoad(), maxSapLoad(), getCreator().toString().c_str() );

	if (_Form)
	{						
		log.displayRawNL("\t%s Bulk=%u Weight=%u", _Form->Name.c_str(), _Form->Bulk, _Form->Weight );
	}
} // displayInLog //


//-----------------------------------------------
// getClientEnchantValue
//-----------------------------------------------
uint16 CGameItem::getClientEnchantValue() const
{
	if  ( _Enchantment.empty() || maxSapLoad() == 0 )
		return 0;
	const uint32 sapLoad = this->sapLoad();
	sint16 sabrinaValue = 0;
	float sabrinaRelativeValue = 1.0f;
	for ( uint i = 0; i < _Enchantment.size(); i++ )
	{
		const CStaticBrick * brick = CSheets::getSBrickForm(_Enchantment[i]);
		if ( !brick )
		{
			nlwarning("getClientEnchantValue : invalid brick %s",_Enchantment[i].toString().c_str());
		}
		else
		{
			if ( brick->SabrinaValue > 0 )
			{
				sabrinaValue+= brick->SabrinaValue;
			}

			if( brick->SabrinaRelativeValue > 0.0f )
			{
				sabrinaRelativeValue += brick->SabrinaRelativeValue;
			}
		}
	}
	if ( sabrinaValue != 0)
	{
		return uint8( 1 + sapLoad / (uint32)(sabrinaValue * sabrinaRelativeValue) );
	}
	else
	{
		nlwarning("an enchanted item has no sabrina value. Dumping sheets");
		for ( uint i = 0; i< _Enchantment.size(); i++ )
		{
			const CStaticBrick * brick = CSheets::getSBrickForm(_Enchantment[i]);
			if ( !brick )
			{
				nlwarning("UNKNOWN ('%s')",_Enchantment[i].toString().c_str());
			}
			else if ( brick->SabrinaValue > 0 )
			{
				nlwarning("'%s'",_Enchantment[i].toString().c_str());
			}
		}
		return 0;
	}
}


//-----------------------------------------------
// getWearPerAction
//-----------------------------------------------
float CGameItem::getWearPerAction() const
{
	if (!_Form) return 0.0f;

	if ( _Form->Family == ITEMFAMILY::MELEE_WEAPON )
	{
		switch( _Form->Type )
		{
		// melee weapons
		case ITEM_TYPE::DAGGER:
			return DaggerWearPerAction;
		case ITEM_TYPE::SWORD:
			return SwordWearPerAction;
		case ITEM_TYPE::MACE:
			return MaceWearPerAction;
		case ITEM_TYPE::AXE:
			return AxeWearPerAction;
		case ITEM_TYPE::SPEAR:
			return SpearWearPerAction;
		case ITEM_TYPE::STAFF:
			return StaffWearPerAction;
		case ITEM_TYPE::MAGICIAN_STAFF:
			return MagicianStaffWearPerAction;
		case ITEM_TYPE::TWO_HAND_SWORD:
			return TwoHandSwordWearPerAction;
		case ITEM_TYPE::TWO_HAND_AXE:
			return TwoHandAxeWearPerAction;
		case ITEM_TYPE::PIKE:
			return PikeWearPerAction;
		case ITEM_TYPE::TWO_HAND_MACE:
			return TwoHandMaceWearPerAction;
		default:
			nlwarning("Bad item type");
			return 0.0f;
		}
	}
	else if ( _Form->Family == ITEMFAMILY::RANGE_WEAPON )
	{
		switch( _Form->Type )
		{
			// range weapon
		case ITEM_TYPE::AUTOLAUCH:
			return AutolauchWearPerAction;
		case ITEM_TYPE::BOWRIFLE:
			return BowrifleWearPerAction;
		case ITEM_TYPE::LAUNCHER:
			return LauncherWearPerAction;
		case ITEM_TYPE::PISTOL:
			return PistolWearPerAction;
		case ITEM_TYPE::BOWPISTOL:
			return BowpistolWearPerAction;
		case ITEM_TYPE::RIFLE:
			return RifleWearPerAction;
		default:
			nlwarning("Bad item type");
			return 0.0f;
		}
	}
	else
	{
		switch(_Form->Family)
		{
		case ITEMFAMILY::CRAFTING_TOOL:
			return CraftingToolWearPerAction;
		case ITEMFAMILY::HARVEST_TOOL:
			return ForageToolWearPerAction;
		case ITEMFAMILY::SHIELD:
			return ShieldWearPerAction;
		case ITEMFAMILY::ARMOR:
			return ArmorWearPerAction;
		case ITEMFAMILY::JEWELRY:
			return JewelryWearPerAction;
		default:
			nlwarning("Bad family for a getWearPerAction");
		};
	}
	
	return 0.0f;
}


//-----------------------------------------------
// a bit of dummy code for debugging..
//-----------------------------------------------
static bool IsRootSheet(NLMISC::CSheetId sheet)
{
	bool init=false;
	static std::set<NLMISC::CSheetId> theSet;
	if (!init)
	{
		init=true;
		theSet.insert(NLMISC::CSheetId("bag.sitem"));
		theSet.insert(NLMISC::CSheetId("guild_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("hand_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("temporary_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("equipment_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("bag_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("pack_animal_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("bot_gift_inventory.sitem"));
		theSet.insert(NLMISC::CSheetId("player_room_inventory.sitem"));
	}
	return theSet.find(sheet)!=theSet.end();
}


//-----------------------------------------------
// post load treatment
//-----------------------------------------------
void CGameItem::postApply(INVENTORIES::TInventory refInventoryId, CCharacter * owner)
{
	_Form = CSheets::getForm(_SheetId);
	
//	GameItemManager.insertItem(this);

	computeItemWornState();

	// Backward Compatibility: Old Item format get old requirement System
	if(!_UseNewSystemRequirement)
		computeRequirementFromOldSystem();

	computeHasPrerequisit();

	// memory optimization for non craftable items, also mandatory for tools otherwise maxDurability() would always return 0
	if (_Form != NULL)
	{
		switch(_Form->Family) 
		{
			//craftable families
		case ITEMFAMILY::ARMOR:
		case ITEMFAMILY::MELEE_WEAPON:
		case ITEMFAMILY::RANGE_WEAPON:
		case ITEMFAMILY::AMMO:
		case ITEMFAMILY::SHIELD:
		case ITEMFAMILY::CRAFTING_TOOL:
		case ITEMFAMILY::HARVEST_TOOL:
			break;
		case ITEMFAMILY::JEWELRY:
			if( _CraftParameters != NULL )
			{
				switch( _CraftParameters->Protection )
				{
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Fire:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Blind:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Madness:
					_CraftParameters->DesertResistanceFactor = 0.25f;
					_CraftParameters->Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
					break;
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Poison:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Sleep:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Snare:
					_CraftParameters->ForestResistanceFactor = 0.25f;
					_CraftParameters->Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
					break;
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Cold:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Shockwave:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Stun:
					_CraftParameters->LacustreResistanceFactor = 0.25f;
					_CraftParameters->Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
					break;
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Electricity:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Slow:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Root:
					_CraftParameters->JungleResistanceFactor = 0.25f;
					_CraftParameters->Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
					break;
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Acid:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Rot:
				case BACK_COMPAT::OLD_PROTECTION_TYPE::Fear:
					_CraftParameters->PrimaryRootResistanceFactor = 0.25f;
					_CraftParameters->Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
					break;
				default:
					_CraftParameters->Protection = BACK_COMPAT::OLD_PROTECTION_TYPE::None;
					break;
				}
			}
			break;
			// non craftable-> release craft parameters structure
		default:
			if (_CraftParameters != NULL)
			{
				delete _CraftParameters;
				_CraftParameters = NULL;
			}
		}
	}

	// if item was equipped, equip it again
	if (refInventoryId >= 0 && refInventoryId < INVENTORIES::NUM_INVENTORY)
	{
		const uint32 slot = _RefInventorySlot;
		_RefInventorySlot = INVENTORIES::INVALID_INVENTORY_SLOT;
		if (owner != NULL)
		{
			const CInventoryPtr inv = owner->getInventory(refInventoryId);
			if (inv)
			{
				CGameItemPtr itemPtr(this);
				inv->insertItem(itemPtr, slot);
			}
			
			//owner->equipCharacter(refInventoryId, slot,getInventorySlot());
		}
	}
	else
	{
		_RefInventorySlot = INVENTORIES::INVALID_INVENTORY_SLOT;
	}
}

//-----------------------------------------------
// variable used for debugging..
//-----------------------------------------------
static bool	ParanoidItemTesting=true;
NLMISC_VARIABLE(bool, ParanoidItemTesting, "watch string 0");

//-----------------------------------------------
// testItemsForBugs
//-----------------------------------------------
void CGameItem::testItemsForBugs()
{
	if (!ParanoidItemTesting)
		return;

	H_AUTO(TestItemsForBugs);

//	NLMISC::CSheetId stackSheet=NLMISC::CSheetId("stack.sitem");

	uint32 freeItemCount=0;
	for (uint32 i=0;i<_Items.size();++i)
	{
		const CGameItem& item= _Items[i];
		// is the item allocated?
		if (item._AllocatorData<0)
		{
			// this item is supposedly allocated so run a few tests
			bool isRootItem= IsRootSheet(item._SheetId);
//			bool isStack= isRootItem|| (item._SheetId==stackSheet);

//			if (isRootItem)
//			{
//				nlassert(item._Parent==NULL);
//			}
//			else
//			{
//				nlassert(item._Parent!=NULL);
//				nlassert(item._Parent->_Children.size()>item.Loc.Slot);
//				nlassert(item._Parent->_Children[item.Loc.Slot]==&item);
//			}

//			if (isStack)
//			{
//				// make sure my children refference me
//				nlassert(item._Children.size()<256);
//				for (uint32 j=0;j<item._Children.size();++j)
//				{
//					if (item._Children[j]!=NULL)
//					{
//						nlassert(item._Children[j]->_Parent==&item);
//						nlassert(item._Children[j]->Loc.Slot==j);
//					}
//				}
//				// make sure I don't have crafting info...
////				nlassert(item._RmUsedForCraft.empty());
//			}
//			else
//			{
//				// make sure I don't have children
//				nlassert(item._Children.empty());
//				// make sure I have a reasonable quantity of crafting info
//				nlassert(item._RmUsedForCraft.size()<40);
//			}
		}
		else
		{
			// this item is free so just skip it and move on to the next one
			++freeItemCount;
		}

		// calculate a quick checksum
		// note that we 0 out the stored checksum first in order to avoid poluting the checksum calculation
		uint32 cs=0;
		uint32 oldCS= item._BugTestChecksum;
		const_cast<CGameItem&>(item)._BugTestChecksum=0;
		for (uint32 j=0;j<sizeof(item)/4;++j)
			cs+=((uint32*)&item)[j];
		const_cast<CGameItem&>(item)._BugTestChecksum=oldCS;

		// if the item has theoreticaly not been modified then do a checksum test
		// otherwise store the checksum for future refference
		if (item._BugTestUpdate!=_BugTestCounter)
		{
			nlassert(cs==item._BugTestChecksum);
		}
		else
		{
			const_cast<CGameItem&>(item)._BugTestChecksum=cs;
		}
	}
	// Check the coherence of the free item list
	uint32 count=0;
	for(uint32 i=_FirstFreeItem; i<_Items.size(); i=_Items[i]._AllocatorData)
	{
		nlassert(count<freeItemCount);
		++count;
	}
	nlassert(count==freeItemCount);
	++_BugTestCounter;
}


//-----------------------------------------------
// testItemsForBugs
//-----------------------------------------------
//void CGameItem::testPlayerInventoryForBugs(const std::vector<CGameItemPtr>& inventory)
//{
//	NLMISC::CSheetId stackSheetId("stack.sitem");
//
//	// make sure all of the inventories exist and have NULL parents
//	nlassert(inventory.size()==INVENTORIES::NUM_INVENTORY);
//	for (uint32 i=0;i<INVENTORIES::NUM_INVENTORY;++i)
//	{
//		nlassert(inventory[i]!=NULL);
//		nlassert(inventory[i]->parent()==NULL);
//	}
//
//	// check the sheets of the inventories
//	nlassert(inventory[INVENTORIES::handling]->getSheetId()==NLMISC::CSheetId("hand_inventory.sitem"));
//	nlassert(inventory[INVENTORIES::temporary]->getSheetId()==NLMISC::CSheetId("temporary_inventory.sitem"));
//	nlassert(inventory[INVENTORIES::pickup]->getSheetId()==NLMISC::CSheetId("temporary_inventory.sitem"));
//	nlassert(inventory[INVENTORIES::equipment]->getSheetId()==NLMISC::CSheetId("equipment_inventory.sitem"));
//	nlassert(inventory[INVENTORIES::bag]->getSheetId()==NLMISC::CSheetId("bag_inventory.sitem"));
//	for( int ii = 0; ii < MAX_INVENTORY_ANIMAL; ++ii ) 
//	{
//		nlassert(inventory[INVENTORIES::pack_animal+ii]->getSheetId()==NLMISC::CSheetId("pack_animal_inventory.sitem")); //must be re-init with true pack animal inventory (but this break character save, waiting the backup system)
//	}
//	nlassert(inventory[INVENTORIES::harvest]->getSheetId()==NLMISC::CSheetId("temporary_inventory.sitem")); 
//	nlassert(inventory[INVENTORIES::bot_gift]->getSheetId()==NLMISC::CSheetId("bot_gift_inventory.sitem"));
////	nlassert(inventory[INVENTORIES::pack_steed ]->getSheetId()==NLMISC::CSheetId("pack_animal_inventory.sitem")); //must be re-init with true pack animal inventory (but this break character save, waiting the backup system)
//
//	// check the inventory childrent
//	for (uint32 i=0;i<INVENTORIES::NUM_INVENTORY;++i)
//	{
////		const std::vector<CGameItemPtr >& children= inventory[i]->getChildren();
//		const std::vector<CGameItemPtr >& children= inventory[i]->_Children;
//		nlassert(children.size()<=256);
//		for (uint32 j=0;j<children.size();++i)
//		{
//			CGameItemPtr item= children[j];
//			if (item==NULL)
//				continue;
//
//			nlassert(item->parent()==inventory[i]);
//			nlassert(item->Loc.Slot==j);
//			nlassert(!IsRootSheet(item->getSheetId()));
//			if (item->getSheetId()==stackSheetId)
//			{
//				nlassert(false);
//				nlassert(item->getEnchantment().empty());
////				nlassert(item->getRmUsedForCraft().empty());
////				const std::vector<CGameItemPtr >& kids= item->getChildren();
//				const std::vector<CGameItemPtr >& kids= item->_Children;
//				nlassert(kids.size()<=256);
//				for (uint32 k=0;k<kids.size();++k)
//				{
//					if (kids[k]==NULL)
//						continue;
//					nlassert(kids[k]->parent()==item);
//					nlassert(kids[k]->Loc.Slot==k);
//					nlassert(!IsRootSheet(kids[k]->getSheetId()));
//					nlassert(kids[k]->getSheetId()!=stackSheetId);
//				}
//			}
//			else
//			{
////				nlassert(item->getChildren().empty());
//				nlassert(item->_Children.empty());
//				nlassert(item->getEnchantment().size()<50);
////				nlassert(item->getRmUsedForCraft().size()<50);
//			}
//		}
//	}
//}

//-----------------------------------------------
// testItemsForBugs
//-----------------------------------------------
void CGameItem::checkItemForBugs()
{
//	NLMISC::CSheetId stackSheetId("stack.sitem");

	// make sure 'this' is somewhere sensible
	CGameItem::_Items.getUniqueIndex(*this);

	// make sure we're not unallocated
	nlassert(_AllocatorData<0);

	// TODO : rewrite the check

//	if (IsRootSheet(getSheetId()))
//	{
//		// a root item
//		nlassert(parent()==NULL);
////		nlassert(getChildren().size()<=256);
//		nlassert(_Children.size() <= 256);
//	}
//	else if (getSheetId()== NLMISC::CSheetId("stack.sitem"))
//	{
//		nlassert(false);
//		// a stack
//		nlassert(parent()!=NULL);
//		nlassert(IsRootSheet(parent()->getSheetId()));
////		nlassert(getChildren().size()<=256);
//		nlassert(_Children.size()<=256);
//		nlassert(getEnchantment().empty());
////		nlassert(getRmUsedForCraft().empty());
////		for (uint32 i=0;i<getChildren().size();++i)
//		for (uint32 i=0;i<_Children.size();++i)
////			if (getChildren()[i]!=NULL)
//			if (_Children[i]!=NULL)
////				getChildren()[i]->checkItemForBugs();
//				_Children[i]->checkItemForBugs();
//	}
//	else
//	{
//		// a normal item
//		nlassert(parent()!=NULL);
//		nlassert(parent()->getSheetId()==stackSheetId || IsRootSheet(parent()->getSheetId()));
////		nlassert(parent()->getChildren().size()>=Loc.Slot)
//		nlassert(parent()->_Children.size() >= Loc.Slot)
////		nlassert(parent()->getChildren()[Loc.Slot]==this)
//		nlassert(parent()->_Children[Loc.Slot] == this)
////		nlassert(getChildren().empty());
//		nlassert(_Children.empty());
//		nlassert(getEnchantment().size()<50);
////		nlassert(getRmUsedForCraft().size()<50);
//	}
}

//-----------------------------------------------
// COMMAND: testParanoidItemSystem
//-----------------------------------------------
NLMISC_COMMAND(testParanoidItemSystem,"run some test code that should provoke a crash","<test case to run>")
{
	log.displayNL("not implemented yet");
#define TEST(x) egs_giinfo("Testing: " #x); x

	return true;	
//#define TEST(x) nlinfo("Testing: " #x); x
//
//	if (args.size()!=1)
//		return false;
//
//	sint test;
//	NLMISC::fromString(args[0], test);
//	switch (test)
//	{
//	case 0:
//		{
//		// setup my test item (should be fine)
//		CGameItem::testItemsForBugs();
//		TEST( CGameItemPtr p0; )
//		CGameItem::testItemsForBugs();
//		TEST( CGameItem* p1= p0.newItem(); )
//		CGameItem::testItemsForBugs();
//
//		// the following should run fine too...
//		CGameItem::testItemsForBugs();
//		TEST( p0->quality(p0->quality()+1); )								
//		CGameItem::testItemsForBugs();
//
//		// the following block should bomb out due to use of 'p1' without derefferencing a gameItemPtr
//		CGameItem::testItemsForBugs();
//		TEST( p1->quality(p1->quality()+1); )								
//		CGameItem::testItemsForBugs();
//		break;
//		}
//
//	case 1:
//		{
//		// test parent child linkage
//		CGameItem::testItemsForBugs();
//		TEST( CGameItemPtr p= CGameItemPtr().newItem(); )
//		p->setParent(p);
//		CGameItem::testItemsForBugs();
//		}
//
//	case 2:
//		{
//		// test parent child linkage
//		CGameItem::testItemsForBugs();
//		TEST( CGameItemPtr p= CGameItemPtr().newItem(); )
////		std::vector<CGameItemPtr >& children= const_cast<std::vector<CGameItemPtr >&>(p->getChildren());
//		std::vector<CGameItemPtr > &children = p->_Children;
//		children.push_back(p);
//		CGameItem::testItemsForBugs();
//		}
//	}
//
//	return true;
//
//#undef TEST
}

//-----------------------------------------------
// testItemsForBugs
//-----------------------------------------------
std::string CGameItem::showItemsStats()
{
//#pragma message(NL_LOC_MSG "Implement me")
	// TODO: implement this

	return "";
//	NLMISC::CSheetId stackSheet=NLMISC::CSheetId("stack.sitem");
//
//	uint32 freeItemCount=0;
//	uint32 rootItemCount=0;
//	uint32 stackChildrenCount=0;
//	uint32 attachedItemCount=0;
//	uint32 detachedItemCount=0;
//	uint32 attachedStackCount=0;
//	uint32 detachedStackCount=0;
//	
//	for (uint32 i=0;i<_Items.size();++i)
//	{
//		const CGameItem& item= _Items[i];
//		// is the item allocated?
//		if (item._AllocatorData<0)
//		{
//			// this item is supposedly allocated so run a few tests
//			bool isRootItem= IsRootSheet(item._SheetId);
//			bool isStack= item._SheetId==stackSheet;
//			
//			if (isRootItem)
//			{
//				nlassert(item._Parent==NULL);
//				++rootItemCount;
//			}
//			else
//			{
//				if (isStack)
//				{
//					if (item._Parent==NULL)
//						++detachedStackCount;
//					else
//						++attachedStackCount;
//				}
//				else
//				{
//					if (item._Parent==NULL)
//						++detachedItemCount;
//					else
//						++attachedItemCount;
//				}
//			}
//			
//			if (isStack)
//			{
//				// make sure my children refference me
//				nlassert(item._Children.size()<256);
//				for (uint32 j=0;j<item._Children.size();++j)
//				{
//					if (item._Children[j]!=NULL)
//					{
//						nlassert(item._Children[j]->_Parent==&item);
//						nlassert(item._Children[j]->Loc.Slot==j);
//						++stackChildrenCount;
//					}
//				}
//			}
//			else
//			{
//				// make sure I don't have children
//				if (!isRootItem)
//					nlassert(item._Children.empty());
//			}
//		}
//		else
//		{
//			// this item is free so just skip it and move on to the next one
//			++freeItemCount;
//		}		
//	}
//	return NLMISC::toString("Item stats: FREE:%d  ROOT:%d  STACK_ATTACHED:%d  STACK_DETACHED:%d  CHILD:%d  ITEM_ATTACHED:%d  ITEM_DETACHED:%d, TOTAL: %d (Allocated Memory %d)",
//		freeItemCount,rootItemCount,attachedStackCount,detachedStackCount,stackChildrenCount,attachedItemCount,detachedItemCount, _Items.size(), sizeof(CGameItem) * _Items.size());
}

//-----------------------------------------------
// COMMAND: launch item tests
//-----------------------------------------------
NLMISC_COMMAND(testItemSystem,"call CGameItem::testItemsForBugs()","")
{
	CGameItem::testItemsForBugs();
	return true;
}

//-----------------------------------------------
// COMMAND: items stats
//-----------------------------------------------
NLMISC_COMMAND(displayItemsStats,"Display items stats and memory allocation","")
{
	log.displayNL( CGameItem::showItemsStats().c_str() );
	return true;
}
