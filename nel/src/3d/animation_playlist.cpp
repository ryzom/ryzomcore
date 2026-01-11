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

#include "nel/3d/animation_playlist.h"
#include "nel/misc/common.h"
#include "nel/misc/stream.h"

using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

CAnimationPlaylist::CAnimationPlaylist()
{
	// Empty the playlist
	emptyPlayList ();

	// Set default wrap mode
	for (uint i=0; i<CChannelMixer::NumAnimationSlot; i++)
		_WrapMode[i]=Clamp;
}

// ***************************************************************************

void CAnimationPlaylist::emptyPlayList ()
{
	// Empty each slot
	for (uint i=0; i<CChannelMixer::NumAnimationSlot; i++)
	{
		_Animations[i]=empty;
		_SkeletonWeight[i]=empty;
		_InvertWeight[i]=false;
		_TimeOrigin[i]=0.f;
		_SpeedFactor[i]=1.f;
		_StartWeight[i]=1.f;
		_EndWeight[i]=1.f;
		_StartWeightTime[i]= 0.f;
		_EndWeightTime[i]= 0.f;
		_Smoothness[i]= 0.f;
	}
}

// ***************************************************************************

void CAnimationPlaylist::setAnimation (uint8 slot, uint animation)
{
	_Animations[slot]=animation;
}

// ***************************************************************************

