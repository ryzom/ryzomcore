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






#ifndef NL_MISC_CONST_H
#define NL_MISC_CONST_H

/// Constants for CTeam class
namespace CTEAM
{
	const uint16	InvalidTeamId = 0xFFFF;
	const uint8		TeamMaxNbMembers = 9;
}

#define RZ_MAX_BUILDING_PER_OUTPOST 4

/**
 * Type representing a unique id in the AI system. Unique id are guaranted to be the same between two server sessions
 * They are used to get a bot entity id from the AI service
 * Null value : 0
 */
typedef uint32 TAIAlias;


/// Invalid AI Instance Id
const uint32	INVALID_AI_INSTANCE = 0xffffffff;


#endif // NL_MISC_CONST_H
