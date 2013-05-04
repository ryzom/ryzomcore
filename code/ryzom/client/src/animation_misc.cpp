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

/////////////
// INCLUDE //
/////////////
// client
#include "animation_misc.h"
#include "animation.h"
// 3d
#include "nel/3d/u_track.h"


///////////
// USING //
///////////
using namespace NL3D;


////////////
// METHOD //
////////////
//---------------------------------------------------
// CAnimationMisc :
// Constructor.
//---------------------------------------------------
CAnimationMisc::CAnimationMisc()
{
}// CAnimationMisc //

//---------------------------------------------------
// interpolate :
// Function to get the position in animation at timeOffset.
// \param idAnim : id of the animation.
// \param timeOffset : time for the interpolation.
// \param result : is the reference on the value to get the result (position).
// \return bool : true if the parameter result is valid.
//---------------------------------------------------
bool CAnimationMisc::interpolate(UAnimationSet *animationSet, uint idAnim, double timeOffset, NLMISC::CVector &result)
{
	// Get the animation set.
	if(!animationSet)
	{
		nlwarning("CAnimationMisc::interpolate(CVector) : AnimationSet not allocated.");
		return false;
	}

	// Check the Animation
	if(idAnim == UAnimationSet::NotFound)
		return false;

	// Get the animation pointer.
	UAnimation *anim = animationSet->getAnimation(idAnim);
	if(!anim)
	{
		nlwarning("CAnimationMisc::interpolate(CVector) : animationSet.getAnimation(%d) return NULL.", idAnim);
		return false;
	}

	// Get the track for the position.
	UTrack* Track = anim->getTrackByName("pos");
	if(!Track)
	{
		nlwarning("CAnimationMisc::interpolate(CVector) : track with the name 'pos' does not exist.");
		return false;
	}

	// Get the result.
	return Track->interpolate((CAnimationTime)timeOffset, result);
}// interpolate //

//---------------------------------------------------
// interpolate :
// Function to get the rotation in an animation at timeOffset.
// \param animationSet : search the animation in this set.
// \param idAnim : id of the animation.
// \param timeOffset : time for the interpolation.
// \param result : is the reference on the value to get the result (rotation).
// \return bool : true if the parameter result is valid.
//---------------------------------------------------
bool CAnimationMisc::interpolate(UAnimationSet *animationSet, uint idAnim, double timeOffset, NLMISC::CQuat &result)
{
	// Get the animation set.
	if(!animationSet)
	{
		nlwarning("CAnimationMisc::interpolate(CQuat) : AnimationSet not allocated.");
		return false;
	}

	// Check the Animation
	if(idAnim == UAnimationSet::NotFound)
		return false;

	// Get the animation pointer.
	UAnimation *anim = animationSet->getAnimation(idAnim);
	if(!anim)
	{
		nlwarning("CAnimationMisc::interpolate(CQuat) : animationSet.getAnimation(%d) return NULL.", idAnim);
		return false;
	}

	// Get the track for the position.
	UTrack* Track = anim->getTrackByName("rotquat");
	if(!Track)
	{
		//nlwarning("CAnimationMisc::interpolate(CQuat) : track with the name 'PathRotQuat' or 'rotquat' does not exist.");
		return false;
	}

	// Get the result.
	return Track->interpolate((CAnimationTime)timeOffset, result);
}// interpolate //


//---------------------------------------------------
// getAnimationLength :
// Return an animation length (in sec).
// \param animationSet : search the animation in this set.
// \param string animName : Animation Name.
// \return double : the length of the animation or 0 if any pb.
// \warning This Method is slower than the one with the animation Id instead of the animation Name.
//---------------------------------------------------
double CAnimationMisc::getAnimationLength(UAnimationSet *animationSet, const std::string &animName)
{
	// Initialize the length to 0.0 like if there is an error.
	double length = 0.0;

	if(animationSet)
	{
		// Get the animation Id.
		uint idAnim = animationSet->getAnimationIdByName(animName);
		if(idAnim != UAnimationSet::NotFound)
			length = getAnimationLength(animationSet, idAnim);
	}

	// Return the length of the animation or 0 is any pb.
	return length;
}// getAnimationLength //

//---------------------------------------------------
// getAnimationLength :
// Return an animation length (in sec).
// \param animationSet : search the animation in this set.
// \param idAnim : id of the animation.
// \return double : the length of the animation or 0 if any pb.
//---------------------------------------------------
double CAnimationMisc::getAnimationLength(UAnimationSet *animationSet, uint idAnim)
{
	// Initialize the length to 0.0 like if there is an error.
	double length = 0.0;

	// Check _AnimationSet.
	if(animationSet)
	{
		// Check idAnim
		if(idAnim != UAnimationSet::NotFound)
		{
			// Get the animation pointer and get the length.
			UAnimation *anim = animationSet->getAnimation(idAnim);
			if(anim)
				length = anim->getEndTime() - anim->getBeginTime();
		}
	}

	// Return the length of the animation or 0 is any pb.
	return length;
}// getAnimationLength //


