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

#include "variables.h"

#include "nel/misc/variable.h"
#include "nel/misc/entity_id.h"

#include "world_position_manager.h"

using namespace std;
using namespace NLMISC;

uint32			NumEntities			= 0;
uint32			NumPlayers			= 0;
bool			Verbose				= false;
bool			CheckPlayerSpeed	= true;


//
NLMISC_VARIABLE(uint32, NumEntities, "Number of entities currently on service");

//
NLMISC_VARIABLE(uint32, NumPlayers, "Number of players currently on service");

//
NLMISC_VARIABLE(bool, Verbose, "Allow display of verbose info");

//
NLMISC_VARIABLE(bool, CheckPlayerSpeed, "Allow checking player speed -- debug only!");


//
NLMISC_DYNVARIABLE(string, Watch0, "Display of entity 0")
{
	static NLMISC::CEntityId	id;
	if (get)
	{
		if (id != NLMISC::CEntityId::Unknown)
			*pointer = CWorldPositionManager::debugString(CWorldPositionManager::getEntityIndex(id));
	}
	else
	{
		id.fromString((*pointer).c_str());
	}
}

//
NLMISC_DYNVARIABLE(string, Watch1, "Display of entity 1")
{
	static NLMISC::CEntityId	id;
	if (get)
	{
		if (id != NLMISC::CEntityId::Unknown)
			*pointer = CWorldPositionManager::debugString(CWorldPositionManager::getEntityIndex(id));
	}
	else
	{
		id.fromString((*pointer).c_str());
	}
}
