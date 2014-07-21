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

#include "persistant_data_service.h"

#include "db_manager.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//-----------------------------------------------
//	callback table for input message 
//
//-----------------------------------------------
TUnifiedCallbackItem CbArray[] =
{
	{ "",	NULL },
};


/*
 * Constructor
 */
CPersistantDataService::CPersistantDataService()
{
}


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN( CPersistantDataService, "PDS", "persistant_data_service", 0, CbArray, "", "" );





/*
 * Initialization
 */
void	CPersistantDataService::init()
{
//	RY_PDS::CPDStringManager	sm;

//	sm.addString(CEntityId(12, 0), string(""));
//	sm.stringExists(string(""), 12);

	CDbManager::init();
}


/*
 * Release
 */
void	CPersistantDataService::release()
{
	CDbManager::release();
}




/*
 * Update
 */
bool	CPersistantDataService::update()
{
	if (!CDbManager::update())
		return false;

	return true;
}
