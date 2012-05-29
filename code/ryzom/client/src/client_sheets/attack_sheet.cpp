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
#include "attack_sheet.h"
//
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_loader.h"


// *********************************************************************************************************
CAttackSheet::CAttackSheet()
{
	ProjectileDelay = 0.f;
	ProjectileMode = MAGICFX::Bomb;
	PlayImpactFXOnlyOnMainTarget = false;
	PlayImpactAnim =  true;
	LetProjectileStickedOnTarget = false;
	ForceUseProjectileDelay = false;
	ApplyItemOffsetToWeaponBone = true;
	AdditionnalStartOffset.set(0.f, 0.f, 0.f);
}

// *********************************************************************************************************
void CAttackSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	bool ok = true;
	ok &= item.getValueByName(AttackBeginFX, (prefix + "AttackBeginFX").c_str());
	ok &= item.getValueByName(AttackLoopFX, (prefix + "AttackLoopFX").c_str());
	ok &= item.getValueByName(AttackEndFX, (prefix + "AttackEndFX").c_str());
	ok &= item.getValueByName(AttackStaticObjectCastFX, (prefix + "AttackStaticObjectCastFX").c_str());
	ok &= item.getValueByName(AttackFailFX, (prefix + "AttackFailFX").c_str());
	ok &= item.getValueByName(ProjectileFX, (prefix + "ProjectileFX").c_str());
	ok &= item.getValueByName(ImpactFX, (prefix + "ImpactFX").c_str());
	//
	ok &= item.getValueByName(ProjectileDelay, (prefix + "ProjectileDelay").c_str());
	ok &= item.getValueByName(StaticObjectProjectileDelay, (prefix + "StaticObjectProjectileDelay").c_str());
	uint32 projectileMode = 0;
	ok &= item.getValueByName(projectileMode, (prefix + "ProjectileMode").c_str());
	ProjectileMode = (MAGICFX::TSpellMode) projectileMode;
	ok &= item.getValueByName(IsImpactLocalised, (prefix + "IsImpactLocalised").c_str());
	ok &= item.getValueByName(PlayImpactFXOnlyOnMainTarget, (prefix + "PlayImpactFXOnlyOnMainTarget").c_str());
	ok &= item.getValueByName(PlayImpactAnim, (prefix + "PlayImpactAnim").c_str());
	ok &= DefaultCastingPoint.build(item, prefix + "DefaultCastingPoint.");
  	ok &= DefaultAimingPoint.build(item, prefix + "DefaultAimingPoint.");
	ok &= item.getValueByName(LetProjectileStickedOnTarget, (prefix + "LetProjectileStickedOnTarget").c_str());
	ok &= item.getValueByName(ForceUseProjectileDelay, (prefix + "ForceUseProjectileDelay").c_str());
	ok &= item.getValueByName(ApplyItemOffsetToWeaponBone, (prefix + "ApplyItemOffsetToWeaponBone").c_str());
	ok &= item.getValueByName(AdditionnalStartOffset.x, (prefix + "AdditionnalStartOffsetX").c_str());
	ok &= item.getValueByName(AdditionnalStartOffset.y, (prefix + "AdditionnalStartOffsetY").c_str());
	ok &= item.getValueByName(AdditionnalStartOffset.z, (prefix + "AdditionnalStartOffsetZ").c_str());
	if (!ok)
	{
		nlwarning("Couldn't read all fields");
	}
}

// *********************************************************************************************************
void CAttackSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialEnum(ProjectileMode);
	f.serial(AttackBeginFX);
	f.serial(AttackLoopFX);
	f.serial(AttackEndFX);
	f.serial(AttackStaticObjectCastFX);
	f.serial(AttackFailFX);
	f.serial(ProjectileFX);
	f.serial(ImpactFX);
	f.serial(ProjectileDelay);
	f.serial(StaticObjectProjectileDelay);
	f.serial(PlayImpactFXOnlyOnMainTarget);  // Only may target play the impact fx (but other target play the impact anim)
	f.serial(IsImpactLocalised);
	f.serial(DefaultCastingPoint);
	f.serial(DefaultAimingPoint);
	f.serial(PlayImpactAnim);
	f.serial(LetProjectileStickedOnTarget);
	f.serial(ForceUseProjectileDelay);
	f.serial(ApplyItemOffsetToWeaponBone);
	f.serial(AdditionnalStartOffset);
}
