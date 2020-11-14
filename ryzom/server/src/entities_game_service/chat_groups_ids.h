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



#ifndef RY_EGS_CHAT_GROUP_IDS_H
#define RY_EGS_CHAT_GROUP_IDS_H

#include "nel/misc/progress_callback.h"
#include "game_share/chat_group.h"

namespace CHAT_GROUPS_IDS
{

static const uint32 TeamBase = 0x0000001;
static const uint32 GuildBase = 0x10000000;
static const uint32 RegionBase = 0x20000000;


inline TGroupId getGuildChatGroupId( const EGSPD::TGuildId & guildId )
{
	NLMISC::CEntityId eId;
	eId.setShortId( guildId + GuildBase );
	eId.setType( RYZOMID::chatGroup );
	return eId;
}

inline TGroupId getTeamChatGroupId( uint32 teamId )
{
	return NLMISC::CEntityId( RYZOMID::chatGroup, teamId + TeamBase );
}

inline TGroupId getRegionChatGroupId( uint32 regionId )
{
	return NLMISC::CEntityId( RYZOMID::chatGroup, regionId + RegionBase );
}

}

#endif // RY_EGS_CHAT_GROUP_IDS_H

/* End of chat_groups_ids.h */
