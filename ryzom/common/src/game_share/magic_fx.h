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



#ifndef RY_MAGIC_FX_H
#define RY_MAGIC_FX_H

#include "nel/misc/types_nl.h"
#include "damage_types.h"
#include "effect_families.h"

namespace MAGICFX
{
	//
	enum TSpellMode
	{
		Bomb = 0,
		Chain,
		Spray,
		SpellModeCount,
		UnknownSpellMode = SpellModeCount,
	};
	//
	enum TSpellType
	{
		Off = 0,
		Cur,
		Mix,
		SpellTypeCount,

		UnknownSpellType = SpellTypeCount,
	};
	// TODO: this enum is also used for attack, so could name it in a more general way
	enum TSpellCastStage
	{
		CastBegin = 0,
		CastLoop,
		CastEnd,
		CastFail,
		SpellCastStageCount,

		UnknownSpellCastStage = SpellCastStageCount,
	};
	//
	const std::string& toString(TSpellMode mode);
	const std::string& toString(TSpellType mode);
	const std::string& toString(TSpellCastStage mode);

	TSpellMode toSpellMode(const std::string &str);

	// the number of possible powers for a spell
	// TODO : this is also used for melee, range, so name it in a more general way in another namespace (NUM_ATTACK_POWER ?)
	const uint NUM_SPELL_POWER = 5;

	// projectile speed in meters per second
	const float PROJECTILE_SPEED = 20.f;

	/** the value of each enum should match the value in spells.spell_list
	  * and in links.id_to_string_array
	  */
	enum TMagicFx
	{
		// offensif
		Piercing = 0,   // ok
		Blunt = 1,      // ok
		Slashing = 2,   // ok
		Cold = 3,       // ok
		Rot = 4,        // ok
		Acid = 5,       // ok
		Fire = 6,		// ok
		Shockwave = 7,  // ok
		Electric = 8,   // ok
		Poison = 9,     // ok
		Curse = 10,     //
		Sickness = 11,  //
		Hatred = 12,    //
		Mezz = 13,      // ok
		Root = 14,      // ok
		Fear = 18,      // ok

		// curatif
		HealHP = 15,
		HealSap = 16,
		HealSta = 17,

		Stun = 19,

		SlowAttack = 20,
		SlowMove = 21,
		Blind = 22,
		Madness = 23,

		// range weapon (reuse magic system for now)
		RangeWeaponFirst = 128,
		RangeWeaponGattling = RangeWeaponFirst,
		RangeWeaponMissile = 129,
		//
		Unknown,
	};
	TMagicFx toMagicFx( DMGTYPE::EDamageType type ,bool Link);
	TMagicFx toMagicFx( EFFECT_FAMILIES::TEffectFamily effect);
	TMagicFx healtoMagicFx(  sint32 healHp, sint32 healSap, sint32 healSta, bool link );


	// enum for the auras fxs
	enum TAuraFX
	{
		NoAura = 0,
		Divinity = 1,
		SpeedMove = 2,
		AuraSap = 3,
		AuraSta = 4,
		AuraHp = 5,
		ProtectionMelee = 6,
		ProtectionRange = 7,
		ProtectionMagic = 8,
		WarCry = 9,
		FireWall = 10,
		WaterWall = 11,
		ThornWall = 12,
		LightningWall = 13,
		OtherFireWall = 14,
		TeleportKami = 15, // No effect aura
		TeleportKara = 16, // No effect aura
	};
}


#endif // RY_MAGIC_FX_H

/* End of magic_fx.h */



























