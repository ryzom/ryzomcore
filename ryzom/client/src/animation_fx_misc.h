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



#ifndef CL_ANIMATION_FX_MISC_H
#define CL_ANIMATION_FX_MISC_H

class CAnimationFXIDArray;

// misc. anim fxs
enum
{
	AnimFXRangeWeaponImpact1 = 0,
	AnimFXRangeWeaponImpact2 = 1,
	AnimFXRangeWeaponImpact3 = 2,
	AnimFXRangeWeaponImpact4 = 3,
	AnimFXRangeWeaponImpact5 = 4,
};

// gather various animation fx used in various places
extern CAnimationFXIDArray AnimFXMisc;

#endif