//---------------------------------------------------
// getAnimationAverageSpeed :
// Get the average speed of an animation (in meters/sec).
// \param animationSet : search the animation in this set.
// \param string animName : Animation Name.
// \return double : the average speed (in m/s).
//---------------------------------------------------
double CAnimationMisc::getAnimationAverageSpeed(UAnimationSet *animationSet, const std::string &animName)
{
	if(animationSet)
	{
		// Get the animation Id.
		uint idAnim = animationSet->getAnimationIdByName(animName);
		if(idAnim != UAnimationSet::NotFound)
			return getAnimationAverageSpeed(animationSet, idAnim);
	}
	else
		nlwarning("CAnimationMisc::getAnimationAverageSpeed : animationSet is NULL");

	// Return the animation average speed.
	return 0.0;
}// getAnimationAverageSpeed //

//---------------------------------------------------
// getAnimationAverageSpeed :
// Get the average speed of an animation (in meters/sec).
// \param animationSet : search the animation in this set.
// \param idAnim : id of the animation.
// \return double : the average speed (in m/s).
//---------------------------------------------------
double CAnimationMisc::getAnimationAverageSpeed(UAnimationSet *animationSet, uint idAnim)
{
	// Initialize the length to 0.0 like if there is an error.
	double length = getAnimationLength(animationSet, idAnim);

	// Check the length.
	if(length <= 0.0)
	{
		nlwarning("CEntityAnimationManager::getAnimationAverageSpeed : length <= 0.0 -> return speed = 0.0");
		return 0.0;
	}

	// Get the distance done by the animation.
	double move = 0.0;
	NLMISC::CVector	startPos, endPos;
	if(interpolate(animationSet, idAnim, 0.0, startPos))
	{
		if(interpolate(animationSet, idAnim, length, endPos))
		{
			NLMISC::CVector mov = endPos - startPos;
			move = mov.norm();
		}
	}

	// Return the animation average speed.
	return (move / length);
}// getAnimationAverageSpeed //


//---------------------------------------------------
// getAnimationRotation :
// Get the rotation done by the animation (in radian).
// \param animationSet : search the animation in this set.
// \param string animName : Animation Name.
// \return double : the rotation (in radian).
//---------------------------------------------------
double CAnimationMisc::getAnimationRotation(NL3D::UAnimationSet *animationSet, const std::string &animName)
{
	if(animationSet)
	{
		// Get the animation Id.
		uint idAnim = animationSet->getAnimationIdByName(animName);
		if(idAnim != UAnimationSet::NotFound)
			return getAnimationRotation(animationSet, idAnim);
	}
	else
		nlwarning("CAnimationMisc::getAnimationRotation : animationSet is NULL");

	// Return the animation average speed.
	return 0.0;
}// getAnimationRotation //

//---------------------------------------------------
// getAnimationRotation :
// Get the rotation done by the animation (in radian).
// \param animationSet : search the animation in this set.
// \param idAnim : id of the animation.
// \return double : the rotation (in radian).
//---------------------------------------------------
double CAnimationMisc::getAnimationRotation(NL3D::UAnimationSet *animationSet, uint idAnim)
{
	// Check the animation Id.
	if(idAnim != CAnimation::UnknownAnim)
	{
		CQuat currentAnimRotStart, currentAnimRotEnd;
		if(CAnimationMisc::interpolate(animationSet, idAnim, 0.0, currentAnimRotStart))
		{
			double animLength = CAnimationMisc::getAnimationLength(animationSet, idAnim);
			if(CAnimationMisc::interpolate(animationSet, idAnim, animLength, currentAnimRotEnd))
			{
				currentAnimRotStart.invert();
				CQuat currentAnimRot =  currentAnimRotStart * currentAnimRotEnd;
				return fabs(currentAnimRot.getAngle());
			}
		}
	}

	return 0.f;
}// getAnimationRotation //

//-----------------------------------------------
// Get the vector covered by the given animation.
// \param animationSet : search the animation in this set.
// \param idAnim : id of the animation.
// \param move : filled with the vector of the move if possible
// \return bool : true if 'move' has been filled, else false.
//-----------------------------------------------
bool CAnimationMisc::getAnimationMove(NL3D::UAnimationSet *animationSet, uint idAnim, NLMISC::CVector &move)	// static
{
	CVector startPos, endPos;
	if(CAnimationMisc::interpolate(animationSet, idAnim, 0.0, startPos))
	{
		double animLength = CAnimationMisc::getAnimationLength(animationSet, idAnim);
		if(animLength)
		{
			if(CAnimationMisc::interpolate(animationSet, idAnim, animLength, endPos))
			{
				// Well Done.
				move = endPos - startPos;
				return true;
			}
		}
	}

	// Something Wrong -> return false.
	return false;
}// getAnimationMove //
