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

#include "camera_animation_manager/camera_animation_step_player_factory.h"

std::vector<std::pair<std::string, ICameraAnimationStepPlayerFactory*> >* ICameraAnimationStepPlayerFactory::Entries;

ICameraAnimationStepPlayer* ICameraAnimationStepPlayerFactory::initStep(const std::string& name, NLMISC::CBitMemStream& impulse)
{
	if (!Entries)
		return NULL;

	// We search the correct step type in our entries
	for (uint i = 0; i < Entries->size(); i++)
	{
		if (name == (*Entries)[i].first)
		{
			ICameraAnimationStepPlayer* ret = (*Entries)[i].second->instanciate();
			if (!ret)
			{
				nlwarning("BUG IN CAMERA ANIMATION STEP PLAYER FACTORY : BAD INIT CODE %s", name.c_str());
				return NULL;
			}
			// We init the step
			if (!ret->initStep(impulse))
			{
				nlwarning("building camera animation step player failed %s", name.c_str());
				delete ret;
				return NULL;
			}

			// We look for children (modifiers or sound triggers)
			uint8 nbModifiers = 0;
			impulse.serial(nbModifiers);
			for (uint8 i = 0; i < nbModifiers; i++)
			{
				// We get the modifier name
				std::string modifierName = "";
				impulse.serial(modifierName);

				ICameraAnimationModifierPlayer* modifier = ICameraAnimationModifierPlayerFactory::initModifier(modifierName, impulse);
				// We add the modifier to the list
				if (modifier)
				{
					ret->addModifier(modifier);
				}
				else
				{
					nlwarning("impossible to init the modifier named %s in step named %s", modifierName.c_str(), name.c_str());
				}
			}

			return ret;
		}
	}
	return NULL;
}

void ICameraAnimationStepPlayerFactory::init()
{
	if (!Entries)
		Entries = new std::vector<std::pair<std::string, ICameraAnimationStepPlayerFactory*> >;
}

ICameraAnimationStepPlayer::~ICameraAnimationStepPlayer()
{
	// We release all the modifiers
	for (std::vector<ICameraAnimationModifierPlayer*>::iterator it = Modifiers.begin(); it != Modifiers.end(); ++it)
	{
		ICameraAnimationModifierPlayer* modifier = *it;
		delete modifier;
	}
	Modifiers.clear();
}

void ICameraAnimationStepPlayer::addModifier(ICameraAnimationModifierPlayer* modifier)
{
	Modifiers.push_back(modifier);
}

TCameraAnimationInfo ICameraAnimationStepPlayer::updateStepAndModifiers(const TCameraAnimationInfo& currCamInfo)
{
	// Updates the step
	TCameraAnimationInfo newInfo = updateStep(currCamInfo);

	// Updates the modifiers
	for (std::vector<ICameraAnimationModifierPlayer*>::iterator it = Modifiers.begin(); it != Modifiers.end(); ++it)
	{
		ICameraAnimationModifierPlayer* modifier = *it;
		
		// We update the modifier
		newInfo = modifier->updateModifier(newInfo);
	}
	return newInfo;
}

void ICameraAnimationStepPlayer::stopStepAndModifiers()
{
	// Stops the step
	stopStep();

	// Stops the modifiers
	for (std::vector<ICameraAnimationModifierPlayer*>::iterator it = Modifiers.begin(); it != Modifiers.end(); ++it)
	{
		ICameraAnimationModifierPlayer* modifier = *it;

		// We stop the modifier
		modifier->stopModifier();
	}
}

NLMISC::CVector ICameraAnimationStepPlayer::computeCurrentLookAtDir(float ratio, const NLMISC::CVector& currPos, const NLMISC::CVector& startLookAtDir,
																	const NLMISC::CVector& endLookAtDir)
{
	// We normalize the start look at direction
	NLMISC::CVector startDir = startLookAtDir;
	startDir.normalize();

	// We normalize the final look at direction
	NLMISC::CVector finalDir = endLookAtDir;
	finalDir.normalize();

	// We compute a vector that goes from the starting look at dir to the final look at dir
	NLMISC::CVector startToFinal = finalDir - startDir;

	// We multiply this vector by the ratio so that we can have a vector that represent the current position we are looking at
	startToFinal = startToFinal * ratio;

	// We compute the position we are looking at
	NLMISC::CVector currLookAtPos = startDir + startToFinal;

	// We compute the direction
	NLMISC::CVector currLookAtDir = currLookAtPos - currPos;
	currLookAtDir.normalize();

	return currLookAtDir;
}
