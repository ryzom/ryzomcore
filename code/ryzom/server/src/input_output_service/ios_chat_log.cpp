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
#include "ios_pd.h"

namespace IOSPD
{
	
void							createTeam(NLMISC::CEntityId team)
{
	PDSLib.log(0);
	PDSLib.logPush(team);
}

void							deleteTeam(NLMISC::CEntityId team)
{
	PDSLib.log(1);
	PDSLib.logPush(team);
}

void							playerJoinsTeam(NLMISC::CEntityId player, NLMISC::CEntityId team)
{
	PDSLib.log(2);
	PDSLib.logPush(player);
	PDSLib.logPush(team);
}

void							playerLeavesTeam(NLMISC::CEntityId player, NLMISC::CEntityId team)
{
	PDSLib.log(3);
	PDSLib.logPush(player);
	PDSLib.logPush(team);
}

	
} // End of IOSPD
