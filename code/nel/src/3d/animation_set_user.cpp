// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std3d.h"

#include "nel/3d/animation_set_user.h"
#include "nel/3d/driver_user.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

UAnimation *CAnimationSetUser::getAnimation (uint animationId)
{

	return _AnimationSet->getAnimation (animationId);
}

// ***************************************************************************

const CAnimationSet* CAnimationSetUser::getAnimationSet () const
{

	return _AnimationSet;
}

// ***************************************************************************
void CAnimationSetUser::setAnimationSampleDivisor(uint sampleDivisor)
{
	_AnimationSet->setAnimationSampleDivisor(sampleDivisor);
}

// ***************************************************************************
uint CAnimationSetUser::getAnimationSampleDivisor() const
{
	return _AnimationSet->getAnimationSampleDivisor();
}

// ***************************************************************************
void CAnimationSetUser::build ()
{

	// build
	_AnimationSet->build ();

	// and preload all SSS shapes that can be spawned during animation
	nlassert(_Owner->getDriver() && _Owner->getShapeBank());
	_AnimationSet->preloadSSSShapes(*_Owner->getDriver(), ((CShapeBankUser*)_Owner->getShapeBank())->_ShapeBank);
}


} // NL3D
