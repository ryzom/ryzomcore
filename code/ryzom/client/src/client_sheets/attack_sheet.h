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



#ifndef CL_ATTACK_SHEET_H
#define CL_ATTACK_SHEET_H

#include "entity_sheet.h"
#include "fx_stick_mode.h"
//
#include "game_share/magic_fx.h"
//
#include "nel/misc/uv.h"


class CAttackSheet
{
public:
	MAGICFX::TSpellMode	ProjectileMode;
	std::string			AttackBeginFX;  // .animation_fx_set
	std::string			AttackLoopFX;   // .animation_fx_set
	std::string			AttackEndFX;    // .animation_fx_set
	std::string			AttackStaticObjectCastFX;    // .animation_fx_set
	std::string			AttackFailFX;   // .animation_fx_set
	std::string			ProjectileFX;
	std::string			ImpactFX;
	float				ProjectileDelay;
	float				StaticObjectProjectileDelay;
	bool				IsImpactLocalised;				  // is the impact localised ?
	                                                  // if so, the localisation is the same for aiming point, main impact, and physical impact, and is the localisation of the physical impact
	                                                  // if not : projectile aiming point is given by "DefaultAimingPoint", physical damage uses its localisation, and main impact uses default localisation of fx.

	CFXStickMode		DefaultCastingPoint;
	CFXStickMode		DefaultAimingPoint;
	bool				PlayImpactFXOnlyOnMainTarget; // Only may target play the impact fx (but other target play the impact anim)
	bool				PlayImpactAnim;
	bool				LetProjectileStickedOnTarget;
	bool				ForceUseProjectileDelay;
	bool				ApplyItemOffsetToWeaponBone;

	NLMISC::CVector     AdditionnalStartOffset;
public:
	// ctor
	CAttackSheet();
	virtual void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};









#endif
