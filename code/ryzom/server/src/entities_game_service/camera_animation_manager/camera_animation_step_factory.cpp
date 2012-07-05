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

#include "camera_animation_manager/camera_animation_step_factory.h"

std::vector<std::pair<std::string, ICameraAnimationStepFactory*> >* ICameraAnimationStepFactory::Entries;

ICameraAnimationStep* ICameraAnimationStepFactory::parseStep(const NLLIGO::IPrimitive* prim, const std::string& filename, const std::string& stepType)
{
	// We search the correct step type in our entries
	for (uint i = 0; i < Entries->size(); i++)
	{
		if (stepType == (*Entries)[i].first)
		{
			ICameraAnimationStep * ret = (*Entries)[i].second->instanciate();
			if (!ret)
			{
				nlerror("BUG IN CAMERA ANIMATION STEP FACTORY : BAD INIT CODE");
				return NULL;
			}
			// We parse the step
			if (!ret->parseStep(prim, filename))
			{
				nlerror("building camera animation step failed");
				delete ret;
				return NULL;
			}

			// We look for children (modifiers or sound triggers)
			for (uint i = 0; i < prim->getNumChildren(); i++)
			{
				const NLLIGO::IPrimitive* child;
				prim->getChild(child, i);

				// We tell the factory to load the modifier in function of its type
				std::string modifierType;
				if (!child->getPropertyByName("class", modifierType))
				{
					nlwarning("<ICameraAnimationStepFactory parseStep> Error while getting the class of a camera animation modifier in primitive number '%s'", filename.c_str());
					continue;
				}

				ICameraAnimationModifier* modifier = ICameraAnimationModifierFactory::parseModifier(child, filename, modifierType);
				// We add the modifier to the list
				if (modifier)
				{
					ret->addModifier(modifier);
				}
			}

			return ret;
		}
	}
	return NULL;
}

void ICameraAnimationStepFactory::init()
{
	if (!Entries)
		Entries = new std::vector<std::pair<std::string, ICameraAnimationStepFactory*> >;
}

void ICameraAnimationStep::addModifier(ICameraAnimationModifier* modifier)
{
	Modifiers.push_back(modifier);
}

void ICameraAnimationStep::sendAnimationStep(const NLMISC::CEntityId& eid)
{
	throw std::exception("The method or operation is not implemented.");
}