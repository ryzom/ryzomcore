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


#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "nel/misc/cdb_manager.h"

namespace NLGUI
{

	class CDBManager : public NLMISC::CCDBManager
	{
	public:
		static CDBManager* getInstance();
		static void release();
		
		NLMISC::CCDBNodeLeaf* getDbProp( const std::string &name, bool create = true );
		void delDbProp( const std::string &name );
		sint32 getDbValue32( const std::string &name );
		NLMISC::CCDBNodeBranch* getDB() const;
	
	private:
		CDBManager();
		~CDBManager();
		
		static CDBManager *instance;
	
	};

}

#endif
