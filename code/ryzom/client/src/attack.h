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



#ifndef CL_CREATURE_ATTACK_H
#define CL_CREATURE_ATTACK_H

#include "animation_fx.h"
#include "animation_fx.h"
//
#include "nel/misc/string_conversion.h"

namespace NL3D
{
	class UAnimationSet;
}


class CCreatureAttackSheet;

/** Description of a creature attack
  */
class CCreatureAttack
{
public:
	// pointer on base sheet
	const CCreatureAttackSheet *Sheet;
	// anim fx set for attack part
	CAnimationFXSet AttackFXSet;
	// anim fx set for custom projectile part (whether custom projectile is used is told in 'Sheet');
	CAnimationFXSet ProjectileFXSet;
	// anim fx set for custom impact part (whether custom impact is used is told in 'Sheet');
	CAnimationFXSet ImpactFXSet;
public:
	// ctor
	CCreatureAttack();
	// Init from parent sheet
	void init(const CCreatureAttackSheet *sheet, NL3D::UAnimationSet *as);
};

/** gather all creature attack in a single place
  */
class CCreatureAttackDictionnary
{
public:
	// get the unique instance of this class
	static CCreatureAttackDictionnary &getInstance();
	/** This :
	  * - Init an animation set.
	  * - Build all creature attacks datas and build their tracks.
	  */
	void init();
	void release();
	// get creature attack by its name (case unsensitive)
	const CCreatureAttack *getAttackByName(const std::string &name) const;
private:
	typedef std::map<std::string,
		             CCreatureAttack,
					 NLMISC::CUnsensitiveStrLessPred> TAttackMap;
	NL3D::UAnimationSet					*_AnimationSet;
	static	CCreatureAttackDictionnary	*_Instance;
	TAttackMap							_AttackMap;
};



#endif
