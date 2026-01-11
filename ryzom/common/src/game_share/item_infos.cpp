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
#include "item_infos.h"


CItemInfos::CItemInfos()
{
	slotId= 0;
	versionInfo= 0;
	Hp= 0;
	HpMax= 0;
	CreatorName= 0;
	CurrentDamage= 0;
	MaxDamage= 0;
	HitRate= 0.0f;
	SapLoadCurrent= 0;
	SapLoadMax= 0;
	Range= 0.0f;
	ParryModifier= 0;
	DodgeModifier= 0;
	AdversaryParryModifier= 0;
	AdversaryDodgeModifier= 0;
	ProtectionFactor = 0.0f;
	MaxSlashingProtection = 0;
	MaxBluntProtection = 0;
	MaxPiercingProtection = 0;
	HpBuff = 0;
	SapBuff = 0;
	StaBuff = 0;
	FocusBuff = 0;
	WearEquipmentMalus = 0.0f;

	RequiredSkill = SKILLS::unknown;
	RequiredSkillLevel = 0;
	RequiredSkill2 = SKILLS::unknown;
	RequiredSkillLevel2 = 0;
	RequiredCharac = CHARACTERISTICS::Unknown;
	RequiredCharacLevel = 0;

	// magic focus factor
	for(uint i=0;i<NumMagicFactorType;i++)
	{
		CastingSpeedFactor[i]= 0.f;
		MagicPowerFactor[i]= 0.f;
	}

	// Protection factor
	for(uint i=0;i<MaxMagicProtectionByJewel;i++)
	{
		MagicProtection[i]= PROTECTION_TYPE::None;
		MagicProtectionFactor[i] = 0;
	}

	DesertMagicResistance = 0;
	ForestMagicResistance = 0;
	LacustreMagicResistance = 0;
	JungleMagicResistance = 0;
	PrimaryRootMagicResistance = 0;
	PetNumber = 0; // 1 based!
}

void CItemInfos::serial(NLMISC::IStream & s)
{
	s.serial( slotId );
	s.serial( versionInfo );
	s.serial( Hp );
	s.serial( HpMax );
	s.serial( CreatorName );
	s.serial( CurrentDamage );
	s.serial( MaxDamage );
	s.serial( HitRate );
	s.serial( SapLoadCurrent );
	s.serial( SapLoadMax );
	s.serial( Range );
	s.serial( ParryModifier );
	s.serial( DodgeModifier );
	s.serial( AdversaryParryModifier );
	s.serial( AdversaryDodgeModifier );
	s.serial( ProtectionFactor );
	s.serial( MaxSlashingProtection );
	s.serial( MaxBluntProtection );
	s.serial( MaxPiercingProtection );
	for(uint i=0;i<MaxMagicProtectionByJewel;i++)
	{
		s.serialEnum( MagicProtection[i] );
		s.serial( MagicProtectionFactor[i] );
	}
	s.serial( DesertMagicResistance );
	s.serial( ForestMagicResistance );
	s.serial( LacustreMagicResistance );
	s.serial( JungleMagicResistance );
	s.serial( PrimaryRootMagicResistance );
	s.serial( HpBuff );
	s.serial( SapBuff );
	s.serial( StaBuff );
	s.serial( FocusBuff );
	s.serial( Enchantment );
	s.serial( WearEquipmentMalus );

	s.serialEnum( RequiredSkill );
	s.serial( RequiredSkillLevel );
	s.serialEnum( RequiredSkill2 );
	s.serial( RequiredSkillLevel2 );
	s.serialEnum( RequiredCharac );
	s.serial( RequiredCharacLevel );

	s.serialCont( TypeSkillMods );

	for(uint i=0;i<NumMagicFactorType;i++)
	{
		s.serial(CastingSpeedFactor[i]);
		s.serial(MagicPowerFactor[i]);
	}
	s.serial( CustomText );
	s.serial( R2ItemDescription );
	s.serial( R2ItemComment );
	s.serial( PetNumber );
}

