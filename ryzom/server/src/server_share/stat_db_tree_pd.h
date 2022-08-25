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



#ifndef RY_STAT_DB_TREE_PD_H
#define RY_STAT_DB_TREE_PD_H

#include "nel/misc/entity_id.h"

#include "stat_db_common.h"
#include "game_share/persistent_data.h"


struct CStatDBValueLeafPD
{
	DECLARE_PERSISTENCE_METHODS

	CStatDBValueLeafPD() : Value(0) {}

	std::string Path;
	sint32		Value;
};

struct CStatDBTableLeafPD
{
	DECLARE_PERSISTENCE_METHODS

	std::string							Path;
	std::map<NLMISC::CEntityId,sint32>	PlayerValues;
	std::map<EGSPD::TGuildId,sint32>	GuildValues;
};

struct CStatDBValueLeavesPD
{
	DECLARE_PERSISTENCE_METHODS

	std::vector<CStatDBValueLeafPD>		ValueLeavesPD;
};


#endif // RY_STAT_DB_TREE_PD_H
