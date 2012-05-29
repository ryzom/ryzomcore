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

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/sheet_id.h"

#include "nel/net/service.h"

#include "game_share/ryzom_version.h"
#include "game_share/tick_event_handler.h"
#include "game_share/singleton_registry.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;


/**
 * CStatsScanServiceSingleton
 */
class CStatsScanServiceSingleton : public NLNET::IService
{
public :

	/*
	 * Initialise the service
	 */
	void init()
	{
		setVersion (RYZOM_VERSION);

		// initialise sheet id file
		CSheetId::init(false);

		// if we are connecting to a shard then start by initializing the tick interface
		if (IService::getInstance()->ConfigFile.getVarPtr("DontUseTS")==NULL || IService::getInstance()->ConfigFile.getVarPtr("DontUseTS")->asInt()==0)
			CTickEventHandler::init(CStatsScanServiceSingleton::tickUpdate);

		CSingletonRegistry::getInstance()->init();
	}

	/*
	 * Service Update
	 */
	bool update()
	{
		CSingletonRegistry::getInstance()->serviceUpdate();
		return true;
	}

	/*
	 * Tick Update
	 */
	static void tickUpdate()
	{
		CSingletonRegistry::getInstance()->tickUpdate();
	}

	/*
	 * Release
	 */
	void release()
	{
		CSingletonRegistry::getInstance()->release();
	}

};


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN( CStatsScanServiceSingleton, "stats_scan", "stats_scan_service", 0, EmptyCallbackArray, "", "" );

