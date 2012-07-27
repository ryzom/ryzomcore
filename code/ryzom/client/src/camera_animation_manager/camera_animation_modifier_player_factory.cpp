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

#include "camera_animation_manager/camera_animation_modifier_player_factory.h"

std::vector<std::pair<std::string, ICameraAnimationModifierPlayerFactory*> >* ICameraAnimationModifierPlayerFactory::Entries;

ICameraAnimationModifierPlayer* ICameraAnimationModifierPlayerFactory::initModifier(const std::string& name, NLMISC::CBitMemStream& impulse)
{
	if (!Entries)
		return NULL;

	// We search the correct modifier type in our entries
	for (uint i = 0; i < Entries->size(); i++)
	{
		if (name == (*Entries)[i].first)
		{
			ICameraAnimationModifierPlayer* ret = (*Entries)[i].second->instanciate();
			if (!ret)
			{
				nlwarning("BUG IN CAMERA ANIMATION MODIFIER PLAYER FACTORY : BAD INIT CODE %s", name.c_str());
				return NULL;
			}
			// We init the modifier
			if (!ret->initModifier(impulse))
			{
				nlwarning("building camera animation modifier player failed %s", name.c_str());
				delete ret;
				return NULL;
			}

			return ret;
		}
	}
	return NULL;
}

void ICameraAnimationModifierPlayerFactory::init()
{
	if (!Entries)
		Entries = new std::vector<std::pair<std::string, ICameraAnimationModifierPlayerFactory*> >;
}

