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



#ifndef RY_SPECIAL_MODIFIER_H
#define RY_SPECIAL_MODIFIER_H

/**
 * CSpecialModifiers
 * \author Fleury David
 * \author Nevrax France
 * \date 2002
 */
struct CSpecialModifiers
{
	enum ESpecialModifiers
	{
//		global_skill_modifier = 0,
		speed_variation_modifier,
		melee_attack_modifier_on_self,
		melee_attack_modifier_on_enemy,
		range_attack_modifier_on_self,
		attack_modifier_on_self,
		range_attack_modifier_on_enemy,
		magic_casting_modifier_on_self,
		magic_casting_modifier_on_enemy,
		chance_to_fail_strategy,
		chance_to_fail_spell,
		chance_to_fail_faber,
		chance_to_fail_harvest,
		chance_to_fail_tracking,
		melee_attack_slow,
		melee_slashing_damage_armor,
		melee_blunt_damage_armor,
		melee_piercing_damage_armor,
		melee_damage_modifier_factor,
		range_damage_modifier_factor,
		creature_melee_taken_damage_factor,
		creature_range_taken_damage_factor,
		combat_brick_latency_multiplier,
		magic_brick_latency_multiplier,
		armor_quality_modifier,
		weapon_quality_modifier,
		armor_absorbtion_multiplier,
		NUM_SPECIAL_MODIFIERS
	};

	/// variation of enemy's chance to hit (Melee) (-10 = -10% (ie, base chance = 90%-> final chance =81%) )
	sint32	MeleeAttackModifierOnEnemy;
	/// variation of entity chance to hit (Melee)
	sint32	MeleeAttackModifierOnSelf;
	/// variation of entity casting chance
	sint32	MagicCastingModifierOnSelf;
	/// variation of entity's enemies casting chance
	sint32	MagicCastingModifierOnEnemy;
	/// variation of enemy's chance to hit (Range) (-10 = -10% (ie, base chance = 90%-> final chance =81%) )
	sint32	RangeAttackModifierOnEnemy;
	/// variation of entity chance to hit (Range)
	sint32	RangeAttackModifierOnSelf;
	/// variation of entity chance to hit (Range)
	sint32	AttackModifierOnSelf;

	/// chance to fail magic/combat/faber sentences because of negative effects
	sint32	ChanceToFailStrategy;
	sint32	ChanceToFailSpell;
	sint32	ChanceToFailFaber;
	sint32	ChanceToFailHarvest;
	sint32	ChanceToFailTracking;
	
	/// increase melee attack latency
	sint32	MeleeAttackSlow;

	/// reduce Slashing damage in melee
	sint32	MeleeSlashingDamageArmor;
	/// reduce Blunt damage in melee
	sint32	MeleeBluntDamageArmor;
	/// reduce Piercing damage in melee
	sint32	MeleePiercingDamageArmor;

	/// Melee Damage modifier factor : all melee damage done by this entity are modified by this factor
	sint32	MeleeDamageModifierFactor;
	/// Melee Damage modifier factor : all melee damage done by this entity are modified by this factor
	sint32	RangeDamageModifierFactor;

	//\name modifiers on damage received by this entity
	//@{
	/// damage given by creatures in melee combat
	sint32	CreatureMeleeTakenDamageFactor;	
	/// damage given by creatures in range combat
	sint32	CreatureRangeTakenDamageFactor;	
	//}@
	// multiplier on combat brick latency
	sint32	CombatBrickLatencyMultiplier;
	// multiplier on magic brick latency
	sint32	MagicBrickLatencyMultiplier;
	// buff on the quality of the armor
	sint32	ArmorQualityModifier;
	// buff on the quality of the armor
	sint32	WeaponQualityModifier;
	// multiplier on armor absorbtion
	sint32	ArmorAbsorbtionMultiplier;

	/**
	 *	Default constructor
	 */
	CSpecialModifiers();

	
	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/**
	 * init all the special modifiers
	 */
	void init();
};

#endif // RY_SPECIAL_MODIFIER_H
/* special_modifier.h */
