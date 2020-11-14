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


#ifndef RYAI_COMBAT_INTERFACE_H
#define RYAI_COMBAT_INTERFACE_H

// Nel Misc
#include "nel/misc/types_nl.h"

// Game share
#include "game_share/ryzom_entity_id.h"
#include "game_share/action_nature.h"
#include "nel/misc/sheet_id.h"

class	CAIEntityPhysical;
class	CModEntityPhysical;

// the class
class CCombatInterface
{
public:
	static void init();
	static void release();

	class CEvent
	{
	public:
		TDataSetRow					_originatorRow;
		TDataSetRow					_targetRow;
		float						_weight;
		ACTNATURE::TActionNature	_nature;
	};
	static std::list <CEvent> _events;
};

#endif
