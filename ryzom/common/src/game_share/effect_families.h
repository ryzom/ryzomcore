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



#ifndef RY_EFFECT_FAMILIES_H
#define RY_EFFECT_FAMILIES_H

#include "nel/misc/types_nl.h"
#include "damage_types.h"
#include "resistance_type.h"

namespace EFFECT_FAMILIES
{
	enum TEffectFamily
	{
		BeginDeathEffects = 0,
			DebuffCharacteristic = BeginDeathEffects,
		EndDeathEffects = DebuffCharacteristic,

		BeginMiscEffects,
			Teleport = BeginMiscEffects,
			ProcShootAgain,
		EndMiscEffects = ProcShootAgain,

		BeginMagicEffects,
			Hot = BeginMagicEffects,
			Dot,
			DoTStackable,

			Stench,
			Bounce,

			RedirectAttacks,
			ReflectDamage,
			ReverseDamage,

			BeginSickness,
				DebuffSkillMelee = BeginSickness,
				DebuffSkillMagic,
				DebuffSkillRange,
				// those last 3 don't exist anymore as player spells

				SlowMove, // Snare
				Snare, // new one

				SlowMelee, // Slow
				SlowMagic,
				SlowRange,
				//->SlowAttackAndCast, //range,melee and magic
				SlowAttack, // new one

				MadnessMelee,
				MadnessMagic,
				MadnessRange,
				//-> Madness,
				Madness,

				DebuffResistBlunt,
				DebuffResistSlash,
				DebuffResistPierce,
				DebuffResistAcid,
				DebuffResistRot,
				DebuffResistCold,
				DebuffResistFire,
				DebuffResistPoison,
				DebuffResistSchock,
				DebuffResistElectricity,
				// there's no more debuff resist
			EndSickness = DebuffResistElectricity,

			BeginCurse,
				Mezz = BeginCurse, // -> sleep
				Stun,
				Root,
				Fear,
				Blind,
			EndCurse = Blind,

			BeginHatred ,
				FaunaHatred =BeginHatred,
				PlantHatred,
				KitinHatred,
				HominHatred,
				DegeneratedHatred,
				PetHatred,
				KamiHatred,
				AllHatred,
				// there's no more hatred for players (AI only send AllHatred)
			EndHatred = AllHatred,

			Invincibility, // used by some bosses

		EndMagicEffects = Invincibility,

		BeginCombatEffect,
			CombatBleed = BeginCombatEffect,
			CombatStun,
			CombatMvtSlow,
			CombatAttackSlow,
			CombatCastSlow,
			CombatSlow,
			CombatDefenseModifier,
			CombatDebuffCombatSkills,
			CombatDebuffDodge,

			CombatDebuffStaminaRegen,
			CombatDebuffSapRegen,
			CombatDebuffHitPointsRegen,
			CombatDebuffFocusRegen,

			CombatDoTRot,
			CombatDoTCold,
			CombatDoTFire,
			CombatDoTElectricity,
			CombatDoTAcid,
			CombatDoTPoison,
			CombatDoTShock,

		EndCombatEffect = CombatDoTShock,

		BeginPowerEffects,
			PowerShielding = BeginPowerEffects,
			PowerLifeAura,
			PowerStaminaAura,
			PowerSapAura,
			PowerUmbrella,
			PowerProtection,
			PowerAntiMagicShield,
			PowerInvulnerability,
			PowerWarCry,
			PowerFireWall,
			PowerThornWall,
			PowerWaterWall,
			PowerLightningWall,

			PowerRootLifeAura,
			PowerRootStaminaAura,
			PowerRootSapAura,
			PowerRootUmbrella,
			PowerRootProtection,
			PowerRootAntiMagicShield,
			PowerRootWarCry,
			PowerRootFireWall,
			PowerRootThornWall,
			PowerRootWaterWall,
			PowerRootLightningWall,

			PowerChgCharac,
			PowerModDefenseSkill,
			PowerModDodgeSkill,
			PowerModParrySkill,
			PowerModCraftSkill,
			PowerModMeleeSkill,
			PowerModRangeSkill,
			PowerModMagicSkill,
			PowerModForageSkill,
			PowerModDesertForageSkill,
			PowerModForestForageSkill,
			PowerModLacustreForageSkill,
			PowerModJungleForageSkill,
			PowerModPrimaryRootForageSkill,
			PowerModMagicProtection,
			PowerSpeedingUp,
			PowerBerserker,
			PowerEnchantWeapon,
		EndPowerEffects = PowerEnchantWeapon,

		BeginOutpostEffects,
			OutpostCombat = BeginOutpostEffects,
			OutpostMagic,
			OutpostForage,
			OutpostCraft,
		EndOutpostEffects = OutpostCraft,

		BeginForageEffects,
			ForageLocateDeposit = BeginForageEffects,
		EndForageEffects = ForageLocateDeposit,

		BeginTotemEffects,
			// stats
			TotemStatsHP = BeginTotemEffects,
			TotemStatsSap,
			TotemStatsSta,
			TotemStatsFoc,

			// regen
			TotemRegenHP,
			TotemRegenSap,
			TotemRegenSta,
			TotemRegenFoc,

			// harvest
			TotemHarvestAgg,
			TotemHarvestQty,
			TotemHarvestZRs,

			// craft
			TotemCraftSuc,

			// combat
			TotemCombatRes,
			TotemCombatPar,
			TotemCombatCri,
			TotemCombatMagOff,
			TotemCombatMagDef,
			TotemCombatMROff,
			TotemCombatMRSpd,
			TotemCombatDS,
			TotemCombatArm,
			TotemCombatPoZ,

			// misc
			TotemMiscMov,
		EndTotemEffects = TotemMiscMov,

		NbEffectFamilies,
		Unknown = NbEffectFamilies,
	};

	/**
	 * get the right effect family from the input string
	 * \param str the input string
	 * \return the TEffectFamily associated to this string (Unknown if the string cannot be interpreted)
	 */
	TEffectFamily toEffectFamily(const std::string &str);

	// get the debuff resist effect corresponding to the damage type
	TEffectFamily getDebuffResistEffect( DMGTYPE::EDamageType );

	// get the combat DoT effect associated to damage type
	TEffectFamily getCombatDoTEffect( DMGTYPE::EDamageType );

	/// convert an effect family to a string
	const std::string & toString(TEffectFamily family);

	/// get the sheetId associated to an effect to display on client interface
	NLMISC::CSheetId getAssociatedSheetId(TEffectFamily family);

	/// return true if the effect is a positive one, return false if it's a negative effect
	bool isEffectABonus(TEffectFamily family);

	/// get the string to use for chat messages (empty if no associated messages or special params)
	const std::string & getAssociatedChatId(TEffectFamily family);

	/// Return the ResistanceType associated to this effect family
	RESISTANCE_TYPE::TResistanceType	getAssociatedResistanceType(TEffectFamily family);

}; // EFFECT_FAMILIES

#endif // RY_EFFECT_FAMILIES_H
/* End of effect_families.h */
