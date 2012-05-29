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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
// misc
#include "nel/misc/path.h"
// Client
#include "animation_set.h"
#include "animation_misc.h"
#include "debug_client.h"
#include "client_cfg.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


////////////
// METHOD //
////////////
//-----------------------------------------------
// CAnimationSet :
// Constructor.
//-----------------------------------------------
CAnimationSet::CAnimationSet()
{
	_MaxDist		= 0.001;
	_Angle			= Pi/2.0;
	_AboutFaceAngle	= 3.0*Pi/4.0;
	_SpeedToWalk	= 4.0;
	_SpeedToRun		= 4.0;
	_WalkDist		= 0.0;
	_RunDist		= 0.0;
	_WalkLength		= 0.0;
	_RunLength		= 0.0;
	_Sheet = NULL;

}// CAnimationSet //

//-----------------------------------------------
// init
//-----------------------------------------------
void CAnimationSet::init(CAnimationSetSheet *sheet, NL3D::UAnimationSet *animationSet)
{
	_Sheet = sheet;
	_AnimationStates.resize(_Sheet->AnimationStates.size());
	for (uint32 i = 0; i < _AnimationStates.size(); ++i)
	{
		_AnimationStates[i].init(&_Sheet->AnimationStates[i], animationSet);
	}

	const CAnimationState *animState = getAnimationState (CAnimationStateSheet::Walk);
	if (animState)
	{
		// Compute the distance to break the idle (done according to the walk state).
		CAnimation::TAnimId walkId = animState->chooseAnim(0, EGSPD::CPeople::Unknown, GSGENDER::male);
		// Check the animation Id.
		if(walkId != CAnimation::UnknownAnim)
		{
			// Get the animation
			const CAnimation *walkAnim = animState->getAnimation(walkId);

			// Compute the dist made by the walk animation.
			CVector mov;
			if(CAnimationMisc::getAnimationMove(animationSet, walkAnim->id(), mov))
			{
				_WalkDist = fabs(mov.y);
				_MaxDist = ClientCfg.MinDistFactor*_WalkDist;
			}
			_WalkLength = CAnimationMisc::getAnimationLength(animationSet, walkAnim->id());

			animState = getAnimationState (CAnimationStateSheet::Run);
			if (animState)
			{
				// Get the run animation ID.
				CAnimation::TAnimId runId = animState->chooseAnim(0, EGSPD::CPeople::Unknown, GSGENDER::male);
				if(runId != CAnimation::UnknownAnim)
				{
					// Get the animation
					const CAnimation *runAnim = animState->getAnimation(runId);

					CVector mov;
					if(CAnimationMisc::getAnimationMove(animationSet, runAnim->id(), mov))
						_RunDist = fabs(mov.y);
					_RunLength = CAnimationMisc::getAnimationLength(animationSet, runAnim->id());

					// Get the walk average speed.
					double aveWalk	= CAnimationMisc::getAnimationAverageSpeed(animationSet, walkAnim->id());
					// Get the run average speed.
					double aveRun	= CAnimationMisc::getAnimationAverageSpeed(animationSet, runAnim->id());
					pushInfoStr(NLMISC::toString("Walk speed(%f).", aveWalk));
					pushInfoStr(NLMISC::toString("Run  speed(%f).", aveRun));

					// Check animations average speed for walk and run.
					if(aveRun<aveWalk)
						pushDebugStr(NLMISC::toString("Walk speed(%f) > Run speed(%f) !", aveWalk, aveRun));

					// Average Speed.
					double ave = (aveWalk+aveRun)/2.0;

					// Compute the min speed to run when walking.
					_SpeedToRun = (ave + aveRun)/2.0;
					// Compute the max speed to walk when running.
					_SpeedToWalk = (ave + aveWalk)/2.0;
				}
				// No animation found to run.
				else
					if(_Sheet->IsRunEssential)
						pushDebugStr("No animation found to run: speedToRun and speedToWalk will return the default value.");
			}
		}
		// No animation found to walk.
		else
			if(_Sheet->IsWalkEssential)
				pushDebugStr("No animation found to walk: maxDist, speedToRun and speedToWalk will return the default value.");
	}

	// Compute the angle after the one the character should turn (left/or right).
	animState = getAnimationState (CAnimationStateSheet::TurnLeft);
	if (animState)
	{
		CAnimation::TAnimId turnLeftId = animState->chooseAnim(0, EGSPD::CPeople::Unknown, GSGENDER::male);
		// Check the animation Id.
		if(turnLeftId != CAnimation::UnknownAnim)
		{
			// Get the animation
			const CAnimation *anim = animState->getAnimation(turnLeftId);
			if (anim)
			{
				CQuat currentAnimRotStart, currentAnimRotEnd;
				if(CAnimationMisc::interpolate(animationSet, anim->id(), 0.0, currentAnimRotStart))
				{
					double animLength = CAnimationMisc::getAnimationLength(animationSet, turnLeftId);
					if(CAnimationMisc::interpolate(animationSet, anim->id(), animLength, currentAnimRotEnd))
					{
						currentAnimRotStart.invert();
						CQuat currentAnimRot =  currentAnimRotStart * currentAnimRotEnd;
						_Angle = 0.75 * fabs(currentAnimRot.getAngle());
					}
				}
			}
		}
	}
}// init //

//-----------------------------------------------
// getAnimationStateByIndex
//-----------------------------------------------
CAnimationState *CAnimationSet::getAnimationStateByIndex(uint index)
{
	if (index >= _AnimationStates.size()) return NULL;
	return &_AnimationStates[index];

}// getAnimationStateByIndex //
