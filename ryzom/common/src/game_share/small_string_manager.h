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

#ifndef SMALL_STRING_MANAGER_H
#define SMALL_STRING_MANAGER_H

#include "nel/misc/types_nl.h"


#include "nel/misc/debug.h"
#include "nel/misc/stream.h"

#include "game_share/object.h"

#include <set>
#include <map>

namespace R2
{

	class CSmallStringManager
	{
	public:
		CSmallStringManager();

		~CSmallStringManager();

		CSmallStringManager(CObject* textManager);

		uint32 registerString(const std::string& phrase,uint32 nb = 1);

		void unregisterString(const std::string& phrase,uint32 nb = 1);

		void unregisterStringById(uint32 id,uint32 nb = 1);


		const std::string& getString(uint32 id) const;

		void serial(NLMISC::IStream& stream);

		void release();

	private:

		uint32 getNewId() ;

		std::map<std::string, uint32>	_StringToId;

		std::map<uint32, std::pair<std::string, uint32> >	_IdToString;

		std::set< uint32 > _FreeIds;

		uint32 _MaxId;

	};


	class CStringTableManager
	{
	public:
		typedef std::map<std::string, uint32> TLocalTable;// TableId -> StringId

	public:
		~CStringTableManager();

		void release();

		void setValue(uint32 tableId ,const std::string& localId, const std::string& value);

		std::string getValue(uint32 tableId, const std::string& localId) const;

		void createTable(uint32 tableId); // create empty table

		void releaseTable(uint32 tableId);

		virtual TLocalTable* getLocalTable(uint32 tableId) const;

	private:

		typedef std::map<uint32, TLocalTable*> TLocalTables; // SessionId -> Table

	private:


	private:
		CSmallStringManager _StringMgr;
		TLocalTables _LocalTables;
	};
}

#endif //SMALL_STRING_MANAGER
