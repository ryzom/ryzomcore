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



#ifndef CL_ATTACK_INFO
#define CL_ATTACK_INFO

#include "game_share/body.h"
#include "game_share/hit_type.h"

/** General a description of an attack. This describe parameters common to melee/magic/range/creature_attack attacks
  * This is filled when an attack behaviour is received
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 1/2004
  */
struct CAttackInfo
{
	uint				 Intensity;				  // intensity of attack
	uint				 PhysicalImpactIntensity; // intensity (for physical damages only)
	BODY::TBodyPart		 Localisation;            // localisation (for physical damages only)
	HITTYPE::THitType	 HitType;				  // if there are physical damages, tells how critical they are
	DMGTYPE::EDamageType DamageType;              // if there are physical damages, give their nature
	BODY::TSide			 Side;         // side of body that receives the impact
};



#endif
