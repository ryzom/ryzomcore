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
// 


#include "camera_animation_manager/camera_animation_player.h"
#include "camera_animation_manager/camera_animation_step_player_factory.h"
#include "time_client.h"
#include "view.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

CCameraAnimationPlayer* CCameraAnimationPlayer::_Instance = NULL;


CCameraAnimationPlayer::CCameraAnimationPlayer()
{
	_IsPlaying = false;
	_CurrStep = NULL;
	_ElapsedTimeForCurrStep = 0.f;
}

CCameraAnimationPlayer::~CCameraAnimationPlayer()
{
	stop();
}

void CCameraAnimationPlayer::start()
{
	if (isPlaying())
		stop();

	_IsPlaying = true;
}

void CCameraAnimationPlayer::stop()
{
	_IsPlaying = false;

	stopStep();
}

void CCameraAnimationPlayer::stopStep()
{
	// We release the step and modifiers
	if (_CurrStep)
	{
		// We first tell the step we stop it
		_CurrStep->stopStepAndModifiers();

		delete _CurrStep;
		_CurrStep = NULL;

		_ElapsedTimeForCurrStep = 0.f;
	}
}

void CCameraAnimationPlayer::playStep(const std::string& stepName, NLMISC::CBitMemStream& impulse)
{
	// We check if we are playing an animation
	if (!isPlaying())
	{
		nlwarning("CameraAnimationPlayer: animation not playing, cannot play step %s", stepName.c_str());
		return;
	}

	// We stop the current step if there is one
	stopStep();

	// We initialize the step with the factory
	_CurrStep = ICameraAnimationStepPlayerFactory::initStep(stepName, impulse);
	if (_CurrStep == NULL)
	{
		nlwarning("CameraAnimationPlayer: cannot create step player %s", stepName.c_str());
		_IsPlaying = false;
		return;
	}

	_ElapsedTimeForCurrStep = 0.f;
}

bool CCameraAnimationPlayer::isPlaying()
{
	return _IsPlaying;
}

TCameraAnimationInfo CCameraAnimationPlayer::update()
{
	// We get the current camera information
	NLMISC::CVector camLookAt = View.view();
	NLMISC::CVector camPos = View.viewPos();

	// We update the elapsed time for this step
	_ElapsedTimeForCurrStep += DT;

	TCameraAnimationInfo currCamInfo(camPos, camLookAt, _ElapsedTimeForCurrStep);

	if (!isPlaying())
		return currCamInfo;
	if (_CurrStep == NULL)
		return currCamInfo;

	// We update the current step
	currCamInfo = _CurrStep->updateStepAndModifiers(currCamInfo);

	return currCamInfo;
}
