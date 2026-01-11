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



#ifndef RY_BRICK_FLAGS_H
#define RY_BRICK_FLAGS_H

#include "nel/misc/types_nl.h"
#include "power_types.h"
#include "slot_equipment.h"
#include "effect_families.h"

namespace BRICK_FLAGS
{
	enum TBrickFlag
	{
		// If you modify this enums, modify the interface's version in config.xml
		BeginCombatFlags = 0,
			Miss = BeginCombatFlags,
			Fumble,
			Hit,
			CriticalHit,

			Parry,
			Dodge,
			ShieldUsed,

			Head,
			Chest,
			Arms,
			Hands,
			Legs,
			Feet,

			Feint,

			Stun,
			Bleed,
			SlowMove,
			SlowAttacks,
		EndCombatFlags = SlowAttacks,

		NbCombatFlags,

		// END combat flags MUST be < 32

		// WARNING Power Flags MUST be in the same order than in power_types.h
		BeginPowerFlags = 32,
			Taunt = BeginPowerFlags,
			Shielding,
			SpeedingUp,
			LifeConcentration,
			StaminaConcentration,
			SapConcentration,
			ConvertStamina,
			ConvertSap,
			Berserk,
			BalanceHp,
			Heal,
			Invulnerability,
			EnchantWeapon,
			ChgCharac,
			ModDefense,
			ModCraftSuccess,
			ModMeleeSuccess,
			ModRangeSuccess,
			ModMagicSuccess,
			ModForageSuccess,
			// consumable items related powers
			HealHpC,
			HealSapC,
			HealStaC,
			HealFocusC,
		EndPowerFlags = HealFocusC,

		NbPowerFlags = EndPowerFlags - 31,

		Aura = 63,

		UnknownFlag,
	};

	/// convert a flag to a string
	const std::string &toString(TBrickFlag flag);

	/// convert a string to a flag
	TBrickFlag toBrickFlag( const std::string &str);

	/// convert a power family to a flag
	TBrickFlag powerTypeToFlag( POWERS::TPowerType powerType );

	/// convert a slot to a combat flag
	TBrickFlag slotToFlag( SLOT_EQUIPMENT::TSlotEquipment slot );

	/// convert an effect family to a flag
	TBrickFlag effectFamilyToFlag( EFFECT_FAMILIES::TEffectFamily family );

}; // BRICK_FLAGS

#endif // RY_BRICK_FLAGS_H
/* End of brick_flags.h */
