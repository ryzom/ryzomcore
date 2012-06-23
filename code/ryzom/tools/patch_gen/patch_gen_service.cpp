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

#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"

#include "game_share/ryzom_version.h"
#include "game_share/tick_event_handler.h"
#include "game_share/singleton_registry.h"

#include "patch_gen_service.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;


/*
 * Initialise the service
 */
void CPatchGenService::init()
{
	setVersion (RYZOM_VERSION);
	CSingletonRegistry::getInstance()->init();
}

/*
 * Service Update
 */
bool CPatchGenService::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();
	return true;
}

/*
 * Release
 */
void CPatchGenService::release()
{
	CSingletonRegistry::getInstance()->release();
}

//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN( CPatchGenService, "PGS", "patch_gen_service", 0, EmptyCallbackArray, "", "" );
