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

#error
#ifndef IOS_CHAT_LOG_H
#define IOS_CHAT_LOG_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/common.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/sheet_id.h>
#include <vector>
#include <map>
#include <pd_lib/pd_lib.h>
#include <game_share/persistent_data.h>

// User #includes

namespace IOSPD
{
	
//
// Forward declarations
//



//
// Typedefs & Enums
//

void							createTeam(NLMISC::CEntityId team);

void							deleteTeam(NLMISC::CEntityId team);

void							playerJoinsTeam(NLMISC::CEntityId player, NLMISC::CEntityId team);

void							playerLeavesTeam(NLMISC::CEntityId player, NLMISC::CEntityId team);

	
} // End of IOSPD

#endif
