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

#include "stdpch.h"

#include "camera_animation_manager/camera_animation_manager.h"
#include "primitives_parser.h"
#include "nel/ligo/primitive.h"
#include "camera_animation_manager/camera_animation_step_factory.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

CCameraAnimationManager* CCameraAnimationManager::_Instance = NULL;

void CCameraAnimationManager::init()
{
	// Just asserts the instance is not already created and we create the manager that will load the camera animations
	nlassert(_Instance == NULL);
	_Instance = new CCameraAnimationManager();
}

void CCameraAnimationManager::release()
{
	// We delete the instance of the manager which will delete the allocated resources
	delete _Instance;
	_Instance = NULL;
}

CCameraAnimationManager::CCameraAnimationManager()
{
	// We get the loaded primitives
	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();

	nlinfo("parsing the camera animations");

	// We begin to parse the camera animations
	CPrimitivesParser::TPrimitivesList::const_iterator first, last;
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		// parse camera animations
		if (!parseCameraAnimations(first->Primitive.RootNode, first->FileName))
		{
			nlwarning("<CCameraAnimationManager constructor> Error while parsing the camera animations in primitive number '%s'", first->FileName.c_str());
		}
	}
}

CCameraAnimationManager::~CCameraAnimationManager()
{
	// Delete the list of animations
	TCameraAnimationContainer::iterator first = Animations.begin();
	for (; first != Animations.end(); ++first)
	{
		first->second.release();
	}
	Animations.clear();
}

bool CCameraAnimationManager::parseCameraAnimations(const IPrimitive* prim, const std::string& filename)
{
	string value;

	// if the node is a camera animation parse it
	if (prim->getPropertyByName("class", value) && !nlstricmp(value.c_str(), "camera_animation_tree"))
	{
		// We get the name of the mission
		prim->getPropertyByName("name", value);

		string animName = value;

		TCameraAnimInfo animInfo;
		animInfo.Name = animName;

		// We now parse the instructions which are children of the camera animation
		for (uint i = 0; i < prim->getNumChildren(); i++)
		{
			const IPrimitive* child;
			prim->getChild(child, i);

			// We tell the factory to load the instructions in function of the type of instruction we have
			string stepType;
			if (!child->getPropertyByName("class", stepType))
			{
				nlwarning("<CCameraAnimationManager parseCameraAnimations> Error while getting the class of a camera animation step in primitive number '%s'", filename.c_str());
				continue;
			}

			ICameraAnimationStep* step = ICameraAnimationStepFactory::parseStep(child, filename, stepType);
			// We add the instruction to the list
			if (step)
			{
				animInfo.Steps.push_back(step);
			}
		}

		// We add the camera animation to the container
		Animations[animName] = animInfo;
		
		return true;
	}
	else
	{
		// lookup recursively in the children
		bool ok = true;
		for (uint i = 0; i < prim->getNumChildren(); ++i)
		{
			const IPrimitive *child;
			if (!prim->getChild(child,i) || !parseCameraAnimations(child, filename))
				ok = false;
		}
		return ok;
	}
}