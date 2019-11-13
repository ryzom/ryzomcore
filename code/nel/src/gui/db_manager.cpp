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
#include "nel/gui/db_manager.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	CDBManager* CDBManager::instance = NULL;

	CDBManager::CDBManager() :
	NLMISC::CCDBManager( "ROOT", 0 )
	{
	}

	CDBManager::~CDBManager()
	{
	}

	CDBManager* CDBManager::getInstance()
	{
		if( instance == NULL )
			instance = new CDBManager();
		return instance;
	}

	void CDBManager::release()
	{
		nlassert( instance != NULL );
		delete instance;
		instance = NULL;
	}

	NLMISC::CCDBNodeLeaf* CDBManager::getDbProp( const std::string &name, bool create )
	{
		return getDbLeaf( name, create );
	}

	void CDBManager::delDbProp( const std::string &name )
	{
		delDbNode( name );
	}

	sint32 CDBManager::getDbValue32( const std::string &name )
	{
		NLMISC::CCDBNodeLeaf *node = getDbProp( name, false );
		if( node != NULL )
			return node->getValue32();
		else
			return 0;
	}

	NLMISC::CCDBNodeBranch* CDBManager::getDB() const
	{
		return _Database;
	}
}