uint CAnimationPlaylist::getAnimation (uint8 slot) const
{
	return _Animations[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted)
{
	_SkeletonWeight[slot]=skeletonId;
	_InvertWeight[slot]=inverted;
}

// ***************************************************************************

uint CAnimationPlaylist::getSkeletonWeight (uint8 slot, bool &inverted) const
{
	inverted=_InvertWeight[slot];
	return _SkeletonWeight[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setTimeOrigin (uint8 slot, TGlobalAnimationTime timeOrigin)
{
	_TimeOrigin[slot]=timeOrigin;
}

// ***************************************************************************

TGlobalAnimationTime CAnimationPlaylist::getTimeOrigin (uint8 slot) const
{
	return _TimeOrigin[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setSpeedFactor (uint8 slot, float speedFactor)
{
	_SpeedFactor[slot]=speedFactor;
}

// ***************************************************************************

float CAnimationPlaylist::getSpeedFactor (uint8 slot) const
{
	return _SpeedFactor[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setStartWeight (uint8 slot, float startWeight, TGlobalAnimationTime time)
{
	_StartWeight[slot]=startWeight;
	_StartWeightTime[slot]=time;
}

// ***************************************************************************

float CAnimationPlaylist::getStartWeight (uint8 slot, TGlobalAnimationTime& time) const
{
	time=_StartWeightTime[slot];
	return _StartWeight[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setEndWeight (uint8 slot, float endWeight, TGlobalAnimationTime time)
{
	_EndWeight[slot]=endWeight;
	_EndWeightTime[slot]=time;
}

// ***************************************************************************

float CAnimationPlaylist::getEndWeight (uint8 slot, TGlobalAnimationTime& time) const
{
	time=_EndWeightTime[slot];
	return _EndWeight[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setWeightSmoothness (uint8 slot, float smoothness)
{
	_Smoothness[slot]=smoothness;
}

// ***************************************************************************

float CAnimationPlaylist::getWeightSmoothness (uint8 slot) const
{
	return _Smoothness[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setupMixer (CChannelMixer& mixer, TGlobalAnimationTime time) const
{
	// For each slot
	for (uint8 s=0; s<CChannelMixer::NumAnimationSlot; s++)
	{
		// *** Set the time

		// Animation enabled
		bool enabled=true;

		// Get the animationSet pointer from the mixer
		const CAnimationSet *animSet=mixer.getAnimationSet ();

		// If is exists
		if (animSet)
		{
			if (_Animations[s]!=empty)
			{
				// Get the local time
				TAnimationTime wrappedTime = getLocalTime (s, time, *animSet);

				// Get the animation
				const CAnimation *pAnimation=animSet->getAnimation (_Animations[s]);

				// Disable mode ?
				if ((_WrapMode[s]==Disable)&&((wrappedTime<pAnimation->getBeginTime ())||(wrappedTime>pAnimation->getEndTime ())))
					enabled=false;

				// Set the time
				if (enabled)
					mixer.setSlotTime (s, wrappedTime);
			}
		}

		// *** Set the animation

		// Still enabled
		if (enabled)
		{
			// empty ?
			if (_Animations[s]==empty)
				// Empty the slot
				mixer.emptySlot (s);
			else
				// Set the animation id
				mixer.setSlotAnimation (s, _Animations[s]);

			// *** Set the skeleton weight

			// empty ?
			if (_SkeletonWeight[s]==empty)
				// Empty the slot
				mixer.resetSkeletonWeight (s);
			else
				// Set the animation id
				mixer.applySkeletonWeight (s, _SkeletonWeight[s], 	_InvertWeight[s]);

			// *** Set the weight
			mixer.setSlotWeight (s, getWeightValue (_StartWeightTime[s], _EndWeightTime[s], time, _StartWeight[s], _EndWeight[s], _Smoothness[s]));
		}
		else
		{
			// Disable this slot
			mixer.emptySlot (s);
		}
	}
}

// ***************************************************************************

float CAnimationPlaylist::getWeightValue (TGlobalAnimationTime startWeightTime, TGlobalAnimationTime endWeightTime, TGlobalAnimationTime time, float startWeight, float endWeight, float smoothness)
{
	// Clamp left
	if (time<=startWeightTime)
		return startWeight;
	// Clamp left
	if (time>=endWeightTime)
		return endWeight;

	// *** Interpolate

	// Linear value
	TGlobalAnimationTime linear=startWeight+(endWeight-startWeight)*(time-startWeightTime)/(endWeightTime-startWeightTime);

	// Linear ?
	if (smoothness<0.0001f)
		return (float)linear;

	// Quadratic value
	double a=2.f*startWeight-2.f*endWeight;
	double b=3.f*endWeight-3.f*startWeight;
	double x=(time-startWeightTime)/(endWeightTime-startWeightTime);
	double xSquare=x*x;
	double xCube=x*xSquare;
	double quad=a*xCube+b*xSquare+startWeight;

	// Interpolate between linear and quadratic
	return (float)(smoothness*quad+(1.f-smoothness)*linear);
}

// ***************************************************************************

void CAnimationPlaylist::setWrapMode (uint8 slot, TWrapMode wrapMode)
{
	_WrapMode[slot]=wrapMode;
}

// ***************************************************************************

CAnimationPlaylist::TWrapMode CAnimationPlaylist::getWrapMode (uint8 slot) const
{
	return _WrapMode[slot];
}

// ***************************************************************************

void CAnimationPlaylist::serial (NLMISC::IStream& f)
{
	// Serial a version
	(void)f.serialVersion (0);

	// Serial all the values
	for (uint i=0; i<CChannelMixer::NumAnimationSlot; i++)
	{
		f.serial (_Animations[i]);
		f.serial (_SkeletonWeight[i]);
		f.serial (_InvertWeight[i]);
		f.serial (_TimeOrigin[i]);
		f.serial (_SpeedFactor[i]);
		f.serial (_StartWeight[i]);
		f.serial (_StartWeightTime[i]);
		f.serial (_EndWeight[i]);
		f.serial (_EndWeightTime[i]);
		f.serial (_Smoothness[i]);
		f.serialEnum (_WrapMode[i]);
	}
}

// ***************************************************************************

TAnimationTime CAnimationPlaylist::getLocalTime (uint8 slot, TGlobalAnimationTime globalTime, const CAnimationSet& animSet) const
{
	// Get the animation
	const CAnimation *pAnimation=animSet.getAnimation (_Animations[slot]);

	// If this animation exists
	if (pAnimation)
	{
		// Compute the non-wrapped time
		TAnimationTime wrappedTime=pAnimation->getBeginTime ()+(TAnimationTime)((globalTime-_TimeOrigin[slot])*_SpeedFactor[slot]);

		// Wrap mode
		switch (_WrapMode[slot])
		{
		case Clamp:
			clamp (wrappedTime, pAnimation->getBeginTime (), pAnimation->getEndTime ());
			break;
		case Repeat:
			// Mod repeat the time
			{
				float length=pAnimation->getEndTime ()-pAnimation->getBeginTime();
				if (wrappedTime>=pAnimation->getBeginTime())
					wrappedTime=pAnimation->getBeginTime()+(float)fmod (wrappedTime-pAnimation->getBeginTime(), length);
				else
					wrappedTime=pAnimation->getBeginTime()+(float)fmod (wrappedTime-pAnimation->getBeginTime(), length)+length;
			}
			break;
			default: break;
		}

		// Return localTime
		return wrappedTime;
	}

	return (TAnimationTime)globalTime;
}

// ***************************************************************************

float CAnimationPlaylist::getLocalWeight (uint8 slot, TGlobalAnimationTime globalTime) const
{
	return getWeightValue (_StartWeightTime[slot], _EndWeightTime[slot], globalTime, _StartWeight[slot], _EndWeight[slot], _Smoothness[slot]);
}

// ***************************************************************************


} // NL3D
