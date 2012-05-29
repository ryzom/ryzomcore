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



#ifndef RY_STAT_DB_MSG_H
#define RY_STAT_DB_MSG_H

#include "nel/misc/entity_id.h"
#include "nel/misc/stream.h"

#include "stat_db_common.h"


struct CStatDBValueLeafMsg
{
	void serial(NLMISC::IStream & f)
	{
		f.serial(Path);
		f.serial(Value);
	}

	CStatDBValueLeafMsg() : Value(0) {}

	std::string Path;
	sint32		Value;
};

struct CStatDBTableLeafMsg
{
	void serial(NLMISC::IStream & f)
	{
		f.serial(Path);
		f.serialCont(PlayerValues);
		f.serialCont(GuildValues);
	}

	std::string							Path;
	std::map<NLMISC::CEntityId,sint32>	PlayerValues;
	std::map<EGSPD::TGuildId,sint32>	GuildValues;
};

struct CStatDBNamesMsg
{
	void serial(NLMISC::IStream & f)
	{
		f.serialCont(PlayerNames);
		f.serialCont(GuildNames);
	}

	std::map<NLMISC::CEntityId,std::string>	PlayerNames;
	std::map<EGSPD::TGuildId,std::string>	GuildNames;
};

struct CStatDBAllLeavesMsg
{
	void serial(NLMISC::IStream & f)
	{
		f.serialCont(ValueLeavesMsg);
		f.serialCont(TableLeavesMsg);
		f.serial(NamesMsg);
	}

	std::vector<CStatDBValueLeafMsg>	ValueLeavesMsg;
	std::vector<CStatDBTableLeafMsg>	TableLeavesMsg;
	CStatDBNamesMsg						NamesMsg;
};


#endif // RY_STAT_DB_MSG_H
