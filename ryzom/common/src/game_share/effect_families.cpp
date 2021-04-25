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
#include "effect_families.h"


namespace EFFECT_FAMILIES
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TEffectFamily)

		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffCharacteristic )

		NL_STRING_CONVERSION_TABLE_ENTRY( Teleport )
		NL_STRING_CONVERSION_TABLE_ENTRY( ProcShootAgain )

		NL_STRING_CONVERSION_TABLE_ENTRY( Hot )
		NL_STRING_CONVERSION_TABLE_ENTRY( Dot )
		NL_STRING_CONVERSION_TABLE_ENTRY( DoTStackable )

		NL_STRING_CONVERSION_TABLE_ENTRY( RedirectAttacks )
		NL_STRING_CONVERSION_TABLE_ENTRY( Stench )
		NL_STRING_CONVERSION_TABLE_ENTRY( Bounce )
		NL_STRING_CONVERSION_TABLE_ENTRY( ReflectDamage )
		NL_STRING_CONVERSION_TABLE_ENTRY( ReverseDamage )

		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffSkillMelee )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffSkillMagic )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffSkillRange )
		NL_STRING_CONVERSION_TABLE_ENTRY( SlowMove )
		NL_STRING_CONVERSION_TABLE_ENTRY( Snare )
		NL_STRING_CONVERSION_TABLE_ENTRY( SlowMelee )
		NL_STRING_CONVERSION_TABLE_ENTRY( SlowMagic )
		NL_STRING_CONVERSION_TABLE_ENTRY( SlowRange )
		NL_STRING_CONVERSION_TABLE_ENTRY( SlowAttack )

		NL_STRING_CONVERSION_TABLE_ENTRY( MadnessMelee )
		NL_STRING_CONVERSION_TABLE_ENTRY( MadnessMagic )
		NL_STRING_CONVERSION_TABLE_ENTRY( MadnessRange )
		NL_STRING_CONVERSION_TABLE_ENTRY( Madness )

		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistBlunt )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistSlash )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistPierce )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistAcid )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistRot )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistCold )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistFire )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistPoison )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistSchock )
		NL_STRING_CONVERSION_TABLE_ENTRY( DebuffResistElectricity )

		NL_STRING_CONVERSION_TABLE_ENTRY( Mezz )
		NL_STRING_CONVERSION_TABLE_ENTRY( Stun )
		NL_STRING_CONVERSION_TABLE_ENTRY( Root )
		NL_STRING_CONVERSION_TABLE_ENTRY( Fear )
		NL_STRING_CONVERSION_TABLE_ENTRY( Blind )

		NL_STRING_CONVERSION_TABLE_ENTRY( FaunaHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( PlantHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( KitinHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( HominHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( DegeneratedHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( PetHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( KamiHatred )
		NL_STRING_CONVERSION_TABLE_ENTRY( AllHatred )

		NL_STRING_CONVERSION_TABLE_ENTRY( Invincibility )

		NL_STRING_CONVERSION_TABLE_ENTRY( CombatBleed )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatStun )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatMvtSlow )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatAttackSlow )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatCastSlow )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatSlow )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDefenseModifier )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDebuffCombatSkills )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDebuffDodge )

		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDebuffStaminaRegen )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDebuffSapRegen )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDebuffHitPointsRegen )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDebuffFocusRegen )

		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTRot )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTCold )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTFire )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTElectricity )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTAcid )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTPoison )
		NL_STRING_CONVERSION_TABLE_ENTRY( CombatDoTShock )

		NL_STRING_CONVERSION_TABLE_ENTRY( PowerShielding )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerLifeAura )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerStaminaAura )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerSapAura )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerUmbrella )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerProtection )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerAntiMagicShield )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerWarCry )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerFireWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerThornWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerWaterWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerLightningWall )

		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootLifeAura )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootStaminaAura )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootSapAura )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootUmbrella )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootProtection )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootAntiMagicShield )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootWarCry )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootFireWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootThornWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootWaterWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerRootLightningWall )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerInvulnerability )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerChgCharac )

		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModDefenseSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModDodgeSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModParrySkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModCraftSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModMeleeSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModRangeSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModMagicSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModForageSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModDesertForageSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModForestForageSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModLacustreForageSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModJungleForageSkill )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModPrimaryRootForageSkill )

		NL_STRING_CONVERSION_TABLE_ENTRY( PowerModMagicProtection )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerSpeedingUp )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerBerserker )
		NL_STRING_CONVERSION_TABLE_ENTRY( PowerEnchantWeapon )

		NL_STRING_CONVERSION_TABLE_ENTRY( OutpostCombat )
		NL_STRING_CONVERSION_TABLE_ENTRY( OutpostMagic )
		NL_STRING_CONVERSION_TABLE_ENTRY( OutpostForage )
		NL_STRING_CONVERSION_TABLE_ENTRY( OutpostCraft )

		NL_STRING_CONVERSION_TABLE_ENTRY( ForageLocateDeposit )

		NL_STRING_CONVERSION_TABLE_ENTRY( TotemStatsHP )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemStatsSap )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemStatsSta )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemStatsFoc )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemRegenHP )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemRegenSap )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemRegenSta )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemRegenFoc )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemHarvestAgg )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemHarvestQty )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemHarvestZRs )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCraftSuc )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatRes )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatPar )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatCri )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatMagOff )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatMagDef )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatMROff )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatMRSpd )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatDS )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatArm )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemCombatPoZ )
		NL_STRING_CONVERSION_TABLE_ENTRY( TotemMiscMov )

		NL_STRING_CONVERSION_TABLE_ENTRY (Unknown)

	NL_END_STRING_CONVERSION_TABLE(TEffectFamily, EffectFamilyConversion, Unknown)

	TEffectFamily toEffectFamily(const std::string &str)
	{
		return EffectFamilyConversion.fromString(str);
	}

	const std::string &toString(TEffectFamily family)
	{
		return EffectFamilyConversion.toString(family);
	}

	TEffectFamily getDebuffResistEffect( DMGTYPE::EDamageType type)
	{
		switch( type )
		{
			case DMGTYPE::SLASHING : return DebuffResistSlash;
			case DMGTYPE::PIERCING : return DebuffResistPierce;
			case DMGTYPE::BLUNT	: return DebuffResistBlunt;
			case DMGTYPE::ROT : return DebuffResistRot;
			case DMGTYPE::ACID : return DebuffResistAcid;
			case DMGTYPE::COLD : return DebuffResistCold;
			case DMGTYPE::FIRE : return DebuffResistFire;
			case DMGTYPE::POISON : return DebuffResistPoison;
			case DMGTYPE::ELECTRICITY : return DebuffResistElectricity;
			case DMGTYPE::SHOCK : return DebuffResistSchock;
			default: break;
		}
		return Unknown;
	}

	TEffectFamily getCombatDoTEffect( DMGTYPE::EDamageType type)
	{
		switch( type )
		{
		case DMGTYPE::ROT : return CombatDoTRot;
		case DMGTYPE::ACID : return CombatDoTAcid;
		case DMGTYPE::COLD : return CombatDoTCold;
		case DMGTYPE::FIRE : return CombatDoTFire;
		case DMGTYPE::POISON : return CombatDoTPoison;
		case DMGTYPE::ELECTRICITY : return CombatDoTElectricity;
		case DMGTYPE::SHOCK : return CombatDoTShock;
		default: break;
		}
		return Unknown;
	}


	bool isEffectABonus(EFFECT_FAMILIES::TEffectFamily family)
	{
		if (family == EFFECT_FAMILIES::Hot)
			return true;

		if (family >= EFFECT_FAMILIES::BeginPowerEffects && family <= EFFECT_FAMILIES::EndPowerEffects)
			return true;

		if (family == EFFECT_FAMILIES::ProcShootAgain)
			return true;

		return false;
	}

	// The conversion table
	const NLMISC::CStringConversion<TEffectFamily>::CPair chatIdTable [] =
	{
		{ "MEZZ",			Mezz },
		{ "STUN",			Stun },
		{ "STUN",			CombatStun },
		{ "ROOT",			Root },
		{ "FEAR",			Fear },
		{ "BLIND",			Blind },
		{ "SLOW_MOVE",		CombatMvtSlow },
		{ "SLOW_MOVE",		SlowMove },
		{ "SLOW_ATTACKS",	CombatAttackSlow },
		{ "DEBUFF_COMBAT",	CombatDebuffCombatSkills },
		{ "DEBUFF_DEFENSE",	CombatDefenseModifier },
		{ "DEBUFF_DODGE",	CombatDebuffDodge },
		{ "INVULNERABILITY",	PowerInvulnerability },
		{ "MADNESS",		Madness },
		{ "STENCH",			Stench },

	};
	NLMISC::CStringConversion<TEffectFamily> conversion(chatIdTable, sizeof(chatIdTable) / sizeof(chatIdTable[0]),  Unknown);

	const std::string EmptyStr;
	const std::string UnknownStr = "Unknown";

	const std::string & getAssociatedChatId(EFFECT_FAMILIES::TEffectFamily family)
	{
		const std::string &str = conversion.toString(family);
		if (str == UnknownStr)
			return EmptyStr;
		else
			return str;
	}

	// conversion effect->sheetid
	const NLMISC::CStringConversion<TEffectFamily>::CPair sheetIdTable [] =
	{
		{ "mezz.sbrick",				Mezz },
		{ "stun.sbrick",				Stun },
		{ "stun.sbrick",				CombatStun },
		{ "root.sbrick",				Root },
		{ "fear.sbrick",				Fear },
		{ "blind.sbrick",				Blind },
		{ "snare.sbrick",				CombatMvtSlow },
		{ "snare.sbrick",				SlowMove },
		{ "slow.sbrick",				CombatAttackSlow },
		{ "slow.sbrick",				CombatSlow },
		{ "stench.sbrick",				Stench },

		{ "rot.sbrick",					CombatDoTRot },
		{ "cold.sbrick",				CombatDoTCold },
		{ "fire.sbrick",				CombatDoTFire },
		{ "electric.sbrick",			CombatDoTElectricity },
		{ "acid.sbrick",				CombatDoTAcid },
		{ "poison.sbrick",				CombatDoTPoison },
		{ "shockwave.sbrick",			CombatDoTShock },

		{ "debuff_dodge.sbrick",		CombatDebuffDodge },
		{ "debuff_combat_skills.sbrick",CombatDebuffCombatSkills },

		{ "shielding.sbrick",			PowerShielding },
		{ "life_aura.sbrick",			PowerLifeAura },
		{ "stamina_aura.sbrick",		PowerStaminaAura },
		{ "sap_aura.sbrick",			PowerSapAura },
		{ "umbrella_aura.sbrick",		PowerUmbrella },
		{ "melee_protection_aura.sbrick",PowerProtection },
		{ "anti_magic_shield_aura.sbrick",PowerAntiMagicShield },
		{ "invulnerability.sbrick",		PowerInvulnerability },
		{ "war_cry_aura.sbrick",		PowerWarCry },
		{ "fire_wall_aura.sbrick",		PowerFireWall },
		{ "thorn_wall_aura.sbrick",		PowerThornWall },
		{ "water_wall_aura.sbrick",		PowerWaterWall },
		{ "lightning_wall_aura.sbrick",	PowerLightningWall },
		{ "chg_charac.sbrick",			PowerChgCharac },
		{ "mod_defense.sbrick",					PowerModDefenseSkill },
		{ "mod_dodge.sbrick",					PowerModDodgeSkill },
		{ "mod_parry.sbrick",					PowerModParrySkill },
		{ "mod_craft_success.sbrick",			PowerModCraftSkill },
		{ "mod_melee_success.sbrick",			PowerModMeleeSkill },
		{ "mod_range_success.sbrick",			PowerModRangeSkill },
		{ "mod_magic_success.sbrick",			PowerModMagicSkill },
		{ "mod_forage_success.sbrick",			PowerModForageSkill },
		{ "mod_desert_forage_success.sbrick",		PowerModDesertForageSkill },
		{ "mod_forest_forage_success.sbrick",		PowerModForestForageSkill },
		{ "mod_lacustre_forage_success.sbrick",	PowerModLacustreForageSkill },
		{ "mod_jungle_forage_success.sbrick",		PowerModJungleForageSkill },
		{ "mod_primary_root_forage_success.sbrick",		PowerModPrimaryRootForageSkill },
		{ "mod_protect_magic.sbrick",	PowerModMagicProtection },
		{ "speeding_up.sbrick",			PowerSpeedingUp },
		{ "berserk.sbrick",				PowerBerserker},
		{ "enchant_weapon.sbrick",		PowerEnchantWeapon},

		{ "Unknown",			Unknown },
	};
	NLMISC::CStringConversion<TEffectFamily> conversionSheetID(sheetIdTable, sizeof(sheetIdTable) / sizeof(sheetIdTable[0]),  Unknown);

	/// get the sheetId associated to an effect to display on client interface
	NLMISC::CSheetId getAssociatedSheetId(TEffectFamily family)
	{
		const std::string &str  = conversionSheetID.toString(family);
		if ( !str.empty() && str != "Unknown")
			return NLMISC::CSheetId(str);
		else
			return NLMISC::CSheetId::Unknown;
	}

	RESISTANCE_TYPE::TResistanceType	getAssociatedResistanceType(TEffectFamily family)
	{
		switch(family)
		{
		case Blind:
		case MadnessMelee:
		case MadnessMagic:
		case MadnessRange:
		case Madness:
			return RESISTANCE_TYPE::Desert;
		case Mezz:
		case SlowMove:
			return RESISTANCE_TYPE::Forest;
		case Stun:
			return RESISTANCE_TYPE::Lacustre;
		case SlowMelee:
		case SlowRange:
		case SlowMagic:
		case SlowAttack:
		case Root:
			return RESISTANCE_TYPE::Jungle;
		case Fear:
			return RESISTANCE_TYPE::PrimaryRoot;
		default:
			return RESISTANCE_TYPE::None;
		};
	}

}; // EFFECT_FAMILIES

