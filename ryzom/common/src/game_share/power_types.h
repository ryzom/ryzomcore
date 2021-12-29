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



#ifndef RY_POWER_TYPES_H
#define RY_POWER_TYPES_H

#include "nel/misc/types_nl.h"

namespace POWERS
{
	enum TPowerType
	{
		/************************************************************************/
		// Warning
		// Power Flags MUST be in the same order than in brick_flags.h
		/************************************************************************/
		BeginPower = 0,
			Taunt = BeginPower,
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

			// consumable item related powers
			HealHpC,
			HealSapC,
			HealStaC,
			HealFocusC,
			ChgCharac,
			ModDefenseSkill,
			ModDodgeSkill,
			ModParrySkill,
			ModCraftSkill,
			ModMeleeSkill,
			ModRangeSkill,
			ModMagicSkill,
			ModForageSkill,
			ModDesertForageSkill,
			ModForestForageSkill,
			ModLacustreForageSkill,
			ModJungleForageSkill,
			ModPrimaryRootForageSkill,
			ModMagicProtection,
		EndPower = ModMagicProtection,

		BeginAuras,
			LifeAura = BeginAuras,
			StaminaAura,
			SapAura,
			MeleeProtection,
			Umbrella,
			AntiMagicShield,
			WarCry,
			FireWall,
			ThornWall,
			WaterWall,
			LightningWall,
		EndAuras = LightningWall,

		NbTypes,
		UnknownType = NbTypes,
	};

	/// convert a power type to a string
	const std::string &toString(TPowerType type);

	/// convert a string to a power type
	TPowerType toPowerType( const std::string &str);


}; // POWERS

#endif // RY_POWER_TYPES_H
/* End of power_types.h */
