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
//
#include "nel/misc/debug.h"
#include "nel/misc/string_conversion.h"
//
#include "brick_flags.h"

using namespace std;
using namespace NLMISC;

namespace BRICK_FLAGS
{
	// The conversion table
	NL_BEGIN_STRING_CONVERSION_TABLE (TBrickFlag)
		NL_STRING_CONVERSION_TABLE_ENTRY (Miss)
		NL_STRING_CONVERSION_TABLE_ENTRY (Fumble)
		NL_STRING_CONVERSION_TABLE_ENTRY (Hit)
		NL_STRING_CONVERSION_TABLE_ENTRY (CriticalHit)
		NL_STRING_CONVERSION_TABLE_ENTRY (Parry)
		NL_STRING_CONVERSION_TABLE_ENTRY (Dodge)
		NL_STRING_CONVERSION_TABLE_ENTRY (ShieldUsed)
		NL_STRING_CONVERSION_TABLE_ENTRY (Head)
		NL_STRING_CONVERSION_TABLE_ENTRY (Chest)
		NL_STRING_CONVERSION_TABLE_ENTRY (Arms)
		NL_STRING_CONVERSION_TABLE_ENTRY (Hands)
		NL_STRING_CONVERSION_TABLE_ENTRY (Legs)
		NL_STRING_CONVERSION_TABLE_ENTRY (Feet)

		NL_STRING_CONVERSION_TABLE_ENTRY (Feint)

		NL_STRING_CONVERSION_TABLE_ENTRY (Stun)
		NL_STRING_CONVERSION_TABLE_ENTRY (Bleed)
		NL_STRING_CONVERSION_TABLE_ENTRY (SlowMove)
		NL_STRING_CONVERSION_TABLE_ENTRY (SlowAttacks)

		NL_STRING_CONVERSION_TABLE_ENTRY (Taunt)
		NL_STRING_CONVERSION_TABLE_ENTRY (SpeedingUp)
		NL_STRING_CONVERSION_TABLE_ENTRY (LifeConcentration)
		NL_STRING_CONVERSION_TABLE_ENTRY (StaminaConcentration)
		NL_STRING_CONVERSION_TABLE_ENTRY (SapConcentration)
		NL_STRING_CONVERSION_TABLE_ENTRY (ConvertStamina)
		NL_STRING_CONVERSION_TABLE_ENTRY (ConvertSap)
		NL_STRING_CONVERSION_TABLE_ENTRY (Berserk)
		NL_STRING_CONVERSION_TABLE_ENTRY (BalanceHp)
		NL_STRING_CONVERSION_TABLE_ENTRY (Heal)
		NL_STRING_CONVERSION_TABLE_ENTRY (Shielding)
		NL_STRING_CONVERSION_TABLE_ENTRY (Invulnerability)
		NL_STRING_CONVERSION_TABLE_ENTRY (EnchantWeapon)
		NL_STRING_CONVERSION_TABLE_ENTRY (ChgCharac)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModDefense)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModMeleeSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModRangeSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModMagicSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModMeleeSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModRangeSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModMagicSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (ModForageSuccess)
		NL_STRING_CONVERSION_TABLE_ENTRY (HealHpC)
		NL_STRING_CONVERSION_TABLE_ENTRY (HealSapC)
		NL_STRING_CONVERSION_TABLE_ENTRY (HealStaC)
		NL_STRING_CONVERSION_TABLE_ENTRY (HealFocusC)

		NL_STRING_CONVERSION_TABLE_ENTRY (Aura)

		NL_STRING_CONVERSION_TABLE_ENTRY (UnknownFlag)
	NL_END_STRING_CONVERSION_TABLE(TBrickFlag, FlagConversion, UnknownFlag)


	const string &toString(TBrickFlag flag)
	{
#if !FINAL_VERSION
		nlassert( NbCombatFlags < 32);
		nlassert( NbPowerFlags < 31);
#endif
		return FlagConversion.toString(flag);
	}

	TBrickFlag toBrickFlag( const string &str)
	{
#if !FINAL_VERSION
		nlassert( NbCombatFlags < 32);
		nlassert( NbPowerFlags < 31);
#endif
		return FlagConversion.fromString(str);
	}

	TBrickFlag powerTypeToFlag( POWERS::TPowerType powerType )
	{
		if (powerType >= POWERS::BeginAuras && powerType <= POWERS::EndAuras)
		{
			return Aura;
		}

		if (powerType >= POWERS::BeginPower && powerType <= POWERS::EndPower)
		{
			return (TBrickFlag)(BeginPowerFlags + powerType - POWERS::BeginPower);
		}

		return UnknownFlag;
	}


	/// convert a slot to a combat flag
	TBrickFlag slotToFlag( SLOT_EQUIPMENT::TSlotEquipment slot )
	{
		switch(slot)
		{
		case SLOT_EQUIPMENT::HEAD:
			return Head;
		case SLOT_EQUIPMENT::CHEST:
			return Chest;
		case SLOT_EQUIPMENT::ARMS:
			return Arms;
		case SLOT_EQUIPMENT::HANDS:
			return Hands;
		case SLOT_EQUIPMENT::LEGS:
			return Legs;
		case SLOT_EQUIPMENT::FEET:
			return Feet;
		default:
			return UnknownFlag;
		};
	}

	/// convert an effect family to a flag
	TBrickFlag effectFamilyToFlag( EFFECT_FAMILIES::TEffectFamily family )
	{
		switch(family)
		{
		case EFFECT_FAMILIES::CombatBleed:
			return Bleed;
		case EFFECT_FAMILIES::CombatStun:
			return Stun;
		case EFFECT_FAMILIES::CombatMvtSlow:
			return SlowMove;
		case EFFECT_FAMILIES::CombatAttackSlow:
			return SlowAttacks;
		default:
			return UnknownFlag;
		};
	}

}; // BRICK_FLAGS
