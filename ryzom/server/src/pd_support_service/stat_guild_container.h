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

#ifndef STAT_GUILD_CONTAINER_H
#define STAT_GUILD_CONTAINER_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"
#include "game_share/persistent_data_tree.h"


//-------------------------------------------------------------------------------------------------
// class CStatGuildContainer
//-------------------------------------------------------------------------------------------------

class CStatGuildContainer: public NLMISC::CRefCount
{
public:
	CStatGuildContainer();
	~CStatGuildContainer();

public:
	// interface for the guild scan job to use
	void startScan();
	void addGuildFile(const NLMISC::CSString& fileName, CPersistentDataRecord& fileContent);
	void endScan();

	// handy introspection routines
	void display(NLMISC::CLog* log=NLMISC::InfoLog);

	// write a table (line per guild member), columns: file name, guild name, guild id, rank, entry date, account, slot 
	void writeMemberListFile(const NLMISC::CSString& path);

	// write a table (line per item in guild inventory), columns: file name, guild name, guild id, slot, <item stats>
	void writeInventoryFile(const NLMISC::CSString& path);

	// write a table (line per guild), columns for all basic properties of the guild
	void writeMiscDataFile(const NLMISC::CSString& path);

private:
	// the data representation for a guild file
	struct CGuildFile
	{
		NLMISC::CSString FileName;
		CPersistentDataTree FileContent;
	};

	typedef std::vector<CGuildFile> TGuildFiles;
	TGuildFiles _GuildFiles;

	bool _InProgress;
};


//-------------------------------------------------------------------------------------------------
#endif
