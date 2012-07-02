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

#include "special_modifier.h"


///////////
// USING //
///////////
using namespace std;
using namespace NLMISC;

//-----------------------------------------------
// CSpecialModifier default constructor
//-----------------------------------------------
CSpecialModifiers::CSpecialModifiers()
{
	init();
} // CSpecialModifier //


//-----------------------------------------------
// CSpecialModifier init
//-----------------------------------------------
void CSpecialModifiers::init()
{
	MeleeAttackModifierOnEnemy	= 0;
	MeleeAttackModifierOnSelf	= 0;
	
	RangeAttackModifierOnEnemy	= 0;
	RangeAttackModifierOnSelf	= 0;
	
	MagicCastingModifierOnSelf	= 0;
	MagicCastingModifierOnEnemy	= 0;
	
	AttackModifierOnSelf		= 0;
	
	ChanceToFailStrategy		= 0;
	ChanceToFailSpell			= 0;
	ChanceToFailFaber			= 0;
	ChanceToFailHarvest			= 0;
	ChanceToFailTracking		= 0;
	
	MeleeSlashingDamageArmor	= 0;
	MeleeBluntDamageArmor		= 0;
	MeleePiercingDamageArmor	= 0;
	
	MeleeDamageModifierFactor	= 1000; /// 1000 * real factor, so 1000 = 1.0
	RangeDamageModifierFactor	= 1000; /// 1000 * real factor, so 1000 = 1.0
	
	CreatureMeleeTakenDamageFactor	= 1000; /// 1000 * real factor, so 1000 = 1.0
	CreatureRangeTakenDamageFactor	= 1000; /// 1000 * real factor, so 1000 = 1.0
	
	MeleeAttackSlow				= 0;
	
	CombatBrickLatencyMultiplier = 1;
	MagicBrickLatencyMultiplier = 1;
	
	ArmorQualityModifier		= 0;
	WeaponQualityModifier		= 0;
	
	ArmorAbsorbtionMultiplier = 100; /// 100 * real factor, so 100 = 1.0
	
} // CSpecialModifier init //



//-----------------------------------------------
// CSpecialModifiers::serial
//-----------------------------------------------
void CSpecialModifiers::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial( MeleeAttackModifierOnEnemy );
	f.serial( MeleeAttackModifierOnSelf );
	
	f.serial( RangeAttackModifierOnEnemy );
	f.serial( RangeAttackModifierOnSelf );
	
	f.serial( AttackModifierOnSelf );
	
	f.serial( MagicCastingModifierOnSelf );
	f.serial( MagicCastingModifierOnEnemy );
	
	f.serial( ChanceToFailStrategy );
	f.serial( ChanceToFailSpell );
	f.serial( ChanceToFailFaber );
	f.serial( ChanceToFailHarvest );
	f.serial( ChanceToFailTracking );
	
	f.serial( MeleeSlashingDamageArmor );
	f.serial( MeleeBluntDamageArmor );
	f.serial( MeleePiercingDamageArmor );
	
	f.serial( MeleeDamageModifierFactor );
	f.serial( RangeDamageModifierFactor );
	
	f.serial(CreatureMeleeTakenDamageFactor);
	f.serial(CreatureRangeTakenDamageFactor);
	
	f.serial( MeleeAttackSlow );
	
	f.serial(CombatBrickLatencyMultiplier);
	f.serial(MagicBrickLatencyMultiplier);
	
	f.serial(ArmorQualityModifier);
	f.serial(WeaponQualityModifier);
	
	f.serial(ArmorAbsorbtionMultiplier);
	
} // CSpecialModifiers::serial //

