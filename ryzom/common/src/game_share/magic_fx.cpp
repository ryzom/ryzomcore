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
#include "magic_fx.h"
//
#include "nel/misc/string_conversion.h"

namespace MAGICFX
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TSpellMode)
		NL_STRING_CONVERSION_TABLE_ENTRY (Bomb)
		NL_STRING_CONVERSION_TABLE_ENTRY (Chain)
		NL_STRING_CONVERSION_TABLE_ENTRY (Spray)
		NL_STRING_CONVERSION_TABLE_ENTRY (UnknownSpellMode)
	NL_END_STRING_CONVERSION_TABLE(TSpellMode, SpellModeConversion, UnknownSpellMode)
	//
	NL_BEGIN_STRING_CONVERSION_TABLE (TSpellType)
		NL_STRING_CONVERSION_TABLE_ENTRY (Off)
		NL_STRING_CONVERSION_TABLE_ENTRY (Cur)
		NL_STRING_CONVERSION_TABLE_ENTRY (Mix)
		NL_STRING_CONVERSION_TABLE_ENTRY (UnknownSpellType)
	NL_END_STRING_CONVERSION_TABLE(TSpellType, SpellTypeConversion, UnknownSpellType)
	//
	NL_BEGIN_STRING_CONVERSION_TABLE (TSpellCastStage)
		NL_STRING_CONVERSION_TABLE_ENTRY (CastBegin)
		NL_STRING_CONVERSION_TABLE_ENTRY (CastLoop)
		NL_STRING_CONVERSION_TABLE_ENTRY (CastEnd)
		NL_STRING_CONVERSION_TABLE_ENTRY (CastFail)
		NL_STRING_CONVERSION_TABLE_ENTRY (UnknownSpellCastStage)
	NL_END_STRING_CONVERSION_TABLE(TSpellCastStage, SpellCastStageConversion, UnknownSpellCastStage)
	//
	// *************************************************************************************
	const std::string& toString(TSpellMode mode)
	{
		return SpellModeConversion.toString(mode);
	}

	TSpellMode toSpellMode(const std::string &str)
	{
		return SpellModeConversion.fromString(str);
	}

	// *************************************************************************************
	const std::string& toString(TSpellType mode)
	{
		return SpellTypeConversion.toString(mode);
	}

	// *************************************************************************************
	const std::string& toString(TSpellCastStage mode)
	{
		return SpellCastStageConversion.toString(mode);
	}

	// *************************************************************************************
	TMagicFx toMagicFx( DMGTYPE::EDamageType type ,bool /* Link */)
	{
		switch(type)
		{
			case DMGTYPE::BLUNT: return Blunt;
			case DMGTYPE::SLASHING: return Slashing;
			case DMGTYPE::PIERCING: return Piercing;
			case DMGTYPE::ROT: return Rot;
			case DMGTYPE::ACID: return Acid;
			case DMGTYPE::COLD: return Cold;
			case DMGTYPE::FIRE: return Fire;
			case DMGTYPE::POISON: return Poison;
			case DMGTYPE::ELECTRICITY: return Electric;
			case DMGTYPE::SHOCK: return Shockwave;
			default:
				return Unknown;
			break;
		}
	}

	// *************************************************************************************
	TMagicFx toMagicFx( EFFECT_FAMILIES::TEffectFamily effect)
	{
		switch(effect)
		{
			case EFFECT_FAMILIES::Mezz: return Mezz;
			case EFFECT_FAMILIES::Root: return Root;
			case EFFECT_FAMILIES::Fear: return Fear;
			case EFFECT_FAMILIES::Stun: return Stun;
			case EFFECT_FAMILIES::Blind: return Blind;

			// madness
			case EFFECT_FAMILIES::Madness:
			case EFFECT_FAMILIES::MadnessMelee:
			case EFFECT_FAMILIES::MadnessMagic:
			case EFFECT_FAMILIES::MadnessRange:
				return Madness;

			// sickness
			case EFFECT_FAMILIES::DebuffSkillMelee:
			case EFFECT_FAMILIES::DebuffSkillMagic:
			case EFFECT_FAMILIES::DebuffSkillRange:
			case EFFECT_FAMILIES::DebuffResistBlunt:
			case EFFECT_FAMILIES::DebuffResistSlash:
			case EFFECT_FAMILIES::DebuffResistPierce:
			case EFFECT_FAMILIES::DebuffResistAcid:
			case EFFECT_FAMILIES::DebuffResistRot:
			case EFFECT_FAMILIES::DebuffResistCold:
			case EFFECT_FAMILIES::DebuffResistFire:
			case EFFECT_FAMILIES::DebuffResistPoison:
			case EFFECT_FAMILIES::DebuffResistSchock:
			case EFFECT_FAMILIES::DebuffResistElectricity:
				return Sickness;

			// curse
			case EFFECT_FAMILIES::Stench:
				return Curse;

			// hatred
			case EFFECT_FAMILIES::FaunaHatred:
			case EFFECT_FAMILIES::PlantHatred:
			case EFFECT_FAMILIES::KitinHatred:
			case EFFECT_FAMILIES::HominHatred:
			case EFFECT_FAMILIES::DegeneratedHatred:
				return Hatred;

			// slow
			case EFFECT_FAMILIES::SlowMelee:
			case EFFECT_FAMILIES::SlowMagic:
			case EFFECT_FAMILIES::SlowRange:
			case EFFECT_FAMILIES::SlowAttack:
				return SlowAttack;

			// snare
			case EFFECT_FAMILIES::SlowMove:
			case EFFECT_FAMILIES::Snare:
				return SlowMove;

			default:
				return Unknown;
		}
	}

	// *************************************************************************************
	TMagicFx healtoMagicFx(  sint32 healHp, sint32 healSap, sint32 healSta, bool link )
	{
		if ( !link )
		{
			if ( healHp > healSap )
			{
				if ( healHp > healSta )
					return HealHP;
				return HealSta;
			}
			else
			{
				if ( healSap > healSta )
					return HealSap;
				return HealSta;
			}
		}
		else
		{
			if ( healHp > healSap )
			{
				if ( healHp > healSta )
					return HealHP;
				return HealSta;
			}
			else
			{
				if ( healSap > healSta )
					return HealSap;
				return HealSta;
			}
		}
	}
}
