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
#include "characs_build.h"
#include "nel/georges/u_form_elm.h"


void loadCharacteristicsFromSheet(const NLGEORGES::UFormElm &rootNode, std::string prefix, sint8 dest[CHARACTERISTICS::NUM_CHARACTERISTICS])
{
	for(uint k = 0; k < CHARACTERISTICS::NUM_CHARACTERISTICS; ++k)
	{
		const std::string &characName = CHARACTERISTICS::toString((CHARACTERISTICS::TCharacteristics) k);
		std::string characPath = prefix + characName;
		if(!rootNode.getValueByName(dest[k], characPath.c_str()))
		{
			nlwarning(("Key " + characName + "not found.").c_str());
		}
	}
}

