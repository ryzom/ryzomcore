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

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

CCameraAnimationPlayer* CCameraAnimationPlayer::_Instance = NULL;


CCameraAnimationPlayer::CCameraAnimationPlayer()
{
	_IsPlaying = false;
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

	// We release the steps and modifiers
	for (std::vector<ICameraAnimationStepPlayer*>::iterator it = _Steps.begin(); it != _Steps.end(); ++it)
	{
		ICameraAnimationStepPlayer* step = *it;
		delete step;
	}
	_Steps.clear();
}

void CCameraAnimationPlayer::playStep(const std::string& stepName, NLMISC::CBitMemStream& impulse)
{
	// We check if we are playing an animation
	if (!isPlaying())
	{
		nlwarning("CameraAnimationPlayer: animation not playing, cannot play step %s", stepName.c_str());
		return;
	}

	// We initialize the step with the factory
	ICameraAnimationStepPlayer* step = ICameraAnimationStepPlayerFactory::initStep(stepName, impulse);
	if (step == NULL)
	{
		nlwarning("CameraAnimationPlayer: cannot create step player %s", stepName.c_str());
		return;
	}
	// We add the step to our list
	_Steps.push_back(step);

	// We start playing the step
	step->playStep();
}

bool CCameraAnimationPlayer::isPlaying()
{
	return _IsPlaying;
}