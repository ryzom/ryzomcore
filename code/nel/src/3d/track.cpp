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

#include "nel/3d/track.h"

#include "nel/misc/rgba.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;

namespace NL3D
{

H_AUTO_DECL( NL3D_UTrack_interpolate )

#define	NL3D_HAUTO_UTRACK_INTERPOLATE			H_AUTO_USE( NL3D_UTrack_interpolate )


// ***************************************************************************

/*
// Some compilation check: force Visual to compile to template
CTrackDefaultFloat ttoto10;
CTrackDefaultVector ttoto11;
CTrackDefaultQuat ttoto12;
CTrackDefaultInt ttoto13;
CTrackDefaultRGBA ttoto16;
CTrackDefaultString ttoto14;
CTrackDefaultBool ttoto15;
*/

// ***************************************************************************
static CAnimatedValueBlock		AnimatedValueBlock;

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, float& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueFloat *value;
	value= dynamic_cast<const CAnimatedValueFloat*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, sint32& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueInt *value;
	value= dynamic_cast<const CAnimatedValueInt*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, CRGBA& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueRGBA *value;
	value= dynamic_cast<const CAnimatedValueRGBA*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, CVector& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueVector *value;
	value= dynamic_cast<const CAnimatedValueVector*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, CQuat& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueQuat *value;
	value= dynamic_cast<const CAnimatedValueQuat*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, std::string& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueString *value;
	value= dynamic_cast<const CAnimatedValueString*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, bool& res)
{
	NL3D_HAUTO_UTRACK_INTERPOLATE;

	// Evaluate it
	const CAnimatedValueBool *value;
	value= dynamic_cast<const CAnimatedValueBool*>(&eval (time, AnimatedValueBlock));

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

} // NL3D
