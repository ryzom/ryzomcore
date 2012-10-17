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

#include "ig_season_callback.h"
#include "weather.h"
#include "character_cl.h"
#include "client_sheets/entity_sheet.h"
#include "entities.h"

CIGSeasonCallback IGSeasonCallback;

void CIGSeasonCallback::instanceGroupAdded(NL3D::UInstanceGroup *ig)
{
	H_AUTO(RZ_IGSeasonCallback)
	// Set the season texture for all the buildings

	uint8 selectedTextureSet = (uint8) computeCurrSeason();

	const uint numInstances = ig->getNumInstance();
	for(uint k = 0; k < numInstances; k++)
	{
		NL3D::UInstance instance = ig->getInstance (k);
		if (!instance.empty())
			instance.selectTextureSet(selectedTextureSet);  // TODO Nico : instant loading here (async not useful)
	}
	// for all the bot objects, force to rebuild them to update their textures
	for (uint k = 0; k < CLFECOMMON::INVALID_SLOT; ++k)
	{
		CCharacterCL *charCL = dynamic_cast<CCharacterCL *>(EntitiesMngr.entity(k));
		if (charCL && charCL->getSheet())
		{
			std::string sheetName = charCL->getSheet()->Id.toString();
			static const  char *botObjectPrefix = "object_";
			if (NLMISC::nlstricmp(sheetName.substr(0, strlen(botObjectPrefix)), botObjectPrefix) == 0)
			{
				for(uint k = 0; k < charCL->instances().size(); ++k)
				{
					charCL->instances()[k].selectTextureSet(selectedTextureSet, false); // want immediate change here (frozen any way ...)
				}
				if (!charCL->instance().empty())
				{
					charCL->instance().selectTextureSet(selectedTextureSet);
				}
			}
		}
	}
}
