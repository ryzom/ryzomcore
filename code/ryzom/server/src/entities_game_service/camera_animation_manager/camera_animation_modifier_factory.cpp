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

#include "camera_animation_manager/camera_animation_modifier_factory.h"

std::vector<std::pair<std::string, ICameraAnimationModifierFactory*> >* ICameraAnimationModifierFactory::Entries;

ICameraAnimationModifier* ICameraAnimationModifierFactory::parseModifier(const NLLIGO::IPrimitive* prim, const std::string& filename, const std::string& modifierType)
{
	// We search the correct modifier type in our entries
	for (uint i = 0; i < Entries->size(); i++)
	{
		if (modifierType == (*Entries)[i].first)
		{
			ICameraAnimationModifier * ret = (*Entries)[i].second->instanciate();
			if (!ret)
			{
				nlerror("BUG IN CAMERA ANIMATION MODIFIER FACTORY : BAD INIT CODE");
				return NULL;
			}
			// We parse the modifier
			if (!ret->parseModifier(prim, filename))
			{
				nlerror("building camera animation modifier failed");
				delete ret;
				return NULL;
			}
			return ret;
		}
	}
	return NULL;
}

void ICameraAnimationModifierFactory::init()
{
	if (!Entries)
		Entries = new std::vector<std::pair<std::string, ICameraAnimationModifierFactory*> >;
}

void ICameraAnimationModifier::sendCameraFullModifier(NLMISC::CBitMemStream& bms)
{
	// We first add the name of the modifier
	std::string name = getModifierName();
	bms.serial(const_cast<std::string&>(name));

	// We ask the modifier to add its information to the message
	sendCameraModifier(bms);
}